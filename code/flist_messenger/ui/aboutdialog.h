#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include <QDialog>
#include <QUrl>

namespace Ui {
class FAboutDialog;
}

class FAboutDialog : public QDialog
{
	Q_OBJECT

public:
	explicit FAboutDialog(QWidget *parent = 0);
	~FAboutDialog();

signals:
	void anchorClicked(QUrl link);

private:
	Ui::FAboutDialog *ui;
};

#endif // ABOUTDIALOG_H
