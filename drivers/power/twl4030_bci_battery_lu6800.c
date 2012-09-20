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
#include <linux/slab.h>
#include <linux/i2c/twl.h>
#include <linux/power_supply.h>
#include <linux/i2c/twl4030-madc.h>
#include <linux/rtc.h>

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

#include <linux/irq.h>		// set_irq_type()


#define TIME_UPDATE_ATCBC   20 
#define TIME_READY_UPDATE_ATCBC     10
#define TIMEOUT_UPDATE_ATCBC    25

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

#if defined(CONFIG_HUB_CHARGING_IC) && defined(CONFIG_HUB_MUIC)
#else
#define CURR_STEP_SIZE		147
#define CURR_PSR_R1		44
#define CURR_PSR_R2		80
#endif

#define BK_VOLT_STEP_SIZE	441
#define BK_VOLT_PSR_R		100

#define ENABLE		1
#define DISABLE		0

s32 muic_device_detection(s32 upon_irq); //20110103 ks.kwon@lge.com

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

    int         send_at_cbc_time;
    int         enable_batt_rst; 
	
	struct power_supply	ac;
	struct power_supply	usb;
// 20100624 taehwan.kim@lge.com  To add Hub battery support[END_LGE]

	struct power_supply	bat;
	struct power_supply	bk_bat;
	struct power_supply usb_bat;
	struct delayed_work	twl4030_bci_monitor_work;
#if BK_BATT
	struct delayed_work	twl4030_bk_bci_monitor_work;
#endif
};
// 20100624 taehwan.kim@lge.com  To add Hub battery support[START_LGE]

// S[, 20120725, mannsik.chung@lge.com, Enable charging by fake mode.
#define UNLIMITED_TEMP_VAL	0xA4
#define UNLIMITED_TEMP_HIGH	390
#define UNLIMITED_TEMP_LOW	-50
static int thermal_fakemode = 0;
// E], 20120725, mannsik.chung@lge.com, Enable charging by fake mode.

static struct twl4030_bci_device_info *refer_di;
static int twl4030battery_temperature(void);
static int twl4030battery_voltage(void);
int check_no_lcd(void); //check test mode in bare board state of factory process
int get_reset_status(void);
void set_trickle_charge_monitor(int enable); //20101104 taehwan.kim@lge.com to fix trickle chg



static int start_monitor = 0; //20101104 taehwan.kim@lge.com to fix trickle chg
static int gauge_initialized = 0; //20101120 taehwan.kim flag for gauge value is stired in first time.
static int battery_changed = 0;
static int gauge_checked = 0;
static int strange_gauge_applied_counter = 0; //20101202 taehwan.kim@lge.com when lower gauge value is allowed after several time tried, during charging
static TYPE_MUIC_MODE muic_mode_batt = MUIC_NONE;
static unsigned long old_time;
static int dummy_gauge = 0;
static int CP_gauge_started = 0;
static int check_store_gauge = 200;
static int test_temp = 0; //will be deletec in 1 week
static int test_temp1 = 0; //will be deletec in 1 week
static int batt_volt_table[101] = {
4131    ,4125    ,4120    ,4093    ,4080    ,4078    ,4070    ,4067    ,4050    ,4040    ,
4037    ,4031    ,4026    ,4018    ,4007    ,4001    ,3992    ,3991    ,3985    ,3974    ,
3965    ,3960    ,3955    ,3944    ,3940    ,3928    ,3926    ,3925    ,3917    ,3909    ,
3902    ,3895    ,3884    ,3880    ,3871    ,3860    ,3852    ,3843    ,3832    ,3827    ,
3816    ,3811    ,3809    ,3805    ,3799    ,3796    ,3792    ,3791    ,3788    ,3785    ,
3783    ,3779    ,3776    ,3774    ,3773    ,3767    ,3763    ,3762    ,3761    ,3759    ,
3756    ,3754    ,3749    ,3746    ,3744    ,3742    ,3741    ,3740    ,3737    ,3734    ,
3730    ,3729    ,3727    ,3723    ,3721    ,3719    ,3716    ,3713    ,3710    ,3706    ,
3704    ,3703    ,3701    ,3694    ,3688    ,3685    ,3681    ,3671    ,3665    ,3657    ,
3650    ,3643    ,3639    ,3637    ,3633    ,3625    ,3597    ,3578    ,3544    ,3470    ,
3358  
};
/* not use
static int batt_chg_table[101] = {
4190    ,4186    ,4183    ,4179    ,4174    ,4169    ,4168    ,4163    ,4162    ,4159    ,
4157    ,4155    ,4147    ,4143    ,4139    ,4133    ,4127    ,4121    ,4111    ,4106    ,
4103    ,4094    ,4093    ,4087    ,4074    ,4070    ,4069    ,4068    ,4064    ,4058    ,
4052    ,4045    ,4034    ,4029    ,4025    ,4020    ,4016    ,4014    ,4005    ,4004    ,
4001    ,3999    ,3996    ,3992    ,3988    ,3986    ,3982    ,3980    ,3973    ,3970    ,
3969    ,3967    ,3966    ,3962    ,3960    ,3958    ,3958    ,3957    ,3957    ,3957    ,
3955    ,3953    ,3951    ,3949    ,3948    ,3946    ,3945    ,3945    ,3945    ,3939    ,
3939    ,3936    ,3927    ,3918    ,3917    ,3913    ,3911    ,3910    ,3904    ,3894    ,
3892    ,3886    ,3882    ,3878    ,3871    ,3863    ,3862    ,3854    ,3853    ,3851    ,
3849    ,3847    ,3846    ,3839    ,3833    ,3828    ,3809    ,3768    ,3715    ,3550    ,
3481    
};
*/
static int dummy_ref_voltage = 998;

//2011-03-23 ntyeongon.moon@lge.com get muic INT_STAT  VBUS [START_LGE]
extern int is_muic_mvbus_on();
//2011-03-23 ntyeongon.moon@lge.com get muic INT_STAT  VBUS [END_LGE]


// 20100624 taehwan.kim@lge.com  To add Hub battery support[END_LGE]

static int LVL_1, LVL_2, LVL_3, LVL_4;

static int read_bci_val(u8 reg_1);
static inline int clear_n_set(u8 mod_no, u8 clear, u8 set, u8 reg);

/** @brief  To check battery distortion
 *  *   @author taehwan.kim@lge.com
 *   *     @date   2010.12.07
 *    *       */
static int get_dummy_gauge(int voltage)
{
    int ret = 0;
    if (voltage > 3750) ret = 11;
    else if (voltage > 3600) ret = 14; //+ 17 -> 3561 (3690)  +9 -> 3632 (3813)
    else if (voltage > 3550) ret = 15;
    else if (voltage > 3500) ret = 10;
    else if (voltage > 3450) ret = 8;
    else if (voltage > 3400) ret = 7;
    else if (voltage > 3300) ret = 4;
    else if (voltage > 3100) ret = 0;
    else ret = 0;
    printk("[BATTERY] dummy_gauge_voltage = %d (%d)\n",ret,voltage);
    return ret;
}

/** @brief  To check battery distortion
 *  *   @author taehwan.kim@lge.com
 *   *     @date   2010.12.07
 *    *       */
