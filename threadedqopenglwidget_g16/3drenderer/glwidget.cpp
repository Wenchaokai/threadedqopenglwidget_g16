/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "renderer.h"

#include "glwidget.h"
#include <qmath.h>
#include <QGuiApplication>
#include <iostream>
using namespace std;

#include <QtCore> ////QFile
#include <gl/GLU.h>
#include <QMutex>


#include <QMouseEvent>
#include <QWheelEvent>

GLWidget::GLWidget(QWidget *parent)
    : QOpenGLWidget(parent)
{
    setMinimumSize(300, 250);

    connect(this, &QOpenGLWidget::aboutToCompose, this, &GLWidget::onAboutToCompose);
    connect(this, &QOpenGLWidget::frameSwapped, this, &GLWidget::onFrameSwapped);
    connect(this, &QOpenGLWidget::aboutToResize, this, &GLWidget::onAboutToResize);
    connect(this, &QOpenGLWidget::resized, this, &GLWidget::onResized);

    m_thread = new QThread;
    m_renderer = new Renderer(this);

#ifdef ENABLE_MT
    m_renderer->moveToThread(m_thread);
#endif
    connect(m_thread, &QThread::finished, m_renderer, &QObject::deleteLater);

    connect(this, &GLWidget::renderRequested, m_renderer, &Renderer::render);
    connect(m_renderer, &Renderer::contextWanted, this, &GLWidget::grabContext);

	v3d_debug("GLWidget::GLWidget complete m_thread:  %p, qGuiApp->thread():  %p", m_thread, qGuiApp->thread());
#ifdef ENABLE_MT
    m_thread->start();
#endif
	xRot = yRot = zRot = 0;

	_zoom = 0; 

	rotate_Timer = new QTimer(this);
	connect(rotate_Timer, SIGNAL(timeout()), this, SLOT(obj_rotate()));

	mousemove_Timer = new QTimer(this);
	connect(mousemove_Timer, SIGNAL(timeout()), this, SLOT(mousemove_enable()));
	mousemove_Timer->start(mousemove_timer_interval);
}

GLWidget::~GLWidget()
{
    m_renderer->prepareExit();
#ifdef ENABLE_MT
    m_thread->quit();
    m_thread->wait();
    delete m_thread;
#endif
	if (rotate_Timer)
		delete rotate_Timer; 
	if (mousemove_Timer)
		delete mousemove_Timer;
}

void GLWidget::initializeGL()
{
	//Q_ASSERT(initializeOpenGLFunctions()); //[Jian] comment out since it will cause crash for sparse texture 2022/05/06. 
}

QSize GLWidget::minimumSizeHint() const
{
	return QSize(50, 50);
}

QSize GLWidget::sizeHint() const
{
	return QSize(800, 800);
}

void GLWidget::resizeGL(int width, int height)
{
	viewW = width; viewH = height;
	if (m_renderer) m_renderer->setupView(width, height);

	/*QOpenGLWidget::UpdateBehavior bh = updateBehavior();

	setUpdateBehavior(QOpenGLWidget::UpdateBehavior::NoPartialUpdate);*/

	v3d_debug("============== GLWidget::resizeGL() width:%d, height:%d =================  ", width, height);

	if (m_renderer->m_inited)
		emit renderRequested();
}

void GLWidget::rotateBy(int xAngle, int yAngle, int zAngle)
{
	xRot += xAngle;
	yRot += yAngle;
	zRot += zAngle;
	m_renderer->xRot = xRot; 
	m_renderer->yRot = yRot;
	m_renderer->zRot = zRot;
	
	//emit renderRequested();
}

void GLWidget::setClearColor(const QColor &color)
{
	clearColor = color;
	if (m_renderer) m_renderer->clearColor = color; 
}

void GLWidget::mousePressEvent(QMouseEvent *event)
{
	if (event->button() == Qt::LeftButton)
	{
		lastPos = event->pos();
		t_mouseclick_left = clock();

		if (rotate_mode)
		{
			PressPos = event->pos();
			if (isRotate)
				setEnableRotate(false);
		}
	}
}

void GLWidget::setEnableRotate(bool enableRotate)
{
	if (enableRotate)
	{
		rotate_Timer->start(rotate_timer_interval);
		isRotate = true;
	}
	else
	{
		rotate_Timer->stop();
		isRotate = false;
	}
}

#define MOUSE_SHIFT(dx, D)  (int(SHIFT_RANGE*2* float(dx)/D))
#define MOUSE_ROT(dr, D)    (double(MOUSE_SENSITIVE*270* float(dr)/D) *ANGLE_TICK)

void GLWidget::mouseMoveEvent(QMouseEvent *event)
{
	if (event->buttons() & Qt::LeftButton)
	{
		if (enable_move)
		{
			double dx = event->x() - lastPos.x();
			double dy = event->y() - lastPos.y();

			lastPos = event->pos();
			//qDebug()<<"MoveEvent LeftButton";
			double xRotStep = MOUSE_ROT(dy, MIN(viewW, viewH));
			double yRotStep = MOUSE_ROT(dx, MIN(viewW, viewH));

			if (m_renderer)
			{
				m_renderer->viewAnimationRotation(xRotStep, yRotStep, 0);

				//if (m_renderer->m_inited)
				{
					v3d_debug("void GLWidget::mouseMoveEvent -xRotStep:%d yRotStep:%d", xRotStep, yRotStep);
					//m_renderer->lockRenderer();
					emit renderRequested();
				}
			}
			enable_move = false;
		}
	}
}



