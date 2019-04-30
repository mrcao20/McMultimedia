#ifndef _I_MC_AUDIO_H_
#define _I_MC_AUDIO_H_

#include "IMcAudioData.h"

#include <qaudioformat.h>

class IMcAudio : public IMcAudioData {
public:
	virtual ~IMcAudio() = default;

	virtual QAudioFormat getAudioFormat() noexcept = 0;
};

#endif // !_I_MC_AUDIO_H_