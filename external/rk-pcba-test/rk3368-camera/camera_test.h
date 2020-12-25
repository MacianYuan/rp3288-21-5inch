#ifndef __CAMERA_TEST_H__
#define __CAMERA_TEST_H__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <string.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>
//#include <linux/ion.h>
//#include "../ion.h"
//#include "../../../system/core/libion/kernel-headers/linux/rockchip_ion.h"
#include "rockchip_ion.h"

#include "camsys_head.h"
#include "mrv_all_regs.h"

#define BACK_SENSOR_TYPE "MIPI"
#define FRONT_SENSOR_TYPE "CIF"
#define BACK_SENSOR_NAME "ov8858"
#define FRONT_SENSOR_NAME "gc2145"  //gc2145

#define BACK_SENSOR_PWRDN_PIN "RK30_PIN1_PB5"
#define BACK_SENSOR_PWREN_PIN "RK30_PIN1_PA0"
#define BACK_SENSOR_RESET_PIN "RK30_PIN0_PD4"

#define FRONT_SENSOR_PWRDN_PIN "RK30_PIN1_PB4"
#define FRONT_SENSOR_PWREN_PIN "RK30_PIN1_PA0"
#define FRONT_SENSOR_RESET_PIN "RK30_PIN0_PD4"

//#include <linux/android_pmem.h>
#include <linux/videodev2.h>
#include <linux/fb.h>
#include <linux/version.h>
#include "../test_case.h"

//hkw add
//#include"../../../hardware/rockchip/camera/SiliconImage/include/camsys_head.h"
#include "../Language/language.h"
//#include "../language.h"
// the func is a while loop func , MUST  run in a single thread.
//return value: 0 is ok ,-1 erro

#define HPOL_HIGH 	0x0
#define HPOL_LOW 	0x1
#define VPOL_HIGH 	0x0
#define VPOL_LOW 	0x1
#define SAMPLEEDGE_NEG 	0x0
#define SAMPLEEDGE_POS 	0x1

#define RGRGGBGB 	0x0
#define GRGRBGBG 	0x1
#define GBGBRGRG 	0x2
#define BGBGGRGR 	0x3

#define Y0Cb0Cr0 	0x0 //Y0Cb0Cr0
#define Y0Cb0Y1Cr1 	0x1 //Y0Cb0 ¡§C Y1Cr1 (not recommended) 
#define Y0CbY1Cr 	0x2 //Y0Cb(0+1)/2 ¡§C Y1Cr(0+1)/2
#define RESERVERD 	0x3

#define YCbYCr 		0x0 
#define YCrYCb 		0x1 
#define CbYCrY 		0x2
#define CrYCbY	 	0x3

#define RAW_PIC		0x0// 321:  	000: raw picture
#define CCIR656		0x1//		 001: ccir656
#define CCIR601 	0x2//		 010: ccir601
#define RGB_BAYER	0x3//		 011: bayer rgb
#define DATA_MODE	0x4//		 100: data mode

typedef unsigned int         uint32_t;

typedef struct rk_camera_info_s {
	camsys_phy_type_t	phy_type;
	//mipi info
	uint32_t    lane_num;
    uint32_t	bit_rate;           // Mbps/lane
    uint32_t	phy_index;          // phy0,phy1
    uint32_t	mipi_img_data_sel;	//2a-RAW8  2B-RAW10  2c-RAW12  
	//cif info
    uint32_t	cif_num; 
    camsys_fmt_t	fmt;
    camsys_cifio_t	cifio;

	//sensor info
	uint32_t	width;				/**< vertical size */
	uint32_t	height;				/**< horizontal size */
    uint32_t	Mode;               /**< supported operating modes */
    uint32_t	YCSequence;
    uint32_t	Conv422;
    uint32_t	BPat;               /**< bayer pattern */
    uint32_t	HPol;               /**< horizontal polarity */
    uint32_t	VPol;               /**< vertical polarity */
    uint32_t	Edge;               /**< sample edge */
    uint32_t	SmiaMode;
    uint32_t	MipiMode;
	uint32_t	SensorOutputMode;
	uint32_t    dev_id;
	unsigned char     sensorname[CAMSYS_NAME_LEN];
	unsigned char     version[CAMSYS_NAME_LEN];
}rk_camera_info_t;

struct camera_msg {
	struct testcase_info *tc_info;
	int result;
	int id;
	int x;
	int y;
	int w;
	int h;
};

extern void* camera_test(void *argc);
//return value: 0 is ok ,-1 erro
extern int stopCameraTest();
extern void finishCameraTest();
extern int Camera_Click_Event(int x,int y);
extern int startCameraTest();
#endif
