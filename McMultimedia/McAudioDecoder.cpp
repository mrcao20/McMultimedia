#include "McAudioDecoder.h"

extern "C" {
#include "libavformat/avformat.h"
#include "libswresample/swresample.h"
};

#include <qqueue.h>
#include <qmutex.h>
#include <qdebug.h>
#include <qaudioformat.h>
#include <qthread.h>

#include "McAVPacketMan.h"
#include "McAudioFrame.h"

#define AUDIO_BUFFER_SIZE 192000

struct McAudioDecoderData {
	AVStream *audioStream{ nullptr };					// 音频流，从外部传入
	AVCodecContext *codecContext{ nullptr };			// context
	AVFrame *frame{ nullptr };							// 音频帧，解码器初始化时初始化，用于解码音频包
	SwrContext *audioConvertCtx{ nullptr };				// 用于音频转换

	QQueue<AVPacket> audioPackets;						// 用于存储传入的音频包
	QMutex mtx;											// 音频包的同步锁
	QAudioFormat audioFormat;							// 音频格式
	AVSampleFormat sampleFormat{ AV_SAMPLE_FMT_FLT };	// 目标格式
	QSharedPointer<McAudioFrame> audioFrame;			// 音频帧，解码出的音频数据将放入该帧中

	DECLARE_ALIGNED(16, quint8, audioData)[AUDIO_BUFFER_SIZE];		// 用于临时存放被解码出的音频数据
	const char *flushStr{ nullptr };					// 刷新解码器所用字符串，由外部设置，当遇到这个字符串时刷新解码器
};

McAudioDecoder::McAudioDecoder(QObject *parent)
	: QObject(parent)
	, d(new McAudioDecoderData())
{
}

McAudioDecoder::~McAudioDecoder(){
	release();
}

void McAudioDecoder::setFlushStr(const char *str) noexcept {
	d->flushStr = str;
}

bool McAudioDecoder::init(AVStream *stream) noexcept {
	if (!stream) {
		return false;
	}

	release();	// 当初始化新的解码器时，释放上一个解码器资源
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

	// 设置音频格式
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
	if (packet->stream_index != d->audioStream->index && strcmp((char*)packet->data, d->flushStr) != 0) {
		return;
	}
	AVPacket tmpPck;
	av_packet_ref(&tmpPck, packet);	// 引用传入的资源包
	QMutexLocker locker(&d->mtx);
	d->audioPackets.enqueue(tmpPck);
}

int McAudioDecoder::getPacketNum() noexcept {
	QMutexLocker locker(&d->mtx);
	return d->audioPackets.size();
}

void McAudioDecoder::clearPacket() noexcept {
	QMutexLocker locker(&d->mtx);
	for (AVPacket &packet : d->audioPackets) {
		av_packet_unref(&packet);
	}
	d->audioPackets.clear();
}

void McAudioDecoder::setAudioFrame(const QSharedPointer<McAudioFrame> &frame) noexcept {
	d->audioFrame = frame;
}

void McAudioDecoder::getAudioData(const std::function<void()> &callback) noexcept {
	clearAudioFrame();	// 当需要读取新数据时，清空上一帧数据

	if (!d->audioStream) {	// 由于音频流必须存在，哪怕是无声视频也会存在音频流以供视频流同步，所以当不存在音频流时输出错误
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

		d->audioFrame->setStartClock(audioClock * 1000);	// s转ms

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

		d->audioFrame->setData(d->audioData);
		d->audioFrame->setSize(resampledDataSize);
		d->audioFrame->setEndClock(audioClock * 1000);	// s转ms
		callback();
	}
}

QAudioFormat McAudioDecoder::getAudioFormat() noexcept {
	return d->audioFormat;
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

void McAudioDecoder::release() noexcept {
	clearPacket();
	if (d->audioFrame) {
		QMutexLocker locker(&d->audioFrame->getMutex());
		clearAudioFrame();
	}
	if (d->audioConvertCtx) 
		swr_free(&d->audioConvertCtx);	// 自动置空
	if (d->frame)
		av_frame_free(&d->frame);	// 自动置空
	if (d->codecContext) {
		avcodec_close(d->codecContext);
		avcodec_free_context(&d->codecContext);	// 自动置空
	}
	d->audioStream = nullptr;
}

void McAudioDecoder::clearAudioFrame() noexcept {
	d->audioFrame->setData(nullptr);
	d->audioFrame->setSize(0);
}
