
#include <QTime>
#include <QSslSocket>

#include "flist_session.h"
#include "flist_account.h"
#include "flist_global.h"
#include "flist_server.h"
#include "flist_character.h"
#include "flist_iuserinterface.h"
#include "flist_channel.h"
#include "flist_parser.h"
#include "flist_message.h"

FSession::FSession(FAccount *account, QString &character, QObject *parent)
    : QObject(parent),
      connected(false),
      account(account),
      sessionid(character),
      character(character),
      tcpsocket(0),
      characterlist(),
      friendslist(),
      bookmarklist(),
      operatorlist(),
      ignorelist(),
      channellist(),
      autojoinchannels(),
      servervariables(),
      knownchannellist(),
      knownopenroomlist(),
      wsready(false),
      socketreadbuffer() {}

FSession::~FSession() {
    if (tcpsocket != 0) {
        delete tcpsocket;
        tcpsocket = 0;
    }
}

FCharacter *FSession::addCharacter(QString name) {
    FCharacter *character = getCharacter(name);
    if (!character) {
        character = new FCharacter(name, friendslist.contains(name));
        characterlist[name] = character;
    }
    return character;
}

void FSession::removeCharacter(QString name) {
    // Get and remove the character from the list.
    FCharacter *character = characterlist.take(name);
    // Delete if not null.
    if (character) {
        delete character;
    }
}

/**
Convert a character's name into a formated hyperlinked HTML text. It will use the correct colors if they're known.
 */
QString FSession::getCharacterHtml(QString name) {
    QString html = "<b><a style=\"color: %1\" href=\"%2\">%3</a></b>";
    FCharacter *character = getCharacter(name);
    if (character) {
        html = html.arg(character->genderColor().name()).arg(character->getUrl()).arg(name);
    } else {
        html = html.arg(FCharacter::genderColors[FCharacter::GENDER_OFFLINE_UNKNOWN].name()).arg(getCharacterUrl(name)).arg(name);
    }
    return html;
}

/**
Tell the server that we wish to join the given channel.
 */
void FSession::joinChannel(QString name) {
    wsSend(generateJsonCommandWithKeyValue("JCH", "channel", name).toStdString().c_str());
}

void FSession::createPublicChannel(QString name) {
    // [0:59 AM]>>CRC {"channel":"test"}
    wsSend(generateJsonCommandWithKeyValue("CRC", "channel", name).toStdString().c_str());
}

void FSession::createPrivateChannel(QString name) {
    // [17:24 PM]>>CCR {"channel":"abc"}
    wsSend(generateJsonCommandWithKeyValue("CCR", "channel", name).toStdString().c_str());
}

FChannel *FSession::addChannel(QString name, QString title) {
    FChannel *channel;
    if (channellist.contains(name)) {
        channel = channellist[name];
        // Ensure that the channel's title is set correctly for ad-hoc channels.
        if (name != title && channel->getTitle() != title) {
            channel->setTitle(title);
        }
        return channel;
    }
    channel = new FChannel(this, this, name, title);
    channellist[name] = channel;
    return channel;
}

FChannel *FSession::getChannel(QString name) {
    return channellist.contains(name) ? channellist[name] : 0;
}

// todo: All the web socket stuff should really go into its own class.
void FSession::connectSession() {
    debugMessage("session->connectSession()");
    if (connected) {
        return;
    }

    connected = true;
    wsready = false;

    // tcpsocket = new QTcpSocket ( this );
    tcpsocket = new QSslSocket(this);
    // tcpsocket->ignoreSslErrors();
    debugMessage("Connecting...");
    // tcpsocket->connectToHost (FLIST_CHAT_SERVER, FLIST_CHAT_SERVER_PORT);
    // tcpsocket->connectToHost (account->server->chatserver_host, account->server->chatserver_port);
    tcpsocket->connectToHostEncrypted(account->server->chatserver_host, account->server->chatserver_port);
    connect(tcpsocket, SIGNAL(connected()), this, SLOT(socketConnected()));
    connect(tcpsocket, SIGNAL(encrypted()), this, SLOT(socketConnected()));
    connect(tcpsocket, SIGNAL(readyRead()), this, SLOT(socketReadReady()));
    connect(tcpsocket, SIGNAL(errorOccurred(QAbstractSocket::SocketError)), this, SLOT(socketError(QAbstractSocket::SocketError)));
    connect(tcpsocket, SIGNAL(sslErrors(QList<QSslError>)), this, SLOT(socketSslError(QList<QSslError>)));
}

void FSession::socketConnected() {
    debugMessage("Connected.");

    tcpsocket->setSocketOption(QAbstractSocket::KeepAliveOption, true);

    // todo: this should use a better random source
    srand(QTime::currentTime().msecsTo(QTime()));
    unsigned char nonce[16];
    int i;
    for (i = 0; i < 16; i++) {
        nonce[i] = (unsigned char)rand();
    }

    QString header;
    header.asprintf(
            "GET / HTTP/1.1\r\n"
            "Upgrade: WebSocket\r\n"
            "Connection: Upgrade\r\n"
            "Host: f-list.net:%d\r\n"
            "Origin: https://%s\r\n"
            "Sec-WebSocket-Key: %s\r\n"
            "Sec-WebSocket-Version: 8\r\n"
            "\r\n",
            account->server->chatserver_port, (const char *)account->server->chatserver_host.toUtf8(), (const char *)QByteArray((const char *)nonce, 16).toBase64());

    tcpsocket->write(header.toUtf8());
    tcpsocket->flush();
}

void FSession::socketError(QAbstractSocket::SocketError error) {
    connected = false;
    emit socketErrorSignal(error);
    if (tcpsocket) {
        tcpsocket->abort();
        tcpsocket->deleteLater();
        tcpsocket = 0;
    }
}

void FSession::socketSslError(QList<QSslError> sslerrors) {
    // QMessageBox msgbox;
    QString errorstring;
    foreach (const QSslError &error, sslerrors) {
        if (!errorstring.isEmpty()) {
            errorstring += ", ";
        }
        errorstring += error.errorString();
    }
    // msgbox.critical ( this, "SSL ERROR DURING LOGIN!", errorstring );
    // messageSystem(0, errorstring, MESSAGE_TYPE_ERROR);
    QString message = QString("SSL Socket Error: %1").arg(errorstring);
    // todo: This should really display message box.
    account->ui->messageSystem(this, message, MESSAGE_TYPE_ERROR);
    debugMessage(message);
}

void FSession::socketReadReady() {
    FJsonHelper helper;

    if (!wsready) {
        QByteArray buffer = tcpsocket->readAll();
        std::string buf(socketreadbuffer);
        buf.append(buffer.begin(), buffer.end());
        if (buf.find("\r\n\r\n") == std::string::npos) {
            // debugMessage("WebSocket waiting...");
            // debugMessage(buf);
            // debugMessage("...");
            socketreadbuffer = buf;
            return;
        } else {
            debugMessage("WebSocket connected.");
            wsready = true;
            socketreadbuffer.clear();
        }
        // todo: verify "Sec-WebSocket-Accept" response
        QMap<QString, QString> valueMap;
        valueMap.insert("method", "ticket");
        valueMap.insert("ticket", account->ticket);
        valueMap.insert("account", account->getUserName());
        valueMap.insert("cname", FLIST_CLIENTID);
        valueMap.insert("cversion", FLIST_VERSIONNUM);
        valueMap.insert("character", character);
        QJsonDocument loginNode = helper.generateJsonNodesFromMap(valueMap);

        std::string idenStr = "IDN " + loginNode.toJson(QJsonDocument::Compact).toStdString();
        // debugMessage("Indentify...");
        wsSend(idenStr);
    } else {
        // debugMessage("receiving...");
        QByteArray buffer = tcpsocket->readAll();
        std::string buf(socketreadbuffer);
        buf.append(buffer.begin(), buffer.end());
        unsigned int lengthsize;
        unsigned int payloadlength;
        unsigned int headersize;
        unsigned int i;
        while (1) {
            if (buf.length() < 2) {
                break;
            }
            payloadlength = buf[1] & 0x7f;
            if (payloadlength < 126) {
                lengthsize = 0;
            } else if (payloadlength == 126) {
                lengthsize = 2;
                if (buf.length() < 4) {
                    break;
                }
                payloadlength = ((buf[2] & 0xff) << 8) | (buf[3] & 0xff);
            } else {
                lengthsize = 8;
                if (buf.length() < 10) {
                    break;
                }
                // Does not handle lengths greater than 4GB
                payloadlength = ((buf[6] & 0xff) << 24) | ((buf[7] & 0xff) << 16) | ((buf[8] & 0xff) << 8) | (buf[9] & 0xff);
            }
            if (buf[1] & 0x80) {
                headersize = lengthsize + 2 + 4;
            } else {
                headersize = lengthsize + 2;
            }
            // todo: sanity check the opcode, final fragment and reserved bits
            // if(buf != 0x81) {
            //         display error
            //         disconnect?
            // }
            if (buf.length() < headersize + payloadlength) {
                break;
            }
            std::string cmd = buf.substr(headersize, payloadlength);
            if (buf[1] & 0x80) {
                for (i = 0; i < payloadlength; i++) {
                    cmd[i] ^= buf[lengthsize + 2 + (i & 0x3)];
                }
            }
            wsRecv(cmd);
            if (buf.length() <= headersize + payloadlength) {
                buf.clear();
                break;
            } else {
                buf = buf.substr(headersize + payloadlength, buf.length() - (headersize + payloadlength));
            }
        }
        socketreadbuffer = buf;
    }
}

