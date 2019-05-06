#ifndef _I_MC_VIDEO_RENDERER_H_
#define _I_MC_VIDEO_RENDERER_H_

#include "McVideoFormat.h"

class McVideoFrame;

class IMcVideoRenderer {
public:
	virtual ~IMcVideoRenderer() = default;

	/*************************************************
	 <函数名称>		getVideoFormat
	 <功    能>		指定一个需要的视频格式
	 <参数说明>		
	 <返回值>		视频格式
	 <函数说明>		这个函数用来指定一个需要的视频格式
	 <作    者>		mrcao
	 <时    间>		2019/5/6
	**************************************************/
	virtual McVideoFormat::PixelFormat getVideoFormat() noexcept = 0;
	/*************************************************
	 <函数名称>		setVideoFrame
	 <功    能>		设置视频帧，用于之后的视频渲染
	 <参数说明>		frame 视频帧
	 <返回值>		
	 <函数说明>		这个函数用来设置视频帧
	 <作    者>		mrcao
	 <时    间>		2019/5/6
	**************************************************/
	virtual void setVideoFrame(const QSharedPointer<McVideoFrame> &frame) noexcept = 0;
	/*************************************************
	 <函数名称>		rendering
	 <功    能>		渲染视频
	 <参数说明>		
	 <返回值>
	 <函数说明>		这个函数用来渲染视频
	 <作    者>		mrcao
	 <时    间>		2019/5/6
	**************************************************/
	virtual void rendering() noexcept = 0;
};

#endif // !_I_MC_VIDEO_RENDERER_H_

