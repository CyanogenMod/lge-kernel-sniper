/*
 * Copyright (C) 2010 LG Electronics Inc.
 * Copyright (C) 2009 Texas Instruments Inc.
 *
 * Modified from mach-omap2/board-zoom-peripherals.c
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/input.h>
#include <linux/input/matrix_keypad.h>
#include <linux/gpio.h>
#include <linux/i2c/twl.h>
#include <linux/regulator/machine.h>
#include <linux/mmc/host.h>
#include <linux/synaptics_i2c_rmi.h>
#include <linux/interrupt.h>
#include <linux/keyreset.h>

#ifdef CONFIG_INPUT_HALLEFFECT_BU52014HV
#include <linux/bu52014hfv.h>
#endif

#ifdef CONFIG_MPU_SENSORS_MPU3050
#include <linux/mpu.h>
#endif

#ifdef CONFIG_LBEE9QMB_RFKILL
#include <linux/lbee9qmb-rfkill.h>
#endif // CONFIG_LBEE9QMB_RFKILL
#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/mach/map.h>

#include <plat/common.h>
#include <plat/usb.h>
#include <plat/control.h>
#include <plat/mux.h>
#include <plat/omap-serial.h>

#include <linux/usb/android_composite.h> 

#include "mmc-twl4030.h"
#include "mux.h"
#include "pm.h"

#include <linux/switch.h>
#if defined(CONFIG_HUB_AMP_WM9093)
#include <mach/wm9093.h>
#endif
#ifdef CONFIG_LGE_SPI
#include <linux/spi/ifx_n721_spi.h>
#include <plat/mcspi.h>
#include <linux/spi/spi.h>
#include <plat/control.h>
#include <linux/delay.h>

#endif /* CONFIG_LGE_SPI */

// LGE_DOM_UPDATE_S hayun.kim 2010/06/15 {
#ifdef CONFIG_SPI_TDMB
#define HUB_TDMB_IRQ_GPIO 		29
#endif
// LGE_DOM_UPDATE_E hayun.kim 2010/06/15 }

#if defined(CONFIG_REGULATOR_LP8720)
#include <linux/regulator/lp8720.h>
static struct lp8720_platform_data lp8720_pdata = {
	.en_gpio_num         = 37,
};
#endif

#define HUB_TS_I2C_INT_GPIO 35
#if defined(CONFIG_BACKLIGHT_AAT2870)
#define AAT2870_BACKLIGHT_ADDRESS 0x60
#endif

#if defined (CONFIG_LEDS_BD2802_LGE)
#define KEY_LEDS_BD2802_ADDRESS 	0x1A
#endif


#ifdef CONFIG_OMAP2_DSS_HDMI
#define HDMI_TX_NAME	"tda19989"
#define TDA998X_I2C_SLAVEADDRESS	0x70
#define HDMI_CEC_NAME	"tda19989cec"
#define TDA998XCEC_I2C_SLAVEADDRESS	0x34
#endif

#ifdef CONFIG_ANDROID_RAM_CONSOLE
extern struct platform_device hub_ram_console_device;
#endif


#include <media/v4l2-int-device.h>

#if (defined(CONFIG_VIDEO_IMX072) || defined(CONFIG_VIDEO_IMX072_MODULE)) 
#include <media/imx072.h>
#include <../drivers/media/video/rt8515.h>
#include <../drivers/media/video/dw9716.h>
extern struct imx072_platform_data hub_imx072_platform_data;
extern struct dw9716_platform_data hub_dw9716_platform_data;
extern struct rt8515_platform_data hub_rt8515_data;
#endif


#ifdef CONFIG_VIDEO_OMAP3
extern void hub_cam_init(void);
#else
#define hub_cam_init()	NULL
#endif

#ifdef CONFIG_INPUT_HALLEFFECT_BU52014HV
#define SNIPER_HF_NORTH_GPIO		28
#define SNIPER_HF_SOUTH_GPIO		29

static struct bu52014hfv_platform_data bu52014hfv_platform_data = {
	.docked_north_gpio = SNIPER_HF_NORTH_GPIO,
	.docked_south_gpio = SNIPER_HF_SOUTH_GPIO,
	.north_is_desk = 1,
};

static struct platform_device sniper_hall_effect_dock = {
	.name	= BU52014HFV_MODULE_NAME,
	.id	= -1,
	.dev	= {
		.platform_data  = &bu52014hfv_platform_data,
	},
};

static void sniper_hall_effect_init(void)
{
	gpio_request(SNIPER_HF_NORTH_GPIO, "sniper dock north");
	gpio_direction_input(SNIPER_HF_NORTH_GPIO);

	gpio_request(SNIPER_HF_SOUTH_GPIO, "sniper dock south");
	gpio_direction_input(SNIPER_HF_SOUTH_GPIO);
}
#endif

//extern struct regulator_init_data hub_vdac;
extern struct regulator_init_data hub_vdsi;
extern void hub_lcd_tv_panel_init(void);

static int board_keymap[] = {
	KEY(0, 0, KEY_VOLUMEUP),
	KEY(1, 0, KEY_VOLUMEDOWN),
	KEY(0, 2, KEY_MENU),
	KEY(0, 1, KEY_HOME),
	KEY(1, 2, KEY_BACK),
	KEY(1, 1, KEY_SEARCH),
	KEY(2, 2, KEY_PROG3),	// Add virtual key for CP DOWN pop up message
	KEY(2, 0, KEY_KPJPCOMMA), //This is Gesture Key
};

static struct matrix_keymap_data board_map_data = {
	.keymap			= board_keymap,
	.keymap_size		= ARRAY_SIZE(board_keymap),
};

static struct twl4030_keypad_data hub_kp_twl4030_data = {
	.keymap_data	= &board_map_data,
	.rows		= 3,
	.cols		= 3,
};

static int hub_reset_keys_up[] = {
        0
};

static struct keyreset_platform_data hub_reset_keys_pdata = {
	/* .crash_key = KEY_RIGHTSHIFT, */
	.keys_up = hub_reset_keys_up,
	.keys_down = {
		KEY_POWER,
		KEY_VOLUMEUP,
		0
	},
};

struct platform_device hub_reset_keys_device = {
         .name = KEYRESET_NAME,
         .dev.platform_data = &hub_reset_keys_pdata,
};

//#define CONFIG_LGE_SLEEP_HUB
#ifdef CONFIG_LGE_MTC_ETA
struct platform_device hub_mtc_eta_log_device = {
	.name = "lge_mtc_eta_logger",
};
#endif

