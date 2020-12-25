/*
 * rt5631.c  --  RT5631 ALSA Soc Audio driver
 *
 * Copyright 2011 Realtek Microelectronics
 *
 * Author: flove <flove@realtek.com>
 *
 * Based on WM8753.c
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/pm.h>
#include <linux/i2c.h>
#include <linux/regmap.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>
#include <sound/initval.h>
#include <sound/tlv.h>

#include "rt5631.h"

#if 0
#define DBG(x...)	printk(x)
#else
#define DBG(x...)
#endif

struct rt5631_priv {
	struct regmap *regmap;
	int codec_version;
	int master;
	int sysclk;
	int rx_rate;
	int bclk_rate;
	int dmic_used_flag;
	int eq_mode;
	int phone_det_level;
	int pll_used_flag;
	struct clk *mclk;
};



static struct snd_soc_codec *rt5631_codec = NULL;
struct delayed_work rt5631_delay_cap; //bard 7-16
EXPORT_SYMBOL(rt5631_delay_cap); //bard 7-16
static int timesofbclk = 32;
bool isPlaybackon = false, isCaptureon = false;

module_param(timesofbclk, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(timeofbclk, "relationship between bclk and fs");

static const DECLARE_TLV_DB_SCALE(dac_vol_tlv, -9435, 37, 0);
static inline int rt5631_write(struct snd_soc_codec *codec,
			unsigned int reg, unsigned int val)
{
	return snd_soc_write(codec, reg, val);
}

static inline unsigned int rt5631_read(struct snd_soc_codec *codec,
				unsigned int reg)
{
	return snd_soc_read(codec, reg);
}

static int rt5631_write_mask(struct snd_soc_codec *codec,
	unsigned int reg, unsigned int value, unsigned int mask)
{
	unsigned int reg_val;
	int ret = 0;

	if (!mask)
		return 0;

	if (mask != 0xffff) {
		reg_val = rt5631_read(codec, reg);
		reg_val &= ~mask;
		reg_val |= (value & mask);
		ret = rt5631_write(codec, reg, reg_val);
	} else {
		ret = rt5631_write(codec, reg, value);
	}

	return ret;
}
/**
 * rt5631_write_index - write index register of 2nd layer
 */
static void rt5631_write_index(struct snd_soc_codec *codec,
		unsigned int reg, unsigned int value)
{
	rt5631_write(codec, RT5631_INDEX_ADD, reg);
	rt5631_write(codec, RT5631_INDEX_DATA, value);
	return;
}

/**
 * rt5631_read_index - read index register of 2nd layer
 */
static unsigned int rt5631_read_index(struct snd_soc_codec *codec,
				unsigned int reg)
{
	unsigned int value;
	int ret = 0;

	ret = rt5631_write(codec, RT5631_INDEX_ADD, reg);
	if (ret < 0)
	{
		return ret;
	}

	value = rt5631_read(codec, RT5631_INDEX_DATA);

	return value;
}

static void rt5631_write_index_mask(struct snd_soc_codec *codec,
	unsigned int reg, unsigned int value, unsigned int mask)
{
	unsigned int reg_val;

	if (!mask)
		return;

	if (mask != 0xffff) {
		reg_val = rt5631_read_index(codec, reg);
		reg_val &= ~mask;
		reg_val |= (value & mask);
		rt5631_write_index(codec, reg, reg_val);
	} else {
		rt5631_write_index(codec, reg, value);
	}

	return;
}

static inline int rt5631_reset(struct snd_soc_codec *codec)
{
	return snd_soc_write(codec, RT5631_RESET, 0);
}
#ifndef DEF_VOL
#define DEF_VOL					0xd4//0xd4 -30dB 0xc0 0dB
#endif
#ifndef DEF_VOL_SPK
#define DEF_VOL_SPK				0xc4
#endif
static const struct reg_default rt5631_reg[] = {
	{ RT5631_SPK_OUT_VOL, (DEF_VOL_SPK<<8) | DEF_VOL_SPK },
	{ RT5631_HP_OUT_VOL, (DEF_VOL<<8) | DEF_VOL },
	{ RT5631_MONO_AXO_1_2_VOL, 0xE0c0 },
/*//	{ RT5631_AUX_IN_VOL, 0x0808 },
	{ RT5631_ADC_REC_MIXER, 0xf0f0 },
	{ RT5631_VDAC_DIG_VOL, 0x0010 },
	{ RT5631_OUTMIXER_L_CTRL, 0xffc0 },
	{ RT5631_OUTMIXER_R_CTRL, 0xffc0 },
	{ RT5631_AXO1MIXER_CTRL, 0x88c0 },
	{ RT5631_AXO2MIXER_CTRL, 0x88c0 },
	{ RT5631_DIG_MIC_CTRL, 0x3000 },
	{ RT5631_MONO_INPUT_VOL, 0x8808 },
	{ RT5631_SPK_MIXER_CTRL, 0xf8f8 },
	{ RT5631_SPK_MONO_OUT_CTRL, 0xfc00 },
	{ RT5631_SPK_MONO_HP_OUT_CTRL, 0x4440 },
	{ RT5631_SDP_CTRL, 0x8000 },
	{ RT5631_MONO_SDP_CTRL, 0x8000 },
	{ RT5631_STEREO_AD_DA_CLK_CTRL, 0x2010 },
	{ RT5631_GEN_PUR_CTRL_REG, 0x0e00 },
	{ RT5631_INT_ST_IRQ_CTRL_2, 0x071a },
	{ RT5631_MISC_CTRL, 0x2040 },
	{ RT5631_DEPOP_FUN_CTRL_2, 0x8000 },
	{ RT5631_SOFT_VOL_CTRL, 0x07e0 },
	{ RT5631_ALC_CTRL_1, 0x0206 },
	{ RT5631_ALC_CTRL_3, 0x2000 },
	{ RT5631_PSEUDO_SPATL_CTRL, 0x0553 },*/
	{RT5631_STEREO_DAC_VOL_2	, 0x0303},
	{RT5631_ADC_REC_MIXER		, 0xb0f0},//Record Mixer source from Mic1 by default
	{RT5631_ADC_CTRL_1		, 0x0004},//STEREO ADC CONTROL 1
	{RT5631_MIC_CTRL_2		, 0x4400},//0x8800},//0x6600}, //Mic1/Mic2 boost 40DB by default
	{RT5631_PWR_MANAG_ADD1		, 0x93e0},
	{RT5631_SDP_CTRL        , 0x8002},
	//increase hpo charge pump VEE
	{RT5631_INDEX_ADD			, 0x45},
	{RT5631_INDEX_DATA			, 0x6530},

	{RT5631_OUTMIXER_L_CTRL		, 0xdfC0},//DAC_L-->OutMixer_L by default
	{RT5631_OUTMIXER_R_CTRL		, 0xdfC0},//DAC_R-->OutMixer_R by default
	{RT5631_AXO1MIXER_CTRL		, 0x8840},//OutMixer_L-->AXO1Mixer by default
	{RT5631_AXO2MIXER_CTRL		, 0x8880},//OutMixer_R-->AXO2Mixer by default
	{RT5631_SPK_MIXER_CTRL		, 0xd8d8},//DAC-->SpeakerMixer
	{RT5631_SPK_MONO_OUT_CTRL	, 0x0c00},//Speaker volume-->SPOMixer(L-->L,R-->R)	
	{RT5631_GEN_PUR_CTRL_REG	, 0x4e00},//Speaker AMP ratio gain is 1.27x
#if defined(CONFIG_ADJUST_VOL_BY_CODEC)
	{RT5631_SPK_MONO_HP_OUT_CTRL	, 0x0000},//HP from outputmixer,speaker out from SpeakerOut Mixer	
#else
	{RT5631_SPK_MONO_HP_OUT_CTRL	, 0x000c},//HP from DAC,speaker out from SpeakerOut Mixer
#endif
	{RT5631_DEPOP_FUN_CTRL_2	, 0x8000},//HP depop by register control	
	{RT5631_INT_ST_IRQ_CTRL_2	, 0x0f18},//enable HP zero cross	
	{RT5631_MIC_CTRL_1		, 0x8000},//set mic 1 to differnetial mode
	{RT5631_GPIO_CTRL		, 0x0000},//set GPIO to input pin	
};
#define RT5631_INIT_REG_LEN ARRAY_SIZE(rt5631_reg)

/*
 * EQ parameter
 */
enum {
	NORMAL,
	CLUB,
	DANCE,
	LIVE,
	POP,
	ROCK,
	OPPO,
	TREBLE,
	BASS,
	HFREQ,	
	SPK_FR	
};

struct hw_eq_preset {
	u16 type;
	u16 value[22];
	u16 ctrl;
};

/*
 * EQ param reg : 0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7,
 *		0x8, 0x9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf
 * EQ control reg : 0x6e
 */
struct hw_eq_preset hweq_preset[] = {
	{NORMAL	, {0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0000}, 0x0000},
	{CLUB	, {0x1C10, 0x0000, 0xC1CC, 0x1E5D, 0x0699, 0xCD48,
		0x188D, 0x0699, 0xC3B6, 0x1CD0, 0x0699, 0x0436,
		0x0000, 0x0000, 0x0000, 0x0000}, 0x000E},
	{DANCE	, {0x1F2C, 0x095B, 0xC071, 0x1F95, 0x0616, 0xC96E,
		0x1B11, 0xFC91, 0xDCF2, 0x1194, 0xFAF2, 0x0436,
		0x0000, 0x0000, 0x0000, 0x0000}, 0x000F},
	{LIVE	, {0x1EB5, 0xFCB6, 0xC24A, 0x1DF8, 0x0E7C, 0xC883,
		0x1C10, 0x0699, 0xDA41, 0x1561, 0x0295, 0x0436,
		0x0000, 0x0000, 0x0000, 0x0000}, 0x000F},
	{POP	, {0x1EB5, 0xFCB6, 0xC1D4, 0x1E5D, 0x0E23, 0xD92E,
		0x16E6, 0xFCB6, 0x0000, 0x0969, 0xF988, 0x0436,
		0x0000, 0x0000, 0x0000, 0x0000}, 0x000F},
	{ROCK	, {0x1EB5, 0xFCB6, 0xC071, 0x1F95, 0x0424, 0xC30A,
		0x1D27, 0xF900, 0x0C5D, 0x0FC7, 0x0E23, 0x0436,
		0x0000, 0x0000, 0x0000, 0x0000}, 0x000F},
	{OPPO	, {0x0000, 0x0000, 0xCA4A, 0x17F8, 0x0FEC, 0xCA4A,
		0x17F8, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0000}, 0x000F},
	{TREBLE	, {0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x188D,
		0x1699, 0x0000, 0x0000, 0x0000}, 0x0010},
	{BASS	, {0x1A43, 0x0C00, 0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0000}, 0x0001},
	//	{HFREQ, {0x1BBC,0x0000,0xC9A4,0x1BBC,0x0000,0x2997,0x142D,0xFCB6,0xEF01,0x1BBC,0x0000,0xE835,0x0FEC,0xC66E,0x1A29,0x1CEE},0x0014},//orig
	//{HFREQ, {0x1BBC,0x0000,0xC9A4,0x1BBC,0x0000,0x2997,0x142D,0xFCB6,0x1E97,0x08AC,0xFCB6,0xEEA6,0x095B,0xC66E,0x1A29,0x1CEE},0x0018},//roy 20120904 
	{HFREQ, {0x1FBC,0x1D18,0x11C1,0x0B2B,0xFF1B,0x1F8D,0x09F3,0xFB54,0xEF01,0x1BBC,0x0000,0xE835,0x2298,0xC66E,0x1A29,0x1CEE},0x0014},//roy 20120914 
	{SPK_FR,{0x1DE4,0xF405,0xC306,0x1D60,0x01F3,0x07CA,0x12AF,0xF805,0xE904,0x1C10,0x0000,0x1C8B,0x0000,0xc5e1,0x1afb,0x1d46},0x0003},
};

