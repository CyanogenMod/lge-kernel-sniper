/*
 * Copyright (C) 2009 Motorola, Inc.
 * Copyright (C) 2009 Google, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 * 02111-1307, USA
 */

#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/switch.h>
#include <linux/workqueue.h>
#include <linux/slab.h>

#include <linux/bu52014hfv.h>

struct bu52014hfv_info {
	int gpio_north;
	int gpio_south;

	int irq_north;
	int irq_south;

	struct work_struct irq_north_work;
	struct work_struct irq_south_work;

	struct workqueue_struct *work_queue;
	struct switch_dev sdev;

	unsigned int north_value;
	unsigned int south_value;
};
enum {
	NO_DOCK,
	DESK_DOCK,
	CAR_DOCK,
};
#ifdef LG_MUIC_DESK_CRADLE
/* LGE_CHANGE_START 2011-03-14 kenneth.kang@lge.com Cradle detected by muic */
struct bu52014hfv_info *desk_cradle_info = NULL;
/* LGE_CHANGE_END 2011-03-14 kenneth.kang@lge.com Cradle detected by muic */
#endif
#define FEATURE_LGE_HOLE_SENSOR_HIDDEN_MENU	 //20101221 seven.kim@lge.com for hole sensor hidden menu

#ifdef FEATURE_LGE_HOLE_SENSOR_HIDDEN_MENU		//20101221 seven.kim@lge.com for hole sensor hidden menu
static atomic_t sensing_hall;

static ssize_t p970_hall_sensing_show(struct device *dev, struct device_attribute *attr, char *buf)
{	
	sprintf(buf, "%d\n", atomic_read(&sensing_hall));	
	return (ssize_t)(strlen(buf) + 1);
}

static ssize_t p970_hall_sensing_store(struct device *dev, struct device_attribute *attr, char *buf, size_t count)
{		

}

static DEVICE_ATTR(sensing, 0664, p970_hall_sensing_show, p970_hall_sensing_store);
#endif /*FEATURE_LGE_HOLE_SENSOR_HIDDEN_MENU*/


static ssize_t print_name(struct switch_dev *sdev, char *buf)
{
	switch (switch_get_state(sdev)) {
	case NO_DOCK:
		return sprintf(buf, "None\n");
	case DESK_DOCK:
		return sprintf(buf, "DESK\n");
	case CAR_DOCK:
		return sprintf(buf, "CAR\n");
	}

	return -EINVAL;
}
static int bu52014hfv_update(struct bu52014hfv_info *info, int gpio, int dock)
{
	int state = !gpio_get_value(gpio);

	if (state)
	{
		#ifdef FEATURE_LGE_HOLE_SENSOR_HIDDEN_MENU		//20101221 seven.kim@lge.com for hole sensor hidden menu
		atomic_set( &sensing_hall, 1 );
		#endif /*FEATURE_LGE_HOLE_SENSOR_HIDDEN_MENU*/
		//seshu start
		//switch_set_state(&info->sdev, dock);
		switch_set_state(&info->sdev, CAR_DOCK); // 20110327 kenneth.kang@lge.com for prevent hall ic work as Desk Cradle 
		//seshu end
	}
	else
	{
		#ifdef FEATURE_LGE_HOLE_SENSOR_HIDDEN_MENU		//20101221 seven.kim@lge.com for hole sensor hidden menu
		atomic_set( &sensing_hall, 0 );
		#endif /*FEATURE_LGE_HOLE_SENSOR_HIDDEN_MENU*/
		
		switch_set_state(&info->sdev, NO_DOCK);
	}

	return state;
}
#ifdef LG_MUIC_DESK_CRADLE
/* LGE_CHANGE_START 2011-03-14 kenneth.kang@lge.com Cradle detected by muic */
int bu52014hfv_updated_by_muic(int dock)
{
	switch_set_state(&desk_cradle_info->sdev, dock);
	printk("bu52014hfv_updated_by_muic() : %d\n", dock);
}
EXPORT_SYMBOL(bu52014hfv_updated_by_muic);
/* LGE_CHANGE_END 2011-03-14 kenneth.kang@lge.com Cradle detected by muic */
#endif

void bu52014hfv_irq_north_work_func(struct work_struct *work)
{
	struct bu52014hfv_info *info = container_of(work,
						    struct bu52014hfv_info,
						    irq_north_work);
	bu52014hfv_update(info, info->gpio_north, info->north_value);
	enable_irq(info->irq_north);
}

void bu52014hfv_irq_south_work_func(struct work_struct *work)
{
	struct bu52014hfv_info *info = container_of(work,
						    struct bu52014hfv_info,
						    irq_south_work);
	bu52014hfv_update(info, info->gpio_south, info->south_value);
	enable_irq(info->irq_south);
}

static irqreturn_t bu52014hfv_isr(int irq, void *dev)
{
	struct bu52014hfv_info *info = dev;

	disable_irq_nosync(irq);

	if (irq == info->irq_north)
		queue_work(info->work_queue, &info->irq_north_work);
	else if (irq == info->irq_south)
		queue_work(info->work_queue, &info->irq_south_work);

	return IRQ_HANDLED;
}

