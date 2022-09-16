

   // avoid compile error from late load of windows.h
#ifdef _MSC_VER
#define NOMINMAX //added by PHC, 2010-05-20 to overcome VC min max macro
#include <windows.h>
#endif

#include <stdio.h>
#include <math.h>

#include "my4dimage.h"
#include <QtWidgets>


#include <fstream>
#include <iostream>
using namespace std;

template <class T> int new3dpointer_v3d(T *** & p, V3DLONG sz0, V3DLONG sz1, V3DLONG sz2, unsigned char * p1d)
{
	if (p != 0) { return 0; } //if the "p" is not empty initially, then do nothing and return un-successful

	p = new T **[sz2];
	if (!p) { return 0; }

	for (V3DLONG i = 0; i < sz2; i++)
	{
		p[i] = new T *[sz1];
		if (!p[i])
		{
			for (V3DLONG j = 0; j < i; j++) { delete[](p[i]); }
			delete[]p;
			p = 0;
			return 0;
		}
		else
		{
			for (V3DLONG j = 0; j < sz1; j++)
				p[i][j] = (T *)(p1d + i*sz1*sz0 * sizeof(T) + j*sz0 * sizeof(T));
		}
	}

	return 1;
}

template <class T> void delete3dpointer_v3d(T *** & p, V3DLONG sz0, V3DLONG sz1, V3DLONG sz2)
{
	if (p == 0) { return; } //if the "p" is empty initially, then do nothing

	for (V3DLONG i = 0; i < sz2; i++)
	{
		if (p[i])
		{
			delete[](p[i]);
			p[i] = 0;
		}
	}

	delete[] p;
	p = 0;

	//stupid method to remove stupid warnings
	sz0 = sz0;
	sz1 = sz1;

	return;
}

template <class T> int new4dpointer_v3d(T **** & p, V3DLONG sz0, V3DLONG sz1, V3DLONG sz2, V3DLONG sz3, unsigned char * p1d)
{
	if (p != 0) { return 0; } //if the "p" is not empty initially, then do nothing and return un-successful

	p = new T ***[sz3];
	if (!p) { return 0; }

	for (V3DLONG i = 0; i < sz3; i++)
	{
		p[i] = 0; //this sentence is very important to assure the function below knows this pointer is initialized as empty!!
		if (!new3dpointer_v3d(p[i], sz0, sz1, sz2, p1d + i*sz2*sz1*sz0 * sizeof(T)))
		{
			v3d_msg("Problem happened in creating 3D pointers for channel.\n", false);
			for (V3DLONG j = 0; j < i; j++) { delete[](p[i]); }
			delete[]p;
			p = 0;
			return 0;
		}
	}

	return 1;
}

template <class T> void delete4dpointer_v3d(T **** & p, V3DLONG sz0, V3DLONG sz1, V3DLONG sz2, V3DLONG sz3)
{
	if (p == 0) { return; } //if the "p" is empty initially, then do nothing

	for (V3DLONG i = 0; i < sz3; i++)
	{
		delete3dpointer_v3d(p[i], sz0, sz1, sz2);
	}

	delete[] p;
	p = 0;

	//stupid method to remove stupid warnings
	sz0 = sz0;
	sz1 = sz1;
	sz2 = sz2;

	return;
}

My4DImage::My4DImage()
{
	data4d_float32 = 0;
	data4d_uint16 = 0;
	data4d_uint8 = 0;
	data4d_virtual = 0; //a pointer used to link the external src data

	curFocusX = -1; //set as invalid values
	curFocusY = -1;
	curFocusZ = -1;


	bLookingGlass = false;

	p_vmax = NULL;
	p_vmin = NULL;

	colorMap = new ColorMap(colorPseudoMaskColor, 256);

	b_proj_worm_mst_diameter_set = false;

	//global_setting.GPara_landmarkMatchingMethod = MATCH_MI; //080820

	atlasColorBlendChannel = 1; //081206
	bUseFirstImgAsMask = false; //081207
	curSearchText = "Enter search text here";

	last_hit_landmark = cur_hit_landmark = -1; //-1 is invalid for default. by PHC, 090119

	trace_bounding_box = NULL_BoundingBox; //090727 RZC: for trace from local view
	trace_z_thickness = 1; //090727 RZC: weight for z-distance of graph

	b_triviewTimerON = false;

}

My4DImage::~My4DImage()
{
	cleanExistData();

	if (colorMap) { delete colorMap; colorMap = 0; } 

}



void **** My4DImage::getData(ImagePixelType & dtype)
{
	dtype = this->getDatatype();
	if (dtype == V3D_UINT8 || dtype == V3D_UINT16 || dtype == V3D_FLOAT32)
		return data4d_virtual;
	else
		return NULL; //temnporarily allow only UINT8, UINT16, and FLOAT32 data
}

double My4DImage::at(int x, int y, int z, int c) const //return a double number because it can always be converted back to UINT8 and UINT16 without information loss
{ //return -1 in case error such as x,y,z,c are illegal values
	bool result = (!data4d_virtual || x < 0 || y < 0 || z < 0 || c < 0 ||
		x >= this->getXDim() || y >= this->getYDim() || z >= this->getZDim() || c >= this->getCDim());
	if (result)
	{
		//v3d_msg("error happened. Check the command line debuging info.");
		printf("error happened. p=%ld x=%d y=%d z=%d c=%d\n", (V3DLONG)data4d_virtual, x, y, z, c);
		return -1;
	}

	switch (this->getDatatype())
	{
	case V3D_UINT8:
		return double(data4d_uint8[c][z][y][x]);
		break;
	case V3D_UINT16:
		return double(data4d_uint16[c][z][y][x]);
		break;
	case V3D_FLOAT32:
		return double(data4d_float32[c][z][y][x]);
		break;
	default:
		return -1;
		break;
	}
}


void My4DImage::loadImage(const char* filename)
{
	cleanExistData();

	bool b_useMylib = false;


	bool lsmFlag = false;
	bool tiffFlag = false;
	QString qFilename = QString(filename);

	if (qFilename.endsWith("lsm") || qFilename.endsWith("LSM")) {
		lsmFlag = true;
	}
	else if (qFilename.endsWith("tif") || qFilename.endsWith("TIF") || qFilename.endsWith("tiff") || qFilename.endsWith("TIFF")) {
		tiffFlag = true;
	}

	if (lsmFlag) {
		b_useMylib = true;
	}
	else if (tiffFlag) {
		b_useMylib = false;
	}
	/*else if (V3dApplication::getMainWindow()) {
		b_useMylib = V3dApplication::getMainWindow()->global_setting.b_UseMylibTiff;
		qDebug() << "My4DImage::loadImage() set b_useMylib to value=" << b_useMylib << " based on global settings from MainWindow";
	}*/


	qDebug() << "My4DImage::loadImage() calling Image4DSimple::loadImage() with b_useMylib=" << b_useMylib;

	Image4DSimple::loadImage(filename, b_useMylib);

	setupData4D();
}

bool My4DImage::reshape(V3DLONG rsz0, V3DLONG rsz1, V3DLONG rsz2, V3DLONG rsz3)
{
	//if (!data4d_uint8) {v3d_msg("now only support unit8 in reshape().");  return false;}

	if (this->getXDim() == rsz0 && this->getYDim() == rsz1 && this->getZDim() == rsz2 && this->getCDim() == rsz3)
	{
		v3d_msg("The dimensions are the same. Do nothing.\n");
		return true;
	}
	if (this->getXDim()*this->getYDim()*this->getZDim()*this->getCDim() != rsz0*rsz1*rsz2*rsz3)
	{
		v3d_msg("The dimensions do not match. The total number of pixels are not the same. Do nothing.\n");
		return false;
	}

	cleanExistData_only4Dpointers();

	this->setXDim(rsz0);
	this->setYDim(rsz1);
	this->setZDim(rsz2);
	this->setCDim(rsz3);

	setupData4D();

	//update GUI

	curFocusX = this->getXDim() / 2; //-= bpos_x+1; //begin from first slices
	curFocusY = this->getYDim() / 2; //-= bpos_y+1;
	curFocusZ = this->getZDim() / 2; //-= bpos_z+1;

	//update the color display mode, as the number of channels could change
	updateViews();

}

