#include <QDebug>
#include <QDaemonLog>
#include <QCoreApplication>
#include <QSocketNotifier>
#include <QCommandLineParser>

#include <sys/socket.h>
#include <unistd.h>

#include <netlink/attr.h>
#include <netlink/netlink.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/ctrl.h>

#include <dispenser.h>

#include "kernelclient.h"

#include "websocketserver.h"


// Generic macros for dealing with netlink sockets
#define GENLMSG_DATA(glh) ((void *)(((char *)NLMSG_DATA(glh) + GENL_HDRLEN)))
#define GENLMSG_PAYLOAD(glh) (NLMSG_PAYLOAD(glh, 0) - GENL_HDRLEN)
#define NLA_DATA(na) ((void *)((char *)(na) + NLA_HDRLEN))

/**
 * Structure describing the memory layout of a Generic Netlink layout.
 * The buffer size of 256 byte here is chosen at will and for simplicity.
 */
struct generic_netlink_msg {
    /** Netlink header comes first. */
    struct nlmsghdr n;
    /** Afterwards the Generic Netlink header */
    struct genlmsghdr g;
    /** Custom data. Space for Netlink Attributes. */
    char buf[256];
};

// Global Variables used for our Netlink example
/** Number of bytes sent or received via send() or recv() */
int nl_rxtx_length;
/** Pointer to Netlink attributes structure within the payload */
struct nlattr *nl_na;
/** Memory for Netlink request message. */
struct generic_netlink_msg nl_request_msg;
/** Memory for Netlink response message. */
struct generic_netlink_msg nl_response_msg;


KernelClient::KernelClient(QObject *parent)
        : QObject{parent}
{
	qRegisterMetaType<qintptr>("qintptr");
}

KernelClient::~KernelClient()
{
	if (nl_fd > 0) {
		::close(nl_fd);
		nl_fd = -1;
	}
}


void KernelClient::start(const QStringList &arguments)
{
	quint16 port = DISPENSER_TCP_PORT;
	const QCommandLineOption portOption(QStringList() << QStringLiteral("p") << QStringLiteral("port"), QCoreApplication::translate("main", "The port the server will listen to."), QStringLiteral("<port number>"));

	QCommandLineParser parser;
	parser.addOption(portOption);
	parser.parse(arguments);

	if (parser.isSet(portOption))  {
		bool ok;
		quint16 portOptionValue = parser.value(portOption).toInt(&ok);
		if (ok)
			port = portOptionValue;
	}

	if (!m_pKernel) {
		open_and_bind_socket();
	}

	qDebug() << "Dispenser Kernel Daemon started. NL family id" << nl_family_id;

	if (!m_pServer)
		m_pServer = new WebSocketServer(port, this);
}

void KernelClient::stop()
{
	if (m_pServer) {
		delete m_pServer;
		m_pServer = nullptr;
	}

	if (m_pKernel) {
		delete m_pKernel;
		m_pKernel = nullptr;
	}

	if (nl_fd > 0) {
		::close(nl_fd);
		nl_fd = -1;
	}
	//if (!isListening())
	//	return;

	//close();

	emit stopped();
}

/**
 *  Read data from kernel.
 */

void KernelClient::readyRead()
{
	qDebug() << "Received data from kernel.";
}

/**
 * nl_attr_put - add an attribute to netlink message
 * @param nlh pointer to the netlink message
 * @param type netlink attribute type that you want to add
 * @param len netlink attribute payload length
 * @param data pointer to the data that will be stored by the new attribute
 *
 * This function updates the length field of the Netlink message (nlmsg_len)
 * by adding the size (header + payload) of the new attribute.
 * From libmnl: https://www.netfilter.org/projects/libmnl/index.html
 */

//#define NL_ALIGN(len) (((len)+3) & ~(3))

void KernelClient::nl_attr_put(nlmsghdr *nlh, uint16_t type, size_t len, const void *data)
{
	struct nlattr *attr = (struct nlattr *)((char *)nlh + NLA_ALIGN(nlh->nlmsg_len));
	uint16_t payload_len = NLA_ALIGN(sizeof(struct nlattr)) + len;
	//uint16_t payload_len = NLA_ALIGN(sizeof(struct nlattr)) + len;
	int pad;

	attr->nla_type = type;
	attr->nla_len = payload_len;
	memcpy((char *)attr + NLA_HDRLEN, data, len);
	pad = NLA_ALIGN(len) - len;
	if (pad > 0)
		memset((char *)attr + NLA_HDRLEN + len, 0, pad);

	nlh->nlmsg_len += NLA_ALIGN(payload_len);
}