static int __devinit bu52014hfv_probe(struct platform_device *pdev)
{
	struct bu52014hfv_platform_data *pdata = pdev->dev.platform_data;
	struct bu52014hfv_info *info;
	int ret = -1;

	info = kzalloc(sizeof(struct bu52014hfv_info), GFP_KERNEL);
	if (!info) {
		ret = -ENOMEM;
		pr_err("%s: could not allocate space for module data: %d\n",
		       __func__, ret);
		goto error_kmalloc_failed;
	}
#ifdef LG_MUIC_DESK_CRADLE	
/* LGE_CHANGE_START 2011-03-14 kenneth.kang@lge.com Cradle detected by muic */
	desk_cradle_info = info;
/* LGE_CHANGE_END 2011-03-14 kenneth.kang@lge.com Cradle detected by muic */
#endif
	/* Initialize hall effect driver data */
	info->gpio_north = pdata->docked_north_gpio;
	info->gpio_south = pdata->docked_south_gpio;

	info->irq_north = gpio_to_irq(pdata->docked_north_gpio);
	info->irq_south = gpio_to_irq(pdata->docked_south_gpio);

	if (pdata->north_is_desk) {
		//seshu
		//info->north_value = DESK_DOCK;
		info->north_value = CAR_DOCK; // N or S act on CAR DOCK
		//seshu end
		info->south_value = CAR_DOCK;
	} else {
		info->north_value = CAR_DOCK;
		//seshu start
		info->south_value = CAR_DOCK; // N or S act on CAR DOCK
		//info->south_value = DESK_DOCK;
		//seshu end
	}

	info->work_queue = create_singlethread_workqueue("bu52014hfv_wq");
	if (!info->work_queue) {
		ret = -ENOMEM;
		pr_err("%s: cannot create work queue: %d\n", __func__, ret);
		goto error_create_wq_failed;
	}
	INIT_WORK(&info->irq_north_work, bu52014hfv_irq_north_work_func);
	INIT_WORK(&info->irq_south_work, bu52014hfv_irq_south_work_func);

	ret = request_irq(info->irq_north, bu52014hfv_isr,
			  IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING,
			  BU52014HFV_MODULE_NAME, info);

	if (ret) {
		pr_err("%s: north request irq failed: %d\n", __func__, ret);
		goto error_request_irq_north_failed;
	}

	ret = request_irq(info->irq_south, bu52014hfv_isr,
			  IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING,
			  BU52014HFV_MODULE_NAME, info);
	if (ret) {
		pr_err("%s: south request irq failed: %d\n", __func__, ret);
		goto error_request_irq_south_failed;
	}

	enable_irq_wake(info->irq_north);
	enable_irq_wake(info->irq_south);

	info->sdev.name = "dock";
	info->sdev.print_name = print_name;
	ret = switch_dev_register(&info->sdev);
	if (ret) {
		pr_err("%s: error registering switch device %d\n",
			__func__, ret);
		goto error_switch_device_failed;
	}
	platform_set_drvdata(pdev, info);

	ret = bu52014hfv_update(info, info->gpio_south, info->south_value);
	if (!ret)
		bu52014hfv_update(info, info->gpio_north, info->north_value);

#ifdef FEATURE_LGE_HOLE_SENSOR_HIDDEN_MENU		//20101221 seven.kim@lge.com for hole sensor hidden menu
	ret = device_create_file(&pdev->dev, &dev_attr_sensing);	
	if (ret) 
	{		
		pr_err("tegra_touch_probe: grip suppression device_create_file failed\n");		
		goto error_devfs_failed;	
	}
#endif /* FEATURE_LGE_HOLE_SENSOR_HIDDEN_MENU */

	return 0;

#ifdef FEATURE_LGE_HOLE_SENSOR_HIDDEN_MENU		//20101221 seven.kim@lge.com for hole sensor hidden menu
error_devfs_failed:
	device_remove_file(&pdev->dev, &dev_attr_sensing);	
#endif /*FEATURE_LGE_HOLE_SENSOR_HIDDEN_MENU*/
error_switch_device_failed:
	free_irq(info->irq_south, info);
error_request_irq_south_failed:
	free_irq(info->irq_north, info);
error_request_irq_north_failed:
	destroy_workqueue(info->work_queue);
error_create_wq_failed:
	kfree(info);
error_kmalloc_failed:
	return ret;
}

static int __devexit bu52014hfv_remove(struct platform_device *pdev)
{
	struct bu52014hfv_info *info = platform_get_drvdata(pdev);

#ifdef FEATURE_LGE_HOLE_SENSOR_HIDDEN_MENU		//20101221 seven.kim@lge.com for hole sensor hidden menu
	device_remove_file(&pdev->dev, &dev_attr_sensing);
#endif /*FEATURE_LGE_HOLE_SENSOR_HIDDEN_MENU*/

	disable_irq_wake(info->irq_north);
	disable_irq_wake(info->irq_south);

	free_irq(info->irq_north, 0);
	free_irq(info->irq_south, 0);

	gpio_free(info->gpio_north);
	gpio_free(info->gpio_south);

	destroy_workqueue(info->work_queue);

	switch_dev_unregister(&info->sdev);

	kfree(info);
	return 0;
}

static struct platform_driver bu52014hfv_driver = {
	.probe = bu52014hfv_probe,
	.remove = __devexit_p(bu52014hfv_remove),
	.driver = {
		   .name = BU52014HFV_MODULE_NAME,
		   .owner = THIS_MODULE,
		   },
};

static int __init bu52014hfv_os_init(void)
{
	return platform_driver_register(&bu52014hfv_driver);
}

static void __exit bu52014hfv_os_exit(void)
{
	platform_driver_unregister(&bu52014hfv_driver);
}

module_init(bu52014hfv_os_init);
module_exit(bu52014hfv_os_exit);

MODULE_DESCRIPTION("Rohm BU52014HFV Hall Effect Driver");
MODULE_AUTHOR("Motorola");
MODULE_LICENSE("GPL");