bool My4DImage::resize(V3DLONG rsz0, V3DLONG rsz1, V3DLONG rsz2, V3DLONG rsz3)
{
	//if (!data4d_uint8) {v3d_msg("now only support unit8 in reshape().");  return false;}

	if (this->getXDim() == rsz0 && this->getYDim() == rsz1 && this->getZDim() == rsz2 && this->getCDim() == rsz3)
	{
		v3d_msg("The dimensions are the same. Do nothing.\n");
		return true;
	}
	if (this->getXDim()*this->getYDim()*this->getZDim()*this->getCDim() != rsz0*rsz1*rsz2*rsz3)
	{
		v3d_msg("The dimensions do not match. The total number of pixels are not the same. Do nothing.\n");

		cleanExistData();
		//cleanExistData_only4Dpointers();

		this->setXDim(rsz0);
		this->setYDim(rsz1);
		this->setZDim(rsz2);
		this->setCDim(rsz3);

		setupData4D();

		//update GUI

		curFocusX = this->getXDim() / 2; //-= bpos_x+1; //begin from first slices
		curFocusY = this->getYDim() / 2; //-= bpos_y+1;
		curFocusZ = this->getZDim() / 2; //-= bpos_z+1;

	}

}

bool My4DImage::permute(V3DLONG dimorder[4]) //081001: can also be impelemented using local swapping a pair of dimensions, and do multiple times; The serial pairs can be determined using quick sort algorithm.
{
	//first check the validity of the dimorder
	V3DLONG i, j, k, c;
	int dim_used_cnt[4]; for (i = 0; i < 4; i++) dim_used_cnt[i] = 0; //initialized as 0
	for (i = 0; i < 4; i++)
	{
		switch (dimorder[i])
		{
		case 0: dim_used_cnt[dimorder[i]]++; break;
		case 1: dim_used_cnt[dimorder[i]]++; break;
		case 2: dim_used_cnt[dimorder[i]]++; break;
		case 3: dim_used_cnt[dimorder[i]]++; break;
		default:
			v3d_msg("dimorder contain invalid value. Must be interger between 0 to 3. No nothing.\n");
			return false;
		}
	}
	for (i = 0; i < 4; i++)
	{
		if (dim_used_cnt[i] != 1)
		{
			v3d_msg("dimorder contain invalid value. Every dimension index 0~3 must be used and only used once!. Do nothing.\n");
			return false;
		}
	}
	if (dimorder[0] == 0 && dimorder[1] == 1 && dimorder[2] == 2 && dimorder[3] == 3)
	{
		v3d_msg("The dimorder is in the same order of the original image. Nothing needs to be done. \n");
		return true;
	}

	//then generate a swap memory for data

	V3DLONG tmp_dim[4]; tmp_dim[0] = this->getXDim(); tmp_dim[1] = this->getYDim(); tmp_dim[2] = this->getZDim(); tmp_dim[3] = this->getCDim();
	My4DImage * tmp_img = 0;
	try
	{
		tmp_img = new My4DImage;
		tmp_img->loadImage(tmp_dim[dimorder[0]], tmp_dim[dimorder[1]], tmp_dim[dimorder[2]], tmp_dim[dimorder[3]], this->getDatatype());
		if (!tmp_img->valid())
		{
			v3d_msg("Fail to produce a swap for image permutation. Do nothing.\n");
			return false;
		}
	}
	catch (...)
	{
		v3d_msg("Fail to allocate the swap memory. Do nothing.");
		if (tmp_img) { delete tmp_img; tmp_img = 0; }
		return false;
	}

	float **** tmp_data4d_float32 = 0;
	USHORTINT16 **** tmp_data4d_uint16 = 0;
	unsigned char **** tmp_data4d_uint8 = 0;

	V3DLONG ind_array[4];
	switch (this->getDatatype())
	{
	case V3D_UINT8:
		tmp_data4d_uint8 = (unsigned char ****)tmp_img->getData();
		for (c = 0; c < this->getCDim(); c++)
		{
			ind_array[3] = c;
			for (k = 0; k < this->getZDim(); k++)
			{
				ind_array[2] = k;
				for (j = 0; j < this->getYDim(); j++)
				{
					ind_array[1] = j;
					for (i = 0; i < this->getXDim(); i++)
					{
						ind_array[0] = i;
						tmp_data4d_uint8[ind_array[dimorder[3]]][ind_array[dimorder[2]]][ind_array[dimorder[1]]][ind_array[dimorder[0]]] = data4d_uint8[c][k][j][i];
					}
				}
			}
		}
		break;
	case V3D_UINT16:
		tmp_data4d_uint16 = (USHORTINT16 ****)tmp_img->getData();
		for (c = 0; c < this->getCDim(); c++)
		{
			ind_array[3] = c;
			for (k = 0; k < this->getZDim(); k++)
			{
				ind_array[2] = k;
				for (j = 0; j < this->getYDim(); j++)
				{
					ind_array[1] = j;
					for (i = 0; i < this->getXDim(); i++)
					{
						ind_array[0] = i;
						tmp_data4d_uint16[ind_array[dimorder[3]]][ind_array[dimorder[2]]][ind_array[dimorder[1]]][ind_array[dimorder[0]]] = data4d_uint16[c][k][j][i];
					}
				}
			}
		}
		break;

	case V3D_FLOAT32:
		tmp_data4d_float32 = (float ****)tmp_img->getData();
		for (c = 0; c < this->getCDim(); c++)
		{
			ind_array[3] = c;
			for (k = 0; k < this->getZDim(); k++)
			{
				ind_array[2] = k;
				for (j = 0; j < this->getYDim(); j++)
				{
					ind_array[1] = j;
					for (i = 0; i < this->getXDim(); i++)
					{
						ind_array[0] = i;
						tmp_data4d_float32[ind_array[dimorder[3]]][ind_array[dimorder[2]]][ind_array[dimorder[1]]][ind_array[dimorder[0]]] = data4d_float32[c][k][j][i];
					}
				}
			}
		}
		break;


	default:
		v3d_msg("Should never be here in permute(). Check your program.\n");
		if (tmp_img) { delete tmp_img; tmp_img = 0; }
		return false;
	}

	unsigned char *tmp_1d = tmp_img->getRawData();
	unsigned char *dst_1d = this->getRawData();
	for (i = 0; i < getTotalBytes(); i++)
	{
		dst_1d[i] = tmp_1d[i];
	}

	if (tmp_img) { delete tmp_img; tmp_img = 0; }

	cleanExistData_only4Dpointers();

	this->setXDim(tmp_dim[dimorder[0]]);
	this->setYDim(tmp_dim[dimorder[1]]);
	this->setZDim(tmp_dim[dimorder[2]]);
	this->setCDim(tmp_dim[dimorder[3]]);

	setupData4D();

	//update GUI

	curFocusX = this->getXDim() / 2; //-= bpos_x+1; //begin from first slices
	curFocusY = this->getYDim() / 2; //-= bpos_y+1;
	curFocusZ = this->getZDim() / 2; //-= bpos_z+1;

	
}

double My4DImage::getChannalMinIntensity(V3DLONG channo) //if channo <0 or out of range, then return the in of all channels
{
	if (!p_vmin) return 0;
	if (channo >= 0 && channo < this->getCDim()) return p_vmin[channo];
	else { V3DLONG tmppos; return maxInVector(p_vmin, this->getCDim(), tmppos); }
}

double My4DImage::getChannalMaxIntensity(V3DLONG channo) //if channo <0 or out of range, then return the max of all channels
{
	if (!p_vmax) return 0;
	if (channo >= 0 && channo < this->getCDim()) return p_vmax[channo];
	else { V3DLONG tmppos; return maxInVector(p_vmin, this->getCDim(), tmppos); }
}


