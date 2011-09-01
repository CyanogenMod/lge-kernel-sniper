/*
 * drivers/media/video/rt8515.c
 *
 * rt8515 Flash driver
 *
 * Copyright (C) 2009 Texas Instruments.
 *
 * Leverage mt9p012.c
 *
 * This file is licensed under the terms of the GNU General Public License
 * version 2. This program is licensed "as is" without any warranty of any
 * kind, whether express or implied.
 */

#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/timer.h>
#include "rt8515.h"

static struct timer_list flash_timer;

// rt8515_private used in the init  spinlock
// and later use it for the s2c transaction
static struct rt8515 rt8515_private;

static int rt8515_delay_strobe(struct rt8515 *flash)
{
	int ret;
	u32 timeout = flash->flash_timeout;
	del_timer_sync(&flash_timer);

	//we're starting the SW watchdog for time defined by the timeout
	// after expiring it will go to flash_off
	setup_timer(&flash_timer, flash->pdata->flash_off, 1);
	ret = mod_timer(&flash_timer, jiffies + usecs_to_jiffies(timeout));
	if (ret)
		printk(KERN_ERR "rt8515 mod_timer() returned %i!\n", ret);
	return ret;
}
u8 inline turn_percent_to_s2c(u8 light_intensity_percentage){
		u8 ret;

		ret = RT8515_TORCH_INT_0_PERCENT;
		if(light_intensity_percentage >= 20)
			ret = RT8515_TORCH_INT_20_PERCENT;
		if(light_intensity_percentage > 22)
			ret = RT8515_TORCH_INT_22_4_PERCENT;
		if(light_intensity_percentage > 25)
			ret = RT8515_TORCH_INT_25_1_PERCENT;
		if(light_intensity_percentage > 28)	
			ret = RT8515_TORCH_INT_28_2_PERCENT;
		if(light_intensity_percentage > 31)	
			ret = RT8515_TORCH_INT_31_6_PERCENT;
		if(light_intensity_percentage > 35)	
			ret = RT8515_TORCH_INT_35_5_PERCENT;
		if(light_intensity_percentage > 39)
			ret = RT8515_TORCH_INT_39_8_PERCENT;
		if(light_intensity_percentage > 44)
			ret = RT8515_TORCH_INT_44_7_PERCENT;
		if(light_intensity_percentage >= 50)
			ret = RT8515_TORCH_INT_50_PERCENT;
		if(light_intensity_percentage >= 56)
			ret = RT8515_TORCH_INT_56_PERCENT;
		if(light_intensity_percentage >= 63)
			ret = RT8515_TORCH_INT_63_PERCENT;
		if(light_intensity_percentage >= 71)
			ret = RT8515_TORCH_INT_71_PERCENT;
		if(light_intensity_percentage >= 79)
			ret = RT8515_TORCH_INT_79_PERCENT;
		if(light_intensity_percentage >= 89)
			ret = RT8515_TORCH_INT_89_PERCENT;
		if(light_intensity_percentage >= 100)
			ret = RT8515_TORCH_INT_100_PERCENT;

		// torch intensity. convert it from the enum to the datasheet table value
		//reverse the values so 15 will represent 0 percent and 0 will represent 100 percent
		ret <<= 4;
		ret = ~ret; 
		ret >>= 4;		

		// make 0 to sent 1 over the S2I, 1 to sent 2 and so on.
		ret += 1;	
		return ret;
		
}

static int rt8515_apply(struct v4l2_int_device *s, int is_flash)
{
	struct rt8515 *flash = s->priv;
	u8 s2c_data;
	unsigned long flags;

	if(is_flash){ // full intensity flash the flash 
		flash->pdata->flash_on();
		return rt8515_delay_strobe(flash); // with full flash we need SW watchedog defined by the timeout being sent
	} else {
		s2c_data = turn_percent_to_s2c(flash->torch_intensity);

		// the s2c transfer have to be done under 75us
		// otherwise the sent 16bit data won't be read as 16bit
		spin_lock_irqsave(&rt8515_private.lock, flags);
		flash->pdata->torch_on(s2c_data);	
		spin_unlock_irqrestore(&rt8515_private.lock, flags);
		
		return 0; 
	}
}

