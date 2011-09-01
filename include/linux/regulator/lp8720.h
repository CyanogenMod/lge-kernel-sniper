/*
 * Regulator driver for National Semiconductors LP8720 chip
 *
 *  Copyright (C) 2009 LG Electronics
 *  Author: 
 *
 * 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#ifndef _LINUX_LP8720_H
#define _LINUX_LP8720_H

#define LP8720_I2C_NAME		"lp8720"
#define LP8720_I2C_ADDR		0x7D

struct lp8720_platform_data {
	int en_gpio_num;
};

typedef enum {
	LDO1	= 0,
	LDO2,
	LDO3,
	LDO4,
	LDO5,
	SWREG
} subpm_output_enum;


#endif /* _LINUX_LP8720_H */

