#include "McMediaControl.h"

#include <qlist.h>

#include "McGlobal.h"

struct McMediaControlData {
	QList<IMcMediaControl *> controls;
};

McMediaControl::McMediaControl(QObject *parent)
	: QObject(parent)
	, d(new McMediaControlData())
{
}

McMediaControl::~McMediaControl(){
}

void McMediaControl::addControl(IMcMediaControl *control) noexcept {
	if (control == this)
		return;
	d->controls.append(control);
}

void McMediaControl::start() noexcept {
	MC_LOOP_CALL_P(d->controls, start);
}

void McMediaControl::pause() noexcept {
	MC_LOOP_CALL_P(d->controls, pause);
}

void McMediaControl::resume() noexcept {
	MC_LOOP_CALL_P(d->controls, resume);
}

void McMediaControl::stop() noexcept {
	MC_LOOP_CALL_P(d->controls, stop);
}

void McMediaControl::quit() noexcept {
	MC_LOOP_CALL_P(d->controls, quit);
}
