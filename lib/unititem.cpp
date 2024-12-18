#include "unititem.h"

#include <QTime>
#include <QDaemonLog>
#include <QtMath>

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

	m_dPressElev = m_cSettings.value("PressureElevation", 0).toDouble();
	m_cSettings.setValue("PressureElevation", QVariant::fromValue(m_dPressElev));

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

	//m_pAlarms.insert(0 * 3600, new Alarm(this, 0, EVERYDAY, 120));

	Alarm::mapFromVariant(this, m_pAlarms, m_cSettings.value("Unit"));

	if (m_pAlarms.size() == 0) {
		//Alarm<UnitItem> *new_alarm = new Alarm<UnitItem>(this);
		//new_alarm->setDays(EVERYDAY);
		//new_alarm->setSeconds(8 * 3600); //Alarm at 8.
		//m_pAlarms.insert(new_alarm->getSeconds(), new_alarm);
		//m_pAlarms.insert(0 * 3600, new Alarm(this, 0 * 3600, EVERYDAY, 120));
		//m_pAlarms.insert(0 * 3600, new Alarm(this, 0 * 3600, EVERYDAY, 4 * 3600));
		m_pAlarms.insert(0 * 3600, new Alarm(this, 0 * 3600, EVERYDAY));
		m_pAlarms.insert(4 * 3600, new Alarm(this, 4 * 3600, EVERYDAY));
		m_pAlarms.insert(8 * 3600, new Alarm(this, 8 * 3600, EVERYDAY));
		m_pAlarms.insert(12 * 3600, new Alarm(this, 12 * 3600, EVERYDAY));
		m_pAlarms.insert(16 * 3600, new Alarm(this, 16 * 3600, EVERYDAY));
		m_pAlarms.insert(20 * 3600, new Alarm(this, 20 * 3600, EVERYDAY));
		//Unit=61847529062527 =  @ 0:00:00 + 4:00:00 interval
		//Unit=515396075647 = @ 0:00:00 + 0:02:00 interval
		//Unit=371085174374527, 371085178060927, 371085181747327, 371085185433727, 371085189120127, 371085192806527
		// = @ 0:00:00 + 0:00:00, 4:00:00, 8:00:00, 12:00:00, 16:00:00, 20:00:00 interval
	}

	m_iAlarmMinimumScheduling = m_cSettings.value("AlarmMinSched", ALARM_MIN_SCHEDULING).toUInt(); //Minimum time to schedule alarm (min time between releases)

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

	//if (m_pDataStream) {
	//	delete m_pDataStream;
	//	m_pDataStream = nullptr;
	//}
}

