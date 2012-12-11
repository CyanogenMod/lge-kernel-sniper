
/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; 
 * vim: set ts=8 sw=8 et:
 */

/* Hub Proximity sensor control */
#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/fb.h>
#include <linux/string.h>
#include <asm/gpio.h>
#include <asm/system.h>
#include <linux/interrupt.h>
#include <linux/i2c.h>
//--[[ LGE_UBIQUIX_MODIFIED_START : shyun@ubiquix.com [2011.07.14] - Not used this header file
//#include <plat/resource.h>
//--]] LGE_UBIQUIX_MODIFIED_END : shyun@ubiquix.com [2011.07.14]- Not used this header file
#include <linux/regulator/consumer.h>
#include <linux/uaccess.h>
#include <linux/input.h>

#include "../mux.h"

#ifdef CONFIG_HAS_EARLYSUSPEND /* 20110304 seven.kim@lge.com late_resume_lcd [START] */
#include <linux/earlysuspend.h>
#endif   /* 20110304 seven.kim@lge.com late_resume_lcd [END] */

#include <linux/wakelock.h>

#define PROXI_LDO_EN	150
#define PROXI_OUT		14
#define proxi_output_1 14
#define I2C_NO_REG 0xff
#define PROXI_MIN_DELAY_NS 10000000
#define PROXI_DEFAULT_DELAY_NS 200000000

#define ALARM_FLIP_ACTIVE		1
#define ALARM_FLIP_DEACTIVE		0
static int alarm_flip = ALARM_FLIP_DEACTIVE;

static atomic_t proxi_status;
static struct regulator *reg;
static bool enabled=false;
static bool lcd_off_skip=false;


// 20100826 jh.koo@lge.com, for stable initialization [START_LGE]
static struct i2c_client *hub_proximity_client = NULL;
// 20100826 jh.koo@lge.com, for stable initialization [END_LGE]

//--[[ LGE_UBIQUIX_MODIFIED_START : shyun@ubiquix.com [2012.07.31] - FQ-RCA Issue(12_0657)
struct wake_lock proxi_irq_wake_lock;
//--]] LGE_UBIQUIX_MODIFIED_END : shyun@ubiquix.com [2012.07.31]- FQ-RCA Issue(12_0657)

//int skip_far_event_flag = 0; 

struct hub_proxi_data {
	struct i2c_client *client;
	struct input_dev *input_dev;
	struct hrtimer timer;
	struct work_struct  work;
	unsigned long delay;
	bool use_int_mode;
	bool wakeup_while_sleep;
#ifdef CONFIG_HAS_EARLYSUSPEND  /* 20110304 seven.kim@lge.com late_resume_lcd [START] */
	struct early_suspend early_suspend;
#endif  /* 20110304 seven.kim@lge.com late_resume_lcd [END] */
};

struct hub_proxi_ctrl_data {
	unsigned short reg;
	unsigned short val;
};

// LGE_JUSTIN_S 20110113 nttaejun.cho@lge.com, proximity sensor
static struct hub_proxi_ctrl_data initialize_seq[]=
{
//--[[ LGE_UBIQUIX_MODIFIED_START : shyun@ubiquix.com [2012.03.30] - CONFIG_PRODUCT_LGE_LU6800 featuring
#if defined(CONFIG_PRODUCT_LGE_LU6800)
//--]] LGE_UBIQUIX_MODIFIED_END : shyun@ubiquix.com [2012.03.30]- CONFIG_PRODUCT_LGE_LU6800 featuring
// LGE_JUSTIN_S ntyouyoung.lee 2011/02/17 
#if 0
	{0x01,0x08},    // B2 mode
	{0x02,0x20},
	{0x03,0x04},
	{0x04,0x03},
	{0x06,0x00},
	{I2C_NO_REG, 0x00}
#else
		{0x01,0x08},   // A mode
		{0x02,0xC2},
		{0x03,0x04},
		{0x04,0x01},
		{0x06,0x00},
		{I2C_NO_REG, 0x00}
#endif
// LGE_JUSTIN_E ntyouyoung.lee 2011/02/17 
#else							// A mode
		{0x01,0x08},
		{0x02,0xC2},
		{0x03,0x04},
		{0x04,0x01},
		{0x06,0x00},
		{I2C_NO_REG, 0x00}
#endif
};
// LGE_JUSTIN_E 20110113 nttaejun.cho@lge.com, proximity sensor
bool lcd_off_skip_check(void)
{
   return lcd_off_skip;
}

