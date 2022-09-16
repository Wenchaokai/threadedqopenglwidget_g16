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




 /*
  * basic_4dimage.cpp
  * last update: 100819: Hanchuan Peng. use MYLIB only for Llinux and Mac, but not WIN32. FIXME: add VC support later.
  * 20120410: add curFileSuffix check for the potential strcmp crashing. by Hanchuan Peng
  */

#include "v3d_message.h"

#include "stackutil.h"
#include "basic_4dimage.h"

#include "mg_image_lib.h"
//#include "v3d_core.h"
#include "../litone/utils.h"

  //extern "C" {
  //#include "../common_lib/src_packages/mylib_tiff/image.h"
  //};

//#ifdef _ALLOW_WORKMODE_MENU_
//#include "../neuron_annotator/utility/ImageLoaderBasic.h"
//#endif

typedef unsigned short int USHORTINT16;

//
// rename multiple files >4G .tiff for one timepoint/pos
//
bool Image4DSimple::renameFile(std::string root, std::string prefix)
{
	//[QT5_MK]
	//b_error = 0;
	//if (!isImageSaved() && !isImageSwapout()) // 
	//{
	//	b_error = 1;
	//	v3d_error("Image is not in saved and swapout status in My4DImageSimple::loadImage2Cache(). \n", false);
	//	return b_error;
	//}

	//const char * curFileSuffix = getSuffix(imgSrcFile);

	//if (!(strcasecmp(curFileSuffix, "tif") == 0 || strcasecmp(curFileSuffix, "tiff") == 0))
	//{
	//	b_error = 1;
	//	return b_error;
	//}

	//QString partFilename = QString::fromStdString(std::string(imgSrcFile));

	//int v_tmp, tp_tmp, pos_tmp, g_tmp;

	//std::string root_tmp, prefix_tmp;

	//std::string newfilename, basename;

	////bool bret = Utils::getTimePositionFromFilename(partFilename, v_tmp, tp_tmp, pos_tmp, g_tmp, root_tmp, prefix_tmp, basename);
	//bool bret = false;
	//if (!bret) // not in LiTone format file name, just copy
	//{
	//	newfilename = root + "/" + basename;

	//	int ret = std::rename(imgSrcFile, newfilename.c_str());
	//	//return true;
	//	if (ret == 0)
	//	{
	//		memset(imgSrcFile, 0, sizeof(imgSrcFile));
	//		memcpy(imgSrcFile, newfilename.c_str(), newfilename.length());
	//	}

	//	return (ret == 0);
	//}

	//QString keyword;
	//if (v_tmp < 0) {
	//	keyword = QString("_t_%1_p_%2").arg(QString::number(tp_tmp), QString::number(pos_tmp));
	//}
	//else {
	//	keyword = QString("_v_%1_t_%2_p_%3").arg(QString::number(v_tmp), QString::number(tp_tmp), QString::number(pos_tmp));
	//}

	//QStringList mylist = Utils::getFileList(partFilename, keyword);

	//v3d_debug("Image4DSimple::copyFile - filename:%s, keyword:%s, mylist.size:%d", partFilename.toStdString().c_str(), keyword.toStdString().c_str(), mylist.size());

	//V3DLONG nfiles = mylist.size();

	//for (V3DLONG ii = 0; ii < nfiles; ++ii)
	//{
	//	QString tmpfileInfo = mylist.at(ii);

	//	bret = Utils::getTimePositionFromFilename(tmpfileInfo, v_tmp, tp_tmp, pos_tmp, g_tmp, root_tmp, prefix_tmp, basename);

	//	if (bret)
	//	{
	//		if (prefix.length() == 0)
	//			prefix = prefix_tmp;

	//		if (v_tmp < 0) {
	//			if (nfiles > 1)
	//				newfilename = root + "/" + prefix + "_t_" + std::to_string(tp_tmp) + "_p_" + std::to_string(pos_tmp) + "_g_" + std::to_string(g_tmp) + ".tiff";
	//			else
	//				newfilename = root + "/" + prefix + "_t_" + std::to_string(tp_tmp) + "_p_" + std::to_string(pos_tmp) + ".tiff";
	//		}
	//		else {
	//			if (nfiles > 1)
	//				newfilename = root + "/" + prefix + "_v_" + std::to_string(v_tmp) + "_t_" + std::to_string(tp_tmp) + "_p_" + std::to_string(pos_tmp) + "_g_" + std::to_string(g_tmp) + ".tiff";
	//			else
	//				newfilename = root + "/" + prefix + "_v_" + std::to_string(v_tmp) + "_t_" + std::to_string(tp_tmp) + "_p_" + std::to_string(pos_tmp) + ".tiff";
	//		}

	//		int ret = std::rename(tmpfileInfo.toStdString().c_str(), newfilename.c_str());
	//		if (ret == 0)
	//		{
	//			memset(imgSrcFile, 0, sizeof(imgSrcFile));
	//			memcpy(imgSrcFile, newfilename.c_str(), newfilename.length());
	//		}
	//	}
	//}
	//[QT5_MK]
	return true;
}
//
// Copy multiple files >4G .tiff for one timepoint/pos, be carefull it will change the filename after copy
//
bool Image4DSimple::copyFile(std::string root, std::string prefix)
{
	//[QT5_MK]
	//b_error = 0;
	//if (!isImageSaved() && !isImageSwapout()) // 
	//{
	//	b_error = 1;
	//	v3d_error("Image is not in saved and swapout status in My4DImageSimple::loadImage2Cache(). \n", false);
	//	return b_error;
	//}

	//const char * curFileSuffix = getSuffix(imgSrcFile);

	//if (!(strcasecmp(curFileSuffix, "tif") == 0 || strcasecmp(curFileSuffix, "tiff") == 0))
	//{
	//	b_error = 1;
	//	return b_error;
	//}

	//QString partFilename = QString::fromStdString(std::string(imgSrcFile));

	//int v_tmp, tp_tmp, pos_tmp, g_tmp;

	//std::string root_tmp, prefix_tmp;

	//std::string newfilename, basename;

	//bool bret = Utils::getTimePositionFromFilename(partFilename, v_tmp, tp_tmp, pos_tmp, g_tmp, root_tmp, prefix_tmp, basename);

	//if (!bret) // not in LiTone format file name, just copy
	//{
	//	newfilename = root + "/" + basename;
	//	QFile::copy(partFilename, QString::fromStdString(newfilename));
	//	return true;
	//}

	//QString keyword;
	//if (v_tmp < 0) {
	//	keyword = QString("_t_%1_p_%2").arg(QString::number(tp_tmp), QString::number(pos_tmp));
	//}
	//else {
	//	keyword = QString("_v_%1_t_%2_p_%3").arg(QString::number(v_tmp), QString::number(tp_tmp), QString::number(pos_tmp));
	//}

	//QStringList mylist = Utils::getFileList(partFilename, keyword);

	//v3d_debug("Image4DSimple::copyFile - filename:%s, keyword:%s, mylist.size:%d", partFilename.toStdString().c_str(), keyword.toStdString().c_str(), mylist.size());

	//V3DLONG nfiles = mylist.size();

	//for (V3DLONG ii = 0; ii < nfiles; ++ii)
	//{
	//	QString tmpfileInfo = mylist.at(ii);
	//	bret = Utils::getTimePositionFromFilename(tmpfileInfo, v_tmp, tp_tmp, pos_tmp, g_tmp, root_tmp, prefix_tmp, basename);

	//	if (bret)
	//	{
	//		if (prefix.length() == 0)
	//			prefix = prefix_tmp;

	//		if (v_tmp < 0) {
	//			if (nfiles > 1)
	//				newfilename = root + "/" + prefix + "_t_" + std::to_string(tp_tmp) + "_p_" + std::to_string(pos_tmp) + "_g_" + std::to_string(g_tmp) + ".tiff";
	//			else
	//				newfilename = root + "/" + prefix + "_t_" + std::to_string(tp_tmp) + "_p_" + std::to_string(pos_tmp) + ".tiff";
	//		}
	//		else {
	//			if (nfiles > 1)
	//				newfilename = root + "/" + prefix + "_v_" + std::to_string(v_tmp) + "_t_" + std::to_string(tp_tmp) + "_p_" + std::to_string(pos_tmp) + "_g_" + std::to_string(g_tmp) + ".tiff";
	//			else
	//				newfilename = root + "/" + prefix + "_v_" + std::to_string(v_tmp) + "_t_" + std::to_string(tp_tmp) + "_p_" + std::to_string(pos_tmp) + ".tiff";
	//		}

	//		bret = QFile::copy(tmpfileInfo, QString::fromStdString(newfilename));
	//		if (bret)
	//		{
	//			memset(imgSrcFile, 0, sizeof(imgSrcFile));
	//			memcpy(imgSrcFile, newfilename.c_str(), newfilename.length());
	//		}
	//	}
	//}
	//[End]
	return true;
}


