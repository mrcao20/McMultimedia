#ifndef _MC_AUDIO_DECODER_H_
#define _MC_AUDIO_DECODER_H_

#include <QObject>
#include "IMcDecoder.h"
#include "IMcAudio.h"

struct McAudioDecoderData;

class McAudioDecoder 
	: public QObject
	, public IMcDecoder
	, public IMcAudio
{
	Q_OBJECT

public:
	explicit McAudioDecoder(QObject *parent = 0);
	virtual ~McAudioDecoder();

	// 设置刷新字符串
	void setFlushStr(const char *str) noexcept override;
	// 初始化解码器
	bool init(AVStream *stream) noexcept override;
	// 添加资源包
	void addPacket(AVPacket *packet) noexcept override;
	// 获取剩余包数量
	int getPacketNum() noexcept override;
	// 清除所有资源包
	void clearPacket() noexcept override;
	// 设置音频帧，解码出的音频数据都将放入该帧中。
	void setAudioFrame(const QSharedPointer<McAudioFrame> &frame) noexcept override;
	// 解码音频包
	void getAudioData(const std::function<void()> &callback) noexcept override;
	// 获取音频格式
	QAudioFormat getAudioFormat() noexcept override;

private:
	// 初始化音频转换器
	bool init_Swr() noexcept;
	// 释放资源
	void release() noexcept;
	// 清空音频帧
	void clearAudioFrame() noexcept;

private:
	QScopedPointer<McAudioDecoderData> d;
};

#endif // !_MC_AUDIO_DECODER_H_