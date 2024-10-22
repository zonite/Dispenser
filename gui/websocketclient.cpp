#include "websocketclient.h"

#include <QWebSocket>
#include <QDataStream>
#include <QProcess>
#include <QCoreApplication>

WebSocketClient::WebSocketClient(UnitItem *unit, QString server)
        : QObject(unit)
{
	m_cDispenserAddress = server;
	m_pUnit = unit;

	connect(m_pUnit, &UnitItem::newCol, this, &WebSocketClient::getSlotCount);


	connect(&m_webSocket, &QWebSocket::connected, this, &WebSocketClient::onConnected);
	connect(&m_webSocket, &QWebSocket::disconnected, this, &WebSocketClient::onClosed);

	connect(&m_webSocket, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error),
	        [=](QAbstractSocket::SocketError error){ this->error(error); });
	connect(&m_webSocket, &QWebSocket::sslErrors, this, &WebSocketClient::sslError);

	connectServer();
	//m_webSocket.ignoreSslErrors();
	//m_webSocket.open(m_cDispenserAddress);
	//m_webSocket.ignoreSslErrors();

	WebSocketWorker *worker;
	worker = new WebSocketWorker(this);
	worker->moveToThread(&m_cWorker);
	connect(this, &WebSocketClient::newData, worker, &WebSocketWorker::wakeScreen);
	m_cWorker.start();
}

WebSocketClient::~WebSocketClient()
{
	m_cWorker.quit();
	disconnectServer();
	//m_webSocket.close();
	m_cWorker.wait();
}

void WebSocketClient::getColCount()
{
	QByteArray data;
	QDataStream out(&data, QIODevice::WriteOnly);
	NEW_UNIT_CMD(cmd, DISPENSER_GENL_COL_NUM);

	out << cmd.toInt;
	out << m_pUnit->numCols();

	m_webSocket.sendBinaryMessage(data);
}

void WebSocketClient::getDoor()
{
	QByteArray data;
	QDataStream out(&data, QIODevice::WriteOnly);
	NEW_UNIT_CMD(cmd, DISPENSER_GENL_UNIT_DOOR);

	out << cmd.toInt;
	out << m_pUnit->getDoor();

	m_webSocket.sendBinaryMessage(data);
}

void WebSocketClient::getLight()
{
	QByteArray data;
	QDataStream out(&data, QIODevice::WriteOnly);
	NEW_UNIT_CMD(cmd, DISPENSER_GENL_UNIT_LIGHT);

	out << cmd.toInt;
	out << m_pUnit->getLight();

	m_webSocket.sendBinaryMessage(data);
}

void WebSocketClient::getNight()
{
	QByteArray data;
	QDataStream out(&data, QIODevice::WriteOnly);
	NEW_UNIT_CMD(cmd, DISPENSER_GENL_UNIT_NIGHT);

	out << cmd.toInt;
	out << m_pUnit->getNight();

	m_webSocket.sendBinaryMessage(data);
}

void WebSocketClient::getCharging()
{
	QByteArray data;
	QDataStream out(&data, QIODevice::WriteOnly);
	NEW_UNIT_CMD(cmd, DISPENSER_GENL_UNIT_CHARGING);

	out << cmd.toInt;
	out << m_pUnit->getCharging();

	m_webSocket.sendBinaryMessage(data);
}

void WebSocketClient::connectServer()
{
	m_webSocket.ignoreSslErrors();
	m_webSocket.open(m_cDispenserAddress);
	m_webSocket.ignoreSslErrors();
}

void WebSocketClient::disconnectServer()
{
	m_webSocket.close();
}

bool WebSocketClient::isConnected()
{
	return m_webSocket.state() == QAbstractSocket::ConnectedState;
}

void WebSocketClient::onConnected()
{
	qDebug() << "WebSocket connected";
	connect(&m_webSocket, &QWebSocket::textMessageReceived,
	        this, &WebSocketClient::textMessageReceived);
	connect(&m_webSocket, &QWebSocket::binaryMessageReceived,
	        this, &WebSocketClient::binaryMessageReceived);
	//m_webSocket.sendTextMessage(QStringLiteral("Hello, world!"));

	emit connected();

	getColCount();
	getDoor();
	getLight();
	getNight();
	getCharging();
}