void Image4DSimple::loadImage(const char* filename)
{
	return this->loadImage(filename, false); //default don't use MYLib
}
//[QT5_MK]
//
//int Image4DSimple::loadImageJTif2Stack(char * filename, unsigned char * & img, V3DLONG * & sz, int & datatype, map<string, uint64> &info, bool& isImageJ)
//{
//	int b_error = 0;
//
//	size_t ct0 = clock();
//
//	try //120329
//	{
//		CImageJ_Tiff imgJ;
//
//		isImageJ = false;
//
//		if (!imgJ.getFileInfo(filename, imageJTiffinfo, isImageJ))
//		{
//			b_error = 1;
//			return b_error;
//		}
//		if (!isImageJ)
//		{
//			b_error = 1;
//			return b_error;
//		}
//
//		if (sz) { delete sz; sz = 0; }
//		if (img) { delete img; img = 0; }
//
//		sz = new V3DLONG[5];
//		if (sz)
//		{
//			sz[0] = info["IMAGEWIDTH"];  //x
//			sz[1] = info["IMAGELENGTH"];	//y
//			sz[2] = info["SLICES"];		//z
//			sz[3] = info["CHANNELS"];	//channels
//		}
//		else
//		{
//			printf("Unable to allocate memory for the size varable! Return.\n");
//			b_error = 1;
//			return b_error;
//		}
//
//		datatype = info["BITSPERSAMPLE"] / 8;
//
//		sz[4] = (datatype == 2) ? KIND_GREY16 : KIND_GREY;
//
//		img = new unsigned char[sz[0] * sz[1] * sz[2] * sz[3] * datatype];
//		if (!img)
//		{
//			printf("Unable to allocate memory for the image varable! Return.\n");
//
//			if (sz) { delete sz; sz = 0; }
//			b_error = 1;
//			return b_error;
//		}
//
//		int kind = GREY16;
//
//		kind = (datatype == 2) ? GREY16 : GREY;
//
//		if (kind == GREY16)
//		{
//			unsigned short *** imgSrc;
//			imgJ.load(filename, imageJTiffinfo, imgSrc);
//			for (uint64 t = 0; t < info["TIMEPOINTS"]; t++)
//			{
//				for (uint64 c = 0; c < info["CHANNELS"]; c++)
//				{
//					void *dst = (void *)&img[(t * info["CHANNELS"] + c) * info["SLICES"] * info["IMAGELENGTH"] * info["IMAGEWIDTH"]];
//					void *src = (void *)imgSrc[t][c];
//					memcpy(dst, src, sizeof(unsigned short)*info["SLICES"] * info["IMAGELENGTH"] * info["IMAGEWIDTH"]);
//					delete[]src;
//				} //channels
//			}
//		}
//		isSaved = true;
//		isAllUpdated = true;
//
//	}
//	catch (...)
//	{
//		v3d_error("An exception is caught for tiff file reading.\n");
//		b_error = 1;
//	}
//
//	size_t ct1 = clock();
//	v3d_debug("loadTif2Stack. total time(ms):%lu\n", ct1 - ct0);
//
//	return b_error;
//
//}
//
// This can only be used : 1). for disk/cache swap - saved and swapped statue 2). the image parameters are all corrected 3). the image state is not changed. 
// It will load the image file : _t_xx_p_xx.tiff or _t_xx_p_xx_g_xx.tiff.  g means the file# which is separated to multiple < 4G file for the same t, pos
//