static int diff_gauge_voltage(int gauge, int voltage)
{
    int ret = 0, i, avg_volt,  match_value = gauge, volt_diff = -2100; 
    if ((muic_mode_batt != MUIC_NONE)&&(gauge_initialized == 1)
            &&(gauge_checked == 1)&&(get_charging_ic_status() != 0)) {
        voltage = voltage - 150;
    }
    if ((gauge <= 75) && (gauge >= 37)) {
        ret = gauge - (voltage - 3729)*20/100-45;
    }
    else if ((gauge <= 83) && (gauge >= 25)) ret = gauge - (voltage - 3700)*50/230-30;
    else if ((gauge <= 100) && (gauge >= 4)) ret = gauge - (voltage - 3580)*100/480;
    else if (gauge == 0) 
    {
        if (voltage > 3520) ret = 80;
		return ret; //ntyeongon.moon 2011-03-26 quick start
    }
    else if (gauge <= 3) 
    {
        if (voltage > 3630) ret = 80;
    }
    else if (gauge >= 100) ret = 100 - (voltage - 3580)*100/600;
    else ret = 0;
#if defined(CONFIG_HUB_MUIC)
	if ((muic_mode_batt == MUIC_NONE) && (get_muic_mode() == MUIC_NONE)) // && (gauge > 4) && (gauge < 90))
#elif defined(CONFIG_MUIC)
	if ((muic_mode_batt == MUIC_NONE) && (muic_get_mode() == MUIC_NONE)) // && (gauge > 4) && (gauge < 90))
#endif
    {
        avg_volt = (refer_di->voltage_uV + voltage)/2;
        dummy_ref_voltage = avg_volt;
        for (i = 0; i <101; i++)
        {
            if (batt_volt_table[i] - avg_volt <= 0)
            {
                if (batt_volt_table[i] - avg_volt >= volt_diff) 
                {
                    volt_diff = batt_volt_table[i] - avg_volt;
                    match_value = 100 - i;
                }
            }            
        }
        printk("[BATTERY] extra batt table avg_volt %d match_value %d volt_diff %d \n",avg_volt, match_value, volt_diff);
        ret = gauge - match_value;
    }

    printk("[BATTERY] diff_gauge_voltage = %d (%d V %d)\n",ret,voltage,gauge);
    return ret;
}

/** @brief  To check battery distortion
 *  *   @author taehwan.kim@lge.com
 *   *     @date   2010.12.07
 *    *       */
static void checck_battery_changed(void)
{
    int gauge_diff = 0;
    int battery_reset = 0;
    int temp_volt = 3700;
#if defined(CONFIG_HUB_MUIC)
    if ((refer_di->enable_batt_rst == 0) && (gauge_initialized == 1)
            && (strange_gauge_applied_counter > 2) 
            && (muic_mode_batt != MUIC_NONE) && (get_muic_mode() != MUIC_NONE))
#elif defined(CONFIG_MUIC)
    if ((refer_di->enable_batt_rst == 0) && (gauge_initialized == 1)
            && (strange_gauge_applied_counter > 2) 
            && (muic_mode_batt != MUIC_NONE) && (muic_get_mode() != MUIC_NONE) 
			&& (!thermal_fakemode)) // 20120725, mannsik.chung@lge.com, Enable charging by fake mode.
#endif
    {
        temp_volt = refer_di->voltage_uV; 
        if (temp_volt > 4150) temp_volt = 4150;
        gauge_diff = diff_gauge_voltage(refer_di->battery_capacity, temp_volt); 
        if (refer_di->battery_capacity < 95)
	    { 
			if ((gauge_diff > 55)||(gauge_diff < -55)) battery_reset = 1;
        }
        else if (refer_di->battery_capacity >= 95)
		{
			if ((gauge_diff > 43)||(gauge_diff < -43)) battery_reset = 1;
        }
        printk("[BATTERY] monitor diff = %d \n",gauge_diff);

        if (battery_reset == 1)
        {
            printk("first changed detector work! \n");
            if ((refer_di->battery_present == 1)&&(!check_no_lcd())) charging_ic_deactive(); 
            msleep(70);
            temp_volt = twl4030battery_voltage();
            if (temp_volt > 4150) temp_volt = 4150;
            gauge_diff = diff_gauge_voltage(refer_di->battery_capacity, temp_volt);
            if ((gauge_diff > 60)||(gauge_diff < -55)) 
            {
                battery_changed = 1;
                refer_di->send_at_cbc_time = 0;
                refer_di->enable_batt_rst = 1;
                strange_gauge_applied_counter = 0;
                printk("battery changed detect! \n");
                if ((refer_di->battery_present == 1)&&(!check_no_lcd())) charging_ic_deactive();
                test_temp = 2;
				#if defined(CONFIG_HUB_MUIC)
                if (get_muic_mode() != MUIC_NONE) dummy_gauge = get_dummy_gauge(temp_volt);
				#elif defined(CONFIG_MUIC)
				if (muic_get_mode() != MUIC_NONE) dummy_gauge = get_dummy_gauge(temp_volt);
				#endif
            }
        }
        //else refer_di->enable_batt_rst = 0;
    }
}


/** @brief  To check battery present from other drivers
  @author taehwan.kim@lge.com
  @date   2010.11.25
  */
int check_battery_present(void)
{
    int batt_present = 1;

    if (twl4030battery_temperature() < 700) batt_present = 1;
    else batt_present = 0;

    return batt_present;
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

    if (ret == MUIC_AP_UART) val = 1;
    else if ((ret == MUIC_CP_UART)||(ret == MUIC_CP_USB)||(ret == MUIC_CP_DOWNLOAD)) val = 2;
    else if (((ret == MUIC_NONE)&&(twl4030battery_temperature() > 700))||(check_no_lcd())) {
         val = 1; //to skip battery waiting animation
         printk("[Charging Animation : Array Test Mode detected \n");
    }
    else val = 0;
    CP_gauge_started = 1;
#elif defined(CONFIG_MUIC)
    ret = muic_get_mode();

    if (ret == MUIC_AP_UART) val = 1;
    else if ((ret == MUIC_CP_UART)||(ret == MUIC_CP_USB)||(ret == MUIC_CP_DOWNLOAD)) val = 2;
    else if (((ret == MUIC_NONE)&&(twl4030battery_temperature() > 700))||(check_no_lcd())) {
         val = 1; //to skip battery waiting animation
         printk("[Charging Animation : Array Test Mode detected \n");
    }
    else val = 0;
    CP_gauge_started = 1;
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
    TYPE_MUIC_MODE muic_mode = MUIC_NONE;

#if defined(CONFIG_HUB_MUIC)
    muic_mode = get_muic_mode();
#elif defined(CONFIG_MUIC)
	muic_mode = muic_get_mode();
#endif
    sscanf(buf, "%d", &val);
    batt_raw_cap = val; //((val * 100) / 96);
    
    CP_gauge_started = 1;
    printk("[BATTERY] CP send AT+CBC = %d to AP %d \n", val, batt_raw_cap);
   
    if ((batt_raw_cap >= 998001) && (batt_raw_cap <= 998100))
    {
        check_store_gauge = batt_raw_cap - 998000;
        printk("get success stored gauge %d \n",check_store_gauge);
    }

    if ((batt_raw_cap >= 0) && (batt_raw_cap <= 104)){
        if ((gauge_initialized == 0)&&(refer_di->enable_batt_rst == 1))
        {
            printk("start to check stored value compare \n");
            if ((batt_raw_cap + dummy_gauge - check_store_gauge < 15)
                    &&(batt_raw_cap + dummy_gauge - check_store_gauge > -15))
            {
                printk("check_store_gauge(%d), refer_di->battery_capacity (%d) \n",check_store_gauge, refer_di->battery_capacity);
                dummy_gauge = check_store_gauge - batt_raw_cap;
                printk("store gauge value is applied %d with %d \n",check_store_gauge,dummy_gauge);
            }
#if 0 //prepare code for future 
            else if ((dummy_ref_voltage != 998) && (get_muic_mode()== MUIC_NONE) && (muic_mode_batt == MUIC_NONE) 
                && (batt_raw_cap > 4) && (batt_raw_cap < 90))
            {
                dummy_gauge = 0 - diff_gauge_voltage(batt_raw_cap, dummy_ref_voltage);                            
            }
            dummy_ref_voltage = 998;
#endif
        }
        refer_di->enable_batt_rst = 0;
        refer_di->send_at_cbc_time = 0;
        battery_changed = 0;
        if (strange_gauge_applied_counter == 0) {
            old_time = jiffies;
            strange_gauge_applied_counter = 1;
        }
        else if((jiffies - old_time > 15 * HZ) &&(strange_gauge_applied_counter < 265)) {
            strange_gauge_applied_counter = strange_gauge_applied_counter + (int)((jiffies - old_time)/(15 * HZ));
            printk("strange_gauge_applied_counter = %d \n",strange_gauge_applied_counter);
            old_time = jiffies;
        }
        if (strange_gauge_applied_counter > 265) strange_gauge_applied_counter = 265;

        if ((refer_di->battery_capacity != batt_raw_cap + dummy_gauge)||(batt_raw_cap < 3)){
            if (dummy_gauge < 0) dummy_gauge++;
            else if (dummy_gauge > 0) dummy_gauge--;
            else dummy_gauge = 0;
        }

        if ((gauge_initialized == 0)||(strange_gauge_applied_counter > 260) ||(strange_gauge_applied_counter < 5)) 
        {
				gauge_initialized = 1;
				refer_di->battery_capacity = batt_raw_cap + dummy_gauge;
        }
        else if (batt_raw_cap + dummy_gauge > refer_di->battery_capacity) 
        {
            if (muic_mode != MUIC_NONE) 
            {
                if ((batt_raw_cap + dummy_gauge - refer_di->battery_capacity > 4))
                    refer_di->battery_capacity = batt_raw_cap + dummy_gauge;
                else refer_di->battery_capacity++;
            }
        }
        else if (batt_raw_cap + dummy_gauge < refer_di->battery_capacity)
        {
            if (muic_mode == MUIC_NONE) 
            {
                if (refer_di->battery_capacity - batt_raw_cap - dummy_gauge > 4)
                    refer_di->battery_capacity = batt_raw_cap + dummy_gauge;
                else refer_di->battery_capacity--;

            }
        }
        //20101231, taehwan.kim@lge.com, Changed for Hub [START_LGE]
        checck_battery_changed();
        //20101231, taehwan.kim@lge.com, Changed for Hub [END_LGE]
        //20101023, taehwan.kim@lge.com, Battery AT command [START_LGE]
        power_supply_changed(&refer_di->bat);    
        cancel_delayed_work(&refer_di->twl4030_bci_monitor_work);
        schedule_delayed_work(&refer_di->twl4030_bci_monitor_work, msecs_to_jiffies(100));
        //20101023, taehwan.kim@lge.com, Battery AT command [END_LGE]
    }
    else if (batt_raw_cap == 998) {
        refer_di->enable_batt_rst = 0;
        refer_di->send_at_cbc_time = TIME_UPDATE_ATCBC;
    }

	return count;
}

