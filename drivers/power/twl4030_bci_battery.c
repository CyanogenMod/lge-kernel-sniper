/*
 * linux/drivers/power/twl4030_bci_battery.c
 *
 * OMAP2430/3430 BCI battery driver for Linux
 *
 * Copyright (C) 2008 Texas Instruments, Inc.
 * Author: Texas Instruments, Inc.
 *
 * This package is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/i2c/twl.h>
#include <linux/power_supply.h>
#include <linux/i2c/twl4030-madc.h>
#include <linux/usb/otg.h>
#include <linux/slab.h>

// 20100624 taehwan.kim@lge.com  To add Hub battery support[START_LGE]
#if defined(CONFIG_HUB_MUIC)
#include "../hub/hub_muic.h"
#endif
#if defined(CONFIG_HUB_CHARGING_IC)
#include "../hub/hub_charging_ic.h"
#endif
#include <linux/max17043_fuelgauge.h>

#define BK_BATT 0 /*taehwan.kim@lge.com 2010-03-12 */
// 20100624 taehwan.kim@lge.com  To add Hub battery support[END_LGE]

#define T2_BATTERY_VOLT		0x04
#define T2_BATTERY_TEMP		0x06
#define T2_BATTERY_CUR		0x08

/* charger constants */
#define NO_PW_CONN		0
#define AC_PW_CONN		0x01
#define USB_PW_CONN		0x02

/* TWL4030_MODULE_USB */
#define REG_POWER_CTRL		0x0AC
#define OTG_EN			0x020
#define REG_PHY_CLK_CTRL	0x0FE
#define REG_PHY_CLK_CTRL_STS	0x0FF
#define PHY_DPLL_CLK		0x01

#define REG_BCICTL1		0x023
#define REG_BCICTL2		0x024
#define CGAIN			0x020
#define ITHEN			0x010
#define ITHSENS			0x007

/* Boot BCI flag bits */
#define BCIAUTOWEN		0x020
#define CONFIG_DONE		0x010
#define BCIAUTOUSB		0x002
#define BCIAUTOAC		0x001
#define BCIMSTAT_MASK		0x03F

/* Boot BCI register */
#define REG_BOOT_BCI		0x007
#define REG_CTRL1		0x00
#define REG_SW1SELECT_MSB	0x07
#define SW1_CH9_SEL		0x02
#define REG_CTRL_SW1		0x012
#define SW1_TRIGGER		0x020
#define EOC_SW1			0x002
#define REG_GPCH9		0x049
#define REG_STS_HW_CONDITIONS	0x0F
#define STS_VBUS		0x080
#define STS_CHG			0x02
#define REG_BCIMSTATEC		0x02
#define REG_BCIMFSTS4		0x010
#define REG_BCIMFSTS2		0x00E
#define REG_BCIMFSTS3		0x00F
#define REG_BCIMFSTS1		0x001
#define USBFASTMCHG		0x004
#define BATSTSPCHG		0x004
#define BATSTSMCHG		0x040
#define VBATOV4			0x020
#define VBATOV3			0x010
#define VBATOV2			0x008
#define VBATOV1			0x004
#define REG_BB_CFG		0x012
#define BBCHEN			0x010

/* Power supply charge interrupt */
#define REG_PWR_ISR1		0x00
#define REG_PWR_IMR1		0x01
#define REG_PWR_EDR1		0x05
#define REG_PWR_SIH_CTRL	0x007

#define USB_PRES		0x004
#define CHG_PRES		0x002

#define USB_PRES_RISING		0x020
#define USB_PRES_FALLING	0x010
#define CHG_PRES_RISING		0x008
#define CHG_PRES_FALLING	0x004
#define AC_STATEC		0x20
#define COR			0x004

/* interrupt status registers */
#define REG_BCIISR1A		0x0
#define REG_BCIISR2A		0x01

/* Interrupt flags bits BCIISR1 */
#define BATSTS_ISR1		0x080
#define VBATLVL_ISR1		0x001

/* Interrupt mask registers for int1*/
#define REG_BCIIMR1A		0x002
#define REG_BCIIMR2A		0x003

 /* Interrupt masks for BCIIMR1 */
#define BATSTS_IMR1		0x080
#define VBATLVL_IMR1		0x001

/* Interrupt edge detection register */
#define REG_BCIEDR1		0x00A
#define REG_BCIEDR2		0x00B
#define REG_BCIEDR3		0x00C

/* BCIEDR2 */
#define	BATSTS_EDRRISIN		0x080
#define BATSTS_EDRFALLING	0x040

/* BCIEDR3 */
#define	VBATLVL_EDRRISIN	0x02

/* Step size and prescaler ratio */
#define TEMP_STEP_SIZE		147
#define TEMP_PSR_R		100

#define VOLT_STEP_SIZE		588
#define VOLT_PSR_R		100

#define CURR_STEP_SIZE		147
#define CURR_PSR_R1		44
#define CURR_PSR_R2		80

#define BK_VOLT_STEP_SIZE	441
#define BK_VOLT_PSR_R		100

#define ENABLE		1
#define DISABLE		0


/* Tunning Value */
#ifdef CONFIG_LGE_LAB3_BOARD
/* LAB 3 */
#define TRICKLE_CHG_MAX 4160
#define TRICKLE_CHG_TIMER_START 4150
#define TRICKLE_CHG_MIN 4140

#define TRICKLE_LIMMIT_ALERT_CHG_MAX 			TRICKLE_CHG_MAX
#define TRICKLE_LIMMIT_ALERT_CHG_TIMER_START 	TRICKLE_CHG_TIMER_START
#define TRICKLE_LIMMIT_ALERT_CHG_MIN 			TRICKLE_CHG_MIN

#define FORCE_FULL_CHARGE_VOLTAGE_LEVEL 4210

// NO Battery adc 634 TODO XXX_mbk
#define NO_BATTERY_ADC_VALUE 625

#else
/* DCM */
#define TRICKLE_CHG_MAX 4220
#define TRICKLE_CHG_TIMER_START 4197
#define TRICKLE_CHG_MIN 4140

#define TRICKLE_LIMMIT_ALERT_CHG_MAX 4000
#define TRICKLE_LIMMIT_ALERT_CHG_TIMER_START 3950
#define TRICKLE_LIMMIT_ALERT_CHG_MIN 3900

#define FORCE_FULL_CHARGE_VOLTAGE_LEVEL 4230

#define NO_BATTERY_ADC_VALUE 720
#endif 


/* Ptr to thermistor table */
int *therm_tbl;

struct twl4030_bci_device_info {
	struct device		*dev;

	unsigned long		update_time;
	int			voltage_uV;
	int			bk_voltage_uV;
	int			current_uA;
	int			temp_C;
	int			charge_rsoc;
	int			charge_status;
// 20100624 taehwan.kim@lge.com  To add Hub battery support[START_LGE]
#if defined(CONFIG_MACH_LGE_HUB) || defined(CONFIG_MACH_LGE_SNIPER)
	int			battery_capacity;
	int			battery_present;
	int			previous_voltage_uV;
	int			previous_temp_C;
	int			previous_charge_status;
	int			previous_battery_capacity;
	int			previous_battery_present;
	int			previous_charge_rsoc;
	int			pre_pre_charge_rsoc;
	struct power_supply	ac;
	struct power_supply	usb;
#endif
// 20100624 taehwan.kim@lge.com  To add Hub battery support[END_LGE]

	struct notifier_block	nb;
	struct power_supply	bat;
	struct power_supply	bk_bat;
	struct power_supply usb_bat;
	struct delayed_work	twl4030_bci_monitor_work;
	struct delayed_work	twl4030_bk_bci_monitor_work;
};
// 20100624 taehwan.kim@lge.com  To add Hub battery support[START_LGE]
#if defined(CONFIG_MACH_LGE_HUB) || defined(CONFIG_MACH_LGE_SNIPER)

// Battery Spec.
#define TEMP_CRITICAL_UPPER	(650)
#define TEMP_CRITICAL_LOWER	(-300)
#define TEMP_LIMIT_UPPER	(600)
#define TEMP_LIMIT_LOWER	(-200)
#define MONITOR_WORK_TIME	(10 * HZ)

extern u32 wakeup_timer_seconds;	// from pm34xx.c
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
//extern int lcd_off_boot;			// to check factory mode
extern int lcd_off_boot ;
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
static struct twl4030_bci_device_info *refer_di = NULL;
static int start_monitor = 0;	//20101104 taehwan.kim@lge.com to fix trickle chg
#endif
// 20100624 taehwan.kim@lge.com  To add Hub battery support[END_LGE]

#if defined(CONFIG_MACH_LGE_HUB) || defined(CONFIG_MACH_LGE_SNIPER)
#else
static int usb_charger_flag;
#endif
static int LVL_1, LVL_2, LVL_3, LVL_4;

#if !defined(FUELGAUGE_AP_ONLY)
static int read_bci_val(u8 reg_1);
#endif

static inline int clear_n_set(u8 mod_no, u8 clear, u8 set, u8 reg);
#if defined(CONFIG_MACH_LGE_HUB) || defined(CONFIG_MACH_LGE_SNIPER)
#else
static int twl4030charger_presence(void);
#endif

// 20101226 dajin.kim@lge.com to handling EOC [Start]
static int end_of_charge = 0;
int set_end_of_charge(int complete)
{
	if(refer_di == NULL) {
		end_of_charge = 0;
		return 0;
	}
	if(refer_di->charge_rsoc == POWER_SUPPLY_TYPE_BATTERY) {
		end_of_charge = 0;
		return 0;
	}

	if(complete) {
		end_of_charge = 1;
	} else {
		end_of_charge = 0;
	}
	return end_of_charge;
}
EXPORT_SYMBOL(set_end_of_charge);
static struct timeval charging_monitor_start_time;
static int charging_monitor_started = 0;
static void set_charging_timer(int en)
{
	if(en) {
		if(!charging_monitor_started) {
			printk(KERN_DEBUG "[Battery] Charging Timer Enabled\n");
			charging_monitor_started = 1;
			do_gettimeofday(&charging_monitor_start_time);
		}
	} else {
		if(charging_monitor_started) {
			printk(KERN_DEBUG "[Battery] Charging Timer Disabled\n");
			charging_monitor_started = 0;
		}
	}
}
int charging_timeout(void)
{
	static struct timeval current_time;
	int time_passed;
	// monitoring not started
	if(charging_monitor_started == 0)
		return 0;

	do_gettimeofday(&current_time);
	if(charging_monitor_start_time.tv_sec > current_time.tv_sec) {
		time_passed = charging_monitor_start_time.tv_sec - current_time.tv_sec;
		time_passed = TIME_T_MAX - time_passed;
	} else {
		time_passed = current_time.tv_sec - charging_monitor_start_time.tv_sec;
	}

	//printk(KERN_DEBUG "[Battery] Time Passed : %dsec\n", time_passed);
	if(time_passed > 30 * 60)	// over 30 min
		return 1;
	return 0;
}
// 20101226 dajin.kim@lge.com to handling EOC [End]

