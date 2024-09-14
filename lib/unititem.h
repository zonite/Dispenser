#ifndef UNITITEM_H
#define UNITITEM_H

#include <QObject>
#include <QVector>
#include <QTimer>

#include "lib_global.h"

#include "colitem.h"
#include "slotitem.h"

//class SlotItem;

class LIB_EXPORT UnitItem : public QObject
{
	Q_OBJECT
public:
	explicit UnitItem(QObject *parent = nullptr);

	SlotItem *slot(int col, int slot);
	ColItem *col(int col);

	const struct dispenser_mmap_unit *getUnitStatus() { return &m_sUnit; }

	void setCounter(__u8);

	void setDoor(char state);
	void setNight(char state);
	void setLight(char state);
	void setCharging(char state);
	bool setInitialized(qint8 init);

	int numCols() { return m_sUnit.ncols; };
	void setCols(int i);
	void setSlots(int i);
	void addCol();

	qint8 initialized() { return m_sUnit.initialized; }
	void initCols();

signals:
	void counterChanged(__u8);
	void doorChanged(__u8);
	void lightChanged(__u8);
	void chargingChanged(__u8);
	void nightChanged(__u8);

private slots:
	void nightEnds();
	void nightStarts();

private:

	struct dispenser_mmap_unit m_sUnit = { 0, 0, 0, 0, 0, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 0, 0, 0, 0 };
	QVector<ColItem> m_cCols;

	QTimer nightStartTimer;
	QTimer nightEndTimer;
};

#endif // UNITITEM_H
