//GC5005_tables.c
/*****************************************************************************/
/*!
 *  \file        GC5005_tables.c \n
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

#if( GC5005_DRIVER_USAGE == USE_CAM_DRV_EN )
*/


#include <ebase/types.h>
#include <ebase/trace.h>
#include <ebase/builtins.h>

#include <common/return_codes.h>

#include "isi.h"
#include "isi_iss.h"
#include "isi_priv.h"
#include "GC5005_MIPI_priv.h"


/*****************************************************************************
 * DEFINES
 *****************************************************************************/


/*****************************************************************************
 * GLOBALS
 *****************************************************************************/

// Image sensor register settings default values taken from data sheet OV13850_DS_1.1_SiliconImage.pdf.
// The settings may be altered by the code in IsiSetupSensor.


//two lane
const IsiRegDescription_t Sensor_g_aRegDescription_twolane[] =
{
	{0xfe, 0x00,"0x0100",eReadWrite},
	{0xfe, 0x00,"0x0100",eReadWrite},
	{0xfe, 0x00,"0x0100",eReadWrite},
	{0xf7, 0x01,"0x0100",eReadWrite},
	{0xf8, 0x11,"0x0100",eReadWrite},
	{0xf9, 0xaa,"0x0100",eReadWrite},
	{0xfa, 0x84,"0x0100",eReadWrite},
	{0xfc, 0x8a,"0x0100",eReadWrite},
	{0xfe, 0x03,"0x0100",eReadWrite},
	{0x10, 0x01,"0x0100",eReadWrite},
	{0xfc, 0x8e,"0x0100",eReadWrite},
	{0xfe, 0x00,"0x0100",eReadWrite},
	{0xfe, 0x00,"0x0100",eReadWrite},
	{0xfe, 0x00,"0x0100",eReadWrite},
	{0x88, 0x03,"0x0100",eReadWrite},
	{0xe7, 0xc0,"0x0100",eReadWrite},

	/*Analog*/
	{0xfe, 0x00,"0x0100",eReadWrite},
	{0x03, 0x06,"0x0100",eReadWrite},                                                                                  
	{0x04, 0xfc,"0x0100",eReadWrite},                                                                                  
	{0x05, 0x01,"0x0100",eReadWrite},                                                                                  
	{0x06, 0xc5,"0x0100",eReadWrite},                                                                                  
	{0x07, 0x00,"0x0100",eReadWrite},                                                                                  
	{0x08, 0x10,"0x0100",eReadWrite},                                                                                  
	{0x09, 0x00,"0x0100",eReadWrite},                                                                                  
	{0x0a, 0x14,"0x0100",eReadWrite},                                                                                                
	{0x0b, 0x00,"0x0100",eReadWrite},                                                                                  
	{0x0c, 0x10,"0x0100",eReadWrite},
	{0x0d, 0x07,"0x0100",eReadWrite},                                                                                  
	{0x0e, 0xa0,"0x0100",eReadWrite},                                                                                  
	{0x0f, 0x0a,"0x0100",eReadWrite},                                                                                  
	{0x10, 0x30,"0x0100",eReadWrite},
	{0x17, 0x55,"0x0100",eReadWrite},                                       
	{0x18, 0x02,"0x0100",eReadWrite},                                                                                  
	{0x19, 0x0a,"0x0100",eReadWrite},
	{0x1a, 0x1b,"0x0100",eReadWrite},                                                                                  
	{0x1c, 0x0c,"0x0100",eReadWrite},                                                                                  
	{0x1d, 0x19,"0x0100",eReadWrite},                                                                                  
	{0x21, 0x16,"0x0100",eReadWrite},
	{0x24, 0xb0,"0x0100",eReadWrite},                                                                                  
	{0x25, 0xc1,"0x0100",eReadWrite},                                                                                  
	{0x27, 0x64,"0x0100",eReadWrite},                                                                                  
	{0x29, 0x28,"0x0100",eReadWrite},
	{0x2a, 0xc3,"0x0100",eReadWrite},                                                                                  
	{0x31, 0x40,"0x0100",eReadWrite},
	{0x32, 0xf8,"0x0100",eReadWrite},                                                                                  
	{0xcd, 0xca,"0x0100",eReadWrite},                                                                                  
	{0xce, 0xff,"0x0100",eReadWrite},                                                                                  
	{0xcf, 0x70,"0x0100",eReadWrite},                                                                                  
	{0xd0, 0xd2,"0x0100",eReadWrite},                                                                                  
	{0xd1, 0xa0,"0x0100",eReadWrite},                                                                                                        
	{0xd3, 0x23,"0x0100",eReadWrite},
	{0xd8, 0x12,"0x0100",eReadWrite},
	{0xdc, 0xb3,"0x0100",eReadWrite},                                                                                  
	{0xe1, 0x1b,"0x0100",eReadWrite},
	{0xe2, 0x00,"0x0100",eReadWrite},
	{0xe4, 0x78,"0x0100",eReadWrite},                                                                                  
	{0xe6, 0x1f,"0x0100",eReadWrite},                                                                                  
	{0xe7, 0xc0,"0x0100",eReadWrite},                                                                                  
	{0xe8, 0x01,"0x0100",eReadWrite},
	{0xe9, 0x02,"0x0100",eReadWrite},                                                                                  
	{0xec, 0x01,"0x0100",eReadWrite}, 
	{0xed, 0x02,"0x0100",eReadWrite},     

	

	/*ISP*/
	{0x80, 0x50,"0x0100",eReadWrite},                                                                    
	{0x90, 0x01,"0x0100",eReadWrite},                                                                    
	{0x92, 0x03,"0x0100",eReadWrite},                                 
	{0x94, 0x04,"0x0100",eReadWrite},                   
	{0x95, 0x07,"0x0100",eReadWrite},                                                                    
	{0x96, 0x98,"0x0100",eReadWrite},                                                                    
	{0x97, 0x0a,"0x0100",eReadWrite},                                                                    
	{0x98, 0x20,"0x0100",eReadWrite},                                                                    
                                                                                                                                         
	/*Gain*/
	{0x99, 0x00,"0x0100",eReadWrite},
	{0x9a, 0x08,"0x0100",eReadWrite},
	{0x9b, 0x10,"0x0100",eReadWrite},
	{0x9c, 0x18,"0x0100",eReadWrite},
	{0x9d, 0x19,"0x0100",eReadWrite},
	{0x9e, 0x1a,"0x0100",eReadWrite},
	{0x9f, 0x1b,"0x0100",eReadWrite},
	{0xa0, 0x1c,"0x0100",eReadWrite},
	{0xb0, 0x50,"0x0100",eReadWrite},
	{0xb1, 0x01,"0x0100",eReadWrite},
	{0xb2, 0x00,"0x0100",eReadWrite},
	{0xb6, 0x00,"0x0100",eReadWrite},

	/*DD*/
	{0xfe, 0x01,"0x0100",eReadWrite},
	{0xc2, 0x02,"0x0100",eReadWrite},
	{0xc3, 0xe0,"0x0100",eReadWrite},
	{0xc4, 0xd9,"0x0100",eReadWrite},
	{0xc5, 0x00,"0x0100",eReadWrite},
	{0xfe, 0x00,"0x0100",eReadWrite},

	/*BLK*/
	{0x40, 0x22,"0x0100",eReadWrite},
	{0x4e, 0x3c,"0x0100",eReadWrite},
	{0x4f, 0x3c,"0x0100",eReadWrite},
	{0x60, 0x00,"0x0100",eReadWrite},
	{0x61, 0x80,"0x0100",eReadWrite},
	{0xab, 0x00,"0x0100",eReadWrite},
	{0xac, 0x30,"0x0100",eReadWrite},
	
	/*Dark Sun*/
	{0x68, 0xf4,"0x0100",eReadWrite},
	{0x6a, 0x00,"0x0100",eReadWrite},
	{0x6b, 0x00,"0x0100",eReadWrite},
	{0x6c, 0x50,"0x0100",eReadWrite},
	{0x6e, 0xc9,"0x0100",eReadWrite},
	
	/*MIPI*/
	{0xfe, 0x03,"0x0100",eReadWrite},
	{0x01, 0x07,"0x0100",eReadWrite},
	{0x02, 0x33,"0x0100",eReadWrite},
	{0x03, 0x93,"0x0100",eReadWrite},
	{0x04, 0x04,"0x0100",eReadWrite},
	{0x05, 0x00,"0x0100",eReadWrite},
	{0x06, 0x00,"0x0100",eReadWrite},
	{0x10, 0x91,"0x0100",eReadWrite},
	{0x11, 0x2b,"0x0100",eReadWrite},
	{0x12, 0xa8,"0x0100",eReadWrite},
	{0x13, 0x0c,"0x0100",eReadWrite},
	{0x15, 0x00,"0x0100",eReadWrite},
	{0x18, 0x01,"0x0100",eReadWrite},
	{0x1b, 0x14,"0x0100",eReadWrite},
	{0x1c, 0x14,"0x0100",eReadWrite},
	{0x21, 0x10,"0x0100",eReadWrite},
	{0x22, 0x05,"0x0100",eReadWrite},
	{0x23, 0x30,"0x0100",eReadWrite},
	{0x24, 0x02,"0x0100",eReadWrite},
	{0x25, 0x16,"0x0100",eReadWrite},
	{0x26, 0x09,"0x0100",eReadWrite},
	{0x29, 0x06,"0x0100",eReadWrite},
	{0x2a, 0x0d,"0x0100",eReadWrite},
	{0x2b, 0x09,"0x0100",eReadWrite},
 	{0xfe, 0x00,"0x0100",eReadWrite},
 	{0x00 ,0x00,"eTableEnd",eTableEnd}

};

