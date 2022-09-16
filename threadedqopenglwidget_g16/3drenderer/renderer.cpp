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


#include <qmath.h>
#include <iostream>

#include <GL/glew.h>
#include "helper.h"

#include "renderer.h"

//#include <QGuiApplication>
#include <QTimer>
#include <QOpenGLContext>

using namespace std;

#include <QFile> ////QFile
#include "qt_windows.h"
#include <QWindow>
#include <QOpenGLTexture>
#include <QOpenGLShader>
#include "glwidget.h"
#include <QGuiApplication>
//#include <gl/GLU.h>
//#include <GL\glew.h> //[Glew_MK]


// Some OpenGL implementations have serious issues with compiling and linking
// shaders on multiple threads concurrently. Avoid this.
Q_GLOBAL_STATIC(QMutex, initMutex)


inline int CheckGLError_print(const char *file, int line)
{
	GLenum glErr;
	while ((glErr = glGetError()) != GL_NO_ERROR)
	{
		/*const GLubyte* sError = gluErrorString(glErr);
		if (sError)
		std::cerr << "GL Error #" << glErr << "(" << sError << ") " << " in file(" << file << ") at line(" << line << ")\n";
		else
		std::cerr << "GL Error #" << glErr << " (no message available)" << " in file(" << file << ") at line(" << line << ")\n";*/
		std::cerr << "GL Error #" << glErr << " (no message available)" << " in file(" << file << ") at line(" << line << ")\n";

	}
	return glErr;
}

Renderer::Renderer(GLWidget *w)
    : m_inited(false),
      m_glwidget(w),
      m_exiting(false)
{
	init_members(); 
}

Renderer::~Renderer() {

	/*if (m_vbo)
		delete m_vbo;
	if (m_program)
	{
		delete m_program;
	}*/

	//glDeleteFramebuffers(1, &framebuffer);
}

void Renderer::init_members()
{
	bShowBoundingBox = true;
	bShowBoundingBox2 = false;
	bShowAxes = true;
	bOrthoView = false;

	bShowCSline = true;
	bShowFSline = true;
	bXSlice = bYSlice = bZSlice = true;
	bFSlice = true; //100729 RZC: it's not slow, default turn on.
	CSbeta = alpha_threshold = 0;
	//
	CSbeta_high = alpha_threshold_high = 1;
	//
	thickness = 1;
	thicknessX = thicknessY = 1; thicknessZ = 1;

	color_background = XYZW(0, 0, 0, 1); // background color for volume
										 //end

	color_background2 = XYZW(.8f, .85f, .9f, 1); // background color for geometric object only
	color_line = XYZW(.5f, .5f, .7f, 1);
	color_proxy = XYZW(1, 1, 1, 1);

	lineType = 1; lineWidth = 1; nodeSize = 0, rootSize = 5;
	polygonMode = 0;
	tryObjShader = 0;


	b_error = 0;

	//protected -------------------------------------

	xCut0 = yCut0 = zCut0 = -1000000; // no cut
	xCut1 = yCut1 = zCut1 = 1000000;  // no cut
	xClip0 = yClip0 = zClip0 = -1000000; // no clip
	xClip1 = yClip1 = zClip1 = 1000000;  // no clip
	viewClip = 1000000;  // no clip

						 //// perspective view frustum
						 //screenW = screenH = 0;

	screenW = 800;
	screenH = 600;

	viewAngle = 31.0;
	zoomRatio = 1;

	viewNear = 4.0;
	viewFar = 15.0;
	viewDistance = 10;

	b_limitedsize = false;
	depth_buffer = 0;
	for (int i = 0; i<5; i++)
	{
		sampleScale[i] = 1; bufSize[i] = 0;
	}

	SLICE_COLOR = XYZW(1, 1, 1, 1);
	renderMode = rmMaxIntensityProjection;
	imgStack_rgb = 0;

	xRot = yRot = zRot = 0;

	_xShift = _yShift = _zShift = 0; //
	dxShift = dyShift = dzShift = 0;

	VOL_X1 = VOL_Y1 = VOL_Z1 = 1;
	VOL_X0 = VOL_Y0 = VOL_Z0 = 0;

	_xRot = _yRot = _zRot = 0;
	dxRot = dyRot = dzRot = 0;

	clearColor = QColor(1, 0, 0, 1);

	for (int i = 0; i<4; i++)
		for (int j = 0; j<4; j++)
			mRot[i * 4 + j] = (i == j) ? 1 : 0; // Identity matrix


	m_vbo = new QOpenGLBuffer();
	m_program = new QOpenGLShaderProgram(this);

	shader_obj = 0;
	shader_backfill = 0;
	shader_glsl = 0;
	shaderTex2D_grey16 = 0;
	shaderTex2D = 0;
	shaderTex3D = 0;
	shaderObj = 0;

	rotateMat = mat4(1.0f);
	projectionMat = mat4(1.0f);
	viewMat = mat4(1.0f);

	core_model = new CoreModel();
	core_model->Init();

	backfill_model = new BackFillModel();
	backfill_model->Init();
}

void Renderer::initialize()
{
	m_inited = true;

	//initializeOpenGLFunctions(); //[QT5]

	//[Glew_MK]
	GLenum err = glewInit();

	if (GLEW_OK != err)
	{
		/* Problem: glewInit failed, something is seriously wrong. */
		printf("Error: %s\n", glewGetErrorString(err));

	}
	if (__GLEW_ARB_sparse_texture)
	{
		printf("sparse texture is supported\n");
	}
	else
	{
		printf("sparse texture is not supported\n");

	}

	//
	QMutexLocker initLock(initMutex());

	//1. Renderer::initialize

	glDisable(GL_DITHER);
	glDisable(GL_FOG);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	//glEnable(GL_POLYGON_OFFSET_FILL);
	//glPolygonOffset(0, +1); // deal z-fighting, 081121

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	color_background = color_background2;
	//if (rgbaBuf == 0)   color_background = color_background2; // only geometric objects, 081023

	//
	boundingBox = UNIT_BoundingBox;

	//2. Renderer_gl1::initialize()

	loadVol();

	BoundingBox& BB = boundingBox;

	BB.x0 = 0;
	BB.y0 = 0;
	BB.z0 = 0;
	BB.x1 = imageX / sampleScaleX * thicknessX;
	BB.y1 = imageY / sampleScaleY * thicknessY;
	BB.z1 = imageZ / sampleScaleZ * thicknessZ / ZSCALE_REDUCE_ARTIFCAT;

	loadShader_litone_glsl_r_grey16();
}

void Renderer::initialize_grey16()
{
	m_inited = true;

	//[Glew_MK]
	GLenum err = glewInit();

	if (GLEW_OK != err)
	{
		/* Problem: glewInit failed, something is seriously wrong. */
		printf("Error: %s\n", glewGetErrorString(err));
	}
	//稀疏纹理支持性
	if (__GLEW_ARB_sparse_texture)
	{
		printf("sparse texture is supported\n");
	}
	else
	{
		printf("sparse texture is not supported\n");
	}

	printf("Vendor graphic card: %s\n", glGetString(GL_VENDOR));
	printf("Renderer: %s\n", glGetString(GL_RENDERER));
	printf("Version GL: %s\n", glGetString(GL_VERSION));
	printf("Version GLSL: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

	QMutexLocker initLock(initMutex());

	//1. Renderer::initialize

	//关闭颜色抖动和雾化
	glDisable(GL_DITHER);
	glDisable(GL_FOG);

	//片元填充方式
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	//zfight处理
	//glEnable(GL_POLYGON_OFFSET_FILL);
	//glPolygonOffset(0, +1); // deal z-fighting, 081121

	//深度测试
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	color_background = color_background2;

	boundingBox = UNIT_BoundingBox;

	//加载shader
	loadShader_litone_glsl_r_grey16();

	//加载纹理
	loadVol_grey16();

	BoundingBox& BB = boundingBox;

	BB.x0 = 0;
	BB.y0 = 0;
	BB.z0 = 0;
	BB.x1 = imageX / sampleScaleX * thicknessX;
	BB.y1 = imageY / sampleScaleY * thicknessY;
	BB.z1 = imageZ / sampleScaleZ * thicknessZ / ZSCALE_REDUCE_ARTIFCAT;
}

void linkGLShader(cwc::glShaderManager& SMgr,
	cwc::glShader*& shader, //output
	const char* vertex, const char* fragment)
{
	glGetError(); // clear error
	shader = SMgr.loadfromMemory(vertex, fragment);
	if (shader == 0)
	{
		qDebug() << "Renderer_gl2::init:  Error Loading, compiling or linking shader\n";
		throw SMgr.getInfoLog();
	}
}

void Renderer::loadShader_litone_glsl_r_grey16()
{

	cleanShader_glsl_r_grey16(); //090705
	v3d_debug("   Renderer_gl2::loadShader");

	m_glwidget->makeCurrent();

	try 
	{
//Lod模式
#ifdef TEX_LOD
		QString deftexlod = "#define TEX_LOD \n";
#else
		QString deftexlod = "#undef TEX_LOD \n";
#endif

		v3d_debug("+++++++++ shader for Volume texture2D");
		//link shader with shader-manager
		//core_model
		linkGLShader(SMgr, shader_obj,
			Q_CSTR(resourceTextFile(":/shader/obg_grey16.vs")),
#ifndef GREY16_TEXT
			Q_CSTR(resourceTextFile(":/shader/obg_rgb.fs"))
#else
	#ifndef TEXTURE_ARRAY
			Q_CSTR(resourceTextFile(":/shader/obg_grey16.fs"))
	#else
			Q_CSTR(resourceTextFile(":/shader/obg_grey16_texturearray.fs"))
	#endif
#endif
			);
		//backfill_model
		linkGLShader(SMgr, shader_backfill,
			Q_CSTR(resourceTextFile(":/shader/backfill_grey16.vs")),
			Q_CSTR(resourceTextFile(":/shader/backfill_grey16.fs"))
		);
	} CATCH_handler("Renderer_gl2::initialze");
	v3d_debug("+++++++++ GLSL shader setup finished.");

	initColormap();

	glGenTextures(1, &texLuts);
	initLookupTableG16();

	glGenTextures(1, &texHistRange);
	initHistRange();
	//the following codes aim at debugging shader num and uniform var
	GLint count = 0;

	GLint size; // size of the variable
	GLenum type; // type of the variable (float, vec3 or mat4, etc)

	const GLsizei bufSize = 128; // maximum name length
	GLchar name[bufSize]; // variable name in GLSL
	GLsizei length; // name length

	int maxVertexUniform, maxFragmentUniform;
	glGetIntegerv(GL_MAX_VERTEX_UNIFORM_COMPONENTS, &maxVertexUniform);
	glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_COMPONENTS, &maxFragmentUniform);

	printf("maxVertexUniform: %d, maxFragmentUniform:%d\n", maxVertexUniform, maxFragmentUniform );
}

void Renderer::cleanShader_glsl_r_grey16()
{
	v3d_debug("    Renderer_gl2::cleanShader_glsl_r_grey16");
	m_glwidget->makeCurrent();  //ensure right context when multiple views animation or mouse drop

	DELETE_AND_ZERO(shaderTex2D);
	DELETE_AND_ZERO(shaderTex3D);
	DELETE_AND_ZERO(shaderObj);

	if (texColormap) {
		glDeleteTextures(1, &texColormap);
		texColormap = 0;
	}
}

void Renderer::paintQtLogo()
{
	m_vbo->bind();
	m_program->setAttributeBuffer(vertexAttr, GL_FLOAT, 0, 3);
	m_program->setAttributeBuffer(normalAttr, GL_FLOAT, vertices.count() * 3 * sizeof(GLfloat), 3);
	m_vbo->release();

	m_program->enableAttributeArray(vertexAttr);
	m_program->enableAttributeArray(normalAttr);

    glDrawArrays(GL_TRIANGLES, 0, vertices.size());

	m_program->disableAttributeArray(normalAttr);
	m_program->disableAttributeArray(vertexAttr);
}

