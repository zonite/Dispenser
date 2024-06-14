#ifndef KERNELCLIENT_H
#define KERNELCLIENT_H

//#include <QTcpServer>
#include <QObject>
#include <QSocketNotifier>
#include <QDataStream>
#include <QBuffer>

#include <linux/netlink.h>
#include <linux/genetlink.h>

QT_FORWARD_DECLARE_CLASS(WebSocketServer)
QT_FORWARD_DECLARE_CLASS(KernelStream)

struct generic_netlink_msg {
    /** Netlink header comes first. */
    struct nlmsghdr n;
    /** Afterwards the Generic Netlink header */
    struct genlmsghdr g;
    /** Custom data. Space for Netlink Attributes. */
    char buf[256];
};

#define NL_ATTR_HDRLEN	NL_ALIGN(sizeof(struct nlattr))
#define NL_NLMSG_HDRLEN	NL_ALIGN(sizeof(struct nlmsghdr))

template <typename T> class KernelStreamIterator {
public:
	//explicit KernelStreamIterator(KernelStream *stream);
	explicit KernelStreamIterator(KernelStream *stream, qsizetype pos = -1);
	~KernelStreamIterator();

	T *data();

private:
	KernelStream *p_mStream = nullptr;
	qsizetype mPos = 0;
};

class KernelStream : public QDataStream
{
public:
	KernelStream &operator<<(nlmsghdr &s);
	KernelStream &operator<<(genlmsghdr &s);
	KernelStream &operator<<(nlattr &s);
	KernelStream &operator<<(QByteArray &s);

	KernelStream &align();
	KernelStream &alignAttr();

	inline int size() { return *pSize(); }
	int *pSize();

	//inline qint64 pos() { return *pPos(); }
	//qint64 *pPos();
	qint64 pos();

private:

};


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

	//Genetlink interface:
	static struct nlmsghdr* nl_hdr_put(KernelStream *out);
	static struct genlmsghdr* genl_hdr_put(KernelStream *out, quint8 cmd, quint8 version = 1);
	//static struct nl_attr* nl_attr_put(KernelStream *out, quint16 type, const char *str);
	static struct nlattr *nl_attr_put(KernelStream *out, quint16 type, const QByteArray *str);
	//static struct nl_attr* nl_attr_put(struct nlmsghdr *nlh, uint16_t type,size_t len, const void *data);
	char *string_strldup(const char *src, size_t size);
	void *nl_attr_get_payload(const struct nlattr *attr);
	const char *nl_attr_get_str(const struct nlattr *attr);
	uint16_t nl_attr_get_u16(const struct nlattr *attr);
	uint16_t nl_attr_get_type(const struct nlattr *attr);
	int nl_attr_type_valid(const struct nlattr *attr, uint16_t max);
	int data_attr_cb(const struct nlattr *attr, void *data);
	int parse_gen_message(const struct nlmsghdr *nlh, void *data);
	//int netlink_parse_nlmsg(netlink_sock_t *nls, char *buf, ssize_t buflen);
	//static ssize_t netlink_recvmsg(netlink_sock_t *nls, struct msghdr *msg, char *buf, size_t bufsz, char **outbuf, size_t *outbufsz);
	//void read_socket(int fd,  nls_opaque, int status);
	ssize_t sendToKernel(KernelStream *out);

	int open_and_bind_socket();
	int resolve_family_id_by_name();
	int get_unit_status();
	int failed;

	/** The family ID resolved by Generic Netlink control interface. Assigned when the kernel module registers the Family */
	int nl_family_id = -1;
	/** Netlink socket's file descriptor. */
	int nl_fd = -1;
	/** Netlink socket address */
	struct sockaddr_nl nl_address;
	struct generic_netlink_msg nl_request_msg;
	struct generic_netlink_msg nl_response_msg;

	QSocketNotifier *m_pKernel = nullptr;

	/** Websocket server */
	WebSocketServer *m_pServer = nullptr;

	QBuffer mOutBuffer;
	QBuffer mInBuffer;

	KernelStream mToKernel;
};


#endif // KERNELCLIENT_H
