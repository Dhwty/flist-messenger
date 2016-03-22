#ifndef FRIENDSDIALOG_H
#define FRIENDSDIALOG_H

#include <QDialog>
#include <QListWidgetItem>

#include "flist_session.h"
#include "flist_character.h"

namespace Ui {
class FriendsDialog;
}

class IgnoreDataProvider;

class FriendsDialog : public QDialog
{
	Q_OBJECT

public:
	explicit FriendsDialog(FSession *session, QWidget *parent = 0);
	~FriendsDialog();

	void showFriends();
	void showIgnore();

signals:
	void privateMessageRequested(QString character);
	//TODO: Better context menu implementation (FContextMenu?)
	void friendContextMenuRequested(QString character);

public slots:
	void notifyCharacterOnline(FSession *s, QString character, bool online);
	void notifyCharacterStatus(FSession *s, QString character);
	
	void notifyFriendsList(FSession *s);
	void notifyFriendAdd(FSession *s, QString character);
	void notifyFriendRemove(FSession *s, QString character);

private:
	Ui::FriendsDialog *ui;
	FSession *session;
	IgnoreDataProvider *ignoreData;

private slots:
	void openPmClicked();
	void friendListContextMenu(const QPoint &point);
	void friendListSelectionChanged(QListWidgetItem *current, QListWidgetItem *previous);
	void friendListDoubleClicked(QListWidgetItem *target);
};

#endif // FRIENDSDIALOG_H