#ifdef CONFIG_LGE_SLEEP_HUB
static struct twl4030_ins sleep_on_seq[] __initdata = {
#if 0
	/* Turn off HFCLKOUT */
	{MSG_SINGULAR(DEV_GRP_P1, 0x19, RES_STATE_OFF), 2},
#endif	       	
	/* Turn OFF VDD1 */
	{MSG_SINGULAR(DEV_GRP_P1, 0xf, RES_STATE_OFF), 2},
	/* Turn OFF VDD2 */
	{MSG_SINGULAR(DEV_GRP_P1, 0x10, RES_STATE_OFF), 2},
	/* Turn OFF VPLL1 */
	{MSG_SINGULAR(DEV_GRP_P1, 0x7, RES_STATE_OFF), 2},

	{MSG_SINGULAR(DEV_GRP_P1, 0xe, RES_STATE_ACTIVE), 0xe},
	//{MSG_SINGULAR(DEV_GRP_P1, 0x17, RES_STATE_OFF), 0xe},

        //{MSG_BROADCAST(DEV_GRP_P3, RES_GRP_PP_PR, RES_TYPE_ALL, RES_TYPE2_R0, RES_STATE_SLEEP), 0x37},
        {MSG_BROADCAST(DEV_GRP_P3, RES_GRP_ALL, RES_TYPE_ALL, RES_TYPE2_R0, RES_STATE_SLEEP), 0x37},	
};

static struct twl4030_ins wakeup_p12_seq[] __initdata = {

	//{MSG_SINGULAR(DEV_GRP_P1, 0x17, RES_STATE_ACTIVE), 0xe},

	/* Turn ON VDD1 */
	{MSG_SINGULAR(DEV_GRP_P1, 0xf, RES_STATE_ACTIVE), 2},
	/* Turn ON VDD2 */
	{MSG_SINGULAR(DEV_GRP_P1, 0x10, RES_STATE_ACTIVE), 2},
	/* Turn ON VPLL1 */
	{MSG_SINGULAR(DEV_GRP_P1, 0x7, RES_STATE_ACTIVE), 2},
	/* Turn on HFCLKOUT */
	{MSG_SINGULAR(DEV_GRP_P1, 0x19, RES_STATE_ACTIVE), 2},

        //{MSG_BROADCAST(DEV_GRP_P3, RES_GRP_PP_PR, RES_TYPE_ALL, RES_TYPE2_R0, RES_STATE_ACTIVE), 0x37},
        {MSG_BROADCAST(DEV_GRP_P3, RES_GRP_ALL, RES_TYPE_ALL, RES_TYPE2_R0, RES_STATE_ACTIVE), 0x37},	
};

static struct twl4030_ins wakeup_p3_seq[] __initdata = {

	{MSG_SINGULAR(DEV_GRP_P1, 0x17, RES_STATE_ACTIVE), 0xe},
	{MSG_SINGULAR(DEV_GRP_P1, 0x19, RES_STATE_ACTIVE), 2},

        //{MSG_BROADCAST(DEV_GRP_P3, RES_GRP_PP_PR, RES_TYPE_ALL, RES_TYPE2_R0, RES_STATE_ACTIVE), 0x37},
        {MSG_BROADCAST(DEV_GRP_P3, RES_GRP_ALL, RES_TYPE_ALL, RES_TYPE2_R0, RES_STATE_ACTIVE), 0x37},	
};

static struct twl4030_ins wrst_seq[] __initdata = {
/*
 * Reset twl4030.
 * Reset VDD1 regulator.
 * Reset VDD2 regulator.
 * Reset VPLL1 regulator.
 * Enable sysclk output.
 * Reenable twl4030.
 */
	{MSG_SINGULAR(DEV_GRP_NULL, 0x1b, RES_STATE_OFF), 2},
	{MSG_SINGULAR(DEV_GRP_P1, 0xf, RES_STATE_WRST), 0x13},
	{MSG_SINGULAR(DEV_GRP_P1, 0x10, RES_STATE_WRST), 0x13},
	{MSG_SINGULAR(DEV_GRP_P1, 0x7, RES_STATE_WRST), 0x60},
	{MSG_SINGULAR(DEV_GRP_P1, 0x19, RES_STATE_ACTIVE), 2},
	{MSG_SINGULAR(DEV_GRP_NULL, 0x1b, RES_STATE_ACTIVE), 2},
};

static struct twl4030_resconfig twl4030_rconfig[] = {
	{.resource = RES_HFCLKOUT,.devgroup = DEV_GRP_P3,.type = -1,
	 .type2 = -1},
	{.resource = RES_VDD1,.devgroup = DEV_GRP_P1,.type = -1,
	 .type2 = -1},
	{.resource = RES_VDD2,.devgroup = DEV_GRP_P1,.type = -1,
	 .type2 = -1},
	{.resource = RES_CLKEN,.devgroup = DEV_GRP_P3,.type = -1,
	 .type2 = 1},
	{0, 0},
};

#else
static struct twl4030_ins __initdata sleep_on_seq[] = {
/*
 * Turn off everything
 */
#if 0
	{MSG_SINGULAR(DEV_GRP_NULL, RES_VDAC, RES_STATE_OFF), 15},
	{MSG_SINGULAR(DEV_GRP_NULL, RES_VAUX2, RES_STATE_OFF), 19},
#endif
	{MSG_BROADCAST(DEV_GRP_NULL, RES_GRP_ALL, 1, 0, RES_STATE_SLEEP), 2},
};

static struct twl4030_ins __initdata wakeup_p12_seq[] = {
/*
 * Reenable everything
 */
//	{MSG_SINGULAR(DEV_GRP_NULL, RES_HFCLKOUT, RES_STATE_ACTIVE), 1},////
	{MSG_SINGULAR(DEV_GRP_NULL, RES_VAUX2, RES_STATE_ACTIVE), 1},
	{MSG_SINGULAR(DEV_GRP_NULL, RES_VDAC, RES_STATE_ACTIVE), 5},
	{MSG_BROADCAST(DEV_GRP_NULL, RES_GRP_ALL, 1, 0, RES_STATE_ACTIVE), 2},
};

static struct twl4030_ins __initdata wakeup_p3_seq[] = {
/*
 * Reenable everything
 */
//	{MSG_SINGULAR(DEV_GRP_NULL, RES_HFCLKOUT, RES_STATE_ACTIVE), 1},////
	{MSG_SINGULAR(DEV_GRP_NULL, RES_VAUX2, RES_STATE_ACTIVE), 1},
	{MSG_SINGULAR(DEV_GRP_NULL, RES_VDAC, RES_STATE_ACTIVE), 5},
	{MSG_BROADCAST(DEV_GRP_NULL, RES_GRP_ALL, 1, 0, RES_STATE_ACTIVE), 2},
};

