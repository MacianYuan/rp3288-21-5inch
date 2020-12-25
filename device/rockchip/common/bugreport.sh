#!/system/bin/sh

timestamp=`date +'%Y-%m-%d-%H-%M-%S'`
storagePath="$EXTERNAL_STORAGE/bugreports"
bugreport=$storagePath/bugreport-$timestamp
screenshotPath="$EXTERNAL_STORAGE/bugreports"
screenshot=$screenshotPath/screenshot-$timestamp.png

# check screen shot folder
if [ ! -e $screenshotPath ]; then
  mkdir -p $screenshotPath
fi

# take screen shot
# we run this as a bg job in case screencap is stuck
/system/bin/screencap -p $screenshot &

# run bugreport
/system/bin/dumpstate -o $bugreport $@

# make files readable
chown root.sdcard_rw $bugreport.txt
chown root.sdcard_rw $screenshot
