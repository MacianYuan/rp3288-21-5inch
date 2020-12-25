//OV2680_tables.c
/*****************************************************************************/
/*!
 *  \file        OV2680_tables.c \n
 *  \version     1.0 \n
 *  \author      Meinicke \n
 *  \brief       Image-sensor-specific tables and other
 *               constant values/structures for OV13850. \n
 *
 *  \revision    $Revision: 803 $ \n
 *               $Author: $ \n
 *               $Date: 2010-02-26 16:35:22 +0100 (Fr, 26 Feb 2010) $ \n
 *               $Id: OV13850_tables.c 803 2010-02-26 15:35:22Z  $ \n
 */
/*  This is an unpublished work, the copyright in which vests in Silicon Image
 *  GmbH. The information contained herein is the property of Silicon Image GmbH
 *  and is supplied without liability for errors or omissions. No part may be
 *  reproduced or used expect as authorized by contract or other written
 *  permission. Copyright(c) Silicon Image GmbH, 2009, all rights reserved.
 */
/*****************************************************************************/
/*
#include "stdinc.h"

#if( OV2680_DRIVER_USAGE == USE_CAM_DRV_EN )
*/


#include <ebase/types.h>
#include <ebase/trace.h>
#include <ebase/builtins.h>

#include <common/return_codes.h>

#include "isi.h"
#include "isi_iss.h"
#include "isi_priv.h"
#include "OV2680_MIPI_priv.h"


/*****************************************************************************
 * DEFINES
 *****************************************************************************/


/*****************************************************************************
 * GLOBALS
 *****************************************************************************/

// Image sensor register settings default values taken from data sheet OV13850_DS_1.1_SiliconImage.pdf.
// The settings may be altered by the code in IsiSetupSensor.

