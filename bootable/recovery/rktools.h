/*************************************************************************
	> File Name: rktools.h
	> Author: jkand.huang
	> Mail: jkand.huang@rock-chips.com
	> Created Time: Mon 23 Jan 2017 03:05:58 PM CST
 ************************************************************************/

#ifndef _RKTOOLS_H
#define _RKTOOLS_H

#define PATH_LEN 50
#define usb_path "/mnt/usb_storage/"
#define sd_path "/mnt/external_sd/"

#define BASEPARAMER_PARTITION_NAME "/baseparameter"
#define BASEPARAMER_PARTITION_SIZE 1024*1024/2

#define NET_FILE_PATH          "./sys/class/net_cmcc_gpio/net_cmcc_led"
#define NET_FILE_PATH_NEW      "/sys/class/leds/net-green/brightness"
#define KETGRUND_FILE_PATH     "./sys/class/ir_cmcc_gpio/ir_cmcc_led"
#define POWER_FILE_PATH        "./sys/class/suspend_cmcc_gpio/suspend_cmcc_led"

#define OFF_VALUE 0
#define ON_VALUE 1

#define EMMC_POINT_NAME "emmc_point_name"
#define SD_POINT_NAME "sd_point_name"
#define SD_POINT_NAME_2 "sd_point_name_2"
#ifdef AB_OTA_UPDATER
#define SD_BLOCK_DEVICE_NODE	("/dev/block/mmcblk0p1")
#endif


static const char *point_items[] = {
    "/dev/block/mmcblk0",
    "/dev/block/mmcblk1",
    "/dev/block/mmcblk2",
    "/dev/block/mmcblk3",
};

enum type{
    MMC,
    SD,
    SDIO,
    SDcombo,
};

static const char *typeName[] = {
    "MMC",
    "SD",
    "SDIO",
    "SDcombo",
};

char* check_media_package(const char *path);
char* getSerial();
int erase_baseparameter();
void startLed();
void stopLed(int state);
void setFlashPoint();
void dumpCmdArgs(int argc, char** argv);
void SplitString(const std::string& s, std::string& result, const std::string& c);
int getEmmcState();
bool IsSpecialName(const char *str);
#endif
