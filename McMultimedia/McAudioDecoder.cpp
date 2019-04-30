#include "McAudioDecoder.h"

extern "C" {
#include "libavformat/avformat.h"
#include "libswresample/swresample.h"
};

#include <qqueue.h>
#include <qmutex.h>
#include <qdebug.h>
#include <qaudioformat.h>

#include "McAVPacketMan.h"
#include "McAudioFrame.h"

struct McAudioDecoderData {
	AVStream *audioStream{ nullptr };					// 音频流，从外部传入
	AVCodecContext *codecContext{ nullptr };			// context
	AVFrame *frame{ nullptr };							// 音频帧，解码器初始化时初始化，用于解码音频包
	SwrContext *audioConvertCtx{ nullptr };				// 用于音频转换

	QQueue<AVPacket> audioPackets;						// 用于存储传入的音频包
	QMutex mtx;											// 音频包的同步锁
	QAudioFormat audioFormat;							// 音频格式
	AVSampleFormat sampleFormat{ AV_SAMPLE_FMT_FLT };	// 目标格式

	DECLARE_ALIGNED(16, quint8, audioData)[192000];		// 用于临时存放被解码出的音频数据
	const char *flushStr{ nullptr };
};

McAudioDecoder::McAudioDecoder(QObject *parent)
	: QObject(parent)
	, d(new McAudioDecoderData())
{
}

McAudioDecoder::~McAudioDecoder(){
}

void McAudioDecoder::setFlushStr(const char *str) noexcept {
	d->flushStr = str;
}

bool McAudioDecoder::init(AVStream *stream) noexcept {
	if (!stream) {
		return false;
	}

	AVCodec *pCodec = avcodec_find_decoder(stream->codecpar->codec_id);
	if (pCodec == NULL) {
		qCritical() << "Audio codec not found.";
		return false;
	}
	d->codecContext = avcodec_alloc_context3(pCodec);
	avcodec_parameters_to_context(d->codecContext, stream->codecpar);
	if (avcodec_open2(d->codecContext, pCodec, NULL) < 0) {
		qCritical() << "Could not open Audio codec.";
		return false;
	}

	d->audioFormat.setChannelCount(d->codecContext->channels);
	d->audioFormat.setSampleRate(d->codecContext->sample_rate);
	d->audioFormat.setSampleSize(8 * av_get_bytes_per_sample(d->sampleFormat));
	d->audioFormat.setSampleType(QAudioFormat::Float);
	d->audioFormat.setCodec("audio/pcm");

	d->frame = av_frame_alloc();

	d->audioStream = stream;

	return init_Swr();
}

void McAudioDecoder::addPacket(AVPacket *packet) noexcept {
	if (!packet || !d->audioStream) {
		return;
	}
	if (packet->stream_index != d->audioStream->index) {
		return;
	}
	AVPacket tmpPck;
	av_packet_ref(&tmpPck, packet);
	QMutexLocker locker(&d->mtx);
	d->audioPackets.enqueue(tmpPck);
}

void McAudioDecoder::getAudioData(const QSharedPointer<McAudioFrame> &frame, const std::function<void()> &callback) noexcept {
	if (!d->audioStream) {
		qCritical() << "audio stream not found, please make sure media started decode";
		return;
	}

	QMutexLocker locker(&d->mtx);
	if (d->audioPackets.isEmpty())
		return;
	McAVPacketMan packet(&d->audioPackets.dequeue());
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
		qCritical() << "Audio send to decoder failed, error code: " << ret;
		return;
	}

	int resampledDataSize;	// 解码出的音频大小
	double audioClock{ 0.0 };	// 当前音频时间戳
	while ((ret = avcodec_receive_frame(d->codecContext, d->frame)) == 0) {
		if (d->frame->pts == AV_NOPTS_VALUE) {
			continue;
		}

		audioClock = av_q2d(d->audioStream->time_base) * d->frame->pts;	// 开始播放的时间

		frame->setStartClock(audioClock * 1000);	// s转ms

		const quint8 **in = (const quint8 **)d->frame->extended_data;
		uint8_t *out[] = { d->audioData };

		int outCount = sizeof(d->audioData) / d->codecContext->channels / av_get_bytes_per_sample(d->sampleFormat);

		int sampleSize = swr_convert(d->audioConvertCtx, out, outCount, in, d->frame->nb_samples);
		if (sampleSize < 0) {
			qCritical() << "swr convert failed";
			continue;
		}

		if (sampleSize == outCount)
			qCritical() << "audio buffer is probably too small";

		resampledDataSize = sampleSize * d->codecContext->channels * av_get_bytes_per_sample(d->sampleFormat);

		// 播放结束的时间
		audioClock += static_cast<double>(resampledDataSize) /
			(av_get_bytes_per_sample(d->sampleFormat) * d->codecContext->channels * d->codecContext->sample_rate);

		frame->setData(d->audioData);
		frame->setSize(resampledDataSize);
		frame->setEndClock(audioClock * 1000);	// s转ms
		callback();
	}
}

QAudioFormat McAudioDecoder::getAudioFormat() noexcept {
	return d->audioFormat;
}

void McAudioDecoder::release() noexcept {
	if (d->audioConvertCtx) 
		swr_free(&d->audioConvertCtx);	// 自动置空
	if (d->frame)
		av_frame_free(&d->frame);	// 自动置空
	if (d->codecContext) {
		avcodec_close(d->codecContext);
		avcodec_free_context(&d->codecContext);	// 自动置空
	}
	d->audioStream = nullptr;
	clearPacket();
}

void McAudioDecoder::clearPacket() noexcept {
	QMutexLocker locker(&d->mtx);
	for (AVPacket &packet : d->audioPackets) {
		av_packet_unref(&packet);
	}
	d->audioPackets.clear();
}

bool McAudioDecoder::init_Swr() noexcept {
	/* get audio channels */
	qint64 inChannelLayout = (d->audioStream->codecpar->channel_layout && d->audioStream->codecpar->channels == av_get_channel_layout_nb_channels(d->audioStream->codecpar->channel_layout)) ?
		d->audioStream->codecpar->channel_layout : av_get_default_channel_layout(d->audioStream->codecpar->channels);

	/* init swr audio convert context */
	qint64 channelLayout = av_get_default_channel_layout(d->codecContext->channels);
	channelLayout &= ~AV_CH_LAYOUT_STEREO_DOWNMIX;
	/*	此函数的第一个参数如果为nullptr，则会分配内存并将其通过返回值返回，
		如果不为nullptr，则不会重新分配内存，将直接在原指针上设置参数
	*/
	d->audioConvertCtx = swr_alloc_set_opts(d->audioConvertCtx, channelLayout, d->sampleFormat
		, d->audioStream->codecpar->sample_rate,
		inChannelLayout, (AVSampleFormat)d->audioStream->codecpar->format, d->audioStream->codecpar->sample_rate, 0, NULL);
	if (!d->audioConvertCtx || (swr_init(d->audioConvertCtx) < 0)) {
		qCritical() << "cannot init swr!!";
		return false;
	}

	return true;
}
