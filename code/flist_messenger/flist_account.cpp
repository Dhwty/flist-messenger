#include "flist_account.h"
#include "flist_global.h"
#include "flist_session.h"

FAccount::FAccount(QObject *parent, FServer *server) : QObject(parent), username(), password(), valid(false), ticketvalid(false), server(server), ui(0) {}

void FAccount::loginSslErrors(QList<QSslError> sslerrors) {
    qDebug() << sslerrors;
    // todo: handle SSL error and pass on error message, or suppress if appropriate
    (void)sslerrors;
}

void FAccount::onLoginError(QString error_id, QString error_message) {
    debugMessage("account->loginHttps() error!");
    emit loginError(this, "Login Error", QString("%1 (%2)").arg(error_message, error_id));
}

void FAccount::loginHandle() {
    debugMessage("account->loginHandle()");
    loginReply->deleteLater();

    ticket = loginReply->ticket->ticket;
    delete loginReply->ticket;
    ticketvalid = true;

    defaultCharacter = loginReply->defaultCharacter;
    characterList = loginReply->characters;

    // todo: extract bookmarks
    // todo: extract friends list
    // todo: extract ignore list
    // todo: extract?

    // todo: get rid of this:
    charactersessions.append(new FSession(this, defaultCharacter, this));

    emit loginComplete(this);
    // emit ticketReady(this, ticket);
}

void FAccount::loginStart() {
    debugMessage("account->loginStart()");
    ticketvalid = false;

    loginReply = fapi->getTicket(username, password);
    connect(loginReply, SIGNAL(sslErrors(QList<QSslError>)), this, SLOT(loginSslErrors(QList<QSslError>)));
    connect(loginReply, SIGNAL(failed(QString, QString)), this, SLOT(onLoginError(QString, QString)));
    connect(loginReply, SIGNAL(succeeded()), this, SLOT(loginHandle()));
}

void FAccount::loginUserPass(QString user, QString pass) {
    debugMessage("account->loginUserPass()");
    username = user;
    password = pass;
    valid = true;
    loginStart();
}

FSession *FAccount::getSession(QString sessionid) {
    // debugMessage("account->getSession()");
    int i;
    // Find existing session, if any.
    for (i = 0; i < charactersessions.length(); i++) {
        if (charactersessions[i]->sessionid == sessionid) {
            return charactersessions[i];
        }
    }
    return 0;
}

FSession *FAccount::getSessionByCharacter(QString character) {
    // debugMessage("account->getSession()");
    int i;
    // Find existing session, if any.
    for (i = 0; i < charactersessions.length(); i++) {
        if (charactersessions[i]->character == character) {
            return charactersessions[i];
        }
    }
    return 0;
}

FSession *FAccount::addSession(QString charactername) {
    // Find existing session for this character, and return it if found.
    FSession *session = getSessionByCharacter(charactername);
    if (session) {
        return session;
    }
    // No session found; create it.
    debugMessage("account->addSession() [new]");
    session = new FSession(this, charactername, this);
    charactersessions.append(session);
    return session;
}
