#include "utils.h"

RegExp Utils::INT_REG_EXP("^([0-9][0-9]{0,3}|[1-5][0-9]{0,4}|[1-6][0-4][0-9]{0,3}|[1-6][0-5][0-4][0-9]{0,2}|[1-6][0-5][0-5][0-2][0-9]{0,1}|[1-6][0-5][0-5][0-3][0-5])$|(^\\t?$)");
RegExp Utils::DOUBLE_REG_EXP("^(-?[0]|-?[1-9][0-9]{0,5})(?:\\.\\d{1,4})?$|(^\\t?$)");
QFont Utils::YAHEI4("Microsoft YaHei", 4);
QFont Utils::YAHEI6("Time New Roman", 6);
QFont Utils::YAHEI8("Time New Roman", 8);
QFont Utils::YAHEI10("Time New Roman", 10);
QFont Utils::YAHEI12("Microsoft YaHei", 12);
QFont Utils::YAHEI14("Microsoft YaHei", 14);



#include <boost/property_tree/ptree.hpp>  
#include <boost/property_tree/json_parser.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <iostream>
#include <fstream>

using namespace boost::property_tree;
using namespace std;


int Utils::phyThreads = -1;

Utils::Utils()
{
}

QString Utils::formatDisplayint(std::string & value)
{
	int num = atoi(value.c_str());
	QString s = QString::number(num, 10);
	return s;
}

QString Utils::formatDisplayFloat(std::string & value, int precise)
{
	double num = QString::fromStdString(value).toDouble();
	QString s = QString::number(num, 'f', precise);
	return s;
}

QString Utils::elapsedTimeDisplayString(double seconds)
{
	long wholeSeconds = (long)std::floor(seconds);
	double fraction = seconds - wholeSeconds;

	long hours = (long)(wholeSeconds / 3600);
	wholeSeconds -= hours * 3600;
	QString hoursString("");
	if (hours > 0) {
		hoursString = QString::number(hours, 'f', 0) + "h ";
	}

	long minutes = (long)(wholeSeconds / 60);
	wholeSeconds -= minutes * 60;
	QString minutesString("");
	if (hours > 0 || minutes > 0) {
		minutesString = QString::number(minutes, 'f', 0) + "m ";
	}

	QString secondsString("");
	if (fraction > 0.01) {
		secondsString = QString::number(wholeSeconds + fraction, 'f', 2) + "s";
	}
	else {
		secondsString = QString::number(wholeSeconds + fraction, 'f', 0) + "s";
	}

	return hoursString + minutesString + secondsString;
}


void Utils::sleep(long time)
{
	QEventLoop loop;
	QTimer::singleShot(time, &loop, SLOT(quit()));
	loop.exec();
}

std::string Utils::newGUID()
{
	boost::uuids::uuid a_uuid = boost::uuids::random_generator()();
	const string tmp_uuid = boost::uuids::to_string(a_uuid);
	return tmp_uuid;
}

double Utils::angle_to_radian(double degree, double min, double second)
{
	double flag = (degree < 0) ? -1.0 : 1.0;
	if (degree < 0) {
		degree = degree * (-1.0);
	}
	double angle = degree + min / 60 + second / 3600;
	double result = flag * (angle * M_PI) / 180;
	return result;
}

void Utils::radian_to_angle(double rad, double ang[])
{
	double flag = (rad < 0) ? -1.0 : 1.0;
	if (rad < 0)
	{
		rad = rad * (-1.0);
	}
	double result = (rad * 180) / M_PI;
	double degree = int(result);
	double min = (result - degree) * 60;
	double second = (min - int(min)) * 60;
	ang[0] = flag * degree;
	ang[1] = int(min);
	ang[2] = second;
}

std::string Utils::doubleToString(const double & val)
{
	char* chCode;
	chCode = new char[20];
	sprintf(chCode, "%.4lf", val);
	std::string str(chCode);
	delete[]chCode;
	return str;
}

string Utils::formatJson(string json)
{
	string result = "";
	int level = 0;
	for (string::size_type index = 0; index < json.size(); index++)
	{
		char c = json[index];

		if (level > 0 && '\n' == json[json.size() - 1])
		{
			result += getLevelStr(level);
		}

		switch (c)
		{
		case '{':
		case '[':
			result = result + c + "\n";
			level++;
			result += getLevelStr(level);
			break;
		case ',':
			result = result + c + "\n";
			result += getLevelStr(level);
			break;
		case '}':
		case ']':
			result += "\n";
			level--;
			result += getLevelStr(level);
			result += c;
			break;
		default:
			result += c;
			break;
		}

	}
	return result;
}

