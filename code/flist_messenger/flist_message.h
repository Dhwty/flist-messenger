#ifndef FLIST_MESSAGE_H
#define FLIST_MESSAGE_H

#include "flist_enums.h"
#include <QExplicitlySharedDataPointer>
#include <QString>
#include <QStringList>
#include <QDateTime>

class FMessageData;

class FMessage
{
public:
	FMessage();
	FMessage(QString message, MessageType messagetype);
	FMessage(const FMessage &);
	FMessage &operator=(const FMessage &);
	~FMessage();


	FMessage &toUser(bool notify = true, bool console = true);
	FMessage &toBroadcast(bool broadcast = true);

	FMessage &toChannel(QString channelname);
	FMessage &toCharacter(QString charactername);

	FMessage &fromSession(QString sessionid);
	FMessage &fromChannel(QString channelname);
	FMessage &fromCharacter(QString charactername);

	QString getPlainTextMessage();
	QString getFormattedMessage();
	QString getMessage();
	MessageType getMessageType();
	bool getConsole();
	bool getNotify();
	bool getBroadcast();
	QStringList &getDestinationChannelList();
	QStringList &getDestinationCharacterList();
	QString getSessionID();
	QString getSourceChannel();
	QString getSourceCharacter();

	QDateTime getTimeStamp();

private:
	QExplicitlySharedDataPointer<FMessageData> data;
};

#endif // FLIST_MESSAGE_H
