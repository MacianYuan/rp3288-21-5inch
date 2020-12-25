#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


#include "OV5648_MIPI_priv.h"
#include "camsys_head.h"
#include "camera_test.h"

//==================================
//sensor setting
//==================================
#define SENSOR_I2C_ADDR					0x6c

#define REG_STREAM_ON					0x0100
#define REG_SOFTWARE_RST				0x0103
#define REG_SOFTWARE_RST_DATA				0x01
#define I2C_NR_ADR_BYTES				2
#define I2C_NR_DAT_BYTES				0x01

#define REG_CHIP_ID_H					0x300a
#define REG_CHIP_ID_L					0x300b

//=================================
#define SENSOR_I2C_NUM					1
#define SENSOR_I2C_RATE					100000
struct rk_sensor_reg {
	unsigned int reg;
	unsigned int val;
};


struct rk_sensor_reg Ov5648_sensor_test[] = {
	{0x0100, 0x00},
	{0x0100, 0x00},
	{0x0100, 0x00},
	{0x3001, 0x00},
	{0x3002, 0x00},
	{0x3011, 0x02},
	{0x3017, 0x05},
	{0x3018, 0x4c},
	{0x301c, 0xd2},
	{0x3022, 0x00},
	{0x3034, 0x1a},
	{0x3035, 0x21},
	{0x3036, 0x69},
	{0x3037, 0x03},
	{0x3038, 0x00},
	{0x3039, 0x00},
	{0x303a, 0x00},
	{0x303b, 0x19},
	{0x303c, 0x11},
	{0x303d, 0x30},
	{0x3105, 0x11},
	{0x3106, 0x05},
	{0x3304, 0x28},
	{0x3305, 0x41},
	{0x3306, 0x30},
	{0x3308, 0x00},
	{0x3309, 0xc8},
	{0x330a, 0x01},
	{0x330b, 0x90},
	{0x330c, 0x02},
	{0x330d, 0x58},
	{0x330e, 0x03},
	{0x330f, 0x20},
	{0x3300, 0x00},
	{0x3503, 0x07},
	{0x3601, 0x33},
	{0x3602, 0x00},
	{0x3611, 0x0e},
	{0x3612, 0x2b},
	{0x3614, 0x50},
	{0x3620, 0x33},
	{0x3622, 0x00},
	{0x3630, 0xad},
	{0x3631, 0x00},
	{0x3632, 0x94},
	{0x3633, 0x17},
	{0x3634, 0x14},
	{0x3704, 0xc0},
	{0x3705, 0x2a},
	{0x3708, 0x66},
	{0x3709, 0x52},
	{0x370b, 0x23},
	{0x370c, 0xcf},
	{0x370d, 0x00},
	{0x370e, 0x00},
	{0x371c, 0x07},
	{0x3739, 0xd2},
	{0x373c, 0x00},
	{0x3800, 0x00},
	{0x3801, 0x00},
	{0x3802, 0x00},
	{0x3803, 0x00},
	{0x3804, 0x0a},
	{0x3805, 0x3f},
	{0x3806, 0x07},
	{0x3807, 0xa3},
	{0x3808, 0x05},
	{0x3809, 0x10},
	{0x380a, 0x03},
	{0x380b, 0xcc},
	{0x380c, 0x0b},
	{0x380d, 0x00},
	{0x380e, 0x03},
	{0x380f, 0xe0},
	{0x3810, 0x00},
	{0x3811, 0x08},
	{0x3812, 0x00},
	{0x3813, 0x04},
	{0x3814, 0x31},
	{0x3815, 0x31},
	{0x3817, 0x00},
	{0x3820, 0x08},
	{0x3821, 0x07},
	{0x3826, 0x03},
	{0x3829, 0x00},
	{0x382b, 0x0b},
	{0x3830, 0x00},
	{0x3836, 0x00},
	{0x3837, 0x00},
	{0x3838, 0x00},
	{0x3839, 0x04},
	{0x383a, 0x00},
	{0x383b, 0x01},
	{0x3b00, 0x00},
	{0x3b02, 0x08},
	{0x3b03, 0x00},
	{0x3b04, 0x04},
	{0x3b05, 0x00},
	{0x3b06, 0x04},
	{0x3b07, 0x08},
	{0x3b08, 0x00},
	{0x3b09, 0x02},
	{0x3b0a, 0x04},
	{0x3b0b, 0x00},
	{0x3b0c, 0x3d},
	{0x3f01, 0x0d},
	{0x3f0f, 0xf5},
	{0x4000, 0x89},
	{0x4001, 0x02},
	{0x4002, 0x45},
	{0x4004, 0x02},
	{0x4005, 0x18},
	{0x4006, 0x08},
	{0x4007, 0x10},
	{0x4008, 0x00},
	{0x4050, 0x6e},
	{0x4051, 0x8f},
	{0x4300, 0xf8},
	{0x4303, 0xff},
	{0x4304, 0x00},
	{0x4307, 0xff},
	{0x4520, 0x00},
	{0x4521, 0x00},
	{0x4511, 0x22},
	{0x4801, 0x0f},
	{0x4814, 0x2a},
	{0x481f, 0x3c},
	{0x4823, 0x3c},
	{0x4826, 0x00},
	{0x481b, 0x3c},
	{0x4827, 0x32},
	{0x4837, 0x18},
	{0x4b00, 0x06},
	{0x4b01, 0x0a},
	{0x4b04, 0x10},
	{0x5000, 0xff},
	{0x5001, 0x00},
	{0x5002, 0x41},
	{0x5003, 0x0a},
	{0x5004, 0x00},
	{0x5043, 0x00},
	{0x5013, 0x00},
	{0x501f, 0x03},
	{0x503d, 0x00},
	{0x5780, 0xfc},
	{0x5781, 0x1f},
	{0x5782, 0x03},
	{0x5786, 0x20},
	{0x5787, 0x40},
	{0x5788, 0x08},
	{0x5789, 0x08},
	{0x578a, 0x02},
	{0x578b, 0x01},
	{0x578c, 0x01},
	{0x578d, 0x0c},
	{0x578e, 0x02},
	{0x578f, 0x01},
	{0x5790, 0x01},
	{0x5a00, 0x08},
	{0x5b00, 0x01},
	{0x5b01, 0x40},
	{0x5b02, 0x00},
	{0x5b03, 0xf0},
	//{0x0000 ,0x00},

