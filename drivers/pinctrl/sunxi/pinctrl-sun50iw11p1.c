/*
 * Allwinner sun50iw11p1 SoCs pinctrl driver.
 *
 * Copyright(c) 2012-2016 Allwinnertech Co., Ltd.
 * Author: huanghuafeng <huafenghuang@allwinnertech.com>
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2.  This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/pinctrl/pinctrl.h>

#include "pinctrl-sunxi.h"

static const struct sunxi_desc_pin sun50iw11p1_pins[] = {
	//Register Name: PB_CFG0
	SUNXI_PIN(SUNXI_PINCTRL_PIN(B, 0),
		SUNXI_FUNCTION(0x0, "gpio_in"),
		SUNXI_FUNCTION(0x1, "gpio_out"),
		SUNXI_FUNCTION(0x2, "uart2"),		/* TX */
		SUNXI_FUNCTION(0x3, "pwm0"),
		SUNXI_FUNCTION(0x4, "jtag0"),		/*  MS  */
		SUNXI_FUNCTION(0x5, "ledc"),
		SUNXI_FUNCTION_IRQ_BANK(0x6, 0, 0),
		SUNXI_FUNCTION(0x7, "io_disabled")),
	SUNXI_PIN(SUNXI_PINCTRL_PIN(B, 1),
		SUNXI_FUNCTION(0x0, "gpio_in"),
		SUNXI_FUNCTION(0x1, "gpio_out"),
		SUNXI_FUNCTION(0x2, "uart2"),		/* RX */
		SUNXI_FUNCTION(0x3, "pwm1"),
		SUNXI_FUNCTION(0x4, "jtag0"),		/* CK  */
		SUNXI_FUNCTION(0x5, "i2s0"),		/* MCLK */
		SUNXI_FUNCTION_IRQ_BANK(0x6, 0, 1),
		SUNXI_FUNCTION(0x7, "io_disabled")),
	SUNXI_PIN(SUNXI_PINCTRL_PIN(B, 2),
		SUNXI_FUNCTION(0x0, "gpio_in"),
		SUNXI_FUNCTION(0x1, "gpio_out"),
		SUNXI_FUNCTION(0x2, "uart2"),		/* RTS */
		SUNXI_FUNCTION(0x3, "pwm2"),
		SUNXI_FUNCTION(0x4, "jtag0"),		/* DO  */
		SUNXI_FUNCTION(0x5, "i2s0"),		/* LRCK */
		SUNXI_FUNCTION_IRQ_BANK(0x6, 0, 2),
		SUNXI_FUNCTION(0x7, "io_disabled")),
	SUNXI_PIN(SUNXI_PINCTRL_PIN(B, 3),
		SUNXI_FUNCTION(0x0, "gpio_in"),
		SUNXI_FUNCTION(0x1, "gpio_out"),
		SUNXI_FUNCTION(0x2, "uart2"),		/* CTS */
		SUNXI_FUNCTION(0x3, "pwm3"),
		SUNXI_FUNCTION(0x4, "jtag0"),		/* DI  */
		SUNXI_FUNCTION(0x5, "i2s0"),		/* BLCK */
		SUNXI_FUNCTION_IRQ_BANK(0x6, 0, 3),
		SUNXI_FUNCTION(0x7, "io_disabled")),
	SUNXI_PIN(SUNXI_PINCTRL_PIN(B, 4),
		SUNXI_FUNCTION(0x0, "gpio_in"),
		SUNXI_FUNCTION(0x1, "gpio_out"),
		SUNXI_FUNCTION(0x2, "uart0"),		/* TX */
		SUNXI_FUNCTION(0x3, "pwm4"),
		SUNXI_FUNCTION(0x4, "i2s0"),		/* DOUT0 */
		SUNXI_FUNCTION(0x5, "i2s0"),		/* DIN1 */
		SUNXI_FUNCTION_IRQ_BANK(0x6, 0, 4),
		SUNXI_FUNCTION(0x7, "io_disabled")),
	SUNXI_PIN(SUNXI_PINCTRL_PIN(B, 5),
		SUNXI_FUNCTION(0x0, "gpio_in"),
		SUNXI_FUNCTION(0x1, "gpio_out"),
		SUNXI_FUNCTION(0x2, "uart0"),		/* RX */
		SUNXI_FUNCTION(0x3, "pwm5"),
		SUNXI_FUNCTION(0x4, "i2s0"),		/* DOUT1 */
		SUNXI_FUNCTION(0x5, "i2s0"),		/* DIN0 */
		SUNXI_FUNCTION_IRQ_BANK(0x6, 0, 5),
		SUNXI_FUNCTION(0x7, "io_disabled")),
	SUNXI_PIN(SUNXI_PINCTRL_PIN(B, 6),
		SUNXI_FUNCTION(0x0, "gpio_in"),
		SUNXI_FUNCTION(0x1, "gpio_out"),
		SUNXI_FUNCTION(0x2, "ir"),		/* RX */
		SUNXI_FUNCTION(0x3, "pwm6"),
		SUNXI_FUNCTION(0x4, "i2s0"),		/* DOUT2 */
		SUNXI_FUNCTION(0x5, "twi0"),		/* SCK */
		SUNXI_FUNCTION_IRQ_BANK(0x6, 0, 6),
		SUNXI_FUNCTION(0x7, "io_disabled")),
	SUNXI_PIN(SUNXI_PINCTRL_PIN(B, 7),
		SUNXI_FUNCTION(0x0, "gpio_in"),
		SUNXI_FUNCTION(0x1, "gpio_out"),
		SUNXI_FUNCTION(0x2, "ir"),		/* TX */
		SUNXI_FUNCTION(0x3, "pwm7"),
		SUNXI_FUNCTION(0x4, "i2s0"),		/* DOUT3 */
		SUNXI_FUNCTION(0x5, "twi0"),		/* SDA */
		SUNXI_FUNCTION_IRQ_BANK(0x6, 0, 7),
		SUNXI_FUNCTION(0x7, "io_disabled")),
