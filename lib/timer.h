#ifndef TIMER_H
#define TIMER_H

#include <QObject>

#include "lib_global.h"

class Timer : public QObject
{
	Q_OBJECT
public:
	explicit Timer(QObject *parent = nullptr);
	virtual ~Timer();

public slots:
	//void timeout();

signals:
	//void releaseTimeout(Timer *alarm);
	void modifyAlarm(Alarm *alarm);
};

#endif // TIMER_H
