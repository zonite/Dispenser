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
	//Alarm::mapFromIntList(this, m_pAlarms, alarmList);

	//m_cAlarms = Alarm::fromVariant(m_cSettings.value("Unit"));

	m_pAlarms.insert(0 * 3600, new Alarm(this, 0, EVERYDAY, 120));
	//Alarm::mapFromVariantList(this, m_pAlarms, m_cSettings.value("Unit").toList());
	if (m_pAlarms.size() == 0) {
		//Alarm<UnitItem> *new_alarm = new Alarm<UnitItem>(this);
		//new_alarm->setDays(EVERYDAY);
		//new_alarm->setSeconds(8 * 3600); //Alarm at 8.
		//m_pAlarms.insert(new_alarm->getSeconds(), new_alarm);
		//m_pAlarms.insert(0 * 3600, new Alarm(this, 0 * 3600, EVERYDAY));
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

	if (m_pDataStream) {
		delete m_pDataStream;
		m_pDataStream = nullptr;
	}
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

void UnitItem::setDataServer(QString server)
{
	if (m_pDataStream) {
		delete m_pDataStream;
	}

	m_pDataStream = new WebSocketClient(this, server);

	connect(m_pDataStream, &WebSocketClient::connected, this, &UnitItem::serverReady);
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
	emit chargingChanged(this);
}

void UnitItem::setInitialized(qint8 init)
{
	m_sUnit.initialized = init;
}

Alarm *UnitItem::getAlarm(int i)
{
	auto k = m_pAlarms.cbegin(), end = m_pAlarms.cend();

	k += i;

	return k != end ? k.value() : nullptr;
}

SlotItem *UnitItem::getNextSlot()
{
	SlotItem *slot = nullptr;

	for (ColItem &col : m_cCols) {
		SlotItem *colSlot = col.getNextReleaseSlot();
		if (!colSlot)
			continue;
		if (slot || slot->getRelease() >= colSlot->getRelease())
			slot = colSlot;
	}
	return slot;
}

void UnitItem::assingReleases()
{
	int fullCount = m_iFullCount, i = 0;
	QVector<int> colReleased(m_cCols.count()); //index for releases.
	QVector<QVector <QDateTime>> releases;
	releases.resize(m_cCols.count());
	QDateTime time = QDateTime::currentDateTime();

	/*
	for (i = 0; i < m_cCols.count(); ++i) {
		//colFull[i] = m_cCols.at(i).getFullCount();
		releases[i].resize(m_cCols.at(i).getFullCount());
	}
	*/

	for (i = 0; i < fullCount; ++i) {
		QDateTime nextRelease = getNextRelease(time);
		const ColItem *releaseFrom = nullptr;
		int slotsLeft = 0;

		for (ColItem &col : m_cCols) {
			QDateTime colNextRelease = col.getNextReleaseTime(time);
			int colRemaining = 0;

			while (colNextRelease < nextRelease && colReleased.at(col.getId()) < col.getFullCount()) {
				//Col release before unit!
				//releases[col.getId()][colReleased[col.getId()]] = colNextRelease;
				releases[col.getId()].append(colNextRelease); //Append release time to col;
				++(colReleased[col.getId()]);
				++i;
				colNextRelease = col.getNextReleaseTime(colNextRelease); //advance time!
			} //This col is released to the point when unit will release next!

			colRemaining = col.getFullCount() - colReleased.at(col.getId());
			if (colRemaining >= slotsLeft) {
				releaseFrom = &col;
				slotsLeft = colRemaining;
			}
		} //Both cols are released until the point of unit being the next.

		if (releaseFrom && slotsLeft) {
			releases[releaseFrom->getId()].append(nextRelease); //append release time
			++(colReleased[releaseFrom->getId()]); //increase release count
			time = nextRelease; //advance time
		}
	}

	//times counted -> assing!
	for (ColItem &col : m_cCols) {
		col.assingReleases(releases.at(col.getId()));
	}
}

int UnitItem::countFull()
{
	int count = 0;

	for (ColItem &col : m_cCols) {
		count += col.countFull();
	}
	m_iFullCount = count;

	return m_iFullCount;
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

void UnitItem::setAlarm(__u64 alarm)
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

void UnitItem::addCol()
{
	setCols(m_cCols.size() + 1);
	//ColItem new_col(this);
	//m_cCols.append(new_col);
}

long UnitItem::getNextRelease(long offset) //Real timer remaining time
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

QDateTime UnitItem::getNextRelease(QDateTime offset) const //DateTime of next release.
{
	int offset_seconds = offset.time().msecsSinceStartOfDay() / 1000;
	int toGoSec = 86400 * 365; //Year in seconds;

	for(const Alarm *alarm : m_pAlarms) {
		int alarm_offset = alarm->getSeconds();
		int alarm_interval = alarm->getInterval();
		int alarm_toGo = (offset_seconds - alarm_offset) % alarm_interval;
		if (alarm_toGo < toGoSec
		                && ((1 << offset.addSecs(alarm_toGo).date().dayOfWeek()) & alarm->getDays())) {
			toGoSec = alarm_toGo;
		}
	}

	return offset.addSecs(toGoSec);
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

const QStringList UnitItem::toStatusStr() const
{
	QStringList list;
	int slotRows = 0, cur_slots;

	for (int i = 0; i < numCols(); ++i) {
		cur_slots = m_cCols.at(i).getSlotCount();
		if (cur_slots > slotRows)
			slotRows = cur_slots;
	}

	for (int i = slotRows - 1; i >= 0; --i) {
		QString line;
		QString times;
		line = QStringLiteral("%1\t").arg(QString::number(i));
		for (int k = 0; k < numCols(); ++k) {
			const ColItem *col = nullptr;
			const SlotItem *slot = nullptr;

			col = const_cast<const ColItem *>( &m_cCols.at(k) );
			if (col)
				slot = const_cast<const SlotItem *>( &col->getSlots()->at(i) );

			if (slot) {
				line += slot->getStateStr();
				times += QStringLiteral("\t");
				times += slot->getRelease().toString("HH:mm:ss");
			}

			line += QStringLiteral("\t");
		}
		line += times;
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

void UnitItem::timerStarted(Alarm *alarm)
{
	Q_UNUSED(alarm);

	emit alarmsChanged(this);
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

void UnitItem::serverReady()
{
	m_pDataStream->getColCount();
	m_pDataStream->getDoor();
	m_pDataStream->getLight();
	m_pDataStream->getNight();
	m_pDataStream->getCharging();
}