static int rt5631_reg_init(struct snd_soc_codec *codec)
{
	int i;

	for (i = 0; i < RT5631_INIT_REG_LEN; i++)
		rt5631_write(codec, rt5631_reg[i].reg, rt5631_reg[i].def);

	return 0;
}
//bard 7-16 s
void rt5631_adc_on(struct work_struct *work)
{
	int val;

	val = snd_soc_read(rt5631_codec,RT5631_ADC_REC_MIXER);
	snd_soc_write(rt5631_codec,RT5631_ADC_REC_MIXER,0xf0f0);

	snd_soc_update_bits(rt5631_codec, RT5631_PWR_MANAG_ADD1,
		RT5631_PWR_ADC_L_CLK | RT5631_PWR_ADC_R_CLK, 0);
	snd_soc_update_bits(rt5631_codec, RT5631_PWR_MANAG_ADD1,
		RT5631_PWR_ADC_L_CLK | RT5631_PWR_ADC_R_CLK,
		RT5631_PWR_ADC_L_CLK | RT5631_PWR_ADC_R_CLK);
	snd_soc_write(rt5631_codec,RT5631_ADC_REC_MIXER,val);
	snd_soc_update_bits(rt5631_codec, RT5631_ADC_CTRL_1,
				RT5631_L_MUTE|RT5631_R_MUTE,0x0);

}
static bool rt5631_volatile_register(struct device *dev, unsigned int reg)
{
	switch (reg) {
	case RT5631_RESET:
	case RT5631_INT_ST_IRQ_CTRL_2:
	case RT5631_INDEX_ADD:
	case RT5631_INDEX_DATA:
	case RT5631_EQ_CTRL:
		return 1;
	default:
		return 0;
	}
}

static bool rt5631_readable_register(struct device *dev, unsigned int reg)
{
	switch (reg) {
	case RT5631_RESET:
	case RT5631_SPK_OUT_VOL:
	case RT5631_HP_OUT_VOL:
	case RT5631_MONO_AXO_1_2_VOL:
	case RT5631_AUX_IN_VOL:
	case RT5631_STEREO_DAC_VOL_1:
	case RT5631_MIC_CTRL_1:
	case RT5631_STEREO_DAC_VOL_2:
	case RT5631_ADC_CTRL_1:
	case RT5631_ADC_REC_MIXER:
	case RT5631_ADC_CTRL_2:
	case RT5631_VDAC_DIG_VOL:
	case RT5631_OUTMIXER_L_CTRL:
	case RT5631_OUTMIXER_R_CTRL:
	case RT5631_AXO1MIXER_CTRL:
	case RT5631_AXO2MIXER_CTRL:
	case RT5631_MIC_CTRL_2:
	case RT5631_DIG_MIC_CTRL:
	case RT5631_MONO_INPUT_VOL:
	case RT5631_SPK_MIXER_CTRL:
	case RT5631_SPK_MONO_OUT_CTRL:
	case RT5631_SPK_MONO_HP_OUT_CTRL:
	case RT5631_SDP_CTRL:
	case RT5631_MONO_SDP_CTRL:
	case RT5631_STEREO_AD_DA_CLK_CTRL:
	case RT5631_PWR_MANAG_ADD1:
	case RT5631_PWR_MANAG_ADD2:
	case RT5631_PWR_MANAG_ADD3:
	case RT5631_PWR_MANAG_ADD4:
	case RT5631_GEN_PUR_CTRL_REG:
	case RT5631_GLOBAL_CLK_CTRL:
	case RT5631_PLL_CTRL:
	case RT5631_INT_ST_IRQ_CTRL_1:
	case RT5631_INT_ST_IRQ_CTRL_2:
	case RT5631_GPIO_CTRL:
	case RT5631_MISC_CTRL:
	case RT5631_DEPOP_FUN_CTRL_1:
	case RT5631_DEPOP_FUN_CTRL_2:
	case RT5631_JACK_DET_CTRL:
	case RT5631_SOFT_VOL_CTRL:
	case RT5631_ALC_CTRL_1:
	case RT5631_ALC_CTRL_2:
	case RT5631_ALC_CTRL_3:
	case RT5631_PSEUDO_SPATL_CTRL:
	case RT5631_INDEX_ADD:
	case RT5631_INDEX_DATA:
	case RT5631_EQ_CTRL:
	case RT5631_VENDOR_ID:
	case RT5631_VENDOR_ID1:
	case RT5631_VENDOR_ID2:
		return 1;
	default:
		return 0;
	}
}

//bard 7-16 e
static const char *rt5631_spol_source_sel[] = {
	"SPOLMIX", "MONOIN_RX", "VDAC", "DACL"};
static const char *rt5631_spor_source_sel[] = {
	"SPORMIX", "MONOIN_RX", "VDAC", "DACR"};
static const char *rt5631_mono_source_sel[] = {"MONOMIX", "MONOIN_RX", "VDAC"};
static const char *rt5631_input_mode_source_sel[] = {
	"Single-end", "Differential"};
static const char *rt5631_mic_boost[] = {"Bypass", "+20db", "+24db", "+30db",
			"+35db", "+40db", "+44db", "+50db", "+52db"};
static const char *rt5631_hpl_source_sel[] = {"LEFT HPVOL", "LEFT DAC"};
static const char *rt5631_hpr_source_sel[] = {"RIGHT HPVOL", "RIGHT DAC"};
static const char *rt5631_eq_sel[] = {"NORMAL", "CLUB", "DANCE", "LIVE", "POP",
				"ROCK", "OPPO", "TREBLE", "BASS"};
static const struct soc_enum rt5631_enum[] = {
SOC_ENUM_SINGLE(RT5631_SPK_MONO_HP_OUT_CTRL, 14, 4, rt5631_spol_source_sel),
SOC_ENUM_SINGLE(RT5631_SPK_MONO_HP_OUT_CTRL, 10, 4, rt5631_spor_source_sel),
SOC_ENUM_SINGLE(RT5631_SPK_MONO_HP_OUT_CTRL, 6, 3, rt5631_mono_source_sel),
SOC_ENUM_SINGLE(RT5631_MIC_CTRL_1, 15, 2,  rt5631_input_mode_source_sel),
SOC_ENUM_SINGLE(RT5631_MIC_CTRL_1, 7, 2,  rt5631_input_mode_source_sel),
SOC_ENUM_SINGLE(RT5631_MONO_INPUT_VOL, 15, 2, rt5631_input_mode_source_sel),
SOC_ENUM_SINGLE(RT5631_MIC_CTRL_2, 12, 9, rt5631_mic_boost),
SOC_ENUM_SINGLE(RT5631_MIC_CTRL_2, 8, 9, rt5631_mic_boost),
SOC_ENUM_SINGLE(RT5631_SPK_MONO_HP_OUT_CTRL, 3, 2, rt5631_hpl_source_sel),
SOC_ENUM_SINGLE(RT5631_SPK_MONO_HP_OUT_CTRL, 2, 2, rt5631_hpr_source_sel),
SOC_ENUM_SINGLE(0, 4, 9, rt5631_eq_sel),
};
static int rt5631_dmic_get(struct snd_kcontrol *kcontrol,
		struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_codec *codec = snd_soc_kcontrol_codec(kcontrol);
	struct rt5631_priv *rt5631 = snd_soc_codec_get_drvdata(codec);

	ucontrol->value.integer.value[0] = rt5631->dmic_used_flag;

	return 0;
}

static void rt5631_close_dmic(struct snd_soc_codec *codec)
{
	rt5631_write_mask(codec, RT5631_DIG_MIC_CTRL,
		RT5631_DMIC_L_CH_MUTE_SHIFT | RT5631_DMIC_R_CH_MUTE_SHIFT,
		RT5631_DMIC_L_CH_MUTE | RT5631_DMIC_R_CH_MUTE);
	rt5631_write_mask(codec, RT5631_DIG_MIC_CTRL,
		RT5631_DMIC_DIS, RT5631_DMIC_ENA_MASK);
	return;
}

static int rt5631_dmic_put(struct snd_kcontrol *kcontrol,
		struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_codec *codec = snd_soc_kcontrol_codec(kcontrol);
	struct rt5631_priv *rt5631 = snd_soc_codec_get_drvdata(codec);
	if (rt5631->dmic_used_flag == ucontrol->value.integer.value[0])
		return 0;

	if (ucontrol->value.integer.value[0]) {
		rt5631->dmic_used_flag = 1;
	} else {
		rt5631_close_dmic(codec);
		rt5631->dmic_used_flag = 0;
	}

	return 0;
}
static int rt5631_eq_sel_get(struct snd_kcontrol *kcontrol,
		struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol);
	struct rt5631_priv *rt5631 = snd_soc_codec_get_drvdata(codec);

//	ucontrol->value.integer.value[0] = rt5631->eq_mode;

	return 0;
}

static void rt5631_update_eqmode(struct snd_soc_codec *codec, int mode)
{
	int i;

//	DBG("enter rt5631_update_eqmode=========\n");
	if (NORMAL == mode) {
		/* In Normal mode, the EQ parameter is cleared,
		 * and hardware LP, BP1, BP2, BP3, HP1, HP2
		 * block control and EQ block are disabled.
		 */
		for (i = RT5631_EQ_BW_LOP; i <= RT5631_EQ_HPF_GAIN; i++)
			rt5631_write_index(codec, i,
				hweq_preset[mode].value[i]);
		rt5631_write_mask(codec, RT5631_EQ_CTRL, 0x0000, 0x003f);
		rt5631_write_index_mask(codec, RT5631_EQ_PRE_VOL_CTRL
						, 0x0000, 0x8000);
	} else {
		/* Fill and update EQ parameter,
		 * and EQ block are enabled.
		 */
		rt5631_write_index_mask(codec, RT5631_EQ_PRE_VOL_CTRL
						, 0x8000, 0x8000);
		rt5631_write(codec, RT5631_EQ_CTRL,
			hweq_preset[mode].ctrl);
		for (i = RT5631_EQ_BW_LOP; i <= RT5631_EQ_HPF_GAIN; i++)
			rt5631_write_index(codec, i,
				hweq_preset[mode].value[i]);
		rt5631_write_mask(codec, RT5631_EQ_CTRL, 0x4000, 0x4000);
	}

	return;
}

static int rt5631_eq_sel_put(struct snd_kcontrol *kcontrol,
		struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol);
	struct rt5631_priv *rt5631 = snd_soc_codec_get_drvdata(codec);

	if (rt5631->eq_mode == ucontrol->value.integer.value[0])
		return 0;

	rt5631_update_eqmode(codec, ucontrol->value.enumerated.item[0]);
//	rt5631->eq_mode = ucontrol->value.integer.value[0];

	return 0;
}

static const struct snd_kcontrol_new rt5631_snd_controls[] = {
SOC_ENUM("MIC1 Mode Control",  rt5631_enum[3]),
SOC_ENUM("MIC1 Boost", rt5631_enum[6]),
SOC_ENUM("MIC2 Mode Control", rt5631_enum[4]),
SOC_ENUM("MIC2 Boost", rt5631_enum[7]),
SOC_ENUM("MONOIN Mode Control", rt5631_enum[5]),
SOC_DOUBLE_TLV("PCM Playback Volume", RT5631_STEREO_DAC_VOL_2, 8, 0, 255, 1, dac_vol_tlv),
SOC_DOUBLE("PCM Playback Switch", RT5631_STEREO_DAC_VOL_1, 15, 7, 1, 1),
SOC_DOUBLE("MONOIN_RX Capture Volume", RT5631_MONO_INPUT_VOL, 8, 0, 31, 1),
SOC_DOUBLE("AXI Capture Volume", RT5631_AUX_IN_VOL, 8, 0, 31, 1),
SOC_SINGLE("AXO1 Playback Switch", RT5631_MONO_AXO_1_2_VOL, 15, 1, 1),
SOC_SINGLE("AXO2 Playback Switch", RT5631_MONO_AXO_1_2_VOL, 7, 1, 1),
SOC_DOUBLE("OUTVOL Playback Volume", RT5631_MONO_AXO_1_2_VOL, 8, 0, 31, 1),
SOC_DOUBLE("Speaker Playback Switch", RT5631_SPK_OUT_VOL, 15, 7, 1, 1),
SOC_DOUBLE("Speaker Playback Volume", RT5631_SPK_OUT_VOL, 8, 0, 63, 1),
SOC_SINGLE("MONO Playback Switch", RT5631_MONO_AXO_1_2_VOL, 13, 1, 1),
SOC_DOUBLE("HP Playback Switch", RT5631_HP_OUT_VOL, 15, 7, 1, 1),
SOC_DOUBLE("HP Playback Volume", RT5631_HP_OUT_VOL, 8, 0, 63, 1),
SOC_SINGLE_EXT("DMIC Capture Switch", 0, 2, 1, 0,
	rt5631_dmic_get, rt5631_dmic_put),
SOC_ENUM_EXT("EQ Mode", rt5631_enum[10], rt5631_eq_sel_get, rt5631_eq_sel_put),
};

static const struct snd_kcontrol_new rt5631_recmixl_mixer_controls[] = {
SOC_DAPM_SINGLE("OUTMIXL Capture Switch", RT5631_ADC_REC_MIXER, 15, 1, 1),
SOC_DAPM_SINGLE("MIC1_BST1 Capture Switch", RT5631_ADC_REC_MIXER, 14, 1, 1),
SOC_DAPM_SINGLE("AXILVOL Capture Switch", RT5631_ADC_REC_MIXER, 13, 1, 1),
SOC_DAPM_SINGLE("MONOIN_RX Capture Switch", RT5631_ADC_REC_MIXER, 12, 1, 1),
};

