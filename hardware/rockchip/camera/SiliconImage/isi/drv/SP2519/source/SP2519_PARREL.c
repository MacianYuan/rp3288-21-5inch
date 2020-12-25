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
 * @file SP2519.c
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

#include "SP2519_priv.h"


#define CC_OFFSET_SCALING  2.0f
#define I2C_COMPLIANT_STARTBIT 1U

/******************************************************************************
 * local macro definitions
 *****************************************************************************/
CREATE_TRACER( SP2519_INFO , "SP2519: ", INFO,    1U );
CREATE_TRACER( SP2519_WARN , "SP2519: ", WARNING, 1U );
CREATE_TRACER( SP2519_ERROR, "SP2519: ", ERROR,   1U );

CREATE_TRACER( SP2519_DEBUG, "SP2519: ", INFO,     1U );

CREATE_TRACER( SP2519_REG_INFO , "SP2519: ", INFO, 1);
CREATE_TRACER( SP2519_REG_DEBUG, "SP2519: ", INFO, 0U );

#define SP2519_SLAVE_ADDR       0x60U                           /**< i2c slave address of the SP2519 camera sensor */
#define SP2519_SLAVE_ADDR2      0x60U                           
#define SP2519_SLAVE_AF_ADDR    0x00U                        /**< i2c slave address of the SP2519 integrated AD5820 */



/******************************************************************************
 * local variable declarations
 *****************************************************************************/
const char SP2519_g_acName[] = "SP2519_ SOC_PARREL";
extern const IsiRegDescription_t SP2519_g_aRegDescription[];
extern const IsiRegDescription_t SP2519_g_svga[];
extern const IsiRegDescription_t SP2519_g_1600x1200[];


const IsiSensorCaps_t SP2519_g_IsiSensorDefaultConfig;


#define SP2519_I2C_START_BIT        (I2C_COMPLIANT_STARTBIT)    // I2C bus start condition
#define SP2519_I2C_NR_ADR_BYTES     (1U)                        // 1 byte base address and 2 bytes sub address
#define SP2519_I2C_NR_DAT_BYTES     (1U)                        // 8 bit registers


/******************************************************************************
 * local function prototypes
 *****************************************************************************/
static RESULT SP2519_IsiCreateSensorIss( IsiSensorInstanceConfig_t *pConfig );
static RESULT SP2519_IsiReleaseSensorIss( IsiSensorHandle_t handle );
static RESULT SP2519_IsiGetCapsIss( IsiSensorHandle_t handle, IsiSensorCaps_t *pIsiSensorCaps );
static RESULT SP2519_IsiSetupSensorIss( IsiSensorHandle_t handle, const IsiSensorConfig_t *pConfig );
static RESULT SP2519_IsiSensorSetStreamingIss( IsiSensorHandle_t handle, bool_t on );
static RESULT SP2519_IsiSensorSetPowerIss( IsiSensorHandle_t handle, bool_t on );
static RESULT SP2519_IsiCheckSensorConnectionIss( IsiSensorHandle_t handle );
static RESULT SP2519_IsiGetSensorRevisionIss( IsiSensorHandle_t handle, uint32_t *p_value);

static RESULT SP2519_IsiGetGainLimitsIss( IsiSensorHandle_t handle, float *pMinGain, float *pMaxGain);
static RESULT SP2519_IsiGetIntegrationTimeLimitsIss( IsiSensorHandle_t handle, float *pMinIntegrationTime, float *pMaxIntegrationTime );
static RESULT SP2519_IsiExposureControlIss( IsiSensorHandle_t handle, float NewGain, float NewIntegrationTime, uint8_t *pNumberOfFramesToSkip, float *pSetGain, float *pSetIntegrationTime );
static RESULT SP2519_IsiGetCurrentExposureIss( IsiSensorHandle_t handle, float *pSetGain, float *pSetIntegrationTime );
static RESULT SP2519_IsiGetAfpsInfoIss ( IsiSensorHandle_t handle, uint32_t Resolution, IsiAfpsInfo_t* pAfpsInfo);
static RESULT SP2519_IsiGetGainIss( IsiSensorHandle_t handle, float *pSetGain );
static RESULT SP2519_IsiGetGainIncrementIss( IsiSensorHandle_t handle, float *pIncr );
static RESULT SP2519_IsiSetGainIss( IsiSensorHandle_t handle, float NewGain, float *pSetGain );
static RESULT SP2519_IsiGetIntegrationTimeIss( IsiSensorHandle_t handle, float *pSetIntegrationTime );
static RESULT SP2519_IsiGetIntegrationTimeIncrementIss( IsiSensorHandle_t handle, float *pIncr );
static RESULT SP2519_IsiSetIntegrationTimeIss( IsiSensorHandle_t handle, float NewIntegrationTime, float *pSetIntegrationTime, uint8_t *pNumberOfFramesToSkip );
static RESULT SP2519_IsiGetResolutionIss( IsiSensorHandle_t handle, uint32_t *pSetResolution );


static RESULT SP2519_IsiRegReadIss( IsiSensorHandle_t handle, const uint32_t address, uint32_t *p_value );
static RESULT SP2519_IsiRegWriteIss( IsiSensorHandle_t handle, const uint32_t address, const uint32_t value );

static RESULT SP2519_IsiGetCalibKFactor( IsiSensorHandle_t handle, Isi1x1FloatMatrix_t **pIsiKFactor );
static RESULT SP2519_IsiGetCalibPcaMatrix( IsiSensorHandle_t   handle, Isi3x2FloatMatrix_t **pIsiPcaMatrix );
static RESULT SP2519_IsiGetCalibSvdMeanValue( IsiSensorHandle_t   handle, Isi3x1FloatMatrix_t **pIsiSvdMeanValue );
static RESULT SP2519_IsiGetCalibCenterLine( IsiSensorHandle_t   handle, IsiLine_t  **ptIsiCenterLine);
static RESULT SP2519_IsiGetCalibClipParam( IsiSensorHandle_t   handle, IsiAwbClipParm_t    **pIsiClipParam );
static RESULT SP2519_IsiGetCalibGlobalFadeParam( IsiSensorHandle_t       handle, IsiAwbGlobalFadeParm_t  **ptIsiGlobalFadeParam);
static RESULT SP2519_IsiGetCalibFadeParam( IsiSensorHandle_t   handle, IsiAwbFade2Parm_t   **ptIsiFadeParam);
static RESULT SP2519_IsiGetIlluProfile( IsiSensorHandle_t   handle, const uint32_t CieProfile, IsiIlluProfile_t **ptIsiIlluProfile );

static RESULT SP2519_IsiMdiInitMotoDriveMds( IsiSensorHandle_t handle );
static RESULT SP2519_IsiMdiSetupMotoDrive( IsiSensorHandle_t handle, uint32_t *pMaxStep );
static RESULT SP2519_IsiMdiFocusSet( IsiSensorHandle_t handle, const uint32_t Position );
static RESULT SP2519_IsiMdiFocusGet( IsiSensorHandle_t handle, uint32_t *pAbsStep );
static RESULT SP2519_IsiMdiFocusCalibrate( IsiSensorHandle_t handle );

static RESULT SP2519_IsiGetSensorMipiInfoIss( IsiSensorHandle_t handle, IsiSensorMipiInfo *ptIsiSensorMipiInfo);
static RESULT SP2519_IsiGetSensorIsiVersion(  IsiSensorHandle_t   handle, unsigned int* pVersion);

/*****************************************************************************/
/**
 *          SP2519_IsiCreateSensorIss
 *
 * @brief   This function creates a new SP2519 sensor instance handle.
 *
 * @param   pConfig     configuration structure to create the instance
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_NULL_POINTER
 * @retval  RET_OUTOFMEM
 *
 *****************************************************************************/
static RESULT SP2519_IsiCreateSensorIss
(
    IsiSensorInstanceConfig_t *pConfig
)
{
    RESULT result = RET_SUCCESS;

    SP2519_Context_t *pSP2519Ctx;

    TRACE( SP2519_INFO, "%s (enter)\n", __FUNCTION__);

    if ( (pConfig == NULL) || (pConfig->pSensor ==NULL) )
    {
        return ( RET_NULL_POINTER );
    }

    pSP2519Ctx = ( SP2519_Context_t * )malloc ( sizeof (SP2519_Context_t) );
    if ( pSP2519Ctx == NULL )
    {
        TRACE( SP2519_ERROR,  "%s: Can't allocate ov14825 context\n",  __FUNCTION__ );
        return ( RET_OUTOFMEM );
    }
    MEMSET( pSP2519Ctx, 0, sizeof( SP2519_Context_t ) );

    result = HalAddRef( pConfig->HalHandle );
    if ( result != RET_SUCCESS )
    {
        free ( pSP2519Ctx );
        return ( result );
    }

    pSP2519Ctx->IsiCtx.HalHandle              = pConfig->HalHandle;
    pSP2519Ctx->IsiCtx.HalDevID               = pConfig->HalDevID;
    pSP2519Ctx->IsiCtx.I2cBusNum              = pConfig->I2cBusNum;
    pSP2519Ctx->IsiCtx.SlaveAddress           = ( pConfig->SlaveAddr == 0 ) ? SP2519_SLAVE_ADDR : pConfig->SlaveAddr;
    pSP2519Ctx->IsiCtx.NrOfAddressBytes       = SP2519_I2C_NR_ADR_BYTES;

    pSP2519Ctx->IsiCtx.I2cAfBusNum            = pConfig->I2cAfBusNum;
    pSP2519Ctx->IsiCtx.SlaveAfAddress         = ( pConfig->SlaveAfAddr == 0 ) ? SP2519_SLAVE_AF_ADDR : pConfig->SlaveAfAddr;
    pSP2519Ctx->IsiCtx.NrOfAfAddressBytes     = 0;

    pSP2519Ctx->IsiCtx.pSensor                = pConfig->pSensor;

    pSP2519Ctx->Configured             = BOOL_FALSE;
    pSP2519Ctx->Streaming              = BOOL_FALSE;
    pSP2519Ctx->TestPattern            = BOOL_FALSE;
    pSP2519Ctx->isAfpsRun              = BOOL_FALSE;

    pConfig->hSensor = ( IsiSensorHandle_t )pSP2519Ctx;

    result = HalSetCamConfig( pSP2519Ctx->IsiCtx.HalHandle, pSP2519Ctx->IsiCtx.HalDevID, true, true, false );
    RETURN_RESULT_IF_DIFFERENT( RET_SUCCESS, result );

    result = HalSetClock( pSP2519Ctx->IsiCtx.HalHandle, pSP2519Ctx->IsiCtx.HalDevID, 10000000U);
    RETURN_RESULT_IF_DIFFERENT( RET_SUCCESS, result );

    TRACE( SP2519_INFO, "%s (exit)\n", __FUNCTION__);

    return ( result );
}