//--[[ LGE_UBIQUIX_MODIFIED_START : shyun@ubiquix.com [2011.10.05] - Remove the compile warning.
//bool set_lcd_off_skip_check(bool skip_check)
void set_lcd_off_skip_check(bool skip_check)
//--]] LGE_UBIQUIX_MODIFIED_END : shyun@ubiquix.com [2011.10.05]- Remove the compile warning.
{
   lcd_off_skip = skip_check;
}

static int hub_proxi_write_reg(struct i2c_client *client, unsigned char reg, unsigned char val)
{
	int err;
	unsigned char buf[2];

	struct i2c_msg	msg = {
		client->addr, 0, 2, buf
	};

	buf[0] = reg;
	buf[1] = val;	

	if ((err = i2c_transfer(client->adapter, &msg, 1)) < 0) {
		dev_err(&client->dev, "i2c write error\n");
	}

	return 0;
}

static int hub_proxi_read_reg(struct i2c_client *client, unsigned char reg, unsigned char *ret)
{
	int err;
	unsigned char buf = reg;
	unsigned char buf_16[2];


	struct i2c_msg msg[2] = {
		{ client->addr, 0, 1, &buf },
		{ client->addr, I2C_M_RD, 2, buf_16}
	};

	if ((err = i2c_transfer(client->adapter, msg, 2)) < 0) {
		dev_err(&client->dev, "i2c read error\n");
	}

	*ret = buf_16[1];

	return 0;
}


static int hub_read_vo_bit(struct i2c_client *client)
{
//--[[ LGE_UBIQUIX_MODIFIED_START : shyun@ubiquix.com [2011.10.05] - Remove the compile warning.
//	int val = 0;
	unsigned char val = 0;
//--]] LGE_UBIQUIX_MODIFIED_END : shyun@ubiquix.com [2011.10.05]- Remove the compile warning.

	if(enabled) {
		msleep(1);
		hub_proxi_read_reg(client, 0x00, &val);
	}
	else
		return -EPERM;

	val = val & 0x01 ? 0 : 1; //0:near, 1:far

//--[[ LGE_UBIQUIX_MODIFIED_START : shyun@ubiquix.com [2011.08.02] - Remove the unnecessary log.
	//printk("[!]%s() read val = %x\n", __func__, val);
//--]] LGE_UBIQUIX_MODIFIED_END : shyun@ubiquix.com [2011.08.02]- Remove the unnecessary log.
	//	hub_proxi_write_reg(client, 0x02, 0x20); // test

	return val;

}

