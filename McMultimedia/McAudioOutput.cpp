#include "McAudioOutput.h"

#include <qaudiooutput.h>
#include <qdebug.h>

#include "IMcAudio.h"
#include "McAudioIO.h"

struct McAudioOutputData {
	IMcAudio *audio{ nullptr };			// 用于获取音频数据

	QAudioOutput *output{ nullptr };	// 音频输出设备
	QIODevice *io{ nullptr };			// 音频IO

	qint64 curAudioEndClock{ 0 };
};

McAudioOutput::McAudioOutput(QObject *parent)
	: QObject(parent)
	, d(new McAudioOutputData())
{
}

McAudioOutput::~McAudioOutput(){
}

qint64 McAudioOutput::getMediaClock() noexcept {
	double clock = d->curAudioEndClock / 1000.0;	// ms转s
	QAudioFormat format = d->audio->getAudioFormat();
	int hwBufSize = d->output->bufferSize() - d->output->bytesFree();
	int bytesPerSec = format.sampleRate() * format.channelCount() * format.sampleSize() / 8;
	clock -= static_cast<double>(hwBufSize) / bytesPerSec;
	return clock * 1000;	// s转ms
}

void McAudioOutput::setAudio(IMcAudio *audio) noexcept {
	d->audio = audio;
}

void McAudioOutput::start() noexcept {
	McAudioIO *io = new McAudioIO(d->audio, this);
	connect(io, &McAudioIO::signal_clockChanged, [this](qint64 startClock, qint64 endClock) {
		emit signal_clockChanged(startClock);
		d->curAudioEndClock = endClock;
	});
	d->io = io;

	d->output = new QAudioOutput(d->audio->getAudioFormat(), this);
	d->output->start(d->io);
}
