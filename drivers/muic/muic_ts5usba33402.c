 /*
  * TI MUIC TS5USBA33402 driver
  *
  * Copyright (C) 2010, 2011, 2012 LGE Inc.
  *
  * Author: Sookyoung Kim <sookyoung.kim@lge.com>
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
#include <linux/charger_rt9524.h>
#include <linux/muic/muic.h>

#include <asm/system.h>
#include <asm/gpio.h>

#include <lge/board.h>
#define DEBUGE_TSU5611
#define DEBUGE

#include <linux/muic/muic_ts5usba33402.h>

#if defined(CONFIG_MHL_TX_SII9244) || defined(CONFIG_MHL_TX_SII9244_LEGACY)
extern void MHL_On(bool on);
#endif

/* LGE_CHANGE [kenneth.kang@lge.com] 2011-01-06, CP retain mode */
static int is_cp_retained;

extern atomic_t muic_charger_detected;

static struct i2c_client *ts5usba33402_client;
//static struct work_struct ts5usba33402_wq;
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
 * 1) Prepare to sense INT_STAT and STATUS bits.
 * 2) Open MUIC paths. -> To keep the path from uboot setting, remove this stage.
 */ 
void muic_init_ts5usba33402(struct i2c_client *client, TYPE_RESET reset)
{
	dev_info(&client->dev, "muic: %s\n", __func__);

	//[jongho3.lee@lge.com]  muic detecting...
	//atomic_set(&muic_charger_detected, 0); 
#ifdef DEBUGE_TSU5611
	muic_i2c_write_byte(client, SW_CONTROL, DP_OPEN|DM_OPEN);
	muic_i2c_write_byte(client, CONTROL_1, ID_200 | SEMREN);
#ifdef CONFIG_MUIC_TSU5611_INIT_BUG_FIX
	/* When boot up timing, CHG_TYPE must be set within TSU5611.
	 * This is chip bug of TSU5611.
	 * Eg: SU540, KU5400, LU5400
	 * TODO: boot up init and reset init may be seperated,
	 *       cause those function implementation is ambigous
	 *       hunsoo.lee@lge.com
	 */
	if (BOOTUP = reset) { /* TSU5611 BUG fix */
		dev_info(&client->dev, "muic: %s, init by BOOTUP\n", __func__);
		muic_i2c_write_byte(client, CONTROL_2, CHG_TYPE);

		for (loop = 0; loop < 250; loop++)
			udelay(1000);
		muic_i2c_write_byte(client, CONTROL_2,
				INT_EN | CP_AUD | CHG_TYPE);
	}
	else {
		muic_i2c_write_byte(client, CONTROL_2, INT_EN)
	}
#else
	muic_i2c_write_byte(client, CONTROL_2, INT_EN);
#endif /* CONFIG_TSU5611_INIT_BUG_FIX */
#else
	/* Clear Default Switch Position (0x03=0x24) */
	muic_i2c_write_byte(client, SW_CONTROL, COMP2_TO_HZ | COMN1_TO_HZ); 

  	/*
  	 * Iniialize MAX14526 for Detection of Accessories
  	 * Enable 200K pull-up and ADC (0x01=0x12)
  	 * Enable Interrupt and set AUD Click/Pop resistor (0x02=0x50)
  	 */
	muic_i2c_write_byte(client, CONTROL_1, ID_200 | SEMREN);
	muic_i2c_write_byte(client, CONTROL_2, CHG_TYPE);

	for (loop = 0; loop < 250; loop++)
		udelay(1000);

	muic_i2c_write_byte(client, CONTROL_2, INT_EN | CP_AUD | CHG_TYPE);
#endif
}
EXPORT_SYMBOL(muic_init_ts5usba33402);


//!![S] 2011-07-04 by pilsu.kim@lge.com : 
void muic_set_mhl_mode_detect(struct i2c_client *client)
{
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

	muic_set_mode(MUIC_MHL);
}
//!![E] 2011-07-04 by pilsu.kim@lge.com :
void muic_set_factory_mode_detect(struct i2c_client *client)
{
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

  	muic_set_mode(MUIC_CP_UART); 
}

