#include "simulator.h"

#include <QDebug>


Simulator::Simulator(UnitItem *unit)
{
	m_pUnit = unit;

	m_cTimer.setTimerType(Qt::VeryCoarseTimer);

	if (m_pUnit->isEmpty()) {
		m_cTimer.setInterval(30000);
		m_iPhase = OPEN;
		m_pSlot = (SlotItem *)&m_pUnit->col(m_pUnit->numCols() - 1)->getSlots()->last();
		connect(&m_cTimer, &QTimer::timeout, this, &Simulator::close);
	} else {
		m_cTimer.setInterval(2000);
		m_iPhase = RELEASE;
		m_pSlot = m_pUnit->getNextSlot();
		connect(&m_cTimer, &QTimer::timeout, this, &Simulator::open);
	}
	//m_pSlot = m_pUnit->getNextSlot();

	m_cTimer.start();

	qDebug() << "Simulator Starting!";
}

void Simulator::open()
{
	switch (m_iPhase) {
	case RELEASE:
		m_pSlot->setRelease(1);
		m_pSlot->setState(slot_state::RELEASE);
		m_iPhase = OPENING;
		break;
	case OPENING:
		m_pSlot->setUp(0);
		m_pSlot->setState(slot_state::OPENING);
		m_pSlot->setFull(0);
		m_iPhase = OPEN;
		break;
	case OPEN:
		m_pSlot->setRelease(0);
		m_pSlot->setDown(1);
		m_pSlot->setState(slot_state::OPEN);
		m_iPhase = OPEN;
		m_cTimer.stop();
		deleteLater();
		break;
	case CLOSING:
		break;
	case CLOSED:
		break;
	}
}

void Simulator::close()
{
	m_cTimer.setInterval(2000);

	switch (m_iPhase) {
	case RELEASE:
		m_pSlot->setRelease(1);
		m_pSlot->setState(slot_state::RELEASE);
		m_iPhase = OPENING;
		break;
	case OPENING:
		m_pSlot->setUp(0);
		m_pSlot->setState(slot_state::OPENING);
		m_pSlot->setFull(0);
		m_iPhase = OPEN;
		break;
	case OPEN:
		m_pSlot->setDown(0);
		m_pSlot->setState(slot_state::CLOSING);
		m_iPhase = CLOSING;
		break;
	case CLOSING:
		m_pSlot->setUp(1);
		m_pSlot->setState(slot_state::CLOSING);
		m_iPhase = CLOSED;
		break;
	case CLOSED:
		ColItem *col = m_pSlot->getCol();
		if (!m_pSlot->getId()) {
			if (!col->getId()) {
				deleteLater();
				return;
			}
			col = m_pUnit->col(col->getId() - 1);
			m_pSlot = (SlotItem *) &col->getSlots()->last();
		} else {
			m_pSlot = col->slot(m_pSlot->getId() - 1);
		}
		m_iPhase = OPEN;
		break;
	}
}

