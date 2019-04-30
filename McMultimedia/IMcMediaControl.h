#ifndef _I_MC_MEDIA_CONTROL_H_
#define _I_MC_MEDIA_CONTROL_H_

class IMcMediaControl {
public:
	virtual ~IMcMediaControl() = default;

	virtual void start() noexcept = 0;
	virtual void pause() noexcept = 0;
	virtual void resume() noexcept = 0;
	virtual void stop() noexcept = 0;
	virtual void quit() noexcept = 0;
};

#endif // !_I_MC_MEDIA_CONTROL_H_

