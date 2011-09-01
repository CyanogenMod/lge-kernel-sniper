/* drivers/video/backlight/e920_aat2862.c 
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
#include <plat/mux.h>	
#include <linux/leds.h>
#include <plat/board.h>
#include <linux/earlysuspend.h>

#include <linux/i2c/twl.h>
#define TWL_LEDA (OMAP_MAX_GPIO_LINES + TWL4030_GPIO_MAX)
#define RGB_LED_CNTL 128
static unsigned touch_key_bl_gpio;
extern unsigned int system_rev;

#define SEND_SC654_SPIF_ADDR(d) do {\
	int len = d;\
	gpio_set_value(touch_key_bl_gpio, 0);\
	udelay(1);\
	while(len--) {\
		gpio_set_value(touch_key_bl_gpio, 1);\
		udelay(1);\
		gpio_set_value(touch_key_bl_gpio, 0);\
		udelay(1);\
	}\
	gpio_set_value(touch_key_bl_gpio, 1);\
	udelay(560);\
} while(0)

#define SEND_SC654_SPIF_DATA(d) SEND_SC654_SPIF_ADDR(d)

#define TOUCH_KEY_BL_INIT() do {\
	if(system_rev >= 2) {\
		touch_key_bl_gpio = RGB_LED_CNTL;\
	} else {\
		touch_key_bl_gpio = TWL_LEDA;\
	}\
	if (0 > gpio_request(touch_key_bl_gpio, "touch_keybl"))\
		printk (KERN_ERR "touch_keybl gpio_request failed\n");\
	if(system_rev >= 2) {\
		gpio_direction_output(touch_key_bl_gpio, 1);\
		mdelay(1);\
	}\
} while(0)

#define TOUCH_KEY_BL_ON() do {\
	if(system_rev >= 2) {\
		/*fade on*/\
		/*SEND_SC654_SPIF_ADDR(4);*/\
		/*SEND_SC654_SPIF_DATA(0);*/\
		/*SEND_SC654_SPIF_ADDR(1);*/\
		/*SEND_SC654_SPIF_DATA(4);*/\
		/*SEND_SC654_SPIF_ADDR(4);*/\
		/*SEND_SC654_SPIF_DATA(1);*/\
		/*led on*/\
		SEND_SC654_SPIF_ADDR(0);\
		SEND_SC654_SPIF_DATA(0x1f);\
		SEND_SC654_SPIF_ADDR(1);\
		SEND_SC654_SPIF_DATA(0xd);/*red*/\
		SEND_SC654_SPIF_ADDR(2);\
		SEND_SC654_SPIF_DATA(0x12);/*green*/\
		SEND_SC654_SPIF_ADDR(3);\
		SEND_SC654_SPIF_DATA(0x12);/*blue*/\
	} else {\
		gpio_set_value(touch_key_bl_gpio, 0);\
	}\
} while(0)

#define TOUCH_KEY_BL_OFF() do {\
	if(system_rev >= 2) {\
		/*fade enable*/\
		/*SEND_SC654_SPIF_ADDR(4);*/\
		/*SEND_SC654_SPIF_DATA(1);*/\
		/*led off*/\
		SEND_SC654_SPIF_ADDR(0);\
		SEND_SC654_SPIF_DATA(0);\
	} else {\
		gpio_set_value(touch_key_bl_gpio, 1);\
	}\
} while(0)

static struct i2c_client *aat2862_i2c_client;

#define I2C_NO_REG	0xFF

#define MAX_BRIGHTNESS 		0x15	// 0000 1010 = 20.32mA
#define DEFAULT_BRIGHTNESS 	0x0D	// 0000 1101 = 12.58mA

#define MODULE_NAME    "hub_aat2862"

#define BL_ON	1
#define BL_OFF	0

#define MAX_LDO_REG		8
static u8 mirror_reg[MAX_LDO_REG] = {0,};

struct aat2862_device {
	struct i2c_client *client;
	struct backlight_device *bl_dev;
    struct early_suspend early_suspend;
};

struct bl_ctrl_data {
	unsigned short reg;
	unsigned short val;
};

