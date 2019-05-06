#ifndef _MC_AUDIO_OUTPUT_H_
#define _MC_AUDIO_OUTPUT_H_

#include <QObject>
#include "IMcMediaClock.h"
#include "IMcControl.h"

struct McAudioOutputData;

class IMcAudio;

class McAudioOutput 
	: public QObject 
	, public IMcMediaClock
	, public IMcControl
{
	Q_OBJECT

public:
	explicit McAudioOutput(QObject *parent = 0);
	virtual ~McAudioOutput();

	// 获取当前音频的时间
	qint64 getMediaClock() noexcept override;

	// 设置音频源
	void setAudio(IMcAudio *audio) noexcept;

	bool start() noexcept override;
	void pause() noexcept override;
	void resume() noexcept override;
	void stop() noexcept override;
	void quit() noexcept override;

	// 单位 ms
	void seek(qint64 pos) noexcept override;

signals:
	void signal_clockChanged(qint64 clock);

private:
	void release() noexcept;
	// 单位 s
	double getPlayTime(int bufSize) noexcept;

private:
	QScopedPointer<McAudioOutputData> d;
};

#endif // !_MC_AUDIO_OUTPUT_H_