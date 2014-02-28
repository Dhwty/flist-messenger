#ifndef FLIST_LOGINSELECTWINDOW_H
#define FLIST_LOGINSELECTWINDOW_H

#include <QObject>
#include <QDialog>
#include <QMainWindow>

#include "flist_account.h"

class FLoginSelectWindow : public QObject
{
Q_OBJECT
public:
	explicit FLoginSelectWindow(FAccount *account, QMainWindow *mainwindow, QObject *parent = 0);

signals:
	void selectComplete(FAccount *account, QString choice);
	void selectCancled(FAccount *account);

public slots:
	void selectBegin();

private slots:

	void selectClicked();
	void cancelClicked();

private:
	void createWindow();
	void destroyWindow();
	FAccount *account;
	QMainWindow *mainwindow;
	QDialog *dialog;

};

#endif // FLIST_LOGINSELECTWINDOW_H
