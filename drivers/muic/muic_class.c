/*
 *  drivers/muic/muic_class.c
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
#include <linux/module.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/fb.h>
#include <linux/notifier.h>
#include <linux/err.h>
#include <linux/delay.h>		/* usleep() */
#include <linux/i2c.h>
#include <linux/muic/muic.h>
#include <linux/charger_rt9524.h>

static struct muic_device *_mdev;

struct class *muic_class;
static atomic_t device_count;

static char *state[MUIC_MODE_NO] = {
	[MUIC_NONE] = "NONE",
	[MUIC_NA_TA] = "NA_TA",
	[MUIC_LG_TA] = "LG_TA",
	[MUIC_AP_UART] = "AP_UART",
	[MUIC_CP_UART] = "CP_UART",
	[MUIC_AP_USB] = "AP_USB",
	[MUIC_CP_USB] = "CP_USB",
	[MUIC_MHL] = "MHL",
};

static ssize_t state_show(struct device *dev, struct device_attribute *attr,
		char *buf)
{
	struct muic_device *mdev = (struct muic_device *)dev_get_drvdata(dev);
	char *str;

	if(state[mdev->mode])
		str = state[mdev->mode];
	else
		str = "no registered state";

	return sprintf(buf, "%s\n", str);
}

static ssize_t state_store(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t count)
{
	int i	= MUIC_MODE_NO;
	int set_mode = -1;
	char *p;
	int len;
	int error = -EINVAL;

	p = memchr(buf, '\n', count);
	len = p ? p - buf : count;

	dev_info(dev, "muic %s: %s, count:%d, sizeof:%d, len:%d\n", 
		__func__, buf, count, sizeof(buf), len);

	if(len > 7){ // max value is 7
		printk(KERN_WARNING "%s, wrong command\n", __func__);
		return -EINVAL;		
	}		

	do{ 
			i--;
#if 0			
			dev_info(dev,"muic %s, buf: %s, state[%d]: %s, size: %d, \n", 
				__func__, buf, i, state[i], sizeof(state[i]) );
#endif			
			if(NULL != state[i]){
				if(!strncmp(buf, state[i], len)){ //  min(len, sizeof(state[i])))){
					dev_info(dev, "%s, store muic mode:%s\n", __func__,  state[i]);
					set_mode = i;
				}
			}		
	} while((set_mode<0) && (i>0));

	if(set_mode<0){
		printk(KERN_WARNING "%s, wrong command\n", __func__);
		return -EINVAL;
	}
	
	muic_set_mode(set_mode);
	dev_info(dev, "%s, end\n", __func__);
	
	return count;
}

static ssize_t name_show(struct device *dev, struct device_attribute *attr,
		char *buf)
{
	struct muic_device *mdev = (struct muic_device *)
		dev_get_drvdata(dev);

	return sprintf(buf, "%s\n", mdev->name);
}

static DEVICE_ATTR(state, S_IRUGO | S_IWUSR, state_show, state_store);
static DEVICE_ATTR(name, S_IRUGO | S_IWUSR, name_show, NULL);

static ssize_t int_state_show(struct device *dev, struct device_attribute *attr,
		char *buf)
{
	struct muic_device *mdev = (struct muic_device *)dev_get_drvdata(dev);
	char *str;

#if 0
	if(state[mdev->mode])
		str = state[mdev->mode];
	else
		str = "no registered state";
#endif

	if(!(mdev->read_int_state))
		return -EPERM; 

	return mdev->read_int_state(mdev, buf); // sprintf(buf, "0x11");
}

static DEVICE_ATTR(int_state, S_IRUGO | S_IWUSR, int_state_show, NULL);


static BLOCKING_NOTIFIER_HEAD(muic_notifier_list);
 
/**
 *  muic_register_client - register a client notifier
 *  @mcdev: muic client device which has a noti block for callback on events
 */
int muic_register_client(struct muic_client_device *mcdev)
{
	return blocking_notifier_chain_register(&muic_notifier_list, &mcdev->muic_notif);
}
EXPORT_SYMBOL(muic_register_client);
 
/**
 *  muic_unregister_client - unregister a client notifier
 *  @nb: notifier block to callback on events
 */
int muic_unregister_client(struct notifier_block *nb)
{
	return blocking_notifier_chain_unregister(&muic_notifier_list, nb);
}
EXPORT_SYMBOL_GPL(muic_unregister_client);
 
/**
 * muic_notifier_call_chain - notify clients of muic_events
 *
 */
int muic_notifier_call_chain(unsigned long val, void *v) 
{
	return blocking_notifier_call_chain(&muic_notifier_list, val, v);
}

EXPORT_SYMBOL(muic_notifier_call_chain);

static int muic_send_event(unsigned long event)
{
	return muic_notifier_call_chain(event, _mdev);
}

/*
 * Function: Read the MUIC register whose internal address is addr
 * 			and save the u8 content into value.
 * Return value: Negative errno upon failure, 0 upon success.
 */
