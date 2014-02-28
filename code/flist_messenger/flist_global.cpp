#include "flist_global.h"
#include <QApplication>
#include <iostream>

QNetworkAccessManager *networkaccessmanager;

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
	networkaccessmanager = new QNetworkAccessManager(qApp);
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