/*****************************************************************************/
/**
 *          SP2519_IsiReleaseSensorIss
 *
 * @brief   This function destroys/releases an SP2519 sensor instance.
 *
 * @param   handle      SP2519 sensor instance handle
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 *
 *****************************************************************************/
static RESULT SP2519_IsiReleaseSensorIss
(
    IsiSensorHandle_t handle
)
{
    SP2519_Context_t *pSP2519Ctx = (SP2519_Context_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( SP2519_INFO, "%s (enter)\n", __FUNCTION__);

    if ( pSP2519Ctx == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    (void)SP2519_IsiSensorSetStreamingIss( pSP2519Ctx, BOOL_FALSE );
    (void)SP2519_IsiSensorSetPowerIss( pSP2519Ctx, BOOL_FALSE );

    (void)HalDelRef( pSP2519Ctx->IsiCtx.HalHandle );

    MEMSET( pSP2519Ctx, 0, sizeof( SP2519_Context_t ) );
    free ( pSP2519Ctx );

    TRACE( SP2519_INFO, "%s (exit)\n", __FUNCTION__);

    return ( result );
}



/*****************************************************************************/
/**
 *          SP2519_IsiGetCapsIss
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
static RESULT SP2519_IsiGetCapsIssInternal
(
    IsiSensorCaps_t   *pIsiSensorCaps
)
{

    RESULT result = RET_SUCCESS;
   TRACE( SP2519_INFO, "%s(%d) (enter)\n", __FUNCTION__,__LINE__);
    
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
    
        pIsiSensorCaps->BusWidth        = ISI_BUSWIDTH_8BIT_ZZ;
        pIsiSensorCaps->Mode            = ISI_MODE_BT601;
        pIsiSensorCaps->FieldSelection  = ISI_FIELDSEL_BOTH;
        pIsiSensorCaps->YCSequence      = ISI_YCSEQ_YCBYCR; //ISI_YCSEQ_YCBYCR|ISI_YCSEQ_CBYCRY;           
        pIsiSensorCaps->Conv422         = ISI_CONV422_NOCOSITED;
        pIsiSensorCaps->BPat            = ISI_BPAT_GBGBRGRG;
        pIsiSensorCaps->HPol            = ISI_HPOL_REFPOS;
        pIsiSensorCaps->VPol            = ISI_VPOL_NEG;
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
	TRACE( SP2519_INFO, "%s(%d) (exit)\n", __FUNCTION__,__LINE__);
    return ( result );
}

static RESULT SP2519_IsiGetCapsIss
(
    IsiSensorHandle_t handle,
    IsiSensorCaps_t   *pIsiSensorCaps
)
{
    SP2519_Context_t *pSP2519Ctx = (SP2519_Context_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( SP2519_INFO, "%s (enter)\n", __FUNCTION__);

    if ( pSP2519Ctx == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    result = SP2519_IsiGetCapsIssInternal(pIsiSensorCaps);
    TRACE( SP2519_INFO, "%s (exit)\n", __FUNCTION__);

    return ( result );
}


/*****************************************************************************/
/**
 *          SP2519_g_IsiSensorDefaultConfig
 *
 * @brief   recommended default configuration for application use via call
 *          to IsiGetSensorIss()
 *
 *****************************************************************************/
const IsiSensorCaps_t SP2519_g_IsiSensorDefaultConfig =
{
    ISI_BUSWIDTH_8BIT_ZZ,         // BusWidth
    ISI_MODE_BT601,              // MIPI
    ISI_FIELDSEL_BOTH,          // FieldSel
    ISI_YCSEQ_YCBYCR,	//ISI_YCSEQ_CBYCRY,           // YCSeq
    ISI_CONV422_NOCOSITED,      // Conv422
    ISI_BPAT_GBGBRGRG,		//Bpat
    ISI_HPOL_REFPOS,            // HPol
    ISI_VPOL_NEG,               // VPol
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
 *          SP2519_SetupOutputFormat
 *
 * @brief   Setup of the image sensor considering the given configuration.
 *
 * @param   handle      SP2519 sensor instance handle
 * @param   pConfig     pointer to sensor configuration structure
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
RESULT SP2519_SetupOutputFormat
(
    SP2519_Context_t       *pSP2519Ctx,
    const IsiSensorConfig_t *pConfig
)
{
    RESULT result = RET_SUCCESS;
    return result;

    TRACE( SP2519_INFO, "%s%s (enter)\n", __FUNCTION__, pSP2519Ctx->isAfpsRun?"(AFPS)":"" );

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
            TRACE( SP2519_ERROR, "%s%s: bus width not supported\n", __FUNCTION__, pSP2519Ctx->isAfpsRun?"(AFPS)":"" );
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
            TRACE( SP2519_ERROR, "%s%s: mode not supported\n", __FUNCTION__, pSP2519Ctx->isAfpsRun?"(AFPS)":"" );
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
            TRACE( SP2519_ERROR, "%s%s: field selection not supported\n", __FUNCTION__, pSP2519Ctx->isAfpsRun?"(AFPS)":"" );
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
            TRACE( SP2519_ERROR, "%s%s: 422 conversion not supported\n", __FUNCTION__, pSP2519Ctx->isAfpsRun?"(AFPS)":"" );
            return ( RET_NOTSUPP );
        }
    }

    /* bayer-pattern */
    switch ( pConfig->BPat )            /* only ISI_BPAT_BGBGGRGR supported, no configuration needed */
    {
	 case ISI_BPAT_GBGBRGRG:
        {
            break;
        }
        case ISI_BPAT_RGRGGBGB:
        {
            break;
        }
	case ISI_BPAT_GRGRBGBG:
	{
		break;
	}
        default:
        {
            TRACE( SP2519_ERROR, "%s%s: bayer pattern not supported\n", __FUNCTION__, pSP2519Ctx->isAfpsRun?"(AFPS)":"" );
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
            TRACE( SP2519_ERROR, "%s%s: HPol not supported\n", __FUNCTION__, pSP2519Ctx->isAfpsRun?"(AFPS)":"" );
            return ( RET_NOTSUPP );
        }
    }

    /* vertical polarity */
    switch ( pConfig->VPol )            /* only ISI_VPOL_NEG supported, no configuration needed */
    {
        case ISI_VPOL_NEG:
        {
            break;
        }

        default:
        {
            TRACE( SP2519_ERROR, "%s%s: VPol not supported\n", __FUNCTION__, pSP2519Ctx->isAfpsRun?"(AFPS)":"" );
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
            TRACE( SP2519_ERROR, "%s%s:  edge mode not supported\n", __FUNCTION__, pSP2519Ctx->isAfpsRun?"(AFPS)":"" );
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
            TRACE( SP2519_ERROR, "%s%s:  gamma not supported\n", __FUNCTION__, pSP2519Ctx->isAfpsRun?"(AFPS)":"" );
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
            TRACE( SP2519_ERROR, "%s%s: color conversion not supported\n", __FUNCTION__, pSP2519Ctx->isAfpsRun?"(AFPS)":"" );
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
            TRACE( SP2519_ERROR, "%s%s: SMIA mode not supported\n", __FUNCTION__, pSP2519Ctx->isAfpsRun?"(AFPS)":"" );
            return ( RET_NOTSUPP );
        }
    }

    switch ( pConfig->MipiMode )        /* only ISI_MIPI_MODE_RAW_12 supported, no configuration needed */
    {
        case ISI_MIPI_MODE_RAW_10:
	{
	    break;
        }
        case ISI_MIPI_OFF:
        {
            break;
        }

        default:
        {
            TRACE( SP2519_ERROR, "%s%s: MIPI mode not supported\n", __FUNCTION__, pSP2519Ctx->isAfpsRun?"(AFPS)":"" );
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
            //TRACE( SP2519_ERROR, "%s%s: AFPS not supported\n", __FUNCTION__, pSP2519Ctx->isAfpsRun?"(AFPS)":"" );
            //return ( RET_NOTSUPP );
        }
    }

    TRACE( SP2519_INFO, "%s%s (exit)\n", __FUNCTION__, pSP2519Ctx->isAfpsRun?"(AFPS)":"");

    return ( result );
}

int SP2519_get_PCLK( SP2519_Context_t *pSP2519Ctx, int XVCLK)
{
    // calculate PCLK
    
    return 0;
}

/*****************************************************************************/
/**
 *          SP2519_SetupOutputWindow
 *
 * @brief   Setup of the image sensor considering the given configuration.
 *
 * @param   handle      SP2519 sensor instance handle
 * @param   pConfig     pointer to sensor configuration structure
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
static RESULT SP2519_SetupOutputWindow
(
    SP2519_Context_t        *pSP2519Ctx,
    const IsiSensorConfig_t *pConfig
)
{
    RESULT result     = RET_SUCCESS;

        /* resolution */
    switch ( pConfig->Resolution )
    {
        case ISI_RES_SVGAP15:
        {
            if((result = IsiRegDefaultsApply((IsiSensorHandle_t)pSP2519Ctx,SP2519_g_svga)) != RET_SUCCESS){
                TRACE( SP2519_ERROR, "%s: failed to set  ISI_RES_SVGAP15 \n", __FUNCTION__ );
            }else{

                TRACE( SP2519_INFO, "%s: success to set  ISI_RES_SVGAP15 \n", __FUNCTION__ );
            }
            break;
        }
        case ISI_RES_1600_1200P7:
        {
            if((result = IsiRegDefaultsApply((IsiSensorHandle_t)pSP2519Ctx,SP2519_g_1600x1200)) != RET_SUCCESS){
                TRACE( SP2519_ERROR, "%s: failed to set  ISI_RES_1600_1200P7 \n", __FUNCTION__ );
            }else{

                TRACE( SP2519_INFO, "%s: success to set  ISI_RES_1600_1200P7  \n", __FUNCTION__ );
            }
            break;
        }
        default:
        {
            TRACE( SP2519_ERROR, "%s: Resolution not supported\n", __FUNCTION__ );
            return ( RET_NOTSUPP );
        }
    }


    return ( result );
}




