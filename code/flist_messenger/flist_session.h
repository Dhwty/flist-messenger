#ifndef FLIST_SESSION_H
#define FLIST_SESSION_H

#include <QObject>
#include <QString>
#include <QTcpSocket>

class FAccount;
class FCharacter;
class JSONNode;

class FSession : public QObject
{
Q_OBJECT
public:
	explicit FSession(FAccount *account, QString &character, QObject *parent = 0);
	~FSession();

	void connectSession();
	
	void wsSend(std::string &data);
	void wsRecv(std::string packet);

	bool isCharacterOnline(QString name) {return characterlist.contains(name);}
	FCharacter *getCharacter(QString name) {return characterlist[name];}

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
	QList<QString> operatorlist; //<List of all known characters that are operators (stored in lower case).

private:


	bool wsready;
	std::string socketreadbuffer;

#define COMMAND(name) void cmd##name(std::string &rawpacket, JSONNode &nodes)
	COMMAND(ADL);
	COMMAND(AOP);
	COMMAND(DOP);
	COMMAND(PIN);
#undef COMMAND
	
};

#endif // FLIST_SESSION_H