QString FSession::generateJsonCommandWithKeyValue(QString command, QString key, QString value) {
    FJsonHelper helper;
    QString result;

    QJsonDocument resultNode = helper.generateJsonNodeWithKeyValue(key, value);
    result = command + " " + resultNode.toJson();

    return result;
}

void FSession::wsSend(const char *command) {
    std::string cmd = command;
    wsSend(cmd);
}

void FSession::wsSend(const char *command, QJsonDocument &nodes) {
    std::string cmd = command + (" " + nodes.toJson().toStdString());
    wsSend(cmd);
}

void FSession::wsSend(std::string &input) {
    if (!connected) {
        // textEdit->append ( "Attempted to send a message, but client is disconnected." );
    } else {
        fix_broken_escaped_apos(input);
        debugMessage(">>" + input);
        QByteArray buf;
        QDataStream stream(&buf, QIODevice::WriteOnly);
        input.resize(input.length());
        // Send WS frame as a single text frame
        stream << (quint8)0x81;
        // Length of frame with mask bit sent
        if (input.length() < 126) {
            stream << (quint8)(input.length() | 0x80);
        } else if (input.length() < 0x10000) {
            stream << (quint8)(126 | 0x80);
            stream << (quint8)(input.length() >> 8);
            stream << (quint8)(input.length() >> 0);
        } else {
            // Does not handle the case if we're trying to send more than 4GB.
            stream << (quint8)(127 | 0x80);
            stream << (quint8)(0x00);
            stream << (quint8)(0x00);
            stream << (quint8)(0x00);
            stream << (quint8)(0x00);
            stream << (quint8)(input.length() >> 24);
            stream << (quint8)(input.length() >> 16);
            stream << (quint8)(input.length() >> 8);
            stream << (quint8)(input.length());
        }
        // The mask to use for this frame.
        // The spec says it should be cryptographically strong random number, but we're using a weak random source instead.
        quint8 mask[4];
        unsigned int i;

        for (i = 0; i < 4; i++) {
            mask[i] = rand() & 0xff;
            stream << mask[i];
        }

        for (i = 0; i < input.length(); i++) {
            stream << (quint8)(input[i] ^ mask[i & 0x3]);
        }
        tcpsocket->write(buf);
        tcpsocket->flush();
    }
}

void FSession::wsRecv(std::string packet) {
    debugMessage("<<" + packet);
    try {
        std::string cmd = packet.substr(0, 3);
        QJsonDocument nodes;
        QVariantMap nodeMap;
        if (packet.length() > 4) {
            nodes.fromJson(packet.substr(4, packet.length() - 4).c_str());
            nodeMap = nodes.toVariant().toMap();
        }
#define CMD(name)                   \
    if (cmd == #name) {             \
        cmd##name(packet, nodeMap); \
        return;                     \
    }
        CMD(ADL); // List of all chat operators.
        CMD(AOP); // Add a chat operator.
        CMD(DOP); // Remove a chat operator.

        CMD(SFC); // Staff report.

        CMD(CDS); // Channel description.
        CMD(CIU); // Channel invite.
        CMD(ICH); // Initial channel data.
        CMD(JCH); // Join channel.
        CMD(LCH); // Leave channel.
        CMD(RMO); // Room mode.

        CMD(LIS); // List of online characters.
        CMD(NLN); // Character is now online.
        CMD(FLN); // Character is now offline.
        CMD(STA); // Status change.

        CMD(CBU); // kick and ban character from channel.
        CMD(CKU); // Kick character from channel.

        CMD(COL); // Channel operator list.
        CMD(COA); // Channel operator add.
        CMD(COR); // Channel operator remove.

        CMD(BRO); // Broadcast message.
        CMD(SYS); // System message.

        CMD(CON); // User count.
        CMD(HLO); // Server hello.
        CMD(IDN); // Identity acknowledged.
        CMD(VAR); // Server variable.

        CMD(FRL); // Friends and bookmarks list.
        CMD(IGN); // Ignore list update.

        CMD(LRP); // Looking for RP message.
        CMD(MSG); // Channel message.
        CMD(PRI); // Private message.
        CMD(RLL); // Dice roll or bottle spin result.

        CMD(TPN); // Typing status.

        CMD(KID); // Custom kink data.
        CMD(PRD); // Profile data.

        CMD(CHA); // Channel list.
        CMD(ORS); // Open room list.

        CMD(RTB); // Real time bridge.

        CMD(ZZZ); // Debug test command.

        CMD(ERR); // Error message.

        CMD(PIN); // Ping.

        debugMessage(QString("The command '%1' was received, but is unknown and could not be processed. %2").arg(QString::fromStdString(cmd), QString::fromStdString(packet)));
    } catch (std::invalid_argument) {
        debugMessage("Server returned invalid json in its response: " + packet);
    } catch (std::out_of_range) {
        debugMessage("Server produced unexpected json without a field we expected: " + packet);
    }
}

#define COMMAND(name) void FSession::cmd##name(std::string &rawpacket, QVariantMap &nodes)

// todo: Merge common code in ADL, AOP and DOP into a separate function.
COMMAND(ADL) {
    (void)rawpacket;
    // The list of current chat-ops.
    // ADL {"ops": ["name1", "name2"]}
    QStringList childnode = nodes.value("ops").toStringList();
    int size = childnode.size();

    for (int i = 0; i < size; ++i) {
        QString op = childnode[i];
        operatorlist[op.toLower()] = op;

        if (isCharacterOnline(op)) {
            // Set flag in character
            FCharacter *character = characterlist[op];
            character->setIsChatOp(true);
        }
        account->ui->setChatOperator(this, op, true);
    }
}

COMMAND(AOP) {
    (void)rawpacket;
    // Add a character to the list of known chat-operators.
    // AOP {"character": "Viona"}
    QString op = nodes.value("character").toString();
    operatorlist[op.toLower()] = op;

    if (isCharacterOnline(op)) {
        // Set flag in character
        FCharacter *character = characterlist[op];
        character->setIsChatOp(true);
    }
    account->ui->setChatOperator(this, op, true);
}

COMMAND(DOP) {
    (void)rawpacket;
    // Remove a character from the list of  chat operators.
    // DOP {"character": "Viona"}
    QString op = nodes.value("character").toString();
    operatorlist.remove(op.toLower());

    if (isCharacterOnline(op)) {
        // Set flag in character
        FCharacter *character = characterlist[op];
        character->setIsChatOp(false);
    }
    account->ui->setChatOperator(this, op, false);
}

COMMAND(SFC) {
    // Staff report.
    // SFC {"action": actionenum, ???}
    // SFC {"action": "report", "callid": "ID?", "character": "Character Name", "logid": "LogID", "report": "Report Text"}
    // SFC {"action": "confirm", "moderator": "Character Name", "character": "Character Name"}
    // The wiki has no documentation on this command.
    QString action = nodes.value("action").toString();
    if (action == "report") {
        QString callid = nodes.value("callid").toString();
        QString character = nodes.value("character").toString();
        QString report = nodes.value("report").toString();
        QString logid;
        QString logstring;
        try {
            logid = nodes.value("logid").toString();
            logstring = QString("<a href=\"https://www.f-list.net/fchat/getLog.php?log=%1\" ><b>Log~</b></a> | ").arg(logid);
        } catch (std::out_of_range) {
            logstring.clear();
        }
        QString message = QString("<b>STAFF ALERT!</b> From %1<br />"
                                  "%2<br />"
                                  "%3"
                                  "<a href=\"#CSA-%4\"><b>Confirm Alert</b></a>")
                                  .arg(character)
                                  .arg(report)
                                  .arg(logstring)
                                  .arg(callid);
        account->ui->messageSystem(this, message, MESSAGE_TYPE_REPORT);
    } else if (action == "confirm") {
        QString moderator = nodes.value("moderator").toString();
        QString character = nodes.value("character").toString();
        QString message = QString("<b>%1</b> is handling <b>%2</b>'s report.").arg(moderator).arg(character);
        account->ui->messageSystem(this, message, MESSAGE_TYPE_REPORT);
    } else {
        debugMessage(QString("Received a staff report with an action of '%1' but we don't know how to handle it. %2").arg(action).arg(QString::fromStdString(rawpacket)));
    }
}

COMMAND(CDS) {
    // Channel description.
    // CDS {"channel": "Channel Name", "description": "Description Text"}
    QString channelname = nodes.value("channel").toString();
    QString description = nodes.value("description").toString();
    FChannel *channel = channellist.value(channelname);
    if (!channel) {
        debugMessage(QString("[SERVER BUG] Was give the description for the channel '%1', but the channel '%1' is unknown (or never joined).  %2")
                             .arg(channelname)
                             .arg(QString::fromStdString(rawpacket)));
        return;
    }
    channel->setDescription(description);
    account->ui->setChannelDescription(this, channelname, description);
}