void WebSocketClient::onClosed()
{
	emit closed();
	qDebug() << "WebSocket closed";
}

void WebSocketClient::error(QAbstractSocket::SocketError error)
{
	qDebug() << "Error connecting to host:" << m_webSocket.errorString() << error;

	if (!m_webSocket.isValid()) {
		m_webSocket.open(m_cDispenserAddress);
	}
}

void WebSocketClient::sslError(const QList<QSslError> &error)
{
	qDebug() << "SSL error occured" << error;

	m_webSocket.ignoreSslErrors(error);
}

void WebSocketClient::textMessageReceived(QString message)
{
	qDebug() << "Message received:" << message;
	//m_webSocket.close();
}

void WebSocketClient::binaryMessageReceived(QByteArray message)
{
	QDataStream in(&message, QIODevice::ReadOnly);
	int i = __DISPENSER_GENL_ATTR_MAX + __DISPENSER_GENL_CMD_MAX; //Max data limit
	EMPTY_CMD(command);
	enum DISPENSER_GENL_ATTRIBUTE attr;
	__u8 cmd, col, slot;

	qDebug() << "Message from server" << m_webSocket.peerAddress();

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
			processSlotMessage(in, col, slot, attr);
			break;
		case DISPENSER_GENL_CMD_COL_STATUS: //u8 col, u8 slot, u32 counter
			qDebug() << "Col status cmd" << cmd;
			processColMessage(in, col, slot, attr);
			break;
		case DISPENSER_GENL_CMD_UNIT_STATUS: //u8 col, u8 slot, u8 state attr, u32 counter
			qDebug() << "Unit status cmd" << cmd;
			processUnitMessage(in, col, slot, attr);
			emit newData();
			break;
		case DISPENSER_GENL_CMD_ENVIRONMENT: //double temp, qnh, dewpoint
			qDebug() << "Environment msg" << cmd;
			processEnvMessage(in, col, slot, attr);
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

void WebSocketClient::modifyAlarm(Alarm *alarm)
{
	QByteArray data;
	QDataStream out(&data, QIODevice::WriteOnly);
	NEW_UNIT_CMD(cmd, DISPENSER_GENL_UNIT_ALARM);

	ColItem *col = qobject_cast<ColItem *>(sender());

	if (col != NULL) {
		cmd.fields.cmd = DISPENSER_GENL_CMD_COL_STATUS;
		cmd.fields.col = col->getId();
	}

	out << cmd.toInt;
	out << alarm->toInt();

	m_webSocket.sendBinaryMessage(data);
}

void WebSocketClient::getSlotCount(ColItem *col)
{
	QByteArray data;
	QDataStream out(&data, QIODevice::WriteOnly);
	NEW_COL_CMD(cmd, DISPENSER_GENL_SLOT_NUM, col->getId());

	if (col->getSlotCount() == 0) {
		qDebug() << "Col not initialized -> connect signals";

		connect(col, &ColItem::newSlot, this, &WebSocketClient::getSlotState);
		connect(col, &ColItem::newSlot, this, &WebSocketClient::getSlotReleaseTime);
		connect(col, &Timer::modifyAlarm, this, &WebSocketClient::modifyAlarm);
	}

	out << cmd.toInt;
	out << col->getSlotCount();

	m_webSocket.sendBinaryMessage(data);
}

void WebSocketClient::getSlotState(SlotItem *slot)
{
	QByteArray data;
	QDataStream out(&data, QIODevice::WriteOnly);
	NEW_SLOT_CMD(cmd, DISPENSER_GENL_SLOT_STATE, slot->getCol()->getId(), slot->getId());

	//Slot has no signals to connect to!
	/*
	if (!slot->getInitialized()) {
		qDebug() << "Slot" << slot->getCol()->getId() << "/" << slot->getId() << "not initialized -> connect signals";

		connect(slot, &SlotItem::signal, this, &WebSocketClient::slot);
	}
	*/

	out << cmd.toInt;
	out << slot->getState();

	m_webSocket.sendBinaryMessage(data);
}

void WebSocketClient::getSlotReleaseTime(SlotItem *slot)
{
	QByteArray data;
	QDataStream out(&data, QIODevice::WriteOnly);
	NEW_SLOT_CMD(cmd, DISPENSER_GENL_UNIT_ALARM, slot->getCol()->getId(), slot->getId());

	//Slot has no signals to connect to!
	/*
	if (!slot->getInitialized()) {
		qDebug() << "Slot" << slot->getCol()->getId() << "/" << slot->getId() << "not initialized -> connect signals";

		connect(slot, &SlotItem::signal, this, &WebSocketClient::slot);
	}
	*/
	__u32 val32 = 0;

	out << cmd.toInt;
	out << val32;

	m_webSocket.sendBinaryMessage(data);
}

void WebSocketClient::processSlotMessage(QDataStream &in, __u8 col, __u8 slot, DISPENSER_GENL_ATTRIBUTE attr)
{
	struct dispenser_mmap_slot new_slot_state = { UNKNOWN, 0, 0, 0, 0, 0 };
	__u8 status = 0, full = 0;
	__u16 val16 = 0;
	__u32 val32 = 0;
	__u64 val64 = 0;
	qint64 relOffset = 0;
	QDateTime releaseTime;
	SlotItem *pSlot = m_pUnit->slot(col, slot);
	qDebug() << "processSlotMessage col =" << col << "slot =" << slot;

	if (!pSlot) {
		qDebug() << QStringLiteral("WebSocketClient::processSlotMessage: Slot ptr == NULL, col = %1, slot = %2").arg(QString::number(col)).arg(QString::number(slot));
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

		dispenser_unpack_slot_status(status, &new_slot_state, &full);

		pSlot->setFull(full);
		pSlot->setUp(new_slot_state.up);
		pSlot->setDown(new_slot_state.down);
		pSlot->setRelease(new_slot_state.release);
		pSlot->setState(status);
		qDebug() << "DISPENSER_GENL_SLOT_STATUS: Slot" << col << "/" << slot << "state is" << pSlot->getStateStr() << "raw state=" << status << "up =" << new_slot_state.up << "down =" << new_slot_state.down << "release =" << new_slot_state.release << "full =" << full;
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
		pSlot->setUp(status >> UP_BIT & 1);
		pSlot->setDown(status >> DOWN_BIT & 1);
		pSlot->setRelease(status >> RELEASE_BIT & 1);
		pSlot->setState(status);
		qDebug() << "DISPENSER_GENL_SLOT_STATE: Slot" << col << "/" << slot << "state is" << pSlot->getStateStr() << "raw state=" << status << ", up =" << pSlot->getUp() << ", down =" << pSlot->getDown() << ", release =" << pSlot->getRel();
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
		//in >> releaseTime;
		//pSlot->setReleaseTime(releaseTime);
		in >> relOffset;
		pSlot->setReleaseTime(QDateTime::currentDateTime().addMSecs(relOffset));
		qDebug() << "Ignore DISPENSER_GENL_UNIT_ALARM.";
		break;
	case __DISPENSER_GENL_ATTR_MAX:
		in >> status;
		qDebug() << "Ignore .";
		break;
	}

}

void WebSocketClient::processColMessage(QDataStream &in, __u8 col, __u8 slot, DISPENSER_GENL_ATTRIBUTE attr)
{
	__u8 status = 0;
	__u16 val16 = 0;
	__u32 val32 = 0;
	__u64 val64 = 0;
	__s32 alarm = 0;
	ColItem *pCol = m_pUnit->col(col);
	qDebug() << "processColRequest col =" << col << "slot =" << slot;

	if (!pCol) {
		qDebug() << QStringLiteral("WebSocketServer::processColRequest: Col ptr == NULL, col = %1.").arg(QString::number(col));
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
		pCol->setSlots(status);
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
		in >> alarm;
		pCol->setAlarm(alarm);
		qDebug() << "Ignore DISPENSER_GENL_UNIT_ALARM.";
		break;
	case __DISPENSER_GENL_ATTR_MAX:
		in >> status;
		qDebug() << "Ignore __DISPENSER_GENL_ATTR_MAX.";
		break;
	}

}

void WebSocketClient::processUnitMessage(QDataStream &in, __u8 col, __u8 slot, DISPENSER_GENL_ATTRIBUTE attr)
{
	__u8 status = 0, colCount = 0;
	__u16 val16 = 0;
	__u32 val32 = 0;
	__u64 val64 = 0;
	__s32 alarm = 0;
	qDebug() << "processUnitMessage";

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
		m_pUnit->setCols(colCount);
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
		m_pUnit->setLight(status);
		break;
	case DISPENSER_GENL_UNIT_DOOR: //Unit door
		in >> status;
		m_pUnit->setDoor(status);
		//qDebug() << "Ignore DISPENSER_GENL_UNIT_DOOR.";
		break;
	case DISPENSER_GENL_UNIT_CHARGING: //Unit charging
		in >> status;
		m_pUnit->setCharging(status);
		//qDebug() << "Ignore DISPENSER_GENL_UNIT_CHARGING.";
		break;
	case DISPENSER_GENL_UNIT_NIGHT: //Unit night
		in >> status;
		m_pUnit->setNight(status);
		break;
	case DISPENSER_GENL_UNIT_ALARM: //Unit alarm
		in >> val32;
		m_pUnit->setAlarm(alarm);
		break;
	case __DISPENSER_GENL_ATTR_MAX:
		in >> status;
		qDebug() << "Ignore .";
		break;
	}

}

