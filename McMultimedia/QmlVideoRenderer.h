#pragma once

#include <QQuickItem>
#include "IMcVideoRenderer.h"

#include "mcmultimedia_global.h"

struct QmlVideoRendererData;

class MCMULTIMEDIA_EXPORT QmlVideoRenderer : public QQuickItem, public IMcVideoRenderer
{
	Q_OBJECT

public:
	QmlVideoRenderer(QQuickItem *parent = 0);
	~QmlVideoRenderer();

	// 指定需要的视频格式
	McVideoFormat::PixelFormat getVideoFormat() noexcept override;
	// 设置视频帧，需要渲染的视频都将通过该视频帧传入
	void setVideoFrame(const QSharedPointer<McVideoFrame> &frame) noexcept override;
	// 渲染
	void rendering() noexcept override;

protected:
	QSGNode *updatePaintNode(QSGNode *node, UpdatePaintNodeData *data) override;
	bool event(QEvent *e) override;

private:
	QScopedPointer<QmlVideoRendererData> d;
};
