#include "McMediaControl.h"

#include <qlist.h>

#include "McGlobal.h"

struct McMediaControlData {
	QList<IMcControl *> controls;
};

McMediaControl::McMediaControl(QObject *parent)
	: QObject(parent)
	, d(new McMediaControlData())
{
}

McMediaControl::~McMediaControl(){
	quit();
}

void McMediaControl::addControl(IMcControl *control) noexcept {
	if (control == this)
		return;
	d->controls.append(control);
}

bool McMediaControl::start() noexcept {
	stop();
	for (auto c : d->controls) {
		if (!c->start()) {
			stop();
			return false;
		}
	}
	return true;
}

void McMediaControl::pause() noexcept {
	MC_LOOP_CALL_P(d->controls, pause());
}

void McMediaControl::resume() noexcept {
	MC_LOOP_CALL_P(d->controls, resume());
}

void McMediaControl::stop() noexcept {
	MC_LOOP_CALL_P(d->controls, stop());
}

void McMediaControl::quit() noexcept {
	MC_LOOP_CALL_P(d->controls, quit());
}

void McMediaControl::seek(qint64 pos) noexcept {
	MC_LOOP_CALL_P(d->controls, seek(pos));
}
