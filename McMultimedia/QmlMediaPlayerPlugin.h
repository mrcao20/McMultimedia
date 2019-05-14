#ifndef _QML_MEDIA_PLAYER_PLUGIN_H_
#define _QML_MEDIA_PLAYER_PLUGIN_H_

#include <QQmlExtensionPlugin>

class QmlMediaPlayerPlugin : public QQmlExtensionPlugin {
	Q_OBJECT
	Q_PLUGIN_METADATA(IID QQmlExtensionInterface_iid)
	Q_DISABLE_COPY(QmlMediaPlayerPlugin)

public:
	explicit QmlMediaPlayerPlugin(QObject *parent = Q_NULLPTR);

	void registerTypes(const char *uri) override;
};

#endif // !_QML_MEDIA_PLAYER_PLUGIN_H_