/*****************************************************************************/
/**
 *          SP2519_SetupImageControl
 *
 * @brief   Sets the image control functions (BLC, AGC, AWB, AEC, DPCC ...)
 *
 * @param   handle      SP2519 sensor instance handle
 * @param   pConfig     pointer to sensor configuration structure
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
RESULT SP2519_SetupImageControl
(
    SP2519_Context_t        *pSP2519Ctx,
    const IsiSensorConfig_t *pConfig
)
{
    RESULT result = RET_SUCCESS;

    uint32_t RegValue = 0U;

    TRACE( SP2519_INFO, "%s (enter)\n", __FUNCTION__);


    return ( result );
}


/*****************************************************************************/
/**
 *          SP2519_AecSetModeParameters
 *
 * @brief   This function fills in the correct parameters in SP2519-Instances
 *          according to AEC mode selection in IsiSensorConfig_t.
 *
 * @note    It is assumed that IsiSetupOutputWindow has been called before
 *          to fill in correct values in instance structure.
 *
 * @param   handle      SP2519 context
 * @param   pConfig     pointer to sensor configuration structure
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
static RESULT SP2519_AecSetModeParameters
(
    SP2519_Context_t       *pSP2519Ctx,
    const IsiSensorConfig_t *pConfig
)
{
    RESULT result = RET_SUCCESS;

    return ( result );
}


/*****************************************************************************/
/**
 *          SP2519_IsiSetupSensorIss
 *
 * @brief   Setup of the image sensor considering the given configuration.
 *
 * @param   handle      SP2519 sensor instance handle
 * @param   pConfig     pointer to sensor configuration structure
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
static RESULT SP2519_IsiSetupSensorIss
(
    IsiSensorHandle_t       handle,
    const IsiSensorConfig_t *pConfig
)
{
    SP2519_Context_t *pSP2519Ctx = (SP2519_Context_t *)handle;

    RESULT result = RET_SUCCESS;

    uint32_t RegValue = 0;

    TRACE( SP2519_INFO, "%s (enter)\n", __FUNCTION__);

    if ( pSP2519Ctx == NULL )
    {
        TRACE( SP2519_ERROR, "%s: Invalid sensor handle (NULL pointer detected)\n", __FUNCTION__ );
        return ( RET_WRONG_HANDLE );
    }

    if ( pConfig == NULL )
    {
        TRACE( SP2519_ERROR, "%s: Invalid configuration (NULL pointer detected)\n", __FUNCTION__ );
        return ( RET_NULL_POINTER );
    }

    if ( pSP2519Ctx->Streaming != BOOL_FALSE )
    {
        return RET_WRONG_STATE;
    }

    MEMCPY( &pSP2519Ctx->Config, pConfig, sizeof( IsiSensorConfig_t ) );

    /* 2.) write default values derived from datasheet and evaluation kit (static setup altered by dynamic setup further below) */
    result = IsiRegDefaultsApply( pSP2519Ctx, SP2519_g_aRegDescription );
    if ( result != RET_SUCCESS )
    {
        return ( result );
    }

    /* sleep a while, that sensor can take over new default values */
    osSleep( 10 );
    #if 1


    /* 3.) verify default values to make sure everything has been written correctly as expected */
    result = IsiRegDefaultsVerify( pSP2519Ctx, SP2519_g_aRegDescription );
    if ( result != RET_SUCCESS )
    {
        return ( result );
    }
    // output of pclk for measurement (only debugging)
    /*result = SP2519_IsiRegWriteIss( pSP2519Ctx, 0x3009U, 0x10U );*/
    RETURN_RESULT_IF_DIFFERENT( RET_SUCCESS, result );
    #endif

    /* 4.) setup output format (RAW10|RAW12) */
    result = SP2519_SetupOutputFormat( pSP2519Ctx, pConfig );
    if ( result != RET_SUCCESS )
    {
        TRACE( SP2519_ERROR, "%s: SetupOutputFormat failed.\n", __FUNCTION__);
        return ( result );
    }

    /* 5.) setup output window */
    result = SP2519_SetupOutputWindow( pSP2519Ctx, pConfig );
    if ( result != RET_SUCCESS )
    {
        TRACE( SP2519_ERROR, "%s: SetupOutputWindow failed.\n", __FUNCTION__);
        return ( result );
    }

    result = SP2519_SetupImageControl( pSP2519Ctx, pConfig );
    if ( result != RET_SUCCESS )
    {
        TRACE( SP2519_ERROR, "%s: SetupImageControl failed.\n", __FUNCTION__);
        return ( result );
    }

    result = SP2519_AecSetModeParameters( pSP2519Ctx, pConfig );
    if ( result != RET_SUCCESS )
    {
        TRACE( SP2519_ERROR, "%s: AecSetModeParameters failed.\n", __FUNCTION__);
        return ( result );
    }
    if (result == RET_SUCCESS)
    {
        pSP2519Ctx->Configured = BOOL_TRUE;
    }

    TRACE( SP2519_INFO, "%s: (exit)\n", __FUNCTION__);

    return ( result );
}



/*****************************************************************************/
/**
 *          SP2519_IsiChangeSensorResolutionIss
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
static RESULT SP2519_IsiChangeSensorResolutionIss
(
    IsiSensorHandle_t   handle,
    uint32_t            Resolution,
    uint8_t             *pNumberOfFramesToSkip
)
{
    SP2519_Context_t *pSP2519Ctx = (SP2519_Context_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( SP2519_INFO, "%s (enter)\n", __FUNCTION__);

    if ( pSP2519Ctx == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    if (pNumberOfFramesToSkip == NULL)
    {
        return ( RET_NULL_POINTER );
    }

    if ( (pSP2519Ctx->Configured != BOOL_TRUE) || (pSP2519Ctx->Streaming != BOOL_FALSE) )
    {
        return RET_WRONG_STATE;
    }

    IsiSensorCaps_t Caps;    
    Caps.Index = 0;
    Caps.Resolution = 0;
    while (SP2519_IsiGetCapsIss( handle, &Caps) == RET_SUCCESS) {
        if (Resolution == Caps.Resolution) {            
            break;
        }
        Caps.Index++;
    }

    if ( (Resolution & Caps.Resolution) == 0 )
    {
        return RET_OUTOFRANGE;
    }

    if ( Resolution == pSP2519Ctx->Config.Resolution )
    {
        // well, no need to worry
        *pNumberOfFramesToSkip = 0;
    }
    else
    {
        // change resolution
        char *szResName = NULL;
        result = IsiGetResolutionName( Resolution, &szResName );
        TRACE( SP2519_DEBUG, "%s: NewRes=0x%08x (%s)\n", __FUNCTION__, Resolution, szResName);

        // update resolution in copy of config in context
        pSP2519Ctx->Config.Resolution = Resolution;

        // tell sensor about that
        result = SP2519_SetupOutputWindow( pSP2519Ctx, &pSP2519Ctx->Config );
        if ( result != RET_SUCCESS )
        {
            TRACE( SP2519_ERROR, "%s: SetupOutputWindow failed.\n", __FUNCTION__);
            return ( result );
        }

        // remember old exposure values
        float OldGain = pSP2519Ctx->AecCurGain;
        float OldIntegrationTime = pSP2519Ctx->AecCurIntegrationTime;

        // update limits & stuff (reset current & old settings)
        result = SP2519_AecSetModeParameters( pSP2519Ctx, &pSP2519Ctx->Config );
        if ( result != RET_SUCCESS )
        {
            TRACE( SP2519_ERROR, "%s: AecSetModeParameters failed.\n", __FUNCTION__);
            return ( result );
        }

        // restore old exposure values (at least within new exposure values' limits)
        uint8_t NumberOfFramesToSkip = 0;
        float   DummySetGain;
        float   DummySetIntegrationTime;
        result = SP2519_IsiExposureControlIss( handle, OldGain, OldIntegrationTime, &NumberOfFramesToSkip, &DummySetGain, &DummySetIntegrationTime );
        if ( result != RET_SUCCESS )
        {
            TRACE( SP2519_ERROR, "%s: SP2519_IsiExposureControlIss failed.\n", __FUNCTION__);
            return ( result );
        }

        // return number of frames that aren't exposed correctly
        *pNumberOfFramesToSkip = NumberOfFramesToSkip + 1;
    }

    TRACE( SP2519_INFO, "%s (exit)\n", __FUNCTION__);

    return ( result );
}



/*****************************************************************************/
/**
 *          SP2519_IsiSensorSetStreamingIss
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
static RESULT SP2519_IsiSensorSetStreamingIss
(
    IsiSensorHandle_t   handle,
    bool_t              on
)
{
    uint32_t RegValue = 0;

    SP2519_Context_t *pSP2519Ctx = (SP2519_Context_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( SP2519_INFO, "%s (enter)\n", __FUNCTION__);

    if ( pSP2519Ctx == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    if ( (pSP2519Ctx->Configured != BOOL_TRUE) || (pSP2519Ctx->Streaming == on) )
    {
        return RET_WRONG_STATE;
    }

    if (on == BOOL_TRUE)
    {
        /* enable streaming */
        // hkw add;
       // result = SP2519_IsiRegWriteIss( handle, 0x0100, 0x1); 
     		 result = RET_SUCCESS;

    }
    else
    {
        /* disable streaming */
       // result = SP2519_IsiRegWriteIss( handle, 0x0100, 0x0);
       result = RET_SUCCESS;


    }

    if (result == RET_SUCCESS)
    {
        pSP2519Ctx->Streaming = on;
    }

    TRACE( SP2519_INFO, "%s (exit)\n", __FUNCTION__);

    return ( result );
}



