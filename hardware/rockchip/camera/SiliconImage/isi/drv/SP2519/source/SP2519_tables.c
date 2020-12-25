#include <ebase/types.h>
#include <ebase/trace.h>
#include <ebase/builtins.h>

#include <common/return_codes.h>

#include "isi.h"
#include "isi_iss.h"
#include "isi_priv.h"
#include "SP2519_priv.h"

//AE
#define  SP2519_P0_0xf7  0x80//78
#define  SP2519_P0_0xf8  0x74//6e
#define  SP2519_P0_0xf9  0x80//74
#define  SP2519_P0_0xfa  0x74//6a
//HEQ
#define  SP2519_P0_0xdd  0x80
#define  SP2519_P0_0xde  0x95
//auto lum
#define SP2519_NORMAL_Y0ffset  	  0x10	//0x0f	 modify by sp_yjp,20120813
#define SP2519_LOWLIGHT_Y0ffset  0x20

/*****************************************************************************
 * DEFINES
 *****************************************************************************/


/*****************************************************************************
 * GLOBALS
 *****************************************************************************/

// Image sensor register settings default values taken from data sheet OV8810_DS_1.1_SiliconImage.pdf.
// The settings may be altered by the code in IsiSetupSensor.
const IsiRegDescription_t SP2519_g_aRegDescription[] =
{
    {0xfd,0x01,"",eReadWrite},
    {0x36,0x02,"",eReadWrite},
    
    {0xfd,0x00,"",eReadWrite},
    {0x30,0x08,"",eReadWrite},
    {0x2f,0x11,"",eReadWrite},
    {0x09,0x01,"",eReadWrite},
    {0xfd,0x00,"",eReadWrite},
    {0x0c,0x55,"",eReadWrite},
    {0x27,0xa5,"",eReadWrite},
    {0x1a,0x4b,"",eReadWrite},
    {0x20,0x2f,"",eReadWrite},
    {0x22,0x5a,"",eReadWrite},
    {0x25,0xad,"",eReadWrite},
    {0x21,0x0d,"",eReadWrite},
    {0x28,0x08,"",eReadWrite}, // System noise ; increase 0x28 = 0x09~0x0b;no more then 0x0b
    {0x1d,0x01,"",eReadWrite}, // MP platform horizon noise exist; 0x1d=0x00
    {0x7a,0x5d,"",eReadWrite},
    {0x70,0x41,"",eReadWrite},
    {0x74,0x40,"",eReadWrite},
    {0x75,0x40,"",eReadWrite},
    {0x15,0x3e,"",eReadWrite},
    {0x71,0x3f,"",eReadWrite},
    {0x7c,0x3f,"",eReadWrite},
    {0x76,0x3f,"",eReadWrite},
    {0x7e,0x29,"",eReadWrite},
    {0x72,0x29,"",eReadWrite},
    {0x77,0x28,"",eReadWrite},
    {0x1e,0x01,"",eReadWrite},
    {0x1c,0x0f,"",eReadWrite},
    {0x2e,0xc5,"",eReadWrite},
    {0x1f,0xc0,"",eReadWrite},
    {0x6c,0x00,"",eReadWrite},
    {0xfd,0x01,"",eReadWrite},
    {0x32,0x00,"",eReadWrite},
    {0xfd,0x02,"",eReadWrite},
    {0x85,0x00,"",eReadWrite},

// ae setting pll 2.5 8-10fps
    {  0xfd,0x00,"",eReadWrite},
    {  0x03,0x02,"",eReadWrite},
    {  0x04,0xe8,"",eReadWrite},
    {  0x05,0x00,"",eReadWrite},
    {  0x06,0x00,"",eReadWrite},
    {  0x07,0x00,"",eReadWrite},
    {  0x08,0x00,"",eReadWrite},
    {  0x09,0x01,"",eReadWrite},
    {  0x0a,0x20,"",eReadWrite},
    {  0xfd,0x01,"",eReadWrite},
    {  0xf0,0x00,"",eReadWrite},
    {  0xf7,0x7c,"",eReadWrite},
    {  0xf8,0x67,"",eReadWrite},
    {  0x02,0x0c,"",eReadWrite},
    {  0x03,0x01,"",eReadWrite},
    {  0x06,0x7c,"",eReadWrite},
    {  0x07,0x00,"",eReadWrite},
    {  0x08,0x01,"",eReadWrite},
    {  0x09,0x00,"",eReadWrite},
    {  0xfd,0x02,"",eReadWrite},
    {  0x3d,0x0f,"",eReadWrite},
    {  0x3e,0x67,"",eReadWrite},
    {  0x3f,0x00,"",eReadWrite},
    {  0x88,0x21,"",eReadWrite},
    {  0x89,0xf8,"",eReadWrite},
    {  0x8a,0x44,"",eReadWrite},
    {  0xfd,0x02,"",eReadWrite},
    {  0xbe,0xd0,"",eReadWrite},
    {  0xbf,0x05,"",eReadWrite},
    {  0xd0,0xd0,"",eReadWrite},
    {  0xd1,0x05,"",eReadWrite},
    {  0xc9,0xd0,"",eReadWrite},
    {  0xca,0x05,"",eReadWrite},

    {0xb8,0x70,"",eReadWrite}, // mean_nr_dummy
    {0xb9,0x80,"",eReadWrite}, // mean_dummy_nr
    {0xba,0x30,"",eReadWrite}, // mean_dummy_low
    {0xbb,0x45,"",eReadWrite}, // mean_low_dummy
    {0xbc,0x90,"",eReadWrite}, // rpc_heq_low
    {0xbd,0x70,"",eReadWrite}, // rpc_heq_dummy
    {0xfd,0x03,"",eReadWrite},
    {0x77,0x48,"",eReadWrite}, // rpc_heq_nr2
// rpc
    {0xfd,0x01,"",eReadWrite},
    {0xe0,0x48,"",eReadWrite},
    {0xe1,0x38,"",eReadWrite},
    {0xe2,0x30,"",eReadWrite},
    {0xe3,0x2c,"",eReadWrite},
    {0xe4,0x2c,"",eReadWrite},
    {0xe5,0x2a,"",eReadWrite},
    {0xe6,0x2a,"",eReadWrite},
    {0xe7,0x28,"",eReadWrite},
    {0xe8,0x28,"",eReadWrite},
    {0xe9,0x28,"",eReadWrite},
    {0xea,0x26,"",eReadWrite},
    {0xf3,0x26,"",eReadWrite},
    {0xf4,0x26,"",eReadWrite},
    {0xfd,0x01,"",eReadWrite}, // ae min gain
    {0x04,0xc0,"",eReadWrite}, // rpc_max_indr
    {0x05,0x26,"",eReadWrite}, // rpc_min_indr
    {0x0a,0x48,"",eReadWrite}, // rpc_max_outdr
    {0x0b,0x26,"",eReadWrite}, // rpc_min_outdr

    {0xfd,0x01,"",eReadWrite}, // ae target
    {0xf2,0x09,"",eReadWrite},
    {0xeb,0x78,"",eReadWrite}, // target_indr
    {0xec,0x78,"",eReadWrite}, // target_outdr
    {0xed,0x06,"",eReadWrite}, // lock_range
    {0xee,0x0a,"",eReadWrite}, // hold_range
    {0xfd,0x02,"",eReadWrite},
    {0x4f,0x46,"",eReadWrite}, // dem_morie_thr

//remove bad pixel
    {0xfd,0x03,"",eReadWrite},
    {0x52,0xff,"",eReadWrite}, // dpix_wht_ofst_outdoor
    {0x53,0x60,"",eReadWrite}, // dpix_wht_ofst_normal1
    {0x94,0x00,"",eReadWrite},  // dpix_wht_ofst_normal2
    {0x54,0x00,"",eReadWrite}, // dpix_wht_ofst_dummy
    {0x55,0x00,"",eReadWrite}, // dpix_wht_ofst_low

    {0x56,0x80,"",eReadWrite}, // dpix_blk_ofst_outdoor
    {0x57,0x80,"",eReadWrite}, // dpix_blk_ofst_normal1
    {0x95,0x00,"",eReadWrite},  // dpix_blk_ofst_normal2
    {0x58,0x00,"",eReadWrite}, // dpix_blk_ofst_dummy
    {0x59,0x00,"",eReadWrite}, // dpix_blk_ofst_low

    {0x5a,0xf6,"",eReadWrite}, // dpix_wht_ratio
    {0x5b,0x00,"",eReadWrite},
    {0x5c,0x88,"",eReadWrite}, // dpix_blk_ratio
    {0x5d,0x00,"",eReadWrite},
    {0x96,0x00,"",eReadWrite}, // dpix_wht/blk_ratio_nr2

    {0xfd,0x03,"",eReadWrite},
    {0x8a,0x00,"",eReadWrite},
    {0x8b,0x00,"",eReadWrite},
    {0x8c,0xff,"",eReadWrite},

    {0x22,0xff,"",eReadWrite}, // dem_gdif_thr_outdoor
    {0x23,0xff,"",eReadWrite}, // dem_gdif_thr_normal
    {0x24,0xff,"",eReadWrite}, // dem_gdif_thr_dummy
    {0x25,0xff,"",eReadWrite}, // dem_gdif_thr_low

    {0x5e,0xff,"",eReadWrite}, // dem_gwnd_wht_outdoor
    {0x5f,0xff,"",eReadWrite}, // dem_gwnd_wht_normal
    {0x60,0xff,"",eReadWrite}, // dem_gwnd_wht_dummy
    {0x61,0xff,"",eReadWrite}, // dem_gwnd_wht_low
    {0x62,0x00,"",eReadWrite}, // dem_gwnd_blk_outdoor
    {0x63,0x00,"",eReadWrite}, // dem_gwnd_blk_normal
    {0x64,0x00,"",eReadWrite}, // dem_gwnd_blk_dummy
    {0x65,0x00,"",eReadWrite}, // dem_gwnd_blk_low

// lsc
    {0xfd,0x01,"",eReadWrite},
    {0x21,0x00,"",eReadWrite}, // lsc_sig_ru lsc_sig_lu
    {0x22,0x00,"",eReadWrite}, // lsc_sig_rd lsc_sig_ld
    {0x26,0x60,"",eReadWrite}, // lsc_gain_thr
    {0x27,0x14,"",eReadWrite}, // lsc_exp_thrl
    {0x28,0x05,"",eReadWrite}, // lsc_exp_thrh
    {0x29,0x00,"",eReadWrite}, // lsc_dec_fac     进dummy态退shading 功能有问题，需关掉
    {0x2a,0x01,"",eReadWrite}, // lsc_rpc_en lens 衰减自适应

// LSC for CHT813
    {0xfd,0x01,"",eReadWrite},
    {0xa1,0x1D,"",eReadWrite}, // lsc_rsx_l
    {0xa2,0x20,"",eReadWrite}, // lsc_rsx_r
    {0xa3,0x20,"",eReadWrite}, // lsc_rsy_u
    {0xa4,0x1D,"",eReadWrite}, // lsc_rsy_d
    {0xa5,0x1D,"",eReadWrite}, // lsc_gxy_l
    {0xa6,0x1D,"",eReadWrite}, // lsc_gxy_r
    {0xa7,0x22,"",eReadWrite}, // lsc_gxy_l
    {0xa8,0x1b,"",eReadWrite}, // lsc_gxy_r
    {0xa9,0x1c,"",eReadWrite}, // lsc_bsx_l
    {0xaa,0x1e,"",eReadWrite}, // lsc_bsx_r
    {0xab,0x1e,"",eReadWrite}, // lsc_bsy_u
    {0xac,0x1c,"",eReadWrite}, // lsc_bsy_d
    {0xad,0x0a,"",eReadWrite}, // lsc_rxy_lu
    {0xae,0x09,"",eReadWrite}, // lsc_rxy_ru
    {0xaf,0x05,"",eReadWrite}, // lsc_rxy_ld
    {0xb0,0x05,"",eReadWrite}, // lsc_rxy_rd
    {0xb1,0x0A,"",eReadWrite}, // lsc_gsx_lu
    {0xb2,0x0a,"",eReadWrite}, // lsc_gsx_ru
    {0xb3,0x05,"",eReadWrite}, // lsc_gsy_ud
    {0xb4,0x07,"",eReadWrite}, // lsc_gsy_dd
    {0xb5,0x0A,"",eReadWrite}, // lsc_bxy_lu
    {0xb6,0x0a,"",eReadWrite}, // lsc_bxy_ru
    {0xb7,0x04,"",eReadWrite}, // lsc_bxy_ld
    {0xb8,0x07,"",eReadWrite}, // lsc_bxy_rd

// awb
    {0xfd,0x02,"",eReadWrite},
    {0x26,0xa0,"",eReadWrite}, // Red channel gain
    {0x27,0x96,"",eReadWrite}, // Blue channel gain
    {0x28,0xcc,"",eReadWrite}, // Y top value limit
    {0x29,0x01,"",eReadWrite}, // Y bot value limit
    {0x2a,0x00,"",eReadWrite}, // rg_limit_log
    {0x2b,0x00,"",eReadWrite}, // bg_limit_log
    {0x2c,0x20,"",eReadWrite}, // Awb image center row start
    {0x2d,0xdc,"",eReadWrite}, // Awb image center row end
    {0x2e,0x20,"",eReadWrite}, // Awb image center col start
    {0x2f,0x96,"",eReadWrite}, // Awb image center col end
    {0x1b,0x80,"",eReadWrite}, // b,g mult a constant for detect white pixel
    {0x1a,0x80,"",eReadWrite}, // r,g mult a constant for detect white pixel
    {0x18,0x16,"",eReadWrite}, // wb_fine_gain_step,wb_rough_gain_step
    {0x19,0x26,"",eReadWrite}, // wb_dif_fine_th, wb_dif_rough_th
    {0x1d,0x04,"",eReadWrite}, // skin detect u bot
    {0x1f,0x06,"",eReadWrite}, // skin detect v bot

// d65 10,
    {0x66,0x36,"",eReadWrite},
    {0x67,0x5c,"",eReadWrite},
    {0x68,0xbb,"",eReadWrite},
    {0x69,0xdf,"",eReadWrite},
    {0x6a,0xa5,"",eReadWrite},

// indoor,
    {0x7c,0x26,"",eReadWrite},
    {0x7d,0x4A,"",eReadWrite},
    {0x7e,0xe0,"",eReadWrite},
    {0x7f,0x05,"",eReadWrite},
    {0x80,0xa6,"",eReadWrite},

// cwf   12,
    {0x70,0x21,"",eReadWrite},
    {0x71,0x41,"",eReadWrite},
    {0x72,0x05,"",eReadWrite},
    {0x73,0x25,"",eReadWrite},
    {0x74,0xaa,"",eReadWrite},

// tl84,
    {0x6b,0x00,"",eReadWrite},
    {0x6c,0x20,"",eReadWrite},
    {0x6d,0x0e,"",eReadWrite},
    {0x6e,0x2a,"",eReadWrite},
    {0x6f,0xaa,"",eReadWrite},

    {0x61,0xdb,"",eReadWrite},
    {0x62,0xfe,"",eReadWrite},
    {0x63,0x37,"",eReadWrite},
    {0x64,0x56,"",eReadWrite},
    {0x65,0x5a,"",eReadWrite},

// f,
    {0x75,0x00,"",eReadWrite},
    {0x76,0x09,"",eReadWrite},
    {0x77,0x02,"",eReadWrite},
    {0x0e,0x16,"",eReadWrite},
    {0x3b,0x09,"",eReadWrite},

    {0xfd,0x02,"",eReadWrite}, // awb outdoor mode
    {0x02,0x00,"",eReadWrite}, // outdoor exp 5msb
    {0x03,0x10,"",eReadWrite}, // outdoor exp 8lsb
    {0x04,0xf0,"",eReadWrite}, // outdoor rpc
    {0xf5,0xb3,"",eReadWrite}, // outdoor rgain top
    {0xf6,0x80,"",eReadWrite}, // outdoor rgain bot
    {0xf7,0xe0,"",eReadWrite}, // outdoor bgain top
    {0xf8,0x89,"",eReadWrite}, // outdoor bgain bot

// skin detect
    {0xfd,0x02,"",eReadWrite},
    {0x08,0x00,"",eReadWrite},
    {0x09,0x04,"",eReadWrite},

    {0xfd,0x02,"",eReadWrite},
    {0xdd,0x0f,"",eReadWrite}, // raw smooth en
    {0xde,0x0f,"",eReadWrite}, // sharpen en

    {0xfd,0x02,"",eReadWrite}, //  sharp
    {0x57,0x30,"",eReadWrite}, // raw_sharp_y_base
    {0x58,0x10,"",eReadWrite}, // raw_sharp_y_min
    {0x59,0xe0,"",eReadWrite}, // raw_sharp_y_max
    {0x5a,0x00,"",eReadWrite}, // raw_sharp_rangek_neg
    {0x5b,0x12,"",eReadWrite}, // raw_sharp_rangek_pos,"

    {0xcb,0x08,"",eReadWrite}, // raw_sharp_range_base_outdoor
    {0xcc,0x0b,"",eReadWrite}, // raw_sharp_range_base_nr
    {0xcd,0x10,"",eReadWrite}, // raw_sharp_range_base_dummy
    {0xce,0x1a,"",eReadWrite}, // raw_sharp_range_base_low

    {0xfd,0x03,"",eReadWrite},
    {0x87,0x04,"",eReadWrite}, // raw_sharp_range_ofst1	4x
    {0x88,0x08,"",eReadWrite}, // raw_sharp_range_ofst2	8x
    {0x89,0x10,"",eReadWrite}, // raw_sharp_range_ofst3	16x

    {0xfd,0x02,"",eReadWrite},
    {0xe8,0x58,"",eReadWrite}, // sharpness gain for increasing pixel’s Y, in outdoor
    {0xec,0x68,"",eReadWrite}, // sharpness gain for decreasing pixel’s Y, in outdoor
    {0xe9,0x60,"",eReadWrite}, // sharpness gain for increasing pixel’s Y, in normal
    {0xed,0x68,"",eReadWrite}, // sharpness gain for decreasing pixel’s Y, in normal
    {0xea,0x58,"",eReadWrite}, // sharpness gain for increasing pixel’s Y,in dummy
    {0xee,0x60,"",eReadWrite}, // sharpness gain for decreasing pixel’s Y, in dummy
    {0xeb,0x48,"",eReadWrite}, // sharpness gain for increasing pixel’s Y,in lowlight
    {0xef,0x40,"",eReadWrite}, // sharpness gain for decreasing pixel’s Y, in low light

    {0xfd,0x02,"",eReadWrite}, // skin sharpen
    {0xdc,0x04,"",eReadWrite}, // skin_sharp_sel肤色降锐化
    {0x05,0x6f,"",eReadWrite}, // skin_num_th2排除肤色降锐化对分辨率卡引起的干扰

// 平滑自适应
    {0xfd,0x02,"",eReadWrite},
    {0xf4,0x30,"",eReadWrite}, // raw_ymin
    {0xfd,0x03,"",eReadWrite},
    {0x97,0x98,"",eReadWrite}, // raw_ymax_outdoor,
    {0x98,0x88,"",eReadWrite}, // raw_ymax_normal
    {0x99,0x88,"",eReadWrite}, // raw_ymax_dummy
    {0x9a,0x80,"",eReadWrite}, // raw_ymax_low
    {0xfd,0x02,"",eReadWrite},
    {0xe4,0xff,"",eReadWrite}, // raw_yk_fac_outdoor
    {0xe5,0xff,"",eReadWrite}, // raw_yk_fac_normal
    {0xe6,0xff,"",eReadWrite}, // raw_yk_fac_dummy
    {0xe7,0xff,"",eReadWrite}, // raw_yk_fac_low

    {0xfd,0x03,"",eReadWrite},
    {0x72,0x18,"",eReadWrite}, // raw_lsc_fac_outdoor
    {0x73,0x28,"",eReadWrite}, // raw_lsc_fac_normal
    {0x74,0x28,"",eReadWrite}, // raw_lsc_fac_dummy
    {0x75,0x30,"",eReadWrite}, // raw_lsc_fac_low

// 四个通道内阈值
    {0xfd,0x02,"",eReadWrite},
    {0x78,0x20,"",eReadWrite},
    {0x79,0x20,"",eReadWrite},
    {0x7a,0x14,"",eReadWrite},
    {0x7b,0x08,"",eReadWrite},

    {0x81,0x02,"",eReadWrite}, // raw_grgb_thr_outdoor
    {0x82,0x20,"",eReadWrite},
    {0x83,0x20,"",eReadWrite},
    {0x84,0x08,"",eReadWrite},

    {0xfd,0x03,"",eReadWrite},
    {0x7e,0x06,"",eReadWrite}, // raw_noise_base_outdoor
    {0x7f,0x0d,"",eReadWrite}, // raw_noise_base_normal
    {0x80,0x10,"",eReadWrite}, // raw_noise_base_dummy
    {0x81,0x16,"",eReadWrite}, // raw_noise_base_low
    {0x7c,0xff,"",eReadWrite}, // raw_noise_base_dark
    {0x82,0x54,"",eReadWrite}, // raw_dns_fac_outdoor,raw_dns_fac_normal
    {0x83,0x43,"",eReadWrite}, // raw_dns_fac_dummy,raw_dns_fac_low}
    {0x84,0x00,"",eReadWrite}, // raw_noise_ofst1 	4x
    {0x85,0x20,"",eReadWrite}, // raw_noise_ofst2	8x
    {0x86,0x40,"",eReadWrite}, // raw_noise_ofst3	16x

// 去紫边功能
    {0xfd,0x03,"",eReadWrite},
    {0x66,0x18,"",eReadWrite}, // pf_bg_thr_normal b-g>thr
    {0x67,0x28,"",eReadWrite}, // pf_rg_thr_normal r-g<thr
    {0x68,0x20,"",eReadWrite}, // pf_delta_thr_normal |val|>thr
    {0x69,0x88,"",eReadWrite}, // pf_k_fac val/16
    {0x9b,0x18,"",eReadWrite}, // pf_bg_thr_outdoor
    {0x9c,0x28,"",eReadWrite}, // pf_rg_thr_outdoor
    {0x9d,0x20,"",eReadWrite}, // pf_delta_thr_outdoor

// Gamma
    {0xfd,0x01,"",eReadWrite},
    {0x8b,0x00,"",eReadWrite},
    {0x8c,0x0f,"",eReadWrite},
    {0x8d,0x21,"",eReadWrite},
    {0x8e,0x2c,"",eReadWrite},
    {0x8f,0x37,"",eReadWrite},
    {0x90,0x46,"",eReadWrite},
    {0x91,0x53,"",eReadWrite},
    {0x92,0x5e,"",eReadWrite},
    {0x93,0x6a,"",eReadWrite},
    {0x94,0x7d,"",eReadWrite},
    {0x95,0x8d,"",eReadWrite},
    {0x96,0x9e,"",eReadWrite},
    {0x97,0xac,"",eReadWrite},
    {0x98,0xba,"",eReadWrite},
    {0x99,0xc6,"",eReadWrite},
    {0x9a,0xd1,"",eReadWrite},
    {0x9b,0xda,"",eReadWrite},
    {0x9c,0xe4,"",eReadWrite},
    {0x9d,0xeb,"",eReadWrite},
    {0x9e,0xf2,"",eReadWrite},
    {0x9f,0xf9,"",eReadWrite},
    {0xa0,0xff,"",eReadWrite},

// CCM
    {0xfd,0x02,"",eReadWrite},
    {0x15,0xa9,"",eReadWrite},
    {0x16,0x84,"",eReadWrite},

// !F
    {0xa0,0x97,"",eReadWrite},
    {0xa1,0xea,"",eReadWrite},
    {0xa2,0xff,"",eReadWrite},
    {0xa3,0x0e,"",eReadWrite},
    {0xa4,0x77,"",eReadWrite},
    {0xa5,0xfa,"",eReadWrite},
    {0xa6,0x08,"",eReadWrite},
    {0xa7,0xcb,"",eReadWrite},
    {0xa8,0xad,"",eReadWrite},
    {0xa9,0x3c,"",eReadWrite},
    {0xaa,0x30,"",eReadWrite},
    {0xab,0x0c,"",eReadWrite},

// F
    {0xac,0x7f,"",eReadWrite},
    {0xad,0x08,"",eReadWrite},
    {0xae,0xf8,"",eReadWrite},
    {0xaf,0xff,"",eReadWrite},
    {0xb0,0x6e,"",eReadWrite},
    {0xb1,0x13,"",eReadWrite},
    {0xb2,0xd2,"",eReadWrite},
    {0xb3,0x6e,"",eReadWrite},
    {0xb4,0x40,"",eReadWrite},
    {0xb5,0x30,"",eReadWrite},
    {0xb6,0x03,"",eReadWrite},
    {0xb7,0x1f,"",eReadWrite},

    {0xfd,0x01,"",eReadWrite}, // auto_sat
    {0xd2,0x2d,"",eReadWrite}, // autosat_en[0]
    {0xd1,0x38,"",eReadWrite}, // lum thr in green enhance
    {0xdd,0x3f,"",eReadWrite},
    {0xde,0x37,"",eReadWrite},

// auto sat
    {0xfd,0x02,"",eReadWrite},
    {0xc1,0x40,"",eReadWrite},
    {0xc2,0x40,"",eReadWrite},
    {0xc3,0x40,"",eReadWrite},
    {0xc4,0x40,"",eReadWrite},
    {0xc5,0x80,"",eReadWrite},
    {0xc6,0x60,"",eReadWrite},
    {0xc7,0x00,"",eReadWrite},
    {0xc8,0x00,"",eReadWrite},

// sat u
    {0xfd,0x01,"",eReadWrite},
    {0xd3,0xa0,"",eReadWrite},
    {0xd4,0xa0,"",eReadWrite},
    {0xd5,0xa0,"",eReadWrite},
    {0xd6,0xa0,"",eReadWrite},

// sat v
    {0xd7,0xa0,"",eReadWrite},
    {0xd8,0xa0,"",eReadWrite},
    {0xd9,0xa0,"",eReadWrite},
    {0xda,0xa0,"",eReadWrite},

    {0xfd,0x03,"",eReadWrite},
    {0x76,0x0a,"",eReadWrite},
    {0x7a,0x40,"",eReadWrite},
    {0x7b,0x40,"",eReadWrite},

// auto_sat
    {0xfd,0x01,"",eReadWrite},
    {0xc2,0xaa,"",eReadWrite}, // u_v_th_outdoor白色物体表面有彩色噪声降低此值
    {0xc3,0xaa,"",eReadWrite}, // u_v_th_nr
    {0xc4,0x66,"",eReadWrite}, // u_v_th_dummy
    {0xc5,0x66,"",eReadWrite}, // u_v_th_low

// low_lum_offset
    {0xfd,0x01,"",eReadWrite},
    {0xcd,0x08,"",eReadWrite},
    {0xce,0x18,"",eReadWrite},
// gw
    {0xfd,0x02,"",eReadWrite},
    {0x32,0x60,"",eReadWrite},
    {0x35,0x60,"",eReadWrite}, // uv_fix_dat
    {0x37,0x13,"",eReadWrite},

// heq
    {0xfd,0x01,"",eReadWrite},
    {0xdb,0x00,"",eReadWrite}, // buf_heq_offset
    {0x10,0x88,"",eReadWrite}, // ku_outdoor
    {0x11,0x88,"",eReadWrite}, // ku_nr
    {0x12,0x90,"",eReadWrite}, // ku_dummy
    {0x13,0x90,"",eReadWrite}, // ku_low
    {0x14,0x9a,"",eReadWrite}, // kl_outdoor
    {0x15,0x9a,"",eReadWrite}, // kl_nr
    {0x16,0x8b,"",eReadWrite}, // kl_dummy
    {0x17,0x88,"",eReadWrite}, // kl_low

    {0xfd,0x03,"",eReadWrite},
    {0x00,0x80,"",eReadWrite}, // ctf_heq_mean
    {0x03,0x68,"",eReadWrite}, // ctf_range_thr   可以排除灰板场景的阈值
    {0x06,0xd8,"",eReadWrite}, // ctf_reg_max
    {0x07,0x28,"",eReadWrite}, // ctf_reg_min
    {0x0a,0xfd,"",eReadWrite}, // ctf_lum_ofst
    {0x01,0x16,"",eReadWrite}, // ctf_posk_fac_outdoor
    {0x02,0x16,"",eReadWrite}, // ctf_posk_fac_nr
    {0x04,0x16,"",eReadWrite}, // ctf_posk_fac_dummy
    {0x05,0x16,"",eReadWrite}, // ctf_posk_fac_low
    {0x0b,0x40,"",eReadWrite}, // ctf_negk_fac_outdoor
    {0x0c,0x40,"",eReadWrite}, // ctf_negk_fac_nr
    {0x0d,0x40,"",eReadWrite}, // ctf_negk_fac_dummy
    {0x0e,0x40,"",eReadWrite}, // ctf_negk_fac_low
    {0x08,0x0c,"",eReadWrite},
    {0x09,0x0c,"",eReadWrite},

    {0xfd,0x02,"",eReadWrite}, // cnr
    {0x8e,0x0a,"",eReadWrite}, // cnr_grad_thr_dummy
    {0x90,0x40,"",eReadWrite}, // 20, cnr_thr_outdoor
    {0x91,0x40,"",eReadWrite}, // 20, cnr_thr_nr
    {0x92,0x60,"",eReadWrite}, // 60, cnr_thr_dummy
    {0x93,0x80,"",eReadWrite}, // 80, cnr_thr_low
    {0x9e,0x44,"",eReadWrite},
    {0x9f,0x44,"",eReadWrite},

    {0xfd,0x02,"",eReadWrite}, // auto
    {0x85,0x00,"",eReadWrite}, // enable 50Hz/60Hz function[4]  [3:0] interval_line
    {0xfd,0x01,"",eReadWrite},
    {0x00,0x00,"",eReadWrite}, // fix mode
    {0xfb,0x25,"",eReadWrite},
    {0x32,0x15,"",eReadWrite}, // ae en
    {0x33,0xef,"",eReadWrite}, // lsc\bpc en
    {0x34,0xef,"",eReadWrite}, // ynr[4]\cnr[0]\gamma[2]\colo[1]
    {0x35,0x40,"",eReadWrite}, // YUYV
    {0xfd,0x00,"",eReadWrite},
    {0x3f,0x00,"",eReadWrite}, // mirror/flip
    {0xfd,0x01,"",eReadWrite},
    {0x50,0x00,"",eReadWrite}, // heq_auto_mode 读状态
    {0x66,0x00,"",eReadWrite}, // effect
    {0xfd,0x02,"",eReadWrite},
    {0xd6,0x0f,"",eReadWrite},

    {0xfd,0x00,"",eReadWrite},
    {0x1b,0x30,"",eReadWrite},
    {0xfd,0x01,"",eReadWrite},
    {0x36,0x00,"",eReadWrite},
    {0x0000 ,0x00,"eTableEnd",eTableEnd}

};