//!![S] 2011-07-20 by pilsu.kim@lge.com : 
#if (0)
void muic_set_develop_mode_detect(struct i2c_client *client)
{
#if 0
	/* Connects CP USB signals to DP3T */
	usif_switch_ctrl(USIF_DP3T);

	dp3t_switch_ctrl(DP3T_CP_USB);
#endif
  	/* Enables 200K, Charger Pump, and ADC (0x01=0x13) */
   	muic_i2c_write_byte(client, CONTROL_1, ID_200 | SEMREN | CP_EN);

  	/* Enable USB Path (0x03=0x00) */
	muic_i2c_write_byte(client, SW_CONTROL, COMP2_TO_U2 | COMN1_TO_U1);

   	muic_set_mode(MUIC_CP_USB);
}
#else
void muic_set_develop_mode_detect(struct i2c_client *client)
{
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
  
   	muic_set_mode(MUIC_AP_UART);
}
#endif
//!![E] 2011-07-20 by pilsu.kim@lge.com : 

void muic_set_special_test_mode_detect(struct i2c_client *client) //UART_MODE
{
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

	muic_set_mode(MUIC_CP_UART);  // CHECK
}

void muic_set_usb_mode_detect(struct i2c_client *client)
{
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

	muic_set_mode(MUIC_AP_USB);
}

void muic_set_charger_detect(struct i2c_client *client,
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

		///////////////////////////////////////////////
		// This line should be modified by a customer.
		// 
		// Wait for Interrupt
		//
		////////////////////////////////////////////////

		/* Read INT_STAT */
		ret = muic_i2c_read_byte(client, INT_STAT, &reg_value);
		ret = muic_i2c_read_byte(client, STATUS, &reg_value);

		if (reg_value & DCPORT) {  
			printk("Charger ID22222\n");
			muic_i2c_write_byte(client, CONTROL_1, ID_200 | SEMREN);
			muic_i2c_write_byte(client, CONTROL_2, INT_EN); 
			muic_set_mode(MUIC_NA_TA);
		} else {
			printk("Charger ID33333\n");
			muic_set_usb_mode_detect(client);
		}
  	}
}

#ifdef DEBUGE_TSU5611
void TSU5611_Reset(struct i2c_client *client)  
{      
	muic_i2c_write_byte(client, 0x09, 0x80);	// hidden register
	muic_i2c_write_byte(client, 0x0b, 0xa8);	// hidden register

	// delay
	mdelay(300);
}
#endif

