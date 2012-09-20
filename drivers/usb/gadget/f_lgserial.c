/*
 * f_lgserial.c - generic USB serial function driver
 *
 * Copyright (C) 2003 Al Borchers (alborchers@steinerpoint.com)
 * Copyright (C) 2008 by David Brownell
 * Copyright (C) 2008 by Nokia Corporation
 * Copyright (C) 2011 by LG Electronics
 *
 * This software is distributed under the terms of the GNU General
 * Public License ("GPL") as published by the Free Software Foundation,
 * either version 2 of that License or (at your option) any later version.
 */

#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/device.h>

#include "u_serial.h"
#include "gadget_chips.h"


/*
 * This function packages a simple "generic serial" port with no real
 * control mechanisms, just raw data transfer over two bulk endpoints.
 *
 * Because it's not standardized, this isn't as interoperable as the
 * CDC ACM driver.  However, for many purposes it's just as functional
 * if you can arrange appropriate host side drivers.
 */

struct lgserial_descs {
	struct usb_endpoint_descriptor	*in;
	struct usb_endpoint_descriptor	*out;
};

struct f_lgserial {
	struct gserial			port;
	u8				data_id;
	u8				port_num;

	struct lgserial_descs		fs;
	struct lgserial_descs		hs;
};

static inline struct f_lgserial *func_to_lgserial(struct usb_function *f)
{
	return container_of(f, struct f_lgserial, port.func);
}

/*-------------------------------------------------------------------------*/

/* interface descriptor: */

static struct usb_interface_descriptor lgserial_interface_desc = {
	.bLength =		USB_DT_INTERFACE_SIZE,
	.bDescriptorType =	USB_DT_INTERFACE,
	/* .bInterfaceNumber = DYNAMIC */
	.bNumEndpoints =	2,
	.bInterfaceClass =	USB_CLASS_VENDOR_SPEC,
/* LGE_CHANGE_S 6/20/2012 [edwardk.kim@lge.com] for USB Descriptor Check */	
	.bInterfaceSubClass =	0xFF,
	.bInterfaceProtocol =	0xFF,
/* LGE_CHANGE_E 6/20/2012 [edwardk.kim@lge.com] for USB Descriptor Check */	
	/* .iInterface = DYNAMIC */
};

/* full speed support: */

static struct usb_endpoint_descriptor lgserial_fs_in_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,
	.bEndpointAddress =	USB_DIR_IN,
	.bmAttributes =		USB_ENDPOINT_XFER_BULK,
};

static struct usb_endpoint_descriptor lgserial_fs_out_desc  = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,
	.bEndpointAddress =	USB_DIR_OUT,
	.bmAttributes =		USB_ENDPOINT_XFER_BULK,
};

static struct usb_descriptor_header *lgserial_fs_function[] = {
	(struct usb_descriptor_header *) &lgserial_interface_desc,
	(struct usb_descriptor_header *) &lgserial_fs_in_desc,
	(struct usb_descriptor_header *) &lgserial_fs_out_desc,
	NULL,
};

/* high speed support: */

static struct usb_endpoint_descriptor lgserial_hs_in_desc  = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,
	.bmAttributes =		USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize =	cpu_to_le16(512),
};

static struct usb_endpoint_descriptor lgserial_hs_out_desc  = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,
	.bmAttributes =		USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize =	cpu_to_le16(512),
};

static struct usb_descriptor_header *lgserial_hs_function[] = {
	(struct usb_descriptor_header *) &lgserial_interface_desc,
	(struct usb_descriptor_header *) &lgserial_hs_in_desc,
	(struct usb_descriptor_header *) &lgserial_hs_out_desc,
	NULL,
};

/* string descriptors: */

static struct usb_string lgserial_string_defs[] = {
	[0].s = "LG Generic Serial",
	{  } /* end of list */
};

static struct usb_gadget_strings lgserial_string_table = {
	.language =		0x0409,	/* en-us */
	.strings =		lgserial_string_defs,
};

static struct usb_gadget_strings *lgserial_strings[] = {
	&lgserial_string_table,
	NULL,
};

/*-------------------------------------------------------------------------*/

static int lgserial_set_alt(struct usb_function *f, unsigned intf, unsigned alt)
{
	struct f_lgserial		*lgserial = func_to_lgserial(f);
	struct usb_composite_dev *cdev = f->config->cdev;

	/* we know alt == 0, so this is an activation or a reset */

	if (lgserial->port.in->driver_data) {
		DBG(cdev, "reset LG generic ttyGS%d\n", lgserial->port_num);
		gserial_disconnect(&lgserial->port);
	} else {
		DBG(cdev, "activate LG generic ttyGS%d\n", lgserial->port_num);
		lgserial->port.in_desc = ep_choose(cdev->gadget,
				lgserial->hs.in, lgserial->fs.in);
		lgserial->port.out_desc = ep_choose(cdev->gadget,
				lgserial->hs.out, lgserial->fs.out);
	}
	gserial_connect(&lgserial->port, lgserial->port_num);
	return 0;
}

