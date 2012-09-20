/* drivers/i2c/chips/akm8973.c - akm8973 compass driver
 *
 * Copyright (C) 2007-2008 HTC Corporation.
 * Author: Hou-Kun Chen <houkun.chen@gmail.com>
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
 */

#include <linux/interrupt.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/irq.h>
#include <linux/miscdevice.h>
#include <asm/gpio.h>
#include <asm/uaccess.h>
#include <linux/delay.h>
#include <linux/input.h>
#include <linux/workqueue.h>
#include <linux/freezer.h>
#include "akm8973.h"
//#include <mach/vreg.h> // antispoon1026 delete
//#include <linux/proximity.h>//antispoon1026 delete
//#include <mach/system.h> //for reading REV. in Board. //antispoon1026

#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif

#define MAX_FAILURE_COUNT 10
#define GPIO_COMPASS_RESET 55//antispoon1026	

#define DEBUG 0 /*Detailed log  in EEPROM*/ //antispoon1119
#define AKMD_DEBUG 0
#define NO_COMPASS_SUPPORT  1
#if AKMD_DEBUG
#define ADBG(fmt, args...) printk(fmt, ##args)
#else
#define ADBG(fmt, args...) do {} while (0)
#endif /* AKMD_DEBUG */

//LGE_CHANGE_S [antispoon@lge.com] 2009-10-27 pmic control define for omap
#define ENABLE_REGULATOR
#define ENABLE_TWL_POWER
#define OPTIMUM_TEST 0
#if OPTIMUM_TEST
#include "kxtf9.h"    
#endif
#include <linux/i2c/twl.h>
#include <linux/regulator/consumer.h>
#define TWL4030_CHIP_PM_RECEIVER    0x4b

#define VMMC2_DEV_GRP       0x1B //0x2B //antispoon1230
#define VMMC2_DEDICATED     0x1E //0x2E
#define ENABLE_VMMC2_P1 0x20 //0x20
#define SET_VMMC2_V30 0x05 //0x0B 0x05=1.8V 0x07=2.5V 0x0B=3.0V antispoon0101
#define VAUX_DEV_GRP_NONE   0x00
#ifdef ENABLE_REGULATOR
static struct regulator *compass_reg; //antispoon1230
static struct regulator *compass_io_reg;
#endif
//LGE_CHANGE_E [antispoon@lge.com] 2009-10-27

static unsigned short normal_i2c[] = { I2C_CLIENT_END };

I2C_CLIENT_INSMOD;

static struct i2c_client *this_client;

struct akm8973_data {
	struct input_dev *input_dev;
//	struct work_struct work;
#ifdef CONFIG_HAS_EARLYSUSPEND
	struct i2c_client *client;
	struct early_suspend early_suspend; /* 20110304 seven.kim@lge.com late_resume_lcd */
#endif
};

/* Addresses to scan -- protected by sense_data_mutex */
//static char sense_data[RBUFF_SIZE + 1];
//static struct mutex sense_data_mutex;

//static DECLARE_WAIT_QUEUE_HEAD(data_ready_wq);
static DECLARE_WAIT_QUEUE_HEAD(open_wq);

//static atomic_t data_ready;
static atomic_t open_count;
static atomic_t open_flag;
static atomic_t reserve_open_flag;

static atomic_t m_flag;
static atomic_t a_flag;
static atomic_t t_flag;
static atomic_t mv_flag;
static atomic_t p_flag;
static int first_start = 0 ;

static int powerDownOrOff=0; //For used current //diyu@lge.com
static int akm8973_set_vreg_check=  0;

//static int failure_count = 0;

static short akmd_delay = 0;

#ifdef CONFIG_HAS_EARLYSUSPEND
//static atomic_t suspend_flag = ATOMIC_INIT(0);
#endif

//static struct akm8973_platform_data *pdata;
extern int sensor_reboot;

/* following are the sysfs callback functions */

#define config_ctrl_reg(name,address) \
static ssize_t name##_show(struct device *dev, struct device_attribute *attr, \
			   char *buf) \
{ \
	struct i2c_client *client = to_i2c_client(dev); \
        return sprintf(buf, "%u\n", i2c_smbus_read_byte_data(client,address)); \
} \
static ssize_t name##_store(struct device *dev, struct device_attribute *attr, \
			    const char *buf,size_t count) \
{ \
	struct i2c_client *client = to_i2c_client(dev); \
	unsigned long val = simple_strtoul(buf, NULL, 10); \
	if (val > 0xff) \
		return -EINVAL; \
	i2c_smbus_write_byte_data(client,address, val); \
        return count; \
} \
static DEVICE_ATTR(name, S_IWUSR | S_IRUGO, name##_show, name##_store)

config_ctrl_reg(ms1, AKECS_REG_MS1);
//antispoon1026 ---------------------------------->
//static int motion_is_off = 0;
static int is_proxi_open(void) {
    return 1;
}


//<- antispoon1026 --------------------------------
int AKI2C_RxData(char *rxData, int length)
{
	struct i2c_msg msgs[] = {
		{
		 .addr = this_client->addr,
		 .flags = 0,
		 .len = 1,
		 .buf = rxData,
		 },
		{
		 .addr = this_client->addr,
		 .flags = I2C_M_RD,
		 .len = length,
		 .buf = rxData,
		 },
	};

	int err;
	
#if DEBUG
	printk(KERN_INFO "%s\n", __FUNCTION__);
#endif

	if ((err = i2c_transfer(this_client->adapter, msgs, 2)) < 0) {
		if(err == -110)
			sensor_reboot = 1;
		
		printk(KERN_ERR "AKI2C_RxData: transfer error\n");
		return -EIO;
	} else {
        return 0;
    }
}

int AKI2C_TxData(char *txData, int length)
{
	struct i2c_msg msg[] = {
		{
		 .addr = this_client->addr,
		 .flags = 0,
		 .len = length,
		 .buf = txData,
		 },
	};

	int err;

#if DEBUG
	printk(KERN_INFO "%s\n", __FUNCTION__);
#endif

	if ((err = i2c_transfer(this_client->adapter, msg, 1)) < 0) {
		if(err == -110)
			sensor_reboot = 1;
		
		printk(KERN_ERR "AKI2C_TxData: transfer error\n");
		return -EIO;
	} else
		return 0;
}

/* LGE_CHANGE_S [jh.koo@lge.com] 2009-06-12 */
#if 0
static int akm8973_config_gpio(int config)
{
	int ret;
    //printk("akm8973_config_gpio(%d)\n", config);
#if 0
	if (config) {	// for wake state
        ret = gpio_direction_output(GPIO_COMPASS_RESET,0);
        if (ret < 0) printk ("fail to set gpio during akm8973 resume\n");
	} else {		// for sleep state
        ret = gpio_direction_input(GPIO_COMPASS_RESET);
        if (ret < 0) printk ("fail to set gpio during akm8973 suspend\n");
	}
#endif
	return 0;
}
#endif
int akm8973_set_vreg(int onoff)
{
//	struct vreg *vreg_compass_30;
//	struct vreg *vreg_compass_27;
//	int err, ret;
	if (onoff==1)
	{
        if (akm8973_set_vreg_check == 0 ) {
#ifdef ENABLE_REGULATOR 
            //regulator_enable(compass_io_reg);
            //ret = twl4030_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER, SET_VMMC2_V30,VMMC2_DEDICATED );
            //if (ret)     printk("antispoon motion power on fail1 \n");
            //ret = twl4030_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER, ENABLE_VMMC2_P1,VMMC2_DEV_GRP );
            //if (ret)     printk("antispoon motion power on fail2 \n");
            //regulator_enable(compass_reg);
#elif defined ENABLE_TWL_POWER
            ret = twl4030_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER, SET_VMMC2_V30,VMMC2_DEDICATED );
            if (ret)     printk("antispoon motion power on fail1 \n");
            ret = twl4030_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER, ENABLE_VMMC2_P1,VMMC2_DEV_GRP );
            if (ret)     printk("antispoon motion power on fail2 \n");
#endif //power on for motion power of heavem        
            msleep (50); //antispoon1027
#if 0
            gpio_direction_output(GPIO_COMPASS_RESET,0);
            msleep (10);
            gpio_direction_output(GPIO_COMPASS_RESET,1);
#endif

#ifdef ENABLE_REGULATOR
#if DEBUG        //test
        printk ("regulator level in akm8973 antispoon \n", regulator_get_voltage(compass_reg));//antispoon1123
#endif
#endif
        akm8973_set_vreg_check=1;
        }
    }
	else
	{
        if (akm8973_set_vreg_check == 1) {
#ifdef ENABLE_REGULATOR
            //regulator_disable(compass_reg);
            //regulator_disable(compass_io_reg);

#elif defined ENABLE_TWL_POWER
        //if (motion_is_off == 1) {
            ret = twl4030_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER, VAUX_DEV_GRP_NONE, VMMC2_DEV_GRP);
            if (ret)     printk("antispoon motion power off fail1 \n");
        //}
#endif
            akm8973_set_vreg_check=0;
        }
	}
	return 0;
}
EXPORT_SYMBOL(akm8973_set_vreg);
/* LGE_CHANGE_E [jh.koo@lge.com] 2009-06-12 */

