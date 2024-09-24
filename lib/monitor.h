#ifndef MONITOR_H
#define MONITOR_H

#include <QObject>


//ffmpeg -ar 48000 -acodec pcm_s16le -f alsa -ac 2 -channel_layout 2.1 -i hw:1,0 -f v4l2 -codec:v h264_v4l2m2m -framerate 30 -video_size 1920x1080 -i /dev/video0 -c:v copy -f mpegts /tmp/test.mpg
//ffmpeg -start_at_zero -copyts -ss 00:00:14.435 -i input.mp4
//    -vf "drawtext=fontfile=/path/to/Arial.ttf:
//          fontsize=45:fontcolor=yellow:box=1:boxcolor=black:x=(W-tw)/2:y=H-th-10:
//          text='Time\: %{pts\:hms}'"
//    -vframes 1 output.png
//  ffmpeg -ar 48000 -acodec pcm_s16le -f alsa -ac 2 -channel_layout 2.1 -i hw:1,0 -f v4l2 -codec:v h264_v4l2m2m -framerate 30 -video_size 1920x1080 -i /dev/video0 -vf "drawtext=fontfile=/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf: text='%{localtime\:%d.%m.%Y\ %H\\\\\:%M\\\\\:%S}': x=(w-tw)/2: y=h-(2*lh): fontcolor=white: box=1: boxcolor=0x00000000@1: fontsize=30" -c:v h264_v4l2m2m -f mpegts -b:v 3000k -crf 15 /tmp/test.mpg
//text='timestamp \: %{pts\:gmtime\:0\:%H\\\:%M\\\:%S}'

//ffmpeg -ar 48000 -acodec pcm_s16le -f alsa -ac 2 -channel_layout 2.1 -i hw:1,0 -f v4l2 -codec:v h264_v4l2m2m -framerate 30 -video_size 1920x1080 -i /dev/video0 -vf "drawtext=fontfile=/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf: text='%{localtime\:%d.%m.%Y\ %H\\\\\:%M\\\\\:%S}': x=(w-tw)/2: y=h-(2*lh): fontcolor=white: box=1: boxcolor=0x00000000@1: boxborderw=2: fontsize=30" -c:v h264_v4l2m2m -f mpegts -b:v 3000k -crf 15 /tmp/test.mpg

class Monitor : public QObject
{
	Q_OBJECT
public:
	explicit Monitor(QObject *parent = nullptr);

signals:

};

#endif // MONITOR_H
