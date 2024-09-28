#include "websocketserver.h"

#include <QtWebSockets/QWebSocket>
#include <QtWebSockets/QWebSocketServer>
#include <QtNetwork/QSslCertificate>
#include <QtNetwork/QSslKey>
#include <QtCore/QFile>
#include <QDaemonLog>

//openssl req -x509 -nodes -newkey rsa:4096 -keyout localhost.key -out localhost.cert -sha512 -days 365 -subj "/C=FI/ST=Helsinki/L=Helsinki/O=Dispenser/OU=Daemon/CN=localhost"

WebSocketServer::WebSocketServer(quint16 port, UnitItem *datamodel)
        : QObject(datamodel),
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

	m_iPort = m_cSettings.value("port", port).toUInt();

	m_pUnit = datamodel;
	connect(m_pUnit, &UnitItem::newCol, this, &WebSocketServer::connectCol);
	connect(m_pUnit, &UnitItem::initialized, this, &WebSocketServer::listen, Qt::QueuedConnection);
	connect(m_pUnit, &UnitItem::lightChanged, this, &WebSocketServer::bcastLight, Qt::QueuedConnection);
	connect(m_pUnit, &UnitItem::doorChanged, this, &WebSocketServer::bcastDoor, Qt::QueuedConnection);
	connect(m_pUnit, &UnitItem::nightChanged, this, &WebSocketServer::bcastNight, Qt::QueuedConnection);
	connect(m_pUnit, &UnitItem::chargingChanged, this, &WebSocketServer::bcastCharging, Qt::QueuedConnection);
	connect(m_pUnit, &UnitItem::alarmsChanged, this, &WebSocketServer::bcastAlarms, Qt::QueuedConnection);
}

WebSocketServer::~WebSocketServer()
{
	m_pWebSocketServer->close();
	qDeleteAll(m_cClients.begin(), m_cClients.end());
}


void WebSocketServer::onNewConnection()
{
	QWebSocket *pSocket = m_pWebSocketServer->nextPendingConnection();

	qDebug() << "Client connected:" << pSocket->peerName() << pSocket->origin();
	qDaemonLog(QStringLiteral("Client connected"), QDaemonLog::ErrorEntry);

	connect(pSocket, &QWebSocket::textMessageReceived, this, &WebSocketServer::processTextMessage);
	connect(pSocket, &QWebSocket::binaryMessageReceived, this, &WebSocketServer::processBinaryMessage);
	connect(pSocket, &QWebSocket::disconnected, this, &WebSocketServer::socketDisconnected);

	m_cClients << pSocket;
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
		QDataStream in(&message, QIODevice::ReadOnly);
		int i = __DISPENSER_GENL_ATTR_MAX + __DISPENSER_GENL_CMD_MAX; //Max data limit
		EMPTY_CMD(command);
		enum DISPENSER_GENL_ATTRIBUTE attr;
		__u8 cmd, col, slot;

		qDebug() << "Message from client" << pClient->peerAddress();

		while (!in.atEnd() && --i > 0) {
			in >> command.toInt;
			cmd = command.fields.cmd;
			attr = command.fields.attr;
			col = command.fields.col;
			slot = command.fields.slot;

			switch(cmd) {
			case DISPENSER_GENL_CMD_UNSPEC:
				qDebug() << "Incorret cmd" << cmd;
				break;
			case DISPENSER_GENL_CMD_RELEASE: //action by attributes
				qDebug() << "Ignore release cmd" << cmd;
				break;
			case DISPENSER_GENL_CMD_SLOT_STATUS: //u8 col, u8 slot, u8 state attr, u32 counter
				qDebug() << "Slot status cmd" << cmd;
				processSlotRequest(in, col, slot, attr);
				break;
			case DISPENSER_GENL_CMD_COL_STATUS: //u8 col, u8 slot, u32 counter
				qDebug() << "Col status cmd" << cmd;
				processColRequest(in, col, slot, attr);
				break;
			case DISPENSER_GENL_CMD_UNIT_STATUS: //u8 col, u8 slot, u8 state attr, u32 counter
				qDebug() << "Unit status cmd" << cmd;
				processUnitRequest(in, col, slot, attr);
				break;
			case DISPENSER_GENL_CMD_ENVIRONMENT: //u32 temp, u32 press, u32 humid, u32 counter //raw temperature
				qDebug() << "Ignore environment cmd" << cmd;
				processUnitRequest(in, col, slot, attr);
				break;
			case DISPENSER_GENL_CMD_TEMPERATURE_CALIBRATION: //u32 C0, u32 counter //calibration data: u16 t1, s16 t2, s16 t3
				qDebug() << "Ignore temp calibration cmd" << cmd;
				break;
			case DISPENSER_GENL_CMD_PRESSURE_CALIBRATION: //u32 C0-1, u32 counter //calibration data: u16 p1, s16 p2-9
				qDebug() << "Ignore temp calibration cmd" << cmd;
				break;
			case DISPENSER_GENL_CMD_HUMIDITY_CALIBRATION: //u32 C0, u32 counter //calibration data: u8 h1, s16 h2, u8 h3, s16 h4-5, u8 h6
				qDebug() << "Ignore temp calibration cmd" << cmd;
				break;
				//	DISPENSER_GENL_CMD_DUMP, //Dumps the mmap-area
			case __DISPENSER_GENL_CMD_MAX:
				break;
			}
		}
	}
}