#ifdef CONFIG_HAS_EARLYSUSPEND
static void aat2862_early_suspend(struct early_suspend *h);
static void aat2862_late_resume(struct early_suspend *h);
static int early_bl_timer = 1;
static int early_bl_value = 0;
#endif

#define LCD_CP_EN 62
#define HUB_I2C_BL_NAME "hub_i2c_bl"

#define LDO_AB_LEVEL_REG	0x24
#define LDO_CD_LEVEL_REG	0x25
#define LDO_ABCD_EN_REG	0x26

static const struct i2c_device_id hub_bl_id[] = {
	{ HUB_I2C_BL_NAME, 0 },
	//{ }
	{ },
};

static int aat2862_write_reg(struct i2c_client *client, unsigned char reg, unsigned char val);
static int aat2862_read_reg(struct i2c_client *client, unsigned char reg, unsigned char *ret);

unsigned int cur_main_lcd_level = DEFAULT_BRIGHTNESS;
static unsigned int saved_main_lcd_level = DEFAULT_BRIGHTNESS;

#ifdef CONFIG_FB_OMAP_BOOTLOADER_INIT
static int backlight_status = BL_ON;
#else
static int backlight_status = BL_OFF;
#endif
static struct aat2862_device *main_aat2862_dev = NULL;
#if 0
static struct bl_ctrl_data pwron_seq[]=
{
#if defined(CONFIG_MACH_LGE_HUB_REV_A)
	{LDO_AB_LEVEL_REG,0x4A},    /* LDOA 1.8V_LCD_IOVCC(0100), LDOB 2.8V_LCD_VCC_VCI (1010) */
	{LDO_CD_LEVEL_REG,0xAA},    /* LDOC 2.8V_TOUCH_VDD (1010), LDOD 2.8V_TOUCH_VCPIN (1010) */
#else
	{LDO_AB_LEVEL_REG,0x4C},	/* LDOA 1.8V_LCD_IOVCC(0100), LDOB 3.0V_LCD_VCC_VCI (1100) */
	{LDO_CD_LEVEL_REG,0x4C},	/* LDOC 1.8V_TOUCH_VDD (0100), LDOD 3.0V_TOUCH_VCPIN (1100) */
#endif
	//{LDO_ABCD_EN_REG, 0x01}, 	//Enable all LDOs
	{LDO_ABCD_EN_REG, 0x0F},   //Enable all LDOs 

	//{0x03, 0xFF-DEFAULT_BRIGHTNESS},	// Main BL ctrl.
	//{0x04, 0x7F-DEFAULT_BRIGHTNESS},	// Sub BL ctrl.
	//{0x05, 0x7F-DEFAULT_BRIGHTNESS},	// Aux1 BL ctrl.
	//{0x06, 0x7F-DEFAULT_BRIGHTNESS},	// Aux2 BL ctrl.
	{I2C_NO_REG, 0x00}  /* End of array */
};
#endif

void aat2862_hreset(void)
{
	gpio_set_value(LCD_CP_EN, 1);
	udelay(10);
	gpio_set_value(LCD_CP_EN, 0);
	udelay(1000);
	gpio_set_value(LCD_CP_EN, 1);
	udelay(10);
}

static int aat2862_write_reg(struct i2c_client *client, unsigned char reg, unsigned char val)
{
	int err;
	u8 buf[2];
	struct i2c_msg msg = {	
		client->addr, 0, 2, buf 
	};

	buf[0] = reg;
	buf[1] = val;
	
	if ((err = i2c_transfer(client->adapter, &msg, 1)) < 0) {
		dev_err(&client->dev, "i2c write error\n");
	}
	
	return 0;
}

static int aat2862_read_reg(struct i2c_client *client, unsigned char reg, unsigned char *ret)
{
	int err;
	unsigned char buf = reg;

	struct i2c_msg msg[2] = { 
		{ client->addr, 0, 1, &buf },
		{ client->addr, I2C_M_RD, 1, ret}
	};

	if ((err = i2c_transfer(client->adapter, msg, 2)) < 0) {
		dev_err(&client->dev, "i2c read error\n");
	}
	
	return 0;
}