	{0x3501, 0x3d},
	{0x3502, 0x00},

	{0x3708, 0x66},
	{0x3709, 0x52},
	{0x370c, 0xcf},
	{0x3800, 0x00}, // xstart = 0
	{0x3801, 0x00}, // x start
	{0x3802, 0x00}, // y start = 0
	{0x3803, 0x00}, // y start
	{0x3804, 0x0a}, // xend = 2623
	{0x3805, 0x3f}, // xend
	{0x3806, 0x07}, // yend = 1955
	{0x3807, 0xa3}, // yend
	{0x3808, 0x05}, // x output size = 1296
	{0x3809, 0x10}, // x output size
	{0x380a, 0x03}, // y output size = 972
	{0x380b, 0xcc}, // y output size
	{0x380c, 0x0b},
	{0x380d, 0x00},
	{0x380e, 0x03},
	{0x380f, 0xe0},
	{0x3810, 0x00}, // isp x win = 8
	{0x3811, 0x08}, // isp x win
	{0x3812, 0x00}, // isp y win = 4
	{0x3813, 0x04}, // isp y win
	{0x3814, 0x31}, // x inc
	{0x3815, 0x31}, // y inc
	{0x3817, 0x00}, // hsync start
	{0x3820, 0x08}, // flip off, v bin off
	{0x3821, 0x07}, // mirror on, h bin on
	{0x4004, 0x02}, // black line number
	{0x4005, 0x18}, // blc level trigger
	{0x4837, 0x17}, // MIPI global timing
//	{0x0000 ,0x00}
};


