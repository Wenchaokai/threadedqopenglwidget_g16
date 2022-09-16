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

#ifndef GLWIDGET_H
#define GLWIDGET_H

//#include "renderer.h"

//#include <QtOpenGLWidgets/QOpenGLWidget> //[QT6]???
#include <QOpenGLWidget>
#include "qopenglfunctions_4_3_core.h"
//#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QVector3D>
#include <QMatrix4x4>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QElapsedTimer>

class GLWidget;
class Renderer; 
// include files

#include "lit3dr_common.h"


class GLWidget : public QOpenGLWidget ,public QOpenGLFunctions_4_3_Core
{
    Q_OBJECT
public:
    explicit GLWidget(QWidget *parent = 0);
    ~GLWidget();

	QSize minimumSizeHint() const override;
	QSize sizeHint() const override;
	void rotateBy(int xAngle, int yAngle, int zAngle);
	void setClearColor(const QColor &color);
	void setEnableRotate(bool enableRotate);
protected:
    void paintEvent(QPaintEvent *) override { }
	void mousePressEvent(QMouseEvent *event) override;
	void mouseMoveEvent(QMouseEvent *event) override;
	void mouseReleaseEvent(QMouseEvent *event) override;

	void wheelEvent(QWheelEvent * event) override;

public:
	virtual void initializeGL();
	virtual void resizeGL(int width, int height);
	void setZoom(float zr);
	virtual void paintGL();

	void setImageStack(My4DImage * imgStack, My4DImage * imgStack_bin2);

	void setImageStack(My4DImage * imgStack, My4DImage * imgStack_bin2, My4DImage * imgStack_raw);

signals:
    void renderRequested();
	void clicked();

public slots:
    void grabContext();

private slots:
    void onAboutToCompose();
    void onFrameSwapped();
    void onAboutToResize();
    void onResized();
	void obj_rotate();
	void mousemove_enable();

private:

	int viewW, viewH;

    QThread *m_thread;
    Renderer *m_renderer;

	int t_mouseclick_left;

	QColor clearColor;
	QPoint lastPos;
	int xRot;
	int yRot;
	int zRot;

	float _xRot, _yRot, _zRot, dxRot, dyRot, dzRot;
	// int _zoom, _xShift, _yShift, _zShift, dxShift, dyShift, dzShift;
	float _zoom, _xShift, _yShift, _zShift, dxShift, dyShift, dzShift; // CMB 2011 Feb 07

	bool rotate_mode = true; //true:model rotates when mousemove. false: origin
	bool isRotate = false; //rotate state
	QTimer *rotate_Timer;
	QTimer *mousemove_Timer;
	const int rotate_timer_interval = 40; //rotation refresh rate(every 50ms)
	const int mousemove_timer_interval = 15; //rotation refresh rate(every 50ms)
	const float rotate_speed = 0.05; //rotation speed
	QPoint PressPos;
	int Rotate_xStep;
	int Rotate_yStep;

	bool enable_move = true;
};


#endif
