/*
 * linux/arch/arm/mach-omap2/board-sniper-mmc.c
 *
 * Copyright (C) 2007-2008 Texas Instruments
 * Copyright (C) 2008 Nokia Corporation
 * Author: Texas Instruments
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/err.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/mmc/host.h>
#include <linux/regulator/consumer.h>
#include <plat/cpu.h>

#include <mach/hardware.h>
#include <plat/control.h>
#include <plat/mmc.h>
#include <plat/board.h>
#include <plat/omap-pm.h>
#include <linux/slab.h>

#ifdef CONFIG_MMC_EMBEDDED_SDIO
#include <linux/mmc/sdio_ids.h>
#include <linux/mmc/sdio_func.h>
#endif

#include "mmc-twl4030.h"
#include <linux/i2c/twl.h>

#if defined(CONFIG_REGULATOR) && \
	(defined(CONFIG_MMC_OMAP_HS) || defined(CONFIG_MMC_OMAP_HS_MODULE))

#if defined(CONFIG_REGULATOR_LP8720)
#include <linux/regulator/lp8720.h>
#endif

static u16 control_pbias_offset;
static u16 control_devconf1_offset;

#define HSMMC_NAME_LEN		9
#define PHOENIX_MMC_CTRL	0xEE

/* Hack :  Phoenix registers*/

#define PHOENIX_CFG_INPUT_PUPD3	0xF2
//#define TI_FS_MMC // 0 = working for B, 1 = working for TI


#if defined(CONFIG_REGULATOR_LP8720)
extern void subpm_set_output(subpm_output_enum outnum, int onoff);
extern void subpm_output_enable(void);
#endif


static struct twl_mmc_controller {
	struct omap_mmc_platform_data	*mmc;
	/* Vcc == configured supply
	 * Vcc_alt == optional
	 *   -	MMC1, supply for DAT4..DAT7
	 *   -	MMC2/MMC2, external level shifter voltage supply, for
	 *	chip (SDIO, eMMC, etc) or transceiver (MMC2 only)
	 */
	struct regulator		*vcc;
	struct regulator		*vcc_aux;
	char				name[HSMMC_NAME_LEN + 1];
} hsmmc[OMAP34XX_NR_MMC];

static int twl_mmc_card_detect(int irq)
{
	unsigned i;

	for (i = 0; i < ARRAY_SIZE(hsmmc); i++) {
		struct omap_mmc_platform_data *mmc;

		mmc = hsmmc[i].mmc;
		if (!mmc)
			continue;
		if (irq != mmc->slots[0].card_detect_irq)
			continue;

		/* NOTE: assumes card detect signal is active-low */
		return !gpio_get_value_cansleep
				(mmc->slots[0].switch_pin);
	}
	return -ENOSYS;
}

static int twl_mmc_get_ro(struct device *dev, int slot)
{
	struct omap_mmc_platform_data *mmc = dev->platform_data;

	/* NOTE: assumes write protect signal is active-high */
	return gpio_get_value_cansleep(mmc->slots[0].gpio_wp);
}

static int twl_mmc_get_cover_state(struct device *dev, int slot)
{
	struct omap_mmc_platform_data *mmc = dev->platform_data;

	/* NOTE: assumes card detect signal is active-low */
	return !gpio_get_value_cansleep(mmc->slots[0].switch_pin);
}

/*
 * MMC Slot Initialization.
 */
