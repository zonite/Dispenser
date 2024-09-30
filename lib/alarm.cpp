#include "alarm.h"

#include <QTime>
#include<QDaemonLog>

//qRegisterMetaTypeStreamOperators<QList<Alarm>>("Alarm");


//template void Alarm::mapFromIntList(UnitItem *parent, QMap<int, Alarm *> &map, QList<int> &list);
//template void Alarm::mapFromIntList(ColItem *parent, QMap<int, Alarm *> &map, QList<int> &list);

//template class Alarm<UnitItem>;
//template class Alarm<ColItem>;

/*
template<typename T>
Alarm::Alarm(Alarm::T *p, const int *i)
{

}
*/
//template<typename T>
//Alarm<T>::Alarm(const T *parent, qint32 sec, weekdays days)
Alarm::Alarm(const Timer *parent, qint32 sec, weekdays days, int interval)
{
	setParent(parent);

	setDays(days);
	setInterval(interval);
	setSeconds(sec);
}

//template<typename T>
//Alarm<T>::Alarm(const T *parent, const int *i)
Alarm::Alarm(const Timer *parent, const __u64 *i)
{
	setParent(parent);

	setupInt(i);
}

//template<typename T>
//void Alarm<T>::setParent(const QObject *parent)
void Alarm::setParent(const Timer *parent)
{
	if (m_pParent == parent) {
		qDaemonLog(QString("Parent set two times!"), QDaemonLog::NoticeEntry);
		return;
	}

	if (m_pParent) {
		qDaemonLog(QString("Parent already set!"), QDaemonLog::NoticeEntry);
		return;
	}

	connectTimer();

	m_pParent = parent;

	if (m_pParent) {
		//QObject::connect(this, &Alarm::releaseTimeout, m_pParent, &T::releaseTimeout);
		//connect(this, &AlarmSignalsSlotsAlarm<T>::releaseTimeout, m_pParent, &T::releaseTimeout);
		//connect(this, &AlarmSignalsSlots::releaseTimeout, m_pParent, &T::releaseTimeout);
		//connect(this, SIGNAL(AlarmSignalsSlots::releaseTimeout(Alarm<T> *)), m_pParent, SLOT(T::releaseTimeout()));
		const UnitItem *unit = dynamic_cast<const UnitItem *>(m_pParent);
		const ColItem *col = dynamic_cast<const ColItem *>(m_pParent);

		if (unit) {
			connect(this, &Alarm::releaseTimeout, unit, &UnitItem::releaseTimeout, Qt::QueuedConnection);
			connect(this, &Alarm::timerStarted, unit, &UnitItem::timerStarted, Qt::QueuedConnection);
			qDaemonLog(QString("Alarms(%1) connected to unit.").arg(QString::number((long long int)this)), QDaemonLog::NoticeEntry);
		}

		if (col) {
			connect(this, &Alarm::releaseTimeout, col, &ColItem::releaseTimeout, Qt::QueuedConnection);
			connect(this, &Alarm::timerStarted, col, &ColItem::timerStarted, Qt::QueuedConnection);
			qDaemonLog(QString("Alarms connected to col.").arg(QString::number((long long int)this)), QDaemonLog::NoticeEntry);
		}

		//connectTimer();
	}
}


/*
Alarm::Alarm(UnitItem *unit, const int *i)
{
	m_pUnit = unit;

	connect(this, &Alarm::releaseTimeout, m_pUnit, &UnitItem::releaseTimeout);
	connectTimer();

	setupInt(i);
}

Alarm::Alarm(ColItem *col, const int *i)
{
	m_pCol = col;

	connect(this, &Alarm::releaseTimeout, m_pCol, &ColItem::releaseTimeout);
	connectTimer();

	setupInt(i);
}
*/
/*
Alarm::Alarm(qint32 sec, weekdays days)
{
	m_iActive = days;
	setSeconds(sec);
}

Alarm::Alarm()
{
}

Alarm::Alarm(const QVariant &var)
{
	unsigned int i = var.toUInt();
	m_iActive = (enum weekdays) ((char) i & 0xFF);
	setSeconds(i >> 8);
}
*/

//template <typename T>
Alarm::~Alarm()
{
	/*
	if (m_pTimer) {
		delete m_pTimer;
		m_pTimer = nullptr;
	}
	*/
}

//template<typename T>
void Alarm::mapFromIntList(Timer *parent, QMap<int, Alarm *> &map, const QList<__u64> &list)
{
	for (const __u64 &item : list) {
		Alarm *alarm = new Alarm(parent, &item);
		map.insert(alarm->getSeconds(), alarm);
	}
}

void Alarm::mapFromVariantList(Timer *parent, QMap<int, Alarm *> &map, const QList<QVariant> &list)
{
	for (const QVariant &var : list) {
		__u64 i = var.toULongLong();
		Alarm *alarm = new Alarm(parent, &i);
		map.insert(alarm->getSeconds(), alarm);
	}

}

