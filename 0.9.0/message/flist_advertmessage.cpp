#include "flist_advertmessage.h"

FAdvertMessage::FAdvertMessage(FChannel* channel, FCharacter* character, QString& msg)
    : FMessage(channel, character, msg)
{
    type = MESSAGETYPE_ROLEPLAYAD;
    parse();
}

void FAdvertMessage::parse()
{
    doesPing = checkPing();

    // Prepare for the button's colour to change.
    channel->setHasNewMessages(true);
    if (doesPing)
    {
        channel->setHighlighted(true);
    }

    QString timestr = QTime::currentTime().toString("h:mm AP");

    output = "<div class=\""+ cssClassNames[type] +"\">";
    output += "<small><i>[" + timestr + "]</i></small> <font color=\"green\"><b>Roleplay ad by</font> " + charNameToHtml();
    output += "</div>";

    output = bbparser->parse(output);
    std::cout << output.toStdString() << std::endl;
}
bool FAdvertMessage::checkPing()
{
    if (sender && sender->name() == selfName)
    {
        // Don't ping if user is the sender
        return false;
    }
    if (channel->getAlwaysPing())
    {
        // Channel always pings.
        return true;
    }
    if (doPing == false)
    {
        // Else, don't ping if settings say don't
        return false;
    }
    if (message.contains(selfName, Qt::CaseInsensitive))
    {
        // Else, ping if name is mentioned
        return true;
    }
    else
    {
        // Else, check pinglist
        foreach (QString s, pingList)
        {
            if (message.contains(s, Qt::CaseInsensitive))
            {
                // Ping if item found
                return true;
            }
        }
    }

    // else, no ping
    return false;
}
