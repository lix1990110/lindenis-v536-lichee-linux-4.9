/*
 * sound\soc\sunxi\sun8iw19-codec.c
 * (C) Copyright 2014-2019
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * yumingfeng <yumingfeng@allwinnertech.com>
 *
 * some simple description for this code
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 */

#include <linux/module.h>
#include <linux/slab.h>
#include <linux/clk.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/pm.h>
#include <linux/regulator/consumer.h>
#include <linux/pinctrl/consumer.h>
#include <linux/dma/sunxi-dma.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>
#include <sound/initval.h>
#include <sound/tlv.h>
#include <linux/sunxi-gpio.h>

#include "sun8iw19-codec.h"
#include "sun8iw19-pcm.h"

static const struct sample_rate sample_rate_conv[] = {
	{44100, 0},
	{48000, 0},
	{8000, 5},
	{32000, 1},
	{22050, 2},
	{24000, 2},
	{16000, 3},
	{11025, 4},
	{12000, 4},
	{192000, 6},
	{96000, 7},
};

static const DECLARE_TLV_DB_SCALE(digital_tlv, -7424, 116, 0);
static const DECLARE_TLV_DB_SCALE(linein_tlv, 0, 600, 0);
static const DECLARE_TLV_DB_SCALE(mic_gain_tlv, 0, 100, 0);

static const unsigned int lineout_tlv[] = {
	TLV_DB_RANGE_HEAD(2),
	0, 1, TLV_DB_SCALE_ITEM(0, 0, 1),
	2, 31, TLV_DB_SCALE_ITEM(-4350, 150, 1),
};

#ifdef SUNXI_CODEC_DAP_ENABLE
static void adcdrc_config(struct snd_soc_codec *codec)
{
	/* Left peak filter attack time */
	snd_soc_write(codec, SUNXI_ADC_DRC_LPFHAT, (0x000B77BF >> 16) & 0xFFFF);
	snd_soc_write(codec, SUNXI_ADC_DRC_LPFLAT, 0x000B77BF & 0xFFFF);
	/* Right peak filter attack time */
	snd_soc_write(codec, SUNXI_ADC_DRC_RPFHAT, (0x000B77BF >> 16) & 0xFFFF);
	snd_soc_write(codec, SUNXI_ADC_DRC_RPFLAT, 0x000B77BF & 0xFFFF);
	/* Left peak filter release time */
	snd_soc_write(codec, SUNXI_ADC_DRC_LPFHRT, (0x00FFE1F8 >> 16) & 0xFFFF);
	snd_soc_write(codec, SUNXI_ADC_DRC_LPFLRT, 0x00FFE1F8 & 0xFFFF);
	/* Right peak filter release time */
	snd_soc_write(codec, SUNXI_ADC_DRC_RPFHRT, (0x00FFE1F8 >> 16) & 0xFFFF);
	snd_soc_write(codec, SUNXI_ADC_DRC_RPFLRT, 0x00FFE1F8 & 0xFFFF);

	/* Left RMS filter attack time */
	snd_soc_write(codec, SUNXI_ADC_DRC_LPFHAT, (0x00012BAF >> 16) & 0xFFFF);
	snd_soc_write(codec, SUNXI_ADC_DRC_LPFLAT, 0x00012BAF & 0xFFFF);
	/* Right RMS filter attack time */
	snd_soc_write(codec, SUNXI_ADC_DRC_RPFHAT, (0x00012BAF >> 16) & 0xFFFF);
	snd_soc_write(codec, SUNXI_ADC_DRC_RPFLAT, 0x00012BAF & 0xFFFF);

	/* smooth filter attack time */
	snd_soc_write(codec, SUNXI_ADC_DRC_SFHAT, (0x00025600 >> 16) & 0xFFFF);
	snd_soc_write(codec, SUNXI_ADC_DRC_SFLAT, 0x00025600 & 0xFFFF);
	/* gain smooth filter release time */
	snd_soc_write(codec, SUNXI_ADC_DRC_SFHRT, (0x00000F04 >> 16) & 0xFFFF);
	snd_soc_write(codec, SUNXI_ADC_DRC_SFLRT, 0x00000F04 & 0xFFFF);

	/* OPL */
	snd_soc_write(codec, SUNXI_ADC_DRC_HOPL, (0xFBD8FBA7 >> 16) & 0xFFFF);
	snd_soc_write(codec, SUNXI_ADC_DRC_LOPL, 0xFBD8FBA7 & 0xFFFF);
	/* OPC */
	snd_soc_write(codec, SUNXI_ADC_DRC_HOPC, (0xF95B2C3F >> 16) & 0xFFFF);
	snd_soc_write(codec, SUNXI_ADC_DRC_LOPC, 0xF95B2C3F & 0xFFFF);
	/* OPE */
	snd_soc_write(codec, SUNXI_ADC_DRC_HOPE, (0xF45F8D6E >> 16) & 0xFFFF);
	snd_soc_write(codec, SUNXI_ADC_DRC_LOPE, 0xF45F8D6E & 0xFFFF);
	/* LT */
	snd_soc_write(codec, SUNXI_ADC_DRC_HLT, (0x01A934F0 >> 16) & 0xFFFF);
	snd_soc_write(codec, SUNXI_ADC_DRC_LLT, 0x01A934F0 & 0xFFFF);
	/* CT */
	snd_soc_write(codec, SUNXI_ADC_DRC_HCT, (0x06A4D3C0 >> 16) & 0xFFFF);
	snd_soc_write(codec, SUNXI_ADC_DRC_LCT, 0x06A4D3C0 & 0xFFFF);
	/* ET */
	snd_soc_write(codec, SUNXI_ADC_DRC_HET, (0x0BA07291 >> 16) & 0xFFFF);
	snd_soc_write(codec, SUNXI_ADC_DRC_LET, 0x0BA07291 & 0xFFFF);
	/* Ki */
	snd_soc_write(codec, SUNXI_ADC_DRC_HKI, (0x00051EB8 >> 16) & 0xFFFF);
	snd_soc_write(codec, SUNXI_ADC_DRC_LKI, 0x00051EB8 & 0xFFFF);
	/* Kc */
	snd_soc_write(codec, SUNXI_ADC_DRC_HKC, (0x00800000 >> 16) & 0xFFFF);
	snd_soc_write(codec, SUNXI_ADC_DRC_LKC, 0x00800000 & 0xFFFF);
	/* Kn */
	snd_soc_write(codec, SUNXI_ADC_DRC_HKN, (0x01000000 >> 16) & 0xFFFF);
	snd_soc_write(codec, SUNXI_ADC_DRC_LKN, 0x01000000 & 0xFFFF);
	/* Ke */
	snd_soc_write(codec, SUNXI_ADC_DRC_HKE, (0x0000F45F >> 16) & 0xFFFF);
	snd_soc_write(codec, SUNXI_ADC_DRC_LKE, 0x0000F45F & 0xFFFF);
}

static void adcdrc_enable(struct snd_soc_codec *codec, bool on)
{
	struct sunxi_codec_info *sunxi_codec = snd_soc_codec_get_drvdata(codec);

	if (on) {
		snd_soc_update_bits(codec, SUNXI_ADC_DAP_CTL,
			(0x1 << ADC_DRC0_EN), (0x1 << ADC_DRC0_EN));

		if (sunxi_codec->adc_dap_enable++ == 0) {
			snd_soc_update_bits(codec, SUNXI_ADC_DAP_CTL,
				(0x1 << ADC_DAP0_EN), (0x1 << ADC_DAP0_EN));
		}
	} else {
		if (--sunxi_codec->adc_dap_enable == 0) {
			snd_soc_update_bits(codec, SUNXI_ADC_DAP_CTL,
				(0x1 << ADC_DAP0_EN), (0x0 << ADC_DAP0_EN));
		}
		snd_soc_update_bits(codec, SUNXI_ADC_DAP_CTL,
			(0x1 << ADC_DRC0_EN), (0x0 << ADC_DRC0_EN));
	}
}

