/*
 * drivers/media/video/ad5820.c
 *
 * AD5820 Coil Motor (LENS) driver
 *
 * Copyright (C) 2008 Texas Instruments.
 *
 * This package is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * Copyright (C) 2008 Texas Instruments.
 */

#include <linux/mutex.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <media/v4l2-int-device.h>
#include <linux/gpio.h>
#include <linux/platform_device.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/completion.h>
#include <linux/hrtimer.h>
#include <linux/ktime.h>
#include <linux/time.h>

#if defined(CONFIG_REGULATOR_LP8720)
#include <linux/regulator/lp8720.h>
#endif

#include "dw9716.h"
#define LP8720_ENABLE   37
#define DW9716_VCM_ENABLE 167

#define DRIVER_NAME  "dw9716"

#if defined(CONFIG_REGULATOR_LP8720)
extern void subpm_set_output(subpm_output_enum outnum, int onoff);
extern void subpm_output_enable(void);
#endif

static int __devinit  dw9716_probe(struct i2c_client *client,
		const struct i2c_device_id *id);
static int __exit dw9716_remove(struct i2c_client *client);

struct dw9716_device {
	const struct dw9716_platform_data *pdata;
	struct v4l2_int_device *v4l2_int_device;
	struct i2c_client *i2c_client;
	int opened;
	u16 current_lens_posn;
	u16 saved_lens_posn;
	int state;
	int power_state;

    struct dw9716_micro_steps_list mstep_list;
    struct completion mstep_completion;
};

static int __exit
dw9716_remove(struct i2c_client *client)
{
	struct dw9716_device *lens = i2c_get_clientdata(client);

	if (!client->adapter)
		return -ENODEV;	/* our client isn't attached */

	v4l2_int_device_unregister(lens->v4l2_int_device);
	i2c_set_clientdata(client, NULL);

	return 0;
}

static const struct i2c_device_id dw9716_id[] = {
	{ DW9716_NAME, 0 },
	{ },
};

static struct i2c_driver dw9716_i2c_driver = {
	.driver = {
		.name = DW9716_NAME,
		.owner = THIS_MODULE,
	},
	.probe = dw9716_probe,
	.remove = __exit_p(dw9716_remove),
	.id_table = dw9716_id,
};

static struct dw9716_device dw9716 = {
	.state = LENS_NOT_DETECTED,
	.current_lens_posn = DEF_LENS_POSN,
};

static struct vcontrol {
	struct v4l2_queryctrl qc;
	int current_value;
} video_control[] = {
	{
		{
			.id = V4L2_CID_FOCUS_ABSOLUTE,
			.type = V4L2_CTRL_TYPE_INTEGER,
			.name = "Lens Position",
			.minimum = 0,
			.maximum = MAX_FOCUS_POS,
			.step = LENS_POSN_STEP,
			.default_value = DEF_LENS_POSN,
		},
		.current_value = DEF_LENS_POSN,
	},
	{
		{
			.id = V4L2_CID_FOCUS_SCRIPT,
			.type = V4L2_CTRL_TYPE_INTEGER64,
			.name = "Lens script",
			.default_value = 0,
		},
		.current_value = 0,
	}
};

static struct i2c_driver dw9716_i2c_driver;

/**
 * find_vctrl - Finds the requested ID in the video control structure array
 * @id: ID of control to search the video control array for
 *
 * Returns the index of the requested ID from the control structure array
 */
static int
find_vctrl(int id)
{
	int i;

	if (id < V4L2_CID_BASE)
		return -EDOM;

	for (i = (ARRAY_SIZE(video_control) - 1); i >= 0; i--)
		if (video_control[i].qc.id == id)
			break;
	if (i < 0)
		i = -EINVAL;
	return i;
}

/**
 * camaf_reg_read - Reads a value from a register in DW9716 Coil driver device.
 * @client: Pointer to structure of I2C client.
 * @value: Pointer to u16 for returning value of register to read.
 *
 * Returns zero if successful, or non-zero otherwise.
 **/
static int camaf_reg_read(struct i2c_client *client, u16 *value)
{
	int err;
	struct i2c_msg msg[1];
	unsigned char data[2];

	if (!client->adapter)
		return -ENODEV;

	msg->addr = client->addr;
	msg->flags = I2C_M_RD;
	msg->len = 2;
	msg->buf = data;

	data[0] = 0;
	data[1] = 0;

	err = i2c_transfer(client->adapter, msg, 1);

	if (err >= 0) {
		*value = ((data[0] & 0xFF) << 8) | (data[1]);
		return 0;
	}
	return err;
}

