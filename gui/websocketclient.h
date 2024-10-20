#ifndef WEBSOCKETCLIENT_H
#define WEBSOCKETCLIENT_H

#include <QObject>
#include <QWebSocket>
#include <QThread>

#include "unititem.h"
#include "colitem.h"
#include "slotitem.h"

class UnitItem;
class WebSocketClient;

class WebSocketWorker : public QObject
{
	Q_OBJECT

public:
	WebSocketWorker(WebSocketClient *client);
	~WebSocketWorker();

public slots:
	void wakeScreen();
	//void doSend();

signals:
	//void done(int result);

private:

	WebSocketClient *m_pClient = nullptr;
};


class WebSocketClient : public QObject
{
	Q_OBJECT
public:
	explicit WebSocketClient(UnitItem *unit, QString server);
	~WebSocketClient();

	void getColCount();
	void getDoor();
	void getLight();
	void getNight();
	void getCharging();


signals:
	void connected();
	void newData();

private slots:
	void onConnected();
	void closed();
	void error(QAbstractSocket::SocketError error);
	void sslError(const QList<QSslError> &error);
	void textMessageReceived(QString message);
	void binaryMessageReceived(QByteArray message);

	//modify alarms
	void modifyAlarm(Alarm * alarm);

	//get col data
	void getSlotCount(ColItem *col);

	//get slot data
	void getSlotState(SlotItem *slot);
	void getSlotReleaseTime(SlotItem *slot);

private:
	void processSlotMessage(QDataStream &in, __u8 col, __u8 slot, enum DISPENSER_GENL_ATTRIBUTE attr);
	void processColMessage(QDataStream &in, __u8 col, __u8 slot, enum DISPENSER_GENL_ATTRIBUTE attr);
	void processUnitMessage(QDataStream &in, __u8 col, __u8 slot, enum DISPENSER_GENL_ATTRIBUTE attr);

	QThread m_cWorker;
	QWebSocket m_webSocket;
	QUrl m_cDispenserAddress;
	UnitItem *m_pUnit;
};

#endif // WEBSOCKETCLIENT_H
