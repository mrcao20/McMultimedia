#include "McMediaDecoder.h"

extern "C" {
#include "libavformat/avformat.h"
};

#include <QtConcurrent>
#include <qdebug.h>

#include "IMcDecoder.h"

#define FLUSH "FLUSH"

struct McMediaDecoderData {
	AVFormatContext *formatCtx{ nullptr };
	AVPacket seekPacket;

	QMap<AVMediaType, IMcDecoder *> decoders;		// 用来存放解码器，解码器由外部注入，注入时必须指定解码器类型

	QString mediaUrl;
	int initSuccessNum{ 0 };						// 解码器初始化成功的数量
	bool isQuit{ false };						// 是否停止解码
	bool isPause{ false };						// 是否暂停解码
};

McMediaDecoder::McMediaDecoder(QObject *parent)
	: QObject(parent)
	, d(new McMediaDecoderData())
{
	av_init_packet(&d->seekPacket);
	d->seekPacket.data = (uint8_t *)FLUSH;
}

McMediaDecoder::~McMediaDecoder(){
	av_packet_unref(&d->seekPacket);
}

void McMediaDecoder::setMediaUrl(const QString &url) noexcept {
	d->mediaUrl = url;
}

void McMediaDecoder::start() noexcept {
	if (!initDecoder(d->mediaUrl.toLocal8Bit().data()))
		return;
	// 后台解码
	QtConcurrent::run(this, &McMediaDecoder::decodeDetached);
}

void McMediaDecoder::stop() noexcept {
	d->isQuit = true;
}

void McMediaDecoder::addDecoder(DecoderType type, IMcDecoder *decoder) noexcept {
	decoder->setFlushStr(FLUSH);
	AVMediaType avType{ AVMEDIA_TYPE_UNKNOWN };
	switch (type)
	{
	case McMediaDecoder::DecoderType::AUDIO:
		avType = AVMEDIA_TYPE_AUDIO;
		break;
	case McMediaDecoder::DecoderType::VIDEO:
		avType = AVMEDIA_TYPE_VIDEO;
		break;
	case McMediaDecoder::DecoderType::SUBTITLE:
		avType = AVMEDIA_TYPE_SUBTITLE;
		break;
	default:
		break;
	}
	d->decoders.insert(avType, decoder);
}

void McMediaDecoder::release() noexcept {
	if (d->formatCtx)
		avformat_close_input(&d->formatCtx);	// 自动置空
}

bool McMediaDecoder::initDecoder(const char *url) {
	// 打开媒体
	AVDictionary *dict = NULL;
	av_dict_set(&dict, "protocol_whitelist", "file,crypto,http,tcp", 0);
	av_dict_set(&dict, "allowed_extensions", "ALL", 0);
	if (avformat_open_input(&d->formatCtx, url, NULL, &dict) != 0) {
		qDebug() << "Couldn't open input stream.";
		return false;
	}

	// 查找是否存在stream
	if (avformat_find_stream_info(d->formatCtx, NULL) < 0) {
		qDebug() << "Couldn't find stream information.";
		return false;
	}

	d->initSuccessNum = 0;		// 清空初始化成功的数量
	for (int i = 0; i < d->formatCtx->nb_streams; ++i) {
		AVStream *stream = d->formatCtx->streams[i];	// 媒体流
		AVMediaType type = d->formatCtx->streams[i]->codec->codec_type;		// 媒体流类型
		if (d->decoders.contains(type)) {	// 如果注入过该类型的解码器，则初始化解码器
			d->initSuccessNum += d->decoders[type]->init(stream) ? 1 : 0;	// 成功加一
		}
	}
	// 如果没有任何一个解码器初始化成功，则直接返回，表示视频播放失败
	if (d->initSuccessNum == 0) {
		qDebug() << tr("没有任何解码器初始化成功，媒体无法解码");
		return false;
	}

	qint64 mediaDuration = d->formatCtx->duration;	// 单位：us
	//emit signal_durationChanged(mediaDuration / 1000L);		// 单位ms

	//Output Info-----------------------------
	printf("---------------- File Information ---------------\n");
	av_dump_format(d->formatCtx, 0, url, 0);
	printf("-------------------------------------------------\n");

	return true;
}

void McMediaDecoder::decodeDetached() {
	AVPacket *packet = (AVPacket *)av_malloc(sizeof(AVPacket));
	
	while (!d->isQuit) {
		if (d->isPause) {
			QThread::msleep(10);
			continue;
		}
		// 如果视频帧读出失败，则休眠一段时间
		if (av_read_frame(d->formatCtx, packet) < 0) {
			QThread::msleep(10);
			continue;
		}
		for (IMcDecoder *decoder : d->decoders) {
			decoder->addPacket(packet);
		}
		av_packet_unref(packet);
	}

	av_freep(&packet);
}
