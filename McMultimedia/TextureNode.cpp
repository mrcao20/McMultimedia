#include "texturenode.h"
#include <QQuickWindow>

TextureNode::TextureNode(QQuickWindow *window, QObject *parent) :
	QObject(parent),
	m_window(window),
	m_id(0),
	m_texture(0),
	m_size(0, 0)
{
	m_texture = m_window->createTextureFromId(m_id, QSize(1, 1)); //初始化使用0号纹理
	setTexture(m_texture);
	setFiltering(QSGTexture::Linear);
}

TextureNode::~TextureNode()
{
	delete m_texture;
}

void TextureNode::newTexture(int id, const QSize &size)
{
	m_id = id;  //原来这里还有锁、由于发信号和执行是有顺序的，我就把它取消了
	m_size = size;

	emit pendingNewTexture();
}

void TextureNode::prepareNode()
{
	int newId = m_id;
	QSize size = m_size;
	m_id = 0;
	if (newId) {
		delete m_texture;
		m_texture = m_window->createTextureFromId(newId, size);
		setTexture(m_texture);
		markDirty(DirtyMaterial); //通知其关联的其它控件纹理，材质发生了改变
		emit textureInUse();
	}
}