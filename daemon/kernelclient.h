#ifndef KERNELCLIENT_H
#define KERNELCLIENT_H

#include <QObject>

#include <linux/genetlink.h>

class KernelClient : public QObject
{
	Q_OBJECT
public:
	explicit KernelClient(QObject *parent = nullptr);

signals:

private:
	int open_and_bind_socket();
	int resolve_family_id_by_name();
	int failed;
};

#endif // KERNELCLIENT_H
