#ifndef FLIST_SETTINGS_H
#define FLIST_SETTINGS_H

#include<QString>
#include<QStringList>
#include<QSettings>
#include"message/flist_message.h"

// Bool to string macro
#define BOOLSTR(b) ( (b) ? "true" : "false" )
// String to bool macro
#define STRBOOL(s) ( (s=="true") ? true : false )

class FSettings
{
public:
    FSettings(QString& path);
    bool postLeaveJoin() {return leaveJoin;}
    bool postOnlineOffline() {return onlineOffline;}
    bool doChatLogs() {return chatLogs;}
    bool doSounds() {return sounds;}
    bool doAlwaysPing() {return alwaysPing;}
    bool doPing() {return ping;}
    bool showHelpdesk() { std::cout << helpdesk << std::endl; return helpdesk;}
    QString* getUsername() {return &username;}
    QStringList* getPingList() {return &pinglist;}
    QStringList* getDefaultChannels() {return &defaultChannels;}
    QString* getPath() {return &path;}

    void setLeaveJoin(bool leaveJoin) { this->leaveJoin = leaveJoin; }
    void setOnlineOffline(bool onlineOffline) { this->onlineOffline = onlineOffline; }
    void setChatLogs(bool chatLogs) { this->chatLogs = chatLogs; }
    void setSounds(bool sounds) { this->sounds = sounds; }
    void setAlwaysPing(bool alwaysPing) { this->alwaysPing = alwaysPing; }
    void setPing(bool ping) { this->ping = ping; }
    void setHelpdesk(bool helpdesk) { this->helpdesk = helpdesk;}
    void setUsername(QString username) {this->username = username;}
    void setPingList(QString list);
    void setDefaultChannels(QString channels);
    void addChannel(QString& channel) { if (defaultChannels.count(channel) == 0) defaultChannels.append(channel);}
    void removeChannel(QString& channel) { defaultChannels.removeAll(channel);}

    void loadSettings();
    void saveSettings();
    void loadDefaultSettings();
private:
    QStringList defaultChannels;
    QStringList pinglist;
    QString username;
    QString path;
    bool leaveJoin;
    bool onlineOffline;
    bool chatLogs;
    bool sounds;
    bool alwaysPing;
    bool ping;
    bool helpdesk;
};

#endif // FLIST_SETTINGS_H
