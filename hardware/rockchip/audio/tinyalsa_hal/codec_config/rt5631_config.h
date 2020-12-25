/*
 * Copyright (C) 2015 Rockchip Electronics Co., Ltd.
*/
/**
 * @file rt5631_config.h
 * @brief
 * @author  RkAudio
 * @version 1.0.8
 * @date 2015-08-24
 */

#ifndef _RT5631_CONFIG_H_
#define _RT5631_CONFIG_H_

#include "config.h"

const struct config_control rt5631_speaker_normal_controls[] = {
	{
        .ctl_name = "HP Playback Switch",
        .int_val = {on,on},
    },
    {
        .ctl_name = "Speaker Playback Switch",
        .int_val = {on,on},
    },
	
	{
        .ctl_name = "HPR Mux",
        .str_val = "RIGHT HPVOL",
    },
	{
        .ctl_name = "HPL Mux",
        .str_val = "LEFT HPVOL",
    },
	{
        .ctl_name = "Speaker Playback Volume",
        .int_val = {55,55},
    },
	{
        .ctl_name = "HP Playback Volume",
        .int_val = {60,60},
    },

};

const struct config_control rt5631_speaker_incall_controls[] = {
};

const struct config_control rt5631_speaker_ringtone_controls[] = {
 };

const struct config_control rt5631_speaker_voip_controls[] = {
};

const struct config_control rt5631_earpiece_normal_controls[] = {


};

const struct config_control rt5631_earpiece_incall_controls[] = {

};

const struct config_control rt5631_earpiece_ringtone_controls[] = {

};

const struct config_control rt5631_earpiece_voip_controls[] = {

};

const struct config_control rt5631_headphone_normal_controls[] = {

};

const struct config_control rt5631_headphone_incall_controls[] = {

};

const struct config_control rt5631_headphone_ringtone_controls[] = {
 
};

const struct config_control rt5631_speaker_headphone_normal_controls[] = {

};

const struct config_control rt5631_speaker_headphone_ringtone_controls[] = {

};

const struct config_control rt5631_headphone_voip_controls[] = {

};

const struct config_control rt5631_headset_normal_controls[] = {

};

const struct config_control rt5631_headset_incall_controls[] = {

};

const struct config_control rt5631_headset_ringtone_controls[] = {

};

const struct config_control rt5631_headset_voip_controls[] = {

};

const struct config_control rt5631_bluetooth_normal_controls[] = {

};

const struct config_control rt5631_bluetooth_incall_controls[] = {

};

const struct config_control rt5631_bluetooth_voip_controls[] = {

};

const struct config_control rt5631_hands_free_mic_capture_controls[] = {
 
};

const struct config_control rt5631_main_mic_capture_controls[] = {
 
};

const struct config_control rt5631_bluetooth_sco_mic_capture_controls[] = {

};

const struct config_control rt5631_playback_off_controls[] = {

};

const struct config_control rt5631_capture_off_controls[] = {

};

const struct config_control rt5631_incall_off_controls[] = {

};

const struct config_control rt5631_voip_off_controls[] = {

};

const struct config_control rt5631_hdmiin_normal_controls[] = {
};

