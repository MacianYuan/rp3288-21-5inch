#ifndef __OV5648_PRIV_H__
#define __OV5648_PRIV_H__

#include <ebase/types.h>
#include <common/return_codes.h>
#include <hal/hal_api.h>



#ifdef __cplusplus
extern "C"
{
#endif

/*
*              SILICONIMAGE LIBISP VERSION NOTE
*
*v0.1.0 : create file -- zyl
*v0.2.0 : modify ov5648 driver --oyyf
*v0.3.0 : support OTP
*v0.4.0 : support OTP i2c info
*v0.5.0 : support MIPI 210Mbps two lanes 20fps preview and 7.5fps capture settings.
*v0.6.0 : support MIPI 420Mbps one lane.
*v0.7.0
	1) sensor OTP data application can enable/disable by property setting.
	2) typical OTP rg/bg value moved to IQ file.
*v0.8.0
*   1). support for isi v0.0xc.0
*   2). change VPol from ISI_VPOL_NEG to ISI_VPOL_POS
*v0.9.0
*   1). support isi v0.0xd.0
*v0.0xa.0 :
*   1). change judgment condition of applying OTP data.
*/

#define CONFIG_SENSOR_DRV_VERSION KERNEL_VERSION(0, 0xa, 0x00) 

//#define MIPI_210MBPS

#define Sensor_CHIP_ID_HIGH_BYTE            (0x300a) // r - 
#define Sensor_CHIP_ID_LOW_BYTE          (0x300b) // r - 

#define Sensor_CHIP_ID_HIGH_BYTE_DEFAULT            (0x56) // r - 
#define Sensor_CHIP_ID_LOW_BYTE_DEFAULT          (0x48) // r - 

#define Sensor_MODE_SELECT  (0x0100)

#define Sensor_SOFTWARE_RST                 (0x0103) // rw - Bit[7:1]not used  Bit[0]software_reset

typedef struct Sensor_VcmInfo_s                 /* ddl@rock-chips.com: v0.3.0 */
{
    uint32_t StartCurrent;
    uint32_t RatedCurrent;
    uint32_t Step;
    uint32_t StepMode;
} Sensor_VcmInfo_t;

typedef struct Sensor_Context_s
{
    IsiSensorContext_t  IsiCtx;                 /**< common context of ISI and ISI driver layer; @note: MUST BE FIRST IN DRIVER CONTEXT */

    //// modify below here ////

    IsiSensorConfig_t   Config;                 /**< sensor configuration */
    bool_t              Configured;             /**< flags that config was applied to sensor */
    bool_t              Streaming;              /**< flags that sensor is streaming data */
    bool_t              TestPattern;            /**< flags that sensor is streaming test-pattern */

    bool_t              isAfpsRun;              /**< if true, just do anything required for Afps parameter calculation, but DON'T access SensorHW! */

    bool_t              GroupHold;

    float               VtPixClkFreq;           /**< pixel clock */
    uint16_t            LineLengthPck;          /**< line length with blanking */
    uint16_t            FrameLengthLines;       /**< frame line length */

    float               AecMaxGain;
    float               AecMinGain;
    float               AecMaxIntegrationTime;
    float               AecMinIntegrationTime;

    float               AecIntegrationTimeIncrement; /**< _smallest_ increment the sensor/driver can handle (e.g. used for sliders in the application) */
    float               AecGainIncrement;            /**< _smallest_ increment the sensor/driver can handle (e.g. used for sliders in the application) */

    float               AecCurGain;
    float               AecCurIntegrationTime;

    uint16_t            OldGain;               /**< gain multiplier */
    uint32_t            OldCoarseIntegrationTime;
    uint32_t            OldFineIntegrationTime;

    IsiSensorMipiInfo   IsiSensorMipiInfo;
	Sensor_VcmInfo_t    VcmInfo;
	uint32_t			preview_minimum_framerate;
} Sensor_Context_t;

#ifdef __cplusplus
}
#endif

#endif