#ifdef DEBUGE_TSU5611
void muic_set_device_none_detect(struct i2c_client *client,
		unsigned char int_stat_value)
{
	s32 loop = 0;
	unsigned char charger_value = 0;

	unsigned char stat_value = 0;
	unsigned char Accessory_value = 0;
	unsigned char orginal_value = 0;
	int i;

	dev_info(&client->dev, "muic: %s: is_cp_retain = 0x%x\n", __func__,
			is_cp_retained);
	
	if(int_stat_value & VBUS) {// VBUS  = High
		dev_info(&client->dev, "muic: %s: VBUS is High....\n", __func__);
		if(is_cp_retained) {
			if (BOOT_CP_USB == is_cp_retained) {
				dev_info(&client->dev, "muic: USB is CP retain\n");
				muic_set_develop_mode_detect(client);
			}
			else if (BOOT_AP_USB == is_cp_retained) {
				dev_info(&client->dev, "muic: USB is AP retain\n");
				muic_set_usb_mode_detect(client);
			}

			goto RetainLabel;
		}
#if defined(CONFIG_MUIC_TS5USBA33402_REV_B)
		if(Int_Status == First_Int) {
#ifdef DEBUGE    
			dev_info(&client->dev, "muic: >> First Interrupt\n");
#endif
			muic_i2c_write_byte(client, 0x09, 0x80);//open hidden register
			muic_i2c_write_byte(client, 0x0b, 0xa8);//MUIC reset

			 //delay
			for (i=0; i < 300; i++)
				udelay(1000);   // delay is must 300ms ~ 600ms

			dev_info(&client->dev, "\n\nmuic: Reset is done\n");
			// SEND_ALL_Regs(1);
			Int_Status = Second_Int;
			 //Processorcommand = PMIC_AccessorInit;// MUIC initial
#if 0    
			printk("\r PMIC_AccessorInit starts \r\n");
#endif  
			goto RetainLabel;
		}
		else {  
#ifdef DEBUGE
			dev_info(&client->dev, "muic: << Second Interrupt\n");
#endif
			//g_API_Function.INT();// normal int rootin
			Int_Status = First_Int;
		}
#endif
		if(int_stat_value & CHGDET) { // Charger detection
			dev_info(&client->dev, "muic: Charger detection....\n");
			// set CHG_TYP to 1 -> Interrupt disable -> Wait for 250ms -> read status 
			muic_i2c_write_byte(client, CONTROL_2, 0x02); 
				
			// if charger type then waiting 250ms
			for (loop = 0; loop < 250; loop++)
				udelay(1000);
				
			muic_i2c_read_byte(client, STATUS, &charger_value); // 06H register read
			dev_info(&client->dev, "muic: STATUS register = %x\n", charger_value);
			muic_i2c_read_byte(client, INT_STAT, &stat_value);
			dev_info(&client->dev, "muic: INT_STAT111 register = %x\n", stat_value);

			if(charger_value & DCPORT) { // charger detected 
				muic_i2c_read_byte(client, INT_STAT,
						&charger_value);
				charger_value = charger_value & 0xf;
				if(charger_value == IDNO_1010) { // IDNO_1010 is 910k
					dev_info(&client->dev, "muic: LG Charger detection....\n");
					// LG Charger function call
					muic_set_charger_detect(client, charger_value);
					Int_Status = First_Int;
				}
				else {
#ifdef DEBUGE
					dev_info(&client->dev, "muic: Other charger detection....\n");
#endif
					// other charger function call
					muic_set_charger_detect(client, charger_value);
					Int_Status = First_Int;
				}
			}
			else {
				if(charger_value & CHPORT) {
#ifdef DEBUGE
					dev_info(&client->dev, "muic: Host/Hub charger detection....\n");
#endif
					// Host/Hub charger, Turn on USB SW, Enable USB Transceiver, Enable charger
					muic_set_charger_detect(client, charger_value);
					Int_Status = First_Int;
			   
				}
				else {
#ifdef DEBUGE
					dev_info(&client->dev, "muic: Invaild charger detection....\n");
#endif
					//Invaild charger, Enable charger
					muic_set_charger_detect(client, charger_value);
					Int_Status = First_Int;
				}
			}
		}
		else { // No charger detection
			dev_info(&client->dev, "muic: Accessory Type detection....\n");
			muic_i2c_read_byte(client, STATUS, &Accessory_value);
			dev_info(&client->dev, "muic: STATUS register = %x\n", Accessory_value);

			if(!(Accessory_value & C1COMP)) {
				muic_i2c_read_byte(client, INT_STAT, &stat_value);
				orginal_value = stat_value;
				dev_info(&client->dev, "muic: INT_STAT111 register = %x\n", stat_value);
				stat_value = stat_value & 0xf;
				dev_info(&client->dev, "muic: INT_STAT222 register = %x\n", stat_value);
				if(stat_value == 0) {
#ifdef DEBUGE
					dev_info(&client->dev, "muic: USB is  OTG....\n");
#endif
					//OTG
					muic_set_mhl_mode_detect(client);
					Int_Status = First_Int;
				}
				else if(stat_value == IDNO_0010) { // IDNO=0010? 56Kohm
#ifdef DEBUGE
					dev_info(&client->dev, "muic: USB is  Factory USB....\n");
#endif

					//Factory USB
					muic_set_develop_mode_detect(client);
					Int_Status = First_Int;
				}
				else if(stat_value == IDNO_0100) { // DNO=0100? 130Kohm :: UART_MODE 
#ifdef DEBUGE
					dev_info(&client->dev, "muic: USB is Factory UART....\n");
#endif
					//Factory UART
					muic_set_factory_mode_detect(client);
					Int_Status = First_Int;
				}
				else if(stat_value == IDNO_0101) { // IDNO_0101 is 180k AP USB
#ifdef DEBUGE
					dev_info(&client->dev, "muic: USB is LG USB....\n");
#endif
					//LG USB
					muic_set_usb_mode_detect(client);
					Int_Status = First_Int;
				}
				else if (stat_value == IDNO_1001) { //IDNO=1001? 620Kohm 
					dev_info(&client->dev, "muic: USB is 620k....\n");

					muic_set_develop_mode_detect(client);
					Int_Status = First_Int;
				}
				else if(stat_value == IDNO_1010) { //IDNO=1010? 910Kohm 
#ifdef DEBUGE
					dev_info(&client->dev, "muic: USB is 910k Cable....\n");
#endif
					//Unknown Cable
					//Processorcommand = TSTCHRG; 

					muic_set_develop_mode_detect(client);
					Int_Status = First_Int;
			  
				}
				else if ((orginal_value == 0x30) || (orginal_value == 0x11)) {
					dev_info(&client->dev, "muic: USB is CP USB....\n");
					muic_set_special_test_mode_detect(client);
					Int_Status = First_Int;
				}
				else if(orginal_value == 0x10) {
#ifdef DEBUGE
					dev_info(&client->dev, "muic: MHL....\n");
#endif
					//Unknown Cable
					//PMIC_Reset();
					//g_API_Function.INT();
					muic_set_mhl_mode_detect(client);
					Int_Status = First_Int;
				}
				else if (orginal_value == 0x5b) {
					dev_info(&client->dev, "muic: USB is 0x5b\n");
					muic_set_usb_mode_detect(client);
					Int_Status = First_Int;
				}
				else if (stat_value == IDNO_0110) { //IDNO_0110 is 160k
					dev_info(&client->dev, "muic: USB is 160k and 0x6\n");
					muic_set_usb_mode_detect(client);
					Int_Status = First_Int;
				}
				else {
#ifdef DEBUGE
					dev_info(&client->dev, "muic: *********************\n");
					dev_info(&client->dev, "muic: *USB is Other USB....*\n");
					dev_info(&client->dev, "muic: *********************\n");
#endif
					//Other USB
					//Processorcommand = TSTCHRG; 

					/*if (Other_usb_count < 3)
					{
						printk("USB is Other count=%d\n", Other_usb_count);
						Other_usb_count = Other_usb_count+1;
						TSU5611_Reset();
						muic_init_ts5usba33402(DEFAULT);	// init rutain again
						muic_mode = MUIC_UNKNOWN;
						Int_Status = Second_Int;
					}
					else
					{*/
						//printk("USB is Other AP USB\n");
						muic_set_usb_mode_detect(client);
						Int_Status = First_Int;
					//}
					
					//TSU5611_Reset();

					//muic_init_ts5usba33402(DEFAULT);	// init rutain again
						
				}
			}
			else {
#ifdef DEBUGE
				dev_info(&client->dev, "muic: USB is a abnormal charger....\n");
#endif
				// Abnormal Charger
				muic_set_usb_mode_detect(client);
				Int_Status = First_Int;
			}
		}
	}
	else { // VBUS = Low
#ifdef DEBUGE
		dev_info(&client->dev, "muic: VBUS is low....\n");
#endif
		if (!(int_stat_value & MR_COMP)) { // Charger detection
			if (int_stat_value == 0x04) {
				dev_info(&client->dev, "muic: BUS is a 0x4 cp uart\n");
				muic_set_factory_mode_detect(client);
					Int_Status = First_Int;
			}
			else if (int_stat_value == 0x00) {
				dev_info(&client->dev, "muic: BUS is a 0x00 MHL gender\n");
				muic_set_mode(MUIC_UNKNOWN);
				Int_Status = First_Int;
			}
			else {
				// muic idle status
				muic_set_mode(MUIC_UNKNOWN);
				Int_Status = First_Int;
			}
		}
		else {
#ifdef DEBUGE
			dev_info(&client->dev, "muic: Idle mode, MR_COMP is high....\n");
#endif
			//Idel mode
			if (int_stat_value == 0x4b) {
				dev_info(&client->dev, "muic: USB is a 0x4b TAKE OFF\n");
				//muic_set_factory_mode_detect(client);
				Int_Status = First_Int;
			}
			//Idel mode
			muic_set_mode(MUIC_UNKNOWN);
		}
	}

	return;

RetainLabel:
	dev_info(&client->dev, "muic: USB retain mode\n");
}