/*****************************************************************************/
/**
 *          SP2519_IsiSensorSetPowerIss
 *
 * @brief   Performs the power-up/power-down sequence of the camera, if possible.
 *
 * @param   handle      SP2519 sensor instance handle
 * @param   on          new power state (BOOL_TRUE=on, BOOL_FALSE=off)
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
static RESULT SP2519_IsiSensorSetPowerIss
(
    IsiSensorHandle_t   handle,
    bool_t              on
)
{
    SP2519_Context_t *pSP2519Ctx = (SP2519_Context_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( SP2519_INFO, "%s (enter)\n", __FUNCTION__);

    if ( pSP2519Ctx == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    pSP2519Ctx->Configured = BOOL_FALSE;
    pSP2519Ctx->Streaming  = BOOL_FALSE;

    TRACE( SP2519_DEBUG, "%s power off \n", __FUNCTION__);
    result = HalSetPower( pSP2519Ctx->IsiCtx.HalHandle, pSP2519Ctx->IsiCtx.HalDevID, false );
    RETURN_RESULT_IF_DIFFERENT( RET_SUCCESS, result );

    TRACE( SP2519_DEBUG, "%s reset on\n", __FUNCTION__);
    result = HalSetReset( pSP2519Ctx->IsiCtx.HalHandle, pSP2519Ctx->IsiCtx.HalDevID, true );
    RETURN_RESULT_IF_DIFFERENT( RET_SUCCESS, result );

    if (on == BOOL_TRUE)
    {
        TRACE( SP2519_DEBUG, "%s power on \n", __FUNCTION__);
        result = HalSetPower( pSP2519Ctx->IsiCtx.HalHandle, pSP2519Ctx->IsiCtx.HalDevID, true );
        RETURN_RESULT_IF_DIFFERENT( RET_SUCCESS, result );

        osSleep( 10 );

        TRACE( SP2519_DEBUG, "%s reset off \n", __FUNCTION__);
        result = HalSetReset( pSP2519Ctx->IsiCtx.HalHandle, pSP2519Ctx->IsiCtx.HalDevID, false );
        RETURN_RESULT_IF_DIFFERENT( RET_SUCCESS, result );

        osSleep( 10 );

        TRACE( SP2519_DEBUG, "%s reset on \n", __FUNCTION__);
        result = HalSetReset( pSP2519Ctx->IsiCtx.HalHandle, pSP2519Ctx->IsiCtx.HalDevID, true );
        RETURN_RESULT_IF_DIFFERENT( RET_SUCCESS, result );

        osSleep( 10 );

        TRACE( SP2519_DEBUG, "%s reset off \n", __FUNCTION__);
        result = HalSetReset( pSP2519Ctx->IsiCtx.HalHandle, pSP2519Ctx->IsiCtx.HalDevID, false );

        osSleep( 50 );
    }

    TRACE( SP2519_INFO, "%s (exit)\n", __FUNCTION__);

    return ( result );
}



/*****************************************************************************/
/**
 *          SP2519_IsiCheckSensorConnectionIss
 *
 * @brief   Checks the I2C-Connection to sensor by reading sensor revision id.
 *
 * @param   handle      SP2519 sensor instance handle
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
static RESULT SP2519_IsiCheckSensorConnectionIss
(
    IsiSensorHandle_t   handle
)
{
    uint32_t RevId;
    uint32_t value;

    RESULT result = RET_SUCCESS;

    TRACE( SP2519_INFO, "%s (enter)\n", __FUNCTION__);

    if ( handle == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }
		
		// hkw fix;
    RevId = SP2519_CHIP_ID_HIGH_BYTE_DEFAULT;
    //RevId = (RevId << 16U) | (SP2519_CHIP_ID_MIDDLE_BYTE_DEFAULT<<8U);
    //RevId = RevId | SP2519_CHIP_ID_LOW_BYTE_DEFAULT;

    result = SP2519_IsiGetSensorRevisionIss( handle, &value );
    if ( (result != RET_SUCCESS) || (RevId != value) )
    {
        TRACE( SP2519_ERROR, "%s RevId = 0x%08x, value = 0x%08x \n", __FUNCTION__, RevId, value );
        return ( RET_FAILURE );
    }

    TRACE( SP2519_DEBUG, "%s RevId = 0x%08x, value = 0x%08x \n", __FUNCTION__, RevId, value );

    TRACE( SP2519_INFO, "%s (exit)\n", __FUNCTION__);

    return ( result );
}



/*****************************************************************************/
/**
 *          SP2519_IsiGetSensorRevisionIss
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
static RESULT SP2519_IsiGetSensorRevisionIss
(
    IsiSensorHandle_t   handle,
    uint32_t            *p_value
)
{
    RESULT result = RET_SUCCESS;

    uint32_t data;

    TRACE( SP2519_INFO, "%s (enter)\n", __FUNCTION__);

    if ( handle == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    if ( p_value == NULL )
    {
        return ( RET_NULL_POINTER );
    }

    *p_value = 0U;
    result = SP2519_IsiRegReadIss ( handle, SP2519_CHIP_ID_HIGH_BYTE, &data );
    *p_value = data;
    //hkw fix;
    //*p_value = ( (data & 0xFF) << 16U );
    //result = SP2519_IsiRegReadIss ( handle, SP2519_CHIP_ID_MIDDLE_BYTE, &data );
    //*p_value |= ( (data & 0xFF) << 8U );
    //result = SP2519_IsiRegReadIss ( handle, SP2519_CHIP_ID_LOW_BYTE, &data );
    //*p_value |= ( (data & 0xFF));

    TRACE( SP2519_INFO, "%s (exit)\n", __FUNCTION__);

    return ( result );
}



/*****************************************************************************/
/**
 *          SP2519_IsiRegReadIss
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
static RESULT SP2519_IsiRegReadIss
(
    IsiSensorHandle_t   handle,
    const uint32_t      address,
    uint32_t            *p_value
)
{
    RESULT result = RET_SUCCESS;

  //TRACE( SP2519_INFO, "%s (enter)\n", __FUNCTION__);
   ALOGD("%s: (enter)\n", __FUNCTION__);

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
        uint8_t NrOfBytes = IsiGetNrDatBytesIss( address, SP2519_g_aRegDescription );
        if ( !NrOfBytes )
        {
            NrOfBytes = 1;
        }
     TRACE( SP2519_REG_DEBUG, "%s (IsiGetNrDatBytesIss %d 0x%08x)\n", __FUNCTION__, NrOfBytes, address);

        *p_value = 0;
        result = IsiI2cReadSensorRegister( handle, address, (uint8_t *)p_value, NrOfBytes, BOOL_TRUE );
    }

  //  TRACE( SP2519_INFO, "%s (exit: 0x%08x 0x%08x)\n", __FUNCTION__, address, *p_value);
  	 ALOGD("%s (exit: 0x%04x 0x%02x)\n", __FUNCTION__, address, *p_value);

    return ( result );
}



/*****************************************************************************/
/**
 *          SP2519_IsiRegWriteIss
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
static RESULT SP2519_IsiRegWriteIss
(
    IsiSensorHandle_t   handle,
    const uint32_t      address,
    const uint32_t      value
)
{
    RESULT result = RET_SUCCESS;

    uint8_t NrOfBytes;

  //  TRACE( SP2519_INFO, "%s (enter)\n", __FUNCTION__);
   ALOGD("%s: (enter)\n", __FUNCTION__);

    if ( handle == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    NrOfBytes = IsiGetNrDatBytesIss( address, SP2519_g_aRegDescription );
    if ( !NrOfBytes )
    {
        NrOfBytes = 1;
    }
//    TRACE( SP2519_REG_DEBUG, "%s (IsiGetNrDatBytesIss %d 0x%08x 0x%08x)\n", __FUNCTION__, NrOfBytes, address, value);

    result = IsiI2cWriteSensorRegister( handle, address, (uint8_t *)(&value), NrOfBytes, BOOL_TRUE );

//    TRACE( SP2519_INFO, "%s (exit: 0x%08x 0x%08x)\n", __FUNCTION__, address, value);

	ALOGD( "%s (exit: 0x%04x 0x%02x)\n", __FUNCTION__, address, value);

    return ( result );
}



/*****************************************************************************/
/**
 *          SP2519_IsiGetGainLimitsIss
 *
 * @brief   Returns the exposure minimal and maximal values of an
 *          SP2519 instance
 *
 * @param   handle       SP2519 sensor instance handle
 * @param   pMinExposure Pointer to a variable receiving minimal exposure value
 * @param   pMaxExposure Pointer to a variable receiving maximal exposure value
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
static RESULT SP2519_IsiGetGainLimitsIss
(
    IsiSensorHandle_t   handle,
    float               *pMinGain,
    float               *pMaxGain
)
{
    SP2519_Context_t *pSP2519Ctx = (SP2519_Context_t *)handle;

    RESULT result = RET_SUCCESS;

    uint32_t RegValue = 0;

    TRACE( SP2519_INFO, "%s: (enter)\n", __FUNCTION__);

    if ( pSP2519Ctx == NULL )
    {
        TRACE( SP2519_ERROR, "%s: Invalid sensor handle (NULL pointer detected)\n", __FUNCTION__ );
        return ( RET_WRONG_HANDLE );
    }

    if ( (pMinGain == NULL) || (pMaxGain == NULL) )
    {
        TRACE( SP2519_ERROR, "%s: NULL pointer received!!\n" );
        return ( RET_NULL_POINTER );
    }

    *pMinGain = pSP2519Ctx->AecMinGain;
    *pMaxGain = pSP2519Ctx->AecMaxGain;

    TRACE( SP2519_INFO, "%s: (enter)\n", __FUNCTION__);

    return ( result );
}



/*****************************************************************************/
/**
 *          SP2519_IsiGetIntegrationTimeLimitsIss
 *
 * @brief   Returns the minimal and maximal integration time values of an
 *          SP2519 instance
 *
 * @param   handle       SP2519 sensor instance handle
 * @param   pMinExposure Pointer to a variable receiving minimal exposure value
 * @param   pMaxExposure Pointer to a variable receiving maximal exposure value
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
static RESULT SP2519_IsiGetIntegrationTimeLimitsIss
(
    IsiSensorHandle_t   handle,
    float               *pMinIntegrationTime,
    float               *pMaxIntegrationTime
)
{
    SP2519_Context_t *pSP2519Ctx = (SP2519_Context_t *)handle;

    RESULT result = RET_SUCCESS;

    uint32_t RegValue = 0;

    TRACE( SP2519_INFO, "%s: (enter)\n", __FUNCTION__);

    if ( pSP2519Ctx == NULL )
    {
        TRACE( SP2519_ERROR, "%s: Invalid sensor handle (NULL pointer detected)\n", __FUNCTION__ );
        return ( RET_WRONG_HANDLE );
    }

    if ( (pMinIntegrationTime == NULL) || (pMaxIntegrationTime == NULL) )
    {
        TRACE( SP2519_ERROR, "%s: NULL pointer received!!\n" );
        return ( RET_NULL_POINTER );
    }

    *pMinIntegrationTime = pSP2519Ctx->AecMinIntegrationTime;
    *pMaxIntegrationTime = pSP2519Ctx->AecMaxIntegrationTime;

    TRACE( SP2519_INFO, "%s: (enter)\n", __FUNCTION__);

    return ( result );
}


/*****************************************************************************/
/**
 *          SP2519_IsiGetGainIss
 *
 * @brief   Reads gain values from the image sensor module.
 *
 * @param   handle                  SP2519 sensor instance handle
 * @param   pSetGain                set gain
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
RESULT SP2519_IsiGetGainIss
(
    IsiSensorHandle_t   handle,
    float               *pSetGain
)
{
    SP2519_Context_t *pSP2519Ctx = (SP2519_Context_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( SP2519_INFO, "%s: (enter)\n", __FUNCTION__);

    if ( pSP2519Ctx == NULL )
    {
        TRACE( SP2519_ERROR, "%s: Invalid sensor handle (NULL pointer detected)\n", __FUNCTION__ );
        return ( RET_WRONG_HANDLE );
    }

    if ( pSetGain == NULL)
    {
        return ( RET_NULL_POINTER );
    }

    *pSetGain = pSP2519Ctx->AecCurGain;

    TRACE( SP2519_INFO, "%s: (exit)\n", __FUNCTION__);

    return ( result );
}



/*****************************************************************************/
/**
 *          SP2519_IsiGetGainIncrementIss
 *
 * @brief   Get smallest possible gain increment.
 *
 * @param   handle                  SP2519 sensor instance handle
 * @param   pIncr                   increment
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
RESULT SP2519_IsiGetGainIncrementIss
(
    IsiSensorHandle_t   handle,
    float               *pIncr
)
{
    SP2519_Context_t *pSP2519Ctx = (SP2519_Context_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( SP2519_INFO, "%s: (enter)\n", __FUNCTION__);

    if ( pSP2519Ctx == NULL )
    {
        TRACE( SP2519_ERROR, "%s: Invalid sensor handle (NULL pointer detected)\n", __FUNCTION__ );
        return ( RET_WRONG_HANDLE );
    }

    if ( pIncr == NULL)
    {
        return ( RET_NULL_POINTER );
    }

    //_smallest_ increment the sensor/driver can handle (e.g. used for sliders in the application)
    *pIncr = pSP2519Ctx->AecGainIncrement;

    TRACE( SP2519_INFO, "%s: (exit)\n", __FUNCTION__);

    return ( result );
}



/*****************************************************************************/
/**
 *          SP2519_IsiSetGainIss
 *
 * @brief   Writes gain values to the image sensor module.
 *          Updates current gain and exposure in sensor struct/state.
 *
 * @param   handle                  SP2519 sensor instance handle
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
RESULT SP2519_IsiSetGainIss
(
    IsiSensorHandle_t   handle,
    float               NewGain,
    float               *pSetGain
)
{
    SP2519_Context_t *pSP2519Ctx = (SP2519_Context_t *)handle;

    RESULT result = RET_SUCCESS;

    uint16_t usGain = 0;

    TRACE( SP2519_INFO, "%s: (enter)\n", __FUNCTION__);

    if ( pSP2519Ctx == NULL )
    {
        TRACE( SP2519_ERROR, "%s: Invalid sensor handle (NULL pointer detected)\n", __FUNCTION__ );
        return ( RET_WRONG_HANDLE );
    }

    if ( pSetGain == NULL)
    {
        TRACE( SP2519_ERROR, "%s: Invalid parameter (NULL pointer detected)\n", __FUNCTION__ );
        return ( RET_NULL_POINTER );
    }

  
    if( NewGain < pSP2519Ctx->AecMinGain ) NewGain = pSP2519Ctx->AecMinGain;
    if( NewGain > pSP2519Ctx->AecMaxGain ) NewGain = pSP2519Ctx->AecMaxGain;

    usGain = (uint16_t)NewGain;

    // write new gain into sensor registers, do not write if nothing has changed
    if( (usGain != pSP2519Ctx->OldGain) )
    {

        pSP2519Ctx->OldGain = usGain;
    }

    //calculate gain actually set
    pSP2519Ctx->AecCurGain = NewGain;

    //return current state
    *pSetGain = pSP2519Ctx->AecCurGain;
    TRACE( SP2519_DEBUG, "%s: g=%f\n", __FUNCTION__, *pSetGain );

    TRACE( SP2519_INFO, "%s: (exit)\n", __FUNCTION__);

    return ( result );
}



/*****************************************************************************/
/**
 *          SP2519_IsiGetIntegrationTimeIss
 *
 * @brief   Reads integration time values from the image sensor module.
 *
 * @param   handle                  SP2519 sensor instance handle
 * @param   pSetIntegrationTime     set integration time
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
RESULT SP2519_IsiGetIntegrationTimeIss
(
    IsiSensorHandle_t   handle,
    float               *pSetIntegrationTime
)
{
    SP2519_Context_t *pSP2519Ctx = (SP2519_Context_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( SP2519_INFO, "%s: (enter)\n", __FUNCTION__);

    if ( pSP2519Ctx == NULL )
    {
        TRACE( SP2519_ERROR, "%s: Invalid sensor handle (NULL pointer detected)\n", __FUNCTION__ );
        return ( RET_WRONG_HANDLE );
    }

    if ( pSetIntegrationTime == NULL )
    {
        return ( RET_NULL_POINTER );
    }

    *pSetIntegrationTime = pSP2519Ctx->AecCurIntegrationTime;

    TRACE( SP2519_INFO, "%s: (exit)\n", __FUNCTION__);

    return ( result );
}



/*****************************************************************************/
/**
 *          SP2519_IsiGetIntegrationTimeIncrementIss
 *
 * @brief   Get smallest possible integration time increment.
 *
 * @param   handle                  SP2519 sensor instance handle
 * @param   pIncr                   increment
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
RESULT SP2519_IsiGetIntegrationTimeIncrementIss
(
    IsiSensorHandle_t   handle,
    float               *pIncr
)
{
    SP2519_Context_t *pSP2519Ctx = (SP2519_Context_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( SP2519_INFO, "%s: (enter)\n", __FUNCTION__);

    if ( pSP2519Ctx == NULL )
    {
        TRACE( SP2519_ERROR, "%s: Invalid sensor handle (NULL pointer detected)\n", __FUNCTION__ );
        return ( RET_WRONG_HANDLE );
    }

    if ( pIncr == NULL )
    {
        return ( RET_NULL_POINTER );
    }

    //_smallest_ increment the sensor/driver can handle (e.g. used for sliders in the application)
    *pIncr = pSP2519Ctx->AecIntegrationTimeIncrement;

    TRACE( SP2519_INFO, "%s: (exit)\n", __FUNCTION__);

    return ( result );
}



/*****************************************************************************/
/**
 *          SP2519_IsiSetIntegrationTimeIss
 *
 * @brief   Writes gain and integration time values to the image sensor module.
 *          Updates current integration time and exposure in sensor
 *          struct/state.
 *
 * @param   handle                  SP2519 sensor instance handle
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
RESULT SP2519_IsiSetIntegrationTimeIss
(
    IsiSensorHandle_t   handle,
    float               NewIntegrationTime,
    float               *pSetIntegrationTime,
    uint8_t             *pNumberOfFramesToSkip
)
{
    SP2519_Context_t *pSP2519Ctx = (SP2519_Context_t *)handle;

    RESULT result = RET_SUCCESS;


    TRACE( SP2519_DEBUG, "%s: Ti=%f\n", __FUNCTION__, *pSetIntegrationTime );
    TRACE( SP2519_INFO, "%s: (exit)\n", __FUNCTION__);

    return ( result );
}




/*****************************************************************************/
/**
 *          SP2519_IsiExposureControlIss
 *
 * @brief   Camera hardware dependent part of the exposure control loop.
 *          Calculates appropriate register settings from the new exposure
 *          values and writes them to the image sensor module.
 *
 * @param   handle                  SP2519 sensor instance handle
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
RESULT SP2519_IsiExposureControlIss
(
    IsiSensorHandle_t   handle,
    float               NewGain,
    float               NewIntegrationTime,
    uint8_t             *pNumberOfFramesToSkip,
    float               *pSetGain,
    float               *pSetIntegrationTime
)
{
    SP2519_Context_t *pSP2519Ctx = (SP2519_Context_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( SP2519_INFO, "%s: (enter)\n", __FUNCTION__);

    if ( pSP2519Ctx == NULL )
    {
        TRACE( SP2519_ERROR, "%s: Invalid sensor handle (NULL pointer detected)\n", __FUNCTION__ );
        return ( RET_WRONG_HANDLE );
    }

    if ( (pNumberOfFramesToSkip == NULL)
            || (pSetGain == NULL)
            || (pSetIntegrationTime == NULL) )
    {
        TRACE( SP2519_ERROR, "%s: Invalid parameter (NULL pointer detected)\n", __FUNCTION__ );
        return ( RET_NULL_POINTER );
    }

    TRACE( SP2519_DEBUG, "%s: g=%f, Ti=%f\n", __FUNCTION__, NewGain, NewIntegrationTime );


    result = SP2519_IsiSetIntegrationTimeIss( handle, NewIntegrationTime, pSetIntegrationTime, pNumberOfFramesToSkip );
    result = SP2519_IsiSetGainIss( handle, NewGain, pSetGain );

    TRACE( SP2519_DEBUG, "%s: set: g=%f, Ti=%f, skip=%d\n", __FUNCTION__, *pSetGain, *pSetIntegrationTime, *pNumberOfFramesToSkip );
    TRACE( SP2519_INFO, "%s: (exit)\n", __FUNCTION__);

    return ( result );
}



/*****************************************************************************/
/**
 *          SP2519_IsiGetCurrentExposureIss
 *
 * @brief   Returns the currently adjusted AE values
 *
 * @param   handle                  SP2519 sensor instance handle
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
RESULT SP2519_IsiGetCurrentExposureIss
(
    IsiSensorHandle_t   handle,
    float               *pSetGain,
    float               *pSetIntegrationTime
)
{
    SP2519_Context_t *pSP2519Ctx = (SP2519_Context_t *)handle;

    RESULT result = RET_SUCCESS;

    uint32_t RegValue = 0;

    TRACE( SP2519_INFO, "%s: (enter)\n", __FUNCTION__);

    if ( pSP2519Ctx == NULL )
    {
        TRACE( SP2519_ERROR, "%s: Invalid sensor handle (NULL pointer detected)\n", __FUNCTION__ );
        return ( RET_WRONG_HANDLE );
    }

    if ( (pSetGain == NULL) || (pSetIntegrationTime == NULL) )
    {
        return ( RET_NULL_POINTER );
    }

    *pSetGain            = pSP2519Ctx->AecCurGain;
    *pSetIntegrationTime = pSP2519Ctx->AecCurIntegrationTime;

    TRACE( SP2519_INFO, "%s: (exit)\n", __FUNCTION__);

    return ( result );
}



/*****************************************************************************/
/**
 *          SP2519_IsiGetResolutionIss
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
RESULT SP2519_IsiGetResolutionIss
(
    IsiSensorHandle_t   handle,
    uint32_t            *pSetResolution
)
{
    SP2519_Context_t *pSP2519Ctx = (SP2519_Context_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( SP2519_INFO, "%s: (enter)\n", __FUNCTION__);

    if ( pSP2519Ctx == NULL )
    {
        TRACE( SP2519_ERROR, "%s: Invalid sensor handle (NULL pointer detected)\n", __FUNCTION__ );
        return ( RET_WRONG_HANDLE );
    }

    if ( pSetResolution == NULL )
    {
        return ( RET_NULL_POINTER );
    }

    *pSetResolution = pSP2519Ctx->Config.Resolution;

    TRACE( SP2519_INFO, "%s: (exit)\n", __FUNCTION__);

    return ( result );
}



/*****************************************************************************/
/**
 *          SP2519_IsiGetAfpsInfoHelperIss
 *
 * @brief   Calc AFPS sub resolution settings for the given resolution
 *
 * @param   pSP2519Ctx             SP2519 sensor instance (dummy!) context
 * @param   Resolution              Any supported resolution to query AFPS params for
 * @param   pAfpsInfo               Reference of AFPS info structure to write the results to
 * @param   AfpsStageIdx            Index of current AFPS stage to use
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 *
 *****************************************************************************/