void aat2862_init(struct i2c_client *client) 
{
	//unsigned i;
	gpio_request(LCD_CP_EN, "lcd bl");
	gpio_direction_output(LCD_CP_EN, 1);

	aat2862_hreset();

#if 0
	for (i = 0; pwron_seq[i].reg != I2C_NO_REG; i++) {
		aat2862_write_reg(client, pwron_seq[i].reg, pwron_seq[i].val);
		mdelay(10);
	}
	backlight_status = BL_ON;
#endif
}

void aat2862_ldo_enable(unsigned char num, int enable) 
{
	unsigned char org;

	aat2862_read_reg(aat2862_i2c_client, LDO_ABCD_EN_REG, &org);

	if(enable)
	{
		aat2862_write_reg(aat2862_i2c_client, LDO_ABCD_EN_REG, org | (1<< num));
	}
	else
	{
		aat2862_write_reg(aat2862_i2c_client, LDO_ABCD_EN_REG, org & ~(1<<num));
	}
}
EXPORT_SYMBOL(aat2862_ldo_enable);

static void aat2862_ldo_read(u8 reg , u8 *val)
{
	if (reg < 0 || reg > MAX_LDO_REG - 1)
	{
		printk("[LDO] Hub ldo invalid register access\n");
		return;
	}

	*val = i2c_smbus_read_byte_data(aat2862_i2c_client, reg);
	printk("[LDO] ldo Reg read 0x%X: Val=0x%X\n", (u8)reg, (u8)*val);
}

static int aat2862_ldo_write(u8 reg , u8 val)
{
	int ret =  -1  ;

	if (reg < 0 || reg > MAX_LDO_REG - 1)
	{
		printk("[LDO] Hub ldo invalid register access\n");
		return ret;
	}

	ret = i2c_smbus_write_byte_data(aat2862_i2c_client, reg , val);
	if(ret >= 0)
	{
		mirror_reg[reg] = val;
		printk("[LDO] Hub ldo Write Reg. = 0x%X, Value 0x%X\n", reg, val);
	}

	return ret;
}

static inline int aat2862_clear_n_set(u8 clear, u8 set, u8 reg)
{
	int ret;
	u8 val = 0;

	/* Gets the initial register value */
	aat2862_ldo_read(reg, &val);

	/* Clearing all those bits to clear */
	val &= ~(clear);

	/* Setting all those bits to set */
	val |= set;

	/* Update the register */
	ret = aat2862_ldo_write(reg, val);
	if (ret)
		return ret;

	return 0;
}

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
//	if(!is_enabled)
		return;

	if(enable)
	{
		switch(ldo_num)
		{
			case ENLDO_A:
				aat2862_ldo_write(LDO_AB_LEVEL_REG, level<<4);
				break;
			case ENLDO_B:
				aat2862_ldo_write(LDO_AB_LEVEL_REG, level);
				break;
			case ENLDO_C:
				aat2862_ldo_write(LDO_CD_LEVEL_REG, level<<4);
				break;
			case ENLDO_D:
				aat2862_ldo_write(LDO_CD_LEVEL_REG, level);
				break;
			default:
				printk("[LDO] Hub ldo invalid register access\n");
				return;
		}

		aat2862_clear_n_set(0, 1<<ldo_num, LDO_ABCD_EN_REG);
	}
	else
	{
		aat2862_clear_n_set(1<<ldo_num, 0, LDO_ABCD_EN_REG);
	}
}
EXPORT_SYMBOL(aat2862_set_ldo);

static void aat2862_touch_ldo_enable(struct i2c_client *client, int on)
{
#if 0
	aat2862_set_ldo(1, ENLDO_D, VLDO_3_0V);
	msleep(1);
	aat2862_set_ldo(1, ENLDO_C, VLDO_1_8V);	
#endif
	if(on)
	{
		aat2862_write_reg(client, LDO_ABCD_EN_REG, 0x0B);
		mdelay(5);
		aat2862_write_reg(client, LDO_ABCD_EN_REG, 0x0F);
	}
	else
	{
		aat2862_write_reg(client, LDO_ABCD_EN_REG, 0x0B);
		mdelay(5);
		aat2862_write_reg(client, LDO_ABCD_EN_REG, 0x03);
	}

//	printk(KERN_WARNING"[!] %s()\n", __func__);
}

