/*
 * drivers/input/sensor/sunxi_gpadc.h
 *
 * Copyright (C) 2016 Allwinner.
 * fuzhaoke <fuzhaoke@allwinnertech.com>
 *
 * SUNXI GPADC Controller Driver Header
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 */

#ifndef SUNXI_GPADC_H
#define SUNXI_GPADC_H

#define GPADC_DEV_NAME		("sunxi-gpadc")

#define OSC_24MHZ		(24000000UL)
#define MAX_SR                  (100000UL)
#define MIN_SR                  (400UL)
#define DEFAULT_SR		(1000UL)
/* voltage range 0~2.3v, unit is uv */
#define VOL_RANGE		(1800000UL)
#define VOL_VALUE_MASK		(0Xfff)


/* GPADC register offset */
#define GP_SR_REG		(0x00) /* Sample Rate config register */
#define GP_CTRL_REG		(0x04) /* control register */
#define GP_CS_EN_REG		(0x08) /* compare and select enable register */
#define GP_FIFO_INTC_REG	(0x0c) /* FIFO interrupt config register */
#define GP_FIFO_INTS_REG	(0x10) /* FIFO interrupt status register */
#define GP_FIFO_DATA_REG	(0X14) /* FIFO data register */
#define GP_CB_DATA_REG		(0X18) /* calibration data register */
#define GP_DATAL_INTC_REG	(0x20)
#define GP_DATAH_INTC_REG	(0x24)
#define GP_DATA_INTC_REG	(0x28)
#define GP_DATAL_INTS_REG	(0x30)
#define GP_DATAH_INTS_REG	(0x34)
#define GP_DATA_INTS_REG	(0x38)
#define GP_CH0_CMP_DATA_REG	(0x40) /* channal 0 compare data register */
#define GP_CH1_CMP_DATA_REG	(0x44) /* channal 1 compare data register */
#define GP_CH2_CMP_DATA_REG	(0x48) /* channal 2 compare data register */
#define GP_CH3_CMP_DATA_REG	(0x4c) /* channal 3 compare data register */
#define GP_CH4_CMP_DATA_REG	(0x50) /* channal 4 compare data register */
#define GP_CH5_CMP_DATA_REG	(0x54) /* channal 5 compare data register */
#define GP_CH6_CMP_DATA_REG	(0x58) /* channal 6 compare data register */
#define GP_CH7_CMP_DATA_REG	(0x5c) /* channal 7 compare data register */
#define GP_CH0_DATA_REG		(0x80) /* channal 0 data register */
#define GP_CH1_DATA_REG		(0x84) /* channal 1 data register */
#define GP_CH2_DATA_REG		(0x88) /* channal 2 data register */
#define GP_CH3_DATA_REG		(0x8c) /* channal 3 data register */
#define GP_CH4_DATA_REG		(0x90) /* channal 4 data register */
#define GP_CH5_DATA_REG		(0x94) /* channal 5 data register */
#define GP_CH6_DATA_REG		(0x98) /* channal 6 data register */
#define GP_CH7_DATA_REG		(0x9c) /* channal 7 data register */

#define LDOA_EFUSE_REG           0x03006224

/*
 * GP_SR_REG default value: 0x01df_002f 50KHZ
 * sample_rate = clk_in/(n+1) = 24MHZ/(0x1df + 1) = 50KHZ
 */
#define GP_SR_CON		(0xffff << 16)

/* GP_CTRL_REG default value:0x0000_0000 */
#define GP_FIRST_CONCERT_DLY	(0xff<<24) /* delay time of the first time */
#define GP_CALI_EN		(1 << 17) /* enable calibration */
#define GP_ADC_EN		(1 << 16) /* GPADC function enable */

/*
 * 00:single conversion mode
 * 01:single-cycle conversion mode
 * 10:continuous mode, 11:burst mode
 */
#define GP_MODE_SELECT		(3 << 8)

/* 0:disable, 1:enable */
#define GP_CH7_CMP_EN		(1 << 23)
#define GP_CH6_CMP_EN		(1 << 22)
#define GP_CH5_CMP_EN		(1 << 21)
#define GP_CH4_CMP_EN		(1 << 20)
#define GP_CH3_CMP_EN		(1 << 19)
#define GP_CH2_CMP_EN		(1 << 18)
#define GP_CH1_CMP_EN		(1 << 17)
#define GP_CH0_CMP_EN		(1 << 16)
#define GP_CH7_SELECT		(1 << 7)
#define GP_CH6_SELECT		(1 << 6)
#define GP_CH5_SELECT		(1 << 5)
#define GP_CH4_SELECT		(1 << 4)
#define GP_CH3_SELECT		(1 << 3)
#define GP_CH2_SELECT		(1 << 2)
#define GP_CH1_SELECT		(1 << 1)
#define GP_CH0_SELECT		(1 << 0)

/*
 * GP_FIFO_INTC_REG default value: 0x0000_0f00
 * 0:disable, 1:enable
 */
#define FIFO_OVER_IRQ_EN	(1 << 17) /* fifo over run irq enable */
#define FIFO_DATA_IRQ_EN	(1 << 16) /* fifo data irq enable */

