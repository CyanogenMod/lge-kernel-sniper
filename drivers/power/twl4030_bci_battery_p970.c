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
#include <linux/platform_device.h>
#include <linux/i2c/twl.h>
#include <linux/power_supply.h>
#include <linux/i2c/twl4030-madc.h>
#include <linux/slab.h>
/* LGE_CHANGE_S, DCM, 2011-05-14, battery temp scnario changed */
#include <linux/i2c/twl.h>
/* LGE_CHANGE_E, DCM, 2011-05-14, battery temp scnario changed */
//LGE_CHANGE_S, DCM, 2011-04-21, TESTCODE ON/OFF
//#define TESTCODE	1
//LGE_CHANGE_E, DCM, 2011-04-21, TESTCODE ON/OFF

// 20100624 taehwan.kim@lge.com  To add Hub battery support[START_LGE]
#if defined(CONFIG_HUB_MUIC)
#include "../hub/hub_muic.h"
#endif
// kibum.lee@lge.com 20120502 MUIC re-work start	
#if defined(CONFIG_MUIC)
#include <linux/muic/muic.h>
#endif
// kibum.lee@lge.com 20120502 MUIC re-work end
#if defined(CONFIG_HUB_CHARGING_IC)
#include "../hub/hub_charging_ic.h"
#endif
#if defined(CONFIG_FUELGAUGE_MAX17043)
#include <linux/max17043_fuelgauge.h>
#endif

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
	int			battery_capacity;
	int			battery_present;
	int			previous_voltage_uV;
	int			previous_temp_C;
	int			previous_charge_status;
	int			previous_battery_capacity;
	int			previous_battery_present;
	int			previous_charge_rsoc;
	int 		justin_capacity; //2011.02.10 ntjinsu.park@lge.com justin fuel gauge
	int			temp_control; // 20120725, mannsik.chung@lge.com, Enable charging by fake mode.

	struct power_supply	ac;
	struct power_supply	usb;
// 20100624 taehwan.kim@lge.com  To add Hub battery support[END_LGE]

	struct power_supply	bat;
	struct power_supply	bk_bat;
	struct power_supply usb_bat;
	struct delayed_work	twl4030_bci_monitor_work;
	struct delayed_work	twl4030_bk_bci_monitor_work;
};
// 20100624 taehwan.kim@lge.com  To add Hub battery support[START_LGE]

// Battery Spec.

// S[, 20120725, mannsik.chung@lge.com, Enable charging by fake mode.
#define UNLIMITED_TEMP_VAL	0xA4
#define UNLIMITED_TEMP_HIGH	390
#define UNLIMITED_TEMP_LOW	-50
static int thermal_fakemode = 0;
// E], 20120725, mannsik.chung@lge.com, Enable charging by fake mode.

//ntyeongon.moon 2011-03-17 HW requirement [START]
#define TEMP_CRITICAL_UPPER	(550)     //(650)
#define TEMP_CRITICAL_LOWER	(-100)    //(-300)
#define TEMP_LIMIT_UPPER	(420)     //(600)
#define TEMP_LIMIT_LOWER	(-50)    //(-200)
//ntyeongon.moon 2011-03-17 HW requirement [END]


#define MONITOR_WORK_TIME	(10 * HZ)

extern u32 wakeup_timer_seconds;	// from pm34xx.c
extern int lcd_off_boot;			// to check factory mode
static struct twl4030_bci_device_info *refer_di = NULL;
static int start_monitor = 0;	//20101104 taehwan.kim@lge.com to fix trickle chg

//2011-03-23 ntyeongon.moon@lge.com get muic INT_STAT  VBUS [START_LGE]
int is_muic_mvbus_on();
//2011-03-23 ntyeongon.moon@lge.com get muic INT_STAT  VBUS [END_LGE]


// 20100624 taehwan.kim@lge.com  To add Hub battery support[END_LGE]

static int LVL_1, LVL_2, LVL_3, LVL_4;

static int read_bci_val(u8 reg_1);
static inline int clear_n_set(u8 mod_no, u8 clear, u8 set, u8 reg);

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
void set_charging_timer(int en)
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
	int ret, val;