int KernelClient::open_and_bind_socket()
{
	// Step 1: Open the socket. Note that protocol = NETLINK_GENERIC in the address family of Netlink (AF_NETLINK)
	nl_fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_GENERIC);
	if (nl_fd < 0) {
		//qDebug() << "Error creating socket()";
		qDaemonLog(QStringLiteral("Error creating socket(). Couldn't start the server. socket(AF_NETLINK, SOCK_RAW, NETLINK_GENERIC) failed"), QDaemonLog::ErrorEntry);
		qApp->quit();
		return -1;
	}

	// Step 2: Bind the socket.
	memset(&nl_address, 0, sizeof(nl_address));
	// tell the socket (nl_address) that we use NETLINK address family
	nl_address.nl_family = AF_NETLINK;

	if (bind(nl_fd, (struct sockaddr *)&nl_address, sizeof(nl_address)) < 0) {
		//close(nl_fd);
		//qDebug() << "Error binding socket";
		qDaemonLog(QStringLiteral("Error binding socket. Couldn't start the server. bind(nl_fd, (struct sockaddr *)&nl_address, sizeof(nl_address) failed"), QDaemonLog::ErrorEntry);
		qApp->quit();
		return -1;
	}

	return resolve_family_id_by_name();
}

int KernelClient::resolve_family_id_by_name()
{
	// Step 3. Resolve the family ID corresponding to the macro `FAMILY_NAME` from `gnl_foobar_xmpl_prop.h`
	// We use come CTRL mechanisms that are part of the Generic Netlink infrastructure.
	// This part is usually behind a nice abstraction in each library, something like
	// `resolve_family_id_by_name()`. You can find more in
	// Linux code: https://elixir.bootlin.com/linux/latest/source/include/uapi/linux/genetlink.h#L30

	// This is required because before we can actually talk to our custom Netlink family, we need the numeric id.
	// Next we ask the Kernel for the ID.

	// Populate the netlink header
	nl_request_msg.n.nlmsg_type = GENL_ID_CTRL;
	// NLM_F_REQUEST is REQUIRED for kernel requests, otherwise the packet is rejected!
	// Kernel reference: https://elixir.bootlin.com/linux/v5.10.16/source/net/netlink/af_netlink.c#L2487
	nl_request_msg.n.nlmsg_flags = NLM_F_REQUEST;
	nl_request_msg.n.nlmsg_seq = 0;
	nl_request_msg.n.nlmsg_pid = getpid();
	nl_request_msg.n.nlmsg_len = NLMSG_LENGTH(GENL_HDRLEN);
	// Populate the payload's "family header" : which in our case is genlmsghdr
	nl_request_msg.g.cmd = CTRL_CMD_GETFAMILY;
	nl_request_msg.g.version = 1;
	// Populate the payload's "netlink attributes"
	nl_na = (struct nlattr *)GENLMSG_DATA(&nl_request_msg);
	nl_na->nla_type = CTRL_ATTR_FAMILY_NAME;

	QByteArray name(DISPENSER_GENL_NAME);
	name.truncate(15);

	nl_na->nla_len = name.length() + 1 + NLA_HDRLEN;
	qstrncpy((char *)NLA_DATA(nl_na), DISPENSER_GENL_NAME, 15); //Family name length can be upto 16 chars including \0

	nl_request_msg.n.nlmsg_len += NLMSG_ALIGN(nl_na->nla_len);
	//NLMSG_ALIGNTO = 4U

	// tell the socket (nl_address) that we use NETLINK address family and that we target
	// the kernel (pid = 0)
	nl_address.nl_family = AF_NETLINK;
	nl_address.nl_pid = 0; // <-- we target the kernel; kernel pid is 0
	nl_address.nl_groups = 0; // we don't use multicast groups
	nl_address.nl_pad = 0;

	// Send the family ID request message to the netlink controller
	nl_rxtx_length = sendto(nl_fd, (char *)&nl_request_msg, nl_request_msg.n.nlmsg_len,
	                        0, (struct sockaddr *)&nl_address, sizeof(nl_address));
	if ((__u32)nl_rxtx_length != nl_request_msg.n.nlmsg_len) {
		::close(nl_fd);
		qDaemonLog(QStringLiteral("Error sending family id request"), QDaemonLog::ErrorEntry);
		qApp->quit();
		return -1;
	}

	// Wait for the response message
	nl_rxtx_length = recv(nl_fd, &nl_response_msg, sizeof(nl_response_msg), 0);
	if (nl_rxtx_length < 0) {
		qDaemonLog(QStringLiteral("Error receiving family id request result"), QDaemonLog::ErrorEntry);
		qApp->quit();
		return -1;
	}

	// Validate response message
	if (!NLMSG_OK((&nl_response_msg.n), (__u32)nl_rxtx_length)) {
		qDaemonLog(QStringLiteral("Family ID request : invalid message"), QDaemonLog::ErrorEntry);
		qDaemonLog(QStringLiteral("Error validating family id request result: invalid length"), QDaemonLog::ErrorEntry);
		qApp->quit();
		return -1;
	}
	if (nl_response_msg.n.nlmsg_type == NLMSG_ERROR) { // error
		qDaemonLog(QStringLiteral("Family ID request : receive error"), QDaemonLog::ErrorEntry);
		qDaemonLog(QStringLiteral("Error validating family id request result: receive error"), QDaemonLog::ErrorEntry);
		qApp->quit();
	    return -1;
	}

	// Extract family ID
	nl_na = (struct nlattr *)GENLMSG_DATA(&nl_response_msg);
	nl_na = (struct nlattr *)((char *)nl_na + NLA_ALIGN(nl_na->nla_len));
	if (nl_na->nla_type == CTRL_ATTR_FAMILY_ID) {
		nl_family_id = *(__u16 *)NLA_DATA(nl_na);
	}
	//NLA_ALIGNTO = 4

	if (-1 == nl_family_id) {
		qDaemonLog(QStringLiteral("Invalid kernel response. Is kernel driver installed?"), QDaemonLog::ErrorEntry);
		qApp->quit();
	}

	m_pKernel = new QSocketNotifier(nl_fd, QSocketNotifier::Read, this);
	connect(m_pKernel, SIGNAL(activated(int)), this, SLOT(readyRead()));
	m_pKernel->setEnabled(true);

	//connect(m_pKernel, QOverload<QSocketDescriptor, QSocketNotifier::Type>::of(&QSocketNotifier::activated),
	//    [=](QSocketDescriptor socket, QSocketNotifier::Type Read){ /* ... */ });

	return 0;
}

