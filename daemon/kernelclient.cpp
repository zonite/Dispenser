#include <QDebug>
#include <QDaemonLog>
#include <QCoreApplication>
#include <QSocketNotifier>
#include <QCommandLineParser>

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/fcntl.h>
#include <linux/sockios.h>
//#include <sys/user.h>
#include <unistd.h>

//libnl (Netlink)
//#include <netlink/attr.h>
//#include <netlink/netlink.h>
//#include <netlink/genl/genl.h>
//#include <netlink/genl/ctrl.h>

#include <linux/netlink.h>

#include <dispenser.h>

#include "kernelclient.h"

#include "websocketserver.h"

#if NLA_ALIGNTO != NLMSG_ALIGNTO
#error "NLA_ALIGNTO and NLMSG_ALIGNTO are not equal!"
#endif

#define MASK(align) (align - 1)
#define PAD(len,align) ((align - (len & MASK(align))) & MASK(align))

#define NLMSG_PAD(size) PAD(size,NLMSG_ALIGNTO)
#define NLA_PAD(size) PAD(size,NLA_ALIGNTO)

//#define PAGE_SIZE NLMSG_GOODSIZE
//#define PAGE_SIZE 8192UL

// Generic macros for dealing with netlink sockets
#define GENLMSG_DATA(glh) ((void *)(((char *)NLMSG_DATA(glh) + GENL_HDRLEN)))
#define GENLMSG_PAYLOAD(glh) (NLMSG_PAYLOAD(glh, 0) - GENL_HDRLEN)
#define NLA_DATA(na) ((void *)((char *)(na) + NLA_HDRLEN))


/** Template instantiations **/

template class KernelStreamIterator<nlattr>;
template class KernelStreamIterator<nlmsghdr>;
template class KernelStreamIterator<genlmsghdr>;

//template KernelStream &KernelStream::operator<<<int>(const int s);
template KernelStream &KernelStream::operator<<(const __u8 s);
template KernelStream &KernelStream::operator<<(const __s8 s);
template KernelStream &KernelStream::operator<<(const __u16 s);
template KernelStream &KernelStream::operator<<(const __s16 s);
template KernelStream &KernelStream::operator<<(const __u32 s);
template KernelStream &KernelStream::operator<<(const __s32 s);
template KernelStream &KernelStream::operator<<(const __u64 s);
template KernelStream &KernelStream::operator<<(const __s64 s);


// Global Variables used for our Netlink example
/** Number of bytes sent or received via send() or recv() */
int nl_rxtx_length;
/** Pointer to Netlink attributes structure within the payload */
//struct nlattr *nl_na;
/** Memory for Netlink request message. */
//struct generic_netlink_msg nl_request_msg;
/** Memory for Netlink response message. */
//struct generic_netlink_msg nl_response_msg;

KernelClient *KernelClient::m_pClient = nullptr;

KernelClient::KernelClient(QObject *parent)
        : QObject{parent}
{
        qDaemonLog(QStringLiteral("KernelClient created."), QDaemonLog::NoticeEntry);

        nl_address.nl_family = AF_NETLINK;
        nl_address.nl_pid = 0; // <-- we target the kernel; kernel pid is 0
        nl_address.nl_groups = 0; // we don't use multicast groups
        nl_address.nl_pad = 0;

        m_pClient = this;

        qRegisterMetaType<qintptr>("qintptr");

        connect(&m_cUnit, &UnitItem::releaseEvent, this, &KernelClient::releaseUnit);
	connect(&m_cUnit, &UnitItem::newCol, this, &KernelClient::connectCol);
	connect(&m_cUnit, &UnitItem::initialized, this, &KernelClient::setUnitStatus);
}

void KernelClient::connectCol(ColItem *col)
{
	connect(col, &ColItem::releaseEvent, this, &KernelClient::releaseCol);
	//connect(col, &ColItem::newSlot, this, &KernelClient::connectSlots);

	//getColStatus(col);
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
	qDebug() << "Open and bind socket.";

	if (!m_pKernel) {
		open_and_bind_socket();
	}

	qDebug() << "Dispenser Kernel Daemon started. NL family id" << nl_family_id;

	if (nl_family_id == -1)
		qApp->quit();

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
		mOutBuffer.close();
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
	recvFromKernel();
}

// will reset buffer
nlmsghdr *KernelClient::nl_hdr_put(KernelStream *out)
{
	QBuffer *buf = dynamic_cast<QBuffer *>(out->device());

	//Wrong type of stream if nullptr returned.
	if (!buf)
		return nullptr;

	//buf->buffer().clear(); //Will deallocate
	//buf->buffer().truncate(0);
	//buf->buffer().reserve(PAGE_SIZE);
	buf->buffer().resize(PAGE_SIZE);
	buf->buffer().fill(0);
	buf->buffer().data_ptr()->size = 0;
	buf->seek(0);
	//buf->reset();
	//qsizetype pos = buf->buffer().size();

	struct nlmsghdr nlmsg_hdr = {0, 0, 0, 0, 0};
	KernelStreamIterator<nlmsghdr> i(out);
	*out << nlmsg_hdr;

	return i.data();
	//return (nlmsghdr *)&buf->buffer().data()[pos];

	//buffer->buffer().append(NLA_HDRLEN, 0);
	//buffer->seek(NLA_HDRLEN);
	//buffer->buffer().data_ptr();

	//PAGE_SIZE;

	//QByteArray::pointer i = buffer->buffer().data_ptr();

	//buffer->pos();
	//nlmsghdr *hdr = (nlmsghdr *)buffer->buffer().data();
}

genlmsghdr *KernelClient::genl_hdr_put(KernelStream *out, quint8 cmd, quint8 version)
{
	QBuffer *buf = dynamic_cast<QBuffer *>(out->device());

	//Wrong type of stream if nullptr returned.
	if (!buf)
		return nullptr;

	struct genlmsghdr genlmsg_hdr = {
		.cmd = cmd,
		.version = version,
		.reserved = 0};

	KernelStreamIterator<genlmsghdr> i(out);
	*out << genlmsg_hdr;

	return i.data();
	//qsizetype pos = buf->buffer().size();
	//return (genlmsghdr *)&buf->buffer().data()[pos];
}

/**
 *  <------- NLA_HDRLEN ------> <-- NLA_ALIGN(payload)-->
 * +---------------------+- - -+- - - - - - - - - -+- - -+
 * |        Header       | Pad |     Payload       | Pad |
 * |   (struct nlattr)   | ing |                   | ing |
 * +---------------------+- - -+- - - - - - - - - -+- - -+
 *  <-------------- nlattr->nla_len -------------->
 */

/**
 * nla_type (16 bits)
 * +---+---+-------------------------------+
 * | N | O | Attribute Type                |
 * +---+---+-------------------------------+
 * N := Carries nested attributes
 * O := Payload stored in network byte order
 *
 * Note: The N and O flag are mutually exclusive.
 */

