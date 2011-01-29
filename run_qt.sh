#!/bin/sh
# Script to filter the input devices to suit Qt env vars

QTE_APP=$1

# grep devices
TMP=`grep -E 'Name|Handlers' /proc/bus/input/devices`


# Get all mouse event inputs
MOUSE=`echo $TMP | awk '
BEGIN{RS="N: Name="; FS="[NH]: ";}
	$2 ~ /mouse/ {
		ev = $2
		# Do something a bit different for touchscreens
		if ($1 ~ /Touchscreen/) {
			sub("Handlers=mouse[0-9]+ event", "", ev)
			text = sprintf("%s tslib:/dev/input/event%d",text,ev)
		}
		else {
			sub("Handlers=mouse", "", ev)
			text = sprintf("%s IntelliMouse:/dev/input/mouse%d",text,ev)
		}
	}
END{sub(" ", "", text); print text}
'`

export QWS_MOUSE_PROTO="$MOUSE"
#echo 'QWS_MOUSE_PROTO='$QWS_MOUSE_PROTO


# Get all keyboard event inputs
KEYBOARD=`echo $TMP | awk '
BEGIN{RS="N: Name="; FS="[NH]: ";}
	$2 ~ /kbd/ {
		ev = $2
		sub("Handlers=kbd event", "", ev)
		# Ignore the keypad
		if ($1 !~ /sh_keysc/) {
			text = sprintf("%s Usb:/dev/input/event%d",text,ev)
		}
	}
END{sub(" ", "", text); print text}
'`

export QWS_KEYBOARD="$KEYBOARD"
#echo 'QWS_KEYBOARD='$QWS_KEYBOARD

# Qt 4.7 requires QWS_DISPLAY to be set
export QWS_DISPLAY="LinuxFb:/dev/fb0"
#echo 'QWS_DISPLAY='$QWS_DISPLAY

# options: "de", "en", ...
export LANG="en"

$QTE_APP -qws