const IsiRegDescription_t Sensor_g_twolane_resolution_2592_1944[] =
{
	{0xfe,0x03,"0x0100",eReadWrite},
	{0x10,0x91,"0x0100",eReadWrite},
	{0xfe,0x00,"0x0100",eReadWrite},
	{0x0000 ,0x00,"eTableEnd",eTableEnd}

};

const IsiRegDescription_t Sensor_g_2592x1944P30_twolane_fpschg[] =
{
	{0xfe,0x00,"0x0100",eReadWrite},
	{0x03,0x07,"0x0100",eReadWrite},
	{0x04,0xc0,"0x0100",eReadWrite},
	{0x0000 ,0x00,"eTableEnd",eTableEnd}
};
const IsiRegDescription_t Sensor_g_2592x1944P25_twolane_fpschg[] =
{
	{0xfe,0x00,"0x0100",eReadWrite},
	{0x03,0x09,"0x0100",eReadWrite},
	{0x04,0x42,"0x0100",eReadWrite},
	{0x0000 ,0x00,"eTableEnd",eTableEnd}
};

const IsiRegDescription_t Sensor_g_2592x1944P20_twolane_fpschg[] =
{
	{0xfe,0x00,"0x0100",eReadWrite},
	{0x03,0x0b,"0x0100",eReadWrite},
	{0x04,0xa0,"0x0100",eReadWrite},
	{0x0000 ,0x00,"eTableEnd",eTableEnd}
};
const IsiRegDescription_t Sensor_g_2592x1944P15_twolane_fpschg[] =
{
	{0xfe,0x00,"0x0100",eReadWrite},
	{0x03,0x0f,"0x0100",eReadWrite},
	{0x04,0x80,"0x0100",eReadWrite},
	{0x0000 ,0x00,"eTableEnd",eTableEnd}
};
const IsiRegDescription_t Sensor_g_2592x1944P10_twolane_fpschg[] =
{
	{0xfe,0x00,"0x0100",eReadWrite},
	{0x03,0x17,"0x0100",eReadWrite},
	{0x04,0x40,"0x0100",eReadWrite},
	{0x0000 ,0x00,"eTableEnd",eTableEnd}
};

const IsiRegDescription_t Sensor_g_2592x1944P7_twolane_fpschg[] =
{
	{0xfe,0x00,"0x0100",eReadWrite},
	{0x03,0x21,"0x0100",eReadWrite},
	{0x04,0x36,"0x0100",eReadWrite},
	{0x0000 ,0x00,"eTableEnd",eTableEnd}
};


