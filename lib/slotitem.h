#ifndef SLOTITEM_H
#define SLOTITEM_H

#include <QObject>
#include <QSettings>

#include "lib_global.h"

#include <dispenser.h>

class ColItem;

class LIB_EXPORT SlotItem : public QObject
{
	Q_OBJECT
public:
	//explicit SlotItem();
	explicit SlotItem(ColItem *parent = nullptr);
	SlotItem(const SlotItem &src);
	~SlotItem();

	void setParent(ColItem *parent) { m_pCol = parent; }
	void setState(__u8 state);
	void setFull(bool full);
	void setUp(__u8 up);
	void setDown(__u8 down);
	void setRelease(__u8 release);
	void setFailedUp(__s32 failures);
	void setFailedDown(__s32 failures);

	ColItem *getCol();
	__s8 getId() { return slotId; }
	void setId(__s8 i) { slotId = i; emit idChanged(this); }
	bool getFull() { return m_bFull; }

	const struct dispenser_mmap_slot *getSlotStatus() { return &m_sSlot; }
	__s32 getFailedUp() { return m_sSlot.up_failed; }
	__s32 getFailedDown() { return m_sSlot.down_failed; }

signals:
	void stateChanged(enum slot_state up);
	void upChanged(char up);
	void downChanged(char down);
	void releaseChanged(char release);
	void fullChanged(bool full);
	void idChanged(SlotItem *slot);

private:
	QSettings m_cSettings;
	struct dispenser_mmap_slot m_sSlot = { UNKNOWN, 0, 0, 0, 0, 0 };
	ColItem *m_pCol = nullptr;
	__s8 slotId = -1;
	bool m_bFull = false;
};

#endif // SLOTITEM_H
