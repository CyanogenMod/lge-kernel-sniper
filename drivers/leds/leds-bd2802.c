/*
 * leds-bd2802.c - RGB LED Driver
 *
 * Copyright (C) 2009 Samsung Electronics
 * Kim Kyuwon <q1.kim@samsung.com>
 * Kim Kyungyoon <kyungyoon.kim@lge.com> modified
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Datasheet: http://www.rohm.com/products/databook/driver/pdf/bd2802gu-e.pdf
 *
 */

#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/leds.h>
#include <linux/hrtimer.h>
#include <linux/slab.h>

#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif

#define MODULE_NAME   "led-bd2802"

#ifndef DEBUG
//#define DEBUG
//#undef DEBUG
#endif

#define BLINK_ON_BOOTING

#define BDCMGB_SEQ_BLINK_ON


#ifdef DEBUG
#define DBG(fmt, args...) 				\
	printk(KERN_DEBUG "[%s] %s(%d): " 		\
		fmt, MODULE_NAME, __func__, __LINE__, ## args); 
#else	/* DEBUG */
#define DBG(...) 
#endif

#define LED_CTL(rgb2en, rgb1en) ((rgb2en) << 4 | ((rgb1en) << 0))

#define RGB_LED_CNTL 			128

#define BD2802_LED_OFFSET		0xa
#define BD2802_COLOR_OFFSET		0x3

#define BD2802_REG_CLKSETUP 		0x00
#define BD2802_REG_CONTROL 		0x01
#define BD2802_REG_HOURSETUP		0x02
#define BD2802_REG_CURRENT1SETUP	0x03
#define BD2802_REG_CURRENT2SETUP	0x04
#define BD2802_REG_WAVEPATTERN		0x05

#define BD2812_DCDCDRIVER		0x40
#define BD2812_PIN_FUNC_SETUP		0x41

#define BD2802_CURRENT_WHITE_PEAK	0x5A /* 18mA */
#define BD2802_CURRENT_WHITE_MAX		0x2D /*9mA*///	0x32 /* 10mA */
#define BD2802_CURRENT_BLUE_MAX		0x32 /* 10mA */
#define BD2802_CURRENT_WHITE_MIN	0x05/*1mA*/
#define BD2802_CURRENT_BLUE_MIN		0x05 /* 1mA */
#define BD2802_CURRENT_000		0x00 /* 0.0mA */

#define BD2802_PATTERN_FULL		0x0F
#define BD2802_PATTERN_HALF		0x09
#define BD2802_TIME_SETUP		0xF3

enum led_ids {
	LED1,
	LED2,
	LED_NUM,
};

enum led_colors {
	RED,
	GREEN,
	BLUE,
	WHITE,
};

enum key_leds {
	MENU,
	HOME,
	BACK,
	SEARCH,
	ALL,
	HIDDEN1,
	HIDDEN2,
};

enum led_direction {
	FORWARD,
	BACKWARD,
};

enum led_bits {
	BD2802_OFF,
	BD2802_BLINK,
	BD2802_ON,
	BD2802_DIMMING,
	BD2802_TEST_ON,
	BD2802_TEST_OFF,
	BD2802_SEQ,
	BD2802_SEQ_END,
	BD2802_SYNC,
};

struct bd2802_led {
	struct i2c_client		*client;
	struct rw_semaphore		rwsem;

	struct hrtimer timer;
	struct work_struct	work;
	struct workqueue_struct	*bd2802_wq;	

	struct hrtimer		touchkey_timer;	
	struct work_struct	touchkey_work;
	struct workqueue_struct	*touchkey_wq;

	struct hrtimer		ledmin_timer;	
	struct work_struct	ledmin_work;
	struct workqueue_struct	*ledmin_wq;
	/*
	 * Making led_classdev as array is not recommended, because array
	 * members prevent using 'container_of' macro. So repetitive works
	 * are needed.
	 */

	/*
	 * Advanced Configuration Function(ADF) mode:
	 * In ADF mode, user can set registers of BD2802GU directly,
	 * therefore BD2802GU doesn't enter reset state.
	 */
	enum led_ids			led_id;
	enum led_colors			color;
//	enum led_bits		state;
	enum led_bits		led_state;
	enum key_leds		key_led;
	enum led_direction  	key_direction;

	/* General attributes of RGB LEDs */
	int				wave_pattern;
	int			white_current;
	int			blue_current;
	int 			blink_enable;
	u8 			register_value[23];
//	int 			led_state;
	int			led_counter;
#ifdef BDCMGB_SEQ_BLINK_ON
	int			led_iskeystopdisable;
	int 			led_issuspend;
	int			led_isresumekeypressed;
	//int			led_isboot;
#endif
#ifdef CONFIG_HAS_EARLYSUSPEND
	struct early_suspend early_suspend; 
#endif
};

static struct i2c_client *bd2802_i2c_client;
static int led_isboot;

/*--------------------------------------------------------------*/
/*	BD2802GU helper functions					*/
/*--------------------------------------------------------------*/

static inline u8 bd2802_get_base_offset(enum led_ids id, enum led_colors color)
{
	return id * BD2802_LED_OFFSET + color * BD2802_COLOR_OFFSET;
}

static inline u8 bd2802_get_reg_addr(enum led_ids id, enum led_colors color,
								u8 reg_offset)
{
	return reg_offset + bd2802_get_base_offset(id, color);
}


/*--------------------------------------------------------------*/
/*	BD2802GU core functions					*/
/*--------------------------------------------------------------*/

static int bd2802_write_byte(struct i2c_client *client, u8 reg, u8 val)
{
	struct bd2802_led *led = i2c_get_clientdata(client);
	int ret=0;
	int reg_add=(int)(reg);
#ifndef BDCMGB_SEQ_BLINK_ON
	if (led->led_state == BD2802_OFF)
	{
		DBG("Only data write and 'LED on' are allowed\n");
		return 0;
	}
#endif
	ret = i2c_smbus_write_byte_data(client, reg, val);
	
	if (ret >= 0)
	{
		led->register_value[reg_add]=val;
		DBG("address = %d value=%d \n", reg,val);
		return 0;
	}

	dev_err(&client->dev, "%s: reg 0x%x, val 0x%x, err %d\n",
						__func__, reg, val, ret);

	return ret;
}

static void bd2802_sw_reset(struct bd2802_led *led)
{
	bd2802_write_byte(led->client, BD2802_REG_CLKSETUP, 0x01);
}

static void bd2802_configure(struct bd2802_led *led)
{
	u8 reg;
#ifdef BDCMGB_SEQ_BLINK_ON
	u8 regvalue = BD2802_TIME_SETUP;

	if(led->led_state == BD2802_SEQ)
	{
		if(led->led_iskeystopdisable)
		{
			regvalue = 0xA5;//0x55;
		}
		else
		{
			if(/*led->*/led_isboot)
				regvalue = 0xF4;
		}
	}

	reg = bd2802_get_reg_addr(LED1, RED, BD2802_REG_HOURSETUP);
	bd2802_write_byte(led->client, reg, regvalue);
	reg = bd2802_get_reg_addr(LED2, RED, BD2802_REG_HOURSETUP);
	bd2802_write_byte(led->client, reg, regvalue);
#else
	reg = bd2802_get_reg_addr(LED1, RED, BD2802_REG_HOURSETUP);
	bd2802_write_byte(led->client, reg, BD2802_TIME_SETUP);
	reg = bd2802_get_reg_addr(LED2, RED, BD2802_REG_HOURSETUP);
	bd2802_write_byte(led->client, reg, BD2802_TIME_SETUP);
#endif
	//bd2802_write_byte(led->client, BD2812_DCDCDRIVER, 0x00); 
	bd2802_write_byte(led->client, BD2812_PIN_FUNC_SETUP, 0x0F);
}

#ifdef BDCMGB_SEQ_BLINK_ON
static bool bd2802_check_colororder(struct bd2802_led *led, int color)
{
	if( (led->led_iskeystopdisable == true) && (color == 0/*white*/) )
		return true;

	return false;
}
#endif

static void bd2802_reset(struct bd2802_led *led)
{
	gpio_set_value(RGB_LED_CNTL, 0);
#ifndef BDCMGB_SEQ_BLINK_ON
	udelay(100);
#else
	udelay(10);
#endif
}

static void bd2802_reset_cancel(struct bd2802_led *led)
{
	gpio_set_value(RGB_LED_CNTL, 1);
#ifndef BDCMGB_SEQ_BLINK_ON
	udelay(150);
#else
	udelay(10);
#endif
	bd2802_configure(led);
}

static void bd2802_enable(struct bd2802_led *led)
{
	bd2802_write_byte(led->client, BD2802_REG_CONTROL, 0x11);
}

static void bd2802_turn_white(struct bd2802_led *led, enum key_leds id)
{
	u8 reg;
	u8 wavepattern = 0x04; //BDCMGB_SEQ_BLINK_ON
#ifdef BDCMGB_SEQ_BLINK_ON
	if(led->led_state == BD2802_SEQ && /*led->*/led_isboot)
		wavepattern = 0x00;
#endif

	if (led->blink_enable)
	{
		switch (id)
		{
			case MENU:
				reg = bd2802_get_reg_addr(LED1, GREEN, BD2802_REG_CURRENT2SETUP);
				bd2802_write_byte(led->client, reg, BD2802_CURRENT_000);
				reg = bd2802_get_reg_addr(LED1, GREEN, BD2802_REG_WAVEPATTERN);
				bd2802_write_byte(led->client, reg, wavepattern); //BDCMGB_SEQ_BLINK_ON
				break;
			case HOME:
				reg = bd2802_get_reg_addr(LED2, RED, BD2802_REG_CURRENT2SETUP);
				bd2802_write_byte(led->client, reg, BD2802_CURRENT_000);
				reg = bd2802_get_reg_addr(LED2, RED, BD2802_REG_WAVEPATTERN);
				bd2802_write_byte(led->client, reg, wavepattern); //BDCMGB_SEQ_BLINK_ON
				break;
			case BACK:
				reg = bd2802_get_reg_addr(LED2, GREEN, BD2802_REG_CURRENT2SETUP);
				bd2802_write_byte(led->client, reg, BD2802_CURRENT_000);
				reg = bd2802_get_reg_addr(LED2, GREEN, BD2802_REG_WAVEPATTERN);
				bd2802_write_byte(led->client, reg, wavepattern); //BDCMGB_SEQ_BLINK_ON
				break;
			case SEARCH:
				reg = bd2802_get_reg_addr(LED1, RED, BD2802_REG_CURRENT2SETUP);
				bd2802_write_byte(led->client, reg, BD2802_CURRENT_000);
				reg = bd2802_get_reg_addr(LED1, RED, BD2802_REG_WAVEPATTERN);
				bd2802_write_byte(led->client, reg, wavepattern); //BDCMGB_SEQ_BLINK_ON
				break;
			case HIDDEN1:
				reg = bd2802_get_reg_addr(LED1, BLUE, BD2802_REG_CURRENT1SETUP);
				bd2802_write_byte(led->client, reg, BD2802_CURRENT_000);
				reg = bd2802_get_reg_addr(LED1, BLUE, BD2802_REG_WAVEPATTERN);
				bd2802_write_byte(led->client, reg, 0x07);
				break;
				break;
			case HIDDEN2:
				reg = bd2802_get_reg_addr(LED2, BLUE, BD2802_REG_CURRENT1SETUP);
				bd2802_write_byte(led->client, reg, BD2802_CURRENT_000);
				reg = bd2802_get_reg_addr(LED2, BLUE, BD2802_REG_WAVEPATTERN);
				bd2802_write_byte(led->client, reg, 0x07);
				break;
			default:
				break;
			}
		}
	else
	{
		switch (id)
		{
			case MENU:
				reg = bd2802_get_reg_addr(LED1, GREEN, BD2802_REG_CURRENT1SETUP);
				bd2802_write_byte(led->client, reg, led->white_current);
				reg = bd2802_get_reg_addr(LED1, GREEN, BD2802_REG_CURRENT2SETUP);
				bd2802_write_byte(led->client, reg, led->white_current);
				break;
			case HOME:
				reg = bd2802_get_reg_addr(LED2, RED, BD2802_REG_CURRENT1SETUP);
				bd2802_write_byte(led->client, reg, led->white_current);
				reg = bd2802_get_reg_addr(LED2, RED, BD2802_REG_CURRENT2SETUP);
				bd2802_write_byte(led->client, reg, led->white_current);
				break;
			case BACK:
				reg = bd2802_get_reg_addr(LED2, GREEN, BD2802_REG_CURRENT1SETUP);
				bd2802_write_byte(led->client, reg, led->white_current);
				reg = bd2802_get_reg_addr(LED2, GREEN, BD2802_REG_CURRENT2SETUP);
				bd2802_write_byte(led->client, reg, led->white_current);
				break;
			case SEARCH:
				reg = bd2802_get_reg_addr(LED1, RED, BD2802_REG_CURRENT1SETUP);
				bd2802_write_byte(led->client, reg, led->white_current);
				reg = bd2802_get_reg_addr(LED1, RED, BD2802_REG_CURRENT2SETUP);
				bd2802_write_byte(led->client, reg, led->white_current);
				break;
			case HIDDEN1://BLUE1
				reg = bd2802_get_reg_addr(LED1, BLUE, BD2802_REG_CURRENT1SETUP);
				bd2802_write_byte(led->client, reg, led->blue_current);
				reg = bd2802_get_reg_addr(LED1, BLUE, BD2802_REG_CURRENT2SETUP);
				bd2802_write_byte(led->client, reg, led->blue_current);
				break;
			case HIDDEN2://BLUE2
				reg = bd2802_get_reg_addr(LED2, BLUE, BD2802_REG_CURRENT1SETUP);
				bd2802_write_byte(led->client, reg, led->blue_current);
				reg = bd2802_get_reg_addr(LED2, BLUE, BD2802_REG_CURRENT2SETUP);
				bd2802_write_byte(led->client, reg, led->blue_current);
				break;
			default:
				break;
		}
	}
}

static void bd2802_turn_blue(struct bd2802_led *led, enum key_leds id)
{
	u8 reg;

	switch (id)
	{
		case MENU:
			reg = bd2802_get_reg_addr(LED1, GREEN, BD2802_REG_CURRENT1SETUP);
			bd2802_write_byte(led->client, reg, BD2802_CURRENT_000);
			reg = bd2802_get_reg_addr(LED1, GREEN, BD2802_REG_CURRENT2SETUP);
			bd2802_write_byte(led->client, reg, BD2802_CURRENT_000);
			break;
		case HOME:
			reg = bd2802_get_reg_addr(LED2, RED, BD2802_REG_CURRENT1SETUP);
			bd2802_write_byte(led->client, reg, BD2802_CURRENT_000);
			reg = bd2802_get_reg_addr(LED2, RED, BD2802_REG_CURRENT2SETUP);
			bd2802_write_byte(led->client, reg, BD2802_CURRENT_000);
			break;
		case BACK:
			reg = bd2802_get_reg_addr(LED2, GREEN, BD2802_REG_CURRENT1SETUP);
			bd2802_write_byte(led->client, reg, BD2802_CURRENT_000);
			reg = bd2802_get_reg_addr(LED2, GREEN, BD2802_REG_CURRENT2SETUP);
			bd2802_write_byte(led->client, reg, BD2802_CURRENT_000);
			break;
		case SEARCH:
			reg = bd2802_get_reg_addr(LED1, RED, BD2802_REG_CURRENT1SETUP);
			bd2802_write_byte(led->client, reg, BD2802_CURRENT_000);
			reg = bd2802_get_reg_addr(LED1, RED, BD2802_REG_CURRENT2SETUP);
			bd2802_write_byte(led->client, reg, BD2802_CURRENT_000);
			break;
		case HIDDEN1:
			reg = bd2802_get_reg_addr(LED1, BLUE, BD2802_REG_CURRENT2SETUP);
			bd2802_write_byte(led->client, reg, led->blue_current);
#ifndef BDCMGB_SEQ_BLINK_ON
			reg = bd2802_get_reg_addr(LED1, BLUE, BD2802_REG_CURRENT1SETUP);
			bd2802_write_byte(led->client, reg, led->blue_current);
#else
			if (led->blink_enable)
			{
				reg = bd2802_get_reg_addr(LED1, BLUE, BD2802_REG_WAVEPATTERN);
				bd2802_write_byte(led->client, reg, 0x0b/*0x00*/);
			}
			else
			{
				reg = bd2802_get_reg_addr(LED1, BLUE, BD2802_REG_CURRENT1SETUP);
				bd2802_write_byte(led->client, reg, led->blue_current);
			}
#endif
			break;
		case HIDDEN2:
			reg = bd2802_get_reg_addr(LED2, BLUE, BD2802_REG_CURRENT2SETUP);
			bd2802_write_byte(led->client, reg, led->blue_current);
#ifndef BDCMGB_SEQ_BLINK_ON
			reg = bd2802_get_reg_addr(LED2, BLUE, BD2802_REG_CURRENT1SETUP);
			bd2802_write_byte(led->client, reg, led->blue_current);
#else
			if (led->blink_enable)
			{
				reg = bd2802_get_reg_addr(LED2, BLUE, BD2802_REG_WAVEPATTERN);
				bd2802_write_byte(led->client, reg, 0x0b/*0x00*/);
			}
			else
			{
				reg = bd2802_get_reg_addr(LED2, BLUE, BD2802_REG_CURRENT1SETUP);
				bd2802_write_byte(led->client, reg, led->blue_current);
			}
#endif
			break;
		default:
			break;
	}
}

static void bd2802_on(struct bd2802_led *led)
{
	bd2802_turn_white(led, MENU);
	bd2802_turn_white(led, HOME);
	bd2802_turn_white(led, BACK);
	bd2802_turn_white(led, SEARCH);
	bd2802_turn_white(led, HIDDEN1);
	bd2802_turn_white(led, HIDDEN2);
	//HARDLINE	bd2802_enable(led);
}

#ifdef BDCMGB_SEQ_BLINK_ON
static void bd2802_on_blue(struct bd2802_led *led)
{
	bd2802_turn_blue(led, MENU);
	bd2802_turn_blue(led, HOME);
	bd2802_turn_blue(led, BACK);
	bd2802_turn_blue(led, SEARCH);
	bd2802_turn_blue(led, HIDDEN1);
	bd2802_turn_blue(led, HIDDEN2);
}
#endif

void resumekey_pressed(void) //power, gesture
{
#ifdef BDCMGB_SEQ_BLINK_ON
	struct bd2802_led *led = i2c_get_clientdata(bd2802_i2c_client);
	
	led->led_isresumekeypressed = 1;
#endif	
}
EXPORT_SYMBOL(resumekey_pressed);

void touchkey_pressed(enum key_leds id)
{
	struct bd2802_led *led = i2c_get_clientdata(bd2802_i2c_client);

#ifndef BDCMGB_SEQ_BLINK_ON
	if (led->led_state==BD2802_SEQ ||
			led->led_state==BD2802_SYNC)
		return;
#else
	if(led->led_state == BD2802_SYNC)
		return;

	//if(led->led_isboot == 0)//if((led->blink_enable == 1) && (led->led_state == BD2802_ON))
	if(led->led_state == BD2802_BLINK) //power on/off
		return;

	if(led->led_state == BD2802_SEQ)
	{
		if(led->led_iskeystopdisable == 0)
			led->led_state=BD2802_ON;
		led->white_current = BD2802_CURRENT_WHITE_MAX;
		led->blue_current = BD2802_CURRENT_000;
		led->blink_enable=0;
		bd2802_sw_reset(led);
		bd2802_reset_cancel(led);
		bd2802_on(led);
		bd2802_enable(led);
	}
#endif //BDCMGB_SEQ_BLINK_ON

	hrtimer_cancel(&led->touchkey_timer);
	hrtimer_cancel(&led->ledmin_timer);

	if (led->led_state == BD2802_DIMMING) {
		led->white_current = BD2802_CURRENT_WHITE_MAX;
		led->blue_current = BD2802_CURRENT_000;	
		bd2802_on(led);
		led->led_state = BD2802_ON;
	}

	if (led->key_led != id)
		bd2802_turn_white(led,led->key_led);

	led->key_led=id;
	DBG("led->key_led =%d\n",led->key_led);

	led->blue_current = BD2802_CURRENT_BLUE_MAX;
	bd2802_turn_blue(led, led->key_led);
	bd2802_turn_blue(led, HIDDEN1);
	bd2802_turn_blue(led, HIDDEN2);

	hrtimer_start(&led->touchkey_timer, ktime_set(0, 200000000), HRTIMER_MODE_REL);
}
EXPORT_SYMBOL(touchkey_pressed);

static void bd2802_off(struct bd2802_led *led)
{
	bd2802_write_byte(led->client, BD2802_REG_CONTROL, 0x00);
}

static void bd2802_work_func(struct work_struct *work)
{
	struct bd2802_led *led = container_of(work, struct bd2802_led, work);

	DBG("led->led_state=%d\n",led->led_state);

	if (led->led_state == BD2802_SEQ_END) {	
		bd2802_turn_white(led,MENU);
		bd2802_turn_white(led,HOME);
		bd2802_turn_white(led,BACK);
		bd2802_turn_white(led,SEARCH);
		led->key_led = ALL;
		led->key_direction = FORWARD;
		led->led_state = BD2802_ON;	
		return;
	}

	if (led->led_state!=BD2802_SEQ)
		return;

	switch(led->key_led) {
		case ALL:
			bd2802_turn_blue(led,MENU);
			led->key_led=MENU;
			break;
		case MENU:
			bd2802_turn_white(led, MENU);
			bd2802_turn_blue(led,HOME);
			led->key_led=HOME;
			led->key_direction=FORWARD;
			break;
		case HOME:
			bd2802_turn_white(led, HOME);
			if (led->key_direction==FORWARD) {
				bd2802_turn_blue(led,BACK);
				led->key_led=BACK;
			}
			else
			{
				bd2802_turn_blue(led,MENU);
				led->key_led=MENU;						
			}
			break;
		case BACK:
			bd2802_turn_white(led, BACK);
			if (led->key_direction==FORWARD)
			{
				bd2802_turn_blue(led,SEARCH);
				led->key_led=SEARCH;
			}
			else
			{
				bd2802_turn_blue(led,HOME);
				led->key_led=HOME;					
			}
			break;
		case SEARCH:
			bd2802_turn_white(led, SEARCH);
			bd2802_turn_blue(led,BACK);
			led->key_led=BACK;
			led->key_direction=BACKWARD;
			break;
		case HIDDEN1:
		case HIDDEN2:
			break;
	}
		led->led_counter++;
}

static enum hrtimer_restart bd2802_timer_func(struct hrtimer *timer)
{
	struct bd2802_led *led = container_of(timer, struct bd2802_led, timer);

	DBG("led->led_counter=%d\n",led->led_counter);

	queue_work(led->bd2802_wq, &led->work);

	if (led->led_state==BD2802_SEQ)
	{
		if (((led->led_counter)%9)==0)
		//if(((led->key_led==HOME)&&(led->key_direction==BACKWARD))||((led->key_led==BACK)&&(led->key_direction==FORWARD)))
			hrtimer_start(&led->timer, ktime_set(1,000000000), HRTIMER_MODE_REL); /* 1 sec */
		else
			hrtimer_start(&led->timer, ktime_set(0, 110000000), HRTIMER_MODE_REL); /* 1 sec */
	}

	return HRTIMER_NORESTART;
}

static void bd2802_touchkey_work_func(struct work_struct *work)
{
	struct bd2802_led *led = container_of(work, struct bd2802_led, touchkey_work);

	led->white_current = BD2802_CURRENT_WHITE_MAX;
	led->blue_current = BD2802_CURRENT_000;
	bd2802_turn_white(led,led->key_led);
	bd2802_turn_blue(led,HIDDEN1);
	bd2802_turn_blue(led,HIDDEN2);
#ifndef BDCMGB_SEQ_BLINK_ON
	hrtimer_start(&led->ledmin_timer, ktime_set(5, 0), HRTIMER_MODE_REL);
#else
/*
	if(led->led_state == BD2802_SEQ)
		hrtimer_start(&led->ledmin_timer, ktime_set(2, 0), HRTIMER_MODE_REL);
	else
*/
	hrtimer_start(&led->ledmin_timer, ktime_set(4, 0), HRTIMER_MODE_REL);
#endif
}

static enum hrtimer_restart bd2802_touchkey_timer_func(struct hrtimer *timer)
{
	struct bd2802_led *led = container_of(timer, struct bd2802_led, touchkey_timer);

	DBG("\n");

	queue_work(led->bd2802_wq, &led->touchkey_work);

	return HRTIMER_NORESTART;
}

static void bd2802_ledmin_work_func(struct work_struct *work)
{
	struct bd2802_led *led = container_of(work, struct bd2802_led, ledmin_work);

#ifdef BDCMGB_SEQ_BLINK_ON
	//if( (led->led_state == BD2802_SEQ) && (led->led_iskeystopdisable == 1) )
	if( led->led_state == BD2802_SEQ)
	{
		if(led->led_iskeystopdisable == 1)
		{
			led->led_state =BD2802_SEQ;
			led->blue_current = BD2802_CURRENT_BLUE_MAX;
			led->white_current = BD2802_CURRENT_000;
			led->blink_enable = 1;
	bd2802_configure(led);
			bd2802_on_blue(led);
			bd2802_enable(led);	
		}
		return;
	}
#endif
	led->white_current = BD2802_CURRENT_WHITE_MIN;
	led->blue_current = BD2802_CURRENT_000;
	bd2802_on(led);
	led->led_state = BD2802_DIMMING;
}

static enum hrtimer_restart bd2802_ledmin_timer_func(struct hrtimer *timer)
{
	struct bd2802_led *led = container_of(timer, struct bd2802_led, ledmin_timer);

	DBG("\n");

	queue_work(led->bd2802_wq, &led->ledmin_work);
	return HRTIMER_NORESTART;
}

static void bd2802_blink_enable(struct bd2802_led *led)
{

	if (led->led_state == BD2802_OFF)
		return;

	bd2802_on(led);
}

static ssize_t bd2802_show_blink_enable(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct bd2802_led *led = i2c_get_clientdata(to_i2c_client(dev));

	int r;
	
	r = snprintf(buf, PAGE_SIZE,
			"%d\n", led->blink_enable);

	return r;
}

static ssize_t bd2802_store_blink_enable(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct bd2802_led *led = i2c_get_clientdata(to_i2c_client(dev));
	int value;
	
	if (!count)
		return -EINVAL;

	value = simple_strtoul(buf, NULL, 10);

//LGE_UPDATE
//	return count;
//LGE_UPDATE
#ifndef BDCMGB_SEQ_BLINK_ON
	if (led->blink_enable==value)
	{
		return count;
	}
	
	if (led->led_state==BD2802_SEQ)
		return count;
#else
//	if(led->led_state == BD2802_SEQ && led->led_iskeystopdisable)
//		return 0;
	if((value&0xA) == 0xA)
	{
		/*led->*/led_isboot = 1;
		value &= 0x1; 
	}
	led->led_state = (value == 1)? BD2802_BLINK: BD2802_ON;
#endif
	led->blink_enable=value;

	down_write(&led->rwsem);

#ifdef BDCMGB_SEQ_BLINK_ON
	if(led->blink_enable == 0)
	{
		led->white_current = BD2802_CURRENT_WHITE_MAX;
		//led->blue_current = BD2802_CURRENT_000;
		
		bd2802_sw_reset(led);
		bd2802_reset_cancel(led);
	}
#endif

	bd2802_blink_enable(led);
	bd2802_enable(led);

	up_write(&led->rwsem);

#ifdef BDCMGB_SEQ_BLINK_ON
	if(led->blink_enable == 0)
	{
		hrtimer_cancel(&led->ledmin_timer);
		hrtimer_start(&led->ledmin_timer, ktime_set(5, 0), HRTIMER_MODE_REL);
	}
#endif
	DBG("blink_enable = %d\n", led->blink_enable);
	return count;
}

static struct device_attribute bd2802_blink_enable_attr = {
	.attr = {
		.name = "blink_enable",
		.mode = 0644,
		.owner = THIS_MODULE
	},
	.show = bd2802_show_blink_enable,
	.store = bd2802_store_blink_enable,
};

static ssize_t bd2802_show_led_start(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct bd2802_led *led = i2c_get_clientdata(to_i2c_client(dev));

	if(led->led_state == BD2802_SEQ)
	{
		if(led->led_iskeystopdisable == 1)
			return snprintf(buf, PAGE_SIZE, "11\n");
		else
			return snprintf(buf, PAGE_SIZE, "10\n");
	}
	else
	{
		return snprintf(buf, PAGE_SIZE, "0\n");
	}
}

static ssize_t bd2802_store_led_start(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct bd2802_led *led = i2c_get_clientdata(to_i2c_client(dev));
	int value;
#ifdef BDCMGB_SEQ_BLINK_ON
	bool bIsStart = false, bIskeystopdisable = false;
#endif
	
	if (!count)
		return -EINVAL;

	value = simple_strtoul(buf, NULL, 10);
	DBG("value=%d\n",value);	

#ifdef BDCMGB_SEQ_BLINK_ON
	if((value&0xA) == 0xA)		bIsStart = true;
	if((value&0x1) == 0x1)		bIskeystopdisable = true;

//	hrtimer_cancel(&led->touchkey_timer);
	hrtimer_cancel(&led->ledmin_timer);

	if(bIsStart == true)
	{
		if(led->led_state == BD2802_SEQ)
		{
			if(!bd2802_check_colororder(led, bIskeystopdisable))
				return count;

			led->led_state = BD2802_OFF;
		}

//		hrtimer_cancel(&led->touchkey_timer);
//		hrtimer_cancel(&led->ledmin_timer);
		
		if (led->led_state == BD2802_OFF || led->led_state == BD2802_DIMMING)
		{
			led->white_current = BD2802_CURRENT_WHITE_MAX;
		       led->blue_current = BD2802_CURRENT_000;
			led->blink_enable=0;
			bd2802_reset_cancel(led);
			if(bIskeystopdisable == true)
				bd2802_on_blue(led);
			else
				bd2802_on(led);
			bd2802_enable(led);
		}

		led->led_state = BD2802_SEQ;
		led->blink_enable = 1;
		led->led_iskeystopdisable = (int)bIskeystopdisable;
		
		if(bIskeystopdisable == true)
		{
			//led->led_iskeystopdisable = 1;
			led->white_current = BD2802_CURRENT_000;
			led->blue_current = BD2802_CURRENT_BLUE_MAX;

			bd2802_configure(led);
			bd2802_on_blue(led);
		}
		else
		{
			led->white_current = BD2802_CURRENT_WHITE_MAX;
			led->blue_current = BD2802_CURRENT_000;

			bd2802_configure(led);
			bd2802_on(led);
		}

//		if(led->led_issuspend == 1)
//			bd2802_enable(led);
	}
	else
	{
		if(bIskeystopdisable == 0)
		{
			if((led->led_state == BD2802_SEQ) && (led->led_iskeystopdisable == 1))
				return 0;
		}
			
//		if(bIskeystopdisable == true)
//			led->led_iskeystopdisable = 0;
		
		if(led->led_state != BD2802_SEQ)
			return 0;

		if(bIskeystopdisable == true)
			led->led_iskeystopdisable = 0;

		if(led->led_issuspend == 1 || /*led->*/led_isboot == 0)
		{
			bd2802_off(led);
			bd2802_reset(led);
			led->led_state = BD2802_OFF;
			return 0;
		}

		led->led_state=BD2802_ON;
		led->white_current = BD2802_CURRENT_WHITE_MAX;
		led->blue_current = BD2802_CURRENT_000;
		led->blink_enable=0;
		bd2802_sw_reset(led);
		bd2802_reset_cancel(led);
		bd2802_on(led);
		bd2802_enable(led);
	
//		hrtimer_cancel(&led->touchkey_timer);
//		hrtimer_cancel(&led->ledmin_timer);

		hrtimer_start(&led->ledmin_timer, ktime_set(5, 0), HRTIMER_MODE_REL);
	}
#else //BDCMGB_SEQ_BLINK_ON
	if (value == 1)
	{
		led->led_state = BD2802_SEQ;
#if defined(BLINK_ON_BOOTING)
		led->white_current = BD2802_CURRENT_WHITE_MAX;
		led->blue_current = BD2802_CURRENT_000;
		led->blink_enable = 1;
		bd2802_configure(led);
		bd2802_on(led);
#else
		led->blue_current = BD2802_CURRENT_BLUE_MAX;
		hrtimer_start(&led->timer, ktime_set(0, 800000000), HRTIMER_MODE_REL); /* 0.8 sec */
#endif	// BLINK_ON_BOOTING
	}
	else if (value==0)
	{
#if defined(BLINK_ON_BOOTING)
		led->led_state=BD2802_ON;
		led->white_current = BD2802_CURRENT_WHITE_MAX;
		led->blue_current = BD2802_CURRENT_000;
		led->blink_enable=0;
		bd2802_sw_reset(led);
		bd2802_reset_cancel(led);
		bd2802_on(led);
		bd2802_enable(led);
#else 
		led->led_state=BD2802_SEQ_END;
		led->blue_current = BD2802_CURRENT_000;
		hrtimer_start(&led->ledmin_timer, ktime_set(10, 0), HRTIMER_MODE_REL);
#endif
	}
	else
	{
		DBG("Value is not valid\n");
		return -EINVAL;
	}
#endif //BDCMGB_SEQ_BLINK_ON
	return count;
}

static ssize_t bd2802_store_led_onoff(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct bd2802_led *led = i2c_get_clientdata(to_i2c_client(dev));
	int value;

	if (!count)
		return -EINVAL;

	value = simple_strtoul(buf, NULL, 10);
	DBG("value=%d\n",value);

	if ((value==1)&&(led->led_state!=BD2802_ON))
	{
		led->led_state = BD2802_ON;
		led->white_current = BD2802_CURRENT_WHITE_MAX;
		led->blue_current = BD2802_CURRENT_000;
		led->blink_enable=0;
		bd2802_reset_cancel(led);
		bd2802_on(led);
		bd2802_enable(led);
	}
	else if ((value==0)&&(led->led_state!=BD2802_OFF))
	{
		bd2802_off(led);
		bd2802_reset(led);
		led->led_state=BD2802_OFF;
	}
	else
	{
		if (value > 1)
		{
			return -EINVAL;
			DBG("Value is not valid\n");
		}
	}
	return count;
}

static ssize_t bd2802_store_led_testmode(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct bd2802_led *led = i2c_get_clientdata(to_i2c_client(dev));
	int value;
	
	if (!count)
		return -EINVAL;

	value = simple_strtoul(buf, NULL, 10);

	DBG("value=%d led->led_state=%d\n",value,led->led_state);

	if ((value==1)&&(led->led_state!=BD2802_TEST_ON))
	{
		led->led_state=BD2802_TEST_ON;
		led->white_current = BD2802_CURRENT_WHITE_MAX;
		led->blue_current = BD2802_CURRENT_000;
		bd2802_reset_cancel(led);
		bd2802_on(led);
		bd2802_enable(led);
	    DBG("TEST LED ON\n");
	}
	else if ((value==0)&&(led->led_state!=BD2802_TEST_OFF))
	{
		bd2802_off(led);
		bd2802_reset(led);
	    led->led_state=BD2802_TEST_OFF;
	    DBG("TEST LED OFF\n");
	}
	else
	{
	    if (value > 1)
	    {
		return -EINVAL;
		DBG("Value is not valid\n");
	}
	}

	return count;
}

static ssize_t bd2802_store_led_sync(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct bd2802_led *led = i2c_get_clientdata(to_i2c_client(dev));
	int value;
	
	if (!count)
		return -EINVAL;

	value = simple_strtoul(buf, NULL, 10);

	if(led->led_state == BD2802_TEST_ON)
		return -EBUSY;
	if(led->led_state == BD2802_SEQ)
		return -EBUSY;

	if(value == 1) {
		if(led->led_state != BD2802_SYNC) {
			hrtimer_cancel(&led->ledmin_timer);
			cancel_work_sync(&led->ledmin_work);
			//flush_workqueue(led->ledmin_wq);
			led->blue_current = BD2802_CURRENT_000;
			bd2802_turn_white(led, HIDDEN1);
			bd2802_turn_white(led, HIDDEN2);
			led->led_state = BD2802_SYNC;
		}
	} else if(value == 0) {
		if(led->led_state == BD2802_SYNC) {
			led->white_current = BD2802_CURRENT_WHITE_MAX;
			led->blue_current = BD2802_CURRENT_000;
			bd2802_on(led);
			led->led_state = BD2802_ON;
			hrtimer_start(&led->ledmin_timer, ktime_set(5, 0), HRTIMER_MODE_REL);
	}
	} else {
		return -EINVAL;
	}

	return count;
}

static ssize_t bd2802_store_led_brightness(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct bd2802_led *led = i2c_get_clientdata(to_i2c_client(dev));
	int value;
	
	if (!count)
		return -EINVAL;

	value = simple_strtoul(buf, NULL, 10);

	DBG("value=%d led->led_state=%d\n",value,led->led_state);

	if(led->led_state != BD2802_SYNC)
		return -EBUSY;
	if(value > 50)
		return -EINVAL;

	led->white_current = value;
	
	bd2802_turn_white(led, MENU);
	bd2802_turn_white(led, HOME);
	bd2802_turn_white(led, BACK);
	bd2802_turn_white(led, SEARCH);
	
	return count;
}


static struct device_attribute bd2802_led_start_attr = {
	.attr = {
		.name = "led_start",
		.mode = 0644,
		.owner = THIS_MODULE
	},
	.show = bd2802_show_led_start,
	.store = bd2802_store_led_start,
};

static struct device_attribute bd2802_led_onoff_attr = {
	.attr = {
		.name = "led_onoff",
		.mode = 0644,
		.owner = THIS_MODULE
	},
	.show = NULL,
	.store = bd2802_store_led_onoff,
};

static struct device_attribute bd2802_led_testmode_attr = {
	.attr = {
		.name = "led_testmode",
		.mode = 0644,
		.owner = THIS_MODULE
	},
	.show = NULL,
	.store = bd2802_store_led_testmode,
};

static struct device_attribute bd2802_led_sync_attr = {
	.attr = {
		.name = "led_sync",
		.mode = 0644,
		.owner = THIS_MODULE
	},
	.show = NULL,
	.store = bd2802_store_led_sync,
};

static struct device_attribute bd2802_led_brightness_attr = {
	.attr = {
		.name = "led_brightness",
		.mode = 0644,
		.owner = THIS_MODULE
	},
	.show = NULL,
	.store = bd2802_store_led_brightness,
};

#define BD2802_SET_REGISTER(reg_addr, reg_name)				\
static ssize_t bd2802_store_reg##reg_addr(struct device *dev,		\
	struct device_attribute *attr, const char *buf, size_t count)	\
{									\
	struct bd2802_led *led = i2c_get_clientdata(to_i2c_client(dev));\
	unsigned long val;						\
	int ret;							\
	int reg_add;						\
	if (!count)							\
		return -EINVAL;						\
	reg_add=(int)(reg_addr);					\
	val = simple_strtoul(buf, NULL, 10);			\
	down_write(&led->rwsem);					\
	led->register_value[reg_add]=(u8)val;				\
	if ((reg_add ==2 )||(reg_add==12))\
	{\
		ret=bd2802_write_byte(led->client, 2, (u8) val);		\
		ret=bd2802_write_byte(led->client, 12, (u8) val);		\
	}\
	else\
		ret=bd2802_write_byte(led->client, reg_addr, (u8) val);		\
	up_write(&led->rwsem);						\
/*	DBG("register_value = %d\n", led->register_value[reg_add]);*/\
	return count;							\
}									\
static ssize_t bd2802_show_reg##reg_addr(struct device *dev,		\
	struct device_attribute *attr, char *buf)	\
{									\
	struct bd2802_led *led = i2c_get_clientdata(to_i2c_client(dev));\
	int r;								\
	int reg_add;							\
	reg_add=reg_addr;	\
	r = snprintf(buf, PAGE_SIZE,"%d\n", led->register_value[reg_add]);\
	DBG("buf = %s\n", buf);\
	return r;\
}			\
static struct device_attribute bd2802_reg##reg_addr##_attr = {		\
	.attr = {.name = reg_name, .mode = 0644, .owner = THIS_MODULE},	\
	.store = bd2802_store_reg##reg_addr,				\
	.show = bd2802_show_reg##reg_addr,				\
};

