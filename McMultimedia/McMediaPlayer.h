#ifndef _MC_MEDIA_PLAYER_H_
#define _MC_MEDIA_PLAYER_H_

#include <QObject>

#include "mcmultimedia_global.h"

class IMcVideoRenderer;

struct McMediaPlayerData;

class MCMULTIMEDIA_EXPORT McMediaPlayer : public QObject {
	Q_OBJECT

public:
	explicit McMediaPlayer(QObject *parent = 0);
	~McMediaPlayer();

	// 如果是m3u8格式视频，一定要包含后缀.m3u8
	void setMediaUrl(const QString &url);

	void play();

	qint64 getPosition();
	void setPosition(qint64 position);

	void setVideoRenderer(IMcVideoRenderer *renderer) noexcept;

signals:
	void signal_durationChanged(qint64 duration);
	void signal_positionChanged(qint64 pos);

private:
	QScopedPointer<McMediaPlayerData> d;
};

#endif // !_MC_MEDIA_PLAYER_H_