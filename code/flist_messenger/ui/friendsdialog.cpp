#include "friendsdialog.h"
#include "ui_friendsdialog.h"
#include "flist_global.h"

#include <QListWidgetItem>

FriendsDialog::FriendsDialog(FSession *session, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FriendsDialog),
    session(session)
{
	ui->setupUi(this);
	ui->buttonBox->button(QDialogButtonBox::Close)->setIcon(QIcon(":/icons/cross.png"));
	
	connect(ui->btnOpenPM, SIGNAL(clicked(bool)), this, SLOT(openPmClicked()));
	connect(ui->btnAddIgnore, SIGNAL(clicked(bool)), this, SLOT(addIgnoreClicked()));
	connect(ui->btnRemIgnore, SIGNAL(clicked(bool)), this, SLOT(removeIgnoreClicked()));
	connect(ui->lwIgnoreList, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), this, SLOT(ignoreListSelectionChanged(QListWidgetItem*,QListWidgetItem*)));
	connect(ui->leIgnoreTarget, SIGNAL(textEdited(QString)), this, SLOT(ignoreTargetTextEdited(QString)));
	connect(ui->lwFriendsList, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(friendListContextMenu(QPoint)));
	connect(ui->lwFriendsList, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), this, SLOT(friendListSelectionChanged(QListWidgetItem*,QListWidgetItem*)));
	connect(ui->lwFriendsList, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(friendListDoubleClicked(QListWidgetItem*)));
	
	connect(session, SIGNAL(notifyCharacterOnline(FSession*,QString,bool)), this, SLOT(notifyCharacterOnline(FSession*,QString,bool)));
	connect(session, SIGNAL(notifyCharacterStatusUpdate(FSession*,QString)), this, SLOT(notifyCharacterStatus(FSession*,QString)));
	connect(session, SIGNAL(notifyIgnoreList(FSession*)), this, SLOT(notifyIgnoreList(FSession*)));
	connect(session, SIGNAL(notifyIgnoreAdd(FSession*,QString)), this, SLOT(notifyIgnoreAdd(FSession*,QString)));
	connect(session, SIGNAL(notifyIgnoreRemove(FSession*,QString)), this, SLOT(notifyIgnoreRemove(FSession*,QString)));
	
	this->notifyFriendsList(session);
	this->notifyIgnoreList(session);
}

FriendsDialog::~FriendsDialog()
{
	delete ui;
}

void FriendsDialog::notifyCharacterOnline(FSession *s, QString character, bool online)
{
	if(!s->isCharacterFriend(character)) { return; }
	
	FCharacter* f = 0;
	QListWidgetItem* lwi = 0;
	
	QList<QListWidgetItem*> items = ui->lwFriendsList->findItems(character, Qt::MatchFixedString);
	if(online && items.count() == 0)
	{
		f = s->getCharacter(character);
		lwi = new QListWidgetItem(*(f->statusIcon()),f->name());
		ui->lwFriendsList->addItem(lwi);
	}
	else if(!online /*&& items.count() > 0*/)
	{
		foreach(QListWidgetItem *i, items)
		{
			int row = ui->lwFriendsList->row(i);
			delete (ui->lwFriendsList->takeItem(row));
		}
	}
}

void FriendsDialog::notifyCharacterStatus(FSession *s, QString character)
{
	FCharacter *c = s->getCharacter(character);
	QList<QListWidgetItem*> items = ui->lwFriendsList->findItems(c->name(), Qt::MatchFixedString);
	if(items.count() > 0)
	{
		items.first()->setIcon(*(c->statusIcon()));
	}
}

void FriendsDialog::notifyFriendsList(FSession *s)
{
	QList<QListWidgetItem*> selected = ui->lwFriendsList->selectedItems();
	QString selectedName;
	if(!selected.empty())
	{
		selectedName = selected.first()->text();
	}
	
	ui->lwFriendsList->clear();
	foreach(QString i, s->getFriendsList())
	{
		if(session->isCharacterOnline(i))
		{
			FCharacter *f = session->getCharacter(i);
			QListWidgetItem *lwi = new QListWidgetItem(*(f->statusIcon()), f->name());
			ui->lwFriendsList->addItem(lwi);
		}
	}
	
	if(selectedName.length() > 0)
	{
		QList<QListWidgetItem*> items = ui->lwFriendsList->findItems(selectedName, Qt::MatchFixedString);
		if(items.count() > 0)
		{
			ui->lwFriendsList->setCurrentItem(items.first());
		}
	}
}