COMMAND(CIU) {
    // Channel invite.
    // CIU {"sender": "Character Name", "name": "Channel Name", "title": "Channel Title"}
    QString charactername = nodes.value("sender").toString();
    QString channelname = nodes.value("name").toString();
    QString channeltitle = nodes.value("title").toString();
    FCharacter *character = getCharacter(charactername);
    if (!character) {
        debugMessage(QString("Received invite to the channel '%1' (title '%2') by '%3' but the character '%3' does not exist. %4")
                             .arg(channelname)
                             .arg(channeltitle)
                             .arg(charactername)
                             .arg(QString::fromStdString(rawpacket)));
        return;
    }
    // todo: Filter the title for problem BBCode characters.
    QString message = makeMessage(QString("/me has invited you to [session=%1]%2[/session].*").arg(channeltitle).arg(channelname), charactername, character, 0,
                                  "<font color=\"yellow\"><b>Channel invite:</b></font> ", "");
    account->ui->messageSystem(this, message, MESSAGE_TYPE_CHANNEL_INVITE);
}

COMMAND(ICH) {
    (void)rawpacket;
    // Initial channel/room data. Sent when this session joins a channel.
    // ICH { "users": [object], "channnel": "Channel Name", "title": "Channel Title", "mode": enum }
    // Where object is: {"identity":"Character Name"}
    // Where enum is: "ads", "chat", "both"
    // ICH {"users": [{"identity": "Shadlor"}, {"identity": "Bunnie Patcher"}, {"identity": "DemonNeko"}, {"identity": "Desbreko"}, {"identity": "Robert Bell"}, {"identity":
    // "Jayson"}, {"identity": "Valoriel Talonheart"}, {"identity": "Jordan Costa"}, {"identity": "Skip Weber"}, {"identity": "Niruka"}, {"identity": "Jake Brian Purplecat"},
    // {"identity": "Hexxy"}], "channel": "Frontpage", "mode": "chat"}
    FChannel *channel;
    QString channelname = nodes.value("channel").toString();
    ;
    // debugMessage(QString("ICH: channel: %1").arg(channelname));
    QString channelmode = nodes.value("mode").toString();
    ;
    // debugMessage(QString("ICH: mode: %1").arg(channelmode));
    QList<QVariant> childnode = nodes.value("users").toList();
    // debugMessage(QString("ICH: users: #%1").arg(childnode.size()));
    QString channeltitle;
    if (channelname.startsWith("ADH-")) {
        try {
            // todo: Wiki says to expect "title" in the ICH command, but none is received
            channeltitle = nodes.value("title").toString();
        } catch (std::out_of_range) {
            channeltitle = channelname;
        }
    } else {
        channeltitle = channelname;
    }
    channel = addChannel(channelname, channeltitle);
    account->ui->addChannel(this, channelname, channeltitle);
    if (channelmode == "both") {
        channel->mode = CHANNEL_MODE_BOTH;
    } else if (channelmode == "ads") {
        channel->mode = CHANNEL_MODE_ADS;
    } else if (channelmode == "chat") {
        channel->mode = CHANNEL_MODE_CHAT;
    } else {
        channel->mode = CHANNEL_MODE_UNKNOWN;
        debugMessage("[SERVER BUG]: Received unknown channel mode '" + channelmode + "' for channel '" + channelname + "'. <<" + QString::fromStdString(rawpacket));
    }
    account->ui->setChannelMode(this, channelname, channel->mode);

    int size = childnode.size();
    debugMessage("Initial channel data for '" + channelname + "', charcter count: " + QString::number(size));
    for (int i = 0; i < size; i++) {
        QMap characterNode = childnode.at(i).toMap();
        QString charactername = characterNode.value("identity").toString();
        if (!isCharacterOnline(charactername)) {
            debugMessage("[SERVER BUG] Server gave us a character in the channel user list that we don't know about yet: " + charactername.toStdString() + ", " + rawpacket);
            continue;
        }
        debugMessage("Add character '" + charactername + "' to channel '" + channelname + "'.");
        channel->addCharacter(charactername, false);
    }
    account->ui->notifyChannelReady(this, channelname);
}

COMMAND(JCH) {
    (void)rawpacket;
    // Join channel notification. Sent when a character joins a channel.
    // JCH {"character": {"identity": "Character Name"}, "channel": "Channel Name", "title": "Channel Title"}
    FChannel *channel;
    QString channelname = nodes.value("channel").toString();
    QString channeltitle;
    QMap<QString, QVariant> characterNode = nodes.value("character").toMap();
    QString charactername = characterNode.value("identity").toString();
    if (channelname.startsWith("ADH-")) {
        channeltitle = nodes.value("title").toString();
    } else {
        channeltitle = channelname;
    }
    channel = addChannel(channelname, channeltitle);
    account->ui->addChannel(this, channelname, channeltitle);
    channel->addCharacter(charactername, true);
    if (charactername == character) {
        channel->join();
    }
}

COMMAND(LCH) {
    // Leave a channel. Sent when a character leaves a channel.
    // LCH {"channel": "Channel Name", "character", "Character Name"}
    FChannel *channel;
    QString channelname = nodes.value("channel").toString();
    QString charactername = nodes.value("character").toString();
    channel = getChannel(channelname);
    if (!channel) {
        debugMessage("[SERVER BUG] Was told about character '" + charactername + "' leaving unknown channel '" + channelname + "'.  " + QString::fromStdString(rawpacket));
        return;
    }
    channel->removeCharacter(charactername);
    if (charactername == character) {
        channel->leave();
        // todo: ui->removeChannel() ?
    }
}

COMMAND(RMO) {
    (void)rawpacket;
    // Room mode.
    // RMO {"mode": mode_enum, "channel": "Channel Name"}
    // Where mode_enum
    QString channelname = nodes.value("channel").toString();
    QString channelmode = nodes.value("mode").toString();
    FChannel *channel = channellist[channelname];
    if (!channel) {
        // todo: Determine if RMO can be sent even if we're not in the channel in question.
        return;
    }
    QString modedescription;
    if (channelmode == "both") {
        channel->mode = CHANNEL_MODE_BOTH;
        modedescription = "chat and ads";
    } else if (channelmode == "ads") {
        channel->mode = CHANNEL_MODE_ADS;
        modedescription = "ads only";
    } else if (channelmode == "chat") {
        channel->mode = CHANNEL_MODE_CHAT;
        modedescription = "chat only";
    } else {
        debugMessage(QString("[SERVER BUG]: Received channel mode update '%1' for channel '%2'. %3").arg(channelmode).arg(channelname).arg(QString::fromStdString(rawpacket)));
        return;
    }
    QString message = "[session=%1]%2[/session]'s mode has been changed to: %3";
    message = bbcodeparser->parse(message).arg(channel->getTitle()).arg(channelname).arg(modedescription);
    account->ui->setChannelMode(this, channelname, channel->mode);
    account->ui->messageChannel(this, channelname, message, MESSAGE_TYPE_CHANNEL_MODE, true);
}

COMMAND(NLN) {
    (void)rawpacket;
    // Character is now online.
    // NLN {"identity": "Character Name", "gender": genderenum, "status": statusenum}
    // Where 'statusenum' is one of: "online"
    // Where 'genderenum' is one of: "Male", "Female",
    QString charactername = nodes.value("identity").toString();
    QString gender = nodes.value("gender").toString();
    QString status = nodes.value("status").toString();
    FCharacter *character = addCharacter(charactername);
    character->setGender(gender);
    character->setStatus(status);
    if (operatorlist.contains(charactername.toLower())) {
        character->setIsChatOp(true);
    }
    emit notifyCharacterOnline(this, charactername, true);
}

COMMAND(LIS) {
    (void)rawpacket;
    // List of online characters. This can be sent in multiple blocks.
    // LIS {"characters": [character, character]
    // Where 'character' is: ["Character Name", genderenum, statusenum, "Status Message"]
    QList<QVariant> childnode = nodes.value("characters").toList();
    int count = childnode.count();
    // debugMessage("Character list count: " + QString::number(size));
    // debugMessage(childnode.write());
    for (int i = 0; i < count; i++) {
        QList<QVariant> characternode = childnode.at(i).toList();
        // debugMessage("char #" + QString::number(i) + " : " + QString::fromStdString(characternode.write()));
        QString charactername = characternode.at(0).toString();
        // debugMessage("charactername: " + charactername);
        QString gender = characternode.at(1).toString();
        // debugMessage("gender: " + gender);
        QString status = characternode.at(2).toString();
        // debugMessage("status: " + status);
        QString statusmessage = characternode.at(3).toString();
        // debugMessage("statusmessage: " + statusmessage);
        FCharacter *character;
        character = addCharacter(charactername);
        character->setGender(gender);
        character->setStatus(status);
        character->setStatusMsg(statusmessage);
        if (operatorlist.contains(charactername.toLower())) {
            character->setIsChatOp(true);
        }
        emit notifyCharacterOnline(this, charactername, true);
    }
}

COMMAND(FLN) {
    (void)rawpacket;
    // Character is now offline.
    // FLN {"character": "Character Name"}
    QString charactername = nodes.value("character").toString();
    if (!isCharacterOnline(charactername)) {
        debugMessage("[SERVER BUG] Received offline message for '" + charactername + "' but they're not listed as being online.");
        return;
    }
    // Iterate over all channels and make the chracacter leave them if they're present.
    for (QHash<QString, FChannel *>::const_iterator iter = channellist.begin(); iter != channellist.end(); iter++) {
        if ((*iter)->isCharacterPresent(charactername)) {
            (*iter)->removeCharacter(charactername);
        }
    }
    emit notifyCharacterOnline(this, charactername, false);
    removeCharacter(charactername);
}

