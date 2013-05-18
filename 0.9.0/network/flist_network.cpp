#include "flist_network.h"

FNetwork::FNetwork(flist_messenger *parent)
    : QTcpSocket()
{
    this->mainprog = parent;
    connected = false;
    doingWS = true;
}
void FNetwork::tryLogin(QString& username, QString& charName, QString& loginTicket)
{
    this->username = username;
    this->charname = charName;
    this->loginTicket = loginTicket;

    setupSocket();
}

void FNetwork::setupSocket()
{
    connectToHost ( "chat.f-list.net", 9722 );
    connect ( this, SIGNAL ( connected() ), this, SLOT ( connectedToSocket() ) );
    connect ( this, SIGNAL ( readyRead() ), this, SLOT ( readReadyOnSocket() ) );
    connect ( this, SIGNAL ( error ( QAbstractSocket::SocketError ) ), this, SLOT ( socketError ( QAbstractSocket::SocketError ) ) );
    connected = true;
}
void FNetwork::connectedToSocket()
{
    connected = true;
    write ( mainprog->WSConnect.c_str() );
    flush();
}
void FNetwork::readReadyOnSocket()
{
    if ( doingWS )
    {
        QByteArray buffer = readAll();
        std::string buf ( buffer.begin(), buffer.end() );

        if ( buf.find ( "\r\n\r\n" ) != std::string::npos )
            doingWS = false;

        JSONNode loginnode;
        JSONNode tempnode ( "method", "ticket" );
        loginnode.push_back ( tempnode );
        tempnode.set_name ( "account" );
        tempnode = username.toStdString();
        loginnode.push_back ( tempnode );
        tempnode.set_name ( "character" );
        tempnode = charname.toStdString();
        loginnode.push_back ( tempnode );
        tempnode.set_name ( "ticket" );
        tempnode = loginTicket.toStdString();
        loginnode.push_back ( tempnode );
        tempnode.set_name ( "cname" );
        tempnode = CLIENTID;
        loginnode.push_back ( tempnode );
        tempnode.set_name ( "cversion" );
        tempnode = VERSIONNUM;
        loginnode.push_back ( tempnode );
        std::string idenStr = "IDN " + loginnode.write();
        sendWS ( idenStr );
    }
    else
    {
        QByteArray buffer = readAll();
        std::string buf ( networkBuffer );
        buf.append ( buffer.begin(), buffer.end() );
        int spos = buf.find ( ( char ) 0 );
        int epos = buf.find ( ( char ) 0xff );

        while ( spos != std::string::npos && epos != std::string::npos )
        {
            std::string cmd = buf.substr ( spos + 1, epos - ( spos + 1 ) );
            mainprog->parseCommand ( cmd );
            spos = buf.find ( ( char ) 0, epos );
            epos = buf.find ( ( char ) 0xff, epos + 1 );
        }

        if ( spos != std::string::npos && epos == std::string::npos )
        {
            networkBuffer = buf.substr ( spos, buf.length() - spos );
        }
        else if ( networkBuffer.length() )
        {
            networkBuffer.clear();
        }
    }
}
void FNetwork::socketError ( QAbstractSocket::SocketError socketError )
{
    mainprog->socketError(socketError);
    connected = false;
    abort();
}
void FNetwork::sendWS ( std::string& input )
{
    if ( !connected )
    {
        std::cout << "Attempted to send a message, but client is disconnected." << std::endl;
    }
    else
    {
        fixBrokenEscapedApos ( input );
        std::cout << ">>" << input << std::endl;
        QByteArray buf;
        QDataStream stream ( &buf, QIODevice::WriteOnly );
        input.resize ( input.length() );
        stream << ( quint8 ) 0;
        stream.writeRawData ( input.c_str(), input.length() );
        stream << ( quint8 ) 0xff;
        write ( buf );
        flush();
    }
}
bool FNetwork::isBrokenEscapedApos ( std::string const &data, std::string::size_type n )
{
    return n + 2 <= data.size()
            and data[n] == '\\'
            and data[n+1] == '\'';
}
void FNetwork::fixBrokenEscapedApos ( std::string &data )
{
    for ( std::string::size_type n = 0; n != data.size(); ++n )
    {
        if ( isBrokenEscapedApos ( data, n ) )
        {
            data.replace ( n, 2, 1, '\'' );
        }
    }
}

