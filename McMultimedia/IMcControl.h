#ifndef _I_MC_CONTROL_H_
#define _I_MC_CONTROL_H_

#include <qglobal.h>

class IMcControl {
public:
	virtual ~IMcControl() = default;

	// 开始
	virtual bool start() noexcept = 0;
	// 暂停
	virtual void pause() noexcept = 0;
	// 恢复
	virtual void resume() noexcept = 0;
	// 停止
	virtual void stop() noexcept = 0;
	// 退出并释放相应资源
	virtual void quit() noexcept = 0;

	// 跳转
	virtual void seek(qint64 pos) noexcept = 0;
};

#endif // !_I_MC_CONTROL_H_