nlattr *KernelClient::nl_attr_put_str(KernelStream *out, quint16 type, const QByteArray *str)
{
	QBuffer *buf = dynamic_cast<QBuffer *>(out->device());
	//Wrong type of stream if nullptr returned.
	if (!buf)
		return nullptr;

	struct nlattr attr = {
		.nla_len = 0,
		.nla_type = type
	};

	quint16 size = str->size() + 1;
	attr.nla_len = NLA_HDRLEN + size;

	KernelStreamIterator<nlattr> i(out);
	*out << attr;
	//*out << str->constData();
	*out << str;
	//out->pad(pad);

	return i.data();
}

/*
nlattr *KernelClient::nl_attr_put(KernelStream *out, quint16 type, const __u8 data)
{
	QBuffer *buf = dynamic_cast<QBuffer *>(out->device());
	//Wrong type of stream if nullptr returned.
	if (!buf)
		return nullptr;

	struct nlattr attr = {
		.nla_len = 0,
		.nla_type = type
	};

	quint16 size = sizeof(data);
	attr.nla_len = NLA_HDRLEN + size;

	KernelStreamIterator<nlattr> i(out);
	*out << attr;
	// *out << str->constData();
	*out << data;
	//out->pad(pad);

	return i.data();
}
*/

template<typename T>
nlattr *KernelClient::nl_attr_put(KernelStream *out, quint16 type, const T data)
{
	QBuffer *buf = dynamic_cast<QBuffer *>(out->device());
	//Wrong type of stream if nullptr returned.
	if (!buf)
		return nullptr;

	struct nlattr attr = {
		.nla_len = 0,
		.nla_type = type
	};

	quint16 size = sizeof(data);
	attr.nla_len = NLA_HDRLEN + size;

	KernelStreamIterator<nlattr> i(out);
	*out << attr;
	//*out << str->constData();
	*out << data;
	//out->pad(pad);

	return i.data();
}

ssize_t KernelClient::sendToKernel(KernelStream *out)
{
	if (nl_fd < 0)
		return nl_fd;

	QBuffer *buf = dynamic_cast<QBuffer *>(out->device());

	if (!buf)
		return -1;

	const QByteArray *array = &buf->data();
	struct nlmsghdr *hdr = (struct nlmsghdr *) array->constData();
	hdr->nlmsg_len = array->size();
	hdr->nlmsg_seq = msg_seq++;

	return sendto(nl_fd, buf->data().constData(), hdr->nlmsg_len,
	              0, (struct sockaddr *)&nl_address, sizeof(nl_address));
}

ssize_t KernelClient::recvFromKernel(void)
{
	ssize_t nl_rx_length = 0;
	struct nlmsghdr *nlmsg = nullptr;
	Buffer inBuffer;

	if (nl_fd < 0)
		return nl_fd;

	//int err = ioctl(nl_fd, FIONREAD, &nl_rx_length); //return bytes available in buffer for read. Get the number of bytes in the input buffer.
	//int err = ioctl(nl_fd, SIOCINQ, &nl_rx_length); //return bytes available in buffer for read. Get the number of bytes in the input buffer.
	//if (err) {
	//	qDaemonLog(QString("IOCTL error %1 errno %2.").arg(err).arg(errno), QDaemonLog::NoticeEntry);
	//}
	//qDaemonLog(QString("Buffer has %1 bytes.").arg(nl_rx_length), QDaemonLog::NoticeEntry);

	//int available;
	//socklen_t optlen = sizeof(available);
	//err = getsockopt(nl_fd, SOL_SOCKET, SO_NREAD, &available, &optlen);

	/*
	nl_rx_length = fcntl(nl_fd, F_GETFL);
	if (!m_pKernel->isEnabled() && (nl_rx_length & O_NONBLOCK)) {
		qDaemonLog(QStringLiteral("Buffer switch to blocking state."), QDaemonLog::NoticeEntry);
		fcntl(nl_fd, F_SETFL, nl_rx_length & (~O_NONBLOCK));
	}
	*/

	do {
		inBuffer.clear();

		nl_rx_length = recv(nl_fd, inBuffer.cur(), inBuffer.capacityLeft(), 0);

		if (nl_rx_length == -1 && (errno == EAGAIN)) {
			qDaemonLog(QStringLiteral("FD has no more data."), QDaemonLog::NoticeEntry);
			break;
		}

		//0x7ffffff880; 0x7ffffff894 ; 0x7ffffff898
		//nlmsg
		// type 16 = GENL_ID_CTRL (controller messages)
		//genlmsg
		// cmd 1 = CTRL_CMD_NEWFAMILY
		//nlattr
		// type 2 = CTRL_ATTR_FAMILY_NAME
		// type 1 = CTRL_ATTR_FAMILY_ID

		if (nl_rx_length < 0) {
			qDaemonLog(QStringLiteral("Error receiving from kernel. Error %1.").arg(errno), QDaemonLog::ErrorEntry);
			//qApp->quit();
			break;
			//return nl_rx_length;
		}

		inBuffer.resize(nl_rx_length); //Resize to the length of the data in buffer
		inBuffer >> &nlmsg;

		if (!nlmsg || !NLMSG_OK((nlmsg), (__u32)nl_rx_length)) {
			//qDaemonLog(QStringLiteral("Family ID request : invalid message"), QDaemonLog::ErrorEntry);
			qDaemonLog(QStringLiteral("Error validating Kernel response: invalid length or nullptr"), QDaemonLog::ErrorEntry);
			//qApp->quit();
			continue;
			//return -1;
		}

		if (nl_family_id > 0 && nlmsg->nlmsg_type == nl_family_id) {
			//Process Dispenser message
			process_dispenser_message(inBuffer);
			continue;
		}

		enableEvents();

		//Process standard messages
		switch (nlmsg->nlmsg_type) {
		case NLMSG_NOOP: //No-op. skip message.
			qDaemonLog(QStringLiteral("NL message skip."), QDaemonLog::ErrorEntry);
			break;
		case NLMSG_DONE: //Dump done. skip message.
			qDaemonLog(QStringLiteral("NL message dump done."), QDaemonLog::ErrorEntry);
			break;
		case NLMSG_ERROR:
			qDaemonLog(QStringLiteral("Error validating Kernel response: receive error"), QDaemonLog::ErrorEntry);
			//qApp->quit();
			//return -1;
			process_error_message(inBuffer);
			break;
		case GENL_ID_CTRL: //GENL controller requests.
			//Request GENL ID.
			process_control_message(inBuffer);
			break;
		default:
			qDaemonLog(QStringLiteral("Unknown type from netlink message"), QDaemonLog::ErrorEntry);
			//qApp->quit();
			//return -1;
			break;
		}


	} while (nl_rx_length);

	return nl_rx_length;

	/**
	 *
	 * Buffer has
	 *
	 * struct nlmsghdr (.nlmsg_type = family_id)
	 * struct genlmsghdr (.cmd = enum DISPENSER_GENL_COMMAND)
	 * struct nlattr (.nla_type = enum DISPENSER_GENL_ATTRIBUTE) multiple times
	 *
	 * */
}

