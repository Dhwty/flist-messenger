#ifndef HELPDIALOG_H
#define HELPDIALOG_H

#include <QDialog>
#include <QUrl>

namespace Ui
{
class FHelpDialog;
}

class FHelpDialog : public QDialog
{
	Q_OBJECT
public:
	explicit FHelpDialog(QWidget *parent = 0);

public slots:

private:
	Ui::FHelpDialog *ui;
};

#endif // HELPDIALOG_H
