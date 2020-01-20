/*
 * A V4L2 driver for imx477 Raw cameras.
 *
 * Copyright (c) 2017 by Allwinnertech Co., Ltd.  http://www.allwinnertech.com
 *
 * Authors:  Zhao Wei <zhaowei@allwinnertech.com>
 *    Liang WeiJie <liangweijie@allwinnertech.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/videodev2.h>
#include <linux/clk.h>
#include <media/v4l2-device.h>
#include <media/v4l2-mediabus.h>
#include <linux/io.h>

#include "camera.h"
#include "sensor_helper.h"

MODULE_AUTHOR("lwj");
MODULE_DESCRIPTION("A low-level driver for IMX477 sensors");
MODULE_LICENSE("GPL");

#define MCLK              (24*1000*1000)
#define V4L2_IDENT_SENSOR 0x0477


/*
 * Our nominal (default) frame rate.
 */

#define SENSOR_FRAME_RATE 30

/*
 * The IMX477 i2c address
 */
#define I2C_ADDR 0x34

#define SENSOR_NUM 0x2
#define SENSOR_NAME "imx477_mipi"
#define SENSOR_NAME_2 "imx477_mipi_2"

/*
 * The default register settings
 */

static struct regval_list sensor_default_regs[] = {
#if 0
	{0x0136, 0x18},
	{0x0137, 0x00},
	{0xe000, 0x00},
	{0x4AE9, 0x18},
	{0x4AEA, 0x08},
	{0xF61C, 0x04},
	{0xF61E, 0x04},
	{0x4AE9, 0x21},
	{0x4AEA, 0x80},
	{0x38a8, 0x1F},
	{0x38a9, 0xFF},
	{0x38aa, 0x1F},
	{0x38ab, 0xFF},
	{0x55d4, 0x00},
	{0x55d5, 0x00},
	{0x55d6, 0x07},
	{0x55d7, 0xFF},
	{0x55e8, 0x07},
	{0x55e9, 0xFF},
	{0x55ea, 0x00},
	{0x55eb, 0x00},
	{0x574c, 0x07},
	{0x574d, 0xFF},
	{0x574e, 0x00},
	{0x574f, 0x00},
	{0x5754, 0x00},
	{0x5755, 0x00},
	{0x5756, 0x07},
	{0x5757, 0xFF},
	{0x5973, 0x04},
	{0x5974, 0x01},
	{0x5d13, 0xC3},
	{0x5d14, 0x58},
	{0x5d15, 0xA3},
	{0x5d16, 0x1D},
	{0x5d17, 0x65},
	{0x5d18, 0x8C},
	{0x5d1a, 0x06},
	{0x5d1b, 0xA9},
	{0x5d1c, 0x45},
	{0x5d1d, 0x3A},
	{0x5d1e, 0xAB},
	{0x5d1f, 0x15},
	{0x5d21, 0x0E},
	{0x5d22, 0x52},
	{0x5d23, 0xAA},
	{0x5d24, 0x7D},
	{0x5d25, 0x57},
	{0x5d26, 0xA8},
	{0x5d37, 0x5A},
	{0x5d38, 0x5A},
	{0x5d77, 0x7F},
	{0x7B75, 0x0E},
	{0x7B76, 0x0B},
	{0x7B77, 0x08},
	{0x7B78, 0x0A},
	{0x7B79, 0x47},
	{0x7B7C, 0x00},
	{0x7B7D, 0x00},
	{0x8D1F, 0x00},
	{0x8D27, 0x00},
	{0x9004, 0x03},
	{0x9200, 0x50},
	{0x9201, 0x6C},
	{0x9202, 0x71},
	{0x9203, 0x00},
	{0x9204, 0x71},
	{0x9205, 0x01},
	{0x9371, 0x6A},
	{0x9373, 0x6A},
	{0x9375, 0x64},
	{0x991A, 0x00},
	{0x996B, 0x8C},
	{0x996C, 0x64},
	{0x996D, 0x50},
	{0x9A4C, 0x0D},
	{0x9A4D, 0x0D},
	{0xA001, 0x0A},
	{0xA003, 0x0A},
	{0xA005, 0x0A},
	{0xA006, 0x01},
	{0xA007, 0xC0},
	{0xA009, 0xC0},

	{0x0601, 0x02},
#else
	{0x0136, 0x18},
	{0x0137, 0x00},
	{0x0808, 0x02},
	{0xE07A, 0x01},
	{0xE000, 0x00},
	{0x4AE9, 0x18},
	{0x4AEA, 0x08},
	{0xF61C, 0x04},
	{0xF61E, 0x04},
	{0x4AE9, 0x21},
	{0x4AEA, 0x80},
	{0x38A8, 0x1F},
	{0x38A9, 0xFF},
	{0x38AA, 0x1F},
	{0x38AB, 0xFF},
	{0x420B, 0x01},
	{0x55D4, 0x00},
	{0x55D5, 0x00},
	{0x55D6, 0x07},
	{0x55D7, 0xFF},
	{0x55E8, 0x07},
	{0x55E9, 0xFF},
	{0x55EA, 0x00},
	{0x55EB, 0x00},
	{0x574C, 0x07},
	{0x574D, 0xFF},
	{0x574E, 0x00},
	{0x574F, 0x00},
	{0x5754, 0x00},
	{0x5755, 0x00},
	{0x5756, 0x07},
	{0x5757, 0xFF},
	{0x5973, 0x04},
	{0x5974, 0x01},
	{0x5D13, 0xC3},
	{0x5D14, 0x58},
	{0x5D15, 0xA3},
	{0x5D16, 0x1D},
	{0x5D17, 0x65},
	{0x5D18, 0x8C},
	{0x5D1A, 0x06},
	{0x5D1B, 0xA9},
	{0x5D1C, 0x45},
	{0x5D1D, 0x3A},
	{0x5D1E, 0xAB},
	{0x5D1F, 0x15},
	{0x5D21, 0x0E},
	{0x5D22, 0x52},
	{0x5D23, 0xAA},
	{0x5D24, 0x7D},
	{0x5D25, 0x57},
	{0x5D26, 0xA8},
	{0x5D37, 0x5A},
	{0x5D38, 0x5A},
	{0x5D77, 0x7F},
	{0x7B7C, 0x00},
	{0x7B7D, 0x00},
	{0x8D1F, 0x00},
	{0x8D27, 0x00},
	{0x9004, 0x03},
	{0x9200, 0x50},
	{0x9201, 0x6C},
	{0x9202, 0x71},
	{0x9203, 0x00},
	{0x9204, 0x71},
	{0x9205, 0x01},
	{0x9371, 0x6A},
	{0x9373, 0x6A},
	{0x9375, 0x64},
	{0x990C, 0x00},
	{0x990D, 0x08},
	{0x9956, 0x8C},
	{0x9957, 0x64},
	{0x9958, 0x50},
	{0x9A48, 0x06},
	{0x9A49, 0x06},
	{0x9A4A, 0x06},
	{0x9A4B, 0x06},
	{0x9A4C, 0x06},
	{0x9A4D, 0x06},
	{0xA001, 0x0A},
	{0xA003, 0x0A},
	{0xA005, 0x0A},
	{0xA006, 0x01},
	{0xA007, 0xC0},
	{0xA009, 0xC0},

#endif
};

#if 0
static struct regval_list sensor_full_regs[] = {
    /*4056x3040 30fps 10b*/
	{0x0100, 0x00},
	{REG_DLY, 0x20},