#if defined (CONFIG_FPGA_V4_PLATFORM) || defined (CONFIG_FPGA_T7_PLATFORM)
	SUNXI_PIN(SUNXI_PINCTRL_PIN(B, 8),
		SUNXI_FUNCTION(0x2, "uart0")),		/* TXD */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(B, 9),
		SUNXI_FUNCTION(0x2, "uart0")),		/* RXD */
#else
	SUNXI_PIN(SUNXI_PINCTRL_PIN(B, 8),
		SUNXI_FUNCTION(0x0, "gpio_in"),
		SUNXI_FUNCTION(0x1, "gpio_out"),
		SUNXI_FUNCTION(0x2, "ir"),		/* TX */
		SUNXI_FUNCTION(0x3, "pwm8"),
		SUNXI_FUNCTION(0x4, "ir"),		/* RX */
		SUNXI_FUNCTION(0x5, "ledc"),		/* DO */
		SUNXI_FUNCTION_IRQ_BANK(0x6, 0, 8),
		SUNXI_FUNCTION(0x7, "io_disabled")),
#endif
	/* HOLE */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(C, 0),
		SUNXI_FUNCTION(0x0, "gpio_in"),
		SUNXI_FUNCTION(0x1, "gpio_out"),
		SUNXI_FUNCTION(0x2, "nand0"),		/* WE */
		SUNXI_FUNCTION(0x3, "sdc0"),		/* CLK */
		SUNXI_FUNCTION(0x4, "spi0"),		/* WP */
		SUNXI_FUNCTION(0x7, "io_disabled")),
	SUNXI_PIN(SUNXI_PINCTRL_PIN(C, 1),
		SUNXI_FUNCTION(0x0, "gpio_in"),
		SUNXI_FUNCTION(0x1, "gpio_out"),
		SUNXI_FUNCTION(0x2, "nand0"),		/* ALE */
		SUNXI_FUNCTION(0x3, "sdc0"),		/* CMD */
		SUNXI_FUNCTION(0x4, "spi0"),		/* MISO */
		SUNXI_FUNCTION(0x7, "io_disabled")),
	SUNXI_PIN(SUNXI_PINCTRL_PIN(C, 2),
		SUNXI_FUNCTION(0x0, "gpio_in"),
		SUNXI_FUNCTION(0x1, "gpio_out"),
		SUNXI_FUNCTION(0x2, "nand0"),		/* CLE */
		SUNXI_FUNCTION(0x3, "sdc0"),		/* D3 */
		SUNXI_FUNCTION(0x4, "spi0"),		/* CS0 */
		SUNXI_FUNCTION(0x7, "io_disabled")),
	SUNXI_PIN(SUNXI_PINCTRL_PIN(C, 3),
		SUNXI_FUNCTION(0x0, "gpio_in"),
		SUNXI_FUNCTION(0x1, "gpio_out"),
		SUNXI_FUNCTION(0x2, "nand0"),		/* CE0 */
		SUNXI_FUNCTION(0x3, "sdc0"),		/* D0 */
		SUNXI_FUNCTION(0x4, "spi0"),		/* HOLD */
		SUNXI_FUNCTION(0x7, "io_disabled")),
	SUNXI_PIN(SUNXI_PINCTRL_PIN(C, 4),
		SUNXI_FUNCTION(0x0, "gpio_in"),
		SUNXI_FUNCTION(0x1, "gpio_out"),
		SUNXI_FUNCTION(0x2, "nand0"),		/* RE */
		SUNXI_FUNCTION(0x3, "sdc0"),		/* D1 */
		SUNXI_FUNCTION(0x4, "spi0"),		/* CLK */
		SUNXI_FUNCTION(0x7, "io_disabled")),
	SUNXI_PIN(SUNXI_PINCTRL_PIN(C, 5),
		SUNXI_FUNCTION(0x0, "gpio_in"),
		SUNXI_FUNCTION(0x1, "gpio_out"),
		SUNXI_FUNCTION(0x2, "nand0"),		/* RB0 */
		SUNXI_FUNCTION(0x3, "sdc0"),		/* D2 */
		SUNXI_FUNCTION(0x4, "spi0"),		/* MOSI */
		SUNXI_FUNCTION(0x7, "io_disabled")),
	SUNXI_PIN(SUNXI_PINCTRL_PIN(C, 6),
		SUNXI_FUNCTION(0x0, "gpio_in"),
		SUNXI_FUNCTION(0x1, "gpio_out"),
		SUNXI_FUNCTION(0x2, "nand0"),		/* DQ7 */
		SUNXI_FUNCTION(0x3, "sdc0"),		/* RST */
		SUNXI_FUNCTION(0x7, "io_disabled")),
	SUNXI_PIN(SUNXI_PINCTRL_PIN(C, 7),
		SUNXI_FUNCTION(0x0, "gpio_in"),
		SUNXI_FUNCTION(0x1, "gpio_out"),
		SUNXI_FUNCTION(0x2, "nand0"),		/* DQ6 */
		SUNXI_FUNCTION(0x5, "boot_sel"),
		SUNXI_FUNCTION(0x7, "io_disabled")),
	/* HOLE */
	//Register Name: PF_CFG0
	SUNXI_PIN(SUNXI_PINCTRL_PIN(F, 0),
		SUNXI_FUNCTION(0x0, "gpio_in"),
		SUNXI_FUNCTION(0x1, "gpio_out"),
		SUNXI_FUNCTION(0x2, "nand0"),		/* DQ5 */
		SUNXI_FUNCTION(0x3, "sim0"),		/* VPPEN */
		SUNXI_FUNCTION(0x4, "jtag0"),		/* MS */
		SUNXI_FUNCTION(0x5, "sdc0"),		/* D1 */
		SUNXI_FUNCTION_IRQ_BANK(0x6, 1, 0),	/* PF_EINT0 */
		SUNXI_FUNCTION(0x7, "io_disabled")),
	SUNXI_PIN(SUNXI_PINCTRL_PIN(F, 1),
		SUNXI_FUNCTION(0x0, "gpio_in"),
		SUNXI_FUNCTION(0x1, "gpio_out"),
		SUNXI_FUNCTION(0x2, "nand0"),		/* DQ4 */
		SUNXI_FUNCTION(0x3, "sim0"),		/* VPPPP */
		SUNXI_FUNCTION(0x4, "jtag0"),		/* DI */
		SUNXI_FUNCTION(0x5, "sdc0"),		/* D0 */
		SUNXI_FUNCTION_IRQ_BANK(0x6, 1, 1),	/* PF_EINT1 */
		SUNXI_FUNCTION(0x7, "io_disabled")),
	SUNXI_PIN(SUNXI_PINCTRL_PIN(F, 2),
		SUNXI_FUNCTION(0x0, "gpio_in"),
		SUNXI_FUNCTION(0x1, "gpio_out"),
		SUNXI_FUNCTION(0x2, "nand0"),		/* DQS */
		SUNXI_FUNCTION(0x3, "sim0"),		/* PWREN */
		SUNXI_FUNCTION(0x4, "uart0"),		/* TX */
		SUNXI_FUNCTION(0x5, "sdc0"),		/* CLK */
		SUNXI_FUNCTION_IRQ_BANK(0x6, 1, 2),	/* PF_EINT2 */
		SUNXI_FUNCTION(0x7, "io_disabled")),
	SUNXI_PIN(SUNXI_PINCTRL_PIN(F, 3),
		SUNXI_FUNCTION(0x0, "gpio_in"),
		SUNXI_FUNCTION(0x1, "gpio_out"),
		SUNXI_FUNCTION(0x2, "nand0"),		/* DQ3 */
		SUNXI_FUNCTION(0x3, "sim0"),		/* CLK */
		SUNXI_FUNCTION(0x4, "jtag0"),		/* DO */
		SUNXI_FUNCTION(0x5, "sdc0"),		/* CMD */
		SUNXI_FUNCTION_IRQ_BANK(0x6, 1, 3),	/* PF_EINT3 */
		SUNXI_FUNCTION(0x7, "io_disabled")),
	SUNXI_PIN(SUNXI_PINCTRL_PIN(F, 4),
		SUNXI_FUNCTION(0x0, "gpio_in"),
		SUNXI_FUNCTION(0x1, "gpio_out"),
		SUNXI_FUNCTION(0x2, "nand0"),		/* DQ2 */
		SUNXI_FUNCTION(0x3, "sim0"),		/* DATA */
		SUNXI_FUNCTION(0x4, "uart0"),		/* RX */
		SUNXI_FUNCTION(0x5, "sdc0"),		/* D3 */
		SUNXI_FUNCTION_IRQ_BANK(0x6, 1, 4),	/* PF_EINT4 */
		SUNXI_FUNCTION(0x7, "io_disabled")),
	SUNXI_PIN(SUNXI_PINCTRL_PIN(F, 5),
		SUNXI_FUNCTION(0x0, "gpio_in"),
		SUNXI_FUNCTION(0x1, "gpio_out"),
		SUNXI_FUNCTION(0x2, "nand0"),		/* DQ1 */
		SUNXI_FUNCTION(0x3, "sim0"),		/* RST */
		SUNXI_FUNCTION(0x4, "jtag0"),		/* CK */
		SUNXI_FUNCTION(0x5, "sdc00"),		/* D2 */
		SUNXI_FUNCTION_IRQ_BANK(0x6, 1, 5),	/* PF_EINT5 */
		SUNXI_FUNCTION(0x7, "io_disabled")),
	SUNXI_PIN(SUNXI_PINCTRL_PIN(F, 6),
		SUNXI_FUNCTION(0x0, "gpio_in"),
		SUNXI_FUNCTION(0x1, "gpio_out"),
		SUNXI_FUNCTION(0x2, "nand0"),		/* DQ0 */
		SUNXI_FUNCTION(0x3, "sim0"),		/* DET */
		SUNXI_FUNCTION(0x4, "spdif"),		/* IN */
		SUNXI_FUNCTION_IRQ_BANK(0x6, 1, 6),	/* PF_EINT6 */
		SUNXI_FUNCTION(0x7, "io_disabled")),

