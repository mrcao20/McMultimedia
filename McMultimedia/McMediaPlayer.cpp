#include "McMediaPlayer.h"

#include "McMediaControl.h"
#include "McMediaDecoder.h"
#include "McAudioDecoder.h"
#include "McVideoDecoder.h"
#include "McAudioOutput.h"
#include "McVideoOutput.h"

struct McMediaPlayerData {
	McMediaControl *mediaControl{ nullptr };	// 媒体的控制器，提供播放暂停等功能
	McMediaDecoder *mediaDecoder{ nullptr };
	McAudioOutput *audioOutput{ nullptr };
	McVideoOutput *videoOutput{ nullptr };
	qint64 position{ 0 };						// 媒体当前播放的位置，单位：ms
};

McMediaPlayer::McMediaPlayer(QObject *parent)
	: QObject(parent)
	, d(new McMediaPlayerData())
{
	McMediaDecoder *mediaDecoder = new McMediaDecoder();
	McAudioDecoder *audioDecoder = new McAudioDecoder(this);
	McVideoDecoder *videoDecoder = new McVideoDecoder(this);

	mediaDecoder->addDecoder(McMediaDecoder::DecoderType::AUDIO, audioDecoder);
	mediaDecoder->addDecoder(McMediaDecoder::DecoderType::VIDEO, videoDecoder);

	McAudioOutput *audioOutput = new McAudioOutput(this);
	McVideoOutput *videoOutput = new McVideoOutput(this);
	audioOutput->setAudio(audioDecoder);
	videoOutput->setVideo(videoDecoder);
	videoOutput->setMediaClock(audioOutput);

	d->mediaDecoder = mediaDecoder;

	d->audioOutput = audioOutput;
	d->videoOutput = videoOutput;
}

McMediaPlayer::~McMediaPlayer(){
	d->mediaDecoder->stop();
}

void McMediaPlayer::setMediaUrl(const QString &url) {
	d->mediaDecoder->setMediaUrl(url);
}

void McMediaPlayer::play() {
	d->mediaDecoder->start();

	d->audioOutput->start();
	d->videoOutput->start();
}

qint64 McMediaPlayer::getPosition() {
	return d->position;
}

void McMediaPlayer::setPosition(qint64 position) {

}

void McMediaPlayer::setVideoRenderer(IMcVideoRenderer *renderer) noexcept {
	d->videoOutput->setRenderer(renderer);
}
