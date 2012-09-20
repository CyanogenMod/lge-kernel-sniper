/*
 * drivers/input/touchscreen/justin_synaptics_keytouch.c
 *
 * Copyright (C) 2011 LG Electronics, Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * 2010-11-07 : Choi Daewan <ntdeaewan.choi@lge.com>
 *
 */

#include <linux/module.h>
#include <linux/delay.h>
#include <linux/earlysuspend.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <mach/gpio.h>
#include <../../../arch/arm/mach-omap2/mux.h>

#include "justin_kr_synaptics_keytouch.h"
#include "justin_kr_synaptics_user.h"
#include "justin_kr_synaptics_config.h"

#define SYNAPTICS_KEYTOUCH_DEBUG 0

#if SYNAPTICS_KEYTOUCH_DEBUG
#define keyts_debug(args...)  printk(args)
#else
#define keyts_debug(args...)
#endif

static struct workqueue_struct *keytouch_wq;

static 	u8	keytouch_init_buf_1[4]={0x00,0x00,0x00,0x07};
static	u8	keytouch_init_buf_2[4]={0x00,0x04,0x00,0x05};
static	u8	keytouch_init_buf_3[4]={0x00,0x10,0xFF,0xFF};
static	u8	keytouch_init_buf_4[4]={0x00,0x11,0xFF,0xFF};

static 	u8	keytouch_read_addr1[2]={0x00,0x00};
static	u8	keytouch_read_addr2[2]={0x00,0x04};
static	u8	keytouch_read_addr3[2]={0x00,0x10};
static	u8	keytouch_read_addr4[2]={0x00,0x11};

static	u8	keytouch_read_data[2]={0x01,0x09};

static	u8	keytouch_menu_flag = 0;
static	u8	keytouch_back_flag = 0;

struct synaptics_keytouch_priv {
	uint16_t addr;
	struct i2c_client *client;
	struct input_dev *input_dev;
	int use_irq;
	bool has_relative_report;
	struct work_struct  work;
	unsigned int interval;
	uint32_t flags;

	struct early_suspend early_suspend;
	struct delayed_work init_delayed_work;
};
struct synaptics_keytouch_priv *p_keyts_priv;

int init_keyts = 0;

static void synaptics_keytouch_early_suspend(struct early_suspend *h);
static void synaptics_keytouch_late_resume(struct early_suspend *h);

#define KEY_TOUCH_INT_N_GPIO					126
#define TOUCH_I2C2_SW_GPIO_162					162
#define TOUCH_LDO_EN_GPIO59						59

#define SYNAPTICS_CONFIG_REG_ADDR					0x0000
#define SYNAPTICS_DATA_REG_ADDR						0x0109

/*************************************************************************/
// Library

#define OT_READ		0x01

// delays and timing routines

static int synaptics_keytouch_write(u8 addr7bit, u8 Buffer[], u8 BytesToSend);
static int synaptics_keytouch_read(u8 addr7bit[], u8 Buffer[], u8 BytesToRead);

extern u8 ts_keytouch_lock;
extern u8 ts_reset_flag;
extern int lcd_backlight_status;
extern int lcd_off_boot;
extern void keyled_touch_on(void);

u8 synaptics_touch_power=0;
u8 synaptics_keytouch_power=0;

static u8 synaptics_keytouch_initialize(void)
{
	u8  pBuffer[OT_NUM_CONFIG_BYTES];
	int ret=0;

	if(init_keyts==-1)	
	{
		return -1;
	}
	else if(init_keyts == 1)
	{
		printk("[keytouch] reset & init !! \n");
		init_keyts = 0;
		ts_reset_flag=1;
		gpio_direction_output(TOUCH_I2C2_SW_GPIO_162, 0);
		gpio_direction_output(TOUCH_LDO_EN_GPIO59, 0);
		mdelay(20);
		gpio_direction_output(TOUCH_LDO_EN_GPIO59, 1);
		gpio_direction_output(TOUCH_I2C2_SW_GPIO_162, 1);
		mdelay(100);
		ts_reset_flag=2;
	}

	ret=synaptics_keytouch_write(keytouch_read_addr1[1], keytouch_init_buf_1, 4);
	synaptics_keytouch_write(keytouch_read_addr2[1], keytouch_init_buf_2, 4);
	synaptics_keytouch_write(keytouch_read_addr3[1], keytouch_init_buf_3, 4);
	synaptics_keytouch_write(keytouch_read_addr4[1], keytouch_init_buf_4, 4);

	synaptics_keytouch_read(keytouch_read_addr1, pBuffer,2);
	synaptics_keytouch_read(keytouch_read_addr2, pBuffer,2);
	synaptics_keytouch_read(keytouch_read_addr3, pBuffer,2);
	synaptics_keytouch_read(keytouch_read_addr4, pBuffer,2);
	
	mdelay(50);
	synaptics_keytouch_read(keytouch_read_data, pBuffer,2);

	init_keyts = 1;

	return ret;
}