string Utils::getLevelStr(int level)
{
	string levelStr = "";
	for (int i = 0; i < level; i++)
	{
		levelStr += "\t";
	}
	return levelStr;
}

int Utils::getTimePointFromFileName(QString fileName)
{
	int ti = fileName.indexOf("_t_");
	if (ti > 0) {
		int pi = fileName.indexOf("_p_");
		if (pi <= 0)
			pi = fileName.indexOf("_g_");
		if (pi <= 0)
			pi = fileName.indexOf(".tif");
		if (pi > 0) {
			int tp = fileName.mid(ti + 3, pi - ti - 3).toInt();
			return tp;
		}
	}

	return 0;
}

int Utils::getPositionFromFileName(QString fileName)
{
	int pi = fileName.indexOf("_p_");
	if (pi > 0) {
		int ei = fileName.indexOf("_g_");
		if (ei <= 0)
			ei = fileName.indexOf(".tif");

		if (ei > 0) {
			int pp = fileName.mid(pi + 3, ei - pi - 3).toInt();
			return pp;
		}
	}

	return 0;
}

bool Utils::deleteFilesUnderDir(const QString & path)
{

	if (path.isEmpty()) {
		return false;
	}
	QDir dir(path);
	if (!dir.exists()) {
		return true;
	}
	dir.setFilter(QDir::AllEntries | QDir::NoDotAndDotDot);
	QFileInfoList fileList = dir.entryInfoList();
	foreach(QFileInfo file, fileList) {
		if (file.isFile()) {
			file.dir().remove(file.fileName());
		}
		else {
			deleteDir(file.absoluteFilePath());
		}
	}

	return true;


}

bool Utils::deleteDir(const QString & path)
{
	if (path.isEmpty()) {
		return false;
	}
	QDir dir(path);
	if (!dir.exists()) {
		return true;
	}
	dir.setFilter(QDir::AllEntries | QDir::NoDotAndDotDot);
	QFileInfoList fileList = dir.entryInfoList();
	foreach(QFileInfo file, fileList) {
		if (file.isFile()) {
			file.dir().remove(file.fileName());
		}
		else {
			deleteDir(file.absoluteFilePath());
		}
	}
	return dir.rmpath(dir.absolutePath());

}

bool Utils::copyDirectoryFiles(const QString & fromDir, const QString & toDir, bool overwrite)
{

	QDir sourceDir(fromDir);
	QDir targetDir(toDir);

	if (!targetDir.exists()) {    //create if not exist
		if (!targetDir.mkdir(targetDir.absolutePath()))
			return false;
	}

	QFileInfoList fileInfoList = sourceDir.entryInfoList();
	foreach(QFileInfo fileInfo, fileInfoList) {
		if (fileInfo.fileName() == "." || fileInfo.fileName() == "..")
			continue;

		if (fileInfo.isDir()) {    //
			if (!copyDirectoryFiles(fileInfo.filePath(),
				targetDir.filePath(fileInfo.fileName()),
				overwrite))
				return false;
		}
		else {            /** */
			if (overwrite && targetDir.exists(fileInfo.fileName())) {
				targetDir.remove(fileInfo.fileName());
			}
			else if (!overwrite && targetDir.exists(fileInfo.fileName())) {
				continue;
			}

			/// copy
			if (!QFile::copy(fileInfo.filePath(),
				targetDir.filePath(fileInfo.fileName()))) {
				return false;
			}
		}
	}
	return true;

}

bool Utils::deskewHeight(int zslices, float zstep, float pixelsize, int& increaseHeight)
{
	float deskewScale = 1.0;
	double yShift = zstep*cos(Utils::angle_to_radian(31, 0, 0)) * deskewScale / pixelsize;
	increaseHeight = (int)(ceil(zslices*yShift));

	return true;
}


//std::string Utils::getAvailableNextFileName(const std::string root, std::string & projectName) throw(CMMError)
//{
//	for (int index = 0; ; index++) {
//		if (index > 0) {
//			projectName = projectName + "_" + std::to_string(index);
//		}
//
//		std::string result = root + "//" + projectName;
//
//
//		QDir root(QString::fromStdString(result));
//		if (!root.exists())
//			return result;
//	}
//}

