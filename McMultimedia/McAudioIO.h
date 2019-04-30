#ifndef _MC_AUDIO_IO_H_
#define _MC_AUDIO_IO_H_

#include <QIODevice>

class IMcAudioData;
class McAudioFrame;

struct McAudioIOData;

class McAudioIO : public QIODevice {
	Q_OBJECT

public:
	explicit McAudioIO(IMcAudioData *audio, QObject *parent = 0);
	~McAudioIO();

signals:
	void signal_clockChanged(qint64 startClock, qint64 endClock);

protected:
	qint64 readData(char * data, qint64 maxSize) override;
	qint64 writeData(const char * data, qint64 maxSize) override;

private:
	qint64 mixAudio(char *buffer, QSharedPointer<McAudioFrame> &frame) noexcept;

private:
	QScopedPointer<McAudioIOData> d;
};

#endif // !_MC_AUDIO_IO_H_