#else
void muic_set_device_none_detect(struct i2c_client *client,
		unsigned char int_stat_value)
{
	unsigned char reg_value;
	s32 ret = 0;
	
	dev_info(&client->dev, "muic: %s: is_cp_retain = 0x%x\n", __func__,
			is_cp_retained);

	if(is_cp_retained) {
		muic_set_develop_mode_detect(client);
	} else if((int_stat_value & IDNO) == IDNO_0100 /*||int_stat_value == 0x11*/) {
		/* IDNO=0100? 130Kohm :: UART_MODE */
		muic_set_factory_mode_detect(client);
	} else if ((int_stat_value & IDNO) == IDNO_0010 ||
		   (int_stat_value & IDNO) == IDNO_1010 ||
		   (int_stat_value & IDNO) == IDNO_1001) {
		/* IDNO=0010? 56Kohm , IDNO=1010? 910Kohm ,IDNO=1001? 620Kohm :: USB_MODE */
		muic_set_develop_mode_detect(client);
	//!![S] 2011-07-04 by pilsu.kim@lge.com : delete code for mhl detect
#if defined(CONFIG_MHL_TX_SII9244) || defined(CONFIG_MHL_TX_SII9244_LEGACY)
	} else if ((int_stat_value == 0x30) || (int_stat_value == 0x11)) {
		muic_set_special_test_mode_detect(client);
#else
	} else if ((int_stat_value == 0x30) ||
		   (int_stat_value == 0x11) ||
		   (int_stat_value == 0x10)) {
		muic_set_special_test_mode_detect(client);
#endif
	//!![E] 2011-07-04 by pilsu.kim@lge.com : 
	} else if (int_stat_value & CHGDET) {
		muic_set_charger_detect(int_stat_value);
	} else if (int_stat_value & VBUS) {
		   
		/* Read a status reigster(0x05) */
		ret = muic_i2c_read_byte(client, STATUS, &reg_value);

		if (reg_value) {
			/* Charger Detected*/
			/* COMP2/COMN1 to H-Z (0x03=0x24) */
			muic_i2c_write_byte(client, SW_CONTROL, COMP2_TO_HZ | COMN1_TO_HZ);			
			muic_set_mode(MUIC_NA_TA);
		} else {
			//!![S] 2011-07-04 by pilsu.kim@lge.com : add detect for mhl cable 
#if defined(CONFIG_MHL_TX_SII9244) || defined(CONFIG_MHL_TX_SII9244_LEGACY)
			if ((int_stat_value & IDNO ) == IDNO_0000) {
				muic_set_mhl_mode_detect(client);			
			} else {
				///* Standard USB Host Detected (0x03=0x23) */				
				muic_set_usb_mode_detect(client); 	// Turn on USB switches 	
			}
#else
			///* Standard USB Host Detected (0x03=0x23) */				
			muic_set_usb_mode_detect(); 	// Turn on USB switches 	
#endif
			//!![E] 2011-07-04 by pilsu.kim@lge.com : 
		}	

	} else if ((int_stat_value & IDNO ) == IDNO_0001) {
		/* Vidoe Cable 24k registor Detected - No TV Attached */
		//current_device = DEVICE_VIDEO_CABLE_NO_LOAD;
	} else if ((int_stat_value & IDNO) == IDNO_0000 ) {
		muic_set_mode(MUIC_NONE);
	} else {
		/* Accessory Not Supported */
		muic_set_mode(MUIC_NONE);
	}
}
#endif