static int AKECS_Init(void)
{
#if DEBUG
	printk(KERN_INFO "%s\n", __FUNCTION__);
#endif
	return 0;
}

static void AKECS_Reset(void)
{
#if 0
    gpio_set_value(GPIO_COMPASS_RESET, 0);
    udelay(120);
    gpio_set_value(GPIO_COMPASS_RESET, 1);
#endif

#if DEBUG
    printk(KERN_INFO "%s\n", __FUNCTION__);

#endif

}

#define COMPASS_RESET	91
#define RECOVER_EEPROM_COUNT 7
 
#if 0
static int AKECS_WriteEEPROM(void)
{
	struct akm8973_data *data = i2c_get_clientdata(this_client);
	char buffer[2];
	char aRecoverData[RECOVER_EEPROM_COUNT+1] = {0x26, 0x77, 0x66, 0xC7, 0x06, 0x06, 0x07};
	char i2cData[6];
	char data2;
	int ret, i;
#if DEBUG
	printk(KERN_INFO "%s\n", __FUNCTION__);
#endif
    msleep(100);
	
		/* Set to EEPROM read mode */
		buffer[0] = AKECS_REG_MS1;
		buffer[1] = AK8973_MS1_READ_EEPROM;
		
		ret = AKI2C_TxData(buffer, 2);
		if (ret < 0)
			return ret;
		msleep(1);

		/* Set to EEPROM read mode */
		buffer[0] = AK8973_EEP_ETST;
		/* Read data */
		ret = AKI2C_RxData(buffer, 1);
		if (ret < 0)
			return ret;
		else
			printk("%x\n", buffer);
		msleep(1);
		
		data2 = buffer[0];
		 if ( data2 == 0xC7)
	  	{
  			#if DEBUG
			printk("GOOD Device - Do nothing %x, %h\n", data2, data2);
			#endif
		
			// Set to PowerDown mode
			buffer[0] = AKECS_REG_MS1;
			buffer[1] = AKECS_MODE_POWERDOWN;
			
			ret = AKI2C_TxData(buffer, 2);
			if (ret < 0)
				return ret;
			msleep(1);
				return;
		   	// do nothing...
		}else {
	  		#if DEBUG
				ADBG("============0============\n");
				printk("Bad Device ETST = %x \n", data2);
			#endif
			
			// Set to PowerDown mode
			buffer[0] = AKECS_REG_MS1;
			buffer[1] = AKECS_MODE_POWERDOWN;
			
			ret = AKI2C_TxData(buffer, 2);
			if (ret < 0)
				return ret;
			msleep(1);
	    	#if DEBUG
				ADBG("============1============\n");
			#endif
		
			/* Set to EEPROM read mode */
			buffer[0] = AKECS_REG_MS1;
			buffer[1] = AK8973_MS1_WRITE_EEPROM;
			
			ret = AKI2C_TxData(buffer, 2);
			if (ret < 0)
				return ret;
			msleep(1);
			#if DEBUG
				ADBG("============2============\n");
			#endif
			for (i=0; i < RECOVER_EEPROM_COUNT; i++)
			{
			  /* Set to EEPROM read mode */
				buffer[0] = AK8973_EEP_ETS+i;
				buffer[1] = aRecoverData[i];

				ret = AKI2C_TxData(buffer, 2);	
			    ADBG("[%x]==>  %x \n", AK8973_EEP_ETS+i, aRecoverData[i] );
			  msleep(1);
			}
	 	 }
			
 
	 #if DEBUG
 		 // Set to PowerDown mode
 		 buffer[0] = AKECS_REG_MS1;
 		 buffer[1] = AKECS_MODE_POWERDOWN;
 		 
 		 ret = AKI2C_TxData(buffer, 2);
 		 if (ret < 0)
 			 return ret;
 		 msleep(1);
 	 
 		 /* Set to EEPROM read mode */
 		 buffer[0] = AKECS_REG_MS1;
 		 buffer[1] = AK8973_MS1_READ_EEPROM;
 		 
 		 ret = AKI2C_TxData(buffer, 2);
 		 if (ret < 0)
 			 return ret;
 		 msleep(1);
 	 
 	 	//*******************************************************************
 	 	//  Print Screen
	 	//*******************************************************************
	 	
		
		// Read EHXGA, EHYGA, EHZGA values
		i2cData[0] = AK8973_EEP_ETS;
		/* Read data */
		ret = AKI2C_RxData(i2cData, 1);
		if (ret < 0)
			return ret;
		
		for (i=0; i< RECOVER_EEPROM_COUNT; i++)
		{
  		#if DEBUG 
			ADBG("REGISTER [%x] = [%x] \n", AK8973_EEP_ETS+i, i2cData[i]);
  		#endif
		}
	#endif
		
		// Set to PowerDown mode
 		 buffer[0] = AKECS_REG_MS1;
 		 buffer[1] = AKECS_MODE_POWERDOWN;
 		 
 		 ret = AKI2C_TxData(buffer, 2);
 		 if (ret < 0)
 			 return ret;

}
#endif



static int AKECS_StartMeasure(void)
{
//	struct akm8973_data *data = i2c_get_clientdata(this_client);
	char buffer[2];
//	int ret;
#if DEBUG
	printk(KERN_INFO "%s\n", __FUNCTION__);
#endif

	if(powerDownOrOff){
		/* Set measure mode */
		buffer[0] = 0x0a;
//		buffer[1] = 1;
		buffer[1] = 2;

		/* Set data */
		return AKI2C_TxData(buffer, 2);
		
		//	ret = AKI2C_TxData(buffer, 2);
		//	if (ret < 0)
		//		return ret;
		
		//	msleep(20);
		//	schedule_work(&data->work);
		//	return ret;
	}else{
        akm8973_set_vreg(1); //antispoon0101
		
		buffer[0] = 0x0a;
//		buffer[1] = 1;
		buffer[1] = 2;

		/* Set data */
		return AKI2C_TxData(buffer, 2);
	
	
	}
	
}

