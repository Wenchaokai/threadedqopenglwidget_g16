/*
 * Copyright (c)2006-2010  Hanchuan Peng (Janelia Farm, Howard Hughes Medical Institute).
 * All rights reserved.
 */


/************
                                            ********* LICENSE NOTICE ************

This folder contains all source codes for the V3D project, which is subject to the following conditions if you want to use it.

You will ***have to agree*** the following terms, *before* downloading/using/running/editing/changing any portion of codes in this package.

1. This package is free for non-profit research, but needs a special license for any commercial purpose. Please contact Hanchuan Peng for details.

2. You agree to appropriately cite this work in your related studies and publications.

Peng, H., Ruan, Z., Long, F., Simpson, J.H., and Myers, E.W. (2010) “V3D enables real-time 3D visualization and quantitative analysis of large-scale biological image data sets,” Nature Biotechnology, Vol. 28, No. 4, pp. 348-353, DOI: 10.1038/nbt.1612. ( http://penglab.janelia.org/papersall/docpdf/2010_NBT_V3D.pdf )

Peng, H, Ruan, Z., Atasoy, D., and Sternson, S. (2010) “Automatic reconstruction of 3D neuron structures using a graph-augmented deformable model,” Bioinformatics, Vol. 26, pp. i38-i46, 2010. ( http://penglab.janelia.org/papersall/docpdf/2010_Bioinfo_GD_ISMB2010.pdf )

3. This software is provided by the copyright holders (Hanchuan Peng), Howard Hughes Medical Institute, Janelia Farm Research Campus, and contributors "as is" and any express or implied warranties, including, but not limited to, any implied warranties of merchantability, non-infringement, or fitness for a particular purpose are disclaimed. In no event shall the copyright owner, Howard Hughes Medical Institute, Janelia Farm Research Campus, or contributors be liable for any direct, indirect, incidental, special, exemplary, or consequential damages (including, but not limited to, procurement of substitute goods or services; loss of use, data, or profits; reasonable royalties; or business interruption) however caused and on any theory of liability, whether in contract, strict liability, or tort (including negligence or otherwise) arising in any way out of the use of this software, even if advised of the possibility of such damage.

4. Neither the name of the Howard Hughes Medical Institute, Janelia Farm Research Campus, nor Hanchuan Peng, may be used to endorse or promote products derived from this software without specific prior written permission.

*************/




/*******************************************************************************************
 *
 * basic_4dimage_create.cpp
 *
 * separate from basic_4dimage.cpp and .h for for better modularity
 * by Hanchuan Peng, 20120413.
 *
 *******************************************************************************************
 */

#include "basic_4dimage.h"

#include <stdio.h>

bool Image4DSimple::createImage(V3DLONG mysz0, V3DLONG mysz1, V3DLONG mysz2, V3DLONG mysz3, ImagePixelType mytype)
{
	if (mysz0<=0 || mysz1<=0 || mysz2<=0 || mysz3<=0) return false; //note that for this sentence I don't change b_error flag
	if (data1d) {delete []data1d; data1d=0; sz0=0; sz1=0; sz2=0;sz3=0; datatype=V3D_UNKNOWN;}
	try //081001
	{
		switch (mytype)
		{
			case V3D_UINT8:
				data1d = new unsigned char [mysz0*mysz1*mysz2*mysz3];
				if (!data1d) {b_error=1;return false;}
				break;
			case V3D_UINT16:
				data1d = new unsigned char [mysz0*mysz1*mysz2*mysz3*2];
				if (!data1d) {b_error=1;return false;}
				break;
			case V3D_FLOAT32:
				data1d = new unsigned char [mysz0*mysz1*mysz2*mysz3*4];
				if (!data1d) {b_error=1;return false;}
				break;
			default:
				b_error=1;
				return false;
				break;
		}
		sz0=mysz0; sz1=mysz1; sz2=mysz2;sz3=mysz3; datatype=mytype; b_error=0; //note that here I update b_error

		isSaved = false;
		isAllUpdated = false; 
		updatedImages = 0;

		V3DLONG stacksize = mysz2*mysz3; 
		statestack_.resize(stacksize);
		for (int ii = 0; ii < stacksize; ii++)
		{
			statestack_[ii] = Cache_Created_Unsaved;
		}

	}
	catch (...)
	{
		v3d_msg("Fail to allocate memory in My4DImageSimple::createImage();\n");
		data1d = 0;
		b_error=1;
		return false;
	}
	return true;
}

