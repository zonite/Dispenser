#ifndef UNITITEM_H
#define UNITITEM_H

#include <QObject>
#include <QVector>
#include <QTimer>
#include <QMap>

#include "lib_global.h"

#include "websocketclient.h"
#include "colitem.h"
#include "slotitem.h"
#include "alarm.h"
#include "timer.h"

//class SlotItem;

//class Alarm;
//template <typename T> class Alarm;

class LIB_EXPORT UnitItem : public Timer
{
	Q_OBJECT
public:
	explicit UnitItem(QObject *parent = nullptr);
	~UnitItem();

	SlotItem *slot(int col, int slot);
	ColItem *col(int col);

	const struct dispenser_mmap_unit *getUnitStatus() { return &m_sUnit; }

	void setDataServer(QString server);

	void setCounter(__u8);

	void setDoor(char state);
	void setNight(char state);
	void setLight(char state);
	void setCharging(char state);
	void setInitialized(qint8 init);

	__u8 getDoor() { return m_sUnit.door; }
	__u8 getNight() { return m_sUnit.night; }
	__u8 getLight() { return m_sUnit.light; }
	__u8 getCharging() { return m_sUnit.charging; }
	__u8 getAlarmCount() { return m_pAlarms.size(); }
	QMap<int, Alarm *> getAlarms() { return m_pAlarms; }
	Alarm *getAlarm(int i);

	inline __s8 numCols() const { return m_sUnit.ncols; };
	void setCols(int i);
	void setSlots(int i);
	void addCol();

	qint8 moduleInitialized() { return m_sUnit.initialized; }
	qint8 daemonInitialized() { return m_bInitialized; }
	long int getNextRelease( long int offset = 0);

	void checkInitialized();
	bool isFull();
	bool isEmpty();
	inline bool isCharging() const { return m_sUnit.charging; }

	const QStringList toStatusStr() const;

public slots:
	void releaseTimeout(Alarm *alarm);
	void timerStarted(Alarm *alarm);
	//void releaseTimeout(Alarm<UnitItem> *alarm);

signals:
	void counterChanged(__u32 counter);
	void doorChanged(__u8 door);
	void lightChanged(__u8 light);
	void chargingChanged(UnitItem *unit);
	void nightChanged(__u8 night);
	void colsChanged(UnitItem *unit);
	void newCol(ColItem *col);
	void initialized(UnitItem *unit);
	void releaseEvent(UnitItem *unit);
	void alarmsChanged(UnitItem *unit);

private slots:
	void nightEnds();
	void nightStarts();

private:
	void initCols();
	void saveAlarms();

	WebSocketClient *m_pDataStream = nullptr;
	struct dispenser_mmap_unit m_sUnit = { 0, 0, 0, 0, 0, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 0, 0, 0, 0 };
	QVector<ColItem> m_cCols;
	bool m_bInitialized = false;
	QMap<int, Alarm *> m_pAlarms; //Release timers!
	//QMap<int, Alarm<UnitItem> *> m_pAlarms; //Release timers!
	QSettings m_cSettings;

	QTimer nightStartTimer;
	QTimer nightEndTimer;
};

#endif // UNITITEM_H
