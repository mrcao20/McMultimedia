#include "McVideoOutput.h"

#include <QtConcurrent>
#include <qpointer.h>

#include "IMcVideo.h"
#include "IMcMediaClock.h"
#include "McVideoFrame.h"
#include "IMcVideoRenderer.h"
#include "McGlobal.h"

struct McVideoOutputData {
	IMcVideo *video{ nullptr };				// 用于获取视频数据
	IMcMediaClock *mediaClock{ nullptr };	// 媒体时钟，用来同步视频 

	QPointer<QObject> rendererObj;			// 渲染器对象，用于判断渲染器是否被删除
	IMcVideoRenderer *renderer{ nullptr };	// 渲染器
	QSharedPointer<McVideoFrame> videoFrame;	// 用来保存视频帧，视频帧由本对象提供且只new一次，避免重复new造成的时间消耗和内存碎片

	bool isQuit{ false };						// 是否停止播放
	bool isPause{ false };						// 是否暂停播放
	qint64 seekPos{ -1 };						// 将要跳转到的位置，默认为-1，表示不跳转，单位：us

	QFuture<void> playThread;					// 播放线程
};

McVideoOutput::McVideoOutput(QObject *parent)
	: QObject(parent)
	, d(new McVideoOutputData())
{
	d->videoFrame.reset(new McVideoFrame());
}

McVideoOutput::~McVideoOutput(){
	quit();
}

void McVideoOutput::setVideo(IMcVideo *video) noexcept {
	d->video = video;
	d->video->setVideoFrame(d->videoFrame);
	if (d->renderer)
		d->video->setVideoFormat(d->renderer->getVideoFormat());
}

void McVideoOutput::setMediaClock(IMcMediaClock *mediaClock) noexcept {
	d->mediaClock = mediaClock;
}

void McVideoOutput::setRenderer(IMcVideoRenderer *renderer) noexcept {
	if (d->renderer == renderer)
		return;
	MC_SAFE_DELETE(d->renderer);	// 如果已经存在一个渲染器，则删除它
	d->rendererObj = dynamic_cast<QObject *>(renderer);
	d->renderer = renderer;
	if (!d->renderer)
		return;
	d->renderer->setVideoFrame(d->videoFrame);
	if(d->video)
		d->video->setVideoFormat(d->renderer->getVideoFormat());
}

bool McVideoOutput::start() noexcept {
	if (!d->video)
		return true;

	quit();
	d->seekPos = -1;
	resume();
	d->isQuit = false;
	d->playThread = QtConcurrent::run(this, &McVideoOutput::startDetach);
	return true;
}

void McVideoOutput::pause() noexcept {
	d->isPause = true;
}

void McVideoOutput::resume() noexcept {
	d->isPause = false;
}

void McVideoOutput::stop() noexcept {
	d->isQuit = true;
}

void McVideoOutput::quit() noexcept {
	stop();
	while (!d->playThread.isFinished())
		QThread::msleep(10);
	release();		// 清除上一次数据
}

void McVideoOutput::seek(qint64 pos) noexcept {
	d->seekPos = pos * 1000;		// ms转us
}

void McVideoOutput::release() noexcept {
	
}

void McVideoOutput::startDetach() noexcept {
	qint64 lastClock = 0;	// 单位us
	while (!d->isQuit) {
		if (d->isPause) {
			QThread::msleep(10);
			continue;
		}
		d->video->getVideoData([this, &lastClock]() {
			qint64 uClock = d->videoFrame->getClock() * 1000;	// 单位us
			/*	如果发生了跳转或视频帧发生了混乱，则不播放在跳转位置之前的视频
				即，当发生跳转时，跳转到的真实位置可能在希望位置之前，则需要去除掉这部分音频
				或当发生跳转时，如果是往后跳，但获取到的视频帧的数据比之前的时间还要大，则表示视频帧发生了混乱
			*/
			if (uClock < d->seekPos || (d->seekPos != -1 && d->seekPos <= lastClock && uClock > lastClock))
				return;
			d->seekPos = -1;
			
			if (mediaSync(d->videoFrame->getClock()) && !d->rendererObj.isNull()) {
				d->renderer->rendering();
				lastClock = uClock;
			}
		});
	}
	if (!d->rendererObj.isNull()) {
		d->videoFrame->setData(nullptr);
		d->renderer->rendering();
	}
}

bool McVideoOutput::mediaSync(qint64 clock) noexcept {
	while (!d->isQuit && d->seekPos == -1) {
		qint64 syncClock = d->mediaClock->getMediaClock();
		if (clock < syncClock)
			return true;

		int delayTime = clock - syncClock;

		delayTime = delayTime > 5 ? 5 : delayTime;

		// 如果视频快了，则等待
		QThread::msleep(delayTime);
	}
	return false;
}