static void aat2862_set_main_current_level(struct i2c_client *client, int level)
{
	struct aat2862_device *dev;
	unsigned char val;
	
    dev = (struct aat2862_device *)i2c_get_clientdata(client);
	cur_main_lcd_level = level; 
	dev->bl_dev->props.brightness = cur_main_lcd_level;

	aat2862_write_reg(client, 0x0,0xff); /* LCD channel enable */

	val = 0xFF - level;
	//val = 0xC0 | (backlight_status<<5) | level;

	aat2862_write_reg(client, 0x03, val);

	if(level > 0x3F){
		val = 0;
	}else{
		val = 0x3F - level;
	}

	aat2862_write_reg(client, 0x04, val);
	aat2862_write_reg(client, 0x05, val);
	aat2862_write_reg(client, 0x06, val);

	mdelay(1);
}

static void leds_brightness_set(struct led_classdev *led_cdev, enum led_brightness value)
{
    value = value/12;
	if (value > MAX_BRIGHTNESS)
		value = MAX_BRIGHTNESS;
    if(early_bl_timer == 0)
    {
        early_bl_value = value;
        return;
    }
	
	aat2862_set_main_current_level(aat2862_i2c_client, value);
	cur_main_lcd_level = value; 
	
	return;
}

static struct led_classdev lcd_backlight = {
	.name = "lcd-backlight",
	.brightness = MAX_BRIGHTNESS,
	.brightness_set = leds_brightness_set,
};

void aat2862_backlight_on(void)
{
	printk("%s received (prev backlight_status: %s)\n", __func__, backlight_status?"ON":"OFF");
	if (backlight_status == BL_ON) return;

	TOUCH_KEY_BL_ON();

	//aat2862_set_main_current_level(main_aat2862_dev->client, DEFAULT_BRIGHTNESS);
	aat2862_set_main_current_level(main_aat2862_dev->client, early_bl_value);
	backlight_status = BL_ON;

	return;
}
EXPORT_SYMBOL(aat2862_backlight_on);

void aat2862_backlight_off(void)
{
	if (backlight_status == BL_OFF) return;
	saved_main_lcd_level = cur_main_lcd_level;
	aat2862_set_main_current_level(main_aat2862_dev->client, 0);
	backlight_status = BL_OFF;	

	TOUCH_KEY_BL_OFF();

	return;
}
EXPORT_SYMBOL(aat2862_backlight_off);

static int hub_bl_set_intensity(struct backlight_device *bd)
{
	struct i2c_client *client = to_i2c_client(bd->dev.parent);

	aat2862_set_main_current_level(client, bd->props.brightness);
	cur_main_lcd_level = bd->props.brightness; 
	
	return 0;
}

static int hub_bl_get_intensity(struct backlight_device *bd)
{
    struct i2c_client *client = to_i2c_client(bd->dev.parent);
    unsigned char val=0;

    aat2862_read_reg(client, 0x03, &val);
	val &= 0x1f;
    return (int)val;
}

ssize_t lcd_backlight_show_level(struct device *dev, struct device_attribute *attr, char *buf)
{
	int r;

	r = snprintf(buf, PAGE_SIZE, "LCD Backlight Level is : %d\n", cur_main_lcd_level);
	
	return r;
}

ssize_t lcd_backlight_store_level(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	int level;
	struct i2c_client *client = to_i2c_client(dev); 

	if (!count)
		return -EINVAL;
	
	level = simple_strtoul(buf, NULL, 10);
	
	if (level > MAX_BRIGHTNESS)
		level = MAX_BRIGHTNESS;

	aat2862_set_main_current_level(client, level);
	cur_main_lcd_level = level; 
	
	return count;
}

static int aat2862_bl_resume(struct i2c_client *client);
static int aat2862_bl_suspend(struct i2c_client *client, pm_message_t state);


ssize_t lcd_backlight_show_on_off(struct device *dev, struct device_attribute *attr, char *buf)
{
	int r;
	r = snprintf(buf, PAGE_SIZE, "%d\n", backlight_status);
	//printk("%s received (prev backlight_status: %s)\n", __func__, backlight_status?"ON":"OFF");

	return r;
}

