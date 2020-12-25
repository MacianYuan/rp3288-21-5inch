#ifndef __OV2659_PRIV_H__
#define __OV2659_PRIV_H__

#include <ebase/types.h>
#include <common/return_codes.h>
#include <hal/hal_api.h>


/*
*              OV2659 DRIVER VERSION NOTE
*v0.1.0x00 : init ov2659 drv version.
*
*v0.2.0:
*   1).add senosr drv version in get sensor i2c info func
*v0.3.0:
*   1). support for isi v0.5.0
*v0.4.0
*   1). support for isi v0.7.0
*v0.5.0
*   1). fix cts verify green screen
*v0.6.0
*   1). fix green screen bug caused by 1600x1200 regs setting.
*v0.7.0
*   1). support for isi v0.0xc.0 
*   2). change VPol from ISI_VPOL_NEG to ISI_VPOL_POS
*v0.8.0
*   1). support isi v0.0xd.0
*/
#define CONFIG_SENSOR_DRV_VERSION KERNEL_VERSION(0, 8, 0) 

#ifdef __cplusplus
extern "C"
{
#endif

#define OV2659_DELAY_5MS                    (0x0000) //delay 5 ms
#define OV2659_MODE_SELECT                  (0x0100) // rw - Bit[7:1]not used  Bit[0]Streaming set 0: software_standby  1: streaming       
#define OV2659_SOFTWARE_RST                 (0x0103) // rw - Bit[7:1]not used  Bit[0]software_reset

#define OV2659_CHIP_ID_HIGH_BYTE_DEFAULT            (0x26) // r - 
#define OV2659_CHIP_ID_MIDDLE_BYTE_DEFAULT          (0x56) // r - 
#define OV2659_CHIP_ID_LOW_BYTE_DEFAULT             (0x00) // r - 

#define OV2659_CHIP_ID_HIGH_BYTE            (0x300a) // r - 
#define OV2659_CHIP_ID_MIDDLE_BYTE          (0x300b) // r - 
#define OV2659_CHIP_ID_LOW_BYTE             (0x300c) // r - 
/*****************************************************************************
* Further defines for driver management
*****************************************************************************/
#define OV2659_DRIVER_INIT              (0x00000001)

/*****************************************************************************
 * ov14825 context structure
 *****************************************************************************/
typedef struct OV2659_Context_s
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
	uint32_t			preview_minimum_framerate;

    IsiSensorMipiInfo   IsiSensorMipiInfo;
} OV2659_Context_t;

#ifdef __cplusplus
}
#endif

#endif

