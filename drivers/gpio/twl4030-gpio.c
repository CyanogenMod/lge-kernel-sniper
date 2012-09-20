/*
 * twl4030_gpio.c -- access to GPIOs on TWL4030/TPS659x0 chips
 *
 * Copyright (C) 2006-2007 Texas Instruments, Inc.
 * Copyright (C) 2006 MontaVista Software, Inc.
 *
 * Code re-arranged and cleaned up by:
 *	Syed Mohammed Khasim <x0khasim@ti.com>
 *
 * Initial Code:
 *	Andy Lowe / Nishanth Menon
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/kthread.h>
#include <linux/irq.h>
#include <linux/gpio.h>
#include <linux/platform_device.h>

//--[[ LGE_UBIQUIX_MODIFIED_START : shyun@ubiquix.com [2011.07.13] - For kzalloc (Justin feature)
#if defined(CONFIG_PRODUCT_LGE_LU6800)
#include <linux/slab.h>
#endif
//--]] LGE_UBIQUIX_MODIFIED_END : shyun@ubiquix.com [2011.07.13]- For kzalloc (Justin feature)

#include <linux/i2c/twl.h>

//--[[ LGE_UBIQUIX_MODIFIED_START : shyun@ubiquix.com [2011.07.13] - Justin Feature is added from justin_froyo #2.
#if defined(CONFIG_PRODUCT_LGE_LU6800)
// LGE_JUSTIN_S 201115 kyoungmo.kang@lge.com, justin test ver
#if 1 
#define JUSTIN_LEDOFF_TEST
#endif 
// LGE_JUSTIN_E 201115 kyoungmo.kang@lge.com, justin test ver


// LGE_JUSTIN_S 201114 kyoungmo.kang@lge.com, KEYLED off 5 seconds
#if ( defined(CONFIG_PRODUCT_LGE_LU6800) ) 
#include <linux/hrtimer.h>

#define LED_A 0
#define LED_B 1
#define LED_ON 0
#define LED_OFF 1
#endif 
// LGE_JUSTIN_E 201114 kyoungmo.kang@lge.com, KEYLED off 5 seconds


// LGE_B_DOM_S 20101215 nttaejun.cho@lge.com, Touch key Led on/off
#include <linux/earlysuspend.h>
#include <linux/delay.h>
#endif
//--]] LGE_UBIQUIX_MODIFIED_END : shyun@ubiquix.com [2011.07.13]- Justin Feature is added from justin_froyo #2.

/*
 * The GPIO "subchip" supports 18 GPIOs which can be configured as
 * inputs or outputs, with pullups or pulldowns on each pin.  Each
 * GPIO can trigger interrupts on either or both edges.
 *
 * GPIO interrupts can be fed to either of two IRQ lines; this is
 * intended to support multiple hosts.
 *
 * There are also two LED pins used sometimes as output-only GPIOs.
 */


static struct gpio_chip twl_gpiochip;
static int twl4030_gpio_irq_base;

/* genirq interfaces are not available to modules */
#ifdef MODULE
#define is_module()	true
#else
#define is_module()	false
#endif

/* GPIO_CTRL Fields */
#define MASK_GPIO_CTRL_GPIO0CD1		BIT(0)
#define MASK_GPIO_CTRL_GPIO1CD2		BIT(1)
#define MASK_GPIO_CTRL_GPIO_ON		BIT(2)

/* Mask for GPIO registers when aggregated into a 32-bit integer */
#define GPIO_32_MASK			0x0003ffff

/* Data structures */
static DEFINE_MUTEX(gpio_lock);

/* store usage of each GPIO. - each bit represents one GPIO */
static unsigned int gpio_usage_count;


//--[[ LGE_UBIQUIX_MODIFIED_START : shyun@ubiquix.com [2011.07.13] - Touch key Led on/off(Justin feature)
// LGE_JUSTIN_S 201114 kyoungmo.kang@lge.com, KEYLED off 5 seconds
#if (defined(CONFIG_PRODUCT_LGE_LU6800) )
struct work_struct *gwork_struct_val; 
#endif 
// LGE_JUSTIN_E 201114 kyoungmo.kang@lge.com, KEYLED off 5 seconds