BD2802_SET_REGISTER(0x00, "0x00");//CLKSETUP
BD2802_SET_REGISTER(0x01, "0x01");//LEDCONTROL
BD2802_SET_REGISTER(0x02, "0x02");//RGB1_HOURSETUP
BD2802_SET_REGISTER(0x03, "0x03");//R1_CURRENT1
BD2802_SET_REGISTER(0x04, "0x04");//R1_CURRENT2
BD2802_SET_REGISTER(0x05, "0x05");//R1_PATTERN
BD2802_SET_REGISTER(0x06, "0x06");//G1_CURRENT1
BD2802_SET_REGISTER(0x07, "0x07");//G1_CURRENT2
BD2802_SET_REGISTER(0x08, "0x08");//G1_PATTERN
BD2802_SET_REGISTER(0x09, "0x09");//B1_CURRENT1
BD2802_SET_REGISTER(0x0a, "0x0A");//B1_CURRENT2
BD2802_SET_REGISTER(0x0b, "0x0B");//B1_PATTERN
BD2802_SET_REGISTER(0x0c, "0x0C");//RGB2_HOURSETUP
BD2802_SET_REGISTER(0x0d, "0x0D");//R2_CURRENT1
BD2802_SET_REGISTER(0x0e, "0x0E");//R2_CURRENT2
BD2802_SET_REGISTER(0x0f, "0x0F");//R2_PATTERN
BD2802_SET_REGISTER(0x10, "0x10");//G2_CURRENT1
BD2802_SET_REGISTER(0x11, "0x11");//G2_CURRENT2
BD2802_SET_REGISTER(0x12, "0x12");//G2_PATTERN
BD2802_SET_REGISTER(0x13, "0x13");//B2_CURRENT1
BD2802_SET_REGISTER(0x14, "0x14");//B2_CURRENT2
BD2802_SET_REGISTER(0x15, "0x15");//B2_PATTERN

