#ifndef __OV8858_H__
#define __OV8858_H__
/*
*              CAMSYS_TEST DRIVER VERSION NOTE
*
*v0.1.0 : ov8858 driver ok, can preview.
*
*/
#define Camsys_Teset_Driver_Version		"Camsys_Test_OV8858_v(0x0,0,1.0)"
#include "camera_test.h"

int Ov8858_sensor_reg_init(int camsys_fd,unsigned int *i2cbase);
int Ov8858_sensor_streamon(int camsys_fd,unsigned int on);
int Ov8858_get_SensorInfo(rk_camera_info_t *rk_camera_info);

#endif