static struct twl4030_ins __initdata wrst_seq[] = {
/*
 * Reset twl4030.
 * Reset VDD1 regulator.
 * Reset VDD2 regulator.
 * Reset VPLL1 regulator.
 * Enable sysclk output.
 * Reenable twl4030.
 */

//################################################################################
	{MSG_SINGULAR(DEV_GRP_NULL, RES_RESET, RES_STATE_OFF), 2},
#if 0	// should be removed.
	{MSG_BROADCAST(DEV_GRP_NULL, RES_GRP_ALL, 0, 1, RES_STATE_ACTIVE),
		0x13},
	{MSG_BROADCAST(DEV_GRP_NULL, RES_GRP_PP, 0, 3, RES_STATE_OFF), 0x13},
#endif

	{MSG_SINGULAR(DEV_GRP_NULL, RES_VDAC, RES_STATE_OFF), 1},
	{MSG_SINGULAR(DEV_GRP_NULL, RES_VAUX2, RES_STATE_OFF), 1},

	{MSG_SINGULAR(DEV_GRP_NULL, RES_VDD1, RES_STATE_WRST), 0x13},
	{MSG_SINGULAR(DEV_GRP_NULL, RES_VDD2, RES_STATE_WRST), 0x13},
	{MSG_SINGULAR(DEV_GRP_NULL, RES_VPLL1, RES_STATE_WRST), 0x35},
	{MSG_SINGULAR(DEV_GRP_P3, RES_HFCLKOUT, RES_STATE_ACTIVE), 2},

	{MSG_SINGULAR(DEV_GRP_NULL, RES_VAUX2, RES_STATE_ACTIVE), 1},
	{MSG_SINGULAR(DEV_GRP_NULL, RES_VDAC, RES_STATE_ACTIVE), 5},

	{MSG_SINGULAR(DEV_GRP_NULL, RES_RESET, RES_STATE_ACTIVE), 2},
//################################################################################


#if 0
	{MSG_SINGULAR(DEV_GRP_NULL, RES_RESET, RES_STATE_OFF), 2},
#if 0	// should be removed.
	{MSG_BROADCAST(DEV_GRP_NULL, RES_GRP_ALL, 0, 1, RES_STATE_ACTIVE),
		0x13},
	{MSG_BROADCAST(DEV_GRP_NULL, RES_GRP_PP, 0, 3, RES_STATE_OFF), 0x13},
#endif

	{MSG_SINGULAR(DEV_GRP_NULL, RES_VDAC, RES_STATE_OFF), 1},
	{MSG_SINGULAR(DEV_GRP_NULL, RES_VAUX2, RES_STATE_OFF), 1},

		{MSG_SINGULAR(DEV_GRP_NULL, RES_VDD1, RES_STATE_WRST), 0x13},
		{MSG_SINGULAR(DEV_GRP_NULL, RES_VDD2, RES_STATE_WRST), 0x13},
		{MSG_SINGULAR(DEV_GRP_NULL, RES_VPLL1, RES_STATE_WRST), 0x35},
		{MSG_SINGULAR(DEV_GRP_P3, RES_HFCLKOUT, RES_STATE_ACTIVE), 2},

		{MSG_SINGULAR(DEV_GRP_NULL, RES_VAUX2, RES_STATE_ACTIVE), 1},
		{MSG_SINGULAR(DEV_GRP_NULL, RES_VDAC, RES_STATE_ACTIVE), 5},

		{MSG_SINGULAR(DEV_GRP_NULL, RES_RESET, RES_STATE_ACTIVE), 2},
#endif
};

static struct twl4030_resconfig twl4030_rconfig[] = {
	{ .resource = RES_VDD1, .devgroup = -1,
	  .type = 1, .type2 = -1, .remap_off = RES_STATE_OFF, .remap_sleep = RES_STATE_OFF
	},
	{ .resource = RES_VDD2, .devgroup = -1,
	  .type = 1, .type2 = -1, .remap_off = RES_STATE_OFF, .remap_sleep = RES_STATE_OFF
	},
	{ .resource = RES_VPLL1, .devgroup = -1,
	  .type = 1, .type2 = -1, .remap_off = RES_STATE_OFF, .remap_sleep = RES_STATE_OFF
	},
	{ .resource = RES_VPLL2, .devgroup = -1,
	  .type = -1, .type2 = 3, .remap_off = -1, .remap_sleep = -1	/*WM9093*/
	},
	{ .resource = RES_VAUX1, .devgroup = -1,
	  .type = -1, .type2 = 3, .remap_off = -1, .remap_sleep = -1	/*2.8V_PROXI*/
	},
	{ .resource = RES_VAUX2, .devgroup = -1,
	  .type = -1, .type2 = 3, .remap_off = -1, .remap_sleep = -1	/*3.0V_MOTION*/
	},
	{ .resource = RES_VAUX3, .devgroup = -1,
	  .type = -1, .type2 = 3, .remap_off = -1, .remap_sleep = -1	/*VT_CAM_IO_1.8V*/
	},
	{ .resource = RES_VAUX4, .devgroup = -1,
	  .type = -1, .type2 = 3, .remap_off = -1, .remap_sleep = -1	/*1.8V_CSI2*/
	},
	{ .resource = RES_VMMC1, .devgroup = -1,
	  .type = -1, .type2 = 3, .remap_off = -1, .remap_sleep = -1	/*VT_CAM_DRV_2.8V*/
	},
	{ .resource = RES_VMMC2, .devgroup = -1,
	  .type = -1, .type2 = 3, .remap_off = -1, .remap_sleep = -1	/*1.8V_MMC_EN*/
	},
	{ .resource = RES_VDAC, .devgroup = -1,
	  .type = -1, .type2 = 3, .remap_off = -1, .remap_sleep = -1	/*1.8V_MOTION_VIO*/
	},
	{ .resource = RES_VSIM, .devgroup = -1,
	  .type = -1, .type2 = 3, .remap_off = -1, .remap_sleep = -1	/*1.8V_WLAN*/
	},
	{ .resource = RES_VINTANA1, .devgroup = DEV_GRP_P1 | DEV_GRP_P3,
	  .type = -1, .type2 = -1, .remap_off = -1, .remap_sleep = -1
	},
	{ .resource = RES_VINTANA2, .devgroup = DEV_GRP_P1 | DEV_GRP_P3,
	  .type = 1, .type2 = -1, .remap_off = -1, .remap_sleep = -1
	},
	{ .resource = RES_VINTDIG, .devgroup = DEV_GRP_P1 | DEV_GRP_P3,
	  .type = -1, .type2 = -1, .remap_off = -1, .remap_sleep = -1
	},
	{ .resource = RES_VIO, .devgroup = DEV_GRP_P3,
	  .type = 1, .type2 = -1, .remap_off = -1, .remap_sleep = -1
	},
	{ .resource = RES_CLKEN, .devgroup = DEV_GRP_P1 | DEV_GRP_P3,
	  .type = 1, .type2 = -1 , .remap_off = -1, .remap_sleep = 0	/* Turn off */
	},
	{ .resource = RES_REGEN, .devgroup = DEV_GRP_P1 | DEV_GRP_P3,
	  .type = 1, .type2 = -1, .remap_off = -1, .remap_sleep = -1
	},
	{ .resource = RES_NRES_PWRON, .devgroup = DEV_GRP_P1 | DEV_GRP_P3,
	  .type = 1, .type2 = -1, .remap_off = -1, .remap_sleep = -1
	},
	{ .resource = RES_SYSEN, .devgroup = 0,
	  .type = 1, .type2 = -1, .remap_off = -1, .remap_sleep = 0	/* Turn off */
	},
	{ .resource = RES_HFCLKOUT, .devgroup = DEV_GRP_P3,
	  .type = 1, .type2 = -1, .remap_off = -1, .remap_sleep = 0	/* Turn off */
	},
	{ .resource = RES_32KCLKOUT, .devgroup = -1,
	  .type = 1, .type2 = -1, .remap_off = -1, .remap_sleep = -1
	},
	{ .resource = RES_RESET, .devgroup = -1,
	  .type = 1, .type2 = -1, .remap_off = -1, .remap_sleep = -1
	},
	{ .resource = RES_Main_Ref, .devgroup = -1,
	  .type = 1, .type2 = -1, .remap_off = -1, .remap_sleep = -1
	},
	{ 0, 0},
};
#endif

