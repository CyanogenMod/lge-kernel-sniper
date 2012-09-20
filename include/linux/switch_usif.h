/*
 *  usif switch driver
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

#ifndef __SWITCH_USIF_H__
#define __SWITCH_USIF_H__

typedef enum {
	USIF_AP,		// 0
	USIF_DP3T,		// 1
} TYPE_USIF_MODE;
//USIF can't switch
typedef enum {
	USIF_CTRL_NOK,		// 0
	USIF_CTRL_OK,		// 1
} TYPE_USIF_CTRL;

//USIF can't switch
struct usif_switch_platform_data {
	char *name;
	unsigned ctrl_gpio;
};

struct usif_switch {
	struct platform_device *pdev;
	unsigned ctrl_gpio;
};

#endif
