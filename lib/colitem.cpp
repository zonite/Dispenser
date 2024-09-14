#include "colitem.h"

#include "unititem.h"

ColItem::ColItem(UnitItem *parent)
        : m_pUnit(parent)
{
	//m_sCol.col_id;
	//m_sCol.slot_count;
}

ColItem::ColItem(const ColItem &src)
        : QObject()
{
	m_pUnit = src.m_pUnit;
}

ColItem::~ColItem()
{
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

	emit idChanged(this);
}

bool ColItem::setSlots(int i)
{
	//QVector<QObject> lista;
	//lista.resize(10);

	if (i == m_cSlots.size())
		return false;

	if (m_cSlots.size())
		m_cSlots.clear();

	m_cSlots.resize(i);
	m_sCol.slot_count = m_cSlots.size();

	return true;
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

char ColItem::getId()
{
	if (m_sCol.col_id < 0 && m_pUnit) {
		m_pUnit->initCols();
	}

	return m_sCol.col_id;
}

void ColItem::initSlots()
{
	for (int i = 0; i < m_cSlots.size(); ++i) {
		m_cSlots[i].setParent(this);
		m_cSlots[i].setId(i);
	}
}

