#include "flist_characterinfodialog.h"

#include <QVBoxLayout>
#include <QTextEdit>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QLabel>
#include <QTabWidget>

namespace Ui
{
	class FCharacterInfoDialogUi
	{
	public:
		QVBoxLayout *vbl;
		QLabel *name;
		QLabel *status;
		QTabWidget *profileTabs;
		QDialogButtonBox *buttons;
		QPushButton *closeButton;
		QTextEdit *profileTab;
		QTextEdit *kinkTab;

		void setupUi(QDialog *dialog)
		{
			vbl = new QVBoxLayout(dialog);

			name = new QLabel(dialog);
			name->setTextFormat(Qt::RichText);
			name->setWordWrap(true);
			name->setTextInteractionFlags(Qt::TextSelectableByMouse|Qt::TextSelectableByKeyboard);
			vbl->addWidget(name);

			status = new QLabel(dialog);
			status->setWordWrap(true);
			status->setTextFormat(Qt::RichText);
			status->setTextInteractionFlags(Qt::TextSelectableByMouse|Qt::TextSelectableByKeyboard);
			vbl->addWidget(status);

			profileTabs = new QTabWidget(dialog);
			profileTab = new QTextEdit();
			profileTab->setReadOnly(true);
			profileTabs->addTab(profileTab, QString("Profile"));
			kinkTab = new QTextEdit();
			kinkTab->setReadOnly(true);
			profileTabs->addTab(kinkTab, QString("Kinks"));
			profileTabs->setCurrentIndex(0);
			vbl->addWidget(profileTabs);

			buttons = new QDialogButtonBox(Qt::Horizontal, dialog);
			closeButton = new QPushButton(QIcon(":/images/cross.png"), QString("Close"));
			buttons->addButton(closeButton, QDialogButtonBox::RejectRole);
			vbl->addWidget(buttons);

			dialog->setLayout(vbl);
			dialog->setWindowTitle(QString("Character Info"));

			QObject::connect(buttons, SIGNAL(rejected()), dialog, SLOT(reject()));
			QMetaObject::connectSlotsByName(dialog);
		}
	};
}

FCharacterInfoDialog::FCharacterInfoDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::FCharacterInfoDialogUi)
{
	ui->setupUi(this);
}

FCharacterInfoDialog::~FCharacterInfoDialog()
{
	delete ui;
}

void FCharacterInfoDialog::setDisplayedCharacter(FCharacter *c)
{
	QString name = QString("<b>%1</b> (%2)").arg(c->name()).arg(c->statusString());
	ui->name->setText(name);

	ui->status->setText(c->statusMsg());
	updateProfile(c);
	updateKinks(c);
}

void FCharacterInfoDialog::updateProfile(FCharacter *c)
{
	updateKeyValues(c->getProfileDataKeys(), c->getProfileData(), ui->profileTab);
}

void FCharacterInfoDialog::updateKinks(FCharacter *c)
{
	updateKeyValues(c->getCustomKinkDataKeys(), c->getCustomKinkData(), ui->kinkTab);
}

void FCharacterInfoDialog::updateKeyValues(QStringList &k, QHash<QString,QString> &kv, QTextEdit *te)
{
	te->clear();
	foreach(QString key, k)
	{
		te->append(QString("<b>%1:</b> %2").arg(key).arg(kv[key]));
	}
}
