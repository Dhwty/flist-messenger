#ifndef FHTTPAPI_ENDPOINT_V1_H
#define FHTTPAPI_ENDPOINT_V1_H

#include "flist_api.h"

namespace FHttpApi {

class Endpoint_v1 : public Endpoint
{
public:
	Endpoint_v1(QNetworkAccessManager *n);

	virtual Request<TicketResponse> *getTicket(crQString username, crQString password,
														 DataTypes additionalInfo = Data_GetTicketDefault);

	class TicketRequest : public Request<TicketResponse>
	{
	public:
		TicketRequest(QNetworkReply *qnr, QString username, QString password);
		virtual ~TicketRequest();

	public slots:
		virtual void onRequestFinished();

	private:
		TicketResponse *resp;
		QString _un;
		QString _p;
	};
};

} // namespace FHttpApi

#endif // FHTTPAPI_ENDPOINT_V1_H
