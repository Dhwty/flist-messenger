#include "flist_loginwindow.h"

#include <QString>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QApplication>
#include <QDesktopWidget>
#include <QMessageBox>


FLoginWindow::FLoginWindow(FAccount *account, QMainWindow *mainwindow, QObject *parent) :
    QObject(parent)
{
	this->account = account;
	this->mainwindow = mainwindow;

	this->dialog = 0;

	//todo: connect signals and slots between this and account

	connect(account, SIGNAL(loginComplete()), this, SLOT(loginComplete()));
}

void FLoginWindow::createWindow() {
	if(dialog != 0) {
		QPushButton *btnConnect = dialog->findChild<QPushButton *>(QString("loginButton"));
		btnConnect->setEnabled(true);
		return;
	}
	dialog = new QDialog(mainwindow);
	dialog->setWindowTitle ( "F-chat messenger - Account Login" );
	dialog->setWindowIcon ( QIcon ( ":/images/icon.png" ) );

	// The "please log in" label
	//QWidget *verticalLayoutWidget = new QWidget ( dialog );
	//QVBoxLayout *verticalLayout = new QVBoxLayout ( verticalLayoutWidget );
	QVBoxLayout *verticalLayout = new QVBoxLayout ( dialog );
	QLabel *label = new QLabel();
	label->setText ( "Please log in using your F-list details." );
	verticalLayout->addWidget ( label );

	// The acc and pass textfields, along with labels
	QGridLayout *gridLayout = new QGridLayout;
	label = new QLabel ( QString ( "Account:" ) );
	gridLayout->addWidget ( label, 0, 0 );
	label = new QLabel ( QString ( "Password:" ) );
	gridLayout->addWidget ( label, 1, 0 );
	//todo: the event filter
	//ReturnLogin* loginreturn = new ReturnLogin(dialog);
	QLineEdit *lineEdit = new QLineEdit();
	//todo: the event filter
	//lineEdit->installEventFilter(loginreturn);
	lineEdit->setObjectName ( QString ( "accountNameInput" ) );
	gridLayout->addWidget ( lineEdit, 0, 1 );
	lineEdit = new QLineEdit;
	//todo: the event filter
	//lineEdit->installEventFilter(loginreturn);
	lineEdit->setEchoMode ( QLineEdit::Password );
	lineEdit->setObjectName ( QString ( "passwordInput" ) );
	gridLayout->addWidget ( lineEdit, 1, 1 );

//        // "Checking version" label
//        lblCheckingVersion = new QLabel( QString ( "Checking version..." ) );
//        gridLayout->addWidget ( lblCheckingVersion, 2, 1 );

	// The login button
	QPushButton *btnConnect = new QPushButton;
	btnConnect->setObjectName ( QString ( "loginButton" ) );
	btnConnect->setText ( "Login" );
	btnConnect->setIcon ( QIcon ( ":/images/tick.png" ) );
//        btnConnect->hide();
	gridLayout->addWidget ( btnConnect, 2, 1 );
	verticalLayout->addLayout ( gridLayout );
	//dialog->setCentralWidget ( verticalLayoutWidget );
	connect ( btnConnect, SIGNAL ( clicked() ), this, SLOT ( connectClicked() ) );

	int wid = QApplication::desktop()->width();
	int hig = QApplication::desktop()->height();
	int mwid = 265;
	int mhig = 100;
	dialog->setGeometry ( ( wid / 2 ) - ( int ) ( mwid*0.5 ), ( hig / 2 ) - ( int ) ( mhig*0.5 ), mwid, mhig );

}

void FLoginWindow::destroyWindow()
{
	if(dialog == 0) {
		return;
	}
	dialog->hide();
	dialog->deleteLater();
	dialog = 0;
}

void FLoginWindow::loginGetUserPass(QString &username, QString &password) {
	createWindow();
	//todo: show window
	//todo:
}

void FLoginWindow::loginError(QString &title, QString &message)
{
	QMessageBox::critical(dialog ? static_cast<QWidget *>(dialog) : static_cast<QWidget *>(mainwindow), title, message);
	createWindow();
}

void FLoginWindow::cancelClicked()
{
	//todo: destroy window
	//todo: pass signal on
}

void FLoginWindow::loginComplete()
{
	//todo: destroy window
	//todo: pass signal on
}

void FLoginWindow::connectClicked()
{
	QString username;
	QString password;
	//todo:
	QPushButton *btnConnect = dialog->findChild<QPushButton *>(QString("loginButton"));
	btnConnect->setEnabled(false);
	QLineEdit *lineEdit = dialog->findChild<QLineEdit *> ( QString ( "accountNameInput" ) );
	username = lineEdit->text();
	lineEdit = dialog->findChild<QLineEdit *> ( QString ( "passwordInput" ) );
	password = lineEdit->text();
	//todo: check if there are empty fields
	//todo: Initiate login
}