	{0x0112, 0x0A},
	{0x0113, 0x0A},
	{0x0114, 0x03},
	{0x0342, 0x1E},
	{0x0343, 0x14},
	{0x0340, 0x0E},
	{0x0341, 0x38},
	{0x0344, 0x00},
	{0x0345, 0x00},
	{0x0346, 0x00},
	{0x0347, 0x00},
	{0x0348, 0x0F},
	{0x0349, 0xD7},
	{0x034A, 0x0B},
	{0x034B, 0xDF},
	{0x00E3, 0x00},
	{0x00E4, 0x00},
	{0x00FC, 0x0A},
	{0x00FD, 0x0A},
	{0x00FE, 0x0A},
	{0x00FF, 0x0A},
	{0x0220, 0x00},
	{0x0221, 0x11},
	{0x0381, 0x01},
	{0x0383, 0x01},
	{0x0385, 0x01},
	{0x0387, 0x01},
	{0x0900, 0x00},
	{0x0901, 0x11},
	{0x0902, 0x02},
	{0x3140, 0x02},
	{0x3C00, 0x00},
	{0x3C01, 0x03},
	{0x3C02, 0xDC},
	{0x3F0D, 0x00},
	{0x5748, 0x07},
	{0x5749, 0xFF},
	{0x574A, 0x00},
	{0x574B, 0x00},
	{0x7B75, 0x0E},
	{0x7B76, 0x09},
	{0x7B77, 0x0C},
	{0x7B78, 0x06},
	{0x7B79, 0x3B},
	{0x7B53, 0x01},
	{0x9369, 0x5A},
	{0x936B, 0x55},
	{0x936D, 0x28},
	{0x9304, 0x03},
	{0x9305, 0x00},
	{0x9E9A, 0x2F},
	{0x9E9B, 0x2F},
	{0x9E9C, 0x2F},
	{0x9E9D, 0x00},
	{0x9E9E, 0x00},
	{0x9E9F, 0x00},
	{0xA2A9, 0x60},
	{0xA2B7, 0x00},
	{0x0401, 0x00},
	{0x0404, 0x00},
	{0x0405, 0x10},
	{0x0408, 0x00},
	{0x0409, 0x00},
	{0x040A, 0x00},
	{0x040B, 0x00},
	{0x040C, 0x0F},
	{0x040D, 0xD8},
	{0x040E, 0x0B},
	{0x040F, 0xE0},
	{0x034C, 0x0F},
	{0x034D, 0xD8},
	{0x034E, 0x0B},
	{0x034F, 0xE0},
	{0x0301, 0x05},
	{0x0303, 0x02},
	{0x0305, 0x02},
	{0x0306, 0x00},
	{0x0307, 0xAF},
	{0x0309, 0x0A},
	{0x030B, 0x01},
	{0x030D, 0x04},
	{0x030E, 0x00},
	{0x030F, 0xC8},
	{0x0310, 0x01},
	{0x0820, 0x12},
	{0x0821, 0xC0},
	{0x0822, 0x00},
	{0x0823, 0x00},
	{0x080A, 0x00},
	{0x080B, 0x87},
	{0x080C, 0x00},
	{0x080D, 0x10},
	{0x080E, 0x00},
	{0x080F, 0x87},
	{0x0810, 0x00},
	{0x0811, 0x5F},
	{0x0812, 0x00},
	{0x0813, 0x5F},
	{0x0814, 0x00},
	{0x0815, 0x10},
	{0x0816, 0x01},
	{0x0817, 0x3F},
	{0x0818, 0x00},
	{0x0819, 0x3F},
	{0xE04C, 0x00},
	{0xE04D, 0x87},
	{0xE04E, 0x00},
	{0xE04F, 0x1F},
	{0x3E20, 0x01},
	{0x3E37, 0x00},
	{0x3F50, 0x00},
	{0x3F56, 0x00},
	{0x3F57, 0xDC},

	{0x0100, 0x01},
};
#endif

static struct regval_list sensor_12bfull_regs[] = {
    /*4056x3040 30fps 12b*/
	{0x0100, 0x00},
	{REG_DLY, 0x20},

	{0x0112, 0x0C},
	{0x0113, 0x0C},
	{0x0114, 0x03},
	{0x0342, 0x23},
	{0x0343, 0x00},
	{0x0340, 0x0C},
	{0x0341, 0x35},
	{0x0344, 0x00},
	{0x0345, 0x00},
	{0x0346, 0x00},
	{0x0347, 0x00},
	{0x0348, 0x0F},
	{0x0349, 0xD7},
	{0x034A, 0x0B},
	{0x034B, 0xDF},
	{0x00E3, 0x00},
	{0x00E4, 0x00},
	{0x00FC, 0x0A},
	{0x00FD, 0x0A},
	{0x00FE, 0x0A},
	{0x00FF, 0x0A},
	{0x0220, 0x00},
	{0x0221, 0x11},
	{0x0381, 0x01},
	{0x0383, 0x01},
	{0x0385, 0x01},
	{0x0387, 0x01},
	{0x0900, 0x00},
	{0x0901, 0x11},
	{0x0902, 0x02},
	{0x3140, 0x02},
	{0x3C00, 0x00},
	{0x3C01, 0x03},
	{0x3C02, 0xA2},
	{0x3F0D, 0x01},
	{0x5748, 0x07},
	{0x5749, 0xFF},
	{0x574A, 0x00},
	{0x574B, 0x00},
	{0x7B75, 0x0A},
	{0x7B76, 0x0C},
	{0x7B77, 0x07},
	{0x7B78, 0x06},
	{0x7B79, 0x3C},
	{0x7B53, 0x01},
	{0x9369, 0x5A},
	{0x936B, 0x55},
	{0x936D, 0x28},
	{0x9304, 0x03},
	{0x9305, 0x00},
	{0x9E9A, 0x2F},
	{0x9E9B, 0x2F},
	{0x9E9C, 0x2F},
	{0x9E9D, 0x00},
	{0x9E9E, 0x00},
	{0x9E9F, 0x00},
	{0xA2A9, 0x60},
	{0xA2B7, 0x00},
	{0x0401, 0x00},
	{0x0404, 0x00},
	{0x0405, 0x10},
	{0x0408, 0x00},
	{0x0409, 0x00},
	{0x040A, 0x00},
	{0x040B, 0x00},
	{0x040C, 0x0F},
	{0x040D, 0xD8},
	{0x040E, 0x0B},
	{0x040F, 0xE0},
	{0x034C, 0x0F},
	{0x034D, 0xD8},
	{0x034E, 0x0B},
	{0x034F, 0xE0},
	{0x0301, 0x05},
	{0x0303, 0x02},
	{0x0305, 0x02},
	{0x0306, 0x00},
	{0x0307, 0xAF},
	{0x0309, 0x0c},
	{0x030B, 0x01},
	{0x030D, 0x04},
	{0x030E, 0x00},
	{0x030F, 0xC8},
	{0x0310, 0x01},
	{0x0820, 0x12},
	{0x0821, 0xC0},
	{0x0822, 0x00},
	{0x0823, 0x00},
	{0x080A, 0x00},
	{0x080B, 0x87},
	{0x080C, 0x00},
	{0x080D, 0x4f},
	{0x080E, 0x00},
	{0x080F, 0x87},
	{0x0810, 0x00},
	{0x0811, 0x5F},
	{0x0812, 0x00},
	{0x0813, 0x5F},
	{0x0814, 0x00},
	{0x0815, 0x4F},
	{0x0816, 0x01},
	{0x0817, 0x3F},
	{0x0818, 0x00},
	{0x0819, 0x3F},
	{0xE04C, 0x00},
	{0xE04D, 0x87},
	{0xE04E, 0x00},
	{0xE04F, 0x1F},
	{0x3E20, 0x01},
	{0x3E37, 0x00},
	{0x3F50, 0x00},
	{0x3F56, 0x01},
	{0x3F57, 0x00},