COMMAND(STA) {
    // Status change.
    // STA {"character": "Character Name", "status": statusenum, "statusmsg": "Status message"}
    QString charactername = nodes.value("character").toString();
    QString status = nodes.value("status").toString();
    QString statusmessage;
    FCharacter *character = getCharacter(charactername);
    if (!character) {
        debugMessage(QString("[SERVER BUG] Received a status update message from the character '%1', but the character '%1' is unknown. %2")
                             .arg(charactername)
                             .arg(QString::fromStdString(rawpacket)));
        return;
    }
    character->setStatus(status);
    try {
        statusmessage = nodes.value("statusmsg").toString();
        character->setStatusMsg(statusmessage);
    } catch (std::out_of_range) {
        // Crown messages can cause there to be no statusmsg.
        /*do nothing*/
    }
    emit notifyCharacterStatusUpdate(this, charactername);
}

COMMAND(CBUCKU) {
    // CBU and CKU commands commoned up. Except for their messages, their behaviour is identical.
    FChannel *channel;
    QString channelname = nodes.value("channel").toString();
    QString charactername = nodes.value("character").toString();
    QString operatorname = nodes.value("operator").toString();
    bool banned = rawpacket.substr(0, 3) == "CBU";
    QString kicktype = banned ? "kicked and banned" : "kicked";
    channel = getChannel(channelname);
    if (!channel) {
        debugMessage(QString("[SERVER BUG] Was told about character '%1' being %4 from channel '%2' by '%3', but the channel '%2' is unknown (or never joined).  %5")
                             .arg(charactername)
                             .arg(channelname)
                             .arg(operatorname)
                             .arg(kicktype)
                             .arg(QString::fromStdString(rawpacket)));
        return;
    }
    if (!channel->isJoined()) {
        debugMessage(QString("[SERVER BUG] Was told about character '%1' being %4 from channel '%2' by '%3', but this session is no longer joined with channel '%2'.  %5")
                             .arg(charactername)
                             .arg(channelname)
                             .arg(operatorname)
                             .arg(kicktype)
                             .arg(QString::fromStdString(rawpacket)));
        return;
    }
    if (!channel->isCharacterPresent(charactername)) {
        debugMessage(QString("[SERVER BUG] Was told about character '%1' being %4 from channel '%2' by '%3', but '%1' is not present in the channel.  %5")
                             .arg(charactername)
                             .arg(channelname)
                             .arg(operatorname)
                             .arg(kicktype)
                             .arg(QString::fromStdString(rawpacket)));
        return;
    }
    if (!channel->isCharacterOperator(operatorname) && !isCharacterOperator(operatorname)) {
        debugMessage(QString("[SERVER BUG] Was told about character '%1' being %4 from channel '%2' by '%3', but '%3' is not a channel operator or a server operator!  %5")
                             .arg(charactername)
                             .arg(channelname)
                             .arg(operatorname)
                             .arg(kicktype)
                             .arg(QString::fromStdString(rawpacket)));
    }
    QString message = QString("<b>%1</b> has %4 <b>%2</b> from %3.").arg(operatorname).arg(charactername).arg(channel->getTitle()).arg(kicktype);
    if (charactername == character) {
        account->ui->messageChannel(this, channelname, message, banned ? MESSAGE_TYPE_KICKBAN : MESSAGE_TYPE_KICK, true, true);
        channel->removeCharacter(charactername);
        channel->leave();
    } else {
        account->ui->messageChannel(this, channelname, message, banned ? MESSAGE_TYPE_KICKBAN : MESSAGE_TYPE_KICK, channel->isCharacterOperator(character), false);
        channel->removeCharacter(charactername);
    }
}

COMMAND(CBU) {
    // Kick and ban character from channel.
    // CBU {"operator": "Character Name", "channel": "Channel Name", "character": "Character Name"}
    cmdCBUCKU(rawpacket, nodes);
}

COMMAND(CKU) {
    // Kick character from channel.
    // CKU {"operator": "Character Name", "channel": "Channel Name", "character": "Character Name"}
    cmdCBUCKU(rawpacket, nodes);
}

COMMAND(COL) {
    // Channel operator list.
    // COL {"channel":"Channel Name", "oplist":["Character Name"]}
    QString channelname = nodes.value("channel").toString();
    FChannel *channel = getChannel(channelname);
    if (!channel) {
        debugMessage(QString("[SERVER BUG] Was given the channel operator list for the channel '%1', but the channel '%1' is unknown (or never joined).  %2")
                             .arg(channelname, QString::fromStdString(rawpacket)));
        return;
    }
    QStringList childnode = nodes.value("oplist").toStringList();
    // todo: clear the existing operator list first
    int size = childnode.size();
    for (int i = 0; i < size; i++) {
        QString charactername = childnode.at(i);
        channel->addOperator(charactername);
    }
}

COMMAND(COA) {
    // Channel operator add.
    // COA {"channel":"Channel Name", "character":"Character Name"}
    QString channelname = nodes.value("channel").toString();
    QString charactername = nodes.value("character").toString();
    FChannel *channel = getChannel(channelname);
    if (!channel) {
        debugMessage(QString("[SERVER BUG] Was told to add '%2' as a channel operator for channel '%1', but the channel '%1' is unknown (or never joined).  %3")
                             .arg(channelname)
                             .arg(charactername)
                             .arg(QString::fromStdString(rawpacket)));
        return;
    }
    // todo: Print a BUG message about adding operators twice.
    channel->addOperator(charactername);
}

COMMAND(COR) {
    // Channel operator remove.
    // COR {"channel":"Channel Name", "character":"Character Name"}
    QString channelname = nodes.value("channel").toString();
    QString charactername = nodes.value("character").toString();
    FChannel *channel = getChannel(channelname);
    if (!channel) {
        debugMessage(QString("[SERVER BUG] Was told to remove '%2' from the list of channel operators for channel '%1', but the channel '%1' is unknown (or never joined).  %3")
                             .arg(channelname)
                             .arg(charactername)
                             .arg(QString::fromStdString(rawpacket)));
        return;
    }
    // todo: Print a bug message  if we're removing someone not on the operator list and skip the whole removal process.
    channel->removeOperator(charactername);
}

COMMAND(BRO) {
    (void)rawpacket;
    // Broadcast message.
    // BRO {"message": "Message Text"}
    QString message = nodes.value("message").toString();
    account->ui->messageAll(this, QString("<b>Broadcast message:</b> %1").arg(bbcodeparser->parse(message)), MESSAGE_TYPE_SYSTEM);
}

COMMAND(SYS) {
    (void)rawpacket;
    // System message
    // SYS {"message": "Message Text"}
    QString message = nodes.value("message").toString();
    account->ui->messageSystem(this, QString("<b>System message:</b> %1").arg(message), MESSAGE_TYPE_SYSTEM);
}

COMMAND(CON) {
    (void)rawpacket;
    // User count.
    // CON {"count": usercount}
    QString count = nodes.value("count").toString();
    // The message doesn't handle the plural case correctly, but that only happens on the test server.
    account->ui->messageSystem(this, QString("%1 users are currently connected.").arg(count), MESSAGE_TYPE_LOGIN);
}

COMMAND(HLO) {
    (void)rawpacket;
    // Server hello. Sent during the initial connection traffic after identification.
    // HLO {"message": "Server Message"}
    QString message = nodes.value("message").toString();
    account->ui->messageSystem(this, QString("<b>%1</b>").arg(message), MESSAGE_TYPE_LOGIN);
    foreach (QString channelname, autojoinchannels) {
        joinChannel(channelname);
    }
}

COMMAND(IDN) {
    // Identity acknowledged.
    // IDN {"character": "Character Name"}
    QString charactername = nodes.value("character").toString();

    QString message = QString("<b>%1</b> connected.").arg(charactername);
    account->ui->messageSystem(this, message, MESSAGE_TYPE_LOGIN);
    if (charactername != character) {
        debugMessage(
                QString("[SERVER BUG] Received IDN response for '%1', but this session is for '%2'. %3").arg(charactername).arg(character).arg(QString::fromStdString(rawpacket)));
    }
}

COMMAND(VAR) {
    (void)rawpacket;
    // Server variable
    // VAR {"value":value, "variable":"Variable_Name"}
    QString value = nodes.value("value").toString();
    QString variable = nodes.value("variable").toString();
    servervariables[variable] = value;
    debugMessage(QString("Server variable: %1 = '%2'").arg(variable).arg(value));
    // todo: Parse and store variables of interest.
}

COMMAND(FRL) {
    (void)rawpacket;
    // Friends and bookmarks list.
    // FRL {"characters":["Character Name"]}
    qDebug() << "FRL ->" << nodes;
    QStringList childnode = nodes.value("characters").toStringList();
    int size = childnode.size();
    for (int i = 0; i < size; i++) {
        QString charactername = childnode.at(i);
        if (!friendslist.contains(charactername)) {
            friendslist.append(charactername);
            // debugMessage(QString("Added friend '%1'.").arg(charactername));
        }
    }
}