//one lane
const IsiRegDescription_t Sensor_g_aRegDescription[] =
{
#if 1
	{0x0103, 0x01, "eReadWrite",eReadWrite},//software reset
	{0x3002, 0x00, "eReadWrite",eReadWrite},//gpio0 input, vsync input, fsin input
	{0x3016, 0x1c, "eReadWrite",eReadWrite},//drive strength = 0x01, bypass latch of hs_enable
	{0x3018, 0x44, "eReadWrite",eReadWrite},//MIPI 10-bit mode
	{0x3020, 0x00, "eReadWrite",eReadWrite},//output raw
	{0x3080, 0x02, "eReadWrite",eReadWrite},//PLL
	{0x3082, 0x37, "eReadWrite",eReadWrite},//PLL
	{0x3084, 0x09, "eReadWrite",eReadWrite},//PLL
	{0x3085, 0x04, "eReadWrite",eReadWrite},//PLL
	{0x3086, 0x01, "eReadWrite",eReadWrite},//PLL
	{0x3501, 0x26, "eReadWrite",eReadWrite},//exposure M
	{0x3502, 0x40, "eReadWrite",eReadWrite},//exposure L
	{0x3503, 0x03, "eReadWrite",eReadWrite},//vts auto, gain manual, exposure manual
	{0x350b, 0x36, "eReadWrite",eReadWrite},//gain L
	{0x3600, 0xb4, "eReadWrite",eReadWrite},//analog control
	{0x3603, 0x35, "eReadWrite",eReadWrite},//
	{0x3604, 0x24, "eReadWrite",eReadWrite},//
	{0x3605, 0x00, "eReadWrite",eReadWrite},//
	{0x3620, 0x26, "eReadWrite",eReadWrite},//
	{0x3621, 0x37, "eReadWrite",eReadWrite},//
	{0x3622, 0x04, "eReadWrite",eReadWrite},//
	{0x3628, 0x00, "eReadWrite",eReadWrite},// analog control
	{0x3705, 0x3c, "eReadWrite",eReadWrite},// sennsor control
	{0x370c, 0x50, "eReadWrite",eReadWrite},//
	{0x370d, 0xc0, "eReadWrite",eReadWrite},//
	{0x3718, 0x88, "eReadWrite",eReadWrite},//
	{0x3720, 0x00, "eReadWrite",eReadWrite},//
	{0x3721, 0x00, "eReadWrite",eReadWrite},//
	{0x3722, 0x00, "eReadWrite",eReadWrite},//
	{0x3723, 0x00, "eReadWrite",eReadWrite},//
	{0x3738, 0x00, "eReadWrite",eReadWrite},//
	{0x370a, 0x23, "eReadWrite",eReadWrite},//
	{0x3717, 0x58, "eReadWrite",eReadWrite},// sensor control
	{0x3781, 0x80, "eReadWrite",eReadWrite},// PSRAM
	{0x3784, 0x0c, "eReadWrite",eReadWrite},//
	{0x3789, 0x60, "eReadWrite",eReadWrite},// PSRAM
	{0x3800, 0x00, "eReadWrite",eReadWrite},// x start H
	{0x3801, 0x00, "eReadWrite",eReadWrite},// x start L
	{0x3802, 0x00, "eReadWrite",eReadWrite},// y start H
	{0x3803, 0x00, "eReadWrite",eReadWrite},// y start L
	{0x3804, 0x06, "eReadWrite",eReadWrite},// x end H
	{0x3805, 0x4f, "eReadWrite",eReadWrite},// x end L
	{0x3806, 0x04, "eReadWrite",eReadWrite},// y end H
	{0x3807, 0xbf, "eReadWrite",eReadWrite},// y end L
	{0x3808, 0x03, "eReadWrite",eReadWrite},// x output size H
	{0x3809, 0x20, "eReadWrite",eReadWrite},// x output size L
	{0x380a, 0x02, "eReadWrite",eReadWrite},// y output size H
	{0x380b, 0x58, "eReadWrite",eReadWrite},// y output size L
	{0x380c, 0x06, "eReadWrite",eReadWrite},// HTS H
	{0x380d, 0xac, "eReadWrite",eReadWrite},// HTS L
	{0x380e, 0x02, "eReadWrite",eReadWrite},// VTS H
	{0x380f, 0x84, "eReadWrite",eReadWrite},// VTS L
	{0x3810, 0x00, "eReadWrite",eReadWrite},// ISP x win H
	{0x3811, 0x04, "eReadWrite",eReadWrite},// ISP x win L
	{0x3812, 0x00, "eReadWrite",eReadWrite},// ISP y win H
	{0x3813, 0x04, "eReadWrite",eReadWrite},// ISP y win L
	{0x3814, 0x31, "eReadWrite",eReadWrite},// x inc
	{0x3815, 0x31, "eReadWrite",eReadWrite},// y inc
	{0x3819, 0x04, "eReadWrite",eReadWrite},// vsync end row
	{0x3820, 0xc2, "eReadWrite",eReadWrite},// vsun48_blc, vflip_blc, vbinf
	{0x3821, 0x01, "eReadWrite",eReadWrite},// hbin
	{0x4000, 0x81, "eReadWrite",eReadWrite},// avg_weight = 0x08, mf_en
	{0x4001, 0x40, "eReadWrite",eReadWrite},// format_trig_beh
	{0x4008, 0x00, "eReadWrite",eReadWrite},// blc_start
	{0x4009, 0x03, "eReadWrite",eReadWrite},// blc_end
	{0x4602, 0x02, "eReadWrite",eReadWrite},// frame reset enable
	{0x481f, 0x36, "eReadWrite",eReadWrite},// CLK PREPARE MIN
	{0x4825, 0x36, "eReadWrite",eReadWrite},// LPX P MIN
	{0x4837, 0x30, "eReadWrite",eReadWrite},// MIPI global timing
	{0x5002, 0x30, "eReadWrite",eReadWrite},//
	{0x5080, 0x00, "eReadWrite",eReadWrite},// test pattern off
	{0x5081, 0x41, "eReadWrite",eReadWrite},// window cut enable, random seed = 0x01
#else
	{0x0103, 0x01, "eReadWrite",eReadWrite}, // software reset
	{0x3002, 0x00, "eReadWrite",eReadWrite}, // gpio0 input, vsync input, fsin input
	{0x3016, 0x1c, "eReadWrite",eReadWrite}, // drive strength = 0x01, bypass latch of hs_enable
	{0x3018, 0x44, "eReadWrite",eReadWrite}, // MIPI 10-bit mode
	{0x3020, 0x00, "eReadWrite",eReadWrite}, // output raw
	{0x3080, 0x02, "eReadWrite",eReadWrite}, // PLL
	{0x3082, 0x37, "eReadWrite",eReadWrite}, // PLL
	{0x3084, 0x09, "eReadWrite",eReadWrite}, // PLL
	{0x3085, 0x04, "eReadWrite",eReadWrite}, // PLL
	{0x3086, 0x01, "eReadWrite",eReadWrite}, // PLL
	{0x3501, 0x26, "eReadWrite",eReadWrite}, // exposure M
	{0x3502, 0x40, "eReadWrite",eReadWrite}, // exposure L
	{0x3503, 0x03, "eReadWrite",eReadWrite}, // vts auto, gain manual, exposure manual
	{0x350b, 0x36, "eReadWrite",eReadWrite}, // gain L
	{0x3600, 0xb4, "eReadWrite",eReadWrite}, // analog control
	{0x3603, 0x35, "eReadWrite",eReadWrite}, //
	{0x3604, 0x24, "eReadWrite",eReadWrite}, //
	{0x3605, 0x00, "eReadWrite",eReadWrite}, //
	{0x3620, 0x26, "eReadWrite",eReadWrite}, //
	{0x3621, 0x37, "eReadWrite",eReadWrite}, //
	{0x3622, 0x04, "eReadWrite",eReadWrite}, //
	{0x3628, 0x00, "eReadWrite",eReadWrite}, // analog control
	{0x3705, 0x3c, "eReadWrite",eReadWrite}, // sennsor control
	{0x370c, 0x50, "eReadWrite",eReadWrite}, //
	{0x370d, 0xc0, "eReadWrite",eReadWrite}, //
	{0x3718, 0x88, "eReadWrite",eReadWrite}, //
	{0x3720, 0x00, "eReadWrite",eReadWrite}, //
	{0x3721, 0x00, "eReadWrite",eReadWrite}, //
	{0x3722, 0x00, "eReadWrite",eReadWrite}, //
	{0x3723, 0x00, "eReadWrite",eReadWrite}, //
	{0x3738, 0x00, "eReadWrite",eReadWrite}, //
	{0x370a, 0x23, "eReadWrite",eReadWrite}, //
	{0x3717, 0x58, "eReadWrite",eReadWrite}, // sensor control
	{0x3781, 0x80, "eReadWrite",eReadWrite}, // PSRAM
	{0x3784, 0x0c, "eReadWrite",eReadWrite}, //
	{0x3789, 0x60, "eReadWrite",eReadWrite}, // PSRAM
	{0x3800, 0x00, "eReadWrite",eReadWrite}, // x start H
	{0x3801, 0x00, "eReadWrite",eReadWrite}, // x start L
	{0x3802, 0x00, "eReadWrite",eReadWrite}, // y start H
	{0x3803, 0x00, "eReadWrite",eReadWrite}, // y start L
	{0x3804, 0x06, "eReadWrite",eReadWrite}, // x end H
	{0x3805, 0x4f, "eReadWrite",eReadWrite}, // x end L
	{0x3806, 0x04, "eReadWrite",eReadWrite}, // y end H
	{0x3807, 0xbf, "eReadWrite",eReadWrite}, // y end L
	{0x3808, 0x03, "eReadWrite",eReadWrite}, // x output size H
	{0x3809, 0x20, "eReadWrite",eReadWrite}, // x output size L
	{0x380a, 0x02, "eReadWrite",eReadWrite}, // y output size H
	{0x380b, 0x58, "eReadWrite",eReadWrite}, // y output size L
	{0x380c, 0x06, "eReadWrite",eReadWrite}, // HTS H
	{0x380d, 0xac, "eReadWrite",eReadWrite}, // HTS L
	{0x380e, 0x02, "eReadWrite",eReadWrite}, // VTS H
	{0x380f, 0x84, "eReadWrite",eReadWrite}, // VTS L
	{0x3810, 0x00, "eReadWrite",eReadWrite}, // ISP x win H
	{0x3811, 0x04, "eReadWrite",eReadWrite}, // ISP x win L
	{0x3812, 0x00, "eReadWrite",eReadWrite}, // ISP y win H
	{0x3813, 0x04, "eReadWrite",eReadWrite}, // ISP y win L
	{0x3814, 0x31, "eReadWrite",eReadWrite}, // x inc
	{0x3815, 0x31, "eReadWrite",eReadWrite}, // y inc
	{0x3819, 0x04, "eReadWrite",eReadWrite}, // vsync end row
	{0x3820, 0xc2, "eReadWrite",eReadWrite}, // vsun48_blc, vflip_blc, vbinf
	{0x3821, 0x01, "eReadWrite",eReadWrite}, // hbin
	{0x4000, 0x81, "eReadWrite",eReadWrite}, // avg_weight = 0x08, mf_en
	{0x4001, 0x40, "eReadWrite",eReadWrite}, // format_trig_beh
	{0x4008, 0x00, "eReadWrite",eReadWrite}, // blc_start
	{0x4009, 0x03, "eReadWrite",eReadWrite}, // blc_end
	{0x4602, 0x02, "eReadWrite",eReadWrite}, // frame reset enable
	{0x481f, 0x36, "eReadWrite",eReadWrite}, // CLK PREPARE MIN
	{0x4825, 0x36, "eReadWrite",eReadWrite}, // LPX P MIN
	{0x4837, 0x30, "eReadWrite",eReadWrite}, // MIPI global timing
	{0x5002, 0x30, "eReadWrite",eReadWrite},
	{0x5080, 0x00, "eReadWrite",eReadWrite}, // test pattern off
	{0x5081, 0x41, "eReadWrite",eReadWrite}, // window cut enable, random seed = 0x01

#endif
    {0x0000 ,0x00, "eReadWrite",eTableEnd}
};
/*
const IsiRegDescription_t Sensor_g_svga[] =
{
	{0x3086, 0x01, "eReadWrite",eReadWrite},//PLL
	{0x3501, 0x26, "eReadWrite",eReadWrite},//exposure M
	{0x3502, 0x40, "eReadWrite",eReadWrite},//exposure L
	{0x3620, 0x26, "eReadWrite",eReadWrite},//analog control
	{0x3621, 0x37, "eReadWrite",eReadWrite},//
	{0x3622, 0x04, "eReadWrite",eReadWrite},//analog control
	{0x370a, 0x23, "eReadWrite",eReadWrite},//sennsor control
	{0x370d, 0xc0, "eReadWrite",eReadWrite},//
	{0x3718, 0x88, "eReadWrite",eReadWrite},//
	{0x3721, 0x00, "eReadWrite",eReadWrite},//
	{0x3722, 0x00, "eReadWrite",eReadWrite},//
	{0x3723, 0x00, "eReadWrite",eReadWrite},//
	{0x3738, 0x00, "eReadWrite",eReadWrite},// sennsor control
	{0x3803, 0x00, "eReadWrite",eReadWrite},// y start L
	{0x3807, 0xbf, "eReadWrite",eReadWrite},// y end L
	{0x3808, 0x03, "eReadWrite",eReadWrite},// x output size H
	{0x3809, 0x20, "eReadWrite",eReadWrite},// x output size L
	{0x380a, 0x02, "eReadWrite",eReadWrite},// y output size H
	{0x380b, 0x58, "eReadWrite",eReadWrite},// y output size L
	{0x380c, 0x06, "eReadWrite",eReadWrite},// HTS H
	{0x380d, 0xac, "eReadWrite",eReadWrite},// HTS L
	{0x380e, 0x02, "eReadWrite",eReadWrite},// VTS H
	{0x380f, 0x84, "eReadWrite",eReadWrite},// VTS L
	{0x3811, 0x04, "eReadWrite",eReadWrite},// ISP x win L
	{0x3813, 0x04, "eReadWrite",eReadWrite},// ISP y win L
	{0x3814, 0x31, "eReadWrite",eReadWrite},// x inc
	{0x3815, 0x31, "eReadWrite",eReadWrite},// y inc
	{0x3820, 0xc2, "eReadWrite",eReadWrite},// vsun48_blc, vflip_blc, vbinf
	{0x3821, 0x01, "eReadWrite",eReadWrite},// hbin
	{0x4008, 0x00, "eReadWrite",eReadWrite},// blc_start
	{0x4009, 0x03, "eReadWrite",eReadWrite},// blc_end
	{0x4837, 0x30, "eReadWrite",eReadWrite},// MIPI global timing
	
    {0x0000, 0x00,"eReadWrite",eReadWrite}
};
*/

