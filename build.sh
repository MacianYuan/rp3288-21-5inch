#!/bin/bash

function build_init(){
	echo "============You're building on Android==========="
	echo "Please choose BoardConfig"
	echo "  1. BoardConfig_nano3288"
	echo "  2. BoardConfig_king3288"
	echo "  3. BoardConfig_rp3288"
	echo "  4. BoardConfig_pro3288"
	echo "  5. BoardConfig_mid3288"
	echo -n "Please input num: "
	read board_num

	#add for yuv camera compatible
	cp hardware/rockchip/camera/Config/cam_board_king3288.xml hardware/rockchip/camera/Config/cam_board_rk3288.xml
	#end add 

	if [ $board_num -eq 1 ];then
		cp device/rockchip/$TARGET_PRODUCT/BoardConfig_nano3288.mk $BOARDCONFIG_RP
		echo "switch to BoardConfig_nano3288"
		 
			#add for nano yuv camera
			cp hardware/rockchip/camera/Config/cam_board_nano3288.xml hardware/rockchip/camera/Config/cam_board_rk3288.xml
			#end add 
	elif [ $board_num -eq 2 ];then
		cp device/rockchip/$TARGET_PRODUCT/BoardConfig_king3288.mk $BOARDCONFIG_RP
		echo "switch to BoardConfig_king3288"
	elif [ $board_num -eq 3 ];then
		cp device/rockchip/$TARGET_PRODUCT/BoardConfig_rp3288.mk $BOARDCONFIG_RP
		echo "switch to BoardConfig_rp3288"
	elif [ $board_num -eq 4 ];then
		cp device/rockchip/$TARGET_PRODUCT/BoardConfig_pro3288.mk $BOARDCONFIG_RP
		echo "switch to BoardConfig_pro3288"
	elif [ $board_num -eq 5 ];then
		cp device/rockchip/$TARGET_PRODUCT/BoardConfig_mid3288.mk $BOARDCONFIG_RP
		echo "switch to BoardConfig_mid3288"
	fi
	exit 1
}


BUILD_VARIANT=userdebug
source build/envsetup.sh >/dev/null && setpaths
TARGET_PRODUCT=`get_build_var TARGET_PRODUCT`

BOARDCONFIG_RP=device/rockchip/$TARGET_PRODUCT/.BoardConfig.mk
if [ -f $BOARDCONFIG_RP ];then
	source $BOARDCONFIG_RP
else
	build_init
fi

#set jdk version
export JAVA_HOME=/usr/lib/jvm/java-8-openjdk-amd64
export PATH=$JAVA_HOME/bin:$PATH
export CLASSPATH=.:$JAVA_HOME/lib:$JAVA_HOME/lib/tools.jar

# source environment and chose target product
DEVICE=`get_build_var TARGET_PRODUCT`
UBOOT_DEFCONFIG=rk3288_secure_defconfig
KERNEL_DEFCONFIG=rockchip_defconfig
KERNEL_DTS=$TARGET_BUILD_DTB

echo "KERNEL_DTS : $KERNEL_DTS"
PACK_TOOL_DIR=RKTools/linux/Linux_Pack_Firmware
IMAGE_PATH=rockdev/Image-$TARGET_PRODUCT
export PROJECT_TOP=`gettop`

lunch $DEVICE-$BUILD_VARIANT
PLATFORM_VERSION=`get_build_var PLATFORM_VERSION`
DATE=$(date  +%Y%m%d.%H%M)





# build uboot
function build_uboot(){
	echo "start build uboot"
	cd u-boot && make $UBOOT_DEFCONFIG && ./mkv7.sh && cd -
	if [ $? -eq 0 ]; then
		echo "Build uboot ok!"
	else
		echo "Build uboot failed!"
		exit 1
	fi
}


# build kernel
function build_kernel(){
	echo "Start build kernel"
	cd kernel && make ARCH=arm $KERNEL_DTS.img -j32 && cd -
	if [ $? -eq 0 ]; then
		echo "Build kernel ok!"
	else
		echo "Build kernel failed!"
		exit 1
	fi
}

# build android
function build_android(){
	echo "start build android"

	#make installclean -j8
	make -j32
	if [ $? -eq 0 ]; then
		echo "Build android ok!"
	else
		echo "Build android failed!"
		exit 1
	fi
}


# mkimage.sh
function build_mkimage(){
	echo "make and copy android images"
	./mkimage.sh
	if [ $? -eq 0 ]; then
		echo "Make image ok!"
	else
		echo "Make image failed!"
		exit 1
	fi
}

# build ota_package
function build_otapackage(){
    INTERNAL_OTA_PACKAGE_OBJ_TARGET=obj/PACKAGING/target_files_intermediates/$TARGET_PRODUCT-target_files-*.zip
    INTERNAL_OTA_PACKAGE_TARGET=$TARGET_PRODUCT-ota-*.zip
    echo "generate ota package"
    make otapackage -j4
    ./mkimage.sh ota
    cp $OUT/$INTERNAL_OTA_PACKAGE_TARGET $IMAGE_PATH/
    cp $OUT/$INTERNAL_OTA_PACKAGE_OBJ_TARGET $IMAGE_PATH/
}


# build update_image
function build_updateimg(){
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
	DATE=$(date  +%Y%m%d-%H%M%S)
	mv $PACK_TOOL_DIR/rockdev/update.img $IMAGE_PATH/update-android8.1-$TARGET_BUILD_DTB-$DATE.img
	rm $PACK_TOOL_DIR/rockdev/Image -rf
}


function build_allsave(){
    build_uboot
	build_kernel
	build_android
    build_mkimage
    build_updateimg
}


OPTIONS="$@"
for option in ${OPTIONS:-allsave}; do
    echo "processing option: $option"
    case $option in
        BoardConfig*.mk)
            CONF=$TOP_DIR/device/rockchip/$RK_TARGET_PRODUCT/$option
            echo "switching to board: $CONF"
            if [ ! -f $CONF ]; then
                echo "not exist!"
                exit 1
            fi

            ln -sf $CONF $BOARD_CONFIG
            ;;
        *)
            eval build_$option
            ;;
    esac
done
