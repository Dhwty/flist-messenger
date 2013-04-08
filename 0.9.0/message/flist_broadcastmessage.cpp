#include "flist_broadcastmessage.h"

FBroadcastMessage::FBroadcastMessage(QString& msg)
    : FMessage(0, 0, msg)
{
    type = MESSAGETYPE_BROADCAST;
    parse();
}
void FBroadcastMessage::parse()
{
    doesPing = checkPing();
    QString timestr = QTime::currentTime().toString("h:mm AP");

    output = "<div class=\""+ cssClassNames[type] +"\">";
    output += "<small><i>[" + timestr + "]</i></small> " + message;
    output += "</div>";

    output = bbparser->parse(output);
}
bool FBroadcastMessage::checkPing()
{
    // Broadcasts always ping
    return true;
}
