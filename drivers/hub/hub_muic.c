/*
 * Sniper MAX14526 & TS5USBA33402 multi MUIC driver
 *
 * Copyright (C) 2010 LGE, Inc.
 *
 * Author: Sookyoung Kim <sookyoung.kim@lge.com>
 *
 * Modified: 2010-11-17, Add MAX14526 control code <daewung.kim@lge.com>
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

#include <linux/module.h>
#include <linux/kernel.h>	// printk()
#include <linux/init.h>		// __init, __exit
#include <linux/uaccess.h>	// copy_from/to_user()
#include <linux/interrupt.h>	// request_irq()
#include <linux/irq.h>		// set_irq_type()
#include <linux/types.h>	// kernel data types
#include <asm/system.h>
// kernel/arch/arm/include/asm/gpio.h includes kernel/arch/arm/plat-omap/include/mach/gpio.h which,
// in turn, includes kernel/include/asm-generic/gpio.h.
// <mach/gpio.h> declares gpio_get|set_value(), gpio_to_irq().
// <asm-generic/gpio.h> declares struct gpio_chip, gpio_request(), gpio_free(), gpio_direction_input|output().
// The actual descriptions are in kernel/drivers/gpio/gpiolib.c and kernel/arch/arm/plat-omap/gpio.c.
#include <asm/gpio.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/proc_fs.h>
#include <linux/workqueue.h>	// INIT_WORK()
#include <linux/wakelock.h>

#include <linux/i2c/twl.h>	// chager_state_update_by_other()

#include "hub_muic.h"
#include "hub_charging_ic.h"
#include "../mux.h"


#define TS5USBA33402	0x10
#define MAX14526	0x20

/* LGE_CHANGE_S [kenneth.kang@lge.com] 2010-12-14, CP retain mode and 910K cable detect*/
#define CABLE_DETECT_910K
//#define CP_RETAIN	//LGE_UPDATE [jaejoong.kim] disable CP_RETAIN
/* LGE_CHANGE_E [kenneth.kang@lge.com] 2010-12-14, CP retain mode and 910K cable detect*/

static const char name_muic_mode[MUIC_MODE_NO][30] = {
	"MUIC_UNKNOWN",		// 0
	"MUIC_NONE",   		// 1
	"MUIC_NA_TA",   	// 2
	"MUIC_LG_TA",   	// 3
	"MUIC_HCHH", 	  	// 4
	"MUIC_INVALID_CHG",  	// 5
	"MUIC_AP_UART",   	// 6
	"MUIC_CP_UART",		// 7
	"MUIC_AP_USB", 		// 8
	"MUIC_CP_USB",		// 9
	"MUIC_TV_OUT_NO_LOAD",	// 10
	"MUIC_EARMIC",		// 11
	"MUIC_TV_OUT_LOAD",	// 12
	"MUIC_OTG",   		// 13
	"MUIC_DESK_CRADLE",	// 14
	"MUIC_RESERVE1",	// 15
	"MUIC_RESERVE2",	// 16
};

static struct i2c_client *muic_client;
static struct delayed_work muic_wq;

static u8 int_stat_val;
static u8 status_val;
static u8 muic_device = TS5USBA33402;

TYPE_USIF_MODE usif_mode = USIF_AP;
TYPE_DP3T_MODE dp3t_mode = DP3T_NC;
TYPE_UPON_IRQ  upon_irq = NOT_UPON_IRQ;
TYPE_MUIC_MODE muic_mode = MUIC_UNKNOWN;

extern s32 key_pressed;
extern s32 key_row;
extern s32 key_col;
static s32 key_was_pressed = 0;

/* kibum.lee@lge.com compile error ICS */
//extern void charger_state_update_by_other(void);

/* LGE_CHANGE_S [kenneth.kang@lge.com] 2010-12-14, CP retain mode */
#ifdef CP_RETAIN
static int is_cp_retained;
#endif
static int hidden_menu_switching;
/* LGE_CHANGE_E [kenneth.kang@lge.com] 2010-12-14, CP retain mode */

/* LGE_CHANGE_START 2011-03-16 kenneth.kang@lge.com patch for Adb offline set and Mass Storage Driver detecting fail */    
static int muic_init_done=0;     // 20110113 ks.kwon@lge.com check muic driver init. state
/* LGE_CHANGE_END 2011-03-16 kenneth.kang@lge.com */

/* Wake lock for usb connection */
struct wlock {
	int wake_lock_on;
	struct wake_lock wake_lock;
};
static struct wlock the_wlock;

void dp3t_switch_ctrl(TYPE_DP3T_MODE mode);
void usif_switch_ctrl(TYPE_USIF_MODE mode);
s32 muic_AP_UART_set(void);
s32 muic_CP_UART_set(void);
s32 muic_AP_USB_set(void);
s32 muic_CP_USB_set(void);



#ifdef CONFIG_PROC_FS
/*--------------------------------------------------------------------
 * BEGINS: Proc file system related functions for MUIC.
 *--------------------------------------------------------------------
 */
#define	HUB_MUIC_PROC_FILE "driver/hmuic"
static struct proc_dir_entry *hub_muic_proc_file;

static ssize_t muic_proc_read(struct file *filp, char *buf, size_t len, loff_t *offset){
	s32 i;
	u32 val;

	for(i=0; i<=5; i++) {
		val = i2c_smbus_read_byte_data(muic_client, i);
		printk(KERN_INFO "[MUIC] Reg 0x%X, Val=0x%X\n", i, val);
	}
	return 0;
}

static ssize_t muic_proc_write(struct file *filp, const char *buf, size_t len, loff_t *off){
	char messages[12];
	u32 val;
	u32 reg;
	s32 err;
	char cmd;

	if(len > 12)
		len = 12;

	if(copy_from_user(messages, buf, len))
		return -EFAULT;

	sscanf(buf, "%c 0x%x 0x%x", &cmd, &reg, &val);

	printk(KERN_INFO "[MUIC] LGE: MUIC_proc_write \n");


	switch(cmd){
// LGE_UPDATE 20110311 [jaejoong.kim@lge.com] add CP Image Downloadfor Hidden Menu - AP CP USB Swithcing
		/* CP Image Download */
		case '3':
                        muic_CP_USB_set();
                        gpio_set_value(26, 0);
			mdelay(100);
                        gpio_set_value(26, 1);
                        break;
// LGE_UPDATE 20110311 [jaejoong.kim@lge.com] add CP Image Downloadfor Hidden Menu - AP CP USB Swithcing

		/* AP_UART mode*/
		case '6':
			muic_AP_UART_set();
			break;
/* LGE_CHANGE_S [kenneth.kang@lge.com] 2011-01-06, CP retain mode and Hidden Menu AP CP Switching Retain */
		/* CP_UART mode*/
		case '7':
			muic_CP_UART_set();
			hidden_menu_switching = 7;
			break;

		/* AP_USB mode*/
		case '8':
			muic_AP_USB_set();
			hidden_menu_switching = 8;
			break;

		/* CP_USB mode*/
		case '9':
			muic_CP_USB_set();
			hidden_menu_switching = 9;
			break;
		case 'w':
			err = i2c_smbus_write_byte_data(muic_client, reg, val);
			printk(KERN_INFO "[MUIC] LGE: Hub MUIC Write Reg. = 0x%X, Value 0x%X\n", reg, val);
			break;

		default :
			printk(KERN_INFO "[MUIC] LGE: Hub MUIC invalid command\n");
		        printk(KERN_INFO "[MUIC] 6: AP_UART, 7: CP_UART, 8: AP_USB, 9: CP_USB\n");
		        printk(KERN_INFO "[MUIC] or \"w address value\"\n");
			break;
	}
	return len;
}

