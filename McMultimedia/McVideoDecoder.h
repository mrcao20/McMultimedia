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

	void setFlushStr(const char *str) noexcept override;
	// 初始化解码器
	bool init(AVStream *stream) noexcept override;
	// 添加资源包
	void addPacket(AVPacket *packet) noexcept override;
	void setVideoFormat(McVideoFormat::PixelFormat format) noexcept override;
	void getVideoData(const QSharedPointer<McVideoFrame> &frame, const std::function<void()> &callback) noexcept override;

	void release() noexcept;

private:
	void clearPacket() noexcept;
	double getEndClock(AVFrame *frame, double startClock) noexcept;

private:
	QScopedPointer<McVideoDecoderData> d;
};

#endif // !_MC_VIDEO_DECODER_H_