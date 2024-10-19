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

class WebSocketClient;

class LIB_EXPORT UnitItem : public Timer
{
	Q_OBJECT
	//Q_PROPERTY(int number READ number WRITE setNumber NOTIFY numberChanged)
public:
	explicit UnitItem(QObject *parent = nullptr);
	~UnitItem();

	SlotItem *viewSlot(int col, int row);
	SlotItem *slot(int col, int slot);
	ColItem *col(int col);


	//int number() const { return mNumber; }
	//void setNumber(int i) { mNumber = i; emit numberChanged(); }

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
	SlotItem *getNextSlot(); //Returns nex slot to be released
	void assingReleases();
	int getFullCount() const { return m_iFullCount; }
	int countFull();

	inline __s8 numCols() const { return m_sUnit.ncols; };
	inline __s8 getRows() const { return m_iRows; };
	__s8 countRows();
	void setCols(int i);
	void setSlots(int i);
	void setAlarm(__u64 alarm);
	void addCol();

	qint8 moduleInitialized() { return m_sUnit.initialized; }
	qint8 daemonInitialized() { return m_bInitialized; }
	long int getNextRelease( long int offset = 0);  //Real timer remaining time
	QDateTime getNextRelease( QDateTime offset) const;  //Real timer remaining time

	void checkInitialized();
	bool isFull();
	bool isEmpty();
	inline bool isCharging() const { return m_sUnit.charging; }

	const QStringList toStatusStr() const;

public slots:
	void releaseTimeout(Alarm *alarm);
	void timerStarted(Alarm *alarm);
	//void releaseTimeout(Alarm<UnitItem> *alarm);

	void preColSlotAppended() { emit preSlotAppended(); }
	void postColSlotAppended() { countRows(); emit postSlotAppended(); }

	void preColSlotRemoved(int index1, int index2) { emit preSlotRemoved(index1, index2); }
	void postColSlotRemoved() { m_iRows = 0; emit postSlotRemoved(); }


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

	void preColAppended();
	void postColAppended();

	void preColRemoved(int index1, int index2);
	void postColRemoved();

	void preSlotAppended();
	void postSlotAppended();

	void preSlotRemoved(int index, int index2);
	void postSlotRemoved();

	//void numberChanged();


private slots:
	void nightEnds();
	void nightStarts();

	//Websocket interface:
	void serverReady();

private:
	void initCols();
	void saveAlarms();

	//int mNumber = 12;

	WebSocketClient *m_pDataStream = nullptr;
	struct dispenser_mmap_unit m_sUnit = { 0, 0, 0, 0, 0, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 0, 0, 0, 0 };
	QVector<ColItem> m_cCols;
	__s8 m_iRows = 0;
	int m_iFullCount = 0;
	bool m_bInitialized = false;
	QMap<int, Alarm *> m_pAlarms; //Release timers!
	//QMap<int, Alarm<UnitItem> *> m_pAlarms; //Release timers!
	QSettings m_cSettings;
	int m_iAlarmMinimumScheduling = ALARM_MIN_SCHEDULING;

	QTimer nightStartTimer;
	QTimer nightEndTimer;
};

#endif // UNITITEM_H
