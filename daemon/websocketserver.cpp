#include "websocketserver.h"

#include <QtWebSockets/QWebSocket>
#include <QtWebSockets/QWebSocketServer>
#include <QtNetwork/QSslCertificate>
#include <QtNetwork/QSslKey>
#include <QtCore/QFile>

//openssl req -x509 -nodes -newkey rsa:4096 -keyout localhost.key -out localhost.cert -sha512 -days 365 -subj "/C=FI/ST=Helsinki/L=Helsinki/O=Dispenser/OU=Daemon/CN=localhost"

WebSocketServer::WebSocketServer(quint16 port, KernelClient *parent)
        : QObject{parent},
        m_pWebSocketServer(nullptr)
{
	m_pWebSocketServer = new QWebSocketServer(QStringLiteral("Dispenser Kernel Daemon"),
	                                          QWebSocketServer::SecureMode,
	                                          this);
	QSslConfiguration sslConfiguration;
	QFile certFile(QStringLiteral(":/localhost.cert"));
	QFile keyFile(QStringLiteral(":/localhost.key"));
	certFile.open(QIODevice::ReadOnly);
	keyFile.open(QIODevice::ReadOnly);
	QSslCertificate certificate(&certFile, QSsl::Pem);
	QSslKey sslKey(&keyFile, QSsl::Rsa, QSsl::Pem);
	certFile.close();
	keyFile.close();
	sslConfiguration.setPeerVerifyMode(QSslSocket::VerifyNone);
	sslConfiguration.setLocalCertificate(certificate);
	sslConfiguration.setPrivateKey(sslKey);
	m_pWebSocketServer->setSslConfiguration(sslConfiguration);

	if (m_pWebSocketServer->listen(QHostAddress::Any, port))
	{
		qDebug() << "Dispenser Kernel Daemon listening on port" << port;
		connect(m_pWebSocketServer, &QWebSocketServer::newConnection,
		        this, &WebSocketServer::onNewConnection);
		connect(m_pWebSocketServer, &QWebSocketServer::sslErrors,
		        this, &WebSocketServer::onSslErrors);
	}
}

WebSocketServer::~WebSocketServer()
{
	m_pWebSocketServer->close();
	qDeleteAll(m_clients.begin(), m_clients.end());
}


void WebSocketServer::onNewConnection()
{
	QWebSocket *pSocket = m_pWebSocketServer->nextPendingConnection();

	qDebug() << "Client connected:" << pSocket->peerName() << pSocket->origin();

	connect(pSocket, &QWebSocket::textMessageReceived, this, &WebSocketServer::processTextMessage);
	connect(pSocket, &QWebSocket::binaryMessageReceived, this, &WebSocketServer::processBinaryMessage);
	connect(pSocket, &QWebSocket::disconnected, this, &WebSocketServer::socketDisconnected);

	m_clients << pSocket;
}

void WebSocketServer::processTextMessage(QString message)
{
	QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
	if (pClient)
	{
		pClient->sendTextMessage(message);
	}
}

void WebSocketServer::processBinaryMessage(QByteArray message)
{
	QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
	if (pClient)
	{
		pClient->sendBinaryMessage(message);
	}
}

void WebSocketServer::socketDisconnected()
{
	qDebug() << "Client disconnected";
	QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
	if (pClient)
	{
		m_clients.removeAll(pClient);
		pClient->deleteLater();
	}
}

void WebSocketServer::onSslErrors(const QList<QSslError> &errors)
{
	Q_UNUSED(errors)

	qDebug() << "Dispenser: Ssl errors occurred";
}