static int twl_mmc_late_init(struct device *dev)
{
	struct omap_mmc_platform_data *mmc = dev->platform_data;
	int ret = 0;
	int i;

#if 0
	/* MMC/SD/SDIO doesn't require a card detect switch */
	if (gpio_is_valid(mmc->slots[0].switch_pin)) {
		ret = gpio_request(mmc->slots[0].switch_pin, "mmc_cd");
		if (ret)
			goto done;
		ret = gpio_direction_input(mmc->slots[0].switch_pin);
		if (ret)
			goto err;
	}
#endif
	/* require at least main regulator */
	for (i = 0; i < ARRAY_SIZE(hsmmc); i++) {
		if (hsmmc[i].name == mmc->slots[0].name) {
			struct regulator *reg;

			hsmmc[i].mmc = mmc;

			switch (i) {
			case 0:
				mmc->slots[0].ocr_mask = MMC_VDD_29_30;
				break;
			case 1:
				reg = regulator_get(dev, "vmmc2");
				if (IS_ERR(reg)) {
					ret = PTR_ERR(reg);
					hsmmc[i].vcc = NULL;
					goto err;
				}
				hsmmc[i].vcc = reg;
				mmc->slots[0].ocr_mask = MMC_VDD_29_30;
				break;
			case 2:
				mmc->slots[0].ocr_mask = MMC_VDD_29_30;
				break;
			default:
				break;
			}
		}
	}

	return 0;

err:
	gpio_free(mmc->slots[0].switch_pin);
done:
	mmc->slots[0].card_detect_irq = 0;
	mmc->slots[0].card_detect = NULL;

	dev_err(dev, "err %d configuring card detect\n", ret);
	return ret;
}

static void twl_mmc_cleanup(struct device *dev)
{
	struct omap_mmc_platform_data *mmc = dev->platform_data;
	int i;

	gpio_free(mmc->slots[0].switch_pin);
	for(i = 0; i < ARRAY_SIZE(hsmmc); i++) {
		regulator_put(hsmmc[i].vcc);
		regulator_put(hsmmc[i].vcc_aux);
	}
}

#ifdef CONFIG_PM

static int twl_mmc_suspend(struct device *dev, int slot)
{
	struct omap_mmc_platform_data *mmc = dev->platform_data;

	disable_irq(mmc->slots[0].card_detect_irq);
	return 0;
}

static int twl_mmc_resume(struct device *dev, int slot)
{
	struct omap_mmc_platform_data *mmc = dev->platform_data;

	enable_irq(mmc->slots[0].card_detect_irq);
	return 0;
}

#else
#define twl_mmc_suspend	NULL
#define twl_mmc_resume	NULL
#endif

#if defined(CONFIG_ARCH_OMAP3) && defined(CONFIG_PM)

static unsigned twl4030_mmc_get_context_loss(struct device *dev)
{
	return omap_pm_get_dev_context_loss_count(dev);
}

#else
#define twl4030_mmc_get_context_loss NULL
#endif

/*LGE_CHANGE_S*/
static int twl_mmc1_set_power(struct device *dev, int slot, int power_on,
				int vdd)
{
	u32 reg, prog_io;
	int ret = 0;
	struct omap_mmc_platform_data *mmc = dev->platform_data;

	pr_debug("%s %s power_on:%d", __func__, mmc->slots[0].name, power_on);

	if (power_on) {
		if (mmc->slots[0].internal_clock) {
			reg = omap_ctrl_readl(OMAP2_CONTROL_DEVCONF0);
			reg |= OMAP2_MMCSDIO1ADPCLKISEL;
			omap_ctrl_writel(reg, OMAP2_CONTROL_DEVCONF0);
		}

		reg = omap_ctrl_readl(control_pbias_offset);
		/* Set MMC I/O to 52Mhz */
		prog_io = omap_ctrl_readl
				(OMAP343X_CONTROL_PROG_IO1);
		prog_io |= OMAP3630_PRG_SDMMC1_SPEEDCTRL;
		omap_ctrl_writel
			(prog_io, OMAP343X_CONTROL_PROG_IO1);
		reg &= ~OMAP2_PBIASLITEPWRDNZ0;

		omap_ctrl_writel(reg, control_pbias_offset);

		reg = omap_ctrl_readl(control_pbias_offset);
		if ((1 << vdd) <= MMC_VDD_165_195)
			reg &= ~OMAP2_PBIASLITEVMODE0;
		else
			reg |= OMAP2_PBIASLITEVMODE0;
		omap_ctrl_writel(reg, control_pbias_offset);

		subpm_set_output(LDO1, 1);
		subpm_output_enable();

		/* 1ms delay required for PBIAS configuration */
		msleep(1);
		reg = omap_ctrl_readl(control_pbias_offset);
		reg |= (OMAP2_PBIASLITEPWRDNZ0 |
					OMAP2_PBIASSPEEDCTRL0);
		omap_ctrl_writel(reg, control_pbias_offset);
	} else {
		reg = omap_ctrl_readl(control_pbias_offset);
		reg &= ~OMAP2_PBIASLITEPWRDNZ0;
		omap_ctrl_writel(reg, control_pbias_offset);

		subpm_set_output(LDO1, 0);
		subpm_output_enable();

		/* 100ms delay required for PBIAS configuration */
		msleep(10);
		reg = omap_ctrl_readl(control_pbias_offset);
		reg |= (OMAP2_PBIASSPEEDCTRL0 | OMAP2_PBIASLITEPWRDNZ0
					| OMAP2_PBIASLITEVMODE0);
		omap_ctrl_writel(reg, control_pbias_offset);
	}

	return ret;
}

