#include "manager.h"

Manager::Manager(QObject *parent)
        : QObject{parent}
{

}

QString Manager::userName()
{
	return m_userName;
}

void Manager::setUserName(const QString &userName)
{
	if (userName == m_userName)
		return;

	m_userName = userName;
	emit userNameChanged();
}