#if defined (CONFIG_FPGA_V4_PLATFORM) || defined(CONFIG_FPGA_V7_PLATFORM)
	/* maybe error,copy from sun50iw10p1 platform */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(F, 9),
		SUNXI_FUNCTION(0x2, "dmic"),		/* DMIC_CLK */
		SUNXI_FUNCTION(0x7, "io_disabled")),
	SUNXI_PIN(SUNXI_PINCTRL_PIN(F, 24),
		SUNXI_FUNCTION(0x5, "spi0"),
		SUNXI_FUNCTION(0x7, "io_disabled")),
	SUNXI_PIN(SUNXI_PINCTRL_PIN(F, 25),
		SUNXI_FUNCTION(0x5, "spi0"),
		SUNXI_FUNCTION(0x7, "io_disabled")),
	SUNXI_PIN(SUNXI_PINCTRL_PIN(F, 26),
		SUNXI_FUNCTION(0x5, "spi0"),
		SUNXI_FUNCTION(0x7, "io_disabled")),
	SUNXI_PIN(SUNXI_PINCTRL_PIN(F, 29),
		SUNXI_FUNCTION(0x5, "spi0"),
		SUNXI_FUNCTION(0x7, "io_disabled")),
	SUNXI_PIN(SUNXI_PINCTRL_PIN(F, 30),
		SUNXI_FUNCTION(0x5, "spi0"),
		SUNXI_FUNCTION(0x7, "io_disabled")),
	SUNXI_PIN(SUNXI_PINCTRL_PIN(F, 31),
		SUNXI_FUNCTION(0x5, "spi0"),
		SUNXI_FUNCTION(0x7, "io_disabled")),