void Image4DSimple::createBlankImage(V3DLONG imgsz0, V3DLONG imgsz1, V3DLONG imgsz2, V3DLONG imgsz3, int imgdatatype)
{
	if (imgsz0<=0 || imgsz1<=0 || imgsz2<=0 || imgsz3<=0)
	{
		v3d_msg("Invalid size parameters in createBlankImage(). Do nothing. \n");
		return;
	}
	if (imgdatatype!=1 && imgdatatype!=2 && imgdatatype!=4)
	{
		v3d_msg("Invalid image datatype parameter in createBlankImage(). Do nothing. \n");
		return;
	}
	
	//otherwise good to go
	
	cleanExistData(); /* note that this variable must be initialized as NULL. */
	
	strcpy(imgSrcFile, "Untitled_blank.v3draw");
	
	//==============
	
	switch (imgdatatype)
	{
		case 1:
			datatype = V3D_UINT8;
			break;
			
		case 2:
			datatype = V3D_UINT16;
			break;
			
		case 4:
			datatype = V3D_FLOAT32;
			break;
			
		default:
			v3d_msg("Something wrong with the program in My4DImageSimple::createBlankImage(), -- should NOT display this message at all. Check your program. \n");
			b_error=1;
			return;
	}
	
	sz0 = imgsz0;
	sz1 = imgsz1;
	sz2 = imgsz2;
	sz3 = imgsz3;
	
	V3DLONG totalbytes = sz0*sz1*sz2*sz3*imgdatatype;
	try { //081001
		data1d = new unsigned char [totalbytes];
	}catch (...) {data1d=0;}
	
	if (!data1d)
	{
		v3d_msg("Fail to allocate memory in My4DImageSimple::createBlankImage(). Check your program. \n");
		sz0=sz1=sz2=sz3=0; datatype=V3D_UNKNOWN;
		b_error=1;
		return;
	}
	
	//[Litone]
	//add by lujian 2018/9/11. TBD memset
	//for (V3DLONG i=0;i<totalbytes;i++) data1d[i] = 0;
	memset(data1d, 0, totalbytes); 

	isSaved = false;
	isAllUpdated = false;
	updatedImages = 0; 

	V3DLONG stacksize = sz2*sz3;
	statestack_.resize(stacksize);
	for (int ii = 0; ii < stacksize; ii++)
	{
		statestack_[ii] = Cache_Created_Unsaved;
	}
	

	//end
	return;
}


void Image4DSimple::blankImageData()
{
	V3DLONG totalbytes = sz0*sz1*sz2*sz3*datatype;
	
	if (data1d)
	{
		cacheState = Cache_Create_Blank;
		memset(data1d, 0, totalbytes);
	}
	else 
		cacheState = Cache_Init;

	isSaved = false;
	isAllUpdated = false;
	updatedImages = 0;

	V3DLONG stacksize = sz2*sz3;
	statestack_.resize(stacksize);
	for (int ii = 0; ii < stacksize; ii++)
	{
		statestack_[ii] = (Image_Cache_State)cacheState;
	}
}