static void adchpf_config(struct snd_soc_codec *codec)
{
	/* HPF */
	snd_soc_write(codec, SUNXI_ADC_DRC_HHPFC, (0xFFFAC1 >> 16) & 0xFFFF);
	snd_soc_write(codec, SUNXI_ADC_DRC_LHPFC, 0xFFFAC1 & 0xFFFF);
}

static void adchpf_enable(struct snd_soc_codec *codec, bool on)
{
	struct sunxi_codec_info *sunxi_codec = snd_soc_codec_get_drvdata(codec);

	if (on) {
		snd_soc_update_bits(codec, SUNXI_ADC_DAP_CTL,
			(0x1 << ADC_HPF0_EN), (0x1 << ADC_HPF0_EN));

		if (sunxi_codec->adc_dap_enable++ == 0) {
			snd_soc_update_bits(codec, SUNXI_ADC_DAP_CTL,
				(0x1 << ADC_DAP0_EN), (0x1 << ADC_DAP0_EN));
		}
	} else {
		if (--sunxi_codec->adc_dap_enable == 0) {
			snd_soc_update_bits(codec, SUNXI_ADC_DAP_CTL,
				(0x1 << ADC_DAP0_EN), (0x0 << ADC_DAP0_EN));
		}
		snd_soc_update_bits(codec, SUNXI_ADC_DAP_CTL,
			(0x1 << ADC_HPF0_EN), (0x0 << ADC_HPF0_EN));
	}
}

static void dacdrc_config(struct snd_soc_codec *codec)
{
	/* Left peak filter attack time */
	snd_soc_write(codec, SUNXI_DAC_DRC_LPFHAT, (0x000B77BF >> 16) & 0xFFFF);
	snd_soc_write(codec, SUNXI_DAC_DRC_LPFLAT, 0x000B77BF & 0xFFFF);
	/* Right peak filter attack time */
	snd_soc_write(codec, SUNXI_DAC_DRC_RPFHAT, (0x000B77BF >> 16) & 0xFFFF);
	snd_soc_write(codec, SUNXI_DAC_DRC_RPFLAT, 0x000B77BF & 0xFFFF);
	/* Left peak filter release time */
	snd_soc_write(codec, SUNXI_DAC_DRC_LPFHRT, (0x00FFE1F8 >> 16) & 0xFFFF);
	snd_soc_write(codec, SUNXI_DAC_DRC_LPFLRT, 0x00FFE1F8 & 0xFFFF);
	/* Right peak filter release time */
	snd_soc_write(codec, SUNXI_DAC_DRC_RPFHRT, (0x00FFE1F8 >> 16) & 0xFFFF);
	snd_soc_write(codec, SUNXI_DAC_DRC_RPFLRT, 0x00FFE1F8 & 0xFFFF);

	/* Left RMS filter attack time */
	snd_soc_write(codec, SUNXI_DAC_DRC_LPFHAT, (0x00012BAF >> 16) & 0xFFFF);
	snd_soc_write(codec, SUNXI_DAC_DRC_LPFLAT, 0x00012BAF & 0xFFFF);
	/* Right RMS filter attack time */
	snd_soc_write(codec, SUNXI_DAC_DRC_RPFHAT, (0x00012BAF >> 16) & 0xFFFF);
	snd_soc_write(codec, SUNXI_DAC_DRC_RPFLAT, 0x00012BAF & 0xFFFF);

	/* smooth filter attack time */
	snd_soc_write(codec, SUNXI_DAC_DRC_SFHAT, (0x00025600 >> 16) & 0xFFFF);
	snd_soc_write(codec, SUNXI_DAC_DRC_SFLAT, 0x00025600 & 0xFFFF);
	/* gain smooth filter release time */
	snd_soc_write(codec, SUNXI_DAC_DRC_SFHRT, (0x00000F04 >> 16) & 0xFFFF);
	snd_soc_write(codec, SUNXI_DAC_DRC_SFLRT, 0x00000F04 & 0xFFFF);

	/* OPL */
	snd_soc_write(codec, SUNXI_DAC_DRC_HOPL, (0xFBD8FBA7 >> 16) & 0xFFFF);
	snd_soc_write(codec, SUNXI_DAC_DRC_LOPL, 0xFBD8FBA7 & 0xFFFF);
	/* OPC */
	snd_soc_write(codec, SUNXI_DAC_DRC_HOPC, (0xF95B2C3F >> 16) & 0xFFFF);
	snd_soc_write(codec, SUNXI_DAC_DRC_LOPC, 0xF95B2C3F & 0xFFFF);
	/* OPE */
	snd_soc_write(codec, SUNXI_DAC_DRC_HOPE, (0xF45F8D6E >> 16) & 0xFFFF);
	snd_soc_write(codec, SUNXI_DAC_DRC_LOPE, 0xF45F8D6E & 0xFFFF);
	/* LT */
	snd_soc_write(codec, SUNXI_DAC_DRC_HLT, (0x01A934F0 >> 16) & 0xFFFF);
	snd_soc_write(codec, SUNXI_DAC_DRC_LLT, 0x01A934F0 & 0xFFFF);
	/* CT */
	snd_soc_write(codec, SUNXI_DAC_DRC_HCT, (0x06A4D3C0 >> 16) & 0xFFFF);
	snd_soc_write(codec, SUNXI_DAC_DRC_LCT, 0x06A4D3C0 & 0xFFFF);
	/* ET */
	snd_soc_write(codec, SUNXI_DAC_DRC_HET, (0x0BA07291 >> 16) & 0xFFFF);
	snd_soc_write(codec, SUNXI_DAC_DRC_LET, 0x0BA07291 & 0xFFFF);
	/* Ki */
	snd_soc_write(codec, SUNXI_DAC_DRC_HKI, (0x00051EB8 >> 16) & 0xFFFF);
	snd_soc_write(codec, SUNXI_DAC_DRC_LKI, 0x00051EB8 & 0xFFFF);
	/* Kc */
	snd_soc_write(codec, SUNXI_DAC_DRC_HKC, (0x00800000 >> 16) & 0xFFFF);
	snd_soc_write(codec, SUNXI_DAC_DRC_LKC, 0x00800000 & 0xFFFF);
	/* Kn */
	snd_soc_write(codec, SUNXI_DAC_DRC_HKN, (0x01000000 >> 16) & 0xFFFF);
	snd_soc_write(codec, SUNXI_DAC_DRC_LKN, 0x01000000 & 0xFFFF);
	/* Ke */
	snd_soc_write(codec, SUNXI_DAC_DRC_HKE, (0x0000F45F >> 16) & 0xFFFF);
	snd_soc_write(codec, SUNXI_DAC_DRC_LKE, 0x0000F45F & 0xFFFF);
}

