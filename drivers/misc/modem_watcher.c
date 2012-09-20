/*
 * Modem Watcher
 *
 * Copyright (C) 2011 LGE, Inc.
 *
 * Author: Jugwan Eom <jugwan.eom@lge.com>
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

#include <linux/device.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/input.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/workqueue.h>
#include <linux/gpio.h>
#include <linux/slab.h> /* XXX */
#include <linux/string.h>

#include <linux/lge/mdm_watcher.h>

struct mdm_watcher_drv_event {
	unsigned int irq;
	unsigned long irqf_flags; /* XXX */
	int delay;
	unsigned int code; /* key code to upper layer */
};

struct mdm_watcher_data {
	struct input_dev *input;

	/* TODO: use arrary of events? */
	struct mdm_watcher_drv_event halt;
	struct mdm_watcher_drv_event auto_shutdown;

	mdm_event_type event_type;
	int enable;
	struct delayed_work delayed_work;
	struct mutex lock; /* TODO: use spinlock? */
};

static irqreturn_t mdm_watcher_interrupt(int irq, void *dev_id)
{
	struct mdm_watcher_data *handler = dev_id;

	if (handler->enable) {
		unsigned long delay = 0;
		/* XXX: comparison between signed and unsigned
		 * But this seems to be safe because irq > 0
		 */
		if (irq == handler->halt.irq) {
				handler->event_type = MDM_HALT;
				delay = handler->halt.delay;
		} else if (irq == handler->auto_shutdown.irq) {
				handler->event_type = MDM_AUTO_SHUTDOWN;
				delay = handler->auto_shutdown.delay;
		} else
				dev_err(&handler->input->dev, "wrong irq number\n");

		schedule_delayed_work(&handler->delayed_work, delay);
	}

	return IRQ_HANDLED;
}

static void mdm_watcher_work_func(struct work_struct *work)
{
	struct delayed_work *delayed_work = container_of(work, struct delayed_work, work);
	struct mdm_watcher_data *handler =
		container_of(delayed_work, struct mdm_watcher_data, delayed_work);

	if (handler->enable) {
		unsigned int code = 0;
		/* TODO: get and check state? */
		switch (handler->event_type) {
			case MDM_HALT:
				code = handler->halt.code; 
				break;
			case MDM_AUTO_SHUTDOWN:
				code = handler->auto_shutdown.code; 
				break;
			default:
				dev_err(&handler->input->dev, "wrong event type\n");
		}
		input_report_key(handler->input, code, 1);
		input_report_key(handler->input, code, 0);
		input_sync(handler->input);
		handler->event_type = MDM_EVENT_MAX;
		printk("mdm_watcher: report event %u\n", code);
	}
}

static ssize_t mdm_watcher_show_enable(struct device *dev, struct device_attribute *attr, char *buf)
{
	int result = 0;
	struct mdm_watcher_data *handler = dev_get_drvdata(dev);

	result  = snprintf(buf, PAGE_SIZE, "%s\n", (handler->enable)  ? "1":"0");

	return result;
}

static ssize_t mdm_watcher_store_enable(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	int enable;
	struct mdm_watcher_data *handler = dev_get_drvdata(dev);

	if (!count)
		return -EINVAL;

	mutex_lock(&handler->lock);
	sscanf(buf, "%d", &enable);

	if (enable) {
		handler->enable = 1;
	} else {

		handler->enable = 0;
	}
	mutex_unlock(&handler->lock);

	return count;
}

static DEVICE_ATTR(enable, 0664, mdm_watcher_show_enable, mdm_watcher_store_enable);

static struct attribute *mdm_watcher_attributes[] = {
	&dev_attr_enable.attr,
	NULL,
};

static const struct attribute_group mdm_watcher_group = {
	.attrs = mdm_watcher_attributes,
};

