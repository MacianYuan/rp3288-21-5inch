#include <ebase/types.h>
#include <ebase/trace.h>
#include <ebase/builtins.h>

#include <common/return_codes.h>
#include <common/misc.h>

#include "isi.h"
#include "isi_iss.h"
#include "isi_priv.h"

#include "S5K4H8_MIPI_priv.h"

#define  S5K4H8_NEWEST_TUNING_XML "S5K4H8_v0.1.0"

/******************************************************************************
 * local macro definitions
 *****************************************************************************/
CREATE_TRACER( S5K4H8_INFO , "S5K4H8: ", INFO,    0U );
CREATE_TRACER( S5K4H8_WARN , "S5K4H8: ", WARNING, 1U );
CREATE_TRACER( S5K4H8_ERROR, "S5K4H8: ", ERROR,   1U );

CREATE_TRACER( S5K4H8_DEBUG, "S5K4H8: ", INFO,     0U );

CREATE_TRACER( S5K4H8_NOTICE0 , "S5K4H8: ", TRACE_NOTICE0, 1);
CREATE_TRACER( S5K4H8_NOTICE1, "S5K4H8: ", TRACE_NOTICE1, 1U );


#define S5K4H8_SLAVE_ADDR       0x5aU
#define S5K4H8_SLAVE_ADDR2      0x20U
#define S5K4H8_SLAVE_AF_ADDR    0x18U //VCM driver is GT9760s
#define Sensor_OTP_SLAVE_ADDR   0x20U

#define S5K4H8_MAXN_GAIN 		(32.0f)
#define S5K4H8_MIN_GAIN_STEP   ( 1.0f / S5K4H8_MAXN_GAIN);
#define S5K4H8_MAX_GAIN_AEC    ( 8.0f )    // 16.0


/*!<
 * Focus position values:
 * 65 logical positions ( 0 - 64 )
 * where 64 is the setting for infinity and 0 for macro
 * corresponding to
 * 1024 register settings (0 - 1023)
 * where 0 is the setting for infinity and 1023 for macro
 */
#define MAX_LOG   64U
#define MAX_REG 1023U

#define MAX_VCMDRV_CURRENT      100U
#define MAX_VCMDRV_REG          1023U


/*!<
 * Lens movement is triggered every 133ms (VGA, 7.5fps processed frames
 * worst case assumed, usually even much slower, see OV5630 driver for
 * details). Thus the lens has to reach the requested position after
 * max. 133ms. Minimum mechanical ringing is expected with mode 1 ,
 * 100us per step. A movement over the full range needs max. 102.3ms
 * (see table 9 AD5820 datasheet).
 */
#define MDI_SLEW_RATE_CTRL 5U /* S3..0 for MOTOR*/


/******************************************************************************
 * local variable declarations
 *****************************************************************************/
const char S5K4H8_g_acName[] = "S5K4H8_MIPI";
extern const IsiRegDescription_t S5K4H8_g_aRegDescription_lsc_OTP_patch[];
extern const IsiRegDescription_t S5K4H8_g_aRegDescription_fourlane[];
extern const IsiRegDescription_t S5K4H8_g_1632x1224_fourlane[];
extern const IsiRegDescription_t S5K4H8_g_1632x1224P30_fourlane_fpschg[];
extern const IsiRegDescription_t S5K4H8_g_1632x1224P25_fourlane_fpschg[];
extern const IsiRegDescription_t S5K4H8_g_1632x1224P20_fourlane_fpschg[];
extern const IsiRegDescription_t S5K4H8_g_1632x1224P15_fourlane_fpschg[];
extern const IsiRegDescription_t S5K4H8_g_1632x1224P10_fourlane_fpschg[];
extern const IsiRegDescription_t S5K4H8_g_3264x2448_fourlane[];
extern const IsiRegDescription_t S5K4H8_g_3264x2448P30_fourlane_fpschg[];
extern const IsiRegDescription_t S5K4H8_g_3264x2448P25_fourlane_fpschg[];
extern const IsiRegDescription_t S5K4H8_g_3264x2448P20_fourlane_fpschg[];
extern const IsiRegDescription_t S5K4H8_g_3264x2448P15_fourlane_fpschg[];
extern const IsiRegDescription_t S5K4H8_g_3264x2448P10_fourlane_fpschg[];
extern const IsiRegDescription_t S5K4H8_g_3264x2448P7_fourlane_fpschg[];
extern const IsiRegDescription_t S5K4H8_g_1632x1224_fourlane_560M[];
extern const IsiRegDescription_t S5K4H8_g_1632x1224P30_fourlane_560M_fpschg[];
extern const IsiRegDescription_t S5K4H8_g_1632x1224P25_fourlane_560M_fpschg[];
extern const IsiRegDescription_t S5K4H8_g_1632x1224P20_fourlane_560M_fpschg[];
extern const IsiRegDescription_t S5K4H8_g_1632x1224P15_fourlane_560M_fpschg[];
extern const IsiRegDescription_t S5K4H8_g_1632x1224P10_fourlane_560M_fpschg[];
extern const IsiRegDescription_t S5K4H8_g_3264x2448_fourlane_560M[];
extern const IsiRegDescription_t S5K4H8_g_3264x2448P25_fourlane_560M_fpschg[];
extern const IsiRegDescription_t S5K4H8_g_3264x2448P20_fourlane_560M_fpschg[];
extern const IsiRegDescription_t S5K4H8_g_3264x2448P15_fourlane_560M_fpschg[];
extern const IsiRegDescription_t S5K4H8_g_3264x2448P10_fourlane_560M_fpschg[];
extern const IsiRegDescription_t S5K4H8_g_3264x2448P7_fourlane_560M_fpschg[];

extern const IsiRegDescription_t S5K4H8_g_1632x1224_twolane[];
extern const IsiRegDescription_t S5K4H8_g_1632x1224P30_twolane_fpschg[];
extern const IsiRegDescription_t S5K4H8_g_1632x1224P25_twolane_fpschg[];
extern const IsiRegDescription_t S5K4H8_g_1632x1224P20_twolane_fpschg[];
extern const IsiRegDescription_t S5K4H8_g_1632x1224P15_twolane_fpschg[];
extern const IsiRegDescription_t S5K4H8_g_1632x1224P10_twolane_fpschg[];
extern const IsiRegDescription_t S5K4H8_g_3264x2448_twolane[];
extern const IsiRegDescription_t S5K4H8_g_3264x2448P15_twolane_fpschg[];
extern const IsiRegDescription_t S5K4H8_g_3264x2448P10_twolane_fpschg[];
extern const IsiRegDescription_t S5K4H8_g_3264x2448P7_twolane_fpschg[];
const IsiSensorCaps_t S5K4H8_g_IsiSensorDefaultConfig;

#define S5K4H8_I2C_START_BIT        (I2C_COMPLIANT_STARTBIT)    // I2C bus start condition
#define S5K4H8_I2C_NR_ADR_BYTES     (2U)                        // 1 byte base address and 2 bytes sub address
#define S5K4H8_I2C_NR_DAT_BYTES     (1U)                        // 8 bit registers

static uint16_t g_suppoted_mipi_lanenum_type = SUPPORT_MIPI_TWO_LANE|SUPPORT_MIPI_FOUR_LANE;
#define DEFAULT_NUM_LANES SUPPORT_MIPI_FOUR_LANE


/******************************************************************************
 * local function prototypes
 *****************************************************************************/
static RESULT S5K4H8_IsiCreateSensorIss( IsiSensorInstanceConfig_t *pConfig );
static RESULT S5K4H8_IsiReleaseSensorIss( IsiSensorHandle_t handle );
static RESULT S5K4H8_IsiGetCapsIss( IsiSensorHandle_t handle, IsiSensorCaps_t *pIsiSensorCaps );
static RESULT S5K4H8_IsiSetupSensorIss( IsiSensorHandle_t handle, const IsiSensorConfig_t *pConfig );
static RESULT S5K4H8_IsiSensorSetStreamingIss( IsiSensorHandle_t handle, bool_t on );
static RESULT S5K4H8_IsiSensorSetPowerIss( IsiSensorHandle_t handle, bool_t on );
static RESULT S5K4H8_IsiCheckSensorConnectionIss( IsiSensorHandle_t handle );
static RESULT S5K4H8_IsiGetSensorRevisionIss( IsiSensorHandle_t handle, uint32_t *p_value);

static RESULT S5K4H8_IsiGetGainLimitsIss( IsiSensorHandle_t handle, float *pMinGain, float *pMaxGain);
static RESULT S5K4H8_IsiGetIntegrationTimeLimitsIss( IsiSensorHandle_t handle, float *pMinIntegrationTime, float *pMaxIntegrationTime );
static RESULT S5K4H8_IsiExposureControlIss( IsiSensorHandle_t handle, float NewGain, float NewIntegrationTime, uint8_t *pNumberOfFramesToSkip, float *pSetGain, float *pSetIntegrationTime );
static RESULT S5K4H8_IsiGetCurrentExposureIss( IsiSensorHandle_t handle, float *pSetGain, float *pSetIntegrationTime );
static RESULT S5K4H8_IsiGetAfpsInfoIss ( IsiSensorHandle_t handle, uint32_t Resolution, IsiAfpsInfo_t* pAfpsInfo);
static RESULT S5K4H8_IsiGetGainIss( IsiSensorHandle_t handle, float *pSetGain );
static RESULT S5K4H8_IsiGetGainIncrementIss( IsiSensorHandle_t handle, float *pIncr );
static RESULT S5K4H8_IsiSetGainIss( IsiSensorHandle_t handle, float NewGain, float *pSetGain );
static RESULT S5K4H8_IsiGetIntegrationTimeIss( IsiSensorHandle_t handle, float *pSetIntegrationTime );
static RESULT S5K4H8_IsiGetIntegrationTimeIncrementIss( IsiSensorHandle_t handle, float *pIncr );
static RESULT S5K4H8_IsiSetIntegrationTimeIss( IsiSensorHandle_t handle, float NewIntegrationTime, float *pSetIntegrationTime, uint8_t *pNumberOfFramesToSkip );
static RESULT S5K4H8_IsiGetResolutionIss( IsiSensorHandle_t handle, uint32_t *pSetResolution );


static RESULT S5K4H8_IsiRegReadIss( IsiSensorHandle_t handle, const uint32_t address, uint32_t *p_value );
static RESULT S5K4H8_IsiRegWriteIss( IsiSensorHandle_t handle, const uint32_t address, const uint32_t value );
static RESULT S5K4H8_IsiRegReadIssEx( IsiSensorHandle_t handle, const uint32_t address, uint32_t *p_value, uint8_t bytes);
static RESULT S5K4H8_IsiRegWriteIssEx( IsiSensorHandle_t handle, const uint32_t address, const uint32_t value, uint8_t bytes);

static RESULT S5K4H8_IsiGetCalibKFactor( IsiSensorHandle_t handle, Isi1x1FloatMatrix_t **pIsiKFactor );
static RESULT S5K4H8_IsiGetCalibPcaMatrix( IsiSensorHandle_t   handle, Isi3x2FloatMatrix_t **pIsiPcaMatrix );
static RESULT S5K4H8_IsiGetCalibSvdMeanValue( IsiSensorHandle_t   handle, Isi3x1FloatMatrix_t **pIsiSvdMeanValue );
static RESULT S5K4H8_IsiGetCalibCenterLine( IsiSensorHandle_t   handle, IsiLine_t  **ptIsiCenterLine);
static RESULT S5K4H8_IsiGetCalibClipParam( IsiSensorHandle_t   handle, IsiAwbClipParm_t    **pIsiClipParam );
static RESULT S5K4H8_IsiGetCalibGlobalFadeParam( IsiSensorHandle_t       handle, IsiAwbGlobalFadeParm_t  **ptIsiGlobalFadeParam);
static RESULT S5K4H8_IsiGetCalibFadeParam( IsiSensorHandle_t   handle, IsiAwbFade2Parm_t   **ptIsiFadeParam);
static RESULT S5K4H8_IsiGetIlluProfile( IsiSensorHandle_t   handle, const uint32_t CieProfile, IsiIlluProfile_t **ptIsiIlluProfile );

static RESULT S5K4H8_IsiMdiInitMotoDriveMds( IsiSensorHandle_t handle );
static RESULT S5K4H8_IsiMdiSetupMotoDrive( IsiSensorHandle_t handle, uint32_t *pMaxStep );
static RESULT S5K4H8_IsiMdiFocusSet( IsiSensorHandle_t handle, const uint32_t Position );
static RESULT S5K4H8_IsiMdiFocusGet( IsiSensorHandle_t handle, uint32_t *pAbsStep );
static RESULT S5K4H8_IsiMdiFocusCalibrate( IsiSensorHandle_t handle );

static RESULT S5K4H8_IsiGetSensorMipiInfoIss( IsiSensorHandle_t handle, IsiSensorMipiInfo *ptIsiSensorMipiInfo);
static RESULT S5K4H8_IsiGetSensorIsiVersion(  IsiSensorHandle_t   handle, unsigned int* pVersion);
static RESULT S5K4H8_IsiGetSensorTuningXmlVersion(  IsiSensorHandle_t   handle, char** pTuningXmlVersion);


static float dctfloor( const float f )
{
    if ( f < 0 )
    {
        return ( (float)((int32_t)f - 1L) );
    }
    else
    {
        return ( (float)((uint32_t)f) );
    }
}

/* OTP START*/
struct otp_struct {
    int flag;
    int module_integrator_id;
    int production_year;
    int production_month;
    int production_day;
    int rg_ratio;
    int bg_ratio;
};

static struct otp_struct g_otp_info ={0};

int  RG_Ratio_Typical=0;
int  BG_Ratio_Typical=0;

#define RG_Ratio_Typical_Default 0x281;
#define BG_Ratio_Typical_Default 0x259;

static bool bDumpRaw_OTP_switch = false;

static int check_read_otp(
	sensor_i2c_write_t*  sensor_i2c_write_p,
	sensor_i2c_read_t*	sensor_i2c_read_p,
	sensor_version_get_t* sensor_version_get_p,
	void* context,
	int camsys_fd
)
{
	int i2c_base_info[3];
	int temp1;
	int ret=0;

	TRACE( S5K4H8_INFO, "%s(%d): check_read_otp\n", __FUNCTION__, __LINE__);	
    i2c_base_info[0] = Sensor_OTP_SLAVE_ADDR; //otp i2c addr
    i2c_base_info[1] = 2; //otp i2c reg size
    i2c_base_info[2] = 1; //otp i2c value size
    //stream on

    ret = sensor_i2c_write_p(context,camsys_fd, S5K4H8_MODE_SELECT, S5K4H8_MODE_SELECT_ON, i2c_base_info );
    if(ret < 0){
		TRACE( S5K4H8_ERROR, "%s(%d): S5K4H8_MODE_SELECT_ON fail!\n", __FUNCTION__, __LINE__);
		goto fail;
	}

    // set the page15 of OTP
    ret = sensor_i2c_write_p(context,camsys_fd, 0x0A02, 0x0f, i2c_base_info );
    if(ret < 0){
		TRACE( S5K4H8_ERROR, "%s(%d): read/write OTP data fail!\n", __FUNCTION__, __LINE__);
		goto fail;
	}

	//OTP enable and read start
	i2c_base_info[1] = 2;
	i2c_base_info[2] = 2;
    ret = sensor_i2c_write_p(context,camsys_fd, 0x0A00, 0x0100, i2c_base_info );
    if(ret < 0){
		TRACE( S5K4H8_ERROR, "%s(%d): read/write OTP data fail!\n", __FUNCTION__, __LINE__);
		goto fail;
	}

	osSleep( 5 );

	//OTP data read
	i2c_base_info[1] = 2;
	i2c_base_info[2] = 1;	
    temp1 = sensor_i2c_read_p(context,camsys_fd,0x0A04,i2c_base_info);
	g_otp_info.flag = temp1;

	if(g_otp_info.flag == 0x10){
	    temp1 = sensor_i2c_read_p(context,camsys_fd,0x0A05,i2c_base_info);
		g_otp_info.module_integrator_id = temp1;
	    temp1 = sensor_i2c_read_p(context,camsys_fd,0x0A06,i2c_base_info);
		g_otp_info.production_year = temp1;
	    temp1 = sensor_i2c_read_p(context,camsys_fd,0x0A07,i2c_base_info);
		g_otp_info.production_month = temp1;
	    temp1 = sensor_i2c_read_p(context,camsys_fd,0x0A08,i2c_base_info);
		g_otp_info.production_day = temp1;

	    temp1 = sensor_i2c_read_p(context,camsys_fd,0x0A09,i2c_base_info);
		g_otp_info.rg_ratio = (temp1&0x3)<<8;
	    temp1 = sensor_i2c_read_p(context,camsys_fd,0x0A0A,i2c_base_info);
		g_otp_info.rg_ratio |= temp1&0xff;
	    temp1 = sensor_i2c_read_p(context,camsys_fd,0x0A0B,i2c_base_info);
		g_otp_info.bg_ratio = (temp1&0x3)<<8;
	    temp1 = sensor_i2c_read_p(context,camsys_fd,0x0A0C,i2c_base_info);
		g_otp_info.bg_ratio |= temp1&0xff;
	}else if(g_otp_info.flag == 0xf1){
	    temp1 = sensor_i2c_read_p(context,camsys_fd,0x0A25,i2c_base_info);
		g_otp_info.module_integrator_id = temp1;
	    temp1 = sensor_i2c_read_p(context,camsys_fd,0x0A26,i2c_base_info);
		g_otp_info.production_year = temp1;
	    temp1 = sensor_i2c_read_p(context,camsys_fd,0x0A27,i2c_base_info);
		g_otp_info.production_month = temp1;
	    temp1 = sensor_i2c_read_p(context,camsys_fd,0x0A28,i2c_base_info);
		g_otp_info.production_day = temp1;

	    temp1 = sensor_i2c_read_p(context,camsys_fd,0x0A29,i2c_base_info);
		g_otp_info.rg_ratio = (temp1&0x3)<<8;
	    temp1 = sensor_i2c_read_p(context,camsys_fd,0x0A2A,i2c_base_info);
		g_otp_info.rg_ratio |= temp1&0xff;
	    temp1 = sensor_i2c_read_p(context,camsys_fd,0x0A2B,i2c_base_info);
		g_otp_info.bg_ratio = (temp1&0x3)<<8;
	    temp1 = sensor_i2c_read_p(context,camsys_fd,0x0A2C,i2c_base_info);
		g_otp_info.bg_ratio |= temp1&0xff;
	}else if((g_otp_info.flag&0x3) == 0){
		goto notsupp;
	}
	TRACE( S5K4H8_ERROR, "%s(%d): rg=0x%x,bg=0x%x!\n", __FUNCTION__, __LINE__,g_otp_info.rg_ratio,g_otp_info.bg_ratio);

	//OTP enable and read end
	i2c_base_info[1] = 2;
	i2c_base_info[2] = 2;
    ret = sensor_i2c_write_p(context,camsys_fd, 0x0A00, 0x0000, i2c_base_info );
    if(ret < 0){
		TRACE( S5K4H8_ERROR, "%s(%d): read/write OTP data fail!\n", __FUNCTION__, __LINE__);
		goto fail;
	}
	osSleep( 5 );
	//stream off
	i2c_base_info[1] = 2;
	i2c_base_info[2] = 1;
	ret = sensor_i2c_write_p(context,camsys_fd, S5K4H8_MODE_SELECT, S5K4H8_MODE_SELECT_OFF, i2c_base_info );
    if(ret < 0){
		TRACE( S5K4H8_ERROR, "%s(%d): S5K4H8_MODE_SELECT_OFF fail!\n", __FUNCTION__, __LINE__);
		goto fail;
	}
success:
	return RET_SUCCESS;
notsupp:
	return RET_NOTSUPP;
fail:
	return RET_FAILURE;
}

