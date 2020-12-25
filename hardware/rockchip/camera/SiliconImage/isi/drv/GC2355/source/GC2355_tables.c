//GC2355_tables.c
/*****************************************************************************/
/*!
 *  \file        GC2355_tables.c \n
 *  \version     1.0 \n
 *  \author      Meinicke \n
 *  \brief       Image-sensor-specific tables and other
 *               constant values/structures for OV13850. \n
 *
 *  \revision    $Revision: 803 $ \n
 *               $Author: $ \n
 *               $Date: 2010-02-26 16:35:22 +0100 (Fr, 26 Feb 2010) $ \n
 *               $Id: GC2355_tables.c 803 2010-02-26 15:35:22Z  $ \n
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

#if( GC2355_DRIVER_USAGE == USE_CAM_DRV_EN )
*/


#include <ebase/types.h>
#include <ebase/trace.h>
#include <ebase/builtins.h>

#include <common/return_codes.h>

#include "isi.h"
#include "isi_iss.h"
#include "isi_priv.h"
#include "GC2355_MIPI_priv.h"


/*****************************************************************************
 * DEFINES
 *****************************************************************************/


/*****************************************************************************
 * GLOBALS
 *****************************************************************************/

// Image sensor register settings default values taken from data sheet GC2355_DS_1.1.pdf.
// The settings may be altered by the code in IsiSetupSensor.

//one lane
const IsiRegDescription_t GC2355_g_aRegDescription[] =
{
/////////////////////////////////////////////////////
//////////////////////  SYS  //////////////////////
/////////////////////////////////////////////////////
	{0xfe, 0x80,"",eReadWrite},
	{0xfe, 0x80,"",eReadWrite},
	{0xfe, 0x80,"",eReadWrite},
	{0xf2, 0x00,"",eReadWrite}, //sync_pad_io_ebi
	{0xf6, 0x00,"",eReadWrite}, //up down
	{0xfc, 0x06,"",eReadWrite},
	{0xf7, 0x19,"",eReadWrite}, //19 //clk_double pll enable
	{0xf8, 0x06,"",eReadWrite}, //Pll mode 2
	{0xf9, 0x0e,"",eReadWrite}, //de//[0] pll enable
	{0xfa, 0x00,"",eReadWrite}, //div
	{0xfe, 0x00,"",eReadWrite},

/////////////////////////////////////////////////////
////////////////  ANALOG & CISCTL  ////////////////
/////////////////////////////////////////////////////
	{0x03, 0x04,"",eReadWrite},
	{0x04, 0x5f,"",eReadWrite},
	{0x05, 0x01,"",eReadWrite}, //HB
	{0x06, 0x22,"",eReadWrite},
	{0x07, 0x00,"",eReadWrite}, //VB
	{0x08, 0x0b,"",eReadWrite},
	{0x0a, 0x00,"",eReadWrite}, //row start
	{0x0c, 0x04,"",eReadWrite}, //0c//col start
	{0x0d, 0x04,"",eReadWrite},
	{0x0e, 0xc0,"",eReadWrite},
	{0x0f, 0x06,"",eReadWrite}, 
	{0x10, 0x50,"",eReadWrite}, //Window setting 1616x1216
	{0x17, 0x14,"",eReadWrite},
	{0x19, 0x0b,"",eReadWrite}, //09
	{0x1b, 0x49,"",eReadWrite}, //48
	{0x1c, 0x12,"",eReadWrite},
	{0x1d, 0x10,"",eReadWrite}, //double reset
	{0x1e, 0xbc,"",eReadWrite}, //a8//col_r/rowclk_mode/rsthigh_en FPN
	{0x1f, 0xc8,"",eReadWrite}, //08//rsgl_s_mode/vpix_s_mode 灯管横条纹
	{0x20, 0x71,"",eReadWrite},
	{0x21, 0x20,"",eReadWrite}, //rsg
	{0x22, 0xa0,"",eReadWrite},
	{0x23, 0x51,"",eReadWrite}, //01
	{0x24, 0x19,"",eReadWrite}, //0b //55
	{0x27, 0x20,"",eReadWrite}, //灯管横条纹
	{0x28, 0x00,"",eReadWrite},
	{0x2b, 0x81,"",eReadWrite}, //80 //00 sf_s_mode FPN
	{0x2c, 0x38,"",eReadWrite}, //50 //5c ispg FPN //去黑太阳
	{0x2e, 0x16,"",eReadWrite}, //05//eq width 
	{0x2f, 0x14,"",eReadWrite}, //[3:0]tx_width 写0能改麻点
	{0x30, 0x00,"",eReadWrite},
	{0x31, 0x01,"",eReadWrite},
	{0x32, 0x02,"",eReadWrite},
	{0x33, 0x03,"",eReadWrite},
	{0x34, 0x07,"",eReadWrite},
	{0x35, 0x0b,"",eReadWrite},
	{0x36, 0x0f,"",eReadWrite},

/////////////////////////////////////////////////////
//////////////////////	 gain	/////////////////////
/////////////////////////////////////////////////////
	{0xb0, 0x50,"",eReadWrite}, //1.25x
	{0xb1, 0x02,"",eReadWrite},
	{0xb2, 0xe0,"",eReadWrite}, //2.86x
	{0xb3, 0x40,"",eReadWrite},
	{0xb4, 0x40,"",eReadWrite},
	{0xb5, 0x40,"",eReadWrite},
	{0xb6, 0x03,"",eReadWrite}, //2.8x

/////////////////////////////////////////////////////
//////////////////////	 crop	/////////////////////
/////////////////////////////////////////////////////
	{0x92, 0x02,"",eReadWrite},
	{0x95, 0x04,"",eReadWrite},
	{0x96, 0xb0,"",eReadWrite},
	{0x97, 0x06,"",eReadWrite},
	{0x98, 0x40,"",eReadWrite}, //out window set 1600x1200

/////////////////////////////////////////////////////
//////////////////////	BLK	/////////////////////
/////////////////////////////////////////////////////
	{0x18, 0x02,"",eReadWrite},
	{0x1a, 0x01,"",eReadWrite},
	{0x40, 0x42,"",eReadWrite},
	{0x41, 0x00,"",eReadWrite},

	{0x44, 0x00,"",eReadWrite},
	{0x45, 0x00,"",eReadWrite},
	{0x46, 0x00,"",eReadWrite},
	{0x47, 0x00,"",eReadWrite},
	{0x48, 0x00,"",eReadWrite},
	{0x49, 0x00,"",eReadWrite},
	{0x4a, 0x00,"",eReadWrite},
	{0x4b, 0x00,"",eReadWrite}, //clear offset

	{0x4e, 0x3c,"",eReadWrite}, //BLK select
	{0x4f, 0x00,"",eReadWrite}, 
	{0x5e, 0x00,"",eReadWrite}, //offset ratio
	{0x66, 0x20,"",eReadWrite}, //dark ratio

	{0x6a, 0x02,"",eReadWrite},
	{0x6b, 0x02,"",eReadWrite},
	{0x6c, 0x00,"",eReadWrite},
	{0x6d, 0x00,"",eReadWrite},
	{0x6e, 0x00,"",eReadWrite},
	{0x6f, 0x00,"",eReadWrite},
	{0x70, 0x02,"",eReadWrite},
	{0x71, 0x02,"",eReadWrite}, //manual offset

/////////////////////////////////////////////////////
//////////////////  Dark sun  /////////////////////
/////////////////////////////////////////////////////
	{0x87, 0x03,"",eReadWrite}, //
	{0xe0, 0xe7,"",eReadWrite}, //dark sun en/extend mode
	{0xe3, 0xc0,"",eReadWrite}, //clamp

/////////////////////////////////////////////////////
//////////////////////	 MIPI	/////////////////////
/////////////////////////////////////////////////////
	{0xfe, 0x03,"",eReadWrite},
	{0x01, 0x83,"",eReadWrite}, //0x87 2lane
	{0x02, 0x00,"",eReadWrite},
	{0x03, 0x90,"",eReadWrite},
	{0x04, 0x01,"",eReadWrite},
	{0x05, 0x00,"",eReadWrite},
	{0x06, 0xa2,"",eReadWrite},
	{0x10, 0x00,"",eReadWrite}, //94//1lane raw8
	{0x11, 0x2b,"",eReadWrite},
	{0x12, 0xd0,"",eReadWrite},
	{0x13, 0x07,"",eReadWrite},

/* p3:0x15 [1:0]clklane_mode
 * 00 : Enter LP mode between Frame; 
 * 01: Enter LP mode between Row; 
 * 10: Continuous HS mode 
 */
	{0x15, 0x60,"",eReadWrite},
	{0x21, 0x10,"",eReadWrite},
	{0x22, 0x05,"",eReadWrite},
	{0x23, 0x30,"",eReadWrite},
	{0x24, 0x02,"",eReadWrite},
	{0x25, 0x15,"",eReadWrite},
	{0x26, 0x08,"",eReadWrite},
	{0x27, 0x06,"",eReadWrite},
	{0x29, 0x06,"",eReadWrite},
	{0x2a, 0x0a,"",eReadWrite},
	{0x2b, 0x08,"",eReadWrite},
	{0x40, 0x00,"",eReadWrite},
	{0x41, 0x00,"",eReadWrite},
	{0x42, 0x40,"",eReadWrite},
	{0x43, 0x06,"",eReadWrite},
	{0xfe, 0x00,"",eReadWrite},
    {0x0000 ,0x00,"eTableEnd",eTableEnd}
};