#endif
	/* HOLE */
	//Register Name: PG_CFG0
	SUNXI_PIN(SUNXI_PINCTRL_PIN(G, 0),
		SUNXI_FUNCTION(0x0, "gpio_in"),
		SUNXI_FUNCTION(0x1, "gpio_out"),
		SUNXI_FUNCTION(0x2, "sdc1"),		/* CLK */
		SUNXI_FUNCTION_IRQ_BANK(0x6, 2, 0),	/* PG_EINT0 */
		SUNXI_FUNCTION(0x7, "io_disabled")),
	SUNXI_PIN(SUNXI_PINCTRL_PIN(G, 1),
		SUNXI_FUNCTION(0x0, "gpio_in"),
		SUNXI_FUNCTION(0x1, "gpio_out"),
		SUNXI_FUNCTION(0x2, "sdc1"),		/* CMD */
		SUNXI_FUNCTION_IRQ_BANK(0x6, 2, 1),	/* PG_EINT1 */
		SUNXI_FUNCTION(0x7, "io_disabled")),
	SUNXI_PIN(SUNXI_PINCTRL_PIN(G, 2),
		SUNXI_FUNCTION(0x0, "gpio_in"),
		SUNXI_FUNCTION(0x1, "gpio_out"),
		SUNXI_FUNCTION(0x2, "sdc1"),		/* D0 */
		SUNXI_FUNCTION_IRQ_BANK(0x6, 2, 2),	/* PG_EINT2 */
		SUNXI_FUNCTION(0x7, "io_disabled")),
	SUNXI_PIN(SUNXI_PINCTRL_PIN(G, 3),
		SUNXI_FUNCTION(0x0, "gpio_in"),
		SUNXI_FUNCTION(0x1, "gpio_out"),
		SUNXI_FUNCTION(0x2, "sdc1"),		/* D1 */
		SUNXI_FUNCTION_IRQ_BANK(0x6, 2, 3),	/* PG_EINT3 */
		SUNXI_FUNCTION(0x7, "io_disabled")),
	SUNXI_PIN(SUNXI_PINCTRL_PIN(G, 4),
		SUNXI_FUNCTION(0x0, "gpio_in"),
		SUNXI_FUNCTION(0x1, "gpio_out"),
		SUNXI_FUNCTION(0x2, "sdc1"),		/* D2 */
		SUNXI_FUNCTION_IRQ_BANK(0x6, 2, 4),	/* PG_EINT4 */
		SUNXI_FUNCTION(0x7, "io_disabled")),
	SUNXI_PIN(SUNXI_PINCTRL_PIN(G, 5),
		SUNXI_FUNCTION(0x0, "gpio_in"),
		SUNXI_FUNCTION(0x1, "gpio_out"),
		SUNXI_FUNCTION(0x2, "sdc1"),		/* D3 */
		SUNXI_FUNCTION_IRQ_BANK(0x6, 2, 5),	/* PG_EINT5 */
		SUNXI_FUNCTION(0x7, "io_disabled")),
	SUNXI_PIN(SUNXI_PINCTRL_PIN(G, 6),
		SUNXI_FUNCTION(0x0, "gpio_in"),
		SUNXI_FUNCTION(0x1, "gpio_out"),
		SUNXI_FUNCTION(0x2, "uart1"),		/* TX */
		SUNXI_FUNCTION(0x3, "twi0"),		/* SCK */
		SUNXI_FUNCTION(0x5, "vdevice"),		/* test */
		SUNXI_FUNCTION_IRQ_BANK(0x6, 2, 6),	/* PG_EINT6 */
		SUNXI_FUNCTION(0x7, "io_disabled")),
	SUNXI_PIN(SUNXI_PINCTRL_PIN(G, 7),
		SUNXI_FUNCTION(0x0, "gpio_in"),
		SUNXI_FUNCTION(0x1, "gpio_out"),
		SUNXI_FUNCTION(0x2, "uart1"),		/* RX */
		SUNXI_FUNCTION(0x3, "twi0"),		/* SDA */
		SUNXI_FUNCTION(0x5, "vdevice"),		/* test */
		SUNXI_FUNCTION_IRQ_BANK(0x6, 2, 7),	/* PG_EINT7 */
		SUNXI_FUNCTION(0x7, "io_disabled")),
	SUNXI_PIN(SUNXI_PINCTRL_PIN(G, 8),
		SUNXI_FUNCTION(0x0, "gpio_in"),
		SUNXI_FUNCTION(0x1, "gpio_out"),
		SUNXI_FUNCTION(0x2, "uart1"),		/* RTS */
		SUNXI_FUNCTION(0x3, "twi1"),		/* SCK */
		SUNXI_FUNCTION_IRQ_BANK(0x6, 2, 8),	/* PG_EINT8 */
		SUNXI_FUNCTION(0x7, "io_disabled")),
	SUNXI_PIN(SUNXI_PINCTRL_PIN(G, 9),
		SUNXI_FUNCTION(0x0, "gpio_in"),
		SUNXI_FUNCTION(0x1, "gpio_out"),
		SUNXI_FUNCTION(0x2, "uart1"),		/* CTS */
		SUNXI_FUNCTION(0x3, "twi1"),		/* SDA */
		SUNXI_FUNCTION_IRQ_BANK(0x6, 2, 9),	/* PG_EINT9 */
		SUNXI_FUNCTION(0x7, "io_disabled")),
	SUNXI_PIN(SUNXI_PINCTRL_PIN(G, 10),
		SUNXI_FUNCTION(0x0, "gpio_in"),
		SUNXI_FUNCTION(0x1, "gpio_out"),
		SUNXI_FUNCTION(0x3, "i2s1"),		/* MCLK */
		SUNXI_FUNCTION(0x4, "ledc"),		/* DO */
		SUNXI_FUNCTION_IRQ_BANK(0x6, 2, 10),	/* PG_EINT10 */
		SUNXI_FUNCTION(0x7, "io_disabled")),
	SUNXI_PIN(SUNXI_PINCTRL_PIN(G, 11),
		SUNXI_FUNCTION(0x0, "gpio_in"),
		SUNXI_FUNCTION(0x1, "gpio_out"),
		SUNXI_FUNCTION(0x2, "uart3"),		/* TX */
		SUNXI_FUNCTION(0x3, "i2s1"),		/* LRCK */
		SUNXI_FUNCTION(0x5, "spi1"),		/* SPI1_CS/DBI_CSX */
		SUNXI_FUNCTION_IRQ_BANK(0x6, 2, 11),	/* PG_EINT11 */
		SUNXI_FUNCTION(0x7, "io_disabled")),
	SUNXI_PIN(SUNXI_PINCTRL_PIN(G, 12),
		SUNXI_FUNCTION(0x0, "gpio_in"),
		SUNXI_FUNCTION(0x1, "gpio_out"),
		SUNXI_FUNCTION(0x2, "uart3"),		/* RX */
		SUNXI_FUNCTION(0x3, "i2s1"),		/* BLCK */
		SUNXI_FUNCTION(0x5, "spi1"),		/* SPI1_CLK/DBI_SCLK */
		SUNXI_FUNCTION_IRQ_BANK(0x6, 2, 12),	/* PG_EINT12 */
		SUNXI_FUNCTION(0x7, "io_disabled")),
	SUNXI_PIN(SUNXI_PINCTRL_PIN(G, 13),
		SUNXI_FUNCTION(0x0, "gpio_in"),
		SUNXI_FUNCTION(0x1, "gpio_out"),
		SUNXI_FUNCTION(0x2, "uart3"),		/* RTS */
		SUNXI_FUNCTION(0x3, "i2s1"),		/* DOUT0 */
		SUNXI_FUNCTION(0x4, "i2s1"),		/* DIN1 */
		SUNXI_FUNCTION(0x5, "spi1"),		/* SPI1_MOSI/DBI_SDO */
		SUNXI_FUNCTION_IRQ_BANK(0x6, 2, 13),	/* PG_EINT13 */
		SUNXI_FUNCTION(0x7, "io_disabled")),
	SUNXI_PIN(SUNXI_PINCTRL_PIN(G, 14),
		SUNXI_FUNCTION(0x0, "gpio_in"),
		SUNXI_FUNCTION(0x1, "gpio_out"),
		SUNXI_FUNCTION(0x2, "uart3"),		/* CTS */
		SUNXI_FUNCTION(0x3, "i2s1"),		/* DOUT1 */
		SUNXI_FUNCTION(0x4, "i2s1"),		/* DIN0 */
		SUNXI_FUNCTION(0x5, "spi1"),		/* SPI1_MISO/DBI_XX */
		SUNXI_FUNCTION_IRQ_BANK(0x6, 2, 14),	/* PG_EINT14 */
		SUNXI_FUNCTION(0x7, "io_disabled")),

	/* HOLE */
	//Register Name: PH_CFG0
	SUNXI_PIN(SUNXI_PINCTRL_PIN(H, 0),
		SUNXI_FUNCTION(0x0, "gpio_in"),
		SUNXI_FUNCTION(0x1, "gpio_out"),
		SUNXI_FUNCTION(0x2, "twi0"),		/* SCK */
		SUNXI_FUNCTION(0x3, "uart0"),		/* TX */
		SUNXI_FUNCTION(0x4, "spi1"),		/* MOSI/DBI_SDO */
		SUNXI_FUNCTION(0x5, "pwm0"),
		SUNXI_FUNCTION_IRQ_BANK(0x6, 3, 0),
		SUNXI_FUNCTION(0x7, "io_disabled")),
	SUNXI_PIN(SUNXI_PINCTRL_PIN(H, 1),
		SUNXI_FUNCTION(0x0, "gpio_in"),
		SUNXI_FUNCTION(0x1, "gpio_out"),
		SUNXI_FUNCTION(0x2, "twi0"),		/* SDA */
		SUNXI_FUNCTION(0x3, "uart0"),		/* RX */
		SUNXI_FUNCTION(0x4, "spi1"),		/* CLK/DBI_SCLK */
		SUNXI_FUNCTION(0x5, "pwm1"),
		SUNXI_FUNCTION_IRQ_BANK(0x6, 3, 1),
		SUNXI_FUNCTION(0x7, "io_disabled")),
	SUNXI_PIN(SUNXI_PINCTRL_PIN(H, 2),
		SUNXI_FUNCTION(0x0, "gpio_in"),
		SUNXI_FUNCTION(0x1, "gpio_out"),
		SUNXI_FUNCTION(0x2, "twi1"),		/* SCK */
		SUNXI_FUNCTION(0x3, "ledc"),		/* DO */
		SUNXI_FUNCTION(0x4, "spi1"),		/* CS/DBI_CSX */
		SUNXI_FUNCTION(0x5, "ir"),		/* RX */
		SUNXI_FUNCTION_IRQ_BANK(0x6, 3, 2),
		SUNXI_FUNCTION(0x7, "io_disabled")),
	SUNXI_PIN(SUNXI_PINCTRL_PIN(H, 3),
		SUNXI_FUNCTION(0x0, "gpio_in"),
		SUNXI_FUNCTION(0x1, "gpio_out"),
		SUNXI_FUNCTION(0x2, "twi1"),		/* SDA */
		SUNXI_FUNCTION(0x3, "spdif"),		/* OUT */
		SUNXI_FUNCTION(0x4, "spi1"),		/* MISO/DBI_XXX */
		SUNXI_FUNCTION(0x5, "ir"),		/* TX */
		SUNXI_FUNCTION_IRQ_BANK(0x6, 3, 3),
		SUNXI_FUNCTION(0x7, "io_disabled")),
	SUNXI_PIN(SUNXI_PINCTRL_PIN(H, 4),
		SUNXI_FUNCTION(0x0, "gpio_in"),
		SUNXI_FUNCTION(0x1, "gpio_out"),
		SUNXI_FUNCTION(0x2, "uart3"),		/* TX */
		SUNXI_FUNCTION(0x3, "spi1"),		/* CS/DBI_CSX */
		SUNXI_FUNCTION(0x4, "spi1"),		/* HOLD/DBI_DCX */
		SUNXI_FUNCTION(0x5, "pwm2"),
		SUNXI_FUNCTION_IRQ_BANK(0x6, 3, 4),
		SUNXI_FUNCTION(0x7, "io_disabled")),
	SUNXI_PIN(SUNXI_PINCTRL_PIN(H, 5),
		SUNXI_FUNCTION(0x0, "gpio_in"),
		SUNXI_FUNCTION(0x1, "gpio_out"),
		SUNXI_FUNCTION(0x2, "uart3"),		/* RX */
		SUNXI_FUNCTION(0x3, "spi1"),		/* CLK/DBI_SCLK */
		SUNXI_FUNCTION(0x4, "spi1"),		/* WP/DBI_TE */
		SUNXI_FUNCTION(0x5, "pwm3"),
		SUNXI_FUNCTION_IRQ_BANK(0x6, 3, 5),
		SUNXI_FUNCTION(0x7, "io_disabled")),
	SUNXI_PIN(SUNXI_PINCTRL_PIN(H, 6),
		SUNXI_FUNCTION(0x0, "gpio_in"),
		SUNXI_FUNCTION(0x1, "gpio_out"),
		SUNXI_FUNCTION(0x2, "uart3"),		/* RTS */
		SUNXI_FUNCTION(0x3, "spi1"),		/* MOSI/DBI_SDO */
		SUNXI_FUNCTION(0x4, "twi0"),		/* SCK */
		SUNXI_FUNCTION(0x5, "pwm4"),
		SUNXI_FUNCTION_IRQ_BANK(0x6, 3, 6),
		SUNXI_FUNCTION(0x7, "io_disabled")),
	SUNXI_PIN(SUNXI_PINCTRL_PIN(H, 7),
		SUNXI_FUNCTION(0x0, "gpio_in"),
		SUNXI_FUNCTION(0x1, "gpio_out"),
		SUNXI_FUNCTION(0x2, "uart3"),		/* CTS */
		SUNXI_FUNCTION(0x3, "spi1"),		/* MISO/DBI_XXX */
		SUNXI_FUNCTION(0x4, "twi0"),		/* SDA */
		SUNXI_FUNCTION(0x5, "pwm5"),
		SUNXI_FUNCTION_IRQ_BANK(0x6, 3, 7),
		SUNXI_FUNCTION(0x7, "io_disabled")),
	//Register Name: PH_CFG1
	SUNXI_PIN(SUNXI_PINCTRL_PIN(H, 8),
		SUNXI_FUNCTION(0x0, "gpio_in"),
		SUNXI_FUNCTION(0x1, "gpio_out"),
		SUNXI_FUNCTION(0x2, "twi1"),		/* SCK */
		SUNXI_FUNCTION(0x3, "spi1"),		/* HOLD/DBI_DCX */
		SUNXI_FUNCTION(0x4, "spdif"),		/* IN */
		SUNXI_FUNCTION(0x5, "ir"),		/* RX */
		SUNXI_FUNCTION_IRQ_BANK(0x6, 3, 8),
		SUNXI_FUNCTION(0x7, "io_disabled")),
	SUNXI_PIN(SUNXI_PINCTRL_PIN(H, 9),
		SUNXI_FUNCTION(0x0, "gpio_in"),
		SUNXI_FUNCTION(0x1, "gpio_out"),
		SUNXI_FUNCTION(0x2, "twi1"),		/* SDA */
		SUNXI_FUNCTION(0x3, "spi1"),		/* WP/DBI_TE */
		SUNXI_FUNCTION(0x4, "ledc"),		/* DO */
		SUNXI_FUNCTION(0x5, "ir"),		/* TX */
		SUNXI_FUNCTION_IRQ_BANK(0x6, 3, 9),
		SUNXI_FUNCTION(0x7, "io_disabled")),
};

