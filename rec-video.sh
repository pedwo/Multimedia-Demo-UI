#!/bin/sh

FILE=$1
EXT=${FILE##*.}

if [ $EXT == "avi" ]; then

	# Audio (raw) & video (H.264)
	gst-launch \
		alsasrc ! audio/x-raw-int,rate=44100,channels=2 \
		! queue ! audioconvert \
		! queue ! mux. \
		gst-sh-mobile-camera-enc cntl_file=ctl/h264-video0-vga-stream.ctl preview=1 \
		! video/x-h264,width=640,height=480,framerate=24/1 \
		! queue ! mux. \
		avimux name=mux ! filesink location=$1

else
	echo "File extension not supported"
fi


