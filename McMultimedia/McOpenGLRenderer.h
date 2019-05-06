#ifndef _MC_OPENGL_RENDERER_H_
#define _MC_OPENGL_RENDERER_H_

#include <QOpenGLWidget>
#include <qopenglfunctions.h>
#include "IMcVideoRenderer.h"

#include "mcmultimedia_global.h"

struct McOpenGLRendererData;

class MCMULTIMEDIA_EXPORT McOpenGLRenderer : public QOpenGLWidget, public IMcVideoRenderer, protected QOpenGLFunctions {
	Q_OBJECT

public:
	explicit McOpenGLRenderer(QWidget *parent = 0);
	virtual ~McOpenGLRenderer();

	// 指定需要的视频格式
	McVideoFormat::PixelFormat getVideoFormat() noexcept override;
	// 设置视频帧，需要渲染的视频都将通过该视频帧传入
	void setVideoFrame(const QSharedPointer<McVideoFrame> &frame) noexcept override;
	// 渲染
	void rendering() noexcept override;

protected:
	//初始化gl
	void initializeGL() override;
	//刷新显示
	void paintGL() override;
	void resizeGL(int width, int height) override;

private:
	QScopedPointer<McOpenGLRendererData> d;
};

#endif // !_MC_OPENGL_RENDERER_H_