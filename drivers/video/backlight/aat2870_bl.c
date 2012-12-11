/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*-
 *  * vim: set ts=8 sw=8 et tw=80:
 *   
* drivers/video/backlight/aat2870_bl.c
 *
 * Copyright (C) 2010 LGE, Inc
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

/*******************************************************************************
 * Keep coding stype necessarily 
 * -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*-
 *  * vim: set ts=8 sw=8 et tw=80:
 *
 ******************************************************************************/ 
#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/backlight.h>
#include <linux/fb.h>
#include <linux/i2c.h>
#include <linux/string.h>
#include <asm/system.h>
#include <mach/hardware.h>
#include <mach/gpio.h>
#include <linux/leds.h>
#include <plat/board.h>
#include <linux/hrtimer.h>

#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif

#define MODULE_NAME		"aat2870"


#ifndef DEBUG
#define DEBUG
//#undef DEBUG
#endif

#ifdef DEBUG
#define DBG(fmt, args...) 				\
	printk(KERN_DEBUG "[%s] %s(%d): " 		\
		fmt, MODULE_NAME, __func__, __LINE__, ## args); 
#else	/* DEBUG */
#define DBG(...) 
#endif

#define ERR(fmt, args...) 				\
	printk(KERN_ERR "[%s] %s(%d): " 		\
		fmt, MODULE_NAME, __func__, __LINE__, ## args); 


 struct i2c_client *aat2870_i2c_client;
EXPORT_SYMBOL(aat2870_i2c_client); //20101222 seven.kim@lge.com for touch ESD recovery

//extern void bd2802_resume_for_lcd(); //2011205 kyungyoon.kim@lge.com lcd resume speed
extern void bd2802_resume_for_lcd(void); //2011205 kyungyoon.kim@lge.com lcd resume speed, // 20120213 taeju.park@lge.com To delete compile warning, specifying 0 arguments


extern unsigned int system_rev;

#define MAX_BRIGHTNESS 		31
#define DEFAULT_BRIGHTNESS 	14 
#define USER_LVL_MAX		255

#define AAT2870_REG0		0x00
#define AAT2870_REG1		0x01
#define AAT2870_REG2		0x02
#define AAT2870_REG3		0x03
#define AAT2870_REG4		0x04
#define AAT2870_REG5		0x05
#define AAT2870_REG6		0x06
#define AAT2870_REG7		0x07
#define AAT2870_REG8		0x08
#define AAT2870_REG9		0x09
#define AAT2870_REG10		0x0A
#define AAT2870_REG11		0x0B
#define AAT2870_REG12		0x0C
#define AAT2870_REG13		0x0D
#define AAT2870_REG14		0x0E
#define AAT2870_REG15		0x0F
#define AAT2870_REG16		0x10
#define AAT2870_REG17		0x11
#define AAT2870_REG18		0x12
#define AAT2870_REG19		0x13
#define AAT2870_REG20		0x14
#define AAT2870_REG21		0x15
#define AAT2870_REG22		0x16
#define AAT2870_REG23		0x17
#define AAT2870_REG24		0x18
#define AAT2870_REG25		0x19
#define AAT2870_REG26		0x1A
#define AAT2870_REG27		0x1B
#define AAT2870_REG28		0x1C
#define AAT2870_REG29		0x1D
#define AAT2870_REG30		0x1E
#define AAT2870_REG31		0x1F
#define AAT2870_REG32		0x20
#define AAT2870_REG33		0x21
#define AAT2870_REG34		0x22
#define AAT2870_REG35		0x23
#define AAT2870_REG36		0x24
#define AAT2870_REG37		0x25
#define AAT2870_REG38		0x26

#define I2C_NO_REG           0xFF

#define MAX_LDO_REG		4

#define BL_ON			1
#define BL_OFF			0

static u8 mirror_reg[MAX_LDO_REG] = {0,};


#if 1 //changhyun.han@lge.com, 20100119 ,added the optimized brightness mode
enum {
	NORMAL_MODE,
	ALS_MODE,
	POWERSAVE_MODE,
	OPTIMIZE_MODE,
};

#else
enum {
	NORMAL_MODE,
	ALS_MODE,
};

#endif

enum {
	SLEEP_STATE,	// backlight off
	WAKE_STATE,	// backlight on
};
	
struct aat2870_device {
	struct i2c_client *client;
	struct backlight_device *bl_dev;
	struct led_classdev *led;
	struct hrtimer als_timer;
	struct work_struct  als_work;
	struct workqueue_struct	 	*als_wq;	

	int level;
	int state;
	int mode;

	int als_register14;
	int als_register16;
	/* 20110216 seven.kim@lge.com to check AAT2870 LDO_ABCD_EN_REG [START] */
	int als_register38;
	/* 20110216 seven.kim@lge.com to check AAT2870 LDO_ABCD_EN_REG [START] */
	int als_level;
    //20100205 kyungyoon.kim@lge.com for LCD resume speed[START]	
	int bl_resumed;
    //20100205 kyungyoon.kim@lge.com for LCD resume speed[END]

#ifdef CONFIG_HAS_EARLYSUSPEND
	struct early_suspend early_suspend; 
#endif
};


/*
Bit3 Bit2 Bit1 Bit0 VLDO(V)
0 0 0 0 1.2
0 0 0 1 1.3
0 0 1 0 1.5
0 0 1 1 1.6
0 1 0 0 1.8
0 1 0 1 2.0
0 1 1 0 2.2
0 1 1 1 2.5
1 0 0 0 2.6
1 0 0 1 2.7
1 0 1 0 2.8
1 0 1 1 2.9
1 1 0 0 3.0
1 1 0 1 3.1
1 1 1 0 3.2
1 1 1 1 3.3
*/
typedef enum {
	VLDO_1_2V,	/* 0 */
	VLDO_1_3V,
	VLDO_1_5V,
	VLDO_1_6V,
	VLDO_1_8V,
	VLDO_2_0V,	/* 5 */
	VLDO_2_2V,
	VLDO_2_5V,
	VLDO_2_6V,
	VLDO_2_7V,
	VLDO_2_8V,	/* 10 */
	VLDO_2_9V,
	VLDO_3_0V,
	VLDO_3_1V,
	VLDO_3_2V,
	VLDO_3_3V,	/* 15 */

	VLDO_MAX,	/* 16 */
} AAT2870_VLDO;

typedef enum {
	ENLDO_A,	/* 0 */
	ENLDO_B,
	ENLDO_C,
	ENLDO_D,

	ENLDO_MAX,	/* 5 */
} AAT2870_ENLDO;

//20100709 kyungtae.oh@lge.com early_suspend [START_LGE]
#ifdef CONFIG_HAS_EARLYSUSPEND
static int early_bl_timer = 1;
static int early_bl_value = 0;
#endif
//20100709 kyungtae.oh@lge.com early_suspend [END_LGE]

#define LCD_CP_EN				62
#define HUB_PANEL_LCD_RESET_N	34
#define HUB_PANEL_LCD_CS		54
#define AAT2870_I2C_BL_NAME	"aat2870_i2c_bl"

#define LDO_AB_LEVEL_REG	0x24
#define LDO_CD_LEVEL_REG	0x25
#define LDO_ABCD_EN_REG		0x26

/* B-Prj LCD Backlight ALS [kyungyoon.kim@lge.com] 2010-11-22 */
struct lcd_ctrl_data {
	unsigned char reg;
	unsigned char val;
};

//shim.shunggoo@lge.com,20121106 ,rollback from KT to EU GB value for ALC
static struct lcd_ctrl_data normal_to_als_mode[]=
{
	{AAT2870_REG18,0x12},	// 4.05 mA //chagne 0x12 < 0x0E
	{AAT2870_REG19,0x15},	// 4.725mA
	{AAT2870_REG20,0x17},	// 5.175 mA
	{AAT2870_REG21,0x19},	// 5.625 mA
	{AAT2870_REG22,0x1A},	// 5.85mA
	{AAT2870_REG23,0x1B},	// 6.075mA
	{AAT2870_REG24,0x1D},	// 6.525mA
	{AAT2870_REG25,0x20},	// 7.2  mA
	{AAT2870_REG26,0x21},	// 7.425mA
	{AAT2870_REG27,0x23},	// 7.875mA
	{AAT2870_REG28,0x25},	// 8.325 mA
	{AAT2870_REG29,0x26},	// 8.55 mA
	{AAT2870_REG30,0x27},	// 8.775 mA
	{AAT2870_REG31,0x28},	// 9 mA
	{AAT2870_REG32,0x29},	// 9.225 mA
	{AAT2870_REG33,0x2A},	// 9.45 mA
	{AAT2870_REG14,0x77},
	{AAT2870_REG15,0x01},
	{AAT2870_REG16,0x95},
	{AAT2870_REG0,0xFF},
	{I2C_NO_REG, 0x00}  /* End of array */

};
//shim.shunggoo@lge.com,20121106 ,rollback from KT to EU GB value for ALC
//changhyun.han@lge.com, 20100119 ,added the optimized brightness mode
static struct lcd_ctrl_data normal_to_als_optimize_mode[]=
{
	{AAT2870_REG18,0x1A}, /* ALS current setting  5.85  mA */
	{AAT2870_REG19,0x22}, /* ALS current setting  7.65  mA */
	{AAT2870_REG20,0x24}, /* ALS current setting  8.10  mA */
	{AAT2870_REG21,0x28}, /* ALS current setting  9.00  mA */
	{AAT2870_REG22,0x29}, /* ALS current setting  9.225 mA */
	{AAT2870_REG23,0x2D}, /* ALS current setting 10.125 mA */
	{AAT2870_REG24,0x2F}, /* ALS current setting 10.575 mA */
	{AAT2870_REG25,0x30}, /* ALS current setting 10.80  mA */
	{AAT2870_REG26,0x3B}, /* ALS current setting 13.275 mA */
	{AAT2870_REG27,0x3E}, /* ALS current setting 13.95  mA */
	{AAT2870_REG28,0x41}, /* ALS current setting 14.625 mA */
	{AAT2870_REG29,0x41}, /* ALS current setting 14.625 mA */
	{AAT2870_REG30,0x41}, /* ALS current setting 14.625 mA */
	{AAT2870_REG31,0x42}, /* ALS current setting 14.85  mA */
	{AAT2870_REG32,0x42}, /* ALS current setting 14.85  mA */
	{AAT2870_REG33,0x5C}, /* ALS current setting 20.70  mA */
	{AAT2870_REG14,0x67},
	{AAT2870_REG15,0x01},
#if 1
	{AAT2870_REG16,0x97},
#else  // kyungrae.jo, 2011-03-11, HW TEST
	{AAT2870_REG16,0xB7},
#endif
	{AAT2870_REG0,0xFF},
	{I2C_NO_REG, 0x00}  /* End of array */
};

static struct lcd_ctrl_data als_to_normal_mode[]=
{
	{AAT2870_REG14,0x26},
	{AAT2870_REG15,0x06},
	{AAT2870_REG0,0xFF},
	{I2C_NO_REG, 0x00}  /* End of array */
};
/* B-Prj LCD Backlight ALS [kyungyoon.kim@lge.com] 2010-11-22 */

static const struct i2c_device_id aat2870_bl_id[] = {
	{ AAT2870_I2C_BL_NAME, 0 },
	{ },	// 20100526 sookyoung.kim@lge.com
};

static int aat2870_write_reg(struct i2c_client *client, 
			     unsigned char reg,
			     unsigned char val);
static int aat2870_read_reg(struct i2c_client *client,
			    unsigned char reg,
			    unsigned char *ret);

static void aat2870_change_mode(struct i2c_client *client, int mode, int force);
unsigned int cur_main_lcd_level = DEFAULT_BRIGHTNESS;
static unsigned int saved_main_lcd_level = DEFAULT_BRIGHTNESS;


/* 20110216 seven.kim@lge.com to check AAT2870 LDO_ABCD_EN_REG [START] */
int aat2870_ldo_status(void);
/* 20110216 seven.kim@lge.com to check AAT2870 LDO_ABCD_EN_REG [START] */

/* 20110218 seven.kim@lge.com to contorl AAT2870 sleep/resume state machine [START] */
static unsigned char g_AAT2870_State_Machine = 0;

enum {
	AAT2870_EARLY_SUSPEND_STATE,
	AAT2870_SUSPEND_STATE,
	AAT2870_RESUME_STATE,
	AAT2870_LATE_RESUME_STATE,	
};
/* 20110218 seven.kim@lge.com to contorl AAT2870 sleep/resume state machine [START] */

static int aat2870_write_reg(struct i2c_client *client, 
			     unsigned char reg,
			     unsigned char val)
{
	struct aat2870_device *dev = NULL;	
	int ret;
	int status = 0;

	dev = (struct aat2870_device *) i2c_get_clientdata(client);

	if (client == NULL) {
		ERR("client is null\n");
		return -ENXIO;
	}

	if (reg ==AAT2870_REG14)
		dev->als_register14=val;

	if (reg ==AAT2870_REG16)
		dev->als_register16=val;
	/* 20110216 seven.kim@lge.com to check AAT2870 LDO_ABCD_EN_REG [START] */
	if (reg == LDO_ABCD_EN_REG)
		dev->als_register38 = val;
	/* 20110216 seven.kim@lge.com to check AAT2870 LDO_ABCD_EN_REG [END] */
	
	ret = i2c_smbus_write_byte_data(client, reg, val);
	if (ret != 0) {
		status = -EIO;
		ERR("fail to write(reg=0x%x,val=0x%x)\n", reg, val);
	}

	return status;
}


static int aat2870_read_reg(struct i2c_client *client, 
			    unsigned char reg, 
			    unsigned char *pval)
{
	int ret;
	int status = 0;

	if (client == NULL) { 
		ERR("client is null\n");
		return -ENXIO;
	}

	ret = i2c_smbus_read_byte_data(client, reg);
	if (ret < 0) {
		status = -EIO;
		ERR("Fail to read reg = 0x%x, val = 0x%x)\n", reg, *pval);
	}

	*pval = ret;

	return status;
}

static void aat2870_hw_reset(void)
{
	/*20110216 seven.kim@lge.com to adjust android sleep flow [START] */
	if(aat2870_ldo_status() == 0) //all LDOs are off
   {	
	gpio_set_value(LCD_CP_EN, 0);
	udelay(10);
	gpio_set_value(LCD_CP_EN, 1);
	udelay(80); /*optimised value for low-temperature condition*/
}
	/*20110216 seven.kim@lge.com to adjust android sleep flow [END] */
}

static void aat2870_device_init(struct i2c_client *client) 
{
////	gpio_request(LCD_CP_EN, "lcd bl");
////	gpio_direction_output(LCD_CP_EN, 1);

	aat2870_hw_reset();
}

void aat2870_ldo_enable(unsigned char num, int enable) 
{
	unsigned char org;

	aat2870_read_reg(aat2870_i2c_client, LDO_ABCD_EN_REG, &org);

	if (enable) {
		aat2870_write_reg(aat2870_i2c_client, 
				LDO_ABCD_EN_REG,
				org | (1<< num));
	}
	else {
		aat2870_write_reg(aat2870_i2c_client, 
				LDO_ABCD_EN_REG, 
				org & ~(1<<num));
	}
}
EXPORT_SYMBOL(aat2870_ldo_enable);

static void aat2870_ldo_read(u8 reg , u8 *val)
{
	if (reg < 0 || reg > MAX_LDO_REG - 1) {
		DBG("ldo invalid register access\n");
		return;
	}

	*val = i2c_smbus_read_byte_data(aat2870_i2c_client, reg);

	DBG("ldo Reg read 0x%X: Val = 0x%X\n", (u8)reg, (u8)*val);

	return;
}


//--[[ LGE_UBIQUIX_MODIFIED_START : shyun@ubiquix.com [2012.08.22] - Touch-ldo function.
u8 aat2870_touch_ldo_read(u8 reg)
{
	int ret = -1;
	u8 val;

	ret = aat2870_read_reg(aat2870_i2c_client, reg, &val);
	if(ret < 0)
		printk("[%s-%d] aat2870_read_reg error [%d]\n",__func__, __LINE__, ret);

	return val;
}

int aat2870_touch_ldo_write(u8 reg , u8 val)
{
	int ret =  -1  ;

	ret = i2c_smbus_write_byte_data(aat2870_i2c_client, reg , val);
	if(ret >= 0) {
		mirror_reg[reg] = val;
		DBG("Write Reg. = 0x%X, Value 0x%X\n", reg, val);
	}

	return ret;
}
//--]] LGE_UBIQUIX_MODIFIED_END : shyun@ubiquix.com [2012.08.22]- Touch-ldo function.

static int aat2870_ldo_write(u8 reg , u8 val)
{
	int ret =  -1  ;

	if (reg < 0 || reg > MAX_LDO_REG - 1) {
		DBG("ldo invalid register access\n");
		return ret;
	}

	ret = i2c_smbus_write_byte_data(aat2870_i2c_client, reg , val);
	if(ret >= 0) {
		mirror_reg[reg] = val;
		DBG("Write Reg. = 0x%X, Value 0x%X\n", reg, val);
	}

	return ret;
}

static inline int aat2870_clear_n_set(u8 clear, u8 set, u8 reg)
{
	int ret;
	u8 val = 0;

	/* Gets the initial register value */
	aat2870_ldo_read(reg, &val);

	/* Clearing all those bits to clear */
	val &= ~(clear);

	/* Setting all those bits to set */
	val |= set;

	/* Update the register */
	ret = aat2870_ldo_write(reg, val);
	if (ret)
		return ret;

	return 0;
}


void aat2870_set_ldo(bool enable, AAT2870_ENLDO ldo_num, AAT2870_VLDO level)
{
	DBG("level = %d\n", level);

	if(enable) {
		switch(ldo_num) {
			case ENLDO_A:
				aat2870_ldo_write(LDO_AB_LEVEL_REG, level<<4);
				break;
			case ENLDO_B:
				aat2870_ldo_write(LDO_AB_LEVEL_REG, level);
				break;
			case ENLDO_C:
				aat2870_ldo_write(LDO_CD_LEVEL_REG, level<<4);
				break;
			case ENLDO_D:
				aat2870_ldo_write(LDO_CD_LEVEL_REG, level);
				break;
			default:
				DBG("ldo invalid register access\n");
				return;
		}

		aat2870_clear_n_set(0, 1<<ldo_num, LDO_ABCD_EN_REG);
	}
	else {
		aat2870_clear_n_set(1<<ldo_num, 0, LDO_ABCD_EN_REG);
	}
}
EXPORT_SYMBOL(aat2870_set_ldo);


#if defined(CONFIG_MACH_LGE_HUB) || defined(CONFIG_MACH_LGE_SNIPER)
void aat2870_touch_ldo_enable(struct i2c_client *client, int on)
{
	if(on) 
	{
		DBG("touch enable..\n");
		
		/*20110214 seven.kim@lge.com to adjust touch ldo control [START] */	
		if(gpio_get_value(LCD_CP_EN) == 0)					/*20110305 seven.kim@lge.com late_resume_lcd [START] */
		{
			gpio_set_value(LCD_CP_EN, 1);
			udelay(80); 
		}
		aat2870_write_reg(client, LDO_AB_LEVEL_REG, 0x4A);
		mdelay(1);											/*20110305 seven.kim@lge.com late_resume_lcd [END] */
		aat2870_write_reg(client, LDO_CD_LEVEL_REG,0x4C);
		mdelay(1);
		aat2870_write_reg(client, LDO_ABCD_EN_REG, 0x0F);
		mdelay(1);
		/*20110214 seven.kim@lge.com to adjust touch ldo control [END] */	
	}
	else 
	{
		DBG("touch disable..\n");
		/*20110214 seven.kim@lge.com to adjust touch ldo control [START] */	
		aat2870_write_reg(client, LDO_ABCD_EN_REG, 0x03);
		/*20110214 seven.kim@lge.com to adjust touch ldo control [END] */	
	}

}
EXPORT_SYMBOL(aat2870_touch_ldo_enable); //20101222 seven.kim@lge.com for touch ESD recovery

static void aat2870_lcd_ldo_enable(struct i2c_client *client, int on)
{
	DBG("on=%d\n",on);
	if(on) 
	{
		aat2870_write_reg(client, LDO_AB_LEVEL_REG, 0x4A);
		mdelay(1);
		aat2870_write_reg(client, LDO_CD_LEVEL_REG,0x4C);
		mdelay(1);
		aat2870_write_reg(client, LDO_ABCD_EN_REG, 0x0F);
		mdelay(1);
	}
	else 
	{
	        aat2870_write_reg(client, LDO_ABCD_EN_REG, 0x00);
		mdelay(1);
	}

}
extern unsigned int mp3_playing;//2011205 kyungyoon.kim@lge.com lcd resume speed
int check_bl_shutdown=0;
void hub_lcd_initialize(void)
{
	printk(" lcd : %s \n", __func__);
#if 0	/* 20110304 seven.kim@lge.com late_resume_lcd */
	DBG("LCD intialize ..mp3_playing=%d\n",mp3_playing);
	if (mp3_playing==1)
	{
		aat2870_device_init(aat2870_i2c_client);
	}
	aat2870_lcd_ldo_enable(aat2870_i2c_client,0);
	mdelay(2);
        gpio_set_value(HUB_PANEL_LCD_RESET_N,0);
	aat2870_lcd_ldo_enable(aat2870_i2c_client,1);
	mdelay(40);
        gpio_set_value(HUB_PANEL_LCD_RESET_N,1);
	mdelay(10); 
	if (mp3_playing==1)
	{
		aat2870_write_reg(aat2870_i2c_client, LDO_AB_LEVEL_REG, 0x4A);
		mdelay(1);
		aat2870_write_reg(aat2870_i2c_client, LDO_CD_LEVEL_REG, 0x4C);
		mdelay(1);
		aat2870_write_reg(aat2870_i2c_client, LDO_ABCD_EN_REG, 0x03);
	}
	check_bl_shutdown=0;
#else /* 20110304 seven.kim@lge.com late_resume_lcd */
	gpio_set_value(LCD_CP_EN, 0);
	udelay(10);
	gpio_set_value(LCD_CP_EN, 1);
	udelay(80); /*optimised value for low-temperature condition*/
	
	aat2870_write_reg(aat2870_i2c_client, LDO_AB_LEVEL_REG, 0x4A);
	mdelay(1);
	aat2870_write_reg(aat2870_i2c_client, LDO_CD_LEVEL_REG, 0x4C);
	mdelay(1);
	aat2870_write_reg(aat2870_i2c_client, LDO_ABCD_EN_REG, 0x0F);
#endif /* 20110304 seven.kim@lge.com late_resume_lcd */
}
EXPORT_SYMBOL(hub_lcd_initialize);
EXPORT_SYMBOL(check_bl_shutdown);

/* 20110216 seven.kim@lge.com to check AAT2870 LDO_ABCD_EN_REG [START] */
int aat2870_ldo_status(void)
{
	struct aat2870_device *dev;
	dev = i2c_get_clientdata(aat2870_i2c_client);
	
	return dev->als_register38;
}
/* 20110216 seven.kim@lge.com to check AAT2870 LDO_ABCD_EN_REG [END] */
void aat2870_shutdown(void)
{
    struct aat2870_device *dev;
    DBG("aat2870_shutdown ..\n");

    aat2870_write_reg(aat2870_i2c_client, LDO_ABCD_EN_REG, 0x00);
    mdelay(1);   	

    dev = i2c_get_clientdata(aat2870_i2c_client);
    dev->bl_resumed = 0;
	
    gpio_direction_output(LCD_CP_EN, 0);
   
	check_bl_shutdown=1;
}
EXPORT_SYMBOL(aat2870_shutdown);
#endif


/* 
 * Main/Sub/Aux Backlight LED Current - BLM/BLS/BL[n][4:0] 

Tab size : 8

--------------------------------------------------------------------
Bit4	Bit3	Bit2	Bit1	Bit0	LED Current (mA) 
--------------------------------------------------------------------
BLM[4]	BLM[3]	BLM[2]	BLM[1]	BLM[0]	REG1 
BLS[4]  BLM[3]  BLS[2]  BLS[1]  BLS[0]  REG2 
BL[n]	BL[n]	BL[n]	BL[n]	BL[n]	REG3 to REG10
--------------------------------------------------------------------
0	0	0	0	0	0.45 
0	0	0	0	1	0.9 
0	0	0	1	0	1.8 
0	0	0	1	1	2.7 
0	0	1	0	0	3.6 
0	0	1	0	1	4.5 
0	0	1	1	0	5.4 
0	0	1	1	1	6.3 
0	1	0	0	0	7.2 
0	1	0	0	1	8.1 
0	1	0	1	0	9.0 
0	1	0	1	1	9.9 
0	1	1	0	0	10.8 
0	1	1	0	1	11.7 
0	1	1	1	0	12.6 
0	1	1	1	1	13.5 
1	0	0	0	0	14.4 
1	0	0	0	1	15.3 
1	0	0	1	0	16.2 
1	0	0	1	1	17.1 
1	0	1	0	0	18.0 
1	0	1	0	1	18.9 
1	0	1	1	0	19.8 
1	0	1	1	1	20.7 
1	1	0	0	0	21.6 
1	1	0	0	1	22.5 
1	1	0	1	0	23.4 
1	1	0	1	1	24.3 
1	1	1	0	0	25.2 
1	1	1	0	1	26.1 
1	1	1	1	0	27.0 
1	1	1	1	1	27.9
--------------------------------------------------------------------
*/

static void aat2870_set_main_current_level(struct i2c_client *client, int level)
{
	struct aat2870_device *dev;
	unsigned char val;

	//DBG("level = %d\n", level);

	dev = (struct aat2870_device *) i2c_get_clientdata(client);
        if (!dev->bl_resumed)
                return;
#if 0  // 20120814 sangki.hyun@lge.com ICS Backlight tunning
	if (cur_main_lcd_level ==level)
		return;
#endif	
	cur_main_lcd_level = level; 
	dev->bl_dev->props.brightness = cur_main_lcd_level;

 //2011.5.24 LG_CHANGE_S lee.hyunji@lge.com TD1396000761: kakao talk popup
//    if (early_bl_timer==0||dev->state==SLEEP_STATE)
//		aat2870_write_reg(client, AAT2870_REG0, 0xff); /* LCD channel enable */
 //2011.5.24 LG_CHANGE_E lee.hyunji@lge.com TD1396000761: kakao talk popup

#if 0 //roll-back 2011-04-07 kyungrae.jo
	//2011-04-03 kyungrae.jo@lge.com, backlight early off
	if (backlight_disable_by_level == 1) //[JKR TEST] backlight early off
	{
		aat2870_write_reg(client, AAT2870_REG0, 0xff); /* LCD channel enable */
		backlight_disable_by_level = 0;
	}
	//2011-04-03 kyungrae.jo@lge.com, backlight early off
#endif

	// LGE_B_DOM_S 2011218 kyungrae.jo@lge.com, use max current
	//max 25.2mA, min 1.8mA
	if(level > 30)
		val = (unsigned char)(level * 28 / 255);
	else
		val = (unsigned char)(level * 2 / 30);
	// LGE_B_DOM_E 2011218 kyungrae.jo@lge.com, use max current
	
	val = 0xE0 | val;

	DBG("fisrt val = 0x%x\n", val);

	// 20120829 sangki.hyun@lge.com backlight i2c failed {
	if(check_bl_shutdown)
	{
		gpio_set_value(LCD_CP_EN, 1);
		check_bl_shutdown = 0;
		udelay(80);
	}
	// 20120829 sangki.hyun@lge.com backlight i2c failed }
	
	aat2870_write_reg(client, AAT2870_REG1, val);

	// TODO : check mdelay
	mdelay(1);	// 20100526 sookyoung.kim@lge.com
}


static void leds_brightness_set(struct led_classdev *led_cdev, 
				enum led_brightness value)
{
	// struct aat2870_device *dev = dev_get_drvdata(led_cdev->dev->parent); // 20120213 taeju.park@lge.com To delete compile warning, unused variable.

	//DBG("\n");

	if (value > USER_LVL_MAX) {
		value = USER_LVL_MAX;
	}

#if 0  // 20120814 sangki.hyun@lge.com ICS Backlight tunning
	//20100709 kyungtae.oh@lge.com for resume [START_LGE]
	if (early_bl_timer == 0) {
		early_bl_value = value;
		return;
	}
	//20100709 kyungtae.oh@lge.com for resume [END_LGE]
#endif

    // LGE_CHANGE_S [park.joonhyuk@lge.com 2011. 4. 4] set the brightness level when system using ALS Mode
    aat2870_set_main_current_level(aat2870_i2c_client, value);
	//DBG("cur_main_lcd_level = %d\n", cur_main_lcd_level);
    /*
	if(dev->mode == ALS_MODE)
	{
	}
	else
	{
	aat2870_set_main_current_level(dev->client, value);
	cur_main_lcd_level = value; 
	}
    */
    // LGE_CHANGE_E [park.joonhyuk@lge.com 2011. 4. 4] set the brightness level when system using ALS Mode

	return;
}


static struct led_classdev lcd_backlight = {
	.name = "lcd-backlight",
	.brightness = MAX_BRIGHTNESS,
	.brightness_set = leds_brightness_set,
};


static void aat2870_backlight_on(struct i2c_client *client)
{
	struct aat2870_device *dev;

	dev = i2c_get_clientdata(client);

	printk("received (prev bl_status: %s)\n", dev->state ? "ON" : "OFF");
//	mdelay(100); //2011.5.24 LG_CHANGE lee.hyunji@lge.com TD1396000761: kakao talk popup

	if (dev->state == WAKE_STATE) 
		return;

	mdelay(100); //2011.5.24 LG_CHANGE lee.hyunji@lge.com TD1396000761: kakao talk popup

	if (dev->mode==0) //NORMAL_MODE
	{
#if 0  // 20120814 sangki.hyun@lge.com ICS Backlight tunning
		aat2870_set_main_current_level(client, early_bl_value);
#else
		;// nothing
#endif
	}
	else
	{
		DBG("als_mode=%d \n",dev->mode);
		cur_main_lcd_level = early_bl_value;
		aat2870_change_mode(client,dev->mode,1);
	}
	//2011.5.24 LG_CHANGE_S lee.hyunji@lge.com TD1396000761: kakao talk popup
    if (early_bl_timer==0||dev->state==SLEEP_STATE)
		aat2870_write_reg(client, AAT2870_REG0, 0xff); 
    //2011.5.24 LG_CHANGE_E lee.hyunji@lge.com TD1396000761: kakao talk popup		
	dev->state = WAKE_STATE;

	return;
}

static void aat2870_backlight_off(struct i2c_client *client)
{
	struct aat2870_device *dev;

	dev = i2c_get_clientdata(client);

	if (dev->state == SLEEP_STATE) 
		return;

	saved_main_lcd_level = cur_main_lcd_level;
	aat2870_set_main_current_level(client, 0);
	dev->state = SLEEP_STATE;	

	return;
}


static int aat2870_bl_set_intensity(struct backlight_device *bd)
{
	struct i2c_client *client = to_i2c_client(bd->dev.parent);
	struct aat2870_device *dev = NULL;

	dev = (struct aat2870_device *) i2c_get_clientdata(client);

	DBG("\n");

	if (!dev->bl_resumed)
                return 0;

	aat2870_set_main_current_level(client, bd->props.brightness);

	cur_main_lcd_level = bd->props.brightness; 
	
	return 0;
}

static int aat2870_bl_get_intensity(struct backlight_device *bd)
{
	struct i2c_client *client = to_i2c_client(bd->dev.parent);
	unsigned char val= 0;

	aat2870_read_reg(client, 0x03, &val);

	val &= 0x1f;

	return (int)val;
}


ssize_t lcd_backlight_show_level(struct device *dev, 
				 struct device_attribute *attr, 
				 char *buf)
{
	int r;

	r = snprintf(buf, PAGE_SIZE,
			"LCD Backlight Level is : %d\n", cur_main_lcd_level);

	DBG("buf = %s\n", buf);
	
	return r;
}


ssize_t lcd_backlight_store_level(struct device *dev, 
				  struct device_attribute *attr,
				  const char *buf,
				  size_t count)
{
	int level;
	struct aat2870_device *drvdata = dev_get_drvdata(dev->parent);

	if (!count)
		return -EINVAL;

	DBG("buf = %s\n", buf);
	
	level = simple_strtoul(buf, NULL, 10);
	
	if (level > USER_LVL_MAX)
		level = USER_LVL_MAX;

	DBG("level = %d\n", level);

	aat2870_set_main_current_level(drvdata->client, level);
	cur_main_lcd_level = level; 
	
	return count;
}

static void aat2870_change_mode(struct i2c_client *client, int mode, int force)
{
	struct aat2870_device *dev = NULL;
	struct lcd_ctrl_data *als_seq;
	struct lcd_ctrl_data *normal_seq;
	struct lcd_ctrl_data *als_optimize_seq;//changhyun.han@lge.com, 20100119 ,added the optimized brightness mode

	int i;

	dev = (struct aat2870_device *) i2c_get_clientdata(client);

	als_seq=normal_to_als_mode;
	normal_seq=als_to_normal_mode;
	als_optimize_seq = normal_to_als_optimize_mode;//changhyun.han@lge.com, 20100119 ,added the optimized brightness mode

	/*
	 * The external ambient light sensor is powered by the SBIAS output 
	 * which is a programmable voltage linear regulator which can provide 
	 * up to 30mA for the sensor bias.  The SBIAS output voltage may be 
	 * programmed and enabled by REG15 Bit 0, Bit 1 and Bit 2.  
	 * REG14 alco provides programming bits to set up ALC circuit and REG16
	 * for configuring polling times to save system power.  
	 * To read ALC sensor data, write to IC address 60h with 
	 * write/read Bit = 1. 
	 */

	if (early_bl_timer!=1)
	{
		dev->mode=mode;
		return;
	}
    if (mode==ALS_MODE)
    {
    	mode=POWERSAVE_MODE; //ALS_MODE == POWERSAVE_MODE
    }
	
	if (force == 0)
	{
	if (mode ==dev->mode)
	{
		DBG("The mode is same, don't update register\n");
		return;
	}
	}

	if (mode == ALS_MODE) 
	{
		for (i = 0; als_seq[i].reg != I2C_NO_REG; i++) 
		{
			aat2870_write_reg(client,als_seq[i].reg, als_seq[i].val);
			DBG("reg =0x%x, val=0x%x\n",als_seq[i].reg,als_seq[i].val);
			mdelay(1);
		}
			mdelay(5);
		dev->mode=ALS_MODE;
	}
	else if (mode == NORMAL_MODE) 
	{
		for (i = 0; normal_seq[i].reg != I2C_NO_REG; i++) 
		{
			aat2870_write_reg(client,normal_seq[i].reg, normal_seq[i].val);
			mdelay(1);
		}
			mdelay(5);
		dev->mode=NORMAL_MODE;
		aat2870_set_main_current_level(client, cur_main_lcd_level);
	}
	else if (mode == POWERSAVE_MODE) //changhyun.han@lge.com, 20100119 ,added the optimized brightness mode
	{
		for (i = 0; als_seq[i].reg != I2C_NO_REG; i++) 
		{
			aat2870_write_reg(client,als_seq[i].reg, als_seq[i].val);
			mdelay(1);
		}
			mdelay(5);
		dev->mode=ALS_MODE;//ALS is Powersave mode
	}
	else if (mode == OPTIMIZE_MODE) 
	{
		for (i = 0; als_optimize_seq[i].reg != I2C_NO_REG; i++) 
		{
			aat2870_write_reg(client,als_optimize_seq[i].reg, als_optimize_seq[i].val);
			mdelay(1);
		}
			mdelay(5);
		dev->mode=POWERSAVE_MODE;
		}
	else {
		ERR("invalid mode = %d\n", mode);
		return;
	}

	dev->mode = mode;
	DBG("dev->mode=%d\n",dev->mode);

	return;
}

static void als_work_func(struct work_struct *work)
{
	struct aat2870_device *drvdata = container_of(work, struct aat2870_device, als_work);
	unsigned char val = 0;
	
	/* 20110305 seven.kim@lge.com late_resume_lcd [START] */
	if(gpio_get_value(LCD_CP_EN) == 0)
		printk("%s : CP EN is LOW \n",__func__);
	else
	{
		aat2870_read_reg(drvdata->client, AAT2870_REG17, &val);
		drvdata->als_level=val;
		DBG("drvdata->als_level=%x\n",drvdata->als_level);
	}
	/* 20110305 seven.kim@lge.com late_resume_lcd [END] */
}

static enum hrtimer_restart als_timer_func(struct hrtimer *timer)
{
	struct aat2870_device *drvdata = container_of(timer, struct aat2870_device, als_timer);

	DBG("\n");
	queue_work(drvdata->als_wq, &drvdata->als_work);
	
	if ((drvdata->mode==ALS_MODE)&&(early_bl_timer==1))
		{
	
		hrtimer_start(&drvdata->als_timer, ktime_set(0,500000000), HRTIMER_MODE_REL); /* 250msec */
		}


	return HRTIMER_NORESTART;
}


ssize_t aat2870_show_als(struct device *dev, 
			 struct device_attribute *attr, 
			 char *buf)
{
	struct aat2870_device *drvdata = dev_get_drvdata(dev->parent);
	int r;

	if (!drvdata) {
		return 0;
	}

#if 1 //changhyun.han@lge.com, 20100119 ,added the optimized brightness mode
	if(drvdata->mode == NORMAL_MODE)
			r = snprintf(buf, PAGE_SIZE, "%s\n","0");
	else if(drvdata->mode == ALS_MODE)
			r = snprintf(buf, PAGE_SIZE, "%s\n","1");
	else
			r = snprintf(buf, PAGE_SIZE, "%s\n","0");
#else
	r = snprintf(buf, PAGE_SIZE, "%s\n", 
			(drvdata->mode == ALS_MODE) ? "1" : "0");
#endif
	return r;
}

//changhyun.han@lge.com, 20100119 ,added the optimized brightness mode
ssize_t aat2870_show_als_option(struct device *dev, 
			 struct device_attribute *attr, 
			 char *buf)
{
	struct aat2870_device *drvdata = dev_get_drvdata(dev->parent);
	int r;

	if (!drvdata) {
		return 0;
	}
	if(drvdata->mode == POWERSAVE_MODE)
			r = snprintf(buf, PAGE_SIZE, "%s\n","0");
	else if(drvdata->mode == OPTIMIZE_MODE)
			r = snprintf(buf, PAGE_SIZE, "%s\n","1");
	else
			r = snprintf(buf, PAGE_SIZE, "%s\n","0");

	return r;
}

ssize_t aat2870_store_als(struct device *dev, 
			  struct device_attribute *attr, 
			  const char *buf, 
			  size_t count)
{
	int value = 0;
	int als_mode = 0;

	struct aat2870_device *drvdata = dev_get_drvdata(dev->parent);

	DBG("buf = %s\n", buf);

	if (!count) {
		return -EINVAL;
	}
	value = simple_strtoul(buf, NULL, 10);
	DBG("value = %x\n", value);

	if (value==1) {
		als_mode = ALS_MODE;
	}
	else if(value==0) 
	{
		als_mode = NORMAL_MODE;
	}
	else
	{
		DBG("value is not valid \n");
		return -EINVAL;
	}

	aat2870_change_mode(drvdata->client, als_mode,0);
//	aat2870_read_reg(drvdata->client, AAT2870_REG17, &drvdata->als_level);
//	DBG("drvdata->als_level = %x\n", drvdata->als_level);
	hrtimer_start(&drvdata->als_timer, ktime_set(0,500000000), HRTIMER_MODE_REL); //20100304 seven.kim@lge.com late_reaume_lcd changed 100ms -> 500ms

	return count;
}

//changhyun.han@lge.com, 20100119 ,added the optimized brightness mode
ssize_t aat2870_store_als_option(struct device *dev, 
			  struct device_attribute *attr, 
			  const char *buf, 
			  size_t count)
{
	int value = 0;
	int als_mode = 0;

	struct aat2870_device *drvdata = dev_get_drvdata(dev->parent);

	DBG("buf = %s\n", buf);

	if (!count) {
		return -EINVAL;
	}
	value = simple_strtoul(buf, NULL, 10);
	DBG("value = %x\n", value);

	if (value==0) {
		als_mode = POWERSAVE_MODE;
	}
	else if(value==1) 
	{
		als_mode = OPTIMIZE_MODE;
	}
	else
	{
		DBG("value is not valid \n");
		return -EINVAL;
	}

	aat2870_change_mode(drvdata->client, als_mode,0);
//	hrtimer_start(&drvdata->als_timer, ktime_set(0,100000000), HRTIMER_MODE_REL); /* 100msec */

	return count;
}


ssize_t aat2870_show_als_level(struct device *dev, 
			       struct device_attribute *attr,
			       char *buf)
{
	struct aat2870_device *drvdata = NULL;
	int r = 0;

	drvdata = dev_get_drvdata(dev->parent);

	if (!drvdata) {
		return 0;
	}

	queue_work(drvdata->als_wq, &drvdata->als_work);//20100127,changhyun.han@lge.com
    if (drvdata->mode==NORMAL_MODE)
		drvdata->als_level=0;
		
	r = snprintf(buf, PAGE_SIZE, "0x%x\n", 	drvdata->als_level);
	DBG("drvdata->als_level = 0x%x\n", drvdata->als_level);

	return r;
}

ssize_t aat2870_store_als_control(struct device *dev, 
			  struct device_attribute *attr, 
			  const char *buf, 
			  size_t count)
{
	struct aat2870_device *drvdata = NULL;
	int value = 0;

	if (!count) {
		return -EINVAL;
	}

	drvdata = dev_get_drvdata(dev->parent);

	value = simple_strtoul(buf, NULL, 10);

	aat2870_write_reg(drvdata->client,AAT2870_REG14,value);

	drvdata->als_register14 =value;
	DBG("control_register14=%d\n",drvdata->als_register14);

	return count;
}

ssize_t aat2870_show_als_control(struct device *dev, 
			       struct device_attribute *attr,
			       char *buf)
{
	struct aat2870_device *drvdata = NULL;
	int r = 0;
	unsigned char val = 0;

	drvdata = dev_get_drvdata(dev->parent);

	if (!drvdata) {
		return 0;
	}

	val = drvdata->als_register14;
	r = snprintf(buf, PAGE_SIZE, "%d\n", val);
	DBG("control_register14=%s\n",buf);

	return r;
}

ssize_t aat2870_store_alsgain_control(struct device *dev, 
			  struct device_attribute *attr, 
			  const char *buf, 
			  size_t count)
{
	struct aat2870_device *drvdata = NULL;
	int value = 0;

	if (!count) {
		return -EINVAL;
	}

	drvdata = dev_get_drvdata(dev->parent);

	value = simple_strtoul(buf, NULL, 10);

	aat2870_write_reg(drvdata->client,AAT2870_REG16,value);

	drvdata->als_register16 =value;
	DBG("control_register16=%d\n",drvdata->als_register16);

	return count;
}

ssize_t aat2870_show_alsgain_control(struct device *dev, 
			       struct device_attribute *attr,
			       char *buf)
{
	struct aat2870_device *drvdata = NULL;
	int r = 0;
	unsigned char val = 0;

	drvdata = dev_get_drvdata(dev->parent);

	if (!drvdata) {
		return 0;
	}

	val = drvdata->als_register16;
	r = snprintf(buf, PAGE_SIZE, "%d\n", val);
	DBG("control_register16=%s\n",buf);

	return r;
}

ssize_t aat2870_store_onoff_control(struct device *dev, 
			  struct device_attribute *attr, 
			  const char *buf, 
			  size_t count)
{
	struct aat2870_device *drvdata = NULL;
	int value = 0;

	if (!count) {
		return -EINVAL;
	}

	DBG("cur_main_lcd_level=%d early_bl_value=%d\n",cur_main_lcd_level,early_bl_value);

	drvdata = dev_get_drvdata(dev->parent);

	value = simple_strtoul(buf, NULL, 10);
	DBG("value=%d\n",value);

	if (value==0)//off
	{
		aat2870_backlight_off(drvdata->client);
		aat2870_write_reg(drvdata->client, AAT2870_REG0, 0x00);
	}
	else if (value==1)//on
	{
		aat2870_write_reg(drvdata->client, AAT2870_REG0, 0xff);
	//	early_bl_value=150;
	//	aat2870_backlight_on(drvdata->client);
	}
	else if (value==2)//shutdown
	{
		aat2870_write_reg(drvdata->client, LDO_ABCD_EN_REG, 0x00);
		gpio_direction_output(LCD_CP_EN, 0);		
	}
	else if (value==3)//on after shutdown
	{
		aat2870_device_init(drvdata->client);
		aat2870_write_reg(drvdata->client, LDO_AB_LEVEL_REG, 0x4A);
		aat2870_write_reg(drvdata->client, LDO_CD_LEVEL_REG, 0x4C);
		aat2870_write_reg(drvdata->client, LDO_ABCD_EN_REG, 0x0F);
		msleep(5);	
	}
	else
		DBG("is not valid value\n");

	return count;
}

DEVICE_ATTR(level, 0664, lcd_backlight_show_level, lcd_backlight_store_level);
DEVICE_ATTR(als, 0644, aat2870_show_als, aat2870_store_als);
DEVICE_ATTR(als_level, 0444, aat2870_show_als_level, NULL);
DEVICE_ATTR(als_control, 0644, aat2870_show_als_control, aat2870_store_als_control);
DEVICE_ATTR(alsgain_control, 0644, aat2870_show_alsgain_control, aat2870_store_alsgain_control);
DEVICE_ATTR(onoff, 0644, NULL, aat2870_store_onoff_control);
DEVICE_ATTR(als_option, 0644, aat2870_show_als_option, aat2870_store_als_option);//changhyun.han@lge.com, 20100119 ,added the optimized brightness mode

static struct backlight_ops aat2870_bl_ops = {
	.update_status	= aat2870_bl_set_intensity,
	.get_brightness	= aat2870_bl_get_intensity,
};


static int aat2870_remove(struct i2c_client *i2c_dev)
{
	struct aat2870_device *dev = NULL;

	dev = (struct aat2870_device *) i2c_get_clientdata(i2c_dev);

	unregister_early_suspend(&dev->early_suspend);

	gpio_free(LCD_CP_EN);

	device_remove_file(dev->led->dev, &dev_attr_level);
	device_remove_file(dev->led->dev, &dev_attr_als);
	device_remove_file(dev->led->dev, &dev_attr_als_level);
	device_remove_file(dev->led->dev, &dev_attr_als_control);
	device_remove_file(dev->led->dev, &dev_attr_alsgain_control);
	device_remove_file(dev->led->dev, &dev_attr_onoff);
	device_remove_file(dev->led->dev, &dev_attr_als_option);//changhyun.han@lge.com, 20100119, added the optimized brightness mode

	backlight_device_unregister(dev->bl_dev);
	led_classdev_unregister(dev->led);
	i2c_set_clientdata(i2c_dev, NULL);
	hrtimer_cancel(&dev->als_timer);
	if (dev->als_wq)
		destroy_workqueue(dev->als_wq);

	return 0;
}	


#ifdef CONFIG_HAS_EARLYSUSPEND
static int aat2870_bl_suspend(struct i2c_client *client, pm_message_t state)
{
	struct aat2870_device *dev;
	DBG("new state: %d\n", state.event);

	dev = i2c_get_clientdata(client);

	aat2870_backlight_off(client);

	aat2870_write_reg(client, AAT2870_REG0, 0x00);

	early_bl_timer = 0;
	//20100205 kyungyoon.kim@lge.com for LCD resume speed[START]	
	dev->bl_resumed=0;
    //20100205 kyungyoon.kim@lge.com for LCD resume speed[END]

	return 0;
}

static int aat2870_bl_resume(struct i2c_client *client)
{
	struct aat2870_device *dev;
        
	dev = i2c_get_clientdata(client);
	DBG("dev->bl_resumed=%d\n",dev->bl_resumed);
	//20100205 kyungyoon.kim@lge.com for LCD resume speed[START]	
	if (dev->bl_resumed==1)
		return 0;
    //20100205 kyungyoon.kim@lge.com for LCD resume speed[END]
	early_bl_timer = 1;

	DBG("early_bl_timer: %d\n", early_bl_timer);
	aat2870_backlight_on(client);

	//20100205 kyungyoon.kim@lge.com for LCD resume speed[START]
	dev->bl_resumed=1;
	//20100205 kyungyoon.kim@lge.com for LCD resume speed[END]
	return 0;
}

//void aat2870_resume_for_lcd()
void aat2870_resume_for_lcd(void) // 20120213 taeju.park@lge.com To delete compile warning, specifying 0 arguments
{
	struct aat2870_device *dev;
	dev = i2c_get_clientdata(aat2870_i2c_client);

#if 0 /* 20110304 seven.kim@lge.com late_resume_lcd */
	DBG("dev->bl_resumed=%d\n",dev->bl_resumed);
	//20100205 kyungyoon.kim@lge.com for LCD resume speed[START]	
	if (dev->bl_resumed==1)
		return;
    //20100205 kyungyoon.kim@lge.com for LCD resume speed[END]
	early_bl_timer = 1;

	aat2870_backlight_on(aat2870_i2c_client);

	if (system_rev >= 3) //Over REV.C
 	bd2802_resume_for_lcd();

	//20100205 kyungyoon.kim@lge.com for LCD resume speed[START]
	dev->bl_resumed=1;
	//20100205 kyungyoon.kim@lge.com for LCD resume speed[END]
#else /* 20110304 seven.kim@lge.com late_resume_lcd */
	//synaptics_ts_resume(hub_ts_client); //seven changed for late_resume_lcd
	//aat2870_backlight_on(aat2870_i2c_client); //seven changed for late_resume_lcd
	//aat2870_bl_resume(aat2870_i2c_client); //20110321 black lcd problem patch
#endif /* 20110304 seven.kim@lge.com late_resume_lcd */
}
EXPORT_SYMBOL(aat2870_resume_for_lcd);

static void aat2870_early_suspend(struct early_suspend *h)
{
	struct aat2870_device *dev;
	DBG("\n");
	dev = container_of(h, struct aat2870_device, early_suspend);

	hrtimer_cancel(&dev->als_timer);
	/* 20110218 seven.kim@lge.com to contorl AAT2870 sleep/resume state machine [START] */
	g_AAT2870_State_Machine = AAT2870_EARLY_SUSPEND_STATE;
	/* 20110218 seven.kim@lge.com to contorl AAT2870 sleep/resume state machine [END] */
	
	aat2870_bl_suspend(dev->client, PMSG_SUSPEND);
	//20100205 kyungyoon.kim@lge.com for LCD resume speed[START]
	dev->bl_resumed=0;
	//20100205 kyungyoon.kim@lge.com for LCD resume speed[END]
}

/* S[, 20120922, mannsik.chung@lge.com, PM from froyo. */
#if defined(CONFIG_PRODUCT_LGE_LU6800)
extern u32 doing_wakeup;
#endif
/* E], 20120922, mannsik.chung@lge.com, PM from froyo. */

static void aat2870_late_resume(struct early_suspend *h)
{
	struct aat2870_device *dev;

	dev = container_of(h, struct aat2870_device, early_suspend);
	DBG("[aat2870_suspend]aat2870_late_resume \n");
	/* 20110218 seven.kim@lge.com to contorl AAT2870 sleep/resume state machine [START] */
	g_AAT2870_State_Machine = AAT2870_LATE_RESUME_STATE;
	/* 20110218 seven.kim@lge.com to contorl AAT2870 sleep/resume state machine [END] */

	 /* 20110304 seven.kim@lge.com late_resume_lcd [START] */	
	 hrtimer_start(&dev->als_timer, ktime_set(0,500000000), HRTIMER_MODE_REL); //20100304 seven.kim@lge.com late_reaume_lcd changed 100ms -> 500ms
	 aat2870_bl_resume(dev->client); //20110321 for black lcd display
 	 /* 20110304 seven.kim@lge.com late_resume_lcd [END] */

/* S[, 20120922, mannsik.chung@lge.com, PM from froyo. */
#if defined(CONFIG_PRODUCT_LGE_LU6800)
	doing_wakeup = 0;
#endif
/* E], 20120922, mannsik.chung@lge.com, PM from froyo. */

}
#endif	/* CONFIG_HAS_EARLYSUSPEND */

static int aat2870_suspend(struct i2c_client *client, pm_message_t state)
{
	struct aat2870_device *dev;
	dev = i2c_get_clientdata(client);
	
	printk("[aat2870_suspend] new state: %d\n",state.event);

	client->dev.power.power_state = state;
#if	0  // 20120814 sangki.hyun@lge.com ICS Backlight tunning
	dev->mode=0;
#endif
	//20100205 kyungyoon.kim@lge.com for LCD resume speed[START]
	dev->bl_resumed=0;
	//20100205 kyungyoon.kim@lge.com for LCD resume speed[END]
	
#if defined(CONFIG_MACH_LGE_HUB) || defined(CONFIG_MACH_LGE_SNIPER)
//	DBG("touch disable..\n");
//	aat2870_touch_ldo_enable(client, 0);
//	msleep(5);
#endif
	gpio_direction_output(LCD_CP_EN, 0);  // 20120829 sangki.hyun@lge.com

// prime@sdcmicro.com The following calls are made by aat2870_shutdown() which is called by panel-hub.c [START]
        #if 0
	aat2870_write_reg(client, LDO_ABCD_EN_REG, 0x00);
	gpio_direction_output(LCD_CP_EN, 0);
	#endif

#if 0 /* 20110304 seven.kim@lge.com late_resume_lcd */
	/* 20110218 seven.kim@lge.com to contorl AAT2870 sleep/resume state machine [START] */
	if (g_AAT2870_State_Machine == AAT2870_RESUME_STATE)
	{
		aat2870_write_reg(aat2870_i2c_client, LDO_ABCD_EN_REG, 0x00);
		mdelay(1);
		/* LGE_UPDATE_S [daewung.kim@lge.com] 20110218, Shutdown aat2870 for saving 0.5mA */
		gpio_direction_output(LCD_CP_EN, 0); //20110227 seven.kim@lge.com to prevent i2c error
		/* LGE_UPDATE_E [daewung.kim@lge.com] 20110218, Shutdown aat2870 for saving 0.5mA */
	}

	g_AAT2870_State_Machine = AAT2870_SUSPEND_STATE;
	/* 20110218 seven.kim@lge.com to contorl AAT2870 sleep/resume state machine [END] */
#endif /* 20110304 seven.kim@lge.com late_resume_lcd */
// prime@sdcmicro.com The following calls are made by aat2870_shutdown() which is called by panel-hub.c [END]

	return 0;
}

static int aat2870_resume(struct i2c_client *client)
{

	DBG("[aat2870_resume] old state: %d\n", client->dev.power.power_state.event);

	client->dev.power.power_state = PMSG_ON;

#if defined(CONFIG_MACH_LGE_HUB) || defined(CONFIG_MACH_LGE_SNIPER)
// prime@sdcmicro.com The following calls are made by hub_lcd_initialize() which is called by panel-hub.c [START]
   #if 0 /* 20110304 seven.kim@lge.com late_resume_lcd */
	/*20110215 seven.kim@lge.com to adjust touch ldo control [START] */
	aat2870_write_reg(client, LDO_AB_LEVEL_REG, 0x4A);
	mdelay(1);
	#if 0 //20110227 seven.kim@lge.com split ldo control to prevent i2c error
	aat2870_write_reg(client, LDO_CD_LEVEL_REG, 0x4C);
	mdelay(1);
	aat2870_write_reg(client, LDO_ABCD_EN_REG, 0x0F);
	mdelay(1);
	#else
	aat2870_write_reg(client, LDO_ABCD_EN_REG, 0x03);
	mdelay(1);
	#endif
	/*20110215 seven.kim@lge.com to adjust touch ldo control [END] */
   #endif /* 20110304 seven.kim@lge.com late_resume_lcd */	
// prime@sdcmicro.com The following calls are made by hub_lcd_initialize() which is called by panel-hub.c [END]
#endif

	/* 20110218 seven.kim@lge.com to contorl AAT2870 sleep/resume state machine [START] */
	g_AAT2870_State_Machine = AAT2870_RESUME_STATE;
	/* 20110218 seven.kim@lge.com to contorl AAT2870 sleep/resume state machine [END] */
	return 0;
}

// kibum.lee@lge.com section mismatch error fix
//static int __init aat2870_probe(struct i2c_client *i2c_dev, const struct i2c_device_id *id)
static int aat2870_probe(struct i2c_client *i2c_dev, const struct i2c_device_id *id)
{
	struct aat2870_device *dev;
	struct backlight_device *bl_dev;
	int err = 0;

	pr_warning("%s() -- start\n", __func__);

	aat2870_i2c_client = i2c_dev;
	
	dev = kzalloc(sizeof(struct aat2870_device), GFP_KERNEL);

	if (dev == NULL) {
		dev_err(&i2c_dev->dev, "fail alloc for aat2870_device\n");
		return 0;
	}

// prime@sdcmicro.com The parameter list of backlight_device_register() was changed in 2.6.35 kernel [START]
#if 0   // prior to 2.6.35
	bl_dev = backlight_device_register(AAT2870_I2C_BL_NAME, 
					   &i2c_dev->dev,
					   NULL, 
					   &aat2870_bl_ops);
	bl_dev->props.max_brightness = MAX_BRIGHTNESS;
	bl_dev->props.brightness = DEFAULT_BRIGHTNESS;
	bl_dev->props.power = FB_BLANK_UNBLANK;
#else
        {
	        struct backlight_properties props;

                memset(&props, 0, sizeof(struct backlight_properties));
                props.max_brightness = MAX_BRIGHTNESS;
                props.brightness = DEFAULT_BRIGHTNESS;
                props.power = FB_BLANK_UNBLANK;
		   props.type = BACKLIGHT_RAW;  // kibum.lee@lge.comc ICS  WARNING message delete

        	bl_dev = backlight_device_register(AAT2870_I2C_BL_NAME, 
        					   &i2c_dev->dev,
        					   NULL, 
        					   &aat2870_bl_ops,
        					   &props);
        }
#endif
// prime@sdcmicro.com The parameter list of backlight_device_register() was changed in 2.6.35 kernel [START]
	bl_dev->props.brightness = USER_LVL_MAX;

	bl_dev->props.power = FB_BLANK_UNBLANK;
	
	dev->bl_dev = bl_dev;
	dev->client = i2c_dev;
	i2c_set_clientdata(i2c_dev, dev);
	//i2c_set_adapdata(i2c_dev->adapter, i2c_dev);

	// TODO:
	dev->level = DEFAULT_BRIGHTNESS;
	dev->state = WAKE_STATE;
	dev->mode = NORMAL_MODE;
	dev->als_level = 0;

	INIT_WORK(&dev->als_work, als_work_func);
	
	dev->als_wq = create_singlethread_workqueue("als_wq");
	if (!dev->als_wq)
		return -ENOMEM;

	hrtimer_init(&dev->als_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	dev->als_timer.function = als_timer_func;


	if(gpio_request(LCD_CP_EN, "lcdcs") < 0) {
		return -ENOSYS;
	}

	// kibum.lee@lge.com ICS temp
	gpio_direction_output(LCD_CP_EN, 1);


//	if(gpio_request(HUB_PANEL_LCD_CS, "lcd cs") < 0) {
//		return;
//	}
	if(gpio_request(HUB_PANEL_LCD_RESET_N, "lcd reset") < 0) {
		return -ENOSYS;
	}

	gpio_direction_output(HUB_PANEL_LCD_RESET_N, 1);

//	aat2870_change_mode(dev->client,ALS_MODE);

#if defined(CONFIG_MACH_LGE_HUB) || defined(CONFIG_MACH_LGE_SNIPER)
	aat2870_touch_ldo_enable(i2c_dev, 1);
#endif

	if (led_classdev_register(&i2c_dev->dev, &lcd_backlight) == 0) {
		dev->led = &lcd_backlight;
		err = device_create_file(dev->led->dev, &dev_attr_level);
		err = device_create_file(dev->led->dev, &dev_attr_als);
		err = device_create_file(dev->led->dev, &dev_attr_als_level);
		err = device_create_file(dev->led->dev, &dev_attr_als_control);
		err = device_create_file(dev->led->dev, &dev_attr_alsgain_control);
		err = device_create_file(dev->led->dev, &dev_attr_onoff);
		err = device_create_file(dev->led->dev, &dev_attr_als_option);//changhyun.han@lge.com, 20100119 ,added the optimized brightness mode
	}

	//aat2870_set_main_current_level(i2c_dev, DEFAULT_BRIGHTNESS); 

	// TODO : error handling

#ifdef CONFIG_HAS_EARLYSUSPEND
	dev->early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN;
	dev->early_suspend.suspend = aat2870_early_suspend;
	dev->early_suspend.resume = aat2870_late_resume;
	register_early_suspend(&dev->early_suspend);
#endif
	dev->bl_resumed = 1;
	
	gpio_request(LCD_CP_EN, "LCD_CP_EN");

	return 0;
}


static struct i2c_driver aat2870_driver = {
	.probe		= aat2870_probe,
	.remove		= aat2870_remove,
//#ifndef CONFIG_HAS_EARLYSUSPEND 	/*LG_CHANGE_S lee.hyunji@lge.com 20110420 Doesn't working ASL mode after resume*/
	.suspend	= aat2870_suspend,
	.resume		= aat2870_resume,
//#endif
	.id_table	= aat2870_bl_id, 

	.driver = {
		.name	= AAT2870_I2C_BL_NAME,
		.owner	= THIS_MODULE,
	},
};


static int __init aat2870_init(void)
{
	return i2c_add_driver(&aat2870_driver);
}

static void __exit aat2870_exit(void)
{
	i2c_del_driver(&aat2870_driver);

	return;
}

module_init(aat2870_init);
module_exit(aat2870_exit);

MODULE_DESCRIPTION("AAT2870 Backlight Control");
MODULE_AUTHOR("Yool-Je Cho <yoolje.cho@lge.com>");
MODULE_LICENSE("GPL");

