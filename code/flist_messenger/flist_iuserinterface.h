#ifndef FLIST_IUSERINTERFACE_H
#define FLIST_IUSERINTERFACE_H

#include "flist_enums.h"

class FSession;
class FMessage;

class iUserInterface
{
public:
	virtual FSession *getSession(QString sessionid) = 0;

	virtual void setChatOperator(FSession *session, QString characteroperator, bool opstatus) = 0;

	virtual void openCharacterProfile(FSession *session, QString charactername) = 0;
	virtual void addCharacterChat(FSession *session, QString charactername) = 0;

	virtual void addChannel(FSession *session, QString name, QString title) = 0;
	virtual void removeChannel(FSession *session, QString name) = 0;
	virtual void addChannelCharacter(FSession *session, QString channelname, QString charactername, bool notify) = 0;
	virtual void removeChannelCharacter(FSession *session, QString channelname, QString charactername) = 0;
	virtual void setChannelOperator(FSession *session, QString channelname, QString charactername, bool opstatus) = 0;
	virtual void joinChannel(FSession *session, QString channelname) = 0;
	virtual void leaveChannel(FSession *session, QString channelname) = 0;
	virtual void setChannelDescription(FSession *session, QString channelname, QString description) = 0;
	virtual void setChannelMode(FSession *session, QString channelname, ChannelMode mode) = 0;
	virtual void notifyChannelReady(FSession *session, QString channelname) = 0;

	virtual void notifyCharacterOnline(FSession *session, QString charactername, bool online) = 0;
	virtual void notifyCharacterStatusUpdate(FSession *session, QString charactername) = 0;
	virtual void setCharacterTypingStatus(FSession *session, QString charactername, TypingStatus typingstatus) = 0;
	virtual void notifyCharacterCustomKinkDataUpdated(FSession *session, QString charactername) = 0;
	virtual void notifyCharacterProfileDataUpdated(FSession *session, QString charactername) = 0;

	virtual void messageMessage(FMessage message) = 0;
	virtual void messageMany(FSession *session, QList<QString> &channels, QList<QString> &characters, bool system, QString message, MessageType messagetype) = 0;
	virtual void messageAll(FSession *session, QString message, MessageType messagetype) = 0;
	virtual void messageChannel(FSession *session, QString channelname, QString message, MessageType messagetype, bool console = false, bool notify = false) = 0;
	virtual void messageCharacter(FSession *session, QString charactername, QString message, MessageType messagetype) = 0;
	virtual void messageSystem(FSession *session, QString message, MessageType messagetype) = 0;

	virtual void updateKnownChannelList(FSession *session) = 0;
	virtual void updateKnownOpenRoomList(FSession *session) = 0;
};

#endif // FLIST_IUSERINTERFACE_H