	{0x0100, 0x01},
#if 0
	/*slave*/
	{0x3F0B, 0x01},
	{0x3041, 0x00},
	{0x3040, 0x00},
	{0x0100, 0x00},
#endif
};

#if 0
static struct regval_list sensor_4k_regs[] = {
    /*4056x2288 30fps 10b*/
	{0x0100, 0x00},
	{REG_DLY, 0x20},
	{0x0112, 0x0A},
	{0x0113, 0x0A},
	{0x0114, 0x03},
	{0x0342, 0x1E},
	{0x0343, 0x14},
	{0x0340, 0x0E},
	{0x0341, 0x38},
	{0x0344, 0x00},
	{0x0345, 0x00},
	{0x0346, 0x01},
	{0x0347, 0x78},
	{0x0348, 0x0F},
	{0x0349, 0xD7},
	{0x034A, 0x0A},
	{0x034B, 0x67},
	{0x00E3, 0x00},
	{0x00E4, 0x00},
	{0x00FC, 0x0A},
	{0x00FD, 0x0A},
	{0x00FE, 0x0A},
	{0x00FF, 0x0A},
	{0x0220, 0x00},
	{0x0221, 0x11},
	{0x0381, 0x01},
	{0x0383, 0x01},
	{0x0385, 0x01},
	{0x0387, 0x01},
	{0x0900, 0x00},
	{0x0901, 0x11},
	{0x0902, 0x02},
	{0x3140, 0x02},
	{0x3C00, 0x00},
	{0x3C01, 0x03},
	{0x3C02, 0xDC},
	{0x3F0D, 0x00},
	{0x5748, 0x07},
	{0x5749, 0xFF},
	{0x574A, 0x00},
	{0x574B, 0x00},
	{0x7B75, 0x0E},
	{0x7B76, 0x09},
	{0x7B77, 0x0C},
	{0x7B78, 0x06},
	{0x7B79, 0x3B},
	{0x7B53, 0x01},
	{0x9369, 0x5A},
	{0x936B, 0x55},
	{0x936D, 0x28},
	{0x9304, 0x03},
	{0x9305, 0x00},
	{0x9E9A, 0x2F},
	{0x9E9B, 0x2F},
	{0x9E9C, 0x2F},
	{0x9E9D, 0x00},
	{0x9E9E, 0x00},
	{0x9E9F, 0x00},
	{0xA2A9, 0x60},
	{0xA2B7, 0x00},
	{0x0401, 0x00},
	{0x0404, 0x00},
	{0x0405, 0x10},
	{0x0408, 0x00},
	{0x0409, 0x00},
	{0x040A, 0x00},
	{0x040B, 0x00},
	{0x040C, 0x0F},
	{0x040D, 0xD8},
	{0x040E, 0x08},
	{0x040F, 0xF0},
	{0x034C, 0x0F},
	{0x034D, 0xD8},
	{0x034E, 0x08},
	{0x034F, 0xF0},
	{0x0301, 0x05},
	{0x0303, 0x02},
	{0x0305, 0x02},
	{0x0306, 0x00},
	{0x0307, 0xAF},
	{0x0309, 0x0A},
	{0x030B, 0x01},
	{0x030D, 0x04},
	{0x030E, 0x00},
	{0x030F, 0xC8},
	{0x0310, 0x01},
	{0x0820, 0x12},
	{0x0821, 0xC0},
	{0x0822, 0x00},
	{0x0823, 0x00},
	{0x080A, 0x00},
	{0x080B, 0x87},
	{0x080C, 0x00},
	{0x080D, 0x10},
	{0x080E, 0x00},
	{0x080F, 0x87},
	{0x0810, 0x00},
	{0x0811, 0x5F},
	{0x0812, 0x00},
	{0x0813, 0x5F},
	{0x0814, 0x00},
	{0x0815, 0x10},
	{0x0816, 0x01},
	{0x0817, 0x3F},
	{0x0818, 0x00},
	{0x0819, 0x3F},
	{0xE04C, 0x00},
	{0xE04D, 0x87},
	{0xE04E, 0x00},
	{0xE04F, 0x1F},
	{0x3E20, 0x01},
	{0x3E37, 0x00},
	{0x3F50, 0x00},
	{0x3F56, 0x00},
	{0x3F57, 0xDC},

	{0x0100, 0x01},
};
#endif

#if 0
static struct regval_list sensor_1080p30fps_regs[] = {
	/*2028x1128 30fps 10b*/
	{0x0100, 0x00},
	{REG_DLY, 0x20},
	{0x0112, 0x0A},
	{0x0113, 0x0A},
	{0x0114, 0x03},
	{0x0342, 0x0F},
	{0x0343, 0xA4},
	{0x0340, 0x1B},/*0D  change for 60fps*/
	{0x0341, 0x58},/*AC  change for 60fps*/
	{0x0344, 0x00},
	{0x0345, 0x00},
	{0x0346, 0x01},
	{0x0347, 0x88},
	{0x0348, 0x0F},
	{0x0349, 0xD7},
	{0x034A, 0x0A},
	{0x034B, 0x57},
	{0x00E3, 0x00},
	{0x00E4, 0x00},
	{0x00FC, 0x0A},
	{0x00FD, 0x0A},
	{0x00FE, 0x0A},
	{0x00FF, 0x0A},
	{0x0220, 0x00},
	{0x0221, 0x11},
	{0x0381, 0x01},
	{0x0383, 0x01},
	{0x0385, 0x01},
	{0x0387, 0x01},
	{0x0900, 0x01},
	{0x0901, 0x22},
	{0x0902, 0x02},
	{0x3140, 0x02},
	{0x3C00, 0x00},
	{0x3C01, 0x01},
	{0x3C02, 0x9C},
	{0x3F0D, 0x00},
	{0x5748, 0x00},
	{0x5749, 0x00},
	{0x574A, 0x00},
	{0x574B, 0xA4},
	{0x7B75, 0x0E},
	{0x7B76, 0x09},
	{0x7B77, 0x08},
	{0x7B78, 0x06},
	{0x7B79, 0x34},
	{0x7B53, 0x00},
	{0x9369, 0x73},
	{0x936B, 0x64},
	{0x936D, 0x5F},
	{0x9304, 0x03},
	{0x9305, 0x80},
	{0x9E9A, 0x2F},
	{0x9E9B, 0x2F},
	{0x9E9C, 0x2F},
	{0x9E9D, 0x00},
	{0x9E9E, 0x00},
	{0x9E9F, 0x00},
	{0xA2A9, 0x27},
	{0xA2B7, 0x03},
	{0x0401, 0x00},
	{0x0404, 0x00},
	{0x0405, 0x10},
	{0x0408, 0x00},
	{0x0409, 0x00},
	{0x040A, 0x00},
	{0x040B, 0x00},
	{0x040C, 0x07},
	{0x040D, 0xEC},
	{0x040E, 0x04},
	{0x040F, 0x68},
	{0x034C, 0x07},
	{0x034D, 0xEC},
	{0x034E, 0x04},
	{0x034F, 0x68},
	{0x0301, 0x05},
	{0x0303, 0x02},
	{0x0305, 0x02},
	{0x0306, 0x00},
	{0x0307, 0xAF},
	{0x0309, 0x0A},
	{0x030B, 0x01},
	{0x030D, 0x04},
	{0x030E, 0x00},
	{0x030F, 0xC8},
	{0x0310, 0x01},
	{0x0820, 0x12},
	{0x0821, 0xC0},
	{0x0822, 0x00},
	{0x0823, 0x00},
	{0x080A, 0x00},
	{0x080B, 0x87},
	{0x080C, 0x00},
	{0x080D, 0x10},
	{0x080E, 0x00},
	{0x080F, 0x87},
	{0x0810, 0x00},
	{0x0811, 0x5F},
	{0x0812, 0x00},
	{0x0813, 0x5F},
	{0x0814, 0x00},
	{0x0815, 0x10},
	{0x0816, 0x01},
	{0x0817, 0x3F},
	{0x0818, 0x00},
	{0x0819, 0x3F},
	{0xE04C, 0x00},
	{0xE04D, 0x87},
	{0xE04E, 0x00},
	{0xE04F, 0x1F},
	{0x3E20, 0x01},
	{0x3E37, 0x00},
	{0x3F50, 0x00},
	{0x3F56, 0x00},
	{0x3F57, 0x73},

