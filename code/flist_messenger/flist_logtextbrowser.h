#ifndef FLIST_LOGTEXTBROWSER_H
#define FLIST_LOGTEXTBROWSER_H

#include <QString>
#include <QTextBrowser>

class iUserInterface;

class FLogTextBrowser : public QTextBrowser
{
	Q_OBJECT
public:
	explicit FLogTextBrowser(iUserInterface *ui, QWidget *parent = 0);
	void setSessionID(QString sessionid);
	QString getSessionID();

protected:
	virtual void contextMenuEvent(QContextMenuEvent *event);
signals:

public slots:
	void openProfile();
	void openWebProfile();
	void openPrivateMessage();
	void copyLink();
	void copyName();
	void joinChannel();
	void confirmReport();
	void append(const QString & text);

private:
	QString flist_copylink;
	QString flist_copyname;
	QString sessionid;
	iUserInterface *ui;
};

#endif // FLIST_LOGTEXTBROWSER_H
