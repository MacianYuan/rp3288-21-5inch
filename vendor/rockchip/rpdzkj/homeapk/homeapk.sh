#!/system/bin/sh

#su
    mv /system/priv-app/Launcher3/Launcher3.apk.bak /system/priv-app/Launcher3/Launcher3.apk
    
    if [ -f /system/priv-app/Launcher3/Launcher3.apk ]; then
	stop homeapk
	echo "333333" > /dev/console
fi
