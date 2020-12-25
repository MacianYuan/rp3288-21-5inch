#include "audio_setting.h"
#include <cutils/properties.h>
#include <string.h>
#include "stdio.h"
#include <stdlib.h>


#define MEDIA_CFG_AUDIO_BYPASS  "media.cfg.audio.bypass"
#define MEDIA_CFG_AUDIO_MUL     "media.cfg.audio.mul"
#define MEDIA_AUDIO_DEVICE      "persist.audio.currentplayback"



int getSettingVersion(){
    return 0;
}

int getSettingMode(){
    return 0;
}

bool isBypass(){
    char value[PROPERTY_VALUE_MAX] = "";
    property_get(MEDIA_CFG_AUDIO_BYPASS, value, "false");
    if(memcmp(value, "true", 4) == 0){
        return true;
    }

    return false;
}

bool isMultiPcm(){
    char value[PROPERTY_VALUE_MAX] = "";
    property_get(MEDIA_CFG_AUDIO_MUL, value, "false");
    if(memcmp(value, "true", 4) == 0){
        return true;
    }

    return false;
}

int getOutputDevice(){
    char value[PROPERTY_VALUE_MAX] = "";
    property_get(MEDIA_AUDIO_DEVICE, value, "");
    return atoi(value);
}