void WebSocketServer::socketDisconnected()
{
	qDebug() << "Client disconnected";
	qDaemonLog(QStringLiteral("Client disconnected"), QDaemonLog::ErrorEntry);

	QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
	if (pClient)
	{
		m_cClients.removeAll(pClient);
		pClient->deleteLater();
	}
}

void WebSocketServer::onSslErrors(const QList<QSslError> &errors)
{
	Q_UNUSED(errors)

	qDebug() << "Dispenser: Ssl errors occurred";
}

void WebSocketServer::connectCol(ColItem *newCol)
{
	connect(newCol, &ColItem::newSlot, this, &WebSocketServer::connectSlot);
	connect(newCol, &ColItem::alarmsChanged, this, &WebSocketServer::bcastColAlarms);
}

void WebSocketServer::connectSlot(SlotItem *newSlot)
{
	connect(newSlot, &SlotItem::stateChanged, this, &WebSocketServer::bcastSlotState);
}

void WebSocketServer::bcastLight(__u8 light)
{
	QByteArray data;
	QDataStream out(&data, QIODevice::WriteOnly);
	NEW_UNIT_CMD(cmd, DISPENSER_GENL_UNIT_LIGHT);

	out << cmd.toInt;
	out << light;

	bcastBinaryMessage(data);
}

void WebSocketServer::bcastDoor(__u8 door)
{
	QByteArray data;
	QDataStream out(&data, QIODevice::WriteOnly);
	NEW_UNIT_CMD(cmd, DISPENSER_GENL_UNIT_DOOR);

	out << cmd.toInt;
	out << door;

	bcastBinaryMessage(data);
}


void WebSocketServer::bcastNight(__u8 night)
{
	QByteArray data;
	QDataStream out(&data, QIODevice::WriteOnly);
	NEW_UNIT_CMD(cmd, DISPENSER_GENL_UNIT_NIGHT);

	//NEW_UNIT_CMD(test, DISPENSER_GENL_UNIT_NIGHT);

	//union dispenser_cmd cmd = new_cmd;
	//cmd.fields.cmd = DISPENSER_GENL_CMD_UNIT_STATUS;
	//cmd.fields.attr = DISPENSER_GENL_UNIT_NIGHT;

	//struct dispenser_cmd tt = { .cmd = DISPENSER_GENL_CMD_UNIT_STATUS, .attr = DISPENSER_GENL_UNIT_NIGHT, .col = 0, .slot = 0 };
	//struct dispenser_cmd tu = {};
	//struct dispenser_cmd tg = { 0, 0, 0, 0 };
	//struct dispenser_cmd tdu = new_cmd;

	out << cmd.toInt;
	out << night;

	//data.append(DISPENSER_GENL_CMD_UNIT_STATUS);
	//data.append(DISPENSER_GENL_UNIT_NIGHT);
	//data.append(night);

	bcastBinaryMessage(data);
}

void WebSocketServer::bcastCharging(UnitItem *unit)
{
	QByteArray data;
	QDataStream out(&data, QIODevice::WriteOnly);
	NEW_UNIT_CMD(cmd, DISPENSER_GENL_UNIT_CHARGING);

	out << cmd.toInt;
	out << unit->getCharging();

	bcastBinaryMessage(data);
}