int Image4DSimple::loadImage_Cache(bool& running, int pos1, int tp1)
{
	//cleanExistData(); // note that this variable must be initialized as NULL.
	//strcpy(imgSrcFile, filename);

	running_ = running;

	b_error = 0;
	if (!isImageSaved() && !isImageSwapout()) // 
	{
		b_error = 1;
		v3d_error("Image is not in saved and swapout status in My4DImageSimple::loadImage2Cache(). \n", false);
		return b_error;
	}


	const char * curFileSuffix = getSuffix(imgSrcFile);
	//printf("The current input file has the suffix [%s]\n", curFileSuffix);

	//v3d_msg("(Win32) Now try to use LIBTIFF (slightly revised by PHC) to read the TIFF/LSM...\n",0);
	if (!(strcasecmp(curFileSuffix, "tif") == 0 || strcasecmp(curFileSuffix, "tiff") == 0))
	{
		b_error = 1;
		return b_error;
	}

	QString partFilename = QString::fromStdString(std::string(imgSrcFile));

	int v_tmp, tp_tmp, pos_tmp, g_tmp;
	std::string root1, prefix1, basename1;

	bool bret = Utils::getTimePositionFromFilename(partFilename, v_tmp, tp_tmp, pos_tmp, g_tmp, root1, prefix1, basename1);

	QString keyword;
	if (v_tmp < 0) {
		keyword = QString("_t_%1_p_%2_g").arg(QString::number(tp_tmp), QString::number(pos_tmp));
	}
	else {
		keyword = QString("_v_%1_t_%2_p_%3_g").arg(QString::number(v_tmp), QString::number(tp_tmp), QString::number(pos_tmp));
	}

	QStringList mylist = Utils::getFileList(partFilename, keyword);

	if (mylist.size() <= 0) //doesn't have 4G file, //[2020/10/27]. add . to avoid the pos#>10, pos 1 will mix with 10~19. 
	{
		if (v_tmp < 0) {
			keyword = QString("_t_%1_p_%2.").arg(QString::number(tp_tmp), QString::number(pos_tmp)); 
		}
		else {
			keyword = QString("_v_%1_t_%2_p_%3.").arg(QString::number(v_tmp), QString::number(tp_tmp), QString::number(pos_tmp));
		}
		mylist = Utils::getFileList(partFilename, keyword);
	}

	v3d_debug("Image4DSimple::loadImage_Cache - filename:%s, keyword:%s, mylist.size:%d", partFilename.toStdString().c_str(), keyword.toStdString().c_str(), mylist.size());

	V3DLONG nfiles = mylist.size();
	if (nfiles < 1)
	{
		v3d_error("The import list is empty. do nothing.\n");
		running = false;
		return b_error;
	}

	ImagePixelType cur_datatype = V3D_UNKNOWN, ndatatype = V3D_UNKNOWN;
	V3DLONG ncolors = 0, nthick = 0;


	V3DLONG totalbytes = sz0*sz1*sz2*sz3*datatype;
	try { //081001
		data1d = new unsigned char[totalbytes];
	}
	catch (...) { data1d = 0; }

	if (!data1d)
	{
		v3d_error("Fail to allocate memory in My4DImageSimple::createBlankImage(). Check your program. \n", false);
		b_error = 2;
		return b_error;

	}

	ndatatype = datatype;
	ncolors = sz_channel; //

	statestack_.clear();
	int totalimages = sz2 * sz_channel * sz_pos * sz_time; // c, z, p, t

	statestack_.resize(totalimages);

	for (int ii = 0; ii < totalimages; ii++)
	{
		statestack_[ii] = Cache_Create_Blank;
	}

	memset(data1d, 0, totalbytes);

	unsigned long long destBufOffset = 0;
	unsigned long long srcBuffSize = 0;

	int imageloaded = 0;
	int cur_imageloaded = 0;

	unsigned char * cur_data1d = 0;
	V3DLONG * cur_sz = 0;

	for (V3DLONG ii = 0; ii < nfiles && running_; ++ii)
	{
		QString tmpfileInfo = mylist.at(ii);
		v3d_debug("loading %i file: {%s}\n", ii, qPrintable(tmpfileInfo));
		cur_data1d = 0;
		cur_sz = 0;
		if (!readSingleImageFile(tmpfileInfo.toUtf8().data(), cur_data1d, cur_sz, cur_datatype))
		{
			v3d_error("Error occurs in reading the file. Exit importing.\n");
			running = false;
			b_error = 1;
			goto Clear_ImageData;
		}
		if (!cur_data1d || !cur_sz)
		{
			v3d_error("Error occurs in reading the file. Exit importing.\n");
			running = false;
			b_error = 2;
			goto Clear_ImageData;
		}
		cur_imageloaded = nthick = cur_sz[2]; // z

		v3d_debug("cur_sz[0]:%d, sz0:%d, cur_sz[1]:%d sz1:%d, cur_sz[3]:%d, cur_datatype:%d,ndatatype:%d\n ", cur_sz[0], sz0, cur_sz[1], sz1, cur_sz[3], cur_datatype, ndatatype);
		if (cur_sz[0] != sz0 || cur_sz[1] != sz1 || cur_sz[3] <= 0 || cur_datatype != ndatatype)
		{
			v3d_error("Error occurs in reading the file : %s , x,y, c, data type are not consistent", tmpfileInfo.toStdString().c_str());
			running = false;
			b_error = 3;
			goto Clear_ImageData;
		}

		//now copy data of different planes into the 5D stack

		if (sz_acqorder == ACQ_Order_XYZCT || sz_acqorder == ACQ_Order_XYZCPT || sz_acqorder == ACQ_Order_XYZCPVT)
		{
			V3DLONG element_bytes = cur_datatype;  // RGB is converted 3 channels of 8 bits 
			V3DLONG cur_block_size = (nthick*sz0*sz1)*(element_bytes);

			if (destBufOffset + cur_block_size > totalbytes)
			{
				v3d_error("Error occurs in reading the file : %s, oversize", tmpfileInfo);
				running = false;
				b_error = 4;
				goto Clear_ImageData;
			}
			memcpy(data1d + destBufOffset, cur_data1d, cur_block_size);
			destBufOffset += cur_block_size;

			int jj = imageloaded;
			for (int ii = 0; ii < cur_imageloaded && jj < totalimages; ii++, jj++)
			{
				statestack_[jj] = Cache_Created_Saved;
			}

			imageloaded += cur_imageloaded;
		}
		else if (sz_acqorder == ACQ_Order_XYCZT || sz_acqorder == ACQ_Order_XYCZPT || sz_acqorder == ACQ_Order_XYCZPVT)
		{
			V3DLONG element_bytes = cur_datatype;  // RGB is converted 3 channels of 8 bits 
			V3DLONG cur_block_size = (nthick*sz0*sz1)*(element_bytes);
			V3DLONG cur_image_size = (sz0*sz1)*(element_bytes);

			if (destBufOffset + cur_block_size > totalbytes)
			{
				v3d_error("Error occurs in reading the file : %s, oversize", tmpfileInfo);

				running = false;
				b_error = 4;
				goto Clear_ImageData;
			}

			int jj = imageloaded;

			V3DLONG cur_destBuffOffset = 0;
			V3DLONG cur_srcBuffOffset = 0;

			V3DLONG cur_c = 0;
			V3DLONG cur_z = 0;
			V3DLONG cur_p = 0;
			V3DLONG cur_t = 0;

			for (int ii = 0; ii < cur_imageloaded && jj < totalimages; ii++, jj++)
			{
				statestack_[jj] = Cache_Created_Saved;

				cur_t = jj / (sz_channel*sz2*sz_pos);
				cur_p = jj / (sz_channel*sz2);
				cur_z = jj / (sz_channel);
				cur_c = jj % (sz_channel);

				V3DLONG dest_index = ((cur_t *sz_pos + cur_p)*sz_channel + cur_c)* sz2 + cur_z;  // change to XYZCT or XYZCPT in memory

				cur_destBuffOffset = dest_index * cur_image_size;

				memcpy(data1d + cur_destBuffOffset, cur_data1d + cur_srcBuffOffset, cur_image_size);

				cur_srcBuffOffset += cur_image_size;

			}

			destBufOffset += cur_block_size;

			imageloaded += cur_imageloaded;
		}

	Clear_ImageData:
		if (cur_data1d) { delete[]cur_data1d; cur_data1d = 0; }
		if (cur_sz) { delete[]cur_sz; cur_sz = 0; }

	}

	if (b_error == 0)
	{
		//[Litone]
		setCacheState(Cache_Created_Saved);
		isSaved = true;

		isAllUpdated = true;

	}
	else
	{
		setCacheState(Cache_Create_Blank);
		isSaved = false;

		isAllUpdated = false;

	}

	//end
	return b_error;
}


