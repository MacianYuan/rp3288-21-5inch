//AR0330_tables.c
/*****************************************************************************/
/*!
 *  \file        AR0330_tables.c \n
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

#if( AR0330_DRIVER_USAGE == USE_CAM_DRV_EN )
*/


#include <ebase/types.h>
#include <ebase/trace.h>
#include <ebase/builtins.h>

#include <common/return_codes.h>

#include "isi.h"
#include "isi_iss.h"
#include "isi_priv.h"
#include "AR0330_MIPI_priv.h"


/*****************************************************************************
 * DEFINES
 *****************************************************************************/


/*****************************************************************************
 * GLOBALS
 *****************************************************************************/

// The settings may be altered by the code in IsiSetupSensor.


#if 1
//two lane
const IsiRegDescription_t Sensor_g_aRegDescription_twolane[] =
{
	{0x301A, 0x0058,"0x0100",eReadWrite_16},			//RESET_REGISTER = 88
	{0x302A, 0x0005,"0x0100",eReadWrite_16},			//VT_PIX_CLK_DIV = 5
	{0x302C, 0x0002,"0x0100",eReadWrite_16},			//VT_SYS_CLK_DIV = 2
	{0x302E, 0x0002,"0x0100",eReadWrite_16},			//PRE_PLL_CLK_DIV = 2
	{0x3030, 0x0029,"0x0100",eReadWrite_16},			//PLL_MULTIPLIER = 41
	{0x3036, 0x000A,"0x0100",eReadWrite_16},			//OP_PIX_CLK_DIV = 10
	{0x3038, 0x0001,"0x0100",eReadWrite_16},			//OP_SYS_CLK_DIV = 1
	{0x31AC, 0x0A0A,"0x0100",eReadWrite_16},			//DATA_FORMAT_BITS = 2570
	{0x31AE, 0x0202,"0x0100",eReadWrite_16},			//SERIAL_FORMAT = 514
	
	//MIPI Port Timing
	{0x31B0, 0x0045,"0x0100",eReadWrite_16},		//FRAME_PREAMBLE = 69
	{0x31B2, 0x0029,"0x0100",eReadWrite_16},		//LINE_PREAMBLE = 41
	{0x31B4, 0x3C44,"0x0100",eReadWrite_16},		//MIPI_TIMING_0 = 15428
	{0x31B6, 0x314D,"0x0100",eReadWrite_16},		//MIPI_TIMING_1 = 12621
	{0x31B8, 0x208A,"0x0100",eReadWrite_16},		//MIPI_TIMING_2 = 8330
	{0x31BA, 0x0207,"0x0100",eReadWrite_16},		//MIPI_TIMING_3 = 519
	{0x31BC, 0x0005,"0x0100",eReadWrite_16},		//MIPI_TIMING_4 = 5
	
	
	//Timing_settings
	{0x3002, 0x0006,"0x0100",eReadWrite_16},		//Y_ADDR_START = 6
	{0x3004, 0x0086,"0x0100",eReadWrite_16},		//X_ADDR_START = 134
	{0x3006, 0x0605,"0x0100",eReadWrite_16},		//Y_ADDR_END = 1541
	{0x3008, 0x0885,"0x0100",eReadWrite_16},		//X_ADDR_END = 2181
	{0x300A, 0x0630,"0x0100",eReadWrite_16},		//FRAME_LENGTH_LINES = 1584
	{0x300C, 0x04DA,"0x0100",eReadWrite_16},		//LINE_LENGTH_PCK = 1242
	{0x3012, 0x0528,"0x0100",eReadWrite_16},		//COARSE_INTEGRATION_TIME = 1320
	{0x3014, 0x0000,"0x0100",eReadWrite_16},		//FINE_INTEGRATION_TIME = 0
	{0x30A2, 0x0001,"0x0100",eReadWrite_16},		//X_ODD_INC = 1
	{0x30A6, 0x0001,"0x0100",eReadWrite_16},		//Y_ODD_INC = 1
	{0x308C, 0x0006,"0x0100",eReadWrite_16},		//Y_ADDR_START_CB = 6
	{0x308A, 0x0086,"0x0100",eReadWrite_16},		//X_ADDR_START_CB = 134
	{0x3090, 0x0605,"0x0100",eReadWrite_16},		//Y_ADDR_END_CB = 1541
	{0x308E, 0x0885,"0x0100",eReadWrite_16},		//X_ADDR_END_CB = 2181
	{0x30AA, 0x0630,"0x0100",eReadWrite_16},		//FRAME_LENGTH_LINES_CB = 1584
	{0x303E, 0x04DA,"0x0100",eReadWrite_16},		//LINE_LENGTH_PCK_CB = 1242
	{0x3016, 0x0527,"0x0100",eReadWrite_16},		//COARSE_INTEGRATION_TIME_CB = 1319
	{0x3018, 0x0000,"0x0100",eReadWrite_16},		//FINE_INTEGRATION_TIME_CB = 0
	{0x30AE, 0x0001,"0x0100",eReadWrite_16},		//X_ODD_INC_CB = 1
	{0x30A8, 0x0001,"0x0100",eReadWrite_16},		//Y_ODD_INC_CB = 1
	{0x3040, 0x0000,"0x0100",eReadWrite_16},		//READ_MODE = 0
	{0x3042, 0x02A0,"0x0100",eReadWrite_16},		//EXTRA_DELAY = 672
	{0x30BA, 0x002C,"0x0100",eReadWrite_16},		//DIGITAL_CTRL = 44
	
	
	
	//Recommended Configuration
	{0x31E0, 0x0303,"0x0100",eReadWrite_16},
	{0x3064, 0x1802,"0x0100",eReadWrite_16},
	{0x3ED2, 0x0146,"0x0100",eReadWrite_16},
	{0x3ED4, 0x8F6C,"0x0100",eReadWrite_16},
	{0x3ED6, 0x66CC,"0x0100",eReadWrite_16},
	{0x3ED8, 0x8C42,"0x0100",eReadWrite_16},
	{0x3EDA, 0x88BC,"0x0100",eReadWrite_16},
	{0x3EDC, 0xAA63,"0x0100",eReadWrite_16},
	{0x305E, 0x00A0,"0x0100",eReadWrite_16},
	{0x0000 ,0x00,"eTableEnd",eTableEnd}   


};

