/*
 * drivers/media/video/rt8515.h
 *
 * Register definitions for the RT8515 Flash Chip.
 *
 * Author:Pallavi Kulkarni (p-kulkarni@ti.com)
 *
 * Copyright (C) 2009 Texas Instruments.
 *
 * This file is licensed under the terms of the GNU General Public License
 * version 2. This program is licensed "as is" without any warranty of any
 * kind, whether express or implied.
 */

#ifndef __RT8515_H__
#define __RT8515_H__

#include <linux/videodev2.h>
#include <mach/isp_user.h>
#include <media/v4l2-int-device.h>

#define RT8515_NAME				"rt8515"

#define RT8515_HW_FLASH_IS_ON 1
#define RT8515_HW_FLASH_MODE  1<<1

struct rt8515 {
	const struct rt8515_platform_data *pdata;
	u32 flash_timeout;
	u8	turned_on;
	u8	torch_intensity;
	u8	flash_intensity;	
	struct v4l2_int_device *v4l2_int_device;
	bool dev_init_done;
	spinlock_t lock; // needed for the c2s transaction
	enum v4l2_power power;		/* Requested power state */
};


/**
 * struct rt8515_platform_data - platform data values and access functions
 *
**/

enum rt8515_intensity_values{
	RT8515_TORCH_INT_0_PERCENT,
	RT8515_TORCH_INT_20_PERCENT,
	RT8515_TORCH_INT_22_4_PERCENT,
	RT8515_TORCH_INT_25_1_PERCENT,
	RT8515_TORCH_INT_28_2_PERCENT,
	RT8515_TORCH_INT_31_6_PERCENT,
	RT8515_TORCH_INT_35_5_PERCENT,
	RT8515_TORCH_INT_39_8_PERCENT,
	RT8515_TORCH_INT_44_7_PERCENT,
	RT8515_TORCH_INT_50_PERCENT,
	RT8515_TORCH_INT_56_PERCENT,
	RT8515_TORCH_INT_63_PERCENT,
	RT8515_TORCH_INT_71_PERCENT,
	RT8515_TORCH_INT_79_PERCENT,
	RT8515_TORCH_INT_89_PERCENT,
	RT8515_TORCH_INT_100_PERCENT,	
};

/**
 * struct rt8515_platform_data - platform data values and access functions
 * @power_set: Power state access function, zero is off, non-zero is on.
 * @flash_on: Turn on the flash.
 * @flash_off: Turn off the flash.
 * @update_hw: Depending on the torch intensity, turn on/off torch.
 * @priv_data_set: device private data (pointer) access function
 */
struct rt8515_platform_data {
	int (*init)(void);
	int (*exit)(void);
	void (*flash_on)(void);
	void (*torch_on)(u8 data);	
	void (*flash_off)(unsigned long dummy);
	int (*update_hw)(struct v4l2_int_device *s);
	int (*priv_data_set)(void *);
};

#endif