void Image4DSimple::loadImage(const char* filename, bool b_useMyLib)
{
	cleanExistData(); // note that this variable must be initialized as NULL.

	strcpy(imgSrcFile, filename);

	V3DLONG * tmp_sz = 0; // note that this variable must be initialized as NULL.
	int tmp_datatype = 0;
	int pixelnbits = 1; //100817

	const char * curFileSuffix = getSuffix(imgSrcFile);
	//printf("The current input file has the suffix [%s]\n", curFileSuffix);

	if (curFileSuffix && (strcasecmp(curFileSuffix, "tif") == 0 || strcasecmp(curFileSuffix, "tiff") == 0 ||
		strcasecmp(curFileSuffix, "lsm") == 0)) //read tiff/lsm stacks
	{
		//printf("Image4DSimple::loadImage loading filename=[%s]\n", filename);

#if defined _WIN32
		{
			//v3d_msg("(Win32) Now try to use LIBTIFF (slightly revised by PHC) to read the TIFF/LSM...\n",0);
			if (strcasecmp(curFileSuffix, "tif") == 0 || strcasecmp(curFileSuffix, "tiff") == 0)
			{
				//[Litone] 2018/9/6
				bool isImageJ, is4Gplus;

				//b_error = isLargeImageJTif(imgSrcFile, isImageJ, is4Gplus);

				//[Jian] remove iMageJ specific format tiff load
				//if (b_error==0 && isImageJ ) // 
				//{


				//	if (loadImageJTif2Stack(imgSrcFile, data1d, tmp_sz, tmp_datatype, imageJTiffinfo, isImageJ))
				//	{
				//		v3d_error("Error happens in TIF file reading (using libtiff). \n", false);
				//		b_error = 1;
				//	}
				//	else
				//	{
				//		sz_kind = tmp_sz[4]; //[Litone] return the kind type
				//		isImageJTiff = true; 
				//	}
				//}				
				//else 
				if (loadLitoneTif2Stack(imgSrcFile, data1d, tmp_sz, tmp_datatype, running_))
					//end
					//if (loadTif2Stack_from_camera(imgSrcFile, data1d, tmp_sz, tmp_datatype))
				{
					v3d_error("Error happens in TIF file reading (using libtiff). \n", false);
					b_error = 1;

				}
				else
				{
					sz_kind = tmp_sz[4]; //[Litone] return the kind type
				}
			}
			else //if ( strcasecmp(curFileSuffix, "lsm")==0 ) //read lsm stacks
			{
				if (loadLsm2Stack(imgSrcFile, data1d, tmp_sz, tmp_datatype))
				{
					v3d_msg("Error happens in LSM file reading (using libtiff, slightly revised by PHC). \n", false);
					b_error = 1;
				}
			}
		}

#else
		if (b_useMyLib)
		{
			v3d_msg("Now try to use MYLIB to read the TIFF/LSM again...\n", 0);
			if (loadTif2StackMylib(imgSrcFile, data1d, tmp_sz, tmp_datatype, pixelnbits))
			{
				v3d_msg("Error happens in TIF/LSM file reading (using MYLIB). Stop. \n", false);
				b_error = 1;
				return;
			}
			else
				b_error = 0; //when succeed then reset b_error
		}
		else
		{
			//v3d_msg("Now try to use LIBTIFF (slightly revised by PHC) to read the TIFF/LSM...\n",0);
			if (strcasecmp(curFileSuffix, "tif") == 0 || strcasecmp(curFileSuffix, "tiff") == 0)
			{
				if (loadTif2Stack(imgSrcFile, data1d, tmp_sz, tmp_datatype))
				{
					v3d_msg("Error happens in TIF file reading (using libtiff). \n", false);
					b_error = 1;
				}
			}
			else //if ( strcasecmp(curFileSuffix, "lsm")==0 ) //read lsm stacks
			{
				if (loadLsm2Stack(imgSrcFile, data1d, tmp_sz, tmp_datatype))
				{
					v3d_msg("Error happens in LSM file reading (using libtiff, slightly revised by PHC). \n", false);
					b_error = 1;
				}
			}
		}
		// printf("Image4DSimple::loadImage finished\n");

#endif

	}

	//else if (curFileSuffix && (strcasecmp(curFileSuffix, "nrrd") == 0 || strcasecmp(curFileSuffix, "nhdr") == 0)) //read nrrd stacks
	//{
	//	// printf("Image4DSimple::loadImage loading filename=[%s]\n", filename);
	//	float pxinfo[4];
	//	float spaceorigin[3];
	//	if (!read_nrrd_with_pxinfo(imgSrcFile, data1d, tmp_sz, tmp_datatype, pxinfo, spaceorigin))
	//	{
	//		v3d_msg("Error happens in NRRD file reading. Stop. \n", false);
	//		b_error = 1;
	//		return;
	//	}
	//	else
	//	{   //copy the pixel size and origin info when the nrrd read is successful
	//		//note that pixinfo[3] is reserved for future extension to include the temporal sampling rate. It is not used right now.
	//		rez_x = pxinfo[0];
	//		rez_y = pxinfo[1];
	//		rez_z = pxinfo[2];

	//		origin_x = spaceorigin[0];
	//		origin_y = spaceorigin[1];
	//		origin_z = spaceorigin[2];
	//	}
	//}

	else if (curFileSuffix && strcasecmp(curFileSuffix, "mrc") == 0) //read mrc stacks
	{
		if (loadMRC2Stack(imgSrcFile, data1d, tmp_sz, tmp_datatype))
		{
			v3d_msg("Error happens in MRC file reading. Stop. \n", false);
			b_error = 1;
			return;
		}
	}
#ifdef _ALLOW_WORKMODE_MENU_
	//else if ( curFileSuffix && ImageLoader::hasPbdExtension(QString(filename)) ) // read v3dpbd - pack-bit-difference encoding for sparse stacks
	else if (curFileSuffix && strcasecmp(curFileSuffix, "v3dpbd") == 0) // read v3dpbd - pack-bit-difference encoding for sparse stacks
	{
		v3d_msg("start to try v3dpbd", 0);
		ImageLoaderBasic imageLoader;
		if (!imageLoader.loadRaw2StackPBD(imgSrcFile, this, false) == 0) {
			v3d_msg("Error happens in v3dpbd file reading. Stop. \n", false);
			b_error = 1;
			return;
		}
		// The following few lines are to avoid disturbing the existing code below
		tmp_datatype = this->getDatatype();
		tmp_sz = new V3DLONG[4];
		tmp_sz[0] = this->getXDim();
		tmp_sz[1] = this->getYDim();
		tmp_sz[2] = this->getZDim();
		tmp_sz[3] = this->getCDim();

		this->setFileName(filename); // PHC added 20121213 to fix a bug in the PDB reading.
	}
#endif
	else //then assume it is Hanchuan's Vaa3D RAW format
	{
		v3d_msg("The data does not have supported image file suffix, -- now this program assumes it is Vaa3D's RAW format and tries to load it... \n", false);
		if (loadRaw2Stack(imgSrcFile, data1d, tmp_sz, tmp_datatype))
		{
			printf("The data doesn't look like a correct 4-byte-size Vaa3D's RAW file. Try 2-byte-raw. \n");
			if (loadRaw2Stack_2byte(imgSrcFile, data1d, tmp_sz, tmp_datatype))
			{
				v3d_msg("Error happens in reading 4-byte-size and 2-byte-size Vaa3D's RAW file. Stop. \n", false);
				b_error = 1;
				return;
			}
		}
	}

	//080302: now convert any 16 bit or float data to the range of 0-255 (i.e. 8bit)
	switch (tmp_datatype)
	{
	case 1:
		datatype = V3D_UINT8;
		break;

	case 2: //080824
		//convert_data_to_8bit((void *&)data1d, tmp_sz, tmp_datatype);
		//datatype = UINT8; //UINT16;
		datatype = V3D_UINT16;
		break;

	case 4:
		//convert_data_to_8bit((void *&)data1d, tmp_sz, tmp_datatype);
		datatype = V3D_FLOAT32; //FLOAT32;
		break;

	default:
		v3d_error("The data type is not UINT8, UINT16 or FLOAT32. Something wrong with the program, -- should NOT display this message at all. Check your program. \n", false);
		if (tmp_sz) { delete[]tmp_sz; tmp_sz = 0; }
		return;
	}

	sz0 = tmp_sz[0];
	sz1 = tmp_sz[1];
	sz2 = tmp_sz[2];
	sz3 = tmp_sz[3]; //no longer merge the 3rd and 4th dimensions

	/* clean all workspace variables */

	if (tmp_sz) { delete[]tmp_sz; tmp_sz = 0; }

	//[Litone]
	setCacheState(Cache_Created_Saved);
	isSaved = true;

	isAllUpdated = true;

	//end
	return;
}

