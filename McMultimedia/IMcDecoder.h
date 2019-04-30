#ifndef _I_MC_DECODER_H_
#define _I_MC_DECODER_H_

struct AVStream;
struct AVPacket;

class IMcDecoder {
public:
	virtual ~IMcDecoder() = default;

	virtual void setFlushStr(const char *str) noexcept = 0;
	virtual bool init(AVStream *stream) noexcept = 0;
	virtual void addPacket(AVPacket *packet) noexcept = 0;
};

#endif // ! _I_MC_DECODER_H_

