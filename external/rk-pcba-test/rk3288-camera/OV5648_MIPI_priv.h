#ifndef __OV5648_H__
#define __OV5648_H__
/*
*              CAMSYS_TEST DRIVER VERSION NOTE
*
*v0.1.0 : ov5648 driver ok, can preview.
*
*/
#define Camsys_Teset_Driver_Version		"Camsys_Test_OV5648_v(0x0,0,1.0)"
#include "camera_test.h"

int Ov5648_sensor_reg_init(int camsys_fd,unsigned int *i2cbase);
int Ov5648_sensor_streamon(int camsys_fd,unsigned int on);
int Ov5648_get_SensorInfo(rk_camera_info_t *rk_camera_info);

#endif