void Image4DSimple::loadImage_slice(char filename[], bool b_useMyLib, V3DLONG zsliceno)
{
	cleanExistData(); // note that this variable must be initialized as NULL.

	strcpy(imgSrcFile, filename);

	V3DLONG * tmp_sz = 0; // note that this variable must be initialized as NULL.
	int tmp_datatype = 0;
	int pixelnbits = 1; //100817

	const char * curFileSuffix = getSuffix(imgSrcFile);
	printf("The current input file has the suffix [%s]\n", curFileSuffix);

	if (curFileSuffix && (strcasecmp(curFileSuffix, "tif") == 0 || strcasecmp(curFileSuffix, "tiff") == 0 ||
		strcasecmp(curFileSuffix, "lsm") == 0)) //read tiff/lsm stacks
	{
		// printf("Image4DSimple::loadImage loading filename=[%s] slice =[%ld]\n", filename, zsliceno);

#if defined _WIN32
		{
			//v3d_msg("(Win32) Now try to use LIBTIFF (slightly revised by PHC) to read the TIFF/LSM...\n",0);
			if (strcasecmp(curFileSuffix, "tif") == 0 || strcasecmp(curFileSuffix, "tiff") == 0)
			{
				if (loadTifSlice(imgSrcFile, data1d, tmp_sz, tmp_datatype, zsliceno, false))
				{
					v3d_msg("Error happens in TIF file reading (using libtiff). \n", false);
					b_error = 1;
				}
			}
			else //if ( strcasecmp(curFileSuffix, "lsm")==0 ) //read lsm stacks
			{
				if (loadLsmSlice(imgSrcFile, data1d, tmp_sz, tmp_datatype, zsliceno, false))
				{
					v3d_msg("Error happens in LSM file reading (using libtiff, slightly revised by PHC). \n", false);
					b_error = 1;
				}
			}
		}

#else
		if (b_useMyLib)
		{
			if (loadTif2StackMylib_slice(imgSrcFile, data1d, tmp_sz, tmp_datatype, pixelnbits, zsliceno))
			{
				v3d_msg("Error happens in TIF/LSM file reading (using MYLIB). Stop. \n", false);
				b_error = 1;
				return;
			}
			else
				b_error = 0; //when succeed then reset b_error
		}
		else
		{
			//v3d_msg("Now try to use LIBTIFF (slightly revised by PHC) to read the TIFF/LSM...\n",0);
			if (strcasecmp(curFileSuffix, "tif") == 0 || strcasecmp(curFileSuffix, "tiff") == 0)
			{
				if (loadTifSlice(imgSrcFile, data1d, tmp_sz, tmp_datatype, zsliceno, false))
				{
					v3d_msg("Error happens in TIF file reading (using libtiff). \n", false);
					b_error = 1;
				}
			}
			else //if ( strcasecmp(curFileSuffix, "lsm")==0 ) //read lsm stacks
			{
				if (loadLsmSlice(imgSrcFile, data1d, tmp_sz, tmp_datatype, zsliceno, false))
				{
					v3d_msg("Error happens in LSM file reading (using libtiff, slightly revised by PHC). \n", false);
					b_error = 1;
				}
			}
		}
		// printf("Image4DSimple::loadImage finished\n");

#endif

	}

	/*
	 else if ( curFileSuffix && strcasecmp(curFileSuffix, "mrc")==0 ) //read mrc stacks
	 {
		 if (loadMRC2Stack_slice(imgSrcFile, data1d, tmp_sz, tmp_datatype, layer))
		 {
			 v3d_msg("Error happens in MRC file reading. Stop. \n", false);
			 b_error=1;
			 return;
		 }
	 }
 #ifdef _ALLOW_WORKMODE_MENU_
	 //else if ( curFileSuffix && ImageLoader::hasPbdExtension(QString(filename)) ) // read v3dpbd - pack-bit-difference encoding for sparse stacks
	 else if ( curFileSuffix && strcasecmp(curFileSuffix, "v3dpbd")==0 ) // read v3dpbd - pack-bit-difference encoding for sparse stacks
	 {
		   v3d_msg("start to try v3dpbd", 0);
		 ImageLoaderBasic imageLoader;
		 if (! imageLoader.loadRaw2StackPBD(imgSrcFile, this, false) == 0) {
			 v3d_msg("Error happens in v3dpbd file reading. Stop. \n", false);
			 b_error=1;
			 return;
		 }
		 // The following few lines are to avoid disturbing the existing code below
		 tmp_datatype=this->getDatatype();
		 tmp_sz=new V3DLONG[4];
		 tmp_sz[0]=this->getXDim();
		 tmp_sz[1]=this->getYDim();
		 tmp_sz[2]=this->getZDim();
		 tmp_sz[3]=this->getCDim();

		 this->setFileName(filename); // PHC added 20121213 to fix a bug in the PDB reading.
	 }
 #endif
	 else //then assume it is Hanchuan's Vaa3D RAW format
	 {
		 v3d_msg("The data does not have supported image file suffix, -- now this program assumes it is Vaa3D's RAW format and tries to load it... \n", false);
		 if (loadRaw2Stack_slice(imgSrcFile, data1d, tmp_sz, tmp_datatype, layer))
		 {
			 printf("The data doesn't look like a correct 4-byte-size Vaa3D's RAW file. Try 2-byte-raw. \n");
			 if (loadRaw2Stack_2byte_slice(imgSrcFile, data1d, tmp_sz, tmp_datatype, layer))
			 {
				 v3d_msg("Error happens in reading 4-byte-size and 2-byte-size Vaa3D's RAW file. Stop. \n", false);
				 b_error=1;
				 return;
			 }
		 }
	 }

	 */

	 //Temporarily do nothing to read other single slice from other formats
	else
	{
		v3d_msg("The single slice reading function is NOT available for other format at this moment.");
		b_error = 1;
		return;
	}

	//080302: now convert any 16 bit or float data to the range of 0-255 (i.e. 8bit)
	switch (tmp_datatype)
	{
	case 1:
		datatype = V3D_UINT8;
		break;

	case 2: //080824
		//convert_data_to_8bit((void *&)data1d, tmp_sz, tmp_datatype);
		//datatype = UINT8; //UINT16;
		datatype = V3D_UINT16;
		break;

	case 4:
		//convert_data_to_8bit((void *&)data1d, tmp_sz, tmp_datatype);
		datatype = V3D_FLOAT32; //FLOAT32;
		break;

	default:
		v3d_msg("The data type is not UINT8, UINT16 or FLOAT32. Something wrong with the program, -- should NOT display this message at all. Check your program. \n");
		if (tmp_sz) { delete[]tmp_sz; tmp_sz = 0; }
		return;
	}

	sz0 = tmp_sz[0];
	sz1 = tmp_sz[1];
	sz2 = tmp_sz[2];
	sz3 = tmp_sz[3]; //no longer merge the 3rd and 4th dimensions

	/* clean all workspace variables */

	if (tmp_sz) { delete[]tmp_sz; tmp_sz = 0; }

	return;
}