static struct device_attribute *bd2802_attributes[] = {
	&bd2802_blink_enable_attr,
	&bd2802_led_start_attr,
	&bd2802_led_onoff_attr,
	&bd2802_led_testmode_attr,
	&bd2802_led_sync_attr,
	&bd2802_led_brightness_attr,
	&bd2802_reg0x00_attr,
	&bd2802_reg0x01_attr,
	&bd2802_reg0x02_attr,
	&bd2802_reg0x03_attr,
	&bd2802_reg0x04_attr,
	&bd2802_reg0x05_attr,
	&bd2802_reg0x06_attr,
	&bd2802_reg0x07_attr,
	&bd2802_reg0x08_attr,
	&bd2802_reg0x09_attr,
	&bd2802_reg0x0a_attr,
	&bd2802_reg0x0b_attr,
	&bd2802_reg0x0c_attr,
	&bd2802_reg0x0d_attr,
	&bd2802_reg0x0e_attr,
	&bd2802_reg0x0f_attr,
	&bd2802_reg0x10_attr,
	&bd2802_reg0x11_attr,
	&bd2802_reg0x12_attr,
	&bd2802_reg0x13_attr,
	&bd2802_reg0x14_attr,
	&bd2802_reg0x15_attr,
};

#ifdef BDCMGB_SEQ_BLINK_ON
void check_chargeroff(void)
{
	if(led_isboot == 1)
	{
		struct bd2802_led *led = i2c_get_clientdata(bd2802_i2c_client);
		if(led->led_issuspend)
		{
			if(led->led_state == BD2802_SEQ && led->led_iskeystopdisable)
			{
				led->led_iskeystopdisable = 0;
				bd2802_off(led);
				bd2802_reset(led);
				led->led_state = BD2802_OFF;
			}
		}
	}
}
EXPORT_SYMBOL(check_chargeroff);
#endif

