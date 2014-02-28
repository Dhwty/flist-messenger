#include "flist_loginselectwindow.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QGroupBox>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>

FLoginSelectWindow::FLoginSelectWindow(FAccount *account, QMainWindow *mainwindow, QObject *parent) :
    QObject(parent)
{
	this->account = account;
	this->mainwindow = mainwindow;

	this->dialog = 0;

}

void FLoginSelectWindow::createWindow() {
	if(dialog != 0) {
		return;
	}
	dialog = new QDialog(mainwindow);
	dialog->setWindowTitle ( "F-chat messenger - Choose Character" );
	dialog->setWindowIcon ( QIcon ( ":/images/icon.png" ) );

	//todo: create and populate window
        QGroupBox *groupBox = new QGroupBox ( dialog );
        groupBox->setObjectName ( QString::fromUtf8 ( "loginGroup" ) );
        groupBox->setGeometry ( QRect ( 0, 0, 250, 30 ) );
        QPushButton *btnConnect = new QPushButton ( groupBox );
        btnConnect->setObjectName ( QString::fromUtf8 ( "loginButton" ) );
        btnConnect->setGeometry ( QRect ( 5, 40, 255, 26 ) );
        btnConnect->setText ( "Login" );
        QComboBox *comboBox = new QComboBox ( groupBox );
        comboBox->setObjectName ( QString::fromUtf8 ( "charSelectBox" ) );
        comboBox->setGeometry ( QRect ( 80, 10, 180, 27 ) );

        for ( int i = 0; i < account->characterList.count(); ++i )
        {
                comboBox->addItem ( account->characterList[i] );

                if ( account->characterList[i] == account->defaultCharacter )
                {
                        comboBox->setCurrentIndex ( i );
                }
        }

        QLabel *label = new QLabel ( groupBox );

        label->setObjectName ( QString::fromUtf8 ( "charlabel" ) );
        label->setGeometry ( QRect ( 10, 13, 71, 21 ) );
        label->setText ( "Character:" );
        //dialog->setCentralWidget ( groupBox );

	//todo: connect signals


	connect ( btnConnect, SIGNAL ( clicked() ), this, SLOT ( selectClicked() ) );
	//connect ( btnCancel, SIGNAL ( clicked() ), this, SLOT ( cancelClicked() ) );


	int wid = QApplication::desktop()->width();
	int hig = QApplication::desktop()->height();
	int mwid = 265;
	int mhig = 100;
	dialog->setGeometry ( ( wid / 2 ) - ( int ) ( mwid*0.5 ), ( hig / 2 ) - ( int ) ( mhig*0.5 ), mwid, mhig );

}

void FLoginSelectWindow::destroyWindow()
{
	if(dialog == 0) {
		return;
	}
	dialog->hide();
	dialog->deleteLater();
	dialog = 0;
}

void FLoginSelectWindow::selectBegin() {
	//create window
	createWindow();
	//todo: populate list
	//todo: show window
	dialog->show();
}

void FLoginSelectWindow::selectClicked() {
	//todo: get selected character name
	QString choice = "";
	destroyWindow();
	emit selectComplete(account, choice);
}

void FLoginSelectWindow::cancelClicked() {
	destroyWindow();
	emit selectCancled(account);
}