void WebSocketServer::bcastAlarms(UnitItem *unit)
{
	QByteArray data;
	QDataStream out(&data, QIODevice::WriteOnly);
	const QMap<int, Alarm *> cAlarms = unit->getAlarms();

	__u8 i = 0;

	for (Alarm * alarm : cAlarms) {
		NEW_UNIT_CMD(cmd, DISPENSER_GENL_UNIT_ALARM);
		cmd.fields.slot = i++;

		out << cmd.toInt;
		out << alarm->toInt();
	}

	/*
	for (int i = 0; i < unit->getAlarmCount(); ++i) {
		Alarm *nthAlarm = unit->getAlarm(i);
		if (unit->getAlarm(i))
		data.append(DISPENSER_GENL_UNIT_ALARM);
		data.append(unit->getAlarm(i));
	}
	*/

	bcastBinaryMessage(data);
}

void WebSocketServer::bcastColCount(UnitItem *unit)
{
	QByteArray data;
	QDataStream out(&data, QIODevice::WriteOnly);
	NEW_UNIT_CMD(cmd, DISPENSER_GENL_UNIT_CHARGING);

	out << cmd.toInt;
	out << unit->numCols();

	bcastBinaryMessage(data);
}

void WebSocketServer::bcastColAlarms(ColItem *col)
{
	QByteArray data;
	QDataStream out(&data, QIODevice::WriteOnly);
	const QMap<int, Alarm *> cAlarms = col->getAlarms();

	__u8 i = 0;
	__u8 colId = col->getId();

	for (Alarm * alarm : cAlarms) {
		NEW_COL_CMD(cmd, DISPENSER_GENL_UNIT_ALARM, col->getId());
		cmd.fields.col = colId;
		cmd.fields.slot = i++;

		out << cmd.toInt;
		out << alarm->toInt();
	}

	bcastBinaryMessage(data);
}

void WebSocketServer::bcastNumSlots(ColItem *col)
{
	QByteArray data;
	QDataStream out(&data, QIODevice::WriteOnly);
	NEW_COL_CMD(cmd, DISPENSER_GENL_SLOT_NUM, col->getId());

	out << cmd.toInt;
	out << col->getSlotCount();

	bcastBinaryMessage(data);
}

void WebSocketServer::bcastSlotState(SlotItem *slot)
{
	QByteArray data;
	QDataStream out(&data, QIODevice::WriteOnly);
	NEW_SLOT_CMD(cmd, DISPENSER_GENL_SLOT_STATE, slot->getCol()->getId(), slot->getId());

	out << cmd.toInt;
	out << slot->getState();

	bcastBinaryMessage(data);
}

void WebSocketServer::bcastBinaryMessage(QByteArray data)
{
	for (QWebSocket *client : m_cClients) {
		client->sendBinaryMessage(data);
	}
}

