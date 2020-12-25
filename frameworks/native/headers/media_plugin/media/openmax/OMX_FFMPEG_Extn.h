/*
 * Copyright (C) 2015 The CyanogenMod Project
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

#ifndef OMX_FFMPEG_Extn_h
#define OMX_FFMPEG_Extn_h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef enum _ROCKCHIP_VIDEO_CODINGTYPE_EXT {
    OMX_VIDEO_CodingFLV1 = 0x01000000,       /**< Sorenson H.263 */
    OMX_VIDEO_CodingDIVX3,                   /**< DIVX3 */
    OMX_VIDEO_CodingVP6,                      /**< VP6 */
    OMX_VIDEO_CodingVC1,                      /**< VP6 */
    OMX_VIDEO_OLD_CodingHEVC,
} ROCKCHIP_VIDEO_CODINGTYPE_EXT;

/**
 * FFMPEG Video Params
 */
typedef struct OMX_VIDEO_PARAM_FFMPEGTYPE {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex;

    OMX_S32 eCodecId;              /**< enum AVCodecID */
    OMX_U32 nWidth;
    OMX_U32 nHeight;
} OMX_VIDEO_PARAM_FFMPEGTYPE;

enum OMX_FFMPEG_AUDIO_CODINGTYPE
{
    OMX_AUDIO_CodingMP2  = 0x7F000001,
    OMX_AUDIO_CodingAC3  = 0x7F000002,
    OMX_AUDIO_CodingAPE  = 0x7F000003,
    OMX_AUDIO_CodingDTS  = 0x7F000004,
    OMX_AUDIO_CodingALAC = 0x7F000005,
};

/** MP2 params */
typedef struct OMX_AUDIO_PARAM_MP2TYPE {
    OMX_U32 nSize;                 /**< size of the structure in bytes */
    OMX_VERSIONTYPE nVersion;      /**< OMX specification version information */
    OMX_U32 nPortIndex;            /**< port that this structure applies to */
    OMX_U32 nChannels;             /**< Number of channels */
    OMX_U32 nBitRate;              /**< Bit rate of the input data.  Use 0 for variable
                                        rate or unknown bit rates */
    OMX_U32 nSampleRate;           /**< Sampling rate of the source data.  Use 0 for
                                        variable or unknown sampling rate. */
    OMX_AUDIO_CHANNELMODETYPE eChannelMode;   /**< Channel mode enumeration */
    OMX_AUDIO_MP3STREAMFORMATTYPE eFormat;  /**< MP3 stream format */
} OMX_AUDIO_PARAM_MP2TYPE;

/** AC3 params */
typedef struct OMX_AUDIO_PARAM_AC3TYPE {
    OMX_U32 nSize;                 /**< size of the structure in bytes */
    OMX_VERSIONTYPE nVersion;      /**< OMX specification version information */
    OMX_U32 nPortIndex;            /**< port that this structure applies to */
    OMX_U32 nChannels;             /**< Number of channels */
    OMX_U32 nBitRate;              /**< Bit rate of the input data.  Use 0 for variable
                                        rate or unknown bit rates */
    OMX_U32 nSamplingRate;         /**< Sampling rate of the source data.  Use 0 for
                                        variable or unknown sampling rate. */
    OMX_AUDIO_CHANNELMODETYPE eChannelMode;   /**< Channel mode enumeration */
} OMX_AUDIO_PARAM_AC3TYPE;

/** APE params */
typedef struct OMX_AUDIO_PARAM_APETYPE {
    OMX_U32 nSize;                /**< size of the structure in bytes */
    OMX_VERSIONTYPE nVersion;     /**< OMX specification version information */
    OMX_U32 nPortIndex;           /**< port that this structure applies to */
    OMX_U32 nChannels;            /**< Number of channels */
    OMX_U32 nBitRate;             /**< Bit rate of the input data.  Use 0 for variable
                                       rate or unknown bit rates */
    OMX_U32 nSamplingRate;        /**< Sampling rate of the source data.  Use 0 for
                                       variable or unknown sampling rate. */
    OMX_U32 nBitsPerSample;       /**< Number of bits in each sample */
    OMX_AUDIO_CHANNELMODETYPE eChannelMode;   /**< Channel mode enumeration */
} OMX_AUDIO_PARAM_APETYPE;

