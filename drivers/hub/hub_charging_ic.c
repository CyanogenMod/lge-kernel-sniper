/*
 * Hub Charging IC driver (MAX8922)
 *
 * Copyright (C) 2009 LGE, Inc.
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

#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <mach/gpio.h>
#include <linux/delay.h>
#include <linux/hrtimer.h>
#include <linux/wakelock.h>
#include "../staging/android//timed_output.h"
#include "hub_charging_ic.h"
/* LGE_CHANGE_S [daewung.kim@lge.com] 2010-12-27, Reboot rather than pwr off when TA inserted */
#include <linux/notifier.h>
#if defined(CONFIG_HUB_MUIC)
#include "hub_muic.h"
#elif defined(CONFIG_MUIC)
#include <linux/muic/muic.h>
#elif defined(CONFIG_LGE_OMAP3_EXT_PWR)
#include "usb_switch.h"
#endif
/* LGE_CHANGE_E [daewung.kim@lge.com] 2010-12-27, Reboot rather than pwr off when TA inserted */

#include <linux/i2c/twl4030-madc.h>
extern int check_battery_present(void);


#define CHR_IC_DELAY				170	/* 500 us */
#define CHR_IC_SET_DEALY			1500	/* 1500 us */

static struct delayed_work	charging_ic_int_work;
static max8922_status charging_ic_status = CHARGING_IC_ACTIVE_DEFAULT;
static struct wake_lock power_off_charging_lock;

extern int set_end_of_charge(int complete);	// from twl4030_bci_battery.c
extern void charger_state_update_by_other(void);	// from twl4030_bci_battery.c


/* Function Prototype */
static void hub_charging_ic_intialize(void);
static irqreturn_t charging_ic_interrupt_handler(int irq, void *data);

#ifdef CONFIG_LGE_CHARGE_CONTROL_BATTERY_FET
void lge_battery_fet_onoff(int on)
{
	if( get_ext_pwr_type() == LT_CABLE )
		on = 0; 

	if(on) {
		printk(KERN_INFO "%s, On %d - GPIO LOW \n", __FUNCTION__, on);
		/* Charging */
		gpio_set_value(CHAR_CONTROL, 0);
	} else {
		printk(KERN_INFO "%s, On %d - GPIO HIGH \n", __FUNCTION__, on);
		/* No Charging */
		gpio_set_value(CHAR_CONTROL, 1);
	}
	return ;
}
EXPORT_SYMBOL(lge_battery_fet_onoff);
#endif 

max8922_status get_charging_ic_status(void)
{
	return charging_ic_status;
}
EXPORT_SYMBOL(get_charging_ic_status);

void charging_ic_deactive_before_setting(void)
{

	printk(KERN_INFO "[charging_msg]%s\n", __FUNCTION__);
	/* If No Battery and Charger IC Deactive, the Device will be power down */
	if( check_battery_present() == 0 ) {
		printk("[charging_msg] %s, Fail... cause No Battery \n",__FUNCTION__);
		return ;
	}

	gpio_set_value(CHG_EN_SET_N_OMAP, 1);
	mdelay(1);
	udelay(500);
	return ;
}

void charging_ic_active_default(void)
{
	if(charging_ic_status == CHARGING_IC_ACTIVE_DEFAULT)
		return;

	switch(charging_ic_status) {
		case CHARGING_IC_TA_MODE :
		case CHARGING_IC_FACTORY_MODE :
			charging_ic_deactive_before_setting();
			break;
		default:
			break;
	}

	gpio_set_value(CHG_EN_SET_N_OMAP, 0);

	mdelay(1);
	udelay(500);

	charging_ic_status = CHARGING_IC_ACTIVE_DEFAULT;
}
EXPORT_SYMBOL(charging_ic_active_default);

void charging_ic_set_ta_mode(void)
{
	if(charging_ic_status == CHARGING_IC_TA_MODE)
		return;

	if(charging_ic_status == CHARGING_IC_FACTORY_MODE)
		charging_ic_deactive_before_setting();

	gpio_set_value(CHG_EN_SET_N_OMAP, 0);
	udelay(CHR_IC_DELAY);
	gpio_set_value(CHG_EN_SET_N_OMAP, 1);
	udelay(CHR_IC_DELAY);
	gpio_set_value(CHG_EN_SET_N_OMAP, 0);

	mdelay(1);
	udelay(500);

	charging_ic_status = CHARGING_IC_TA_MODE;
}
EXPORT_SYMBOL(charging_ic_set_ta_mode);

void charging_ic_set_usb_mode()
{
	//charging_ic_set_ta_mode();
	charging_ic_active_default();
}
EXPORT_SYMBOL(charging_ic_set_usb_mode);