static int twl_mmc2_set_power(struct device *dev, int slot, int power_on,
				int vdd)
{
	int ret = 0;
	struct twl_mmc_controller *c = &hsmmc[1];
	struct omap_mmc_platform_data *mmc = dev->platform_data;
	int mode;

	pr_debug("%s %s power_on:%d", __func__, c->name, power_on);

	if (power_on) {
		/* only MMC2 supports a CLKIN */
		if (mmc->slots[0].internal_clock) {
			u32 reg;

			reg = omap_ctrl_readl(control_devconf1_offset);
			reg |= OMAP2_MMCSDIO2ADPCLKISEL;
			omap_ctrl_writel(reg, control_devconf1_offset);
		}
	}

#if 0
	mode = power_on ? regulator_enable(c->vcc): regulator_disable(c->vcc);
#else
	mode = power_on ? REGULATOR_MODE_NORMAL : REGULATOR_MODE_STANDBY;
	ret = regulator_set_mode(c->vcc, mode);
#endif

	return ret;
}

static int twl_mmc3_set_power(struct device *dev, int slot, int power_on, int vdd)
{
	struct omap_mmc_platform_data *mmc = dev->platform_data;
	pr_debug("%s %s power_on:%d", __func__, mmc->slots[0].name, power_on);
	return 0;
}

static int twl_mmc1_set_sleep(struct device *dev, int slot, int sleep, int vdd,
			      int cardsleep)
{
	struct omap_mmc_platform_data *mmc = dev->platform_data;
	pr_debug("%s %s sleep:%d", __func__, mmc->slots[0].name, sleep);
	return 0;
}

static int twl_mmc2_set_sleep(struct device *dev, int slot, int sleep, int vdd,
			       int cardsleep)
{
	struct omap_mmc_platform_data *mmc = dev->platform_data;
	pr_debug("%s %s sleep:%d", __func__, mmc->slots[0].name, sleep);
	return 0;
}
/*LGE_CHANGE_E*/

static int twl_mmc3_set_sleep(struct device *dev, int slot, int sleep, int vdd,
			       int cardsleep)
{
	struct omap_mmc_platform_data *mmc = dev->platform_data;
	pr_debug("%s %s sleep:%d", __func__, mmc->slots[0].name, sleep);
	return 0;
}

static struct omap_mmc_platform_data *hsmmc_data[OMAP34XX_NR_MMC] __initdata;

#ifdef CONFIG_MMC_EMBEDDED_SDIO
static struct sdio_embedded_func wifi_func_array[] = {
	{
		.f_class        = SDIO_CLASS_BT_A,
		.f_maxblksize   = 512,
	},
	{
		.f_class        = SDIO_CLASS_WLAN,
		.f_maxblksize   = 512,
	},
};

