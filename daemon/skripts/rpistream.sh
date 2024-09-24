#!/usr/bin/bash


#ffmpeg -re -f video4linux2 -i /dev/video0 -vcodec libx264 -vprofile baseline -acodec aac -strict -2 -f flv rtmp://localhost/show/stream

#ffmpeg -ar 48000 -acodec pcm_s16le -f alsa -ac 2 -channel_layout 2.1 -i hw:1,0 -f v4l2 -codec:v h264_v4l2m2m -framerate 30 -video_size 1920x1080 -i /dev/video0 -vf "drawtext=fontfile=/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf: text='%{localtime\:%d.%m.%Y\ %H\\\\\:%M\\\\\:%S}': x=(w-tw)/2: y=h-(2*lh): fontcolor=white: box=1: boxcolor=0x00000000@1: fontsize=30" -c:v h264_v4l2m2m -f mpegts -b:v 3000k -crf 15 /tmp/test.mpg

#capture for rpi:
ffmpeg -fflags +genpts -thread_queue_size 32 -ar 48000 -acodec pcm_s16le -f alsa -ac 2 -channel_layout 2.1 -i hw:1,0 -thread_queue_size 32 -f video4linux2 -codec:v h264 -i /dev/video0 -use_wallclock_as_timestamps 1  -write_tmcd on -c:v copy -acodec aac -ab 192k -strict -2 -f flv rtmp://127.0.0.1/restream/dispenser128
#capture for rpi with timecode:
ffmpeg -fflags +genpts -thread_queue_size 32 -ar 48000 -acodec pcm_s16le -f alsa -ac 2 -channel_layout 2.1 -i hw:1,0 -thread_queue_size 32 -f video4linux2 -codec:v h264_v4l2m2m -i /dev/video0 -vf "drawtext=fontfile=/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf: text='%{localtime\:%d.%m.%Y\ %H\\\\\:%M\\\\\:%S}': x=(w-tw)/2: y=h-(4*lh): fontcolor=white: box=1: boxcolor=0x00000000@1: boxborderw=2: fontsize=30" -use_wallclock_as_timestamps 1 -write_tmcd on -c:v h264_v4l2m2m -acodec aac -ab 192k -strict -2 -b:v 6000k -f flv rtmp://127.0.0.1/restream/dispenser128

#re-encode for email
date ; ffmpeg -c:v h264_v4l2m2m -i rtmp://127.0.0.1/restream/dispenser128 -vf "drawtext=fontfile=/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf: text='%{localtime\:%d.%m.%Y\ %H\\\\\:%M\\\\\:%S}': x=(w-tw)/2: y=h-(2*lh): fontcolor=white: box=1: boxcolor=0x00000000@1: boxborderw=2: fontsize=30" -c:v h264_v4l2m2m -f mp4 -b:v 3000k -b:a 192k -y -crf 15 -t 16 /tmp/test.mp4
#re-encode for email (timestamp already in)
date ; ffmpeg -c:v h264 -i rtmp://127.0.0.1/restream/dispenser128 -c:v libx264 -f mp4 -b:v 3000k -b:a 192k -y -crf 15 -t 16 /tmp/test.mp4
