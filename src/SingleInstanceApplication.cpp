#include "SingleInstanceApplication.h"
#include <QLocalSocket>

SingleInstanceApplication::SingleInstanceApplication(int &argc, char **argv, const QString &uniqueKey)
    : QApplication(argc, argv)
    , m_uniqueKey(uniqueKey)
    , m_server(nullptr)
    , m_shouldExit(false)
{
    QLocalSocket socket;
    socket.connectToServer(m_uniqueKey);
    if (socket.waitForConnected(500)) {
        m_shouldExit = true;
        return;
    }

    QLocalServer::removeServer(m_uniqueKey);

    m_server = new QLocalServer(this);
    if (!m_server->listen(m_uniqueKey)) {
        m_shouldExit = true;
        return;
    }

    m_shouldExit = false;
    connect(m_server, &QLocalServer::newConnection, this, &SingleInstanceApplication::handleNewConnection);
}

SingleInstanceApplication::~SingleInstanceApplication(){
    if (m_server) {
        m_server->close();
        QLocalServer::removeServer(m_uniqueKey);
    }
}

void SingleInstanceApplication::handleNewConnection(){
    QLocalSocket *socket = m_server->nextPendingConnection();
    if (socket) {
        socket->deleteLater();
    }
}
