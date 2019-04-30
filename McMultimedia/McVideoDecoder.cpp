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
	AVStream *videoStream{ nullptr };			// ��Ƶ��
	AVCodecContext *codecContext{ nullptr };	// context
	AVFrame *frame{ nullptr };					// ԭʼ��Ƶ֡
	AVFrame *frameDst{ nullptr };				// ת�����Ŀ����Ƶ֡
	quint8 *outBuffer{ nullptr };				// ָ��Ŀ����Ƶ֡������
	SwsContext *videoConvertCtx{ nullptr };		// ����ת����Ƶ֡
	AVPixelFormat format{ AV_PIX_FMT_NONE };		// ��Ƶ�����Ŀ���ʽ�����ⲿ����

	QMutex mtx;									// ��Ƶ����ͬ����
	QQueue<AVPacket> videoPackets;				// ��ű������Ĵ��������Ƶ��

	const char *flushStr{ nullptr };
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
	if (d->format == AV_PIX_FMT_NONE) {
		qCritical() << "video format not set.";
		return false;
	}
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
	d->frameDst = av_frame_alloc();

	d->outBuffer = (unsigned char *)av_malloc(av_image_get_buffer_size(d->format, d->codecContext->width, d->codecContext->height, 1));
	av_image_fill_arrays(d->frameDst->data, d->frameDst->linesize, d->outBuffer,
		d->format, d->codecContext->width, d->codecContext->height, 1);

	d->videoConvertCtx = sws_getContext(d->codecContext->width, d->codecContext->height, d->codecContext->pix_fmt,
		d->codecContext->width, d->codecContext->height, d->format, SWS_BICUBIC, NULL, NULL, NULL);

	return true;
}

void McVideoDecoder::addPacket(AVPacket *packet) noexcept {
	if (!packet || !d->videoStream) {
		return;
	}
	if (packet->stream_index != d->videoStream->index) {
		return;
	}

	AVPacket tmpPck;
	av_packet_ref(&tmpPck, packet);
	QMutexLocker locker(&d->mtx);
	d->videoPackets.enqueue(tmpPck);
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

void McVideoDecoder::getVideoData(const QSharedPointer<McVideoFrame> &frame, const std::function<void()> &callback) noexcept {
	if (!d->videoStream) {
		qCritical() << "video stream not found, please make sure media started decode";
		return;
	}
	QMutexLocker locker(&d->mtx);
	if (d->videoPackets.isEmpty())
		return;
	McAVPacketMan packet(&d->videoPackets.dequeue());
	locker.unlock();

	//�յ�������� ˵���ո�ִ�й���ת ������Ҫ�ѽ����������� ���һ��        
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

		videoClock *= av_q2d(d->videoStream->time_base);	// ��ʼ���ŵ�ʱ�䣬��λ��s

		videoClock = getEndClock(d->frame, videoClock);

		int dstHeight = sws_scale(d->videoConvertCtx, (const unsigned char* const*)d->frame->data, d->frame->linesize, 0, d->codecContext->height, d->frameDst->data, d->frameDst->linesize);
		
		frame->setData(d->outBuffer);
		frame->setLinesize(d->frameDst->linesize[0]);
		frame->setHeight(dstHeight);
		frame->setClock(videoClock * 1000);		// sתms
		callback();
	}
}

void McVideoDecoder::release() noexcept {
	if (d->outBuffer)
		av_freep(&d->outBuffer);	// �Զ��ÿ�
	if (d->frame)
		av_frame_free(&d->frame);	// �Զ��ÿ�
	if (d->frameDst)
		av_frame_free(&d->frameDst);
	if (d->codecContext) {
		avcodec_close(d->codecContext);
		avcodec_free_context(&d->codecContext);	// �Զ��ÿ�
	}
	if (d->videoConvertCtx) {
		sws_freeContext(d->videoConvertCtx);
		d->videoConvertCtx = nullptr;
	}
	d->videoStream = nullptr;
	clearPacket();
}

void McVideoDecoder::clearPacket() noexcept {
	QMutexLocker locker(&d->mtx);
	for (AVPacket &packet : d->videoPackets) {
		av_packet_unref(&packet);
	}
	d->videoPackets.clear();
}

double McVideoDecoder::getEndClock(AVFrame *frame, double startClock) noexcept {
	double delay;
	delay = av_q2d(d->codecContext->time_base);
	delay += d->frame->repeat_pict * (delay * 0.5);
	startClock += delay;

	return startClock;
}