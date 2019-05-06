#ifndef _MC_MEDIA_DECODER_H_
#define _MC_MEDIA_DECODER_H_
#pragma execution_character_set("utf-8")

#include <QObject>
#include "IMcControl.h"
#include "IMcMediaDecoder.h"

enum AVMediaType;

struct McMediaDecoderData;

class IMcDecoder;

class McMediaDecoder : public QObject, public IMcControl, public IMcMediaDecoder {
	Q_OBJECT

public:
	enum class DecoderType {
		AUDIO,
		VIDEO,
		SUBTITLE
	};

	explicit McMediaDecoder(QObject *parent = 0);
	~McMediaDecoder();

	// 开始
	bool start() noexcept override;
	// 暂停
	void pause() noexcept override;
	// 恢复
	void resume() noexcept override;
	// 停止
	void stop() noexcept override;
	// 退出
	void quit() noexcept override;

	// 跳转
	void seek(qint64 pos) noexcept override;

	// 设置媒体路径
	void setMediaUrl(const QString &url) noexcept override;
	// 添加解码器
	void addDecoder(DecoderType type, IMcDecoder *decoder) noexcept;

signals:
	// 媒体总时长，单位：ms
	void signal_durationChanged(qint64 duration);
	// 解码结束
	void signal_decodeFinished();

private:
	// 释放资源
	void release() noexcept;
	// 初始化解码器
	bool initDecoder(const char *url) noexcept;
	// 后台解码，使用QtConcurrent
	void decodeDetached() noexcept;
	// 判断是否跳过本次解码
	bool isContinue() noexcept;
	// 判断是否解码结束
	bool isFinished() noexcept;
	// 解码器跳转
	bool seekTo(qint64 &pos) noexcept;

private:
	QScopedPointer<McMediaDecoderData> d;
};

#endif // !_MC_MEDIA_DECODER_H_