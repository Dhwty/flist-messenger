#ifndef FLIST_SESSION_H
#define FLIST_SESSION_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QTcpSocket>

#include "flist_channelsummary.h"

class FAccount;
class FChannel;
class FCharacter;
class JSONNode;

class FSession : public QObject
{
Q_OBJECT
public:
	explicit FSession(FAccount *account, QString &character, QObject *parent = 0);
	~FSession();

	void connectSession();
	
	void wsSend(const char *command);
	void wsSend(const char *command, JSONNode &nodes);
	void wsSend(std::string &data);
	void wsRecv(std::string packet);

	bool isCharacterOnline(QString name) {return characterlist.contains(name);}
	bool isCharacterOperator(QString name) {return operatorlist.contains(name);}
	bool isCharacterIgnored(QString name) {return ignorelist.contains(name.toLower(), Qt::CaseInsensitive);}
	FCharacter *addCharacter(QString name);
	FCharacter *getCharacter(QString name) {return characterlist.contains(name) ? characterlist[name] : 0;}

	void joinChannel(QString name);
	FChannel *addChannel(QString name, QString title);
	FChannel *getChannel(QString name);

signals:
	void socketErrorSignal(QAbstractSocket::SocketError);
	//void wsRecv(std::string data);
	void processCommand(std::string rawinput, std::string cmd, JSONNode &nodes);
	void recvMessage(QString type, QString session, QString chan, QString sender, QString message);

public slots:
	void socketConnected();
	void socketError(QAbstractSocket::SocketError);
	void socketReadReady();

public:
	bool connected;
	FAccount *account;
	QString character;

	QTcpSocket *tcpsocket;

	QHash<QString, FCharacter *> characterlist; //< List of all known characters on the server/session.
	QList<QString> friendslist; //<List of friends for this session's character.
	QList<QString> bookmarklist; //<List of friends for this session's character.
	QList<QString> operatorlist; //<List of all known characters that are chat operators (stored in lower case).
	QStringList ignorelist; //<List of all characters that are being ignored.
	QHash<QString, FChannel *> channellist; //<List of channels that this session has joined (or was previously joined to).
	QStringList autojoinchannels; //<List of channels the client should join upon connecting.
	QHash<QString, QString> servervariables; //<List of variables as reported by the server.

	QList<FChannelSummary> knownchannellist; //<List of known public channels, as reported by the server.
	QList<FChannelSummary> knownopenroomlist; //<List of known open rooms, as reported by the server.

private:


	bool wsready;
	std::string socketreadbuffer;

#define COMMAND(name) void cmd##name(std::string &rawpacket, JSONNode &nodes)
	COMMAND(ADL);
	COMMAND(AOP);
	COMMAND(DOP);

	COMMAND(CDS);
	COMMAND(ICH);
	COMMAND(JCH);
	COMMAND(LCH);

	COMMAND(LIS);
	COMMAND(NLN);
	COMMAND(FLN);
	COMMAND(STA);

	COMMAND(CBUCKU);
	COMMAND(CBU);
	COMMAND(CKU);

	COMMAND(COL);
	COMMAND(COA);
	COMMAND(COR);
	
	COMMAND(BRO);
	COMMAND(SYS);

	COMMAND(CON);
	COMMAND(HLO);
	COMMAND(IDN);
	COMMAND(VAR);

	COMMAND(FRL);
	COMMAND(IGN);

	QString makeMessage(QString message, QString charactername, FCharacter *character, FChannel *channel = 0, QString prefix = "", QString postfix = "");
	COMMAND(LRP);
	COMMAND(MSG);
	COMMAND(PRI);
	COMMAND(RLL);

	COMMAND(CHA);
	COMMAND(ORS);

	COMMAND(RTB);

	COMMAND(PIN);
#undef COMMAND
	
};

#endif // FLIST_SESSION_H
