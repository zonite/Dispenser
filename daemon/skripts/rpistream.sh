#!/usr/bin/bash


#ffmpeg -re -f video4linux2 -i /dev/video0 -vcodec libx264 -vprofile baseline -acodec aac -strict -2 -f flv rtmp://localhost/show/stream

#ffmpeg -ar 48000 -acodec pcm_s16le -f alsa -ac 2 -channel_layout 2.1 -i hw:1,0 -f v4l2 -codec:v h264_v4l2m2m -framerate 30 -video_size 1920x1080 -i /dev/video0 -vf "drawtext=fontfile=/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf: text='%{localtime\:%d.%m.%Y\ %H\\\\\:%M\\\\\:%S}': x=(w-tw)/2: y=h-(2*lh): fontcolor=white: box=1: boxcolor=0x00000000@1: fontsize=30" -c:v h264_v4l2m2m -f mpegts -b:v 3000k -crf 15 /tmp/test.mpg

#capture for rpi:
ffmpeg -fflags +genpts -thread_queue_size 32 -ar 48000 -acodec pcm_s16le -f alsa -ac 2 -channel_layout 2.1 -i hw:1,0 -thread_queue_size 32 -f video4linux2 -codec:v h264 -i /dev/video0 -use_wallclock_as_timestamps 1  -write_tmcd on -c:v copy -acodec aac -ab 192k -strict -2 -f flv rtmp://127.0.0.1/restream/dispenser128
#capture for rpi with timecode:
ffmpeg -fflags +genpts -thread_queue_size 32 -ar 48000 -acodec pcm_s16le -f alsa -ac 2 -channel_layout 2.1 -i hw:1,0 -thread_queue_size 32 -f video4linux2 -codec:v h264_v4l2m2m -i /dev/video0 -vf "drawtext=fontfile=/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf: text='%{localtime\:%d.%m.%Y\ %H\\\\\:%M\\\\\:%S}': x=(w-tw)/2: y=h-(4*lh): fontcolor=white: box=1: boxcolor=0x00000000@1: boxborderw=2: fontsize=30" -use_wallclock_as_timestamps 1 -write_tmcd on -c:v h264_v4l2m2m -acodec aac -ab 192k -strict -2 -b:v 6000k -f flv rtmp://127.0.0.1/restream/dispenser128
ffmpeg -fflags +genpts -thread_queue_size 1024 -ar 48000 -acodec pcm_s16le -f alsa -ac 2 -channel_layout 2.1 -i hw:1,0 -thread_queue_size 1024 -f video4linux2 -codec:v h264_v4l2m2m -i /dev/video0 -vf "drawtext=fontfile=/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf: text='%{localtime\:%d.%m.%Y\ %H\\\\\:%M\\\\\:%S}': x=(w-tw)/2: y=h-(4*lh): fontcolor=white: box=1: boxcolor=0x00000000@1: boxborderw=2: fontsize=30" -use_wallclock_as_timestamps 1 -write_tmcd on -c:v h264_v4l2m2m -acodec aac -ab 192k -strict -2 -b:v 6000k -f flv rtmp://127.0.0.1/restream/dispenser128

#re-encode for email
date ; ffmpeg -c:v h264_v4l2m2m -i rtmp://127.0.0.1/restream/dispenser128 -vf "drawtext=fontfile=/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf: text='%{localtime\:%d.%m.%Y\ %H\\\\\:%M\\\\\:%S}': x=(w-tw)/2: y=h-(2*lh): fontcolor=white: box=1: boxcolor=0x00000000@1: boxborderw=2: fontsize=30" -c:v h264_v4l2m2m -f mp4 -b:v 3000k -b:a 192k -y -crf 15 -t 16 /tmp/test.mp4
#re-encode for email (timestamp already in)
date ; ffmpeg -c:v h264 -i rtmp://127.0.0.1/restream/dispenser128 -c:v libx264 -f mp4 -b:v 3000k -b:a 192k -y -crf 15 -t 16 /tmp/test.mp4

ffmpeg -f dshow -i video="screen-capture-recorder":audio="Stereo Mix (IDT High Definition" \
-vcodec libx264 -preset ultrafast -tune zerolatency -r 10 -async 1 -acodec libmp3lame -ab 24k -ar 22050 -bsf:v h264_mp4toannexb \
-maxrate 750k -bufsize 3000k -f mpegts udp://192.168.5.215:48550

If you want to choose specific streams then use the select option. In this next example the video stream is split and scaled to two different sized outputs, and the audio is encoded only once but used by both outputs. Otherwise you would have to unnecessarily re-encode the same audio multiple times. The ignore value for the onfail option in the last output will keep the other outputs running even if that last output fails. The default value for onfail is abort.

ffmpeg -i input -filter_complex \
"[0:v]split=2[s0][s1]; \
 [s0]scale=1280:-2[v0]; \
 [s1]scale=640:-2[v1]" \
-map "[v0]" -map "[v1]" -map 0:a -c:v libx264 -c:a aac -f tee \
"[select=\'v:0,a\']local0.mkv| \
 [select=\'v:0,a\':f=flv]rtmp://server0/app/instance/playpath| \
 [select=\'v:1,a\']local1.mkv| \
 [select=\'v:1,a\':f=flv:onfail=ignore]rtmp://server1/app/instance/playpath"

#ffmpeg -fflags +genpts -f lavfi -i color=s=1920x1080:color=0xff0000 -an -s 1920x1080 -r 30 -vf "drawtext=fontfile=/usr/share/fonts/dejavu/DejaVuSans.ttf: text='%{localtime\:%d.%m.%Y\ %H\\\\\:%M\\\\\:%S}': x=(w-tw)/2: y=h-(4*lh): fontcolor=white: box=1: boxcolor=0x00000000@1: boxborderw=2: fontsize=30" -use_wallclock_as_timestamps 1 -write_tmcd on -t 15 -f flv /tmp/test.flv

#vcgencmd measure_temp
#v4l2-ctl --device=/dev/video0 --set-fmt-video=width=1920,height=1080
#v4l2-ctl --set-ctrl video_bitrate=300000
