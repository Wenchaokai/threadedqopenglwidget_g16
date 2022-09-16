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
#include <GL/glew.h>
#include "../3drenderer/glwidget.h"
#include "../3drenderer/renderer.h"

#include "../3drenderer/mainwindow.h"

#include <QApplication>
#include <QMainWindow>
#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
	#include <qscreen.h>
#else
	#include <QDesktopWidget>
#endif

#include <QSurfaceFormat>
#include <QOpenGLContext>
#include <QCommandLineParser>
#include <QCommandLineOption>


using namespace std; 

//static QString getGlString(QOpenGLFunctions *functions, GLenum name)
//{
//    if (const GLubyte *p = functions->glGetString(name))
//        return QString::fromLatin1(reinterpret_cast<const char *>(p));
//    return QString();
//}

static QString getGlString(GLenum name) {
	if (const GLubyte* p = glGetString(name))
		    return QString::fromLatin1(reinterpret_cast<const char *>(p));
	return QString();
}




//inline
RGBA32f sampling3dRGBA8(/*RGBA32f& sample,*/ RGBA8* data, V3DLONG dim1, V3DLONG dim2, V3DLONG dim3,
	V3DLONG x, V3DLONG y, V3DLONG z, V3DLONG dx, V3DLONG dy, V3DLONG dz)
{
	RGBA32f avg;
	avg.r = avg.g = avg.b = avg.a = 0;
	float d = (dx*dy*dz);
	if (d>0 && x >= 0 && y >= 0 && z >= 0 && x + dx <= dim1 && y + dy <= dim2 && z + dz <= dim3)
	{
		V3DLONG xi, yi, zi;
		for (zi = 0; zi<dz; zi++)
			for (yi = 0; yi<dy; yi++)
				for (xi = 0; xi<dx; xi++)
				{
					RGBA8 tmp = data[(z + zi)*dim2*dim1 + (y + yi)*dim1 + (x + xi)];
					avg.r += tmp.r;
					avg.g += tmp.g;
					avg.b += tmp.b;
					avg.a += tmp.a;
				}
		avg.r /= d;
		avg.g /= d;
		avg.b /= d;
		avg.a /= d;
		//sample = avg;
	}
	return avg;
}


//copy from "..\3drenderer\renderer_gl1.h" _safeReference3DBuf, and modified
void _safeReference3DBuf(RGBA8* rgbaBuf, RGBA8* &_safe3DBuf, int bufX, int bufY, int bufZ,
	int &safeX, int &safeY, int &safeZ, V3DInfo * v3dInfo)
{

	V3DLONG limitX = v3dInfo->limit_X;
	V3DLONG limitY = v3dInfo->limit_Y;
	V3DLONG limitZ = v3dInfo->limit_Z;

	V3DLONG fillX = power_of_two_ceil(bufX);
	V3DLONG fillY = power_of_two_ceil(bufY);
	V3DLONG fillZ = power_of_two_ceil(bufZ);
	if (fillX <= limitX && fillY <= limitY && fillZ <= limitZ)
	{
		safeX = bufX;
		safeY = bufY;
		safeZ = bufZ;
		try {
			_safe3DBuf = new RGBA8[bufX * bufY * bufZ];
		}
		catch (const bad_alloc& e) {
			return;
		}
		memcpy(_safe3DBuf, rgbaBuf, bufX * bufY * bufZ * sizeof(RGBA8));
		return;
	}

	float sx, sy, sz;
	V3DLONG dx, dy, dz;
	sx = float(bufX) / MIN(limitX, bufX);
	sy = float(bufY) / MIN(limitY, bufY);
	sz = float(bufZ) / MIN(limitZ, bufZ);
	dx = V3DLONG(sx);
	dy = V3DLONG(sy);
	dz = V3DLONG(sz);
	//MESSAGE_ASSERT(dx*dy*dz >= 1); // down sampling
	limitX = V3DLONG(bufX / sx);
	limitY = V3DLONG(bufY / sy);
	limitZ = V3DLONG(bufZ / sz);
	safeX = limitX;
	safeY = limitY;
	safeZ = limitZ;
	//qDebug("	safe = %dx%dx%d", limitX, limitY, limitZ);

	try {
		_safe3DBuf = new RGBA8[safeX*safeY*safeZ];
	}
	catch (const bad_alloc& e) {
		return;
	}
	//memset(_safe3DBuf, 0, sizeof(RGBA8)*safeX*safeY*safeZ);
	V3DLONG ox, oy, oz;
	V3DLONG ix, iy, iz;
	for (oz = 0; oz < safeZ; oz++)
		for (oy = 0; oy < safeY; oy++)
			for (ox = 0; ox < safeX; ox++)
			{
				ix = CLAMP(0, bufX - 1, IROUND(ox*sx));
				iy = CLAMP(0, bufY - 1, IROUND(oy*sy));
				iz = CLAMP(0, bufZ - 1, IROUND(oz*sz));

				RGBA32f rgbaf;
				RGBA8 rgba8;
				rgbaf = sampling3dRGBA8(rgbaBuf, bufX, bufY, bufZ, ix, iy, iz, dx, dy, dz);
				rgba8.r = (unsigned char)rgbaf.r;
				rgba8.g = (unsigned char)rgbaf.g;
				rgba8.b = (unsigned char)rgbaf.b;
				rgba8.a = (unsigned char)rgbaf.a;

				_safe3DBuf[oz*(safeY*safeX) + oy*(safeX)+ox] = rgba8;
			}
}