static struct file_operations hub_muic_proc_ops = {
	.read = muic_proc_read,
	.write = muic_proc_write,
};

static void create_hub_muic_proc_file(void){
	hub_muic_proc_file = create_proc_entry(HUB_MUIC_PROC_FILE, 0777, NULL);
	if(hub_muic_proc_file) {
		//hub_muic_proc_file->owner = THIS_MODULE; // 20100617 kernel API changed
		hub_muic_proc_file->proc_fops = &hub_muic_proc_ops;
	} else
		printk(KERN_INFO "[MUIC] LGE: Hub MUIC proc file create failed!\n");
}

static void remove_hub_muic_proc_file(void){
	remove_proc_entry(HUB_MUIC_PROC_FILE, NULL);
}

/*--------------------------------------------------------------------
 * ENDS: Proc file system related functions for MUIC.
 *--------------------------------------------------------------------
 */
#endif //CONFIG_PROC_FS

TYPE_MUIC_MODE get_muic_mode(void){
	return muic_mode;
}
EXPORT_SYMBOL(get_muic_mode);

/*
 * Function: Read the MUIC register whose internal address is addr
 *           and save the u8 content into value.
 * Return value: Negative errno upon failure, 0 upon success.
 */
static s32 muic_i2c_read_byte(u8 addr, u8 *value){
	*value = i2c_smbus_read_byte_data(muic_client, (u8)addr);
	if(*value < 0){
		printk(KERN_INFO "[MUIC] muic_i2c_read_byte failed.\n");
		return *value;
	}

	return 0;
}

/*
 * Function: Write u8 value to the MUIC register whose internal address is addr.
 * Return value: Negative errno upon failure, 0 upon success.
 */
static s32 muic_i2c_write_byte(u8 addr, u8 value){
	s32 ret;
	ret = i2c_smbus_write_byte_data(muic_client, (u8)addr, (u8)value);
	if(ret < 0) printk(KERN_INFO "[MUIC] muic_i2c_write_byte failed.\n");
	return ret;
}

static void set_wakelock(u32 set) {

	if (set > 0) {
		if(!the_wlock.wake_lock_on)
			wake_lock(&the_wlock.wake_lock);
	} else {
		if(the_wlock.wake_lock_on)
			wake_unlock(&the_wlock.wake_lock);
	}
	the_wlock.wake_lock_on = set;

	printk(KERN_WARNING "[MUIC] wake_lock : %s\n", (set ? "on" : "off"));
}

void usif_switch_ctrl(TYPE_USIF_MODE mode){

/* LGE_CHANGE_START 2011-03-16 kenneth.kang@lge.com patch for Adb offline set and Mass Storage Driver detecting fail */    
	/* 20110113 ks.kwon@lge.com check muic driver init. state [START] */
	if(!muic_init_done){

		printk(KERN_WARNING "[MUIC] MUIC has not been initialized! Nothing will be done!!!.\n");
		//return 0; 
		return; // 20120213 taeju.park@lge.com To delete compile warning, void return
	}	
    /* 20110113 ks.kwon@lge.com check muic driver init. state [END] */
/* LGE_CHANGE_END 2011-03-16 kenneth.kang@lge.com */    	

	if(mode == USIF_AP){
		gpio_set_value(USIF_IN_1_GPIO, 0);
	}
	else if(mode == USIF_DP3T){
		gpio_set_value(USIF_IN_1_GPIO, 1);
	}
	else{
		/* Just keep the current path */
	}
	usif_mode = mode;
}
EXPORT_SYMBOL(usif_switch_ctrl);

void dp3t_switch_ctrl(TYPE_DP3T_MODE mode){
	if(mode == DP3T_AP_UART){
		gpio_set_value(IFX_VBUS_EN, 0);
		gpio_set_value(DP3T_IN_1_GPIO, 1);
		gpio_set_value(DP3T_IN_2_GPIO, 0);
	}
	else if(mode == DP3T_CP_UART){
		gpio_set_value(IFX_VBUS_EN, 0);
		gpio_set_value(DP3T_IN_1_GPIO, 0);
		gpio_set_value(DP3T_IN_2_GPIO, 1);
	}
	else if(mode == DP3T_CP_USB){
		gpio_set_value(IFX_VBUS_EN, 1);
		gpio_set_value(DP3T_IN_1_GPIO, 1);
		gpio_set_value(DP3T_IN_2_GPIO, 1);
	}
	else if(mode == DP3T_NC){
		gpio_set_value(IFX_VBUS_EN, 0);
		gpio_set_value(DP3T_IN_1_GPIO, 0);
		gpio_set_value(DP3T_IN_2_GPIO, 0);
	}
	else{
		/* Just keep the current path */
	}
	dp3t_mode = mode;
}

s32 muic_AP_UART_set(void){

	s32 ret;
/* LGE_CHANGE_START 2011-03-16 kenneth.kang@lge.com patch for Adb offline set and Mass Storage Driver detecting fail */    
	/* 20110113 ks.kwon@lge.com check muic driver init. state [START] */
	if(!muic_init_done){

		printk(KERN_WARNING "[MUIC] MUIC has not been initialized! Nothing will be done!!!.\n");
		return 0;
	}	
    /* 20110113 ks.kwon@lge.com check muic driver init. state [END] */
/* LGE_CHANGE_END 2011-03-16 kenneth.kang@lge.com */

	/* Connect CP UART signals to AP */
	//usif_switch_ctrl(USIF_AP);
	/* Connect AP UART to MUIC UART */

	// kibum.lee@lge.com ICS temp
	gpio_direction_output(IFX_VBUS_EN, 1);
	gpio_direction_output(DP3T_IN_1_GPIO, 1);
	gpio_direction_output(DP3T_IN_2_GPIO, 1);
	
	
	dp3t_switch_ctrl(DP3T_AP_UART);

	if (muic_device == MAX14526) {
		/* ID_200, VLDO 2.6V, ADC Enable, Charge Pump.*/
		ret = muic_i2c_write_byte(CONTROL_1, MID_200 | MADC_EN | MCP_EN);
	}
	else {
		/* ID_200, VLDO 2.6V, SEMREN on. ADC is auto.*/
		ret = muic_i2c_write_byte(CONTROL_1, MID_200 | MSEMREN);
	}
	/* Connect DP, DM to UART_TX, UART_RX*/
	ret = muic_i2c_write_byte(SW_CONTROL, DP_UART | DM_UART);

	/* Turn on charger IC with FACTORY mode */
	charging_ic_set_factory_mode();

	muic_mode = MUIC_AP_UART;
	printk(KERN_WARNING "[MUIC] muic_distinguish_vbus_accessory(): AP_UART\n");

	set_wakelock(0);

	return ret;
}

