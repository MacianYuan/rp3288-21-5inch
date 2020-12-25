#ifndef __TC358749XBG_PRIV_H__
#define __TC358749XBG_PRIV_H__

#include <ebase/types.h>
#include <common/return_codes.h>
#include <hal/hal_api.h>

/*
*v0.1.0x00 : Create file;
*v0.2.0x00 : change 749's drive capability to avoid pic size error;
*v0.3.0x00 :
*	1) add the state machine mechanism;
*	2) add supported resolution: 1080P30, 1080I60, 576P/I50, 480P/I60;
*/
#define CONFIG_SENSOR_DRV_VERSION KERNEL_VERSION(0, 3, 0)



#ifdef __cplusplus
extern "C"
{
#endif

#define TC358749XBG_DELAY_5MS                    (0x0000) //delay 5 ms
#define TC358749XBG_MODE_SELECT                  (0x0100) // rw - Bit[7:1]not used  Bit[0]Streaming set 0: software_standby  1: streaming       
#define TC358749XBG_SOFTWARE_RST                 (0x7080) // rw - Bit[7:1]not used  Bit[0]software_reset

#define TC358749XBG_CHIP_ID_HIGH_BYTE_DEFAULT         (0x0147)//  (0x0147) // r - 
#define TC358749XBG_CHIP_ID_MIDDLE_BYTE_DEFAULT          (0x0081) // r - 
#define TC358749XBG_CHIP_ID_LOW_BYTE_DEFAULT             (0x0000) // r - 

#define TC358749XBG_CHIP_ID_HIGH_BYTE            (0x0000) // r - 
#define TC358749XBG_CHIP_ID_MIDDLE_BYTE          (0x0002) // r - 
#define TC358749XBG_CHIP_ID_LOW_BYTE             (0x0004) // r - 
/*****************************************************************************
* Further defines for driver management
*****************************************************************************/
#define TC358749XBG_DRIVER_INIT              (0x00000001)

/*****************************************************************************
 *context structure
 *****************************************************************************/
typedef struct TC358749XBG_Context_s
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
} TC358749XBG_Context_t;

#ifdef __cplusplus
}
#endif

#endif

