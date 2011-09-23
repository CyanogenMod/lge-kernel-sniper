/*
 * Regulator driver for National Semiconductors LP8720 chip
 *
 *  Copyright (C) 2009 LG Electronics
 *  Author: 
 *
 * 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */
#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/backlight.h>
#include <linux/fb.h>
#include <linux/i2c.h>
#include <linux/string.h>
#include <asm/gpio.h>
#include <asm/system.h>

#include <linux/regulator/lp8720.h>

#define TIMESTEP_25US			1 //default
#define TIMESTEP_50US			0

#define LP8720_GENERAL_SETTING		0x00
#define LP8720_LDO1_SETTING		0x01
#define LP8720_LDO2_SETTING		0x02
#define LP8720_LDO3_SETTING		0x03
#define LP8720_LDO4_SETTING		0x04
#define LP8720_LDO5_SETTING		0x05
#define LP8720_BUCK_SETTING1		0x06
#define LP8720_BUCK_SETTING2		0x07
#define LP8720_OUTPUT_ENABLE		0x08
#define LP8720_PULLDOWN_BITS		0x09
#define LP8720_STATUS_BITS		0x0a
#define LP8720_INTERRUPT_BITS		0x0b
#define LP8720_INTERRUPT_MASK		0x0c

#define LP8720_STARTUP_DELAY_0		0x00
#define LP8720_STARTUP_DELAY_1TS	0x20
#define LP8720_STARTUP_DELAY_2TS	0x40
#define LP8720_STARTUP_DELAY_3TS	0x60
#define LP8720_STARTUP_DELAY_4TS	0x80
#define LP8720_STARTUP_DELAY_5TS	0xa0
#define LP8720_STARTUP_DELAY_6TS	0xc0
#define LP8720_NO_STARTUP		0xe0


static unsigned char lp8720_output_status = 0x00; //default on 0x3F
struct i2c_client *lp8720_client=NULL;
static void lp8720_init(void);

/* <sunggyun.yu@lge.com> for recovery condition check */
static int lp8720_read_reg(struct i2c_client *client, u8 reg, u8 *data)
{
	int ret;
	u8 buf[1];
	struct i2c_msg msg[2];

	buf[0] = reg; 

	msg[0].addr = client->addr;
	msg[0].flags = 0;
	msg[0].len = 1;
	msg[0].buf = buf;

	msg[1].addr = client->addr;
	msg[1].flags = I2C_M_RD;
	msg[1].len = 1;
	msg[1].buf = buf;

	ret = i2c_transfer(client->adapter, msg, 2);
	if (ret != 2) 
		return -EIO;

	*data = buf[0];
	
	return 0;
}

static void lp8720_write_reg(struct i2c_client *client, u8 reg, u8 data)
{
	int err;

	struct i2c_msg msg;
	u8 buf[2];

	msg.addr = (u16)client->addr;
	msg.flags =0;
	msg.len =2;

	buf[0]=reg;
	buf[1]=data;

	msg.buf = &buf[0];

	if ((err = i2c_transfer(client->adapter, &msg, 1)) < 0) {
		dev_err(&client->dev, "i2c write error\n");
	}

	return;
}

void subpm_set_output(subpm_output_enum outnum, int onoff)
{
	if(outnum > 5) {
		dev_err(&lp8720_client->dev, "outnum error\n");
		return;
	}

	if(onoff == 0)
		lp8720_output_status &= ~(1<<outnum);
	else
		lp8720_output_status |= (1<<outnum);
	//printk(KERN_ERR "%s num %d on %d\n", __func__, outnum, onoff);
}

void subpm_output_enable(void)
{
	u8 data;
	struct lp8720_platform_data *pdata;

	if(lp8720_client == NULL)
	{
		printk(KERN_ERR "%s lp8720_client NULL\n", __func__);
		return;
	}

	pdata = lp8720_client->dev.platform_data;

#if 0
	lp8720_read_reg(lp8720_client, LP8720_GENERAL_SETTING, &data);
	printk(KERN_ERR "%s GENERAL_SETTING %02x\n", __func__, data);
	lp8720_read_reg(lp8720_client, LP8720_OUTPUT_ENABLE, &data);
	printk(KERN_ERR "%s OUTPUT_ENABLE %02x\n", __func__, data);
	lp8720_read_reg(lp8720_client, LP8720_LDO1_SETTING, &data);
	printk(KERN_ERR "%s LDO1_SETTING %02x\n", __func__, data);
	lp8720_read_reg(lp8720_client, LP8720_LDO2_SETTING, &data);
	printk(KERN_ERR "%s LDO2_SETTING %02x\n", __func__, data);
	lp8720_read_reg(lp8720_client, LP8720_LDO3_SETTING, &data);
	printk(KERN_ERR "%s LDO3_SETTING %02x\n", __func__, data);
	lp8720_read_reg(lp8720_client, LP8720_LDO4_SETTING, &data);
	printk(KERN_ERR "%s LDO4_SETTING %02x\n", __func__, data);
	lp8720_read_reg(lp8720_client, LP8720_LDO5_SETTING, &data);
	printk(KERN_ERR "%s LDO5_SETTING %02x\n", __func__, data);
	lp8720_read_reg(lp8720_client, LP8720_BUCK_SETTING1, &data);
	printk(KERN_ERR "%s BUCK_SETTING1 %02x\n", __func__, data);
	lp8720_read_reg(lp8720_client, LP8720_BUCK_SETTING2, &data);
	printk(KERN_ERR "%s BUCK_SETTING2 %02x\n", __func__, data);
#endif

	lp8720_write_reg(lp8720_client, LP8720_OUTPUT_ENABLE, 0x80 | lp8720_output_status);

	/* <sunggyun.yu@lge.com> recover settings after thermal shutdown or any reset */
#if 0
	lp8720_read_reg(lp8720_client, LP8720_LDO5_SETTING, &data);
	//printk(KERN_ERR "LP8720_LDO5_SETTING %02x\n", data);
	if (data == 0xff) {
		lp8720_init();
		printk(KERN_ERR "%s LP8720 registers recovered.\n", __func__);
	}
#endif
}

