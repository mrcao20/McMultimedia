#include "QmlMediaPlayerPlugin.h"

#include <qqml.h>

#include "QmlMediaPlayer.h"

QmlMediaPlayerPlugin::QmlMediaPlayerPlugin(QObject *parent) 
	: QQmlExtensionPlugin(parent)
{
}

void QmlMediaPlayerPlugin::registerTypes(const char *uri) {
	qmlRegisterType<QmlMediaPlayer>(uri, 1, 0, "MediaPlayer");
}