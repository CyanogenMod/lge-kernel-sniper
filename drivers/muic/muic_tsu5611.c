 /*
  * TI MUIC TSU5611 driver
  *
  * Copyright (C) 2012 LGE Inc.
  *
  * Author: HunSoo Lee <hunsoo.lee@lge.com>
  *         Seungho Park <seungho1.park@lge.com>
  *         2011/11/22:change driver structure to use MUIC subsystem.
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
  */

#include <linux/module.h>
#include <linux/kernel.h>	/* printk() */
#include <linux/init.h>		/* __init, __exit */
#include <linux/uaccess.h>	/* copy_from/to_user() */
#include <linux/interrupt.h>	/* request_irq() */
#include <linux/irq.h>		/* set_irq_type() */
#include <linux/types.h>	/* kernel data types */
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/device.h>
#include <linux/delay.h>	/* usleep() */
#include <linux/proc_fs.h>
#include <linux/workqueue.h>	/* INIT_WORK() */
#include <linux/wakelock.h>
#include <linux/muic/muic.h>
#if !defined(CONFIG_MAX8971_CHARGER)
#include <linux/charger_rt9524.h>
#else
#include <linux/max8971.h>
#endif

#include <asm/system.h>
#include <asm/gpio.h>

#include <lge/board.h>
#define DEBUGE_TSU5611
#define DEBUGE

#include <linux/muic/muic_tsu5611.h>

static int muic_retain_mode;

extern atomic_t muic_charger_detected;

static struct i2c_client *tsu5611;
//static struct work_struct tsu5611_wq;
//static int Other_usb_count =0;
static int Int_Status = 0;

typedef enum int_{
	First_Int = 0,
	Second_Int,
}INT;

/* LGE_SJIT 2012-01-27 [dojip.kim@lge.com]
 * Add private device handle
 */
struct ts5usb_device {
	struct i2c_client *client;
	int gpio_int;
	int gpio_mhl;
	int gpio_ifx_vbus;
	int irq;
	struct work_struct muic_wq;
	struct wake_lock muic_wake_lock;
};

/*
 * Initialize MUIC, i.e., the CONTROL_1,2 and SW_CONTROL registers.
 * 1) Prepare to sense INT_STATUS1 and STATUS bits.
 * 2) Open MUIC paths. -> To keep the path from uboot setting, remove this stage.
 */
void muic_init_tsu5611(struct i2c_client *client, TYPE_RESET reset)
{
	dev_info(&client->dev, "muic: %s\n", __func__);

	muic_i2c_write_byte(client, SW_CONTROL, DP_OPEN|DM_OPEN);
	muic_i2c_write_byte(client, CONTROL_1, ID_200 | SEMREN);
	/* When boot up timing, CHG_TYPE must be set within TSU5611.
	 * This is chip bug of TSU5611.
	 * Eg: SU540, KU5400, LU5400
	 * TODO: boot up init and reset init may be seperated,
	 *       cause those function implementation is ambigous
	 *       hunsoo.lee@lge.com
	 */
	if (BOOTUP == reset) { /* TSU5611 BUG fix */
		dev_info(&client->dev, "muic: %s, init by BOOTUP\n", __func__);
		muic_i2c_write_byte(client, CONTROL_2, CHG_TYPE);
		mdelay(250);

		muic_i2c_write_byte(client, CONTROL_2,
				INT_EN | CP_AUD | CHG_TYPE);
	}else {
		muic_i2c_write_byte(client, CONTROL_2, INT_EN);
	}
}
EXPORT_SYMBOL(muic_init_tsu5611);