void data4dp_to_rgba3d_copy(My4DImage * img4dp, V3DLONG dim5,
	V3DLONG start1, V3DLONG start2, V3DLONG start3, V3DLONG start4,
	V3DLONG size1, V3DLONG size2, V3DLONG size3, V3DLONG size4,
	RGBA8* rgbaBuf, V3DLONG bufSize[5],
	unsigned int *hist_min, unsigned int *hist_max,
	unsigned int*  vrange,
	RGBA16** p_luts16,
	int channels_On,
	unsigned int *channel_index_On,
	MixOP mixOp, int nthreads
)
{
	if (rgbaBuf == 0 || bufSize == 0)
		return;

	//add copyRaw2QImage_Slice
	ImagePixelType dtype;
	unsigned short int **** p4d = (unsigned short int ****)img4dp->getData(dtype);
	//void **** p4d = img4dp->getData(dtype);
	if (!p4d)
	{
		v3d_error("Fail to run the data4dp_to_rgba3d_copy(), no image data pointer now.");
		return;
	}

	bool bRescale = mixOp.rescale;
	float fb = mixOp.brightness;
	float fc = mixOp.contrast;
	int op = mixOp.op;
	RGB8 mask;
	mask.r = (mixOp.maskR) ? 255 : 0;
	mask.g = (mixOp.maskG) ? 255 : 0;
	mask.b = (mixOp.maskB) ? 255 : 0;
	unsigned int fb_128 = fb * 128 + 128; //Normalized from float [-1,1) to unsigned int [0,256)
	unsigned int fc_128 = (CLAMP(0, 128, fc) * 128) + 128; //Normalized from float [0,1.633123935319537e+16) to unsigned int [128,128*128+128=16512)

														   //unsigned char* mC = new unsigned char[size4];
														   //memset(mC, 0, size4);

														   //there may be a memory issue? by PHC 20110122
														   //	if (img4dp.su!=1)
														   //	{
														   //		v3d_msg("Your data is not 8bit. Now this data4dp_to_rgba3d(0 function supports only 8bit data.");
														   //		return;
														   //	}

														   //V3DLONG dim1 = img4dp.sx; V3DLONG dim2 = img4dp.sy; V3DLONG dim3 = img4dp.sz;
														   //V3DLONG dim4 = img4dp.sc;
	V3DLONG dim1 = img4dp->getXDim(); V3DLONG dim2 = img4dp->getYDim(); V3DLONG dim3 = img4dp->getZDim();
	V3DLONG dim4 = img4dp->getCDim();
#define SAMPLE(it, ic, ix,iy,iz, dx,dy,dz) \
				(unsigned char)sampling3dUINT8( img4dp, (it*dim4/imageT + ic), \
												ix, iy, iz, dx, dy, dz )

#define SAMPLE2(si, ix,iy,iz, dx,dy,dz, dxyz) \
                        (unsigned char)sampling3dUINT8_2( img4dp, si, ix, iy, iz, dx, dy, dz, dxyz)

	// only convert 1<=dim4<=4 ==> RGBA
	V3DLONG imageX, imageY, imageZ, imageC, imageT;
	{
		imageX = bufSize[0];
		imageY = bufSize[1];
		imageZ = bufSize[2];
		//imageC = MIN(4, size4); // <=4
		imageC = size4; // <=4
		imageT = bufSize[4];
	}
	if (imageX*imageY*imageZ*imageC*imageT == 0)
		return;

	float sx, sy, sz;
	V3DLONG dx, dy, dz;
	sx = float(size1) / imageX;
	sy = float(size2) / imageY;
	sz = float(size3) / imageZ;
	dx = V3DLONG(sx);
	dy = V3DLONG(sy);
	dz = V3DLONG(sz);
	V3DLONG dxyz = dx*dy*dz;
	//MESSAGE_ASSERT(dx*dy*dz >= 1); //down sampling

	V3DLONG ot;
	V3DLONG ox, oy, oz;
	//V3DLONG ix, iy, iz;

	V3DLONG otOffset, ozOffset, oyOffset, oxOffset;

	//V3DLONG SAM0, SAM1, SAM2, SAM3;

	/*unsigned int iz_step, iy_step, ix_step;

	iz_step = (sz < 1) ? 1 : (unsigned int)(sz);
	iy_step = (sy < 1) ? 1 : (unsigned int)(sy);
	ix_step = (sx < 1) ? 1 : (unsigned int)(sx);*/

	float iz_step, iy_step, ix_step;

	iz_step = sz;
	iy_step = sy;
	ix_step = sx;

	V3DLONG otOffset_step = imageZ*imageY*imageX;
	V3DLONG ozOffset_step = imageY*imageX;
	V3DLONG oyOffset_step = imageX;


	for (ot = 0, otOffset = 0; ot < imageT; ot++, otOffset += otOffset_step)
	{
		//SAM0 = ot*dim4 / imageT + 0;
		//SAM1 = SAM0 + 1;
		//SAM2 = SAM0 + 2;
		//SAM3 = SAM0 + 3;
		//otOffset = ot*(imageZ*imageY*imageX);

		vector<thread> threads;
		//uint32 nthreads = 4;
		nthreads = (nthreads <= 0) ? 1 : nthreads;
		int block_size = imageZ / nthreads + 1;
		uint32 thd = 0;
		if (block_size <= 0)
		{
			block_size = imageZ;
			nthreads = 1;
		}


		for (uint32 thd = 0; thd < nthreads; thd++)
		{

			threads.push_back(thread([size4, start1, start2, start3, otOffset, ozOffset, imageX, imageY, imageZ, ix_step, iy_step, iz_step, oyOffset_step, ozOffset_step,
				channels_On, channel_index_On, p4d, bRescale, hist_min, hist_max, vrange, p_luts16, op, mask, rgbaBuf, fc, fb, fc_128, fb_128, thd, block_size]
			{

				unsigned char* mC = new unsigned char[size4];
				memset(mC, 0, size4);

				float ix, iy, iz;
				V3DLONG oz = block_size * thd;
				V3DLONG ozOffset = oz*ozOffset_step;
				V3DLONG end = ((thd + 1)*block_size >= imageZ) ? imageZ : (thd + 1)*block_size;

				iz = start3 + oz*iz_step;

				for (; oz < end; oz++, iz += iz_step, ozOffset += ozOffset_step)
				{

#define HIST_RESCALE(k,C)\
					if(C<hist_min[k])\
					{\
						C = hist_min[k];\
					}\
					else if(C>hist_max[k])\
					{\
						C=hist_max[k];\
					}

#define BRIGHTEN_TRANSFORM( C ) \
					if (fc != 1 || fb != 0) \
					{ \
						unsigned int CC=C;\
						C = C*(fc_128) + (fb_128*vrange[k]); \
						C = C >> 7; \
						if(C<(CC+vrange[k])) \
						{ \
							C = hist_min[k]; \
						} \
						else \
						{ \
							C = C - CC - vrange[k]; \
							C = CLAMP(hist_min[k], hist_max[k], C); \
						} \
					}

					//ozOffset = oz*(imageY*imageX);
					//iz = start3 + CLAMP(0, dim3 - 1, IROUND(oz*sz));
					//iz = (iz >= (dim3 - 1)) ? (dim3 - 1) : iz;
					//V3DLONG iy = start2;
					float iy = start2;

					for (V3DLONG oy = 0, oyOffset = 0; oy < imageY; oy++, iy += iy_step, oyOffset += oyOffset_step)
					{
						//oyOffset = oy*imageX;
						V3DLONG oxOffset = otOffset + ozOffset + oyOffset;
						//iy = start2 + CLAMP(0, dim2 - 1, IROUND(oy*sy));
						//iy = (iy >= (dim2 - 1)) ? (dim2 - 1) : iy;
						//V3DLONG ix = start1;
						float ix = start1;

						for (V3DLONG ox = 0; ox < imageX; ox++, ix += ix_step)
						{
							//ix = start1 + CLAMP(0, dim1 - 1, IROUND(ox*sx));
							//ix = (ix >= (dim1 - 1)) ? (dim1 - 1) : ix;

							V3DLONG ix_norm = (V3DLONG)ix;
							V3DLONG iy_norm = (V3DLONG)iy;
							V3DLONG iz_norm = (V3DLONG)iz;

							//add copyRaw2QImage_Slice
							RGBA8 rgba;
							for (int k_on = 0; k_on < channels_On; k_on++)
							{
								int k = channel_index_On[k_on];
								//unsigned int C = p4d[k][iz][iy][ix];
								unsigned int C = p4d[k][iz_norm][iy_norm][ix_norm];
								HIST_RESCALE(k, C);
								BRIGHTEN_TRANSFORM(C);
								unsigned int CC = C - hist_min[k];
								mC[k] = ((!bRescale) ? C : (((C - hist_min[k]) << 8) - CC) / vrange[k]);
							} //end of K
							RGB8 o = lookup_mix_litone(mC, size4, channel_index_On, channels_On, p_luts16, op, mask);
							rgba.r = o.r;
							rgba.g = o.g;
							rgba.b = o.b;
							//float t = (0.f + rgba.r + rgba.g + rgba.b) / 3.0;
							unsigned int t = (rgba.r + rgba.g + rgba.b) / 3;
							rgba.a = (unsigned char)t;
							rgbaBuf[oxOffset++] = rgba;
						} //end of X

						  //if (imageC == 1) {
						  //	for (ox = 0; ox<imageX; ox++) {
						  //		ix = start1 + CLAMP(0, dim1 - 1, IROUND(ox*sx));
						  //		RGBA8 rgba;
						  //		rgba.r = SAMPLE2(SAM0, ix, iy, iz, dx, dy, dz, dxyz);
						  //		rgba.g = 0;
						  //		rgba.b = 0;
						  //		float t = (0.f + rgba.r + rgba.g + rgba.b);
						  //		rgba.a = (unsigned char)t;
						  //		rgbaBuf[oxOffset++] = rgba;
						  //	}
						  //}

						  //if (imageC == 2) {
						  //	for (ox = 0; ox<imageX; ox++) {
						  //		ix = start1 + CLAMP(0, dim1 - 1, IROUND(ox*sx));
						  //		RGBA8 rgba;
						  //		rgba.r = SAMPLE2(SAM0, ix, iy, iz, dx, dy, dz, dxyz);
						  //		rgba.g = SAMPLE2(SAM1, ix, iy, iz, dx, dy, dz, dxyz);;
						  //		rgba.b = 0;
						  //		float t = (0.f + rgba.r + rgba.g + rgba.b) / 2.0;
						  //		rgba.a = (unsigned char)t;
						  //		rgbaBuf[oxOffset++] = rgba;
						  //	}
						  //}

						  //if (imageC == 3) {
						  //	for (ox = 0; ox<imageX; ox++) {
						  //		ix = start1 + CLAMP(0, dim1 - 1, IROUND(ox*sx));
						  //		RGBA8 rgba;
						  //		rgba.r = SAMPLE2(SAM0, ix, iy, iz, dx, dy, dz, dxyz);
						  //		rgba.g = SAMPLE2(SAM1, ix, iy, iz, dx, dy, dz, dxyz);
						  //		rgba.b = SAMPLE2(SAM2, ix, iy, iz, dx, dy, dz, dxyz);
						  //		float t = (0.f + rgba.r + rgba.g + rgba.b) / 3.0;
						  //		rgba.a = (unsigned char)t;
						  //		rgbaBuf[oxOffset++] = rgba;
						  //	}
						  //}

						  //if (imageC >= 4) {
						  //	for (ox = 0; ox<imageX; ox++) {
						  //		ix = start1 + CLAMP(0, dim1 - 1, IROUND(ox*sx));
						  //		RGBA8 rgba;
						  //		rgba.r = SAMPLE2(SAM0, ix, iy, iz, dx, dy, dz, dxyz);
						  //		rgba.g = SAMPLE2(SAM1, ix, iy, iz, dx, dy, dz, dxyz);
						  //		rgba.b = SAMPLE2(SAM2, ix, iy, iz, dx, dy, dz, dxyz);
						  //		rgba.a = SAMPLE2(SAM3, ix, iy, iz, dx, dy, dz, dxyz);
						  //		rgbaBuf[oxOffset++] = rgba;
						  //	}
						  //}
					} //end of Y
#undef define HIST_RESCALE(k,C)
#undef BRIGHTEN_TRANSFORM( C )
				} //End of Z 

				if (mC)
				{
					delete[]mC; mC = 0;
				}

			} //end of thread func
			));
		}
		try
		{
			for (auto &t : threads) {
				t.join();
			}
			threads.clear();
		}
		catch (...)
		{
			v3d_error("Fail to run the data4dp_to_rgba3d_copy(), start thread failed");
		}

	} // end of T


	  //if (mC) { delete []mC; mC = 0; }
}


