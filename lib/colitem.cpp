#include "colitem.h"

#include <QTime>
#include <QDaemonLog>

#include "unititem.h"

ColItem::ColItem(UnitItem *parent)
        : m_pUnit(parent)
{
	//m_sCol.col_id;
	//m_sCol.slot_count;
	m_iAlarmMinimumScheduling = m_cSettings.value("AlarmMinSched", ALARM_MIN_SCHEDULING).toUInt(); //Minimum time to schedule alarm (min time between releases)
}

ColItem::ColItem(const ColItem &src)
        : Timer()
{
	m_pUnit = src.m_pUnit;
}

ColItem::~ColItem()
{
	saveAlarms();

	for (const Alarm *alarm : m_pAlarms) {
		delete alarm;
		alarm = nullptr;
	}

	m_pAlarms.clear();

	m_cSlots.clear();
}

SlotItem *ColItem::slot(int slot)
{
	if (slot < 0 || slot >= m_cSlots.length())
		return nullptr;

	return &m_cSlots[slot];
}

void ColItem::setColId(__s8 id)
{
	if (id < 0)
		return;

	m_sCol.col_id = id;

	//QList<int> alarmList = m_cSettings.value(QString("Col%1").arg(m_sCol.col_id)).value<QList<int>>();
	//Alarm::mapFromIntList(this, m_pAlarms, alarmList);
	Alarm::mapFromVariantList(this, m_pAlarms, m_cSettings.value(QString("Col%1").arg((int)m_sCol.col_id)).toList());
	Alarm::mapFromVariant(this, m_pAlarms, m_cSettings.value(QString("Col%1").arg((int)m_sCol.col_id)));
	//Alarm::mapFromVariant(this, m_pAlarms, m_cSettings.value("Unit"));

	//m_pAlarms.insert(0 * 3600, new Alarm(this, 0 * 3600, EVERYDAY, 300, 1));
	m_pAlarms.insert(0 * 3600, new Alarm(this, 0 * 3600, EVERYDAY));
	m_pAlarms.insert(4 * 3600, new Alarm(this, 4 * 3600, EVERYDAY));
	m_pAlarms.insert(8 * 3600, new Alarm(this, 8 * 3600, EVERYDAY));
	m_pAlarms.insert(20 * 3600, new Alarm(this, 20 * 3600, EVERYDAY));

	saveAlarms();
	//emit idChanged(this);
}

void ColItem::setAlarm(__u64 alarm)
{
	Alarm *newAlarm = new Alarm(this, &alarm);

	if (newAlarm->getDays() == NONE) {
		Alarm *toDelete = m_pAlarms[newAlarm->getSeconds()];
		m_pAlarms.remove(newAlarm->getSeconds());
		delete toDelete;
		delete newAlarm;
	} else if (m_pAlarms.contains(newAlarm->getSeconds())) {
		m_pAlarms[newAlarm->getSeconds()]->setDays(newAlarm->getDays());
		m_pAlarms[newAlarm->getSeconds()]->setInterval(newAlarm->getInterval());
		delete newAlarm;
		newAlarm = nullptr;
	} else {
		m_pAlarms[newAlarm->getSeconds()] = newAlarm;
	}
}

void ColItem::setSlots(int i)
{
	//QVector<QObject> lista;
	//lista.resize(10);

	qDaemonLog(QStringLiteral("ColItem slot count has changed %1 -> %2.").arg(m_cSlots.size()).arg(i), QDaemonLog::ErrorEntry);

	if (i == m_cSlots.size())
		return;

	emit preSlotRemoved(0, m_cSlots.size());
	if (m_cSlots.size())
		m_cSlots.clear();
	emit postSlotRemoved();

	emit preSlotAppended();
	m_cSlots.resize(i);
	m_sCol.slot_count = m_cSlots.size();

	initSlots();
	m_bInitialized = true;

	emit postSlotAppended();

	emit slotCountChanged(this);

	return;
}

void ColItem::addSlot()
{
	setSlots(m_cSlots.size() + 1);
	//SlotItem new_slot(this);
	//m_cSlots.append(new_slot);
	//m_cSlots.append();
	//m_cSlots.resize(m_cSlots.size() + 1);

	//m_sCol.slot_count = m_cSlots.size();
}

void ColItem::assingReleases(const QVector<QDateTime> list)
{
	for (int i = 0, k = 0; (k < m_cSlots.count()) && (i < list.count()); ++k) {
		if (m_cSlots.at(k).getFull()) {
			m_cSlots[k].setReleaseTime(list[i++]);
		}
	}
}

