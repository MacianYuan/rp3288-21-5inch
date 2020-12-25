#!/bin/bash
set -e

. build/envsetup.sh >/dev/null && setpaths

export PATH=$ANDROID_BUILD_PATHS:$PATH
TARGET_PRODUCT=`get_build_var TARGET_PRODUCT`
TARGET_HARDWARE=`get_build_var TARGET_BOARD_HARDWARE`
TARGET_BOARD_PLATFORM=`get_build_var TARGET_BOARD_PLATFORM`
TARGET_DEVICE_DIR=`get_build_var TARGET_DEVICE_DIR`
PLATFORM_VERSION=`get_build_var PLATFORM_VERSION`
PLATFORM_SECURITY_PATCH=`get_build_var PLATFORM_SECURITY_PATCH`
TARGET_BUILD_VARIANT=`get_build_var TARGET_BUILD_VARIANT`
BOARD_SYSTEMIMAGE_PARTITION_SIZE=`get_build_var BOARD_SYSTEMIMAGE_PARTITION_SIZE`
BOARD_USE_SPARSE_SYSTEM_IMAGE=`get_build_var BOARD_USE_SPARSE_SYSTEM_IMAGE`
TARGET_ARCH=`get_build_var TARGET_ARCH`
TARGET_OUT_VENDOR=`get_build_var TARGET_OUT_VENDOR`
TARGET_BASE_PARAMETER_IMAGE=`get_build_var TARGET_BASE_PARAMETER_IMAGE`
HIGH_RELIABLE_RECOVERY_OTA=`get_build_var HIGH_RELIABLE_RECOVERY_OTA`
BOARD_USES_AB_IMAGE=`get_build_var BOARD_USES_AB_IMAGE`
BOARD_USES_RECOVERY_AS_BOOT=`get_build_var BOARD_USES_RECOVERY_AS_BOOT`
echo TARGET_BOARD_PLATFORM=$TARGET_BOARD_PLATFORM
echo TARGET_PRODUCT=$TARGET_PRODUCT
echo TARGET_HARDWARE=$TARGET_HARDWARE
echo TARGET_BUILD_VARIANT=$TARGET_BUILD_VARIANT
echo BOARD_SYSTEMIMAGE_PARTITION_SIZE=$BOARD_SYSTEMIMAGE_PARTITION_SIZE
echo BOARD_USE_SPARSE_SYSTEM_IMAGE=$BOARD_USE_SPARSE_SYSTEM_IMAGE
echo TARGET_BASE_PARAMETER_IMAGE==$TARGET_BASE_PARAMETER_IMAGE
echo HIGH_RELIABLE_RECOVERY_OTA=$HIGH_RELIABLE_RECOVERY_OTA
echo BOARD_USES_AB_IMAGE=$BOARD_USES_AB_IMAGE
echo BOARD_USES_RECOVERY_AS_BOOT=$BOARD_USES_RECOVERY_AS_BOOT
TARGET="withoutkernel"
if [ "$1"x != ""x  ]; then
         TARGET=$1
fi

IMAGE_PATH=rockdev/Image-$TARGET_PRODUCT
UBOOT_PATH=u-boot
KERNEL_PATH=kernel
KERNEL_CONFIG=$KERNEL_PATH/.config
rm -rf $IMAGE_PATH
mkdir -p $IMAGE_PATH

FSTYPE=ext4
echo system filesysystem is $FSTYPE

BOARD_CONFIG=device/rockchip/common/device.mk

PARAMETER=${TARGET_DEVICE_DIR}/parameter_ab.txt

KERNEL_SRC_PATH=`grep TARGET_PREBUILT_KERNEL ${BOARD_CONFIG} |grep "^\s*TARGET_PREBUILT_KERNEL *:= *[\w]*\s" |awk  '{print $3}'`

[ $(id -u) -eq 0 ] || FAKEROOT=fakeroot

BOOT_OTA="ota"

[ $TARGET != $BOOT_OTA -a $TARGET != "withoutkernel" ] && echo "unknow target[${TARGET}],exit!" && exit 0

    if [ ! -f $OUT/kernel ]
    then
	    echo "kernel image not fount![$OUT/kernel] "
        read -p "copy kernel from TARGET_PREBUILT_KERNEL[$KERNEL_SRC_PATH] (y/n) n to exit?"
        if [ "$REPLY" == "y" ]
        then
            [ -f $KERNEL_SRC_PATH ]  || \
                echo -n "fatal! TARGET_PREBUILT_KERNEL not eixit! " || \
                echo -n "check you configuration in [${BOARD_CONFIG}] " || exit 0

            cp ${KERNEL_SRC_PATH} $OUT/kernel

        else
            exit 0
        fi
    fi


	echo -n "create misc.img.... "
	cp -a rkst/Image/misc.img $IMAGE_PATH/misc.img
	cp -a rkst/Image/pcba_small_misc.img $IMAGE_PATH/pcba_small_misc.img
	cp -a rkst/Image/pcba_whole_misc.img $IMAGE_PATH/pcba_whole_misc.img
	echo "done."