bool convert_data_to_8bit(void * &img, V3DLONG * sz, int datatype)
{
	if (!img || !sz)
	{
		fprintf(stderr, "The input to convert_data_to_8bit() are invalid [%s][%d].\n", __FILE__, __LINE__);
		return false;
	}
    
	if (datatype!=2 && datatype!=4)
	{
		fprintf(stderr, "This function convert_type2uint8_3dimg_1dpt() is designed to convert 16 bit and single-precision-float only [%s][%d].\n", __FILE__, __LINE__);
		return false;
	}
    
	if (sz[0]<1 || sz[1]<1 || sz[2]<1 || sz[3]<1)
	{
		fprintf(stderr, "Input image size is not valid [%s][%d].\n", __FILE__, __LINE__);
		return false;
	}
    
	V3DLONG totalunits = sz[0] * sz[1] * sz[2] * sz[3];
	unsigned char * outimg = new unsigned char [totalunits];
	if (!outimg)
	{
		fprintf(stderr, "Fail to allocate memory. [%s][%d].\n", __FILE__, __LINE__);
		return false;
	}
    
	if (datatype==2) //following is new method 090718, PHC
	{
		unsigned short int * tmpimg = (unsigned short int *)img;
		V3DLONG i; double maxvv=tmpimg[0];
		for (i=0;i<totalunits;i++)
		{
			maxvv = (maxvv<tmpimg[i]) ? tmpimg[i] : maxvv;
		}
		if (maxvv>255.0)
		{
			maxvv = 255.0/maxvv;
            for (/*V3DLONG*/ i=0;i<totalunits;i++)
			{
				outimg[i] = (unsigned char)(double(tmpimg[i])*maxvv);
			}
		}
		else
		{
            for (/*V3DLONG*/ i=0;i<totalunits;i++)
			{
				outimg[i] = (unsigned char)(tmpimg[i]); //then no need to rescale
			}
		}
	}
	else
	{
		float * tmpimg = (float *)img;
		V3DLONG i; double maxvv=tmpimg[0], minvv=tmpimg[0];
		for (i=0;i<totalunits;i++)
		{
			if (tmpimg[i]>maxvv) maxvv = tmpimg[i];
			else if (tmpimg[i]<minvv) minvv = tmpimg[i];
		}
		if (maxvv!=minvv)
		{
			double w = 255.0/(maxvv-minvv);
            for (/*V3DLONG*/ i=0;i<totalunits;i++)
			{
				outimg[i] = (unsigned char)(double(tmpimg[i]-minvv)*w);
			}
		}
		else
		{
            for (/*V3DLONG*/ i=0;i<totalunits;i++)
			{
				outimg[i] = (unsigned char)(tmpimg[i]); //then no need to rescale. If the original value is small than 0 or bigger than 255, then let it be during the type-conversion
			}
		}
	}
    
	//copy to output data
    
	delete [] ((unsigned char *)img); //as I know img was originally allocated as (unsigned char *)
	img = outimg;
    
	return true;
}


//add by eva 2019 - 9 - 19, used in plugin 3d stitch
Image4DSimple::Image4DSimple(const Image4DSimple &src) {
	cloneFrom(src);
}
bool Image4DSimple::convert_to_FLOAT32() {
	if (this->datatype == V3D_FLOAT32) {
		return true;
	}

	if (this->datatype != V3D_UINT8 && this->datatype != V3D_UINT16) {
		fprintf(stderr, "This function convert_type2float32 is designed to convert 8 bit and 16bit only [%s][%d].\n", __FILE__, __LINE__);
		return false;
	}

	if (sz0 < 1 || sz1 < 1 || sz2 < 1 || sz3 < 1) {
		fprintf(stderr, "Input image size is not valid [%s][%d].\n", __FILE__, __LINE__);
		return false;
	}

	V3DLONG totalunits = sz0*sz1*sz2*sz3;
	float* outimg = new float[totalunits];
	if (!outimg) {
		fprintf(stderr, "Fail to allocate memory. [%s][%d].\n", __FILE__, __LINE__);
		return false;
	}

	if (datatype == V3D_UINT8) {
		v3d_uint8 * tmpimg = (v3d_uint8 *)this->getRawData();
		for (V3DLONG i = 0; i < totalunits; i++) {
			outimg[i] = (v3d_float32)(tmpimg[i]);
		}
	}
	else {
		v3d_uint16 * tmpimg = (v3d_uint16 *)this->getRawData();
		for (V3DLONG i = 0; i < totalunits; i++) {
			outimg[i] = (v3d_float32)(tmpimg[i]);
		}
	}

	//delete [] (this->getRawData());
	this->setNewRawDataPointer((unsigned char *)outimg);
	this->setDatatype(V3D_FLOAT32);
	return true;
}

