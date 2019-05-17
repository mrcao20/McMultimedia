#ifndef RENDERTHREAD_H
#define RENDERTHREAD_H
#include <QThread>
#include <QOffscreenSurface>
#include <QOpenGLContext>
#include "McVideoFrame.h"
#include "QmlOpenGLRenderer.h"

QT_FORWARD_DECLARE_CLASS(QOpenGLFramebufferObject)
QT_FORWARD_DECLARE_CLASS(Nv12Render)
QT_FORWARD_DECLARE_CLASS(DecodeThread)
class RenderThread : public QThread
{
	Q_OBJECT
public:
	RenderThread(QSize &size, QObject *parent = nullptr);
	~RenderThread();
	QOpenGLContext *context{ nullptr };
	QOffscreenSurface *surface{ nullptr }; //其create方法只能在GUI线程(主线程)调用，所以弄成了全局变量，方便访问
	QString videoSource;

	void setSize(QSize size) { m_size = size; }
	void setFrame(McVideoFrame *frame) { m_frame = frame; }

signals:
	void textureReady(int, const QSize); //视频帧渲染好了

public slots:
	void renderNext(); //渲染下一帧

	void shutDown(); //关闭退出

private:
	QOpenGLFramebufferObject *m_renderFbo{ 0 }; //用于渲染
	QOpenGLFramebufferObject *m_displayFbo{ 0 }; //用于显示
	QSize m_size;
	//Nv12Render *m_videoRender{ nullptr }; //渲染器
	QmlOpenGLRenderer *m_videoRender{ nullptr }; //渲染器
	DecodeThread *m_decoder{ nullptr }; //解码
	McVideoFrame *m_frame;
};

#endif // RENDERTHREAD_H