static void dacdrc_enable(struct snd_soc_codec *codec, bool on)
{
	struct sunxi_codec_info *sunxi_codec = snd_soc_codec_get_drvdata(codec);

	if (on) {
		/* detect noise when ET enable */
		snd_soc_update_bits(codec, SUNXI_DAC_DRC_CTRL,
			(0x1 << DAC_DRC_NOISE_DET_EN),
			(0x1 << DAC_DRC_NOISE_DET_EN));
		/* 0x0:RMS filter; 0x1:Peak filter */
		snd_soc_update_bits(codec, SUNXI_DAC_DRC_CTRL,
			(0x1 << DAC_DRC_SIGNAL_SEL),
			(0x1 << DAC_DRC_SIGNAL_SEL));
		/* delay function enable */
		snd_soc_update_bits(codec, SUNXI_DAC_DRC_CTRL,
			(0x1 << DAC_DRC_DELAY_EN),
			(0x0 << DAC_DRC_DELAY_EN));

		snd_soc_update_bits(codec, SUNXI_DAC_DRC_CTRL,
			(0x1 << DAC_DRC_LT_EN),
			(0x1 << DAC_DRC_LT_EN));
		snd_soc_update_bits(codec, SUNXI_DAC_DRC_CTRL,
			(0x1 << DAC_DRC_ET_EN),
			(0x1 << DAC_DRC_ET_EN));

		snd_soc_update_bits(codec, SUNXI_DAC_DAP_CTL,
			(0x1 << DDAP_DRC_EN),
			(0x1 << DDAP_DRC_EN));

		if (sunxi_codec->dac_dap_enable++ == 0)
			snd_soc_update_bits(codec, SUNXI_DAC_DAP_CTL,
				(0x1 << DDAP_EN),
				(0x1 << DDAP_EN));
	} else {
		if (--sunxi_codec->dac_dap_enable == 0)
			snd_soc_update_bits(codec, SUNXI_DAC_DAP_CTL,
				(0x1 << DDAP_EN),
				(0x0 << DDAP_EN));

		snd_soc_update_bits(codec, SUNXI_DAC_DAP_CTL,
			(0x1 << DDAP_DRC_EN),
			(0x0 << DDAP_DRC_EN));

		/* detect noise when ET enable */
		snd_soc_update_bits(codec, SUNXI_DAC_DRC_CTRL,
			(0x1 << DAC_DRC_NOISE_DET_EN),
			(0x0 << DAC_DRC_NOISE_DET_EN));
		/* 0x0:RMS filter; 0x1:Peak filter */
		snd_soc_update_bits(codec, SUNXI_DAC_DRC_CTRL,
			(0x1 << DAC_DRC_SIGNAL_SEL),
			(0x1 << DAC_DRC_SIGNAL_SEL));
		/* delay function enable */
		snd_soc_update_bits(codec, SUNXI_DAC_DRC_CTRL,
			(0x1 << DAC_DRC_DELAY_EN),
			(0x0 << DAC_DRC_DELAY_EN));

		snd_soc_update_bits(codec, SUNXI_DAC_DRC_CTRL,
			(0x1 << DAC_DRC_LT_EN),
			(0x0 << DAC_DRC_LT_EN));
		snd_soc_update_bits(codec, SUNXI_DAC_DRC_CTRL,
			(0x1 << DAC_DRC_ET_EN),
			(0x0 << DAC_DRC_ET_EN));
	}
}

static void dachpf_config(struct snd_soc_codec *codec)
{
	/* HPF */
	snd_soc_write(codec, SUNXI_DAC_DRC_HHPFC, (0xFFFAC1 >> 16) & 0xFFFF);
	snd_soc_write(codec, SUNXI_DAC_DRC_LHPFC, 0xFFFAC1 & 0xFFFF);
}

static void dachpf_enable(struct snd_soc_codec *codec, bool on)
{
	struct sunxi_codec_info *sunxi_codec = snd_soc_codec_get_drvdata(codec);

	if (on) {
		snd_soc_update_bits(codec, SUNXI_DAC_DAP_CTL,
			(0x1 << DDAP_HPF_EN),
			(0x1 << DDAP_HPF_EN));

		if (sunxi_codec->dac_dap_enable++ == 0)
			snd_soc_update_bits(codec, SUNXI_DAC_DAP_CTL,
				(0x1 << DDAP_EN),
				(0x1 << DDAP_EN));
	} else {
		if (--sunxi_codec->dac_dap_enable == 0)
			snd_soc_update_bits(codec, SUNXI_DAC_DAP_CTL,
				(0x1 << DDAP_EN),
				(0x0 << DDAP_EN));

		snd_soc_update_bits(codec, SUNXI_DAC_DAP_CTL,
			(0x1 << DDAP_HPF_EN),
			(0x0 << DDAP_HPF_EN));
	}
}
#endif

#ifdef SUNXI_CODEC_HUB_ENABLE
static int sunxi_codec_get_hub_mode(struct snd_kcontrol *kcontrol,
			struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_component *component = snd_kcontrol_chip(kcontrol);
	struct snd_soc_codec *codec = snd_soc_component_to_codec(component);
	unsigned int reg_val;

	reg_val = snd_soc_read(codec, SUNXI_DAC_DPC);

	ucontrol->value.integer.value[0] = ((reg_val & (1 << DAC_HUB_EN)) ? 2 : 1);

	return 0;
}

static int sunxi_codec_set_hub_mode(struct snd_kcontrol *kcontrol,
			struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_component *component = snd_kcontrol_chip(kcontrol);
	struct snd_soc_codec *codec = snd_soc_component_to_codec(component);

	switch (ucontrol->value.integer.value[0]) {
	case	0:
		snd_soc_update_bits(codec, SUNXI_DAC_DPC,
				(0x1 << DAC_HUB_EN), (0x0 << DAC_HUB_EN));
		break;
	case	1:
		snd_soc_update_bits(codec, SUNXI_DAC_DPC,
				(0x1 << DAC_HUB_EN), (0x1 << DAC_HUB_EN));
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

/* sunxi codec hub mdoe select */
static const char * const sunxi_codec_hub_function[] = {
				"hub_disable", "hub_enable"};

static const struct soc_enum sunxi_codec_hub_mode_enum[] = {
	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(sunxi_codec_hub_function),
			sunxi_codec_hub_function),
};
#endif

static int sunxi_codec_speaker_event(struct snd_soc_dapm_widget *w,
				struct snd_kcontrol *k, int event)
{
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);
	struct sunxi_codec_info *sunxi_codec = snd_soc_codec_get_drvdata(codec);
	struct codec_spk_config *spk_cfg = &(sunxi_codec->spk_config);

	switch (event) {
	case	SND_SOC_DAPM_POST_PMU:
		if (spk_cfg->used) {
			gpio_set_value(spk_cfg->spk_gpio, spk_cfg->pa_level);
			/* time delay to wait spk pa work fine */
			mdelay(spk_cfg->pa_msleep);
		}
		break;
	case	SND_SOC_DAPM_PRE_PMD:
		if (spk_cfg->used)
			gpio_set_value(spk_cfg->spk_gpio, !(spk_cfg->pa_level));
		break;
	default:
		break;
	}
	return 0;
}

static int sunxi_codec_lineout_event(struct snd_soc_dapm_widget *w,
				  struct snd_kcontrol *k, int event)
{
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);

	switch (event) {
	case	SND_SOC_DAPM_POST_PMU:
		mdelay(80);
		snd_soc_update_bits(codec, SUNXI_DAC_ANA_CTL,
				(0x1 << LINEOUTL_EN),
				(0x1 << LINEOUTL_EN));
		break;

	case	SND_SOC_DAPM_PRE_PMD:
		snd_soc_update_bits(codec, SUNXI_DAC_ANA_CTL,
				(0x1 << LINEOUTL_EN),
				(0x0 << LINEOUTL_EN));
		msleep(100);
		break;
	default:
		break;
	}
	return 0;
}


static int sunxi_codec_playback_event(struct snd_soc_dapm_widget *w,
				struct snd_kcontrol *k, int event)
{
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);

	switch (event) {
	case	SND_SOC_DAPM_POST_PMU:
		snd_soc_update_bits(codec, SUNXI_DAC_DPC,
				(0x1<<EN_DAC), (0x1<<EN_DAC));
		/* for work fine */
		msleep(30);
	/* DACL to left channel LINEOUT Mute control 0:mute 1: not mute */
		snd_soc_update_bits(codec, SUNXI_DAC_ANA_CTL,
				(0x1 << DACLMUTE),
				(0x1 << DACLMUTE));
		break;
	case	SND_SOC_DAPM_POST_PMD:
		snd_soc_update_bits(codec, SUNXI_DAC_DPC,
				(0x1<<EN_DAC), (0x0<<EN_DAC));
	/* DACL to left channel LINEOUT Mute control 0:mute 1: not mute */
		snd_soc_update_bits(codec, SUNXI_DAC_ANA_CTL,
				(0x1 << DACLMUTE),
				(0x0 << DACLMUTE));
		break;
	default:
		break;
	}
	return 0;
}