bool Image4DSimple::saveImage(const char filename[])
{
	if (!data1d || !filename){
		v3d_error("This image data is empty or the file name is invalid. Nothing done.\n");
		return false;
	}

	V3DLONG mysz[9];
	mysz[0] = sz0;
	mysz[1] = sz1;
	mysz[2] = sz2;
	mysz[3] = sz3;
	mysz[4] = sz_kind;
	mysz[5] = sz_channel;
	mysz[6] = sz_pos;
	mysz[7] = sz_time;
	mysz[8] = sz_acqorder;


	//[Litone]. add kind dimension, 

	int dt;
	switch (datatype)
	{
	case V3D_UINT8:  dt = 1; break;
	case V3D_UINT16:  dt = 2; break;
	case V3D_FLOAT32:  dt = 4; break;
	default:
		v3d_error("The data type is unsupported. Nothing done.\n");
		return false;
		break;
	}

	//if (strlen(filename) > 5) {
	//	const char * suffix = getSuffix((char *)filename);
	//	if (suffix && (strcasecmp(suffix, "nrrd") == 0 ||
	//		strcasecmp(suffix, "nhdr") == 0))
	//	{
	//		float pxinfo[4];
	//		float spaceorigin[3];
	//		// use nrrd_write
	//		pxinfo[0] = rez_x;
	//		pxinfo[1] = rez_y;
	//		pxinfo[2] = rez_z;

	//		spaceorigin[0] = origin_x;
	//		spaceorigin[1] = origin_y;
	//		spaceorigin[2] = origin_z;
	//		return write_nrrd_with_pxinfo(filename, data1d, mysz, dt, pxinfo, spaceorigin);
	//	}
	//}
	//[Litone]
   //return ::saveImage(filename, data1d, mysz, dt);

	bool ret = false;

	//[Todo] debug only
	//isImageJTiff = true; 
	//end
	V3DLONG imagesize = sz0*sz1*sz2*sz3*dt;  //x, y, z, c/pt
	V3DLONG imagesize_limit_4G = 4293918720L; //4294967296L - 1048576L = 4GB-1MB(header);

	//if (isImageJTiff || imagesize > imagesize_limit_4G)
	//[ToDo] 2019/4/12 for testing only
	/*if (isImageJTiff )
	{
		ret = ::saveImageJ_litone_tiff(filename, data1d, mysz, dt, 9);
	}
	else
	{
		ret = ::saveImage_litone_tiff(filename, data1d, mysz, dt, 9);

	}*/
	//const char filename[], const unsigned char * data1d, const V3DLONG * sz, const int datatype, map<string, uint64> &info, int szBuflen, bool& running, int& fileCount
	//map<string, double> info; 
	//bool running = true; 
	int  fileCount = 0;



	int retCode = saveImage_litone_4G_tiff(filename, data1d, mysz, dt, _omeTiffInfo, 9, running_, fileCount);

	//[End]

	if (retCode == 0)
	{
		setCacheState(Cache_Created_Saved);
		isSaved = true;
		savedFilecount_ = fileCount;
		ret = true;
	}

	return ret;

}


bool Image4DSimple::saveImage()
{
	bool ret = saveImage(imgSrcFile);

	return ret;

}
bool Image4DSimple::saveImage(bool& running)
{
	running_ = running;

	bool ret = saveImage(imgSrcFile);

	return ret;

}

bool Image4DSimple::loadImage()
{
	char tmp_imgSrcFile[1024];

	memset(tmp_imgSrcFile, 0, sizeof(tmp_imgSrcFile));
	strcpy(tmp_imgSrcFile, imgSrcFile);

	loadImage(tmp_imgSrcFile);  // can't use imgSrcFile directly because it will be clean in cleanExistData(); 

	return true;
}

void Image4DSimple::enableSaveLoad(bool running)
{
	running_ = running;

}

