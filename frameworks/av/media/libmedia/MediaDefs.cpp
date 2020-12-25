/*
 * Copyright (C) 2009 The Android Open Source Project
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

#include <media/MediaDefs.h>

namespace android {

const char *MEDIA_MIMETYPE_IMAGE_JPEG = "image/jpeg";

const char *MEDIA_MIMETYPE_VIDEO_VP8 = "video/x-vnd.on2.vp8";
const char *MEDIA_MIMETYPE_VIDEO_VP9 = "video/x-vnd.on2.vp9";
const char *MEDIA_MIMETYPE_VIDEO_AVC = "video/avc";
const char *MEDIA_MIMETYPE_VIDEO_HEVC = "video/hevc";
const char *MEDIA_MIMETYPE_VIDEO_MPEG4 = "video/mp4v-es";
const char *MEDIA_MIMETYPE_VIDEO_H263 = "video/3gpp";
const char *MEDIA_MIMETYPE_VIDEO_MPEG2 = "video/mpeg2";
const char *MEDIA_MIMETYPE_VIDEO_RAW = "video/raw";
const char *MEDIA_MIMETYPE_VIDEO_DOLBY_VISION = "video/dolby-vision";
const char *MEDIA_MIMETYPE_VIDEO_SCRAMBLED = "video/scrambled";

const char *MEDIA_MIMETYPE_AUDIO_AMR_NB = "audio/3gpp";
const char *MEDIA_MIMETYPE_AUDIO_AMR_WB = "audio/amr-wb";
const char *MEDIA_MIMETYPE_AUDIO_MPEG = "audio/mpeg";
const char *MEDIA_MIMETYPE_AUDIO_MPEG_LAYER_I = "audio/mpeg-L1";
const char *MEDIA_MIMETYPE_AUDIO_MPEG_LAYER_II = "audio/mpeg-L2";
const char *MEDIA_MIMETYPE_AUDIO_MIDI = "audio/midi";
const char *MEDIA_MIMETYPE_AUDIO_AAC = "audio/mp4a-latm";
const char *MEDIA_MIMETYPE_AUDIO_QCELP = "audio/qcelp";
const char *MEDIA_MIMETYPE_AUDIO_VORBIS = "audio/vorbis";
const char *MEDIA_MIMETYPE_AUDIO_OPUS = "audio/opus";
const char *MEDIA_MIMETYPE_AUDIO_G711_ALAW = "audio/g711-alaw";
const char *MEDIA_MIMETYPE_AUDIO_G711_MLAW = "audio/g711-mlaw";
const char *MEDIA_MIMETYPE_AUDIO_RAW = "audio/raw";
const char *MEDIA_MIMETYPE_AUDIO_FLAC = "audio/flac";
const char *MEDIA_MIMETYPE_AUDIO_AAC_ADTS = "audio/aac-adts";
const char *MEDIA_MIMETYPE_AUDIO_MSGSM = "audio/gsm";
const char *MEDIA_MIMETYPE_AUDIO_AC3 = "audio/ac3";
const char *MEDIA_MIMETYPE_AUDIO_EAC3 = "audio/eac3";
const char *MEDIA_MIMETYPE_AUDIO_SCRAMBLED = "audio/scrambled";

const char *MEDIA_MIMETYPE_CONTAINER_MPEG4 = "video/mp4";
const char *MEDIA_MIMETYPE_CONTAINER_WAV = "audio/x-wav";
const char *MEDIA_MIMETYPE_CONTAINER_OGG = "application/ogg";
const char *MEDIA_MIMETYPE_CONTAINER_MATROSKA = "video/x-matroska";
const char *MEDIA_MIMETYPE_CONTAINER_MPEG2TS = "video/mp2ts";
const char *MEDIA_MIMETYPE_CONTAINER_AVI = "video/avi";
const char *MEDIA_MIMETYPE_CONTAINER_MPEG2PS = "video/mp2p";

const char *MEDIA_MIMETYPE_TEXT_3GPP = "text/3gpp-tt";
const char *MEDIA_MIMETYPE_TEXT_SUBRIP = "application/x-subrip";
const char *MEDIA_MIMETYPE_TEXT_VTT = "text/vtt";
const char *MEDIA_MIMETYPE_TEXT_CEA_608 = "text/cea-608";
const char *MEDIA_MIMETYPE_TEXT_CEA_708 = "text/cea-708";
const char *MEDIA_MIMETYPE_DATA_TIMED_ID3 = "application/x-id3v4";

//add for ffmpeg extractor
const char *MEDIA_MIMETYPE_CONTAINER_MPG = "video/mpg";
const char *MEDIA_MIMETYPE_CONTAINER_FLV = "video/flv";
const char *MEDIA_MIMETYPE_CONTAINER_WIMO_VER1 = "video/wimo-ver1";
const char *MEDIA_MIMETYPE_CONTAINER_WMV = "video/wmv";
const char *MEDIA_MIMETYPE_CONTAINER_MOV = "video/mov";
const char *MEDIA_MIMETYPE_CONTAINER_WMA = "audio/x-ms-wma";
const char *MEDIA_MIMETYPE_CONTAINER_ASF = "video/ffm-asf";
const char *MEDIA_MIMETYPE_CONTAINER_REALVIDEO = "video/ffm-rmvb";
const char *MEDIA_MIMETYPE_CONTAINER_APE = "audio/ffm-ape";
const char *MEDIA_MIMETYPE_CONTAINER_DTS = "audio/ffm-dts";
const char *MEDIA_MIMETYPE_CONTAINER_FLAC = "audio/ffm-flac";
const char *MEDIA_MIMETYPE_CONTAINER_VC1 = "video/ffm-vc1";
const char *MEDIA_MIMETYPE_CONTAINER_HEVC = "video/ffm-hevc";
const char *MEDIA_MIMETYPE_CONTAINER_DIVX = "video/ffm-divx";
const char *MEDIA_MIMETYPE_CONTAINER_WEBM = "video/ffm-webm";
const char *MEDIA_MIMETYPE_CONTAINER_RM = "video/ffm-rm";
const char *MEDIA_MIMETYPE_CONTAINER_RA = "audio/ffm-ra";
const char *MEDIA_MIMETYPE_CONTAINER_FFMPEG = "video/ffm-ffmpeg";
const char *MEDIA_MIMETYPE_CONTAINER_MP2 = "video/ffm-mp2";
const char *MEDIA_MIMETYPE_AUDIO_ALAC = "audio/ffm-alac";
const char *MEDIA_MIMETYPE_AUDIO_RA = "audio/ffm-audiora";
const char *MEDIA_MIMETYPE_AUDIO_FFMPEG = "audio/ffm-ffmpeg";
const char *MEDIA_MIMETYPE_AUDIO_DTS = "audio/ffm-dts";
const char *MEDIA_MIMETYPE_AUDIO_APE = "audio/ffm-audioape";
const char *MEDIA_MIMETYPE_VIDEO_REALVIDEO = "video/ffm-videormvb";
const char *MEDIA_MIMETYPE_VIDEO_FFMPEG = "video/ffm-videoffmpeg";
const char *MEDIA_MIMETYPE_VIDEO_WMV = "video/ffm-wmv";
const char *MEDIA_MIMETYPE_VIDEO_DIVX = "video/ffm-videodivx";
const char *MEDIA_MIMETYPE_VIDEO_FLV =  "video/flv";
const char *MEDIA_MIMETYPE_VIDEO_VC1 =  "video/wvc1";
const char *MEDIA_MIMETYPE_VIDEO_WMV3 =  "video/x-ms-wmv";

const char *MEDIA_MIMETYPE_AUDIO_WMA = "audio/x-ms-wma";
const char *MEDIA_MIMETYPE_AUDIO_WMAPRO = "audio/x-ms-wmapro";
const char *MEDIA_MIMETYPE_AUDIO_WAV = "audio/wav";//pcm/adpcm

}  // namespace android
