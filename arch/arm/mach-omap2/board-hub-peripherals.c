/*
 * Copyright (C) 2010 LG Electronics Inc.
 * Copyright (C) 2009 Texas Instruments Inc.
 *
 * Modified from mach-omap2/board-hub-peripherals.c
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
#include <linux/regulator/fixed.h>
#include <linux/mmc/host.h>
/* Added for touch screen support */
#include <linux/synaptics_i2c_rmi.h>
/* Added for twl4030 keyreset */
#include <linux/keyreset.h>

#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/mach/map.h>

#include <plat/common.h>
#include <plat/usb.h>
#include "control.h"
#include <plat/omap-serial.h>

#include <mach/board-hub.h>

#include "mux.h"
#include "hsmmc.h"
#include "common-board-devices.h"

#ifdef CONFIG_INPUT_HALLEFFECT_BU52014HV
#include <linux/bu52014hfv.h>
#endif

/* LGE_CHANGE_S, ryu.seeyeol@lge.com, 2011-01-28, Porting for Sensor Driver */
#ifdef CONFIG_MPU_SENSORS_MPU3050
#include <linux/mpu.h>
#endif
/* LGE_CHANGE_E, ryu.seeyeol@lge.com, 2011-01-28, Porting for Sensor Driver */

/* LGE_UPDATE_S, jaewoo56.lee@lge.com, Bluetooth for Broadcom */
#ifdef CONFIG_LBEE9QMB_RFKILL
#include <linux/lbee9qmb-rfkill.h>
// Bluetooth chanha.park@lge.com
#include <plat/omap-serial.h>
#endif
/* LGE_UPDATE_E, jaewoo56.lee@lge.com, Bluetooth for Broadcom */

// 20100624 junyeop.kim@lge.com, add the headset/wm9093 platform device [START_LGE]
#include <linux/switch.h>
#if defined(CONFIG_HUB_AMP_WM9093) || defined(CONFIG_HUB_HEADSET_DET)
#include <mach/wm9093.h>
#endif
// 20100624 junyeop.kim@lge.com, add the headset/wm9093 platform device [END_LGE]

#ifdef CONFIG_LGE_SPI
#include <linux/spi/ifx_n721_spi.h>
#include <plat/mcspi.h>
#include <linux/spi/spi.h>
#include <linux/delay.h>
#endif // CONFIG_LGE_SPI


// --[[ LGE_UBIQUIX_MODIFIED_START : hskim@mnbt.co.kr [2012.03.29] - TDMB
#ifdef CONFIG_LGE_BROADCAST_TDMB
#define HUB_TDMB_IRQ_GPIO	93
#endif /* CONFIG_LGE_BROADCAST_TDMB */
// --]] LGE_UBIQUIX_MODIFIED_END : hskim@mnbt.co.kr [2012.03.29] - TDMB

#ifdef CONFIG_REGULATOR_LP8720
#include <linux/regulator/lp8720.h>
static struct lp8720_platform_data lp8720_pdata = {
	.en_gpio_num         = 37,
};
extern void __init subpm_lp8720_init(void);
#endif

// 20100619 jh.koo@lge.com set GPIO for Hub [START_LGE]
//--[[ LGE_UBIQUIX_MODIFIED_START : shyun@ubiquix.com [2012.03.21] - When added to the board-ku5900-peripherals.c will be rollback.(TBD)
//#ifdef CONFIG_TOUCHSCREEN_HUB_SYNAPTICS
#if defined(CONFIG_TOUCHSCREEN_HUB_SYNAPTICS) || defined(CONFIG_TOUCHSCREEN_KU5900_SYNAPTICS)
#define HUB_TS_I2C_INT_GPIO 	 	35
#endif
//--]] LGE_UBIQUIX_MODIFIED_END : shyun@ubiquix.com [2012.03.21]- When added to the board-ku5900-peripherals.c will be rollback.(TBD)
// 20100619 jh.koo@lge.com set GPIO for Hub [END_LGE]

// 20100629 jh.koo@lge.com set backlight slave address for Hub [START_LGE]
#ifdef CONFIG_BACKLIGHT_AAT2870
#define AAT2870_BACKLIGHT_ADDRESS 	0x60
#endif
// 20100629 jh.koo@lge.com set backlight slave address for Hub [END_LGE]

/*20101115 LGE_CHANGE kyungyoon.kim@lge.com Key LED Controller*/
//--[[ LGE_UBIQUIX_MODIFIED_START : shyun@ubiquix.com [2012.03.21] - When added to the board-ku5900-peripherals.c will be rollback.(TBD)
//#ifdef CONFIG_LEDS_BD2802_LGE
#if defined(CONFIG_LEDS_BD2802_LGE) || defined(CONFIG_LEDS_BD2802_KU5900)
//--]] LGE_UBIQUIX_MODIFIED_END : shyun@ubiquix.com [2012.03.21]- When added to the board-ku5900-peripherals.c will be rollback.(TBD)
#define KEY_LEDS_BD2802_ADDRESS 	0x1A
#endif
/*20101115 LGE_CHANGE kyungyoon.kim@lge.com Key LED Controller*/

//--[[ LGE_UBIQUIX_MODIFIED_START : ymjun@mnbt.co.kr [2012.03.27] - CAM from GB
#include <media/v4l2-int-device.h>

#if (defined(CONFIG_VIDEO_IMX072) || defined(CONFIG_VIDEO_IMX072_MODULE)) 
#include <media/imx072.h>
#include <../drivers/media/video/rt8515.h>
#include <../drivers/media/video/dw9716.h>
extern struct imx072_platform_data hub_imx072_platform_data;
extern struct dw9716_platform_data hub_dw9716_platform_data;
extern struct rt8515_platform_data hub_rt8515_data;
#endif

#if (defined(CONFIG_VIDEO_YACD5B1S) || defined(CONFIG_VIDEO_YACD5B1S_MODULE)) 
#include <../drivers/media/video/yacd5b1s.h>
extern struct yacd5b1s_platform_data hub_yacd5b1s_platform_data;
#endif


