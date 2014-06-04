#ifndef DATA_H
#define DATA_H

#include <QHash>
#include <QList>
#include <QStringList>
#include <QPair>
#include <QDateTime>

namespace FHttpApi
{

/* Data to be included in a response.
 * The response may contain more but will not contain less.*/
enum DataType
{
	Data_None                   = 0,
	Data_Characters             = 0x0001,
	Data_Friends                = 0x0002,
	Data_Bookmarks              = 0x0004,
	Data_IncomingFriendRequests = 0x0008,
	Data_OutgoingFriendRequests = 0x0010,
	Data_Kinks                  = 0x0020,
	Data_Description            = 0x0040,
	Data_CustomKinks            = 0x0080,
	Data_Images                 = 0x0100,
	Data_Infotags               = 0x0200,
	Data_KinkGroups             = 0x0400,
	Data_Groups                 = 0x0800,
	Data_ListItems              = 0x1000,

	Data_FriendRequests   = Data_IncomingFriendRequests | Data_OutgoingFriendRequests,
	Data_AllWatches       = Data_Bookmarks | Data_Friends,
	Data_GetTicketDefault = Data_Bookmarks | Data_Characters,
	Data_QuickProfile     = Data_Infotags  | Data_CustomKinks
};
Q_DECLARE_FLAGS(DataTypes, DataType)


struct Ticket
{
	QString ticket;
	QString name;
	QString password;

	Ticket(QString &t, QString &n);
};

struct Friendship
{
	QString from;
	QString to;
};

struct TicketResponse
{
	DataTypes available;
	Ticket *ticket;
	QString defaultCharacter;
	QStringList characters;
	QStringList bookmarks;
	QList<Friendship> friendships;
};

struct FriendRequest
{
	int id;
	QString from;
	QString to;
};

struct FriendBookmarkResponse
{
	DataTypes available;
	QStringList bookmarks;
	QList<Friendship> friends;
	QList<FriendRequest> friendrequests;
};

struct Character
{
	int id;
	QString name;
};

struct CustomKink
{
	QString name;
	QString description;
};

struct CharacterImage
{
	int id;
	QString extension;
	int width, height;
	QString hash;
	QDateTime uploaded;
};

struct CharacterData : Character
{
	DataTypes available;
	QDateTime created, updated;
	QStringList kinks;
	QList<CustomKink> customKinks;
	QString description;
	QList<CharacterImage> images;
	QHash<QString, QString> infoTags;
};

struct CharacterMemo
{
	Character character;
	QDateTime updated;
	QString message;
};
}
Q_DECLARE_OPERATORS_FOR_FLAGS(FHttpApi::DataTypes)
#endif // DATA_H
