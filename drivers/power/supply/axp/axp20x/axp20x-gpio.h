/*
 * drivers/power/supply/axp/axp20x/axp20x-gpio.h
 * (C) Copyright 2010-2016
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * caiyongheng <caiyongheng@allwinnertech.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 */

#ifndef AXP20X_GPIO_H
#define AXP20X_GPIO_H
#include "axp20x.h"

#define AXP_GPIO_IRQ_EDGE_RISING   (0x1<<7)
#define AXP_GPIO_IRQ_EDGE_FALLING  (0x1<<6)
#define AXP_GPIO_EDGE_TRIG_MASK    (AXP_GPIO_IRQ_EDGE_RISING | \
				AXP_GPIO_IRQ_EDGE_FALLING)

#define AXP_GPIO0_CFG              (AXP20X_GPIO0_CTL)     /* 0x90 */
#define AXP_GPIO1_CFG              (AXP20X_GPIO1_CTL)     /* 0x92 */
#define AXP_GPIO2_CFG              (AXP20X_GPIO2_CTL)     /* 0x93 */
#define AXP_GPIO3_CFG              (AXP20X_GPIO3_CTL)     /* 0x95 */
#define AXP_GPIO4_CFG              (AXP20X_OFF_CTL)       /* 0x32 */
#define AXP_GPIO012_STATE          (AXP20X_GPIO012_SIGNAL)/* 0x94 */
#define AXP_GPIO3_STATE            (AXP20X_GPIO3_CTL)     /* 0x94 */
#define AXP_GPIO0123_INTEN         (AXP20X_INTEN5)        /* 0x44 */
#define AXP_GPIO0123_INTSTA        (AXP20X_INTSTS5)       /* 0x4C */

#endif /* AXP20X_GPIO_H */