static int AKECS_PowerDown(void)
{
	char buffer[2];
	int ret;
#if 1//DEBUG
//For_Resume_Speed	printk(KERN_INFO "%s\n", __FUNCTION__);
#endif

	/* Set powerdown mode */
	buffer[0] = 0x0a;
	buffer[1] = 0;
	/* Set data */
	ret = AKI2C_TxData(buffer, 2);
	if (ret < 0)
		return ret;

	/* Dummy read for clearing INT pin */
	//buffer[0] = AKECS_REG_TMPS;
	/* Read data */
	//ret = AKI2C_RxData(buffer, 1);
	//if (ret < 0)
	//	return ret;

	return ret;
}

//LGE_CHANGE_S diyu@lge.com.  To add for used current
#if 0
static int AKECS_PowerOff(void)
{
	//char buffer[2];
	int ret;
    ret = AKECS_PowerDown();
	akm8973_set_vreg(0);
	return 0;
	
}
#endif
//LGE_CHANGE_E diyu@lge.com


static int AKECS_StartE2PRead(void)
{
	char buffer[2];
#if DEBUG
	printk(KERN_INFO "%s\n", __FUNCTION__);
#endif

	/* Set E2P mode */
	buffer[0] = AKECS_REG_MS1;
	buffer[1] = AKECS_MODE_E2P_READ;
	/* Set data */
	return AKI2C_TxData(buffer, 2);
}

#if 0
static int AKECS_GetData(void)
{
	char buffer[RBUFF_SIZE + 1];
	int ret;
#if DEBUG
	printk(KERN_INFO "%s\n", __FUNCTION__);
#endif

	memset(buffer, 0, RBUFF_SIZE + 1);

	// read C0 - C4
	buffer[0] = AKECS_REG_ST;
	ret = AKI2C_RxData(buffer, RBUFF_SIZE + 1);
	if (ret < 0)
		return ret;

	mutex_lock(&sense_data_mutex);
	memcpy(sense_data, buffer, sizeof(buffer));
	atomic_set(&data_ready, 1);
	wake_up(&data_ready_wq);
	mutex_unlock(&sense_data_mutex);

	return 0;
}
#endif

int AKECS_SetMode(char mode)
{
	int ret;
#if DEBUG
	printk(KERN_INFO "%s\n", __FUNCTION__);
#endif

	switch (mode) {
	case AKECS_MODE_MEASURE:
		ret = AKECS_StartMeasure();
		break;
	case AKECS_MODE_E2P_READ:
		ret = AKECS_StartE2PRead();
		break;
	case AKECS_MODE_POWERDOWN:
		#if 0
		if(powerDownOrOff)
			ret = AKECS_PowerDown();
		else
			ret = AKECS_PowerOff();
		#else
		ret = AKECS_PowerDown();
		#endif

		break;
	default:
		return -EINVAL;
	}

	/* wait at least 300us after changing mode */
	msleep(1);
	return ret;
}

static int AKECS_TransRBuff(char *rbuf, int size)
{
#if 0
#if DEBUG
	printk(KERN_INFO "%s\n", __FUNCTION__);
#endif
	wait_event_interruptible_timeout(data_ready_wq,
					 atomic_read(&data_ready), 1000);

	if (!atomic_read(&data_ready)) {
#ifdef CONFIG_HAS_EARLYSUSPEND
		if (!atomic_read(&suspend_flag)) {
#endif
			printk(KERN_ERR "AKECS_TransRBUFF: Data not ready\n");
			failure_count++;
			if (failure_count >= MAX_FAILURE_COUNT) {
				printk(KERN_ERR
				       "AKECS_TransRBUFF: successive %d failure.\n",
				       failure_count);
				atomic_set(&open_flag, -1);
				wake_up(&open_wq);
				failure_count = 0;
			}
#ifdef CONFIG_HAS_EARLYSUSPEND
		}
#endif
		return -1;
	}

	mutex_lock(&sense_data_mutex);
	memcpy(&rbuf[1], &sense_data[1], size);
	atomic_set(&data_ready, 0);
	mutex_unlock(&sense_data_mutex);


	failure_count = 0;
	return 0;
#endif

	if(size < RBUFF_SIZE + 1)
	  return -EINVAL;

	// read C0 - C4
	rbuf[0] = AKECS_REG_ST;
	return AKI2C_RxData(rbuf, RBUFF_SIZE + 1);

}
#if NO_COMPASS_SUPPORT
void report_rotation_direct(int value)
{
    if (atomic_read(&a_flag)) {
        struct akm8973_data *data = i2c_get_clientdata(this_client);
        
        if (value == 99)
        {
            printk("[MOTION] Landscape report \n"); //landscape
            input_report_abs(data->input_dev, ABS_X, -40);
            input_report_abs(data->input_dev, ABS_Y, 645);
            input_report_abs(data->input_dev, ABS_Z, 40);
            input_sync(data->input_dev);
        }
        if (value == 98)
        {
            printk("[MOTION] Portrait report \n"); //portrat
            input_report_abs(data->input_dev, ABS_X, -700);
            input_report_abs(data->input_dev, ABS_Y, -65);
            input_report_abs(data->input_dev, ABS_Z, 40);
            input_sync(data->input_dev);
        }
    }
}
EXPORT_SYMBOL(report_rotation_direct);
#endif
static void AKECS_Report_Value(short *rbuf)
{
	struct akm8973_data *data = i2c_get_clientdata(this_client);
//	int i = 1;
//#if DEBUG
	printk("%s\n", __FUNCTION__);
//#endif
#if DEBUG
/*	printk("AKECS_Report_Value: yaw = %d, pitch = %d, roll = %d ", rbuf[0],
	       rbuf[1], rbuf[2]);
	printk("                    tmp = %d, m_stat= %d, g_stat=%d ", rbuf[3],
	       rbuf[4], rbuf[5]);
	printk("      Acceleration:   x = %d LSB, y = %d LSB, z = %d LSB\n",
	       rbuf[6], rbuf[7], rbuf[8]);
	printk("          Magnetic:   x = %d LSB, y = %d LSB, z = %d LSB",
	       rbuf[9], rbuf[10], rbuf[11]);*/ //antispoon test
#endif
#if 0
	/* Report magnetic sensor information */
	if (atomic_read(&m_flag)) {
		input_report_abs(data->input_dev, ABS_HAT0X, rbuf[0]);
		input_report_abs(data->input_dev, ABS_HAT0Y, rbuf[1]);
		input_report_abs(data->input_dev, ABS_BRAKE, rbuf[2]);
		input_report_abs(data->input_dev, ABS_RUDDER, rbuf[4]);
	}

	/* Report acceleration sensor information */
	if (atomic_read(&a_flag)) {
		input_report_abs(data->input_dev, ABS_RX, rbuf[6]);
		input_report_abs(data->input_dev, ABS_RY, rbuf[7]);
		input_report_abs(data->input_dev, ABS_RZ, rbuf[8]);
		input_report_abs(data->input_dev, ABS_WHEEL, rbuf[5]);
	}

	/* Report temperature information */
	if (atomic_read(&t_flag)) {
		input_report_abs(data->input_dev, ABS_THROTTLE, rbuf[3]);
	}

	if (atomic_read(&mv_flag)) {
		input_report_abs(data->input_dev, ABS_X, rbuf[9]);
		input_report_abs(data->input_dev, ABS_Y, rbuf[10]);
		input_report_abs(data->input_dev, ABS_Z, rbuf[11]);
	}

	/* Proximity */
	if(atomic_read(&p_flag))
		input_report_abs(data->input_dev, ABS_DISTANCE, rbuf[12]);
#else /*original*/

	/* Report magnetic sensor information */
	if (atomic_read(&m_flag)) {
		input_report_abs(data->input_dev, ABS_RX, rbuf[0]);
		input_report_abs(data->input_dev, ABS_RY, rbuf[1]);
		input_report_abs(data->input_dev, ABS_RZ, rbuf[2]);
		input_report_abs(data->input_dev, ABS_RUDDER, rbuf[4]);
	}

	/* Report acceleration sensor information */
	if (atomic_read(&a_flag)) {
//#if DEBUG
        printk("\n antispoon report_accel %d,%d,%d,%d \n",rbuf[6], rbuf[7], rbuf[8], rbuf[5]);
//#endif
		input_report_abs(data->input_dev, ABS_X, rbuf[6]);
		input_report_abs(data->input_dev, ABS_Y, rbuf[7]);
		input_report_abs(data->input_dev, ABS_Z, rbuf[8]);
		input_report_abs(data->input_dev, ABS_WHEEL, rbuf[5]);
	}

	/* Report temperature information */
	if (atomic_read(&t_flag)) {
		input_report_abs(data->input_dev, ABS_THROTTLE, rbuf[3]);
	}

	if (atomic_read(&mv_flag)) {
//#if DEBUG
        printk("antispoon report_mv %d,%d,%d \n",rbuf[9], rbuf[10], rbuf[11]);
//#endif
		input_report_abs(data->input_dev, ABS_HAT0X, rbuf[9]);
		input_report_abs(data->input_dev, ABS_HAT0Y, rbuf[10]);
		input_report_abs(data->input_dev, ABS_BRAKE, rbuf[11]); //antispo
	}

	/* Proximity */
	if(atomic_read(&p_flag)){
		if(is_proxi_open()==1)
			input_report_abs(data->input_dev, ABS_DISTANCE, 1);/*rbuf[12]);*/
		if(is_proxi_open()==0)
			input_report_abs(data->input_dev, ABS_DISTANCE, 0);/*rbuf[12]);*/
	}
#endif 
	input_sync(data->input_dev);
}

