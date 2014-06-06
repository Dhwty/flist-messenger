#include "flist_logincontroller.h"

FLoginController::FLoginController(FHttpApi::Endpoint *e, FAccount *a, QObject *parent) :
	QObject(parent), ep(e), account(a)
{
}

void FLoginController::requestLogin(QString username, QString password)
{
	ticketrequest = ep->getTicket(username, password);
	QObject::connect(ticketrequest, SIGNAL(succeeded()), this, SLOT(ticketRequestSucceeded()));
	QObject::connect(ticketrequest, SIGNAL(failed(QString,QString)), this, SLOT(ticketRequestFailed(QString,QString)));
}

void FLoginController::ticketRequestSucceeded()
{
	account->ticket = ticketrequest->ticket->ticket;
}

void FLoginController::ticketRequestFailed(QString error_id, QString error_message)
{
	emit loginFailed(error_id, error_message);
	QString msg("%1 (%2)");
	display->showError(msg.arg(error_message, error_id));
}

void FLoginController::requestConnect(QString character)
{
	(void)character;
}

void FLoginController::saveCredentials(QString username, QString password)
{
	(void)username; (void)password;
}

void FLoginController::clearCredentials()
{

}