/** @brief  To check battery present from other drivers
  @author taehwan.kim@lge.com
  @date   2010.11.25
  */
int check_battery_present(void)
{
	if(refer_di == NULL)
		return 0;
		
    if(refer_di->temp_C == 0xBA00E00)
    	return 0;
    return 1;
}
EXPORT_SYMBOL(check_battery_present);

/** @brief  To check PIF connected for factory test mode
    @author taehwan.kim@lge.com
    @date   2010.05.14
*/
static ssize_t pif_detect_show(struct device *dev,
			       struct device_attribute *attr,
			       char *buf)
{
	int val;

#if defined(CONFIG_HUB_MUIC)
	int muic_mode;
	muic_mode = get_muic_mode();

	if(muic_mode == MUIC_UNKNOWN) {
		val = -1;
	} else if (lcd_off_boot && !check_battery_present()) {
		val = 2;
	} else if (muic_mode == MUIC_AP_UART) {
		val = 1;
	} else if ((muic_mode == MUIC_CP_UART) || (muic_mode == MUIC_CP_USB)) {
		val = 2;
	} else {
		val = 0;
	}
#elif defined(CONFIG_LGE_OMAP3_EXT_PWR)
	switch( get_ext_pwr_type()) {
		case LT_CABLE:
			val = 1;
			break;
		default :
			val = 0;
			break;
	}
#else
#error
#endif 

	sprintf(buf, "%d\n", val);
	return (ssize_t)(strlen(buf) + 1);
}

/** @brief  To store battery capacity value that is sent via AT command from CP
    @author taehwan.kim@lge.com
    @date   2010.0602.
*/
static ssize_t batt_soc_store(struct device *dev, struct device_attribute *attr,
			      const char *buf, size_t count)
{
#if 0
	int val, batt_raw_cap;

	if(system_rev >= 4)	// B-Project Rev.D (AP Fuel Gauge)
		return count;

#if 1
	val = simple_strtoul(buf, NULL, 10);
#else
	sscanf(buf, "%d", &val);
#endif

	//batt_raw_cap = ((val * 100) / 95)-1;
	batt_raw_cap = val;
	if (batt_raw_cap > 100)
		batt_raw_cap = 100;

	if (batt_raw_cap < 5) {
		batt_raw_cap = twl4030battery_capacity(refer_di);
		printk(KERN_DEBUG "[BATTERY] use twl5030 capacity "
				"instead of fuel guage\n");
	}
	printk("[BATTERY] write AT+CBC = %d to batt_soc_storei as %d\n", val, batt_raw_cap);

	refer_di->battery_capacity = batt_raw_cap;
	charger_state_update_by_other();
#endif
	return count;
}

/** @brief  To send AT%FUELRST for gauge IC calibration in CP via AT-command
    @author taehwan.kim@lge.com
    @date   2010.0520.
*/
static ssize_t fuel_cal_show(struct device *dev,
			     struct device_attribute *attr,
			     char *buf)
{
#if 0
	int val, gauge_diff;
	static int gauge_cal_check = 1;
	TYPE_MUIC_MODE muic_mode = MUIC_NONE;

	if(system_rev >= 4) {	// B-Project Rev.D (AP Fuel Gauge)
		val = 0;
		return sprintf(buf, "%d\n", val);
	}

	muic_mode = get_muic_mode();
	gauge_diff = refer_di->battery_capacity -(refer_di->voltage_uV-3300) *100 /900;

	if (((gauge_diff > 30) || (gauge_diff < -40)) && (muic_mode != MUIC_NONE)) {
		printk("[BATTERY] error in gauge IC value, difference value is %d \n",gauge_diff);
		gauge_cal_check = 0;
	}
	printk("[BATTERY] success to access fuel_cal_show \n");
	if (gauge_cal_check == 0)
	{
		val = 1;
		printk("[BATTERY] Gauge Calibration started in fuel_cal_show\n");
		if (refer_di->battery_present == 1) charging_ic_deactive();
		//refer_di->gauge_error = 1;
		gauge_cal_check = 1;
	}
	else
	{
		//refer_di->gauge_error = 0;
		val = 0;
	}
#endif
	return sprintf(buf, "0\n");
}
static DEVICE_ATTR(pif, S_IRUGO | S_IWUSR, pif_detect_show, NULL);
static DEVICE_ATTR(gauge_if, S_IRUGO | S_IWUSR, fuel_cal_show, batt_soc_store);
// 20110312 dajin.kim@lge.com for twl4030 VBUS Detect [START]
static int twl4030vbus_en(void)
{
	int ret = 0;
	u8 hw_conditions = 0;
	/* read charger power supply status */
	ret = twl_i2c_read_u8(TWL4030_MODULE_PM_MASTER, &hw_conditions,
		REG_STS_HW_CONDITIONS);
	if (ret)
		return ret;

	if(hw_conditions & STS_VBUS)
		return 1;

	return 0;
}
// 20110312 dajin.kim@lge.com for twl4030 VBUS Detect [END]
/*
 * Report and clear the charger presence event.
 */
static inline int twl4030charger_presence_evt(void)
{
	int ret = 0;
	u8 chg_sts = 0, set = 0, clear = 0;

	/* read charger power supply status */
	ret = twl_i2c_read_u8(TWL4030_MODULE_PM_MASTER, &chg_sts,
		REG_STS_HW_CONDITIONS);
	if (ret)
		return IRQ_NONE;

	if (chg_sts & STS_CHG) { /* If the AC charger have been connected */
		/* configuring falling edge detection for CHG_PRES */
		set = CHG_PRES_FALLING;
		clear = CHG_PRES_RISING;
	} else { /* If the AC charger have been disconnected */
		/* configuring rising edge detection for CHG_PRES */
		set = CHG_PRES_RISING;
		clear = CHG_PRES_FALLING;
	}

	/* Update the interrupt edge detection register */
	clear_n_set(TWL4030_MODULE_INT, clear, set, REG_PWR_EDR1);

	return 0;
}

/*
 * Interrupt service routine
 *
 * Attends to TWL 4030 power module interruptions events, specifically
 * USB_PRES (USB charger presence) CHG_PRES (AC charger presence) events
 *
 */
static irqreturn_t twl4030charger_interrupt(int irq, void *_di)
{
#ifdef CONFIG_LOCKDEP
	/* WORKAROUND for lockdep forcing IRQF_DISABLED on us, which
	 * we don't want and can't tolerate.  Although it might be
	 * friendlier not to borrow this thread context...
	 */
	local_irq_enable();
#endif

	twl4030charger_presence_evt();

	return IRQ_HANDLED;
}

/*
 * This function handles the twl4030 battery presence interrupt
 */
static int twl4030battery_presence_evt(void)
{
	int ret;
	u8 batstsmchg = 0, batstspchg = 0;

	/* check for the battery presence in main charge*/
	ret = twl_i2c_read_u8(TWL4030_MODULE_MAIN_CHARGE,
			&batstsmchg, REG_BCIMFSTS3);
	if (ret)
		return ret;

	/* check for the battery presence in precharge */
	ret = twl_i2c_read_u8(TWL4030_MODULE_PRECHARGE,
			&batstspchg, REG_BCIMFSTS1);
	if (ret)
		return ret;

	/*
	 * REVISIT: Physically inserting/removing the batt
	 * does not seem to generate an int on 3430ES2 SDP.
	 */
	if ((batstspchg & BATSTSPCHG) || (batstsmchg & BATSTSMCHG)) {
		/* In case of the battery insertion event */
		ret = clear_n_set(TWL4030_MODULE_INTERRUPTS, BATSTS_EDRRISIN,
			BATSTS_EDRFALLING, REG_BCIEDR2);
		if (ret)
			return ret;
	} else {
		/* In case of the battery removal event */
		ret = clear_n_set(TWL4030_MODULE_INTERRUPTS, BATSTS_EDRFALLING,
			BATSTS_EDRRISIN, REG_BCIEDR2);
		if (ret)
			return ret;
	}

	return 0;
}

/*
 * This function handles the twl4030 battery voltage level interrupt.
 */
static int twl4030battery_level_evt(void)
{
	int ret = 0;
	u8 mfst = 0;

	/* checking for threshold event */
	ret = twl_i2c_read_u8(TWL4030_MODULE_MAIN_CHARGE,
			&mfst, REG_BCIMFSTS2);
	if (ret)
		return ret;

	/* REVISIT could use a bitmap */
	if (mfst & VBATOV4) {
		LVL_4 = 1;
		LVL_3 = 0;
		LVL_2 = 0;
		LVL_1 = 0;
	} else if (mfst & VBATOV3) {
		LVL_4 = 0;
		LVL_3 = 1;
		LVL_2 = 0;
		LVL_1 = 0;
	} else if (mfst & VBATOV2) {
		LVL_4 = 0;
		LVL_3 = 0;
		LVL_2 = 1;
		LVL_1 = 0;
	} else {
		LVL_4 = 0;
		LVL_3 = 0;
		LVL_2 = 0;
		LVL_1 = 1;
	}

	return 0;
}

/*
 * Interrupt service routine
 *
 * Attends to BCI interruptions events,
 * specifically BATSTS (battery connection and removal)
 * VBATOV (main battery voltage threshold) events
 *
 */
static irqreturn_t twl4030battery_interrupt(int irq, void *_di)
{
	u8 isr1a_val = 0, isr2a_val = 0, clear_2a = 0, clear_1a = 0;
	int ret = 0;

#ifdef CONFIG_LOCKDEP
	/* WORKAROUND for lockdep forcing IRQF_DISABLED on us, which
	 * we don't want and can't tolerate.  Although it might be
	 * friendlier not to borrow this thread context...
	 */
	local_irq_enable();
#endif

	ret = twl_i2c_read_u8(TWL4030_MODULE_INTERRUPTS, &isr1a_val,
				REG_BCIISR1A);
	if (ret)
		return IRQ_NONE;

	ret = twl_i2c_read_u8(TWL4030_MODULE_INTERRUPTS, &isr2a_val,
				REG_BCIISR2A);
	if (ret)
		return IRQ_NONE;

	clear_2a = (isr2a_val & VBATLVL_ISR1) ? (VBATLVL_ISR1) : 0;
	clear_1a = (isr1a_val & BATSTS_ISR1) ? (BATSTS_ISR1) : 0;

	/* cleaning BCI interrupt status flags */
	ret = twl_i2c_write_u8(TWL4030_MODULE_INTERRUPTS,
			clear_1a , REG_BCIISR1A);
	if (ret)
		return IRQ_NONE;

	ret = twl_i2c_write_u8(TWL4030_MODULE_INTERRUPTS,
			clear_2a , REG_BCIISR2A);
	if (ret)
		return IRQ_NONE;

	/* battery connetion or removal event */
	if (isr1a_val & BATSTS_ISR1)
		twl4030battery_presence_evt();
	/* battery voltage threshold event*/
	else if (isr2a_val & VBATLVL_ISR1)
		twl4030battery_level_evt();
	else
		return IRQ_NONE;

	return IRQ_HANDLED;
}

