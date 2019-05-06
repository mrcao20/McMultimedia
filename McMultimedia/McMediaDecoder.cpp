#include "McMediaDecoder.h"

extern "C" {
#include "libavformat/avformat.h"
};

#include <QtConcurrent>
#include <qdebug.h>

#include "IMcDecoder.h"
#include "McGlobal.h"

#define FLUSH "FLUSH"

struct McMediaDecoderData {
	AVFormatContext *formatCtx{ nullptr };
	AVPacket seekPacket;						// 跳转所用资源包

	QMap<AVMediaType, IMcDecoder *> decoders;	// 用来存放解码器，解码器由外部注入，注入时必须指定解码器类型

	QString mediaUrl;							// 媒体路径
	int initSuccessNum{ 0 };					// 解码器初始化成功的数量
	bool isQuit{ false };						// 是否停止解码
	bool isPause{ false };						// 是否暂停解码

	qint64 seekPos{ -1 };						// 将要跳转到的位置，默认为-1，表示不跳转，单位：us

	QFuture<void> decodeThread;					// 解码线程
};

McMediaDecoder::McMediaDecoder(QObject *parent)
	: QObject(parent)
	, d(new McMediaDecoderData())
{
	// 初始化跳转资源包
	av_new_packet(&d->seekPacket, qstrlen(FLUSH) + 1);
	d->seekPacket.data = (uint8_t *)FLUSH;
}

McMediaDecoder::~McMediaDecoder(){
	quit();
	av_packet_unref(&d->seekPacket);
}

void McMediaDecoder::setMediaUrl(const QString &url) noexcept {
	d->mediaUrl = url;
}

bool McMediaDecoder::start() noexcept {
	quit();	// 开始时退出先前的解码器
	if (!initDecoder(d->mediaUrl.toLocal8Bit().data()))	// 初始化解码器
		return false;
	d->seekPos = -1;
	resume();
	d->isQuit = false;
	// 后台解码
	d->decodeThread = QtConcurrent::run(this, &McMediaDecoder::decodeDetached);
	return true;
}

void McMediaDecoder::pause() noexcept {
	d->isPause = true;
}

void McMediaDecoder::resume() noexcept {
	d->isPause = false;
}

void McMediaDecoder::stop() noexcept {
	d->isQuit = true;
}

void McMediaDecoder::quit() noexcept {
	stop();
	while (!d->decodeThread.isFinished())	// 等待结束
		QThread::msleep(10);
	release();		// 清除上一次数据
}

void McMediaDecoder::seek(qint64 pos) noexcept {
	d->seekPos = pos * 1000;		// ms转us
}

