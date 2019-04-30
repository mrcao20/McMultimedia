#ifndef _I_MC_VIDEO_H_
#define _I_MC_VIDEO_H_

#include <functional>
#include <qsharedpointer.h>
#include "McVideoFormat.h"

class McVideoFrame;

class IMcVideo {
public:
	virtual ~IMcVideo() = default;

	virtual void setVideoFormat(McVideoFormat::PixelFormat format) noexcept = 0;
	virtual void getVideoData(const QSharedPointer<McVideoFrame> &frame, const std::function<void()> &callback) noexcept = 0;
};

#endif // !_I_MC_VIDEO_H_