/**
 * camaf_reg_write - Writes a value to a register in DW9716 Coil driver device.
 * @client: Pointer to structure of I2C client.
 * @value: Value of register to write.
 *
 * Returns zero if successful, or non-zero otherwise.
 **/
static int camaf_reg_write(struct i2c_client *client, u16 value)
{
	int err;
	struct i2c_msg msg[1];
	unsigned char data[2];
	int retry = 0;

    if (!client->adapter){
        return -ENODEV;
    }

again:
	msg->addr = client->addr;
	msg->flags = 0;
	msg->len = 2;
	msg->buf = data;

	data[0] = (u8)(value >> 8);
	data[1] = (u8)(value & 0xFF);

    err = i2c_transfer(client->adapter, msg, 1);

    if (err >= 0){
        return 0;
    }

	if (retry <= DW9716_I2C_RETRY_COUNT) {
		dev_dbg(&client->dev, "retry ... %d", retry);
		retry++;
		set_current_state(TASK_UNINTERRUPTIBLE);
		schedule_timeout(msecs_to_jiffies(20));
		goto again;
	}

	return err;
}

/**
 * dw9716_detect - Detects DW9716 Coil driver device.
 * @client: Pointer to structure of I2C client.
 *
 * Returns 0 if successful, -1 if camera is off or if test register value
 * wasn't stored properly, or returned errors from either camaf_reg_write or
 * camaf_reg_read functions.
 **/
static int dw9716_detect(struct i2c_client *client)
{
	int err = 0;
	u16 wposn = 0, rposn = 0;
	u16 posn = 0x05;

	wposn = (CAMAF_DW9716_POWERDN(CAMAF_DW9716_ENABLE) |
						CAMAF_DW9716_DATA(posn));
	printk(" dw9716_detect \n");
	err = camaf_reg_write(client, wposn);
	if (err) {
		printk(KERN_ERR "Unable to write DW9716 \n");
		return err;
	}

	err = camaf_reg_read(client, &rposn);
	if (err) {
		printk(KERN_ERR "Unable to read DW9716 \n");
		return err;
	}

	if (wposn != rposn) {
		printk(KERN_ERR "W/R MISMATCH\n");
		//return -1;
	}

	posn = 0;
	wposn = (CAMAF_DW9716_POWERDN(CAMAF_DW9716_ENABLE) |
						CAMAF_DW9716_DATA(posn));
	err = camaf_reg_write(client, wposn);

	return err;
}

/**
 * dw9716_af_setfocus - Sets the desired focus.
 * @posn: Desired focus position, 0 (far) - 100 (close).
 *
 * Returns 0 on success, -EINVAL if camera is off or focus value is out of
 * bounds, or returned errors from either camaf_reg_write or camaf_reg_read
 * functions.
 **/
int dw9716_af_setfocus(u16 posn)
{
	struct dw9716_device *af_dev = &dw9716;
	struct i2c_client *client = af_dev->i2c_client;
	u16 cur_focus_value = 0;
	u16 reg;
	int ret = -EINVAL;

	if (posn > MAX_FOCUS_POS) {
		printk(KERN_ERR "Bad posn params 0x%x\n", posn);
		return ret;
	}

	if ((af_dev->power_state == V4L2_POWER_OFF) ||
		(af_dev->power_state == V4L2_POWER_STANDBY)) {
		af_dev->current_lens_posn = posn;
		return 0;
	}

	ret = camaf_reg_read(client, &cur_focus_value);

	if (ret) {
		printk(KERN_ERR "Read of current Lens position failed\n");
		return ret;
	}

	if (CAMAF_DW9716_DATA_R(cur_focus_value) == posn) {
		printk(KERN_DEBUG "Device already in requested focal point\n");
		return ret;
	}

	reg = CAMAF_DW9716_POWERDN(CAMAF_DW9716_ENABLE) |
				CAMAF_DW9716_DATA(posn);

	ret = camaf_reg_write(client, reg);

	if (ret)
		printk(KERN_ERR "Setfocus register write failed\n");
	dw9716.current_lens_posn = posn;
	return ret;
}
EXPORT_SYMBOL(dw9716_af_setfocus);

