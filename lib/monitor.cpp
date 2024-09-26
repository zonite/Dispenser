#include "monitor.h"

#include <QTime>
#include <QDir>
#include <QTemporaryFile>
#include <QProcess>

#include <QDaemonLog>

Monitor::Monitor(UnitItem *unit)
        : QObject(unit)
{
	ReEncoder *encoder;
	m_pUnit = unit;

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

	m_cReportScript = m_cSettings.value("ReportScript", "/usr/bin/createReport.sh").toString();
	m_cSendScript = m_cSettings.value("SendScript", "/usr/bin/sendReport.sh").toString();
	m_cReencodeScript = m_cSettings.value("ReencodeScript", "/usr/bin/reencode.sh").toString();
	m_cStartRecScript = m_cSettings.value("StartRecScript", "/usr/bin/startRec.sh").toString();
	m_cStopRecScript = m_cSettings.value("StopRecScript", "/usr/bin/stopRec.sh").toString();
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

	encoder = new ReEncoder(this);
	encoder->moveToThread(&reencodeThread);

	//connections!
	connect(this, &Monitor::reencode, encoder, &ReEncoder::doReEncode);
	connect(encoder, &ReEncoder::done, this, &Monitor::encoderReady);

	connect(&m_cReleaseTimer, &QTimer::timeout, this, &Monitor::aboutToRelease);
	connect(&m_cSendTimer, &QTimer::timeout, this, &Monitor::sendMail);
	connect(&m_cInhibitTimer, &QTimer::timeout, this, &Monitor::forceSend);

	connect(m_pUnit, &UnitItem::newCol, this, &Monitor::newCol);
	connect(m_pUnit, &UnitItem::alarmsChanged, this, &Monitor::startReleaseTimer);
	connect(m_pUnit, &UnitItem::releaseEvent, this, &Monitor::unitReleaseEvent);
	connect(m_pUnit, &UnitItem::chargingChanged, this, &Monitor::chargingChanged);

	reencodeThread.start();
	startReleaseTimer(m_pUnit);
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
	connect(col, &ColItem::alarmsChanged, this, &Monitor::startReleaseTimer);
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
	startReleaseTimer(m_pUnit);
}

void Monitor::colReleaseEvent(ColItem *col)
{
	*this << tr("%1:\tDispenser release from column %2!")
	                .arg(QTime::currentTime().toString())
	                .arg(col->getId());

	add(RELEASE);
	startReleaseTimer(m_pUnit);
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

	*this << tr("%1:\tDispenser Start Recording for release.")
	                .arg(QTime::currentTime().toString());

	qDaemonLog(QStringLiteral("Release about to happen -> start record"), QDaemonLog::NoticeEntry);

	startReleaseTimer(m_pUnit);
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

void Monitor::forceSend()
{
	reencodeThread.terminate();
	reencodeThread.start();

	m_cSendTimer.stop();

	send();
}

void Monitor::startReleaseTimer(UnitItem *unit)
{
	if (unit != m_pUnit) {
		if (unit)
			m_pUnit = unit;
	}

	long msec = m_pUnit->getNextRelease(m_iReleaseLeadTime);

	m_cReleaseTimer.start(msec - m_iReleaseLeadTime);

	qDaemonLog(QStringLiteral("Next release in %1. Wake up in %2")
	           .arg(QString::number(msec))
	           .arg(QString::number(msec - m_iReleaseLeadTime)), QDaemonLog::NoticeEntry);
}

void Monitor::send()
{
	QTemporaryFile message;
	//QString message = QDir::tempPath() + QStringLiteral("/message");
	message.open();

	if (m_iEvents & REENCODE) {
		QTemporaryFile body;
		QProcess pack;
		QStringList args;

		if (body.open()) {
			QStringList content;
			QTextStream out(&body);

			generateMessage(content);

			for(QString &line : content) {
				out << line << QStringLiteral("\n");
			}
			out.flush();
			body.flush();
		}
		body.fileName();


		QFile video(QStringLiteral("/tmp/send.mov"));
		video.rename(QStringLiteral("send-") + QDateTime::currentDateTime().toString("YYYY-MM-DD_HH.mm") + QStringLiteral(".mov"));
		args << QStringLiteral("Dispenser Release Event")
		     << (QDir::tempPath() + body.fileName())
		     << video.fileName()
		     << QDir::tempPath() + message.fileName();

		pack.start(m_cReportScript, args);
	} else {
		QStringList content;
		content << QStringLiteral("From: dispenser@nykyri.eu")
		     << QStringLiteral("To: Minna Rakas <minna_mj@hotmail.com")
		     << QStringLiteral("Subject: Dispenser Release Event")
		     << QStringLiteral("");

		generateMessage(content);

		QTextStream out(&message);
		for(QString &line : content) {
			out << line << QStringLiteral("\n");
		}
		out.flush();
	}
	message.flush();

	QProcess send;
	QString prog = m_cSendScript;
	QStringList args;

	for (struct address &rcpt : m_cAddresses) {
		if (rcpt.mask & m_iEvents) {
			QStringList args;
			args << QStringLiteral("dispenser@nykyri.eu")
			     << rcpt.email
			     << QDir::tempPath() + message.fileName();

			send.start(prog, args);
			send.waitForFinished();
			send.readAll();
		}
	}
	m_iEvents = NONE;
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

void Monitor::generateMessage(QStringList &lines)
{
	lines << tr("Event log:")
	      << QStringLiteral("");
	lines << m_cLog;

	lines << QStringLiteral("")
	      << QStringLiteral("")
	      << tr("Unit Status:");

	m_cLog.clear();

	lines << m_pUnit->toStatusStr()
	      << QStringLiteral("");

	lines << tr("End of raport");

	lines << QStringLiteral("")
	      << QStringLiteral("")
	      << tr("Live video http://nfs.nykyri.eu:8080/player.html");
}

Monitor &Monitor::operator<<(QString text)
{
	m_cLog << text;

	m_cSendTimer.start(m_iSendTime);
	if (!m_cInhibitTimer.isActive())
		m_cInhibitTimer.start(m_iInhibitTime);

	return *this;
}


ReEncoder::ReEncoder(Monitor *monitor)
//        : QObject(monitor)
{
	m_pMonitor = monitor;
}

void ReEncoder::doReEncode()
{
	if (!m_pMonitor) {
		emit done(-1);

		return;
	}

	QProcess encode;
	QString prog = m_pMonitor->getStartRec();
	QStringList args;

	encode.start(prog, args);
	encode.waitForFinished();
	encode.readAll();

	QThread::sleep(m_pMonitor->getDuration()); //sleep the wanted duration and sleep

	prog = m_pMonitor->getStopRec();

	encode.start(prog, args);
	encode.waitForFinished();
	encode.readAll();

	prog = m_pMonitor->getReencode();
	args.clear();
	args << m_pMonitor->getRecLocation();

	encode.start(prog, args);
	encode.waitForFinished();
	encode.readAll();

	emit done(0);
}
