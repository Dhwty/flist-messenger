#include "aboutdialog.h"
#include "ui_aboutdialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QDialogButtonBox>
#include <QTextBrowser>
#include <QPushButton>

#include "flist_global.h"

FAboutDialog::FAboutDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::FAboutDialog)
{
	ui->setupUi(this);

	QString hdr;
	hdr = "<span style=\" font-size:12pt; font-weight:600;\">"
	      FLIST_NAME
	      "</span><br/><span style=\"font-weight:600;\">"
	      FLIST_VERSIONNUM
	      "</span><br/>Using Qt "
	      QT_VERSION_STR;

	ui->headerText->setText(hdr);
	QPushButton *close = ui->buttonBox->button(QDialogButtonBox::Close);
	close->setIcon(QIcon(":/images/cross.png"));
	setWindowTitle("About F-list Messenger");
	connect(ui->textBrowser, SIGNAL(anchorClicked(QUrl)), this, SIGNAL(anchorClicked(QUrl)));
}

FAboutDialog::~FAboutDialog()
{
	delete ui;
}