	{0x0100, 0x01},

};
#endif

#if 0
static struct regval_list sensor_1080p60fps_regs[] = {
	/*2028x1128 60fps 10b*/
	{0x0100, 0x00},
	{REG_DLY, 0x20},
	{0x0112, 0x0A},
	{0x0113, 0x0A},
	{0x0114, 0x03},
	{0x0342, 0x0F},
	{0x0343, 0xA4},
	{0x0340, 0x06},/*1B   change for 60fps*/
	{0x0341, 0xD6},/*58   change for 60fps*/
	{0x0344, 0x00},
	{0x0345, 0x00},
	{0x0346, 0x01},
	{0x0347, 0x88},
	{0x0348, 0x0F},
	{0x0349, 0xD7},
	{0x034A, 0x0A},
	{0x034B, 0x57},
	{0x00E3, 0x00},
	{0x00E4, 0x00},
	{0x00FC, 0x0A},
	{0x00FD, 0x0A},
	{0x00FE, 0x0A},
	{0x00FF, 0x0A},
	{0x0220, 0x00},
	{0x0221, 0x11},
	{0x0381, 0x01},
	{0x0383, 0x01},
	{0x0385, 0x01},
	{0x0387, 0x01},
	{0x0900, 0x01},
	{0x0901, 0x22},
	{0x0902, 0x02},
	{0x3140, 0x02},
	{0x3C00, 0x00},
	{0x3C01, 0x01},
	{0x3C02, 0x9C},
	{0x3F0D, 0x00},
	{0x5748, 0x00},
	{0x5749, 0x00},
	{0x574A, 0x00},
	{0x574B, 0xA4},
	{0x7B75, 0x0E},
	{0x7B76, 0x09},
	{0x7B77, 0x08},
	{0x7B78, 0x06},
	{0x7B79, 0x34},
	{0x7B53, 0x00},
	{0x9369, 0x73},
	{0x936B, 0x64},
	{0x936D, 0x5F},
	{0x9304, 0x03},
	{0x9305, 0x80},
	{0x9E9A, 0x2F},
	{0x9E9B, 0x2F},
	{0x9E9C, 0x2F},
	{0x9E9D, 0x00},
	{0x9E9E, 0x00},
	{0x9E9F, 0x00},
	{0xA2A9, 0x27},
	{0xA2B7, 0x03},
	{0x0401, 0x00},
	{0x0404, 0x00},
	{0x0405, 0x10},
	{0x0408, 0x00},
	{0x0409, 0x00},
	{0x040A, 0x00},
	{0x040B, 0x00},
	{0x040C, 0x07},
	{0x040D, 0xEC},
	{0x040E, 0x04},
	{0x040F, 0x68},
	{0x034C, 0x07},
	{0x034D, 0xEC},
	{0x034E, 0x04},
	{0x034F, 0x68},
	{0x0301, 0x05},
	{0x0303, 0x02},
	{0x0305, 0x02},
	{0x0306, 0x00},
	{0x0307, 0xAF},
	{0x0309, 0x0A},
	{0x030B, 0x01},
	{0x030D, 0x04},
	{0x030E, 0x00},
	{0x030F, 0xC8},
	{0x0310, 0x01},
	{0x0820, 0x12},
	{0x0821, 0xC0},
	{0x0822, 0x00},
	{0x0823, 0x00},
	{0x080A, 0x00},
	{0x080B, 0x87},
	{0x080C, 0x00},
	{0x080D, 0x10},
	{0x080E, 0x00},
	{0x080F, 0x87},
	{0x0810, 0x00},
	{0x0811, 0x5F},
	{0x0812, 0x00},
	{0x0813, 0x5F},
	{0x0814, 0x00},
	{0x0815, 0x10},
	{0x0816, 0x01},
	{0x0817, 0x3F},
	{0x0818, 0x00},
	{0x0819, 0x3F},
	{0xE04C, 0x00},
	{0xE04D, 0x87},
	{0xE04E, 0x00},
	{0xE04F, 0x1F},
	{0x3E20, 0x01},
	{0x3E37, 0x00},
	{0x3F50, 0x00},
	{0x3F56, 0x00},
	{0x3F57, 0x73},
	{0x0100, 0x01},

};
#endif

#if 0
static struct regval_list sensor_720p120fps_regs[] = {
	/*1348x750 120fps 10b*/
	{0x0100, 0x00},
	{REG_DLY, 0x20},
	{0x0112, 0x0A},
	{0x0113, 0x0A},
	{0x0114, 0x03},
	{0x0342, 0x0F},
	{0x0343, 0xA4},
	{0x0340, 0x03},/*1b  change  for 30fps*/
	{0x0341, 0x6B},/*58*/
	{0x0344, 0x00},
	{0x0345, 0x00},
	{0x0346, 0x01},
	{0x0347, 0x88},
	{0x0348, 0x0F},
	{0x0349, 0xD7},
	{0x034A, 0x0A},
	{0x034B, 0x57},
	{0x00E3, 0x00},
	{0x00E4, 0x00},
	{0x00FC, 0x0A},
	{0x00FD, 0x0A},
	{0x00FE, 0x0A},
	{0x00FF, 0x0A},
	{0x0220, 0x00},
	{0x0221, 0x11},
	{0x0381, 0x01},
	{0x0383, 0x01},
	{0x0385, 0x01},
	{0x0387, 0x01},
	{0x0900, 0x01},
	{0x0901, 0x22},
	{0x0902, 0x02},
	{0x3140, 0x02},
	{0x3C00, 0x00},
	{0x3C01, 0x01},
	{0x3C02, 0x9C},
	{0x3F0D, 0x00},
	{0x5748, 0x00},
	{0x5749, 0x00},
	{0x574A, 0x00},
	{0x574B, 0xA4},
	{0x7B75, 0x0E},
	{0x7B76, 0x09},
	{0x7B77, 0x08},
	{0x7B78, 0x06},
	{0x7B79, 0x34},
	{0x7B53, 0x00},
	{0x9369, 0x73},
	{0x936B, 0x64},
	{0x936D, 0x5F},
	{0x9304, 0x03},
	{0x9305, 0x80},
	{0x9E9A, 0x2F},
	{0x9E9B, 0x2F},
	{0x9E9C, 0x2F},
	{0x9E9D, 0x00},
	{0x9E9E, 0x00},
	{0x9E9F, 0x00},
	{0xA2A9, 0x27},
	{0xA2B7, 0x03},
	{0x0401, 0x02},
	{0x0404, 0x00},
	{0x0405, 0x18},
	{0x0408, 0x00},
	{0x0409, 0x02},
	{0x040A, 0x00},
	{0x040B, 0x00},
	{0x040C, 0x07},
	{0x040D, 0xE8},
	{0x040E, 0x04},
	{0x040F, 0x66},
	{0x034C, 0x05},
	{0x034D, 0x44},
	{0x034E, 0x02},
	{0x034F, 0xEE},
	{0x0301, 0x05},
	{0x0303, 0x02},
	{0x0305, 0x02},
	{0x0306, 0x00},
	{0x0307, 0xAF},
	{0x0309, 0x0A},
	{0x030B, 0x01},
	{0x030D, 0x04},
	{0x030E, 0x00},
	{0x030F, 0xC8},
	{0x0310, 0x01},
	{0x0820, 0x12},
	{0x0821, 0xC0},
	{0x0822, 0x00},
	{0x0823, 0x00},
	{0x080A, 0x00},
	{0x080B, 0x87},
	{0x080C, 0x00},
	{0x080D, 0x10},
	{0x080E, 0x00},
	{0x080F, 0x87},
	{0x0810, 0x00},
	{0x0811, 0x5F},
	{0x0812, 0x00},
	{0x0813, 0x5F},
	{0x0814, 0x00},
	{0x0815, 0x10},
	{0x0816, 0x01},
	{0x0817, 0x3F},
	{0x0818, 0x00},
	{0x0819, 0x3F},
	{0xE04C, 0x00},
	{0xE04D, 0x87},
	{0xE04E, 0x00},
	{0xE04F, 0x1F},
	{0x3E20, 0x01},
	{0x3E37, 0x00},
	{0x3F50, 0x00},
	{0x3F56, 0x00},
	{0x3F57, 0x73},
	{0x0100, 0x01},
};
#endif