#if defined(CONFIG_HUB_MUIC)
	ret = get_muic_mode();

	if(ret == MUIC_UNKNOWN) {
		val = -1;
	} else if (lcd_off_boot && !check_battery_present()) {
		val = 2;
	} else if (ret == MUIC_AP_UART) {
		val = 1;
	} else if ((ret == MUIC_CP_UART) || (ret == MUIC_CP_USB)) {
		val = 2;
	} else {
		val = 0;
	}
// kibum.lee@lge.com 20120502 MUIC re-work start	
#elif defined(CONFIG_MUIC)
	int muic_mode;
	muic_mode = muic_get_mode();

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
// kibum.lee@lge.com 20120502 MUIC re-work end	
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
	int val, batt_raw_cap;
    /* S[, 20111015, mschung@ubiquix.com, Fuelgauge workaround on displaying 1%. */
    static cpcap_toolow_cnt = 0;
    /* E], 20111015, mschung@ubiquix.com, Fuelgauge workaround on displaying 1%. */

#if 1
	val = simple_strtoul(buf, NULL, 10);
#else
	sscanf(buf, "%d", &val);
#endif

	//batt_raw_cap = ((val * 100) / 95)-1;
	batt_raw_cap = val;

	printk("[BATTERY] write AT+CBC = %d to batt_soc_storei as %d\n", val, batt_raw_cap);

	/* S[, 20111015, mschung@ubiquix.com, Fuelgauge workaround on displaying 1%. */
	if (batt_raw_cap < 3 && cpcap_toolow_cnt < 3)
	{
		cpcap_toolow_cnt ++;
	}
	else
	{
		cpcap_toolow_cnt = 0;

		refer_di->justin_capacity = refer_di->battery_capacity = batt_raw_cap;
		charger_state_update_by_other();
	}
	/* E], 20111015, mschung@ubiquix.com, Fuelgauge workaround on displaying 1%. */

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
int twl4030charger_usb_en(int enable)
{
	return 0;
}

/*
 * Return battery temperature
 * Or < 0 on failure.
 */
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

	if (temp > 720) {		// Floating BATT Temp Pin
		return 0xBA00E00;	// means No Battery or Dummy Battery
	}
	
	// 20101226 dajin.kim@lge.com Convert adc value to temperature
	temp = adc2temperature(temp);

    return temp;

}


/*
 * Return battery voltage
 * Or < 0 on failure.
 */
static int twl4030battery_voltage(void)
{
	int volt;

	if(system_rev >= 2){	// B-Project Rev.D (AP Fuel Gauge)
		return max17043_get_voltage();
	}
	else
	{
		volt = read_bci_val(T2_BATTERY_VOLT);
		return (volt * VOLT_STEP_SIZE) / VOLT_PSR_R; 
	} 
}

/*
 * Return battery capacity
 * Or < 0 on failure.
 */
static int twl4030battery_capacity(struct twl4030_bci_device_info *di)
{
	int ret = 0;

	static int vol_history[20] = {0};
	static int vol_history_idx = 0;
	int vol_normalize = 0;
	int i = 0;

	if(system_rev >= 2) {	// B-Project Rev.D (AP Fuel Gauge)
		ret = max17043_get_capacity();
	}
	else {				// Use Calculated capacity from voltage
                // LGE_CHANGE_S 20121011 subum.choi WBT(TD2170398097)
                //vol_history[vol_history_idx++] = di->voltage_uV;
                vol_history[((vol_history_idx >= 20) ? 0 : vol_history_idx++)] = di->voltage_uV;
                // LGE_CHANGE_E 20121011 subum.choi WBT(TD2170398097)

		if(vol_history_idx >= 20)
			vol_history_idx = 0;

		while(vol_history[i] != 0 && i < 20) {
			vol_normalize += vol_history[i++];
		}
		if(i != 0)
			vol_normalize = vol_normalize / i;
		ret = (vol_normalize - 3200) / 10;
	}

	if(ret < 0)
		ret = 0;
	else if(ret > 100)
		ret = 100;
	return ret;
}