static enum hrtimer_restart dw9716_timer_callback(struct hrtimer *timer)
{
    complete(&dw9716.mstep_completion);

    return HRTIMER_NORESTART;
}

static enum dw9716_drive_mode select_drive_mode(u16 new_pos, u16 cur_pos, u32 time)
{
	u16 offset = abs(new_pos - cur_pos);

	if (time < DW9716_TIME_6_25us * offset) {
		return DW9716_DRIVE_MODE_DIRECT_0;
	} else if (time < DW9716_TIME_12_50us * offset) {
		return DW9716_DRIVE_MODE_8_per_50us;
	} else if (time < DW9716_TIME_25_00us * offset) {
		return DW9716_DRIVE_MODE_4_per_50us;
	} else if (time < DW9716_TIME_50_00us * offset) {
		return DW9716_DRIVE_MODE_2_per_50us;
	} else if (time < DW9716_TIME_100_00us * offset) {
		return DW9716_DRIVE_MODE_2_per_100us;
	} else {
		return DW9716_DRIVE_MODE_2_per_200us;
	}
}

/**
 * dw9716_af_setscript - Sets the desired focus script.
 * @posn: Desired focus position, 0 (far) - 100 (close).
 *
 * Returns 0 on success, -EINVAL if camera is off or focus value is out of
 * bounds, or returned errors from either camaf_reg_write or camaf_reg_read
 * functions.
 **/
