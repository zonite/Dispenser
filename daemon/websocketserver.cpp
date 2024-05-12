#include "websocketserver.h"

#include <QtWebSockets/QWebSocket>
#include <QtWebSockets/QWebSocketServer>
#include <QtNetwork/QSslCertificate>
#include <QtNetwork/QSslKey>


WebSocketServer::WebSocketServer(quint16 port, QObject *parent)
        : QObject{parent},
        m_pWebSocketServer(nullptr)
{

}
