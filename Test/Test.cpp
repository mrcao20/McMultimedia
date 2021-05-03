#include "Test.h"

#include <qevent.h>
#include <qdebug.h>

#include "McMediaPlayer.h"

Test::Test(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	 
	m_mediaPlayer = new McMediaPlayer(this);
	m_mediaPlayer->setVideoRenderer(ui.videoOutput);

	connect(m_mediaPlayer, &McMediaPlayer::signal_durationChanged, [this](qint64 duration) {
		ui.progressBar->setRange(0, duration);
	});
	connect(m_mediaPlayer, &McMediaPlayer::signal_positionChanged, [this](qint64 pos) {
		if (qAbs(pos - ui.progressBar->value()) < 1000) {
			return;
		}
		ui.progressBar->setValue(pos);
	});
	connect(ui.progressBar, &QSlider::valueChanged, [this](int pos) {
		if (qAbs(pos - m_mediaPlayer->getPosition()) < 1000) {
			return;
		}
		m_mediaPlayer->setPosition(pos);
	});

	connect(ui.stopBtn, &QPushButton::clicked, [this]() {
		m_mediaPlayer->stop();
	});

	connect(ui.backwardBtn, &QPushButton::clicked, [this]() {
		m_mediaPlayer->setPosition(m_mediaPlayer->getPosition() - 3000);
	});

	connect(ui.playBtn, &QPushButton::clicked, [this]() {
		if (m_mediaPlayer->getState() == McMediaPlayer::StoppedState) {
			m_mediaPlayer->play();
		}
		else if (m_mediaPlayer->getState() == McMediaPlayer::PlayingState) {
			m_mediaPlayer->pause();
		}
		else if (m_mediaPlayer->getState() == McMediaPlayer::PausedState) {
			m_mediaPlayer->resume();
		}
	});

	connect(ui.forwardBtn, &QPushButton::clicked, [this]() {
		m_mediaPlayer->setPosition(m_mediaPlayer->getPosition() + 1000);
	});

	connect(ui.pushButton, &QPushButton::clicked, [this]() {
		QString path;
		path = R"(F:\Music\All About That Bass.mp3)";
		m_mediaPlayer->setMediaUrl(path);
		m_mediaPlayer->play();
	});
	
	ui.progressBar->installEventFilter(this);
}

bool Test::eventFilter(QObject *obj, QEvent *event)
{
	if (obj == ui.progressBar) {
		if (event->type() == QEvent::MouseButtonPress) {
			QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
			if (mouseEvent->button() == Qt::LeftButton) {
				int duration = ui.progressBar->maximum() - ui.progressBar->minimum();
				int pos = ui.progressBar->minimum() + duration * (static_cast<double>(mouseEvent->x()) / ui.progressBar->width());
				if (pos != ui.progressBar->sliderPosition()) {
					ui.progressBar->setValue(pos);
					//m_mediaPlayer->setPosition(static_cast<qint64>(pos));
					//return true;
				}
			}
		}
	}
	return QObject::eventFilter(obj, event);
}