void My4DImage::setupData4D()
{
	if (this->getError() == 1 || !this->getRawData())
	{
		v3d_msg("Invalid input data for setting up 4D pointers setupData4D().\n", false);
		return;
	}

	printf("before updateminmaxvalues My4DImage::setupData4D: %d\n", this->getDatatype());

	if (!updateminmaxvalues())
	{
		v3d_msg("Fail to run successfully updateminmaxvalues(). Thus discontinue the setupData4D().\n", false);
		return;
	}
	printf("after updateminmaxvalues My4DImage::setupData4D: %d\n", this->getDatatype());

	switch (this->getDatatype())
	{
	case V3D_UINT8:

		printf("enter V3D_UINT8 My4DImage::setupData4D\n");
		if (!new4dpointer_v3d(data4d_uint8, this->getXDim(), this->getYDim(), this->getZDim(), this->getCDim(), this->getRawData()))
		{
			printf("enter V3D_UINT8, setError My4DImage::setupData4D\n");
			this->setError(1);
			return;
		}
		data4d_virtual = (void ****)data4d_uint8;

		//if (sz3==1) //080829: as tere may be crop operation that keeps only one channel, thus always generate the colormap
		//createColorMap(256); //just use 256 for a safe pool for cell editing. No need to update the colormap again as in the constructor function the default colormap is already set
		break;

	case V3D_UINT16:

		printf("enter V3D_UINT16 My4DImage::setupData4D\n");

		if (!new4dpointer_v3d(data4d_uint16, this->getXDim(), this->getYDim(), this->getZDim(), this->getCDim(), this->getRawData()))
		{
			printf("enter V3D_UINT16 setError My4DImage::setupData4D\n");
			this->setError(1);
			return;
		}
		data4d_virtual = (void ****)data4d_uint16;

		//080824: copied from wano project
		createColorMap(int(p_vmax[0]) + 1000); //add 1000 for a safe pool for cell editing.
		printf("set the color map max=%d\n", int(p_vmax[0]));

		break;

	case V3D_FLOAT32:
		if (!new4dpointer_v3d(data4d_float32, this->getXDim(), this->getYDim(), this->getZDim(), this->getCDim(), this->getRawData()))
		{
			this->setError(1);
			return;
		}
		data4d_virtual = (void ****)data4d_float32;

		//FIXME: this may be a problem and need further test of float type for color indexing
		//createColorMap(256); //just use 256 for a safe pool for cell editing.
		break;

	default:
		this->setError(1);
		v3d_msg("Invalid data type found in setupData4D(). Should never happen, - check with V3D developers.");
		return;
		//break;
	}

	//set up the default color mapping table. 100824, PHC
	setupDefaultColorChannelMapping();

	//
	curFocusX = this->getXDim() >> 1; //begin from mid location
	curFocusY = this->getYDim() >> 1;
	curFocusZ = this->getZDim() >> 1;

}


void My4DImage::updateData4D()
{
	if (this->getError() == 1 || !this->getRawData())
	{
		v3d_error("Invalid input data for setting up 4D pointers setupData4D()");
		return;
	}

	v3d_debug("before updateminmaxvalues My4DImage::updateData4D: %d\n", this->getDatatype());

	if (!updateminmaxvalues())
	{
		v3d_error("Fail to run successfully updateminmaxvalues(). Thus discontinue the setupData4D().\n");
		return;
	}
	v3d_debug("after updateminmaxvalues My4DImage::updateData4D: %d\n", this->getDatatype());

	//set up the default color mapping table. 100824, PHC
	setupDefaultColorChannelMapping();

	//
	curFocusX = this->getXDim() >> 1; //begin from mid location
	curFocusY = this->getYDim() >> 1;
	curFocusZ = this->getZDim() >> 1;

}


void My4DImage::setupDefaultColorChannelMapping() //20100824, PHC
{
	listChannels.clear();
	for (V3DLONG i = 0; i < this->getCDim(); i++)
	{
		DataChannelColor dc;
		dc.n = i;
		dc.on = true;
		V3DLONG tmp = i % 7;
		dc.color.a = 255;
		dc.color.r = dc.color.g = dc.color.b = 0;
		switch (tmp)
		{
		case 0: dc.color.r = 255; break;
		case 1: dc.color.g = 255; break;
		case 2: dc.color.b = 255; break;
		case 3: dc.color.r = 255; dc.color.g = 255; dc.color.b = 255; break;
		case 4: dc.color.r = 255; dc.color.g = 255; break;
		case 5: dc.color.r = 255; dc.color.b = 255; break;
		case 6: dc.color.g = 255; dc.color.b = 255; break;
		default:
			v3d_msg("Your should never see this msg. Check with V3D developer.");
		}

		listChannels.append(dc);
	}
}

bool My4DImage::updateminmaxvalues()
{
	if (this->getError() == 1 || !this->getRawData() || this->getCDim() <= 0 || this->getXDim() <= 0 || this->getYDim() <= 0 || this->getZDim() <= 0)
	{
		v3d_error("My4DImage::updateminmaxvalues() The image data is invalid.\n");
		return false;
	}

	//always delete the two pointers and recreate because if the image is altered in a plugin, the # of color channels may change
	if (p_vmax) { delete[]p_vmax; p_vmax = 0; }
	if (p_vmin) { delete[]p_vmin; p_vmin = 0; }

	try
	{
		p_vmax = new double[this->getCDim()];
		p_vmin = new double[this->getCDim()];
	}
	catch (...)
	{
		v3d_error("Error happened in allocating memory in updateminmaxvalues().\n");
		this->setError(1);
		if (p_vmax) { delete[]p_vmax; p_vmax = 0; }
		if (p_vmin) { delete[]p_vmin; p_vmin = 0; }
		return false;
	}

	V3DLONG i, tmppos;
	V3DLONG channelPageSize = this->getTotalUnitNumberPerChannel();

	switch (this->getDatatype())
	{
	case V3D_UINT8:
		for (i = 0; i < this->getCDim(); i++)
		{
			unsigned char minvv, maxvv;
			V3DLONG tmppos_min, tmppos_max;
			unsigned char *datahead = (unsigned char *)getRawDataAtChannel(i);
			minMaxInVector(datahead, channelPageSize, tmppos_min, minvv, tmppos_max, maxvv);
			p_vmax[i] = maxvv; p_vmin[i] = minvv;
			//v3d_msg(QString("channel %1 min=[%2] max=[%3]").arg(i).arg(p_vmin[i]).arg(p_vmax[i]),0);
			v3d_debug("My4DImage::updateminmaxvalues channel %d min=[%.2f] max=[%.2f]", i, p_vmin[i], p_vmax[i]);
		}
		break;

	case V3D_UINT16:
		for (i = 0; i < this->getCDim(); i++)
		{
			USHORTINT16 minvv, maxvv;
			V3DLONG tmppos_min, tmppos_max;
			USHORTINT16 *datahead = (USHORTINT16 *)getRawDataAtChannel(i);
			minMaxInVector(datahead, channelPageSize, tmppos_min, minvv, tmppos_max, maxvv);
			p_vmax[i] = maxvv; p_vmin[i] = minvv;
			//v3d_msg(QString("channel %1 min=[%2] max=[%3]").arg(i).arg(p_vmin[i]).arg(p_vmax[i]),0);
			v3d_debug("My4DImage::updateminmaxvalues channel %d min=[%.2f] max=[%.2f]", i, p_vmin[i], p_vmax[i]);
		}
		break;

	case V3D_FLOAT32:
		for (i = 0; i < this->getCDim(); i++)
		{
			float minvv, maxvv;

			V3DLONG tmppos_min, tmppos_max;
			float *datahead = (float *)getRawDataAtChannel(i);

			if (0) //for debugging purpose. 2014-08-22
			{
				minvv = datahead[0], maxvv = datahead[0];
				for (V3DLONG myii = 1; myii < channelPageSize; myii++)
				{
					if (minvv > datahead[myii]) minvv = datahead[myii];
					else if (maxvv < datahead[myii]) maxvv = datahead[myii];
				}

				p_vmax[i] = maxvv; p_vmin[i] = minvv;
				//v3d_msg(QString("channel %1 min=[%2] max=[%3]").arg(i).arg(p_vmin[i]).arg(p_vmax[i]));
				v3d_debug("My4DImage::updateminmaxvalues channel %d min=[%.2f] max=[%.2f]", i, p_vmin[i], p_vmax[i]);
			}
			else
			{
				if (minMaxInVector(datahead, channelPageSize, tmppos_min, minvv, tmppos_max, maxvv))
				{
					p_vmax[i] = maxvv; p_vmin[i] = minvv;
					//v3d_msg(QString("channel %1 min=[%2] max=[%3]").arg(i).arg(p_vmin[i]).arg(p_vmax[i]), 0);
					v3d_debug("My4DImage::updateminmaxvalues channel %d min=[%.2f] max=[%.2f]", i, p_vmin[i], p_vmax[i]);
				}
				else
				{
					//v3d_msg("fail");
					v3d_error("My4DImage::updateminmaxvalues fail");
				}
			}

		}
		break;

	default:
		this->setError(1);
		v3d_error("Invalid data type found in updateminmaxvalues(). Should never happen, - check with V3D developers.");
		return false;
	}

	return true;
}

