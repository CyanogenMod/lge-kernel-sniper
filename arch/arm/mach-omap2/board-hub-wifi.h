/*
 * linux/arch/arm/mach-omap2/board-zoom2-wifi.h
 *
 * Copyright (C) 2010 Texas Instruments Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef _BOARD_HUB_WIFI_H
#define _BOARD_HUB_WIFI_H

#define HUB_WIFI_PMENA_GPIO	23
#define HUB_WIFI_IRQ_GPIO	24

/* LGE_CHANGE_S, [younggil.lee@lge.com], 2011-05-04, <add Setting enable Wifi Host wakeup> */
void config_wlan_mux(void);
/* LGE_CHANGE_E, [younggil.lee@lge.com], 2011-05-04, <add Setting enable Wifi Host wakeup> */

#endif /* _BOARD_ZOOM2_WIFI_H */
