#include "renderthread.h"
#include <QOpenGLFramebufferObject>
#include <QGuiApplication>
#include "nv12render.h"
#include <QDebug>

RenderThread::RenderThread(QSize &size, QObject *parent) :
	QThread(parent),
	m_size(size)
{
	//m_decoder = new DecodeThread; //实例化解码线程
}

RenderThread::~RenderThread()
{
}

void RenderThread::renderNext()
{
	context->makeCurrent(surface); //使上下文回到离屏的surface

	if (!m_renderFbo) {
		QOpenGLFramebufferObjectFormat format;
		format.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
		m_renderFbo = new QOpenGLFramebufferObject(m_size, format); //创建和windown(显示屏)格式一样的FBO
		m_displayFbo = new QOpenGLFramebufferObject(m_size, format);
		m_videoRender = new QmlOpenGLRenderer(); //实例化解码器
		//m_videoRender->initialize();   //初始化
		//m_decoder->setUrl(videoSource); //设置视频地址
		//m_decoder->start(); //开始解码
	}

	m_renderFbo->bind(); //绑定渲染的FBO到当前上下文
	context->functions()->glViewport(0, 0, m_size.width(), m_size.height()); //重调视口

//    m_videoRender->render();
	m_videoRender->rendering(m_frame->getData(), m_frame->getWidth(), m_frame->getHeight()); //渲染一帧视频
	context->functions()->glFlush(); //刷新一下

	m_renderFbo->bindDefault();  //渲染的FBO绑定到默认的上下文(也就是与主屏的surface对应的那个上下文),测试了，不绑定也可以
	qSwap(m_renderFbo, m_displayFbo); //交换两个FBO的内容

	emit textureReady(m_displayFbo->texture(), m_size);
}

void RenderThread::shutDown()
{
	context->makeCurrent(surface);
	delete m_renderFbo;
	delete m_displayFbo;
	delete m_videoRender;
	context->doneCurrent();
	delete context;

	surface->deleteLater();

	exit();
	moveToThread(QGuiApplication::instance()->thread());
}