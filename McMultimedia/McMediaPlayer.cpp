#include "McMediaPlayer.h"

#include <qdebug.h>
#include <qevent.h>
#include <qcoreapplication.h>

#include "McMediaControl.h"
#include "McMediaDecoder.h"
#include "McAudioDecoder.h"
#include "McVideoDecoder.h"
#include "McAudioOutput.h"
#include "McVideoOutput.h"

struct McMediaPlayerData {
	IMcMediaControl *mediaControl{ nullptr };	// 媒体的控制器，提供播放暂停等功能
	IMcMediaDecoder *mediaDecoder{ nullptr };	// 解码器
	IMcVideoOutput *videoOutput{ nullptr };		// 视频输出
	qint64 position{ 0 };						// 媒体当前播放的位置，单位：ms
	qint64 duration{ 0 };						// 媒体总时长，单位 ms

	QString mediaUrl;						// 当前媒体
	McMediaPlayer::State state{ McMediaPlayer::StoppedState };	// 当前状态
};

McMediaPlayer::McMediaPlayer(QObject *parent)
	: QObject(parent)
	, d(new McMediaPlayerData())
{
	McMediaControl *control = new McMediaControl(this);

	McMediaDecoder *mediaDecoder = new McMediaDecoder(control);
	McAudioOutput *audioOutput = new McAudioOutput(control);
	McVideoOutput *videoOutput = new McVideoOutput(control);

	McAudioDecoder *audioDecoder = new McAudioDecoder(audioOutput);
	McVideoDecoder *videoDecoder = new McVideoDecoder(videoOutput);

	mediaDecoder->addDecoder(McMediaDecoder::DecoderType::AUDIO, audioDecoder);
	mediaDecoder->addDecoder(McMediaDecoder::DecoderType::VIDEO, videoDecoder);
	
	audioOutput->setAudio(audioDecoder);
	videoOutput->setVideo(videoDecoder);
	videoOutput->setMediaClock(audioOutput);

	control->addControl(mediaDecoder);
	control->addControl(audioOutput);
	control->addControl(videoOutput);

	connect(mediaDecoder, &McMediaDecoder::signal_durationChanged, [this](qint64 dur) {
		d->position = 0;
		d->duration = dur;
		emit signal_durationChanged(d->duration);
	});
	connect(mediaDecoder, &McMediaDecoder::signal_decodeFinished, [this]() {
		if (d->state == StoppedState) {
			return;
		}
		setPos(d->duration);
		stop();
	});
	connect(audioOutput, &McAudioOutput::signal_clockChanged, this, &McMediaPlayer::setPos);

	d->mediaControl = control;
	d->mediaDecoder = mediaDecoder;
	d->videoOutput = videoOutput;
}

McMediaPlayer::~McMediaPlayer(){
}

void McMediaPlayer::setMediaUrl(const QString &url) noexcept {
	d->mediaUrl = url;
	d->mediaDecoder->setMediaUrl(url);
}

McMediaPlayer::State McMediaPlayer::getState() noexcept {
	return d->state;
}

qint64 McMediaPlayer::getPosition() noexcept {
	return d->position;
}

void McMediaPlayer::setPosition(qint64 position) noexcept {
	// 视频跳转时不能直接跳转到视频结尾，必须要比结尾小一点，此处小100ms
	qint64 maxPos = d->duration - 100;
	position = position >= maxPos ? maxPos : position;
	setPos(position);
	d->mediaControl->seek(position);
}

void McMediaPlayer::setVideoRenderer(IMcVideoRenderer *renderer) noexcept {
	d->videoOutput->setRenderer(renderer);
}

void McMediaPlayer::play() noexcept {
	if (d->state != State::StoppedState)
		return;
	if (d->mediaUrl.isEmpty()) {
		qCritical() << "please set media!!";
		return;
	}
	d->mediaControl->start();
	setState(State::PlayingState);
}

void McMediaPlayer::stop() noexcept {
	if (d->state == State::StoppedState)
		return;
	setState(State::StoppedState);
	d->mediaControl->stop();
	// 由于quit函数可能会造成暂停从而造成界面卡顿，所以这里采用异步调用
	qApp->postEvent(this, new QEvent(QEvent::User));
}

void McMediaPlayer::pause() noexcept {
	if (d->state == State::PausedState)
		return;
	setState(State::PausedState);
	d->mediaControl->pause();
}

void McMediaPlayer::resume() noexcept {
	if (d->state == State::PlayingState)
		return;
	setState(State::PlayingState);
	d->mediaControl->resume();
}

bool McMediaPlayer::event(QEvent *e) {
	if (e->type() != QEvent::User)
		return QObject::event(e);
	d->mediaControl->quit();
	return true;
}

void McMediaPlayer::setState(State state) noexcept {
	d->state = state;
	emit signal_stateChanged(d->state);
}

void McMediaPlayer::setPos(qint64 pos) noexcept {
	pos = pos < 0 ? 0 : pos;
	pos = pos > d->duration ? d->duration : pos;
	// 当需要跳转到的位置不等于0，也不等于总时长，并且和当前位置的间隔小于1000ms，则不跳转
	if (pos != 0 && pos != d->duration && (qAbs(pos - d->position) < 1000)) {
		return;
	}
	// 当需要跳转到的位置和当前位置一样，则不跳转
	if (d->position == pos) {
		return;
	}
	d->position = pos;
	emit signal_positionChanged(d->position);
}