static struct twl4030_script sleep_on_script = {
	.script = sleep_on_seq,
	.size = ARRAY_SIZE(sleep_on_seq),
	.flags = TWL4030_SLEEP_SCRIPT,
};

static struct twl4030_script wakeup_p12_script = {
	.script = wakeup_p12_seq,
	.size = ARRAY_SIZE(wakeup_p12_seq),
	.flags = TWL4030_WAKEUP12_SCRIPT,
};

static struct twl4030_script wakeup_p3_script = {
	.script = wakeup_p3_seq,
	.size = ARRAY_SIZE(wakeup_p3_seq),
	.flags = TWL4030_WAKEUP3_SCRIPT,
};

static struct twl4030_script wrst_script  = {
	.script = wrst_seq,
	.size = ARRAY_SIZE(wrst_seq),
	.flags = TWL4030_WRST_SCRIPT,
};

static struct twl4030_script *twl4030_scripts[] = {
	&wakeup_p12_script,
	&sleep_on_script,
	&wakeup_p3_script,
	&wrst_script,
};

static struct twl4030_power_data hub_t2scripts_data = {
	.scripts = twl4030_scripts,
	.num = ARRAY_SIZE(twl4030_scripts),
	.resource_config = twl4030_rconfig,
};

static struct regulator_consumer_supply hub_vmmc1_supply = {
	.supply		= "vmmc1",
};

static struct regulator_consumer_supply hub_vsim_supply = {
	.supply		= "vmmc_aux",
};

static struct regulator_consumer_supply hub_vmmc2_supply = {
	.supply		= "vmmc2",
};

/* VMMC1 for VT_CAM_DRV_2.8V */
static struct regulator_init_data hub_vmmc1 = {
	.constraints = {
		.min_uV			= 2850000,
		.max_uV			= 2850000,
		.valid_modes_mask	= REGULATOR_MODE_NORMAL
					| REGULATOR_MODE_STANDBY,
		.valid_ops_mask		= REGULATOR_CHANGE_VOLTAGE
					| REGULATOR_CHANGE_MODE
					| REGULATOR_CHANGE_STATUS,
	},
	.num_consumer_supplies  = 1,
	.consumer_supplies      = &hub_vmmc1_supply,
};

/* VMMC2 for MMC2 card - eMMC */
static struct regulator_init_data hub_vmmc2 = {
	.constraints = {
		.min_uV			= 1850000,
		.max_uV			= 1850000,
		.apply_uV		= true,
		.valid_modes_mask	= REGULATOR_MODE_NORMAL
					| REGULATOR_MODE_STANDBY,
		.valid_ops_mask		= REGULATOR_CHANGE_MODE
					| REGULATOR_CHANGE_STATUS,
//		.always_on = true,
//		.boot_on = true,
	},
	.num_consumer_supplies  = 1,
	.consumer_supplies      = &hub_vmmc2_supply,
};

/* VSIM for 1.8V_WLAN */
static struct regulator_init_data hub_vsim = {
	.constraints = {
		.min_uV			= 1800000,
		.max_uV			= 3000000,
		.valid_modes_mask	= REGULATOR_MODE_NORMAL
					| REGULATOR_MODE_STANDBY,
		.valid_ops_mask		= REGULATOR_CHANGE_VOLTAGE
					| REGULATOR_CHANGE_MODE
					| REGULATOR_CHANGE_STATUS,
#if defined(CONFIG_MACH_LGE_HUB)  //TI_Feature ?BT
/*
           DON'T REMOVE THIS CODE WHICH IS REQUIRED FOR BT PROPER FUNCTIONALITY.
           FOR ANY CLARIFICATIONS, PLEASE CONTACT TI BT TEAM.
*/
		.always_on = true,
		.boot_on = true,
#endif
	},
	.num_consumer_supplies  = 1,
	.consumer_supplies      = &hub_vsim_supply,
};

static struct regulator_consumer_supply hub_vaux1_supply = {
	.supply		= "vaux1",
};
static struct regulator_init_data hub_vaux1= {
	.constraints = {
		.min_uV			= 2800000,
		.max_uV			= 2800000,
		.apply_uV		= true,
		.always_on		= false,
		.boot_on		= false,
		.valid_modes_mask       = REGULATOR_MODE_NORMAL
					| REGULATOR_MODE_STANDBY,
		.valid_ops_mask		= REGULATOR_CHANGE_MODE
					| REGULATOR_CHANGE_STATUS | REGULATOR_CHANGE_VOLTAGE,
	},
	.num_consumer_supplies  = 1,
	.consumer_supplies      = &hub_vaux1_supply,
};

