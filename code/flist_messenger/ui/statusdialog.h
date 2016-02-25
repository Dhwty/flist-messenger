#ifndef STATUSDIALOG_H
#define STATUSDIALOG_H

#include <QDialog>
#include <QString>

namespace Ui {
class StatusDialog;
}

class StatusDialog : public QDialog
{
	Q_OBJECT

public:
	explicit StatusDialog(QWidget *parent = 0);
	void setShownStatus(QString status, QString statusMessage);
	~StatusDialog();

signals:
	void statusSelected(QString status, QString statusMessage);

private:
	Ui::StatusDialog *ui;
	
private slots:
	void onAccepted();
};

#endif // STATUSDIALOG_H