int dw9716_af_setscript(s32 script)
{
	int i, ret = -EINVAL;
	u16 cur_pos;
	u16 new_pos;
	struct timespec start_time;
	s32 time_offset;
	s32 time_correction = 0;
	s32 time_to_wait;
	enum dw9716_drive_mode drive_mode;

	struct dw9716_micro_steps_list __user *userp =
		(struct dw9716_micro_steps_list __user*) script;

	struct hrtimer timer;

	if( copy_from_user(&dw9716.mstep_list, userp, sizeof(*userp)) != 0 )
	{
		printk(KERN_ERR "V4L2_CID_FOCUS_SCRIPT - copy_from_user failed\n");
		return ret;
	}

	if ((dw9716.power_state == V4L2_POWER_OFF) ||
		(dw9716.power_state == V4L2_POWER_STANDBY)) {
		return 0;
	}

	hrtimer_init(&timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	timer.function = dw9716_timer_callback;

	cur_pos = dw9716.current_lens_posn;

    if(dw9716.mstep_list.count > DW9716_MAX_STEP_COUNT){
        dw9716.mstep_list.count = DW9716_MAX_STEP_COUNT;
    }

	for (i = 0; i < dw9716.mstep_list.count; i++) {
		new_pos = dw9716.mstep_list.steps[i].lens_pos;

		if (new_pos > MAX_FOCUS_POS) {
			printk(KERN_ERR "Bad posn params 0x%x \n", new_pos);
			return ret;
		}

		time_to_wait = dw9716.mstep_list.steps[i].time_sleep_ns - time_correction;

		if (time_to_wait < 500000) {
			time_to_wait = 500000;
		}

		drive_mode = select_drive_mode(new_pos, cur_pos, time_to_wait);

		ktime_get_ts(&start_time);

		ret = camaf_reg_write(dw9716.i2c_client,
					CAMAF_DW9716_POWERDN(CAMAF_DW9716_ENABLE) |
					CAMAF_DW9716_DATA(new_pos) |
					CAMAF_DW9716_MODE(drive_mode));

		if (time_to_wait > 1000000) {
			init_completion(&dw9716.mstep_completion);
			hrtimer_start(&timer, ktime_set(0, time_to_wait), HRTIMER_MODE_REL);
		}

		if (ret) {
			hrtimer_cancel(&timer);
			return ret;
		}

		cur_pos = new_pos;

		if (time_to_wait > 1000000) {
			wait_for_completion(&dw9716.mstep_completion);
		} else {
			udelay(time_to_wait / 1000);
		}

		ktime_get_ts(&dw9716.mstep_list.steps[i].exec_time);

		time_offset = dw9716.mstep_list.steps[i].exec_time.tv_nsec - start_time.tv_nsec;
		if (time_offset < 0) {
			time_offset = 1000000000 - start_time.tv_nsec + dw9716.mstep_list.steps[i].exec_time.tv_nsec;
		}

		time_correction = time_offset - time_to_wait;

		if (time_correction < 0) {
			time_correction = 0;
		}
	}

	dw9716.current_lens_posn = cur_pos;

	if (copy_to_user(userp, &dw9716.mstep_list, sizeof(*userp)) != 0)
		printk(KERN_ERR "copy_to_user failed\n");

	return 0;
}
EXPORT_SYMBOL(dw9716_af_setscript);

/**
 * dw9716_af_getfocus - Gets the focus value from device.
 * @value: Pointer to u16 variable which will contain the focus value.
 *
 * Returns 0 if successful, -EINVAL if camera is off, or return value of
 * camaf_reg_read if fails.
 **/
int dw9716_af_getfocus(u16 *value)
{
	int ret = -EINVAL;
	u16 posn = 0;

	struct dw9716_device *af_dev = &dw9716;
	struct i2c_client *client = af_dev->i2c_client;

	if ((af_dev->power_state == V4L2_POWER_OFF) ||
		(af_dev->power_state == V4L2_POWER_STANDBY))
		return ret;

	ret = camaf_reg_read(client, &posn);

	if (ret) {
		printk(KERN_ERR "Read of current Lens position failed\n");
		return ret;
	}
	*value = CAMAF_DW9716_DATA_R(posn);
	dw9716.current_lens_posn = CAMAF_DW9716_DATA_R(posn);
	return ret;
}
EXPORT_SYMBOL(dw9716_af_getfocus);

/**
 * ioctl_queryctrl - V4L2 lens interface handler for VIDIOC_QUERYCTRL ioctl
 * @s: pointer to standard V4L2 device structure
 * @qc: standard V4L2 VIDIOC_QUERYCTRL ioctl structure
 *
 * If the requested control is supported, returns the control information
 * from the video_control[] array.  Otherwise, returns -EINVAL if the
 * control is not supported.
 */
static int ioctl_queryctrl(struct v4l2_int_device *s,
				struct v4l2_queryctrl *qc)
{
	int i;

	i = find_vctrl(qc->id);
	if (i == -EINVAL)
		qc->flags = V4L2_CTRL_FLAG_DISABLED;

	if (i < 0)
		return -EINVAL;

	*qc = video_control[i].qc;
	return 0;
}

/**
 * ioctl_g_ctrl - V4L2 DW9716 lens interface handler for VIDIOC_G_CTRL ioctl
 * @s: pointer to standard V4L2 device structure
 * @vc: standard V4L2 VIDIOC_G_CTRL ioctl structure
 *
 * If the requested control is supported, returns the control's current
 * value from the video_control[] array.  Otherwise, returns -EINVAL
 * if the control is not supported.
 */
static int ioctl_g_ctrl(struct v4l2_int_device *s,
			     struct v4l2_control *vc)
{
	struct vcontrol *lvc;
	int i;
	u16 curr_posn;

	i = find_vctrl(vc->id);
	if (i < 0)
		return -EINVAL;
	lvc = &video_control[i];
	switch (vc->id) {
	case  V4L2_CID_FOCUS_ABSOLUTE:
		if (dw9716_af_getfocus(&curr_posn))
			return -EFAULT;
		vc->value = curr_posn;
		lvc->current_value = curr_posn;
		break;
	}

	return 0;
}

/**
 * ioctl_s_ctrl - V4L2 DW9716 lens interface handler for VIDIOC_S_CTRL ioctl
 * @s: pointer to standard V4L2 device structure
 * @vc: standard V4L2 VIDIOC_S_CTRL ioctl structure
 *
 * If the requested control is supported, sets the control's current
 * value in HW (and updates the video_control[] array).  Otherwise,
 * returns -EINVAL if the control is not supported.
 */
static int ioctl_s_ctrl(struct v4l2_int_device *s,
			     struct v4l2_control *vc)
{
	int retval = -EINVAL;
	int i;
	struct vcontrol *lvc;

	i = find_vctrl(vc->id);
	if (i < 0)
		return -EINVAL;
	lvc = &video_control[i];


	switch (vc->id) {
	case V4L2_CID_FOCUS_ABSOLUTE:
		retval = dw9716_af_setfocus(vc->value);
		if (!retval)
			lvc->current_value = vc->value;
		break;

    case V4L2_CID_FOCUS_SCRIPT:
		retval = dw9716_af_setscript(vc->value);
		if (!retval)
			lvc->current_value = vc->value;
		break;
	}

	return retval;
}

/**
 * ioctl_g_priv - V4L2 sensor interface handler for vidioc_int_g_priv_num
 * @s: pointer to standard V4L2 device structure
 * @p: void pointer to hold sensor's private data address
 *
 * Returns device's (sensor's) private data area address in p parameter
 */
static int ioctl_g_priv(struct v4l2_int_device *s, void *p)
{
	struct dw9716_device *lens = s->priv;

	return lens->pdata->priv_data_set(p);

}


static int dw9716_power_on(struct dw9716_device *lens)
{
	struct i2c_client *c = lens->i2c_client;
	int rval = -EINVAL;
	printk("dw9716_power_on \n");
	if (lens->pdata->power_set)
		rval = lens->pdata->power_set(V4L2_POWER_ON);

	if (rval < 0) {
		v4l_err(c, "Unable to set the power state ON\n");
		return rval;
	}

	mdelay(50);
	return 0;
}

static int dw9716_power_off(struct dw9716_device *lens)
{
	struct i2c_client *c = lens->i2c_client;
	int rval = -EINVAL;
	printk("dw9716_power_off \n");
	if (lens->pdata->power_set)
		rval = lens->pdata->power_set(V4L2_POWER_OFF);

	if (rval < 0) {
		v4l_err(c, "Unable to set the power state OFF\n");
		return rval;
	}

	return 0;
}

static int dw9716_power_standby(struct dw9716_device *lens)
{
	struct i2c_client *c = lens->i2c_client;
	int rval = -EINVAL;
	printk("dw9716_power_standby \n");
	if (lens->pdata->power_set)
		rval = lens->pdata->power_set(V4L2_POWER_STANDBY);

	if (rval < 0) {
		v4l_err(c, "Unable to set the power state STANDBY\n");
		return rval;
	}

	return 0;
}


/**
 * ioctl_s_power - V4L2 sensor interface handler for vidioc_int_s_power_num
 * @s: pointer to standard V4L2 device structure
 * @on: power state to which device is to be set
 *
 * Sets devices power state to requrested state, if possible.
 */
static int ioctl_s_power(struct v4l2_int_device *s, enum v4l2_power on)
{
	struct dw9716_device *lens = s->priv;
	struct i2c_client *c = lens->i2c_client;
	int rval = 0;

	switch (on) {
	case V4L2_POWER_ON:
		rval = dw9716_power_on(lens);
		if (rval)
			goto error;

		/*if (lens->power_state == V4L2_POWER_STANDBY) {
			rval = dw9716_reginit(c);
			if (rval < 0) {
				v4l_err(c, "Unable to initialize " DW9716_NAME
					" lens HW\n");
				lens->state = LENS_NOT_DETECTED;
				return rval;
			}
		}*/
		break;
	case V4L2_POWER_OFF:
		rval = dw9716_power_off(lens);
		break;
	case V4L2_POWER_STANDBY:
		rval = dw9716_power_standby(lens);
		break;
	}

	lens->power_state = on;
error:
	return rval;
}

/**
 * ioctl_dev_init - V4L2 sensor interface handler for vidioc_int_dev_init_num
 * @s: pointer to standard V4L2 device structure
 *
 * Initialise the device when slave attaches to the master.  Returns 0 if
 * dw9716 device could be found, otherwise returns appropriate error.
 */
static int ioctl_dev_init(struct v4l2_int_device *s)
{
	struct dw9716_device *lens = s->priv;
	struct i2c_client *c = lens->i2c_client;
	int err;
    
    if (gpio_request(DW9716_VCM_ENABLE, "dw9716_ps") != 0)
        return -EIO;


	err = dw9716_power_on(lens);
	if (err)
		return -ENODEV;

	err = dw9716_detect(c);
	if (err < 0) {
		v4l_err(c, "Unable to detect " DW9716_NAME
			" lens HW\n");
		lens->state = LENS_NOT_DETECTED;
        subpm_set_output(LDO5,0);
        subpm_output_enable();
            
        gpio_set_value(DW9716_VCM_ENABLE, 0);
		return err;
	}
	lens->state = LENS_DETECTED;
	pr_info(DW9716_NAME " Lens HW detected\n");

	/*err = dw9716_reginit(c);
	if (err < 0) {
		v4l_err(c, "Unable to initialize " DW9716_NAME
			" lens HW\n");
		lens->state = LENS_NOT_DETECTED;
		return err;
	}*/

//	err = dw9716_power_off(lens);
//	if (err)
//		return -ENODEV;
		
	subpm_set_output(LDO5,0);
	subpm_output_enable();
	printk( "dw9716_lens_power_set(OFF)\n");
    gpio_set_value(DW9716_VCM_ENABLE, 0);
	
	return 0;
}

static struct v4l2_int_ioctl_desc dw9716_ioctl_desc[] = {
	{ .num = vidioc_int_s_power_num,
	  .func = (v4l2_int_ioctl_func *)ioctl_s_power },
	{ .num = vidioc_int_dev_init_num,
	  .func = (v4l2_int_ioctl_func *)ioctl_dev_init},
	{ .num = vidioc_int_g_priv_num,
	  .func = (v4l2_int_ioctl_func *)ioctl_g_priv },
	{ .num = vidioc_int_queryctrl_num,
	  .func = (v4l2_int_ioctl_func *)ioctl_queryctrl },
	{ .num = vidioc_int_g_ctrl_num,
	  .func = (v4l2_int_ioctl_func *)ioctl_g_ctrl },
	{ .num = vidioc_int_s_ctrl_num,
	  .func = (v4l2_int_ioctl_func *)ioctl_s_ctrl },
};

static struct v4l2_int_slave dw9716_slave = {
	.ioctls = dw9716_ioctl_desc,
	.num_ioctls = ARRAY_SIZE(dw9716_ioctl_desc),
};

static struct v4l2_int_device dw9716_int_device = {
	.module = THIS_MODULE,
	.name = DRIVER_NAME,
	.priv = &dw9716,
	.type = v4l2_int_type_slave,
	.u = {
		.slave = &dw9716_slave,
	},
};


/**
 * dw9716_probe - Probes the driver for valid I2C attachment.
 * @client: Pointer to structure of I2C client.
 *
 * Returns 0 if successful, or -EBUSY if unable to get client attached data.
 **/
static int __devinit dw9716_probe(struct i2c_client *client,
		const struct i2c_device_id *id)
{
	struct dw9716_device *lens = &dw9716;
	int err;

	if (i2c_get_clientdata(client)) {
		printk(KERN_ERR " DTA BUSY %s\n", client->name);
		return -EBUSY;
	}

	lens->pdata = client->dev.platform_data;

	if (!lens->pdata) {
		dev_err(&client->dev, "no platform data?\n");
		return -ENODEV;
	}

	lens->v4l2_int_device = &dw9716_int_device;

	lens->i2c_client = client;
	i2c_set_clientdata(client, lens);

	err = v4l2_int_device_register(lens->v4l2_int_device);
	if (err) {
		printk(KERN_ERR "Failed to Register " DRIVER_NAME
						" as V4L2 device.\n");
		i2c_set_clientdata(client, NULL);
	} else
		printk(KERN_ERR "Registered " DRIVER_NAME
						" as V4L2 device.\n");

	return 0;
}


/**
 * dw9716_init - Module initialisation.
 *
 * Returns 0 if successful, or -EINVAL if device couldn't be initialized, or
 * added as a character device.
 **/
static int __init dw9716_init(void)
{
	int err;

	err = i2c_add_driver(&dw9716_i2c_driver);
	if (err)
		goto fail;
	printk(KERN_ERR "Registered " DRIVER_NAME " as i2c device.\n");

	return err;
fail:
	printk(KERN_ERR "Failed to register " DRIVER_NAME ".\n");
	return err;
}

late_initcall(dw9716_init);
/**
 * dw9716_cleanup - Module cleanup.
 **/
static void __exit dw9716_cleanup(void)
{
	i2c_del_driver(&dw9716_i2c_driver);
}
module_exit(dw9716_cleanup);

MODULE_AUTHOR("Texas Instruments");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("DW9716 LENS driver");
MODULE_DESCRIPTION("DW9716 LENS driver");
