//ov8825 the same with ov14825

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
 * @file GC2155.c
 *
 * @brief
 *   ADD_DESCRIPTION_HERE
 *
 *****************************************************************************/
#include <ebase/types.h>
#include <ebase/trace.h>
#include <ebase/builtins.h>

#include <common/return_codes.h>
#include <common/misc.h>

#include "isi.h"
#include "isi_iss.h"
#include "isi_priv.h"

#include "GC2155_priv.h"


#define CC_OFFSET_SCALING  2.0f
#define I2C_COMPLIANT_STARTBIT 1U

/******************************************************************************
 * local macro definitions
 *****************************************************************************/
CREATE_TRACER( GC2155_INFO , "GC2155: ", INFO,    1U );
CREATE_TRACER( GC2155_WARN , "GC2155: ", WARNING, 1U );
CREATE_TRACER( GC2155_ERROR, "GC2155: ", ERROR,   1U );

CREATE_TRACER( GC2155_DEBUG, "GC2155: ", INFO,     1U );

CREATE_TRACER( GC2155_REG_INFO , "GC2155: ", INFO, 1);
CREATE_TRACER( GC2155_REG_DEBUG, "GC2155: ", INFO, 0U );

#define GC2155_SLAVE_ADDR       0x78U                           /**< i2c slave address of the GC2155 camera sensor */
#define GC2155_SLAVE_AF_ADDR    0x00U                           /**< i2c slave address of the GC2155 integrated AD5820 */



/******************************************************************************
 * local variable declarations
 *****************************************************************************/
const char GC2155_g_acName[] = "GC2155_ SOC_PARREL";
extern const IsiRegDescription_t GC2155_g_aRegDescription[];
extern const IsiRegDescription_t GC2155_g_svga[];
extern const IsiRegDescription_t GC2155_g_1600x1200[];


const IsiSensorCaps_t GC2155_g_IsiSensorDefaultConfig;


#define GC2155_I2C_START_BIT        (I2C_COMPLIANT_STARTBIT)    // I2C bus start condition
#define GC2155_I2C_NR_ADR_BYTES     (1U)                        // 1 byte base address and 2 bytes sub address
#define GC2155_I2C_NR_DAT_BYTES     (1U)                        // 8 bit registers


/******************************************************************************
 * local function prototypes
 *****************************************************************************/
static RESULT GC2155_IsiCreateSensorIss( IsiSensorInstanceConfig_t *pConfig );
static RESULT GC2155_IsiReleaseSensorIss( IsiSensorHandle_t handle );
static RESULT GC2155_IsiGetCapsIss( IsiSensorHandle_t handle, IsiSensorCaps_t *pIsiSensorCaps );
static RESULT GC2155_IsiSetupSensorIss( IsiSensorHandle_t handle, const IsiSensorConfig_t *pConfig );
static RESULT GC2155_IsiSensorSetStreamingIss( IsiSensorHandle_t handle, bool_t on );
static RESULT GC2155_IsiSensorSetPowerIss( IsiSensorHandle_t handle, bool_t on );
static RESULT GC2155_IsiCheckSensorConnectionIss( IsiSensorHandle_t handle );
static RESULT GC2155_IsiGetSensorRevisionIss( IsiSensorHandle_t handle, uint32_t *p_value);

static RESULT GC2155_IsiGetGainLimitsIss( IsiSensorHandle_t handle, float *pMinGain, float *pMaxGain);
static RESULT GC2155_IsiGetIntegrationTimeLimitsIss( IsiSensorHandle_t handle, float *pMinIntegrationTime, float *pMaxIntegrationTime );
static RESULT GC2155_IsiExposureControlIss( IsiSensorHandle_t handle, float NewGain, float NewIntegrationTime, uint8_t *pNumberOfFramesToSkip, float *pSetGain, float *pSetIntegrationTime );
static RESULT GC2155_IsiGetCurrentExposureIss( IsiSensorHandle_t handle, float *pSetGain, float *pSetIntegrationTime );
static RESULT GC2155_IsiGetAfpsInfoIss ( IsiSensorHandle_t handle, uint32_t Resolution, IsiAfpsInfo_t* pAfpsInfo);
static RESULT GC2155_IsiGetGainIss( IsiSensorHandle_t handle, float *pSetGain );
static RESULT GC2155_IsiGetGainIncrementIss( IsiSensorHandle_t handle, float *pIncr );
static RESULT GC2155_IsiSetGainIss( IsiSensorHandle_t handle, float NewGain, float *pSetGain );
static RESULT GC2155_IsiGetIntegrationTimeIss( IsiSensorHandle_t handle, float *pSetIntegrationTime );
static RESULT GC2155_IsiGetIntegrationTimeIncrementIss( IsiSensorHandle_t handle, float *pIncr );
static RESULT GC2155_IsiSetIntegrationTimeIss( IsiSensorHandle_t handle, float NewIntegrationTime, float *pSetIntegrationTime, uint8_t *pNumberOfFramesToSkip );
static RESULT GC2155_IsiGetResolutionIss( IsiSensorHandle_t handle, uint32_t *pSetResolution );


static RESULT GC2155_IsiRegReadIss( IsiSensorHandle_t handle, const uint32_t address, uint32_t *p_value );
static RESULT GC2155_IsiRegWriteIss( IsiSensorHandle_t handle, const uint32_t address, const uint32_t value );

static RESULT GC2155_IsiGetCalibKFactor( IsiSensorHandle_t handle, Isi1x1FloatMatrix_t **pIsiKFactor );
static RESULT GC2155_IsiGetCalibPcaMatrix( IsiSensorHandle_t   handle, Isi3x2FloatMatrix_t **pIsiPcaMatrix );
static RESULT GC2155_IsiGetCalibSvdMeanValue( IsiSensorHandle_t   handle, Isi3x1FloatMatrix_t **pIsiSvdMeanValue );
static RESULT GC2155_IsiGetCalibCenterLine( IsiSensorHandle_t   handle, IsiLine_t  **ptIsiCenterLine);
static RESULT GC2155_IsiGetCalibClipParam( IsiSensorHandle_t   handle, IsiAwbClipParm_t    **pIsiClipParam );
static RESULT GC2155_IsiGetCalibGlobalFadeParam( IsiSensorHandle_t       handle, IsiAwbGlobalFadeParm_t  **ptIsiGlobalFadeParam);
static RESULT GC2155_IsiGetCalibFadeParam( IsiSensorHandle_t   handle, IsiAwbFade2Parm_t   **ptIsiFadeParam);
static RESULT GC2155_IsiGetIlluProfile( IsiSensorHandle_t   handle, const uint32_t CieProfile, IsiIlluProfile_t **ptIsiIlluProfile );

static RESULT GC2155_IsiMdiInitMotoDriveMds( IsiSensorHandle_t handle );
static RESULT GC2155_IsiMdiSetupMotoDrive( IsiSensorHandle_t handle, uint32_t *pMaxStep );
static RESULT GC2155_IsiMdiFocusSet( IsiSensorHandle_t handle, const uint32_t Position );
static RESULT GC2155_IsiMdiFocusGet( IsiSensorHandle_t handle, uint32_t *pAbsStep );
static RESULT GC2155_IsiMdiFocusCalibrate( IsiSensorHandle_t handle );

static RESULT GC2155_IsiGetSensorMipiInfoIss( IsiSensorHandle_t handle, IsiSensorMipiInfo *ptIsiSensorMipiInfo);
static RESULT GC2155_IsiGetSensorIsiVersion(  IsiSensorHandle_t   handle, unsigned int* pVersion);

/*****************************************************************************/
/**
 *          GC2155_IsiCreateSensorIss
 *
 * @brief   This function creates a new GC2155 sensor instance handle.
 *
 * @param   pConfig     configuration structure to create the instance
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_NULL_POINTER
 * @retval  RET_OUTOFMEM
 *
 *****************************************************************************/
static RESULT GC2155_IsiCreateSensorIss
(
    IsiSensorInstanceConfig_t *pConfig
)
{
    RESULT result = RET_SUCCESS;

    GC2155_Context_t *pGC2155Ctx;

    TRACE( GC2155_INFO, "%s (enter)\n", __FUNCTION__);

    if ( (pConfig == NULL) || (pConfig->pSensor ==NULL) )
    {
        return ( RET_NULL_POINTER );
    }

    pGC2155Ctx = ( GC2155_Context_t * )malloc ( sizeof (GC2155_Context_t) );
    if ( pGC2155Ctx == NULL )
    {
        TRACE( GC2155_ERROR,  "%s: Can't allocate ov14825 context\n",  __FUNCTION__ );
        return ( RET_OUTOFMEM );
    }
    MEMSET( pGC2155Ctx, 0, sizeof( GC2155_Context_t ) );

    result = HalAddRef( pConfig->HalHandle );
    if ( result != RET_SUCCESS )
    {
        free ( pGC2155Ctx );
        return ( result );
    }

    pGC2155Ctx->IsiCtx.HalHandle              = pConfig->HalHandle;
    pGC2155Ctx->IsiCtx.HalDevID               = pConfig->HalDevID;
    pGC2155Ctx->IsiCtx.I2cBusNum              = pConfig->I2cBusNum;
    pGC2155Ctx->IsiCtx.SlaveAddress           = ( pConfig->SlaveAddr == 0 ) ? GC2155_SLAVE_ADDR : pConfig->SlaveAddr;
    pGC2155Ctx->IsiCtx.NrOfAddressBytes       = GC2155_I2C_NR_ADR_BYTES;

    pGC2155Ctx->IsiCtx.I2cAfBusNum            = pConfig->I2cAfBusNum;
    pGC2155Ctx->IsiCtx.SlaveAfAddress         = ( pConfig->SlaveAfAddr == 0 ) ? GC2155_SLAVE_AF_ADDR : pConfig->SlaveAfAddr;
    pGC2155Ctx->IsiCtx.NrOfAfAddressBytes     = 0;

    pGC2155Ctx->IsiCtx.pSensor                = pConfig->pSensor;

    pGC2155Ctx->Configured             = BOOL_FALSE;
    pGC2155Ctx->Streaming              = BOOL_FALSE;
    pGC2155Ctx->TestPattern            = BOOL_FALSE;
    pGC2155Ctx->isAfpsRun              = BOOL_FALSE;

    pConfig->hSensor = ( IsiSensorHandle_t )pGC2155Ctx;

    result = HalSetCamConfig( pGC2155Ctx->IsiCtx.HalHandle, pGC2155Ctx->IsiCtx.HalDevID, true, true, false );
    RETURN_RESULT_IF_DIFFERENT( RET_SUCCESS, result );

    result = HalSetClock( pGC2155Ctx->IsiCtx.HalHandle, pGC2155Ctx->IsiCtx.HalDevID, 10000000U);
    RETURN_RESULT_IF_DIFFERENT( RET_SUCCESS, result );

    TRACE( GC2155_INFO, "%s (exit)\n", __FUNCTION__);

    return ( result );
}



/*****************************************************************************/
/**
 *          GC2155_IsiReleaseSensorIss
 *
 * @brief   This function destroys/releases an GC2155 sensor instance.
 *
 * @param   handle      GC2155 sensor instance handle
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 *
 *****************************************************************************/
