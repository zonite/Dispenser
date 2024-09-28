#ifndef WEBSOCKETCLIENT_H
#define WEBSOCKETCLIENT_H

#include <QObject>
#include <QWebSocket>

#include "unititem.h"
#include "colitem.h"
#include "slotitem.h"

class UnitItem;

class WebSocketClient : public QObject
{
	Q_OBJECT
public:
	explicit WebSocketClient(UnitItem *unit, QString server);
	~WebSocketClient();

signals:

private slots:
	void onConnected();
	void closed();
	void textMessageReceived(QString message);
	void binaryMessageReceived(QByteArray message);
private:
	QWebSocket m_webSocket;
	QUrl m_cDispenserAddress;
	UnitItem *m_pUnit;
};

#endif // WEBSOCKETCLIENT_H