s32 muic_AP_USB_set(void){

	s32 ret;
/* LGE_CHANGE_START 2011-03-16 kenneth.kang@lge.com patch for Adb offline set and Mass Storage Driver detecting fail */    
	/* 20110113 ks.kwon@lge.com check muic driver init. state [START] */
	if(!muic_init_done){

		printk(KERN_WARNING "[MUIC] MUIC has not been initialized! Nothing will be done!!!.\n");
		return 0;
	}	
    /* 20110113 ks.kwon@lge.com check muic driver init. state [END] */
/* LGE_CHANGE_END 2011-03-16 kenneth.kang@lge.com */

	/* Connect CP UART signals to AP */
	//usif_switch_ctrl(USIF_AP);

	/* AP USB does not pass through DP3T.
	 * Just connect AP UART to MUIC UART. */
	dp3t_switch_ctrl(DP3T_AP_UART);

	if (muic_device == MAX14526) {
		/* ID_200, VLDO 2.6V, ADC Enable,
		 * USB 2.0 also needs the charge pump (CP_EN) on.*/
		ret = muic_i2c_write_byte(CONTROL_1, MID_200 | MADC_EN | MCP_EN);
	}
	else {
		ret = muic_i2c_write_byte(SW_CONTROL, DP_OPEN | DM_OPEN);
		/* ID_200, VLDO 2.6V, SEMREN on. ADC is auto.
		 * USB 2.0 also needs the charge pump (CP_EN) on.*/
		ret = muic_i2c_write_byte(CONTROL_1, MID_200 | MSEMREN | MCP_EN);
	}
	/* Connect DP, DM to USB_DP, USB_DM */
	ret = muic_i2c_write_byte(SW_CONTROL, DP_USB | DM_USB);

	/* Turn on charger IC with TA mode */
	charging_ic_set_usb_mode();

	muic_mode = MUIC_AP_USB;
	printk(KERN_WARNING "[MUIC] muic_distinguish_vbus_accessory(): AP_USB\n");

	/* wake lock for usb connection */
	set_wakelock(1);

	return ret;
}

s32 muic_CP_UART_set(void){

	s32 ret;

/* LGE_CHANGE_START 2011-03-16 kenneth.kang@lge.com patch for Adb offline set and Mass Storage Driver detecting fail */    
	/* 20110113 ks.kwon@lge.com check muic driver init. state [START] */
	if(!muic_init_done){

		printk(KERN_WARNING "[MUIC] MUIC has not been initialized! Nothing will be done!!!.\n");
		return 0;
	}	
    /* 20110113 ks.kwon@lge.com check muic driver init. state [END] */
/* LGE_CHANGE_END 2011-03-16 kenneth.kang@lge.com */
	/* Connect CP UART signals to DP3T */
	//usif_switch_ctrl(USIF_DP3T);
	/* Connect CP UART to MUIC UART */
	dp3t_switch_ctrl(DP3T_CP_UART);

	if (muic_device == MAX14526) {
		/* ID_200, VLDO 2.6V, ADC is auto, Charge Pump.*/
		ret = muic_i2c_write_byte(CONTROL_1, MID_200 | MADC_EN | MCP_EN);
	}
	else {
		/* ID_200, VLDO 2.6V, SEMREN on. ADC is auto.*/
		ret = muic_i2c_write_byte(CONTROL_1, MID_200 | MSEMREN);
	}
	/* Connect DP, DM to UART_TX, UART_RX*/
	ret = muic_i2c_write_byte(SW_CONTROL, DP_UART | DM_UART);

	/* Turn on charger IC with FACTORY mode */
	charging_ic_set_factory_mode();

	muic_mode = MUIC_CP_UART;
	printk(KERN_WARNING "[MUIC] muic_distinguish_vbus_accessory(): CP_UART\n");

	/* wake lock for the factory mode */
/* LGE_CHANG_START 2011-03-30 kenneth.kang@lge.com wakelock on when AT command sequence by CP UART */
	set_wakelock(1);
/* LGE_CHANGE_END 2011-03-30 kenneth.kang@lge.com */

	return ret;
}

s32 muic_CP_USB_set(void){

	s32 ret;
/* LGE_CHANGE_START 2011-03-16 kenneth.kang@lge.com patch for Adb offline set and Mass Storage Driver detecting fail */    
	/* 20110113 ks.kwon@lge.com check muic driver init. state [START] */
	if(!muic_init_done){

		printk(KERN_WARNING "[MUIC] MUIC has not been initialized! Nothing will be done!!!.\n");
		return 0;
	}	
    /* 20110113 ks.kwon@lge.com check muic driver init. state [END] */
/* LGE_CHANGE_END 2011-03-16 kenneth.kang@lge.com */

	/* Connect CP UART signals to AP */
	//usif_switch_ctrl(USIF_AP);
	/* Connect CP USB to MUIC UART */
	dp3t_switch_ctrl(DP3T_CP_USB);

	if (muic_device == MAX14526) {
		/* ID_200, VLDO 2.6V, ADC is auto.
		 * USB 2.0 also needs the charge pump (CP_EN) on.*/
		ret = muic_i2c_write_byte(CONTROL_1, MID_200 | MADC_EN | MCP_EN);
	}
	else {
		ret = muic_i2c_write_byte(SW_CONTROL, DP_OPEN | DM_OPEN);
		/* ID_200, VLDO 2.6V, SEMREN on. ADC is auto.
		 * USB 2.0 also needs the charge pump (CP_EN) on.*/
		ret = muic_i2c_write_byte(CONTROL_1, MID_200 | MSEMREN | MCP_EN);
	}

	/* Connect DP, DM to UART_TX, UART_RX*/
	ret = muic_i2c_write_byte(SW_CONTROL, DP_UART | DM_UART);

	/* Turn on charger IC with FACTORY mode */
	charging_ic_set_factory_mode();

	muic_mode = MUIC_CP_USB;
	printk(KERN_ERR "[MUIC] muic_distinguish_vbus_accessory(): CP_USB\n");

	/* wake lock for the factory mode */
	set_wakelock(1);

	return ret;
}


/* Initialize MUIC, i.e., the CONTROL_1,2 and SW_CONTROL registers.
 * 1) Prepare to sense INT_STAT and STATUS bits.
 * 2) Open MUIC paths. -> To keep the path from uboot setting, remove this stage.
 */
