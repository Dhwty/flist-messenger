#include "flist_logtextbrowser.h"

#include <QMenu>
#include <QContextMenuEvent>
#include <QClipboard>
#include <QApplication>
#include <QDesktopServices>
#include <QScrollBar>
#include <QFile>
#include <QDir>
#include <QImageReader>
#include <QMovie>
#include "flist_global.h"
#include "flist_iuserinterface.h"
#include "flist_session.h"

FLogTextBrowser::FLogTextBrowser(iUserInterface *ui, QWidget *parent) : QTextBrowser(parent), flist_copylink(), flist_copyname(), sessionid(), ui(ui) {
    manager.setParent(this);
    signalMapper.setParent(this);

    connect(&manager, SIGNAL(finished(QNetworkReply *)), this, SLOT(resourceLoaded(QNetworkReply *)));
    connect(&signalMapper, SIGNAL(mappedInt(int)), this, SLOT(animate(int)));
}

void FLogTextBrowser::contextMenuEvent(QContextMenuEvent *event) {
    flist_copylink = anchorAt(event->pos());
    QTextCursor cursor = textCursor();
    QMenu *menu = new QMenu;
    QAction *action;
    if (flist_copylink.isEmpty()) {
        // Plain text selected
    } else if (flist_copylink.startsWith("https://www.f-list.net/c/")) {
        flist_copyname = flist_copylink.mid(QString("https://www.f-list.net/c/").length());
        if (flist_copyname.endsWith("/")) {
            flist_copyname = flist_copyname.left(flist_copyname.length() - 1);
        }
        menu->addAction(QString("Open Profile"), this, SLOT(openProfile()));
        // todo: Get the list of available sessions. Create a submenu with all available characters if there is more than one (or this session isn't vlaid).
        menu->addAction(QString("Open PM"), this, SLOT(openPrivateMessage()));
        menu->addAction(QString("Copy Profile Link"), this, SLOT(copyLink()));
        menu->addAction(QString("Copy Name"), this, SLOT(copyName()));
        menu->addSeparator();
    } else if (flist_copylink.startsWith("#AHI-")) {
        flist_copyname = flist_copylink.mid(5);
        // todo: Get the list of available sessions. Create a submenu with all available characters if there is more than one (or this session isn't vlaid).
        menu->addAction(QString("Join Channel"), this, SLOT(joinChannel()));
        // todo: Maybe get the name the plain text of the link and make that available for copying?
        menu->addAction(QString("Copy Channel ID"), this, SLOT(copyName()));
        menu->addSeparator();
    } else if (flist_copylink.startsWith("#CSA-")) {
        flist_copyname = flist_copylink.mid(5);
        // todo: If possible, get which session this actually came from and use that.
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

void FLogTextBrowser::setSessionID(QString sessionid) {
    this->sessionid = sessionid;
}

QString FLogTextBrowser::getSessionID() {
    return sessionid;
}

void FLogTextBrowser::openProfile() {
    if (ui) {
        ui->openCharacterProfile(ui->getSession(sessionid), flist_copyname);
    } else {
        // todo:
    }
}

void FLogTextBrowser::openWebProfile() {
    QDesktopServices::openUrl(flist_copylink);
}

void FLogTextBrowser::openPrivateMessage() {
    if (ui) {
        ui->addCharacterChat(ui->getSession(sessionid), flist_copyname);
    } else {
        // todo:
    }
}

void FLogTextBrowser::copyLink() {
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(flist_copylink, QClipboard::Clipboard);
    clipboard->setText(flist_copylink, QClipboard::Selection);
}

void FLogTextBrowser::copyName() {
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(flist_copyname, QClipboard::Clipboard);
    clipboard->setText(flist_copyname, QClipboard::Selection);
}

void FLogTextBrowser::joinChannel() {
    FSession *session = ui->getSession(sessionid);
    if (!session) {
        return;
    }
    session->joinChannel(flist_copyname);
}

void FLogTextBrowser::confirmReport() {
    FSession *session = ui->getSession(sessionid);
    if (!session) {
        return;
    }
    session->sendConfirmStaffReport(flist_copyname);
}

void FLogTextBrowser::append(const QString &text) {
    QString newText = identifyAndLoadServerResources(text);

    // debugMessage("Going to append: " + newText);

    QTextCursor cur = textCursor();
    if (cur.hasSelection()) {
        int oldPosition = cur.position();
        int oldAnchor = cur.anchor();

        int vpos = verticalScrollBar()->value();
        int vmax = verticalScrollBar()->maximum();

        QTextBrowser::append(newText);

        cur.setPosition(oldAnchor, QTextCursor::MoveAnchor);
        cur.setPosition(oldPosition, QTextCursor::KeepAnchor);
        setTextCursor(cur);
        if (vpos != vmax) {
            verticalScrollBar()->setValue(vpos);
        } else {
            vmax = verticalScrollBar()->maximum();
            verticalScrollBar()->setValue(vmax);
        }
    } else {
        QTextBrowser::append(newText);
    }
}

void FLogTextBrowser::animate(int id) {
    // debugMessage("FLogTextBrowser::animate() -> Called with ID: " + QString::number(id));

    if (id < 1 || urls.count() < id) {
        debugMessage("FLogTextBrowser::animate() -> Mapped out of bounds, exiting.");
        return;
    }

    QPair<QMovie *, QUrl> _data = urls.at(id - 1);

    // debugMessage("FLogTextBrowser::animate() -> Movie found with ID: " + QString::number(id));

    document()->addResource(QTextDocument::ImageResource, _data.second, _data.first->currentPixmap());
    setLineWrapColumnOrWidth(lineWrapColumnOrWidth());
}

void FLogTextBrowser::resourceLoaded(QNetworkReply *resource) {
    for (int i = 0; i < loadedResources.count(); i++) {
        QNetworkReply *item = loadedResources.at(i);

        if (item != resource || !item->isFinished()) {
            continue;
        }

        copyFileToCache(item->readAll(), resource->url());
    }
}

QString FLogTextBrowser::identifyAndLoadServerResources(const QString &text) {
    QString resultText = text;
    // debugMessage("FLogTextBrowser::identifyAndLoadServerResource() -> Called with input text: " + text);

    // this should capture all relevant image links (we only want eicons and avatars, nothing else!)
    static QRegularExpression reg(R"RX(<img class="e?icon" src="https://static.f-list.net/images/([^()"' ]*)*)RX",
                                  QRegularExpression::CaseInsensitiveOption | QRegularExpression::DotMatchesEverythingOption | QRegularExpression::MultilineOption
                                          | QRegularExpression::DontCaptureOption);

    // debugMessage("FLogTextBrowser::identifyAndLoadServerResource() -> Is Image RegEx valid? ->" + QString::number(reg.isValid()));

    QRegularExpressionMatchIterator imageMatches = reg.globalMatch(resultText, 0, QRegularExpression::NormalMatch, QRegularExpression::NoMatchOption);

    QStringList capturedTexts;

    while (imageMatches.hasNext()) {
        QRegularExpressionMatch match = imageMatches.next();

        // capture groups start at 0, so we do <=
        for (int i = 0; i <= reg.captureCount(); ++i) {
            QString item = match.captured(i);
            int indexCutoff = item.indexOf("https");
            item = item.sliced(indexCutoff);
            capturedTexts.append(item);
        }
    }

    int count = capturedTexts.count();

    if (count < 1) {
        // debugMessage("FLogTextBrowser::identifyAndLoadServerResource() -> Found no matches, exiting...");
        return resultText;
    }

    for (int i = 0; i < count; i++) {
        QString item = capturedTexts.at(i);
        // debugMessage("FLogTextBrowser::identifyAndLoadServerResource() -> Loading this resource: " + item);

        QUrl _url = QUrl(item);
        if (loadedUrls.value(_url)) {
            // already loaded
            continue;
        }

        QNetworkReply *reply = manager.get(QNetworkRequest(_url));
        loadedResources.append(reply);
    }

    return resultText;
}

QDir FLogTextBrowser::generateFoldersAndReturnDir(QUrl url) {
    QDir cache = QDir("cache");
    if (!cache.exists()) {
        QDir().mkdir("cache");
    }

    if (url.toString().contains("eicon")) {
        if (!cache.cd("eicon")) {
            cache.mkdir("eicon");
            cache.cd("eicon");
        }
    } else if (url.toString().contains("avatar")) {
        if (!cache.cd("avatar")) {
            cache.mkdir("avatar");
            cache.cd("avatar");
        }
    }

    return cache;
}

void FLogTextBrowser::copyFileToCache(QByteArray input, QUrl url) {
    QString path = generatePathToFile(url);
    QFile file = QFile(path);

    if (!file.open(QIODeviceBase::ReadWrite)) {
        debugMessage("FLogTextBrowser::copyFileToCache() -> File could not be opened for read/write, exiting.");
        return;
    }

    if (loadedUrls.value(url)) {
        // resource already exists
        // debugMessage("FLogTextBrowser::copyFileToCache() -> Skipping file that was already loaded and should either be playing or cached.");
        file.close();
        return;
    }

    file.write(input);
    file.close();

    loadedUrls.insert(url, true);

    QImageReader imgReader = QImageReader();
    QString imageFormat = QString(imgReader.imageFormat(path));

    // debugMessage("FLogTextBrowser::copyFileToCache() -> Image Format is: " + imageFormat);

    if (imageFormat.compare("gif", Qt::CaseInsensitive) == 0) {
        // debugMessage("FLogTextBrowser::copyFileToCache() -> Encountered gif, creating movie...");
        QMovie *movie = new QMovie(this);
        movie->setFileName(path);

        // debugMessage("FLogTextBrowser::copyFileToCache() -> Format: " + QString(movie->format()));
        // debugMessage("FLogTextBrowser::copyFileToCache() -> frameCount: " + QString::number(movie->frameCount()));
        // debugMessage("FLogTextBrowser::copyFileToCache() -> Speed: " + QString::number(movie->speed()));

        QPair<QMovie *, QUrl> _data;
        _data.first = movie;
        _data.second = url;
        urls.append(_data);
        // debugMessage("FLogTextBrowser::copyFileToCache() -> Added movie to URL list, mapping...");
        signalMapper.setMapping(movie, urls.count());
        // debugMessage("FLogTextBrowser::copyFileToCache() -> Connecting movie to signal mapper...");
        connect(movie, SIGNAL(frameChanged(int)), &signalMapper, SLOT(map()));
        // debugMessage("FLogTextBrowser::copyFileToCache() -> Starting movie.");

        // reset all gifs to keep them in sync
        int _count = urls.count();
        for (int i = 0; i < _count; i++) {
            QPair<QMovie *, QUrl> _data = urls.at(i);
            _data.first->stop();
            _data.first->start();
        }
    } else {
        // debugMessage("FLogTextBrowser::copyFileToCache() -> Adding resource for url:" + url.toString());
        document()->addResource(QTextDocument::ImageResource, url, input);
        // trigger re-render of our QTextBrowser to display newly loaded resource
        setLineWrapColumnOrWidth(lineWrapColumnOrWidth());
    }
}

QString FLogTextBrowser::generatePathToFile(QUrl url) {
    QDir cache = generateFoldersAndReturnDir(url);

    return cache.absoluteFilePath(url.fileName());
}
