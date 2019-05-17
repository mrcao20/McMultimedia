#include "QmlVideoRenderer.h"

#include <qquickwindow.h>

#include "McVideoFrame.h"
#include "QmlOpenGLRenderer.h"
#include "QmlSGVideoNode.h"

struct QmlVideoRendererData {
	QScopedPointer<QmlOpenGLRenderer> renderer{ nullptr };
	QSharedPointer<McVideoFrame> frame;		// 视频帧，由外部传入
	bool frameChanged{ false };				// 标志视频帧是否发生了改变
};

QmlVideoRenderer::QmlVideoRenderer(QQuickItem *parent)
	: QQuickItem(parent)
	, d(new QmlVideoRendererData)
{
	setFlag(QQuickItem::ItemHasContents, true);
}

QmlVideoRenderer::~QmlVideoRenderer()
{
}

McVideoFormat::PixelFormat QmlVideoRenderer::getVideoFormat() noexcept {
	return McVideoFormat::PixelFormat::Format_YUV420P;
}

void QmlVideoRenderer::setVideoFrame(const QSharedPointer<McVideoFrame> &frame) noexcept {
	d->frame = frame;
}

void QmlVideoRenderer::rendering() noexcept {
	d->frameChanged = true;
	//QMetaObject::invokeMethod(this, "update"); // slower than directly postEvent
	QCoreApplication::postEvent(this, new QEvent(QEvent::User));	// QtQuick只能在GUI线程调用update函数更新界面
}

QSGNode *QmlVideoRenderer::updatePaintNode(QSGNode *node, UpdatePaintNodeData *data) {
	Q_UNUSED(data);
	if (!node)
		node = new QmlSGVideoNode(window());
	QmlSGVideoNode *sgvn = static_cast<QmlSGVideoNode*>(node);
	Q_ASSERT(sgvn);
	QMutexLocker locker(&d->frame->getMutex());
	if (d->frameChanged)
		sgvn->setCurrentFrame(d->frame.data());
	d->frameChanged = false;
	qDebug() << "ssss";
	sgvn->setTexturedRectGeometry(boundingRect());
	return node;
}

bool QmlVideoRenderer::event(QEvent *e) {
	if (e->type() != QEvent::User)
		return QQuickItem::event(e);
	update();
	return true;
}
