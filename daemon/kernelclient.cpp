#include <QDebug>
#include <QDaemonLog>
#include <QCoreApplication>
#include <QSocketNotifier>
#include <QCommandLineParser>

#include <sys/socket.h>
#include <unistd.h>

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