int muic_set_mhl_mode(struct muic_client_device *mcdev)// (struct i2c_client *client)
{
	struct i2c_client *client = tsu5611; //to_i2c_client(%mcdev->dev);// i2c_verify_client(&mcdev->dev); 

	dev_info(&client->dev, "muic: %s entry.\n", __func__);
#if 0
	/* Connect CP UART signals to AP */
	usif_switch_ctrl(USIF_AP);
	/*
	 * AP USB does not pass through DP3T.
	 * Just connect AP UART to MUIC UART.
	 */
	dp3t_switch_ctrl(DP3T_NC);
#endif
	//muic_i2c_write_byte(SW_CONTROL, COMP2_TO_DP2 | COMN1_TO_DN1);
	muic_i2c_write_byte(client, SW_CONTROL, COMP2_TO_HZ | COMN1_TO_HZ);

  	/* Enable 200K, Charger Pump, and ADC (0x01=0x13) */
	//muic_i2c_write_byte(CONTROL_1, ID_200 | SEMREN | CP_EN);
#if 0
	MHL_On(1);
#endif

//	muic_set_mode(MUIC_MHL);
}

int muic_set_cp_usb_mode(struct muic_client_device *mcdev)// (struct i2c_client *client)
{
	struct i2c_client *client = tsu5611; //to_i2c_client(%mcdev->dev);// i2c_verify_client(&mcdev->dev);

	if(!client)
	{
		printk("muic %s, client : %ld", __func__, (long)client);
		return -1;
	}

	dev_info(&client->dev, "muic: %s entry.\n", __func__);
#if 0
	/* Connects CP UART signals to DP3T */
	usif_switch_ctrl(USIF_DP3T);

	/* Connects CP UART to MUIC UART */
	dp3t_switch_ctrl(DP3T_CP_UART);
#endif
	/* Enables 200K, Charger Pump, and ADC (0x01=0x13)  */
  	muic_i2c_write_byte(client, CONTROL_1, ID_200 | SEMREN | CP_EN);

  	/* Enables UART Path (0x03=0x00) */
  	muic_i2c_write_byte(client, SW_CONTROL, COMP2_TO_U2 | COMN1_TO_U1);

//  	muic_set_mode(MUIC_CP_UART);
	return 0;
}

int muic_set_ap_uart_mode(struct muic_client_device *mcdev)// (struct i2c_client *client)
{
	struct i2c_client *client = tsu5611; //to_i2c_client(%mcdev->dev);// i2c_verify_client(&mcdev->dev);

	dev_info(&client->dev, "muic: %s\n", __func__);
#if 0
	/* Connects CP UART signals to AP */
	usif_switch_ctrl(USIF_AP);
	/* Connects AP UART to MUIC UART */
	dp3t_switch_ctrl(DP3T_AP_UART);
#endif

  	/* Enables 200K, Charger Pump, and ADC (0x01=0x13) */
   	muic_i2c_write_byte(client, CONTROL_1, ID_200 | SEMREN | CP_EN);

  	/* Enable USB Path (0x03=0x00) */
	muic_i2c_write_byte(client, SW_CONTROL, COMP2_TO_U2 | COMN1_TO_U1);

//   	muic_set_mode(MUIC_AP_UART);
	return 0;
}

int muic_set_cp_uart_mode(struct muic_client_device *mcdev) //UART_MODE
{
	struct i2c_client *client = tsu5611; //to_i2c_client(%mcdev->dev);// i2c_verify_client(&mcdev->dev);

	if(!client)
	{
		printk("muic %s, client : %ld", __func__, (long)client);
		return -1;
	}

	dev_info(&client->dev, "muic: %s\n", __func__);
#if 0
	/* Connects CP UART signals to DP3T */
	usif_switch_ctrl(USIF_DP3T);

	/* Connects CP UART to MUIC UART */
	dp3t_switch_ctrl(DP3T_CP_UART);
#endif
	/* Enables 200K, Charger Pump, and ADC (0x01=0x13)  */
	muic_i2c_write_byte(client, CONTROL_1, ID_200 | SEMREN | CP_EN);

	/* Enables USB Path (0x03=0x00) */
	muic_i2c_write_byte(client, SW_CONTROL, COMP2_TO_U2 | COMN1_TO_U1);

	return 0;
//	muic_set_mode(MUIC_CP_UART);  // CHECK
}

