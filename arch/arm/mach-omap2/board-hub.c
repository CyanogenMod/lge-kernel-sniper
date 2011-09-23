/*
 * Copyright (C) 2010 LG Electronics Inc.
 *
 * from board-hub.c
 *
 * Copyright (C) 2009 Texas Instruments Inc.
 *
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
#include <linux/delay.h>

#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/delay.h>

#include <mach/board-hub.h>

#include <plat/common.h>
#include <plat/board.h>
#include <plat/omap_device.h>
#include <plat/onenand.h>
#include <plat/usb.h>
#include <plat/opp_twl_tps.h>
#include <plat/timer-gp.h>

#include "mux.h"
#include "sdram-hynix-h8mbx00u0mer-0em.h"
#include "smartreflex-class1p5.h"
#include "pm.h"

/* LGE_CHANGE_S, [younggil.lee@lge.com], 2011-05-04, <add Setting enable Wifi Host wakeup> */
#include "board-hub-wifi.h"
/* LGE_CHANGE_E, [younggil.lee@lge.com], 2011-05-04, <add Setting enable Wifi Host wakeup> */


#ifndef CONFIG_LGE_SLEEP_HUB
//*************************************/
/* C1 - MPU WFI + Core active */
/* C2 - MPU WFI + Core inactive */
/* C3 - MPU CSWR + Core inactive */
/* C4 - MPU OFF + Core iactive */
/* C5 - MPU RET + Core RET */
/* C6 - MPU OFF + Core RET */
/* C7 - MPU OFF + Core OFF */
static struct cpuidle_params sniper_cpuidle_params[] = {
	/* C1 */
	{1, 110, 162, 5},
	/* C2 */
	{0, 106, 180, 309},
	/* C3 */
	{1, 107, 410, 46057},
	/* C4 */
	{0, 121, 3374, 46057},
	/* C5 */
	{1, 855, 1146, 46057},
	/* C6 */
	{0, 7580, 4134, 484329},
	/* C7 */
	{1, 7505, 15274, 484329},
};
#endif

#ifdef CONFIG_PM
static struct omap_volt_vc_data vc_config = {
	/* MPU */
	.vdd0_on	= 1200000, /* 1.2v */
	.vdd0_onlp	= 1000000, /* 1.0v */
	.vdd0_ret	=  975000, /* 0.975v */
	.vdd0_off	=  600000, /* 0.6v */
	/* CORE */
	.vdd1_on	= 1150000, /* 1.15v */
	.vdd1_onlp	= 1000000, /* 1.0v */
	.vdd1_ret	=  975000, /* 0.975v */
	.vdd1_off	=  600000, /* 0.6v */

#if 0 /* Original */
	.clksetup = 0xff,
	.voltoffset	= 0xff,
	.voltsetup2	= 0xff,
	.voltsetup_time1 = 0xfff,
	.voltsetup_time2 = 0xfff,
#else
	.clksetup	= 0x14A,
	.voltoffset	= 0x118,
	.voltsetup2	= 0x32,
	.voltsetup_time1 = 0x00B3,
	.voltsetup_time2 = 0x00A0,
#endif
};

#ifdef CONFIG_TWL4030_CORE
static struct omap_volt_pmic_info omap_pmic_mpu = { /* and iva */
	.name = "twl",
	.slew_rate = 4000,
	.step_size = 12500,
	.i2c_addr = 0x12,
	.i2c_vreg = 0x0, /* (vdd0) VDD1 -> VDD1_CORE -> VDD_MPU */
	.vsel_to_uv = omap_twl_vsel_to_uv,
	.uv_to_vsel = omap_twl_uv_to_vsel,
	.onforce_cmd = omap_twl_onforce_cmd,
	.on_cmd = omap_twl_on_cmd,
	.sleepforce_cmd = omap_twl_sleepforce_cmd,
	.sleep_cmd = omap_twl_sleep_cmd,
	.vp_config_erroroffset = 0,
	.vp_vstepmin_vstepmin = 0x01,
	.vp_vstepmax_vstepmax = 0x08, /* 20110812 dongyu.gwak@lge.com SmartReflex 0x4->0x8 */
	.vp_vlimitto_timeout_us = 0x200,
	.vp_vlimitto_vddmin = 0x14,
	.vp_vlimitto_vddmax = 0x44,
};

