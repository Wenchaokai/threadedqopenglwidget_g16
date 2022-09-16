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

#ifndef RENDERER_H
#define RENDERER_H

//#include <GL\glew.h>
//#include <QtOpenGL/QtOpenGL>

//#include <qopenglext.h>

//#include <QtOpenGLWidgets/QOpenGLWidget>
//#include <QOpenGLFunctions>
//#include <QOpenGLExtraFunctions>
//#include <QOpenGLShaderProgram>
//#include <QOpenGLBuffer>
//#include <QOpenGLTexture>


#include <QVector3D>
#include <QMatrix4x4>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QElapsedTimer>


using GLuint = unsigned int;
using GLdouble = double;
using GLint = int;
using GLenum = unsigned int;
class QOpenGLShader;
class QOpenGLTexture;
class QOpenGLShaderProgram;
class QOpenGLBuffer;

//#include <qopengl.h> 

//#include "glwidget.h"

class GLWidget;

// include files
#include "glsl_r.h"
#include "lit3dr_common.h"
#include "core_model.h"
#include "backfill_model.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/constants.hpp>

// lookup and mix multi-channel to RGBA8
#define OP_MAX	0
#define OP_SUM	1
#define OP_MEAN	2
#define OP_OIT	3  //Order Independent Transparency
#define OP_INDEX	-1


struct MixOP
{
	int op;
	bool rescale;
	bool maskR, maskG, maskB;
	float brightness, contrast; //ratio
	MixOP() {
		op = OP_MAX;  rescale = true;
		maskR = maskG = maskB = true;
		brightness = 0; contrast = 1;
	}
};

struct Channel
{
	int n;				// index
	bool on;
	RGBA8 color;
	Channel() { n = 0; on = true;  color.r = color.g = color.b = color.a = 255; }
	int lutNo = -1;
};
struct ChannelSharedData
{
	MixOP mixOp;
	QList<Channel> listChannel;
	QVector< QVector<RGBA8> > luts;
	bool bGlass;
};


//inline
void make_linear_lut_one(RGBA8 color, QVector<RGBA8>& lut);
//inline
RGB8 lookup_mix_litone(unsigned char* mC, int N, unsigned int* channel_index_On, int channels_On, RGBA16** mLut, int op, RGB8 mask = XYZ(255, 255, 255));


// utilities method
QString resourceTextFile(QString filename);

//Helping function
inline void set_colormap_curve(QPolygonF &curve, qreal x, int iy) // 0.0<=(x)<=1.0, 0<=(iy)<=255
{
	x = qMax(0.0, qMin(1.0, x));
	qreal y = qMax(0.0, qMin(1.0, iy / 255.0));
	curve << QPointF(x, y);
}

void getLimitedSampleScaleBufSize(V3DLONG dim1, V3DLONG dim2, V3DLONG dim3, V3DLONG dim4, V3DLONG dim5,
	float sampleScale[5], V3DLONG bufSize[5], V3DInfo * v3dInfo);

bool reshapeImageStack(Image4DSimple & src, Image4DSimple & dest, int axis, int nthreads);

void _copySliceFromStack(RGBA8* rgbaBuf, int imageX, int imageY, int imageZ,
	RGBA8* slice, int copyW, int stack_i, int slice_i,
	RGBA8* rgbaYzx = 0, RGBA8* rgbaXzy = 0);


#define CATCH_handler( func_name ) \
	catch (std::exception& e) { \
		\
		v3d_error("    *** std exception occurred in %s", func_name); \
		v3d_debug("%s %s %s", func_name, " std", e.what()); \
		\
	} catch (const char* str) { \
		\
		v3d_error("    *** GLSL exception occurred in %s", func_name); \
		v3d_debug("%s %s %s", func_name, "GLSL", str); \
		\
	} catch (...) { \
		\
		v3d_debug("%s %s %s", func_name, "UNKOWN", "unknown exception" ); \
		\
	}

#define N_CHANNEL 3
#define FILL_CHANNEL 4  // power_of_two

#define MAT4x4_TO_MAT16(m4x4, m16)  memcpy( (m16), &((m4x4)[0][0]), sizeof(m4x4))
#define MAT16_TO_MAT4x4(m16, m4x4)  memcpy( &((m4x4)[0][0]), (m16), sizeof(m4x4))