static int read_bci_val(u8 reg)
{
	int ret, temp;
	u8 val = 0;

// 20100816 taehwan.kim@lge.com Set TWL4030 register for MADC voltage check [START_LGE]
    ret = clear_n_set(TWL4030_MODULE_MAIN_CHARGE, 0, USBFASTMCHG, REG_BCIMFSTS4);
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

static enum power_supply_property twl4030_bci_battery_props[] = {
	POWER_SUPPLY_PROP_STATUS,
	POWER_SUPPLY_PROP_ONLINE,
	POWER_SUPPLY_PROP_VOLTAGE_NOW,
	POWER_SUPPLY_PROP_CURRENT_NOW,
	POWER_SUPPLY_PROP_CAPACITY,
	POWER_SUPPLY_PROP_TEMP,
	POWER_SUPPLY_PROP_HEALTH,
	POWER_SUPPLY_PROP_TECHNOLOGY,
	POWER_SUPPLY_PROP_PRESENT,
	POWER_SUPPLY_PROP_TEMP_CONTROL, // 20120725, mannsik.chung@lge.com, Enable charging by fake mode.
};

static enum power_supply_property twl4030_ac_usb_bci_battery_props[] = {
	POWER_SUPPLY_PROP_ONLINE,
};

extern u32 doing_wakeup;

static void twl4030_bci_battery_read_status(struct twl4030_bci_device_info *di)
{
	max8922_status chr_ic_status = CHARGING_IC_DEACTIVE;
	chr_ic_status = get_charging_ic_status();

    TYPE_MUIC_MODE muic_mode = MUIC_NONE;
	muic_mode = muic_get_mode();
	/* Read Battery Status */
	di->temp_C = twl4030battery_temperature();			// Read Temperature

	if(system_rev >= 2 && di->battery_present) {			// Adjust RCOMP for fuelgauge(Rev.D)
		max17043_set_rcomp_by_temperature(di->temp_C);
		// TODO : max17043_update
	}
//20120221, hrgo@ubiquix.com , When is suddenly Over temperature,  retry read temperature. [START]
	if (di->temp_C >600 && di->temp_C != 0xBA00E00){
		msleep(10);
		printk("[BATTERY] When suddenly OverTemp. temp_C : %d \n", di->temp_C);
		di->temp_C = twl4030battery_temperature();
		printk("[BATTERY] When suddenly OverTemp. retry temp_C : %d \n", di->temp_C);
	}
//20120221, hrgo@ubiquix.com , When is suddenly Over temperature,  retry read temperature. [END]
	di->voltage_uV = twl4030battery_voltage();			// Read Voltage
	di->battery_capacity = twl4030battery_capacity(di);	// Read Capacity
	di->battery_present = check_battery_present();		// Set Battery Present
	printk("[BATTERY] +++ vol:%d, chr:%d, muic:%d, temp:%d, cap:%d \n",
		di->voltage_uV ,chr_ic_status ,muic_mode ,di->temp_C,di->battery_capacity );
	/* hub do not use BCI block. so we cannot measure battery current */
	di->current_uA = 0;
}

static void set_battery_status(struct twl4030_bci_device_info *di)
{

// kibum.lee@lge.com 20120502 MUIC re-work start
#if defined(CONFIG_HUB_MUIC)
	TYPE_MUIC_MODE muic_mode = get_muic_mode();
#elif defined(CONFIG_MUIC)
	TYPE_MUIC_MODE muic_mode = muic_get_mode();
#endif
// kibum.lee@lge.com 20120502 MUIC re-work end

	// Set Power Resources
	switch (muic_mode) {
		case MUIC_AP_UART:
		case MUIC_CP_UART:
		case MUIC_CP_USB:
			di->charge_rsoc = POWER_SUPPLY_TYPE_UPS;
			wakeup_timer_seconds = 0;	// disable wakeup.
			break;
		case MUIC_NA_TA:
		case MUIC_LG_TA:
		case MUIC_TA_1A:
		case MUIC_INVALID_CHG:
			di->charge_rsoc = POWER_SUPPLY_TYPE_MAINS;
			wakeup_timer_seconds = 60;	// to wake up to monitor battery when charging mode
			break;
		case MUIC_AP_USB:
			di->charge_rsoc = POWER_SUPPLY_TYPE_USB;
			wakeup_timer_seconds = 0;	// disable wakeup. The phone will be wakeup because of wakelock
			break;
		default :
			di->charge_rsoc = POWER_SUPPLY_TYPE_BATTERY;
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
		} else if((di->temp_C < TEMP_CRITICAL_LOWER ||
			  di->temp_C > TEMP_CRITICAL_UPPER) &&
			  di->temp_control != UNLIMITED_TEMP_VAL) {	// 20120725, mannsik.chung@lge.com, Enable charging by fake mode.
			// Charging Stoped
			di->charge_status = POWER_SUPPLY_STATUS_NOT_CHARGING;
		} else if((di->temp_C < TEMP_LIMIT_LOWER ||
			  di->temp_C > TEMP_LIMIT_UPPER) &&
			  di->temp_control != UNLIMITED_TEMP_VAL) {	// 20120725, mannsik.chung@lge.com, Enable charging by fake mode.
			// Charging Limited
			di->charge_status = POWER_SUPPLY_STATUS_CHARGING;
		} else if(start_monitor||di->battery_capacity >= 100) {			// Charging FULL
			di->charge_status = POWER_SUPPLY_STATUS_FULL;
		} else {					// Normal Charging
			di->charge_status = POWER_SUPPLY_STATUS_CHARGING;
		}
	}

	/*ntyeongon.moon 2011-04-01 CP USB Charging mode [START]*/
	if((di->charge_status==POWER_SUPPLY_STATUS_DISCHARGING)
		&& power_supply_am_i_supplied(&di->bat)    )
    {
		di->charge_status = POWER_SUPPLY_STATUS_CHARGING;
	}
	/*ntyeongon.moon 2011-04-01 CP USB Charging mode [END]*/

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
	if((di->temp_C < 0 || di->temp_C > 60) && di->temp_control != UNLIMITED_TEMP_VAL){ // 20120725, mannsik.chung@lge.com, Enable charging by fake mode.
		// Ignore when low or high temperature
		return 0;
	}

	// Set Voltage Limit
	// TODO : 300 and -250 is tuning value. must be verified.
	upper_limit = 300;
	lower_limit = -250;

	// Check battery changed while phone turned on
	if(di->battery_present  == 1 &&
	   di->previous_battery_present == 0) {		// Battery Inserted
		need_recalibration = 1;
	} else {
		int voltage_diff;
		voltage_diff = di->voltage_uV - di->previous_voltage_uV;
		if(voltage_diff > upper_limit) {			// too much voltage increase
			if(di->previous_charge_rsoc != POWER_SUPPLY_TYPE_BATTERY) {
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
		mdelay(50);						// wait for voltage drop - charging ic deactived.

		max17043_do_calibrate();		// Fuel Gauge Recalibration takes 1~3 sec.

	}
	return need_recalibration;
}

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
    if((di->temp_C < TEMP_CRITICAL_LOWER ||
	   di->temp_C > TEMP_CRITICAL_UPPER) && // Critical Temperature! Must stop charging
	   di->temp_control != UNLIMITED_TEMP_VAL){ // 20120725, mannsik.chung@lge.com, Enable charging by fake mode.
		start_monitor = 0;
		set_end_of_charge(0);
		set_charging_timer(0);
		if (di->battery_present == 1)
        	charging_ic_deactive();
        return 0;
    } else if(di->temp_C < TEMP_LIMIT_LOWER ||
              di->temp_C > TEMP_LIMIT_UPPER) {	// Charging Limit
		trickle_chg_max = 4000;
		trickle_chg_timer_start = 3950;
		trickle_chg_min = 3900;
	} else {									// Normal Charging
		trickle_chg_max = 4220;					// to unintentional charging stop
		trickle_chg_timer_start = 4197;
		trickle_chg_min = 4140;
		if((di->previous_temp_C < TEMP_LIMIT_LOWER ||
		   di->previous_temp_C > TEMP_LIMIT_UPPER) &&
		   di->temp_control != UNLIMITED_TEMP_VAL) { // 20120725, mannsik.chung@lge.com, Enable charging by fake mode.
			start_monitor = 0;
			set_end_of_charge(0);
			set_charging_timer(0);
		}
	}

	// Deactive charger for protect overcharge & monitoring
	if(chr_ic_status != CHARGING_IC_DEACTIVE) {
		if(/*di->voltage_uV >= trickle_chg_max ||*/ charging_timeout() || end_of_charge) {
			// Battery Full Charged Once. Entering Trickle Charging Mode
			if(!charging_timeout())		// Do not stop charging when timeout occured
				charging_ic_deactive();	// It's not actually charged full.
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
	}
	return 0;
}

static void backup_battery_info(struct twl4030_bci_device_info *di)
{
	di->previous_temp_C = di->temp_C;
    di->previous_voltage_uV = di->voltage_uV;
	di->previous_battery_capacity = di->battery_capacity;
	di->previous_charge_status = di->charge_status;
	di->previous_charge_rsoc = di->charge_rsoc;
	di->previous_battery_present = di->battery_present;
}

static void twl4030_bci_battery_update_status(struct twl4030_bci_device_info *di)
{
	twl4030_bci_battery_read_status(di);

	set_battery_status(di);
	switch(check_battery_changed(di)) {
		case 2:		// calibration done.
			set_battery_charging(di);
			di->voltage_uV = di->previous_voltage_uV;		// display voltage before charging ic deactived
			break;
		case 1:		// calbration start
			backup_battery_info(di);
			break;
		default:	// nornal state
			set_battery_charging(di);
			backup_battery_info(di);
			break;
	}
	
	power_supply_changed(&di->bat);

}

void charger_state_update_by_other(void)
{
	if(refer_di == NULL)
		return;

	// Update Battery Information
	cancel_delayed_work_sync(&refer_di->twl4030_bci_monitor_work);
	twl4030_bci_battery_update_status(refer_di);
	schedule_delayed_work(&refer_di->twl4030_bci_monitor_work, MONITOR_WORK_TIME);
}
EXPORT_SYMBOL(charger_state_update_by_other);

static void twl4030_bci_battery_work(struct work_struct *work)
{
	struct twl4030_bci_device_info *di = container_of(work,
			struct twl4030_bci_device_info, twl4030_bci_monitor_work.work);

	twl4030_bci_battery_update_status(di);
	schedule_delayed_work(&di->twl4030_bci_monitor_work, MONITOR_WORK_TIME); // 10s
}


#define to_twl4030_bci_device_info(x) container_of((x), \
			struct twl4030_bci_device_info, bat);

#define to_twl4030_bk_bci_device_info(x) container_of((x), \
		struct twl4030_bci_device_info, bk_bat);

