/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "Sensors"

#include <hardware/sensors.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>
#include <math.h>
#include <poll.h>
#include <pthread.h>
#include <linux/input.h>

#include "gsensor_test.h"
#include "../../hardware/rockchip/sensor/st/mma8452_kernel.h"              // 声明驱动为 HAL 提供的功能接口. 应该用更加抽象的文件名.
#include "common.h"
#include "test_case.h"
#include "language.h"

#define EVENT_TYPE_ACCEL_X          ABS_X
#define EVENT_TYPE_ACCEL_Y          ABS_Y
#define EVENT_TYPE_ACCEL_Z          ABS_Z
#define ACCELERATION_RATIO_ANDROID_TO_HW        (9.80665f / 16384)
#define  CTL_DEV_PATH    "/dev/mma8452_daemon"

static float g_x = 0;
static float g_y = 0;
static float g_z = 0;

static int openInput(const char* inputName)
{
    int fd = -1;
    const char *dirname = "/dev/input";
    char devname[512];
    char *filename;
    DIR *dir;
    struct dirent *de;

    //return getInput(inputName);

    dir = opendir(dirname);
    if(dir == NULL)
        return -1;
    strcpy(devname, dirname);
    filename = devname + strlen(devname);
    *filename++ = '/';
    while((de = readdir(dir))) {
        if(de->d_name[0] == '.' &&
                (de->d_name[1] == '\0' ||
                        (de->d_name[1] == '.' && de->d_name[2] == '\0')))
            continue;
        strcpy(filename, de->d_name);
        fd = open(devname, O_RDONLY);
        if (fd>=0) {
            char name[80];
            if (ioctl(fd, EVIOCGNAME(sizeof(name) - 1), &name) < 1) {
                name[0] = '\0';
            }
            if (!strcmp(name, inputName)) {
                break;
            } else {
                close(fd);
                fd = -1;
            }
        }
    }
    closedir(dir);
   
    return fd;
}

static int  processEvent(int code, int value)
{
    float v;

    switch (code) 
    {
        case EVENT_TYPE_ACCEL_X:
            g_x = value * ACCELERATION_RATIO_ANDROID_TO_HW;
            break;
        case EVENT_TYPE_ACCEL_Y:
            g_y = value * ACCELERATION_RATIO_ANDROID_TO_HW;
            break;
        case EVENT_TYPE_ACCEL_Z:
            g_z = value * ACCELERATION_RATIO_ANDROID_TO_HW;
            break;
    }

    return 0;
}

static int readEvents(int fd)
{
    int i;
    struct input_event  event;

    for (i=0; i<6; i++) {
        ssize_t n = read(fd, &event,sizeof(struct input_event));
        if (n < 0)
        {
            printf("gsensor read fail!\n");
            return n;
        }

        int type = event.type;
        if (type == EV_ABS)
            processEvent(event.code, event.value);
    }

    return 0;
}

void* gsensor_test(void *argv)
{
    int do_calibration;
    int is_calibration = 0;
    int tempFd;
    int ret;
    int fd;
    short delay = 20; // 20ms
    struct gsensor_msg g_msg;
    struct testcase_info *tc_info = (struct testcase_info*)argv;

    /*remind ddr test*/
    if (tc_info->y <= 0)
        tc_info->y  = get_cur_print_y();	

    g_msg.y = tc_info->y;
    ui_print_xy_rgba(0,g_msg.y,255,255,0,255,"%s:[%s..] \n",PCBA_GSENSOR,PCBA_TESTING);
    tc_info->result = 0;

    fd = openInput("gsensor");
    if (fd < 0)
    {
        ui_print_xy_rgba(0,g_msg.y,255,0,0,255,"%s:[%s]\n",PCBA_GSENSOR,PCBA_FAILED);
        g_msg.result = -1;
        tc_info->result = -1;
        return argv;
    }

    int fd_dev = open(CTL_DEV_PATH, O_RDONLY);
    if (fd_dev<0)
    {
        printf("open gsensor demon fail\n");
        ui_print_xy_rgba(0, g_msg.y, 255, 0, 0, 255, "%s:[%s]\n", PCBA_GSENSOR, PCBA_FAILED);
        g_msg.result = -1;
        tc_info->result = -1;
        close(fd);
        return argv;
    }

    ret = ioctl(fd_dev, GSENSOR_IOCTL_APP_SET_RATE, &delay);
    if (ret < 0)
    {
        printf("set sensor rate fail!\n");
        ui_print_xy_rgba(0, g_msg.y, 255, 0, 0, 255, "%s:[%s]\n", PCBA_GSENSOR, PCBA_FAILED);
        g_msg.result = -1;
        tc_info->result = -1;
        close(fd_dev);
        close(fd);
        return argv;
    }

    ret = ioctl(fd_dev, MMA_IOCTL_START);
    if (ret < 0)
    {
        printf("start sensor fail!\n");
        ui_print_xy_rgba(0, g_msg.y, 255, 0, 0, 255, "%s:[%s]\n", PCBA_GSENSOR, PCBA_FAILED);
        g_msg.result = -1;
        tc_info->result = -1;
        close(fd_dev);
        close(fd);
        return argv;
    }

    tempFd = open("/sys/class/sensor_class/accel_calibration", O_RDWR);
    if (tempFd > 0)
        printf("open /sys/class/sensor_class/accel_calibration success\n");

    script_fetch("gsensor", "calibrate", &do_calibration, 1);
    printf("gsensor do calibration = %d\n", do_calibration);
    for (;;)
    {
        readEvents(fd);
        if (tempFd && do_calibration && !is_calibration) {
            char c = '1';
            ret = write(tempFd, &c, 1);
            if (ret >= 0) {
                printf("gsensor calibrate success\n");
                is_calibration = 1;
            } else {
                printf("gsensor calibrate failed, ret = %d, %s\n", ret, strerror(errno));
            }
        }
        if (do_calibration)
            ui_print_xy_rgba(0, g_msg.y, is_calibration?0:255, is_calibration?255:0, 0, 255, "%s:[%s]   %s[%s] %4f %4f %4f\n",
                                          PCBA_GSENSOR, PCBA_SECCESS, PCBA_GSENSOR_CALIBRATE,
                                          is_calibration ? PCBA_SECCESS: PCBA_FAILED, g_x, g_y, g_z);
        else
            ui_print_xy_rgba(0, g_msg.y, 0, 255, 0, 255, "%s:[%s] %4f %4f %4f\n", PCBA_GSENSOR, PCBA_SECCESS, g_x, g_y, g_z);
        usleep(1000000);
    }
    if (tempFd)
        close(tempFd);
    close(fd);
    close(fd_dev);

    ui_print_xy_rgba(0, g_msg.y, 0, 255, 0, 255, "%s:[%s]\n", PCBA_GSENSOR, PCBA_SECCESS);
    tc_info->result = 0;
    return argv;
}