#define CTRL_FLASH_START		0
#define CTRL_FLASH_INTENSITY		1
#define CTRL_TORCH_INTENSITY		2
#define CTRL_FLASH_TIMEOUT		3

static struct v4l2_queryctrl rt8515_ctrls[] = {

/*  Not used. If torch_intensity != 0 then start Torch, if torch_intensity == 0 start Flash*/

	{
		.id		= V4L2_CID_FLASH_STROBE,
		.type 		= V4L2_CTRL_TYPE_BOOLEAN,
		.name		= "Flash start [on/off]",
		.minimum 	= 0,
		.maximum 	= 1,
		.step 		= 1,
		.default_value 	= 0
	},
	{
		.id		= V4L2_CID_FLASH_INTENSITY,
		.type 		= V4L2_CTRL_TYPE_INTEGER,
		.name		= "Flash intensity [%]",
		.minimum 	= 0,
		.maximum 	= 100,
		.step 		= 100,
		.default_value 	= 100
	},	
	{
		.id		= V4L2_CID_TORCH_INTENSITY,
		.type		= V4L2_CTRL_TYPE_INTEGER,
		.name		= "Torch intensity [%]",
		.minimum	= 0,
		.maximum	= 100, // in percents
		.step		= 1,
		.default_value	= 100,
		.flags		= V4L2_CTRL_FLAG_SLIDER,
	},
	{
		.id		= V4L2_CID_FLASH_TIMEOUT,
		.type		= V4L2_CTRL_TYPE_INTEGER,
		.name		= "Flash timeout [us]",
		.minimum	= 1,
		.maximum	= 1000000,
		.flags		= V4L2_CTRL_FLAG_SLIDER,
	},

};

/**
 * find_vctrl - Finds the requested ID in the video control structure array
 * @id: ID of control to search the video control array for
 *
 * Returns the index of the requested ID from the control structure array
 */
static int find_vctrl(int id)
{
	int i;

	if (id < V4L2_CID_BASE)
		return -EDOM;

	for (i = (ARRAY_SIZE(rt8515_ctrls) - 1); i >= 0; i--) {
		if (rt8515_ctrls[i].id == id)
			return i;
	}
	return -EINVAL;
}

/**
 * rt8515_ioctl_queryctrl - V4L2 flash interface handler for VIDIOC_QUERYCTRL ioctl
 * @s: pointer to standard V4L2 device structure
 * @qc: standard V4L2 VIDIOC_QUERYCTRL ioctl structure
 *
 * If the requested control is supported, returns the control information
 * from the video_control[] array.  Otherwise, returns -EINVAL if the
 * control is not supported.
 */
static int rt8515_ioctl_queryctrl(struct v4l2_int_device *s,
					struct v4l2_queryctrl *qc)
{
	int i;

	i = find_vctrl(qc->id);
	if (i == -EINVAL)
		qc->flags = V4L2_CTRL_FLAG_DISABLED;

	if (i < 0)
		return -EINVAL;
	*qc = rt8515_ctrls[i];
	return 0;
}

/**
 * rt8515_ioctl_g_ctrl - V4L2 rt8515 flash interface handler for VIDIOC_G_CTRL ioctl
 * @s: pointer to standard V4L2 device structure
 * @vc: standard V4L2 VIDIOC_G_CTRL ioctl structure
 *
 * If the requested control is supported, returns the control's current
 * value from the video_control[] array.  Otherwise, returns -EINVAL
 * if the control is not supported.
 */
static int rt8515_ioctl_g_ctrl(struct v4l2_int_device *s,
			struct v4l2_control *vc)
{
	struct rt8515 *flash = s->priv;
	u8 temp;
	
