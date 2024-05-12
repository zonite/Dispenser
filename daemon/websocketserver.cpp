#include "websocketserver.h"

#include <QtWebSockets/QWebSocket>
#include <QtWebSockets/QWebSocketServer>
#include <QtNetwork/QSslCertificate>
#include <QtNetwork/QSslKey>


WebSocketServer::WebSocketServer(quint16 port, KernelClient *parent)
        : QObject{parent},
        m_pWebSocketServer(nullptr)
{

}

void WebSocketServer::onNewConnection()
{

}

void WebSocketServer::processTextMessage(QString message)
{

}

void WebSocketServer::processBinaryMessage(QByteArray message)
{

}

void WebSocketServer::socketDisconnected()
{

}

void WebSocketServer::onSslErrors(const QList<QSslError> &errors)
{

}
