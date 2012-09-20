/*
 * Copyright (C) 2010 LG Electronics Inc.
 * Copyright (C) 2009-2010 Texas Instruments Inc.
 * Mikkel Christensen <mlc@ti.com>
 * Felipe Balbi <balbi@ti.com>
 *
 * Modified from mach-omap2/board-ldp.c
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/input.h>
#include <linux/gpio.h>
#include <linux/i2c/twl.h>

#include <linux/ion.h>
#include <linux/memblock.h>

#include <asm/mach-types.h>
#include <asm/mach/arch.h>

#include <plat/common.h>
#include <plat/board.h>
#include <plat/usb.h>

#include <mach/board-hub.h>

#include "mux.h"
#include "sdram-hynix-h8mbx00u0mer-0em.h"

#include "pm.h"
/* LGE_CHANGE_S, [younggil.lee@lge.com], 2011-05-04, <add Setting enable Wifi Host wakeup> */
#include "board-hub-wifi.h"
/* LGE_CHANGE_E, [younggil.lee@lge.com], 2011-05-04, <add Setting enable Wifi Host wakeup> */
#include "omap_ion.h"
#include "omap_ram_console.h"

static void __init omap_hub_init_early(void)
{
	omap2_init_common_infrastructure();
	omap2_init_common_devices(h8mbx00u0mer0em_sdrc_params,
					  h8mbx00u0mer0em_sdrc_params);
}

#ifdef CONFIG_OMAP_MUX
static struct omap_board_mux board_mux[] __initdata = {
	{ .reg_offset = OMAP_MUX_TERMINATOR },
};
#endif

/*
 * cpuidle C-states definition override from the default values.
 * The 'exit_latency' field is the sum of sleep and wake-up latencies.
 */
static struct cpuidle_params hub_cpuidle_params[] = {
	/* C1 */
	{110 + 162, 5 , 1},
	/* C2 */
	{106 + 180, 309, 1},
	/* C3 */
	{107 + 410, 46057, 1},
	/* C4 */
	{121 + 3374, 46057, 0},
	/* C5 */
	{855 + 1146, 46057, 1},
	/* C6 */
	{7580 + 4134, 484329, 0},
	/* C7 */
	{7505 + 15274, 484329, 1},
};

static void __init omap_hub_init(void)
{
	omap3_mux_init(board_mux, OMAP_PACKAGE_CBP);
	omap3_pm_init_cpuidle(hub_cpuidle_params);
	/* LGE_CHANGE_S, [younggil.lee@lge.com], 2011-05-04, <add Setting enable Wifi Host wakeup> */
	config_wlan_mux();
	/* LGE_CHANGE_E, [younggil.lee@lge.com], 2011-05-04, <add Setting enable Wifi Host wakeup> */
	hub_peripherals_init();

	// ion settings
	omap_register_ion();
}

static void __init hub_reserve(void)
{
	/* LGE_CHANGE_E, [jinsu.park@lge.com], 2012-06-08, <add Setting enable ram console> */
	/* Remove the ram console region from kernel's map */
	memblock_remove(OMAP_RAM_CONSOLE_START_DEFAULT, OMAP_RAM_CONSOLE_SIZE_DEFAULT);

	/* do the static reservations first */
	memblock_remove(OMAP3_PHYS_ADDR_SMC_MEM, PHYS_ADDR_SMC_SIZE);

#ifdef CONFIG_ION_OMAP
	omap_ion_init();
#endif
	omap_reserve();
}

MACHINE_START(LGE_HUB, "LGE Hub board")
	.boot_params	= 0x80000100,
	.reserve	= hub_reserve,
	.map_io		= omap3_map_io,
	.init_early	= omap_hub_init_early,
	.init_irq	= omap_init_irq,
	.init_machine	= omap_hub_init,
	.timer		= &omap_timer,
MACHINE_END
