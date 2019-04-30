#ifndef _MC_AUDIO_OUTPUT_H_
#define _MC_AUDIO_OUTPUT_H_

#include <QObject>
#include "IMcMediaClock.h"

struct McAudioOutputData;

class IMcAudio;

class McAudioOutput 
	: public QObject 
	, public IMcMediaClock
{
	Q_OBJECT

public:
	explicit McAudioOutput(QObject *parent = 0);
	virtual ~McAudioOutput();

	qint64 getMediaClock() noexcept override;

	void setAudio(IMcAudio *audio) noexcept;

	void start() noexcept;

signals:
	void signal_clockChanged(qint64 clock);

private:
	QScopedPointer<McAudioOutputData> d;
};

#endif // !_MC_AUDIO_OUTPUT_H_