#if 0
static struct regval_list sensor_4k_30fps_regs_slave[] = {
	{0x0100, 0x00},
	{REG_DLY, 0x20},

	{0x0112, 0x0C},
	{0x0113, 0x0C},
	{0x0114, 0x03},
	{0x0342, 0x23},
	{0x0343, 0x00},
	{0x0340, 0x0C},
	{0x0341, 0x35},
	{0x0344, 0x00},
	{0x0345, 0x00},
	{0x0346, 0x00},
	{0x0347, 0x00},
	{0x0348, 0x0F},
	{0x0349, 0xD7},
	{0x034A, 0x0B},
	{0x034B, 0xDF},
	{0x00E3, 0x00},
	{0x00E4, 0x00},
	{0x00FC, 0x0A},
	{0x00FD, 0x0A},
	{0x00FE, 0x0A},
	{0x00FF, 0x0A},
	{0x0220, 0x00},
	{0x0221, 0x11},
	{0x0381, 0x01},
	{0x0383, 0x01},
	{0x0385, 0x01},
	{0x0387, 0x01},
	{0x0900, 0x00},
	{0x0901, 0x11},
	{0x0902, 0x02},
	{0x3140, 0x02},
	{0x3C00, 0x00},
	{0x3C01, 0x03},
	{0x3C02, 0xA2},
	{0x3F0D, 0x01},
	{0x5748, 0x07},
	{0x5749, 0xFF},
	{0x574A, 0x00},
	{0x574B, 0x00},
	{0x7B75, 0x0A},
	{0x7B76, 0x0C},
	{0x7B77, 0x07},
	{0x7B78, 0x06},
	{0x7B79, 0x3C},
	{0x7B53, 0x01},
	{0x9369, 0x5A},
	{0x936B, 0x55},
	{0x936D, 0x28},
	{0x9304, 0x03},
	{0x9305, 0x00},
	{0x9E9A, 0x2F},
	{0x9E9B, 0x2F},
	{0x9E9C, 0x2F},
	{0x9E9D, 0x00},
	{0x9E9E, 0x00},
	{0x9E9F, 0x00},
	{0xA2A9, 0x60},
	{0xA2B7, 0x00},
	{0x0401, 0x00},
	{0x0404, 0x00},
	{0x0405, 0x10},
	{0x0408, 0x00},
	{0x0409, 0x00},
	{0x040A, 0x00},
	{0x040B, 0x00},
	{0x040C, 0x0F},
	{0x040D, 0xD8},
	{0x040E, 0x0B},
	{0x040F, 0xE0},
	{0x034C, 0x0F},
	{0x034D, 0xD8},
	{0x034E, 0x0B},
	{0x034F, 0xE0},
	{0x0301, 0x05},
	{0x0303, 0x02},
	{0x0305, 0x02},
	{0x0306, 0x00},
	{0x0307, 0xAF},
	{0x0309, 0x0c},
	{0x030B, 0x01},
	{0x030D, 0x04},
	{0x030E, 0x00},
	{0x030F, 0xC8},
	{0x0310, 0x01},
	{0x0820, 0x12},
	{0x0821, 0xC0},
	{0x0822, 0x00},
	{0x0823, 0x00},
	{0x080A, 0x00},
	{0x080B, 0x87},
	{0x080C, 0x00}, /*hs-preare*/
	{0x080D, 0x00}, /*hs-preare   0x4f*/
	{0x080E, 0x00},
	{0x080F, 0x87},
	{0x0810, 0x00},
	{0x0811, 0x5F},
	{0x0812, 0x00},
	{0x0813, 0x5F},
	{0x0814, 0x00},
	{0x0815, 0x4F},
	{0x0816, 0x01},
	{0x0817, 0x3F},
	{0x0818, 0x00},
	{0x0819, 0x3F},
	{0xE04C, 0x00},
	{0xE04D, 0x87},
	{0xE04E, 0x00},
	{0xE04F, 0x1F},
	{0x3E20, 0x01},
	{0x3E37, 0x00},
	{0x3F50, 0x00},
	{0x3F56, 0x01},
	{0x3F57, 0x00},

	/*slave*/
	{0x3F0B, 0x01},
	{0x3041, 0x00},
	{0x3040, 0x00},
	{0x0100, 0x00},
	{0x0100, 0x01},
	{REG_DLY, 0x50},
};
#endif
/*
 * Here we'll try to encapsulate the changes for just the output
 * video format.
 *
 */

static struct regval_list sensor_fmt_raw[] = {

};


/*
 * Code for dealing with controls.
 * fill with different sensor module
 * different sensor module has different settings here
 * if not support the follow function ,retrun -EINVAL
 */

static int sensor_g_exp(struct v4l2_subdev *sd, __s32 *value)
{
	struct sensor_info *info = to_state(sd);
	*value = info->exp;
	sensor_dbg("sensor_get_exposure = %d\n", info->exp);
	return 0;
}

/* static int imx477_sensor_vts; */
static int sensor_s_exp(struct v4l2_subdev *sd, unsigned int exp_val)
{
	data_type explow, exphigh;
	struct sensor_info *info = to_state(sd);

	exp_val = (exp_val + 8) >> 4;
	exphigh = (unsigned char)((0xff00 & exp_val) >> 8);
	explow = (unsigned char)((0x00ff & exp_val));

	sensor_write(sd, 0x0203, explow);
	sensor_write(sd, 0x0202, exphigh);

	sensor_dbg("sensor_set_exp = %d line Done!\n", exp_val);

	info->exp = exp_val;
	return 0;
}

static int sensor_g_gain(struct v4l2_subdev *sd, __s32 *value)
{
	struct sensor_info *info = to_state(sd);
	*value = info->gain;
	sensor_dbg("sensor_get_gain = %d\n", info->gain);
	return 0;
}

