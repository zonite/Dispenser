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

	void getColCount();
	void getDoor();
	void getLight();
	void getNight();
	void getCharging();


signals:
	void connected();

private slots:
	void onConnected();
	void closed();
	void error(QAbstractSocket::SocketError error);
	void textMessageReceived(QString message);
	void binaryMessageReceived(QByteArray message);

	//modify alarms
	void modifyAlarm(Alarm * alarm);

	//get col data
	void getSlotCount(ColItem *col);

	//get slot data
	void getSlotState(SlotItem *slot);

private:
	void processSlotMessage(QDataStream &in, __u8 col, __u8 slot, enum DISPENSER_GENL_ATTRIBUTE attr);
	void processColMessage(QDataStream &in, __u8 col, __u8 slot, enum DISPENSER_GENL_ATTRIBUTE attr);
	void processUnitMessage(QDataStream &in, __u8 col, __u8 slot, enum DISPENSER_GENL_ATTRIBUTE attr);

	QWebSocket m_webSocket;
	QUrl m_cDispenserAddress;
	UnitItem *m_pUnit;
};

#endif // WEBSOCKETCLIENT_H