// kibum.lee@lge.com 20120502 MUIC re-work
#if defined (CONFIG_MUIC)
#include <linux/muic/muic.h>
#if defined (CONFIG_MUIC_MAX14526DEEWP)
#include <linux/muic/muic_max14526deewp.h>
#elif defined (CONFIG_MUIC_TSUSBA33402)
#include<linux/muic/muic_ts5usba33402.h>
#endif 		// CONFIG_MUIC_MAX14526DEEWP
#if defined (CONFIG_DP3T)
#include <linux/switch_dp3t.h>
struct dp3t_switch_platform_data dp3t_pdata = {
	.ctrl_gpio1 = GPIO_DP3T_IN_1,
	.ctrl_gpio2	= GPIO_DP3T_IN_2,
	.ctrl_ifx_vbus_gpio	= GPIO_IFX_USB_VBUS_EN,
};

static struct platform_device dp3t_dev = {
	.name = "switch-dp3t",
	.id	  = -1,
	.dev	= {	
		.platform_data = &dp3t_pdata,
	},
};
#endif		// CONFIG_DP3T
#if defined (CONFIG_USIF)
#include <linux/switch_usif.h>
struct usif_switch_platform_data usif_pdata = {
	.ctrl_gpio = GPIO_USIF_IN_1,
};

static struct platform_device usif_dev = {
	.name = "switch-usif",
	.id	  = -1,
	.dev	= {	
		.platform_data = &usif_pdata,
	},
};
#endif		// CONFIG_USIF
#endif		// CONFIG_MUIC

// kibum.lee@lge.com 20120502 MUIC re-work

#ifdef CONFIG_VIDEO_OMAP3
extern void hub_cam_init(void);
#else
#define hub_cam_init()	NULL
#endif
//--]] LGE_UBIQUIX_MODIFIED_END : ymjun@mnbt.co.kr [2012.03.27] - CAM from GB

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
//DEJA	omap_mux_init_signal("gpio_28", OMAP_PIN_INPUT_PULLDOWN);

	gpio_request(SNIPER_HF_SOUTH_GPIO, "sniper dock south");
	gpio_direction_input(SNIPER_HF_SOUTH_GPIO);
//DEJA	omap_mux_init_signal("gpio_29", OMAP_PIN_INPUT_PULLDOWN);
}
#endif //CONFIG_INPUT_HALLEFFECT_BU52014HV

static int board_keymap[] = {
	// 20100619 jh.koo@lge.com Hub key [START_LGE]
	KEY(2, 1, KEY_TESTMODE_UNLOCK),
	KEY(0, 0, KEY_VOLUMEUP),
	KEY(1, 0, KEY_VOLUMEDOWN),
	KEY(0, 2, KEY_MENU),
	KEY(0, 1, KEY_HOME),
	KEY(1, 2, KEY_BACK),
	KEY(1, 1, KEY_SEARCH),
	KEY(2, 2, KEY_PROG3),	// Add virtual key for CP DOWN pop up message
	// 20100619 jh.koo@lge.com Hub key [END_LGE]
	KEY(2, 0, KEY_KPJPCOMMA), //This is Gesture Key
};

static struct matrix_keymap_data board_map_data = {
	.keymap			= board_keymap,
	.keymap_size		= ARRAY_SIZE(board_keymap),
};

static struct twl4030_keypad_data hub_kp_twl4030_data = {
	.keymap_data	= &board_map_data,
// 20100619 jh.koo@lge.com Hub key [START_LGE]		
	.rows		= 3,
	.cols		= 3,
// 20100619 jh.koo@lge.com Hub key [END_LGE]	
};

static int hub_reset_keys_up[] = {
        0
};

static struct keyreset_platform_data hub_reset_keys_pdata = {
	.reset_fn = NULL,
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

/*LGE_CHANGE <sunggyun.yu@lge.com excerpt from board-rx51-peripheral.c*/
static struct twl4030_ins sleep_on_seq[] = {
/*
 * Turn off everything
 */
#if 0 // LGE_CHANGE_S [daewung.kim@lge.com] 2011-02-27, Sensor LDO always ON
	{MSG_SINGULAR(DEV_GRP_NULL, RES_VDAC, RES_STATE_OFF), 15},
	{MSG_SINGULAR(DEV_GRP_NULL, RES_VAUX2, RES_STATE_OFF), 19},
#endif // LGE_CHANGE_E [daewung.kim@lge.com] 2011-02-27, Sensor LDO always ON
	{MSG_BROADCAST(DEV_GRP_NULL, RES_GRP_ALL, 1, 0, RES_STATE_SLEEP), 2},
};

static struct twl4030_ins wakeup_p12_seq[] = {
/*
 * Reenable everything
 */
//	{MSG_SINGULAR(DEV_GRP_NULL, RES_HFCLKOUT, RES_STATE_ACTIVE), 1},////
	{MSG_SINGULAR(DEV_GRP_NULL, RES_VAUX2, RES_STATE_ACTIVE), 1},
	{MSG_SINGULAR(DEV_GRP_NULL, RES_VDAC, RES_STATE_ACTIVE), 5},
	{MSG_BROADCAST(DEV_GRP_NULL, RES_GRP_ALL, 1, 0, RES_STATE_ACTIVE), 2},
};

static struct twl4030_ins wakeup_p3_seq[] = {
/*
 * Reenable everything
 */
//	{MSG_SINGULAR(DEV_GRP_NULL, RES_HFCLKOUT, RES_STATE_ACTIVE), 1},////
	{MSG_SINGULAR(DEV_GRP_NULL, RES_VAUX2, RES_STATE_ACTIVE), 1},
	{MSG_SINGULAR(DEV_GRP_NULL, RES_VDAC, RES_STATE_ACTIVE), 5},
	{MSG_BROADCAST(DEV_GRP_NULL, RES_GRP_ALL, 1, 0, RES_STATE_ACTIVE), 2},
};

static struct twl4030_ins wrst_seq[] = {
/*
 * Reset twl4030.
 * Reset VDD1 regulator.
 * Reset VDD2 regulator.
 * Reset VPLL1 regulator.
 * Enable sysclk output.
 * Reenable twl4030.
 */
	{MSG_SINGULAR(DEV_GRP_NULL, RES_RESET, RES_STATE_OFF), 2},
// 20101112 yoolje.cho@lge.com correction for warm reset [START_LGE]
#if 0	// should be removed.
	{MSG_BROADCAST(DEV_GRP_NULL, RES_GRP_ALL, 0, 1, RES_STATE_ACTIVE),
		0x13},
	{MSG_BROADCAST(DEV_GRP_NULL, RES_GRP_PP, 0, 3, RES_STATE_OFF), 0x13},
#endif
// 20101112 yoolje.cho@lge.com correction for warm reset [END_LGE]