static struct regulator_consumer_supply heaven_vaux2_supply = {
            .supply         = "vaux2",
};

static struct regulator_init_data hub_vaux2= {
	.constraints = {
		.min_uV                 = 2800000,
		.max_uV                 = 2800000,
		.apply_uV		= true,
//		.always_on = true,
		.boot_on = true,

		.valid_modes_mask       = REGULATOR_MODE_NORMAL		|
			REGULATOR_MODE_STANDBY,
		.valid_ops_mask		= REGULATOR_CHANGE_MODE 	|
			REGULATOR_CHANGE_VOLTAGE	|
			REGULATOR_CHANGE_STATUS,
	},
	.num_consumer_supplies  = 1,
	.consumer_supplies      = &heaven_vaux2_supply,
};

static struct omap2_hsmmc_info mmc[] __initdata = {
	{
		.name		= "sdcard",
		.mmc		= 1,
		.caps		= MMC_CAP_4_BIT_DATA,
		.cover_only	= true,
		.gpio_cd	= 10,
		.gpio_wp	= -EINVAL,
		.nonremovable	= false,
		.power_saving	= false,
	},
	{
		.name		= "emmc",
		.mmc		= 2,
		.caps		= MMC_CAP_4_BIT_DATA | MMC_CAP_8_BIT_DATA,
		.gpio_cd	= -EINVAL,
		.gpio_wp	= -EINVAL,
		.nonremovable	= true,
#if 0
		.power_saving	= true,
#else
		.power_saving	= false,
#endif
	},
	// - hsmmc_info data for MMC3
	{
		.mmc		= 3,
		.caps		= MMC_CAP_4_BIT_DATA,
		.gpio_wp	= -EINVAL,
		.gpio_cd	= -EINVAL,
		//.nonremovable	= true,
		//.power_saving	= true,
		//.power_saving	= false,
	},
	{}      /* Terminator */
};
static struct regulator_consumer_supply hub_vpll2_supply = {
    .supply 	= "vpll2",
};

static struct regulator_init_data hub_vpll2 = {
	.constraints = {
		.min_uV = 1800000,
		.max_uV = 1800000,
		.apply_uV = true,
		.valid_modes_mask = REGULATOR_MODE_NORMAL
			| REGULATOR_MODE_STANDBY,
		.valid_ops_mask = REGULATOR_CHANGE_MODE
			| REGULATOR_CHANGE_STATUS,
	},
	.num_consumer_supplies = 1,
	.consumer_supplies = &hub_vpll2_supply,	
};

static struct regulator_consumer_supply hub_vdac_supply = {
	.supply         = "vdac",
};

static struct regulator_init_data hub_vdac = {
	.constraints = {
		.min_uV                 = 1800000,
		.max_uV                 = 1800000,
		.apply_uV = true,
//		.always_on = true,
		.boot_on = true,
		.valid_modes_mask       = REGULATOR_MODE_NORMAL
					| REGULATOR_MODE_STANDBY,
		.valid_ops_mask         = REGULATOR_CHANGE_MODE
					| REGULATOR_CHANGE_STATUS,
	},
	.num_consumer_supplies  = 1,
	.consumer_supplies      = &hub_vdac_supply,
};

static int hub_twl_gpio_setup(struct device *dev,
		unsigned gpio, unsigned ngpio)
{
	/* gpio + 0 is "mmc0_cd" (input/IRQ) */
//	mmc[0].gpio_cd = gpio + 0;
#if 1
	twl4030_mmc_init(mmc);
#else
	omap2_hsmmc_init(mmc);
#endif

	/* link regulators to MMC adapters ... we "know" the
	 * regulators will be set up only *after* we return.
	*/
	//hub_vmmc1_supply.dev = mmc[0].dev;
	//hub_vsim_supply.dev = mmc[0].dev;
	//hub_vmmc2_supply.dev = mmc[1].dev;

	return 0;
}


static int hub_batt_table[] = {
/* 0 C*/
30800, 29500, 28300, 27100,
26000, 24900, 23900, 22900, 22000, 21100, 20300, 19400, 18700, 17900,
17200, 16500, 15900, 15300, 14700, 14100, 13600, 13100, 12600, 12100,
11600, 11200, 10800, 10400, 10000, 9630,  9280,  8950,  8620,  8310,
8020,  7730,  7460,  7200,  6950,  6710,  6470,  6250,  6040,  5830,
5640,  5450,  5260,  5090,  4920,  4760,  4600,  4450,  4310,  4170,
4040,  3910,  3790,  3670,  3550
};

static struct twl4030_bci_platform_data hub_bci_data = {
	.battery_tmp_tbl	= hub_batt_table,
	.tblsize		= ARRAY_SIZE(hub_batt_table),
};

static struct twl4030_usb_data hub_usb_data = {
	.usb_mode	= T2_USB_MODE_ULPI,
};

static struct twl4030_gpio_platform_data hub_gpio_data = {
	.gpio_base	= OMAP_MAX_GPIO_LINES,
	.irq_base	= TWL4030_GPIO_IRQ_BASE,
	.irq_end	= TWL4030_GPIO_IRQ_END,
	.setup		= hub_twl_gpio_setup,
////	.use_leds	= 1,
};

static struct twl4030_madc_platform_data hub_madc_data = {
	.irq_line	= 1,
};

static struct twl4030_codec_audio_data hub_audio_data = {
	.audio_mclk = 26000000,
	.reset_registers = 1,
	.offset_cncl_path = 0x20,
};

static struct twl4030_codec_data hub_codec_data = {
	.audio_mclk = 26000000,
	.audio = &hub_audio_data,
};

static struct twl4030_platform_data hub_twldata = {
	.irq_base	= TWL4030_IRQ_BASE,
	.irq_end	= TWL4030_IRQ_END,

	/* platform_data for children goes here */
	.bci		= &hub_bci_data,
	.madc		= &hub_madc_data,
	.usb		= &hub_usb_data,
	.gpio		= &hub_gpio_data,
	.keypad		= &hub_kp_twl4030_data,
	.codec		= &hub_codec_data,
	.power 		= &hub_t2scripts_data,
	.vmmc1		= &hub_vmmc1,
	.vmmc2		= &hub_vmmc2,
	.vsim		= &hub_vsim,
	.vpll2		= &hub_vpll2,
	.vdac		= &hub_vdac,
	.vaux1		= &hub_vaux1,
	.vaux2		= &hub_vaux2,
};

