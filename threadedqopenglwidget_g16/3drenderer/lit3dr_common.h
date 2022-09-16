
#ifndef LIT3DR_COMMON_H_
#define LIT3DR_COMMON_H_

#include "color_xyz.h"
#include "v3d_message.h"  //log
#include "v3d_basicdatatype.h"
#include "basic_4dimage.h"
#include "my4dimage.h"
#include <QtCore>

//if define USE_SIMU, use simulation data: t3d_t_0_p_0.tiff, if undefine, use Data-Axolotl-2020-09-08//multiview_0//multiview_0_t_0_p_0.tiff 
#define USE_SIMU_DATA 
//#define TEST_SPARSE_TEXTURE
//if define MOC_DATA_FOR_SPARSE_TEXTURE, will update the 1/2 of X Y block to be white for test purpose
//#define MOC_DATA_FOR_SPARSE_TEXTURE

//#define MOC_DATA_FOR_SPARSE_TEXTURE_RGB_SEP
//
//#define HALF_Z_BIN2

//*
#define ENABLE_MT
//*

#define CHECK_GLError_print() CheckGLError_print(__FILE__, __LINE__)

#define ZSCALE_REDUCE_ARTIFCAT 1

//*
#define GREY16_TEXT
//*

//*//
#define TEXTURE_ARRAY
//*//


//=====================================================================================================================
//#ifdef WIN32  //081001: limited by 3G
//	#define LIMIT_VOLX 512
//	#define LIMIT_VOLY 512
//	#define LIMIT_VOLZ 128
//#else
	#define LIMIT_VOLX 512
	#define LIMIT_VOLY 512
	#define LIMIT_VOLZ 256
//#endif
//	#define LIMIT_VOLX 256
//	#define LIMIT_VOLY 256

#define IS_LESS_64BIT ((sizeof(void*)<8)? true:false)
#define IS_FITTED_VOLUME(dim1,dim2,dim3)  (dim1<=LIMIT_VOLX && dim2<=LIMIT_VOLY && dim3<=LIMIT_VOLZ)

#define ANGLE_X0 (15)			//degree
#define ANGLE_Y0 (360-20)		//degree
#define ANGLE_Z0 (360-2)		//degree
#define ANGLE_TICK 1			//divided
#define MOUSE_SENSITIVE 1.0f
#define SHIFT_RANGE 100 		//percent of bound
#define ZOOM_RANGE  100         //percent of fov
#define ZOOM_RANGE_RATE 5       //zoom rate of fov
#define CLIP_RANGE  200 		//size of (-100,100)
#define ZTHICK_RANGE 20			//times
#define TRANSPARENT_RANGE 100   //nonlinear divided

#define POPMENU_OPACITY 1

#define WIN_SIZEX 1024 //800
#define WIN_SIZEY 768  //800
#define CTRL_SIZEX 350
#define MINVIEW_SIZEX 700  //800
#define MINVIEW_SIZEY 700  //800

#define QEvent_Ready (QEvent::User +1)
#define QEvent_OpenFiles (QEvent::User +2)
#define QEvent_DropFiles (QEvent::User +3)
#define QEvent_InitControlValue (QEvent::User +4)
#define QEvent_OpenLocal3DView (QEvent::User +5)
#define QEvent_HistoryChanged (QEvent::User +6) //20170801 RZC: a notify after Undo Histoty changed
#define QEvent_V3d_WaitDialogStart (QEvent::User +7) //add by eva 2019-7-11
#define QEvent_V3d_WaitDialogClose  (QEvent::User +8) //add by eva 2019-7-11


//========================================================================================================================
#ifdef Q_WS_MAC
#define CTRL2_MODIFIER Qt::MetaModifier
#else
#define CTRL2_MODIFIER Qt::ControlModifier
#endif

#define ALT2_MODIFIER Qt::AltModifier
#ifdef Q_WS_X11
#define ATL2_MODIFIER Qt::GroupSwitchModifier
#endif


