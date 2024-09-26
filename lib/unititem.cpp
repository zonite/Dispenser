#include "unititem.h"

#include <QTime>
#include <QDaemonLog>

#include <localinfo.h>

Q_DECLARE_METATYPE(QList<int>)

UnitItem::UnitItem(QObject *parent)
        : Timer{parent}
{
	/*
	m_sUnit.button;
	m_sUnit.calib_data;
	m_sUnit.charging;
	m_sUnit.counter;
	m_sUnit.door;
	m_sUnit.humidity;
	m_sUnit.light;
	m_sUnit.ncols;
	m_sUnit.night;
	m_sUnit.nslots;
	m_sUnit.pressure;
	m_sUnit.temperature;
	*/
	int msetToSunrise = QTime::currentTime().msecsTo(LocalInfo::getSunrise());
	int msetToSunset = QTime::currentTime().msecsTo(LocalInfo::getSunset());

	if (msetToSunrise < 0)
		msetToSunrise += 86400000;
	if (msetToSunset < 0)
		msetToSunset += 86400000;

	m_sUnit.night = msetToSunrise > msetToSunset ? 0 : 1;

	connect(&nightStartTimer, &QTimer::timeout, this, &UnitItem::nightStarts);
	connect(&nightEndTimer, &QTimer::timeout, this, &UnitItem::nightEnds);

	nightStartTimer.setTimerType(Qt::VeryCoarseTimer);
	nightEndTimer.setTimerType(Qt::VeryCoarseTimer);

	nightStartTimer.setInterval(24*3600*1000);
	nightEndTimer.setInterval(24*3600*1000);

	nightStartTimer.start(msetToSunset);
	nightEndTimer.start(msetToSunrise);

	//qRegisterMetaTypeStreamOperators<QList<QVariant>>("QList<QVariant>");
	//qRegisterMetaTypeStreamOperators<QList<int>>("QList<int>");
	//qRegisterMetaTypeStreamOperators<Alarm>("Alarm");

	//m_cAlarms = Alarm::toAlarm(m_cSettings.value("Unit").value<QList<int>>());

	//qRegisterMetaTypeStreamOperators<QList<Alarm> >("QList<Alarm>");

	//m_cAlarms = m_cSettings.value("Unit").value<QList<Alarm> >();

	//m_cSettings.value("Unit").value<QList<QVariant>>();
	//Alarm al;

	//QVariant a = QVariant::fromValue(al);
	//QVariant list = QVariant::fromValue(m_cAlarms);
	//QList<QVariant> var = m_cAlarms;

	//int i = al;
	//QList<int> ilist(m_cAlarms);

	//a = al;
	//list = m_cAlarms;

	//QVariant list = m_cSettings.value("Unit");

	//QVariant list = m_cSettings.value("Unit").value<QList<QVariant>>();
	//QList<QVariant> list = m_cSettings.value("Unit").toList();

	//Alarm::mapFromIntList(this, m_pAlarms, alarmList);
	Alarm::mapFromVariantList(this, m_pAlarms, m_cSettings.value("Unit").toList());
	//Alarm::mapFromIntList(this, m_pAlarms, alarmList);

	//m_cAlarms = Alarm::fromVariant(m_cSettings.value("Unit"));

	if (m_pAlarms.size() == 0) {
		//Alarm<UnitItem> *new_alarm = new Alarm<UnitItem>(this);
		//new_alarm->setDays(EVERYDAY);
		//new_alarm->setSeconds(8 * 3600); //Alarm at 8.
		//m_pAlarms.insert(new_alarm->getSeconds(), new_alarm);
		m_pAlarms.insert(0 * 3600, new Alarm(this, 0 * 3600, EVERYDAY));
		//m_pAlarms.insert(4 * 3600, new Alarm(this, 4 * 3600, EVERYDAY));
		//m_pAlarms.insert(8 * 3600, new Alarm(this, 8 * 3600, EVERYDAY));
		//m_pAlarms.insert(12 * 3600, new Alarm(this, 12 * 3600, EVERYDAY));
		//m_pAlarms.insert(16 * 3600, new Alarm(this, 16 * 3600, EVERYDAY));
		//m_pAlarms.insert(20 * 3600, new Alarm(this, 20 * 3600, EVERYDAY));
	}

	saveAlarms();

	qDaemonLog(QString("Alarms are set. Count = %1").arg(m_pAlarms.count()), QDaemonLog::NoticeEntry);

	//Alarm::clean(m_cAlarms);
}

UnitItem::~UnitItem()
{
	saveAlarms();

	for (const Alarm *alarm : m_pAlarms) {
		delete alarm;
		alarm = nullptr;
	}

	m_pAlarms.clear();
}


SlotItem *UnitItem::slot(int column, int slot)
{
	ColItem *pCol = col(column);

	if (pCol)
		return pCol->slot(slot);

	return nullptr;
}

ColItem *UnitItem::col(int col)
{
	if (col < 0 || col >= m_cCols.length())
		return nullptr;

	return &m_cCols[col];
}

void UnitItem::setCounter(__u8 i)
{
	if (m_sUnit.counter != i) {
		if (i < m_sUnit.counter)
			qDaemonLog(QStringLiteral("Counter decreased!"), QDaemonLog::ErrorEntry);

		m_sUnit.counter = i;

		emit counterChanged(m_sUnit.counter);
	}
}

