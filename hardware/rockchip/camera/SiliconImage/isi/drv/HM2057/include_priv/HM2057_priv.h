#ifndef __HM2057_PRIV_H__
#define __HM2057_PRIV_H__

#include <ebase/types.h>
#include <common/return_codes.h>
#include <hal/hal_api.h>



#ifdef __cplusplus
extern "C"
{
#endif

/*v0.1.0:
*   1).add senosr drv version in get sensor i2c info func
*v0.2.0:
*   1). support for isi v0.5.0
*v0.3.0
*   1). support for isi v0.6.0
*v0.4.0
*   1). support for isi v0.7.0
*v0.5.0
*   1). support for isi v0.0xc.0
*   2). change VPol from ISI_VPOL_NEG to ISI_VPOL_POS
*v0.6.0
*   1). support isi v0.0xd.0
*/
#define CONFIG_SENSOR_DRV_VERSION KERNEL_VERSION(0, 6, 0) 


#define HM2057_DELAY_5MS                    (0x0000) //delay 5 ms
#define HM2057_MODE_SELECT                  (0x0100) // rw - Bit[7:1]not used  Bit[0]Streaming set 0: software_standby  1: streaming       
#define HM2057_SOFTWARE_RST                 (0x0022) //(0x0103) // rw - Bit[7:1]not used  Bit[0]software_reset

#define HM2057_CHIP_ID_HIGH_BYTE_DEFAULT            (0x20) // r - 
#define HM2057_CHIP_ID_MIDDLE_BYTE_DEFAULT          (0x56) // r - 
#define HM2057_CHIP_ID_LOW_BYTE_DEFAULT             (0x19) // r - 

#define HM2057_CHIP_ID_HIGH_BYTE            (0x0001) // r - 
#define HM2057_CHIP_ID_MIDDLE_BYTE          (0x0002) // r - 
#define HM2057_CHIP_ID_LOW_BYTE             (0x0003) // r - 
/*****************************************************************************
* Further defines for driver management
*****************************************************************************/
#define HM2057_DRIVER_INIT              (0x00000001)

/*****************************************************************************
 * ov14825 context structure
 *****************************************************************************/
typedef struct HM2057_Context_s
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
} HM2057_Context_t;

#ifdef __cplusplus
}
#endif

#endif

