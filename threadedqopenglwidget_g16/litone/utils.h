#ifndef UTILS_H
#define UTILS_H

#if defined(USE_Qt5_VS2015_Win7_81) || defined(USE_Qt5_VS2015_Win10_10_14393) 
  #include <QtWidgets>
#else
  #include <QtGui>
#endif
//#include <QRegExp>
#include <QRegularExpression>
//#include <QtCore5Compat/qregexp.h>
//#include "view_summary.h"
//#include "mda_summary.h"
#include "v3d_basicdatatype.h"

using RegExp = QRegExp;

#define GB (1024*1024*1024)
#define MB (1024*1024)

enum
{
	SPACE_OK = 0,  
	DISK_Not_Enough = 1,
	MEM_TP_Exceed_4G = 2,
	MEM_Not_Enough = 3
};

class ImageParam;

class Utils
{
public:
	Utils();
	
	static QString formatDisplayint(std::string & value);
	static QString formatDisplayFloat(std::string & value, int precise);
	static QString elapsedTimeDisplayString(double seconds);
	static void sleep(long  time);
	static std::string newGUID();
	static std::vector<std::string>  splitstr(const std::string& str, const std::string& pattern);
	static bool getMaxSubstr(const QString& str1, const QString & str2, QString& substr);
	static bool getMaxSubstr(const QStringList& list, QString& substr);

	static double angle_to_radian(double degree, double min, double second);
	static void radian_to_angle(double rad, double ang[]);
	static std::string doubleToString(const double &val);
	static std::string formatJson(std::string json);
	static std::string getLevelStr(int level);

	static int getTimePointFromFileName(QString fileName);
	static int getPositionFromFileName(QString fileName);

	static RegExp INT_REG_EXP;
	static RegExp DOUBLE_REG_EXP;

	static QFont YAHEI4;
	static QFont YAHEI6;
	static QFont YAHEI8;
	static QFont YAHEI10;
	static QFont YAHEI12;
	static QFont YAHEI14;
	static int phyThreads;

	static bool deleteDir(const QString &path); 
	static bool copyDirectoryFiles(const QString &fromDir, const QString &toDir, bool overwrite);
	static bool deleteFilesUnderDir(const QString &path); 

	static bool deskewHeight(int zslices, float zstep, float pixelsize, int& increaseHeight);

	//static int LoadPSFTIFF(const char *file, float *&buf, uint64 &frames, uint64 &height, uint64 &width);

	// memory, disk check


//	static std::string getAvailableNextFileName(const std::string root, std::string & projectName) throw (CMMError);

	static bool getTimePositionFromFilename(const QString & filename, int& v, int& tp, int& pos, int& g, std::string& root, std::string& prefix, std::string& basenameparsed);
	static QStringList getFileList(const QString & individualFileName, const QString& keyword);

	static void getNumberOfThreads(int& nthreads);
};


#endif // UTILS_H