// LGE_B_DOM_S 20101215 nttaejun.cho@lge.com, Touch key Led on/off
#if ( defined(CONFIG_PRODUCT_LGE_LU6800) && defined(JUSTIN_LEDOFF_TEST) )
static void twl4030_early_suspend(struct early_suspend *handler); 	
static void twl4030_late_resume(struct early_suspend *handler);		

struct twl4030_data {
	struct input_dev *input_dev;
	struct early_suspend early_suspend;

// LGE_JUSTIN_S 201114 kyoungmo.kang@lge.com, KEYLED off 5 seconds
//	struct hrtimer touchkey_timer_twl;	
	struct work_struct  touchkey_work_twl;
//	struct workqueue_struct	 	*touchkey_wq_twl;
// LGE_JUSTIN_E 201114 kyoungmo.kang@lge.com, KEYLED off 5 seconds

//	struct hrtimer timer;
	unsigned long delay;
};
#endif
//--]] LGE_UBIQUIX_MODIFIED_END : shyun@ubiquix.com [2011.07.13]- Touch key Led on/off(Justin feature)

/*----------------------------------------------------------------------*/

/*
 * To configure TWL4030 GPIO module registers
 */
static inline int gpio_twl4030_write(u8 address, u8 data)
{
	return twl_i2c_write_u8(TWL4030_MODULE_GPIO, data, address);
}

/*----------------------------------------------------------------------*/

/*
 * LED register offsets (use TWL4030_MODULE_{LED,PWMA,PWMB}))
 * PWMs A and B are dedicated to LEDs A and B, respectively.
 */

#define TWL4030_LED_LEDEN	0x0

/* LEDEN bits */
#define LEDEN_LEDAON		BIT(0)
#define LEDEN_LEDBON		BIT(1)
#define LEDEN_LEDAEXT		BIT(2)
#define LEDEN_LEDBEXT		BIT(3)
#define LEDEN_LEDAPWM		BIT(4)
#define LEDEN_LEDBPWM		BIT(5)
#define LEDEN_PWM_LENGTHA	BIT(6)
#define LEDEN_PWM_LENGTHB	BIT(7)

#define TWL4030_PWMx_PWMxON	0x0
#define TWL4030_PWMx_PWMxOFF	0x1

#define PWMxON_LENGTH		BIT(7)

//--[[ LGE_UBIQUIX_MODIFIED_START : shyun@ubiquix.com [2011.07.13] - KEYLED off 5 seconds(Justin feature)
#if ( defined(CONFIG_PRODUCT_LGE_LU6800) )
u8 key_led_flag_twl =0;
#endif
//--]] LGE_UBIQUIX_MODIFIED_END : shyun@ubiquix.com [2011.07.13]- KEYLED off 5 seconds(Justin feature)


/*----------------------------------------------------------------------*/

/*
 * To read a TWL4030 GPIO module register
 */
static inline int gpio_twl4030_read(u8 address)
{
	u8 data;
	int ret = 0;

	ret = twl_i2c_read_u8(TWL4030_MODULE_GPIO, &data, address);
	return (ret < 0) ? ret : data;
}

/*----------------------------------------------------------------------*/

static u8 cached_leden;		/* protected by gpio_lock */

/* The LED lines are open drain outputs ... a FET pulls to GND, so an
 * external pullup is needed.  We could also expose the integrated PWM
 * as a LED brightness control; we initialize it as "always on".
 */
static void twl4030_led_set_value(int led, int value)
{
	u8 mask = LEDEN_LEDAON | LEDEN_LEDAPWM;
	int status;

	if (led)
		mask <<= 1;

	printk("[%s]-[%d] mutex_lock[IN]!\n", __func__, __LINE__);
	mutex_lock(&gpio_lock);
	printk("[%s]-[%d] mutex_lock[OUT]!\n", __func__, __LINE__);

	if (value)
		cached_leden &= ~mask;
	else
		cached_leden |= mask;
	status = twl_i2c_write_u8(TWL4030_MODULE_LED, cached_leden,
			TWL4030_LED_LEDEN);
	mutex_unlock(&gpio_lock);
	printk("[%s]-[%d] mutex_unlock!\n", __func__, __LINE__);
}