EXPORT_SYMBOL(subpm_set_output);
EXPORT_SYMBOL(subpm_output_enable);

static void lp8720_init(void)
{
	struct lp8720_platform_data *pdata = lp8720_client->dev.platform_data;

	lp8720_write_reg(lp8720_client, LP8720_LDO1_SETTING, LP8720_NO_STARTUP | 0x1F); //3.3v - MMC0
	lp8720_write_reg(lp8720_client, LP8720_LDO2_SETTING, LP8720_NO_STARTUP | 0x1d); //3.0v - MOTOR
	lp8720_write_reg(lp8720_client, LP8720_LDO3_SETTING, /*LP8720_STARTUP_DELAY_3TS*/LP8720_NO_STARTUP | 0x17); //2.7v
	lp8720_write_reg(lp8720_client, LP8720_LDO4_SETTING, LP8720_NO_STARTUP | 0x11); //1.8v
	lp8720_write_reg(lp8720_client, LP8720_LDO5_SETTING, LP8720_NO_STARTUP | 0x19); //2.8v
	lp8720_write_reg(lp8720_client, LP8720_BUCK_SETTING1, LP8720_NO_STARTUP | 0x09);
	lp8720_write_reg(lp8720_client, LP8720_BUCK_SETTING2, 0x09); //1.2v
	lp8720_write_reg(lp8720_client, LP8720_OUTPUT_ENABLE, 0x80 | lp8720_output_status);
	lp8720_write_reg(lp8720_client, LP8720_PULLDOWN_BITS, 0xBF); //don't  reset registers after thermal shutdown.

	gpio_direction_output(pdata->en_gpio_num, 1);

	return;
}

static int __init lp8720_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct lp8720_platform_data *pdata;

	if (i2c_get_clientdata(client))
		return -EBUSY;

	pdata = client->dev.platform_data;
	gpio_request(pdata->en_gpio_num, "lp8720");

	lp8720_client = client;
	lp8720_init();

	printk("$$$$$$$$$$$$$$$ lp8720 $$$$$$$$$$$$\n");

	return 0;
}

static int lp8720_remove(struct i2c_client *client)
{
	struct lp8720_platform_data *pdata;
	pdata = client->dev.platform_data;

	gpio_direction_output(pdata->en_gpio_num, 0);
	return 0;
}	

static int lp8720_suspend(struct i2c_client *client, pm_message_t mesg)
{
	struct lp8720_platform_data *pdata;
	pdata = client->dev.platform_data;

	/*if sdcard is inserted, power must be alive*/
	if (lp8720_output_status & 0x01) return 0;

	//printk("skykrkrk %s status %02x\n", __func__, lp8720_output_status);
	gpio_direction_output(pdata->en_gpio_num, 0);
	return 0;
}

static int lp8720_resume(struct i2c_client *client)
{
	/*if sdcard is inserted, power must be alive*/
	if (lp8720_output_status & 0x01) {
		return 0;
	}

	lp8720_init();

	return 0;
}

static const struct i2c_device_id lp8720_ids[] = {
	{ LP8720_I2C_NAME, 0 },	/*lp8720*/
	{ /* end of list */ },
};

static struct i2c_driver subpm_lp8720_driver = {
	.probe		= lp8720_probe,
	.remove		= lp8720_remove,
	.suspend	= lp8720_suspend,
	.resume		= lp8720_resume,
	.id_table	= lp8720_ids,
	.driver		= {
		.name = LP8720_I2C_NAME,
		.owner = THIS_MODULE,
	},
};


static int __init subpm_lp8720_init(void)
{
	return i2c_add_driver(&subpm_lp8720_driver);
}

static void __exit subpm_lp8720_exit(void)
{
	i2c_del_driver(&subpm_lp8720_driver);
}

#ifdef CONFIG_MACH_LGE_OMAP3
arch_initcall(subpm_lp8720_init);
#else
module_init(subpm_lp8720_init);
#endif
module_exit(subpm_lp8720_exit);

MODULE_AUTHOR("LG Electronics");
MODULE_DESCRIPTION("LP8720 Regulator Driver");
MODULE_LICENSE("GPL");

