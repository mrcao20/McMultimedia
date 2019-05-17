#include "nv12render.h"
#include <QPainter>
#include <QPaintEngine>
#include <math.h>

Nv12Render::Nv12Render()
{
}

Nv12Render::~Nv12Render()
{
}

void Nv12Render::initialize()
{
	initializeOpenGLFunctions();

	glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
	glDisable(GL_DEPTH_TEST);

	const char *vsrc1 =
		"attribute vec4 vertexIn; \
             attribute vec4 textureIn; \
             varying vec4 textureOut;  \
             uniform mediump mat4 matrix;\
             void main(void)           \
             {                         \
                 gl_Position = vertexIn * matrix; \
                 textureOut = textureIn; \
             }";

	const char *fsrc1 =
		"varying mediump vec4 textureOut;\n"
		"uniform sampler2D textureY;\n"
		"uniform sampler2D textureUV;\n"
		"void main(void)\n"
		"{\n"
		"vec3 yuv; \n"
		"vec3 rgb; \n"
		"yuv.x = texture2D(textureY, textureOut.st).r - 0.0625; \n"
		"yuv.y = texture2D(textureUV, textureOut.st).r - 0.5; \n"
		"yuv.z = texture2D(textureUV, textureOut.st).g - 0.5; \n"
		"rgb = mat3( 1,       1,         1, \n"
		"0,       -0.39465,  2.03211, \n"
		"1.13983, -0.58060,  0) * yuv; \n"
		"gl_FragColor = vec4(rgb, 1); \n"
		"}\n";

	program1.addCacheableShaderFromSourceCode(QOpenGLShader::Vertex, vsrc1);
	program1.addCacheableShaderFromSourceCode(QOpenGLShader::Fragment, fsrc1);
	program1.link();
	program1.bind();

	GLfloat vertex[]{
		-1.0f,+1.0f,  //视点默认在(0,0,0)点，朝向是z轴负方向，即从屏幕里看向屏幕外。所以视频顶点的z值要大些。也可以通过关闭深度测试，实现同时看到视频和其他图形，而且关闭深度测试可以提高程序运行效率
		+1.0f,+1.0f,
		+1.0f,-1.0f,
		-1.0f,-1.0f,
		//纹理坐标
		0.0f,1.0f,1.0f,
		1.0f,1.0f,1.0f,
		1.0f,0.0f,1.0f,
		0.0f,0.0f,1.0f,
	};
	vbo.create();
	vbo.bind();
	vbo.allocate(vertex, sizeof(vertex));
	program1.enableAttributeArray("vertexIn");
	program1.enableAttributeArray("textureIn");
	program1.setAttributeBuffer("vertexIn", GL_FLOAT, 0, 2, 2 * sizeof(GLfloat));
	program1.setAttributeBuffer("textureIn", GL_FLOAT, 2 * 4 * sizeof(GLfloat), 3, 3 * sizeof(GLfloat));
	program1.setUniformValue("textureY", 3);
	program1.setUniformValue("textureUV", 4);

	textureY = new QOpenGLTexture(QOpenGLTexture::Target2D);
	textureUV = new QOpenGLTexture(QOpenGLTexture::Target2D);
	textureY->create();
	textureUV->create();
	textureY->bind(3);
	textureUV->bind(4);
	m_fAngle = 0;
	m_fScale = 1;
}

void Nv12Render::render()
{
	glClearColor(0.5f, 0.5f, 0.7f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	QMatrix4x4 modelview;
	modelview.rotate(m_fAngle, 0.0f, 1.0f, 0.0f);
	modelview.rotate(m_fAngle, 1.0f, 0.0f, 0.0f);
	modelview.rotate(m_fAngle, 0.0f, 0.0f, 1.0f);
	modelview.scale(m_fScale);
	modelview.translate(0.0f, -0.2f, 0.0f);

	program1.setUniformValue("matrix", modelview);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 3);

	m_fAngle += 1.0f;
}

void Nv12Render::renderFrame(uchar *imgPtr, uint videoW, uint videoH)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glActiveTexture(GL_TEXTURE3);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, videoW, videoH, 0, GL_RED, GL_UNSIGNED_BYTE, imgPtr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glActiveTexture(GL_TEXTURE4);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG, videoW >> 1, videoH >> 1, 0, GL_RG, GL_UNSIGNED_BYTE, imgPtr + videoW * videoH);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	QMatrix4x4 modelview;
	modelview.rotate(m_fAngle, 0.0f, 1.0f, 0.0f);
	modelview.rotate(m_fAngle, 1.0f, 0.0f, 0.0f);
	modelview.rotate(m_fAngle, 0.0f, 0.0f, 1.0f);
	modelview.scale(m_fScale);
	modelview.translate(0.0f, -0.2f, 0.0f);
	program1.setUniformValue("matrix", modelview);

	glDrawArrays(GL_QUADS, 0, 4);

	m_fAngle += 1.0f;
}