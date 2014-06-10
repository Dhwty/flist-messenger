#include "flist_api.h"

#include <QUrl>
#include <QByteArray>

namespace FHttpApi {

QNetworkReply *Endpoint::request(QUrl u, QHash<QString, QString> params)
{
	QUrl encParams;
	QHashIterator<QString,QString> i(params);
	while(i.hasNext())
	{
		i.next();
		encParams.addQueryItem(i.key(), i.value());
	}

	QNetworkRequest request(u);
	request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
#if QT_VERSION >= 0x050000
	QIODevice postData = encParams.query(QUrl::FullyEncoded).toUtf8();
#else
	QByteArray postData = encParams.encodedQuery();
#endif
	QNetworkReply *reply = qnam->post(request, postData);
	return reply;
}

BaseRequest::BaseRequest(QNetworkReply *r) : reply(r)
{
	QObject::connect(reply, SIGNAL(sslErrors(QList<QSslError>)), this, SLOT(onSslError(QList<QSslError>)));
	QObject::connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(onError(QNetworkReply::NetworkError)));
	QObject::connect(reply, SIGNAL(finished()), this, SLOT(onRequestFinished()));
}
BaseRequest::~BaseRequest() { }

void BaseRequest::onError(QNetworkReply::NetworkError code)
{
	QString error_id = QString("network_failure.%1").arg(code);
	QString error_message = reply->errorString();

	emit failed(error_id, error_message);
}

}