void KernelClient::enableEvents()
{
	static bool enabled = false;
	int flags = 0;

	if (enabled)
		return;

	if (m_pKernel) {
		//m_pKernel = new QSocketNotifier(nl_fd, QSocketNotifier::Read, this);
		connect(m_pKernel, SIGNAL(activated(int)), this, SLOT(readyRead()));

		//family acquired... Switch to nonblocking state for event driven io.
		flags = fcntl(nl_fd, F_GETFL);
		fcntl(nl_fd, F_SETFL, flags | O_NONBLOCK);

		m_pKernel->setEnabled(true);
		enabled = true;
	}
}

void KernelClient::process_dispenser_message(Buffer &in)
{
	struct genlmsghdr *genl = nullptr;
	//struct genl_info genl_info = { 0, 0, 0, 0, 0, 0, { 0, 0 }, 0 };
	struct nlattr *attrs[DISPENSER_GENL_ATTR_COUNT] = { nullptr };

	in >> &genl;

	if (!genl) {
		//invalid
		qDaemonLog(QStringLiteral("Invalid genl header"), QDaemonLog::ErrorEntry);
		return;
	}

	//genl_info.attrs = attrs;
	//genl_info.genlhdr = genl;

	parse_dispenser_nlattr(in, attrs); //setup attrs struct with dispenser attrs

	switch (genl->cmd) {
	case DISPENSER_GENL_CMD_UNSPEC:
		qDaemonLog(QStringLiteral("Invalid command"), QDaemonLog::ErrorEntry);
		break;

	case DISPENSER_GENL_CMD_RELEASE: //action by attributes, for daemon, release event received (command received).
		qDaemonLog(QStringLiteral("Release Command received"), QDaemonLog::NoticeEntry);
		break;

	case DISPENSER_GENL_CMD_SLOT_STATUS: //u8 col, u8 slot, u8 state attr
		qDaemonLog(QStringLiteral("Slot Status Info"), QDaemonLog::NoticeEntry);
		parse_slot_cmd(attrs);
		break;

	case DISPENSER_GENL_CMD_COL_STATUS: //u8 col, u8 slot, u32 counter
		qDaemonLog(QStringLiteral("Col Status Info"), QDaemonLog::NoticeEntry);
		parse_col_cmd(attrs);
		break;

	case DISPENSER_GENL_CMD_UNIT_STATUS: //u8 col, u8 slot, u8 state attr
		qDaemonLog(QStringLiteral("Unit Status Info"), QDaemonLog::NoticeEntry);
		parse_unit_cmd(attrs);
		break;
	case DISPENSER_GENL_CMD_ENVIRONMENT: //u32 attr //raw temperature
		qDaemonLog(QStringLiteral("Environment Info"), QDaemonLog::NoticeEntry);
		break;
	case DISPENSER_GENL_CMD_TEMPERATURE_CALIBRATION: //calibration data
		qDaemonLog(QStringLiteral("Calibration Temperature Data"), QDaemonLog::NoticeEntry);
		break;
	case DISPENSER_GENL_CMD_PRESSURE_CALIBRATION: //calibration data
		qDaemonLog(QStringLiteral("Calibration Pressure Data"), QDaemonLog::NoticeEntry);
		break;
	case DISPENSER_GENL_CMD_HUMIDITY_CALIBRATION: //calibration data
		qDaemonLog(QStringLiteral("Calibration Humidity Data"), QDaemonLog::NoticeEntry);
		break;
//	case DISPENSER_GENL_CMD_DUMP: //Dumps the mmap-area
//		qDaemonLog(QStringLiteral("Memory Dump"), QDaemonLog::NoticeEntry);
//		break;
	}
}

ssize_t KernelClient::process_control_message(Buffer &in)
{
	struct genlmsghdr *genl = nullptr;

	in >> &genl;

	if (!genl) {
		//invalid
		return -1;
	}

	//enum control cmd:
	switch (genl->cmd) {
	case CTRL_CMD_UNSPEC:
		break;
	case CTRL_CMD_NEWFAMILY: //Return family ID.
		qDaemonLog(QStringLiteral("Netlink Control received New family."), QDaemonLog::NoticeEntry);
		process_control_newfamily(in);
		break;
	case CTRL_CMD_DELFAMILY:
		qDaemonLog(QStringLiteral("Netlink Control received Del family."), QDaemonLog::NoticeEntry);
		break;
	case CTRL_CMD_GETFAMILY:
		qDaemonLog(QStringLiteral("Netlink Control received Get family."), QDaemonLog::NoticeEntry);
		break;
	case CTRL_CMD_NEWOPS:
		qDaemonLog(QStringLiteral("Netlink Control received New Ops."), QDaemonLog::NoticeEntry);
		break;
	case CTRL_CMD_DELOPS:
		qDaemonLog(QStringLiteral("Netlink Control received Del Ops."), QDaemonLog::NoticeEntry);
		break;
	case CTRL_CMD_GETOPS:
		qDaemonLog(QStringLiteral("Netlink Control received Get Ops."), QDaemonLog::NoticeEntry);
		break;
	case CTRL_CMD_NEWMCAST_GRP:
		qDaemonLog(QStringLiteral("Netlink Control received New Mcast."), QDaemonLog::NoticeEntry);
		break;
	case CTRL_CMD_DELMCAST_GRP:
		qDaemonLog(QStringLiteral("Netlink Control received Del Mcast."), QDaemonLog::NoticeEntry);
		break;
	case CTRL_CMD_GETMCAST_GRP: /* unused */
		qDaemonLog(QStringLiteral("Netlink Control received Get Mcast."), QDaemonLog::NoticeEntry);
		break;
	case CTRL_CMD_GETPOLICY:
		qDaemonLog(QStringLiteral("Netlink Control received Get Policy."), QDaemonLog::NoticeEntry);
		break;
	}

	return in.size();
}

void KernelClient::parse_dispenser_nlattr(Buffer &in, nlattr **attrs)
{
	struct nlattr *attr = nullptr;
	__u16 i = DISPENSER_GENL_ATTR_COUNT * 2;

	in >> &attr;

	while (attr && i) {
		if (attr->nla_type < DISPENSER_GENL_ATTR_COUNT) {
			attrs[attr->nla_type] = attr;
		}

		in >> &attr;
		--i;
	}
}

