#!/system/bin/sh
#su

    mv /system/priv-app/Launcher3/Launcher3.apk /system/priv-app/Launcher3/Launcher3.apk.bak
		echo "122222222" > /dev/console

if [ -f /system/priv-app/Launcher3/Launcher3.apk.bak ]; then
	stop nohomeapk
	echo "333333" > /dev/console
fi