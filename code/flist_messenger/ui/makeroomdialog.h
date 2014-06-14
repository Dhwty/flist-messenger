#ifndef MAKEROOMDIALOG_H
#define MAKEROOMDIALOG_H

#include <QDialog>

namespace Ui {
class FMakeRoomDialog;
}

class FMakeRoomDialog : public QDialog
{
	Q_OBJECT

public:
	explicit FMakeRoomDialog(QWidget *parent = 0);
	~FMakeRoomDialog();

signals:
	void requestedRoomCreation(QString name);

private:
	Ui::FMakeRoomDialog *ui;

private slots:
	void onAccept();
	void onNameChange(QString newtext);
};

#endif // MAKEROOMDIALOG_H