void KernelClient::parse_slot_cmd(nlattr *attrs[])
{
	SlotItem *pSlot;
	//ColItem *pCol;
	__u8 *status;
	__u8 *col;
	__u8 *slot;
	__u32 *failed_up, *failed_down, *counter;

	if (!attrs[DISPENSER_GENL_COL_NUM] || !attrs[DISPENSER_GENL_SLOT_NUM]) {
		//No col or no slot
		qDaemonLog(QStringLiteral("Slot identifier bad"), QDaemonLog::ErrorEntry);
		return;
	}
	get_nlattr_data(attrs[DISPENSER_GENL_COL_NUM], &col);
	get_nlattr_data(attrs[DISPENSER_GENL_SLOT_NUM], &slot);

	pSlot = m_cUnit.slot(*col, *slot);
	//pCol = m_cUnit[*col];

	if (pSlot) {
		if (attrs[DISPENSER_GENL_SLOT_FAILED_UP]) {
			get_nlattr_data(attrs[DISPENSER_GENL_SLOT_FAILED_UP], &failed_up);
			pSlot->setFailedUp(*failed_up);
		}

		if (attrs[DISPENSER_GENL_SLOT_FAILED_DOWN]) {
			get_nlattr_data(attrs[DISPENSER_GENL_SLOT_FAILED_DOWN], &failed_down);
			pSlot->setFailedDown(*failed_down);
		}

		if (attrs[DISPENSER_GENL_SLOT_STATUS]) {
			struct dispenser_mmap_slot new_slot_state = { UNKNOWN, 0, 0, 0, 0, 0};
			unsigned char full = 0;

			get_nlattr_data(attrs[DISPENSER_GENL_SLOT_STATUS], &status);

			dispenser_unpack_slot_status(*status, &new_slot_state, &full);

			pSlot->setFull(full);
			pSlot->setUp(new_slot_state.up);
			pSlot->setDown(new_slot_state.down);
			pSlot->setRelease(new_slot_state.release);
			pSlot->setState(new_slot_state.state);

			qDaemonLog(QString("NL: Slot %1/%2 status full=%3, up=%4, down=%5, release=%6, state=%7.")
			           .arg(*col)
			           .arg(*slot)
			           .arg(full)
			           .arg(QString::number(new_slot_state.up))
			           .arg(QString::number(new_slot_state.down))
			           .arg(QString::number(new_slot_state.release))
			           .arg(SlotItem::stateToStr(new_slot_state.state)), QDaemonLog::NoticeEntry);

			//if (new_slot_state.state == UNKNOWN) {
			//	setSlotStatus(pSlot);
			//}
		}

		if (attrs[DISPENSER_GENL_MEM_COUNTER]) {
			get_nlattr_data(attrs[DISPENSER_GENL_MEM_COUNTER], &counter);
			m_cUnit.setCounter(*counter);
		}

		if (!m_cUnit.daemonInitialized()) { //Test if all cols and slots are initialized
			m_cUnit.checkInitialized();
		}
	}
}

void KernelClient::parse_col_cmd(nlattr *attrs[])
{
	ColItem *pCol;
	__u32 *counter;
	__u8 *col, *slot;

	if (!attrs[DISPENSER_GENL_COL_NUM]) {
		//No col or no slot
		qDaemonLog(QStringLiteral("Col identifier bad"), QDaemonLog::ErrorEntry);
		return;
	}

	get_nlattr_data(attrs[DISPENSER_GENL_COL_NUM], &col);

	pCol = m_cUnit.col(*col);

	if (pCol) {
		if (attrs[DISPENSER_GENL_SLOT_NUM]) {
			get_nlattr_data(attrs[DISPENSER_GENL_SLOT_NUM], &slot);
			pCol->setSlots(*slot);

			//if (pCol->setSlots(*slot)) {
			        //connectSlots(pCol);
			        //pCol->initSlots();
			//}
		}

		if (attrs[DISPENSER_GENL_MEM_COUNTER]) {
			get_nlattr_data(attrs[DISPENSER_GENL_MEM_COUNTER], &counter);
			m_cUnit.setCounter(*counter);
		}
	}
}

void KernelClient::parse_unit_cmd(nlattr *attrs[])
{
	struct dispenser_mmap_unit received_unit_data = { 0, 0, 0, 0, 0, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 0, 0, 0, 0 };
	__u32 *counter = 0;
	__u8 *status = 0, *num_cols = 0, *num_slots = 0, *initialized = 0;

	if (attrs[DISPENSER_GENL_UNIT_STATUS]) {
		get_nlattr_data(attrs[DISPENSER_GENL_UNIT_STATUS], &status);
		dispenser_unpack_unit_status(*status, &received_unit_data);
		m_cUnit.setDoor(received_unit_data.door);
		//m_cUnit.setNight(received_unit_data.night); //Night is never set by the module.
		m_cUnit.setCharging(received_unit_data.charging);
		m_cUnit.setLight(received_unit_data.light);
		qDaemonLog(QString("NL: Unit status door=%1, night=%2, charge=%3, light=%4.")
		           .arg(QString::number(received_unit_data.door))
		           .arg(QString::number(received_unit_data.night))
		           .arg(QString::number(received_unit_data.charging))
		           .arg(QString::number(received_unit_data.light)), QDaemonLog::NoticeEntry);
	}

	if (attrs[DISPENSER_GENL_COL_NUM]) {
		get_nlattr_data(attrs[DISPENSER_GENL_COL_NUM], &num_cols);
		m_cUnit.setCols(*num_cols);
		//connectCols();
		//m_cUnit.initCols();
	}

	if (attrs[DISPENSER_GENL_SLOT_NUM]) {
		get_nlattr_data(attrs[DISPENSER_GENL_SLOT_NUM], &num_slots);
		m_cUnit.setSlots(*num_slots);
	}

	if (attrs[DISPENSER_GENL_MEM_COUNTER]) {
		get_nlattr_data(attrs[DISPENSER_GENL_MEM_COUNTER], &counter);
		m_cUnit.setCounter(*counter);
	}

	if (attrs[DISPENSER_GENL_INITIALIZED]) {
		get_nlattr_data(attrs[DISPENSER_GENL_INITIALIZED], &initialized);
		m_cUnit.setInitialized(*initialized);
	}
}

/**
 * enum nlmsgerr_attrs - nlmsgerr attributes
 * @NLMSGERR_ATTR_UNUSED: unused
 * @NLMSGERR_ATTR_MSG: error message string (string)
 * @NLMSGERR_ATTR_OFFS: offset of the invalid attribute in the original
 *	 message, counting from the beginning of the header (u32)
 * @NLMSGERR_ATTR_COOKIE: arbitrary subsystem specific cookie to
 *	be used - in the success case - to identify a created
 *	object or operation or similar (binary)
 * @NLMSGERR_ATTR_POLICY: policy for a rejected attribute
 * @NLMSGERR_ATTR_MISS_TYPE: type of a missing required attribute,
 *	%NLMSGERR_ATTR_MISS_NEST will not be present if the attribute was
 *	missing at the message level
 * @NLMSGERR_ATTR_MISS_NEST: offset of the nest where attribute was missing
 * @__NLMSGERR_ATTR_MAX: number of attributes
 * @NLMSGERR_ATTR_MAX: highest attribute number
 */

