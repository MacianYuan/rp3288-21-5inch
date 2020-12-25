#ifndef __GC2145_MIPI_PRI_H__
#define __GC2145_MIPI_PRI_H__
/*
*              CAMSYS_TEST DRIVER VERSION NOTE
*
*v0.1.0 : gc2145 driver ok, can preview.
*
*/
#define Camsys_Teset_Driver_Version		"Camsys_Test_GC0329_v(0x0,0,1.0)"
#include "camera_test.h"

int Gc0329_sensor_reg_init(int camsys_fd,unsigned int *i2cbase);
int Gc0329_sensor_streamon(int camsys_fd,unsigned int on);
int Gc0329_get_SensorInfo(rk_camera_info_t *rk_camera_info);

#endif

