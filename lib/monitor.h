#ifndef MONITOR_H
#define MONITOR_H

#include <QObject>
#include <QSettings>
#include <QThread>
#include <QTimer>
#include <QMutex>

#include "lib_global.h"

#include "unititem.h"
#include "colitem.h"
#include "slotitem.h"

//ffmpeg -ar 48000 -acodec pcm_s16le -f alsa -ac 2 -channel_layout 2.1 -i hw:1,0 -f v4l2 -codec:v h264_v4l2m2m -framerate 30 -video_size 1920x1080 -i /dev/video0 -c:v copy -f mpegts /tmp/test.mpg
//ffmpeg -start_at_zero -copyts -ss 00:00:14.435 -i input.mp4
//    -vf "drawtext=fontfile=/path/to/Arial.ttf:
//          fontsize=45:fontcolor=yellow:box=1:boxcolor=black:x=(W-tw)/2:y=H-th-10:
//          text='Time\: %{pts\:hms}'"
//    -vframes 1 output.png
//  ffmpeg -ar 48000 -acodec pcm_s16le -f alsa -ac 2 -channel_layout 2.1 -i hw:1,0 -f v4l2 -codec:v h264_v4l2m2m -framerate 30 -video_size 1920x1080 -i /dev/video0 -vf "drawtext=fontfile=/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf: text='%{localtime\:%d.%m.%Y\ %H\\\\\:%M\\\\\:%S}': x=(w-tw)/2: y=h-(2*lh): fontcolor=white: box=1: boxcolor=0x00000000@1: fontsize=30" -c:v h264_v4l2m2m -f mpegts -b:v 3000k -crf 15 /tmp/test.mpg
//text='timestamp \: %{pts\:gmtime\:0\:%H\\\:%M\\\:%S}'

//ffmpeg -ar 48000 -acodec pcm_s16le -f alsa -ac 2 -channel_layout 2.1 -i hw:1,0 -f v4l2 -codec:v h264_v4l2m2m -framerate 30 -video_size 1920x1080 -i /dev/video0 -vf "drawtext=fontfile=/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf: text='%{localtime\:%d.%m.%Y\ %H\\\\\:%M\\\\\:%S}': x=(w-tw)/2: y=h-(2*lh): fontcolor=white: box=1: boxcolor=0x00000000@1: boxborderw=2: fontsize=30" -c:v h264_v4l2m2m -f mpegts -b:v 3000k -crf 15 /tmp/test.mpg

//Monitor on
///opt/vc/bin/tvservice -p
//Monitor off
///opt/vc/bin/tvservice -o

class Monitor;

class ReEncoder : public QObject
{
	Q_OBJECT

public:
	enum sendmask {
		NONE = 0x0,
		RELEASE = 0x1,
		CHARGING = 0x2,
		FILLED = 0x4,
		STATE = 0x8,
		EMPTY = 0x10,
		REENCODE = 0x20,
		ALL = 0x3F,
	};

	struct address {
		QString email = nullptr;
		enum sendmask mask = NONE;
	};

	ReEncoder(Monitor *monitor);
	~ReEncoder();

public slots:
	void doReEncode();
	void doSend();

signals:
	void done(int result);

private:
	void syncAddresses();

	QTimer m_cEncodeTimer;
	Monitor *m_pMonitor = nullptr;
	QSettings m_cSettings;

	QString m_cMailServer;
	QString m_cMailSender;

	QVector<struct address> m_cAddresses;
};




class Monitor : public QObject
{
	Q_OBJECT
public:
	enum sendmask {
		NONE = 0x0,
		RELEASE = 0x1,
		CHARGING = 0x2,
		FILLED = 0x4,
		STATE = 0x8,
		EMPTY = 0x10,
		REENCODE = 0x20,
		ALL = 0x3F,
	};


	explicit Monitor(UnitItem *unit);
	~Monitor();

	const QString getStartRec() const { return m_cStartRecScript;}
	const QString getStopRec() const { return m_cStopRecScript;}
	const QString getReencode() const { return m_cReencodeScript;}
	unsigned long getDuration() const { return m_iClipDuration; }
	const QString getRecLocation() const { return m_cRTMPrecLocation + m_cRTMPstreamName + ".flv"; }
	const UnitItem *getUnit() const { return m_pUnit; }
	bool generatingMessage() const;

	QStringList getLog();
	enum sendmask getEvents();

	void lockLog() { m_cLogMutex.lock(); }
	void unlockLog() { m_cLogMutex.unlock(); }

public slots:

	void newSlot(SlotItem *slot);
	void newCol(ColItem *col);
	void releaseSlot(SlotItem *slot);
	void stateChanged(SlotItem *slot);
	void fullChanged(SlotItem *slot);
	void unitReleaseEvent(UnitItem *unit);
	void colReleaseEvent(ColItem *col);
	void chargingChanged(UnitItem *unit);

signals:
	void reencode();
	void sendMessage();

private slots:
	void aboutToRelease();
	void encoderReady();
	void sendMail();
	void forceSend();
	void startReleaseTimer(UnitItem *unit);

private:
	void send();
	inline void add(enum sendmask event) { m_iEvents = (enum sendmask) (m_iEvents | event); }
	void generateMessage(QStringList &lines);

	Monitor &operator<<(QString text);

	QStringList m_cLog;
	enum sendmask m_iEvents = NONE;
	QMutex m_cLogMutex;

	QSettings m_cSettings;
	UnitItem *m_pUnit = nullptr;

	bool m_bEncoding = false;
	QThread reencodeThread;
	QTimer m_cReleaseTimer; //Activates X seconds before release
	QTimer m_cSendTimer;
	QTimer m_cInhibitTimer;

	QString m_cReportScript;
	QString m_cSendScript;
	QString m_cReencodeScript;
	QString m_cStartRecScript;
	QString m_cStopRecScript;
	QString m_cRTMPappName;
	QString m_cRTMPstreamName;
	QString m_cRTMPrecLocation;
	quint32 m_iClipDuration;
	quint32 m_iInhibitTime; //sendmessage timeout
	quint32 m_iReleaseLeadTime; //Time before release to start recording
	quint32 m_iSendTime; //lag after last update before sending mail
};


#endif // MONITOR_H
