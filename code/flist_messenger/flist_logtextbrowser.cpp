#include "flist_logtextbrowser.h"

#include <QMenu>
#include <QContextMenuEvent>
#include <QClipboard>
#include <QApplication>
#include <QDesktopServices>
#include <QScrollBar>
#include "flist_global.h"
#include "flist_iuserinterface.h"
#include "flist_session.h"

FLogTextBrowser::FLogTextBrowser(iUserInterface *ui, QWidget *parent) :
        QTextBrowser(parent),
	flist_copylink(),
	flist_copyname(),
	sessionid(),
	ui(ui)
{
}

void FLogTextBrowser::contextMenuEvent(QContextMenuEvent *event)
{
	flist_copylink = anchorAt(event->pos());
	QTextCursor cursor = textCursor();
	QMenu *menu = new QMenu;
	QAction *action;
	if(flist_copylink.isEmpty()) {
		//Plain text selected
	} else if(flist_copylink.startsWith("https://www.f-list.net/c/")) {
		flist_copyname = flist_copylink.mid(QString("https://www.f-list.net/c/").length());
		if(flist_copyname.endsWith("/")) {
			flist_copyname = flist_copyname.left(flist_copyname.length() - 1);
		}
		menu->addAction(QString("Open Profile"), this, SLOT(openProfile()));
		//todo: Get the list of available sessions. Create a submenu with all available characters if there is more than one (or this session isn't vlaid).
		menu->addAction(QString("Open PM"), this, SLOT(openPrivateMessage()));
		menu->addAction(QString("Copy Profile Link"), this, SLOT(copyLink()));
		menu->addAction(QString("Copy Name"), this, SLOT(copyName()));
		menu->addSeparator();
	} else if(flist_copylink.startsWith("#AHI-")) {
		flist_copyname = flist_copylink.mid(5);
		//todo: Get the list of available sessions. Create a submenu with all available characters if there is more than one (or this session isn't vlaid).
		menu->addAction(QString("Join Channel"), this, SLOT(joinChannel()));
		//todo: Maybe get the name the plain text of the link and make that available for copying?
		menu->addAction(QString("Copy Channel ID"), this, SLOT(copyName()));
		menu->addSeparator();
	} else if(flist_copylink.startsWith("#CSA-")) {
		flist_copyname = flist_copylink.mid(5);
		//todo: If possible, get which session this actually came from and use that.
		menu->addAction(QString("Confirm Staff Report"), this, SLOT(confirmReport()));
		menu->addAction(QString("Copy Call ID"), this, SLOT(copyName()));
		menu->addSeparator();
	} else {
		menu->addAction(QString("Copy Link"), this, SLOT(copyLink()));
		menu->addSeparator();
	}
	action = menu->addAction(QString("Copy Selection"), this, SLOT(copy()));
	action->setEnabled(cursor.hasSelection());
	menu->addAction(QString("Select All"), this, SLOT(selectAll()));

	menu->exec(event->globalPos());
	delete menu;
}

void FLogTextBrowser::setSessionID(QString sessionid)
{
	this->sessionid = sessionid;
}
QString FLogTextBrowser::getSessionID()
{
	return sessionid;
}

void FLogTextBrowser::openProfile()
{
	if(ui) {
		ui->openCharacterProfile(ui->getSession(sessionid), flist_copyname);
	} else {
		//todo:
	}
}
void FLogTextBrowser::openWebProfile()
{
	QDesktopServices::openUrl(flist_copylink);
}
void FLogTextBrowser::openPrivateMessage()
{
	if(ui) {
		ui->addCharacterChat(ui->getSession(sessionid), flist_copyname);
	} else {
		//todo:
	}
}
void FLogTextBrowser::copyLink()
{
	QClipboard *clipboard = QApplication::clipboard();
	clipboard->setText(flist_copylink, QClipboard::Clipboard);
	clipboard->setText(flist_copylink, QClipboard::Selection);
}
void FLogTextBrowser::copyName()
{
	QClipboard *clipboard = QApplication::clipboard();
	clipboard->setText(flist_copyname, QClipboard::Clipboard);
	clipboard->setText(flist_copyname, QClipboard::Selection);
}

void FLogTextBrowser::joinChannel()
{
	FSession *session = ui->getSession(sessionid);
	if(!session) {
		return;
	}
	session->joinChannel(flist_copyname);
}

void FLogTextBrowser::confirmReport()
{
	FSession *session = ui->getSession(sessionid);
	if(!session) {
		return;
	}
	session->sendConfirmStaffReport(flist_copyname);
}

void FLogTextBrowser::append(const QString & text)
{
	QTextCursor cur = textCursor();
	if (cur.hasSelection())
	{
		int oldPosition = cur.position();
		int oldAnchor = cur.anchor();

		int vpos = verticalScrollBar()->value();
		int vmax = verticalScrollBar()->maximum();

		QTextBrowser::append(text);

		cur.setPosition(oldAnchor, QTextCursor::MoveAnchor);
		cur.setPosition(oldPosition, QTextCursor::KeepAnchor);
		setTextCursor(cur);
		if (vpos != vmax) { verticalScrollBar()->setValue(vpos); }
		else
		{
			vmax = verticalScrollBar()->maximum();
			verticalScrollBar()->setValue(vmax);
		}
	}
	else { QTextBrowser::append(text); }
}
