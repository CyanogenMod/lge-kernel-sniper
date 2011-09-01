/* drivers/video/backlight/leds_sc654.c
 *
 * (c) 2010 LG Electronics
 *	Ben Dooks <ben@simtec.co.uk>
 *
 * SC654 - LEDs GPIO driver
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
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
#include <linux/leds.h>
#include <plat/board.h>

#include <linux/i2c/twl.h>

#include <linux/earlysuspend.h>

#ifndef DEBUG
//#define DEBUG
//#undef DEBUG
#endif

struct sc654_led_device {
	struct led_classdev led_dev;
	struct early_suspend early_suspend;
	uint8_t key_led1_level;
	uint8_t key_led2_level;
	uint8_t key_led_onoff;
};

#define TWL_LEDA (OMAP_MAX_GPIO_LINES + TWL4030_GPIO_MAX)
#define RGB_LED_CNTL 128
#define USER_KEY_LVL_MAX 	31
#define MODULE_NAME		"led-sc654"

static unsigned touch_key_bl_gpio;
extern unsigned int system_rev;

#ifdef DEBUG
#define DBG(fmt, args...) 				\
	printk(KERN_DEBUG "[%s] %s(%d): " 		\
		fmt, MODULE_NAME, __func__, __LINE__, ## args); 
#else	/* DEBUG */
#define DBG(...) 
#endif

static void key_led_data_write(unsigned char address, unsigned char data)
{	
	if (address >5)
		return;
	
	local_irq_disable();
	gpio_set_value(touch_key_bl_gpio, 0);
	udelay(1);
	while(address--) 
	{
		gpio_set_value(touch_key_bl_gpio, 1);
		udelay(1);
		gpio_set_value(touch_key_bl_gpio, 0);
		udelay(1);
	}
	gpio_set_value(touch_key_bl_gpio, 1);
	local_irq_enable();
	udelay(560);
	local_irq_disable();
	gpio_set_value(touch_key_bl_gpio, 0);
	udelay(1);
	while(data--) {
		gpio_set_value(touch_key_bl_gpio, 1);
		udelay(1);
		gpio_set_value(touch_key_bl_gpio, 0);
		udelay(1);
	}
	gpio_set_value(touch_key_bl_gpio, 1);
	local_irq_enable();
	udelay(560);	
	DBG("address = %d data= %d \n",address,data);
}

static void key_led_data_init(struct sc654_led_device *led_data)
{
	touch_key_bl_gpio = RGB_LED_CNTL;
	
	if (0 > gpio_request(touch_key_bl_gpio, "touch_keybl"))
		printk (KERN_ERR "touch_keybl gpio_request failed\n");
	
		gpio_direction_output(touch_key_bl_gpio, 1);
		mdelay(1);
		
		key_led_data_write(0,0x1f);
		key_led_data_write(5,0x05);
		key_led_data_write(4,0x03);
}

static void key_led1_set(struct sc654_led_device *led_data)
{
	if (!led_data->key_led_onoff)
	{
		key_led_data_write(0,0x1f);
		key_led_data_write(5,0x05);
		key_led_data_write(4,0x03);
	}
	
	key_led_data_write(1,led_data->key_led1_level);
	led_data->key_led_onoff=1;
}

static void key_led2_set(struct sc654_led_device *led_data)
{
	key_led_data_write(2,led_data->key_led2_level);
}

static void key_led_off(struct sc654_led_device *led_data)
{
	gpio_set_value(touch_key_bl_gpio, 0);
	mdelay(11);
	led_data->key_led_onoff=0;
}

ssize_t key_led1_show_level(struct device *dev, 
			 struct device_attribute *attr, 
			 char *buf)
{
	struct sc654_led_device *led_data = dev_get_drvdata(dev);
	int r;

	r = snprintf(buf, PAGE_SIZE,
			"%d\n", led_data->key_led1_level);

	DBG("buf = %s\n", buf);
	
	return r;
}

ssize_t key_led1_store_level(struct device *dev, 
			  struct device_attribute *attr, 
			  const char *buf, 
			  size_t count)
{
	struct sc654_led_device *led_data = dev_get_drvdata(dev);
	int level;

	if (!count)
		return -EINVAL;

//	DBG("buf = %d\n",*buf);
	
	level = simple_strtoul(buf, NULL, 10);

	
	if (level > USER_KEY_LVL_MAX)
		level = USER_KEY_LVL_MAX;

//	DBG("level = %d\n", level);

	if (level ==led_data->key_led1_level)
		return count;

	led_data->key_led1_level=level;

	key_led1_set(led_data); 
	
	return count;
}

ssize_t key_led2_show_level(struct device *dev, 
			 struct device_attribute *attr, 
			 char *buf)
{
	struct sc654_led_device *led_data = dev_get_drvdata(dev);
	int r;

	r = snprintf(buf, PAGE_SIZE,
			"%d\n", led_data->key_led2_level);

	DBG("buf = %s\n", buf);
	
	return r;
}

ssize_t key_led2_store_level(struct device *dev, 
			  struct device_attribute *attr, 
			  const char *buf, 
			  size_t count)
{
	struct sc654_led_device *led_data = dev_get_drvdata(dev);
	int level;

	if (!count)
		return -EINVAL;

//	DBG("buf = %d\n",*buf);
	
	level = simple_strtoul(buf, NULL, 10);

	
	if (level > USER_KEY_LVL_MAX)
		level = USER_KEY_LVL_MAX;

//	DBG("level = %d\n", level);

	if (level ==led_data->key_led2_level)
		return count;

	led_data->key_led2_level=level;
	
	key_led2_set(led_data); 
	
	return count;
}

