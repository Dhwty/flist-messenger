#ifndef FLIST_GLOBAL_H
#define FLIST_GLOBAL_H

#include <QNetworkAccessManager>
#include "flist_api.h"
#include "flist_enums.h"

class BBCodeParser; 
class FSettings;

extern QNetworkAccessManager *networkaccessmanager;
extern BBCodeParser *bbcodeparser;
extern FHttpApi::Endpoint *fapi;
extern FSettings *settings;

void debugMessage(QString str, DebugMessageType d);
void debugMessage(std::string str, DebugMessageType d);
void debugMessage(const char *str, DebugMessageType d);
void globalInit();
void globalQuit();
bool is_broken_escaped_apos(std::string const &data, std::string::size_type n);
void fix_broken_escaped_apos (std::string &data);
QString escapeFileName(QString infilename);
QString htmlToPlainText(QString input);

// Centre a window on the screen it's mostly on. No idea what happens if
// the window is not on any screen.
void centerOnScreen(QWidget *widge);

#define FLIST_NAME "F-List Messenger [Beta]"
#define FLIST_VERSIONNUM "0.9.1." GIT_HASH
#define FLIST_VERSION FLIST_NAME " " FLIST_VERSIONNUM
#define FLIST_CLIENTID "F-List Desktop Client"

enum dbgMessageTypes {
    DBG_GENERAL 0,
    DBG_SERVER 1
};

#define DBG_GENERAL 0
#define DBG_SERVER


#define FLIST_CHAT_SERVER "chat.f-list.net"
//#define FLIST_CHAT_SERVER_PORT 8722 //Test server
//#define FLIST_CHAT_SERVER_PORT 9722 //Real server (plain text)
#define FLIST_CHAT_SERVER_PORT 9799 //Real server (encrypted)

#endif // FLIST_GLOBAL_H
