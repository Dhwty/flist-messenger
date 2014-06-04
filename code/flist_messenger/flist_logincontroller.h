#ifndef FLIST_LOGINCONTROLLER_H
#define FLIST_LOGINCONTROLLER_H

#include <QObject>

class FLoginWindow;

class FLoginController : public QObject
{
	Q_OBJECT
public:
	explicit FLoginController(QObject *parent = 0);

	void setWidget(FLoginWindow *w);

signals:
	void loginSucceeded();
	void loginFailed();
	void connectSucceeded();
	void connectFailed();

public slots:
	void requestLogin(QString username, QString password);
	void requestConnect(QString character);
	void saveCredentials(QString username, QString password);
	void clearCredentials();

private:
	FLoginWindow *display;
};

#endif // FLIST_LOGINCONTROLLER_H
