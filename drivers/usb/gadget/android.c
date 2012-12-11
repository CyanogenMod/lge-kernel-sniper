/*
 * Gadget Driver for Android
 *
 * Copyright (C) 2008 Google, Inc.
 * Author: Mike Lockwood <lockwood@android.com>
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

//#define DEBUG 
//#define VERBOSE_DEBUG

#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>

#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/utsname.h>
#include <linux/platform_device.h>

#include <linux/usb/ch9.h>
#include <linux/usb/composite.h>
#include <linux/usb/gadget.h>

#include "gadget_chips.h"

/*
 * Kbuild is not very cooperative with respect to linking separately
 * compiled library objects into one module.  So for now we won't use
 * separate compilation ... ensuring init/exit sections work to shrink
 * the runtime footprint, and giving us at least some parts of what
 * a "gcc --combine ... part1.c part2.c part3.c ... " build would.
 */
#include "usbstring.c"
#include "config.c"
#include "epautoconf.c"
#include "composite.c"

/* LGE_SJIT_S 10/21/2011 [mohamed.khadri@lge.com] 
            LG Gadget driver - nmea and diag funcation extensions  */
#if defined(CONFIG_LGE_ANDROID_USB)
#include "f_lgserial.c"
#endif
/* LGE_SJIT_E 10/21/2011 [mohamed.khadri@lge.com]
            LG Gadget driver - nmea and diag funcation extensions  */

#include "f_mass_storage.c"
#include "u_serial.c"
#include "f_acm.c"
#include "f_adb.c"
#include "f_mtp.c"
#include "f_accessory.c"

#if defined(CONFIG_LGE_ANDROID_USB_DONOTUSE)
#include "f_ecm.c"
#else
#define USB_ETH_RNDIS y
#include "f_rndis.c"
#include "rndis.c"
#endif 

#include "u_ether.c"


MODULE_AUTHOR("Mike Lockwood");
MODULE_DESCRIPTION("Android Composite USB Driver");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0");

static const char longname[] = "Gadget Android";

/* LGE_SJIT_S 10/21/2011 [mohamed.khadri@lge.com]
            LG Gadget driver - nmea and diag funcation extensions  */
#if defined(CONFIG_LGE_ANDROID_USB)

/* LG vendor and product IDs, can overridden by userspace */
#define VENDOR_ID               0x1004 
#if defined(CONFIG_LGE_ANDROID_USB_PID)
#define PRODUCT_ID              CONFIG_LGE_ANDROID_USB_PID
#else
#define PRODUCT_ID              0x61D7 
#endif /* defined(CONFIG_LGE_ANDROID_USB_PID) */

#define DIAG_ENABLED            0x01
#define ACM_ENABLED             0x02
#define NMEA_ENABLED            0x04

#else
/* Default vendor and product IDs, overridden by userspace */
#define VENDOR_ID               0x18D1
#define PRODUCT_ID              0x0D02
#endif /* defined(CONFIG_LGE_ANDROID_USB) */
/* LGE_SJIT_E 10/21/2011 [mohamed.khadri@lge.com]
            LG Gadget driver - nmea and diag funcation extensions  */

struct android_usb_function {
	char *name;
	void *config;

	struct device *dev;
	char *dev_name;
	struct device_attribute **attributes;

	/* for android_dev.enabled_functions */
	struct list_head enabled_list;

	/* Optional: initialization during gadget bind */
	int (*init)(struct android_usb_function *, struct usb_composite_dev *);
	/* Optional: cleanup during gadget unbind */
	void (*cleanup)(struct android_usb_function *);
	/* Optional: called when the function is added the list of
	 *		enabled functions */
	void (*enable)(struct android_usb_function *);
	/* Optional: called when it is removed */
	void (*disable)(struct android_usb_function *);

	int (*bind_config)(struct android_usb_function *, struct usb_configuration *);

	/* Optional: called when the configuration is removed */
	void (*unbind_config)(struct android_usb_function *, struct usb_configuration *);
	/* Optional: handle ctrl requests before the device is configured */
	int (*ctrlrequest)(struct android_usb_function *,
					struct usb_composite_dev *,
					const struct usb_ctrlrequest *);
};

struct android_dev {
	struct android_usb_function **functions;
	struct list_head enabled_functions;
	struct usb_composite_dev *cdev;
	struct device *dev;

	bool enabled;
	struct mutex mutex;
	bool connected;
	int disable_depth;
	bool disconnected;
	bool sw_connected;
	struct work_struct work;
/* LGE_SJIT_S 10/21/2011 [mohamed.khadri@lge.com]
            LG Gadget driver */
#if defined(CONFIG_LGE_ANDROID_USB)
        int serial_flags;
#endif
/* LGE_SJIT_E 10/21/2011 [mohamed.khadri@lge.com]
            LG Gadget driver */
};

static struct class *android_class;
static struct android_dev *_android_dev;
static int android_bind_config(struct usb_configuration *c);
static void android_unbind_config(struct usb_configuration *c);

/* string IDs are assigned dynamically */
#define STRING_MANUFACTURER_IDX		0
#define STRING_PRODUCT_IDX		1
#define STRING_SERIAL_IDX		2

static char manufacturer_string[256];
static char product_string[256];
static char serial_string[256];

/* String Table */
static struct usb_string strings_dev[] = {
	[STRING_MANUFACTURER_IDX].s = manufacturer_string,
	[STRING_PRODUCT_IDX].s = product_string,
	[STRING_SERIAL_IDX].s = serial_string,
	{  }			/* end of list */
};

static struct usb_gadget_strings stringtab_dev = {
	.language	= 0x0409,	/* en-us */
	.strings	= strings_dev,
};

static struct usb_gadget_strings *dev_strings[] = {
	&stringtab_dev,
	NULL,
};

static struct usb_device_descriptor device_desc = {
	.bLength              = sizeof(device_desc),
	.bDescriptorType      = USB_DT_DEVICE,
	.bcdUSB               = __constant_cpu_to_le16(0x0200),
/* LGE_CHANGE_S 6/20/2012 [edwardk.kim@lge.com] for USB Descriptor Check */	
	.bDeviceClass  	  = USB_CLASS_MISC,	
//	.bDeviceClass		  = USB_CLASS_PER_INTERFACE,
	.bDeviceSubClass	  = 0x02,
	.bDeviceProtocol	  = 0x01,	
/* LGE_CHANGE_E 6/20/2012 [edwardk.kim@lge.com] for USB Descriptor Check */		
	.idVendor             = __constant_cpu_to_le16(VENDOR_ID),
	.idProduct            = __constant_cpu_to_le16(PRODUCT_ID),
	.bcdDevice            = __constant_cpu_to_le16(0xffff),
	.bNumConfigurations   = 1,
};