#if 1 /* Shutdown issue at the case of USB booting [kyungyoon.kim@lge.com] 2010-12-25 */
void charging_ic_set_usb_mode_from_ta_mode(void)
{
	gpio_set_value(CHG_EN_SET_N_OMAP, 1);
	mdelay(1);
	udelay(500);
	gpio_set_value(CHG_EN_SET_N_OMAP, 0);
	mdelay(1);
	udelay(500);

	charging_ic_status = CHARGING_IC_ACTIVE_DEFAULT;
}
EXPORT_SYMBOL(charging_ic_set_usb_mode_from_ta_mode);
#endif /* Shutdown issue at the case of USB booting [kyungyoon.kim@lge.com] 2010-12-25 */

void charging_ic_set_factory_mode()
{
	if(charging_ic_status == CHARGING_IC_FACTORY_MODE)
		return;

	if(charging_ic_status == CHARGING_IC_TA_MODE)
		charging_ic_deactive_before_setting();

	gpio_set_value(CHG_EN_SET_N_OMAP, 0);
	udelay(CHR_IC_DELAY);
	gpio_set_value(CHG_EN_SET_N_OMAP, 1);
	udelay(CHR_IC_DELAY);
	gpio_set_value(CHG_EN_SET_N_OMAP, 0);
	udelay(CHR_IC_DELAY);
	gpio_set_value(CHG_EN_SET_N_OMAP, 1);
	udelay(CHR_IC_DELAY);
	gpio_set_value(CHG_EN_SET_N_OMAP, 0);
	udelay(CHR_IC_DELAY);
	gpio_set_value(CHG_EN_SET_N_OMAP, 1);
	udelay(CHR_IC_DELAY);
	gpio_set_value(CHG_EN_SET_N_OMAP, 0);

	mdelay(1);
	udelay(500);

	charging_ic_status = CHARGING_IC_FACTORY_MODE;
}
EXPORT_SYMBOL(charging_ic_set_factory_mode);

void charging_ic_deactive(void)
{
	charging_ic_status = CHARGING_IC_DEACTIVE;
	charging_ic_deactive_before_setting();
	return ;
}
EXPORT_SYMBOL(charging_ic_deactive);

static void charging_ic_work_func(struct work_struct *work)
{
	printk(KERN_INFO "[charging_msg] %s\n", __FUNCTION__);
#if defined(CONFIG_PRODUCT_LGE_LU6800)
	//20101102 taehwan.kim@lge.com To support factory cal to prevent to deactive[START_LGE]
	charger_state_update_by_other();
	//20101102 taehwan.kim@lge.com To support factory cal to prevent to deactive[END_LGE]
#else
	if(charging_ic_status != CHARGING_IC_DEACTIVE) {
		set_end_of_charge(1);
		//charging_ic_deactive();
		charger_state_update_by_other();
		printk(KERN_INFO "[Battery] Charging IC - EOC\n");
	}
#endif
}

static void hub_charging_ic_intialize(void)
{
}

static irqreturn_t charging_ic_interrupt_handler(int irq, void *data)
{
	printk(KERN_INFO "[charging_msg] %s: charger ic interrupt occured \n", __func__);
    if (charging_ic_status != CHARGING_IC_DEACTIVE)
    	schedule_delayed_work(&charging_ic_int_work, msecs_to_jiffies(500));

	return IRQ_HANDLED;
}

#ifdef CONFIG_LGE_OMAP3_EXT_PWR
void setting_for_factory_mode_hw_req(void)
{
#if 0 /* XXX_mbk */
	/* Battery FET OFF */
	lge_battery_fet_onoff(0);
#else
	extern void dp3t_switch_ctrl(TYPE_DP3T_MODE mode);
	
	/* HW요청사항...Battery가 있으면...2A가 아닌 960mA로 충전하도록
	   Charger IC가 2A로 충전을 계속하게 되면 버티지 못함. */
	if( check_battery_present() ) {
		printk(KERN_INFO "%s, Charger Current Setting TA mode from Factory Mode\n", __FUNCTION__);
		charging_ic_set_ta_mode();
	}
#endif

	/* USB Switch to CP */
	dp3t_switch_ctrl(DP3T_CP_USB);
}

extern int lge_usb_acc_detect(void);
extern int get_external_power_status(void);
extern int get_madc_probe_done(void);
extern void charger_state_update_by_other(void);