static const struct snd_kcontrol_new rt5631_recmixr_mixer_controls[] = {
SOC_DAPM_SINGLE("MONOIN_RX Capture Switch", RT5631_ADC_REC_MIXER, 4, 1, 1),
SOC_DAPM_SINGLE("AXIRVOL Capture Switch", RT5631_ADC_REC_MIXER, 5, 1, 1),
SOC_DAPM_SINGLE("MIC2_BST2 Capture Switch", RT5631_ADC_REC_MIXER, 6, 1, 1),
SOC_DAPM_SINGLE("OUTMIXR Capture Switch", RT5631_ADC_REC_MIXER, 7, 1, 1),
};

static const struct snd_kcontrol_new rt5631_spkmixl_mixer_controls[] = {
SOC_DAPM_SINGLE("RECMIXL Playback Switch", RT5631_SPK_MIXER_CTRL, 15, 1, 1),
SOC_DAPM_SINGLE("MIC1_P Playback Switch", RT5631_SPK_MIXER_CTRL, 14, 1, 1),
SOC_DAPM_SINGLE("DACL Playback Switch", RT5631_SPK_MIXER_CTRL, 13, 1, 1),
SOC_DAPM_SINGLE("OUTMIXL Playback Switch", RT5631_SPK_MIXER_CTRL, 12, 1, 1),
};

static const struct snd_kcontrol_new rt5631_spkmixr_mixer_controls[] = {
SOC_DAPM_SINGLE("OUTMIXR Playback Switch", RT5631_SPK_MIXER_CTRL, 4, 1, 1),
SOC_DAPM_SINGLE("DACR Playback Switch", RT5631_SPK_MIXER_CTRL, 5, 1, 1),
SOC_DAPM_SINGLE("MIC2_P Playback Switch", RT5631_SPK_MIXER_CTRL, 6, 1, 1),
SOC_DAPM_SINGLE("RECMIXR Playback Switch", RT5631_SPK_MIXER_CTRL, 7, 1, 1),
};

static const struct snd_kcontrol_new rt5631_outmixl_mixer_controls[] = {
SOC_DAPM_SINGLE("RECMIXL Playback Switch", RT5631_OUTMIXER_L_CTRL, 15, 1, 1),
SOC_DAPM_SINGLE("RECMIXR Playback Switch", RT5631_OUTMIXER_L_CTRL, 14, 1, 1),
SOC_DAPM_SINGLE("DACL Playback Switch", RT5631_OUTMIXER_L_CTRL, 13, 1, 1),
SOC_DAPM_SINGLE("MIC1_BST1 Playback Switch", RT5631_OUTMIXER_L_CTRL, 12, 1, 1),
SOC_DAPM_SINGLE("MIC2_BST2 Playback Switch", RT5631_OUTMIXER_L_CTRL, 11, 1, 1),
SOC_DAPM_SINGLE("MONOIN_RXP Playback Switch", RT5631_OUTMIXER_L_CTRL, 10, 1, 1),
SOC_DAPM_SINGLE("AXILVOL Playback Switch", RT5631_OUTMIXER_L_CTRL, 9, 1, 1),
SOC_DAPM_SINGLE("AXIRVOL Playback Switch", RT5631_OUTMIXER_L_CTRL, 8, 1, 1),
SOC_DAPM_SINGLE("VDAC Playback Switch", RT5631_OUTMIXER_L_CTRL, 7, 1, 1),
};

static const struct snd_kcontrol_new rt5631_outmixr_mixer_controls[] = {
SOC_DAPM_SINGLE("VDAC Playback Switch", RT5631_OUTMIXER_R_CTRL, 7, 1, 1),
SOC_DAPM_SINGLE("AXIRVOL Playback Switch", RT5631_OUTMIXER_R_CTRL, 8, 1, 1),
SOC_DAPM_SINGLE("AXILVOL Playback Switch", RT5631_OUTMIXER_R_CTRL, 9, 1, 1),
SOC_DAPM_SINGLE("MONOIN_RXN Playback Switch", RT5631_OUTMIXER_R_CTRL, 10, 1, 1),
SOC_DAPM_SINGLE("MIC2_BST2 Playback Switch", RT5631_OUTMIXER_R_CTRL, 11, 1, 1),
SOC_DAPM_SINGLE("MIC1_BST1 Playback Switch", RT5631_OUTMIXER_R_CTRL, 12, 1, 1),
SOC_DAPM_SINGLE("DACR Playback Switch", RT5631_OUTMIXER_R_CTRL, 13, 1, 1),
SOC_DAPM_SINGLE("RECMIXR Playback Switch", RT5631_OUTMIXER_R_CTRL, 14, 1, 1),
SOC_DAPM_SINGLE("RECMIXL Playback Switch", RT5631_OUTMIXER_R_CTRL, 15, 1, 1),
};

static const struct snd_kcontrol_new rt5631_AXO1MIX_mixer_controls[] = {
SOC_DAPM_SINGLE("MIC1_BST1 Playback Switch", RT5631_AXO1MIXER_CTRL, 15 , 1, 1),
SOC_DAPM_SINGLE("MIC2_BST2 Playback Switch", RT5631_AXO1MIXER_CTRL, 11, 1, 1),
SOC_DAPM_SINGLE("OUTVOLL Playback Switch", RT5631_AXO1MIXER_CTRL, 7 , 1 , 1),
SOC_DAPM_SINGLE("OUTVOLR Playback Switch", RT5631_AXO1MIXER_CTRL, 6, 1, 1),
};

static const struct snd_kcontrol_new rt5631_AXO2MIX_mixer_controls[] = {
SOC_DAPM_SINGLE("MIC1_BST1 Playback Switch", RT5631_AXO2MIXER_CTRL, 15, 1, 1),
SOC_DAPM_SINGLE("MIC2_BST2 Playback Switch", RT5631_AXO2MIXER_CTRL, 11, 1, 1),
SOC_DAPM_SINGLE("OUTVOLL Playback Switch", RT5631_AXO2MIXER_CTRL, 7, 1, 1),
SOC_DAPM_SINGLE("OUTVOLR Playback Switch", RT5631_AXO2MIXER_CTRL, 6, 1 , 1),
};

static const struct snd_kcontrol_new rt5631_spolmix_mixer_controls[] = {
SOC_DAPM_SINGLE("SPKVOLL Playback Switch", RT5631_SPK_MONO_OUT_CTRL, 15, 1, 1),
SOC_DAPM_SINGLE("SPKVOLR Playback Switch", RT5631_SPK_MONO_OUT_CTRL, 14, 1, 1),
};

static const struct snd_kcontrol_new rt5631_spormix_mixer_controls[] = {
SOC_DAPM_SINGLE("SPKVOLL Playback Switch", RT5631_SPK_MONO_OUT_CTRL, 13, 1, 1),
SOC_DAPM_SINGLE("SPKVOLR Playback Switch", RT5631_SPK_MONO_OUT_CTRL, 12, 1, 1),
};

static const struct snd_kcontrol_new rt5631_monomix_mixer_controls[] = {
SOC_DAPM_SINGLE("OUTVOLL Playback Switch", RT5631_SPK_MONO_OUT_CTRL, 11, 1, 1),
SOC_DAPM_SINGLE("OUTVOLR Playback Switch", RT5631_SPK_MONO_OUT_CTRL, 10, 1, 1),
};

static const struct snd_kcontrol_new rt5631_spol_mux_control =
SOC_DAPM_ENUM("Route", rt5631_enum[0]);
static const struct snd_kcontrol_new rt5631_spor_mux_control =
SOC_DAPM_ENUM("Route", rt5631_enum[1]);
static const struct snd_kcontrol_new rt5631_mono_mux_control =
SOC_DAPM_ENUM("Route", rt5631_enum[2]);

static const struct snd_kcontrol_new rt5631_hpl_mux_control =
SOC_DAPM_ENUM("Route", rt5631_enum[8]);
static const struct snd_kcontrol_new rt5631_hpr_mux_control =
SOC_DAPM_ENUM("Route", rt5631_enum[9]);

static int spk_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);
	static int spkl_out_enable, spkr_out_enable;

	switch (event) {
	case SND_SOC_DAPM_POST_PMU:

#if (RT5631_ALC_DAC_FUNC_ENA == 1)		
	       rt5631_alc_enable(codec, 1);
#endif		

		if (!spkl_out_enable && !strcmp(w->name, "SPKL Amp")) {
			snd_soc_update_bits(codec, RT5631_PWR_MANAG_ADD4,RT5631_PWR_SPK_L_VOL,
					RT5631_PWR_SPK_L_VOL);
			snd_soc_update_bits(codec, RT5631_PWR_MANAG_ADD1,RT5631_PWR_SPK_L_VOL,
					RT5631_PWR_SPK_L_VOL);
			snd_soc_update_bits(codec, RT5631_SPK_OUT_VOL,RT5631_L_MUTE,
					0);
			spkl_out_enable = 1;
		}
		if (!spkr_out_enable && !strcmp(w->name, "SPKR Amp")) {
			snd_soc_update_bits(codec, RT5631_PWR_MANAG_ADD4,
					RT5631_PWR_SPK_R_VOL ,RT5631_PWR_SPK_R_VOL);
			snd_soc_update_bits(codec, RT5631_PWR_MANAG_ADD1,
					RT5631_PWR_CLASS_D,RT5631_PWR_CLASS_D);
			snd_soc_update_bits(codec, RT5631_SPK_OUT_VOL,
					RT5631_R_MUTE,0);
			spkr_out_enable = 1;
		}
		break;

	case SND_SOC_DAPM_POST_PMD:
		if (spkl_out_enable && !strcmp(w->name, "SPKL Amp")) {
			snd_soc_update_bits(codec, RT5631_SPK_OUT_VOL,
					RT5631_L_MUTE,RT5631_L_MUTE);
			snd_soc_update_bits(codec, RT5631_PWR_MANAG_ADD4,
					RT5631_PWR_SPK_L_VOL,0 );
			spkl_out_enable = 0;
		}
		if (spkr_out_enable && !strcmp(w->name, "SPKR Amp")) {
			snd_soc_update_bits(codec, RT5631_SPK_OUT_VOL,
					RT5631_R_MUTE, RT5631_R_MUTE);
			snd_soc_update_bits(codec, RT5631_PWR_MANAG_ADD4,
					RT5631_PWR_SPK_R_VOL,0);
			spkr_out_enable = 0;
		}
		if (0 == spkl_out_enable && 0 == spkr_out_enable)
			snd_soc_update_bits(codec, RT5631_PWR_MANAG_ADD1,
					RT5631_PWR_CLASS_D,0);

#if (RT5631_ALC_DAC_FUNC_ENA == 1)			
		rt5631_alc_enable(codec, 0);
#endif

		break;

	default:
		return 0;
	}

	return 0;
}
/**
 * depop_seq_mute_stage - step by step depop sequence in mute stage.
 * @enable: mute/unmute
 *
 * When mute/unmute headphone, the depop sequence is done in step by step.
 */
 static void hp_depop_mode2_onebit(struct snd_soc_codec *codec, int enable)
{
	unsigned int soft_vol, hp_zc;

	rt5631_write_mask(codec, RT5631_DEPOP_FUN_CTRL_2, 0, RT5631_EN_ONE_BIT_DEPOP);

	soft_vol = rt5631_read(codec, RT5631_SOFT_VOL_CTRL);
	rt5631_write(codec, RT5631_SOFT_VOL_CTRL, 0);
	hp_zc = rt5631_read(codec, RT5631_INT_ST_IRQ_CTRL_2);
	rt5631_write(codec, RT5631_INT_ST_IRQ_CTRL_2, hp_zc & 0xf7ff);
	if (enable) {
		rt5631_write_index(codec, RT5631_TEST_MODE_CTRL, 0x84c0);
		rt5631_write_index(codec, RT5631_SPK_INTL_CTRL, 0x309f);
		rt5631_write_index(codec, RT5631_CP_INTL_REG2, 0x6530);
		rt5631_write(codec, RT5631_DEPOP_FUN_CTRL_2,
				RT5631_EN_CAP_FREE_DEPOP);
	} else {
		rt5631_write(codec, RT5631_DEPOP_FUN_CTRL_2, 0);
		schedule_timeout_uninterruptible(msecs_to_jiffies(100));
	}

	rt5631_write(codec, RT5631_SOFT_VOL_CTRL, soft_vol);
	rt5631_write(codec, RT5631_INT_ST_IRQ_CTRL_2, hp_zc);

	return;
}