SlotItem *UnitItem::viewSlot(int column, int row)
{
	ColItem *pCol = col(column);

	if (pCol)
		return pCol->slot(pCol->getSlotCount() - 1 - row);

	return nullptr;
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

//void UnitItem::setDataServer(QString server)
//{
//	if (m_pDataStream) {
//		delete m_pDataStream;
//	}

//	m_pDataStream = new WebSocketClient(this, server);

//	connect(m_pDataStream, &WebSocketClient::connected, this, &UnitItem::serverReady);

        //disable alarms:
//	for (Alarm *alarm : m_pAlarms) {
//		alarm->disconnectTimer();
//	}
//}

void UnitItem::disconnectAlarms()
{
	//disable alarms:
	for (Alarm *alarm : m_pAlarms) {
		alarm->disconnectTimer();
	}
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

void UnitItem::setTemperature(double temp)
{
	if (temp != m_dTemperature) {
		m_dTemperature = temp;
		emit tempChanged(m_dTemperature);
	}
}

void UnitItem::setPressure(double pres)
{
	if (pres != m_dPressure) {
		m_dPressure = pres;
		emit presChanged(getQNH());
	}
}

void UnitItem::setHumidity(double humi)
{
	if (humi != m_dHumidity) {
		m_dHumidity = humi;
		emit humiChanged(m_dHumidity);
	}
}

void UnitItem::setQNH(double pres)
{
	setPressure(pres + m_dPressElev);
}

double UnitItem::getTemperature()
{
	return m_dTemperature;
}

double UnitItem::getQNH()
{
	return m_dPressure - m_dPressElev;
}

double UnitItem::getDewpoint()
{
	return (m_dTemperature - (14.55 + 0.114 * m_dTemperature) * (1 - (0.01 * m_dHumidity)) - qPow(((2.5 + 0.007 * m_dTemperature) * (1 - (0.01 * m_dHumidity))),3) - (15.9 + 0.117 * m_dTemperature) * qPow((1 - (0.01 * m_dHumidity)), 14));
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
		if (!slot || slot->getRelease() >= colSlot->getRelease())
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
			time = nextRelease.addMSecs(1); //advance time
		}
		//printReleases(releases); //DEBUG!!!
	}

	//times counted -> assing!
	for (ColItem &col : m_cCols) {
		col.assingReleases(releases.at(col.getId()));
	}

	return;
	//printReleases(releases);
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

__s8 UnitItem::countRows()
{
	__s8 rows = 0;
	for (const ColItem &col : m_cCols) {
		if (col.getSlotCount() > rows) {
			rows = col.getSlotCount();
		}
	}
	m_iRows = rows;

	return m_iRows;
}

void UnitItem::setCols(int i)
{
	if (m_cCols.size() == i)
		return;

	qDaemonLog(QString("Unit: cols changed %1 -> %2").arg(m_cCols.size()).arg(i), QDaemonLog::NoticeEntry);

	emit preColRemoved(0, m_cCols.size());
	if (m_cCols.size() != 0)
		m_cCols.clear();
	emit postColRemoved();


	emit preColAppended();
	m_cCols.resize(i);
	m_sUnit.ncols = m_cCols.size();

	initCols();
	emit postColAppended();

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
	int offset_seconds = offset.time().msecsSinceStartOfDay();
	int toGoSec = 24 * 86400 * 1000; //24 days in milliseconds;

	for(const Alarm *alarm : m_pAlarms) {
		int alarm_offset = alarm->getSeconds() * 1000;
		int alarm_interval = alarm->getInterval() * 1000;
		//int alarm_toGo = (((alarm_offset - offset_seconds) % alarm_interval) + alarm_interval) % alarm_interval;
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

void UnitItem::printReleases(QVector<QVector<QDateTime>> releases)
{
	for (int i = 0; i < releases.size(); ++i) {
		for (int k = 0; k < releases.at(i).size(); ++k) {
			qDebug() << QStringLiteral("Alarms %1/%2: at %3")
			          .arg(i)
			          .arg(k)
			          .arg(releases.at(i).at(k).toString());
		}
	}
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

		connect(&m_cCols[i], &ColItem::preSlotAppended, this, &UnitItem::preColSlotAppended);
		connect(&m_cCols[i], &ColItem::postSlotAppended, this, &UnitItem::postColSlotAppended);
		connect(&m_cCols[i], &ColItem::preSlotRemoved, this, &UnitItem::preColSlotRemoved);
		connect(&m_cCols[i], &ColItem::postSlotRemoved, this, &UnitItem::postColSlotRemoved);

		emit newCol(&m_cCols[i]);
	}
}

void UnitItem::saveAlarms()
{
	//m_cSettings.setValue("Unit", QVariant::fromValue(m_cAlarms));
	//m_cSettings.setValue("Unit", Alarm::toVariant(m_cAlarms));
	//m_cSettings.setValue("Unit", QVariant::fromValue(Alarm::toIntList(m_pAlarms)));
	if (m_pAlarms.size() == 1) {
		m_cSettings.setValue("Unit", m_pAlarms.first()->toInt());
	} else {
		m_cSettings.setValue("Unit", QVariant::fromValue(Alarm::toVariantList(m_pAlarms)));
	}

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

//void UnitItem::serverReady()
//{
        //Moved to WebSocketClient
        //m_pDataStream->getColCount();
        //m_pDataStream->getDoor();
        //m_pDataStream->getLight();
        //m_pDataStream->getNight();
        //m_pDataStream->getCharging();
//}
