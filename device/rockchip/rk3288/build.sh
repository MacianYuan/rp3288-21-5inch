#!/bin/bash
usage()
{
    echo "USAGE: [-o] [-u] [-v VERSION_NAME]"
    echo "No ARGS means use default build option"
    echo "WHERE: -o = generate ota package       "
    echo "       -u = generate update.img        "
    echo "       -v = set build variant"
    exit 1
}

BUILD_UBOOT=false
BUILD_KERNEL=false
BUILD_ANDROID=false
BUILD_UPDATE_IMG=false
BUILD_OTA=false
BUILD_VARIANT=userdebug

# check pass argument
while getopts "UKAouv:" arg
do
    case $arg in
        U)
            echo "will build u-boot"
            BUILD_UBOOT=true
            ;;
        K)
            echo "will build kernel"
            BUILD_KERNEL=true
            ;;
        A)
            echo "will build android"
            BUILD_ANDROID=true
            ;;
        o)
            echo "will build ota package"
            BUILD_OTA=true
            ;;
        u)
            echo "will build update.img"
            BUILD_UPDATE_IMG=true
            ;;
        v)
            BUILD_VARIANT=$OPTARG
            ;;
        ?)
            usage ;;
    esac
done

source build/envsetup.sh >/dev/null && setpaths
TARGET_PRODUCT=`get_build_var TARGET_PRODUCT`

#set jdk version
export JAVA_HOME=/usr/lib/jvm/java-8-openjdk-amd64
export PATH=$JAVA_HOME/bin:$PATH
export CLASSPATH=.:$JAVA_HOME/lib:$JAVA_HOME/lib/tools.jar
# source environment and chose target product
DEVICE=`get_build_var TARGET_PRODUCT`
UBOOT_DEFCONFIG=rk3288_secure_defconfig
KERNEL_DEFCONFIG=rockchip_defconfig
KERNEL_DTS=rk3288-th804

echo "KERNEL_DTS : $KERNEL_DTS"
PACK_TOOL_DIR=RKTools/linux/Linux_Pack_Firmware
IMAGE_PATH=rockdev/Image-$TARGET_PRODUCT
export PROJECT_TOP=`gettop`

lunch $DEVICE-$BUILD_VARIANT

PLATFORM_VERSION=`get_build_var PLATFORM_VERSION`
DATE=$(date  +%Y%m%d.%H%M)
STUB_PATH=Image/"$KERNEL_DTS"_"$PLATFORM_VERSION"_"$DATE"_RELEASE_TEST_$BUILD_VARIANT
STUB_PATH="$(echo $STUB_PATH | tr '[:lower:]' '[:upper:]')"
export STUB_PATH=$PROJECT_TOP/$STUB_PATH
export STUB_PATCH_PATH=$STUB_PATH/PATCHES

# build uboot
if [ "$BUILD_UBOOT" = true ] ; then
echo "start build uboot"
cd u-boot && make distclean && make $UBOOT_DEFCONFIG && ./mkv7.sh && cd -
if [ $? -eq 0 ]; then
    echo "Build uboot ok!"
else
    echo "Build uboot failed!"
    exit 1
fi
fi

# build kernel
if [ "$BUILD_KERNEL" = true ] ; then
echo "Start build kernel"
cd kernel && make clean && make ARCH=arm $KERNEL_DEFCONFIG && make ARCH=arm $KERNEL_DTS.img -j64 && cd -
if [ $? -eq 0 ]; then
    echo "Build kernel ok!"
else
    echo "Build kernel failed!"
    exit 1
fi
fi

# build android
if [ "$BUILD_ANDROID" = true ] ; then
echo "start build android"

make installclean -j8
make -j64
if [ $? -eq 0 ]; then
    echo "Build android ok!"
else
    echo "Build android failed!"
    exit 1
fi
fi

