#ifndef WEBSOCKETSERVER_H
#define WEBSOCKETSERVER_H

#include <QObject>
#include <QtCore/QList>
#include <QtCore/QByteArray>
#include <QtNetwork/QSslError>
#include <QSettings>

#include "kernelclient.h"
#include "unititem.h"
#include "colitem.h"
#include "slotitem.h"

QT_FORWARD_DECLARE_CLASS(QWebSocketServer)
QT_FORWARD_DECLARE_CLASS(QWebSocket)

class WebSocketServer : public QObject
{
	Q_OBJECT
public:
	explicit WebSocketServer(quint16 port, UnitItem *datamodel);
	~WebSocketServer() override;

signals:

private slots:
	void onNewConnection();
	void processTextMessage(QString message);
	void processBinaryMessage(QByteArray message);
	void socketDisconnected();
	void onSslErrors(const QList<QSslError> &errors);

	void listen(UnitItem *unit);
	void connectCol(ColItem *newCol);
	void connectSlot(SlotItem *newslot);

	//Unit Slots:
	void bcastLight(__u8 light);
	void bcastDoor(__u8 door);
	void bcastNight(__u8 night);
	void bcastCharging(UnitItem *unit);
	void bcastAlarms(UnitItem *unit);
	void bcastColCount(UnitItem *unit);

	//Col Slots:
	void bcastColAlarms(ColItem *unit);
	void bcastNumSlots(ColItem *col);

	//Slot Slots:
	void bcastSlotState(SlotItem *slot);
	void bcastSlotReleaseTime(SlotItem * slot);

private:
	void bcastBinaryMessage(QByteArray data);
	void processSlotRequest(QDataStream &in, __u8 col, __u8 slot, enum DISPENSER_GENL_ATTRIBUTE attr);
	void processColRequest(QDataStream &in, __u8 col, __u8 slot, enum DISPENSER_GENL_ATTRIBUTE attr);
	void processUnitRequest(QDataStream &in, __u8 col, __u8 slot, enum DISPENSER_GENL_ATTRIBUTE attr);

	UnitItem *m_pUnit = nullptr;
	QWebSocketServer *m_pWebSocketServer = nullptr;
	QList<QWebSocket *> m_cClients;
	quint16 m_iPort = -1;
	QSettings m_cSettings;
};

#endif // WEBSOCKETSERVER_H