COMMAND(IGN) {
    // Ignore list update. Behaviour of the command depends on the "action" field.
    // IGN {"action": "init", "characters":, ["Character Name"]}
    // IGN {"action": "add", "characters":, "Character Name"}
    // IGN {"action": "delete", "characters":, "Character Name"}
    QString action = nodes.value("action").toString();
    if (action == "init") {
        QStringList childnode = nodes.value("characters").toStringList();
        ignorelist.clear();
        int size = childnode.size();
        for (int i = 0; i < size; i++) {
            QString charactername = childnode.at(i);
            if (!ignorelist.contains(charactername, Qt::CaseInsensitive)) {
                ignorelist.append(charactername.toLower());
            }
        }
        emit notifyIgnoreList(this);
    } else if (action == "add") {
        QString charactername = nodes.value("character").toString();
        if (ignorelist.contains(charactername, Qt::CaseInsensitive)) {
            debugMessage(
                    QString("[BUG] Was told to add '%1' to our ignore list, but '%1' is already on our ignore list. %2").arg(charactername).arg(QString::fromStdString(rawpacket)));
        } else {
            ignorelist.append(charactername.toLower());
        }
        emit notifyIgnoreAdd(this, charactername);
    } else if (action == "delete") {
        QString charactername = nodes.value("character").toString();
        if (!ignorelist.contains(charactername, Qt::CaseInsensitive)) {
            debugMessage(QString("[BUG] Was told to remove '%1' from our ignore list, but '%1' is not on our ignore list. %2")
                                 .arg(charactername)
                                 .arg(QString::fromStdString(rawpacket)));
        } else {
            ignorelist.removeAll(charactername.toLower());
        }
        emit notifyIgnoreRemove(this, charactername);
    } else {
        debugMessage(QString("[SERVER BUG] Received ignore command(IGN) but the action '%1' is unknown. %2").arg(action).arg(QString::fromStdString(rawpacket)));
        return;
    }
}

QString FSession::makeMessage(QString message, QString charactername, FCharacter *character, FChannel *channel, QString prefix, QString postfix) {
    QString characterprefix;
    QString characterpostfix;
    if (isCharacterOperator(charactername)) {
        // todo: choose a different icon
        characterprefix += "<img src=\":/images/auction-hammer.png\" />";
    }
    if (isCharacterOperator(charactername) || (channel && channel->isCharacterOperator(charactername))) {
        characterprefix += "<img src=\":/images/auction-hammer.png\" />";
    }
    QString messagebody;
    if (message.startsWith("/me 's ")) {
        messagebody = message.mid(7, -1);
        messagebody = bbcodeparser->parse(messagebody);
        characterpostfix += "'s"; // todo: HTML escape
    } else if (message.startsWith("/me ")) {
        messagebody = message.mid(4, -1);
        messagebody = bbcodeparser->parse(messagebody);
    } else if (message.startsWith("/warn ")) {
        messagebody = message.mid(6, -1);
        messagebody = QString("<span id=\"warning\">%1</span>").arg(bbcodeparser->parse(messagebody));
    } else {
        messagebody = bbcodeparser->parse(message);
    }
    QString messagefinal;
    if (character != NULL) {
        messagefinal = QString("<b><a style=\"color: %1\" href=\"%2\">%3%4%5</a></b> %6")
                               .arg(character->genderColor().name())
                               .arg(character->getUrl())
                               .arg(characterprefix)
                               .arg(charactername) // todo: HTML escape
                               .arg(characterpostfix)
                               .arg(messagebody);
    } else {
        messagefinal = QString("<b><a style=\"color: %1\" href=\"%2\">%3%4%5</a></b> %6")
                               .arg(FCharacter::genderColors[FCharacter::GENDER_OFFLINE_UNKNOWN].name())
                               .arg(getCharacterUrl(charactername))
                               .arg(characterprefix)
                               .arg(charactername) // todo: HTML escape
                               .arg(characterpostfix)
                               .arg(messagebody);
    }
    if (message.startsWith("/me")) {
        messagefinal = QString("%1<i>*%2</i>%3").arg(prefix).arg(messagefinal).arg(postfix);
    } else {
        messagefinal = QString("%1%2%3").arg(prefix).arg(messagefinal).arg(postfix);
    }

    return messagefinal;
}

COMMAND(LRP) {
    // Looking for RP message.
    // LRP {"channel": "Channel Name", "character": "Character Name", "message": "Message Text"}
    QString channelname = nodes.value("channel").toString();
    QString charactername = nodes.value("character").toString();
    QString message = nodes.value("message").toString();
    FChannel *channel = getChannel(channelname);
    FCharacter *character = getCharacter(charactername);
    if (!channel) {
        debugMessage(QString("[SERVER BUG] Received an RP ad from the channel '%1' but the channel '%1' is unknown. %2").arg(channelname).arg(QString::fromStdString(rawpacket)));
        return;
    }
    if (!character) {
        debugMessage(QString("[SERVER BUG] Received an RP ad from '%1' in the channel '%2' but the character '%1' is unknown. %3")
                             .arg(charactername)
                             .arg(channelname)
                             .arg(QString::fromStdString(rawpacket)));
        // todo: Allow it to be displayed anyway?
        return;
    }
    if (isCharacterIgnored(charactername)) {
        // Ignore message
        return;
    }
    QString messagefinal = makeMessage(message, charactername, character, channel, "<font color=\"green\"><b>Roleplay ad by</b></font> ", "");
    FMessage fmessage(messagefinal, MESSAGE_TYPE_RPAD);
    fmessage.toChannel(channelname).fromChannel(channelname).fromCharacter(charactername).fromSession(sessionid);
    account->ui->messageMessage(fmessage);
}

COMMAND(MSG) {
    // Channel message.
    // MSG {"channel": "Channel Name", "character": "Character Name", "message": "Message Text"}
    QString channelname = nodes.value("channel").toString();
    QString charactername = nodes.value("character").toString();
    QString message = nodes.value("message").toString();
    FChannel *channel = getChannel(channelname);
    FCharacter *character = getCharacter(charactername);
    if (!channel) {
        debugMessage(QString("[SERVER BUG] Received a message from the channel '%1' but the channel '%1' is unknown. %2").arg(channelname).arg(QString::fromStdString(rawpacket)));
        return;
    }
    if (!character) {
        debugMessage(QString("[SERVER BUG] Received a message from '%1' in the channel '%2' but the character '%1' is unknown. %3")
                             .arg(charactername)
                             .arg(channelname)
                             .arg(QString::fromStdString(rawpacket)));
        // todo: Allow it to be displayed anyway?
        return;
    }
    if (isCharacterIgnored(charactername)) {
        // Ignore message
        return;
    }
    QString messagefinal = makeMessage(message, charactername, character, channel);
    FMessage fmessage(messagefinal, MESSAGE_TYPE_CHAT);
    fmessage.toChannel(channelname).fromChannel(channelname).fromCharacter(charactername).fromSession(sessionid);
    account->ui->messageMessage(fmessage);
}

COMMAND(PRI) {
    // Private message.
    // PRI {"character": "Character Name", "message": "Message Text"}
    QString charactername = nodes.value("character").toString();
    QString message = nodes.value("message").toString();
    FCharacter *character = getCharacter(charactername);
    if (!character) {
        debugMessage(QString("[SERVER BUG] Received a message from the character '%1', but the character '%1' is unknown. %2")
                             .arg(charactername)
                             .arg(QString::fromStdString(rawpacket)));
        // todo: Allow it to be displayed anyway?
        return;
    }
    if (isCharacterIgnored(charactername)) {
        // Ignore message
        return;
    }

    QString messagefinal = makeMessage(message, charactername, character);
    account->ui->addCharacterChat(this, charactername);
    FMessage fmessage(messagefinal, MESSAGE_TYPE_CHAT);
    fmessage.toCharacter(charactername).fromCharacter(charactername).fromSession(sessionid);
    account->ui->messageMessage(fmessage);
}

COMMAND(RLL) {
    // Dice roll or bottle spin result.
    // Bottle spin:
    // RLL {"type": "bottle", "message": "Message Text", "channel": "Channel Name", "character": "Character Name", "target": "Character Name"}
    // Dice roll:
    // RLL {"type": "dice", "message": "Message Text", "channel": "Channel Name", "character": "Character Name", "results": [number], "endresult": number}
    // PM roll:
    // RLL {"type": "dice", "message": "Message Text", "recipient": "Partner Character Name", "character": "Rolling Character Name", "endresult": number, "rolls": ["2d20", "3",
    // "-1d10"], "results": [number, number, number]}
    QString channelname;
    try {
        channelname = nodes.value("channel").toString();
    } catch (std::out_of_range) {
    }
    QString charactername = nodes.value("character").toString();
    if (channelname.isEmpty() && charactername == this->character) {
        // We're the character rolling in a PM, so the "recipient" field contains the character we want.
        charactername = nodes.value("recipient").toString();
    }
    QString message = nodes.value("message").toString();
    if (isCharacterIgnored(charactername)) {
        // Ignore message
        return;
    }
    if (!channelname.isEmpty()) {
        FChannel *channel = getChannel(channelname);
        // FCharacter *character = getCharacter(charactername);
        if (!channel) {
            debugMessage(QString("[SERVER BUG] Received a dice roll result from the channel '%1' but the channel '%1' is unknown. %2")
                                 .arg(channelname)
                                 .arg(QString::fromStdString(rawpacket)));
            // todo: Dump the message to console anyway?
            return;
        }
        // todo: Maybe extract character name and make it a link and colored like normal.
        account->ui->messageChannel(this, channelname, bbcodeparser->parse(message), MESSAGE_TYPE_ROLL, true);
    } else {
        account->ui->addCharacterChat(this, charactername);
        QString messagefinal = bbcodeparser->parse(message);
        FMessage fmessage(messagefinal, MESSAGE_TYPE_ROLL);
        fmessage.toCharacter(charactername).fromCharacter(this->character).fromSession(sessionid);
        account->ui->messageMessage(fmessage);
    }
}