if [ "$BUILD_VARIANT" = user ] ; then
    if [ -e $PROJECT_TOP/build_back/system/build.prop ] ; then
        echo "restore system/build.prop"
        cp $PROJECT_TOP/build_back/system/build.prop $OUT/system/
    else
        echo "backup system/build.prop"
        mkdir -p $PROJECT_TOP/build_back/system
        cp $OUT/system/build.prop $PROJECT_TOP/build_back/system/build.prop
    fi
    if [ -e $PROJECT_TOP/build_back/system/etc/prop.default ] ; then
        echo "restore system/etc/prop.default"
        cp $PROJECT_TOP/build_back/system/etc/prop.default $OUT/system/etc/
    else
        echo "backup system/etc/prop.default"
        mkdir -p $PROJECT_TOP/build_back/system/etc
        cp $OUT/system/etc/prop.default $PROJECT_TOP/build_back/system/etc/prop.default
    fi
    if [ -e $PROJECT_TOP/build_back/vendor/build.prop ] ; then
        echo "restore vendor/build.prop"
        cp $PROJECT_TOP/build_back/vendor/build.prop $OUT/vendor/
    else
        echo "backup vendor/build.prop"
        mkdir -p $PROJECT_TOP/build_back/vendor
        cp $OUT/vendor/build.prop $PROJECT_TOP/build_back/vendor/build.prop
    fi
    if [ -e $PROJECT_TOP/build_back/vendor/default.prop ] ; then
        echo "restore vendor/default.prop"
        cp $PROJECT_TOP/build_back/vendor/default.prop $OUT/vendor/
    else
        echo "backup vendor/default.prop"
        mkdir -p $PROJECT_TOP/build_back/vendor
        cp $OUT/vendor/default.prop $PROJECT_TOP/build_back/vendor/default.prop
    fi
fi

# mkimage.sh
echo "make and copy android images"
./mkimage.sh
if [ $? -eq 0 ]; then
    echo "Make image ok!"
else
    echo "Make image failed!"
    exit 1
fi

# build ota_package
if [ "$BUILD_OTA" = true ] ; then
    INTERNAL_OTA_PACKAGE_OBJ_TARGET=obj/PACKAGING/target_files_intermediates/$TARGET_PRODUCT-target_files-*.zip
    INTERNAL_OTA_PACKAGE_TARGET=$TARGET_PRODUCT-ota-*.zip
    echo "generate ota package"
    make otapackage -j4
    ./mkimage.sh ota
    cp $OUT/$INTERNAL_OTA_PACKAGE_TARGET $IMAGE_PATH/
    cp $OUT/$INTERNAL_OTA_PACKAGE_OBJ_TARGET $IMAGE_PATH/
fi

# build update_image
if [ "$BUILD_UPDATE_IMG" = true ] ; then
	mkdir -p $PACK_TOOL_DIR/rockdev/Image/
	cp -f $IMAGE_PATH/* $PACK_TOOL_DIR/rockdev/Image/

	echo "Make update.img"
	cd $PACK_TOOL_DIR/rockdev && ./mkupdate.sh
	if [ $? -eq 0 ]; then
		echo "Make update image ok!"
	else
		echo "Make update image failed!"
		exit 1
	fi

	cd -
	mv $PACK_TOOL_DIR/rockdev/update.img $IMAGE_PATH/
	rm $PACK_TOOL_DIR/rockdev/Image -rf
fi

mkdir -p $STUB_PATH

#Generate patches
mkdir -p $STUB_PATCH_PATH && .repo/repo/repo forall -c "git diff > local_diff.patch" && (find . -name local_diff.patch -type f -size 0c| xargs rm) && (find . -path ./IMAGE -prune -o -name local_diff.patch | cpio -pdv $STUB_PATCH_PATH/)
if [ -d $STUB_PATCH_PATH/IMAGE ]; then
    rm $STUB_PATCH_PATH/IMAGE -rf
fi

#Copy stubs
cp commit_id.xml $STUB_PATH/manifest_${DATE}.xml

mkdir -p $STUB_PATH/kernel
cp kernel/.config $STUB_PATH/kernel
cp kernel/vmlinux $STUB_PATH/kernel

mkdir -p $STUB_PATH/IMAGES/
cp $IMAGE_PATH/* $STUB_PATH/IMAGES/
cp build.sh $STUB_PATH/build.sh
#Save build command info
echo "UBOOT:  defconfig: $UBOOT_DEFCONFIG" >> $STUB_PATH/build_cmd_info
echo "KERNEL: defconfig: $KERNEL_DEFCONFIG, dts: $KERNEL_DTS" >> $STUB_PATH/build_cmd_info
echo "ANDROID:$DEVICE-$BUILD_VARIANT" >> $STUB_PATH/build_cmd_info
