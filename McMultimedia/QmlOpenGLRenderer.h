#pragma once

#include <qopenglfunctions.h>

struct QmlOpenGLRendererData;

class QmlOpenGLRenderer : protected QOpenGLFunctions
{
public:
	QmlOpenGLRenderer();
	virtual ~QmlOpenGLRenderer();

	void clearup() noexcept;
	// 渲染
	void rendering(uchar *yuv, uint width, uint height) noexcept;
	// 调整渲染器大小
	void resizeRenderer(int width, int height) noexcept;

private:
	QScopedPointer<QmlOpenGLRendererData> d;
};