void McMediaDecoder::addDecoder(DecoderType type, IMcDecoder *decoder) noexcept {
	decoder->setFlushStr(FLUSH);	// 添加解码器时设置刷新字符串
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

bool McMediaDecoder::initDecoder(const char *url) noexcept {
	// 打开媒体
	AVDictionary *dict = NULL;
	av_dict_set(&dict, "protocol_whitelist", "file,crypto,http,tcp", 0);	// 网络协议报名单
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
	emit signal_durationChanged(mediaDuration / 1000L);		// 单位ms

	//Output Info-----------------------------
	printf("---------------- File Information ---------------\n");
	av_dump_format(d->formatCtx, 0, url, 0);
	printf("-------------------------------------------------\n");

	return true;
}

void McMediaDecoder::decodeDetached() noexcept {
	// 申请一个packet，用于接收读出的数据包
	AVPacket *packet = (AVPacket *)av_malloc(sizeof(AVPacket));
	
	bool isRepeat = false;			// 是否需要重复播放
	bool isReadFinished = false;	// 读包是否结束
	while (!d->isQuit) {
		qint64 seekPos = d->seekPos;	// 记录需要跳转到的位置
		d->seekPos = -1;	// 跳转结束，将将要跳转的位置置为-1，表示不需要跳转
		// 如果播放的视频为m3u8格式，并且需要跳转到的位置为视频开始，则重新解码播放
		if (seekPos != -1 && seekPos < d->formatCtx->start_time && d->mediaUrl.endsWith(".m3u8")) {
			isRepeat = true;
			break;
		}
		// 跳转
		if (seekTo(seekPos)) {
			// 如果跳转成功
			MC_LOOP_CALL_P(d->decoders, clearPacket());	// 清除解码器的资源包
			MC_LOOP_CALL_P(d->decoders, addPacket(&d->seekPacket));	// 添加跳转资源包
			continue;
		}
		if (d->isPause) {
			QThread::msleep(10);
			continue;
		}
		if (isContinue()) {
			QThread::msleep(10);
			continue;
		}
		// 如果视频帧读出失败，则休眠一段时间
		if (av_read_frame(d->formatCtx, packet) < 0) {
			if (isFinished() && !isReadFinished) {
				emit signal_decodeFinished();
				isReadFinished = true;
				break;
			}
			QThread::msleep(10);
			continue;
		}
		isReadFinished = false;
		MC_LOOP_CALL_P(d->decoders, addPacket(packet));
		av_packet_unref(packet);
	}

	av_freep(&packet);

	if (isRepeat) {
		QtConcurrent::run([this]() {
			MC_LOOP_CALL_P(d->decoders, clearPacket());
			start();
		});
	}
}

bool McMediaDecoder::isContinue() noexcept {
	int iContinueNum = 0;	// 表示解码器中的包数量太多，需要跳过的解码器的个数
	for (IMcDecoder *decoder : d->decoders) {
		// 如果解码器中的包数量太多，该解码器请求跳过，此处最大包数量为100
		iContinueNum += (decoder->getPacketNum() > 100) ? 1 : 0;
	}
	// 如果需要跳过的解码器数等于解码器数量，则暂时不再读取媒体包
	if (iContinueNum == d->initSuccessNum) {
		return true;
	}
	return false;
}

bool McMediaDecoder::isFinished() noexcept {
	int isFinishedNum = 0;	// 表示解码器中的包数量为0的解码器个数
	for (IMcDecoder *decoder : d->decoders) {
		isFinishedNum += (decoder->getPacketNum() == 0) ? 1 : 0;
	}
	// 如果包数量为0的解码器数等于解码器总数量，则完成解码
	if (isFinishedNum == d->initSuccessNum) {
		return true;
	}
	return false;
}

bool McMediaDecoder::seekTo(qint64 &pos) noexcept {
	// 当将要跳转的位置为-1时，表示不需要跳转，否则表示需要跳转
	if (pos == -1)
		return false;
	// 此处第二个参数为媒体流的index，此处指定为-1，那么第三个参数就为微秒计的媒体时间
	/*	如果指定为具体媒体流index，那么需要通过
			AVRational aVRational = av_get_time_base_q();
			seekPos = av_rescale_q(seekPos, aVRational, pFormatCtx->streams[seekIndex]->time_base);
		来计算出第三个参数的值
	*/
	// AVSEEK_FLAG_BACKWARD表示如果0、2为关键帧，而跳转到1帧的话，将从0帧开始播放
	//if (av_seek_frame(m_pFormatCtx, -1, pos, AVSEEK_FLAG_BACKWARD) < 0) {
	int seekFlag = 0;
	if (d->mediaUrl.endsWith(".m3u8")) {
		//seekFlag |= AVSEEK_FLAG_ANY;
		//seekFlag |= AVSEEK_FLAG_FRAME;
		seekFlag |= AVSEEK_FLAG_BACKWARD;
	}
	if (avformat_seek_file(d->formatCtx, -1, INT64_MIN, pos, INT64_MAX, seekFlag) < 0) {
		if (pos < d->formatCtx->start_time) {
			pos = d->formatCtx->start_time;
			return seekTo(pos);
		}
		qCritical() << "Seek failed.";
		return false;
	}

	return true;
}