if [ $TARGET == $BOOT_OTA ]
then
	echo -n "create system.img boot.img bootloader.img tos.img oem.img vendor.img..."
	cp -rf  $OUT/obj/PACKAGING/target_files_intermediates/*-target_files*/IMAGES/*.img  $IMAGE_PATH/
  echo "done."
else
	if [ -d $OUT/system ]; then
		echo -n "create system.img..."
		cp -f $OUT/system.img $IMAGE_PATH/system.img
	  echo "done."
	fi
	
	echo -n "create boot.img with kernel and resource... "
	[ -d $OUT/recovery/root ] && \
	mkbootfs $OUT/recovery/root | minigzip > $OUT/ramdisk-recovery.img && \
	      truncate -s "%4" $OUT/ramdisk-recovery.img && \
	      mkbootimg --kernel $OUT/kernel --ramdisk $OUT/ramdisk-recovery.img --second kernel/resource.img --os_version $PLATFORM_VERSION --os_patch_level $PLATFORM_SECURITY_PATCH --cmdline buildvariant=$TARGET_BUILD_VARIANT --output $OUT/boot.img && \
	cp -a $OUT/boot.img $IMAGE_PATH/
	echo "done."

	if [ -d $OUT/vendor ]; then
			echo -n "create vendor.img..."
	    cp -f $OUT/vendor.img $IMAGE_PATH/vendor.img
	    echo "done."
	fi
	
	if [ -d $OUT/oem ]; then
		echo -n "create oem.img..."
	  cp -f $OUT/oem.img $IMAGE_PATH/oem.img
	  echo "done."
	fi

	if [ -f $UBOOT_PATH/uboot.img ]
	then
		echo -n "create uboot.img..."
		cp -a $UBOOT_PATH/uboot.img $IMAGE_PATH/uboot.img
		echo "done."
	else
		echo "$UBOOT_PATH/uboot.img not fount! Please make it from $UBOOT_PATH first!"
	fi

	if [ -f $UBOOT_PATH/trust_nand.img ]
	then
	        echo -n "create trust.img..."
	        cp -a $UBOOT_PATH/trust_nand.img $IMAGE_PATH/trust.img
	        echo "done."
	elif [ -f $UBOOT_PATH/trust_with_ta.img ]
	then
	        echo -n "create trust.img..."
	        cp -a $UBOOT_PATH/trust_with_ta.img $IMAGE_PATH/trust.img
	        echo "done."
	elif [ -f $UBOOT_PATH/trust.img ]
	then
	        echo -n "create trust.img..."
	        cp -a $UBOOT_PATH/trust.img $IMAGE_PATH/trust.img
	        echo "done."
	
	else    
	        echo "$UBOOT_PATH/trust.img not fount! Please make it from $UBOOT_PATH first!"
	fi
fi

if [ -f $UBOOT_PATH/*_loader_*.bin ]
then
        echo -n "create loader..."
        cp -a $UBOOT_PATH/*_loader_*.bin $IMAGE_PATH/MiniLoaderAll.bin
        echo "done."
else
	if [ -f $UBOOT_PATH/*loader*.bin ]; then
		echo -n "create loader..."
		cp -a $UBOOT_PATH/*loader*.bin $IMAGE_PATH/MiniLoaderAll.bin
		echo "done."
	elif [ "$TARGET_PRODUCT" == "px3" -a -f $UBOOT_PATH/RKPX3Loader_miniall.bin ]; then
        echo -n "create loader..."
        cp -a $UBOOT_PATH/RKPX3Loader_miniall.bin $IMAGE_PATH/MiniLoaderAll.bin
        echo "done."
	else
        echo "$UBOOT_PATH/*MiniLoaderAll_*.bin not fount! Please make it from $UBOOT_PATH first!"
	fi
fi

if [ -f $KERNEL_PATH/resource.img ]
then
        echo -n "create resource.img..."
        cp -a $KERNEL_PATH/resource.img $IMAGE_PATH/resource.img
        echo "done."
else
        echo "$KERNEL_PATH/resource.img not fount!"
fi

if [ "$BOARD_USES_AB_IMAGE" = "true" ]; then
	echo "BOARD_USES_AB_IMAGE is true... "
	if [ -f ${TARGET_DEVICE_DIR}/parameter_ab.txt ]; then
		cp -a ${TARGET_DEVICE_DIR}/parameter_ab.txt $IMAGE_PATH/parameter.txt
		echo "done."
  else
		echo "${TARGET_DEVICE_DIR}/parameter_ab.txt not fount! Please make it from ${TARGET_DEVICE_DIR} first!"
  fi
fi

if [ "$TARGET_BASE_PARAMETER_IMAGE"x != ""x ]
then
    if [ -f $TARGET_BASE_PARAMETER_IMAGE ]
    then
        echo -n "create baseparameter..."
        cp -a $TARGET_BASE_PARAMETER_IMAGE $IMAGE_PATH/baseparameter.img
        echo "done."
    else
        echo "$TARGET_BASE_PARAMETER_IMAGE not fount!"
    fi
fi

chmod a+r -R $IMAGE_PATH/
