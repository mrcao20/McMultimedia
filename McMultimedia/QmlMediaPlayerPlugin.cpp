#include "QmlMediaPlayerPlugin.h"

#include <qqml.h>

#include "QmlMediaPlayer.h"
#include "QmlVideoRenderer.h"

QmlMediaPlayerPlugin::QmlMediaPlayerPlugin(QObject *parent) 
	: QQmlExtensionPlugin(parent)
{
}

void QmlMediaPlayerPlugin::registerTypes(const char *uri) {
	qmlRegisterType<QmlMediaPlayer>(uri, 1, 0, "MediaPlayer");
	qmlRegisterType<QmlVideoRenderer>(uri, 1, 0, "VideoRenderer");
}