static void hub_proxi_power_onoff(bool enable)
{
// LGE_JUSTIN_S 20101229 nttaejun.cho@lge.com, proximity sensor
//--[[ LGE_UBIQUIX_MODIFIED_START : shyun@ubiquix.com [2012.03.30] - CONFIG_PRODUCT_LGE_LU6800 featuring

#if defined(CONFIG_PRODUCT_LGE_LU6800)
//--]] LGE_UBIQUIX_MODIFIED_END : shyun@ubiquix.com [2012.03.30]- CONFIG_PRODUCT_LGE_LU6800 featuring

	if(enable){
		gpio_direction_output(PROXI_LDO_EN, 1);
	//	gpio_set_value(PROXI_LDO_EN, 1);
		enabled = true; 	
    	lcd_off_skip = true;
	//	printk("[!] %s() - PROXI_LDO_EN(GPIO 150) enabled\n", __func__);
	}
	else {
		gpio_direction_output(PROXI_LDO_EN, 0);
	//	gpio_set_value(PROXI_LDO_EN, 0);
		enabled = false;
    	lcd_off_skip = false;
	//	printk("[!] %s() - PROXI_LDO_EN(GPIO 150) disabled\n", __func__);
	}
	//	printk("[!] %s() - PROXI_LDO_EN(GPIO 150) value : %d\n", __func__, gpio_get_value(PROXI_LDO_EN));
#endif 
// LGE_JUSTIN_S 20101229 nttaejun.cho@lge.com, proximity sensor	

//--[[ LGE_UBIQUIX_MODIFIED_START : shyun@ubiquix.com [2012.03.30] - KU5900 / P970 featuring
#if defined(CONFIG_MACH_LGE_HEAVEN_REV_A) || defined(CONFIG_PRODUCT_LGE_KU5900) || defined(CONFIG_PRODUCT_LGE_P970) /*defined(CONFIG_MACH_LGE_HUB)*/
//--]] LGE_UBIQUIX_MODIFIED_END : shyun@ubiquix.com [2012.03.30]- KU5900 / P970 featuring

	if(enable) {
		regulator_enable(reg);
		enabled = true; 
		lcd_off_skip = true;
//--[[ LGE_UBIQUIX_MODIFIED_START : shyun@ubiquix.com [2011.08.02] - Remove the unnecessary log.
		//printk("[!] %s() - PROXI_LDO_EN enabled\n", __func__);		
//--]] LGE_UBIQUIX_MODIFIED_END : shyun@ubiquix.com [2011.08.02]- Remove the unnecessary log.
	}
	else {
		regulator_disable(reg);
		enabled = false;
		lcd_off_skip = false;
//--[[ LGE_UBIQUIX_MODIFIED_START : shyun@ubiquix.com [2011.08.02] - Remove the unnecessary log.
		//printk("[!] %s() - PROXI_LDO_EN disabled\n", __func__);
//--]] LGE_UBIQUIX_MODIFIED_END : shyun@ubiquix.com [2011.08.02]- Remove the unnecessary log.
	}
#endif

#if 0
	if(enable){
		gpio_direction_output(PROXI_LDO_EN, 1);
		gpio_set_value(PROXI_LDO_EN, 1);
		enabled = true; 	
		printk("[!] %s() - PROXI_LDO_EN(GPIO 150) enabled\n", __func__);
	}
	else {
		gpio_direction_output(PROXI_LDO_EN, 0);
		gpio_set_value(PROXI_LDO_EN, 0);
		enabled = false;
		printk("[!] %s() - PROXI_LDO_EN(GPIO 150) disabled\n", __func__);
	}
	//	printk("[!] %s() - PROXI_LDO_EN(GPIO 150) value : %d\n", __func__, gpio_get_value(PROXI_LDO_EN));
#endif

	return;
}

static void hub_proxi_i2c_init(struct i2c_client *client)
{
	unsigned int i;

	for (i = 0; initialize_seq[i].reg != I2C_NO_REG; i++) {
		hub_proxi_write_reg(client, initialize_seq[i].reg, initialize_seq[i].val);
		printk("[!] %s() - reg : 0x%x, val : 0x%x\n", __func__, initialize_seq[i].reg, initialize_seq[i].val);
	}
}

static void hub_proxi_enable(struct i2c_client *client)
{
	struct hub_proxi_data *data = i2c_get_clientdata(client);

	if (enabled == true) return;

	hub_proxi_power_onoff(1);
	msleep(10);				// for stable i2c init
	hub_proxi_i2c_init(data->client);

#if 0
	if (data->use_int_mode)
		enable_irq(client->irq);
	else
		hrtimer_start(&data->timer, ktime_set(1, 0), HRTIMER_MODE_REL);
#endif

}

static void hub_proxi_disable(struct i2c_client *client)
{
	struct hub_proxi_data* data = i2c_get_clientdata(client);

	if (enabled == false) return;
	
#if 0
	if (data->use_int_mode)
		disable_irq(client->irq);
	else
		hrtimer_cancel(&data->timer);
#endif

	cancel_work_sync(&data->work);

	hub_proxi_power_onoff(0);

}

static void hub_proxi_det_work(struct work_struct *work)
{
	struct hub_proxi_data *data = container_of(work, struct hub_proxi_data, work);

//	if(skip_far_event_flag == 1)
//	{
//		skip_far_event_flag = 0;
//		return;
//	}

//--[[ LGE_UBIQUIX_MODIFIED_START : shyun@ubiquix.com [2012.07.31] - FQ-RCA Issue(12_0657)
	wake_lock(&proxi_irq_wake_lock);

	atomic_set(&proxi_status, hub_read_vo_bit(data->client));

//	int vo = hub_read_vo_bit(data->client);

	input_report_abs(data->input_dev, ABS_DISTANCE, atomic_read(&proxi_status));
//--[[ LGE_UBIQUIX_MODIFIED_START : shyun@ubiquix.com [2011.08.02] - Remove the unnecessary log.
	//printk("%s distance=%d\n", __FUNCTION__, atomic_read(&proxi_status));
//--]] LGE_UBIQUIX_MODIFIED_END : shyun@ubiquix.com [2011.08.02]- Remove the unnecessary log.
	input_sync(data->input_dev);

	wake_lock_timeout(&proxi_irq_wake_lock, msecs_to_jiffies(2000));
//--]] LGE_UBIQUIX_MODIFIED_END : shyun@ubiquix.com [2012.07.31]- FQ-RCA Issue(12_0657)
}