static int sensor_s_gain(struct v4l2_subdev *sd, int gain_val)
{
	struct sensor_info *info = to_state(sd);
	data_type gainlow = 0;
	data_type gainhigh = 0;
	int gainana = 0;
	int upper = 0;
	int lower = 0;

	if (gain_val <= 22*16) {
		gainana = 1024 - 16384 / gain_val;
		upper = 0x01;
		lower = 0x00;
	} else {
		gainana = 978;
		upper = gain_val/(16*22);
		lower = 16*((gain_val%(16*22))/22);
	}

	gainlow = (unsigned char)(gainana & 0xff);
	gainhigh = (unsigned char)((gainana >> 8) & 0xff);

	sensor_write(sd, 0x0205, gainlow);
	sensor_write(sd, 0x0204, gainhigh);

	sensor_write(sd, 0x3ff9, 0x01);
	sensor_write(sd, 0x020e, upper);
	sensor_write(sd, 0x020f, lower);

	sensor_dbg("sensor_set_gain = %d, Done!\n", gain_val);
	info->gain = gain_val;

	return 0;
}

static int imx477_mipi_sensor_vts;
static int sensor_s_exp_gain(struct v4l2_subdev *sd,
			     struct sensor_exp_gain *exp_gain)
{
	int shutter, frame_length;
	struct sensor_info *info = to_state(sd);
	int exp_val, gain_val;

	exp_val = exp_gain->exp_val;
	gain_val = exp_gain->gain_val;

	if (gain_val < 1*16)
		gain_val = 16;
	if (gain_val > 352*16 - 1)
		gain_val = 352*16 - 1;

	if (exp_val > 0xfffff)
		exp_val = 0xfffff;

	shutter = exp_val / 16;
	if (shutter > imx477_mipi_sensor_vts - 4)
		frame_length = shutter + 4;
	else
		frame_length = imx477_mipi_sensor_vts;

	sensor_write(sd, 0x0341, (frame_length & 0xff));
	sensor_write(sd, 0x0340, (frame_length >> 8));

	sensor_s_exp(sd, exp_val);
	sensor_s_gain(sd, gain_val);

	sensor_dbg("sensor_set_gain exp = %d, %d Done!\n", gain_val, exp_val);

	info->exp = exp_val;
	info->gain = gain_val;
	return 0;
}

static int sensor_s_sw_stby(struct v4l2_subdev *sd, int on_off)
{
	int ret;
	data_type rdval;

	ret = sensor_read(sd, 0x3000, &rdval);
	if (ret != 0)
		return ret;

	if (on_off == STBY_ON)
		ret = sensor_write(sd, 0x0100, rdval & 0xfe);
	else
		ret = sensor_write(sd, 0x0100, rdval | 0x01);
	return ret;
}

/*
 * Stuff that knows about the sensor.
 */