s32 muic_i2c_read_byte(struct i2c_client *client, u8 addr, u8 *value)
{
	*value = i2c_smbus_read_byte_data(client, (u8)addr);
	if (*value < 0) {
		printk(KERN_INFO "[MUIC] muic_i2c_read_byte failed.\n");
		return *value;
	} else {
		return 0;
	}
}
EXPORT_SYMBOL(muic_i2c_read_byte);

/*
 * Function: Write u8 value to the MUIC regi ster whose internal address is addr.
 * Return value: Negative errno upon failure, 0 upon success.
 */
s32 muic_i2c_write_byte(struct i2c_client *client, u8 addr, u8 value)
{
	s32 ret;
	ret = i2c_smbus_write_byte_data(client, (u8)addr, (u8)value);
	if (ret < 0) 
		printk(KERN_INFO "[MUIC] muic_i2c_write_byte failed.\n");
	return ret;
}
EXPORT_SYMBOL(muic_i2c_write_byte);

// [jongho3.lee@lge.com] get muic mode for charger
TYPE_MUIC_MODE muic_get_mode(void)
{
	if(_mdev == NULL) {
		printk(KERN_ERR "muic device not installed!\n");
		return MUIC_UNINITED;
	}

	return _mdev->mode;
}
EXPORT_SYMBOL(muic_get_mode);

int muic_set_mode(TYPE_MUIC_MODE mode)
{
	if(_mdev == NULL) {
		printk(KERN_ERR "muic device not installed!\n");
		return -ENODEV;
	}

	printk(KERN_INFO "muic: %s, mode:%s\n",
		__func__, state[mode]);
		
	_mdev->mode = mode;
	muic_send_event(mode);
	
	return 0;
}
EXPORT_SYMBOL(muic_set_mode);
#if 0
//LGE_Changes_S chulhwhee.shim@lge.com, 2010.12.7  FOTA update 
int fota_ebl_download(void)
{
   return 0;
}
//LGE_Changes_E chulhwhee.shim@lge.com, 2010.12.7  FOTA update
#endif
static int create_muic_class(void)
{
	if (!muic_class) {
		muic_class = class_create(THIS_MODULE, "muic");
		if (IS_ERR(muic_class))
			return PTR_ERR(muic_class);
		atomic_set(&device_count, 0);
	}

	return 0;
}


int muic_device_register(struct muic_device *mdev, struct muic_ops *ops)
{
	int ret;

	if (!mdev)
		return -EINVAL;

	if (_mdev != NULL) {
		printk(KERN_ERR "muic device is alread installed!\n");
		return -EPERM;
	}

	if (!muic_class) {
		ret = create_muic_class();
		if (ret < 0)
			return ret;
	}

	mdev->index = atomic_inc_return(&device_count);
	mdev->dev = device_create(muic_class, NULL,
		MKDEV(0, mdev->index), mdev, mdev->name);

	if (IS_ERR(mdev->dev))
		return PTR_ERR(mdev->dev);

	ret = device_create_file(mdev->dev, &dev_attr_state);
	if (ret < 0)
		goto err_create_file_1;

	ret = device_create_file(mdev->dev, &dev_attr_name);
	if (ret < 0)
		goto err_create_file_2;

	ret = device_create_file(mdev->dev, &dev_attr_int_state);
	if (ret < 0)
		goto err_create_file_2;


	mdev->mode = MUIC_UNKNOWN;
	_mdev = mdev;
	
	return 0;

err_create_file_2:
	device_remove_file(mdev->dev, &dev_attr_state);
err_create_file_1:
	device_destroy(muic_class, MKDEV(0, mdev->index));
	printk(KERN_ERR "muic: Failed to register driver %s\n", mdev->name);

	return ret;
}
EXPORT_SYMBOL_GPL(muic_device_register);

void muic_device_unregister(struct muic_device *mdev)
{
	device_remove_file(mdev->dev, &dev_attr_name);
	device_remove_file(mdev->dev, &dev_attr_state);
	device_destroy(muic_class, MKDEV(0, mdev->index));
	dev_set_drvdata(mdev->dev, NULL);
	_mdev = NULL;
}
EXPORT_SYMBOL_GPL(muic_device_unregister);

static void __exit muic_class_exit(void)
{
	class_destroy(muic_class);
}

struct class *muic_class;

static int __init muic_class_init(void)
{
	muic_class = class_create(THIS_MODULE, "muic");
	if (IS_ERR(muic_class)) {
		printk(KERN_WARNING "Unable to create muic class; errno = %ld\n",
				PTR_ERR(muic_class));
		return PTR_ERR(muic_class);
	}

	//muic_class->dev_attrs = muic_device_attributes;
	//muic_class->suspend = muic_suspend;
	//muic_class->resume = muic_resume;
	return 0;
}

subsys_initcall(muic_class_init);
module_exit(muic_class_exit);

MODULE_AUTHOR("Seungho Park <seungho1.park@lge.com>");
MODULE_DESCRIPTION("MUIC class driver");
MODULE_LICENSE("GPL v2");