static int synaptics_keytouch_write(u8 addr7bit, u8 Buffer[], u8 BytesToSend)
{
	u8 count=0;
	int ret;
	struct i2c_msg msg[2];

	/* write register */
	msg[0].addr = 0x2C;
	msg[0].flags = 0;
	msg[0].len = BytesToSend;
	msg[0].buf = &Buffer[0];

	ret = i2c_transfer(p_keyts_priv->client->adapter, &msg[0], 1);

	if(ret<0)
	{
		printk("[keytouch] i2c write fail !! address : 0x%x\n",addr7bit);
		return -1;
	}

	keyts_debug("[keytouch] write address : 0x%x : ",addr7bit);

	for(count=0;count < BytesToSend;count++)
		keyts_debug("0x%x ",Buffer[count]);

	keyts_debug("\n");

	return OT_SUCCESS;
}

static int synaptics_keytouch_read(u8 addr7bit[], u8 Buffer[], u8 BytesToRead)
{
	u8 count=0;
	int ret;
	struct i2c_msg msg[2];

	/* write register */
	msg[0].addr = 0x2C;
	msg[0].flags = 0;
	msg[0].len = BytesToRead;
	msg[0].buf = &addr7bit[0];

	/* read register */
	msg[1].addr = 0x2C;
	msg[1].flags = I2C_M_RD;
	msg[1].len = 2;
	msg[1].buf = &Buffer[0];

	ret = i2c_transfer(p_keyts_priv->client->adapter, &msg[0], 1);
	ret = i2c_transfer(p_keyts_priv->client->adapter, &msg[1], 1);

	if(ret<0)
	{
		printk("[keytouch] i2c read fail !! address : 0x%x\n",addr7bit[1]);
		return -1;
	}

	keyts_debug("[keytouch] read address : 0x%x : ",addr7bit[1]);

	for(count=0;count < BytesToRead;count++)
		keyts_debug("0x%x ",Buffer[count]);

	keyts_debug("\n");

	return OT_SUCCESS;
}

#define CONTROL_PADCONF_GPIO127	(0xFA002A54 + 2) //0xFA00 2A56
#define OMAP_PIN_MODE4			0x4
static void synaptics_keytouch_gpio_cfg(void)
{
	int ret = -1;
	
	/* Set the ts_gpio pin mux */
//--[[ LGE_UBIQUIX_MODIFIED_START : shyun@ubiquix.com [2012.04.05] - Mux mode setting through omap_mux_init_signal API.
//--[[ LGE_UBIQUIX_MODIFIED_START : shyun@ubiquix.com [2012.03.16] - Can't setting MUX Mode through omap_mux_init_signal API
#if 1
	ret = omap_mux_init_signal("sdmmc1_dat4.gpio_126", OMAP_PIN_INPUT);
	if(ret < 0)
		printk("[%s] omap_mux_init_signal ret = %d", __func__, ret);
#else
	__raw_writew((OMAP_PIN_INPUT | OMAP_PIN_MODE4), CONTROL_PADCONF_GPIO127 );	//GPIO_126[31:16] Input Mode. SHYUN_TBD
#endif	
//--]] LGE_UBIQUIX_MODIFIED_END : shyun@ubiquix.com [2012.03.16]- Can't setting MUX Mode through omap_mux_init_signal API
//--]] LGE_UBIQUIX_MODIFIED_END : shyun@ubiquix.com [2012.04.05]- Mux mode setting through omap_mux_init_signal API.
	
//--[[ LGE_UBIQUIX_MODIFIED_START : shyun@ubiquix.com [2011.07.13] - Compile error in Kernel 2.6.35
#if 0
	omap_set_gpio_debounce(KEY_TOUCH_INT_N_GPIO, 1);
	omap_set_gpio_debounce_time(KEY_TOUCH_INT_N_GPIO, 0xa);
#else
	if(gpio_request(KEY_TOUCH_INT_N_GPIO, "keytouch_i2c_sw") < 0)
		printk("keytouch reqeust error!!!!!!!!!!!\n");

	gpio_direction_input(KEY_TOUCH_INT_N_GPIO);

	gpio_set_debounce(KEY_TOUCH_INT_N_GPIO, 100);
#endif
//--]] LGE_UBIQUIX_MODIFIED_END : shyun@ubiquix.com [2011.07.13]- Compile error in Kernel 2.6.35

	return;
}