static struct usb_configuration android_config_driver = {
/* LGE_SJIT_S 10/21/2011 [mohamed.khadri@lge.com]
            LG Gadget driver */
#if !defined(CONFIG_LGE_ANDROID_USB)
	.label		= "android",
#else
	.label          = "LG-android",
#endif
/* LGE_SJIT_E 10/21/2011 [mohamed.khadri@lge.com]
            LG Gadget driver */
	.unbind		= android_unbind_config,
	.bConfigurationValue = 1,

//!![S] 2011-05-06 by pilsu.kim@leg.com : change to bmAttributes for USB-IF Test
//++            If you want to pass USB-IF test, you have to change like below things
//++            bmAttributes    = USB_CONFIG__ATT_ONE | USB_CONFIG_ATT_WAKEUP
//++             ## Do Not use USB_CONFIG_ATT_SELFPOWER ##
#if defined(CONFIG_LGE_ANDROID_USB)
        .bmAttributes   = USB_CONFIG_ATT_ONE | USB_CONFIG_ATT_WAKEUP,
#else
        .bmAttributes   = USB_CONFIG_ATT_ONE | USB_CONFIG_ATT_SELFPOWER,
#endif
//!![E] 2011-05-06 by pilsu.kim@lge.com :
	.bMaxPower	= 0xFA, /* 500ma */
};

static void android_work(struct work_struct *data)
{
	struct android_dev *dev = container_of(data, struct android_dev, work);
	struct usb_composite_dev *cdev = dev->cdev;
	char *disconnected[2] = { "USB_STATE=DISCONNECTED", NULL };
	char *connected[2]    = { "USB_STATE=CONNECTED", NULL };
	char *configured[2]   = { "USB_STATE=CONFIGURED", NULL };
	char **uevent_envp = NULL;
	unsigned long flags;

	spin_lock_irqsave(&cdev->lock, flags);
        if (cdev->config)
		uevent_envp = configured;
	else if (dev->connected != dev->sw_connected)
		uevent_envp = dev->connected ? connected : disconnected;
/* LGE_CHANGE_S 6/14/2012 [edwardk.kim@lge.com] */	
	else if(dev->disconnected)
	{	
		uevent_envp = disconnected;
		dev->disconnected = 0;
	}
/* LGE_CHANGE_E 6/14/2012 [edwardk.kim@lge.com] */
	dev->sw_connected = dev->connected;
	spin_unlock_irqrestore(&cdev->lock, flags);

	if (uevent_envp) {
		if (uevent_envp == configured)
			msleep(50);
		kobject_uevent_env(&dev->dev->kobj, KOBJ_CHANGE, uevent_envp);
		pr_info("%s: sent uevent %s\n", __func__, uevent_envp[0]);
	} else {
		pr_info("%s: did not send uevent (%d %d %p)\n", __func__,
			 dev->connected, dev->sw_connected, cdev->config);
	}
}

static void android_enable(struct android_dev *dev)
{
	struct usb_composite_dev *cdev = dev->cdev;

	if (WARN_ON(!dev->disable_depth))
		return;

	if (--dev->disable_depth == 0) {
		usb_add_config(cdev, &android_config_driver,
					android_bind_config);
		usb_gadget_connect(cdev->gadget);
	}
}

static void android_disable(struct android_dev *dev)
{
	struct usb_composite_dev *cdev = dev->cdev;

	if (dev->disable_depth++ == 0) {
		usb_gadget_disconnect(cdev->gadget);
		/* Cancel pending control requests */
		usb_ep_dequeue(cdev->gadget->ep0, cdev->req);
		usb_remove_config(cdev, &android_config_driver);
	}
}


/* LGE_SJIT_S 10/21/2011 [mohamed.khadri@lge.com]
            LG Gadget driver */
#if defined(CONFIG_LGE_ANDROID_USB)
/* Two Serial ports for NMEA and DIAG */
#define MAX_LG_SERIAL_INSTANCES 2
/* Restrict Max acm instances to 1 for consistent LG serial port behavior */
#define MAX_ACM_INSTANCES 1 
#else
#define MAX_ACM_INSTANCES 4
#endif 
/* LGE_SJIT_E 10/21/2011 [mohamed.khadri@lge.com]
            LG Gadget driver */
/*-------------------------------------------------------------------------*/
/* Supported functions initialization */

struct adb_data {
	bool opened;
	bool enabled;
};

	static int
adb_function_init(struct android_usb_function *f,
		struct usb_composite_dev *cdev)
{
	f->config = kzalloc(sizeof(struct adb_data), GFP_KERNEL);
	if (!f->config)
		return -ENOMEM;

	return adb_setup();
}

static void adb_function_cleanup(struct android_usb_function *f)
{
	adb_cleanup();
	kfree(f->config);
}

static int adb_function_bind_config(struct android_usb_function *f, struct usb_configuration *c)
{
	return adb_bind_config(c);
}

static void adb_android_function_enable(struct android_usb_function *f)
{
	struct android_dev *dev = _android_dev;
	struct adb_data *data = f->config;

	data->enabled = true;

	/* Disable the gadget until adbd is ready */
	if (!data->opened)
		android_disable(dev);
}

static void adb_android_function_disable(struct android_usb_function *f)
{
	struct android_dev *dev = _android_dev;
	struct adb_data *data = f->config;

	data->enabled = false;

	/* Balance the disable that was called in closed_callback */
	if (!data->opened)
		android_enable(dev);
}

static struct android_usb_function adb_function = {
	.name		= "adb",
	.enable         = adb_android_function_enable,
	.disable        = adb_android_function_disable,
	.init		= adb_function_init,
	.cleanup	= adb_function_cleanup,
	.bind_config	= adb_function_bind_config,
};

static void adb_ready_callback(void)
{
	struct android_dev *dev = _android_dev;
	struct adb_data *data = adb_function.config;

	mutex_lock(&dev->mutex);

	data->opened = true;

	if (data->enabled)
		android_enable(dev);

	mutex_unlock(&dev->mutex);
}

static void adb_closed_callback(void)
{
	struct android_dev *dev = _android_dev;
	struct adb_data *data = adb_function.config;

	mutex_lock(&dev->mutex);

	data->opened = false;

	if (data->enabled)
		android_disable(dev);

	mutex_unlock(&dev->mutex);
}

struct acm_function_config {
	int instances;
};

static int acm_function_init(struct android_usb_function *f, struct usb_composite_dev *cdev)
{
/* LGE_SJIT_S 10/21/2011 [mohamed.khadri@lge.com]
            LG Gadget driver - nmea and diag funcation extensions  */
#if defined(CONFIG_LGE_ANDROID_USB)
        int setupReturn = 0;
        struct android_dev *dev = _android_dev;
#endif
/* LGE_SJIT_E 10/21/2011 [mohamed.khadri@lge.com]
            LG Gadget driver - nmea and diag funcation extensions  */
	f->config = kzalloc(sizeof(struct acm_function_config), GFP_KERNEL);

	if (!f->config)
		return -ENOMEM;
/* LGE_SJIT_S 10/21/2011 [mohamed.khadri@lge.com]
            LG Gadget driver - nmea and diag funcation extensions  */
#if defined(CONFIG_LGE_ANDROID_USB)
        if(!dev->serial_flags)
            setupReturn = gserial_setup(cdev->gadget, MAX_ACM_INSTANCES+MAX_LG_SERIAL_INSTANCES);

        if(0==setupReturn) 
            dev->serial_flags |= ACM_ENABLED;

        /* default ACM instances to 1 for LG configuration*/
        ((struct acm_function_config *)f->config)->instances = MAX_ACM_INSTANCES;

        return setupReturn;
#else
        return gserial_setup(cdev->gadget, MAX_ACM_INSTANCES);
#endif
/* LGE_SJIT_E 10/21/2011 [mohamed.khadri@lge.com]
            LG Gadget driver - nmea and diag funcation extensions  */
}

