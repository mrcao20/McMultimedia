#ifndef _MC_AUDIO_FRAME_H_
#define _MC_AUDIO_FRAME_H_

#include "qglobal.h"

// POCO，用于保存解码出的音频帧
class McAudioFrame {
public:
	quint8 *getData() { return m_data; };
	void setData(quint8 *data) { m_data = data; };

	int getSize() { return m_size; };
	void setSize(int size) { m_size = size; };

	qint64 getStartClock() { return m_startClock; };
	void setStartClock(qint64 clock) { m_startClock = clock; };

	qint64 getEndClock() { return m_endClock; };
	void setEndClock(qint64 clock) { m_endClock = clock; };

private:
	quint8 *m_data{ nullptr };
	qint64 m_size{ 0 };
	qint64 m_startClock{ 0 };	// 开始时间，单位ms
	qint64 m_endClock{ 0 };		// 结束时间，单位ms
};

#endif // !_MC_AUDIO_FRAME_H_