static void hp_mute_unmute_depop_onebit(struct snd_soc_codec *codec, int enable)
{
	unsigned int soft_vol, hp_zc;

	rt5631_write_mask(codec, RT5631_DEPOP_FUN_CTRL_2, 0, RT5631_EN_ONE_BIT_DEPOP);
	soft_vol = rt5631_read(codec, RT5631_SOFT_VOL_CTRL);
	rt5631_write(codec, RT5631_SOFT_VOL_CTRL, 0);
	hp_zc = rt5631_read(codec, RT5631_INT_ST_IRQ_CTRL_2);
	rt5631_write(codec, RT5631_INT_ST_IRQ_CTRL_2, hp_zc & 0xf7ff);
	if (enable) {
		schedule_timeout_uninterruptible(msecs_to_jiffies(10));
		rt5631_write_index(codec, RT5631_SPK_INTL_CTRL, 0x307f);
		rt5631_write_mask(codec, RT5631_HP_OUT_VOL, 0,
				RT5631_L_MUTE | RT5631_R_MUTE);
		schedule_timeout_uninterruptible(msecs_to_jiffies(300));

	} else {
		rt5631_write_mask(codec, RT5631_HP_OUT_VOL,
			RT5631_L_MUTE | RT5631_R_MUTE, RT5631_L_MUTE | RT5631_R_MUTE);
		schedule_timeout_uninterruptible(msecs_to_jiffies(100));
	}
	rt5631_write(codec, RT5631_SOFT_VOL_CTRL, soft_vol);
	rt5631_write(codec, RT5631_INT_ST_IRQ_CTRL_2, hp_zc);

	return;
}

static void hp_depop2(struct snd_soc_codec *codec, int enable)
{
	unsigned int soft_vol, hp_zc;

	rt5631_write_mask(codec, RT5631_DEPOP_FUN_CTRL_2,
		RT5631_EN_ONE_BIT_DEPOP, RT5631_EN_ONE_BIT_DEPOP);
	soft_vol = rt5631_read(codec, RT5631_SOFT_VOL_CTRL);
	rt5631_write(codec, RT5631_SOFT_VOL_CTRL, 0);
	hp_zc = rt5631_read(codec, RT5631_INT_ST_IRQ_CTRL_2);
	rt5631_write(codec, RT5631_INT_ST_IRQ_CTRL_2, hp_zc & 0xf7ff);
	if (enable) {
		rt5631_write_index(codec, RT5631_SPK_INTL_CTRL, 0x303e);
		rt5631_write_mask(codec, RT5631_PWR_MANAG_ADD3,
			RT5631_PWR_CHARGE_PUMP | RT5631_PWR_HP_L_AMP | RT5631_PWR_HP_R_AMP,
			RT5631_PWR_CHARGE_PUMP | RT5631_PWR_HP_L_AMP | RT5631_PWR_HP_R_AMP);
		rt5631_write(codec, RT5631_DEPOP_FUN_CTRL_1,
			RT5631_POW_ON_SOFT_GEN | RT5631_EN_DEPOP2_FOR_HP);
		schedule_timeout_uninterruptible(msecs_to_jiffies(100));
		rt5631_write_mask(codec, RT5631_PWR_MANAG_ADD3,
			RT5631_PWR_HP_DEPOP_DIS, RT5631_PWR_HP_DEPOP_DIS);
	} else {
		rt5631_write_index(codec, RT5631_SPK_INTL_CTRL, 0x303F);
		rt5631_write(codec, RT5631_DEPOP_FUN_CTRL_1,
			RT5631_POW_ON_SOFT_GEN | RT5631_EN_MUTE_UNMUTE_DEPOP |
			RT5631_PD_HPAMP_L_ST_UP | RT5631_PD_HPAMP_R_ST_UP);
		schedule_timeout_uninterruptible(msecs_to_jiffies(75));
		rt5631_write(codec, RT5631_DEPOP_FUN_CTRL_1,
			RT5631_POW_ON_SOFT_GEN | RT5631_PD_HPAMP_L_ST_UP | RT5631_PD_HPAMP_R_ST_UP);
		rt5631_write_mask(codec, RT5631_PWR_MANAG_ADD3, 0,
					RT5631_PWR_HP_DEPOP_DIS);
		rt5631_write(codec, RT5631_DEPOP_FUN_CTRL_1,
			RT5631_POW_ON_SOFT_GEN | RT5631_EN_DEPOP2_FOR_HP |
			RT5631_PD_HPAMP_L_ST_UP | RT5631_PD_HPAMP_R_ST_UP);
		schedule_timeout_uninterruptible(msecs_to_jiffies(80));
		rt5631_write(codec, RT5631_DEPOP_FUN_CTRL_1, RT5631_POW_ON_SOFT_GEN);
		rt5631_write_mask(codec, RT5631_PWR_MANAG_ADD3, 0,
			RT5631_PWR_CHARGE_PUMP | RT5631_PWR_HP_L_AMP | RT5631_PWR_HP_R_AMP);
	}

	rt5631_write(codec, RT5631_SOFT_VOL_CTRL, soft_vol);
	rt5631_write(codec, RT5631_INT_ST_IRQ_CTRL_2, hp_zc);

	return;
}

static void hp_mute_unmute_depop(struct snd_soc_codec *codec, int enable)
{
	unsigned int soft_vol, hp_zc;

	rt5631_write_mask(codec, RT5631_DEPOP_FUN_CTRL_2,
		RT5631_EN_ONE_BIT_DEPOP, RT5631_EN_ONE_BIT_DEPOP);
	soft_vol = rt5631_read(codec, RT5631_SOFT_VOL_CTRL);
	rt5631_write(codec, RT5631_SOFT_VOL_CTRL, 0);
	hp_zc = rt5631_read(codec, RT5631_INT_ST_IRQ_CTRL_2);
	rt5631_write(codec, RT5631_INT_ST_IRQ_CTRL_2, hp_zc & 0xf7ff);
	if (enable) {
		schedule_timeout_uninterruptible(msecs_to_jiffies(10));
		rt5631_write_index(codec, RT5631_SPK_INTL_CTRL, 0x302f);
		rt5631_write(codec, RT5631_DEPOP_FUN_CTRL_1,
			RT5631_POW_ON_SOFT_GEN | RT5631_EN_MUTE_UNMUTE_DEPOP |
			RT5631_EN_HP_R_M_UN_MUTE_DEPOP | RT5631_EN_HP_L_M_UN_MUTE_DEPOP);
		rt5631_write_mask(codec, RT5631_HP_OUT_VOL, 0,
				RT5631_L_MUTE | RT5631_R_MUTE);
		schedule_timeout_uninterruptible(msecs_to_jiffies(160));
	} else {
		rt5631_write_index(codec, RT5631_SPK_INTL_CTRL, 0x302f);
		rt5631_write(codec, RT5631_DEPOP_FUN_CTRL_1,
			RT5631_POW_ON_SOFT_GEN | RT5631_EN_MUTE_UNMUTE_DEPOP |
			RT5631_EN_HP_R_M_UN_MUTE_DEPOP | RT5631_EN_HP_L_M_UN_MUTE_DEPOP);
		rt5631_write_mask(codec, RT5631_HP_OUT_VOL,
			RT5631_L_MUTE | RT5631_R_MUTE, RT5631_L_MUTE | RT5631_R_MUTE);
		schedule_timeout_uninterruptible(msecs_to_jiffies(150));
	}

	rt5631_write(codec, RT5631_SOFT_VOL_CTRL, soft_vol);
	rt5631_write(codec, RT5631_INT_ST_IRQ_CTRL_2, hp_zc);

	return;
}
static int hp_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);
	struct rt5631_priv *rt5631 = snd_soc_codec_get_drvdata(codec);
	static bool hp_en;
	int pu_l, pu_r;

	pu_l = rt5631_read(codec, RT5631_PWR_MANAG_ADD4) & RT5631_PWR_HP_L_OUT_VOL;
	pu_r = rt5631_read(codec, RT5631_PWR_MANAG_ADD4) & RT5631_PWR_HP_R_OUT_VOL;
	switch (event) {
	case SND_SOC_DAPM_PRE_PMD:
		if ((pu_l && pu_r) && hp_en) {
			if (rt5631->codec_version) {
				hp_mute_unmute_depop_onebit(codec, 0);
				hp_depop_mode2_onebit(codec, 0);
			} else {
				hp_mute_unmute_depop(codec, 0);
				hp_depop2(codec, 0);
			}
			hp_en = false;
		}
		break;

	case SND_SOC_DAPM_POST_PMU:
		if ((pu_l && pu_r) && !hp_en) {
			if (rt5631->codec_version) {
				hp_depop_mode2_onebit(codec, 1);
				hp_mute_unmute_depop_onebit(codec, 1);
			} else {
				hp_depop2(codec, 1);
				hp_mute_unmute_depop(codec, 1);
			}
			hp_en = true;
		}
		break;

	default:
		break;
	}

	return 0;
}

static int dac_to_hp_event(struct snd_soc_dapm_widget *w,
		struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);
	struct rt5631_priv *rt5631 = snd_soc_codec_get_drvdata(codec);
	static bool hp_en;

	switch (event) {
	case SND_SOC_DAPM_PRE_PMD:
		if (hp_en) {
			if (rt5631->codec_version) {
				hp_mute_unmute_depop_onebit(codec, 0);
				hp_depop_mode2_onebit(codec, 0);
			} else {
				hp_mute_unmute_depop(codec, 0);
				hp_depop2(codec, 0);
			}
			hp_en = false;
		}
		break;

	case SND_SOC_DAPM_POST_PMU:
		if (!hp_en) {
			if (rt5631->codec_version) {
				hp_depop_mode2_onebit(codec, 1);
				hp_mute_unmute_depop_onebit(codec, 1);
			} else {
				hp_depop2(codec, 1);
				hp_mute_unmute_depop(codec, 1);
			}
			hp_en = true;
		}
		break;

	default:
		break;
	}

	return 0;
}

static int mic_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);
	int val_mic1, val_mic2;

	val_mic1 = rt5631_read(codec, RT5631_PWR_MANAG_ADD2) &
				RT5631_PWR_MIC1_BOOT_GAIN;
	val_mic2 = rt5631_read(codec, RT5631_PWR_MANAG_ADD2) &
				RT5631_PWR_MIC2_BOOT_GAIN;
	switch (event) {
	case SND_SOC_DAPM_POST_PMU:
		/*
		 * If microphone is stereo, need not copy ADC channel
		 * If mic1 is used, copy ADC left to right
		 * If mic2 is used, copy ADC right to left
		 */
		if (val_mic1 && val_mic2)
			rt5631_write_mask(codec, RT5631_INT_ST_IRQ_CTRL_2,
							0x0000, 0xc000);
		else if (val_mic1)
			rt5631_write_mask(codec, RT5631_INT_ST_IRQ_CTRL_2,
							0x4000, 0xc000);
		else if (val_mic2)
			rt5631_write_mask(codec, RT5631_INT_ST_IRQ_CTRL_2,
							0x8000, 0xc000);
		else
			rt5631_write_mask(codec, RT5631_INT_ST_IRQ_CTRL_2,
							0x0000, 0xc000);
		break;

	default:
		break;
	}

	return 0;
}

static int auxo1_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);
	static bool aux1_en;

	switch (event) {
	case SND_SOC_DAPM_PRE_PMD:
		if (aux1_en) {
			rt5631_write_mask(codec, RT5631_MONO_AXO_1_2_VOL,
						RT5631_L_MUTE, RT5631_L_MUTE);
			aux1_en = false;
		}
		break;

	case SND_SOC_DAPM_POST_PMU:
		if (!aux1_en) {
			rt5631_write_mask(codec, RT5631_MONO_AXO_1_2_VOL,
						0, RT5631_L_MUTE);
			aux1_en = true;
		}
		break;

	default:
		break;
	}

	return 0;
}

static int auxo2_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);
	static bool aux2_en;

	switch (event) {
	case SND_SOC_DAPM_PRE_PMD:
		if (aux2_en) {
			snd_soc_update_bits(codec, RT5631_MONO_AXO_1_2_VOL,
						 RT5631_R_MUTE,RT5631_R_MUTE);
			aux2_en = false;
		}
		break;

	case SND_SOC_DAPM_POST_PMU:
		if (!aux2_en) {
			snd_soc_update_bits(codec, RT5631_MONO_AXO_1_2_VOL,
						RT5631_R_MUTE,0);
			aux2_en = true;
		}
		break;

	default:
		break;
	}

	return 0;
}