#ifdef CONFIG_HAS_EARLYSUSPEND
static int bd2802_bl_suspend(struct i2c_client *client, pm_message_t state)
{
	struct bd2802_led *led = i2c_get_clientdata(client);
	DBG("\n");

	if (led->led_state==BD2802_TEST_ON)
		return 0;

#ifdef BDCMGB_SEQ_BLINK_ON
	if (led->led_state==BD2802_SEQ)
		return 0;
#endif

	bd2802_off(led);
#ifdef BDCMGB_SEQ_BLINK_ON
	bd2802_reset(led);
#endif
	led->led_state = BD2802_OFF;
	return 0;
}

static int bd2802_bl_resume(struct i2c_client *client)
{
	struct bd2802_led *led = i2c_get_clientdata(client);
	DBG("\n");

	led->led_state = BD2802_ON;
	led->white_current = BD2802_CURRENT_WHITE_MAX;
	led->blue_current = BD2802_CURRENT_000;
/*	if (system_rev >=4) //OVER REV.D
	{
		bd2802_write_byte(led->client, BD2812_DCDCDRIVER, 0x00);
		bd2802_write_byte(led->client, BD2812_PIN_FUNC_SETUP, 0x0F);
	}*/
#ifdef BDCMGB_SEQ_BLINK_ON
	led->blink_enable=0;
	bd2802_sw_reset(led);
	bd2802_reset_cancel(led);
#endif
	bd2802_on(led);
	bd2802_enable(led);

	hrtimer_start(&led->ledmin_timer, ktime_set(5, 0), HRTIMER_MODE_REL);
	return 0;
}


