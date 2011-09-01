/*
 * Hub Vibrator driver (SM100 IC, OMAP3430)
 *
 * Copyright (C) 2009 LGE, Inc.
 * 
 * Author: Jinkyu Choi <jinkyu@lge.com>
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

#define USE_SUBPM

#include <linux/platform_device.h>
#include <mach/gpio.h>
#include <linux/delay.h>
#include "../staging/android/timed_output.h"
#include <linux/hrtimer.h>
#include <plat/dmtimer.h>
#include <linux/regulator/consumer.h>
#ifdef USE_SUBPM
#if defined(CONFIG_REGULATOR_LP8720)
#include <linux/regulator/lp8720.h>
#endif
#endif

#define HUB_VIBRATOR_DEBUG 0
 #if HUB_VIBRATOR_DEBUG
 #define DEBUG_MSG(args...)  printk(args)
 #else
 #define DEBUG_MSG(args...)
 #endif


#define HUB_VIBE_GPIO_EN 			57
#define HUB_VIBE_PWM				56
#define HUB_VIBE_GPTIMER_NUM 		10

#define PWM_DUTY_MAX     1158 /* 22.43 kHz */
#define PLTR_VALUE		(0xFFFFFFFF - PWM_DUTY_MAX)
#define PWM_DUTY_HALF	(0xFFFFFFFF - (PWM_DUTY_MAX >> 1))
struct omap_dm_timer *omap_vibrator_timer = NULL;

#ifdef USE_SUBPM
#else
static struct regulator *vibe_regulator = NULL;
#endif

struct timed_vibrator_data {
	struct timed_output_dev dev;
	struct hrtimer timer;
	spinlock_t lock;
	unsigned 	gpio;
	int 		max_timeout;
	u8 		active_low;
};

static void hub_vibrator_gpio_enable (int enable)
{
	DEBUG_MSG("[!] %s() enable : %d.\n", __func__, enable);

	if (enable) {
		gpio_set_value(HUB_VIBE_GPIO_EN, 1);
//		gpio_set_value(HUB_VIBE_PWM, 1);
	} else {
		gpio_set_value(HUB_VIBE_GPIO_EN, 0);
//		gpio_set_value(HUB_VIBE_PWM, 0);
	}
}

static inline void hub_vibrator_LDO_enable(int val)
{
	DEBUG_MSG("[!] %s() start.\n", __func__);

#ifdef USE_SUBPM
	subpm_set_output(LDO2, val);
	subpm_output_enable();
#else
	if (val == 1){
		regulator_enable(vibe_regulator);
	}else{
		regulator_disable(vibe_regulator);
	}
#endif
}

static int hub_vibrator_intialize(void)
{
	/* Disable amp */
	hub_vibrator_gpio_enable(0);

	/* Select clock */
	omap_dm_timer_enable(omap_vibrator_timer);
	omap_dm_timer_set_source(omap_vibrator_timer, OMAP_TIMER_SRC_SYS_CLK);

	/* set a period */
	omap_dm_timer_set_load(omap_vibrator_timer, 1, PLTR_VALUE);

	/* set a duty */
	omap_dm_timer_set_match(omap_vibrator_timer, 1, PWM_DUTY_HALF);
	omap_dm_timer_set_pwm(omap_vibrator_timer, 0, 1, OMAP_TIMER_TRIGGER_OVERFLOW_AND_COMPARE);
	
	return 0;
}

static int hub_vibrator_force_set(int nForce)
{

	unsigned int nTmp;

	DEBUG_MSG("[!] %s() - nForce : %d.\n", __func__, nForce);


	/* Check the Force value with Max and Min force value */
	if (nForce > 127) nForce = 127;
	if (nForce < -127) nForce = -127;
	
	//woochan.seo, 20101027, INSERT, for reduce the power of vibration. I can't use the 'cast' sentence.
	//nForce *= 0.85;		//ERR. I cannot use the 'cast' sentence.
	
#if 0	
	if (nForce <= 127 && nForce > 40) 	 nForce -= 40;
	else if (nForce >= -127 && nForce < -40) nForce += 40;
	else 	
	 nForce = 0;
#endif
	//woochan.seo, 20101027, INSERT, for reduce the power of vibration. I can't use the 'cast' sentence.
	
	if (nForce == 0) {
		hub_vibrator_gpio_enable(0);
		omap_dm_timer_stop(omap_vibrator_timer);		
	} else {
		hub_vibrator_gpio_enable(1);
		nTmp = 0xFFFFFFF7 - (((127 - nForce) * 9) >> 1);
		omap_dm_timer_set_match(omap_vibrator_timer, 1, nTmp);
		omap_dm_timer_start(omap_vibrator_timer);		
	}
	return 0;
}

static enum hrtimer_restart vibrator_timer_func(struct hrtimer *timer)
{
	DEBUG_MSG("[!] %s() start.\n", __func__);

	hub_vibrator_force_set(0);
	return HRTIMER_NORESTART;
}

static int vibrator_get_time(struct timed_output_dev *dev)
{
	struct timed_vibrator_data *data = container_of(dev, struct timed_vibrator_data, dev);
	DEBUG_MSG("[!] %s() start.\n", __func__);

	if (hrtimer_active(&data->timer)) {
		ktime_t r = hrtimer_get_remaining(&data->timer);
		return r.tv.sec * 1000 + r.tv.nsec / 1000000;
	} else
		return 0;
}

