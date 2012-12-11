/*
 *  MUIC class driver
 *
 * Copyright (C) 2011 LG Electronics, Inc.
 * Author: Seungho Park <seungho1.park@lge.com>
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

#ifndef __LINUX_MUIC_H__
#define __LINUX_MUIC_H__

#include <linux/switch.h>
#include <linux/wakelock.h>
#include <linux/i2c.h>
#include <linux/muic/muic_client.h>

typedef enum {
	MUIC_UNINITED = -1,
	MUIC_UNKNOWN = 0,	// 0 - Error in detection or unsupported accessory.
	MUIC_NONE,		// 1 - No accessory is plugged in.
	MUIC_NA_TA,		// 2 - Not used actually. Special TA for North America.
	MUIC_LG_TA,		// 3
	MUIC_TA_1A,		// 4
	MUIC_INVALID_CHG,	// 5
	MUIC_AP_UART,		// 6
	MUIC_CP_UART,		// 7
	MUIC_AP_USB,		// 8
	MUIC_CP_USB,		// 9
	MUIC_TV_OUT_NO_LOAD,	// 10 - Not used.
	MUIC_EARMIC,		// 11
	MUIC_TV_OUT_LOAD,	// 12 - Not used.
	MUIC_OTG,		// 13 - Not used.
	//!![S] 2011-07-05 by pilsu.kim@lge.com : 
	MUIC_MHL,		// 14
	//!![E] 2011-07-05 by pilsu.kim@lge.com :
//#if	defined(CONFIG_PRODUCT_LGE_LU6800) || defined(CONFIG_PRODUCT_LGE_KU5900)
/* [LGE_CHANGE] 20120908 pyocool.cho@lge.com "for p970" */
#if	defined(CONFIG_PRODUCT_LGE_LU6800) || defined(CONFIG_PRODUCT_LGE_KU5900) || defined(CONFIG_PRODUCT_LGE_P970)
	MUIC_CP_DOWNLOAD,	// 15
	MUIC_ILLEGAL_CHG,	//16
	MUIC_RESERVE1,		// 17
	MUIC_MODE_NO,		// 18	
#else
	MUIC_RESERVE1,		// 15
	MUIC_RESERVE2,		// 16
	MUIC_RESERVE3,		// 17
	MUIC_MODE_NO,		// 18
#endif	
} TYPE_MUIC_MODE;

typedef enum {
	CHARGING_UNKNOWN,
	CHARGING_NONE,
	CHARGING_NA_TA,
	CHARGING_LG_TA,		// MUIC_LG_TA
	CHARGING_TA_1A,
	CHARGING_INVALID_CHG,
// LGE_CHANGE [euiseop.shin@lge.com] 2011-04-13, LGE_P940, add Charger Mode for P940 [START_LGE]
	CHARGING_UART,		// MUIC_AP_UART
	CHARGING_RESERVE1,	// dummy
// LGE_CHANGE [euiseop.shin@lge.com] 2011-04-13, LGE_P940, add Charger Mode for P940 [END_LGE]
	CHARGING_USB,		// MUIC_AP_USB
	CHARGING_FACTORY,
} TYPE_CHARGING_MODE;

typedef enum {
	NOT_UPON_IRQ,	// 0
	UPON_IRQ,	// 1
} TYPE_UPON_IRQ;

typedef enum {
	DEFAULT,	/* 0 - Just apply the default register settings. */
	RESET,		/* 1 - Fully reset the MUIC. It takes 250msec. */
	BOOTUP,		/* 2 - TSU5611 BUG fix */
} TYPE_RESET;

typedef enum {
	NO_RETAIN,
	BOOT_CP_USB = 1,
	BOOT_AP_USB = 2,
	BOOT_RECOVERY = 3,
	BOOT_MHL = 4,
} TYPE_RETAIN_MODE;

struct muic_device {
	const char	*name;
	struct device	*dev;
	void (*event_handler)(struct muic_device *);
	int (*read_int_state)(struct muic_device *, char *); 
	unsigned int mode;
	int		index;
};

struct muic_ops {
	void (*irq_handler)(struct muic_device *);
};

/* LGE_SJIT 2012-01-27 [dojip.kim@lge.com] Add platform_data */
struct muic_platform_data {
	int gpio_int;
	int gpio_mhl;      /* MHL specific codes */
	int gpio_ifx_vbus; /* MHL specific codes */
};

extern int muic_device_register(struct muic_device *mdev, struct muic_ops *ops);
extern void muic_device_unregister(struct muic_device *mdev);

TYPE_MUIC_MODE muic_get_mode(void);
int muic_set_mode(TYPE_MUIC_MODE mode);
int muic_register_client(struct muic_client_device *);
s32 muic_i2c_read_byte(struct i2c_client *client, u8 addr, u8 *value);
s32 muic_i2c_write_byte(struct i2c_client *client, u8 addr, u8 value);

#endif /* __LINUX_SWITCH_H__ */
