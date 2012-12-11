#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/types.h>
//LGE_LU6800_S 20101215 kyungrae.jo@lge.com, JUSTIN porting
#include <linux/fb.h>
#include <linux/earlysuspend.h>
#include <linux/leds.h>
#include <linux/i2c.h>
//LGE_LU6800_E 20101215 kyungrae.jo@lge.com, JUSTIN porting
#include <mach/lm3528.h>

//LGE_LU6800_S 20101215 kyungrae.jo@lge.com, JUSTIN porting
#define LCD_CP_EN			149
#define MAX_BRIGHTNESS		0x7F	// MAX current, about ??? cd/m2
// sangki.hyun@lge.com default brightness change #define DEFAULT_BRIGHTNESS	0x69	// about 220 cd/m2
#define DEFAULT_BRIGHTNESS	0x65	// about 186.2 cd/m2

struct lm3528_device {
	struct i2c_client *client;
	struct backlight_device *bl_dev;
	struct led_classdev *led;

#ifdef CONFIG_HAS_EARLYSUSPEND
	struct early_suspend early_suspend;
#endif
};

static struct lm3528_device *main_lm3528_dev = NULL;
static struct i2c_client 	*lm3528_i2c_client;
unsigned int cur_main_lcd_level = DEFAULT_BRIGHTNESS;
#ifdef CONFIG_HAS_EARLYSUSPEND
static int early_bl_timer = 1;	//when screen off: 0, screen on: 1
static int early_bl_value = 0;
#endif
//LGE_LU6800_E 20101215 kyungrae.jo@lge.com, JUSTIN porting

//LGE_LU6800_S 20110419 kyungrae.jo@lge.com, Check CP_USB & No-Battery
int check_battery_present(void);

// kibum.lee@lge.com 20120502 MUIC re-work start
#if defined(CONFIG_MUIC)
int muic_get_mode();
#else
int get_muic_mode();
#endif
// kibum.lee@lge.com 20120502 MUIC re-work end

//LGE_LU6800_E 20110419 kyungrae.jo@lge.com, Check CP_USB & No-Battery

/* SYSFS for brightness control
 */

//LGE_LU6800_S 20110205 ntdeaewan.choi@lge.com, keyled & backlight flag
u8 backlight_keyled_flag=1;

/* S[, 20111110, mschung@ubiquix.com, Enhanced power consumption, at playing MP3. */
#if 0  // sangki.hyun@lge.com temp
extern u32 doing_wakeup;
EXPORT_SYMBOL(doing_wakeup);
#endif
/* E], 20111110, mschung@ubiquix.com, Enhanced power consumption, at playing MP3. */

//LGE_LU6800_E 20110205 ntdeaewan.choi@lge.com, keyled & backlight flag

#if 0 //remove compile warning
static ssize_t	brightness_show(struct device* dev,
								struct device_attribute* attr, char* buf)
{
	struct lm3528_platform_data*	pdata	=	dev->platform_data;
	int		val;

	if ((val = lm3528_get_bmain(&pdata->private)) < 0)
		return	0;

	return	snprintf(buf, PAGE_SIZE, "%d\n", val);
}

static ssize_t brightness_store(struct device* dev,
								struct device_attribute* attr,
								const char* buf, size_t count)
{
	struct lm3528_platform_data*	pdata	=	dev->platform_data;

	lm3528_set_bmain(&pdata->private, simple_strtol(buf, NULL, 10));

	return	count;
}

static DEVICE_ATTR(brightness, 0644, brightness_show, brightness_store);

/* SYSFS for LCD backlight ON/OFF
 */
static ssize_t	enable_show(struct device* dev,
								struct device_attribute* attr, char *buf)
{
	struct lm3528_platform_data*	pdata	=	dev->platform_data;
	int		val	=	lm3528_get_hwen(&pdata->private, pdata->gpio_hwen);

	return	snprintf(buf, PAGE_SIZE, "%d\n", val);
}

static ssize_t enable_store(struct device* dev,
								struct device_attribute* attr,
								const char* buf, size_t count)
{
	struct lm3528_platform_data*	pdata	=	dev->platform_data;

	lm3528_set_hwen(&pdata->private, pdata->gpio_hwen, (int)simple_strtol(buf, NULL, 10));

	return	count;
}