bool Image4DSimple::constant_create(V3DLONG size_x, V3DLONG size_y, V3DLONG size_z, V3DLONG size_c, int constant, ImagePixelType type) {
	switch (type)
	{
	case V3D_UINT8: {
		V3DLONG total_units = size_x * size_y * size_z * size_c;
		uint8 *data_ptr = new uint8[total_units];
		if (!data_ptr)
		{
			v3d_error("Fail to run the Image4DSimple::constant_create() function, fail to allocate memory for data_ptr.\n");
			return false;
		}
		for (V3DLONG i = 0; i < total_units; i++)
			data_ptr[i] = (uint8)constant;
		setData((unsigned char *)data_ptr, size_x, size_y, size_z, size_c, type);
		break;
	}
	case V3D_UINT16: {
		V3DLONG total_units = size_x * size_y * size_z * size_c;
		uint16 *data_ptr = new uint16[total_units];
		if (!data_ptr)
		{
			v3d_error("Fail to run the Image4DSimple::constant_create() function, fail to allocate memory for data_ptr.\n");
			return false;
		}
		for (V3DLONG i = 0; i < total_units; i++)
			data_ptr[i] = (uint16)constant;
		setData((unsigned char *)data_ptr, size_x, size_y, size_z, size_c, type);
		break;
	}
	case V3D_FLOAT32: {
		V3DLONG total_units = size_x * size_y * size_z * size_c;
		float *data_ptr = new float[total_units];
		if (!data_ptr)
		{
			v3d_error("Fail to run the Image4DSimple::constant_create() function, fail to allocate memory for data_ptr.\n");
			return false;
		}
		for (V3DLONG i = 0; i < total_units; i++)
			data_ptr[i] = (float)constant;
		setData((unsigned char *)data_ptr, size_x, size_y, size_z, size_c, type);
		break;
	}
	default: {
		fprintf(stderr, "constant_create is designed for 8 bit, 16bit and float only [%s][%d].\n", __FILE__, __LINE__);
		break;
	}
	}

	return true;
}

/*
pixel-wise '+', the result is stored in the left value of '+'
@param x
right value of binary operator '+'
@return
left value of binary operator '+'
*/
Image4DSimple & Image4DSimple::operator + (Image4DSimple &x) {
	assert(datatype == x.datatype);
	assert(sz0 == x.sz0 && sz1 == x.sz1 && sz2 == x.sz2 && sz3 == x.sz3);

	V3DLONG totalunits = sz0*sz1*sz2*sz3;

	switch (datatype) {
	case V3D_UINT8: {
		v3d_uint8 * tmpimg = (v3d_uint8 *)this->getRawData();
		v3d_uint8 * ptr = (v3d_uint8 *)x.getRawData();
		for (V3DLONG i = 0; i < totalunits; i++)
			tmpimg[i] += ptr[i];
		break;
	}
	case V3D_UINT16: {
		v3d_uint16 * tmpimg = (v3d_uint16 *)this->getRawData();
		v3d_uint16 * ptr = (v3d_uint16 *)x.getRawData();
		for (V3DLONG i = 0; i < totalunits; i++)
			tmpimg[i] += ptr[i];
		break;
	}
	case V3D_FLOAT32: {
		v3d_float32 * tmpimg = (v3d_float32 *)this->getRawData();
		v3d_float32 * ptr = (v3d_float32 *)x.getRawData();
		for (V3DLONG i = 0; i < totalunits; i++)
			tmpimg[i] += ptr[i];
		break;
	}
	default: {
		fprintf(stderr, "Operator \"+\" is designed for 8 bit, 16bit and float only [%s][%d].\n", __FILE__, __LINE__);
		break;
	}
	}
	return *this;
}