static RESULT GC2155_IsiReleaseSensorIss
(
    IsiSensorHandle_t handle
)
{
    GC2155_Context_t *pGC2155Ctx = (GC2155_Context_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( GC2155_INFO, "%s (enter)\n", __FUNCTION__);

    if ( pGC2155Ctx == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    (void)GC2155_IsiSensorSetStreamingIss( pGC2155Ctx, BOOL_FALSE );
    (void)GC2155_IsiSensorSetPowerIss( pGC2155Ctx, BOOL_FALSE );

    (void)HalDelRef( pGC2155Ctx->IsiCtx.HalHandle );

    MEMSET( pGC2155Ctx, 0, sizeof( GC2155_Context_t ) );
    free ( pGC2155Ctx );

    TRACE( GC2155_INFO, "%s (exit)\n", __FUNCTION__);

    return ( result );
}



/*****************************************************************************/
/**
 *          GC2155_IsiGetCapsIss
 *
 * @brief   fills in the correct pointers for the sensor description struct
 *
 * @param   param1      pointer to sensor capabilities structure
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
static RESULT GC2155_IsiGetCapsIssInternal
(
    IsiSensorCaps_t   *pIsiSensorCaps
)
{

    RESULT result = RET_SUCCESS;


    if ( pIsiSensorCaps == NULL )
    {
        return ( RET_NULL_POINTER );
    }
    else
    {
        switch (pIsiSensorCaps->Index) 
        {
            case 0:
            {
                pIsiSensorCaps->Resolution = ISI_RES_1600_1200P7;
                break;
            }
            case 1:
            {
                pIsiSensorCaps->Resolution = ISI_RES_SVGAP15;
                break;
            }
            default:
            {
                result = RET_OUTOFRANGE;
                goto end;
            }

        }
    
        pIsiSensorCaps->BusWidth        = ISI_BUSWIDTH_12BIT;
        pIsiSensorCaps->Mode            = ISI_MODE_PICT|ISI_MODE_BT601;
        pIsiSensorCaps->FieldSelection  = ISI_FIELDSEL_BOTH;
        pIsiSensorCaps->YCSequence      = ISI_YCSEQ_YCBYCR|ISI_YCSEQ_CBYCRY;           
        pIsiSensorCaps->Conv422         = ISI_CONV422_NOCOSITED;
        pIsiSensorCaps->BPat            = ISI_BPAT_RGRGGBGB ;
        pIsiSensorCaps->HPol            = ISI_HPOL_REFPOS;
        pIsiSensorCaps->VPol            = ISI_VPOL_POS;
        pIsiSensorCaps->Edge            = ISI_EDGE_RISING;
        pIsiSensorCaps->Bls             = ISI_BLS_OFF;
        pIsiSensorCaps->Gamma           = ISI_GAMMA_ON;
        pIsiSensorCaps->CConv           = ISI_CCONV_ON;
        pIsiSensorCaps->BLC             = ( ISI_BLC_AUTO );
        pIsiSensorCaps->AGC             = ( ISI_AGC_AUTO );
        pIsiSensorCaps->AWB             = ( ISI_AWB_AUTO );
        pIsiSensorCaps->AEC             = ( ISI_AEC_AUTO );
        pIsiSensorCaps->DPCC            = ( ISI_DPCC_AUTO );

        pIsiSensorCaps->DwnSz           = ISI_DWNSZ_SUBSMPL;
        pIsiSensorCaps->CieProfile      = 0;
        pIsiSensorCaps->SmiaMode        = ISI_SMIA_OFF;
        pIsiSensorCaps->MipiMode        = ISI_MIPI_OFF;
        pIsiSensorCaps->AfpsResolutions = ( ISI_AFPS_NOTSUPP );
        pIsiSensorCaps->SensorOutputMode = ISI_SENSOR_OUTPUT_MODE_YUV;
    }
    
end:
    return ( result );
}
static RESULT GC2155_IsiGetCapsIss
(
    IsiSensorHandle_t handle,
    IsiSensorCaps_t   *pIsiSensorCaps
)
{
    GC2155_Context_t *pGC2155Ctx = (GC2155_Context_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( GC2155_INFO, "%s (enter)\n", __FUNCTION__);

    if ( pGC2155Ctx == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    result = GC2155_IsiGetCapsIssInternal(pIsiSensorCaps);
    TRACE( GC2155_INFO, "%s (exit)\n", __FUNCTION__);

    return ( result );
}



/*****************************************************************************/
/**
 *          GC2155_g_IsiSensorDefaultConfig
 *
 * @brief   recommended default configuration for application use via call
 *          to IsiGetSensorIss()
 *
 *****************************************************************************/
const IsiSensorCaps_t GC2155_g_IsiSensorDefaultConfig =
{
    ISI_BUSWIDTH_12BIT,         // BusWidth
    ISI_MODE_BT601,              // MIPI
    ISI_FIELDSEL_BOTH,          // FieldSel
    ISI_YCSEQ_CBYCRY,           // YCSeq
    ISI_CONV422_NOCOSITED,      // Conv422
    ISI_BPAT_RGRGGBGB,//ISI_BPAT_BGBGGRGR,          // BPat
    ISI_HPOL_REFPOS,            // HPol
    ISI_VPOL_POS,               // VPol
    ISI_EDGE_RISING,            // Edge
    ISI_BLS_OFF,                // Bls
    ISI_GAMMA_ON,              // Gamma
    ISI_CCONV_ON,              // CConv
    ISI_RES_SVGAP15,          // Res
    ISI_DWNSZ_SUBSMPL,          // DwnSz
    ISI_BLC_AUTO,               // BLC
    ISI_AGC_AUTO,                // AGC
    ISI_AWB_AUTO,                // AWB
    ISI_AEC_AUTO,                // AEC
    ISI_DPCC_AUTO,               // DPCC
    0,            // CieProfile, this is also used as start profile for AWB (if not altered by menu settings)
    ISI_SMIA_OFF,               // SmiaMode
    ISI_MIPI_OFF,       // MipiMode
    ISI_AFPS_NOTSUPP,           // AfpsResolutions
    ISI_SENSOR_OUTPUT_MODE_YUV,
    0,
};



/*****************************************************************************/
/**
 *          GC2155_SetupOutputFormat
 *
 * @brief   Setup of the image sensor considering the given configuration.
 *
 * @param   handle      GC2155 sensor instance handle
 * @param   pConfig     pointer to sensor configuration structure
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
RESULT GC2155_SetupOutputFormat
(
    GC2155_Context_t       *pGC2155Ctx,
    const IsiSensorConfig_t *pConfig
)
{
    RESULT result = RET_SUCCESS;
    return result;

    TRACE( GC2155_INFO, "%s%s (enter)\n", __FUNCTION__, pGC2155Ctx->isAfpsRun?"(AFPS)":"" );

    /* bus-width */
    switch ( pConfig->BusWidth )        /* only ISI_BUSWIDTH_12BIT supported, no configuration needed here */
    {
        case ISI_BUSWIDTH_10BIT:
        case ISI_BUSWIDTH_8BIT_ZZ:
        case ISI_BUSWIDTH_12BIT:
        {
            break;
        }

        default:
        {
            TRACE( GC2155_ERROR, "%s%s: bus width not supported\n", __FUNCTION__, pGC2155Ctx->isAfpsRun?"(AFPS)":"" );
            return ( RET_NOTSUPP );
        }
    }

    /* mode */
    switch ( pConfig->Mode )            /* only ISI_MODE_BAYER supported, no configuration needed here */
    {
        case( ISI_MODE_MIPI ):
        case ISI_MODE_BAYER:
        case ISI_MODE_BT601:
        case ISI_MODE_PICT:
        case  ISI_MODE_DATA:
        {
            break;
        }

        default:
        {
            TRACE( GC2155_ERROR, "%s%s: mode not supported\n", __FUNCTION__, pGC2155Ctx->isAfpsRun?"(AFPS)":"" );
            return ( RET_NOTSUPP );
        }
    }

    /* field-selection */
    switch ( pConfig->FieldSelection )  /* only ISI_FIELDSEL_BOTH supported, no configuration needed */
    {
        case ISI_FIELDSEL_BOTH:
        {
            break;
        }

        default:
        {
            TRACE( GC2155_ERROR, "%s%s: field selection not supported\n", __FUNCTION__, pGC2155Ctx->isAfpsRun?"(AFPS)":"" );
            return ( RET_NOTSUPP );
        }
    }

    switch ( pConfig->YCSequence )
    {
        default:
        {
            break;
        }
    }

    /* 422 conversion */
    switch ( pConfig->Conv422 )         /* only ISI_CONV422_NOCOSITED supported, no configuration needed */
    {
        case ISI_CONV422_NOCOSITED:
        case ISI_CONV422_COSITED:
        case ISI_CONV422_INTER:
        {
            break;
        }

        default:
        {
            TRACE( GC2155_ERROR, "%s%s: 422 conversion not supported\n", __FUNCTION__, pGC2155Ctx->isAfpsRun?"(AFPS)":"" );
            return ( RET_NOTSUPP );
        }
    }

    /* bayer-pattern */
    switch ( pConfig->BPat )            /* only ISI_BPAT_BGBGGRGR supported, no configuration needed */
    {
        case ISI_BPAT_BGBGGRGR:
        case ISI_BPAT_RGRGGBGB:
        {
            break;
        }

        default:
        {
            TRACE( GC2155_ERROR, "%s%s: bayer pattern not supported\n", __FUNCTION__, pGC2155Ctx->isAfpsRun?"(AFPS)":"" );
            return ( RET_NOTSUPP );
        }
    }

    /* horizontal polarity */
    switch ( pConfig->HPol )            /* only ISI_HPOL_REFPOS supported, no configuration needed */
    {
        case ISI_HPOL_REFPOS:
        {
            break;
        }

        default:
        {
            TRACE( GC2155_ERROR, "%s%s: HPol not supported\n", __FUNCTION__, pGC2155Ctx->isAfpsRun?"(AFPS)":"" );
            return ( RET_NOTSUPP );
        }
    }

    /* vertical polarity */
    switch ( pConfig->VPol )            /* no configuration needed */
    {
        case ISI_VPOL_NEG:
        {
            break;
        }
        case ISI_VPOL_POS:
        {
            break;
        }

        default:
        {
            TRACE( GC2155_ERROR, "%s%s: VPol not supported\n", __FUNCTION__, pGC2155Ctx->isAfpsRun?"(AFPS)":"" );
            return ( RET_NOTSUPP );
        }
    }


    /* edge */
    switch ( pConfig->Edge )            /* only ISI_EDGE_RISING supported, no configuration needed */
    {
        case ISI_EDGE_RISING:
        {
            break;
        }

        case ISI_EDGE_FALLING:          /*TODO for MIPI debug*/
        {
            break;
        }

        default:
        {
            TRACE( GC2155_ERROR, "%s%s:  edge mode not supported\n", __FUNCTION__, pGC2155Ctx->isAfpsRun?"(AFPS)":"" );
            return ( RET_NOTSUPP );
        }
    }

    /* gamma */
    switch ( pConfig->Gamma )           /* only ISI_GAMMA_OFF supported, no configuration needed */
    {
        case ISI_GAMMA_ON:
        case ISI_GAMMA_OFF:
        {
            break;
        }

        default:
        {
            TRACE( GC2155_ERROR, "%s%s:  gamma not supported\n", __FUNCTION__, pGC2155Ctx->isAfpsRun?"(AFPS)":"" );
            return ( RET_NOTSUPP );
        }
    }

    /* color conversion */
    switch ( pConfig->CConv )           /* only ISI_CCONV_OFF supported, no configuration needed */
    {
        case ISI_CCONV_OFF:
        case ISI_CCONV_ON:
        {
            break;
        }

        default:
        {
            TRACE( GC2155_ERROR, "%s%s: color conversion not supported\n", __FUNCTION__, pGC2155Ctx->isAfpsRun?"(AFPS)":"" );
            return ( RET_NOTSUPP );
        }
    }

    switch ( pConfig->SmiaMode )        /* only ISI_SMIA_OFF supported, no configuration needed */
    {
        case ISI_SMIA_OFF:
        {
            break;
        }

        default:
        {
            TRACE( GC2155_ERROR, "%s%s: SMIA mode not supported\n", __FUNCTION__, pGC2155Ctx->isAfpsRun?"(AFPS)":"" );
            return ( RET_NOTSUPP );
        }
    }

    switch ( pConfig->MipiMode )        /* only ISI_MIPI_MODE_RAW_12 supported, no configuration needed */
    {
        case ISI_MIPI_MODE_RAW_10:
        case ISI_MIPI_OFF:
        {
            break;
        }

        default:
        {
            TRACE( GC2155_ERROR, "%s%s: MIPI mode not supported\n", __FUNCTION__, pGC2155Ctx->isAfpsRun?"(AFPS)":"" );
            return ( RET_NOTSUPP );
        }
    }

    switch ( pConfig->AfpsResolutions ) /* no configuration needed */
    {
        case ISI_AFPS_NOTSUPP:
        {
            break;
        }
        default:
        {
            // don't care about what comes in here
            //TRACE( GC2155_ERROR, "%s%s: AFPS not supported\n", __FUNCTION__, pGC2155Ctx->isAfpsRun?"(AFPS)":"" );
            //return ( RET_NOTSUPP );
        }
    }

    TRACE( GC2155_INFO, "%s%s (exit)\n", __FUNCTION__, pGC2155Ctx->isAfpsRun?"(AFPS)":"");

    return ( result );
}

int GC2155_get_PCLK( GC2155_Context_t *pGC2155Ctx, int XVCLK)
{
    // calculate PCLK
    
    return 0;
}

/*****************************************************************************/
/**
 *          GC2155_SetupOutputWindow
 *
 * @brief   Setup of the image sensor considering the given configuration.
 *
 * @param   handle      GC2155 sensor instance handle
 * @param   pConfig     pointer to sensor configuration structure
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
static RESULT GC2155_SetupOutputWindow
(
    GC2155_Context_t        *pGC2155Ctx,
    const IsiSensorConfig_t *pConfig
)
{
    RESULT result     = RET_SUCCESS;

        /* resolution */
    switch ( pConfig->Resolution )
    {
        case ISI_RES_SVGAP15:
        {
            if((result = IsiRegDefaultsApply((IsiSensorHandle_t)pGC2155Ctx,GC2155_g_svga)) != RET_SUCCESS){
                TRACE( GC2155_ERROR, "%s: failed to set  ISI_RES_SVGAP15 \n", __FUNCTION__ );
            }else{

                TRACE( GC2155_INFO, "%s: success to set  ISI_RES_SVGAP15 \n", __FUNCTION__ );
            }
            break;
        }
        case ISI_RES_1600_1200P7:
        {
            if((result = IsiRegDefaultsApply((IsiSensorHandle_t)pGC2155Ctx,GC2155_g_1600x1200)) != RET_SUCCESS){
                TRACE( GC2155_ERROR, "%s: failed to set  ISI_RES_1600_1200P7 \n", __FUNCTION__ );
            }else{

                TRACE( GC2155_INFO, "%s: success to set  ISI_RES_1600_1200P7  \n", __FUNCTION__ );
            }
            break;
        }
        default:
        {
            TRACE( GC2155_ERROR, "%s: Resolution not supported\n", __FUNCTION__ );
            return ( RET_NOTSUPP );
        }
    }


    return ( result );
}