static DEVICE_ATTR(enable, 0644, enable_show, enable_store);
#endif

// LGE_JUSTIN_S 2011120 kyungrae.jo@lge.com, for TEST MODE

static void lm3528_set_main_current_level(struct i2c_client *client, int level);
static ssize_t lcd_backlight_store_on_off(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	int on_off;
	int ret;
	struct lm3528_platform_data*	pdata;
	struct i2c_client *client = to_i2c_client(dev);

	if (!count)
		return -EINVAL;

	printk("%s\n", __func__);

	on_off = simple_strtoul(buf, NULL, 10);
	sscanf(buf, "%d", &ret);
	pdata = client->dev.platform_data;

	printk(KERN_ERR "%d\n",on_off);

	if(ret == 0){
		lm3528_set_bmain(&pdata->private, 0);
	}else {
	    lm3528_set_bmain(&pdata->private, DEFAULT_BRIGHTNESS);
	}

	return count;

}

DEVICE_ATTR(backlight_on_off, 0664, NULL, lcd_backlight_store_on_off);

// LGE_JUSTIN_E 2011120 kyungrae.jo@lge.com, for TEST MODE



/* Driver
 */

//LGE_LU6800_S 20101215 kyungrae.jo@lge.com, JUSTIN porting, brightness control
static void lm3528_set_main_current_level(struct i2c_client *client, int level)
{
	struct lm3528_device *dev;
	struct lm3528_platform_data*	pdata;
	unsigned char muic_mode; //20110419 kyungrae.jo@lge.com, Check CP_USB & No-Battery

	dev = (struct lm3528_device *)i2c_get_clientdata(client);
	cur_main_lcd_level = level;
	dev->bl_dev->props.brightness = cur_main_lcd_level;

	//LGE_LU6800_S 20110419 kyungrae.jo@lge.com, Check CP_USB & No-Battery
// kibum.lee@lge.com 20120502 MUIC re-work start	
#if defined(CONFIG_MUIC)
	muic_mode = muic_get_mode();
#else
	muic_mode = get_muic_mode();
#endif
// kibum.lee@lge.com 20120502 MUIC re-work end
	if((muic_mode == 9 || muic_mode == 7) && (check_battery_present() == 0))
	{
		printk(KERN_INFO"CP_USB/UART & No-Battery -> Backlight level = 0x28(40)\n");
		level = 0x28; //2.892mA
	}
	//LGE_LU6800_E 20110419 kyungrae.jo@lge.com, Check CP_USB & No-Battery

	pdata = client->dev.platform_data;
	lm3528_set_bmain(&pdata->private, level);

	//LGE_LU6800_S 20110205 ntdeaewan.choi@lge.com, keyled & backlight flag
	if(level==0)	backlight_keyled_flag=0;
	else			backlight_keyled_flag=1;
	//LGE_LU6800_E 20110205 ntdeaewan.choi@lge.com, keyled & backlight flag
}
//LGE_LU6800_E 20101215 kyungrae.jo@lge.com, JUSTIN porting, brightness control

//LGE_LU6800_S 20101215 kyungrae.jo@lge.com, "Setting" => "Brightness control" use this function
static void leds_brightness_set(struct led_classdev *led_cdev, enum led_brightness value)
{
	struct lm3528_platform_data*	pdata;
	u8 level;
	pdata = lm3528_i2c_client->dev.platform_data;

	// value : 30 ~ 255
	// lm3528 brightness : 0 ~ 127
	//2011-03-11, Max:UI 255(driver 127) / Min:UI 30(driver 70)
	if(value >= 30)
		level = ((value - 30) * 57) / 225 + 70;
	else
		level = value*70/30;

	//printk("[lm3528]leds_brightness_set : %d, %d\n, ", value, level);
	if(level > MAX_BRIGHTNESS)
		level = MAX_BRIGHTNESS;
	else if (level < 0)
		level = 0;

	if(early_bl_timer == 0) {
		early_bl_value = level;
		return;
	}

	//lm3528_set_bmain(&pdata->private, value);
	lm3528_set_main_current_level(lm3528_i2c_client, level);
	cur_main_lcd_level = level;

	return;
}

static struct led_classdev lcd_backlight = {
	.name = "lcd-backlight",
	.brightness = MAX_BRIGHTNESS,
	.brightness_set = leds_brightness_set,
};
//LGE_LU6800_E 20101215 kyungrae.jo@lge.com, "Setting" => "Brightness control" use this function