static void bd2802_early_suspend(struct early_suspend *h)
{
	struct bd2802_led *led;
	DBG("\n");

	led = container_of(h, struct bd2802_led, early_suspend);

#ifndef BDCMGB_SEQ_BLINK_ON
	if(led->led_state==BD2802_SEQ ||
	   led->led_state == BD2802_TEST_ON)
		return;
#else
	led->led_issuspend = 1;
	led->led_isresumekeypressed =0;

	if(/*led->*/led_isboot == 0 || led->led_state==BD2802_SEQ)
		return;
#endif

	hrtimer_cancel(&led->timer);
	hrtimer_cancel(&led->touchkey_timer);
	hrtimer_cancel(&led->ledmin_timer);


	cancel_work_sync(&led->work);
	cancel_work_sync(&led->touchkey_work);
	cancel_work_sync(&led->ledmin_work);


	bd2802_bl_suspend(led->client, PMSG_SUSPEND);
}


static void bd2802_late_resume(struct early_suspend *h)
{
	struct bd2802_led *led;
	DBG("\n");

	led = container_of(h, struct bd2802_led, early_suspend);

#ifndef BDCMGB_SEQ_BLINK_ON
	if (led->led_state==BD2802_SEQ)
		return;
#else
	led->led_issuspend = 0;
//	if ((led->led_state == BD2802_SEQ) && (led->led_iskeystopdisable == 1))
	if ( (led->led_state == BD2802_SEQ) && 
		(led->led_iskeystopdisable == 1 || led->led_isresumekeypressed == 0) )
	{
		return;
	}
	led->led_isresumekeypressed =0;

	if(/*led->*/led_isboot == 0)
		return;
#endif
	bd2802_bl_resume(led->client);
}
#endif	/* CONFIG_HAS_EARLYSUSPEND */

