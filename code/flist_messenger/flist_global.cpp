#include "flist_global.h"
#include <QApplication>
#include <QSettings>
#include <QByteArray>
#include <iostream>
#include "flist_parser.h"
#include "flist_settings.h"

QNetworkAccessManager *networkaccessmanager = 0;
BBCodeParser *bbcodeparser = 0;
QString settingsfile;
QString logpath;
FSettings *settings = 0;

void debugMessage(QString str) {
	std::cout << str.toUtf8().data() << std::endl;
}

void debugMessage(std::string str) {
	std::cout << str << std::endl;
}

void debugMessage(const char *str) {
	std::cout << str << std::endl;
}

void globalInit()
{
	//todo: parse command line for options
	//todo: make settingsfile configurable
	settingsfile = qApp->applicationDirPath() + "/settings.ini";
	//todo: make logpath configurable
	logpath = qApp->applicationDirPath() + "/logs";

	networkaccessmanager = new QNetworkAccessManager(qApp);
	bbcodeparser = new BBCodeParser();
	//settings = new QSettings(settingsfile, QSettings::IniFormat);
	settings = new FSettings(settingsfile, qApp);
}

void globalQuit()
{

}



bool is_broken_escaped_apos(std::string const &data, std::string::size_type n)
{
        return n + 2 <= data.size()
                        and data[n] == '\\'
                        and data[n+1] == '\'';
}
void fix_broken_escaped_apos (std::string &data)
{
        for ( std::string::size_type n = 0; n != data.size(); ++n )
        {
                if ( is_broken_escaped_apos ( data, n ) )
                {
                        data.replace ( n, 2, 1, '\'' );
                }
        }
}

QString escapeFileName(QString infilename)
{
	QByteArray inname(infilename.toUtf8());
	QByteArray outname;
	for(int i = 0; i < inname.length(); i++) {
		unsigned char c = inname.at(i);
		if((c >= 'A' && c <= 'Z') ||
		   (c >= 'a' && c <= 'z') ||
		   (c >= '0' && c <= '9') ||
		   c == '-' ||
		   c == '_' ||
		   c == ' ' ||
		   c == '.') {
			outname.append(c);
		} else {
			outname.append('%');
			outname.append(QByteArray::number(c, 16));
		}
	}
	return QString::fromUtf8(outname);
}

QString htmlToPlainText(QString input) {
	QString output = input;
	//Strip tags
	output.replace(QRegExp("<[^>]*>"), "");
	//Convert escaped symbols. Only worries about those in the ASCII range.
	output.replace("&quot;", "\"");
	output.replace("&lt;", "<");
	output.replace("&gt;", ">");
	output.replace("&apos;", "'");
	output.replace("&nbsp;", " ");
	output.replace("&amp;", "&");
	return output;
}
