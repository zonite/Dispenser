#ifndef WEBSOCKETCLIENT_H
#define WEBSOCKETCLIENT_H

#include <QObject>

#include <QWebSocket>

class WebSocketClient : public QObject
{
	Q_OBJECT
public:
	explicit WebSocketClient(QObject *parent = nullptr);

signals:

private slots:
	void onConnected();
	void closed();
	void textMessageReceived(QString message);
	void binaryMessageReceived(QByteArray message);
private:
	QWebSocket m_webSocket;
};

#endif // WEBSOCKETCLIENT_H
