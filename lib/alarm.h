#ifndef ALARM_H
#define ALARM_H

#include <QObject>
#include <QTimer>
#include <QDebug>
#include <QList>

#include <dispenser.h>

#include "unititem.h"
#include "colitem.h"

//class Alarm;

//class AlarmList : public QList<Alarm> {
//
//};
class Alarm;
class UnitItem;
class ColItem;

bool cmp(const Alarm &a, const Alarm &b);

class Alarm // : public QObject
{
//	Q_OBJECT
public:
	explicit Alarm(qint32 sec, enum weekdays days = EVERYDAY);
	Alarm();
	~Alarm();
	Alarm(const Alarm &src);
	Alarm(const QVariant &var);
	Alarm &operator=(const Alarm &src);

	bool operator()(const Alarm &a, const Alarm &b) const;
	bool operator<(const Alarm &b) const;

	//inline operator QVariant() const { return QVariant::fromValue(*this); } //convert to QVariant
	//inline operator int() const { return ((m_iSeconds << 8) | m_iActive); } //convert to int

	//static QList<Alarm> &fromVariant(const QVariant &var);
	static void clean(QList<Alarm> &list);

	static QList<Alarm> fromVariant(QVariant var);
	static Alarm fromInt(int i);
	static QVariant toVariant(QList<Alarm> alarms);
	static QList<int> toIntList(QList<Alarm> alarms);
	int toInt() const { return ((m_iSeconds << 8) | m_iActive); }

	QTimer *setParent(UnitItem *unit);
	QTimer *setParent(ColItem *col);
	void setAlarm(QTime time);
	void setSeconds(qint32 seconds);
	void setDays(enum weekdays days);

	UnitItem *getUnit() { return m_pUnit; }
	ColItem *getCol() { return m_pCol; }
	qint32 getSeconds() const { return m_iSeconds; }
	QString getTimevalue() const;
	enum weekdays getDays() const { return m_iActive; }

//signals:

private:
	void updateCol();
	void startTimer();

	qint32 m_iSeconds = -1;
	enum weekdays m_iActive = NONE;
	ColItem *m_pCol = nullptr;
	UnitItem *m_pUnit = nullptr;
	QTimer m_cTimer;
	//char colId = -1;
};

//inline operator QVariant(QList<Alarm> list) { return QVariant::fromValue(list); }

QDebug operator<<(QDebug dbg, const Alarm &alarm);
QDataStream &operator<<(QDataStream &out, const Alarm &alarm);
QDataStream &operator>>(QDataStream &in, Alarm &alarm);

//qRegisterMetaTypeStreamOperators<QList<Alarm>>("Alarm");

Q_DECLARE_METATYPE(Alarm);

#endif // ALARM_H
