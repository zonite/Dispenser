#ifndef KERNELCLIENT_H
#define KERNELCLIENT_H

//#include <QTcpServer>
#include <QObject>
#include <QSocketNotifier>
#include <QDataStream>
#include <QBuffer>

#include <linux/netlink.h>
#include <linux/genetlink.h>

#include <unititem.h>
#include <alarm.h>

#include "daemon.h"
#include "buffer.h"

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

/**
 * Use similar struct as in Kernel source net/genetlink.h
 *
 * struct genl_info - receiving information
 * @snd_seq: sending sequence number
 * @snd_portid: netlink portid of sender
 * @nlhdr: netlink message header
 * @genlhdr: generic netlink message header
 * @userhdr: user specific header
 * @attrs: netlink attributes
 * @_net: network namespace
 * @user_ptr: user pointers
 * @extack: extended ACK report struct
 */
struct genl_info {
	__u32			snd_seq;
	__u32			snd_portid;
	struct nlmsghdr *	nlhdr;
	struct genlmsghdr *	genlhdr;
	void *			userhdr;
	struct nlattr **	attrs;
//	possible_net_t		_net;
	void *			user_ptr[2];
	struct netlink_ext_ack *extack;
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

class KernelStream : protected QDataStream
{
public:
	explicit KernelStream();

	QDataStream &operator<<(const char *s) = delete;
	QDataStream &operator<<(QVariant s) = delete;
	QDataStream &operator<<(QVariant *s) = delete;

	KernelStream &operator<<(nlmsghdr &s);
	KernelStream &operator<<(genlmsghdr &s);
	KernelStream &operator<<(nlattr &s);
	virtual KernelStream &operator<<(const QByteArray *s);
	template <typename T> KernelStream &operator<<(const T s);
	//KernelStream &operator<<(const char *s) override;

	KernelStream &operator>>(const nlmsghdr **s);
	KernelStream &operator>>(const genlmsghdr **s);
	KernelStream &operator>>(const nlattr **s);

	void *cur();
	const void *constCur();

	KernelStream &align();
	KernelStream &alignAttr();

	inline int size() { return *pSize(); }
	inline __u32 uSize() { return (__u32)*pSize(); }
	int *pSize();

	//inline qint64 pos() { return *pPos(); }
	//qint64 *pPos();
	qint64 pos();

	using QDataStream::device;
	using QDataStream::setDevice;

private:
	QBuffer m_cBuffer;
};


class KernelClient : public QObject
{
	Q_OBJECT
public:
	explicit KernelClient(QObject *parent = nullptr);
	~KernelClient();

	//Methods to access kernel
	void setLight(int on);
	void setNight(int on);
	//void setRelease(Alarm *alarm); //Bit mask days which to fire. Seconds from 00:00 localtime to release. Num col to release (negative to release unit)

	static KernelClient *getKernel() { return m_pClient; }
signals:
	void stopped();

public slots:
	void start(const QStringList &arguments);
	void stop();

private slots:
	//void threadStarted();
	//void threadFinished();
	void readyRead();

	//Initialize data
	void connectCols(UnitItem *unit);
	void connectSlots(ColItem *col);
	void moduleAndDaemonInitialized(UnitItem *unit);

	void setUnitStatus();
	void getColStatus(ColItem *col);

	void connectCol(ColItem *col);

	//Releasing:
	void release(__s8 col = -1, __s8 slot = -1, bool force = false, int count = 1); //release now
	void release(SlotItem *slot, bool force = false); //release now
	void release(ColItem *col, bool force = false, int count = 1); //release now

	void releaseUnit(UnitItem *unit);
	void releaseCol(ColItem *col);

protected:
//	void incomingConnection(qintptr) Q_DECL_OVERRIDE;

private:

	//Genetlink interface:
	static struct nlmsghdr* nl_hdr_put(KernelStream *out);
	static struct genlmsghdr* genl_hdr_put(KernelStream *out, quint8 cmd, quint8 version = 1);
	//static struct nl_attr* nl_attr_put(KernelStream *out, quint16 type, const char *str);

	static nlattr *nl_attr_put_str(KernelStream *out, quint16 type, const QByteArray *str);
	template <typename T> static struct nlattr *nl_attr_put(KernelStream *out, quint16 type, const T data);

