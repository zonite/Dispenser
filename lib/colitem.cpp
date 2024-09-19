#include "colitem.h"

#include <QDaemonLog>

#include "unititem.h"

ColItem::ColItem(UnitItem *parent)
        : m_pUnit(parent)
{
	//m_sCol.col_id;
	//m_sCol.slot_count;
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
	Alarm::mapFromVariantList(this, m_pAlarms, m_cSettings.value(QString("Col%1").arg(m_sCol.col_id)).toList());

	//emit idChanged(this);
}

void ColItem::setSlots(int i)
{
	//QVector<QObject> lista;
	//lista.resize(10);

	if (i == m_cSlots.size())
		return;

	if (m_cSlots.size())
		m_cSlots.clear();

	m_cSlots.resize(i);
	m_sCol.slot_count = m_cSlots.size();

	initSlots();
	m_bInitialized = true;

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

char ColItem::getId() const
{
	if (m_sCol.col_id < 0 && m_pUnit) {
		//Error
		//m_pUnit->initCols();
		qDaemonLog(QStringLiteral("Uninitilized column."), QDaemonLog::ErrorEntry);
	}

	return m_sCol.col_id;
}

void ColItem::releaseTimeout(Alarm *alarm)
{
	Q_UNUSED(alarm);

	emit releaseEvent(this);
}

void ColItem::initSlots()
{
	for (int i = 0; i < m_cSlots.size(); ++i) {
		m_cSlots[i].setParentNid(this, i);
		//m_cSlots[i].setId(i);
	}
}

void ColItem::saveAlarms()
{
	//m_cSettings.setValue(QString("Col%1").arg(m_sCol.col_id), QVariant::fromValue(Alarm::toVariantList(m_pAlarms)));
	m_cSettings.setValue(QString("Col%1").arg(m_sCol.col_id), QVariant::fromValue(Alarm::toVariantList(m_pAlarms)));

	m_cSettings.sync();
}