void WebSocketServer::processSlotRequest(QDataStream &in, __u8 col, __u8 slot, DISPENSER_GENL_ATTRIBUTE attr)
{
	__u8 status = 0;
	__u16 val16 = 0;
	__u32 val32 = 0;
	__u64 val64 = 0;
	SlotItem *pSlot = m_pUnit->slot(col, slot);
	qDebug() << "processSlotRequest col =" << col << "slot =" << slot;

	if (!pSlot) {
		qDaemonLog(QStringLiteral("WebSocketServer::processSlotRequest: Slot ptr == NULL, col = %1, slot = %2").arg(QString::number(col)).arg(QString::number(slot)), QDaemonLog::ErrorEntry);
		return;
	}

	switch (attr) {
	case DISPENSER_GENL_ATTR_UNSPEC:
		in >> status;
		qDebug() << "Ignore DISPENSER_GENL_ATTR_UNSPEC.";
		break;
	case DISPENSER_GENL_MEM_COUNTER: //u32 attr
		in >> val32;
		qDebug() << "Ignore DISPENSER_GENL_MEM_COUNTER.";
		break;
	case DISPENSER_GENL_RELEASE_COUNT: //u8 attr
		in >> status;
		qDebug() << "Ignore DISPENSER_GENL_RELEASE_COUNT.";
		break;
	case DISPENSER_GENL_COL_NUM: //u8 attr
		in >> status;
		qDebug() << "Ignore DISPENSER_GENL_COL_NUM.";
		break;
	case DISPENSER_GENL_SLOT_NUM: //u8 attr
		in >> status;
		qDebug() << "Ignore DISPENSER_GENL_SLOT_NUM.";
		break;
	case DISPENSER_GENL_SLOT_STATUS: //bitfield up,down,release,+enum state (5bits) (settable)
		in >> status;
		bcastSlotState(pSlot);
		break;
	case DISPENSER_GENL_SLOT_FAILED_UP: //u32 attr
		in >> val32;
		qDebug() << "Ignore DISPENSER_GENL_SLOT_FAILED_UP.";
		break;
	case DISPENSER_GENL_SLOT_FAILED_DOWN: //u32 attr
		in >> val32;
		qDebug() << "Ignore DISPENSER_GENL_SLOT_FAILED_DOWN.";
		break;
	case DISPENSER_GENL_UNIT_STATUS: //bitfield door,power,night,light (night+light settable)
		in >> status;
		qDebug() << "Ignore DISPENSER_GENL_UNIT_STATUS.";
		break;
	case DISPENSER_GENL_TEMPERATURE: //u32 attr //raw temperature
		in >> val32;
		qDebug() << "Ignore DISPENSER_GENL_TEMPERATURE.";
		break;
	case DISPENSER_GENL_PRESSURE: //u32 attr //raw pressure
		in >> val32;
		qDebug() << "Ignore DISPENSER_GENL_PRESSURE.";
		break;
	case DISPENSER_GENL_HUMIDITY: //u32 attr //raw humidity
		in >> val32;
		qDebug() << "Ignore DISPENSER_GENL_HUMIDITY.";
		break;
	case DISPENSER_GENL_CALIBRATION0: //calibration data u64
		in >> val64;
		qDebug() << "Ignore DISPENSER_GENL_CALIBRATION0.";
		break;
	case DISPENSER_GENL_CALIBRATION1: //calibration data s16
		in >> val16;
		qDebug() << "Ignore DISPENSER_GENL_CALIBRATION1.";
		break;
	case DISPENSER_GENL_INITIALIZED: //calibration data u8
		in >> status;
		qDebug() << "Ignore DISPENSER_GENL_INITIALIZED.";
		break;
	case DISPENSER_GENL_SLOT_STATE: //enum slot_state
		in >> status;
		bcastSlotState(pSlot);
		break;
	case DISPENSER_GENL_UNIT_LIGHT: //Unit light
		in >> status;
		qDebug() << "Ignore DISPENSER_GENL_UNIT_LIGHT.";
		break;
	case DISPENSER_GENL_UNIT_DOOR: //Unit door
		in >> status;
		qDebug() << "Ignore DISPENSER_GENL_UNIT_DOOR.";
		break;
	case DISPENSER_GENL_UNIT_CHARGING: //Unit charging
		in >> status;
		qDebug() << "Ignore DISPENSER_GENL_UNIT_CHARGING.";
		break;
	case DISPENSER_GENL_UNIT_NIGHT: //Unit night
		in >> status;
		qDebug() << "Ignore DISPENSER_GENL_UNIT_NIGHT.";
		break;
	case DISPENSER_GENL_UNIT_ALARM: //Unit alarm
		in >> val32;
		qDebug() << "Ignore DISPENSER_GENL_UNIT_ALARM.";
		break;
	case __DISPENSER_GENL_ATTR_MAX:
		in >> status;
		qDebug() << "Ignore .";
		break;
	}
}

