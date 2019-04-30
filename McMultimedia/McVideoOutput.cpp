#include "McVideoOutput.h"

#include <QtConcurrent>

#include "IMcVideo.h"
#include "IMcMediaClock.h"
#include "McVideoFrame.h"
#include "IMcVideoRenderer.h"

struct McVideoOutputData {
	IMcVideo *video{ nullptr };				// 用于获取视频数据
	IMcMediaClock *mediaClock{ nullptr };	// 媒体时钟，用来同步视频 

	IMcVideoRenderer *renderer{ nullptr };
	QSharedPointer<McVideoFrame> videoFrame;	// 用来保存视频帧，视频帧由本对象提供且只new一次，避免重复new造成的时间消耗和内存碎片
};

McVideoOutput::McVideoOutput(QObject *parent)
	: QObject(parent)
	, d(new McVideoOutputData())
{
	d->videoFrame.reset(new McVideoFrame());
}

McVideoOutput::~McVideoOutput(){
}

void McVideoOutput::setVideo(IMcVideo *video) noexcept {
	d->video = video;
	if (d->renderer)
		d->video->setVideoFormat(d->renderer->getVideoFormat());
}

void McVideoOutput::setMediaClock(IMcMediaClock *mediaClock) noexcept {
	d->mediaClock = mediaClock;
}

void McVideoOutput::setRenderer(IMcVideoRenderer *renderer) noexcept {
	d->renderer = renderer;
	d->renderer->setVideoFrame(d->videoFrame);
	if(d->video)
		d->video->setVideoFormat(d->renderer->getVideoFormat());
}

void McVideoOutput::start() noexcept {
	QtConcurrent::run([this]() {
		while (true) {
			d->video->getVideoData(d->videoFrame, [this]() {
				mediaSync(d->videoFrame->getClock());
				d->renderer->rendering();
			});
		}
	});
}

bool McVideoOutput::mediaSync(qint64 clock) noexcept {
	while (true) {
		qint64 syncClock = d->mediaClock->getMediaClock();
		if (clock < syncClock)
			return true;

		int delayTime = clock - syncClock;

		delayTime = delayTime > 5 ? 5 : delayTime;

		// 如果视频快了，则等待
		QThread::msleep(delayTime);
	}
}