/*
pixel-wise '-', the result is stored in the left value of '-'
@param x
right value of binary operator '-'
@return
left value of binary operator '-'
*/
Image4DSimple & Image4DSimple::operator - (Image4DSimple &x) {
	assert(datatype == x.datatype);
	assert(sz0 == x.sz0 && sz1 == x.sz1 && sz2 == x.sz2 && sz3 == x.sz3);

	V3DLONG totalunits = sz0*sz1*sz2*sz3;

	switch (datatype) {
	case V3D_UINT8: {
		v3d_uint8 * tmpimg = (v3d_uint8 *)this->getRawData();
		v3d_uint8 * ptr = (v3d_uint8 *)x.getRawData();
		for (V3DLONG i = 0; i < totalunits; i++)
			tmpimg[i] -= ptr[i];
		break;
	}
	case V3D_UINT16: {
		v3d_uint16 * tmpimg = (v3d_uint16 *)this->getRawData();
		v3d_uint16 * ptr = (v3d_uint16 *)x.getRawData();
		for (V3DLONG i = 0; i < totalunits; i++)
			tmpimg[i] -= ptr[i];
		break;
	}
	case V3D_FLOAT32: {
		v3d_float32 * tmpimg = (v3d_float32 *)this->getRawData();
		v3d_float32 * ptr = (v3d_float32 *)x.getRawData();
		for (V3DLONG i = 0; i < totalunits; i++)
			tmpimg[i] -= ptr[i];
		break;
	}
	default: {
		fprintf(stderr, "Operator \"-\" is designed to 8 bit, 16bit and float only [%s][%d].\n", __FILE__, __LINE__);
		break;
	}
	}
	return *this;
}

/*
pixel-wise '*', the result is stored in the left value of '*'
@param x
right value of binary operator '*'
@return
left value of binary operator '*'
*/
Image4DSimple & Image4DSimple::operator * (Image4DSimple &x) {
	assert(datatype == x.datatype);
	assert(sz0 == x.sz0 && sz1 == x.sz1 && sz2 == x.sz2 && sz3 == x.sz3);

	V3DLONG totalunits = sz0*sz1*sz2*sz3;

	switch (datatype) {
	case V3D_UINT8: {
		v3d_uint8 * tmpimg = (v3d_uint8 *)this->getRawData();
		v3d_uint8 * ptr = (v3d_uint8 *)x.getRawData();
		for (V3DLONG i = 0; i < totalunits; i++)
			tmpimg[i] *= ptr[i];
		break;
	}
	case V3D_UINT16: {
		v3d_uint16 * tmpimg = (v3d_uint16 *)this->getRawData();
		v3d_uint16 * ptr = (v3d_uint16 *)x.getRawData();
		for (V3DLONG i = 0; i < totalunits; i++)
			tmpimg[i] *= ptr[i];
		break;
	}
	case V3D_FLOAT32: {
		v3d_float32 * tmpimg = (v3d_float32 *)this->getRawData();
		v3d_float32 * ptr = (v3d_float32 *)x.getRawData();
		for (V3DLONG i = 0; i < totalunits; i++)
			tmpimg[i] *= ptr[i];
		break;
	}
	default: {
		fprintf(stderr, "Operator \"*\" is designed to 8 bit, 16bit and float only [%s][%d].\n", __FILE__, __LINE__);
		break;
	}
	}
	return *this;
}

/*
pixel-wise '/', the result is stored in the left value of '/'
@param x
right value of binary operator '/'
@return
left value of binary operator '/'
*/
Image4DSimple & Image4DSimple::operator / (Image4DSimple &x) {
	assert(datatype == x.datatype);
	assert(sz0 == x.sz0 && sz1 == x.sz1 && sz2 == x.sz2 && sz3 == x.sz3);

	V3DLONG totalunits = sz0*sz1*sz2*sz3;

	switch (datatype) {
	case V3D_UINT8: {
		v3d_uint8 * tmpimg = (v3d_uint8 *)this->getRawData();
		v3d_uint8 * ptr = (v3d_uint8 *)x.getRawData();
		for (V3DLONG i = 0; i < totalunits; i++)
			tmpimg[i] /= ptr[i];
		break;
	}
	case V3D_UINT16: {
		v3d_uint16 * tmpimg = (v3d_uint16 *)this->getRawData();
		v3d_uint16 * ptr = (v3d_uint16 *)x.getRawData();
		for (V3DLONG i = 0; i < totalunits; i++)
			tmpimg[i] /= ptr[i];
		break;
	}
	case V3D_FLOAT32: {
		v3d_float32 * tmpimg = (v3d_float32 *)this->getRawData();
		v3d_float32 * ptr = (v3d_float32 *)x.getRawData();
		for (V3DLONG i = 0; i < totalunits; i++)
			tmpimg[i] /= ptr[i];
		break;
	}
	default: {
		fprintf(stderr, "Operator \"/\" is designed to 8 bit, 16bit and float only [%s][%d].\n", __FILE__, __LINE__);
		break;
	}
	}
	return *this;
}