static irqreturn_t hub_proxi_int_handler(int irq, void *dev_id)
{
	struct hub_proxi_data *data = dev_id;

//--[[ LGE_UBIQUIX_MODIFIED_START : shyun@ubiquix.com [2011.08.02] - Remove the unnecessary log.
	//printk("[Proximity_IRQ] [irq = %d]\n",irq);
//--]] LGE_UBIQUIX_MODIFIED_END : shyun@ubiquix.com [2011.08.02]- Remove the unnecessary log.

	if(alarm_flip == ALARM_FLIP_DEACTIVE)
		schedule_work(&data->work);
	return IRQ_HANDLED; 												  
}	  

static enum hrtimer_restart hub_proxi_timer_func(struct hrtimer *timer)
{
	struct hub_proxi_data *data = container_of(timer, struct hub_proxi_data, timer);
	
	schedule_work(&data->work);
	hrtimer_start(&data->timer, ktime_set(0, data->delay), HRTIMER_MODE_REL); /* 200 msec */

   	return HRTIMER_NORESTART;
}

static ssize_t hub_proxi_status_show(struct device *dev,  struct device_attribute *attr,  char *buf)
{
	struct hub_proxi_data* data = dev_get_drvdata(dev);
	int status;
	status = hub_read_vo_bit(data->client);

	// 	sprintf(buf, "%d\n", atomic_read(&proxi_status));
	sprintf(buf, "%d\n", status);
	return (ssize_t)(strlen(buf) + 1);
}

static ssize_t hub_proxi_onoff_show(struct device *dev,  struct device_attribute *attr,  char *buf)
{
	sprintf(buf, "%d\n", enabled==true);
	return (ssize_t)(strlen(buf) + 1);
}

/* S[, 20120922, mannsik.chung@lge.com, PM from froyo. */
#if defined(CONFIG_PRODUCT_LGE_LU6800)
extern void omap_pm_cpu_set_freq(unsigned long f);
extern u32 te_cpu_idle_block;
#endif
/* E], 20120922, mannsik.chung@lge.com, PM from froyo. */

static ssize_t hub_proxi_onoff_store(struct device *dev,  struct device_attribute *attr,  const char *buf, size_t count)
{
	int val;
	int ret;
	struct hub_proxi_data* data = dev_get_drvdata(dev);

	printk("[ %s ]-(%d) [IN]\n",__func__, __LINE__);

/* S[, 20120922, mannsik.chung@lge.com, PM from froyo. */
#if defined(CONFIG_PRODUCT_LGE_LU6800)
	te_cpu_idle_block = 1;
#endif
/* E], 20120922, mannsik.chung@lge.com, PM from froyo. */

	val = simple_strtoul(buf, NULL, 10);
	sscanf(buf, "%d", &ret);

	//sprintf(buf, "%d\n", atomic_read(&proxi_status));

	if(ret)	{
		hub_proxi_enable(data->client);
		enable_irq(data->client->irq);
	}
	else {
		disable_irq(data->client->irq);
		hub_proxi_disable(data->client);		
	}

	data->wakeup_while_sleep = (bool)ret;

	return count;
}
// 20100827 jh.koo@lge.com [START_LGE]
int hub_proximity_check(void)
{
	int proximity_status;
	alarm_flip = ALARM_FLIP_ACTIVE;
	
	hub_proxi_enable(hub_proximity_client);
	disable_irq(hub_proximity_client->irq);

	proximity_status = hub_read_vo_bit(hub_proximity_client);

	printk(KERN_DEBUG"[!]%s() - proximity status : %d\n", __func__, proximity_status);

	hub_proxi_disable(hub_proximity_client);	
	alarm_flip = ALARM_FLIP_DEACTIVE;
	
	return proximity_status;
}
EXPORT_SYMBOL(hub_proximity_check);
// 20100827 jh.koo@lge.com [END_LGE]

