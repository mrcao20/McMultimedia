#include "McAudioOutput.h"

#include <qaudiooutput.h>
#include <qdebug.h>

#include "IMcAudio.h"
#include "McAudioIO.h"
#include "McGlobal.h"

struct McAudioOutputData {
	IMcAudio *audio{ nullptr };			// 用于获取音频数据

	McAudioIO *io{ nullptr };			// IO
	QAudioOutput *output{ nullptr };	// 音频输出设备

	qint64 audioEndClock{ 0 };			// 当前已写入音频设备的数据的结束时间
	bool isStop{ false };				// 音频输出是否停止
};

McAudioOutput::McAudioOutput(QObject *parent)
	: QObject(parent)
	, d(new McAudioOutputData())
{
}

McAudioOutput::~McAudioOutput(){
	quit();
}

qint64 McAudioOutput::getMediaClock() noexcept {
	if (!d->audio)
		return -1;
	double clock = d->audioEndClock / 1000.0;	// ms转s
	int hwBufSize = d->output->bufferSize() - d->output->bytesFree();	// 还未播放完全的音频数据
	clock -= getPlayTime(hwBufSize);
	return clock * 1000;	// s转ms
}

void McAudioOutput::setAudio(IMcAudio *audio) noexcept {
	d->audio = audio;
}

bool McAudioOutput::start() noexcept {
	if (!d->audio)
		return true;
	quit();
	d->isStop = false;	// 开始音频输出
	d->output = new QAudioOutput(d->audio->getAudioFormat(), this);
	McAudioIO *io = new McAudioIO(d->audio, d->output);

	connect(d->output, &QAudioOutput::stateChanged, [this](QAudio::State state) {
		if (state == QAudio::IdleState && !d->isStop) {	// 如果音频设备为空闲状态，则需要重新开始播放
			pause();
			resume();
		}
	});
	connect(io, &McAudioIO::signal_clockChanged, [this](qint64 startClock, qint64 startIndex, qint64 endClock, qint64 remainDataSize) {
		startClock += (getPlayTime(startIndex) * 1000);	// s转ms
		emit signal_clockChanged(startClock);
		endClock -= (getPlayTime(remainDataSize) * 1000);	// s转ms
		d->audioEndClock = endClock;
	});

	d->output->start(io);
	d->io = io;
	return true;
}

void McAudioOutput::pause() noexcept {
	if (d->isStop)
		return;
	MC_SAFE_CALL(d->output, suspend());
}

void McAudioOutput::resume() noexcept {
	if (d->isStop)
		return;
	MC_SAFE_CALL(d->output, resume());
}

void McAudioOutput::stop() noexcept {
	pause();	// 此处不调用stop，仅仅只暂停音频输出
	d->isStop = true;	// 停止音频输出
}

void McAudioOutput::quit() noexcept {
	stop();
	MC_SAFE_CALL(d->output, stop());	// 释放音频资源
	release();		// 释放本地资源，重新开始
}

void McAudioOutput::seek(qint64 pos) noexcept {
	d->audioEndClock = pos;
	MC_SAFE_CALL(d->io, seekTo(pos));
}

void McAudioOutput::release() noexcept {
	MC_SAFE_CALL(d->output, deleteLater());
	d->output = nullptr;
	d->io = nullptr;
}

double McAudioOutput::getPlayTime(int bufSize) noexcept {
	QAudioFormat format = d->audio->getAudioFormat();
	int bytesPerSec = format.sampleRate() * format.channelCount() * format.sampleSize() / 8;
	return static_cast<double>(bufSize) / bytesPerSec;
}