/* write 1 to flush TX FIFO, self clear to 0 */
#define FIFO_FLUSH		(1 << 4)

/*
 * GP_FIFO_INTS_REG default value: 0x0000_0000
 * 0:no pending irq, 1: over pending, need write 1 to clear flag
 */
#define FIFO_OVER_PEND		(1 << 17) /* fifo over pending flag */
#define FIFO_DATA_PEND		(1 << 16) /* fifo data pending flag */
#define FIFO_CNT		(0x3f << 8) /* the data count in fifo */

/* GP_FIFO_DATA_REG default value: 0x0000_0000 */
#define GP_FIFO_DATA		(0xfff << 0) /* GPADC data in fifo */

/* GP_CB_DATA_REG default value: 0x0000_0000 */
#define GP_CB_DATA		(0xfff << 0) /* GPADC calibration data */

/* GP_INTC_REG default value: 0x0000_0000 */
#define GP_CH7_LOW_IRQ_EN	(1 << 7) /* 0:disable, 1:enable */
#define GP_CH6_LOW_IRQ_EN	(1 << 6)
#define GP_CH5_LOW_IRQ_EN	(1 << 5)
#define GP_CH4_LOW_IRQ_EN	(1 << 4)
#define GP_CH3_LOW_IRQ_EN	(1 << 3)
#define GP_CH2_LOW_IRQ_EN	(1 << 2)
#define GP_CH1_LOW_IRQ_EN	(1 << 1)
#define GP_CH0_LOW_IRQ_EN	(1 << 0)
#define GP_CH7_HIG_IRQ_EN	(1 << 7)
#define GP_CH6_HIG_IRQ_EN	(1 << 6)
#define GP_CH5_HIG_IRQ_EN	(1 << 5)
#define GP_CH4_HIG_IRQ_EN	(1 << 4)
#define GP_CH3_HIG_IRQ_EN	(1 << 3)
#define GP_CH2_HIG_IRQ_EN	(1 << 2)
#define GP_CH1_HIG_IRQ_EN	(1 << 1)
#define GP_CH0_HIG_IRQ_EN	(1 << 0)
#define GP_CH7_DATA_IRQ_EN	(1 << 7)
#define GP_CH6_DATA_IRQ_EN	(1 << 6)
#define GP_CH5_DATA_IRQ_EN	(1 << 5)
#define GP_CH4_DATA_IRQ_EN	(1 << 4)
#define GP_CH3_DATA_IRQ_EN	(1 << 3)
#define GP_CH2_DATA_IRQ_EN	(1 << 2)
#define GP_CH1_DATA_IRQ_EN	(1 << 1)
#define GP_CH0_DATA_IRQ_EN	(1 << 0)

/* GP_INTS_REG default value: 0x0000_0000 */
#define GP_CH7_LOW		(1 << 7) /* 0:no pending, 1:pending */
#define GP_CH6_LOW		(1 << 6)
#define GP_CH5_LOW		(1 << 5)
#define GP_CH4_LOW		(1 << 4)
#define GP_CH3_LOW		(1 << 3)
#define GP_CH2_LOW		(1 << 2)
#define GP_CH1_LOW		(1 << 1)
#define GP_CH0_LOW		(1 << 0)
#define GP_CH7_HIG		(1 << 7)
#define GP_CH6_HIG		(1 << 6)
#define GP_CH5_HIG		(1 << 5)
#define GP_CH4_HIG		(1 << 4)
#define GP_CH3_HIG		(1 << 3)
#define GP_CH2_HIG		(1 << 2)
#define GP_CH1_HIG		(1 << 1)
#define GP_CH0_HIG		(1 << 0)
#define GP_CH7_DATA		(1 << 7)
#define GP_CH6_DATA		(1 << 6)
#define GP_CH5_DATA		(1 << 5)
#define GP_CH4_DATA		(1 << 4)
#define GP_CH3_DATA		(1 << 3)
#define GP_CH2_DATA		(1 << 2)
#define GP_CH1_DATA		(1 << 1)
#define GP_CH0_DATA		(1 << 0)

/* GP_CH0_CMP_DATA_REG default value 0x0bff_0400 */
#define GP_CH0_CMP_HIG_DATA		(0xfff << 16)
#define GP_CH0_CMP_LOW_DATA		(0xfff << 0)
/* GP_CH1_CMP_DATA_REG default value 0x0bff_0400 */
#define GP_CH1_CMP_HIG_DATA		(0xfff << 16)
#define GP_CH1_CMP_LOW_DATA		(0xfff << 0)
/* GP_CH2_CMP_DATA_REG default value 0x0bff_0400 */
#define GP_CH2_CMP_HIG_DATA		(0xfff << 16)
#define GP_CH2_CMP_LOW_DATA		(0xfff << 0)
/* GP_CH3_CMP_DATA_REG default value 0x0bff_0400 */
#define GP_CH3_CMP_HIG_DATA		(0xfff << 16)
#define GP_CH3_CMP_LOW_DATA		(0xfff << 0)
/* GP_CH4_CMP_DATA_REG default value 0x0bff_0400 */
#define GP_CH4_CMP_HIG_DATA		(0xfff << 16)
#define GP_CH4_CMP_LOW_DATA		(0xfff << 0)
/* GP_CH5_CMP_DATA_REG default value 0x0bff_0400 */
#define GP_CH5_CMP_HIG_DATA		(0xfff << 16)
#define GP_CH5_CMP_LOW_DATA		(0xfff << 0)
/* GP_CH6_CMP_DATA_REG default value 0x0bff_0400 */
#define GP_CH6_CMP_HIG_DATA		(0xfff << 16)
#define GP_CH6_CMP_LOW_DATA		(0xfff << 0)
/* GP_CH7_CMP_DATA_REG default value 0x0bff_0400 */
#define GP_CH7_CMP_HIG_DATA		(0xfff << 16)
#define GP_CH7_CMP_LOW_DATA		(0xfff << 0)

