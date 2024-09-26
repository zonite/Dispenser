#ifndef MONITOR_H
#define MONITOR_H

#include <QObject>
#include <QSettings>
#include <QThread>
#include <QTimer>

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



class ReEncoder : public QObject
{
	Q_OBJECT

public slots:
	void doReEncode();

signals:
	void done(int result);

private:
	QTimer m_cEncodeTimer;
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

	struct address {
		QString email = nullptr;
		enum sendmask mask = NONE;
	};


	explicit Monitor(QObject *parent = nullptr);
	~Monitor();

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

private slots:
	void aboutToRelease();
	void encoderReady();
	void sendMail();
	void forceSend();

private:
	void send();
	void syncAddresses();
	inline void add(enum sendmask event) { m_iEvents = (enum sendmask) (m_iEvents | event); }

	Monitor &operator<<(QString text);

	QStringList m_cLog;
	enum sendmask m_iEvents = NONE;

	QSettings m_cSettings;
	QVector<struct address> m_cAddresses;

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