static int AKECS_GetOpenStatus(void)
{
#if DEBUG
	printk(KERN_INFO "%s\n", __FUNCTION__);
    printk("antispoon open_flasg %d\n",atomic_read(&open_flag));
#endif
	wait_event_interruptible(open_wq, (atomic_read(&open_flag) != 0));
#if DEBUG
    printk("antispoon int_event wakeup\n");
#endif
	return atomic_read(&open_flag);
}

static int AKECS_GetCloseStatus(void)
{
#if DEBUG
	printk(KERN_INFO "%s\n", __FUNCTION__);
#endif
	wait_event_interruptible(open_wq, (atomic_read(&open_flag) <= 0));
	return atomic_read(&open_flag);
}

static void AKECS_CloseDone(void)
{
#if DEBUG
	printk(KERN_INFO "%s\n", __FUNCTION__);
#endif
	atomic_set(&m_flag, 1);
	atomic_set(&a_flag, 1);
	atomic_set(&t_flag, 1);
	atomic_set(&mv_flag, 1);
	atomic_set(&p_flag, 1);
}

static int akm_aot_open(struct inode *inode, struct file *file)
{
	int ret = -1;
#if DEBUG
	printk( "%s\n", __FUNCTION__);
#endif
	if (atomic_cmpxchg(&open_count, 0, 1) == 0) {
		if (atomic_cmpxchg(&open_flag, 0, 1) == 0) {
			atomic_set(&reserve_open_flag, 1);
			wake_up(&open_wq);
			ret = 0;
		}
	}
	return ret;
}

static int akm_aot_release(struct inode *inode, struct file *file)
{
#if DEBUG
	printk(KERN_INFO "%s\n", __FUNCTION__);
#endif
	atomic_set(&reserve_open_flag, 0);
	atomic_set(&open_flag, 0);
	atomic_set(&open_count, 0);
	wake_up(&open_wq);
	return 0;
}

//--[[ LGE_UBIQUIX_MODIFIED_START : shyun@ubiquix.com [2012.03.30] - Modify the func parameter
#if 0
static int
akm_aot_ioctl(struct inode *inode, struct file *file,
	      unsigned int cmd, unsigned long arg)
