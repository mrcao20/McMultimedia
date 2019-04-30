#include "McAudioIO.h"

#include <qdebug.h>

#include "IMcAudioData.h"
#include "McAudioFrame.h"
#include "McGlobal.h"

struct McAudioIOData {
	IMcAudioData *audio{ nullptr };				// 用于获取音频数据
	QSharedPointer<McAudioFrame> audioFrame;	// 用于保存解码出的音频数据，音频帧由本对象提供，且只new一次，避免重复new造成的时间损耗和内存碎片的产生
};

McAudioIO::McAudioIO(IMcAudioData *audio, QObject *parent)
	: QIODevice(parent)
	, d(new McAudioIOData())
{
	open(QIODevice::ReadWrite);
	d->audio = audio;
	d->audioFrame.reset(new McAudioFrame());	// 分配音频帧
}

McAudioIO::~McAudioIO(){
}

qint64 McAudioIO::readData(char *data, qint64 maxSize) {
	if (maxSize <= 0 || (d->audioFrame->getData() && d->audioFrame->getSize() > maxSize))	// maxSize小于等于0或者可写入空间不足以写入当前帧，直接返回
		return 0;
	qint64 size = 0;
	if (d->audioFrame->getData()) {	// 如果存在未播放的音频数据
		size += mixAudio(data, d->audioFrame);
	}
	while (size < maxSize) {	// 如果音频缓冲未写满
		d->audio->getAudioData(d->audioFrame, [&]() {
			if (d->audioFrame->getSize() > maxSize - size)	// 如果剩余空间不足以写入当前帧，则保存帧直接返回
				return;
			
			size += mixAudio(data + size, d->audioFrame);
		});
		if (d->audioFrame->getData())	// 如果当前帧存在，则表示剩余空间不足以写入当前帧，直接退出返回
			break;
	}
	return size;
}

qint64 McAudioIO::writeData(const char *data, qint64 maxSize) {
	return 0;
}

qint64 McAudioIO::mixAudio(char *buffer, QSharedPointer<McAudioFrame> &frame) noexcept {
	emit signal_clockChanged(frame->getStartClock(), frame->getEndClock());
	qint64 frameSize = frame->getSize();
	memcpy(buffer, frame->getData(), frameSize);
	frame->setData(nullptr);	// 音频数据成功写入，置空
	return frameSize;
}
