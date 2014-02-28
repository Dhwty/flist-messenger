#ifndef FLIST_LOGINWINDOW_H
#define FLIST_LOGINWINDOW_H

#include <QObject>
#include <QDialog>
#include <QMainWindow>

#include "flist_account.h"

class FLoginWindow : public QObject
{
Q_OBJECT
public:
	explicit FLoginWindow(FAccount *account, QMainWindow *mainwindow, QObject *parent = 0);

signals:

public slots:

	void loginGetUserPass(QString &username, QString &password);
	void loginError(QString &title, QString &message);
	void loginComplete();
	void connectClicked();
	void cancelClicked();

private:
	void createWindow();
	void destroyWindow();
	FAccount *account;
	QMainWindow *mainwindow;
	QDialog *dialog;
};

#endif // FLIST_LOGINWINDOW_H
