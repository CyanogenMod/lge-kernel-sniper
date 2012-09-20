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

#include "usb_switch.h"
#include "hub_charging_ic.h"
#include "../mux.h"

/* LGE_CHANGE_S [LS855:FW:bking.moon@lge.com] 2011-02-15, */ 
#if 1 /* mbk_temp */ 
#include <linux/i2c/twl4030-madc.h>
#endif 
/* LGE_CHANGE_E [LS855:FW:bking.moon@lge.com] 2011-02-15 */

/* LGE_CHANGE_S [kenneth.kang@lge.com] 2010-12-14, CP retain mode and 910K cable detect*/
#define CABLE_DETECT_910K
//#define CP_RETAIN
/* LGE_CHANGE_E [kenneth.kang@lge.com] 2010-12-14, CP retain mode and 910K cable detect*/

/* LGE_CHANGE_S [kenneth.kang@lge.com] 2011-01-06, CP retain mode and Hidden Menu AP CP Switching Retain */
#ifdef CP_RETAIN
static int is_cp_retained;
#endif
static int hidden_menu_switching;
/* LGE_CHANGE_E [kenneth.kang@lge.com] 2011-01-06, CP retain mode */

/* Wake lock for usb connection */
struct wlock {
	int wake_lock_on;
	struct wake_lock wake_lock;
};
static struct wlock the_wlock;

void dp3t_switch_ctrl(TYPE_DP3T_MODE mode);
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
	printk(KERN_INFO "%s: Read is not available ", __func__ );
	return 0;
}

#define RESET_MDM 26
static ssize_t muic_proc_write(struct file *filp, const char *buf, size_t len, loff_t *off){
	char messages[12];
	u32 val;
	u32 reg;
	char cmd;

	if(len > 12)
		len = 12;

	if(copy_from_user(messages, buf, len))
		return -EFAULT;

	sscanf(buf, "%c 0x%x 0x%x", &cmd, &reg, &val);

	printk(KERN_INFO "[MUIC] LGE: MUIC_proc_write \n");


	switch(cmd){
		 case '3':
            muic_CP_USB_set();
            gpio_set_value(RESET_MDM, 0);
			mdelay(100);
            gpio_set_value(RESET_MDM, 1);
            break;
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
/* LGE_CHANGE_E [kenneth.kang@lge.com] 2011-01-06, CP retain mode */
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

#if 0
TYPE_MUIC_MODE get_muic_mode(void){
	int muic_mode_temp;
	switch( get_ext_pwr_type()) {
		case NO_CABLE:
			muic_mode_temp = MUIC_NONE;
			break;
		case LT_CABLE:
		case TA_CABLE:
		case FORGED_TA_CABLE:
			muic_mode_temp = MUIC_LG_TA;
			break;
		default :
			muic_mode_temp = MUIC_UNKNOWN;
			break;
	}
	return muic_mode_temp;
}
EXPORT_SYMBOL(get_muic_mode);
#endif

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

void dp3t_switch_ctrl(TYPE_DP3T_MODE mode){
	
	if(mode == DP3T_AP_UART){
		gpio_set_value(MDM_USB_VBUS_EN, 0);
		gpio_set_value(USB_MDM_SW, 1);
	}
	else if(mode == DP3T_CP_UART){
		gpio_set_value(MDM_USB_VBUS_EN, 0);
		gpio_set_value(USB_MDM_SW, 0);
	}
	else if(mode == DP3T_CP_USB){
		gpio_set_value(MDM_USB_VBUS_EN, 1);
		gpio_set_value(USB_MDM_SW, 1);
	}
	else if(mode == DP3T_NC){
		gpio_set_value(MDM_USB_VBUS_EN, 0);
		gpio_set_value(USB_MDM_SW, 0);
	}
	else{
		/* Just keep the current path */
	}
}

s32 muic_AP_UART_set(void){

	s32 ret = 0;

	/* Connect AP UART to MUIC UART */
	dp3t_switch_ctrl(DP3T_AP_UART);

	set_wakelock(0);

	return ret;
}

s32 muic_AP_USB_set(void){

	s32 ret = 0;

	/* AP USB does not pass through DP3T.
	 * Just connect AP UART to MUIC UART. */
	dp3t_switch_ctrl(DP3T_AP_UART);

	/* wake lock for usb connection */
	set_wakelock(1);

	return ret;
}

s32 muic_CP_UART_set(void){

	s32 ret = 0;

	/* Connect CP UART to MUIC UART */
	dp3t_switch_ctrl(DP3T_CP_UART);

	/* wake lock for the factory mode */
	set_wakelock(0);

	return ret;
}

s32 muic_CP_USB_set(void){

	s32 ret = 0;

	/* Connect CP USB to MUIC UART */
	dp3t_switch_ctrl(DP3T_CP_USB);

	/* wake lock for the factory mode */
	set_wakelock(1);

	return ret;
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
	int err = 0;

	/* wake lock for usb connection */
	wake_lock_init(&the_wlock.wake_lock, WAKE_LOCK_SUSPEND, "usb_connection");
	the_wlock.wake_lock_on=0;

	/* GPIO initialization */
	omap_mux_init_gpio(USB_MDM_SW, OMAP_PIN_OUTPUT);
	omap_mux_init_gpio(MDM_USB_VBUS_EN, OMAP_PIN_OUTPUT);

	if (gpio_request(USB_MDM_SW, "DP3T switch control 1 GPIO") < 0) {
		printk(KERN_INFO "[MUIC] GPIO %d USB_MDM_SW is already occupied by other driver!\n", USB_MDM_SW);
		return -ENOSYS;
	}

	if (gpio_request(MDM_USB_VBUS_EN, "IFX VBUS EN") < 0) {
		printk(KERN_INFO "[MUIC] GPIO %d MDM_USB_VBUS_EN is already occupied by other driver!\n", MDM_USB_VBUS_EN);
		return -ENOSYS;
	}

	/* Prepare a human accessible method to control MUIC */
	create_hub_muic_proc_file();
/* B-Prj Power off charging [kyungyoon.kim@lge.com] 2010-12-15 */
	err = device_create_file(&client->dev, &dev_attr_wake_lock);
	err = device_create_file(&client->dev, &dev_attr_charging_mode);
/* B-Prj Power off charging [kyungyoon.kim@lge.com] 2010-12-15 */

/* LGE_CHANGE_S [LS855:FW:bking.moon@lge.com] 2011-02-16, */ 
#if 0 /* mbk_temp */ 
	dp3t_switch_ctrl(DP3T_CP_USB);
#endif 
/* LGE_CHANGE_E [LS855:FW:bking.moon@lge.com] 2011-02-16 */

	printk(KERN_INFO "[MUIC] muic_probe(): done!\n");

	return ret;
}

static s32 muic_remove(struct i2c_client *client){
	gpio_free(USB_MDM_SW);
	gpio_free(MDM_USB_VBUS_EN);

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
//For_Resume_Speed	printk(KERN_INFO "[MUIC] muic_suspend \n");
	return 0;
}

static s32 muic_resume(struct i2c_client *client){
	client->dev.power.power_state = PMSG_ON;
//For_Resume_Speed	printk(KERN_INFO "[MUIC] muic_resume \n");
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