	/*
	static struct nlattr *nl_attr_put(KernelStream *out, quint16 type, const __s8 data);
	static struct nlattr *nl_attr_put(KernelStream *out, quint16 type, const __u16 data);
	static struct nlattr *nl_attr_put(KernelStream *out, quint16 type, const __s16 data);
	static struct nlattr *nl_attr_put(KernelStream *out, quint16 type, const __u32 data);
	static struct nlattr *nl_attr_put(KernelStream *out, quint16 type, const __s32 data);
	static struct nlattr *nl_attr_put(KernelStream *out, quint16 type, const __u64 data);
	static struct nlattr *nl_attr_put(KernelStream *out, quint16 type, const __s64 data);
	*/

	//static struct nl_attr* nl_attr_put(struct nlmsghdr *nlh, uint16_t type,size_t len, const void *data);
	char *string_strldup(const char *src, size_t size);
	void *nl_attr_get_payload(const struct nlattr *attr);
	const char *nl_attr_get_str(const struct nlattr *attr);
	uint16_t nl_attr_get_u16(const struct nlattr *attr);
	uint16_t nl_attr_get_type(const struct nlattr *attr);
	int nl_attr_type_valid(const struct nlattr *attr, uint16_t max);
	int data_attr_cb(const struct nlattr *attr, void *data);
	//int netlink_parse_nlmsg(netlink_sock_t *nls, char *buf, ssize_t buflen);
	//static ssize_t netlink_recvmsg(netlink_sock_t *nls, struct msghdr *msg, char *buf, size_t bufsz, char **outbuf, size_t *outbufsz);
	//void read_socket(int fd,  nls_opaque, int status);
	ssize_t sendToKernel(KernelStream *out);
	ssize_t recvFromKernel(void);
	void enableEvents(void);

	/** Dispenser kernel receiver interface */
	void process_dispenser_message(Buffer &in); //Receives GENL Dispenser message from kernel
	void parse_dispenser_nlattr(Buffer &in, struct nlattr **attrs); //Parse attributes in GENL Dispenser CMD
	void parse_slot_cmd(struct nlattr *attrs[]); //Parse slot status CMD
	void parse_col_cmd(struct nlattr *attrs[]); //Parse col status CMD
	void parse_unit_cmd(struct nlattr *attrs[]); //Parse unit status CMD


	ssize_t process_error_message(Buffer &in);
	ssize_t process_control_message(Buffer &in);
	void process_control_newfamily(Buffer &in);
	//void process_dispenser_message(Buffer &in);
	//ssize_t process_event(KernelStream *in);
	ssize_t get_nlattr_data(struct nlattr *attr, char **str);
	ssize_t get_nlattr_data(struct nlattr *attr, QByteArray *str);
	ssize_t get_nlattr_data(struct nlattr *attr, __u8 **i);
	ssize_t get_nlattr_data(struct nlattr *attr, __u16 **i);
	ssize_t get_nlattr_data(struct nlattr *attr, __u32 **i);

	int open_and_bind_socket();
	int resolve_family_id_by_name();
	int get_unit_status();
	int failed;


	KernelStream &initRequest(KernelStream *buffer, enum DISPENSER_GENL_COMMAND cmd);


	//Kernel commands:
	void getUnitStatus();
	void getSlotStatus(SlotItem *slot);
	void setColStatus(ColItem *col);
	void setSlotStatus(SlotItem *slot);


	//Data

	static KernelClient *m_pClient;

	/** The family ID resolved by Generic Netlink control interface. Assigned when the kernel module registers the Family */
	int nl_family_id = -1;
	/** Netlink socket's file descriptor. */
	int nl_fd = -1;
	/** Netlink socket address */
	struct sockaddr_nl nl_address;
	//struct generic_netlink_msg nl_request_msg;
	//struct generic_netlink_msg nl_response_msg;

	QSocketNotifier *m_pKernel = nullptr;

	/** Websocket server */
	WebSocketServer *m_pServer = nullptr;
	/** Datamodel */
	UnitItem m_cUnit;
	/** Alarmmodel */

	QBuffer mOutBuffer;
	Buffer mInBuffer;

	KernelStream mToKernel;
	KernelStream mFromKernel;

	unsigned int msg_seq = 0; //sequence num of nl messages to kernel.
};


#endif // KERNELCLIENT_H