/** ALAC params */
typedef struct OMX_AUDIO_PARAM_ALACTYPE {
    OMX_U32 nSize;                /**< size of the structure in bytes */
    OMX_VERSIONTYPE nVersion;     /**< OMX specification version information */
    OMX_U32 nPortIndex;           /**< port that this structure applies to */
    OMX_U32 nChannels;            /**< Number of channels */
    OMX_U32 nBitRate;             /**< Bit rate of the input data.  Use 0 for variable
                                       rate or unknown bit rates */
    OMX_U32 nSamplingRate;        /**< Sampling rate of the source data.  Use 0 for
                                       variable or unknown sampling rate. */
    OMX_U32 nBitsPerSample;       /**< Number of bits in each sample */
    OMX_AUDIO_CHANNELMODETYPE eChannelMode;   /**< Channel mode enumeration */
} OMX_AUDIO_PARAM_ALACTYPE;

/** DTS params */
typedef struct OMX_AUDIO_PARAM_DTSTYPE {
    OMX_U32 nSize;                 /**< size of the structure in bytes */
    OMX_VERSIONTYPE nVersion;      /**< OMX specification version information */
    OMX_U32 nPortIndex;            /**< port that this structure applies to */
    OMX_U32 nChannels;             /**< Number of channels */
    OMX_U32 nBitRate;              /**< Bit rate of the input data.  Use 0 for variable
                                        rate or unknown bit rates */
    OMX_U32 nSamplingRate;         /**< Sampling rate of the source data.  Use 0 for
                                        variable or unknown sampling rate. */
    OMX_AUDIO_CHANNELMODETYPE eChannelMode;   /**< Channel mode enumeration */
} OMX_AUDIO_PARAM_DTSTYPE;

/** FFMPEG Audio params */
typedef struct OMX_AUDIO_PARAM_FFMPEGTYPE {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex;

    OMX_S32 eCodecId;              /**< enum AVCodecID */
    OMX_U32 nChannels;             /**< Number of channels */
    OMX_U32 nBitRate;              /**< Bit rate of the input data.  Use 0 for variable
                                        rate or unknown bit rates */
    OMX_U32 nBitsPerSample;        /**< Number of bits in each sample */
    OMX_U32 nSampleRate;           /**< Sampling rate of the source data.  Use 0 for
                                        variable or unknown sampling rate. */
    OMX_U32 nBlockAlign;           /**< is the block alignment, or block size, in bytes of the audio codec */

    OMX_S32 eSampleFormat;         /**< enum AVSampleFormat */
} OMX_AUDIO_PARAM_FFMPEGTYPE;

enum OMX_FFMPEG_INDEXTYPE
{
    OMX_IndexParamAudioMp2     = 0x7FB0001,  /**< reference: OMX_AUDIO_PARAM_MP2TYPE */
    OMX_IndexParamAudioAc3     = 0x7FB0002,  /**< reference: OMX_AUDIO_PARAM_AC3TYPE */
    OMX_IndexParamAudioApe     = 0x7FB0003,  /**< reference: OMX_AUDIO_PARAM_APETYPE */
    OMX_IndexParamAudioDts     = 0x7FB0004,  /**< reference: OMX_AUDIO_PARAM_DTSTYPE */
    OMX_IndexParamVideoFFmpeg  = 0x7FB0005,  /**< reference: OMX_VIDEO_PARAM_FFMPEGTYPE */
    OMX_IndexParamAudioFFmpeg  = 0x7FB0006,  /**< reference: OMX_AUDIO_PARAM_FFMPEGTYPE */
    OMX_IndexParamAudioAlac    = 0x7FB0007,  /**< reference: OMX_AUDIO_PARAM_ALACTYPE */
};

#ifdef __cplusplus
};
#endif /* __cplusplus */

#endif
