/*
 * heaven aat2862 LED driver IC
 *
 * Copyright (C) 2009 LGE, Inc.
 *
 * Author: Shin hyesung <newcomet@lge.com>
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
#include <linux/device.h>
#include <linux/delay.h>
#include <asm/gpio.h>
#include <asm/system.h>
#include <linux/interrupt.h>
#include <linux/i2c.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>

#define LED_DRIVER_EN 		149
#define MAX_LED_IC_REG		8

static struct i2c_client *led_ic_client;
static u8 mirror_reg[MAX_LED_IC_REG] = {0,};
static u8 is_enabled	= true;

static void aat2862_led_ic_read(u8 reg , u8 *val)
{
	if (reg < 0 || reg > MAX_LED_IC_REG-1)
	{
		printk("[LED IC] LGE: Heaven led_ic invalid register access\n");
		return;
	}

	*val = i2c_smbus_read_byte_data(led_ic_client, reg);
	printk("[LED IC] led_ic Reg read 0x%X: Val=0x%X\n", (u8)reg, (u8)*val);
}

static int aat2862_led_ic_write(u8 reg , u8 val)
{
	int ret =  -1  ;

	if (reg < 0 || reg > MAX_LED_IC_REG-1)
	{
		printk("[LED IC] LGE: Heaven led_ic invalid register access\n");
		return ret;
	}

	ret = i2c_smbus_write_byte_data(led_ic_client, reg , val);
	if(ret >= 0)
	{
		mirror_reg[reg] = val;
		printk("[LED IC] LGE: Heaven led_ic Write Reg. = 0x%X, Value 0x%X\n", reg, val);
	}

	return ret;
}

#ifdef CONFIG_PROC_FS
#define	HEAVEN_LED_IC_PROC_FILE	"driver/led_ic"
static ssize_t led_ic_proc_read (struct file *filp, char *buf, size_t len, loff_t *offset)
{
	return 0;
}

static ssize_t led_ic_proc_write (struct file *filp, const char *buf, size_t len, loff_t *off)
{
	return len;
}

static struct file_operations heaven_led_ic_proc_ops = {
	.read = led_ic_proc_read,
	.write = led_ic_proc_write,
};

static void create_heaven_led_ic_proc_file(void)
{
	struct proc_dir_entry *heaven_led_ic_proc_file = NULL;

	heaven_led_ic_proc_file = create_proc_entry(HEAVEN_LED_IC_PROC_FILE, 0777, NULL);
	if (heaven_led_ic_proc_file) {
		heaven_led_ic_proc_file->owner = THIS_MODULE;
		heaven_led_ic_proc_file->proc_fops = &heaven_led_ic_proc_ops;
	} else
		printk(KERN_INFO "LGE: Heaven led_ic proc file create failed!\n");
}

static void remove_heaven_led_ic_proc_file(void)
{
	remove_proc_entry(HEAVEN_LED_IC_PROC_FILE, NULL);
}
#endif

static inline int aat2862_clear_n_set(u8 clear, u8 set, u8 reg)
{
	int ret;
	u8 val = 0;

	/* Gets the initial register value */
	aat2862_led_ic_read(reg, &val);

	/* Clearing all those bits to clear */
	val &= ~(clear);

	/* Setting all those bits to set */
	val |= set;

	/* Update the register */
	ret = aat2862_led_ic_write(reg, val);
	if (ret)
		return ret;

	return 0;
}