/** @brief  To send AT%FUELRST for gauge IC calibration in CP via AT-command
    @author taehwan.kim@lge.com
    @date   2010.0520.
*/
static ssize_t fuel_cal_show(struct device *dev,
			     struct device_attribute *attr,
			     char *buf )
{
    int val;
    static int gauge_cal_check = 1;
    int deactive_volt = 0;
    int gauge_diff = 0;

    TYPE_MUIC_MODE muic_mode = MUIC_NONE;
#if defined(CONFIG_HUB_MUIC)
    muic_mode = get_muic_mode();
#elif defined(CONFIG_MUIC)
    muic_mode = muic_get_mode();
#endif
    if ((refer_di->battery_present == 1) && !check_no_lcd() && (!thermal_fakemode)) // 20120725, mannsik.chung@lge.com, Enable charging by fake mode.
		charging_ic_deactive();    

    if (gauge_initialized == 0) //for charging mode test
    {
        mdelay(250);
        deactive_volt = twl4030battery_voltage();
        if (deactive_volt > 4130) deactive_volt = 4000;
        val = ((twl4030battery_voltage() - 3100)*100)/1009;
        if (val >= 100) test_temp1++;
        printk("[BATTERY] charging mode read voltage %d in volt %d [cnt %d] \n",val,deactive_volt, test_temp1);
    }
    else 
    {
        mdelay(30);
        deactive_volt = twl4030battery_voltage();
        gauge_diff = diff_gauge_voltage(refer_di->battery_capacity, deactive_volt);
        if (((gauge_diff > 25) || (gauge_diff < -15)) && !(refer_di->battery_capacity >= 96 && deactive_volt > 4100)
                && !(refer_di->battery_capacity <= 2 && deactive_volt <3500)
				&& (!thermal_fakemode)) // 20120725, mannsik.chung@lge.com, Enable charging by fake mode.
		{
            gauge_cal_check = 0;
        }
        printk("[BATTERY] success to access fuel_cal_show \n"); 
        if (gauge_cal_check == 0) 
        {
            val = 1;
            printk("[BATTERY] Gauge Calibration started in fuel_cal_show\n");
            if (refer_di->battery_present == 1) charging_ic_deactive();
            gauge_cal_check = 1;
            refer_di->enable_batt_rst = 1;
            gauge_initialized = 0;
            test_temp = 1;
            dummy_gauge = get_dummy_gauge(deactive_volt);
        }
        else
        {
            if (refer_di->battery_capacity > 90 && refer_di->battery_capacity <= 95 
                    && deactive_volt > 4030 && muic_mode == MUIC_NONE) {
                dummy_gauge = 96 - refer_di->battery_capacity;
                refer_di->battery_capacity = refer_di->battery_capacity + dummy_gauge;
                printk("[BATTERY] dummy %d for cradle charger \n",dummy_gauge); 
            }
            val = 0;
        }
        gauge_checked = 1;
    }
    return sprintf(buf, "%d\n", val);
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
                0xF, REG_BCIIMR2A);
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
                0xF, REG_BCIIMR1A);
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

//ntyeongon.moon 20110511 Temperature Based B-prj [START]
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
//#endif
//ntyeongon.moon 20110511 Temperature Based B-prj [END]


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
		//ntyeongon.moon 20110511 Temperature Based B-prj [START]
		//temp = 280;
		temp = 200;
        //ntyeongon.moon 20110511 Temperature Based B-prj [END]
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
    int volt = read_bci_val(T2_BATTERY_VOLT);

    return (volt * VOLT_STEP_SIZE) / VOLT_PSR_R;
}

/*
 * Return the battery current
 * Or < 0 on failure.
 */
 #if defined(CONFIG_HUB_CHARGING_IC) && defined(CONFIG_HUB_MUIC)
 #else
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

/*
 * Returns an integer value, that means,
 * NO_PW_CONN  no power supply is connected
 * AC_PW_CONN  if the AC power supply is connected
 * USB_PW_CONN  if the USB power supply is connected
 * AC_PW_CONN + USB_PW_CONN if USB and AC power supplies are both connected
 *
 * Or < 0 on failure.
 */

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
    POWER_SUPPLY_PROP_ATCOMMAND_SET, //20101023, taehwan.kim@lge.com, Battery AT command
	POWER_SUPPLY_PROP_TEMP_CONTROL, // 20120725, mannsik.chung@lge.com, Enable charging by fake mode.
};

#if BK_BATT
static enum power_supply_property twl4030_bk_bci_battery_props[] = {
	POWER_SUPPLY_PROP_VOLTAGE_NOW,
};
#endif

static enum power_supply_property twl4030_ac_usb_bci_battery_props[] = {
	POWER_SUPPLY_PROP_ONLINE,
};

static void
twl4030_bk_bci_battery_read_status(struct twl4030_bci_device_info *di)
{
	di->bk_voltage_uV = twl4030backupbatt_voltage();
}

#if BK_BATT
static void twl4030_bk_bci_battery_work(struct work_struct *work)
{
	struct twl4030_bci_device_info *di = container_of(work,
		struct twl4030_bci_device_info,
		twl4030_bk_bci_monitor_work.work);

	twl4030_bk_bci_battery_read_status(di);
	schedule_delayed_work(&di->twl4030_bk_bci_monitor_work, HZ*10);
}
#endif