ssize_t KernelClient::process_error_message(Buffer &in)
{
	struct nlattr *attr = nullptr;
	struct nlmsghdr *errorhdr = nullptr;
	struct genlmsghdr *errorgenlhdr = nullptr;
	__u16 i = __NLMSGERR_ATTR_MAX * 2;
	QByteArray str;
	__u32 *val;
	__s32 *error;

	in >> &error;

	if (error)
		qDaemonLog(QString("Netlink errno: %1").arg(*error), QDaemonLog::ErrorEntry);

	in >> &errorhdr;

	in >> &errorgenlhdr;

	in >> &attr;

	while (attr && i) {
		switch (attr->nla_type) {
		case NLMSGERR_ATTR_UNUSED:
			break;
		case NLMSGERR_ATTR_MSG:
			get_nlattr_data(attr, &str);
			qDaemonLog(QString("Netlink error: ")+str, QDaemonLog::ErrorEntry);
			break;
		case NLMSGERR_ATTR_OFFS:
			get_nlattr_data(attr, &val);
			qDaemonLog(QString("Netlink error attribute offset %1.").arg((__s32)*val), QDaemonLog::ErrorEntry);
			break;
		case NLMSGERR_ATTR_COOKIE:
			qDaemonLog(QString("Netlink error binary cookie length = %1.").arg(attr->nla_len), QDaemonLog::ErrorEntry);
			break;
		case NLMSGERR_ATTR_POLICY:
			get_nlattr_data(attr, &val);
			qDaemonLog(QString("Netlink error policy for rejected %1.").arg(*val), QDaemonLog::ErrorEntry);
			break;
		case NLMSGERR_ATTR_MISS_TYPE:
			get_nlattr_data(attr, &val);
			qDaemonLog(QString("Netlink error type of missing attribute=%1.").arg(*val), QDaemonLog::ErrorEntry);
			break;
		case NLMSGERR_ATTR_MISS_NEST:
			get_nlattr_data(attr, &val);
			qDaemonLog(QString("Netlink error offset of the missing attribute=%1.").arg(*val), QDaemonLog::ErrorEntry);
			break;
		}

		in >> &attr;
		--i;
	}

	return 0;
}

void KernelClient::process_control_newfamily(Buffer &in)
{
	struct nlattr *attr = nullptr;
	bool correct_family = false;
	__u16 *family_id = nullptr;
	__u16 lenght = 0; //attr->nla_len;
	char *name = nullptr;
	__u16 i = CTRL_ATTR_MAX * 2;
	__u32 *value = 0, *max_attr = 0;

	in >> &attr;

	while (attr && i) {
		switch (attr->nla_type) {
		case CTRL_ATTR_UNSPEC:
			//qDaemonLog(QStringLiteral("Control attributes end."), QDaemonLog::ErrorEntry);
			break;
		case CTRL_ATTR_FAMILY_ID: //u16
			get_nlattr_data(attr, &family_id);
			qDaemonLog(QString("Netlink Control received family id %1.").arg(*family_id), QDaemonLog::NoticeEntry);
			break;
		case CTRL_ATTR_FAMILY_NAME: //string
			lenght = get_nlattr_data(attr, &name);
			if (lenght > GENL_NAMSIZ)
				lenght = GENL_NAMSIZ;
			if (!strncmp(name, DISPENSER_GENL_NAME, lenght)) {
				correct_family = true;
				qDaemonLog(QString("Netlink Control received CORRECT family name = %1.").arg(name), QDaemonLog::NoticeEntry);
			} else {
				qDaemonLog(QString("Netlink Control received WRONG family name = %1.").arg(name), QDaemonLog::ErrorEntry);
			}
			break;
		case CTRL_ATTR_VERSION: //u32
			get_nlattr_data(attr, &value);
			qDaemonLog(QStringLiteral("Netlink Control received version (%1).").arg(*value), QDaemonLog::NoticeEntry);
			break;
		case CTRL_ATTR_HDRSIZE: //u32 //Size of user specified header
			get_nlattr_data(attr, &value);
			qDaemonLog(QStringLiteral("Netlink Control received hdrsize (%1).").arg(*value), QDaemonLog::NoticeEntry);
			break;
		case CTRL_ATTR_MAXATTR: //u32 //Size of attr list (dispenser max attr)
			get_nlattr_data(attr, &max_attr);
			if (*max_attr == DISPENSER_GENL_ATTR_MAX) {
				qDaemonLog(QStringLiteral("Netlink Control received CORRECT maxattr=%1 == %2.").arg(*max_attr).arg(DISPENSER_GENL_ATTR_MAX), QDaemonLog::NoticeEntry);
			} else {
				qDaemonLog(QStringLiteral("Netlink Control received WRONG maxattr=%1 == %2.").arg(*max_attr).arg(DISPENSER_GENL_ATTR_MAX), QDaemonLog::ErrorEntry);
			}
			break;
		case CTRL_ATTR_OPS: //supported opeations of dispenser, long list
			get_nlattr_data(attr, &value);
			qDaemonLog(QString("Netlink Control received ops == %1.").arg(*value), QDaemonLog::NoticeEntry);
			break;
		case CTRL_ATTR_MCAST_GROUPS:
			qDaemonLog(QStringLiteral("Netlink Control received groups."), QDaemonLog::NoticeEntry);
			break;
		case CTRL_ATTR_POLICY:
			qDaemonLog(QStringLiteral("Netlink Control received policy."), QDaemonLog::NoticeEntry);
			break;
		case CTRL_ATTR_OP_POLICY:
			qDaemonLog(QStringLiteral("Netlink Control received op policy."), QDaemonLog::ErrorEntry);
			break;
		case CTRL_ATTR_OP:
			qDaemonLog(QStringLiteral("Netlink Control received op."), QDaemonLog::ErrorEntry);
			break;
		}

		in >> &attr;
		--i;
	}

	if (correct_family && family_id) {
		nl_family_id = *family_id;

		enableEvents();
	}

	return;
}

/*
ssize_t KernelClient::process_event(KernelStream *in)
{
	QBuffer *buf = dynamic_cast<QBuffer *>(in->device());

	if (!buf)
		return -1;



	return -1;
}
*/

ssize_t KernelClient::get_nlattr_data(nlattr *attr, char **str)
{
	*str = (char *) NLA_DATA(attr);

	return attr->nla_len - NLA_HDRLEN;
}

ssize_t KernelClient::get_nlattr_data(nlattr *attr, QByteArray *str)
{
	str->setRawData((const char *)NLA_DATA(attr), attr->nla_len);

	return attr->nla_len - NLA_HDRLEN;
}

ssize_t KernelClient::get_nlattr_data(nlattr *attr, __u8 **i)
{
	*i = (__u8 *) NLA_DATA(attr);

	return attr->nla_len - NLA_HDRLEN;
}

