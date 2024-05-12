#ifndef KERNELCLIENT_H
#define KERNELCLIENT_H

//#include <QTcpServer>
#include <QObject>
#include <QSocketNotifier>

#include <linux/genetlink.h>

QT_FORWARD_DECLARE_CLASS(WebSocketServer)

class KernelClient : public QObject
{
	Q_OBJECT
public:
	explicit KernelClient(QObject *parent = nullptr);
	~KernelClient();

signals:
	void stopped();

public slots:
	void start(const QStringList &arguments);
	void stop();

private slots:
	//void threadStarted();
	//void threadFinished();
	void readyRead();

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

	QSocketNotifier *m_pKernel = nullptr;

	/** Websocket server */
	WebSocketServer *m_pServer = nullptr;
};

#endif // KERNELCLIENT_H