void muic_initialize(TYPE_RESET reset){

	s32 ret;

	printk(KERN_WARNING "[MUIC] muic_initialize()\n");

	if (reset) {
		if (muic_i2c_read_byte(DEVICE_ID, &muic_device) < 0) {
			printk(KERN_ERR "[MUIC] Device detection falied! Set default device.\n");
			muic_device = TS5USBA33402;
		}
		muic_device &= 0xf0;
		printk(KERN_INFO "[MUIC] Device : %s\n", (muic_device == MAX14526) ? "MAX14526" : "TSUSBA33402");
	}
	else {
		// Clear Default Switch Position (0x03=0x24)
		ret = muic_i2c_write_byte(SW_CONTROL, DP_OPEN | DM_OPEN);
	}

	if (muic_device == MAX14526) {
//		ret = muic_i2c_write_byte(CONTROL_1, MID_200 | MADC_EN);
//		ret = muic_i2c_write_byte(CONTROL_2, MINT_EN);
	}
	else {
		/* Reset MUIC - Disable all */
		if(reset){
			ret = muic_i2c_write_byte(CONTROL_1, 0x00);
			ret = muic_i2c_write_byte(CONTROL_2, MUSB_DET_DIS);// USB_DET_DIS is neg enabled
		}

		ret = muic_i2c_write_byte(CONTROL_1, MID_200 | MSEMREN);
		ret = muic_i2c_write_byte(CONTROL_2, MCHG_TYP);

		if(reset)
			msleep(250);
		else
			msleep(70);

		/* Enable interrupt (INT_EN = 1). Keep other bits the same */
		ret = muic_i2c_write_byte(CONTROL_2, MINT_EN | MCHG_TYP);
	}

	/* Default setting : CP_UART to DP3T Switch. */
	usif_switch_ctrl(USIF_DP3T);

	set_wakelock(0);
}

/* Distinguish charger type.
 * This function is called *ONLY IF* INT_STAT's CHGDET == 1, i.e., a charger is detected.
 *
 * Chargers becomes to use the default MUIC setting
 * because the detection always happens only after the MUIC_NONE mode
 * which leads the default MUIC setting.
 * Thus, we don't need to explictly set MUIC registers here.
 */
s32 muic_distinguish_charger(void){

	s32 ret = 0;

	/* Enable charger IC in TA mode */
	charging_ic_set_ta_mode();

	/* Connect CP UART signals to AP */
	//usif_switch_ctrl(USIF_DP3T);

	/* Connect AP UART to MUIC UART */
	dp3t_switch_ctrl(DP3T_AP_UART);

	/* IDNO == 0x05 180K. LG TA */
	if ((int_stat_val & MIDNO) == 0x05 || (int_stat_val & MIDNO) == 0x0b) {
		muic_mode = MUIC_LG_TA;
		printk(KERN_WARNING "[MUIC] muic_distinguish_charger(): MUIC_LG_TA\n");
		set_wakelock(0);
		return ret;
	}

	if (muic_device == MAX14526) {
		/* Prepare for reading charger type */
		ret = muic_i2c_write_byte(CONTROL_2, MINT_EN | MCHG_TYP);
		/* LGE_UPDATE_S [daewung.kim@lge.com] 2010-12-01, interrupt clear in TA detection */
		msleep(70);
		ret = muic_i2c_read_byte(INT_STAT, &status_val);
		/* LGE_UPDATE_E [daewung.kim@lge.com] 2010-12-01, interrupt clear in TA detection */
	}

	/* Read STATUS */
	ret = muic_i2c_read_byte(STATUS, &status_val);
	if (ret < 0) {
		printk(KERN_INFO "[MUIC] STATUS reading failed\n");
		muic_mode = MUIC_UNKNOWN;
		set_wakelock(0);
		return ret;
	}

	printk(KERN_INFO "[MUIC] STATUS = %x\n", status_val);

	/* DCPORT == 1. Dedicated Charger */
	if ((status_val & MDCPORT) != 0) {
		muic_mode = MUIC_NA_TA;
		printk(KERN_WARNING "[MUIC] muic_distinguish_charger(): MUIC_NA_TA\n");
	}
	/* CHPORT == 1. HCHH */
	else if ((status_val & MCHPORT) != 0) {
		muic_mode = MUIC_HCHH;
		printk(KERN_WARNING "[MUIC] muic_distinguish_charger(): MUIC_HCHH\n");
	}
	/* DCPORT == 0 && CHPORT == 0. Definitely INVALID_CHG */
	else {
		muic_mode = MUIC_INVALID_CHG;
		printk(KERN_WARNING "[MUIC] muic_distinguish_charger(): MUIC_INVALID_CHG\n");
	}

	set_wakelock(0);
	return ret;
}
/* LGE_UPDATE_E [daewung.kim@lge.com] 2010-12-01, modify detection scheme */
#if 0
/* Distinguish accessory type which supplies VBUS.
 * These accessories includes AP_UART, CP_UART, AP_USB, and CP_USB.
 * It should be called *ONLY IF* VBUS == 1.
 */
s32 muic_distinguish_vbus_accessory(s32 upon_irq){

	s32 ret = 0;

	if ((key_col == 3) && (key_row == 0)) // Volume up
		key_was_pressed = 1;
	else if ((key_col == 3) && (key_row == 1)) // Volume down
		key_was_pressed = 2;

	switch(int_stat_val & MIDNO) {

	/* CP_USB */
	case 0x02: // 56 kOhm
		if (key_was_pressed == 2)
			muic_AP_UART_set();
		else
			muic_CP_USB_set();
		break;

	/* CP_UART */
	case 0x04: // 130 kOhm
		muic_CP_UART_set();
		break;

	/* AP_USB */
	case 0x05: // 180 kOhm
        case 0x0b: // Open
		if (key_was_pressed == 2)
			muic_CP_USB_set();
		else
			muic_AP_USB_set();
		break;

	/* CP_USB */
	case 0x0a: // 910 kOhm for Factory Demand
	case 0x0d: // Unused...
		muic_CP_USB_set();
		break;

	/* Default = Unknown accessory */
	default:
		printk(KERN_WARNING "[MUIC] Failed to detect VBUS accessory type. Try again!\n");
		muic_mode = MUIC_UNKNOWN;
		ret = -1;
	}

	/* MUIC DP/DM path settle down and CP_EN delay */
	msleep(1);
	key_was_pressed = 0;

	return ret;
}

