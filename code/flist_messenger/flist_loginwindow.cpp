#include "flist_loginwindow.h"
#include "ui_flist_loginwindow.h"

FLoginWindow::FLoginWindow(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::FLoginWindow)
{
	ui->setupUi(this);

	//Set the proper labels and icons
	QPushButton *csok, *cscancel;
	csok = ui->charSelectButtons->button(QDialogButtonBox::Ok);
	csok->setIcon(QIcon(":/images/plug-connect.png"));
	csok->setText(QString("Connect"));
	cscancel = ui->charSelectButtons->button(QDialogButtonBox::Cancel);
	cscancel->setIcon(QIcon(":/images/cross.png"));
	cscancel->setText("Cancel");

	connect(ui->loginButton, SIGNAL(clicked()), this, SLOT(loginClicked()));
	connect(csok, SIGNAL(clicked()), this, SLOT(connectClicked()));
	connect(cscancel, SIGNAL(clicked()), this, SLOT(showLoginPage()));
	connect(ui->dismissMessage, SIGNAL(clicked()), this, SLOT(dismissMessage()));
}

FLoginWindow::~FLoginWindow()
{
	delete ui;
}

void FLoginWindow::showError(QString message)
{
	ui->message->setText(message);
	lastPage = ui->outerStack->currentIndex();
	ui->outerStack->setCurrentWidget(ui->messagePage);
}

void FLoginWindow::showLoginPage()
{
	ui->outerStack->setCurrentWidget(ui->loginPage);
}

void FLoginWindow::showConnectPage(FAccount *account)
{
	ui->character->clear();
	foreach (QString c, account->characterList)
	{
		ui->character->addItem(c);
	}
	ui->outerStack->setCurrentWidget(ui->charselectPage);
	QString defaultCharacter = account->defaultCharacter;
	ui->character->setCurrentIndex(ui->character->findText(defaultCharacter));
}

void FLoginWindow::dismissMessage()
{
	ui->outerStack->setCurrentIndex(lastPage);
}

void FLoginWindow::loginClicked()
{
	QString un = ui->username->text();
	QString pass = ui->password->text();

	if(un.isEmpty() || pass.isEmpty())
	{
		showError("Please enter both username and password");
	}
	else
	{
		emit loginRequested(un, pass);
	}
}

void FLoginWindow::connectClicked()
{
	emit connectRequested(ui->character->currentText());
}