#else
static int akm_aot_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
#endif
//--]] LGE_UBIQUIX_MODIFIED_END : shyun@ubiquix.com [2012.03.30]- Modify the func parameter
{
	void __user *argp = (void __user *)arg;
	short flag;
#if DEBUG
	printk(KERN_INFO "diyu %s\n", __FUNCTION__);
#endif

	switch (cmd) {
	case ECS_IOCTL_APP_SET_MFLAG:
	case ECS_IOCTL_APP_SET_AFLAG:
	case ECS_IOCTL_APP_SET_TFLAG:
	case ECS_IOCTL_APP_SET_MVFLAG:
	case ECS_IOCTL_APP_SET_PFLAG:
		#if DEBUG
			printk(KERN_INFO "diyu %s-1\n", __FUNCTION__);
		#endif
		if (copy_from_user(&flag, argp, sizeof(flag)))
			return -EFAULT;
		if (flag < 0 || flag > 1)
			return -EINVAL;
		break;
	case ECS_IOCTL_APP_SET_DELAY:
		#if DEBUG
			printk(KERN_INFO "diyu %s-2\n", __FUNCTION__);
		#endif
		if (copy_from_user(&flag, argp, sizeof(flag)))
			return -EFAULT;
		break;
	default:
		#if DEBUG
			printk(KERN_INFO "diyu %s-3\n", __FUNCTION__);
		#endif
		break;
	}

	switch (cmd) {
	case ECS_IOCTL_APP_SET_MFLAG:
		#if DEBUG
			printk(KERN_INFO "diyu %s-4\n", __FUNCTION__);
	        printk ("antispoon mvflag %d\n",flag);
        #endif
		atomic_set(&m_flag, flag);
#if OPTIMUM_TEST
        if (flag == 1 ) KIONIX_ACCEL_enable_outputs();
        else {
            if (atomic_read(&a_flag) == 0) KIONIX_ACCEL_disable_outputs();
        }
#endif
		break;
	case ECS_IOCTL_APP_GET_MFLAG:
		#if DEBUG
			printk(KERN_INFO "diyu %s-5\n", __FUNCTION__);
		#endif
		flag = atomic_read(&m_flag);
		break;
	case ECS_IOCTL_APP_SET_AFLAG:
//		#if DEBUG
			printk(KERN_INFO "diyu %s-6 (flasg = %d)\n", __FUNCTION__, flag);
		    printk ("antispoon  ECS_IOCTL_APP_SET_AFLAG %d\n",flag);
//        #endif
		atomic_set(&a_flag, flag); //antispoon1101
#if OPTIMUM_TEST
        if (flag == 1 ) KIONIX_ACCEL_enable_outputs();
        else {
            if (atomic_read(&m_flag) == 0) KIONIX_ACCEL_disable_outputs();
        }
#endif
		break;
	case ECS_IOCTL_APP_GET_AFLAG:
		#if DEBUG
			printk(KERN_INFO "diyu %s-7\n", __FUNCTION__);
		#endif
		flag = atomic_read(&a_flag);
		break;
	case ECS_IOCTL_APP_SET_TFLAG:
		#if DEBUG
			printk(KERN_INFO "diyu %s-8\n", __FUNCTION__);
		#endif
		atomic_set(&t_flag, flag);
		break;
	case ECS_IOCTL_APP_GET_TFLAG:
		#if DEBUG
			printk(KERN_INFO "diyu %s-9\n", __FUNCTION__);
		#endif
		flag = atomic_read(&t_flag);
		break;\
	case ECS_IOCTL_APP_SET_PFLAG:
		#if DEBUG
			printk(KERN_INFO "diyu %s-8\n", __FUNCTION__);
		#endif
		atomic_set(&p_flag, flag);
		break;
	case ECS_IOCTL_APP_GET_PFLAG:
		#if DEBUG
			printk(KERN_INFO "diyu %s-9\n", __FUNCTION__);
		#endif
		flag = atomic_read(&p_flag);
		break;			
	case ECS_IOCTL_APP_SET_MVFLAG:
		#if DEBUG
			printk(KERN_INFO "diyu %s-10\n", __FUNCTION__);
	        printk ("antispoon mvflag %d\n",flag);
    	#endif
		atomic_set(&mv_flag, flag);
		break;
	case ECS_IOCTL_APP_GET_MVFLAG:
		#if DEBUG
			printk(KERN_INFO "diyu %s-11\n", __FUNCTION__);
		#endif
		flag = atomic_read(&mv_flag);
		break;
	case ECS_IOCTL_APP_SET_DELAY:
		#if DEBUG
			printk(KERN_INFO "diyu %s-12\n", __FUNCTION__);
		#endif
		akmd_delay = flag;
		break;
	case ECS_IOCTL_APP_GET_DELAY:
		#if DEBUG
			printk(KERN_INFO "diyu %s-13\n", __FUNCTION__);
		#endif
		flag = akmd_delay;
		break;
	default:
		#if DEBUG
			printk(KERN_INFO "diyu %s-14\n", __FUNCTION__);
		#endif
		return -ENOTTY;
	}

	switch (cmd) {
	case ECS_IOCTL_APP_GET_MFLAG:
	case ECS_IOCTL_APP_GET_AFLAG:
	case ECS_IOCTL_APP_GET_TFLAG:
	case ECS_IOCTL_APP_GET_PFLAG:
	case ECS_IOCTL_APP_GET_MVFLAG:
	case ECS_IOCTL_APP_GET_DELAY:
		#if DEBUG
			printk(KERN_INFO "diyu %s-15\n", __FUNCTION__);
		#endif
		if (copy_to_user(argp, &flag, sizeof(flag)))
			return -EFAULT;
		break;
	default:
		#if DEBUG
			printk(KERN_INFO "diyu %s-16\n", __FUNCTION__);
		#endif
		break;
	}
	#if DEBUG
		printk(KERN_INFO "diyu %s-17\n", __FUNCTION__);
	#endif

	return 0;
}

static int akmd_open(struct inode *inode, struct file *file)
{
	if(first_start == 0) {
		AKECS_Reset();
		first_start++;
	}

#if DEBUG
	printk(KERN_INFO "%s\n", __FUNCTION__);
#endif
	return nonseekable_open(inode, file);
}

static int akmd_release(struct inode *inode, struct file *file)
{
#if DEBUG
	printk(KERN_INFO "%s\n", __FUNCTION__);
#endif
	AKECS_CloseDone();
	return 0;
}

//--[[ LGE_UBIQUIX_MODIFIED_START : shyun@ubiquix.com [2012.03.30] - Modified function parameter 
#if 0
static int
akmd_ioctl(struct inode *inode, struct file *file, unsigned int cmd,
	   unsigned long arg)