void synaptics_keytouch_power_on(void)
{
	printk("[SHYUN] [%s] [!touch_power = %d, !keytouch_power = %d]\n", 
														__func__, !synaptics_touch_power, !synaptics_keytouch_power);

	if(!synaptics_touch_power && !synaptics_keytouch_power)
	{
		gpio_direction_output(TOUCH_LDO_EN_GPIO59, 1);
		gpio_direction_output(TOUCH_I2C2_SW_GPIO_162, 1);
	}
	synaptics_keytouch_power=1;
//	printk("[touch] %s : touch=%d, keytouch=%d\n",__func__,synaptics_touch_power,synaptics_keytouch_power);
	return;
}

void synaptics_keytouch_power_off(void)
{
	synaptics_keytouch_power=0;

	printk("[SHYUN] [%s] [!touch_power = %d, !keytouch_power = %d]\n", 
														__func__, !synaptics_touch_power, !synaptics_keytouch_power);
	
	if(!synaptics_touch_power && !synaptics_keytouch_power)
	{
		gpio_direction_output(TOUCH_I2C2_SW_GPIO_162, 0);
		mdelay(30);
		gpio_direction_output(TOUCH_LDO_EN_GPIO59, 0);
	}
//	printk("[touch] %s : touch=%d, keytouch=%d\n",__func__,synaptics_touch_power,synaptics_keytouch_power);
	return;
}

static u8 keyts_pBuffer[2] = {0x00,0x00};
static u8 keyts_fail_count=0;

static void synaptics_keytouch_init_delayed_work(struct work_struct *work)
{
	int ret, key_work_flag;
	
	local_irq_save(key_work_flag);
	
	keyts_pBuffer[0]=0;
	keyts_pBuffer[1]=0;

	if(ts_keytouch_lock>0)	ts_keytouch_lock--;

	if(!gpio_get_value(KEY_TOUCH_INT_N_GPIO) && init_keyts==1 && lcd_backlight_status && !ts_reset_flag)
	{
		ret = synaptics_keytouch_read(keytouch_read_data, keyts_pBuffer,2);
		
		if(ret<0 && !ts_keytouch_lock && init_keyts>-1)
		{
			keyts_fail_count++;
			if(keyts_fail_count>5)
			{
				synaptics_keytouch_initialize();
				mdelay(200);
				keyts_fail_count=0;
			}
		}
		else if(ret>=0 && !ts_keytouch_lock && init_keyts>-1)
		{
			//down event
			if(!keytouch_menu_flag && keyts_pBuffer[1]==0x04)
			{
				input_report_key(p_keyts_priv->input_dev, KEY_MENU, 1);
				input_sync(p_keyts_priv->input_dev);
				keytouch_menu_flag = 1;
				printk("KEY_MENU Pressed!\n");
//--[[ LGE_UBIQUIX_MODIFIED_START : shyun@ubiquix.com [2011.07.13] - Keytouch led feature is enabled
//--[[ LGE_UBIQUIX_MODIFIED_START : shyun@ubiquix.com [2011.07.11] - shyun TBD
				//keyled_touch_on();
//--]] LGE_UBIQUIX_MODIFIED_END : shyun@ubiquix.com [2011.07.11]- shyun TBD
//--]] LGE_UBIQUIX_MODIFIED_END : shyun@ubiquix.com [2011.07.13]- Keytouch led feature is enabled
			}
			else if(!keytouch_back_flag && keyts_pBuffer[1]==0x01)
			{
				input_report_key(p_keyts_priv->input_dev, KEY_BACK, 1);
				input_sync(p_keyts_priv->input_dev);
				keytouch_back_flag = 1;
				printk("KEY_BACK Pressed!\n");
//--[[ LGE_UBIQUIX_MODIFIED_START : shyun@ubiquix.com [2011.07.13] - Keytouch led feature is enabled
//--[[ LGE_UBIQUIX_MODIFIED_START : shyun@ubiquix.com [2011.07.11] - shyun TBD
				//keyled_touch_on();
//--]] LGE_UBIQUIX_MODIFIED_END : shyun@ubiquix.com [2011.07.11]- shyun TBD
//--]] LGE_UBIQUIX_MODIFIED_END : shyun@ubiquix.com [2011.07.13]- Keytouch led feature is enabled
			}
			else if(!keytouch_menu_flag && !keytouch_back_flag && keyts_pBuffer[1]==0x05)
			{
				input_report_key(p_keyts_priv->input_dev, KEY_MENU, 1);
				input_report_key(p_keyts_priv->input_dev, KEY_BACK, 1);
				input_sync(p_keyts_priv->input_dev);
				keytouch_menu_flag = 1;
				keytouch_back_flag = 1;
				printk("KEY_MENU / KEY_BACK Pressed!\n");
//--[[ LGE_UBIQUIX_MODIFIED_START : shyun@ubiquix.com [2011.07.13] - Keytouch led feature is enabled
//--[[ LGE_UBIQUIX_MODIFIED_START : shyun@ubiquix.com [2011.07.11] - shyun TBD
				//keyled_touch_on();
//--]] LGE_UBIQUIX_MODIFIED_END : shyun@ubiquix.com [2011.07.11]- shyun TBD
//--]] LGE_UBIQUIX_MODIFIED_END : shyun@ubiquix.com [2011.07.13]- Keytouch led feature is enabled
			}
			else // up event
			{
				if(keytouch_menu_flag)
				{
					input_report_key(p_keyts_priv->input_dev, KEY_MENU, 0);
					input_sync(p_keyts_priv->input_dev);
					keytouch_menu_flag = 0;
					printk("KEY_MENU Pressed!\n");
				}
				if(keytouch_back_flag)
				{
					input_report_key(p_keyts_priv->input_dev, KEY_BACK, 0);
					input_sync(p_keyts_priv->input_dev);
					keytouch_back_flag = 0;
					printk("KEY_BACK Pressed!\n");
				}
			}
		}
	}
	else if(ts_keytouch_lock)// up event
		{
			if(keytouch_menu_flag)
			{
				input_report_key(p_keyts_priv->input_dev, KEY_MENU, 0);
				input_sync(p_keyts_priv->input_dev);
				keytouch_menu_flag = 0;
				printk("[keytouch] (MENU_KEY) realse.\n");
			}
			if(keytouch_back_flag)
			{
				input_report_key(p_keyts_priv->input_dev, KEY_BACK, 0);
				input_sync(p_keyts_priv->input_dev);
				keytouch_back_flag = 0;
				printk("[keytouch] (BACK_KEY) realse.\n");
			}
		}

	local_irq_restore(key_work_flag);
	p_keyts_priv->interval = msecs_to_jiffies(30);
   	schedule_delayed_work(&p_keyts_priv->init_delayed_work, p_keyts_priv->interval);
}