s32 muic_ts5usba33402_detect_accessory(struct i2c_client *client, s32 upon_irq)
{
	s32 ret = 0;
	s32 loop = 0;
	TYPE_MUIC_MODE muic_mode = muic_get_mode();
	struct ts5usb_device *dev = i2c_get_clientdata(client);

	u8 int_stat_value;

	/*
	 * Upon an MUIC IRQ (MUIC_INT_N falls),
	 * wait 70ms before reading INT_STAT and STATUS.
	 * After the reads, MUIC_INT_N returns to high
	 * (but the INT_STAT and STATUS contents will be held).
	 *
	 * Do this only if muic_ts5usba33402_detect_accessory() was called upon IRQ. 
	 */
	if (upon_irq) {
		for (loop = 0; loop < 70; loop++)
			udelay(1000);
	}	

	/* Read INT_STAT */
	ret = muic_i2c_read_byte(client, INT_STAT, &int_stat_value);
	dev_info(&client->dev, "muic: %s: int_stat_value:0x%x :%d\n", __func__,
			int_stat_value, muic_mode);

	if (ret < 0) {
		dev_info(&client->dev, "muic: INT_STAT reading failed\n");
		muic_set_mode(MUIC_UNKNOWN);

		return ret;
	}
	// printk(KERN_INFO "[MUIC] IDNO = %d\n", (int_stat_value & IDNO));
	// ret = muic_i2c_write_byte(client, CONTROL_2, 0x00); //interrupt masked by ks.kwon@lge.com for debugging

	/* Branch according to the previous muic_mode */
	switch (muic_mode) {

	/* 
	 * MUIC_UNKNOWN is reached in two cases both do not have nothing to do with IRQ.
	 * First, at the initialization time where the muic_mode is not available yet.
	 * Second, whenever the current muic_mode detection is failed.
	 */
	case MUIC_UNKNOWN :

	/*
	 * If the previous muic_mode was MUIC_NONE,
	 * the only possible condition for a MUIC IRQ is plugging in an accessory.
	 */
	case MUIC_NONE :
		muic_set_device_none_detect(client, int_stat_value);
		break;

	/* 
	 * If the previous muic_mode was MUIC_NA_TA, MUIC_LG_TA, MUIC_TA_1A, MUIC_INVALID_CHG,
	 * MUIC_AP_UART, MUIC_CP_UART, MUIC_AP_USB, MUIC_OTG, or MUIC_CP_USB,
	 * the only possible condition for a MUIC IRQ is plugging out the accessory.
	 * 
	 * In this case, initialize MUIC and wait an IRQ.
	 * We don't need to wait 250msec because this is not an erronous case
	 * (we need to reset the facility to set STATUS for an erronous case and
	 * have to wait 250msec) and, if this is not an erronous case, the facility
	 * was already initialized at the system booting.
	 */
	case MUIC_AP_UART :
	case MUIC_CP_UART :

		dev_info(&client->dev, "muic: Detect step1 MUIC_AP_UART || MUIC_CP_UART\n");
		if ((int_stat_value & IDNO) == IDNO_1011) {	        
			muic_i2c_write_byte(client, SW_CONTROL, COMP2_TO_HZ | COMN1_TO_HZ);
			/* Exit Factory Mode */
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
		/* 
		 * Check if VBUS == 0 && IDNO == 0x1011 (open).
		 * If so, it's MUIC_NONE.
		 * Otherwise, it's an erronous situation. MUIC_UNKNOWN.
		 */
		if ((int_stat_value & VBUS) == 0) {
			/* USB Host Removed */
			muic_set_mode(MUIC_NONE);
		}
		break;
	//!![S] 2011-07-05 by pilsu.kim@lge.com : 
#if defined(CONFIG_MHL_TX_SII9244) || defined(CONFIG_MHL_TX_SII9244_LEGACY)
	case MUIC_MHL :
		dev_info(&client->dev, "muic: Detect step3  mhl \n");
		if ((int_stat_value & VBUS) == 0) {
			//MHL_On(0);
			muic_set_mode(MUIC_NONE);
		}
		break;
#endif
	//!![E] 2011-07-05 by pilsu.kim@lge.com : 
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
				muic_init_ts5usba33402(client, DEFAULT);
				gpio_set_value(dev->gpio_ifx_vbus, 0);
			}
			else {
				dev_info(&client->dev, "muic: mhl switch not completed\n");
				while (!gpio_get_value(dev->gpio_mhl)) {
					udelay(500);
					MHL_On(0);
				}
				dev_info(&client->dev, "muic: mhl switch completed\n");
				muic_init_ts5usba33402(client, DEFAULT);
				gpio_set_value(dev->gpio_ifx_vbus, 0);
			}
		}
		else {
			muic_init_ts5usba33402(client, DEFAULT);
			gpio_set_value(dev->gpio_ifx_vbus, 0);
		}
#else
		muic_init_ts5usba33402(client, DEFAULT);
		//charging_ic_deactive();
		dev_info(&client->dev, "muic: charging_ic_deactive()\n");
#endif
	}

	return ret;
}