static ssize_t hub_proxi_delay_show(struct device *dev,  struct device_attribute *attr,  char *buf)
{
	struct hub_proxi_data* data = dev_get_drvdata(dev);
	sprintf(buf, "%lu\n", data->delay);
	return (ssize_t)(strlen(buf) + 1);
}

static ssize_t hub_proxi_delay_store(struct device *dev,  struct device_attribute *attr,  const char *buf, size_t count)
{
	unsigned long val;
	struct hub_proxi_data* data = dev_get_drvdata(dev);

	val = simple_strtoul(buf, NULL, 10) * 1000000;  //ms to ns

	if(val < PROXI_MIN_DELAY_NS)
		data->delay = PROXI_MIN_DELAY_NS;
	else
		data->delay = val;

	return count;
}

static ssize_t hub_proxi_wake_show(struct device *dev,  struct device_attribute *attr,  char *buf)
{
	struct hub_proxi_data* data = dev_get_drvdata(dev);
	sprintf(buf, "%lu\n", data->delay);
	return (ssize_t)(strlen(buf) + 1);
}

static ssize_t hub_proxi_wake_store(struct device *dev,  struct device_attribute *attr,  const char *buf, size_t count)
{
	unsigned long val;
	struct hub_proxi_data* data = dev_get_drvdata(dev);

	val = simple_strtoul(buf, NULL, 10);

	data->wakeup_while_sleep = (bool)val;

	return count;
}

//echo 1 > /sys/bus/i2c/drivers/hub_proxi/2-0044/onoff

static DEVICE_ATTR(status, 0444, hub_proxi_status_show, NULL);
//--[[ LGE_UBIQUIX_MODIFIED_START : shyun@ubiquix.com [2011.09.21] - For CTS Permission error.(0666 -> 0664)
static DEVICE_ATTR(onoff, 0664, hub_proxi_onoff_show, hub_proxi_onoff_store);
static DEVICE_ATTR(delay, 0664, hub_proxi_delay_show, hub_proxi_delay_store);
static DEVICE_ATTR(wake, 0664, hub_proxi_wake_show, hub_proxi_wake_store);
//--]] LGE_UBIQUIX_MODIFIED_END : shyun@ubiquix.com [2011.09.21]- For CTS Permission error.(0666 -> 0664)

static struct attribute *hub_proxi_attributes[] = {
	&dev_attr_status.attr,
	&dev_attr_onoff.attr,
	&dev_attr_delay.attr,
	&dev_attr_wake.attr,
	NULL
};

static const struct attribute_group hub_proxi_group = {
	.attrs = hub_proxi_attributes,
};

#ifdef CONFIG_HAS_EARLYSUSPEND  /* 20110304 seven.kim@lge.com late_resume_lcd [START] */
static void hub_proxi_early_suspend(struct early_suspend *handler);
static void hub_proxi_late_resume(struct early_suspend *handler);
#endif  /* 20110304 seven.kim@lge.com late_resume_lcd [END] */