static int apply_otp_data(IsiSensorHandle_t   handle,struct otp_struct *otp_ptr)
{
#define GAIN_DEFAULT       0x0100
	
	S5K4H8_Context_t *pS5K4H8Ctx = (S5K4H8_Context_t *)handle;
	uint16_t r_ratio, b_ratio;
	char prop_value[PROPERTY_VALUE_MAX];

	r_ratio = 512 * (RG_Ratio_Typical) / (otp_ptr->rg_ratio);
	b_ratio = 512 * (BG_Ratio_Typical) / (otp_ptr->bg_ratio);

	if (!r_ratio || !b_ratio)
	{
		TRACE( S5K4H8_ERROR, "%s(%d): r_ratio or b_ratio is wrong!\n", __FUNCTION__, __LINE__);
		return	RET_FAILURE;
	}
	uint16_t R_GAIN;
	uint16_t B_GAIN;
	uint16_t Gr_GAIN;
	uint16_t Gb_GAIN;
	uint16_t G_GAIN;

	if (r_ratio >= 512)
	{
		if (b_ratio >= 512)
		{
			R_GAIN = (uint16_t)(GAIN_DEFAULT * r_ratio / 512);
			G_GAIN = GAIN_DEFAULT;
			B_GAIN = (uint16_t)(GAIN_DEFAULT * b_ratio / 512);
		}
		else
		{
			R_GAIN = (uint16_t)(GAIN_DEFAULT * r_ratio / b_ratio);
			G_GAIN = (uint16_t)(GAIN_DEFAULT * 512 / b_ratio);
			B_GAIN = GAIN_DEFAULT;
		}
	}
	else
	{
		if (b_ratio >= 512)
		{
			R_GAIN = GAIN_DEFAULT;
			G_GAIN = (uint16_t)(GAIN_DEFAULT * 512 / r_ratio);
			B_GAIN = (uint16_t)(GAIN_DEFAULT *  b_ratio / r_ratio);
		}
		else
		{
			Gr_GAIN = (uint16_t)(GAIN_DEFAULT * 512 / r_ratio);
			Gb_GAIN = (uint16_t)(GAIN_DEFAULT * 512 / b_ratio);

			if (Gr_GAIN >= Gb_GAIN)
			{
				R_GAIN = GAIN_DEFAULT;
				G_GAIN = (uint16_t)(GAIN_DEFAULT * 512 / r_ratio);
				B_GAIN = (uint16_t)(GAIN_DEFAULT * b_ratio / r_ratio);
			}
			else
			{
				R_GAIN = (uint16_t)(GAIN_DEFAULT * r_ratio / b_ratio);
				G_GAIN = (uint16_t)(GAIN_DEFAULT * 512 / b_ratio);
				B_GAIN = GAIN_DEFAULT;
			}
		}
	}
	// update sensor WB gain
	property_get("sys_graphic.cam_otp_awb_enable", prop_value, "true");
	if (!strcmp(prop_value,"true")) {	
		S5K4H8_IsiRegWriteIssEx ( pS5K4H8Ctx, 0x6028, 0x4000, 2);
		S5K4H8_IsiRegWriteIssEx ( pS5K4H8Ctx, 0x602a, 0x3058, 2);
		S5K4H8_IsiRegWriteIssEx ( pS5K4H8Ctx, 0x6f12, 0x01, 1);
		S5K4H8_IsiRegWriteIssEx ( pS5K4H8Ctx, 0x020e, G_GAIN, 2);	
		S5K4H8_IsiRegWriteIssEx ( pS5K4H8Ctx, 0x0210, R_GAIN, 2);
		S5K4H8_IsiRegWriteIssEx ( pS5K4H8Ctx, 0x0212, B_GAIN, 2);
		S5K4H8_IsiRegWriteIssEx ( pS5K4H8Ctx, 0x0214, G_GAIN, 2);
	}
#if 0//it seems not work here
	//apply lsc data
	IsiRegDefaultsApply( pS5K4H8Ctx, S5K4H8_g_aRegDescription_lsc_OTP_patch);
	osSleep(10);
	S5K4H8_IsiRegWriteIssEx ( pS5K4H8Ctx, 0x6028, 0x4000, 2);
    S5K4H8_IsiRegWriteIssEx ( pS5K4H8Ctx, 0x602A, 0x0B00, 2);
    S5K4H8_IsiRegWriteIssEx ( pS5K4H8Ctx, 0x6F12, 0x01, 1);
    osSleep(100);
#endif
	TRACE( S5K4H8_NOTICE0,  "%s: success!!!\n",  __FUNCTION__ );
	return RET_SUCCESS;
}

static RESULT S5K4H8_IsiSetOTPInfo
(
    IsiSensorHandle_t       handle,
    uint32_t OTPInfo
)
{
	RESULT result = RET_SUCCESS;

    S5K4H8_Context_t *pS5K4H8Ctx = (S5K4H8_Context_t *)handle;

    TRACE( S5K4H8_INFO, "%s (enter)\n", __FUNCTION__);

    if ( pS5K4H8Ctx == NULL )
    {
        TRACE( S5K4H8_ERROR, "%s: Invalid sensor handle (NULL pointer detected)\n", __FUNCTION__ );
        return ( RET_WRONG_HANDLE );
    }

	RG_Ratio_Typical = OTPInfo>>16;
	BG_Ratio_Typical = OTPInfo&0xffff;
	TRACE( S5K4H8_NOTICE0, "%s:  --(RG,BG) in IQ file:(0x%x, 0x%x)\n", __FUNCTION__ , RG_Ratio_Typical, BG_Ratio_Typical);
	if((RG_Ratio_Typical==0) && (BG_Ratio_Typical==0)){
		TRACE( S5K4H8_ERROR, "%s:  --OTP typical value in IQ file is zero, we will try another match rule.\n", __FUNCTION__);
        RG_Ratio_Typical = RG_Ratio_Typical_Default;
        BG_Ratio_Typical = BG_Ratio_Typical_Default;
	}
	TRACE( S5K4H8_NOTICE0, "%s:  --Finally, the (RG,BG) is (0x%x, 0x%x)\n", __FUNCTION__ , RG_Ratio_Typical, BG_Ratio_Typical);

	return (result);
}

static RESULT S5K4H8_IsiEnableOTP
(
    IsiSensorHandle_t       handle,
    const bool_t enable
)
{
	RESULT result = RET_SUCCESS;

    S5K4H8_Context_t *pS5K4H8Ctx = (S5K4H8_Context_t *)handle;

    TRACE( S5K4H8_INFO, "%s (enter)\n", __FUNCTION__);

    if ( pS5K4H8Ctx == NULL )
    {
        TRACE( S5K4H8_ERROR, "%s: Invalid sensor handle (NULL pointer detected)\n", __FUNCTION__ );
        return ( RET_WRONG_HANDLE );
    }
	bDumpRaw_OTP_switch = enable;
	return (result);
}

/* OTP END*/


/*****************************************************************************/
/**
 *          S5K4H8_IsiCreateSensorIss
 *
 * @brief   This function creates a new S5K4H8 sensor instance handle.
 *
 * @param   pConfig     configuration structure to create the instance
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_NULL_POINTER
 * @retval  RET_OUTOFMEM
 *
 *****************************************************************************/
static RESULT S5K4H8_IsiCreateSensorIss
(
    IsiSensorInstanceConfig_t *pConfig
)
{
    RESULT result = RET_SUCCESS;
	int32_t current_distance;
    S5K4H8_Context_t *pS5K4H8Ctx;

    TRACE( S5K4H8_INFO, "%s (enter)\n", __FUNCTION__);

    if ( (pConfig == NULL) || (pConfig->pSensor ==NULL) )
    {
        return ( RET_NULL_POINTER );
    }

    pS5K4H8Ctx = ( S5K4H8_Context_t * )malloc ( sizeof (S5K4H8_Context_t) );
    if ( pS5K4H8Ctx == NULL )
    {
        TRACE( S5K4H8_ERROR,  "%s: Can't allocate S5K4H8 context\n",  __FUNCTION__ );
        return ( RET_OUTOFMEM );
    }
    MEMSET( pS5K4H8Ctx, 0, sizeof( S5K4H8_Context_t ) );

    result = HalAddRef( pConfig->HalHandle );
    if ( result != RET_SUCCESS )
    {
        free ( pS5K4H8Ctx );
        return ( result );
    }
    
    pS5K4H8Ctx->IsiCtx.HalHandle              = pConfig->HalHandle;
    pS5K4H8Ctx->IsiCtx.HalDevID               = pConfig->HalDevID;
    pS5K4H8Ctx->IsiCtx.I2cBusNum              = pConfig->I2cBusNum;
    pS5K4H8Ctx->IsiCtx.SlaveAddress           = ( pConfig->SlaveAddr == 0 ) ? S5K4H8_SLAVE_ADDR : pConfig->SlaveAddr;
    pS5K4H8Ctx->IsiCtx.NrOfAddressBytes       = 2U;

    pS5K4H8Ctx->IsiCtx.I2cAfBusNum            = pConfig->I2cAfBusNum;
    pS5K4H8Ctx->IsiCtx.SlaveAfAddress         = ( pConfig->SlaveAfAddr == 0 ) ? S5K4H8_SLAVE_AF_ADDR : pConfig->SlaveAfAddr;
    pS5K4H8Ctx->IsiCtx.NrOfAfAddressBytes     = 0U;

    pS5K4H8Ctx->IsiCtx.pSensor                = pConfig->pSensor;

    pS5K4H8Ctx->Configured             = BOOL_FALSE;
    pS5K4H8Ctx->Streaming              = BOOL_FALSE;
    pS5K4H8Ctx->TestPattern            = BOOL_FALSE;
    pS5K4H8Ctx->isAfpsRun              = BOOL_FALSE;
    /* ddl@rock-chips.com: v0.3.0 */
    current_distance = pConfig->VcmRatedCurrent - pConfig->VcmStartCurrent;
    current_distance = current_distance*MAX_VCMDRV_REG/MAX_VCMDRV_CURRENT;    
    pS5K4H8Ctx->VcmInfo.Step = (current_distance+(MAX_LOG-1))/MAX_LOG;
    pS5K4H8Ctx->VcmInfo.StartCurrent   = pConfig->VcmStartCurrent*MAX_VCMDRV_REG/MAX_VCMDRV_CURRENT;    
    pS5K4H8Ctx->VcmInfo.RatedCurrent   = pS5K4H8Ctx->VcmInfo.StartCurrent + MAX_LOG*pS5K4H8Ctx->VcmInfo.Step;
    pS5K4H8Ctx->VcmInfo.StepMode       = pConfig->VcmStepMode;    
	
	pS5K4H8Ctx->IsiSensorMipiInfo.sensorHalDevID = pS5K4H8Ctx->IsiCtx.HalDevID;
	if(pConfig->mipiLaneNum & g_suppoted_mipi_lanenum_type)
        pS5K4H8Ctx->IsiSensorMipiInfo.ucMipiLanes = pConfig->mipiLaneNum;
    else{
        TRACE( S5K4H8_ERROR, "%s don't support lane numbers :%d,set to default %d\n", __FUNCTION__,pConfig->mipiLaneNum,DEFAULT_NUM_LANES);
        pS5K4H8Ctx->IsiSensorMipiInfo.ucMipiLanes = DEFAULT_NUM_LANES;
    }
	
    pConfig->hSensor = ( IsiSensorHandle_t )pS5K4H8Ctx;

    result = HalSetCamConfig( pS5K4H8Ctx->IsiCtx.HalHandle, pS5K4H8Ctx->IsiCtx.HalDevID, false, true, false );
    RETURN_RESULT_IF_DIFFERENT( RET_SUCCESS, result );

    result = HalSetClock( pS5K4H8Ctx->IsiCtx.HalHandle, pS5K4H8Ctx->IsiCtx.HalDevID, 24000000U);
    RETURN_RESULT_IF_DIFFERENT( RET_SUCCESS, result );

    TRACE( S5K4H8_INFO, "%s (exit)\n", __FUNCTION__);

    return ( result );
}



/*****************************************************************************/
/**
 *          S5K4H8_IsiReleaseSensorIss
 *
 * @brief   This function destroys/releases an S5K4H8 sensor instance.
 *
 * @param   handle      S5K4H8 sensor instance handle
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 *
 *****************************************************************************/