static void synaptics_keytouch_work_func(struct work_struct *work)
{
	if(init_keyts != 1)	return;
	keyts_debug("[keytouch] keytouch work queue start. \n");
	return;
}

static int synaptics_keytouch_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int ret = 0;

	synaptics_keytouch_power_on();
	printk("[keytouch] %s() -- start\n\n\n", __func__);
	
#if 0
	if(lcd_off_boot ==1) 	
	{
		printk("[keytouch] No Device LCD\n");
		ret = -ENODEV;
		return ret;
	}
#endif	
	
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		printk(KERN_ERR "synaptics_keytouch_probe: need I2C_FUNC_I2C\n");
		ret = -ENODEV;
		goto err_check_functionality_failed;
	}
	
	p_keyts_priv = kzalloc(sizeof(*p_keyts_priv), GFP_KERNEL);

	if (p_keyts_priv == NULL) {
		ret = -ENOMEM;
		goto err_alloc_data_failed;
	}

	synaptics_keytouch_gpio_cfg();

	p_keyts_priv->client = client;
	i2c_set_clientdata(client, p_keyts_priv);

	INIT_WORK(&p_keyts_priv->work, synaptics_keytouch_work_func);
	INIT_DELAYED_WORK(&p_keyts_priv->init_delayed_work, synaptics_keytouch_init_delayed_work);

	p_keyts_priv->input_dev = input_allocate_device();
	p_keyts_priv->input_dev->name = "hub_keytouch";

	set_bit(EV_KEY, p_keyts_priv->input_dev->evbit);
	// button
	set_bit(KEY_MENU, p_keyts_priv->input_dev->keybit);
	set_bit(KEY_BACK, p_keyts_priv->input_dev->keybit);

	/* p_keyts_priv->input_dev->name = keytouch->keypad_info->name; */
	ret = input_register_device(p_keyts_priv->input_dev);

	keyts_debug("[keytouch] request_irq\n");

	p_keyts_priv->early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN - 1;
	p_keyts_priv->early_suspend.suspend = synaptics_keytouch_early_suspend;
	p_keyts_priv->early_suspend.resume = synaptics_keytouch_late_resume;
	register_early_suspend(&p_keyts_priv->early_suspend);

	keyts_debug("[keytouch] synaptics_keytouch_probe: Start keytouch %s in %s mode\n",
				p_keyts_priv->input_dev->name, p_keyts_priv->use_irq ? "interrupt" : "polling");
	
	mdelay(100);
	ret = synaptics_keytouch_initialize();
	
	schedule_delayed_work(&p_keyts_priv->init_delayed_work, msecs_to_jiffies(30000));

	return 0;