ssize_t lcd_backlight_store_on_off(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	int on_off;
	struct i2c_client *client = to_i2c_client(dev); 

	if (!count)
		return -EINVAL;
	
	printk("%s received (prev backlight_status: %s)\n", __func__, backlight_status?"ON":"OFF");
	
	on_off = simple_strtoul(buf, NULL, 10);
	
	printk(KERN_ERR "%d",on_off);
	
	if(on_off == 1){
		early_bl_value=DEFAULT_BRIGHTNESS;
		aat2862_bl_resume(client);
	}else if(on_off == 0)
	    aat2862_bl_suspend(client, PMSG_SUSPEND);
	
	return count;

}

ssize_t gpio_test_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int r;

	printk("gpio_test_show");

	return r;
}

ssize_t gpio_test_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	int num_gpio, num_direction, num_value;
	char *last;
	struct i2c_client *client = to_i2c_client(dev); 

	if (!count)
		return -EINVAL;
	
	
	
	num_gpio = simple_strtoul(buf, &last, 10);
	++last;
	num_direction = simple_strtoul(last, &last, 10);
	++last;
	num_value = simple_strtoul(last, &last, 10);

	
	printk("gpio_test  num_gpio = %d num_direction = %d num_value=%d \n",num_gpio, num_direction, num_value);
	if((1) && (num_direction==0 ||num_direction==1)&& (num_direction==0 ||num_direction==1) )
	{
		gpio_direction_output(num_gpio, num_direction);
		gpio_set_value(num_gpio, num_value);
	}
	else
		printk("wrong number!!!!!!!!!!!\n");
	
	return count;

}

DEVICE_ATTR(level, 0664, lcd_backlight_show_level, lcd_backlight_store_level);
DEVICE_ATTR(backlight_on_off, 0666, lcd_backlight_show_on_off, lcd_backlight_store_on_off);
DEVICE_ATTR(gpio_test, 0666, gpio_test_show, gpio_test_store);

static struct backlight_ops hub_bl_ops = {
	.update_status = hub_bl_set_intensity,
	.get_brightness = hub_bl_get_intensity,
};

static int __init aat2862_probe(struct i2c_client *i2c_dev, const struct i2c_device_id *id)
{
	struct aat2862_device *dev;
	struct backlight_device *bl_dev;
	int err;

	printk(KERN_INFO"%s: i2c probe start\n", __func__);

	aat2862_i2c_client = i2c_dev;

	dev = kzalloc(sizeof(struct aat2862_device), GFP_KERNEL);
	if(dev == NULL) {
		dev_err(&i2c_dev->dev,"fail alloc for aat2862_device\n");
		return 0;
	}

	main_aat2862_dev = dev;

	bl_dev = backlight_device_register(HUB_I2C_BL_NAME, &i2c_dev->dev, NULL, &hub_bl_ops);
	bl_dev->props.max_brightness = MAX_BRIGHTNESS;
	bl_dev->props.brightness = DEFAULT_BRIGHTNESS;
	bl_dev->props.power = FB_BLANK_UNBLANK;
	
	dev->bl_dev = bl_dev;
	dev->client = i2c_dev;
	i2c_set_clientdata(i2c_dev, dev);

	//aat2862_init(i2c_dev);

	aat2862_touch_ldo_enable(i2c_dev, 1);

	led_classdev_register(&i2c_dev->dev, &lcd_backlight);

	aat2862_set_main_current_level(i2c_dev, DEFAULT_BRIGHTNESS); 
	err = device_create_file(&i2c_dev->dev, &dev_attr_level);
	err = device_create_file(&i2c_dev->dev, &dev_attr_backlight_on_off);
	err = device_create_file(&i2c_dev->dev, &dev_attr_gpio_test);

#ifdef CONFIG_HAS_EARLYSUSPEND
    dev->early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN;
    dev->early_suspend.suspend = aat2862_early_suspend;
    dev->early_suspend.resume = aat2862_late_resume;
    register_early_suspend(&dev->early_suspend);
#endif
	gpio_request(LCD_CP_EN, "LCD_CP_EN");
	TOUCH_KEY_BL_INIT();
	TOUCH_KEY_BL_ON();

	return 0;
}