#else
static int
akmd_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
#endif
//--]] LGE_UBIQUIX_MODIFIED_END : shyun@ubiquix.com [2012.03.30]- Modified function parameter 
{
#if DEBUG
	int i;
#endif
	void __user *argp = (void __user *)arg;

	char msg[RBUFF_SIZE + 1], rwbuf[16];//, numfrq[2];
	int ret = -1, status;
	short mode, value[13], delay; /* step_count,*/
//	char *pbuffer = 0;
#if DEBUG
	printk(KERN_INFO "%s", __FUNCTION__);
#endif

	switch (cmd) {
	case ECS_IOCTL_READ:
	case ECS_IOCTL_WRITE:
		#if DEBUG
				printk(KERN_INFO "diyu - - 1 \n", cmd);
		#endif

		if (copy_from_user(&rwbuf, argp, sizeof(rwbuf)))
			return -EFAULT;
		
		break;
	case ECS_IOCTL_SET_MODE:
		
		#if DEBUG
					printk(KERN_INFO "diyu - - 2 \n", cmd);
		#endif
		if (copy_from_user(&mode, argp, sizeof(mode)))
			return -EFAULT;
		break;
	case ECS_IOCTL_SET_YPR:
		
		#if DEBUG
						printk(KERN_INFO "diyu - - 3 \n", cmd);
		#endif
		if (copy_from_user(&value, argp, sizeof(value)))
			return -EFAULT;
		break;
	//case ECS_IOCTL_SET_STEP_CNT:
	//	if (copy_from_user(&step_count, argp, sizeof(step_count)))
	//		return -EFAULT;
	//	break;
	default:
	
		#if DEBUG
					printk(KERN_INFO "diyu - - 4 \n", cmd);
		#endif
		break;
	}

	switch (cmd) {
	case ECS_IOCTL_INIT:
#if DEBUG
		printk(KERN_INFO "diyu ECS_IOCTL_INIT %x\n", cmd);
#endif
		ret = AKECS_Init();
		if (ret < 0)
			return ret;
		break;
	case ECS_IOCTL_RESET:
#if DEBUG
		printk(KERN_INFO "diyu ECS_IOCTL_RESET %x\n", cmd);
#endif
		AKECS_Reset();
		break;
	case ECS_IOCTL_READ:
#if DEBUG
		printk(KERN_INFO "diyu ECS_IOCTL_READ %x\n", cmd);
		printk(" len %02x:", rwbuf[0]);
		printk(" addr %02x:", rwbuf[1]);
#endif
		if (rwbuf[0] < 1)
			return -EINVAL;
		ret = AKI2C_RxData(&rwbuf[1], rwbuf[0]);
#if DEBUG
		for(i=0; i<rwbuf[0]; i++){
			printk(KERN_INFO " %02x", rwbuf[i+1]);
		}
		printk("diyu ret = %d\n", ret);
#endif
		if (ret < 0)
			return ret;
		break;
	case ECS_IOCTL_WRITE:
#if DEBUG
		//printk(KERN_INFO "diyu ECS_IOCTL_WRITE %x\n", cmd);
		//printk(" len %02x:", rwbuf[0]);
		for(i=0; i<rwbuf[0]; i++){
			printk(" %02x", rwbuf[i+1]);
		}
#endif
		if (rwbuf[0] < 2)
			return -EINVAL;
		ret = AKI2C_TxData(&rwbuf[1], rwbuf[0]);
#if DEBUG
		//printk(KERN_INFO " ret = %d\n", ret);
#endif
		if (ret < 0)
			return ret;
		break;
	case ECS_IOCTL_SET_MODE:
#if DEBUG
		printk(KERN_INFO "diyu ECS_IOCTL_SET_MODE %x mode=%x\n", cmd, mode);
#endif
		ret = AKECS_SetMode((char)mode);
#if DEBUG
		printk(" ret = %d\n", ret);
#endif
		if (ret < 0)
			return ret;
		break;
	case ECS_IOCTL_GETDATA:
#if DEBUG
		printk(KERN_INFO "diyu ECS_IOCTL_GETDATA %x\n", cmd);
#endif
		ret = AKECS_TransRBuff(msg, RBUFF_SIZE+1);
#if DEBUG
		printk(KERN_INFO " ret = %d\n", ret);
#endif
		if (ret < 0)
			return ret;
#if DEBUG
		for(i=0; i<ret; i++){
			printk(" %02x", msg[i]);
		}
		printk("\n");
#endif
		break;
	case ECS_IOCTL_SET_YPR:
//#if DEBUG
		printk(KERN_INFO "diyu ECS_IOCTL_SET_YPR %x ypr=%x\n", cmd, value);
//#endif
		AKECS_Report_Value(value);
		break;
	case ECS_IOCTL_GET_OPEN_STATUS:
#if DEBUG
		printk(KERN_INFO "diyu ECS_IOCTL_GET_OPEN_STATUS %x start\n", cmd);
#endif
		status = AKECS_GetOpenStatus();
#if DEBUG
		printk("diyu ECS_IOCTL_GET_OPEN_STATUS %x end status=%x\n", cmd, status);
#endif
		break;
	case ECS_IOCTL_GET_CLOSE_STATUS:
#if DEBUG
		printk("diyu ECS_IOCTL_GET_CLOSE_STATUS %x start\n", cmd);
#endif
		status = AKECS_GetCloseStatus();
#if DEBUG
		printk("diyu ECS_IOCTL_GET_CLOSE_STATUS %x end status=%x\n", cmd, status);
#endif
		break;
//	case ECS_IOCTL_GET_CALI_DATA:
//		pbuffer = get_akm_cal_ram();
//#if DEBUG
//		printk("ECS_IOCTL_GET_CALI_DATA %x pbuffer=%x\n", cmd, pbuffer);
//#endif
//		break;
	case ECS_IOCTL_GET_DELAY:
		delay = akmd_delay;
#if DEBUG
		printk(KERN_INFO "diyu ECS_IOCTL_GET_DELAY %x delay=%x\n", cmd, delay);
#endif
		break;
	default:
#if DEBUG
		printk(KERN_INFO "diyu Unknown cmd %x\n", cmd);
#endif
		return -ENOTTY;
	}

	switch (cmd) {
	case ECS_IOCTL_READ:
		#if DEBUG
					printk(KERN_INFO "diyu - - 21 \n", cmd);
		#endif
		if (copy_to_user(argp, &rwbuf, sizeof(rwbuf)))
			return -EFAULT;
		break;
	case ECS_IOCTL_GETDATA:
		#if DEBUG
					printk(KERN_INFO "diyu - - 22 \n", cmd);
		#endif
		if (copy_to_user(argp, &msg, sizeof(msg)))
			return -EFAULT;
		break;
//	case ECS_IOCTL_GET_NUMFRQ:
//		if (copy_to_user(argp, &numfrq, sizeof(numfrq)))
//			return -EFAULT;
//		break;
	case ECS_IOCTL_GET_OPEN_STATUS:
	case ECS_IOCTL_GET_CLOSE_STATUS:
		#if DEBUG
					printk(KERN_INFO "diyu - - 23 \n", cmd);
		#endif
		if (copy_to_user(argp, &status, sizeof(status)))
			return -EFAULT;
		break;
//	case ECS_IOCTL_GET_CALI_DATA:
//		if (copy_to_user(argp, pbuffer, MAX_CALI_SIZE))
//			return -EFAULT;
//		break;
	case ECS_IOCTL_GET_DELAY:
		#if DEBUG
					printk(KERN_INFO "diyu - - 24 \n", cmd);
		#endif
		if (copy_to_user(argp, &delay, sizeof(delay)))
			return -EFAULT;
		break;
	default:
		#if DEBUG
					printk(KERN_INFO "diyu - - 25 ", cmd);
		#endif
		break;
	}
		#if DEBUG
						printk(KERN_INFO "diyu - - end ", cmd);
		#endif

	return 0;
}

#if 0
static void akm_work_func(struct work_struct *work)
{
#if DEBUG
	printk(KERN_INFO "%s\n", __FUNCTION__);
#endif
	if (AKECS_GetData() < 0)
		printk(KERN_ERR "akm_work_func: Get data failed\n");
}
#endif


#if 0
static int akm8976_suspend(struct i2c_client *client, pm_message_t mesg)
{
	atomic_set(&suspend_flag, 1);
	if (atomic_read(&open_flag) == 2)
		AKECS_SetMode(AKECS_MODE_POWERDOWN);

	atomic_set(&reserve_open_flag, atomic_read(&open_flag));
	atomic_set(&open_flag, 0);
	wake_up(&open_wq);
	disable_irq(this_client->irq);
	
return 0;
}
#endif 

#ifdef CONFIG_HAS_EARLYSUSPEND

#ifdef CONFIG_HAS_EARLYSUSPEND /* 20110304 seven.kim@lge.com late_resume_lcd [START] */
static int hub_akm8973_resume(struct i2c_client *client);
static int hub_akm8973_suspend(struct i2c_client *client, pm_message_t mesg);

static void akm8973_early_suspend(struct early_suspend *handler)
{
	struct akm8973_data *akm;
	
	akm = container_of(handler, struct akm8973_data, early_suspend);
	
	hub_akm8973_suspend(akm->client, PMSG_SUSPEND);
	
//	return 0;
}

static void akm8973_late_resume(struct early_suspend *handler)
{
	struct akm8973_data *akm;
	akm = container_of(handler, struct akm8973_data, early_suspend);

	hub_akm8973_resume(akm->client);
	
//	return 0;
}
#endif /* 20110304 seven.kim@lge.com late_resume_lcd [END] */

//static void akm8973_early_suspend(struct early_suspend *handler)
//static void akm8973_suspend(struct early_suspend *handler)

static int hub_akm8973_suspend(struct i2c_client *client, pm_message_t mesg)
{
#if 0
#if DEBUG
	printk(KERN_INFO "%s\n", __FUNCTION__);
#endif
	atomic_set(&suspend_flag, 1);
//	if (atomic_read(&open_flag) == 2) //antispoon0125
		AKECS_SetMode(AKECS_MODE_POWERDOWN);
	atomic_set(&reserve_open_flag, atomic_read(&open_flag));
	atomic_set(&open_flag, 0);
	wake_up(&open_wq);
	//disable_irq(this_client->irq); //antispoon0225

/* LGE_CHANGE_S [jh.koo@lge.com] 2009-07-10 */
	
	akm8973_set_vreg(0);
	akm8973_config_gpio(0);

/* LGE_CHANGE_E [jh.koo@lge.com] 2009-07-10 */
#else
#if 1//DEBUG
//For_Resume_Speed	printk(KERN_INFO "%s\n", __FUNCTION__);
#endif
	AKECS_PowerDown();
#endif

	return 0;

}
#endif
#if 0
static int akm8976_resume(struct i2c_client *client)
{
	enable_irq(this_client->irq);
	if (atomic_read(&open_flag) == 2)
		AKECS_SetMode(AKECS_MODE_PFFD);
	atomic_set(&suspend_flag, 0);
	atomic_set(&open_flag, atomic_read(&reserve_open_flag));
	wake_up(&open_wq);
	return 0;
}
#endif