COMMAND(TPN) {
    // Typing status.
    // TPN {"status": typing_enum, "character": "Character Name"}
    // Where 'typing_enum' is one of: "typing", "paused", "clear"
    QString charactername = nodes.value("character").toString();
    QString typingstatus = nodes.value("status").toString();
    FCharacter *character = getCharacter(charactername);
    if (!character) {
        debugMessage(QString("[SERVER BUG] Received a typing status update for the character '%1' but the character '%1' is unknown. %2")
                             .arg(charactername)
                             .arg(QString::fromStdString(rawpacket)));
        return;
    }
    TypingStatus status;
    if (typingstatus == "typing") {
        status = TYPING_STATUS_TYPING;
    } else if (typingstatus == "paused") {
        status = TYPING_STATUS_PAUSED;
    } else if (typingstatus == "clear") {
        status = TYPING_STATUS_CLEAR;
    } else {
        debugMessage(QString("[SERVER BUG] Received a typing status update of '%2' for the character '%1' but the typing status '%2' is unknown. %3")
                             .arg(charactername)
                             .arg(typingstatus)
                             .arg(QString::fromStdString(rawpacket)));
        status = TYPING_STATUS_CLEAR;
    }
    account->ui->setCharacterTypingStatus(this, charactername, status);
}

COMMAND(KID) {
    // Custom kink data.
    // KID {"type": kinkdataenum, "character": "Character Name", ???}
    // KID {"type": "start", "character": "Character Name", "message": "Custom kinks of Character Name"}
    // KID {"type": "end", "character": "Character Name", "message": "End of custom kinks."}
    // KID {"type": "custom", "character": "Character Name", "key": "Key Text", "value": "Value Text"}
    // Where "kinkdataenum" is one of: "start", "end", "custom"
    QString type = nodes.value("type").toString();
    QString charactername = nodes.value("character").toString();
    FCharacter *character = getCharacter(charactername);
    if (!character) {
        debugMessage(QString("[SERVER BUG] Received custom kink data for the character '%1' but the character '%1' is unknown. %2")
                             .arg(charactername)
                             .arg(QString::fromStdString(rawpacket)));
        return;
    }
    if (type == "start") {
        character->clearCustomKinkData();
        // account->ui->notifyCharacterCustomKinkDataReset(this, charactername);
    } else if (type == "end") {
        account->ui->notifyCharacterCustomKinkDataUpdated(this, charactername);
    } else if (type == "custom") {
        QString key = nodes.value("key").toString();
        QString value = nodes.value("value").toString();
        character->addCustomKinkData(key, value);
    } else {
        debugMessage(QString("[BUG] Received custom kink data for the character '%1' with a type of '%2' but we don't know how to handle '%2'. %3")
                             .arg(charactername)
                             .arg(QString::fromStdString(rawpacket)));
    }
}

COMMAND(PRD) {
    // Profile data.
    // PRD {"type": profiledataenum, "character": "Character Name", ???}
    // PRD {"type": "start", "character": "Character Name", "message": "Profile of Character Name"}
    // PRD {"type": "end", "character": "Character Name", "message": "End of Profile"}
    // PRD {"type": "info", "character": "Character Name", "key": "Key Text", "value": "Value Text"}
    // PRD {"type": "select", "character": "Character Name", ???}
    // Where "profiledataenum" is one of: "start", "end", "info", "select"
    QString type = nodes.value("type").toString();
    QString charactername = nodes.value("character").toString();
    FCharacter *character = getCharacter(charactername);
    if (!character) {
        debugMessage(QString("[SERVER BUG] Received profile data for the character '%1' but the character '%1' is unknown. %2")
                             .arg(charactername)
                             .arg(QString::fromStdString(rawpacket)));
        return;
    }
    if (type == "start") {
        character->clearProfileData();
        // account->ui->notifyCharacterProfileDataReset(this, charactername);
    } else if (type == "end") {
        account->ui->notifyCharacterProfileDataUpdated(this, charactername);
    } else if (type == "info") {
        QString key = nodes.value("key").toString();
        QString value = nodes.value("value").toString();
        character->addProfileData(key, value);
    } else {
        // todo: "select" is referred to in the wiki but no additional detail is given.
        debugMessage(QString("[BUG] Received profile data for the character '%1' with a type of '%2' but we don't know how to handle '%2'. %3")
                             .arg(charactername)
                             .arg(QString::fromStdString(rawpacket)));
    }
}

COMMAND(CHA) {
    (void)rawpacket;
    // Channel list.
    // CHA {"channels": [{"name": "Channel Name", "characters": character_count}]}
    knownchannellist.clear();
    QList<QVariant> childnode = nodes.value("channels").toList();
    int size = childnode.size();
    for (int i = 0; i < size; i++) {
        QVariantMap channelnode = childnode.at(i).toMap();
        QString channelname = channelnode.value("name").toString();
        QString channelcountstring = channelnode.value("characters").toString();

        bool _ok = false;
        int channelcount = channelcountstring.toInt(&_ok);
        if (!_ok) {
            qDebug() << "CHA ->"
                     << "Could not convert channel count to int, original string value is:" << channelcountstring;
            channelcount = 0;
        }
        knownchannellist.append(FChannelSummary(FChannelSummary::Public, channelname, channelcount));
    }
    account->ui->updateKnownChannelList(this);
}

COMMAND(ORS) {
    (void)rawpacket;
    // Open room list.
    // CHA {"channels": [{"name": "Channel Name", "title": "Channel Title", "characters": character_count}]}
    knownopenroomlist.clear();
    QList<QVariant> childnode = nodes.value("channels").toList();
    int size = childnode.size();
    for (int i = 0; i < size; i++) {
        QVariantMap channelnode = childnode.at(i).toMap();
        QString channelname = channelnode.value("name").toString();
        QString channeltitle = channelnode.value("title").toString();
        QString channelcountstring = channelnode.value("characters").toString();

        bool _ok = false;
        int channelcount = channelcountstring.toInt(&_ok);
        if (!_ok) {
            qDebug() << "ORS ->"
                     << "Could not convert channel count to int, original string value is:" << channelcountstring;
            channelcount = 0;
        }

        knownopenroomlist.append(FChannelSummary(FChannelSummary::Private, channelname, channeltitle, channelcount));
    }
    account->ui->updateKnownOpenRoomList(this);
}

COMMAND(RTB) {
    (void)rawpacket;
    (void)nodes;
    // Real time bridge.
    // RTB {"type":typeenum, ???}
    // RTB {"type":"note", "sender": "Character Name", "subject": "Subject Text", "id":id}
    // RTB {"type":"trackadd","name":"Character Name"}
    // RTB {"type":"trackrem","name":"Character Name"}
    // RTB {"type":"friendrequest","name":"Character Name"}
    // RTB {"type":"friendadd","name":"Character Name"}
    // RTB {"type":"friendremove","name":"Character Name"}

    // todo: Determine all the RTB messages.
    QString type = nodes.value("type").toString();
    if (type == "note") {
        QString charactername = nodes.value("sender").toString();
        QString subject = nodes.value("subject").toString();
        QString id = nodes.value("id").toString();

        QString message = "Note recieved from %1: <a href=\"https://www.f-list.net/view_note.php?note_id=%2\">%3</a>";
        message = message.arg(getCharacterHtml(charactername)).arg(id).arg(subject);
        FMessage fmessage(message, MESSAGE_TYPE_NOTE);
        fmessage.toUser().toCharacter(charactername).fromCharacter(charactername).fromSession(sessionid);
        account->ui->messageMessage(fmessage);
        // account->ui->messageSystem(this, message, MESSAGE_TYPE_NOTE);
    } else if (type == "trackadd") {
        QString charactername = nodes.value("name").toString();
        if (!friendslist.contains(charactername)) {
            friendslist.append(charactername);
            // debugMessage(QString("Added friend '%1'.").arg(charactername));
        }
        QString message = "Bookmark update: %1 has been added to your bookmarks."; // todo: escape characters?
        message = message.arg(getCharacterHtml(charactername));
        FMessage fmessage(message, MESSAGE_TYPE_BOOKMARK);
        fmessage.toUser().toCharacter(charactername).fromCharacter(charactername).fromSession(sessionid);
        account->ui->messageMessage(fmessage);
        // account->ui->messageSystem(this, message, MESSAGE_TYPE_BOOKMARK);
    } else if (type == "trackrem") {
        // todo: Update bookmark list? (Removing has the complication in that bookmarks and friends aren't distinguished and multiple instances of friends may exist.)
        QString charactername = nodes.value("name").toString();
        QString message = "Bookmark update: %1 has been removed from your bookmarks."; // todo: escape characters?
        message = message.arg(getCharacterHtml(charactername));
        FMessage fmessage(message, MESSAGE_TYPE_BOOKMARK);
        fmessage.toUser().toCharacter(charactername).fromCharacter(charactername).fromSession(sessionid);
        account->ui->messageMessage(fmessage);
        // account->ui->messageSystem(this, message, MESSAGE_TYPE_BOOKMARK);
    } else if (type == "friendrequest") {
        QString charactername = nodes.value("name").toString();
        QString message =
                "Friend update: %1 has requested to be friends with one of your characters. Visit <a href=\"%2\">%2</a> to view the request on F-List."; // todo: escape characters?
        message = message.arg(charactername).arg("https://www.f-list.net/messages.php?show=friends");
        FMessage fmessage(message, MESSAGE_TYPE_FRIEND);
        fmessage.toUser().toCharacter(charactername).fromCharacter(charactername).fromSession(sessionid);
        account->ui->messageMessage(fmessage);
        // account->ui->messageSystem(this, message, MESSAGE_TYPE_FRIEND);
    } else if (type == "friendadd") {
        QString charactername = nodes.value("name").toString();
        if (!friendslist.contains(charactername)) {
            friendslist.append(getCharacterHtml(charactername));
            // debugMessage(QString("Added friend '%1'.").arg(charactername));
        }
        QString message = "Friend update: %1 has become friends with one of your characters."; // todo: escape characters?
        message = message.arg(getCharacterHtml(charactername));
        FMessage fmessage(message, MESSAGE_TYPE_FRIEND);
        fmessage.toUser().toCharacter(charactername).fromCharacter(charactername).fromSession(sessionid);
        account->ui->messageMessage(fmessage);
        // account->ui->messageSystem(this, message, MESSAGE_TYPE_FRIEND);
    } else if (type == "friendremove") {
        // todo: Update bookmark/friend list? (Removing has the complication in that bookmarks and friends aren't distinguished and multiple instances of friends may exist.)
        QString charactername = nodes.value("name").toString();
        QString message = "Friend update: %1 was removed as a friend from one of your characters."; // todo: escape characters?
        message = message.arg(getCharacterHtml(charactername));
        FMessage fmessage(message, MESSAGE_TYPE_FRIEND);
        fmessage.toUser().toCharacter(charactername).fromCharacter(charactername).fromSession(sessionid);
        account->ui->messageMessage(fmessage);
        // account->ui->messageSystem(this, message, MESSAGE_TYPE_FRIEND);
    } else {
        QString message = "Received an unknown/unhandled Real Time Bridge message of type \"%1\". Received packet: %2"; // todo: escape characters?
        message = message.arg(type).arg(rawpacket.c_str());
        account->ui->messageSystem(this, message, MESSAGE_TYPE_ERROR);
    }
    // debugMessage(QString("Real time bridge: %1").arg(QString::fromStdString(rawpacket)));
}

