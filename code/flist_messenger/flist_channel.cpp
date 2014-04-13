#include "flist_channel.h"
#include "flist_session.h"
#include "flist_account.h"
#include "flist_iuserinterface.h"

FChannel::FChannel(QObject *parent, FSession *session, QString name, QString title) :
	QObject(parent),
	session(session),
	name(name),
	title(title),
	characterlist(),
	operatorlist(),
	joined(true),
	mode(CHANNEL_MODE_BOTH)
{
	if(name.startsWith("ADH-")) {
		type = CHANTYPE_ADHOC;
	} else {
		type = CHANTYPE_NORMAL;
	}
}

void FChannel::addCharacter(QString charactername, bool notify) {
	QString lowername = charactername.toLower();
	if(!characterlist.contains(lowername) || characterlist.value(lowername).isEmpty()) {
		characterlist[lowername] = charactername;
	}
	session->account->ui->addChannelCharacter(session, name, charactername, notify);
}
void FChannel::removeCharacter(QString charactername) {
	characterlist.remove(charactername.toLower());
	session->account->ui->removeChannelCharacter(session, name, charactername);
} 

void FChannel::addOperator(QString charactername) {
	QString lowername = charactername.toLower();
	if(!operatorlist.contains(lowername) || operatorlist.value(lowername).isEmpty()) {
		operatorlist[lowername] = charactername;
	}
	session->account->ui->setChannelOperator(session, name, charactername, true);	
}
void FChannel::removeOperator(QString charactername) {
	operatorlist.remove(charactername.toLower());
	session->account->ui->setChannelOperator(session, name, charactername, false);	
}


void FChannel::join()
{
	joined = true;
	session->account->ui->joinChannel(session, name);
}

void FChannel::leave()
{
	joined = false;
	characterlist.clear();
	operatorlist.clear();
	session->account->ui->leaveChannel(session, name);	
}
