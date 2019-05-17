#include "McVideoDecoder.h"

extern "C" {
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libavutil/imgutils.h"
};

#include <qthread.h>
#include <qdebug.h>
#include <qmutex.h>
#include <qqueue.h>

#include "McAVPacketMan.h"
#include "McVideoFrame.h"

struct McVideoDecoderData {
	AVStream *videoStream{ nullptr };			// 视频流
	AVCodecContext *codecContext{ nullptr };	// context
	AVFrame *frame{ nullptr };					// 原始视频帧
	AVFrame *frameDst{ nullptr };				// 转换后的目标视频帧
	quint8 *outBuffer{ nullptr };				// 指向目标视频帧的数据
	SwsContext *videoConvertCtx{ nullptr };		// 用于转换视频帧
	AVPixelFormat format{ AV_PIX_FMT_NONE };		// 视频解码的目标格式，从外部传入
	QSharedPointer<McVideoFrame> videoFrame;	// 视频帧，解码出的视频数据都将放入该帧中。

	QMutex mtx;									// 视频包的同步锁
	QQueue<AVPacket> videoPackets;				// 存放被读出的待解码的视频包

	const char *flushStr{ nullptr };			// 刷新解码器所用字符串，由外部设置，当遇到这个字符串时刷新解码器
};

McVideoDecoder::McVideoDecoder(QObject *parent)
	: QObject(parent)
	, d(new McVideoDecoderData())
{
}

McVideoDecoder::~McVideoDecoder(){
	release();
}

void McVideoDecoder::setFlushStr(const char *str) noexcept {
	d->flushStr = str;
}

bool McVideoDecoder::init(AVStream *stream) noexcept {
	if (!stream) {
		return false;
	}
	release();	// 释放上一个解码器的资源
	AVCodec *pCodec = avcodec_find_decoder(stream->codecpar->codec_id);
	if (pCodec == NULL) {
		qCritical() << "Video codec not found.";
		return false;
	}
	d->codecContext = avcodec_alloc_context3(pCodec);
	avcodec_parameters_to_context(d->codecContext, stream->codecpar);
	if (avcodec_open2(d->codecContext, pCodec, NULL) < 0) {
		qCritical() << "Could not open video codec.";
		return false;
	}
	d->videoStream = stream;

	d->frame = av_frame_alloc();

	return true;
}

void McVideoDecoder::addPacket(AVPacket *packet) noexcept {
	if (!packet || !d->videoStream) {
		return;
	}
	if (packet->stream_index != d->videoStream->index && strcmp((char*)packet->data, d->flushStr) != 0) {
		return;
	}

	AVPacket tmpPck;
	av_packet_ref(&tmpPck, packet);
	QMutexLocker locker(&d->mtx);
	d->videoPackets.enqueue(tmpPck);
}

int McVideoDecoder::getPacketNum() noexcept {
	QMutexLocker locker(&d->mtx);
	return d->videoPackets.size();
}

void McVideoDecoder::clearPacket() noexcept {
	QMutexLocker locker(&d->mtx);
	for (AVPacket &packet : d->videoPackets) {
		av_packet_unref(&packet);
	}
	d->videoPackets.clear();
}

void McVideoDecoder::setVideoFormat(McVideoFormat::PixelFormat format) noexcept {
	switch (format)
	{
	case McVideoFormat::Format_Invalid:
		d->format = AV_PIX_FMT_NONE;
		break;
	case McVideoFormat::Format_ARGB:
		d->format = AV_PIX_FMT_ARGB;
		break;
	case McVideoFormat::Format_BGRA:
		d->format = AV_PIX_FMT_BGRA;
		break;
	case McVideoFormat::Format_RGB32:
		d->format = AV_PIX_FMT_RGB32;
		break;
	case McVideoFormat::Format_YUV420P:
		d->format = AV_PIX_FMT_YUV420P;
		break;
	default:
		break;
	}
}

void McVideoDecoder::setVideoFrame(const QSharedPointer<McVideoFrame> &frame) noexcept {
	d->videoFrame = frame;
}