static struct embedded_sdio_data omap_wifi_emb_data = {
	.cis    = {
		//.vendor         = SDIO_VENDOR_ID_TI,
		//.device         = SDIO_DEVICE_ID_TI_WL12xx,
		.vendor		  = SDIO_VENDOR_ID_BRCM,
		.device 		= SDIO_DEVICE_ID_BCM4329,
		.blksize        = 512,
		//.max_dtr        = 24000000,
		.max_dtr        = 50000000,
	},
	.cccr   = {
		.multi_block    = 1,
		.low_speed      = 0,
		.wide_bus       = 1,
		.high_power     = 0,
		//.high_speed     = 0,
		.high_speed     = 1,		
		.disable_cd = 1,
	},
	.funcs  = wifi_func_array,
//	.num_funcs = 2,
	.quirks = MMC_QUIRK_VDD_165_195 | MMC_QUIRK_LENIENT_FUNC0,
};
#endif

void __init twl4030_mmc_init(struct omap2_hsmmc_info *controllers)
{
	struct omap2_hsmmc_info *c;
	int nr_hsmmc = ARRAY_SIZE(hsmmc_data);
	int i;
	u32 reg;
	int controller_cnt = 0;

	control_pbias_offset = OMAP343X_CONTROL_PBIAS_LITE;
	control_devconf1_offset = OMAP343X_CONTROL_DEVCONF1;

	for (c = controllers; c->mmc; c++) {
		struct twl_mmc_controller *hc = hsmmc + controller_cnt;
		struct omap_mmc_platform_data *mmc = hsmmc_data[controller_cnt];
		if (!c->mmc || c->mmc > nr_hsmmc) {
			pr_debug("MMC%d: no such controller\n", c->mmc);
			continue;
		}
		if (mmc) {
			pr_debug("MMC%d: already configured\n", c->mmc);
			continue;
		}

		mmc = kzalloc(sizeof(struct omap_mmc_platform_data),
			      GFP_KERNEL);
		if (!mmc) {
			pr_err("Cannot allocate memory for mmc device!\n");
			goto done;
		}

		if (c->name)
			strncpy(hc->name, c->name, HSMMC_NAME_LEN);
		else
			snprintf(hc->name, ARRAY_SIZE(hc->name),
				"mmc%islot%i", c->mmc, 1);

#ifdef CONFIG_MMC_EMBEDDED_SDIO
		if (c->mmc == CONFIG_TIWLAN_MMC_CONTROLLER) {
			mmc->slots[0].embedded_sdio = &omap_wifi_emb_data;
			mmc->slots[0].register_status_notify =
				&omap_wifi_status_register;
			mmc->slots[0].card_detect = &omap_wifi_status;
		}
#elif defined(CONFIG_TIWLAN_SDIO)
		if (c->mmc == CONFIG_TIWLAN_MMC_CONTROLLER)
			mmc->name = "TIWLAN_SDIO";
#endif
		mmc->slots[0].name = hc->name;
		mmc->nr_slots = 1;
		mmc->slots[0].caps = c->caps;
		mmc->slots[0].internal_clock = !c->ext_clock;
		mmc->dma_mask = 0xffffffff;
		mmc->init = twl_mmc_late_init;

		/* Register offset Mapping */
		if (cpu_is_omap44xx())
			mmc->regs_map = (u16 *) omap4_mmc_reg_map;
		else
			mmc->regs_map = (u16 *) omap3_mmc_reg_map;

		if (!cpu_is_omap44xx())
			mmc->get_context_loss_count = twl4030_mmc_get_context_loss;

		/* note: twl4030 card detect GPIOs can disable VMMCx ... */
		if (gpio_is_valid(c->gpio_cd)) {
			mmc->cleanup = twl_mmc_cleanup;
			mmc->suspend = twl_mmc_suspend;
			mmc->resume = twl_mmc_resume;

			mmc->slots[0].switch_pin = c->gpio_cd;
			mmc->slots[0].card_detect_irq =
						gpio_to_irq(c->gpio_cd);
			if (c->cover_only)
				mmc->slots[0].get_cover_state =
						twl_mmc_get_cover_state;
			else
				mmc->slots[0].card_detect =
						twl_mmc_card_detect;
		} else
			mmc->slots[0].switch_pin = -EINVAL;

		mmc->slots[0].remux = c->remux;

		/* write protect normally uses an OMAP gpio */
		if (gpio_is_valid(c->gpio_wp)) {
			gpio_request(c->gpio_wp, "mmc_wp");
			gpio_direction_input(c->gpio_wp);

			mmc->slots[0].gpio_wp = c->gpio_wp;
			mmc->slots[0].get_ro = twl_mmc_get_ro;
		} else
			mmc->slots[0].gpio_wp = -EINVAL;

		if (c->cover_only)
			mmc->slots[0].cover = 1;

		if (c->nonremovable)
			mmc->slots[0].nonremovable = 1;

		if (c->power_saving)
			mmc->slots[0].power_saving = 1;

		if (c->no_off)
			mmc->slots[0].no_off = 1;

		if (c->vcc_aux_disable_is_sleep)
			mmc->slots[0].vcc_aux_disable_is_sleep = 1;

		/* NOTE:  MMC slots should have a Vcc regulator set up.
		 * This may be from a TWL4030-family chip, another
		 * controllable regulator, or a fixed supply.
		 *
		 * temporary HACK: ocr_mask instead of fixed supply
		 */
		mmc->slots[0].ocr_mask = c->ocr_mask;

		switch (c->mmc) {
		case 1:
			/* on-chip level shifting via PBIAS0/PBIAS1 */
			mmc->slots[0].set_power = twl_mmc1_set_power;
			mmc->slots[0].set_sleep = twl_mmc1_set_sleep;

			/* Omap3630 HSMMC1 supports only 4-bit */
			if (cpu_is_omap3630() &&
					(c->caps & MMC_CAP_8_BIT_DATA)) {
				c->caps &= ~MMC_CAP_8_BIT_DATA;
				c->caps |= MMC_CAP_4_BIT_DATA;
				mmc->slots[0].caps = c->caps;
			}
			break;
		case 2:
			if (c->ext_clock)
				c->transceiver = 1;
			if (c->transceiver && (c->caps & MMC_CAP_8_BIT_DATA)) {
				c->caps &= ~MMC_CAP_8_BIT_DATA;
				c->caps |= MMC_CAP_4_BIT_DATA;
			}
			/* off-chip level shifting, or none */
			mmc->slots[0].set_power = twl_mmc2_set_power;
			mmc->slots[0].set_sleep = twl_mmc2_set_sleep;
#ifdef CONFIG_MMC_EMBEDDED_SDIO
			mmc->slots[0].ocr_mask  = MMC_VDD_165_195;
#endif
			break;
		case 3:
			if (c->ext_clock)
				c->transceiver = 1;
			if (c->transceiver && (c->caps & MMC_CAP_8_BIT_DATA)) {
				c->caps &= ~MMC_CAP_8_BIT_DATA;
				c->caps |= MMC_CAP_4_BIT_DATA;
			}
			/* off-chip level shifting, or none */
			mmc->slots[0].set_power = twl_mmc3_set_power;
			mmc->slots[0].set_sleep = twl_mmc3_set_sleep;
#ifdef CONFIG_MMC_EMBEDDED_SDIO
			mmc->slots[0].ocr_mask  = MMC_VDD_165_195;
#endif
			break;
		default:
			pr_err("MMC%d configuration not supported!\n", c->mmc);
			kfree(mmc);
			continue;
		}
		hsmmc_data[controller_cnt] = mmc;
#ifdef TI_FS_MMC
		omap2_init_mmc(hsmmc_data[controller_cnt], c->mmc);
#endif
		controller_cnt++;
	}
#ifndef TI_FS_MMC
	{
		int init_array[] = {1, 0, 2, 3, 4};
		int counti, countj;
		for (countj = 0; countj < OMAP34XX_NR_MMC; countj++) {
			counti = init_array[countj];
			omap2_init_mmc(hsmmc_data[counti], counti+1);
		}
	}
#endif

	/* pass the device nodes back to board setup code */
	controller_cnt = 0;
	for (c = controllers; c->mmc; c++) {
		struct omap_mmc_platform_data *mmc = hsmmc_data[controller_cnt];

		if (!c->mmc || c->mmc > nr_hsmmc)
			continue;
		c->dev = mmc->dev;
	}

done:
	for (i = 0; i < controller_cnt; i++)
		kfree(hsmmc_data[i]);
}

#endif
