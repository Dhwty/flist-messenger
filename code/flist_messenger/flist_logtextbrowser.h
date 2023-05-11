#ifndef FLIST_LOGTEXTBROWSER_H
#define FLIST_LOGTEXTBROWSER_H

#include <QString>
#include <QTextBrowser>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QSignalMapper>

class iUserInterface;

class FLogTextBrowser : public QTextBrowser {
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
        void append(const QString &text);

    private slots:
        void animate(int id);
        void resourceLoaded(QNetworkReply *resource);

    private:
        QSignalMapper signalMapper;
        QNetworkAccessManager manager;
        QString identifyAndLoadServerResources(const QString &text);
        QDir generateFoldersAndReturnDir(QUrl url);
        void copyFileToCache(QByteArray input, QUrl url);
        QString generatePathToFile(QUrl url);
        QList<QNetworkReply *> loadedResources;
        QList<QPair<QMovie *, QUrl>> urls;
        QHash<QUrl, bool> loadedUrls;

        QString flist_copylink;
        QString flist_copyname;
        QString sessionid;
        iUserInterface *ui;
};

#endif // FLIST_LOGTEXTBROWSER_H