static RESULT S5K4H8_IsiReleaseSensorIss
(
    IsiSensorHandle_t handle
)
{
    S5K4H8_Context_t *pS5K4H8Ctx = (S5K4H8_Context_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( S5K4H8_INFO, "%s (enter)\n", __FUNCTION__);

    if ( pS5K4H8Ctx == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    (void)S5K4H8_IsiSensorSetStreamingIss( pS5K4H8Ctx, BOOL_FALSE );
    (void)S5K4H8_IsiSensorSetPowerIss( pS5K4H8Ctx, BOOL_FALSE );

    (void)HalDelRef( pS5K4H8Ctx->IsiCtx.HalHandle );

    MEMSET( pS5K4H8Ctx, 0, sizeof( S5K4H8_Context_t ) );
    free ( pS5K4H8Ctx );

    TRACE( S5K4H8_INFO, "%s (exit)\n", __FUNCTION__);

    return ( result );
}



/*****************************************************************************/
/**
 *          S5K4H8_IsiGetCapsIss
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
static RESULT S5K4H8_IsiGetCapsIssInternal
(
    IsiSensorCaps_t   *pIsiSensorCaps,
    uint32_t  mipi_lanes
)
{
    RESULT result = RET_SUCCESS;
    
    if ( pIsiSensorCaps == NULL )
    {
        return ( RET_NULL_POINTER );
    }
    else
    {
        if (mipi_lanes == SUPPORT_MIPI_FOUR_LANE) { 
            switch (pIsiSensorCaps->Index) 
            {
            	#ifdef FOUR_LANE_700M
                case 0:
                {
                    pIsiSensorCaps->Resolution = ISI_RES_3264_2448P30;
                    break;
                }
                case 1:
                {
                    pIsiSensorCaps->Resolution = ISI_RES_3264_2448P25;
                    break;
                }
                case 2:
                {
                    pIsiSensorCaps->Resolution = ISI_RES_3264_2448P20;
                    break;
                }
                case 3:
                {
                    pIsiSensorCaps->Resolution = ISI_RES_3264_2448P15;
                    break;
                }
                case 4:
                {
                    pIsiSensorCaps->Resolution = ISI_RES_3264_2448P10;
                    break;
                }
                case 5:
                {
                    pIsiSensorCaps->Resolution = ISI_RES_3264_2448P7;
                    break;
                }
                case 6:
                {
                    pIsiSensorCaps->Resolution = ISI_RES_1632_1224P30;
                    break;
                }
                case 7:
                {
                    pIsiSensorCaps->Resolution = ISI_RES_1632_1224P25;
                    break;
                }

                case 8:
                {
                    pIsiSensorCaps->Resolution = ISI_RES_1632_1224P20;
                    break;
                }

                case 9:
                {
                    pIsiSensorCaps->Resolution = ISI_RES_1632_1224P15;
                    break;
                }
                
                case 10:
                {
                    pIsiSensorCaps->Resolution = ISI_RES_1632_1224P10;
                    break;
                }
				#endif

				#ifdef FOUR_LANE_560M
                case 0:
                {
                    pIsiSensorCaps->Resolution = ISI_RES_3264_2448P25;
                    break;
                }
                case 1:
                {
                    pIsiSensorCaps->Resolution = ISI_RES_3264_2448P20;
                    break;
                }
				case 2:
                {
                    pIsiSensorCaps->Resolution = ISI_RES_3264_2448P15;
                    break;
                }
                case 3:
                {
                    pIsiSensorCaps->Resolution = ISI_RES_3264_2448P10;
                    break;
                }
                case 4:
                {
                    pIsiSensorCaps->Resolution = ISI_RES_3264_2448P7;
                    break;
                }
                case 5:
                {
                    pIsiSensorCaps->Resolution = ISI_RES_1632_1224P30;
                    break;
                }
                case 6:
                {
                    pIsiSensorCaps->Resolution = ISI_RES_1632_1224P25;
                    break;
                }

                case 7:
                {
                    pIsiSensorCaps->Resolution = ISI_RES_1632_1224P20;
                    break;
                }

                case 8:
                {
                    pIsiSensorCaps->Resolution = ISI_RES_1632_1224P15;
                    break;
                }
                
                case 9:
                {
                    pIsiSensorCaps->Resolution = ISI_RES_1632_1224P10;
                    break;
                }
				#endif
				
                default:
                {
                    result = RET_OUTOFRANGE;
                    goto end;
                }

            }
        } else if(mipi_lanes == SUPPORT_MIPI_TWO_LANE) {
            switch (pIsiSensorCaps->Index) 
            {
                case 0:
                {
                    pIsiSensorCaps->Resolution = ISI_RES_3264_2448P15;
                    break;
                }
                case 1:
                {
                    pIsiSensorCaps->Resolution = ISI_RES_3264_2448P10;
                    break;
                }
                case 2:
                {
                    pIsiSensorCaps->Resolution = ISI_RES_3264_2448P7;
                    break;
                }
                case 3:
                {
                    pIsiSensorCaps->Resolution = ISI_RES_1632_1224P30;
                    break;
                }
                case 4:
                {
                    pIsiSensorCaps->Resolution = ISI_RES_1632_1224P25;
                    break;
                }

                case 5:
                {
                    pIsiSensorCaps->Resolution = ISI_RES_1632_1224P20;
                    break;
                }

                case 6:
                {
                    pIsiSensorCaps->Resolution = ISI_RES_1632_1224P15;
                    break;
                }
                
                case 7:
                {
                    pIsiSensorCaps->Resolution = ISI_RES_1632_1224P10;
                    break;
                }

                default:
                {
                    result = RET_OUTOFRANGE;
                    goto end;
                }

            }
        }  else if(mipi_lanes == SUPPORT_MIPI_ONE_LANE) {
            switch (pIsiSensorCaps->Index) 
            {
                default:
                {
                    result = RET_OUTOFRANGE;
                    goto end;
                }

            }
        }              
    
        pIsiSensorCaps->BusWidth        = ISI_BUSWIDTH_10BIT;
        pIsiSensorCaps->Mode            = ISI_MODE_MIPI;
        pIsiSensorCaps->FieldSelection  = ISI_FIELDSEL_BOTH;
        pIsiSensorCaps->YCSequence      = ISI_YCSEQ_YCBYCR;
        pIsiSensorCaps->Conv422         = ISI_CONV422_NOCOSITED;
        pIsiSensorCaps->BPat            = ISI_BPAT_GRGRBGBG;//ISI_BPAT_GBGBRGRG;//ISI_BPAT_BGBGGRGR;
        pIsiSensorCaps->HPol            = ISI_HPOL_REFPOS;
        pIsiSensorCaps->VPol            = ISI_VPOL_NEG;
        pIsiSensorCaps->Edge            = ISI_EDGE_FALLING;
        pIsiSensorCaps->Bls             = ISI_BLS_OFF;
        pIsiSensorCaps->Gamma           = ISI_GAMMA_OFF;
        pIsiSensorCaps->CConv           = ISI_CCONV_OFF;
        pIsiSensorCaps->BLC             = ( ISI_BLC_AUTO | ISI_BLC_OFF);
        pIsiSensorCaps->AGC             = ( ISI_AGC_OFF );
        pIsiSensorCaps->AWB             = ( ISI_AWB_OFF );
        pIsiSensorCaps->AEC             = ( ISI_AEC_OFF );
        pIsiSensorCaps->DPCC            = ( ISI_DPCC_AUTO | ISI_DPCC_OFF );

        pIsiSensorCaps->DwnSz           = ISI_DWNSZ_SUBSMPL;
        pIsiSensorCaps->CieProfile      = ( ISI_CIEPROF_A
                                          | ISI_CIEPROF_D50
                                          | ISI_CIEPROF_D65
                                          | ISI_CIEPROF_D75
                                          | ISI_CIEPROF_F2
                                          | ISI_CIEPROF_F11 );
        pIsiSensorCaps->SmiaMode        = ISI_SMIA_OFF;
        pIsiSensorCaps->MipiMode        = ISI_MIPI_MODE_RAW_10; 
        pIsiSensorCaps->AfpsResolutions = ( ISI_AFPS_NOTSUPP );
		pIsiSensorCaps->SensorOutputMode = ISI_SENSOR_OUTPUT_MODE_RAW;
    }
end:
    return result;
}
 
static RESULT S5K4H8_IsiGetCapsIss
(
    IsiSensorHandle_t handle,
    IsiSensorCaps_t   *pIsiSensorCaps
)
{
    S5K4H8_Context_t *pS5K4H8Ctx = (S5K4H8_Context_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( S5K4H8_INFO, "%s (enter)\n", __FUNCTION__);

    if ( pS5K4H8Ctx == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }
    
    result = S5K4H8_IsiGetCapsIssInternal(pIsiSensorCaps,pS5K4H8Ctx->IsiSensorMipiInfo.ucMipiLanes );
    
    TRACE( S5K4H8_INFO, "%s (exit)\n", __FUNCTION__);

    return ( result );
}



/*****************************************************************************/
/**
 *          S5K4H8_g_IsiSensorDefaultConfig
 *
 * @brief   recommended default configuration for application use via call
 *          to IsiGetSensorIss()
 *
 *****************************************************************************/
const IsiSensorCaps_t S5K4H8_g_IsiSensorDefaultConfig =
{
    ISI_BUSWIDTH_10BIT,         // BusWidth
    ISI_MODE_MIPI,              // MIPI
    ISI_FIELDSEL_BOTH,          // FieldSel
    ISI_YCSEQ_YCBYCR,           // YCSeq
    ISI_CONV422_NOCOSITED,      // Conv422
    ISI_BPAT_GRGRBGBG,//ISI_BPAT_GBGBRGRG,//ISI_BPAT_BGBGGRGR,          // BPat
    ISI_HPOL_REFPOS,            // HPol
    ISI_VPOL_NEG,               // VPol
    ISI_EDGE_RISING,            // Edge
    ISI_BLS_OFF,                // Bls
    ISI_GAMMA_OFF,              // Gamma
    ISI_CCONV_OFF,              // CConv
    ISI_RES_1632_1224P30, 
    ISI_DWNSZ_SUBSMPL,          // DwnSz
    ISI_BLC_AUTO,               // BLC
    ISI_AGC_OFF,                // AGC
    ISI_AWB_OFF,                // AWB
    ISI_AEC_OFF,                // AEC
    ISI_DPCC_OFF,               // DPCC
    ISI_CIEPROF_F11,            // CieProfile, this is also used as start profile for AWB (if not altered by menu settings)
    ISI_SMIA_OFF,               // SmiaMode
    ISI_MIPI_MODE_RAW_10,       // MipiMode
    ISI_AFPS_NOTSUPP,           // AfpsResolutions
    ISI_SENSOR_OUTPUT_MODE_RAW,
    0,
};

/*****************************************************************************/
/**
 *          S5K4H8_SetupOutputFormat
 *
 * @brief   Setup of the image sensor considering the given configuration.
 *
 * @param   handle      S5K4H8 sensor instance handle
 * @param   pConfig     pointer to sensor configuration structure
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_NULL_POINTER
 *****************************************************************************/
RESULT S5K4H8_SetupOutputFormat
(
    S5K4H8_Context_t       *pS5K4H8Ctx,
    const IsiSensorConfig_t *pConfig
)
{
    RESULT result = RET_SUCCESS;

    TRACE( S5K4H8_INFO, "%s%s (enter)\n", __FUNCTION__, pS5K4H8Ctx->isAfpsRun?"(AFPS)":"" );

    /* bus-width */
    switch ( pConfig->BusWidth )        /* only ISI_BUSWIDTH_12BIT supported, no configuration needed here */
    {
        case ISI_BUSWIDTH_10BIT:
        {
            break;
        }

        default:
        {
            TRACE( S5K4H8_ERROR, "%s%s: bus width not supported\n", __FUNCTION__, pS5K4H8Ctx->isAfpsRun?"(AFPS)":"" );
            return ( RET_NOTSUPP );
        }
    }

    /* mode */
    switch ( pConfig->Mode )            /* only ISI_MODE_BAYER supported, no configuration needed here */
    {
        case( ISI_MODE_MIPI ):
        {
            break;
        }

        default:
        {
            TRACE( S5K4H8_ERROR, "%s%s: mode not supported\n", __FUNCTION__, pS5K4H8Ctx->isAfpsRun?"(AFPS)":"" );
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
            TRACE( S5K4H8_ERROR, "%s%s: field selection not supported\n", __FUNCTION__, pS5K4H8Ctx->isAfpsRun?"(AFPS)":"" );
            return ( RET_NOTSUPP );
        }
    }

    /* only Bayer mode is supported by S5K4H8 sensor, so the YCSequence parameter is not checked */
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
        {
            break;
        }

        default:
        {
            TRACE( S5K4H8_ERROR, "%s%s: 422 conversion not supported\n", __FUNCTION__, pS5K4H8Ctx->isAfpsRun?"(AFPS)":"" );
            return ( RET_NOTSUPP );
        }
    }

    /* bayer-pattern */
    switch ( pConfig->BPat )            /* only ISI_BPAT_BGBGGRGR supported, no configuration needed */
    {
        case ISI_BPAT_BGBGGRGR:
        {
            break;
        }
        case ISI_BPAT_GBGBRGRG:
        {
            break;
        }
        case ISI_BPAT_GRGRBGBG:
        {
            break;
        }
        case ISI_BPAT_RGRGGBGB:
        {
            break;
        }

        default:
        {
            TRACE( S5K4H8_ERROR, "%s%s: bayer pattern not supported\n", __FUNCTION__, pS5K4H8Ctx->isAfpsRun?"(AFPS)":"" );
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
            TRACE( S5K4H8_ERROR, "%s%s: HPol not supported\n", __FUNCTION__, pS5K4H8Ctx->isAfpsRun?"(AFPS)":"" );
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
            TRACE( S5K4H8_ERROR, "%s%s: VPol not supported\n", __FUNCTION__, pS5K4H8Ctx->isAfpsRun?"(AFPS)":"" );
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
            TRACE( S5K4H8_ERROR, "%s%s:  edge mode not supported\n", __FUNCTION__, pS5K4H8Ctx->isAfpsRun?"(AFPS)":"" );
            return ( RET_NOTSUPP );
        }
    }

    /* gamma */
    switch ( pConfig->Gamma )           /* only ISI_GAMMA_OFF supported, no configuration needed */
    {
        case ISI_GAMMA_OFF:
        {
            break;
        }

        default:
        {
            TRACE( S5K4H8_ERROR, "%s%s:  gamma not supported\n", __FUNCTION__, pS5K4H8Ctx->isAfpsRun?"(AFPS)":"" );
            return ( RET_NOTSUPP );
        }
    }

    /* color conversion */
    switch ( pConfig->CConv )           /* only ISI_CCONV_OFF supported, no configuration needed */
    {
        case ISI_CCONV_OFF:
        {
            break;
        }

        default:
        {
            TRACE( S5K4H8_ERROR, "%s%s: color conversion not supported\n", __FUNCTION__, pS5K4H8Ctx->isAfpsRun?"(AFPS)":"" );
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
            TRACE( S5K4H8_ERROR, "%s%s: SMIA mode not supported\n", __FUNCTION__, pS5K4H8Ctx->isAfpsRun?"(AFPS)":"" );
            return ( RET_NOTSUPP );
        }
    }

    switch ( pConfig->MipiMode )        /* only ISI_MIPI_MODE_RAW_12 supported, no configuration needed */
    {
        case ISI_MIPI_MODE_RAW_10:
        {
            break;
        }

        default:
        {
            TRACE( S5K4H8_ERROR, "%s%s: MIPI mode not supported\n", __FUNCTION__, pS5K4H8Ctx->isAfpsRun?"(AFPS)":"" );
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
            //TRACE( S5K4H8_ERROR, "%s%s: AFPS not supported\n", __FUNCTION__, pS5K4H8Ctx->isAfpsRun?"(AFPS)":"" );
            //return ( RET_NOTSUPP );
        }
    }

    TRACE( S5K4H8_INFO, "%s%s (exit)\n", __FUNCTION__, pS5K4H8Ctx->isAfpsRun?"(AFPS)":"");

    return ( result );
}

int S5K4H8_get_PCLK( S5K4H8_Context_t *pS5K4H8Ctx, int XVCLK)
{
	int pll_multplier,pre_pll_clk_div,vt_sys_clk_div,vt_pix_clk_div;
	uint32_t pclk = 0;
	
	if(pS5K4H8Ctx->IsiSensorMipiInfo.ucMipiLanes == SUPPORT_MIPI_FOUR_LANE){
		S5K4H8_IsiRegReadIssEx(pS5K4H8Ctx,0x0300,&vt_pix_clk_div,2);
		S5K4H8_IsiRegReadIssEx(pS5K4H8Ctx,0x0302,&vt_sys_clk_div,2);
		S5K4H8_IsiRegReadIssEx(pS5K4H8Ctx,0x0304,&pre_pll_clk_div,2);
		S5K4H8_IsiRegReadIssEx(pS5K4H8Ctx,0x0306,&pll_multplier,2);
		
		pclk = 2*XVCLK*pll_multplier/(pre_pll_clk_div*vt_pix_clk_div*vt_sys_clk_div);
		//pclk = 4*XVCLK*pll_multplier/(pre_pll_clk_div*vt_pix_clk_div*vt_sys_clk_div);
		
		return pclk*1000000; 
		
	}else if(pS5K4H8Ctx->IsiSensorMipiInfo.ucMipiLanes == SUPPORT_MIPI_TWO_LANE)
	{
		S5K4H8_IsiRegReadIssEx(pS5K4H8Ctx,0x0300,&vt_pix_clk_div,2);
		S5K4H8_IsiRegReadIssEx(pS5K4H8Ctx,0x0302,&vt_sys_clk_div,2);
		S5K4H8_IsiRegReadIssEx(pS5K4H8Ctx,0x0304,&pre_pll_clk_div,2);
		S5K4H8_IsiRegReadIssEx(pS5K4H8Ctx,0x0306,&pll_multplier,2);

		pclk = 2*XVCLK*pll_multplier/(pre_pll_clk_div*vt_pix_clk_div*vt_sys_clk_div);
		return pclk*1000000;
	}
	else	
		return 300000000;
}

/*****************************************************************************/
/**
 *          S5K4H8_SetupOutputWindow
 *
 * @brief   Setup of the image sensor considering the given configuration.
 *
 * @param   handle      S5K4H8 sensor instance handle
 * @param   pConfig     pointer to sensor configuration structure
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_NULL_POINTER
 * hkw fix
 *****************************************************************************/

static RESULT S5K4H8_SetupOutputWindowInternal
(
    S5K4H8_Context_t        *pS5K4H8Ctx,
    const IsiSensorConfig_t *pConfig,
    bool_t set2Sensor,
    bool_t res_no_chg
)
{
    RESULT result     = RET_SUCCESS;
    uint16_t usFrameLengthLines = 0;
    uint16_t usLineLengthPck    = 0;
	uint16_t usTimeHts;
	uint16_t usTimeVts;
    float    rVtPixClkFreq      = 0.0f;
    int xclk = 24;
    
	TRACE( S5K4H8_INFO, "%s (enter)\n", __FUNCTION__);
	
	if(pS5K4H8Ctx->IsiSensorMipiInfo.ucMipiLanes == SUPPORT_MIPI_ONE_LANE){
	} else if(pS5K4H8Ctx->IsiSensorMipiInfo.ucMipiLanes == SUPPORT_MIPI_TWO_LANE){
    	pS5K4H8Ctx->IsiSensorMipiInfo.ulMipiFreq = 720;

        switch ( pConfig->Resolution )
        {
            case ISI_RES_1632_1224P30:
            case ISI_RES_1632_1224P25:
            case ISI_RES_1632_1224P20:
            case ISI_RES_1632_1224P15:
            case ISI_RES_1632_1224P10:            
            {
                if (set2Sensor == BOOL_TRUE) {                    
                    if (res_no_chg == BOOL_FALSE) {
						result = IsiRegDefaultsApply( pS5K4H8Ctx, S5K4H8_g_1632x1224_twolane);
                    }

                    if (pConfig->Resolution == ISI_RES_1632_1224P30) {
                        result = IsiRegDefaultsApply( pS5K4H8Ctx, S5K4H8_g_1632x1224P30_twolane_fpschg);
                    } else if (pConfig->Resolution == ISI_RES_1632_1224P25) {
                        result = IsiRegDefaultsApply( pS5K4H8Ctx, S5K4H8_g_1632x1224P25_twolane_fpschg);
                    } else if (pConfig->Resolution == ISI_RES_1632_1224P20) {
                        result = IsiRegDefaultsApply( pS5K4H8Ctx, S5K4H8_g_1632x1224P20_twolane_fpschg);
                    } else if (pConfig->Resolution == ISI_RES_1632_1224P15) {
                        result = IsiRegDefaultsApply( pS5K4H8Ctx, S5K4H8_g_1632x1224P15_twolane_fpschg);
                    } else if (pConfig->Resolution == ISI_RES_1632_1224P10) {
                        result = IsiRegDefaultsApply( pS5K4H8Ctx, S5K4H8_g_1632x1224P10_twolane_fpschg);
                    }
        		}
    			usTimeHts = 0x0ea0; 
                if (pConfig->Resolution == ISI_RES_1632_1224P30) {
                    usTimeVts = 0x09c2;
                } else if (pConfig->Resolution == ISI_RES_1632_1224P25) {
                    usTimeVts = 0x05d7;
                } else if (pConfig->Resolution == ISI_RES_1632_1224P20) {
                    usTimeVts = 0x074d;
                } else if (pConfig->Resolution == ISI_RES_1632_1224P15) {
                    usTimeVts = 0x09bc;
                } else if (pConfig->Resolution == ISI_RES_1632_1224P10) {
                    usTimeVts = 0x0e9a;
                }
                
    		    /* sleep a while, that sensor can take over new default values */
    		    osSleep( 10 );
    			break;
            }
            case ISI_RES_3264_2448P7:
            case ISI_RES_3264_2448P10:
            case ISI_RES_3264_2448P15:
            {
                if (set2Sensor == BOOL_TRUE) {
                    if (res_no_chg == BOOL_FALSE) {
						result = IsiRegDefaultsApply( pS5K4H8Ctx, S5K4H8_g_3264x2448_twolane);
        		    }
					if (pConfig->Resolution == ISI_RES_3264_2448P15) {                        
                        result = IsiRegDefaultsApply( pS5K4H8Ctx, S5K4H8_g_3264x2448P15_twolane_fpschg);
                    } else if (pConfig->Resolution == ISI_RES_3264_2448P10) {                        
                       result = IsiRegDefaultsApply( pS5K4H8Ctx, S5K4H8_g_3264x2448P10_twolane_fpschg);
                    } else if (pConfig->Resolution == ISI_RES_3264_2448P7) {
                       result = IsiRegDefaultsApply( pS5K4H8Ctx, S5K4H8_g_3264x2448P7_twolane_fpschg);
                    }
        		}

    			usTimeHts = 0x0ea0;                
				if (pConfig->Resolution == ISI_RES_3264_2448P15) {                        
                    usTimeVts = 0x09c2;
                } else if (pConfig->Resolution == ISI_RES_3264_2448P10) {                        
                    usTimeVts = 0x0bb5;
                } else if (pConfig->Resolution == ISI_RES_3264_2448P7) {
                    usTimeVts = 0x0ea3;
                }
    		    /* sleep a while, that sensor can take over new default values */
    		    osSleep( 10 );
    			break;
            }
        }

    } else if(pS5K4H8Ctx->IsiSensorMipiInfo.ucMipiLanes == SUPPORT_MIPI_FOUR_LANE) {
		#ifdef FOUR_LANE_700M
		
		#ifdef FOUR_LANE_700M_to_672M
		pS5K4H8Ctx->IsiSensorMipiInfo.ulMipiFreq = 672;
		#else
		pS5K4H8Ctx->IsiSensorMipiInfo.ulMipiFreq = 720;
		#endif

        switch ( pConfig->Resolution )
        {
            case ISI_RES_1632_1224P30:
            case ISI_RES_1632_1224P25:
            case ISI_RES_1632_1224P20:
            case ISI_RES_1632_1224P15:
            case ISI_RES_1632_1224P10:            
            {
                if (set2Sensor == BOOL_TRUE) {                    
                    if (res_no_chg == BOOL_FALSE) {
						result = IsiRegDefaultsApply( pS5K4H8Ctx, S5K4H8_g_1632x1224_fourlane);
                    }

                    if (pConfig->Resolution == ISI_RES_1632_1224P30) {                        
                        result = IsiRegDefaultsApply( pS5K4H8Ctx, S5K4H8_g_1632x1224P30_fourlane_fpschg);
                    } else if (pConfig->Resolution == ISI_RES_1632_1224P25) {
                        result = IsiRegDefaultsApply( pS5K4H8Ctx, S5K4H8_g_1632x1224P25_fourlane_fpschg);
                    } else if (pConfig->Resolution == ISI_RES_1632_1224P20) {
                        result = IsiRegDefaultsApply( pS5K4H8Ctx, S5K4H8_g_1632x1224P20_fourlane_fpschg);
                    } else if (pConfig->Resolution == ISI_RES_1632_1224P15) {
                        result = IsiRegDefaultsApply( pS5K4H8Ctx, S5K4H8_g_1632x1224P15_fourlane_fpschg);
                    } else if (pConfig->Resolution == ISI_RES_1632_1224P10) {
                        result = IsiRegDefaultsApply( pS5K4H8Ctx, S5K4H8_g_1632x1224P10_fourlane_fpschg);
                    }
        		}
    			usTimeHts = 0x0ea0; 
                if (pConfig->Resolution == ISI_RES_1632_1224P30) {
                    usTimeVts = 0x09c2;
                } else if (pConfig->Resolution == ISI_RES_1632_1224P25) {
                    usTimeVts = 0x0bb5;
                } else if (pConfig->Resolution == ISI_RES_1632_1224P20) {
                    usTimeVts = 0x0ea3;
                } else if (pConfig->Resolution == ISI_RES_1632_1224P15) {
                    usTimeVts = 0x1384;
                } else if (pConfig->Resolution == ISI_RES_1632_1224P10) {
                    usTimeVts = 0x1d46;
                }
                
    		    /* sleep a while, that sensor can take over new default values */
    		    osSleep( 10 );
    			break;
            }
            case ISI_RES_3264_2448P7:
            case ISI_RES_3264_2448P10:
            case ISI_RES_3264_2448P15:
            case ISI_RES_3264_2448P20:
            case ISI_RES_3264_2448P25:
            case ISI_RES_3264_2448P30:
            {
                if (set2Sensor == BOOL_TRUE) {
                    if (res_no_chg == BOOL_FALSE) {
						result = IsiRegDefaultsApply( pS5K4H8Ctx, S5K4H8_g_3264x2448_fourlane);
        		    }

                    if (pConfig->Resolution == ISI_RES_3264_2448P30) {                        
                        result = IsiRegDefaultsApply( pS5K4H8Ctx, S5K4H8_g_3264x2448P30_fourlane_fpschg);
                    } else if (pConfig->Resolution == ISI_RES_3264_2448P25) {                        
                        result = IsiRegDefaultsApply( pS5K4H8Ctx, S5K4H8_g_3264x2448P25_fourlane_fpschg);
                    } else if (pConfig->Resolution == ISI_RES_3264_2448P20) {                        
                        result = IsiRegDefaultsApply( pS5K4H8Ctx, S5K4H8_g_3264x2448P20_fourlane_fpschg);
                    } else if (pConfig->Resolution == ISI_RES_3264_2448P15) {                        
                        result = IsiRegDefaultsApply( pS5K4H8Ctx, S5K4H8_g_3264x2448P15_fourlane_fpschg);
                    } else if (pConfig->Resolution == ISI_RES_3264_2448P10) {                        
                        result = IsiRegDefaultsApply( pS5K4H8Ctx, S5K4H8_g_3264x2448P10_fourlane_fpschg);
                    } else if (pConfig->Resolution == ISI_RES_3264_2448P7) {
                        result = IsiRegDefaultsApply( pS5K4H8Ctx, S5K4H8_g_3264x2448P7_fourlane_fpschg);
                    }
        		}

    			usTimeHts = 0x0ea0;                
                if (pConfig->Resolution == ISI_RES_3264_2448P30) {                        
                    usTimeVts = 0x09c2;
                } else if (pConfig->Resolution == ISI_RES_3264_2448P25) {                        
                    usTimeVts = 0x0bb5;
                } else if (pConfig->Resolution == ISI_RES_3264_2448P20) {                        
                    usTimeVts = 0x0ea3;
                } else if (pConfig->Resolution == ISI_RES_3264_2448P15) {                        
                    usTimeVts = 0x1384;
                } else if (pConfig->Resolution == ISI_RES_3264_2448P10) {                        
                    usTimeVts = 0x1d46;
                } else if (pConfig->Resolution == ISI_RES_3264_2448P7) {
                    usTimeVts = 0x29d1;
                }
    		    /* sleep a while, that sensor can take over new default values */
    		    osSleep( 10 );
    			break;
            }
        }
		#endif
		#ifdef FOUR_LANE_560M
		pS5K4H8Ctx->IsiSensorMipiInfo.ulMipiFreq = 560;

        switch ( pConfig->Resolution )
        {
            case ISI_RES_1632_1224P30:
            case ISI_RES_1632_1224P25:
            case ISI_RES_1632_1224P20:
            case ISI_RES_1632_1224P15:
            case ISI_RES_1632_1224P10:            
            {
                if (set2Sensor == BOOL_TRUE) {                    
                    if (res_no_chg == BOOL_FALSE) {
						result = IsiRegDefaultsApply( pS5K4H8Ctx, S5K4H8_g_1632x1224_fourlane_560M);
                    }

                    if (pConfig->Resolution == ISI_RES_1632_1224P30) {                        
                        result = IsiRegDefaultsApply( pS5K4H8Ctx, S5K4H8_g_1632x1224P30_fourlane_560M_fpschg);
                    } else if (pConfig->Resolution == ISI_RES_1632_1224P25) {
                        result = IsiRegDefaultsApply( pS5K4H8Ctx, S5K4H8_g_1632x1224P25_fourlane_560M_fpschg);
                    } else if (pConfig->Resolution == ISI_RES_1632_1224P20) {
                        result = IsiRegDefaultsApply( pS5K4H8Ctx, S5K4H8_g_1632x1224P20_fourlane_560M_fpschg);
                    } else if (pConfig->Resolution == ISI_RES_1632_1224P15) {
                        result = IsiRegDefaultsApply( pS5K4H8Ctx, S5K4H8_g_1632x1224P15_fourlane_560M_fpschg);
                    } else if (pConfig->Resolution == ISI_RES_1632_1224P10) {
                        result = IsiRegDefaultsApply( pS5K4H8Ctx, S5K4H8_g_1632x1224P10_fourlane_560M_fpschg);
                    }
        		}
    			usTimeHts = 0x0ea0; 
                if (pConfig->Resolution == ISI_RES_1632_1224P30) {
                    usTimeVts = 0x04E0;
                } else if (pConfig->Resolution == ISI_RES_1632_1224P25) {
                    usTimeVts = 0x05d9;
                } else if (pConfig->Resolution == ISI_RES_1632_1224P20) {
                    usTimeVts = 0x0750;
                } else if (pConfig->Resolution == ISI_RES_1632_1224P15) {
                    usTimeVts = 0x09c0;
                } else if (pConfig->Resolution == ISI_RES_1632_1224P10) {
                    usTimeVts = 0x0ea0;
                }
                
    		    /* sleep a while, that sensor can take over new default values */
    		    osSleep( 10 );
    			break;
            }
            case ISI_RES_3264_2448P7:
            case ISI_RES_3264_2448P10:
            case ISI_RES_3264_2448P15:
            case ISI_RES_3264_2448P20:
            case ISI_RES_3264_2448P25:
            {
                if (set2Sensor == BOOL_TRUE) {
                    if (res_no_chg == BOOL_FALSE) {
						result = IsiRegDefaultsApply( pS5K4H8Ctx, S5K4H8_g_3264x2448_fourlane_560M);
        		    }
					if (pConfig->Resolution == ISI_RES_3264_2448P25) {                        
                        result = IsiRegDefaultsApply( pS5K4H8Ctx, S5K4H8_g_3264x2448P25_fourlane_560M_fpschg);
                    } else if (pConfig->Resolution == ISI_RES_3264_2448P20) {                        
                        result = IsiRegDefaultsApply( pS5K4H8Ctx, S5K4H8_g_3264x2448P20_fourlane_560M_fpschg);
                    } else if (pConfig->Resolution == ISI_RES_3264_2448P15) {                        
                        result = IsiRegDefaultsApply( pS5K4H8Ctx, S5K4H8_g_3264x2448P15_fourlane_560M_fpschg);
                    } else if (pConfig->Resolution == ISI_RES_3264_2448P10) {                        
                        result = IsiRegDefaultsApply( pS5K4H8Ctx, S5K4H8_g_3264x2448P10_fourlane_560M_fpschg);
                    } else if (pConfig->Resolution == ISI_RES_3264_2448P7) {
                        result = IsiRegDefaultsApply( pS5K4H8Ctx, S5K4H8_g_3264x2448P7_fourlane_560M_fpschg);
                    }
        		}

    			usTimeHts = 0x0ea0;                
                if (pConfig->Resolution == ISI_RES_3264_2448P25) {                        
                    usTimeVts = 0x09BC;
                } else if (pConfig->Resolution == ISI_RES_3264_2448P20) {                        
                    usTimeVts = 0x0bae;
                } else if (pConfig->Resolution == ISI_RES_3264_2448P15) {                        
                    usTimeVts = 0x0f93;
                } else if (pConfig->Resolution == ISI_RES_3264_2448P10) {                        
                    usTimeVts = 0x175c;
                } else if (pConfig->Resolution == ISI_RES_3264_2448P7) {
                    usTimeVts = 0x2160;
                }
    		    /* sleep a while, that sensor can take over new default values */
    		    osSleep( 10 );
    			break;
            }
        }
		#endif
    }

/* 2.) write default values derived from datasheet and evaluation kit (static setup altered by dynamic setup further below) */
    
	usLineLengthPck = usTimeHts;
    usFrameLengthLines = usTimeVts;
	rVtPixClkFreq = S5K4H8_get_PCLK(pS5K4H8Ctx, xclk);

    // store frame timing for later use in AEC module
    pS5K4H8Ctx->VtPixClkFreq     = rVtPixClkFreq;
    pS5K4H8Ctx->LineLengthPck    = usLineLengthPck;
    pS5K4H8Ctx->FrameLengthLines = usFrameLengthLines;

    TRACE( S5K4H8_INFO, "%s  (exit): Resolution %dx%d@%dfps  MIPI %dlanes  res_no_chg: %d   rVtPixClkFreq: %f \n", __FUNCTION__,
                        ISI_RES_W_GET(pConfig->Resolution),ISI_RES_H_GET(pConfig->Resolution),
                        ISI_FPS_GET(pConfig->Resolution),
                        pS5K4H8Ctx->IsiSensorMipiInfo.ucMipiLanes,
                        res_no_chg,rVtPixClkFreq);

    return ( result );
}


/*****************************************************************************/
/**
 *          S5K4H8_SetupImageControl
 *
 * @brief   Sets the image control functions (BLC, AGC, AWB, AEC, DPCC ...)
 *
 * @param   handle      S5K4H8 sensor instance handle
 * @param   pConfig     pointer to sensor configuration structure
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_NULL_POINTER
 *****************************************************************************/
RESULT S5K4H8_SetupImageControl
(
    S5K4H8_Context_t        *pS5K4H8Ctx,
    const IsiSensorConfig_t *pConfig
)
{
    RESULT result = RET_SUCCESS;

    uint32_t RegValue = 0U;

    TRACE( S5K4H8_INFO, "%s (enter)\n", __FUNCTION__);

    switch ( pConfig->Bls )      /* only ISI_BLS_OFF supported, no configuration needed */
    {
        case ISI_BLS_OFF:
        {
            break;
        }

        default:
        {
            TRACE( S5K4H8_ERROR, "%s: Black level not supported\n", __FUNCTION__ );
            return ( RET_NOTSUPP );
        }
    }

    /* black level compensation */
    switch ( pConfig->BLC )
    {
        case ISI_BLC_OFF:
        {
            /* turn off black level correction (clear bit 0) */
            //result = S5K4H8_IsiRegReadIss(  pS5K4H8Ctx, S5K4H8_BLC_CTRL00, &RegValue );
            //result = S5K4H8_IsiRegWriteIss( pS5K4H8Ctx, S5K4H8_BLC_CTRL00, RegValue & 0x7F);
            break;
        }

        case ISI_BLC_AUTO:
        {
            /* turn on black level correction (set bit 0)
             * (0x331E[7] is assumed to be already setup to 'auto' by static configration) */
            //result = S5K4H8_IsiRegReadIss(  pS5K4H8Ctx, S5K4H8_BLC_CTRL00, &RegValue );
            //result = S5K4H8_IsiRegWriteIss( pS5K4H8Ctx, S5K4H8_BLC_CTRL00, RegValue | 0x80 );
            break;
        }

        default:
        {
            TRACE( S5K4H8_ERROR, "%s: BLC not supported\n", __FUNCTION__ );
            return ( RET_NOTSUPP );
        }
    }

    /* automatic gain control */
    switch ( pConfig->AGC )
    {
        case ISI_AGC_OFF:
        {
            break;
        }

        default:
        {
            TRACE( S5K4H8_ERROR, "%s: AGC not supported\n", __FUNCTION__ );
            return ( RET_NOTSUPP );
        }
    }

    /* automatic white balance */
    switch( pConfig->AWB )
    {
        case ISI_AWB_OFF:
        {
            break;
        }

        default:
        {
            TRACE( S5K4H8_ERROR, "%s: AWB not supported\n", __FUNCTION__ );
            return ( RET_NOTSUPP );
        }
    }

    switch( pConfig->AEC )
    {
        case ISI_AEC_OFF:
        {
            break;
        }

        default:
        {
            TRACE( S5K4H8_ERROR, "%s: AEC not supported\n", __FUNCTION__ );
            return ( RET_NOTSUPP );
        }
    }


    switch( pConfig->DPCC )
    {
        case ISI_DPCC_OFF:
        {
            break;
        }

        case ISI_DPCC_AUTO:
        {
            break;
        }

        default:
        {
            TRACE( S5K4H8_ERROR, "%s: DPCC not supported\n", __FUNCTION__ );
            return ( RET_NOTSUPP );
        }
    }

    return ( result );
}

static RESULT S5K4H8_SetupOutputWindow
(
    S5K4H8_Context_t        *pS5K4H8Ctx,
    const IsiSensorConfig_t *pConfig    
)
{
    bool_t res_no_chg;

    if ((ISI_RES_W_GET(pConfig->Resolution)==ISI_RES_W_GET(pS5K4H8Ctx->Config.Resolution)) && 
        (ISI_RES_W_GET(pConfig->Resolution)==ISI_RES_W_GET(pS5K4H8Ctx->Config.Resolution))) {
        res_no_chg = BOOL_TRUE;
        
    } else {
        res_no_chg = BOOL_FALSE;
    }

    return S5K4H8_SetupOutputWindowInternal(pS5K4H8Ctx,pConfig,BOOL_TRUE, BOOL_FALSE);
}

/*****************************************************************************/
/**
 *          S5K4H8_AecSetModeParameters
 *
 * @brief   This function fills in the correct parameters in S5K4H8-Instances
 *          according to AEC mode selection in IsiSensorConfig_t.
 *
 * @note    It is assumed that IsiSetupOutputWindow has been called before
 *          to fill in correct values in instance structure.
 *
 * @param   handle      S5K4H8 context
 * @param   pConfig     pointer to sensor configuration structure
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_NULL_POINTER
 *****************************************************************************/
static RESULT S5K4H8_AecSetModeParameters
(
    S5K4H8_Context_t       *pS5K4H8Ctx,
    const IsiSensorConfig_t *pConfig
)
{
    RESULT result = RET_SUCCESS;

    TRACE( S5K4H8_INFO, "%s%s (enter)  Res: 0x%x  0x%x\n", __FUNCTION__, pS5K4H8Ctx->isAfpsRun?"(AFPS)":"",
        pS5K4H8Ctx->Config.Resolution, pConfig->Resolution);

    if ( (pS5K4H8Ctx->VtPixClkFreq == 0.0f) )
    {
        TRACE( S5K4H8_ERROR, "%s%s: Division by zero!\n", __FUNCTION__  );
        return ( RET_OUTOFRANGE );
    }

    //as of mail from Omnivision FAE the limit is VTS - 6 (above that we observed a frame
    //exposed way too dark from time to time)
    // (formula is usually MaxIntTime = (CoarseMax * LineLength + FineMax) / Clk
    //                     MinIntTime = (CoarseMin * LineLength + FineMin) / Clk )
    pS5K4H8Ctx->AecMaxIntegrationTime = ( ((float)(pS5K4H8Ctx->FrameLengthLines - 4)) * ((float)pS5K4H8Ctx->LineLengthPck)) / pS5K4H8Ctx->VtPixClkFreq;
    pS5K4H8Ctx->AecMinIntegrationTime = 0.0001f;

    TRACE( S5K4H8_INFO, "%s%s: AecMaxIntegrationTime = %f \n", __FUNCTION__, pS5K4H8Ctx->isAfpsRun?"(AFPS)":"", pS5K4H8Ctx->AecMaxIntegrationTime  );

    pS5K4H8Ctx->AecMaxGain = S5K4H8_MAX_GAIN_AEC;
    pS5K4H8Ctx->AecMinGain = 1.0f;

    //_smallest_ increment the sensor/driver can handle (e.g. used for sliders in the application)
    pS5K4H8Ctx->AecIntegrationTimeIncrement = ((float)pS5K4H8Ctx->LineLengthPck) / pS5K4H8Ctx->VtPixClkFreq;
    pS5K4H8Ctx->AecGainIncrement = S5K4H8_MIN_GAIN_STEP;

    //reflects the state of the sensor registers, must equal default settings
    pS5K4H8Ctx->AecCurGain               = pS5K4H8Ctx->AecMinGain;
    pS5K4H8Ctx->AecCurIntegrationTime    = 0.0f;
    pS5K4H8Ctx->OldCoarseIntegrationTime = 0;
    pS5K4H8Ctx->OldFineIntegrationTime   = 0;
    //pS5K4H8Ctx->GroupHold                = true; //must be true (for unknown reason) to correctly set gain the first time

    TRACE( S5K4H8_INFO, "%s%s (exit)\n", __FUNCTION__, pS5K4H8Ctx->isAfpsRun?"(AFPS)":"");

    return ( result );
}

/*****************************************************************************/
/**
 *          S5K4H8_IsiSetupSensorIss
 *
 * @brief   Setup of the image sensor considering the given configuration.
 *
 * @param   handle      S5K4H8 sensor instance handle
 * @param   pConfig     pointer to sensor configuration structure
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
static RESULT S5K4H8_IsiSetupSensorIss
(
    IsiSensorHandle_t       handle,
    const IsiSensorConfig_t *pConfig
)
{
    S5K4H8_Context_t *pS5K4H8Ctx = (S5K4H8_Context_t *)handle;

    RESULT result = RET_SUCCESS;

    uint32_t RegValue = 0;

    TRACE( S5K4H8_INFO, "%s (enter)\n", __FUNCTION__);

    if ( pS5K4H8Ctx == NULL )
    {
        TRACE( S5K4H8_ERROR, "%s: Invalid sensor handle (NULL pointer detected)\n", __FUNCTION__ );
        return ( RET_WRONG_HANDLE );
    }

    if ( pConfig == NULL )
    {
        TRACE( S5K4H8_ERROR, "%s: Invalid configuration (NULL pointer detected)\n", __FUNCTION__ );
        return ( RET_NULL_POINTER );
    }

    if ( pS5K4H8Ctx->Streaming != BOOL_FALSE )
    {
        return RET_WRONG_STATE;
    }

    MEMCPY( &pS5K4H8Ctx->Config, pConfig, sizeof( IsiSensorConfig_t ) );

    /* 1.) SW reset of image sensor (via I2C register interface)  be careful, bits 6..0 are reserved, reset bit is not sticky */
    result = S5K4H8_IsiRegWriteIssEx ( pS5K4H8Ctx, S5K4H8_SOFTWARE_RST, S5K4H8_SOFTWARE_RST_VALUE, 1);
    RETURN_RESULT_IF_DIFFERENT( RET_SUCCESS, result );

    osSleep( 10 );

    TRACE( S5K4H8_DEBUG, "%s: S5K4H8 System-Reset executed\n", __FUNCTION__);

    // disable streaming during sensor setup
    // (this seems not to be necessary, however Omnivision is doing it in their
    // reference settings, simply overwrite upper bits since setup takes care
    // of 'em later on anyway)
    result = S5K4H8_IsiRegWriteIssEx( pS5K4H8Ctx, S5K4H8_MODE_SELECT, S5K4H8_MODE_SELECT_OFF, 1);
    if ( result != RET_SUCCESS )
    {
        TRACE( S5K4H8_ERROR, "%s: Can't write S5K4H8 Image System Register (disable streaming failed)\n", __FUNCTION__ );
        return ( result );
    }

    /* 2.) write default values derived from datasheet and evaluation kit (static setup altered by dynamic setup further below) */
    if(pS5K4H8Ctx->IsiSensorMipiInfo.ucMipiLanes == SUPPORT_MIPI_FOUR_LANE){
        result = IsiRegDefaultsApply( pS5K4H8Ctx, S5K4H8_g_aRegDescription_fourlane);
    }else if(pS5K4H8Ctx->IsiSensorMipiInfo.ucMipiLanes == SUPPORT_MIPI_TWO_LANE){
    }
    if ( result != RET_SUCCESS )
    {
        return ( result );
    }

    /* sleep a while, that sensor can take over new default values */
    osSleep( 10 );


    /* 3.) verify default values to make sure everything has been written correctly as expected */
	#if 0
	result = IsiRegDefaultsVerify( pS5K4H8Ctx, S5K4H8_g_aRegDescription );
    if ( result != RET_SUCCESS )
    {
        return ( result );
    }
	#endif

    /* 4.) setup output format (RAW10|RAW12) */
    result = S5K4H8_SetupOutputFormat( pS5K4H8Ctx, pConfig );
    if ( result != RET_SUCCESS )
    {
        TRACE( S5K4H8_ERROR, "%s: SetupOutputFormat failed.\n", __FUNCTION__);
        return ( result );
    }

    /* 5.) setup output window */
    result = S5K4H8_SetupOutputWindow( pS5K4H8Ctx, pConfig );
    if ( result != RET_SUCCESS )
    {
        TRACE( S5K4H8_ERROR, "%s: SetupOutputWindow failed.\n", __FUNCTION__);
        return ( result );
    }

    result = S5K4H8_SetupImageControl( pS5K4H8Ctx, pConfig );
    if ( result != RET_SUCCESS )
    {
        TRACE( S5K4H8_ERROR, "%s: SetupImageControl failed.\n", __FUNCTION__);
        return ( result );
    }

    result = S5K4H8_AecSetModeParameters( pS5K4H8Ctx, pConfig );
    if ( result != RET_SUCCESS )
    {
        TRACE( S5K4H8_ERROR, "%s: AecSetModeParameters failed.\n", __FUNCTION__);
        return ( result );
    }
    if (result == RET_SUCCESS)
    {
        pS5K4H8Ctx->Configured = BOOL_TRUE;
    }
    #if 1//s5k4h8 need below operation,I don't know yet.
    result = S5K4H8_IsiRegWriteIssEx( pS5K4H8Ctx, S5K4H8_MODE_SELECT, S5K4H8_MODE_SELECT_ON, 1);
    if ( result != RET_SUCCESS )
    {
        TRACE( S5K4H8_ERROR, "%s: Can't write S5K4H8 Image System Register (disable streaming failed)\n", __FUNCTION__ );
        return ( result );
    }

    result = S5K4H8_IsiRegWriteIssEx( pS5K4H8Ctx, S5K4H8_MODE_SELECT, S5K4H8_MODE_SELECT_OFF, 1);
    if ( result != RET_SUCCESS )
    {
        TRACE( S5K4H8_ERROR, "%s: Can't write S5K4H8 Image System Register (disable streaming failed)\n", __FUNCTION__ );
        return ( result );
    }
    osSleep( 5 );
   #endif

    if((g_otp_info.flag==0x10) || (g_otp_info.flag==0xf1)){
        TRACE( S5K4H8_NOTICE0, "%s: apply OTP info !!\n", __FUNCTION__);
		apply_otp_data(handle,&g_otp_info);
    }

    TRACE( S5K4H8_INFO, "%s: (exit)\n", __FUNCTION__);

    return ( result );
}



