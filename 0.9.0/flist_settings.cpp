#include "flist_settings.h"

FSettings::FSettings(QString &path)
    : helpdesk(false)
{
    this->path = path;
    if (QFile::exists(path))
        loadSettings();
    else
        loadDefaultSettings();
}
void FSettings::saveSettings()
{
    QSettings settings(path, QSettings::IniFormat);
    settings.setValue("join", BOOLSTR(leaveJoin));
    settings.setValue("online", BOOLSTR(onlineOffline));
    settings.setValue("ping", BOOLSTR(ping));
    settings.setValue("sounds", BOOLSTR(sounds));
    settings.setValue("alwaysping", BOOLSTR(alwaysPing));
    settings.setValue("helpdesk", BOOLSTR(helpdesk));
    settings.setValue("logs", BOOLSTR(chatLogs));
    settings.setValue("username", username);
    QString pings, s;
    foreach (s, pinglist)
    {
        pings.append(", ");
        pings.append(s);
    }
    settings.setValue("pinglist", pings.mid(2));
    QString channels;
    foreach(QString c, defaultChannels)
    {
        channels.append("|||");
        channels.append(c);
    }
    settings.setValue("channels", channels.mid(3));
}
void FSettings::loadSettings()
{
    QSettings settings(path, QSettings::IniFormat);
    if (settings.status() != QSettings::NoError)
        loadDefaultSettings();
    else
    {
        leaveJoin = settings.value("join").toBool();
        onlineOffline = settings.value("online").toBool();
        ping = settings.value("ping").toBool();
        sounds = settings.value("sounds").toBool();
        alwaysPing = settings.value("alwaysping").toBool();
        helpdesk = settings.value("helpdesk").toBool();
        chatLogs = settings.value("logs").toBool();
        username = settings.value("username").toString();

        QString pings = settings.value("pinglist").toString();
        setPingList(pings);
        QString channels = settings.value("channels").toString();
        setDefaultChannels(channels);
    }
    FMessage::doPing = ping;
    FMessage::doAlwaysPing = alwaysPing;
}

void FSettings::loadDefaultSettings()
{
    pinglist.clear();
    defaultChannels.clear();
    defaultChannels.append(QString("Frontpage"));
    leaveJoin = true;
    onlineOffline = true;
    ping = true;
    sounds = true;
    alwaysPing = false;
    helpdesk = false;
    chatLogs = true;
}
void FSettings::setDefaultChannels(QString channels)
{
    if (channels != "")
    {
        QStringList c = channels.split("|||");
        foreach (QString s, c)
            defaultChannels.append(s);
    }
}
void FSettings::setPingList(QString list)
{
    if (list != "")
    {
        pinglist.clear();
        QStringList l = list.split(", ");
        foreach (QString s, l)
            pinglist.append(s);
    }
    FMessage::pingList = pinglist;
}
