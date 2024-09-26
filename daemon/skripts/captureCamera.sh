#!/usr/bin/bash

#-colorspace 1 -color_primaries 1 -color_trc 1 -bsf:v h264_metadata=video_full_range_flag=0

#-movflags +write_colr

#echo "app ${1} stream ${2}"

ffmpeg -fflags +genpts -thread_queue_size 1024 -ar 48000 -acodec pcm_s16le -f alsa -ac 2 -channel_layout 2.1 -i hw:1,0 -thread_queue_size 1024 -f v4l2 -codec:v h264_v4l2m2m -video_size 1920x1080  -colorspace 1 -color_primaries 1 -color_trc 1  -i /dev/video0 -movflags +write_colr -vf "drawtext=fontfile=/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf: text='%{localtime\:%d.%m.%Y\ %H\\\\\:%M\\\\\:%S}': x=(w-tw)/2: y=h-(4*lh): fontcolor=white: box=1: boxcolor=0x00000000@1: boxborderw=2: fontsize=30" -use_wallclock_as_timestamps 1 -write_tmcd on -c:v h264_v4l2m2m -acodec aac -ab 192k -strict -2 -b:v 6000k -f flv "rtmp://127.0.0.1/${1}/${2}" 2> /dev/null > /dev/null
