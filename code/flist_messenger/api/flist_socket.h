#ifndef FLIST_SOCKET_H
#define FLIST_SOCKET_H

#include <QObject>
#include <QTimer>
#include <QWebSocket>
#include <QMetaEnum>

class FSocket : public QObject {
        Q_OBJECT
    public:
        FSocket(QString host, QObject* parent = nullptr);
        void socketConnect();
        void send(QString message);

    signals:
        void socketConnected();
        void socketReceived(QString);
        void socketSSLErrors(QString errors);
        void socketError(QString error);

    private slots:
        void wSocketConnected();
        void wSocketReceived(QString message);
        void wSocketSSLErrors(QList<QSslError> errors);
        void wSocketError(QAbstractSocket::SocketError error);
        void wKeepAlive();

    private:
        QWebSocket* m_socket;
        QString m_host;
        QTimer m_keepAliveTimer;

        void startKeepAlive();
        void stopKeepAlive();
};

#endif // FLIST_SOCKET_H