static int sunxi_codec_capture_event(struct snd_soc_dapm_widget *w,
				struct snd_kcontrol *k, int event)
{
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);

	switch (event) {
	case	SND_SOC_DAPM_POST_PMU:
		/* delay 80ms to avoid the capture pop at the beginning */
		msleep(80);
		snd_soc_update_bits(codec, SUNXI_ADC_FIFOC,
				(0x1<<EN_AD), (0x1<<EN_AD));
		break;
	case	SND_SOC_DAPM_POST_PMD:
		snd_soc_update_bits(codec, SUNXI_ADC_FIFOC,
				(0x1<<EN_AD), (0x0<<EN_AD));
		break;
	default:
		break;
	}
	return 0;
}

static const struct snd_kcontrol_new sunxi_codec_controls[] = {
#ifdef SUNXI_CODEC_HUB_ENABLE
	SOC_ENUM_EXT("codec hub mode", sunxi_codec_hub_mode_enum[0],
				sunxi_codec_get_hub_mode,
				sunxi_codec_set_hub_mode),
#endif
	SOC_SINGLE_TLV("digital volume", SUNXI_DAC_DPC,
					DVOL, 0x3F, 0, digital_tlv),
	SOC_SINGLE_TLV("LINEIN gain volume", SUNXI_ADCL_ANA_CTL,
					LINEINLG, 0x1, 0, linein_tlv),
	SOC_SINGLE_TLV("MIC1 gain volume", SUNXI_ADCL_ANA_CTL,
					PGA_GAIN_CTRL, 0x1F, 0, mic_gain_tlv),
	SOC_SINGLE_TLV("LINEOUT volume", SUNXI_DAC_ANA_CTL,
					LINEOUT_VOL, 0x1F, 0, lineout_tlv),
};

static const struct snd_kcontrol_new left_input_mixer[] = {
	SOC_DAPM_SINGLE("MIC1 Boost Switch", SUNXI_ADCL_ANA_CTL, MIC1AMPEN, 1, 0),
	SOC_DAPM_SINGLE("LINEINL Switch", SUNXI_ADCL_ANA_CTL, LINEINLEN, 1, 0),
};

/*lineout output source*/
static const char * const left_lineout_text[] = {
	"DACL_SINGLE", "DACL_R",
};

static const struct soc_enum left_lineout_enum =
	SOC_ENUM_SINGLE(SUNXI_DAC_ANA_CTL, LINEOUTLDIFFEN,
			ARRAY_SIZE(left_lineout_text), left_lineout_text);

static const struct snd_kcontrol_new left_lineout_mux =
	SOC_DAPM_ENUM("Left LINEOUT Mux", left_lineout_enum);

static const struct snd_soc_dapm_widget sunxi_codec_dapm_widgets[] = {
	SND_SOC_DAPM_AIF_IN_E("DACL", "Playback", 0, SUNXI_DAC_ANA_CTL,
				DACLEN, 0,
				sunxi_codec_playback_event,
				SND_SOC_DAPM_POST_PMU | SND_SOC_DAPM_POST_PMD),
	SND_SOC_DAPM_AIF_OUT_E("ADCL", "Capture", 0, SUNXI_ADCL_ANA_CTL,
				ADCLEN, 0,
				sunxi_codec_capture_event,
				SND_SOC_DAPM_POST_PMU | SND_SOC_DAPM_POST_PMD),

	SND_SOC_DAPM_MIXER("Left Input Mixer", SND_SOC_NOPM, 0, 0,
			left_input_mixer, ARRAY_SIZE(left_input_mixer)),

	SND_SOC_DAPM_MUX("Left LINEOUT Mux", SND_SOC_NOPM,
			0, 0, &left_lineout_mux),

	SND_SOC_DAPM_MICBIAS("MainMic Bias", SUNXI_MICBIAS_ANA_CTL,
				MMICBIASEN, 0),

	SND_SOC_DAPM_INPUT("MIC1"),
	SND_SOC_DAPM_LINE("LINEIN", NULL),
	SND_SOC_DAPM_INPUT("LINEINL"),
	SND_SOC_DAPM_OUTPUT("LINEOUTL"),

	SND_SOC_DAPM_LINE("LINEOUT", sunxi_codec_lineout_event),
	SND_SOC_DAPM_SPK("External Speaker", sunxi_codec_speaker_event),
};

static const struct snd_soc_dapm_route sunxi_codec_dapm_routes[] = {
	/* dapm input route */
	{"MIC1", NULL, "MainMic Bias"},
	{"Left Input Mixer", "LINEINL Switch", "LINEINL"},
	{"Left Input Mixer", "MIC1 Boost Switch", "MIC1"},
	{"ADCL", NULL, "Left Input Mixer"},

	{"MainMic Bias", NULL, "Main Mic"},
	{"LINEINL", NULL, "LINEIN"},

	/* dapm output route */
	{"Left LINEOUT Mux", "DACL_SINGLE", "DACL"},
	{"Left LINEOUT Mux", "DACL_R", "DACL"},

	{"LINEOUT", NULL, "Left LINEOUT Mux"},
};

static void sunxi_codec_init(struct snd_soc_codec *codec)
{
	struct sunxi_codec_info *sunxi_codec = snd_soc_codec_get_drvdata(codec);

	/* Enable ADCFDT to overcome niose at the beginning */
	snd_soc_update_bits(codec, SUNXI_ADC_FIFOC,
			(0x7 << ADCDFEN), (0x7 << ADCDFEN));

	/* init the mic pga and vol params */
	snd_soc_update_bits(codec, SUNXI_DAC_ANA_CTL,
			0x1F << LINEOUT_VOL,
			sunxi_codec->lineout_vol << LINEOUT_VOL);
	snd_soc_update_bits(codec, SUNXI_DAC_DPC,
			0x3F << DVOL,
			sunxi_codec->digital_vol << DVOL);
	snd_soc_update_bits(codec, SUNXI_ADCL_ANA_CTL,
			0x1F << PGA_GAIN_CTRL,
			sunxi_codec->main_gain << PGA_GAIN_CTRL);
	snd_soc_update_bits(codec, SUNXI_ADCL_ANA_CTL,
			0x3 << IOPLINE, 0x1 << IOPLINE);

#ifdef SUNXI_CODEC_DAP_ENABLE
	if (sunxi_codec->hw_config.adcdrc_cfg)
		adcdrc_config(codec);

	if (sunxi_codec->hw_config.adchpf_cfg)
		adchpf_config(codec);

	if (sunxi_codec->hw_config.dacdrc_cfg)
		dacdrc_config(codec);

	if (sunxi_codec->hw_config.dachpf_cfg)
		dachpf_config(codec);
#endif
}