const struct config_control rt5631_hdmiin_off_controls[] = {

};
const struct config_route_table rt5631_config_table = {
    //speaker
    .speaker_normal = {
        .sound_card = 0,
        .devices = DEVICES_0,
        .controls = rt5631_speaker_normal_controls,
        .controls_count = sizeof(rt5631_speaker_normal_controls) / sizeof(struct config_control),
    },
    .speaker_incall = {
        .sound_card = 0,
        .devices = DEVICES_0,
        .controls = rt5631_speaker_incall_controls,
        .controls_count = sizeof(rt5631_speaker_incall_controls) / sizeof(struct config_control),
    },
    .speaker_ringtone = {
        .sound_card = 0,
        .devices = DEVICES_0,
        .controls = rt5631_speaker_ringtone_controls,
        .controls_count = sizeof(rt5631_speaker_ringtone_controls) / sizeof(struct config_control),
    },
    .speaker_voip = {
        .sound_card = 0,
        .devices = DEVICES_0,
        .controls = rt5631_speaker_voip_controls,
        .controls_count = sizeof(rt5631_speaker_voip_controls) / sizeof(struct config_control),
    },

    //earpiece
    .earpiece_normal = {
        .sound_card = 0,
        .devices = DEVICES_0,
        .controls = rt5631_earpiece_normal_controls,
        .controls_count = sizeof(rt5631_earpiece_normal_controls) / sizeof(struct config_control),
    },
    .earpiece_incall = {
        .sound_card = 0,
        .devices = DEVICES_0,
        .controls = rt5631_earpiece_incall_controls,
        .controls_count = sizeof(rt5631_earpiece_incall_controls) / sizeof(struct config_control),
    },
    .earpiece_ringtone = {
        .sound_card = 0,
        .devices = DEVICES_0,
        .controls = rt5631_earpiece_ringtone_controls,
        .controls_count = sizeof(rt5631_earpiece_ringtone_controls) / sizeof(struct config_control),
    },
    .earpiece_voip = {
        .sound_card = 0,
        .devices = DEVICES_0,
        .controls = rt5631_earpiece_voip_controls,
        .controls_count = sizeof(rt5631_earpiece_voip_controls) / sizeof(struct config_control),
    },

    //headphone
    .headphone_normal = {
        .sound_card = 0,
        .devices = DEVICES_0,
        .controls = rt5631_headphone_normal_controls,
        .controls_count = sizeof(rt5631_headphone_normal_controls) / sizeof(struct config_control),
    },
    .headphone_incall = {
        .sound_card = 0,
        .devices = DEVICES_0,
        .controls = rt5631_headphone_incall_controls,
        .controls_count = sizeof(rt5631_headphone_incall_controls) / sizeof(struct config_control),
    },
    .headphone_ringtone = {
        .sound_card = 0,
        .devices = DEVICES_0,
        .controls = rt5631_headphone_ringtone_controls,
        .controls_count = sizeof(rt5631_headphone_ringtone_controls) / sizeof(struct config_control),
    },
    .speaker_headphone_normal = {
        .sound_card = 0,
        .devices = DEVICES_0,
        .controls = rt5631_speaker_headphone_normal_controls,
        .controls_count = sizeof(rt5631_speaker_headphone_normal_controls) / sizeof(struct config_control),
    },
    .speaker_headphone_ringtone = {
        .sound_card = 0,
        .devices = DEVICES_0,
        .controls = rt5631_speaker_headphone_ringtone_controls,
        .controls_count = sizeof(rt5631_speaker_headphone_ringtone_controls) / sizeof(struct config_control),
    },
    .headphone_voip = {
        .sound_card = 0,
        .devices = DEVICES_0,
        .controls = rt5631_headphone_voip_controls,
        .controls_count = sizeof(rt5631_headphone_voip_controls) / sizeof(struct config_control),
    },

    //headset
    .headset_normal = {
        .sound_card = 0,
        .devices = DEVICES_0,
        .controls = rt5631_headset_normal_controls,
        .controls_count = sizeof(rt5631_headset_normal_controls) / sizeof(struct config_control),
    },
    .headset_incall = {
        .sound_card = 0,
        .devices = DEVICES_0,
        .controls = rt5631_headset_incall_controls,
        .controls_count = sizeof(rt5631_headset_incall_controls) / sizeof(struct config_control),
    },
    .headset_ringtone = {
        .sound_card = 0,
        .devices = DEVICES_0,
        .controls = rt5631_headset_ringtone_controls,
        .controls_count = sizeof(rt5631_headset_ringtone_controls) / sizeof(struct config_control),
    },
    .headset_voip = {
        .sound_card = 0,
        .devices = DEVICES_0,
        .controls = rt5631_headset_voip_controls,
        .controls_count = sizeof(rt5631_headset_voip_controls) / sizeof(struct config_control),
    },

    //bluetooth
    .bluetooth_normal = {
        .sound_card = 0,
        .devices = DEVICES_0,
        .controls = rt5631_bluetooth_normal_controls,
        .controls_count = sizeof(rt5631_bluetooth_normal_controls) / sizeof(struct config_control),
    },
    .bluetooth_incall = {
        .sound_card = 0,
        .devices = DEVICES_0_1,
        .controls = rt5631_bluetooth_incall_controls,
        .controls_count = sizeof(rt5631_bluetooth_incall_controls) / sizeof(struct config_control),
    },
    .bluetooth_voip = {
        .sound_card = 0,
        .devices = DEVICES_0_1,
        .controls = rt5631_bluetooth_voip_controls,
        .controls_count = sizeof(rt5631_bluetooth_voip_controls) / sizeof(struct config_control),
    },

    //capture
    .main_mic_capture = {
        .sound_card = 0,
        .devices = DEVICES_0,
        .controls = rt5631_main_mic_capture_controls,
        .controls_count = sizeof(rt5631_main_mic_capture_controls) / sizeof(struct config_control),
    },
    .hands_free_mic_capture = {
        .sound_card = 0,
        .devices = DEVICES_0,
        .controls = rt5631_hands_free_mic_capture_controls,
        .controls_count = sizeof(rt5631_hands_free_mic_capture_controls) / sizeof(struct config_control),
    },
    .bluetooth_sco_mic_capture = {
        .sound_card = 0,
        .devices = DEVICES_0_1,
        .controls = rt5631_bluetooth_sco_mic_capture_controls,
        .controls_count = sizeof(rt5631_bluetooth_sco_mic_capture_controls) / sizeof(struct config_control),
    },

    //off
    .playback_off = {
        .controls = rt5631_playback_off_controls,
        .controls_count = sizeof(rt5631_playback_off_controls) / sizeof(struct config_control),
    },
    .capture_off = {
        .controls = rt5631_capture_off_controls,
        .controls_count = sizeof(rt5631_capture_off_controls) / sizeof(struct config_control),
    },
    .incall_off = {
        .controls = rt5631_incall_off_controls,
        .controls_count = sizeof(rt5631_incall_off_controls) / sizeof(struct config_control),
    },
    .voip_off = {
        .controls = rt5631_voip_off_controls,
        .controls_count = sizeof(rt5631_voip_off_controls) / sizeof(struct config_control),
    },
    .hdmiin_normal = {
        .controls = rt5631_hdmiin_normal_controls,
        .controls_count = sizeof(rt5631_hdmiin_normal_controls) / sizeof(struct config_control),
    },

    .hdmiin_off = {
        .controls = rt5631_hdmiin_off_controls,
        .controls_count = sizeof(rt5631_hdmiin_off_controls) / sizeof(struct config_control),
    },
    //hdmi
    .hdmi_normal = {
        .sound_card = 1,
        .devices = DEVICES_0,
        .controls_count = 0,
    },

    //usb audio
    .usb_normal = {
        .sound_card = 2,
        .devices = DEVICES_0,
        .controls_count = 0,
    },
    .usb_capture = {
        .sound_card = 2,
        .devices = DEVICES_0,
        .controls_count = 0,
    },
};


#endif //_RT5631_CONFIG_H_
