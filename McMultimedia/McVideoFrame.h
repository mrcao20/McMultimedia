#ifndef _MC_VIDEO_FRAME_H_
#define _MC_VIDEO_FRAME_H_

#include <qmutex.h>

// POCO，用于保存解码出的视频帧
class McVideoFrame {
public:
	quint8 *getData() { return m_data; };
	void setData(quint8 *data) { m_data = data; };

	int getWidth() { return m_width; };
	void setWidth(int width) { m_width = width; };

	int getHeight() { return m_height; };
	void setHeight(int height) { m_height = height; };

	int getLinesize() { return m_width; }
	void setLinesize(int linesize) { m_width = linesize; };

	qint64 getClock() { return m_clock; };
	void setClock(qint64 clock) { m_clock = clock; };

	QMutex &getMutex() { return m_mtx; }

private:
	quint8 *m_data{ nullptr };
	int m_width{ 0 };
	int m_height{ 0 };
	qint64 m_clock{ 0 };	// 单位ms
	QMutex m_mtx;			// 同步锁，同一时间只能有一个线程使用视频帧
};

#endif // !_MC_VIDEO_FRAME_H_