int muic_set_ap_usb_mode(struct muic_client_device *mcdev)// (struct i2c_client *client)
{
	struct i2c_client *client = tsu5611; //to_i2c_client(%mcdev->dev);// i2c_verify_client(&mcdev->dev);

	if(!client)
	{
		printk("muic %s, client : %d", __func__, client);
		return -1;
	}

	dev_info(&client->dev, "muic: %s entry.\n", __func__);

	mdelay(200);
	muic_i2c_write_byte(client, SW_CONTROL, COMP2_TO_HZ | COMN1_TO_HZ);

	mdelay(200);
#if 0
	/* Connect CP UART signals to AP */
	usif_switch_ctrl(USIF_AP);
	/*
	 * AP USB does not pass through DP3T.
	 * Just connect AP UART to MUIC UART.
	 */
	dp3t_switch_ctrl(DP3T_AP_UART);
#endif

	muic_i2c_write_byte(client, SW_CONTROL, COMP2_TO_DP2 | COMN1_TO_DN1);

  	/* Enable 200K, Charger Pump, and ADC (0x01=0x13) */
   	muic_i2c_write_byte(client, CONTROL_1, ID_200 | SEMREN | CP_EN);

//	muic_set_mode(MUIC_AP_USB);
	return 0;
}

void muic_set_charger_mode(struct i2c_client *client,
		unsigned char int_stat_value)
{
	s32 ret = 0;
  	unsigned char reg_value;

	dev_info(&client->dev, "muic: Charger ID = 0x%x\n", int_stat_value);

  	if (((int_stat_value & IDNO) == IDNO_0101) ||
	    ((int_stat_value & IDNO) == IDNO_1011)) {
		/*LG Proprietary TA Detected 180K ohm on ID */
		muic_i2c_write_byte(client, CONTROL_1, ID_200 | SEMREN);
		muic_i2c_write_byte(client, CONTROL_2, INT_EN);
		muic_set_mode(MUIC_LG_TA);
	} else if ((int_stat_value & IDNO) == IDNO_0110) {
		/* 1A charger detected */
		muic_set_mode(MUIC_TA_1A);
	} else {
		dev_info(&client->dev, "muic: Charger ID11111\n");
		/* Enable interrpt and charger type detection (0x02=0x42) */
		muic_i2c_write_byte(client, CONTROL_2, INT_EN | CHG_TYPE);

		/* Read INT_STATUS1 */
		ret = muic_i2c_read_byte(client, INT_STATUS1, &reg_value);
		ret = muic_i2c_read_byte(client, STATUS, &reg_value);

		if (reg_value & DCPORT) {
			printk("Charger ID22222\n");
			muic_i2c_write_byte(client, CONTROL_1, ID_200 | SEMREN);
			muic_i2c_write_byte(client, CONTROL_2, INT_EN);
			muic_set_mode(MUIC_NA_TA);
		} else {
			printk("Charger ID33333\n");
			muic_set_mode(MUIC_AP_USB);
//			muic_set_usb_mode_detect(client);
		}
  	}
}

int muic_process_retention_mode(struct i2c_client *client)
{
	if(muic_retain_mode <0 ){
		printk(KERN_ERR "muic: %s, muic_retain_mode:%d is wrong state\n",
			__func__, muic_retain_mode);
		muic_retain_mode = 0;

		return -EINVAL;
	}

	if (BOOT_CP_USB == muic_retain_mode) {
		//dev_info(&client->dev, "muic: USB is CP retain\n");
		muic_set_mode(MUIC_CP_USB);
		//muic_set_develop_mode_detect(client);
	}
	else if (BOOT_AP_USB == muic_retain_mode) {
		//dev_info(&client->dev, "muic: USB is AP retain\n");
		muic_set_mode(MUIC_AP_USB);
		//muic_set_usb_mode_detect(client);
	}else if (BOOT_MHL == muic_retain_mode){
		muic_set_mode(MUIC_MHL);
	}else if (BOOT_MHL == muic_retain_mode){
		muic_set_mode(MUIC_CP_UART);
	}

	return muic_retain_mode;
}