static int mono_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);
	static bool mono_en;

	switch (event) {
	case SND_SOC_DAPM_PRE_PMD:
		if (mono_en) {
			snd_soc_update_bits(codec, RT5631_MONO_AXO_1_2_VOL,
						RT5631_MUTE_MONO ,RT5631_MUTE_MONO );
			snd_soc_update_bits(codec, RT5631_PWR_MANAG_ADD3,
						RT5631_PWR_MONO_DEPOP_DIS,0);
			mono_en = false;
		}
		break;

	case SND_SOC_DAPM_POST_PMU:
		if (!mono_en) {
			snd_soc_update_bits(codec, RT5631_PWR_MANAG_ADD3,
				RT5631_PWR_MONO_DEPOP_DIS,RT5631_PWR_MONO_DEPOP_DIS);
			snd_soc_update_bits(codec, RT5631_MONO_AXO_1_2_VOL,
						RT5631_MUTE_MONO ,0 );
			mono_en = true;
		}
		break;

	default:
		break;
	}

	return 0;
}

/**
 * config_common_power - control all common power of codec system
 * @pmu: power up or not
 */
static int config_common_power(struct snd_soc_codec *codec, bool pmu)
{
	struct rt5631_priv *rt5631 = snd_soc_codec_get_drvdata(codec);
	unsigned int mux_val;
/*
	if (pmu) {
		snd_soc_update_bits(codec, RT5631_PWR_MANAG_ADD1,
			RT5631_PWR_MAIN_I2S_EN | RT5631_PWR_DAC_REF,
			RT5631_PWR_MAIN_I2S_EN | RT5631_PWR_DAC_REF);
		mux_val = rt5631_read(codec, RT5631_SPK_MONO_HP_OUT_CTRL);
		//if (!(mux_val & HP_L_MUX_SEL_DAC_L))
			snd_soc_update_bits(codec, RT5631_PWR_MANAG_ADD1,
				 RT5631_PWR_DAC_L_TO_MIXER , RT5631_PWR_DAC_L_TO_MIXER);
		//if (!(mux_val & HP_R_MUX_SEL_DAC_R))
			snd_soc_update_bits(codec, RT5631_PWR_MANAG_ADD1,
				RT5631_PWR_DAC_R_TO_MIXER,RT5631_PWR_DAC_R_TO_MIXER);
		if (rt5631->pll_used_flag)
			snd_soc_update_bits(codec, RT5631_PWR_MANAG_ADD2,
						RT5631_PWR_PLL1,RT5631_PWR_PLL1);
	} else if (isPlaybackon == false && isCaptureon == false){
		snd_soc_update_bits(codec, RT5631_PWR_MANAG_ADD1,
			RT5631_PWR_MAIN_I2S_EN | RT5631_PWR_DAC_REF |
			RT5631_PWR_DAC_L_TO_MIXER | RT5631_PWR_DAC_R_TO_MIXER,0);
		if (rt5631->pll_used_flag)
			snd_soc_update_bits(codec, RT5631_PWR_MANAG_ADD2,
						RT5631_PWR_PLL1,0);
	}
*/
	return 0;
}
static int adc_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);
	static bool pmu;

	switch (event) {
	case SND_SOC_DAPM_POST_PMD:
		if (pmu) {
			isPlaybackon = false;
			config_common_power(codec, false);
			pmu = false;
		}
		break;

	case SND_SOC_DAPM_PRE_PMU:
		if (!pmu) {
			isPlaybackon = true;
			config_common_power(codec, true);
			pmu = true;
		}
		break;

	default:
		break;
	}

	return 0;
}

static int dac_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = snd_soc_codec_get_drvdata(codec);;
	static bool pmu;

	switch (event) {
	case SND_SOC_DAPM_POST_PMD:
		if (pmu) {
			isCaptureon = false;
			config_common_power(codec, false);
			pmu = false;
		}
		break;

	case SND_SOC_DAPM_PRE_PMU:
		if (!pmu) {
			isCaptureon = true;
			config_common_power(codec, true);
			pmu = true;
		}
		break;

	default:
		break;
	}

	return 0;
}

static const struct snd_soc_dapm_widget rt5631_dapm_widgets[] = {
SND_SOC_DAPM_INPUT("MIC1"),
SND_SOC_DAPM_INPUT("MIC2"),
SND_SOC_DAPM_INPUT("AXIL"),
SND_SOC_DAPM_INPUT("AXIR"),
SND_SOC_DAPM_INPUT("MONOIN_RXN"),
SND_SOC_DAPM_INPUT("MONOIN_RXP"),

SND_SOC_DAPM_MICBIAS("Mic Bias1", RT5631_PWR_MANAG_ADD2, 3, 0),
SND_SOC_DAPM_MICBIAS("Mic Bias2", RT5631_PWR_MANAG_ADD2, 2, 0),

SND_SOC_DAPM_PGA_E("Mic1 Boost", RT5631_PWR_MANAG_ADD2, 5, 0, NULL, 0,
		mic_event, SND_SOC_DAPM_POST_PMU),
SND_SOC_DAPM_PGA_E("Mic2 Boost", RT5631_PWR_MANAG_ADD2, 4, 0, NULL, 0,
		mic_event, SND_SOC_DAPM_POST_PMU),
SND_SOC_DAPM_PGA("MONOIN_RXP Boost", RT5631_PWR_MANAG_ADD4, 7, 0, NULL, 0),
SND_SOC_DAPM_PGA("MONOIN_RXN Boost", RT5631_PWR_MANAG_ADD4, 6, 0, NULL, 0),
SND_SOC_DAPM_PGA("AXIL Boost", RT5631_PWR_MANAG_ADD4, 9, 0, NULL, 0),
SND_SOC_DAPM_PGA("AXIR Boost", RT5631_PWR_MANAG_ADD4, 8, 0, NULL, 0),
SND_SOC_DAPM_MIXER("MONO_IN", SND_SOC_NOPM, 0, 0, NULL, 0),

SND_SOC_DAPM_MIXER("RECMIXL Mixer", RT5631_PWR_MANAG_ADD2, 11, 0,
		&rt5631_recmixl_mixer_controls[0],
		ARRAY_SIZE(rt5631_recmixl_mixer_controls)),
SND_SOC_DAPM_MIXER("RECMIXR Mixer", RT5631_PWR_MANAG_ADD2, 10, 0,
		&rt5631_recmixr_mixer_controls[0],
		ARRAY_SIZE(rt5631_recmixr_mixer_controls)),
SND_SOC_DAPM_MIXER("ADC Mixer", SND_SOC_NOPM, 0, 0, NULL, 0),

SND_SOC_DAPM_ADC_E("Left ADC", "Left ADC HIFI Capture",
		RT5631_PWR_MANAG_ADD1, 11, 0,
		adc_event, SND_SOC_DAPM_POST_PMD | SND_SOC_DAPM_PRE_PMU),
SND_SOC_DAPM_ADC_E("Right ADC", "Right ADC HIFI Capture",
		RT5631_PWR_MANAG_ADD1, 10, 0,
		adc_event, SND_SOC_DAPM_POST_PMD | SND_SOC_DAPM_PRE_PMU),
SND_SOC_DAPM_DAC_E("Left DAC", "Left DAC HIFI Playback",
		RT5631_PWR_MANAG_ADD1, 9, 0,
		dac_event, SND_SOC_DAPM_POST_PMD | SND_SOC_DAPM_PRE_PMU),
SND_SOC_DAPM_DAC_E("Right DAC", "Right DAC HIFI Playback",
		RT5631_PWR_MANAG_ADD1, 8, 0,
		dac_event, SND_SOC_DAPM_POST_PMD | SND_SOC_DAPM_PRE_PMU),
SND_SOC_DAPM_DAC("Voice DAC", "Voice DAC Mono Playback", SND_SOC_NOPM, 0, 0),
SND_SOC_DAPM_PGA("Voice DAC Boost", SND_SOC_NOPM, 0, 0, NULL, 0),

SND_SOC_DAPM_MIXER("SPKMIXL Mixer", RT5631_PWR_MANAG_ADD2, 13, 0,
		&rt5631_spkmixl_mixer_controls[0],
		ARRAY_SIZE(rt5631_spkmixl_mixer_controls)),
SND_SOC_DAPM_MIXER("OUTMIXL Mixer", RT5631_PWR_MANAG_ADD2, 15, 0,
		&rt5631_outmixl_mixer_controls[0],
		ARRAY_SIZE(rt5631_outmixl_mixer_controls)),
SND_SOC_DAPM_MIXER("OUTMIXR Mixer", RT5631_PWR_MANAG_ADD2, 14, 0,
		&rt5631_outmixr_mixer_controls[0],
		ARRAY_SIZE(rt5631_outmixr_mixer_controls)),
SND_SOC_DAPM_MIXER("SPKMIXR Mixer", RT5631_PWR_MANAG_ADD2, 12, 0,
		&rt5631_spkmixr_mixer_controls[0],
		ARRAY_SIZE(rt5631_spkmixr_mixer_controls)),

SND_SOC_DAPM_PGA("Left SPK Vol", RT5631_PWR_MANAG_ADD4, 15, 0, NULL, 0),
SND_SOC_DAPM_PGA("Right SPK Vol", RT5631_PWR_MANAG_ADD4, 14, 0, NULL, 0),
SND_SOC_DAPM_PGA_E("Left HP Vol", RT5631_PWR_MANAG_ADD4, 11, 0, NULL, 0,
		hp_event, SND_SOC_DAPM_PRE_PMD | SND_SOC_DAPM_POST_PMU),
SND_SOC_DAPM_PGA_E("Right HP Vol", RT5631_PWR_MANAG_ADD4, 10, 0, NULL, 0,
		hp_event, SND_SOC_DAPM_PRE_PMD | SND_SOC_DAPM_POST_PMU),

SND_SOC_DAPM_PGA_E("Left DAC_HP", SND_SOC_NOPM, 0, 0, NULL, 0,
	dac_to_hp_event, SND_SOC_DAPM_PRE_PMD | SND_SOC_DAPM_POST_PMU),
SND_SOC_DAPM_PGA_E("Right DAC_HP", SND_SOC_NOPM, 0, 0, NULL, 0,
	dac_to_hp_event, SND_SOC_DAPM_PRE_PMD | SND_SOC_DAPM_POST_PMU),

SND_SOC_DAPM_PGA("Left Out Vol", RT5631_PWR_MANAG_ADD4, 13, 0, NULL, 0),
SND_SOC_DAPM_PGA("Right Out Vol", RT5631_PWR_MANAG_ADD4, 12, 0, NULL, 0),

SND_SOC_DAPM_MIXER_E("AXO1MIX Mixer", RT5631_PWR_MANAG_ADD3, 11, 0,
		&rt5631_AXO1MIX_mixer_controls[0],
		ARRAY_SIZE(rt5631_AXO1MIX_mixer_controls),
		auxo1_event, SND_SOC_DAPM_PRE_PMD | SND_SOC_DAPM_POST_PMU),
SND_SOC_DAPM_MIXER("SPOLMIX Mixer", SND_SOC_NOPM, 0, 0,
		&rt5631_spolmix_mixer_controls[0],
		ARRAY_SIZE(rt5631_spolmix_mixer_controls)),
SND_SOC_DAPM_MIXER("MONOMIX Mixer", RT5631_PWR_MANAG_ADD3, 9, 0,
		&rt5631_monomix_mixer_controls[0],
		ARRAY_SIZE(rt5631_monomix_mixer_controls)),
SND_SOC_DAPM_MIXER("SPORMIX Mixer", SND_SOC_NOPM, 0, 0,
		&rt5631_spormix_mixer_controls[0],
		ARRAY_SIZE(rt5631_spormix_mixer_controls)),
SND_SOC_DAPM_MIXER_E("AXO2MIX Mixer", RT5631_PWR_MANAG_ADD3, 10, 0,
		&rt5631_AXO2MIX_mixer_controls[0],
		ARRAY_SIZE(rt5631_AXO2MIX_mixer_controls),
		auxo2_event, SND_SOC_DAPM_PRE_PMD | SND_SOC_DAPM_POST_PMU),

SND_SOC_DAPM_MUX("SPOL Mux", SND_SOC_NOPM, 0, 0, &rt5631_spol_mux_control),
SND_SOC_DAPM_MUX("SPOR Mux", SND_SOC_NOPM, 0, 0, &rt5631_spor_mux_control),
SND_SOC_DAPM_MUX("Mono Mux", SND_SOC_NOPM, 0, 0, &rt5631_mono_mux_control),
SND_SOC_DAPM_MUX("HPL Mux", SND_SOC_NOPM, 0, 0, &rt5631_hpl_mux_control),
SND_SOC_DAPM_MUX("HPR Mux", SND_SOC_NOPM, 0, 0, &rt5631_hpr_mux_control),

SND_SOC_DAPM_PGA_E("Mono Amp", RT5631_PWR_MANAG_ADD3, 7, 0, NULL, 0,
		mono_event, SND_SOC_DAPM_PRE_PMD | SND_SOC_DAPM_POST_PMU),
SND_SOC_DAPM_PGA_E("SPKL Amp", SND_SOC_NOPM, 0, 0, NULL, 0,
		spk_event, SND_SOC_DAPM_POST_PMD | SND_SOC_DAPM_POST_PMU),
SND_SOC_DAPM_PGA_E("SPKR Amp", SND_SOC_NOPM, 1, 0, NULL, 0,
		spk_event, SND_SOC_DAPM_POST_PMD | SND_SOC_DAPM_POST_PMU),

SND_SOC_DAPM_OUTPUT("AUXO1"),
SND_SOC_DAPM_OUTPUT("AUXO2"),
SND_SOC_DAPM_OUTPUT("SPOL"),
SND_SOC_DAPM_OUTPUT("SPOR"),
SND_SOC_DAPM_OUTPUT("HPOL"),
SND_SOC_DAPM_OUTPUT("HPOR"),
SND_SOC_DAPM_OUTPUT("MONO"),
};