err_alloc_data_failed:
	keyts_debug("[keytouch] err_alloc_data_failed !! \n");
	return -1;
err_check_functionality_failed:
	return ret;
}

static int synaptics_keytouch_remove(struct i2c_client *client)
{
	struct synaptics_keytouch_priv *keytouch = i2c_get_clientdata(client);
	unregister_early_suspend(&keytouch->early_suspend);

	if (keytouch->use_irq)
		free_irq(client->irq, keytouch);

	input_unregister_device(keytouch->input_dev);
	kfree(keytouch);
	return 0;
}

static int synaptics_keytouch_suspend(struct i2c_client *client, pm_message_t mesg)
{
	struct synaptics_keytouch_priv *keytouch = i2c_get_clientdata(client);
	int ret;

	init_keyts = -1;
	cancel_delayed_work_sync(&keytouch->init_delayed_work);

	if (keytouch->use_irq)
		disable_irq(client->irq);

	ret = cancel_work_sync(&keytouch->work);
	mdelay(30);

/* S, 20110923, mschung@ubiquix.com, Moved to backlight driver(lm3528_bl.c) for proper sequence. */
#if 0
	synaptics_keytouch_power_off();
#endif
/* E, 20110923, mschung@ubiquix.com, Moved to backlight driver(lm3528_bl.c) for proper sequence. */

	return 0;
}

static int synaptics_keytouch_resume(struct i2c_client *client)
{
	struct synaptics_keytouch_priv *keytouch = i2c_get_clientdata(client);
	int ret;
	
	if (keytouch->use_irq)
		enable_irq(client->irq);
	
	if(!init_keyts)	ret=synaptics_keytouch_initialize();
	
	if(ret<0)
	{
		mdelay(30);
//		printk("[keytouch] %s : %d\n",__func__,__LINE__);
		ret=synaptics_keytouch_initialize();
	}
	schedule_delayed_work(&keytouch->init_delayed_work, msecs_to_jiffies(500));

	return 0;
}

static void synaptics_keytouch_early_suspend(struct early_suspend *h)
{
	struct synaptics_keytouch_priv *keytouch;
	init_keyts = -1;
	keytouch = container_of(h, struct synaptics_keytouch_priv, early_suspend);
	synaptics_keytouch_suspend(keytouch->client, PMSG_SUSPEND);

	return;
}

static void synaptics_keytouch_late_resume(struct early_suspend *h)
{
	struct synaptics_keytouch_priv *keytouch;
	init_keyts = 0;
	keytouch = container_of(h, struct synaptics_keytouch_priv, early_suspend);

/* S, 20110923, mschung@ubiquix.com, Moved to backlight driver(lm3528_bl.c) for proper sequence. */
#if 0
	synaptics_keytouch_power_on();
	mdelay(200);
#endif
/* E, 20110923, mschung@ubiquix.com, Moved to backlight driver(lm3528_bl.c) for proper sequence. */

//	printk("[keytouch] resume point\n");
	synaptics_keytouch_resume(keytouch->client);

	return;
}

static const struct i2c_device_id synaptics_keytouch_id[] = {
	{ "hub_keytouch", 0 },
	{ },
};

static struct i2c_driver synaptics_keytouch_driver = {
	.probe		= synaptics_keytouch_probe,
	.remove		= synaptics_keytouch_remove,
//	.suspend	= synaptics_keytouch_suspend,
//	.resume		= synaptics_keytouch_resume,
	.id_table	= synaptics_keytouch_id,
	.driver = {
		.name	= "hub_keytouch",
		.owner = THIS_MODULE,
	},
};

static int __devinit synaptics_keytouch_init(void)
{
   	keyts_debug("[keytouch] synaptics keytouch init\n");
	return i2c_add_driver(&synaptics_keytouch_driver);
}

static void __exit synaptics_keytouch_exit(void)
{
	i2c_del_driver(&synaptics_keytouch_driver);

	if (keytouch_wq)
		destroy_workqueue(keytouch_wq);
	return;
}

module_init(synaptics_keytouch_init);
module_exit(synaptics_keytouch_exit);

MODULE_DESCRIPTION("Synaptics Touchscreen Driver");
MODULE_AUTHOR("Choi Daewan <ntdeaewan.choi@lge.com>");
MODULE_LICENSE("GPL");

