/*
 *  dp3t switch driver
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

#ifndef __SWITCH_DP3T_H__
#define __SWITCH_DP3T_H__

typedef enum {
	DP3T_NC,		// 0
	DP3T_AP_UART,	// 1
	DP3T_CP_UART,	// 2
	DP3T_CP_USB,	// 3
} TYPE_DP3T_MODE;

struct dp3t_switch_ctrl_combi {
	unsigned gpio1;
	unsigned gpio2;
};

struct dp3t_switch_platform_data {
	char *name;
	unsigned ctrl_gpio1;
	unsigned ctrl_gpio2;
	unsigned ctrl_ifx_vbus_gpio;
	struct dp3t_switch_ctrl_combi dp3t_ctrl_combi[4];
};

struct dp3t_switch {
	struct platform_device *pdev;
	unsigned ctrl_gpio1;
	unsigned ctrl_gpio2;
	unsigned ctrl_ifx_vbus_gpio;
	struct dp3t_switch_ctrl_combi dp3t_ctrl_combi[4];
};	

#endif
