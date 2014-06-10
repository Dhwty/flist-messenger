#include "flist_settings.h"

#include <QSettings>

FSettings::FSettings(QString settingsfile, QObject *parent) :
        QObject(parent),
	settingsfile(settingsfile)
{
	qsettings = new QSettings(settingsfile, QSettings::IniFormat);
	
}
FSettings::~FSettings()
{
	qsettings->sync();
	delete qsettings;
}

#define GETSETBOOL(func, text, dflt)					\
	bool FSettings::get##func() {return qsettings->value(text, dflt).toBool();} \
	void FSettings::set##func(bool opt) {qsettings->setValue(text, opt);}
#define GETSETSTRING(func, text, dflt)					\
	QString FSettings::get##func() {return qsettings->value(text, dflt).toString();} \
	void FSettings::set##func(QString opt) {qsettings->setValue(text, opt);}

#define GETSET(type, totype, func, text, dflt)				\
	type FSettings::get##func() {return qsettings->value(text, dflt).totype();} \
	void FSettings::set##func(type opt) {qsettings->setValue(text, opt);}

#define GETSETANY(type, totype, func)			\
	type FSettings::get##func(QString key, type dflt) {return qsettings->value(key, dflt).totype();} \
	void FSettings::set##func(QString key, type value) {qsettings->setValue(key, value);}

#define GETSETGROUP(type, totype, func, text, dflt)			\
	type FSettings::get##func(QString group) {return qsettings->value(text, dflt).totype();} \
	void FSettings::set##func(QString group, type opt) {qsettings->setValue(text, opt);}
#define GETSETCHANNEL(type, totype, func, text, dflt)			\
	GETSETGROUP(type, totype, func, QString("Channel/%1/%2").arg(group, text), dflt)
#define GETSETCHARACTER(type, totype, func, text, dflt) \
	GETSETGROUP(type, totype, func, QString("Character/%1/%2").arg(group, text), dflt)

//Generic
GETSETANY(bool, toBool, Bool)
GETSETANY(QString, toString, String)

//Account
GETSETSTRING(UserAccount, "Global/account", "")
//Channels
GETSETSTRING(DefaultChannels, "Global/default_channels", "Frontpage|||F-chat Desktop Client")
//Logging
GETSETBOOL(LogChat, "Global/log_chat", true)
//Show message options
GETSETBOOL(ShowOnlineOfflineMessage, "Global/show_online_offline", true)
GETSETBOOL(ShowJoinLeaveMessage, "Global/show_join_leave", true)
//Sound options
GETSETBOOL(PlaySounds, "Global/play_sounds", false)

