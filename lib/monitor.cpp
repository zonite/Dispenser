#include "monitor.h"

#include <QTime>
#include <QTemporaryFile>
#include <QProcess>

Monitor::Monitor(QObject *parent)
        : QObject{parent}
{
	ReEncoder *encoder;

	m_cSettings.value("DestAddresses");

	if (m_cSettings.value("DestAddresses").canConvert(QMetaType::QStringList)) {
		QList<QVariant> addresses = m_cSettings.value("DestAddresses").toList();
		QList<QVariant> masks = m_cSettings.value("SendMasks").toList();

		for (int i = 0; i < addresses.size(); ++i) {
			struct address newAddress;

			newAddress.email = addresses.at(i).toString();
			if (masks.size() < i)
				newAddress.mask = (enum sendmask) masks.at(i).toUInt();
			else
				newAddress.mask = ALL;

			m_cAddresses.append(newAddress);
		}
	} else  {
		m_cAddresses.resize(1);
		m_cAddresses[0].email = m_cSettings.value("DestAddresses", "zonite@nykyri.eu").toString();
		m_cAddresses[0].mask = (enum sendmask) m_cSettings.value("SendMasks", ALL).toInt();
	}

	m_cReportScript = m_cSettings.value("ReportScript", "createReport.sh").toString();
	m_cSendScript = m_cSettings.value("SendScript", "sendReport.sh").toString();
	m_cReencodeScript = m_cSettings.value("ReencodeScript", "reencode.sh").toString();
	m_cStartRecScript = m_cSettings.value("StartRecScript", "startRec.sh").toString();
	m_cStopRecScript = m_cSettings.value("StopRecScript", "stopRec.sh").toString();
	m_cRTMPappName = m_cSettings.value("RTMPappName", "reencode").toString();
	m_cRTMPstreamName = m_cSettings.value("RTMPstreamName", "dispenser128").toString();
	m_cRTMPrecLocation = m_cSettings.value("RTMPrecLocation", "/var/www/rec/").toString();
	m_iClipDuration = m_cSettings.value("ClipDuration", 15).toUInt();
	m_iInhibitTime = m_cSettings.value("SendEmailTimeout", 60000).toUInt(); //sendmessage timeout
	m_iReleaseLeadTime = m_cSettings.value("PreRecordTime", 4000).toUInt(); //Time before release to start recording
	m_iSendTime = m_cSettings.value("SendEmailLag", 10000).toUInt(); //lag after last update before sending mail

	m_cReleaseTimer.setTimerType(Qt::VeryCoarseTimer);
	m_cSendTimer.setTimerType(Qt::VeryCoarseTimer);
	m_cInhibitTimer.setTimerType(Qt::VeryCoarseTimer);

	m_cReleaseTimer.setSingleShot(true);
	m_cSendTimer.setSingleShot(true);
	m_cInhibitTimer.setSingleShot(true);

	encoder = new ReEncoder;
	encoder->moveToThread(&reencodeThread);

	//connections!
	connect(this, &Monitor::reencode, encoder, &ReEncoder::doReEncode);
	connect(encoder, &ReEncoder::done, this, &Monitor::encoderReady);

	connect(&m_cReleaseTimer, &QTimer::timeout, this, &Monitor::aboutToRelease);
	connect(&m_cSendTimer, &QTimer::timeout, this, &Monitor::sendMail);
	connect(&m_cInhibitTimer, &QTimer::timeout, this, &Monitor::forceSend);

	reencodeThread.start();
}

Monitor::~Monitor()
{
	reencodeThread.quit();

	syncAddresses();

	m_cSettings.setValue("ReportScript", m_cReportScript);
	m_cSettings.setValue("SendScript", m_cSendScript);
	m_cSettings.setValue("ReencodeScript", m_cReencodeScript);
	m_cSettings.setValue("StartRecScript", m_cStartRecScript);
	m_cSettings.setValue("StopRecScript", m_cStopRecScript);
	m_cSettings.setValue("RTMPappName", m_cRTMPappName);
	m_cSettings.setValue("RTMPstreamName", m_cRTMPstreamName);
	m_cSettings.setValue("RTMPrecLocation",m_cRTMPrecLocation);
	m_cSettings.setValue("ClipDuration", m_iClipDuration);
	m_cSettings.setValue("SendEmailTimeout", m_iInhibitTime); //sendmessage timeout
	m_cSettings.setValue("PreRecordTime", m_iReleaseLeadTime); //Time before release to start recording
	m_cSettings.setValue("SendEmailLag", m_iSendTime); //lag after last update before sending mail

	m_cSettings.sync();

	reencodeThread.wait();
}