/*****************************************************************************/
/**
 *          S5K4H8_IsiChangeSensorResolutionIss
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
 *****************************************************************************/
static RESULT S5K4H8_IsiChangeSensorResolutionIss
(
    IsiSensorHandle_t   handle,
    uint32_t            Resolution,
    uint8_t             *pNumberOfFramesToSkip
)
{
    S5K4H8_Context_t *pS5K4H8Ctx = (S5K4H8_Context_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( S5K4H8_INFO, "%s (enter)  Resolution: %dx%d@%dfps\n", __FUNCTION__,
        ISI_RES_W_GET(Resolution),ISI_RES_H_GET(Resolution), ISI_FPS_GET(Resolution));

    if ( pS5K4H8Ctx == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    if (pNumberOfFramesToSkip == NULL)
    {
        return ( RET_NULL_POINTER );
    }

    if ( (pS5K4H8Ctx->Configured != BOOL_TRUE) )
    {
        return RET_WRONG_STATE;
    }

    IsiSensorCaps_t Caps;
    
    Caps.Index = 0;
    Caps.Resolution = 0;
    while (S5K4H8_IsiGetCapsIss( handle, &Caps) == RET_SUCCESS) {
        if (Resolution == Caps.Resolution) {            
            break;
        }
        Caps.Index++;
    }

    if (Resolution != Caps.Resolution) {
        return RET_OUTOFRANGE;
    }

    if ( Resolution == pS5K4H8Ctx->Config.Resolution )
    {
        // well, no need to worry
        *pNumberOfFramesToSkip = 0;
    }
    else
    {
        // change resolution
        char *szResName = NULL;
        bool_t res_no_chg;

        if (!((ISI_RES_W_GET(Resolution)==ISI_RES_W_GET(pS5K4H8Ctx->Config.Resolution)) && 
            (ISI_RES_W_GET(Resolution)==ISI_RES_W_GET(pS5K4H8Ctx->Config.Resolution))) ) {

            if (pS5K4H8Ctx->Streaming != BOOL_FALSE) {
                TRACE( S5K4H8_ERROR, "%s: Sensor is streaming, Change resolution is not allow\n",__FUNCTION__);
                return RET_WRONG_STATE;
            }
            res_no_chg = BOOL_FALSE;
        } else {
            res_no_chg = BOOL_TRUE;
        }
        
        result = IsiGetResolutionName( Resolution, &szResName );
        TRACE( S5K4H8_DEBUG, "%s: NewRes=0x%08x (%s)\n", __FUNCTION__, Resolution, szResName);

        // update resolution in copy of config in context        
        pS5K4H8Ctx->Config.Resolution = Resolution;

        // tell sensor about that
        result = S5K4H8_SetupOutputWindowInternal( pS5K4H8Ctx, &pS5K4H8Ctx->Config, BOOL_TRUE, res_no_chg);
        if ( result != RET_SUCCESS )
        {
            TRACE( S5K4H8_ERROR, "%s: SetupOutputWindow failed.\n", __FUNCTION__);
            return ( result );
        }

        // remember old exposure values
        float OldGain = pS5K4H8Ctx->AecCurGain;
        float OldIntegrationTime = pS5K4H8Ctx->AecCurIntegrationTime;

        // update limits & stuff (reset current & old settings)
        result = S5K4H8_AecSetModeParameters( pS5K4H8Ctx, &pS5K4H8Ctx->Config );
        if ( result != RET_SUCCESS )
        {
            TRACE( S5K4H8_ERROR, "%s: AecSetModeParameters failed.\n", __FUNCTION__);
            return ( result );
        }

        // restore old exposure values (at least within new exposure values' limits)
        uint8_t NumberOfFramesToSkip;
        float   DummySetGain;
        float   DummySetIntegrationTime;

        result = S5K4H8_IsiExposureControlIss( handle, OldGain, OldIntegrationTime, &NumberOfFramesToSkip, &DummySetGain, &DummySetIntegrationTime );
        if ( result != RET_SUCCESS )
        {
            TRACE( S5K4H8_ERROR, "%s: S5K4H8_IsiExposureControlIss failed.\n", __FUNCTION__);
            return ( result );
        }

        // return number of frames that aren't exposed correctly
        if (res_no_chg == BOOL_TRUE)
            *pNumberOfFramesToSkip = 0;
        else 
            *pNumberOfFramesToSkip = NumberOfFramesToSkip + 1;
        
    }

    TRACE( S5K4H8_INFO, "%s (exit)  result: 0x%x   pNumberOfFramesToSkip: %d \n", __FUNCTION__, result,
        *pNumberOfFramesToSkip);

    return ( result );
}

/*****************************************************************************/
/**
 *          S5K4H8_IsiSensorSetStreamingIss
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
static RESULT S5K4H8_IsiSensorSetStreamingIss														  
(																									  
	IsiSensorHandle_t	handle, 																	  
	bool_t				on																			  
)																									  
{																									  
	uint32_t RegValue = 0,i=0;																		  
																										
	S5K4H8_Context_t *pS5K4H8Ctx = (S5K4H8_Context_t *)handle;										  
																									  
	RESULT result = RET_SUCCESS;																	  
																									  
	TRACE( S5K4H8_DEBUG, "%s (enter)  on = %d\n", __FUNCTION__,on); 								  
																									  
	if ( pS5K4H8Ctx == NULL )																		  
	{																								  
		return ( RET_WRONG_HANDLE );																  
	}																								  
																									  
	if ( (pS5K4H8Ctx->Configured != BOOL_TRUE) || (pS5K4H8Ctx->Streaming == on) )					  
	{																								  
		return RET_WRONG_STATE; 																	  
	}																								  
																									  
	if (on == BOOL_TRUE)																			  
	{																								  
		/* timing check */																				
		for(i=0; i<4; i++){ 																		  
			S5K4H8_IsiRegReadIssEx(pS5K4H8Ctx, 0x0005, &RegValue, 1);									
			if(RegValue == 0xff )																			
				break;																						  
			osSleep( 15 );																				
			if(i == 4)																					
				TRACE( S5K4H8_ERROR, "%s stream off state error!\n", __FUNCTION__); 					  
		}																							  
																									  
		/* enable streaming */																		  
		result = S5K4H8_IsiRegWriteIssEx ( pS5K4H8Ctx, S5K4H8_MODE_SELECT, S5K4H8_MODE_SELECT_ON, 1); 
		RETURN_RESULT_IF_DIFFERENT( RET_SUCCESS, result );											  
	}																								  
	else																							  
	{																								  
		/* timing check */																				
		for(i=0; i<4; i++){ 																		  
			S5K4H8_IsiRegReadIssEx(pS5K4H8Ctx, 0x0005, &RegValue, 1);									
			if(RegValue != 0xff )																			
				break;																						  
			osSleep( 30 );																				
			if(i == 4)																					
				TRACE( S5K4H8_ERROR, "%s stream on state error!\n", __FUNCTION__);						  
		}																							  
																									  
		/* disable streaming */ 																	  
		result = S5K4H8_IsiRegWriteIssEx ( pS5K4H8Ctx, S5K4H8_MODE_SELECT, S5K4H8_MODE_SELECT_OFF, 1);
		RETURN_RESULT_IF_DIFFERENT( RET_SUCCESS, result );											  
	}																								  
																									  
	if (result == RET_SUCCESS)																		  
	{																								  
		pS5K4H8Ctx->Streaming = on; 																  
	}																								  
																									  
	TRACE( S5K4H8_DEBUG, "%s (exit)\n", __FUNCTION__);												  
																									  
	return ( result );																				  
}																									  