static void twl4030_bci_battery_read_status(struct twl4030_bci_device_info *di)
{
    //20101023, taehwan.kim@lge.com, Changed for Hub [START_LGE]
    //ntyeongon.moon 20110511 Temperature Based B-prj [START]
    //close 20110503 di->temp_C = (di->temp_C *5 +(775 - (twl4030battery_temperature()*650)/453))/6;
    di->temp_C = twl4030battery_temperature(); //20110503
    //ntyeongon.moon 20110511 Temperature Based B-prj [END]

//20120125, hrgo@ubiquix.com , When is suddenly Over temperature,  retry read temperature. [START]
	if (di->temp_C >600 && di->temp_C != 0xBA00E00){
		msleep(10);
		printk("[BATTERY] When suddenly OverTemp. temp_C : %d \n", di->temp_C);
		di->temp_C = twl4030battery_temperature();
		printk("[BATTERY] When suddenly OverTemp. retry temp_C : %d \n", di->temp_C);
	}
//20120125, hrgo@ubiquix.com , When is suddenly Over temperature,  retry read temperature. [END]

    if (gauge_initialized == 0) {
        //if (di->battery_capacity <= 99) 
        di->battery_capacity = ((twl4030battery_voltage() - 3100)*100)/1099;
        //else if (di->battery_capacity == 100) di->battery_capacity = 100;
    }
#if defined(CONFIG_HUB_CHARGING_IC) && defined(CONFIG_HUB_MUIC)
    {
        int batt_volt, trickle_chg_max = 4198, trickle_chg_min = 4140;
        max8922_status chr_ic_status = CHARGING_IC_DEACTIVE;
        muic_mode_batt = MUIC_NONE;
        muic_mode_batt = get_muic_mode();
        //if (muic_mode_batt != MUIC_NONE) di->charge_status = POWER_SUPPLY_STATUS_CHARGING;
        //else di->charge_status = POWER_SUPPLY_STATUS_DISCHARGING;

        /* temp code - should be remove */
        batt_volt = twl4030battery_voltage();
        //ntyeongon.moon 20110504 change initialized level. [START]
		//prevent power on under critical low voltage in power off charging.
		if( di->voltage_uV==3600 && batt_volt <=3250)
		{
			di->voltage_uV = batt_volt;
		}
		//ntyeongon.moon 20110504 change initialized level for critical low level [END]
        if (batt_volt > 2400) {
            if ((di->battery_capacity >= 100) && (di->battery_capacity <= 104))
                di->voltage_uV = ((2*di->voltage_uV)+ batt_volt)/3;
            else di->voltage_uV = ((6*di->voltage_uV)+ batt_volt)/7;		
        }
        /* read battery voltage via fuel gage */
        chr_ic_status = get_charging_ic_status();

        /*temperature limit when temperature lower than -5 degree or higher than 42
         * dgree, charging voltage should be up to 4.0V*/
        if (!check_no_lcd() && (di->temp_C < -50 || di->temp_C > 500))
        {
            printk("[BATTERY] Start Temperature charging limit up to 4.0V \n");
            trickle_chg_max = 4170;
            trickle_chg_min = 4080;
        }   

        /* recharging algorithm */
        if (!check_no_lcd() && (di->temp_C < -170 || di->temp_C > 550)) 
        {
            printk("[BATTERY] STOP charging by high temperature \n");
            if (refer_di->battery_present == 1) charging_ic_deactive();
        }
        else if ((di->voltage_uV <= trickle_chg_min || di->battery_capacity < 99) &&
                chr_ic_status == CHARGING_IC_DEACTIVE &&		/* charging IC is deactive */
                muic_mode_batt != MUIC_NONE && !check_no_lcd())					/* device connected */
        {
            switch (muic_mode_batt)
            {
                case MUIC_UNKNOWN:
                    break;
                case MUIC_AP_UART:
                case MUIC_CP_UART:
                case MUIC_CP_USB:
                case MUIC_CP_DOWNLOAD: //20101125 taehwan.kim@lge.com for 910k
                    if (refer_di->battery_present == 1) charging_ic_deactive();
                    if ((refer_di->battery_present == 1) && (refer_di->enable_batt_rst == 0)) charging_ic_set_factory_mode();
                    break;

                case MUIC_NA_TA:
                case MUIC_LG_TA:
                case MUIC_HCHH:
                case MUIC_INVALID_CHG:
                    if (refer_di->battery_present == 1) charging_ic_deactive();
                    if ((CP_gauge_started == 1) && (refer_di->battery_present == 1) 
                            && (refer_di->enable_batt_rst == 0)) charging_ic_set_ta_mode();
                    break;
                case MUIC_AP_USB:
                case MUIC_ILLEGAL_CHG:
                    if (refer_di->battery_present == 1) charging_ic_deactive();
                    if ((CP_gauge_started == 1) && (refer_di->battery_present == 1) 
                            && (refer_di->enable_batt_rst == 0)) charging_ic_set_usb_mode(); 
                    break;

                default :
                    //if (refer_di->battery_present == 1) charging_ic_deactive();
                    break;
            }
            //start_monitor = 0; //20101104 taehwan.kim@lge.com to fix trickle chg
        }
        else if (((di->voltage_uV >= trickle_chg_max) && (di->battery_capacity > 98)) 
                && (di->battery_capacity == 100) 
                && chr_ic_status != CHARGING_IC_DEACTIVE && 
                ((muic_mode_batt >= MUIC_NA_TA && muic_mode_batt <= MUIC_INVALID_CHG)||(muic_mode_batt == MUIC_ILLEGAL_CHG)) && 
                !check_no_lcd())
        {
            start_monitor = 1;
            if (refer_di->battery_present == 1) charging_ic_deactive();
            printk("[BATTERY] charger deactivate for trickle charge \n");
        }
        else if (di->battery_capacity == 0)
        {
            start_monitor = 1;
            printk("[BATTERY] low battery monitor is started\n");
        }
        else if (muic_mode_batt == MUIC_NONE || check_no_lcd()) {
            start_monitor = 0;
        }
        else if (muic_mode_batt != MUIC_NONE && di->voltage_uV >= 4100 && !check_no_lcd())
        {
            printk("[BATTERY] continued trickling charging monitor \n");   
            start_monitor = 1;
        }
        else {
            start_monitor = 0; 
        }
        printk("[BATTERY] +++ vol:%d, chr:%d, muic: %d, temp %d bk %d cap %d cnt %d Qrst %d dum %d no %d\n", 
                di->voltage_uV, chr_ic_status, muic_mode_batt, di->temp_C, 
                twl4030backupbatt_voltage(), di->battery_capacity, di->send_at_cbc_time, 
                test_temp,dummy_gauge, strange_gauge_applied_counter);
#if 1 /* by hskim@mnbt.co.kr */
		/* by usnoh@ubiquix.com : for debugging */
		{
			struct timespec ts;
			struct rtc_time tm;
			extern void rtc_time_to_tm_for_kor(unsigned long time, struct rtc_time *tm);
			getnstimeofday(&ts);
			rtc_time_to_tm_for_kor(ts.tv_sec, &tm);
			pr_info("Current time : "
				"%d-%02d-%02d %02d:%02d:%02d\n", 
				tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
				tm.tm_hour, tm.tm_min, tm.tm_sec);
		}
		{
			extern int meminfo_proc_show_simple( void );
			meminfo_proc_show_simple();
		}
#endif
	}

    /* hub do not use BCI block. so we cannot measure battery current */
    di->current_uA = 0;
#elif defined(CONFIG_HUB_CHARGING_IC) && defined(CONFIG_MUIC)
    {
        int batt_volt, trickle_chg_max = 4198, trickle_chg_min = 4140;
        max8922_status chr_ic_status = CHARGING_IC_DEACTIVE;
        muic_mode_batt = MUIC_NONE;
		muic_mode_batt = muic_get_mode();
        //if (muic_mode_batt != MUIC_NONE) di->charge_status = POWER_SUPPLY_STATUS_CHARGING;
        //else di->charge_status = POWER_SUPPLY_STATUS_DISCHARGING;

        /* temp code - should be remove */
        batt_volt = twl4030battery_voltage();
        //ntyeongon.moon 20110504 change initialized level. [START]
		//prevent power on under critical low voltage in power off charging.
		if( di->voltage_uV==3600 && batt_volt <=3250)
		{
			di->voltage_uV = batt_volt;
		}
		//ntyeongon.moon 20110504 change initialized level for critical low level [END]
        if (batt_volt > 2400) {
            if ((di->battery_capacity >= 100) && (di->battery_capacity <= 104))
                di->voltage_uV = ((2*di->voltage_uV)+ batt_volt)/3;
            else di->voltage_uV = ((6*di->voltage_uV)+ batt_volt)/7;		
        }
        /* read battery voltage via fuel gage */
        chr_ic_status = get_charging_ic_status();

        /*temperature limit when temperature lower than -5 degree or higher than 42
         * dgree, charging voltage should be up to 4.0V*/
        if (!check_no_lcd() && (di->temp_C < -50 || di->temp_C > 500))
        {
            printk("[BATTERY] Start Temperature charging limit up to 4.0V \n");
            trickle_chg_max = 4170;
            trickle_chg_min = 4080;
        }   

        /* recharging algorithm */
        if (!check_no_lcd() && (di->temp_C < -170 || di->temp_C > 550) &&
			di->temp_control != UNLIMITED_TEMP_VAL) // 20120725, mannsik.chung@lge.com, Enable charging by fake mode. 
        {
            printk("[BATTERY] STOP charging by high temperature \n");
            if (refer_di->battery_present == 1) charging_ic_deactive();
        }
        else if ((di->voltage_uV <= trickle_chg_min || di->battery_capacity < 99) &&
                chr_ic_status == CHARGING_IC_DEACTIVE &&	/* charging IC is deactive */
                muic_mode_batt != MUIC_NONE && !check_no_lcd() &&	/* device connected */
				di->temp_control != UNLIMITED_TEMP_VAL)	// 20120725, mannsik.chung@lge.com, Enable charging by fake mode.
        {
            switch (muic_mode_batt)
            {
                case MUIC_UNKNOWN:
                    break;
                case MUIC_AP_UART:
                case MUIC_CP_UART:
                case MUIC_CP_USB:
                case MUIC_CP_DOWNLOAD: //20101125 taehwan.kim@lge.com for 910k
                    if (refer_di->battery_present == 1) charging_ic_deactive();
                    if ((refer_di->battery_present == 1) && (refer_di->enable_batt_rst == 0)) charging_ic_set_factory_mode();
                    break;

                case MUIC_NA_TA:
                case MUIC_LG_TA:
                case MUIC_TA_1A:
                case MUIC_INVALID_CHG:
                    if (refer_di->battery_present == 1) charging_ic_deactive();
                    if ((CP_gauge_started == 1) && (refer_di->battery_present == 1) 
                            && (refer_di->enable_batt_rst == 0)) charging_ic_set_ta_mode();
                    break;
                case MUIC_AP_USB:
                case MUIC_ILLEGAL_CHG:
                    if (refer_di->battery_present == 1) charging_ic_deactive();
                    if ((CP_gauge_started == 1) && (refer_di->battery_present == 1) 
                            && (refer_di->enable_batt_rst == 0)) charging_ic_set_usb_mode(); 
                    break;

                default :
                    //if (refer_di->battery_present == 1) charging_ic_deactive();
                    break;
            }
            //start_monitor = 0; //20101104 taehwan.kim@lge.com to fix trickle chg
        }
        else if (((di->voltage_uV >= trickle_chg_max) && (di->battery_capacity > 98)) 
                && (di->battery_capacity == 100) 
                && chr_ic_status != CHARGING_IC_DEACTIVE && 
                ((muic_mode_batt >= MUIC_NA_TA && muic_mode_batt <= MUIC_INVALID_CHG)||(muic_mode_batt == MUIC_ILLEGAL_CHG)) && 
                !check_no_lcd() &&
				di->temp_control != UNLIMITED_TEMP_VAL) // 20120725, mannsik.chung@lge.com, Enable charging by fake mode.
        {
            start_monitor = 1;
            if (refer_di->battery_present == 1) charging_ic_deactive();
            printk("[BATTERY] charger deactivate for trickle charge \n");
        }
        else if (di->battery_capacity == 0)
        {
            start_monitor = 1;
            printk("[BATTERY] low battery monitor is started\n");
        }
        else if (muic_mode_batt == MUIC_NONE || check_no_lcd()) {
            start_monitor = 0;
        }
        else if (muic_mode_batt != MUIC_NONE && di->voltage_uV >= 4100 && !check_no_lcd())
        {
            printk("[BATTERY] continued trickling charging monitor \n");   
            start_monitor = 1;
        }
        else {
            start_monitor = 0; 
        }
        printk("[BATTERY] +++ vol:%d, chr:%d, muic: %d, temp %d bk %d cap %d cnt %d Qrst %d dum %d no %d\n", 
                di->voltage_uV, chr_ic_status, muic_mode_batt, di->temp_C, 
                twl4030backupbatt_voltage(), di->battery_capacity, di->send_at_cbc_time, 
                test_temp,dummy_gauge, strange_gauge_applied_counter);
#if 1 /* by hskim@mnbt.co.kr */
		/* by usnoh@ubiquix.com : for debugging */
		{
			struct timespec ts;
			struct rtc_time tm;
			extern void rtc_time_to_tm_for_kor(unsigned long time, struct rtc_time *tm);
			getnstimeofday(&ts);
			rtc_time_to_tm_for_kor(ts.tv_sec, &tm);
			pr_info("Current time : "
				"%d-%02d-%02d %02d:%02d:%02d\n", 
				tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
				tm.tm_hour, tm.tm_min, tm.tm_sec);
		}
		{
			extern int meminfo_proc_show_simple( void );
			meminfo_proc_show_simple();
		}
#endif
	}

    /* hub do not use BCI block. so we cannot measure battery current */
    di->current_uA = 0;

#else
    di->voltage_uV = twl4030battery_voltage();
    di->current_uA = twl4030battery_current();
    di->capacity = twl4030battery_capacity(di);
#endif
    //20101023, taehwan.kim@lge.com, Changed for Hub [END_LGE]
}