const IsiRegDescription_t Sensor_g_1600x1200[] =
{
#if 1
	{0x3086, 0x00, "eReadWrite",eReadWrite},//PLL
	//{0x3501, 0x4e, "eReadWrite",eReadWrite},//exposure M
	//{0x3502, 0xe0, "eReadWrite",eReadWrite},//exposure L
	{0x3620, 0x26, "eReadWrite",eReadWrite},// analog control
	{0x3621, 0x37, "eReadWrite",eReadWrite},//
	{0x3622, 0x04, "eReadWrite",eReadWrite},// analog control
	{0x370a, 0x21, "eReadWrite",eReadWrite},// sennsor control
	{0x370d, 0xc0, "eReadWrite",eReadWrite},//
	{0x3718, 0x88, "eReadWrite",eReadWrite},//
	{0x3721, 0x00, "eReadWrite",eReadWrite},//
	{0x3722, 0x00, "eReadWrite",eReadWrite},//
	{0x3723, 0x00, "eReadWrite",eReadWrite},//
	{0x3738, 0x00, "eReadWrite",eReadWrite},// sennsor control
	{0x3803, 0x00, "eReadWrite",eReadWrite},// y start L
	{0x3807, 0xbf, "eReadWrite",eReadWrite},// y end L
	{0x3808, 0x06, "eReadWrite",eReadWrite},// x output size H
	{0x3809, 0x40, "eReadWrite",eReadWrite},// x output size L
	{0x380a, 0x04, "eReadWrite",eReadWrite},// y output size H
	{0x380b, 0xb0, "eReadWrite",eReadWrite},// y output size L
	{0x380c, 0x06, "eReadWrite",eReadWrite},// HTS H
	{0x380d, 0xa4, "eReadWrite",eReadWrite},// HTS L
	{0x380e, 0x05, "eReadWrite",eReadWrite},// VTS H
	{0x380f, 0x0e, "eReadWrite",eReadWrite},// VTS L
	{0x3811, 0x08, "eReadWrite",eReadWrite},// ISP x win L
	{0x3813, 0x08, "eReadWrite",eReadWrite},// ISP y win L
	{0x3814, 0x11, "eReadWrite",eReadWrite},// x inc
	{0x3815, 0x11, "eReadWrite",eReadWrite},// y inc
	{0x3820, 0xc0, "eReadWrite",eReadWrite},// vsun48_blc, vflip_blc, vbin off
	{0x3821, 0x00, "eReadWrite",eReadWrite},// hbin off
	{0x4008, 0x02, "eReadWrite",eReadWrite},// blc_start
	{0x4009, 0x09, "eReadWrite",eReadWrite},// blc_end
	{0x4837, 0x18, "eReadWrite",eReadWrite},// MIPI global timing
#else
	// sysclk = 33Mhz, MIPI data rate 330Mbps
	{0x3086, 0x01, "eReadWrite",eReadWrite}, // PLL
	{0x3501, 0x4e, "eReadWrite",eReadWrite}, // exposure M
	{0x3502, 0xe0, "eReadWrite",eReadWrite}, // exposure L
	{0x3620, 0x24, "eReadWrite",eReadWrite}, // analog control
	{0x3621, 0x34, "eReadWrite",eReadWrite}, //
	{0x3622, 0x03, "eReadWrite",eReadWrite}, // analog control
	{0x370a, 0x21, "eReadWrite",eReadWrite}, //
	{0x370d, 0x00, "eReadWrite",eReadWrite}, //
	{0x3718, 0x80, "eReadWrite",eReadWrite}, //
	{0x3721, 0x09, "eReadWrite",eReadWrite}, //
	{0x3722, 0x0b, "eReadWrite",eReadWrite}, //
	{0x3723, 0x48, "eReadWrite",eReadWrite}, //
	{0x3738, 0x99, "eReadWrite",eReadWrite}, // sennsor control
	{0x3803, 0x00, "eReadWrite",eReadWrite}, // y start L
	{0x3807, 0xbf, "eReadWrite",eReadWrite}, // y end L
	{0x3808, 0x06, "eReadWrite",eReadWrite}, // x output size H
	{0x3809, 0x40, "eReadWrite",eReadWrite}, // x output size L
	{0x380a, 0x04, "eReadWrite",eReadWrite}, // y output size H
	{0x380b, 0xb0, "eReadWrite",eReadWrite}, // y output size H
	{0x380c, 0x06, "eReadWrite",eReadWrite}, // HTS H
	{0x380d, 0xa4, "eReadWrite",eReadWrite}, // HTS L
	{0x380e, 0x05, "eReadWrite",eReadWrite}, // VTS H
	{0x380f, 0x0e, "eReadWrite",eReadWrite}, // VTS L
	{0x3811, 0x08, "eReadWrite",eReadWrite}, // ISP x win L
	{0x3813, 0x08, "eReadWrite",eReadWrite}, // ISP y win L
	{0x3814, 0x11, "eReadWrite",eReadWrite}, // x inc
	{0x3815, 0x11, "eReadWrite",eReadWrite}, // y inc
	{0x3820, 0xc0, "eReadWrite",eReadWrite}, // vsun48_blc, vflip_blc, vbin off
	{0x3821, 0x00, "eReadWrite",eReadWrite}, // hbin off
	{0x4008, 0x02, "eReadWrite",eReadWrite}, // blc_start
	{0x4009, 0x09, "eReadWrite",eReadWrite}, // blc_end
	{0x4837, 0x30, "eReadWrite",eReadWrite}, // MIPI global timing
#endif
    {0x0000, 0x00,"eReadWrite",eTableEnd}
};