static void acm_function_cleanup(struct android_usb_function *f)
{
/* LGE_SJIT_S 10/21/2011 [mohamed.khadri@lge.com]
            LG Gadget driver - nmea and diag funcation extensions  */
#if defined(CONFIG_LGE_ANDROID_USB)
        struct android_dev *dev = _android_dev;
        dev->serial_flags &= ~ACM_ENABLED;
        if(!dev->serial_flags) 
#endif
       	    gserial_cleanup();
/* LGE_SJIT_E 10/21/2011 [mohamed.khadri@lge.com]
            LG Gadget driver - nmea and diag funcation extensions  */
	kfree(f->config);
	f->config = NULL;
}

static int acm_function_bind_config(struct android_usb_function *f, struct usb_configuration *c)
{
	int i;
	int ret = 0;
	struct acm_function_config *config = f->config;

	for (i = 0; i < config->instances; i++) {
		ret = acm_bind_config(c, i);
		if (ret) {
			pr_err("Could not bind acm%u config\n", i);
			break;
		}
	}

	return ret;
}

static ssize_t acm_instances_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct android_usb_function *f = dev_get_drvdata(dev);
	struct acm_function_config *config = f->config;
	return sprintf(buf, "%d\n", config->instances);
}

static ssize_t acm_instances_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct android_usb_function *f = dev_get_drvdata(dev);
	struct acm_function_config *config = f->config;
	int value;

	sscanf(buf, "%d", &value);
	if (value > MAX_ACM_INSTANCES)
		value = MAX_ACM_INSTANCES;
	config->instances = value;
	return size;
}
/* LGE_SJIT_S 10/21/2011 [mohamed.khadri@lge.com]
            LG Gadget driver - Make read only to limit ACM instances  */
#if !defined(CONFIG_LGE_ANDROID_USB)
static DEVICE_ATTR(instances, S_IRUGO | S_IWUSR, acm_instances_show, acm_instances_store);
#else
static DEVICE_ATTR(instances, S_IRUGO , acm_instances_show, acm_instances_store);
#endif
/* LGE_SJIT_E 10/21/2011 [mohamed.khadri@lge.com]
            LG Gadget driver - Make read only to limit ACM instances  */
static struct device_attribute *acm_function_attributes[] = { &dev_attr_instances, NULL };

static struct android_usb_function acm_function = {
	.name		= "acm",
	.init		= acm_function_init,
	.cleanup	= acm_function_cleanup,
	.bind_config	= acm_function_bind_config,
	.attributes	= acm_function_attributes,
};
/* LGE_SJIT_S 10/21/2011 [mohamed.khadri@lge.com]
            LG Gadget driver - nmea and diag funcation extensions  */
#if defined(CONFIG_LGE_ANDROID_USB)
static int lgserial_function_init(struct android_usb_function *f, struct usb_composite_dev *cdev)
{
    int ret = 0;
    struct android_dev *dev = _android_dev;

    if(!dev->serial_flags)
        ret = gserial_setup(cdev->gadget, MAX_ACM_INSTANCES+MAX_LG_SERIAL_INSTANCES);

    if(0==ret) 
       dev->serial_flags |= strcmp(f->name,"nmea")==0?NMEA_ENABLED:DIAG_ENABLED;

    return ret;

}
int lgserial_function_bind_config(struct android_usb_function *f,struct usb_configuration *c)
{
        return lgserial_bind_config(c, strcmp(f->name,"nmea")==0?MAX_ACM_INSTANCES+1:MAX_ACM_INSTANCES,f->name);
}

static void lgserial_function_cleanup(struct android_usb_function *f)
{
        struct android_dev *dev = _android_dev;

        dev->serial_flags &= strcmp(f->name,"nmea")==0?~NMEA_ENABLED:~DIAG_ENABLED;
        if(!dev->serial_flags)
            gserial_cleanup();
}

static struct android_usb_function nmea_function = {
        .name = "nmea",
        .init = lgserial_function_init,
        .cleanup = lgserial_function_cleanup,
        .bind_config = lgserial_function_bind_config,
};

static struct android_usb_function diag_function = {
        .name = "gser",
        .init = lgserial_function_init,
        .cleanup = lgserial_function_cleanup,
        .bind_config = lgserial_function_bind_config,
};

#endif
/* LGE_SJIT_E 10/21/2011 [mohamed.khadri@lge.com]
            LG Gadget driver - nmea and diag funcation extensions  */
static int mtp_function_init(struct android_usb_function *f, struct usb_composite_dev *cdev)
{
	return mtp_setup();
}

static void mtp_function_cleanup(struct android_usb_function *f)
{
	mtp_cleanup();
}

static int mtp_function_bind_config(struct android_usb_function *f, struct usb_configuration *c)
{
	return mtp_bind_config(c, false);
}

static int ptp_function_init(struct android_usb_function *f, struct usb_composite_dev *cdev)
{
	/* nothing to do - initialization is handled by mtp_function_init */
	return 0;
}

static void ptp_function_cleanup(struct android_usb_function *f)
{
	/* nothing to do - cleanup is handled by mtp_function_cleanup */
}

static int ptp_function_bind_config(struct android_usb_function *f, struct usb_configuration *c)
{
	return mtp_bind_config(c, true);
}

static int mtp_function_ctrlrequest(struct android_usb_function *f,
						struct usb_composite_dev *cdev,
						const struct usb_ctrlrequest *c)
{
	return mtp_ctrlrequest(cdev, c);
}

static struct android_usb_function mtp_function = {
	.name		= "mtp",
	.init		= mtp_function_init,
	.cleanup	= mtp_function_cleanup,
	.bind_config	= mtp_function_bind_config,
	.ctrlrequest	= mtp_function_ctrlrequest,
};

/* PTP function is same as MTP with slightly different interface descriptor */
static struct android_usb_function ptp_function = {
	.name		= "ptp",
	.init		= ptp_function_init,
	.cleanup	= ptp_function_cleanup,
	.bind_config	= ptp_function_bind_config,
};

#if !defined(CONFIG_LGE_ANDROID_USB_DONOTUSE)
struct rndis_function_config {
	u8      ethaddr[ETH_ALEN];
	u32     vendorID;
	char	manufacturer[256];
	bool	wceis;
};

static int rndis_function_init(struct android_usb_function *f, struct usb_composite_dev *cdev)
{
	f->config = kzalloc(sizeof(struct rndis_function_config), GFP_KERNEL);
	if (!f->config)
		return -ENOMEM;
	return 0;
}

static void rndis_function_cleanup(struct android_usb_function *f)
{
	kfree(f->config);
	f->config = NULL;
}

static int rndis_function_bind_config(struct android_usb_function *f,
					struct usb_configuration *c)
{
	int ret;
	struct rndis_function_config *rndis = f->config;

	if (!rndis) {
		pr_err("%s: rndis_pdata\n", __func__);
		return -1;
	}

	pr_info("%s MAC: %02X:%02X:%02X:%02X:%02X:%02X\n", __func__,
		rndis->ethaddr[0], rndis->ethaddr[1], rndis->ethaddr[2],
		rndis->ethaddr[3], rndis->ethaddr[4], rndis->ethaddr[5]);

