#ifndef _OME_TIFFINFO_H_
#define _OME_TIFFINFO_H_
class OME_TIFFINFO
{
public: 
	OME_TIFFINFO(); 
	OME_TIFFINFO(const OME_TIFFINFO & info);
	OME_TIFFINFO & operator=(const OME_TIFFINFO& info);

	double pixelSize_xy_um;
	double spacing_z_um;
	
};

#endif