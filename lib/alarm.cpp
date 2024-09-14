#include "alarm.h"


Alarm::Alarm(qint32 sec, weekdays days, QObject *parent)
{
	m_iSeconds = sec;
	m_iActive = days;
}