static int twl4030_bci_battery_get_property(struct power_supply *psy,
					    enum power_supply_property psp,
					    union power_supply_propval *val)
{
	struct twl4030_bci_device_info *di;
#if defined(CONFIG_HUB_MUIC)
#else
	int status = 0;
#endif	
	max8922_status chr_ic_status = CHARGING_IC_DEACTIVE;
	chr_ic_status = get_charging_ic_status();

    TYPE_MUIC_MODE muic_mode = MUIC_NONE;


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
#if 0 //ntyeongon.moon 2011-03-16  full-charge status comes from soc level [START]
			if(di->voltage_uV > 4190) {		// QM Requested for Dummy Battery Test (LGP970)
				val->intval = POWER_SUPPLY_STATUS_FULL;
			}
#endif //ntyeongon.moon 2011-03-16  full-charge status comes from soc level [END]
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
			// S[, 20120725, mannsik.chung@lge.com, Enable charging by fake mode.
			else if (di->temp_control == UNLIMITED_TEMP_VAL) 
			{
				if (di->temp_C > UNLIMITED_TEMP_HIGH)
					val->intval = UNLIMITED_TEMP_HIGH;
				else if (di->temp_C < UNLIMITED_TEMP_LOW)
					val->intval = UNLIMITED_TEMP_LOW;
				else
					val->intval = di->temp_C;
			}
			// E], 20120725, mannsik.chung@lge.com, Enable charging by fake mode.
			else
				val->intval = di->temp_C;
			break;
		case POWER_SUPPLY_PROP_ONLINE:
#if defined(CONFIG_HUB_MUIC)  || defined(CONFIG_MUIC)

