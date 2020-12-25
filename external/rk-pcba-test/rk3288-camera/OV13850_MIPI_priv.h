#ifndef __OV13850_H__
#define __OV13850_H__
/*
*              CAMSYS_TEST DRIVER VERSION NOTE
*
*v0.1.0 : ov13850 driver ok, can preview.
*
*/
#define Camsys_Teset_Driver_Version		"Camsys_Test_OV13850_v(0x0,0,1.0)"
#include "camera_test.h"

int Ov13850_sensor_reg_init(int camsys_fd,unsigned int *i2cbase);
int Ov13850_sensor_streamon(int camsys_fd,unsigned int on);
int Ov13850_get_SensorInfo(rk_camera_info_t *rk_camera_info);

#endif

