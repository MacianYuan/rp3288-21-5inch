#ifndef __SP2509V_PRIV_H__
#define __SP2509V_PRIV_H__

#include <ebase/types.h>
#include <common/return_codes.h>
#include <hal/hal_api.h>



#ifdef __cplusplus
extern "C"
{
#endif

/*v0.1.0:
*   1).init version
*/
#define CONFIG_SENSOR_DRV_VERSION KERNEL_VERSION(0, 1, 0) 

#define Sensor_MODE_SELECT                  (0xac)
#define Sensor_PAGE_SELECT                  (0xfd)
#define Sensor_SOFTWARE_RST                 (0xfd)//treat page select register as 'reset' register,ugly...but easy for me...
#define Sensor_SOFTWARE_RST_DATA			(0x00)

#define Sensor_CHIP_ID_HIGH_BYTE_DEFAULT    (0x25)
#define Sensor_CHIP_ID_MIDDLE_BYTE_DEFAULT  (0x25)
#define Sensor_CHIP_ID_LOW_BYTE_DEFAULT     (0x09)

#define Sensor_CHIP_ID_HIGH_BYTE            (0x02)
#define Sensor_CHIP_ID_MIDDLE_BYTE          (0x02)
#define Sensor_CHIP_ID_LOW_BYTE             (0x03)

typedef struct Sensor_VcmInfo_s
{
    uint32_t StartCurrent;
    uint32_t RatedCurrent;
    uint32_t Step;
    uint32_t StepMode;
} Sensor_VcmInfo_t;

typedef struct Sensor_Context
{
    IsiSensorContext_t  IsiCtx;                 /**< common context of ISI and ISI driver layer; @note: MUST BE FIRST IN DRIVER CONTEXT */
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

