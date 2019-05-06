#ifndef _I_MC_MEDIA_CLOCK_H_
#define _I_MC_MEDIA_CLOCK_H_

#include <qglobal.h>

class IMcMediaClock {
public:
	virtual ~IMcMediaClock() = default;

	/*************************************************
	 <函数名称>		getMediaClock
	 <功    能>		获取同步时间戳，该时间戳用来同步其他媒体，比如使用音频来同步视频
	 <参数说明>
	 <返回值>		当前时间戳
	 <函数说明>		这个函数用来获取同步时间戳
	 <作    者>		mrcao
	 <时    间>		2019/5/6
	**************************************************/
	virtual qint64 getMediaClock() noexcept = 0;
};

#endif // !_I_MC_MEDIA_CLOCK_H_