void Monitor::newSlot(SlotItem *slot)
{
	connect(slot, &SlotItem::releaseChanged, this, &Monitor::releaseSlot);
	connect(slot, &SlotItem::stateChanged, this, &Monitor::stateChanged);
	connect(slot, &SlotItem::fullChanged, this, &Monitor::fullChanged);
}

void Monitor::newCol(ColItem *col)
{
	connect(col, &ColItem::newSlot, this, &Monitor::newSlot);
	connect(col, &ColItem::releaseEvent, this, &Monitor::colReleaseEvent);
}

void Monitor::releaseSlot(SlotItem *slot)
{
	*this << tr("%1\tSlot %2/%3 release. State is %4, lock is %5, up sensor is %6, down sensor is %7.")
	                .arg(QTime::currentTime().toString())
	                .arg(slot->getCol()->getId())
	                .arg(slot->getId())
	                .arg(slot->getStateStr())
	                .arg(slot->getSlotStatus()->release ? tr("released") : tr("locked"))
	                .arg(slot->getSlotStatus()->up)
	                .arg(slot->getSlotStatus()->down);

}

void Monitor::stateChanged(SlotItem *slot)
{
	*this << tr("%1\tSlot %2/%3 state change -> %4. Lock is %5, up sensor is %6, down sensor is %7.")
	                .arg(QTime::currentTime().toString())
	                .arg(slot->getCol()->getId())
	                .arg(slot->getId())
	                .arg(slot->getStateStr())
	                .arg(slot->getSlotStatus()->release ? tr("released") : tr("locked"))
	                .arg(slot->getSlotStatus()->up)
	                .arg(slot->getSlotStatus()->down);

	add(STATE);

	if (slot->getCol()->getUnit()->isEmpty()) {
		*this << tr("%1\tDispenser is now EMPTY!")
		                .arg(QTime::currentTime().toString());
		add(EMPTY);
	}
}

void Monitor::fullChanged(SlotItem *slot)
{
	if (slot->getCol()->getUnit()->isFull()) {
		*this << tr("%1:\tDispenser is now filled!")
		                .arg(QTime::currentTime().toString());

		add(FILLED);
	}
}

void Monitor::unitReleaseEvent(UnitItem *unit)
{
	Q_UNUSED(unit);

	*this << tr("%1:\tDispenser release!")
	                .arg(QTime::currentTime().toString());

	add(RELEASE);
}

void Monitor::colReleaseEvent(ColItem *col)
{
	*this << tr("%1:\tDispenser release from column %2!")
	                .arg(QTime::currentTime().toString())
	                .arg(col->getId());

	add(RELEASE);
}

void Monitor::chargingChanged(UnitItem *unit)
{
	if (unit->isCharging()) {
		*this << tr("%1:\tDispenser power restored!")
		                .arg(QTime::currentTime().toString());
	} else {
		*this << tr("%1:\tDispenser batteries are discharging!")
		                .arg(QTime::currentTime().toString());
	}
	add(CHARGING);
}

void Monitor::aboutToRelease()
{
	add(REENCODE);

	emit reencode();
}

void Monitor::encoderReady()
{
	m_cSendTimer.start(m_iSendTime);
}

void Monitor::sendMail()
{
	if (reencodeThread.isRunning())
		return;

	send();
}

void Monitor::send()
{
	QTemporaryFile message;

	if (m_iEvents & REENCODE) {
		QTemporaryFile body;
		QProcess *pack = new QProcess(this);

		if (body.open()) {
			//fill content
		}
		body.fileName();

		QFile video;
	} else {

	}

}

void Monitor::syncAddresses()
{
	QVariantList addresses;
	QVariantList masks;

	for (int i = 0; i < m_cAddresses.size(); ++i) {
		addresses.append(QVariant(m_cAddresses.at(i).email));
		masks.append(m_cAddresses.at(i).mask);
	}

	m_cSettings.setValue("DestAddresses", QVariant::fromValue(addresses));
	m_cSettings.setValue("SendMasks", QVariant::fromValue(masks));
}

Monitor &Monitor::operator<<(QString text)
{
	m_cLog << text;

	m_cSendTimer.start(m_iSendTime);
	if (!m_cInhibitTimer.isActive())
		m_cInhibitTimer.start(m_iInhibitTime);

	return *this;
}
