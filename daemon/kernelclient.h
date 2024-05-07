#ifndef KERNELCLIENT_H
#define KERNELCLIENT_H

#include <QObject>

class KernelClient : public QObject
{
	Q_OBJECT
public:
	explicit KernelClient(QObject *parent = nullptr);

signals:

};

#endif // KERNELCLIENT_H