ssize_t KernelClient::get_nlattr_data(nlattr *attr, __u16 **i)
{
	*i = (__u16 *) NLA_DATA(attr);

	return attr->nla_len - NLA_HDRLEN;
}

ssize_t KernelClient::get_nlattr_data(nlattr *attr, __u32 **i)
{
	*i = (__u32 *) NLA_DATA(attr);

	return attr->nla_len - NLA_HDRLEN;
};


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

/*
void KernelClient::nl_attr_put(nlmsghdr *nlh, uint16_t type, size_t len, const void *data)
{
	struct nlattr *attr = (struct nlattr *)((char *)nlh + NLA_ALIGN(nlh->nlmsg_len));
	uint16_t payload_len = NLA_HDRLEN + len;
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
*/

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
	nl_address.nl_groups = DISPENSER_GENL_MCGROUP_MASK; //nl group mask

	if (bind(nl_fd, (struct sockaddr *)&nl_address, sizeof(nl_address)) < 0) {
		//close(nl_fd);
		//qDebug() << "Error binding socket";
		qDaemonLog(QStringLiteral("Error binding socket. Couldn't start the server. bind(nl_fd, (struct sockaddr *)&nl_address, sizeof(nl_address) failed"), QDaemonLog::ErrorEntry);
		qApp->quit();
		return -1;
	}

	m_pKernel = new QSocketNotifier(nl_fd, QSocketNotifier::Read, this);

	/** Done internally by KernelStream: **/
	//mOutBuffer.open(QIODevice::ReadWrite);
	//mInBuffer.open(QIODevice::ReadWrite);
	//mToKernel.setDevice(&mOutBuffer);
	//FromKernel.setDevice(&mInBuffer);

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
	//QBuffer msg;
	//QDataStream toKernel(&msg);
	//struct genl_info info;
	//struct nlattr *attrs[__DISPENSER_GENL_ATTR_MAX];
	//info.attrs = attrs;
	int flags = 0;

	//Make sure we block until we have family id!
	flags = fcntl(nl_fd, F_GETFL);
	if (flags & O_NONBLOCK) {
		qDaemonLog(QStringLiteral("FD switch to blocking state."), QDaemonLog::NoticeEntry);
		fcntl(nl_fd, F_SETFL, flags & (~O_NONBLOCK));
	}

	nlmsghdr *nlmsg = nl_hdr_put(&mToKernel);
	if (!nlmsg) {
		//error
		qDaemonLog(QStringLiteral("Error netlink message header"), QDaemonLog::ErrorEntry);
		qApp->quit();
		return -1;
	}

	nlmsg->nlmsg_type = GENL_ID_CTRL;
	// NLM_F_REQUEST is REQUIRED for kernel requests, otherwise the packet is rejected!
	// Kernel reference: https://elixir.bootlin.com/linux/v5.10.16/source/net/netlink/af_netlink.c#L2487
	nlmsg->nlmsg_flags = NLM_F_REQUEST;
	nlmsg->nlmsg_seq = 0;
	nlmsg->nlmsg_pid = getpid();

	// Populate the payload's "family header" : which in our case is genlmsghdr

	genlmsghdr *genlmsg = genl_hdr_put(&mToKernel, CTRL_CMD_GETFAMILY);
	if (!genlmsg) {
		//error
		qDaemonLog(QStringLiteral("Error genetlink message header"), QDaemonLog::ErrorEntry);
		qApp->quit();
		return -1;
	}
	//genlmsg->version = 1;

	//nl_request_msg.n.nlmsg_type = GENL_ID_CTRL;
	// NLM_F_REQUEST is REQUIRED for kernel requests, otherwise the packet is rejected!
	// Kernel reference: https://elixir.bootlin.com/linux/v5.10.16/source/net/netlink/af_netlink.c#L2487
	//nl_request_msg.n.nlmsg_flags = NLM_F_REQUEST;
	//nl_request_msg.n.nlmsg_seq = 0;
	//nl_request_msg.n.nlmsg_pid = getpid();
	//nl_request_msg.n.nlmsg_len = NLMSG_LENGTH(GENL_HDRLEN);
	// Populate the payload's "family header" : which in our case is genlmsghdr
	//nl_request_msg.g.cmd = CTRL_CMD_GETFAMILY;
	//nl_request_msg.g.version = 1;
	// Populate the payload's "netlink attributes"

	QByteArray name = DISPENSER_GENL_NAME;
	name.truncate(GENL_NAMSIZ - 1); //Family name length can be upto 16 chars including \0
	nlattr* name_attr = nl_attr_put_str(&mToKernel, CTRL_ATTR_FAMILY_NAME, &name);
	Q_UNUSED(name_attr);

	//nl_na = (struct nlattr *)GENLMSG_DATA(&nl_request_msg);
	//nl_na->nla_type = CTRL_ATTR_FAMILY_NAME;

	//QByteArray name(DISPENSER_GENL_NAME);
	//name.truncate(15);

	//nl_na->nla_len = name.length() + 1 + NLA_HDRLEN;
	//qstrncpy((char *)NLA_DATA(nl_na), DISPENSER_GENL_NAME, 15); //Family name length can be upto 16 chars including \0

	//nl_request_msg.n.nlmsg_len += NLMSG_ALIGN(nl_na->nla_len);
	//NLMSG_ALIGNTO = 4U

	// tell the socket (nl_address) that we use NETLINK address family and that we target
	// the kernel (pid = 0)

	nl_rxtx_length = sendToKernel(&mToKernel);
	// Send the family ID request message to the netlink controller
	//nl_rxtx_length = sendto(nl_fd, (char *)&nl_request_msg, nl_request_msg.n.nlmsg_len,
	//                        0, (struct sockaddr *)&nl_address, sizeof(nl_address));

	//if ((__u32)nl_rxtx_length != nl_request_msg.n.nlmsg_len) {
	if ((__u32)nl_rxtx_length != mToKernel.uSize()) {
		::close(nl_fd);
		qDaemonLog(QStringLiteral("Error sending family id request"), QDaemonLog::ErrorEntry);
		qApp->quit();
		return -1;
	}

	// Wait for the response message
	//nl_rxtx_length = recv(nl_fd, &nl_response_msg, sizeof(nl_response_msg), 0);
	nl_rxtx_length = recvFromKernel();

	if (nl_fd < 0) {
		qDaemonLog(QStringLiteral("Error receiving family id request"), QDaemonLog::ErrorEntry);
		qApp->quit();
		return -1;
	}

	/** Succeeded! Initialize data structures. Fetch data from kernel. **/
	connect(&m_cUnit, &UnitItem::nightChanged, this, &KernelClient::setUnitStatus, Qt::QueuedConnection);
	connect(&m_cUnit, &UnitItem::colsChanged, this, &KernelClient::connectCols, Qt::QueuedConnection);
	connect(&m_cUnit, &UnitItem::initialized, this, &KernelClient::moduleAndDaemonInitialized, Qt::QueuedConnection);

	//request unit status
	getUnitStatus();

	return 0;

	/*
	if (nl_rxtx_length < 0) {
		qDaemonLog(QStringLiteral("Error receiving family id request result"), QDaemonLog::ErrorEntry);
		qApp->quit();
		return -1;
	}
	*/

	// Validate response message
	/*
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
	*/
	//process_event(&mFromKernel);

	/*
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
*/

	//m_pKernel = new QSocketNotifier(nl_fd, QSocketNotifier::Read, this);
	//connect(m_pKernel, SIGNAL(activated(int)), this, SLOT(readyRead()));
	//m_pKernel->setEnabled(true);

	//connect(m_pKernel, QOverload<QSocketDescriptor, QSocketNotifier::Type>::of(&QSocketNotifier::activated),
	//    [=](QSocketDescriptor socket, QSocketNotifier::Type Read){ /* ... */ });

	//return 0;
}