	ret = gether_setup_name(c->cdev->gadget, rndis->ethaddr, "rndis");
	if (ret) {
		pr_err("%s: gether_setup failed\n", __func__);
		return ret;
	}

	if (rndis->wceis) {
		/* "Wireless" RNDIS; auto-detected by Windows */
		rndis_iad_descriptor.bFunctionClass =
						USB_CLASS_WIRELESS_CONTROLLER;
		rndis_iad_descriptor.bFunctionSubClass = 0x01;
		rndis_iad_descriptor.bFunctionProtocol = 0x03;
		rndis_control_intf.bInterfaceClass =
						USB_CLASS_WIRELESS_CONTROLLER;
		rndis_control_intf.bInterfaceSubClass =	 0x01;
		rndis_control_intf.bInterfaceProtocol =	 0x03;
	}

	return rndis_bind_config(c, rndis->ethaddr, rndis->vendorID,
				    rndis->manufacturer);
}

static void rndis_function_unbind_config(struct android_usb_function *f,
						struct usb_configuration *c)
{
	gether_cleanup();
}

static ssize_t rndis_manufacturer_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct android_usb_function *f = dev_get_drvdata(dev);
	struct rndis_function_config *config = f->config;
	return sprintf(buf, "%s\n", config->manufacturer);
}

static ssize_t rndis_manufacturer_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct android_usb_function *f = dev_get_drvdata(dev);
	struct rndis_function_config *config = f->config;

	if (size >= sizeof(config->manufacturer))
		return -EINVAL;
	if (sscanf(buf, "%s", config->manufacturer) == 1)
		return size;
	return -1;
}

static DEVICE_ATTR(manufacturer, S_IRUGO | S_IWUSR, rndis_manufacturer_show,
						    rndis_manufacturer_store);

static ssize_t rndis_wceis_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct android_usb_function *f = dev_get_drvdata(dev);
	struct rndis_function_config *config = f->config;
	return sprintf(buf, "%d\n", config->wceis);
}

static ssize_t rndis_wceis_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct android_usb_function *f = dev_get_drvdata(dev);
	struct rndis_function_config *config = f->config;
	int value;

	if (sscanf(buf, "%d", &value) == 1) {
		config->wceis = value;
		return size;
	}
	return -EINVAL;
}

static DEVICE_ATTR(wceis, S_IRUGO | S_IWUSR, rndis_wceis_show,
					     rndis_wceis_store);

static ssize_t rndis_ethaddr_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct android_usb_function *f = dev_get_drvdata(dev);
	struct rndis_function_config *rndis = f->config;
	return sprintf(buf, "%02x:%02x:%02x:%02x:%02x:%02x\n",
		rndis->ethaddr[0], rndis->ethaddr[1], rndis->ethaddr[2],
		rndis->ethaddr[3], rndis->ethaddr[4], rndis->ethaddr[5]);
}

static ssize_t rndis_ethaddr_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct android_usb_function *f = dev_get_drvdata(dev);
	struct rndis_function_config *rndis = f->config;

	if (sscanf(buf, "%02x:%02x:%02x:%02x:%02x:%02x\n",
		    (int *)&rndis->ethaddr[0], (int *)&rndis->ethaddr[1],
		    (int *)&rndis->ethaddr[2], (int *)&rndis->ethaddr[3],
		    (int *)&rndis->ethaddr[4], (int *)&rndis->ethaddr[5]) == 6)
		return size;
	return -EINVAL;
}

static DEVICE_ATTR(ethaddr, S_IRUGO | S_IWUSR, rndis_ethaddr_show,
					       rndis_ethaddr_store);

static ssize_t rndis_vendorID_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct android_usb_function *f = dev_get_drvdata(dev);
	struct rndis_function_config *config = f->config;
	return sprintf(buf, "%04x\n", config->vendorID);
}

static ssize_t rndis_vendorID_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct android_usb_function *f = dev_get_drvdata(dev);
	struct rndis_function_config *config = f->config;
	int value;

	if (sscanf(buf, "%04x", &value) == 1) {
		config->vendorID = value;
		return size;
	}
	return -EINVAL;
}

static DEVICE_ATTR(vendorID, S_IRUGO | S_IWUSR, rndis_vendorID_show,
						rndis_vendorID_store);

static struct device_attribute *rndis_function_attributes[] = {
	&dev_attr_manufacturer,
	&dev_attr_wceis,
	&dev_attr_ethaddr,
	&dev_attr_vendorID,
	NULL
};

static struct android_usb_function rndis_function = {
	.name		= "rndis",
	.init		= rndis_function_init,
	.cleanup	= rndis_function_cleanup,
	.bind_config	= rndis_function_bind_config,
	.unbind_config	= rndis_function_unbind_config,
	.attributes	= rndis_function_attributes,
};
#endif

#if defined(CONFIG_LGE_ANDROID_USB_DONOTUSE)
// ecm function
struct ecm_function_config {
	u8      ethaddr[ETH_ALEN];
	u32     vendorID;
	char	manufacturer[256];
	bool	wceis;
};

static int ecm_function_init(struct android_usb_function *f, struct usb_composite_dev *cdev)
{
	f->config = kzalloc(sizeof(struct ecm_function_config), GFP_KERNEL);
	if (!f->config)
		return -ENOMEM;
	return 0;
}

static void ecm_function_cleanup(struct android_usb_function *f)
{
	gether_cleanup();
	kfree(f->config);
	f->config = NULL;
}

static int ecm_function_bind_config(struct android_usb_function *f,
					struct usb_configuration *c)
{
	int ret;
	struct ecm_function_config *ecm = f->config;

	if (!ecm) {
		pr_err("%s: ecm_pdata\n", __func__);
		return -1;
	}

	pr_info("%s MAC: %02X:%02X:%02X:%02X:%02X:%02X\n", __func__,
		ecm->ethaddr[0], ecm->ethaddr[1], ecm->ethaddr[2],
		ecm->ethaddr[3], ecm->ethaddr[4], ecm->ethaddr[5]);

	ret = gether_setup_name(c->cdev->gadget, ecm->ethaddr, "ecm");
	if (ret) {
		pr_err("%s: gether_setup failed\n", __func__);
		return ret;
	}
	
	return ecm_bind_config(c, ecm->ethaddr);
}

static void ecm_function_unbind_config(struct android_usb_function *f,
						struct usb_configuration *c)
{
	gether_cleanup();
}

static ssize_t ecm_manufacturer_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct android_usb_function *f = dev_get_drvdata(dev);
	struct ecm_function_config *config = f->config;
	return sprintf(buf, "%s\n", config->manufacturer);
}

static ssize_t ecm_manufacturer_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct android_usb_function *f = dev_get_drvdata(dev);
	struct ecm_function_config *config = f->config;

	if (size >= sizeof(config->manufacturer))
		return -EINVAL;
	if (sscanf(buf, "%s", config->manufacturer) == 1)
		return size;
	return -1;
}

static DEVICE_ATTR(manufacturer, S_IRUGO | S_IWUSR, ecm_manufacturer_show,
						    ecm_manufacturer_store);

static ssize_t ecm_ethaddr_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct android_usb_function *f = dev_get_drvdata(dev);
	struct ecm_function_config *ecm = f->config;
	return sprintf(buf, "%02x:%02x:%02x:%02x:%02x:%02x\n",
		ecm->ethaddr[0], ecm->ethaddr[1], ecm->ethaddr[2],
		ecm->ethaddr[3], ecm->ethaddr[4], ecm->ethaddr[5]);
}

