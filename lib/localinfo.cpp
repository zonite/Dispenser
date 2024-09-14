#include "localinfo.h"

LocalInfo::LocalInfo()
{

}

QTime LocalInfo::getSunrise()
{
	return QTime(8, 0);
}

QTime LocalInfo::getSunset()
{
	return QTime(20, 0);
}