static int __devinit bd2802_probe(struct i2c_client *client,
			const struct i2c_device_id *id)
{
	struct bd2802_led *led;
	int ret, i;

	pr_warning("%s() -- start\n", __func__);

	led = kzalloc(sizeof(struct bd2802_led), GFP_KERNEL);
	if (!led) {
		dev_err(&client->dev, "failed to allocate driver data\n");
		return -ENOMEM;
	}

	led->client = client;
	i2c_set_clientdata(client, led);

	INIT_WORK(&led->work, bd2802_work_func);
	INIT_WORK(&led->touchkey_work, bd2802_touchkey_work_func);
	INIT_WORK(&led->ledmin_work, bd2802_ledmin_work_func);

	led->bd2802_wq = create_singlethread_workqueue("bd2802_wq");
	if (!led->bd2802_wq)
		return -ENOMEM;
/* 
	led->touchkey_wq = create_singlethread_workqueue("touchkey_wq");
	if (!led->touchkey_wq)
		return -ENOMEM;

	led->ledmin_wq = create_singlethread_workqueue("ledmin_wq");
	if (!led->ledmin_wq)
		return -ENOMEM;
*/
	bd2802_i2c_client = led->client;
	/* Default attributes */
	led->wave_pattern = BD2802_PATTERN_FULL;
	led->blink_enable =0;
	led->led_state = BD2802_SEQ;
	led->key_led = ALL;
	led->key_direction= FORWARD;
	led->led_counter=0;
#if defined(BLINK_ON_BOOTING)
	led->white_current = BD2802_CURRENT_WHITE_MAX;
	led->blue_current = BD2802_CURRENT_000;
#else
	led->white_current = BD2802_CURRENT_WHITE_MAX;
	led->blue_current = BD2802_CURRENT_BLUE_MAX;
#endif
#ifdef BDCMGB_SEQ_BLINK_ON
	led->led_iskeystopdisable = 0;
	led->led_issuspend = 0;
	led->led_isresumekeypressed = 0;
	/*led->*/led_isboot = 0;
#endif

	init_rwsem(&led->rwsem);

	for (i = 0; i < ARRAY_SIZE(bd2802_attributes); i++) {
		ret = device_create_file(&led->client->dev,
						bd2802_attributes[i]);
		if (ret) {
			dev_err(&led->client->dev, "failed: sysfs file %s\n",
					bd2802_attributes[i]->attr.name);
			goto failed_unregister_dev_file;
		}
	}

	hrtimer_init(&led->timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	led->timer.function = bd2802_timer_func;
#if defined(BLINK_ON_BOOTING)
#else
	hrtimer_start(&led->timer, ktime_set(4, 0), HRTIMER_MODE_REL);
#endif
	
	hrtimer_init(&led->touchkey_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	led->touchkey_timer.function = bd2802_touchkey_timer_func;
	
	hrtimer_init(&led->ledmin_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	led->ledmin_timer.function = bd2802_ledmin_timer_func;
	
#ifdef CONFIG_HAS_EARLYSUSPEND
	led->early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN;
	led->early_suspend.suspend = bd2802_early_suspend;
	led->early_suspend.resume = bd2802_late_resume;
	register_early_suspend(&led->early_suspend);
#endif

//	bd2802_configure(led);
#if defined(BLINK_ON_BOOTING)
	led->blink_enable =1;
#endif
	bd2802_on(led);
	bd2802_enable(led);

//LGE_UPDATE
#ifndef BDCMGB_SEQ_BLINK_ON
	led->led_state=BD2802_ON;
#else
	led->led_state=BD2802_BLINK;
#endif
//LGE_UPDATE
	return 0;

failed_unregister_dev_file:
	for (i--; i >= 0; i--)
		device_remove_file(&led->client->dev, bd2802_attributes[i]);

	return ret;
}

static int __exit bd2802_remove(struct i2c_client *client)
{
	struct bd2802_led *led = i2c_get_clientdata(client);
	int i;

	hrtimer_cancel(&led->timer);
	hrtimer_cancel(&led->touchkey_timer);
	hrtimer_cancel(&led->ledmin_timer);

	bd2802_reset(led);

	for (i = 0; i < ARRAY_SIZE(bd2802_attributes); i++)
		device_remove_file(&led->client->dev, bd2802_attributes[i]);
	i2c_set_clientdata(client, NULL);
	kfree(led);

	if (led->bd2802_wq)
		destroy_workqueue(led->bd2802_wq);
	
	if (led->touchkey_wq)
		destroy_workqueue(led->touchkey_wq);

	if (led->ledmin_wq)
		destroy_workqueue(led->ledmin_wq);

	return 0;
}

static int bd2802_suspend(struct i2c_client *client, pm_message_t mesg)
{
	struct bd2802_led *led = i2c_get_clientdata(client);

	DBG("BD2802 suspend\n");

	if (led->led_state == BD2802_TEST_ON)
		return 0;

	gpio_set_value(RGB_LED_CNTL, 0);
	led->led_state = BD2802_OFF;
	return 0;
}

static int bd2802_resume(struct i2c_client *client)
{
	struct bd2802_led *led = i2c_get_clientdata(client);
	DBG("\n");

	led->led_state = BD2802_ON;	
/*  TODO : to wakeup from Touch LED suspend with out blinking */
	led->blink_enable =0;
/*  TODO : to wakeup from Touch LED suspend with out blinking */

	bd2802_reset_cancel(led);
//HARDLINE	bd2802_on(led);
	return 0;
}

static const struct i2c_device_id bd2802_id[] = {
	{ "BD2802", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, bd2802_id);

static struct i2c_driver bd2802_i2c_driver = {
	.driver	= {
		.name	= "BD2802",
	},
	.probe		= bd2802_probe,
	.remove		= __exit_p(bd2802_remove),
#ifndef CONFIG_HAS_EARLYSUSPEND 
	.suspend	= bd2802_suspend,
	.resume		= bd2802_resume,
#endif 
	.id_table	= bd2802_id,
};

#if 1
void __init bd2802_init(void)
{	
	i2c_add_driver(&bd2802_i2c_driver);
}
#else
static int __init bd2802_init(void)
{
	return i2c_add_driver(&bd2802_i2c_driver);
}
module_init(bd2802_init);

static void __exit bd2802_exit(void)
{
	i2c_del_driver(&bd2802_i2c_driver);

	if (bd2802_wq)
		destroy_workqueue(bd2802_wq);
}
module_exit(bd2802_exit);
#endif

MODULE_AUTHOR("Kim Kyuwon <q1.kim@samsung.com>");
MODULE_DESCRIPTION("BD2802 LED driver");
MODULE_LICENSE("GPL v2");
