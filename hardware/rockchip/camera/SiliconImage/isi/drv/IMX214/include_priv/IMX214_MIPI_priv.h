#ifndef __IMX214_PRIV_H__
#define __IMX214_PRIV_H__

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
*v0.1.0 : create file -- oyyf
*v0.2.0 : correct 4lane demo mistakes, but 4lane still not work
*v0.3.0 : change imx214 2lane register setting from external to internal 
*v0.4.0 : fix exposure and awb capture in setting -- oyyf
*v0.5.0 : add sensor_version_get_p callback func param in check_read_otp func.
*v0.6.0
*   1). support for isi v0.0xc.0
*   2). change VPol from ISI_VPOL_NEG to ISI_VPOL_POS
*v0.7.0
*   1). ID Address: 0x1617 ID_num: 0x214
*   2). don't Check Sensor Connection again
*v0.8.0:
*   1). support isi v0.0xd.0
*v0.9.0:
*   1). limit frame rate.
*v0.a.0:
*   1).gain change to 16.
*v0.b.0:
*   1).change usFrameLengthLines.
*v0.0xe.0 :
*   1). change judgment condition of applying OTP data.
*/

#define CONFIG_SENSOR_DRV_VERSION KERNEL_VERSION(0, 0xe, 0x00)


#define Sensor_CHIP_ID_HIGH_BYTE            (0x0016) // r -
#define Sensor_CHIP_ID_LOW_BYTE          (0x0017) // r -

#define Sensor_CHIP_ID_HIGH_BYTE_DEFAULT            (0x02) // r -
#define Sensor_CHIP_ID_LOW_BYTE_DEFAULT          (0x14) // r -

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