static ssize_t ecm_ethaddr_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct android_usb_function *f = dev_get_drvdata(dev);
	struct ecm_function_config *ecm = f->config;

	if (sscanf(buf, "%02x:%02x:%02x:%02x:%02x:%02x\n",
		    (int *)&ecm->ethaddr[0], (int *)&ecm->ethaddr[1],
		    (int *)&ecm->ethaddr[2], (int *)&ecm->ethaddr[3],
		    (int *)&ecm->ethaddr[4], (int *)&ecm->ethaddr[5]) == 6)
		return size;
	return -EINVAL;
}

static DEVICE_ATTR(ethaddr, S_IRUGO | S_IWUSR, ecm_ethaddr_show,
					       ecm_ethaddr_store);

static ssize_t ecm_vendorID_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct android_usb_function *f = dev_get_drvdata(dev);
	struct ecm_function_config *config = f->config;
	return sprintf(buf, "%04x\n", config->vendorID);
}

static ssize_t ecm_vendorID_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct android_usb_function *f = dev_get_drvdata(dev);
	struct ecm_function_config *config = f->config;
	int value;

	if (sscanf(buf, "%04x", &value) == 1) {
		config->vendorID = value;
		return size;
	}
	return -EINVAL;
}

static DEVICE_ATTR(vendorID, S_IRUGO | S_IWUSR, ecm_vendorID_show,
						ecm_vendorID_store);

static struct device_attribute *ecm_function_attributes[] = {
	&dev_attr_manufacturer,
	&dev_attr_ethaddr,
	&dev_attr_vendorID,
	NULL
};

static struct android_usb_function ecm_function = {
	.name			= "ecm",
	.init			= ecm_function_init,
	.cleanup		= ecm_function_cleanup,
	.bind_config	= ecm_function_bind_config,
	.unbind_config	= ecm_function_unbind_config,
	.attributes		= ecm_function_attributes,
};
#endif 

struct mass_storage_function_config {
	struct fsg_config fsg;
	struct fsg_common *common;
};

static int mass_storage_function_init(struct android_usb_function *f,
					struct usb_composite_dev *cdev)
{
	struct mass_storage_function_config *config = NULL;
	struct fsg_common *common = NULL;
	int err;

	config = kzalloc(sizeof(struct mass_storage_function_config),
								GFP_KERNEL);
	if (!config)
		return -ENOMEM;

	config->fsg.nluns = 1;
	config->fsg.luns[0].removable = 1;

	common = fsg_common_init(NULL, cdev, &config->fsg);
	if (IS_ERR(common)) {
		kfree(config);
		return PTR_ERR(common);
	}

	err = sysfs_create_link(&f->dev->kobj,
				&common->luns[0].dev.kobj,
				"lun");
	if (err) {
		kfree(config);
		return err;
	}

	config->common = common;
	f->config = config;
	return 0;
}

static void mass_storage_function_cleanup(struct android_usb_function *f)
{
	kfree(f->config);
	f->config = NULL;
}

static int mass_storage_function_bind_config(struct android_usb_function *f,
						struct usb_configuration *c)
{
	struct mass_storage_function_config *config = f->config;
	return fsg_bind_config(c->cdev, c, config->common);
}

static ssize_t mass_storage_inquiry_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct android_usb_function *f = dev_get_drvdata(dev);
	struct mass_storage_function_config *config = f->config;
	return sprintf(buf, "%s\n", config->common->inquiry_string);
}

static ssize_t mass_storage_inquiry_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct android_usb_function *f = dev_get_drvdata(dev);
	struct mass_storage_function_config *config = f->config;
	if (size >= sizeof(config->common->inquiry_string))
		return -EINVAL;
	if (sscanf(buf, "%s", config->common->inquiry_string) != 1)
		return -EINVAL;
	return size;
}

static DEVICE_ATTR(inquiry_string, S_IRUGO | S_IWUSR,
					mass_storage_inquiry_show,
					mass_storage_inquiry_store);

static struct device_attribute *mass_storage_function_attributes[] = {
	&dev_attr_inquiry_string,
	NULL
};

static struct android_usb_function mass_storage_function = {
	.name		= "mass_storage",
	.init		= mass_storage_function_init,
	.cleanup	= mass_storage_function_cleanup,
	.bind_config	= mass_storage_function_bind_config,
	.attributes	= mass_storage_function_attributes,
};


static int accessory_function_init(struct android_usb_function *f,
					struct usb_composite_dev *cdev)
{
	return acc_setup();
}

static void accessory_function_cleanup(struct android_usb_function *f)
{
	acc_cleanup();
}

static int accessory_function_bind_config(struct android_usb_function *f,
						struct usb_configuration *c)
{
	return acc_bind_config(c);
}

static int accessory_function_ctrlrequest(struct android_usb_function *f,
						struct usb_composite_dev *cdev,
						const struct usb_ctrlrequest *c)
{
	return acc_ctrlrequest(cdev, c);
}

static struct android_usb_function accessory_function = {
	.name		= "accessory",
	.init		= accessory_function_init,
	.cleanup	= accessory_function_cleanup,
	.bind_config	= accessory_function_bind_config,
	.ctrlrequest	= accessory_function_ctrlrequest,
};


static struct android_usb_function *supported_functions[] = {
	&adb_function,
	&acm_function,
	&mtp_function,
	&ptp_function,
#if defined(CONFIG_LGE_ANDROID_USB_DONOTUSE)
	&ecm_function,
#else	
	&rndis_function,
#endif
	&mass_storage_function,
	&accessory_function,
/* LGE_SJIT_S 10/21/2011 [mohamed.khadri@lge.com]
            LG Gadget driver - nmea and diag funcation extensions  */
#if defined(CONFIG_LGE_ANDROID_USB)
        &nmea_function,
        &diag_function,
#endif
/* LGE_SJIT_E 10/21/2011 [mohamed.khadri@lge.com]
            LG Gadget driver - nmea and diag funcation extensions  */
	NULL
};


static int android_init_functions(struct android_usb_function **functions,
				  struct usb_composite_dev *cdev)
{
	struct android_dev *dev = _android_dev;
	struct android_usb_function *f = NULL;
	struct device_attribute **attrs = NULL;
	struct device_attribute *attr = NULL;
	int err;
	int index = 0;

	for (; (f = *functions++); index++) {
		f->dev_name = kasprintf(GFP_KERNEL, "f_%s", f->name);
		f->dev = device_create(android_class, dev->dev,
				MKDEV(0, index), f, f->dev_name);
		if (IS_ERR(f->dev)) {
			pr_err("%s: Failed to create dev %s", __func__,
							f->dev_name);
			err = PTR_ERR(f->dev);
			goto err_create;
		}

		if (f->init) {
			err = f->init(f, cdev);
			if (err) {
				pr_err("%s: Failed to init %s", __func__,
								f->name);
				goto err_out;
			}
		}

		attrs = f->attributes;
		if (attrs) {
			while ((attr = *attrs++) && !err)
				err = device_create_file(f->dev, attr);
		}
		if (err) {
			pr_err("%s: Failed to create function %s attributes",
					__func__, f->name);
			goto err_out;
		}
	}
	return 0;

err_out:
	device_destroy(android_class, f->dev->devt);
err_create:
	kfree(f->dev_name);
	return err;
}