static void lgserial_disable(struct usb_function *f)
{
	struct f_lgserial	*lgserial = func_to_lgserial(f);
	struct usb_composite_dev *cdev = f->config->cdev;

	DBG(cdev, "LG generic ttyGS%d deactivated\n", lgserial->port_num);
	gserial_disconnect(&lgserial->port);
}

/*-------------------------------------------------------------------------*/

/* serial function driver setup/binding */

static int lgserial_bind(struct usb_configuration *c, struct usb_function *f)
{
	struct usb_composite_dev *cdev = c->cdev;
	struct f_lgserial		*lgserial = func_to_lgserial(f);
	int			status;
	struct usb_ep		*ep;

	/* allocate instance-specific interface IDs */
	status = usb_interface_id(c, f);
	if (status < 0)
		goto fail;
	lgserial->data_id = status;
	lgserial_interface_desc.bInterfaceNumber = status;

	status = -ENODEV;

	/* allocate instance-specific endpoints */
	ep = usb_ep_autoconfig(cdev->gadget, &lgserial_fs_in_desc);
	if (!ep)
		goto fail;
	lgserial->port.in = ep;
	ep->driver_data = cdev;	/* claim */

	ep = usb_ep_autoconfig(cdev->gadget, &lgserial_fs_out_desc);
	if (!ep)
		goto fail;
	lgserial->port.out = ep;
	ep->driver_data = cdev;	/* claim */

	/* copy descriptors, and track endpoint copies */
	f->descriptors = usb_copy_descriptors(lgserial_fs_function);

	lgserial->fs.in = usb_find_endpoint(lgserial_fs_function,
			f->descriptors, &lgserial_fs_in_desc);
	lgserial->fs.out = usb_find_endpoint(lgserial_fs_function,
			f->descriptors, &lgserial_fs_out_desc);


	/* support all relevant hardware speeds... we expect that when
	 * hardware is dual speed, all bulk-capable endpoints work at
	 * both speeds
	 */
	if (gadget_is_dualspeed(c->cdev->gadget)) {
		lgserial_hs_in_desc.bEndpointAddress =
				lgserial_fs_in_desc.bEndpointAddress;
		lgserial_hs_out_desc.bEndpointAddress =
				lgserial_fs_out_desc.bEndpointAddress;

		/* copy descriptors, and track endpoint copies */
		f->hs_descriptors = usb_copy_descriptors(lgserial_hs_function);

		lgserial->hs.in = usb_find_endpoint(lgserial_hs_function,
				f->hs_descriptors, &lgserial_hs_in_desc);
		lgserial->hs.out = usb_find_endpoint(lgserial_hs_function,
				f->hs_descriptors, &lgserial_hs_out_desc);
	}

	DBG(cdev, "LG generic ttyGS%d: %s speed IN/%s OUT/%s\n",
			lgserial->port_num,
			gadget_is_dualspeed(c->cdev->gadget) ? "dual" : "full",
			lgserial->port.in->name, lgserial->port.out->name);
	return 0;

fail:
	/* we might as well release our claims on endpoints */
	if (lgserial->port.out)
		lgserial->port.out->driver_data = NULL;
	if (lgserial->port.in)
		lgserial->port.in->driver_data = NULL;

	ERROR(cdev, "%s: can't bind, err %d\n", f->name, status);

	return status;
}

static void lgserial_unbind(struct usb_configuration *c, struct usb_function *f)
{
	if (gadget_is_dualspeed(c->cdev->gadget))
		usb_free_descriptors(f->hs_descriptors);
	usb_free_descriptors(f->descriptors);
	kfree(func_to_lgserial(f));
}

/**
 * lgserial_bind_config - add a generic serial function to a configuration
 * @c: the configuration to support the serial instance
 * @port_num: /dev/ttyGS* port this interface will use
 * Context: single threaded during gadget setup
 *
 * Returns zero on success, else negative errno.
 *
 * Caller must have called @lgserialial_setup() with enough ports to
 * handle all the ones it binds.  Caller is also responsible
 * for calling @lgserialial_cleanup() before module unload.
 */
int lgserial_bind_config(struct usb_configuration *c, u8 port_num, char* name)
{
	struct f_lgserial	*lgserial;
	int		status;

	/* REVISIT might want instance-specific strings to help
	 * distinguish instances ...
	 */

	/* maybe allocate device-global string ID */
	if (lgserial_string_defs[0].id == 0) {
		status = usb_string_id(c->cdev);
		if (status < 0)
			return status;
		lgserial_string_defs[0].id = status;
	}

	/* allocate and initialize one new instance */
	lgserial = kzalloc(sizeof *lgserial, GFP_KERNEL);
	if (!lgserial)
		return -ENOMEM;

	lgserial->port_num = port_num;

	lgserial->port.func.name = name;
	lgserial->port.func.strings = lgserial_strings;
	lgserial->port.func.bind = lgserial_bind;
	lgserial->port.func.unbind = lgserial_unbind;
	lgserial->port.func.set_alt = lgserial_set_alt;
	lgserial->port.func.disable = lgserial_disable;

	status = usb_add_function(c, &lgserial->port.func);
	if (status)
		kfree(lgserial);
	return status;
}

