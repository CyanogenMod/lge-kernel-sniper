/*
 *  drivers/muic/muic_client_class.c
 *
 * Copyright (C) 2011 LG Electronics, Inc.
 * Author: Seungho Park <seungho1.park@lge.com>
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

#include <asm/gpio.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/err.h>
#include <linux/delay.h>		/* usleep() */
#include <linux/i2c.h>
#include <linux/muic/muic.h>
#include <linux/muic/muic_client.h>
#include <linux/charger_rt9524.h>

struct class *muic_client_class;
static atomic_t device_count;

static int muic_notifier_callback(struct notifier_block *self,
				 unsigned long event, void *data)
{
	struct muic_client_device *mcdev;
	//struct muic_device *mdev = data;

	mcdev = container_of(self, struct muic_client_device, muic_notif);
	
	mcdev->mode = event;
	
	if (!mcdev->ops)
		return 0;
	
	mutex_lock(&mcdev->ops_lock);
	
	switch (event) {
	case MUIC_NONE:
		if(mcdev->ops->on_none) {
			mcdev->ops->on_none(mcdev);
		}
		break;
	case MUIC_NA_TA:
		if(mcdev->ops->on_na_ta) {
			mcdev->ops->on_na_ta(mcdev);
		}
		break;
	case MUIC_LG_TA:
		if(mcdev->ops->on_lg_ta) {
			mcdev->ops->on_lg_ta(mcdev);
		}
		break;
	case MUIC_AP_UART:
		if(mcdev->ops->on_ap_uart) {
			mcdev->ops->on_ap_uart(mcdev);
		}
		break;
	case MUIC_AP_USB:
		if(mcdev->ops->on_ap_usb) {
			mcdev->ops->on_ap_usb(mcdev);
		}
		break;
	case MUIC_CP_UART:
		if(mcdev->ops->on_cp_uart)
			mcdev->ops->on_cp_uart(mcdev);
		break;
	case MUIC_CP_USB:
		if(mcdev->ops->on_cp_usb)
			mcdev->ops->on_cp_usb(mcdev);
		break;
	case MUIC_MHL:
		if(mcdev->ops->on_mhl) {
			mcdev->ops->on_mhl(mcdev);
		}
		break;
	default:
		break;
	}
	
	mutex_unlock(&mcdev->ops_lock);
	
	return 0;
}

static int muic_client_register(struct muic_client_device *mcdev)
{
	memset(&mcdev->muic_notif, 0, sizeof(mcdev->muic_notif));
	mcdev->muic_notif.notifier_call = muic_notifier_callback;
	muic_register_client(mcdev);
	
	return 0;
}

/*
static int create_muic_client_class(void)
{
	if (!muic_client_class) {
		muic_client_class = class_create(THIS_MODULE, "muic-client");
		if (IS_ERR(muic_client_class))
			return PTR_ERR(muic_client_class);
		atomic_set(&device_count, 0);
	}

	return 0;
}
*/

static void muic_client_device_release(struct device *dev)
{
    struct muic_client_device *mcdev = to_muic_client_device(dev);
    kfree(mcdev);
}

int muic_client_dev_register(const char *name, void *devdata, struct muic_client_ops *ops)
{
	int ret;
	struct muic_client_device *new_mcdev;

	new_mcdev = kzalloc(sizeof(struct muic_client_device), GFP_KERNEL);
	if (!new_mcdev)
		return -ENOMEM;

	new_mcdev->dev.class = muic_client_class;
	new_mcdev->dev.parent = NULL;
	new_mcdev->dev.release = muic_client_device_release;
	new_mcdev->ops = ops;
	mutex_init(&new_mcdev->ops_lock);
	dev_set_name(&new_mcdev->dev, name);
	dev_set_drvdata(&new_mcdev->dev, devdata);

	ret = device_register(&new_mcdev->dev);
	if (ret) {
		kfree(new_mcdev);
		return ret;
	}

	ret = muic_client_register(new_mcdev);
	if (ret) {
		device_unregister(&new_mcdev->dev);
		return ret;
	}

	return ret;
}
EXPORT_SYMBOL_GPL(muic_client_dev_register);

void muic_client_dev_unregister(struct muic_client_device *mcdev)
{
	//device_remove_file(mcdev->dev, &dev_attr_name);
	//device_remove_file(mcdev->dev, &dev_attr_state);
	device_destroy(muic_client_class, MKDEV(0, mcdev->index));
	dev_set_drvdata(&mcdev->dev, NULL);
}
EXPORT_SYMBOL_GPL(muic_client_dev_unregister);

static void __exit muic_client_class_exit(void)
{
	class_destroy(muic_client_class);
}

static int __init muic_client_class_init(void)
{
	muic_client_class = class_create(THIS_MODULE, "muic-client");
	if (IS_ERR(muic_client_class)) {
		printk(KERN_WARNING "Unable to create muic client class; errno = %ld\n",
				PTR_ERR(muic_client_class));
		return PTR_ERR(muic_client_class);
	}

	//muic_class->dev_attrs = muic_device_attributes;
	//muic_class->suspend = muic_suspend;
	//muic_class->resume = muic_resume;
	return 0;
}

subsys_initcall(muic_client_class_init);
module_exit(muic_client_class_exit)

MODULE_AUTHOR("Seungho Park <seungho1.park@lge.com>");
MODULE_DESCRIPTION("MUIC client class driver");
MODULE_LICENSE("GPL v2");
