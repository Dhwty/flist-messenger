#ifndef FSERVER_H
#define FSERVER_H

#include <QObject>
#include <QHash>
#include <QString>

class FAccount;
class FCharacter;
class FSession;

class FServer : public QObject {
        Q_OBJECT
    public:
        explicit FServer(QObject *parent = 0);
        FAccount *addAccount();
        FSession *getSession(QString sessionid);

    signals:

    public slots:

    public:
        QString chatserver_host;
        QString chatserver_port;

        QList<FAccount *> accounts; //< User accounts that are logged on. (Should only be one.)
                                    // QHash<QString, FCharacterProfile *> charactercache; //< List of all known characters on the server.
};

#endif                              // FSERVER_H