void set_charging_current(void)
{
	static int old_status = -1;
	int current_status = -1;

	if( !get_madc_probe_done() )
		return ;

	current_status = get_external_power_status();
	if( old_status != current_status ) {

		/* mbk_temp */
		lge_usb_acc_detect();

		switch( get_ext_pwr_type() ) {
			case NO_CABLE:
				break;
			case LT_CABLE:
				// if LT_Cable, u-boot already set charing current. in kernel no need to set 
				//charging_ic_set_factory_mode();

				/* mbk_temp HW Request */
				setting_for_factory_mode_hw_req();
				break;
			case TA_CABLE:
			case FORGED_TA_CABLE:
				charging_ic_set_ta_mode();
				break;
		}
		printk("[charging_msg] %s, External Power Type %d \n", __FUNCTION__, get_ext_pwr_type());

		old_status = current_status;
	}

	charger_state_update_by_other();
	return ;
}

static int external_power_on = 0;
void set_external_power_detect(int on)
{
	printk("[charging_msg] %s, ext_pwr status %d \n", __FUNCTION__, on);

#if 0
	if( 0 == on ) {
		lge_battery_fet_onoff(1);
	}
#endif

	external_power_on = !!on;
	return ;
}
int get_external_power_status(void)
{
	return external_power_on;
}
#endif 

ssize_t charging_ic_show_status(struct device *dev,
			 struct device_attribute *attr,
			 char *buf)
{
	if(charging_ic_status == CHARGING_IC_DEACTIVE)
		return snprintf(buf, PAGE_SIZE, "0\n");
	else
		return snprintf(buf, PAGE_SIZE, "1\n");
}
ssize_t charging_ic_store_status(struct device *dev,
			  struct device_attribute *attr,
			  const char *buf,
			  size_t count)
{
	if(buf[0] == '0') {
		charging_ic_deactive();
	} else if(buf[0] == '1') {
		charging_ic_set_ta_mode();
	}
	return count;
}
DEVICE_ATTR(charging_state, 0664, charging_ic_show_status, charging_ic_store_status);

ssize_t charging_ic_show_poc(struct device *dev,
			 struct device_attribute *attr,
			 char *buf)
{
	int res;

	res = wake_lock_active(&power_off_charging_lock);

	if(res)
		return snprintf(buf, PAGE_SIZE, "1\n");
	return snprintf(buf, PAGE_SIZE, "0\n");

}
ssize_t charging_ic_store_poc(struct device *dev,
			  struct device_attribute *attr,
			  const char *buf,
			  size_t count)
{
	if(buf[0] == '0') {
		if(wake_lock_active(&power_off_charging_lock))
			wake_unlock(&power_off_charging_lock);
		printk("[Battery] Power Off Charging - End!\n");
	} else if(buf[0] == '1') {
		if(!wake_lock_active(&power_off_charging_lock))
			wake_lock(&power_off_charging_lock);
		printk("[Battery] Power Off Charging - Start!\n");
	}
	return count;
}
DEVICE_ATTR(power_off_charging, 0664, charging_ic_show_poc, charging_ic_store_poc);

static int charging_ic_probe(struct platform_device *dev)
{
	int ret = 0;

	ret = gpio_request(CHG_EN_SET_N_OMAP, "hub charging_ic_en");
	if (ret < 0) {
		printk(KERN_ERR "%s: Failed to request GPIO_%d for "
				"charging_ic\n", __func__, CHG_EN_SET_N_OMAP);
		return -ENOSYS;
	}
	gpio_direction_output(CHG_EN_SET_N_OMAP, 0);

	ret = gpio_request(CHG_STATUS_N_OMAP, "hub charging_ic_status");
	if (ret < 0) {
		printk(KERN_ERR "%s: Failed to request GPIO_%d for "
				"charging_ic_status\n", __func__,
				CHG_STATUS_N_OMAP);
		goto err_gpio_request_failed;
	}
	gpio_direction_input(CHG_STATUS_N_OMAP);

	ret = request_irq(gpio_to_irq(CHG_STATUS_N_OMAP),
			charging_ic_interrupt_handler,
			IRQF_TRIGGER_RISING,
			"Charging_ic_driver", NULL);
	if (ret < 0) {
		printk(KERN_ERR "%s: Failed to request IRQ for "
				"charging_ic_status\n", __func__);
		goto err_request_irq_failed;
	}

#ifdef CONFIG_LGE_CHARGE_CONTROL_BATTERY_FET
	ret = gpio_request(CHAR_CONTROL, "hub battery_fet");
	if (ret < 0) {
		printk(KERN_ERR "%s: Failed to request GPIO_%d for "
				"charging_ic\n", __func__, CHAR_CONTROL);
		goto err_gpio_request_char_control;
	}
	gpio_direction_output(CHAR_CONTROL, 0);
#endif 

	INIT_DELAYED_WORK(&charging_ic_int_work,
				charging_ic_work_func);

	wake_lock_init(&power_off_charging_lock, WAKE_LOCK_SUSPEND, "Power Off Charging");
	
	charging_ic_status = CHARGING_IC_DEACTIVE;

	hub_charging_ic_intialize();

	// for AT Command AT%CHARGE
	// sysfs path : /sys/devices/platform/hub_charging_ic/charging_state
	ret = device_create_file(&dev->dev, &dev_attr_charging_state);
	if (ret < 0) {
		pr_err("%s:File device creation failed: %d\n", __func__, ret);
		//ret = -ENODEV;
	}

	// for Power Off Charging
	// sysfs path : /sys/devices/platform/hub_charging_ic/power_off_charging
	ret = device_create_file(&dev->dev, &dev_attr_power_off_charging);
	if (ret < 0) {
		pr_err("%s:File device creation failed: %d\n", __func__, ret);
		//ret = -ENODEV;
	}

	enable_irq_wake(gpio_to_irq(CHG_STATUS_N_OMAP)); //20101104 taehwan.kim@lge.com enable charger wakeup
	return 0;

#ifdef CONFIG_LGE_CHARGE_CONTROL_BATTERY_FET
	gpio_free(CHAR_CONTROL);
err_gpio_request_char_control:
#endif 
	free_irq(gpio_to_irq(CHG_STATUS_N_OMAP), NULL);
err_request_irq_failed:
	gpio_free(CHG_STATUS_N_OMAP);
err_gpio_request_failed:
	gpio_free(CHG_EN_SET_N_OMAP);

	return ret;
}