COMMAND(ZZZ) {
    (void)rawpacket;
    // Debug test command.
    // ZZZ {"message": "???"}
    // This command is not documented.
    QString message = nodes.value("message").toString();
    account->ui->messageSystem(this, QString("<b>Debug Reply:</b> %1").arg(message), MESSAGE_TYPE_SYSTEM);
}

COMMAND(ERR) {
    // Error message.
    // ERR {"number": error_number, "message": "Error Message"}

    FJsonHelper helper;

    QString errornumberstring = nodes.value("number").toString();
    QString errormessage = nodes.value("message").toString();
    QString message = QString("<b>Error %1: </b> %2").arg(errornumberstring).arg(errormessage);
    account->ui->messageSystem(this, message, MESSAGE_TYPE_ERROR);
    bool ok;
    int errornumber = errornumberstring.toInt(&ok);
    if (!ok) {
        debugMessage(QString("Received an error message but could not convert the error number to an integer. Error number '%1', error message '%2' : %3")
                             .arg(errornumberstring)
                             .arg(errormessage)
                             .arg(QString::fromStdString(rawpacket)));
        return;
    }
    // Handle special cases.
    // todo: Parse the error and pass along to the UI for more informative feedback.
    switch (errornumber) {
        case 34: // Error 34 is not in the wiki, but the existing code sends out another identification if it is received.
            {
                QMap<QString, QString> valueMap;
                valueMap.insert("method", "ticket");
                valueMap.insert("ticket", account->ticket);
                valueMap.insert("account", account->getUserName());
                valueMap.insert("cname", FLIST_CLIENTID);
                valueMap.insert("cversion", FLIST_VERSIONNUM);
                valueMap.insert("character", character);
                QJsonDocument loginNode = helper.generateJsonNodesFromMap(valueMap);
                std::string idenStr = "IDN " + loginNode.toJson(QJsonDocument::Compact).toStdString();
                // debugMessage("Indentify...");
                wsSend(idenStr);
                break;
            }
        default:
            break;
    }
}

COMMAND(PIN) {
    (void)rawpacket;
    (void)nodes;
    // debugMessage("Ping!");
    std::string cmd = "PIN";
    wsSend(cmd);
}

// todo: Lots of duplicated between sendChannelMessage() and sendChannelAdvertisement() that can be refactored into a common function.
void FSession::sendChannelMessage(QString channelname, QString message) {
    FJsonHelper helper;

    // Confirm channel is known, joined and has the right permissions.
    FChannel *channel = getChannel(channelname);
    if (!channel) {
        account->ui->messageSystem(this, QString("Tried to send a message to '%1' but the channel is unknown or has never been joined. Message: %2").arg(channelname).arg(message),
                                   MESSAGE_TYPE_FEEDBACK);
        return;
    }
    if (!channel->isJoined()) {
        account->ui->messageSystem(this, QString("Tried to send a message to '%1' but you are not currently in the channel. Message: %2").arg(channelname).arg(message),
                                   MESSAGE_TYPE_FEEDBACK);
        return;
    }
    if (channel->mode == CHANNEL_MODE_ADS) {
        account->ui->messageSystem(this, QString("Tried to send a message to '%1' but the channel only allows advertisements. Message: %2").arg(channelname).arg(message),
                                   MESSAGE_TYPE_FEEDBACK);
        return;
    }
    QMap<QString, QString> valueMap;
    valueMap.insert("channel", channelname);
    valueMap.insert("message", message);
    QJsonDocument messageNode = helper.generateJsonNodesFromMap(valueMap);
    wsSend("MSG", messageNode);
    // Escape HTML characters.
    message.replace('&', "&amp;").replace('<', "&lt;").replace('>', "&gt;");
    // Send the message to the UI now.
    QString messagefinal = makeMessage(message, character, getCharacter(character), channel);
    FMessage fmessage(messagefinal, MESSAGE_TYPE_CHAT);
    fmessage.toChannel(channelname).fromChannel(channelname).fromCharacter(this->character).fromSession(sessionid);
    account->ui->messageMessage(fmessage);
}

void FSession::sendChannelAdvertisement(QString channelname, QString message) {
    FJsonHelper helper;

    // Confirm channel is known, joined and has the right permissions.
    FChannel *channel = getChannel(channelname);
    if (!channel) {
        account->ui->messageSystem(this, QString("Tried to send a message to '%1' but the channel is unknown or has never been joined. Message: %2").arg(channelname).arg(message),
                                   MESSAGE_TYPE_FEEDBACK);
        return;
    }
    if (!channel->isJoined()) {
        account->ui->messageSystem(this, QString("Tried to send a message to '%1' but you are not currently in the channel. Message: %2").arg(channelname).arg(message),
                                   MESSAGE_TYPE_FEEDBACK);
        return;
    }
    if (channel->mode == CHANNEL_MODE_CHAT) {
        account->ui->messageSystem(this, QString("Tried to send a message to '%1' but the channel does not allow advertisements. Message: %2").arg(channelname).arg(message),
                                   MESSAGE_TYPE_FEEDBACK);
        return;
    }
    QMap<QString, QString> valueMap;
    valueMap.insert("channel", channelname);
    valueMap.insert("message", message);
    QJsonDocument messageNode = helper.generateJsonNodesFromMap(valueMap);
    wsSend("LRP", messageNode);
    // Escape HTML characters.
    message.replace('&', "&amp;").replace('<', "&lt;").replace('>', "&gt;");
    // Send the message to the UI now.
    QString messagefinal = makeMessage(message, character, getCharacter(character), channel, "<font color=\"green\"><b>Roleplay ad by</font> ", "");
    FMessage fmessage(messagefinal, MESSAGE_TYPE_RPAD);
    fmessage.toChannel(channelname).fromChannel(channelname).fromCharacter(this->character).fromSession(sessionid);
    account->ui->messageMessage(fmessage);
}

void FSession::sendCharacterMessage(QString charactername, QString message) {
    FJsonHelper helper;

    // Confirm character is known, online and we are not ignoring them.
    if (!isCharacterOnline(charactername)) {
        account->ui->messageSystem(this, QString("Tried to send a message to '%1' but they are offline or unknown. Message: %2").arg(charactername).arg(message),
                                   MESSAGE_TYPE_FEEDBACK);
        return;
    }
    if (isCharacterIgnored(charactername)) {
        account->ui->messageSystem(this, QString("Tried to send a message to '%1' but YOU are ignoring them. Message: %2").arg(charactername).arg(message), MESSAGE_TYPE_FEEDBACK);
        return;
    }
    // Make packet and send it.
    QMap<QString, QString> valueMap;
    valueMap.insert("recipient", charactername);
    valueMap.insert("message", message);
    QJsonDocument messageNode = helper.generateJsonNodesFromMap(valueMap);
    wsSend("PRI", messageNode);
    // Escape HTML characters.
    // todo: use a proper function
    message.replace('&', "&amp;").replace('<', "&lt;").replace('>', "&gt;");
    // Send the message to the UI now.
    QString messagefinal = makeMessage(message, this->character, getCharacter(this->character));
    FMessage fmessage(messagefinal, MESSAGE_TYPE_CHAT);
    fmessage.toCharacter(charactername).fromCharacter(this->character).fromSession(sessionid);
    account->ui->messageMessage(fmessage);
}

void FSession::sendChannelLeave(QString channelname) {
    FJsonHelper helper;
    QJsonDocument nodes = helper.generateJsonNodeWithKeyValue("channel", channelname);
    wsSend("LCH", nodes);
}

