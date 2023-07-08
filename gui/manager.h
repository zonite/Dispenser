#ifndef MANAGER_H
#define MANAGER_H

#include <QObject>
#include <QString>
//#include <QQuickItem>
//#include <QtQml/qqmlparserstatus.h>
#include <QtQml/qqml.h>

class Manager : public QObject
{
	Q_OBJECT
	//Q_INTERFACES(QQmlParserStatus)
	Q_PROPERTY(QString userName READ userName WRITE setUserName NOTIFY userNameChanged)
	QML_ELEMENT
public:
	explicit Manager(QObject *parent = nullptr);

	QString userName();
	void setUserName(const QString &userName);

signals:
	void userNameChanged();

private:
	QString m_userName;

};

#endif // MANAGER_H