const IsiRegDescription_t GC2355_g_svga[] =
{
    {0x0000 ,0x00,"eTableEnd",eTableEnd}
};

const IsiRegDescription_t GC2355_g_1600x1200[] =
{
    {0x0000 ,0x00,"eTableEnd",eTableEnd}

};

const IsiRegDescription_t GC2355_g_1600x1200_30fps[] =
{
    {0xfe, 0x00, "eReadWrite",eReadWrite},
	{0x03, 0x04, "eReadWrite",eReadWrite},// VTS H
	{0x04, 0xD9, "eReadWrite",eReadWrite},// VTS L
	{0x0000 ,0x00,"eTableEnd",eTableEnd}
};
const IsiRegDescription_t GC2355_g_1600x1200_20fps[] =
{
    {0xfe, 0x00, "eReadWrite",eReadWrite},
	{0x03, 0x07, "eReadWrite",eReadWrite},// VTS H
	{0x04, 0x4a, "eReadWrite",eReadWrite},// VTS L
	{0x0000 ,0x00,"eTableEnd",eTableEnd}
};
const IsiRegDescription_t GC2355_g_1600x1200_15fps[] =
{
    {0xfe, 0x00, "eReadWrite",eReadWrite},
	{0x03, 0x09, "eReadWrite",eReadWrite},// VTS H
	{0x04, 0xb7, "eReadWrite",eReadWrite},// VTS L
	{0x0000 ,0x00,"eTableEnd",eTableEnd}
};

const IsiRegDescription_t GC2355_g_1600x1200_10fps[] =
{
    {0xfe, 0x00, "eReadWrite",eReadWrite},
	{0x03, 0x0e, "eReadWrite",eReadWrite},// VTS H
	{0x04, 0x93, "eReadWrite",eReadWrite},// VTS L
	{0x0000 ,0x00,"eTableEnd",eTableEnd}
};