void FSession::sendConfirmStaffReport(QString callid) {
    FJsonHelper helper;
    QMap<QString, QString> valueMap;
    valueMap.insert("action", "confirm");
    valueMap.insert("moderator", character);
    valueMap.insert("callid", callid);
    QJsonDocument nodes = helper.generateJsonNodesFromMap(valueMap);

    wsSend("SFC", nodes);
}

void FSession::sendIgnoreAdd(QString character) {
    FJsonHelper helper;
    QMap<QString, QString> valueMap;

    character = character.toLower();

    valueMap.insert("character", character);
    valueMap.insert("action", "add");
    QJsonDocument nodes = helper.generateJsonNodesFromMap(valueMap);

    wsSend("IGN", nodes);
}

void FSession::sendIgnoreDelete(QString character) {
    FJsonHelper helper;
    QMap<QString, QString> valueMap;

    character = character.toLower();

    valueMap.insert("character", character);
    valueMap.insert("action", "delete");
    QJsonDocument nodes = helper.generateJsonNodesFromMap(valueMap);

    wsSend("IGN", nodes);
}

void FSession::sendStatus(QString status, QString statusmsg) {
    FJsonHelper helper;
    QMap<QString, QString> valueMap;

    valueMap.insert("status", status);
    valueMap.insert("statusmsg", statusmsg);
    QJsonDocument nodes = helper.generateJsonNodesFromMap(valueMap);

    wsSend("STA", nodes);
}

void FSession::sendCharacterTimeout(QString character, int minutes, QString reason) {
    FJsonHelper helper;
    QMap<QString, QString> valueMap;

    valueMap.insert("character", character);
    valueMap.insert("time", QString::number(minutes));
    valueMap.insert("reason", reason);
    QJsonDocument nodes = helper.generateJsonNodesFromMap(valueMap);

    wsSend("TMO", nodes);
}

void FSession::sendTypingNotification(QString character, TypingStatus status) {
    QString statusText = "clear";
    switch (status) {
        case TYPING_STATUS_CLEAR:
            statusText = "clear";
            break;
        case TYPING_STATUS_PAUSED:
            statusText = "paused";
            break;
        case TYPING_STATUS_TYPING:
            statusText = "typing";
            break;
    }

    FJsonHelper helper;
    QMap<QString, QString> valueMap;

    valueMap.insert("status", statusText);
    valueMap.insert("character", character);
    QJsonDocument nodes = helper.generateJsonNodesFromMap(valueMap);

    wsSend("TPN", nodes);
}

void FSession::sendDebugCommand(QString payload) {
    FJsonHelper helper;
    QJsonDocument node = helper.generateJsonNodeWithKeyValue("command", payload);

    wsSend("ZZZ", node);
}

void FSession::kickFromChannel(QString channel, QString character) {
    FJsonHelper helper;
    QMap<QString, QString> valueMap;

    valueMap.insert("character", character);
    valueMap.insert("channel", channel);
    QJsonDocument nodes = helper.generateJsonNodesFromMap(valueMap);

    wsSend("CKU", nodes);
}

void FSession::kickFromChat(QString character) {
    FJsonHelper helper;
    QJsonDocument node = helper.generateJsonNodeWithKeyValue("character", character);

    wsSend("KIK", node);
}

void FSession::banFromChannel(QString channel, QString character) {
    FJsonHelper helper;
    QMap<QString, QString> valueMap;

    valueMap.insert("character", character);
    valueMap.insert("channel", channel);
    QJsonDocument nodes = helper.generateJsonNodesFromMap(valueMap);

    wsSend("CBU", nodes);
}

void FSession::banFromChat(QString character) {
    FJsonHelper helper;
    QJsonDocument node = helper.generateJsonNodeWithKeyValue("character", character);

    wsSend("ACB", node);
}

void FSession::unbanFromChannel(QString channel, QString character) {
    FJsonHelper helper;
    QMap<QString, QString> valueMap;

    valueMap.insert("character", character);
    valueMap.insert("channel", channel);
    QJsonDocument nodes = helper.generateJsonNodesFromMap(valueMap);

    wsSend("CUB", nodes);
}

void FSession::unbanFromChat(QString character) {
    FJsonHelper helper;
    QJsonDocument node = helper.generateJsonNodeWithKeyValue("character", character);

    wsSend("UNB", node);
}

void FSession::setRoomIsPublic(QString channel, bool isPublic) {
    FJsonHelper helper;
    QMap<QString, QString> valueMap;

    valueMap.insert("channel", channel);
    valueMap.insert("status", isPublic ? "public" : "private");
    QJsonDocument nodes = helper.generateJsonNodesFromMap(valueMap);

    wsSend("RST", nodes);
}

void FSession::inviteToChannel(QString channel, QString character) {
    FJsonHelper helper;
    QMap<QString, QString> valueMap;

    valueMap.insert("character", character);
    valueMap.insert("channel", channel);
    QJsonDocument nodes = helper.generateJsonNodesFromMap(valueMap);

    wsSend("CIU", nodes);
}

void FSession::giveChanop(QString channel, QString character) {
    FJsonHelper helper;
    QMap<QString, QString> valueMap;

    valueMap.insert("character", character);
    valueMap.insert("channel", channel);
    QJsonDocument nodes = helper.generateJsonNodesFromMap(valueMap);

    wsSend("COA", nodes);
}

void FSession::takeChanop(QString channel, QString character) {
    FJsonHelper helper;
    QMap<QString, QString> valueMap;

    valueMap.insert("character", character);
    valueMap.insert("channel", channel);
    QJsonDocument nodes = helper.generateJsonNodesFromMap(valueMap);

    wsSend("COR", nodes);
}

void FSession::giveGlobalop(QString character) {
    FJsonHelper helper;
    QJsonDocument node = helper.generateJsonNodeWithKeyValue("character", character);

    wsSend("AOP", node);
}

void FSession::takeGlobalop(QString character) {
    FJsonHelper helper;
    QJsonDocument node = helper.generateJsonNodeWithKeyValue("character", character);

    wsSend("DOP", node);
}

void FSession::giveReward(QString character) {
    FJsonHelper helper;
    QJsonDocument node = helper.generateJsonNodeWithKeyValue("character", character);

    wsSend("RWD", node);
}

void FSession::requestChannelBanList(QString channel) {
    FJsonHelper helper;
    QJsonDocument node = helper.generateJsonNodeWithKeyValue("channel", channel);

    wsSend("CBL", node);
}

void FSession::requestChanopList(QString channel) {
    FJsonHelper helper;
    QJsonDocument node = helper.generateJsonNodeWithKeyValue("channel", channel);

    wsSend("COL", node);
}

void FSession::killChannel(QString channel) {
    FJsonHelper helper;
    QJsonDocument node = helper.generateJsonNodeWithKeyValue("channel", channel);

    wsSend("KIC", node);
}

void FSession::broadcastMessage(QString message) {
    FJsonHelper helper;
    QJsonDocument node = helper.generateJsonNodeWithKeyValue("message", message);

    wsSend("BRO", node);
}

void FSession::setChannelDescription(QString channelname, QString description) {
    FJsonHelper helper;
    QMap<QString, QString> valueMap;

    valueMap.insert("channel", channelname);
    valueMap.insert("description", description);
    QJsonDocument nodes = helper.generateJsonNodesFromMap(valueMap);

    wsSend("CDS", nodes);
}

void FSession::setChannelMode(QString channel, ChannelMode mode) {
    FJsonHelper helper;
    QMap<QString, QString> valueMap;

    valueMap.insert("channel", channel);
    valueMap.insert("mode", ChannelModeEnum.valueToKey(mode));
    QJsonDocument nodes = helper.generateJsonNodesFromMap(valueMap);

    wsSend("RMO", nodes);
}

void FSession::setChannelOwner(QString channel, QString newOwner) {
    FJsonHelper helper;
    QMap<QString, QString> valueMap;

    valueMap.insert("channel", channel);
    valueMap.insert("character", newOwner);
    QJsonDocument nodes = helper.generateJsonNodesFromMap(valueMap);

    wsSend("CSO", nodes);
}

void FSession::spinBottle(QString channel) {
    FJsonHelper helper;
    QMap<QString, QString> valueMap;

    valueMap.insert("channel", channel);
    valueMap.insert("dice", "bottle");
    QJsonDocument nodes = helper.generateJsonNodesFromMap(valueMap);

    wsSend("RLL", nodes);
}

void FSession::rollDiceChannel(QString channel, QString dice) {
    FJsonHelper helper;
    QMap<QString, QString> valueMap;

    valueMap.insert("channel", channel);
    valueMap.insert("dice", dice);
    QJsonDocument nodes = helper.generateJsonNodesFromMap(valueMap);

    wsSend("RLL", nodes);
}

void FSession::rollDicePM(QString recipient, QString dice) {
    FJsonHelper helper;
    QMap<QString, QString> valueMap;

    valueMap.insert("recipient", recipient);
    valueMap.insert("dice", dice);
    QJsonDocument nodes = helper.generateJsonNodesFromMap(valueMap);

    wsSend("RLL", nodes);
}

void FSession::requestChannels() {
    wsSend("CHA");
    wsSend("ORS");
}

void FSession::requestProfileKinks(QString character) {
    FJsonHelper helper;
    QJsonDocument node = helper.generateJsonNodeWithKeyValue("character", character);

    wsSend("PRO", node);
    wsSend("KIN", node);
}
