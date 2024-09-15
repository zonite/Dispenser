#include "alarm.h"

#include <QTime>

//qRegisterMetaTypeStreamOperators<QList<Alarm>>("Alarm");

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

Alarm::~Alarm()
{
	/*
	if (m_pTimer) {
		delete m_pTimer;
		m_pTimer = nullptr;
	}
	*/
}

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

/*
QList<Alarm> &Alarm::fromVariant(const QVariant &var)
{
	QList<QVariant> list = var.toList();

	QList<Alarm> alarms = list;
}
*/

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

Alarm Alarm::fromInt(int i)
{
	Alarm a;

	a.m_iActive = (enum weekdays) ((char) i & 0xFF);
	a.setSeconds(i >> 8);

	return a;
}

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

void Alarm::setAlarm(QTime time)
{
	setSeconds(time.msecsSinceStartOfDay() / 1000);
}

void Alarm::setSeconds(qint32 seconds)
{
	m_iSeconds = seconds % 86400;
}

void Alarm::setDays(weekdays days)
{
	m_iActive = (enum weekdays) ((char) m_iActive | (char) days);
}

QString Alarm::getTimevalue() const
{
	return QString("%i:%i").arg(getSeconds() / 3600 ).arg(getSeconds() / 60 % 60);
}

void Alarm::startTimer()
{
	int msecToRelease;

	m_cTimer.setTimerType(Qt::VeryCoarseTimer);
	m_cTimer.setInterval(24 * 3600 * 1000);

	msecToRelease = m_iSeconds * 1000 - QTime::currentTime().msecsSinceStartOfDay();
	if (msecToRelease < 0)
		msecToRelease += 86400000;

	m_cTimer.start(msecToRelease);
}

QDebug operator<<(QDebug dbg, const Alarm &alarm)
{
	dbg.nospace() << "Alarm " << alarm.getTimevalue() << ". Days " << alarm.getDays() << ".";

	return dbg;
}

QDataStream &operator<<(QDataStream &out, const Alarm &alarm)
{
	out << QVariant(alarm.getSeconds()) << QVariant((char)alarm.getDays());

	return out;
}

QDataStream &operator>>(QDataStream &in, Alarm &alarm)
{
	QVariant a, b;
	in >> a >> b;

	alarm.setDays((enum weekdays) b.toChar().toLatin1());
	alarm.setSeconds(b.toInt());

	return in;
}

bool cmp(const Alarm &a, const Alarm &b)
{
	return a.getSeconds() < b.getSeconds();
}
