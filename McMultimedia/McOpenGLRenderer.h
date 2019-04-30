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

	McVideoFormat::PixelFormat getVideoFormat() noexcept override;
	void setVideoFrame(const QSharedPointer<McVideoFrame> &frame) noexcept override;
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