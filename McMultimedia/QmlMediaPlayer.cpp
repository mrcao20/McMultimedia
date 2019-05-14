#include "QmlMediaPlayer.h"

QmlMediaPlayer::QmlMediaPlayer(QQuickItem *parent)
	: QQuickItem(parent)
{
	connect(&m_mediaPlayer, &McMediaPlayer::signal_positionChanged, [this](qint64 pos) {
		m_position = static_cast<qreal>(pos);
		emit signal_positionChanged();
	});
	connect(&m_mediaPlayer, &McMediaPlayer::signal_durationChanged, [this](qint64 duration) {
		m_duration = static_cast<qreal>(duration);
		emit signal_durationChanged();
	});
	connect(&m_mediaPlayer, &McMediaPlayer::signal_stateChanged, [this](McMediaPlayer::State state) {
		m_state = static_cast<QmlMediaPlayer::State>(state);
		emit signal_stateChanged();
	});
	connect(this, &QmlMediaPlayer::signal_sourceChanged, [this]() {
		m_mediaPlayer.setMediaUrl(m_mediaUrl);
		play();
	});
}

void QmlMediaPlayer::play() noexcept {
	if (m_mediaPlayer.getState() == McMediaPlayer::PlayingState)
		return;
	if (m_mediaPlayer.getState() == McMediaPlayer::PausedState)
		m_mediaPlayer.resume();
	else if (m_mediaPlayer.getState() == McMediaPlayer::StoppedState)
		m_mediaPlayer.play();
}

void QmlMediaPlayer::pause() noexcept {
	m_mediaPlayer.pause();
}

void QmlMediaPlayer::stop() noexcept {
	m_mediaPlayer.stop();
}

void QmlMediaPlayer::seek(qreal offset) noexcept {
	m_mediaPlayer.setPosition(m_mediaPlayer.getPosition() + offset);
}
