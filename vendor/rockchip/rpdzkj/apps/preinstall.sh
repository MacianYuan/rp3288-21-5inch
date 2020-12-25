#!/system/bin/sh

ERR=0
if [ -d /vendor/usr/preinstall ];then
#       /system/xbin/su
        chmod -R 777 /vendor/usr/preinstall
        for file in /vendor/usr/preinstall/*.apk
        do
                /system/bin/pm install -r $file
                if [ $? -eq 0 ]; then
#                       rm -rf $file
                        echo "/vendor/usr/preinstall.sh pm install $file" > /dev/console
                else
                        ERR=1
                        echo "/vendor/usr/preinstall.sh pm install $file failed : $?" > /dev/console
                fi
        done
#       rm -rf /system/usr/preinstall

        if [ $ERR -eq 0 ]; then
                mv /vendor/bin/preinstall.sh /vendor/usr/preinstall/
        fi
fi