	//LGE_CHANGE_S [sunggyun.yu@lge.com] 2011-02-08, fix for I2C3 error after warm reset
	{MSG_SINGULAR(DEV_GRP_NULL, RES_VDAC, RES_STATE_OFF), 1},
	{MSG_SINGULAR(DEV_GRP_NULL, RES_VAUX2, RES_STATE_OFF), 1},
	//LGE_CHANGE_E [sunggyun.yu@lge.com] 2011-02-08, fix for I2C3 error after warm reset

	{MSG_SINGULAR(DEV_GRP_NULL, RES_VDD1, RES_STATE_WRST), 0x13},
	{MSG_SINGULAR(DEV_GRP_NULL, RES_VDD2, RES_STATE_WRST), 0x13},
	{MSG_SINGULAR(DEV_GRP_NULL, RES_VPLL1, RES_STATE_WRST), 0x35},
	{MSG_SINGULAR(DEV_GRP_P3, RES_HFCLKOUT, RES_STATE_ACTIVE), 2},

	//LGE_CHANGE_S [sunggyun.yu@lge.com] 2011-02-08, fix for I2C3 error after warm reset
	{MSG_SINGULAR(DEV_GRP_NULL, RES_VAUX2, RES_STATE_ACTIVE), 1},
	{MSG_SINGULAR(DEV_GRP_NULL, RES_VDAC, RES_STATE_ACTIVE), 5},
	//LGE_CHANGE_E [sunggyun.yu@lge.com] 2011-02-08, fix for I2C3 error after warm reset