static void android_cleanup_functions(struct android_usb_function **functions)
{
	struct android_usb_function *f = NULL;

	while (*functions) {
		f = *functions++;

		if (f->dev) {
			device_destroy(android_class, f->dev->devt);
			kfree(f->dev_name);
		}

		if (f->cleanup)
			f->cleanup(f);
	}
}

static int
android_bind_enabled_functions(struct android_dev *dev,
			       struct usb_configuration *c)
{
	struct android_usb_function *f = NULL;
	int ret;

	list_for_each_entry(f, &dev->enabled_functions, enabled_list) {
		ret = f->bind_config(f, c);
		if (ret) {
			pr_err("%s: %s failed", __func__, f->name);
			return ret;
		}
	}
	return 0;
}

static void
android_unbind_enabled_functions(struct android_dev *dev,
			       struct usb_configuration *c)
{
	struct android_usb_function *f = NULL;

	list_for_each_entry(f, &dev->enabled_functions, enabled_list) {
		if (f->unbind_config)
			f->unbind_config(f, c);
	}
}

static int android_enable_function(struct android_dev *dev, char *name)
{
	struct android_usb_function **functions = dev->functions;
	struct android_usb_function *f = NULL;
	while ((f = *functions++)) {
		if (!strcmp(name, f->name)) {
			list_add_tail(&f->enabled_list, &dev->enabled_functions);
			return 0;
		}
	}
	return -EINVAL;
}

/*-------------------------------------------------------------------------*/
/* /sys/class/android_usb/android%d/ interface */

static ssize_t
functions_show(struct device *pdev, struct device_attribute *attr, char *buf)
{
	struct android_dev *dev = dev_get_drvdata(pdev);
	struct android_usb_function *f;
	char *buff = buf;

	mutex_lock(&dev->mutex);

	list_for_each_entry(f, &dev->enabled_functions, enabled_list)
		buff += sprintf(buff, "%s,", f->name);

	mutex_unlock(&dev->mutex);

	if (buff != buf)
		*(buff-1) = '\n';
	return buff - buf;
}

static ssize_t
functions_store(struct device *pdev, struct device_attribute *attr,
			       const char *buff, size_t size)
{
	struct android_dev *dev = dev_get_drvdata(pdev);
	char *name = NULL;
	char buf[256];
	char *b = NULL;
	int err;

	mutex_lock(&dev->mutex);

	if (dev->enabled) {
		mutex_unlock(&dev->mutex);
		return -EBUSY;
	}

	INIT_LIST_HEAD(&dev->enabled_functions);

	strncpy(buf, buff, sizeof(buf));
	b = strim(buf);

	while (b) {
		name = strsep(&b, ",");
		if (name) {
			err = android_enable_function(dev, name);
			if (err)
/* LGE_SJIT_S 10/21/2011 [mohamed.khadri@lge.com]
            LG Gadget driver */
#if !defined(CONFIG_LGE_ANDROID_USB)
				pr_err("android_usb: Cannot enable '%s'", name);
#else
				pr_err("LG-android_usb: Cannot enable '%s'", name);
#endif
/* LGE_SJIT_E 10/21/2011 [mohamed.khadri@lge.com]
            LG Gadget driver */
		}
	}

	mutex_unlock(&dev->mutex);

	return size;
}

static ssize_t enable_show(struct device *pdev, struct device_attribute *attr,
			   char *buf)
{
	struct android_dev *dev = dev_get_drvdata(pdev);
	return sprintf(buf, "%d\n", dev->enabled);
}

static ssize_t enable_store(struct device *pdev, struct device_attribute *attr,
			    const char *buff, size_t size)
{
	struct android_dev *dev = dev_get_drvdata(pdev);
	struct usb_composite_dev *cdev = dev->cdev;
	struct android_usb_function *f;
	int enabled = 0;

	mutex_lock(&dev->mutex);

	sscanf(buff, "%d", &enabled);
	if (enabled && !dev->enabled) {
/* LGE_SJIT_S 10/21/2011 [mohamed.khadri@lge.com]
            LG Gadget driver - Override if enabled from kconfig   */
#if defined(CONFIG_LGE_ANDROID_USB_FUNC)
		functions_store(pdev,attr,CONFIG_LGE_ANDROID_USB_FUNC_LIST,0);
#endif
		cdev->next_string_id = 0;
		/* update values in composite driver's copy of device descriptor */
/* LGE_CHANGE_S 4/30/2012 [edwardk.kim@lge.com] */
//#if !defined(CONFIG_LGE_ANDROID_USB)
		cdev->desc.idVendor = device_desc.idVendor;
//#endif
/* LGE_CHANGE_E 4/30/2012 [edwardk.kim@lge.com] */

#if !defined(CONFIG_LGE_ANDROID_USB_PID)
		cdev->desc.idProduct = device_desc.idProduct;
#endif
/* LGE_SJIT_E 10/21/2011 [mohamed.khadri@lge.com]
            LG Gadget driver - Override if enabled from kconfig  */
		cdev->desc.bcdDevice = device_desc.bcdDevice;
		cdev->desc.bDeviceClass = device_desc.bDeviceClass;
		cdev->desc.bDeviceSubClass = device_desc.bDeviceSubClass;
		cdev->desc.bDeviceProtocol = device_desc.bDeviceProtocol;
		list_for_each_entry(f, &dev->enabled_functions, enabled_list) {
			if (f->enable)
				f->enable(f);
		}
		android_enable(dev);
		dev->enabled = true;
	} else if (!enabled && dev->enabled) {
		/* Cancel pending control requests */
		usb_ep_dequeue(cdev->gadget->ep0, cdev->req);
		android_disable(dev);
		list_for_each_entry(f, &dev->enabled_functions, enabled_list) {
			if (f->disable)
				f->disable(f);
		}
		dev->enabled = false;
	} else {
/* LGE_SJIT_S 10/21/2011 [mohamed.khadri@lge.com]
            LG Gadget driver */
#if !defined(CONFIG_LGE_ANDROID_USB)
		pr_err("android_usb: already %s\n",
                             dev->enabled ? "enabled" : "disabled");
#else
		pr_err("LG-android_usb: already %s\n",
				dev->enabled ? "enabled" : "disabled");
#endif
/* LGE_SJIT_E 10/21/2011 [mohamed.khadri@lge.com]
            LG Gadget driver */
	}

	mutex_unlock(&dev->mutex);
	return size;
}

static ssize_t state_show(struct device *pdev, struct device_attribute *attr,
			   char *buf)
{
	struct android_dev *dev = dev_get_drvdata(pdev);
	struct usb_composite_dev *cdev = dev->cdev;
	char *state = "DISCONNECTED";
	unsigned long flags;

	if (!cdev)
		goto out;

	spin_lock_irqsave(&cdev->lock, flags);
        if (cdev->config)
		state = "CONFIGURED";
	else if (dev->connected)
		state = "CONNECTED";
	spin_unlock_irqrestore(&cdev->lock, flags);
out:
	return sprintf(buf, "%s\n", state);
}
/* LGE_SJIT_S 10/21/2011 [mohamed.khadri@lge.com]
            LG Gadget driver - Added flag  */