const IsiRegDescription_t Sensor_g_twolane_resolution_2048_1536[] =
{
	{0x301A, 0x0058,"0x0100",eReadWrite_16},			//RESET_REGISTER = 88
	{0x302A, 0x0005,"0x0100",eReadWrite_16},			//VT_PIX_CLK_DIV = 5
	{0x302C, 0x0002,"0x0100",eReadWrite_16},			//VT_SYS_CLK_DIV = 2
	{0x302E, 0x0002,"0x0100",eReadWrite_16},			//PRE_PLL_CLK_DIV = 2
	{0x3030, 0x0029,"0x0100",eReadWrite_16},			//PLL_MULTIPLIER = 41
	{0x3036, 0x000A,"0x0100",eReadWrite_16},			//OP_PIX_CLK_DIV = 10
	{0x3038, 0x0001,"0x0100",eReadWrite_16},			//OP_SYS_CLK_DIV = 1
	{0x31AC, 0x0A0A,"0x0100",eReadWrite_16},			//DATA_FORMAT_BITS = 2570
	{0x31AE, 0x0202,"0x0100",eReadWrite_16},			//SERIAL_FORMAT = 514
	
	//MIPI Port Timing
	{0x31B0, 0x0045,"0x0100",eReadWrite_16},		//FRAME_PREAMBLE = 69
	{0x31B2, 0x0029,"0x0100",eReadWrite_16},		//LINE_PREAMBLE = 41
	{0x31B4, 0x3C44,"0x0100",eReadWrite_16},		//MIPI_TIMING_0 = 15428
	{0x31B6, 0x314D,"0x0100",eReadWrite_16},		//MIPI_TIMING_1 = 12621
	{0x31B8, 0x208A,"0x0100",eReadWrite_16},		//MIPI_TIMING_2 = 8330
	{0x31BA, 0x0207,"0x0100",eReadWrite_16},		//MIPI_TIMING_3 = 519
	{0x31BC, 0x0005,"0x0100",eReadWrite_16},		//MIPI_TIMING_4 = 5
	
	
	//Timing_settings
	{0x3002, 0x0006,"0x0100",eReadWrite_16},		//Y_ADDR_START = 6
	{0x3004, 0x0086,"0x0100",eReadWrite_16},		//X_ADDR_START = 134
	{0x3006, 0x0605,"0x0100",eReadWrite_16},		//Y_ADDR_END = 1541
	{0x3008, 0x0885,"0x0100",eReadWrite_16},		//X_ADDR_END = 2181
	{0x300A, 0x0630,"0x0100",eReadWrite_16},		//FRAME_LENGTH_LINES = 1584
	{0x300C, 0x04DA,"0x0100",eReadWrite_16},		//LINE_LENGTH_PCK = 1242
	{0x3012, 0x0528,"0x0100",eReadWrite_16},		//COARSE_INTEGRATION_TIME = 1320
	{0x3014, 0x0000,"0x0100",eReadWrite_16},		//FINE_INTEGRATION_TIME = 0
	{0x30A2, 0x0001,"0x0100",eReadWrite_16},		//X_ODD_INC = 1
	{0x30A6, 0x0001,"0x0100",eReadWrite_16},		//Y_ODD_INC = 1
	{0x308C, 0x0006,"0x0100",eReadWrite_16},		//Y_ADDR_START_CB = 6
	{0x308A, 0x0086,"0x0100",eReadWrite_16},		//X_ADDR_START_CB = 134
	{0x3090, 0x0605,"0x0100",eReadWrite_16},		//Y_ADDR_END_CB = 1541
	{0x308E, 0x0885,"0x0100",eReadWrite_16},		//X_ADDR_END_CB = 2181
	{0x30AA, 0x0630,"0x0100",eReadWrite_16},		//FRAME_LENGTH_LINES_CB = 1584
	{0x303E, 0x04DA,"0x0100",eReadWrite_16},		//LINE_LENGTH_PCK_CB = 1242
	{0x3016, 0x0527,"0x0100",eReadWrite_16},		//COARSE_INTEGRATION_TIME_CB = 1319
	{0x3018, 0x0000,"0x0100",eReadWrite_16},		//FINE_INTEGRATION_TIME_CB = 0
	{0x30AE, 0x0001,"0x0100",eReadWrite_16},		//X_ODD_INC_CB = 1
	{0x30A8, 0x0001,"0x0100",eReadWrite_16},		//Y_ODD_INC_CB = 1
	{0x3040, 0x0000,"0x0100",eReadWrite_16},		//READ_MODE = 0
	{0x3042, 0x02A0,"0x0100",eReadWrite_16},		//EXTRA_DELAY = 672
	{0x30BA, 0x002C,"0x0100",eReadWrite_16},		//DIGITAL_CTRL = 44
	
	
	
	//Recommended Configuration
	{0x31E0, 0x0303,"0x0100",eReadWrite_16},
	{0x3064, 0x1802,"0x0100",eReadWrite_16},
	{0x3ED2, 0x0146,"0x0100",eReadWrite_16},
	{0x3ED4, 0x8F6C,"0x0100",eReadWrite_16},
	{0x3ED6, 0x66CC,"0x0100",eReadWrite_16},
	{0x3ED8, 0x8C42,"0x0100",eReadWrite_16},
	{0x3EDA, 0x88BC,"0x0100",eReadWrite_16},
	{0x3EDC, 0xAA63,"0x0100",eReadWrite_16},
	{0x305E, 0x00A0,"0x0100",eReadWrite_16},
	{0x0000 ,0x00,"eTableEnd",eTableEnd}   


};


