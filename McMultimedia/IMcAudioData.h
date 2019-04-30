#ifndef _I_MC_AUDIO_DATA_H_
#define _I_MC_AUDIO_DATA_H_

#include <functional>
#include <qsharedpointer.h>

class McAudioFrame;

class IMcAudioData {
public:
	virtual ~IMcAudioData() = default;

	virtual void getAudioData(const QSharedPointer<McAudioFrame> &frame, const std::function<void()> &callback) noexcept = 0;
};

#endif // !_I_MC_AUDIO_DATA_H_