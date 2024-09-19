#include "alarm.h"

#include <QTime>

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
Alarm::Alarm(const Timer *parent, qint32 sec, weekdays days)
{
	setParent(parent);

	setDays(days);
	setSeconds(sec);
}

//template<typename T>
//Alarm<T>::Alarm(const T *parent, const int *i)
Alarm::Alarm(const Timer *parent, const int *i)
{
	setParent(parent);

	setupInt(i);
}

//template<typename T>
//void Alarm<T>::setParent(const QObject *parent)
void Alarm::setParent(const Timer *parent)
{
	m_pParent = parent;

	if (m_pParent) {
		//QObject::connect(this, &Alarm::releaseTimeout, m_pParent, &T::releaseTimeout);
		//connect(this, &AlarmSignalsSlotsAlarm<T>::releaseTimeout, m_pParent, &T::releaseTimeout);
		//connect(this, &AlarmSignalsSlots::releaseTimeout, m_pParent, &T::releaseTimeout);
		//connect(this, SIGNAL(AlarmSignalsSlots::releaseTimeout(Alarm<T> *)), m_pParent, SLOT(T::releaseTimeout()));
		const UnitItem *unit = dynamic_cast<const UnitItem *>(m_pParent);
		const ColItem *col = dynamic_cast<const ColItem *>(m_pParent);

		if (unit) {
			connect(this, &Alarm::releaseTimeout, unit, &UnitItem::releaseTimeout);
		}

		if (col) {
			connect(this, &Alarm::releaseTimeout, col, &ColItem::releaseTimeout);
		}

		connectTimer();
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
void Alarm::mapFromIntList(Timer *parent, QMap<int, Alarm *> &map, const QList<int> &list)
{
	for (const int &item : list) {
		Alarm *alarm = new Alarm(parent, &item);
		map.insert(alarm->getSeconds(), alarm);
	}
}

void Alarm::mapFromVariantList(Timer *parent, QMap<int, Alarm *> &map, const QList<QVariant> &list)
{
	for (const QVariant &var : list) {
		int i = var.toInt();
		Alarm *alarm = new Alarm(parent, &i);
		map.insert(alarm->getSeconds(), alarm);
	}

}

//template<typename T>
QList<int> Alarm::toIntList(const QMap<int, Alarm *> alarms)
{
	QList<int> ints;

	for (const Alarm *alarm : alarms) {
		int i = alarm->toInt();
		ints.append(i);
	}

	return ints;
}

QList<QVariant> Alarm::toVariantList(const QMap<int, Alarm *> alarms)
{
	QList<QVariant> ints;
	for (const Alarm *alarm : alarms) {
		int i = alarm->toInt();
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
	m_iSeconds = seconds % 86400;

	startTimer();
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
void Alarm::setupInt(const int *i)
{
	if (i) {
		setDays((enum weekdays) ((char) (*i) & 0xFF));
		setSeconds((*i) >> 8);
	}
}

//template<typename T>
bool Alarm::checkDay()
{
	int weekday = QDate::currentDate().dayOfWeek();

	return (weekday & m_iActive);
}

//template<typename T>
QString Alarm::getTimevalue() const
{
	return QString("%1:%2").arg(getSeconds() / 3600 ).arg(getSeconds() / 60 % 60);
}

//template<typename T>
void Alarm::startTimer()
{
	int msecToRelease;

	msecToRelease = m_iSeconds * 1000 - QTime::currentTime().msecsSinceStartOfDay();
	if (msecToRelease < 0)
		msecToRelease += 86400000;

	m_cTimer.start(msecToRelease);
}

//template<typename T>
void Alarm::connectTimer()
{
	m_cTimer.setTimerType(Qt::VeryCoarseTimer);
	m_cTimer.setInterval(24 * 3600 * 1000);

	//QObject::connect(&m_cTimer, &QTimer::timeout, this, &Alarm::timeout);
	//connect(&m_cTimer, &QTimer::timeout, this, &Alarm<T>::timeout);
	//connect(&m_cTimer, &QTimer::timeout, this, &AlarmSignalsSlots::timeout);
	connect(&m_cTimer, &QTimer::timeout, this, &Alarm::timeout);
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
QDataStream &operator>>(QDataStream &in, Alarm &alarm)
{
	QVariant a, b;
	in >> a >> b;

	alarm.setDays((enum weekdays) b.toChar().toLatin1());
	alarm.setSeconds(b.toInt());

	return in;
}

//template<typename T>
bool cmp(const Alarm &a, const Alarm &b)
{
	return a.getSeconds() < b.getSeconds();
}

void Alarm::timeout() {
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