static const unsigned sun50iw11p1_irq_bank_base[] = {
	SUNXI_PIO_BANK_BASE(PB_BASE, 0),
	SUNXI_PIO_BANK_BASE(PC_BASE, 1),
	SUNXI_PIO_BANK_BASE(PF_BASE, 2),
	SUNXI_PIO_BANK_BASE(PG_BASE, 3),
	SUNXI_PIO_BANK_BASE(PH_BASE, 4),
};

static const unsigned sun50iw11p1_bank_base[] = {
	SUNXI_PIO_BANK_BASE(PB_BASE, 0),
	SUNXI_PIO_BANK_BASE(PF_BASE, 1),
	SUNXI_PIO_BANK_BASE(PG_BASE, 2),
	SUNXI_PIO_BANK_BASE(PH_BASE, 3),
};

static const struct sunxi_pinctrl_desc sun50iw11p1_pinctrl_data = {
	.pins = sun50iw11p1_pins,
	.npins = ARRAY_SIZE(sun50iw11p1_pins),
	.pin_base = 0,
	.banks = ARRAY_SIZE(sun50iw11p1_bank_base),
	.bank_base = sun50iw11p1_bank_base,
	.irq_banks = ARRAY_SIZE(sun50iw11p1_irq_bank_base),
	.irq_bank_base = sun50iw11p1_irq_bank_base,
};