/*
Bit4 Bit3 Bit2 Bit1 Bit0 Current (mA)
0 0 0 0 0 30
0 0 0 0 1 29.03
0 0 0 1 0 28.06
0 0 0 1 1 27.10
0 0 1 0 0 26.13
0 0 1 0 1 25.16
0 0 1 1 0 24.19
0 0 1 1 1 23.23
0 1 0 0 0 22.26
0 1 0 0 1 21.29
0 1 0 1 0 20.32
0 1 0 1 1 19.35
0 1 1 0 0 18.38
0 1 1 0 1 17.42
0 1 1 1 0 16.45
0 1 1 1 1 15.48
1 0 0 0 0 14.52
1 0 0 0 1 13.55
1 0 0 1 0 12.58
1 0 0 1 1 11.61
1 0 1 0 0 10.65
1 0 1 0 1 9.68
1 0 1 1 0 8.71
1 0 1 1 1 7.74
1 1 0 0 0 6.77
1 1 0 0 1 5.81
1 1 0 1 0 4.84
1 1 0 1 1 3.87
1 1 1 0 0 2.9
1 1 1 0 1 1.94
1 1 1 1 0 0.97
1 1 1 1 1 0.48
*/
typedef enum
{
	LEDC_30,		/* 0 */
	LEDC_29_03,
	LEDC_28_06,
	LEDC_27_10,
	LEDC_26_13,
	LEDC_25_16,		/* 5 */
	LEDC_24_19,
	LEDC_23_23,
	LEDC_22_26,
	LEDC_21_29,
	LEDC_20_32,		/* 10 */
	LEDC_19_35,
	LEDC_18_38,
	LEDC_17_42,
	LEDC_16_45,
	LEDC_15_48,		/* 15 */
	LEDC_14_52,
	LEDC_13_55,
	LEDC_12_58,
	LEDC_11_61,
	LEDC_10_65,		/* 20 */
	LEDC_9_68,
	LEDC_8_71,
	LEDC_7_74,
	LEDC_6_77,
	LEDC_5_81,		/* 25 */
	LEDC_4_84,
	LEDC_3_87,
	LEDC_2_9,
	LEDC_1_94,
	LEDC_0_97,		/* 30 */
	LEDC_0_48,

	LEDC_MAX,		/* 32 */
} AAT2862_LEDC;


/* SUB, AUX1, AUX2 */
/* led value: refer leds-heaven.c */
#define HEAVEN_LED_OFF	0x0
#define HEAVEN_LED_R		0x1
#define HEAVEN_LED_G		0x2
#define HEAVEN_LED_B		0x4

#define 	SUB_ON	0x20
#define 	AUX1_ON	0x20
#define 	AUX2_ON	0x20

#define	LED_CURRENT	LEDC_20_32

void aat2862_set_rgb_led(u8 led_value)
{
	if(!is_enabled)
		return;

	/* off */
	if(led_value == HEAVEN_LED_OFF)
	{
		aat2862_clear_n_set(SUB_ON, 0, 0x04);
		aat2862_clear_n_set(AUX1_ON, 0, 0x05);
		aat2862_clear_n_set(AUX2_ON, 0, 0x06);

		return;
	}

	if((led_value & HEAVEN_LED_R) == HEAVEN_LED_R)
	{
		aat2862_clear_n_set(0, SUB_ON | LED_CURRENT, 0x04);
	}

	if((led_value & HEAVEN_LED_G) == HEAVEN_LED_G)
	{
		aat2862_clear_n_set(0, AUX1_ON | LED_CURRENT, 0x05);
	}

	if((led_value & HEAVEN_LED_B) == HEAVEN_LED_B)
	{
		aat2862_clear_n_set(0, AUX2_ON | LED_CURRENT, 0x06);
	}

	return;
}
EXPORT_SYMBOL(aat2862_set_rgb_led);

/*
FADE_TIME[1] FADE_TIME[0] Timing (ms)
0 0 850
0 1 650
1 0 425
1 1 850
*/
typedef enum
{
	FADE_TIME_DEFAULT,	/* 0 */
	FADE_TIME_650,
	FADE_TIME_450,
	FADE_TIME_850,

	FADE_TIME_MAX,	/* 5 */
} AAT2862_FADE_TIME;

/* MAIN */
#define 	MAIN_ON	0x20
#define 	FADE_MAIN	0x08
void aat2862_set_bl_led(bool enable, u8 fade_en, AAT2862_FADE_TIME fade_time, AAT2862_LEDC current_level)
{
	if(!is_enabled)
		return;

	if(fade_en)
	{
		aat2862_clear_n_set(0, FADE_MAIN | (fade_time << 4), 0x07);
	}
	else
		aat2862_clear_n_set(FADE_MAIN, 0, 0x07);

	if(enable)
	{
		if(current_level >= LEDC_MAX)
			return;

		aat2862_clear_n_set(0, MAIN_ON | current_level, 0x03);
	}
	else
	{
		aat2862_clear_n_set(MAIN_ON, 0, 0x03);
	}
}
EXPORT_SYMBOL(aat2862_set_bl_led);

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
typedef enum
{
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
} AAT2862_VLDO;

typedef enum
{
	ENLDO_A,	/* 0 */
	ENLDO_B,
	ENLDO_C,
	ENLDO_D,

	ENLDO_MAX,	/* 5 */
} AAT2862_ENLDO;