static struct i2c_board_info __initdata hub_i2c_boardinfo[] = {
	{
		I2C_BOARD_INFO("twl5030", 0x48),
		.flags		= I2C_CLIENT_WAKE,
		.irq		= INT_34XX_SYS_NIRQ,
		.platform_data	= &hub_twldata,
	},
};

static void hub_synaptics_dev_init(void)
{
	/* Set the ts_gpio pin mux */
	omap_mux_init_signal("gpio_35", OMAP_PIN_INPUT);

	if (gpio_request(HUB_TS_I2C_INT_GPIO, "touch") < 0) {
		printk(KERN_ERR "can't get synaptics pen down GPIO\n");
		return;
	}
	gpio_direction_input(HUB_TS_I2C_INT_GPIO);
//	omap_set_gpio_debounce(HUB_TS_I2C_INT_GPIO, 1);
//	omap_set_gpio_debounce_time(HUB_TS_I2C_INT_GPIO, 0xa);
	gpio_set_debounce(HUB_TS_I2C_INT_GPIO, 0xa);
}

static struct synaptics_i2c_rmi_platform_data hub_ts_synaptics_platform_data[] = {
	{
		.version	= 0x0,
		.irqflags	= IRQF_TRIGGER_FALLING,
	},
};

static struct i2c_board_info __initdata hub_i2c_bus2_info[] = {
	{
		//I2C_BOARD_INFO("hub_i2c_bl", HUB_BACKLIGHT_ADDRESS),
#if defined(CONFIG_BACKLIGHT_AAT2870)
		I2C_BOARD_INFO("aat2870_i2c_bl", AAT2870_BACKLIGHT_ADDRESS),
#endif
	},
	{
		I2C_BOARD_INFO("hub_synaptics_ts",	0x20),
		.platform_data = &hub_ts_synaptics_platform_data,
		.irq = OMAP_GPIO_IRQ(HUB_TS_I2C_INT_GPIO),
	},
#if defined(CONFIG_HUB_AMP_WM9093)
	{
	 I2C_BOARD_INFO(WM9093_I2C_NAME, WM9093_I2C_ADDR),
	 },
#endif /* CONFIG_AUDIO_AMP_WM9093 */
    //3. hub muic
    {
    	I2C_BOARD_INFO("hub_i2c_muic", 0x88>>1),
    },	
#if defined (CONFIG_LEDS_BD2802_LGE)
    {
    	I2C_BOARD_INFO("BD2802", KEY_LEDS_BD2802_ADDRESS),    
    },
#endif
#if defined (CONFIG_FUELGAUGE_MAX17043)
    {
    	I2C_BOARD_INFO("max17043", 0x36),    
    },
#endif
};


#if defined(CONFIG_MPU_SENSORS_MPU3050) || defined(CONFIG_MPU_SENSORS_MPU3050_MODULE)

#define SENSOR_MPU_NAME "mpu3050"
#define MPU_GPIO_IRQ 41
#define ACCEL_GPIO_IRQ 42

static struct mpu3050_platform_data mpu3050_data = {
    .int_config  = 0x10,
    .orientation = { 0,  -1,  0, 
                     -1,  0,  0, 
                      0,  0,  -1 },
    .level_shifter = 0,
    .accel = {
#ifdef CONFIG_MPU_SENSORS_MPU3050_MODULE
         .get_slave_descr = NULL,
#else
         .get_slave_descr = get_accel_slave_descr,
#endif
		.adapt_num   = 3,  // The i2c bus to which the mpu device is 
                           // connected
        .bus         = EXT_SLAVE_BUS_SECONDARY,
        .address     = 0x0F,
        .orientation = {  0, 1 ,  0, 
                          1, 0,  0, 
                          0,  0,  -1 },
    },
    .compass = {
#ifdef CONFIG_MPU_SENSORS_MPU3050_MODULE
        .get_slave_descr = NULL,
#else
        .get_slave_descr = get_compass_slave_descr,
#endif
		.adapt_num   = 3,  // The i2c bus to which the mpu device is 
                           // connected
        .bus         = EXT_SLAVE_BUS_PRIMARY,
        .address     = 0x0D,
        .orientation = {  0,  1,  0, 
                          1,  0,  0, 
                          0,  0,  -1},
    },
};

static void mpu3050_dev_init(void)
{
	if (gpio_request(MPU_GPIO_IRQ, "MPUIRQ") < 0) {
		printk(KERN_ERR "can't get MPUIRQ GPIO\n");
		return;
	}
	gpio_direction_input(MPU_GPIO_IRQ);
#if 0
	if (gpio_request(ACCEL_GPIO_IRQ, "ACCELIRQ") < 0) {
		printk(KERN_ERR "can't get ACCELIRQ GPIO\n");
		return;
	}
	gpio_direction_input(ACCEL_GPIO_IRQ);
#endif	
}

#endif

static struct i2c_board_info __initdata hub_i2c_bus3_info[] = {
#ifndef CONFIG_MPU_SENSORS_MPU3050
	{
		I2C_BOARD_INFO("kxtf9", 0x0F),
	},
	{
		I2C_BOARD_INFO("hub_akm8973",0x0D),
	},	
	{
		I2C_BOARD_INFO("heaven_gyro", 0x68),
	},	
#else
	{
		I2C_BOARD_INFO(SENSOR_MPU_NAME,0x68),
		.platform_data = &mpu3050_data,
		.irq = (IH_GPIO_BASE + MPU_GPIO_IRQ),
	},	

#endif 

#if defined(CONFIG_REGULATOR_LP8720)
	{
		I2C_BOARD_INFO(LP8720_I2C_NAME,  LP8720_I2C_ADDR),
		.platform_data =&lp8720_pdata,
	},
#endif

#if defined(CONFIG_GP2AP_PROXIMITY)
	{	 I2C_BOARD_INFO("hub_proxi", 0x44),	 
//		.irq = OMAP_GPIO_IRQ(14),	 
	},
#endif

#if (defined(CONFIG_VIDEO_IMX072) || defined(CONFIG_VIDEO_IMX072_MODULE)) 
	{
		I2C_BOARD_INFO("imx072", IMX072_I2C_ADDR),
		.platform_data = &hub_imx072_platform_data,
	},
		
