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

#include <linux/i2c/twl.h>


//#define TESTCODE	1



#if defined(CONFIG_HUB_MUIC)
#include "../hub/hub_muic.h"
#endif
#if defined(CONFIG_HUB_CHARGING_IC)
#include "../hub/hub_charging_ic.h"
#endif
#include <linux/max17043_fuelgauge.h>

#define BK_BATT 0


#define T2_BATTERY_VOLT		0x04
#define T2_BATTERY_TEMP		0x06
#define T2_BATTERY_CUR		0x08


#define NO_PW_CONN		0
#define AC_PW_CONN		0x01
#define USB_PW_CONN		0x02


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


#define BCIAUTOWEN		0x020
#define CONFIG_DONE		0x010
#define BCIAUTOUSB		0x002
#define BCIAUTOAC		0x001
#define BCIMSTAT_MASK		0x03F


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


#define REG_BCIISR1A		0x0
#define REG_BCIISR2A		0x01


#define BATSTS_ISR1		0x080
#define VBATLVL_ISR1		0x001


#define REG_BCIIMR1A		0x002
#define REG_BCIIMR2A		0x003


#define BATSTS_IMR1		0x080
#define VBATLVL_IMR1		0x001


#define REG_BCIEDR1		0x00A
#define REG_BCIEDR2		0x00B
#define REG_BCIEDR3		0x00C


#define	BATSTS_EDRRISIN		0x080
#define BATSTS_EDRFALLING	0x040


#define	VBATLVL_EDRRISIN	0x02


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



#ifdef CONFIG_LGE_LAB3_BOARD

#define TRICKLE_CHG_MAX 4160
#define TRICKLE_CHG_TIMER_START 4150
#define TRICKLE_CHG_MIN 4140

#define TRICKLE_LIMMIT_ALERT_CHG_MAX 			TRICKLE_CHG_MAX
#define TRICKLE_LIMMIT_ALERT_CHG_TIMER_START 	TRICKLE_CHG_TIMER_START
#define TRICKLE_LIMMIT_ALERT_CHG_MIN 			TRICKLE_CHG_MIN

#define FORCE_FULL_CHARGE_VOLTAGE_LEVEL 4210


#define NO_BATTERY_ADC_VALUE 625

#else

#define TRICKLE_CHG_MAX 4210
#define TRICKLE_CHG_TIMER_START 4150
#define TRICKLE_CHG_MIN 4140

#define TRICKLE_LIMMIT_ALERT_CHG_MAX 4000
#define TRICKLE_LIMMIT_ALERT_CHG_TIMER_START 3950
#define TRICKLE_LIMMIT_ALERT_CHG_MIN 3900

#define FORCE_FULL_CHARGE_VOLTAGE_LEVEL 4230

#define NO_BATTERY_ADC_VALUE 720
#endif 



int *therm_tbl;

#ifdef TESTCODE
struct pseudo_batt_info_type {
	int mode;
	int id;
	int therm;
	int temp;
	int volt;
	int capacity;
	int charging;
};
#endif


struct twl4030_bci_device_info {
	struct device		*dev;

	unsigned long		update_time;
	int			voltage_uV;
	int			bk_voltage_uV;
	int			current_uA;
	int			temp_C;
	int			charge_rsoc;
	int			charge_status;

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


	struct power_supply	bat;
	struct power_supply	bk_bat;
	struct power_supply     usb_bat;
	struct delayed_work	twl4030_bci_monitor_work;
	struct delayed_work	twl4030_bk_bci_monitor_work;

#ifdef TESTCODE
	struct pseudo_batt_info_type pseudo_batt_info;
#endif

};

#if defined(CONFIG_MACH_LGE_HUB) || defined(CONFIG_MACH_LGE_SNIPER)


#define CONV_VOLTAGE(value) ((value * 150000) / (60 * 1023))
#define TEMP_CRITICAL_UPPER			(55 * 10)
#define TEMP_CRITICAL_LOWER			(-15 * 10)
#define TEMP_LIMIT_UPPER			(99 * 10)
#define TEMP_LIMIT_LOWER			(-99 * 10)
#define MONITOR_WORK_TIME			(10 * HZ)
#define TEMP_CALL_CRITICAL_UPPER	(43 * 10)
#define TEMP_CALL_RECHARGING_UPPER 	(41 * 10)

extern int check_no_lcd(void);

extern int voice_get_curmode(void);


extern u32 wakeup_timer_seconds;	

extern int need_to_quickstart;			
static struct twl4030_bci_device_info *refer_di = NULL;
static int start_monitor = 0;	

static int overtemp_call_recharging = 1;

#endif


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
static int charging_timer_length = 0;
void set_charging_timer(int en)
{
	int time;
	if(refer_di == NULL) {
		charging_timer_length = 0;
		return;
	}

	if(en) {
		if(start_monitor) {
			if(charging_timer_length == 0) {
				do_gettimeofday(&charging_monitor_start_time);
				charging_timer_length = 15;
			}
		} else {
			time = (4200 - refer_di->voltage_uV);
			if(time < 3)
				time = 3;
			
			if(charging_timer_length == 0 || charging_timer_length > time) {
				do_gettimeofday(&charging_monitor_start_time);
				charging_timer_length = time;
				
			}
		}
	} else {
		charging_timer_length = 0;
	}
}
int charging_timeout(void)
{
	static struct timeval current_time;
	__kernel_time_t time_passed;
	
	
	if(charging_timer_length == 0 || refer_di == NULL) {
		return 0;
	}

	do_gettimeofday(&current_time);
	if(charging_monitor_start_time.tv_sec <= current_time.tv_sec) {
		time_passed = current_time.tv_sec - charging_monitor_start_time.tv_sec;
	} else {
		time_passed = charging_monitor_start_time.tv_sec - current_time.tv_sec;
		time_passed = TIME_T_MAX - time_passed;
	}

	if(time_passed > 3 * 60 * 60) {
		
		return 0;
	}
	
	if(time_passed > charging_timer_length * 60)	
		return 1;
	return 0;
}