void GLWidget::mouseReleaseEvent(QMouseEvent *event)
{
	//emit clicked();
	//emit renderRequested();
	if (rotate_mode)
	{
		if (!isRotate)
		{
			if (event->pos().x() != PressPos.x() && event->pos().y() != PressPos.y())
			{
				int t_release = clock();
				double dx = event->pos().x() - PressPos.x();
				double dy = event->pos().y() - PressPos.y();
				if (t_release - t_mouseclick_left < 300)
				{
					dx = dx * rotate_speed;
					dy = dy * rotate_speed;
					Rotate_xStep = MOUSE_ROT(dy, MIN(viewW, viewH));
					Rotate_yStep = MOUSE_ROT(dx, MIN(viewW, viewH));
					setEnableRotate(true);
				}
			}
		}
	}
}

void GLWidget::obj_rotate()
{
	m_renderer->viewAnimationRotation(Rotate_xStep, Rotate_yStep, 0);
	emit renderRequested();
}


void GLWidget::mousemove_enable()
{
	enable_move = true;
}

void GLWidget::wheelEvent(QWheelEvent *event)
{
	int zoomin_sign = -1;  //default
	//setFocus(); // accept KeyPressEvent,

	
#if (QT_VERSION >= QT_VERSION_CHECK(6,0,0))
	float d = (event->pixelDelta().y()) / 100; //QT6
#else 
	float d = (event->delta()) / 100;  // ~480
#endif
	//angleDelta
	//
									   //qDebug("V3dR_GLWidget::wheelEvent->delta = %g",d);
#define MOUSE_ZOOM(dz)    (int(dz*4* MOUSE_SENSITIVE));
#define MOUSE_ZROT(dz)    (int(dz*8* MOUSE_SENSITIVE));

	int zoomStep = MOUSE_ZOOM(d);
	int zRotStep = MOUSE_ZROT(d);

	setZoom((zoomin_sign * zoomStep) + _zoom);  //20170804 RZC: add zoomin_sign in global_setting.b_scrollupZoomin


	event->accept();
}

void GLWidget::setZoom(float zr)
{
	//qDebug("V3dR_GLWidget::setZoom = %i",zr);
	zr = CLAMP(-ZOOM_RANGE, ZOOM_RANGE, zr);
	if (_zoom != zr) {
		_zoom = zr;
		if (m_renderer)
		{
			m_renderer->setZoom(+float(zr) / 100.f * ZOOM_RANGE_RATE); //sign can switch zoom orientation
		}
		emit renderRequested();
	}
}
void GLWidget::paintGL()
{
	v3d_debug("============== GLWidget::paintGL() =================  "); 

}

// This should be the RGB(?)
void GLWidget::setImageStack(My4DImage * imgStack, My4DImage * imgStack_bin2)
{
	if (m_renderer)
	{
		Lit3drParam param; 

		//param.void Renderer::setupData(Lit3drParam param)
		param.image4d = imgStack; 
		param.image4d_bin2 = imgStack_bin2;
		param.b_local = true; 
		//param.
		m_renderer->setupData(param);

		param.image4d = 0; // pass image4d to m_renderer
		param.image4d_bin2 = 0; 
	}
}

void GLWidget::setImageStack(My4DImage * imgStack, My4DImage * imgStack_bin2, My4DImage * imgStack_raw)
{
	if (m_renderer)
	{
		Lit3drParam param;

		//param.void Renderer::setupData(Lit3drParam param)
		param.image4d = imgStack;
		param.image4d_raw = imgStack_raw; //
		param.image4d_bin2 = imgStack_bin2;
		param.b_local = true;
		//param.
		m_renderer->setupData(param);

		param.image4d = 0; // pass image4d to m_renderer
		param.image4d_bin2 = 0;
	}
}

void GLWidget::onAboutToCompose()
{
    // We are on the gui thread here. Composition is about to
    // begin. Wait until the render thread finishes.
	//v3d_debug("============== GLWidget::onAboutToCompose() start =================  ");
#ifdef ENABLE_MT	
	m_renderer->lockRenderer();
#endif
	//v3d_debug("============== GLWidget::onAboutToCompose() end =================  ");

}

void GLWidget::onFrameSwapped()
{
//	v3d_debug("============== GLWidget::onFrameSwapped() start =================  ");

#ifdef ENABLE_MT
   m_renderer->unlockRenderer();
 //   // Assuming a blocking swap, our animation is driven purely by the
 //   // vsync in this example.
	////[Todo] Stop Animation
	//emit renderRequested();
#endif

//   v3d_debug("============== GLWidget::onFrameSwapped() end =================  ");

}

void GLWidget::onAboutToResize()
{
	//v3d_debug("============== GLWidget::onAboutToResize() start =================  ");

	//if (!m_renderer->inRending)
#ifdef ENABLE_MT
    m_renderer->lockRenderer();
#endif
	//v3d_debug("============== GLWidget::onAboutToResize() end =================  ");

}

void GLWidget::onResized()
{
	//v3d_debug("============== GLWidget::onResized() start =================  ");

#ifdef ENABLE_MT
	m_renderer->unlockRenderer();
#endif
	//v3d_debug("============== GLWidget::onResized() end  =================  ");

}

void GLWidget::grabContext()
{
	//v3d_debug("============== GLWidget::grabContext() start =================  ");
#ifdef ENABLE_MT
	m_renderer->lockRenderer();
	QMutexLocker lock(m_renderer->grabMutex()); //QT5
	context()->moveToThread(m_thread);
    m_renderer->grabCond()->wakeAll();
    m_renderer->unlockRenderer();
#endif
	//v3d_debug("============== GLWidget::grabContext() end =================  ");

}

