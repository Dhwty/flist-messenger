#include "flist_reportmessage.h"

FReportMessage::FReportMessage(FCharacter* character, QString& msg)
    : FMessage(0, character, msg)
{
    type = MESSAGETYPE_BROADCAST;
    parse();
}

void FReportMessage::parse()
{
    doesPing = checkPing();
    QString timestr = QTime::currentTime().toString("h:mm AP");

    output = "<div class=\""+ cssClassNames[type] +"\">";
    output += "<small><i>[" + timestr + "]</i></small> " + message;
    output += "</div>";

    output = bbparser->parse(output);
}
bool FReportMessage::checkPing()
{
    // Moderators should always get pings from this.
    return true;
}
