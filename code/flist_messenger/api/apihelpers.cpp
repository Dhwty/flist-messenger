#include "flist_api.h"

#include <QUrl>
#include <QByteArray>

namespace FHttpApi {

    QByteArray Endpoint::encodePostData(QHash<QString, QString> params) {
        QByteArray result;

        auto cend = params.cend();
        for (auto cit = params.cbegin(); cit != cend; ++cit) {
            QString value = cit.value();
            result.append(cit.key().toUtf8());
            if (value.length() > 0) {
                result.append('=');
                result.append(QUrl::toPercentEncoding(value, "", "+"));
            }
            result.append('&');
        }

        // remove the last &
        result.chop(1);

        return result;
    }

    QNetworkReply *Endpoint::request(QUrl u, QHash<QString, QString> params) {
        QNetworkRequest request(u);
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
        QByteArray postData = encodePostData(params);
        QNetworkReply *reply = qnam->post(request, postData);
        return reply;
    }

    BaseRequest::BaseRequest(QNetworkReply *r) : reply(r) {
        QObject::connect(reply, SIGNAL(sslErrors(QList<QSslError>)), this, SLOT(onSslError(QList<QSslError>)));
        QObject::connect(reply, SIGNAL(errorOccurred(QNetworkReply::NetworkError)), this, SLOT(onError(QNetworkReply::NetworkError)));
        QObject::connect(reply, SIGNAL(finished()), this, SLOT(onRequestFinished()));
    }

    BaseRequest::~BaseRequest() {}

    void BaseRequest::onError(QNetworkReply::NetworkError code) {
        QString error_id = QString("network_failure.%1").arg(code);
        QString error_message = reply->errorString();

        emit failed(error_id, error_message);
    }

} // namespace FHttpApi