static const struct snd_soc_dapm_route rt5631_dapm_routes[] = {
	{"Mic1 Boost", NULL, "MIC1"},
	{"Mic2 Boost", NULL, "MIC2"},
	{"MONOIN_RXP Boost", NULL, "MONOIN_RXP"},
	{"MONOIN_RXN Boost", NULL, "MONOIN_RXN"},
	{"AXIL Boost", NULL, "AXIL"},
	{"AXIR Boost", NULL, "AXIR"},

	{"MONO_IN", NULL, "MONOIN_RXP Boost"},
	{"MONO_IN", NULL, "MONOIN_RXN Boost"},

	{"RECMIXL Mixer", "OUTMIXL Capture Switch", "OUTMIXL Mixer"},
	{"RECMIXL Mixer", "MIC1_BST1 Capture Switch", "Mic1 Boost"},
	{"RECMIXL Mixer", "AXILVOL Capture Switch", "AXIL Boost"},
	{"RECMIXL Mixer", "MONOIN_RX Capture Switch", "MONO_IN"},

	{"RECMIXR Mixer", "OUTMIXR Capture Switch", "OUTMIXR Mixer"},
	{"RECMIXR Mixer", "MIC2_BST2 Capture Switch", "Mic2 Boost"},
	{"RECMIXR Mixer", "AXIRVOL Capture Switch", "AXIR Boost"},
	{"RECMIXR Mixer", "MONOIN_RX Capture Switch", "MONO_IN"},

	{"ADC Mixer", NULL, "RECMIXL Mixer"},
	{"ADC Mixer", NULL, "RECMIXR Mixer"},
	{"Left ADC", NULL, "ADC Mixer"},
	{"Right ADC", NULL, "ADC Mixer"},

	{"Voice DAC Boost", NULL, "Voice DAC"},

	{"SPKMIXL Mixer", "RECMIXL Playback Switch", "RECMIXL Mixer"},
	{"SPKMIXL Mixer", "MIC1_P Playback Switch", "MIC1"},
	{"SPKMIXL Mixer", "DACL Playback Switch", "Left DAC"},
	{"SPKMIXL Mixer", "OUTMIXL Playback Switch", "OUTMIXL Mixer"},

	{"SPKMIXR Mixer", "OUTMIXR Playback Switch", "OUTMIXR Mixer"},
	{"SPKMIXR Mixer", "DACR Playback Switch", "Right DAC"},
	{"SPKMIXR Mixer", "MIC2_P Playback Switch", "MIC2"},
	{"SPKMIXR Mixer", "RECMIXR Playback Switch", "RECMIXR Mixer"},

	{"OUTMIXL Mixer", "RECMIXL Playback Switch", "RECMIXL Mixer"},
	{"OUTMIXL Mixer", "RECMIXR Playback Switch", "RECMIXR Mixer"},
	{"OUTMIXL Mixer", "DACL Playback Switch", "Left DAC"},
	{"OUTMIXL Mixer", "MIC1_BST1 Playback Switch", "Mic1 Boost"},
	{"OUTMIXL Mixer", "MIC2_BST2 Playback Switch", "Mic2 Boost"},
	{"OUTMIXL Mixer", "MONOIN_RXP Playback Switch", "MONOIN_RXP Boost"},
	{"OUTMIXL Mixer", "AXILVOL Playback Switch", "AXIL Boost"},
	{"OUTMIXL Mixer", "AXIRVOL Playback Switch", "AXIR Boost"},
	{"OUTMIXL Mixer", "VDAC Playback Switch", "Voice DAC Boost"},

	{"OUTMIXR Mixer", "RECMIXL Playback Switch", "RECMIXL Mixer"},
	{"OUTMIXR Mixer", "RECMIXR Playback Switch", "RECMIXR Mixer"},
	{"OUTMIXR Mixer", "DACR Playback Switch", "Right DAC"},
	{"OUTMIXR Mixer", "MIC1_BST1 Playback Switch", "Mic1 Boost"},
	{"OUTMIXR Mixer", "MIC2_BST2 Playback Switch", "Mic2 Boost"},
	{"OUTMIXR Mixer", "MONOIN_RXN Playback Switch", "MONOIN_RXN Boost"},
	{"OUTMIXR Mixer", "AXILVOL Playback Switch", "AXIL Boost"},
	{"OUTMIXR Mixer", "AXIRVOL Playback Switch", "AXIR Boost"},
	{"OUTMIXR Mixer", "VDAC Playback Switch", "Voice DAC Boost"},

	{"Left SPK Vol",  NULL, "SPKMIXL Mixer"},
	{"Right SPK Vol",  NULL, "SPKMIXR Mixer"},
	{"Left HP Vol",  NULL, "OUTMIXL Mixer"},
	{"Left Out Vol",  NULL, "OUTMIXL Mixer"},
	{"Right Out Vol",  NULL, "OUTMIXR Mixer"},
	{"Right HP Vol",  NULL, "OUTMIXR Mixer"},

	{"AXO1MIX Mixer", "MIC1_BST1 Playback Switch", "Mic1 Boost"},
	{"AXO1MIX Mixer", "OUTVOLL Playback Switch", "Left Out Vol"},
	{"AXO1MIX Mixer", "OUTVOLR Playback Switch", "Right Out Vol"},
	{"AXO1MIX Mixer", "MIC2_BST2 Playback Switch", "Mic2 Boost"},

	{"AXO2MIX Mixer", "MIC1_BST1 Playback Switch", "Mic1 Boost"},
	{"AXO2MIX Mixer", "OUTVOLL Playback Switch", "Left Out Vol"},
	{"AXO2MIX Mixer", "OUTVOLR Playback Switch", "Right Out Vol"},
	{"AXO2MIX Mixer", "MIC2_BST2 Playback Switch", "Mic2 Boost"},

	{"SPOLMIX Mixer", "SPKVOLL Playback Switch", "Left SPK Vol"},
	{"SPOLMIX Mixer", "SPKVOLR Playback Switch", "Right SPK Vol"},

	{"SPORMIX Mixer", "SPKVOLL Playback Switch", "Left SPK Vol"},
	{"SPORMIX Mixer", "SPKVOLR Playback Switch", "Right SPK Vol"},

	{"MONOMIX Mixer", "OUTVOLL Playback Switch", "Left Out Vol"},
	{"MONOMIX Mixer", "OUTVOLR Playback Switch", "Right Out Vol"},

	{"SPOL Mux", "SPOLMIX", "SPOLMIX Mixer"},
	{"SPOL Mux", "MONOIN_RX", "MONO_IN"},
	{"SPOL Mux", "VDAC", "Voice DAC Boost"},
	{"SPOL Mux", "DACL", "Left DAC"},

	{"SPOR Mux", "SPORMIX", "SPORMIX Mixer"},
	{"SPOR Mux", "MONOIN_RX", "MONO_IN"},
	{"SPOR Mux", "VDAC", "Voice DAC Boost"},
	{"SPOR Mux", "DACR", "Right DAC"},

	{"Mono Mux", "MONOMIX", "MONOMIX Mixer"},
	{"Mono Mux", "MONOIN_RX", "MONO_IN"},
	{"Mono Mux", "VDAC", "Voice DAC Boost"},

	{"Right DAC_HP", "NULL", "Right DAC"},
	{"Left DAC_HP", "NULL", "Left DAC"},

	{"HPL Mux", "LEFT HPVOL", "Left HP Vol"},
	{"HPL Mux", "LEFT DAC", "Left DAC_HP"},
	{"HPR Mux", "RIGHT HPVOL", "Right HP Vol"},
	{"HPR Mux", "RIGHT DAC", "Right DAC_HP"},

	{"SPKL Amp", NULL, "SPOL Mux"},
	{"SPKR Amp", NULL, "SPOR Mux"},
	{"Mono Amp", NULL, "Mono Mux"},

	{"AUXO1", NULL, "AXO1MIX Mixer"},
	{"AUXO2", NULL, "AXO2MIX Mixer"},
	{"SPOL", NULL, "SPKL Amp"},
	{"SPOR", NULL, "SPKR Amp"},

	{"HPOL", NULL, "HPL Mux"},
	{"HPOR", NULL, "HPR Mux"},

	{"MONO", NULL, "Mono Amp"}
};

static int rt5631_add_widgets(struct snd_soc_codec *codec)
{
	struct snd_soc_dapm_context *dapm = snd_soc_codec_get_dapm(codec);

	snd_soc_dapm_new_controls(dapm, rt5631_dapm_widgets,
			ARRAY_SIZE(rt5631_dapm_widgets));
	snd_soc_dapm_add_routes(dapm, rt5631_dapm_routes, ARRAY_SIZE(rt5631_dapm_routes));

	return 0;
}
struct coeff_clk_div {
	u32 mclk;
	u32 bclk;
	u32 rate;
	u16 reg_val;
};

/* PLL divisors */
struct pll_div {
	u32 pll_in;
	u32 pll_out;
	u16 reg_val;
};

static const struct pll_div codec_master_pll_div[] = {
	{2048000,  8192000,  0x0ea0},
	{3686400,  8192000,  0x4e27},
	{12000000,  8192000,  0x456b},
	{13000000,  8192000,  0x495f},
	{13100000,  8192000,  0x0320},
	{2048000,  11289600,  0xf637},
	{3686400,  11289600,  0x2f22},
	{12000000,  11289600,  0x3e2f},
	{13000000,  11289600,  0x4d5b},
	{13100000,  11289600,  0x363b},
	{2048000,  16384000,  0x1ea0},
	{3686400,  16384000,  0x9e27},
	{12000000,  16384000,  0x452b},
	{13000000,  16384000,  0x542f},
	{13100000,  16384000,  0x03a0},
	{2048000,  16934400,  0xe625},
	{3686400,  16934400,  0x9126},
	{12000000,  16934400,  0x4d2c},
	{13000000,  16934400,  0x742f},
	{13100000,  16934400,  0x3c27},
	{2048000,  22579200,  0x2aa0},
	{3686400,  22579200,  0x2f20},
	{12000000,  22579200,  0x7e2f},
	{13000000,  22579200,  0x742f},
	{13100000,  22579200,  0x3c27},
	{2048000,  24576000,  0x2ea0},
	{3686400,  24576000,  0xee27},
	{12000000,  24576000,  0x2915},
	{13000000,  24576000,  0x772e},
	{13100000,  24576000,  0x0d20},
	{26000000,  24576000,  0x2027},
	{26000000,  22579200,  0x392f},
	{24576000,  22579200,  0x0921},
	{24576000,  24576000,  0x02a0},
};

static const struct pll_div codec_slave_pll_div[] = {
	{256000,  2048000,  0x46f0},
	{256000,  4096000,  0x3ea0},
	{352800,  5644800,  0x3ea0},
	{512000,  8192000,  0x3ea0},
	{1024000,  8192000,  0x46f0},
	{705600,  11289600,  0x3ea0},
	{1024000,  16384000,  0x3ea0},
	{1411200,  22579200,  0x3ea0},
	{1536000,  24576000,  0x3ea0},
	{2048000,  16384000,  0x1ea0},
	{2822400,  22579200,  0x1ea0},
	{2822400,  45158400,  0x5ec0},
	{5644800,  45158400,  0x46f0},
	{3072000,  24576000,  0x1ea0},
	{3072000,  49152000,  0x5ec0},
	{6144000,  49152000,  0x46f0},
	{705600,  11289600,  0x3ea0},
	{705600,  8467200,  0x3ab0},
	{24576000,  24576000,  0x02a0},
	{1411200,  11289600,  0x1690},
	{2822400,  11289600,  0x0a90},
	{1536000,  12288000,  0x1690},
	{3072000,  12288000,  0x0a90},
};

