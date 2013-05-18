#ifndef FLIST_NETWORK_H
#define FLIST_NETWORK_H

#include<QtNetwork/QTcpSocket>
#include "../../libjson/libJSON.h"
#include "../../libjson/Source/NumberToString.h"
#include "../flist_messenger.h"


class flist_messenger;

class FNetwork : QTcpSocket
{
    Q_OBJECT
public:
    FNetwork(flist_messenger* parent);
    void tryLogin(QString& username, QString& charName, QString& loginTicket);
    void fixBrokenEscapedApos ( std::string &data );
    void sendWS(std::string& out);

public slots:
    void readReadyOnSocket();
    void connectedToSocket();
    void socketError ( QAbstractSocket::SocketError socketError );

    void sendPing();
    void sendDebug(QString& command);
    void sendJoinChannel(QString& channel);
    void sendLeaveChannel(QString& channel);
    void sendSetStatus(QString& status, QString& message);
    void sendIgnoreAdd(QString& character);
    void sendIgnoreDelete(QString& character);
    void sendChannelsRequest();
    void sendProomsRequest();
    void sendChannelKick(QString& channel, QString& character);
    void sendChannelBan(QString& channel, QString& character);
    void sendKick(QString& character);
    void sendBan(QString& character);
    void sendMakeRoom(QString& channel);
    void sendRestrictRoom(QString& channel, QString& restriction);
    void sendChannelInvite(QString& channel, QString& character);
    void sendChannelAddOp(QString& channel, QString& character);
    void sendChannelDeleteOp(QString& channel, QString& character);
    void sendAddOp(QString& character);
    void sendDeleteOp(QString& character);
    void sendReward(QString& character);
    void sendChannelUnban(QString& channel, QString& character);
    void sendRequestBanlist(QString& channel);
    void sendSetDescription(QString& channel, QString& description);
    void sendRequestCoplist(QString& channel);
    void sendTimeout(QString& character, int time, QString& reason);
    void sendUnban(QString& character);
    void sendCreateChannel(QString& channel);
    void sendKillChannel(QString& channel);
    void sendBroadcast(QString& broadcast);
    void sendSetMode(QString& channel, QString& mode);
    void sendRoll(QString& channel, QString& dice);
    void sendChatMessage(QString& channel, QString& message);
    void sendPrivateMessage(QString& character, QString& message);
    void sendAdvert(QString& channel, QString& message);
    void sendConfirmReport(QString& moderator, QString& callid);
    void sendTypingChanged(QString& character, QString& status);
    void sendRequestCharacterInfo(QString& character);
    void sendReport(QString& character, QString& logid, QString& report);

private:
    void setupSocket();
    bool isBrokenEscapedApos ( std::string const &data, std::string::size_type n );
    void parseCommand(QString& input);
    flist_messenger* mainprog;
    bool connected;
    bool doingWS;
    std::string networkBuffer;

    QString username;
    QString charname;
    QString loginTicket;
};

#endif // FLIST_NETWORK_H
