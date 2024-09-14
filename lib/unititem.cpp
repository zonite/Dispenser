#include "unititem.h"

#include <QTime>
#include <QDaemonLog>

#include <localinfo.h>

UnitItem::UnitItem(QObject *parent)
        : QObject{parent}
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
	if (state == m_sUnit.door)
		return;

	m_sUnit.door = state;
	emit doorChanged(m_sUnit.door);
}

void UnitItem::setNight(char state)
{
	if (state == m_sUnit.night)
		return;

	m_sUnit.night = state;
	emit nightChanged(m_sUnit.night);
}

void UnitItem::setLight(char state)
{
	if (state == m_sUnit.light)
		return;

	m_sUnit.light = state;
	emit lightChanged(m_sUnit.light);
}

void UnitItem::setCharging(char state)
{
	if (state == m_sUnit.charging)
		return;

	m_sUnit.charging = state;
	emit chargingChanged(m_sUnit.charging);
}

bool UnitItem::setInitialized(qint8 init) //returns true, if update to kernel needs to be sent
{
	if (m_sUnit.initialized)
		return false;

	if (init) { //Kernel portion is initialized


		return false;
	} else { //Kernel portion is not initialized

		m_sUnit.initialized = 1;
		return true;
	}
}

void UnitItem::setCols(int i)
{
	if (m_cCols.size() == i)
		return;

	if (m_cCols.size() != 0)
		m_cCols.clear();

	m_cCols.resize(i);
	m_sUnit.ncols = m_cCols.size();
}

void UnitItem::setSlots(int i)
{
	if (m_sUnit.nslots == i)
		return;

	m_sUnit.ncols = i;
}

void UnitItem::addCol()
{
	setCols(m_cCols.size() + 1);
	//ColItem new_col(this);
	//m_cCols.append(new_col);
}

void UnitItem::initCols()
{
	for (int i = 0; i < m_cCols.size(); ++i) {
		m_cCols[i].setParent(this);
		m_cCols[i].setColId(i);
	}
}

void UnitItem::nightEnds()
{
	setNight(0);
}

void UnitItem::nightStarts()
{
	setNight(1);
}