static struct omap_volt_pmic_info omap_pmic_core = {
	.name = "twl",
	.slew_rate = 4000,
	.step_size = 12500,
	.i2c_addr = 0x12,
	.i2c_vreg = 0x1, /* (vdd1) VDD2 -> VDD2_CORE -> VDD_CORE */
	.vsel_to_uv = omap_twl_vsel_to_uv,
	.uv_to_vsel = omap_twl_uv_to_vsel,
	.onforce_cmd = omap_twl_onforce_cmd,
	.on_cmd = omap_twl_on_cmd,
	.sleepforce_cmd = omap_twl_sleepforce_cmd,
	.sleep_cmd = omap_twl_sleep_cmd,
	.vp_config_erroroffset = 0,
	.vp_vstepmin_vstepmin = 0x01,
	.vp_vstepmax_vstepmax = 0x04,
	.vp_vlimitto_timeout_us = 0x200,
	.vp_vlimitto_vddmin = 0x13, /* 20110812 dongyu.gwak@lge.com 0x18->0x13*/
	.vp_vlimitto_vddmax = 0x42,
};
#endif /* CONFIG_TWL4030_CORE */
#endif /* CONFIG_PM */

static void __init omap_hub_map_io(void)
{
	omap2_set_globals_36xx();
	omap34xx_map_common_io();
}

static struct omap_board_config_kernel hub_config[] __initdata = {
};

#if defined(CONFIG_MTD_ONENAND_OMAP2) || \
	defined(CONFIG_MTD_ONENAND_OMAP2_MODULE)
static struct mtd_partition onenand_partitions[] = {
	{
		.name		= "x-loader",
		.offset		= 0,
		.size		= 2 * (64 * 4096),
		.mask_flags	= MTD_WRITEABLE,	/* force read-only */
	},
	{
		.name		= "u-boot",
		.offset		= MTDPART_OFS_APPEND,
		.size		= 16 * (64 * 4096),
		.mask_flags	= MTD_WRITEABLE,	/* force read-only */
	},
	{
		.name		= "kernel",
		.offset		= MTDPART_OFS_APPEND,
		.size		= 20 * (64 * 4096),
	},
	{
		.name		= "recovery",
		.offset		= MTDPART_OFS_APPEND,
		.size		= 20 * (64 * 4096),
	},
	{
		.name		= "lgdrm",
		.offset		= MTDPART_OFS_APPEND,
		.size		= 12 * (64 * 4096),
	},
	{
		.name		= "misc",
		.offset		= MTDPART_OFS_APPEND,
		.size		= 2 * (64 * 4096),
	},
	{
		.name		= "divxkey",
		.offset		= MTDPART_OFS_APPEND,
		.size		= 12 * (64 * 4096),
	},
	{
		.name		= "userdata",
		.offset		= MTDPART_OFS_APPEND,
		.size		= 2556 * (64 * 4096),
	},
	{
		.name		= "system",
		.offset		= MTDPART_OFS_APPEND,
		.size		= 1200 * (64 * 4096),
	},
	{
		.name		= "cache",
		.offset		= MTDPART_OFS_APPEND,
		.size		= 256 * (64 * 4096),
	},
};

static struct omap_onenand_platform_data board_onenand_data = {
	.cs		= 0,
	//.gpio_irq	= 63,
	.parts		= onenand_partitions,
	.nr_parts	= ARRAY_SIZE(onenand_partitions),
	.flags		= ONENAND_SYNC_READWRITE, // TODO: verify 20100616
	.dma_channel	= -1,   /* disable DMA in OMAP OneNAND driver */
};

static void __init hub_onenand_init(void)
{
	gpmc_onenand_init(&board_onenand_data);
}
#endif

static void __init omap_hub_init_irq(void)
{
	omap_board_config = hub_config;
	omap_board_config_size = ARRAY_SIZE(hub_config);
#ifndef CONFIG_LGE_SLEEP_HUB
	omap3_pm_init_cpuidle(sniper_cpuidle_params);
#endif
	omap2_init_common_hw(h8mbx00u0mer0em_sdrc_params,
			h8mbx00u0mer0em_sdrc_params);
#ifdef CONFIG_OMAP_32K_TIMER
	omap2_gp_clockevent_set_gptimer(1);
#endif
	omap_init_irq();
}

#ifdef CONFIG_OMAP_MUX
static struct omap_board_mux board_mux[] __initdata = {
	{ .reg_offset = OMAP_MUX_TERMINATOR },
};
#else
#define board_mux	NULL
#endif

/* OPP MPU/IVA Clock Frequency */
struct opp_frequencies {
	unsigned long mpu;
	unsigned long iva;
};

static struct opp_frequencies opp_freq_add_table[] __initdata = {
  {
	.mpu = 800000000,
	.iva = 660000000,
  },
  {
	.mpu = 1000000000,
	.iva =  800000000,
  },
  {
	.mpu = 1200000000,
	.iva =   65000000,
  },