/*
 * Enable/Disable hardware battery level event notifications.
 */
static int twl4030battery_hw_level_en(int enable)
{
	int ret;

	if (enable) {
		/* unmask VBATOV interrupt for INT1 */
		ret = clear_n_set(TWL4030_MODULE_INTERRUPTS, VBATLVL_IMR1,
			0, REG_BCIIMR2A);
		if (ret)
			return ret;

		/* configuring interrupt edge detection for VBATOv */
		ret = clear_n_set(TWL4030_MODULE_INTERRUPTS, 0,
			VBATLVL_EDRRISIN, REG_BCIEDR3);
		if (ret)
			return ret;
	} else {
		/* mask VBATOV interrupt for INT1 */
		ret = clear_n_set(TWL4030_MODULE_INTERRUPTS, 0,
			VBATLVL_IMR1, REG_BCIIMR2A);
		if (ret)
			return ret;
	}

	return 0;
}

/*
 * Enable/disable hardware battery presence event notifications.
 */
static int twl4030battery_hw_presence_en(int enable)
{
	int ret;

	if (enable) {
		/* unmask BATSTS interrupt for INT1 */
		ret = clear_n_set(TWL4030_MODULE_INTERRUPTS, BATSTS_IMR1,
			0, REG_BCIIMR1A);
		if (ret)
			return ret;

		/* configuring interrupt edge for BATSTS */
		ret = clear_n_set(TWL4030_MODULE_INTERRUPTS, 0,
			BATSTS_EDRRISIN | BATSTS_EDRFALLING, REG_BCIEDR2);
		if (ret)
			return ret;
	} else {
		/* mask BATSTS interrupt for INT1 */
		ret = clear_n_set(TWL4030_MODULE_INTERRUPTS, 0,
			BATSTS_IMR1, REG_BCIIMR1A);
		if (ret)
			return ret;
	}

	return 0;
}

/*
 * Enable/Disable AC Charge funtionality.
 */
static int twl4030charger_ac_en(int enable)
{
	int ret;

	if (enable) {
		/* forcing the field BCIAUTOAC (BOOT_BCI[0) to 1 */
		ret = clear_n_set(TWL4030_MODULE_PM_MASTER, 0,
			(CONFIG_DONE | BCIAUTOWEN | BCIAUTOAC),
			REG_BOOT_BCI);
		if (ret)
			return ret;
	} else {
		/* forcing the field BCIAUTOAC (BOOT_BCI[0) to 0*/
		ret = clear_n_set(TWL4030_MODULE_PM_MASTER, BCIAUTOAC,
			(CONFIG_DONE | BCIAUTOWEN),
			REG_BOOT_BCI);
		if (ret)
			return ret;
	}

	return 0;
}

/*
 * Enable/Disable USB Charge funtionality.
 */
#if defined(CONFIG_MACH_LGE_HUB)	/* LGE_CHANGE [HUB: newcomet@lge.com] on 2009-11-17, for <Charger int. from MUIC> */
int twl4030charger_usb_en(int enable)
{
	return 0;
}
#else
int twl4030charger_usb_en(int enable)
{
	u8 value;
	int ret;
	unsigned long timeout;

	if (enable) {
		/* Check for USB charger conneted */
		ret = twl4030charger_presence();
		if (ret < 0)
			return ret;

		if (!(ret & USB_PW_CONN))
			return -ENXIO;

		/* forcing the field BCIAUTOUSB (BOOT_BCI[1]) to 1 */
		ret = clear_n_set(TWL4030_MODULE_PM_MASTER, 0,
			(CONFIG_DONE | BCIAUTOWEN | BCIAUTOUSB),
			REG_BOOT_BCI);
		if (ret)
			return ret;

		ret = clear_n_set(TWL4030_MODULE_USB, 0, PHY_DPLL_CLK,
			REG_PHY_CLK_CTRL);
		if (ret)
			return ret;

		value = 0;
		timeout = jiffies + msecs_to_jiffies(50);

		while ((!(value & PHY_DPLL_CLK)) &&
			time_before(jiffies, timeout)) {
			udelay(10);
			ret = twl_i2c_read_u8(TWL4030_MODULE_USB, &value,
				REG_PHY_CLK_CTRL_STS);
			if (ret)
				return ret;
		}

		/* OTG_EN (POWER_CTRL[5]) to 1 */
		ret = clear_n_set(TWL4030_MODULE_USB, 0, OTG_EN,
			REG_POWER_CTRL);
		if (ret)
			return ret;

		mdelay(50);

		/* forcing USBFASTMCHG(BCIMFSTS4[2]) to 1 */
		ret = clear_n_set(TWL4030_MODULE_MAIN_CHARGE, 0,
			USBFASTMCHG, REG_BCIMFSTS4);
		if (ret)
			return ret;
	} else {
		twl4030charger_presence();
		ret = clear_n_set(TWL4030_MODULE_PM_MASTER, BCIAUTOUSB,
			(CONFIG_DONE | BCIAUTOWEN), REG_BOOT_BCI);
		if (ret)
			return ret;
	}

	return 0;
}
#endif /* CONFIG_MACH_LGE_HUB */

/*
 * Return battery temperature
 * Or < 0 on failure.
 */
#if defined(CONFIG_MACH_LGE_HUB) || defined(CONFIG_MACH_LGE_SNIPER)
struct temperature_table {
	int tmp;	// temperature * 10
	int adc;	// adc raw data
};
static int adc2temperature(int adc)
{
	#define TEMP_MULTIPLEX 10
	static struct temperature_table thm_tbl[] = {
		{ 600,	120},	//  60 C
		{ 500,	155},	//  50 C
		{ 400,	205},	//  40 C
		{ 300,	265},	//  30 C
		{ 200,	350},	//  20 C
		{ 100,	440},	//  10 C
		{   0,	525},	//  0 C
		{-100,	580},	// -10 C
		{-200,	645},	// -20 C
		{-1, -1}		// end of data
	};
		
	int tmp;
	int grad, inter;
	int i = 0;

	// Select Data
	while(thm_tbl[i].adc != -1) {
		if(adc < thm_tbl[i].adc)
			break;
		i++;
	}
	// Calculate
	if(i == 0) {
		grad = -5 * TEMP_MULTIPLEX;		// Guessing Value
		inter = (thm_tbl[0].tmp * TEMP_MULTIPLEX) - grad * thm_tbl[0].adc;
	} else if(thm_tbl[i].adc == -1) {
		grad = -2 * TEMP_MULTIPLEX;		// Guessing Value
		inter = (thm_tbl[i-1].tmp * TEMP_MULTIPLEX) - grad * thm_tbl[i-1].adc;
	} else {
		grad = (thm_tbl[i].tmp - thm_tbl[i-1].tmp) * TEMP_MULTIPLEX / (thm_tbl[i].adc - thm_tbl[i-1].adc);
		inter = (thm_tbl[i].tmp * TEMP_MULTIPLEX) - grad * thm_tbl[i].adc;
	}
	tmp = grad * adc + inter;
	tmp /= TEMP_MULTIPLEX;
	
	return tmp;
	#undef TEMP_MULTIPLEX
}
/* LGE_CHANGE [HUB: newcomet@lge.com] on 2009-11-17, for <battery temp.> */
/* HUB use ADC2 port for battery temperature measurement */
/* B-Project use ADC2 port for battery temperature measurement */
/* ADCIN2 ~ ADCIN7 */
#define CONV_VOLTAGE(value) ((value * 150000) / (60 * 1023))
/* ADCIN8 */
#define CONV_VOLTAGE_8(value) ( (value * 7000) / 1023 )

static int twl4030battery_temperature(void)
{
	struct twl4030_madc_request req;
	int temp;

	req.channels = (1 << 2);
	req.do_avg = 0;
	req.method = TWL4030_MADC_SW1;
	req.active = 0;
	req.func_cb = NULL;
	twl4030_madc_conversion(&req);
	temp = (u16)req.rbuf[2];

    //20101109 taehwan.kim@lge.com Defence code when TWL4030_madc read fail [START_LGE]
    if ((temp < 0) || (temp > 1000))
    {
        printk(KERN_ERR "Fail to read twl4030_madc i2c for temperature \n");
        temp = 200;
    }
    //20101109 taehwan.kim@lge.com Defence code when TWL4030_madc read fail [END_LGE]

#if 0
	// VBUS Voltage Check ( ADCIN8 )
	req.channels = (1 << 8);
	req.do_avg = 0;
	req.method = TWL4030_MADC_SW1;
	req.active = 0;
	req.func_cb = NULL;
	twl4030_madc_conversion(&req);
	printk("%s, VBUS %dmV,%d(0x%03X)\n", __FUNCTION__,CONV_VOLTAGE_8((u16)req.rbuf[8]), (u16)req.rbuf[8], (u16)req.rbuf[8]);
#endif 

	if (temp > NO_BATTERY_ADC_VALUE) {		// Floating BATT Temp Pin
		return 0xBA00E00;	// means No Battery or Dummy Battery
	}
	
	// 20101226 dajin.kim@lge.com Convert adc value to temperature
	temp = adc2temperature(temp);

	//printk(KERN_INFO "[charging_msg] %s: battery temperature %dmV,%d(0x%03X) -> degreed %dC\n", __FUNCTION__,CONV_VOLTAGE((u16)req.rbuf[2]), (u16)req.rbuf[2], (u16)req.rbuf[2], temp);

    return temp;

}
#else
static int twl4030battery_temperature(void)
{
	u8 val = 0;
	int temp, curr, volt, res, ret;

	/* Getting and calculating the thermistor voltage */
	ret = read_bci_val(T2_BATTERY_TEMP);
	if (ret < 0)
		return ret;

	volt = (ret * TEMP_STEP_SIZE) / TEMP_PSR_R;

	/* Getting and calculating the supply current in micro ampers */
	ret = twl_i2c_read_u8(TWL4030_MODULE_MAIN_CHARGE, &val,
		 REG_BCICTL2);
	if (ret)
		return 0;

	curr = ((val & ITHSENS) + 1) * 10;

	/* Getting and calculating the thermistor resistance in ohms*/
	res = volt * 1000 / curr;

	/*calculating temperature*/
	for (temp = 58; temp >= 0; temp--) {
		int actual = therm_tbl[temp];
		if ((actual - res) >= 0)
			break;
	}

	/* Negative temperature */
	if (temp < 3) {
		if (temp == 2)
			temp = -1;
		else if (temp == 1)
			temp = -2;
		else
			temp = -3;
	}

	return temp + 1;
}
#endif

