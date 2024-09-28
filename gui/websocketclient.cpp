#include "websocketclient.h"

#include <QWebSocket>

WebSocketClient::WebSocketClient(QObject *parent)
        : QObject{parent}
{
	m_cDispenserAddress = "https://dispenser128.nykyri.eu:8080";

	connect(&m_webSocket, &QWebSocket::connected, this, &WebSocketClient::onConnected);
	connect(&m_webSocket, &QWebSocket::disconnected, this, &WebSocketClient::closed);

	m_webSocket.open(m_cDispenserAddress);
}

void WebSocketClient::onConnected()
{
	qDebug() << "WebSocket connected";
	connect(&m_webSocket, &QWebSocket::textMessageReceived,
	        this, &WebSocketClient::textMessageReceived);
	connect(&m_webSocket, &QWebSocket::binaryMessageReceived,
	        this, &WebSocketClient::binaryMessageReceived);
	//m_webSocket.sendTextMessage(QStringLiteral("Hello, world!"));

}

void WebSocketClient::closed()
{
	qDebug() << "WebSocket closed";
}

void WebSocketClient::textMessageReceived(QString message)
{
	qDebug() << "Message received:" << message;
	//m_webSocket.close();
}

void WebSocketClient::binaryMessageReceived(QByteArray message)
{

}