//LGE_LU6800_S 20101215 kyungrae.jo@lge.com, JUSTIN porting
static int lm3528bl_set_brightness(struct backlight_device *bd)
{
	struct lm3528_platform_data*	pdata;

	printk("[lm3528]%s:%d\n",__func__,bd->props.brightness);

	pdata = lm3528_i2c_client->dev.platform_data;
	//lm3528_set_bmain(&pdata->private, bd->props.brightness);
	lm3528_set_main_current_level(lm3528_i2c_client, bd->props.brightness);
	cur_main_lcd_level = bd->props.brightness;

	return 0;
}

static int lm3528bl_get_brightness(struct backlight_device *bd)
{
	struct lm3528_platform_data*	pdata;
	unsigned char val=0;

	pdata = lm3528_i2c_client->dev.platform_data;
	val = lm3528_get_bmain(&pdata->private);

//	printk("[lm3528]%s:%d\n",__func__,val);

	return (int)val;
}

static struct backlight_ops lm3528bl_ops = {
	.update_status	= lm3528bl_set_brightness,
	.get_brightness	= lm3528bl_get_brightness,
};

static int lm3528_suspend(struct i2c_client *client, pm_message_t state)
{
#ifndef CONFIG_HAS_EARLYSUSPEND
	struct lm3528_platform_data*	pdata;
//	printk("[lm3528]%s\n",__func__);

	pdata = client->dev.platform_data;
	client->dev.power.power_state = state;
	//lm3528_set_hwen(client->dev.platform_data, LCD_CP_EN, 0);
	lm3528_set_bmain(&pdata->private, 0);
#endif
	return 0;
}

static int lm3528_resume(struct i2c_client *client)
{
#ifndef CONFIG_HAS_EARLYSUSPEND
	struct lm3528_platform_data*	pdata;
//	printk("[lm3528]%s\n",__func__);

	pdata = client->dev.platform_data;
	client->dev.power.power_state = PMSG_ON;
	//lm3528_set_hwen(client->dev.platform_data, LCD_CP_EN, 1);
	lm3528_set_bmain(&pdata->private, cur_main_lcd_level);
#endif
//	doing_wakeup = 0;
	return 0;
}
//LGE_LU6800_E 20101215 kyungrae.jo@lge.com, JUSTIN porting


int lcd_backlight_status = 1;
int lcd_status_backup = 1;
int lcd_cp_en_status = 1;
extern void synaptics_keytouch_power_on(void);
extern void synaptics_touch_power_on(void);
extern void synaptics_keytouch_power_off(void);
extern void synaptics_touch_power_off(void);

int lm3258_power_switch(int val)
{
	printk("HSK lm3258_power_switch_#1.\n");

	if(lcd_cp_en_status == val)
		return;
//	printk("[jslee] lm3258_power_switch_#2.(lcd_cp_en_status : %d, val : %d)\n", lcd_cp_en_status, val);
	if(val==0)
	{
		gpio_direction_output(149, 0);		// LCD_CP_EN

		/* S, 20110923, mschung@ubiquix.com, Moved from touchscreen driver(_synaptics_ts.c, _synaptics_keytouch.c). */
		synaptics_keytouch_power_off();
		synaptics_touch_power_off();
		/* E, 20110923, mschung@ubiquix.com, Moved from touchscreen driver(_synaptics_ts.c, _synaptics_keytouch.c). */

		lcd_cp_en_status = 0;
		printk("[lcd & touch power] OFF!\n");
	}
	else
	{
		/* S, 20110923, mschung@ubiquix.com, Moved from touchscreen driver(_synaptics_ts.c, _synaptics_keytouch.c). */
		synaptics_keytouch_power_on();
		synaptics_touch_power_on();
		/* E, 20110923, mschung@ubiquix.com, Moved from touchscreen driver(_synaptics_ts.c, _synaptics_keytouch.c). */

		gpio_direction_output(149, 1);		// LCD_CP_EN
		lcd_cp_en_status = 1;
		printk("[lcd & touch power] ON!\n");
		mdelay(50);
	}
	return 0;
}
// hycho for HDMI
void lm3528_set_backlight_for_hdmi(int onoff)
{
        struct lm3528_device *dev;
        struct lm3528_platform_data*    pdata;

	if(lm3528_i2c_client)
	        dev = (struct lm3528_device *)i2c_get_clientdata(lm3528_i2c_client);
	else
		return;
        printk("[lm3528]%s, onoff=%d, cur_level = %d\n",__func__, onoff, cur_main_lcd_level);
 
	if(onoff)
	{
		dev->bl_dev->props.brightness = cur_main_lcd_level;	

        	pdata = lm3528_i2c_client->dev.platform_data;
	        lm3528_set_bmain(&pdata->private, cur_main_lcd_level);
	}
	else
	{
		dev->bl_dev->props.brightness = 0;	

        	pdata = lm3528_i2c_client->dev.platform_data;
	        lm3528_set_bmain(&pdata->private, 0);
	} 
}