#else
const IsiRegDescription_t Sensor_g_aRegDescription_twolane[] =
{
	//PLL_settings
		//STATE = Master Clock, 76700000
		  {0x301A, 0x0058,"0x0100",eReadWrite_16},		//RESET_REGISTER = 88
		  {0x302A, 0x0005,"0x0100",eReadWrite_16},		//VT_PIX_CLK_DIV = 5
		  {0x302C, 0x0002,"0x0100",eReadWrite_16},		//VT_SYS_CLK_DIV = 2
		  {0x302E, 0x0002,"0x0100",eReadWrite_16},		//PRE_PLL_CLK_DIV = 2
		  {0x3030, 0x003B,"0x0100",eReadWrite_16},		//PLL_MULTIPLIER = 59
		  {0x3036, 0x000A,"0x0100",eReadWrite_16},		//OP_PIX_CLK_DIV = 10
		  {0x3038, 0x0001,"0x0100",eReadWrite_16},		//OP_SYS_CLK_DIV = 1
		  {0x31AC, 0x0A0A,"0x0100",eReadWrite_16},		//DATA_FORMAT_BITS = 2570
		  {0x31AE, 0x0202,"0x0100",eReadWrite_16},		//SERIAL_FORMAT = 514
														   
		//MIPI Port Timing								   
		  {0x31B0, 0x005E,"0x0100",eReadWrite_16},		//FRAME_PREAMBLE = 94
		  {0x31B2, 0x0038,"0x0100",eReadWrite_16},		//LINE_PREAMBLE = 56
		  {0x31B4, 0x4F66,"0x0100",eReadWrite_16},		//MIPI_TIMING_0 = 20326
		  {0x31B6, 0x4215,"0x0100",eReadWrite_16},		//MIPI_TIMING_1 = 16917
		  {0x31B8, 0x308B,"0x0100",eReadWrite_16},		//MIPI_TIMING_2 = 12427
		  {0x31BA, 0x028A,"0x0100",eReadWrite_16},		//MIPI_TIMING_3 = 650
		  {0x31BC, 0x0008,"0x0100",eReadWrite_16},		//MIPI_TIMING_4 = 8
														   
														   
		// timing_settings								   
		 {0x3002, 0x0006,"0x0100",eReadWrite_16},		//Y_ADDR_START = 6
		 {0x3004, 0x0086,"0x0100",eReadWrite_16},		//X_ADDR_START = 134
		 {0x3006, 0x0605,"0x0100",eReadWrite_16},		//Y_ADDR_END = 1541
		 {0x3008, 0x0885,"0x0100",eReadWrite_16},		//X_ADDR_END = 2181
		 {0x300A, 0x080C,"0x0100",eReadWrite_16},		//FRAME_LENGTH_LINES = 2060
		 {0x300C, 0x04DA,"0x0100",eReadWrite_16},		//LINE_LENGTH_PCK = 1242
		 {0x3012, 0x080A,"0x0100",eReadWrite_16},		//COARSE_INTEGRATION_TIME = 2058
		 {0x3014, 0x0000,"0x0100",eReadWrite_16},		//FINE_INTEGRATION_TIME = 0
		 {0x30A2, 0x0001,"0x0100",eReadWrite_16},		//X_ODD_INC = 1
		 {0x30A6, 0x0001,"0x0100",eReadWrite_16},		//Y_ODD_INC = 1
		 {0x308C, 0x007E,"0x0100",eReadWrite_16},		//Y_ADDR_START_CB = 126
		 {0x308A, 0x0006,"0x0100",eReadWrite_16},		//X_ADDR_START_CB = 6
		 {0x3090, 0x058D,"0x0100",eReadWrite_16},		//Y_ADDR_END_CB = 1421
		 {0x308E, 0x0905,"0x0100",eReadWrite_16},		//X_ADDR_END_CB = 2309
		 {0x30AA, 0x0802,"0x0100",eReadWrite_16},		//FRAME_LENGTH_LINES_CB = 2050
		 {0x303E, 0x04E0,"0x0100",eReadWrite_16},		//LINE_LENGTH_PCK_CB = 1248
		 {0x3016, 0x0801,"0x0100",eReadWrite_16},		//COARSE_INTEGRATION_TIME_CB = 2049
		 {0x3018, 0x0000,"0x0100",eReadWrite_16},		//FINE_INTEGRATION_TIME_CB = 0
		 {0x30AE, 0x0001,"0x0100",eReadWrite_16},		//X_ODD_INC_CB = 1
		 {0x30A8, 0x0001,"0x0100",eReadWrite_16},		//Y_ODD_INC_CB = 1
		 {0x3010, 0xBEFF,"0x0100",eReadWrite_16},	//mirror and flip mode select  
		 {0x3040, 0x0000,"0x0100",eReadWrite_16},		//READ_MODE = 0
		 {0x3042, 0x02C1,"0x0100",eReadWrite_16},		//EXTRA_DELAY = 705
		 {0x30BA, 0x002C,"0x0100",eReadWrite_16},		//DIGITAL_CTRL = 44
															
															
		 //commended Configuration							
		  {0x31E0, 0x0303,"0x0100",eReadWrite_16},
		  {0x3064, 0x1802,"0x0100",eReadWrite_16},
		  {0x3ED2, 0x0146,"0x0100",eReadWrite_16},
		  {0x3ED4, 0x8F6C,"0x0100",eReadWrite_16},
		  {0x3ED6, 0x66CC,"0x0100",eReadWrite_16},
		  {0x3ED8, 0x8C42,"0x0100",eReadWrite_16},
		  {0x3EDA, 0x88BC,"0x0100",eReadWrite_16},
		  {0x3EDC, 0xAA63,"0x0100",eReadWrite_16},
		  {0x305E, 0x00A0,"0x0100",eReadWrite_16},
		
		 {0x30CE, 0x0020,"0x0100",eReadWrite_16},  //auto adjust fps   
		 {0x0000 ,0x00,"eTableEnd",eTableEnd}   

};
const IsiRegDescription_t Sensor_g_twolane_resolution_2048_1536[] =
{
	//PLL_settings
		//STATE = Master Clock, 76700000
		  {0x301A, 0x0058,"0x0100",eReadWrite_16},		//RESET_REGISTER = 88
		  {0x302A, 0x0005,"0x0100",eReadWrite_16},		//VT_PIX_CLK_DIV = 5
		  {0x302C, 0x0002,"0x0100",eReadWrite_16},		//VT_SYS_CLK_DIV = 2
		  {0x302E, 0x0002,"0x0100",eReadWrite_16},		//PRE_PLL_CLK_DIV = 2
		  {0x3030, 0x003B,"0x0100",eReadWrite_16},		//PLL_MULTIPLIER = 59
		  {0x3036, 0x000A,"0x0100",eReadWrite_16},		//OP_PIX_CLK_DIV = 10
		  {0x3038, 0x0001,"0x0100",eReadWrite_16},		//OP_SYS_CLK_DIV = 1
		  {0x31AC, 0x0A0A,"0x0100",eReadWrite_16},		//DATA_FORMAT_BITS = 2570
		  {0x31AE, 0x0202,"0x0100",eReadWrite_16},		//SERIAL_FORMAT = 514
														   
		//MIPI Port Timing								   
		  {0x31B0, 0x005E,"0x0100",eReadWrite_16},		//FRAME_PREAMBLE = 94
		  {0x31B2, 0x0038,"0x0100",eReadWrite_16},		//LINE_PREAMBLE = 56
		  {0x31B4, 0x4F66,"0x0100",eReadWrite_16},		//MIPI_TIMING_0 = 20326
		  {0x31B6, 0x4215,"0x0100",eReadWrite_16},		//MIPI_TIMING_1 = 16917
		  {0x31B8, 0x308B,"0x0100",eReadWrite_16},		//MIPI_TIMING_2 = 12427
		  {0x31BA, 0x028A,"0x0100",eReadWrite_16},		//MIPI_TIMING_3 = 650
		  {0x31BC, 0x0008,"0x0100",eReadWrite_16},		//MIPI_TIMING_4 = 8
														   
														   
		// timing_settings								   
		 {0x3002, 0x0006,"0x0100",eReadWrite_16},		//Y_ADDR_START = 6
		 {0x3004, 0x0086,"0x0100",eReadWrite_16},		//X_ADDR_START = 134
		 {0x3006, 0x0605,"0x0100",eReadWrite_16},		//Y_ADDR_END = 1541
		 {0x3008, 0x0885,"0x0100",eReadWrite_16},		//X_ADDR_END = 2181
		 {0x300A, 0x080C,"0x0100",eReadWrite_16},		//FRAME_LENGTH_LINES = 2060
		 {0x300C, 0x04DA,"0x0100",eReadWrite_16},		//LINE_LENGTH_PCK = 1242
		 {0x3012, 0x080A,"0x0100",eReadWrite_16},		//COARSE_INTEGRATION_TIME = 2058
		 {0x3014, 0x0000,"0x0100",eReadWrite_16},		//FINE_INTEGRATION_TIME = 0
		 {0x30A2, 0x0001,"0x0100",eReadWrite_16},		//X_ODD_INC = 1
		 {0x30A6, 0x0001,"0x0100",eReadWrite_16},		//Y_ODD_INC = 1
		 {0x308C, 0x007E,"0x0100",eReadWrite_16},		//Y_ADDR_START_CB = 126
		 {0x308A, 0x0006,"0x0100",eReadWrite_16},		//X_ADDR_START_CB = 6
		 {0x3090, 0x058D,"0x0100",eReadWrite_16},		//Y_ADDR_END_CB = 1421
		 {0x308E, 0x0905,"0x0100",eReadWrite_16},		//X_ADDR_END_CB = 2309
		 {0x30AA, 0x0802,"0x0100",eReadWrite_16},		//FRAME_LENGTH_LINES_CB = 2050
		 {0x303E, 0x04E0,"0x0100",eReadWrite_16},		//LINE_LENGTH_PCK_CB = 1248
		 {0x3016, 0x0801,"0x0100",eReadWrite_16},		//COARSE_INTEGRATION_TIME_CB = 2049
		 {0x3018, 0x0000,"0x0100",eReadWrite_16},		//FINE_INTEGRATION_TIME_CB = 0
		 {0x30AE, 0x0001,"0x0100",eReadWrite_16},		//X_ODD_INC_CB = 1
		 {0x30A8, 0x0001,"0x0100",eReadWrite_16},		//Y_ODD_INC_CB = 1
		 {0x3010, 0xBEFF,"0x0100",eReadWrite_16},	//mirror and flip mode select  
		 {0x3040, 0x0000,"0x0100",eReadWrite_16},		//READ_MODE = 0
		 {0x3042, 0x02C1,"0x0100",eReadWrite_16},		//EXTRA_DELAY = 705
		 {0x30BA, 0x002C,"0x0100",eReadWrite_16},		//DIGITAL_CTRL = 44
															
															
		 //commended Configuration							
		  {0x31E0, 0x0303,"0x0100",eReadWrite_16},
		  {0x3064, 0x1802,"0x0100",eReadWrite_16},
		  {0x3ED2, 0x0146,"0x0100",eReadWrite_16},
		  {0x3ED4, 0x8F6C,"0x0100",eReadWrite_16},
		  {0x3ED6, 0x66CC,"0x0100",eReadWrite_16},
		  {0x3ED8, 0x8C42,"0x0100",eReadWrite_16},
		  {0x3EDA, 0x88BC,"0x0100",eReadWrite_16},
		  {0x3EDC, 0xAA63,"0x0100",eReadWrite_16},
		  {0x305E, 0x00A0,"0x0100",eReadWrite_16},
		
		 {0x30CE, 0x0020,"0x0100",eReadWrite_16},  //auto adjust fps   
		 {0x0000 ,0x00,"eTableEnd",eTableEnd}   

};

