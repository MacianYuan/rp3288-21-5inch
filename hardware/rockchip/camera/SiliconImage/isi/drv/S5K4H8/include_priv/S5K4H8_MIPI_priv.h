#ifndef __S5K4H8_PRIV_H__
#define __S5K4H8_PRIV_H__

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
*v0.1.0x00 : create file 2017.02.22
*v0.2.0x00 : support mipi four lanes and two lanes.
*v0.3.0x00 : mipi_speed_560Mbps_preview_1632x1224_30fps_capture_3264x2448_24fps
*/


#define CONFIG_SENSOR_DRV_VERSION  KERNEL_VERSION(0, 3, 0)

//#define FOUR_LANE_700M
//#define FOUR_LANE_700M_to_672M   //reg:0x030e  val:0x00a8

#define FOUR_LANE_560M

/*****************************************************************************
 * System control registers
 *****************************************************************************/
#define S5K4H8_MODE_SELECT                  (0x0100)
#define S5K4H8_MODE_SELECT_OFF              (0x00U)
#define S5K4H8_MODE_SELECT_ON				(0x01U)

#define S5K4H8_SOFTWARE_RST                 (0x0103)
#define S5K4H8_SOFTWARE_RST_VALUE			(0x01)

#define S5K4H8_CHIP_ID_HIGH_BYTE            (0x0000)
#define S5K4H8_CHIP_ID_MIDDLE_BYTE          (0x0000)
#define S5K4H8_CHIP_ID_LOW_BYTE             (0x0001)
#define S5K4H8_CHIP_ID_HIGH_BYTE_DEFAULT    (0x40)
#define S5K4H8_CHIP_ID_MIDDLE_BYTE_DEFAULT  (0x40)
#define S5K4H8_CHIP_ID_LOW_BYTE_DEFAULT     (0x88)

#define S5K4H8_AEC_AGC_ADJ_H                  (0x0204)
#define S5K4H8_AEC_AGC_ADJ_L                  (0x0205)

#define S5K4H8_AEC_EXPO_FINE                (0x0200)
#define S5K4H8_AEC_EXPO_COARSE              (0x0202)


/*****************************************************************************
 * ov14825 context structure
 *****************************************************************************/
typedef struct S5K4H8_VcmInfo_s                 /* ddl@rock-chips.com: v0.3.0 */
{
    uint32_t StartCurrent;
    uint32_t RatedCurrent;
    uint32_t Step;
    uint32_t StepMode;
} S5K4H8_VcmInfo_t;
typedef struct S5K4H8_Context_s
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
	S5K4H8_VcmInfo_t    VcmInfo;
	uint32_t			preview_minimum_framerate;
} S5K4H8_Context_t;

#ifdef __cplusplus
}
#endif

#endif
