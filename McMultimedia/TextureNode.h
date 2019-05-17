#ifndef TEXTURENODE_H
#define TEXTURENODE_H
#include <QSGSimpleTextureNode>
#include <QObject>

QT_FORWARD_DECLARE_CLASS(QQuickWindow)
QT_FORWARD_DECLARE_CLASS(QSGTexture)
class TextureNode : public QObject, public QSGSimpleTextureNode
{
	Q_OBJECT
public:
	explicit TextureNode(QQuickWindow *window, QObject *parent = nullptr);
	~TextureNode();

signals:
	void pendingNewTexture(); //有一帧新视频挂起
	void textureInUse(); //纹理正在使用、可以渲染下一帧了

public slots:
	void newTexture(int id, const QSize &size); //当子线程渲染好一帧时调用
	void prepareNode();  //当渲染线程准备开始渲染时，会发出beforRendering信号；这时会调用该函数

private:
	int m_id;
	QSize m_size;
	QSGTexture *m_texture{ nullptr };
	QQuickWindow *m_window{ nullptr };
};

#endif // TEXTURENODE_H