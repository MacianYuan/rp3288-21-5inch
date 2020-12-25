//OV7251_tables.c
/*****************************************************************************/
/*!
 *  \file        OV7251_tables.c \n
 *  \version     1.0 \n
 *  \author      Meinicke \n
 *  \brief       Image-sensor-specific tables and other
 *               constant values/structures for OV7251. \n
 *
 *  \revision    $Revision: 803 $ \n
 *               $Author: $ \n
 *               $Date: 2016-9-30 09:37:08  $ \n
 *               $Id: OV7251_tables.c 2016-9-30 09:36:57   \n
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

#if( OV7251_DRIVER_USAGE == USE_CAM_DRV_EN )
*/


#include <ebase/types.h>
#include <ebase/trace.h>
#include <ebase/builtins.h>

#include <common/return_codes.h>

#include "isi.h"
#include "isi_iss.h"
#include "isi_priv.h"
#include "OV7251_MIPI_priv.h"


/*****************************************************************************
 * DEFINES
 *****************************************************************************/


/*****************************************************************************
 * GLOBALS
 *****************************************************************************/

// Image sensor register settings default values taken from data sheet OV13850_DS_1.1_SiliconImage.pdf.
// The settings may be altered by the code in IsiSetupSensor.

//one lane
const IsiRegDescription_t Sensor_g_aRegDescription_onelane[] =
{
	//XVCLK=24Mhz, SCLK=4x120Mhz, MIPI 640Mbps, DACCLK=240Mhz

	{0x0000 ,0x00,"eTableEnd",eTableEnd}

};

