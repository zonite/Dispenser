#ifndef COLITEM_H
#define COLITEM_H

#include <QObject>
#include <QVector>

#include "lib_global.h"

#include "slotitem.h"

class UnitItem;

class LIB_EXPORT ColItem : public QObject
{
	Q_OBJECT
public:
	explicit ColItem(UnitItem *parent = nullptr);
	ColItem(const ColItem &src);
	~ColItem();

	void initSlots();

	SlotItem *slot(int slot);

	void setColId(__s8 id);
	bool setSlots(int i);
	void addSlot();
	char getId();
	char getSlotCount() { return m_sCol.slot_count; }

signals:
	void idChanged(ColItem *col);

private:
	struct dispenser_mmap_column m_sCol = { .col_id = -1, .slot_count = -1 };
	QVector<SlotItem> m_cSlots;
	UnitItem *m_pUnit = nullptr;
};

#endif // COLITEM_H
