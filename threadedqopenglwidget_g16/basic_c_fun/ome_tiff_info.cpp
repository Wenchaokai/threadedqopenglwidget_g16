#include "ome_tiff_info.h"

OME_TIFFINFO::OME_TIFFINFO()
{
	pixelSize_xy_um = 0.108; 
	spacing_z_um = 0.25; 
}

OME_TIFFINFO::OME_TIFFINFO(const OME_TIFFINFO & info)
{
	this->pixelSize_xy_um = info.pixelSize_xy_um;
	this->spacing_z_um = info.spacing_z_um;
}

OME_TIFFINFO & OME_TIFFINFO::operator=(const OME_TIFFINFO & info)
{
	// TODO: insert return statement here
	this->pixelSize_xy_um = info.pixelSize_xy_um;
	this->spacing_z_um = info.spacing_z_um;
	return *this;
}
