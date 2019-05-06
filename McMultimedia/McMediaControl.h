#ifndef _MC_MEDIA_CONTROL_H_
#define _MC_MEDIA_CONTROL_H_

#include <QObject>
#include "IMcMediaControl.h"

struct McMediaControlData;

class McMediaControl : public QObject, public IMcMediaControl {
	Q_OBJECT

public:
	explicit McMediaControl(QObject *parent = 0);
	virtual ~McMediaControl();

	// Ìí¼Ó¿ØÖÆÆ÷
	void addControl(IMcControl *control) noexcept;

	bool start() noexcept override;
	void pause() noexcept override;
	void resume() noexcept override;
	void stop() noexcept override;
	void quit() noexcept override;

	void seek(qint64 pos) noexcept override;

private:
	QScopedPointer<McMediaControlData> d;
};

#endif // !_MC_MEDIA_CONTROL_H_