#include "endpoint_v1.h"
#include "../libjson/libJSON.h"
#include "../libjson/Source/NumberToString.h"
#include <string>

namespace FHttpApi {

Endpoint_v1::Endpoint_v1(QNetworkAccessManager *n) : Endpoint(n)
{
}

Request<TicketResponse> *Endpoint_v1::getTicket(
		crQString username,
		crQString password,
		DataTypes additionalInfo)
{
	(void)additionalInfo;
	QString loginurl( "https://www.f-list.net/json/getApiTicket.json" );
	QHash<QString,QString> params;
	params.insert("secure","yes");
	params.insert("account", username);
	params.insert("password", password);

	QNetworkReply *reply = request(loginurl, params);
	return new TicketRequest(reply, username, password);
}

Endpoint_v1::TicketRequest::TicketRequest(QNetworkReply *qnr, QString username, QString password) :
	Request<FHttpApi::TicketResponse>(qnr), _un(username), _p(password) { }

Endpoint_v1::TicketRequest::~TicketRequest() { }

namespace {

void qslFromJsonArray(JSONNode &from, QStringList &to)
{
	int children = from.size();
	for(int i = 0; i < children; i++)
	{
		QString val = from[i].as_string().c_str();
		to.append(val);
	}
}

}

void Endpoint_v1::TicketRequest::onRequestFinished()
{
	QByteArray respbytes = reply->readAll();
	reply->deleteLater();
	std::string response ( respbytes.begin(), respbytes.end() );

	JSONNode respnode = libJSON::parse ( response );
	JSONNode errnode = respnode.at ( "error" );
	if (errnode.as_string() != "") {
		emit failed(QString("server_failure"), QString(errnode.as_string().c_str()));
		return;
	}

	resp->available = FHttpApi::Data_GetTicketDefault;

	JSONNode subnode = respnode.at ("ticket");
	ticket->ticket = subnode.as_string().c_str();
	ticket->name = _un;
	ticket->password = _p;

	subnode = respnode.at("default_character");
	defaultCharacter = subnode.as_string().c_str();

	subnode = respnode.at("characters");
	qslFromJsonArray(subnode, characters);

	subnode = respnode.at("bookmarks");
	qslFromJsonArray(subnode, bookmarks);

	emit succeeded();
}

} // namespace FHttpApi