#define TEXTURE_UNIT0_3D(b) \
if (b) \
{ \
	texture_unit0_3D = true;  glDisable(GL_TEXTURE_2D);  glEnable(GL_TEXTURE_3D); \
} \
else \
{ \
	texture_unit0_3D = false; glDisable(GL_TEXTURE_3D);  glEnable(GL_TEXTURE_2D); \
}


///////////////////////////////////////////////////////////////////////////////
#define __interaction_controls__

#define NORMALIZE_angle( angle ) \
{ \
    while (angle < 0)                  angle += 360 * ANGLE_TICK; \
    while (angle > 360 * ANGLE_TICK)   angle -= 360 * ANGLE_TICK; \
}
#define NORMALIZE_angleStep( angle ) \
{ \
    while (angle < -180 * ANGLE_TICK)   angle += 360 * ANGLE_TICK; \
    while (angle >  180 * ANGLE_TICK)   angle -= 360 * ANGLE_TICK; \
}


//End

enum RenderMode {
	rmCrossSection = 0,
	rmAlphaBlendingProjection,
	rmMaxIntensityProjection,
	rmMinIntensityProjection,
};

enum v3dr_VolSlice {
	vsSliceNone = 0,
	vsXslice = 1,
	vsYslice = 2,
	vsZslice = 3,
	vsFslice = 4,
};

enum v3dr_DataClass {
	dcDataNone = 0,
	dcVolume = 1,
	dcSurface = 2
};

void linkGLShader(cwc::glShaderManager& SMgr,
	cwc::glShader*& shader, //output
	const char* vertex, const char* fragment);

class Renderer : public QObject //, protected QOpenGLExtraFunctions
{
    Q_OBJECT

public:
    Renderer(GLWidget *w);
	~Renderer();
	void lockRenderer() {
		m_renderMutex.lock();
	}
    void unlockRenderer() { 
		m_renderMutex.unlock();
	}
    QMutex *grabMutex() { return &m_grabMutex; }
    QWaitCondition *grabCond() { return &m_grabCond; }
    void prepareExit() { m_exiting = true; m_grabCond.wakeAll(); }

	void initialize();
	void initialize_grey16();

	void loadShader_litone_glsl_r_grey16();
	void cleanShader_glsl_r_grey16();


signals:
    void contextWanted();

public slots:
    void render();

private:
    void paintQtLogo();

	void paint_litone();

	void renderVol();

	void setUnitVolumeSpace();

	void drawBackFillVolCube();

	void drawCrossLine(float lineWidth);

	void equAlphaBlendingProjection();

	void equMaxIntensityProjection();

	void equMinIntensityProjection();

	void equCrossSection();

	void drawVol();

	void updateVolCutRange();

	void drawUnitVolume();

	void shaderTexBegin_glsl(bool stream);

	void shaderTexBegin_glsl_grey16(bool stream);

	void shaderTexEnd_glsl();

	void shaderTexEnd_glsl_grey16();

	void drawStackZ(float direction, int section, bool t3d, bool stream);

	void drawStackZ_grey16(float direction, int section, bool t3d, bool stream, int channels);

	void drawStackY_grey16(float direction, int section, bool t3d, bool stream, int channels);

	void drawStackX_grey16(float direction, int section, bool t3d, bool stream, int channels);

	void _drawStack_grey16(double ts, double th, double tw, double s0, double s1, double h0, double h1, double w0, double w1, double ds, int slice0, int slice1, int thickness, GLuint tex3D, GLuint texs[], int stack_i, float direction, int section, bool b_tex3d, bool b_stream, int channels);

	void createGeometry();
    void quad(qreal x1, qreal y1, qreal x2, qreal y2, qreal x3, qreal y3, qreal x4, qreal y4);
    void extrude(qreal x1, qreal y1, qreal x2, qreal y2);

	void init_members();

public: 
	// [Start] Sample
	QColor clearColor;
	int xRot;
	int yRot;
	int zRot;

	//[End] sample
	bool m_inited;
	bool inRending = false; 
	bool m_renderMutexLocked = false; 

private:
	//QVector<cgElement*> vecElements;
	mat4 rotateMat;