	{MSG_SINGULAR(DEV_GRP_NULL, RES_RESET, RES_STATE_ACTIVE), 2},
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
	{ .resource = RES_MAIN_REF, .devgroup = -1,
	  .type = 1, .type2 = -1, .remap_off = -1, .remap_sleep = -1
	},
	{ 0, 0},
};

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
		.always_on = true,//+DEJA
		.boot_on = true,//+DEJA
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
/* LGE_UPDATE_S, jaewoo56.lee@lge.com, Slow of Bluetooth UART issue */
#if defined(CONFIG_MACH_LGE_HUB)  //TI_Feature ?BT
/*
           DON'T REMOVE THIS CODE WHICH IS REQUIRED FOR BT PROPER FUNCTIONALITY.
           FOR ANY CLARIFICATIONS, PLEASE CONTACT TI BT TEAM.
*/
		.always_on = true,
		.boot_on = true,
#endif
/* LGE_UPDATE_E, jaewoo56.lee@lge.com, Slow of Bluetooth UART issue */
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

static struct regulator_consumer_supply hub_vaux2_supply = {
            .supply         = "vaux2",
};

static struct regulator_init_data hub_vaux2= {
	.constraints = {
		.min_uV                 = 2800000,
		.max_uV                 = 2800000,
		.apply_uV		= true,
/*LGE_CHANGE_S sunggyun.yu@lge.com*/
//		.always_on = true,
		.boot_on = true,
/*LGE_CHANGE_E sunggyun.yu@lge.com*/

		.valid_modes_mask       = REGULATOR_MODE_NORMAL		|
			REGULATOR_MODE_STANDBY,
		.valid_ops_mask		= REGULATOR_CHANGE_MODE 	|
			REGULATOR_CHANGE_VOLTAGE	|
			REGULATOR_CHANGE_STATUS,
	},
	.num_consumer_supplies  = 1,
	.consumer_supplies      = &hub_vaux2_supply,
};

static struct omap2_hsmmc_info mmc[] = {
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
	//20100729 Wi-Fi taewonee.kim@lge.com - [for lu3k_froyo][START]
	// - hsmmc_info data for MMC3
	{
		.mmc		= 3,
		.caps		= MMC_CAP_4_BIT_DATA,
		.gpio_wp	= -EINVAL,
		.gpio_cd	= -EINVAL,
		//.nonremovable	= true,
	},
	//20100729 Wi-Fi taewonee.kim@lge.com - [for lu3k_froyo][END]
	{}      /* Terminator */
};

//static struct regulator_consumer_supply hub_vpll2_supplies[] = {
//	REGULATOR_SUPPLY("vdds_dsi", "omapdss"),
//	REGULATOR_SUPPLY("vdds_dsi", "omapdss_dsi1"),
//};
static struct regulator_consumer_supply hub_vpll2_supplies = {
    .supply 	= "vpll2",
};

//static struct regulator_consumer_supply hub_vdac_supply =
//	REGULATOR_SUPPLY("vdda_dac", "omapdss_venc");
static struct regulator_consumer_supply hub_vdac_supply = {
	.supply         = "vdac",
};

static struct regulator_init_data hub_vpll2 = {
	.constraints = {
		.min_uV                 = 1800000,
		.max_uV                 = 1800000,
		.apply_uV               = true,
		.valid_modes_mask       = REGULATOR_MODE_NORMAL
					| REGULATOR_MODE_STANDBY,
		.valid_ops_mask         = REGULATOR_CHANGE_MODE
					| REGULATOR_CHANGE_STATUS,
	},
	.num_consumer_supplies = 1,
	.consumer_supplies = &hub_vpll2_supplies,
};

static struct regulator_init_data hub_vdac = {
	.constraints = {
		.min_uV                 = 1800000,
		.max_uV                 = 1800000,
		.apply_uV = true,
/*LGE_CHANGE_S sunggyun.yu@lge.com*/
//		.always_on = true,
		.boot_on = true,
/*LGE_CHANGE_E sunggyun.yu@lge.com*/
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
	//mmc[0].gpio_cd = gpio + 0;
	omap2_hsmmc_init(mmc);

	/* link regulators to MMC adapters ... we "know" the
	 * regulators will be set up only *after* we return.
	*/
	//hub_vmmc1_supply.dev = mmc[0].dev;	// 20100630 taehwan.kim@lge.com Hub Sensor Power
	//hub_vsim_supply.dev = mmc[0].dev;
	//hub_vmmc2_supply.dev = mmc[1].dev;	// 20100630 taehwan.kim@lge.com Hub Sensor Power

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
};

static struct twl4030_madc_platform_data hub_madc_data = {
	.irq_line	= 1,
};

static struct twl4030_codec_audio_data hub_audio_data = {
// prime@sdcmicro.com Make TWL4030 driver reset registers on init [START]
	.reset_registers = 1,
// prime@sdcmicro.com Make TWL4030 driver reset registers on init [END]
// prime@sdcmicro.com Make TWL4030 driver initialize offset cancelation(not to be overwritten with invalid value) [START]
	.offset_cncl_path = 0x20,
// prime@sdcmicro.com Make TWL4030 driver initialize offset cancelation(not to be overwritten with invalid value) [END]
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
	.vaux1		= &hub_vaux1,	// 20100716 jh.koo@lge.com Hub Vibrator Power
	.vaux2		= &hub_vaux2,	// 20100630 taehwan.kim@lge.com Hub Sensor Power
};

/* Added for touch screen support */
//--[[ LGE_UBIQUIX_MODIFIED_START : shyun@ubiquix.com [2012.03.21] - When added to the board-ku5900-peripherals.c will be rollback.(TBD)
//#ifdef CONFIG_TOUCHSCREEN_HUB_SYNAPTICS
#if defined(CONFIG_TOUCHSCREEN_HUB_SYNAPTICS) || defined(CONFIG_TOUCHSCREEN_KU5900_SYNAPTICS)
//--]] LGE_UBIQUIX_MODIFIED_END : shyun@ubiquix.com [2012.03.21]- When added to the board-ku5900-peripherals.c will be rollback.(TBD)
static void hub_synaptics_dev_init(void)
{
	int ret = -1;

	/* Set the ts_gpio pin mux */
//--[[ LGE_UBIQUIX_MODIFIED_START : shyun@ubiquix.com [2012.06.15] - DGMS - Touch not action in cpu-off mode status(OFF MDOE : Wakeup-enable / Input).
	ret = omap_mux_init_signal("gpio_35", OMAP_PIN_INPUT | OMAP_WAKEUP_EN | OMAP_OFFOUT_EN);
//	ret = omap_mux_init_signal("gpio_35", OMAP_PIN_INPUT);
//--]] LGE_UBIQUIX_MODIFIED_END : shyun@ubiquix.com [2012.06.15]- DGMS - Touch not action in cpu-off mode status(OFF MDOE : Wakeup-enable / Input).
	if(ret < 0)
		printk("[%s] omap3_mux_init_signal error [ret = %d]!\n", __func__, ret);

	if (gpio_request(HUB_TS_I2C_INT_GPIO, "touch") < 0) {
		printk(KERN_ERR "can't get synaptics pen down GPIO\n");
		return;
	}
	gpio_direction_input(HUB_TS_I2C_INT_GPIO);
// prime@sdcmicro.com Fix the compilation error in 2.6.35 kernel [START]
	gpio_set_debounce(HUB_TS_I2C_INT_GPIO, 0xa);
// prime@sdcmicro.com Fix the compilation error in 2.6.35 kernel [END]
}

static struct synaptics_i2c_rmi_platform_data hub_ts_synaptics_platform_data[] = {
	{
		.version	= 0x0,
		.irqflags	= IRQF_TRIGGER_FALLING,
	}
};
// 20100619 jh.koo@lge.com Hub touchscreen [END_LGE]
#endif

#if defined(CONFIG_MUIC)
/* LGE_SJIT 2012-01-27 [dojip.kim@lge.com] add muic platform data */
static struct muic_platform_data muic_pdata = {
	.gpio_int = MUIC_INT_GPIO,
	.gpio_ifx_vbus = GPIO_IFX_USB_VBUS_EN,
	//.gpio_mhl = NULL,	
};
#endif

static struct i2c_board_info __initdata hub_i2c_bus2_info[] = {
// 20100629 jh.koo@lge.com Hub Backlight [START_LGE]
#if defined(CONFIG_BACKLIGHT_AAT2870)
	{
		I2C_BOARD_INFO("aat2870_i2c_bl", AAT2870_BACKLIGHT_ADDRESS),
	},
#endif
// 20100629 jh.koo@lge.com Hub Backlight [END_LGE]

// 20100619 jh.koo@lge.com Hub touchscreen [START_LGE]
//--[[ LGE_UBIQUIX_MODIFIED_START : shyun@ubiquix.com [2012.03.21] - When added to the board-ku5900-peripherals.c will be rollback.(TBD)
#if defined(CONFIG_TOUCHSCREEN_HUB_SYNAPTICS) || defined(CONFIG_TOUCHSCREEN_KU5900_SYNAPTICS)
//--]] LGE_UBIQUIX_MODIFIED_END : shyun@ubiquix.com [2012.03.21]- When added to the board-ku5900-peripherals.c will be rollback.(TBD)
	{
		I2C_BOARD_INFO("hub_synaptics_ts", 0x20),
		.platform_data = &hub_ts_synaptics_platform_data,
		.irq = OMAP_GPIO_IRQ(HUB_TS_I2C_INT_GPIO),
	},
#endif
// 20100619 jh.koo@lge.com Hub touchscreen [END_LGE]

#if defined(CONFIG_HUB_AMP_WM9093)	// 20100625 junyeop.kim@lge.com wm9093(amp) [START_LGE]
	{
		I2C_BOARD_INFO(WM9093_I2C_NAME, WM9093_I2C_ADDR),
	 },
#endif /* CONFIG_AUDIO_AMP_WM9093 */	// 20100625 junyeop.kim@lge.com wm9093(amp) [END_LGE]

#if defined(CONFIG_HUB_MUIC)
    {
    	I2C_BOARD_INFO("hub_i2c_muic", 0x88>>1),
    },
#elif defined(CONFIG_MUIC)
    {
    	I2C_BOARD_INFO("hub_i2c_muic", 0x88>>1),
	.irq = OMAP_GPIO_IRQ(MUIC_INT_GPIO),
	.platform_data = &muic_pdata,			
    },
#endif

/*20101115 LGE_CHANGE kyungyoon.kim@lge.com Key LED Controller*/
//--[[ LGE_UBIQUIX_MODIFIED_START : shyun@ubiquix.com [2012.03.21] - When added to the board-ku5900-peripherals.c will be rollback.(TBD)
#if defined (CONFIG_LEDS_BD2802_LGE) || defined(CONFIG_LEDS_BD2802_KU5900)
//--]] LGE_UBIQUIX_MODIFIED_END : shyun@ubiquix.com [2012.03.21]- When added to the board-ku5900-peripherals.c will be rollback.(TBD)
    {
    	I2C_BOARD_INFO("BD2802", KEY_LEDS_BD2802_ADDRESS),    
    },
#endif
/*20101115 LGE_CHANGE kyungyoon.kim@lge.com Key LED Controller*/
#if defined (CONFIG_FUELGAUGE_MAX17043)
    {
    	I2C_BOARD_INFO("max17043", 0x36),    
    },
#endif
};

/* LGE_CHANGE_S, ryu.seeyeol@lge.com, 2011-03-03, Porting for MPLv3.3.3 */
/* LGE_CHANGE_S, ryu.seeyeol@lge.com, 2011-01-28, Porting for Sensor Driver */
#if defined(CONFIG_MPU_SENSORS_MPU3050) || defined(CONFIG_MPU_SENSORS_MPU3050_MODULE)

#define SENSOR_MPU_NAME "mpu3050"
#define MPU_GPIO_IRQ 41
/* LGE_CHANGE_S, hyun.seungjin@lge.com, 2011-03-04, Use Accel IRQ */
#define ACCEL_GPIO_IRQ 42
/* LGE_CHANGE_E, hyun.seungjin@lge.com, 2011-03-04, Use Accel IRQ */

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
/* LGE_CHANGE_S, hyun.seungjin@lge.com, 2011-03-04, Use Accel IRQ */
/* LGE_CHANGE_S, ryu.seeyeol@lge.com, 2011-04-21, Don't Use Accel IRQ */
//		.irq		 = (IH_GPIO_BASE + ACCEL_GPIO_IRQ),
/* LGE_CHANGE_E, ryu.seeyeol@lge.com, 2011-04-21, Don't Use Accel IRQ */
/* LGE_CHANGE_E, hyun.seungjin@lge.com, 2011-03-04, Use Accel IRQ */
    },
    .compass = {
#ifdef CONFIG_MPU_SENSORS_MPU3050_MODULE
	    .get_slave_descr = NULL,
#else
	    .get_slave_descr = get_compass_slave_descr,
#endif
	    .adapt_num   = 3,	// The i2c bus to which the mpu device is 
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
}

#endif
/* LGE_CHANGE_E, ryu.seeyeol@lge.com, 2011-01-28, Porting for Sensor Driver */
/* LGE_CHANGE_E, ryu.seeyeol@lge.com, 2011-03-03, Porting for MPLv3.3.3 */

// 20100624 taehwan.kim@lge.com, add the i2c3 platform device [START_LGE]
static struct i2c_board_info __initdata hub_i2c_bus3_info[] = {
/* LGE_CHANGE_S, ryu.seeyeol@lge.com, 2011-03-03, Porting for MPLv3.3.3 */
/* LGE_CHANGE_S, ryu.seeyeol@lge.com, 2011-01-28, Porting for Sensor Driver */
#if defined(CONFIG_MPU_SENSORS_MPU3050) || defined(CONFIG_MPU_SENSORS_MPU3050_MODULE)
	{
		I2C_BOARD_INFO(SENSOR_MPU_NAME,0x68),
		.platform_data = &mpu3050_data,
		.irq = (IH_GPIO_BASE + MPU_GPIO_IRQ),
	},
#endif
/* LGE_CHANGE_E, ryu.seeyeol@lge.com, 2011-01-28, Porting for Sensor Driver */
/* LGE_CHANGE_E, ryu.seeyeol@lge.com, 2011-03-03, Porting for MPLv3.3.3 */

#if defined(CONFIG_REGULATOR_LP8720)
	{
		I2C_BOARD_INFO(LP8720_I2C_NAME,  LP8720_I2C_ADDR),
		.platform_data =&lp8720_pdata,
	},
#endif

/* Proximity Sensor */
#if defined(CONFIG_BJ_PROXI_SENSOR)
	{	 
		I2C_BOARD_INFO("black_proxi", 0x44),
	},
#elif defined(CONFIG_GP2AP_PROXIMITY)
	{	 I2C_BOARD_INFO("hub_proxi", 0x44),
	},
#endif


#if 0
/* Accelerometer Sensor */
#if defined(CONFIG_BJ_KXTF9_SENSOR)
	{
		I2C_BOARD_INFO("kxtf9", 0x0F),
	},
#endif

/* Compass Sensor */
#if defined(CONFIG_BJ_COMPASS_SENSOR)
	{
		I2C_BOARD_INFO("black_akm8973",0x0D),
	},	
#endif

/* Gyro Sensor */
#if defined(CONFIG_BJ_MPU3050_SENSOR)
	{
		I2C_BOARD_INFO("heaven_gyro", 0x68),
	},	
#endif
#endif

//--[[ LGE_UBIQUIX_MODIFIED_START : ymjun@mnbt.co.kr [2012.03.27] - CAM from GB
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
#if (defined(CONFIG_VIDEO_YACD5B1S) || defined(CONFIG_VIDEO_YACD5B1S_MODULE)) 
	{
		I2C_BOARD_INFO("yacd5b1s", YACD5B1S_I2C_ADDR),
		.platform_data = &hub_yacd5b1s_platform_data,
	},
#endif
//--]] LGE_UBIQUIX_MODIFIED_END : ymjun@mnbt.co.kr [2012.03.27] - CAM from GB
};

// 20100624 taehwan.kim@lge.com, add the i2c3 platform device [END_LGE]

static int __init omap_i2c_init(void)
{
	/* Disable OMAP 3630 internal pull-ups for I2Ci */
	if (cpu_is_omap3630()) {

		u32 prog_io;

		prog_io = omap_ctrl_readl(OMAP343X_CONTROL_PROG_IO1);
		/* Program (bit 19)=1 to disable internal pull-up on I2C1 */
		prog_io |= OMAP3630_PRG_I2C1_PULLUPRESX;
		/* Program (bit 0)=1 to disable internal pull-up on I2C2 */
		prog_io |= OMAP3630_PRG_I2C2_PULLUPRESX;
		omap_ctrl_writel(prog_io, OMAP343X_CONTROL_PROG_IO1);

		prog_io = omap_ctrl_readl(OMAP36XX_CONTROL_PROG_IO2);
		/* Program (bit 7)=1 to disable internal pull-up on I2C3 */
		prog_io |= OMAP3630_PRG_I2C3_PULLUPRESX;
		omap_ctrl_writel(prog_io, OMAP36XX_CONTROL_PROG_IO2);

		prog_io = omap_ctrl_readl(OMAP36XX_CONTROL_PROG_IO_WKUP1);
		/* Program (bit 5)=1 to disable internal pull-up on I2C4(SR) */
		prog_io |= OMAP3630_PRG_SR_PULLUPRESX;
		omap_ctrl_writel(prog_io, OMAP36XX_CONTROL_PROG_IO_WKUP1);
	}

	omap_pmic_init(1, 400, "twl5030", INT_34XX_SYS_NIRQ, &hub_twldata);
	omap_register_i2c_bus(2, 400, hub_i2c_bus2_info,
			ARRAY_SIZE(hub_i2c_bus2_info));
	omap_register_i2c_bus(3, 400, hub_i2c_bus3_info,
			ARRAY_SIZE(hub_i2c_bus3_info));
	return 0;
}

// 20100624 junyeop.kim@lge.com, add the headset platform device [START_LGE]
#ifdef CONFIG_HUB_HEADSET_DET
static struct gpio_switch_platform_data hub_headset_data = {
	.name = "h2w",
	.gpio = 170,
};

static struct platform_device lge_hub_headset_device = {
	.name		= "hub_headset",
	.id		= -1,
	.dev.platform_data = &hub_headset_data,
};
#endif
// 20100624 junyeop.kim@lge.com, add the headset platform device [END_LGE]

// 20100624 taehwan.kim@lge.com, add the charger platform device [START_LGE]
#ifdef CONFIG_HUB_CHARGING_IC
static struct platform_device hub_charging_ic_device = {
	.name = "hub_charging_ic",
	.id = -1,
};
#endif
// 20100624 taehwan.kim@lge.com, add the charger platform device [END_LGE]

// LGE_CHANGE_S 20120206 daewung.kim@lge.com, UART settings
static struct omap_uart_port_info omap_serial_platform_data[] = {
	{
		.use_dma	= 0,
		.dma_rx_buf_size = DEFAULT_RXDMA_BUFSIZE,
		.dma_rx_poll_rate = DEFAULT_RXDMA_POLLRATE,
		.dma_rx_timeout = DEFAULT_RXDMA_TIMEOUT,
		.auto_sus_timeout = DEFAULT_AUTOSUSPEND_DELAY,
		.flags		= 1,
	},
	{
		.use_dma	= 0,
		.dma_rx_buf_size = DEFAULT_RXDMA_BUFSIZE,
		.dma_rx_poll_rate = DEFAULT_RXDMA_POLLRATE,
		.dma_rx_timeout = DEFAULT_RXDMA_TIMEOUT,
		.auto_sus_timeout = DEFAULT_AUTOSUSPEND_DELAY,
		.flags		= 1,
		.rts_mux_driver_control  = 1,
	},
	{
		.use_dma	= 0,
		.dma_rx_buf_size = DEFAULT_RXDMA_BUFSIZE,
		.dma_rx_poll_rate = DEFAULT_RXDMA_POLLRATE,
		.dma_rx_timeout = DEFAULT_RXDMA_TIMEOUT,
		.auto_sus_timeout = DEFAULT_AUTOSUSPEND_DELAY,
		.flags		= 1,
	},
	{
		.use_dma	= 0,
		.dma_rx_buf_size = DEFAULT_RXDMA_BUFSIZE,
		.dma_rx_poll_rate = DEFAULT_RXDMA_POLLRATE,
		.dma_rx_timeout = DEFAULT_RXDMA_TIMEOUT,
		.auto_sus_timeout = DEFAULT_AUTOSUSPEND_DELAY,
		.flags		= 1,
	},
	{
		.flags		= 0
	}
};
// LGE_CHANGE_E 20120206 daewung.kim@lge.com, UART settings

/* LGE_CHANGE_S [jjlee05@lge.com] 2010-06-28 */
static struct omap_musb_board_data musb_board_data = {
	.interface_type		= MUSB_INTERFACE_ULPI,
	.mode			= MUSB_PERIPHERAL,
	.power			= 100,
};

//--[[ LGE_UBIQUIX_MODIFIED_START : ymjun@mnbt.co.kr [2012.03.27] - CAM from GB
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
//--]] LGE_UBIQUIX_MODIFIED_END : ymjun@mnbt.co.kr [2012.03.27] - CAM from GB

/* LGE_CHANGE_E [jjlee05@lge.com] 2010-06-28 */

// 20100716 jh.koo@lge.com Hub Vibrator Power [START_LGE]
#ifdef CONFIG_HUB_VIBRATOR
static struct platform_device hub_vibrator_device = {
	.name	= "hub_vibrator",
	.id	= -1,
};
#endif
// 20100716 jh.koo@lge.com Hub Vibrator Power [END_LGE]

/* LGE_CHANGE_S, hyun.seungjin@lge.com, 2011-02-23, Move directory from driver/hub to misc */
#ifdef CONFIG_BJ_MOTION_SENSOR
/* LGE_CHANGE_S sglee76@lge.com Gyro */
static struct platform_device heaven_motion_device = {
	.name = "motion_sensor",
	.id   = -1,
};
/* LGE_CHANGE_E sglee76@lge.com Gyro */
#endif
/* LGE_CHANGE_E, hyun.seungjin@lge.com, 2011-02-23, Move directory from driver/hub to misc */

// Bluetooth chanha.park@lge.com
// LGE_SJIT_S 11/18/2011 [mohamed.khadri@lge.com] BT UART Enable
#ifdef CONFIG_LBEE9QMB_RFKILL
#define BT_UART_DEV_NAME "/dev/ttyO1"

static bool uart_req;
/* Call the uart disable of serial driver */
static int plat_bt_uart_disable(void)
{
        int port_id = 0;
        int err = 0;
        if (uart_req) {
                sscanf(BT_UART_DEV_NAME, "/dev/ttyO%d", &port_id);
                err = omap_serial_ext_uart_disable(port_id);
                if (!err)
                        uart_req = false;
        }
        return err;
}

/* Call the uart enable of serial driver */
static int plat_bt_uart_enable(void)
{
        int port_id = 0;
        int err = 0;
        if (!uart_req) {
                sscanf(BT_UART_DEV_NAME, "/dev/ttyO%d", &port_id);
                err = omap_serial_ext_uart_enable(port_id);
                if (!err)
                        uart_req = true;
        }
        return err;
}

static struct lbee9qmb_platform_data lbee9qmb_platform = {
	.gpio_reset = 16,
	.gpio_btwake = 52,
	.gpio_hostwake = 43,
	.active_low = 0,
	.delay = 10,
// LGE_SJIT_S 11/18/2011 [mohamed.khadri@lge.com] BT UART Enable
        .chip_enable = plat_bt_uart_enable,
        .chip_disable = plat_bt_uart_disable,
// LGE_SJIT_E 11/18/2011 [mohamed.khadri@lge.com] BT UART Enable

};

static struct platform_device black_bcm4329_device = {
	.name = "lbee9qmb-rfkill",
	.dev = {
		.platform_data = &lbee9qmb_platform,
	},
};
// LGE_SJIT_E 11/18/2011 [mohamed.khadri@lge.com] BT UART Enable
#endif/* CONFIG_LBEE9QMB_RFKILL */

// 20101121 BT: dohyung10.lee@lge.com - For the BD Address Read /write [Start]
#ifdef CONFIG_BD_ADDRESS
struct platform_device bd_address_device = {
	.name = "bd_address",
	.id = -1,
};
#endif
// 20101121 BT: dohyung10.lee@lge.com - For the BD Address Read /write [End]


/* LGE_UPDATE_S [daewung.kim@lge.com] 2010-09-14, GPS_PORTING */
#ifdef CONFIG_HUB_GPS_GPIO
static struct platform_device hub_gps_gpio =
{
	.name = "hub_gps_gpio",
	.id   = -1,
};
#endif
/* LGE_UPDATE_E [daewung.kim@lge.com] 2010-09-14 */

/* B-Prj Key LED Added [kyungyoon.kim@lge.com] 2010-10-29 */
#ifdef CONFIG_LED_SC654
static struct platform_device led_sc654=
{
	.name = "led_sc654",
	.id   = -1,
};
#endif
/* B-Prj Key LED Added [kyungyoon.kim@lge.com] 2010-10-29 */

/* 2011_01_13 by seunghyun.yi@lge.com for FOTA AP - > CP UART4 interface [START]*/
#ifdef CONFIG_HUB_MODEM_CONTROL
static struct platform_device hub_modem_device=
{
		.name = "modem_ctrl",
		.id 	= -1,
};
#endif // CONFIG_HUB_MODEM_CONTROL
/* 2011_01_13 by seunghyun.yi@lge.com for FOTA AP - > CP UART4 interface [END]*/  

static struct platform_device *hub_devices[] __initdata = {
/* 20100326 junyeop.kim@lge.com for headset device */
#ifdef CONFIG_HUB_HEADSET_DET
	&lge_hub_headset_device,
#endif

/* LGE_CHANGE_S [daewung.kim@lge.com] 2010-09-14, GPS_PORTING */
#ifdef CONFIG_HUB_GPS_GPIO
	&hub_gps_gpio,
#endif
/* LGE_UPDATE_E [daewung.kim@lge.com] 2010-09-14 */

// 20100624 taehwan.kim@lge.com, add the charger platform device [START_LGE]
#ifdef CONFIG_HUB_CHARGING_IC
	&hub_charging_ic_device,
#endif
// 20100624 taehwan.kim@lge.com, add the charger platform device [END_LGE]

//--[[ LGE_UBIQUIX_MODIFIED_START : ymjun@mnbt.co.kr [2012.03.27] - CAM from GB
#if (defined(CONFIG_VIDEO_IMX072) || defined(CONFIG_VIDEO_IMX072_MODULE)) 
	&flash_rt8515_device, 		// 20100426 hyungwoo.ku@lge.com Flash driver for camera
	&hub_rt8515_device,		// 20100426 hyungwoo.ku@lge.com Flash driver for camera
#endif
//--]] LGE_UBIQUIX_MODIFIED_END : ymjun@mnbt.co.kr [2012.03.27] - CAM from GB

// 20100716 jh.koo@lge.com Hub Vibrator device
#ifdef CONFIG_HUB_VIBRATOR
	&hub_vibrator_device,
#endif
	
