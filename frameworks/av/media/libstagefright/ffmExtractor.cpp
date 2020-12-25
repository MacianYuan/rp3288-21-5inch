#define LOG_NDEBUG 0
#define LOG_TAG "ffmExtractor"
#include <utils/Log.h>
//#define DUMP_TO_FILE


#include <media/stagefright/MediaExtractor.h>
#include <media/stagefright/DataSource.h>
#include <media/stagefright/MediaDefs.h>
#include <utils/String8.h>
#include <dlfcn.h>  // for dlopen/dlclose

#include "include/ffmExtractor.h"
#define LOGV ALOGV
#define LOGI ALOGI
#define LOGE ALOGE
#define LOGD ALOGD

namespace android {


void *g_RkDemuxLib = NULL;


void ffmpegDemuxLib()
{
    char  value[PROPERTY_VALUE_MAX];
    bool  bGtsCtsTest = false;

    g_RkDemuxLib = NULL;
    if (property_get("cts_gts.status", value, NULL) && (!strcasecmp(value, "true"))) {
        ALOGI("This is cts/gts  test.");
        bGtsCtsTest = true;
    }
    if (g_RkDemuxLib == NULL && !bGtsCtsTest) {
         g_RkDemuxLib = dlopen("libffmpeg_utils.so", RTLD_LAZY);
    }
    if (g_RkDemuxLib == NULL) {
        ALOGE("Cannot load library %s dlerror = %s",FFMPEG_DEMUX_LIB,dlerror());
    }
    return;
}

ffmpegExtractorFactory *ffmpegExtractorFactoryFunction()
{
    static ffmpegExtractorFactory *mediaFactoryFunction = NULL;

    ffmpegDemuxLib();
    if (g_RkDemuxLib == NULL) {
        LOGE("lib open failed");
        return NULL;
    }

    mediaFactoryFunction = (ffmpegExtractorFactory *) dlsym(g_RkDemuxLib, CREATE_FFMPEG_EXTRACTOR);

    if(mediaFactoryFunction == NULL) {
        LOGE(" dlsym for ExtendedExtractor factory function failed, dlerror = %s \n", dlerror());
    }

    return mediaFactoryFunction;
}

sp<MediaExtractor> ffmExtractor::CreateExtractor(const sp<DataSource> &source,const char* mime, const sp<AMessage> &meta) 
{
    ffmpegExtractorFactory *f = ffmpegExtractorFactoryFunction();
    if(f==NULL) {
        return NULL;
    }
    ALOGE("CreateExtractor mime = %s",mime);
    sp<MediaExtractor> extractor = (MediaExtractor *)f((const void *)mime, (void *)&meta, (void *)&source);
    if(extractor == NULL) {
        LOGE(" ExtendedExtractor failed to instantiate extractor \n");
    }

    return extractor;
}

bool ffmExtractor::SniffFFMPEG(const sp<DataSource> &source, String8 *mimeType, float *confidence, sp<AMessage> *meta)
{
    static SniffFunc *snifferFunc = NULL;

    ffmpegDemuxLib();
    if (g_RkDemuxLib == NULL) {
        LOGE("lib open failed");
        return false;
    }

    snifferFunc = (SniffFunc *)dlsym(g_RkDemuxLib, FFMPEG_SNIFFER);
    if(snifferFunc == NULL) {
        LOGE(" Unable to init Extended Sniffers, dlerror = %s \n", dlerror());
        return false;
    }
    ALOGE("SniffFFMPEG mime = %s confidence = %f",(*mimeType).string(), *confidence);
    bool ret = snifferFunc((void *)&source, (void *)mimeType, (void *)confidence, (void *)meta);
    if (meta == NULL) {
        ALOGE("xlm %s %d in meta == NULL",__FUNCTION__,__LINE__);
    }
    return ret;
}

}  // namespace android

