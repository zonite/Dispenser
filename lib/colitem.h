#ifndef COLITEM_H
#define COLITEM_H

#include <QObject>
#include <QVector>

#include "lib_global.h"

#include "slotitem.h"
#include "alarm.h"


class UnitItem;
class Alarm;
//template <typename T> class Alarm;

class LIB_EXPORT ColItem : public Timer
{
	Q_OBJECT
public:
	explicit ColItem(UnitItem *parent = nullptr);
	ColItem(const ColItem &src);
	~ColItem();


	SlotItem *slot(int slot);

	void setParent(UnitItem *unit) { m_pUnit = unit; }
	void setSlots(int i);
	void setColId(__s8 id);
	void addSlot();

	UnitItem *getUnit() const;
	char getId() const;
	bool getInitialized() const { return m_bInitialized; }
	char getSlotCount() const { return m_sCol.slot_count; }
	const QVector<SlotItem> *getSlots() const { return &m_cSlots; }
	bool isFull() const;
	bool isEmpty() const;

public slots:
	//void releaseTimeout(Alarm<ColItem> *alarm);
	void releaseTimeout(Alarm *alarm);
	void timerStarted(Alarm *alarm);

signals:
	void slotCountChanged(ColItem *col);
	void releaseEvent(ColItem *col);
	void newSlot(SlotItem *slot);
	void alarmsChanged(ColItem *col);
	//void idChanged(ColItem *col);

private:
	void initSlots();
	void saveAlarms();

	QSettings m_cSettings;
	QMap<int, Alarm *> m_pAlarms; //Release timer!
	//QMap<int, Alarm<ColItem> *> m_pAlarms; //Release timer!

	struct dispenser_mmap_column m_sCol = { .col_id = -1, .slot_count = -1 };
	QVector<SlotItem> m_cSlots;
	UnitItem *m_pUnit = nullptr;
	bool m_bInitialized = false;
};

#endif // COLITEM_H