void Renderer::render()
{
	size_t t0 = clock();

	if (m_exiting)
		return;

	QOpenGLContext *ctx = m_glwidget->context();
	if (!ctx) // QOpenGLWidget not yet initialized
		return;

#ifdef ENABLE_MT
	// Grab the context.
	m_grabMutex.lock();
	emit contextWanted();
	m_grabCond.wait(&m_grabMutex);
	QMutexLocker lock(&m_renderMutex);
	m_grabMutex.unlock();
#endif
	if (m_exiting)
		return;

	Q_ASSERT(ctx->thread() == QThread::currentThread());

	bool calledMakeCurrent = false;

	m_glwidget->makeCurrent();
	calledMakeCurrent = true;

	size_t t0_makeCurrent = clock();

	if (!m_inited) {
		
		QSurfaceFormat format;
		format.setDepthBufferSize(16);
		//*//
		format.setStencilBufferSize(8);
		//*//
		format.setSamples(16);    // Set the number of samples used for multisampling, 

		//*//单双并不影响
		//format.setSwapBehavior(QSurfaceFormat::SwapBehavior::DoubleBuffer); // this is important to avoid the partial frame update 

		//*//
		ctx->setFormat(format);
#ifndef GREY16_TEXT
		initialize(); //This is for Qt OpenGL, non sparse texture.  Good example for sparse texure 
#else
		initialize_grey16(); 
#endif
	}

	size_t t0_paint = clock();
	paint_litone(); // should call initialize() or initialize_litone_sparseTexure_glew(), 

	size_t t1_paint = clock();

	//m_program->release();

	// Make no context current on this thread and move the QOpenGLWidget's
	// context back to the gui thread.

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

#ifdef ENABLE_MT
	size_t finish_t = clock();

	glFinish();

	size_t swap_b = clock();

	//ctx->swapBuffers(ctx->surface());  //  this is also important to avoid the partial frame update during mutlti-thread rendering
#endif

	size_t done_c = clock();

	m_glwidget->doneCurrent();

	size_t t1_paint_swap = clock();

#ifdef ENABLE_MT
	ctx->moveToThread(qGuiApp->thread());

	//ctx->swapBuffers(ctx->surface());

#endif
	// Schedule composition. Note that this will use QueuedConnection, meaning
	// that update() will be invoked on the gui thread.
	inRending = false; 

	size_t t1_paint_move = clock();

	QMetaObject::invokeMethod(m_glwidget, "update");

	size_t t1 = clock();

	v3d_debug("Renderer::render complete and move thread from %p back to %p, time cost total:%d(ms), initial:%d(ms), paint:%d(ms), moveback:%d(ms), swap:%d(ms), moveToThread:%d(ms), update:%d(ms), t0_makeCurrent - t0:%d(ms), t0_paint - t0_makeCurrent:%d(ms)\n",
		QThread::currentThread(), qGuiApp->thread(), t1 - t0, t0_paint - t0, t1_paint - t0_paint, t1 - t1_paint, 
		t1_paint_swap - t1_paint, t1_paint_move - t1_paint_swap, t1- t1_paint_move, t0_makeCurrent - t0, t0_paint - t0_makeCurrent);

#ifdef ENABLE_MT
	v3d_debug("finish: %d(ms), swap_time: %d ,done_current: %d(ms)", swap_b - finish_t, done_c - swap_b, t1_paint_swap - done_c);
#endif
	v3d_debug("swap_time: %d ,done_current: %d(ms)", done_c - t1_paint, t1_paint_swap - done_c);

	/*render_times++;
	v3d_debug("render_times: %d", render_times);*/

}

void Renderer::paint_litone()
{
	glClearColor(1.0, 1.0, 0.0, 1.0); 
	
	//2020/1/26
	glDepthRange(0, 1);

	glClearStencil(0); 
	glClearDepth(1); 

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	//setup view: orth, persective
	glViewport(0, 0, screenW, screenH);

	setProjection();
	setViewPos();

	//*//换函数用model矩阵
	//absolute translation
	/*{

		XYZ T(_xShift, _yShift, _zShift);  				//qDebug("T= %f %f %f", T.x, T.y, T.z);
		dxShift = dyShift = dzShift = 0;  // clear relative shift step

		double s = 1.4 / (float)SHIFT_RANGE;  // 1.4 ~ sqrt(2);
		T = T*s;
		glTranslated(T.x, T.y, T.z);
	}*/
	//*//

	//current relative small rotation, always around center of model
	{
		if (!_animationRotEnabled)
		{
			//XYZ R(dxRot, dyRot, dzRot);  					//qDebug("R= %f %f %f", R.x, R.y, R.z);
			XYZ R(dxRot, -dyRot, -dzRot);	// [Litone] change by jian 2019/12/28. the rotation direction is different

			dxRot = dyRot = dzRot = 0;  // clear relative rotation step

			double angle = norm(R) / (float)ANGLE_TICK;       //qDebug("angle=%f", angle);
			if (angle)
			{
				normalize(R);          						//qDebug("R= %f %f %f", R.x, R.y, R.z);
				glRotated(angle, R.x, R.y, R.z);
			}
		}
		else
		{
			rotateMat = glm::rotate(rotateMat, glm::radians(_angleStep), vec3(_rotVect.x, _rotVect.y, _rotVect.z));
			_angleStep = 0;
			_rotVect.x = _rotVect.y = _rotVect.z = 0;
			dxRot = dyRot = dzRot = 0;
			_animationRotEnabled = false;
		}
	}

	//glScaled(flip_X, flip_Y, flip_Z); // make y-axis downward conformed with image coordinate

	/// [20220518 Jian Trial Clear Buff]

	glDisable(GL_DEPTH_TEST);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //add

	glMatrixMode(GL_MODELVIEW);
	// here, MUST BE normalized space of [-1,+1]^3;

	glGetDoublev(GL_MODELVIEW_MATRIX, volumeViewMatrix); //no scale here, used for drawUnitVolume()

	glPushMatrix();

	glGetIntegerv(GL_VIEWPORT, viewport);            // used for selectObj(smMarkerCreate)
	glGetDoublev(GL_PROJECTION_MATRIX, projectionMatrix);    // used for selectObj(smMarkerCreate)
	glGetDoublev(GL_MODELVIEW_MATRIX, markerViewMatrix);    // used for selectObj(smMarkerCreate)
	glPopMatrix();

	renderVol();
	return;
}

void Renderer::renderVol()
{
	drawVol();
}

void Renderer::setUnitVolumeSpace()
{
	BoundingBox BB = boundingBox;  // a copy
	float DX = BB.Dx();
	float DY = BB.Dy();
	float DZ = BB.Dz();
	float maxD = BB.Dmax();

	float s[3];

	s[0] = DX * flip_X / maxD * 2;
	s[1] = DY * flip_Y / maxD * 2;
	s[2] = DZ * flip_Z / maxD * 2 ;

	glScaled(s[0], s[1] , s[2]);
	glTranslated(-.5, -.5, -.5);
}

//acety
void Renderer::drawBackFillVolCube()
{
	if ((VOL_X1 - VOL_X0 < 0) || (VOL_Y1 - VOL_Y0 < 0) || (VOL_Z1 - VOL_Z0 < 0)) return;

	glPushAttrib(GL_DEPTH_BUFFER_BIT);
	//glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);

	if (isinit < 1)
	{
		backfill_model->InitStack(VOL_X1, VOL_X0, VOL_Y1, VOL_Y0, VOL_Z1, VOL_Z0);
		isinit++;
	}
	backfill_model->rotateModelMatrix(rotateMat);
	backfill_model->CalModelMatrix(boundingBox);
	mat4 modelmat = backfill_model->GetModelMatrix();	

	shader_backfill->begin();
	shader_backfill->setUniformmat("ModelMatrix", GL_FALSE, modelmat, 0);
	shader_backfill->setUniformmat("ViewMatrix", GL_FALSE, viewMat, 1);
	shader_backfill->setUniformmat("ProjectionMatrix", GL_FALSE, projectionMat, 2);
	backfill_model->Render();
	shader_backfill->end();
	
	glPopAttrib();
}

void Renderer::drawCrossLine(float lineWidth)
{
	glPushAttrib(GL_LINE_BIT | GL_POLYGON_BIT);
	//	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	//	glEnable(GL_POLYGON_OFFSET_LINE);
	//	glPolygonOffset(-1, -1); // deal z-fighting, 081121

	//	float d = 0.0001f; // here is unit volume
	//	glDepthRange(0, 1-d);

	glLineStipple(1, 0x7070);
	glEnable(GL_LINE_STIPPLE);

	glLineWidth(lineWidth); // work only before glBegin(GL_LINES), by RZC 080827
							//glBegin(GL_QUADS);
	glBegin(GL_LINES);
	{
		glColor3fv(color_line.c);
		//x
		glVertex3f(0, VOL_Y0, VOL_Z0);		glVertex3f(1, VOL_Y0, VOL_Z0);
		//glVertex3f(1, VOL_Y0+d, VOL_Z0+d);	glVertex3f(0, VOL_Y0+d, VOL_Z0+d);
		//y
		glVertex3f(VOL_X0, 0, VOL_Z0);		glVertex3f(VOL_X0, 1, VOL_Z0);
		//glVertex3f(VOL_X0+d, 1, VOL_Z0+d);	glVertex3f(VOL_X0+d, 0, VOL_Z0+d);
		//z
		glVertex3f(VOL_X0, VOL_Y0, 0);		glVertex3f(VOL_X0, VOL_Y0, 1);
		//glVertex3f(VOL_X0+d, VOL_Y0+d, 1);	glVertex3f(VOL_X0+d, VOL_Y0+d, 0);
	}
	glEnd();

	//	glDepthRange(0, 1);
	//glLineWidth(1);
	//glDisable(GL_LINE_STIPPLE);
	glPopAttrib();
}

void Renderer::equAlphaBlendingProjection()
{
	//[QT5_MK]
	//glBlendEquationEXT(GL_FUNC_ADD_EXT);//this is important
	glBlendEquation(GL_FUNC_ADD_EXT);//this is important
									 //[End]
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);	// back to front when depth-test on, A for all of RGB
														//glBlendFunc(GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR);	// back to front when depth-test on, RGBA separated, strange color
}

void Renderer::equMaxIntensityProjection()
{
	//[QT5_MK]
	glBlendEquationEXT(GL_MAX_EXT);    //seems not be controlled by GL_BLEND
	//glBlendEquation(GL_MAX_EXT);    //seems not be controlled by GL_BLEND
									//[End]
}

void Renderer::equMinIntensityProjection()
{
	//[QT5_MK]
	glBlendEquationEXT(GL_MIN_EXT);    //seems not be controlled by GL_BLEND
	//glBlendEquation(GL_MIN_EXT);    //seems not be controlled by GL_BLEND

									//[End]
}

void Renderer::equCrossSection()
{
	//[QT5_MK]
	//glBlendEquationEXT(GL_FUNC_ADD_EXT);//this is important
	//glBlendColorEXT(1, 1, 1, 1 - CSbeta);
	glBlendEquation(GL_FUNC_ADD_EXT);//this is important
	glBlendColor(1, 1, 1, 1 - CSbeta);
	//[End]
	glBlendFunc(GL_CONSTANT_ALPHA, GL_ONE_MINUS_CONSTANT_ALPHA); // constant Alpha
																 //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);	// back to front when depth-test on, Alpha for all of RGB
}

void Renderer::drawVol()
{
	//---------------------------------------
	if (CSbeta >= 1) return; // 081202, avoid be hit when invisible

	float af = 3.0f;
	//1.0f;
	alpha_threshold = pow(double(CSbeta), double(af));
	//[Litone]
	alpha_threshold_high = pow(double(CSbeta_high), double(af)); //add by litone 2019/11/21
																 //
	color_proxy.a = pow(1.0 - double(CSbeta), double(1.0 / af)); //change to double() by PHC, 2010-05-20
	SLICE_COLOR = color_proxy;

	switch (renderMode)
	{
	case rmAlphaBlendingProjection:
		glEnable(GL_BLEND);      equAlphaBlendingProjection();
		glEnable(GL_ALPHA_TEST);
		//[Litone] 2019/11/21
		//glAlphaFunc(GL_GREATER, alpha_threshold); // > threshold Alpha
		glAlphaFunc(GL_GREATER, alpha_threshold); // > threshold Alpha
		glAlphaFunc(GL_LEQUAL, alpha_threshold_high); // >= threshold Alpha, 080930
													  //
		break;

	case rmMaxIntensityProjection:
		//if (has_image() && !b_renderTextureLast) // if rendering texture first, we can clear - otherwise this is done in prepareVol()
		{
			glColor3f(0, 0, 0);
			drawBackFillVolCube(); // clear the project region to zero for MIP
		}
		glEnable(GL_BLEND);      equMaxIntensityProjection();
		glEnable(GL_ALPHA_TEST);
		//[Litone] 2019/11/21
		//glAlphaFunc(GL_GEQUAL, alpha_threshold); // >= threshold Alpha, 080930
		glAlphaFunc(GL_GEQUAL, alpha_threshold); // >= threshold Alpha, 080930
		glAlphaFunc(GL_LEQUAL, alpha_threshold_high); // >= threshold Alpha, 080930
													  //[End]
		break;

	case rmMinIntensityProjection:
		//if (has_image() && !b_renderTextureLast) // if rendering texture first, we can clear - otherwise this is done in prepareVol()
		{
			glColor3f(0.8, 0.8, 0.8);
			drawBackFillVolCube(); // clear the project region to a high gray-level value for MIP (black won't do fine)
		}
		glEnable(GL_BLEND);      equMinIntensityProjection();
		glEnable(GL_ALPHA_TEST);
		//[Litone] 2019/11/21
		//glAlphaFunc(GL_LEQUAL, 1 - alpha_threshold); // >= threshold Alpha, 080930
		glAlphaFunc(GL_LEQUAL, 1 - alpha_threshold);
		glAlphaFunc(GL_GEQUAL, 1 - alpha_threshold_high);
		//[End]
		break;

	case rmCrossSection:
		break;

	default: // unknown ?
		break;
	}

	// modulate by color_proxy, by RZC 080930
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE); // this is default, but make sure;
																 //glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glColor4fv(SLICE_COLOR.c);

	glDisable(GL_LIGHTING); //volume lighting is difference from polygon object

	glShadeModel(GL_FLAT); //flat for slice rectangle
	{
		updateVolCutRange();
		glEnable(GL_TEXTURE_2D); //glBindTexture(GL_TEXTURE_2D, 0);

		drawUnitVolume(); // drawStackX/Y/Z

		glDisable(GL_TEXTURE_2D);

	}
	glShadeModel(GL_SMOOTH);

	glDisable(GL_BLEND); //090429 RZC: no effect to glBlendEquationEXT(GL_MAX_EXT), must set to GL_FUNC_ADD_EXT
	//[QT5_MK]
	//glBlendEquationEXT(GL_FUNC_ADD_EXT);
	glBlendEquation(GL_FUNC_ADD_EXT);
	//[End]
	glDisable(GL_ALPHA_TEST);

}