			val->intval = di->charge_rsoc;
//2011-03-23 ntyeongon.moon@lge.com get muic INT_STAT  VBUS [START_LGE]
			 if(!is_muic_mvbus_on()) val->intval = 0;
//2011-03-23 ntyeongon.moon@lge.com get muic INT_STAT  VBUSVBUS [END_LGE]
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
#endif  // CONFIG_HUB_MUIC
		case POWER_SUPPLY_PROP_PRESENT:
			val->intval = di->battery_present;
#if defined(CONFIG_HUB_MUIC)
			muic_mode = get_muic_mode();
			if(muic_mode == MUIC_CP_USB  ||		// JIG
			   muic_mode == MUIC_CP_UART ||
			   muic_mode == MUIC_AP_UART) {
				val->intval = 1;
			} else if(muic_mode == MUIC_NONE) {	// Dummy Battery
				val->intval = 1;
			}
// kibum.lee@lge.com 20120502 MUIC re-work start
#elif defined(CONFIG_MUIC)
			muic_mode = muic_get_mode();
			if(muic_mode == MUIC_CP_USB  ||		// JIG
			   muic_mode == MUIC_CP_UART ||
			   muic_mode == MUIC_AP_UART && 
			   chr_ic_status != CHARGING_IC_DEACTIVE) {
				val->intval = 1;
			} else if(muic_mode == MUIC_NONE) {	// Dummy Battery
				val->intval = 1;
			}
#else
	#error
// kibum.lee@lge.com 20120502 MUIC re-work end
#endif
            break;