static RESULT SP2519_IsiGetAfpsInfoHelperIss(
    SP2519_Context_t   *pSP2519Ctx,
    uint32_t            Resolution,
    IsiAfpsInfo_t*      pAfpsInfo,
    uint32_t            AfpsStageIdx
)
{
    RESULT result = RET_SUCCESS;

    TRACE( SP2519_INFO, "%s: (enter)\n", __FUNCTION__);

    DCT_ASSERT(pSP2519Ctx != NULL);
    DCT_ASSERT(pAfpsInfo != NULL);
    DCT_ASSERT(AfpsStageIdx <= ISI_NUM_AFPS_STAGES);

    // update resolution in copy of config in context
    pSP2519Ctx->Config.Resolution = Resolution;

    // tell sensor about that
    result = SP2519_SetupOutputWindow( pSP2519Ctx, &pSP2519Ctx->Config );
    if ( result != RET_SUCCESS )
    {
        TRACE( SP2519_ERROR, "%s: SetupOutputWindow failed for resolution ID %08x.\n", __FUNCTION__, Resolution);
        return ( result );
    }

    // update limits & stuff (reset current & old settings)
    result = SP2519_AecSetModeParameters( pSP2519Ctx, &pSP2519Ctx->Config );
    if ( result != RET_SUCCESS )
    {
        TRACE( SP2519_ERROR, "%s: AecSetModeParameters failed for resolution ID %08x.\n", __FUNCTION__, Resolution);
        return ( result );
    }

    // take over params
    pAfpsInfo->Stage[AfpsStageIdx].Resolution = Resolution;
    pAfpsInfo->Stage[AfpsStageIdx].MaxIntTime = pSP2519Ctx->AecMaxIntegrationTime;
    pAfpsInfo->AecMinGain           = pSP2519Ctx->AecMinGain;
    pAfpsInfo->AecMaxGain           = pSP2519Ctx->AecMaxGain;
    pAfpsInfo->AecMinIntTime        = pSP2519Ctx->AecMinIntegrationTime;
    pAfpsInfo->AecMaxIntTime        = pSP2519Ctx->AecMaxIntegrationTime;
    pAfpsInfo->AecSlowestResolution = Resolution;

    TRACE( SP2519_INFO, "%s: (exit)\n", __FUNCTION__);

    return ( result );
}