void Renderer::updateVolCutRange()
{
	//	Renderer::updateVolCutRange();
	xCut0 = CLAMP(0, imageX - 1, xCut0);
	yCut0 = CLAMP(0, imageY - 1, yCut0);
	zCut0 = CLAMP(0, imageZ - 1, zCut0);
	xCut1 = CLAMP(0, imageX - 1, xCut1);
	yCut1 = CLAMP(0, imageY - 1, yCut1);
	zCut1 = CLAMP(0, imageZ - 1, zCut1);

	// [0,1] range cut box
	VOL_X0 = (imageX - 1 <= 0) ? 0 : float(xCut0) / (imageX - 1);
	VOL_X1 = (imageX - 1 <= 0) ? 0 : float(xCut1) / (imageX - 1);
	VOL_Y0 = (imageY - 1 <= 0) ? 0 : float(yCut0) / (imageY - 1);
	VOL_Y1 = (imageY - 1 <= 0) ? 0 : float(yCut1) / (imageY - 1);
	VOL_Z0 = (imageZ - 1 <= 0) ? 0 : float(zCut0) / (imageZ - 1);
	VOL_Z1 = (imageZ - 1 <= 0) ? 0 : float(zCut1) / (imageZ - 1);

	if (renderMode == rmCrossSection)
	{
		VOL_X1 = VOL_X0;
		VOL_Y1 = VOL_Y0;
		VOL_Z1 = VOL_Z0;
		//100801
		dataViewProcBox = BoundingBox(start1, start2, start3,
			start1 + (size1 - 1), start2 + (size2 - 1), start3 + (size3 - 1));
	}
	else
	{
		
		dataViewProcBox = BoundingBox(start1 + VOL_X0*(size1 - 1), start2 + VOL_Y0*(size2 - 1), start3 + VOL_Z0*(size3 - 1),
			start1 + VOL_X1*(size1 - 1), start2 + VOL_Y1*(size2 - 1), start3 + VOL_Z1*(size3 - 1));
	}
}

void Renderer::drawUnitVolume()
{
	//if (!rgbaBuf || bufSize[3] < 1) return; // no image data, 081002
#ifndef GREY16_TEXT
	if (!imgStack_rgb || bufSize[3] < 1) return; // no image data, 081002
#else
	if (!imgStack_raw_StackZ || bufSize[3] < 1) return; // no image data, 081002

#endif
	bool b_stream = false;
	bool b_tex3d = false;

	{
		realX = safeX;
		realY = safeY;
		realZ = safeZ;
	}
	fillX = _getTexFillSize(realX);
	fillY = _getTexFillSize(realY);
	fillZ = _getTexFillSize(realZ);

	if (realX <= 0 || realY <= 0 || realZ <= 0) return; //081026 to prevent drawing before subloadTex

	double mm[4][4];
	MAT16_TO_MAT4x4(volumeViewMatrix, mm);

	double ax, ay, az;
	ax = (mm[0][2] - mm[0][3]) / (mm[3][2] - mm[3][3]);
	ay = (mm[1][2] - mm[1][3]) / (mm[3][2] - mm[3][3]);
	az = (mm[2][2] - mm[2][3]) / (mm[3][2] - mm[3][3]);

	xCut1 = realX; 
	yCut1 = realY; 
	zCut1 = realZ;
	xCut0 = yCut0 = zCut0 = 0;

#ifndef GREY16_TEXT
	shaderTexBegin_glsl(b_stream);
#else
	shaderTexBegin_glsl_grey16(b_stream);
#endif
	{

		int section = (renderMode == rmCrossSection);
		{
#ifndef GREY16_TEXT
			drawStackZ(az, section, b_tex3d, b_stream);
#else
			int channels = imgStack_raw_StackZ->getCDim(); 
			drawStackZ_grey16(az, section, b_tex3d, b_stream, channels);
#endif
		}
	}

#ifndef GREY16_TEXT
	shaderTexEnd_glsl(); 
#else
	shaderTexEnd_glsl_grey16();
#endif
}

void Renderer::shaderTexBegin_glsl(bool stream)
{
	//shader_glsl = (texture_unit0_3D && !stream) ? shaderTex3D : shaderTex2D;
	shader_glsl = shader_obj;

	//int format_bgra = (stream && pbo_image_format == GL_BGRA) ? 1 : 0;
	int format_bgra = 0;
	//if (IF_VOL_SHADER)
	{
		shader_glsl->begin(); //must before setUniform
		shader_glsl->setUniform1i("volume", 0); //GL_TEXTURE0
		glActiveTextureARB(GL_TEXTURE1_ARB);
		glBindTexture(GL_TEXTURE_2D, texColormap);
		shader_glsl->setUniform1i("colormap", 1); //GL_TEXTURE1

		float n = FILL_CHANNEL - 1; // 0-based
		shader_glsl->setUniform3f("channel", 0 / n, 1 / n, 2 / n);
		//shader_glsl->setUniform1i("blend_mode", renderMode);
		//shader_glsl->setUniform1i("format_bgra", format_bgra);
	}
}

struct RGBAui 
{
	unsigned int r, g, b, a;
};

RGBAui mLut[2][3];


void Renderer::shaderTexBegin_glsl_grey16(bool stream)
{
	shader_glsl = shader_obj;

	int format_bgra = 0;
	//if (IF_VOL_SHADER)
	{
		shader_glsl->begin(); //must before setUniform

		size_t colormap_time = clock();

		glActiveTextureARB(GL_TEXTURE0_ARB);
		glBindTexture(GL_TEXTURE_2D, texColormap);
		shader_glsl->setUniform1i("colormap", 0); //GL_TEXTURE0

		size_t channel_time = clock();

		CHECK_GLError_print();
		shader_glsl->setUniform1i("channel0", 1); //GL_TEXTURE1
		shader_glsl->setUniform1i("channel1", 2); //GL_TEXTURE2
		shader_glsl->setUniform1i("channel2", 3); //GL_TEXTURE3
		
		CHECK_GLError_print();

		size_t c_count_time = clock();

		float n = FILL_CHANNEL - 1; // 0-based
		shader_glsl->setUniform3f("channel", 0 / n, 1 / n, 2 / n);

		size_t blendmode_time = clock();
	
		shader_glsl->setUniform1i("blend_mode", renderMode);
		shader_glsl->setUniform1i("format_bgra", format_bgra);
		CHECK_GLError_print();

		size_t luts_time = clock();

		glActiveTextureARB(GL_TEXTURE4_ARB);
		glBindTexture(GL_TEXTURE_2D, texLuts);
		shader_glsl->setUniform1i("texLuts", 4); //GL_TEXTURE4

		size_t hist_time = clock();

		glActiveTextureARB(GL_TEXTURE5_ARB);
		glBindTexture(GL_TEXTURE_2D, texHistRange);
		shader_glsl->setUniform1i("texHistRange", 5); //GL_TEXTURE5

		size_t end_time = clock();

		v3d_debug("colormap time = %d, channel time = %d, channel_count_time time = %d, blendmode_time time = %d, luts time = %d, hist time = %d",
			channel_time - colormap_time, c_count_time - channel_time, blendmode_time - c_count_time, luts_time - blendmode_time, hist_time - luts_time, end_time - hist_time);
	}
}

void Renderer::shaderTexEnd_glsl()
{
	//if (IF_VOL_SHADER)
	{
		// off colormap
		//glActiveTextureARB(GL_TEXTURE1_ARB);
		//[QT5_MK]
		glActiveTexture(GL_TEXTURE1);
		//
		glDisable(GL_TEXTURE_2D);
		//glActiveTextureARB(GL_TEXTURE0_ARB);
		//[QT5_MK]
		glActiveTexture(GL_TEXTURE0);
		//

	}
	//shader = 0;
}

void Renderer::shaderTexEnd_glsl_grey16()
{
	//if (IF_VOL_SHADER)
	{
		// off colormap
		//glActiveTextureARB(GL_TEXTURE1_ARB);
		//[QT5_MK]
		glActiveTexture(GL_TEXTURE0);
		//
		glDisable(GL_TEXTURE_2D);
		//glActiveTextureARB(GL_TEXTURE0_ARB);
		//[QT5_MK]
		glActiveTexture(GL_TEXTURE1);
		//

	}
	//shader = 0;
}

void Renderer::drawStackZ(float direction, int section, bool t3d, bool stream)
{
	double ts = double(realZ) / fillZ / ZSCALE_REDUCE_ARTIFCAT;
	double th = double(realY) / fillY;
	double tw = double(realX) / fillX;
	double s0 = VOL_Z0;
	double s1 = VOL_Z1 / ZSCALE_REDUCE_ARTIFCAT;
	double h0 = VOL_Y0;
	double h1 = VOL_Y1;
	double w0 = VOL_X0;
	double w1 = VOL_X1;
	double ds = (realZ <= 1) ? 0 : (1.f / (realZ - 1));
	int slice0 = s0*((realZ <= 1) ? 0 : realZ - 1); //zCut0;
	int slice1 = s1*ZSCALE_REDUCE_ARTIFCAT*((realZ <= 1) ? 0 : realZ - 1); //zCut1;
																		   //slice0 = 0; 	Zslice_data = imageZ-1;
	ds /= ZSCALE_REDUCE_ARTIFCAT;

	core_model->UpdateParam(ts, th, tw, s0, s1, h0, h1, w0, w1, ds, slice0, slice1, int(thicknessZ),
		Ztex_list, (1),
		direction, section, 0);

	size_t t0 = clock();
	if (isinit < 2)
	{
		core_model->InitStack();
		core_model->SetTextureId(textureID_rgb);
		isinit++;
	}

	size_t t1 = clock();

	v3d_debug("initstack = %d", t1 - t0);
	core_model->rotateModelMatrix(rotateMat);
	core_model->CalModelMatrix(boundingBox);
	mat4 modelmat = core_model->GetModelMatrix();

	shader_obj->begin();
	shader_obj->setUniformmat("ModelMatrix", GL_FALSE, modelmat, 0);
	shader_obj->setUniformmat("ViewMatrix", GL_FALSE, viewMat, 1);
	shader_obj->setUniformmat("ProjectionMatrix", GL_FALSE, projectionMat, 2);
	//关闭所用的shader

	size_t t2 = clock();
	core_model->Render();
	size_t t3 = clock();
	v3d_debug("rendertime = %d", t3 - t2);
	shader_obj->end();
}

void Renderer::drawStackZ_grey16(float direction, int section, bool t3d, bool stream, int channels)
{
	double ts = double(realZ) / fillZ / ZSCALE_REDUCE_ARTIFCAT;
	double th = double(realY) / fillY;
	double tw = double(realX) / fillX;
	double s0 = VOL_Z0;
	double s1 = VOL_Z1 / ZSCALE_REDUCE_ARTIFCAT;
	double h0 = VOL_Y0;
	double h1 = VOL_Y1;
	double w0 = VOL_X0;
	double w1 = VOL_X1;
	double ds = (realZ <= 1) ? 0 : (1.f / (realZ - 1));
	int slice0 = s0*((realZ <= 1) ? 0 : realZ - 1); //zCut0;
	int slice1 = s1*ZSCALE_REDUCE_ARTIFCAT*((realZ <= 1) ? 0 : realZ - 1); //zCut1;
																		   //slice0 = 0; 	Zslice_data = imageZ-1;
	ds /= ZSCALE_REDUCE_ARTIFCAT;

	core_model->UpdateParam(ts, th, tw, s0, s1, h0, h1, w0, w1, ds, slice0, slice1, int(thicknessZ),
		Ztex_list, (1),
		direction, section, channels);

	size_t t0 = clock();
	if (isinit < 2)
	{
		core_model->InitStack();
#ifdef TEXTURE_ARRAY
		for (int ch = 0; ch < channels; ch++)
		{
			core_model->SetTextureId(textureID[ch], ch);	
		}
#endif
		isinit++;
	}

	size_t t1 = clock();

	v3d_debug("initstack = %d", t1- t0);
	core_model->rotateModelMatrix(rotateMat);
	core_model->CalModelMatrix(boundingBox);
	mat4 modelmat = core_model->GetModelMatrix();

	shader_obj->begin();
	shader_obj->setUniformmat("ModelMatrix", GL_FALSE, modelmat, 0);
	shader_obj->setUniformmat("ViewMatrix", GL_FALSE, viewMat, 1);
	shader_obj->setUniformmat("ProjectionMatrix", GL_FALSE, projectionMat, 2);
	//关闭所用的shader
	
	size_t t2 = clock();
	core_model->Render();
	size_t t3 = clock();
	v3d_debug("rendertime = %d", t3 - t2);
	shader_obj->end();
}