		case POWER_SUPPLY_PROP_CAPACITY:
			#if defined(CONFIG_HUB_MUIC)
				muic_mode = get_muic_mode();
			#elif defined(CONFIG_MUIC)
				muic_mode = muic_get_mode();
			#endif
			val->intval = di->battery_capacity;
			printk("[BATTER] cap:%d , muic:%d \n",val->intval, muic_mode);
			// for Lockscreen sync with status bar
#if 0  //ntyeongon.moon 2011-03-16  full-charge status comes from soc level [START]
			if(di->charge_rsoc == POWER_SUPPLY_TYPE_MAINS ||
			   di->charge_rsoc == POWER_SUPPLY_TYPE_USB) {	// When Charging Mode
				if(di->charge_status == POWER_SUPPLY_STATUS_FULL) {
					val->intval = 100;
				} else if(di->battery_present == 1 &&	// Battery Present and not Full
					  val->intval == 100) {		// but battery level is 100%
					val->intval = 99;		// view as 99% for lockscreen
				} else if(di->voltage_uV > 4190) {	// QM Requested for Dummy Battery Test (LGP970)
					val->intval = 100;
				}
			}
#endif //ntyeongon.moon 2011-03-16  full-charge status comes from soc level [END]

			break;
			/* FIXME : It depends on H/W specific */
		case POWER_SUPPLY_PROP_HEALTH:
			if(!di->battery_present)
				val->intval = POWER_SUPPLY_HEALTH_UNKNOWN;
			else if(di->voltage_uV > 5000)				// Over voltage
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

// S[, 20120725, mannsik.chung@lge.com, Enable charging by fake mode.
		case POWER_SUPPLY_PROP_TEMP_CONTROL:
//#ifdef CONFIG_MACH_LGE_CX2 // <-- not defined.
//			lge_dynamic_nvdata_read(LGE_NVDATA_DYNAMIC_CHARGING_TEMP_OFFSET, &temp_val, 1);
//
//			val->intval = (int)temp_val;
//#else
			val->intval = (int)di->temp_control;
//#endif
			break;
// E], 20120725, mannsik.chung@lge.com, Enable charging by fake mode.
		default:
			return -EINVAL;
	}
	return 0;
}

// S[, 20120725, mannsik.chung@lge.com, Enable charging by fake mode.
int twl4030_bci_battery_set_property(struct power_supply *psy,
				enum power_supply_property psp,
				const union power_supply_propval *val)
{
	int cable_check;
	struct twl4030_bci_device_info *di;

	di = to_twl4030_bci_device_info(psy);

	switch (psp) {

	case POWER_SUPPLY_PROP_TEMP_CONTROL:
		printk("[TWL4030] set di->temp_control = %02X ", val->intval);
		if (val->intval == UNLIMITED_TEMP_VAL) {
			di->temp_control = val->intval;
		}
		else {
			di->temp_control = 0;
		}
		//FIXME: write to nv data.
//		lge_dynamic_nvdata_write(LGE_NVDATA_DYNAMIC_CHARGING_TEMP_OFFSET, &(di->temp_control), 1);
		break;
	default:
		return -EINVAL;
	}
	return 0;
}
EXPORT_SYMBOL(twl4030_bci_battery_set_property);
// E], 20120725, mannsik.chung@lge.com, Enable charging by fake mode.