/*****************************************************************************/
/**
 *          SP2519_IsiGetAfpsInfoIss
 *
 * @brief   Returns the possible AFPS sub resolution settings for the given resolution series
 *
 * @param   handle                  SP2519 sensor instance handle
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
RESULT SP2519_IsiGetAfpsInfoIss(
    IsiSensorHandle_t   handle,
    uint32_t            Resolution,
    IsiAfpsInfo_t*      pAfpsInfo
)
{
    SP2519_Context_t *pSP2519Ctx = (SP2519_Context_t *)handle;

    RESULT result = RET_SUCCESS;

    uint32_t RegValue = 0;

    TRACE( SP2519_INFO, "%s: (enter)\n", __FUNCTION__);

    if ( pSP2519Ctx == NULL )
    {
        TRACE( SP2519_ERROR, "%s: Invalid sensor handle (NULL pointer detected)\n", __FUNCTION__ );
        return ( RET_WRONG_HANDLE );
    }

    if ( pAfpsInfo == NULL )
    {
        return ( RET_NULL_POINTER );
    }

    // use currently configured resolution?
    if (Resolution == 0)
    {
        Resolution = pSP2519Ctx->Config.Resolution;
    }

    TRACE( SP2519_INFO, "%s: (exit)\n", __FUNCTION__);

    return ( result );
}



/*****************************************************************************/
/**
 *          SP2519_IsiGetCalibKFactor
 *
 * @brief   Returns the SP2519 specific K-Factor
 *
 * @param   handle       SP2519 sensor instance handle
 * @param   pIsiKFactor  Pointer to Pointer receiving the memory address
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
static RESULT SP2519_IsiGetCalibKFactor
(
    IsiSensorHandle_t   handle,
    Isi1x1FloatMatrix_t **pIsiKFactor
)
{
    SP2519_Context_t *pSP2519Ctx = (SP2519_Context_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( SP2519_INFO, "%s: (enter)\n", __FUNCTION__);

    if ( pSP2519Ctx == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    if ( pIsiKFactor == NULL )
    {
        return ( RET_NULL_POINTER );
    }

    *pIsiKFactor = NULL;

    TRACE( SP2519_INFO, "%s: (exit)\n", __FUNCTION__);

    return ( result );
}


/*****************************************************************************/
/**
 *          SP2519_IsiGetCalibPcaMatrix
 *
 * @brief   Returns the SP2519 specific PCA-Matrix
 *
 * @param   handle          SP2519 sensor instance handle
 * @param   pIsiPcaMatrix   Pointer to Pointer receiving the memory address
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
static RESULT SP2519_IsiGetCalibPcaMatrix
(
    IsiSensorHandle_t   handle,
    Isi3x2FloatMatrix_t **pIsiPcaMatrix
)
{
    SP2519_Context_t *pSP2519Ctx = (SP2519_Context_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( SP2519_INFO, "%s: (enter)\n", __FUNCTION__);

    if ( pSP2519Ctx == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    if ( pIsiPcaMatrix == NULL )
    {
        return ( RET_NULL_POINTER );
    }

    *pIsiPcaMatrix = NULL;

    TRACE( SP2519_INFO, "%s: (exit)\n", __FUNCTION__);

    return ( result );
}



/*****************************************************************************/
/**
 *          SP2519_IsiGetCalibSvdMeanValue
 *
 * @brief   Returns the sensor specific SvdMean-Vector
 *
 * @param   handle              SP2519 sensor instance handle
 * @param   pIsiSvdMeanValue    Pointer to Pointer receiving the memory address
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
static RESULT SP2519_IsiGetCalibSvdMeanValue
(
    IsiSensorHandle_t   handle,
    Isi3x1FloatMatrix_t **pIsiSvdMeanValue
)
{
    IsiSensorContext_t *pSensorCtx = (IsiSensorContext_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( SP2519_INFO, "%s: (enter)\n", __FUNCTION__);

    if ( pSensorCtx == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    if ( pIsiSvdMeanValue == NULL )
    {
        return ( RET_NULL_POINTER );
    }

    *pIsiSvdMeanValue = NULL;

    TRACE( SP2519_INFO, "%s: (exit)\n", __FUNCTION__);

    return ( result );
}



/*****************************************************************************/
/**
 *          SP2519_IsiGetCalibSvdMeanValue
 *
 * @brief   Returns a pointer to the sensor specific centerline, a straight
 *          line in Hesse normal form in Rg/Bg colorspace
 *
 * @param   handle              SP2519 sensor instance handle
 * @param   pIsiSvdMeanValue    Pointer to Pointer receiving the memory address
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
static RESULT SP2519_IsiGetCalibCenterLine
(
    IsiSensorHandle_t   handle,
    IsiLine_t           **ptIsiCenterLine
)
{
    IsiSensorContext_t *pSensorCtx = (IsiSensorContext_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( SP2519_INFO, "%s: (enter)\n", __FUNCTION__);

    if ( pSensorCtx == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    if ( ptIsiCenterLine == NULL )
    {
        return ( RET_NULL_POINTER );
    }

    *ptIsiCenterLine = NULL;

    TRACE( SP2519_INFO, "%s: (exit)\n", __FUNCTION__);

    return ( result );
}



/*****************************************************************************/
/**
 *          SP2519_IsiGetCalibClipParam
 *
 * @brief   Returns a pointer to the sensor specific arrays for Rg/Bg color
 *          space clipping
 *
 * @param   handle              SP2519 sensor instance handle
 * @param   pIsiSvdMeanValue    Pointer to Pointer receiving the memory address
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
static RESULT SP2519_IsiGetCalibClipParam
(
    IsiSensorHandle_t   handle,
    IsiAwbClipParm_t    **pIsiClipParam
)
{
    IsiSensorContext_t *pSensorCtx = (IsiSensorContext_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( SP2519_INFO, "%s: (enter)\n", __FUNCTION__);

    if ( pSensorCtx == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    if ( pIsiClipParam == NULL )
    {
        return ( RET_NULL_POINTER );
    }

    *pIsiClipParam = NULL;

    TRACE( SP2519_INFO, "%s: (exit)\n", __FUNCTION__);

    return ( result );
}



/*****************************************************************************/
/**
 *          SP2519_IsiGetCalibGlobalFadeParam
 *
 * @brief   Returns a pointer to the sensor specific arrays for AWB out of
 *          range handling
 *
 * @param   handle              SP2519 sensor instance handle
 * @param   pIsiSvdMeanValue    Pointer to Pointer receiving the memory address
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
static RESULT SP2519_IsiGetCalibGlobalFadeParam
(
    IsiSensorHandle_t       handle,
    IsiAwbGlobalFadeParm_t  **ptIsiGlobalFadeParam
)
{
    IsiSensorContext_t *pSensorCtx = (IsiSensorContext_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( SP2519_INFO, "%s: (enter)\n", __FUNCTION__);

    if ( pSensorCtx == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    if ( ptIsiGlobalFadeParam == NULL )
    {
        return ( RET_NULL_POINTER );
    }

    *ptIsiGlobalFadeParam = NULL;

    TRACE( SP2519_INFO, "%s: (exit)\n", __FUNCTION__);

    return ( result );
}



/*****************************************************************************/
/**
 *          SP2519_IsiGetCalibFadeParam
 *
 * @brief   Returns a pointer to the sensor specific arrays for near white
 *          pixel parameter calculations
 *
 * @param   handle              SP2519 sensor instance handle
 * @param   pIsiSvdMeanValue    Pointer to Pointer receiving the memory address
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
static RESULT SP2519_IsiGetCalibFadeParam
(
    IsiSensorHandle_t   handle,
    IsiAwbFade2Parm_t   **ptIsiFadeParam
)
{
    IsiSensorContext_t *pSensorCtx = (IsiSensorContext_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( SP2519_INFO, "%s: (enter)\n", __FUNCTION__);

    if ( pSensorCtx == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    if ( ptIsiFadeParam == NULL )
    {
        return ( RET_NULL_POINTER );
    }

    *ptIsiFadeParam = NULL;

    TRACE( SP2519_INFO, "%s: (exit)\n", __FUNCTION__);

    return ( result );
}

/*****************************************************************************/
/**
 *          SP2519_IsiGetIlluProfile
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
static RESULT SP2519_IsiGetIlluProfile
(
    IsiSensorHandle_t   handle,
    const uint32_t      CieProfile,
    IsiIlluProfile_t    **ptIsiIlluProfile
)
{
    SP2519_Context_t *pSP2519Ctx = (SP2519_Context_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( SP2519_INFO, "%s: (enter)\n", __FUNCTION__);

    if ( pSP2519Ctx == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    if ( ptIsiIlluProfile == NULL )
    {
        return ( RET_NULL_POINTER );
    }
    TRACE( SP2519_INFO, "%s: (exit)\n", __FUNCTION__);

    return ( result );
}



/*****************************************************************************/
/**
 *          SP2519_IsiGetLscMatrixTable
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
static RESULT SP2519_IsiGetLscMatrixTable
(
    IsiSensorHandle_t   handle,
    const uint32_t      CieProfile,
    IsiLscMatrixTable_t **pLscMatrixTable
)
{
    SP2519_Context_t *pSP2519Ctx = (SP2519_Context_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( SP2519_INFO, "%s: (enter)\n", __FUNCTION__);

    if ( pSP2519Ctx == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    if ( pLscMatrixTable == NULL )
    {
        return ( RET_NULL_POINTER );
    }
    else

    TRACE( SP2519_INFO, "%s: (exit)\n", __FUNCTION__);

    return ( result );
}


/*****************************************************************************/
/**
 *          SP2519_IsiMdiInitMotoDriveMds
 *
 * @brief   General initialisation tasks like I/O initialisation.
 *
 * @param   handle              SP2519 sensor instance handle
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
static RESULT SP2519_IsiMdiInitMotoDriveMds
(
    IsiSensorHandle_t   handle
)
{
    SP2519_Context_t *pSP2519Ctx = (SP2519_Context_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( SP2519_INFO, "%s: (enter)\n", __FUNCTION__);

    if ( pSP2519Ctx == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    TRACE( SP2519_INFO, "%s: (exit)\n", __FUNCTION__);

    return ( result );
}



/*****************************************************************************/
/**
 *          SP2519_IsiMdiSetupMotoDrive
 *
 * @brief   Setup of the MotoDrive and return possible max step.
 *
 * @param   handle          SP2519 sensor instance handle
 *          pMaxStep        pointer to variable to receive the maximum
 *                          possible focus step
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
static RESULT SP2519_IsiMdiSetupMotoDrive
(
    IsiSensorHandle_t   handle,
    uint32_t            *pMaxStep
)
{
    SP2519_Context_t *pSP2519Ctx = (SP2519_Context_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( SP2519_INFO, "%s: (enter)\n", __FUNCTION__);

    if ( pSP2519Ctx == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    if ( pMaxStep == NULL )
    {
        return ( RET_NULL_POINTER );
    }


    TRACE( SP2519_INFO, "%s: (exit)\n", __FUNCTION__);

    return ( result );
}



/*****************************************************************************/
/**
 *          SP2519_IsiMdiFocusSet
 *
 * @brief   Drives the lens system to a certain focus point.
 *
 * @param   handle          SP2519 sensor instance handle
 *          AbsStep         absolute focus point to apply
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
static RESULT SP2519_IsiMdiFocusSet
(
    IsiSensorHandle_t   handle,
    const uint32_t      Position
)
{
    SP2519_Context_t *pSP2519Ctx = (SP2519_Context_t *)handle;

    RESULT result = RET_SUCCESS;

    uint32_t nPosition;
    uint8_t  data[2] = { 0, 0 };

    TRACE( SP2519_INFO, "%s: (enter)\n", __FUNCTION__);

    if ( pSP2519Ctx == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }
    RETURN_RESULT_IF_DIFFERENT( RET_SUCCESS, result );

    TRACE( SP2519_INFO, "%s: (exit)\n", __FUNCTION__);

    return ( result );
}



/*****************************************************************************/
/**
 *          SP2519_IsiMdiFocusGet
 *
 * @brief   Retrieves the currently applied focus point.
 *
 * @param   handle          SP2519 sensor instance handle
 *          pAbsStep        pointer to a variable to receive the current
 *                          focus point
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
static RESULT SP2519_IsiMdiFocusGet
(
    IsiSensorHandle_t   handle,
    uint32_t            *pAbsStep
)
{
    SP2519_Context_t *pSP2519Ctx = (SP2519_Context_t *)handle;

    RESULT result = RET_SUCCESS;
    uint8_t  data[2] = { 0, 0 };

    TRACE( SP2519_INFO, "%s: (enter)\n", __FUNCTION__);

    if ( pSP2519Ctx == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    if ( pAbsStep == NULL )
    {
        return ( RET_NULL_POINTER );
    }

    TRACE( SP2519_INFO, "%s: (exit)\n", __FUNCTION__);

    return ( result );
}



/*****************************************************************************/
/**
 *          SP2519_IsiMdiFocusCalibrate
 *
 * @brief   Triggers a forced calibration of the focus hardware.
 *
 * @param   handle          SP2519 sensor instance handle
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
static RESULT SP2519_IsiMdiFocusCalibrate
(
    IsiSensorHandle_t   handle
)
{
    SP2519_Context_t *pSP2519Ctx = (SP2519_Context_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( SP2519_INFO, "%s: (enter)\n", __FUNCTION__);

    if ( pSP2519Ctx == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    TRACE( SP2519_INFO, "%s: (exit)\n", __FUNCTION__);

    return ( result );
}



/*****************************************************************************/
/**
 *          SP2519_IsiActivateTestPattern
 *
 * @brief   Triggers a forced calibration of the focus hardware.
 *
 * @param   handle          SP2519 sensor instance handle
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NULL_POINTER
 *
 ******************************************************************************/