int KernelClient::get_unit_status()
{
	/** Send Message to the Kernel requesting inut status */
	memset(&nl_request_msg, 0, sizeof(nl_request_msg));
	memset(&nl_response_msg, 0, sizeof(nl_response_msg));

	nl_request_msg.n.nlmsg_len = NLMSG_LENGTH(GENL_HDRLEN);
	// This is NOT the property for proper "routing" of the netlink message (that is located in the socket struct).
	// This is family id for "good" messages or NLMSG_ERROR (0x2) for error messages
	nl_request_msg.n.nlmsg_type = nl_family_id;

	// You can use flags in an application specific way, e.g. NLM_F_CREATE or NLM_F_EXCL.
	// Some flags have pre-defined functionality, like NLM_F_DUMP or NLM_F_ACK (Netlink will
	// do actions before your callback in the kernel can start its processing). You can see
	// some examples in https://elixir.bootlin.com/linux/v5.10.16/source/net/netlink/af_netlink.c
	//
	// NLM_F_REQUEST is REQUIRED for kernel requests, otherwise the packet is rejected!
	// Kernel reference: https://elixir.bootlin.com/linux/v5.10.16/source/net/netlink/af_netlink.c#L2487
	//
	// if you add "NLM_F_DUMP" flag, the .dumpit callback will be invoked in the kernel
	nl_request_msg.n.nlmsg_flags = NLM_F_REQUEST;
	// It is up to you if you want to split a data transfer into multiple sequences. (application specific)
	nl_request_msg.n.nlmsg_seq = 0;
	// Port ID. Not necessarily the process id of the current process. This field
	// could be used to identify different points or threads inside your application
	// that send data to the kernel. This has nothing to do with "routing" the packet to
	// the kernel, because this is done by the socket itself
	nl_request_msg.n.nlmsg_pid = getpid();
	// nl_request_msg.g.cmd = GNL_FOOBAR_XMPL_C_REPLY_WITH_NLMSG_ERR;
	nl_request_msg.g.cmd = DISPENSER_GENL_CMD_UNIT_STATUS;
	// You can evolve your application over time using different versions or ignore it.
	// Application specific; receiver can check this value and do specific logic.
	nl_request_msg.g.version = 1; // app specific; we don't use this on the receiving side in our example


	nl_na = (struct nlattr *)GENLMSG_DATA(&nl_request_msg);
	nl_na->nla_type = DISPENSER_GENL_CMD_UNSPEC;GNL_FOOBAR_XMPL_A_MSG
	nl_na->nla_len = sizeof(MESSAGE_TO_KERNEL) + NLA_HDRLEN; // Message length
	memcpy(NLA_DATA(nl_na), MESSAGE_TO_KERNEL, sizeof(MESSAGE_TO_KERNEL));
	nl_request_msg.n.nlmsg_len += NLMSG_ALIGN(nl_na->nla_len);

}