int Ov5648_sensor_reg_init(int camsys_fd,unsigned int *i2cbase)
{
    int err,i2cbytes,i;
    struct rk_sensor_reg *sensor_reg;
    unsigned char *i2cchar;
    camsys_sysctrl_t sysctl;    
    camsys_i2c_info_t i2cinfo;
    int id;
    int size;

    i2cinfo.bus_num = SENSOR_I2C_NUM;
    i2cinfo.slave_addr = SENSOR_I2C_ADDR;
    i2cinfo.reg_addr = REG_SOFTWARE_RST;
    i2cinfo.reg_size = I2C_NR_ADR_BYTES; 
    i2cinfo.val = REG_SOFTWARE_RST_DATA;
    i2cinfo.val_size = I2C_NR_DAT_BYTES;
    i2cinfo.i2cbuf_directly = 0;
    i2cinfo.speed = SENSOR_I2C_RATE;
/*
	i2cinfo.reg_addr = REG_STREAM_ON;
	i2cinfo.val = 0;
	err = ioctl(camsys_fd, CAMSYS_I2CWR, &i2cinfo);
	if (err<0) {
		printf("CAMSYS_I2CWR failed\n");
	} else {
		printf("I2c write: 0x%x : 0x%x\n",i2cinfo.reg_addr,i2cinfo.val);
	}

	i2cinfo.reg_addr = REG_SOFTWARE_RST;
	i2cinfo.val = REG_SOFTWARE_RST_DATA;
*/
    err = ioctl(camsys_fd, CAMSYS_I2CWR, &i2cinfo);
    if (err<0) {
        printf("CAMSYS_I2CWR failed\n");
    } else {
        printf("I2c write: 0x%x : 0x%x\n",i2cinfo.reg_addr,i2cinfo.val);
    }
	printf("%s %d  hcc\n",__FUNCTION__,__LINE__);  

    usleep(20000);    
    
    i2cinfo.reg_addr = REG_CHIP_ID_H;
    i2cinfo.val_size = 0x01;       
    err = ioctl(camsys_fd, CAMSYS_I2CRD, &i2cinfo);
    if (err<0) {
        printf("CAMSYS_I2CRD failed\n");
    } else {
        printf("I2c read: 0x%x : 0x%x\n",i2cinfo.reg_addr,i2cinfo.val);
        id = (i2cinfo.val<<8);
    }
  
    i2cinfo.reg_addr = REG_CHIP_ID_L;
    err = ioctl(camsys_fd, CAMSYS_I2CRD, &i2cinfo);
    if (err<0) {
        printf("CAMSYS_I2CRD failed\n");
    } else {
        printf("I2c read: 0x%x : 0x%x\n",i2cinfo.reg_addr,i2cinfo.val);
        id |= i2cinfo.val;
    }

    printf("\n!!!!!!!!!!Sensor ID: 0x%x!!!!!!!!!!\n",id);
    
    i2cchar = (unsigned char*)i2cbase;

	sensor_reg = Ov5648_sensor_test;
	size = sizeof(Ov5648_sensor_test)/sizeof(struct rk_sensor_reg);

    i2cbytes = 0x00;
    for (i=0; i<size; i++) {
        *i2cchar++ = (sensor_reg->reg&0xff00)>>8; 
        *i2cchar++ = (sensor_reg->reg&0xff);
        *i2cchar++ = (sensor_reg->val&0xff);
        sensor_reg++;
        i2cbytes += 3;
    }
    printf("i2cbytes(%d)\n",i2cbytes);

    i2cinfo.bus_num = SENSOR_I2C_NUM;
    i2cinfo.slave_addr = SENSOR_I2C_ADDR;
    i2cinfo.i2cbuf_directly = 1;
    i2cinfo.i2cbuf_bytes = ((3<<16)|i2cbytes);
    i2cinfo.speed = SENSOR_I2C_RATE;
    err = ioctl(camsys_fd, CAMSYS_I2CWR, &i2cinfo);
    if (err<0) {
        printf("CAMSYS_I2CWR buf failed\n"); 
    }
    return err;
}
int Ov5648_sensor_streamon(int camsys_fd,unsigned int on)
{
    int err,i2cbytes,i;
    struct rk_sensor_reg *sensor_reg;
    camsys_sysctrl_t sysctl;    
    camsys_i2c_info_t i2cinfo;
    int id;

    i2cinfo.bus_num = SENSOR_I2C_NUM;
    i2cinfo.slave_addr = SENSOR_I2C_ADDR;
    i2cinfo.reg_addr = REG_STREAM_ON;
    i2cinfo.reg_size = 2; 
    i2cinfo.val = on;
    i2cinfo.val_size = I2C_NR_DAT_BYTES;
    i2cinfo.i2cbuf_directly = 0;
    i2cinfo.speed = SENSOR_I2C_RATE;
       
    err = ioctl(camsys_fd, CAMSYS_I2CWR, &i2cinfo);
    if (err < 0) {
        printf("extdev_streamon failed!\n");
    }
    printf("%s(%d): Sensor stream on : %d\n", __func__, __LINE__, on);
    
    return err;
}

