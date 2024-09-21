#ifndef ALARM_H
#define ALARM_H

#include <QObject>
#include <QTimer>
#include <QDebug>
#include <QList>

#include <dispenser.h>
#include "lib_global.h"

#include "timer.h"
#include "unititem.h"
#include "colitem.h"


//class AlarmList : public QList<Alarm> {
//
//};
//template <typename T> class Alarm;
//class Alarm;
//class UnitItem;
//class ColItem;

//bool cmp(const Alarm &a, const Alarm &b);

/*
class AlarmSignalsSlots : public QObject
{
	Q_OBJECT
public:
	explicit AlarmSignalsSlots(QObject *parent = 0)
		: QObject(parent) {}
	virtual ~AlarmSignalsSlots() {};

public slots:
	void timeout();

signals:
	void releaseTimeout(Alarm<UnitItem> *alarm);
	void releaseTimeout(Alarm<ColItem> *alarm);
};
*/


//template <typename T>
//class Alarm : public AlarmSignalsSlots
class LIB_EXPORT Alarm : public Timer
{
	Q_OBJECT
public:
	//explicit Alarm(qint32 sec, enum weekdays days = EVERYDAY);
	//Alarm(UnitItem *unit, const int *i = nullptr);
	//Alarm(ColItem *col, const int *i = nullptr);
	Alarm(const Timer *parent, qint32 sec, enum weekdays days = EVERYDAY);
	Alarm(const Timer *parent, const int *i = nullptr);
	//Alarm(const T *parent, qint32 sec, enum weekdays days = EVERYDAY);
	//Alarm(const T *parent, const int *i = nullptr);
	//template <typename T> Alarm(T *p, const int *i = nullptr);
	virtual ~Alarm();
	//Alarm(const Alarm &src);
	//Alarm(const QVariant &var);
	//Alarm &operator=(const Alarm &src);

	//bool operator()(const Alarm &a, const Alarm &b) const;
	//bool operator<(const Alarm &b) const;

	//inline operator QVariant() const { return QVariant::fromValue(*this); } //convert to QVariant
	inline operator int() const { return ((m_iSeconds << 8) | m_iActive); } //convert to int

	//static QList<Alarm> &fromVariant(const QVariant &var);
	//static void clean(QList<Alarm> &list);
	//template<typename T> static void mapFromIntList(T *parent, QMap<int, Alarm*> &map, QList<int> &list);
	static void mapFromIntList(Timer *parent, QMap<int, Alarm*> &map, const QList<int> &list);
	static void mapFromVariantList(Timer *parent, QMap<int, Alarm*> &map, const QList<QVariant> &list);
	//static void mapFromIntList(T *parent, QMap<int, Alarm*> &map, const QList<int> &list);
	//static QList<Alarm> fromVariant(QVariant var);
	//static Alarm fromInt(int i);
	//static QVariant toVariant(QList<Alarm> alarms);
	//static QList<int> toIntList(QList<Alarm> alarms);
	static QList<int> toIntList(const QMap<int, Alarm*> alarms);
	static QList<QVariant> toVariantList(const QMap<int, Alarm*> alarms);
	int toInt() const { return ((m_iSeconds << 8) | m_iActive); }

	//QTimer *setParent(UnitItem *unit);
	//QTimer *setParent(ColItem *col);
	void setAlarm(QTime time);
	void setSeconds(qint32 seconds);
	void setDays(enum weekdays days);
	void orDays(enum weekdays days);
	void setupInt(const int *i);
	bool checkDay();

	const Timer *getParent() { return m_pParent; }
	//const T *getParent() { return m_pParent; }
	//UnitItem *getUnit() { return m_pUnit; }
	//ColItem *getCol() { return m_pCol; }
	qint32 getSeconds() const { return m_iSeconds; }
	QString getTimevalue() const;
	enum weekdays getDays() const { return m_iActive; }
	int getRemaining();
	bool isActive();

public slots:
	void timeout();

signals:
	void releaseTimeout(Alarm *alarm);

private:
	//void setParent(const T *parent);
	void setParent(const Timer *parent);

	void updateCol();
	void startTimer();
	void connectTimer();

	qint32 m_iSeconds = -1;
	enum weekdays m_iActive = NONE;
	//ColItem *m_pCol = nullptr;
	//UnitItem *m_pUnit = nullptr;
	//const T *m_pParent = nullptr;
	const Timer *m_pParent = nullptr;
	QTimer m_cTimer;
	//char colId = -1;
};

//inline operator QVariant(QList<Alarm> list) { return QVariant::fromValue(list); }

QDebug operator<<(QDebug dbg, const Alarm &alarm);
QDataStream &operator<<(QDataStream &out, const Alarm &alarm);
QDataStream &operator>>(QDataStream &in, Alarm &alarm);

/*
template<typename T> QDebug operator<<(QDebug dbg, const Alarm<T> &alarm);
template<typename T> QDataStream &operator<<(QDataStream &out, const Alarm<T> &alarm);
template<typename T> QDataStream &operator>>(QDataStream &in, Alarm<T> &alarm);
*/

//qRegisterMetaTypeStreamOperators<QList<Alarm>>("Alarm");

//Q_DECLARE_METATYPE(Alarm<UnitItem>);
//Q_DECLARE_METATYPE(Alarm<ColItem>);

#endif // ALARM_H