int tsu5611_process_reset_muic(struct i2c_client *client)
{
	if(Int_Status == First_Int) {
		dev_info(&client->dev, "muic: trying reset n redetect cause of TSU5611 chipset bug\n");
		muic_i2c_write_byte(client, 0x09, 0x80);//open hidden register
		muic_i2c_write_byte(client, 0x0b, 0xa8);//MUIC reset

		mdelay(300); // delay is must 300ms ~ 600ms after reset

		dev_info(&client->dev, "muic:  device reset is done\n");
		Int_Status = Second_Int;

//		goto RetainLabel;
	}
	else {
		dev_info(&client->dev, "muic: << Second Interrupt\n");
		Int_Status = First_Int;
	}

	return Int_Status;
}

#ifdef DEBUGE_TSU5611
int muic_set_device_none_detect(struct i2c_client *client,
		unsigned char int_stat_value)
{
//	s32 loop = 0;
	unsigned char charger_value = 0;

	unsigned char stat_value = 0;
//	unsigned char Accessory_value = 0;
//	unsigned char orginal_value = 0;
	unsigned char id_register_value = 0;

//	int i;

	id_register_value = 0x0f & int_stat_value;

	dev_info(&client->dev, "muic: %s, muic_retain_mode:%d, Int_Status:%d, INT_STATUS:0x%2x\n",
		__func__,muic_retain_mode, Int_Status, int_stat_value);


	if(int_stat_value & VBUS) {// VBUS  = High
		dev_info(&client->dev, "muic: %s: VBUS is High....\n", __func__);

		if(muic_process_retention_mode(client))
			return 0;


		if(Second_Int == tsu5611_process_reset_muic(client))
			return 0;


		if( (int_stat_value & CHGDET) || (charger_value & CHPORT))
		{
			dev_info(&client->dev, "muic: Charger detection....\n");
			muic_i2c_write_byte(client, CONTROL_2, 0x02);

			mdelay(250); // if charger type then waiting 250ms

			muic_i2c_read_byte(client, STATUS, &charger_value); // 06H register read
			muic_i2c_read_byte(client, INT_STATUS1, &stat_value);
			dev_info(&client->dev, "muic: STATUS register:0x%x, INT_STATUS1 register:0x%x\n",
				charger_value, stat_value);

			muic_set_charger_mode(client, charger_value);
//			Int_Status = First_Int;
		}else if((id_register_value == IDNO_0010) // 56k + VBUS
			|| (id_register_value == IDNO_1010) // 910k + VBUS
			|| (id_register_value == IDNO_1001)) // 620k + VBUS
		{
			muic_set_mode(MUIC_CP_USB);
		}else if(id_register_value == IDNO_0100){
			muic_set_mode(MUIC_CP_UART);
		}else if(IDNO_0000 ==id_register_value){
			muic_set_mode(MUIC_MHL);

#if 0
		}else if(0){
			muic_set_mode(MUIC_AP_UART);
#endif
		}else{
			muic_set_mode(MUIC_AP_USB);
#if 0
		}else{
			muic_set_mode(MUIC_UNKNOWN);
#endif
		}
	}else{
#if 0 
		if(IDNO_0010 == id_register_value){ // 56k + no VBUS
			muic_set_mode(MUIC_AP_UART);
		}else{
			muic_set_mode(MUIC_NONE);
		}
#endif
			muic_set_mode(MUIC_NONE);

	}

	return 0;
}
#endif

