#ifndef FLIST_IUSERINTERFACE_H
#define FLIST_IUSERINTERFACE_H

class FSession;

//todo: move this elsewhere
enum MessageType {
	MESSAGE_TYPE_LOGIN,
	MESSAGE_TYPE_ONLINE,
	MESSAGE_TYPE_OFFLINE,
	MESSAGE_TYPE_CHANNEL_DESCRIPTION,
	MESSAGE_TYPE_JOIN,
	MESSAGE_TYPE_LEAVE,
	MESSAGE_TYPE_KICK,
	MESSAGE_TYPE_KICKBAN,
	MESSAGE_TYPE_IGNORE_UPDATE,
	MESSAGE_TYPE_SYSTEM,
	MESSAGE_TYPE_BROADCAST,
	MESSAGE_TYPE_RPAD,
	MESSAGE_TYPE_CHAT,
};

class iUserInterface
{
public:
	virtual void setChatOperator(FSession *session, QString characteroperator, bool opstatus) = 0;

	virtual void addCharacterChat(FSession *session, QString charactername) = 0;

	virtual void addChannel(FSession *session, QString name, QString title) = 0;
	virtual void removeChannel(FSession *session, QString name) = 0;
	virtual void addChannelCharacter(FSession *session, QString channelname, QString charactername, bool notify) = 0;
	virtual void removeChannelCharacter(FSession *session, QString channelname, QString charactername) = 0;
	virtual void setChannelOperator(FSession *session, QString channelname, QString charactername, bool opstatus) = 0;
	virtual void joinChannel(FSession *session, QString channelname) = 0;
	virtual void leaveChannel(FSession *session, QString channelname) = 0;
	virtual void setChannelDescription(FSession *session, QString channelname, QString description) = 0;
	virtual void notifyCharacterOnline(FSession *session, QString charactername, bool online) = 0;
	virtual void notifyIgnoreUpdate(FSession *session) = 0;
	virtual void setIgnoreCharacter(FSession *session, QString charactername, bool ignore) = 0;

	virtual void messageMany(FSession *session, QList<QString> &channels, QList<QString> &characters, bool system, QString message, MessageType messagetype) = 0;
	virtual void messageAll(FSession *session, QString message, MessageType messagetype) = 0;
	virtual void messageChannel(FSession *session, QString channelname, QString message, MessageType messagetype, bool console = false, bool notify = false) = 0;
	virtual void messageCharacter(FSession *session, QString charactername, QString message, MessageType messagetype) = 0;
	virtual void messageSystem(FSession *session, QString message, MessageType messagetype) = 0;
};

#endif // FLIST_IUSERINTERFACE_H