void McVideoDecoder::getVideoData(const std::function<void()> &callback) noexcept {
	if (!d->videoStream) {
		//qCritical() << "video stream not found, please make sure media started decode";
		QThread::msleep(10);
		return;
	}
	QMutexLocker locker(&d->mtx);
	if (d->videoPackets.isEmpty())
		return;
	McAVPacketMan packet(&d->videoPackets.dequeue());
	locker.unlock();

	//收到这个数据 说明刚刚执行过跳转 现在需要把解码器的数据 清除一下        
	if (strcmp((char*)packet->data, d->flushStr) == 0) {
		avcodec_flush_buffers(d->codecContext);
		return;
	}

	/* while return -11 means packet have data not resolved,
	 * this packet cannot be unref
	 */
	int ret = avcodec_send_packet(d->codecContext, packet.data());
	if (ret < 0) {
		qCritical() << "Video send to decoder failed, error code: " << ret;
		return;
	}

	double videoClock;
	while ((ret = avcodec_receive_frame(d->codecContext, d->frame)) == 0) {

		if ((videoClock = d->frame->pts) == AV_NOPTS_VALUE) {
			continue;
		}

		videoClock *= av_q2d(d->videoStream->time_base);	// 开始播放的时间，单位：s

		//videoClock = getEndClock(d->frame, videoClock);

		if (!initOutContext())
			continue;
		int dstHeight = sws_scale(d->videoConvertCtx, (const unsigned char* const*)d->frame->data, d->frame->linesize, 0, d->codecContext->height, d->frameDst->data, d->frameDst->linesize);
		
		QMutexLocker frameLocker(&d->videoFrame->getMutex());
		d->videoFrame->setData(d->outBuffer);
		d->videoFrame->setLinesize(d->frameDst->linesize[0]);
		d->videoFrame->setHeight(dstHeight);
		d->videoFrame->setClock(videoClock * 1000);		// s转ms
		frameLocker.unlock();
		callback();
	}
}

void McVideoDecoder::release() noexcept {
	clearPacket();
	if (d->videoFrame) {
		QMutexLocker frameLocker(&d->videoFrame->getMutex());
		d->videoFrame->setData(nullptr);
	}
	if (d->outBuffer)
		av_freep(&d->outBuffer);	// 自动置空
	if (d->frame)
		av_frame_free(&d->frame);	// 自动置空
	if (d->frameDst)
		av_frame_free(&d->frameDst);
	if (d->codecContext) {
		avcodec_close(d->codecContext);
		avcodec_free_context(&d->codecContext);	// 自动置空
	}
	if (d->videoConvertCtx) {
		sws_freeContext(d->videoConvertCtx);
		d->videoConvertCtx = nullptr;
	}
	d->videoStream = nullptr;
}

double McVideoDecoder::getEndClock(AVFrame *frame, double startClock) noexcept {
	double delay;
	delay = av_q2d(d->codecContext->time_base);
	delay += d->frame->repeat_pict * (delay * 0.5);
	startClock += delay;

	return startClock;
}

bool McVideoDecoder::initOutContext() noexcept {
	if (d->format == AV_PIX_FMT_NONE) {
		qDebug() << "video format not set.";
		return false;
	}
	if (d->videoConvertCtx)
		return true;
	d->frameDst = av_frame_alloc();
	// 申请一块内存，指向输出视频帧
	d->outBuffer = (unsigned char *)av_malloc(av_image_get_buffer_size(d->format, d->codecContext->width, d->codecContext->height, 1));
	av_image_fill_arrays(d->frameDst->data, d->frameDst->linesize, d->outBuffer,
		d->format, d->codecContext->width, d->codecContext->height, 1);

	// 初始化视频帧转换器
	d->videoConvertCtx = sws_getContext(d->codecContext->width, d->codecContext->height, d->codecContext->pix_fmt,
		d->codecContext->width, d->codecContext->height, d->format, SWS_BICUBIC, NULL, NULL, NULL);
	return true;
}
