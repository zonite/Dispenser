#ifndef ALARM_H
#define ALARM_H

#include <QObject>
#include <QTimer>

#include <dispenser.h>

#include "unititem.h"
#include "colitem.h"

class Alarm;

class AlarmList : public QList<Alarm> {

};

class Alarm : public QObject
{
	Q_OBJECT
public:
	explicit Alarm(qint32 sec, enum weekdays days = EVERYDAY, QObject *parent = nullptr);

	ColItem *getCol() { return col; }
	qint32 getSeconds() { return m_iSeconds; }
	enum weekdays getDays() { return m_iActive; }

signals:

private:
	void updateCol();

	qint32 m_iSeconds = -1;
	enum weekdays m_iActive = NONE;
	ColItem *col = nullptr;
	UnitItem *unit = nullptr;
	//char colId = -1;
};

#endif // ALARM_H