	{
		I2C_BOARD_INFO(DW9716_NAME,  DW9716_AF_I2C_ADDR),
		.platform_data = &hub_dw9716_platform_data,
	},
#endif

};

static int __init omap_i2c_init(void)
{
	omap_register_i2c_bus(1, 400, NULL, hub_i2c_boardinfo,
			ARRAY_SIZE(hub_i2c_boardinfo));
	omap_register_i2c_bus(2, 400, NULL,hub_i2c_bus2_info,
			ARRAY_SIZE(hub_i2c_bus2_info));
	omap_register_i2c_bus(3, 400, NULL,hub_i2c_bus3_info, 
			ARRAY_SIZE(hub_i2c_bus3_info));
	return 0;
}

static struct gpio_switch_platform_data hub_headset_data = {
	.name = "h2w",
	.gpio = 170,
};

static struct platform_device lge_hub_headset_device = {
	.name		= "hub_headset",
	.id		= -1,
	.dev.platform_data = &hub_headset_data,
};
static struct platform_device hub_charging_ic_device = {
	.name = "hub_charging_ic",
	.id = -1,
};

static struct omap_uart_port_info omap_serial_platform_data[] = {
	{
		.use_dma	= 0,
		.dma_rx_buf_size = DEFAULT_RXDMA_BUFSIZE,
		.dma_rx_poll_rate = DEFAULT_RXDMA_POLLRATE,
		.dma_rx_timeout = DEFAULT_RXDMA_TIMEOUT,
		.idle_timeout	= DEFAULT_IDLE_TIMEOUT,
		.flags		= 1,
		.rts_padconf = 0x17e,
		.rts_override = 0x0,
		.cts_padconf = 0x180,
		.padconf = 0x1,
	},
	{
		.use_dma	= 0,
		.dma_rx_buf_size = DEFAULT_RXDMA_BUFSIZE,
		.dma_rx_poll_rate = DEFAULT_RXDMA_POLLRATE,
		.dma_rx_timeout = DEFAULT_RXDMA_TIMEOUT,
		.idle_timeout	= DEFAULT_IDLE_TIMEOUT,
		.flags		= 1,
		.rts_padconf = 0x176,
		.rts_override = 0x0,
		.cts_padconf = 0x174,
		.padconf = 0x1,
	},
	{
		.use_dma	= 0,
		.dma_rx_buf_size = DEFAULT_RXDMA_BUFSIZE,
		.dma_rx_poll_rate = DEFAULT_RXDMA_POLLRATE,
		.dma_rx_timeout = DEFAULT_RXDMA_TIMEOUT,
		.idle_timeout	= DEFAULT_IDLE_TIMEOUT,
		.flags		= 1,
		.rts_padconf = 0x19c,
		.rts_override = 0x0,
		.cts_padconf = 0x19a,
		.padconf = 0x1,
	},
	{
		.use_dma	= 0,
		.dma_rx_buf_size = DEFAULT_RXDMA_BUFSIZE,
		.dma_rx_poll_rate = DEFAULT_RXDMA_POLLRATE,
		.dma_rx_timeout = DEFAULT_RXDMA_TIMEOUT,
		.idle_timeout	= DEFAULT_IDLE_TIMEOUT,
		.flags		= 1,
	},
	{
		.flags		= 0
	}
};

static struct omap_musb_board_data musb_board_data = {
	.interface_type		= MUSB_INTERFACE_ULPI,
//	.mode			= MUSB_OTG,
	.mode			= MUSB_PERIPHERAL,
	.power			= 100,
};

static struct usb_mass_storage_platform_data usb_mass_storage_pdata = {
	.vendor   = "LGE",
	.product     = "Android",
	.release        = 0xffff,
};

#if (defined(CONFIG_VIDEO_IMX072) || defined(CONFIG_VIDEO_IMX072_MODULE)) 
static struct platform_device hub_rt8515_device = {
	.name		= "rt8515",
	.id			= -1,
	.dev		= {
 	.platform_data = &hub_rt8515_data,
	},
};

static struct platform_device flash_rt8515_device = {
	.name		="flash_rt8515",
	.id		= -1,
};
#endif
#ifndef CONFIG_USB_ANDROID_MASS_STORAGE
static struct platform_device mass_storage_device = {
	.name  = "usb_mass_storage",
	.id    = -1,
	.dev   = {
	    .platform_data = &usb_mass_storage_pdata,
	},
};
#endif

#ifdef CONFIG_HUB_VIBRATOR
static struct platform_device hub_vibrator_device = {
	.name	 = "hub_vibrator",
	.id		= -1,
};
#endif

static struct platform_device ers_kernel = {
	.name = "ers-kernel",
};

#ifdef CONFIG_HUB_MOTION
static struct platform_device heaven_motion_device = {
	.name = "motion_sensor",
	.id   = -1,
};
#endif

#ifdef CONFIG_LBEE9QMB_RFKILL
static struct lbee9qmb_platform_data lbee9qmb_platform = {
	.gpio_reset = 16,
#ifdef BRCM_BT_WAKE
	.gpio_hostwake = 43,
#endif	
#ifdef BRCM_HOST_WAKE
	.gpio_btwake = 52,
#endif	
};

static struct platform_device hub_bcm4329_device = {
	.name = "lbee9qmb-rfkill",
	.dev = {
		.platform_data = &lbee9qmb_platform,
	},
};

#ifdef BRCM_BT_WAKE
static struct platform_device hub_bcm4329_btwake_device = {
	.name = "lbee9qmb-rfkill_btwake",
	.dev = {
		.platform_data = &lbee9qmb_platform,
	},
};
#endif

struct platform_device bd_address_device = {
	.name = "bd_address",
	.id = -1,
};

#endif // CONFIG_LBEE9QMB_RFKILL

static struct platform_device hub_gps_gpio =
{
	.name = "hub_gps_gpio",
	.id   = -1,
};

#ifdef CONFIG_LED_SC654
static struct platform_device led_sc654=
{
	.name = "led_sc654",
	.id   = -1,
};
#endif