ssize_t fade_state_store(struct device *dev, 
			  struct device_attribute *attr, 
			  const char *buf, 
			  size_t count)
{
	struct sc654_led_device *led_data = dev_get_drvdata(dev);
	int onoff;

	if (!count)
		return -EINVAL;

	DBG("buf = %d\n",*buf);
	
	onoff = simple_strtoul(buf, NULL, 10);

	DBG("onoff = %d\n", onoff);

	if (onoff)
	{
		key_led_data_write(4,0x3);/*fade on*/	
	}
	else
	{
		key_led_data_write(4,0x0);/*fade on*/				
	}	
	
	return count;
}

DEVICE_ATTR(led_level_1, 0666, key_led1_show_level, key_led1_store_level);
DEVICE_ATTR(led_level_2, 0666, key_led2_show_level, key_led2_store_level);
DEVICE_ATTR(fade_state, 0666, NULL, fade_state_store);


static void led_sc654_set(struct led_classdev *led_dev,
			     enum led_brightness value)
{
	struct sc654_led_device *led_data;

	led_data->key_led1_level=value;
	led_data->key_led2_level=value;	
	
	key_led1_set(led_data);
	key_led2_set(led_data);
}

static int led_sc654_resume(struct platform_device *pdev)
{
	DBG("led_sc654_resume");
	return 0;
}

static int led_sc654_suspend(struct platform_device *pdev, pm_message_t mesg)
{
	DBG("led_sc654_suspend");
	return 0;
}

static int led_sc654_remove(struct platform_device *pdev)
{
	struct sc654_led_device *led_data;
	led_classdev_unregister(&led_data->led_dev);
	unregister_early_suspend(&led_data->early_suspend);

	device_remove_file(led_data->led_dev.dev, &dev_attr_led_level_1);
	device_remove_file(led_data->led_dev.dev, &dev_attr_led_level_2);
	device_remove_file(led_data->led_dev.dev, &dev_attr_fade_state);
	return 0;
}

static int led_sc654_early_suspend(struct early_suspend *h)
{
	struct sc654_led_device *dev;

	dev = container_of(h, struct sc654_led_device, early_suspend);
	key_led_off(dev);
	DBG("led_sc654_early_suspend");
}

static int led_sc654_late_resume(struct early_suspend *h)
{
	struct sc654_led_device *dev;

	dev = container_of(h, struct sc654_led_device, early_suspend);

	key_led_data_init(dev);
	key_led1_set(dev);
	key_led2_set(dev);
	DBG("led_sc654_late_resume");
}


static int __init led_sc654_probe(struct platform_device *pdev)
{
	struct sc654_led_device *led_data;
	int error;

	led_data = kzalloc(sizeof(struct sc654_led_device), GFP_KERNEL);

	if (led_data == NULL) {
		error = -ENOMEM;
		goto err_alloc_data_failed;
	}

	led_data->led_dev.name = MODULE_NAME;
	led_data->led_dev.brightness_set = led_sc654_set;

	led_data->key_led1_level =28;
	led_data->key_led2_level =8;
	led_data->key_led_onoff=1;

	error = led_classdev_register(&pdev->dev, &led_data->led_dev);
	if (error<0)
		goto err;

	error = device_create_file(led_data->led_dev.dev, &dev_attr_led_level_1);
	if (error < 0) {
		pr_err("%s:File device creation failed: %d\n", __func__, error);
		error = -ENODEV;
		goto err_create_file_level1_failed;
	}

	error = device_create_file(led_data->led_dev.dev, &dev_attr_led_level_2);
	if (error < 0) {
		pr_err("%s:File device creation failed: %d\n", __func__, error);
		error = -ENODEV;
		goto err_create_file_level2_failed;
	}

	error = device_create_file(led_data->led_dev.dev, &dev_attr_fade_state);
	if (error < 0) {
		pr_err("%s:File device creation failed: %d\n", __func__, error);
		error = -ENODEV;
		goto err_create_file_fade_failed;
	}

	key_led_data_init(led_data);
	key_led1_set(led_data);
	key_led2_set(led_data);
	DBG("led_sc654_probe");
#ifdef CONFIG_HAS_EARLYSUSPEND
	led_data->early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN+1 ;
	led_data->early_suspend.suspend = led_sc654_early_suspend;
	led_data->early_suspend.resume = led_sc654_late_resume;
	register_early_suspend(&led_data->early_suspend);
#endif

      	return 0;

err:
	led_classdev_unregister(&led_data->led_dev);
err_create_file_level1_failed:
	device_remove_file(led_data->led_dev.dev, &dev_attr_led_level_1);
err_create_file_level2_failed:
	device_remove_file(led_data->led_dev.dev, &dev_attr_led_level_2);
err_create_file_fade_failed:
	device_remove_file(led_data->led_dev.dev, &dev_attr_fade_state);
err_alloc_data_failed:
	return error;
}

static struct platform_driver led_sc654_driver = {
	.driver		= {
		.name	= "led_sc654",
		.owner	= THIS_MODULE,
	},
	.probe		= led_sc654_probe,
	.remove		= led_sc654_remove,
	.suspend		= led_sc654_suspend,
	.resume		= led_sc654_resume,
};

#if 1
void __init led_sc654_init(void)
{
	platform_driver_register(&led_sc654_driver);
}
#else
static int __init led_sc654_init(void)
{
	return platform_driver_register(&led_sc654_driver);
}

static void __exit led_sc654_exit(void)
{
	platform_driver_unregister(&led_sc654_driver);
}

module_init(led_sc654_init);
module_exit(led_sc654_exit);
#endif

MODULE_AUTHOR("KyungYoon.Kim@lge.com>");
MODULE_DESCRIPTION("LED driver for the SC654");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:led-sc654");