void UnitItem::setDoor(char state)
{
	qDaemonLog(QString("Set door = %1").arg(QString::number(state)), QDaemonLog::NoticeEntry);
	if (state == m_sUnit.door)
		return;

	m_sUnit.door = state;
	emit doorChanged(m_sUnit.door);
}

void UnitItem::setNight(char state)
{
	qDaemonLog(QString("Set night = %1").arg(QString::number(state)), QDaemonLog::NoticeEntry);
	if (state == m_sUnit.night)
		return;

	m_sUnit.night = state;
	emit nightChanged(m_sUnit.night);
}

void UnitItem::setLight(char state)
{
	qDaemonLog(QString("Set light = %1").arg(QString::number(state)), QDaemonLog::NoticeEntry);
	if (state == m_sUnit.light)
		return;

	m_sUnit.light = state;
	emit lightChanged(m_sUnit.light);
}

void UnitItem::setCharging(char state)
{
	qDaemonLog(QString("Set charging = %1").arg(QString::number(state)), QDaemonLog::NoticeEntry);
	if (state == m_sUnit.charging)
		return;

	m_sUnit.charging = state;
	emit chargingChanged(m_sUnit.charging);
}

void UnitItem::setInitialized(qint8 init)
{
	m_sUnit.initialized = init;
}

void UnitItem::setCols(int i)
{
	if (m_cCols.size() == i)
		return;

	qDaemonLog(QString("Unit: cols changed %1 -> %2").arg(m_cCols.size()).arg(i), QDaemonLog::NoticeEntry);

	if (m_cCols.size() != 0)
		m_cCols.clear();

	m_cCols.resize(i);
	m_sUnit.ncols = m_cCols.size();

	initCols();

	emit colsChanged(this);
}

void UnitItem::setSlots(int i)
{
	if (m_sUnit.nslots == i)
		return;

	m_sUnit.nslots = i;
}

void UnitItem::addCol()
{
	setCols(m_cCols.size() + 1);
	//ColItem new_col(this);
	//m_cCols.append(new_col);
}

void UnitItem::checkInitialized()
{
	if (m_bInitialized)
		return;

	for (int i = 0; i < m_cCols.size(); ++i) {
		const ColItem *col = &m_cCols.at(i);
		const QVector<SlotItem> *Slots = col->getSlots();

		if (!col->getInitialized())
			return;

		for (int k = 0; k < Slots->size(); ++k) {
			const SlotItem *slot = &Slots->at(k);

			if (!slot->getInitialized())
				return;
		}
	}
	m_bInitialized = true;

	emit initialized(this);
}

bool UnitItem::isFull()
{
	for (int i = 0; i < numCols(); ++i) {
		if (!m_cCols.at(i).isFull())
			return false;
	}

	return true;
}

bool UnitItem::isEmpty()
{
	for (int i = 0; i < numCols(); ++i) {
		if (!m_cCols.at(i).isEmpty())
			return false;
	}

	return true;
}

const QStringList UnitItem::toStatusStr()
{
	QStringList list;
	int slotRows = 0, cur_slots;

	for (int i = 0; i < numCols(); ++i) {
		cur_slots = m_cCols.at(i).getSlotCount();
		if (cur_slots > slotRows)
			slotRows = cur_slots;
	}

	for (int i = 0; i < slotRows; ++i) {
		QString line;
		for (int k = 0; k < numCols(); ++k) {
			SlotItem *slot = col(k)->slot(i);
			if (slot)
				line += slot->getStateStr();
			line += QStringLiteral("\t");
		}
		list << line;
	}
	return list;
}

void UnitItem::releaseTimeout(Alarm *alarm)
{
	Q_UNUSED(alarm);

	emit releaseEvent(this);

	qDaemonLog(QString("Unit: Release timeout at %1. Is active %2, next in %3s")
	           .arg(QTime::currentTime().toString("hh:mm:ss"))
	           .arg(alarm->isActive())
	           .arg(alarm->getRemaining() / 1000), QDaemonLog::NoticeEntry);

	/*
	int weekday = QDate::currentDate().dayOfWeek();

	if (weekday & alarm->getDays()) {
		emit releaseEvent(this);
	}
	*/
}

void UnitItem::initCols()
{
	for (int i = 0; i < m_cCols.size(); ++i) {
		m_cCols[i].setParent(this);
		m_cCols[i].setColId(i);

		emit newCol(&m_cCols[i]);

	}
}

void UnitItem::saveAlarms()
{
	//m_cSettings.setValue("Unit", QVariant::fromValue(m_cAlarms));
	//m_cSettings.setValue("Unit", Alarm::toVariant(m_cAlarms));
	//m_cSettings.setValue("Unit", QVariant::fromValue(Alarm::toIntList(m_pAlarms)));
	m_cSettings.setValue("Unit", QVariant::fromValue(Alarm::toVariantList(m_pAlarms)));

	m_cSettings.sync();
}

void UnitItem::nightEnds()
{
	setNight(0);
}

void UnitItem::nightStarts()
{
	setNight(1);
}