//--[[ LGE_UBIQUIX_MODIFIED_START : shyun@ubiquix.com [2011.07.13] - Justin Feature is added from justin_froyo #5.
// LGE_B_DOM_S 20110104 nttaejun.cho@lge.com, Touch key Led on/off
#if ( defined(CONFIG_PRODUCT_LGE_LU6800) && defined(JUSTIN_LEDOFF_TEST) )
static  void twl4030_ledB_set_value(int led, int value)
{
	u8 mask = LEDEN_LEDBON | LEDEN_LEDBPWM;
	int status;

	if (led)
		mask <<= 1;

	printk("[%s]-[%d] mutex_lock[IN]!\n", __func__, __LINE__);
	mutex_lock(&gpio_lock);
	printk("[%s]-[%d] mutex_lock[OUT]!\n", __func__, __LINE__);
	if (value)
		cached_leden &= ~mask;
	else
		cached_leden |= mask;
	status = twl_i2c_write_u8(TWL4030_MODULE_LED, cached_leden,
			TWL4030_LED_LEDEN);
	mutex_unlock(&gpio_lock);
	printk("[%s]-[%d] mutex_unlock!\n", __func__, __LINE__);
} 
#endif
// LGE_B_DOM_E 20110104 nttaejun.cho@lge.com, Touch key Led on/off
//--]] LGE_UBIQUIX_MODIFIED_END : shyun@ubiquix.com [2011.07.13]- Justin Feature is added from justin_froyo #5.


static int twl4030_set_gpio_direction(int gpio, int is_input)
{
	u8 d_bnk = gpio >> 3;
	u8 d_msk = BIT(gpio & 0x7);
	u8 reg = 0;
	u8 base = REG_GPIODATADIR1 + d_bnk;
	int ret = 0;

	printk("[%s]-[%d] mutex_lock[IN]!\n", __func__, __LINE__);
	mutex_lock(&gpio_lock);
	printk("[%s]-[%d] mutex_lock[OUT]!\n", __func__, __LINE__);
	ret = gpio_twl4030_read(base);
	if (ret >= 0) {
		if (is_input)
			reg = ret & ~d_msk;
		else
			reg = ret | d_msk;

		ret = gpio_twl4030_write(base, reg);
	}
	mutex_unlock(&gpio_lock);
	printk("[%s]-[%d] mutex_unlock!\n", __func__, __LINE__);
	return ret;
}

static int twl4030_set_gpio_dataout(int gpio, int enable)
{
	u8 d_bnk = gpio >> 3;
	u8 d_msk = BIT(gpio & 0x7);
	u8 base = 0;

	if (enable)
		base = REG_SETGPIODATAOUT1 + d_bnk;
	else
		base = REG_CLEARGPIODATAOUT1 + d_bnk;

	return gpio_twl4030_write(base, d_msk);
}

static int twl4030_get_gpio_datain(int gpio)
{
	u8 d_bnk = gpio >> 3;
	u8 d_off = gpio & 0x7;
	u8 base = 0;
	int ret = 0;

	if (unlikely((gpio >= TWL4030_GPIO_MAX)
		|| !(gpio_usage_count & BIT(gpio))))
		return -EPERM;

	base = REG_GPIODATAIN1 + d_bnk;
	ret = gpio_twl4030_read(base);
	if (ret > 0)
		ret = (ret >> d_off) & 0x1;

	return ret;
}

/*----------------------------------------------------------------------*/