//template<typename T>
QList<__u64> Alarm::toIntList(const QMap<int, Alarm *> alarms)
{
	QList<__u64> ints;

	for (const Alarm *alarm : alarms) {
		__u64 i = alarm->toInt();
		ints.append(i);
	}

	return ints;
}

QList<QVariant> Alarm::toVariantList(const QMap<int, Alarm *> alarms)
{
	QList<QVariant> ints;
	for (const Alarm *alarm : alarms) {
		__u64 i = alarm->toInt();
		ints.append(i);
	}

	return ints;

}

/*
Alarm::Alarm(const Alarm &src)
{
	m_iActive = src.m_iActive;
	setSeconds(src.m_iSeconds);
}

Alarm &Alarm::operator=(const Alarm &src)
{
	m_iActive = src.m_iActive;
	setSeconds(src.m_iSeconds);

	return *this;
}

bool Alarm::operator()(const Alarm &a, const Alarm &b) const
{
	return a.getSeconds() < b.getSeconds();
}

bool Alarm::operator<(const Alarm &b) const
{
	return m_iSeconds < b.m_iSeconds;
}
*/

/*
QList<Alarm> &Alarm::fromVariant(const QVariant &var)
{
	QList<QVariant> list = var.toList();

	QList<Alarm> alarms = list;
}
*/

/*
void Alarm::clean(QList<Alarm> &list)
{
//	std::sort(list.begin(), list.end(), cmp);
	std::sort(list.begin(), list.end());

	QList<Alarm>::iterator i = list.begin();
	QList<Alarm>::iterator next;
	//QListIterator<Alarm> k(list);

	for (next = i + 1; next != list.end();) {
		if (i->getSeconds() == next->getSeconds()) {
			i->setDays(next->getDays());
			next = list.erase(next);
		} else {
			next = ++i + 1;
		}

	}
	//Done.
}
*/

/*
QList<Alarm> Alarm::fromVariant(QVariant var)
{
	QList<Alarm> alarms;

	if (var.canConvert<QVariantList>()) {
		QSequentialIterable iterable = var.value<QSequentialIterable>();
		foreach (const QVariant &v, iterable) {
			alarms.append(Alarm::fromInt(v.toInt()));
		}
	}

	return alarms;
}
*/

/*
Alarm Alarm::fromInt(int i)
{
	Alarm a;

	a.m_iActive = (enum weekdays) ((char) i & 0xFF);
	a.setSeconds(i >> 8);

	return a;
}
*/

/*
QVariant Alarm::toVariant(QList<Alarm> alarms)
{
	return QVariant::fromValue(toIntList(alarms));
}

QList<int> Alarm::toIntList(QList<Alarm> alarms)
{
	QList<int> ints;

	foreach(const Alarm &a, alarms) {
		ints.append(a.toInt());
	}

	return ints;
}
*/

/*
QTimer *Alarm::setParent(UnitItem *unit)
{
	if (m_pUnit || m_pCol || !unit)
		return nullptr;

	m_pUnit = unit;

	//QObject::connect(&m_cTimer, &QTimer::timeout, m_pUnit, &UnitItem::releaseTimeout);
	startTimer();

	return &m_cTimer;
}

QTimer *Alarm::setParent(ColItem *col)
{
	if (m_pUnit || m_pCol || !col)
		return nullptr;

	m_pCol = col;

	startTimer();

	return &m_cTimer;
}
*/

//template<typename T>
void Alarm::setAlarm(QTime time)
{
	setSeconds(time.msecsSinceStartOfDay() / 1000);
}

//template<typename T>
void Alarm::setSeconds(qint32 seconds)
{
	m_iSeconds = (((seconds % 86400) + 86400) % 86400) * 1000;

	startTimer();
	qDaemonLog(QString("Alarm started. Currect time is %1. Alarm is at %2:%3:%4. Interval %5. To go %6min.")
	           .arg(QTime::currentTime().toString("hh:mm"))
	           .arg(m_iSeconds / 3600000).arg((m_iSeconds / 60000) % 60).arg((m_iSeconds/1000) % 60)
	           .arg(QTime::fromMSecsSinceStartOfDay(m_iInterval).toString())
	           .arg(QTime::fromMSecsSinceStartOfDay(m_cTimer.remainingTime()).toString())
	           , QDaemonLog::NoticeEntry);
}

void Alarm::setInterval(quint32 interval)
{
	m_iInterval = ((interval > 86400 * 2) ? 86400 : interval) * 1000;
	m_cTimer.setInterval(m_iInterval);
}

//template<typename T>
void Alarm::setDays(weekdays days)
{
	m_iActive = days;
}

//template<typename T>
void Alarm::orDays(weekdays days)
{
	m_iActive = (enum weekdays) ((char) m_iActive | (char) days);
}

