#ifndef _I_MC_MEDIA_DECODER_H_
#define _I_MC_MEDIA_DECODER_H_

#include <qstring.h>

class IMcMediaDecoder {
public:
	virtual ~IMcMediaDecoder() = default;

	/*************************************************
	 <函数名称>		setMediaUrl
	 <功    能>		设置媒体路径
	 <参数说明>		url	媒体路径，可以是本地路径，也可以是网络路径
	 <返回值>		
	 <函数说明>		这个函数用来设置媒体路径
	 <作    者>		mrcao
	 <时    间>		2019/5/6
	**************************************************/
	virtual void setMediaUrl(const QString &url) noexcept = 0;
};

#endif // !_I_MC_MEDIA_DECODER_H_

