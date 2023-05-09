#include "flist_socket.h"

FSocket::FSocket(QString host, QObject *parent) : QObject{parent} {
    m_host = host;

    m_socket = new QWebSocket("file:///", QWebSocketProtocol::VersionLatest, this);

    connect(m_socket, SIGNAL(connected()), this, SLOT(wSocketConnected()));
    connect(m_socket, SIGNAL(textMessageReceived(QString)), this, SLOT(wSocketReceived(QString)));
    connect(m_socket, SIGNAL(errorOccurred(QAbstractSocket::SocketError)), this, SLOT(wSocketError(QAbstractSocket::SocketError)));
    connect(m_socket, SIGNAL(sslErrors(QList<QSslError>)), this, SLOT(wSocketSSLErrors(QList<QSslError>)));

    connect(&m_keepAliveTimer, SIGNAL(timeout()), this, SLOT(wKeepAlive()));
}

void FSocket::socketConnect() {
    qDebug() << "FSocket::socketConnect() - Opening connection to host:" << QUrl(m_host);
    m_socket->open(QUrl(m_host));
}

void FSocket::send(QString message) {
    if (m_socket == nullptr) {
        qDebug() << "FSocket::send() - Send was called but socket was destroyed earlier.";
        emit socketSSLErrors("Socket invalid, please reconnect.");
    }

    qDebug() << "FSocket::send() - Socket sending message:" << message;

    m_socket->sendTextMessage(message);
}

// Some of these are optional but I like having them here. - GH
void FSocket::wSocketConnected() {
    qDebug() << "FSocket::wSocketConnected() - Socket connected.";

    emit socketConnected();
}

void FSocket::wSocketReceived(QString message) {
    // qDebug() << "FSocket::wSocketReceived() - Socket recieved message:" << message;

    emit socketReceived(message);
}

void FSocket::wSocketSSLErrors(QList<QSslError> errors) {
    QString resultErrors;
    auto cend = errors.cend();
    for (auto cit = errors.cbegin(); cit != cend; ++cit) {
        if (!resultErrors.isEmpty()) {
            resultErrors.append(", ");
        }
        resultErrors.append(cit->errorString());
    }

    qDebug() << "FSocket::wSocketSSLErrors() - Socket encountered SSL errors. ->" << resultErrors;

    // proper output is handled by session
    emit socketSSLErrors(resultErrors);
}

void FSocket::wSocketError(QAbstractSocket::SocketError error) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QAbstractSocket::SocketError>();
    QString errorType = metaEnum.valueToKey(error);
    qDebug() << "FSocket::wSocketError() - Socket encountered error. ->" << errorType;

    m_socket->abort();
    m_socket->deleteLater();
    m_socket = nullptr;

    emit socketError(errorType);
}

void FSocket::wKeepAlive() {
    qDebug() << "FSocket::wKeepAlive() - Pinging server.";
    m_socket->ping();
}

void FSocket::startKeepAlive() {
    m_keepAliveTimer.setInterval(30000);
    m_keepAliveTimer.setSingleShot(false);

    m_keepAliveTimer.start();
}

void FSocket::stopKeepAlive() {
    if (m_keepAliveTimer.isActive()) {
        m_keepAliveTimer.stop();
    }
}
