#ifndef _MC_AUDIO_IO_H_
#define _MC_AUDIO_IO_H_

#include <QIODevice>

class IMcAudioData;
class McAudioFrame;

struct McAudioIOData;

class McAudioIO : public QIODevice {
	Q_OBJECT

public:
	explicit McAudioIO(IMcAudioData *audio, QObject *parent = 0);
	~McAudioIO();

	void seekTo(qint64 pos) noexcept;

signals:
	/*************************************************
	 <函数名称>		signal_clockChanged
	 <功    能>		信号，发送出当前音频的关键信息
	 <参数说明>		startClock 开始时间，当前整个音频帧的开始时间，如果需要获取真实时间，需要加上startIndex对应的时间 单位 ms
					startIndex 写入音频设备的当前音频帧的起始位置，也表示之前写入的数据大小
					endClock 当前整个音频帧的结束时间，如果需要获取真实时间，需要减去remainDataSize对应的时间 单位 ms
					remainDataSize 当前帧还剩余未写入的音频数据
	 <返回值>
	 <函数说明>		这个函数用来发送出当前音频的关键信息
	 <作    者>		mrcao
	 <时    间>		2019/5/6
	**************************************************/
	void signal_clockChanged(qint64 startClock, qint64 startIndex, qint64 endClock, qint64 remainDataSize);

protected:
	qint64 readData(char * data, qint64 maxSize) override;
	qint64 writeData(const char * data, qint64 maxSize) override;

private:
	/*************************************************
	 <函数名称>		mixAudio
	 <功    能>		写入音频数据
	 <参数说明>		buffer 音频缓冲区
					bufferIndex 表示需要从音频缓冲区的这个位置开始写入，也表示之前已经写入的大小
					frame 音频帧
					dataIndex 表示从音频帧的当前位置写入，也表示当前音频帧已经写入的大小
					maxSize 音频缓冲区最大可写入大小
	 <返回值>
	 <函数说明>		这个函数用来写入音频数据
	 <作    者>		mrcao
	 <时    间>		2019/5/6
	**************************************************/
	qint64 mixAudio(char *buffer, qint64 bufferIndex, const QSharedPointer<McAudioFrame> &frame, qint64 dataIndex, qint64 maxSize) noexcept;

private:
	QScopedPointer<McAudioIOData> d;
};

#endif // !_MC_AUDIO_IO_H_