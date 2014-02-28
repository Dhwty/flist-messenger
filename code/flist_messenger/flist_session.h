#ifndef FLIST_SESSION_H
#define FLIST_SESSION_H

#include <QObject>
#include <QString>
#include <QTcpSocket>

class FAccount;

class FSession : public QObject
{
Q_OBJECT
public:
	explicit FSession(FAccount *account, QString &character, QObject *parent = 0);
	~FSession();

	void connectSession();
	
	void wsSend(std::string &data);

signals:
	void socketErrorSignal(QAbstractSocket::SocketError);
	void wsRecv(std::string data);

public slots:
	void socketConnected();
	void socketError(QAbstractSocket::SocketError);
	void socketReadReady();

public:
	bool connected;
	FAccount *account;
	QString character;

	QTcpSocket *tcpsocket;

private:


	bool wsready;
	std::string socketreadbuffer;

};

#endif // FLIST_SESSION_H