/*
 * ID_200 Set		Connect UID LDO to ID pin via 200KOhm
 * VLDO Reset		Use 2.6V for all accessory detection (except for MIC bias)
 * SEMREN Set		Enable Send-End, Mic removal comparators and UID LDO -> Sets MR_COMP, SEND/END bits
 * ADC_EN Set		Enable ADC and UID LDO to generate IDNO. TI MUIC turns on this automatically.
 * CP_EN Set		Enable charge pump for USB2.0 and headset.
 * INT_EN Set		Enable MUIC interrupt generation to OMAP
 * CHG_TYP Set		Enable charger type recognition (TA or HCHH) -> Sets DCPORT, CHPORT bits
 * USB_DET_DIS Reset	Enable charger detection (Charger or USB) -> Sets CHGDET bit
 *
 * MUIC MODE
 *
 * V C D C IDNO IDNO IDNO
 * B H C H 200K 2.2K 620
 * U G P P
 * S D O O
 *   E R R
 *   T T T
 *Cases detected by muic_distinguish_charger():
 * 1 1 1 - 0101 ---- ---- NA_TA (ID resistor 180KOhm) - Not used actually.
 * 1 1 1 - ---- ---- ---- LG_TA
 * 1 1 0 1 ---- ---- ---- HCHH (High current host/hub charger) - Not used actually.
 * 1 1 0 0 ---- ---- ---- Invalid charger
 *
 *Cases detected by muic_distinguish_vbus_accessory():
 * 1 0 0 0 0010 ---- ---- AP_UART (ID resistor 56KOhm)
 * 1 0 0 0 0100 ---- ---- CP_UART (ID resistor 130KOhm)
 * 1 0 0 0 1011 ---- ---- AP_USB (ID resistor open)
 * 1 0 0 0 ???? ---- ---- CP_USB (ID resistor ????) - Not defined yet.
 *
 *Cases detected by muic_distinguish_non_vbus_accessory():
 * 0 0 - - 0001 ---- ---- TV_OUT_NO_LOAD (ID resistor 24KOhm.) - Not used.
 * 0 0 - - 0000 01XX ---- EARMIC (ID resistor ground)
 * 0 0 - - 0000 0000 0001 TV_OUT_LOAD (ID resistor ground) - Not used.
 * 0 0 - - 0000 0000 0000 OTG (ID resistor ground) - Not used.
 */
