#!/usr/bin/bash

#echo "src ${1} dst ${2} duration ${3}"

#reencode
#ffmpeg -i /var/www/streaming/rec/dispenser128-1727213365.flv -profile:v high -movflags +write_colr -crf 20 -c:v h264 -t 15 -y -colorspace 1 -color_primaries 1 -color_trc 1 -bsf:v h264_metadata=video_full_range_flag=0 -b:a 192k /tmp/test.mov
#copy
#ffmpeg -i /var/www/streaming/rec/dispenser128-1727213365.flv -movflags +write_colr -crf 20 -c:v copy -t 15 -y -colorspace 1 -color_primaries 1 -color_trc 1 -bsf:v h264_metadata=video_full_range_flag=0 -b:a 192k /tmp/reencoded.mov


#ffmpeg -i /var/www/rec/dispenser128.flv -movflags +write_colr -c:v copy -c:a copy -t 15 -y -colorspace 1 -color_primaries 1 -color_trc 1 /tmp/send.mov
ffmpeg -i "${1}" -movflags +write_colr -c:v copy -c:a copy -t 15 -y -r 30 -colorspace 1 -color_primaries 1 -color_trc 1 -bsf:v h264_metadata=video_full_range_flag=1 /tmp/send.mov