/* GP_CH0_DATA_REG default value:0x0000_0000 */
#define GP_CH0_DATA_MASK		(0xfff << 0) /* channel 0 data mask */
/* GP_CH1_DATA_REG default value:0x0000_0000 */
#define GP_CH1_DATA_MASK		(0xfff << 0) /* channel 1 data mask */
/* GP_CH2_DATA_REG default value:0x0000_0000 */
#define GP_CH2_DATA_MASK		(0xfff << 0) /* channel 2 data mask */
/* GP_CH3_DATA_REG default value:0x0000_0000 */
#define GP_CH3_DATA_MASK		(0xfff << 0) /* channel 3 data mask */
/* GP_CH4_DATA_REG default value:0x0000_0000 */
#define GP_CH4_DATA_MASK		(0xfff << 0) /* channel 4 data mask */
/* GP_CH5_DATA_REG default value:0x0000_0000 */
#define GP_CH5_DATA_MASK		(0xfff << 0) /* channel 5 data mask */
/* GP_CH6_DATA_REG default value:0x0000_0000 */
#define GP_CH6_DATA_MASK		(0xfff << 0) /* channel 6 data mask */
/* GP_CH7_DATA_REG default value:0x0000_0000 */
#define GP_CH7_DATA_MASK		(0xfff << 0) /* channel 7 data mask */
#define GP_CALIBRATION_ENABLE		(0x1 << 17)
#define CHANNEL_0_SELECT		0x01
#define CHANNEL_1_SELECT		0x02
#define CHANNEL_2_SELECT		0x04
#define CHANNEL_3_SELECT		0x08
#define CHANNEL_4_SELECT		0x10
#define CHANNEL_5_SELECT		0x20
#define CHANNEL_6_SELECT		0x40
#define CHANNEL_7_SELECT		0x80

#define CHANNEL_MAX_NUM			8
#define KEY_MAX_CNT			(13)
#define VOL_NUM				KEY_MAX_CNT
#define MAXIMUM_INPUT_VOLTAGE		1800
#define DEVIATION			100
#define SUNXIKEY_DOWN			(MAXIMUM_INPUT_VOLTAGE-DEVIATION)
#define SUNXIKEY_UP			SUNXIKEY_DOWN
#define MAXIMUM_SCALE			128
#define SCALE_UNIT			(MAXIMUM_INPUT_VOLTAGE/MAXIMUM_SCALE)
#define SAMPLING_FREQUENCY		10

enum {
	DEBUG_INFO = 1U << 0,
	DEBUG_RUN  = 1U << 1,
};

enum gp_select_mode {
	GP_SINGLE_MODE = 0,
	GP_SINGLE_CYCLE_MODE,
	GP_CONTINUOUS_MODE,
	GP_BURST_MODE,
};

enum gp_channel_id {
	GP_CH_0 = 0,
	GP_CH_1,
	GP_CH_2,
	GP_CH_3,
	GP_CH_4,
	GP_CH_5,
	GP_CH_6,
	GP_CH_7,
};

struct sunxi_config {
	u32 channel_select;
	u32 channel_data_select;
	u32 channel_compare_select;
	u32 channel_cld_select;
	u32 channel_chd_select;
	u32 channel_compare_lowdata[CHANNEL_MAX_NUM];
	u32 channel_compare_higdata[CHANNEL_MAX_NUM];
};

struct sunxi_gpadc {
	struct platform_device	*pdev;
	struct input_dev *input_gpadc[CHANNEL_MAX_NUM];
	struct sunxi_config gpadc_config;
	struct clk *mclk;
	struct clk *pclk;
	void __iomem *reg_base;
	int irq_num;
	u32 channel_num;
	u32 scankeycodes[KEY_MAX_CNT];
	u32 gpadc_sample_rate;
	char key_name[16];
	u32 key_num;
	u8 key_cnt;
	u8 compare_before;
	u8 compare_later;
	u8 key_code;
	u32 key_val;
};

struct status_reg {
	char *pst;
	char *ped;
	unsigned char channel;
	unsigned char val;
};

struct vol_reg {
	char *pst;
	char *ped;
	unsigned char index;
	unsigned long vol;
};

struct sr_reg {
	char *pst;
	unsigned long val;
};

struct filter_reg {
	char *pst;
	unsigned long val;
};

#endif