int Ov5648_get_SensorInfo(rk_camera_info_t *rk_camera_info)
{
	//rk_camera_info_t rk_camera_info;
	//rk_camera_info->Camsys_Teset_Driver_Version = "Camsys_Test_OV5648_v(0x0,0,1.0)";
	//rk_camera_info->sensor_name     = "OV5648";

	rk_camera_info->phy_type		= CamSys_Phy_Mipi; //cif:CamSys_Phy_Cif mipi:CamSys_Phy_Mipi
	rk_camera_info->lane_num		= 2; //values:1,2,4
	rk_camera_info->bit_rate		= 328; //lane_num(1):720, lane_num(2):328, lane_num(4):408
	rk_camera_info->phy_index		= 0x0; //Rx/Tx:0x1 Rx:0x0
	rk_camera_info->mipi_img_data_sel = 0x2b; //cif:0x2c mipi:0x2b
	rk_camera_info->cif_num			= 0;
	rk_camera_info->fmt				= CamSys_Fmt_Raw_10b;
	rk_camera_info->cifio			= CamSys_SensorBit0_CifBit4;

	rk_camera_info->width			= 1296;
	rk_camera_info->height			= 972;

	rk_camera_info->Mode			= RGB_BAYER;
	rk_camera_info->YCSequence		= CbYCrY;		   
	rk_camera_info->Conv422 		= Y0Cb0Y1Cr1;
	rk_camera_info->BPat			= BGBGGRGR ;
	rk_camera_info->HPol			= HPOL_HIGH;
	rk_camera_info->VPol			= VPOL_LOW;
	rk_camera_info->Edge			= SAMPLEEDGE_POS;
	//rk_camera_info->SmiaMode		= ISI_SMIA_OFF;
	//rk_camera_info->MipiMode		= ISI_MIPI_OFF;
	//rk_camera_info->SensorOutputMode = ISI_SENSOR_OUTPUT_MODE_YUV;

    rk_camera_info->dev_id = CAMSYS_DEVID_SENSOR_1B; //back:CAMSYS_DEVID_SENSOR_1A front:CAMSYS_DEVID_SENSOR_1B

	strlcpy((char*)rk_camera_info->sensorname, "OV5648",sizeof(rk_camera_info->sensorname));
	strlcpy((char*)rk_camera_info->version, "0x1.0.0",sizeof(rk_camera_info->version));
	return 0;
}

