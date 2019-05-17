#include "QmlSGVideoNode.h"

#include <QSGOpaqueTextureMaterial>

#include "McVideoFrame.h"

QmlSGVideoNode::QmlSGVideoNode(QQuickWindow *win)
	: m_material(new QSGOpaqueTextureMaterial)
{
	setFlag(QSGNode::OwnsGeometry);
	setFlag(QSGNode::OwnsMaterial);
	setMaterial(m_material);

	renderer = new QmlOpenGLRenderer();
	//m_material->setTexture(win->createTextureFromId(2, renderer->size()));
}

QmlSGVideoNode::~QmlSGVideoNode()
{
}

void QmlSGVideoNode::setCurrentFrame(McVideoFrame *frame) noexcept {
	renderer->rendering(frame->getData(), frame->getWidth(), frame->getHeight());
	markDirty(DirtyMaterial);
}

/* Helpers */
static inline void qSetGeom(QSGGeometry::TexturedPoint2D *v, const QPointF &p)
{
	v->x = p.x();
	v->y = p.y();
}

static inline void qSetTex(QSGGeometry::TexturedPoint2D *v, const QPointF &p)
{
	v->tx = p.x();
	v->ty = p.y();
}


void QmlSGVideoNode::setTexturedRectGeometry(const QRectF &rect) noexcept {

	QSGGeometry *g = geometry();
	if (!g)
		g = new QSGGeometry(QSGGeometry::defaultAttributes_TexturedPoint2D(), 4);
	QSGGeometry::TexturedPoint2D *v = g->vertexDataAsTexturedPoint2D();

	// Set geometry first
	qSetGeom(v + 0, rect.topLeft());
	qSetGeom(v + 1, rect.bottomLeft());
	qSetGeom(v + 2, rect.topRight());
	qSetGeom(v + 3, rect.bottomRight());

	qSetTex(v + 0, rect.topLeft());
	qSetTex(v + 1, rect.bottomLeft());
	qSetTex(v + 2, rect.topRight());
	qSetTex(v + 3, rect.bottomRight());

	if (!geometry())
		setGeometry(g);

	markDirty(DirtyGeometry);
}