/*
 * Return battery voltage
 * Or < 0 on failure.
 */
static int twl4030battery_voltage(void)
{
#ifdef FUELGAUGE_AP_ONLY
    return max17043_get_voltage();
#else
	int volt;
	u8 hwsts;
	struct twl4030_madc_request req;

	if(system_rev >= 4)	// B-Project Rev.D (AP Fuel Gauge)
		return max17043_get_voltage();

	twl_i2c_read_u8(TWL4030_MODULE_PM_MASTER, &hwsts,
		REG_STS_HW_CONDITIONS);

	if ((hwsts & STS_CHG) || (hwsts & STS_VBUS)) {
		/* AC or USB charger connected
		 * BCI Module requests MADC for info about BTEM,VBUS,ICHG,VCHG
		 * every 50ms. This info is made available through BCI reg
		 */
		volt = read_bci_val(T2_BATTERY_VOLT);
		return (volt * VOLT_STEP_SIZE) / VOLT_PSR_R;
	} else {
		/* No charger present.
		* BCI registers is not automatically updated.
		* Request MADC for information - 'SW1 software conversion req'
		*/
		req.channels = (1 << 12);
		req.do_avg = 0;
		req.method = TWL4030_MADC_SW1;
		req.active = 0;
		req.func_cb = NULL;
		twl4030_madc_conversion(&req);
		volt = (u16)req.rbuf[12];
		return (volt * VOLT_STEP_SIZE) / VOLT_PSR_R;
	}
#endif
}

#if defined(CONFIG_MACH_LGE_HUB)
#else
/*
 * Return the battery current
 * Or < 0 on failure.
 */
static int twl4030battery_current(void)
{
	int ret, curr = read_bci_val(T2_BATTERY_CUR);
	u8 val = 0;

	ret = twl_i2c_read_u8(TWL4030_MODULE_MAIN_CHARGE, &val,
		REG_BCICTL1);
	if (ret)
		return ret;

	if (val & CGAIN) /* slope of 0.44 mV/mA */
		return (curr * CURR_STEP_SIZE) / CURR_PSR_R1;
	else /* slope of 0.88 mV/mA */
		return (curr * CURR_STEP_SIZE) / CURR_PSR_R2;
}
#endif
/*
 * Return battery capacity
 * Or < 0 on failure.
 */
static int twl4030battery_capacity(struct twl4030_bci_device_info *di)
{
	int ret = 0;

#if defined(CONFIG_MACH_LGE_HUB) || defined(CONFIG_MACH_LGE_SNIPER)

#ifdef FUELGAUGE_AP_ONLY
    ret = max17043_get_capacity();
#else
	static int vol_history[20] = {0};
	static int vol_history_idx = 0;
	int vol_normalize = 0;
	int i = 0;

	if(system_rev >= 4) {	// B-Project Rev.D (AP Fuel Gauge)
		ret = max17043_get_capacity();
	} else {				// Use Calculated capacity from voltage
		vol_history[vol_history_idx++] = di->voltage_uV;

		if(vol_history_idx >= 20)
			vol_history_idx = 0;

		while(vol_history[i] != 0 && i < 20) {
			vol_normalize += vol_history[i++];
		}
		if(i != 0)
			vol_normalize = vol_normalize / i;
		ret = (vol_normalize - 3200) / 10;
	}
#endif

	if(ret < 0)
		ret = 0;
	else if(ret > 100)
		ret = 100;
#else
	/*
	 * need to get the correct percentage value per the
	 * battery characteristics. Approx values for now.
	 */
	if (di->voltage_uV < 3230 || LVL_1) {
		ret = 5;
		LVL_1 = 0;
	} else if ((di->voltage_uV < 3340 && di->voltage_uV > 3230)
		|| LVL_2) {
		ret = 20;
		LVL_2 = 0;
	} else if ((di->voltage_uV < 3550 && di->voltage_uV > 3340)
		|| LVL_3) {
		ret = 50;
		LVL_3 = 0;
	} else if ((di->voltage_uV < 3830 && di->voltage_uV > 3550)
		|| LVL_4) {
		ret = 75;
		LVL_4 = 0;
	} else if (di->voltage_uV > 3830)
		ret = 90;
#endif
	return ret;
}

#if BK_BATT
/*
 * Return the battery backup voltage
 * Or < 0 on failure.
 */
static int twl4030backupbatt_voltage(void)
{
	struct twl4030_madc_request req;
	int temp;

	req.channels = (1 << 9);
	req.do_avg = 0;
	req.method = TWL4030_MADC_SW1;
	req.active = 0;
	req.func_cb = NULL;
	twl4030_madc_conversion(&req);
	temp = (u16)req.rbuf[9];

	return  (temp * BK_VOLT_STEP_SIZE) / BK_VOLT_PSR_R;
}
#endif

#if defined(CONFIG_MACH_LGE_HUB) || defined(CONFIG_MACH_LGE_SNIPER)
#else
/*
 * Returns an integer value, that means,
 * NO_PW_CONN  no power supply is connected
 * AC_PW_CONN  if the AC power supply is connected
 * USB_PW_CONN  if the USB power supply is connected
 * AC_PW_CONN + USB_PW_CONN if USB and AC power supplies are both connected
 *
 * Or < 0 on failure.
 */
static int twl4030charger_presence(void)
{
	int ret = 0;
	u8 hwsts = 0;

	ret = twl_i2c_read_u8(TWL4030_MODULE_PM_MASTER, &hwsts,
		REG_STS_HW_CONDITIONS);
	if (ret) {
		pr_err("twl4030_bci: error reading STS_HW_CONDITIONS\n");
		return ret;
	}

	ret = (hwsts & STS_CHG) ? AC_PW_CONN : NO_PW_CONN;
	ret += (hwsts & STS_VBUS) ? USB_PW_CONN : NO_PW_CONN;

	if (ret & USB_PW_CONN)
		usb_charger_flag = 1;
	else
		usb_charger_flag = 0;

	return ret;

}

/*
 * Returns the main charge FSM status
 * Or < 0 on failure.
 */
static int twl4030bci_status(void)
{
	int ret;
	u8 status = 0;

	ret = twl_i2c_read_u8(TWL4030_MODULE_MAIN_CHARGE,
		&status, REG_BCIMSTATEC);
	if (ret) {
		pr_err("twl4030_bci: error reading BCIMSTATEC\n");
		return ret;
	}

	return (int) (status & BCIMSTAT_MASK);
}
#endif

#if !defined(FUELGAUGE_AP_ONLY)
static int read_bci_val(u8 reg)
{
	int ret, temp;
	u8 val = 0;

// 20100816 taehwan.kim@lge.com Set TWL4030 register for MADC voltage check [START_LGE]
#if defined(CONFIG_MACH_LGE_HUB) || defined(CONFIG_MACH_LGE_SNIPER)
    ret = clear_n_set(TWL4030_MODULE_MAIN_CHARGE, 0, USBFASTMCHG, REG_BCIMFSTS4);
#endif //CONFIG_MACH_LGE_HUB
// 20100816 taehwan.kim@lge.com Set TWL4030 register for MADC voltage check [END_LGE]

	/* reading MSB */
	ret = twl_i2c_read_u8(TWL4030_MODULE_MAIN_CHARGE, &val,
		reg + 1);
	if (ret)
		return ret;

	temp = ((int)(val & 0x03)) << 8;

	/* reading LSB */
	ret = twl_i2c_read_u8(TWL4030_MODULE_MAIN_CHARGE, &val,
		reg);
	if (ret)
		return ret;

	return temp | val;
}
#endif

/*
 * Settup the twl4030 BCI module to enable backup
 * battery charging.
 */
static int twl4030backupbatt_voltage_setup(void)
{
	int ret;

	/* Starting backup batery charge */
	ret = clear_n_set(TWL4030_MODULE_PM_RECEIVER, 0, BBCHEN,
		REG_BB_CFG);
	if (ret)
		return ret;

	return 0;
}

#if defined(CONFIG_MACH_LGE_HUB) || defined(CONFIG_MACH_LGE_SNIPER)
#else
/*
 * Settup the twl4030 BCI module to measure battery
 * temperature
 */
static int twl4030battery_temp_setup(void)
{
	int ret;

	/* Enabling thermistor current */
	ret = clear_n_set(TWL4030_MODULE_MAIN_CHARGE, 0, ITHEN,
		REG_BCICTL1);
	if (ret)
		return ret;

	return 0;
}
#endif
/*
 * Sets and clears bits on an given register on a given module
 */
static inline int clear_n_set(u8 mod_no, u8 clear, u8 set, u8 reg)
{
	int ret;
	u8 val = 0;

	/* Gets the initial register value */
	ret = twl_i2c_read_u8(mod_no, &val, reg);
	if (ret)
		return ret;

	/* Clearing all those bits to clear */
	val &= ~(clear);

	/* Setting all those bits to set */
	val |= set;

	/* Update the register */
	ret = twl_i2c_write_u8(mod_no, val, reg);
	if (ret)
		return ret;

	return 0;
}

static int twl4030battery_charger_event(struct notifier_block *nb,
		unsigned long event, void *_data)
{
	twl4030charger_usb_en(event == USB_EVENT_VBUS);
	return 0;
}

static enum power_supply_property twl4030_bci_battery_props[] = {
	POWER_SUPPLY_PROP_STATUS,
	POWER_SUPPLY_PROP_ONLINE,
	POWER_SUPPLY_PROP_VOLTAGE_NOW,
	POWER_SUPPLY_PROP_CURRENT_NOW,
	POWER_SUPPLY_PROP_CAPACITY,
	POWER_SUPPLY_PROP_TEMP,
#if defined(CONFIG_MACH_LGE_HUB)	/* LGE_CHANGE [HUB: newcomet@lge.com] on 2009-9-23, for <HAL:battery info. path> */
	POWER_SUPPLY_PROP_HEALTH,
	POWER_SUPPLY_PROP_TECHNOLOGY,
    POWER_SUPPLY_PROP_PRESENT,
#endif
};

#if BK_BATT
static enum power_supply_property twl4030_bk_bci_battery_props[] = {
	POWER_SUPPLY_PROP_VOLTAGE_NOW,
};
#endif

#if defined(CONFIG_MACH_LGE_HUB)	/* LGE_CHANGE [HUB: newcomet@lge.com] on 2009-9-29, for <HAL:battery info. path> */
static enum power_supply_property twl4030_ac_usb_bci_battery_props[] = {
	POWER_SUPPLY_PROP_ONLINE,
};
#endif

#if BK_BATT
static void
twl4030_bk_bci_battery_read_status(struct twl4030_bci_device_info *di)
{
	di->bk_voltage_uV = twl4030backupbatt_voltage();
}

