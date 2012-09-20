/*
 * B-Project GPS GPIO Control driver
 *
 * Copyright (C) 2010 LGE, Inc.
 *
 * Author: Daewung Kim <daewung.kim@lge.com>
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

#include <linux/kernel.h>
#include <linux/device.h>

#include <linux/platform_device.h>
#include <mach/hardware.h>
#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <mach/gpio.h>

#include "../mux.h"


#define GPIO_GPS_PWR_ON		152
#define GPIO_GPS_RESET_N	153

static ssize_t gps_gpio_reset_show(struct device *dev, 
		struct device_attribute *attr, char *buf)
{
	int value;
	printk(KERN_INFO "gps_gpio_reset_show\n");

	value = gpio_get_value(GPIO_GPS_RESET_N);

	return sprintf(buf, "%d\n", value);
}

static ssize_t gps_gpio_reset_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	int value;
	printk(KERN_INFO "gps_gpio_reset_store\n");

	sscanf(buf, "%d", &value);

	gpio_direction_output(GPIO_GPS_RESET_N, value);

	return size;
}



static ssize_t gps_gpio_poweron_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	int value;
	printk(KERN_INFO "gps_gpio_poweron_show\n");

	value = gpio_get_value(GPIO_GPS_PWR_ON);

	return sprintf(buf, "%d\n", value);
}

static ssize_t gps_gpio_poweron_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	int value;
	printk(KERN_INFO "gps_gpio_poweron_store\n");

	sscanf(buf, "%d", &value);

	gpio_direction_output(GPIO_GPS_PWR_ON, value);

	return size;
}

static DEVICE_ATTR(reset, S_IRUGO | S_IWUSR, gps_gpio_reset_show, gps_gpio_reset_store);
static DEVICE_ATTR(poweron, S_IRUGO | S_IWUSR, gps_gpio_poweron_show, gps_gpio_poweron_store);


static int hub_gps_gpio_probe(struct platform_device *pdev)
{
	int retval = 0;

	printk(KERN_INFO "hub_gps_gpio_probe\n");

	omap_mux_init_gpio(GPIO_GPS_PWR_ON,  OMAP_PIN_OUTPUT);
	omap_mux_init_gpio(GPIO_GPS_RESET_N, OMAP_PIN_OUTPUT);

	retval = gpio_request(GPIO_GPS_PWR_ON,  "GPS power on GPIO");
	if (retval)
	{
		printk(KERN_ERR "hub_gps_gpio_probe: GPIO %d is already used!\n", GPIO_GPS_PWR_ON);
		return retval;
	}
	
	retval = gpio_request(GPIO_GPS_RESET_N, "GPS reset GPIO");
	if (retval)
	{
		printk(KERN_ERR "hub_gps_gpio_probe: GPIO %d is already used!\n", GPIO_GPS_RESET_N);
		return retval;
	}

	retval = device_create_file(&pdev->dev, &dev_attr_reset);
	if (retval)
		goto error;
	
	retval = device_create_file(&pdev->dev, &dev_attr_poweron);
	if (retval)
		goto error;

  /* LGE_CHANGE_S, ericjs.kim@lge.com, 2011-05-08, after root -> system, fix for first try dead  */		
	gpio_direction_output(GPIO_GPS_PWR_ON, 0);	 
	gpio_direction_output(GPIO_GPS_RESET_N, 1);	
	//gpio_direction_output(GPIO_GPS_RESET_N, 0);
	/* LGE_CHANGE_E, ericjs.kim@lge.com, 2011-05-08, after root -> system, fix for first try dead  */	
	
	return 0;

error:
	printk(KERN_ERR "hub_gps_gpio_probe: Cannot create file desc.!\n");
	device_remove_file(&pdev->dev, &dev_attr_reset);
	device_remove_file(&pdev->dev, &dev_attr_poweron);
	
	return retval;
}



static int hub_gps_gpio_remove(struct platform_device *pdev)
{
	printk(KERN_INFO "hub_gps_gpio_remove\n");
	device_remove_file(&pdev->dev, &dev_attr_reset);
	device_remove_file(&pdev->dev, &dev_attr_poweron);
	return 0;
}


// platform_driver
static struct platform_driver hub_gps_gpio_driver = {
    .probe      = hub_gps_gpio_probe,
    .remove     = hub_gps_gpio_remove,
    .driver     = {
        .name   = "hub_gps_gpio",
				.owner  = THIS_MODULE
		},
};


static int __devinit gps_gpio_init(void)
{
	printk(KERN_INFO "gps_gpio_init\n");
	return platform_driver_register(&hub_gps_gpio_driver);
}

static void __exit gps_gpio_exit(void)
{
	printk(KERN_INFO "gps_gpio_exit\n");
	platform_driver_unregister(&hub_gps_gpio_driver);
}

module_init(gps_gpio_init);
module_exit(gps_gpio_exit);

MODULE_AUTHOR("LG Electronics");
MODULE_DESCRIPTION("GPS GPIO Controller");
MODULE_LICENSE("GPL v2");