//=========================================================
//
// COMMANDS CLIENT -> SERVER.
//
//=========================================================
void FNetwork::sendPing()
{
    std::string msg = "PIN";
    sendWS(msg);
}
void FNetwork::sendDebug(QString& command)
{
    JSONNode root;
    JSONNode temp;
    temp.set_name("command");
    temp = command.toStdString().c_str();
    root.push_back(temp);
    std::string debug = "ZZZ ";
    debug += root.write().c_str();
    sendWS( debug );
}
void FNetwork::sendJoinChannel ( QString& channel )
{
    JSONNode joinnode;
    JSONNode channode ( "channel", channel.toStdString() );
    joinnode.push_back ( channode );
    std::string msg = "JCH " + joinnode.write();

    sendWS ( msg );
}
void FNetwork::sendLeaveChannel(QString& channel)
{
    std::string chan = channel.toStdString();
    JSONNode leavenode;
    JSONNode channode ( "channel", chan );
    leavenode.push_back ( channode );
    std::string msg = "LCH " + leavenode.write();
    sendWS ( msg );
}
void FNetwork::sendSetStatus(QString& status, QString& message)
{

    JSONNode stanode;
    JSONNode statusnode ( "status", status.toStdString() );
    JSONNode stamsgnode ( "statusmsg", message.toStdString() );
    stanode.push_back ( statusnode );
    stanode.push_back ( stamsgnode );
    std::string msg = "STA " + stanode.write();
    sendWS ( msg );
}
void FNetwork::sendIgnoreAdd(QString& character)
{
    character = character.toLower();
    JSONNode ignorenode;
    JSONNode targetnode ( "character", character.toStdString() );
    JSONNode actionnode ( "action", "add" );
    ignorenode.push_back ( targetnode );
    ignorenode.push_back ( actionnode );
    std::string msg = "IGN " + ignorenode.write();
    sendWS ( msg );
}
void FNetwork::sendIgnoreDelete(QString& character)
{
    character = character.toLower();
    JSONNode ignorenode;
    JSONNode targetnode ( "character", character.toStdString() );
    JSONNode actionnode ( "action", "delete" );
    ignorenode.push_back ( targetnode );
    ignorenode.push_back ( actionnode );
    std::string msg = "IGN " + ignorenode.write();
    sendWS ( msg );
}
void FNetwork::sendChannelsRequest()
{
    std::string out = "CHA";
    sendWS ( out );
}
void FNetwork::sendProomsRequest()
{
    std::string out = "ORS";
    sendWS ( out );
}
void FNetwork::sendChannelKick(QString& channel, QString& character)
{
    JSONNode kicknode;
    JSONNode charnode ( "character", character.toStdString() );
    kicknode.push_back ( charnode );
    JSONNode channode ( "channel", channel.toStdString() );
    kicknode.push_back ( channode );
    std::string out = "CKU " + kicknode.write();
    sendWS ( out );
}
void FNetwork::sendChannelBan(QString& channel, QString& character)
{
    JSONNode kicknode;
    JSONNode charnode ( "character", character.toStdString() );
    kicknode.push_back ( charnode );
    JSONNode channode ( "channel", channel.toStdString() );
    kicknode.push_back ( channode );
    std::string out = "CBU " + kicknode.write();
    sendWS ( out );

}
void FNetwork::sendKick(QString& character)
{
    JSONNode kicknode;
    JSONNode charnode ( "character", character.toStdString() );
    kicknode.push_back ( charnode );
    std::string out = "KIK " + kicknode.write();
    sendWS ( out );
}
void FNetwork::sendBan(QString& character)
{
    JSONNode node;
    JSONNode charnode ( "character", character.toStdString() );
    node.push_back ( charnode );
    std::string out = "ACB " + node.write();
    sendWS ( out );
}
void FNetwork::sendMakeRoom(QString& channel)
{
    JSONNode makenode;
    JSONNode namenode ( "channel", channel.toStdString() );
    makenode.push_back ( namenode );
    std::string out = "CCR " + makenode.write();
    sendWS ( out );
}
void FNetwork::sendRestrictRoom(QString& channel, QString& restriction)
{
    JSONNode statusnode;
    JSONNode channelnode("channel", channel.toStdString());
    JSONNode statnode("status", restriction.toStdString());
    statusnode.push_back(channelnode);
    statusnode.push_back(statnode);
    std::string out = "RST " + statusnode.write();
    sendWS( out );
}
void FNetwork::sendChannelInvite(QString& channel, QString& character)
{
    JSONNode invitenode;
    JSONNode charnode ( "character", character.toStdString() );
    invitenode.push_back ( charnode );
    JSONNode channode ( "channel", channel.toStdString() );
    invitenode.push_back ( channode );
    std::string out = "CIU " + invitenode.write();
    sendWS ( out );
}
void FNetwork::sendChannelAddOp(QString& channel, QString& character)
{
    JSONNode opnode;
    JSONNode charnode ( "character", character.toStdString() );
    opnode.push_back ( charnode );
    JSONNode channode ( "channel", channel.toStdString() );
    opnode.push_back ( channode );
    std::string out = "COA " + opnode.write();
    sendWS ( out );
}
void FNetwork::sendChannelDeleteOp(QString& channel, QString& character)
{
    JSONNode opnode;
    JSONNode charnode ( "character", character.toStdString() );
    opnode.push_back ( charnode );
    JSONNode channode ( "channel", channel.toStdString() );
    opnode.push_back ( channode );
    std::string out = "COR " + opnode.write();
    sendWS ( out );
}
void FNetwork::sendAddOp(QString& character)
{
    JSONNode opnode;
    JSONNode charnode ( "character", character.toStdString() );
    opnode.push_back ( charnode );
    std::string out = "AOP " + opnode.write();
    sendWS ( out );
}
void FNetwork::sendDeleteOp(QString& character)
{
    JSONNode node;
    JSONNode charnode ( "character", character.toStdString() );
    node.push_back ( charnode );
    std::string out = "DOP " + node.write();
    sendWS ( out );
}
void FNetwork::sendReward(QString& character)
{
    JSONNode node;
    JSONNode charnode ( "character", character.toStdString() );
    node.push_back ( charnode );
    std::string out = "RWD " + node.write();
    sendWS ( out );
}
void FNetwork::sendChannelUnban(QString& channel, QString& character)
{
    JSONNode node;
    JSONNode charnode ( "character", character.toStdString() );
    node.push_back ( charnode );
    JSONNode channode ( "channel", channel.toStdString() );
    node.push_back ( channode );
    std::string out = "CUB " + node.write();
    sendWS ( out );
}
void FNetwork::sendRequestBanlist(QString& channel)
{
    JSONNode node;
    JSONNode channode ( "channel", channel.toStdString() );
    node.push_back ( channode );
    std::string out = "CBL " + node.write();
    sendWS ( out );
}
void FNetwork::sendSetDescription(QString& channel, QString& description)
{
    JSONNode node;
    JSONNode channode ( "channel", channel.toStdString() );
    node.push_back ( channode );
    JSONNode descnode ( "description", description.toStdString() );
    node.push_back ( descnode );
    std::string out = "CDS " + node.write();
    sendWS ( out );
}
void FNetwork::sendRequestCoplist(QString& channel)
{
    JSONNode node;
    JSONNode channode ( "channel", channel.toStdString() );
    node.push_back ( channode );
    std::string out = "COL " + node.write();
    sendWS ( out );
}
void FNetwork::sendTimeout(QString& character, int time, QString& reason)
{
    JSONNode node;
    JSONNode charnode ( "character", character.toStdString() );
    node.push_back ( charnode );
    JSONNode timenode ( "time", time );
    node.push_back ( timenode );
    JSONNode renode ( "reason", reason.toStdString() );
    node.push_back ( renode );
    std::string out = "TMO " + node.write();
    sendWS ( out );
}
void FNetwork::sendUnban(QString& character)
{
    JSONNode node;
    JSONNode charnode ( "character", character.toStdString() );
    node.push_back ( charnode );
    std::string out = "UNB " + node.write();
    sendWS ( out );
}
void FNetwork::sendCreateChannel(QString& channel)
{
    JSONNode node;
    JSONNode channode ( "channel", channel.toStdString() );
    node.push_back ( channode );
    std::string out = "CRC " + node.write();
    sendWS ( out );
}
void FNetwork::sendKillChannel(QString& channel)
{
    JSONNode node;
    JSONNode channode ( "channel", channel.toStdString() );
    node.push_back ( channode );
    std::string out = "KIC " + node.write();
    sendWS ( out );
}
void FNetwork::sendBroadcast(QString& broadcast)
{
    JSONNode node;
    JSONNode msgnode ( "message", broadcast.toStdString() );
    node.push_back ( msgnode );
    std::string out = "BRO " + node.write();
    sendWS ( out );
}
void FNetwork::sendSetMode(QString& channel, QString& mode)
{
    JSONNode node;
    JSONNode channode("channel", channel.toStdString());
    JSONNode modenode("mode", mode.toStdString());
    node.push_back(channode);
    node.push_back(modenode);
    std::string out = "RMO " + node.write();
    sendWS(out);
}
void FNetwork::sendRoll(QString& channel, QString& dice)
{
    std::string out = "RLL ";
    JSONNode node;
    JSONNode channode("channel", channel.toStdString());
    JSONNode dicenode("dice", dice.toStdString());
    node.push_back(channode);
    node.push_back(dicenode);
    out += node.write();
    sendWS(out);
}
void FNetwork::sendChatMessage(QString& channel, QString& message)
{
    JSONNode msgnode;
    JSONNode channode ( "channel", channel.toStdString() );
    JSONNode textnode ( "message", message.toStdString() );
    msgnode.push_back ( channode );
    msgnode.push_back ( textnode );
    std::string msg = "MSG " + msgnode.write();
    sendWS ( msg );
}
void FNetwork::sendPrivateMessage(QString& character, QString& message)
{
    JSONNode msgnode;
    JSONNode targetnode ( "recipient", character.toStdString() );
    JSONNode textnode ( "message", message.toStdString() );
    msgnode.push_back ( targetnode );
    msgnode.push_back ( textnode );
    std::string msg = "PRI " + msgnode.write();
    sendWS ( msg );
}
void FNetwork::sendAdvert(QString &channel, QString &message)
{
    JSONNode msgnode;
    JSONNode channode("channel", channel.toStdString());
    JSONNode textnode("message", message.toStdString());
    msgnode.push_back(channode);
    msgnode.push_back(textnode);
    std::string msg = "LRP " + msgnode.write();
    sendWS(msg);
}
void FNetwork::sendConfirmReport(QString &moderator, QString &callid)
{
    JSONNode node;
    JSONNode actionnode("action", "confirm");
    JSONNode modnode("moderator", moderator.toStdString());
    JSONNode idnode("callid", callid.toStdString());
    node.push_back(actionnode);
    node.push_back(modnode);
    node.push_back(idnode);
    std::string output = "SFC " + node.write();
    sendWS(output);
}
void FNetwork::sendTypingChanged(QString &character, QString &status)
{
    JSONNode node;
    JSONNode statusnode ( "status", status.toStdString() );
    JSONNode charnode ( "character", character.toStdString() );
    node.push_back ( statusnode );
    node.push_back ( charnode );
    std::string msg = "TPN " + node.write();
    sendWS ( msg );
}
void FNetwork::sendRequestCharacterInfo(QString &character)
{
    JSONNode outNode;
    JSONNode cn ( "character", character.toStdString() );
    outNode.push_back ( cn );
    std::string out = "PRO " + outNode.write();
    sendWS ( out );
    out = "KIN " + outNode.write();
    sendWS ( out );
}
void FNetwork::sendReport(QString &character, QString &logid, QString &report)
{
    JSONNode node;
    JSONNode actionnode("action", "report");
    JSONNode logidnode("logid", logid.toStdString());
    JSONNode charnode("character", character.toStdString());
    JSONNode reportnode("report", report.toStdString());
    node.push_back(actionnode);
    node.push_back(charnode);
    node.push_back(reportnode);
    node.push_back(logidnode);
    std::string output = "SFC " + node.write();
    sendWS(output);
}