#define POST_EVENT(pQ, eventType) {	if (pQ!=NULL) QCoreApplication::postEvent(pQ, new QEvent( QEvent::Type(eventType) )); }
#define SEND_EVENT(pQ, eventType) {	if (pQ!=NULL) QCoreApplication::sendEvent(pQ, new QEvent( QEvent::Type(eventType) )); }
#define POST_CLOSE(pQ)	POST_EVENT(pQ, QEvent::Close)
#define ACTIVATE(w)	  { if(w) {QWidget* pQ=(QWidget*)w; pQ->raise(); POST_EVENT(pQ, QEvent::MouseButtonPress);} }

#define SLEEP(t)  { QTime qtime;  qtime.start();  while( qtime.elapsed() < t); }

#define DELETE_AND_ZERO(p)	{ if ((p)!=NULL) delete (p); (p) = NULL; }
#define Q_CSTR(qs)  ( (qs).toStdString().c_str() )
#define QCOLOR_BGRA8(bgra)  ( QColor::fromRgba((unsigned int)(bgra)) )  //QRgb = #AArrGGbb = BGRA8

#define MESSAGE(s) \
{\
	QMessageBox::information(0, "MESSAGE", QObject::tr("%1 \n\n in file(%2) at line(%3)").arg(s).arg(__FILE__).arg(__LINE__)); \
}
#define MESSAGE_ASSERT(s) \
{\
	if (!(s)) \
		QMessageBox::critical(0, "ASSERT", QObject::tr("ASSERT(%1) in file(%2) at line(%3)").arg(#s).arg(__FILE__).arg(__LINE__)); \
	Q_ASSERT(s); \
}

#define CATCH_TO_QString( type, msg ) \
	catch (std::exception& e) { \
		type = "[std]"; \
		msg = e.what(); \
	} \
	catch (int id) { \
		type = "[int]"; \
		msg = QString("exception id = %1").arg(id); \
	} \
	catch (V3DLONG id) { \
		type = "[V3DLONG]"; \
		msg = QString("exception id = %1").arg(id); \
	} \
	catch (unsigned int id) { \
		type = "[uint]"; \
		msg = QString("exception id = %1").arg(id); \
	} \
	catch (unsigned V3DLONG id) { \
		type = "[ulong]"; \
		msg = QString("exception id = %1").arg(id); \
	} \
	catch (char* str) { \
		type = "[char]"; \
		msg = (const char*)str; \
	} \
	catch (const char* str) { \
		type = "[cchar]"; \
		msg = str; \
	} \
	catch (...) { \
		type = "[unknown]"; \
		msg = QString("unknown exception in file(%1) at line(%2)").arg(__FILE__).arg(__LINE__)); \
	}





// 090424RZC: because QColorDialog::getColor cannot handle correctly when user clicks Cancel
inline bool v3dr_getColorDialog( QColor *color, QWidget *parent=0)
{
	QRgb input = 0xff000000;
	if (color)	input = color->rgba();
	bool ok;
	//QRgb ouput = QColorDialog::getRgba(input, &ok, parent);  //also include alpha channel
	//if (ok && color)  *color = QColor::fromRgba( ouput );
	return ok;
}

//inline QColor QColorFromRGBA8(RGBA8 c)
//[QT5-MK]
inline  QColor QColorFromRGBA8(RGBA8 c)
{
	return QColor(c.c[0], c.c[1], c.c[2], c.c[3]);
}

//inline RGBA8 RGBA8FromQColor(QColor qc)
//[QT5-MK]
 inline RGBA8 RGBA8FromQColor(QColor qc)
{
	RGBA8 c;
	c.r=qc.red(); c.g=qc.green(); c.b=qc.blue(); c.a=qc.alpha();
	return c;
}

#define QCOLOR(rgba8)   QColorFromRGBA8( rgba8 )
#define VCOLOR(rgba8)   qVariantFromValue(QColorFromRGBA8( rgba8 ))

#if defined(USE_Qt5_VS2015_Win7_81) || defined(USE_Qt5_VS2015_Win10_10_14393)
#define QCOLORV(var)    (var.value<QColor>( ))
 //[QT5_MK]
//#define RGBA8V(var)     RGBA8FromQColor(var.value<QColor>( ))
 inline RGBA8 RGBA8V(QVariant var)
 {
	 return RGBA8FromQColor(var.value<QColor>());
 }
#else
#define QCOLORV(var)    (qVariantValue<QColor>( var ))
#define RGBA8V(var)     RGBA8FromQColor(qVariantValue<QColor>( var ))
#endif

 class V3DInfo {
 public:
	 V3DInfo() {};
	 void setOnline() { online3D = true; limit_X = limit_Y = 512; limit_Z = 256; };
	 void setOffline() { online3D = false; };
	 void setLimitSize(int X, int Y, int Z) { limit_X = X; limit_Y = Y; limit_Z = Z; };
	 void setLimitSize(int X, int Y, int Z, bool compress) {
		 limit_X = X; limit_Y = Y; limit_Z = Z; textureCompress = compress;
	 };

	 bool online3D = false;
	 int limit_X = 512;
	 int limit_Y = 512;
	 int limit_Z = 256;
	 bool textureCompress = false;
 };

 struct Lit3drParam
 {
	 My4DImage* image4d;
	 My4DImage* image4d_bin2;

	 My4DImage* image4d_raw;

	
	 int b_local; // 0,1,2,3

	 QPoint local_win_pos;
	 QSize local_win_size;

	 bool b_use_512x512x256;
	 bool b_still_open;

	 RGBA8 *total_rgbaBuf, *rgbaBuf;
	 float sampleScale[5];
	 V3DLONG bufSize[5]; //(x,y,z,c,t) 090731: add time dim
	 bool b_limitedsize;
	 bool isSimulatedData;
	 int data_unitbytes;
	 unsigned char* data4dp;
	 unsigned char**** data4d_uint8;

	 //V3DLONG timePointSize, positionSize;//add by eva 2019-1-4
	 V3DLONG dim1, dim2, dim3, dim4, dim5;
	 V3DLONG start1, start2, start3, start4, start5;
	 V3DLONG size1, size2, size3, size4, size5;
	 BoundingBox dataBox;
	 BoundingBox dataViewProcBox;
	 bool bSavedDataFor3DViewer;

	 // for texture
	 int imageX, imageY, imageZ, imageT;
	 int safeX, safeY, safeZ;
	 int realX, realY, realZ, realF;
	 int fillX, fillY, fillZ, fillF;



	 Lit3drParam() {
		 b_use_512x512x256 = true; b_local = b_still_open = false;
		 image4d = 0; 
		 // for saving resampled
		 total_rgbaBuf = 0; rgbaBuf = 0; b_limitedsize = true; isSimulatedData = false;
		 sampleScale[0] = sampleScale[1] = sampleScale[2] = sampleScale[3] = sampleScale[4] = 1;
		 bufSize[0] = bufSize[1] = bufSize[2] = bufSize[3] = bufSize[4] = 0;
		 data4dp = 0; data4d_uint8 = 0;
		 dim1 = dim2 = dim3 = dim4 = dim5 = 0;
		 start1 = start2 = start3 = start4 = start5 = 0;
		 size1 = size2 = size3 = size4 = size5 = 0;
		 bSavedDataFor3DViewer = true;
		 /*zthickness=1;*/
	 }
	 ~Lit3drParam() {
		 if (image4d) delete image4d;
		 DELETE_AND_ZERO(total_rgbaBuf);
		 DELETE_AND_ZERO(rgbaBuf); DELETE_AND_ZERO(data4dp);
	 }
 };

#endif /* LIT3DR_COMMON_H_ */
