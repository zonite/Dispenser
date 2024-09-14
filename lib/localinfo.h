#ifndef LOCALINFO_H
#define LOCALINFO_H

#include <QTime>

class LocalInfo
{
public:
	LocalInfo();

	static QTime getSunrise();
	static QTime getSunset();
};

#endif // LOCALINFO_H