void aat2862_set_ldo(bool enable, AAT2862_ENLDO ldo_num, AAT2862_VLDO level)
{
	if(!is_enabled)
		return;

	if(enable)
	{
		switch(ldo_num)
		{
			case ENLDO_A:
				aat2862_led_ic_write(0x00, level<<4);
				break;
			case ENLDO_B:
				aat2862_led_ic_write(0x00, level);
				break;
			case ENLDO_C:
				aat2862_led_ic_write(0x01, level<<4);
				break;
			case ENLDO_D:
				aat2862_led_ic_write(0x01, level);
				break;
			default:
				printk("[LED IC] LGE: Heaven led_ic invalid register access\n");
				return;
		}

		aat2862_clear_n_set(0, 1<<ldo_num, 0x02);
	}
	else
	{
		aat2862_clear_n_set(1<<ldo_num, 0, 0x02);
	}
}
EXPORT_SYMBOL(aat2862_set_ldo);

static void aat2862_test(void)
{
}

static void aat2862_led_ic_init(void)
{

	gpio_set_value(LED_DRIVER_EN, 1);
	udelay(100);

	/* pin grouping - BL1~4:Main, BL5~6:Sub, Aux1, Aux2 */
	aat2862_led_ic_write(0x05, 0x80);
	aat2862_led_ic_write(0x06, 0x80);

	aat2862_test();

	printk("[LED IC] HEAVEN LED_IC aat2862 init\n");

	is_enabled = true;
}



static int __init led_ic_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int ret;

	led_ic_client = client;

	ret = gpio_request(LED_DRIVER_EN, "led_ic EN gpio");
	if (ret < 0) {
		printk("[LED IC] led_ic LED_DRIVER_EN Gpio request Fail\n");
		return -ENOSYS;
	}

	ret = gpio_direction_output(LED_DRIVER_EN, 0);
	if (ret < 0)
		printk("[LED IC] led_ic LED_DRIVER_EN GPIO direction Setting was failed\n");

#ifdef CONFIG_PROC_FS
	create_heaven_led_ic_proc_file();
#endif

	aat2862_led_ic_init();

	return ret;
}

static int led_ic_remove(struct i2c_client *client)
{
	gpio_free(LED_DRIVER_EN);
	i2c_set_clientdata(client, NULL);
#ifdef CONFIG_PROC_FS
	remove_heaven_led_ic_proc_file();
#endif
	return 0;
}

static int led_ic_suspend(struct i2c_client *client, pm_message_t mesg)
{
	/* all register are reset to their default */
	gpio_set_value(LED_DRIVER_EN, 0);

	is_enabled = false;

	return 0;
}

static int led_ic_resume(struct i2c_client *client)
{
	int reg;
	u8 val = 0;
	u32 ret = -1;

	/* restore LED IC register */
	gpio_set_value(LED_DRIVER_EN, 1);

	//udelay(100);
#if 1
	//ret = i2c_smbus_write_block_data(led_ic_client, reg , ARRAY_SIZE(mirror_reg), mirror_reg);
#else
	for (reg = 0; reg < MAX_LED_IC_REG; reg++)
	{
		val = mirror_reg[reg];
		ret = i2c_smbus_write_byte_data(led_ic_client, reg , val);
	}
#endif

	is_enabled = true;

	return 0;
}

static const struct i2c_device_id led_ic_ids[] = {
	{ "heaven_i2c_led_ic", 0 },
	{ /* end of list */ },
};

static struct i2c_driver led_ic_driver = {
	.probe	 = led_ic_probe,
	.remove	 = led_ic_remove,
#ifdef CONFIG_PM
	.suspend = led_ic_suspend,
	.resume = led_ic_resume,
#endif
	.id_table	= led_ic_ids,
	.driver	 = {
		.name = "heaven_i2c_led_ic",
		.owner = THIS_MODULE,
	},
};

static int __init led_ic_init(void)
{
	return i2c_add_driver(&led_ic_driver);
}

static void __exit led_ic_exit(void)
{
	i2c_del_driver(&led_ic_driver);
}

module_init(led_ic_init);
module_exit(led_ic_exit);

MODULE_AUTHOR("LG Electronics");
MODULE_DESCRIPTION("heaven led_ic Driver");
MODULE_LICENSE("GPL");