QStringList Utils::getFileList(const QString & individualFileName, const QString& keyword)
{
	QStringList myList;
	myList.clear();

	//Get the image files namelist in the directory

	QFileInfo fileInfo(individualFileName);
	QString curFilePath = fileInfo.path();
	QString curSuffix = fileInfo.suffix();

	QDir dir(curFilePath);
	if (!dir.exists()) {
		qWarning("Cannot find the directory");
		return myList;
	}

	QStringList imgfilters;

	imgfilters.append("*." + curSuffix);


	foreach(QString file, dir.entryList(imgfilters, QDir::Files, QDir::Name)) {
		//
		// avoid to load the file >4GB. Jian 2019/4/5 

		if (file.indexOf(keyword) >= 0)
			myList.append(curFilePath + "/" + file);

	}

	myList.sort();

	return myList;

}


bool Utils::getTimePositionFromFilename(const QString & filename, int& v, int& tp, int& pos, int& g, std::string& root, std::string& prefix, std::string& basenameparsed)
{

	QFileInfo fileInfo(filename);

	QString basename = fileInfo.completeBaseName();

	root = fileInfo.path().toStdString();
	basenameparsed = basename.toStdString();

	RegExp r("(\\d+)");		//find digits

	QString fileNameDigits;

	v = tp = pos = g = -1;

	V3DLONG posStr = 0;

	bool prefixFlag = false;

	while ((posStr = r.indexIn(basename, posStr)) != -1)
	{
		if (posStr >= 3)
		{
			QString indictStr = basename.mid(posStr - 3, 3);

			fileNameDigits = r.cap(1);
			V3DLONG num_tmp = fileNameDigits.toULong();

			if (indictStr.compare("_v_") == 0) {
				v = num_tmp;

				prefix = basename.left(posStr - 3).toStdString();

				prefixFlag = true;
			}
			else if (indictStr.compare("_t_") == 0)
			{
				tp = num_tmp;

				if (!prefixFlag)
					prefix = basename.left(posStr - 3).toStdString();
			}
			else if (indictStr.compare("_p_") == 0)
			{
				pos = num_tmp;
			}
			else if (indictStr.compare("_g_") == 0)
			{
				g = num_tmp;
			}

		}

		posStr += r.matchedLength();
	}

	return true;

}

std::vector<std::string>  Utils::splitstr(const std::string& str, const std::string& pattern)
{
	std::vector<std::string>  li;
	std::string subStr;
	std::string tPattern;
	size_t      patternLen = pattern.length();

	for (size_t i = 0; i < str.length(); i++)
	{
		if (pattern[0] == str[i])
		{
			tPattern = str.substr(i, patternLen);
			if (tPattern == pattern)
			{
				i += patternLen - 1;
				if (!subStr.empty())
				{
					li.push_back(subStr);
					subStr.clear();
				}
			}
			else
			{
				subStr.push_back(str[i]);
			}
		}
		else
		{
			subStr.push_back(str[i]);
		}
	}

	if (!subStr.empty())
	{
		li.push_back(subStr);
	}

	return li;
}

bool Utils::getMaxSubstr(const QString& str1, const QString & str2, QString& substr)
{
	int ii = 0;
	for (; ii < str1.length() && ii < str2.length(); ii++)
	{
		if (str1.at(ii) != str2.at(ii))
		{
			break;
		}
	}
	substr = str1.left(ii);
	return true;

}

bool Utils::getMaxSubstr(const QStringList & list, QString & substr)
{
	if (list.size() < 1)
		return false;

	substr = list.at(0);

	for (int ii = 1; ii < list.size(); ii++)
	{
		QString str2 = list.at(ii);
		getMaxSubstr(substr, str2, substr);
	}

	return true;
}

//
// run one time - to mitigate the system call will slow when cpu is busy
//
void Utils::getNumberOfThreads(int& nthreads)
{
	if (phyThreads <= 0)
	{
		SYSTEM_INFO si;
		GetSystemInfo(&si);
		phyThreads = nthreads = si.dwNumberOfProcessors;
	}
	else
	{
		nthreads = phyThreads;
	}
}