//
//struct Channel
//{
//	int n;				// index
//	bool on;
//	RGBA8 color;
//	Channel() { n = 0; on = true;  color.r = color.g = color.b = color.a = 255; }
//	int lutNo = -1;
//};
//struct ChannelSharedData
//{
//	MixOP mixOp;
//	QList<Channel> listChannel;
//	QVector< QVector<RGBA8> > luts;
//	bool bGlass;
//};

inline
//void make_linear_lut_one(RGBA8 color, QVector<RGBA8>& lut)
//{
//	assert(lut.size() == 256); //////// must be
//	for (int j = 0; j<256; j++)
//	{
//		float f = j / 255.0;  //110801 fixed the bug of max=254
//		lut[j].r = color.r *f;
//		lut[j].g = color.g *f;
//		lut[j].b = color.b *f;
//		lut[j].a = color.a;   //only alpha is constant
//	}
//}

void printImage(unsigned char * img, int width, int height, int bw, int bh)
{

	printf("printImage, width:%d, height:%d\n", width, height);
	if (img == NULL)
	{
		printf("printImage, img is null , return width:%d, height:%d\n", width, height);
		return;
	}
	int bytes_per_pixel;

	int datatype = V3D_FLOAT32; 

	switch (datatype)
	{
	case V3D_UINT8:  bytes_per_pixel = 1; break;
	case V3D_UINT16:  bytes_per_pixel = 2; break;
	case V3D_FLOAT32:  bytes_per_pixel = 4; break;
	default:
		v3d_error("getPixel:The data type is unsupported. Nothing done.\n");
		return;
		break;
	}

	for (int jj = 0; jj < height && jj < bh; jj++)
	{
		for (int ii = 0; ii < width && ii < bw; ii++)
		{
			printf("%u ", *(img + jj*width * bytes_per_pixel + ii));
		}
		printf("\n");
	}
}

