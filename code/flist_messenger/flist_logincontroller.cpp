#include "flist_logincontroller.h"
#include "flist_messenger.h"

FLoginController::FLoginController(FHttpApi::Endpoint *e, FAccount *a, QObject *parent) :
	QObject(parent), display(0), ep(e), account(a)
{
}

void FLoginController::setWidget(FLoginWindow *w)
{
	display = w;
	connect(display, SIGNAL(loginRequested(QString,QString)), this, SLOT(requestLogin(QString,QString)));
}

void FLoginController::requestLogin(QString username, QString password)
{
	display->setEnabled(false);

	connect(account, SIGNAL(loginError(FAccount*,QString,QString)), this, SLOT(loginError(FAccount*,QString,QString)));
	connect(account, SIGNAL(loginComplete(FAccount*)), this, SLOT(loginComplete(FAccount*)));
	account->loginUserPass(username, password);
}

void FLoginController::requestConnect(QString character)
{
	static_cast<flist_messenger*>(parent())->startConnect(character);
}

void FLoginController::saveCredentials(QString username, QString password)
{
	(void)username; (void)password;
}

void FLoginController::clearCredentials()
{

}

void FLoginController::loginError(FAccount *a, QString errorTitle, QString errorMsg)
{
	(void)a;
	QString msg = QString("%1: %2").arg(errorTitle).arg(errorMsg);
	display->showError(msg);
	display->clearPassword();
}

void FLoginController::loginComplete(FAccount *a)
{
	(void)a;
	display->showConnectPage(account);
	connect(display,SIGNAL(connectRequested(QString)),this,SLOT(requestConnect(QString)));

}