/* LGE_CHANGE_S [kenneth.kang@lge.com] 2011-07-26, CP retain mode */
static int __init muic_state(char *str)
{
	s32 muic_value = simple_strtol(str, NULL, 0);
	is_cp_retained = muic_value;
	printk(KERN_INFO "muic: CP Retain : %d\n", muic_value);

	return 1;
}
__setup("muic_state=", muic_state);
/* LGE_CHANGE_E [kenneth.kang@lge.com] 2011-07-26, CP retain mode */

static void ts5usba33402_wq_func(struct work_struct *work)
{
	s32 ret = 0;
	struct ts5usb_device *dev = container_of(work,
			struct ts5usb_device, muic_wq);
	struct i2c_client *client = dev->client;

	wake_lock(&dev->muic_wake_lock);

	dev_info(&client->dev, "muic: %s()\n", __func__);
	
	ret = muic_ts5usba33402_detect_accessory(client, UPON_IRQ);
	//[jongho3.lee@lge.com]
	dev_info(&client->dev, "muic: muic_detect_accessory(UPON_IRQ)detedted!!!!\n");
	//muic_set_charger_detected();	
	wake_unlock(&dev->muic_wake_lock);
}

static irqreturn_t ts5usba33402_interrupt_handler(s32 irq, void *data)
{
	struct ts5usb_device *dev = data;
	/* Make the interrupt on MUIC INT wake up OMAP which is in suspend mode */
	schedule_work(&dev->muic_wq);
	return IRQ_HANDLED;
}