static int charging_ic_remove(struct platform_device *dev)
{
	charging_ic_deactive();

	free_irq(CHG_STATUS_N_OMAP, NULL);

	gpio_free(CHG_EN_SET_N_OMAP);
	gpio_free(CHG_STATUS_N_OMAP);

	return 0;
}

static int charging_ic_suspend(struct platform_device *dev, pm_message_t state)
{
	dev->dev.power.power_state = state;
	return 0;
}

static int charging_ic_resume(struct platform_device *dev)
{
	dev->dev.power.power_state = PMSG_ON;
	return 0;
}

static struct platform_driver charging_ic_driver = {
	.probe = charging_ic_probe,
	.remove = charging_ic_remove,
	.suspend = charging_ic_suspend,
	.resume= charging_ic_resume,
	.driver = {
		.name = "hub_charging_ic",
	},
};

/* LGE_CHANGE_S [daewung.kim@lge.com] 2010-12-27, Reboot rather than pwr off when TA inserted */
extern char reset_mode;
extern void emergency_restart(void);

static int lge_chg_reboot_event(struct notifier_block *this,
				unsigned long event, void *cmd)
{
	int mode = 0;
	
	if (event == SYS_POWER_OFF)
	{
#if defined(CONFIG_HUB_MUIC)
		mode = get_muic_mode();
/*LGSI_LGP970_FroyoToGB_Reboot_ChargerAnimation_Ajeesh_24AUG2011_START*/
		if(((MUIC_NA_TA <= mode) && (mode <= MUIC_INVALID_CHG)) || (mode == MUIC_AP_USB))
/*LGSI_LGP970_FroyoToGB_Reboot_ChargerAnimation_Ajeesh_24AUG2011_END*/

// kibum.lee@lge.com 20120502 MUIC re-work start
#elif defined(CONFIG_MUIC)
		mode = muic_get_mode();
		if(((MUIC_NA_TA <= mode) && (mode <= MUIC_INVALID_CHG)) || (mode == MUIC_AP_USB))
// kibum.lee@lge.com 20120502 MUIC re-work end
#elif defined(CONFIG_LGE_OMAP3_EXT_PWR)
		if ( get_external_power_status() )
#else
#error
#endif
		{
			reset_mode = 'C';
			emergency_restart();
		}
	}
	
	return NOTIFY_DONE;
}

struct notifier_block lge_chg_reboot_nb = {
	.notifier_call = lge_chg_reboot_event,
};

extern int register_reboot_notifier(struct notifier_block *nb);
/* LGE_CHANGE_E [daewung.kim@lge.com] 2010-12-27, Reboot rather than pwr off when TA inserted */

static int __init hub_charging_ic_init(void)
{
/* LGE_CHANGE_S [daewung.kim@lge.com] 2010-12-27, Reboot rather than pwr off when TA inserted */
	register_reboot_notifier(&lge_chg_reboot_nb);
/* LGE_CHANGE_E [daewung.kim@lge.com] 2010-12-27, Reboot rather than pwr off when TA inserted */
	return platform_driver_register(&charging_ic_driver);
}

static void __exit hub_charging_ic_exit(void)
{
	platform_driver_unregister(&charging_ic_driver);
}

module_init(hub_charging_ic_init);
module_exit(hub_charging_ic_exit);

MODULE_AUTHOR("LG Electronics");
MODULE_DESCRIPTION("hub charging_ic Driver");
MODULE_LICENSE("GPL");