//template<typename T>
void Alarm::setupInt(const __u64 *i)
{
	if (i) {
		setDays((enum weekdays) ((char) (*i) & 0xFF));
		setInterval((*i) >> 32 & 0xFFFFFFFF);
		setSeconds((*i) >> 8);
	}
}

//template<typename T>
bool Alarm::checkDay()
{
	int weekday = QDate::currentDate().dayOfWeek();

	return (weekday & m_iActive);
}

//Disable alarms permanently
void Alarm::disconnectTimer()
{
	disconnect(&m_cTimer);
	m_bInhibit = true;
	m_cTimer.stop();
}

//template<typename T>
QString Alarm::getTimevalue() const
{
	return QString("%1:%2").arg(getSeconds() / 3600 ).arg(getSeconds() / 60 % 60);
}

int Alarm::remainingTime() const
{
	return m_cTimer.remainingTime();
}

int Alarm::getRemaining()
{
	return m_cTimer.remainingTime();
}

bool Alarm::isActive()
{
	return m_cTimer.isActive();
}


//template<typename T>
void Alarm::startTimer()
{
	int msecToRelease;
	if (m_bInhibit)
		return;

	msecToRelease = (((m_iSeconds - QTime::currentTime().msecsSinceStartOfDay()) % m_iInterval) + m_iInterval) % m_iInterval;
	//if (msecToRelease < 0)
	//	msecToRelease += 86400000;

	//m_cTimer.start(msecToRelease);
	//m_cTimer.setInterval(m_iInterval * 1000);
	m_cTimer.start(msecToRelease);

	qDaemonLog(QString("Alarm Start %1. To timeout %2, interval %3 @ %4, Alarm stores tmout %5, interv %6.")
	           .arg(QTime::currentTime().toString("hh:mm:ss"))
	           .arg(QTime::fromMSecsSinceStartOfDay(m_cTimer.remainingTime()).toString())
	           .arg(QTime::fromMSecsSinceStartOfDay(m_cTimer.interval()).toString())
	           .arg(QTime::currentTime().addMSecs(m_cTimer.remainingTime()).toString())
	           .arg(QTime::fromMSecsSinceStartOfDay(m_iSeconds).toString())
	           .arg(QTime::fromMSecsSinceStartOfDay(m_iInterval).toString()), QDaemonLog::NoticeEntry);

	emit timerStarted(this);
}

//template<typename T>
void Alarm::connectTimer()
{
	connect(&m_cTimer, &QTimer::timeout, this, &Alarm::timeout);

	m_cTimer.setTimerType(Qt::VeryCoarseTimer);
	//m_cTimer.setInterval(m_iInterval);
	//m_cTimer.setInterval(24 * 3600 * 1000);

	//QObject::connect(&m_cTimer, &QTimer::timeout, this, &Alarm::timeout);
	//connect(&m_cTimer, &QTimer::timeout, this, &Alarm<T>::timeout);
	//connect(&m_cTimer, &QTimer::timeout, this, &AlarmSignalsSlots::timeout);
}

//template<typename T>
QDebug operator<<(QDebug dbg, const Alarm &alarm)
{
	dbg.nospace() << "Alarm " << alarm.getTimevalue() << ". Days " << alarm.getDays() << ".";

	return dbg;
}

//template<typename T>
QDataStream &operator<<(QDataStream &out, const Alarm &alarm)
{
	out << QVariant(alarm.getSeconds()) << QVariant((char)alarm.getDays());

	return out;
}

//template<typename T>
/*
QDataStream &operator>>(QDataStream &in, Alarm &alarm)
{
	QVariant a, b;
	in >> a >> b;

	alarm.setDays((enum weekdays) b.toChar().toLatin1());
	alarm.setSeconds(b.toInt());

	return in;
}
*/
//template<typename T>
/*
bool cmp(const Alarm &a, const Alarm &b)
{
	return a.getSeconds() < b.getSeconds();
}
*/
void Alarm::timeout() {
	m_cTimer.start((((m_iSeconds - QTime::currentTime().msecsSinceStartOfDay()) % m_iInterval) + m_iInterval) % m_iInterval);
	qDaemonLog(QString("Alarm timed out at %1.").arg(QTime::currentTime().toString("hh:mm:ss")), QDaemonLog::NoticeEntry);
	if (checkDay())
		emit releaseTimeout(this);
}

/*
void AlarmSignalsSlots::timeout()
{
	Alarm<UnitItem> *unit = dynamic_cast<Alarm<UnitItem> *>(this);
	Alarm<ColItem> *col = dynamic_cast<Alarm<ColItem> *>(this);

	if (unit && unit->checkDay()) {
		emit releaseTimeout(unit);
	}

	if (col && col->checkDay()) {
		emit releaseTimeout(col);
	}
}
*/