static int twl_request(struct gpio_chip *chip, unsigned offset)
{
	int status = 0;

	printk("[%s]-[%d] mutex_lock[IN]!\n", __func__, __LINE__);
	mutex_lock(&gpio_lock);
	printk("[%s]-[%d] mutex_lock[OUT]!\n", __func__, __LINE__);

	/* Support the two LED outputs as output-only GPIOs. */
	if (offset >= TWL4030_GPIO_MAX) {
		u8	ledclr_mask = LEDEN_LEDAON | LEDEN_LEDAEXT
				| LEDEN_LEDAPWM | LEDEN_PWM_LENGTHA;
		u8	module = TWL4030_MODULE_PWMA;

		offset -= TWL4030_GPIO_MAX;
		if (offset) {
			ledclr_mask <<= 1;
			module = TWL4030_MODULE_PWMB;
		}

		/* initialize PWM to always-drive */
		status = twl_i2c_write_u8(module, 0x7f,
				TWL4030_PWMx_PWMxOFF);
		if (status < 0)
			goto done;
		status = twl_i2c_write_u8(module, 0x7f,
				TWL4030_PWMx_PWMxON);
		if (status < 0)
			goto done;

		/* init LED to not-driven (high) */
		module = TWL4030_MODULE_LED;
		status = twl_i2c_read_u8(module, &cached_leden,
				TWL4030_LED_LEDEN);
		if (status < 0)
			goto done;
		cached_leden &= ~ledclr_mask;
		status = twl_i2c_write_u8(module, cached_leden,
				TWL4030_LED_LEDEN);
		if (status < 0)
			goto done;

		status = 0;
		goto done;
	}

	/* on first use, turn GPIO module "on" */
	if (!gpio_usage_count) {
		struct twl4030_gpio_platform_data *pdata;
		u8 value = MASK_GPIO_CTRL_GPIO_ON;

		/* optionally have the first two GPIOs switch vMMC1
		 * and vMMC2 power supplies based on card presence.
		 */
		pdata = chip->dev->platform_data;
		value |= pdata->mmc_cd & 0x03;

		status = gpio_twl4030_write(REG_GPIO_CTRL, value);
	}

	if (!status)
		gpio_usage_count |= (0x1 << offset);

done:
	mutex_unlock(&gpio_lock);
	printk("[%s]-[%d] mutex_unlock!\n", __func__, __LINE__);
	return status;
}

static void twl_free(struct gpio_chip *chip, unsigned offset)
{
	if (offset >= TWL4030_GPIO_MAX) {
		twl4030_led_set_value(offset - TWL4030_GPIO_MAX, 1);
		return;
	}

	printk("[%s]-[%d] mutex_lock[IN]!\n", __func__, __LINE__);
	mutex_lock(&gpio_lock);
	printk("[%s]-[%d] mutex_lock[OUT]!\n", __func__, __LINE__);

	gpio_usage_count &= ~BIT(offset);

	/* on last use, switch off GPIO module */
	if (!gpio_usage_count)
		gpio_twl4030_write(REG_GPIO_CTRL, 0x0);

	mutex_unlock(&gpio_lock);
	printk("[%s]-[%d] mutex_unlock!\n", __func__, __LINE__);
}

static int twl_direction_in(struct gpio_chip *chip, unsigned offset)
{
	return (offset < TWL4030_GPIO_MAX)
		? twl4030_set_gpio_direction(offset, 1)
		: -EINVAL;
}

static int twl_get(struct gpio_chip *chip, unsigned offset)
{
	int status = 0;

	if (offset < TWL4030_GPIO_MAX)
		status = twl4030_get_gpio_datain(offset);
	else if (offset == TWL4030_GPIO_MAX)
		status = cached_leden & LEDEN_LEDAON;
	else
		status = cached_leden & LEDEN_LEDBON;
	return (status < 0) ? 0 : status;
}

static int twl_direction_out(struct gpio_chip *chip, unsigned offset, int value)
{
	if (offset < TWL4030_GPIO_MAX) {
		twl4030_set_gpio_dataout(offset, value);
		return twl4030_set_gpio_direction(offset, 0);
	} else {
		twl4030_led_set_value(offset - TWL4030_GPIO_MAX, value);
		return 0;
	}
}

