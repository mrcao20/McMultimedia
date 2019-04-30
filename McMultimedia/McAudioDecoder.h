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

	void setFlushStr(const char *str) noexcept override;
	// 初始化解码器
	bool init(AVStream *stream) noexcept override;
	// 添加资源包
	void addPacket(AVPacket *packet) noexcept override;
	// 解码音频包
	void getAudioData(const QSharedPointer<McAudioFrame> &frame, const std::function<void()> &callback) noexcept override;
	QAudioFormat getAudioFormat() noexcept override;

	void release() noexcept;

private:
	void clearPacket() noexcept;
	bool init_Swr() noexcept;

private:
	QScopedPointer<McAudioDecoderData> d;
};

#endif // !_MC_AUDIO_DECODER_H_