static int mdm_watcher_probe(struct platform_device *pdev)
{
	struct mdm_watcher_platform_data *pdata = pdev->dev.platform_data;
	struct mdm_watcher_data *handler; 
	struct device *dev_sys = &pdev->dev;
	int ret = 0;
	int i;

	handler = kzalloc(sizeof(struct mdm_watcher_data), GFP_KERNEL);
	if (!handler) {
		ret = -ENOMEM;
		goto fail_malloc;
	}

	handler->input = input_allocate_device();
	if (!handler->input) {
		ret = -ENOMEM;
		goto fail_input_allocate;
	}
	handler->input->name = "modem_watcher";
	set_bit(EV_KEY, handler->input->evbit);
	set_bit(EV_SYN, handler->input->evbit);

	handler->enable= 1;
	INIT_DELAYED_WORK(&handler->delayed_work, mdm_watcher_work_func);

	for (i = 0; i < pdata->len; i++) {
		char *event_name = NULL;
		struct mdm_watcher_drv_event *drv_event;
		struct mdm_watcher_event *pdev_event;

		pdev_event = &pdata->event[i];
		switch(pdev_event->type) {
			case MDM_HALT:
				drv_event = &handler->halt;
				//sprintf(event_name, "mdm_watcher_halt");
				event_name = "mdm_watcher_halt";
				break;
			case MDM_AUTO_SHUTDOWN:
				drv_event = &handler->auto_shutdown;
				//sprintf(event_name, "mdm_watcher_auto_shutdown");
				event_name = "mdm_watcher_auto_shutdown";
				break;
			default:
				dev_err(&pdev->dev, "wrong event index\n");
				continue;
		}

		gpio_request(pdev_event->gpio_irq, event_name);
		gpio_direction_input(pdev_event->gpio_irq);

		drv_event->irq = gpio_to_irq(pdev_event->gpio_irq);
		drv_event->delay = msecs_to_jiffies(pdev_event->msecs_delay);
		drv_event->code = pdev_event->key_code;
		drv_event->irqf_flags = pdev_event->irqf_flags;
		set_bit(drv_event->code, handler->input->keybit);

		ret = request_irq(drv_event->irq, mdm_watcher_interrupt,
				drv_event->irqf_flags, event_name, handler);
		if (ret) {
			dev_err(&pdev->dev, "failed to register interrupt, %u\n",
					drv_event->irq);
			/* TODO: error handling*/
		}

		ret = enable_irq_wake(drv_event->irq);
		if (ret) {
			dev_err(&pdev->dev, "failed to enable irq wakeup, %u\n",
					drv_event->irq);
			free_irq(drv_event->irq, NULL);
		}
		printk("mdm_watcher: event %d, code %u\n",
				pdev_event->type, pdev_event->key_code);
	}

	if (sysfs_create_group(&dev_sys->kobj, &mdm_watcher_group)) {
                dev_err(&pdev->dev, "failed to create sysfs\n");
	 }

	 mutex_init(&handler->lock);

	 ret = input_register_device(handler->input);
	 if (ret) {
		 dev_err(&pdev->dev, "failed to register input device\n");
		 goto fail_input_register;
	 }

	 platform_set_drvdata(pdev, handler);

	 printk("mdm_watcher: successfully initialized\n");

	 return 0;

#if 0 /* TODO: error handling */
fail_request_irq:
	 input_unregister_device(handler->input);
#endif
fail_input_register:
	 input_free_device(handler->input);
fail_input_allocate:
	 kfree(handler);
fail_malloc:
	 //gpio_free(pdata->gpio_irq);

	 printk("modem_watcher: there is an error in probing\n");
	 return ret;
}

static int mdm_watcher_remove(struct platform_device *pdev)
{
	int ret = 0;

	struct mdm_watcher_data *handler = platform_get_drvdata(pdev);

	if (!handler->halt.irq)
		free_irq(handler->halt.irq, NULL);
	if (!handler->auto_shutdown.irq)
		free_irq(handler->auto_shutdown.irq, NULL);

	input_unregister_device(handler->input);
	input_free_device(handler->input);
	kfree(handler);

	if (!handler->halt.irq)
		gpio_free(irq_to_gpio(handler->halt.irq));
	if (!handler->auto_shutdown.irq)
		gpio_free(irq_to_gpio(handler->auto_shutdown.irq));

	return ret;
}

static struct platform_driver mdm_watcher_driver = {
	.probe		= mdm_watcher_probe,
	.remove		= __devexit_p(mdm_watcher_remove),
	.driver		= {
		.name	= MDM_WATCHER_NAME,
		.owner	= THIS_MODULE,
	},
};

static int __init mdm_watcher_init(void)
{
	return platform_driver_register(&mdm_watcher_driver);
}
module_init(mdm_watcher_init);


static void __exit mdm_watcher_exit(void)
{
	platform_driver_unregister(&mdm_watcher_driver);
}
module_exit(mdm_watcher_exit);

MODULE_AUTHOR("LG Electronics");
MODULE_DESCRIPTION("Modem Watcher Driver");
MODULE_LICENSE("GPL");