/*
pixel-wise '/', the result is stored in the left value of '/'
@param x
right value of binary operator '/'
@return
left value of binary operator '/'
*/
Image4DSimple & Image4DSimple::operator / (const int x) {
	if (datatype == V3D_UINT8) {
		for (long long i = 0; i < sz0*sz1*sz2*sz3; i++) {
			data1d[i] /= x;
		}
	}
	else if (datatype == V3D_UINT16) {
		for (long long i = 0; i < sz0*sz1*sz2*sz3; i++) {
			((uint16 *)data1d)[i] /= x;
		}
	}
	else if (datatype == V3D_FLOAT32) {
		for (long long i = 0; i < sz0*sz1*sz2*sz3; i++) {
			((float *)data1d)[i] /= x;
		}
	}
	return *this;
}

/*
pixel-wise '='
@param x
source data
@return
variable after assignment
*/
Image4DSimple & Image4DSimple::operator = (Image4DSimple &x) {
	if (data1d == x.data1d) {
		Image4DSimple buffer;
		buffer.cloneFrom(x);
		buffer.moveTo(*this);
	}
	else
		cloneFrom(x);
	return *this;
}

bool Image4DSimple::isEmpty_simple() {
	return data1d == NULL;
}

/*
Summation.
@return
the summation of data
*/
double Image4DSimple::sum_simple() {
	V3DLONG totalunits = sz0*sz1*sz2*sz3;
	double s = 0;
	switch (datatype) {
	case V3D_UINT8: {
		v3d_uint8 * tmpimg = (v3d_uint8 *)this->getRawData();
		for (V3DLONG i = 0; i < totalunits; i++)
			s += tmpimg[i];
		break;
	}
	case V3D_UINT16: {
		v3d_uint16 * tmpimg = (v3d_uint16 *)this->getRawData();
		for (V3DLONG i = 0; i < totalunits; i++)
			s += tmpimg[i];
		break;
	}
	case V3D_FLOAT32: {
		v3d_float32 * tmpimg = (v3d_float32 *)this->getRawData();
		for (V3DLONG i = 0; i < totalunits; i++)
			s += tmpimg[i];
		break;
	}
	default: {
		fprintf(stderr, "Function \"sum\" is designed to 8 bit, 16bit and float only [%s][%d].\n", __FILE__, __LINE__);
		break;
	}
	}
	return s;
}

/*
Get average value.
@return
the average value
*/
double Image4DSimple::mean_simple() {
	V3DLONG totalunits = sz0*sz1*sz2*sz3;
	double s = sum_simple() / totalunits;
	return s;
}

/*
Get biggest value.
@return
the biggest value
*/

double Image4DSimple::max_simple() {
	V3DLONG totalunits = sz0*sz1*sz2*sz3;
	double s = 0;
	switch (datatype) {
	case V3D_UINT8: {
		v3d_uint8 * tmpimg = (v3d_uint8 *)this->getRawData();
		for (V3DLONG i = 0; i < totalunits; i++)
			if (s < tmpimg[i])
				s = tmpimg[i];
		break;
	}
	case V3D_UINT16: {
		v3d_uint16 * tmpimg = (v3d_uint16 *)this->getRawData();
		for (V3DLONG i = 0; i < totalunits; i++)
			if (s < tmpimg[i])
				s = tmpimg[i];
		break;
	}
	case V3D_FLOAT32: {
		v3d_float32 * tmpimg = (v3d_float32 *)this->getRawData();
		for (V3DLONG i = 0; i < totalunits; i++)
			if (s < tmpimg[i])
				s = tmpimg[i];
		break;
	}
	default: {
		fprintf(stderr, "Function \"max\" is designed to 8 bit, 16bit and float only [%s][%d].\n", __FILE__, __LINE__);
		break;
	}
	}
	return s;
}

/*
Get smallest value.
@return
the smallest value
*/