static void twl4030_bk_bci_battery_work(struct work_struct *work)
{
	struct twl4030_bci_device_info *di = container_of(work,
		struct twl4030_bci_device_info,
		twl4030_bk_bci_monitor_work.work);

	twl4030_bk_bci_battery_read_status(di);
#if defined(CONFIG_MACH_LGE_HUB) || defined(CONFIG_MACH_LGE_SNIPER)
	schedule_delayed_work(&di->twl4030_bk_bci_monitor_work, HZ*10);
#else
	schedule_delayed_work(&di->twl4030_bk_bci_monitor_work, 500);
#endif
}
#endif

static void twl4030_bci_battery_read_status(struct twl4030_bci_device_info *di)
{
#if defined(CONFIG_MACH_LGE_HUB) || defined(CONFIG_MACH_LGE_SNIPER)
	/* Read Battery Status */
	di->temp_C = twl4030battery_temperature();		// Read Temperature
	di->battery_present = check_battery_present();		// Set Battery Present
#ifdef FUELGAUGE_AP_ONLY
	if(di->battery_present) {			// Adjust RCOMP for fuelgauge(Rev.D)
#else
	if(system_rev >= 4 && di->battery_present) {		// Adjust RCOMP for fuelgauge(Rev.D)
#endif
		max17043_set_rcomp_by_temperature(di->temp_C);
		// TODO : max17043_update
	}
	di->voltage_uV = twl4030battery_voltage();		// Read Voltage
	di->battery_capacity = twl4030battery_capacity(di);	// Read Capacity
	di->battery_present = check_battery_present();		// Set Battery Present
	/* hub do not use BCI block. so we cannot measure battery current */
	di->current_uA = 0;
#else
	di->temp_C = twl4030battery_temperature();
	di->voltage_uV = twl4030battery_voltage();
	di->current_uA = twl4030battery_current();
	di->capacity = twl4030battery_capacity(di);
#endif
}

static void set_battery_status(struct twl4030_bci_device_info *di)
{
#if defined(CONFIG_HUB_MUIC)
	TYPE_MUIC_MODE muic_mode = get_muic_mode();

	// Set Power Resources
	switch (muic_mode) {
		case MUIC_AP_UART:
		case MUIC_CP_UART:
		case MUIC_CP_USB:
			di->charge_rsoc = POWER_SUPPLY_TYPE_UPS;
			wakeup_timer_seconds = 0;	// disable wakeup.
			if(!twl4030vbus_en()) {
				di->charge_rsoc = POWER_SUPPLY_TYPE_BATTERY;				
			}
			break;
		case MUIC_NA_TA:
		case MUIC_LG_TA:
		case MUIC_HCHH:
		case MUIC_INVALID_CHG:
			di->charge_rsoc = POWER_SUPPLY_TYPE_MAINS;
                        wakeup_timer_seconds = 60;	// to wake up to monitor battery when charging mode
			if(!twl4030vbus_en()) {
				printk(KERN_WARNING "[Battery] MUIC mode is TA but VBUS is low.\n");
				//muic_device_detection(NOT_UPON_IRQ);
				di->charge_rsoc = POWER_SUPPLY_TYPE_BATTERY;
			}
			break;
		case MUIC_AP_USB:
			di->charge_rsoc = POWER_SUPPLY_TYPE_USB;
                        wakeup_timer_seconds = 0;	// disable wakeup. The phone will be wakeup because of wakelock
			if(!twl4030vbus_en()) {
				printk(KERN_WARNING "[Battery] Forced AP_USB.\n");
				//muic_device_detection(NOT_UPON_IRQ);
				di->charge_rsoc = POWER_SUPPLY_TYPE_BATTERY;
			}
			break;
		default :
			di->charge_rsoc = POWER_SUPPLY_TYPE_BATTERY;
                        wakeup_timer_seconds = 0;	// disable wakeup.
			if(di->battery_capacity <= 1)	// enable wakeup for monitoring power-off voltage
				wakeup_timer_seconds = 60;
			if(twl4030vbus_en() && muic_mode != MUIC_UNKNOWN) {
				printk(KERN_WARNING "[Battery] MUIC mode is NONE but VBUS is high.\n");
				//muic_device_detection(NOT_UPON_IRQ);
			}
		break;
	}
#elif defined(CONFIG_LGE_OMAP3_EXT_PWR)
	// Set Power Resources
	switch( get_ext_pwr_type() ) {
		case LT_CABLE:
			di->charge_rsoc = POWER_SUPPLY_TYPE_UPS;
			break;
		case TA_CABLE:
		case FORGED_TA_CABLE:
			di->charge_rsoc = POWER_SUPPLY_TYPE_MAINS;
			break;
		case USB_CABLE:
		case ABNORMAL_USB_100MA:
		case ABNORMAL_USB_400MA:
			di->charge_rsoc = POWER_SUPPLY_TYPE_USB;
			break;
		case NO_CABLE:
			di->charge_rsoc = POWER_SUPPLY_TYPE_BATTERY;
			break;
		case NO_INIT_CABLE:
		case UNKNOWN_CABLE:
		default:
			di->charge_rsoc = POWER_SUPPLY_TYPE_BATTERY;
			break;
	}
#else
#error
#endif 

	switch( di->charge_rsoc ) {
		case POWER_SUPPLY_TYPE_UPS:
			wakeup_timer_seconds = 0;	// disable wakeup.
			break;
		case POWER_SUPPLY_TYPE_MAINS:
			wakeup_timer_seconds = 60;	// to wake up to monitor battery when charging mode
			break;
		case POWER_SUPPLY_TYPE_USB:
			wakeup_timer_seconds = 0;	// disable wakeup. The phone will be wakeup because of wakelock
			break;
		case POWER_SUPPLY_TYPE_BATTERY:
		default:
			wakeup_timer_seconds = 0;	// disable wakeup.
			if(di->battery_capacity <= 1)	// enable wakeup for monitoring power-off voltage
				wakeup_timer_seconds = 60;
			break;
	}
	
	// Set Charging Status
	if(di->charge_rsoc == POWER_SUPPLY_TYPE_UPS ||		// JIG
	   di->charge_rsoc == POWER_SUPPLY_TYPE_BATTERY) {	// Battery
		di->charge_status = POWER_SUPPLY_STATUS_DISCHARGING;
	} else {
		if(di->battery_present == 0) {			// Battery not present. Display as not charging
			di->charge_status = POWER_SUPPLY_STATUS_NOT_CHARGING;		
		} else if(di->temp_C < TEMP_CRITICAL_LOWER ||
			  di->temp_C > TEMP_CRITICAL_UPPER) {	// Charging Stoped
			di->charge_status = POWER_SUPPLY_STATUS_NOT_CHARGING;
		} else if(di->temp_C < TEMP_LIMIT_LOWER ||
			  di->temp_C > TEMP_LIMIT_UPPER) {	// Charging Limited
			di->charge_status = POWER_SUPPLY_STATUS_CHARGING;
		} else if((start_monitor || end_of_charge) &&
			  di->battery_capacity > 97) {		// Charging FULL
			di->charge_status = POWER_SUPPLY_STATUS_FULL;
		} else {					// Normal Charging
			di->charge_status = POWER_SUPPLY_STATUS_CHARGING;
		}
	}
}

static int check_battery_changed(struct twl4030_bci_device_info *di)
{
	static int battery_calibration_enabled = 0;
	int need_recalibration = 0;
	int upper_limit, lower_limit;
	
	if(battery_calibration_enabled) {	// Voltage Calibration Complete
		start_monitor = 0;
		set_end_of_charge(0);
		set_charging_timer(0);
		di->charge_status = POWER_SUPPLY_STATUS_CHARGING;
		battery_calibration_enabled = 0;
		printk(KERN_INFO "[BATTERY] ReCalibration Done!\n");
		return 2;
	}

	if(di->charge_rsoc == POWER_SUPPLY_TYPE_BATTERY)
		return 0;
	if(di->battery_present == 0)			// Cannot do anything without battery
		return 0;
	if(di->temp_C < 0 || di->temp_C > 60)	// Ignore when low or high temperature
		return 0;

	// Set Voltage Limit
	// TODO : 300 and -250 is tuning value. must be verified.
	upper_limit = 300;
	lower_limit = -250;

	// Check battery changed while phone turned on
	if(di->battery_present  == 1 &&
	   di->previous_battery_present == 0) {		// Battery Inserted
		need_recalibration = 1;

/* LGE_CHANGE_S [LS855:bking.moon@lge.com] 2011-02-27, */ 
#ifdef CONFIG_LGE_LAB3_BOARD
		if( check_battery_present() ) {
			extern void setting_for_factory_mode_hw_req(void);
			setting_for_factory_mode_hw_req();
		}
#endif 
/* LGE_CHANGE_E [LS855:bking.moon@lge.com] 2011-02-27 */

	} else {
		int voltage_diff;
		voltage_diff = di->voltage_uV - di->previous_voltage_uV;
		if(voltage_diff > upper_limit) {			// too much voltage increase
			if(di->previous_charge_rsoc != POWER_SUPPLY_TYPE_BATTERY &&
			   di->pre_pre_charge_rsoc != POWER_SUPPLY_TYPE_BATTERY) {
			   	need_recalibration = 1;
			}
		} else if(voltage_diff < lower_limit) {	// too much voltage drop
			need_recalibration = 1;
		}
	}

	if(need_recalibration) {
		charging_ic_deactive();
		battery_calibration_enabled = 1;
		di->charge_status = POWER_SUPPLY_STATUS_UNKNOWN;
		printk(KERN_INFO "[BATTERY] Need To Calibration!\n");
		printk(KERN_INFO "[BATTERY] %dmV -> %dmV\n", di->previous_voltage_uV, di->voltage_uV);
		mdelay(50);				// wait for voltage drop - charging ic deactived.
		max17043_do_calibrate();		// Fuel Gauge Recalibration takes 1~3 sec.
	}
	return need_recalibration;
}

#ifdef CONFIG_LGE_CHARGE_CONTROL_BATTERY_FET
extern void lge_battery_fet_onoff(int on); 
#endif 

static int set_battery_charging(struct twl4030_bci_device_info *di)
{
	int trickle_chg_max, trickle_chg_min, trickle_chg_timer_start;
	max8922_status chr_ic_status = get_charging_ic_status();
	
	if(di->battery_present == 0 || 						// No Battery State
		di->charge_rsoc == POWER_SUPPLY_TYPE_BATTERY) {	// No Charging Source
		start_monitor = 0;
		set_end_of_charge(0);
		set_charging_timer(0);
		return 0;
	}
	
	/*
	 * Recharging algorithm
	 *  - High Temperature : up to 4.0V
	 *  - Normal : up to 4.2V
	 */
	// Set maximum charging voltage
	if(di->temp_C < TEMP_CRITICAL_LOWER ||
	   di->temp_C > TEMP_CRITICAL_UPPER) {		// Critical Temperature! Must stop charging
		start_monitor = 0;
		set_end_of_charge(0);
		set_charging_timer(0);
		if (di->battery_present == 1) {
			printk(KERN_INFO "[charging_msg] %s: Temprature Critical Charger Off %d\n", __func__, di->temp_C);
#ifdef CONFIG_LGE_CHARGE_CONTROL_BATTERY_FET
			lge_battery_fet_onoff(0);
#else 
			charging_ic_deactive();
#endif 
		}
		return 0;
	} else if(di->temp_C < TEMP_LIMIT_LOWER ||
		  di->temp_C > TEMP_LIMIT_UPPER) {	// Charging Limit
		trickle_chg_max = TRICKLE_LIMMIT_ALERT_CHG_MAX;
		trickle_chg_timer_start = TRICKLE_LIMMIT_ALERT_CHG_TIMER_START;
		trickle_chg_min = TRICKLE_LIMMIT_ALERT_CHG_MIN;
	} else {									// Normal Charging
		trickle_chg_max = TRICKLE_CHG_MAX;					// to unintentional charging stop
		trickle_chg_timer_start = TRICKLE_CHG_TIMER_START;
		trickle_chg_min = TRICKLE_CHG_MIN;
		if(di->previous_temp_C < TEMP_LIMIT_LOWER ||
		   di->previous_temp_C > TEMP_LIMIT_UPPER) {
			start_monitor = 0;
			set_end_of_charge(0);
			set_charging_timer(0);
		}
	}

	// Deactive charger for protect overcharge & monitoring
	if(chr_ic_status != CHARGING_IC_DEACTIVE) {
		if(di->voltage_uV >= trickle_chg_max || charging_timeout() || end_of_charge) {
			// Battery Full Charged Once. Entering Trickle Charging Mode
			/* LGE_CHANGE_S, kwuiseok.kim@lge.com, 2011-02-28, */
			//if(!charging_timeout())		// Do not stop charging when timeout occured
			if(di->voltage_uV >= trickle_chg_max || end_of_charge)
			/* LGE_CHANGE_E, kwuiseok.kim@lge.com, 2011-02-28, */
			{
				printk(KERN_INFO "[charging_msg] %s: End of Chargeing.. Charger Off di->voltage_uv %d, end_of_charge %d", __func__, di->voltage_uV, end_of_charge );
#ifdef CONFIG_LGE_CHARGE_CONTROL_BATTERY_FET
				lge_battery_fet_onoff(0);
#else 
				charging_ic_deactive();	// It's not actually charged full.
#endif 
			}
			set_charging_timer(0);
			set_end_of_charge(0);
			if(start_monitor == 0) {
				printk(KERN_DEBUG "[Battery] Trickle Charging Start!\n");
				start_monitor = 1;
			}
		} else if(di->voltage_uV >= trickle_chg_timer_start) {
			set_charging_timer(1);
		}
	} else if(di->voltage_uV < trickle_chg_min) {	// active charger for recharging
		printk(KERN_INFO "[charging_msg] %s; Recharging.. %d\n", __func__, di->voltage_uV);
#ifdef CONFIG_LGE_CHARGE_CONTROL_BATTERY_FET
		lge_battery_fet_onoff(1);
#else 
		switch(di->charge_rsoc) {
			case POWER_SUPPLY_TYPE_MAINS:
				charging_ic_set_ta_mode();
				break;
			case POWER_SUPPLY_TYPE_USB:
				charging_ic_set_usb_mode();
				break;
			case POWER_SUPPLY_TYPE_UPS:
				charging_ic_set_factory_mode();
				break;
			default:
				break;
		}
#endif 
	}
	return 0;
}

static void backup_battery_info(struct twl4030_bci_device_info *di)
{
	di->pre_pre_charge_rsoc = di->previous_charge_rsoc;
	di->previous_temp_C = di->temp_C;
	di->previous_voltage_uV = di->voltage_uV;
	di->previous_battery_capacity = di->battery_capacity;
	di->previous_charge_status = di->charge_status;
	di->previous_charge_rsoc = di->charge_rsoc;
	di->previous_battery_present = di->battery_present;
}

static void twl4030_bci_battery_update_status(struct twl4030_bci_device_info *di)
{
#if defined(CONFIG_MACH_LGE_HUB) || defined(CONFIG_MACH_LGE_SNIPER)
#else
	int old_charge_source = di->charge_rsoc;
	int old_charge_status = di->charge_status;
	int old_capacity = di->capacity;
	static int stable_count;
#endif

	twl4030_bci_battery_read_status(di);
	di->charge_status = POWER_SUPPLY_STATUS_UNKNOWN;
	if (power_supply_am_i_supplied(&di->bat))
		di->charge_status = POWER_SUPPLY_STATUS_CHARGING;
	else
		di->charge_status = POWER_SUPPLY_STATUS_DISCHARGING;

#if defined(CONFIG_MACH_LGE_HUB)  || defined(CONFIG_MACH_LGE_SNIPER)
	set_battery_status(di);
	// Recheck Battery Status and find errors
	if(di->charge_status == POWER_SUPPLY_STATUS_FULL &&
	   di->voltage_uV < 4050) {
		// TODO : recheck MUIC stat for I2C Fail
	}

	switch(check_battery_changed(di)) {
		case 2:		// calibration done.
			set_battery_charging(di);
			di->voltage_uV = di->previous_voltage_uV;		// display voltage before charging ic deactived
			break;
		case 1:		// calbration start
			backup_battery_info(di);
			break;
		default:	// normal state
			set_battery_charging(di);
			backup_battery_info(di);
			break;
	}
	power_supply_changed(&di->bat);
#else
	/*
	 * Since Charger interrupt only happens for AC plug-in
	 * and not for usb plug-in, we use the next update
	 * cycle to update the status of the power_supply
	 * change to user space.
	 * Read the current usb_charger_flag
	 * compare this with the value from the last
	 * update cycle to determine if there was a
	 * change.
	 */

	di->charge_rsoc = usb_charger_flag;

	/*
	 * Battery voltage fluctuates, when we are on a threshold
	 * level, we do not want to keep informing user of the
	 * capacity fluctuations, or he the battery progress will
	 * keep moving.
	 */
	if (old_capacity != di->capacity)
		stable_count = 0;
	else
		stable_count++;
	/*
	 * Send uevent to user space to notify
	 * of some battery specific events.
	 * Ac plugged in, USB plugged in and Capacity
	 * level changed.
	 * called every 100 jiffies = 0.7 seconds
	 * 20 stable cycles means capacity did not change
	 * in the last 15 seconds.
	 */
	if ((old_charge_status != di->charge_status)
			|| (stable_count == 20)
			|| (old_charge_source !=  di->charge_rsoc)) {
		power_supply_changed(&di->bat);
	}


#endif
}

#if defined(CONFIG_MACH_LGE_HUB)	/* LGE_CHANGE [HUB: newcomet@lge.com] on 2009-10-12, for <HAL:battery info. path> */
void charger_state_update_by_other(void)
{
	if(refer_di == NULL)
		return;

	// Update Battery Information
	cancel_delayed_work_sync(&refer_di->twl4030_bci_monitor_work);
	twl4030_bci_battery_update_status(refer_di);
#ifdef FUELGAUGE_AP_ONLY
		// do nothing
#else
	if(system_rev >= 4) {
		// do nothing
	} else {
		schedule_delayed_work(&refer_di->twl4030_bci_monitor_work, MONITOR_WORK_TIME);
	}
#endif
}
EXPORT_SYMBOL(charger_state_update_by_other);
#endif

static void twl4030_bci_battery_work(struct work_struct *work)
{
	struct twl4030_bci_device_info *di = container_of(work,
		struct twl4030_bci_device_info, twl4030_bci_monitor_work.work);

	twl4030_bci_battery_update_status(di);
#if defined(CONFIG_MACH_LGE_HUB) || defined(CONFIG_MACH_LGE_SNIPER)
#ifdef FUELGAUGE_AP_ONLY
		// do nothing
#else
	if(system_rev >= 4) {
		// do nothing
	} else {
		schedule_delayed_work(&di->twl4030_bci_monitor_work, MONITOR_WORK_TIME); // 10s
	}
#endif
#else
	schedule_delayed_work(&di->twl4030_bci_monitor_work, 100);
#endif
}


#define to_twl4030_bci_device_info(x) container_of((x), \
			struct twl4030_bci_device_info, bat);