static struct platform_device *hub_devices[] __initdata = {
	&lge_hub_headset_device, 
	&hub_gps_gpio,
	&hub_charging_ic_device,
// Do not call mass_storage_device struct before init usb_mass_storage
#ifndef CONFIG_USB_ANDROID_MASS_STORAGE
	&mass_storage_device,
#endif	
#if (defined(CONFIG_VIDEO_IMX072) || defined(CONFIG_VIDEO_IMX072_MODULE)) 
	&flash_rt8515_device,
	&hub_rt8515_device,
#endif
#ifdef CONFIG_HUB_VIBRATOR
	&hub_vibrator_device,
#endif
	&hub_reset_keys_device,
#ifdef CONFIG_LBEE9QMB_RFKILL
	&hub_bcm4329_device,
#endif // CONFIG_LBEE9QMB_RFKILL
#ifdef CONFIG_LGE_MTC_ETA
	&hub_mtc_eta_log_device,
#endif
#ifdef CONFIG_ANDROID_RAM_CONSOLE
	&hub_ram_console_device,
#endif
	&ers_kernel,

#ifdef CONFIG_HUB_MOTION
	&heaven_motion_device,        
#endif

#ifdef CONFIG_INPUT_HALLEFFECT_BU52014HV
	&sniper_hall_effect_dock,
#endif
#ifdef CONFIG_LED_SC654
	&led_sc654,
#endif
#ifdef CONFIG_LBEE9QMB_RFKILL
#ifdef BRCM_BT_WAKE
	&hub_bcm4329_btwake_device,
#endif	
#endif // CONFIG_LBEE9QMB_RFKILL
	&bd_address_device,
};

static void __init hub_mmc1_gpio_init(void)
{
	u32 reg;

	reg = omap_ctrl_readl(OMAP36XX_CONTROL_WKUP_CTRL);
	reg |= OMAP36XX_GPIO_IO_PWRDNZ;
	omap_ctrl_writel(reg, OMAP36XX_CONTROL_WKUP_CTRL);

	reg = omap_ctrl_readl(OMAP343X_CONTROL_PBIAS_LITE);
	reg |= OMAP343X_PBIASLITEPWRDNZ1;	//power is stable
	reg &= ~OMAP343X_PBIASLITEVMODE1;	//1.8V
	omap_ctrl_writel(reg, OMAP343X_CONTROL_PBIAS_LITE);

	printk(KERN_ERR "%s\n", __func__);

//	mdelay(100);
//	omap_mux_init_signal("gpio_128", OMAP_PIN_OUTPUT);
}


#ifdef CONFIG_LGE_SPI
static struct omap2_mcspi_device_config ifxn721_mcspi1_config = {
	.turbo_mode = 0,
	.single_channel = 0,	/* 0: slave, 1: master */
};
static struct omap2_mcspi_device_config ifxn721_mcspi2_config = {
	.turbo_mode = 0,
#ifdef CONFIG_LGE_SPI_SLAVE
	.single_channel = 0,	/* 0: slave, 1: master */
#else
	.single_channel = 1,
#endif
};

#if 0
static struct ifx_spi_platform_data spi0_pd = {
	.mrdy_gpio = 127, /*OMAP_SEND*/
	.srdy_gpio = 176, /*MODEM_SEND*/
};
#endif

static struct ifx_spi_platform_data spi1_pd = {
	.mrdy_gpio = 22, /* MRDY */
	.srdy_gpio = 21, /* SRDY */
};

static struct spi_board_info hub_ipc_spi_board_info[] __initdata = {
#if 0
	{
		.modalias = "ifxn721",
		.bus_num = 1,
		.chip_select = 0,
		.max_speed_hz = 24000000,
		.platform_data = &spi0_pd,
		.controller_data = &ifxn721_mcspi1_config,
	},
#endif
	{
		.modalias = "ifxn721",
		.bus_num = 2,
		.chip_select = 0,
#ifdef CONFIG_LGE_SPI_SLAVE
//		.max_speed_hz = 12000000,
		.max_speed_hz = 24000000,
#else
		.max_speed_hz = 24000000,
#endif
		.platform_data = &spi1_pd,
		.controller_data = &ifxn721_mcspi2_config,
	},
};

static void __init hub_ipc_spi_init(void)
{
	spi_register_board_info(hub_ipc_spi_board_info, ARRAY_SIZE(hub_ipc_spi_board_info));
}
#endif /* CONFIG_LGE_SPI */

// LGE_DOM_UPDATE_S hayun.kim 2010/06/14 {
#ifdef CONFIG_SPI_TDMB
static struct omap2_mcspi_device_config lgsic2102_mcspi_config = {
	.turbo_mode = 0,
	.single_channel = 1,	/* 0: slave, 1: master */
};

static struct spi_board_info hub_tdmb_spi_board_info[] __initdata = {
	[0] = {
	       .modalias = "tdmb_lg2102",
	       .bus_num = 3,
	       .chip_select = 0,
	       .max_speed_hz = 6000*1000,
	       .controller_data = &lgsic2102_mcspi_config,
	       .irq = OMAP_GPIO_IRQ(HUB_TDMB_IRQ_GPIO),
	       },
};

static void __init hub_tdmb_spi_init(void)
{
	spi_register_board_info(hub_tdmb_spi_board_info, ARRAY_SIZE(hub_tdmb_spi_board_info));
}
#endif
// LGE_DOM_UPDATE_E hayun.kim 2010/06/14 }


#if 1
extern void __init max17043_init(void);
extern void __init bd2802_init(void);
#ifdef CONFIG_LED_SC654
extern void __init led_sc654_init(void);
#endif
static int __init sniper_rev_bc_dev_init(void)
{
	max17043_init();
	bd2802_init();
	return 0;
}
#endif

static void enable_board_wakeup_source(void)
{
	/* T2 interrupt line (keypad) */
	omap_mux_init_signal("sys_nirq",
		OMAP_WAKEUP_EN | OMAP_PIN_INPUT_PULLUP);
}

void __init hub_peripherals_init(void)
{
	hub_mmc1_gpio_init();
	omap_i2c_init();
	platform_add_devices(hub_devices, ARRAY_SIZE(hub_devices));

	hub_synaptics_dev_init();

#if defined(CONFIG_MPU_SENSORS_MPU3050) || defined(CONFIG_MPU_SENSORS_MPU3050_MODULE)
	mpu3050_dev_init();
#endif

//	omap_serial_init();
	omap_serial_init(omap_serial_platform_data);	
#ifdef CONFIG_INPUT_HALLEFFECT_BU52014HV
	sniper_hall_effect_init();
#endif
#ifdef CONFIG_PANEL_HUB
	hub_lcd_tv_panel_init();
#endif
	usb_musb_init(&musb_board_data);
	enable_board_wakeup_source();
	hub_cam_init();

#ifdef CONFIG_LGE_SPI
	hub_ipc_spi_init();
#endif

#ifdef CONFIG_SPI_TDMB
	hub_tdmb_spi_init();
#endif

}

module_init(sniper_rev_bc_dev_init)

