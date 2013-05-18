#include "flist_systemmessage.h"

FSystemMessage::FSystemMessage(SystemMessageType sysType, FChannel* channel, QString& msg)
    : FMessage(channel, 0, msg),
      systemType(sysType)
{
    type = MESSAGETYPE_SYSTEM;
    parse();
}

void FSystemMessage::parse()
{
    doesPing = checkPing();
    QString timestr = QTime::currentTime().toString("h:mm AP");

    output = "<div class=\""+ cssClassNames[type] +"\">";
    output += "<small><i>[" + timestr + "]</i></small> " + message;
    output += "</div>";

    output = bbparser->parse(output);
}
bool FSystemMessage::checkPing()
{
    // Don't ping on leave/join or online/offline messages
    if (systemType == SYSTYPE_JOIN ||
        systemType == SYSTYPE_ONLINE ||
        systemType == SYSTYPE_FEEDBACK
    )
        return false;

    // Else, ping if name is mentioned
    // (Even if settings say don't. This is for bans, kicks, etc. Quite important~)
    if (message.contains(selfName, Qt::CaseInsensitive))
        return true;

    // else
    return false;
}
