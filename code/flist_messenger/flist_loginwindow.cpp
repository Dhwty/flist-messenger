#include "flist_loginwindow.h"
#include "ui_flist_loginwindow.h"
#include "usereturn.h"

#include <QMessageBox>

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

	ReturnFilter *rf = new ReturnFilter(this);
	this->installEventFilter(rf);

	connect(rf, SIGNAL(plainEnter()), this, SLOT(enterPressed()));
}

FLoginWindow::~FLoginWindow()
{
	delete ui;
}

void FLoginWindow::showError(QString message)
{
	if (ui->outerStack->currentWidget() == ui->messagePage)
	{
		QString currentText = ui->message->text();
		currentText.append("\n");
		currentText.append(message);
		ui->message->setText(currentText);
	}
	else
	{
		ui->message->setText(message);
		lastPage = ui->outerStack->currentIndex();
		ui->outerStack->setCurrentWidget(ui->messagePage);
	}
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

void FLoginWindow::enterPressed()
{
	QWidget *cw = ui->outerStack->currentWidget();
	if (cw == ui->loginPage)
	{
		loginClicked();
	}
	else if (cw == ui->charselectPage)
	{
		connectClicked();
	}
	else if (cw == ui->messagePage)
	{
		dismissMessage();
	}
	else
	{
		QMessageBox::critical(this, "This shouldn't happen", "You somehow managed to press enter while a nonexistent page was selected. Please file a bug report explaining what you just did.");
	}
}