void WebSocketClient::processEnvMessage(QDataStream &in, __u8 col, __u8 slot, DISPENSER_GENL_ATTRIBUTE attr)
{
	__u8 status = 0, colCount = 0;
	__u16 val16 = 0;
	__u32 val32 = 0;
	__u64 val64 = 0;
	__s32 alarm = 0;
	double envData = 0;
	qDebug() << "processEnvMessage";

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
		in >> envData;
		m_pUnit->setTemperature(envData);
		qDebug() << "Received DISPENSER_GENL_TEMPERATURE.";
		break;
	case DISPENSER_GENL_PRESSURE: //u32 attr //raw pressure
		in >> envData;
		m_pUnit->setQNH(envData);
		qDebug() << "Received DISPENSER_GENL_PRESSURE.";
		break;
	case DISPENSER_GENL_HUMIDITY: //u32 attr //raw humidity
		in >> envData;
		m_pUnit->setHumidity(envData);
		qDebug() << "Received DISPENSER_GENL_HUMIDITY.";
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
		m_pUnit->setLight(status);
		break;
	case DISPENSER_GENL_UNIT_DOOR: //Unit door
		in >> status;
		m_pUnit->setDoor(status);
		//qDebug() << "Ignore DISPENSER_GENL_UNIT_DOOR.";
		break;
	case DISPENSER_GENL_UNIT_CHARGING: //Unit charging
		in >> status;
		m_pUnit->setCharging(status);
		//qDebug() << "Ignore DISPENSER_GENL_UNIT_CHARGING.";
		break;
	case DISPENSER_GENL_UNIT_NIGHT: //Unit night
		in >> status;
		m_pUnit->setNight(status);
		break;
	case DISPENSER_GENL_UNIT_ALARM: //Unit alarm
		in >> val32;
		m_pUnit->setAlarm(alarm);
		break;
	case __DISPENSER_GENL_ATTR_MAX:
		in >> status;
		qDebug() << "Ignore .";
		break;
	}

}


WebSocketWorker::WebSocketWorker(WebSocketClient *client)
{
	m_pClient = client;

	QCoreApplication::setOrganizationName(ORGANIZATION);
	QCoreApplication::setOrganizationDomain(DOMAIN);
	QCoreApplication::setApplicationName(APPNAME);

	//m_cSettings.value("DestAddresses");
}

WebSocketWorker::~WebSocketWorker()
{

}

void WebSocketWorker::wakeScreen()
{
	static QDateTime lastRun = QDateTime::currentDateTime();

	if (lastRun.secsTo(QDateTime::currentDateTime()) < 600) {
		//not yet at sleep
		return;
	}

	QProcess encode;
	QStringList args;
	args << QStringLiteral("dpms") << QStringLiteral("force") << QStringLiteral("on");

	encode.start(QStringLiteral("/usr/bin/xset"), args);
	encode.waitForFinished(2000); //recored control timeout
	encode.readAll();
}
