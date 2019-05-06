#ifndef _MC_VIDEO_DECODER_H_
#define _MC_VIDEO_DECODER_H_

#include <QObject>
#include "IMcDecoder.h"
#include "IMcVideo.h"

struct AVStream;
struct AVPacket;
struct AVFrame;

struct McVideoDecoderData;

class McVideoDecoder 
	: public QObject
	, public IMcDecoder
	, public IMcVideo
{
	Q_OBJECT

public:
	explicit McVideoDecoder(QObject *parent = 0);
	virtual ~McVideoDecoder();

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

	// 设置解码出的视频帧格式
	void setVideoFormat(McVideoFormat::PixelFormat format) noexcept override;
	// 解码视频包
	void getVideoData(const QSharedPointer<McVideoFrame> &frame, const std::function<void()> &callback) noexcept override;

private:
	// 释放资源
	void release() noexcept;
	double getEndClock(AVFrame *frame, double startClock) noexcept;

private:
	QScopedPointer<McVideoDecoderData> d;
};

#endif // !_MC_VIDEO_DECODER_H_