void My4DImage::loadImage(V3DLONG imgsz0, V3DLONG imgsz1, V3DLONG imgsz2, V3DLONG imgsz3, int imgdatatype) //an overloaded function to create a blank image
{
	Image4DSimple::createBlankImage(imgsz0, imgsz1, imgsz2, imgsz3, imgdatatype);
	if (this->getError() == 1 || !this->getRawData())
	{
		v3d_error("Error happened in creating 1d data.\n");
		return;
	}

	try {
		p_vmax = new double[this->getCDim()];
		p_vmin = new double[this->getCDim()];
	}
	catch (...)
	{
		v3d_error("Error happened in allocating memory.\n");
		this->setError(1);
		if (p_vmax) { delete[]p_vmax; p_vmax = NULL; }
		if (p_vmin) { delete[]p_vmin; p_vmin = NULL; }
		return;
	}

	V3DLONG i, tmppos;
	V3DLONG channelPageSize = V3DLONG(this->getXDim())*this->getYDim()*this->getZDim();

	switch (this->getDatatype())
	{
	case V3D_UINT8:
		if (!new4dpointer(data4d_uint8, this->getXDim(), this->getYDim(), this->getZDim(), this->getCDim(), this->getRawData()))
		{
			this->setError(1);
			return;
		}
		data4d_virtual = (void ****)data4d_uint8;

		for (i = 0; i < this->getCDim(); i++)
		{
			p_vmax[i] = 0; //no need to compute as it is blank
			p_vmin[i] = 0;
		}

		//if (sz3==1)
		//createColorMap(256); //just use 256 for a safe pool for cell editing. 060501

		break;

	case V3D_UINT16:
		if (!new4dpointer(data4d_uint16, this->getXDim(), this->getYDim(), this->getZDim(), this->getCDim(), (USHORTINT16 *)this->getRawData()))
		{
			v3d_error("Warning: this data type UINT16 has not been supported in display yet.\n");
			this->setError(1);
			return;
		}
		data4d_virtual = (void ****)data4d_uint16;

		for (i = 0; i < this->getCDim(); i++)
		{
			p_vmax[i] = 0;
			p_vmin[i] = 0;
		}

		createColorMap(int(p_vmax[0]) + 1000); //add 1000 for a safe pool for cell editing. 060501
		v3d_info("My4DImage::loadImage set the color map max=%d\n", int(p_vmax[0]));

		//printf("Warning: this data type UINT16 has not been supported in display yet.\n");

		break;

	case V3D_FLOAT32:
		if (!new4dpointer(data4d_float32, this->getXDim(), this->getYDim(), this->getZDim(), this->getCDim(), (float *)this->getRawData()))
		{
			this->setError(1);
			return;
		}
		data4d_virtual = (void ****)data4d_float32;

		for (i = 0; i < this->getCDim(); i++)
		{
			p_vmax[i] = 0;
			p_vmin[i] = 0;
		}

		v3d_msg("Warning: this data type FLOAT32 has not been supported in display yet - create blank image.\n");

		break;

	default:
		this->setError(1);
		return;
		//break;
	}

	curFocusX = this->getXDim() >> 1; //change to middle slide 090718. //begin from first slices. Original is 1, should be wrong. corrected to 0 on 060426
	curFocusY = this->getYDim() >> 1;
	curFocusZ = this->getZDim() >> 1;

}



bool My4DImage::saveMovie()
{
	//	// begin the save process
	//
	//	QString outputFile = QFileDialog::getSaveFileName(0,
	//													  "Choose a filename to save under",
	//													  //"./",
	//													  QString(this->getFileName())+".cp.tif",
	//													  "Save file format (*.raw *.tif)");
	//
	//	while (outputFile.isEmpty()) //note that I used isEmpty() instead of isNull, although seems the Cancel operation will return a null string. phc 060422
	//	{
	//    	if(QMessageBox::Yes == QMessageBox::question (0, "", "Are you sure you do NOT want to save?", QMessageBox::Yes, QMessageBox::No))
	//	    {
	//		    return false;
	//		}
	//		outputFile = QFileDialog::getSaveFileName(0,
	//												  "Choose a filename to save under",
	//												  "./",
	//												  "Save file format (*.raw *.tif)");
	//	}
	//
	//	saveImage(qPrintable(outputFile));
	//
	//	printf("Current image is saved to the file %s\n", qPrintable(outputFile));
	//
	return true;
}


bool My4DImage::saveFile()
{
	// begin the save process

	QString outputFile = QFileDialog::getSaveFileName(0,
		"Choose a filename to save under",
		//"./",
		QString(this->getFileName()) + ".cp.tif",
		"Save file format (*.raw *.tif)");

	while (outputFile.isEmpty()) //note that I used isEmpty() instead of isNull, although seems the Cancel operation will return a null string. phc 060422
	{
		if (QMessageBox::Yes == QMessageBox::question(0, "", "Are you sure you do NOT want to save?", QMessageBox::Yes, QMessageBox::No))
		{
			return false;
		}
		outputFile = QFileDialog::getSaveFileName(0,
			"Choose a filename to save under",
			"./",
			"Save file format (*.raw *.tif)");
	}

	saveImage(qPrintable(outputFile));

	printf("Current image is saved to the file %s\n", qPrintable(outputFile));

	return true;
}

bool My4DImage::saveFile(char filename[]) {
	QString outputFile(filename);
	return saveFile(outputFile);
}

bool My4DImage::saveFile(QString outputFile)
{
	while (outputFile.isEmpty())
	{
		if (QMessageBox::Yes == QMessageBox::question(0, "", "Are you sure you do NOT want to save?", QMessageBox::Yes, QMessageBox::No))
		{
			return false;
		}
		outputFile = QFileDialog::getSaveFileName(0,
			"Choose a filename to save under",
			"./",
			"Save file format (*.raw *.tif)");
	}

	saveImage(qPrintable(outputFile));

	printf("Current image is saved to the file %s\n", qPrintable(outputFile));
	//if (p_mainWidget) {p_mainWidget->setCurrentFileName(filename);}

	return true;
}

void My4DImage::crop(int landmark_crop_opt)
{
	//get the bounding boxes

	//if (!p_xy_view)
	//	return;

	//QRect b_xy = p_xy_view->getRoiBoundingRect();

	//V3DLONG bpos_x = qBound(V3DLONG(0), V3DLONG(qMax(b_xy.left(), 0)), this->getXDim() - 1),
	//	bpos_c = 0;
	//V3DLONG epos_x = qBound(V3DLONG(0), V3DLONG(qMin(b_xy.right(), 0)), this->getXDim() - 1),
	//	epos_c = this->getCDim() - 1;

	//if (bpos_x > epos_x)
	//{
	//	v3d_msg("The roi polygons in three views are not intersecting! No crop is done!\n");
	//	return;
	//}

	//// create new data, copy over, and delete the original data

	//crop(bpos_x, epos_x, 0, 0, 0, 0, bpos_c, epos_c, landmark_crop_opt);

	return;
}