//LGE_LU6800_S 20101215 kyungrae.jo@lge.com, apply EARLYSUSPEND
#ifdef CONFIG_HAS_EARLYSUSPEND
static int lm3528bl_suspend(struct i2c_client *client)
{
	struct lm3528_platform_data*	pdata;
	pdata = client->dev.platform_data;

	printk("[lm3528]%s\n",__func__);

#if 0
	lm3528_set_hwen(&pdata->private, LCD_CP_EN, 0);	//problem: LCD timeout waiting TE
	//lm3528_set_bmain(&pdata->private, 0);
#else
	lm3528_set_main_current_level(client, 0);
#endif

    early_bl_timer = 0;
//jonghee00.lee
	if(lcd_status_backup == 0)
	{
		lm3258_power_switch(0);
	}
	lcd_backlight_status = 0;


	return 0;
}

/* S[, 20120922, mannsik.chung@lge.com, PM from froyo. */
#if defined(CONFIG_PRODUCT_LGE_LU6800)
extern u32 doing_wakeup;
#endif
/* E], 20120922, mannsik.chung@lge.com, PM from froyo. */

static int lm3528bl_resume(struct i2c_client *client)
{
	struct lm3528_platform_data*	pdata;
	pdata = client->dev.platform_data;

  mdelay(100);//NATTING_TEST : Justin LCD resume time °³¼±...

//	printk("[lm3528]%s\n",__func__);
//jonghee00.lee
		lm3258_power_switch(1);
		lm3528_set_hwen(&pdata->private, LCD_CP_EN, 1);


	    lcd_backlight_status = 1;

    early_bl_timer = 1;
#if 0
	lm3528_set_hwen(&pdata->private, LCD_CP_EN, 1);	//problem: LCD timeout waiting TE
	//lm3528_set_bmain(&pdata->private, early_bl_value);
#else
	lm3528_set_main_current_level(client, early_bl_value);
#endif

/* S[, 20120922, mannsik.chung@lge.com, PM from froyo. */
#if defined(CONFIG_PRODUCT_LGE_LU6800)
	doing_wakeup = 0;
#endif
/* E], 20120922, mannsik.chung@lge.com, PM from froyo. */

    return 0;
}

static void lm3528_early_suspend(struct early_suspend *h)
{
	struct lm3528_device *dev;
	dev = container_of(h, struct lm3528_device, early_suspend);
    lm3528bl_suspend(dev->client);
}

static void lm3528_late_resume(struct early_suspend *h)
{
	struct lm3528_device *dev;
	dev = container_of(h, struct lm3528_device, early_suspend);
	lm3528bl_resume(dev->client);
}

#endif//CONFIG_HAS_EARLYSUSPEND
//LGE_LU6800_E 20101215 kyungrae.jo@lge.com, apply EARLYSUSPEND