#if defined(CONFIG_MACH_LGE_HUB) || defined(CONFIG_MACH_LGE_SNIPER)
#else
static void twl4030_bci_battery_external_power_changed(struct power_supply *psy)
{
	struct twl4030_bci_device_info *di = to_twl4030_bci_device_info(psy);

	cancel_delayed_work(&di->twl4030_bci_monitor_work);
	schedule_delayed_work(&di->twl4030_bci_monitor_work, 0);
}
#endif

#define to_twl4030_bk_bci_device_info(x) container_of((x), \
		struct twl4030_bci_device_info, bk_bat);

#if BK_BATT
static int twl4030_bk_bci_battery_get_property(struct power_supply *psy,
					enum power_supply_property psp,
					union power_supply_propval *val)
{
	struct twl4030_bci_device_info *di = to_twl4030_bk_bci_device_info(psy);

	switch (psp) {
	case POWER_SUPPLY_PROP_VOLTAGE_NOW:
		val->intval = di->bk_voltage_uV;
		break;
	default:
		return -EINVAL;
	}

	return 0;
}
#endif

#if defined(CONFIG_MACH_LGE_HUB) || defined(CONFIG_MACH_LGE_SNIPER)
#else
static int twl4030_usb_battery_get_property(struct power_supply *psy,
					enum power_supply_property psp,
					union power_supply_propval *val)
{
	switch (psp) {
	case POWER_SUPPLY_PROP_ONLINE:
		val->intval = usb_charger_flag;
		break;
	default:
		return -EINVAL;
	}

	return 0;
}
#endif

static int twl4030_bci_battery_get_property(struct power_supply *psy,
					enum power_supply_property psp,
					union power_supply_propval *val)
{
	struct twl4030_bci_device_info *di;
	// int status = 0; // 20120213 taeju.park@lge.com To delete compile warning, unused variable.

	/* LGE_CHANGE_S [sookyoung.kim@lge.com] 2010-03-19 */
#if defined(CONFIG_HUB_MUIC) && !defined(CONFIG_LGE_OMAP3_EXT_PWR)
	TYPE_MUIC_MODE muic_mode;
#endif
	/* LGE_CHANGE_E [sookyoung.kim@lge.com] 2010-03-19 */

