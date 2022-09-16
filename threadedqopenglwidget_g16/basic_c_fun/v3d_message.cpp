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




 //by Hanchuan Peng
 //2009-2011

#include "v3d_message.h"
#include <stdio.h>

#if defined(USE_Qt5_VS2015_Win7_81) || defined(USE_Qt5_VS2015_Win10_10_14393)
  #include <QtWidgets>
#else
  #include <QtGui>
#endif
#include <QString>
#include <QMessageBox>

//#include "../litone/litone_logger.h"

void v3d_info(const char *format, ...) {
	
	char buf[4096];

	va_list ap;
	va_start(ap, format);
	int len = vsprintf(buf, format, ap);
	va_end(ap);

	std::string value(buf);

	v3d_msg_info(value.c_str());

}

void v3d_error(const char *format, ...) {

	char buf[4096];

	va_list ap;
	va_start(ap, format);
	int len = vsprintf(buf, format, ap);
	va_end(ap);

	std::string value(buf);

	v3d_msg_error(value.c_str());

}

void v3d_debug(const char *format, ...) {

	char buf[4096];

	va_list ap;
	va_start(ap, format);
	int len = vsprintf(buf, format, ap);
	va_end(ap);

	std::string value(buf);

	v3d_msg_debug(value.c_str());

}

void v3d_msg_debug(const char * msg)
{
	QString text = QString::fromStdString(msg);
	text.replace(QString("\n"), QString(""));
	printf("%s\n", text.toStdString().c_str()); 
	//LitoneLogger::loggerDebug(text);
}

void v3d_msg_error(const char * msg)
{
	QString text = QString::fromStdString(msg);
	text.replace(QString("\n"), QString(""));
	printf("%s\n", text.toStdString().c_str());
	//LitoneLogger::loggerError(text);
}

void v3d_msg_info(const char * msg)
{
	QString text = QString::fromStdString(msg);
	text.replace(QString("\n"), QString(""));
	printf("%s\n", text.toStdString().c_str());
//	LitoneLogger::loggerInfo(text);
}

void v3d_msg(const char *msg, bool b_disp_QTDialog)
{
	// The below instance check is to make v3d_msg compatible with command-line mode
	if (!b_disp_QTDialog) {
		v3d_msg_info(msg);
	}
	else if (QCoreApplication::instance() != 0 && b_disp_QTDialog) {
		QMessageBox::information(0, "Information", msg);
	}
}

void v3d_msg(const QString & msg, bool b_disp_QTDialog) //note that if I don't force (char *) conversion then there is a crash. noted by Hanchuan, 090516
{
	v3d_msg((char *)(qPrintable(msg)), b_disp_QTDialog);
}

QString current_time_stamp()
{
	return QString(__TIME__) + " " + QString(__DATE__);
}