static RESULT SP2519_IsiActivateTestPattern
(
    IsiSensorHandle_t   handle,
    const bool_t        enable
)
{
    SP2519_Context_t *pSP2519Ctx = (SP2519_Context_t *)handle;

    RESULT result = RET_SUCCESS;

    return ( result );
}



/*****************************************************************************/
/**
 *          SP2519_IsiGetSensorMipiInfoIss
 *
 * @brief   Triggers a forced calibration of the focus hardware.
 *
 * @param   handle          SP2519 sensor instance handle
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NULL_POINTER
 *
 ******************************************************************************/
static RESULT SP2519_IsiGetSensorMipiInfoIss
(
    IsiSensorHandle_t   handle,
    IsiSensorMipiInfo   *ptIsiSensorMipiInfo
)
{
    SP2519_Context_t *pSP2519Ctx = (SP2519_Context_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( SP2519_INFO, "%s: (enter)\n", __FUNCTION__);

    if ( pSP2519Ctx == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }


    if ( ptIsiSensorMipiInfo == NULL )
    {
        return ( result );
    }

    TRACE( SP2519_INFO, "%s: (exit)\n", __FUNCTION__);

    return ( result );
}

static RESULT SP2519_IsiGetSensorIsiVersion
(  IsiSensorHandle_t   handle,
   unsigned int*     pVersion
)
{
    SP2519_Context_t *pSP2519Ctx = (SP2519_Context_t *)handle;

    RESULT result = RET_SUCCESS;


    TRACE( SP2519_INFO, "%s: (enter)\n", __FUNCTION__);

    if ( pSP2519Ctx == NULL )
    {
    	TRACE( SP2519_ERROR, "%s: pSP2519Ctx IS NULL\n", __FUNCTION__);
        return ( RET_WRONG_HANDLE );
    }

	if(pVersion == NULL)
	{
		TRACE( SP2519_ERROR, "%s: pVersion IS NULL\n", __FUNCTION__);
        return ( RET_WRONG_HANDLE );
	}

	*pVersion = CONFIG_ISI_VERSION;
	return result;
}

/*****************************************************************************/
/**
 *          SP2519_IsiGetSensorIss
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
RESULT SP2519_IsiGetSensorIss
(
    IsiSensor_t *pIsiSensor
)
{
    RESULT result = RET_SUCCESS;

    TRACE( SP2519_INFO, "%s (enter)\n", __FUNCTION__);

    if ( pIsiSensor != NULL )
    {
        pIsiSensor->pszName                             = SP2519_g_acName;
        pIsiSensor->pRegisterTable                      = SP2519_g_aRegDescription;
        pIsiSensor->pIsiSensorCaps                      = &SP2519_g_IsiSensorDefaultConfig;
				pIsiSensor->pIsiGetSensorIsiVer									= SP2519_IsiGetSensorIsiVersion;
        pIsiSensor->pIsiCreateSensorIss                 = SP2519_IsiCreateSensorIss;
        pIsiSensor->pIsiReleaseSensorIss                = SP2519_IsiReleaseSensorIss;
        pIsiSensor->pIsiGetCapsIss                      = SP2519_IsiGetCapsIss;
        pIsiSensor->pIsiSetupSensorIss                  = SP2519_IsiSetupSensorIss;
        pIsiSensor->pIsiChangeSensorResolutionIss       = SP2519_IsiChangeSensorResolutionIss;
        pIsiSensor->pIsiSensorSetStreamingIss           = SP2519_IsiSensorSetStreamingIss;
        pIsiSensor->pIsiSensorSetPowerIss               = SP2519_IsiSensorSetPowerIss;
        pIsiSensor->pIsiCheckSensorConnectionIss        = SP2519_IsiCheckSensorConnectionIss;
        pIsiSensor->pIsiGetSensorRevisionIss            = SP2519_IsiGetSensorRevisionIss;
        pIsiSensor->pIsiRegisterReadIss                 = SP2519_IsiRegReadIss;
        pIsiSensor->pIsiRegisterWriteIss                = SP2519_IsiRegWriteIss;

        /* AEC functions */
        pIsiSensor->pIsiExposureControlIss              = SP2519_IsiExposureControlIss;
        pIsiSensor->pIsiGetGainLimitsIss                = SP2519_IsiGetGainLimitsIss;
        pIsiSensor->pIsiGetIntegrationTimeLimitsIss     = SP2519_IsiGetIntegrationTimeLimitsIss;
        pIsiSensor->pIsiGetCurrentExposureIss           = SP2519_IsiGetCurrentExposureIss;
        pIsiSensor->pIsiGetGainIss                      = SP2519_IsiGetGainIss;
        pIsiSensor->pIsiGetGainIncrementIss             = SP2519_IsiGetGainIncrementIss;
        pIsiSensor->pIsiSetGainIss                      = SP2519_IsiSetGainIss;
        pIsiSensor->pIsiGetIntegrationTimeIss           = SP2519_IsiGetIntegrationTimeIss;
        pIsiSensor->pIsiGetIntegrationTimeIncrementIss  = SP2519_IsiGetIntegrationTimeIncrementIss;
        pIsiSensor->pIsiSetIntegrationTimeIss           = SP2519_IsiSetIntegrationTimeIss;
        pIsiSensor->pIsiGetResolutionIss                = SP2519_IsiGetResolutionIss;
        pIsiSensor->pIsiGetAfpsInfoIss                  = SP2519_IsiGetAfpsInfoIss;

        /* AWB specific functions */
        pIsiSensor->pIsiGetCalibKFactor                 = SP2519_IsiGetCalibKFactor;
        pIsiSensor->pIsiGetCalibPcaMatrix               = SP2519_IsiGetCalibPcaMatrix;
        pIsiSensor->pIsiGetCalibSvdMeanValue            = SP2519_IsiGetCalibSvdMeanValue;
        pIsiSensor->pIsiGetCalibCenterLine              = SP2519_IsiGetCalibCenterLine;
        pIsiSensor->pIsiGetCalibClipParam               = SP2519_IsiGetCalibClipParam;
        pIsiSensor->pIsiGetCalibGlobalFadeParam         = SP2519_IsiGetCalibGlobalFadeParam;
        pIsiSensor->pIsiGetCalibFadeParam               = SP2519_IsiGetCalibFadeParam;
        pIsiSensor->pIsiGetIlluProfile                  = SP2519_IsiGetIlluProfile;
        pIsiSensor->pIsiGetLscMatrixTable               = SP2519_IsiGetLscMatrixTable;

        /* AF functions */
        pIsiSensor->pIsiMdiInitMotoDriveMds             = SP2519_IsiMdiInitMotoDriveMds;
        pIsiSensor->pIsiMdiSetupMotoDrive               = SP2519_IsiMdiSetupMotoDrive;
        pIsiSensor->pIsiMdiFocusSet                     = SP2519_IsiMdiFocusSet;
        pIsiSensor->pIsiMdiFocusGet                     = SP2519_IsiMdiFocusGet;
        pIsiSensor->pIsiMdiFocusCalibrate               = SP2519_IsiMdiFocusCalibrate;

        /* MIPI */
        pIsiSensor->pIsiGetSensorMipiInfoIss            = SP2519_IsiGetSensorMipiInfoIss;

        /* Testpattern */
        pIsiSensor->pIsiActivateTestPattern             = SP2519_IsiActivateTestPattern;
    }
    else
    {
        result = RET_NULL_POINTER;
    }

    TRACE( SP2519_INFO, "%s (exit)\n", __FUNCTION__);

    return ( result );
}