static int __init hub_proxi_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int ret;
	struct hub_proxi_data *data;
	struct device *dev = &client->dev;

	printk("[shyun] hub_proxi_probe [IN]\n");

	// 20100827 jh.koo@lge.com [START_LGE]
	hub_proximity_client = client;
	// 20100827 jh.koo@lge.com [END_LGE]

	//Event_to_application(client);

	data = kzalloc(sizeof(struct hub_proxi_data), GFP_KERNEL);
	if (!data) {
		return -ENOMEM;
	}

	// 20100810 jh.koo@lge.com GPIO Initialization [START_LGE]
	omap_mux_init_gpio(PROXI_LDO_EN, OMAP_PIN_OUTPUT);
	omap_mux_init_gpio(PROXI_OUT, OMAP_PIN_INPUT_PULLUP | OMAP_PIN_OFF_WAKEUPENABLE);

	ret = gpio_request(PROXI_LDO_EN, "proximity enable gpio");
	if(ret < 0) {	
		printk("can't get hub proximity enable GPIO\n");
		kzfree(data);
		return -ENOSYS;
	}

	data->use_int_mode = true; //interrupt mode
	//	gpio_request(proxi_output_1, "proxi int gpio");
	//	gpio_direction_input(proxi_output_1); 
	//	request_irq(client->irq, hub_proxi_sleep_int_handler, IRQF_DISABLED | IRQF_TRIGGER_FALLING, "proxi_driver", data); 
	//	enable_irq_wake(client->irq); //wake up irq

	if (data->use_int_mode) {

		printk(KERN_WARNING"%s() : interrupt mode. START\n", __func__);

		if (gpio_request(PROXI_OUT, "proxi interrupt gpio") < 0) {
			printk("can't get hub proxi irq GPIO\n");
			kzfree(data);
			return -ENOSYS;
		}

		ret = gpio_direction_input(PROXI_OUT); 

		ret = request_irq(gpio_to_irq(PROXI_OUT), hub_proxi_int_handler, IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING, "proximity_interrupt", data); 
		if (ret < 0){
			printk(KERN_INFO "[Proximity INT] GPIO 14 IRQ line set up failed!\n");
			free_irq(gpio_to_irq(PROXI_OUT), data);
			return -ENOSYS;
		}	
#if 1
		/* Make the interrupt on wake up OMAP which is in suspend mode */		
		ret = enable_irq_wake(gpio_to_irq(PROXI_OUT));		
		if(ret < 0){
			printk(KERN_INFO "[Proximity INT] GPIO 14 wake up source setting failed!\n");
			disable_irq_wake(gpio_to_irq(PROXI_OUT));
			return -ENOSYS;
		}
#endif				
		printk(KERN_WARNING"%s() : interrupt mode. END\n", __func__);

		//		ret = request_irq(client->irq, hub_proxi_int_handler, IRQF_DISABLED | IRQF_TRIGGER_FALLING, "proxi_driver", data); 
	} 
	else {
		hrtimer_init(&data->timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
		data->timer.function = hub_proxi_timer_func;
		data->delay = PROXI_DEFAULT_DELAY_NS;
		//hrtimer_start(&data->timer, ktime_set(0, data->delay), HRTIMER_MODE_REL);
	}

//--[[ LGE_UBIQUIX_MODIFIED_START : shyun@ubiquix.com [2012.07.31] - FQ-RCA Issue(12_0657)
	wake_lock_init(&proxi_irq_wake_lock, WAKE_LOCK_SUSPEND, "proxi_irq");
//--]] LGE_UBIQUIX_MODIFIED_END : shyun@ubiquix.com [2012.07.31]- FQ-RCA Issue(12_0657)
	
	INIT_WORK(&data->work, hub_proxi_det_work);

	/*LGE_CHANGE_S [kyw2029@lge.com] 2010-01-04, ldoc control*/
#if defined(CONFIG_MACH_LGE_HEAVEN_REV_A)
	reg = regulator_get(dev, "vaux3");
	if (reg == NULL) {
		printk(KERN_ERR": Failed to get PROXI power resources !! \n");
		return -ENODEV;
	}
//--[[ LGE_UBIQUIX_MODIFIED_START : shyun@ubiquix.com [2012.03.30] - KU5900 / P970 Featuring
#elif defined(CONFIG_PRODUCT_LGE_KU5900) || defined(CONFIG_PRODUCT_LGE_P970) /*defined(CONFIG_MACH_LGE_HUB)*/
//--]] LGE_UBIQUIX_MODIFIED_END : shyun@ubiquix.com [2012.03.30]- KU5900 / P970 Featuring
	reg = regulator_get(dev, "vaux1");
	if (reg == NULL) {
		printk(KERN_ERR": Failed to get PROXI power resources !! \n");
		return -ENODEV;
	}
#endif
	/*LGE_CHANGE_S [kyw2029@lge.com] 2010-01-04, ldoc control*/

	//hub_proxi_power_onoff(1);
	//hub_proxi_i2c_init(client);

	data->client = client;
	i2c_set_clientdata(client, data);

	data->input_dev = input_allocate_device();
	if (data->input_dev == NULL) {
		printk(KERN_ERR "%s: input_allocate: not enough memory\n",
				__FUNCTION__);
		return -ENOMEM;
	}

	set_bit(EV_KEY, data->input_dev->evbit);
	set_bit(KEY_POWER, data->input_dev->keybit);
	set_bit(EV_ABS, data->input_dev->evbit);
	input_set_abs_params(data->input_dev, ABS_DISTANCE, 0, 1, 0, 0);
	data->input_dev->name = "proximity";
//--[[ LGE_UBIQUIX_MODIFIED_START : shyun@ubiquix.com [2012.03.30] - Not used this member value is ICS
// 20101004 jh.koo@lge.com, fix initial operation of proximity sensor [START_LGE]
//	data->input_dev->abs[ABS_DISTANCE] = -1;
// 20101004 jh.koo@lge.com, fix initial operation of proximity sensor [END_LGE]
//--]] LGE_UBIQUIX_MODIFIED_END : shyun@ubiquix.com [2012.03.30]- Not used this member value is ICS
	ret = input_register_device(data->input_dev);
	if (ret) {
		printk(KERN_ERR "%s: Fail to register device\n", __FUNCTION__);
		goto ERROR1;
	}

	if ((ret = sysfs_create_group(&dev->kobj, &hub_proxi_group)))
		goto ERROR3;

#ifdef CONFIG_HAS_EARLYSUSPEND /* 20110304 seven.kim@lge.com late_resume_lcd [START] */ //justin LCD resume speed up
//--[[ LGE_UBIQUIX_MODIFIED_START : shyun@ubiquix.com [2012.03.30] - CONFIG_PRODUCT_LGE_LU6800 Featuring
#ifdef CONFIG_PRODUCT_LGE_LU6800
//--]] LGE_UBIQUIX_MODIFIED_END : shyun@ubiquix.com [2012.03.30]- CONFIG_PRODUCT_LGE_LU6800 Featuring

  data->early_suspend.level = 45;//BL=50, TS=49, TK=48
#else
	data->early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN - 1;
#endif
	data->early_suspend.suspend = hub_proxi_early_suspend;
	data->early_suspend.resume = hub_proxi_late_resume;
	register_early_suspend(&data->early_suspend);
#endif /* 20110304 seven.kim@lge.com late_resume_lcd [END] */

//--[[ LGE_UBIQUIX_MODIFIED_START : shyun@ubiquix.com [2011.10.05] - Modify the log.
	printk("[shyun] hub_proxi_probe [NORMAL OUT]\n");
//--]] LGE_UBIQUIX_MODIFIED_END : shyun@ubiquix.com [2011.10.05]- Modify the log.

	return 0;

ERROR3:
	input_unregister_device(data->input_dev);
	printk("[shyun] hub_proxi_probe ERROR3 [ERR OUT] [ret = %d]\n", ret);
ERROR1:
	printk("[shyun] hub_proxi_probe ERROR1 [ERR OUT] [ret = %d]\n", ret);
	kfree(data);

	return ret;
}