void Renderer::drawStackY_grey16(float direction, int section, bool t3d, bool stream, int channels)
{
	double ts = double(realY) / fillY;
	double th = double(realZ) / fillZ;
	double tw = double(realX) / fillX;
	double s0 = VOL_Y0;
	double s1 = VOL_Y1;
	double h0 = VOL_Z0;
	double h1 = VOL_Z1;
	double w0 = VOL_X0;
	double w1 = VOL_X1;
	double ds = (realY <= 1) ? 0 : (1.f / (realY - 1));
	int slice0 = s0*((realY <= 1) ? 0 : realY - 1); //yCut0;
	int slice1 = s1*((realY <= 1) ? 0 : realY - 1); //yCut1;
													//slice0 = 0; 	Zslice_data = imageY-1;

													//SET_Priority(2);
	glPushName(vsYslice);
	_drawStack_grey16(ts, th, tw, s0, s1, h0, h1, w0, w1, ds, slice0, slice1, int(thicknessY),
		tex3D, Ytex_list, (2),
		direction, section, t3d, stream, channels);
	glPopName();
}

void Renderer::drawStackX_grey16(float direction, int section, bool t3d, bool stream, int channels)
{
	double ts = double(realX) / fillX;
	double th = double(realZ) / fillZ;
	double tw = double(realY) / fillY;
	double s0 = VOL_X0;
	double s1 = VOL_X1;
	double h0 = VOL_Z0;
	double h1 = VOL_Z1;
	double w0 = VOL_Y0;
	double w1 = VOL_Y1;
	double ds = (realX <= 1) ? 0 : (1.f / (realX - 1));
	int slice0 = s0*((realX <= 1) ? 0 : realX - 1); //xCut0;
	int slice1 = s1*((realX <= 1) ? 0 : realX - 1); //xCut1;
													//slice0 = 0; 	Zslice_data = imageX-1;
													//	v3d_debug("Renderer::drawStackX ts:%f, th:%f, tw:%f, [s0:%f,s1:%f], [h0:%f,h1:%f], [w0:%f,w1:%f], ds:%f ", ts, th, tw, s0, s1, h0, h1, w0, w1, ds);

													//SET_Priority(3);
	glPushName(vsXslice);
	_drawStack_grey16(ts, th, tw, s0, s1, h0, h1, w0, w1, ds, slice0, slice1, int(thicknessX),
		tex3D, Xtex_list, (3),
		direction, section, t3d, stream, channels);
	glPopName();
}

void Renderer::_drawStack_grey16(double ts, double th, double tw,
	double s0, double s1, double h0, double h1, double w0, double w1,
	double ds, int slice0, int slice1, int thickness,
	GLuint tex3D, GLuint texs[], int stack_i,
	float direction, int section, bool b_tex3d, bool b_stream, int channels)
{
	//v3d_debug("		s(%g-%g)h(%g-%g)w(%g-%g)", s0,s1, h0,h1, w0,w1);
	if ((s1 - s0 < 0) || (h1 - h0 < 0) || (w1 - w0 < 0)) return; // no draw
	if (thickness < 1) return; // only support thickness>=1

	if (section > 0) { // cross-section
		h0 = 0;		h1 = 1;
		w0 = 0;		w1 = 1;
		s1 = s0;
		slice1 = slice0;
	}
	/*
	srand((unsigned)time(NULL));

	int half = rand() % 2;
	int saveSlice0 = slice0;
	int saveSlice1 = slice1;
	if (half == 0)
	{
	s1 = s1 / 2;
	slice1 = slice1 / 2;
	}
	else
	{
	s0 = s1 / 2;
	slice0 = slice1 / 2;
	}*/
	//v3d_debug("Renderer::_drawStack half:%d, slice0/1:[%d, %d]", half, slice0, slice1);
	//	double moreslice = ((tex3D)? 4 : 1); // 081009: more slice for tex3D
	double step, slice, s;
	if (direction < 0) {
		step = (+1); ///moreslice);
		slice = slice0;
		s = s0;
	}
	else {
		step = (-1); ///moreslice);
		slice = slice1;
		s = s1;
	}

	double tw0 = tw*w0;  double tw1 = tw*w1;
	double th0 = th*h0;  double th1 = th*h1;

	for (int ch = 0; ch < channels; ch++)
	{
		glActiveTexture(GL_TEXTURE1 + ch);

		for (;
			slice0 <= slice && slice <= slice1;
			slice += step, s += step * ds
			)
		{
			if (!b_tex3d)
			{
				int slice_g16 = slice*channels;

				glBindTexture(GL_TEXTURE_2D, texs[int(slice_g16 + ch) + 1]); //[0] reserved for pbo tex
				setTexParam2D();
			}

			double tss = ts*s;
			int k_repeat = thickness;
			if ((step > 0 && slice == slice1)
				|| (step < 0 && slice == slice0)
				)  k_repeat = 1; // 081106

			for (int k = 0; k < k_repeat; k++) // 081105
			{
				double ids = step * k*ds / thickness;
				double idts = ts*ids;

				glBegin(GL_QUADS);

				if (!b_tex3d)  glTexCoord2d(tw0, th0);
				else if (stack_i == 1) glTexCoord3d(tw0, th0, tss + idts);
				else if (stack_i == 2) glTexCoord3d(tw0, tss + idts, th0);
				else if (stack_i == 3) glTexCoord3d(tss + idts, tw0, th0);
				if (stack_i == 1) glVertex3d(w0, h0, s + ids);
				else if (stack_i == 2) glVertex3d(w0, s + ids, h0);
				else if (stack_i == 3) glVertex3d(s + ids, w0, h0);

				if (!b_tex3d)  glTexCoord2d(tw1, th0);
				else if (stack_i == 1) glTexCoord3d(tw1, th0, tss + idts);
				else if (stack_i == 2) glTexCoord3d(tw1, tss + idts, th0);
				else if (stack_i == 3) glTexCoord3d(tss + idts, tw1, th0);
				if (stack_i == 1) glVertex3d(w1, h0, s + ids);
				else if (stack_i == 2) glVertex3d(w1, s + ids, h0);
				else if (stack_i == 3) glVertex3d(s + ids, w1, h0);

				if (!b_tex3d)  glTexCoord2d(tw1, th1);
				else if (stack_i == 1) glTexCoord3d(tw1, th1, tss + idts);
				else if (stack_i == 2) glTexCoord3d(tw1, tss + idts, th1);
				else if (stack_i == 3) glTexCoord3d(tss + idts, tw1, th1);
				if (stack_i == 1) glVertex3d(w1, h1, s + ids);
				else if (stack_i == 2) glVertex3d(w1, s + ids, h1);
				else if (stack_i == 3) glVertex3d(s + ids, w1, h1);

				if (!b_tex3d)  glTexCoord2d(tw0, th1);
				else if (stack_i == 1) glTexCoord3d(tw0, th1, tss + idts);
				else if (stack_i == 2) glTexCoord3d(tw0, tss + idts, th1);
				else if (stack_i == 3) glTexCoord3d(tss + idts, tw0, th1);
				if (stack_i == 1) glVertex3d(w0, h1, s + ids);
				else if (stack_i == 2) glVertex3d(w0, s + ids, h1);
				else if (stack_i == 3) glVertex3d(s + ids, w0, h1);

				glEnd();
			}
		}


		//if (b_stream) _streamTex_end();
	}

}

void Renderer::createGeometry()
{
    vertices.clear();
    normals.clear();

    qreal x1 = +0.06f;
    qreal y1 = -0.14f;
    qreal x2 = +0.14f;
    qreal y2 = -0.06f;
    qreal x3 = +0.08f;
    qreal y3 = +0.00f;
    qreal x4 = +0.30f;
    qreal y4 = +0.22f;

    quad(x1, y1, x2, y2, y2, x2, y1, x1);
    quad(x3, y3, x4, y4, y4, x4, y3, x3);

    extrude(x1, y1, x2, y2);
    extrude(x2, y2, y2, x2);
    extrude(y2, x2, y1, x1);
    extrude(y1, x1, x1, y1);
    extrude(x3, y3, x4, y4);
    extrude(x4, y4, y4, x4);
    extrude(y4, x4, y3, x3);

    const int NumSectors = 100;
    const qreal sectorAngle = 2 * qreal(M_PI) / NumSectors;

    for (int i = 0; i < NumSectors; ++i) {
        qreal angle = i * sectorAngle;
        qreal x5 = 0.30 * sin(angle);
        qreal y5 = 0.30 * cos(angle);
        qreal x6 = 0.20 * sin(angle);
        qreal y6 = 0.20 * cos(angle);

        angle += sectorAngle;
        qreal x7 = 0.20 * sin(angle);
        qreal y7 = 0.20 * cos(angle);
        qreal x8 = 0.30 * sin(angle);
        qreal y8 = 0.30 * cos(angle);

        quad(x5, y5, x6, y6, x7, y7, x8, y8);

        extrude(x6, y6, x7, y7);
        extrude(x8, y8, x5, y5);
    }

    for (int i = 0;i < vertices.size();i++)
        vertices[i] *= 2.0f;
}

void Renderer::quad(qreal x1, qreal y1, qreal x2, qreal y2, qreal x3, qreal y3, qreal x4, qreal y4)
{
    vertices << QVector3D(x1, y1, -0.05f);
    vertices << QVector3D(x2, y2, -0.05f);
    vertices << QVector3D(x4, y4, -0.05f);

    vertices << QVector3D(x3, y3, -0.05f);
    vertices << QVector3D(x4, y4, -0.05f);
    vertices << QVector3D(x2, y2, -0.05f);

    QVector3D n = QVector3D::normal
        (QVector3D(x2 - x1, y2 - y1, 0.0f), QVector3D(x4 - x1, y4 - y1, 0.0f));

    normals << n;
    normals << n;
    normals << n;

    normals << n;
    normals << n;
    normals << n;

    vertices << QVector3D(x4, y4, 0.05f);
    vertices << QVector3D(x2, y2, 0.05f);
    vertices << QVector3D(x1, y1, 0.05f);

    vertices << QVector3D(x2, y2, 0.05f);
    vertices << QVector3D(x4, y4, 0.05f);
    vertices << QVector3D(x3, y3, 0.05f);

    n = QVector3D::normal
        (QVector3D(x2 - x4, y2 - y4, 0.0f), QVector3D(x1 - x4, y1 - y4, 0.0f));

    normals << n;
    normals << n;
    normals << n;

    normals << n;
    normals << n;
    normals << n;
}

void Renderer::extrude(qreal x1, qreal y1, qreal x2, qreal y2)
{
    vertices << QVector3D(x1, y1, +0.05f);
    vertices << QVector3D(x2, y2, +0.05f);
    vertices << QVector3D(x1, y1, -0.05f);

    vertices << QVector3D(x2, y2, -0.05f);
    vertices << QVector3D(x1, y1, -0.05f);
    vertices << QVector3D(x2, y2, +0.05f);

    QVector3D n = QVector3D::normal
        (QVector3D(x2 - x1, y2 - y1, 0.0f), QVector3D(0.0f, 0.0f, -0.1f));

    normals << n;
    normals << n;
    normals << n;

    normals << n;
    normals << n;
    normals << n;
}

void Renderer::initColormap()
{
	v3d_debug("   Renderer::initColormap");

	for (int ch = 0; ch<N_CHANNEL; ch++)
	{
		for (int i = 0; i<256; i++) // intensity
		{
			for (int j = 0; j<3; j++)  colormap[ch][i].c[j] = (ch == j) * 255;	colormap[ch][i].a = i;
			//for (int j=0; j<3; j++)  colormap[ch][i].c[j] = (ch!=j)*i;	colormap[ch][i].a = i;
		}

		for (int j = 0; j<4; j++) // RGBA
		{
			colormap_curve[ch][j].clear();
			int y;
			y = colormap[ch][0].c[j];	   set_colormap_curve(colormap_curve[ch][j], 0.0, y);
			y = colormap[ch][255].c[j];    set_colormap_curve(colormap_curve[ch][j], 1.0, y);
		}
	}

	glGenTextures(1, &texColormap);
	glBindTexture(GL_TEXTURE_2D, texColormap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); // MUST use nearest filter
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); // MUST use nearest filter

																	   // load texture
	glTexImage2D(GL_TEXTURE_2D, // target
		0, // level
		GL_RGBA, // texture format
		256, // width
		FILL_CHANNEL,   // height
		0, // border
		GL_RGBA, // image format
		GL_UNSIGNED_BYTE, // image type
		&colormap[0][0]);
	//NULL); ////////////////////////////////////// load ON DEMAND when drawing

}

//inline
void make_linear_lut_one(RGBA8 color, QVector<RGBA8>& lut)
{
	assert(lut.size() == 256); //////// must be
	for (int j = 0; j<256; j++)
	{
		float f = j / 255.0;  //110801 fixed the bug of max=254
		lut[j].r = color.r *f;
		lut[j].g = color.g *f;
		lut[j].b = color.b *f;
		lut[j].a = color.a;   //only alpha is constant
	}
}

