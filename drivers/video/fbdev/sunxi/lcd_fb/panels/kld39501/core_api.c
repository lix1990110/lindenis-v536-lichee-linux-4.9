/*
 * core_api/core_api.c
 *
 * Copyright (c) 2007-2019 Allwinnertech Co., Ltd.
 * Author: zhengxiaobin <zhengxiaobin@allwinnertech.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#include "core_api.h"
#include <linux/spi/spidev.h>
#include <linux/spi/spi.h>

static struct spi_device *spi_device;
static struct disp_panel_para info[LCD_FB_MAX];

static int comm_out(unsigned int sel, unsigned char cmd)
{

	struct spi_transfer	t;

	if (!spi_device)
		return -1;

	DC(sel, 0);
	memset(&t, 0, sizeof(struct spi_transfer));


	t.tx_buf 	= &cmd;
	t.len		= 1;
	t.bits_per_word	= 8;
	t.speed_hz = 24000000;

	return spi_sync_transfer(spi_device, &t, 1);
}

static int reg_out(unsigned int sel, unsigned char reg)
{
	struct spi_transfer	t;

	if (!spi_device)
		return -1;

	memset(&t, 0, sizeof(struct spi_transfer));
	DC(sel, 1);

	t.tx_buf 	= &reg;
	t.len		= 1;
	t.bits_per_word	= 8;
	t.speed_hz = 24000000;

	return spi_sync_transfer(spi_device, &t, 1);
}

void address(unsigned int sel, int x, int y, int width, int height)
{
	comm_out(sel, 0x2B); /* Set row address */
	reg_out(sel, (y >> 8) & 0xff);
	reg_out(sel, y & 0xff);
	reg_out(sel, (height >> 8) & 0xff);
	reg_out(sel, height & 0xff);
	comm_out(sel, 0x2A); /* Set coloum address */
	reg_out(sel, (x >> 8) & 0xff);
	reg_out(sel, x & 0xff);
	reg_out(sel, (width >> 8) & 0xff);
	reg_out(sel, width & 0xff);
}

static int black_screen(unsigned int sel)
{
	struct spi_transfer t;
	int i = 0, ret = -1;
	unsigned char *pixel = NULL;
	unsigned char Bpp = 3;

	if (!spi_device)
		return -1;

	ret = bsp_disp_get_panel_info(sel, &info[sel]);
	if (ret) {
		lcd_fb_wrn("get panel info fail!\n");
		goto OUT;
	}
	if (info[sel].lcd_pixel_fmt == LCDFB_FORMAT_RGB_565 ||
	    info[sel].lcd_pixel_fmt == LCDFB_FORMAT_BGR_565)
		Bpp = 2;

	comm_out(sel, 0x2c);
	DC(sel, 1);
	pixel = kmalloc(info[sel].lcd_x * Bpp, GFP_KERNEL | __GFP_ZERO);
	if (!pixel)
		goto OUT;

	memset(&t, 0, sizeof(struct spi_transfer));
	t.tx_buf = pixel;
	t.len = info[sel].lcd_x * Bpp;
	t.bits_per_word = 8;
	t.speed_hz = 40000000;

	for (i = 0; i < info[sel].lcd_y; ++i)
		ret = spi_sync_transfer(spi_device, &t, 1);

	kfree(pixel);
OUT:

	return ret;
}

static int spi_init(void)
{
	int ret = -1;
	struct spi_master *master;

	master = spi_busnum_to_master(1);
	if (!master) {
		lcd_fb_wrn("fail to get master\n");
		goto OUT;
	}

	spi_device = spi_alloc_device(master);
	if (!spi_device) {
		lcd_fb_wrn("fail to get spi device\n");
		goto OUT;
	}

	spi_device->bits_per_word = 8;
	spi_device->max_speed_hz = 60000000; /*60Mhz*/
	spi_device->mode = SPI_MODE_0;

	ret = spi_setup(spi_device);
	if (ret) {
		lcd_fb_wrn("Faile to setup spi\n");
		goto FREE;
	}

	lcd_fb_inf("Init spi1:bits_per_word:%d max_speed_hz:%d mode:%d\n",
		   spi_device->bits_per_word, spi_device->max_speed_hz,
		   spi_device->mode);

	ret = 0;
	goto OUT;

FREE:
	spi_master_put(master);
	kfree(spi_device);
	spi_device = NULL;
OUT:
	return ret;
}


void reset_panel(unsigned int sel)
{
	int ret = 0;
	lcd_fb_here;

	ret = spi_init();
	if (ret) {
		lcd_fb_wrn("Init spi fail!\n");
		return;
	}
	sunxi_lcd_pin_cfg(sel, 1);
	RESET(sel, 1);
	sunxi_lcd_delay_ms(100);
	RESET(sel, 0);
	sunxi_lcd_delay_ms(100);
	RESET(sel, 1);
}


