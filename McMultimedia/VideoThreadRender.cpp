#include "VideoThreadRender.h"
#include "renderthread.h"
#include "texturenode.h"
#include <QQuickWindow>
#include <qsize.h>
#include "McVideoFrame.h"

//QList<QThread*> VideoThreadRender::threads;
VideoThreadRender::VideoThreadRender(QQuickItem *parent)
	: m_renderThread(0),
	QQuickItem(parent)
{
	connect(this, &QQuickItem::windowChanged, [this](QQuickWindow* window) {
		connect(window, &QQuickWindow::widthChanged, [this, window]() {
			m_renderThread->setSize(window->size());
		});
	});
	setFlag(ItemHasContents, true); //有图像显示，设置为true
	m_renderThread = new RenderThread(QSize(512, 512), nullptr); //实例子线程，和下面的渲染线程不是同一个，渲染线程是说的Scene Graphics里面的渲染线程
}

McVideoFormat::PixelFormat VideoThreadRender::getVideoFormat() noexcept {
	return McVideoFormat::PixelFormat::Format_YUV420P;
}

void VideoThreadRender::setVideoFrame(const QSharedPointer<McVideoFrame> &frame) noexcept {
	this->frame = frame;
}

void VideoThreadRender::rendering() noexcept {
	
}

void VideoThreadRender::ready()
{
	m_renderThread->surface = new QOffscreenSurface(); //实例一个离屏的Surface，有点像不显示的Window，使得opengl的contex能够绑定到它上面
	m_renderThread->surface->setFormat(m_renderThread->context->format());
	m_renderThread->surface->create(); //根据文档QOffscreenSurface的create只能在GUI线程调用，所以在这里做了实例和初始化。

	m_renderThread->videoSource = m_videoSource;

	m_renderThread->moveToThread(m_renderThread); //移动到子线程循环
	//当场景失效后，关闭子线程的资源
	connect(window(), &QQuickWindow::sceneGraphInvalidated, m_renderThread, &RenderThread::shutDown, Qt::QueuedConnection);
	//启动子线程
	m_renderThread->start();
	update(); //再update一次用于实例TextureNode，因为程序刚初始化时会调用一次，但在初始化子线程后，返回了，所以要再来一次实例TextureNode。
}
//此函数是由渲染线程调用的，不是在GUI线程
QSGNode *VideoThreadRender::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *)
{
	TextureNode *node = static_cast<TextureNode *>(oldNode);

	if (!m_renderThread->context) {
		QOpenGLContext *current = window()->openglContext();
		current->doneCurrent(); //取消opengl在当前上下文中的绑定，因为下面要设置shareContext，即将sharedContext移动到子线程

		m_renderThread->context = new QOpenGLContext();
		m_renderThread->context->setFormat(current->format());
		m_renderThread->context->setShareContext(current);
		m_renderThread->context->create();
		m_renderThread->context->moveToThread(m_renderThread); //context有线程归属性，一个context只能被它关联的线程调用makeCurrent，不能被其它线程调用;也只能有一个对应的surface
															   //一个线程在同一时刻也只能有一个context
		current->makeCurrent(window()); //恢复绑定

		m_renderThread->setFrame(frame.data());
		QMetaObject::invokeMethod(this, "ready"); //跨线程调用
		return 0;
	}

	if (!node) {
		node = new TextureNode(window()); //实例化自定义的纹理结点

		//当纹理在子线程渲染好后，将纹理id、大小设置到自定义的QSimpleTextureNode结构中
		connect(m_renderThread, &RenderThread::textureReady, node, &TextureNode::newTexture, Qt::DirectConnection);
		//update函数调用后，渲染线程会在适当的时候发出beforRendering信号
		connect(node, &TextureNode::pendingNewTexture, window(), &QQuickWindow::update, Qt::QueuedConnection);
		//在开始渲染之前，把子线程渲染好的纹理设置到QSimpletTextureNode中，以便渲染线程使用
		connect(window(), &QQuickWindow::beforeRendering, node, &TextureNode::prepareNode, Qt::DirectConnection);
		//渲染好的纹理被使用后，通知子线程渲染下一帧
		connect(node, &TextureNode::textureInUse, m_renderThread, &RenderThread::renderNext, Qt::QueuedConnection);

		// Get the production of FBO textures started..
		QMetaObject::invokeMethod(m_renderThread, "renderNext", Qt::QueuedConnection);
	}

	node->setRect(boundingRect()); //设置显示区域，为qml分配的整个区域
		return node;
}

void VideoThreadRender::setVideoSource(QString s)
{
	if (m_videoSource != s) {
		emit videoSourceChanged();
	}
	m_videoSource = s;
}

QString VideoThreadRender::videoSource()
{
	return m_videoSource;
}