void WebSocketServer::processColRequest(QDataStream &in, __u8 col, __u8 slot, DISPENSER_GENL_ATTRIBUTE attr)
{
	__u8 status = 0;
	__u16 val16 = 0;
	__u32 val32 = 0;
	__u64 val64 = 0;
	ColItem *pCol = m_pUnit->col(col);
	qDebug() << "processColRequest col =" << col << "slot =" << slot;

	if (!pCol) {
		qDaemonLog(QStringLiteral("WebSocketServer::processColRequest: Col ptr == NULL, col = %1.").arg(QString::number(col)), QDaemonLog::ErrorEntry);
		return;
	}

	switch (attr) {
	case DISPENSER_GENL_ATTR_UNSPEC:
		in >> status;
		qDebug() << "Ignore DISPENSER_GENL_ATTR_UNSPEC.";
		break;
	case DISPENSER_GENL_MEM_COUNTER: //u32 attr
		in >> val32;
		qDebug() << "Ignore DISPENSER_GENL_MEM_COUNTER.";
		break;
	case DISPENSER_GENL_RELEASE_COUNT: //u8 attr
		in >> status;
		qDebug() << "Ignore DISPENSER_GENL_RELEASE_COUNT.";
		break;
	case DISPENSER_GENL_COL_NUM: //u8 attr
		in >> status;
		qDebug() << "Ignore DISPENSER_GENL_COL_NUM.";
		break;
	case DISPENSER_GENL_SLOT_NUM: //u8 attr
		in >> status;
		bcastNumSlots(pCol);
		break;
	case DISPENSER_GENL_SLOT_STATUS: //bitfield up,down,release,+enum state (5bits) (settable)
		in >> status;
		qDebug() << "Ignore DISPENSER_GENL_SLOT_STATUS.";
		break;
	case DISPENSER_GENL_SLOT_FAILED_UP: //u32 attr
		in >> val32;
		qDebug() << "Ignore DISPENSER_GENL_SLOT_FAILED_UP.";
		break;
	case DISPENSER_GENL_SLOT_FAILED_DOWN: //u32 attr
		in >> val32;
		qDebug() << "Ignore DISPENSER_GENL_SLOT_FAILED_DOWN.";
		break;
	case DISPENSER_GENL_UNIT_STATUS: //bitfield door,power,night,light (night+light settable)
		in >> status;
		qDebug() << "Ignore DISPENSER_GENL_UNIT_STATUS.";
		break;
	case DISPENSER_GENL_TEMPERATURE: //u32 attr //raw temperature
		in >> val32;
		qDebug() << "Ignore DISPENSER_GENL_TEMPERATURE.";
		break;
	case DISPENSER_GENL_PRESSURE: //u32 attr //raw pressure
		in >> val32;
		qDebug() << "Ignore DISPENSER_GENL_PRESSURE.";
		break;
	case DISPENSER_GENL_HUMIDITY: //u32 attr //raw humidity
		in >> val32;
		qDebug() << "Ignore DISPENSER_GENL_HUMIDITY.";
		break;
	case DISPENSER_GENL_CALIBRATION0: //calibration data u64
		in >> val64;
		qDebug() << "Ignore DISPENSER_GENL_CALIBRATION0.";
		break;
	case DISPENSER_GENL_CALIBRATION1: //calibration data s16
		in >> val16;
		qDebug() << "Ignore DISPENSER_GENL_CALIBRATION1.";
		break;
	case DISPENSER_GENL_INITIALIZED: //calibration data u8
		in >> status;
		qDebug() << "Ignore DISPENSER_GENL_INITIALIZED.";
		break;
	case DISPENSER_GENL_SLOT_STATE: //enum slot_state
		in >> status;
		qDebug() << "Ignore DISPENSER_GENL_SLOT_STATE.";
		break;
	case DISPENSER_GENL_UNIT_LIGHT: //Unit light
		in >> status;
		qDebug() << "Ignore DISPENSER_GENL_UNIT_LIGHT.";
		break;
	case DISPENSER_GENL_UNIT_DOOR: //Unit door
		in >> status;
		qDebug() << "Ignore DISPENSER_GENL_UNIT_DOOR.";
		break;
	case DISPENSER_GENL_UNIT_CHARGING: //Unit charging
		in >> status;
		qDebug() << "Ignore DISPENSER_GENL_UNIT_CHARGING.";
		break;
	case DISPENSER_GENL_UNIT_NIGHT: //Unit night
		in >> status;
		qDebug() << "Ignore DISPENSER_GENL_UNIT_NIGHT.";
		break;
	case DISPENSER_GENL_UNIT_ALARM: //Unit alarm
		in >> val32;
		qDebug() << "Ignore DISPENSER_GENL_UNIT_ALARM.";
		break;
	case __DISPENSER_GENL_ATTR_MAX:
		in >> status;
		qDebug() << "Ignore .";
		break;
	}
}