void My4DImage::crop(V3DLONG bpos_x, V3DLONG epos_x, V3DLONG bpos_y, V3DLONG epos_y, V3DLONG bpos_z, V3DLONG epos_z, V3DLONG bpos_c, V3DLONG epos_c, int landmark_crop_opt)
{
	//get the bounding boxes

	//if (!p_xy_view)
	//	return;

	//bpos_x = qBound(V3DLONG(0), bpos_x, this->getXDim() - 1);
	//bpos_y = qBound(V3DLONG(0), bpos_y, this->getYDim() - 1);
	//bpos_z = qBound(V3DLONG(0), bpos_z, this->getZDim() - 1);
	//bpos_c = qBound(V3DLONG(0), bpos_c, this->getCDim() - 1);

	//epos_x = qBound(V3DLONG(0), epos_x, this->getXDim() - 1);
	//epos_y = qBound(V3DLONG(0), epos_y, this->getYDim() - 1);
	//epos_z = qBound(V3DLONG(0), epos_z, this->getZDim() - 1);
	//epos_c = qBound(V3DLONG(0), epos_c, this->getCDim() - 1);

	//if (bpos_x > epos_x || bpos_y > epos_y || bpos_z > epos_z)
	//{
	//	v3d_msg("The parameters of crop() are invalid! No crop is done!\n");
	//	return;
	//}

	//// create new data, copy over, and delete the original data

	//V3DLONG nsz0 = epos_x - bpos_x + 1,
	//	nsz1 = epos_y - bpos_y + 1,
	//	nsz2 = epos_z - bpos_z + 1,
	//	nsz3 = epos_c - bpos_c + 1;

	//float **** ndata4d_float32 = 0;
	//USHORTINT16 **** ndata4d_uint16 = 0;
	//unsigned char **** ndata4d_uint8 = 0;

	//unsigned char * ndata1d = 0;
	//ImagePixelType ndatatype = this->getDatatype();

	//V3DLONG i, j, k, c, i0, j0, k0, c0;

	//try
	//{
	//	switch (this->getDatatype())
	//	{
	//	case V3D_UINT8:
	//		ndata1d = new unsigned char[nsz0 * nsz1 * nsz2 * nsz3];
	//		if (!ndata1d)
	//		{
	//			v3d_msg("Cannot allocate memory for the cropped image. You may want to free memory by closing unnecessary programs.\n");
	//			return;
	//		}

	//		if (!new4dpointer_v3d(ndata4d_uint8, nsz0, nsz1, nsz2, nsz3, ndata1d))
	//		{
	//			this->setError(1);
	//			if (ndata1d) { delete ndata1d; ndata1d = 0; }
	//			return;
	//		}
	//		for (c = bpos_c, c0 = 0; c <= epos_c; c++, c0++)
	//		{
	//			for (k = bpos_z, k0 = 0; k <= epos_z; k++, k0++)
	//			{
	//				for (j = bpos_y, j0 = 0; j <= epos_y; j++, j0++)
	//				{
	//					for (i = bpos_x, i0 = 0; i <= epos_x; i++, i0++)
	//					{
	//						ndata4d_uint8[c0][k0][j0][i0] = data4d_uint8[c][k][j][i];
	//					}
	//				}
	//			}
	//		}

	//		break;

	//	case V3D_UINT16:
	//		ndata1d = new unsigned char[nsz0 * nsz1 * nsz2 * nsz3 * sizeof(unsigned short int)];
	//		if (!ndata1d)
	//		{
	//			v3d_msg("Cannot allocate memory for the cropped image. You may want to free memory by closing unnecessary programs.\n");
	//			return;
	//		}

	//		if (!new4dpointer_v3d(ndata4d_uint16, nsz0, nsz1, nsz2, nsz3, ndata1d))
	//		{
	//			this->setError(1);
	//			if (ndata1d) { delete ndata1d; ndata1d = 0; }
	//			return;
	//		}
	//		for (c = bpos_c, c0 = 0; c <= epos_c; c++, c0++)
	//		{
	//			for (k = bpos_z, k0 = 0; k <= epos_z; k++, k0++)
	//			{
	//				for (j = bpos_y, j0 = 0; j <= epos_y; j++, j0++)
	//				{
	//					for (i = bpos_x, i0 = 0; i <= epos_x; i++, i0++)
	//					{
	//						ndata4d_uint16[c0][k0][j0][i0] = data4d_uint16[c][k][j][i];
	//					}
	//				}
	//			}
	//		}

	//		break;

	//	case V3D_FLOAT32:
	//		ndata1d = new unsigned char[nsz0 * nsz1 * nsz2 * nsz3 * sizeof(float)];
	//		if (!ndata1d)
	//		{
	//			v3d_msg("Cannot allocate memory for the cropped image. You may want to free memory by closing unnecessary programs.\n");
	//			return;
	//		}

	//		if (!new4dpointer_v3d(ndata4d_float32, nsz0, nsz1, nsz2, nsz3, ndata1d))
	//		{
	//			this->setError(1);
	//			if (ndata1d) { delete ndata1d; ndata1d = 0; }
	//			return;
	//		}
	//		for (c = bpos_c, c0 = 0; c <= epos_c; c++, c0++)
	//		{
	//			for (k = bpos_z, k0 = 0; k <= epos_z; k++, k0++)
	//			{
	//				for (j = bpos_y, j0 = 0; j <= epos_y; j++, j0++)
	//				{
	//					for (i = bpos_x, i0 = 0; i <= epos_x; i++, i0++)
	//					{
	//						ndata4d_float32[c0][k0][j0][i0] = data4d_float32[c][k][j][i];
	//					}
	//				}
	//			}
	//		}

	//		break;

	//	default:
	//		this->setError(1);
	//		if (ndata1d) { delete ndata1d; ndata1d = 0; }
	//		return;
	//		//break;
	//	}
	//}
	//catch (...)
	//{
	//	v3d_msg("Error happens in crop();\n");
	//	return;
	//}

	//cleanExistData_butKeepFileName();

	//this->setRawDataPointer(ndata1d);

	//data4d_float32 = ndata4d_float32;
	//data4d_uint16 = ndata4d_uint16;
	//data4d_uint8 = ndata4d_uint8;

	//this->setDatatype(ndatatype);

	//this->setXDim(nsz0);
	//this->setYDim(nsz1);
	//this->setZDim(nsz2);
	//this->setCDim(nsz3);

	//try
	//{
	//	p_vmax = new double[this->getCDim()];
	//	p_vmin = new double[this->getCDim()];
	//}
	//catch (...)
	//{
	//	v3d_msg("Fail to allocate memory.");
	//	this->setError(1);
	//	if (p_vmax) { delete[]p_vmax; p_vmax = NULL; }
	//	if (p_vmin) { delete[]p_vmin; p_vmin = NULL; }
	//	return;
	//}

	//switch (this->getDatatype())
	//{
	//case V3D_UINT8:
	//	data4d_virtual = (void ****)data4d_uint8;
	//	break;

	//case V3D_UINT16:
	//	data4d_virtual = (void ****)data4d_uint16;
	//	break;

	//case V3D_FLOAT32:
	//	data4d_virtual = (void ****)data4d_float32;
	//	break;

	//default:
	//	this->setError(1);
	//	return;
	//	//break;
	//}

	////update minmax

	//if (!updateminmaxvalues())
	//{
	//	v3d_msg("Fail to run successfully updateminmaxvalues() in proj_general_resampling()..\n", false);
	//	return;
	//}

	////
	//curFocusX = this->getXDim() / 2; //-= bpos_x+1; //begin from first slices
	//curFocusY = this->getYDim() / 2; //-= bpos_y+1;
	//curFocusZ = this->getZDim() / 2; //-= bpos_z+1;

	////update the color display mode, as the number of channels could change
	//if (p_mainWidget->getColorType() != colorPseudoMaskColor && p_mainWidget->getColorType() != colorHanchuanFlyBrainColor && p_mainWidget->getColorType() != colorArnimFlyBrainColor) //otherwise does not need to change
	//{
	//	if (getCDim() >= 3)
	//		p_mainWidget->setColorType(colorRGB);
	//	else if (getCDim() == 2)
	//		p_mainWidget->setColorType(colorRG);
	//	else //==1
	//		p_mainWidget->setColorType(colorRed2Gray);
	//}

	////080828: update the landmarks
	//if (landmark_crop_opt == 1 || landmark_crop_opt == 2)
	//{
	//	LocationSimple tmp_pt(-1, -1, -1);
	//	for (V3DLONG i = listLandmarks.count() - 1; i >= 0; i--)
	//	{
	//		tmp_pt = listLandmarks.at(i);
	//		tmp_pt.x -= bpos_x;
	//		tmp_pt.y -= bpos_y;
	//		tmp_pt.z -= bpos_z;
	//		if (landmark_crop_opt == 2) //in this case subtrast the min, but keep all landmarks
	//		{
	//			listLandmarks.replace(i, tmp_pt);
	//		}
	//		else //in this case, i.e. ==1, subtrast min and remove all that outside the bbox
	//		{
	//			if (tmp_pt.x < 0 || tmp_pt.x >= (epos_x - bpos_x + 1) ||
	//				tmp_pt.y < 0 || tmp_pt.y >= (epos_y - bpos_y + 1) ||
	//				tmp_pt.z < 0 || tmp_pt.z >= (epos_z - bpos_z + 1))
	//				listLandmarks.removeAt(i);
	//			else
	//				listLandmarks.replace(i, tmp_pt);
	//		}
	//	}
	//	listLocationRelationship.clear();
	//}

	////update view

	//p_xy_view->deleteROI();

	//p_mainWidget->updateDataRelatedGUI();
	//p_mainWidget->setWindowTitle_Suffix("_crop");

	return;
}



