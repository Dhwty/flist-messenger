#include "flist_chatmessage.h"

FChatMessage::FChatMessage(FChannel* channel, FCharacter* character, QString& msg)
    : FMessage(channel, character, msg)
{
    type = (channel->type() == FChannel::CHANTYPE_PM) ? MESSAGETYPE_PRIVMESSAGE : MESSAGETYPE_CHANMESSAGE;
    parse();
}

void FChatMessage::parse()
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
    output += "<small><i>[" + timestr + "]</i></small> " + charNameToHtml();
    output += "</div>";

    output = bbparser->parse(output);
    std::cout << output.toStdString() << std::endl;
}
bool FChatMessage::checkPing()
{
    if (sender && sender->name() == selfName)
    {
        // Don't ping if user is the sender
        return false;
    }
    if (type == MESSAGETYPE_PRIVMESSAGE)
    {
        // Else, ping if PM
        return true;
    }
    if (channel->getAlwaysPing())
    {
        // The user wants this channel to ping on every message.
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

    // else:
    return false;
}