static int sunxi_codec_startup(struct snd_pcm_substream *substream,
				struct snd_soc_dai *dai)
{
	struct snd_soc_codec *codec = dai->codec;
	struct sunxi_codec_info *sunxi_codec = snd_soc_codec_get_drvdata(codec);

	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
#ifdef SUNXI_CODEC_DAP_ENABLE
		if (sunxi_codec->hw_config.dacdrc_cfg)
			dacdrc_enable(codec, 1);
		if (sunxi_codec->hw_config.dachpf_cfg)
			dachpf_enable(codec, 1);
#endif
		snd_soc_dai_set_dma_data(dai, substream,
					&sunxi_codec->playback_dma_param);
	} else {
#ifdef SUNXI_CODEC_DAP_ENABLE
		if (sunxi_codec->hw_config.adcdrc_cfg)
			adcdrc_enable(codec, 1);
		if (sunxi_codec->hw_config.dachpf_cfg)
			adchpf_enable(codec, 1);
#endif
		snd_soc_dai_set_dma_data(dai, substream,
				&sunxi_codec->capture_dma_param);
	}

	return 0;
}

static int sunxi_codec_hw_params(struct snd_pcm_substream *substream,
		struct snd_pcm_hw_params *params, struct snd_soc_dai *dai)
{
	struct snd_soc_codec *codec = dai->codec;
	int i = 0;

	switch (params_format(params)) {
	case	SNDRV_PCM_FORMAT_S16_LE:
		if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
			snd_soc_update_bits(codec, SUNXI_DAC_FIFOC,
				(3 << FIFO_MODE), (3 << FIFO_MODE));
			snd_soc_update_bits(codec, SUNXI_DAC_FIFOC,
				(1 << TX_SAMPLE_BITS), (0 << TX_SAMPLE_BITS));
		} else {
			snd_soc_update_bits(codec, SUNXI_ADC_FIFOC,
				(1 << RX_FIFO_MODE), (1 << RX_FIFO_MODE));
			snd_soc_update_bits(codec, SUNXI_ADC_FIFOC,
				(1 << RX_SAMPLE_BITS), (0 << RX_SAMPLE_BITS));
		}
		break;
	case	SNDRV_PCM_FORMAT_S24_LE:
		if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
			snd_soc_update_bits(codec, SUNXI_DAC_FIFOC,
				(3 << FIFO_MODE), (0 << FIFO_MODE));
			snd_soc_update_bits(codec, SUNXI_DAC_FIFOC,
				(1 << TX_SAMPLE_BITS), (1 << TX_SAMPLE_BITS));
		} else {
			snd_soc_update_bits(codec, SUNXI_ADC_FIFOC,
				(1 << RX_FIFO_MODE), (0 << RX_FIFO_MODE));
			snd_soc_update_bits(codec, SUNXI_ADC_FIFOC,
				(1 << RX_SAMPLE_BITS), (1 << RX_SAMPLE_BITS));
		}
		break;
	default:
		break;
	}

	for (i = 0; i < ARRAY_SIZE(sample_rate_conv); i++) {
		if (sample_rate_conv[i].samplerate == params_rate(params)) {
			if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
				snd_soc_update_bits(codec, SUNXI_DAC_FIFOC,
					(0x7 << DAC_FS),
					(sample_rate_conv[i].rate_bit << DAC_FS));
			} else {
				if (sample_rate_conv[i].samplerate > 48000)
					return -EINVAL;
				snd_soc_update_bits(codec, SUNXI_ADC_FIFOC,
					(0x7 << ADC_FS),
					(sample_rate_conv[i].rate_bit<<ADC_FS));
			}
		}
	}

	switch (params_channels(params)) {
	case 1:
		if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
			snd_soc_update_bits(codec, SUNXI_DAC_FIFOC,
				(1 << DAC_MONO_EN), 1 << DAC_MONO_EN);
		} else {
			snd_soc_update_bits(codec, SUNXI_ADC_FIFOC,
				(1 << ADC_CHAN_SEL), (1 << ADC_CHAN_SEL));
		}
		break;
	case 2:
		if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
			snd_soc_update_bits(codec, SUNXI_DAC_FIFOC,
				(1 << DAC_MONO_EN), (0 << DAC_MONO_EN));
		} else {
			pr_err("[%s] audiocodec can not support 2 channels"
				" for capture.\n", __func__);
			return -EINVAL;
		}
		break;
	default:
		pr_err("[%s] audiocodec only support mono or stereo mode.\n", __func__);
		return -EINVAL;
	}

	return 0;
}

static int sunxi_codec_set_sysclk(struct snd_soc_dai *dai,
			int clk_id, unsigned int freq, int dir)
{
	struct snd_soc_codec *codec = dai->codec;
	struct sunxi_codec_info *sunxi_codec = snd_soc_codec_get_drvdata(codec);

	if (clk_set_rate(sunxi_codec->pll_clk, freq)) {
		dev_err(sunxi_codec->dev, "set pll_clk rate failed\n");
		return -EINVAL;
	}

	return 0;
}

static void sunxi_codec_shutdown(struct snd_pcm_substream *substream,
				struct snd_soc_dai *dai)
{
#ifdef SUNXI_CODEC_DAP_ENABLE
	struct snd_soc_codec *codec = dai->codec;
	struct sunxi_codec_info *sunxi_codec = snd_soc_codec_get_drvdata(codec);

	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
		if (sunxi_codec->hw_config.dacdrc_cfg)
			dacdrc_enable(codec, 0);
		if (sunxi_codec->hw_config.dachpf_cfg)
			dachpf_enable(codec, 0);
	} else {
		if (sunxi_codec->hw_config.adcdrc_cfg)
			adcdrc_enable(codec, 0);

		if (sunxi_codec->hw_config.adchpf_cfg)
			adchpf_enable(codec, 0);
	}
#endif
}

static int sunxi_codec_digital_mute(struct snd_soc_dai *dai, int mute)
{
	return 0;
}

static int sunxi_codec_prepare(struct snd_pcm_substream *substream,
				struct snd_soc_dai *dai)
{
	struct snd_soc_codec *codec = dai->codec;

	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
		snd_soc_update_bits(codec, SUNXI_DAC_FIFOC,
			(1 << FIFO_FLUSH), (1 << FIFO_FLUSH));
		snd_soc_write(codec, SUNXI_DAC_FIFOS,
			(1 << DAC_TXE_INT | 1 << DAC_TXU_INT | 1 << DAC_TXO_INT));
		snd_soc_write(codec, SUNXI_DAC_CNT, 0);
	} else {
		snd_soc_update_bits(codec, SUNXI_ADC_FIFOC,
				(1 << ADC_FIFO_FLUSH), (1 << ADC_FIFO_FLUSH));
		snd_soc_write(codec, SUNXI_ADC_FIFOS,
				(1 << ADC_RXA_INT | 1 << ADC_RXO_INT));
		snd_soc_write(codec, SUNXI_ADC_CNT, 0);
	}
	return 0;
}