//inline
RGB8 lookup_mix_litone(unsigned char* mC, int N, unsigned int* channel_index_On, int channels_On, RGBA16** mLut, int op, RGB8 mask)
{

#define R(k) (mLut[k][ mC[k] ].r)
#define G(k) (mLut[k][ mC[k] ].g)
#define B(k) (mLut[k][ mC[k] ].b)
#define A(k) (mLut[k][ mC[k] ].a)
	//[PM] can save 100 ms
	//#define AR(k) (A(k)*R(k))
	//#define AG(k) (A(k)*G(k))
	//#define AB(k) (A(k)*B(k))
#define AR(k) (R(k))
#define AG(k) (G(k))
#define AB(k) (B(k))
	//end

	//int N = mC.size();
	//assert(N <= mLut.size());
	//printf("lookup_mix:N:%d, mLut.size():%d\n", N, mLut.size()); 
	unsigned int o1, o2, o3;

	int k0 = channel_index_On[0];

	o1 = mLut[k0][mC[k0]].r;
	o2 = mLut[k0][mC[k0]].g;
	o3 = mLut[k0][mC[k0]].b;

	//
	if (N == 1)
	{
		int clamp_max = 65536;

		/*o1 = CLAMP(0, clamp_max, o1)/255;
		o2 = CLAMP(0, clamp_max, o2)/255;
		o3 = CLAMP(0, clamp_max, o3)/255;*/

		o1 = CLAMP(0, clamp_max, o1) >> 8;
		o2 = CLAMP(0, clamp_max, o2) >> 8;
		o3 = CLAMP(0, clamp_max, o3) >> 8;

		RGB8 oC;

		oC.r = o1;
		oC.g = o2;
		oC.b = o3;
		oC.r &= mask.r;
		oC.g &= mask.g;
		oC.b &= mask.b;
		return oC;

	}
	//end
	if (op == OP_MAX)
	{
		//for (int k = 0; k<N; k++)
		for (int k_on = 1; k_on<channels_On; k_on++)
		{
			int k = channel_index_On[k_on];
			o1 = MAX(o1, AR(k));
			o2 = MAX(o2, AG(k));
			o3 = MAX(o3, AB(k));
		}
	}
	else if (op == OP_SUM)
	{
		//for (int k = 0; k<N; k++)
		for (int k_on = 1; k_on<channels_On; k_on++)
		{
			int k = channel_index_On[k_on];
			o1 += AR(k);
			o2 += AG(k);
			o3 += AB(k);
		}
	}
	else if (op == OP_MEAN)
	{
		//for (int k = 0; k<N; k++)
		for (int k_on = 1; k_on<channels_On; k_on++)
		{
			int k = channel_index_On[k_on];
			o1 += AR(k);
			o2 += AG(k);
			o3 += AB(k);
		}
		o1 /= N;
		o2 /= N;
		o3 /= N;
	}
	else if (op == OP_OIT)
	{
		float avg_1, avg_2, avg_3, avg_a1, avg_a2, avg_a3, avg_a;
		avg_1 = avg_2 = avg_3 = avg_a1 = avg_a2 = avg_a3 = avg_a = 0;
		//for (int k = 0; k<N; k++)
		for (int k_on = 0; k_on<channels_On; k_on++)
		{
			int k = channel_index_On[k_on];

			o1 = AR(k) >> 8;
			o2 = AG(k) >> 8;
			o3 = AB(k) >> 8;
			//			avg_a1 += o1;
			//			avg_a2 += o2;
			//			avg_a3 += o3;
			//			avg_1 += o1*o1;
			//			avg_2 += o2*o2;
			//			avg_3 += o3*o3;
			float a = MAX(o1, MAX(o2, o3));
			//(o1+o2+o3)/3;
			avg_a += a;
			avg_1 += o1 *a;
			avg_2 += o2 *a;
			avg_3 += o3 *a;
		}
		//avg_alpha
		//		avg_a1 /=N;
		//		avg_a2 /=N;
		//		avg_a3 /=N;
		avg_a /= N;	avg_a1 = avg_a2 = avg_a3 = avg_a;

		//avg_color
		avg_1 /= N;
		avg_2 /= N;
		avg_3 /= N;
		//(1-avg_alpha)^n
		float bg_a1 = pow(1 - avg_a1, N);
		float bg_a2 = pow(1 - avg_a2, N);
		float bg_a3 = pow(1 - avg_a3, N);
		float bg_color = 1;
		//0.5;

		// dst_color = avg_color * (1-(1-avg_alpha)^n) + bg_color * (1-avg_alpha)^n
		o1 = avg_1*(1 - bg_a1) + bg_color*bg_a1;
		o2 = avg_2*(1 - bg_a2) + bg_color*bg_a2;
		o3 = avg_3*(1 - bg_a3) + bg_color*bg_a3;

		o1 = CLAMP(0, 1, o1);
		o2 = CLAMP(0, 1, o2);
		o3 = CLAMP(0, 1, o3);

		RGB8 oC;
		oC.r = o1*255.0 + 0.5;
		oC.g = o2*255.0 + 0.5;
		oC.b = o3*255.0 + 0.5;
		oC.r &= mask.r;
		oC.g &= mask.g;
		oC.b &= mask.b;
		return oC;

	}
	// OP_INDEX ignored
	int clamp_max = 65536;

	/*o1 = CLAMP(0, clamp_max, o1)/255;
	o2 = CLAMP(0, clamp_max, o2)/255;
	o3 = CLAMP(0, clamp_max, o3)/255;*/

	o1 = CLAMP(0, clamp_max, o1) >> 8;
	o2 = CLAMP(0, clamp_max, o2) >> 8;
	o3 = CLAMP(0, clamp_max, o3) >> 8;

	RGB8 oC;

	oC.r = o1;
	oC.g = o2;
	oC.b = o3;
	oC.r &= mask.r;
	oC.g &= mask.g;
	oC.b &= mask.b;
	return oC;

#undef R(k)
#undef G(k)
#undef B(k)
#undef A(k)
#undef AR(k)
#undef AG(k)
#undef AB(k)
}

bool Renderer::initLookupTableG16()
{
	v3d_debug("   Renderer::initColormap");

	QVector< QVector<RGBA8> > luts;
	
	ChannelSharedData csData;

	csData.mixOp.brightness = 0;
	csData.mixOp.contrast = 1;

	for (int c = 0; c < FILL_CHANNEL; c++)
	{
		Channel ch;
		ch.n = c;
		ch.on = true;
		ch.color.r = ch.color.g = ch.color.b = 0; ch.color.a = 255;

		if (c == 0)
		{
			ch.color.r = 255;
			//ch.color.g = 255;
		}
		else if (c == 1)
		{
			ch.color.g = 255;
		}
		else if (c == 2)
		{
			ch.color.b = 255;
		}

		csData.listChannel.push_back(ch);
	}

	//4.compute lookup-tables

	QVector<RGBA8> lut(256);
	for (int k = 0; k < FILL_CHANNEL; k++)
	{

		make_linear_lut_one(csData.listChannel[k].color, lut);
		csData.luts.push_back(lut);
	}
	
	//if (p_luts16) { delete[]p_luts16; p_luts16 = 0; }
	//RGBA16** p_luts16 = new RGBA16*[channelSize];
	
	/*if (!p_luts16)
	{
		v3d_error("Fail to run the V3DEngine::prepare() function, fail in p_luts16 create!\n");
		return false;
	}*/


	for (int ch = 0; ch < FILL_CHANNEL; ch++)
	{
		//RGBA16* ch_luts = new RGBA16[256];
		RGBA16* ch_luts = (RGBA16*)(&(texLutsData[ch]));

		if (!ch_luts)
		{
			v3d_error("Fail to run the V3DEngine::prepare() function, fail in ch_luts create!\n");
			return false;
		}
		for (int i = 0; i < 256; i++)
		{
			RGBA8 lut8 = csData.luts[ch][i];

			ch_luts[i].r = ((unsigned short)lut8.r)*((unsigned short)lut8.a);
			ch_luts[i].g = ((unsigned short)lut8.g)*((unsigned short)lut8.a);
			ch_luts[i].b = ((unsigned short)lut8.b)*((unsigned short)lut8.a);
			ch_luts[i].a = lut8.a;

		}
		//p_luts16[ch] = ch_luts;
	}


	for (int ch = 0; ch < 1; ch++)
	{
		//RGBA16* ch_luts = new RGBA16[256];
		RGBA16* ch_luts = (RGBA16*)(&(texLutsData[ch]));

		if (!ch_luts)
		{
			v3d_error("Fail to run the V3DEngine::prepare() function, fail in ch_luts create!\n");
			return false;
		}
		printf(" \n===============Lut =================\n ");
		for (int i = 0; i < 256; i++)
		{
			//RGBA8 lut8 = csData.luts[ch][i];
			printf("<%d,%d,%d,%d> ", ch_luts[i].r, ch_luts[i].g, ch_luts[i].b, ch_luts[i].a);
/*
			ch_luts[i].r = ((unsigned short)lut8.r)*((unsigned short)lut8.a);
			ch_luts[i].g = ((unsigned short)lut8.g)*((unsigned short)lut8.a);
			ch_luts[i].b = ((unsigned short)lut8.b)*((unsigned short)lut8.a);
			ch_luts[i].a = lut8.a;*/

		}
		printf(" \n=============== end Lut =================\n ");
		//p_luts16[ch] = ch_luts;
	}



	glBindTexture(GL_TEXTURE_2D, texLuts);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); // MUST use nearest filter
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); // MUST use nearest filter

	// load texture
	


	glTexImage2D(GL_TEXTURE_2D, // target
		0, // level
		   //GL_R16UI, // texture format, report error, can't support
		GL_RGBA16UI,
		//GL_RED, // texture format
		256, // width
		FILL_CHANNEL, // height
		0, // border
		GL_RGBA_INTEGER,
		//GL_RED, // image format
		GL_UNSIGNED_SHORT, // image type
		&texLutsData[0][0]);

	//glTexImage2D(GL_TEXTURE_2D, // target
	//	0, // level
	//	   //GL_R16UI, // texture format, report error, can't support
	//	GL_R16UI,
	//	//GL_RED, // texture format
	//	256*4, // width
	//	FILL_CHANNEL, // height
	//	0, // border
	//	GL_RED_INTEGER,
	//	//GL_RED, // image format
	//	GL_UNSIGNED_SHORT, // image type
	//	&texLutsData[0][0]);
	
	return true; 

}

bool Renderer::initHistRange()
{
	v3d_debug("   Renderer::initHistRange");

	HISTRange16 histC; 
	for (int ch = 0; ch < FILL_CHANNEL; ch++)
	{
		histC.channelOn = true; 
		//histC.channelOn = false;
		RGBA16 colorC;
		colorC.r = colorC.g = colorC.b = 0;  colorC.a = 255;

		if (ch == 0)
		{
			histC.hist_min = 310;
			histC.hist_max = 923;
			//histC.hist_min = 0;
			//histC.hist_max = 3644;
			colorC.r = 255;
			colorC.g = 255;
		}
		else if (ch == 1)
		{
			histC.hist_min = 342;
			histC.hist_max = 3545;
			colorC.g = 255;
		}
		else if (ch == 2)
		{
			histC.hist_min = 343;
			histC.hist_max = 526;
			colorC.b = 255;
		}
		else
		{
			histC.hist_min = 343;
			histC.hist_max = 526;
			colorC.b = 255;
		}
		histC.v_range = histC.hist_max - histC.hist_min;
		if (histC.v_range <= 0)  histC.v_range = 1;

		
		histC.color = colorC; 
		texHistData[ch] = histC;

	}


	glBindTexture(GL_TEXTURE_2D, texHistRange);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); // MUST use nearest filter
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); // MUST use nearest filter

	// load texture
	glTexImage2D(GL_TEXTURE_2D, // target
		0, // level
		   //GL_R16UI, // texture format, report error, can't support
		GL_R16UI,
		//GL_RED, // texture format
		HistRangeCols, // width
		FILL_CHANNEL, // height
		0, // border
		GL_RED_INTEGER,
		//GL_RED, // image format
		GL_UNSIGNED_SHORT, // image type
		&texHistData[0]);

	return true;

}