s32 muic_device_detection(s32 upon_irq){

	s32 ret = 0;

	/* Read INT_STAT */
	if ( muic_i2c_read_byte(INT_STAT, &int_stat_val) < 0 ) {
		printk(KERN_INFO "[MUIC] INT_STAT reading failed\n");
		muic_mode = MUIC_UNKNOWN;
		return int_stat_val;
	}

	printk(KERN_INFO "[MUIC] INT_STAT = %x\n", int_stat_val);

	/* Branch according to the previous muic_mode */
	switch(muic_mode){

		/* MUIC_UNKNOWN is reached in two cases both do not have nothing to do with IRQ.
		 * First, at the initialization time where the muic_mode is not available yet.
		 * Second, whenever the current muic_mode detection is failed.
		 */
		case MUIC_UNKNOWN:
		/* If the previous muic_mode was MUIC_NONE,
		 * the only possible condition for a MUIC IRQ is plugging in an accessory.
		 */

		case MUIC_NONE:
			/* VBUS == 1.
			 * MUIC_NA_TA, MUIC_LG_TA, MUIC_HCHH, MUIC_INVALID_CHG,
			 * MUIC_AP_UART, MUIC_CP_UART, MUIC_AP_USB, or MUIC_CP_USB.
			 */
			if((int_stat_val & MVBUS) != 0){
				/* CHGDET == 1, i.e., D+/D- are short. Charger is detected.
				 * MUIC_NA_TA, MUIC_LG_TA, MUIC_HCHH, or MUIC_INVALID_CHG.
				 */
				if((int_stat_val & MCHGDET) != 0){
					/* Charger type disctinction */
					ret = muic_distinguish_charger();
				}
				/* CHGDET == 0, i.e., D+/D- are not short. No charger is detected.
				 * MUIC_AP_UART, MUIC_CP_UART, MUIC_AP_USB, MUIC_OTG, or MUIC_CP_USB.
				 */
				else{
					/* VBUS accessory type disctinction */
					ret = muic_distinguish_vbus_accessory(upon_irq);
				}
			}
			/* VBUS == 0. There is no senario */
			else{
				muic_mode = MUIC_NONE;
				printk(KERN_WARNING "[MUIC] MUIC_NONE\n");
			}
			break;

		/* If the previous muic_mode was MUIC_NA_TA, MUIC_LG_TA, MUIC_HCHH, MUIC_INVALID_CHG,
		 * MUIC_AP_UART, MUIC_CP_UART, MUIC_AP_USB, MUIC_OTG, or MUIC_CP_USB,
		 * the only possible condition for a MUIC IRQ is plugging out the accessory.
		 *
		 * In this case, initialize MUIC and wait an IRQ.
		 * We don't need to wait 250msec because this is not an erronous case
		 * (we need to reset the facility to set STATUS for an erronous case and
		 * have to wait 250msec) and, if this is not an erronous case, the facility
		 * was already initialized at the system booting.
		 */
		case MUIC_NA_TA:
		case MUIC_LG_TA:
		case MUIC_HCHH:
		case MUIC_INVALID_CHG:
		case MUIC_AP_UART:
		case MUIC_CP_UART:
		case MUIC_AP_USB:
		case MUIC_CP_USB:

			if ((int_stat_val & MVBUS) == 0 && (int_stat_val & MIDNO) == 0x0b) {
				muic_mode = MUIC_NONE;
				printk(KERN_WARNING "[MUIC] muic_device_detection(): MUIC_NONE\n");
				/* Connect CP UART signals to AP */
				//usif_switch_ctrl(USIF_AP);
				/* DP3T switch has nothing to do with. Open DP3T */
				dp3t_switch_ctrl(DP3T_NC);
				/* Open DP, DM */
				ret = muic_i2c_write_byte(SW_CONTROL, DP_OPEN | DM_OPEN);

				/* wake lock for usb connection */
				set_wakelock(0);
			}
			else if ((int_stat_val & MVBUS) != 0) {
				/* CHGDET == 1, i.e., D+/D- are short. Charger is detected.
				 * MUIC_NA_TA, MUIC_LG_TA, MUIC_HCHH, or MUIC_INVALID_CHG.
				 */
				if((int_stat_val & MCHGDET) != 0){
					/* Charger type disctinction */
					ret = muic_distinguish_charger();
				}
				else{
					/* VBUS accessory type disctinction */
					ret = muic_distinguish_vbus_accessory(upon_irq);
				}
			}
			else{
				muic_mode = MUIC_UNKNOWN;
				ret = -1;
				printk(KERN_WARNING "[MUIC] Failed to distinguish muic_mode. Try again!\n");
			}
			break;

		default:
			printk(KERN_WARNING "[MUIC] Failed to detect an accessory. Try again!");
			muic_mode = MUIC_UNKNOWN;
			ret = -1;
			break;
	}

	if (muic_device == TS5USBA33402)
		ret = muic_i2c_write_byte(CONTROL_2, MINT_EN | MCHG_TYP); /* INT_EN, CHG_TYP */

	if (muic_mode == MUIC_UNKNOWN)
		muic_initialize(DEFAULT);

	if (muic_mode == MUIC_NONE) {	//20101002 ks.kwon@lge.com charging_ic_deactive only for MUIC_NONE
		charging_ic_deactive();	//20100506 ks.kwon@lge.com for charging animation, by demand from taehwan.kim
		printk(KERN_INFO "[MUIC]charging_ic_deactive()\n");
	}

	charger_state_update_by_other();
	return ret;
}
#else
static void muic_device_none_detect(void)
{
	u8 reg_value;

// LGE_UPDATE_S 20110228 [jaejoong.kim@lge.com] modify key_col value from 3 to 4
	if ((key_col == 4) && (key_row == 0)) // Volume up
		key_was_pressed = 1;
	else if ((key_col == 4) && (key_row == 1)) // Volume down
		key_was_pressed = 2;
// LGE_UPDATE_E 20110228 [jaejoong.kim@lge.com] modify key_col value from 3 to 4

	printk(KERN_WARNING "[MUIC] Device_None_Detect int_stat_val = 0x%x\n",int_stat_val);

/* LGE_CHANGE_S [kenneth.kang@lge.com] 2010-12-14, CP retain mode */
#if 0 // CP_RETAIN	//block CP_RETAIN for commercial version
	if (is_cp_retained)
	{
		muic_CP_USB_set();
	}
	// IDNO=0100? 130Kohm :: CP UART MODE
	else if(((int_stat_val & MIDNO) == 0x04) || (hidden_menu_switching == 7)) {
/* LGE_CHANGE_E [kenneth.kang@lge.com] 2011-01-06, CP retain mode */
#else		
	if(((int_stat_val & MIDNO) == 0x04) || (hidden_menu_switching == 7)) {
#endif	
		muic_CP_UART_set();
	}
	// IDNO=0010? 56Kohm  :: CP USB MODE
	else if (((int_stat_val & MIDNO ) == 0x02) || (hidden_menu_switching == 9)){
		if (key_was_pressed == 2)
			muic_AP_UART_set();
		else
			muic_CP_USB_set();
	}
	// LGE_UPDATE_S [kenneth.kang@lge.com] 2010-12-12, for 910K factory download
	// IDNO=1010? 910Kohm :: CP USB MODE
#ifdef 	CABLE_DETECT_910K	
	else if ((int_stat_val & MIDNO ) == 0x0a) {
		muic_CP_USB_set();
	}
	else if ((int_stat_val & MIDNO ) == 0x09) {
		muic_CP_USB_set();
	}
#endif	
	// CHGDET=1?  :: HIGH CURRENT USB or TA?
	else if (int_stat_val & MCHGDET) {
		muic_distinguish_charger();
	}
	// VBUS=1?  :: TA or AP USB?
	else if (int_stat_val & MVBUS) {
		if (muic_device == MAX14526) {
			// COMP2 to H-Z / COMN1 to C1COMP (0x03=0x23)
			muic_i2c_write_byte(SW_CONTROL, 0x23);

			msleep(3);

			// Read STATUS_REG (0x05)
			muic_i2c_read_byte(STATUS, &reg_value);

			if (reg_value & 0x01 ) {
				// Dedicated Charger(TA) Detected
				// COMP2 to H-Z / COMN1 to H-Z (0x03=0x24)
				muic_i2c_write_byte(SW_CONTROL, DP_OPEN | DM_OPEN);

				charging_ic_set_ta_mode();

				muic_mode = MUIC_NA_TA;
				printk(KERN_WARNING "[MUIC] Charger detected\n");
			}
			else {	// USB Detected
// LGE_UPDATE_S 20110521 [jaejoong.kim@lge.com] block CP usb for commercial version
#if 0
				if (key_was_pressed == 2)
					muic_CP_USB_set();
				else
#endif /* #if 0 */
// LGE_UPDATE_S 20110521 [jaejoong.kim@lge.com] block CP usb for commercial version
					muic_AP_USB_set();
			}
		}
		else { 	// USB Detected
// LGE_UPDATE_S 20110521 [jaejoong.kim@lge.com] block CP usb for commercial version
#if 0
			if (key_was_pressed == 2)
				muic_CP_USB_set();
			else
#endif /* #if 0 */
// LGE_UPDATE_S 20110521 [jaejoong.kim@lge.com] block CP usb for commercial version
				muic_AP_USB_set();
		}
	}
	else {
		// Accessory Not Supported
		muic_mode = MUIC_NONE;
	}

	key_was_pressed = 0;
}

s32 muic_device_detection(s32 upon_irq)
{
/* LGE_CHANGE_START 2011-03-16 kenneth.kang@lge.com patch for Adb offline set and Mass Storage Driver detecting fail */    
	/* 20110113 ks.kwon@lge.com check muic driver init. state [START] */
	if(!muic_init_done){

		printk(KERN_WARNING "[MUIC] MUIC has not been initialized! Nothing will be done!!!.\n");
		return 0;
	}	
    /* 20110113 ks.kwon@lge.com check muic driver init. state [END] */
/* LGE_CHANGE_END 2011-03-16 kenneth.kang@lge.com */

//+DEJA_S : for debugging must remove
//	muic_AP_UART_set();
//	return 0;
//+DEJA_E

	// Read INT_STAT_REG (0x04)
	muic_i2c_read_byte(INT_STAT, &int_stat_val);
	printk(KERN_INFO "[MUIC] INT_STAT = %x\n", int_stat_val);

	switch (muic_mode)
	{
	case MUIC_NONE:
	case MUIC_UNKNOWN:
		muic_device_none_detect();
		break;

	// CP UART Mode
	case MUIC_CP_UART:
	case MUIC_AP_UART:
		if ((int_stat_val & MIDNO) == 0x0b) {
			muic_mode = MUIC_NONE;
		}
		else {
			muic_device_none_detect();
		}
		break;

	// TA Mode
	case MUIC_NA_TA:
	case MUIC_LG_TA:
	case MUIC_HCHH:
	case MUIC_INVALID_CHG:
		if ((int_stat_val & MVBUS) == 0) {
			// Exit Charger Mode
			muic_mode = MUIC_NONE;
		}
		else {
			// Bug fix: charger detect interrupt 2 times
			set_wakelock(0);
		}
		break;

	// USB Mode
	case MUIC_AP_USB:
	case MUIC_CP_USB:
		if ((int_stat_val & MVBUS) == 0){
			// Exit USB Mode
			muic_mode = MUIC_NONE;
		}
		else {
			muic_device_none_detect();
		}
		break;

	default:
		muic_mode = MUIC_NONE;
		break;
	}

	if (muic_mode == MUIC_NONE) {
		muic_initialize(DEFAULT);
		printk(KERN_INFO "[MUIC] muic none\n");
		charging_ic_deactive();
		printk(KERN_INFO "[MUIC] charging_ic_deactive()\n");
	}

	charger_state_update_by_other();

	return 0;
}
EXPORT_SYMBOL(muic_device_detection);
#endif
/* LGE_UPDATE_E [daewung.kim@lge.com] 2010-12-01, modify detection scheme */

static void muic_wq_func(struct work_struct *muic_wq){
	muic_device_detection(UPON_IRQ);
}

static irqreturn_t muic_interrupt_handler(s32 irq, void *data){

	printk(KERN_WARNING "[MUIC] muic_interrupt_handler(): muic_mode = %s\n",
		name_muic_mode[(s32)muic_mode]);

	set_wakelock(1);

	if (muic_device == TS5USBA33402)
		schedule_delayed_work(&muic_wq, msecs_to_jiffies(70));
	else
		schedule_delayed_work(&muic_wq, msecs_to_jiffies(250));//HZ / 10);

	return IRQ_HANDLED;
}

/* B-Prj Power off charging [kyungyoon.kim@lge.com] 2010-12-15 */
ssize_t muic_store_wake_lock(struct device *dev,
			  struct device_attribute *attr,
			  const char *buf,
			  size_t count)
{
	int value = 0;

    	printk(KERN_INFO "[MUIC] muic_store_wake_lock \n");

	if (!count) {
		return -EINVAL;
	}

	value = simple_strtoul(buf, NULL, 10);
    	if (value >2 )
		return -EINVAL;

	printk(KERN_INFO "[MUIC]wake_lock=%d\n",value);
	set_wakelock(value);
	return count;
}

/* Shutdown issue at the case of USB booting [kyungyoon.kim@lge.com] 2010-12-25 */
ssize_t muic_store_charging_mode(struct device *dev, 
			  struct device_attribute *attr, 
			  const char *buf, 
			  size_t count)
{
	int value = 0;

    	printk(KERN_INFO "[MUIC] muic_store_charging_mode \n");
	
	if (!count) {
		return -EINVAL;
	}

	value = simple_strtoul(buf, NULL, 10);

    	if (value >2 )
		return -EINVAL;

	printk(KERN_INFO "[MUIC]charging_mode=%d\n",value);

	if (value==1)
		charging_ic_set_usb_mode_from_ta_mode();

	return count;
}
/* LGE_CHANGE_START 2011-03-16 kenneth.kang@lge.com patch for Adb offline set and Mass Storage Driver detecting fail */    
void vbus_irq_muic_handler(int state)
{
    if(muic_init_done){
        printk("[MUIC] %s vbus_state = %d \n",__func__,state);
        if (state == 2)
        {
            //if (muic_mode == MUIC_NONE) schedule_work(&muic_wq);
            if (muic_mode == MUIC_NONE) schedule_work(&muic_wq.work); // 20120213 taeju.park@lge.com To delete compile warning, incompatible types
        }
        else
        {
            //if (muic_mode != MUIC_NONE) schedule_work(&muic_wq);        
            if (muic_mode != MUIC_NONE) schedule_work(&muic_wq.work); // 20120213 taeju.park@lge.com To delete compile warning, incompatible types
        }
    }
    printk("[MUIC] %s completed \n",__func__);
}
EXPORT_SYMBOL(vbus_irq_muic_handler);
/* LGE_CHANGE_END 2011-03-16 kenneth.kang@lge.com */

DEVICE_ATTR(charging_mode, 0664, NULL, muic_store_charging_mode);
/* Shutdown issue at the case of USB booting [kyungyoon.kim@lge.com] 2010-12-25 */

DEVICE_ATTR(wake_lock, 0664, NULL, muic_store_wake_lock);
/* B-Prj Power off charging [kyungyoon.kim@lge.com] 2010-12-15 */
/*
 * muic_probe() is called in the middle of booting sequence due to '__init'.
 * '__init' causes muic_probe() to be released after the booting.
 */

// kibum.lee@lge.com section mismatch error fix
//static s32 __init muic_probe(struct i2c_client *client, const struct i2c_device_id *id){
static s32 muic_probe(struct i2c_client *client, const struct i2c_device_id *id){

	s32 ret = 0;
	u32 retry_no = 0;
/* B-Prj Power off charging [kyungyoon.kim@lge.com] 2010-12-15 */
	int err = 0;
/* B-Prj Power off charging [kyungyoon.kim@lge.com] 2010-12-15 */

	muic_client = client;

	/* wake lock for usb connection */
	wake_lock_init(&the_wlock.wake_lock, WAKE_LOCK_SUSPEND, "usb_connection");
	the_wlock.wake_lock_on=0;

	/* GPIO initialization */
	omap_mux_init_gpio(MUIC_INT_GPIO, OMAP_PIN_INPUT_PULLUP | OMAP_PIN_OFF_WAKEUPENABLE);
	omap_mux_init_gpio(DP3T_IN_1_GPIO, OMAP_PIN_OUTPUT);
	omap_mux_init_gpio(DP3T_IN_2_GPIO, OMAP_PIN_OUTPUT);
	omap_mux_init_gpio(USIF_IN_1_GPIO, OMAP_PIN_OUTPUT);
	omap_mux_init_gpio(IFX_VBUS_EN, OMAP_PIN_OUTPUT);

	/* Initialize GPIO direction before use.
	 * Check if other driver already occupied it.
	 */
	if (gpio_request(USIF_IN_1_GPIO, "USIF switch control GPIO") < 0) {
		printk(KERN_INFO "[MUIC] GPIO %d USIF1_SW is already occupied by other driver!\n", USIF_IN_1_GPIO);
		/* We know board_hub.c:ifx_n721_configure_gpio() performs a gpio_request on this pin first.
		 * Because the prior gpio_request is also for the analog switch control, this is not a confliction.*/
		return -ENOSYS;
	}

	if (gpio_request(DP3T_IN_1_GPIO, "DP3T switch control 1 GPIO") < 0) {
		printk(KERN_INFO "[MUIC] GPIO %d DP3T_IN_1_GPIO is already occupied by other driver!\n", DP3T_IN_1_GPIO);
		return -ENOSYS;
	}

	if (gpio_request(DP3T_IN_2_GPIO, "DP3T switch control 2 GPIO") < 0) {
		printk(KERN_INFO "[MUIC] GPIO %d DP3T_IN_2_GPIO is already occupied by other driver!\n", DP3T_IN_2_GPIO);
		return -ENOSYS;
	}

	if (gpio_request(IFX_VBUS_EN, "IFX VBUS EN") < 0) {
		printk(KERN_INFO "[MUIC] GPIO %d IFX_VBUS_EN is already occupied by other driver!\n", IFX_VBUS_EN);
		return -ENOSYS;
	}

	/* Initialize GPIO 40 (MUIC_INT_N).
	 * Check if other driver already occupied it.
	 */
	if (gpio_request(MUIC_INT_GPIO, "MUIC IRQ GPIO") < 0) {
		printk(KERN_INFO "[MUIC] GPIO %d MUIC_INT_N is already occupied by other driver!\n", MUIC_INT_GPIO);
		return -ENOSYS;
	}

	/* Initialize GPIO direction before use or IRQ setting */
	if (gpio_direction_input(MUIC_INT_GPIO) < 0) {
		printk(KERN_INFO "[MUIC] GPIO %d MUIC_INT_N direction initialization failed!\n", MUIC_INT_GPIO);
		return -ENOSYS;
	}
//[[UBIQUIX_CHANGE_START, 0120419, hrgo@ubiquix.com, For AP/CP switching
	gpio_direction_output(DP3T_IN_1_GPIO, 0);
	gpio_direction_output(DP3T_IN_2_GPIO, 0);
	gpio_direction_output(USIF_IN_1_GPIO, 0);
	gpio_direction_output(IFX_VBUS_EN, 0);
//UBIQUIX_CHANGE_START, 0120419, hrgo@ubiquix.com, For AP/CP switching]]
	/* Register MUIC work queue function */
	INIT_DELAYED_WORK(&muic_wq, muic_wq_func);

	/* Set up an IRQ line and enable the involved interrupt handler.
	 * From this point, a MUIC_INT_N can invoke muic_interrupt_handler().
	 * muic_interrupt_handler merely calls schedule_work() with muic_wq_func().
	 * muic_wq_func() actually performs the accessory detection.
	 */
	if (request_irq(gpio_to_irq(MUIC_INT_GPIO),
			  muic_interrupt_handler,
			  IRQF_TRIGGER_FALLING,
			  "muic_irq",
			  &client->dev) < 0) {
		printk(KERN_INFO "[MUIC] GPIO %d MUIC_INT_N IRQ line set up failed!\n", MUIC_INT_GPIO);
		free_irq(gpio_to_irq(MUIC_INT_GPIO), &client->dev);
		return -ENOSYS;
	}

	/* Make the interrupt on MUIC INT wake up OMAP which is in suspend mode */
	if (enable_irq_wake(gpio_to_irq(MUIC_INT_GPIO)) < 0) {
		printk(KERN_INFO "[MUIC] GPIO %d MUIC_INT_N wake up source setting failed!\n", MUIC_INT_GPIO);
		disable_irq_wake(gpio_to_irq(MUIC_INT_GPIO));
		return -ENOSYS;
	}

	/* Prepare a human accessible method to control MUIC */
	create_hub_muic_proc_file();
/* B-Prj Power off charging [kyungyoon.kim@lge.com] 2010-12-15 */
	err = device_create_file(&client->dev, &dev_attr_wake_lock);
	err = device_create_file(&client->dev, &dev_attr_charging_mode);
/* B-Prj Power off charging [kyungyoon.kim@lge.com] 2010-12-15 */
	/* Initialize MUIC - Finally MUIC INT becomes enabled */
/* LGE_CHANGE_START 2011-03-16 kenneth.kang@lge.com patch for Adb offline set and Mass Storage Driver detecting fail */    
	 muic_init_done = 1; //20110113 ks.kwon@lge.com check muic driver init. state
/* LGE_CHANGE_END 2011-03-16 kenneth.kang@lge.com */
	muic_initialize(RESET);

	mdelay(70);
	ret = muic_device_detection(NOT_UPON_IRQ);

	/* If an erronous situation occurs, try again */
	while(ret < 0 && retry_no < 3){
		printk(KERN_INFO "[MUIC] muic_probe(): muic_device_detection() failed %d times\n", ++retry_no);
		ret = muic_device_detection(NOT_UPON_IRQ);
	}

	printk(KERN_INFO "[MUIC] muic_probe(): done!\n");

	return ret;
}

static s32 muic_remove(struct i2c_client *client){
	free_irq(gpio_to_irq(MUIC_INT_GPIO), &client->dev);
	gpio_free(MUIC_INT_GPIO);
	gpio_free(USIF_IN_1_GPIO);
	gpio_free(DP3T_IN_1_GPIO);
	gpio_free(DP3T_IN_2_GPIO);
	gpio_free(IFX_VBUS_EN);

	if (muic_device == MAX14526) {
		// CTRL2_REG(0x02=0x01)
		muic_i2c_write_byte(CONTROL_2, MUSB_DET_DIS);
		// CTRL1_REG(0x01=0x00)
		muic_i2c_write_byte(CONTROL_1, 0x00);
		// SW_CTRL_REG(0x03=0x24)
		muic_i2c_write_byte(SW_CONTROL, DP_OPEN | DM_OPEN);
	}
/* B-Prj Power off charging [kyungyoon.kim@lge.com] 2010-12-15 */
	device_remove_file(&client->dev, &dev_attr_wake_lock);
	device_remove_file(&client->dev, &dev_attr_charging_mode);
/* B-Prj Power off charging [kyungyoon.kim@lge.com] 2010-12-15 */
	i2c_set_clientdata(client, NULL);	// For what?
	remove_hub_muic_proc_file();

	/* wake lock for usb connection */
	wake_lock_destroy(&the_wlock.wake_lock);

	return 0;
}

static s32 muic_suspend(struct i2c_client *client, pm_message_t state){
	client->dev.power.power_state = state;
	cancel_delayed_work(&muic_wq);
	printk(KERN_INFO "[MUIC] muic_suspend \n");
	return 0;
}

static s32 muic_resume(struct i2c_client *client){
	client->dev.power.power_state = PMSG_ON;
	printk(KERN_INFO "[MUIC] muic_resume \n");
	return 0;
}

static const struct i2c_device_id muic_ids[] = {
	{"hub_i2c_muic", 0},
	{/* end of list */},
};

/* Allow user space tools to figure out which devices this driver can control.
 * The first parameter should be 'i2c' for i2c client chip drivers.
 */
//MODULE_MUIC_TABLE(i2c, muic_ids);

static struct i2c_driver muic_driver = {
	.probe      = muic_probe,
	.remove	    = muic_remove,
	.suspend    = muic_suspend,
	.resume     = muic_resume,
	.id_table   = muic_ids,
	.driver     = {
	.name       = "hub_i2c_muic",
	.owner      = THIS_MODULE,
	},
};

static s32 __init muic_init(void){
	printk(KERN_WARNING "[MUIC] muic_init()\n");
	return i2c_add_driver(&muic_driver);
}

static void __exit muic_exit(void){
	i2c_del_driver(&muic_driver);
}

/* LGE_CHANGE_S [kenneth.kang@lge.com] 2010-12-14, CP retain mode */
#ifdef CP_RETAIN
static s32 __init muic_state(char *str)
{
	s32 muic_value = simple_strtol(str, NULL, 0);
	is_cp_retained = muic_value;
	printk(KERN_INFO "[MUIC] CP Retain : %d\n", muic_value);
	return 1;
}
__setup("muic_state=", muic_state);
#endif
/* LGE_CHANGE_E [kenneth.kang@lge.com] 2010-12-14, CP retain mode */
module_init(muic_init);
module_exit(muic_exit);

MODULE_AUTHOR("LG Electronics");
MODULE_DESCRIPTION("Hub MUIC Driver");
MODULE_LICENSE("GPL");
