#include "McAudioIO.h"

#include <qdebug.h>
#include <qmutex.h>

#include "IMcAudioData.h"
#include "McAudioFrame.h"
#include "McGlobal.h"

struct McAudioIOData {
	IMcAudioData *audio{ nullptr };				// 用于获取音频数据
	QSharedPointer<McAudioFrame> audioFrame;	// 用于保存解码出的音频数据，音频帧由本对象提供，且只new一次，避免重复new造成的时间损耗和内存碎片的产生
	qint64 dataIndex{ 0 };						// 如果存在未播放的音频数据，则表示该数据的起始下标
	qint64 seekPos{ -1 };						// 将要跳转到的位置，默认为-1，表示不跳转，单位：us
};

McAudioIO::McAudioIO(IMcAudioData *audio, QObject *parent)
	: QIODevice(parent)
	, d(new McAudioIOData())
{
	open(QIODevice::ReadWrite);
	d->audio = audio;
	d->audioFrame.reset(new McAudioFrame());	// 分配音频帧
	d->audio->setAudioFrame(d->audioFrame);
}

McAudioIO::~McAudioIO(){
}

void McAudioIO::seekTo(qint64 pos) noexcept {
	d->seekPos = pos * 1000;		// ms转us
}

qint64 McAudioIO::readData(char *data, qint64 maxSize) {
	if (maxSize <= 0)	// maxSize小于等于0，直接返回
		return 0;
	qint64 size = 0;
	// 如果存在未播放的音频数据并且如果没有跳转，则播放剩余数据
	if (d->audioFrame->getData() && d->dataIndex < d->audioFrame->getSize() && d->seekPos == -1) {
		size = mixAudio(data, 0, d->audioFrame, d->dataIndex, maxSize);
		d->dataIndex += size;
	}
	while (size < maxSize) {	// 如果音频缓冲未写满时
		d->audio->getAudioData([&, this]() {
			/*	如果发生了跳转，则不播放在跳转位置之前的音频
				即，当发生跳转时，跳转到的真实位置可能在希望位置之前，则需要去除掉这部分音频
			*/
			if (d->audioFrame->getStartClock() * 1000 < d->seekPos) {
				size = 0;
				return;
			}
			d->seekPos = -1;

			d->dataIndex = mixAudio(data, size, d->audioFrame, 0, maxSize);
			size += d->dataIndex;
		});
		// 如果数据不存在或者数据大小小于等于0，表示可能没有数据可读取，则直接退出返回
		if (!d->audioFrame->getData() || d->audioFrame->getSize() <= 0)
			break;
	}
	return size;
}

qint64 McAudioIO::writeData(const char *data, qint64 maxSize) {
	return 0;
}

qint64 McAudioIO::mixAudio(char *buffer, qint64 bufferIndex, const QSharedPointer<McAudioFrame> &frame, qint64 dataIndex, qint64 maxSize) noexcept {
	QMutexLocker locker(&d->audioFrame->getMutex());
	if (!frame->getData() || frame->getSize() <= 0)	// 如果音频数据不存在，或者音频大小小于等于0，则不写入，直接返回
		return 0;
	qint64 frameSize = frame->getSize() - dataIndex;	// 获取当前还剩余的音频大小
	frameSize = frameSize < 0 ? 0 : frameSize;			// 最小值不能小于0
	frameSize = qMin(frameSize, maxSize - bufferIndex);	// 获取可写入的大小
	emit signal_clockChanged(frame->getStartClock(), dataIndex, frame->getEndClock(), frame->getSize() - dataIndex - frameSize);
	memcpy(buffer + bufferIndex, frame->getData() + dataIndex, frameSize);
	return frameSize;	// 返回已写入音频大小
}
