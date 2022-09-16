

#ifndef MY4D_IMAGE_H
#define MY4D_IMAGE_H


//#if defined(USE_Qt5_VS2015_Win7_81) || defined(USE_Qt5_VS2015_Win10_10_14393)
//	#include <QtWidgets>
//#else
//	#include <QtGui>
//#endif
#include <QTime>

#include <mutex>
#include <condition_variable>

#include "../basic_c_fun/v3d_message.h"

#include "../basic_c_fun/stackutil.h"
#include "../basic_c_fun/color_xyz.h"
#include "../basic_c_fun/basic_4dimage.h"

#include "../basic_c_fun/img_definition.h"
#include "../basic_c_fun/volimg_proc.h"
#include "colormap.h"



//////////////////////////////////////////////////////////////////////////////////////////


#include <QBasicTimer>
#include <QPolygonF>
#include <QTextBrowser>


struct CurveTracePara; //in curve_trace_para_dialog.h
struct Options_Rotate;

typedef unsigned short int USHORTINT16;

enum ImagePlaneDisplayType { imgPlaneUndefined, imgPlaneX, imgPlaneY, imgPlaneZ };
enum AxisCode { axis_x, axis_y, axis_z, axis_c };
enum ImageResamplingCode { PRS_Z_ONLY, PRS_X_ONLY, PRS_Y_ONLY, PRS_XY_SAME, PRS_XYZ_SAME };
enum ImageMaskingCode { IMC_XYZ_INTERSECT, IMC_XYZ_UNION, IMC_XY, IMC_YZ, IMC_XZ };

template <class T> int new3dpointer_v3d(T *** & p, V3DLONG sz0, V3DLONG sz1, V3DLONG sz2, unsigned char * p1d);
template <class T> void delete3dpointer_v3d(T *** & p, V3DLONG sz0, V3DLONG sz1, V3DLONG sz2);

template <class T> int new4dpointer_v3d(T **** & p, V3DLONG sz0, V3DLONG sz1, V3DLONG sz2, V3DLONG sz3, unsigned char * p1d);
template <class T> void delete4dpointer_v3d(T **** & p, V3DLONG sz0, V3DLONG sz1, V3DLONG sz2, V3DLONG sz3);


template <class T> QPixmap copyRaw2QPixmap(const T ** p2d, V3DLONG sz0, V3DLONG sz1);
template <class T> QPixmap copyRaw2QPixmap(const T ** p2dRed, const unsigned char ** p2dGreen, const unsigned char ** p2dBlue, V3DLONG sz0, V3DLONG sz1);
template <class T> QPixmap copyRaw2QPixmap_xPlanes(const T **** p4d, V3DLONG sz0, V3DLONG sz1, V3DLONG sz2, V3DLONG sz3, ImageDisplayColorType Ctype, V3DLONG cpos, bool bIntensityRescale, double *p_vmax, double *p_vmin);
template <class T> QPixmap copyRaw2QPixmap_yPlanes(const T **** p4d, V3DLONG sz0, V3DLONG sz1, V3DLONG sz2, V3DLONG sz3, ImageDisplayColorType Ctype, V3DLONG cpos, bool bIntensityRescale, double *p_vmax, double *p_vmin);
template <class T> QPixmap copyRaw2QPixmap_zPlanes(const T **** p4d, V3DLONG sz0, V3DLONG sz1, V3DLONG sz2, V3DLONG sz3, ImageDisplayColorType Ctype, V3DLONG cpos, bool bIntensityRescale, double *p_vmax, double *p_vmin);
template <class T> QPixmap copyRaw2QPixmap(const T **** p4d, V3DLONG sz0, V3DLONG sz1, V3DLONG sz2, V3DLONG sz3, ImageDisplayColorType Ctype, V3DLONG cpos, ImagePlaneDisplayType disType, bool bIntensityRescale, double *p_vmax, double *p_vmin);

//QPixmap copyRaw2QPixmap_colormap(const void **** p4d, ImagePixelType dtype, V3DLONG sz0, V3DLONG sz1, V3DLONG sz2, V3DLONG sz3, V3DLONG cpos, const ColorMap *pc, ImagePlaneDisplayType disType);
//QPixmap copyRaw2QPixmap_xPlanes_colormap(const void **** p4d, ImagePixelType dtype, V3DLONG sz0, V3DLONG sz1, V3DLONG sz2, V3DLONG sz3, V3DLONG cpos, const ColorMap *pc);
//QPixmap copyRaw2QPixmap_yPlanes_colormap(const void **** p4d, ImagePixelType dtype, V3DLONG sz0, V3DLONG sz1, V3DLONG sz2, V3DLONG sz3, V3DLONG cpos, const ColorMap *pc);
//QPixmap copyRaw2QPixmap_zPlanes_colormap(const void **** p4d, ImagePixelType dtype, V3DLONG sz0, V3DLONG sz1, V3DLONG sz2, V3DLONG sz3, V3DLONG cpos, const ColorMap *pc);