bool My4DImage::rotate(ImagePlaneDisplayType ptype, const Options_Rotate & r_opt)
{
	//if (!data4d_uint8 && !data4d_uint16 && !data4d_float32)
	//{
	//	v3d_msg("None of the 4d data pointers is valid in rotate().");  return false;
	//}

	//V3DLONG insz[4]; insz[0] = this->getXDim(); insz[1] = this->getYDim(); insz[2] = this->getZDim(); insz[3] = this->getCDim();
	//unsigned char * outvol1d = 0;
	//V3DLONG *outsz = 0;
	//bool b_res = false;

	//switch (ptype)
	//{
	//case imgPlaneX:
	//	if (data4d_uint8)
	//		b_res = rotate_inPlaneX(this->getRawData(), insz, r_opt, outvol1d, outsz);
	//	else if (data4d_uint16)
	//	{
	//		unsigned short int * tmpout = 0;
	//		b_res = rotate_inPlaneX((unsigned short int *)(this->getRawData()), insz, r_opt, tmpout, outsz);
	//		outvol1d = (unsigned char *)tmpout;
	//	}
	//	else if (data4d_float32)
	//	{
	//		float * tmpout = 0;
	//		b_res = rotate_inPlaneX((float *)(this->getRawData()), insz, r_opt, tmpout, outsz);
	//		outvol1d = (unsigned char *)tmpout;
	//	}
	//	break;

	//case imgPlaneY:
	//	if (data4d_uint8)
	//		b_res = rotate_inPlaneY(this->getRawData(), insz, r_opt, outvol1d, outsz);
	//	else if (data4d_uint16)
	//	{
	//		unsigned short int * tmpout = 0;
	//		b_res = rotate_inPlaneY((unsigned short int *)(this->getRawData()), insz, r_opt, tmpout, outsz);
	//		outvol1d = (unsigned char *)tmpout;
	//	}
	//	else if (data4d_float32)
	//	{
	//		float * tmpout = 0;
	//		b_res = rotate_inPlaneY((float *)(this->getRawData()), insz, r_opt, tmpout, outsz);
	//		outvol1d = (unsigned char *)tmpout;
	//	}
	//	break;

	//case imgPlaneZ:
	//	if (data4d_uint8)
	//		b_res = rotate_inPlaneZ(this->getRawData(), insz, r_opt, outvol1d, outsz);
	//	else if (data4d_uint16)
	//	{
	//		unsigned short int * tmpout = 0;
	//		b_res = rotate_inPlaneZ((unsigned short int *)(this->getRawData()), insz, r_opt, tmpout, outsz);
	//		outvol1d = (unsigned char *)tmpout;
	//	}
	//	else if (data4d_float32)
	//	{
	//		float * tmpout = 0;
	//		b_res = rotate_inPlaneZ((float *)(this->getRawData()), insz, r_opt, tmpout, outsz);
	//		outvol1d = (unsigned char *)tmpout;
	//	}
	//	break;

	//default:
	//	return false;
	//}

	////assign the rotated image to the current image and update the pointers
	//if (b_res)
	//{
	//	setNewImageData(outvol1d, outsz[0], outsz[1], outsz[2], outsz[3], this->getDatatype());
	//}
	//else
	//{
	//	v3d_msg("The rotate operation fails.\n");
	//	return false;
	//}
	//if (outsz) { delete[]outsz; outsz = 0; }

	////update view
	//updateViews();

	return true;
}