static int hub_proxi_remove(struct i2c_client *client)
{
	struct hub_proxi_data *data = i2c_get_clientdata(client);

	hub_proxi_disable(client);

	if (client->irq) {
		free_irq(client->irq, &client->dev);
		gpio_free(PROXI_OUT);
	}

	sysfs_remove_group(&client->dev.kobj, &hub_proxi_group);
	input_unregister_device(data->input_dev);
	kfree(data);
	i2c_set_clientdata(client, NULL);
/*LGE_CHANGE_S [kyw2029@lge.com] 2010-01-04, ldoc control*/
//--[[ LGE_UBIQUIX_MODIFIED_START : shyun@ubiquix.com [2011.08.02] - cd /sys/bus/i2c/devices/3-0044/
//--[[ LGE_UBIQUIX_MODIFIED_START : shyun@ubiquix.com [2012.03.30] - KU5900 / P970 Featuring
#if defined(CONFIG_MACH_LGE_HEAVEN_REV_A) || defined(CONFIG_PRODUCT_LGE_KU5900) || defined(CONFIG_PRODUCT_LGE_P970)/*defined(CONFIG_MACH_LGE_HUB)*/
//--]] LGE_UBIQUIX_MODIFIED_END : shyun@ubiquix.com [2012.03.30]- KU5900 / P970 Featuring
//--]] LGE_UBIQUIX_MODIFIED_END : shyun@ubiquix.com [2011.08.02]- cd /sys/bus/i2c/devices/3-0044/
	regulator_put(reg);
#endif
/*LGE_CHANGE_S [kyw2029@lge.com] 2010-01-04, ldoc control*/
	return 0;
}

static int hub_proxi_suspend(struct i2c_client *client, pm_message_t mesg)
{
	struct hub_proxi_data *data = i2c_get_clientdata(client);

	if (!enabled) return 0;
	printk("%s\n", __FUNCTION__);
	if (atomic_read(&proxi_status)) return 0;
#if 0
	if (hrtimer_try_to_cancel(&data->timer) > 0) //timer is active
		hrtimer_cancel(&data->timer);
#endif
	if (data->wakeup_while_sleep)
	{
		printk("%s() wakeup_while_sleep.\n", __FUNCTION__);
		//		enable_irq(client->irq);
	}	
	else
		hub_proxi_disable(client);
	return 0;
}

