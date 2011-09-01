/* 
 * arch/arm/mach-msm/lge/lge_emmc_direct_access.c
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
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/moduleparam.h>

#include "version_F1.h"

static atomic_t enable = ATOMIC_INIT(1);


static ssize_t version_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	return sprintf(buf, LGVER_SW_VERSION);
}
static ssize_t fboot_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int bootinfo =atomic_read(&enable);
	
	return sprintf(buf, "%d\n",  bootinfo);

}

static ssize_t fboot_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
        int value;
		
        sscanf(buf, "%d", &value);
        atomic_set(&enable, value);

        return size;
}

 /*
 * /sys/devices/platform/lge_emmc_direct_access/version
 * /sys/devices/platform/lge_emmc_direct_access/boot_info
 */
 
static DEVICE_ATTR(version, 0444, version_show, NULL);
static DEVICE_ATTR(boot_info, 0666, fboot_show, fboot_store);

static int lge_emmc_direct_access_probe(struct platform_device *pdev)
{
	int ret;
	
	ret = device_create_file(&pdev->dev, &dev_attr_version);
	if (ret < 0) {
		printk("lge_emmc_direct_access: failed to create sysfs\n");
	}

	ret = device_create_file(&pdev->dev, &dev_attr_boot_info);
	if (ret < 0) {
		printk("lge_emmc_direct_access: failed to create sysfs\n");
	}

	
	return ret;
}

static struct platform_device lge_emmc_direct_access_device = {
	.name = "lge_emmc_direct_access",
	.id = -1,
};

static struct platform_driver lge_emmc_direct_access_driver = {
	.probe = lge_emmc_direct_access_probe,
	.driver = {
	.name = "lge_emmc_direct_access",
	},
};

static int __init lge_emmc_direct_access_init(void)
{
	int ret;
	
	printk(KERN_INFO"%s: finished\n", __func__);
	
	ret = platform_device_register(&lge_emmc_direct_access_device);
	if (ret < 0) {
		printk("lge_emmc_direct_access: failed to register platform device\n");
		goto failed;
	}
	
	ret = platform_driver_register(&lge_emmc_direct_access_driver);
	if (ret < 0) {
		printk("lge_emmc_direct_access: failed to register platform driver\n");
		goto failed2;
	}
	
	printk("lge_emmc_direct_access: init success\n");
	
	return 0;
		
failed2:
	platform_device_unregister(&lge_emmc_direct_access_device);
failed:
	return ret;
}

static void __exit lge_emmc_direct_access_exit(void)
{
	platform_driver_unregister(&lge_emmc_direct_access_driver);
	platform_device_unregister(&lge_emmc_direct_access_device);
}

module_init(lge_emmc_direct_access_init);
module_exit(lge_emmc_direct_access_exit);

MODULE_DESCRIPTION("LGE emmc direct access apis");
MODULE_AUTHOR("SeHyun Kim <sehyuny.kim@lge.com>");
MODULE_LICENSE("GPL");
