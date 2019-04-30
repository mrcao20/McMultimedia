#ifndef _MC_MEDIA_DECODER_H_
#define _MC_MEDIA_DECODER_H_
#pragma execution_character_set("utf-8")

#include <QObject>

enum AVMediaType;

struct McMediaDecoderData;

class IMcDecoder;

class McMediaDecoder : public QObject {
	Q_OBJECT

public:
	enum class DecoderType {
		AUDIO,
		VIDEO,
		SUBTITLE
	};

	explicit McMediaDecoder(QObject *parent = 0);
	~McMediaDecoder();

	void setMediaUrl(const QString &url) noexcept;
	void start() noexcept;
	void stop() noexcept;
	void addDecoder(DecoderType type, IMcDecoder *decoder) noexcept;

	void release() noexcept;

private:
	bool initDecoder(const char *url);
	void decodeDetached();

private:
	QScopedPointer<McMediaDecoderData> d;
};

#endif // !_MC_MEDIA_DECODER_H_