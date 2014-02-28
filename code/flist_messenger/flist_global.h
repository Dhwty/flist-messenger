#ifndef FLIST_GLOBAL_H
#define FLIST_GLOBAL_H

#include <QNetworkAccessManager>

extern QNetworkAccessManager *networkaccessmanager;

void debugMessage(QString str);
void debugMessage(std::string str);
void debugMessage(const char *str);
void globalInit();
void globalQuit();
bool is_broken_escaped_apos(std::string const &data, std::string::size_type n);
void fix_broken_escaped_apos (std::string &data);

#define FLIST_VERSIONNUM "0.8.6"
#define FLIST_VERSION "F-List Messenger [Beta] " FLIST_VERSIONNUM
#define FLIST_CLIENTID "F-List Desktop Client"


#define FLIST_CHAT_SERVER "chat.f-list.net"
#define FLIST_PORT 8722 //Test server
//#define FLIST_PORT 9722 //Real server


#endif // FLIST_GLOBAL_H