KernelStream &KernelClient::initRequest(KernelStream *buffer, enum DISPENSER_GENL_COMMAND cmd)
{
	nlmsghdr *nlmsg = nl_hdr_put(buffer);
	if (!nlmsg) {
		//error
		qDaemonLog(QStringLiteral("Error netlink message header"), QDaemonLog::ErrorEntry);
		return *buffer;
	}

	//Dispenser Kernel Client message
	nlmsg->nlmsg_type = nl_family_id;
	// NLM_F_REQUEST is REQUIRED for kernel requests, otherwise the packet is rejected!
	// Kernel reference: https://elixir.bootlin.com/linux/v5.10.16/source/net/netlink/af_netlink.c#L2487
	nlmsg->nlmsg_flags = NLM_F_REQUEST;
	nlmsg->nlmsg_seq = 0;
	nlmsg->nlmsg_pid = getpid();

	// Populate the payload's "family header" : which in our case is genlmsghdr

	genlmsghdr *genlmsg = genl_hdr_put(buffer, cmd);
	if (!genlmsg) {
		//error
		qDaemonLog(QStringLiteral("Error genetlink message header"), QDaemonLog::ErrorEntry);
		return *buffer;
	}

	return *buffer;
}

void KernelClient::getUnitStatus()
{
	KernelStream toKernel;
	initRequest(&toKernel, DISPENSER_GENL_CMD_UNIT_STATUS);
	sendToKernel(&toKernel);
}

void KernelClient::getColStatus(ColItem *col)
{
	KernelStream toKernel;

	if (!col)
		return;

	initRequest(&toKernel, DISPENSER_GENL_CMD_COL_STATUS);
	nl_attr_put(&toKernel, DISPENSER_GENL_COL_NUM, col->getId());
	sendToKernel(&toKernel);
}

void KernelClient::getSlotStatus(SlotItem *slot)
{
	KernelStream toKernel;

	if (!slot)
		return;

	initRequest(&toKernel, DISPENSER_GENL_CMD_SLOT_STATUS);
	nl_attr_put(&toKernel, DISPENSER_GENL_COL_NUM, slot->getCol()->getId());
	nl_attr_put(&toKernel, DISPENSER_GENL_SLOT_NUM, slot->getId());
	sendToKernel(&toKernel);
}

void KernelClient::setUnitStatus()
{
	KernelStream toKernel;
	__u8 status = dispenser_pack_unit_status(m_cUnit.getUnitStatus());

	initRequest(&toKernel, DISPENSER_GENL_CMD_UNIT_STATUS);
	nl_attr_put(&toKernel, DISPENSER_GENL_UNIT_STATUS, status);
	nl_attr_put(&toKernel, DISPENSER_GENL_INITIALIZED, m_cUnit.daemonInitialized());
	sendToKernel(&toKernel);
}

//Nothing settable at the moment...
void KernelClient::setColStatus(ColItem *col)
{
	KernelStream toKernel;

	if (!col)
		return;

	initRequest(&toKernel, DISPENSER_GENL_CMD_COL_STATUS);
	nl_attr_put(&toKernel, DISPENSER_GENL_COL_NUM, col->getId());
	sendToKernel(&toKernel);
}

void KernelClient::setSlotStatus(SlotItem *slot)
{
	KernelStream toKernel;
	__u8 status;

	if (!slot)
		return;

	status = dispenser_pack_slot_status(slot->getSlotStatus(), slot->getFull());

	initRequest(&toKernel, DISPENSER_GENL_CMD_SLOT_STATUS);
	nl_attr_put(&toKernel, DISPENSER_GENL_COL_NUM, slot->getCol()->getId());
	nl_attr_put(&toKernel, DISPENSER_GENL_SLOT_NUM, slot->getId());
	nl_attr_put(&toKernel, DISPENSER_GENL_SLOT_STATUS, status);
	nl_attr_put(&toKernel, DISPENSER_GENL_SLOT_FAILED_UP, slot->getFailedUp());
	nl_attr_put(&toKernel, DISPENSER_GENL_SLOT_FAILED_DOWN, slot->getFailedDown());
	sendToKernel(&toKernel);
}

void KernelClient::connectCols(UnitItem *unit)
{
	ColItem *col = nullptr;
	int count = unit->numCols();

	for (int i = 0; i < count; ++i) {
		col = unit->col(i);
		if (col) {
			connect(col, &ColItem::slotCountChanged, this, &KernelClient::connectSlots, Qt::QueuedConnection);

			getColStatus(col);
		}
	}
}

void KernelClient::connectSlots(ColItem *col)
{
	SlotItem *slot = nullptr;
	int count;

	if (!col)
		return;

	count = col->getSlotCount();

	for (int i = 0; i < count; ++i) {
		slot = col->slot(i);
		if (slot) {
			//connect(slot, &SlotItem::idChanged, this, &KernelClient::getSlotStatus);

			if (m_cUnit.moduleInitialized())
				setSlotStatus(slot);
			else
				getSlotStatus(slot);
		}
	}
}

void KernelClient::moduleAndDaemonInitialized(UnitItem *unit)
{
	//Module and Daemon has been Initialized.


	//Connect signals to websocketserver!

	Q_UNUSED(unit);
	Q_UNUSED(m_pServer);

	//loadAlarms();
}

void KernelClient::setLight(int on)
{
	m_cUnit.setLight(on);

	setUnitStatus();
}

void KernelClient::setNight(int on)
{
	m_cUnit.setNight(on);

	setUnitStatus();
}

//Not implemented
/*
void KernelClient::setRelease(Alarm *alarm)
{
	KernelStream toKernel;

	if (!alarm)
		return;

	//initRequest(&toKernel, DISPENSER_GENL_CMD_SET_ALARM);
	if (alarm->getCol())
		nl_attr_put(&toKernel, DISPENSER_GENL_COL_NUM, alarm->getCol()->getId());
	//nl_attr_put(&toKernel, DISPENSER_GENL_ALARM_SECONDS, alarm->getSeconds());
	//nl_attr_put(&toKernel, DISPENSER_GENL_ALARM_DAYS, alarm->getDays());
	//sendToKernel(&toKernel);
}
*/

