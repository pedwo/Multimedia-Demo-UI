#!/bin/sh

FILE=$1
EXT=${FILE##*.}

if [ $EXT == "avi" ]; then
	DEMUX=avidemux
elif [ $EXT == "mp4" ] || [ $EXT == "mov" ]; then
	DEMUX=qtdemux
else
	echo "File extension not supported"
	exit
fi

gst-launch \
	filesrc location=$1 ! $DEMUX name=demux \
		demux.audio_00 ! queue max-size-time=2000000000 ! decodebin ! audioconvert ! audioresample ! autoaudiosink \
		demux.video_00 ! queue ! gst-sh-mobile-dec ! gst-sh-mobile-sink