static struct coeff_clk_div coeff_div[] = {
	/* sysclk is 256fs */
	{2048000,  8000 * 32,  8000, 0x1000},
	{2048000,  8000 * 64,  8000, 0x0000},
	{2822400,  11025 * 32,  11025,  0x1000},
	{2822400,  11025 * 64,  11025,  0x0000},
	{4096000,  16000 * 32,  16000,  0x1000},
	{4096000,  16000 * 64,  16000,  0x0000},
	{5644800,  22050 * 32,  22050,  0x1000},
	{5644800,  22050 * 64,  22050,  0x0000},
	{8192000,  32000 * 32,  32000,  0x1000},
	{8192000,  32000 * 64,  32000,  0x0000},
	{11289600,  44100 * 32,  44100,  0x1000},
	{11289600,  44100 * 64,  44100,  0x0000},
	{12288000,  48000 * 32,  48000,  0x1000},
	{12288000,  48000 * 64,  48000,  0x0000},
	{22579200,  88200 * 32,  88200,  0x1000},
	{22579200,  88200 * 64,  88200,  0x0000},
	{24576000,  96000 * 32,  96000,  0x1000},
	{24576000,  96000 * 64,  96000,  0x0000},
	{22579200,  176400 * 32,  176400,  0x1000},
	{22579200,  176400 * 64,  176400,  0x0000},
	{24576000,  192000 * 32,  192000,  0x1000},
	{24576000,  162000 * 64,  192000,  0x0000},
	/* sysclk is 512fs */
	{4096000,  8000 * 32,  8000, 0x3000},
	{4096000,  8000 * 64,  8000, 0x2000},
	{5644800,  11025 * 32,  11025, 0x3000},
	{5644800,  11025 * 64,  11025, 0x2000},
	{8192000,  16000 * 32,  16000, 0x3000},
	{8192000,  16000 * 64,  16000, 0x2000},
	{11289600,  22050 * 32,  22050, 0x3000},
	{11289600,  22050 * 64,  22050, 0x2000},
	{16384000,  32000 * 32,  32000, 0x3000},
	{16384000,  32000 * 64,  32000, 0x2000},
	{22579200,  44100 * 32,  44100, 0x3000},
	{22579200,  44100 * 64,  44100, 0x2000},
	{24576000,  48000 * 32,  48000, 0x3000},
	{24576000,  48000 * 64,  48000, 0x2000},
	{45158400,  88200 * 32,  88200, 0x3000},
	{45158400,  88200 * 64,  88200, 0x2000},
	{49152000,  96000 * 32,  96000, 0x3000},
	{49152000,  96000 * 64,  96000, 0x2000},
	/* sysclk is 24.576Mhz or 22.5792Mhz */
	{24576000,  8000 * 32,  8000,  0x7080},
	{24576000,  8000 * 64,  8000,  0x6080},
	{24576000,  16000 * 32,  16000,  0x5080},
	{24576000,  16000 * 64,  16000,  0x4080},
	{24576000,  24000 * 32,  24000,  0x5000},
	{24576000,  24000 * 64,  24000,  0x4000},
	{24576000,  32000 * 32,  32000,  0x3080},
	{24576000,  32000 * 64,  32000,  0x2080},
	{22579200,  11025 * 32,  11025,  0x7000},
	{22579200,  11025 * 64,  11025,  0x6000},
	{22579200,  22050 * 32,  22050,  0x5000},
	{22579200,  22050 * 64,  22050,  0x4000},
};

static int get_coeff(int mclk, int rate, int timesofbclk)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(coeff_div); i++) {
		if (coeff_div[i].mclk == mclk && coeff_div[i].rate == rate &&
			(coeff_div[i].bclk / coeff_div[i].rate) == timesofbclk)
			return i;
	}
	return -EINVAL;
}
static int get_coeff_in_slave_mode(int mclk, int rate)
{
	return get_coeff(mclk, rate, timesofbclk);
}

static int get_coeff_in_master_mode(int mclk, int rate, int bclk)
{
	return get_coeff(mclk, rate, (bclk / rate));
}

static void rt5631_set_dmic_params(struct snd_soc_codec *codec,
	struct snd_pcm_hw_params *params)
{
	int rate;

	snd_soc_update_bits(codec, RT5631_GPIO_CTRL,RT5631_GPIO_PIN_FUN_SEL_MASK | RT5631_GPIO_DMIC_FUN_SEL_MASK,
		RT5631_GPIO_PIN_FUN_SEL_GPIO_DIMC | RT5631_GPIO_DMIC_FUN_SEL_DIMC);
	snd_soc_update_bits(codec, RT5631_DIG_MIC_CTRL,RT5631_DMIC_ENA_MASK, RT5631_DMIC_ENA);
	snd_soc_update_bits(codec, RT5631_DIG_MIC_CTRL,RT5631_DMIC_L_CH_LATCH_MASK| RT5631_DMIC_R_CH_LATCH_MASK,
		RT5631_DMIC_L_CH_LATCH_FALLING | RT5631_DMIC_R_CH_LATCH_RISING);

	rate = params_rate(params);
	switch (rate) {
	case 44100:
	case 48000:
		snd_soc_update_bits(codec, RT5631_DIG_MIC_CTRL, RT5631_DMIC_CLK_CTRL_MASK,
			RT5631_DMIC_CLK_CTRL_TO_32FS);
		break;

	case 32000:
	case 22050:
		snd_soc_update_bits(codec, RT5631_DIG_MIC_CTRL,RT5631_DMIC_CLK_CTRL_MASK,
			RT5631_DMIC_CLK_CTRL_TO_64FS);
		break;

	case 16000:
	case 11025:
	case 8000:
		snd_soc_update_bits(codec, RT5631_DIG_MIC_CTRL,RT5631_DMIC_CLK_CTRL_MASK,
			RT5631_DMIC_CLK_CTRL_TO_128FS);
		break;

	default:
		break;
	}

	snd_soc_update_bits(codec, RT5631_DIG_MIC_CTRL,RT5631_DMIC_L_CH_MUTE | RT5631_DMIC_R_CH_MUTE,
		RT5631_DMIC_L_CH_UNMUTE | RT5631_DMIC_R_CH_UNMUTE);

	return;
}

static int rt5631_hifi_pcm_params(struct snd_pcm_substream *substream,
		struct snd_pcm_hw_params *params, struct snd_soc_dai *dai)
{
	struct snd_soc_codec *codec = dai->codec;
	struct rt5631_priv *rt5631 = snd_soc_codec_get_drvdata(codec);
	int stream = substream->stream, rate = params_rate(params), coeff;
	unsigned int iface = 0;

//	rt5631->sysclk = 24576000;
	dev_dbg(codec->dev, "enter %s\n", __func__);

	if (!rt5631->master){
		coeff = get_coeff_in_slave_mode(rt5631->sysclk, rate);
		}
	else{
		coeff = get_coeff_in_master_mode(rt5631->sysclk, rate,
					rate * timesofbclk);
		}
	if (coeff < 0) {
		dev_err(codec->dev, "Fail to get coeff\n");
//		return coeff;
	}

	switch (params_width(params)) {
	case 16:
		break;
	case 20:
		iface |= RT5631_SDP_I2S_DL_20;
		break;
	case 24:
		iface |= RT5631_SDP_I2S_DL_24;
		break;
	case 8:
		iface |= RT5631_SDP_I2S_DL_8;
		break;
	default:
		return -EINVAL;
	}

	if (SNDRV_PCM_STREAM_CAPTURE == stream) {
		if (rt5631->dmic_used_flag)
			rt5631_set_dmic_params(codec, params);
	}
	snd_soc_update_bits(codec, RT5631_SDP_CTRL,
		RT5631_SDP_I2S_DL_MASK, iface);

	if (coeff >= 0)
	snd_soc_write(codec, RT5631_STEREO_AD_DA_CLK_CTRL,
					0x3000);

	return 0;
}

static int rt5631_hifi_codec_set_dai_fmt(struct snd_soc_dai *codec_dai,
						unsigned int fmt)
{
	struct snd_soc_codec *codec = codec_dai->codec;
	struct rt5631_priv *rt5631 = snd_soc_codec_get_drvdata(codec);
	unsigned int iface = 0;

	dev_dbg(codec->dev, "enter %s\n", __func__);

	switch (fmt & SND_SOC_DAIFMT_MASTER_MASK) {
	case SND_SOC_DAIFMT_CBM_CFM:
		rt5631->master = 1;
		break;
	case SND_SOC_DAIFMT_CBS_CFS:
		iface |= RT5631_SDP_MODE_SEL_SLAVE;
		rt5631->master = 0;
		break;
	default:
		return -EINVAL;
	}

	switch (fmt & SND_SOC_DAIFMT_FORMAT_MASK) {
	case SND_SOC_DAIFMT_I2S:
		break;
	case SND_SOC_DAIFMT_LEFT_J:
		iface |= RT5631_SDP_I2S_DF_LEFT;
		break;
	case SND_SOC_DAIFMT_DSP_A:
		iface |= RT5631_SDP_I2S_DF_PCM_A;
		break;
	case SND_SOC_DAIFMT_DSP_B:
		iface  |= RT5631_SDP_I2S_DF_PCM_B;
		break;
	default:
		return -EINVAL;
	}

	switch (fmt & SND_SOC_DAIFMT_INV_MASK) {
	case SND_SOC_DAIFMT_NB_NF:
		break;
	case SND_SOC_DAIFMT_IB_NF:
		iface |= RT5631_SDP_I2S_BCLK_POL_CTRL;
		break;
	default:
		return -EINVAL;
	}

	snd_soc_write(codec, RT5631_SDP_CTRL, iface);

	return 0;
}

static int rt5631_hifi_codec_set_dai_sysclk(struct snd_soc_dai *codec_dai,
				int clk_id, unsigned int freq, int dir)
{
	struct snd_soc_codec *codec = codec_dai->codec;
	struct rt5631_priv *rt5631 = snd_soc_codec_get_drvdata(codec);

	dev_dbg(codec->dev, "enter %s, syclk=%d\n", __func__, freq);
	if ((freq >= (256 * 8000)) && (freq <= (512 * 96000))) {
		rt5631->sysclk = freq;
		return 0;
	}

	rt5631->sysclk = 24576000;
	return 0;
}

static int rt5631_codec_set_dai_pll(struct snd_soc_dai *codec_dai, int pll_id,
		int source, unsigned int freq_in, unsigned int freq_out)
{
	struct snd_soc_codec *codec = codec_dai->codec;
	struct rt5631_priv *rt5631 = snd_soc_codec_get_drvdata(codec);
	int i, ret = -EINVAL;

	dev_dbg(codec->dev, "enter %s\n", __func__);

	if (!freq_in || !freq_out) {
		dev_dbg(codec->dev, "PLL disabled\n");

//		snd_soc_update_bits(codec, RT5631_GLOBAL_CLK_CTRL,
//			RT5631_SYSCLK_SOUR_SEL_MASK,
//			RT5631_SYSCLK_SOUR_SEL_MCLK);

		return 0;
	}

	if (rt5631->master) {
		for (i = 0; i < ARRAY_SIZE(codec_master_pll_div); i++)
			if (freq_in == codec_master_pll_div[i].pll_in &&
			freq_out == codec_master_pll_div[i].pll_out) {
				dev_info(codec->dev,
					"change PLL in master mode\n");
				snd_soc_write(codec, RT5631_PLL_CTRL,
					codec_master_pll_div[i].reg_val);
				schedule_timeout_uninterruptible(
					msecs_to_jiffies(20));
				snd_soc_update_bits(codec,
					RT5631_GLOBAL_CLK_CTRL,
					RT5631_SYSCLK_SOUR_SEL_MASK |
					RT5631_PLLCLK_SOUR_SEL_MASK,
					RT5631_SYSCLK_SOUR_SEL_PLL |
					RT5631_PLLCLK_SOUR_SEL_MCLK);
				ret = 0;
				break;
			}
	} else {
		for (i = 0; i < ARRAY_SIZE(codec_slave_pll_div); i++)
			if (freq_in == codec_slave_pll_div[i].pll_in &&
			freq_out == codec_slave_pll_div[i].pll_out) {
				dev_info(codec->dev,
					"change PLL in slave mode\n");
				snd_soc_write(codec, RT5631_PLL_CTRL,
					codec_slave_pll_div[i].reg_val);
				schedule_timeout_uninterruptible(
					msecs_to_jiffies(20));
				snd_soc_update_bits(codec,
					RT5631_GLOBAL_CLK_CTRL,
					RT5631_SYSCLK_SOUR_SEL_MASK |
					RT5631_PLLCLK_SOUR_SEL_MASK,
					RT5631_SYSCLK_SOUR_SEL_PLL |
					RT5631_PLLCLK_SOUR_SEL_BCLK);
				rt5631->pll_used_flag = 1;
				ret = 0;
				break;
			}
	}

	return ret;
}