	mat4 projectionMat;
	mat4 viewMat;

	vec3 viewPos;
	vec3 viewHead;

	CoreModel* core_model;
	BackFillModel* backfill_model;

	int isinit = 0;

	int render_times = 0;

    //bool m_inited;
    qreal m_fAngle;
    qreal m_fScale;
    QVector<QVector3D> vertices;
    QVector<QVector3D> normals;
    QOpenGLShaderProgram* m_program;
    QOpenGLBuffer* m_vbo;

#ifdef USE_SIMU_DATA

	static const int max_zSlice = 127;
	static const int z_scale = 1;
#else
	//static const int max_zSlice = 281; 
	static const int max_zSlice = 281;
	static const int z_scale = 1; 
#endif
	//static const int max_zSlice = 10;

	QOpenGLTexture *textures[max_zSlice];

	//[End] sample

    int vertexAttr;
    int normalAttr;
    int matrixUniform;
    GLWidget *m_glwidget;
    QMutex m_renderMutex;
    QElapsedTimer m_elapsed;
    QMutex m_grabMutex;
    QWaitCondition m_grabCond;
    bool m_exiting;

	//shaders
	cwc::glShaderManager SMgr;
	cwc::glShader *shader_obj, *shader_backfill;
	cwc::glShader *shader_glsl, *shaderTex2D, *shaderTex3D, *shaderObj, *shaderTex2D_grey16;

	//Uniform Value 
	int volume_uniloc, colormap_uniloc, channel_uniloc, blend_mode_uniloc, format_bgra_uniloc; 

	//
	//glShader *shader, *shaderTex2D, *shaderTex3D;

	GLuint texColormap; // nearest filter, [x-coord for intensity][y-coord for channel]

	QPolygonF colormap_curve[N_CHANNEL][4]; // [n-channel][RGBA]
	RGBA8 colormap[FILL_CHANNEL][256];      // [n-channel][256-intensity]

	GLuint texLuts;
	RGBA16 texLutsData[FILL_CHANNEL][256];

	GLuint texHistRange;
	HISTRange16 texHistData[FILL_CHANNEL]; // [n-channel][on/off, min, max, vrange, color]

	//RGBA8 *total_rgbaBuf, *rgbaBuf;
	float sampleScale[5];
	V3DLONG bufSize[5]; //(x,y,z,c,t) 090731: add time dim

	V3DLONG dim1, dim2, dim3, dim4, dim5;
	V3DLONG start1, start2, start3, start4, start5;
	V3DLONG size1, size2, size3, size4, size5;
	BoundingBox dataBox;
	BoundingBox dataViewProcBox; //current clip box that data are visible (and thus are processable). 091113 PHC

	bool texture_unit0_3D, tex_stream_buffer, drawing_fslice;

	//*//
	//for rgb
	GLuint textureID_rgb;
	//for 16grey
	GLuint textureID[3];
	//*//
	GLuint tex3D, texFslice;
	GLenum texture_format, image_format, image_type;

	GLuint *Ztex_list, *Ytex_list, *Xtex_list;

	RGBA8 *Zslice_data, *Yslice_data, *Xslice_data, *Fslice_data;
	
	unsigned short *Zslice_data_g16, *Yslice_data_g16, *Xslice_data_g16, *Fslice_data_g16;

	RGBA8 *rgbaBuf_Yzx, *rgbaBuf_Xzy;
	float thicknessX, thicknessY, thicknessZ;
	float thicknessXY_real_times;//add by eva 2019-11-13, if z-thick<1, set thicknessZ=1, thicknessX=thicknessX/z-thick and so as thicknessY, thicknessXY_real_times = z-thick. see more in renderer_obj.cpp Renderer_gl1::setThickness(double t).
	float sampleScaleX, sampleScaleY, sampleScaleZ;
	int imageX, imageY, imageZ, imageT;
	int safeX, safeY, safeZ;
	int realX, realY, realZ, realF;
	int fillX, fillY, fillZ, fillF;
	GLdouble volumeViewMatrix[16]; // for choosing stack direction

