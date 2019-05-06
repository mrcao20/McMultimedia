/*******************************************************************
 <文件名>		McAVPacketMan.h
 <概要说明>		使用RAII管理AVPacket
 <详细说明>		本文件主要使用RAII原则来管理AVPacket的释放
 <作   者>		mrcao
 <日   期>		2019/5/16
********************************************************************/


#ifndef _MC_AV_PACKET_MAN_H_
#define _MC_AV_PACKET_MAN_H_

extern "C" {
#include "libavcodec/avcodec.h"
}

#include <qglobal.h>

class McAVPacketMan {
public:
	explicit inline McAVPacketMan(AVPacket *p)
		: d(p)
	{}
	inline ~McAVPacketMan() {
		if (d)
			av_packet_unref(d);
	}

	inline AVPacket *data() {
		return d;
	}

	inline AVPacket &operator*() const {
		Q_ASSERT(d);
		return *d;
	}

	inline AVPacket *operator->() const {
		return d;
	}

private:
	AVPacket *d{ nullptr };

private:
	Q_DISABLE_COPY(McAVPacketMan)
};

#endif // !_MC_AV_PACKET_MAN_H_