void Renderer::loadVol()
{

	v3d_debug("  Renderer::loadVol");
	//makeCurrent(); //ensure right context when multiple views animation or mouse drop, 081105

	if (!imgStack_rgb || bufSize[3] < 1) return; // no image data, 081002

	QElapsedTimer qtime;  qtime.start();

	///////////////////////////////////////////////////////////////
	// set coordinate frame size
	sampleScaleX = sampleScale[0];
	sampleScaleY = sampleScale[1];
	sampleScaleZ = sampleScale[2];
	imageX = bufSize[0];
	imageY = bufSize[1];
	imageZ = bufSize[2];
	imageT = bufSize[4];

	bool ok;

	fillX = _getTexFillSize(imageX);
	fillY = _getTexFillSize(imageY);
	fillZ = _getTexFillSize(imageZ);
	v3d_debug("   sampleScale = %gx%gx%g""   sampledImage = %dx%dx%d""   fillTexture = %dx%dx%d",
		sampleScaleX, sampleScaleY, sampleScaleZ, imageX, imageY, imageZ, fillX, fillY, fillZ);

	{
		//tryTex3D = 0; //091015: no need, because tex3D & tex_stream_buffer is not related now.

		v3d_debug("Renderer::loadVol() - creating data structures for managing 2D texture slice set\n");

		Ztex_list = new GLuint[imageZ + 1]; //+1 for pbo tex
		Ytex_list = new GLuint[imageY + 1];
		Xtex_list = new GLuint[imageX + 1];
		memset(Ztex_list, 0, sizeof(GLuint)*(imageZ + 1));
		memset(Ytex_list, 0, sizeof(GLuint)*(imageY + 1));
		memset(Xtex_list, 0, sizeof(GLuint)*(imageX + 1));
		glGenTextures(imageZ + 1, Ztex_list);
		glGenTextures(imageY + 1, Ytex_list);
		glGenTextures(imageX + 1, Xtex_list);


		int X = _getBufFillSize(imageX);
		int Y = _getBufFillSize(imageY);
		int Z = _getBufFillSize(imageZ);
		Zslice_data = new RGBA8[Y * X];//[Z][y][x] //base order
		Yslice_data = new RGBA8[Z * X];//[Y][z][x]
		Xslice_data = new RGBA8[Z * Y];//[X][z][y]
		memset(Zslice_data, 0, sizeof(RGBA8)* (Y * X));
		memset(Yslice_data, 0, sizeof(RGBA8)* (Z * X));
		memset(Xslice_data, 0, sizeof(RGBA8)* (Z * Y));

		// optimized copy slice data in setupStackTexture, by RZC 2008-10-04
	}

	v3d_debug("   setupStack: id & buffer ....................... cost time = %g sec", qtime.elapsed()*0.001);

	texture_format = GL_RGBA;

	image_format = GL_RGBA;

	image_type = GL_UNSIGNED_BYTE;

	subloadTex(0, 0, 0, true);//modified by eva 2019-1-4
}

void Renderer::loadVol_grey16()
{
	v3d_debug("Renderer::loadVol_grey16");
	//makeCurrent(); //ensure right context when multiple views animation or mouse drop, 081105

	if (!imgStack_raw_StackZ || bufSize[3] < 1) return; // no image data, 081002

	QElapsedTimer qtime;  qtime.start();

	///////////////////////////////////////////////////////////////
	// set coordinate frame size
	sampleScaleX = sampleScale[0];
	sampleScaleY = sampleScale[1];
	sampleScaleZ = sampleScale[2];
	imageX = bufSize[0];
	imageY = bufSize[1];
	imageZ = bufSize[2];
	imageT = bufSize[4];

	bool ok;
	
	fillX = imageX;
	fillY = imageY;
	fillZ = imageZ;
	v3d_debug("   sampleScale = %gx%gx%g""   sampledImage = %dx%dx%d""   fillTexture = %dx%dx%d",
		sampleScaleX, sampleScaleY, sampleScaleZ, imageX, imageY, imageZ, fillX, fillY, fillZ);

	{
		//tryTex3D = 0; //091015: no need, because tex3D & tex_stream_buffer is not related now.

		v3d_debug("Renderer::loadVol() - creating data structures for managing 2D texture slice set\n");
		int channels = imgStack_raw_StackZ->getCDim(); 

		//channels = 2; 

		Ztex_list = new GLuint[(imageZ + 1)*channels]; //+1 for pbo tex
		Ytex_list = new GLuint[(imageY + 1)*channels];
		Xtex_list = new GLuint[(imageX + 1)*channels];
		memset(Ztex_list, 0, sizeof(GLuint)*(imageZ + 1)*channels);
		memset(Ytex_list, 0, sizeof(GLuint)*(imageY + 1)*channels);
		memset(Xtex_list, 0, sizeof(GLuint)*(imageX + 1)*channels);
		glGenTextures((imageZ + 1)*channels, Ztex_list);
		glGenTextures((imageY + 1)*channels, Ytex_list);
		glGenTextures((imageX + 1)*channels, Xtex_list);

		int X = imageX;
		int Y = imageY;
		int Z = imageZ;
		Zslice_data_g16 = new unsigned short[Y * X*channels];//[Z][y][x] //base order
		Yslice_data_g16 = new unsigned short[Z * X*channels];//[Y][z][x]
		Xslice_data_g16 = new unsigned short[Z * Y*channels];//[X][z][y]
		memset(Zslice_data_g16, 0, sizeof(unsigned short)* (Y * X)*channels);
		memset(Yslice_data_g16, 0, sizeof(unsigned short)* (Z * X)*channels);
		memset(Xslice_data_g16, 0, sizeof(unsigned short)* (Z * Y)*channels);

		// optimized copy slice data in setupStackTexture, by RZC 2008-10-04
	}

	v3d_debug("   setupStack: id & buffer ....................... cost time = %g sec", qtime.elapsed()*0.001);


	///////////////////////////////////////
	//if (texture_format == -1)
	{
		texture_format = GL_RGBA;
	}
	//if (image_format == -1)
	{
		image_format = GL_RGBA;
	}
	//if (image_type == -1)
	{
		image_type = GL_UNSIGNED_BYTE;
	}

	subloadTex_grey16(0, 0, 0, true);//modified by eva 2019-1-4
	///////////////////////////////////////

}

int  Renderer::_getTexFillSize(int w)
{
	return power_of_two_ceil(w);
}

int  Renderer::_getBufFillSize(int w)
{

	return power_of_two_ceil(w); //always use power_of_two

}

//add by eva 2019-1-4
void Renderer::subloadTex(V3DLONG v, V3DLONG timepoint, V3DLONG position, bool bfirst)
{
	if (texture_format == -1)  return; // not done by loadVol
	if (!imgStack_rgb || bufSize[3] < 1) return; // no image data, 081002

	QElapsedTimer qtime;  qtime.start();
	{
		v3d_debug("Calling setupStackTexture() from Renderer::subloadTex()\n");

		setupStackTexture(v, timepoint, position, bfirst);  // use a temporary buffer, so first

	}
	v3d_info("   subloadTex  [view %d][position %d][timepoint %d] ...... cost time = %g sec", v, position, timepoint, qtime.elapsed()*0.001);

}

//add by eva 2019-1-4
void Renderer::subloadTex_grey16(V3DLONG v, V3DLONG timepoint, V3DLONG position, bool bfirst)
{
	if (texture_format == -1)  return; // not done by loadVol
	if (!imgStack_raw_StackZ || bufSize[3] < 1) return; // no image data, 081002

	QElapsedTimer qtime;  qtime.start();
	{
		v3d_debug("Calling setupStackTexture() from Renderer::subloadTex()\n");
#ifndef TEXTURE_ARRAY
		setupStackTexture_grey16(v, timepoint, position, bfirst);  // use a temporary buffer, so first
#else
		setupStackArrayTexture_grey16(v, timepoint, position, bfirst);
#endif
	}
	v3d_info("   subloadTex  [view %d][position %d][timepoint %d] ...... cost time = %g sec", v, position, timepoint, qtime.elapsed()*0.001);

}

void Renderer::setupData(Lit3drParam& param)
{

	b_limitedsize = false; //091022, 100720: down-sampling only if tryTexStream==0
	{
		// creating data for 3dviewer when needed
		//isSimulatedData = false;
		bool bLocal = false;

		if (b_limitedsize)
		{
			v3d_info("	Down-sampling to 512x512x256 ");
		}

		try
		{
			//Image4DSimple* image4d = param.image4d;
			My4DImage* image4d = param.image4d;
			imgStack_rgb = param.image4d;
			imgStack_rgb_bin2 = param.image4d_bin2;
			imgStack_raw_StackZ = param.image4d_raw;
			//My4DImage* image4d = v3dr_getImage4d(_idep);
			unsigned char* data4dp = image4d->getRawData();
			if (image4d && image4d->getCDim() > 0)
			{
				bLocal = param.b_local;
				//bLimited = param.b_use_512x512x256; //091015: no need this, because always can use stream texture
				
			/*	data_unitbytes = image4d->getUnitBytes();
				data4dp = image4d->getRawData();
				data4d_uint8 = image4d->getRawData;*/

				size1 = dim1 = image4d->getXDim();
				size2 = dim2 = image4d->getYDim();
				size3 = dim3 = image4d->getZDim();
				size4 = dim4 = image4d->getCDim();
				size5 = dim5 = 1;
				if (image4d->getTDim() > 1 && image4d->getTimePackType() == TIME_PACK_C)
				{
					assert(image4d->getCDim() >= image4d->getTDim());

					size4 = dim4 = image4d->getCDim() / image4d->getTDim();
					size5 = dim5 = image4d->getTDim();
				}
				start1 = 0;
				start2 = 0;
				start3 = 0;
				start4 = 0;
				start5 = 0;

				if (bLocal)
				{
					/*size1 = param.local_size.x;
					size2 = param.local_size.y;
					size3 = param.local_size.z;*/
					sampleScale[0] = sampleScale[1] = sampleScale[2] = 1;

				/*	start1 = param.local_start.x;
					start2 = param.local_start.y;
					start3 = param.local_start.z;*/
					//data4dp += start3*(dim2*dim1) + start2*(dim1) + start1;
				}
			}
			else // image4d==0  default coordinate frame for surface
			{
				size1 = dim1 = 0; //DEFAULT_DIM1;
				size2 = dim2 = 0; //DEFAULT_DIM2;
				size3 = dim3 = 0; //DEFAULT_DIM3;
				size4 = dim4 = 0; // this make no rgbaBuf allocated
				size5 = dim5 = 0; // packed time
				start1 = 0;
				start2 = 0;
				start3 = 0;
				start4 = 0;
				start5 = 0;
				//data4dp = 0; // this make no rgbaBuf allocated
			}

			v3dInfo = new V3DInfo(); 
			if (b_limitedsize)
			{
				v3dInfo->limit_X = size1; 
				v3dInfo->limit_Y = size2;
				v3dInfo->limit_Z = size3;

				getLimitedSampleScaleBufSize(size1, size2, size3, size4, size5, sampleScale, bufSize, v3dInfo);
			}
			else
			{
				bufSize[0] = size1;
				bufSize[1] = size2;
				bufSize[2] = size3;
				bufSize[3] = size4;
				bufSize[4] = size5;
				sampleScale[0] = sampleScale[1] = sampleScale[2] = sampleScale[3] = sampleScale[4] = 1;
			}

			//total_rgbaBuf = rgbaBuf = 0; //(RGBA*)-1; //test whether the new sets pointer to 0 when failed
			//if (data4dp && size4 > 0)
			//{
			//	// only RGB, first 3 channels of original image
			//	total_rgbaBuf = rgbaBuf = new RGBA8[bufSize[0] * bufSize[1] * bufSize[2] * 1 * bufSize[4]];
			//}

			v3d_debug("   data4dp = %0p \t(start %dx%dx%d_%d_%d, size %dx%dx%d_%d_%d)", data4dp,
				start1, start2, start3, start4, start5, size1, size2, size3, size4, size5);

			//v3d_debug("   rgbaBuf = %0p \t(%dx%dx%d_%d_%d)", rgbaBuf, bufSize[0], bufSize[1], bufSize[2], bufSize[3], bufSize[4]);


			dataViewProcBox = dataBox = BoundingBox(start1, start2, start3, start1 + (size1 - 1), start2 + (size2 - 1), start3 + (size3 - 1));

			v3d_debug("   data box in original image space @\t(%g %g %g)--(%g %g %g)", dataBox.x0, dataBox.y0, dataBox.z0, dataBox.x1, dataBox.y1, dataBox.z1);

		} CATCH_handler("Renderer_gl1::setupData");


	} // end if else for creating data for 3dviewer
}