	di = to_twl4030_bci_device_info(psy);
/*
	switch (psp) {
		case POWER_SUPPLY_PROP_STATUS:
			val->intval = di->charge_status;
			return 0;
		default:
			break;
	}
*/
	switch (psp) {
		case POWER_SUPPLY_PROP_STATUS:
			val->intval = di->charge_status;
			if(di->voltage_uV > FORCE_FULL_CHARGE_VOLTAGE_LEVEL) {		// QM Requested for Dummy Battery Test (LGP970)
				val->intval = POWER_SUPPLY_STATUS_FULL;
			}
			break;
		case POWER_SUPPLY_PROP_VOLTAGE_NOW:
			val->intval = di->voltage_uV;
			break;
		case POWER_SUPPLY_PROP_CURRENT_NOW:
			val->intval = di->current_uA;
			break;
		case POWER_SUPPLY_PROP_TEMP:
			if(di->battery_present == 0)		// No Battery or Dummy Battery
				val->intval = 200;
			else
	                val->intval = di->temp_C - 40;	// -40 is LGP970 only value
                        //val->intval = di->temp_C;
			break;
		case POWER_SUPPLY_PROP_ONLINE:
#if defined(CONFIG_HUB_MUIC) || defined(CONFIG_LGE_OMAP3_EXT_PWR)
			val->intval = di->charge_rsoc;
			break;
#else
			status = twl4030bci_status();
			if ((status & AC_STATEC) == AC_STATEC)
				val->intval = POWER_SUPPLY_TYPE_MAINS;
			else if (usb_charger_flag)
				val->intval = POWER_SUPPLY_TYPE_USB;
			else
				val->intval = 0;
			break;
#endif
		case POWER_SUPPLY_PROP_PRESENT:
			val->intval = di->battery_present;
#if defined(CONFIG_HUB_MUIC) && !defined(CONFIG_LGE_OMAP3_EXT_PWR)
			muic_mode = get_muic_mode();
			if(muic_mode == MUIC_CP_USB  ||		// JIG
			   muic_mode == MUIC_CP_UART ||
			   muic_mode == MUIC_AP_UART) {
				val->intval = 1;
			} else if(muic_mode == MUIC_NONE) {	// Dummy Battery
				val->intval = 1;
			}
#endif

			break;
		case POWER_SUPPLY_PROP_CAPACITY:
#if defined(CONFIG_MACH_LGE_HUB) || defined(CONFIG_MACH_LGE_SNIPER)
			val->intval = di->battery_capacity;
			// for Lockscreen sync with status bar
			if(di->charge_rsoc == POWER_SUPPLY_TYPE_MAINS ||
			   di->charge_rsoc == POWER_SUPPLY_TYPE_USB) {	// When Charging Mode
				if(di->charge_status == POWER_SUPPLY_STATUS_FULL) {
					val->intval = 100;
				} else if(di->battery_present == 1 &&	// Battery Present and not Full
					  val->intval == 100) {		// but battery level is 100%
					val->intval = 99;		// view as 99% for lockscreen
				} else if(di->voltage_uV > FORCE_FULL_CHARGE_VOLTAGE_LEVEL) {	// QM Requested for Dummy Battery Test (LGP970)
					val->intval = 100;
				}
			}
#else //CONFIG_MACH_LGE_HUB
			val->intval = twl4030battery_capacity(di);
#endif //CONFIG_MACH_LGE_HUB
			break;
#if defined(CONFIG_MACH_LGE_HUB)	/* LGE_CHANGE [HUB: newcomet@lge.com] on 2009-9-23, for <HAL:battery info. path> */
			/* FIXME : It depends on H/W specific */
		case POWER_SUPPLY_PROP_HEALTH:
			if(!di->battery_present)
				val->intval = POWER_SUPPLY_HEALTH_UNKNOWN;
			else if(di->voltage_uV > 5000)			// Over voltage
				val->intval = POWER_SUPPLY_HEALTH_OVERVOLTAGE;
			else if(di->temp_C < TEMP_CRITICAL_LOWER)	// Cold
				val->intval = POWER_SUPPLY_HEALTH_COLD;
			else if(di->temp_C > TEMP_CRITICAL_UPPER)	// Hot
				val->intval = POWER_SUPPLY_HEALTH_OVERHEAT;
			else
				val->intval = POWER_SUPPLY_HEALTH_GOOD;
			break;
		case POWER_SUPPLY_PROP_TECHNOLOGY:
			val->intval = POWER_SUPPLY_TECHNOLOGY_LION;	// Fixed value : Li-ion
			break;
#endif

		default:
			return -EINVAL;
	}
	return 0;
}

#if defined(CONFIG_MACH_LGE_HUB)	/* LGE_CHANGE [HUB: newcomet@lge.com] on 2009-9-29, for <HAL:battery info. path> */
#define to_twl4030_ac_bci_device_info(x) container_of((x), \
		struct twl4030_bci_device_info, ac);

#define to_twl4030_usb_bci_device_info(x) container_of((x), \
		struct twl4030_bci_device_info, usb);

static int twl4030_ac_bci_battery_get_property(struct power_supply *psy,
					enum power_supply_property psp,
					union power_supply_propval *val)
{
/* LGE_CHANGE_S [sookyoung.kim@lge.com] 2010-03-19 */
#if defined(CONFIG_HUB_MUIC) && !defined(CONFIG_LGE_OMAP3_EXT_PWR)
		TYPE_MUIC_MODE muic_mode = MUIC_NONE;
#endif
/* LGE_CHANGE_E [sookyoung.kim@lge.com] 2010-03-19 */

	switch (psp) {
		case POWER_SUPPLY_PROP_ONLINE:
#if defined(CONFIG_HUB_MUIC)
/* LGE_CHANGE_S [sookyoung.kim@lge.com] 2010-03-19 */
			muic_mode = get_muic_mode();
			if ( muic_mode == MUIC_LG_TA || muic_mode == MUIC_NA_TA || muic_mode == MUIC_HCHH || muic_mode == MUIC_INVALID_CHG )
				val->intval = 1;
			else
				val->intval = 0;
			break;
/* LGE_CHANGE_E [sookyoung.kim@lge.com] 2010-03-19 */
#elif defined(CONFIG_LGE_OMAP3_EXT_PWR)
			if( TA_CABLE == get_ext_pwr_type() ) {
				val->intval = 1;
			} else {
				val->intval = 0;
			}
			break;
#else
#error
#endif 
		default:
			return -EINVAL;
	}

	return 0;
}

static int twl4030_usb_bci_battery_get_property(struct power_supply *psy,
					enum power_supply_property psp,
					union power_supply_propval *val)
{
/* LGE_CHANGE_S [sookyoung.kim@lge.com] 2010-03-19 */
#if defined(CONFIG_HUB_MUIC) && !defined(CONFIG_LGE_OMAP3_EXT_PWR)
		TYPE_MUIC_MODE muic_mode = MUIC_NONE;
#endif
/* LGE_CHANGE_E [sookyoung.kim@lge.com] 2010-03-19 */

	switch (psp) {
		case POWER_SUPPLY_PROP_ONLINE:
#if defined(CONFIG_HUB_MUIC)
/* LGE_CHANGE_S [sookyoung.kim@lge.com] 2010-03-19 */
			muic_mode = get_muic_mode();
			if ( muic_mode == MUIC_AP_USB)
				val->intval = 1;
			else
				val->intval = 0;
			break;
/* LGE_CHANGE_E [sookyoung.kim@lge.com] 2010-03-19 */
#elif defined(CONFIG_LGE_OMAP3_EXT_PWR)
			if( USB_CABLE == get_ext_pwr_type() ) {
				val->intval = 1;
			} else {
				val->intval = 0;
			}
			break;
#else
#error
#endif 
		default:
			return -EINVAL;
	}

	return 0;
}
#endif


static char *twl4030_bci_supplied_to[] = {
#if defined(CONFIG_MACH_LGE_HUB)	/* LGE_CHANGE [HUB: newcomet@lge.com] on 2009-9-22, for <HAL:battery info. path> */
	"battery",
	"ac",
	"usb",
#else
	"twl4030_bci_battery",
#endif
};