template<class T>
bool _interpolation3D(Image4DSimple &src, Image4DSimple &dest, double pixel_size, double z_size) {
	try
	{
		int channels = src.getChannels();
		if (pixel_size < z_size) {
			// expanding z dim
			double scale = z_size / pixel_size;
			long long new_z = (long long)(scale * src.getZDim());
			T *data = new T[new_z * src.getXDim() * src.getYDim()*channels];
			if (!data)
			{
				v3d_error("Fail to run the _interpolation3D() function, fail to allocate memory for data.\n");
				return false;
			}
			//dest.setData((unsigned char *)data, src.getXDim(), src.getYDim(), new_z, 1, src.getDatatype());
			dest.setData((unsigned char *)data, src.getXDim(), src.getYDim(), new_z, channels, src.getDatatype());
			dest.setChannels(channels);
			dest.setZDim(new_z); // TBD
			Image4DProxy <Image4DSimple> proxy_src(&src);
			Image4DProxy <Image4DSimple> proxy_dest(&dest);
			for (long long iz = 0; iz < new_z; iz++) {
				double f_z = iz / scale;
				long long g_z = (long long)f_z;
				double d_z = f_z - g_z;
				for (long long iy = 0; iy < src.getYDim(); iy++) {
					for (long long ix = 0; ix < src.getXDim(); ix++) {
						for (long long ic = 0; ic < channels; ic++) {
							if (g_z < src.getZDim() - 1)
								*((T *)proxy_dest.at(ix, iy, iz, ic)) = (1 - d_z) * (*((T *)proxy_src.at(ix, iy, g_z, ic))) + d_z * (*((T *)proxy_src.at(ix, iy, g_z + 1, ic)));
							else if (g_z == src.getZDim() - 1)
								*((T *)proxy_dest.at(ix, iy, iz, ic)) = *((T *)proxy_src.at(ix, iy, g_z, ic));
							else
								*((T *)proxy_dest.at(ix, iy, iz, ic)) = 0;
						}
					}
				}
			}
		}
		else {
			// bilinear interpolation on x-y plane
			double scale = pixel_size / z_size;
			long long new_x = (long long)(src.getXDim() * scale), new_y = (long long)(src.getYDim() * scale);
			T *data = new T[new_x * new_y * src.getZDim()*channels];
			dest.setData((unsigned char *)data, new_x, new_y, src.getZDim(), channels, src.getDatatype());
			dest.setChannels(channels);
			dest.setZDim(src.getZDim());// TBD
			Image4DProxy <Image4DSimple> proxy_src(&src);
			Image4DProxy <Image4DSimple> proxy_dest(&dest);
			for (long long iz = 0; iz < src.getZDim(); iz++) {
				for (long long iy = 0; iy < new_y; iy++) {
					// transfer back, new y (long long) => old y (float)
					double f_y = iy / scale;
					long long g_y = (long long)f_y;
					double d_y = f_y - g_y;
					for (long long ix = 0; ix < new_x; ix++) {
						double f_x = ix / scale;
						long long g_x = (long long)f_x;
						double d_x = f_x - g_x;

						for (long long ic = 0; ic < channels; ic++) {

							if (g_x < src.getXDim() - 1) {
								if (g_y < src.getYDim() - 1) {
									*((T *)proxy_dest.at(ix, iy, iz, ic)) = (1 - d_x) * (1 - d_y) * (*((T *)proxy_src.at(g_x, g_y, iz, ic))) +
										d_x * (1 - d_y) * (*((T *)proxy_src.at(g_x + 1, g_y, iz, ic))) +
										(1 - d_x) * d_y * (*((T *)proxy_src.at(g_x, g_y + 1, iz, ic))) +
										d_x * d_y * (*((T *)proxy_src.at(g_x + 1, g_y + 1, iz, ic)));
								}
								else if (g_y == src.getYDim() - 1) {
									*((T *)proxy_dest.at(ix, iy, iz, ic)) = (1 - d_x) * (*((T *)proxy_src.at(g_x, g_y, iz, ic))) +
										d_x * (*((T *)proxy_src.at(g_x + 1, g_y, iz, ic)));
								}
								else
									*((T *)proxy_dest.at(ix, iy, iz, ic)) = 0;
							}
							else if (g_x == src.getXDim() - 1) {
								if (g_y < src.getYDim() - 1) {
									*((T *)proxy_dest.at(ix, iy, iz, ic)) = (1 - d_y) * (*((T *)proxy_src.at(g_x, g_y, iz, ic))) +
										d_y * (*((T *)proxy_src.at(g_x, g_y + 1, iz, ic)));
								}
								else if (g_y == src.getYDim() - 1)
									*((T *)proxy_dest.at(ix, iy, iz, ic)) = *((T *)proxy_src.at(g_x, g_y, iz, ic));
								else
									*((T *)proxy_dest.at(ix, iy, iz, ic)) = 0;
							}
							else
								*((T *)proxy_dest.at(ix, iy, iz, ic)) = 0;
						}

					}
				}
			}
		}
		return true;
	}
	catch (...)
	{
		v3d_error("Fail to run the _interpolation3D() function.\n");
		return false;
	}

}

