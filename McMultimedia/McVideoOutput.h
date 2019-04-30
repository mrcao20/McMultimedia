#ifndef _MC_VIDEO_OUTPUT_H_
#define _MC_VIDEO_OUTPUT_H_

#include <QObject>

class IMcVideo;
class IMcMediaClock;
class IMcVideoRenderer;

struct McVideoOutputData;

class McVideoOutput : public QObject {
	Q_OBJECT

public:
	explicit McVideoOutput(QObject *parent = 0);
	~McVideoOutput();

	void setVideo(IMcVideo *video) noexcept;

	void setMediaClock(IMcMediaClock *mediaClock) noexcept;

	void setRenderer(IMcVideoRenderer *renderer) noexcept;

	void start() noexcept;

private:
	bool mediaSync(qint64 clock) noexcept;

private:
	QScopedPointer<McVideoOutputData> d;
};

#endif // !_MC_VIDEO_OUTPUT_H_