const IsiRegDescription_t SP2519_g_svga[] =
{
    {0xfd,0x00,"",eReadWrite},
    {0x47,0x00,"",eReadWrite},
    {0x48,0x00,"",eReadWrite},
    {0x49,0x04,"",eReadWrite},
    {0x4a,0xb0,"",eReadWrite},
    {0x4b,0x00,"",eReadWrite},
    {0x4c,0x00,"",eReadWrite},
    {0x4d,0x06,"",eReadWrite},
    {0x4e,0x40,"",eReadWrite},
    {0xfd,0x01,"",eReadWrite},
    {0x06,0x00,"",eReadWrite},
    {0x07,0x40,"",eReadWrite},
    {0x08,0x00,"",eReadWrite},
    {0x09,0x40,"",eReadWrite},
    {0x0a,0x02,"",eReadWrite},	//600
    {0x0b,0x58,"",eReadWrite},
    {0x0c,0x03,"",eReadWrite},	//800
    {0x0d,0x20,"",eReadWrite},
    {0x0e,0x01,"",eReadWrite},
    {0xfd,0x00,"",eReadWrite},	//software wake
    {0x0000 ,0x00,"eTableEnd",eTableEnd}
};

const IsiRegDescription_t SP2519_g_1600x1200[] =
{
    {0xfd,0x00,"",eReadWrite},
    {0x47,0x00,"",eReadWrite},
    {0x48,0x00,"",eReadWrite},
    {0x49,0x04,"",eReadWrite},
    {0x4a,0xb0,"",eReadWrite},

    {0x4b,0x00,"",eReadWrite},
    {0x4c,0x00,"",eReadWrite},
    {0x4d,0x06,"",eReadWrite},
    {0x4e,0x40,"",eReadWrite},

    {0xfd,0x01,"",eReadWrite},
    {0x0e,0x00,"",eReadWrite},
    {0xfd,0x00,"",eReadWrite},
    {0x0000 ,0x00,"eTableEnd",eTableEnd}

};