static int __devinit twl4030_bci_battery_probe(struct platform_device *pdev)
{
	struct twl4030_bci_platform_data *pdata = pdev->dev.platform_data;
	struct twl4030_bci_device_info *di;
	int irq;
	int ret;

	therm_tbl = pdata->battery_tmp_tbl;

	di = kzalloc(sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	di->dev = &pdev->dev;
#if defined(CONFIG_MACH_LGE_HUB)	/* LGE_CHANGE [HUB: newcomet@lge.com] on 2009-9-22, for <HAL:battery info. path> */
	/* refer - com_android_server_BatteryService.cpp */
	di->bat.name = "battery";
#else
	di->bat.name = "twl4030_bci_battery";
#endif
	di->bat.supplied_to = twl4030_bci_supplied_to;
	di->bat.num_supplicants = ARRAY_SIZE(twl4030_bci_supplied_to);
	di->bat.type = POWER_SUPPLY_TYPE_BATTERY;
	di->bat.properties = twl4030_bci_battery_props;
	di->bat.num_properties = ARRAY_SIZE(twl4030_bci_battery_props);
	di->bat.get_property = twl4030_bci_battery_get_property;
#if defined(CONFIG_MACH_LGE_HUB) || defined(CONFIG_MACH_LGE_SNIPER)
	di->bat.external_power_changed = NULL;
#else
	di->bat.external_power_changed = twl4030_bci_battery_external_power_changed;
#endif

	di->charge_status = POWER_SUPPLY_STATUS_UNKNOWN;
	di->bat.set_charged = NULL;

/* LGE_CHANGE_S [taehwan.kim@lge.com] 2010-3-12, android NOT need bk battery voltage*/
#if BK_BATT
	di->bk_bat.name = "twl4030_bci_bk_battery";
	di->bk_bat.type = POWER_SUPPLY_TYPE_BATTERY;
	di->bk_bat.properties = twl4030_bk_bci_battery_props;
	di->bk_bat.num_properties = ARRAY_SIZE(twl4030_bk_bci_battery_props);
	di->bk_bat.get_property = twl4030_bk_bci_battery_get_property;
	di->bk_bat.external_power_changed = NULL;
	di->bk_bat.set_charged = NULL;
#endif //BK_BATT
/* LGE_CHANGE_E [taehwan.kim@lge.com] 2010-3-12, android NOT need bk battery voltage*/


#if defined(CONFIG_MACH_LGE_HUB)	/* LGE_CHANGE [HUB: newcomet@lge.com] on 2009-9-29, for <HAL:battery info. path> */
	di->ac.name = "ac";
	di->ac.type = POWER_SUPPLY_TYPE_MAINS; //LGE_CHANGE [antispoon@lge.com] 2010-01-12
	di->ac.properties = twl4030_ac_usb_bci_battery_props;
	di->ac.num_properties = ARRAY_SIZE(twl4030_ac_usb_bci_battery_props);
	di->ac.get_property = twl4030_ac_bci_battery_get_property;
	di->ac.external_power_changed = NULL;
	di->ac.set_charged = NULL;

	di->usb.name = "usb";
	di->usb.type = POWER_SUPPLY_TYPE_USB; //LGE_CHANGE [antispoon@lge.com] 2010-01-12
	di->usb.properties = twl4030_ac_usb_bci_battery_props;
	di->usb.num_properties = ARRAY_SIZE(twl4030_ac_usb_bci_battery_props);
	di->usb.get_property = twl4030_usb_bci_battery_get_property;
	di->usb.external_power_changed = NULL;
	di->usb.set_charged = NULL;
	
#else

	/*
	 * Android expects a battery type POWER_SUPPLY_TYPE_USB
	 * as a usb charger battery. This battery
	 * and its "online" property are used to determine if the
	 * usb cable is plugged in or not.
	 */
	di->usb_bat.name = "twl4030_bci_usb_src";
	di->usb_bat.supplied_to = twl4030_bci_supplied_to;
	di->usb_bat.type = POWER_SUPPLY_TYPE_USB;
	di->usb_bat.properties = twl4030_usb_battery_props;
	di->usb_bat.num_properties = ARRAY_SIZE(twl4030_usb_battery_props);
	di->usb_bat.get_property = twl4030_usb_battery_get_property;
	di->usb_bat.external_power_changed = NULL;
#endif
#if defined(CONFIG_MACH_LGE_HUB)	/* LGE_CHANGE [HUB: newcomet@lge.com] on 2009-11-17, for <Charger int. from MUIC> */
	twl4030charger_ac_en(DISABLE);
	twl4030charger_usb_en(DISABLE);
	twl4030battery_hw_level_en(DISABLE);
	twl4030battery_hw_presence_en(DISABLE);
#else
	twl4030charger_ac_en(ENABLE);
	twl4030charger_usb_en(ENABLE);
	twl4030battery_hw_level_en(ENABLE);
	twl4030battery_hw_presence_en(ENABLE);
#endif /* CONFIG_MACH_LGE_HUB */
	platform_set_drvdata(pdev, di);

#if defined(CONFIG_MACH_LGE_HUB) || defined(CONFIG_MACH_LGE_SNIPER)
#else
/* LGE_CHANGE [HUB: newcomet@lge.com] on 2009-11-17, for <battery temp.> */
/* HUB use ADC2 port for battery temperature measurement */
	/* settings for temperature sensing */
	ret = twl4030battery_temp_setup();
	if (ret)
		goto temp_setup_fail;
#endif
	/* enabling GPCH09 for read back battery voltage */
	ret = twl4030backupbatt_voltage_setup();
	if (ret)
		goto voltage_setup_fail;

	/* REVISIT do we need to request both IRQs ?? */

	/* request BCI interruption */
	irq = platform_get_irq(pdev, 1);
	ret = request_irq(irq, twl4030battery_interrupt,
		0, pdev->name, NULL);
	if (ret) {
		dev_dbg(&pdev->dev, "could not request irq %d, status %d\n",
			irq, ret);
		goto batt_irq_fail;
	}

	/* request Power interruption */
	irq = platform_get_irq(pdev, 0);
	ret = request_irq(irq, twl4030charger_interrupt,
		0, pdev->name, di);

	if (ret) {
		dev_dbg(&pdev->dev, "could not request irq %d, status %d\n",
			irq, ret);
		goto chg_irq_fail;
	}

	ret = power_supply_register(&pdev->dev, &di->bat);
	if (ret) {
		dev_dbg(&pdev->dev, "failed to register main battery\n");
		goto batt_failed;
	}

	INIT_DELAYED_WORK_DEFERRABLE(&di->twl4030_bci_monitor_work,
				twl4030_bci_battery_work);
#ifdef FUELGAUGE_AP_ONLY
		// do nothing
#else
	if(system_rev >= 4) {
		// do nothing
	} else {
		schedule_delayed_work(&di->twl4030_bci_monitor_work, 5 * HZ); //taehwan.kim@lge.com add delay for secure ops
	}
#endif

/* LGE_CHANGE_S [taehwan.kim@lge.com] 2010-3-12, android NOT need bk battery voltage*/
#if BK_BATT
	ret = power_supply_register(&pdev->dev, &di->bk_bat);
	if (ret) {
		dev_dbg(&pdev->dev, "failed to register backup battery\n");
		goto bk_batt_failed;
	}

	INIT_DELAYED_WORK_DEFERRABLE(&di->twl4030_bk_bci_monitor_work,
				twl4030_bk_bci_battery_work);
	schedule_delayed_work(&di->twl4030_bk_bci_monitor_work, HZ*1);
#endif //BK_BATT
/* LGE_CHANGE_E [taehwan.kim@lge.com] 2010-3-12, android NOT need bk battery voltage*/

#if defined(CONFIG_MACH_LGE_HUB)	/* LGE_CHANGE [HUB: newcomet@lge.com] on 2009-9-29, for <HAL:battery info. path> */
	ret = power_supply_register(&pdev->dev, &di->ac);
	if (ret) {
		dev_dbg(&pdev->dev, "failed to register battery ac online\n");
		goto ac_online_failed;
	}

	ret = power_supply_register(&pdev->dev, &di->usb);
	if (ret) {
		dev_dbg(&pdev->dev, "failed to register battery usb online\n");
		goto usb_online_failed;
	}
#else
	ret = power_supply_register(&pdev->dev, &di->usb_bat);
	if (ret) {
		dev_dbg(&pdev->dev, "failed to register usb battery\n");
		goto usb_batt_failed;
	}
#endif
	ret = device_create_file(&pdev->dev, &dev_attr_pif);
	if (ret) {
		printk( "PIF detection register failed: Fail\n");
		return ret;
	}

	ret = device_create_file(&pdev->dev, &dev_attr_gauge_if);
	if (ret) {
		printk( "chager off sysfs register failed: Fail\n");
		return ret;
	}

	// Update First Battery Information
	refer_di = di;
	twl4030_bci_battery_read_status(di);
	set_battery_status(di);	
	
	backup_battery_info(di);
	di->bk_voltage_uV = 3700;

	di->nb.notifier_call = twl4030battery_charger_event;
	ret = otg_register_notifier(otg_get_transceiver(), &di->nb);
	if (ret) {
		dev_dbg(&pdev->dev, "failed to register usb battery\n");
		goto otg_notify_failed;
	}

	return 0;

otg_notify_failed:
	power_supply_unregister(&di->usb_bat);
#if defined(CONFIG_MACH_LGE_HUB)	/* LGE_CHANGE [HUB: newcomet@lge.com] on 2009-9-29, for <HAL:battery info. path> */
usb_online_failed:
	power_supply_unregister(&di->bat);
ac_online_failed:
	power_supply_unregister(&di->bat);
#else
usb_batt_failed:
	power_supply_unregister(&di->bk_bat);
#endif
#if BK_BATT
bk_batt_failed:
#endif
	power_supply_unregister(&di->bat);
batt_failed:
	free_irq(irq, di);
chg_irq_fail:
	irq = platform_get_irq(pdev, 1);
	free_irq(irq, NULL);
batt_irq_fail:
voltage_setup_fail:
#if !defined(CONFIG_MACH_LGE_HUB)
temp_setup_fail:
#endif
	twl4030charger_ac_en(DISABLE);
	twl4030charger_usb_en(DISABLE);
	twl4030battery_hw_level_en(DISABLE);
	twl4030battery_hw_presence_en(DISABLE);
	kfree(di);

	return ret;
}

static int __devexit twl4030_bci_battery_remove(struct platform_device *pdev)
{
	struct twl4030_bci_device_info *di = platform_get_drvdata(pdev);
	int irq;

	otg_unregister_notifier(otg_get_transceiver(), &di->nb);
	twl4030charger_ac_en(DISABLE);
	twl4030charger_usb_en(DISABLE);
	twl4030battery_hw_level_en(DISABLE);
	twl4030battery_hw_presence_en(DISABLE);

	irq = platform_get_irq(pdev, 0);
	free_irq(irq, di);

	irq = platform_get_irq(pdev, 1);
	free_irq(irq, NULL);

	flush_scheduled_work();
	power_supply_unregister(&di->bat);
/* LGE_CHANGE_S [taehwan.kim@lge.com] 2010-3-12, android NOT need bk battery voltage*/
#if BK_BATT
	power_supply_unregister(&di->bk_bat);
#endif //BK_BATT
/* LGE_CHANGE_E [taehwan.kim@lge.com] 2010-3-12, android NOT need bk battery voltage*/
	platform_set_drvdata(pdev, NULL);
	kfree(di);

	return 0;
}

#ifdef CONFIG_PM
static int twl4030_bci_battery_suspend(struct platform_device *pdev,
	pm_message_t state)
{
#if !defined(FUELGAUGE_AP_ONLY)  || BK_BATT
	struct twl4030_bci_device_info *di = platform_get_drvdata(pdev);
#endif

	//di->charge_status = POWER_SUPPLY_STATUS_UNKNOWN;  //20101116 taehwan.kim@lge.com
#ifdef FUELGAUGE_AP_ONLY
		// do nothing
#else
	if(system_rev >= 4) {
		// do nothing
	} else {
		cancel_delayed_work(&di->twl4030_bci_monitor_work);
	}
#endif
#if BK_BATT
	cancel_delayed_work(&di->twl4030_bk_bci_monitor_work);
#endif
	return 0;
}

static int twl4030_bci_battery_resume(struct platform_device *pdev)
{
#if !defined(FUELGAUGE_AP_ONLY)  || BK_BATT
	struct twl4030_bci_device_info *di = platform_get_drvdata(pdev);
#endif

#ifdef FUELGAUGE_AP_ONLY
		// do nothing
#else
	if(system_rev >= 4) {
		// do nothing
	} else {
		twl4030_bci_battery_update_status(di);
		schedule_delayed_work(&di->twl4030_bci_monitor_work, 10 * HZ);
	}
#endif

#if BK_BATT
	schedule_delayed_work(&di->twl4030_bk_bci_monitor_work, msecs_to_jiffies(400)); //20101109 taehwan.kim@lge.com 0->600ms
#endif
	return 0;
}
#else
#define twl4030_bci_battery_suspend	NULL
#define twl4030_bci_battery_resume	NULL
#endif /* CONFIG_PM */

static struct platform_driver twl4030_bci_battery_driver = {
	.probe		= twl4030_bci_battery_probe,
	.remove		= __devexit_p(twl4030_bci_battery_remove),
	.suspend	= twl4030_bci_battery_suspend,
	.resume		= twl4030_bci_battery_resume,
	.driver		= {
		.name	= "twl4030_bci",
		.owner  = THIS_MODULE,
	},
};

MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:twl4030_bci");
MODULE_AUTHOR("Texas Instruments Inc");

static int __init twl4030_battery_init(void)
{
	return platform_driver_register(&twl4030_bci_battery_driver);
}
module_init(twl4030_battery_init);

static void __exit twl4030_battery_exit(void)
{
	platform_driver_unregister(&twl4030_bci_battery_driver);
}
module_exit(twl4030_battery_exit);