static void twl4030_bci_battery_update_status(struct twl4030_bci_device_info *di)
{
	twl4030_bci_battery_read_status(di);

//   twl4030_bci_battery_read_status(di); //close ntyeongon.moon 2011-03-25 dupulication.
    di->charge_status = POWER_SUPPLY_STATUS_UNKNOWN;

	/* S[, mannsik.chung@lge.com, If charging IC is deactivated, charge_status should be NOT_CHARGING. */
	if (get_charging_ic_status() == CHARGING_IC_DEACTIVE)
	{
		di->charge_status = POWER_SUPPLY_STATUS_NOT_CHARGING;
	}
	else
	/* E], mannsik.chung@lge.com, If charging IC is deactivated, charge_status should be NOT_CHARGING. */

	//2011-04-01 ntyeongon.moon @lge.com Add full chage status update [START]
	#if defined(CONFIG_HUB_MUIC)
    if ((di->battery_capacity >= 96)&&(get_muic_mode() != MUIC_NONE)) 
	#elif defined(CONFIG_MUIC)
	if ((di->battery_capacity >= 96)&&(muic_get_mode() != MUIC_NONE)) 
	#endif
	   di->charge_status = POWER_SUPPLY_STATUS_FULL;
   //2011-04-01 ntyeongon.moon @lge.com Add full chage status update [END]
    else if (power_supply_am_i_supplied(&di->bat))
        di->charge_status = POWER_SUPPLY_STATUS_CHARGING;
    //20101012 taehwan.kim@lge.com Add full chage status update [START_LGE]
    //close else if ((di-> battery_capacity >= 96)&&(get_muic_mode() != MUIC_NONE)) di->charge_status = POWER_SUPPLY_STATUS_FULL;
    //20101012 taehwan.kim@lge.com Add full chage status update [END_LGE]
    else
        di->charge_status = POWER_SUPPLY_STATUS_DISCHARGING;
}


