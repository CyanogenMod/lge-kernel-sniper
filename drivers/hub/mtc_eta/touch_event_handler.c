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

//#define MTC_ETA_DEBUG
#ifdef MTC_ETA_DEBUG
#define PDEBUG(fmt, args...) printk("mtc_eta_touch: " fmt, ## args)
#else
#define PDEBUG(fmt, args...)
#endif

static struct input_handler input_handler;
static struct work_struct event_log_work;
static struct log_data_touch_event touch_event;

void mtc_eta_add_logging_event(struct mtc_eta_log *log);

static int eta_abs_event[] = {
	ABS_MT_TOUCH_MAJOR,
	ABS_MT_WIDTH_MAJOR,
	ABS_MT_POSITION_X,
	ABS_MT_POSITION_Y,
};

/* (MT sync -> sync) report after no major, x or y report means that touch is pen-up */
enum {
	TOUCH_STATE_NONE,
	TOUCH_STATE_DOWN,
	TOUCH_STATE_SYNC,
	TOUCH_STATE_SYNC_MT,
};
static int touch_state = TOUCH_STATE_NONE;

static int mtc_eta_event_log_connect(struct input_handler *handler, struct input_dev *dev, const struct input_device_id *id)
{
	int i;
	int ret;
	struct input_handle *handle;

	if (dev->name == NULL)
		return 0;

	if (strcmp(dev->name, "hub_synaptics_touch") != 0)
		return 0;

	for (i = 0 ; i < ARRAY_SIZE(eta_abs_event) - 1 ; i++) {
		if (test_bit(eta_abs_event[i], dev->absbit))
			break;
	}
	if (i == ARRAY_SIZE(eta_abs_event))
		return -ENODEV;

	PDEBUG("connect () %s\n",dev->name);

	handle = kzalloc(sizeof(*handle), GFP_KERNEL);
	if(!handle)
		return -ENOMEM;

	handle->dev = dev;
	handle->handler = handler;
	handle->name = "mtc_eta_touch_event_log";
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

	new_log_item->id = MTC_ETA_LOG_ID_TOUCH;
	new_log_item->data.touch.action = touch_event.action;
	new_log_item->data.touch.x = touch_event.x;
	new_log_item->data.touch.y = touch_event.y;
	PDEBUG("NEW: id %d\n", new_log_item->id);
	PDEBUG("NEW: touch action 0x%x\n", new_log_item->data.touch.action);
	printk("NEW: touch x %d\n", new_log_item->data.touch.x);
	printk("NEW: touch y %d\n", new_log_item->data.touch.y);

	mtc_eta_add_logging_event(new_log_item);
}

static void mtc_eta_event_log_event(struct input_handle *handle, unsigned int type, unsigned int code, int value)
{
	int temp_x =0;
	int temp_y= 0;
	
	printk("type: %u, code: 0x%x, value: %d\n", type, code, value);
	
	switch (type) {
		case EV_SYN:
			switch (code) {
				case SYN_REPORT:
					if (touch_state == TOUCH_STATE_SYNC_MT) {
						touch_event.action = ETA_TOUCH_UP;
						touch_state = TOUCH_STATE_NONE;
					} else if (touch_state == TOUCH_STATE_DOWN) {
						touch_state = TOUCH_STATE_SYNC;
					}
					else {
						PDEBUG("Wrong touch state in SYN_REPORT\n");
						return;
					}
					schedule_work(&event_log_work);
					printk("handler: touch action 0x%x\n",
							touch_event.action);
					printk("handler: touch x %u\n",
							touch_event.x);
					printk("handler: touch y %u\n",
							touch_event.y);

					break;
				case SYN_MT_REPORT:
					if (touch_state == TOUCH_STATE_SYNC) {
						touch_state = TOUCH_STATE_SYNC_MT;
					}
#ifdef MTC_ETA_DEBUG
					else if (touch_state != TOUCH_STATE_DOWN) {
						PDEBUG("Wrong touch state in SYN_MT_REPORT\n");
					}
#endif

					break;
			}
			break;
		case EV_ABS:
			switch (code) {
				case ABS_MT_TOUCH_MAJOR:
					if (touch_state == TOUCH_STATE_NONE) {
						touch_event.action = ETA_TOUCH_DOWN;
					} else if (touch_state == TOUCH_STATE_SYNC) {
						touch_event.action = ETA_TOUCH_MOVETO;
					}
#ifdef MTC_ETA_DEBUG
					else {
						PDEBUG("Wrong touch state in ABS_MT_TOUCH_MAJOR\n");
					}
#endif
					touch_state = TOUCH_STATE_DOWN;
					break;
				case ABS_MT_POSITION_X:
					temp_x = (value * 462 / 1000) ;

					touch_event.x = temp_x;
					//touch_event.x = (value * 1000l / 2055) - 15; // 2.055 = 986(max X)/480(width)
#ifdef MTC_ETA_DEBUG
					if (touch_state != TOUCH_STATE_DOWN) {
						printk("Wrong touch state in ABS_MT_POSITION_X\n");
					}
#endif
					break;
				case ABS_MT_POSITION_Y:
					temp_y = (value * 462 / 1000) ;

						
					touch_event.y = temp_y;

					//touch_event.y = (value * 1000 / 2055) - 15; // 2.055 = 1644(max Y)/800(height)
#ifdef MTC_ETA_DEBUG
					if (touch_state != TOUCH_STATE_DOWN) {
						printk("Wrong touch state in ABS_MT_POSITION_Y\n");
					}
#endif
					break;
			}
			break;
	}

}

int hub_start_touch_logging(void)
{
	int ret = 0;

	input_handler.name = "mtc_eta_log_touch";
	input_handler.connect = mtc_eta_event_log_connect;
	input_handler.disconnect = mtc_eta_event_log_disconnect;
	input_handler.event = mtc_eta_event_log_event;
	input_handler.id_table = mtc_eta_event_log_ids;
	ret = input_register_handler(&input_handler);
	if (ret != 0)
		printk("%s:fail to registers input handler\n", __func__);

	INIT_WORK(&event_log_work, event_log_work_func);

	return ret;
}

void hub_stop_touch_logging(void)
{
	input_unregister_handler(&input_handler);
}

