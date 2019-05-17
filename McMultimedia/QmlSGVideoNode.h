#pragma once

#include <QSGGeometryNode>

#include <qquickwindow.h>

#include "QmlOpenGLRenderer.h"

class McVideoFrame;

QT_FORWARD_DECLARE_CLASS(QSGOpaqueTextureMaterial)
class QmlSGVideoNode : public QSGGeometryNode
{

public:
	QmlSGVideoNode(QQuickWindow *win);
	~QmlSGVideoNode();

	void setCurrentFrame(McVideoFrame *frame) noexcept;
	void setTexturedRectGeometry(const QRectF &rect) noexcept;

private:
	QSGOpaqueTextureMaterial *m_material{ nullptr };
	QmlOpenGLRenderer *renderer{ nullptr };

};
