#!/system/bin/sh

	chmod 4777 /vendor/usr/superuser/root_enable/su
	chmod 777 /system/xbin
	chmod 4777 /system/xbin/su
	cp /vendor/usr/superuser/root_enable/su /system/xbin/su
	cp /vendor/usr/superuser/root_enable/su /system/xbin/daemonsu
	chmod 4777 /system/xbin/su
	chmod 4777 /system/xbin/daemonsu
/system/xbin/su --daemon

if [ -f /system/xbin/daemonsu ]; then
	stop superuser
	echo "superuser" > /dev/console
fi
