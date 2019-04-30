#ifndef _I_MC_VIDEO_RENDERER_H_
#define _I_MC_VIDEO_RENDERER_H_

#include "McVideoFormat.h"

class McVideoFrame;

class IMcVideoRenderer {
public:
	virtual ~IMcVideoRenderer() = default;

	virtual McVideoFormat::PixelFormat getVideoFormat() noexcept = 0;
	virtual void setVideoFrame(const QSharedPointer<McVideoFrame> &frame) noexcept = 0;
	virtual void rendering() noexcept = 0;
};

#endif // !_I_MC_VIDEO_RENDERER_H_