/*****************************************************************************/
/**
 *          GC2155_SetupImageControl
 *
 * @brief   Sets the image control functions (BLC, AGC, AWB, AEC, DPCC ...)
 *
 * @param   handle      GC2155 sensor instance handle
 * @param   pConfig     pointer to sensor configuration structure
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
RESULT GC2155_SetupImageControl
(
    GC2155_Context_t        *pGC2155Ctx,
    const IsiSensorConfig_t *pConfig
)
{
    RESULT result = RET_SUCCESS;

    uint32_t RegValue = 0U;

    TRACE( GC2155_INFO, "%s (enter)\n", __FUNCTION__);


    return ( result );
}


/*****************************************************************************/
/**
 *          GC2155_AecSetModeParameters
 *
 * @brief   This function fills in the correct parameters in GC2155-Instances
 *          according to AEC mode selection in IsiSensorConfig_t.
 *
 * @note    It is assumed that IsiSetupOutputWindow has been called before
 *          to fill in correct values in instance structure.
 *
 * @param   handle      GC2155 context
 * @param   pConfig     pointer to sensor configuration structure
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
static RESULT GC2155_AecSetModeParameters
(
    GC2155_Context_t       *pGC2155Ctx,
    const IsiSensorConfig_t *pConfig
)
{
    RESULT result = RET_SUCCESS;

    return ( result );
}


/*****************************************************************************/
/**
 *          GC2155_IsiSetupSensorIss
 *
 * @brief   Setup of the image sensor considering the given configuration.
 *
 * @param   handle      GC2155 sensor instance handle
 * @param   pConfig     pointer to sensor configuration structure
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
static RESULT GC2155_IsiSetupSensorIss
(
    IsiSensorHandle_t       handle,
    const IsiSensorConfig_t *pConfig
)
{
    GC2155_Context_t *pGC2155Ctx = (GC2155_Context_t *)handle;

    RESULT result = RET_SUCCESS;

    uint32_t RegValue = 0;

    TRACE( GC2155_INFO, "%s (enter)\n", __FUNCTION__);

    if ( pGC2155Ctx == NULL )
    {
        TRACE( GC2155_ERROR, "%s: Invalid sensor handle (NULL pointer detected)\n", __FUNCTION__ );
        return ( RET_WRONG_HANDLE );
    }

    if ( pConfig == NULL )
    {
        TRACE( GC2155_ERROR, "%s: Invalid configuration (NULL pointer detected)\n", __FUNCTION__ );
        return ( RET_NULL_POINTER );
    }

    if ( pGC2155Ctx->Streaming != BOOL_FALSE )
    {
        return RET_WRONG_STATE;
    }

    MEMCPY( &pGC2155Ctx->Config, pConfig, sizeof( IsiSensorConfig_t ) );

    /* 2.) write default values derived from datasheet and evaluation kit (static setup altered by dynamic setup further below) */
    result = IsiRegDefaultsApply( pGC2155Ctx, GC2155_g_aRegDescription );
    if ( result != RET_SUCCESS )
    {
        return ( result );
    }

    /* sleep a while, that sensor can take over new default values */
    osSleep( 10 );
    #if 0


    /* 3.) verify default values to make sure everything has been written correctly as expected */
    result = IsiRegDefaultsVerify( pGC2155Ctx, GC2155_g_aRegDescription );
    if ( result != RET_SUCCESS )
    {
        return ( result );
    }
    // output of pclk for measurement (only debugging)
    result = GC2155_IsiRegWriteIss( pGC2155Ctx, 0x3009U, 0x10U );
    RETURN_RESULT_IF_DIFFERENT( RET_SUCCESS, result );
    #endif

    /* 4.) setup output format (RAW10|RAW12) */
    result = GC2155_SetupOutputFormat( pGC2155Ctx, pConfig );
    if ( result != RET_SUCCESS )
    {
        TRACE( GC2155_ERROR, "%s: SetupOutputFormat failed.\n", __FUNCTION__);
        return ( result );
    }

    /* 5.) setup output window */
    result = GC2155_SetupOutputWindow( pGC2155Ctx, pConfig );
    if ( result != RET_SUCCESS )
    {
        TRACE( GC2155_ERROR, "%s: SetupOutputWindow failed.\n", __FUNCTION__);
        return ( result );
    }

    result = GC2155_SetupImageControl( pGC2155Ctx, pConfig );
    if ( result != RET_SUCCESS )
    {
        TRACE( GC2155_ERROR, "%s: SetupImageControl failed.\n", __FUNCTION__);
        return ( result );
    }

    result = GC2155_AecSetModeParameters( pGC2155Ctx, pConfig );
    if ( result != RET_SUCCESS )
    {
        TRACE( GC2155_ERROR, "%s: AecSetModeParameters failed.\n", __FUNCTION__);
        return ( result );
    }
    if (result == RET_SUCCESS)
    {
        pGC2155Ctx->Configured = BOOL_TRUE;
    }

    TRACE( GC2155_INFO, "%s: (exit)\n", __FUNCTION__);

    return ( result );
}



/*****************************************************************************/
/**
 *          GC2155_IsiChangeSensorResolutionIss
 *
 * @brief   Change image sensor resolution while keeping all other static settings.
 *          Dynamic settings like current gain & integration time are kept as
 *          close as possible. Sensor needs 2 frames to engage (first 2 frames
 *          are not correctly exposed!).
 *
 * @note    Re-read current & min/max values as they will probably have changed!
 *
 * @param   handle                  Sensor instance handle
 * @param   Resolution              new resolution ID
 * @param   pNumberOfFramesToSkip   reference to storage for number of frames to skip
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_WRONG_STATE
 * @retval  RET_OUTOFRANGE
 *
 *****************************************************************************/