static void vibrator_enable(struct timed_output_dev *dev, int value)
{
	struct timed_vibrator_data *data = container_of(dev, struct timed_vibrator_data, dev);
	static atomic_t vibrator_work = ATOMIC_INIT(0);
	unsigned long flags;
	
	DEBUG_MSG("[!] %s() - value : %d.\n", __func__, value);

	if (atomic_inc_return(&vibrator_work) != 1) {
		atomic_dec(&vibrator_work);
		return;
	}
	
	spin_lock_irqsave(&data->lock, flags);
	hrtimer_cancel(&data->timer);
	
	if (value > 0) {
		if (value > data->max_timeout)
			value = data->max_timeout;
		hub_vibrator_force_set(127); /* set Max Gain. */
		hrtimer_start(&data->timer, ktime_set(value / 1000, (value % 1000) * 1000000), HRTIMER_MODE_REL);
	} else {
		hub_vibrator_force_set(0);
	}

	spin_unlock_irqrestore(&data->lock, flags);

	atomic_dec(&vibrator_work);
}

/* Interface for Android Platform */
struct timed_vibrator_data hub_vibrator_data = {
	.dev.name = "vibrator",
	.dev.enable = vibrator_enable,
	.dev.get_time = vibrator_get_time,
	.max_timeout = 10000, /* max time for vibrator enable 10 sec. */
};

static int vibrator_probe(struct platform_device *dev)
{
	int ret = 0;

#ifdef USE_SUBPM
#else
	vibe_regulator = regulator_get(&dev->dev, "vaux1");
	if (vibe_regulator == NULL) {
		printk("LGE: vaux1 regulator get fail\n");
		goto err1;
	}
#endif

	ret = gpio_request(HUB_VIBE_GPIO_EN, "Hub Vibrator Enable");
	if (ret < 0) {
		printk(KERN_ERR "%s: Failed to request GPIO_%d for vibrator\n", __func__, HUB_VIBE_GPIO_EN);
		goto err1;
	}
	gpio_direction_output(HUB_VIBE_GPIO_EN, 0);
//	gpio_direction_output(HUB_VIBE_PWM, 0);

	omap_vibrator_timer = omap_dm_timer_request_specific(HUB_VIBE_GPTIMER_NUM);
	if (omap_vibrator_timer == NULL) {
		printk(KERN_ERR "%s: failed to request omap pwm timer.\n", __func__);
		ret = -ENODEV;
		goto err2;
	}
	omap_dm_timer_disable(omap_vibrator_timer);	

	hub_vibrator_intialize();

	hrtimer_init(&hub_vibrator_data.timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	hub_vibrator_data.timer.function = vibrator_timer_func;
	spin_lock_init(&hub_vibrator_data.lock);

	ret = timed_output_dev_register(&hub_vibrator_data.dev);
	if (ret < 0) {
		timed_output_dev_unregister(&hub_vibrator_data.dev);
		ret = -ENODEV;
		goto err3;
	}
	hub_vibrator_LDO_enable(1); /* Vibrator LDO Power On */		

	printk("LGE: Hub Vibrator Initialization was done\n");
	
	return 0;

err3:
	omap_dm_timer_free(omap_vibrator_timer);
	omap_vibrator_timer = NULL;
err2:
	gpio_free(HUB_VIBE_GPIO_EN);
err1:
	return ret;
}

static int vibrator_remove(struct platform_device *dev)
{
	hub_vibrator_gpio_enable(0);
	omap_dm_timer_stop(omap_vibrator_timer);
	omap_dm_timer_disable(omap_vibrator_timer);
	omap_dm_timer_free(omap_vibrator_timer);	
	timed_output_dev_unregister(&hub_vibrator_data.dev);
	hub_vibrator_LDO_enable(0); /* Vibrator LDO Power off */
	omap_vibrator_timer = NULL;
	gpio_free(HUB_VIBE_GPIO_EN);
	
	return 0;
}

#ifdef CONFIG_PM
static int vibrator_suspend(struct platform_device *pdev, pm_message_t state)
{
	printk("LGE: Hub Vibrator Driver Suspend\n");
	hub_vibrator_LDO_enable(0);
	hub_vibrator_force_set(0);
	omap_dm_timer_stop(omap_vibrator_timer);
	omap_dm_timer_disable(omap_vibrator_timer);
	return 0;
}

static int vibrator_resume(struct platform_device *pdev)
{
	printk("LGE: Hub Vibrator Driver Resume\n");
	hub_vibrator_LDO_enable(1);
	hub_vibrator_intialize();
	return 0;
}
#endif

static void vibrator_shutdown(struct platform_device *pdev)
{
	hub_vibrator_force_set(0);
	hub_vibrator_LDO_enable(0);	
}

static struct platform_driver vibrator_driver = {
	.probe = vibrator_probe,
	.remove = vibrator_remove,
	.shutdown = vibrator_shutdown,
#ifdef CONFIG_PM
	.suspend = vibrator_suspend,
	.resume = vibrator_resume,
#endif
	.driver = {
		.name = "hub_vibrator",
	},
};

static int __init hub_vibrator_init(void)
{
	printk("LGE: Hub Vibrator Driver Init\n");
	return platform_driver_register(&vibrator_driver);
}

static void __exit hub_vibrator_exit(void)
{
	printk("LGE: Hub Vibrator Driver Exit\n");
	platform_driver_unregister(&vibrator_driver);
} 

module_init(hub_vibrator_init);
module_exit(hub_vibrator_exit);

MODULE_AUTHOR("LG Electronics");
MODULE_DESCRIPTION("heaven Vibrator Driver");
MODULE_LICENSE("GPL");