  { 0, 0 },
};

static void __init omap_hub_init(void)
{
	omap3_mux_init(board_mux, OMAP_PACKAGE_CBP);
	/* LGE_CHANGE_S, [younggil.lee@lge.com], 2011-05-04, <add Setting enable Wifi Host wakeup> */
	config_wlan_mux();
	/* LGE_CHANGE_E, [younggil.lee@lge.com], 2011-05-04, <add Setting enable Wifi Host wakeup> */
	hub_peripherals_init();

#if defined(CONFIG_MTD_ONENAND_OMAP2) || \
	defined(CONFIG_MTD_ONENAND_OMAP2_MODULE)
	hub_onenand_init();
#endif
	sr_class1p5_init();

#ifdef CONFIG_PM
#ifdef CONFIG_TWL4030_CORE
	omap_voltage_register_pmic(&omap_pmic_core, "core");
	omap_voltage_register_pmic(&omap_pmic_mpu, "mpu");
#endif
	omap_voltage_init_vc(&vc_config);
#endif
}

/* must be called after omap2_common_pm_init() */
static int __init hub_opp_init(void)
{
	struct omap_hwmod *mh, *dh;
	struct omap_opp *mopp, *dopp;
	struct device *mdev, *ddev;
	struct opp_frequencies *opp_freq;


	if (!cpu_is_omap3630())
		return 0;

	mh = omap_hwmod_lookup("mpu");
	if (!mh || !mh->od) {
		pr_err("%s: no MPU hwmod device.\n", __func__);
		return 0;
	}

	dh = omap_hwmod_lookup("iva");
	if (!dh || !dh->od) {
		pr_err("%s: no DSP hwmod device.\n", __func__);
		return 0;
	}

	mdev = &mh->od->pdev.dev;
	ddev = &dh->od->pdev.dev;

	/* add MPU and IVA clock frequencies */
	for (opp_freq = opp_freq_add_table; opp_freq->mpu; opp_freq++) {
		/* check enable/disable status of MPU frequecy setting */
		mopp = opp_find_freq_exact(mdev, opp_freq->mpu, false);
		if (IS_ERR(mopp))
			mopp = opp_find_freq_exact(mdev, opp_freq->mpu, true);
		if (IS_ERR(mopp)) {
			pr_err("%s: MPU does not support %lu MHz\n", __func__, opp_freq->mpu / 1000000);
			continue;
		}

		/* check enable/disable status of IVA frequency setting */
		dopp = opp_find_freq_exact(ddev, opp_freq->iva, false);
		if (IS_ERR(dopp))
			dopp = opp_find_freq_exact(ddev, opp_freq->iva, true);
		if (IS_ERR(dopp)) {
			pr_err("%s: DSP does not support %lu MHz\n", __func__, opp_freq->iva / 1000000);
			continue;
		}

		/* try to enable MPU frequency setting */
		if (opp_enable(mopp)) {
			pr_err("%s: OPP cannot enable MPU:%lu MHz\n", __func__, opp_freq->mpu / 1000000);
			continue;
		}

		/* try to enable IVA frequency setting */
		if (opp_enable(dopp)) {
			pr_err("%s: OPP cannot enable DSP:%lu MHz\n", __func__, opp_freq->iva / 1000000);
			opp_disable(mopp);
			continue;
		}

		/* verify that MPU and IVA frequency settings are available */
		mopp = opp_find_freq_exact(mdev, opp_freq->mpu, true);
		dopp = opp_find_freq_exact(ddev, opp_freq->iva, true);
		if (!mopp || !dopp) {
			pr_err("%s: OPP requested MPU: %lu MHz and DSP: %lu MHz not found\n",
				__func__, opp_freq->mpu / 1000000, opp_freq->iva / 1000000);
			continue;
		}

		dev_info(mdev, "OPP enabled %lu MHz\n", opp_freq->mpu / 1000000);
		dev_info(ddev, "OPP enabled %lu MHz\n", opp_freq->iva / 1000000);
	}

	return 0;
}
device_initcall(hub_opp_init);

MACHINE_START(LGE_HUB, "LGE Sniper board")
	.phys_io	= 0x48000000, /* L4_34XX_PHYS */
	.io_pg_offst	= ((0xfa000000 /* L4_34XX_VIRT */) >> 18) & 0xfffc,
	.boot_params	= 0x80000100,
	.map_io		= omap_hub_map_io,
	.init_irq	= omap_hub_init_irq,
	.init_machine	= omap_hub_init,
	.timer		= &omap_timer,
MACHINE_END
