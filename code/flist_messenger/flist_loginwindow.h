#ifndef FLIST_LOGINWINDOW_H
#define FLIST_LOGINWINDOW_H

#include <QDialog>

#include "flist_account.h"

namespace Ui {
class FLoginWindow;
}

class FLoginWindow : public QWidget
{
	Q_OBJECT

public:
	explicit FLoginWindow(QWidget *parent = 0);
	~FLoginWindow();

signals:
	void loginRequested(QString username, QString password);
	void saveCredentialsRequested(QString username, QString password);
	void clearCredentialsRequested();
	void connectRequested(QString characterName);

public slots:
	void showError(QString message);
	void showLoginPage();
	void showConnectPage(FAccount *account);

private:
	Ui::FLoginWindow *ui;
	int lastPage;

private slots:
	void dismissMessage();
	void loginClicked();
	void connectClicked();
};

#endif // FLIST_LOGINWINDOW_H
