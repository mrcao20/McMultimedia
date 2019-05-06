#ifndef _I_MC_AUDIO_H_
#define _I_MC_AUDIO_H_

#include "IMcAudioData.h"

#include <qaudioformat.h>

class IMcAudio : public IMcAudioData {
public:
	virtual ~IMcAudio() = default;

	/*************************************************
	 <函数名称>		getAudioFormat
	 <功    能>		获取音频格式。
	 <参数说明>
	 <返回值>		音频格式
	 <函数说明>		这个函数用来获取音频格式。
	 <作    者>		mrcao
	 <时    间>		2019/5/6
	**************************************************/
	virtual QAudioFormat getAudioFormat() noexcept = 0;
};

#endif // !_I_MC_AUDIO_H_