#include "makeroomdialog.h"
#include "ui_makeroomdialog.h"

#include <QPushButton>

FMakeRoomDialog::FMakeRoomDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FMakeRoomDialog)
{
	ui->setupUi(this);

	QPushButton *okButton = ui->buttonBox->button(QDialogButtonBox::Ok);
	okButton->setIcon(QIcon(":/images/tick.png"));
	okButton->setText("Create");
	okButton->setDisabled(true);

	QPushButton *cancelButton = ui->buttonBox->button(QDialogButtonBox::Cancel);
	cancelButton->setIcon(QIcon(":/images/cross.png"));

	connect(ui->buttonBox, SIGNAL(rejected()), ui->roomName, SLOT(clear()));
	connect(ui->buttonBox, SIGNAL(rejected()), this, SLOT(close()));
	connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(onAccept()));
	connect(ui->roomName, SIGNAL(textChanged(QString)), this, SLOT(onNameChange(QString)));
}

FMakeRoomDialog::~FMakeRoomDialog()
{
	delete ui;
}

void FMakeRoomDialog::onAccept()
{
	if (!ui->roomName->text().isEmpty())
	{
		emit requestedRoomCreation(ui->roomName->text());
		hide();
	}
}

void FMakeRoomDialog::onNameChange(QString newtext)
{
	ui->buttonBox->setDisabled(newtext.isEmpty());
}