#endif



const IsiRegDescription_t Sensor_g_2048x1536P30_twolane_fpschg[] =
{
	{0x30AA, 0x0528,"0x0100",eReadWrite_16},
	{0x0000 ,0x00,"eTableEnd",eTableEnd}
};
const IsiRegDescription_t Sensor_g_2048x1536P25_twolane_fpschg[] =
{
	{0x30AA, 0x0630,"0x0100",eReadWrite_16},
	{0x0000 ,0x00,"eTableEnd",eTableEnd}
};
const IsiRegDescription_t Sensor_g_2048x1536P20_twolane_fpschg[] =
{
	{0x30AA, 0x07BC,"0x0100",eReadWrite_16},
	{0x0000 ,0x00,"eTableEnd",eTableEnd}
};

const IsiRegDescription_t Sensor_g_2048x1536P15_twolane_fpschg[] =
{
	{0x30AA, 0x0A50,"0x0100",eReadWrite_16},
	{0x0000 ,0x00,"eTableEnd",eTableEnd}
};
const IsiRegDescription_t Sensor_g_2048x1536P10_twolane_fpschg[] =
{
	{0x30AA, 0x0F78,"0x0100",eReadWrite_16},
	{0x0000 ,0x00,"eTableEnd",eTableEnd}
};

#if 0
const IsiRegDescription_t Sensor_g_2592x1944P15_twolane_fpschg[] =
{
	{0x380e, 0x07,"0x0100",eReadWrite_16},
	{0x380f, 0xc0,"0x0100",eReadWrite_16},
	{0x0000 ,0x00,"eTableEnd",eTableEnd}
};
const IsiRegDescription_t Sensor_g_2592x1944P7_twolane_fpschg[] =
{
	{0x380e, 0x0f,"0x0100",eReadWrite_16},
	{0x380f, 0x80,"0x0100",eReadWrite_16},
	{0x0000 ,0x00,"eTableEnd",eTableEnd}
};
#endif