#define RT5631_STEREO_RATES SNDRV_PCM_RATE_8000_192000
#define RT5631_FORMAT	(SNDRV_PCM_FMTBIT_S16_LE | \
			SNDRV_PCM_FMTBIT_S20_3LE | \
			SNDRV_PCM_FMTBIT_S24_LE | \
			SNDRV_PCM_FMTBIT_S8)

static const struct snd_soc_dai_ops rt5631_ops = {
	.hw_params = rt5631_hifi_pcm_params,
	.set_fmt = rt5631_hifi_codec_set_dai_fmt,
	.set_sysclk = rt5631_hifi_codec_set_dai_sysclk,
	.set_pll = rt5631_codec_set_dai_pll,
};

static struct snd_soc_dai_driver rt5631_dai[] = {
	{
		.name = "rt5631-hifi",
		.id = 1,
		.playback = {
			.stream_name = "HIFI Playback",
			.channels_min = 1,
			.channels_max = 2,
			.rates = RT5631_STEREO_RATES,
			.formats = RT5631_FORMAT,
		},
		.capture = {
			.stream_name = "HIFI Capture",
			.channels_min = 1,
			.channels_max = 2,
			.rates = RT5631_STEREO_RATES,
			.formats = RT5631_FORMAT,
		},
		.ops = &rt5631_ops,
	},
};

static int rt5631_set_bias_level(struct snd_soc_codec *codec,
			enum snd_soc_bias_level level)
{
	struct rt5631_priv *rt5631 = snd_soc_codec_get_drvdata(codec);
	int ret;

	switch (level) {
	case SND_SOC_BIAS_ON:
	case SND_SOC_BIAS_PREPARE:
			if (IS_ERR(rt5631->mclk))
			break;

		if (snd_soc_codec_get_bias_level(codec) == SND_SOC_BIAS_ON) {
			clk_disable_unprepare(rt5631->mclk);
		} else {
			ret = clk_prepare_enable(rt5631->mclk);
			if (ret)
				return ret;
		}
		rt5631_write_mask(codec, RT5631_PWR_MANAG_ADD3,
			RT5631_PWR_VREF | RT5631_PWR_MAIN_BIAS, RT5631_PWR_VREF | RT5631_PWR_MAIN_BIAS);
		rt5631_write_mask(codec, RT5631_PWR_MANAG_ADD2,
			RT5631_PWR_MICBIAS1_VOL | RT5631_PWR_MICBIAS2_VOL,
			RT5631_PWR_MICBIAS1_VOL | RT5631_PWR_MICBIAS2_VOL);
		break;

	case SND_SOC_BIAS_STANDBY:
		
//		snd_soc_write(codec, RT5631_PWR_MANAG_ADD1, 0x0000);
		snd_soc_write(codec, RT5631_PWR_MANAG_ADD2, 0x0000);
//		snd_soc_write(codec, RT5631_PWR_MANAG_ADD3, 0x0000);
		snd_soc_write(codec, RT5631_PWR_MANAG_ADD4, 0x0000);
		break;

	case SND_SOC_BIAS_OFF:
		rt5631_write_mask(codec, RT5631_SPK_OUT_VOL,
			RT5631_L_MUTE | RT5631_R_MUTE, RT5631_L_MUTE | RT5631_R_MUTE);
		rt5631_write_mask(codec, RT5631_HP_OUT_VOL,
			RT5631_L_MUTE | RT5631_R_MUTE, RT5631_L_MUTE | RT5631_R_MUTE);
//		snd_soc_write(codec, RT5631_PWR_MANAG_ADD1, 0x0000);
		snd_soc_write(codec, RT5631_PWR_MANAG_ADD2, 0x0000);
//		snd_soc_write(codec, RT5631_PWR_MANAG_ADD3, 0x0000);
		snd_soc_write(codec, RT5631_PWR_MANAG_ADD4, 0x0000);
		break;

	default:
		break;
	}
	snd_soc_codec_get_dapm(codec)->bias_level = level;

	return 0;
}
static int rt5631_probe(struct snd_soc_codec *codec)
{
	struct rt5631_priv *rt5631 = snd_soc_codec_get_drvdata(codec);
	unsigned int val;
	
	rt5631->mclk = devm_clk_get(codec->dev, "mclk");
	if (PTR_ERR(rt5631->mclk) == -EPROBE_DEFER)
		return -EPROBE_DEFER;
	val = rt5631_read_index(codec, RT5631_ADDA_MIXER_INTL_REG3);
	if(val < 0)
	{
		return -ENODEV;
	}
	if (val & 0x0002)
		rt5631->codec_version = 1;
	else
		rt5631->codec_version = 0;

	rt5631_reset(codec);
	snd_soc_update_bits(codec, RT5631_PWR_MANAG_ADD3,
		 RT5631_PWR_VREF | RT5631_PWR_MAIN_BIAS, RT5631_PWR_VREF | RT5631_PWR_MAIN_BIAS);
	msleep(80);
	snd_soc_update_bits(codec, RT5631_PWR_MANAG_ADD3,RT5631_PWR_FAST_VREF_CTRL ,
					RT5631_PWR_FAST_VREF_CTRL);
	rt5631_reg_init(codec);
	if (rt5631->phone_det_level == 1)
		rt5631_write(codec, RT5631_JACK_DET_CTRL,0x4e80);
	else
		rt5631_write(codec, RT5631_JACK_DET_CTRL,0x4bc0);
/*		
	rt5631_reset(codec);
	snd_soc_update_bits(codec, RT5631_PWR_MANAG_ADD3,

		RT5631_PWR_VREF | RT5631_PWR_MAIN_BIAS,
		RT5631_PWR_VREF | RT5631_PWR_MAIN_BIAS);

	msleep(80);
	snd_soc_update_bits(codec, RT5631_PWR_MANAG_ADD3,
		RT5631_PWR_FAST_VREF_CTRL, RT5631_PWR_FAST_VREF_CTRL);
	// enable HP zero cross 
	snd_soc_write(codec, RT5631_INT_ST_IRQ_CTRL_2, 0x0f18);
*/
	/* power off ClassD auto Recovery */
	if (rt5631->codec_version)
		snd_soc_update_bits(codec, RT5631_INT_ST_IRQ_CTRL_2,
					0x2000, 0x2000);
	else
		snd_soc_update_bits(codec, RT5631_INT_ST_IRQ_CTRL_2,
					0x2000, 0);
	snd_soc_codec_get_dapm(codec)->bias_level = SND_SOC_BIAS_STANDBY;
	rt5631_codec = codec;
	// DMIC
/*	if (rt5631->dmic_used_flag) {
		snd_soc_update_bits(codec, RT5631_GPIO_CTRL,
			RT5631_GPIO_PIN_FUN_SEL_MASK |
			RT5631_GPIO_DMIC_FUN_SEL_MASK,
			RT5631_GPIO_PIN_FUN_SEL_GPIO_DIMC |
			RT5631_GPIO_DMIC_FUN_SEL_DIMC);
		snd_soc_update_bits(codec, RT5631_DIG_MIC_CTRL,
			RT5631_DMIC_L_CH_LATCH_MASK |
			RT5631_DMIC_R_CH_LATCH_MASK,
			RT5631_DMIC_L_CH_LATCH_FALLING |
			RT5631_DMIC_R_CH_LATCH_RISING);
	}

	snd_soc_codec_init_bias_level(codec, SND_SOC_BIAS_STANDBY);
*/
/*
//bard 7-16 s
	INIT_DELAYED_WORK(&rt5631_delay_cap,rt5631_adc_on);
//bard 7-16 e
	snd_soc_add_codec_controls(codec, rt5631_snd_controls,
		ARRAY_SIZE(rt5631_snd_controls));
	rt5631_add_widgets(codec);
*/
	return 0;
}
/*
 * detect short current for mic1
 */
int rt5631_ext_mic_detect(void)
{
	struct snd_soc_codec *codec = rt5631_codec;
	int det;

	rt5631_write_mask(codec, RT5631_MIC_CTRL_2, RT5631_MICBIAS1_S_C_DET_ENA,
				RT5631_MICBIAS1_S_C_DET_MASK);
	det = rt5631_read(codec, RT5631_INT_ST_IRQ_CTRL_2) & 0x0001;
	rt5631_write_mask(codec, RT5631_INT_ST_IRQ_CTRL_2, 0x0001, 0x00001);

	return det;
}
EXPORT_SYMBOL_GPL(rt5631_ext_mic_detect);

static struct snd_soc_codec_driver soc_codec_dev_rt5631 = {
	.probe = rt5631_probe,
	.set_bias_level = rt5631_set_bias_level,
	.suspend_bias_off = true,
	.controls = rt5631_snd_controls,
	.num_controls = ARRAY_SIZE(rt5631_snd_controls),
	.dapm_widgets = rt5631_dapm_widgets,
	.num_dapm_widgets = ARRAY_SIZE(rt5631_dapm_widgets),
	.dapm_routes = rt5631_dapm_routes,
	.num_dapm_routes = ARRAY_SIZE(rt5631_dapm_routes),
};

static const struct i2c_device_id rt5631_i2c_id[] = {
	{ "rt5631", 0 },
	{ "alc5631", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, rt5631_i2c_id);

#ifdef CONFIG_OF
static const struct of_device_id rt5631_i2c_dt_ids[] = {
	{ .compatible = "realtek,rt5631"},
	{ .compatible = "realtek,alc5631"},
	{ }
};
MODULE_DEVICE_TABLE(of, rt5631_i2c_dt_ids);
#endif

static const struct regmap_config rt5631_regmap_config = {
	.reg_bits = 8,
	.val_bits = 16,

	.readable_reg = rt5631_readable_register,
	.volatile_reg = rt5631_volatile_register,
	.max_register = RT5631_VENDOR_ID2,
	.reg_defaults = rt5631_reg,
	.num_reg_defaults = ARRAY_SIZE(rt5631_reg),
	.cache_type = REGCACHE_RBTREE,
};

static int rt5631_i2c_probe(struct i2c_client *i2c,
		    const struct i2c_device_id *id)
{
	struct rt5631_priv *rt5631;
	struct device_node *node = i2c->dev.of_node;
	struct i2c_adapter *adapter = to_i2c_adapter(i2c->dev.parent);
	int ret,reg;
	int m;

	rt5631 = devm_kzalloc(&i2c->dev, sizeof(struct rt5631_priv),
			      GFP_KERNEL);
	if (NULL == rt5631)
		return -ENOMEM;

        if (!i2c_check_functionality(adapter, I2C_FUNC_I2C)) {
                dev_warn(&adapter->dev,
                         "I2C-Adapter doesn't support I2C_FUNC_I2C\n");
                return -EIO;
        }
	reg = RT5631_SPK_OUT_VOL;
	
	for(m=0; m<10; m++)
	{
		ret = i2c_master_send(i2c, &reg, 1);
		printk("*");
		if(ret >= 0)
			break;
		else
			msleep(50);
	}
	printk("\n");
	
	if (ret < 0){
		printk("RT5631 probe error\n");
		return ret;
	}


#ifdef  CONFIG_OF
	ret = of_property_read_u32(node,"phone_det_level",&rt5631->phone_det_level);
	if (ret < 0)
		printk("%s get phone_det_level error\n",__func__);
	else
		printk("RT5631 codec: phone_det_level %s",rt5631->phone_det_level ? "HIGH":"LOW");
#endif
	i2c_set_clientdata(i2c, rt5631);

	rt5631->regmap = devm_regmap_init_i2c(i2c, &rt5631_regmap_config);
	if (IS_ERR(rt5631->regmap))
		return PTR_ERR(rt5631->regmap);
		
	ret = snd_soc_register_codec(&i2c->dev, &soc_codec_dev_rt5631,
			rt5631_dai, ARRAY_SIZE(rt5631_dai));
	return ret;
}

static int rt5631_i2c_remove(struct i2c_client *client)
{
	snd_soc_unregister_codec(&client->dev);
	return 0;
}

static struct i2c_driver rt5631_i2c_driver = {
	.driver = {
		.name = "rt5631",
		.of_match_table = of_match_ptr(rt5631_i2c_dt_ids),
	},
	.probe = rt5631_i2c_probe,
	.remove   = rt5631_i2c_remove,
	.id_table = rt5631_i2c_id,
};

module_i2c_driver(rt5631_i2c_driver);

MODULE_DESCRIPTION("ASoC RT5631 driver");
MODULE_AUTHOR("flove <flove@realtek.com>");
MODULE_LICENSE("GPL");