#define DESCRIPTOR_ATTR(field, format_string, flags)			\
static ssize_t								\
field ## _show(struct device *dev, struct device_attribute *attr,	\
		char *buf)						\
{									\
	return sprintf(buf, format_string, device_desc.field);		\
}									\
static ssize_t								\
field ## _store(struct device *dev, struct device_attribute *attr,	\
		const char *buf, size_t size)		       		\
{									\
	int value;					       		\
	if (sscanf(buf, format_string, &value) == 1) {			\
		device_desc.field = value;				\
		return size;						\
	}								\
	return -1;							\
}									\
static DEVICE_ATTR(field, flags, field ## _show, field ## _store);
/* LGE_SJIT_E 10/21/2011 [mohamed.khadri@lge.com]
            LG Gadget driver - Added flag  */
#define DESCRIPTOR_STRING_ATTR(field, buffer)				\
static ssize_t								\
field ## _show(struct device *dev, struct device_attribute *attr,	\
		char *buf)						\
{									\
	return sprintf(buf, "%s", buffer);				\
}									\
static ssize_t								\
field ## _store(struct device *dev, struct device_attribute *attr,	\
		const char *buf, size_t size)		       		\
{									\
	if (size >= sizeof(buffer)) return -EINVAL;			\
	if (sscanf(buf, "%s", buffer) == 1) {			       	\
		return size;						\
	}								\
	return -1;							\
}									\
static DEVICE_ATTR(field, S_IRUGO | S_IWUSR, field ## _show, field ## _store);

/* LGE_SJIT_S 10/21/2011 [mohamed.khadri@lge.com]
            LG Gadget driver - Read only  */
#if !defined(CONFIG_LGE_ANDROID_USB)
DESCRIPTOR_ATTR(idVendor, "%04x\n", S_IRUGO|S_IWUSR)
#else
/* LGE_CHANGE_S 4/30/2012 [edwardk.kim@lge.com] */
//DESCRIPTOR_ATTR(idVendor, "%04x\n", S_IRUGO)											
DESCRIPTOR_ATTR(idVendor, "%04x\n", S_IRUGO|S_IWUSR)
/* LGE_CHANGE_E 4/30/2012 [edwardk.kim@lge.com] */
#endif
#if !defined(CONFIG_LGE_ANDROID_USB_PID)
DESCRIPTOR_ATTR(idProduct, "%04x\n", S_IRUGO|S_IWUSR)
#else
DESCRIPTOR_ATTR(idProduct, "%04x\n", S_IRUGO)
#endif
/* LGE_SJIT_E 10/21/2011 [mohamed.khadri@lge.com]
            LG Gadget driver - Read only  */
DESCRIPTOR_ATTR(bcdDevice, "%04x\n", S_IRUGO|S_IWUSR)
DESCRIPTOR_ATTR(bDeviceClass, "%02x\n", S_IRUGO|S_IWUSR)
DESCRIPTOR_ATTR(bDeviceSubClass, "%02x\n", S_IRUGO|S_IWUSR)
DESCRIPTOR_ATTR(bDeviceProtocol, "%02x\n", S_IRUGO|S_IWUSR)
DESCRIPTOR_STRING_ATTR(iManufacturer, manufacturer_string)
DESCRIPTOR_STRING_ATTR(iProduct, product_string)
DESCRIPTOR_STRING_ATTR(iSerial, serial_string)
/* LGE_SJIT_S 10/21/2011 [mohamed.khadri@lge.com]
            LG Gadget driver -  Read only if enabled from kconfig  */
#if !defined(CONFIG_LGE_ANDROID_USB_FUNC)
static DEVICE_ATTR(functions, S_IRUGO | S_IWUSR, functions_show, functions_store);
#else 
static DEVICE_ATTR(functions, S_IRUGO, functions_show, NULL);
#endif
/* LGE_SJIT_E 10/21/2011 [mohamed.khadri@lge.com]
            LG Gadget driver - Read only if enabled from kconfig  */
static DEVICE_ATTR(enable, S_IRUGO | S_IWUSR, enable_show, enable_store);
static DEVICE_ATTR(state, S_IRUGO, state_show, NULL);

static struct device_attribute *android_usb_attributes[] = {
	&dev_attr_idVendor,
	&dev_attr_idProduct,
	&dev_attr_bcdDevice,
	&dev_attr_bDeviceClass,
	&dev_attr_bDeviceSubClass,
	&dev_attr_bDeviceProtocol,
	&dev_attr_iManufacturer,
	&dev_attr_iProduct,
	&dev_attr_iSerial,
	&dev_attr_functions,
	&dev_attr_enable,
	&dev_attr_state,
	NULL
};

/*-------------------------------------------------------------------------*/
/* Composite driver */

static int android_bind_config(struct usb_configuration *c)
{
	struct android_dev *dev = _android_dev;
	int ret = 0;

	ret = android_bind_enabled_functions(dev, c);
	if (ret)
		return ret;

	return 0;
}

static void android_unbind_config(struct usb_configuration *c)
{
	struct android_dev *dev = _android_dev;

	android_unbind_enabled_functions(dev, c);
}

static int android_bind(struct usb_composite_dev *cdev)
{
	struct android_dev *dev = _android_dev;
	struct usb_gadget	*gadget = cdev->gadget;
	int			gcnum, id, ret;
#if defined(CONFIG_LGE_ANDROID_USB)
	unsigned int val[4] = { 0 };
	unsigned int reg;
#endif // #if defined(CONFIG_LGE_ANDROID_USB)

	usb_gadget_disconnect(gadget);

	ret = android_init_functions(dev->functions, cdev);
	if (ret)
		return ret;

	/* Allocate string descriptor numbers ... note that string
	 * contents can be overridden by the composite_dev glue.
	 */
	id = usb_string_id(cdev);
	if (id < 0)
		return id;
	strings_dev[STRING_MANUFACTURER_IDX].id = id;
	device_desc.iManufacturer = id;

	id = usb_string_id(cdev);
	if (id < 0)
		return id;
	strings_dev[STRING_PRODUCT_IDX].id = id;
	device_desc.iProduct = id;

	/* Default strings - should be updated by userspace */
/* LGE_SJIT_S 10/21/2011 [mohamed.khadri@lge.com]
            LG Gadget driver   */
#if defined(CONFIG_LGE_ANDROID_USB)
	strncpy(manufacturer_string, "LG_Electronics", sizeof(manufacturer_string) - 1);
	strncpy(product_string, "LG Android", sizeof(product_string) - 1);
/* LGE_SJIT_E 10/21/2011 [mohamed.khadri@lge.com]
            LG Gadget driver */

/* LGE_CHANGE_S 15/05/2012 [edwardk.kim@lge.com] */
#ifdef CONFIG_ARCH_OMAP3
#define DIE_ID_REG_BASE			(L4_WK_34XX_BASE + 0xA000)
#define DIE_ID_REG_OFFSET		 0x200
#endif /* CONFIG_ARCH_OMAP3 */

	reg = DIE_ID_REG_BASE + DIE_ID_REG_OFFSET;

	if (cpu_is_omap34xx()) {
		val[0] = omap_readl(reg + 0x18);
		val[1] = omap_readl(reg + 0x1C);
		val[2] = omap_readl(reg + 0x20);
		val[3] = omap_readl(reg + 0x24);
	}
	
	snprintf(serial_string, sizeof(serial_string) -1 /* MAX_USB_SERIAL_NUM */, "%08X%08X%08X%08X",
					val[3], val[2], val[1], val[0]);
/* LGE_CHANGE_E 15/05/2012 [edwardk.kim@lge.com] */
#else 
    strncpy(manufacturer_string, "Android", sizeof(manufacturer_string) - 1);
    strncpy(product_string, "Android", sizeof(product_string) - 1);
		strncpy(serial_string, "0123456789ABCDEF", sizeof(serial_string) - 1);
#endif /* defined(CONFIG_LGE_ANDROID_USB) */

	id = usb_string_id(cdev);
	if (id < 0)
		return id;
	strings_dev[STRING_SERIAL_IDX].id = id;
	device_desc.iSerialNumber = id;

	gcnum = usb_gadget_controller_number(gadget);
	if (gcnum >= 0)
		device_desc.bcdDevice = cpu_to_le16(0x0200 + gcnum);
	else {
		/* gadget zero is so simple (for now, no altsettings) that
		 * it SHOULD NOT have problems with bulk-capable hardware.
		 * so just warn about unrcognized controllers -- don't panic.
		 *
		 * things like configuration and altsetting numbering
		 * can need hardware-specific attention though.
		 */
		pr_warning("%s: controller '%s' not recognized\n",
			longname, gadget->name);
		device_desc.bcdDevice = __constant_cpu_to_le16(0x9999);
	}

	usb_gadget_set_selfpowered(gadget);
	dev->cdev = cdev;

	return 0;
}

static int android_usb_unbind(struct usb_composite_dev *cdev)
{
	struct android_dev *dev = _android_dev;

	cancel_work_sync(&dev->work);
	android_cleanup_functions(dev->functions);
	return 0;
}

static struct usb_composite_driver android_usb_driver = {
	.name		= "android_usb",
	.dev		= &device_desc,
	.strings	= dev_strings,
	.unbind		= android_usb_unbind,
};

static int
android_setup(struct usb_gadget *gadget, const struct usb_ctrlrequest *c)
{
	struct android_dev		*dev = _android_dev;
	struct usb_composite_dev	*cdev = get_gadget_data(gadget);
	struct usb_request		*req = cdev->req;
	struct android_usb_function	*f = NULL;
	int value = -EOPNOTSUPP;
	unsigned long flags;

	req->zero = 0;
	req->complete = composite_setup_complete;
	req->length = 0;
	gadget->ep0->driver_data = cdev;

	list_for_each_entry(f, &dev->enabled_functions, enabled_list) {
		if (f->ctrlrequest) {
			value = f->ctrlrequest(f, cdev, c);
			if (value >= 0)
				break;
		}
	}

	/* Special case the accessory function.
	 * It needs to handle control requests before it is enabled.
	 */
	if (value < 0)
		value = acc_ctrlrequest(cdev, c);

	if (value < 0)
		value = composite_setup(gadget, c);

	spin_lock_irqsave(&cdev->lock, flags);
	if (!dev->connected) {
		dev->connected = 1;
		schedule_work(&dev->work);
	}
	else if (c->bRequest == USB_REQ_SET_CONFIGURATION && cdev->config) {
		schedule_work(&dev->work);
	}
	spin_unlock_irqrestore(&cdev->lock, flags);

	return value;
}
/* LGE_CHANGE_S 6/14/2012 [edwardk.kim@lge.com] */	
 // for usb disconnect event  
void android_USB_disconnect(){  
     struct android_dev *dev = _android_dev;  
     struct usb_composite_dev *cdev = get_gadget_data(dev->cdev->gadget);  
     unsigned long flags;  
     
     composite_disconnect(dev->cdev->gadget);  
#if 1  	
  	printk(KERN_INFO "====>[ANDROID].usb_discon() from MUIC\n");
//	dump_stack();
#endif      
     spin_lock_irqsave(&cdev->lock, flags);  
     dev->disconnected = 1;  
     dev->connected = 0; 
     schedule_work(&dev->work);  
     spin_unlock_irqrestore(&cdev->lock, flags);  
}  
/* LGE_CHANGE_E 6/14/2012 [edwardk.kim@lge.com] */	
static void android_disconnect(struct usb_gadget *gadget)
{
	struct android_dev *dev = _android_dev;
	struct usb_composite_dev *cdev = get_gadget_data(gadget);
	unsigned long flags;
#if 1 	
	printk(KERN_INFO "====>[ANDROID].usb_discon()\n");
//	dump_stack();
#endif
	composite_disconnect(gadget);

	spin_lock_irqsave(&cdev->lock, flags);
	dev->connected = 0;
	schedule_work(&dev->work);
	spin_unlock_irqrestore(&cdev->lock, flags);
}

static int android_create_device(struct android_dev *dev)
{
	struct device_attribute **attrs = android_usb_attributes;
	struct device_attribute *attr = NULL;
	int err;

	dev->dev = device_create(android_class, NULL,
					MKDEV(0, 0), NULL, "android0");
	if (IS_ERR(dev->dev))
		return PTR_ERR(dev->dev);

	dev_set_drvdata(dev->dev, dev);

	while ((attr = *attrs++)) {
		err = device_create_file(dev->dev, attr);
		if (err) {
			device_destroy(android_class, dev->dev->devt);
			return err;
		}
	}
	return 0;
}


static int __init init(void)
{
	struct android_dev *dev = NULL;
	int err;
	android_class = class_create(THIS_MODULE, "android_usb");
	if (IS_ERR(android_class))
		return PTR_ERR(android_class);

	dev = kzalloc(sizeof(*dev), GFP_KERNEL);
	if (!dev)
		return -ENOMEM;
/* LGE_SJIT_S 10/21/2011 [mohamed.khadri@lge.com]
            LG Gadget driver */
#if defined(CONFIG_LGE_ANDROID_USB)
        dev->serial_flags = 0x00; 
#endif
/* LGE_SJIT_E 10/21/2011 [mohamed.khadri@lge.com]
            LG Gadget driver */
	dev->disable_depth = 1;
	dev->functions = supported_functions;
	INIT_LIST_HEAD(&dev->enabled_functions);
	INIT_WORK(&dev->work, android_work);
	mutex_init(&dev->mutex);

	err = android_create_device(dev);
	if (err) {
		class_destroy(android_class);
		kfree(dev);
		return err;
	}
/* LGE_CHANGE_S 6/14/2012 [edwardk.kim@lge.com] */
	dev->connected = 0;  
	dev->sw_connected = 0;
	dev->disconnected = 0;
/* LGE_CHANGE_E 6/14/2012 [edwardk.kim@lge.com] */

	_android_dev = dev;

	/* Override composite driver functions */
	composite_driver.setup = android_setup;
	composite_driver.disconnect = android_disconnect;


	pr_debug("%s: android init start\n", __func__);


	return usb_composite_probe(&android_usb_driver, android_bind);
}
module_init(init);

static void __exit cleanup(void)
{
	usb_composite_unregister(&android_usb_driver);
	class_destroy(android_class);
	kfree(_android_dev);
	_android_dev = NULL;
}
module_exit(cleanup);