static void twl_set(struct gpio_chip *chip, unsigned offset, int value)
{
	if (offset < TWL4030_GPIO_MAX)
		twl4030_set_gpio_dataout(offset, value);
	else
		twl4030_led_set_value(offset - TWL4030_GPIO_MAX, value);
}

static int twl_to_irq(struct gpio_chip *chip, unsigned offset)
{
	return (twl4030_gpio_irq_base && (offset < TWL4030_GPIO_MAX))
		? (twl4030_gpio_irq_base + offset)
		: -EINVAL;
}

static struct gpio_chip twl_gpiochip = {
	.label			= "twl4030",
	.owner			= THIS_MODULE,
	.request		= twl_request,
	.free			= twl_free,
	.direction_input	= twl_direction_in,
	.get			= twl_get,
	.direction_output	= twl_direction_out,
	.set			= twl_set,
	.to_irq			= twl_to_irq,
	.can_sleep		= 1,
};

/*----------------------------------------------------------------------*/

static int __devinit gpio_twl4030_pulls(u32 ups, u32 downs)
{
	u8		message[6];
	unsigned	i, gpio_bit;

	/* For most pins, a pulldown was enabled by default.
	 * We should have data that's specific to this board.
	 */
	for (gpio_bit = 1, i = 1; i < 6; i++) {
		u8		bit_mask;
		unsigned	j;

		for (bit_mask = 0, j = 0; j < 8; j += 2, gpio_bit <<= 1) {
			if (ups & gpio_bit)
				bit_mask |= 1 << (j + 1);
			else if (downs & gpio_bit)
				bit_mask |= 1 << (j + 0);
		}
		message[i] = bit_mask;
	}

	return twl_i2c_write(TWL4030_MODULE_GPIO, message,
				REG_GPIOPUPDCTR1, 5);
}

static int __devinit gpio_twl4030_debounce(u32 debounce, u8 mmc_cd)
{
	u8		message[4];

	/* 30 msec of debouncing is always used for MMC card detect,
	 * and is optional for everything else.
	 */
	message[1] = (debounce & 0xff) | (mmc_cd & 0x03);
	debounce >>= 8;
	message[2] = (debounce & 0xff);
	debounce >>= 8;
	message[3] = (debounce & 0x03);

	return twl_i2c_write(TWL4030_MODULE_GPIO, message,
				REG_GPIO_DEBEN1, 3);
}

static int gpio_twl4030_remove(struct platform_device *pdev);

//--[[ LGE_UBIQUIX_MODIFIED_START : shyun@ubiquix.com [2011.07.13] - Justin feature is added from justin_froyo. #6
// LGE_JUSTIN_S 201114 kyoungmo.kang@lge.com, KEYLED off 5 seconds
#if ( defined(CONFIG_PRODUCT_LGE_LU6800) && defined(JUSTIN_LEDOFF_TEST) )

void keyled_touch_on(void)
{
	struct twl4030_data *twl_data = container_of(gwork_struct_val, struct twl4030_data, touchkey_work_twl);


	//printk(" #### kkm key led ON_timer update  ##### ");
    if(0 == key_led_flag_twl)
    {
		/* Be Here insert !  key led on */
		key_led_flag_twl = 1;	    
		twl4030_led_set_value(LED_B, LED_ON);
		//printk(" #### kkm key led ON Driver ##### ");
    }

//    hrtimer_start(&twl_data->touchkey_timer_twl, ktime_set(5, 0), HRTIMER_MODE_REL); /*5 sec */

}

#if 0
static void keyled_touchkey_work_func(struct work_struct *work)
{	
	/* Be Here insert ! key led off */
	key_led_flag_twl = 0;
    twl4030_led_set_value(LED_B, LED_OFF);
	//printk(" #### kkm key led OFF  ##### ");
}

static enum hrtimer_restart keyled_touchkey_timer_func(struct hrtimer *timer)
{
	struct twl4030_data *twl_data = container_of(gwork_struct_val, struct twl4030_data, touchkey_work_twl);

