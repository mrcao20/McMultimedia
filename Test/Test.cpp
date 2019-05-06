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
		QString path = qApp->applicationDirPath() + "/";
#ifdef _DEBUG
		path += "../Release/";
#endif // _DEBUG

		path += "a.mp4";
		path = R"(D:\temp\Resource\操作视频\刺激频率与骨骼肌收缩的关系.flv)";
		//path = R"(D:\SVN_Repository\TAS-100原始版本\TAS-100\bin\Debug\Resource原始数据\肌肉神经实验\刺激频率与反应的关系\3\05录像.flv)";
		path = R"(D:\temp\Resource\操作视频\蟾蜍在体心肌动作电位与心电图记录.wmv)";
		//path = R"(D:\temp\hf6PkPOD8-jg34zxxvKPlf-j+seG8Izu-ocdWQZDCQ==\hf6PkPOD8-jg34zxxvKPlf-j+seG8Izu-ocdWQZDCQ==.m3u8)";
		path = "http://127.0.0.1:8099/static/temp/index.m3u8";
		//path = qApp->applicationDirPath() +  R"(\temp\index.m3u8)";
		//path = R"(http://localhost:8099/static/刺激强度与反应的关系.mp4)";
		//path = R"(D:\temp\222.mp4)";
		//path = R"(D:\Downloads\走马+-+曲肖冰.mp3)";
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
