#include "endpoint_v1.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <string>

namespace FHttpApi {

    Endpoint_v1::Endpoint_v1(QNetworkAccessManager *n) : Endpoint(n) {}

    Request<TicketResponse> *Endpoint_v1::getTicket(crQString username, crQString password, DataTypes additionalInfo) {
        (void)additionalInfo;
        QString loginurl("https://www.f-list.net/json/getApiTicket.json");
        QHash<QString, QString> params;
        params.insert("secure", "yes");
        params.insert("account", username);
        params.insert("password", password);

        QNetworkReply *reply = request(loginurl, params);
        return new TicketRequest(reply, username, password);
    }

    Endpoint_v1::TicketRequest::TicketRequest(QNetworkReply *qnr, QString username, QString password) : Request<FHttpApi::TicketResponse>(qnr), _un(username), _p(password) {}

    Endpoint_v1::TicketRequest::~TicketRequest() {}

    void Endpoint_v1::TicketRequest::qStringListFromJsonArray(QJsonArray &from, QStringList &to) {
        for (int i = 0; i < from.count(); i++) {
            QString val = from[i].toString();
            to.append(val);
        }
    }

    void Endpoint_v1::TicketRequest::onRequestFinished() {
        QByteArray respbytes = reply->readAll();
        reply->deleteLater();

        QJsonDocument responseDoc = QJsonDocument::fromJson(respbytes);

        // TODO: Improve handling - iterate over array and search for an object?
        if (responseDoc.isArray()) {
            emit failed(QString("server_failure"), "Unknown server response. Expected object, got array.");
            return;
        }

        QVariantMap responseMap = responseDoc.toVariant().toMap();
        QString errnode = responseMap.value("error").toString();
        if (!errnode.isEmpty()) {
            emit failed(QString("server_failure"), errnode);
            return;
        }

        available = FHttpApi::Data_GetTicketDefault;

        QString subnode = responseMap.value("ticket").toString();
        ticket = new Ticket();
        ticket->ticket = subnode;
        ticket->name = _un;
        ticket->password = _p;

        subnode = responseMap.value("default_character").toString();
        defaultCharacter = subnode;

        QJsonArray arraySubNode = responseMap.value("characters").toJsonArray();
        qStringListFromJsonArray(arraySubNode, characters);

        arraySubNode = responseMap.value("bookmarks").toJsonArray();
        qStringListFromJsonArray(arraySubNode, bookmarks);

        emit succeeded();
    }

} // namespace FHttpApi