bool My4DImage::flip(AxisCode my_axiscode)
{
	if (!valid()) { return false; }
	V3DLONG i, j, k, c;

	switch (this->getDatatype())
	{
	case V3D_UINT8:
		switch (my_axiscode)
		{
		case axis_x:
		{
			V3DLONG hsz0 = floor((double)(this->getXDim() - 1) / 2.0); if (hsz0 * 2 < this->getXDim() - 1) hsz0 += 1;
			for (c = 0; c < this->getCDim(); c++)
				for (k = 0; k < this->getZDim(); k++)
					for (j = 0; j < this->getYDim(); j++)
						for (i = 0; i < hsz0; i++)
						{
							unsigned char tmpv = data4d_uint8[c][k][j][this->getXDim() - i - 1];
							data4d_uint8[c][k][j][this->getXDim() - 1 - i] = data4d_uint8[c][k][j][i];
							data4d_uint8[c][k][j][i] = tmpv;
						}
		}
		break;
		case axis_y:
		{
			V3DLONG hsz1 = floor((double)(this->getYDim() - 1) / 2.0); if (hsz1 * 2 < this->getYDim() - 1) hsz1 += 1;
			qDebug("%d %d", this->getYDim(), hsz1);
			for (c = 0; c < this->getCDim(); c++)
				for (k = 0; k < this->getZDim(); k++)
					for (j = 0; j < hsz1; j++)
						for (i = 0; i < this->getXDim(); i++)
						{
							unsigned char tmpv = data4d_uint8[c][k][this->getYDim() - j - 1][i];
							data4d_uint8[c][k][this->getYDim() - 1 - j][i] = data4d_uint8[c][k][j][i];
							data4d_uint8[c][k][j][i] = tmpv;
						}
		}
		break;
		case axis_z:
		{
			V3DLONG hsz2 = floor((double)(this->getZDim() - 1) / 2.0); if (hsz2 * 2 < this->getZDim() - 1) hsz2 += 1;
			for (c = 0; c < this->getCDim(); c++)
				for (k = 0; k < hsz2; k++)
					for (j = 0; j < this->getYDim(); j++)
						for (i = 0; i < this->getXDim(); i++)
						{
							unsigned char tmpv = data4d_uint8[c][this->getZDim() - k - 1][j][i];
							data4d_uint8[c][this->getZDim() - 1 - k][j][i] = data4d_uint8[c][k][j][i];
							data4d_uint8[c][k][j][i] = tmpv;
						}
		}
		break;
		case axis_c:
		{
			V3DLONG hsz3 = floor((double)(this->getCDim() - 1) / 2.0); if (hsz3 * 2 < this->getCDim() - 1) hsz3 += 1;
			for (c = 0; c < hsz3; c++)
			{
				for (k = 0; k < this->getZDim(); k++)
					for (j = 0; j < this->getYDim(); j++)
						for (i = 0; i < this->getXDim(); i++)
						{
							unsigned char tmpv = data4d_uint8[this->getCDim() - c - 1][k][j][i];
							data4d_uint8[this->getCDim() - c - 1][k][j][i] = data4d_uint8[c][k][j][i];
							data4d_uint8[c][k][j][i] = tmpv;
						}

				double tmpc;
				tmpc = p_vmax[this->getCDim() - c - 1]; p_vmax[this->getCDim() - c - 1] = p_vmax[c]; p_vmax[c] = tmpc;
				tmpc = p_vmin[this->getCDim() - c - 1]; p_vmin[this->getCDim() - c - 1] = p_vmin[c]; p_vmin[c] = tmpc;
			}
		}
		break;
		default:
			break;
		}
		break;

	case V3D_UINT16:
		switch (my_axiscode)
		{
		case axis_x:
		{
			V3DLONG hsz0 = floor((double)(this->getXDim() - 1) / 2.0); if (hsz0 * 2 < this->getXDim() - 1) hsz0 += 1;
			for (c = 0; c < this->getCDim(); c++)
				for (k = 0; k < this->getZDim(); k++)
					for (j = 0; j < this->getYDim(); j++)
						for (i = 0; i < hsz0; i++)
						{
							unsigned short int tmpv = data4d_uint16[c][k][j][this->getXDim() - i - 1];
							data4d_uint16[c][k][j][this->getXDim() - 1 - i] = data4d_uint16[c][k][j][i];
							data4d_uint16[c][k][j][i] = tmpv;
						}
		}
		break;
		case axis_y:
		{
			V3DLONG hsz1 = floor((double)(this->getYDim() - 1) / 2.0); if (hsz1 * 2 < this->getYDim() - 1) hsz1 += 1;
			for (c = 0; c < this->getCDim(); c++)
				for (k = 0; k < this->getZDim(); k++)
					for (j = 0; j < hsz1; j++)
						for (i = 0; i < this->getXDim(); i++)
						{
							unsigned short int tmpv = data4d_uint16[c][k][this->getYDim() - j - 1][i];
							data4d_uint16[c][k][this->getYDim() - 1 - j][i] = data4d_uint16[c][k][j][i];
							data4d_uint16[c][k][j][i] = tmpv;
						}
		}
		break;
		case axis_z:
		{
			V3DLONG hsz2 = floor((double)(this->getZDim() - 1) / 2.0); if (hsz2 * 2 < this->getZDim() - 1) hsz2 += 1;
			for (c = 0; c < this->getCDim(); c++)
				for (k = 0; k < hsz2; k++)
					for (j = 0; j < this->getYDim(); j++)
						for (i = 0; i < this->getXDim(); i++)
						{
							unsigned short int tmpv = data4d_uint16[c][this->getZDim() - k - 1][j][i];
							data4d_uint16[c][this->getZDim() - 1 - k][j][i] = data4d_uint16[c][k][j][i];
							data4d_uint16[c][k][j][i] = tmpv;
						}
		}
		break;
		case axis_c:
		{
			V3DLONG hsz3 = floor((double)(this->getCDim() - 1) / 2.0); if (hsz3 * 2 < this->getCDim() - 1) hsz3 += 1;
			for (c = 0; c < hsz3; c++)
			{
				for (k = 0; k < this->getZDim(); k++)
					for (j = 0; j < this->getYDim(); j++)
						for (i = 0; i < this->getXDim(); i++)
						{
							unsigned short int tmpv = data4d_uint16[this->getCDim() - c - 1][k][j][i];
							data4d_uint16[this->getCDim() - c - 1][k][j][i] = data4d_uint16[c][k][j][i];
							data4d_uint16[c][k][j][i] = tmpv;
						}

				double tmpc;
				tmpc = p_vmax[this->getCDim() - c - 1]; p_vmax[this->getCDim() - c - 1] = p_vmax[c]; p_vmax[c] = tmpc;
				tmpc = p_vmin[this->getCDim() - c - 1]; p_vmin[this->getCDim() - c - 1] = p_vmin[c]; p_vmin[c] = tmpc;
			}
		}
		break;
		default:
			break;
		}
		break;


	case V3D_FLOAT32:
		switch (my_axiscode)
		{
		case axis_x:
		{
			V3DLONG hsz0 = floor((double)(this->getXDim() - 1) / 2.0); if (hsz0 * 2 < this->getXDim() - 1) hsz0 += 1;
			for (c = 0; c < this->getCDim(); c++)
				for (k = 0; k < this->getZDim(); k++)
					for (j = 0; j < this->getYDim(); j++)
						for (i = 0; i < hsz0; i++)
						{
							float tmpv = data4d_float32[c][k][j][this->getXDim() - i - 1];
							data4d_float32[c][k][j][this->getXDim() - 1 - i] = data4d_float32[c][k][j][i];
							data4d_float32[c][k][j][i] = tmpv;
						}
		}
		break;
		case axis_y:
		{
			V3DLONG hsz1 = floor((double)(this->getYDim() - 1) / 2.0); if (hsz1 * 2 < this->getYDim() - 1) hsz1 += 1;
			for (c = 0; c < this->getCDim(); c++)
				for (k = 0; k < this->getZDim(); k++)
					for (j = 0; j < hsz1; j++)
						for (i = 0; i < this->getXDim(); i++)
						{
							float tmpv = data4d_float32[c][k][this->getYDim() - j - 1][i];
							data4d_float32[c][k][this->getYDim() - 1 - j][i] = data4d_float32[c][k][j][i];
							data4d_float32[c][k][j][i] = tmpv;
						}
		}
		break;
		case axis_z:
		{
			V3DLONG hsz2 = floor((double)(this->getZDim() - 1) / 2.0); if (hsz2 * 2 < this->getZDim() - 1) hsz2 += 1;
			for (c = 0; c < this->getCDim(); c++)
				for (k = 0; k < hsz2; k++)
					for (j = 0; j < this->getYDim(); j++)
						for (i = 0; i < this->getXDim(); i++)
						{
							float tmpv = data4d_float32[c][this->getZDim() - k - 1][j][i];
							data4d_float32[c][this->getZDim() - 1 - k][j][i] = data4d_float32[c][k][j][i];
							data4d_float32[c][k][j][i] = tmpv;
						}
		}
		break;
		case axis_c:
		{
			V3DLONG hsz3 = floor((double)(this->getCDim() - 1) / 2.0); if (hsz3 * 2 < this->getCDim() - 1) hsz3 += 1;
			for (c = 0; c < hsz3; c++)
			{
				for (k = 0; k < this->getZDim(); k++)
					for (j = 0; j < this->getYDim(); j++)
						for (i = 0; i < this->getXDim(); i++)
						{
							float tmpv = data4d_float32[this->getCDim() - c - 1][k][j][i];
							data4d_float32[this->getCDim() - c - 1][k][j][i] = data4d_float32[c][k][j][i];
							data4d_float32[c][k][j][i] = tmpv;
						}

				double tmpc;
				tmpc = p_vmax[this->getCDim() - c - 1]; p_vmax[this->getCDim() - c - 1] = p_vmax[c]; p_vmax[c] = tmpc;
				tmpc = p_vmin[this->getCDim() - c - 1]; p_vmin[this->getCDim() - c - 1] = p_vmin[c]; p_vmin[c] = tmpc;
			}
		}
		break;
		default:
			break;
		}
		break;

	default:
		this->setError(1);
		return false;
		//break;
	}

	//update view
	updateViews();
	return true;
}

bool My4DImage::invertcolor(int channo) //channo < 0 will invert all channels. Only works for uint8
{
	if (this->getDatatype() != V3D_UINT8)
	{
		v3d_msg("Now the color inversion program only supports 8bit data. Check your data first.\n");
		return false;
	}
	if (channo >= getCDim())
	{
		v3d_msg("Invalid chan parameter in invertcolor();\n");
		return false;
	}

	if (channo >= 0)
	{
		V3DLONG chanbytes = getTotalUnitNumberPerChannel();
		unsigned char *p_end = getRawData() + (channo + 1)*chanbytes, *p = 0;
		for (p = getRawData() + channo*chanbytes; p < p_end; p++) { *p = 255 - *p; }
	}
	else
	{
		V3DLONG chanbytes = getTotalUnitNumber();
		unsigned char *p_end = getRawData() + chanbytes, *p = 0;
		for (p = getRawData(); p < p_end; p++) { *p = 255 - *p; }
	}

	updateViews();
	return true;
}

bool My4DImage::scaleintensity(int channo, double lower_th, double higher_th, double target_min, double target_max) //map the value linear from [lower_th, higher_th] to [target_min, target_max].
{
	if (channo >= getCDim())
	{
		v3d_msg("Invalid chan parameter in scaleintensity();\n");
		return false;
	}

	double t;
	if (lower_th > higher_th) { t = lower_th; lower_th = higher_th; higher_th = t; }
	if (target_min > target_max) { t = target_min; target_min = target_max; target_max = t; }

	double rate = (higher_th == lower_th) ? 1 : (target_max - target_min) / (higher_th - lower_th); //if the two th vals equal, then later-on t-lower_th will be 0 anyway

	V3DLONG i, j, k, c;

	V3DLONG channelPageSize = getTotalUnitNumberPerChannel();
	switch (this->getDatatype())
	{
	case V3D_UINT8:
		for (c = 0; c < this->getCDim(); c++)
		{
			if (channo >= 0 && c != channo)
				continue;
			for (k = 0; k < this->getZDim(); k++)
				for (j = 0; j < this->getYDim(); j++)
					for (i = 0; i < this->getXDim(); i++)
					{
						t = data4d_uint8[c][k][j][i];
						if (t > higher_th) t = higher_th;
						else if (t < lower_th) t = lower_th;
						data4d_uint8[c][k][j][i] = (unsigned char)((t - lower_th)*rate + target_min);
					}
		}

		break;

	case V3D_UINT16:
		for (c = 0; c < this->getCDim(); c++)
		{
			if (channo >= 0 && c != channo)
				continue;
			for (k = 0; k < this->getZDim(); k++)
				for (j = 0; j < this->getYDim(); j++)
					for (i = 0; i < this->getXDim(); i++)
					{
						t = data4d_uint16[c][k][j][i];
						if (t > higher_th) t = higher_th;
						else if (t < lower_th) t = lower_th;
						data4d_uint16[c][k][j][i] = (USHORTINT16)((t - lower_th)*rate + target_min);
					}
		}
		break;

	case V3D_FLOAT32:
		for (c = 0; c < this->getCDim(); c++)
		{
			if (channo >= 0 && c != channo)
				continue;
			for (k = 0; k < this->getZDim(); k++)
				for (j = 0; j < this->getYDim(); j++)
					for (i = 0; i < this->getXDim(); i++)
					{
						t = data4d_float32[c][k][j][i];
						if (t > higher_th) t = higher_th;
						else if (t < lower_th) t = lower_th;
						data4d_float32[c][k][j][i] = (t - lower_th)*rate + target_min;
					}
		}
		break;
	default:
		v3d_msg("invalid datatype in scaleintensity();\n");
		return false;
	}

	//update min and max
	if (!updateminmaxvalues())
	{
		v3d_msg("Fail to run successfully updateminmaxvalues() in scaleintensity()..\n", false);
		return false;
	}


	updateViews();
	return true;
}