static int sensor_power(struct v4l2_subdev *sd, int on)
{
	int ret = 0;

	switch (on) {
	case STBY_ON:
		sensor_dbg("STBY_ON!\n");
		cci_lock(sd);
		ret = sensor_s_sw_stby(sd, STBY_ON);
		if (ret < 0)
			sensor_err("soft stby falied!\n");
		usleep_range(10000, 12000);
		cci_unlock(sd);
		break;
	case STBY_OFF:
		sensor_dbg("STBY_OFF!\n");
		cci_lock(sd);
		usleep_range(10000, 12000);
		ret = sensor_s_sw_stby(sd, STBY_OFF);
		if (ret < 0)
			sensor_err("soft stby off falied!\n");
		cci_unlock(sd);
		break;
	case PWR_ON:
		sensor_print("PWR_ON!\n");
		cci_lock(sd);
		vin_gpio_set_status(sd, PWDN, 1);
		vin_gpio_set_status(sd, RESET, 1);
		vin_gpio_set_status(sd, POWER_EN, 1);
		vin_gpio_write(sd, RESET, CSI_GPIO_LOW);
		vin_gpio_write(sd, PWDN, CSI_GPIO_LOW);
		vin_gpio_write(sd, POWER_EN, CSI_GPIO_HIGH);
		vin_set_pmu_channel(sd, IOVDD, ON);
		vin_set_pmu_channel(sd, AVDD, ON);
		vin_set_pmu_channel(sd, DVDD, ON);


		usleep_range(10000, 12000);
		vin_gpio_write(sd, RESET, CSI_GPIO_HIGH);
		vin_gpio_write(sd, PWDN, CSI_GPIO_HIGH);
		usleep_range(7000, 8000);
		vin_set_mclk_freq(sd, MCLK);
		vin_set_mclk(sd, ON);
		usleep_range(10000, 12000);
		cci_unlock(sd);
		break;
	case PWR_OFF:
		sensor_print("PWR_OFF!\n");
		cci_lock(sd);
		vin_gpio_set_status(sd, PWDN, 1);
		vin_gpio_set_status(sd, RESET, 1);
		vin_gpio_write(sd, RESET, CSI_GPIO_LOW);
		vin_gpio_write(sd, PWDN, CSI_GPIO_LOW);
		vin_set_mclk(sd, OFF);
		vin_set_pmu_channel(sd, AFVDD, OFF);
		vin_set_pmu_channel(sd, AVDD, OFF);
		vin_set_pmu_channel(sd, IOVDD, OFF);
		vin_set_pmu_channel(sd, DVDD, OFF);
		vin_gpio_write(sd, POWER_EN, CSI_GPIO_LOW);
		vin_gpio_set_status(sd, RESET, 0);
		vin_gpio_set_status(sd, PWDN, 0);
		vin_gpio_set_status(sd, POWER_EN, 0);
		cci_unlock(sd);
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static int sensor_reset(struct v4l2_subdev *sd, u32 val)
{
	switch (val) {
	case 0:
		vin_gpio_write(sd, RESET, CSI_GPIO_HIGH);
		usleep_range(10000, 12000);
		break;
	case 1:
		vin_gpio_write(sd, RESET, CSI_GPIO_LOW);
		usleep_range(10000, 12000);
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

static int sensor_detect(struct v4l2_subdev *sd)
{
	data_type rdval = 0;

	sensor_read(sd, 0x0000, &rdval);
	sensor_print("%s read value is 0x%x\n", __func__, rdval);
	sensor_read(sd, 0x0001, &rdval);
	sensor_print("%s read value is 0x%x\n", __func__, rdval);
	sensor_read(sd, 0x0016, &rdval);
	sensor_print("%s read value is 0x%x\n", __func__, rdval);
	return 0;
}

static int sensor_init(struct v4l2_subdev *sd, u32 val)
{
	int ret;
	struct sensor_info *info = to_state(sd);

	sensor_dbg("sensor_init\n");

	/*Make sure it is a target sensor */
	ret = sensor_detect(sd);
	if (ret) {
		sensor_err("chip found is not an target chip.\n");
		return ret;
	}

	info->focus_status = 0;
	info->low_speed = 0;
	info->width = 4056;
	info->height = 3040;
	info->hflip = 0;
	info->vflip = 0;
	info->gain = 0;

	info->tpf.numerator = 1;
	info->tpf.denominator = 30;	/* 30fps */

	return 0;
}

static long sensor_ioctl(struct v4l2_subdev *sd, unsigned int cmd, void *arg)
{
	int ret = 0;
	struct sensor_info *info = to_state(sd);

	switch (cmd) {
	case GET_CURRENT_WIN_CFG:
		if (info->current_wins != NULL) {
			memcpy(arg, info->current_wins,
				sizeof(struct sensor_win_size));
			ret = 0;
		} else {
			sensor_err("empty wins!\n");
			ret = -1;
		}
		break;
	case SET_FPS:
		ret = 0;
		break;
	case VIDIOC_VIN_SENSOR_EXP_GAIN:
		ret = sensor_s_exp_gain(sd, (struct sensor_exp_gain *)arg);
		break;
	case VIDIOC_VIN_SENSOR_CFG_REQ:
		sensor_cfg_req(sd, (struct sensor_config *)arg);
		break;
	default:
		return -EINVAL;
	}
	return ret;
}

/*
 * Store information about the video data format.
 */
static struct sensor_format_struct sensor_formats[] = {
	{
		.desc = "Raw RGB Bayer",
		.mbus_code = MEDIA_BUS_FMT_SRGGB10_1X10,
		.regs = sensor_fmt_raw,
		.regs_size = ARRAY_SIZE(sensor_fmt_raw),
		.bpp = 1
	},
};
#define N_FMTS ARRAY_SIZE(sensor_formats)

/*
 * Then there is the issue of window sizes.  Try to capture the info here.
 */

static struct sensor_win_size sensor_win_sizes[] = {
	{  /*4000*3000 15fps*/
	 .width = 3840,/*4000,*/
	 .height = 2160,/*3000,*/
	 .hoffset = 24,
	 .voffset = 20,
	 .hts = 7700,
	 .vts = 3640,
	 .pclk = 600 * 1000 * 1000,
	 .mipi_bps = 1200 * 1000 * 1000,
	 .fps_fixed = 30,
	 .bin_factor = 1,
	 .intg_min = 1 << 4,
	 .intg_max = (3640 - 20) << 4,
	 .gain_min = 1 << 4,
	 .gain_max = 22 << 4,
	 .regs = sensor_12bfull_regs,
	 .regs_size = ARRAY_SIZE(sensor_12bfull_regs),
	 .set_size = NULL,
	 },

#if 0
	{  /*4000*3000 15fps*/
	 .width = 4000,
	 .height = 3000,
	 .hoffset = 24,
	 .voffset = 20,
	 .hts = 7700,
	 .vts = 3640,
	 .pclk = 600 * 1000 * 1000,
	 .mipi_bps = 1200 * 1000 * 1000,
	 .fps_fixed = 30,
	 .bin_factor = 1,
	 .intg_min = 1 << 4,
	 .intg_max = (3640 - 20) << 4,
	 .gain_min = 1 << 4,
	 .gain_max = 22 << 4,
	 .regs = sensor_12bfull_regs,
	 .regs_size = ARRAY_SIZE(sensor_12bfull_regs),
	 .set_size = NULL,
	 },
#endif
   #if 0
	{  /*4000*3000 15fps*/
	 .width = 4000,
	 .height = 3000,
	 .hoffset = 24,
	 .voffset = 20,
	 .hts = 7700,
	 .vts = 3640,
	 .pclk = 420 * 1000 * 1000,
	 .mipi_bps = 420 * 1000 * 1000,
	 .fps_fixed = 30,
	 .bin_factor = 1,
	 .intg_min = 1 << 4,
	 .intg_max = (3640 - 20) << 4,
	 .gain_min = 1 << 4,
	 .gain_max = 22 << 4,
	 .regs = sensor_full_regs,
	 .regs_size = ARRAY_SIZE(sensor_full_regs),
	 .set_size = NULL,
	 },
	#endif

    #if 0
	{   /*4k 15fps*/
	 .width = 3840,/*4032*/
	 .height = 2160,/*2268*/
	 .hoffset = 104,/*8*/
	 .voffset = 64,/*10*/
	 .hts = 7700,
	 .vts = 3640,
	 .pclk = 420 * 1000 * 1000,
	 .mipi_bps = 840 * 1000 * 1000,
	 .fps_fixed = 30,
	 .bin_factor = 1,
	 .intg_min = 1 << 4,
	 .intg_max = (3640 - 20) << 4,
	 .gain_min = 1 << 4,
	 .gain_max = 352 << 4,
	 .regs = sensor_4k_regs,
	 .regs_size = ARRAY_SIZE(sensor_4k_regs),
	 .set_size = NULL,
	 },
    #endif
	#if 0
	{   /*30fps*/
	 .width = 1920,
	 .height = 1080,
	 .hoffset = 54,
	 .voffset = 24,
	 .hts = 4004,
	 .vts = 3500,
	 .pclk = 420 * 1000 * 1000,
	 .mipi_bps = 840 * 1000 * 1000,
	 .fps_fixed = 30,
	 .bin_factor = 1,
	 .intg_min = 1 << 4,
	 .intg_max = (3500 - 20) << 4,
	 .gain_min = 1 << 4,
	 .gain_max = 352 << 4,
	 .regs = sensor_1080p30fps_regs,
	 .regs_size = ARRAY_SIZE(sensor_1080p30fps_regs),
	 .set_size = NULL,
	 },
     #endif

	 #if 0
	{   /*60fps*/
		  .width = 1920,
		  .height = 1080,
		  .hoffset = 54,
		  .voffset = 24,
		  .hts = 4004,
		  .vts = 1750,
		  .pclk = 420 * 1000 * 1000,
		  .mipi_bps = 840 * 1000 * 1000,
		  .fps_fixed = 30,
		  .bin_factor = 1,
		  .intg_min = 1 << 4,
		  .intg_max = (1750 - 20) << 4,
		  .gain_min = 1 << 4,
		  .gain_max = 352 << 4,
		  .regs = sensor_1080p60fps_regs,
		  .regs_size = ARRAY_SIZE(sensor_1080p60fps_regs),
		  .set_size = NULL,
	},
	 #endif

     #if 0
	{  /*720p120fps*/
	 .width = 1280,
	 .height = 720,
	 .hoffset = 34,
	 .voffset = 15,
	 .hts = 4004,
	 .vts = 875,
	 .pclk = 420 * 1000 * 1000,
	 .mipi_bps = 840 * 1000 * 1000,
	 .fps_fixed = 30,
	 .bin_factor = 1,
	 .intg_min = 1 << 4,
	 .intg_max = (875 - 20) << 4,
	 .gain_min = 1 << 4,
	 .gain_max = 352 << 4,
	 .regs = sensor_720p120fps_regs,
	 .regs_size = ARRAY_SIZE(sensor_720p120fps_regs),
	 .set_size = NULL,
	 },
	 #endif
};

#define N_WIN_SIZES (ARRAY_SIZE(sensor_win_sizes))

static int sensor_g_mbus_config(struct v4l2_subdev *sd,
				struct v4l2_mbus_config *cfg)
{
	cfg->type = V4L2_MBUS_CSI2;
	cfg->flags = 0 | V4L2_MBUS_CSI2_4_LANE | V4L2_MBUS_CSI2_CHANNEL_0;

	return 0;
}

static int sensor_g_ctrl(struct v4l2_ctrl *ctrl)
{
	struct sensor_info *info =
			container_of(ctrl->handler, struct sensor_info, handler);
	struct v4l2_subdev *sd = &info->sd;

	switch (ctrl->id) {
	case V4L2_CID_GAIN:
		return sensor_g_gain(sd, &ctrl->val);
	case V4L2_CID_EXPOSURE:
		return sensor_g_exp(sd, &ctrl->val);
	}
	return -EINVAL;
}

static int sensor_s_ctrl(struct v4l2_ctrl *ctrl)
{
	struct sensor_info *info =
			container_of(ctrl->handler, struct sensor_info, handler);
	struct v4l2_subdev *sd = &info->sd;

	switch (ctrl->id) {
	case V4L2_CID_GAIN:
		return sensor_s_gain(sd, ctrl->val);
	case V4L2_CID_EXPOSURE:
		return sensor_s_exp(sd, ctrl->val);
	}
	return -EINVAL;
}

static int sensor_reg_init(struct sensor_info *info)
{
	int ret;
	struct v4l2_subdev *sd = &info->sd;
	struct sensor_format_struct *sensor_fmt = info->fmt;
	struct sensor_win_size *wsize = info->current_wins;
	/* struct sensor_exp_gain exp_gain; */

	ret = sensor_write_array(sd, sensor_default_regs,
				 ARRAY_SIZE(sensor_default_regs));
	if (ret < 0) {
		sensor_err("write sensor_default_regs error\n");
		return ret;
	}

	sensor_dbg("sensor_reg_init\n");

	sensor_write_array(sd, sensor_fmt->regs, sensor_fmt->regs_size);

	if (wsize->regs)
		sensor_write_array(sd, wsize->regs, wsize->regs_size);

	if (wsize->set_size)
		wsize->set_size(sd);

	info->width = wsize->width;
	info->height = wsize->height;
	imx477_mipi_sensor_vts = wsize->vts;

	/*exp_gain.exp_val = 12480;*/
	/*exp_gain.gain_val = 48;*/
	/*sensor_s_exp_gain(sd, &exp_gain);*/

	sensor_print("s_fmt set width = %d, height = %d\n", wsize->width,
		     wsize->height);

	return 0;
}

static int sensor_s_stream(struct v4l2_subdev *sd, int enable)
{
	struct sensor_info *info = to_state(sd);

	sensor_print("%s on = %d, %d*%d %x\n", __func__, enable,
		     info->current_wins->width,
		     info->current_wins->height, info->fmt->mbus_code);
	if (!enable) {
		vin_gpio_set_status(sd, SM_HS, 0);
		vin_gpio_set_status(sd, SM_VS, 0);
			return 0;
	} else {
		vin_gpio_set_status(sd, SM_VS, 3);
		vin_gpio_set_status(sd, SM_HS, 3);
	}

	return sensor_reg_init(info);
}

/* ----------------------------------------------------------------------- */

static const struct v4l2_ctrl_ops sensor_ctrl_ops = {
	.g_volatile_ctrl = sensor_g_ctrl,
	.s_ctrl = sensor_s_ctrl,
};

static const struct v4l2_subdev_core_ops sensor_core_ops = {
	.reset = sensor_reset,
	.init = sensor_init,
	.s_power = sensor_power,
	.ioctl = sensor_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl32 = sensor_compat_ioctl32,
#endif
};

static const struct v4l2_subdev_video_ops sensor_video_ops = {
	.s_parm = sensor_s_parm,
	.g_parm = sensor_g_parm,
	.s_stream = sensor_s_stream,
	.g_mbus_config = sensor_g_mbus_config,
};

static const struct v4l2_subdev_pad_ops sensor_pad_ops = {
	.enum_mbus_code = sensor_enum_mbus_code,
	.enum_frame_size = sensor_enum_frame_size,
	.get_fmt = sensor_get_fmt,
	.set_fmt = sensor_set_fmt,
};

static const struct v4l2_subdev_ops sensor_ops = {
	.core = &sensor_core_ops,
	.video = &sensor_video_ops,
	.pad = &sensor_pad_ops,
};

/* ----------------------------------------------------------------------- */
static struct cci_driver cci_drv[] = {
	{
		.name = SENSOR_NAME,
		.addr_width = CCI_BITS_16,
		.data_width = CCI_BITS_8,
	}, {
		.name = SENSOR_NAME_2,
		.addr_width = CCI_BITS_16,
		.data_width = CCI_BITS_8,
	}
};

static int sensor_init_controls(struct v4l2_subdev *sd, const struct v4l2_ctrl_ops *ops)
{
	struct sensor_info *info = to_state(sd);
	struct v4l2_ctrl_handler *handler = &info->handler;
	struct v4l2_ctrl *ctrl;
	int ret = 0;

	v4l2_ctrl_handler_init(handler, 2);

	v4l2_ctrl_new_std(handler, ops, V4L2_CID_GAIN, 1 * 1600,
			      256 * 1600, 1, 1 * 1600);
	ctrl = v4l2_ctrl_new_std(handler, ops, V4L2_CID_EXPOSURE, 0,
			      65536 * 16, 1, 0);
	if (ctrl != NULL)
		ctrl->flags |= V4L2_CTRL_FLAG_VOLATILE;

	if (handler->error) {
		ret = handler->error;
		v4l2_ctrl_handler_free(handler);
	}

	sd->ctrl_handler = handler;

	return ret;
}

static int sensor_dev_id;
static int sensor_probe(struct i2c_client *client,
			const struct i2c_device_id *id)
{
	struct v4l2_subdev *sd;
	struct sensor_info *info;
	int i;

	info = kzalloc(sizeof(struct sensor_info), GFP_KERNEL);
	if (info == NULL)
		return -ENOMEM;
	sd = &info->sd;
	if (client) {
		for (i = 0; i < SENSOR_NUM; i++) {
			if (!strcmp(cci_drv[i].name, client->name))
				break;
		}
		cci_dev_probe_helper(sd, client, &sensor_ops, &cci_drv[i]);
	} else {
		cci_dev_probe_helper(sd, client, &sensor_ops, &cci_drv[sensor_dev_id++]);
	}

	sensor_init_controls(sd, &sensor_ctrl_ops);

	mutex_init(&info->lock);

	info->fmt = &sensor_formats[0];
	info->fmt_pt = &sensor_formats[0];
	info->win_pt = &sensor_win_sizes[0];
	info->fmt_num = N_FMTS;
	info->win_size_num = N_WIN_SIZES;
	info->sensor_field = V4L2_FIELD_NONE;
	info->stream_seq = MIPI_BEFORE_SENSOR;
	info->combo_mode = CMB_TERMINAL_RES | CMB_PHYA_OFFSET2 | MIPI_NORMAL_MODE;
	info->af_first_flag = 1;
	info->exp = 0;
	info->gain = 0;

	return 0;
}
static int sensor_remove(struct i2c_client *client)
{
	struct v4l2_subdev *sd;
	int i;

	if (client) {
		for (i = 0; i < SENSOR_NUM; i++) {
			if (!strcmp(cci_drv[i].name, client->name))
				break;
		}
		sd = cci_dev_remove_helper(client, &cci_drv[i]);
	} else {
		sd = cci_dev_remove_helper(client, &cci_drv[sensor_dev_id++]);
	}
	kfree(to_state(sd));
	return 0;
}

static const struct i2c_device_id sensor_id[] = {
	{SENSOR_NAME, 0},
	{}
};

static const struct i2c_device_id sensor_id_2[] = {
	{SENSOR_NAME_2, 0},
	{}
};

MODULE_DEVICE_TABLE(i2c, sensor_id);
MODULE_DEVICE_TABLE(i2c, sensor_id_2);

static struct i2c_driver sensor_driver[] = {
	{
		.driver = {
			   .owner = THIS_MODULE,
			   .name = SENSOR_NAME,
			   },
		.probe = sensor_probe,
		.remove = sensor_remove,
		.id_table = sensor_id,
	}, {
		.driver = {
			   .owner = THIS_MODULE,
			   .name = SENSOR_NAME_2,
			   },
		.probe = sensor_probe,
		.remove = sensor_remove,
		.id_table = sensor_id_2,
	},
};
static __init int init_sensor(void)
{
	int i, ret = 0;

	sensor_dev_id = 0;

	for (i = 0; i < SENSOR_NUM; i++)
		ret = cci_dev_init_helper(&sensor_driver[i]);

	return ret;
}

static __exit void exit_sensor(void)
{
	int i;

	sensor_dev_id = 0;

	for (i = 0; i < SENSOR_NUM; i++)
		cci_dev_exit_helper(&sensor_driver[i]);
}

module_init(init_sensor);
module_exit(exit_sensor);