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


#ifdef CONFIG_OMAP2_DSS_HDMI
//#include <linux/subpm_bh6172.h>
#if defined(CONFIG_REGULATOR_LP8720)
#include <linux/regulator/lp8720.h>
#define LP8720_ENABLE 37
#endif
#endif


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
#define LCD_CP_EN						149
/* 20100623 kyungtae.oh@lge.com MIPI DSI setting [END]*/

enum {
	DSS_IX_DSI1=0,
	DSS_IX_DSI2
};

extern void hub_lcd_initialize(void);
extern void aat2870_shutdown(void);

#ifdef CONFIG_OMAP2_DSS_HDMI
#define HDMI_REG_EN_GPIO                55

extern void subpm_set_output(subpm_output_enum outnum, int onoff);
extern void subpm_output_enable(void);
extern unsigned int cur_main_lcd_level;

int hdmi_power_initialize =0;
bool hdmi_detect_power_enabled = 0;  // sghyun_test

struct hdmi_platform_data {
	void (*set_min_bus_tput)(struct device *dev, u8 agent_id, unsigned long r);
	void (*set_max_mpu_wakeup_lat)(struct device *dev, long t);
};
#endif // CONFIG_OMAP2_DSS_HDMI


#ifdef CONFIG_OMAP2_DSS_HDMI
static void justin_hdmi_reset_enable(int level)
{
	/* Set GPIO_97 to high to pull SiI9022 HDMI transmitter out of reset
	* and low to disable it.
	*/
	gpio_request(HDMI_REG_EN_GPIO, "hdmi reset"); // == 2011.06.03 === hycho@ubiquix.com for warning delete
	gpio_direction_output(HDMI_REG_EN_GPIO, 1);
	gpio_set_value(HDMI_REG_EN_GPIO, level);
}

static int justin_panel_enable_hdmi(struct omap_dss_device *dssdev)
{
//[[ 20120521 goochang.jeong@lge.com for HDMI start
	int MaximumCount = 10;

	while ( hdmi_power_initialize==1 && MaximumCount-->0)
		msleep(100);

	hdmi_power_initialize =1;

	// CONTROL_PADCONF_DSS_PCLK
	omap_writel(omap_readl(0x480020D4) & ~0x7, 0x480020D4);
	
	extern void lp8720_reinit();
	lp8720_reinit();

	mdelay(20);

	//justin_panel_power_enable(1);
	justin_hdmi_reset_enable(1);
	mdelay(20);

	//2010-5-8 changhoony.lee@lge.com sub-PMIC Power on
#if 0  //20120521 goochang.jeong@lge.com for HDMI 
	subpm_set_output(SWREG,1);
	subpm_output_enable();
#endif

	subpm_set_output(LDO1,1);
	subpm_output_enable();

/* 20110531 comment by jslee@ubiquix.com
	subpm_set_output(LDO2,1);
	subpm_output_enable();
*/
	if(HDMI_DSS_DBG) printk(KERN_INFO "%s :: Enable_hdmi\n",__func__);
	msleep(20);
	
	return 0;
}

static void justin_panel_disable_hdmi(struct omap_dss_device *dssdev)
{
	int MaximumCount = 10;
	while ( hdmi_power_initialize==0 && MaximumCount-->0)
		msleep(100);

	MaximumCount = 10;
	
	extern int is_hdmi_nxp_driver_enabled(void);
	while(is_hdmi_nxp_driver_enabled() && MaximumCount-->0)
		msleep(100);
	/* prevent power off, before closing nxp driver.*/

	// CONTROL_PADCONF_DSS_PCLK
	omap_writel(omap_readl(0x480020D4) | 0x7, 0x480020D4);

	justin_hdmi_reset_enable(0);
	
	//2010-5-8 changhoony.lee@lge.com sub-PMIC Power off
	subpm_set_output(LDO1,0);
	subpm_output_enable();

/* 20110531 comment by jslee@ubiquix.com
	subpm_set_output(LDO2,0);
	subpm_output_enable();
*/	
#if 0	
	subpm_set_output(SWREG,0);
	subpm_output_enable();
#endif
	msleep(40);

	if(HDMI_DSS_DBG) printk(KERN_INFO "%s :: Disable_hdmi\n",__func__);
	hdmi_power_initialize =0;
}

