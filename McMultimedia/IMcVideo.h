#ifndef _I_MC_VIDEO_H_
#define _I_MC_VIDEO_H_

#include <functional>
#include <qsharedpointer.h>
#include "McVideoFormat.h"

class McVideoFrame;

class IMcVideo {
public:
	virtual ~IMcVideo() = default;

	/*************************************************
	 <函数名称>		setVideoFormat
	 <功    能>		设置视频格式，该格式将决定解码出的视频帧的格式
	 <参数说明>		format	视频格式
	 <返回值>
	 <函数说明>		这个函数用来设置视频格式
	 <作    者>		mrcao
	 <时    间>		2019/5/6
	**************************************************/
	virtual void setVideoFormat(McVideoFormat::PixelFormat format) noexcept = 0;
	/*************************************************
	 <函数名称>		setVideoFrame
	 <功    能>		设置视频帧，解码出的视频数据都将放入该帧中。
	 <参数说明>		frame 视频帧，解码出的视频将放入该视频帧中
	 <返回值>
	 <函数说明>		这个函数用来设置视频帧
	 <作    者>		mrcao
	 <时    间>		2019/5/7
	**************************************************/
	virtual void setVideoFrame(const QSharedPointer<McVideoFrame> &frame) noexcept = 0;
	/*************************************************
	 <函数名称>		getVideoData
	 <功    能>		解码视频
	 <参数说明>		callback 回调函数，如果解码成功，则调用此函数，否则不调用
	 <返回值>
	 <函数说明>		这个函数用来解码视频
	 <作    者>		mrcao
	 <时    间>		2019/5/6
	**************************************************/
	virtual void getVideoData(const std::function<void()> &callback) noexcept = 0;
};

#endif // !_I_MC_VIDEO_H_