#ifdef CONFIG_HAS_EARLYSUSPEND
//static void akm8973_early_resume(struct early_suspend *handler)
//static void akm8973_resume(struct early_suspend *handler)
static int hub_akm8973_resume(struct i2c_client *client)
{
#if 0
#if DEBUG
	printk(KERN_INFO "%s\n", __FUNCTION__);
#endif

/* LGE_CHANGE_S [jh.koo@lge.com] 2009-07-10 */
	akm8973_config_gpio(1);
	akm8973_set_vreg(1);

	AKECS_Reset();
/* LGE_CHANGE_E [jh.koo@lge.com] 2009-07-10 */

	atomic_set(&suspend_flag, 0);
	atomic_set(&open_flag, atomic_read(&reserve_open_flag));
	wake_up(&open_wq);
#else
	char buffer[2];

#if 1//DEBUG
//For_Resume_Speed	printk(KERN_INFO "%s\n", __FUNCTION__);
#endif

	/* Set measure mode */
	buffer[0] = 0x0a;
//	buffer[1] = 0x01;
	buffer[1] = 0x02;

	/* Set data */
	return AKI2C_TxData(buffer, 2);

#endif
	return 0;
}
#endif // #ifdef CONFIG_HAS_EARLYSUSPEND

static int akm8973_init_client(struct i2c_client *client)
{
//	struct akm8973_data *data;
//	int ret;
#if DEBUG
	printk(KERN_INFO "%s\n", __FUNCTION__);
#endif

//	data = i2c_get_clientdata(client);
//
//	mutex_init(&sense_data_mutex);
//
//	pdata = client->dev.platform_data;
//	if (pdata == NULL) {
//		pdata = kzalloc(sizeof(*pdata), GFP_KERNEL);
//		if (pdata == NULL) {
//			ret = -ENOMEM;
//			goto err_alloc_data_failed;
//		} else {
//			pdata->reset = ECS_RST;
//			pdata->clk_on = ECS_CLK_ON;
//			pdata->intr = ECS_INTR;
//		}
//	}
//
//	init_waitqueue_head(&data_ready_wq);
	init_waitqueue_head(&open_wq);

	/* As default, report all information */
	atomic_set(&m_flag, 1);
	atomic_set(&a_flag, 1);
	atomic_set(&t_flag, 1);
	atomic_set(&mv_flag, 1);
	atomic_set(&p_flag, 1);

	return 0;

//err_alloc_data_failed:
//	return ret;
}

static struct file_operations akmd_fops = {
	.owner = THIS_MODULE,
	.open = akmd_open,
	.release = akmd_release,
//--[[ LGE_UBIQUIX_MODIFIED_START : shyun@ubiquix.com [2012.03.30] - .ioctl replace to .unlocked_ioctl
//	.ioctl = akmd_ioctl,
	.unlocked_ioctl = akmd_ioctl,
//--]] LGE_UBIQUIX_MODIFIED_END : shyun@ubiquix.com [2012.03.30]- .ioctl replace to .unlocked_ioctl
};

static struct file_operations akm_aot_fops = {
	.owner = THIS_MODULE,
	.open = akm_aot_open,
	.release = akm_aot_release,
//--[[ LGE_UBIQUIX_MODIFIED_START : shyun@ubiquix.com [2012.03.30] - .ioctl replace to .unlocked_ioctl
//	.ioctl = akm_aot_ioctl,
	.unlocked_ioctl = akm_aot_ioctl,
//--]] LGE_UBIQUIX_MODIFIED_END : shyun@ubiquix.com [2012.03.30]- .ioctl replace to .unlocked_ioctl
};

static struct miscdevice akm_aot_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "akm8973_aot",
	.fops = &akm_aot_fops,
};

static struct miscdevice akmd_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "akm8973_daemon",
	.fops = &akmd_fops,
};

static int check_result_value = -1; /*0: Fail,  1: Pass,  -1 : No result */
static ssize_t akm_checkresult_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	//printk("%s\n", __FUNCTION__);
	return sprintf(buf, "%d\n", check_result_value);
}

static ssize_t akm_checkresult_store(
		struct device *dev, struct device_attribute *attr, 
		const char *buf, size_t size)
{
	int value;
	sscanf(buf, "%d", &value);
	//printk("%s\n", __FUNCTION__);

	check_result_value = value; 
	return size;
}

static DEVICE_ATTR(checkresult, S_IRUGO | S_IWUSR, akm_checkresult_show, akm_checkresult_store);

static int check_opmode_value = 0; /*0: Normal, 1: Factory Mode*/
static ssize_t akm_checkopmode_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	//printk("%s\n", __FUNCTION__);
	//check_opmode_value =1;
	return sprintf(buf, "%d\n", check_opmode_value);
}

static ssize_t akm_checkopmode_store(
		struct device *dev, struct device_attribute *attr, 
		const char *buf, size_t size)
{
	int value;
	sscanf(buf, "%d", &value);
	//printk("%s\n", __FUNCTION__);

	check_opmode_value = value; 
	if(check_opmode_value ==1)
		powerDownOrOff = 1;
	else if(check_opmode_value ==0)
		powerDownOrOff = 0;
		
	return size;
}

static DEVICE_ATTR(checkopmode, S_IRUGO | S_IWUSR, akm_checkopmode_show, akm_checkopmode_store);

int hub_akm8973_probe(struct i2c_client *client, const struct i2c_device_id * devid)
{
	struct akm8973_data *akm;
	int err;
#ifdef ENABLE_REGULATOR
    struct device *akm_dev = &client->dev;
#endif

	printk("hub_akm8973_probe [IN]\n");


	printk(KERN_INFO "%s\n", __FUNCTION__);
/* LGE_CHANGE_S [antispoon@lge.com] 2009-11-20 */
#ifdef ENABLE_REGULATOR
    compass_reg= regulator_get(akm_dev,"vaux2"); 
    if (compass_reg == NULL) {
        printk("akm8973 antispoon : Failed to get motion power regulator !! \n");
        return -ENODEV;
    }
    compass_io_reg = regulator_get(akm_dev,"vdac");
    if (compass_io_reg == NULL) {
        printk("akm8973 antispoon : Failed to get motion io power regulator !! \n");
        return -ENODEV;
    }
#endif
#if 0
    if (gpio_request(GPIO_COMPASS_RESET,"hub_akm8973_reset") < 0) {
        printk("[ERROR!!] GPIO_COMPASS_RESET request\n");
        goto exit_check_functionality_failed; 
    }
#endif
    akm8973_set_vreg(1); //antispoon0101
/* LGE_CHANGE_E [jh.koo@lge.com] 2009-11-20 */
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		err = -ENODEV;
		goto exit_check_functionality_failed;
	}

	akm = kzalloc(sizeof(struct akm8973_data), GFP_KERNEL);
	if (!akm) {
		err = -ENOMEM;
		goto exit_alloc_data_failed;
	}