void FriendsDialog::notifyFriendAdd(FSession *s, QString character)
{
	if(!s->isCharacterOnline(character)) { return; }
	
	QList<QListWidgetItem*> items = ui->lwFriendsList->findItems(character, Qt::MatchFixedString);
	if(items.count() > 0) { return; }
	
	FCharacter *f = s->getCharacter(character);
	QListWidgetItem *lwi = new QListWidgetItem(*(f->statusIcon()), f->name());
	ui->lwFriendsList->addItem(lwi);
}

void FriendsDialog::notifyFriendRemove(FSession *s, QString character)
{
	if(!s->isCharacterFriend(character)) { return; }
	
	QList<QListWidgetItem*> items = ui->lwFriendsList->findItems(character, Qt::MatchFixedString);
	if(items.count() == 0) { return; }
	
	foreach(QListWidgetItem *i, items)
	{
		ui->lwFriendsList->removeItemWidget(i);
	}
}

void FriendsDialog::notifyIgnoreList(FSession *s)
{
	QList<QListWidgetItem*> selected = ui->lwIgnoreList->selectedItems();
	QString selectedName;
	if(!selected.empty())
	{
		selectedName = selected.first()->text();
	}
	
	ui->lwIgnoreList->clear();
	foreach(QString i, s->getIgnoreList())
	{
		ui->lwIgnoreList->addItem(i);
	}
	
	if(selectedName.length() > 0)
	{
		QList<QListWidgetItem*> items = ui->lwIgnoreList->findItems(selectedName, Qt::MatchFixedString);
		if(items.count() > 0)
		{
			ui->lwFriendsList->setCurrentItem(items.first());
		}
	}
}

void FriendsDialog::notifyIgnoreAdd(FSession *s, QString character)
{
	if(s != session) { return; }
	
	QList<QListWidgetItem*> items = ui->lwIgnoreList->findItems(character, Qt::MatchFixedString);
	if(items.count() > 0) { return; }
	
	ui->lwIgnoreList->addItem(character);
}

void FriendsDialog::notifyIgnoreRemove(FSession *s, QString character)
{
	if(s != session) { return; }
	
	QList<QListWidgetItem*> items = ui->lwIgnoreList->findItems(character, Qt::MatchFixedString);
	if(items.count() == 0) { return; }
	
	int row = ui->lwIgnoreList->row(items.first());
	delete (ui->lwIgnoreList->takeItem(row));
}

void FriendsDialog::openPmClicked()
{
	QList<QListWidgetItem*> selected = ui->lwFriendsList->selectedItems();
	if(!selected.empty())
	{
		QString name = selected.first()->text();
		emit privateMessageRequested(name);
	}
}

void FriendsDialog::addIgnoreClicked()
{
	QString character = ui->leIgnoreTarget->text().simplified();
	if(character != "" && !session->isCharacterIgnored(character))
	{
		session->sendIgnoreAdd(character);
		ui->leIgnoreTarget->clear();
		ui->btnAddIgnore->setEnabled(false);
	}
}

void FriendsDialog::removeIgnoreClicked()
{
	QString character = ui->lwIgnoreList->currentItem()->text();
	
	if(character != "" && session->isCharacterIgnored(character))
	{
		session->sendIgnoreDelete(character);
		ui->leIgnoreTarget->clear();
		ui->btnAddIgnore->setEnabled(false);
	}
}

void FriendsDialog::ignoreListSelectionChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
	if(current == previous) { return; }
	
	if(!current)
	{
		ui->btnRemIgnore->setEnabled(false);
		return;
	}
	
	ui->btnAddIgnore->setEnabled(false);
	ui->btnRemIgnore->setEnabled(true);
	ui->leIgnoreTarget->setText(current->text());
}

void FriendsDialog::ignoreTargetTextEdited(QString newText)
{
	QList<QListWidgetItem*> items = ui->lwIgnoreList->findItems(newText, Qt::MatchFixedString);
	if(items.count() > 0)
	{
		ui->btnRemIgnore->setEnabled(true);
		ui->btnAddIgnore->setEnabled(false);
		ui->lwIgnoreList->setCurrentItem(items.first());
	}
	else
	{
		ui->btnRemIgnore->setEnabled(false);
		ui->btnAddIgnore->setEnabled(newText.simplified() != "");
		ui->lwIgnoreList->selectionModel()->clear();
	}
}

void FriendsDialog::friendListContextMenu(const QPoint &point)
{
	 QListWidgetItem* lwi = ui->lwFriendsList->itemAt(point);
	 if(lwi)
	 {
		 emit friendContextMenuRequested(lwi->text());
	 }
}

void FriendsDialog::friendListSelectionChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
	if(current == previous) { return; }
	
	ui->btnOpenPM->setEnabled(current != 0);
}

void FriendsDialog::friendListDoubleClicked(QListWidgetItem *target)
{
	emit privateMessageRequested(target->text());
}