static int sun50iw11p1_pinctrl_probe(struct platform_device *pdev)
{
	return sunxi_pinctrl_init(pdev, &sun50iw11p1_pinctrl_data);
}

static struct of_device_id sun50iw11p1_pinctrl_match[] = {
	{ .compatible = "allwinner,sun50iw11p1-pinctrl", },
	{}
};
MODULE_DEVICE_TABLE(of, sun50iw11p1_pinctrl_match);

static struct platform_driver sun50iw11p1_pinctrl_driver = {
	.probe	= sun50iw11p1_pinctrl_probe,
	.driver	= {
		.name		= "sun50iw11p1-pinctrl",
		.owner		= THIS_MODULE,
		.pm		= &sunxi_pinctrl_pm_ops,
		.of_match_table	= sun50iw11p1_pinctrl_match,
	},
};

static int __init sun50iw11p1_pio_init(void)
{
	int ret;
	ret = platform_driver_register(&sun50iw11p1_pinctrl_driver);
	if (ret) {
		pr_err("register sun50iw11p1 pio controller failed\n");
		return -EINVAL;
	}
	return 0;
}
postcore_initcall(sun50iw11p1_pio_init);

MODULE_AUTHOR("Huangshuosheng<huangshuosheng@allwinnertech.com>");
MODULE_DESCRIPTION("Allwinner sun50iw10p1 pio pinctrl driver");
MODULE_LICENSE("GPL");
