#include "websocketclient.h"

#include <QWebSocket>

WebSocketClient::WebSocketClient(QObject *parent)
        : QObject{parent}
{

	connect(&m_webSocket, &QWebSocket::connected, this, &WebSocketClient::onConnected);
	connect(&m_webSocket, &QWebSocket::disconnected, this, &WebSocketClient::closed);

}

void WebSocketClient::onConnected()
{
	//if (m_debug)
	//	qDebug() << "WebSocket connected";
	connect(&m_webSocket, &QWebSocket::textMessageReceived,
	        this, &WebSocketClient::textMessageReceived);
	m_webSocket.sendTextMessage(QStringLiteral("Hello, world!"));

}

void WebSocketClient::closed()
{

}

void WebSocketClient::textMessageReceived(QString message)
{
	//if (m_debug)
	//    qDebug() << "Message received:" << message;
	m_webSocket.close();
}

void WebSocketClient::binaryMessageReceived(QByteArray message)
{

}
