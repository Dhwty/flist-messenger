#include "flist_account.h"
#include "flist_global.h"
#include "../libjson/libJSON.h"
#include "../libjson/Source/NumberToString.h"

#include "flist_session.h"


FAccount::FAccount(QObject *parent, FServer *server) :
	QObject(parent),
	username(),
	password(),
	valid(false),
	ticketvalid(false),
	server(server)
{
}


void FAccount::loginHttps()
{
	debugMessage("account->loginHttps()");
	ticketvalid = false;
	//send HTTPS request for ticket
	loginurl = QString ( "https://www.f-list.net/json/getApiTicket.json" );
        //loginurl.addQueryItem("secure", "yes");
        //loginurl.addQueryItem("account", username);
        //loginurl.addQueryItem("password", password);
	loginparam = QUrlQuery();
	loginparam.addQueryItem("secure", "yes");
	loginparam.addQueryItem("account", username);
	loginparam.addQueryItem("password", password);
	QNetworkRequest request( loginurl );
        //request.setHeader(QNetworkRequest::ContentTypeHeader, "application/octet-stream");
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
        QByteArray postData;
#if QT_VERSION >= 0x050000
	postData = loginparam.query(QUrl::FullyEncoded).toUtf8();
#else
	postData = loginparam.encodedQuery();
#endif
	loginreply = networkaccessmanager->post ( request, postData ); //using account.loginreply since this will replace the existing login system.
	loginreply->ignoreSslErrors();
	connect ( loginreply, SIGNAL ( finished() ), this, SLOT ( loginHandle() ) );
	connect ( loginreply, SIGNAL ( sslErrors( QList<QSslError> ) ), this, SLOT ( loginSslErrors( QList<QSslError> ) ) );
}

void FAccount::loginSslErrors( QList<QSslError> sslerrors )
{
	//todo: handle SSL error and pass on error message, or suppress if appropriate
    (void) sslerrors;
}

void FAccount::loginHandle()
{
	debugMessage("account->loginHandle()");
	//handle the HTTPS reply
	if(loginreply->error() != QNetworkReply::NoError) {
		debugMessage("account->loginHandle() error!");
		//Bad response, emit error.
		QString title = "Response Error";
                QString message = "Response error while connecting for login ticket. Error: ";
		message.append ( NumberToString::_uitoa<unsigned int> ( ( unsigned int ) loginreply->error() ).c_str() );
                
		emit loginError(this, title, message);
		return;
	}

	QByteArray respbytes = loginreply->readAll();

	loginreply->deleteLater();
        std::string response ( respbytes.begin(), respbytes.end() );
        JSONNode respnode = libJSON::parse ( response );
        JSONNode childnode = respnode.at ( "error" );

        if (childnode.as_string() != "") {
		QString title = "Login Error";
		QString message = ("Error from server: " + childnode.as_string()).c_str();
		emit loginError(this, title, message);
                return;
        }

        JSONNode subnode = respnode.at ("ticket");

	ticketvalid = true;
	ticket = subnode.as_string().c_str();

        subnode = respnode.at("default_character");
	defaultCharacter = subnode.as_string().c_str();
        childnode = respnode.at("characters");
        int children = childnode.size();
	characterList.clear();
        for (int i = 0; i < children; ++i) {
                QString addchar = childnode[i].as_string().c_str();
                characterList.append(addchar);
        }

	//todo: extract bookmarks
	//todo: extract friends list
	//todo: extract ignore list
	//todo: extract?

	//todo: get rid of this:
	charactersessions.append(new FSession(this, defaultCharacter, this));

	emit loginComplete(this);
	//emit ticketReady(this, ticket);
}

void FAccount::loginStart()
{
	debugMessage("account->loginStart()");
	if(username.length() == 0 || password.length() == 0 || !valid) {
		emit loginGetLogin(this, username, password);
	}
	ticketvalid = false;
	loginHttps();
}

void FAccount::loginUserPass(QString user, QString pass)
{
	debugMessage("account->loginUserPass()");
	username = user;
	password = pass;
	valid = true;
	ticketvalid = false;
	loginStart();
}

FSession *FAccount::getSession(QString character)
{
	//debugMessage("account->getSession()");
	int i;
	//Find existing session, if any.
	for(i = 0; i < charactersessions.length(); i++) {
		if(charactersessions[i]->character == character) {
			return charactersessions[i];
		}
	}
	debugMessage("account->getSession() [new]");
	//todo: Find character and create a session.
	FSession *session = new FSession(this, character, this);
	charactersessions.append(session);
	return session;
	//todo: return NULL if none found
}