//add by eva 2019-1-10
void Renderer::setupStackTexture(V3DLONG v, V3DLONG timepoint, V3DLONG position, bool bfirst)
{

	v3d_debug("Renderer::setupStackTexture() start");

	size_t t0 = clock();

	RGBA8* tex3DBuf = NULL;
	RGBA8* tex3DBuf_bin2 = NULL;

	{
		//tex3DBuf = _safeReference3DBuf(rgbaBuf, imageX, imageY, imageZ, safeX, safeY, safeZ); //081008

		int width, height, channel, zslice, pos, tp, view;
		ImagePixelType datatype;
		
		if (!imgStack_rgb)
		{
			v3d_error("Fail to run the Renderer::setupStackTexture() function, imgStack_rgb is null, p=%d, t=%d\n", position, timepoint);
			return;
		}

		unsigned char * imgData = imgStack_rgb->getRawData();
		if (!imgData)
		{
			v3d_error("Fail to run the Renderer::setupStackTexture() function, imgData is null, p=%d, t=%d\n", position, timepoint);
			return;
		}
		tex3DBuf = (RGBA8*)imgData;


		unsigned char * imgData_bin2 = imgStack_rgb_bin2->getRawData();
		if (!imgData_bin2)
		{
			v3d_error("Fail to run the Renderer::setupStackTexture() function, imgData is null, p=%d, t=%d\n", position, timepoint);
			return;
		}
		tex3DBuf_bin2 = (RGBA8*)imgData_bin2;
		//realX = safeX;
		//realY = safeY;
		//realZ = safeZ;

		realX = safeX = width = imgStack_rgb->getXDim(); 
		realY = safeY = height = imgStack_rgb->getYDim(); 
		realZ = safeZ = zslice = imgStack_rgb->getZDim(); 
	}
	fillX = _getTexFillSize(realX);
	fillY = _getTexFillSize(realY);
	fillZ = _getTexFillSize(realZ);

	v3d_debug("	texture:   real = %dx%dx%d   fill = %dx%dx%d", realX, realY, realZ, fillX, fillY, fillZ);
	
	// 3 stacks of 2D texture ===================================================================
	for (int stack_i = 1; stack_i <= 1; stack_i++)
	{
		int n_slice = 0;
		RGBA8* p_slice = 0;
		GLuint* p_tex = 0;
		int w = 0, h = 0;
		int sw = 0, sh = 0;

		switch (stack_i)
		{
		case 1: //Z[y][x]
			n_slice = realZ;
			p_slice = Zslice_data;
			p_tex = Ztex_list;
			w = fillX, h = fillY;
			sw = realX, sh = realY;
			break;
		case 2: //Y[z][x]
			n_slice = realY;
			p_slice = Yslice_data;
			p_tex = Ytex_list;
			w = fillX, h = fillZ;
			sw = realX, sh = realZ;
			break;
		case 3: //X[z][y]
			n_slice = realX;
			p_slice = Xslice_data;
			p_tex = Xtex_list;
			w = fillY, h = fillZ;
			sw = realY, sh = realZ;
			break;
		}

		glGenTextures(1, &textureID_rgb);
		glBindTexture(GL_TEXTURE_2D_ARRAY, textureID_rgb);
		setTexParamArray();
		glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, texture_format, w, h, imageZ, 0, image_format, image_type, NULL);
		for (int i = 0; i < n_slice; i++)
		{
			RGBA8* p_first = NULL;
			//bfirst = true;//add by eva 2019-2-28, always true, make sure don't use cache
			if (bfirst)
			{
				if (stack_i == 1 && i >= n_slice / 2)
				{
#ifdef HALF_Z_BIN2
					p_first = p_slice;
					if (p_first) _copySliceFromStack(tex3DBuf_bin2, realX/2, realY/2, realZ, p_first, w/2, stack_i, i);
					glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, w/2, h/2, 1, image_format, image_type, p_first);
					//glGenerateMipmap(GL_TEXTURE_2D);
					continue;
#endif
				}
				p_first = p_slice;
				if (p_first) _copySliceFromStack(tex3DBuf, realX, realY, realZ, p_first, w, stack_i, i);
				glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, w, h, 1, image_format, image_type, p_first);
				//glGenerateMipmap(GL_TEXTURE_2D);
			}
		}
	}
}

void Renderer::setupStackTexture_grey16(V3DLONG v, V3DLONG timepoint, V3DLONG position, bool bfirst)
{
	v3d_debug("Renderer::setupStackTexture_grey16() start");

	size_t t0 = clock();

	unsigned short* tex3DBuf = NULL;
	//unsigned short* tex3DBuf_bin2 = NULL;

	{
		//tex3DBuf = _safeReference3DBuf(rgbaBuf, imageX, imageY, imageZ, safeX, safeY, safeZ); //081008

		int width, height, channel, zslice, pos, tp, view;
		ImagePixelType datatype;

		if (!imgStack_raw_StackZ)
		{
			v3d_error("Fail to run the Renderer::setupStackTexture_grey16() function, imgStack_rgb is null, p=%d, t=%d\n", position, timepoint);
			return;
		}

		//unsigned char * imgData = imgStack_raw->getRawDataAtChannel();
		/*if (!imgData)
		{
			v3d_error("Fail to run the Renderer::setupStackTexture() function, imgData is null, p=%d, t=%d\n", position, timepoint);
			return;
		}
		tex3DBuf = (unsigned short*)imgData;*/


		//realX = safeX;
		//realY = safeY;
		//realZ = safeZ;

		realX = safeX = width = imgStack_raw_StackZ->getXDim();
		realY = safeY = height = imgStack_raw_StackZ->getYDim();
		realZ = safeZ = zslice = imgStack_raw_StackZ->getZDim();
	}
	//fillX = _getTexFillSize(realX);
	//fillY = _getTexFillSize(realY);
	//fillZ = _getTexFillSize(realZ);

	fillX = realX;
	fillY = realY;
	fillZ = realZ;

	int channels = imgStack_raw_StackZ->getCDim(); 

	int nthreads = 2;

	imgStack_raw_StackY = new Image4DSimple();
	reshapeImageStack(*imgStack_raw_StackZ, *imgStack_raw_StackY, 1, nthreads); //
	imgStack_raw_StackX = new Image4DSimple();
	reshapeImageStack(*imgStack_raw_StackZ, *imgStack_raw_StackX, 0, nthreads); //

	//channels = 2; 

	int w1, h1;
	ImagePixelType dt1;

	v3d_debug("	texture:   real = %dx%dx%d   fill = %dx%dx%d", realX, realY, realZ, fillX, fillY, fillZ);

	Image4DSimple *imgStack_raw; 

	// 3 stacks of 2D texture ===================================================================
	for (int stack_i = 1; stack_i <= 3; stack_i++)
	{
		int n_slice = 0;
		unsigned short* p_slice = 0;
		GLuint* p_tex = 0;
		int w = 0, h = 0;
		int sw = 0, sh = 0;

		switch (stack_i)
		{
		case 1: //Z[y][x]
			n_slice = realZ;
			p_slice = Zslice_data_g16;
			p_tex = Ztex_list;
			w = fillX, h = fillY;
			sw = realX, sh = realY;

			imgStack_raw = imgStack_raw_StackZ;
			break;
		case 2: //Y[z][x]
			n_slice = realY;
			p_slice = Yslice_data_g16;
			p_tex = Ytex_list;
			w = fillX, h = fillZ;
			sw = realX, sh = realZ;
			imgStack_raw = imgStack_raw_StackY;
			break;
		case 3: //X[z][y]
			n_slice = realX;
			p_slice = Xslice_data_g16;
			p_tex = Xtex_list;
			w = fillY, h = fillZ;
			sw = realY, sh = realZ;
			imgStack_raw = imgStack_raw_StackX;
			break;
		}
	
		for (int i = 0; i < n_slice; i++)
		{
			for (int ch = 0; ch < channels; ch++)
			{
			
				glBindTexture(GL_TEXTURE_2D, p_tex[ i * channels + ch + 1]); //[0] reserved for pbo tex
				unsigned short* p_first = NULL;
				bfirst = true;//add by eva 2019-2-28, always true, make sure don't use cache
				if (bfirst)
				{
					if (stack_i == 2 && ch == 2 && i == 511)
					{
						printf(" crash point \n"); 
					}
					p_first =(unsigned short*)(imgStack_raw->getImage(i, ch, 0, 0, w1, h1, dt1));
					//if (p_first) _copySliceFromStack(tex3DBuf, realX, realY, realZ, p_first, w, stack_i, i);
					int bw = 5; 
					int bh = 5; 
					if (i % 100 == 0)
					{
						printf("\n ******************* start of slice :%d \n", i);
						imgStack_raw->printImage((unsigned char*)p_first, w1, h1, bw, bh);
						printf("\n ******************* end of slice :%d \n", i ); 
					}

					setTexParam2D();

					glTexImage2D(GL_TEXTURE_2D, // target
						0, // level
						GL_R16UI, // texture format
						w, // width
						h, // height
						0, // border
						GL_RED_INTEGER, // image format
						GL_UNSIGNED_SHORT, // image type
						p_first);

					//glTexImage2D(GL_TEXTURE_2D, // target
					//	0, // level
					//	texture_format, // texture format
					//	w, // width
					//	h, // height
					//	0, // border
					//	image_format, // image format
					//	image_type, // image type
					//	p_first);
					//CHECK_GLErrorString_throw(); // can throw const char* exception, RZC 080925
					//glGenerateMipmap(GL_TEXTURE_2D);
				}
				
			}
		}
		size_t t1 = clock();

		v3d_debug("Renderer::setupStackTexture() end, total time cost:%d\n", t1 - t0);

	}

}

void Renderer::setupStackArrayTexture_grey16(V3DLONG v, V3DLONG timepoint, V3DLONG position, bool bfirst)
{
	v3d_debug("Renderer::setupStackArrayTexture_grey16() start");
	size_t t0 = clock();
	{
		int width, height, channel, zslice, pos, tp, view;
		ImagePixelType datatype;

		if (!imgStack_raw_StackZ)
		{
			v3d_error("Fail to run the Renderer::setupStackArrayTexture_grey16() function, imgStack_rgb is null, p=%d, t=%d\n", position, timepoint);
			return;
		}

		realX = safeX = width = imgStack_raw_StackZ->getXDim();
		realY = safeY = height = imgStack_raw_StackZ->getYDim();
		realZ = safeZ = zslice = imgStack_raw_StackZ->getZDim();
	}

	fillX = realX;
	fillY = realY;
	fillZ = realZ;

	int channels = imgStack_raw_StackZ->getCDim();

	int nthreads = 2;


	size_t t11 = clock();
	imgStack_raw_StackY = new Image4DSimple();
	reshapeImageStack(*imgStack_raw_StackZ, *imgStack_raw_StackY, 1, nthreads); //
	imgStack_raw_StackX = new Image4DSimple();
	reshapeImageStack(*imgStack_raw_StackZ, *imgStack_raw_StackX, 0, nthreads); //
	v3d_debug("Renderer::reshapeImageStack() end, total time cost:%d--------------------------------------->\n", clock() - t11);

																				//channels = 2; 
	int w1, h1;
	ImagePixelType dt1;

	v3d_debug("	texture:   real = %dx%dx%d   fill = %dx%dx%d", realX, realY, realZ, fillX, fillY, fillZ);

	Image4DSimple *imgStack_raw;

	// 3 stacks of 2D texture ===================================================================
	for (int stack_i = 1; stack_i <= 1; stack_i++)
	{
		int n_slice = 0;
		unsigned short* p_slice = 0;
		GLuint* p_tex = 0;
		int w = 0, h = 0;
		int sw = 0, sh = 0;

		switch (stack_i)
		{
		case 1: //Z[y][x]
			n_slice = realZ;
			p_slice = Zslice_data_g16;
			p_tex = Ztex_list;
			w = fillX, h = fillY;
			sw = realX, sh = realY;

			imgStack_raw = imgStack_raw_StackZ;
			break;
		case 2: //Y[z][x]
			n_slice = realY;
			p_slice = Yslice_data_g16;
			p_tex = Ytex_list;
			w = fillX, h = fillZ;
			sw = realX, sh = realZ;
			imgStack_raw = imgStack_raw_StackY;
			break;
		case 3: //X[z][y]
			n_slice = realX;
			p_slice = Xslice_data_g16;
			p_tex = Xtex_list;
			w = fillY, h = fillZ;
			sw = realY, sh = realZ;
			imgStack_raw = imgStack_raw_StackX;
			break;
		}

		for (int ch = 0; ch < channels; ch++)
		{
			glGenTextures(1, &textureID[ch]);
			glBindTexture(GL_TEXTURE_2D_ARRAY, textureID[ch]);
			setTexParamArray();
			glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_R16UI, w, h, imageZ, 0, GL_RED_INTEGER, GL_UNSIGNED_SHORT, NULL);
			for (int i = 0; i < n_slice; i++)
			{
				unsigned short* p_first = NULL;
				bfirst = true;//add by eva 2019-2-28, always true, make sure don't use cache
				if (bfirst)
				{
					if (stack_i == 2 && ch == 2 && i == 511)
					{
						printf(" crash point \n");
					}
					p_first = (unsigned short*)(imgStack_raw->getImage(i, ch, 0, 0, w1, h1, dt1));
					//if (p_first) _copySliceFromStack(tex3DBuf, realX, realY, realZ, p_first, w, stack_i, i);
					int bw = 5;
					int bh = 5;
					if (i % 100 == 0)
					{
						printf("\n ******************* start of slice :%d \n", i);
						imgStack_raw->printImage((unsigned char*)p_first, w1, h1, bw, bh);
						printf("\n ******************* end of slice :%d \n", i);
					}
					glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, w, h, 1, GL_RED_INTEGER, GL_UNSIGNED_SHORT, p_first);
				}
			}
		}
	}
	size_t t1 = clock();

	v3d_debug("Renderer::setupStackTexture() end, total time cost:%d\n", t1 - t0);
}