/*****************************************************************************/
/**
 *          S5K4H8_IsiSensorSetPowerIss
 *
 * @brief   Performs the power-up/power-down sequence of the camera, if possible.
 *
 * @param   handle      S5K4H8 sensor instance handle
 * @param   on          new power state (BOOL_TRUE=on, BOOL_FALSE=off)
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_NULL_POINTER
 *****************************************************************************/
static RESULT S5K4H8_IsiSensorSetPowerIss
(
    IsiSensorHandle_t   handle,
    bool_t              on
)
{
    S5K4H8_Context_t *pS5K4H8Ctx = (S5K4H8_Context_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( S5K4H8_INFO, "%s (enter)\n", __FUNCTION__);

    if ( pS5K4H8Ctx == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    pS5K4H8Ctx->Configured = BOOL_FALSE;
    pS5K4H8Ctx->Streaming  = BOOL_FALSE;

    TRACE( S5K4H8_DEBUG, "%s power off \n", __FUNCTION__);
    result = HalSetPower( pS5K4H8Ctx->IsiCtx.HalHandle, pS5K4H8Ctx->IsiCtx.HalDevID, false );
    RETURN_RESULT_IF_DIFFERENT( RET_SUCCESS, result );

    TRACE( S5K4H8_DEBUG, "%s reset on\n", __FUNCTION__);
    result = HalSetReset( pS5K4H8Ctx->IsiCtx.HalHandle, pS5K4H8Ctx->IsiCtx.HalDevID, true );
    RETURN_RESULT_IF_DIFFERENT( RET_SUCCESS, result );

    if (on == BOOL_TRUE)
    {
        TRACE( S5K4H8_DEBUG, "%s power on \n", __FUNCTION__);
        result = HalSetPower( pS5K4H8Ctx->IsiCtx.HalHandle, pS5K4H8Ctx->IsiCtx.HalDevID, true );
        RETURN_RESULT_IF_DIFFERENT( RET_SUCCESS, result );

        osSleep( 10 );

        TRACE( S5K4H8_DEBUG, "%s reset off \n", __FUNCTION__);
        result = HalSetReset( pS5K4H8Ctx->IsiCtx.HalHandle, pS5K4H8Ctx->IsiCtx.HalDevID, false );
        RETURN_RESULT_IF_DIFFERENT( RET_SUCCESS, result );

        osSleep( 10 );

        TRACE( S5K4H8_DEBUG, "%s reset on \n", __FUNCTION__);
        result = HalSetReset( pS5K4H8Ctx->IsiCtx.HalHandle, pS5K4H8Ctx->IsiCtx.HalDevID, true );
        RETURN_RESULT_IF_DIFFERENT( RET_SUCCESS, result );

        osSleep( 10 );

        TRACE( S5K4H8_DEBUG, "%s reset off \n", __FUNCTION__);
        result = HalSetReset( pS5K4H8Ctx->IsiCtx.HalHandle, pS5K4H8Ctx->IsiCtx.HalDevID, false );

        osSleep( 50 );
    }

    TRACE( S5K4H8_INFO, "%s (exit)\n", __FUNCTION__);

    return ( result );
}



/*****************************************************************************/
/**
 *          S5K4H8_IsiCheckSensorConnectionIss
 *
 * @brief   Checks the I2C-Connection to sensor by reading sensor revision id.
 *
 * @param   handle      S5K4H8 sensor instance handle
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_NULL_POINTER
 *****************************************************************************/
static RESULT S5K4H8_IsiCheckSensorConnectionIss
(
    IsiSensorHandle_t   handle
)
{
    uint32_t RevId;
    uint32_t value;

    RESULT result = RET_SUCCESS;

    TRACE( S5K4H8_INFO, "%s (enter)\n", __FUNCTION__);

    if ( handle == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    RevId = S5K4H8_CHIP_ID_HIGH_BYTE_DEFAULT;
    RevId = (RevId << 16U) | (S5K4H8_CHIP_ID_MIDDLE_BYTE_DEFAULT<<8U);
    RevId = RevId | S5K4H8_CHIP_ID_LOW_BYTE_DEFAULT;

    result = S5K4H8_IsiGetSensorRevisionIss( handle, &value );
    if ( (result != RET_SUCCESS) || (RevId != value) )
    {
        TRACE( S5K4H8_ERROR, "%s RevId = 0x%08x, value = 0x%08x \n", __FUNCTION__, RevId, value );
        return ( RET_FAILURE );
    }

    TRACE( S5K4H8_DEBUG, "%s RevId = 0x%08x, value = 0x%08x \n", __FUNCTION__, RevId, value );

    TRACE( S5K4H8_INFO, "%s (exit)\n", __FUNCTION__);

    return ( result );
}



/*****************************************************************************/
/**
 *          S5K4H8_IsiGetSensorRevisionIss
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
static RESULT S5K4H8_IsiGetSensorRevisionIss
(
    IsiSensorHandle_t   handle,
    uint32_t            *p_value
)
{
    RESULT result = RET_SUCCESS;

    uint32_t data;

    TRACE( S5K4H8_INFO, "%s (enter)\n", __FUNCTION__);

    if ( handle == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    if ( p_value == NULL )
    {
        return ( RET_NULL_POINTER );
    }

    *p_value = 0U;
    result = S5K4H8_IsiRegReadIss ( handle, S5K4H8_CHIP_ID_HIGH_BYTE, &data );
    *p_value = ( (data & 0xFF) << 16U );
    result = S5K4H8_IsiRegReadIss ( handle, S5K4H8_CHIP_ID_MIDDLE_BYTE, &data );
    *p_value |= ( (data & 0xFF) << 8U );
    result = S5K4H8_IsiRegReadIss ( handle, S5K4H8_CHIP_ID_LOW_BYTE, &data );
    *p_value |= ( (data & 0xFF));

    TRACE( S5K4H8_INFO, "%s (exit)\n", __FUNCTION__);

    return ( result );
}



/*****************************************************************************/
/**
 *          S5K4H8_IsiRegReadIss
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
 *****************************************************************************/
static RESULT S5K4H8_IsiRegReadIss
(
    IsiSensorHandle_t   handle,
    const uint32_t      address,
    uint32_t            *p_value
)
{
    RESULT result = RET_SUCCESS;

    TRACE( S5K4H8_INFO, "%s (enter)\n", __FUNCTION__);

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
        uint8_t NrOfBytes = IsiGetNrDatBytesIss( address, S5K4H8_g_1632x1224_fourlane );
        if ( !NrOfBytes )
        {
            NrOfBytes = 1;
        }

        *p_value = 0;

        IsiSensorContext_t *pSensorCtx = (IsiSensorContext_t *)handle;        
        result = IsiI2cReadSensorRegister( handle, address, (uint8_t *)p_value, NrOfBytes, BOOL_TRUE );
    }

    TRACE( S5K4H8_INFO, "%s (exit: 0x%08x 0x%08x)\n", __FUNCTION__, address, *p_value);

    return ( result );
}

static RESULT S5K4H8_IsiRegReadIssEx
(
    IsiSensorHandle_t   handle,
    const uint32_t      address,
    uint32_t            *p_value,
    uint8_t             bytes
)
{
    RESULT result = RET_SUCCESS;

    TRACE( S5K4H8_INFO, "%s (enter)\n", __FUNCTION__);

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
        uint8_t NrOfBytes = bytes;
        if ( !NrOfBytes )
        {
            NrOfBytes = 1;
        }

        *p_value = 0;

        IsiSensorContext_t *pSensorCtx = (IsiSensorContext_t *)handle;        
        result = IsiI2cReadSensorRegister( handle, address, (uint8_t *)p_value, NrOfBytes, BOOL_TRUE );
    }

    TRACE( S5K4H8_INFO, "%s (exit: 0x%08x 0x%08x)\n", __FUNCTION__, address, *p_value);

    return ( result );
}


/*****************************************************************************/
/**
 *          S5K4H8_IsiRegWriteIss
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
 *****************************************************************************/
static RESULT S5K4H8_IsiRegWriteIss
(
    IsiSensorHandle_t   handle,
    const uint32_t      address,
    const uint32_t      value
)
{
    RESULT result = RET_SUCCESS;

    uint8_t NrOfBytes;

    TRACE( S5K4H8_INFO, "%s (enter)\n", __FUNCTION__);

    if ( handle == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    NrOfBytes = IsiGetNrDatBytesIss( address, S5K4H8_g_1632x1224_fourlane );
    if ( !NrOfBytes )
    {
        NrOfBytes = 1;
    }

    result = IsiI2cWriteSensorRegister( handle, address, (uint8_t *)(&value), NrOfBytes, BOOL_TRUE );

    TRACE( S5K4H8_INFO, "%s (exit: 0x%08x 0x%08x)\n", __FUNCTION__, address, value);

    return ( result );
}


static RESULT S5K4H8_IsiRegWriteIssEx
(
    IsiSensorHandle_t   handle,
    const uint32_t      address,
    const uint32_t      value,
    const uint8_t       bytes
)
{
    RESULT result = RET_SUCCESS;

    uint8_t NrOfBytes;

    TRACE( S5K4H8_INFO, "%s (enter)\n", __FUNCTION__);

    if ( handle == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    NrOfBytes = bytes;
    if ( !NrOfBytes )
    {
        NrOfBytes = 1;
    }

    result = IsiI2cWriteSensorRegister( handle, address, (uint8_t *)(&value), NrOfBytes, BOOL_TRUE );

    TRACE( S5K4H8_INFO, "%s (exit: 0x%08x 0x%08x)\n", __FUNCTION__, address, value);

    return ( result );
}

/*****************************************************************************/
/**
 *          S5K4H8_IsiGetGainLimitsIss
 *
 * @brief   Returns the exposure minimal and maximal values of an
 *          S5K4H8 instance
 *
 * @param   handle       S5K4H8 sensor instance handle
 * @param   pMinExposure Pointer to a variable receiving minimal exposure value
 * @param   pMaxExposure Pointer to a variable receiving maximal exposure value
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_NULL_POINTER
 *****************************************************************************/
static RESULT S5K4H8_IsiGetGainLimitsIss
(
    IsiSensorHandle_t   handle,
    float               *pMinGain,
    float               *pMaxGain
)
{
    S5K4H8_Context_t *pS5K4H8Ctx = (S5K4H8_Context_t *)handle;

    RESULT result = RET_SUCCESS;

    uint32_t RegValue = 0;

    TRACE( S5K4H8_INFO, "%s: (enter)\n", __FUNCTION__);

    if ( pS5K4H8Ctx == NULL )
    {
        TRACE( S5K4H8_ERROR, "%s: Invalid sensor handle (NULL pointer detected)\n", __FUNCTION__ );
        return ( RET_WRONG_HANDLE );
    }

    if ( (pMinGain == NULL) || (pMaxGain == NULL) )
    {
        TRACE( S5K4H8_ERROR, "%s: NULL pointer received!!\n" );
        return ( RET_NULL_POINTER );
    }

    *pMinGain = pS5K4H8Ctx->AecMinGain;
    *pMaxGain = pS5K4H8Ctx->AecMaxGain;

    TRACE( S5K4H8_INFO, "%s: (enter)\n", __FUNCTION__);

    return ( result );
}


/*****************************************************************************/
/**
 *          S5K4H8_IsiGetIntegrationTimeLimitsIss
 *
 * @brief   Returns the minimal and maximal integration time values of an
 *          S5K4H8 instance
 *
 * @param   handle       S5K4H8 sensor instance handle
 * @param   pMinExposure Pointer to a variable receiving minimal exposure value
 * @param   pMaxExposure Pointer to a variable receiving maximal exposure value
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_NULL_POINTER
 *****************************************************************************/
static RESULT S5K4H8_IsiGetIntegrationTimeLimitsIss
(
    IsiSensorHandle_t   handle,
    float               *pMinIntegrationTime,
    float               *pMaxIntegrationTime
)
{
    S5K4H8_Context_t *pS5K4H8Ctx = (S5K4H8_Context_t *)handle;

    RESULT result = RET_SUCCESS;

    uint32_t RegValue = 0;

    TRACE( S5K4H8_INFO, "%s: (enter)\n", __FUNCTION__);

    if ( pS5K4H8Ctx == NULL )
    {
        TRACE( S5K4H8_ERROR, "%s: Invalid sensor handle (NULL pointer detected)\n", __FUNCTION__ );
        return ( RET_WRONG_HANDLE );
    }

    if ( (pMinIntegrationTime == NULL) || (pMaxIntegrationTime == NULL) )
    {
        TRACE( S5K4H8_ERROR, "%s: NULL pointer received!!\n" );
        return ( RET_NULL_POINTER );
    }

    *pMinIntegrationTime = pS5K4H8Ctx->AecMinIntegrationTime;
    *pMaxIntegrationTime = pS5K4H8Ctx->AecMaxIntegrationTime;

    TRACE( S5K4H8_INFO, "%s: (enter)\n", __FUNCTION__);

    return ( result );
}


/*****************************************************************************/
/**
 *          S5K4H8_IsiGetGainIss
 *
 * @brief   Reads gain values from the image sensor module.
 *
 * @param   handle                  S5K4H8 sensor instance handle
 * @param   pSetGain                set gain
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_NULL_POINTER
 *****************************************************************************/
RESULT S5K4H8_IsiGetGainIss
(
    IsiSensorHandle_t   handle,
    float               *pSetGain
)
{
	uint32_t data= 0;
	uint32_t result_gain= 0;
	
	S5K4H8_Context_t *pS5K4H8Ctx = (S5K4H8_Context_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( S5K4H8_INFO, "%s: (enter)\n", __FUNCTION__);

    if ( pS5K4H8Ctx == NULL )
    {
        TRACE( S5K4H8_ERROR, "%s: Invalid sensor handle (NULL pointer detected)\n", __FUNCTION__ );
        return ( RET_WRONG_HANDLE );
    }

    if ( pSetGain == NULL)
    {
        return ( RET_NULL_POINTER );
    }

    *pSetGain = pS5K4H8Ctx->AecCurGain;   
    TRACE( S5K4H8_INFO, "%s: (exit)\n", __FUNCTION__);

    return ( result );
}



/*****************************************************************************/
/**
 *          S5K4H8_IsiGetGainIncrementIss
 *
 * @brief   Get smallest possible gain increment.
 *
 * @param   handle                  S5K4H8 sensor instance handle
 * @param   pIncr                   increment
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_NULL_POINTER
 *****************************************************************************/
RESULT S5K4H8_IsiGetGainIncrementIss
(
    IsiSensorHandle_t   handle,
    float               *pIncr
)
{
    S5K4H8_Context_t *pS5K4H8Ctx = (S5K4H8_Context_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( S5K4H8_INFO, "%s: (enter)\n", __FUNCTION__);

    if ( pS5K4H8Ctx == NULL )
    {
        TRACE( S5K4H8_ERROR, "%s: Invalid sensor handle (NULL pointer detected)\n", __FUNCTION__ );
        return ( RET_WRONG_HANDLE );
    }

    if ( pIncr == NULL)
    {
        return ( RET_NULL_POINTER );
    }

    //_smallest_ increment the sensor/driver can handle (e.g. used for sliders in the application)
    *pIncr = pS5K4H8Ctx->AecGainIncrement;

    TRACE( S5K4H8_INFO, "%s: (exit)\n", __FUNCTION__);

    return ( result );
}



/*****************************************************************************/
/**
 *          S5K4H8_IsiSetGainIss
 *
 * @brief   Writes gain values to the image sensor module.
 *          Updates current gain and exposure in sensor struct/state.
 *
 * @param   handle                  S5K4H8 sensor instance handle
 * @param   NewGain                 gain to be set
 * @param   pSetGain                set gain
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NULL_POINTER
 * @retval  RET_INVALID_PARM
 * @retval  RET_FAILURE
 *****************************************************************************/
RESULT S5K4H8_IsiSetGainIss
(
    IsiSensorHandle_t   handle,
    float               NewGain,
    float               *pSetGain
)
{
    S5K4H8_Context_t *pS5K4H8Ctx = (S5K4H8_Context_t *)handle;

    RESULT result = RET_SUCCESS;

    uint16_t usGain = 0;
	uint32_t data= 0;
	uint32_t result_gain= 0;

    TRACE( S5K4H8_INFO, "%s: (enter) pS5K4H8Ctx->AecMaxGain(%f) \n", __FUNCTION__,pS5K4H8Ctx->AecMaxGain);

    if ( pS5K4H8Ctx == NULL )
    {
        TRACE( S5K4H8_ERROR, "%s: Invalid sensor handle (NULL pointer detected)\n", __FUNCTION__ );
        return ( RET_WRONG_HANDLE );
    }

    if ( pSetGain == NULL)
    {
        TRACE( S5K4H8_ERROR, "%s: Invalid parameter (NULL pointer detected)\n", __FUNCTION__ );
        return ( RET_NULL_POINTER );
    }

  
    if( NewGain < pS5K4H8Ctx->AecMinGain ) NewGain = pS5K4H8Ctx->AecMinGain;
    if( NewGain > pS5K4H8Ctx->AecMaxGain ) NewGain = pS5K4H8Ctx->AecMaxGain;

    usGain = (uint16_t)(NewGain * S5K4H8_MAXN_GAIN + 0.5f);
    TRACE( S5K4H8_INFO, "%s:NewGain=%f, usGain =%d, pS5K4H8Ctx->OldGain =%d\n", __FUNCTION__, NewGain,usGain, pS5K4H8Ctx->OldGain);

    // write new gain into sensor registers, do not write if nothing has changed
    if( usGain != pS5K4H8Ctx->OldGain )
    {
        result = S5K4H8_IsiRegWriteIssEx( pS5K4H8Ctx, S5K4H8_AEC_AGC_ADJ_H, usGain&0xffff, 2);
        RETURN_RESULT_IF_DIFFERENT( RET_SUCCESS, result );

        pS5K4H8Ctx->OldGain = usGain;
    }

    //calculate gain actually set
    pS5K4H8Ctx->AecCurGain = ( (float)usGain ) / S5K4H8_MAXN_GAIN;

    //return current state
    *pSetGain = pS5K4H8Ctx->AecCurGain;
    TRACE( S5K4H8_INFO, "%s: psetgain=%f, NewGain=%f,result_gain=%x \n", __FUNCTION__, *pSetGain, NewGain,result_gain);

    TRACE( S5K4H8_INFO, "%s: (exit)\n", __FUNCTION__);

    return ( result );
}


/*****************************************************************************/
/**
 *          S5K4H8_IsiGetIntegrationTimeIss
 *
 * @brief   Reads integration time values from the image sensor module.
 *
 * @param   handle                  S5K4H8 sensor instance handle
 * @param   pSetIntegrationTime     set integration time
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_NULL_POINTER
 *****************************************************************************/
RESULT S5K4H8_IsiGetIntegrationTimeIss
(
    IsiSensorHandle_t   handle,
    float               *pSetIntegrationTime
)
{
    S5K4H8_Context_t *pS5K4H8Ctx = (S5K4H8_Context_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( S5K4H8_INFO, "%s: (enter)\n", __FUNCTION__);

    if ( pS5K4H8Ctx == NULL )
    {
        TRACE( S5K4H8_ERROR, "%s: Invalid sensor handle (NULL pointer detected)\n", __FUNCTION__ );
        return ( RET_WRONG_HANDLE );
    }

    if ( pSetIntegrationTime == NULL )
    {
        return ( RET_NULL_POINTER );
    }

    *pSetIntegrationTime = pS5K4H8Ctx->AecCurIntegrationTime;

    TRACE( S5K4H8_INFO, "%s: (exit)\n", __FUNCTION__);

    return ( result );
}



/*****************************************************************************/
/**
 *          S5K4H8_IsiGetIntegrationTimeIncrementIss
 *
 * @brief   Get smallest possible integration time increment.
 *
 * @param   handle                  S5K4H8 sensor instance handle
 * @param   pIncr                   increment
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_NULL_POINTER
 *****************************************************************************/
RESULT S5K4H8_IsiGetIntegrationTimeIncrementIss
(
    IsiSensorHandle_t   handle,
    float               *pIncr
)
{
    S5K4H8_Context_t *pS5K4H8Ctx = (S5K4H8_Context_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( S5K4H8_INFO, "%s: (enter)\n", __FUNCTION__);

    if ( pS5K4H8Ctx == NULL )
    {
        TRACE( S5K4H8_ERROR, "%s: Invalid sensor handle (NULL pointer detected)\n", __FUNCTION__ );
        return ( RET_WRONG_HANDLE );
    }

    if ( pIncr == NULL )
    {
        return ( RET_NULL_POINTER );
    }

    //_smallest_ increment the sensor/driver can handle (e.g. used for sliders in the application)
    *pIncr = pS5K4H8Ctx->AecIntegrationTimeIncrement;

    TRACE( S5K4H8_INFO, "%s: (exit)\n", __FUNCTION__);

    return ( result );
}



/*****************************************************************************/
/**
 *          S5K4H8_IsiSetIntegrationTimeIss
 *
 * @brief   Writes gain and integration time values to the image sensor module.
 *          Updates current integration time and exposure in sensor
 *          struct/state.
 *
 * @param   handle                  S5K4H8 sensor instance handle
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
 *****************************************************************************/
RESULT S5K4H8_IsiSetIntegrationTimeIss
(
    IsiSensorHandle_t   handle,
    float               NewIntegrationTime,
    float               *pSetIntegrationTime,
    uint8_t             *pNumberOfFramesToSkip
)
{
    S5K4H8_Context_t *pS5K4H8Ctx = (S5K4H8_Context_t *)handle;

    RESULT result = RET_SUCCESS;

    uint32_t CoarseIntegrationTime = 0;
	uint32_t data= 0;
	uint32_t result_intertime= 0;
	
    float ShutterWidthPck = 0.0f; //shutter width in pixel clock periods

    TRACE( S5K4H8_INFO, "%s: (enter) NewIntegrationTime: %f (min: %f   max: %f)\n", __FUNCTION__,
        NewIntegrationTime,
        pS5K4H8Ctx->AecMinIntegrationTime,
        pS5K4H8Ctx->AecMaxIntegrationTime);

    if ( pS5K4H8Ctx == NULL )
    {
        TRACE( S5K4H8_ERROR, "%s: Invalid sensor handle (NULL pointer detected)\n", __FUNCTION__ );
        return ( RET_WRONG_HANDLE );
    }

    if ( (pSetIntegrationTime == NULL) || (pNumberOfFramesToSkip == NULL) )
    {
        TRACE( S5K4H8_ERROR, "%s: Invalid parameter (NULL pointer detected)\n", __FUNCTION__ );
        return ( RET_NULL_POINTER );
    }

    //maximum and minimum integration time is limited by the sensor, if this limit is not
    //considered, the exposure control loop needs lots of time to return to a new state
    //so limit to allowed range
    if ( NewIntegrationTime > pS5K4H8Ctx->AecMaxIntegrationTime ) NewIntegrationTime = pS5K4H8Ctx->AecMaxIntegrationTime;
    if ( NewIntegrationTime < pS5K4H8Ctx->AecMinIntegrationTime ) NewIntegrationTime = pS5K4H8Ctx->AecMinIntegrationTime;

    //the actual integration time is given by
    //integration_time = ( coarse_integration_time * line_length_pck + fine_integration_time ) / vt_pix_clk_freq
    //=>
    //coarse_integration_time = (int)( integration_time * vt_pix_clk_freq  / line_length_pck )
    //fine_integration_time   = integration_time * vt_pix_clk_freq - coarse_integration_time * line_length_pck
    //
    //fine integration is not supported by S5K4H8
    //=>
    //coarse_integration_time = (int)( integration_time * vt_pix_clk_freq  / line_length_pck + 0.5 )

    ShutterWidthPck = NewIntegrationTime * ( (float)pS5K4H8Ctx->VtPixClkFreq );

    // avoid division by zero
    if ( pS5K4H8Ctx->LineLengthPck == 0 )
    {
        TRACE( S5K4H8_ERROR, "%s: Division by zero!\n", __FUNCTION__ );
        return ( RET_DIVISION_BY_ZERO );
    }

    //calculate the integer part of the integration time in units of line length
    //calculate the fractional part of the integration time in units of pixel clocks
    //CoarseIntegrationTime = (uint32_t)( ShutterWidthPck / ((float)pS5K4H8Ctx->LineLengthPck) );
    //FineIntegrationTime   = ( (uint32_t)ShutterWidthPck ) - ( CoarseIntegrationTime * pS5K4H8Ctx->LineLengthPck );
    CoarseIntegrationTime = (uint32_t)( ShutterWidthPck / ((float)pS5K4H8Ctx->LineLengthPck) + 0.5f );

    // write new integration time into sensor registers
    // do not write if nothing has changed
    if( CoarseIntegrationTime != pS5K4H8Ctx->OldCoarseIntegrationTime )
    {
        result = S5K4H8_IsiRegWriteIssEx( pS5K4H8Ctx, S5K4H8_AEC_EXPO_COARSE, (CoarseIntegrationTime & 0x0000FFFFU), 2);
        RETURN_RESULT_IF_DIFFERENT( RET_SUCCESS, result );

        pS5K4H8Ctx->OldCoarseIntegrationTime = CoarseIntegrationTime;   // remember current integration time
        *pNumberOfFramesToSkip = 1U; //skip 1 frame
    }
    else
    {
        *pNumberOfFramesToSkip = 0U; //no frame skip
    }

    //calculate integration time actually set
    //pS5K4H8Ctx->AecCurIntegrationTime = ( ((float)CoarseIntegrationTime) * ((float)pS5K4H8Ctx->LineLengthPck) + ((float)FineIntegrationTime) ) / pS5K4H8Ctx->VtPixClkFreq;
    pS5K4H8Ctx->AecCurIntegrationTime = ((float)CoarseIntegrationTime) * ((float)pS5K4H8Ctx->LineLengthPck) / pS5K4H8Ctx->VtPixClkFreq;

    //return current state
    *pSetIntegrationTime = pS5K4H8Ctx->AecCurIntegrationTime;

    TRACE( S5K4H8_INFO, "%s:\n"
         "pS5K4H8Ctx->VtPixClkFreq:%f pS5K4H8Ctx->LineLengthPck:%x \n"
         "SetTi=%f    NewTi=%f  CoarseIntegrationTime=%x \n"
         "result_intertime = %x \n", __FUNCTION__, 
         pS5K4H8Ctx->VtPixClkFreq,pS5K4H8Ctx->LineLengthPck,
         *pSetIntegrationTime,NewIntegrationTime,CoarseIntegrationTime,
         result_intertime);
    TRACE( S5K4H8_INFO, "%s: (exit)\n", __FUNCTION__);

    return ( result );
}




/*****************************************************************************/
/**
 *          S5K4H8_IsiExposureControlIss
 *
 * @brief   Camera hardware dependent part of the exposure control loop.
 *          Calculates appropriate register settings from the new exposure
 *          values and writes them to the image sensor module.
 *
 * @param   handle                  S5K4H8 sensor instance handle
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
 *****************************************************************************/
RESULT S5K4H8_IsiExposureControlIss
(
    IsiSensorHandle_t   handle,
    float               NewGain,
    float               NewIntegrationTime,
    uint8_t             *pNumberOfFramesToSkip,
    float               *pSetGain,
    float               *pSetIntegrationTime
)
{
    S5K4H8_Context_t *pS5K4H8Ctx = (S5K4H8_Context_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( S5K4H8_INFO, "%s: (enter)\n", __FUNCTION__);

    if ( pS5K4H8Ctx == NULL )
    {
        TRACE( S5K4H8_ERROR, "%s: Invalid sensor handle (NULL pointer detected)\n", __FUNCTION__ );
        return ( RET_WRONG_HANDLE );
    }

    if ( (pNumberOfFramesToSkip == NULL)
            || (pSetGain == NULL)
            || (pSetIntegrationTime == NULL) )
    {
        TRACE( S5K4H8_ERROR, "%s: Invalid parameter (NULL pointer detected)\n", __FUNCTION__ );
        return ( RET_NULL_POINTER );
    }

    TRACE( S5K4H8_INFO, "%s: g=%f, Ti=%f \n", __FUNCTION__, NewGain, NewIntegrationTime );

    result = S5K4H8_IsiSetIntegrationTimeIss( handle, NewIntegrationTime, pSetIntegrationTime, pNumberOfFramesToSkip );
    result = S5K4H8_IsiSetGainIss( handle, NewGain, pSetGain );

    TRACE( S5K4H8_INFO, "%s: set: g=%f, Ti=%f, skip=%d\n", __FUNCTION__, *pSetGain, *pSetIntegrationTime, *pNumberOfFramesToSkip );
    TRACE( S5K4H8_INFO, "%s: (exit)\n", __FUNCTION__);

    return ( result );
}



/*****************************************************************************/
/**
 *          S5K4H8_IsiGetCurrentExposureIss
 *
 * @brief   Returns the currently adjusted AE values
 *
 * @param   handle                  S5K4H8 sensor instance handle
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_NULL_POINTER
 *****************************************************************************/
RESULT S5K4H8_IsiGetCurrentExposureIss
(
    IsiSensorHandle_t   handle,
    float               *pSetGain,
    float               *pSetIntegrationTime
)
{
    S5K4H8_Context_t *pS5K4H8Ctx = (S5K4H8_Context_t *)handle;

    RESULT result = RET_SUCCESS;

    uint32_t RegValue = 0;

    TRACE( S5K4H8_INFO, "%s: (enter)\n", __FUNCTION__);

    if ( pS5K4H8Ctx == NULL )
    {
        TRACE( S5K4H8_ERROR, "%s: Invalid sensor handle (NULL pointer detected)\n", __FUNCTION__ );
        return ( RET_WRONG_HANDLE );
    }

    if ( (pSetGain == NULL) || (pSetIntegrationTime == NULL) )
    {
        return ( RET_NULL_POINTER );
    }

    *pSetGain            = pS5K4H8Ctx->AecCurGain;
    *pSetIntegrationTime = pS5K4H8Ctx->AecCurIntegrationTime;

    TRACE( S5K4H8_INFO, "%s: (exit)\n", __FUNCTION__);

    return ( result );
}



/*****************************************************************************/
/**
 *          S5K4H8_IsiGetResolutionIss
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
 *****************************************************************************/
RESULT S5K4H8_IsiGetResolutionIss
(
    IsiSensorHandle_t   handle,
    uint32_t            *pSetResolution
)
{
    S5K4H8_Context_t *pS5K4H8Ctx = (S5K4H8_Context_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( S5K4H8_INFO, "%s: (enter)\n", __FUNCTION__);

    if ( pS5K4H8Ctx == NULL )
    {
        TRACE( S5K4H8_ERROR, "%s: Invalid sensor handle (NULL pointer detected)\n", __FUNCTION__ );
        return ( RET_WRONG_HANDLE );
    }

    if ( pSetResolution == NULL )
    {
        return ( RET_NULL_POINTER );
    }

    *pSetResolution = pS5K4H8Ctx->Config.Resolution;

    TRACE( S5K4H8_INFO, "%s: (exit)\n", __FUNCTION__);

    return ( result );
}



/*****************************************************************************/
/**
 *          S5K4H8_IsiGetAfpsInfoHelperIss
 *
 * @brief   Calc AFPS sub resolution settings for the given resolution
 *
 * @param   pS5K4H8Ctx             S5K4H8 sensor instance (dummy!) context
 * @param   Resolution              Any supported resolution to query AFPS params for
 * @param   pAfpsInfo               Reference of AFPS info structure to write the results to
 * @param   AfpsStageIdx            Index of current AFPS stage to use
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 *****************************************************************************/
static RESULT S5K4H8_IsiGetAfpsInfoHelperIss(
    S5K4H8_Context_t   *pS5K4H8Ctx,
    uint32_t            Resolution,
    IsiAfpsInfo_t*      pAfpsInfo,
    uint32_t            AfpsStageIdx
)
{
    RESULT result = RET_SUCCESS;

    TRACE( S5K4H8_INFO, "%s: (enter)\n", __FUNCTION__);

    DCT_ASSERT(pS5K4H8Ctx != NULL);
    DCT_ASSERT(pAfpsInfo != NULL);
    DCT_ASSERT(AfpsStageIdx <= ISI_NUM_AFPS_STAGES);

    // update resolution in copy of config in context
    pS5K4H8Ctx->Config.Resolution = Resolution;

    // tell sensor about that
    result = S5K4H8_SetupOutputWindowInternal( pS5K4H8Ctx, &pS5K4H8Ctx->Config,BOOL_FALSE,BOOL_FALSE );
    if ( result != RET_SUCCESS )
    {
        TRACE( S5K4H8_ERROR, "%s: SetupOutputWindow failed for resolution ID %08x.\n", __FUNCTION__, Resolution);
        return ( result );
    }

    // update limits & stuff (reset current & old settings)
    result = S5K4H8_AecSetModeParameters( pS5K4H8Ctx, &pS5K4H8Ctx->Config );
    if ( result != RET_SUCCESS )
    {
        TRACE( S5K4H8_ERROR, "%s: AecSetModeParameters failed for resolution ID %08x.\n", __FUNCTION__, Resolution);
        return ( result );
    }

    // take over params
    pAfpsInfo->Stage[AfpsStageIdx].Resolution = Resolution;
    pAfpsInfo->Stage[AfpsStageIdx].MaxIntTime = pS5K4H8Ctx->AecMaxIntegrationTime;
    pAfpsInfo->AecMinGain           = pS5K4H8Ctx->AecMinGain;
    pAfpsInfo->AecMaxGain           = pS5K4H8Ctx->AecMaxGain;
    pAfpsInfo->AecMinIntTime        = pS5K4H8Ctx->AecMinIntegrationTime;
    pAfpsInfo->AecMaxIntTime        = pS5K4H8Ctx->AecMaxIntegrationTime;
    pAfpsInfo->AecSlowestResolution = Resolution;
    TRACE( S5K4H8_INFO, "%s: (exit)\n", __FUNCTION__);

    return ( result );
}

/*****************************************************************************/
/**
 *          S5K4H8_IsiGetAfpsInfoIss
 *
 * @brief   Returns the possible AFPS sub resolution settings for the given resolution series
 *
 * @param   handle                  S5K4H8 sensor instance handle
 * @param   Resolution              Any resolution within the AFPS group to query;
 *                                  0 (zero) to use the currently configured resolution
 * @param   pAfpsInfo               Reference of AFPS info structure to store the results
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NULL_POINTER
 * @retval  RET_NOTSUPP
 *****************************************************************************/
RESULT S5K4H8_IsiGetAfpsInfoIss(
    IsiSensorHandle_t   handle,
    uint32_t            Resolution,
    IsiAfpsInfo_t*      pAfpsInfo
)
{
    S5K4H8_Context_t *pS5K4H8Ctx = (S5K4H8_Context_t *)handle;

    RESULT result = RET_SUCCESS;

    uint32_t RegValue = 0;

    TRACE( S5K4H8_INFO, "%s: (enter)\n", __FUNCTION__);

    if ( pS5K4H8Ctx == NULL )
    {
        TRACE( S5K4H8_ERROR, "%s: Invalid sensor handle (NULL pointer detected)\n", __FUNCTION__ );
        return ( RET_WRONG_HANDLE );
    }

    if ( pAfpsInfo == NULL )
    {
        return ( RET_NULL_POINTER );
    }

    // use currently configured resolution?
    if (Resolution == 0)
    {
        Resolution = pS5K4H8Ctx->Config.Resolution;
    }

    // prepare index
    uint32_t idx = 0;

    // set current resolution data in info struct
    pAfpsInfo->CurrResolution = pS5K4H8Ctx->Config.Resolution;
    pAfpsInfo->CurrMinIntTime = pS5K4H8Ctx->AecMinIntegrationTime;
    pAfpsInfo->CurrMaxIntTime = pS5K4H8Ctx->AecMaxIntegrationTime;

    // allocate dummy context used for Afps parameter calculation as a copy of current context
    S5K4H8_Context_t *pDummyCtx = (S5K4H8_Context_t*) malloc( sizeof(S5K4H8_Context_t) );
    if ( pDummyCtx == NULL )
    {
        TRACE( S5K4H8_ERROR,  "%s: Can't allocate dummy S5K4H8 context\n",  __FUNCTION__ );
        return ( RET_OUTOFMEM );
    }
    *pDummyCtx = *pS5K4H8Ctx;

    // set AFPS mode in dummy context
    pDummyCtx->isAfpsRun = BOOL_TRUE;

#define AFPSCHECKANDADD(_res_) \
    { \
        RESULT lres = S5K4H8_IsiGetAfpsInfoHelperIss( pDummyCtx, _res_, pAfpsInfo, idx); \
        if ( lres == RET_SUCCESS ) \
        { \
            ++idx; \
        } \
        else \
        { \
            UPDATE_RESULT( result, lres ); \
        } \
    }

    // check which AFPS series is requested and build its params list for the enabled AFPS resolutions
    switch (pS5K4H8Ctx->IsiSensorMipiInfo.ucMipiLanes)
    {
        case SUPPORT_MIPI_ONE_LANE:
        {
            break;
        }

        case SUPPORT_MIPI_TWO_LANE:
        {
            switch(Resolution)
            {
				default:
					TRACE( S5K4H8_DEBUG,  "%s: Resolution %08x not supported by AFPS\n",  __FUNCTION__, Resolution );
					result = RET_NOTSUPP;
					break;
				   
				case ISI_RES_1632_1224P30:
				case ISI_RES_1632_1224P25:
				case ISI_RES_1632_1224P20:
				case ISI_RES_1632_1224P15:
				case ISI_RES_1632_1224P10:
					AFPSCHECKANDADD( ISI_RES_1632_1224P30 );
					AFPSCHECKANDADD( ISI_RES_1632_1224P25 );
					AFPSCHECKANDADD( ISI_RES_1632_1224P20 );
					AFPSCHECKANDADD( ISI_RES_1632_1224P15 );
					AFPSCHECKANDADD( ISI_RES_1632_1224P10 );
					break;

	            case ISI_RES_3264_2448P15:
	            case ISI_RES_3264_2448P10:
	            case ISI_RES_3264_2448P7:
	                AFPSCHECKANDADD( ISI_RES_3264_2448P15 );
	                AFPSCHECKANDADD( ISI_RES_3264_2448P10 );
	                //AFPSCHECKANDADD( ISI_RES_3264_2448P7 );
	                break;
			}
            break;
        }

        case SUPPORT_MIPI_FOUR_LANE:
        {
            switch(Resolution)
            {
                default:
                    TRACE( S5K4H8_DEBUG,  "%s: Resolution %08x not supported by AFPS\n",  __FUNCTION__, Resolution );
                    result = RET_NOTSUPP;
                    break;
				#ifdef FOUR_LANE_700M
                case ISI_RES_1632_1224P30:
                case ISI_RES_1632_1224P25:
                case ISI_RES_1632_1224P20:
                case ISI_RES_1632_1224P15:
                case ISI_RES_1632_1224P10:
                    AFPSCHECKANDADD( ISI_RES_1632_1224P30 );
                    AFPSCHECKANDADD( ISI_RES_1632_1224P25 );
                    AFPSCHECKANDADD( ISI_RES_1632_1224P20 );
                    AFPSCHECKANDADD( ISI_RES_1632_1224P15 );
                    AFPSCHECKANDADD( ISI_RES_1632_1224P10 );
                    break;
                    
                case ISI_RES_3264_2448P30:
                case ISI_RES_3264_2448P25:
                case ISI_RES_3264_2448P20:
                case ISI_RES_3264_2448P15:
                case ISI_RES_3264_2448P10:
                case ISI_RES_3264_2448P7:
                    AFPSCHECKANDADD( ISI_RES_3264_2448P30 );
                    AFPSCHECKANDADD( ISI_RES_3264_2448P25 );
                    AFPSCHECKANDADD( ISI_RES_3264_2448P20 );
                    AFPSCHECKANDADD( ISI_RES_3264_2448P15 );
                    AFPSCHECKANDADD( ISI_RES_3264_2448P10 );
                    //AFPSCHECKANDADD( ISI_RES_3264_2448P7 );
                    break;
				#endif
				#ifdef FOUR_LANE_560M
                case ISI_RES_1632_1224P30:
                case ISI_RES_1632_1224P25:
                case ISI_RES_1632_1224P20:
                case ISI_RES_1632_1224P15:
                case ISI_RES_1632_1224P10:
                    AFPSCHECKANDADD( ISI_RES_1632_1224P30 );
                    AFPSCHECKANDADD( ISI_RES_1632_1224P25 );
                    AFPSCHECKANDADD( ISI_RES_1632_1224P20 );
                    AFPSCHECKANDADD( ISI_RES_1632_1224P15 );
                    AFPSCHECKANDADD( ISI_RES_1632_1224P10 );
                    break;
                    
                case ISI_RES_3264_2448P25:
                case ISI_RES_3264_2448P20:
                case ISI_RES_3264_2448P15:
                case ISI_RES_3264_2448P10:
                //case ISI_RES_3264_2448P7:
                    AFPSCHECKANDADD( ISI_RES_3264_2448P25 );
                    AFPSCHECKANDADD( ISI_RES_3264_2448P20 );
                    AFPSCHECKANDADD( ISI_RES_3264_2448P15 );
                    AFPSCHECKANDADD( ISI_RES_3264_2448P10 );
                    //AFPSCHECKANDADD( ISI_RES_3264_2448P7 );
                    break;
				#endif
                // check next series here...
            }
        

            break;
        }

        default:
            TRACE( S5K4H8_ERROR,  "%s: pS5K4H8Ctx->IsiSensorMipiInfo.ucMipiLanes(0x%x) is invalidate!\n", 
                __FUNCTION__, pS5K4H8Ctx->IsiSensorMipiInfo.ucMipiLanes );
            result = RET_FAILURE;
            break;

    }

    // release dummy context again
    free(pDummyCtx);

    TRACE( S5K4H8_INFO, "%s: (exit)\n", __FUNCTION__);

    return ( result );
}



/*****************************************************************************/
/**
 *          S5K4H8_IsiGetCalibKFactor
 *
 * @brief   Returns the S5K4H8 specific K-Factor
 *
 * @param   handle       S5K4H8 sensor instance handle
 * @param   pIsiKFactor  Pointer to Pointer receiving the memory address
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NULL_POINTER
 *****************************************************************************/
static RESULT S5K4H8_IsiGetCalibKFactor
(
    IsiSensorHandle_t   handle,
    Isi1x1FloatMatrix_t **pIsiKFactor
)
{
	return ( RET_SUCCESS );
	S5K4H8_Context_t *pS5K4H8Ctx = (S5K4H8_Context_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( S5K4H8_INFO, "%s: (enter)\n", __FUNCTION__);

    if ( pS5K4H8Ctx == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    if ( pIsiKFactor == NULL )
    {
        return ( RET_NULL_POINTER );
    }

    //*pIsiKFactor = (Isi1x1FloatMatrix_t *)&S5K4H8_KFactor;

    TRACE( S5K4H8_INFO, "%s: (exit)\n", __FUNCTION__);

    return ( result );
}


/*****************************************************************************/
/**
 *          S5K4H8_IsiGetCalibPcaMatrix
 *
 * @brief   Returns the S5K4H8 specific PCA-Matrix
 *
 * @param   handle          S5K4H8 sensor instance handle
 * @param   pIsiPcaMatrix   Pointer to Pointer receiving the memory address
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NULL_POINTER
 *****************************************************************************/
static RESULT S5K4H8_IsiGetCalibPcaMatrix
(
    IsiSensorHandle_t   handle,
    Isi3x2FloatMatrix_t **pIsiPcaMatrix
)
{
	return ( RET_SUCCESS );
	S5K4H8_Context_t *pS5K4H8Ctx = (S5K4H8_Context_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( S5K4H8_INFO, "%s: (enter)\n", __FUNCTION__);

    if ( pS5K4H8Ctx == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    if ( pIsiPcaMatrix == NULL )
    {
        return ( RET_NULL_POINTER );
    }

    //*pIsiPcaMatrix = (Isi3x2FloatMatrix_t *)&S5K4H8_PCAMatrix;

    TRACE( S5K4H8_INFO, "%s: (exit)\n", __FUNCTION__);

    return ( result );
}



/*****************************************************************************/
/**
 *          S5K4H8_IsiGetCalibSvdMeanValue
 *
 * @brief   Returns the sensor specific SvdMean-Vector
 *
 * @param   handle              S5K4H8 sensor instance handle
 * @param   pIsiSvdMeanValue    Pointer to Pointer receiving the memory address
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NULL_POINTER
 *****************************************************************************/
static RESULT S5K4H8_IsiGetCalibSvdMeanValue
(
    IsiSensorHandle_t   handle,
    Isi3x1FloatMatrix_t **pIsiSvdMeanValue
)
{
    IsiSensorContext_t *pSensorCtx = (IsiSensorContext_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( S5K4H8_INFO, "%s: (enter)\n", __FUNCTION__);

    if ( pSensorCtx == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    if ( pIsiSvdMeanValue == NULL )
    {
        return ( RET_NULL_POINTER );
    }

    //*pIsiSvdMeanValue = (Isi3x1FloatMatrix_t *)&S5K4H8_SVDMeanValue;

    TRACE( S5K4H8_INFO, "%s: (exit)\n", __FUNCTION__);

    return ( result );
}



/*****************************************************************************/
/**
 *          S5K4H8_IsiGetCalibSvdMeanValue
 *
 * @brief   Returns a pointer to the sensor specific centerline, a straight
 *          line in Hesse normal form in Rg/Bg colorspace
 *
 * @param   handle              S5K4H8 sensor instance handle
 * @param   pIsiSvdMeanValue    Pointer to Pointer receiving the memory address
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NULL_POINTER
 *****************************************************************************/
static RESULT S5K4H8_IsiGetCalibCenterLine
(
    IsiSensorHandle_t   handle,
    IsiLine_t           **ptIsiCenterLine
)
{
    IsiSensorContext_t *pSensorCtx = (IsiSensorContext_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( S5K4H8_INFO, "%s: (enter)\n", __FUNCTION__);

    if ( pSensorCtx == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    if ( ptIsiCenterLine == NULL )
    {
        return ( RET_NULL_POINTER );
    }

   // *ptIsiCenterLine = (IsiLine_t*)&S5K4H8_CenterLine;

    TRACE( S5K4H8_INFO, "%s: (exit)\n", __FUNCTION__);

    return ( result );
}



/*****************************************************************************/
/**
 *          S5K4H8_IsiGetCalibClipParam
 *
 * @brief   Returns a pointer to the sensor specific arrays for Rg/Bg color
 *          space clipping
 *
 * @param   handle              S5K4H8 sensor instance handle
 * @param   pIsiSvdMeanValue    Pointer to Pointer receiving the memory address
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NULL_POINTER
 *****************************************************************************/
static RESULT S5K4H8_IsiGetCalibClipParam
(
    IsiSensorHandle_t   handle,
    IsiAwbClipParm_t    **pIsiClipParam
)
{
    IsiSensorContext_t *pSensorCtx = (IsiSensorContext_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( S5K4H8_INFO, "%s: (enter)\n", __FUNCTION__);

    if ( pSensorCtx == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    if ( pIsiClipParam == NULL )
    {
        return ( RET_NULL_POINTER );
    }

    //*pIsiClipParam = (IsiAwbClipParm_t *)&S5K4H8_AwbClipParm;

    TRACE( S5K4H8_INFO, "%s: (exit)\n", __FUNCTION__);

    return ( result );
}



/*****************************************************************************/
/**
 *          S5K4H8_IsiGetCalibGlobalFadeParam
 *
 * @brief   Returns a pointer to the sensor specific arrays for AWB out of
 *          range handling
 *
 * @param   handle              S5K4H8 sensor instance handle
 * @param   pIsiSvdMeanValue    Pointer to Pointer receiving the memory address
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NULL_POINTER
 *****************************************************************************/
static RESULT S5K4H8_IsiGetCalibGlobalFadeParam
(
    IsiSensorHandle_t       handle,
    IsiAwbGlobalFadeParm_t  **ptIsiGlobalFadeParam
)
{
    IsiSensorContext_t *pSensorCtx = (IsiSensorContext_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( S5K4H8_INFO, "%s: (enter)\n", __FUNCTION__);

    if ( pSensorCtx == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    if ( ptIsiGlobalFadeParam == NULL )
    {
        return ( RET_NULL_POINTER );
    }

    //*ptIsiGlobalFadeParam = (IsiAwbGlobalFadeParm_t *)&S5K4H8_AwbGlobalFadeParm;

    TRACE( S5K4H8_INFO, "%s: (exit)\n", __FUNCTION__);

    return ( result );
}



/*****************************************************************************/
/**
 *          S5K4H8_IsiGetCalibFadeParam
 *
 * @brief   Returns a pointer to the sensor specific arrays for near white
 *          pixel parameter calculations
 *
 * @param   handle              S5K4H8 sensor instance handle
 * @param   pIsiSvdMeanValue    Pointer to Pointer receiving the memory address
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NULL_POINTER
 *****************************************************************************/
static RESULT S5K4H8_IsiGetCalibFadeParam
(
    IsiSensorHandle_t   handle,
    IsiAwbFade2Parm_t   **ptIsiFadeParam
)
{
    IsiSensorContext_t *pSensorCtx = (IsiSensorContext_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( S5K4H8_INFO, "%s: (enter)\n", __FUNCTION__);

    if ( pSensorCtx == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    if ( ptIsiFadeParam == NULL )
    {
        return ( RET_NULL_POINTER );
    }

   // *ptIsiFadeParam = (IsiAwbFade2Parm_t *)&S5K4H8_AwbFade2Parm;

    TRACE( S5K4H8_INFO, "%s: (exit)\n", __FUNCTION__);

    return ( result );
}

/*****************************************************************************/
/**
 *          S5K4H8_IsiGetIlluProfile
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
 *****************************************************************************/
static RESULT S5K4H8_IsiGetIlluProfile
(
    IsiSensorHandle_t   handle,
    const uint32_t      CieProfile,
    IsiIlluProfile_t    **ptIsiIlluProfile
)
{
    S5K4H8_Context_t *pS5K4H8Ctx = (S5K4H8_Context_t *)handle;

    RESULT result = RET_SUCCESS;
	return ( result );
}



/*****************************************************************************/
/**
 *          S5K4H8_IsiGetLscMatrixTable
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
 *****************************************************************************/
static RESULT S5K4H8_IsiGetLscMatrixTable
(
    IsiSensorHandle_t   handle,
    const uint32_t      CieProfile,
    IsiLscMatrixTable_t **pLscMatrixTable
)
{
    S5K4H8_Context_t *pS5K4H8Ctx = (S5K4H8_Context_t *)handle;

    RESULT result = RET_SUCCESS;
	return ( result );
}


/*****************************************************************************/
/**
 *          S5K4H8_IsiMdiInitMotoDriveMds
 *
 * @brief   General initialisation tasks like I/O initialisation.
 *
 * @param   handle              S5K4H8 sensor instance handle
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NULL_POINTER
 *****************************************************************************/
static RESULT S5K4H8_IsiMdiInitMotoDriveMds
(
    IsiSensorHandle_t   handle
)
{
    S5K4H8_Context_t *pS5K4H8Ctx = (S5K4H8_Context_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( S5K4H8_INFO, "%s: (enter)\n", __FUNCTION__);

    if ( pS5K4H8Ctx == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    TRACE( S5K4H8_INFO, "%s: (exit)\n", __FUNCTION__);

    return ( result );
}



/*****************************************************************************/
/**
 *          S5K4H8_IsiMdiSetupMotoDrive
 *
 * @brief   Setup of the MotoDrive and return possible max step.
 *
 * @param   handle          S5K4H8 sensor instance handle
 *          pMaxStep        pointer to variable to receive the maximum
 *                          possible focus step
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NULL_POINTER
 *****************************************************************************/
static RESULT S5K4H8_IsiMdiSetupMotoDrive
(
    IsiSensorHandle_t   handle,
    uint32_t            *pMaxStep
)
{
    S5K4H8_Context_t *pS5K4H8Ctx = (S5K4H8_Context_t *)handle;
	uint32_t vcm_movefull_t;
    RESULT result = RET_SUCCESS;

    TRACE( S5K4H8_INFO, "%s: (enter)\n", __FUNCTION__);

    if ( pS5K4H8Ctx == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    if ( pMaxStep == NULL )
    {
        return ( RET_NULL_POINTER );
    }
 if ((pS5K4H8Ctx->VcmInfo.StepMode & 0x0c) != 0) {
 	vcm_movefull_t = 64* (1<<(pS5K4H8Ctx->VcmInfo.StepMode & 0x03)) *1024/((1 << (((pS5K4H8Ctx->VcmInfo.StepMode & 0x0c)>>2)-1))*1000);
 }else{
 	vcm_movefull_t =64*1023/1000;
   TRACE( S5K4H8_ERROR, "%s: (---NO SRC---)\n", __FUNCTION__);
 }
 
	  *pMaxStep = (MAX_LOG|(vcm_movefull_t<<16));
   // *pMaxStep = MAX_LOG;

    result = S5K4H8_IsiMdiFocusSet( handle, MAX_LOG );

    TRACE( S5K4H8_INFO, "%s: (exit)\n", __FUNCTION__);

    return ( result );
}



/*****************************************************************************/
/**
 *          S5K4H8_IsiMdiFocusSet
 *
 * @brief   Drives the lens system to a certain focus point.
 *
 * @param   handle          S5K4H8 sensor instance handle
 *          AbsStep         absolute focus point to apply
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NULL_POINTER
 *****************************************************************************/
static RESULT S5K4H8_IsiMdiFocusSet
(
    IsiSensorHandle_t   handle,
    const uint32_t      Position
)
{
    S5K4H8_Context_t *pS5K4H8Ctx = (S5K4H8_Context_t *)handle;

    RESULT result = RET_SUCCESS;

    uint32_t nPosition;
    uint8_t  data[2] = { 0, 0 };

    TRACE( S5K4H8_INFO, "%s: (enter)\n", __FUNCTION__);

    if ( pS5K4H8Ctx == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    /* map 64 to 0 -> infinity */
    //nPosition = ( Position >= MAX_LOG ) ? 0 : ( MAX_REG - (Position * 16U) );
	if( Position > MAX_LOG ){
		TRACE( S5K4H8_ERROR, "%s: pS5K4H8Ctx Position (%d) max_position(%d)\n", __FUNCTION__,Position, MAX_LOG);
		//Position = MAX_LOG;
	}	
    /* ddl@rock-chips.com: v0.3.0 */
    if ( Position >= MAX_LOG )
        nPosition = pS5K4H8Ctx->VcmInfo.StartCurrent;
    else 
        nPosition = pS5K4H8Ctx->VcmInfo.StartCurrent + (pS5K4H8Ctx->VcmInfo.Step*(MAX_LOG-Position));
    /* ddl@rock-chips.com: v0.6.0 */
    if (nPosition > MAX_VCMDRV_REG)  
        nPosition = MAX_VCMDRV_REG;

    TRACE( S5K4H8_INFO, "%s: focus set position_reg_value(%d) position(%d) \n", __FUNCTION__, nPosition, Position);
    data[0] = (uint8_t)(0x00U | (( nPosition & 0x3F0U ) >> 4U));
	data[1] = (uint8_t)( ((nPosition & 0x0FU) << 4U) | pS5K4H8Ctx->VcmInfo.StepMode );
	
    //TRACE( S5K4H8_ERROR, "%s: value = %d, 0x%02x 0x%02x\n", __FUNCTION__, nPosition, data[0], data[1] );

    result = HalWriteI2CMem( pS5K4H8Ctx->IsiCtx.HalHandle,
                             pS5K4H8Ctx->IsiCtx.I2cAfBusNum,
                             pS5K4H8Ctx->IsiCtx.SlaveAfAddress,
                             0,
                             pS5K4H8Ctx->IsiCtx.NrOfAfAddressBytes,
                             data,
                             2U );
    RETURN_RESULT_IF_DIFFERENT( RET_SUCCESS, result );

    TRACE( S5K4H8_INFO, "%s: (exit)\n", __FUNCTION__);
    return ( result );
}



/*****************************************************************************/
/**
 *          S5K4H8_IsiMdiFocusGet
 *
 * @brief   Retrieves the currently applied focus point.
 *
 * @param   handle          S5K4H8 sensor instance handle
 *          pAbsStep        pointer to a variable to receive the current
 *                          focus point
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NULL_POINTER
 *****************************************************************************/
static RESULT S5K4H8_IsiMdiFocusGet
(
    IsiSensorHandle_t   handle,
    uint32_t            *pAbsStep
)
{
    S5K4H8_Context_t *pS5K4H8Ctx = (S5K4H8_Context_t *)handle;

    RESULT result = RET_SUCCESS;
    uint8_t  data[2] = { 0, 0 };

    TRACE( S5K4H8_INFO, "%s: (enter)\n", __FUNCTION__);

    if ( pS5K4H8Ctx == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    if ( pAbsStep == NULL )
    {
        return ( RET_NULL_POINTER );
    }

    result = HalReadI2CMem( pS5K4H8Ctx->IsiCtx.HalHandle,
                            pS5K4H8Ctx->IsiCtx.I2cAfBusNum,
                            pS5K4H8Ctx->IsiCtx.SlaveAfAddress,
                            0,
                            pS5K4H8Ctx->IsiCtx.NrOfAfAddressBytes,
                            data,
                            2U );
    RETURN_RESULT_IF_DIFFERENT( RET_SUCCESS, result );

    TRACE( S5K4H8_ERROR, "%s: value = 0x%02x 0x%02x\n", __FUNCTION__, data[0], data[1] );

    *pAbsStep = ( ((uint32_t)(data[0] & 0x3FU)) << 4U ) | ( ((uint32_t)data[1]) >> 4U );

	if( *pAbsStep <= pS5K4H8Ctx->VcmInfo.StartCurrent)
    {
        *pAbsStep = MAX_LOG;
    }
    else if((*pAbsStep>pS5K4H8Ctx->VcmInfo.StartCurrent) && (*pAbsStep<=pS5K4H8Ctx->VcmInfo.RatedCurrent))
    {
        *pAbsStep = (pS5K4H8Ctx->VcmInfo.RatedCurrent - *pAbsStep ) / pS5K4H8Ctx->VcmInfo.Step;
    }
	else
	{
		*pAbsStep = 0;
	}
    TRACE( S5K4H8_INFO, "%s: (exit)\n", __FUNCTION__);

    return ( result );
}



/*****************************************************************************/
/**
 *          S5K4H8_IsiMdiFocusCalibrate
 *
 * @brief   Triggers a forced calibration of the focus hardware.
 *
 * @param   handle          S5K4H8 sensor instance handle
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NULL_POINTER
 *****************************************************************************/
static RESULT S5K4H8_IsiMdiFocusCalibrate
(
    IsiSensorHandle_t   handle
)
{
    S5K4H8_Context_t *pS5K4H8Ctx = (S5K4H8_Context_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( S5K4H8_INFO, "%s: (enter)\n", __FUNCTION__);

    if ( pS5K4H8Ctx == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    TRACE( S5K4H8_INFO, "%s: (exit)\n", __FUNCTION__);

    return ( result );
}



/*****************************************************************************/
/**
 *          S5K4H8_IsiActivateTestPattern
 *
 * @brief   Triggers a forced calibration of the focus hardware.
 *
 * @param   handle          S5K4H8 sensor instance handle
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NULL_POINTER
 ******************************************************************************/
static RESULT S5K4H8_IsiActivateTestPattern
(
    IsiSensorHandle_t   handle,
    const bool_t        enable
)
{
    S5K4H8_Context_t *pS5K4H8Ctx = (S5K4H8_Context_t *)handle;

    RESULT result = RET_SUCCESS;
	return ( result );
}



/*****************************************************************************/
/**
 *          S5K4H8_IsiGetSensorMipiInfoIss
 *
 * @brief   Triggers a forced calibration of the focus hardware.
 *
 * @param   handle          S5K4H8 sensor instance handle
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NULL_POINTER
  ******************************************************************************/
static RESULT S5K4H8_IsiGetSensorMipiInfoIss
(
    IsiSensorHandle_t   handle,
    IsiSensorMipiInfo   *ptIsiSensorMipiInfo
)
{
    S5K4H8_Context_t *pS5K4H8Ctx = (S5K4H8_Context_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( S5K4H8_INFO, "%s: (enter)\n", __FUNCTION__);

    if ( pS5K4H8Ctx == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }


    if ( ptIsiSensorMipiInfo == NULL )
    {
        return ( result );
    }

	ptIsiSensorMipiInfo->ucMipiLanes = pS5K4H8Ctx->IsiSensorMipiInfo.ucMipiLanes;
    ptIsiSensorMipiInfo->ulMipiFreq= pS5K4H8Ctx->IsiSensorMipiInfo.ulMipiFreq;
    ptIsiSensorMipiInfo->sensorHalDevID = pS5K4H8Ctx->IsiSensorMipiInfo.sensorHalDevID;
    TRACE( S5K4H8_INFO, "%s: (exit)\n", __FUNCTION__);

    return ( result );
}

static RESULT S5K4H8_IsiGetSensorIsiVersion
(  IsiSensorHandle_t   handle,
   unsigned int*     pVersion
)
{
    S5K4H8_Context_t *pS5K4H8Ctx = (S5K4H8_Context_t *)handle;

    RESULT result = RET_SUCCESS;


    TRACE( S5K4H8_INFO, "%s: (enter)\n", __FUNCTION__);

    if ( pS5K4H8Ctx == NULL )
    {
    	TRACE( S5K4H8_ERROR, "%s: pS5K4H8Ctx IS NULL\n", __FUNCTION__);
        return ( RET_WRONG_HANDLE );
    }

	if(pVersion == NULL)
	{
		TRACE( S5K4H8_ERROR, "%s: pVersion IS NULL\n", __FUNCTION__);
        return ( RET_WRONG_HANDLE );
	}

	*pVersion = CONFIG_ISI_VERSION;
	return result;
}

static RESULT S5K4H8_IsiGetSensorTuningXmlVersion
(  IsiSensorHandle_t   handle,
   char**     pTuningXmlVersion
)
{
    S5K4H8_Context_t *pS5K4H8Ctx = (S5K4H8_Context_t *)handle;

    RESULT result = RET_SUCCESS;


    TRACE( S5K4H8_INFO, "%s: (enter)\n", __FUNCTION__);

    if ( pS5K4H8Ctx == NULL )
    {
    	TRACE( S5K4H8_ERROR, "%s: pS5K4H8Ctx IS NULL\n", __FUNCTION__);
        return ( RET_WRONG_HANDLE );
    }

	if(pTuningXmlVersion == NULL)
	{
		TRACE( S5K4H8_ERROR, "%s: pVersion IS NULL\n", __FUNCTION__);
        return ( RET_WRONG_HANDLE );
	}

	*pTuningXmlVersion = S5K4H8_NEWEST_TUNING_XML;
	return result;
}

static RESULT S5K4H8_IsiSetSensorFrameRateLimit(IsiSensorHandle_t handle, uint32_t minimum_framerate)
{
    S5K4H8_Context_t *pS5K4H8Ctx = (S5K4H8_Context_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( S5K4H8_INFO, "%s: (enter)\n", __FUNCTION__);

    if ( pS5K4H8Ctx == NULL )
    {
    	TRACE( S5K4H8_ERROR, "%s: pS5K4H8Ctx IS NULL\n", __FUNCTION__);
        return ( RET_WRONG_HANDLE );
    }
	
	pS5K4H8Ctx->preview_minimum_framerate = minimum_framerate;
	return RET_SUCCESS;
}


/*****************************************************************************/
/**
 *          S5K4H8_IsiGetSensorIss
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
RESULT S5K4H8_IsiGetSensorIss
(
    IsiSensor_t *pIsiSensor
)
{
    RESULT result = RET_SUCCESS;

    TRACE( S5K4H8_INFO, "%s (enter)\n", __FUNCTION__);

    if ( pIsiSensor != NULL )
    {
        pIsiSensor->pszName                             = S5K4H8_g_acName;
        pIsiSensor->pRegisterTable                      = S5K4H8_g_1632x1224_fourlane;
        pIsiSensor->pIsiSensorCaps                      = &S5K4H8_g_IsiSensorDefaultConfig;
		pIsiSensor->pIsiGetSensorIsiVer					= S5K4H8_IsiGetSensorIsiVersion;//oyyf
		pIsiSensor->pIsiGetSensorTuningXmlVersion		= S5K4H8_IsiGetSensorTuningXmlVersion;//oyyf
		pIsiSensor->pIsiCheckOTPInfo                    = check_read_otp;//zyc
		pIsiSensor->pIsiSetSensorOTPInfo				= S5K4H8_IsiSetOTPInfo;
		pIsiSensor->pIsiEnableSensorOTP					= S5K4H8_IsiEnableOTP;
        pIsiSensor->pIsiCreateSensorIss                 = S5K4H8_IsiCreateSensorIss;
        pIsiSensor->pIsiReleaseSensorIss                = S5K4H8_IsiReleaseSensorIss;
        pIsiSensor->pIsiGetCapsIss                      = S5K4H8_IsiGetCapsIss;
        pIsiSensor->pIsiSetupSensorIss                  = S5K4H8_IsiSetupSensorIss;
        pIsiSensor->pIsiChangeSensorResolutionIss       = S5K4H8_IsiChangeSensorResolutionIss;
        pIsiSensor->pIsiSensorSetStreamingIss           = S5K4H8_IsiSensorSetStreamingIss;
        pIsiSensor->pIsiSensorSetPowerIss               = S5K4H8_IsiSensorSetPowerIss;
        pIsiSensor->pIsiCheckSensorConnectionIss        = S5K4H8_IsiCheckSensorConnectionIss;
        pIsiSensor->pIsiGetSensorRevisionIss            = S5K4H8_IsiGetSensorRevisionIss;
        pIsiSensor->pIsiRegisterReadIss                 = S5K4H8_IsiRegReadIss;
        pIsiSensor->pIsiRegisterWriteIss                = S5K4H8_IsiRegWriteIss;

        /* AEC functions */
        pIsiSensor->pIsiExposureControlIss              = S5K4H8_IsiExposureControlIss;
        pIsiSensor->pIsiGetGainLimitsIss                = S5K4H8_IsiGetGainLimitsIss;
        pIsiSensor->pIsiGetIntegrationTimeLimitsIss     = S5K4H8_IsiGetIntegrationTimeLimitsIss;
        pIsiSensor->pIsiGetCurrentExposureIss           = S5K4H8_IsiGetCurrentExposureIss;
        pIsiSensor->pIsiGetGainIss                      = S5K4H8_IsiGetGainIss;
        pIsiSensor->pIsiGetGainIncrementIss             = S5K4H8_IsiGetGainIncrementIss;
        pIsiSensor->pIsiSetGainIss                      = S5K4H8_IsiSetGainIss;
        pIsiSensor->pIsiGetIntegrationTimeIss           = S5K4H8_IsiGetIntegrationTimeIss;
        pIsiSensor->pIsiGetIntegrationTimeIncrementIss  = S5K4H8_IsiGetIntegrationTimeIncrementIss;
        pIsiSensor->pIsiSetIntegrationTimeIss           = S5K4H8_IsiSetIntegrationTimeIss;
        pIsiSensor->pIsiGetResolutionIss                = S5K4H8_IsiGetResolutionIss;
        pIsiSensor->pIsiGetAfpsInfoIss                  = S5K4H8_IsiGetAfpsInfoIss;

        /* AWB specific functions */
        pIsiSensor->pIsiGetCalibKFactor                 = S5K4H8_IsiGetCalibKFactor;
        pIsiSensor->pIsiGetCalibPcaMatrix               = S5K4H8_IsiGetCalibPcaMatrix;
        pIsiSensor->pIsiGetCalibSvdMeanValue            = S5K4H8_IsiGetCalibSvdMeanValue;
        pIsiSensor->pIsiGetCalibCenterLine              = S5K4H8_IsiGetCalibCenterLine;
        pIsiSensor->pIsiGetCalibClipParam               = S5K4H8_IsiGetCalibClipParam;
        pIsiSensor->pIsiGetCalibGlobalFadeParam         = S5K4H8_IsiGetCalibGlobalFadeParam;
        pIsiSensor->pIsiGetCalibFadeParam               = S5K4H8_IsiGetCalibFadeParam;
        pIsiSensor->pIsiGetIlluProfile                  = S5K4H8_IsiGetIlluProfile;
        pIsiSensor->pIsiGetLscMatrixTable               = S5K4H8_IsiGetLscMatrixTable;

        /* AF functions */
        pIsiSensor->pIsiMdiInitMotoDriveMds             = S5K4H8_IsiMdiInitMotoDriveMds;
        pIsiSensor->pIsiMdiSetupMotoDrive               = S5K4H8_IsiMdiSetupMotoDrive;
        pIsiSensor->pIsiMdiFocusSet                     = S5K4H8_IsiMdiFocusSet;
        pIsiSensor->pIsiMdiFocusGet                     = S5K4H8_IsiMdiFocusGet;
        pIsiSensor->pIsiMdiFocusCalibrate               = S5K4H8_IsiMdiFocusCalibrate;

        /* MIPI */
        pIsiSensor->pIsiGetSensorMipiInfoIss            = S5K4H8_IsiGetSensorMipiInfoIss;

        /* Testpattern */
        pIsiSensor->pIsiActivateTestPattern             = S5K4H8_IsiActivateTestPattern;
        pIsiSensor->pIsiSetSensorFrameRateLimit			= S5K4H8_IsiSetSensorFrameRateLimit;
    }
    else
    {
        result = RET_NULL_POINTER;
    }

    TRACE( S5K4H8_INFO, "%s (exit)\n", __FUNCTION__);

    return ( result );
}

static RESULT S5K4H8_IsiGetSensorI2cInfo(sensor_i2c_info_t** pdata)
{
    sensor_i2c_info_t* pSensorI2cInfo;

    pSensorI2cInfo = ( sensor_i2c_info_t * )malloc ( sizeof (sensor_i2c_info_t) );

    if ( pSensorI2cInfo == NULL )
    {
        TRACE( S5K4H8_ERROR,  "%s: Can't allocate S5K4H8 context\n",  __FUNCTION__ );
        return ( RET_OUTOFMEM );
    }
    MEMSET( pSensorI2cInfo, 0, sizeof( sensor_i2c_info_t ) );

    
    pSensorI2cInfo->i2c_addr = S5K4H8_SLAVE_ADDR;
    pSensorI2cInfo->i2c_addr2 = S5K4H8_SLAVE_ADDR2;
    pSensorI2cInfo->soft_reg_addr = S5K4H8_SOFTWARE_RST;
    pSensorI2cInfo->soft_reg_value = S5K4H8_SOFTWARE_RST_VALUE;
    pSensorI2cInfo->reg_size = 2;
    pSensorI2cInfo->value_size = 1;

    {
        IsiSensorCaps_t Caps;
        sensor_caps_t *pCaps;
        uint32_t lanes,i;        

        for (i=0; i<3; i++) {
            lanes = (1<<i);
            ListInit(&pSensorI2cInfo->lane_res[i]);
            if (g_suppoted_mipi_lanenum_type & lanes) {
                Caps.Index = 0;            
                while(S5K4H8_IsiGetCapsIssInternal(&Caps,lanes)==RET_SUCCESS) {
                    pCaps = malloc(sizeof(sensor_caps_t));
                    if (pCaps != NULL) {
                        memcpy(&pCaps->caps,&Caps,sizeof(IsiSensorCaps_t));
                        ListPrepareItem(pCaps);
                        ListAddTail(&pSensorI2cInfo->lane_res[i], pCaps);
                    }
                    Caps.Index++;
                }
            }
        }
    }
    
    ListInit(&pSensorI2cInfo->chipid_info);

    sensor_chipid_info_t* pChipIDInfo_H = (sensor_chipid_info_t *) malloc( sizeof(sensor_chipid_info_t) );
    if ( !pChipIDInfo_H )
    {
        return RET_OUTOFMEM;
    }
    MEMSET( pChipIDInfo_H, 0, sizeof(*pChipIDInfo_H) );    
    pChipIDInfo_H->chipid_reg_addr = S5K4H8_CHIP_ID_HIGH_BYTE;  
    pChipIDInfo_H->chipid_reg_value = S5K4H8_CHIP_ID_HIGH_BYTE_DEFAULT;
    ListPrepareItem( pChipIDInfo_H );
    ListAddTail( &pSensorI2cInfo->chipid_info, pChipIDInfo_H );

    sensor_chipid_info_t* pChipIDInfo_M = (sensor_chipid_info_t *) malloc( sizeof(sensor_chipid_info_t) );
    if ( !pChipIDInfo_M )
    {
        return RET_OUTOFMEM;
    }
    MEMSET( pChipIDInfo_M, 0, sizeof(*pChipIDInfo_M) ); 
    pChipIDInfo_M->chipid_reg_addr = S5K4H8_CHIP_ID_MIDDLE_BYTE;
    pChipIDInfo_M->chipid_reg_value = S5K4H8_CHIP_ID_MIDDLE_BYTE_DEFAULT;
    ListPrepareItem( pChipIDInfo_M );
    ListAddTail( &pSensorI2cInfo->chipid_info, pChipIDInfo_M );
    
    sensor_chipid_info_t* pChipIDInfo_L = (sensor_chipid_info_t *) malloc( sizeof(sensor_chipid_info_t) );
    if ( !pChipIDInfo_L )
    {
        return RET_OUTOFMEM;
    }
    MEMSET( pChipIDInfo_L, 0, sizeof(*pChipIDInfo_L) ); 
    pChipIDInfo_L->chipid_reg_addr = S5K4H8_CHIP_ID_LOW_BYTE;
    pChipIDInfo_L->chipid_reg_value = S5K4H8_CHIP_ID_LOW_BYTE_DEFAULT;
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
IsiCamDrvConfig_t IsiCamDrvConfig =
{
    0,
    S5K4H8_IsiGetSensorIss,
    {
        0,                      /**< IsiSensor_t.pszName */
        0,                      /**< IsiSensor_t.pRegisterTable */
        0,                      /**< IsiSensor_t.pIsiSensorCaps */
        0,						/**< IsiSensor_t.pIsiGetSensorIsiVer_t>*/   //oyyf add
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
		0,
		0,						/**< IsiSensor_t.pIsiGetColorIss */
    },
    S5K4H8_IsiGetSensorI2cInfo,
};


