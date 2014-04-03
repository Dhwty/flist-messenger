#ifndef FLIST_CHANNEL_H
#define FLIST_CHANNEL_H

#include <QObject>
#include <QString>
#include <QList>
#include "flist_enums.h"

class FSession;

class FChannel : public QObject
{
	Q_OBJECT
public:
	enum ChannelType {
		CHANTYPE_NORMAL,
		CHANTYPE_ADHOC,
		CHANTYPE_PM,
		CHANTYPE_CONSOLE,
		CHANTYPE_MAX
	};

public:
	explicit FChannel(QObject *parent, FSession *session, QString name, QString title);

	bool isCharacterPresent(QString charactername) {return characterlist.contains(charactername);}
	bool isCharacterOperator(QString charactername) {return operatorlist.contains(charactername);}
	//todo: Figure out a better function name than 'isJoined'.
	bool isJoined() {return joined;}

	void addCharacter(QString charactername, bool notify);
	void removeCharacter(QString charactername);

	void addOperator(QString charactername);
	void removeOperator(QString charactername);

	void join();
	void leave();

	QString getTitle() {return title;}
	void setTitle(QString title) {this->title = title;}

	QString getDescription() {return description;}
	void setDescription(QString &newdescription) {description = newdescription;}

signals:

public slots:

public:
	//todo: Make the following private.
	FSession *session;
	QString name; //<Server name for this channel/room
	QString title; //<Title for this room.
	QString description; //<Long description for the channel/room.
	QList<QString> characterlist; //<List of all characters within a channel.
	QList<QString> operatorlist; //<List of all channel operators.
	bool joined; //<Indicates if this session is currently joined with this channel.
	ChannelMode mode; //<The mode of the channel.
	ChannelType type; 
};


#endif // FLIST_CHANNEL_H