	&hub_reset_keys_device,

/* LGE_UPDATE_S, jaewoo56.lee@lge.com, Bluetooth for Broadcom */
#ifdef CONFIG_LBEE9QMB_RFKILL
	&black_bcm4329_device,
#endif // CONFIG_LBEE9QMB_RFKILL
/* LGE_UPDATE_E, jaewoo56.lee@lge.com, Bluetooth for Broadcom */

/* LGE_CHANGE_S, hyun.seungjin@lge.com, 2011-02-23, Move directory from driver/hub to misc */
/* LGE_CHANGE_S sglee76@lge.com Gyro */
#ifdef CONFIG_BJ_MOTION_SENSOR
	&heaven_motion_device,
#endif
/* LGE_CHANGE_E sglee76@lge.com Gyro */
/* LGE_CHANGE_E, hyun.seungjin@lge.com, 2011-02-23, Move directory from driver/hub to misc */

#ifdef CONFIG_INPUT_HALLEFFECT_BU52014HV
	&sniper_hall_effect_dock,
#endif

/* B-Prj Key LED Added [kyungyoon.kim@lge.com] 2010-10-29 */
#ifdef CONFIG_LED_SC654
	&led_sc654,
#endif
/* B-Prj Key LED Added [kyungyoon.kim@lge.com] 2010-10-29 */



// 20101121 BT: dohyung10.lee@lge.com - For the BD Address Read /write [Start]
#ifdef CONFIG_BD_ADDRESS
	&bd_address_device,
#endif
// 20101121 BT: dohyung10.lee@lge.com - For the BD Address Read /write [End]

/* 2011_01_13 by seunghyun.yi@lge.com for FOTA AP - > CP UART4 interface [Start]*/ 
#ifdef CONFIG_HUB_MODEM_CONTROL
	&hub_modem_device,
#endif
/* 2011_01_13 by seunghyun.yi@lge.com for FOTA AP - > CP UART4 interface [End]*/

// kibum.lee@lge.com 20120502 MUIC re-work
#if defined (CONFIG_DP3T)
	&dp3t_dev,
#endif
#if defined (CONFIG_USIF)
	&usif_dev,
#endif	
// kibum.lee@lge.com 20120502 MUIC re-work
};

/* sunggyun.yu@lge.com for GPIO_126, 127, 129 */
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
#ifdef CONFIG_LGE_SPI_SLAVE /* 20110308 dongyu.gwak@lge.com Lab3 SPI Slave Compile*/
	.single_channel = 0,	/* 0: slave, 1: master */
#else
//LGE_UPDATED_S [eungbo.shim.lge.com] -- Changed Master OMAP3-IFX For A-Project 
	.single_channel = 1,
//LGE_UPDATED_E [eungbo.shim.lge.com] -- Changed Master OMAP3-IFX For A-Project	
#endif
};