int check_battery_present(void)
{
	if(refer_di == NULL)
		return 0;
		
	if(refer_di->temp_C == 0xBA00E00)
		return 0;
	return 1;
}
EXPORT_SYMBOL(check_battery_present);


static ssize_t pif_detect_show(struct device *dev,
			       struct device_attribute *attr,
			       char *buf)
{
	int ret, val;

#if defined(CONFIG_HUB_MUIC)
	ret = get_muic_mode();

	if(ret == MUIC_UNKNOWN) {
		val = -1;
	} else if (check_no_lcd() && !check_battery_present()) {
		val = 2;
	} else if (ret == MUIC_AP_UART) {
		val = 1;
	} else if ((ret == MUIC_CP_UART) || (ret == MUIC_CP_USB)) {
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


static ssize_t batt_soc_store(struct device *dev, struct device_attribute *attr,
			      const char *buf, size_t count)
{
#if 0
	int val, batt_raw_cap;

	if(system_rev >= 4)	
		return count;

#if 1
	val = simple_strtoul(buf, NULL, 10);
#else
	sscanf(buf, "%d", &val);
#endif

	
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


static ssize_t fuel_cal_show(struct device *dev,
			     struct device_attribute *attr,
			     char *buf)
{
#if 0
	int val, gauge_diff;
	static int gauge_cal_check = 1;
	TYPE_MUIC_MODE muic_mode = MUIC_NONE;

	if(system_rev >= 4) {	
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
		
		gauge_cal_check = 1;
	}
	else
	{
		
		val = 0;
	}
#endif
	return sprintf(buf, "0\n");
}
static DEVICE_ATTR(pif, S_IRUGO | S_IWUSR, pif_detect_show, NULL);

static DEVICE_ATTR(gauge_if, 0644, fuel_cal_show, batt_soc_store);

static int twl4030vbus_en(void)
{
	int ret = 0;
	u8 hw_conditions = 0;
	
	ret = twl_i2c_read_u8(TWL4030_MODULE_PM_MASTER, &hw_conditions,
		REG_STS_HW_CONDITIONS);
	if (ret)
		return ret;

	if(hw_conditions & STS_VBUS)
		return 1;

	return 0;
}



static inline int twl4030charger_presence_evt(void)
{
	int ret = 0;
	u8 chg_sts = 0, set = 0, clear = 0;

	
	ret = twl_i2c_read_u8(TWL4030_MODULE_PM_MASTER, &chg_sts,
		REG_STS_HW_CONDITIONS);
	if (ret)
		return IRQ_NONE;

	if (chg_sts & STS_CHG) { 
		
		set = CHG_PRES_FALLING;
		clear = CHG_PRES_RISING;
	} else { 
		
		set = CHG_PRES_RISING;
		clear = CHG_PRES_FALLING;
	}

	
	clear_n_set(TWL4030_MODULE_INT, clear, set, REG_PWR_EDR1);

	return 0;
}


static irqreturn_t twl4030charger_interrupt(int irq, void *_di)
{
#ifdef CONFIG_LOCKDEP
	
	local_irq_enable();
#endif

	twl4030charger_presence_evt();

	return IRQ_HANDLED;
}


static int twl4030battery_presence_evt(void)
{
	int ret;
	u8 batstsmchg = 0, batstspchg = 0;

	
	ret = twl_i2c_read_u8(TWL4030_MODULE_MAIN_CHARGE,
			&batstsmchg, REG_BCIMFSTS3);
	if (ret)
		return ret;

	
	ret = twl_i2c_read_u8(TWL4030_MODULE_PRECHARGE,
			&batstspchg, REG_BCIMFSTS1);
	if (ret)
		return ret;

	
	if ((batstspchg & BATSTSPCHG) || (batstsmchg & BATSTSMCHG)) {
		
		ret = clear_n_set(TWL4030_MODULE_INTERRUPTS, BATSTS_EDRRISIN,
			BATSTS_EDRFALLING, REG_BCIEDR2);
		if (ret)
			return ret;
	} else {
		
		ret = clear_n_set(TWL4030_MODULE_INTERRUPTS, BATSTS_EDRFALLING,
			BATSTS_EDRRISIN, REG_BCIEDR2);
		if (ret)
			return ret;
	}

	return 0;
}


static int twl4030battery_level_evt(void)
{
	int ret = 0;
	u8 mfst = 0;

	
	ret = twl_i2c_read_u8(TWL4030_MODULE_MAIN_CHARGE,
			&mfst, REG_BCIMFSTS2);
	if (ret)
		return ret;

	
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


static irqreturn_t twl4030battery_interrupt(int irq, void *_di)
{
	u8 isr1a_val = 0, isr2a_val = 0, clear_2a = 0, clear_1a = 0;
	int ret = 0;

#ifdef CONFIG_LOCKDEP
	
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

	
	ret = twl_i2c_write_u8(TWL4030_MODULE_INTERRUPTS,
			clear_1a , REG_BCIISR1A);
	if (ret)
		return IRQ_NONE;

	ret = twl_i2c_write_u8(TWL4030_MODULE_INTERRUPTS,
			clear_2a , REG_BCIISR2A);
	if (ret)
		return IRQ_NONE;

	
	if (isr1a_val & BATSTS_ISR1)
		twl4030battery_presence_evt();
	
	else if (isr2a_val & VBATLVL_ISR1)
		twl4030battery_level_evt();
	else
		return IRQ_NONE;

	return IRQ_HANDLED;
}


static int twl4030battery_hw_level_en(int enable)
{
	int ret;

	if (enable) {
		
		ret = clear_n_set(TWL4030_MODULE_INTERRUPTS, VBATLVL_IMR1,
			0, REG_BCIIMR2A);
		if (ret)
			return ret;

		
		ret = clear_n_set(TWL4030_MODULE_INTERRUPTS, 0,
			VBATLVL_EDRRISIN, REG_BCIEDR3);
		if (ret)
			return ret;
	} else {
		
		ret = clear_n_set(TWL4030_MODULE_INTERRUPTS, 0,
			VBATLVL_IMR1, REG_BCIIMR2A);
		if (ret)
			return ret;
	}

	return 0;
}


static int twl4030battery_hw_presence_en(int enable)
{
	int ret;

	if (enable) {
		
		ret = clear_n_set(TWL4030_MODULE_INTERRUPTS, BATSTS_IMR1,
			0, REG_BCIIMR1A);
		if (ret)
			return ret;

		
		ret = clear_n_set(TWL4030_MODULE_INTERRUPTS, 0,
			BATSTS_EDRRISIN | BATSTS_EDRFALLING, REG_BCIEDR2);
		if (ret)
			return ret;
	} else {
		
		ret = clear_n_set(TWL4030_MODULE_INTERRUPTS, 0,
			BATSTS_IMR1, REG_BCIIMR1A);
		if (ret)
			return ret;
	}

	return 0;
}


static int twl4030charger_ac_en(int enable)
{
	int ret;

	if (enable) {
		
		ret = clear_n_set(TWL4030_MODULE_PM_MASTER, 0,
			(CONFIG_DONE | BCIAUTOWEN | BCIAUTOAC),
			REG_BOOT_BCI);
		if (ret)
			return ret;
	} else {
		
		ret = clear_n_set(TWL4030_MODULE_PM_MASTER, BCIAUTOAC,
			(CONFIG_DONE | BCIAUTOWEN),
			REG_BOOT_BCI);
		if (ret)
			return ret;
	}

	return 0;
}


#if defined(CONFIG_MACH_LGE_HUB)	
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
		
		ret = twl4030charger_presence();
		if (ret < 0)
			return ret;

		if (!(ret & USB_PW_CONN))
			return -ENXIO;

		
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

		
		ret = clear_n_set(TWL4030_MODULE_USB, 0, OTG_EN,
			REG_POWER_CTRL);
		if (ret)
			return ret;

		mdelay(50);

		
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
#endif


#if defined(CONFIG_MACH_LGE_HUB) || defined(CONFIG_MACH_LGE_SNIPER)
struct temperature_table {
	int tmp;	
	int adc;	
};
static int adc2temperature(int adc)
{
	#define TEMP_MULTIPLEX 10
	static struct temperature_table thm_tbl[] = {
		{ 600,	120},	
		{ 500,	155},	
		{ 400,	205},	
		{ 300,	265},	
		{ 200,	350},	
		{ 100,	440},	
		{   0,	525},	
		{-100,	580},	
		{-200,	645},	
		{-1, -1}		
	};
		
	int tmp;
	int grad, inter;
	int i = 0;

	
	while(thm_tbl[i].adc != -1) {
		if(adc < thm_tbl[i].adc)
			break;
		i++;
	}
	
	if(i == 0) {
		grad = -5 * TEMP_MULTIPLEX;		
		inter = (thm_tbl[0].tmp * TEMP_MULTIPLEX) - grad * thm_tbl[0].adc;
	} else if(thm_tbl[i].adc == -1) {
		grad = -2 * TEMP_MULTIPLEX;		
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


    if ((temp < 0) || (temp > 1000))
    {
        
        temp = 200;
    }
    

	if (temp > 720) {		
		return 0xBA00E00;	
	}
	
	
	temp = adc2temperature(temp);


    return temp;

}
#else
static int twl4030battery_temperature(void)
{
	u8 val = 0;
	int temp, curr, volt, res, ret;

	
	ret = read_bci_val(T2_BATTERY_TEMP);
	if (ret < 0)
		return ret;

	volt = (ret * TEMP_STEP_SIZE) / TEMP_PSR_R;

	
	ret = twl_i2c_read_u8(TWL4030_MODULE_MAIN_CHARGE, &val,
		 REG_BCICTL2);
	if (ret)
		return 0;

	curr = ((val & ITHSENS) + 1) * 10;

	
	res = volt * 1000 / curr;

	
	for (temp = 58; temp >= 0; temp--) {
		int actual = therm_tbl[temp];
		if ((actual - res) >= 0)
			break;
	}

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


static int twl4030battery_voltage(void)
{
#ifdef FUELGAUGE_AP_ONLY
    return max17043_get_voltage();
#else
	int volt;
	u8 hwsts;
	struct twl4030_madc_request req;

	if(system_rev >= 4)	
		return max17043_get_voltage();

	twl_i2c_read_u8(TWL4030_MODULE_PM_MASTER, &hwsts,
		REG_STS_HW_CONDITIONS);

	if ((hwsts & STS_CHG) || (hwsts & STS_VBUS)) {
		
		volt = read_bci_val(T2_BATTERY_VOLT);
		return (volt * VOLT_STEP_SIZE) / VOLT_PSR_R;
	} else {
		
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

static int twl4030battery_current(void)
{
	int ret, curr = read_bci_val(T2_BATTERY_CUR);
	u8 val = 0;

	ret = twl_i2c_read_u8(TWL4030_MODULE_MAIN_CHARGE, &val,
		REG_BCICTL1);
	if (ret)
		return ret;

	if (val & CGAIN) 
		return (curr * CURR_STEP_SIZE) / CURR_PSR_R1;
	else 
		return (curr * CURR_STEP_SIZE) / CURR_PSR_R2;
}
#endif

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

	if(system_rev >= 4) {	
		ret = max17043_get_capacity();
	} else {				
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


#if defined(CONFIG_MACH_LGE_HUB) || defined(CONFIG_MACH_LGE_SNIPER)
    ret = clear_n_set(TWL4030_MODULE_MAIN_CHARGE, 0, USBFASTMCHG, REG_BCIMFSTS4);
#endif 


	
	ret = twl_i2c_read_u8(TWL4030_MODULE_MAIN_CHARGE, &val,
		reg + 1);
	if (ret)
		return ret;

	temp = ((int)(val & 0x03)) << 8;

	
	ret = twl_i2c_read_u8(TWL4030_MODULE_MAIN_CHARGE, &val,
		reg);
	if (ret)
		return ret;

	return temp | val;
}
#endif


static int twl4030backupbatt_voltage_setup(void)
{
	int ret;

	
	ret = clear_n_set(TWL4030_MODULE_PM_RECEIVER, 0, BBCHEN,
		REG_BB_CFG);
	if (ret)
		return ret;

	return 0;
}

#if defined(CONFIG_MACH_LGE_HUB) || defined(CONFIG_MACH_LGE_SNIPER)
#else

static int twl4030battery_temp_setup(void)
{
	int ret;

	
	ret = clear_n_set(TWL4030_MODULE_MAIN_CHARGE, 0, ITHEN,
		REG_BCICTL1);
	if (ret)
		return ret;

	return 0;
}
#endif

static inline int clear_n_set(u8 mod_no, u8 clear, u8 set, u8 reg)
{
	int ret;
	u8 val = 0;

	
	ret = twl_i2c_read_u8(mod_no, &val, reg);
	if (ret)
		return ret;

	
	val &= ~(clear);

	
	val |= set;

	
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
#if defined(CONFIG_MACH_LGE_HUB)	
	POWER_SUPPLY_PROP_HEALTH,
	POWER_SUPPLY_PROP_TECHNOLOGY,
    POWER_SUPPLY_PROP_PRESENT,
#endif

#ifdef TESTCODE
	POWER_SUPPLY_PROP_PSEUDO_BATT, 
#endif

};

#if BK_BATT
static enum power_supply_property twl4030_bk_bci_battery_props[] = {
	POWER_SUPPLY_PROP_VOLTAGE_NOW,
};
#endif

#if defined(CONFIG_MACH_LGE_HUB)	
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
	
	di->temp_C = twl4030battery_temperature();		
#ifdef FUELGAUGE_AP_ONLY
	if(di->battery_present) {			
#else
	if(system_rev >= 4 && di->battery_present) {		
#endif
		max17043_set_rcomp_by_temperature(di->temp_C);
		
	}
	di->voltage_uV = twl4030battery_voltage();		
	di->battery_capacity = twl4030battery_capacity(di);	
	di->battery_present = check_battery_present();		

	di->current_uA = 0;
#else
	di->temp_C = twl4030battery_temperature();
	di->voltage_uV = twl4030battery_voltage();
	di->current_uA = twl4030battery_current();
	di->capacity = twl4030battery_capacity(di);
#endif

#ifdef TESTCODE
	if( di->pseudo_batt_info.mode ) {
		if( -1 != di->pseudo_batt_info.temp ) {
			di->temp_C = di->pseudo_batt_info.temp;
		}
		if( -1 != di->pseudo_batt_info.id ) {
			di->battery_present = di->pseudo_batt_info.id;
		}
		if( -1 != di->pseudo_batt_info.volt ) {
			di->voltage_uV = di->pseudo_batt_info.volt;		
		}
		if( -1 != di->pseudo_batt_info.capacity ) {
			di->battery_capacity = di->pseudo_batt_info.capacity;	
		}
	}
#endif

}

static void set_battery_status(struct twl4030_bci_device_info *di)
{
#if defined(CONFIG_HUB_MUIC)
	TYPE_MUIC_MODE muic_mode = get_muic_mode();

	
	switch (muic_mode) {
		case MUIC_AP_UART:
		case MUIC_CP_UART:
		case MUIC_CP_USB:
			di->charge_rsoc = POWER_SUPPLY_TYPE_UPS;
			wakeup_timer_seconds = 0;	
			if(!twl4030vbus_en()) {
				di->charge_rsoc = POWER_SUPPLY_TYPE_BATTERY;				
			}
			break;
		case MUIC_NA_TA:
		case MUIC_LG_TA:
		case MUIC_HCHH:
		case MUIC_INVALID_CHG:
			di->charge_rsoc = POWER_SUPPLY_TYPE_MAINS;
			wakeup_timer_seconds = 60;	
			if(!twl4030vbus_en()) {
				
				di->charge_rsoc = POWER_SUPPLY_TYPE_BATTERY;
			}


			break;
		case MUIC_AP_USB:
			di->charge_rsoc = POWER_SUPPLY_TYPE_USB;
			wakeup_timer_seconds = 0;	
			if(!twl4030vbus_en()) {
				di->charge_rsoc = POWER_SUPPLY_TYPE_BATTERY;
			}
			break;
		default :
			di->charge_rsoc = POWER_SUPPLY_TYPE_BATTERY;
			wakeup_timer_seconds = 0;	
			if(di->battery_capacity <= 1)	
				wakeup_timer_seconds = 60;
			if(twl4030vbus_en() && muic_mode != MUIC_UNKNOWN) {
				
			}

		break;
	}
#elif defined(CONFIG_LGE_OMAP3_EXT_PWR)
	
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
	
	
	if(di->charge_rsoc == POWER_SUPPLY_TYPE_UPS ||		
	   di->charge_rsoc == POWER_SUPPLY_TYPE_BATTERY) {	
		di->charge_status = POWER_SUPPLY_STATUS_DISCHARGING;
	} else {
		if(voice_get_curmode()==0) {
			if(di->battery_present == 0) {			
				di->charge_status = POWER_SUPPLY_STATUS_NOT_CHARGING;
			} else if(di->temp_C < TEMP_CRITICAL_LOWER || di->temp_C > TEMP_CRITICAL_UPPER) {	
				di->charge_status = POWER_SUPPLY_STATUS_NOT_CHARGING;
			} else if(di->temp_C < TEMP_LIMIT_LOWER || di->temp_C > TEMP_LIMIT_UPPER) {	
				di->charge_status = POWER_SUPPLY_STATUS_CHARGING;
			} else if((start_monitor || end_of_charge) && di->battery_capacity > 97) {		
				di->charge_status = POWER_SUPPLY_STATUS_FULL;
			} else {					
				di->charge_status = POWER_SUPPLY_STATUS_CHARGING;
			}

		} else {
			if(di->battery_present == 0) {			
				di->charge_status = POWER_SUPPLY_STATUS_NOT_CHARGING;
			} else if(di->temp_C < TEMP_CRITICAL_LOWER) {
				di->charge_status = POWER_SUPPLY_STATUS_NOT_CHARGING;
			} else if(di->temp_C >= TEMP_CALL_CRITICAL_UPPER) {
				di->charge_status = POWER_SUPPLY_STATUS_NOT_CHARGING;
			} else if(overtemp_call_recharging == 0) {
				di->charge_status = POWER_SUPPLY_STATUS_NOT_CHARGING;
			} else if(di->temp_C < TEMP_LIMIT_LOWER || di->temp_C > TEMP_LIMIT_UPPER) {	
				di->charge_status = POWER_SUPPLY_STATUS_CHARGING;
			} else if((start_monitor || end_of_charge) && di->battery_capacity > 97) {		
				di->charge_status = POWER_SUPPLY_STATUS_FULL;
			} else {					
				di->charge_status = POWER_SUPPLY_STATUS_CHARGING;
			}
		}
		
	}
}

static int check_battery_changed(struct twl4030_bci_device_info *di)
{
	static int battery_calibration_enabled = 0;
	int need_recalibration = 0;
	int upper_limit, lower_limit;
	
	if(battery_calibration_enabled) {	
		start_monitor = 0;
		set_end_of_charge(0);
		set_charging_timer(0);
		di->charge_status = POWER_SUPPLY_STATUS_CHARGING;
		battery_calibration_enabled = 0;
		
		return 2;
	}

	if(di->charge_rsoc == POWER_SUPPLY_TYPE_BATTERY)
		return 0;
	if(di->battery_present == 0)			
		return 0;
	if(di->temp_C < 0 || di->temp_C > 600)	
		return 0;

	upper_limit = 300;
	lower_limit = -250;

	
	if(di->previous_battery_present == 0) {		
		need_recalibration = 1;

 
#ifdef CONFIG_LGE_LAB3_BOARD
		if( check_battery_present() ) {
			extern void setting_for_factory_mode_hw_req(void);
			setting_for_factory_mode_hw_req();
		}
#endif 


	} else {
		int voltage_diff;
		voltage_diff = di->voltage_uV - di->previous_voltage_uV;
		if(voltage_diff > upper_limit) {			
			if(di->previous_charge_rsoc != POWER_SUPPLY_TYPE_BATTERY &&
			   di->pre_pre_charge_rsoc != POWER_SUPPLY_TYPE_BATTERY) {
			   	need_recalibration = 1;
			}
		} else if(voltage_diff < lower_limit) {	
			need_recalibration = 1;
		}
	}

	if(need_recalibration) {
		charging_ic_deactive();
		battery_calibration_enabled = 1;
		di->charge_status = POWER_SUPPLY_STATUS_UNKNOWN;
		
		mdelay(50);				
		max17043_do_calibrate();		
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
	
	if(di->battery_present == 0 || 						
		di->charge_rsoc == POWER_SUPPLY_TYPE_BATTERY) {	
		if(start_monitor == 1)
			
		start_monitor = 0;
		set_end_of_charge(0);
		set_charging_timer(0);
		return 0;
	}
	
	
	if(voice_get_curmode()==0) {
		overtemp_call_recharging = 1;
		if(di->temp_C < TEMP_CRITICAL_LOWER ||
		   di->temp_C > TEMP_CRITICAL_UPPER) {		
			start_monitor = 0;
			set_end_of_charge(0);
			set_charging_timer(0);
			if (di->battery_present == 1) {
				
#ifdef CONFIG_LGE_CHARGE_CONTROL_BATTERY_FET
				lge_battery_fet_onoff(0);
#else 
				charging_ic_deactive();
#endif 
			}
			return 0;
		} else if(di->temp_C < TEMP_LIMIT_LOWER ||
			  di->temp_C > TEMP_LIMIT_UPPER) {	
			trickle_chg_max = TRICKLE_LIMMIT_ALERT_CHG_MAX;
			trickle_chg_timer_start = TRICKLE_LIMMIT_ALERT_CHG_TIMER_START;
			trickle_chg_min = TRICKLE_LIMMIT_ALERT_CHG_MIN;
		} else {									
			trickle_chg_max = TRICKLE_CHG_MAX;					
			trickle_chg_timer_start = TRICKLE_CHG_TIMER_START;
			trickle_chg_min = TRICKLE_CHG_MIN;
			if(di->previous_temp_C < TEMP_LIMIT_LOWER ||
			   di->previous_temp_C > TEMP_LIMIT_UPPER) {
				start_monitor = 0;
				set_end_of_charge(0);
				set_charging_timer(0);
			}
		}
	} else { 
		if(di->temp_C > TEMP_CALL_CRITICAL_UPPER) {
			overtemp_call_recharging = 0;
			start_monitor = 0;
			set_end_of_charge(0);
			set_charging_timer(0);
			if (di->battery_present == 1) {
				
#ifdef CONFIG_LGE_CHARGE_CONTROL_BATTERY_FET
				lge_battery_fet_onoff(0);
#else 
				charging_ic_deactive();
#endif 
			}
			return 0;
		} else if(di->temp_C < TEMP_CALL_RECHARGING_UPPER&& 
					di->previous_temp_C >= TEMP_CALL_RECHARGING_UPPER&&
					overtemp_call_recharging == 0) {
			trickle_chg_max = TRICKLE_CHG_MAX;					
			trickle_chg_timer_start = TRICKLE_CHG_TIMER_START;
			trickle_chg_min = TRICKLE_CHG_MIN;
			overtemp_call_recharging = 1;
		} else if(di->temp_C >= TEMP_CALL_RECHARGING_UPPER&&
			di->previous_temp_C >= TEMP_CALL_RECHARGING_UPPER&&
			overtemp_call_recharging == 0) {
			overtemp_call_recharging = 0;
			start_monitor = 0;
			set_end_of_charge(0);
			set_charging_timer(0);
			if (di->battery_present == 1) {
				charging_ic_deactive();
			}
			return 0;	
		} else if(di->temp_C < TEMP_LIMIT_LOWER || di->temp_C > TEMP_LIMIT_UPPER) {
			trickle_chg_max = TRICKLE_LIMMIT_ALERT_CHG_MAX;
			trickle_chg_timer_start = TRICKLE_LIMMIT_ALERT_CHG_TIMER_START;
			trickle_chg_min = TRICKLE_LIMMIT_ALERT_CHG_MIN;
		} else {
			overtemp_call_recharging = 1;
			trickle_chg_max = TRICKLE_CHG_MAX;					
			trickle_chg_timer_start = TRICKLE_CHG_TIMER_START;
			trickle_chg_min = TRICKLE_CHG_MIN;
			if(di->previous_temp_C < TEMP_LIMIT_LOWER ||
			   di->previous_temp_C > TEMP_LIMIT_UPPER) {
				start_monitor = 0;
				set_end_of_charge(0);
				set_charging_timer(0);
			}
		}
	
	}
	if(chr_ic_status != CHARGING_IC_DEACTIVE) {
		if(di->voltage_uV >= trickle_chg_max || end_of_charge || charging_timeout()) {
			if(start_monitor) {
				charging_ic_deactive();
				set_charging_timer(0);
			} else {
				start_monitor = 1;
				if(di->voltage_uV >= trickle_chg_max || end_of_charge) {		

#ifdef CONFIG_LGE_CHARGE_CONTROL_BATTERY_FET
				lge_battery_fet_onoff(0);
#else 
				charging_ic_deactive();	
#endif 
					set_charging_timer(0);
				} else {
					set_charging_timer(0);
					set_charging_timer(1);
				}
			}
			set_end_of_charge(0);
		} else if(di->voltage_uV >= trickle_chg_timer_start) {
			set_charging_timer(1);
		}
	} else if(di->voltage_uV < trickle_chg_min) {	
		if(start_monitor) {
			set_charging_timer(1);
		}

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

#if defined(CONFIG_MACH_LGE_HUB)  || defined(CONFIG_MACH_LGE_SNIPER)
	set_battery_status(di);
	switch(check_battery_changed(di)) {
		case 2:		
			set_battery_charging(di);
			di->voltage_uV = di->previous_voltage_uV;		
			break;
		case 1:		
			backup_battery_info(di);
			break;
		default:	
			set_battery_charging(di);
			backup_battery_info(di);
			break;
	}
	power_supply_changed(&di->bat);
#else

	di->charge_rsoc = usb_charger_flag;
	if (old_capacity != di->capacity) {
		stable_count = 0;
    } else {
		stable_count++;
	}
	if ((old_charge_status != di->charge_status)
			|| (stable_count == 20)
			|| (old_charge_source !=  di->charge_rsoc)) {
		power_supply_changed(&di->bat);
	}


#endif
}

#if defined(CONFIG_MACH_LGE_HUB)	
void charger_state_update_by_other(void)
{
	if(refer_di == NULL)
		return;

	cancel_delayed_work(&refer_di->twl4030_bci_monitor_work);
	twl4030_bci_battery_update_status(refer_di);
#ifdef FUELGAUGE_AP_ONLY
#else
	if(system_rev >= 4) {
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
#else
	if(system_rev >= 4) {
	} else {
		schedule_delayed_work(&di->twl4030_bci_monitor_work, MONITOR_WORK_TIME); 
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
#ifdef TESTCODE
static int twl4030_bci_battery_set_property(struct power_supply *psy,
					    enum power_supply_property psp,
					    const union power_supply_propval *val)
{
	struct twl4030_bci_device_info *di;
	di = to_twl4030_bci_device_info(psy);
	switch (psp) {
		case POWER_SUPPLY_PROP_PSEUDO_BATT:
			if (sscanf(val->strval, "%d %d %d %d %d %d %d", 
						&di->pseudo_batt_info.mode, 
						&di->pseudo_batt_info.id, 
						&di->pseudo_batt_info.therm,
						&di->pseudo_batt_info.temp, 
						&di->pseudo_batt_info.volt, 
						&di->pseudo_batt_info.capacity, 
						&di->pseudo_batt_info.charging) != 7)
			{
				memset(&di->pseudo_batt_info, 0, sizeof(struct pseudo_batt_info_type));
				printk(KERN_ERR "%s: <usage> echo [*mode] [*ID] [therm] [*temp] [*volt] [*cap] [charging] > pseudo_batt\n", __FUNCTION__);
			} else {
				printk(KERN_INFO "%s: pseudo_batt set\n mode 		: %d\n id 			: %d\n therm 		: %d\n temp 		: %d\n volt 		: %d\n capacity 	: %d\n charging 	: %d\n", __FUNCTION__, 
						di->pseudo_batt_info.mode, 
						di->pseudo_batt_info.id, 
						di->pseudo_batt_info.therm, 
						di->pseudo_batt_info.temp, 
						di->pseudo_batt_info.volt, 
						di->pseudo_batt_info.capacity, 
						di->pseudo_batt_info.charging);
			}
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

#if defined(CONFIG_HUB_MUIC) && !defined(CONFIG_LGE_OMAP3_EXT_PWR)
	TYPE_MUIC_MODE muic_mode;
#endif

	di = to_twl4030_bci_device_info(psy);
	switch (psp) {
	case POWER_SUPPLY_PROP_STATUS:
		val->intval = di->charge_status;
			if(di->voltage_uV > FORCE_FULL_CHARGE_VOLTAGE_LEVEL) {		
				val->intval = POWER_SUPPLY_STATUS_FULL;
	}
			break;
	case POWER_SUPPLY_PROP_VOLTAGE_NOW:
		val->intval = di->voltage_uV * 1000;
		break;
	case POWER_SUPPLY_PROP_CURRENT_NOW:
		val->intval = di->current_uA;
		break;
	case POWER_SUPPLY_PROP_TEMP:
			if(di->battery_present == 0)		
				val->intval = 200;
			else
		val->intval = di->temp_C;
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
			if(muic_mode == MUIC_CP_USB  ||		
			   muic_mode == MUIC_CP_UART ||
			   muic_mode == MUIC_AP_UART) {
				val->intval = 1;
			} else if(muic_mode == MUIC_NONE) {	
				val->intval = 1;
			}
#endif

			break;
	case POWER_SUPPLY_PROP_CAPACITY:
#if defined(CONFIG_MACH_LGE_HUB) || defined(CONFIG_MACH_LGE_SNIPER)
			muic_mode = get_muic_mode();
			val->intval = di->battery_capacity;
			if(di->charge_rsoc == POWER_SUPPLY_TYPE_MAINS ||
			   di->charge_rsoc == POWER_SUPPLY_TYPE_USB) {	
				if(di->charge_status == POWER_SUPPLY_STATUS_FULL) {
					val->intval = 100;
				} else if(di->voltage_uV > 4230) {	
					val->intval = 100;
				} else if(di->battery_present == 1 &&	
					  val->intval == 100) {		
					val->intval = 99;		
				}
			}
#else 
		val->intval = twl4030battery_capacity(di);
#endif 
			break;
#if defined(CONFIG_MACH_LGE_HUB)
		case POWER_SUPPLY_PROP_HEALTH:
			if(voice_get_curmode()==0) {
				if(!di->battery_present) {
					val->intval = POWER_SUPPLY_HEALTH_UNKNOWN;
				} else if(di->voltage_uV > 5000) {		
					val->intval = POWER_SUPPLY_HEALTH_OVERVOLTAGE;
				} else if(di->temp_C < TEMP_CRITICAL_LOWER)	{ 
					val->intval = POWER_SUPPLY_HEALTH_COLD;
				} else if(di->temp_C > TEMP_CRITICAL_UPPER)	{ 
					val->intval = POWER_SUPPLY_HEALTH_OVERHEAT;
				} else {
					val->intval = POWER_SUPPLY_HEALTH_GOOD;
				}
			} else {

				if(!di->battery_present) {
					val->intval = POWER_SUPPLY_HEALTH_UNKNOWN;
				} else if(di->voltage_uV > 5000) {			
					val->intval = POWER_SUPPLY_HEALTH_OVERVOLTAGE;
				} else if(di->temp_C < TEMP_CRITICAL_LOWER)	{
					val->intval = POWER_SUPPLY_HEALTH_COLD;
				} else if(di->temp_C > TEMP_CALL_CRITICAL_UPPER) {
					val->intval = POWER_SUPPLY_HEALTH_OVERHEAT;
				} else if(overtemp_call_recharging == 0 && di->temp_C >= TEMP_CALL_RECHARGING_UPPER) {
					val->intval = POWER_SUPPLY_HEALTH_OVERHEAT;
				} else if(overtemp_call_recharging == 1 || di->temp_C < TEMP_CALL_RECHARGING_UPPER) {
					val->intval = POWER_SUPPLY_HEALTH_GOOD;
				}
			}
			break;
		case POWER_SUPPLY_PROP_TECHNOLOGY:
			val->intval = POWER_SUPPLY_TECHNOLOGY_LION;	
			break;
#endif
#ifdef TESTCODE
		case POWER_SUPPLY_PROP_PSEUDO_BATT:
			val->intval = di->pseudo_batt_info.mode;
			break;
#endif
		default:
			return -EINVAL;
	}
	return 0;
}

#if defined(CONFIG_MACH_LGE_HUB)	
#define to_twl4030_ac_bci_device_info(x) container_of((x), \
		struct twl4030_bci_device_info, ac);

#define to_twl4030_usb_bci_device_info(x) container_of((x), \
		struct twl4030_bci_device_info, usb);

static int twl4030_ac_bci_battery_get_property(struct power_supply *psy,
					enum power_supply_property psp,
					union power_supply_propval *val)
{
#if defined(CONFIG_HUB_MUIC) && !defined(CONFIG_LGE_OMAP3_EXT_PWR)
		TYPE_MUIC_MODE muic_mode = MUIC_NONE;
#endif

	switch (psp) {
		case POWER_SUPPLY_PROP_ONLINE:
#if defined(CONFIG_HUB_MUIC)
			muic_mode = get_muic_mode();
			if ( muic_mode == MUIC_LG_TA || muic_mode == MUIC_NA_TA || muic_mode == MUIC_HCHH || muic_mode == MUIC_INVALID_CHG )
				val->intval = 1;
			else
				val->intval = 0;
			break;
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
#if defined(CONFIG_HUB_MUIC) && !defined(CONFIG_LGE_OMAP3_EXT_PWR)
		TYPE_MUIC_MODE muic_mode = MUIC_NONE;
#endif

	switch (psp) {
		case POWER_SUPPLY_PROP_ONLINE:
#if defined(CONFIG_HUB_MUIC)
			muic_mode = get_muic_mode();
			if ( muic_mode == MUIC_AP_USB)
				val->intval = 1;
			else
				val->intval = 0;
			break;
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
#if defined(CONFIG_MACH_LGE_HUB)	
	"battery",
	"ac",
	"usb",
#else
	"twl4030_bci_battery",
#endif
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
#if defined(CONFIG_MACH_LGE_HUB)	
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
#ifdef TESTCODE
	di->bat.set_property = twl4030_bci_battery_set_property;
#endif
#if defined(CONFIG_MACH_LGE_HUB) || defined(CONFIG_MACH_LGE_SNIPER)
	di->bat.external_power_changed = NULL;
#else
	di->bat.external_power_changed = twl4030_bci_battery_external_power_changed;
#endif
	di->bat.set_charged = NULL;

#if BK_BATT
	di->bk_bat.name = "twl4030_bci_bk_battery";
	di->bk_bat.type = POWER_SUPPLY_TYPE_BATTERY;
	di->bk_bat.properties = twl4030_bk_bci_battery_props;
	di->bk_bat.num_properties = ARRAY_SIZE(twl4030_bk_bci_battery_props);
	di->bk_bat.get_property = twl4030_bk_bci_battery_get_property;
	di->bk_bat.external_power_changed = NULL;
	di->bk_bat.set_charged = NULL;
#endif 


#if defined(CONFIG_MACH_LGE_HUB)
	di->ac.name = "ac";
	di->ac.type = POWER_SUPPLY_TYPE_MAINS; 
	di->ac.properties = twl4030_ac_usb_bci_battery_props;
	di->ac.num_properties = ARRAY_SIZE(twl4030_ac_usb_bci_battery_props);
	di->ac.get_property = twl4030_ac_bci_battery_get_property;
	di->ac.external_power_changed = NULL;
	di->ac.set_charged = NULL;

	di->usb.name = "usb";
	di->usb.type = POWER_SUPPLY_TYPE_USB; 
	di->usb.properties = twl4030_ac_usb_bci_battery_props;
	di->usb.num_properties = ARRAY_SIZE(twl4030_ac_usb_bci_battery_props);
	di->usb.get_property = twl4030_usb_bci_battery_get_property;
	di->usb.external_power_changed = NULL;
	di->usb.set_charged = NULL;

#else
	di->usb_bat.name = "twl4030_bci_usb_src";
	di->usb_bat.supplied_to = twl4030_bci_supplied_to;
	di->usb_bat.type = POWER_SUPPLY_TYPE_USB;
	di->usb_bat.properties = twl4030_usb_battery_props;
	di->usb_bat.num_properties = ARRAY_SIZE(twl4030_usb_battery_props);
	di->usb_bat.get_property = twl4030_usb_battery_get_property;
	di->usb_bat.external_power_changed = NULL;
#endif
#if defined(CONFIG_MACH_LGE_HUB)	
	twl4030charger_ac_en(DISABLE);
	twl4030charger_usb_en(DISABLE);
	twl4030battery_hw_level_en(DISABLE);
	twl4030battery_hw_presence_en(DISABLE);
#else
	twl4030charger_ac_en(ENABLE);
	twl4030charger_usb_en(ENABLE);
	twl4030battery_hw_level_en(ENABLE);
	twl4030battery_hw_presence_en(ENABLE);
#endif 
	platform_set_drvdata(pdev, di);

#if defined(CONFIG_MACH_LGE_HUB) || defined(CONFIG_MACH_LGE_SNIPER)
#else
	ret = twl4030battery_temp_setup();
	if (ret)
		goto temp_setup_fail;
#endif
	ret = twl4030backupbatt_voltage_setup();
	if (ret)
		goto voltage_setup_fail;
	irq = platform_get_irq(pdev, 1);
	ret = request_irq(irq, twl4030battery_interrupt,
		0, pdev->name, NULL);
	if (ret) {
		dev_dbg(&pdev->dev, "could not request irq %d, status %d\n",
			irq, ret);
		goto batt_irq_fail;
	}

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
#ifdef FUELGAUGE_AP_ONLY
#else
	if(system_rev >= 4) {
	} else {
		schedule_delayed_work(&di->twl4030_bci_monitor_work, 5 * HZ); 
	}
#endif

#if BK_BATT
	ret = power_supply_register(&pdev->dev, &di->bk_bat);
	if (ret) {
		dev_dbg(&pdev->dev, "failed to register backup battery\n");
		goto bk_batt_failed;
	}

	INIT_DELAYED_WORK_DEFERRABLE(&di->twl4030_bk_bci_monitor_work, twl4030_bk_bci_battery_work);
	schedule_delayed_work(&di->twl4030_bk_bci_monitor_work, HZ*1);
#endif 

#if defined(CONFIG_MACH_LGE_HUB)
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
		return ret;
	}

	ret = device_create_file(&pdev->dev, &dev_attr_gauge_if);
	if (ret) {
		return ret;
	}

	refer_di = di;

	charging_timer_length = 0;
	twl4030_bci_battery_read_status(di);
	set_battery_status(di);	
	
	di->previous_charge_rsoc = di->charge_rsoc;
	backup_battery_info(di);
	di->bk_voltage_uV = 3700;

	if(need_to_quickstart && di->battery_present == 1) {
		charging_ic_deactive();
		max17043_do_calibrate();
		set_battery_charging(di);
	}

	return 0;

#if defined(CONFIG_MACH_LGE_HUB)	
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
#if BK_BATT
	power_supply_unregister(&di->bk_bat);
#endif 
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

#ifdef FUELGAUGE_AP_ONLY
#else
	if(system_rev >= 4) {
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
#else
		twl4030_bci_battery_update_status(di);
		schedule_delayed_work(&di->twl4030_bci_monitor_work, 10 * HZ);
#endif

#if BK_BATT
	schedule_delayed_work(&di->twl4030_bk_bci_monitor_work, msecs_to_jiffies(400)); 
#endif
	return 0;
}
#else
#define twl4030_bci_battery_suspend	NULL
#define twl4030_bci_battery_resume	NULL
#endif 

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
