/*
 * Copyright (C) 2010 Texas Instruments Inc.
 * Copyright (C) 2010 LG Electronic Inc.
 *
 * Modified from mach-omap2/board-hub-pannel.c
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/i2c/twl.h>
//#include <linux/spi/spi.h>
//#include <plat/mcspi.h>
#include <video/omapdss.h>

#include <linux/regulator/machine.h>
#include <linux/regulator/consumer.h>
//#include <linux/leds.h>
#include "mux.h"
#include <linux/delay.h>
#include <video/lge-dsi-panel.h>


#define LCD_PANEL_BACKLIGHT_GPIO 	(7 + OMAP_MAX_GPIO_LINES)
#define LCD_PANEL_RESET_GPIO_PROD	96
#define LCD_PANEL_RESET_GPIO_PILOT	55
#define LCD_PANEL_QVGA_GPIO		56

#define SIL9022_RESET_GPIO 		97 
/* Added for backlight support */
#define ENABLE_VPLL2_DEDICATED          0x05
#define ENABLE_VPLL2_DEV_GRP            0xE0
#define TWL4030_VPLL2_DEV_GRP           0x33
#define TWL4030_VPLL2_DEDICATED         0x36


/* 20100623 kyungtae.oh@lge.com MIPI DSI setting [START]*/
#define DSI_CLOCK_POLARITY  0   /* +/- pin order */
#define DSI_DATA0_POLARITY  0   /* +/- pin order */
#define DSI_DATA1_POLARITY  0   /* +/- pin order */
#define DSI_CLOCK_LANE      3   /* Clock lane position: 1 */
#define DSI_DATA0_LANE      1   /* Data0 lane position: 2 */
#define DSI_DATA1_LANE      2   /* Data1 lane position: 3 */

#define LCD_RESET_N         34
#define LCD_TE              53
#define LCD_CS              54
/* 20100623 kyungtae.oh@lge.com MIPI DSI setting [END]*/

enum {
	DSS_IX_DSI1=0,
	DSS_IX_DSI2
};

extern void hub_lcd_initialize(void);
extern void aat2870_shutdown(void);

#ifdef CONFIG_OMAP2_DSS_HDMI
#define HDMI_REG_EN_GPIO    104

extern void subpm_set_output(subpm_output_enum outnum, int onoff);
extern void subpm_output_enable(void);
#endif // CONFIG_OMAP2_DSS_HDMI


#ifdef CONFIG_OMAP2_DSS_HDMI
void hub_hdmi_reset_enable(int level)
{
	/* Set GPIO_97 to high to pull SiI9022 HDMI transmitter out of reset
	* and low to disable it.
	*/
	//gpio_request(HDMI_REG_EN_GPIO, "hdmi reset");
	gpio_direction_output(HDMI_REG_EN_GPIO, level);
}

static int hub_panel_enable_hdmi(struct omap_dss_device *dssdev)
{
	hub_hdmi_reset_enable(1);

	//2010-5-8 changhoony.lee@lge.com sub-PMIC Power on
	subpm_set_output(SWREG,1);
	subpm_output_enable();

	subpm_set_output(LDO1,1);
	subpm_output_enable();

	subpm_set_output(LDO2,1);
	subpm_output_enable();

	printk(KERN_INFO "Enable_hdmi",__func__,__LINE__);
	
	return 0;
}

static void hub_panel_disable_hdmi(struct omap_dss_device *dssdev)
{
	hub_hdmi_reset_enable(0);
	
	//2010-5-8 changhoony.lee@lge.com sub-PMIC Power off
	subpm_set_output(LDO1,0);
	subpm_output_enable();

	subpm_set_output(LDO2,0);
	subpm_output_enable();

	subpm_set_output(SWREG,0);
	subpm_output_enable();

	printk(KERN_INFO "Disable_hdmi",__func__,__LINE__);
}

struct hdmi_platform_data hub_hdmi_data = {
//#ifdef CONFIG_PM //enabled at the time of PM
//	.set_min_bus_tput = omap_pm_set_min_bus_tput,
//	.set_max_mpu_wakeup_lat =  omap_pm_set_max_mpu_wakeup_lat,
//#endif
};