	queue_work(twl_data->touchkey_wq_twl, &twl_data->touchkey_work_twl);

}
#endif

EXPORT_SYMBOL(keyled_touch_on);
#endif 
// LGE_JUSTIN_E 201114 kyoungmo.kang@lge.com, KEYLED off 5 seconds




// LGE_B_DOM_S 20101215 nttaejun.cho@lge.com, Touch key Led on/off
#if ( defined(CONFIG_PRODUCT_LGE_LU6800) && defined(JUSTIN_LEDOFF_TEST))
#define LEDBON_   0
#define LEDBOFF_  1 

static void twl4030_early_suspend(struct early_suspend *h)
{
	struct twl4030_data *twl_data;  

	twl_data = container_of(h, struct twl4030_data, early_suspend);

// LGE_JUSTIN_S 201114 kyoungmo.kang@lge.com, KEYLED off 5 seconds
//	hrtimer_cancel(&twl_data->touchkey_timer_twl);
// LGE_JUSTIN_E 201114 kyoungmo.kang@lge.com, KEYLED off 5 seconds


//	twl4030_ledB_set_value(0, LEDBOFF_);
//	twl4030_led_set_value(LED_B, LED_OFF);
//	printk("[#######] akm8973_early_suspend() \n");
}


static void twl4030_late_resume(struct early_suspend *h)
{
	struct twl4030_data *twl_data;

	twl_data = container_of(h, struct twl4030_data, early_suspend);

// LGE_JUSTIN_S 201114 kyoungmo.kang@lge.com, KEYLED off 5 seconds
//	hrtimer_start(&twl_data->touchkey_timer_twl, ktime_set(5, 1), HRTIMER_MODE_REL); /*5 sec */
// LGE_JUSTIN_E 201114 kyoungmo.kang@lge.com, KEYLED off 5 seconds


//	twl4030_ledB_set_value(0, LEDBON_);

//	printk("[#######] akm8973_late_resume() \n");	
}
#endif
// LGE_B_DOM_E 20101215 nttaejun.cho@lge.com, Touch key Led on/off

// LGE_JUSTIN_S 20110128 nttaejun.cho@lge.com, Led on/off
#define TWL4030_LED_ON		255
#define TWL4030_LED_OFF		0

#if ( defined(CONFIG_PRODUCT_LGE_LU6800) )
static ssize_t twl4030_led_onoff_store(struct device *dev, struct device_attribute *attr, char *buf, size_t count)
{
	int    val;

	sscanf(buf, "%d", &val);

	printk("[ %s ]-(%d) [IN / val = %d]\n",__func__, __LINE__, val);
	  
	if (val == TWL4030_LED_ON) 
	{
		printk("[%s] TWL4030_LED_ON!\n",__func__);
		twl4030_ledB_set_value(0, LEDBON_);
	} 
	else if (val == TWL4030_LED_OFF)
	{
		printk("[%s] TWL4030_LED_OFF!\n",__func__);
		twl4030_ledB_set_value(0, LEDBOFF_);
		twl4030_led_set_value(LED_B, LED_OFF);
	}
	else
	{
		printk("[%s] on/off error, value = %d\n",__func__, val);
		return -1;
	}

	return count;
}

//--[[ LGE_UBIQUIX_MODIFIED_START : shyun@ubiquix.com [2011.10.12] - Sysfs access permission is modified.
//static DEVICE_ATTR(led_onoff, 0666, NULL, twl4030_led_onoff_store);
static DEVICE_ATTR(led_onoff, 0664, NULL, twl4030_led_onoff_store);
//--]] LGE_UBIQUIX_MODIFIED_END : shyun@ubiquix.com [2011.10.12]- Sysfs access permission is modified.

static struct attribute *twl4030_led_attributes[] = {
	&dev_attr_led_onoff.attr,
	NULL
};

static const struct attribute_group twl4030_led_group = {
	.attrs = twl4030_led_attributes,
};
#endif
// LGE_JUSTIN_E 20110128 nttaejun.cho@lge.com, Led on/off
//--]] LGE_UBIQUIX_MODIFIED_END : shyun@ubiquix.com [2011.07.13]- Justin feature is added from justin_froyo. #6