void init_panel(unsigned int sel)
{
	unsigned int rotate;


	if (bsp_disp_get_panel_info(sel, &info[sel])) {
		lcd_fb_wrn("get panel info fail!\n");
		return;
	}

	sunxi_lcd_delay_ms(120);

	comm_out(sel, 0x11); /* Sleep Out */
	sunxi_lcd_delay_ms(200);

	comm_out(sel, 0xf0); /* comm_out Set Control */
	reg_out(sel, 0xc3);
	comm_out(sel, 0xf0); /* */
	reg_out(sel, 0x96);

	comm_out(sel, 0xb9); /* 2data line enable */
	reg_out(sel, 0x00);
	reg_out(sel, 0x00);

	comm_out(sel, 0x36); /* Memory reg_out Access Control */

	/*MY MX MV ML RGB MH 0 0*/
	if (info[sel].lcd_x > info[sel].lcd_y)
		rotate = 0x20;
	else
		rotate = 0x40;
	reg_out(sel, rotate); /*horizon scrren*/

	comm_out(sel, 0x35); /* TE on */
	reg_out(sel, 0x00);

	comm_out(sel, 0x3A); /* Interface Pixel Format */
	/* 55----RGB565;66---RGB666 */
	if (info[sel].lcd_pixel_fmt == LCDFB_FORMAT_RGB_565 ||
	    info[sel].lcd_pixel_fmt == LCDFB_FORMAT_BGR_565) {
		reg_out(sel, 0x55);
		comm_out(sel, 0x36); /* Memory reg_out Access Control */
		if (info[sel].lcd_pixel_fmt == LCDFB_FORMAT_RGB_565)
			reg_out(sel, rotate | 0x08);
		else
			reg_out(sel, rotate & 0xf7);
	} else if (info[sel].lcd_pixel_fmt == LCDFB_FORMAT_ARGB_8888)
		reg_out(sel, 0x77);
	else
		reg_out(sel, 0x66);

	comm_out(sel, 0xB4); /* Display Inversion Control */
	reg_out(sel, 0x01);  /* 1-dot */

	comm_out(sel, 0xB7); /* Entry Mode Set */
	reg_out(sel, 0x46);  /* c6 */

	comm_out(sel, 0xe8); /* Display Output Ctrl Adjust */
	reg_out(sel, 0x40);
	reg_out(sel, 0x8a);
	reg_out(sel, 0x00);
	reg_out(sel, 0x00);
	reg_out(sel, 0x29);
	reg_out(sel, 0x19);
	reg_out(sel, 0xa5);
	reg_out(sel, 0x33);

	comm_out(sel, 0xc1);
	reg_out(sel, 0x10);

	comm_out(sel, 0xc2);
	reg_out(sel, 0xa7);

	comm_out(sel, 0xc5); /* VCOM Control */
	reg_out(sel, 0x22);

	comm_out(sel, 0xe0); /* Positive Voltage Gamma Control */
	reg_out(sel, 0xf0);
	reg_out(sel, 0x12);
	reg_out(sel, 0x15);
	reg_out(sel, 0x07);
	reg_out(sel, 0x05);
	reg_out(sel, 0x06);
	reg_out(sel, 0x3A);
	reg_out(sel, 0x55);
	reg_out(sel, 0x4B);
	reg_out(sel, 0x39);
	reg_out(sel, 0x12);
	reg_out(sel, 0x13);
	reg_out(sel, 0x1C);
	reg_out(sel, 0x20);

	comm_out(sel, 0xe1); /* Negative Voltage Gamma Control */
	reg_out(sel, 0xf0);
	reg_out(sel, 0x12);
	reg_out(sel, 0x15);
	reg_out(sel, 0x07);
	reg_out(sel, 0x05);
	reg_out(sel, 0x05);
	reg_out(sel, 0x38);
	reg_out(sel, 0x44);
	reg_out(sel, 0x4C);
	reg_out(sel, 0x3C);
	reg_out(sel, 0x19);
	reg_out(sel, 0x17);
	reg_out(sel, 0x20);
	reg_out(sel, 0x23);

	comm_out(sel, 0xf0);
	reg_out(sel, 0x3c);
	comm_out(sel, 0xf0);
	reg_out(sel, 0x69);
	address(sel, 0, 0, info[sel].lcd_x - 1, info[sel].lcd_y - 1);
	sunxi_lcd_delay_ms(200); /* Delay 120ms */

	comm_out(sel, 0x29); /* Display ON */

	black_screen(sel);
}

void exit_panel(unsigned int sel)
{
	comm_out(sel, 0x28);
	sunxi_lcd_delay_ms(20);
	comm_out(sel, 0x10);
	sunxi_lcd_delay_ms(20);

	RESET(sel, 0);
	sunxi_lcd_delay_ms(10);

	sunxi_lcd_pin_cfg(sel, 0);

	if (spi_device) {
		if (spi_device->master->cleanup)
			spi_device->master->cleanup(spi_device);
		spi_master_put(spi_device->master);
		kfree(spi_device);
		spi_device = NULL;
	}
}

int panel_blank(unsigned int sel, unsigned int en)
{
	if (en)
		comm_out(sel, 0x28);
	else
		comm_out(sel, 0x29);
	return 0;
}

int panel_set_var(unsigned int sel, struct fb_info *p_info)
{
	return 0;
}


int panel_dma_transfer(unsigned int sel, void *buf, unsigned int len)
{
	struct spi_transfer t;
	int ret = 0;

	if (!spi_device)
		return -1;

	memset(&t, 0, sizeof(struct spi_transfer));

	t.tx_buf = (void *)buf;
	t.len = len;
	t.bits_per_word = 8;
	t.speed_hz = 40000000;

	ret = spi_sync_transfer(spi_device, &t, 1);


	return ret;
}