/* LGE_SJIT 2012-01-27 [dojip.kim@lge.com] define muic_device */
static struct muic_device muic_dev = {
	.name = "ts5usba33402",
};

static int __devinit ts5usba33402_probe(struct i2c_client *client,
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

	ts5usba33402_client = client;

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
	INIT_WORK(&dev->muic_wq, ts5usba33402_wq_func);

	/* 
	 * Set up an IRQ line and enable the involved interrupt handler.
	 * From this point, a MUIC_INT_N can invoke muic_interrupt_handler().
	 * muic_interrupt_handler merely calls schedule_work() with muic_wq_func().
	 * muic_wq_func() actually performs the accessory detection.
	 */
	ret = request_irq(dev->irq, ts5usba33402_interrupt_handler,
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

	/* Initializes MUIC - Finally MUIC INT becomes enabled */
	if (BOOT_FIRST == is_cp_retained) { /* Recovery mode */
		muic_mode = MUIC_CP_UART;
		muic_init_ts5usba33402(client, BOOTUP);
		dev_info(&client->dev, "muic: %s: first boot\n", __func__);
	}
	else {
		muic_init_ts5usba33402(client, BOOTUP);
		muic_ts5usba33402_detect_accessory(client, NOT_UPON_IRQ);
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

static int __devexit ts5usba33402_remove(struct i2c_client *client)
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

static const struct i2c_device_id ts5usba33402_ids[] = {
	{"ts5usba33402", 0},
	{/* end of list */},
};
MODULE_DEVICE_TABLE(i2c, ts5usba33402_ids);

static struct i2c_driver ts5usba33402_driver = {
	.probe	  = ts5usba33402_probe,
	.remove	  = __devexit_p(ts5usba33402_remove),
//	.suspend  = ts5usba33402_suspend,
//	.resume   = ts5usba33402_resume,
	.id_table = ts5usba33402_ids,
	.driver	  = {
		.name	= "ts5usba33402",
		.owner	= THIS_MODULE,
	},
};

static int __init ts5usba33402_init(void)
{
	return i2c_add_driver(&ts5usba33402_driver);
}

static void __exit ts5usba33402_exit(void)
{
	i2c_del_driver(&ts5usba33402_driver);
}

module_init(ts5usba33402_init);
module_exit(ts5usba33402_exit);

MODULE_AUTHOR("LG Electronics");
MODULE_DESCRIPTION("ts5usba33402 MUIC Driver");
MODULE_LICENSE("GPL");