static int __devinit gpio_twl4030_probe(struct platform_device *pdev)
{
	struct twl4030_gpio_platform_data *pdata = pdev->dev.platform_data;

//--[[ LGE_UBIQUIX_MODIFIED_START : shyun@ubiquix.com [2011.07.13] - Led on/off
#if ( defined(CONFIG_PRODUCT_LGE_LU6800) )
	struct device *dev = &pdev->dev;
#endif
//--]] LGE_UBIQUIX_MODIFIED_END : shyun@ubiquix.com [2011.07.13]- Led on/off

	int ret;

//--[[ LGE_UBIQUIX_MODIFIED_START : shyun@ubiquix.com [2011.07.13] - Led on/off
#if ( defined(CONFIG_PRODUCT_LGE_LU6800) && defined(JUSTIN_LEDOFF_TEST))
	struct twl4030_data *twl_data;
#endif
//--]] LGE_UBIQUIX_MODIFIED_END : shyun@ubiquix.com [2011.07.13]- Led on/off

	printk("[SHYUN] [%s] - [%d] [IN]\n", __func__, __LINE__);

	/* maybe setup IRQs */
	if (pdata->irq_base) {
		if (is_module()) {
			dev_err(&pdev->dev,
				"can't dispatch IRQs from modules\n");
			goto no_irqs;
		}
		ret = twl4030_sih_setup(TWL4030_MODULE_GPIO);
		if (ret < 0)
			return ret;
		WARN_ON(ret != pdata->irq_base);
		twl4030_gpio_irq_base = ret;
	}

no_irqs:
	/*
	 * NOTE:  boards may waste power if they don't set pullups
	 * and pulldowns correctly ... default for non-ULPI pins is
	 * pulldown, and some other pins may have external pullups
	 * or pulldowns.  Careful!
	 */
	ret = gpio_twl4030_pulls(pdata->pullups, pdata->pulldowns);
	if (ret)
		dev_dbg(&pdev->dev, "pullups %.05x %.05x --> %d\n",
				pdata->pullups, pdata->pulldowns,
				ret);

	ret = gpio_twl4030_debounce(pdata->debounce, pdata->mmc_cd);
	if (ret)
		dev_dbg(&pdev->dev, "debounce %.03x %.01x --> %d\n",
				pdata->debounce, pdata->mmc_cd,
				ret);

	twl_gpiochip.base = pdata->gpio_base;
	twl_gpiochip.ngpio = TWL4030_GPIO_MAX;
	twl_gpiochip.dev = &pdev->dev;

	/* NOTE: we assume VIBRA_CTL.VIBRA_EN, in MODULE_AUDIO_VOICE,
	 * is (still) clear if use_leds is set.
	 */
	if (pdata->use_leds)
		twl_gpiochip.ngpio += 2;

	ret = gpiochip_add(&twl_gpiochip);
	if (ret < 0) {
		dev_err(&pdev->dev,
				"could not register gpiochip, %d\n",
				ret);
		twl_gpiochip.ngpio = 0;
		gpio_twl4030_remove(pdev);
	} else if (pdata->setup) {
		int status;

		status = pdata->setup(&pdev->dev,
				pdata->gpio_base, TWL4030_GPIO_MAX);
		if (status)
			dev_dbg(&pdev->dev, "setup --> %d\n", status);
	}

//--[[ LGE_UBIQUIX_MODIFIED_START : shyun@ubiquix.com [2011.07.13] - Touch key Led on/off
#if ( defined(CONFIG_PRODUCT_LGE_LU6800) && defined(JUSTIN_LEDOFF_TEST) )
//	twl4030_ledB_set_value(0, LEDBON_);
//    printk("#*#*#* kkm_lde_on #*#*#\n"); 

	twl_data = kzalloc(sizeof(struct twl4030_data), GFP_KERNEL);
	if (!twl_data) {
		return -ENOMEM;
	}

	twl_data->input_dev = input_allocate_device();

	if (!twl_data->input_dev) {
		printk(KERN_ERR
		       "hub_akm8973_probe: Failed to allocate input device\n");
		return -ENOMEM;
	}

	twl_data->input_dev->name = "ledonoff";

// LGE_JUSTIN_S 20110128 nttaejun.cho@lge.com, Led on/off
#if ( defined(CONFIG_PRODUCT_LGE_LU6800) )
	//	init. sysfs	 
  if (sysfs_create_group(&dev->kobj, &twl4030_led_group))
  {
	    printk("[twl4030_gpio] sysfs_create_group FAIL \n");
	    return -ENOMEM;
  }
#endif
// LGE_JUSTIN_E 20110128 nttaejun.cho@lge.com, Led on/off

// LGE_JUSTIN_S 201114 kyoungmo.kang@lge.com, KEYLED off 5 seconds
#if 0
#if ( defined(CONFIG_PRODUCT_LGE_LU6800) && defined(JUSTIN_LEDOFF_TEST) ) 

    INIT_WORK(&twl_data->touchkey_work_twl, keyled_touchkey_work_func);
	gwork_struct_val = &twl_data->touchkey_work_twl;
	twl_data->touchkey_wq_twl = create_singlethread_workqueue("touchkey_wq_twl");
	if (!twl_data->touchkey_wq_twl)
		return -ENOMEM;

//	hrtimer_init(&twl_data->touchkey_timer_twl, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
//	twl_data->touchkey_timer_twl.function = keyled_touchkey_timer_func;

//    hrtimer_start(&twl_data->touchkey_timer_twl, ktime_set(10, 0), HRTIMER_MODE_REL); /*After 10 sec key led off*/

	
#endif 
#endif
// LGE_JUSTIN_E 201114 kyoungmo.kang@lge.com, KEYLED off 5 seconds

#if 0	
	twl_data->early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN;
	twl_data->early_suspend.suspend = twl4030_early_suspend;
	twl_data->early_suspend.resume = twl4030_late_resume;
	register_early_suspend(&twl_data->early_suspend);
#endif	

#endif
//--]] LGE_UBIQUIX_MODIFIED_END : shyun@ubiquix.com [2011.07.13]- Touch key Led on/off

	printk("[SHYUN] [%s] - [%d] [OUT][ret = %d]\n", __func__, __LINE__, ret);
	return ret;
}