const IsiRegDescription_t Sensor_g_1600x1200_30fps[] =
{
	{0x380e, 0x05, "eReadWrite",eReadWrite},// VTS H
	{0x380f, 0x0e, "eReadWrite",eReadWrite},// VTS L
	{0x0000, 0x00, "eReadWrite",eTableEnd}
	
};
const IsiRegDescription_t Sensor_g_1600x1200_20fps[] =
{
	{0x380e, 0x07, "eReadWrite",eReadWrite},// VTS H
	{0x380f, 0x95, "eReadWrite",eReadWrite},// VTS L
	{0x0000, 0x00, "eReadWrite",eTableEnd}
	
};
const IsiRegDescription_t Sensor_g_1600x1200_15fps[] =
{
	{0x380e, 0x0a, "eReadWrite",eReadWrite},// VTS H
	{0x380f, 0x1c, "eReadWrite",eReadWrite},// VTS L
	{0x0000, 0x00, "eReadWrite",eTableEnd}
	
};

const IsiRegDescription_t Sensor_g_1600x1200_10fps[] =
{
	{0x380e, 0x0f, "eReadWrite",eReadWrite},// VTS H
	{0x380f, 0x2a, "eReadWrite",eReadWrite},// VTS L
	{0x0000, 0x00, "eReadWrite",eTableEnd}
	
};


