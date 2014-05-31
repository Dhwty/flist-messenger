#ifndef FLIST_CHARACTERINFODIALOG_H
#define FLIST_CHARACTERINFODIALOG_H

#include <QDialog>
#include <QHash>
#include <QTextEdit>

#include "flist_character.h"

namespace Ui
{
	class FCharacterInfoDialogUi;
}

class FCharacterInfoDialog : public QDialog
{
	Q_OBJECT
public:
	explicit FCharacterInfoDialog(QWidget *parent = 0);
	~FCharacterInfoDialog();

	void setDisplayedCharacter(FCharacter *c);
	void updateProfile(FCharacter *c);
	void updateKinks(FCharacter *c);

signals:

public slots:

private:
	Ui::FCharacterInfoDialogUi *ui;

	void updateKeyValues(QStringList &k, QHash<QString,QString> &kv, QTextEdit *te);
};

#endif // FLIST_CHARACTERINFODIALOG_H