void KernelClient::release(__s8 col, __s8 slot, bool force, __u8 count)
{
	KernelStream toKernel;
	if (count > 0 && force)
		count = -count;

	initRequest(&toKernel, DISPENSER_GENL_CMD_RELEASE);
	nl_attr_put(&toKernel, DISPENSER_GENL_RELEASE_COUNT, count); //Defaults to one. Not needed to be set.

	if (col >= 0) {
		nl_attr_put(&toKernel, DISPENSER_GENL_COL_NUM, col);
		if (slot >= 0)
			nl_attr_put(&toKernel, DISPENSER_GENL_SLOT_NUM, slot);
	}

	sendToKernel(&toKernel);
}

void KernelClient::release(SlotItem *slot, bool force)
{
	if (slot)
		return release(slot->getCol()->getId(), slot->getId(), force);
}

void KernelClient::release(ColItem *col, bool force, __u8 count)
{
	if (col)
		return release(col->getId(), -1, force, count);
}

void KernelClient::releaseUnit(UnitItem *unit)
{
	Q_UNUSED(unit);

	release();
}

void KernelClient::releaseCol(ColItem *col)
{
	release(col, false, 1);
}



/*
int KernelClient::get_unit_status()
{
	/ ** Send Message to the Kernel requesting unit status * /
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
	//nl_na->nla_type = DISPENSER_GENL_CMD_UNSPEC;GNL_FOOBAR_XMPL_A_MSG
	//nl_na->nla_len = sizeof(MESSAGE_TO_KERNEL) + NLA_HDRLEN; // Message length
	//memcpy(NLA_DATA(nl_na), MESSAGE_TO_KERNEL, sizeof(MESSAGE_TO_KERNEL));
	nl_request_msg.n.nlmsg_len += NLMSG_ALIGN(nl_na->nla_len);

	return 0;
}
*/

KernelStream::KernelStream()
{
	m_cBuffer.open(QIODevice::ReadWrite);
	setDevice(&m_cBuffer);
}

KernelStream &KernelStream::operator<<(nlmsghdr &s)
{
	writeRawData((char *)&s, NLMSG_HDRLEN);

	return this->align();
}

template <typename T>
KernelStream &KernelStream::operator<<(T s)
{
	writeRawData((char *)&s, sizeof(T));

	return this->align();
}

void *KernelStream::cur()
{
	QBuffer *buf = dynamic_cast<QBuffer *>(this->device());
	if (!buf) {
		return nullptr;
	}

	qint64 pos = buf->pos();
	void *data = &(buf->buffer().data()[pos]);

	return data;
}

const void *KernelStream::constCur()
{
	QBuffer *buf = dynamic_cast<QBuffer *>(this->device());
	if (!buf) {
		return nullptr;
	}

	qint64 pos = buf->pos();
	const void *data = &(buf->buffer().constData()[pos]);

	return data;
}

KernelStream &KernelStream::operator>>(const nlmsghdr **s)
{
	*s = (const nlmsghdr *)constCur();

	skipRawData(NLMSG_HDRLEN);

	return *this;
}

KernelStream &KernelStream::operator>>(const genlmsghdr **s)
{
	*s = (const genlmsghdr *)constCur();

	skipRawData(GENL_HDRLEN);

	return *this;
}

KernelStream &KernelStream::operator>>(const nlattr **s)
{
	*s = (const nlattr *)constCur();

	skipRawData(NLA_HDRLEN);

	return *this;
}

KernelStream &KernelStream::align()
{
	QBuffer *buf = dynamic_cast<QBuffer *>(this->device());
	if (!buf) {
		return *this;
	}

	qsizetype pad = NLMSG_PAD(buf->size());
	QByteArray padding(pad, 0);

	if (pad)
		writeRawData(padding, pad);

	return *this;
}

KernelStream &KernelStream::alignAttr()
{
	QBuffer *buf = dynamic_cast<QBuffer *>(this->device());
	if (!buf) {
		return *this;
	}

	qsizetype pad = NLA_PAD(buf->size());
	QByteArray padding(pad, 0);

	if (pad)
		writeRawData(padding, pad);

	return *this;
}

int *KernelStream::pSize()
{
	QBuffer *buf = dynamic_cast<QBuffer *>(this->device());
	if (!buf) {
		return nullptr;
	}

	QByteArray *data = &buf->buffer();
	return &data->data_ptr()->size;
}

qint64 KernelStream::pos()
{
	QBuffer *buf = dynamic_cast<QBuffer *>(this->device());
	if (!buf) {
		return -1;
	}

	return buf->pos();
}

KernelStream &KernelStream::operator<<(genlmsghdr &s)
{
	writeRawData((char *)&s, GENL_HDRLEN);

	return this->align();
}

KernelStream &KernelStream::operator<<(nlattr &s)
{
	writeRawData((char *)&s, NLA_HDRLEN);

	return this->alignAttr();
}

// QByteArray must be zero terminated!
KernelStream &KernelStream::operator<<(const QByteArray *s)
{
	writeRawData(s->constData(), s->size());
	writeRawData("", 1);

	return this->alignAttr();
}

// x/48xb 0x555559fc48 0x555559fbf8
// x/48cb 0x555559fc48

/*
template<typename T>
KernelStreamIterator<T>::KernelStreamIterator(KernelStream *stream)
{
	QBuffer *buf = dynamic_cast<QBuffer *>(stream->device());
	//Wrong type of stream if nullptr returned.
	if (!buf) {
		p_mStream = nullptr;
		return;
	}

	KernelStreamIterator(stream, buf->size());
	//KernelStreamIterator(stream, buf->buffer().size());
}
*/

template<typename T>
KernelStreamIterator<T>::KernelStreamIterator(KernelStream *stream, qsizetype pos)
{
	QBuffer *buf = dynamic_cast<QBuffer *>(stream->device());
	qsizetype size = 0;

	//Wrong type of stream if nullptr returned.
	if (!buf) {
		p_mStream = nullptr;
		return;
	}

	size = buf->size();
	p_mStream = stream;
	mPos = pos;

	if (mPos < 0 || mPos > size)
		mPos = size;
}

template<typename T>
KernelStreamIterator<T>::~KernelStreamIterator()
{

}

template<typename T>
T *KernelStreamIterator<T>::data()
{
	if (!p_mStream) {
		return nullptr;
	}

	QBuffer *buf = dynamic_cast<QBuffer *>(p_mStream->device());
	if (!buf) {
		p_mStream = nullptr;
		return nullptr;
	}

	if (buf->size() < mPos) {
		return nullptr;
	}

	return (T *)&(buf->buffer().data()[mPos]);
}