static struct omap_dss_device hub_hdmi_device = {
	.name = "hdmi",
	.driver_name = "hdmi_panel",
	.type = OMAP_DISPLAY_TYPE_DPI,
	.channel = OMAP_DSS_CHANNEL_LCD,
	.phy.dpi.data_lines = 24,
	.platform_enable = hub_panel_enable_hdmi,
	.platform_disable = hub_panel_disable_hdmi,
	.dev		= {
		.platform_data = &hub_hdmi_data,
	},
};
#endif // CONFIG_OMAP2_DSS_HDMI

static int hub_panel_enable_lcd(struct omap_dss_device *dssdev)
{
#if 1 /* 20110304 seven.kim@lge.com late_resume_lcd */
	hub_lcd_initialize();
#else
	gpio_request(LCD_RESET_N, "lcd reset_n");
	gpio_direction_output(LCD_RESET_N, 1);
	mdelay(1);
	gpio_direction_output(LCD_RESET_N, 0);		// LG logo delete
	mdelay(20);

	hub_lcd_initialize(); /* 20110304 seven.kim@lge.com late_resume_lcd */		// backlight delete

	gpio_direction_output(LCD_RESET_N, 1);
	mdelay(20);
#endif
	return 0;
}

/* Added implementation for panel_power_disable */
static void hub_panel_disable_lcd(struct omap_dss_device *dssdev)
{
	aat2870_shutdown(); /* 20110304 seven.kim@lge.com late_resume_lcd */
	gpio_direction_output(LCD_RESET_N, 0);
}

static struct lge_dsi_panel_data dsi_lgd_panel = {
	.name			= "hub_panel",
	.reset_gpio		= LCD_RESET_N,
	.use_ext_te		= true,
	.ext_te_gpio		= LCD_TE,
	.esd_interval		= false,
	.set_backlight		= NULL
};

static struct omap_dss_device hub_lcd_device = {
	.name			= "lcd",
	.driver_name		= "hub_panel",
	.type			= OMAP_DISPLAY_TYPE_DSI,

	.data			= &dsi_lgd_panel,

	.phy.dsi		= {
		.clk_lane	= DSI_CLOCK_LANE,
		.clk_pol	= DSI_CLOCK_POLARITY,
		.data1_lane	= DSI_DATA0_LANE,
		.data1_pol	= DSI_DATA0_POLARITY,
		.data2_lane	= DSI_DATA1_LANE,
		.data2_pol	= DSI_DATA1_POLARITY,
		.type		= OMAP_DSS_DSI_TYPE_CMD_MODE,
		.module		= DSS_IX_DSI1,
	},
	
	.clocks = {
		.dispc = {
			.channel = {
				.lck_div	= 1,
				.pck_div	= 3,
			},
			.dispc_fclk_src	= OMAP_DSS_CLK_SRC_DSI_PLL_HSDIV_DISPC,
		},
		
		.dsi = {
			.regn		= 13,
			.regm		= 164,
			.regm_dispc	= 8,
			.regm_dsi	= 12,
			
			.lp_clk_div	= 5,
			.dsi_fclk_src	= OMAP_DSS_CLK_SRC_DSI_PLL_HSDIV_DSI,
		},
	},
	
	.platform_enable	= hub_panel_enable_lcd,
	.platform_disable	= hub_panel_disable_lcd,
	.channel		= OMAP_DSS_CHANNEL_LCD,
	.skip_init		= false,
};

static struct omap_dss_device *hub_dss_devices[] = {
	&hub_lcd_device,
#ifdef CONFIG_OMAP2_DSS_HDMI
	&hub_hdmi_device,
#endif
};

static struct omap_dss_board_info hub_dss_data = {
	.num_devices		= ARRAY_SIZE(hub_dss_devices),
	.devices		= hub_dss_devices,
	.default_device		= &hub_lcd_device,
};

void __init hub_display_init(void)
{
	omap_mux_init_signal("gpio_53", OMAP_PIN_INPUT);
	omap_mux_init_signal("gpio_34", OMAP_PIN_OUTPUT);
	omap_mux_init_signal("gpio_54", OMAP_PIN_OUTPUT);
	
	omap_display_init(&hub_dss_data);
}
