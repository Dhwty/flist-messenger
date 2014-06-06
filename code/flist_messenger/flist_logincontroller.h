#ifndef FLIST_LOGINCONTROLLER_H
#define FLIST_LOGINCONTROLLER_H

#include <QObject>

#include "flist_api.h"
#include "api/endpoint_v1.h"
#include "flist_account.h"
#include "flist_loginwindow.h"

class FLoginWindow;

class FLoginController : public QObject
{
	Q_OBJECT
public:
	explicit FLoginController(FHttpApi::Endpoint *e, FAccount *a, QObject *parent = 0);

	void setWidget(FLoginWindow *w);

signals:
	void loginSucceeded();
	void loginFailed(QString error_id, QString error_message);
	void connectSucceeded();
	void connectFailed(QString error_id, QString error_message);

public slots:
	void requestLogin(QString username, QString password);
	void requestConnect(QString character);
	void saveCredentials(QString username, QString password);
	void clearCredentials();

private:
	FLoginWindow *display;
	FHttpApi::Endpoint *ep;
	FHttpApi::Request<FHttpApi::TicketResponse> *ticketrequest;
	FAccount *account;

private slots:
	void ticketRequestSucceeded();
	void ticketRequestFailed(QString error_id, QString error_message);
};

#endif // FLIST_LOGINCONTROLLER_H
