/******************************************************************************
 *
 * Copyright 2010, Dream Chip Technologies GmbH. All rights reserved.
 * No part of this work may be reproduced, modified, distributed, transmitted,
 * transcribed, or translated into any language or computer format, in any form
 * or by any means without written permission of:
 * Dream Chip Technologies GmbH, Steinriede 10, 30827 Garbsen / Berenbostel,
 * Germany
 *
 *****************************************************************************/
/**
 * @file cam_types.h
 *
 * @brief Interface description for image sensor specific implementation (iss).
 *
 *****************************************************************************/
/**
 * @page module_name_page Module Name
 * Describe here what this module does.
 *
 * For a detailed list of functions and implementation detail refer to:
 * - @ref module_name
 *
 * @defgroup cam_types   Common Camera Type Definitions
 * @{
 *
 */
#ifndef __CAM_TYPES_H__
#define __CAM_TYPES_H__

#include <common/list.h>

#ifdef __cplusplus
extern "C"
{
#endif



/*****************************************************************************/
/**
 * @brief   doortype of an illumination profile
 */
/*****************************************************************************/
typedef enum  CamDoorType_e
{
    CAM_DOOR_TYPE_OUTDOOR = 0,
    CAM_DOOR_TYPE_INDOOR  = ( !CAM_DOOR_TYPE_OUTDOOR )
} CamDoorType_t;



/*****************************************************************************/
/**
 * @brief   doortype of an illumination profile
 */
/*****************************************************************************/
typedef enum CamAwbType_e
{
    CAM_AWB_TYPE_MANUAL  = 0,
    CAM_AWB_TYPE_AUTO    = ( !CAM_AWB_TYPE_MANUAL )
} CamAwbType_t;



/*****************************************************************************/
/**
 * @brief   3 channel color components
 */
/*****************************************************************************/
typedef enum Cam3ChColorComponent_e
{
    CAM_3CH_COLOR_COMPONENT_RED     = 0,
    CAM_3CH_COLOR_COMPONENT_GREEN   = 1,
    CAM_3CH_COLOR_COMPONENT_BLUE    = 2,
    CAM_3CH_COLOR_COMPONENT_MAX
} Cam3ChColorComponent_t;



/*****************************************************************************/
/**
 * @brief   4 channel color components
 */
/*****************************************************************************/
typedef enum Cam4ChColorComponent_e
{
    CAM_4CH_COLOR_COMPONENT_RED     = 0,
    CAM_4CH_COLOR_COMPONENT_GREENR  = 1,
    CAM_4CH_COLOR_COMPONENT_GREENB  = 2,
    CAM_4CH_COLOR_COMPONENT_BLUE    = 3,
    CAM_4CH_COLOR_COMPONENT_MAX
} Cam4ChColorComponent_t;

/******************************************************************************/
/**
 * @brief   A structure to represent a 5x5 matrix.
 *
 *          The 25 values are laid out as follows (zero based index):
 *
 *               | 00 01 02 03 04 | \n
 *               | 05 06 07 08 09 | \n
 *               | 10 11 12 13 14 | \n
 *               | 15 16 17 18 19 | \n
 *               | 20 21 22 23 24 | \n
 *         
 * @note    The 25 values are represented as unsigned char numbers.
 *
 *****************************************************************************/
typedef struct Cam5x5UCharMatrix_s
{
	uint8_t uCoeff[5*5];			  /**< array of 5x5 unsigned char values */
} Cam5x5UCharMatrix_t;


/*****************************************************************************/
/**
 * @brief   Matrix coefficients
 *
 *          | 0 |
 *
 * @note    Coefficients are represented as float numbers
 */
/*****************************************************************************/
typedef struct Cam1x1FloatMatrix_s
{
    float fCoeff[1];
} Cam1x1FloatMatrix_t;



/*****************************************************************************/
/**
 * @brief   Matrix coefficients
 *
 *          | 0 | 1 | 2 |
 *
 * @note    Coefficients are represented as float numbers
 */
/*****************************************************************************/
typedef struct Cam1x3FloatMatrix_s
{
    float fCoeff[3];
} Cam1x3FloatMatrix_t;



/*****************************************************************************/
/**
 * @brief   Matrix coefficients
 *
 *          | 0 | 1 | ... | 4 |
 *
 * @note    Coefficients are represented as float numbers
 */
/*****************************************************************************/
typedef struct Cam1x4FloatMatrix_s
{
    float fCoeff[4];
} Cam1x4FloatMatrix_t;



/*****************************************************************************/
/**
 * @brief   Matrix coefficients
 *
 *          | 0 | 1 | ... | 6 |
 *
 * @note    Coefficients are represented as float numbers
 */
/*****************************************************************************/
typedef struct Cam1x6FloatMatrix_s
{
    float fCoeff[6];
} Cam1x6FloatMatrix_t;



/*****************************************************************************/
/**
 * @brief   Matrix coefficients
 *
 *          | 0 | 1 | ... | 15 |
 *
 * @note    Coefficients are represented as float numbers
 */
/*****************************************************************************/
typedef struct Cam1x16FloatMatrix_s
{
    float fCoeff[16];
} Cam1x16FloatMatrix_t;



/*****************************************************************************/
/**
 * @brief   Matrix coefficients
 *
 *          | 0 | 1 |
 *
 * @note    Coefficients are represented as float numbers
 */
/*****************************************************************************/
typedef struct Cam2x1FloatMatrix
{
    float fCoeff[2];
} Cam2x1FloatMatrix_t;



/*****************************************************************************/
/**
 * @brief   Matrix coefficients
 *
 *          | 0 | 1 |
 *          | 2 | 3 |
 *
 * @note    Coefficients are represented as float numbers
 */
/*****************************************************************************/
typedef struct Cam2x2FloatMatrix
{
    float fCoeff[4];
} Cam2x2FloatMatrix_t;



/*****************************************************************************/
/**
 * @brief   Matrix coefficients
 *
 *          | 0 | 1 |  2 |
 *
 * @note    Coefficients are represented as float numbers
 */
/*****************************************************************************/
typedef struct Cam3x1FloatMatrix
{
    float fCoeff[3];
} Cam3x1FloatMatrix_t;



/*****************************************************************************/
/**
 * @brief   Matrix coefficients
 *
 *          | 0 | 1 |  2 |
 *          | 3 | 4 |  5 |
 *
 * @note    Coefficients are represented as float numbers
 */
/*****************************************************************************/
typedef struct Cam3x2FloatMatrix_s
{
    float fCoeff[6];
} Cam3x2FloatMatrix_t;



/*****************************************************************************/
/**
 * @brief   Matrix coefficients
 *
 *          | 0 | 1 |  2 |
 *          | 3 | 4 |  5 |
 *          | 6 | 7 |  8 |
 *
 * @note    Coefficients are represented as float numbers
 */
/*****************************************************************************/
typedef struct Cam3x3FloatMatrix_s
{
    float fCoeff[9];
} Cam3x3FloatMatrix_t;


/******************************************************************************/
/**
 * @brief   A structure to represent a 5x5 matrix.
 *
 *          The 25 values are laid out as follows (zero based index):
 *
 *               | 00 01 02 03 04 | \n
 *               | 05 06 07 08 09 | \n
 *               | 10 11 12 13 14 | \n
 *               | 15 16 17 18 19 | \n
 *               | 20 21 22 23 24 | \n
 *         
 * @note    The 25 values are represented as float numbers.
 *
 *****************************************************************************/
typedef struct Cam5x5FloatMatrix_s
{
    float fCoeff[25U];              /**< array of 5x5 float values */
} Cam5x5FloatMatrix_t;


/*****************************************************************************/
/**
 * @brief   Matrix coefficients
 *
 *          |   0 |   1 |   2 |   3 |   4 |   5 |   6 |   7 | ....
 *          |  17 |  18 |  19 |  20 |  21 |  22 |  23 |  24 | ....
 *          |  34 |  35 |  36 |  37 |  38 |  39 |  40 |  41 | ....
 *          ...
 *          ...
 *          ...
 *          | 271 | 272 | 273 | 274 | 275 | 276 | 277 | 278 | .... | 288 |
 *
 * @note    Coefficients are represented as short numbers
 */
/*****************************************************************************/
typedef struct Cam17x17FloatMatrix_s
{
    float fCoeff[17 * 17];
} Cam17x17FloatMatrix_t;



/*****************************************************************************/
/**
 * @brief   Matrix coefficients
 *
 *          | 0 | 1 | 2 |
 *
 * @note    Coefficients are represented as short numbers
 */
/*****************************************************************************/
typedef struct Cam1x3ShortMatrix_s
{
    int16_t Coeff[3];
} Cam1x3ShortMatrix_t;



/*****************************************************************************/
/**
 * @brief   Matrix coefficients
 *
 *          | 0 | 1 | 2 | ... | 4 |
 *
 * @note    Coefficients are represented as short numbers
 */
/*****************************************************************************/
typedef struct Cam1x4UShortMatrix_s
{
    uint16_t uCoeff[4];
} Cam1x4UShortMatrix_t;



/*****************************************************************************/
/**
 * @brief   Matrix coefficients
 *
 *          | 0 | 1 | 2 | ... | 16 |
 *
 * @note    Coefficients are represented as short numbers
 */
/*****************************************************************************/
typedef struct Cam1x17UShortMatrix_s
{
    uint16_t uCoeff[17];
} Cam1x17UShortMatrix_t;



/*****************************************************************************/
/**
 * @brief   Matrix coefficients
 *
 *          |   0 |   1 |   2 |   3 |   4 |   5 |   6 |   7 | ....
 *          |  17 |  18 |  19 |  20 |  21 |  22 |  23 |  24 | ....
 *          |  34 |  35 |  36 |  37 |  38 |  39 |  40 |  41 | ....
 *          ...
 *          ...
 *          ...
 *          | 271 | 272 | 273 | 274 | 275 | 276 | 277 | 278 | .... | 288 |
 *
 * @note    Coefficients are represented as short numbers
 */
/*****************************************************************************/
typedef struct Cam17x17UShortMatrix_s
{
    uint16_t uCoeff[17 * 17];
} Cam17x17UShortMatrix_t;



/*****************************************************************************/
/**
 * @brief   name/identifier of a resolution
 */
/*****************************************************************************/
#define CAM_RESOLUTION_NAME         ( 15U )
typedef char                        CamResolutionName_t[CAM_RESOLUTION_NAME];


/*****************************************************************************/
/**
 * @brief   name/identifier of a resolution
 */
/*****************************************************************************/
#define CAM_FRAMERATE_NAME          ( 25U )
typedef char                        CamFramerateName_t[CAM_FRAMERATE_NAME];



/*****************************************************************************/
/**
 * @brief   name/identifier of an illumination
 */
/*****************************************************************************/
#define CAM_BLS_PROFILE_NAME        ( 10U )
typedef char                        CamBlsProfileName_t[CAM_BLS_PROFILE_NAME];


/*****************************************************************************/
/**
 * @brief   name/identifier of an illumination
 */
/*****************************************************************************/
#define CAM_ILLUMINATION_NAME       ( 20U )
typedef char                        CamIlluminationName_t[CAM_ILLUMINATION_NAME];


/*****************************************************************************/
/**
 * @brief   name/identifier of a lense shade correction profile (LscProfile)
 */
/*****************************************************************************/
#define CAM_LSC_PROFILE_NAME        ( 25U )
typedef char                        CamLscProfileName_t[CAM_LSC_PROFILE_NAME];


/*****************************************************************************/
/**
 * @brief   name/identifier of a color correction profile (CcProfile)
 */
/*****************************************************************************/
#define CAM_CC_PROFILE_NAME         ( 20U )
typedef char                        CamCcProfileName_t[CAM_CC_PROFILE_NAME];


/*****************************************************************************/
/**
 * @brief   name/identifier of a auto white balance profile (AwbProfile)
 */
/*****************************************************************************/
#define CAM_AWB_PROFILE_NAME        ( 20U )
typedef char                        CamAwbProfileName_t[CAM_AWB_PROFILE_NAME];


/*****************************************************************************/
/**
 * @brief   name/identifier of an exposure scheme (EcmScheme)
 */
/*****************************************************************************/
#define CAM_ECM_SCHEME_NAME         ( 20U )
typedef char                        CamEcmSchemeName_t[CAM_ECM_SCHEME_NAME];


/*****************************************************************************/
/**
 * @brief   name/identifier of an exposure profile (EcmProfile)
 */
/*****************************************************************************/
#define CAM_ECM_PROFILE_NAME        ( 20U )
typedef char                        CamEcmProfileName_t[CAM_ECM_PROFILE_NAME];


/*****************************************************************************/
/**
 * @brief   name/identifier of a chromatic abberation correction profile
 *          (CacProfile)
 */
/*****************************************************************************/
#define CAM_CAC_PROFILE_NAME        ( 20U )
typedef char                        CamCacProfileName_t[CAM_CAC_PROFILE_NAME];


/*****************************************************************************/
/**
 * @brief   name/identifier of a denoising prefilter profile (DpfProfile)
 */
/*****************************************************************************/
#define CAM_DPF_PROFILE_NAME        ( 20U )
typedef char                        CamDpfProfileName_t[CAM_DPF_PROFILE_NAME];


/*****************************************************************************/
/**
 * @brief   name/identifier of a denoising prefilter profile (DpccProfile)
 */
/*****************************************************************************/
#define CAM_DPCC_PROFILE_NAME       ( 20U )
typedef char                        CamDpccProfileName_t[CAM_DPCC_PROFILE_NAME];


/*****************************************************************************/
/**
 * @brief   framerate profile
 */
/*****************************************************************************/
typedef struct CamFrameRate_s
{
    void                *p_next;            /**< for adding to a list */

    CamFramerateName_t  name;               /**< name of framerate scheme */
    float               fps;                /**< framerate */
} CamFrameRate_t;


/*****************************************************************************/
/**
 * @brief   resolution profile
 */
/*****************************************************************************/
typedef struct CamResolution_t
{
    void                *p_next;            /**< for adding to a list */

    CamResolutionName_t name;               /**< desctriptor */
    uint16_t            width;              /**< resolution width */
    uint16_t            height;             /**< resolution height */
    uint32_t            id;                 /**< bitmask identifier */

    List                framerates;         /**< list of framerates */
} CamResolution_t;



/*****************************************************************************/
/**
 * @brief   parameters for a sensorgain to saturation interpolation
 */
/*****************************************************************************/
typedef struct CamSaturationCurve_s
{
    uint16_t    ArraySize;
    float       *pSensorGain;
    float       *pSaturation;
} CamSaturationCurve_t;



/*****************************************************************************/
/**
 * @brief   parameters for a sensorgain to vignetting (compensation)
 *          interpolation
 */
/*****************************************************************************/
typedef struct CamVignettingCurve_s
{
    uint16_t    ArraySize;
    float       *pSensorGain;
    float       *pVignetting;
} CamVignettingCurve_t;



/*****************************************************************************/
/**
 * @brief   parameters for a sensorgain to vignetting (compensation)
 *          interpolation
 *
 */
/*****************************************************************************/
typedef struct CamLscMatrix_s
{
    Cam17x17UShortMatrix_t  LscMatrix[CAM_4CH_COLOR_COMPONENT_MAX];
} CamLscMatrix_t;



/*****************************************************************************/
/**
 * @brief   BLS calibration structure
 */
/*****************************************************************************/
typedef struct CamBlsProfile_s
{
    void                        *p_next;                /**< for adding to a list */

    CamBlsProfileName_t         name;                   /**< profile name */
    CamResolutionName_t         resolution;             /**< resolution link */

    Cam1x4UShortMatrix_t        level;                  /**< black level for all 4 color components */
} CamBlsProfile_t;



/*****************************************************************************/
/**
 * @brief   Illumination specific structure.
 */
/*****************************************************************************/
typedef struct CamIlluProfile_s
{
    void                        *p_next;                /**< for adding to a list */

    CamIlluminationName_t       name;                   /**< name of the illumination profile (i.e. "D65", "A", ... )*/
    uint32_t                    id;                     /**< unique id */

    CamDoorType_t               DoorType;               /**< indoor or outdoor profile */
    CamAwbType_t                AwbType;                /**< manual or auto profile */

    /* for manual white balance data */
    Cam3x3FloatMatrix_t         CrossTalkCoeff;         /**< CrossTalk matrix coefficients */
    Cam1x3FloatMatrix_t         CrossTalkOffset;        /**< CrossTalk offsets */
    Cam1x4FloatMatrix_t         ComponentGain;          /**< White Balance Gains*/

    /* gaussian mixture modell */
    Cam2x1FloatMatrix_t         GaussMeanValue;         /**< */
    Cam2x2FloatMatrix_t         CovarianceMatrix;       /**< */
    Cam1x1FloatMatrix_t         GaussFactor;            /**< */
    Cam2x1FloatMatrix_t         Threshold;              /**< */

    /* adaptive color correctio */
    CamSaturationCurve_t        SaturationCurve;        /**< stauration over gain curve */

    /* adative lense shade correction */
    CamVignettingCurve_t        VignettingCurve;        /**< vignetting over gain curve */

    #define CAM_NO_CC_PROFILES  ( 10 )                  /**< max number of cc-profiles per illumination */
    int32_t                     cc_no;
    CamCcProfileName_t          cc_profiles[CAM_NO_CC_PROFILES];

    #define CAM_NO_LSC_PROFILES ( 5 )
    #define CAM_NO_RESOLUTIONS  ( 4 )
    int32_t                     lsc_res_no;
    int32_t                     lsc_no[CAM_NO_RESOLUTIONS];
    CamLscProfileName_t         lsc_profiles[CAM_NO_RESOLUTIONS][CAM_NO_LSC_PROFILES];
} CamIlluProfile_t;



/*****************************************************************************/
/**
 * @brief   LSC profile
 */
/*****************************************************************************/
typedef struct CamLscProfile_s
{
    void                    *p_next;                                /**< for adding to a list */

    CamLscProfileName_t     name;                                   /**< profile name */
    CamResolutionName_t     resolution;                             /**< resolution link */
    CamIlluminationName_t   illumination;                           /**< illumination link */
    float                   vignetting;                             /**< vignetting value */

    uint16_t                LscSectors;
    uint16_t                LscNo;
    uint16_t                LscXo;
    uint16_t                LscYo;

    uint16_t                LscXGradTbl[8];
    uint16_t                LscYGradTbl[8];
    uint16_t                LscXSizeTbl[8];
    uint16_t                LscYSizeTbl[8];

    Cam17x17UShortMatrix_t  LscMatrix[CAM_4CH_COLOR_COMPONENT_MAX];     /**< matrix for different color channels */
} CamLscProfile_t;



/*****************************************************************************/
/**
 * @brief   CC profile
 *
 */
/*****************************************************************************/
typedef struct CamCcProfile_s
{
    void                    *p_next;                /**< for adding to a list */

    CamCcProfileName_t      name;                   /**< profile name */
    float                   saturation;             /**< saturation value */

    Cam3x3FloatMatrix_t     CrossTalkCoeff;         /**< CrossTalk matrix coefficients */
    Cam1x3FloatMatrix_t     CrossTalkOffset;        /**< CrossTalk offsets */
    Cam1x4FloatMatrix_t     ComponentGain;          /**< White Balance Gains*/
} CamCcProfile_t;



/*****************************************************************************/
/**
 * @brief   CAC calibration structure
 */
/*****************************************************************************/
typedef struct CamCacProfile_s
{
    void                    *p_next;            /**< for adding to a list */

    CamCacProfileName_t     name;               /**< profile name */
    CamResolutionName_t     resolution;         /**< resolution link */

    uint8_t                 x_ns;               /**< horizontal normalization shift */
    uint8_t                 x_nf;               /**< horizontal normalization factor */
    uint8_t                 y_ns;               /**< vertical normalization shift */
    uint8_t                 y_nf;               /**< vertical normalization factor */

    Cam1x3FloatMatrix_t     Red;                /**< coeffciencts A, B and C for red */
    Cam1x3FloatMatrix_t     Blue;               /**< coeffciencts A, B and C for blue */

    int16_t                 hCenterOffset;
    int16_t                 vCenterOffset;
} CamCacProfile_t;

/**
 * @brief   Enumeration type to configure de-noising level.
 *
 */
typedef enum CamerIcIspFltDeNoiseLevel_e
{
    CAMERIC_ISP_FLT_DENOISE_LEVEL_INVALID   = 0,
    CAMERIC_ISP_FLT_DENOISE_LEVEL_0         = 1,
    CAMERIC_ISP_FLT_DENOISE_LEVEL_1         = 2,
    CAMERIC_ISP_FLT_DENOISE_LEVEL_2         = 3,
    CAMERIC_ISP_FLT_DENOISE_LEVEL_3         = 4,
    CAMERIC_ISP_FLT_DENOISE_LEVEL_4         = 5,
    CAMERIC_ISP_FLT_DENOISE_LEVEL_5         = 6,
    CAMERIC_ISP_FLT_DENOISE_LEVEL_6         = 7,
    CAMERIC_ISP_FLT_DENOISE_LEVEL_7         = 8,
    CAMERIC_ISP_FLT_DENOISE_LEVEL_8         = 9,
    CAMERIC_ISP_FLT_DENOISE_LEVEL_9         = 10,
    CAMERIC_ISP_FLT_DENOISE_LEVEL_10        = 11,
    CAMERIC_ISP_FLT_DENOISE_LEVEL_TEST      = 12,
    CAMERIC_ISP_FLT_DENOISE_LEVEL_MAX
} CamerIcIspFltDeNoiseLevel_t;



/**
 * @brief   Enumeration type to configure sharpening level.
 *
 */
typedef enum CamerIcIspFltSharpeningLevel_e
{
    CAMERIC_ISP_FLT_SHARPENING_LEVEL_INVALID   = 0,
    CAMERIC_ISP_FLT_SHARPENING_LEVEL_0         = 1,
    CAMERIC_ISP_FLT_SHARPENING_LEVEL_1         = 2,
    CAMERIC_ISP_FLT_SHARPENING_LEVEL_2         = 3,
    CAMERIC_ISP_FLT_SHARPENING_LEVEL_3         = 4,
    CAMERIC_ISP_FLT_SHARPENING_LEVEL_4         = 5,
    CAMERIC_ISP_FLT_SHARPENING_LEVEL_5         = 6,
    CAMERIC_ISP_FLT_SHARPENING_LEVEL_6         = 7,
    CAMERIC_ISP_FLT_SHARPENING_LEVEL_7         = 8,
    CAMERIC_ISP_FLT_SHARPENING_LEVEL_8         = 9,
    CAMERIC_ISP_FLT_SHARPENING_LEVEL_9         = 10,
    CAMERIC_ISP_FLT_SHARPENING_LEVEL_10        = 11,
    CAMERIC_ISP_FLT_SHARPENING_LEVEL_MAX
} CamerIcIspFltSharpeningLevel_t;


/*****************************************************************************/
/**
 * @brief   parameters for a sensorgain to saturation interpolation
 */
/*****************************************************************************/
typedef struct CamDenoiseLevelCurve_s
{
    uint16_t    ArraySize;
    float       *pSensorGain;
    CamerIcIspFltDeNoiseLevel_t        *pDlevel;
} CamDenoiseLevelCurve_t;

/*****************************************************************************/
/**
 * @brief   parameters for a sensorgain to saturation interpolation
 */
/*****************************************************************************/
typedef struct CamSharpeningLevelCurve_s
{
    uint16_t    ArraySize;
    float       *pSensorGain;
    CamerIcIspFltSharpeningLevel_t      *pSlevel;
} CamSharpeningLevelCurve_t;

/*****************************************************************************/
/**
 * @brief   MFD calibration structure
 */
/*****************************************************************************/
typedef struct CamMfdProfile_s
{
	uint8_t					enable;			/**< mfd enable*/
	float 					gain[3];		/**< mfd gain */
	float 					frames[3];		/**< mfd frames */
} CamMfdProfile_t;

/*****************************************************************************/
/**
 * @brief   UVNR calibration structure
 */
/*****************************************************************************/
typedef struct CamUvnrProfile_s
{
	uint8_t					enable;			/**< uvnr enable*/
	float 					gain[3];		/**< uvnr gain */
	float 					ratio[3];		/**< uvnr ratio */
	float 					distances[3];	/**< uvnr distances */
} CamUvnrProfile_t;

/*****************************************************************************/
typedef struct CamFilterLevelRegConf_s {
  uint8_t FiltLevelRegConfEnable;
  uint8_t* p_FiltLevel;
  uint8_t  ArraySize;
  uint8_t* p_grn_stage1;    /* ISP_FILT_MODE register fields*/
  uint8_t  grn_stage1_ArraySize;
  uint8_t* p_chr_h_mode;    /* ISP_FILT_MODE register fields*/
  uint8_t  chr_h_mode_ArraySize;
  uint8_t* p_chr_v_mode;    /* ISP_FILT_MODE register fields*/
  uint8_t  chr_v_mode_ArraySize;
  uint32_t*  p_thresh_bl0;
  uint8_t  thresh_bl0_ArraySize;
  uint32_t*  p_thresh_bl1;
  uint8_t  thresh_bl1_ArraySize;
  uint32_t*  p_thresh_sh0;
  uint8_t  thresh_sh0_ArraySize;
  uint32_t*  p_thresh_sh1;
  uint8_t  thresh_sh1_ArraySize;
  uint32_t*  p_fac_sh1;
  uint8_t  fac_sh1_ArraySize;
  uint32_t*  p_fac_sh0;
  uint8_t  fac_sh0_ArraySize;
  uint32_t*  p_fac_mid;
  uint8_t  fac_mid_ArraySize;
  uint32_t*  p_fac_bl0;
  uint8_t  fac_bl0_ArraySize;
  uint32_t*  p_fac_bl1;
  uint8_t  fac_bl1_ArraySize;
} CamFilterLevelRegConf_t;


/*****************************************************************************/
/**
 * @brief   DPF calibration structure
 */
/*****************************************************************************/
typedef struct CamDpfProfile_s
{
    void                    *p_next;            /**< for adding to a list */

    CamDpfProfileName_t     name;               /**< profile name */
    CamResolutionName_t     resolution;         /**< resolution link */

    uint16_t                nll_segmentation;
    Cam1x17UShortMatrix_t   nll_coeff;

    uint16_t                SigmaGreen;         /**< */
    uint16_t                SigmaRedBlue;       /**< */
    float                   fGradient;          /**< */
    float                   fOffset;            /**< */
    Cam1x4FloatMatrix_t     NfGains;            /**< */

	CamMfdProfile_t			Mfd;			    /**< mfd struct*/
	CamUvnrProfile_t		Uvnr;			    /**< uvnr struct*/
    CamDenoiseLevelCurve_t        DenoiseLevelCurve;
    CamSharpeningLevelCurve_t     SharpeningLevelCurve;
	float FilterEnable;
	CamFilterLevelRegConf_t FiltLevelRegConf;
} CamDpfProfile_t;



/*****************************************************************************/
/**
 * @brief   DPCC calibration structure
 */
/*****************************************************************************/
typedef struct CamDpccProfile_s
{
    void                    *p_next;            /**< for adding to a list */

    CamDpccProfileName_t    name;               /**< profile name */
    CamResolutionName_t     resolution;         /**< resolution link */

    uint32_t                isp_dpcc_mode;
    uint32_t                isp_dpcc_output_mode;
    uint32_t                isp_dpcc_set_use;
    uint32_t                isp_dpcc_methods_set_1;
    uint32_t                isp_dpcc_methods_set_2;
    uint32_t                isp_dpcc_methods_set_3;
    uint32_t                isp_dpcc_line_thresh_1;
    uint32_t                isp_dpcc_line_mad_fac_1;
    uint32_t                isp_dpcc_pg_fac_1;
    uint32_t                isp_dpcc_rnd_thresh_1;
    uint32_t                isp_dpcc_rg_fac_1;
    uint32_t                isp_dpcc_line_thresh_2;
    uint32_t                isp_dpcc_line_mad_fac_2;
    uint32_t                isp_dpcc_pg_fac_2;
    uint32_t                isp_dpcc_rnd_thresh_2;
    uint32_t                isp_dpcc_rg_fac_2;
    uint32_t                isp_dpcc_line_thresh_3;
    uint32_t                isp_dpcc_line_mad_fac_3;
    uint32_t                isp_dpcc_pg_fac_3;
    uint32_t                isp_dpcc_rnd_thresh_3;
    uint32_t                isp_dpcc_rg_fac_3;
    uint32_t                isp_dpcc_ro_limits;
    uint32_t                isp_dpcc_rnd_offs;
} CamDpccProfile_t;



/*****************************************************************************/
/**
 * @brief   Contains pointers to parameter arrays for Rg/Bg color space
 *          clipping
 */
/*****************************************************************************/
typedef struct CamAwbClipParm_s
{
    float       *pRg1;
    float       *pMaxDist1;
    float       *pRg2;
    float       *pMaxDist2;
    uint16_t    ArraySize1;
    uint16_t    ArraySize2;
} CamAwbClipParm_t;



/*****************************************************************************/
/**
 * @brief   Contains pointers to parameter arrays for AWB out of range
 *          handling
 *
 */
/*****************************************************************************/
typedef struct CamAwbGlobalFadeParm_s
{
    float       *pGlobalFade1;
    float       *pGlobalGainDistance1;
    float       *pGlobalFade2;
    float       *pGlobalGainDistance2;
    uint16_t    ArraySize1;
    uint16_t    ArraySize2;
} CamAwbGlobalFadeParm_t;



/*****************************************************************************/
/**
 * @brief   Contains pointers to parameter arrays for near white pixel
 *          parameter calculations
 */
/*****************************************************************************/
typedef struct CamAwbFade2Parm_s
{
    float*      pFade;
    float*      pCbMinRegionMax;
    float*      pCrMinRegionMax;
    float*      pMaxCSumRegionMax;
    float*      pCbMinRegionMin;
    float*      pCrMinRegionMin;
    float*      pMaxCSumRegionMin;
	float*		pMinCRegionMax;
	float*    	pMinCRegionMin;
	float*		pMaxYRegionMax;
	float*		pMaxYRegionMin;
	float*		pMinYMaxGRegionMax;
	float*		pMinYMaxGRegionMin;
	float*  	pRefCb;
	float*		pRefCr;
	uint16_t	regionAdjustEnable;
    uint16_t    ArraySize;
} CamAwbFade2Parm_t;



/*****************************************************************************/
/**
 *          IsiLine_t
 *
 * @brief   Contains parameters for a straight line in Hesse normal form in
 *          Rg/Bg colorspace
 *
 */
/*****************************************************************************/
typedef struct CamCenterLine_s
{
    float f_N0_Rg;                                  /**< Rg component of normal vector */
    float f_N0_Bg;                                  /**< Bg component of normal vector */
    float f_d;                                      /**< Distance of normal vector     */
} CamCenterLine_t;



/*****************************************************************************/
/**
 * @brief   Global AWB IIR Filter
 */
/*****************************************************************************/
typedef struct CamCalibIIR_s
{
    float       fIIRDampCoefAdd;                        /**< incrementer of damping coefficient */
    float       fIIRDampCoefSub;                        /**< decrementer of damping coefficient */
    float       fIIRDampFilterThreshold;                /**< threshold for incrementing or decrementing of damping coefficient */

    float       fIIRDampingCoefMin;                     /**< minmuim value of damping coefficient */
    float       fIIRDampingCoefMax;                     /**< maximum value of damping coefficient */
    float       fIIRDampingCoefInit;                    /**< initial value of damping coefficient */

    uint16_t    IIRFilterSize;                          /**< number of filter items */
    float       fIIRFilterInitValue;                    /**< initial value of the filter items */
} CamCalibIIR_t;



/*****************************************************************************/
/**
 * @brief   Global AWB calibration structure
 */
/*****************************************************************************/
typedef struct CamCalibAwbGlobal_s
{
    void                    *p_next;                    /**< for adding to a list */

    CamAwbProfileName_t     name;                       /**< profile name */
    CamResolutionName_t     resolution;                 /**< resolution link */

    Cam3x1FloatMatrix_t     SVDMeanValue;
    Cam3x2FloatMatrix_t     PCAMatrix;
    CamCenterLine_t         CenterLine;
    Cam1x1FloatMatrix_t     KFactor;

    CamAwbClipParm_t        AwbClipParam;               /**< clipping parameter in Rg/Bg space */
    CamAwbGlobalFadeParm_t  AwbGlobalFadeParm;
    CamAwbFade2Parm_t       AwbFade2Parm;

    float                   fRgProjIndoorMin;
    float                   fRgProjOutdoorMin;
    float                   fRgProjMax;
    float                   fRgProjMaxSky;

	float 					fRgProjALimit;    //oyyf
	float					fRgProjAWeight;		//oyyf
	float 					fRgProjYellowLimit;		//oyyf
	float					fRgProjIllToCwf;		//oyyf
	float					fRgProjIllToCwfWeight;	//oyyf

    CamIlluminationName_t   outdoor_clipping_profile;

    float                   fRegionSize;
    float                   fRegionSizeInc;
    float                   fRegionSizeDec;
    float                   awbMeasWinWidthScale;
    float                   awbMeasWinHeightScale;

	float 					fExppriorOutdoorSwitchOff;
    CamCalibIIR_t           IIR;
} CamCalibAwbGlobal_t;



/*****************************************************************************/
/**
 * @brief   ECM scheme
 */
/*****************************************************************************/
typedef struct CamEcmScheme_s
{
    void                    *p_next;                    /**< for adding to a list */

    CamEcmSchemeName_t      name;                       /**< scheme name */

    float                   OffsetT0Fac;                /**< start flicker avoidance above OffsetT0Fac * Tflicker integration time */
    float                   SlopeA0;                    /**< slope of gain */
} CamEcmScheme_t;



/*****************************************************************************/
/**
 * @brief   ECM profile
 */
/*****************************************************************************/
typedef struct CamEcmProfile_s
{
    void                    *p_next;                    /**< for adding to a list */

    CamEcmProfileName_t     name;                       /**< profile name => serves as resolution link as well */

    List                    ecm_scheme;                 /**< list of ECM schemes; at least one item is expected */
} CamEcmProfile_t;

/*****************************************************************************/
/**
 * @brief   Matrix coefficients
 *
 *          | 0 | 1 |  2 |3| 4 |  5 |
 *
 * @note    Coefficients are represented as float numbers
 */
/*****************************************************************************/
typedef struct Cam6x1FloatMatrix
{
	float fCoeff[6];
} Cam6x1FloatMatrix_t;

typedef struct CamAECGridWeight_s
{
	uint8_t   *pWeight;
	uint16_t  ArraySize;
}CamAECGridWeight_t;

/*****************************************************************************/
/**
 * @brief   Global AEC calibration structure
 */
/*****************************************************************************/
typedef struct CamCalibAecGlobal_s
{
    float                   SetPoint;                   /**< set point to hit by the ae control system */
    float                   ClmTolerance;
    float                   DampOverStill;              /**< damping coefficient for still image mode */
    float                   DampUnderStill;             /**< damping coefficient for still image mode */
    float                   DampOverVideo;              /**< damping coefficient for video mode */
    float                   DampUnderVideo;             /**< damping coefficient for video mode */
    float                   AfpsMaxGain;
    CamAECGridWeight_t 		GridWeights;//cxf
    float					EcmDotEnable;
    Cam6x1FloatMatrix_t     EcmTimeDot;
    Cam6x1FloatMatrix_t     EcmGainDot;
    float                   MeasuringWinWidthScale;//cxf
    float                   MeasuringWinHeightScale;//cxf
} CamCalibAecGlobal_t;

/******************************************************************************/
/**
 * @brief   Enumeration type for x scaling of the gamma curve 
 *
 * @note    This structure needs to be converted to driver structure
 *
 *****************************************************************************/
typedef enum CamEngineGammaOutXScale_e
{
    CAM_ENGINE_GAMMAOUT_XSCALE_INVALID  = 0,    /**< lower border (only for an internal evaluation) */
    CAM_ENGINE_GAMMAOUT_XSCALE_LOG      = 1,    /**< logarithmic segmentation from 0 to 4095 
                                                     (64,64,64,64,128,128,128,128,256,256,256,512,512,512,512,512) */
    CAM_ENGINE_GAMMAOUT_XSCALE_EQU      = 2,    /**< equidistant segmentation from 0 to 4095
                                                     (256, 256, ... ) */
    CAM_ENGINE_GAMMAOUT_XSCALE_MAX              /**< upper border (only for an internal evaluation) */
} CamEngineGammaOutXScale_t;

 /******************************************************************************/
 /**
 * @brief   This macro defines the number of elements in a gamma-curve.
 *
 *****************************************************************************/
#define CAMERIC_ISP_GAMMA_CURVE_SIZE        17
/* @endcond */


/******************************************************************************/
/**
 * @brief   Structure to configure the gamma curve.
 *
 * @note    This structure needs to be converted to driver structure
 *
 *****************************************************************************/
typedef struct CamEngineGammaOutCurve_s
{
    CamEngineGammaOutXScale_t   xScale;
    uint16_t                    GammaY[CAMERIC_ISP_GAMMA_CURVE_SIZE];
} CamEngineGammaOutCurve_t;



/*****************************************************************************/
/**
 * @brief   Gamma calibration structure
 */
/*****************************************************************************/
typedef struct CamCalibGammaOut_s
{
	CamEngineGammaOutCurve_t Curve;

} CamCalibGammaOut_t;


/*****************************************************************************/
/**
 * @brief   System data structure.
 */
/*****************************************************************************/
typedef struct CamCalibSystemData_s
{
    bool_t                  AfpsDefault;
} CamCalibSystemData_t;



#ifdef __cplusplus
}
#endif

/* @} cam_types */

#endif /* __CAM_TYPES_H__ */

