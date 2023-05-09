#ifndef FLIST_ACCOUNT_H
#define FLIST_ACCOUNT_H

#include "flist_common.h"
#include <QObject>
#include <QString>
#include <QList>
#include <QSslError>
#include <QUrl>
#if QT_VERSION >= 0x050000
#    include <QUrlQuery>
#endif
#include <QNetworkReply>
#include "flist_api.h"

class FSession;
class FServer;
class iUserInterface;

class FAccount : public QObject {
        Q_OBJECT
    public:
        FAccount(QObject *parent, FServer *server);

        QString getUserName() { return username; }

        void setUserName(QString name) { username = name; }

        QString getPassword() { return password; }

        void setPassword(QString pass) { password = pass; }

        FSession *getSession(QString sessionid);
        FSession *getSessionByCharacter(QString character);
        FSession *addSession(QString character);

    public slots:
        void loginStart();
        void loginUserPass(QString user, QString pass);

    private slots:
        void loginSslErrors(QList<QSslError> sslerrors);
        void onLoginError(QString error_id, QString error_message);
        void loginHandle();

    signals:
        void loginError(FAccount *account, QString errortitle, QString errorsring);
        void loginComplete(FAccount *account);

        void ticketReady(FAccount *account, QString ticket);

    public:
        // todo: make this stuff private
        QUrl loginurl;        //< URL used by the ticket login process.
        QUrlQuery loginparam; //< Holds parameter passed to the URL ticket login.
        // QNetworkReply *loginreply; //< The reply yo the URL ticket login.

        QString defaultCharacter;
        QList<QString> characterList;

        QString ticket;

        QList<FSession *> charactersessions;

    private:
        QString username;
        QString password;
        bool valid;

        bool ticketvalid;

        FHttpApi::Request<FHttpApi::TicketResponse> *loginReply;

    public:
        FServer *server;
        iUserInterface *ui; //<The interface to the GUI part of the application.
};

#endif                      // FLIST_ACCOUNT_H