#define to_twl4030_ac_bci_device_info(x) container_of((x), \
		struct twl4030_bci_device_info, ac);

#define to_twl4030_usb_bci_device_info(x) container_of((x), \
		struct twl4030_bci_device_info, usb);

static int twl4030_ac_bci_battery_get_property(struct power_supply *psy,
					enum power_supply_property psp,
					union power_supply_propval *val)
{

	max8922_status chr_ic_status = CHARGING_IC_DEACTIVE;
	chr_ic_status = get_charging_ic_status();
	TYPE_MUIC_MODE muic_mode = MUIC_NONE;


	switch (psp) {
		case POWER_SUPPLY_PROP_ONLINE:
/* LGE_CHANGE_S [sookyoung.kim@lge.com] 2010-03-19 */
#if defined(CONFIG_HUB_MUIC)
			muic_mode = get_muic_mode();
			if ( muic_mode == MUIC_LG_TA || muic_mode == MUIC_NA_TA || muic_mode == MUIC_HCHH || muic_mode == MUIC_INVALID_CHG )
				val->intval = 1;
			else
				val->intval = 0;
			break;
// kibum.lee@lge.com 20120502 MUIC re-work start
#elif defined(CONFIG_MUIC)
			muic_mode = muic_get_mode();
			if ( muic_mode == MUIC_LG_TA || muic_mode == MUIC_NA_TA || muic_mode == MUIC_TA_1A || muic_mode == MUIC_INVALID_CHG && chr_ic_status != CHARGING_IC_DEACTIVE)
				val->intval = 1;
			else
				val->intval = 0;
			break;
// kibum.lee@lge.com 20120502 MUIC re-work end	
#endif
/* LGE_CHANGE_E [sookyoung.kim@lge.com] 2010-03-19 */
		default:
			return -EINVAL;
	}

	return 0;
}

static int twl4030_usb_bci_battery_get_property(struct power_supply *psy,
					enum power_supply_property psp,
					union power_supply_propval *val)
{

		TYPE_MUIC_MODE muic_mode = MUIC_NONE;


	switch (psp) {
		case POWER_SUPPLY_PROP_ONLINE:
/* LGE_CHANGE_S [sookyoung.kim@lge.com] 2010-03-19 */
#if defined(CONFIG_HUB_MUIC)
			muic_mode = get_muic_mode();
			if ( muic_mode == MUIC_AP_USB)
				val->intval = 1;
			else
				val->intval = 0;
			break;
// kibum.lee@lge.com 20120502 MUIC re-work start
#elif defined(CONFIG_MUIC)
			muic_mode = muic_get_mode();
			if ( muic_mode == MUIC_AP_USB)
				val->intval = 1;
			else
				val->intval = 0;
			break;
// kibum.lee@lge.com 20120502 MUIC re-work end	
#endif
/* LGE_CHANGE_E [sookyoung.kim@lge.com] 2010-03-19 */
		default:
			return -EINVAL;
	}

	return 0;
}



static char *twl4030_bci_supplied_to[] = {
	"battery",
	"ac",
	"usb",
};

