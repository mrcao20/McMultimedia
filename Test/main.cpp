#include "Test.h"
#include <QtWidgets/QApplication>
#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include "QmlMediaPlayer.h"
#include "QmlVideoRenderer.h"
#include "VideoThreadRender.h"

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	Test w;
	w.show();
	return a.exec();

	QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

	QGuiApplication app(argc, argv);

	QQmlApplicationEngine engine;
	qmlRegisterType<QmlMediaPlayer>("McMultimedia", 1, 0, "MediaPlayer");
	qmlRegisterType<VideoThreadRender>("McMultimedia", 1, 0, "VideoRenderer");
	engine.load(QUrl(QStringLiteral("qrc:/Test/main.qml")));
	if (engine.rootObjects().isEmpty())
		return -1;

	return app.exec();
}