	GLint viewport[4];
	GLdouble projectionMatrix[16];
	GLdouble markerViewMatrix[16];

	float VOL_X1, VOL_X0, VOL_Y1, VOL_Y0, VOL_Z1, VOL_Z0;

	int VOLUME_FILTER;
	
	RGBA32f SLICE_COLOR; // proxy geometry color+alpha

	float CSbeta, alpha_threshold;
	float CSbeta_high, alpha_threshold_high; //add by Litone 2019/11/21

	RGBA32f color_background, color_background2, color_line, color_proxy;

	bool bShowBoundingBox, bShowBoundingBox2, bShowAxes, bOrthoView;
	bool bShowCSline, bShowFSline, bFSlice, bXSlice, bYSlice, bZSlice;
	int lineType, lineWidth, nodeSize, rootSize;
	int polygonMode, tryObjShader;
	double thickness; //changed from int to double, PHC, 090215
	V3DLONG xCut0, xCut1, yCut0, yCut1, zCut0, zCut1;            // for volume
	float xClip0, xClip1, yClip0, yClip1, zClip0, zClip1;    // for surface
	float viewClip;

	V3DLONG screenW, screenH;
	
	GLdouble viewNear, viewFar, viewAngle;
	float viewDistance, zoomRatio;

	bool b_error;
	bool b_selecting;

	bool b_limitedsize;
	float *depth_buffer;

	RenderMode renderMode;
	
	GLdouble mRot[16];
	static const int flip_X = +1, flip_Y = -1, flip_Z = -1; // make y-axis downward conformed with image coordinate
	QPoint lastPos;

	float _xRot, _yRot, _zRot, dxRot, dyRot, dzRot;
	// int _zoom, _xShift, _yShift, _zShift, dxShift, dyShift, dzShift;
	float _zoom, _xShift, _yShift, _zShift, dxShift, dyShift, dzShift; // CMB 2011 Feb 07

	bool _animationRotEnabled = false;
	XYZ	  _rotVect;
	float _angleStep = 0;

	bool _absRot = false; 

	V3DInfo *v3dInfo;

	//sparse texture 


	GLuint num_page_Sizes;
	GLuint page_sizes_x[10];
	GLuint page_sizes_y[10];
	GLuint page_sizes_z[10];// Figure out how many page sizes are available for a 2D texture

	GLuint PAGE_SIZE_X, PAGE_SIZE_Y;

public:

	virtual void initColormap();

	bool initLookupTableG16();

	bool initHistRange();

	void loadVol();

	void loadVol_grey16();

	int _getTexFillSize(int w);

	int _getBufFillSize(int w);

	void subloadTex(V3DLONG v, V3DLONG timepoint, V3DLONG position, bool bfirst);
	void subloadTex_grey16(V3DLONG v, V3DLONG timepoint, V3DLONG position, bool bfirst);
	void setupData(Lit3drParam& param);
	void setupStackTexture(V3DLONG v, V3DLONG timepoint, V3DLONG position, bool bfirst);

	void setupStackTexture_grey16(V3DLONG v, V3DLONG timepoint, V3DLONG position, bool bfirst);

	void setupStackArrayTexture_grey16(V3DLONG v, V3DLONG timepoint, V3DLONG position, bool bfirst);

	void setTexParam2D();
	void setTexParam3D();
	void Renderer::setTexParamArray();
	void setupView(int width, int height);

	void setProjection();

	void setViewPos();

	void blendBrighten(float fbright, float fcontrast);

	void setZoom(float ratio);

	void viewAnimationRotation(double xRotStep, double yRotStep, double zRotStep);

	void animationModelRotation(double angleStep, XYZ rotVect, float xRotStep, float yRotStep, float zRotStep);

public: 
	BoundingBox boundingBox; 

	My4DImage *imgStack_rgb, *imgStack_rgb_bin2; //XY 

	My4DImage *imgStack_raw_StackZ;
	Image4DSimple *imgStack_raw_StackY, *imgStack_raw_StackX;

	
public: //frame buffer

	GLuint framebuffer;
	GLuint textureColorbuffer;
	GLuint rbo;

};

void ViewRotToModel(mat4 modelmat, float& vx, float& vy, float& vz); 

#endif
