#include "flist_server.h"

#include "flist_global.h"
#include "flist_account.h"
#include "flist_character.h"

FServer::FServer(QObject *parent) :
	QObject(parent),
	chatserver_host(FLIST_CHAT_SERVER),
	chatserver_port(FLIST_CHAT_SERVER_PORT),
	accounts()
{
}

FAccount *FServer::addAccount()
{
	FAccount *account = new FAccount(this, this);
	accounts.append(account);
	return account;
}

FSession *FServer::getSession(QString sessionid)
{
	FSession *session;
	foreach(FAccount *account, accounts) {
		session = account->getSession(sessionid);
		if(session) {
			return session;
		}
	}
	return 0;
}

FCharacter *characterAdd(QString &name)
{
	
	//todo: Check if character is already in cache.
	(void) name;
	return 0;
}

FCharacter *characterGet(QString &name)
{
	//todo: Check if character is already in cache.
    (void) name;
    return 0;
}

void characterRemove(QString &name)
{
	//todo: Iterate over open accounts and sessions and see if the character is still refered to.
    (void) name;
}