void Renderer::setTexParam2D()
{
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	
	VOLUME_FILTER = 1; 

	if (VOLUME_FILTER == 1) {

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	else {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}
}

void Renderer::setTexParam3D()
{
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	if (VOLUME_FILTER == 1) {
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	else {
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}
}

void Renderer::setTexParamArray()
{
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void Renderer::setupView(int width, int height)
{
	if (!m_inited)
	{
		return; 
	}
	v3d_debug(" Renderer::setupView -width:%d height:%d", width, height);
	//m_glwidget->makeCurrent();

	screenW = width;
	screenH = height;

	/*int side = qMin(width, height);
	glViewport((width - side) / 2, (height - side) / 2, side, side);*/
	//glViewport(0, 0, width, height);

	//m_glwidget->doneCurrent();

	/*glViewport(0, 0, width, height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	setProjection();

	glMatrixMode(GL_MODELVIEW);
	
	m_glwidget->doneCurrent(); */

}

void Renderer::setProjection()
{
	GLdouble aspect = double(screenW) / MAX(screenH, 1);

	if (bOrthoView)
	{
		GLdouble halfw = 1.3*aspect *zoomRatio;
		GLdouble halfh = 1.3        *zoomRatio;
		projectionMat = glm::ortho(-halfw, halfw, -halfh, halfh, viewNear, viewFar);
	}
	else
	{
		//[QT5_MK]
		GLdouble fov = viewAngle*zoomRatio;
		GLdouble rFov = fov * 3.14159265 / 180.0;
		GLdouble fH = tan(rFov)*viewNear;
		GLdouble fW = fH * aspect;
		projectionMat = glm::frustum(-fW, fW, -fH, fH, viewNear, viewFar);
		//projectionMat = glm::perspective(viewAngle*zoomRatio, aspect, viewNear, viewFar);
	}
}

void Renderer::setViewPos()
{
	viewPos = vec3(0.0f, 0.0f, -viewDistance);
	viewHead = vec3(0.0f, 0.0f, 1.0f);
	viewMat = glm::lookAt(viewPos, viewPos + viewHead, glm::vec3(0.0f, 1.0f, 0.0f));
}

void Renderer::blendBrighten(float fbright, float fcontrast) // fast, 8-bit precision
{
	//fcontrast = 1.5;
	if (fbright == 0 && fcontrast == 1) return;

	if (fbright >= -1 && fbright <= 1)
	{
		glPushAttrib(GL_ENABLE_BIT);
		glDisable(GL_DEPTH_TEST);
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();
		glEnable(GL_BLEND);
#define RECT_BLEND	glRecti(-100,-100, 100,100) // assume [-1,+1]^3 view

		if (fcontrast <= 1)
		{
			if (fbright >= 0)
			{
				//[QT5_MK]
				//glBlendEquationEXT(GL_FUNC_ADD_EXT);
				glBlendEquation(GL_FUNC_ADD_EXT);
				//[End]
			}
			else //fbright <0
			{
				//[QT5_MK]
				//glBlendEquationEXT(GL_FUNC_REVERSE_SUBTRACT_EXT);
				glBlendEquation(GL_FUNC_REVERSE_SUBTRACT_EXT);
				//[End]
				fbright = -fbright;
			}
			glBlendFunc(GL_ONE, GL_SRC_ALPHA); // new_color = 1*fbright + fcontrast*old_color
			glColor4f(fbright, fbright, fbright, fcontrast);
			RECT_BLEND;
		}

		else //fcontrast >1
		{
			float res = fcontrast - 1; // int(fcontrast);
			if (res)
			{
				//[QT5_MK]
				//glBlendEquationEXT(GL_FUNC_ADD_EXT);
				glBlendEquation(GL_FUNC_ADD_EXT);
				//[End]
				glBlendFunc(GL_DST_COLOR, GL_ONE); // new_color = res*old_color + 1*old_color;
				glColor4f(res, res, res, 1);
				RECT_BLEND;
			}

			if (fbright >= 0)
			{
				//[QT5_MK]
				//glBlendEquationEXT(GL_FUNC_ADD_EXT);
				glBlendEquation(GL_FUNC_ADD_EXT);
				//[End]
			}
			else //fbright <0
			{
				//[QT5_MK]
				//glBlendEquationEXT(GL_FUNC_REVERSE_SUBTRACT_EXT);
				glBlendEquation(GL_FUNC_REVERSE_SUBTRACT_EXT);
				//
				fbright = -fbright;
			}
			glBlendFunc(GL_ONE, GL_ONE); // new_color = 1*fbright + 1*old_color
			glColor4f(fbright, fbright, fbright, 1);
			RECT_BLEND;
		}

		glPopMatrix();
		glPopAttrib();
	}
}

/*
* reshape the Image Stack from XYZ to ZYX(stack_i =3), or ZXY(stack_i=2)
*/
template<class T>
bool _reshape3DImageStack_T(Image4DSimple &src, Image4DSimple &dest, int axis, int nthreads)
{

	try
	{
		T *data = new T[src.getTotalUnitNumber()];
		if (!data)
		{
			v3d_error("Fail to run the reshapeImageStack() function, fail to allocate memory for data.\n");
			return false;
		}

		int channels = src.getChannels();

		Image4DProxy<Image4DSimple> proxy_src(&src);

		switch (axis)
		{
		case 0:  // [Z*Y] * X 

		{
			dest.setData((unsigned char *)data, src.getYDim(), src.getZDim(), src.getXDim(), channels, src.getDatatype());
			dest.setChannels(channels);

			Image4DProxy<Image4DSimple> proxy_dest(&dest);
			for (long long iz = 0; iz < src.getZDim(); iz++) {
				for (long long iy = 0; iy < src.getYDim(); iy++) {
					for (long long ix = 0; ix < src.getXDim(); ix++) {
						for (long long ic = 0; ic < channels; ic++) {
							*((T *)proxy_dest.at(iy, iz, ix, ic)) = *((T *)proxy_src.at(ix, iy, iz, ic));
						}
					}
				}
			}

			//dest.setSlices(dest.getZDim());

			break;
		}
		case 1:  // [Z*X] -> Y 
		{
			dest.setData((unsigned char *)data, src.getXDim(), src.getZDim(), src.getYDim(), channels, src.getDatatype());
			dest.setChannels(channels);

			Image4DProxy<Image4DSimple> proxy_dest(&dest);
			for (long long iz = 0; iz < src.getZDim(); iz++) {
				for (long long iy = 0; iy < src.getYDim(); iy++) {
					for (long long ix = 0; ix < src.getXDim(); ix++) {
						for (long long ic = 0; ic < channels; ic++) {
							//*((T *)proxy_dest.at(src.getXDim() - ix - 1, iz, iy, ic)) = *((T *)proxy_src.at(ix, iy, iz, ic));
							*((T *)proxy_dest.at(ix, iz, iy, ic)) = *((T *)proxy_src.at(ix, iy, iz, ic));
						}
					}
				}
			}
			//dest.setSlices(dest.getZDim());

			break;
		}


		}


		return true;
	}
	catch (...)
	{
		v3d_error("Fail to run the ReshapeImageStack() function.\n");
		return false;
	}



}

bool reshapeImageStack(Image4DSimple & src, Image4DSimple & dest, int axis, int nthreads)
{
	bool ret = false;
	switch (src.getDatatype())
	{
	case V3D_UINT8: {
		ret = _reshape3DImageStack_T<uint8>(src, dest, axis, nthreads);
		break;
	}
	case V3D_UINT16: {
		ret = _reshape3DImageStack_T<uint16>(src, dest, axis, nthreads);
		break;
	}
	case V3D_FLOAT32: {
		ret = _reshape3DImageStack_T<float>(src, dest, axis, nthreads);
		break;
	}
	default: {
		cerr << "Not implemented for type " << src.getDatatype() << " !" << endl;

		ret = false;
		///exit(-1);
		break;
	}

	}
	return ret;
}

////////////////////////////////////////////////////////////////////////////////
void _copySliceFromStack(RGBA8* rgbaBuf, int bufX, int bufY, int bufZ,
	RGBA8* slice, int copyW, int stack_i, int slice_i,
	RGBA8* rgbaYzx, RGBA8* rgbaXzy)
{
	V3DLONG x, y, z;
	switch (stack_i)
	{
	case 1: //Zslice_data[y][x]
		z = slice_i;
		if (copyW == bufX)
			memcpy(slice, rgbaBuf + z*(bufY*bufX), sizeof(RGBA8)*bufY*bufX);
		else
			for (y = 0; y < bufY; y++)
			{
				memcpy(slice + y*(copyW), rgbaBuf + z*(bufY*bufX) + y*(bufX), sizeof(RGBA8)*bufX);
			}
		break;

	case 2: //Yslice_data[z][x]
		y = slice_i;
		if (rgbaYzx  && copyW == bufX)
			memcpy(slice, rgbaYzx + y*(bufZ*bufX), sizeof(RGBA8)*bufZ*bufX);
		else
			for (z = 0; z < bufZ; z++)
			{
				memcpy(slice + z*(copyW), rgbaBuf + z*(bufY*bufX) + y*(bufX), sizeof(RGBA8)*bufX);
			}
		break;

	case 3: //Xslice_data[z][y]
		x = slice_i;
		if (rgbaXzy  && copyW == bufY)
			memcpy(slice, rgbaXzy + x*(bufZ*bufY), sizeof(RGBA8)*bufZ*bufY);
		else if (rgbaXzy)
			for (z = 0; z < bufZ; z++)
			{
				memcpy(slice + z*(copyW), rgbaXzy + x*(bufZ*bufY) + z*(bufY), sizeof(RGBA8)*bufY);
			}
		else
			for (z = 0; z < bufZ; z++)
			{
				for (y = 0; y < bufY; y++)
				{
					slice[z*(copyW)+y] = rgbaBuf[z*(bufY*bufX) + y*(bufX)+x];
				}
			}
		break;
	}
}

void getLimitedSampleScaleBufSize(V3DLONG dim1, V3DLONG dim2, V3DLONG dim3, V3DLONG dim4, V3DLONG dim5,
	float s[5], V3DLONG bs[5], V3DInfo * v3dInfo)
{
	//MESSAGE_ASSERT(s);
	//MESSAGE_ASSERT(bs);

	V3DLONG dimx = dim1;
	V3DLONG dimy = dim2;
	V3DLONG dimz = dim3;
	V3DLONG dimc = dim4;
	V3DLONG dimt = dim5;

	s[0] = (dimx>0) ? MIN(v3dInfo->limit_X, dimx) / float(dimx) : 1;
	s[1] = (dimy>0) ? MIN(v3dInfo->limit_Y, dimy) / float(dimy) : 1;
	s[2] = (dimz>0) ? MIN(v3dInfo->limit_Z, dimz) / float(dimz) : 1;
	s[3] = 1;
	s[4] = 1;

	bs[0] = V3DLONG(s[0] * dim1);
	bs[1] = V3DLONG(s[1] * dim2);
	bs[2] = V3DLONG(s[2] * dim3);
	bs[3] = dim4;
	bs[4] = dim5;
}

QString resourceTextFile(QString filename)
{

	//QFile inputFile(":/subdir/input.txt");
	qDebug() << "Load shader: " << filename;

	QFile inputFile(filename);
	if (inputFile.open(QIODevice::ReadOnly) == false)
		qDebug() << "   *** ERROR in Load shader: " << filename;

	QTextStream in(&inputFile);
	QString line = in.readAll();

	//cout << line.toStdString().c_str() << endl; 

	inputFile.close();
	return line;

}

void Renderer::setZoom(float ratio)
{
	if (ratio <-1) // [-inf,-1) zoom out..............
	{
		ratio = -ratio;
		zoomRatio = 2 + (150 / viewAngle - 1)*(1 - 2 / (1 + ratio));
	}
	else if (ratio < 0) // [-1, 0) zoom out
	{
		ratio = -ratio;
		zoomRatio = 1 + (ratio);
	}
	else	// [0+, +inf] zoom in.....................
	{
		zoomRatio = 1 / (1 + (ratio)*(ratio));
	}

	v3d_debug("Renderer::setZoom zoomRatio:%f", zoomRatio);
}

void ViewRotToModel(mat4 rotateMat, double& vx, double& vy, double& vz)
{
	double  mx, my, mz;
	// mr = Mt * vr,  Mt = Mi for rotation, Mt is row-first-index

	mx = rotateMat[0].x * vx + rotateMat[0].y * vy + rotateMat[0].z * vz;
	my = rotateMat[1].x * vx + rotateMat[1].y * vy + rotateMat[1].z * vz;
	mz = rotateMat[2].x * vx + rotateMat[2].y * vy + rotateMat[2].z * vz; 

	vx = mx; vy = my; vz = mz;
}

//add by Jian 2020/07/10. more smooth mouse move ctrl after preview. Git#187 
void Renderer::viewAnimationRotation(double xRotStep, double yRotStep, double zRotStep)
{
	double rx, ry, rz;
	rx = -xRotStep; ry = yRotStep; rz = zRotStep;
	ViewRotToModel(rotateMat, rx, ry, rz);

	XYZ rotVect(rx, ry, rz);	// [Litone] change by jian 2019/12/28. the rotation direction is different

	double angleStep = norm(rotVect) / (float)ANGLE_TICK;       //qDebug("angle=%f", angle);
	if (angleStep)
	{
		normalize(rotVect);
		animationModelRotation(angleStep, rotVect, rx, ry, rz);
	}

}

void Renderer::animationModelRotation(double angleStep, XYZ rotVect, float xRotStep, float yRotStep, float zRotStep)
{

	dxRot = dyRot = dzRot = 0;
	_absRot = false;
	_animationRotEnabled = true;

	_xRot += xRotStep;
	_yRot += yRotStep;
	_zRot += zRotStep;

	NORMALIZE_angle(_xRot);
	NORMALIZE_angle(_yRot);
	NORMALIZE_angle(_zRot);

	_angleStep = angleStep;
	_rotVect = rotVect;
}