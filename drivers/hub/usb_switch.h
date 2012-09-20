/*
 * The only header file for Hub TI TS5USBA33402 MUIC driver
 *
 * Copyright (C) 2010 LGE, Inc.
 *
 * Author: Sookyoung Kim <sookyoung.kim@lge.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */

#ifndef __USB_SWITCH_H__
#define __USB_SWITCH_H__

#define USB_MDM_SW	161	
#define MDM_USB_VBUS_EN	101	

typedef enum {
	MUIC_UNKNOWN,		// 0 - Error in detection or unsupported accessory.
	MUIC_NONE,		// 1 - No accessory is plugged in.
	MUIC_NA_TA,		// 2 - Not used actually. Special TA for North America.
	MUIC_LG_TA,		// 3
	MUIC_HCHH,		// 4 - Not used actually.
	MUIC_INVALID_CHG,	// 5
	MUIC_AP_UART,		// 6
	MUIC_CP_UART,		// 7
	MUIC_AP_USB,		// 8
	MUIC_CP_USB,		// 9 - Not defined yet.
	MUIC_TV_OUT_NO_LOAD,	// 10 - Not used.
	MUIC_EARMIC,		// 11
	MUIC_TV_OUT_LOAD,	// 12 - Not used.
	MUIC_OTG,		// 13 - Not used.
	MUIC_RESERVE1,		// 14
	MUIC_RESERVE2,		// 15
	MUIC_RESERVE3,		// 16
	MUIC_MODE_NO,		// 17
} TYPE_MUIC_MODE;

TYPE_MUIC_MODE get_muic_mode(void);

typedef enum {
	DP3T_NC,	// 0
	DP3T_AP_UART,	// 1
	DP3T_CP_UART,	// 2
	DP3T_CP_USB,	// 3
} TYPE_DP3T_MODE;

#endif /* __USB_SWITCH_H__  */