static int __init lm3528bl_probe(struct i2c_client* client,
							const struct i2c_device_id* id)
{
	printk("[jslee] lm3528_bl.c - lm3528bl_probe start.\n");

	//LGE_LU6800_S 20101215 kyungrae.jo@lge.com, JUSTIN porting
	struct lm3528_device	*dev;
	struct backlight_device *bl_dev;
	//LGE_LU6800_E 20101215 kyungrae.jo@lge.com, JUSTIN porting
	struct lm3528_platform_data*	pdata;
	int	error;

	struct backlight_properties props; // 20110617 added by  jslee@ubiquix.com 

	//LGE_LU6800_S 20101215 kyungrae.jo@lge.com, JUSTIN porting
	lm3528_i2c_client = client;

	dev = kzalloc(sizeof(struct lm3528_device), GFP_KERNEL);

	if (dev == NULL) {
		dev_err(&client->dev, "fail alloc for lm3528_device\n");
		return 0;
	}
	printk("[jslee] lm3528_bl.c - lm3528bl_probe_#1.\n");
	main_lm3528_dev = dev;
#if 0	// froyo
	bl_dev = backlight_device_register(LM3528_I2C_NAME, &client->dev, NULL, &lm3528bl_ops);
#else
#if 1  // sghyun_justin
		memset(&props, 0, sizeof(struct backlight_properties));
		props.max_brightness = MAX_BRIGHTNESS;
		props.brightness = DEFAULT_BRIGHTNESS;
		props.power = FB_BLANK_UNBLANK;
		props.type = BACKLIGHT_RAW;
#endif
	bl_dev = backlight_device_register(LM3528_I2C_NAME, &client->dev, NULL, &lm3528bl_ops, &props);
#endif
	bl_dev->props.max_brightness = MAX_BRIGHTNESS;
	bl_dev->props.brightness = DEFAULT_BRIGHTNESS;
	bl_dev->props.power = FB_BLANK_UNBLANK;

	dev->bl_dev = bl_dev;
	dev->client = client;
	i2c_set_clientdata(client, dev);
	//LGE_LU6800_E 20101215 kyungrae.jo@lge.com, JUSTIN porting

	pdata	=	client->dev.platform_data;
	gpio_request(LCD_CP_EN, "backlight_enable");
	//gpio_direction_output(pdata->gpio_hwen, 1);	// OUTPUT
	lm3528_init(&pdata->private, client);

	//LGE_LU6800_S 20101215 kyungrae.jo@lge.com, JUSTIN porting
	led_classdev_register(&client->dev, &lcd_backlight);	//UI control

	lm3528_set_bmain(&pdata->private, DEFAULT_BRIGHTNESS);

	//device_create_file(&client->dev, &dev_attr_enable);
	//device_create_file(&client->dev, &dev_attr_brightness);
	// LGE_JUSTIN_S 2011120 kyungrae.jo@lge.com, for TEST MODE
	error = device_create_file(&client->dev, &dev_attr_backlight_on_off);
	// LGE_JUSTIN_E 2011120 kyungrae.jo@lge.com, for TEST MODE


#ifdef CONFIG_HAS_EARLYSUSPEND
	dev->early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN;
	dev->early_suspend.suspend = lm3528_early_suspend;
	dev->early_suspend.resume = lm3528_late_resume;
	register_early_suspend(&dev->early_suspend);
#endif

	//LGE_LU6800_E 20101215 kyungrae.jo@lge.com, JUSTIN porting
	printk("[jslee] lm3528_bl.c - lm3528bl_probe end.\n");
	return	0;
}

static int	lm3528bl_remove(struct i2c_client* client)
{
	gpio_free(LCD_CP_EN);	
	return	0;
}

static const struct i2c_device_id lm3528bl_ids[] = {
	{	LM3528_I2C_NAME, 0 },	// LM3528
	{},
};

static struct i2c_driver lm3528bl_driver = {
	.probe		=	lm3528bl_probe,
	.remove		=	lm3528bl_remove,
	//LGE_LU6800_S 20101215 kyungrae.jo@lge.com, JUSTIN porting
	.suspend	=	lm3528_suspend,
	.resume		=	lm3528_resume,
	//LGE_LU6800_E 20101215 kyungrae.jo@lge.com, JUSTIN porting
	.id_table	=	lm3528bl_ids,
	.driver = {
		.name	=	LM3528_I2C_NAME,
		.owner	=	THIS_MODULE,
	},
};

static int __init lm3528bl_init(void)
{
	return	i2c_add_driver(&lm3528bl_driver);
}

static void __exit lm3528bl_exit(void)
{
	i2c_del_driver(&lm3528bl_driver);
}

module_init(lm3528bl_init);
module_exit(lm3528bl_exit);

MODULE_AUTHOR("LG Electronics");
MODULE_DESCRIPTION("Backlight driver (LM3528)");
MODULE_LICENSE("GPL");
