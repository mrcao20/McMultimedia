#ifndef _I_MC_AUDIO_DATA_H_
#define _I_MC_AUDIO_DATA_H_

#include <functional>
#include <qsharedpointer.h>

class McAudioFrame;

class IMcAudioData {
public:
	virtual ~IMcAudioData() = default;

	/*************************************************
	 <函数名称>		setAudioFrame
	 <功    能>		设置音频帧，解码出的音频数据都将放入该帧中。
	 <参数说明>		frame 音频帧，解码出的音频将放入该音频帧中
	 <返回值>
	 <函数说明>		这个函数用来设置音频帧。
	 <作    者>		mrcao
	 <时    间>		2019/5/7
	**************************************************/
	virtual void setAudioFrame(const QSharedPointer<McAudioFrame> &frame) noexcept = 0;
	/*************************************************
	 <函数名称>		getAudioData
	 <功    能>		解码音频。
	 <参数说明>		callback 回调函数，如果解码成功，则调用此函数，否则不调用
	 <返回值>		
	 <函数说明>		这个函数用来解码音频。
	 <作    者>		mrcao
	 <时    间>		2019/5/6
	**************************************************/
	virtual void getAudioData(const std::function<void()> &callback) noexcept = 0;
};

#endif // !_I_MC_AUDIO_DATA_H_