UnitItem *ColItem::getUnit() const
{
	return m_pUnit;
}

__s8 ColItem::getId() const
{
	if (m_sCol.col_id < 0 && m_pUnit) {
		//Error
		//m_pUnit->initCols();
		qDaemonLog(QStringLiteral("Uninitilized column."), QDaemonLog::ErrorEntry);
	}

	return m_sCol.col_id;
}

bool ColItem::isFull() const
{
	for (int i = 0; i < m_cSlots.size(); ++i) {
		if (!m_cSlots.at(i).getFull()) {
			return false;
		}
	}

	return true;
}

bool ColItem::isEmpty() const
{
	for (int i = 0; i < m_cSlots.size(); ++i) {
		if (m_cSlots.at(i).getFull()) {
			return false;
		}
	}

	return true;
}

int ColItem::countFull()
{
	int count = 0;

	for (const SlotItem &slot : m_cSlots) {
		count += slot.getFull();
	}
	m_iFullCount = count;

	return m_iFullCount;
}

SlotItem *ColItem::getNextReleaseSlot()
{
	for (SlotItem &slot : m_cSlots) {
		if (slot.getFull()) {
			return &slot;
		}
	}

	return nullptr;
}

long ColItem::getNextReleaseAlarm(long offset)
{
	long int next = INT_MAX;
	int current;

	for (const Alarm *alarm : m_pAlarms) {
		current = alarm->remainingTime();
		if (current < next && current - offset > 0)
			next = current;
	}

	return next;
}

QDateTime ColItem::getNextReleaseTime(QDateTime offset) //DateTime of next release.
{
	int offset_seconds = offset.time().msecsSinceStartOfDay();
	int toGoSec = 24 * 86400 * 1000; //24 days in milliseconds

	for(const Alarm *alarm : m_pAlarms) {
		int alarm_offset = alarm->getSeconds() * 1000;
		int alarm_interval = alarm->getInterval() * 1000;
		int alarm_toGo = (((alarm_offset - offset_seconds) % alarm_interval) + alarm_interval) % alarm_interval;
		if (alarm_toGo < m_iAlarmMinimumScheduling) {
			alarm_toGo += alarm_interval;
		}
		if (alarm_toGo < toGoSec
		                && ((1 << (offset.addMSecs(alarm_toGo).date().dayOfWeek() - 1)) & alarm->getDays())) {
			toGoSec = alarm_toGo;
		}
	}

	return offset.addMSecs(toGoSec);
}

void ColItem::releaseTimeout(Alarm *alarm)
{
	Q_UNUSED(alarm);

	qDaemonLog(QString("Col %4: Release timeout at %1. Is active %2, next in %3s")
	           .arg(QTime::currentTime().toString("hh:mm:ss"))
	           .arg(alarm->isActive())
	           .arg(alarm->getRemaining() / 1000)
	           .arg(m_sCol.col_id), QDaemonLog::NoticeEntry);

	emit releaseEvent(this);
}

void ColItem::timerStarted(Alarm *alarm)
{
	Q_UNUSED(alarm);

	emit alarmsChanged(this);
}

void ColItem::initSlots()
{
	for (int i = 0; i < m_cSlots.size(); ++i) {
		m_cSlots[i].setParentNid(this, i);
		//m_cSlots[i].setId(i);

		emit newSlot(&m_cSlots[i]);
	}
}

void ColItem::saveAlarms()
{
	//m_cSettings.setValue(QString("Col%1").arg(m_sCol.col_id), QVariant::fromValue(Alarm::toVariantList(m_pAlarms)));
	//m_cSettings.setValue(QString("Col%1").arg(m_sCol.col_id), QVariant::fromValue(Alarm::toVariantList(m_pAlarms)));
	if (m_pAlarms.size() == 0) {
		m_cSettings.setValue(QString("Col%1").arg((int)m_sCol.col_id), QVariant());
	} else if (m_pAlarms.size() == 1) {
		m_cSettings.setValue(QString("Col%1").arg((int)m_sCol.col_id), m_pAlarms.first()->toInt());
	} else {
		m_cSettings.setValue(QString("Col%1").arg((int)m_sCol.col_id), QVariant::fromValue(Alarm::toVariantList(m_pAlarms)));
	}

	m_cSettings.sync();

	qDaemonLog(QString("Alarms are synced. Count = %1").arg(m_pAlarms.count()), QDaemonLog::NoticeEntry);
}