bool My4DImage::thresholdintensity(int channo, double th) //anything < th will be 0, others unchanged
{
	if (channo >= getCDim())
	{
		v3d_msg("Invalid chan parameter in thresholdintensity();\n");
		return false;
	}

	V3DLONG i, j, k, c;

	V3DLONG channelPageSize = getTotalUnitNumberPerChannel();
	switch (this->getDatatype())
	{
	case V3D_UINT8:
		for (c = 0; c < this->getCDim(); c++)
		{
			if (channo >= 0 && c != channo)
				continue;
			for (k = 0; k < this->getZDim(); k++)
				for (j = 0; j < this->getYDim(); j++)
					for (i = 0; i < this->getXDim(); i++)
					{
						if (data4d_uint8[c][k][j][i] < th) data4d_uint8[c][k][j][i] = 0;
					}
		}

		break;

	case V3D_UINT16:
		for (c = 0; c < this->getCDim(); c++)
		{
			if (channo >= 0 && c != channo)
				continue;
			for (k = 0; k < this->getZDim(); k++)
				for (j = 0; j < this->getYDim(); j++)
					for (i = 0; i < this->getXDim(); i++)
					{
						if (data4d_uint16[c][k][j][i] < th) data4d_uint16[c][k][j][i] = 0;
					}
		}
		break;

	case V3D_FLOAT32:
		for (c = 0; c < this->getCDim(); c++)
		{
			if (channo >= 0 && c != channo)
				continue;
			for (k = 0; k < this->getZDim(); k++)
				for (j = 0; j < this->getYDim(); j++)
					for (i = 0; i < this->getXDim(); i++)
					{
						if (data4d_float32[c][k][j][i] < th) data4d_float32[c][k][j][i] = p_vmin[c];
					}
		}
		break;
	default:
		v3d_msg("invalid datatype in scaleintensity();\n");
		return false;
	}

	//update min and max
	if (!updateminmaxvalues())
	{
		v3d_msg("Fail to run successfully updateminmaxvalues() in scaleintensity()..\n", false);
		return false;
	}


	updateViews();
	return true;
}

bool My4DImage::binarizeintensity(int channo, double th) //anything < th will be 0, others will be 1
{
	if (channo >= getCDim())
	{
		v3d_msg("Invalid chan parameter in invertcolor();\n");
		return false;
	}

	V3DLONG i, j, k, c;

	V3DLONG channelPageSize = getTotalUnitNumberPerChannel();
	switch (this->getDatatype())
	{
	case V3D_UINT8:
		for (c = 0; c < this->getCDim(); c++)
		{
			if (channo >= 0 && c != channo)
				continue;
			for (k = 0; k < this->getZDim(); k++)
				for (j = 0; j < this->getYDim(); j++)
					for (i = 0; i < this->getXDim(); i++)
					{
						data4d_uint8[c][k][j][i] = (data4d_uint8[c][k][j][i] < th) ? 0 : 1;
					}
		}

		break;

	case V3D_UINT16:
		for (c = 0; c < this->getCDim(); c++)
		{
			if (channo >= 0 && c != channo)
				continue;
			for (k = 0; k < this->getZDim(); k++)
				for (j = 0; j < this->getYDim(); j++)
					for (i = 0; i < this->getXDim(); i++)
					{
						data4d_uint16[c][k][j][i] = (data4d_uint16[c][k][j][i] < th) ? 0 : 1;
					}
		}
		break;

	case V3D_FLOAT32:
		for (c = 0; c < this->getCDim(); c++)
		{
			if (channo >= 0 && c != channo)
				continue;
			for (k = 0; k < this->getZDim(); k++)
				for (j = 0; j < this->getYDim(); j++)
					for (i = 0; i < this->getXDim(); i++)
					{
						data4d_float32[c][k][j][i] = (data4d_float32[c][k][j][i] < th) ? 0 : 1;
					}
		}
		break;
	default:
		v3d_msg("invalid datatype in scaleintensity();\n");
		return false;
	}

	//update min and max
	if (!updateminmaxvalues())
	{
		v3d_msg("Fail to run successfully updateminmaxvalues() in scaleintensity()..\n", false);
		return false;
	}


	updateViews();
	return true;
}



void My4DImage::cleanExistData_butKeepFileName()
{
	V3DLONG i;
	char oldFileName[1024];
	const char * srcFileName = this->getFileName();
	for (i = 0; i < 1024; i++)
	{
		oldFileName[i] = srcFileName[i];
		if (srcFileName[i] == '\0') break;
	}

	this->cleanExistData();

	this->setFileName(oldFileName);
}

void My4DImage::cleanExistData()
{
	cleanExistData_only4Dpointers();
	Image4DSimple::cleanExistData();
}

void My4DImage::cleanExistData_only4Dpointers()
{
	if (data4d_uint8 || data4d_uint16 || data4d_float32 || data4d_virtual) //080416. Only try to free space and set up b_error flag if applicable
	{
		switch (this->getDatatype())
		{
		case V3D_UINT8:
			delete4dpointer_v3d(data4d_uint8, this->getXDim(), this->getYDim(), this->getZDim(), this->getCDim());
			data4d_virtual = 0;
			break;

		case V3D_UINT16:
			delete4dpointer_v3d(data4d_uint16, this->getXDim(), this->getYDim(), this->getZDim(), this->getCDim());
			data4d_virtual = 0;
			break;

		case V3D_FLOAT32:
			delete4dpointer_v3d(data4d_float32, this->getXDim(), this->getYDim(), this->getZDim(), this->getCDim());
			data4d_virtual = 0;
			break;

		default:
			this->setError(1);
			return;
			//break;
		}
	}

	if (p_vmax) { delete[]p_vmax; p_vmax = NULL; }
	if (p_vmin) { delete[]p_vmin; p_vmin = NULL; }
}





/////////////////////////////////////////////////////////////////////////////////////////////////


bool file_exist(const char * filename)
{
	bool exist;

	ifstream tmpf;
	tmpf.open(filename);
	if (!tmpf) exist = false; else exist = true;
	tmpf.close();

	return exist;
}



void My4DImage::updateViews()
{

	//if (p_xy_view) //seems update() will not really update , thus try the stupid brute force method
	//{
	//	p_xy_view->updateViewPlane();
	//}
	///*if (p_yz_view) {p_yz_view->updateViewPlane(); }
	//if (p_zx_view)	{p_zx_view->updateViewPlane(); }*/
	//if (p_focusPointFeatureWidget) { setFocusFeatureViewText(); } // p_focusPointFeatureWidget->update();}
}

void My4DImage::createColorMap(int len, ImageDisplayColorType c)
{
	if (colorMap)
	{
		delete colorMap;
		colorMap = NULL;
	}

	//colorMap = new ColorMap(colorPseudoMaskColor, len);
	colorMap = new ColorMap(c, len); //070717
}

void My4DImage::switchColorMap(int len, ImageDisplayColorType c)
{
	if (colorMap && len > 0 && (c == colorPseudoMaskColor || c == colorArnimFlyBrainColor || c == colorHanchuanFlyBrainColor)) //when switch colormap, must make sure the expected new colormap length/type are valid
	{
		delete colorMap;
		colorMap = NULL;
	}

	colorMap = new ColorMap(c, len); //
									 //updateViews();//by PHC, 090211 //comment off by RZC 110804, no need
}

void My4DImage::getColorMapInfo(int & len, ImageDisplayColorType & c)
{
	if (colorMap)
	{
		len = colorMap->len;
		c = colorMap->ctype;
	}
}