s32 muic_tsu5611_detect_accessory(struct i2c_client *client, s32 upon_irq)
{
	s32 ret = 0;
	s32 loop = 0;
	TYPE_MUIC_MODE muic_mode = muic_get_mode();
	struct ts5usb_device *dev = i2c_get_clientdata(client);

	u8 int_stat_value;

	/*
	 * Upon an MUIC IRQ (MUIC_INT_N falls),
	 * wait 70ms before reading INT_STATUS1 and STATUS.
	 * After the reads, MUIC_INT_N returns to high
	 * (but the INT_STATUS1 and STATUS contents will be held).
	 *
	 * Do this only if muic_tsu5611_detect_accessory() was called upon IRQ.
	 */
	if (upon_irq) {
		mdelay(70);
	}

	/* Read INT_STATUS1 */
	ret = muic_i2c_read_byte(client, INT_STATUS1, &int_stat_value);
	dev_info(&client->dev, "muic: %s: int_stat_value:0x%x :%d\n", __func__,
			int_stat_value, muic_mode);

	if (ret < 0) {
		dev_info(&client->dev, "muic: INT_STATUS1 reading failed\n");
		muic_set_mode(MUIC_UNKNOWN);

		return ret;
	}

	switch (muic_mode) {/* Branch according to the previous muic_mode */

		case MUIC_UNKNOWN :
		case MUIC_NONE :
			muic_set_device_none_detect(client, int_stat_value);
			break;

		case MUIC_AP_UART :
		case MUIC_CP_UART :
			dev_info(&client->dev, "muic: MUIC_AP_UART || MUIC_CP_UART\n");
			if(!(int_stat_value & VBUS)
				&&(IDNO_0010 == (int_stat_value & 0x0f))){

				muic_set_mode(MUIC_CP_USB);
			} else if (!(int_stat_value & VBUS)){
				muic_i2c_write_byte(client, SW_CONTROL, COMP2_TO_HZ | COMN1_TO_HZ);
				muic_set_mode(MUIC_NONE);
			}
			break;

		case MUIC_NA_TA :
		case MUIC_LG_TA :
		case MUIC_TA_1A :
		case MUIC_INVALID_CHG :
			dev_info(&client->dev, "muic: Detect step2\n");
			if (((int_stat_value & VBUS) == 0) ||
			    ((int_stat_value & CHGDET) == 0)) {
				muic_set_mode(MUIC_NONE);
			}

			break;

		case MUIC_AP_USB :
		case MUIC_CP_USB :
#if 0
			if( IDNO_0010 == (int_stat_value & 0x0f) && (!(int_stat_value & VBUS)) ){
				muic_set_mode(MUIC_AP_UART);
			}else if ((int_stat_value & VBUS) == 0) {
#endif
			{
				/* USB Host Removed */
				muic_set_mode(MUIC_NONE);
			}
			break;
#if defined(CONFIG_MHL_TX_SII9244) || defined(CONFIG_MHL_TX_SII9244_LEGACY)
		case MUIC_MHL :
			dev_info(&client->dev, "muic: Detect step3  mhl \n");
			if ((int_stat_value & VBUS) == 0) {
				//MHL_On(0);
				muic_set_mode(MUIC_NONE);
			}
			break;
#endif
		default:
			dev_info(&client->dev, "muic: Failed to detect an accessory. Try again!");
			muic_set_mode(MUIC_UNKNOWN);
			ret = -1;
			break;
	}

	muic_mode = muic_get_mode();
	dev_info(&client->dev, "muic: muic_detection_mode=%d \n", muic_mode);

	if (muic_mode == MUIC_UNKNOWN || muic_mode == MUIC_NONE) {
#if defined(CONFIG_MHL_TX_MUIC_BUG_FIX)
		/* SJIT 2012-01-27 [dojip.kim@lge.com] P940 GB
		 * tsu5611 300ms delay side effect bug fixed test code
		 */
		if (Int_Status == Second_Int) {
			if (!gpio_get_value(dev->gpio_mhl)) {
				dev_info(&client->dev, "muic: wait for mhl switch completed\n");
				muic_init_tsu5611(client, DEFAULT);
				gpio_set_value(dev->gpio_ifx_vbus, 0);
			}
			else {
				dev_info(&client->dev, "muic: mhl switch not completed\n");
				while (!gpio_get_value(dev->gpio_mhl)) {
					udelay(500);
					muic_set_mode(MUIC_MHL);
//					MHL_On(0);
				}
				dev_info(&client->dev, "muic: mhl switch completed\n");
				muic_init_tsu5611(client, DEFAULT);
				gpio_set_value(dev->gpio_ifx_vbus, 0);
			}
		}
		else {
			muic_init_tsu5611(client, DEFAULT);
			gpio_set_value(dev->gpio_ifx_vbus, 0);
		}
#else
		muic_init_tsu5611(client, DEFAULT);
		//charging_ic_deactive();
		dev_info(&client->dev, "muic: charging_ic_deactive()\n");
#endif
	}

// LGE_CHANGES_S [dukwung.kim@lge.com] 2012-03-19
#if defined(CONFIG_MAX8971_CHARGER)
	        if(muic_mode == MUIC_NA_TA || muic_mode == MUIC_LG_TA || muic_mode == MUIC_TA_1A)
		{
		        printk("[MUIC] max8971_start_charging TA\n");
		        max8971_start_charging(900);
									         }
		else if(muic_mode == MUIC_AP_USB)
		{
		        printk("[MUIC]max8971_start_charging USB\n");
		        max8971_start_charging(500);
										        }
		/* else if( muic_mode == MUIC_CP_USB || muic_mode ==  MUIC_CP_UART)
		{
		        printk("[MUIC] max8971_start_charging Factory\n");
		        max8971_start_charging(1550);
		}
		  */
		else if( muic_mode == MUIC_CP_USB )
		{
		 //printk("[MUIC] max8971_stop due to MUIC_CP_USB mode \n");
		        printk("[MUIC] max8971_start_charging Factory MUIC_CP_USB \n");
		        // max8971_start_charging(1550);
		        // max8971_stop_charging();
		        max8971_start_Factory_charging();
		}
		else if(  muic_mode ==  MUIC_CP_UART)
		{
		         printk("[MUIC] max8971_start_charging Factory MUIC_CP_UART \n");
	                //printk("[MUIC] max8971_stop due to MUIC_CP_UART mode \n");
	                // max8971_stop_charging();
	                // max8971_start_charging(1550);
	                 max8971_start_Factory_charging();
	        }
	        else if( muic_mode == MUIC_MHL)
	        {
	                printk("[MUIC] max8971_start_charging MHL\n");
	                max8971_start_charging(400);
	        }
	        else if( muic_mode == MUIC_NONE)
	        {
	                printk("[MUIC] max8971_stop_charging\n");
	                max8971_stop_charging();
		}
		else
		       printk("[MUIC] can not open muic mode\n");
		// LGE_CHANGES_E [dukwung.kim@lge.com] 2012-03-19
#endif
	return ret;
}

/* LGE_CHANGE_S [kenneth.kang@lge.com] 2011-07-26, CP retain mode */
static int __init muic_state(char *str)
{
	s32 muic_value = simple_strtol(str, NULL, 0);
	muic_retain_mode = muic_value;
	printk(KERN_INFO "muic: Retain : %d\n", muic_value);

	return 1;
}
__setup("muic_state=", muic_state);/* LGE_CHANGE_E [kenneth.kang@lge.com] 2011-07-26, CP retain mode */


static void tsu5611_wq_func(struct work_struct *work)
{
	s32 ret = 0;
	struct ts5usb_device *dev = container_of(work,
			struct ts5usb_device, muic_wq);
	struct i2c_client *client = dev->client;

	wake_lock(&dev->muic_wake_lock);

	dev_info(&client->dev, "muic: %s()\n", __func__);

	ret = muic_tsu5611_detect_accessory(client, UPON_IRQ);
//	dev_info(&client->dev, "muic: muic_detect_accessory(UPON_IRQ)detedted!!!!\n");
	wake_unlock(&dev->muic_wake_lock);
}

static irqreturn_t tsu5611_interrupt_handler(s32 irq, void *data)
{
	struct ts5usb_device *dev = data;
	/* Make the interrupt on MUIC INT wake up OMAP which is in suspend mode */
	schedule_work(&dev->muic_wq);
	return IRQ_HANDLED;
}

static int muic_int_stat_read(struct muic_client_device *mcdev,
	char *buf)
{	
	struct i2c_client *client = tsu5611; //to_i2c_client(%mcdev->dev);// i2c_verify_client(&mcdev->dev);

	u32 ret;
	u32 value;
	unsigned int len;

	ret = muic_i2c_read_byte(client, INT_STATUS1, &value);
//	value = i2c_smbus_read_byte_data(muic_client, INT_STAT);
	dev_info(&client->dev, "[MUIC] INT_STAT = 0x%02x\n", (0xff & value));
	
	len = sprintf(buf, "%02x\n", 0xff & value);

	return len;
}



/* LGE_SJIT 2012-01-27 [dojip.kim@lge.com] define muic_device */
static struct muic_device muic_dev = {
	.name = "tsu5611",
	.read_int_state	= muic_int_stat_read,
};

static struct muic_client_ops tsu5611_ops = {
	.on_ap_uart = muic_set_ap_uart_mode,
	.on_ap_usb 	= muic_set_ap_usb_mode,
	.on_cp_uart = muic_set_cp_uart_mode,
	.on_cp_usb 	= muic_set_cp_usb_mode,
	//.on_mhl		= muic_set_mhl_mode,
};

static int __devinit tsu5611_probe(struct i2c_client *client,
		const struct i2c_device_id *id)
{
	int ret = 0;
	TYPE_MUIC_MODE muic_mode;
	struct ts5usb_device *dev = NULL;
	struct muic_platform_data *pdata = client->dev.platform_data;

	dev_info(&client->dev, "muic: %s()\n", __func__);

	if (!pdata) {
		dev_err(&client->dev, "muic: %s: no platform data\n", __func__);
		return -EINVAL;
	}

	dev = kzalloc(sizeof(struct ts5usb_device), GFP_KERNEL);
	if (!dev) {
		dev_err(&client->dev, "muic: %s: no memory\n", __func__);
		return -ENOMEM;
	}

	dev->client = client;
	dev->gpio_int = pdata->gpio_int;
	dev->gpio_mhl = pdata->gpio_mhl;
	dev->gpio_ifx_vbus = pdata->gpio_ifx_vbus;
	dev->irq = client->irq;

	tsu5611 = client;

	/*
	 * Initializes gpio_wk8 (MUIC_INT_N).
	 * Checks if other driver already occupied it.
	 */
	ret = gpio_request(dev->gpio_int, "MUIC IRQ GPIO");
	if (ret < 0) {
		dev_err(&client->dev, "muic: GPIO %d is already used\n",
				dev->gpio_int);
		ret = -ENOSYS;
		goto err_gpio_request;
	}
	/* Initializes GPIO direction before use or IRQ setting */
	gpio_direction_input(dev->gpio_int);

	/* Registers MUIC work queue function */
	INIT_WORK(&dev->muic_wq, tsu5611_wq_func);

	/*
	 * Set up an IRQ line and enable the involved interrupt handler.
	 * From this point, a MUIC_INT_N can invoke muic_interrupt_handler().
	 * muic_interrupt_handler merely calls schedule_work() with muic_wq_func().
	 * muic_wq_func() actually performs the accessory detection.
	 */
	ret = request_irq(dev->irq, tsu5611_interrupt_handler,
			  IRQF_TRIGGER_FALLING, "muic_irq", dev);
	if (ret < 0) {
		dev_err(&client->dev, "muic: %s: request_irq failed!\n",
				__func__);
		goto err_request_irq;
	}

	//disable_irq_wake(gpio_to_irq(MUIC_INT));

	/* Prepares a human accessible method to control MUIC */
	//create_cosmo_muic_proc_file();

	/* Selects one of the possible muic chips */
	//muic_detect_device();

	wake_lock_init(&dev->muic_wake_lock, WAKE_LOCK_SUSPEND,
			"muic_wake_lock");

	ret = muic_device_register(&muic_dev, NULL);
	if (ret < 0) {
		dev_err(&client->dev, "muic: %s: muic_device_register failed\n",
				__func__);
		goto err_muic_device_register;
	}

	i2c_set_clientdata(client, dev);

	// hunsoo.lee
	printk("%s, registering ops\n", __func__);
	muic_client_dev_register(dev->client->name, dev, &tsu5611_ops);

	/* Initializes MUIC - Finally MUIC INT becomes enabled */
	if (BOOT_RECOVERY == muic_retain_mode) { /* Recovery mode */
		muic_mode = MUIC_CP_UART;
		muic_init_tsu5611(client, BOOTUP);
		dev_info(&client->dev, "muic: %s: first boot\n", __func__);
	}
	else {
		muic_init_tsu5611(client, BOOTUP);
		muic_tsu5611_detect_accessory(client, NOT_UPON_IRQ);
		muic_mode = muic_get_mode();
	}
	muic_set_mode(muic_mode);

	/* Makes the interrupt on MUIC INT wake up OMAP which is in suspend mode */
	ret = enable_irq_wake(dev->irq);
	if (ret < 0) {
		dev_err(&client->dev, "muic: GPIO %d wake up source setting failed!\n", dev->gpio_int);
		disable_irq_wake(dev->irq);
		goto err_irq_wake;
	}

	dev_info(&client->dev, "muic: muic_probe()\n");

	return ret;

err_irq_wake:
	muic_device_unregister(&muic_dev);
err_muic_device_register:
	wake_lock_destroy(&dev->muic_wake_lock);
	free_irq(dev->irq, dev);
err_request_irq:
	gpio_free(dev->gpio_int);
err_gpio_request:
	kfree(dev);

	return ret;
}

static int __devexit tsu5611_remove(struct i2c_client *client)
{
	struct ts5usb_device *dev = i2c_get_clientdata(client);

	/* LGE_SJIT 2012-01-27 [dojip.kim@lge.com] unregister muic device */
	muic_device_unregister(&muic_dev);
	cancel_work_sync(&dev->muic_wq);
	wake_lock_destroy(&dev->muic_wake_lock);
	free_irq(dev->irq, dev);
	gpio_free(dev->gpio_int);
	i2c_set_clientdata(client, NULL);
	kfree(dev);
//	remove_cosmo_muic_proc_file();
	return 0;
}

static const struct i2c_device_id tsu5611_ids[] = {
	{"tsu5611", 0},
	{/* end of list */},
};
MODULE_DEVICE_TABLE(i2c, tsu5611_ids);



static struct i2c_driver tsu5611_driver = {
	.probe	  = tsu5611_probe,
	.remove	  = __devexit_p(tsu5611_remove),
//	.suspend  = tsu5611_suspend,
//	.resume   = tsu5611_resume,
	.id_table = tsu5611_ids,
	.driver	  = {
		.name	= "tsu5611",
		.owner	= THIS_MODULE,
	},
};

static int __init tsu5611_init(void)
{
	return i2c_add_driver(&tsu5611_driver);
}

static void __exit tsu5611_exit(void)
{
	i2c_del_driver(&tsu5611_driver);
}

module_init(tsu5611_init);
module_exit(tsu5611_exit);

MODULE_AUTHOR("LG Electronics");
MODULE_DESCRIPTION("tsu5611 MUIC Driver");
MODULE_LICENSE("GPL");