static bool justin_hdmi_get_device_detect(struct omap_dss_device *dssdev)
{
	return hdmi_detect_power_enabled;
}

struct hdmi_platform_data justin_hdmi_data = {
#if 0   
	.set_min_bus_tput = omap_pm_set_min_bus_tput,
#endif
};

static struct omap_dss_device justin_hdmi_device = {
	.name = "hdmi",
	.driver_name = "hdmi_panel",
	.type = OMAP_DISPLAY_TYPE_DPI,
	.clocks = {   
		.dispc = {
			.dispc_fclk_src = OMAP_DSS_CLK_SRC_DSI_PLL_HSDIV_DISPC,
		},
	},
	.phy.dpi.data_lines = 24,
	.platform_enable = justin_panel_enable_hdmi,
	.platform_disable = justin_panel_disable_hdmi,
	.get_device_detect = justin_hdmi_get_device_detect,
	.dev		= {
		.platform_data = &justin_hdmi_data,
	},
};
#endif	//CONFIG_OMAP2_DSS_HDMI

static int justin_panel_enable_lcd(struct omap_dss_device *dssdev)
{
#if 1 /* 20110304 seven.kim@lge.com late_resume_lcd */
//	hub_lcd_initialize();
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
static void justin_panel_disable_lcd(struct omap_dss_device *dssdev)
{
#ifdef CONFIG_OMAP2_DSS_HDMI//NATTING_TEMP
	  // shouldn't reset LCD while LCD ON
	  if (cur_main_lcd_level) {
		if(HDMI_DSS_DBG)
			printk(KERN_INFO "%s::Trying To Turn Off LCD during BL ON::should return here\n", __func__);
		return 0;
	  }
#endif
		gpio_set_value(LCD_RESET_N, 0);
}

static struct lge_dsi_panel_data dsi_lgd_panel = {
	.name			= "justin_panel",
	.reset_gpio		= LCD_RESET_N,
	.use_ext_te		= true,
	.ext_te_gpio		= LCD_TE,
	.esd_interval		= false,
	.set_backlight		= NULL
};

static struct omap_dss_device justin_lcd_device = {
	.name			= "lcd",
	.driver_name		= "justin_panel",
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
				.lck_div	= 2,
				.pck_div	= 3,
			},
			.dispc_fclk_src	= OMAP_DSS_CLK_SRC_DSI_PLL_HSDIV_DISPC,
		},
		
		.dsi = {
			.regn		= 13,
			.regm		= 160,
			.regm_dispc	= 4,
			.regm_dsi	= 12,
			
			.lp_clk_div	= 3,
			.dsi_fclk_src	= OMAP_DSS_CLK_SRC_DSI_PLL_HSDIV_DSI,
		},
	},
	
	.platform_enable	= justin_panel_enable_lcd,
	.platform_disable	= justin_panel_disable_lcd,
	.channel		= OMAP_DSS_CHANNEL_LCD,
	.skip_init		= false,
};


static struct omap_dss_device *justin_dss_devices[] = {
	&justin_lcd_device,
#ifdef CONFIG_OMAP2_DSS_HDMI
	&justin_hdmi_device,
#endif
};

static struct omap_dss_board_info justin_dss_data = {
	.num_devices		= ARRAY_SIZE(justin_dss_devices),
	.devices		= justin_dss_devices,
	.default_device		= &justin_lcd_device,
};

void __init hub_display_init(void)
{
	omap_mux_init_signal("gpio_53", OMAP_PIN_INPUT);
	omap_mux_init_signal("gpio_34", OMAP_PIN_OUTPUT);
	omap_mux_init_signal("gpio_149", OMAP_PIN_OUTPUT);
	omap_display_init(&justin_dss_data);
}