static int aat2862_remove(struct i2c_client *i2c_dev)
{
	struct aat2862_device *dev = i2c_get_clientdata(i2c_dev);

	unregister_early_suspend(&dev->early_suspend);
	gpio_free(LCD_CP_EN);
 	device_remove_file(&i2c_dev->dev, &dev_attr_level);
 	device_remove_file(&i2c_dev->dev, &dev_attr_backlight_on_off);
 	device_remove_file(&i2c_dev->dev, &dev_attr_gpio_test);
	dev = (struct aat2862_device *)i2c_get_clientdata(i2c_dev);
	backlight_device_unregister(dev->bl_dev);
	i2c_set_clientdata(i2c_dev, NULL);

	return 0;
}	

static int aat2862_suspend(struct i2c_client *client, pm_message_t state)
{
	printk(KERN_INFO"%s: new state: %d\n",__func__, state.event);

	client->dev.power.power_state = state;
	
	aat2862_touch_ldo_enable(client, 0);
	mdelay(5);

	aat2862_write_reg(client, LDO_ABCD_EN_REG, 0x00);

	gpio_direction_output(LCD_CP_EN, 0);

	return 0;
}

#ifdef CONFIG_HAS_EARLYSUSPEND
static int aat2862_bl_suspend(struct i2c_client *client, pm_message_t state)
{
    printk(KERN_INFO"%s: new state: %d\n",__func__, state.event);

    //client->dev.power.power_state = state;
    aat2862_backlight_off();
    aat2862_write_reg(client, 0x03, 0x0F);
    aat2862_write_reg(client, 0x04, 0x0F);
    aat2862_write_reg(client, 0x05, 0x0F);
    aat2862_write_reg(client, 0x06, 0x0F);
    early_bl_timer = 0;
    return 0;
}
#endif

static int aat2862_resume(struct i2c_client *client)
{
	aat2862_init(client);
	printk(KERN_INFO"%s: old state: %d\n",__func__, client->dev.power.power_state.event);
	client->dev.power.power_state = PMSG_ON;
	aat2862_write_reg(client, LDO_AB_LEVEL_REG, 0x4C);
	aat2862_write_reg(client, LDO_CD_LEVEL_REG, 0x4C);
	
	aat2862_write_reg(client, LDO_ABCD_EN_REG, 0x03);
	mdelay(5);
	aat2862_touch_ldo_enable(client, 1);
//	aat2862_write_reg(client, LDO_ABCD_EN_REG, 0x0F);
//	mdelay(10);
	//aat2862_backlight_on();

	return 0;
}

#ifdef CONFIG_HAS_EARLYSUSPEND
static int aat2862_bl_resume(struct i2c_client *client)
{
    early_bl_timer = 1;
    aat2862_backlight_on();
    
    return 0;
}

static void aat2862_early_suspend(struct early_suspend *h)
{
    struct aat2862_device *dev;
 
    dev = container_of(h, struct aat2862_device, early_suspend);
    aat2862_bl_suspend(dev->client, PMSG_SUSPEND);
}

static void aat2862_late_resume(struct early_suspend *h)
{
    struct aat2862_device *dev;
 
    dev = container_of(h, struct aat2862_device, early_suspend);
    aat2862_bl_resume(dev->client);
}
#endif

static struct i2c_driver main_aat2862_driver = {
	.probe = aat2862_probe,
	.remove = aat2862_remove,
	.suspend = aat2862_suspend,
	.resume = aat2862_resume,
	.id_table = hub_bl_id, 
	.driver = {
		.name = HUB_I2C_BL_NAME,
	},
};


static int __init lcd_backlight_init(void)
{
	static int err=0;

	err = i2c_add_driver(&main_aat2862_driver);

	return err;
}
 
module_init(lcd_backlight_init);

MODULE_DESCRIPTION("AAT2862 Backlight Control");
MODULE_AUTHOR("kyungtae Oh <kyungtae.oh@lge.com>");
MODULE_LICENSE("GPL");