const IsiRegDescription_t Sensor_g_onelane_resolution_640_480[] =
{
    //XVCLK=24Mhz,640x480@100fps,raw10,MIPI CLK=800MHZ
	{0x0103 ,0x01,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// software reset
	{0x0100 ,0x00,"0x0100",eReadWrite},// ,"0x0100",eReadWrite},// PLL
	{0x3005 ,0x00,"0x0100",eReadWrite},// ,"0x0100",eReadWrite},// PLL
	{0x3012 ,0xc0,"0x0100",eReadWrite},// ,"0x0100",eReadWrite},// PLL
	{0x3013 ,0xd2,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// PLL
	{0x3014 ,0x04,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// PLL
	{0x3016 ,0xf0,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// MIPI 10-bit mode
	{0x3017 ,0xf0,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// MIPI PHY
	{0x3018 ,0xf0,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// MIPI PHY
	{0x301a ,0xf0,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// MIPI 4 lane
	{0x301b ,0xf0,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// MIPI control
	{0x301c ,0xf0,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// MIPI control
	{0x3023 ,0x07,"0x0100",eReadWrite},// 
	{0x3037 ,0xf0,"0x0100",eReadWrite},// 
	{0x3098 ,0x04,"0x0100",eReadWrite},// 
	{0x3099 ,0x28,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// exposure HH
	{0x309a ,0x05,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// exposure H
	{0x309b ,0x04,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// exposure L
	{0x30b0 ,0x0a,"0x0100",eReadWrite},//  08, 8bit raw
	{0x30b1 ,0x01,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// short exposure H
	{0x30b3 ,0x64,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// shour exposure L
	{0x30b4 ,0x03,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// gain H
	{0x30b5 ,0x05,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// gain L
	{0x3106 ,0x12,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// short gain H
	{0x3500 ,0x00,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// short gain L
	{0x3501 ,0x1f,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// analog control
	{0x3502 ,0x80,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// analog control
	{0x3503 ,0x07,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// analog control
	{0x3509 ,0x10,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// analog control
	{0x350b ,0x20,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// analog control
	{0x3600 ,0x1c,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// analog control
	{0x3602 ,0x62,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// analog control
	{0x3620 ,0xb7,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// analog control
	{0x3622 ,0x04,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// analog control
	{0x3626 ,0x21,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// analog control
	{0x3627 ,0x30,"0x0100",eReadWrite},// 
	{0x3630 ,0x44,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// PLL2
	{0x3631 ,0x35,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// PLL2
	{0x3634 ,0x60,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// PLL2
	{0x3636 ,0x00,"0x0100",eReadWrite},// 
	{0x3662 ,0x01,"0x0100",eReadWrite},// 03, 8 bit raw
	{0x3664 ,0xf0,"0x0100",eReadWrite},// 
	{0x3669 ,0x1a,"0x0100",eReadWrite},// 
	{0x366a ,0x00,"0x0100",eReadWrite},//
	{0x366b ,0x50,"0x0100",eReadWrite},// 
	{0x3705 ,0xc1,"0x0100",eReadWrite},// 
	{0x3709 ,0x40,"0x0100",eReadWrite},// 
	{0x373c ,0x08,"0x0100",eReadWrite},// 
	{0x3742 ,0x00,"0x0100",eReadWrite},// 
	{0x3757 ,0xb3,"0x0100",eReadWrite},// 
	{0x3788 ,0x00,"0x0100",eReadWrite},// 
	{0x37a8 ,0x01,"0x0100",eReadWrite},// 
	{0x37a9 ,0xc0,"0x0100",eReadWrite},// 
	{0x3800 ,0x00,"0x0100",eReadWrite},// 
	{0x3801 ,0x04,"0x0100",eReadWrite},// 
	{0x3802 ,0x00,"0x0100",eReadWrite},// 
	{0x3803 ,0x04,"0x0100",eReadWrite},// 
	{0x3804 ,0x02,"0x0100",eReadWrite},// 
	{0x3805 ,0x8b,"0x0100",eReadWrite},// 
	{0x3806 ,0x01,"0x0100",eReadWrite},// 
	{0x3807 ,0xeb,"0x0100",eReadWrite},// 
	{0x3808 ,0x02,"0x0100",eReadWrite},// 
	{0x3809 ,0x80,"0x0100",eReadWrite},// 
	{0x380a ,0x01,"0x0100",eReadWrite},// 
	{0x380b ,0xe0,"0x0100",eReadWrite},// 
	{0x380c ,0x03,"0x0100",eReadWrite},// 
	{0x380d ,0xa0,"0x0100",eReadWrite},// 
	{0x380e ,0x02,"0x0100",eReadWrite},// 
	{0x380f ,0x04,"0x0100",eReadWrite},// 
	{0x3810 ,0x00,"0x0100",eReadWrite},// 
	{0x3811 ,0x04 ,"0x0100",eReadWrite},//
	{0x3812 ,0x00,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// OTP program disable
	{0x3813 ,0x05,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// OTP power up load data enable, power load setting enable, software load setting enable
	{0x3814 ,0x11,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// OTP start address H
	{0x3815 ,0x11,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// OTP start address L
	{0x3820 ,0x40,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// H crop start H
	{0x3821 ,0x00,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// H crop start L
	{0x382f ,0xc4,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// V crop start H
	{0x3832 ,0xff,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// V crop start L
	{0x3833 ,0xff,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// H crop end H
	{0x3834 ,0x00,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// H crop end L
	{0x3835 ,0x05,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// V crop end H
	{0x3837 ,0x00,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// V crop end L
	{0x3b80 ,0x00,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// H output size H
	{0x3b81 ,0xa5,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// H output size L
	{0x3b82 ,0x10,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// V output size H
	{0x3b83 ,0x00,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// V output size L
	{0x3b84 ,0x08,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// HTS H
	{0x3b85 ,0x00,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// HTS L
	{0x3b86 ,0x01,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// VTS H
	{0x3b87 ,0x00,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// VTS L
	{0x3b88 ,0x00,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// H win off H
	{0x3b89 ,0x00,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// H win off L
	{0x3b8a ,0x00,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// V win off H
	{0x3b8b ,0x05,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// V win off L
	{0x3b8c ,0x00,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// H inc
	{0x3b8d ,0x00,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// V inc
	{0x3b8e ,0x00,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// V flip off, V bin on
	{0x3b8f ,0x1a,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// H mirror on, H bin on
	{0x3b94 ,0x05,"0x0100",eReadWrite},// 
	{0x3b95 ,0xf2,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// cut_en, vts_auto, blk_col_dis
	{0x3b96 ,0x40,"0x0100",eReadWrite},//
	{0x3c00 ,0x89,"0x0100",eReadWrite},// 
	{0x3c01 ,0xab,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// BLC offset trig en, format change trig en, gain trig en, exp trig en, median en
	{0x3c02 ,0x01,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// BLC
	{0x3c03 ,0x00,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// BLC
	{0x3c04 ,0x00,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// BLC
	{0x3c05 ,0x03,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// BLC
	{0x3c06 ,0x00,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// BLC
	{0x3c07 ,0x05,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// BLC
	{0x3c0c ,0x00,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// BLC
	{0x3c0d ,0x00,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// BLC
	{0x3c0e ,0x00,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// BLC
	{0x3c0f ,0x00,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// BLC
	{0x4001 ,0xc2,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// BLC
	{0x4004 ,0x04,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// BLC
	{0x4005 ,0x20,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// BLC
	{0x404e ,0x01,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// BLC
	{0x4300 ,0xff,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// BLC
	{0x4301 ,0x00,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// BLC
	{0x4600 ,0x00,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// BLC
	{0x4601 ,0x4e,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// BLC
	{0x4801 ,0x0f,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// BLC
	{0x4806 ,0x0f,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// BLC
	{0x4819 ,0xaa,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// BLC
	{0x4823 ,0x3e,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// BLC
	{0x4837 ,0x19,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// BLC
	{0x4a0d ,0x00,"0x0100",eReadWrite},// 
	{0x5000 ,0x85,"0x0100",eReadWrite},//  
	{0x5001 ,0x80,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// BLC
	{0x0100 ,0x01,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// BLC
	{0x0000 ,0x00,"eTableEnd",eTableEnd}
};


const IsiRegDescription_t Sensor_g_onelane_resolution_320_240[] =
{
	//320x240  200fps  raw8
	{0x0103 ,0x01,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// software reset
	{0x0100 ,0x00,"0x0100",eReadWrite},// ,"0x0100",eReadWrite},// PLL
	{0x3005 ,0x00,"0x0100",eReadWrite},// ,"0x0100",eReadWrite},// PLL
	{0x3012 ,0xc0,"0x0100",eReadWrite},// ,"0x0100",eReadWrite},// PLL
	{0x3013 ,0xd2,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// PLL
	{0x3014 ,0x04,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// PLL
	{0x3016 ,0x10,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// MIPI 10-bit mode
	{0x3017 ,0x00,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// MIPI PHY
	{0x3018 ,0x00,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// MIPI PHY
	{0x301a ,0x00,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// MIPI 4 lane
	{0x301b ,0x00,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// MIPI control
	{0x301c ,0x00,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// MIPI control
	{0x3023 ,0x05,"0x0100",eReadWrite},// 
	{0x3037 ,0xf0,"0x0100",eReadWrite},// 
	{0x3098 ,0x04,"0x0100",eReadWrite},// 
	{0x3099 ,0x28,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// exposure HH
	{0x309a ,0x05,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// exposure H
	{0x309b ,0x04,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// exposure L
	{0x30b0 ,0x0a,"0x0100",eReadWrite},//  08, 8 bit raw
	{0x30b1 ,0x01,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// short exposure H
	{0x30b3 ,0x64,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// shour exposure L
	{0x30b4 ,0x03,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// gain H
	{0x30b5 ,0x05,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// gain L
	{0x3106 ,0xda,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// short gain H
	{0x3500 ,0x00,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// short gain L
	{0x3501 ,0x02,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// analog control
	{0x3502 ,0x00,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// analog control
	{0x3503 ,0x07,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// analog control
	{0x3509 ,0x10,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// analog control
	{0x350b ,0x10,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// analog control
	{0x3600 ,0x1c,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// analog control
	{0x3602 ,0x62,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// analog control
	{0x3620 ,0xb7,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// analog control
	{0x3622 ,0x04,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// analog control
	{0x3626 ,0x21,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// analog control
	{0x3627 ,0x30,"0x0100",eReadWrite},// 
	{0x3630 ,0x44,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// PLL2
	{0x3631 ,0x35,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// PLL2
	{0x3634 ,0x60,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// PLL2
	{0x3636 ,0x00,"0x0100",eReadWrite},// 
	{0x3662 ,0x01,"0x0100",eReadWrite},// 
	{0x3663 ,0x70,"0x0100",eReadWrite},// 
	{0x3664 ,0xf0,"0x0100",eReadWrite},//
	{0x3666 ,0x0a,"0x0100",eReadWrite},//
	{0x3669 ,0x1a,"0x0100",eReadWrite},// 
	{0x366a ,0x00,"0x0100",eReadWrite},//
	{0x366b ,0x50,"0x0100",eReadWrite},// 
	{0x3673 ,0x01,"0x0100",eReadWrite},// 
	{0x3674 ,0xef,"0x0100",eReadWrite},// 
	{0x3675 ,0x03,"0x0100",eReadWrite},// 	
	{0x3705 ,0x41,"0x0100",eReadWrite},// 
	{0x3709 ,0x40,"0x0100",eReadWrite},// 
	{0x373c ,0xe8,"0x0100",eReadWrite},// 
	{0x3742 ,0x00,"0x0100",eReadWrite},// 
	{0x3757 ,0xb3,"0x0100",eReadWrite},// 
	{0x3788 ,0x00,"0x0100",eReadWrite},// 
	{0x37a8 ,0x02,"0x0100",eReadWrite},// 
	{0x37a9 ,0x14,"0x0100",eReadWrite},// 
	{0x3800 ,0x00,"0x0100",eReadWrite},// 
	{0x3801 ,0x04,"0x0100",eReadWrite},// 
	{0x3802 ,0x00,"0x0100",eReadWrite},// 
	{0x3803 ,0x00,"0x0100",eReadWrite},// 
	{0x3804 ,0x02,"0x0100",eReadWrite},// 
	{0x3805 ,0x8b,"0x0100",eReadWrite},// 
	{0x3806 ,0x01,"0x0100",eReadWrite},// 
	{0x3807 ,0xef,"0x0100",eReadWrite},// 
	{0x3808 ,0x01,"0x0100",eReadWrite},// 
	{0x3809 ,0x40,"0x0100",eReadWrite},// 
	{0x380a ,0x00,"0x0100",eReadWrite},// 
	{0x380b ,0xf0,"0x0100",eReadWrite},// 
	{0x380c ,0x03,"0x0100",eReadWrite},// 
	{0x380d ,0x04,"0x0100",eReadWrite},// 
	{0x380e ,0x01,"0x0100",eReadWrite},// 
	{0x380f ,0x30,"0x0100",eReadWrite},// 
	{0x3810 ,0x00,"0x0100",eReadWrite},// 
	{0x3811 ,0x04,"0x0100",eReadWrite},//
	{0x3812 ,0x00,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// OTP program disable
	{0x3813 ,0x05,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// OTP power up load data enable, power load setting enable, software load setting enable
	{0x3814 ,0x31,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// OTP start address H
	{0x3815 ,0x31,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// OTP start address L
	{0x3820 ,0x42,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// H crop start H
	{0x3821 ,0x01,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// H crop start L
	{0x382f ,0x0e,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// V crop start H
	{0x3832 ,0x00,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// V crop start L
	{0x3833 ,0x05,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// H crop end H
	{0x3834 ,0x00,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// H crop end L
	{0x3835 ,0x0c,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// V crop end H
	{0x3837 ,0x00,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// V crop end L
	{0x3b80 ,0x00,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// H output size H
	{0x3b81 ,0xa5,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// H output size L
	{0x3b82 ,0x10,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// V output size H
	{0x3b83 ,0x00,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// V output size L
	{0x3b84 ,0x08,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// HTS H
	{0x3b85 ,0x00,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// HTS L
	{0x3b86 ,0x01,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// VTS H
	{0x3b87 ,0x00,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// VTS L
	{0x3b88 ,0x00,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// H win off H
	{0x3b89 ,0x00,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// H win off L
	{0x3b8a ,0x00,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// V win off H
	{0x3b8b ,0x05,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// V win off L
	{0x3b8c ,0x00,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// H inc
	{0x3b8d ,0x00,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// V inc
	{0x3b8e ,0x00,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// V flip off, V bin on
	{0x3b8f ,0x1a,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// H mirror on, H bin on
	{0x3b94 ,0x05,"0x0100",eReadWrite},// 
	{0x3b95 ,0xf2,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// cut_en, vts_auto, blk_col_dis
	{0x3b96 ,0x40,"0x0100",eReadWrite},//
	{0x3c00 ,0x89,"0x0100",eReadWrite},// 
	{0x3c01 ,0x63,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// BLC offset trig en, format change trig en, gain trig en, exp trig en, median en
	{0x3c02 ,0x01,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// BLC
	{0x3c03 ,0x00,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// BLC
	{0x3c04 ,0x00,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// BLC
	{0x3c05 ,0x03,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// BLC
	{0x3c06 ,0x00,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// BLC
	{0x3c07 ,0x06,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// BLC
	{0x3c0c ,0x01,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// BLC
	{0x3c0d ,0x82,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// BLC
	{0x3c0e ,0x01,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// BLC
	{0x3c0f ,0x30,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// BLC
	{0x4001 ,0x40,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// BLC
	{0x4004 ,0x02,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// BLC
	{0x4005 ,0x00,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// BLC
	{0x404e ,0x01,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// BLC
	{0x4300 ,0xff,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// BLC
	{0x4301 ,0x00,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// BLC
    {0x4501 ,0x48,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// BLC
	{0x4600 ,0x00,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// BLC
	{0x4601 ,0x4e,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// BLC
	{0x4801 ,0x0f,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// BLC
	{0x4806 ,0x0f,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// BLC
	{0x4819 ,0xaa,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// BLC
	{0x4823 ,0x3e,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// BLC
	{0x4837 ,0x19,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// BLC
	{0x4a0d ,0x00,"0x0100",eReadWrite},// 
	{0x4a47 ,0x7f,"0x0100",eReadWrite},// 
    {0x4a49 ,0xf0,"0x0100",eReadWrite},// 
    {0x4a4b ,0x30,"0x0100",eReadWrite},// 
	{0x5000 ,0x85,"0x0100",eReadWrite},//  
	{0x5001 ,0x80,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// BLC
	{0x0100 ,0x01,"0x0100",eReadWrite},//  ,"0x0100",eReadWrite},// BLC
    {0x0000 ,0x00,"eTableEnd",eTableEnd}
};