static void battery_info_change_update(struct twl4030_bci_device_info *di)
{
    volatile u8		is_changed = 0;

    twl4030_bci_battery_update_status(di);

    /* FIXME: temp range - need to modify */
    if((di->temp_C <= di->previous_temp_C -100) || (di->temp_C >= di->previous_temp_C + 100))	/* +- 0.2 C */
    {
        di->previous_temp_C = di->temp_C;
        is_changed = 1;
    }

    if((di->voltage_uV <= di->previous_voltage_uV -20) || (di->voltage_uV >= di->previous_voltage_uV + 20))	/* +- 20mV */
    {
        printk("[BATTERY] voltage changed \n");
        di->previous_voltage_uV = di->voltage_uV;
        is_changed = 1;
    }

    if(di->charge_status != di->previous_charge_status)
    {
        printk("[BATTERY] charge state changed \n");
        di->previous_charge_status = di->charge_status;
        is_changed = 1;
    }
    if ((di->send_at_cbc_time == TIME_READY_UPDATE_ATCBC) || (di->send_at_cbc_time > TIME_UPDATE_ATCBC)) 
    {
        printk("[BATTERY] cbc time changed \n");
        is_changed = 1;
    }

    if(di->battery_capacity != di->previous_battery_capacity)
    {
        printk("[BATTERY] capacity changed \n");
        di->previous_battery_capacity = di->battery_capacity;
        is_changed = 1;
    }

    if (di->enable_batt_rst == 1)
    {
        printk("[BATTERY] gauge reset changed \n");
        is_changed = 1;
    }

    if(is_changed)
    {
        msleep(50); //20101104 taehwan.kim@lge.com to reduce voltage change update
        power_supply_changed(&di->bat);
    }
    else
    {
        //20101023, taehwan.kim@lge.com, Battery AT command [START_LGE]
        if (refer_di->enable_batt_rst == 0) {
            if (di->voltage_uV > 3400) {
                cancel_delayed_work(&di->twl4030_bci_monitor_work);
                schedule_delayed_work(&di->twl4030_bci_monitor_work, HZ*7);	/* 10S */
            }
            else {
                cancel_delayed_work(&di->twl4030_bci_monitor_work);                                            
                schedule_delayed_work(&di->twl4030_bci_monitor_work, HZ*2); /* 1S */
            }
        }
        else
        {
            cancel_delayed_work(&di->twl4030_bci_monitor_work);
            schedule_delayed_work(&di->twl4030_bci_monitor_work, HZ*2);	/* 1S */
        }
        //20101023, taehwan.kim@lge.com, Battery AT command [START_LGE]
    }
}

void charger_state_update_by_other(void)
{
    //20101023, taehwan.kim@lge.com, Battery AT command [START_LGE]
    cancel_delayed_work(&refer_di->twl4030_bci_monitor_work);
    schedule_delayed_work(&refer_di->twl4030_bci_monitor_work, 0);
    //20101023, taehwan.kim@lge.com, Battery AT command [END_LGE]
}
EXPORT_SYMBOL(charger_state_update_by_other);


static void twl4030_bci_battery_work(struct work_struct *work)
{
	struct twl4030_bci_device_info *di = container_of(work,
			struct twl4030_bci_device_info, twl4030_bci_monitor_work.work);

    //20101023, taehwan.kim@lge.com, Battery AT command [START_LGE]
    di->send_at_cbc_time++; 
    battery_info_change_update(di);
    if (di->send_at_cbc_time == TIMEOUT_UPDATE_ATCBC) di->send_at_cbc_time = 0;
//	printk(KERN_ERR "[BATTERY] muic_device_detection(0)\n");
#if 0 //ntyreongon.moon 2011-03-18 close - battery charging icon problem
    if (di->send_at_cbc_time == 2 || di->send_at_cbc_time == 12 ) muic_device_detection(0); //20110103 ks.kwon@lge.com MUIC detection update
#endif	
    //20101023, taehwan.kim@lge.com, Battery AT command [END_LGE]
}


#define to_twl4030_bci_device_info(x) container_of((x), \
			struct twl4030_bci_device_info, bat);

