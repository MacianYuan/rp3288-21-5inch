#define LOG_TAG "RkNativeDisplayManager"

#include "android_os_Parcel.h"
#include "android_util_Binder.h"
#include "android/graphics/Bitmap.h"
#include "android/graphics/GraphicsJNI.h"
#include "core_jni_helpers.h"

#include <JNIHelp.h>
#include <ScopedUtfChars.h>
#include <jni.h>
#include <memory>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <xf86drm.h>
#include <xf86drmMode.h>

#include <cutils/log.h>
#include <cutils/properties.h>
#include <drm_fourcc.h>
#include <string>
#include <map>
#include <vector>
#include <iostream>
#include <inttypes.h>
#include <sstream>

#include <linux/netlink.h>
#include <sys/socket.h>
#include "rkdisplay/drmresources.h"
#include "rkdisplay/drmmode.h"
#include "rkdisplay/drmconnector.h"

namespace android{

#define BASE_OFFSET 8*1024
#define DEFAULT_BRIGHTNESS  50
#define DEFAULT_CONTRAST  50
#define DEFAULT_SATURATION  50
#define DEFAULT_HUE  50
#define DEFAULT_OVERSCAN_VALUE 100

static struct {
    jclass clazz;
    jmethodID ctor;
    jfieldID width;
    jfieldID height;
    jfieldID refreshRate;
    jfieldID clock;
    jfieldID flags;
    jfieldID interlaceFlag;
    jfieldID yuvFlag;
    jfieldID connectorId;
    jfieldID mode_type;
    jfieldID idx;
    jfieldID hsync_start;
    jfieldID hsync_end;
    jfieldID htotal;
    jfieldID hskew;
    jfieldID vsync_start;
    jfieldID vsync_end;
    jfieldID vtotal;
    jfieldID vscan;
} gRkPhysicalDisplayInfoClassInfo;

static struct{
    jclass clazz;
    jmethodID ctor;
    jfieldID color_capa;
    jfieldID depth_capa;
}gRkColorModeSupportInfo;

struct lut_data{
    uint16_t size;
    uint16_t lred[1024];
    uint16_t lgreen[1024];
    uint16_t lblue[1024];
};

struct lut_info{
    struct lut_data main;
    struct lut_data aux;
};
int mFd=0;
DrmResources *drm_ = NULL;
DrmConnector* primary=NULL;
DrmConnector* extend=NULL;
struct lut_info* mlut=NULL;

///////////////////////////////////////////////////////////////////////////////////////////////
static bool builtInHdmi(int type){
    return type == DRM_MODE_CONNECTOR_HDMIA || type == DRM_MODE_CONNECTOR_HDMIB;
}

static bool isGammaSetEnable(int type) {
    return type == DRM_MODE_CONNECTOR_eDP || type == DRM_MODE_CONNECTOR_LVDS ||
        type == DRM_MODE_CONNECTOR_DSI || type == DRM_MODE_CONNECTOR_DPI;
}

static void updateConnectors(){
    if (drm_->connectors().size() == 2) {
        bool foundHdmi=false;
        int cnt=0,crtcId1=0,crtcId2=0;
        for (auto &conn : drm_->connectors()) {
            if (cnt == 0 && drm_->GetCrtcFromConnector(conn.get())) {
                ALOGD("encoderId1: %d", conn->encoder()->id());
                crtcId1 = drm_->GetCrtcFromConnector(conn.get())->id();
            } else if (drm_->GetCrtcFromConnector(conn.get())){
                ALOGD("encoderId2: %d", conn->encoder()->id());
                crtcId2 = drm_->GetCrtcFromConnector(conn.get())->id();
            }

            if (builtInHdmi(conn->get_type()))
                foundHdmi=true;
            cnt++;
        }
        ALOGD("crtc: %d %d foundHdmi %d", crtcId1, crtcId2, foundHdmi);
        char property[PROPERTY_VALUE_MAX];
        property_get("sys.hwc.device.primary", property, "null");
        if (crtcId1 == crtcId2 && foundHdmi && strstr(property, "HDMI-A") == NULL) {
            for (auto &conn : drm_->connectors()) {
                if (builtInHdmi(conn->get_type()) && conn->state() == DRM_MODE_CONNECTED) {
                    extend = conn.get();
                    conn->set_display(1);
                } else if(!builtInHdmi(conn->get_type()) && conn->state() == DRM_MODE_CONNECTED) {
                    primary = conn.get();
                    conn->set_display(0);
                }
            }
        } else {
            primary = drm_->GetConnectorFromType(HWC_DISPLAY_PRIMARY);
            extend = drm_->GetConnectorFromType(HWC_DISPLAY_EXTERNAL);
        }

    } else {
        primary = drm_->GetConnectorFromType(HWC_DISPLAY_PRIMARY);
        extend = drm_->GetConnectorFromType(HWC_DISPLAY_EXTERNAL);
    }
}

static void hotPlugUpdate(){
    DrmConnector *mextend = NULL;
    DrmConnector *mprimary = NULL;

    for (auto &conn : drm_->connectors()) {
        drmModeConnection old_state = conn->state();

        conn->UpdateModes();

        drmModeConnection cur_state = conn->state();
        ALOGD("old_state %d cur_state %d conn->get_type() %d", old_state, cur_state, conn->get_type());

        if (cur_state == old_state)
            continue;
        ALOGI("%s event  for connector %u\n",
                cur_state == DRM_MODE_CONNECTED ? "Plug" : "Unplug", conn->id());

        if (cur_state == DRM_MODE_CONNECTED) {
            if (conn->possible_displays() & HWC_DISPLAY_EXTERNAL_BIT)
                mextend = conn.get();
            else if (conn->possible_displays() & HWC_DISPLAY_PRIMARY_BIT)
                mprimary = conn.get();
        }
    }

    /*
     * status changed?
     */
    drm_->DisplayChanged();

    DrmConnector *old_primary = drm_->GetConnectorFromType(HWC_DISPLAY_PRIMARY);
    mprimary = mprimary ? mprimary : old_primary;
    if (!mprimary || mprimary->state() != DRM_MODE_CONNECTED) {
        mprimary = NULL;
        for (auto &conn : drm_->connectors()) {
            if (!(conn->possible_displays() & HWC_DISPLAY_PRIMARY_BIT))
                continue;
            if (conn->state() == DRM_MODE_CONNECTED) {
                mprimary = conn.get();
                break;
            }
        }
    }

    if (!mprimary) {
        ALOGE("%s %d Failed to find primary display\n", __FUNCTION__, __LINE__);
        return;
    }
    if (mprimary != old_primary) {
        drm_->SetPrimaryDisplay(mprimary);
    }

    DrmConnector *old_extend = drm_->GetConnectorFromType(HWC_DISPLAY_EXTERNAL);
    mextend = mextend ? mextend : old_extend;
    if (!mextend || mextend->state() != DRM_MODE_CONNECTED) {
        mextend = NULL;
        for (auto &conn : drm_->connectors()) {
            if (!(conn->possible_displays() & HWC_DISPLAY_EXTERNAL_BIT))
                continue;
            if (conn->id() == mprimary->id())
                continue;
            if (conn->state() == DRM_MODE_CONNECTED) {
                mextend = conn.get();
                break;
            }
        }
    }
    drm_->SetExtendDisplay(mextend);
    drm_->DisplayChanged();
    drm_->UpdateDisplayRoute();
    drm_->ClearDisplay();

    updateConnectors();
}

static void nativeInit(JNIEnv* env, jobject obj) {
    if (drm_ == NULL) {
        drm_ = new DrmResources();
        drm_->Init();
        ALOGD("nativeInit: ");
        hotPlugUpdate();
        if (primary == NULL) {
            for (auto &conn : drm_->connectors()) {
                if ((conn->possible_displays() & HWC_DISPLAY_PRIMARY_BIT)) {
                    drm_->SetPrimaryDisplay(conn.get());
                    primary = conn.get();
                }
                if ((conn->possible_displays() & HWC_DISPLAY_EXTERNAL_BIT) && conn->state() == DRM_MODE_CONNECTED) {
                    drm_->SetExtendDisplay(conn.get());
                    extend = conn.get();
                }
            }
        }
        ALOGD("primary: %p extend: %p", primary, extend);
    }
}

#define BUFFER_LENGTH    256
#define AUTO_BIT_RESET 0x00
#define RESOLUTION_AUTO 1<<0
#define COLOR_AUTO (1<<1)
#define HDCP1X_EN (1<<2)
#define RESOLUTION_WHITE_EN (1<<3)

struct drm_display_mode {
    /* Proposed mode values */
    int clock;		/* in kHz */
    int hdisplay;
    int hsync_start;
    int hsync_end;
    int htotal;
    int vdisplay;
    int vsync_start;
    int vsync_end;
    int vtotal;
    int vrefresh;
    int vscan;
    unsigned int flags;
    int picture_aspect_ratio;
};

enum output_format {
    output_rgb=0,
    output_ycbcr444=1,
    output_ycbcr422=2,
    output_ycbcr420=3,
    output_ycbcr_high_subsampling=4,  // (YCbCr444 > YCbCr422 > YCbCr420 > RGB)
    output_ycbcr_low_subsampling=5	, // (RGB > YCbCr420 > YCbCr422 > YCbCr444)
    invalid_output=6,
};

enum  output_depth{
    Automatic=0,
    depth_24bit=8,
    depth_30bit=10,
};

struct overscan {
    unsigned int maxvalue;
    unsigned short leftscale;
    unsigned short rightscale;
    unsigned short topscale;
    unsigned short bottomscale;
};

struct hwc_inital_info{
    char device[128];
    unsigned int framebuffer_width;
    unsigned int framebuffer_height;
    float fps;
};

struct bcsh_info {
    unsigned short brightness;
    unsigned short contrast;
    unsigned short saturation;
    unsigned short hue;
};

struct screen_info {
    int type;
    struct drm_display_mode resolution;// 52 bytes
    enum output_format  format; // 4 bytes
    enum output_depth depthc; // 4 bytes
    unsigned int feature;//4 //4 bytes
};

struct disp_info {
    struct screen_info screen_list[5];
    struct overscan scan;//12 bytes
    struct hwc_inital_info hwc_info; //140 bytes
    struct bcsh_info bcsh;
    unsigned int reserve[128];
    struct lut_data mlutdata;/*6k+4*/
};

struct file_base_paramer
{
    struct disp_info main;
    struct disp_info aux;
};

static char const *const device_template[] =
{
    "/dev/block/platform/1021c000.dwmmc/by-name/baseparameter",
    "/dev/block/platform/30020000.dwmmc/by-name/baseparameter",
    "/dev/block/platform/fe330000.sdhci/by-name/baseparameter",
    "/dev/block/platform/ff520000.dwmmc/by-name/baseparameter",
    "/dev/block/platform/ff0f0000.dwmmc/by-name/baseparameter",
    "/dev/block/rknand_baseparameter",
    NULL
};

const char* GetBaseparameterFile(void)
{
    int i = 0;

    while (device_template[i]) {
        if (!access(device_template[i], R_OK | W_OK))
            return device_template[i];
        ALOGD("temp[%d]=%s access=%d(%s)", i,device_template[i], errno, strerror(errno));
        i++;
    }
    return NULL;
}

static int setGamma(int fd, uint32_t crtc_id, uint32_t size,
        uint16_t *red, uint16_t *green, uint16_t *blue)
{
    int ret = drmModeCrtcSetGamma(fd, crtc_id, size, red, green, blue);
    if (ret < 0)
        ALOGE("fail to SetGamma %d(%s)", ret, strerror(errno));
    return ret;
}
#ifdef TEST_BASE_PARMARTER
static void saveHwcInitalInfo(struct file_base_paramer *base_paramer, int dpy){
    if (dpy == HWC_DISPLAY_PRIMARY_BIT){
        int len;
        char property[PROPERTY_VALUE_MAX];
        base_paramer->main.hwc_info.framebuffer_width = 1920;
        base_paramer->main.hwc_info.framebuffer_height = 1080;
        base_paramer->main.hwc_info.fps = 60.00;
        memset(property,0,sizeof(property));
        len = property_get("sys.hwc.device.primary", property, NULL);
        if (len) {
            memcpy(base_paramer->main.hwc_info.device, property, strlen(property));
        } else {
            base_paramer->main.hwc_info.device[0]='\0';
        }
    } else {
        int len;
        char property[PROPERTY_VALUE_MAX];
        base_paramer->aux.hwc_info.framebuffer_width = 1920;
        base_paramer->aux.hwc_info.framebuffer_height = 1080;
        base_paramer->aux.hwc_info.fps = 60.00;
        memset(property,0,sizeof(property));
        len = property_get("sys.hwc.device.extend", property, NULL);
        if (len)
            memcpy(base_paramer->aux.hwc_info.device, property, strlen(property));
        else
            base_paramer->aux.hwc_info.device[0]='\0';
    }
}
#endif

static void freeLutInfo(){
    if (mlut) {
        free(mlut);
        mlut=NULL;
    }
}

static int findSuitableInfoSlot(struct disp_info* info, int type) 
{
    int found=0;
    for (int i=0;i<5;i++) {
        if (info->screen_list[i].type !=0 && info->screen_list[i].type == type) {
            found = i;
            break;
        } else if (info->screen_list[i].type !=0 && found == false){
            found++;
        }
    }
    if (found == -1) {
        found = 0;
        ALOGD("noting saved, used the first slot");
    }
    ALOGD("findSuitableInfoSlot: %d type=%d", found, type);
    return found;
}

static bool getBaseParameterInfo(struct file_base_paramer* base_paramer)
{
    int file;
    const char *baseparameterfile = GetBaseparameterFile();
    if (baseparameterfile) {
        file = open(baseparameterfile, O_RDWR);
        if (file > 0) {
            unsigned int length = lseek(file, 0L, SEEK_END);

            lseek(file, 0L, SEEK_SET);
            ALOGD("getBaseParameterInfo size=%d", (int)sizeof(*base_paramer));
            if (length >  sizeof(*base_paramer)) {
                read(file, (void*)&(base_paramer->main), sizeof(base_paramer->main));
                lseek(file, BASE_OFFSET, SEEK_SET);
                read(file, (void*)&(base_paramer->aux), sizeof(base_paramer->aux));
                return true;
            }
        }
    }
    return false;
}

static void saveBcshConfig(struct file_base_paramer *base_paramer, int dpy){
    if (dpy == HWC_DISPLAY_PRIMARY_BIT){
        char property[PROPERTY_VALUE_MAX];

        memset(property,0,sizeof(property));
        property_get("persist.sys.brightness.main", property, "0");
        if (atoi(property) > 0)
            base_paramer->main.bcsh.brightness = atoi(property);
        else
            base_paramer->main.bcsh.brightness = DEFAULT_BRIGHTNESS;

        memset(property,0,sizeof(property));
        property_get("persist.sys.contrast.main", property, "0");
        if (atoi(property) > 0)
            base_paramer->main.bcsh.contrast = atoi(property);
        else
            base_paramer->main.bcsh.contrast = DEFAULT_CONTRAST;

        memset(property,0,sizeof(property));
        property_get("persist.sys.saturation.main", property, "0");
        if (atoi(property) > 0)
            base_paramer->main.bcsh.saturation = atoi(property);
        else
            base_paramer->main.bcsh.saturation = DEFAULT_SATURATION;

        memset(property,0,sizeof(property));
        property_get("persist.sys.hue.main", property, "0");
        if (atoi(property) > 0)
            base_paramer->main.bcsh.hue = atoi(property);
        else
            base_paramer->main.bcsh.hue = DEFAULT_HUE;
    } else {
        char property[PROPERTY_VALUE_MAX];

        memset(property,0,sizeof(property));
        property_get("persist.sys.brightness.aux", property, "0");
        if (atoi(property) > 0)
            base_paramer->aux.bcsh.brightness = atoi(property);
        else
            base_paramer->aux.bcsh.brightness = DEFAULT_BRIGHTNESS;

        memset(property,0,sizeof(property));
        property_get("persist.sys.contrast.aux", property, "0");
        if (atoi(property) > 0)
            base_paramer->aux.bcsh.contrast = atoi(property);
        else
            base_paramer->aux.bcsh.contrast = DEFAULT_CONTRAST;

        memset(property,0,sizeof(property));
        property_get("persist.sys.saturation.aux", property, "0");
        if (atoi(property) > 0)
            base_paramer->aux.bcsh.saturation = atoi(property);
        else
            base_paramer->aux.bcsh.saturation = DEFAULT_SATURATION;

        memset(property,0,sizeof(property));
        property_get("persist.sys.hue.aux", property, "0");
        if (atoi(property) > 0)
            base_paramer->aux.bcsh.hue = atoi(property);
        else
            base_paramer->aux.bcsh.hue = DEFAULT_HUE;
    }
}

static void nativeSaveConfig(JNIEnv* env, jobject obj) {
    char buf[BUFFER_LENGTH];
    bool isMainHdmiConnected=false;
    bool isAuxHdmiConnected = false;
    int foundMainIdx=-1,foundAuxIdx=-1;
    struct file_base_paramer base_paramer;

    if (primary != NULL) {
        std::vector<DrmMode> mModes = primary->modes();
        char resolution[PROPERTY_VALUE_MAX];
        unsigned int w=0,h=0,hsync_start=0,hsync_end=0,htotal=0;
        unsigned int vsync_start=0,vsync_end=0,vtotal=0,flags=0;
        float vfresh=0.0000;

        property_get("persist.sys.resolution.main", resolution, "0x0@0.00-0-0-0-0-0-0-0");
        if (strncmp(resolution, "Auto", 4) != 0 && strncmp(resolution, "0x0p0-0", 7) !=0)
            sscanf(resolution,"%dx%d@%f-%d-%d-%d-%d-%d-%d-%x", &w, &h, &vfresh, &hsync_start,&hsync_end,
                    &htotal,&vsync_start,&vsync_end, &vtotal, &flags);
        for (size_t c = 0; c < mModes.size(); ++c){
            const DrmMode& info = mModes[c];
            char curDrmModeRefresh[16];
            char curRefresh[16];
            float mModeRefresh;
            if (info.flags() & DRM_MODE_FLAG_INTERLACE)
                mModeRefresh = info.clock()*2 / (float)(info.v_total()* info.h_total()) * 1000.0f;
            else
                mModeRefresh = info.clock()/ (float)(info.v_total()* info.h_total()) * 1000.0f;
            sprintf(curDrmModeRefresh, "%.2f", mModeRefresh);
            sprintf(curRefresh, "%.2f", vfresh);
            if (info.h_display() == w &&
                    info.v_display() == h &&
                    info.h_sync_start() == hsync_start &&
                    info.h_sync_end() == hsync_end &&
                    info.h_total() == htotal &&
                    info.v_sync_start() == vsync_start &&
                    info.v_sync_end() == vsync_end &&
                    info.v_total()==vtotal &&
                    atof(curDrmModeRefresh)==atof(curRefresh)) {
                ALOGD("***********************found main idx %d ****************", (int)c);
                foundMainIdx = c;
                sprintf(buf, "display=%d,iface=%d,enable=%d,mode=%s\n",
                        primary->display(), primary->get_type(), primary->state(), resolution);
                break;
            }
        }
    }

    if (extend != NULL) {
        std::vector<DrmMode> mModes = extend->modes();
        char resolution[PROPERTY_VALUE_MAX];
        unsigned int w=0,h=0,hsync_start=0,hsync_end=0,htotal=0;
        unsigned int vsync_start=0,vsync_end=0,vtotal=0,flags;
        float vfresh=0;

        property_get("persist.sys.resolution.aux", resolution, "0x0@0.00-0-0-0-0-0-0-0");
        if (strncmp(resolution, "Auto", 4) != 0 && strncmp(resolution, "0x0p0-0", 7) !=0)
            sscanf(resolution,"%dx%d@%f-%d-%d-%d-%d-%d-%d-%x", &w, &h, &vfresh, &hsync_start,&hsync_end,&htotal,&vsync_start,&vsync_end,
                    &vtotal, &flags);
        for (size_t c = 0; c < mModes.size(); ++c){
            const DrmMode& info = mModes[c];
            char curDrmModeRefresh[16];
            char curRefresh[16];
            float mModeRefresh;
            if (info.flags() & DRM_MODE_FLAG_INTERLACE)
                mModeRefresh = info.clock()*2 / (float)(info.v_total()* info.h_total()) * 1000.0f;
            else
                mModeRefresh = info.clock()/ (float)(info.v_total()* info.h_total()) * 1000.0f;
            sprintf(curDrmModeRefresh, "%.2f", mModeRefresh);
            sprintf(curRefresh, "%.2f", vfresh);
            if (info.h_display() == w &&
                    info.v_display() == h &&
                    info.h_sync_start() == hsync_start &&
                    info.h_sync_end() == hsync_end &&
                    info.h_total() == htotal &&
                    info.v_sync_start() == vsync_start &&
                    info.v_sync_end() == vsync_end &&
                    info.v_total()==vtotal &&
                    atof(curDrmModeRefresh)==atoi(curRefresh)) {
                ALOGD("***********************found aux idx %d ****************", (int)c);
                foundAuxIdx = c;
                break;
            }
        }
    }

    int file;
    const char *baseparameterfile = GetBaseparameterFile();
    if (!baseparameterfile) {
        sync();
        return;
    }
    file = open(baseparameterfile, O_RDWR);
    if (file < 0) {
        ALOGW("base paramter file can not be opened");
        sync();
        return;
    }
    // caculate file's size and read it
    unsigned int length = lseek(file, 0L, SEEK_END);
    lseek(file, 0L, SEEK_SET);
    if(length < sizeof(base_paramer)) {
        ALOGE("BASEPARAME data's length is error\n");
        sync();
        close(file);
        return;
    }

    read(file, (void*)&(base_paramer.main), sizeof(base_paramer.main));
    lseek(file, BASE_OFFSET, SEEK_SET);
    read(file, (void*)&(base_paramer.aux), sizeof(base_paramer.aux));

    for (auto &conn : drm_->connectors()) {
        if (conn->state() == DRM_MODE_CONNECTED 
                && (conn->get_type() == DRM_MODE_CONNECTOR_HDMIA)
                && (conn->possible_displays() & HWC_DISPLAY_PRIMARY_BIT))
            isMainHdmiConnected = true;
        else if(conn->state() == DRM_MODE_CONNECTED 
                && (conn->get_type() == DRM_MODE_CONNECTOR_HDMIA)
                && (conn->possible_displays() & HWC_DISPLAY_EXTERNAL_BIT))
            isAuxHdmiConnected = true;
    }
    ALOGD("nativeSaveConfig: size=%d isMainHdmiConnected=%d", (int)sizeof(base_paramer.main), isMainHdmiConnected);
    for (auto &conn : drm_->connectors()) {
        if (conn->state() == DRM_MODE_CONNECTED 
                && (conn->possible_displays() & HWC_DISPLAY_PRIMARY_BIT)) {
            char property[PROPERTY_VALUE_MAX];
            int w=0,h=0,hsync_start=0,hsync_end=0,htotal=0;
            int vsync_start=0,vsync_end=0,vtotal=0,flags=0;
            int left=0,top=0,right=0,bottom=0;
            float vfresh=0;
            int slot = findSuitableInfoSlot(&base_paramer.main, conn->get_type());
            if (isMainHdmiConnected && conn->get_type() == DRM_MODE_CONNECTOR_TV)
                continue;

            base_paramer.main.screen_list[slot].type = conn->get_type();
            base_paramer.main.screen_list[slot].feature &= AUTO_BIT_RESET;
            property_get("persist.sys.resolution.main", property, "0x0@0.00-0-0-0-0-0-0-0");
            if (strncmp(property, "Auto", 4) != 0 && strncmp(property, "0x0p0-0", 7) !=0) {
                ALOGD("saveConfig resolution = %s", property);
                std::vector<DrmMode> mModes = primary->modes();
                sscanf(property,"%dx%d@%f-%d-%d-%d-%d-%d-%d-%x", &w, &h, &vfresh, &hsync_start,&hsync_end,&htotal,&vsync_start,&vsync_end,
                        &vtotal, &flags);

                ALOGD("last base_paramer.main.resolution.hdisplay = %d,  vdisplay=%d(%s@%f)",
                        base_paramer.main.screen_list[slot].resolution.hdisplay,
                        base_paramer.main.screen_list[slot].resolution.vdisplay,
                        base_paramer.main.hwc_info.device,  base_paramer.main.hwc_info.fps);
                base_paramer.main.screen_list[slot].resolution.hdisplay = w;
                base_paramer.main.screen_list[slot].resolution.vdisplay = h;
                base_paramer.main.screen_list[slot].resolution.hsync_start = hsync_start;
                base_paramer.main.screen_list[slot].resolution.hsync_end = hsync_end;
                if (foundMainIdx != -1)
                    base_paramer.main.screen_list[slot].resolution.clock = mModes[foundMainIdx].clock();
                else if (flags & DRM_MODE_FLAG_INTERLACE)
                    base_paramer.main.screen_list[slot].resolution.clock = (htotal*vtotal*vfresh/2)/1000.0f;
                else
                    base_paramer.main.screen_list[slot].resolution.clock = (htotal*vtotal*vfresh)/1000.0f;
                base_paramer.main.screen_list[slot].resolution.htotal = htotal;
                base_paramer.main.screen_list[slot].resolution.vsync_start = vsync_start;
                base_paramer.main.screen_list[slot].resolution.vsync_end = vsync_end;
                base_paramer.main.screen_list[slot].resolution.vtotal = vtotal;
                base_paramer.main.screen_list[slot].resolution.flags = flags;
                ALOGD("saveBaseParameter foundMainIdx=%d clock=%d", foundMainIdx, base_paramer.main.screen_list[slot].resolution.clock);
            } else {
                base_paramer.main.screen_list[slot].feature|= RESOLUTION_AUTO;
                memset(&base_paramer.main.screen_list[slot].resolution, 0, sizeof(base_paramer.main.screen_list[slot].resolution));
            }

            memset(property,0,sizeof(property));
            property_get("persist.sys.overscan.main", property, "overscan 100,100,100,100");
            sscanf(property, "overscan %d,%d,%d,%d",
                    &left,
                    &top,
                    &right,
                    &bottom);
            base_paramer.main.scan.leftscale = (unsigned short)left;
            base_paramer.main.scan.topscale = (unsigned short)top;
            base_paramer.main.scan.rightscale = (unsigned short)right;
            base_paramer.main.scan.bottomscale = (unsigned short)bottom;

            memset(property,0,sizeof(property));
            property_get("persist.sys.color.main", property, "Auto");
            if (strncmp(property, "Auto", 4) != 0){
                if (strstr(property, "RGB") != 0)
                    base_paramer.main.screen_list[slot].format = output_rgb;
                else if (strstr(property, "YCBCR444") != 0)
                    base_paramer.main.screen_list[slot].format = output_ycbcr444;
                else if (strstr(property, "YCBCR422") != 0)
                    base_paramer.main.screen_list[slot].format = output_ycbcr422;
                else if (strstr(property, "YCBCR420") != 0)
                    base_paramer.main.screen_list[slot].format = output_ycbcr420;
                else {
                    base_paramer.main.screen_list[slot].feature |= COLOR_AUTO;
                    base_paramer.main.screen_list[slot].format = output_ycbcr_high_subsampling;
                }

                if (strstr(property, "8bit") != NULL)
                    base_paramer.main.screen_list[slot].depthc = depth_24bit;
                else if (strstr(property, "10bit") != NULL)
                    base_paramer.main.screen_list[slot].depthc = depth_30bit;
                else
                    base_paramer.main.screen_list[slot].depthc = Automatic;
                ALOGD("saveConfig: color=%d-%d", base_paramer.main.screen_list[slot].format, base_paramer.main.screen_list[slot].depthc);
            } else {
                base_paramer.main.screen_list[slot].depthc = Automatic;
                base_paramer.main.screen_list[slot].format = output_ycbcr_high_subsampling;
                base_paramer.main.screen_list[slot].feature |= COLOR_AUTO;
            }

            memset(property,0,sizeof(property));
            property_get("persist.sys.hdcp1x.main", property, "0");
            if (atoi(property) > 0)
                base_paramer.main.screen_list[slot].feature |= HDCP1X_EN;

            memset(property,0,sizeof(property));
            property_get("persist.sys.resolution_white.main", property, "0");
            if (atoi(property) > 0)
                base_paramer.main.screen_list[slot].feature |= RESOLUTION_WHITE_EN;
            saveBcshConfig(&base_paramer, HWC_DISPLAY_PRIMARY_BIT);
#ifdef TEST_BASE_PARMARTER
            /*save aux fb & device*/
            saveHwcInitalInfo(&base_paramer, HWC_DISPLAY_PRIMARY_BIT);
#endif
        } else if(conn->state() == DRM_MODE_CONNECTED 
                && (conn->possible_displays() & HWC_DISPLAY_EXTERNAL_BIT) 
                && (conn->encoder() != NULL)) {
            char property[PROPERTY_VALUE_MAX];
            int w=0,h=0,hsync_start=0,hsync_end=0,htotal=0;
            int vsync_start=0,vsync_end=0,vtotal=0,flags=0;
            float vfresh=0;
            int left=0,top=0,right=0,bottom=0;
            int slot = findSuitableInfoSlot(&base_paramer.aux, conn->get_type());

            if (isAuxHdmiConnected && conn->get_type() == DRM_MODE_CONNECTOR_TV)
                continue;

            base_paramer.aux.screen_list[slot].type = conn->get_type();
            base_paramer.aux.screen_list[slot].feature &= AUTO_BIT_RESET;
            property_get("persist.sys.resolution.aux", property, "0x0p0-0");
            if (strncmp(property, "Auto", 4) != 0 && strncmp(property, "0x0p0-0", 7) !=0) {
                std::vector<DrmMode> mModes = extend->modes();
                sscanf(property,"%dx%d@%f-%d-%d-%d-%d-%d-%d-%x", &w, &h, &vfresh, &hsync_start,&hsync_end,&htotal,&vsync_start,&vsync_end,
                        &vtotal, &flags);
                base_paramer.aux.screen_list[slot].resolution.hdisplay = w;
                base_paramer.aux.screen_list[slot].resolution.vdisplay = h;
                if (foundMainIdx != -1)
                    base_paramer.aux.screen_list[slot].resolution.clock = mModes[foundMainIdx].clock();
                else if (flags & DRM_MODE_FLAG_INTERLACE)
                    base_paramer.aux.screen_list[slot].resolution.clock = (htotal*vtotal*vfresh/2) / 1000.0f;
                else
                    base_paramer.aux.screen_list[slot].resolution.clock = (htotal*vtotal*vfresh) / 1000.0f;
                base_paramer.aux.screen_list[slot].resolution.hsync_start = hsync_start;
                base_paramer.aux.screen_list[slot].resolution.hsync_end = hsync_end;
                base_paramer.aux.screen_list[slot].resolution.htotal = htotal;
                base_paramer.aux.screen_list[slot].resolution.vsync_start = vsync_start;
                base_paramer.aux.screen_list[slot].resolution.vsync_end = vsync_end;
                base_paramer.aux.screen_list[slot].resolution.vtotal = vtotal;
                base_paramer.aux.screen_list[slot].resolution.flags = flags;
            } else {
                base_paramer.aux.screen_list[slot].feature |= RESOLUTION_AUTO;
                memset(&base_paramer.aux.screen_list[slot].resolution, 0, sizeof(base_paramer.aux.screen_list[slot].resolution));
            }

            memset(property,0,sizeof(property));
            property_get("persist.sys.overscan.aux", property, "overscan 100,100,100,100");
            sscanf(property, "overscan %d,%d,%d,%d",
                    &left,
                    &top,
                    &right,
                    &bottom);
            base_paramer.aux.scan.leftscale = (unsigned short)left;
            base_paramer.aux.scan.topscale = (unsigned short)top;
            base_paramer.aux.scan.rightscale = (unsigned short)right;
            base_paramer.aux.scan.bottomscale = (unsigned short)bottom;

            memset(property,0,sizeof(property));
            property_get("persist.sys.color.aux", property, "Auto");
            if (strncmp(property, "Auto", 4) != 0){
                char color[16];
                char depth[16];

                sscanf(property, "%s-%s", color, depth);
                if (strncmp(color, "RGB", 3) == 0)
                    base_paramer.aux.screen_list[slot].format = output_rgb;
                else if (strncmp(color, "YCBCR444", 8) == 0)
                    base_paramer.aux.screen_list[slot].format = output_ycbcr444;
                else if (strncmp(color, "YCBCR422", 8) == 0)
                    base_paramer.aux.screen_list[slot].format = output_ycbcr422;
                else if (strncmp(color, "YCBCR420", 8) == 0)
                    base_paramer.aux.screen_list[slot].format = output_ycbcr420;
                else {
                    base_paramer.aux.screen_list[slot].feature |= COLOR_AUTO;
                    base_paramer.aux.screen_list[slot].format = output_ycbcr_high_subsampling;
                }

                if (strncmp(depth, "8bit", 4) == 0)
                    base_paramer.aux.screen_list[slot].depthc = depth_24bit;
                else if (strncmp(depth, "10bit", 5) == 0)
                    base_paramer.aux.screen_list[slot].depthc = depth_30bit;
                else
                    base_paramer.aux.screen_list[slot].depthc = Automatic;
            } else {
                base_paramer.aux.screen_list[slot].feature |= COLOR_AUTO;
                base_paramer.aux.screen_list[slot].depthc = Automatic;
                base_paramer.aux.screen_list[slot].format = output_ycbcr_high_subsampling;
            }

            memset(property,0,sizeof(property));
            property_get("persist.sys.hdcp1x.aux", property, "0");
            if (atoi(property) > 0)
                base_paramer.aux.screen_list[slot].feature |= HDCP1X_EN;

            memset(property,0,sizeof(property));
            property_get("persist.sys.resolution_white.aux", property, "0");
            if (atoi(property) > 0)
                base_paramer.aux.screen_list[slot].feature |= RESOLUTION_WHITE_EN;
            /*add for BCSH*/
            saveBcshConfig(&base_paramer, HWC_DISPLAY_EXTERNAL_BIT);
#ifdef TEST_BASE_PARMARTER
            /*save aux fb & device*/
            saveHwcInitalInfo(&base_paramer, HWC_DISPLAY_EXTERNAL_BIT);
#endif
        }
    }

    if (mlut != NULL) {
        int mainLutSize = mlut->main.size*sizeof(uint16_t);
        int auxLutSize = mlut->aux.size*sizeof(uint16_t);
        if (mainLutSize) {
            base_paramer.main.mlutdata.size = mlut->main.size;
            memcpy(base_paramer.main.mlutdata.lred, mlut->main.lred, mainLutSize);
            memcpy(base_paramer.main.mlutdata.lgreen, mlut->main.lred, mainLutSize);
            memcpy(base_paramer.main.mlutdata.lblue, mlut->main.lred, mainLutSize);
        }

        if (auxLutSize) {
            base_paramer.aux.mlutdata.size = mlut->aux.size;
            memcpy(base_paramer.aux.mlutdata.lred, mlut->aux.lred, mainLutSize);
            memcpy(base_paramer.aux.mlutdata.lgreen, mlut->aux.lred, mainLutSize);
            memcpy(base_paramer.aux.mlutdata.lblue, mlut->aux.lred, mainLutSize);
        }
    }
    freeLutInfo();
    lseek(file, 0L, SEEK_SET);
    write(file, (char*)(&base_paramer.main), sizeof(base_paramer.main));
    lseek(file, BASE_OFFSET, SEEK_SET);
    write(file, (char*)(&base_paramer.aux), sizeof(base_paramer.aux));
    close(file);
    sync();
    /*
       ALOGD("[%s] hdmi:%d,%d,%d,%d,%d,%d foundMainIdx %d\n", __FUNCTION__,
       base_paramer.main.resolution.hdisplay,
       base_paramer.main.resolution.vdisplay,
       base_paramer.main.resolution.hsync_start,
       base_paramer.main.resolution.hsync_end,
       base_paramer.main.resolution.htotal,
       base_paramer.main.resolution.flags,
       foundMainIdx);

       ALOGD("[%s] tve:%d,%d,%d,%d,%d,%d foundAuxIdx %d\n", __FUNCTION__,
       base_paramer.aux.resolution.hdisplay,
       base_paramer.aux.resolution.vdisplay,
       base_paramer.aux.resolution.hsync_start,
       base_paramer.aux.resolution.hsync_end,
       base_paramer.aux.resolution.htotal,
       base_paramer.aux.resolution.flags,
       foundAuxIdx);
     */
}

static void nativeUpdateConnectors(JNIEnv* env, jobject obj){
    hotPlugUpdate();
}

static void nativeSetMode(JNIEnv* env, jobject obj, jint dpy,jint iface_type, jstring mode)
{
    int display = dpy;
    int type = iface_type;
    const char* mMode = env->GetStringUTFChars(mode, NULL);

    ALOGD("nativeSetMode %s display %d iface_type %d", mMode, display, type);
    if (display == HWC_DISPLAY_PRIMARY){
        property_set("persist.sys.resolution.main", mMode);
    } else if (display == HWC_DISPLAY_EXTERNAL) {
        property_set("persist.sys.resolution.aux", mMode);
    }

}

static bool getResolutionInfo(int dpy, char* resolution)
{
    drmModePropertyBlobPtr blob;
    drmModeObjectPropertiesPtr props;
    DrmConnector* mCurConnector = NULL;
    DrmCrtc *crtc = NULL;
    struct drm_mode_modeinfo *drm_mode;
    struct file_base_paramer base_paramer;
    int value;
    bool found = false;
    int slot = 0;

    if (dpy == HWC_DISPLAY_PRIMARY) {
        mCurConnector = primary;
    } else if(dpy == HWC_DISPLAY_EXTERNAL) {
        mCurConnector = extend;
    }

    if (getBaseParameterInfo(&base_paramer)) {
        if (dpy == HWC_DISPLAY_PRIMARY) {
            slot = findSuitableInfoSlot(&base_paramer.main, mCurConnector->get_type());
            if (!base_paramer.main.screen_list[slot].resolution.hdisplay ||
                    !base_paramer.main.screen_list[slot].resolution.clock ||
                    !base_paramer.main.screen_list[slot].resolution.vdisplay) {
                sprintf(resolution, "%s", "Auto");
                return resolution;
            }
        } else if (dpy == HWC_DISPLAY_EXTERNAL) {
            slot = findSuitableInfoSlot(&base_paramer.aux, mCurConnector->get_type());
            if (!base_paramer.aux.screen_list[slot].resolution.hdisplay ||
                    !base_paramer.aux.screen_list[slot].resolution.clock ||
                    !base_paramer.aux.screen_list[slot].resolution.vdisplay) {
                sprintf(resolution, "%s", "Auto");
                return resolution;
            }
        }
    }

    if (mCurConnector != NULL) {
        crtc = drm_->GetCrtcFromConnector(mCurConnector);
        if (crtc == NULL) {
            return false;
        }
        props = drmModeObjectGetProperties(drm_->fd(), crtc->id(), DRM_MODE_OBJECT_CRTC);
        for (int i = 0; !found && (size_t)i < props->count_props; ++i) {
            drmModePropertyPtr p = drmModeGetProperty(drm_->fd(), props->props[i]);
            if (!strcmp(p->name, "MODE_ID")) {
                found = true;
                if (!drm_property_type_is(p, DRM_MODE_PROP_BLOB)) {
                    ALOGE("%s:line=%d,is not blob",__FUNCTION__,__LINE__);
                    drmModeFreeProperty(p);
                    drmModeFreeObjectProperties(props);
                    return false;
                }
                if (!p->count_blobs)
                    value = props->prop_values[i];
                else
                    value = p->blob_ids[0];
                blob = drmModeGetPropertyBlob(drm_->fd(), value);
                if (!blob) {
                    ALOGE("%s:line=%d, blob is null",__FUNCTION__,__LINE__);
                    drmModeFreeProperty(p);
                    drmModeFreeObjectProperties(props);
                    return false;
                }

                float vfresh;
                drm_mode = (struct drm_mode_modeinfo *)blob->data;
                if (drm_mode->flags & DRM_MODE_FLAG_INTERLACE)
                    vfresh = drm_mode->clock *2/ (float)(drm_mode->vtotal * drm_mode->htotal) * 1000.0f;
                else
                    vfresh = drm_mode->clock / (float)(drm_mode->vtotal * drm_mode->htotal) * 1000.0f;
                ALOGD("nativeGetCurMode: crtc_id=%d clock=%d w=%d %d %d %d %d %d flag=0x%x vfresh %.2f drm.vrefresh=%.2f", 
                        crtc->id(), drm_mode->clock, drm_mode->hdisplay, drm_mode->hsync_start,
                        drm_mode->hsync_end, drm_mode->vdisplay, drm_mode->vsync_start, drm_mode->vsync_end, drm_mode->flags,
                        vfresh, (float)drm_mode->vrefresh);
                sprintf(resolution, "%dx%d@%.2f-%d-%d-%d-%d-%d-%d-%x", drm_mode->hdisplay, drm_mode->vdisplay, vfresh,
                        drm_mode->hsync_start, drm_mode->hsync_end, drm_mode->htotal, 
                        drm_mode->vsync_start, drm_mode->vsync_end, drm_mode->vtotal,
                        drm_mode->flags);
                drmModeFreePropertyBlob(blob);
            }
            drmModeFreeProperty(p);
        }
        drmModeFreeObjectProperties(props);
    } else {
        return false;
    }

    return true;
}

static jstring nativeGetCurCorlorMode(JNIEnv* env, jobject obj, jint dpy)
{
    char colorMode[PROPERTY_VALUE_MAX];
    struct file_base_paramer base_paramer;
    int len=0;
    DrmConnector* mCurConnector;

    if (dpy == HWC_DISPLAY_PRIMARY) {
        len = property_get("persist.sys.color.main", colorMode, NULL);
    } else if (dpy == HWC_DISPLAY_EXTERNAL) {
        len = property_get("persist.sys.color.aux", colorMode, NULL);
    }

    ALOGD("nativeGetCurCorlorMode: property=%s", colorMode);
    if (dpy == HWC_DISPLAY_PRIMARY) {
        mCurConnector = primary;
    }else if (dpy == HWC_DISPLAY_EXTERNAL){
        mCurConnector = extend;
    } 
    if (!len) {
        if (getBaseParameterInfo(&base_paramer) && mCurConnector != NULL) {
            int slot = 0;
            if (dpy == HWC_DISPLAY_PRIMARY)
                slot = findSuitableInfoSlot(&base_paramer.main, mCurConnector->get_type());
            else
                slot = findSuitableInfoSlot(&base_paramer.aux, mCurConnector->get_type());

            if (dpy == HWC_DISPLAY_PRIMARY) {
                if (base_paramer.main.screen_list[slot].depthc == Automatic &&
                        base_paramer.main.screen_list[slot].format == output_ycbcr_high_subsampling)
                    sprintf(colorMode, "%s", "Auto");
            } else if (dpy == HWC_DISPLAY_EXTERNAL) {
                if (base_paramer.aux.screen_list[slot].depthc == Automatic &&
                        base_paramer.aux.screen_list[slot].format == output_ycbcr_high_subsampling)
                    sprintf(colorMode, "%s", "Auto");
            }
            ALOGD("nativeGetCurCorlorMode:  %d-%d", 
                    base_paramer.main.screen_list[slot].format, base_paramer.main.screen_list[slot].depthc);
        }
    }
    ALOGD("nativeGetCurCorlorMode: colorMode=%s", colorMode);
    return env->NewStringUTF(colorMode);
}

static jstring nativeGetCurMode(JNIEnv* env, jobject obj, jint dpy, jint iface_type)
{
#if 0
    int display=dpy;
    int type = iface_type;
    char resolution[PROPERTY_VALUE_MAX];
    ALOGD("nativeGetCurMode: dpy %d iface_type %d", display, type);
    if (display == HWC_DISPLAY_PRIMARY) {
        property_get("persist.sys.resolution.main", resolution, "0x0p0-0");
    } else if (display == HWC_DISPLAY_EXTERNAL) {
        property_get("persist.sys.resolution.aux", resolution, "0x0p0-0");
    }
    return env->NewString((const jchar*)resolution, strlen(resolution));
#else
    char resolution[128];
    bool found=false;
    found = getResolutionInfo(dpy, resolution);
    if (!found) {
        sprintf(resolution, "%s", "Auto");
    }
    return env->NewStringUTF(resolution);
#endif
}

static jint nativeGetNumConnectors(JNIEnv* env, jobject obj)
{
    int numConnectors=0;

    numConnectors = drm_->connectors().size();
    return static_cast<jint>(numConnectors);
}


static jint nativeGetConnectionState(JNIEnv* env, jobject obj, jint dpy)
{
    drmModeConnection cur_state=DRM_MODE_UNKNOWNCONNECTION;

    if (dpy == HWC_DISPLAY_PRIMARY && primary)
        cur_state = primary->state();
    else if (dpy == HWC_DISPLAY_EXTERNAL && extend)
        cur_state = extend->state();

    ALOGD("nativeGetConnectionState cur_state %d ", cur_state);
    return static_cast<jint>(cur_state);
}

static jint nativeGetBuiltIn(JNIEnv* env, jobject obj, jint dpy)
{
    int built_in=0;

    if (dpy == HWC_DISPLAY_PRIMARY && primary)
        built_in = primary->get_type();
    else if (dpy == HWC_DISPLAY_EXTERNAL && extend)
        built_in = extend->get_type();
    else
        built_in = 0;

    return static_cast<jint>(built_in);
}

static jobject nativeGetCorlorModeConfigs(JNIEnv* env, jclass clazz,
        jint dpy){
    int display = dpy;
    DrmConnector* mCurConnector;
    uint64_t color_capacity=0;
    uint64_t depth_capacity=0;
    jobject infoObj = env->NewObject(gRkColorModeSupportInfo.clazz,
            gRkColorModeSupportInfo.ctor);

    if (display == HWC_DISPLAY_PRIMARY) {
        mCurConnector = primary;
    }else if (display == HWC_DISPLAY_EXTERNAL){
        mCurConnector = extend;
    } else {
        return NULL;
    }

    if (mCurConnector != NULL) {
        if (mCurConnector->hdmi_output_mode_capacity_property().id())
            mCurConnector->hdmi_output_mode_capacity_property().value( &color_capacity);

        if (mCurConnector->hdmi_output_depth_capacity_property().id())
            mCurConnector->hdmi_output_depth_capacity_property().value(&depth_capacity);

        env->SetIntField(infoObj, gRkColorModeSupportInfo.color_capa, (int)color_capacity);
        env->SetIntField(infoObj, gRkColorModeSupportInfo.depth_capa, (int)depth_capacity);
        ALOGD("nativeGetCorlorModeConfigs: corlor=%d depth=%d ",(int)color_capacity,(int)depth_capacity);
    }

    return infoObj;
}

static void checkOverscanInfo(jint* mOverscan)
{
    if (mOverscan[0] < 50)
        mOverscan[0] = 50;
    else if (mOverscan[0] > 100)
        mOverscan[0] = 100;

    if (mOverscan[1] < 50)
        mOverscan[1] = 50;
    else if (mOverscan[1] > 100)
        mOverscan[1] = 100;

    if (mOverscan[2] < 50)
        mOverscan[2] = 50;
    else if (mOverscan[2] > 100)
        mOverscan[2] = 100;

    if (mOverscan[3] < 50)
        mOverscan[3] = 50;
    else if (mOverscan[3] > 100)
        mOverscan[3] = 100;
}

static void checkBcshInfo(jint* mBcsh)
{
    if (mBcsh[0] < 0)
        mBcsh[0] = 0;
    else if (mBcsh[0] > 100)
        mBcsh[0] = 100;

    if (mBcsh[1] < 0)
        mBcsh[1] = 0;
    else if (mBcsh[1] > 100)
        mBcsh[1] = 100;

    if (mBcsh[2] < 0)
        mBcsh[2] = 0;
    else if (mBcsh[2] > 100)
        mBcsh[2] = 100;

    if (mBcsh[3] < 0)
        mBcsh[3] = 0;
    else if (mBcsh[3] > 100)
        mBcsh[3] = 100;
}

static jintArray nativeGetOverscan(JNIEnv* env, jobject obj, jint dpy)
{
    jintArray jOverscanArray = env->NewIntArray(4);
    jint *mOverscan = new jint[4];
    int len=0;
    struct file_base_paramer base_paramer;
    bool foudBaseParameter=false;
    char mOverscanProperty[PROPERTY_VALUE_MAX];

    if (dpy == HWC_DISPLAY_PRIMARY)
        len = property_get("persist.sys.overscan.main", mOverscanProperty, NULL);
    else if (dpy == HWC_DISPLAY_EXTERNAL)
        len = property_get("persist.sys.overscan.aux", mOverscanProperty, NULL);
    else {
        ALOGE("error dpy: %d", dpy);
        return jOverscanArray;
    }

    if (len <= 0) {
        foudBaseParameter = getBaseParameterInfo(&base_paramer);
        if (foudBaseParameter) {
            mOverscan[0] = (int)base_paramer.main.scan.leftscale;
            mOverscan[1] = (int)base_paramer.main.scan.topscale;
            mOverscan[2] = (int)base_paramer.main.scan.rightscale;
            mOverscan[3] = (int)base_paramer.main.scan.bottomscale;
        } else {
            mOverscan[0] = DEFAULT_OVERSCAN_VALUE;
            mOverscan[1] = DEFAULT_OVERSCAN_VALUE;
            mOverscan[2] = DEFAULT_OVERSCAN_VALUE;
            mOverscan[3] = DEFAULT_OVERSCAN_VALUE;
        }
    } else {
        sscanf(mOverscanProperty, "overscan %d,%d,%d,%d", &mOverscan[0], &mOverscan[1], &mOverscan[2], &mOverscan[3]);
    }
    checkOverscanInfo(mOverscan);
    ALOGD("nativeGetOverscan: property=%s value=%d,%d,%d,%d", mOverscanProperty, mOverscan[0], mOverscan[1], mOverscan[2], mOverscan[3]);
    env->SetIntArrayRegion(jOverscanArray, 0, 4, mOverscan);
    if (mOverscan!=NULL)
        delete[] mOverscan;
    return jOverscanArray;
}

static jintArray nativeGetBcsh(JNIEnv* env, jobject obj, jint dpy)
{
    jintArray jBcshArray = env->NewIntArray(4);
    jint *mBcsh = new jint[4];
    char mBcshProperty[PROPERTY_VALUE_MAX];
    struct file_base_paramer base_paramer;
    bool foudBaseParameter=false;

    foudBaseParameter = getBaseParameterInfo(&base_paramer);
    if (dpy == HWC_DISPLAY_PRIMARY) {
        if (property_get("persist.sys.brightness.main", mBcshProperty, NULL) > 0)
            mBcsh[0] = atoi(mBcshProperty);
        else if (foudBaseParameter)
            mBcsh[0] = base_paramer.main.bcsh.brightness;
        else
            mBcsh[0] = DEFAULT_BRIGHTNESS;

        memset(mBcshProperty, 0, sizeof(mBcshProperty));
        if (property_get("persist.sys.contrast.main", mBcshProperty, NULL) > 0)
            mBcsh[1] = atoi(mBcshProperty);
        else if (foudBaseParameter)
            mBcsh[1] = base_paramer.main.bcsh.contrast;
        else
            mBcsh[1] = DEFAULT_CONTRAST;

        memset(mBcshProperty, 0, sizeof(mBcshProperty));
        if (property_get("persist.sys.saturation.main", mBcshProperty, NULL) > 0)
            mBcsh[2] = atoi(mBcshProperty);
        else if (foudBaseParameter)
            mBcsh[2] = base_paramer.main.bcsh.saturation;
        else
            mBcsh[2] = DEFAULT_SATURATION;

        memset(mBcshProperty, 0, sizeof(mBcshProperty));
        if (property_get("persist.sys.hue.main",mBcshProperty, NULL) > 0)
            mBcsh[3] = atoi(mBcshProperty);
        else if (foudBaseParameter)
            mBcsh[2] = base_paramer.main.bcsh.hue;
        else
            mBcsh[3] = DEFAULT_HUE;
    } else if (dpy == HWC_DISPLAY_EXTERNAL){
        if (property_get("persist.sys.brightness.aux", mBcshProperty, NULL) > 0)
            mBcsh[0] = atoi(mBcshProperty);
        else if (foudBaseParameter)
            mBcsh[0] = base_paramer.aux.bcsh.brightness;
        else 
            mBcsh[0] = DEFAULT_BRIGHTNESS;

        memset(mBcshProperty, 0, sizeof(mBcshProperty));
        if (property_get("persist.sys.contrast.aux", mBcshProperty, NULL) > 0)
            mBcsh[1] = atoi(mBcshProperty);
        else if (foudBaseParameter)
            mBcsh[1] = base_paramer.aux.bcsh.contrast;
        else
            mBcsh[1] = DEFAULT_CONTRAST;

        memset(mBcshProperty, 0, sizeof(mBcshProperty));
        if (property_get("persist.sys.saturation.aux", mBcshProperty, NULL) > 0)
            mBcsh[2] = atoi(mBcshProperty);
        else if (foudBaseParameter)
            mBcsh[2] = base_paramer.aux.bcsh.saturation;
        else
            mBcsh[2] = DEFAULT_SATURATION;

        memset(mBcshProperty, 0, sizeof(mBcshProperty));
        if (property_get("persist.sys.hue.aux",mBcshProperty, NULL) > 0)
            mBcsh[3] = atoi(mBcshProperty);
        else if (foudBaseParameter)
            mBcsh[2] = base_paramer.aux.bcsh.hue;
        else
            mBcsh[3] = DEFAULT_HUE;
    }
    checkBcshInfo(mBcsh);
    ALOGD("Bcsh: %d %d %d %d main.bcsh: %d %d %d %d", mBcsh[0], mBcsh[1], mBcsh[2], mBcsh[3],
            base_paramer.main.bcsh.brightness, base_paramer.main.bcsh.contrast, base_paramer.main.bcsh.saturation, base_paramer.main.bcsh.hue);
    env->SetIntArrayRegion(jBcshArray, 0, 4, mBcsh);
    if (mBcsh!=NULL)
        delete[] mBcsh;
    return jBcshArray;
}

static jint nativeSetGamma(JNIEnv* env, jobject obj,
        jint dpy, jint size, jintArray r, jintArray g, jintArray b){
    int display = dpy;
    DrmConnector* mCurConnector;
    int ret=0;

    jsize jrsize = env->GetArrayLength(r);
    jsize jgsize = env->GetArrayLength(g);
    jsize jbsize = env->GetArrayLength(b);

    jint* jr_data = env->GetIntArrayElements(r, /* isCopy */ NULL);
    jint* jg_data = env->GetIntArrayElements(g, /* isCopy */ NULL);
    jint* jb_data = env->GetIntArrayElements(b, /* isCopy */ NULL);

    uint16_t* red = (uint16_t*)malloc(jrsize*sizeof(uint16_t));
    uint16_t* green = (uint16_t*)malloc(jgsize*sizeof(uint16_t));
    uint16_t* blue = (uint16_t*)malloc(jbsize*sizeof(uint16_t));

    for (int i=0;i<jrsize;i++) {
        red[i] = jr_data[i];
    }
    for (int i=0;i<jgsize;i++) {
        green[i] = jg_data[i];
    }
    for (int i=0;i<jbsize;i++) {
        blue[i] = jb_data[i];
    }

    if (display == HWC_DISPLAY_PRIMARY) {
        mCurConnector = primary;
    }else if (display == HWC_DISPLAY_EXTERNAL){
        mCurConnector = extend;
    } else {
        return -1;
    }

    if (mCurConnector != NULL) {
        if (isGammaSetEnable(mCurConnector->get_type())) {
            int mCurCrtcId = drm_->GetCrtcFromConnector(mCurConnector)->id();
            ret = setGamma(drm_->fd(), mCurCrtcId, (int)size, (uint16_t*)red, (uint16_t*)green, (uint16_t*)blue);
            if (ret<0)
                ALOGD("nativeSetGamma failed: dpy=%d size=%d r[%d %d] rgb_size= %d %d %d red[%d %d]", display, size, 
                        jr_data[0], jr_data[1],jrsize, jgsize, jbsize
                        ,red[0], red[1]);
            else {
                if (mlut == NULL) {
                    mlut = (lut_info*)malloc(sizeof(*mlut));
                }
                if (display == HWC_DISPLAY_PRIMARY) {
                    mlut->main.size = size;
                    memcpy(mlut->main.lred, red, jrsize*sizeof(uint16_t));
                    memcpy(mlut->main.lgreen, red, jgsize*sizeof(uint16_t));
                    memcpy(mlut->main.lblue, red, jbsize*sizeof(uint16_t));
                } else {
                    mlut->aux.size = size;
                    memcpy(mlut->aux.lred, red, jrsize*sizeof(uint16_t));
                    memcpy(mlut->aux.lgreen, red, jgsize*sizeof(uint16_t));
                    memcpy(mlut->aux.lblue, red, jbsize*sizeof(uint16_t));
                }
            }
        }
    }
    if (red)
        free(red);
    if (green)
        free(green);
    if (blue)
        free(blue);
    env->ReleaseIntArrayElements(r, jr_data, 0);
    env->ReleaseIntArrayElements(g, jg_data, 0);
    env->ReleaseIntArrayElements(b, jb_data, 0);
    return ret;
}


static jobjectArray nativeGetDisplayConfigs(JNIEnv* env, jclass clazz,
        jint dpy) {
    int display = dpy;

    std::vector<DrmMode> mModes;
    DrmConnector* mCurConnector;
    int idx=0;

    if (display == HWC_DISPLAY_PRIMARY) {
        mCurConnector = primary;
        if (primary != NULL)
            mModes = primary->modes();
        ALOGD("primary built_in %d", mCurConnector->built_in());
    }else if (display == HWC_DISPLAY_EXTERNAL){
        if (extend != NULL) {
            mCurConnector = extend;
            mModes = extend->modes();
            ALOGD("extend : %d", extend->built_in());
        } else
            return NULL;
    } else {
        return NULL;
    }

    if (mModes.size() == 0)
        return NULL;

    jobjectArray configArray = env->NewObjectArray(mModes.size(),
            gRkPhysicalDisplayInfoClassInfo.clazz, NULL);

    for (size_t c = 0; c < mModes.size(); ++c) {
        const DrmMode& info = mModes[c];
        float vfresh;
        if (info.flags() & DRM_MODE_FLAG_INTERLACE)
            vfresh = info.clock()*2 / (float)(info.v_total()* info.h_total()) * 1000.0f;
        else
            vfresh = info.clock()/ (float)(info.v_total()* info.h_total()) * 1000.0f;
        jobject infoObj = env->NewObject(gRkPhysicalDisplayInfoClassInfo.clazz,
                gRkPhysicalDisplayInfoClassInfo.ctor);
        env->SetIntField(infoObj, gRkPhysicalDisplayInfoClassInfo.width, info.h_display());
        env->SetIntField(infoObj, gRkPhysicalDisplayInfoClassInfo.height, info.v_display());
        env->SetFloatField(infoObj, gRkPhysicalDisplayInfoClassInfo.refreshRate, vfresh);//1000 * 1000 * 1000 /
        env->SetIntField(infoObj, gRkPhysicalDisplayInfoClassInfo.clock, info.clock());
        env->SetIntField(infoObj, gRkPhysicalDisplayInfoClassInfo.flags, info.flags());
        env->SetBooleanField(infoObj, gRkPhysicalDisplayInfoClassInfo.interlaceFlag, info.flags()&(1<<4));
        env->SetBooleanField(infoObj, gRkPhysicalDisplayInfoClassInfo.yuvFlag, (info.flags()&(1<<24) || info.flags()&(1<<23)));
        env->SetIntField(infoObj, gRkPhysicalDisplayInfoClassInfo.connectorId, mCurConnector->id());//mode_type
        env->SetIntField(infoObj, gRkPhysicalDisplayInfoClassInfo.mode_type, info.type());
        env->SetIntField(infoObj, gRkPhysicalDisplayInfoClassInfo.idx, idx);
        env->SetIntField(infoObj, gRkPhysicalDisplayInfoClassInfo.hsync_start, info.h_sync_start());
        env->SetIntField(infoObj, gRkPhysicalDisplayInfoClassInfo.hsync_end, info.h_sync_end());
        env->SetIntField(infoObj, gRkPhysicalDisplayInfoClassInfo.htotal, info.h_total());
        env->SetIntField(infoObj, gRkPhysicalDisplayInfoClassInfo.hskew, info.h_skew());
        env->SetIntField(infoObj, gRkPhysicalDisplayInfoClassInfo.vsync_start, info.v_sync_start());
        env->SetIntField(infoObj, gRkPhysicalDisplayInfoClassInfo.vsync_end, info.v_sync_end());
        env->SetIntField(infoObj, gRkPhysicalDisplayInfoClassInfo.vtotal, info.v_total());
        env->SetIntField(infoObj, gRkPhysicalDisplayInfoClassInfo.vscan, info.v_scan());
        idx++;
        ALOGV("display%d. mode[%d]  %dx%d info.fps %f clock %d   hsync_start %d hsync_enc %d htotal %d hskew %d", 
                display,(int)c, info.h_display(), info.v_display(), info.v_refresh(), info.clock(),  info.h_sync_start(),info.h_sync_end(),
                info.h_total(), info.h_skew());
        ALOGV("vsync_start %d vsync_end %d vtotal %d vscan %d flags 0x%x", info.v_sync_start(), info.v_sync_end(),
                info.v_total(), info.v_scan(), info.flags());

        env->SetObjectArrayElement(configArray, static_cast<jsize>(c), infoObj);
        env->DeleteLocalRef(infoObj);
    }

    return configArray;
}


// ----------------------------------------------------------------------------
//com.android.server.rkdisplay
static const JNINativeMethod sRkDrmModeMethods[] = {
    {"nativeInit", "()V",
        (void*) nativeInit},
    {"nativeUpdateConnectors", "()V",
        (void*) nativeUpdateConnectors},
    {"nativeSaveConfig", "()V",
        (void*) nativeSaveConfig},
    {"nativeGetDisplayConfigs", "(I)[Lcom/android/server/rkdisplay/RkDisplayModes$RkPhysicalDisplayInfo;",
        (void*)nativeGetDisplayConfigs},
    {"nativeGetNumConnectors", "()I",
        (void*)nativeGetNumConnectors},
    {"nativeSetMode", "(IILjava/lang/String;)V",
        (void*)nativeSetMode},
    {"nativeGetCurMode", "(II)Ljava/lang/String;",
        (void*)nativeGetCurMode},
    {"nativeGetCurCorlorMode", "(I)Ljava/lang/String;",
        (void*)nativeGetCurCorlorMode},
    {"nativeGetBuiltIn", "(I)I",
        (void*)nativeGetBuiltIn},
    {"nativeGetConnectionState", "(I)I",
        (void*)nativeGetConnectionState},
    {"nativeGetCorlorModeConfigs", "(I)Lcom/android/server/rkdisplay/RkDisplayModes$RkColorCapacityInfo;",
        (void*)nativeGetCorlorModeConfigs},
    {"nativeGetBcsh", "(I)[I",
        (void*)nativeGetBcsh},
    {"nativeGetOverscan", "(I)[I",
        (void*)nativeGetOverscan},
    {"nativeSetGamma", "(II[I[I[I)I",
        (void*)nativeSetGamma},
};

#define FIND_CLASS(var, className) \
    var = env->FindClass(className); \
    LOG_FATAL_IF(! var, "Unable to find class " className);

#define GET_METHOD_ID(var, clazz, methodName, methodDescriptor) \
    var = env->GetMethodID(clazz, methodName, methodDescriptor); \
    LOG_FATAL_IF(! var, "Unable to find method " methodName);

#define GET_FIELD_ID(var, clazz, fieldName, fieldDescriptor) \
    var = env->GetFieldID(clazz, fieldName, fieldDescriptor); \
    LOG_FATAL_IF(! var, "Unable to find field " fieldName);

int register_com_android_server_rkdisplay_RkDisplayModes(JNIEnv* env)
{
    int res = jniRegisterNativeMethods(env, "com/android/server/rkdisplay/RkDisplayModes",
            sRkDrmModeMethods, NELEM(sRkDrmModeMethods));
    LOG_FATAL_IF(res < 0, "Unable to register native methods register_com_android_server_rkdisplay_RkDisplayModes");
    (void)res; // Don't complain about unused variable in the LOG_NDEBUG case

    jclass clazz;
    FIND_CLASS(clazz, "com/android/server/rkdisplay/RkDisplayModes");

    FIND_CLASS(gRkPhysicalDisplayInfoClassInfo.clazz, "com/android/server/rkdisplay/RkDisplayModes$RkPhysicalDisplayInfo");
    gRkPhysicalDisplayInfoClassInfo.clazz = jclass(env->NewGlobalRef(gRkPhysicalDisplayInfoClassInfo.clazz));
    GET_METHOD_ID(gRkPhysicalDisplayInfoClassInfo.ctor,
            gRkPhysicalDisplayInfoClassInfo.clazz, "<init>", "()V");
    GET_FIELD_ID(gRkPhysicalDisplayInfoClassInfo.width, gRkPhysicalDisplayInfoClassInfo.clazz, "width", "I");
    GET_FIELD_ID(gRkPhysicalDisplayInfoClassInfo.height, gRkPhysicalDisplayInfoClassInfo.clazz, "height", "I");
    GET_FIELD_ID(gRkPhysicalDisplayInfoClassInfo.refreshRate, gRkPhysicalDisplayInfoClassInfo.clazz, "refreshRate", "F");
    GET_FIELD_ID(gRkPhysicalDisplayInfoClassInfo.clock, gRkPhysicalDisplayInfoClassInfo.clazz, "clock", "I");
    GET_FIELD_ID(gRkPhysicalDisplayInfoClassInfo.flags, gRkPhysicalDisplayInfoClassInfo.clazz, "flags", "I");
    GET_FIELD_ID(gRkPhysicalDisplayInfoClassInfo.interlaceFlag, gRkPhysicalDisplayInfoClassInfo.clazz, "interlaceFlag", "Z");
    GET_FIELD_ID(gRkPhysicalDisplayInfoClassInfo.yuvFlag, gRkPhysicalDisplayInfoClassInfo.clazz, "yuvFlag", "Z");
    GET_FIELD_ID(gRkPhysicalDisplayInfoClassInfo.connectorId, gRkPhysicalDisplayInfoClassInfo.clazz, "connectorId", "I");
    GET_FIELD_ID(gRkPhysicalDisplayInfoClassInfo.mode_type, gRkPhysicalDisplayInfoClassInfo.clazz, "mode_type", "I");
    GET_FIELD_ID(gRkPhysicalDisplayInfoClassInfo.idx, gRkPhysicalDisplayInfoClassInfo.clazz, "idx", "I");

    GET_FIELD_ID(gRkPhysicalDisplayInfoClassInfo.hsync_start, gRkPhysicalDisplayInfoClassInfo.clazz, "hsync_start", "I");
    GET_FIELD_ID(gRkPhysicalDisplayInfoClassInfo.hsync_end, gRkPhysicalDisplayInfoClassInfo.clazz, "hsync_end", "I");
    GET_FIELD_ID(gRkPhysicalDisplayInfoClassInfo.htotal, gRkPhysicalDisplayInfoClassInfo.clazz, "htotal", "I");
    GET_FIELD_ID(gRkPhysicalDisplayInfoClassInfo.hskew, gRkPhysicalDisplayInfoClassInfo.clazz, "hskew", "I");
    GET_FIELD_ID(gRkPhysicalDisplayInfoClassInfo.vsync_start, gRkPhysicalDisplayInfoClassInfo.clazz, "vsync_start", "I");
    GET_FIELD_ID(gRkPhysicalDisplayInfoClassInfo.vsync_end, gRkPhysicalDisplayInfoClassInfo.clazz, "vsync_end", "I");
    GET_FIELD_ID(gRkPhysicalDisplayInfoClassInfo.vtotal, gRkPhysicalDisplayInfoClassInfo.clazz, "vtotal", "I");
    GET_FIELD_ID(gRkPhysicalDisplayInfoClassInfo.vscan, gRkPhysicalDisplayInfoClassInfo.clazz, "vscan", "I");

    FIND_CLASS(gRkColorModeSupportInfo.clazz, "com/android/server/rkdisplay/RkDisplayModes$RkColorCapacityInfo");
    gRkColorModeSupportInfo.clazz = jclass(env->NewGlobalRef(gRkColorModeSupportInfo.clazz));
    GET_METHOD_ID(gRkColorModeSupportInfo.ctor,
            gRkColorModeSupportInfo.clazz, "<init>", "()V");
    GET_FIELD_ID(gRkColorModeSupportInfo.color_capa, gRkColorModeSupportInfo.clazz, "color_capa", "I");
    GET_FIELD_ID(gRkColorModeSupportInfo.depth_capa, gRkColorModeSupportInfo.clazz, "depth_capa", "I");
    return 0;
}
};