static int __init twl4030_bci_battery_probe(struct platform_device *pdev)
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

	// S[, 20120725, mannsik.chung@lge.com, Enable charging by fake mode.
	if (thermal_fakemode)
		di->temp_control = UNLIMITED_TEMP_VAL;
	else
		di->temp_control = 0;
	// E], 20120725, mannsik.chung@lge.com, Enable charging by fake mode.

	/* refer - com_android_server_BatteryService.cpp */
	di->bat.name = "battery";
	di->bat.supplied_to = twl4030_bci_supplied_to;
	di->bat.num_supplicants = ARRAY_SIZE(twl4030_bci_supplied_to);
	di->bat.type = POWER_SUPPLY_TYPE_BATTERY;
	di->bat.properties = twl4030_bci_battery_props;
	di->bat.num_properties = ARRAY_SIZE(twl4030_bci_battery_props);
	di->bat.get_property = twl4030_bci_battery_get_property;
	di->bat.set_property = twl4030_bci_battery_set_property; // 20120725, mannsik.chung@lge.com, Enable charging by fake mode.
	di->bat.external_power_changed = NULL;
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

	twl4030charger_ac_en(DISABLE);
	twl4030charger_usb_en(DISABLE);
	twl4030battery_hw_level_en(DISABLE);
	twl4030battery_hw_presence_en(DISABLE);
	platform_set_drvdata(pdev, di);

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

	INIT_DELAYED_WORK_DEFERRABLE(&di->twl4030_bci_monitor_work, twl4030_bci_battery_work);
	schedule_delayed_work(&di->twl4030_bci_monitor_work, 5 * HZ); //taehwan.kim@lge.com add delay for secure ops

/* LGE_CHANGE_S [taehwan.kim@lge.com] 2010-3-12, android NOT need bk battery voltage*/
#if BK_BATT
	ret = power_supply_register(&pdev->dev, &di->bk_bat);
	if (ret) {
		dev_dbg(&pdev->dev, "failed to register backup battery\n");
		goto bk_batt_failed;
	}

	INIT_DELAYED_WORK_DEFERRABLE(&di->twl4030_bk_bci_monitor_work, twl4030_bk_bci_battery_work);
	schedule_delayed_work(&di->twl4030_bk_bci_monitor_work, HZ*1);
#endif //BK_BATT
/* LGE_CHANGE_E [taehwan.kim@lge.com] 2010-3-12, android NOT need bk battery voltage*/

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
	return 0;

usb_online_failed:
	power_supply_unregister(&di->bat);
ac_online_failed:
	power_supply_unregister(&di->bat);
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
	twl4030charger_ac_en(DISABLE);
	twl4030charger_usb_en(DISABLE);
	twl4030battery_hw_level_en(DISABLE);
	twl4030battery_hw_presence_en(DISABLE);
	kfree(di);

	return ret;
}

static int __exit twl4030_bci_battery_remove(struct platform_device *pdev)
{
	struct twl4030_bci_device_info *di = platform_get_drvdata(pdev);
	int irq;

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
	struct twl4030_bci_device_info *di = platform_get_drvdata(pdev);
    //di->charge_status = POWER_SUPPLY_STATUS_UNKNOWN;  //20101116 taehwan.kim@lge.com
	cancel_delayed_work(&di->twl4030_bci_monitor_work);    
	return 0;
}

static int twl4030_bci_battery_resume(struct platform_device *pdev)
{
	struct twl4030_bci_device_info *di = platform_get_drvdata(pdev);
// 2010.12.22 dajin.kim@lge.com [Start] for battery monitoring during charging
	twl4030_bci_battery_update_status(di);
// 2010.12.22 dajin.kim@lge.com [End] for battery monitoring during charging

	schedule_delayed_work(&di->twl4030_bci_monitor_work, 10 * HZ);
	return 0;
}
#else
#define twl4030_bci_battery_suspend	NULL
#define twl4030_bci_battery_resume	NULL
#endif /* CONFIG_PM */

static struct platform_driver twl4030_bci_battery_driver = {
	.probe		= twl4030_bci_battery_probe,
	.remove		= __exit_p(twl4030_bci_battery_remove),
	.suspend	= twl4030_bci_battery_suspend,
	.resume		= twl4030_bci_battery_resume,
	.driver		= {
		.name	= "twl4030_bci",
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

// S[, 20120725, mannsik.chung@lge.com, Enable charging by fake mode.
static s32 __init check_thermal_fakemode(char *str)
{
    s32 thermal_fakemode_value = simple_strtol(str, NULL, 0);
    thermal_fakemode = thermal_fakemode_value;
    printk(KERN_INFO "[BATTERY] Thermal Fake Mode : %d\n", thermal_fakemode_value);
    return 1;
}
__setup("thermal_fakemode=", check_thermal_fakemode);
// E], 20120725, mannsik.chung@lge.com, Enable charging by fake mode.