//=========================================================
//
// SERVER COMMAND PARSING.
//
//=========================================================



void FNetwork::parseCommand(QString &input)
{
/*
    try
    {
        std::cout << "<<" << input.toStdString() << std::endl;
        std::string cmd = input.substr ( 0, 3 );
        JSONNode nodes;

        if ( input.length() > 4 )
        {
            nodes = libJSON::parse ( input.substr ( 4, input.length() - 4 ) );
        }

        if ( cmd == "ADL" )
        {
            JSONNode childnode = nodes.at ( "ops" );
            int size = childnode.size();
            QList<QString> opList = new QList<QString>();
            for ( int i = 0;i < size;++i )
            {
                QString op = childnode[i].as_string().c_str();
                opList.append ( op.toLower() );
            }
            mainprog->setOpList(opList);
        }
        else if ( cmd == "AOP" )
        {
            //AOP {"character": "Viona"}
            QString ch = nodes.at ( "character" ).as_string().c_str();
            mainprog->addOp(ch);
        }
        else if ( cmd == "DOP" )
        {
            //DOP {"character": "Viona"}
            QString ch = nodes.at ( "character" ).as_string().c_str();
            mainprog->removeOp(ch.toLower());
        }
        else if ( cmd == "BRO" )
        {
            QString message(QString::fromUtf8(nodes.at ( "message" ).as_string().c_str()));
            mainprog->receiveBroadcast(message);
        }
        else if ( cmd == "CDS" )
        {
            QString channel = nodes.at ( "channel" ).as_string().c_str();

            if ( channelList.count ( channel ) == 0 )
            {
                printDebugInfo("[SERVER BUG] Server gave us the description of a channel we don't know about yet: " + input);
                return;
            }

            QString desc(QString::fromUtf8(nodes.at ( "description" ).as_string().c_str()));

            channelList[channel]->setDescription ( desc );
            QString msg = "You have joined <b>" + channelList[channel]->title() + "</b>: " + desc;
            FMessage fmsg(FMessage::SYSTYPE_FEEDBACK, channelList[channel], 0, msg, currentPanel);
        }
        else if ( cmd == "CHA" )
        {
            cd_channelsList->clear();
            JSONNode childnode = nodes.at ( "channels" );
            int size = childnode.size();

            for ( int i = 0;i < size; ++i )
            {
                JSONNode channelnode = childnode.at ( i );
                QString name = channelnode.at ( "name" ).as_string().c_str();
                int characters;
                QString cs = channelnode.at ( "characters" ).as_string().c_str();
                characters = cs.toInt();
                printDebugInfo("Channel with characters: " + characters);
                // QString mode = channelnode.at("mode").as_string().c_str();
                ChannelListItem* chan = new ChannelListItem ( name, characters );
                addToChannelsDialogList ( chan );
            }
        }
        else if ( cmd == "CIU" )
        {
            //CIU {"sender": "EagerToPlease", "name": "ADH-085bcf60bef81b0790b7", "title": "Domination and Degradation"}
            QString output;
            QString sender = nodes.at ( "sender" ).as_string().c_str();
            QString name = nodes.at ( "name" ).as_string().c_str();
            QString title = nodes.at ( "title" ).as_string().c_str();
            output = "<b>" + sender + "</b> has invited you to join the room <a href=\"#AHI-" + name + "\">" + title + "</a>.";
            FMessage fmsg(FMessage::SYSTYPE_FEEDBACK, currentPanel, 0, output, currentPanel);
        }
        else if ( cmd == "CKU" )
        {
            // CKU {"operator": "Maeve", "character": "Viona", "channel": "ADH-ad41f30904d30b529d08"}
            // Channel kick
            QString op = nodes.at("operator").as_string().c_str();
            QString chara = nodes.at("character").as_string().c_str();
            QString chan = nodes.at("channel").as_string().c_str();
            FChannel* channel = channelList[chan];
            FCharacter* character = characterList[chara];
            if (!channel)
            {
                printDebugInfo("[SERVER ERROR] Server tells us about a kick, but the channel doesn't exist.");
            } else if (!character){
                printDebugInfo("[SERVER ERROR] Server tells us about a kick, but the recipient doesn't exist.");
            } else {
                QString output;
                output = QString("<b>");
                output+= op;
                output+= QString("</b> has kicked <b>");
                output+= chara;
                output+= QString("</b> from ");
                output+= channel->title();
                if (chara == charName)
                {
                    std::string chanstr = channel->name().toStdString();
                    leaveChannel(chanstr, false);
                    FMessage fmsg(FMessage::SYSTYPE_KICKBAN, currentPanel, 0, output, currentPanel);
                } else {
                    FMessage fmsg(FMessage::SYSTYPE_KICKBAN, channel, 0, output, currentPanel);
                }
            }
        }
        else if ( cmd == "CBU" )
        {
            // CBU {"operator": "Maximilian Paton", "character": "Viona", "channel": "ADH-0e67e06da606b550020b"}
            // Channel ban
            QString op = nodes.at("operator").as_string().c_str();
            QString chara = nodes.at("character").as_string().c_str();
            QString chan = nodes.at("channel").as_string().c_str();
            FChannel* channel = channelList[chan];
            FCharacter* character = characterList[chara];
            if (!channel)
            {
                printDebugInfo("[ERROR] Server tells us about a ban, but the channel doesn't exist.");
            } else if (!character){
                printDebugInfo("[ERROR] Server tells us about a ban, but the recipient doesn't exist.");
            } else {
                QString output("<b>");
                output+= op;
                output+= "</b> has kicked and banned <b>";
                output+= chara;
                output+= "</b> from ";
                output+= channel->title();
                if (chara == charName)
                {
                    std::string chanstr = channel->name().toStdString();
                    leaveChannel(chanstr, false);
                    FMessage fmsg(FMessage::SYSTYPE_KICKBAN, currentPanel, 0, output, currentPanel);
                } else {
                    FMessage fmsg(FMessage::SYSTYPE_KICKBAN, channel, 0, output, currentPanel);
                }
            }
        }
        else if ( cmd == "COA" )
        {
            // COA {"channel": "Diapers/Infantilism"}
        }
        else if ( cmd == "COR" )
        {
            // COR {"channel": "Diapers/Infantilism"}
        }
        else if ( cmd == "COL" )
        {
            QString channelname = nodes.at ( "channel" ).as_string().c_str();

            if ( channelList.count ( channelname ) != 0 )
            {
                FChannel* channel = channelList[channelname];
                QStringList ops;
                JSONNode childnode = nodes.at ( "oplist" );
                int size = childnode.size();

                for ( int i = 0; i < size; ++i )
                {
                    QString username = childnode.at ( i ).as_string().c_str();
                    ops.push_back ( username );
                }

                channel->setOps ( ops );
            }
        }
        else if ( cmd == "CON" )
        {
            QString msg;
            msg += nodes["count"].as_string().c_str();
            msg += " users are currently connected.";
            FMessage fmsg(FMessage::SYSTYPE_FEEDBACK, currentPanel, 0, msg, currentPanel);
        }
        else if ( cmd == "ERR" )
        {
            QString output;
            QString message = nodes.at ( "message" ).as_string().c_str();
            QString number = nodes.at ( "number" ).as_string().c_str();
            output = "<b>Error " + number + ": </b>" + message;

            if ( textEdit )
                FMessage fmsg(FMessage::SYSTYPE_FEEDBACK, currentPanel, 0, output, currentPanel);
            else
                QMessageBox::critical ( this, "Error", message );

            if(number == "34")
            {
                JSONNode loginnode;
                JSONNode tempnode ( "method", "ticket" );
                loginnode.push_back ( tempnode );
                tempnode.set_name ( "account" );
                tempnode = username.toStdString();
                loginnode.push_back ( tempnode );
                tempnode.set_name ( "character" );
                tempnode = charName.toStdString();
                loginnode.push_back ( tempnode );
                tempnode.set_name ( "ticket" );
                tempnode = loginTicket.toStdString();
                loginnode.push_back ( tempnode );
                tempnode.set_name ( "cname" );
                tempnode = CLIENTID;
                loginnode.push_back ( tempnode );
                tempnode.set_name ( "cversion" );
                tempnode = VERSIONNUM;
                loginnode.push_back ( tempnode );
                std::string idenStr = "IDN " + loginnode.write();
                sendWS ( idenStr );
            }
        }
        else if ( cmd == "FLN" )
        {
            QString remchar = nodes.at ( "character" ).as_string().c_str();
            bool posted = false;
            QString offline = "<b>" + remchar + "</b> has disconnected.";
            if ( se_onlineOffline && selfFriendsList.contains ( remchar ) )
            {
                FMessage fmsg(FMessage::SYSTYPE_ONLINE, currentPanel, 0, offline, currentPanel);
                posted = true;
            }
            QString pmPanel = "PM-" + remchar;
            if (channelList.count(pmPanel))
            {
                channelList[pmPanel]->setTyping ( FChannel::TYPINGSTATUS_CLEAR );
                if (posted == false || channelList[pmPanel] != currentPanel)
                    FMessage fmsg(FMessage::SYSTYPE_ONLINE, channelList[pmPanel], 0, offline, currentPanel);


                QString paneltitle = remchar;
                paneltitle += " (Offline)";
                channelList[pmPanel]->setTitle ( paneltitle );
                QString empty = "";
                channelList[pmPanel]->setRecipient(empty);
            }

            if ( characterList.count ( remchar ) )
            {
                FCharacter* character = characterList[remchar];

                for ( QHash<QString, FChannel*>::const_iterator iter = channelList.begin();iter != channelList.end(); ++iter )
                {
                    if (se_leaveJoin && (*iter)->charList().count(character))
                    {
                        QString output = "[b]";
                        output += remchar;
                        output += "[/b] has left the channel.";
                        FMessage fmsg(FMessage::SYSTYPE_JOIN, *iter, 0, output, currentPanel);
                    }
                    ( *iter )->remChar ( character );
                }

                character = 0;

                delete characterList[remchar];
                characterList.remove ( remchar );
            }
        }
        else if ( cmd == "FRL" )
        {
            JSONNode childnode = nodes.at ( "characters" );

            int children = childnode.size();

            for ( int i = 0; i < children; ++i )
            {
                QString addchar = childnode.at(i).as_string().c_str();

                if ( !selfFriendsList.contains ( addchar ) )
                {
                    selfFriendsList.append ( addchar );
                }
            }
        }
        else if ( cmd == "HLO" )
        {
            QString msg = "<B>";
            msg += nodes.at ( "message" ).as_string().c_str();
            msg += "</B>";
            FMessage fmsg(FMessage::SYSTYPE_FEEDBACK, currentPanel, 0, msg, currentPanel);

            foreach (QString s, defaultChannels)
            {
                joinChannel(s);
            }
        }
        else if ( cmd == "ICH" )
        {
            QString channelname = nodes.at ( "channel" ).as_string().c_str();
            bool isAdh = channelname.startsWith ( "ADH-" );

            if ( channelList.count ( channelname ) == 0 )
            {
                if ( isAdh )
                {
                    channelList[channelname] = new FChannel ( channelname, FChannel::CHANTYPE_ADHOC );
                }
                else
                {
                    channelList[channelname] = new FChannel ( channelname, FChannel::CHANTYPE_NORMAL );
                }
            }

            FChannel* channel = channelList[channelname];
            JSONNode childnode = nodes.at ( "users" );

            int size = childnode.size();

            for ( int i = 0;i < size; ++i )
            {
                QString charname = childnode.at ( i ).at ( "identity" ).as_string().c_str();
                FCharacter* character = 0;

                if ( characterList.count ( charname ) == 0 )
                {
                    printDebugInfo("[SERVER BUG] Server gave us a character in the channel user list that we don't know about yet: " + charname.toStdString() + ", " + input);
                    continue;
                }

                character = characterList[charname];
                channel->addChar ( character, false );
            }
            channel->sortChars();
            refreshUserlist();
        }
        else if ( cmd == "IDN" )
        {
            QString msg = "<B>";
            msg += nodes["character"].as_string().c_str();
            msg += "</B> Connected.";
            FMessage fmsg(FMessage::SYSTYPE_FEEDBACK, console, 0, msg, console);
        }
        else if ( cmd == "IGN" )
        {
//			[12:38 PM]<<IGN {"character":"Kalyra","action":"add"}
//			[12:38 PM]<<IGN {"character":"Kalyra","action":"delete"}
            if ( nodes["action"].as_string() == "init" )
            {
                JSONNode childnode = nodes.at("characters");
                int count = childnode.size();
                for ( int i = 0 ; i < count ; ++i )
                {
                    QString charname = childnode.at( i ).as_string().c_str();
                    if ( !selfIgnoreList.contains( charname ) )
                        selfIgnoreList.append( charname );
                }
            }
            if ( nodes["action"].as_string() == "add" )
            {
                QString character = nodes["character"].as_string().c_str();
                selfIgnoreList.append ( character );
                QString out = character +  QString ( " has been added to your ignore list." );
                FMessage fmsg(FMessage::SYSTYPE_FEEDBACK, currentPanel, 0, out, currentPanel);
            }
            if ( nodes["action"].as_string() == "delete" )
            {
                QString character = nodes["character"].as_string().c_str();
                selfIgnoreList.removeAll ( character );
                QString out = character + QString ( " has been removed from your ignore list." );
                FMessage fmsg(FMessage::SYSTYPE_FEEDBACK, currentPanel, 0, out, currentPanel);
            }
            if (friendsDialog)
                refreshFriendLists();
        }
        else if ( cmd == "JCH" )
        {
            QString channel = nodes.at ( "channel" ).as_string().c_str();

            if ( channelList.count ( channel ) == 0 )
            {
                QString adh = "ADH-";
                if ( channel.startsWith ( adh ) )
                {
                    FChannel* chan = new FChannel(channel, FChannel::CHANTYPE_ADHOC);
                    channelList[channel] = chan;
                    QString channeltitle = nodes.at ( "title" ).as_string().c_str();
                    chan->setTitle ( channeltitle );
                    chan->pushButton = addToActivePanels(channel, channeltitle);
                }
                else
                {
                    FChannel* chan = new FChannel(channel, FChannel::CHANTYPE_NORMAL);
                    channelList[channel] = chan;
                    chan->setTitle(channel);
                    chan->pushButton = addToActivePanels(channel, channel);
                }
            }
            else if ( channelList[channel]->getActive() == false )
            {
                channelList[channel]->setActive ( true );
                channelList[channel]->pushButton->setVisible(true);
            }

            QString charname = nodes.at ( "character" ).at ( "identity" ).as_string().c_str();

            FCharacter* character = 0;

            if ( characterList.count ( charname ) == 0 )
            {
                printDebugInfo("[SERVER BUG]: Server told us about a character joining, but we don't know about them yet. " + charname.toStdString());
                return;
            }
            character = characterList[charname];
            channelList[channel]->addChar ( character );
            if ( charname == this->charName )
            {
                switchTab ( channel );
            }
            else
            {
                if ( currentPanel->name() == channel )
                    refreshUserlist();
                if (se_leaveJoin)
                {
                    QString output = "<b>";
                    output += charname;
                    output += "</b> has joined the channel.";
                    FMessage fmsg(FMessage::SYSTYPE_JOIN, channelList[channel], 0, output, currentPanel);
                }
            }
        }
        else if ( cmd == "KID" )
        {
            // [19:41 PM]>>KIN {"character":"Cinnamon Flufftail"}
            // [19:41 PM]<<KID {"message": "(custom) kinks of Cinnamon Flufftail.", "type": "start"}
            // [19:41 PM](custom) kinks of Cinnamon Flufftail.
            // [19:41 PM]<<KID {"type": "custom", "value": "<3", "key": "*Viona"}
            // [19:41 PM]*Viona: <3
            // [19:41 PM]<<KID {"message": "End of (custom) kinks.", "type": "end"}
            QString type = nodes.at ( "type" ).as_string().c_str();

            if ( type == "start" )
            {
                ci_teKinks->clear();
            }
            else if ( type == "custom" )
            {
                QString out;
                QString value = nodes.at ( "value" ).as_string().c_str();
                QString key = nodes.at ( "key" ).as_string().c_str();
                out = QString ( "<b>" ) + key + QString ( ":</b> " ) + value;
                ci_teKinks->append ( out );
            }
        }
        else if ( cmd == "LCH" )
        {
            QString channel = nodes.at ( "channel" ).as_string().c_str();

            if ( channelList.count ( channel ) == 0 )
            {
                return;
            }

            QString charname = nodes.at ( "character" ).as_string().c_str();
            FCharacter* character = 0;
            if ( characterList.count ( charname ) == 0 )
            {
                return;
            }

            if ( charname == charName )
            {
                std::string stdchan = channel.toStdString();
                leaveChannel ( stdchan, false );
                return;
            }
            else
            {
                character = characterList[charname];
                channelList[channel]->remChar ( character );
                if (se_leaveJoin)
                {
                    QString output = "<b>";
                    output += charname;
                    output += "</b> has left the channel.";
                    FMessage fmsg(FMessage::SYSTYPE_JOIN, channelList[channel], 0, output, currentPanel);
                }
            }
        }
        else if ( cmd == "LIS" )
        {
            nodes.preparse();
            JSONNode childnode = nodes.at ( "characters" );
            int size = childnode.size();

            for ( int i = 0;i < size;++i )
            {
                int j = 0;
                JSONNode charnode = childnode.at ( i );
                QString addchar = charnode.at ( 0 ).as_string().c_str();	// Identity

                if ( characterList.count ( addchar ) == 0 )
                {
                    characterList[addchar] = new FCharacter ( addchar, selfFriendsList.count(addchar) > 0 ? true : false );
                }

                FCharacter* character = characterList[addchar];

                QString gender = charnode.at ( 1 ).as_string().c_str();	// Gender
                character->setGender ( gender );
                QString status = charnode.at ( 2 ).as_string().c_str();	// Status
                character->setStatus ( status );
                QString statusmsg = charnode.at ( 3 ).as_string().c_str();	// Status
                character->setStatusMsg ( statusmsg );

                if ( opList.contains ( addchar.toLower() ) )
                    character->setIsChatOp ( true );
            }
        }
        else if ( cmd == "LRP" )
        {
            //<<LRP {"message":":3","character":"Prison","channel":"Sex Driven LFRP"}
            QString channelname = nodes.at ( "channel" ).as_string().c_str();

            if ( channelList.count ( channelname ) == 0 )
            {
                return;
            }
            FChannel* channel = channelList[channelname];
            QString character = nodes.at ( "character" ).as_string().c_str();
            if ( selfIgnoreList.count ( character ) )
            {
                // Ignore message
                return;
            }
            QString message(QString::fromUtf8(nodes.at ( "message" ).as_string().c_str()));

            QString genderColor = "#FFFFFF";
            bool isOp = false;


            FCharacter* chanchar = 0;
            if ( characterList.count ( character ) != 0 )
            {
                chanchar = characterList[character];
                genderColor = chanchar->genderColor().name();
                isOp = ( chanchar->isChatOp() || channel->isOp( chanchar ) || channel->isOwner( chanchar ) );
            }
            FMessage fmsg(FMessage::MESSAGETYPE_ROLEPLAYAD, channel, chanchar, message, currentPanel);
        }
        else if ( cmd == "MSG" )
        {
            QString channelname = nodes.at ( "channel" ).as_string().c_str();

            if ( channelList.count ( channelname ) == 0 )
            {
                return;
            }

            FChannel* channel = channelList[channelname];

            QString character = nodes.at ( "character" ).as_string().c_str();

            if ( selfIgnoreList.count ( character.toLower() ) )
            {
                // Ignore message
                return;
            }

            QString message(QString::fromUtf8(nodes.at ( "message" ).as_string().c_str()));

            FCharacter* chanchar = 0;
            if ( characterList.count ( character ) != 0 )
            {
                chanchar = characterList[character];
            }

            FMessage fmsg(FMessage::MESSAGETYPE_CHANMESSAGE, channel, chanchar, message, currentPanel);
        }
        else if ( cmd == "NLN" )
        {
            QString addchar = nodes.at ( "identity" ).as_string().c_str();

            if ( characterList.count ( addchar ) == 0 )
            {
                characterList[addchar] = new FCharacter ( addchar, selfFriendsList.count(addchar) > 0 ? true : false );
            }

            bool posted = false;
            QString online = "<b>" + addchar + "</b> has connected.";
            if ( se_onlineOffline && selfFriendsList.contains ( addchar ) )
            {
                FMessage fmsg(FMessage::SYSTYPE_ONLINE, currentPanel, 0, online, currentPanel);
                posted = true;
            }
            QString pmPanel = "PM-" + addchar;
            if (channelList.count(pmPanel))
            {
                if (posted == false || channelList[pmPanel] != currentPanel)
                    FMessage fmsg(FMessage::SYSTYPE_ONLINE, channelList[pmPanel], 0, online, currentPanel);
                channelList[pmPanel]->setRecipient(addchar);
                QString paneltitle = characterList[addchar]->PMTitle();
                channelList[pmPanel]->setTitle ( paneltitle );
            }
            FCharacter* character = characterList[addchar];

            QString gender = nodes.at ( "gender" ).as_string().c_str();
            character->setGender ( gender );
            QString status = nodes.at ( "status" ).as_string().c_str();
            character->setStatus ( status );

            if ( opList.contains ( addchar.toLower() ) )
                character->setIsChatOp ( true );
        }
        else if ( cmd == "ORS" )
        {
//             ORS
//    {"channels": [
//     {"name": "ADH-29a2ec641d78e5bd197e", "characters": "1", "title": "Eifania's Little Room"},
//     {"name": "ADH-74e4caef2965f4b33dd4", "characters": "1", "title": "Acrophobia"},
//     {"name": "ADH-fa132c6f2740c5ebaed7", "characters": "10", "title": "Femboy Faggot Fucksluts"}
//    ]}
            cd_proomsList->clear();
            JSONNode childnode = nodes.at ( "channels" );
            int size = childnode.size();

            for ( int i = 0;i < size; ++i )
            {
                JSONNode channelnode = childnode.at ( i );
                QString name = channelnode.at ( "name" ).as_string().c_str();
                QString cs = channelnode.at ( "characters" ).as_string().c_str();
                int characters = cs.toInt();
                QString title = channelnode.at ( "title" ).as_string().c_str();
                ChannelListItem* chan = new ChannelListItem ( name, title, characters );
                addToProomsDialogList ( chan );
            }
        }
        else if ( cmd == "PIN" )
        {
            std::string msg = "PIN";
            sendWS ( msg );
        }
        else if ( cmd == "PRD" )
        {
            QString type = nodes.at ( "type" ).as_string().c_str();

            if ( type == "start" )
            {
                ci_teProfile->clear();
            }
            else if ( type == "info" )
            {
                QString out;
                QString value = nodes.at ( "value" ).as_string().c_str();
                QString key = nodes.at ( "key" ).as_string().c_str();
                out = QString ( "<b>" ) + key + QString ( ":</b> " ) + value;
                ci_teProfile->append ( out );
            }
        }
        else if ( cmd == "PRI" )
        {
            QString character = nodes.at ( "character" ).as_string().c_str();

            if ( selfIgnoreList.count ( character.toLower() ) )
            {
//				std::string out = "IGN ";
//				JSONNode notify;
//				JSONNode ch ( "character", character.toStdString() );
//				JSONNode ac ( "action", "notify" );
//				notify.push_back ( ch );
//				notify.push_back ( ac );
//				out += notify.write();
//				sendWS ( out );
            }
            else
            {
                QString message(QString::fromUtf8(nodes.at ( "message" ).as_string().c_str()));
                receivePM ( message, character );
            }
        }
        else if ( cmd == "RLL" )
        {
            // RLL {"message": "[b]Chromatic[/b] rolls 1d6: [b]2[/b]", "character": "Chromatic", "channel": "ADH-8b02d6012cbad0e7e2c0"}
            QString output = nodes.at ( "message" ).as_string().c_str();
            QString channelname = nodes.at ( "channel" ).as_string().c_str();

            FChannel* channel = channelList[channelname];
            FMessage fmsg(FMessage::SYSTYPE_FEEDBACK, channel, 0, output, currentPanel);
        }
        else if ( cmd == "RTB" )
        {
            // RTB {"type":"note","sender":"Viona","subject":"test"}
        }
        else if ( cmd == "SFC" )
        {
            // A staff report
            QString output;
            QString action = nodes.at ( "action" ).as_string().c_str();

            if ( action == "report" )
            {
                bool logged;
                QString callid = nodes.at ( "callid" ).as_string().c_str();
                QString character = nodes.at ( "character" ).as_string().c_str();
                QString logid;
                try
                {
                    logid = nodes.at ( "logid" ).as_string().c_str();
                    logged = true;
                }
                catch ( std::out_of_range )
                {
                    logged = false;
                }
                QString report = nodes.at ( "report" ).as_string().c_str();
                output	= "<b>STAFF ALERT!</b> From " + character + "<br />";
                output += report + "<br />";
                if (logged)
                    output += "<a href=\"#LNK-http://www.f-list.net/fchat/getLog.php?log=" + logid + "\" ><b>Log~</b></a> | ";
                output += "<a href=\"#CSA-" + callid + "\"><b>Confirm Alert</b></a>";
                FMessage fmsg(FMessage::MESSAGETYPE_REPORT, currentPanel, 0, output, currentPanel, channelList);
            }
            else if ( action == "confirm" )
            {
                output = "<b>";
                output += nodes.at ( "moderator" ).as_string().c_str();
                output += "</b> is handling <b>";
                output += nodes.at ( "character" ).as_string().c_str();
                output += "</b>\'s report.";
                FMessage fmsg(FMessage::SYSTYPE_FEEDBACK, currentPanel, 0, output, currentPanel);
            }
        }
        else if ( cmd == "STA" )
        {
            QString stachar = nodes.at ( "character" ).as_string().c_str();

            if ( characterList.count ( stachar ) )
            {
                FCharacter* character = characterList[stachar];
                QString status(QString::fromUtf8(nodes.at ( "status" ).as_string().c_str()));
                character->setStatus ( status );
                QString statmsg;
                // Crown messages can cause there to be no statusmsg.
                try
                {
                    statmsg = nodes.at ( "statusmsg" ).as_string().c_str();
                    character->setStatusMsg ( statmsg );
                }
                catch ( std::out_of_range )
                {
                    statmsg = "";
                }

                if ( se_onlineOffline && selfFriendsList.contains ( stachar ) )
                {
                    QString statusline = "<b>" + stachar + "</b> is now " + character->statusString();

                    if ( statmsg.length() != 0 )
                        statusline += " (" + statmsg + ")";

                    FMessage fmsg(FMessage::SYSTYPE_ONLINE, currentPanel, 0, statusline, currentPanel);
                }

                if ( channelList.count ( "PM-" + stachar ) )
                {
                    FChannel* pmPanel = channelList["PM-"+stachar];
                    QString paneltitle = character->PMTitle();
                    pmPanel->setTitle ( paneltitle );
                }
            }
            else
            {
                printDebugInfo("[SERVER BUG]: Server told us status for a character we don't know about: " + input);
            }

            refreshUserlist();
        }
        else if ( cmd == "RLL" )
        {
            // Dice rolling or bottling.
            QString output;
            QString message = nodes.at("message").as_string().c_str();
            QString channelname = nodes.at("channel").as_string().c_str();
            FChannel* channel = channelList[channelname];
            if (channel)
            {
                output = message;
                FMessage fmsg(FMessage::SYSTYPE_DICE, channel, 0, output, currentPanel);
            }
        }
        else if ( cmd == "SYS" )
        {
            QString output;
            QString message = nodes.at ( "message" ).as_string().c_str();
            output = "<b>System message:</b> " + message;
            FMessage fmsg(FMessage::SYSTYPE_FEEDBACK, currentPanel, 0, output, currentPanel);
        }
        else if ( cmd == "TPN" )
        {
            // Unparsed command: TPN {"status": "clear", "character": "Becca Greene"}
            // Unparsed command: TPN {"status": "typing", "character": "Becca Greene"}
            // Unparsed command: TPN {"status": "paused", "character": "Becca Greene"}
            QString status = nodes.at ( "status" ).as_string().c_str();
            QString character = nodes.at ( "character" ).as_string().c_str();
            QString panelName = "PM-" + character;

            if ( channelList.count ( panelName ) != 0 )
            {
                FChannel* panel = channelList[panelName];

                if ( status == "typing" )
                {
                    panel->setTyping ( FChannel::TYPINGSTATUS_TYPING );
                }
                else if ( status == "paused" )
                {
                    panel->setTyping ( FChannel::TYPINGSTATUS_PAUSED );
                }
                else // if (status == "clear")
                {
                    panel->setTyping ( FChannel::TYPINGSTATUS_CLEAR );
                }

                panel->updateButtonColor();
            }
        }
        else if ( cmd == "VAR" )
        {
            // <<VAR {"value":4096,"variable":"chat_max"}
            QString value = nodes.at("value").as_string().c_str();
            QString variable = nodes.at("value").as_string().c_str();
            serverVariables[variable] = value;
        }
        else if ( cmd == "RMO" )
        {
            //			[12:15 AM] Unparsed command: RMO {"mode":"chat","channel":"ADH-af9c1cd5e1bf31220ab2"}
            //			[12:15 AM] Unparsed command: RMO {"mode":"both","channel":"ADH-af9c1cd5e1bf31220ab2"}
            //			[12:15 AM] Unparsed command: RMO {"mode":"ads","channel":"ADH-af9c1cd5e1bf31220ab2"}
            QString output;
            QString mode = nodes.at("mode").as_string().c_str();
            QString channel = nodes.at("channel").as_string().c_str();
            FChannel* chan = channelList[channel];
            if (chan==0) return;
            QString name = channelList[channel]->title();
            if (mode == "ads")
            {
                chan->setMode(FChannel::CHANMODE_ADS);
                output = name + "'s mode was changed to: Ads only.";
            }
            else if (mode == "chat")
            {
                chan->setMode(FChannel::CHANMODE_CHAT);
                output = name + "'s mode was changed to: Chat only.";
            }
            else
            {
                chan->setMode(FChannel::CHANMODE_BOTH);
                output = name + "'s mode was changed to: Chat and ads.";
            }
            chan->setMode(FChannel::CHANMODE_ADS);
            FMessage fmsg(FMessage::SYSTYPE_FEEDBACK, chan, 0, output, currentPanel);
        }
        else if ( cmd == "ZZZ" )
        {
            QString output;
            QString message = nodes.at ( "message" ).as_string().c_str();
            output = "<b>Debug Reply:</b> " + message;
            FMessage fmsg(FMessage::SYSTYPE_FEEDBACK, currentPanel, 0, output, currentPanel);
        }
        else
        {
            printDebugInfo("Unparsed command: " + input);
            QString qinput = "Unparsed command: ";
            qinput += input.c_str();
            FMessage fmsg(FMessage::SYSTYPE_FEEDBACK, currentPanel, 0, qinput, currentPanel);
        }
    }
    catch ( std::invalid_argument )
    {
        printDebugInfo("Server returned invalid json in its response: " + input);
    }
    catch ( std::out_of_range )
    {
        printDebugInfo("Server produced unexpected json without a field we expected: " + input);
    }
*/
}