	temp = flash->pdata->update_hw(s);

	// we take the hardware status from the pdata 
	// it's useful if timeout was asserted
	
	flash->turned_on = (temp & RT8515_HW_FLASH_IS_ON);

	switch (vc->id) {
	case V4L2_CID_FLASH_STROBE:
		vc->value = flash->turned_on;
		break;
	case V4L2_CID_FLASH_INTENSITY:
		vc->value = flash->flash_intensity;
		break;		
	case V4L2_CID_TORCH_INTENSITY:
		vc->value = flash->torch_intensity;
		break;
	case V4L2_CID_FLASH_TIMEOUT:
		vc->value = flash->flash_timeout;
		break;

		
	default:
		return -EINVAL;
	}
	return 0;
}

/**
 * ioctl_s_ctrl - V4L2 rt8515 flash interface handler for VIDIOC_S_CTRL ioctl
 * @s: pointer to standard V4L2 device structure
 * @vc: standard V4L2 VIDIOC_S_CTRL ioctl structure
 *
 * If the requested control is supported, sets the control's current
 * value in HW (and updates the video_control[] array).  Otherwise,
 * returns -EINVAL if the control is not supported.
 */
static int rt8515_ioctl_s_ctrl(struct v4l2_int_device *s,
				struct v4l2_control *vc)
{
	struct rt8515 *flash = s->priv;
	unsigned long dummy; // used for flash_off, needed by the timer function on a first place
	int is_flash = -1;

	switch (vc->id) {
	case V4L2_CID_FLASH_STROBE:
		flash->turned_on = vc->value;
		is_flash = 1;
		break;
	case V4L2_CID_FLASH_TIMEOUT:
		flash->flash_timeout = vc->value;
		break;
	case V4L2_CID_FLASH_INTENSITY:
		flash->flash_intensity = vc->value;
		break;		
	case V4L2_CID_TORCH_INTENSITY:
		flash->torch_intensity = vc->value;
		is_flash = 0;
		break;
	default:
		return -EINVAL;
	}

	if (is_flash >= 0) {
		if(vc->value){
			return rt8515_apply(s, is_flash);

		}else{
			del_timer_sync(&flash_timer);		 
			flash->pdata->flash_off(dummy);
	}

	}

	return 0;
}

/**
 * rt8515_init_controls - Setting default values for intensity and flash timeout.
 * @s: pointer to standard V4L2 device structure
 */
static void rt8515_init_controls(struct v4l2_int_device *s)
{
	struct rt8515 *flash = s->priv;
	
	flash->turned_on = rt8515_ctrls
			[CTRL_FLASH_START].default_value;
	flash->flash_intensity = rt8515_ctrls
			[CTRL_FLASH_INTENSITY].default_value;
	flash->torch_intensity = rt8515_ctrls
			[CTRL_TORCH_INTENSITY].default_value;	
	flash->flash_timeout = rt8515_ctrls
			[CTRL_FLASH_TIMEOUT].default_value;	
}

/**
 * rt8515_ioctl_s_power - V4L2 flash interface handler for vidioc_int_s_power_num
 * @s: pointer to standard V4L2 device structure
 * @state: power state to which device is to be set
 *
 * Sets devices power state to requrested state, if possible.
 */
static int rt8515_ioctl_s_power(struct v4l2_int_device *s,
				 enum v4l2_power state)
{
	struct rt8515 *flash = s->priv;

	switch (state) {
	case V4L2_POWER_ON:
		if (!flash->dev_init_done) {
			flash->dev_init_done = true;
			rt8515_init_controls(s);
		}
		flash->power = V4L2_POWER_ON;
		break;

	case V4L2_POWER_OFF:
	case V4L2_POWER_STANDBY:
		flash->power = V4L2_POWER_OFF;
		break;
	}
	return 0;

}

/**
 * rt8515_ioctl_g_priv - V4L2 sensor interface handler for vidioc_int_g_priv_num
 * @s: pointer to standard V4L2 device structure
 * @p: void pointer to hold flash's private data address
 *
 * Returns device's (flash's) private data area address in p parameter
 */
