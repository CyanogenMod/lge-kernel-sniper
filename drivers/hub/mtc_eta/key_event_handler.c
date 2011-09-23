/*
 *  Copyright (c) 2010 LGE.
 *
 *  All source code in this file is licensed under the following license
 *  except where indicated.
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  version 2 as published by the Free Software Foundation.
 *  
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, you can find it at http://www.fsf.org
 */

#include <linux/platform_device.h>
#include <linux/input.h>
#include <linux/workqueue.h>
#include <linux/slab.h>

#include "lge_mtc_eta.h"

#define MTC_ETA_DEBUG
#ifdef MTC_ETA_DEBUG
#define PDEBUG(fmt, args...) printk("mtc_eta_key: " fmt, ## args)
#else
#define PDEBUG(fmt, args...)
#endif
#define NPDEBUG(fmt, args...)

static struct input_handler input_handler;
static struct work_struct event_log_work;
static struct log_data_key_event key_event;

void mtc_eta_add_logging_event(struct mtc_eta_log *log);

static int eta_key_list[] = {
	KEY_VOLUMEUP,
	KEY_VOLUMEDOWN,
//20110416 yongman.kwon@lge.com [LS855] for logging touch key event [START]
//these key is supported by touch sensor.
//	KEY_MENU,
//	KEY_HOME,
//	KEY_BACK,
//	KEY_SEARCH,
//20110416 yongman.kwon@lge.com [LS855] for logging touch key event [END]
	KEY_KPJPCOMMA,//hak.lee@lge.com
	KEY_POWER,//hak.lee@lge.com
};

static int mtc_eta_event_log_connect(struct input_handler *handler, struct input_dev *dev, const struct input_device_id *id)
{
	int i;
	int ret;
	struct input_handle *handle;

	if (dev->name == NULL)
		return 0;
//hak.lee@lge.com
	if ((strcmp(dev->name, "TWL4030_Keypad") != 0) &&
			(strcmp(dev->name, "twl4030_pwrbutton") != 0))
		return 0;
//hak.lee@lge.com
#if 0
	for (i = 0; i < KEY_MAX; i++) {
		if (test_bit(i, state->keybit) && test_bit(i, dev->keybit))
			break;
	}
	if (i == KEY_MAX)
		return -ENODEV;
#endif

	for (i = 0 ; i < ARRAY_SIZE(eta_key_list) - 1; i++) {
		if (test_bit(eta_key_list[i], dev->keybit))
			break;
	}
	if (i == ARRAY_SIZE(eta_key_list))
		return -ENODEV;

	PDEBUG("connect () %s\n",dev->name);

	handle = kzalloc(sizeof(*handle), GFP_KERNEL);
	if(!handle)
		return -ENOMEM;

	handle->dev = dev;
	handle->handler = handler;
	handle->name = "mtc_eta_key_event_log";
	handle->private = NULL;

	ret = input_register_handle(handle);
	if (ret)
		goto err_input_register_handle;

	ret = input_open_device(handle);
	if (ret)
		goto err_input_open_device;

	return 0;

err_input_open_device:
	input_unregister_handle(handle);
err_input_register_handle:
	kfree(handle);
	return ret;
}

static void mtc_eta_event_log_disconnect(struct input_handle *handle)
{
	input_close_device(handle);
	input_unregister_handle(handle);
	kfree(handle);
}


static const struct input_device_id mtc_eta_event_log_ids[] = {
	{
		.flags = INPUT_DEVICE_ID_MATCH_EVBIT,
		.evbit = { BIT_MASK(EV_KEY) },
	},
	{ },
};
MODULE_DEVICE_TABLE(input, mtc_eta_event_log_ids);

static void event_log_work_func(struct work_struct *work)
{
	struct mtc_eta_log *new_log_item;

	new_log_item = kmalloc(sizeof(struct mtc_eta_log), GFP_ATOMIC);
	if (!new_log_item) {
		printk("mtc_eta_key: cannot allocate new_item buffer\n");
		return;
	}

	new_log_item->id = MTC_ETA_LOG_ID_KEY;
	new_log_item->data.key.hold = key_event.hold;
	new_log_item->data.key.key_code = key_event.key_code;;
	PDEBUG("NEW: id %d\n", new_log_item->id);
	PDEBUG("NEW: key hold %d\n", new_log_item->data.key.hold);
	PDEBUG("NEW: key code 0x%x\n", new_log_item->data.key.key_code);

	mtc_eta_add_logging_event(new_log_item);
}

static void mtc_eta_event_log_event(struct input_handle *handle, unsigned int type, unsigned int code, int value)
{
	NPDEBUG("type: %u, code: 0x%x, value: %d\n", type, code, value);

	if (type == EV_KEY) {
		key_event.hold = value;
		key_event.key_code = code;
		schedule_work(&event_log_work);
		PDEBUG("handler: key hold %d\n", key_event.hold);
		PDEBUG("handler: key code 0x%x\n", key_event.key_code);
	}
}

int hub_start_key_logging(void)
{
	int ret = 0;

	PDEBUG("start key logging\n");

	input_handler.name = "mtc_eta_log_key";
	input_handler.connect = mtc_eta_event_log_connect;
	input_handler.disconnect = mtc_eta_event_log_disconnect;
	input_handler.event = mtc_eta_event_log_event;
	input_handler.id_table = mtc_eta_event_log_ids;
	ret = input_register_handler(&input_handler);
	if (ret != 0)
		printk("%s: fail to registers input handler\n", __func__);

	INIT_WORK(&event_log_work, event_log_work_func);

	return ret;
}

void hub_stop_key_logging(void)
{
	PDEBUG("stop key logging\n");
	input_unregister_handler(&input_handler);
}