static int sunxi_codec_trigger(struct snd_pcm_substream *substream,
				int cmd, struct snd_soc_dai *dai)
{
	struct sunxi_codec_info *sunxi_codec = snd_soc_codec_get_drvdata(dai->codec);

	switch (cmd) {
	case SNDRV_PCM_TRIGGER_START:
	case SNDRV_PCM_TRIGGER_RESUME:
	case SNDRV_PCM_TRIGGER_PAUSE_RELEASE:
		if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
			regmap_update_bits(sunxi_codec->regmap, SUNXI_DAC_FIFOC,
				(1 << DAC_DRQ_EN), (1 << DAC_DRQ_EN));
		else
			regmap_update_bits(sunxi_codec->regmap, SUNXI_ADC_FIFOC,
				(1 << ADC_DRQ_EN), (1 << ADC_DRQ_EN));
		break;
	case SNDRV_PCM_TRIGGER_STOP:
	case SNDRV_PCM_TRIGGER_SUSPEND:
	case SNDRV_PCM_TRIGGER_PAUSE_PUSH:
		if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
			regmap_update_bits(sunxi_codec->regmap, SUNXI_DAC_FIFOC,
				(1 << DAC_DRQ_EN), (0 << DAC_DRQ_EN));
		else
			regmap_update_bits(sunxi_codec->regmap, SUNXI_ADC_FIFOC,
				(1 << ADC_DRQ_EN), (0 << ADC_DRQ_EN));
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

static const struct snd_soc_dai_ops sunxi_codec_dai_ops = {
	.startup	= sunxi_codec_startup,
	.hw_params	= sunxi_codec_hw_params,
	.shutdown	= sunxi_codec_shutdown,
	.digital_mute	= sunxi_codec_digital_mute,
	.set_sysclk	= sunxi_codec_set_sysclk,
	.trigger	= sunxi_codec_trigger,
	.prepare	= sunxi_codec_prepare,
};

static struct snd_soc_dai_driver sunxi_codec_dai[] = {
	{
		.name	= "sun8iw19codec",
		.playback = {
			.stream_name = "Playback",
			.channels_min = 1,
			.channels_max = 2,
			.rates	= SNDRV_PCM_RATE_8000_192000
				| SNDRV_PCM_RATE_KNOT,
			.formats = SNDRV_PCM_FMTBIT_S16_LE
				| SNDRV_PCM_FMTBIT_S24_LE,
		},
		.capture = {
			.stream_name = "Capture",
			.channels_min = 1,
			.channels_max = 1,
			.rates = SNDRV_PCM_RATE_8000_48000
				| SNDRV_PCM_RATE_KNOT,
			.formats = SNDRV_PCM_FMTBIT_S16_LE
				| SNDRV_PCM_FMTBIT_S24_LE,
		},
		.ops = &sunxi_codec_dai_ops,
	},
};

static int sunxi_codec_probe(struct snd_soc_codec *codec)
{
	int ret;
	struct snd_soc_dapm_context *dapm = &codec->component.dapm;

	ret = snd_soc_add_codec_controls(codec, sunxi_codec_controls,
					ARRAY_SIZE(sunxi_codec_controls));
	if (ret)
		pr_err("failed to register codec controls!\n");

	snd_soc_dapm_new_controls(dapm, sunxi_codec_dapm_widgets,
				ARRAY_SIZE(sunxi_codec_dapm_widgets));
	snd_soc_dapm_add_routes(dapm, sunxi_codec_dapm_routes,
				ARRAY_SIZE(sunxi_codec_dapm_routes));

	sunxi_codec_init(codec);

	return 0;
}

static int sunxi_codec_remove(struct snd_soc_codec *codec)
{
	return 0;
}

static int sunxi_gpio_iodisable(u32 gpio)
{
	char pin_name[8];
	u32 config;
	u32 ret = 0;

	sunxi_gpio_to_name(gpio, pin_name);
	config = 7 << 16;
	ret = pin_config_set(SUNXI_PINCTRL, pin_name, config);

	return ret;
}

static int sunxi_codec_suspend(struct snd_soc_codec *codec)
{
	struct sunxi_codec_info *sunxi_codec = snd_soc_codec_get_drvdata(codec);
	struct codec_spk_config *spk_cfg = &(sunxi_codec->spk_config);

	pr_debug("Enter %s\n", __func__);

	if (spk_cfg->used)
		sunxi_gpio_iodisable(spk_cfg->spk_gpio);

	clk_disable_unprepare(sunxi_codec->module_clk);

	clk_disable_unprepare(sunxi_codec->pll_clk);

	pr_debug("End %s\n", __func__);

	return 0;
}

static int sunxi_codec_resume(struct snd_soc_codec *codec)
{
	struct sunxi_codec_info *sunxi_codec = snd_soc_codec_get_drvdata(codec);
	struct codec_spk_config *spk_cfg = &(sunxi_codec->spk_config);

	pr_debug("Enter %s\n", __func__);

	if (clk_prepare_enable(sunxi_codec->pll_clk)) {
		dev_err(sunxi_codec->dev, "enable pll_clk failed, resume exit\n");
		return -EBUSY;
	}

	if (clk_prepare_enable(sunxi_codec->module_clk)) {
		dev_err(sunxi_codec->dev, "enable  module_clk failed, resume exit\n");
		clk_disable_unprepare(sunxi_codec->pll_clk);
		return -EBUSY;
	}

	if (spk_cfg->used) {
		gpio_direction_output(spk_cfg->spk_gpio, 1);
		gpio_set_value(spk_cfg->spk_gpio, !(spk_cfg->pa_level));
	}

	sunxi_codec_init(codec);

	pr_debug("End %s\n", __func__);

	return 0;
}

static unsigned int sunxi_codec_read(struct snd_soc_codec *codec,
					unsigned int reg)
{
	struct sunxi_codec_info *sunxi_codec = snd_soc_codec_get_drvdata(codec);
	unsigned int reg_val;

	regmap_read(sunxi_codec->regmap, reg, &reg_val);

	return reg_val;
}

static int sunxi_codec_write(struct snd_soc_codec *codec,
				unsigned int reg, unsigned int val)
{
	struct sunxi_codec_info *sunxi_codec = snd_soc_codec_get_drvdata(codec);

	regmap_write(sunxi_codec->regmap, reg, val);

	return 0;
};

static struct snd_soc_codec_driver soc_codec_dev_sunxi = {
	.probe = sunxi_codec_probe,
	.remove = sunxi_codec_remove,
	.suspend = sunxi_codec_suspend,
	.resume = sunxi_codec_resume,
	.read = sunxi_codec_read,
	.write = sunxi_codec_write,
	.ignore_pmdown_time = 1,
};

static struct snd_soc_dai_driver sunxi_cpudai_dai = {
	.name	= "sunxi-internal-cpudai",
	.playback = {
		.channels_min = 1,
		.channels_max = 2,
		.rates = SNDRV_PCM_RATE_8000_192000 |
			SNDRV_PCM_RATE_KNOT,
		.formats = SNDRV_PCM_FMTBIT_S16_LE |
			SNDRV_PCM_FMTBIT_S24_LE	|
			SNDRV_PCM_FMTBIT_S32_LE,
	},
	.capture = {
		.channels_min = 1,
		.channels_max = 1,
		.rates = SNDRV_PCM_RATE_8000_48000 |
			SNDRV_PCM_RATE_KNOT,
		.formats = SNDRV_PCM_FMTBIT_S16_LE |
			SNDRV_PCM_FMTBIT_S24_LE	|
			SNDRV_PCM_FMTBIT_S32_LE,
	},
	.ops	= NULL,
};

static const struct snd_soc_component_driver sunxi_asoc_cpudai_component = {
	.name = "sun8iw19-internal-cpudai",
};

static struct reg_label reg_labels[] = {
	REG_LABEL(SUNXI_DAC_DPC),
	REG_LABEL(SUNXI_DAC_FIFOC),
	REG_LABEL(SUNXI_DAC_FIFOS),
	REG_LABEL(SUNXI_DAC_CNT),
	REG_LABEL(SUNXI_DAC_DG),
	REG_LABEL(SUNXI_ADC_FIFOC),
	REG_LABEL(SUNXI_ADC_FIFOS),
	REG_LABEL(SUNXI_ADC_CNT),
	REG_LABEL(SUNXI_ADC_DG),
#ifdef SUNXI_CODEC_DAP_ENABLE
	REG_LABEL(SUNXI_DAC_DAP_CTL),
	REG_LABEL(SUNXI_ADC_DAP_CTL),
#endif
	REG_LABEL_END,
};

static ssize_t show_audio_reg(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct sunxi_codec_info *sunxi_codec = dev_get_drvdata(dev);
	int count = 0, i = 0;
	unsigned int reg_val;
	unsigned int size = ARRAY_SIZE(reg_labels);

	count += sprintf(buf, "dump audiocodec reg:\n");

	while ((i < size) && (reg_labels[i].name != NULL)) {
		regmap_read(sunxi_codec->regmap,
				reg_labels[i].address, &reg_val);
		count += sprintf(buf + count, "[%s] 0x%03x: 0x%08x save_val:0x%08x\n",
			reg_labels[i].name, (reg_labels[i].address),
			reg_val, reg_labels[i].value);
		i++;
	}

	return count;
}

/* ex:
 * param 1: 0 read;1 write
 * param 2: reg value;
 * param 3: write value;
	read:
		echo 0,0x00> audio_reg
	write:
		echo 1,0x00,0xa > audio_reg
*/
static ssize_t store_audio_reg(struct device *dev, struct device_attribute *attr,
				const char *buf, size_t count)
{
	int ret;
	int rw_flag;
	int input_reg_val = 0;
	int input_reg_offset = 0;
	struct sunxi_codec_info *sunxi_codec = dev_get_drvdata(dev);

	ret = sscanf(buf, "%d,0x%x,0x%x", &rw_flag,
			&input_reg_offset, &input_reg_val);
	dev_info(dev, "ret:%d, reg_offset:%d, reg_val:0x%x\n",
			ret, input_reg_offset, input_reg_val);

	if (!(rw_flag == 1 || rw_flag == 0)) {
		pr_err("not rw_flag\n");
		ret = count;
		goto out;
	}

	if (input_reg_offset > SUNXI_BIAS_ANA_CTL) {
		pr_err("ERROR: the reg offset[0x%03x] > SUNXI_BIAS_ANA_CTL[0x%03x]\n",
			input_reg_offset, SUNXI_BIAS_ANA_CTL);
		ret = count;
		goto out;
	}

	if (rw_flag) {
		regmap_write(sunxi_codec->regmap,
				input_reg_offset, input_reg_val);
	} else {
		regmap_read(sunxi_codec->regmap,
				input_reg_offset, &input_reg_val);
		dev_info(dev, "\n\n Reg[0x%x] : 0x%08x\n\n",
				input_reg_offset, input_reg_val);
	}
	ret = count;
out:
	return ret;
}

static DEVICE_ATTR(audio_reg, 0644, show_audio_reg, store_audio_reg);

static struct attribute *audio_debug_attrs[] = {
	&dev_attr_audio_reg.attr,
	NULL,
};

static struct attribute_group audio_debug_attr_group = {
	.name   = "audio_reg_debug",
	.attrs  = audio_debug_attrs,
};

static const struct regmap_config sunxi_codec_regmap_config = {
	.reg_bits = 32,
	.reg_stride = 4,
	.val_bits = 32,
	.max_register = SUNXI_BIAS_ANA_CTL,
	.cache_type = REGCACHE_NONE,
};

static int sunxi_internal_codec_probe(struct platform_device *pdev)
{
	struct sunxi_codec_info *sunxi_codec;
	struct resource res;
	struct resource *memregion;
	struct device_node *np = pdev->dev.of_node;
	int ret;
	unsigned int temp_val;

	sunxi_codec = devm_kzalloc(&pdev->dev,
				sizeof(struct sunxi_codec_info), GFP_KERNEL);
	if (!sunxi_codec) {
		dev_err(&pdev->dev, "Can't allocate sunxi codec memory\n");
		ret = -ENOMEM;
		goto err_node_put;
	}
	dev_set_drvdata(&pdev->dev, sunxi_codec);
	sunxi_codec->dev = &pdev->dev;

	sunxi_codec->pll_clk = of_clk_get(np, 0);
	if (IS_ERR_OR_NULL(sunxi_codec->pll_clk)) {
		dev_err(&pdev->dev, "pll_clk not exist or invaild\n");
		ret = PTR_ERR(sunxi_codec->pll_clk);
		goto err_devm_kfree;
	}

	sunxi_codec->module_clk = of_clk_get(np, 1);
	if (IS_ERR_OR_NULL(sunxi_codec->module_clk)) {
		dev_err(&pdev->dev, "module_clk not exist or invaild\n");
		ret = PTR_ERR(sunxi_codec->module_clk);
		goto err_pllclk_put;
	} else {
		if (clk_set_parent(sunxi_codec->module_clk,
				sunxi_codec->pll_clk)) {
			dev_err(&pdev->dev, "set parent of module_clk to pll_clk failed\n");
			ret = -EBUSY;
			goto err_moduleclk_put;
		}
		if (clk_prepare_enable(sunxi_codec->pll_clk)) {
			dev_err(&pdev->dev, "pll_clk enable failed\n");
			ret = -EBUSY;
			goto err_moduleclk_put;
		}
		if (clk_prepare_enable(sunxi_codec->module_clk)) {
			dev_err(&pdev->dev, "module_clk enable failed\n");
			ret = -EBUSY;
			goto err_pllclk_disable;
		}
	}

	/* codec reg_base */
	ret = of_address_to_resource(np, 0, &res);
	if (ret) {
		dev_err(&pdev->dev, "Failed to get sunxi resource\n");
		return -EINVAL;
		goto err_moduleclk_disable;
	}

	memregion = devm_request_mem_region(&pdev->dev, res.start,
				resource_size(&res), "sunxi-internal-codec");
	if (!memregion) {
		dev_err(&pdev->dev, "sunxi memory region already claimed\n");
		ret = -EBUSY;
		goto err_devm_free_memregion;
	}

	sunxi_codec->digital_base = devm_ioremap(&pdev->dev,
					res.start, resource_size(&res));
	if (!sunxi_codec->digital_base) {
		dev_err(&pdev->dev, "sunxi digital_base ioremap failed\n");
		ret = -EBUSY;
		goto err_moduleclk_disable;
	}

	sunxi_codec->regmap = devm_regmap_init_mmio(&pdev->dev,
				sunxi_codec->digital_base,
				&sunxi_codec_regmap_config);
	if (IS_ERR_OR_NULL(sunxi_codec->regmap)) {
		dev_err(&pdev->dev, "regmap init failed\n");
		ret = PTR_ERR(sunxi_codec->regmap);
		goto err_digital_iounmap;
	}

	ret = of_property_read_u32(np, "digital_vol", &temp_val);
	if (ret < 0) {
		dev_warn(&pdev->dev, "digital volume get failed\n");
		sunxi_codec->digital_vol = 0;
	} else {
		sunxi_codec->digital_vol = temp_val;
	}

	ret = of_property_read_u32(np, "lineout_vol", &temp_val);
	if (ret < 0) {
		dev_warn(&pdev->dev, "lineout volume get failed\n");
		sunxi_codec->lineout_vol = 0x1F;
	} else {
		sunxi_codec->lineout_vol = temp_val;
	}

	ret = of_property_read_u32(np, "main_gain", &temp_val);
	if (ret < 0) {
		dev_warn(&pdev->dev, "main gain get failed\n");
		sunxi_codec->main_gain = 32;
	} else {
		sunxi_codec->main_gain = temp_val;
	}

	ret = of_property_read_u32(np, "pa_msleep_time", &temp_val);
	if (ret < 0) {
		dev_warn(&pdev->dev, "pa_msleep get failed\n");
		sunxi_codec->spk_config.pa_msleep = 160;
	} else {
		sunxi_codec->spk_config.pa_msleep = temp_val;
	}

	ret = of_property_read_u32(np, "pa_level", &temp_val);
	if (ret < 0) {
		dev_warn(&pdev->dev, "pa_level get failed.\n");
		sunxi_codec->spk_config.pa_level = 1;
	} else {
		sunxi_codec->spk_config.pa_level = temp_val;
	}

	pr_info("digital_vol:%d, lineout_vol:%d, main_gain:%d,"
		" pa_msleep:%d, pa_level:%d\n",
		sunxi_codec->digital_vol,
		sunxi_codec->lineout_vol,
		sunxi_codec->main_gain,
		sunxi_codec->spk_config.pa_msleep,
		sunxi_codec->spk_config.pa_level);

#ifdef SUNXI_CODEC_DAP_ENABLE
	ret = of_property_read_u32(np, "adcdrc_cfg", &temp_val);
	if (ret < 0) {
		pr_err("[%s] adcdrc_cfg configs missing or invalid.\n", __func__);
		ret = -EINVAL;
	} else {
		sunxi_codec->hw_config.adcdrc_cfg = temp_val;
	}

	ret = of_property_read_u32(np, "adchpf_cfg", &temp_val);
	if (ret < 0) {
		pr_err("[%s] adchpf_cfg configs missing or invalid.\n", __func__);
		ret = -EINVAL;
	} else {
		sunxi_codec->hw_config.adchpf_cfg = temp_val;
	}

	ret = of_property_read_u32(np, "dacdrc_cfg", &temp_val);
	if (ret < 0) {
		pr_err("[%s] dacdrc_cfg configs missing or invalid.\n", __func__);
		ret = -EINVAL;
	} else {
		sunxi_codec->hw_config.dacdrc_cfg = temp_val;
	}

	ret = of_property_read_u32(np, "dachpf_cfg", &temp_val);
	if (ret < 0) {
		pr_err("[%s] dachpf_cfg configs missing or invalid.\n", __func__);
		ret = -EINVAL;
	} else {
		sunxi_codec->hw_config.dachpf_cfg = temp_val;
	}

	pr_info("adcdrc_cfg:%d, adchpf_cfg:%d, dacdrc_cfg:%d, dachpf:%d\n",
		sunxi_codec->hw_config.adcdrc_cfg,
		sunxi_codec->hw_config.adchpf_cfg,
		sunxi_codec->hw_config.dacdrc_cfg,
		sunxi_codec->hw_config.dachpf_cfg);
#endif
	ret = of_get_named_gpio(np, "gpio-spk", 0);
	if (ret >= 0) {
		sunxi_codec->spk_config.used = 1;
		sunxi_codec->spk_config.spk_gpio = ret;
		if (!gpio_is_valid(sunxi_codec->spk_config.spk_gpio)) {
			dev_err(&pdev->dev, "gpio-spk is invalid\n");
			ret = -EINVAL;
			goto err_digital_iounmap;
		} else {
			pr_info("gpio-spk:%d\n",
					sunxi_codec->spk_config.spk_gpio);
			ret = devm_gpio_request(&pdev->dev,
					sunxi_codec->spk_config.spk_gpio, "SPK");
			if (ret) {
				dev_err(&pdev->dev, "failed to request gpio-spk gpio\n");
				ret = -EBUSY;
				goto err_digital_iounmap;
			} else {
				gpio_direction_output(sunxi_codec->spk_config.spk_gpio, 1);
				gpio_set_value(sunxi_codec->spk_config.spk_gpio,
					!(sunxi_codec->spk_config.pa_level));
			}
		}
	} else {
		sunxi_codec->spk_config.used = 0;
	}

	ret = snd_soc_register_codec(&pdev->dev, &soc_codec_dev_sunxi,
				sunxi_codec_dai, ARRAY_SIZE(sunxi_codec_dai));
	if (ret < 0) {
		dev_err(&pdev->dev, "register codec failed\n");
		goto err_digital_iounmap;
	}

	ret = of_address_to_resource(np, 0, &res);
	if (ret) {
		dev_err(&pdev->dev, "Can't parse device node resource\n");
		ret = -ENODEV;
		goto err_unregister_codec;
	}

	sunxi_codec->playback_dma_param.dma_addr = res.start+SUNXI_DAC_TXDATA;
	sunxi_codec->playback_dma_param.dma_drq_type_num = DRQDST_AUDIO_CODEC;
	sunxi_codec->playback_dma_param.dst_maxburst = 4;
	sunxi_codec->playback_dma_param.src_maxburst = 4;

	sunxi_codec->capture_dma_param.dma_addr = res.start+SUNXI_ADC_RXDATA;
	sunxi_codec->capture_dma_param.dma_drq_type_num = DRQSRC_AUDIO_CODEC;
	sunxi_codec->capture_dma_param.src_maxburst = 4;
	sunxi_codec->capture_dma_param.dst_maxburst = 4;

	ret = snd_soc_register_component(&pdev->dev, &sunxi_asoc_cpudai_component,
					&sunxi_cpudai_dai, 1);
	if (ret) {
		dev_err(&pdev->dev, "Could not register DAI: %d\n", ret);
		ret = -EBUSY;
		goto err_unregister_codec;
	}

	ret = asoc_dma_platform_register(&pdev->dev, 0);
	if (ret) {
		dev_err(&pdev->dev, "Could not register PCM: %d\n", ret);
		goto err_unregister_component;
	}
	ret  = sysfs_create_group(&pdev->dev.kobj, &audio_debug_attr_group);
	if (ret) {
		dev_warn(&pdev->dev, "failed to create attr group\n");
		goto err_unregister_platform;
	}

	dev_warn(&pdev->dev, "[%s] codec probe finished.\n", __func__);

	return 0;

err_unregister_platform:
	asoc_dma_platform_unregister(&pdev->dev);
err_unregister_component:
	snd_soc_unregister_component(&pdev->dev);
err_unregister_codec:
	snd_soc_unregister_codec(&pdev->dev);
err_digital_iounmap:
	devm_iounmap(&pdev->dev, sunxi_codec->digital_base);
err_moduleclk_disable:
	clk_disable_unprepare(sunxi_codec->module_clk);
err_moduleclk_put:
	clk_put(sunxi_codec->module_clk);
err_pllclk_disable:
	clk_disable_unprepare(sunxi_codec->pll_clk);
err_pllclk_put:
	clk_put(sunxi_codec->pll_clk);
err_devm_kfree:
	devm_kfree(&pdev->dev, sunxi_codec);
err_node_put:
	of_node_put(np);
err_devm_free_memregion:
	devm_release_mem_region(&pdev->dev, memregion->start,
						resource_size(memregion));
	return ret;
}

static int  __exit sunxi_internal_codec_remove(struct platform_device *pdev)
{
	struct sunxi_codec_info *sunxi_codec = dev_get_drvdata(&pdev->dev);

	sysfs_remove_group(&pdev->dev.kobj, &audio_debug_attr_group);
	asoc_dma_platform_unregister(&pdev->dev);
	snd_soc_unregister_component(&pdev->dev);
	snd_soc_unregister_codec(&pdev->dev);
	clk_disable_unprepare(sunxi_codec->module_clk);
	clk_put(sunxi_codec->module_clk);
	clk_disable_unprepare(sunxi_codec->pll_clk);
	clk_put(sunxi_codec->pll_clk);
	devm_iounmap(&pdev->dev, sunxi_codec->digital_base);
	devm_kfree(&pdev->dev, sunxi_codec);
	platform_set_drvdata(pdev, NULL);

	dev_warn(&pdev->dev, "[%s] codec remove finished.\n", __func__);

	return 0;
}

static const struct of_device_id sunxi_internal_codec_of_match[] = {
	{ .compatible = "allwinner,sunxi-internal-codec", },
};

static struct platform_driver sunxi_internal_codec_driver = {
	.driver = {
		.name = "sunxi-internal-codec",
		.owner = THIS_MODULE,
		.of_match_table = sunxi_internal_codec_of_match,
	},
	.probe = sunxi_internal_codec_probe,
	.remove = __exit_p(sunxi_internal_codec_remove),
};
module_platform_driver(sunxi_internal_codec_driver);

MODULE_DESCRIPTION("SUNXI Codec ASoC driver");
MODULE_AUTHOR("yumingfeng <yumingfeng@allwinnertech.com>");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:sunxi-internal-codec");
