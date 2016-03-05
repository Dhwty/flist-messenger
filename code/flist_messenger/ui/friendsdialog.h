#ifndef FRIENDSDIALOG_H
#define FRIENDSDIALOG_H

#include <QDialog>
#include <QListWidgetItem>

#include "flist_session.h"
#include "flist_character.h"

namespace Ui {
class FriendsDialog;
}

class FriendsDialog : public QDialog
{
	Q_OBJECT

public:
	explicit FriendsDialog(FSession *session, QWidget *parent = 0);
	~FriendsDialog();

signals:
	privateMessageRequested(QString character);
	//TODO: Better context menu implementation (FContextMenu?)
	friendContextMenuRequested(QString character);

public slots:
	void notifyCharacterOnline(FSession *s, QString character, bool online);
	void notifyCharacterStatus(FSession *s, QString character);
	
	void notifyFriendsList(FSession *s);
	void notifyFriendAdd(FSession *s, QString character);
	void notifyFriendRemove(FSession *s, QString character);

	void notifyIgnoreList(FSession *s);
	void notifyIgnoreAdd(FSession *s, QString character);
	void notifyIgnoreRemove(FSession *s, QString character);

private:
	Ui::FriendsDialog *ui;
	FSession *session;

private slots:
	void openPmClicked();
	void addIgnoreClicked();
	void removeIgnoreClicked();
	void ignoreListSelectionChanged(QListWidgetItem *current, QListWidgetItem *previous);
	void ignoreTargetTextEdited(QString newText);
	void friendListContextMenu(const QPoint &point);
	void friendListSelectionChanged(QListWidgetItem *current, QListWidgetItem *previous);
	void friendListDoubleClicked(QListWidgetItem *target);
};

#endif // FRIENDSDIALOG_H