extern void omap_pm_cpu_set_freq(unsigned long f);

static int hub_proxi_resume(struct i2c_client *client)
{
	struct hub_proxi_data *data = i2c_get_clientdata(client);
//	printk("%s\n", __FUNCTION__);
	if (!enabled) return 0;

//	if (data->wakeup_while_sleep)
//		skip_far_event_flag = 1;//On wakelock sleep, resume func called

#if 0
	if (data->wakeup_while_sleep)
		disable_irq(client->irq);

	if (hrtimer_try_to_cancel(&data->timer) == 0) //timer is not active
		hrtimer_start(&data->timer, ktime_set(0, data->delay), HRTIMER_MODE_REL);
#endif
// 20101115 jh.koo@lge.com work queue after wake up [START_LGE]
	if(enabled)
		schedule_work(&data->work);
// 20101115 jh.koo@lge.com work queue after wake up [END_LGE]

	if (!data->wakeup_while_sleep)
		hub_proxi_enable(client);
	return 0;
}

#ifdef CONFIG_HAS_EARLYSUSPEND /* 20110304 seven.kim@lge.com late_resume_lcd [START] */
static void hub_proxi_early_suspend(struct early_suspend *h)
{
	//printk("%s\n", __func__);
	hub_proxi_suspend(hub_proximity_client, PMSG_SUSPEND);
}

static void hub_proxi_late_resume(struct early_suspend *h)
{
	//printk("%s\n", __func__);
	hub_proxi_resume(hub_proximity_client);
}
#endif /* 20110304 seven.kim@lge.com late_resume_lcd [END] */

static const struct i2c_device_id hub_proxi_ids[] = {

//--[[ LGE_UBIQUIX_MODIFIED_START : shyun@ubiquix.com [2012.03.30] - Featuring in ICS.
#if defined(CONFIG_PRODUCT_LGE_LU6800)
	{ "justin_proxi", 0 },
#elif defined(CONFIG_PRODUCT_LGE_KU5900) || defined(CONFIG_PRODUCT_LGE_P970)
	{ "black_proxi", 0 },
#else
#error "Please check the product definition(Proximity)"
#endif
//--]] LGE_UBIQUIX_MODIFIED_END : shyun@ubiquix.com [2012.03.30]- Featuring in ICS.

	{ /* end of list */ },
};

static struct i2c_driver hub_proxi_driver = {
	.probe	 = hub_proxi_probe,
	.remove	 = hub_proxi_remove,
#ifndef CONFIG_HAS_EARLYSUSPEND /* 20110304 seven.kim@lge.com late_resume_lcd [START] */
	.suspend = hub_proxi_suspend,
	.resume = hub_proxi_resume,
#endif /* 20110304 seven.kim@lge.com late_resume_lcd [END] */
	.id_table	= hub_proxi_ids,
	.driver	 = {

//--[[ LGE_UBIQUIX_MODIFIED_START : shyun@ubiquix.com [2012.03.30] - Featuring in ICS
#if defined(CONFIG_PRODUCT_LGE_LU6800)
		.name = "justin_proxi",
#elif defined(CONFIG_PRODUCT_LGE_KU5900) || defined(CONFIG_PRODUCT_LGE_P970)
		.name = "black_proxi",
#else
#error "Please check the product definition(Proximity)"
#endif
//--]] LGE_UBIQUIX_MODIFIED_END : shyun@ubiquix.com [2012.03.30]- Featuring in ICS

		.owner = THIS_MODULE,
	},
};

static int __init hub_proxi_init(void)
{
	int ret = -1;
	printk("[hub_proxi_init] [IN]\n");

	ret = i2c_add_driver(&hub_proxi_driver);

	printk("[hub_proxi_init] [OUT] [ret = %d]\n", ret);
	
	return ret; 
}

static void __exit hub_proxi_exit(void)
{
	i2c_del_driver(&hub_proxi_driver);
}

module_init(hub_proxi_init);
module_exit(hub_proxi_exit);

MODULE_AUTHOR("LG Electronics");
MODULE_DESCRIPTION("Justin Proximity Driver");
MODULE_LICENSE("GPL");