/* Cannot use __devexit as gpio_twl4030_probe() calls us */
static int gpio_twl4030_remove(struct platform_device *pdev)
{
	struct twl4030_gpio_platform_data *pdata = pdev->dev.platform_data;
	int status;

	if (pdata->teardown) {
		status = pdata->teardown(&pdev->dev,
				pdata->gpio_base, TWL4030_GPIO_MAX);
		if (status) {
			dev_dbg(&pdev->dev, "teardown --> %d\n", status);
			return status;
		}
	}

	status = gpiochip_remove(&twl_gpiochip);
	if (status < 0)
		return status;

	if (is_module())
		return 0;

	/* REVISIT no support yet for deregistering all the IRQs */
	WARN_ON(1);
	return -EIO;
}

/* Note:  this hardware lives inside an I2C-based multi-function device. */
MODULE_ALIAS("platform:twl4030_gpio");

static struct platform_driver gpio_twl4030_driver = {
	.driver.name	= "twl4030_gpio",
	.driver.owner	= THIS_MODULE,
	.probe		= gpio_twl4030_probe,
	.remove		= gpio_twl4030_remove,
};

static int __init gpio_twl4030_init(void)
{
	return platform_driver_register(&gpio_twl4030_driver);
}
subsys_initcall(gpio_twl4030_init);

static void __exit gpio_twl4030_exit(void)
{
	platform_driver_unregister(&gpio_twl4030_driver);
}
module_exit(gpio_twl4030_exit);

MODULE_AUTHOR("Texas Instruments, Inc.");
MODULE_DESCRIPTION("GPIO interface for TWL4030");
MODULE_LICENSE("GPL");