static int rt8515_ioctl_g_priv(struct v4l2_int_device *s, void *p)
{
	struct rt8515 *flash = s->priv;
	return flash->pdata->priv_data_set(p);

}

static int rt8515_ioctl_dev_init(struct v4l2_int_device *s)
{
	return 0;
}


static struct v4l2_int_ioctl_desc rt8515_ioctl_desc[] = {
	{ .num = vidioc_int_s_power_num,
	  .func = (v4l2_int_ioctl_func *)rt8515_ioctl_s_power },
	{ .num = vidioc_int_dev_init_num, 
	  .func = (v4l2_int_ioctl_func *)rt8515_ioctl_dev_init},
	{ .num = vidioc_int_g_priv_num,
	  .func = (v4l2_int_ioctl_func *)rt8515_ioctl_g_priv },
	{ .num = vidioc_int_queryctrl_num,
	  .func = (v4l2_int_ioctl_func *)rt8515_ioctl_queryctrl },
	{ .num = vidioc_int_g_ctrl_num,
	  .func = (v4l2_int_ioctl_func *)rt8515_ioctl_g_ctrl },
	{ .num = vidioc_int_s_ctrl_num,
	  .func = (v4l2_int_ioctl_func *)rt8515_ioctl_s_ctrl },
};


static struct v4l2_int_slave rt8515_slave = {
	.ioctls = rt8515_ioctl_desc,
	.num_ioctls = ARRAY_SIZE(rt8515_ioctl_desc),
};

static struct rt8515 rt8515_dev;

static struct v4l2_int_device rt8515_int_device = {
	.module = THIS_MODULE,
	.name = RT8515_NAME,
	.priv = &rt8515_dev,
	.type = v4l2_int_type_slave,
	.u = {
		.slave = &rt8515_slave,
	},
};

static int rt8515_probe(struct platform_device *pdev)
{
	struct rt8515_platform_data *pdata = pdev->dev.platform_data;

	if (!pdata) {
		printk(KERN_ERR "rt8515 platform data not supplied\n");
		return -ENOENT;
	}

	rt8515_dev.pdata = pdata;

	if (v4l2_int_device_register(&rt8515_int_device)) {
		printk(KERN_ERR "Could not register "
					" rt8515 as v4l2_int_device\n");
		return -ENOENT;
	}

	printk(KERN_ERR "rt8515_probe success\n");
	return pdata->init();
}

static int rt8515_remove(struct platform_device *pdev)
{
	struct rt8515_platform_data *pdata = pdev->dev.platform_data;
	int ret = 0;

	ret = del_timer(&flash_timer);
	if (ret)
		printk(KERN_ERR "flash_timer is still in use!\n");

	return pdata->exit();
}

static struct platform_driver rt8515_driver = {
	.probe	 = rt8515_probe,
	.remove	 = rt8515_remove,
	.driver	 = {
		.name = "rt8515",
		.owner = THIS_MODULE,
	},
};

/**
 * rt8515_init - Module initialisation.
 *
 * Returns 0 if successful, or -ENODEV if device couldn't be initialized, or
 * added as a character device.
 **/
static int __init rt8515_init(void)
{
	int ret;

	ret = platform_driver_register(&rt8515_driver);
	if (ret)
		return ret;

	printk(KERN_ERR "Registered rt8515_init as i2c device.\n");
	spin_lock_init(&rt8515_private.lock);	   

	return 0;
}

/**
 * rt8515_exit - Module cleanup.
 **/
static void __exit rt8515_exit(void)
{

	v4l2_int_device_unregister(&rt8515_int_device);

	platform_driver_unregister(&rt8515_driver);
}

late_initcall(rt8515_init);
module_exit(rt8515_exit);

MODULE_DESCRIPTION("RT8515 flash LED driver");
MODULE_LICENSE("GPL");