static void twl4030_bci_battery_external_power_changed(struct power_supply *psy)
{
	struct twl4030_bci_device_info *di = to_twl4030_bci_device_info(psy);

    cancel_delayed_work(&di->twl4030_bci_monitor_work);
    schedule_delayed_work(&di->twl4030_bci_monitor_work, msecs_to_jiffies(600));
}

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

	switch (psp) {
		case POWER_SUPPLY_PROP_STATUS:
#if 1 /* 20110802, mschung@ubiquix.com, Show battery status in chargerlogo. */
			val->intval = di->charge_status;
#else
            if ((gauge_initialized == 0)||(gauge_checked == 0)||(battery_changed == 1)) val->intval = POWER_SUPPLY_STATUS_UNKNOWN;
            else val->intval = di->charge_status;
#endif
			
            return 0;

		case POWER_SUPPLY_PROP_VOLTAGE_NOW:
			val->intval = di->voltage_uV;
			break;
		case POWER_SUPPLY_PROP_CURRENT_NOW:
			val->intval = di->current_uA;
			break;
		case POWER_SUPPLY_PROP_TEMP:
			// S[, 20120725, mannsik.chung@lge.com, Enable charging by fake mode.
			if (di->temp_control == UNLIMITED_TEMP_VAL) 
			{
				if (di->temp_C > UNLIMITED_TEMP_HIGH)
					val->intval = UNLIMITED_TEMP_HIGH;
				else if (di->temp_C < UNLIMITED_TEMP_LOW)
					val->intval = UNLIMITED_TEMP_LOW;
				else
					val->intval = di->temp_C;
			}
			// E], 20120725, mannsik.chung@lge.com, Enable charging by fake mode.
			else if (di->battery_present == 1) val->intval = di->temp_C;
            else val->intval = 150;		
			break;
		case POWER_SUPPLY_PROP_ONLINE:
#if defined(CONFIG_HUB_MUIC)
            muic_mode = get_muic_mode();
            if ( muic_mode == MUIC_LG_TA || muic_mode == MUIC_NA_TA || muic_mode == MUIC_HCHH 
                    || muic_mode == MUIC_INVALID_CHG || muic_mode == MUIC_ILLEGAL_CHG)
                val->intval = POWER_SUPPLY_TYPE_MAINS;
            else if ((muic_mode == MUIC_CP_USB)||(muic_mode == MUIC_CP_UART)
                    ||(muic_mode == MUIC_AP_UART)||(muic_mode == MUIC_CP_DOWNLOAD))
                val->intval = POWER_SUPPLY_TYPE_MAINS;
            else if (muic_mode == MUIC_AP_USB)
                val->intval = POWER_SUPPLY_TYPE_USB;
            else
                val->intval = 0;
//2011-03-14 ntyeongon.moon@lge.com get muic INT_STAT  VBUS [START_LGE]
			 if(!is_muic_mvbus_on()) val->intval = 0;
//2011-03-14 ntyeongon.moon@lge.com get muic INT_STAT  VBUSVBUS [END_LGE]

			 
			//printk("+++ Battery charger info. muic: %d, val:%d\n", muic_mode, val->intval);	
			break;
#elif defined(CONFIG_MUIC)
            muic_mode = muic_get_mode();
            if ( muic_mode == MUIC_LG_TA || muic_mode == MUIC_NA_TA || muic_mode == MUIC_TA_1A 
                    || muic_mode == MUIC_INVALID_CHG || muic_mode == MUIC_ILLEGAL_CHG && chr_ic_status != CHARGING_IC_DEACTIVE )
                val->intval = POWER_SUPPLY_TYPE_MAINS;
            else if ((muic_mode == MUIC_CP_USB)||(muic_mode == MUIC_CP_UART)
                    ||(muic_mode == MUIC_AP_UART)||(muic_mode == MUIC_CP_DOWNLOAD) && chr_ic_status != CHARGING_IC_DEACTIVE )
                val->intval = POWER_SUPPLY_TYPE_MAINS;
            else if (muic_mode == MUIC_AP_USB && chr_ic_status != CHARGING_IC_DEACTIVE )
                val->intval = POWER_SUPPLY_TYPE_USB;
            else
                val->intval = 0;
//2011-03-14 ntyeongon.moon@lge.com get muic INT_STAT  VBUS [START_LGE]
			 if(!is_muic_mvbus_on()) val->intval = 0;
//2011-03-14 ntyeongon.moon@lge.com get muic INT_STAT  VBUSVBUS [END_LGE]

			 
			//printk("+++ Battery charger info. muic: %d, val:%d\n", muic_mode, val->intval);	
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
#if defined(CONFIG_HUB_MUIC)
            muic_mode = get_muic_mode();
            if (check_no_lcd() == 1) 
            {
                di->battery_present = 0;
                val->intval = 1;
            }
            else if ((muic_mode == MUIC_CP_USB)||(muic_mode == MUIC_CP_UART)
                    ||(muic_mode == MUIC_AP_UART)||(muic_mode == MUIC_CP_DOWNLOAD)) 
            {
                di->battery_present = 0;
                val->intval = 1;
            }
            else if ((system_rev >= 2) && (twl4030battery_temperature() < 700)) 
            {
                di->battery_present = 1;
                val->intval = 1;
            }
            else if ((system_rev == 1) && (twl4030battery_temperature() < 40)) 
            {
                di->battery_present = 1;
                val->intval = 1;
            }
            else 
            {
                di->battery_present = 0;
                val->intval = 0;
            }
            break;		
#elif defined(CONFIG_MUIC)
            muic_mode = muic_get_mode();
            if (check_no_lcd() == 1) 
            {
                di->battery_present = 0;
                val->intval = 1;
            }
            else if ((muic_mode == MUIC_CP_USB)||(muic_mode == MUIC_CP_UART)
                    ||(muic_mode == MUIC_AP_UART)||(muic_mode == MUIC_CP_DOWNLOAD) && chr_ic_status != CHARGING_IC_DEACTIVE ) 
            {
                di->battery_present = 0;
                val->intval = 1;
            }
            else if ((system_rev >= 2) && (twl4030battery_temperature() < 700)) 
            {
                di->battery_present = 1;
                val->intval = 1;
            }
            else if ((system_rev == 1) && (twl4030battery_temperature() < 40)) 
            {
                di->battery_present = 1;
                val->intval = 1;
            }
            else 
            {
                di->battery_present = 0;
                val->intval = 0;
            }
            break;
#endif
            //20101023, taehwan.kim@lge.com, Battery AT command [START_LGE]   
        case POWER_SUPPLY_PROP_ATCOMMAND_SET:
            if (di->send_at_cbc_time >= TIME_UPDATE_ATCBC) { //send AT%CBC in every 40s
                printk("POWER_SUPPLY_PROP_ATCOMMAND_SET 3 \n");
                val->intval = 3;        
            }
            else if (di->send_at_cbc_time >= TIME_READY_UPDATE_ATCBC) {
                //printk("ready to send AT CBC and release block ATCMD \n");
                val->intval = 2;
            }
#if 1   //if you want to send AT%FUELRST from framework, enable with a modification 
            else if ((di->enable_batt_rst == 1)&&(gauge_checked == 1)) {
                printk("POWER_SUPPLY_PROP_ATCOMMAND_SET 1 \n");
                val->intval = 1;
                gauge_initialized = 0;
                di->send_at_cbc_time = TIME_UPDATE_ATCBC- 8;
            }
#endif
            else {
                //If temperature is not changed, no at command is sent to CP.
                //Refer to BatteryService.java for AT%BATTEMP
                //printk("POWER_SUPPLY_PROP_ATCOMMAND_SET 0 \n");
                val->intval = 0;
            }
            break;
            //20101023, taehwan.kim@lge.com, Battery AT command [END_LGE]        
            /* LGE_CHANGE_E [taehwan.kim@lge.com] on 2010-01-13, <Test for present>> */

		case POWER_SUPPLY_PROP_CAPACITY:
#if defined(CONFIG_HUB_MUIC)
            muic_mode = get_muic_mode();
            //di->battery_capacity = ((di->voltage_uV - 3300)*100)/850;
            if (di->battery_capacity < 0)  val->intval = 0;
            else if (di->battery_capacity >= 96)  val->intval = 100;
            else if (di->battery_capacity >= 90) val->intval = ((di->battery_capacity * 100) / 96)+1; 
            else val->intval =  ((di->battery_capacity * 100) / 96);			
			break;
#elif defined(CONFIG_MUIC)
            muic_mode = muic_get_mode();
            //di->battery_capacity = ((di->voltage_uV - 3300)*100)/850;
            if (di->battery_capacity < 0)  val->intval = 0;
            else if (di->battery_capacity >= 96)  val->intval = 100;
            else if (di->battery_capacity >= 90) val->intval = ((di->battery_capacity * 100) / 96)+1; 
            else val->intval =  ((di->battery_capacity * 100) / 96);			
			break;
#endif
			/* FIXME : It depends on H/W specific */
		case POWER_SUPPLY_PROP_HEALTH:
            // 20100816 taehwan.kim@lge.com Temp info for hub [START_LGE] 
#if defined(CONFIG_HUB_MUIC)
            muic_mode = get_muic_mode();
            if (check_no_lcd() == 1) 
                val->intval = 1;
            else if (twl4030battery_temperature() >= 700)
                val->intval = 1;
            else if(((muic_mode != MUIC_CP_USB)&&(muic_mode != MUIC_CP_UART)
                        &&(muic_mode != MUIC_AP_UART)&&(muic_mode != MUIC_CP_DOWNLOAD)) 
                    && (di->temp_C > 600))  /* Overheat */	
                val->intval = 2;
            else if(di->voltage_uV > 4400)		/* Over voltage */
                val->intval = 4;
            else if((di->battery_present == 1) && (di->temp_C < -170))		/* Cold */
                val->intval = 6;
            // 20100816 taehwan.kim@lge.com Temp info for hub [END_LGE]    
            else
                val->intval = 1;

            break;
#elif defined(CONFIG_MUIC)
            muic_mode = muic_get_mode();
// S[, 20120725, mannsik.chung@lge.com, Enable charging by fake mode.
			if (di->temp_control == UNLIMITED_TEMP_VAL)
				val->intval = 1;
			else
// E], 20120725, mannsik.chung@lge.com, Enable charging by fake mode.
            if (check_no_lcd() == 1) 
                val->intval = 1;
            else if (twl4030battery_temperature() >= 700)
                val->intval = 1;
            else if(((muic_mode != MUIC_CP_USB)&&(muic_mode != MUIC_CP_UART)
                        &&(muic_mode != MUIC_AP_UART)&&(muic_mode != MUIC_CP_DOWNLOAD)) 
                    && (di->temp_C > 600))  /* Overheat */	
                val->intval = 2;
            else if(di->voltage_uV > 4400)		/* Over voltage */
                val->intval = 4;
            else if((di->battery_present == 1) && (di->temp_C < -170))		/* Cold */
                val->intval = 6;
            // 20100816 taehwan.kim@lge.com Temp info for hub [END_LGE]    
            else
                val->intval = 1;

            break;
#endif
		case POWER_SUPPLY_PROP_TECHNOLOGY:
			val->intval = 2;	/* Fixed value : Li-ion */				
			break;

// S[, 20120725, mannsik.chung@lge.com, Enable charging by fake mode.
		case POWER_SUPPLY_PROP_TEMP_CONTROL:
			val->intval = (int)di->temp_control;
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

	TYPE_MUIC_MODE muic_mode = MUIC_NONE;
	max8922_status chr_ic_status = CHARGING_IC_DEACTIVE;
	chr_ic_status = get_charging_ic_status();

	switch (psp) {
		case POWER_SUPPLY_PROP_ONLINE:
/* LGE_CHANGE_S [sookyoung.kim@lge.com] 2010-03-19 */
#if defined(CONFIG_HUB_MUIC)
			muic_mode = get_muic_mode();

            if ( muic_mode == MUIC_LG_TA || muic_mode == MUIC_NA_TA || muic_mode == MUIC_HCHH 
                    || muic_mode == MUIC_INVALID_CHG || muic_mode == MUIC_ILLEGAL_CHG)
                val->intval = 1;
            else if ((muic_mode == MUIC_CP_USB)||(muic_mode == MUIC_CP_UART)
                    ||(muic_mode == MUIC_AP_UART)||(muic_mode == MUIC_CP_DOWNLOAD))
                val->intval = 1;
            else
                val->intval = 0;			
			break;
#elif defined(CONFIG_MUIC)
			muic_mode = muic_get_mode();

            if ( muic_mode == MUIC_LG_TA || muic_mode == MUIC_NA_TA || muic_mode == MUIC_TA_1A 
                    || muic_mode == MUIC_INVALID_CHG || muic_mode == MUIC_ILLEGAL_CHG && chr_ic_status != CHARGING_IC_DEACTIVE)
                val->intval = 1;
            else if ((muic_mode == MUIC_CP_USB)||(muic_mode == MUIC_CP_UART)
                    ||(muic_mode == MUIC_AP_UART)||(muic_mode == MUIC_CP_DOWNLOAD && chr_ic_status != CHARGING_IC_DEACTIVE))
                val->intval = 1;
            else
                val->intval = 0;			
			break;
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
#elif defined(CONFIG_MUIC)
			muic_mode = muic_get_mode();
			if ( muic_mode == MUIC_AP_USB)
				val->intval = 1;
			else
				val->intval = 0;
			break;
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

	printk("[twl4030_bci_battery_lu6800] probe - (Fixed error process, Prevent exception)\n");

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
	di->bat.external_power_changed = twl4030_bci_battery_external_power_changed;
	di->bat.set_charged = NULL;	
    di->charge_status = POWER_SUPPLY_STATUS_UNKNOWN;	

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

    di->previous_voltage_uV = 0;
    di->previous_temp_C = 0;
    di->previous_charge_status = POWER_SUPPLY_STATUS_UNKNOWN;
    di->temp_C = 200; 
    //2010.11.04 taehwan.kim@lge.com to initialize battery value [START_LGE]
    di->battery_capacity = ((twl4030battery_voltage()-3100)*100)/1100; //Too low gauge can cause 0% shut down
    if (di->battery_capacity < 0)  di->battery_capacity = 0;
    else if ( di->battery_capacity > 100)  di->battery_capacity = 100;
    di->previous_battery_capacity = 55;
    di->enable_batt_rst = 0;
    di->battery_present = 0;
    di->send_at_cbc_time = 0;
    di->voltage_uV = 3600;
    //2010.09.06 taehwan.kim@lge.com to initialize battery value [END_LGE]	
	twl4030charger_ac_en(DISABLE);
	twl4030charger_usb_en(DISABLE);
	twl4030battery_hw_level_en(DISABLE);
	twl4030battery_hw_presence_en(DISABLE);
	platform_set_drvdata(pdev, di);

    refer_di = di;

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

//--[[ LGE_UBIQUIX_MODIFIED_START : shyun@ubiquix.com [2012.07.23] - twl4030_bci_monitor_work feature first.
#if 0
	ret = power_supply_register(&pdev->dev, &di->bat);
	if (ret) {
		dev_dbg(&pdev->dev, "failed to register main battery\n");
		goto batt_failed;
	}

	INIT_DELAYED_WORK_DEFERRABLE(&di->twl4030_bci_monitor_work, twl4030_bci_battery_work);
    schedule_delayed_work(&di->twl4030_bci_monitor_work, msecs_to_jiffies(700)); //taehwan.kim@lge.com add delay for secure ops
#else
	INIT_DELAYED_WORK_DEFERRABLE(&di->twl4030_bci_monitor_work, twl4030_bci_battery_work);
    schedule_delayed_work(&di->twl4030_bci_monitor_work, msecs_to_jiffies(700)); //taehwan.kim@lge.com add delay for secure ops

	ret = power_supply_register(&pdev->dev, &di->bat);
	if (ret) {
		dev_dbg(&pdev->dev, "failed to register main battery\n");
		goto batt_failed;
	}
#endif
//--]] LGE_UBIQUIX_MODIFIED_END : shyun@ubiquix.com [2012.07.23]- twl4030_bci_monitor_work feature first.

/* LGE_CHANGE_S [taehwan.kim@lge.com] 2010-3-12, android NOT need bk battery voltage*/
#if BK_BATT
    ret = power_supply_register(&pdev->dev, &di->bk_bat);
    if (ret) {
         dev_dbg(&pdev->dev, "failed to register backup battery\n");
         goto bk_batt_failed;
    }

//--[[ LGE_UBIQUIX_MODIFIED_START : shyun@ubiquix.com [2012.07.05] - BK_BATT Featuring error.
    INIT_DELAYED_WORK_DEFERRABLE(&di->twl4030_bk_bci_monitor_work, twl4030_bk_bci_battery_work);
    schedule_delayed_work(&di->twl4030_bk_bci_monitor_work, HZ*1);
#else
	printk("[SHYUN] [%s] - [%d] [NO BK_BATT]\n", __func__, __LINE__);
#endif //BK_BATT

//    INIT_DELAYED_WORK_DEFERRABLE(&di->twl4030_bk_bci_monitor_work, twl4030_bk_bci_battery_work);
//    schedule_delayed_work(&di->twl4030_bk_bci_monitor_work, HZ*1);
//--]] LGE_UBIQUIX_MODIFIED_END : shyun@ubiquix.com [2012.07.05]- BK_BATT Featuring error.

     //#endif //BK_BATT
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

//--[[ LGE_UBIQUIX_MODIFIED_START : shyun@ubiquix.com [2012.07.23] - Add the debug msg & fixed error process.
	printk("[twl4030_bci_battery_lu6800] [%s] - [%d] [NORMAL return]\n", __func__, __LINE__);
	
	return 0;

usb_online_failed:
	printk("[twl4030_bci_battery_lu6800] [%s] - [%d] [usb_online_failed]\n", __func__, __LINE__);
	power_supply_unregister(&di->usb);
ac_online_failed:
	printk("[twl4030_bci_battery_lu6800] [%s] - [%d] [ac_online_failed]\n", __func__, __LINE__);
	power_supply_unregister(&di->ac);
#if BK_BATT
bk_batt_failed:
	printk("[twl4030_bci_battery_lu6800] [%s] - [%d] [bk_batt_failed]\n", __func__, __LINE__);
	power_supply_unregister(&di->bk_bat);
#endif
batt_failed:
	printk("[twl4030_bci_battery_lu6800] [%s] - [%d] [batt_failed]\n", __func__, __LINE__);
	free_irq(irq, di);
chg_irq_fail:
	printk("[twl4030_bci_battery_lu6800] [%s] - [%d] [chg_irq_fail]\n", __func__, __LINE__);
	irq = platform_get_irq(pdev, 1);
	free_irq(irq, NULL);
batt_irq_fail:
	printk("[twl4030_bci_battery_lu6800] [%s] - [%d] [batt_irq_fail]\n", __func__, __LINE__);
voltage_setup_fail:
	printk("[twl4030_bci_battery_lu6800] [%s] - [%d] [voltage_setup_fail]\n", __func__, __LINE__);

	twl4030charger_ac_en(DISABLE);
	twl4030charger_usb_en(DISABLE);
	twl4030battery_hw_level_en(DISABLE);
	twl4030battery_hw_presence_en(DISABLE);
	kfree(di);

	printk("[twl4030_bci_battery_lu6800] [%s] - [%d] [voltage_setup_fail ret = %d]\n", __func__, __LINE__, ret);
//--]] LGE_UBIQUIX_MODIFIED_END : shyun@ubiquix.com [2012.07.23]- Add the debug msg & fixed error process.

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

    if (start_monitor){
        printk("[BATTERY] Trickle_charge_monitor on\n");
        set_trickle_charge_monitor(1);
    }

    //di->charge_status = POWER_SUPPLY_STATUS_UNKNOWN;  //20101116 taehwan.kim@lge.com
	cancel_delayed_work(&di->twl4030_bci_monitor_work);    
#if BK_BATT
	cancel_delayed_work(&di->twl4030_bk_bci_monitor_work);
#endif
	return 0;
}

static int twl4030_bci_battery_resume(struct platform_device *pdev)
{
	struct twl4030_bci_device_info *di = platform_get_drvdata(pdev);
    //di->send_at_cbc_time = TIME_UPDATE_ATCBC-2; //20101204 taehwan.kim@lge.com update in every resume time 
    schedule_delayed_work(&di->twl4030_bci_monitor_work, 0); 
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

