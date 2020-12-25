#!/system/bin/sh

	echo "nosuperuser 00" > /dev/console
	chmod 4777 /vendor/usr/superuser/root_disable/su
	chmod 777 /system/xbin
	chmod 4777 /system/xbin/su
	cp /vendor/usr/superuser/root_disable/su /system/xbin/su
	mv  /system/xbin/daemonsu /system/xbin/nodaemonsu
	chmod 4777 /system/xbin/su
	echo "nosuperuser 11" > /dev/console
	
	
if [ -f /system/xbin/nodaemonsu ]; then
	rm /system/xbin/nodaemonsu
	stop nosuperuser
	echo "nosuperuser 22" > /dev/console
fi
