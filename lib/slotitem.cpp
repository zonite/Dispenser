#include "slotitem.h"

#include <QDaemonLog>

#include "colitem.h"

SlotItem::SlotItem(ColItem *parent)
        : m_pCol(parent)
//SlotItem::SlotItem()
{
	/*
	m_sSlot.state;
	m_sSlot.up;
	m_sSlot.down;
	m_sSlot.release;
	m_sSlot.up_failed;
	m_sSlot.down_failed;
	*/
}

SlotItem::SlotItem(const SlotItem &src)
        : Timer()
{
	m_pCol = src.m_pCol;
	m_bFull = src.m_bFull;
	m_sSlot = src.m_sSlot;
}

SlotItem::~SlotItem()
{
	char col = m_pCol->getId();

	m_cSettings.beginGroup(QString("Col%1Slot%2").arg(QString::number(col)).arg(QString::number(m_iSlotId)));
	m_cSettings.setValue("state", m_sSlot.state);
	m_cSettings.setValue("full", m_bFull);
	m_cSettings.setValue("up", m_sSlot.up);
	m_cSettings.setValue("down", m_sSlot.down);
	m_cSettings.setValue("release", m_sSlot.release);
	m_cSettings.setValue("failed_up", m_sSlot.up_failed);
	m_cSettings.setValue("failed_down", m_sSlot.down_failed);
	m_cSettings.endGroup();

	qDaemonLog(QString("Slot %1/%2 syncing").arg(QString::number(col)).arg(QString::number(m_iSlotId)), QDaemonLog::NoticeEntry);
	m_cSettings.sync();
}

const char *SlotItem::stateToStr(slot_state state)
{
	switch (state) {
	case UNKNOWN:
		return "Unknown";
	case FAILED:
		return "Failed";
	case CLOSED:
		return "Closed";
	case RELEASE:
		return "Release";
	case OPENING:
		return "Opening";
	case OPEN:
		return "Open";
	case CLOSING:
		return "Closing";
	}
	return "Invalid";
}

void SlotItem::setState(__u8 state)
{
	enum slot_state saved_state = UNKNOWN;

	if (state == UNKNOWN) {
		char col = this->getCol()->getId();
		bool full;
		char up, down, release;
		int failed_up, failed_down;


		m_cSettings.beginGroup(QStringLiteral("Col%1Slot%2").arg(QString::number(col)).arg(QString::number(m_iSlotId)));
		saved_state = (enum slot_state) m_cSettings.value("state", UNKNOWN).toInt();
		full = m_cSettings.value("full", 0).toInt();
		up = m_cSettings.value("up", -1).toInt();
		down = m_cSettings.value("down", -1).toInt();
		release = m_cSettings.value("release", -1).toInt();
		failed_up = m_cSettings.value("failed_up", 0).toInt();
		failed_down = m_cSettings.value("failed_down", 0).toInt();
		m_cSettings.endGroup();

		if (up == m_sSlot.up && down == m_sSlot.down && release == m_sSlot.release) {
			state = saved_state;
		}

		if (state == UNKNOWN) {
			if (!m_sSlot.up && m_sSlot.down) {
				state = OPEN;
			} else if (!m_sSlot.down) {
				if (m_sSlot.up) {
					state = m_sSlot.release ? RELEASE : CLOSED;
				} else {
					state = CLOSING;
				}
			}
		}

		if (state != CLOSED && state != RELEASE)
			setFull(false);
		else
			setFull(full);

		setFailedUp(failed_up);
		setFailedDown(failed_down);
	}

	m_bInitialized = true;

	if (state != m_sSlot.state) {
		m_sSlot.state = (enum slot_state) state;
		emit stateChanged(this);
		//emit stateChanged(m_sSlot.state);
	}
}

void SlotItem::setFull(bool full)
{
	if (full != m_bFull) {
		m_bFull = full;

		m_pCol->getUnit()->countFull();
		emit fullChanged(this);
		//emit fullChanged(m_bFull);
		//m_pCol->getUnit()->assingReleases();
	}
}

void SlotItem::setUp(__u8 up)
{
	if (up != m_sSlot.up) {
		m_sSlot.up = up;

		emit upChanged(this);
		//emit upChanged(m_sSlot.up);
	}
}

void SlotItem::setDown(__u8 down)
{
	if (down != m_sSlot.down) {
		m_sSlot.down = down;

		emit downChanged(this);
		//emit downChanged(m_sSlot.down);
	}

}

void SlotItem::setRelease(__u8 release)
{
	if (release != m_sSlot.release) {
		m_sSlot.release = release;

		emit releaseChanged(this);
		//emit releaseChanged(m_sSlot.release);
	}
}

void SlotItem::setFailedUp(__s32 failures)
{
	if (failures != m_sSlot.up_failed) {
		m_sSlot.up_failed = failures;
	}
}

void SlotItem::setFailedDown(__s32 failures)
{
	if (failures != m_sSlot.down_failed) {
		m_sSlot.down_failed = failures;
	}
}

void SlotItem::setReleaseTime(QDateTime time)
{
	m_cReleaseTime = time;

	qDaemonLog(QStringLiteral("Slot %1/%2 release set to %3")
	           .arg(QString::number(m_pCol->getId()))
	           .arg(QString::number(m_iSlotId))
	           .arg(m_cReleaseTime.toString()), QDaemonLog::NoticeEntry);
	emit releaseTimeChanged(this);
}

ColItem *SlotItem::getCol() const
{
	if (!m_pCol) {
		qDaemonLog(QStringLiteral("Uninitilized column."), QDaemonLog::ErrorEntry);
		assert(true);
	}

	return m_pCol;
}

void SlotItem::setParentNid(ColItem *parent, __s8 i)
{
	m_pCol = parent;
	m_iSlotId = i;

	if (!m_pCol) {
		qDaemonLog(QStringLiteral("Uninitilized column."), QDaemonLog::ErrorEntry);
		return;
	}

	m_cSettings.beginGroup(QStringLiteral("Col%1Slot%2").arg(QString::number(m_pCol->getId())).arg(QString::number(m_iSlotId)));
	m_sSlot.state = (enum slot_state) m_cSettings.value("state", UNKNOWN).toInt();
	setFull(m_cSettings.value("full", 0).toInt());
	setUp(m_cSettings.value("up", -1).toInt());
	setDown(m_cSettings.value("down", -1).toInt());
	setRelease(m_cSettings.value("release", -1).toInt());
	setFailedUp(m_cSettings.value("failed_up", 0).toInt());
	setFailedDown(m_cSettings.value("failed_down", 0).toInt());
	m_cSettings.endGroup();
}

QString SlotItem::getStateStr() const
{
	return stateToStr(m_sSlot.state);
}

/*
void SlotItem::setId(__s8 i)
{
	slotId = i;
	emit idChanged(this);
}
*/