void WebSocketServer::processUnitRequest(QDataStream &in, __u8 col, __u8 slot, DISPENSER_GENL_ATTRIBUTE attr)
{
	__u8 status = 0, colCount = 0;
	__u16 val16 = 0;
	__u32 val32 = 0;
	__u64 val64 = 0;
	qDebug() << "processUnitRequest";

	Q_UNUSED(col);
	Q_UNUSED(slot);

	switch (attr) {
	case DISPENSER_GENL_ATTR_UNSPEC:
		in >> status;
		qDebug() << "Ignore DISPENSER_GENL_ATTR_UNSPEC.";
		break;
	case DISPENSER_GENL_MEM_COUNTER: //u32 attr
		in >> val32;
		qDebug() << "Ignore DISPENSER_GENL_MEM_COUNTER.";
		break;
	case DISPENSER_GENL_RELEASE_COUNT: //u8 attr
		in >> status;
		qDebug() << "Ignore DISPENSER_GENL_RELEASE_COUNT.";
		break;
	case DISPENSER_GENL_COL_NUM: //u8 attr
		in >> colCount;
		bcastColCount(m_pUnit);
		break;
	case DISPENSER_GENL_SLOT_NUM: //u8 attr
		in >> status;
		qDebug() << "Ignore DISPENSER_GENL_SLOT_NUM.";
		break;
	case DISPENSER_GENL_SLOT_STATUS: //bitfield up,down,release,+enum state (5bits) (settable)
		in >> status;
		qDebug() << "Ignore DISPENSER_GENL_SLOT_STATUS.";
		break;
	case DISPENSER_GENL_SLOT_FAILED_UP: //u32 attr
		in >> val32;
		qDebug() << "Ignore DISPENSER_GENL_SLOT_FAILED_UP.";
		break;
	case DISPENSER_GENL_SLOT_FAILED_DOWN: //u32 attr
		in >> val32;
		qDebug() << "Ignore DISPENSER_GENL_SLOT_FAILED_DOWN.";
		break;
	case DISPENSER_GENL_UNIT_STATUS: //bitfield door,power,night,light (night+light settable)
		in >> status;
		qDebug() << "Ignore DISPENSER_GENL_UNIT_STATUS.";
		break;
	case DISPENSER_GENL_TEMPERATURE: //u32 attr //raw temperature
		in >> val32;
		qDebug() << "Ignore DISPENSER_GENL_TEMPERATURE.";
		break;
	case DISPENSER_GENL_PRESSURE: //u32 attr //raw pressure
		in >> val32;
		qDebug() << "Ignore DISPENSER_GENL_PRESSURE.";
		break;
	case DISPENSER_GENL_HUMIDITY: //u32 attr //raw humidity
		in >> val32;
		qDebug() << "Ignore DISPENSER_GENL_HUMIDITY.";
		break;
	case DISPENSER_GENL_CALIBRATION0: //calibration data u64
		in >> val64;
		qDebug() << "Ignore DISPENSER_GENL_CALIBRATION0.";
		break;
	case DISPENSER_GENL_CALIBRATION1: //calibration data s16
		in >> val16;
		qDebug() << "Ignore DISPENSER_GENL_CALIBRATION1.";
		break;
	case DISPENSER_GENL_INITIALIZED: //calibration data u8
		in >> status;
		qDebug() << "Ignore DISPENSER_GENL_INITIALIZED.";
		break;
	case DISPENSER_GENL_SLOT_STATE: //enum slot_state
		in >> status;
		qDebug() << "Ignore DISPENSER_GENL_SLOT_STATE.";
		break;
	case DISPENSER_GENL_UNIT_LIGHT: //Unit light
		in >> status;
		bcastLight(m_pUnit->getLight());
		break;
	case DISPENSER_GENL_UNIT_DOOR: //Unit door
		in >> status;
		bcastDoor(m_pUnit->getDoor());
		qDebug() << "Ignore DISPENSER_GENL_UNIT_DOOR.";
		break;
	case DISPENSER_GENL_UNIT_CHARGING: //Unit charging
		in >> status;
		bcastCharging(m_pUnit);
		qDebug() << "Ignore DISPENSER_GENL_UNIT_CHARGING.";
		break;
	case DISPENSER_GENL_UNIT_NIGHT: //Unit night
		in >> status;
		bcastNight(m_pUnit->getNight());
		break;
	case DISPENSER_GENL_UNIT_ALARM: //Unit alarm
		in >> val32;
		bcastAlarms(m_pUnit);
		break;
	case __DISPENSER_GENL_ATTR_MAX:
		in >> status;
		qDebug() << "Ignore .";
		break;
	}
}

void WebSocketServer::listen(UnitItem *unit)
{
	Q_UNUSED(unit)

	if (m_pWebSocketServer->listen(QHostAddress::Any, m_iPort))
	{
		qDebug() << "Dispenser Kernel Daemon listening on port" << m_iPort;
		connect(m_pWebSocketServer, &QWebSocketServer::newConnection,
		        this, &WebSocketServer::onNewConnection);
		connect(m_pWebSocketServer, &QWebSocketServer::sslErrors,
		        this, &WebSocketServer::onSslErrors);
	}

}
