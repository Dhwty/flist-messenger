#ifndef FLIST_API_H
#define FLIST_API_H

#include <QObject>
#include <QFlags>
#include <QHash>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QUrl>

#include "api/data.h"

namespace FHttpApi
{

	class BaseRequest : public QObject
	{
		Q_OBJECT

	public:
		BaseRequest(QNetworkReply *r);
		virtual ~BaseRequest();

	signals:
		void succeeded();
		void failed(QString error_id, QString error_message);

	public slots:
		virtual void onRequestFinished() = 0;
		virtual void onSslError(const QList<QSslError> &errors) { (void)errors; }
		virtual void onError(QNetworkReply::NetworkError code);

	protected:
		QNetworkReply *reply;
	};

	template<typename T>
	class Request : public BaseRequest, public T
	{
	public:
		Request(QNetworkReply *r) : BaseRequest(r) { }
		virtual ~Request() { }

		T *data() { return (T*)this; }
	};

	class Endpoint
	{
	public:
		Endpoint(QNetworkAccessManager *n) : qnam(n) { }
		virtual ~Endpoint() { };

		typedef const QString &crQString;
		typedef const Ticket *cpTicket;

		/* Acceptable additional information:
		 * Data_Friends, Data_Bookmarks, Data_Characters
		 * */
		virtual Request<TicketResponse> *getTicket(crQString username, crQString password,
															 DataTypes additionalInfo = Data_GetTicketDefault) = 0;

//		/* Acceptable data types:
//		 * Data_Friends, Data_Bookmarks, Data_IncomingFriendRequests, Data_OutgoingFriendRequests
//		 */
//		virtual Request<FriendBookmarkResponse> *getFriendsBookmarks(cpTicket t, DataTypes which = Data_AllWatches ) = 0;
//		virtual Request<void> *addBookmark(cpTicket t, crQString target) = 0;
//		virtual Request<void> *delBookmark(cpTicket t, crQString target) = 0;
//		virtual Request<void> *addFriend(cpTicket t, crQString from, crQString to) = 0;
//		virtual Request<void> *remFriend(cpTicket t, crQString from, crQString to) = 0;
//		virtual Request<void> *acceptFriendRequest(cpTicket t, int request_id) = 0;
//		virtual Request<void> *cancelFriendRequest(cpTicket t, int request_id) = 0;

//		virtual Request<QList<Character> > *getCharacterList(cpTicket t) = 0;

//		/* Acceptable data types:
//		 * Data_Kinks, Data_Description, Data_CustomKinks, Data_Images, Data_Infotags
//		 */
//		virtual Request<CharacterData> *getCharacterData(cpTicket t, crQString name, DataTypes d = Data_QuickProfile) = 0;
//		virtual Request<CharacterMemo> *getCharacterMemo(cpTicket t, int idCharacter) = 0;
//		virtual Request<void> *setCharacterMemo(cpTicket t, int idCharacter, crQString memo);

//		virtual Request *getIgnoreList(cpTicket t) = 0;
//		virtual Request *addIgnore(cpTicket t, int idCharacter) = 0;
//		virtual Request *addIgnore(cpTicket t, crQString character) = 0;
//		virtual Request *removeIgnore(cpTicket t, crQString character) = 0;

//		virtual Request *chatReport(cpTicket t, crQString sender, crQString reported_character, crQString channel, crQString log, crQString report_text) = 0;

//		virtual Request *getPublicInfo(DataTypes d) = 0;

	protected:
		QNetworkAccessManager *qnam;

		QNetworkReply *request(QUrl u, QHash<QString,QString> param);
	};
}

#endif // FLIST_API_H