static RESULT SP2519_IsiGetSensorI2cInfo(sensor_i2c_info_t** pdata)
{
    sensor_i2c_info_t* pSensorI2cInfo;
	
   TRACE( SP2519_INFO, "%s (%d)(enter)\n", __FUNCTION__,__LINE__);
    pSensorI2cInfo = ( sensor_i2c_info_t * )malloc ( sizeof (sensor_i2c_info_t) );

    if ( pSensorI2cInfo == NULL )
    {
        TRACE( SP2519_ERROR,  "%s: Can't allocate ov14825 context\n",  __FUNCTION__ );
        return ( RET_OUTOFMEM );
    }
    MEMSET( pSensorI2cInfo, 0, sizeof( sensor_i2c_info_t ) );

    
    pSensorI2cInfo->i2c_addr = SP2519_SLAVE_ADDR;
    pSensorI2cInfo->soft_reg_addr = SP2519_SOFTWARE_RST;
    pSensorI2cInfo->soft_reg_value = SP2519_SOFTWARE_RST_DATA;
    pSensorI2cInfo->reg_size = SP2519_I2C_NR_ADR_BYTES;
    pSensorI2cInfo->value_size = SP2519_I2C_NR_DAT_BYTES;

    {
        IsiSensorCaps_t Caps;
        sensor_caps_t *pCaps;
        uint32_t lanes,i;
        
        ListInit(&pSensorI2cInfo->lane_res[0]);
        ListInit(&pSensorI2cInfo->lane_res[1]);
        ListInit(&pSensorI2cInfo->lane_res[2]);
        
        Caps.Index = 0;            
        while(SP2519_IsiGetCapsIssInternal(&Caps)==RET_SUCCESS) {
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
    pChipIDInfo_H->chipid_reg_addr = SP2519_CHIP_ID_HIGH_BYTE;  
    pChipIDInfo_H->chipid_reg_value = SP2519_CHIP_ID_HIGH_BYTE_DEFAULT;
    ListPrepareItem( pChipIDInfo_H );
    ListAddTail( &pSensorI2cInfo->chipid_info, pChipIDInfo_H );

    sensor_chipid_info_t* pChipIDInfo_M = (sensor_chipid_info_t *) malloc( sizeof(sensor_chipid_info_t) );
    if ( !pChipIDInfo_M )
    {
        return RET_OUTOFMEM;
    }
    MEMSET( pChipIDInfo_M, 0, sizeof(*pChipIDInfo_M) ); 
    pChipIDInfo_M->chipid_reg_addr = SP2519_CHIP_ID_MIDDLE_BYTE;
    pChipIDInfo_M->chipid_reg_value = SP2519_CHIP_ID_MIDDLE_BYTE_DEFAULT;
    ListPrepareItem( pChipIDInfo_M );
    ListAddTail( &pSensorI2cInfo->chipid_info, pChipIDInfo_M );
    
    sensor_chipid_info_t* pChipIDInfo_L = (sensor_chipid_info_t *) malloc( sizeof(sensor_chipid_info_t) );
    if ( !pChipIDInfo_L )
    {
        return RET_OUTOFMEM;
    }
    MEMSET( pChipIDInfo_L, 0, sizeof(*pChipIDInfo_L) ); 
    pChipIDInfo_L->chipid_reg_addr = SP2519_CHIP_ID_LOW_BYTE;
    pChipIDInfo_L->chipid_reg_value = SP2519_CHIP_ID_LOW_BYTE_DEFAULT;
    ListPrepareItem( pChipIDInfo_L );
    ListAddTail( &pSensorI2cInfo->chipid_info, pChipIDInfo_L );

	//oyyf sensor drv version
	pSensorI2cInfo->sensor_drv_version = CONFIG_SENSOR_DRV_VERSION;
	
    *pdata = pSensorI2cInfo;
     TRACE( SP2519_INFO, "%s (%d)(exit)\n", __FUNCTION__,__LINE__);
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
    SP2519_IsiGetSensorIss,
    {
        0,                      /**< IsiSensor_t.pszName */
        0,                      /**< IsiSensor_t.pRegisterTable */
        0,                      /**< IsiSensor_t.pIsiSensorCaps */
        0,											/**< IsiSensor_t.pIsiGetSensorIsiVer_t>*/   //oyyf add
        0,                      /**< IsiSensor_t.pIsiGetSensorTuningXmlVersion_t>*/   //oyyf add 
        0,                      /**< IsiSensor_t.pIsiWhiteBalanceIlluminationChk>*/   //ddl@rock-chips.com 
        0,                      /**< IsiSensor_t.pIsiWhiteBalanceIlluminationSet>*/   //ddl@rock-chips.com
        0,                      /**< IsiSensor_t.pIsiCheckOTPInfo>*/  //zyc 
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
    },
    SP2519_IsiGetSensorI2cInfo,
};



