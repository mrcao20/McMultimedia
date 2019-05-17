#ifndef VIDEOTHREADRENDER_H
#define VIDEOTHREADRENDER_H
#include <QQuickItem>
#include "IMcVideoRenderer.h"

#include "mcmultimedia_global.h"

struct VideoThreadRenderData;

QT_FORWARD_DECLARE_CLASS(RenderThread)
class MCMULTIMEDIA_EXPORT VideoThreadRender : public QQuickItem, public IMcVideoRenderer
{
	Q_OBJECT
		Q_PROPERTY(QString videoSource READ videoSource WRITE setVideoSource NOTIFY videoSourceChanged)
public:
	VideoThreadRender(QQuickItem *parent = nullptr);
	static QList<QThread*> threads;

	// 指定需要的视频格式
	McVideoFormat::PixelFormat getVideoFormat() noexcept override;
	// 设置视频帧，需要渲染的视频都将通过该视频帧传入
	void setVideoFrame(const QSharedPointer<McVideoFrame> &frame) noexcept override;
	// 渲染
	void rendering() noexcept override;

public Q_SLOTS:
	void ready();

signals:
	void videoSourceChanged();

protected:
	QSGNode *updatePaintNode(QSGNode *, UpdatePaintNodeData *) override;

private:
	RenderThread *m_renderThread{ nullptr };
	QString m_videoSource;

	void setVideoSource(QString);
	QString videoSource();

	QSharedPointer<McVideoFrame> frame;		// 视频帧，由外部传入
};

#endif // VIDEOTHREADRENDER_H