#ifndef KERNELCLIENT_H
#define KERNELCLIENT_H

#include <QTcpServer>

#include <linux/genetlink.h>

class KernelClient : public QObject
{
	Q_OBJECT
public:
	explicit KernelClient(QObject *parent = nullptr);
	~KernelClient();

signals:
	void stopped();

public slots:
	void start(const QStringList &);
	void stop();

private slots:
	void threadStarted();
	void threadFinished();

protected:
//	void incomingConnection(qintptr) Q_DECL_OVERRIDE;

private:

	int open_and_bind_socket();
	int resolve_family_id_by_name();
	int failed;

	/** The family ID resolved by Generic Netlink control interface. Assigned when the kernel module registers the Family */
	int nl_family_id = -1;
	/** Netlink socket's file descriptor. */
	int nl_fd = -1;
	/** Netlink socket address */
	struct sockaddr_nl nl_address;

};

#endif // KERNELCLIENT_H
