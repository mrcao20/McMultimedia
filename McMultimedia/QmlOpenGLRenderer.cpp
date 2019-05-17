#include "QmlOpenGLRenderer.h"

#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <qmutex.h>

#include "McVideoFrame.h"

#define VERTEXIN 0
#define TEXTUREIN 1

struct QmlOpenGLRendererData {
	//shader程序
	QScopedPointer<QOpenGLShaderProgram> program{ nullptr };
	QScopedPointer<QOpenGLShader> vshader;
	QScopedPointer<QOpenGLShader> fshader;
	QOpenGLBuffer vbo;
	GLuint textureUniformY, textureUniformU, textureUniformV; //opengl中y、u、v分量位置
	QOpenGLTexture *textureY = nullptr, *textureU = nullptr, *textureV = nullptr;
	GLuint idY, idU, idV; //自己创建的纹理对象ID，创建错误返回0
};

QmlOpenGLRenderer::QmlOpenGLRenderer()
	: d(new QmlOpenGLRendererData)
{
	initializeOpenGLFunctions();
	//glEnable(GL_DEPTH_TEST);

	static const GLfloat vertices[]{
		//顶点坐标
		-1.0f,-1.0f,
		-1.0f,+1.0f,
		+1.0f,+1.0f,
		+1.0f,-1.0f,
		//纹理坐标
		0.0f,1.0f,
		0.0f,0.0f,
		1.0f,0.0f,
		1.0f,1.0f,
	};

	d->vbo.create();
	d->vbo.bind();
	d->vbo.allocate(vertices, sizeof(vertices));

	d->vshader.reset(new QOpenGLShader(QOpenGLShader::Vertex));
	const char *vsrc =
		"attribute vec4 vertexIn; \
    attribute vec2 textureIn; \
    varying vec2 textureOut;  \
    void main(void)           \
    {                         \
        gl_Position = vertexIn; \
        textureOut = textureIn; \
    }";
	d->vshader->compileSourceCode(vsrc);

	d->fshader.reset(new QOpenGLShader(QOpenGLShader::Fragment));
	const char *fsrc = "varying vec2 textureOut; \
    uniform sampler2D tex_y; \
    uniform sampler2D tex_u; \
    uniform sampler2D tex_v; \
    void main(void) \
    { \
        vec3 yuv; \
        vec3 rgb; \
        yuv.x = texture2D(tex_y, textureOut).r; \
        yuv.y = texture2D(tex_u, textureOut).r - 0.5; \
        yuv.z = texture2D(tex_v, textureOut).r - 0.5; \
        rgb = mat3( 1,       1,         1, \
                    0,       -0.39465,  2.03211, \
                    1.13983, -0.58060,  0) * yuv; \
        gl_FragColor = vec4(rgb, 1); \
    }";
	d->fshader->compileSourceCode(fsrc);

	d->program.reset(new QOpenGLShaderProgram());
	d->program->addShader(d->vshader.data());
	d->program->addShader(d->fshader.data());
	d->program->bindAttributeLocation("vertexIn", VERTEXIN);
	d->program->bindAttributeLocation("textureIn", TEXTUREIN);
	d->program->link();
	d->program->bind();
	d->program->enableAttributeArray(VERTEXIN);
	d->program->enableAttributeArray(TEXTUREIN);
	d->program->setAttributeBuffer(VERTEXIN, GL_FLOAT, 0, 2, 2 * sizeof(GLfloat));
	d->program->setAttributeBuffer(TEXTUREIN, GL_FLOAT, 8 * sizeof(GLfloat), 2, 2 * sizeof(GLfloat));

	d->textureUniformY = d->program->uniformLocation("tex_y");
	d->textureUniformU = d->program->uniformLocation("tex_u");
	d->textureUniformV = d->program->uniformLocation("tex_v");
	d->textureY = new QOpenGLTexture(QOpenGLTexture::Target2D);
	d->textureU = new QOpenGLTexture(QOpenGLTexture::Target2D);
	d->textureV = new QOpenGLTexture(QOpenGLTexture::Target2D);
	d->textureY->create();
	d->textureU->create();
	d->textureV->create();
	d->idY = d->textureY->textureId();
	d->idU = d->textureU->textureId();
	d->idV = d->textureV->textureId();
	glClearColor(0.0, 0.0, 0.0, 0.0);
}


QmlOpenGLRenderer::~QmlOpenGLRenderer()
{
}

void QmlOpenGLRenderer::clearup() noexcept {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void QmlOpenGLRenderer::rendering(uchar *yuv, uint width, uint height) noexcept {
	clearup();
	//    QMatrix4x4 m;
	//    m.perspective(60.0f, 4.0f/3.0f, 0.1f, 100.0f );//透视矩阵随距离的变化，图形跟着变化。屏幕平面中心就是视点（摄像头）,需要将图形移向屏幕里面一定距离。
	//    m.ortho(-2,+2,-2,+2,-10,10);//近裁剪平面是一个矩形,矩形左下角点三维空间坐标是（left,bottom,-near）,右上角点是（right,top,-near）所以此处为负，表示z轴最大为10；
									//远裁剪平面也是一个矩形,左下角点空间坐标是（left,bottom,-far）,右上角点是（right,top,-far）所以此处为正，表示z轴最小为-10；
									//此时坐标中心还是在屏幕水平面中间，只是前后左右的距离已限制。
	glActiveTexture(GL_TEXTURE0);  //激活纹理单元GL_TEXTURE0,系统里面的
	glBindTexture(GL_TEXTURE_2D, d->idY); //绑定y分量纹理对象id到激活的纹理单元
	//使用内存中的数据创建真正的y分量纹理数据
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, yuv);
	//https://blog.csdn.net/xipiaoyouzi/article/details/53584798 纹理参数解析
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glActiveTexture(GL_TEXTURE1); //激活纹理单元GL_TEXTURE1
	glBindTexture(GL_TEXTURE1, d->idU);
	//使用内存中的数据创建真正的u分量纹理数据
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width >> 1, height >> 1, 0, GL_RED, GL_UNSIGNED_BYTE, yuv + width * height);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glActiveTexture(GL_TEXTURE2); //激活纹理单元GL_TEXTURE2
	glBindTexture(GL_TEXTURE_2D, d->idV);
	//使用内存中的数据创建真正的v分量纹理数据
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width >> 1, height >> 1, 0, GL_RED, GL_UNSIGNED_BYTE, yuv + width * height * 5 / 4);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	//指定y纹理要使用新值
	glUniform1i(d->textureUniformY, 0);
	//指定u纹理要使用新值
	glUniform1i(d->textureUniformU, 1);
	//指定v纹理要使用新值
	glUniform1i(d->textureUniformV, 2);
	//使用顶点数组方式绘制图形
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	glFlush();
}

void QmlOpenGLRenderer::resizeRenderer(int width, int height) noexcept {
	glViewport(0, 0, width, height);
}
