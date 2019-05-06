#ifndef _MC_VIDEO_OUTPUT_H_
#define _MC_VIDEO_OUTPUT_H_

#include <QObject>
#include "IMcControl.h"
#include "IMcVideoOutput.h"

class IMcVideo;
class IMcMediaClock;

struct McVideoOutputData;

class McVideoOutput : public QObject, public IMcControl, public IMcVideoOutput {
	Q_OBJECT

public:
	explicit McVideoOutput(QObject *parent = 0);
	~McVideoOutput();

	// 设置视频源
	void setVideo(IMcVideo *video) noexcept;

	// 设置同步对象，视频将会根据该时间戳同步
	void setMediaClock(IMcMediaClock *mediaClock) noexcept;

	// 设置视频渲染器
	void setRenderer(IMcVideoRenderer *renderer) noexcept override;

	bool start() noexcept override;
	void pause() noexcept override;
	void resume() noexcept override;
	void stop() noexcept override;
	void quit() noexcept override;

	// 单位 ms
	void seek(qint64 pos) noexcept override;

private:
	void release() noexcept;
	void startDetach() noexcept;
	// 单位 ms
	bool mediaSync(qint64 clock) noexcept;

private:
	QScopedPointer<McVideoOutputData> d;
};

#endif // !_MC_VIDEO_OUTPUT_H_