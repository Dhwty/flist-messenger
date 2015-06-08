#ifndef FLIST_CHANNEL_H
#define FLIST_CHANNEL_H

#include <QObject>
#include <QString>
#include <QList>
#include <QMap>
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

	bool isCharacterPresent(QString charactername) {return characterlist.contains(charactername.toLower());}
    bool isCharacterOperator(QString charactername) {return operatorlist.contains(charactername.toLower());}

	void addCharacter(QString charactername, bool notify);
	void removeCharacter(QString charactername);

	void addOperator(QString charactername);
	void removeOperator(QString charactername);

	void join();
	void leave();

    bool joined() {return _joined;}
    QString title() {return _title;}
    QString name() {return _name;}
    QString description() {return _description;}

    void setTitle(const QString &newTitle) {this->_title = newTitle;}
    void setDescription(const QString &newdescription) {_description = newdescription;}

signals:

public slots:

private:
    FSession *session;
    QString _name; //<Server name for this channel/room
    QString _title; //<Title for this room.
    QString _description; //<Long description for the channel/room.
	QMap<QString, QString> characterlist; //<List of all characters within a channel.
	QMap<QString, QString> operatorlist; //<List of all channel operators.
    bool _joined; //<Indicates if this session is currently joined with this channel.
public:
	ChannelMode mode; //<The mode of the channel.
	ChannelType type; 
};


#endif // FLIST_CHANNEL_H