static struct ifx_spi_platform_data spi1_pd = {
	.mrdy_gpio = 22, /* MRDY */
	.srdy_gpio = 21, /* SRDY */
};

static struct spi_board_info hub_ipc_spi_board_info[] __initdata = {
	{
		.modalias = "ifxn721",
		.bus_num = 2,
		.chip_select = 0,
#ifdef CONFIG_LGE_SPI_SLAVE /* 20110308 dongyu.gwak@lge.com */
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

// --[[ LGE_UBIQUIX_MODIFIED_START : hskim@mnbt.co.kr [2012.03.29] - TDMB
#ifdef CONFIG_LGE_BROADCAST_TDMB
#define SPI_TDMB_BUS_NUM 3

#ifdef CONFIG_LGE_BROADCAST_FC8050
static struct omap2_mcspi_device_config fc8050_mcspi_config = {
	.turbo_mode = 0,
	.single_channel = 1,	// 0: slave, 1: master
};

static struct spi_board_info hub_tdmb_spi_board_info[] __initdata = {
	[0] = {
	       .modalias = "tdmb_fc8050",	
	       .bus_num = SPI_TDMB_BUS_NUM,
	       .chip_select = 0,	
	       .max_speed_hz = 24000*1000,	
	       .controller_data = &fc8050_mcspi_config,	
	       .irq = OMAP_GPIO_IRQ(HUB_TDMB_IRQ_GPIO),	
	      },
};
#endif /* CONFIG_LGE_BROADCAST_FC8050 */

static void __init hub_tdmb_spi_init(void)
{
	spi_register_board_info(hub_tdmb_spi_board_info, ARRAY_SIZE(hub_tdmb_spi_board_info));
}
#endif /* CONFIG_LGE_BROADCAST_TDMB */
// --]] LGE_UBIQUIX_MODIFIED_END : hskim@mnbt.co.kr [2012.03.29] - TDMB

// prime@sdcmicro.com Added function to initialize SYS_NIRQ pin [START]
static void enable_board_wakeup_source(void)
{
	/* T2 interrupt line (keypad) */
	omap_mux_init_signal("sys_nirq",
		OMAP_WAKEUP_EN | OMAP_PIN_INPUT_PULLUP);
	omap_mux_init_signal("gpio_43",
		OMAP_INPUT_EN | OMAP_OFF_EN | OMAP_OFFOUT_EN | OMAP_PIN_OFF_WAKEUPENABLE);
}
// prime@sdcmicro.com Added function to initialize SYS_NIRQ pin [END]

void __init hub_peripherals_init(void)
{
	hub_mmc1_gpio_init();

	omap_i2c_init();

#ifdef CONFIG_REGULATOR_LP8720
	subpm_lp8720_init();
#endif
	platform_add_devices(hub_devices, ARRAY_SIZE(hub_devices));

//--[[ LGE_UBIQUIX_MODIFIED_START : shyun@ubiquix.com [2012.03.21] - When added to the board-ku5900-peripherals.c will be rollback.(TBD)
//#ifdef CONFIG_TOUCHSCREEN_HUB_SYNAPTICS
#if defined(CONFIG_TOUCHSCREEN_HUB_SYNAPTICS) || defined(CONFIG_TOUCHSCREEN_KU5900_SYNAPTICS)
	hub_synaptics_dev_init();
#endif
//--]] LGE_UBIQUIX_MODIFIED_END : shyun@ubiquix.com [2012.03.21]- When added to the board-ku5900-peripherals.c will be rollback.(TBD)
/* LGE_CHANGE_S, ryu.seeyeol@lge.com, 2011-03-03, Porting for MPLv3.3.3 */
#if defined(CONFIG_MPU_SENSORS_MPU3050) || defined(CONFIG_MPU_SENSORS_MPU3050_MODULE)
	mpu3050_dev_init();
#endif
/* LGE_CHANGE_E, ryu.seeyeol@lge.com, 2011-03-03, Porting for MPLv3.3.3 */

	omap_serial_board_init(omap_serial_platform_data);

#ifdef CONFIG_INPUT_HALLEFFECT_BU52014HV
	sniper_hall_effect_init();
#endif
#ifdef CONFIG_PANEL_HUB
	hub_display_init();
#endif
	usb_musb_init(&musb_board_data);
	enable_board_wakeup_source();

//--[[ LGE_UBIQUIX_MODIFIED_START : ymjun@mnbt.co.kr [2012.03.27] - CAM from GB
	hub_cam_init();
//--]] LGE_UBIQUIX_MODIFIED_END : ymjun@mnbt.co.kr [2012.03.27] - CAM from GB

#ifdef CONFIG_LGE_SPI
	hub_ipc_spi_init();
#endif

//--[[ LGE_UBIQUIX_MODIFIED_START : hskim@mnbt.co.kr [2012.03.29] - TDMB
#ifdef CONFIG_LGE_BROADCAST_TDMB
	hub_tdmb_spi_init();
#endif
//--]] LGE_UBIQUIX_MODIFIED_END : hskim@mnbt.co.kr [2012.03.29] - TDMB
}