double Image4DSimple::min_simple() {

	/*V3DLONG totalunits = sz0*sz1*sz2*sz3;
	double s = numeric_limits<double>::max();
	switch (datatype) {
	case V3D_UINT8: {
		v3d_uint8 * tmpimg = (v3d_uint8 *)this->getRawData();
		for (V3DLONG i = 0; i < totalunits; i++)
			if (s > tmpimg[i])
				s = tmpimg[i];
		break;
	}
	case V3D_UINT16: {
		v3d_uint16 * tmpimg = (v3d_uint16 *)this->getRawData();
		for (V3DLONG i = 0; i < totalunits; i++)
			if (s > tmpimg[i])
				s = tmpimg[i];
		break;
	}
	case V3D_FLOAT32: {
		v3d_float32 * tmpimg = (v3d_float32 *)this->getRawData();
		for (V3DLONG i = 0; i < totalunits; i++)
			if (s > tmpimg[i])
				s = tmpimg[i];
		break;
	}
	default: {
		fprintf(stderr, "Function \"min\" is designed to 8 bit, 16bit and float only [%s][%d].\n", __FILE__, __LINE__);
		break;
	}
	}
	return s;*/
	return 0;
}

/* Operate sqrt() on each pixel. */
void Image4DSimple::pixelSqrt() {
	V3DLONG totalunits = sz0*sz1*sz2*sz3;
	switch (datatype) {
	case V3D_UINT8: {
		v3d_uint8 * tmpimg = (v3d_uint8 *)this->getRawData();
		for (V3DLONG i = 0; i < totalunits; i++)
			tmpimg[i] = sqrt(tmpimg[i]);
		break;
	}
	case V3D_UINT16: {
		v3d_uint16 * tmpimg = (v3d_uint16 *)this->getRawData();
		for (V3DLONG i = 0; i < totalunits; i++)
			tmpimg[i] = sqrt(tmpimg[i]);
		break;
	}
	case V3D_FLOAT32: {
		v3d_float32 * tmpimg = (v3d_float32 *)this->getRawData();
		for (V3DLONG i = 0; i < totalunits; i++)
			tmpimg[i] = sqrt(tmpimg[i]);
		break;
	}
	default: {
		fprintf(stderr, "Function \"pixelSqrt\" is designed to 8 bit, 16bit and float only [%s][%d].\n", __FILE__, __LINE__);
		break;
	}
	}
}

/*
Move internal data to another variable. This function will erase caller variable's data.
@param x
destinate variable
*/
void Image4DSimple::moveTo(Image4DSimple &x) {
	x.cleanExistData();
	x.sz0 = sz0, x.sz1 = sz1, x.sz2 = sz2, x.sz3 = sz3;
	x.sz_time = sz_time;
	x.datatype = datatype;
	x.data1d = data1d;
	x.timepacktype = timepacktype;
	x.sz_channel = sz_channel; 
	x.sz_pos = sz_pos; 
	x.sz_kind = sz_kind; 

	strcpy_s(x.imgSrcFile, imgSrcFile);
	x.b_error = b_error;
	x.rez_x = rez_x, x.rez_y = rez_y, x.rez_z = rez_z;
	x.valid_zslicenum = valid_zslicenum;
	data1d = NULL;
	this->cleanExistData();
}

/*
Clone data from another variable.
@param x
source variable
*/
bool Image4DSimple::cloneFrom(const Image4DSimple &x) {
	this->cleanExistData();
	sz0 = x.sz0, sz1 = x.sz1, sz2 = x.sz2, sz3 = x.sz3;
	sz_time = x.sz_time;
	datatype = x.datatype;
	sz_channel = x.sz_channel;
	sz_pos = x.sz_pos;
	sz_kind = x.sz_kind; 

	data1d = new unsigned char[x.getTotalBytes()];
	if (!data1d)
	{
		v3d_error("Fail to run the Image4DSimple::cloneFrom() function, fail to allocate memory for data1d.\n");
		return false;
	}
	memcpy_s(data1d, x.getTotalBytes(), x.data1d, x.getTotalBytes());
	timepacktype = x.timepacktype;
	strcpy_s(imgSrcFile, x.imgSrcFile);
	b_error = x.b_error;
	rez_x = x.rez_x, rez_y = x.rez_y, rez_z = x.rez_z;
	valid_zslicenum = x.valid_zslicenum;
	return true;
}