static RESULT GC2155_IsiChangeSensorResolutionIss
(
    IsiSensorHandle_t   handle,
    uint32_t            Resolution,
    uint8_t             *pNumberOfFramesToSkip
)
{
    GC2155_Context_t *pGC2155Ctx = (GC2155_Context_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( GC2155_INFO, "%s (enter)\n", __FUNCTION__);

    if ( pGC2155Ctx == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    if (pNumberOfFramesToSkip == NULL)
    {
        return ( RET_NULL_POINTER );
    }

    if ( (pGC2155Ctx->Configured != BOOL_TRUE) || (pGC2155Ctx->Streaming != BOOL_FALSE) )
    {
        return RET_WRONG_STATE;
    }

    IsiSensorCaps_t Caps;    
    Caps.Index = 0;
    Caps.Resolution = 0;
    while (GC2155_IsiGetCapsIss( handle, &Caps) == RET_SUCCESS) {
        if (Resolution == Caps.Resolution) {            
            break;
        }
        Caps.Index++;
    }

    if ( (Resolution & Caps.Resolution) == 0 )
    {
        return RET_OUTOFRANGE;
    }

    if ( Resolution == pGC2155Ctx->Config.Resolution )
    {
        // well, no need to worry
        *pNumberOfFramesToSkip = 0;
    }
    else
    {
        // change resolution
        char *szResName = NULL;
        result = IsiGetResolutionName( Resolution, &szResName );
        TRACE( GC2155_DEBUG, "%s: NewRes=0x%08x (%s)\n", __FUNCTION__, Resolution, szResName);

        // update resolution in copy of config in context
        pGC2155Ctx->Config.Resolution = Resolution;
        
        // hkw add for gc2155 exposure;
        #if 1
        if(Resolution == ISI_RES_1600_1200P7){
		    uint32_t value;
			unsigned   int pid=0,shutter,temp_reg;
			TRACE( GC2155_ERROR, "---------enter %s----------------:\n", __FUNCTION__);
			result = IsiWriteRegister( handle, 0xfe, 0X00 );
            if ( result != RET_SUCCESS )
            {
               TRACE( GC2155_ERROR, "%s: IsiWriteRegister failed.\n", __FUNCTION__);
            }
				result = IsiWriteRegister( handle, 0xb6, 0X00 );
            if ( result != RET_SUCCESS )
            {
               TRACE( GC2155_ERROR, "%s: IsiWriteRegister failed.\n", __FUNCTION__);
            }	 
					 
			result = IsiReadRegister( handle, 0x03, &value );
            if ( result != RET_SUCCESS )
            {
                TRACE( GC2155_ERROR, "%s: failed to read reg 0x03, value: %d\n", 
                            __FUNCTION__,  value );
                
            }
            TRACE( GC2155_ERROR, "%s: ---- read reg 0x03, value: 0x%x----\n", 
                            __FUNCTION__,  value );
			pid |= (value << 8);
			result = IsiReadRegister( handle, 0x04, &value );
            if ( result != RET_SUCCESS )
            {
                TRACE( GC2155_ERROR, "%s: failed to read 0x04, value:0x%x\n", 
                            __FUNCTION__,    value );
                
            }
			TRACE( GC2155_ERROR, "%s:  ----read 0x04, value:0x%x----\n", 
                            __FUNCTION__,    value );
			pid |= (value & 0xff);
			TRACE( GC2155_ERROR, "%s:  ----pid:0x%x----\n", 
                            __FUNCTION__,    pid);
			shutter=pid;
			
			temp_reg= shutter/2;
					
		    if(temp_reg < 1) temp_reg = 1;
					
			result = IsiWriteRegister( handle, 0x03, ((temp_reg>>8)&0x1f) );
            if ( result != RET_SUCCESS )
            {
               TRACE( GC2155_ERROR, "%s: IsiWriteRegister failed.\n", __FUNCTION__);
            }
				result = IsiWriteRegister( handle, 0x04, (temp_reg&0xff) );
            if ( result != RET_SUCCESS )
            {
               TRACE( GC2155_ERROR, "%s: IsiWriteRegister failed.\n", __FUNCTION__);
            }	 
        }
        #endif
        
        // tell sensor about that
        result = GC2155_SetupOutputWindow( pGC2155Ctx, &pGC2155Ctx->Config );
        if ( result != RET_SUCCESS )
        {
            TRACE( GC2155_ERROR, "%s: SetupOutputWindow failed.\n", __FUNCTION__);
            return ( result );
        }
        osSleep(200);
        // remember old exposure values
        float OldGain = pGC2155Ctx->AecCurGain;
        float OldIntegrationTime = pGC2155Ctx->AecCurIntegrationTime;

        // update limits & stuff (reset current & old settings)
        result = GC2155_AecSetModeParameters( pGC2155Ctx, &pGC2155Ctx->Config );
        if ( result != RET_SUCCESS )
        {
            TRACE( GC2155_ERROR, "%s: AecSetModeParameters failed.\n", __FUNCTION__);
            return ( result );
        }

        // restore old exposure values (at least within new exposure values' limits)
        uint8_t NumberOfFramesToSkip = 0;
        float   DummySetGain;
        float   DummySetIntegrationTime;
        result = GC2155_IsiExposureControlIss( handle, OldGain, OldIntegrationTime, &NumberOfFramesToSkip, &DummySetGain, &DummySetIntegrationTime );
        if ( result != RET_SUCCESS )
        {
            TRACE( GC2155_ERROR, "%s: GC2155_IsiExposureControlIss failed.\n", __FUNCTION__);
            return ( result );
        }

        // return number of frames that aren't exposed correctly
        *pNumberOfFramesToSkip = NumberOfFramesToSkip + 1;
    }

    TRACE( GC2155_INFO, "%s (exit)\n", __FUNCTION__);

    return ( result );
}



/*****************************************************************************/
/**
 *          GC2155_IsiSensorSetStreamingIss
 *
 * @brief   Enables/disables streaming of sensor data, if possible.
 *
 * @param   handle      Sensor instance handle
 * @param   on          new streaming state (BOOL_TRUE=on, BOOL_FALSE=off)
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_WRONG_STATE
 *
 *****************************************************************************/
static RESULT GC2155_IsiSensorSetStreamingIss
(
    IsiSensorHandle_t   handle,
    bool_t              on
)
{
    uint32_t RegValue = 0;

    GC2155_Context_t *pGC2155Ctx = (GC2155_Context_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( GC2155_INFO, "%s (enter)\n", __FUNCTION__);

    if ( pGC2155Ctx == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    if ( (pGC2155Ctx->Configured != BOOL_TRUE) || (pGC2155Ctx->Streaming == on) )
    {
        return RET_WRONG_STATE;
    }

    if (on == BOOL_TRUE)
    {
        /* enable streaming */
        result = GC2155_IsiRegWriteIss( handle, 0x0100, 0x1);

    }
    else
    {
        /* disable streaming */
        result = GC2155_IsiRegWriteIss( handle, 0x0100, 0x0);


    }

    if (result == RET_SUCCESS)
    {
        pGC2155Ctx->Streaming = on;
    }

    TRACE( GC2155_INFO, "%s (exit)\n", __FUNCTION__);

    return ( result );
}



/*****************************************************************************/
/**
 *          GC2155_IsiSensorSetPowerIss
 *
 * @brief   Performs the power-up/power-down sequence of the camera, if possible.
 *
 * @param   handle      GC2155 sensor instance handle
 * @param   on          new power state (BOOL_TRUE=on, BOOL_FALSE=off)
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
static RESULT GC2155_IsiSensorSetPowerIss
(
    IsiSensorHandle_t   handle,
    bool_t              on
)
{
    GC2155_Context_t *pGC2155Ctx = (GC2155_Context_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( GC2155_INFO, "%s (enter)\n", __FUNCTION__);

    if ( pGC2155Ctx == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    pGC2155Ctx->Configured = BOOL_FALSE;
    pGC2155Ctx->Streaming  = BOOL_FALSE;

    TRACE( GC2155_DEBUG, "%s power off \n", __FUNCTION__);
    result = HalSetPower( pGC2155Ctx->IsiCtx.HalHandle, pGC2155Ctx->IsiCtx.HalDevID, false );
    RETURN_RESULT_IF_DIFFERENT( RET_SUCCESS, result );

    TRACE( GC2155_DEBUG, "%s reset on\n", __FUNCTION__);
    result = HalSetReset( pGC2155Ctx->IsiCtx.HalHandle, pGC2155Ctx->IsiCtx.HalDevID, true );
    RETURN_RESULT_IF_DIFFERENT( RET_SUCCESS, result );

    if (on == BOOL_TRUE)
    {
        TRACE( GC2155_DEBUG, "%s power on \n", __FUNCTION__);
        result = HalSetPower( pGC2155Ctx->IsiCtx.HalHandle, pGC2155Ctx->IsiCtx.HalDevID, true );
        RETURN_RESULT_IF_DIFFERENT( RET_SUCCESS, result );

        osSleep( 10 );

        TRACE( GC2155_DEBUG, "%s reset off \n", __FUNCTION__);
        result = HalSetReset( pGC2155Ctx->IsiCtx.HalHandle, pGC2155Ctx->IsiCtx.HalDevID, false );
        RETURN_RESULT_IF_DIFFERENT( RET_SUCCESS, result );

        osSleep( 10 );

        TRACE( GC2155_DEBUG, "%s reset on \n", __FUNCTION__);
        result = HalSetReset( pGC2155Ctx->IsiCtx.HalHandle, pGC2155Ctx->IsiCtx.HalDevID, true );
        RETURN_RESULT_IF_DIFFERENT( RET_SUCCESS, result );

        osSleep( 10 );

        TRACE( GC2155_DEBUG, "%s reset off \n", __FUNCTION__);
        result = HalSetReset( pGC2155Ctx->IsiCtx.HalHandle, pGC2155Ctx->IsiCtx.HalDevID, false );

        osSleep( 50 );
    }

    TRACE( GC2155_INFO, "%s (exit)\n", __FUNCTION__);

    return ( result );
}



/*****************************************************************************/
/**
 *          GC2155_IsiCheckSensorConnectionIss
 *
 * @brief   Checks the I2C-Connection to sensor by reading sensor revision id.
 *
 * @param   handle      GC2155 sensor instance handle
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
static RESULT GC2155_IsiCheckSensorConnectionIss
(
    IsiSensorHandle_t   handle
)
{
    uint32_t RevId;
    uint32_t value;

    RESULT result = RET_SUCCESS;

    TRACE( GC2155_INFO, "%s (enter)\n", __FUNCTION__);

    if ( handle == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }
		
	
    RevId = GC2155_CHIP_ID_HIGH_BYTE_DEFAULT;
    RevId = (RevId << 16U) | (GC2155_CHIP_ID_MIDDLE_BYTE_DEFAULT<<8U);
    RevId = RevId | GC2155_CHIP_ID_LOW_BYTE_DEFAULT;

    result = GC2155_IsiGetSensorRevisionIss( handle, &value );
    if ( (result != RET_SUCCESS) || (RevId != value) )
    {
        TRACE( GC2155_ERROR, "%s RevId = 0x%08x, value = 0x%08x \n", __FUNCTION__, RevId, value );
        return ( RET_FAILURE );
    }

    TRACE( GC2155_DEBUG, "%s RevId = 0x%08x, value = 0x%08x \n", __FUNCTION__, RevId, value );

    TRACE( GC2155_INFO, "%s (exit)\n", __FUNCTION__);

    return ( result );
}



/*****************************************************************************/
/**
 *          GC2155_IsiGetSensorRevisionIss
 *
 * @brief   reads the sensor revision register and returns this value
 *
 * @param   handle      pointer to sensor description struct
 * @param   p_value     pointer to storage value
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
static RESULT GC2155_IsiGetSensorRevisionIss
(
    IsiSensorHandle_t   handle,
    uint32_t            *p_value
)
{
    RESULT result = RET_SUCCESS;

    uint32_t data;

    TRACE( GC2155_INFO, "%s (enter)\n", __FUNCTION__);

    if ( handle == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    if ( p_value == NULL )
    {
        return ( RET_NULL_POINTER );
    }

    *p_value = 0U;
    result = GC2155_IsiRegReadIss ( handle, GC2155_CHIP_ID_HIGH_BYTE, &data );
	
    *p_value = ( (data & 0xFF) << 16U );
    result = GC2155_IsiRegReadIss ( handle, GC2155_CHIP_ID_MIDDLE_BYTE, &data );
    *p_value |= ( (data & 0xFF) << 8U );
    result = GC2155_IsiRegReadIss ( handle, GC2155_CHIP_ID_LOW_BYTE, &data );
    *p_value |= ( (data & 0xFF));

    TRACE( GC2155_INFO, "%s (exit)\n", __FUNCTION__);

    return ( result );
}



/*****************************************************************************/
/**
 *          GC2155_IsiRegReadIss
 *
 * @brief   grants user read access to the camera register
 *
 * @param   handle      pointer to sensor description struct
 * @param   address     sensor register to write
 * @param   p_value     pointer to value
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
static RESULT GC2155_IsiRegReadIss
(
    IsiSensorHandle_t   handle,
    const uint32_t      address,
    uint32_t            *p_value
)
{
    RESULT result = RET_SUCCESS;

  //  TRACE( GC2155_INFO, "%s (enter)\n", __FUNCTION__);

    if ( handle == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    if ( p_value == NULL )
    {
        return ( RET_NULL_POINTER );
    }
    else
    {
        uint8_t NrOfBytes = IsiGetNrDatBytesIss( address, GC2155_g_aRegDescription );
        if ( !NrOfBytes )
        {
            NrOfBytes = 1;
        }
 //       TRACE( GC2155_REG_DEBUG, "%s (IsiGetNrDatBytesIss %d 0x%08x)\n", __FUNCTION__, NrOfBytes, address);

        *p_value = 0;
        result = IsiI2cReadSensorRegister( handle, address, (uint8_t *)p_value, NrOfBytes, BOOL_TRUE );
    }

  //  TRACE( GC2155_INFO, "%s (exit: 0x%08x 0x%08x)\n", __FUNCTION__, address, *p_value);

    return ( result );
}



/*****************************************************************************/
/**
 *          GC2155_IsiRegWriteIss
 *
 * @brief   grants user write access to the camera register
 *
 * @param   handle      pointer to sensor description struct
 * @param   address     sensor register to write
 * @param   value       value to write
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 *
 *****************************************************************************/
static RESULT GC2155_IsiRegWriteIss
(
    IsiSensorHandle_t   handle,
    const uint32_t      address,
    const uint32_t      value
)
{
    RESULT result = RET_SUCCESS;

    uint8_t NrOfBytes;

  //  TRACE( GC2155_INFO, "%s (enter)\n", __FUNCTION__);

    if ( handle == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    NrOfBytes = IsiGetNrDatBytesIss( address, GC2155_g_aRegDescription );
    if ( !NrOfBytes )
    {
        NrOfBytes = 1;
    }
//    TRACE( GC2155_REG_DEBUG, "%s (IsiGetNrDatBytesIss %d 0x%08x 0x%08x)\n", __FUNCTION__, NrOfBytes, address, value);

    result = IsiI2cWriteSensorRegister( handle, address, (uint8_t *)(&value), NrOfBytes, BOOL_TRUE );

//    TRACE( GC2155_INFO, "%s (exit: 0x%08x 0x%08x)\n", __FUNCTION__, address, value);

    return ( result );
}



/*****************************************************************************/
/**
 *          GC2155_IsiGetGainLimitsIss
 *
 * @brief   Returns the exposure minimal and maximal values of an
 *          GC2155 instance
 *
 * @param   handle       GC2155 sensor instance handle
 * @param   pMinExposure Pointer to a variable receiving minimal exposure value
 * @param   pMaxExposure Pointer to a variable receiving maximal exposure value
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
static RESULT GC2155_IsiGetGainLimitsIss
(
    IsiSensorHandle_t   handle,
    float               *pMinGain,
    float               *pMaxGain
)
{
    GC2155_Context_t *pGC2155Ctx = (GC2155_Context_t *)handle;

    RESULT result = RET_SUCCESS;

    uint32_t RegValue = 0;

    TRACE( GC2155_INFO, "%s: (enter)\n", __FUNCTION__);

    if ( pGC2155Ctx == NULL )
    {
        TRACE( GC2155_ERROR, "%s: Invalid sensor handle (NULL pointer detected)\n", __FUNCTION__ );
        return ( RET_WRONG_HANDLE );
    }

    if ( (pMinGain == NULL) || (pMaxGain == NULL) )
    {
        TRACE( GC2155_ERROR, "%s: NULL pointer received!!\n" );
        return ( RET_NULL_POINTER );
    }

    *pMinGain = pGC2155Ctx->AecMinGain;
    *pMaxGain = pGC2155Ctx->AecMaxGain;

    TRACE( GC2155_INFO, "%s: (enter)\n", __FUNCTION__);

    return ( result );
}



/*****************************************************************************/
/**
 *          GC2155_IsiGetIntegrationTimeLimitsIss
 *
 * @brief   Returns the minimal and maximal integration time values of an
 *          GC2155 instance
 *
 * @param   handle       GC2155 sensor instance handle
 * @param   pMinExposure Pointer to a variable receiving minimal exposure value
 * @param   pMaxExposure Pointer to a variable receiving maximal exposure value
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
static RESULT GC2155_IsiGetIntegrationTimeLimitsIss
(
    IsiSensorHandle_t   handle,
    float               *pMinIntegrationTime,
    float               *pMaxIntegrationTime
)
{
    GC2155_Context_t *pGC2155Ctx = (GC2155_Context_t *)handle;

    RESULT result = RET_SUCCESS;

    uint32_t RegValue = 0;

    TRACE( GC2155_INFO, "%s: (enter)\n", __FUNCTION__);

    if ( pGC2155Ctx == NULL )
    {
        TRACE( GC2155_ERROR, "%s: Invalid sensor handle (NULL pointer detected)\n", __FUNCTION__ );
        return ( RET_WRONG_HANDLE );
    }

    if ( (pMinIntegrationTime == NULL) || (pMaxIntegrationTime == NULL) )
    {
        TRACE( GC2155_ERROR, "%s: NULL pointer received!!\n" );
        return ( RET_NULL_POINTER );
    }

    *pMinIntegrationTime = pGC2155Ctx->AecMinIntegrationTime;
    *pMaxIntegrationTime = pGC2155Ctx->AecMaxIntegrationTime;

    TRACE( GC2155_INFO, "%s: (enter)\n", __FUNCTION__);

    return ( result );
}


/*****************************************************************************/
/**
 *          GC2155_IsiGetGainIss
 *
 * @brief   Reads gain values from the image sensor module.
 *
 * @param   handle                  GC2155 sensor instance handle
 * @param   pSetGain                set gain
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
RESULT GC2155_IsiGetGainIss
(
    IsiSensorHandle_t   handle,
    float               *pSetGain
)
{
    GC2155_Context_t *pGC2155Ctx = (GC2155_Context_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( GC2155_INFO, "%s: (enter)\n", __FUNCTION__);

    if ( pGC2155Ctx == NULL )
    {
        TRACE( GC2155_ERROR, "%s: Invalid sensor handle (NULL pointer detected)\n", __FUNCTION__ );
        return ( RET_WRONG_HANDLE );
    }

    if ( pSetGain == NULL)
    {
        return ( RET_NULL_POINTER );
    }

    *pSetGain = pGC2155Ctx->AecCurGain;

    TRACE( GC2155_INFO, "%s: (exit)\n", __FUNCTION__);

    return ( result );
}



/*****************************************************************************/
/**
 *          GC2155_IsiGetGainIncrementIss
 *
 * @brief   Get smallest possible gain increment.
 *
 * @param   handle                  GC2155 sensor instance handle
 * @param   pIncr                   increment
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
RESULT GC2155_IsiGetGainIncrementIss
(
    IsiSensorHandle_t   handle,
    float               *pIncr
)
{
    GC2155_Context_t *pGC2155Ctx = (GC2155_Context_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( GC2155_INFO, "%s: (enter)\n", __FUNCTION__);

    if ( pGC2155Ctx == NULL )
    {
        TRACE( GC2155_ERROR, "%s: Invalid sensor handle (NULL pointer detected)\n", __FUNCTION__ );
        return ( RET_WRONG_HANDLE );
    }

    if ( pIncr == NULL)
    {
        return ( RET_NULL_POINTER );
    }

    //_smallest_ increment the sensor/driver can handle (e.g. used for sliders in the application)
    *pIncr = pGC2155Ctx->AecGainIncrement;

    TRACE( GC2155_INFO, "%s: (exit)\n", __FUNCTION__);

    return ( result );
}



/*****************************************************************************/
/**
 *          GC2155_IsiSetGainIss
 *
 * @brief   Writes gain values to the image sensor module.
 *          Updates current gain and exposure in sensor struct/state.
 *
 * @param   handle                  GC2155 sensor instance handle
 * @param   NewGain                 gain to be set
 * @param   pSetGain                set gain
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NULL_POINTER
 * @retval  RET_INVALID_PARM
 * @retval  RET_FAILURE
 *
 *****************************************************************************/
RESULT GC2155_IsiSetGainIss
(
    IsiSensorHandle_t   handle,
    float               NewGain,
    float               *pSetGain
)
{
    GC2155_Context_t *pGC2155Ctx = (GC2155_Context_t *)handle;

    RESULT result = RET_SUCCESS;

    uint16_t usGain = 0;

    TRACE( GC2155_INFO, "%s: (enter)\n", __FUNCTION__);

    if ( pGC2155Ctx == NULL )
    {
        TRACE( GC2155_ERROR, "%s: Invalid sensor handle (NULL pointer detected)\n", __FUNCTION__ );
        return ( RET_WRONG_HANDLE );
    }

    if ( pSetGain == NULL)
    {
        TRACE( GC2155_ERROR, "%s: Invalid parameter (NULL pointer detected)\n", __FUNCTION__ );
        return ( RET_NULL_POINTER );
    }

  
    if( NewGain < pGC2155Ctx->AecMinGain ) NewGain = pGC2155Ctx->AecMinGain;
    if( NewGain > pGC2155Ctx->AecMaxGain ) NewGain = pGC2155Ctx->AecMaxGain;

    usGain = (uint16_t)NewGain;

    // write new gain into sensor registers, do not write if nothing has changed
    if( (usGain != pGC2155Ctx->OldGain) )
    {

        pGC2155Ctx->OldGain = usGain;
    }

    //calculate gain actually set
    pGC2155Ctx->AecCurGain = NewGain;

    //return current state
    *pSetGain = pGC2155Ctx->AecCurGain;
    TRACE( GC2155_DEBUG, "%s: g=%f\n", __FUNCTION__, *pSetGain );

    TRACE( GC2155_INFO, "%s: (exit)\n", __FUNCTION__);

    return ( result );
}



/*****************************************************************************/
/**
 *          GC2155_IsiGetIntegrationTimeIss
 *
 * @brief   Reads integration time values from the image sensor module.
 *
 * @param   handle                  GC2155 sensor instance handle
 * @param   pSetIntegrationTime     set integration time
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
RESULT GC2155_IsiGetIntegrationTimeIss
(
    IsiSensorHandle_t   handle,
    float               *pSetIntegrationTime
)
{
    GC2155_Context_t *pGC2155Ctx = (GC2155_Context_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( GC2155_INFO, "%s: (enter)\n", __FUNCTION__);

    if ( pGC2155Ctx == NULL )
    {
        TRACE( GC2155_ERROR, "%s: Invalid sensor handle (NULL pointer detected)\n", __FUNCTION__ );
        return ( RET_WRONG_HANDLE );
    }

    if ( pSetIntegrationTime == NULL )
    {
        return ( RET_NULL_POINTER );
    }

    *pSetIntegrationTime = pGC2155Ctx->AecCurIntegrationTime;

    TRACE( GC2155_INFO, "%s: (exit)\n", __FUNCTION__);

    return ( result );
}



/*****************************************************************************/
/**
 *          GC2155_IsiGetIntegrationTimeIncrementIss
 *
 * @brief   Get smallest possible integration time increment.
 *
 * @param   handle                  GC2155 sensor instance handle
 * @param   pIncr                   increment
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
RESULT GC2155_IsiGetIntegrationTimeIncrementIss
(
    IsiSensorHandle_t   handle,
    float               *pIncr
)
{
    GC2155_Context_t *pGC2155Ctx = (GC2155_Context_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( GC2155_INFO, "%s: (enter)\n", __FUNCTION__);

    if ( pGC2155Ctx == NULL )
    {
        TRACE( GC2155_ERROR, "%s: Invalid sensor handle (NULL pointer detected)\n", __FUNCTION__ );
        return ( RET_WRONG_HANDLE );
    }

    if ( pIncr == NULL )
    {
        return ( RET_NULL_POINTER );
    }

    //_smallest_ increment the sensor/driver can handle (e.g. used for sliders in the application)
    *pIncr = pGC2155Ctx->AecIntegrationTimeIncrement;

    TRACE( GC2155_INFO, "%s: (exit)\n", __FUNCTION__);

    return ( result );
}



/*****************************************************************************/
/**
 *          GC2155_IsiSetIntegrationTimeIss
 *
 * @brief   Writes gain and integration time values to the image sensor module.
 *          Updates current integration time and exposure in sensor
 *          struct/state.
 *
 * @param   handle                  GC2155 sensor instance handle
 * @param   NewIntegrationTime      integration time to be set
 * @param   pSetIntegrationTime     set integration time
 * @param   pNumberOfFramesToSkip   number of frames to skip until AE is
 *                                  executed again
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NULL_POINTER
 * @retval  RET_INVALID_PARM
 * @retval  RET_FAILURE
 * @retval  RET_DIVISION_BY_ZERO
 *
 *****************************************************************************/
RESULT GC2155_IsiSetIntegrationTimeIss
(
    IsiSensorHandle_t   handle,
    float               NewIntegrationTime,
    float               *pSetIntegrationTime,
    uint8_t             *pNumberOfFramesToSkip
)
{
    GC2155_Context_t *pGC2155Ctx = (GC2155_Context_t *)handle;

    RESULT result = RET_SUCCESS;


    TRACE( GC2155_DEBUG, "%s: Ti=%f\n", __FUNCTION__, *pSetIntegrationTime );
    TRACE( GC2155_INFO, "%s: (exit)\n", __FUNCTION__);

    return ( result );
}




/*****************************************************************************/
/**
 *          GC2155_IsiExposureControlIss
 *
 * @brief   Camera hardware dependent part of the exposure control loop.
 *          Calculates appropriate register settings from the new exposure
 *          values and writes them to the image sensor module.
 *
 * @param   handle                  GC2155 sensor instance handle
 * @param   NewGain                 newly calculated gain to be set
 * @param   NewIntegrationTime      newly calculated integration time to be set
 * @param   pNumberOfFramesToSkip   number of frames to skip until AE is
 *                                  executed again
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NULL_POINTER
 * @retval  RET_INVALID_PARM
 * @retval  RET_FAILURE
 * @retval  RET_DIVISION_BY_ZERO
 *
 *****************************************************************************/
RESULT GC2155_IsiExposureControlIss
(
    IsiSensorHandle_t   handle,
    float               NewGain,
    float               NewIntegrationTime,
    uint8_t             *pNumberOfFramesToSkip,
    float               *pSetGain,
    float               *pSetIntegrationTime
)
{
    GC2155_Context_t *pGC2155Ctx = (GC2155_Context_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( GC2155_INFO, "%s: (enter)\n", __FUNCTION__);

    if ( pGC2155Ctx == NULL )
    {
        TRACE( GC2155_ERROR, "%s: Invalid sensor handle (NULL pointer detected)\n", __FUNCTION__ );
        return ( RET_WRONG_HANDLE );
    }

    if ( (pNumberOfFramesToSkip == NULL)
            || (pSetGain == NULL)
            || (pSetIntegrationTime == NULL) )
    {
        TRACE( GC2155_ERROR, "%s: Invalid parameter (NULL pointer detected)\n", __FUNCTION__ );
        return ( RET_NULL_POINTER );
    }

    TRACE( GC2155_DEBUG, "%s: g=%f, Ti=%f\n", __FUNCTION__, NewGain, NewIntegrationTime );


    result = GC2155_IsiSetIntegrationTimeIss( handle, NewIntegrationTime, pSetIntegrationTime, pNumberOfFramesToSkip );
    result = GC2155_IsiSetGainIss( handle, NewGain, pSetGain );

    TRACE( GC2155_DEBUG, "%s: set: g=%f, Ti=%f, skip=%d\n", __FUNCTION__, *pSetGain, *pSetIntegrationTime, *pNumberOfFramesToSkip );
    TRACE( GC2155_INFO, "%s: (exit)\n", __FUNCTION__);

    return ( result );
}



/*****************************************************************************/
/**
 *          GC2155_IsiGetCurrentExposureIss
 *
 * @brief   Returns the currently adjusted AE values
 *
 * @param   handle                  GC2155 sensor instance handle
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
RESULT GC2155_IsiGetCurrentExposureIss
(
    IsiSensorHandle_t   handle,
    float               *pSetGain,
    float               *pSetIntegrationTime
)
{
    GC2155_Context_t *pGC2155Ctx = (GC2155_Context_t *)handle;

    RESULT result = RET_SUCCESS;

    uint32_t RegValue = 0;

    TRACE( GC2155_INFO, "%s: (enter)\n", __FUNCTION__);

    if ( pGC2155Ctx == NULL )
    {
        TRACE( GC2155_ERROR, "%s: Invalid sensor handle (NULL pointer detected)\n", __FUNCTION__ );
        return ( RET_WRONG_HANDLE );
    }

    if ( (pSetGain == NULL) || (pSetIntegrationTime == NULL) )
    {
        return ( RET_NULL_POINTER );
    }

    *pSetGain            = pGC2155Ctx->AecCurGain;
    *pSetIntegrationTime = pGC2155Ctx->AecCurIntegrationTime;

    TRACE( GC2155_INFO, "%s: (exit)\n", __FUNCTION__);

    return ( result );
}



/*****************************************************************************/
/**
 *          GC2155_IsiGetResolutionIss
 *
 * @brief   Reads integration time values from the image sensor module.
 *
 * @param   handle                  sensor instance handle
 * @param   pSettResolution         set resolution
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
RESULT GC2155_IsiGetResolutionIss
(
    IsiSensorHandle_t   handle,
    uint32_t            *pSetResolution
)
{
    GC2155_Context_t *pGC2155Ctx = (GC2155_Context_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( GC2155_INFO, "%s: (enter)\n", __FUNCTION__);

    if ( pGC2155Ctx == NULL )
    {
        TRACE( GC2155_ERROR, "%s: Invalid sensor handle (NULL pointer detected)\n", __FUNCTION__ );
        return ( RET_WRONG_HANDLE );
    }

    if ( pSetResolution == NULL )
    {
        return ( RET_NULL_POINTER );
    }

    *pSetResolution = pGC2155Ctx->Config.Resolution;

    TRACE( GC2155_INFO, "%s: (exit)\n", __FUNCTION__);

    return ( result );
}



/*****************************************************************************/
/**
 *          GC2155_IsiGetAfpsInfoHelperIss
 *
 * @brief   Calc AFPS sub resolution settings for the given resolution
 *
 * @param   pGC2155Ctx             GC2155 sensor instance (dummy!) context
 * @param   Resolution              Any supported resolution to query AFPS params for
 * @param   pAfpsInfo               Reference of AFPS info structure to write the results to
 * @param   AfpsStageIdx            Index of current AFPS stage to use
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 *
 *****************************************************************************/
static RESULT GC2155_IsiGetAfpsInfoHelperIss(
    GC2155_Context_t   *pGC2155Ctx,
    uint32_t            Resolution,
    IsiAfpsInfo_t*      pAfpsInfo,
    uint32_t            AfpsStageIdx
)
{
    RESULT result = RET_SUCCESS;

    TRACE( GC2155_INFO, "%s: (enter)\n", __FUNCTION__);

    DCT_ASSERT(pGC2155Ctx != NULL);
    DCT_ASSERT(pAfpsInfo != NULL);
    DCT_ASSERT(AfpsStageIdx <= ISI_NUM_AFPS_STAGES);

    // update resolution in copy of config in context
    pGC2155Ctx->Config.Resolution = Resolution;

    // tell sensor about that
    result = GC2155_SetupOutputWindow( pGC2155Ctx, &pGC2155Ctx->Config );
    if ( result != RET_SUCCESS )
    {
        TRACE( GC2155_ERROR, "%s: SetupOutputWindow failed for resolution ID %08x.\n", __FUNCTION__, Resolution);
        return ( result );
    }

    // update limits & stuff (reset current & old settings)
    result = GC2155_AecSetModeParameters( pGC2155Ctx, &pGC2155Ctx->Config );
    if ( result != RET_SUCCESS )
    {
        TRACE( GC2155_ERROR, "%s: AecSetModeParameters failed for resolution ID %08x.\n", __FUNCTION__, Resolution);
        return ( result );
    }

    // take over params
    pAfpsInfo->Stage[AfpsStageIdx].Resolution = Resolution;
    pAfpsInfo->Stage[AfpsStageIdx].MaxIntTime = pGC2155Ctx->AecMaxIntegrationTime;
    pAfpsInfo->AecMinGain           = pGC2155Ctx->AecMinGain;
    pAfpsInfo->AecMaxGain           = pGC2155Ctx->AecMaxGain;
    pAfpsInfo->AecMinIntTime        = pGC2155Ctx->AecMinIntegrationTime;
    pAfpsInfo->AecMaxIntTime        = pGC2155Ctx->AecMaxIntegrationTime;
    pAfpsInfo->AecSlowestResolution = Resolution;

    TRACE( GC2155_INFO, "%s: (exit)\n", __FUNCTION__);

    return ( result );
}

/*****************************************************************************/
/**
 *          GC2155_IsiGetAfpsInfoIss
 *
 * @brief   Returns the possible AFPS sub resolution settings for the given resolution series
 *
 * @param   handle                  GC2155 sensor instance handle
 * @param   Resolution              Any resolution within the AFPS group to query;
 *                                  0 (zero) to use the currently configured resolution
 * @param   pAfpsInfo               Reference of AFPS info structure to store the results
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NULL_POINTER
 * @retval  RET_NOTSUPP
 *
 *****************************************************************************/
RESULT GC2155_IsiGetAfpsInfoIss(
    IsiSensorHandle_t   handle,
    uint32_t            Resolution,
    IsiAfpsInfo_t*      pAfpsInfo
)
{
    GC2155_Context_t *pGC2155Ctx = (GC2155_Context_t *)handle;

    RESULT result = RET_SUCCESS;

    uint32_t RegValue = 0;

    TRACE( GC2155_INFO, "%s: (enter)\n", __FUNCTION__);

    if ( pGC2155Ctx == NULL )
    {
        TRACE( GC2155_ERROR, "%s: Invalid sensor handle (NULL pointer detected)\n", __FUNCTION__ );
        return ( RET_WRONG_HANDLE );
    }

    if ( pAfpsInfo == NULL )
    {
        return ( RET_NULL_POINTER );
    }

    // use currently configured resolution?
    if (Resolution == 0)
    {
        Resolution = pGC2155Ctx->Config.Resolution;
    }

    TRACE( GC2155_INFO, "%s: (exit)\n", __FUNCTION__);

    return ( result );
}



/*****************************************************************************/
/**
 *          GC2155_IsiGetCalibKFactor
 *
 * @brief   Returns the GC2155 specific K-Factor
 *
 * @param   handle       GC2155 sensor instance handle
 * @param   pIsiKFactor  Pointer to Pointer receiving the memory address
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
static RESULT GC2155_IsiGetCalibKFactor
(
    IsiSensorHandle_t   handle,
    Isi1x1FloatMatrix_t **pIsiKFactor
)
{
    GC2155_Context_t *pGC2155Ctx = (GC2155_Context_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( GC2155_INFO, "%s: (enter)\n", __FUNCTION__);

    if ( pGC2155Ctx == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    if ( pIsiKFactor == NULL )
    {
        return ( RET_NULL_POINTER );
    }

    *pIsiKFactor = NULL;

    TRACE( GC2155_INFO, "%s: (exit)\n", __FUNCTION__);

    return ( result );
}


/*****************************************************************************/
/**
 *          GC2155_IsiGetCalibPcaMatrix
 *
 * @brief   Returns the GC2155 specific PCA-Matrix
 *
 * @param   handle          GC2155 sensor instance handle
 * @param   pIsiPcaMatrix   Pointer to Pointer receiving the memory address
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
static RESULT GC2155_IsiGetCalibPcaMatrix
(
    IsiSensorHandle_t   handle,
    Isi3x2FloatMatrix_t **pIsiPcaMatrix
)
{
    GC2155_Context_t *pGC2155Ctx = (GC2155_Context_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( GC2155_INFO, "%s: (enter)\n", __FUNCTION__);

    if ( pGC2155Ctx == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    if ( pIsiPcaMatrix == NULL )
    {
        return ( RET_NULL_POINTER );
    }

    *pIsiPcaMatrix = NULL;

    TRACE( GC2155_INFO, "%s: (exit)\n", __FUNCTION__);

    return ( result );
}



/*****************************************************************************/
/**
 *          GC2155_IsiGetCalibSvdMeanValue
 *
 * @brief   Returns the sensor specific SvdMean-Vector
 *
 * @param   handle              GC2155 sensor instance handle
 * @param   pIsiSvdMeanValue    Pointer to Pointer receiving the memory address
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
static RESULT GC2155_IsiGetCalibSvdMeanValue
(
    IsiSensorHandle_t   handle,
    Isi3x1FloatMatrix_t **pIsiSvdMeanValue
)
{
    IsiSensorContext_t *pSensorCtx = (IsiSensorContext_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( GC2155_INFO, "%s: (enter)\n", __FUNCTION__);

    if ( pSensorCtx == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    if ( pIsiSvdMeanValue == NULL )
    {
        return ( RET_NULL_POINTER );
    }

    *pIsiSvdMeanValue = NULL;

    TRACE( GC2155_INFO, "%s: (exit)\n", __FUNCTION__);

    return ( result );
}



/*****************************************************************************/
/**
 *          GC2155_IsiGetCalibSvdMeanValue
 *
 * @brief   Returns a pointer to the sensor specific centerline, a straight
 *          line in Hesse normal form in Rg/Bg colorspace
 *
 * @param   handle              GC2155 sensor instance handle
 * @param   pIsiSvdMeanValue    Pointer to Pointer receiving the memory address
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
static RESULT GC2155_IsiGetCalibCenterLine
(
    IsiSensorHandle_t   handle,
    IsiLine_t           **ptIsiCenterLine
)
{
    IsiSensorContext_t *pSensorCtx = (IsiSensorContext_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( GC2155_INFO, "%s: (enter)\n", __FUNCTION__);

    if ( pSensorCtx == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    if ( ptIsiCenterLine == NULL )
    {
        return ( RET_NULL_POINTER );
    }

    *ptIsiCenterLine = NULL;

    TRACE( GC2155_INFO, "%s: (exit)\n", __FUNCTION__);

    return ( result );
}



/*****************************************************************************/
/**
 *          GC2155_IsiGetCalibClipParam
 *
 * @brief   Returns a pointer to the sensor specific arrays for Rg/Bg color
 *          space clipping
 *
 * @param   handle              GC2155 sensor instance handle
 * @param   pIsiSvdMeanValue    Pointer to Pointer receiving the memory address
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
static RESULT GC2155_IsiGetCalibClipParam
(
    IsiSensorHandle_t   handle,
    IsiAwbClipParm_t    **pIsiClipParam
)
{
    IsiSensorContext_t *pSensorCtx = (IsiSensorContext_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( GC2155_INFO, "%s: (enter)\n", __FUNCTION__);

    if ( pSensorCtx == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    if ( pIsiClipParam == NULL )
    {
        return ( RET_NULL_POINTER );
    }

    *pIsiClipParam = NULL;

    TRACE( GC2155_INFO, "%s: (exit)\n", __FUNCTION__);

    return ( result );
}



/*****************************************************************************/
/**
 *          GC2155_IsiGetCalibGlobalFadeParam
 *
 * @brief   Returns a pointer to the sensor specific arrays for AWB out of
 *          range handling
 *
 * @param   handle              GC2155 sensor instance handle
 * @param   pIsiSvdMeanValue    Pointer to Pointer receiving the memory address
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
static RESULT GC2155_IsiGetCalibGlobalFadeParam
(
    IsiSensorHandle_t       handle,
    IsiAwbGlobalFadeParm_t  **ptIsiGlobalFadeParam
)
{
    IsiSensorContext_t *pSensorCtx = (IsiSensorContext_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( GC2155_INFO, "%s: (enter)\n", __FUNCTION__);

    if ( pSensorCtx == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    if ( ptIsiGlobalFadeParam == NULL )
    {
        return ( RET_NULL_POINTER );
    }

    *ptIsiGlobalFadeParam = NULL;

    TRACE( GC2155_INFO, "%s: (exit)\n", __FUNCTION__);

    return ( result );
}



/*****************************************************************************/
/**
 *          GC2155_IsiGetCalibFadeParam
 *
 * @brief   Returns a pointer to the sensor specific arrays for near white
 *          pixel parameter calculations
 *
 * @param   handle              GC2155 sensor instance handle
 * @param   pIsiSvdMeanValue    Pointer to Pointer receiving the memory address
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
static RESULT GC2155_IsiGetCalibFadeParam
(
    IsiSensorHandle_t   handle,
    IsiAwbFade2Parm_t   **ptIsiFadeParam
)
{
    IsiSensorContext_t *pSensorCtx = (IsiSensorContext_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( GC2155_INFO, "%s: (enter)\n", __FUNCTION__);

    if ( pSensorCtx == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    if ( ptIsiFadeParam == NULL )
    {
        return ( RET_NULL_POINTER );
    }

    *ptIsiFadeParam = NULL;

    TRACE( GC2155_INFO, "%s: (exit)\n", __FUNCTION__);

    return ( result );
}

/*****************************************************************************/
/**
 *          GC2155_IsiGetIlluProfile
 *
 * @brief   Returns a pointer to illumination profile idetified by CieProfile
 *          bitmask
 *
 * @param   handle              sensor instance handle
 * @param   CieProfile
 * @param   ptIsiIlluProfile    Pointer to Pointer receiving the memory address
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
static RESULT GC2155_IsiGetIlluProfile
(
    IsiSensorHandle_t   handle,
    const uint32_t      CieProfile,
    IsiIlluProfile_t    **ptIsiIlluProfile
)
{
    GC2155_Context_t *pGC2155Ctx = (GC2155_Context_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( GC2155_INFO, "%s: (enter)\n", __FUNCTION__);

    if ( pGC2155Ctx == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    if ( ptIsiIlluProfile == NULL )
    {
        return ( RET_NULL_POINTER );
    }
    TRACE( GC2155_INFO, "%s: (exit)\n", __FUNCTION__);

    return ( result );
}



/*****************************************************************************/
/**
 *          GC2155_IsiGetLscMatrixTable
 *
 * @brief   Returns a pointer to illumination profile idetified by CieProfile
 *          bitmask
 *
 * @param   handle              sensor instance handle
 * @param   CieProfile
 * @param   ptIsiIlluProfile    Pointer to Pointer receiving the memory address
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
static RESULT GC2155_IsiGetLscMatrixTable
(
    IsiSensorHandle_t   handle,
    const uint32_t      CieProfile,
    IsiLscMatrixTable_t **pLscMatrixTable
)
{
    GC2155_Context_t *pGC2155Ctx = (GC2155_Context_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( GC2155_INFO, "%s: (enter)\n", __FUNCTION__);

    if ( pGC2155Ctx == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    if ( pLscMatrixTable == NULL )
    {
        return ( RET_NULL_POINTER );
    }
    else

    TRACE( GC2155_INFO, "%s: (exit)\n", __FUNCTION__);

    return ( result );
}


/*****************************************************************************/
/**
 *          GC2155_IsiMdiInitMotoDriveMds
 *
 * @brief   General initialisation tasks like I/O initialisation.
 *
 * @param   handle              GC2155 sensor instance handle
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
static RESULT GC2155_IsiMdiInitMotoDriveMds
(
    IsiSensorHandle_t   handle
)
{
    GC2155_Context_t *pGC2155Ctx = (GC2155_Context_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( GC2155_INFO, "%s: (enter)\n", __FUNCTION__);

    if ( pGC2155Ctx == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    TRACE( GC2155_INFO, "%s: (exit)\n", __FUNCTION__);

    return ( result );
}



/*****************************************************************************/
/**
 *          GC2155_IsiMdiSetupMotoDrive
 *
 * @brief   Setup of the MotoDrive and return possible max step.
 *
 * @param   handle          GC2155 sensor instance handle
 *          pMaxStep        pointer to variable to receive the maximum
 *                          possible focus step
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
static RESULT GC2155_IsiMdiSetupMotoDrive
(
    IsiSensorHandle_t   handle,
    uint32_t            *pMaxStep
)
{
    GC2155_Context_t *pGC2155Ctx = (GC2155_Context_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( GC2155_INFO, "%s: (enter)\n", __FUNCTION__);

    if ( pGC2155Ctx == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    if ( pMaxStep == NULL )
    {
        return ( RET_NULL_POINTER );
    }


    TRACE( GC2155_INFO, "%s: (exit)\n", __FUNCTION__);

    return ( result );
}



/*****************************************************************************/
/**
 *          GC2155_IsiMdiFocusSet
 *
 * @brief   Drives the lens system to a certain focus point.
 *
 * @param   handle          GC2155 sensor instance handle
 *          AbsStep         absolute focus point to apply
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
static RESULT GC2155_IsiMdiFocusSet
(
    IsiSensorHandle_t   handle,
    const uint32_t      Position
)
{
    GC2155_Context_t *pGC2155Ctx = (GC2155_Context_t *)handle;

    RESULT result = RET_SUCCESS;

    uint32_t nPosition;
    uint8_t  data[2] = { 0, 0 };

    TRACE( GC2155_INFO, "%s: (enter)\n", __FUNCTION__);

    if ( pGC2155Ctx == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }
    RETURN_RESULT_IF_DIFFERENT( RET_SUCCESS, result );

    TRACE( GC2155_INFO, "%s: (exit)\n", __FUNCTION__);

    return ( result );
}



/*****************************************************************************/
/**
 *          GC2155_IsiMdiFocusGet
 *
 * @brief   Retrieves the currently applied focus point.
 *
 * @param   handle          GC2155 sensor instance handle
 *          pAbsStep        pointer to a variable to receive the current
 *                          focus point
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
static RESULT GC2155_IsiMdiFocusGet
(
    IsiSensorHandle_t   handle,
    uint32_t            *pAbsStep
)
{
    GC2155_Context_t *pGC2155Ctx = (GC2155_Context_t *)handle;

    RESULT result = RET_SUCCESS;
    uint8_t  data[2] = { 0, 0 };

    TRACE( GC2155_INFO, "%s: (enter)\n", __FUNCTION__);

    if ( pGC2155Ctx == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    if ( pAbsStep == NULL )
    {
        return ( RET_NULL_POINTER );
    }

    TRACE( GC2155_INFO, "%s: (exit)\n", __FUNCTION__);

    return ( result );
}



/*****************************************************************************/
/**
 *          GC2155_IsiMdiFocusCalibrate
 *
 * @brief   Triggers a forced calibration of the focus hardware.
 *
 * @param   handle          GC2155 sensor instance handle
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
static RESULT GC2155_IsiMdiFocusCalibrate
(
    IsiSensorHandle_t   handle
)
{
    GC2155_Context_t *pGC2155Ctx = (GC2155_Context_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( GC2155_INFO, "%s: (enter)\n", __FUNCTION__);

    if ( pGC2155Ctx == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    TRACE( GC2155_INFO, "%s: (exit)\n", __FUNCTION__);

    return ( result );
}



/*****************************************************************************/
/**
 *          GC2155_IsiActivateTestPattern
 *
 * @brief   Triggers a forced calibration of the focus hardware.
 *
 * @param   handle          GC2155 sensor instance handle
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NULL_POINTER
 *
 ******************************************************************************/
static RESULT GC2155_IsiActivateTestPattern
(
    IsiSensorHandle_t   handle,
    const bool_t        enable
)
{
    GC2155_Context_t *pGC2155Ctx = (GC2155_Context_t *)handle;

    RESULT result = RET_SUCCESS;

    return ( result );
}



/*****************************************************************************/
/**
 *          GC2155_IsiGetSensorMipiInfoIss
 *
 * @brief   Triggers a forced calibration of the focus hardware.
 *
 * @param   handle          GC2155 sensor instance handle
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NULL_POINTER
 *
 ******************************************************************************/
static RESULT GC2155_IsiGetSensorMipiInfoIss
(
    IsiSensorHandle_t   handle,
    IsiSensorMipiInfo   *ptIsiSensorMipiInfo
)
{
    GC2155_Context_t *pGC2155Ctx = (GC2155_Context_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( GC2155_INFO, "%s: (enter)\n", __FUNCTION__);

    if ( pGC2155Ctx == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }


    if ( ptIsiSensorMipiInfo == NULL )
    {
        return ( result );
    }

    TRACE( GC2155_INFO, "%s: (exit)\n", __FUNCTION__);

    return ( result );
}

static RESULT GC2155_IsiGetSensorIsiVersion
(  IsiSensorHandle_t   handle,
   unsigned int*     pVersion
)
{
    GC2155_Context_t *pGC2155Ctx = (GC2155_Context_t *)handle;

    RESULT result = RET_SUCCESS;


    TRACE( GC2155_INFO, "%s: (enter)\n", __FUNCTION__);

    if ( pGC2155Ctx == NULL )
    {
    	TRACE( GC2155_ERROR, "%s: pGC2155Ctx IS NULL\n", __FUNCTION__);
        return ( RET_WRONG_HANDLE );
    }

	if(pVersion == NULL)
	{
		TRACE( GC2155_ERROR, "%s: pVersion IS NULL\n", __FUNCTION__);
        return ( RET_WRONG_HANDLE );
	}

	*pVersion = CONFIG_ISI_VERSION;
	return result;
}

/*****************************************************************************/
/**
 *          GC2155_IsiGetSensorIss
 *
 * @brief   fills in the correct pointers for the sensor description struct
 *
 * @param   param1      pointer to sensor description struct
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
RESULT GC2155_IsiGetSensorIss
(
    IsiSensor_t *pIsiSensor
)
{
    RESULT result = RET_SUCCESS;

    TRACE( GC2155_INFO, "%s (enter)\n", __FUNCTION__);

    if ( pIsiSensor != NULL )
    {
        pIsiSensor->pszName                             = GC2155_g_acName;
        pIsiSensor->pRegisterTable                      = GC2155_g_aRegDescription;
        pIsiSensor->pIsiSensorCaps                      = &GC2155_g_IsiSensorDefaultConfig;
				pIsiSensor->pIsiGetSensorIsiVer									= GC2155_IsiGetSensorIsiVersion;
        pIsiSensor->pIsiCreateSensorIss                 = GC2155_IsiCreateSensorIss;
        pIsiSensor->pIsiReleaseSensorIss                = GC2155_IsiReleaseSensorIss;
        pIsiSensor->pIsiGetCapsIss                      = GC2155_IsiGetCapsIss;
        pIsiSensor->pIsiSetupSensorIss                  = GC2155_IsiSetupSensorIss;
        pIsiSensor->pIsiChangeSensorResolutionIss       = GC2155_IsiChangeSensorResolutionIss;
        pIsiSensor->pIsiSensorSetStreamingIss           = GC2155_IsiSensorSetStreamingIss;
        pIsiSensor->pIsiSensorSetPowerIss               = GC2155_IsiSensorSetPowerIss;
        pIsiSensor->pIsiCheckSensorConnectionIss        = GC2155_IsiCheckSensorConnectionIss;
        pIsiSensor->pIsiGetSensorRevisionIss            = GC2155_IsiGetSensorRevisionIss;
        pIsiSensor->pIsiRegisterReadIss                 = GC2155_IsiRegReadIss;
        pIsiSensor->pIsiRegisterWriteIss                = GC2155_IsiRegWriteIss;

        /* AEC functions */
        pIsiSensor->pIsiExposureControlIss              = GC2155_IsiExposureControlIss;
        pIsiSensor->pIsiGetGainLimitsIss                = GC2155_IsiGetGainLimitsIss;
        pIsiSensor->pIsiGetIntegrationTimeLimitsIss     = GC2155_IsiGetIntegrationTimeLimitsIss;
        pIsiSensor->pIsiGetCurrentExposureIss           = GC2155_IsiGetCurrentExposureIss;
        pIsiSensor->pIsiGetGainIss                      = GC2155_IsiGetGainIss;
        pIsiSensor->pIsiGetGainIncrementIss             = GC2155_IsiGetGainIncrementIss;
        pIsiSensor->pIsiSetGainIss                      = GC2155_IsiSetGainIss;
        pIsiSensor->pIsiGetIntegrationTimeIss           = GC2155_IsiGetIntegrationTimeIss;
        pIsiSensor->pIsiGetIntegrationTimeIncrementIss  = GC2155_IsiGetIntegrationTimeIncrementIss;
        pIsiSensor->pIsiSetIntegrationTimeIss           = GC2155_IsiSetIntegrationTimeIss;
        pIsiSensor->pIsiGetResolutionIss                = GC2155_IsiGetResolutionIss;
        pIsiSensor->pIsiGetAfpsInfoIss                  = GC2155_IsiGetAfpsInfoIss;

        /* AWB specific functions */
        pIsiSensor->pIsiGetCalibKFactor                 = GC2155_IsiGetCalibKFactor;
        pIsiSensor->pIsiGetCalibPcaMatrix               = GC2155_IsiGetCalibPcaMatrix;
        pIsiSensor->pIsiGetCalibSvdMeanValue            = GC2155_IsiGetCalibSvdMeanValue;
        pIsiSensor->pIsiGetCalibCenterLine              = GC2155_IsiGetCalibCenterLine;
        pIsiSensor->pIsiGetCalibClipParam               = GC2155_IsiGetCalibClipParam;
        pIsiSensor->pIsiGetCalibGlobalFadeParam         = GC2155_IsiGetCalibGlobalFadeParam;
        pIsiSensor->pIsiGetCalibFadeParam               = GC2155_IsiGetCalibFadeParam;
        pIsiSensor->pIsiGetIlluProfile                  = GC2155_IsiGetIlluProfile;
        pIsiSensor->pIsiGetLscMatrixTable               = GC2155_IsiGetLscMatrixTable;

        /* AF functions */
        pIsiSensor->pIsiMdiInitMotoDriveMds             = GC2155_IsiMdiInitMotoDriveMds;
        pIsiSensor->pIsiMdiSetupMotoDrive               = GC2155_IsiMdiSetupMotoDrive;
        pIsiSensor->pIsiMdiFocusSet                     = GC2155_IsiMdiFocusSet;
        pIsiSensor->pIsiMdiFocusGet                     = GC2155_IsiMdiFocusGet;
        pIsiSensor->pIsiMdiFocusCalibrate               = GC2155_IsiMdiFocusCalibrate;

        /* MIPI */
        pIsiSensor->pIsiGetSensorMipiInfoIss            = GC2155_IsiGetSensorMipiInfoIss;

        /* Testpattern */
        pIsiSensor->pIsiActivateTestPattern             = GC2155_IsiActivateTestPattern;
    }
    else
    {
        result = RET_NULL_POINTER;
    }

    TRACE( GC2155_INFO, "%s (exit)\n", __FUNCTION__);

    return ( result );
}

static RESULT GC2155_IsiGetSensorI2cInfo(sensor_i2c_info_t** pdata)
{
    sensor_i2c_info_t* pSensorI2cInfo;

    pSensorI2cInfo = ( sensor_i2c_info_t * )malloc ( sizeof (sensor_i2c_info_t) );

    if ( pSensorI2cInfo == NULL )
    {
        TRACE( GC2155_ERROR,  "%s: Can't allocate ov14825 context\n",  __FUNCTION__ );
        return ( RET_OUTOFMEM );
    }
    MEMSET( pSensorI2cInfo, 0, sizeof( sensor_i2c_info_t ) );

    
    pSensorI2cInfo->i2c_addr = GC2155_SLAVE_ADDR;
    pSensorI2cInfo->soft_reg_addr = GC2155_SOFTWARE_RST;
    pSensorI2cInfo->soft_reg_value = GC2155_SOFTWARE_RST_DATA;
    pSensorI2cInfo->reg_size = GC2155_I2C_NR_ADR_BYTES;
    pSensorI2cInfo->value_size = GC2155_I2C_NR_DAT_BYTES;

    {
        IsiSensorCaps_t Caps;
        sensor_caps_t *pCaps;
        uint32_t lanes,i;
        
        ListInit(&pSensorI2cInfo->lane_res[0]);
        ListInit(&pSensorI2cInfo->lane_res[1]);
        ListInit(&pSensorI2cInfo->lane_res[2]);
        
        Caps.Index = 0;            
        while(GC2155_IsiGetCapsIssInternal(&Caps)==RET_SUCCESS) {
            pCaps = malloc(sizeof(sensor_caps_t));
            if (pCaps != NULL) {
                memcpy(&pCaps->caps,&Caps,sizeof(IsiSensorCaps_t));
                ListPrepareItem(pCaps);
                ListAddTail(&pSensorI2cInfo->lane_res[0], pCaps);
            }
            Caps.Index++;
        }
    }
    
    ListInit(&pSensorI2cInfo->chipid_info);

    sensor_chipid_info_t* pChipIDInfo_H = (sensor_chipid_info_t *) malloc( sizeof(sensor_chipid_info_t) );
    if ( !pChipIDInfo_H )
    {
        return RET_OUTOFMEM;
    }
    MEMSET( pChipIDInfo_H, 0, sizeof(*pChipIDInfo_H) );    
    pChipIDInfo_H->chipid_reg_addr = GC2155_CHIP_ID_HIGH_BYTE;  
    pChipIDInfo_H->chipid_reg_value = GC2155_CHIP_ID_HIGH_BYTE_DEFAULT;
    ListPrepareItem( pChipIDInfo_H );
    ListAddTail( &pSensorI2cInfo->chipid_info, pChipIDInfo_H );

    sensor_chipid_info_t* pChipIDInfo_M = (sensor_chipid_info_t *) malloc( sizeof(sensor_chipid_info_t) );
    if ( !pChipIDInfo_M )
    {
        return RET_OUTOFMEM;
    }
    MEMSET( pChipIDInfo_M, 0, sizeof(*pChipIDInfo_M) ); 
    pChipIDInfo_M->chipid_reg_addr = GC2155_CHIP_ID_MIDDLE_BYTE;
    pChipIDInfo_M->chipid_reg_value = GC2155_CHIP_ID_MIDDLE_BYTE_DEFAULT;
    ListPrepareItem( pChipIDInfo_M );
    ListAddTail( &pSensorI2cInfo->chipid_info, pChipIDInfo_M );
    
    sensor_chipid_info_t* pChipIDInfo_L = (sensor_chipid_info_t *) malloc( sizeof(sensor_chipid_info_t) );
    if ( !pChipIDInfo_L )
    {
        return RET_OUTOFMEM;
    }
    MEMSET( pChipIDInfo_L, 0, sizeof(*pChipIDInfo_L) ); 
    pChipIDInfo_L->chipid_reg_addr = GC2155_CHIP_ID_LOW_BYTE;
    pChipIDInfo_L->chipid_reg_value = GC2155_CHIP_ID_LOW_BYTE_DEFAULT;
    ListPrepareItem( pChipIDInfo_L );
    ListAddTail( &pSensorI2cInfo->chipid_info, pChipIDInfo_L );

	//oyyf sensor drv version
	pSensorI2cInfo->sensor_drv_version = CONFIG_SENSOR_DRV_VERSION;
	
    *pdata = pSensorI2cInfo;
    return RET_SUCCESS;
}

/******************************************************************************
 * See header file for detailed comment.
 *****************************************************************************/

/*****************************************************************************/
/**
 */
/*****************************************************************************/
IsiCamDrvConfig_t IsiCamDrvConfig =
{
    0,
    GC2155_IsiGetSensorIss,
    {
        0,                      /**< IsiSensor_t.pszName */
        0,                      /**< IsiSensor_t.pRegisterTable */
        0,                      /**< IsiSensor_t.pIsiSensorCaps */
        0,						/**< IsiSensor_t.pIsiGetSensorIsiVer_t>*/   //oyyf add
        0,                      /**< IsiSensor_t.pIsiGetSensorTuningXmlVersion_t>*/   //oyyf add
        0,                      /**< IsiSensor_t.pIsiWhiteBalanceIlluminationChk>*/   //ddl@rock-chips.com 
        0,                      /**< IsiSensor_t.pIsiWhiteBalanceIlluminationSet>*/   //ddl@rock-chips.com
        0,                      /**< IsiSensor_t.pIsiCheckOTPInfo>*/  //zyc
		0,						/**< IsiSensor_t.pIsiSetSensorOTPInfo>*/  //zyl
		0,						/**< IsiSensor_t.pIsiEnableSensorOTP>*/  //zyl
        0,                      /**< IsiSensor_t.pIsiCreateSensorIss */
        0,                      /**< IsiSensor_t.pIsiReleaseSensorIss */
        0,                      /**< IsiSensor_t.pIsiGetCapsIss */
        0,                      /**< IsiSensor_t.pIsiSetupSensorIss */
        0,                      /**< IsiSensor_t.pIsiChangeSensorResolutionIss */
        0,                      /**< IsiSensor_t.pIsiSensorSetStreamingIss */
        0,                      /**< IsiSensor_t.pIsiSensorSetPowerIss */
        0,                      /**< IsiSensor_t.pIsiCheckSensorConnectionIss */
        0,                      /**< IsiSensor_t.pIsiGetSensorRevisionIss */
        0,                      /**< IsiSensor_t.pIsiRegisterReadIss */
        0,                      /**< IsiSensor_t.pIsiRegisterWriteIss */
        0,                      /**< IsiSensor_t.pIsiIsEvenFieldIss */
        0,                      /**< IsiSensor_t.pIsiExposureControlIss */
        0,                      /**< IsiSensor_t.pIsiGetGainLimitsIss */
        0,                      /**< IsiSensor_t.pIsiGetIntegrationTimeLimitsIss */
        0,                      /**< IsiSensor_t.pIsiGetCurrentExposureIss */
        0,                      /**< IsiSensor_t.pIsiGetGainIss */
        0,                      /**< IsiSensor_t.pIsiGetGainIncrementIss */
        0,                      /**< IsiSensor_t.pIsiSetGainIss */
        0,                      /**< IsiSensor_t.pIsiGetIntegrationTimeIss */
        0,                      /**< IsiSensor_t.pIsiGetIntegrationTimeIncrementIss */
        0,                      /**< IsiSensor_t.pIsiSetIntegrationTimeIss */
        0,                      /**< IsiSensor_t.pIsiGetResolutionIss */
        0,                      /**< IsiSensor_t.pIsiGetAfpsInfoIss */

        0,                      /**< IsiSensor_t.pIsiGetCalibKFactor */
        0,                      /**< IsiSensor_t.pIsiGetCalibPcaMatrix */
        0,                      /**< IsiSensor_t.pIsiGetCalibSvdMeanValue */
        0,                      /**< IsiSensor_t.pIsiGetCalibCenterLine */
        0,                      /**< IsiSensor_t.pIsiGetCalibClipParam */
        0,                      /**< IsiSensor_t.pIsiGetCalibGlobalFadeParam */
        0,                      /**< IsiSensor_t.pIsiGetCalibFadeParam */
        0,                      /**< IsiSensor_t.pIsiGetIlluProfile */
        0,                      /**< IsiSensor_t.pIsiGetLscMatrixTable */

        0,                      /**< IsiSensor_t.pIsiMdiInitMotoDriveMds */
        0,                      /**< IsiSensor_t.pIsiMdiSetupMotoDrive */
        0,                      /**< IsiSensor_t.pIsiMdiFocusSet */
        0,                      /**< IsiSensor_t.pIsiMdiFocusGet */
        0,                      /**< IsiSensor_t.pIsiMdiFocusCalibrate */

        0,                      /**< IsiSensor_t.pIsiGetSensorMipiInfoIss */

        0,                      /**< IsiSensor_t.pIsiActivateTestPattern */
        0,
        0,						/**< IsiSensor_t.pIsiGetColorIss */
    },
    GC2155_IsiGetSensorI2cInfo,
};