//	INIT_WORK(&akm->work, akm_work_func);
	i2c_set_clientdata(client, akm);
	akm8973_init_client(client);
	this_client = client;

	akm->input_dev = input_allocate_device();

	if (!akm->input_dev) {
		err = -ENOMEM;
		printk(KERN_ERR
		       "hub_akm8973_probe: Failed to allocate input device\n");
		goto exit_input_dev_alloc_failed;
	}

	set_bit(EV_ABS, akm->input_dev->evbit);

	/* yaw */
	input_set_abs_params(akm->input_dev, ABS_RX, 0, 23040, 0, 0);
	/* pitch */
	input_set_abs_params(akm->input_dev, ABS_RY, -11520, 11520, 0, 0);
	/* roll */
	input_set_abs_params(akm->input_dev, ABS_RZ, -5760, 5760, 0, 0);
	/* x-axis acceleration */
	input_set_abs_params(akm->input_dev, ABS_X, -5760, 5760, 0, 0);
	/* y-axis acceleration */
	input_set_abs_params(akm->input_dev, ABS_Y, -5760, 5760, 0, 0);
	/* z-axis acceleration */
	input_set_abs_params(akm->input_dev, ABS_Z, -5760, 5760, 0, 0);
	/* temparature */
	input_set_abs_params(akm->input_dev, ABS_THROTTLE, -30, 85, 0, 0);
	/* status of magnetic sensor */
	input_set_abs_params(akm->input_dev, ABS_RUDDER, 0, 3, 0, 0);
	/* status of acceleration sensor */
	input_set_abs_params(akm->input_dev, ABS_WHEEL, 0, 3, 0, 0);
	#if 0
	/* x-axis of raw magnetic vector */
	input_set_abs_params(akm->input_dev, ABS_HAT0X, -2048, 2032, 0, 0);
	/* y-axis of raw magnetic vector */
	input_set_abs_params(akm->input_dev, ABS_HAT0Y, -2048, 2032, 0, 0);
	/* z-axis of raw magnetic vector */
	input_set_abs_params(akm->input_dev, ABS_BRAKE, -2048, 2032, 0, 0);
	#endif
	/* proximity */
	input_set_abs_params(akm->input_dev, ABS_DISTANCE, 0, 1, 0, 0);

	akm->input_dev->name = "compass";

	err = input_register_device(akm->input_dev);

	if (err) {
		printk(KERN_ERR
		       "hub_akm8973_probe: Unable to register input device: %s\n",
		       akm->input_dev->name);
		goto exit_input_register_device_failed;
	}

	err = misc_register(&akmd_device);
	if (err) {
		printk(KERN_ERR "hub_akm8973_probe: akmd_device register failed\n");
		goto exit_misc_device_register_failed;
	}

	err = misc_register(&akm_aot_device);
	if (err) {
		printk(KERN_ERR
		       "hub_akm8973_probe: akm_aot_device register failed\n");
		goto exit_misc_device_register_failed;
	}

	err = device_create_file(&client->dev, &dev_attr_checkopmode);
	if (err) {
		printk( "check opmode: Fail\n");
		return err;
	}
	err = device_create_file(&client->dev, &dev_attr_checkresult);
		if (err) {
			printk( "check opmode: Fail\n");
			return err;
		}

	err = device_create_file(&client->dev, &dev_attr_ms1);

	 
//	if(system_rev == LGE_PCB_VER_D) //antispoon1026
//		AKECS_WriteEEPROM();
	
#ifdef CONFIG_HAS_EARLYSUSPEND
	#if 1 /* 20110304 seven.kim@lge.com late_resume_lcd [START] */
//#ifdef CONFIG_MACH_LGE_JUSTIN//justin LCD resume speed up
#ifdef CONFIG_PRODUCT_LGE_LU6800//justin LCD resume speed up	
  akm->early_suspend.level = 45;//NATTING_TEST
#else
	akm->early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN -1; /* 20110304 seven.kim@lge.com late_resume_lcd */
#endif
	akm->early_suspend.suspend = akm8973_early_suspend;
	akm->early_suspend.resume = akm8973_late_resume;
	register_early_suspend(&akm->early_suspend);
	#else //origianl
//	akm->early_suspend.suspend = akm8973_early_suspend;
//	akm->early_suspend.resume = akm8973_early_resume;
//	register_early_suspend(&akm->early_suspend);
	#endif /* 20110304 seven.kim@lge.com late_resume_lcd [END] */
#endif

	printk("hub_akm8973_probe [NORMAL OUT]\n");

	return 0;

exit_misc_device_register_failed:
exit_input_register_device_failed:
	input_free_device(akm->input_dev);
exit_input_dev_alloc_failed:
	kfree(akm);
exit_alloc_data_failed:
exit_check_functionality_failed:
	return err;
}

static int akm8973_detect(struct i2c_client *client, int kind,
			  struct i2c_board_info *info)
{
#if DEBUG
	printk(KERN_INFO "%s\n", __FUNCTION__);
#endif
	strlcpy(info->type, "akm8973", I2C_NAME_SIZE);
	return 0;
}

static int hub_akm8973_remove(struct i2c_client *client)
{
	struct akm8973_data *akm = i2c_get_clientdata(client);
#if DEBUG
	printk(KERN_INFO "AKM8973A compass driver: init\n");
#endif
	input_unregister_device(akm->input_dev);
	kfree(akm);
	return 0;
}

static const struct i2c_device_id hub_akm8973_id[] = {
//--[[ LGE_UBIQUIX_MODIFIED_START : shyun@ubiquix.com [2011.07.28] - Featuring is added.
#if defined(CONFIG_PRODUCT_LGE_LU6800)
	{ "justin_akm8973", 0 },
#elif defined(CONFIG_PRODUCT_LGE_KU5900) || defined(CONFIG_PRODUCT_LGE_P970)
	{ "black_akm8973", 0 },
#else
#error "Please check the product definition(ECompass)"
#endif
//--]] LGE_UBIQUIX_MODIFIED_END : shyun@ubiquix.com [2011.07.28]- Featuring is added.
	{ }
};

static struct i2c_driver akm8973_driver = {
	.class = I2C_CLASS_HWMON,
	.probe = hub_akm8973_probe,
	.remove = hub_akm8973_remove,
#ifndef CONFIG_HAS_EARLYSUSPEND /* 20110304 seven.kim@lge.com late_resume_lcd [START] */	
	.suspend	= hub_akm8973_suspend,
	.resume		= hub_akm8973_resume,
#endif /* 20110304 seven.kim@lge.com late_resume_lcd [END] */
	.id_table = hub_akm8973_id,
	.driver = {
		   .owner = THIS_MODULE,
//--[[ LGE_UBIQUIX_MODIFIED_START : shyun@ubiquix.com [2011.07.28] - Featuring is added.
#if defined(CONFIG_PRODUCT_LGE_LU6800)
		   .name = "justin_akm8973",
#elif defined(CONFIG_PRODUCT_LGE_KU5900) || defined(CONFIG_PRODUCT_LGE_P970)
		   .name = "black_akm8973",
#else
#error "Please check the product definition(ECompass)"
#endif
//--]] LGE_UBIQUIX_MODIFIED_END : shyun@ubiquix.com [2011.07.28]- Featuring is added.
		   },
	.detect = akm8973_detect,
	//.address_data = &addr_data,
};

static int __init akm8973_init(void)
{
#if DEBUG
	printk(KERN_INFO "HUB_AKM8973A compass driver: init\n");
#endif
	return i2c_add_driver(&akm8973_driver);
}

static void __exit akm8973_exit(void)
{
#if DEBUG
	printk(KERN_INFO "HUB_AKM8973A compass driver: exit\n");
#endif
	i2c_del_driver(&akm8973_driver);
}

module_init(akm8973_init);
module_exit(akm8973_exit);

MODULE_AUTHOR("Hou-Kun Chen <hk_chen@htc.com>");
MODULE_DESCRIPTION("HUB_AKM8973 compass driver");
MODULE_LICENSE("GPL");
