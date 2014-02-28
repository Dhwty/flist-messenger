#ifndef FLIST_ACCOUNT_H
#define FLIST_ACCOUNT_H

#include "flist_common.h"
#include <QObject>
#include <QString>
#include <QList>
#include <QSslError>
#include <QUrl>
#if QT_VERSION >= 0x050000
#include <QUrlQuery>
#endif
#include <QNetworkReply>

class FSession;

class FAccount : public QObject
{
	Q_OBJECT
public:
	FAccount();
	QString &getUserName() {return username;}
	void setUserName(QString name) {username = name;}
	void setUserName(QString &name) {username = name;}
	QString &getPassword() {return password;}
	void setPassword(QString &pass) {password = pass;}
	FSession *getSession(QString &character);

public slots:
	void loginStart();
	void loginUserPass(QString &user, QString &pass);

private slots:
	void loginHttps();
	void loginSslErrors( QList<QSslError> sslerrors );
	void loginHandle();

signals:
	void loginGetLogin(FAccount *account, QString &username, QString &password);
	void loginError(FAccount *account, QString &errortitle, QString &errorsring);
	void loginComplete(FAccount *account);

	void ticketReady(FAccount *account, QString &ticket);

public:
	//todo: make this stuff private
	QUrl loginurl; //< URL used by the ticket login process.
	QUrlQuery loginparam; //< Holds parameter passed to the URL ticket login.
	QNetworkReply *loginreply; //< The reply yo the URL ticket login.

	QString defaultCharacter;
	QList<QString> characterList;

	QString ticket;

	QList<FSession *> charactersessions;

private:
	QString username;
	QString password;
	bool valid;

	bool ticketvalid;

};

#endif // FLIST_ACCOUNT_H
