/******************************************************************/
/*  Copyright (C)  ROCK-CHIPS FUZHOU . All Rights Reserved.       */
/*******************************************************************
 * File    :   lights.cpp
 * Desc    :   Implement lights adjust HAL
 * Author  :   CMY
 * Date    :   2009-07-22
 * Notes   :   ..............
 *
 * Revision 1.00  2009/07/22 CMY
 * Revision 2.00 2012/01/08 yxj
 * support button charge lights
 * Revision 3.00 2016/11/01 yhx
 *
 * ...................
 * ********************************************************************/

#define LOG_TAG "Lights Hal"

#include <cutils/log.h>
#include <cutils/atomic.h>

#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <hardware/lights.h>

#define LOGV(fmt,args...) ALOGV(fmt,##args)
#define LOGD(fmt,args...) ALOGD(fmt,##args)
#define LOGI(fmt,args...) ALOGI(fmt,##args)
#define LOGW(fmt,args...) ALOGW(fmt,##args)
#define LOGE(fmt,args...) ALOGE(fmt,##args)

/*****************************************************************************/
#define BACKLIGHT_PATH  "/sys/class/backlight/rk28_bl/brightness"
#define BACKLIGHT_PATH1 "/sys/class/backlight/backlight/brightness" // for kernel 4.4
#define BUTTON_LED_PATH "sys/class/leds/rk29_key_led/brightness"
#define BATTERY_LED_PATH "sys/class/leds/battery_led/brightness"
int g_bl_fd = 0;   //backlight fd
int g_btn_fd = 0; //button light fd
int g_bat_fd = 0; //battery charger fd
static pthread_once_t g_init = PTHREAD_ONCE_INIT;
static pthread_mutex_t g_lock = PTHREAD_MUTEX_INITIALIZER;

static int light_device_open(const struct hw_module_t* module, const char* name, struct hw_device_t** device);

static void init_g_lock(void)
{
    pthread_mutex_init(&g_lock, NULL);
}

static int write_int(char const *path, int value)
{
    int fd;
    static int already_warned;

    already_warned = 0;

    LOGV("write_int: path %s, value %d", path, value);
    fd = open(path, O_RDWR);

    if (fd >= 0) {
        char buffer[20];
        int bytes = sprintf(buffer, "%d\n", value);
        int amt = write(fd, buffer, bytes);
        close(fd);
        return amt == -1 ? -errno : 0;
    } else {
        if (already_warned == 0) {
            LOGE("write_int failed to open %s\n", path);
            already_warned = 1;
        }
        return -errno;
    }
}

static int rgb_to_brightness(struct light_state_t const *state)
{
    unsigned int color = state->color & 0x00ffffff;
    unsigned char brightness = ((77*((color>>16)&0x00ff)) + 
        (150*((color>>8)&0x00ff)) + (29*(color&0x00ff))) >> 8;
    return brightness;
}

int set_backlight_light(struct light_device_t* dev, struct light_state_t const* state)
{
    int err = 0;
    int brightness = rgb_to_brightness(state);
    pthread_mutex_lock(&g_lock);
    err = write_int(BACKLIGHT_PATH, brightness);
    if (err !=0)
        err = write_int(BACKLIGHT_PATH1, brightness);
    pthread_mutex_unlock(&g_lock);
    return 0;
}

int set_keyboard_light(struct light_device_t* dev, struct light_state_t const* state)
{
    LOGI(">>> Enter set_keyboard_light");
    return 0;
}

int set_buttons_light(struct light_device_t* dev, struct light_state_t const* state)
{
    int err = 0;
    int brightness = rgb_to_brightness(state);
    pthread_mutex_lock(&g_lock);
    err = write_int(BUTTON_LED_PATH, brightness?1:0);
    pthread_mutex_unlock(&g_lock);
    return 0;
}

int set_battery_light(struct light_device_t* dev, struct light_state_t const* state)
{
    int err = 0;
    int brightness = rgb_to_brightness(state);
    pthread_mutex_lock(&g_lock);
    err = write_int(BATTERY_LED_PATH, brightness?1:0);
    pthread_mutex_unlock(&g_lock);
    return 0;
}

int set_notifications_light(struct light_device_t* dev, struct light_state_t const* state)
{
    LOGI(">>> Enter set_notifications_light");
    return 0;
}

int set_attention_light(struct light_device_t* dev, struct light_state_t const* state)
{
    LOGI(">>> Enter set_attention_light");
    return 0;
}

static int lights_device_close(struct light_device_t *dev) 
{
    LOGI(">>> Enter light_device_close");
    if (dev)
        free(dev);
    return 0;
}

/**
 * module methods
 */
static int lights_device_open(const struct hw_module_t* module, char const* name, 
        struct hw_device_t** device)
{
    LOGI(">>> Enter light_device_open:%s\n",name);
    int status = 0;

    struct light_device_t *dev;
    dev = (light_device_t*)malloc(sizeof(*dev));

    /* initialize our state here */
    memset(dev, 0, sizeof(*dev));

    /* initialize the procs */
    dev->common.tag = HARDWARE_DEVICE_TAG;
    dev->common.version = LIGHTS_DEVICE_API_VERSION_1_0;
    dev->common.module = (struct hw_module_t*)module;
    dev->common.close = (int (*)(struct hw_device_t*))lights_device_close;
    *device = &dev->common;
   
    if (!strcmp(name, LIGHT_ID_BACKLIGHT)) {
        dev->set_light = set_backlight_light;
    }else if(!strcmp(name, LIGHT_ID_KEYBOARD)) {
        dev->set_light = set_keyboard_light;
    }else if(!strcmp(name, LIGHT_ID_BUTTONS)) {
        dev->set_light = set_buttons_light;
    }else if(!strcmp(name, LIGHT_ID_BATTERY)) {
        dev->set_light = set_battery_light;
    }else if(!strcmp(name, LIGHT_ID_NOTIFICATIONS)) {
        dev->set_light = set_notifications_light;
    }else if(!strcmp(name, LIGHT_ID_ATTENTION)) {
        dev->set_light = set_attention_light;
    }else{
        LOGI(">>> undefine light id");
        free(dev);
        *device = NULL;
        return -EINVAL;
    }
    pthread_once(&g_init,init_g_lock);
    return status;
}

static struct hw_module_methods_t light_module_methods = {
    .open = lights_device_open
};

/*
 * The lights Module
 */
struct hw_module_t HAL_MODULE_INFO_SYM = {
    .tag = HARDWARE_MODULE_TAG,
    .version_major = 1,
    .version_minor = 0,
    .id = LIGHTS_HARDWARE_MODULE_ID,
    .name = "lights module",
    .author = "Rockchip, Inc.",
    .methods = &light_module_methods,
};

