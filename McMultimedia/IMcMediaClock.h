#ifndef _I_MC_MEDIA_CLOCK_H_
#define _I_MC_MEDIA_CLOCK_H_

#include <qglobal.h>

class IMcMediaClock {
public:
	virtual ~IMcMediaClock() = default;

	virtual qint64 getMediaClock() noexcept = 0;
};

#endif // !_I_MC_MEDIA_CLOCK_H_

