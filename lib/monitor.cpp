#include "monitor.h"

#include <QTime>
#include <QDir>
#include <QTemporaryFile>
#include <QProcess>

#include <QDaemonLog>
#include <SmtpMime>

Monitor::Monitor(UnitItem *unit)
        : QObject(unit)
{
	ReEncoder *encoder;
	m_pUnit = unit;

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
	connect(this, &Monitor::sendMessage, encoder, &ReEncoder::doSend);
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

bool Monitor::generatingMessage() const
{
	return m_cSendTimer.isActive();
}

QStringList Monitor::getLog()
{
	QStringList log;

	log = m_cLog;
	m_cLog.clear();

	return log;
}

Monitor::sendmask Monitor::getEvents()
{
	enum sendmask events;

	events = m_iEvents;
	m_iEvents = NONE;

	return events;
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
	connect(col, &ColItem::alarmsChanged, this, &Monitor::startColReleaseTimer);
}

void Monitor::releaseSlot(SlotItem *slot)
{
	*this << tr("%1:\tSlot %2/%3 release. State is %4, lock is %5, up sensor is %6, down sensor is %7.")
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
	*this << tr("%1:\tSlot %2/%3 state change -> %4. Lock is %5, up sensor is %6, down sensor is %7.")
	                .arg(QTime::currentTime().toString())
	                .arg(QString::number(slot->getCol()->getId()))
	                .arg(QString::number(slot->getId()))
	                .arg(slot->getStateStr())
	                .arg(slot->getSlotStatus()->release ? tr("released") : tr("locked"))
	                .arg(QString::number(slot->getSlotStatus()->up))
	                .arg(QString::number(slot->getSlotStatus()->down));

	add(STATE);

	if (slot->getCol()->getUnit()->isEmpty()) {
		*this << tr("%1:\tDispenser is now EMPTY!")
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
	                .arg(QString::number(col->getId()));

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

	m_bEncoding = true;
	emit reencode();

	*this << tr("%1:\tDispenser Start Recording for release.")
	                .arg(QTime::currentTime().toString());

	qDaemonLog(QStringLiteral("Release about to happen -> start record"), QDaemonLog::NoticeEntry);

	startReleaseTimer(m_pUnit);
}

void Monitor::encoderReady()
{
	//m_cSendTimer.start(m_iSendTime);
	qDaemonLog(QStringLiteral("ENCODE ready!"), QDaemonLog::NoticeEntry);

	if (m_cSendTimer.isActive()) {
		qDaemonLog(QStringLiteral("Send timer active after ENCODE ready. Waiting..."), QDaemonLog::NoticeEntry);
		return;
	}

	m_cInhibitTimer.stop();
	send();
}

void Monitor::sendMail()
{
	qDaemonLog(QStringLiteral("Send timer expired."), QDaemonLog::NoticeEntry);

	if (m_bEncoding) {
		qDaemonLog(QStringLiteral("Send timer: encoder running -> return."), QDaemonLog::NoticeEntry);
		return;
	}

	m_cInhibitTimer.stop();

	send();
}

void Monitor::forceSend()
{
	qDaemonLog(QStringLiteral("Inhibit timer expired!"), QDaemonLog::NoticeEntry);

	//reencodeThread.terminate();
	//m_bEncoding = false;
	//reencodeThread.start();

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

void Monitor::startColReleaseTimer(ColItem *col)
{
	if (!col) {
		return;
	}

	long msec = col->getNextRelease(m_iReleaseLeadTime);

	m_cReleaseTimer.start(msec - m_iReleaseLeadTime);

	qDaemonLog(QStringLiteral("Next release in %1 from col = %2. Wake up in %3")
	           .arg(QString::number(msec))
	           .arg(QString::number(col->getId()))
	           .arg(QString::number(msec - m_iReleaseLeadTime)), QDaemonLog::NoticeEntry);
}

void Monitor::send()
{
	emit sendMessage();

	return;
}

void ReEncoder::syncAddresses()
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
	m_cLogMutex.lock();
	m_cLog << text;
	m_cLogMutex.unlock();

	m_cSendTimer.start(m_iSendTime);
	if (!m_cInhibitTimer.isActive())
		m_cInhibitTimer.start(m_iInhibitTime);

	return *this;
}


ReEncoder::ReEncoder(Monitor *monitor)
//        : QObject(monitor)
{
	m_pMonitor = monitor;

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
		m_cAddresses[0].email = m_cSettings.value("DestAddresses", "zonite@icloud.com").toString();
		m_cAddresses[0].mask = (enum sendmask) m_cSettings.value("SendMasks", ALL).toInt();
	}

	m_cMailServer = m_cSettings.value("MailServer", "mail.nykyri.eu").toString();
	m_cMailSender = m_cSettings.value("MailSender", "dispenser@nykyri.eu").toString();
}

ReEncoder::~ReEncoder()
{
	syncAddresses();

	m_cSettings.sync();
}

void ReEncoder::doReEncode()
{
	if (!m_pMonitor) {
		emit done(-1);

		qDaemonLog(QStringLiteral("ReEncoder no monitor!"), QDaemonLog::ErrorEntry);
		return;
	}

	qDaemonLog(QStringLiteral("Record."), QDaemonLog::NoticeEntry);

	QProcess encode;
	QString startSh = m_pMonitor->getStartRec();
	QString stopSh = m_pMonitor->getStopRec();
	QStringList args;

	encode.start(stopSh, args);
	encode.waitForFinished(2000); //recored control timeout
	encode.readAll();

	encode.start(startSh, args);
	encode.waitForFinished(2000); //recored control timeout
	encode.readAll();

	QThread::sleep(m_pMonitor->getDuration()); //sleep the wanted duration and sleep

	encode.start(stopSh, args);
	encode.waitForFinished(2000); //recored control timeout
	encode.readAll();

	QString prog = m_pMonitor->getReencode();
	args.clear();
	args << m_pMonitor->getRecLocation();

	encode.start(prog, args); // Video will be in /tmp/send.mov
	encode.waitForFinished(30000); //reencode timeout.
	encode.readAll();

	qDaemonLog(QStringLiteral("Record done."), QDaemonLog::NoticeEntry);

	emit done(0);
}

void ReEncoder::doSend()
{
	if (m_pMonitor) {
		if (m_pMonitor->generatingMessage()) {
			qDaemonLog(QStringLiteral("ReEncored::doSend: Monitor timer is still running."), QDaemonLog::NoticeEntry);
			return;
		}
	} else {
		qDaemonLog(QStringLiteral("ReEncoder::doSend: no Monitor."), QDaemonLog::NoticeEntry);
		return;
	}

	QStringList m_cLog, cLines;
	enum sendmask m_iEvents;

	m_pMonitor->lockLog();
	m_cLog = m_pMonitor->getLog();
	m_iEvents = (ReEncoder::sendmask) m_pMonitor->getEvents();
	m_pMonitor->unlockLog();

	if (m_cLog.isEmpty()) {
		qDaemonLog(QStringLiteral("Empty Log. Cancel send and return."), QDaemonLog::NoticeEntry);
		return;
	}

	//Progress with message!
	SmtpClient smtp(m_cMailServer, 25, SmtpClient::TcpConnection);

	MimeMessage message;
	message.setSender(EmailAddress(m_cMailSender, tr("Dispenser Daemon")));

	for (struct address &rcpt : m_cAddresses) {
		if (rcpt.mask & m_iEvents) {
			message.addRecipient(rcpt.email);
			qDaemonLog(QStringLiteral("Sending to: %1").arg(rcpt.email), QDaemonLog::NoticeEntry);
		}
	}

	if (m_iEvents & RELEASE) {
		message.setSubject(tr("Dispenser Release Event"));
	} else if (m_iEvents & CHARGING) {
		message.setSubject(tr("Dispenser Charging Changed"));
	} else {
		message.setSubject(tr("Dispenser Event"));
	}

	cLines << tr("Event log:")
	      << QStringLiteral("");
	cLines << m_cLog;

	cLines << QStringLiteral("")
	      << QStringLiteral("")
	      << tr("Unit Status:");

	cLines << m_pMonitor->getUnit()->toStatusStr()
	      << QStringLiteral("");

	cLines << tr("End of report");

	cLines << QStringLiteral("")
	      << QStringLiteral("")
	      << tr("Live video http://dispenser.nykyri.eu:8080/player.html");

	QString content;

	for (const QString &line : cLines) {
		content += line;
		content += "\n";
	}

	MimeText text;
	text.setText(content);
	message.addPart(&text);

	QFile video("/tmp/send.mov");
	video.rename(QDir::tempPath() + QStringLiteral("/release-") + QDateTime::currentDateTime().toString("yyyy-MM-dd_HH.mm") + QStringLiteral(".mov"));
	MimeAttachment attachment(&video);
	if (video.exists()) {
		attachment.setContentType("video/quicktime");
		message.addPart(&attachment);
	}

	smtp.connectToHost();
	if (!smtp.waitForReadyConnected()) {
		qDebug() << "Failed to connect to host!";
		return;
	}

	/*
	smtp.login("zonite", "test");
	if (!smtp.waitForAuthenticated()) {
		qDebug() << "Failed to login!";
		return;
	}
	*/

	smtp.sendMail(message);
	if (!smtp.waitForMailSent()) {
		qDebug() << "Failed to send mail!";
		return;
	}

	smtp.quit();

	video.remove();
}

/*
QFile video(QStringLiteral("/tmp/send.mov"));
//QString message = QDir::tempPath() + QStringLiteral("/message");

qDaemonLog(QStringLiteral("Send report. Video exists %1").arg(video.exists()), QDaemonLog::NoticeEntry);

if (m_iEvents & REENCODE && video.exists()) {
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


	//QFile video(QStringLiteral("/tmp/send.mov"));
	video.rename(QDir::tempPath() + QStringLiteral("/send-") + QDateTime::currentDateTime().toString("yyyy-MM-dd_HH.mm") + QStringLiteral(".mov"));
	args << QStringLiteral("Dispenser Release Event")
	     << body.fileName()
	     << video.fileName()
	     << "/tmp/dispenser.msg";
	//     << message.fileName();

	qDaemonLog(QStringLiteral("MPack video."), QDaemonLog::NoticeEntry);

	pack.start(m_cReportScript, args);
	pack.waitForFinished();
	pack.readAll();
} else {
	QFile message("/tmp/dispenser.msg");
	message.open(QIODevice::WriteOnly | QIODevice::Text);

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
	message.close();

	qDaemonLog(QStringLiteral("Email generated"), QDaemonLog::NoticeEntry);
}
QFile message("/tmp/dispenser.msg");

QProcess send;
QString prog = m_cSendScript;
QStringList args;

m_iEvents = NONE;

void ReEncode::generateMessage(QStringList &lines)
{
}
*/