bool interpolation3D(Image4DSimple &src, Image4DSimple &dest, double pixel_size, double z_size) {
	//assert(src.getCDim() == 1);
	Image4DSimple tmp;
	bool ret = false;
	switch (src.getDatatype())
	{
	case V3D_UINT8: {
		ret = _interpolation3D<uint8>(src, tmp, pixel_size, z_size);
		break;
	}
	case V3D_UINT16: {
		ret = _interpolation3D<uint16>(src, tmp, pixel_size, z_size);
		break;
	}
	case V3D_FLOAT32: {
		ret = _interpolation3D<float>(src, tmp, pixel_size, z_size);
		break;
	}
	default: {
		//cerr << "Not implemented!" << endl;
		break;
	}
	}
	tmp.moveTo(dest);
	return ret;
}

bool convertRGBImageStack(My4DImage* imgStack, My4DImage* imgStackRgb, V3DLONG bufSize[] )
{
	ChannelSharedData csData;
	unsigned int *channel_index_On;
	int channels_On;
	unsigned int*  vrange;
	RGBA16** p_luts16;
	unsigned int * hist_min;
	unsigned int * hist_max;

	int channelSize = imgStack->getCDim();

	csData.mixOp.brightness = 0; 
	csData.mixOp.contrast = 1; 

	for (int c = 0; c < channelSize; c++)
	{
		Channel ch; 
		ch.n = c; 
		ch.on = true; 
		ch.color.r = ch.color.g = ch.color.b = 0; ch.color.a = 255;

		if (c == 0)
		{
			ch.color.r = 255;
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

	//csData.mixOp.contrast = 1; 
	//csData.mixOp.brightness = 0.8; 

	//2.compute listChannel on
	//if (channel_index_On) { delete[]channel_index_On; channel_index_On = 0; }
	channel_index_On = new unsigned int[channelSize];
	if (!channel_index_On)
	{
		v3d_error("Fail to run the V3DEngine::prepare() function, fail in channel_index_On create!\n");
		return false;
	}
	bool allChannelOn = true; 
	if (allChannelOn)
	{
		for (int c = 0; c < channelSize; c++)
		{
			channel_index_On[c] = c;
		}
		channels_On = channelSize;
	}
	else
	{
		channels_On = 0;
		for (int c = 0; c < channelSize; c++)
		{
			if (csData.listChannel[c].on)
			{
				channel_index_On[channels_On] = c;
				channels_On++;
			}
		}
	}
	if (channels_On <= 0)
	{
		v3d_error("Fail to run the V3DEngine::prepare() function, no channel selected!\n");
		return false;
	}

	//3.compute hist vrange
	//if (vrange) { delete[]vrange; vrange = 0; }
	vrange = new unsigned int[channelSize];
	//if (hist_min) { delete[]hist_min; hist_min = 0; }
	hist_min = new unsigned int[channelSize];
//	if (hist_max) { delete[]hist_max; hist_max = 0; }
	hist_max = new unsigned int[channelSize];
	if (!vrange || !hist_min || !hist_max)
	{
		v3d_error("Fail to run the V3DEngine::prepare() function, fail in vrange or hist_min or hist_max create!\n");
		return false;
	}
#ifdef USE_SIMU_DATA
	/*unsigned int contrastMin = 0;
	unsigned int contrastMax = 512;*/
	unsigned int contrastMin = 300;
	unsigned int contrastMax = 3000;
	/*unsigned int contrastMin = 0;
	unsigned int contrastMax = 512;*/
#else
	unsigned int contrastMin = 300;
	unsigned int contrastMax = 3000;
#endif
	//unsigned int contrastMin = 400; 
	////unsigned int contrastMin = 0;
	////unsigned int contrastMax = 11111;
	//unsigned int contrastMax = 10000;
	//unsigned int contrastMax = 1024;

#ifdef USE_SIMU_DATA 

	/*for (int c = 0; c < channelSize; c++)
	{
		hist_min[c] = contrastMin;
		hist_max[c] = contrastMax;
		vrange[c] = hist_max[c] - hist_min[c];
		if (vrange[c] <= 0)  vrange[c] = 1;
	}*/

	for (int c = 0; c < channelSize; c++)
	{
		if (c == 0)
		{
			hist_min[c] = 310;
			hist_max[c] = 923;
		}
		else if (c == 1)
		{
			hist_min[c] = 342;
			hist_max[c] = 3545;
		}
		else if (c == 2)
		{
			hist_min[c] = 343;
			hist_max[c] = 526;
		}
		vrange[c] = hist_max[c] - hist_min[c];
		if (vrange[c] <= 0)  vrange[c] = 1;
	}

	/*for (int c = 0; c < channelSize; c++)
	{
		hist_min[c] = contrastMin;
		hist_max[c] = contrastMax;
		vrange[c] = hist_max[c] - hist_min[c];
		if (vrange[c] <= 0)  vrange[c] = 1;
	}*/
#else
	for (int c = 0; c < channelSize; c++)
	{
		if (c == 0)
		{
			hist_min[c] = 343;
			hist_max[c] = 942;
		}
		else if (c == 1)
		{
			hist_min[c] = 346;
			hist_max[c] = 3502;
		}
		else if (c == 2)
		{
			hist_min[c] = 329;
			hist_max[c] = 518;
		}
		vrange[c] = hist_max[c] - hist_min[c];
		if (vrange[c] <= 0)  vrange[c] = 1;
	}

#endif

	QVector< QVector<RGBA8> > luts;


	//4.compute lookup-tables
	if (csData.luts.isEmpty())
	{
		QVector<RGBA8> lut(256);
		for (int k = 0; k < channelSize; k++)
		{
			
			make_linear_lut_one(csData.listChannel[k].color, lut);
			csData.luts.push_back(lut);
		}
	}
	//if (p_luts16) { delete[]p_luts16; p_luts16 = 0; }
	p_luts16 = new RGBA16*[channelSize];
	if (!p_luts16)
	{
		v3d_error("Fail to run the V3DEngine::prepare() function, fail in p_luts16 create!\n");
		return false;
	}


	for (int ch = 0; ch < channelSize; ch++)
	{
		RGBA16* ch_luts = new RGBA16[256];

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
		p_luts16[ch] = ch_luts;

	}


	size_t ct1 = clock();

	V3DLONG total_rgbaBuf_size = bufSize[0] * bufSize[1] * bufSize[2];

	RGBA8* total_rgbaBuf = 0;

	if (!total_rgbaBuf) // allocate the mem
	{
		try {
			total_rgbaBuf = new RGBA8[total_rgbaBuf_size];
		}
		catch (const bad_alloc& e) {
			return false;
		}
	}


	//[End] PM
	size_t ct21 = clock();
	v3d_debug("V3DEngine::updateData(). 2.1 new total_rgbaBuf total time(ms):%lu", ct21 - ct1);


	int nthreads = 2;

	
	data4dp_to_rgba3d_copy(imgStack, 1,
		0, 0, 0, 0,
		imgStack->getXDim(), imgStack->getYDim(), imgStack->getZDim(), imgStack->getChannels(),
		total_rgbaBuf, bufSize,
		hist_min, hist_max, vrange, p_luts16, channels_On, channel_index_On, csData.mixOp, nthreads);

	size_t ct2 = clock();
	v3d_debug("V3DEngine::updateData(). 2 data4dp_to_rgba3d_copy total time(ms):%lu, nthreads:%d \n", ct2 - ct1, nthreads);

	int safeX, safeY, safeZ; 

	safeX = bufSize[0]; 
	safeY = bufSize[1]; 
	safeZ = bufSize[2];

	V3DInfo v3dInfo; 

	v3dInfo.limit_X = safeX; 
	v3dInfo.limit_Y = safeY;
	v3dInfo.limit_Z = safeZ;

	RGBA8* tex3DBuf = NULL;
	_safeReference3DBuf(total_rgbaBuf, tex3DBuf, bufSize[0], bufSize[1], bufSize[2], safeX, safeY, safeZ, &v3dInfo);
	size_t ct3 = clock();
	v3d_debug("V3DEngine::updateData(). 3 _safeReference3DBuf total time(ms):%lu\n", ct3 - ct2);

	ImagePixelType datatype = V3D_FLOAT32;
	imgStackRgb->createImage(safeX, safeY, safeZ, 1, datatype); //RGBA
	imgStackRgb->setChannels(1); //
	
	for (int z = 0; z < safeZ; z++)
	{
		bool ret = imgStackRgb->updateData((unsigned char*)(tex3DBuf + z*safeX * safeY), safeX, safeY,z, 0, 0, 0, V3D_FLOAT32);
		if (!ret )
		{
			v3d_error("Fail to run the V3DEngine::updateData() function, fail in updateImage(), retCode=%d\n", ret);
			return false;
		}
	}
	//imgStackRgb->setupData4D();

	return true; 
	
}


int main( int argc, char ** argv )
{
#ifdef _WIN32 
	if (AttachConsole(ATTACH_PARENT_PROCESS) || AllocConsole())
	{
		freopen("CONOUT$", "w", stdout);
		freopen("CONOUT$", "w", stderr);
	}
#endif


	QCoreApplication::setAttribute(Qt::AA_UseDesktopOpenGL);

    QApplication a( argc, argv );

    QCoreApplication::setApplicationName("Qt Threaded QOpenGLWidget Example");
    QCoreApplication::setOrganizationName("QtProject");
    QCoreApplication::setApplicationVersion(QT_VERSION_STR);

    QCommandLineParser parser;
    parser.setApplicationDescription(QCoreApplication::applicationName());
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption singleOption("single", "Single thread");
    parser.addOption(singleOption);
    parser.process(a);

    // Two top-level windows with two QOpenGLWidget children in each.
    // The rendering for the four QOpenGLWidgets happens on four separate threads.
	V3DLONG bufSize[5];

	My4DImage* imgStack = new My4DImage(); 
	My4DImage* imgStackRgb = new My4DImage(); 
	My4DImage* imgStackRgb_bin2 = new My4DImage();

	double pixel_size = 6.5147;
	double z_size = 10;

#ifdef USE_SIMU_DATA
	int width, height, zslice, channels, p, tp;
	int acq_Order;
	width = 512;
	height = 512;
	zslice = 201;
	channels = 3;
	p = 1;
	tp = 1;
	acq_Order = ACQ_Order_XYCZT; // 

	imgStack->setFileName("D://save//Data-Axolotl-2020-09-08-v2//multiview_0//multiview_t_0_p_0.tiff");
#else
	int width, height, zslice, channels, p, tp;
	int acq_Order;
	width = 1024;
	height = 1024;
	zslice = 281;
	channels = 3;
	p = 1;
	tp = 1;
	acq_Order = ACQ_Order_XYCZT; // 

	imgStack->setFileName("D://clientdata//Data-Axolotl-2020-09-08//multiview_0//multiview_0_t_0_p_0.tiff");
#endif
	
	imgStack->setCacheState(Cache_Swapout_Saved);
	imgStack->setXDim(width);
	imgStack->setYDim(height);
	imgStack->setZDim(zslice);

	imgStack->setCDim(channels);
	imgStack->setChannels(channels);
	imgStack->setPDim(p); 
	imgStack->setTDim(tp);
	imgStack->setAcqOrder(acq_Order);

	imgStack->setDatatype(V3D_UINT16);
	imgStack->setTimePackType(TIME_PACK_C);
	imgStack->setKDim(KIND_GREY16);

	bool running = true; 

	imgStack->loadImage_Cache(running, 0, 0 );
	imgStack->setupData4D();

	bufSize[0] = imgStack->getXDim(); 
	bufSize[1] = imgStack->getYDim();
	bufSize[2] = imgStack->getZDim();
	bufSize[3] = bufSize[4] = 1;  // C, T
	
	convertRGBImageStack(imgStack, imgStackRgb, bufSize);

	imgStackRgb->setCDim(1);
	imgStackRgb->setChannels(1);
	imgStackRgb->setPDim(p);
	imgStackRgb->setTDim(tp);
	imgStackRgb->setAcqOrder(acq_Order);

	ImagePixelType dt;

	for (int z = 0; z < 1; z++)
	{
		unsigned char* image = (unsigned char*)imgStackRgb->getImage(0, 0, 0, 0, width, height, dt);

		int bw = 16;
		int bh = 16;

		imgStackRgb->printImage(image, width, height, bw, bh);

		v3d_debug( " =========  z:%d =============================== ", z ); 

	}
	imgStackRgb->setupData4D();

	bufSize[0] = imgStack->getXDim() / 2;
	bufSize[1] = imgStack->getYDim() /2 ;
	bufSize[2] = imgStack->getZDim();
	bufSize[3] = bufSize[4] = 1;  // C, T

	convertRGBImageStack(imgStack, imgStackRgb_bin2, bufSize);

	imgStackRgb_bin2->setCDim(1);
	imgStackRgb_bin2->setChannels(1);
	imgStackRgb_bin2->setPDim(p);
	imgStackRgb_bin2->setTDim(tp);
	imgStackRgb_bin2->setAcqOrder(acq_Order);

	
	ImagePixelType dt2;
	v3d_debug(" =========  imgStackRgb_bin2: X: %d, Y:%d, Z:%d =============================== ", imgStackRgb_bin2->getXDim(), imgStackRgb_bin2->getYDim(), imgStackRgb_bin2->getZDim());

	for (int z = 0; z < 1; z++)
	{
		unsigned char* image = (unsigned char*)imgStackRgb->getImage(0, 0, 0, 0, width, height, dt2);

		int bw = 16;
		int bh = 16;

		imgStackRgb->printImage(image, width, height, bw, bh);

		v3d_debug(" =========  z:%d =============================== ", z);

	}

	imgStackRgb_bin2->setupData4D();

	GLWidget topLevelGlWidget;

	topLevelGlWidget.setImageStack(imgStackRgb, imgStackRgb_bin2, imgStack);

	//geometry()是openg的客户区，此处是设置渲染显示框的位置
	QPoint pos = topLevelGlWidget.geometry().topLeft() + QPoint(200, 200);

	topLevelGlWidget.setWindowTitle(QStringLiteral("Threaded QOpenGLWidget example top level"));
	topLevelGlWidget.resize(800, 600);
	topLevelGlWidget.move(pos);
	topLevelGlWidget.show();
	
	return a.exec();
}