struct DataChannelColor
{
	int n;			  // index
	RGBA8 color;
	bool on;
};




class My4DImage : public QObject, public Image4DSimple
{
	Q_OBJECT;

public:
	double at(int x, int y, int z, int c = 0) const; //return a double number because it can always be converted back to UINT8 and UINT16 without information loss
	void **** getData(ImagePixelType & dtype);
	void **** getData() { return data4d_virtual; }
	bool isEmpty() { return (!data4d_virtual) ? true : false; }

	bool valid() {
		return (!data4d_virtual || !(this->Image4DSimple::valid())) ? false : true;
	}

	void updateData4D();
	void loadImage(const char* filename);
	void setupData4D();
	void setupDefaultColorChannelMapping();
	bool updateminmaxvalues();
	void loadImage(V3DLONG imgsz0, V3DLONG imgsz1, V3DLONG imgsz2, V3DLONG imgsz3, int imgdatatype); //an overloaded function to create a blank image

	void setFocusX(V3DLONG x) { curFocusX = (x >= 1 && x <= this->getXDim()) ? x - 1 : -1; }
	void setFocusY(V3DLONG y) { curFocusY = (y >= 1 && y <= this->getYDim()) ? y - 1 : -1; }
	void setFocusZ(V3DLONG z) { curFocusZ = (z >= 1 && z <= this->getZDim()) ? z - 1 : -1; }
	V3DLONG getFocusX() { return curFocusX; }
	V3DLONG getFocusY() { return curFocusY; }
	V3DLONG getFocusZ() { return curFocusZ; }

	My4DImage();
	~My4DImage();

	float **** data4d_float32;
	USHORTINT16 **** data4d_uint16;
	unsigned char **** data4d_uint8;
	void **** data4d_virtual;

	void updateViews();

	bool reshape(V3DLONG rsz0, V3DLONG rsz1, V3DLONG rsz2, V3DLONG rsz3);
	//[Litone]
	bool resize(V3DLONG rsz0, V3DLONG rsz1, V3DLONG rsz2, V3DLONG rsz3);
	//End
	bool permute(V3DLONG dimorder[4]);

	double * p_vmax, *p_vmin; //whole volume max/min values. Use pointer to handle multiple channels separately
	double getChannalMinIntensity(V3DLONG channo);
	double getChannalMaxIntensity(V3DLONG channo);

	bool bLinkFocusViews;
	bool bDisplayFocusCross;
	bool bLookingGlass;

	ColorMap *colorMap;

	void createColorMap(int len, ImageDisplayColorType c = colorPseudoMaskColor);
	void switchColorMap(int len, ImageDisplayColorType c);
	void getColorMapInfo(int &len, ImageDisplayColorType &c);

	V3DLONG curFocusX, curFocusY, curFocusZ;
	

	void cleanExistData_butKeepFileName();
	void cleanExistData();
	void cleanExistData_only4Dpointers();

	QString setFocusFeatureViewText();



	//void setFlagImgValScaleDisplay(bool t) {bImgValScaleDisplay=t;}
	bool getFlagImgValScaleDisplay();

	void setFlagLookingGlass(bool t) { bLookingGlass = t; }
	bool getFlagLookingGlass() { return bLookingGlass; }
	
	QList <DataChannelColor> listChannels; //100824


	int atlasColorBlendChannel;
	bool bUseFirstImgAsMask;
	QString curSearchText;

	//timer //20120702
	QTime triviewTimer;  //added on 20120702.
	bool b_triviewTimerON;
	
	bool saveMovie();
	bool saveFile();
	bool saveFile(char filename[]);
	bool saveFile(QString outputFile);
	void crop(int landmark_crop_opt);
	void crop(V3DLONG bpos_x, V3DLONG epos_x, V3DLONG bpos_y, V3DLONG epos_y, V3DLONG bpos_z, V3DLONG epos_z, V3DLONG bpos_c, V3DLONG epos_c, int landmark_crop_opt);

	bool invertcolor(int channo); //channo < 0 will invert all channels. Only works for uint8
	bool scaleintensity(int channo, double lower_th, double higher_th, double target_min, double target_max); //map the value linear from [lower_th, higher_th] to [target_min, target_max].
	bool thresholdintensity(int channo, double th); //anything < th will be 0, others unchanged
	bool binarizeintensity(int channo, double th); //anything < th will be 0, others will be 1

	bool rotate(ImagePlaneDisplayType ptype, const Options_Rotate & r_opt);
	bool flip(AxisCode my_axiscode);

	bool b_proj_worm_mst_diameter_set;
	
	V3DLONG last_hit_landmark, cur_hit_landmark;
	BoundingBox trace_bounding_box;
	float trace_z_thickness;

	
	static const int MAX_history = 30;
	int cur_history;
	
};




struct BlendingImageInfo
{
	My4DImage * pimg;
	int channo;
	double rr, gg, bb;
};




#endif // v3d_core.h

