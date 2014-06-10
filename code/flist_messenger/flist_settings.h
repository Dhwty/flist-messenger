#ifndef FLIST_SETTINGS_H
#define FLIST_SETTINGS_H

#include <QObject>
#include <QString>

class QSettings;

class FSettings : public QObject
{
	Q_OBJECT
public:
	explicit FSettings(QString settingsfile, QObject *parent = 0);
	~FSettings();

#define PROTOGETSET(func, typ)			\
	typ get##func();			\
	void set##func(typ opt)

#define PROTOGETSETANY(func, typ, defaultvalue)			\
	typ get##func(QString key, typ dflt = defaultvalue);	\
	void set##func(QString key, typ value)

#define PROTOGETSETGROUP(func, typ)			\
	typ get##func(QString group);			\
	void set##func(QString group, typ opt)

//Generic
	PROTOGETSETANY(Bool, bool, false);
	PROTOGETSETANY(String, QString, "");

//Account
	PROTOGETSET(UserAccount, QString);
//Channels
	PROTOGETSET(DefaultChannels, QString);
//Logging
	PROTOGETSET(LogChat, bool);
//Show message options
	PROTOGETSET(ShowOnlineOfflineMessage, bool);
	PROTOGETSET(ShowJoinLeaveMessage, bool);
//Sound options
	PROTOGETSET(PlaySounds, bool);


#undef PROTOGETSET

	QSettings *qsettings;

private:
	QString settingsfile;
signals:

public slots:

};

#endif // FLIST_SETTINGS_H
