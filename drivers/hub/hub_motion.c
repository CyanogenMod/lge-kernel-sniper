/*
 *  heaven_motion.c
 *
 *  heaven motion sensor driver  (Accelerometer, Gyroscope Sensor)  
 * 
 * Copyright (C) 2009 LGE Inc.
 *
 *   chpark96@lge.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
  
#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <asm/gpio.h>
#include <asm/system.h>
#include <linux/workqueue.h>
#include <linux/device.h>
#include <linux/string.h>
//#include <mach/resource.h>
#include <mach/hardware.h>
#include <asm/uaccess.h>
#include <linux/ioctl.h>
#include <linux/miscdevice.h>
#include <linux/regulator/consumer.h>
#ifdef CONFIG_ARCH_OMAP
#include <mach/gpio.h>
#endif
#include "hub_motion.h"
#include "mpu3050.h"
#include "akm8973.h"
#include "kionix_accel_api.h"
#include "kxtf9.h"
#include "kxtf9_if.h"

//#include <linux/i2c/twl4030.h>

 #ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif

#define  MISC_DYNAMIC_MINOR		 255
#define  MAX_MOTION_DATA_LENGTH	 10
#define  MIN_MOTION_POLLING_TIME    10
//#define  HEAVEN_GYRO_INT_GPIO         41
//#define MAX_LGE_ACCEL_SAMPLERATE    7  /* for JSR256 API */


struct heaven_motion_device {
	struct input_dev            *input_dev;         
	struct input_dev            *input_dev1;	 /* motion daemon process */	       
	struct hrtimer              timer[5];        /* [0] acceleroemter raw data, [1] tilt , [2] Gyro, [3] Compass, [4] composite */

	struct work_struct     		accel_work;
	struct workqueue_struct	 	*accel_wq;
	
	struct work_struct     		tilt_work;
	struct workqueue_struct	 	*tilt_wq;
	
	struct work_struct     		gyro_work;
	struct workqueue_struct	 	*gyro_wq;
	
	struct work_struct     		compass_work;
	struct workqueue_struct	 	*compass_wq;

	struct work_struct     		composite_work;
	struct workqueue_struct	 	*composite_wq;
	
	int  irq;
	int  use_irq;

#ifdef CONFIG_HAS_EARLYSUSPEND
	struct early_suspend early_suspend;
#endif
};

struct i2c_client    *kxtf9_i2c_client;

static struct heaven_motion_device   heaven_motion_dev;
static struct regulator 	 		  *heaven_motion_reg;
static struct regulator				  *heaven_gyro_vio_reg;

static atomic_t compass_flag;
static atomic_t	   accel_flag;
static atomic_t	   tilt_flag;    
static atomic_t	   tap_flag;
static atomic_t	   shake_flag;
static atomic_t	   snap_flag;
static atomic_t	   flip_flag;
static atomic_t   gyro_flag;
static atomic_t composite_flag;
static atomic_t 	   yawimage_flag;    
static atomic_t	   accel_delay;
static atomic_t   compass_delay;
static atomic_t	   tilt_delay;
static atomic_t   gyro_delay;
static atomic_t composite_delay;
static atomic_t cal_flag;
static atomic_t cal_result;

static atomic_t   suspend_flag;
static atomic_t   bypass_flag;
static atomic_t	   jsr256_flag;
static atomic_t   poweronoff_flag;

static atomic_t	   tilt_roll,tilt_pitch,tilt_yaw;
static atomic_t   accel_x,accel_y,accel_z;
static atomic_t   gyro_x, gyro_y, gyro_z;
static atomic_t	   mag_x, mag_y, mag_z;
static atomic_t composite[3];

#ifdef CONFIG_HAS_EARLYSUSPEND
static void mpu3050_early_suspend(struct early_suspend *h);
static void mpu3050_late_resume(struct early_suspend *h);
#endif

//Compass API
//void magnetic_input_report(int *);

//Accelerometer API
/*---------------------------------------------------------------------------
	kxtf9_reg_i2c_client
   ---------------------------------------------------------------------------*/		
void  kxtf9_reg_i2c_client(struct i2c_client *client)
{
	printk("kxtf9_reg_i2c_client..........................\n");
	kxtf9_i2c_client =  client;
}

/*---------------------------------------------------------------------------
  motion_sensor_power_on/off
  ---------------------------------------------------------------------------*/
#define ACCEL_CTRL_REG3		0x1D
#define ACCEL_CTRL_REG1		0x1B
#define ACCEL_PC1_ON		0x80
#define ACCEL_PC1_OFF			0x00  //stand-by-mode
#define AMI304_REG_CTRL1 0x1B
#define AMI304_REG_CTRL2 0x1C
#define AMI304_REG_CTRL3 0x1D

void motion_sensor_power_on(void)
{
	static unsigned char   	tempbuf[4]={0,};
	char buffer[2];

	/* Accelerometer power on */
#if DEBUG
	printk("[%s:%d] Accelerometer and Compass Sensor \n", __FUNCTION__, __LINE__);
#endif
/*
	tempbuf[3]=0;
	tempbuf[1] = 1;

	kxtf9_i2c_read(ACCEL_CTRL_REG3,&tempbuf[3],tempbuf[1]);

	tempbuf[3] |= 1<<7;
	tempbuf[1] = 2;
	
	kxtf9_i2c_write(ACCEL_CTRL_REG3,&tempbuf[3]);   // sets SRST bit to reboot

	msleep(1);
	*/
	
	tempbuf[3]=0;
	tempbuf[1] = 1;
	
	kxtf9_i2c_read(ACCEL_CTRL_REG1,&tempbuf[3],tempbuf[1]);

	tempbuf[3] |= ACCEL_PC1_ON;

	kxtf9_i2c_write(ACCEL_CTRL_REG1 ,&tempbuf[3]);

	msleep(1);

       /* Compass power on */
	AKECS_SetMode(AKECS_MODE_MEASURE);

}

void motion_sensor_power_off(void)
{
	static unsigned char   	tempbuf[4]={0,};

#if DEBUG
	printk("[%s:%d] Accelerometer and Compass Sensor \n", __FUNCTION__, __LINE__);
#endif

	tempbuf[3]=0;
	tempbuf[1] = 1;
	
	kxtf9_i2c_read(ACCEL_CTRL_REG1,&tempbuf[3],tempbuf[1]);
	
	/* Accelerometer power off */
	tempbuf[3] |= ACCEL_PC1_OFF;
	tempbuf[1] = 2;
	kxtf9_i2c_write(ACCEL_CTRL_REG1 ,&tempbuf[3] );

       /* Compass power off */
	AKECS_SetMode(AKECS_MODE_POWERDOWN);

}


/*---------------------------------------------------------------------------
	 motion_send_event function
   ---------------------------------------------------------------------------*/	
void motion_send_accel_detection(int accelx,int accely,int accelz)
{
	if (atomic_read(&accel_flag)) {
		input_report_abs(heaven_motion_dev.input_dev,ABS_X, accelx);
		input_report_abs(heaven_motion_dev.input_dev,ABS_Y, accely);
		input_report_abs(heaven_motion_dev.input_dev,ABS_Z, accelz);
		input_sync(heaven_motion_dev.input_dev);
	}
}
void motion_send_tilt_detection(int yaw,int pitch,int roll)
{
	if (atomic_read(&tilt_flag))
	{
		input_report_rel(heaven_motion_dev.input_dev,REL_RX,yaw);
		input_report_rel(heaven_motion_dev.input_dev,REL_RY,pitch);
		input_report_rel(heaven_motion_dev.input_dev,REL_RZ,roll);
				
		input_sync(heaven_motion_dev.input_dev);
	}
}
void motion_send_composite_detection(int *data)
{
	int buf[3] = {0,};
	memcpy(buf, data, sizeof(int)*3);

	//printk("composite %d %d %d ... %d %d %d\n", buf[0],buf[1],buf[2],buf[9],buf[10],buf[11]);
	if (atomic_read(&composite_flag))
	{
		input_report_abs(heaven_motion_dev.input_dev,ABS_GAS,buf[0]);
		input_report_abs(heaven_motion_dev.input_dev,ABS_HAT1X, buf[1]);
		input_report_abs(heaven_motion_dev.input_dev,ABS_HAT1Y, buf[2]);
		//input_report_abs(heaven_motion_dev.input_dev,ABS_HAT2X, buf[3]);
		//input_report_abs(heaven_motion_dev.input_dev,ABS_HAT2Y, buf[4]);
		//input_report_abs(heaven_motion_dev.input_dev,ABS_HAT3X, buf[5]);
		//input_report_abs(heaven_motion_dev.input_dev,ABS_HAT3Y, buf[6]);
		//input_report_abs(heaven_motion_dev.input_dev,ABS_TILT_X, buf[7]);
		//input_report_abs(heaven_motion_dev.input_dev,ABS_TILT_Y, buf[8]);
		//input_report_abs(heaven_motion_dev.input_dev,ABS_TOOL_WIDTH, buf[9]);
		//input_report_abs(heaven_motion_dev.input_dev,ABS_VOLUME, buf[10]);
		//input_report_abs(heaven_motion_dev.input_dev,ABS_MISC, buf[11]);
		//input_report_rel(heaven_motion_dev.input_dev,REL_MISC, buf[12]); 
				
		input_sync(heaven_motion_dev.input_dev);
	}
}
void motion_send_gyro_detection(int gyro_x,int gyro_y,int gyro_z)
{
	/*DY2*///lprintk(D_SENSOR,"[Gyro_accel][%s:%d]\n",__FUNCTION__, __LINE__);

	if (atomic_read(&gyro_flag)) {
		/*DY*///lprintk("[Gyro_accel][%s:%d]\n",__FUNCTION__, __LINE__);
		input_report_rel(heaven_motion_dev.input_dev,REL_Z, gyro_x);
		input_report_rel(heaven_motion_dev.input_dev,REL_MISC, gyro_y);
		input_report_rel(heaven_motion_dev.input_dev,REL_MAX, gyro_z);

		input_sync(heaven_motion_dev.input_dev);
	}
}

void motion_send_compass_detection(int compass_x,int compass_y,int compass_z)
{
	/*DY2*///lprintk(D_SENSOR,"[Gyro_accel][%s:%d]\n",__FUNCTION__, __LINE__);

	if (atomic_read(&compass_flag)) {
		//printk("[motion_send_compass_detection] %d %d %d \n", compass_x, compass_y, compass_z);
		input_report_abs(heaven_motion_dev.input_dev, ABS_HAT0X, compass_x);
		input_report_abs(heaven_motion_dev.input_dev, ABS_HAT0Y, compass_y);
		input_report_abs(heaven_motion_dev.input_dev, ABS_BRAKE, compass_z);

		input_sync(heaven_motion_dev.input_dev);
	}
}

void motion_send_tap_detection(int type,int direction)
{
	if(atomic_read(&tap_flag))
	{
		input_report_rel(heaven_motion_dev.input_dev,REL_X,type); 
		input_report_rel(heaven_motion_dev.input_dev,REL_Y,direction); 
		
		input_sync(heaven_motion_dev.input_dev);
	}
}
void motion_send_flip_detection(int value)
{
	if(atomic_read(&flip_flag))
	{
		input_report_rel(heaven_motion_dev.input_dev,REL_WHEEL,value); 
		input_sync(heaven_motion_dev.input_dev);
	}
}
void motion_send_shake_detection(int value)
{
	if(atomic_read(&shake_flag))
	{
		input_report_rel(heaven_motion_dev.input_dev,REL_HWHEEL,value); 
		input_sync(heaven_motion_dev.input_dev);
	}
}
void motion_send_snap_detection(int direction)
{
	if(atomic_read(&snap_flag))
	{
		input_report_rel(heaven_motion_dev.input_dev,REL_DIAL,direction); 
		
		input_sync(heaven_motion_dev.input_dev);
	}
}
void motion_send_yawimage_detection(int direction)
{
	if (atomic_read(&yawimage_flag))
	{
		input_report_abs(heaven_motion_dev.input_dev, ABS_THROTTLE, direction);
		input_sync(heaven_motion_dev.input_dev);
	}
}


/*---------------------------------------------------------------------------
	 work function
   ---------------------------------------------------------------------------*/		  
static void motion_accel_work_func(struct work_struct *work)                         
{
	int current_x = 0, current_y = 0, current_z = 0;
	u8 raw_data[6]; // xyz data bytes from hardware
	int accel_data[3];
	int onoff_bypass;

	onoff_bypass = atomic_read(&bypass_flag);
	
	if(onoff_bypass){
		//get_accel_raw_data
		//NvAccelerometerI2CGetRegsPassThrough(0x06, &raw_data[0], 6);	
		//printk ("[motion_accel_work_func] data0=[%d], data1=[%d], data2=[%d], data3=[%d], data4=[%d], data5=[%d] \n",raw_data[0],raw_data[1],raw_data[2],raw_data[3],raw_data[4],raw_data[5]);
		kxtf9_accel_api_read_data(&accel_data[0]);
		current_x = accel_data[0]*10;
		current_y = accel_data[1]*10;
		current_z = accel_data[2]*10;
		//printk ("[motion_accel_work_func:1] gx=[%d], gy=[%d], gz=[%d] \n",current_x,current_y,current_z);	
	}else{
		current_x = atomic_read(&accel_x);
		current_y = atomic_read(&accel_y);
		current_z = atomic_read(&accel_z);		

		//printk ("[motion_accel_work_func:2] gx=[%d], gy=[%d], gz=[%d] \n",current_x,current_y,current_z);	
	}

	//printk ("[motion_accel_work_func: %d] gx=[%d], gy=[%d], gz=[%d] \n", onoff_bypass, current_x, current_y, current_z);	

	motion_send_accel_detection(current_x,current_y,current_z);
	
}
static void motion_tilt_work_func(struct work_struct *work)                         
{
	int current_yaw = 0, current_pitch = 0, current_roll = 0;
	int data[12] = {0,};
	int i=0;

	current_yaw   = atomic_read(&tilt_yaw);
	current_pitch = atomic_read(&tilt_pitch);
	current_roll  = atomic_read(&tilt_roll);

		
	motion_send_tilt_detection(current_yaw, current_pitch, current_roll);
}

static void motion_gyro_work_func(struct work_struct *work)
{
	int current_x = 0, current_y = 0, current_z = 0;

	current_x = atomic_read(&gyro_x);
	current_y = atomic_read(&gyro_y);
	current_z = atomic_read(&gyro_z);

	motion_send_gyro_detection(current_x,current_y,current_z);

}

static void motion_compass_work_func(struct work_struct *work)
{
	int mag_val[3];

	mag_val[0]= atomic_read(&mag_x);
	mag_val[1]= atomic_read(&mag_y);
	mag_val[2]= atomic_read(&mag_z);
	//printk ("[motion_compass_work_func:] mx=[%d], my=[%d], mz=[%d] \n",mag_val[0],mag_val[1],mag_val[2]);	
	
	motion_send_compass_detection(mag_val[0], mag_val[1], mag_val[2]);
}


static void motion_composite_work_func(struct work_struct *work)
{
	int data[3] = {0,};
	int i = 0;

	for (i = 0; i < 3; i++) {
		data[i] = atomic_read(&composite[i]);
	}
	motion_send_composite_detection(data);

}
/*---------------------------------------------------------------------------
	 motion polling timer
   ---------------------------------------------------------------------------*/		  
static enum hrtimer_restart motion_accel_timer_func(struct hrtimer *timer)
{	
	unsigned long  polling_time;   

	if(atomic_read(&accel_flag))
	{	
		queue_work(heaven_motion_dev.accel_wq, &heaven_motion_dev.accel_work);

		polling_time = atomic_read(&accel_delay);
		hrtimer_start(&heaven_motion_dev.timer[0], ktime_set(0,polling_time*1000000), HRTIMER_MODE_REL); 
	}

    	return HRTIMER_NORESTART;
}

static enum hrtimer_restart motion_tilt_timer_func(struct hrtimer *timer)
{	
	unsigned long  polling_time;   

	if(atomic_read(&tilt_flag))
	{	
		queue_work(heaven_motion_dev.tilt_wq, &heaven_motion_dev.tilt_work);

		polling_time = atomic_read(&tilt_delay);
		hrtimer_start(&heaven_motion_dev.timer[1], ktime_set(0,polling_time*1000000), HRTIMER_MODE_REL); 
	}

    	return HRTIMER_NORESTART;
}

static enum hrtimer_restart motion_gyro_timer_func(struct hrtimer *timer)
{
	unsigned long polling_time;

	if (atomic_read(&gyro_flag))
	{
		queue_work(heaven_motion_dev.gyro_wq, &heaven_motion_dev.gyro_work);

		polling_time = atomic_read(&gyro_delay);
		hrtimer_start(&heaven_motion_dev.timer[2],ktime_set(0,polling_time*1000000),HRTIMER_MODE_REL);
	}

	return HRTIMER_NORESTART;
}

static enum hrtimer_restart motion_compass_timer_func(struct hrtimer *timer)
{
	unsigned long polling_time;

	if (atomic_read(&compass_flag)) {
		queue_work(heaven_motion_dev.compass_wq, &heaven_motion_dev.compass_work);

		polling_time = atomic_read(&compass_delay);
		hrtimer_start(&heaven_motion_dev.timer[3], ktime_set(0, polling_time * 1000000), HRTIMER_MODE_REL);
	}

	return HRTIMER_NORESTART;
}


static enum hrtimer_restart motion_composite_timer_func(struct hrtimer *timer)
{
	unsigned long polling_time;

	if (atomic_read(&composite_flag)) {
		queue_work(heaven_motion_dev.composite_wq, &heaven_motion_dev.composite_work);

		polling_time = atomic_read(&composite_delay);
		hrtimer_start(&heaven_motion_dev.timer[4], ktime_set(0, polling_time * 1000000), HRTIMER_MODE_REL);
	}

	return HRTIMER_NORESTART;
}

/*---------------------------------------------------------------------------
	 sensor enable/disable (Sensor HAL)
   ---------------------------------------------------------------------------*/		  
static ssize_t motion_accel_onoff_store(struct device *dev,  struct device_attribute *attr,const char *buf, size_t count)
{
	u32    val;
	
	val = simple_strtoul(buf, NULL, 10);

	//printk("[motion_set_accel_onoff_store]  flag [%d]\n",val);

	if(val)
	{
		atomic_set(&accel_flag, 1);	
	}
	else
	{
		atomic_set(&accel_flag, 0);

		//atomic_set(&accel_x,0);
		//atomic_set(&accel_y,0);
		//atomic_set(&accel_z,0);
	}

	return count;
	
}

static ssize_t motion_tilt_onoff_store(struct device *dev,  struct device_attribute *attr,  char *buf, size_t count)
{
	u32    val;
	int i;	

	val = simple_strtoul(buf, NULL, 10);

	//printk("[motion_set_tilt_onoff_store]  flag [%d]\n",val);
		   
	if(val)
	{
		atomic_set(&tilt_flag, 1);
	}
	else
	{
		atomic_set(&tilt_flag, 0);

		atomic_set(&tilt_roll,  0);
		atomic_set(&tilt_pitch, 0);
		atomic_set(&tilt_yaw,   0);
	}

	return count;
	
}

static ssize_t motion_gyro_onoff_store(struct device *dev, struct device_attribute *attr, char *buf, size_t count)
{
	u32    val;
	val = simple_strtoul(buf, NULL, 10);
	//lprintk("[motion_gyro_onoff_store] gyro_flag [%d]\n",val);

	if (val) {
		atomic_set(&gyro_flag, 1);
	} else {
		atomic_set(&gyro_flag, 0);

		atomic_set(&gyro_x,  0);
		atomic_set(&gyro_y, 0);
		atomic_set(&gyro_z,   0);
		
	}

	return count;
}

static ssize_t motion_compass_onoff_store(struct device *dev, struct device_attribute *attr, char *buf, size_t count)
{
	u32    val;
	val = simple_strtoul(buf, NULL, 10);
	//printk("[motion_compass_onoff_store] compass_flag [%d]\n",val);

	if (val) {
		atomic_set(&compass_flag, 1);
	} else {
		atomic_set(&compass_flag, 0);
	}

	return count;
}

static ssize_t motion_composite_onoff_store(struct device *dev, struct device_attribute *attr, char *buf, size_t count)
{
	u32    val;
	val = simple_strtoul(buf, NULL, 10);
	//printk("[motion_compass_onoff_store] compass_flag [%d]\n",val);

	if (val) {
		atomic_set(&composite_flag, 1);
	} else {
		atomic_set(&composite_flag, 0);

		atomic_set(&composite[0],  0);
		atomic_set(&composite[1],  0);
		atomic_set(&composite[2],  0);	
	}

	return count;
}

static ssize_t motion_tap_onoff_store(struct device *dev,  struct device_attribute *attr,  char *buf, size_t count)
{
	u32    val;
	
	val = simple_strtoul(buf, NULL, 10);

	//printk("[motion_set_tap_onoff_store] tap.... flag [%d]\n",val);

	if(val)
	{
		atomic_set(&tap_flag, 1);

	}
	else
	{
		atomic_set(&tap_flag, 0);
	}
	
	return count;
	
}

static ssize_t motion_flip_onoff_store(struct device *dev,  struct device_attribute *attr,  char *buf, size_t count)
{
	u32    val;
	
	val = simple_strtoul(buf, NULL, 10);

	//printk("[motion_set_flip_onoff_store]  flag [%d]\n",val);

	if(val)
	{
		atomic_set(&flip_flag, 1);
	}
	else
	{
		atomic_set(&flip_flag, 0);	

	}

	return count;
	
}
static ssize_t motion_shake_onoff_store(struct device *dev,  struct device_attribute *attr,const char *buf, size_t count)
{
	u32    val;
	
	val = simple_strtoul(buf, NULL, 10);

	//printk("[motion_set_shake_onoff_store]  flag [%d]\n",val);

	if(val)
	{
		atomic_set(&shake_flag, 1);

	}
	else
	{
		atomic_set(&shake_flag, 0);
	}

	return count;
	
}

static ssize_t motion_snap_onoff_store(struct device *dev,  struct device_attribute *attr,const char *buf, size_t count)
{
	u32    val;
	
	val = simple_strtoul(buf, NULL, 10);

	printk("[motion_set_snap_onoff_store]  flag [%d]\n",val);

	if(val)
	{
		atomic_set(&snap_flag, 1);	
	}
	else
	{
		atomic_set(&snap_flag, 0);
	}

	return count;
	
}

static ssize_t motion_yawimage_onoff_store(struct device *dev,  struct device_attribute *attr,const char *buf, size_t count)
{
	u32    val;
	
	val = simple_strtoul(buf, NULL, 10);

	//printk("[motion_yawimage_onoff_store]  flag [%d]\n",val);
		   
	if(val)
	{
		atomic_set(&yawimage_flag, 1);
	}
	else
	{
		atomic_set(&yawimage_flag, 0);
	}

	return count;
	
}


static ssize_t motion_accel_delay_store(struct device *dev,  struct device_attribute *attr,const char *buf, size_t count)
{
	u32     val;
	unsigned long   current_delay = 100;

	val = simple_strtoul(buf, NULL, 10);

	current_delay = (short)val;

	//printk("[motion_set_accel_delay_store]  val [%d] current_delay[%ld]\n",val,current_delay);

	if(atomic_read(&accel_flag))
	{
		hrtimer_cancel(&heaven_motion_dev.timer[0]);	   

		if(current_delay < MIN_MOTION_POLLING_TIME)
		{
			current_delay = MIN_MOTION_POLLING_TIME;
		}

		atomic_set(&accel_delay, current_delay);
		hrtimer_start(&heaven_motion_dev.timer[0], ktime_set(0,current_delay*1000000), HRTIMER_MODE_REL);
	}

	return count;
}

static ssize_t motion_tilt_delay_store(struct device *dev,  struct device_attribute *attr,const char *buf, size_t count)
{
	u32     val;
	unsigned long   current_delay = 100;

	val = simple_strtoul(buf, NULL, 10);

	current_delay = (short)val;

	//printk("[motion_set_tilt_delay_store]  val [%d] current_delay[%ld]\n",val,current_delay);

	if(atomic_read(&tilt_flag))
	{
		hrtimer_cancel(&heaven_motion_dev.timer[1]);	   

		
		if(current_delay < MIN_MOTION_POLLING_TIME)
		{
			current_delay = MIN_MOTION_POLLING_TIME;
		}
		else if (current_delay == 999)
		{
			current_delay = atomic_read(&tilt_delay);
			if (current_delay < MIN_MOTION_POLLING_TIME)
			{
				current_delay = MIN_MOTION_POLLING_TIME;
			}
		}

		//printk("[motion_set_tilt_delay_store]  val [%d] current_delay[%ld]\n",val,current_delay);
		
		atomic_set(&tilt_delay, current_delay);
		hrtimer_start(&heaven_motion_dev.timer[1], ktime_set(0,current_delay*1000000), HRTIMER_MODE_REL);

	}

	return count;

}

ssize_t motion_gyro_delay_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	u32    val;
	unsigned long current_delay = 100;

	val = simple_strtoul(buf, NULL, 10);
	current_delay = (short)val;

#if DEBUG
	//lprintk(D_SENSOR, "[motion_gyro_delay_store]  val [%d] current_delay[%ld]\n",val,current_delay);
#endif

	if (atomic_read(&gyro_flag)) {
		hrtimer_cancel(&heaven_motion_dev.timer[2]);

		if (current_delay < MIN_MOTION_POLLING_TIME) {
			current_delay = MIN_MOTION_POLLING_TIME;
		}

		atomic_set(&gyro_delay, current_delay);
		hrtimer_start(&heaven_motion_dev.timer[2], ktime_set(0, current_delay * 1000000), HRTIMER_MODE_REL);

	}

	return count;
}

ssize_t motion_compass_delay_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	u32    val;
	unsigned long current_delay = 100;

	val = simple_strtoul(buf, NULL, 10);
	current_delay = (short)val;

#if DEBUG
	//lprintk(D_SENSOR,"[motion_compass_delay_store]  flag [%d] current_delay[%ld]\n",val, current_delay);
#endif

	if (atomic_read(&compass_flag)) {
		hrtimer_cancel(&heaven_motion_dev.timer[3]);

		if (current_delay < MIN_MOTION_POLLING_TIME) {
			current_delay = MIN_MOTION_POLLING_TIME;
		}

		atomic_set(&compass_delay, current_delay);
		hrtimer_start(&heaven_motion_dev.timer[3], ktime_set(0, current_delay * 1000000), HRTIMER_MODE_REL);
	}

	return count;
}

ssize_t motion_composite_delay_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	u32    val;
	unsigned long current_delay = 100;

	val = simple_strtoul(buf, NULL, 10);
	current_delay = (short)val;

#if DEBUG
	//lprintk(D_SENSOR,"[motion_compass_delay_store]  flag [%d] current_delay[%ld]\n",val, current_delay);
#endif

	if (atomic_read(&composite_flag)) {
		hrtimer_cancel(&heaven_motion_dev.timer[4]);

		if (current_delay < MIN_MOTION_POLLING_TIME) {
			current_delay = MIN_MOTION_POLLING_TIME;
		}

		atomic_set(&composite_delay, current_delay);
		hrtimer_start(&heaven_motion_dev.timer[4], ktime_set(0, current_delay * 1000000), HRTIMER_MODE_REL);
	}

	return count;
}

static ssize_t motion_cal_onoff_show(struct device *dev, struct device_attribute *attr, char *buf, size_t count)
{
	u32    val;
	val = atomic_read(&cal_result);
	atomic_set(&cal_result, 2);
	return sprintf(buf, "%d\n",val);
}

static ssize_t motion_cal_onoff_store(struct device *dev, struct device_attribute *attr, char *buf, size_t count)
{
	u32    val;
	val = simple_strtoul(buf, NULL, 10);

	if (val) {
		atomic_set(&cal_flag, 1);
	} else {
		atomic_set(&cal_flag, 0);
	}

	return count;
}


static DEVICE_ATTR(accel_onoff,0666,NULL,motion_accel_onoff_store);
static DEVICE_ATTR(tilt_onoff,0666,NULL,motion_tilt_onoff_store);
static DEVICE_ATTR(gyro_onoff, 0666, NULL, motion_gyro_onoff_store);
static DEVICE_ATTR(compass_onoff, 0666, NULL, motion_compass_onoff_store);
static DEVICE_ATTR(tap_onoff,0666,NULL,motion_tap_onoff_store);
static DEVICE_ATTR(flip_onoff,0666,NULL,motion_flip_onoff_store);
static DEVICE_ATTR(shake_onoff,0666,NULL,motion_shake_onoff_store);
static DEVICE_ATTR(snap_onoff,0666,NULL,motion_snap_onoff_store);
static DEVICE_ATTR(yawimage_onoff,0666,NULL,motion_yawimage_onoff_store);
static DEVICE_ATTR(composite_onoff,0666,NULL,motion_composite_onoff_store);

static DEVICE_ATTR(accel_delay,0666,NULL,motion_accel_delay_store);
static DEVICE_ATTR(tilt_delay,0666,NULL,motion_tilt_delay_store);
static DEVICE_ATTR(gyro_delay, 0666, NULL, motion_gyro_delay_store);
static DEVICE_ATTR(compass_delay, 0666, NULL, motion_compass_delay_store);
static DEVICE_ATTR(composite_delay, 0666, NULL, motion_composite_delay_store);


//Sensor Calibration
static DEVICE_ATTR(cal_onoff, 0666, motion_cal_onoff_show, motion_cal_onoff_store);

static struct attribute *heaven_motion_attributes[] = {
	&dev_attr_accel_onoff.attr,
	&dev_attr_gyro_onoff.attr,		
	&dev_attr_tilt_onoff.attr,
	&dev_attr_compass_onoff.attr,
	&dev_attr_composite_onoff.attr,		
	&dev_attr_tap_onoff.attr,
	&dev_attr_shake_onoff.attr,
	&dev_attr_snap_onoff.attr,
	&dev_attr_flip_onoff.attr,
	&dev_attr_yawimage_onoff.attr,		
	&dev_attr_tilt_delay.attr,
	&dev_attr_accel_delay.attr,
	&dev_attr_gyro_delay.attr,
	&dev_attr_compass_delay.attr,
	&dev_attr_composite_delay.attr,		
	&dev_attr_cal_onoff.attr,	
	NULL
};

static const struct attribute_group heaven_motion_group = {
	.attrs = heaven_motion_attributes,
};

/*---------------------------------------------------------------------------
	 ioctl command for heaven motion daemon process
   ---------------------------------------------------------------------------*/		  
static int heaven_motion_open(struct inode *inode, struct file *file)
{
	int status = -1;
	
       status = nonseekable_open(inode,file);      
	   
       return status;
}
static int heaven_motion_release(struct inode *inode, struct file *file)
{
	//printk("motion close\n");
	return 0;
}
static int heaven_motion_ioctl(struct inode *inode, struct file *file, unsigned int cmd,unsigned long arg)
{
	void __user *argp = (void __user *)arg;
	unsigned char   	data[MAX_MOTION_DATA_LENGTH]={0,};	
	int                        buf[13] = {0,};
	int     		     	flag = 0;
	int                        delay = 0;
	unsigned char   	rwbuf[200]={0,};     /* MPU3050 i2c MAX data length */
       int 				ret = 0;
	int i = 0;
	int onoff_flag = 0;
	char buffer[2];

	switch (cmd) 
	{
	case MOTION_IOCTL_ENABLE_DISABLE:
		 /*
			0 : disable sensor
			1:  orientation (tilt)
			2:  accelerometer
			3: tap
			4: shake
		*/
		flag = HEAVEN_SENSOR_NONE;

		if(atomic_read(&accel_flag)){
			flag |= HEAVEN_ACCELEROMETER;
		}

		if(atomic_read(&tilt_flag)){
			
			flag |= HEAVEN_TILT;
		}

		if (atomic_read(&gyro_flag)) {			
			flag |= HEAVEN_GYRO;
		}

		if (atomic_read(&compass_flag)) {			
			flag |= HEAVEN_COMPASS;
		}

		if (atomic_read(&composite_flag)) {			
			flag |= HEAVEN_COMPOSITE;
		}	

		if(atomic_read(&tap_flag)){
			flag |= HEAVEN_TAP;
		}

		if(atomic_read(&flip_flag)){
			flag |= HEAVEN_FLIP;
		}

		if(atomic_read(&shake_flag)){
			flag |= HEAVEN_SHAKE;
		}

		if(atomic_read(&snap_flag)){
			flag |= HEAVEN_SNAP;
		}

		if(atomic_read(&yawimage_flag)){
			flag |= HEAVEN_YAWIMAGE;
		}


		if (atomic_read(&cal_flag)) {
		//lprintk(".............if(atomic_read(&snap_flag)){................\n");
			flag |= HEAVEN_CALIBRATION;
		}
		
		 if (copy_to_user(argp,&flag, sizeof(flag)))
		 {
			return -EFAULT;
		 } 
		 break;
	case MOTION_IOCTL_ACCEL_RAW:
		  if (copy_from_user(&buf, argp, sizeof(buf)))
		  {
			return -EFAULT;
		   }		  
		 /* 
			buf[0] = x;   
			buf[1] = y;			
			buf[2] = z;				 
		  */
		   atomic_set(&accel_x,buf[0]);  
		   atomic_set(&accel_y,buf[1]);
		   atomic_set(&accel_z,buf[2]);
		   //motion_send_accel_detection(buf[0],buf[1],buf[2]);

		   break;  		 		    	 
	case MOTION_IOCTL_TILT:
		  if (copy_from_user(&buf, argp, sizeof(buf)))
		  {
			return -EFAULT;
		   }		  

		   atomic_set(&tilt_yaw,buf[0]);
		   atomic_set(&tilt_pitch,buf[1]);
		   atomic_set(&tilt_roll,buf[2]);  

		   break;
	case MOTION_IOCTL_COMPOSITE:
		  if (copy_from_user(&buf, argp, sizeof(buf)))
		  {
			return -EFAULT;
		   }		  
		   for(i=0;i<3;i++)
		   {
		   		atomic_set(&composite[i],buf[i]);  
		   }

		   break;
	case MOTION_IOCTL_GYRO_RAW:
		if (copy_from_user(&buf, argp, sizeof(buf))) {
			return -EFAULT;
		}
		/* 	buf[0], [1], [2] = gyro_x,  gyro_y,  gyro_z; */
		atomic_set(&gyro_x, buf[0]);
		atomic_set(&gyro_y, buf[1]);
		atomic_set(&gyro_z, buf[2]);

		break;
	case MOTION_IOCTL_MAGNETIC_RAW: /* wkkim add to recieve compass raw value */
		if (copy_from_user(buf, argp, sizeof(int) * 3)) {
			return -EFAULT;
		}
		//magnetic_input_report(buf);
		atomic_set(&mag_x, buf[0]);
		atomic_set(&mag_y, buf[1]);
		atomic_set(&mag_z, buf[2]);
		break;
	case MOTION_IOCTL_TAP:         
		 if (copy_from_user(&buf, argp, sizeof(buf)))
		  {
			return -EFAULT;
		   }		  
		 /* 
			buf[0] = type;   
			buf[1] = direction;			 
		  */
		   motion_send_tap_detection(buf[0],buf[1]);
		 break;
	case MOTION_IOCTL_FLIP:
		 if (copy_from_user(&buf, argp, sizeof(buf)))
		  {
			return -EFAULT;
		   }		  
		   motion_send_flip_detection(buf[0]);
		 break;
       case MOTION_IOCTL_SHAKE:   
	   	  if (copy_from_user(&buf, argp, sizeof(buf)))
		  {
			return -EFAULT;
		   }		  
		 /* 
			buf[0] = event;   
		  */
		   motion_send_shake_detection(buf[0]);
		 break;
	case MOTION_IOCTL_SNAP:    
		 if (copy_from_user(&buf, argp, sizeof(buf)))
		  {
			return -EFAULT;
		  }
		 /* 
			buf[0] = direction;
		  */
		 motion_send_snap_detection(buf[0]);
		 break;	 

	case MOTION_IOCTL_YAWIMAGE:    
		 if (copy_from_user(&buf, argp, sizeof(buf)))
		 {
			return -EFAULT;
		 }
		 // buf[0] = positive or negative
		 motion_send_yawimage_detection(buf[0]);
		 break;	 


	case MOTION_IOCTL_SENSOR_DELAY:
		  delay = atomic_read(&tilt_delay);

		  printk("MOTION_IOCTL_SENSOR_DELAY[%d]",delay);
		  
		 if (copy_to_user(argp, &delay, sizeof(delay)))
		 {
		 	 return -EFAULT;
		 }
		 break;
	case MOTION_IOCTL_SENSOR_SUSPEND_RESUME:
		onoff_flag = atomic_read(&suspend_flag);

		if (copy_to_user(argp, &onoff_flag, sizeof(onoff_flag))) {
			return -EFAULT;
		}
		break;
	case MOTION_IOCTL_ACCEL_COMPASS_SLEEP_MODE:
		//lprintk(".............MOTION_IOCTL_ACCEL_COMPASS_SLEEP_MODE................\n");
		motion_sensor_power_off();
		break;
	case MOTION_IOCTL_ACCEL_COMPASS_SLEEP_WAKE_UP:
		//lprintk(".............MOTION_IOCTL_ACCEL_COMPASS_SLEEP_WAKE_UP................\n");
		motion_sensor_power_on();
		break;		  
	case MOTION_IOCTL_MPU3050_I2C_READ:
		if (copy_from_user(&rwbuf, argp, sizeof(rwbuf)))
		{
			printk("FAIL!!!!!!copy_from_user.................MOTION_IOCTL_MPU3050_I2C_READ");
			return -EFAULT;
		}
		
		/*	
		rwbuf[0] = slave_addr;   // slave addr - GYRO(0x68-MPU) 
     		rwbuf[1] = length;            // number of bytes to read
     		rwbuf[2] = reg;	            // register address	
		rwbuf[3] = data;  
		*/
		if (rwbuf[1] < 1)
		{
			printk("EINVAL ERROR......I2C SLAVE MOTION_IOCTL_I2C_READ : rwbuf[1] < 1...\n");
			
		        return -EINVAL;
		}

		if(rwbuf[0] == GYRO_I2C_SLAVE_ADDR)
		{
			ret = mpu3050_i2c_read(rwbuf[2],&rwbuf[3],rwbuf[1]);
			if (ret < 0){			
				printk("MOTION_IOCTL_I2C_READ : GYRO_I2C_SLAVE_ADDR Address ERROR[%d]\n",rwbuf[0]);
				return -EINVAL;
			}
		}
		else if(rwbuf[0] == 0x0F)
		{
			ret = kxtf9_i2c_read(rwbuf[2],&rwbuf[3],rwbuf[1]);
			if (ret < 0){
				printk("MOTION_IOCTL_I2C_READ : ACCEL_I2C_SLAVE_ADDR Address ERROR[%d]\n",rwbuf[0]);
				return -EINVAL;
			}
		}
		else if(rwbuf[0] == 0x0E)
		{
			rwbuf[3] = rwbuf[2];
			ret = AKI2C_RxData(&rwbuf[3],rwbuf[1]);
			if (ret < 0){
				printk("MOTION_IOCTL_I2C_READ : ACCEL_I2C_SLAVE_ADDR Address ERROR[%d]\n",rwbuf[0]);
				return -EINVAL;
			}
		}		
		else
		{
			printk("......I2C SLAVE ADDRESS ERROR!!!...[0x%x]...\n",rwbuf[0]);
			return -EINVAL;
		}
		
		if (copy_to_user(argp, &rwbuf, sizeof(rwbuf)))
		{
		       printk("EINVAL ERROR......I2C SLAVE MOTION_IOCTL_I2C_READ : rwbuf[1] < 1...\n");
		 	 return -EFAULT;
		}
			
		break;
	case MOTION_IOCTL_MPU3050_I2C_WRITE:
		if (copy_from_user(&rwbuf, argp, sizeof(rwbuf)))
		{
		       printk("EINVAL ERROR.....copy_from_user.I2C SLAVE MOTION_IOCTL_I2C_WRITE \n");
			return -EFAULT;
		}
		/*	
   		rwbuf[0] = slave_addr;  // slave addr - GYRO(0x68-MPU) 
     		rwbuf[1] = 2;                   // number of bytes to write +1
     		rwbuf[2] = reg;               // register address
     		rwbuf[3] = value;          // register value		
		*/
		if (rwbuf[1] < 2)
		{
		       printk("MOTION_IOCTL_WRITE ..length ERROR!!![%d].....\n",rwbuf[1]);
			return -EINVAL;
		}

		if(rwbuf[0] == GYRO_I2C_SLAVE_ADDR)
		{
			ret = mpu3050_i2c_write(&rwbuf[2],rwbuf[1]);
			if (ret < 0){
				 printk("MOTION_IOCTL_WRITE  : GYRO_I2C_SLAVE_ADDR Address ERROR[%d]\n",rwbuf[0]);
				return -EINVAL;
			}
		}
		else if(rwbuf[0] == 0x0F)
		{
			ret = kxtf9_i2c_write(rwbuf[2],rwbuf[3]);
			if (ret < 0){
				  printk("[KXTF9] MOTION_IOCTL_WRITE  : ACCEL_I2C_SLAVE_ADDR ERROR[%d]\n",rwbuf[0]);
				return -EINVAL;
			}
		}
		else if(rwbuf[0] == 0x0E)
		{
			ret = AKI2C_TxData(&rwbuf[2],rwbuf[1]);
			if (ret < 0){
				  printk("[KXTF9] MOTION_IOCTL_WRITE  : COMPASS_I2C_SLAVE_ADDR ERROR[%d]\n",rwbuf[0]);
				return -EINVAL;
			}
		}		
		else
		{
			printk("......I2C SLAVE ADDRESS ERROR!!!...[0x%x]...\n",rwbuf[0]);
			return -EINVAL;
		}
		break;
	case MOTION_IOCTL_ACCEL_INT_ENABLE_DISABLE:
		//lprintk(".............MOTION_IOCTL_ACCEL_INT_ENABLE_DISABLE................\n");
		if (copy_from_user(&buf, argp, sizeof(buf))) {
			return -EFAULT;
		}
		atomic_set(&bypass_flag, buf[0]);
		onoff_flag = atomic_read(&bypass_flag);
		if(onoff_flag == 1)
		{
			kxtf9_enable_irq();
		}else if(onoff_flag == 0){
			//kxtf9_accel_disable_tap();
			kxtf9_disable_irq();
		}
		break;		
	case MOTION_IOCTL_ACCEL_INIT:
		kxtf9_accel_init();
		break;	

		case MOTION_IOCTL_CALIBRATION_FINISHED:
			printk(".............MOTION_IOCTL_CALIBRATION_FINISHED................\n");
			if (copy_from_user(&buf, argp, sizeof(buf))) {
				return -EFAULT;
			}

			atomic_set(&cal_flag, buf[0]);
			atomic_set(&cal_result, buf[1]);
			
			break;

	default:
		break;
	}

	return 0;
	
}

static struct file_operations  heaven_motion_fops = {
	.owner    = THIS_MODULE,
	.open      = heaven_motion_open,
	.release  = heaven_motion_release,
	.ioctl       = heaven_motion_ioctl,
};

static struct miscdevice  heaven_motion_misc_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = HEAVEN_MOTION_IOCTL_NAME,
	.fops   = &heaven_motion_fops,
};

/*---------------------------------------------------------------------------
	 accel.  driver
---------------------------------------------------------------------------*/		  
static int __init accel_i2c_probe(struct i2c_client *client)
{	
       printk("----------accel_i2c_probe\n");	
	kxtf9_reg_i2c_client(client);

	return 0;
}
static int  accel_i2c_remove(struct i2c_client *client)
{
	return 0;
}

#define accel_i2c_suspend	NULL
#define accel_i2c_resume		NULL

static const struct i2c_device_id accel_i2c_id[] = {
	{HEAVEN_I2C_ACCEL_NAME, 0 },	
	{ /* end of list */ },
};

static struct i2c_driver accel_i2c_driver = {
       .probe     =  accel_i2c_probe,
	.remove  =  accel_i2c_remove,
	.id_table = accel_i2c_id,
#ifdef CONFIG_PM
	.suspend = accel_i2c_suspend,
	.resume	 = accel_i2c_resume,
#endif
	.driver = {
       .name = HEAVEN_I2C_ACCEL_NAME,
	},
};

/*---------------------------------------------------------------------------
	 gyro driver
---------------------------------------------------------------------------*/		  
static int __init gyro_i2c_probe(struct i2c_client *client)
{	
	printk("----------gyro_i2c_probe\n");	
	mpu3050_reg_i2c_client(client);

	return 0;
		
}
static int  gyro_i2c_remove(struct i2c_client *client)
{
	return 0;
}

#define gyro_i2c_suspend		NULL
#define gyro_i2c_resume		NULL

static const struct i2c_device_id gyro_i2c_id[] = {
	{HEAVEN_I2C_GYRO_NAME, 0 },	
	{ /* end of list */ },
};

static struct i2c_driver gyro_i2c_driver = {
       .probe = gyro_i2c_probe,
	.remove =  gyro_i2c_remove,
	.id_table = gyro_i2c_id,
#ifdef CONFIG_PM
	.suspend = gyro_i2c_suspend,
	.resume	 = gyro_i2c_resume,
#endif
	.driver = {
       .name = HEAVEN_I2C_GYRO_NAME,
	},
};

/*---------------------------------------------------------------------------
	 platform device
   ---------------------------------------------------------------------------*/		  
static int __init heaven_motion_probe(struct platform_device *pdev)
{
     int err = 0;
     struct device *dev = &pdev->dev;	 

     printk(KERN_INFO"%s: probe start\n", __func__);
    
   /*---------------------------------------------------------------------------
       register i2c driver
     ---------------------------------------------------------------------------*/	 	
      err = i2c_add_driver(&gyro_i2c_driver);
     if(err < 0){
	    printk("************* LGE: gyro_i2c_test_client fail\n");
	    goto err_i2c_add_driver;
     }

     err = i2c_add_driver(&accel_i2c_driver);
     if(err < 0){
	    printk("************* LGE: accel_i2c_test_client fail\n");
	    goto err_i2c_add_driver;
     }	 
  
    

     /*---------------------------------------------------------------------------
       register misc device
     ---------------------------------------------------------------------------*/	 
       err = misc_register(&heaven_motion_misc_device);
       if (err) {
		printk(KERN_ERR"heaven_motion_misc_device register failed\n");
		goto exit_misc_device_register_failed;
	}

     /*---------------------------------------------------------------------------
       register input device
      ---------------------------------------------------------------------------*/
      heaven_motion_dev.input_dev = input_allocate_device();
      if(heaven_motion_dev.input_dev == NULL)
      {
           printk(KERN_ERR"heaven_motion_sesnor_probe: input_allocate_device (1) failed\n");
           goto err_input_allocate1;		
      }  

      heaven_motion_dev.input_dev->name = HEAVEN_MOTION_INPUT_NAME;
  
       set_bit(EV_SYN,heaven_motion_dev.input_dev->evbit);	
	set_bit(EV_REL,heaven_motion_dev.input_dev->evbit);

	set_bit(REL_X,heaven_motion_dev.input_dev->relbit);  // TAP - Type 
	set_bit(REL_Y,heaven_motion_dev.input_dev->relbit);  // TAP - Direction   
	set_bit(REL_RX,heaven_motion_dev.input_dev->relbit);  // TILT - Roll
	set_bit(REL_RY,heaven_motion_dev.input_dev->relbit);  // TILT - PITCH   
	set_bit(REL_RZ,heaven_motion_dev.input_dev->relbit);  // TILT - Yaw
	set_bit(REL_HWHEEL,heaven_motion_dev.input_dev->relbit); // SHAKE
	set_bit(REL_DIAL,heaven_motion_dev.input_dev->relbit);   // SNAP - Direction 
	set_bit(REL_WHEEL,heaven_motion_dev.input_dev->relbit);  // FLIP

	set_bit(REL_Z,heaven_motion_dev.input_dev->relbit);  	// gyro_x
	set_bit(REL_MISC,heaven_motion_dev.input_dev->relbit);  // gyro_y
	set_bit(REL_MAX,heaven_motion_dev.input_dev->relbit);  	// gyro_z
  
 	set_bit(EV_ABS,heaven_motion_dev.input_dev->evbit);
	input_set_abs_params(heaven_motion_dev.input_dev, ABS_X, -5760, 5760, 0, 0);
	input_set_abs_params(heaven_motion_dev.input_dev, ABS_Y, -5760, 5760, 0, 0);
	input_set_abs_params(heaven_motion_dev.input_dev, ABS_Z, -5760, 5760, 0, 0);

	input_set_abs_params(heaven_motion_dev.input_dev, ABS_GAS, 0, 0, 0, 0);
	input_set_abs_params(heaven_motion_dev.input_dev, ABS_HAT1X, 0, 0, 0, 0);
	input_set_abs_params(heaven_motion_dev.input_dev, ABS_HAT1Y, 0, 0, 0, 0);
	//input_set_abs_params(heaven_motion_dev.input_dev, ABS_HAT2X, 0, 0, 0, 0);
	//input_set_abs_params(heaven_motion_dev.input_dev, ABS_HAT2Y, 0, 0, 0, 0);
	//input_set_abs_params(heaven_motion_dev.input_dev, ABS_HAT3X, 0, 0, 0, 0);
	//input_set_abs_params(heaven_motion_dev.input_dev, ABS_HAT3Y, 0, 0, 0, 0);
	//input_set_abs_params(heaven_motion_dev.input_dev, ABS_TILT_X, 0, 0, 0, 0);
	//input_set_abs_params(heaven_motion_dev.input_dev, ABS_TILT_Y, 0, 0, 0, 0);
	//input_set_abs_params(heaven_motion_dev.input_dev, ABS_TOOL_WIDTH, 0, 0, 0, 0);
	//input_set_abs_params(heaven_motion_dev.input_dev, ABS_VOLUME, 0, 0, 0, 0);
	//input_set_abs_params(heaven_motion_dev.input_dev, ABS_MISC, 0, 0, 0, 0);

	/* x-axis of raw magnetic vector */
	input_set_abs_params(heaven_motion_dev.input_dev, ABS_HAT0X, -2048, 2032, 0, 0);
	/* y-axis of raw magnetic vector */
	input_set_abs_params(heaven_motion_dev.input_dev, ABS_HAT0Y, -2048, 2032, 0, 0);
	/* z-axis of raw magnetic vector */
	input_set_abs_params(heaven_motion_dev.input_dev, ABS_BRAKE, -2048, 2032, 0, 0);

	input_set_abs_params(heaven_motion_dev.input_dev, ABS_THROTTLE, 0, 0, 0, 0); //Yaw Image
       
       err = input_register_device(heaven_motion_dev.input_dev);
       if(err){
             printk(KERN_ERR"heaven_motion_sesnor_probe: input_allocate_device (1) failed \n");
             goto err_input_allocate1;		
       }

#if 1 /*MUST DO IT*/
//	atomic_set(&accel_flag, 1);
//	atomic_set(&accel_delay, 200);
#endif

#if 1
	atomic_set(&accel_x,0);
	atomic_set(&accel_y,10240);
	atomic_set(&accel_z,0);
#endif

	atomic_set(&cal_result, 2); //0: pass 1:fail 2:initial state

	/*---------------------------------------------------------------------------
		init. sysfs
	---------------------------------------------------------------------------*/			 
       if ((err = sysfs_create_group(&dev->kobj, &heaven_motion_group)))
       {
	    printk("[motion_sensor] sysfs_create_group FAIL \n");
	    goto err_sysfs_create;
      }

	/*---------------------------------------------------------------------------
		INIT_WORK 
	---------------------------------------------------------------------------*/		  			
	INIT_WORK(&heaven_motion_dev.accel_work, motion_accel_work_func);
	INIT_WORK(&heaven_motion_dev.tilt_work, motion_tilt_work_func);
	INIT_WORK(&heaven_motion_dev.gyro_work, motion_gyro_work_func);
	INIT_WORK(&heaven_motion_dev.compass_work, motion_compass_work_func);
	INIT_WORK(&heaven_motion_dev.composite_work, motion_composite_work_func);

    /*---------------------------------------------------------------------------
		init. workqueue 
	---------------------------------------------------------------------------*/		  			
	heaven_motion_dev.accel_wq = create_singlethread_workqueue("motion_accel_wq");
	if (!heaven_motion_dev.accel_wq) {
		printk("[motion_sensor] couldn't create accel queue\n");
		goto err_motion_accel_wq;
	}

	heaven_motion_dev.tilt_wq = create_singlethread_workqueue("motion_tilt_wq");
	if (!heaven_motion_dev.tilt_wq) {
		printk("[motion_sensor] couldn't create tilt queue\n");
		goto err_motion_tilt_wq;
	}

	heaven_motion_dev.gyro_wq = create_singlethread_workqueue("motion_gyro_wq");
	if (!heaven_motion_dev.gyro_wq) {
		printk("[motion_sensor] couldn't create gyro work queue\n");
		goto err_motion_gyro_wq;
	}

	heaven_motion_dev.compass_wq = create_singlethread_workqueue("motion_compass_wq");
	if (!heaven_motion_dev.compass_wq) {
		printk("[motion_sensor] couldn't create accel work queue\n");
		goto err_motion_compass_wq;
	}

	heaven_motion_dev.composite_wq = create_singlethread_workqueue("motion_composite_wq");
	if (!heaven_motion_dev.composite_wq) {
		printk("[motion_sensor] couldn't create accel work queue\n");
		goto err_motion_composite_wq;
	}

   /*---------------------------------------------------------------------------
		init. timer
    ---------------------------------------------------------------------------*/
	hrtimer_init(&heaven_motion_dev.timer[0], CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	heaven_motion_dev.timer[0].function = motion_accel_timer_func;
	 
	hrtimer_init(&heaven_motion_dev.timer[1], CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	heaven_motion_dev.timer[1].function = motion_tilt_timer_func;

	hrtimer_init(&heaven_motion_dev.timer[2], CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	heaven_motion_dev.timer[2].function = motion_gyro_timer_func;

	hrtimer_init(&heaven_motion_dev.timer[3], CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	heaven_motion_dev.timer[3].function = motion_compass_timer_func;
	
	hrtimer_init(&heaven_motion_dev.timer[4], CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	heaven_motion_dev.timer[4].function = motion_composite_timer_func;
	
     /*---------------------------------------------------------------------------
       power 
     ---------------------------------------------------------------------------*/	
	heaven_gyro_vio_reg = regulator_get(&pdev->dev, "vdac");
	if (heaven_gyro_vio_reg == NULL) {
		printk(KERN_ERR": Failed to get motion power resources !! \n");
		return -ENODEV;
	}

	heaven_motion_reg = regulator_get(&pdev->dev, "vaux2");
	if (heaven_motion_reg == NULL) {
		printk(KERN_ERR": Failed to get motion power resources !! \n");
		return -ENODEV;
	}
	  
#ifdef CONFIG_HAS_EARLYSUSPEND
	heaven_motion_dev.early_suspend.level = EARLY_SUSPEND_LEVEL_STOP_DRAWING;
	heaven_motion_dev.early_suspend.suspend = mpu3050_early_suspend;
	heaven_motion_dev.early_suspend.resume = mpu3050_late_resume;
	register_early_suspend(&heaven_motion_dev.early_suspend);
#endif

	 return 0;	

err_i2c_add_driver:
       i2c_del_driver(&gyro_i2c_driver);
	i2c_del_driver(&accel_i2c_driver);
err_input_allocate1:   
	input_unregister_device(heaven_motion_dev.input_dev);	
exit_misc_device_register_failed:
err_sysfs_create:
       printk("+++++ LGE: heaven motion misc_device_register_failed\n");        
err_motion_accel_wq:
	printk("##  sensor: accel timer_failed\n");
	destroy_workqueue(heaven_motion_dev.accel_wq);
err_motion_tilt_wq:
	printk("##  sensor: tilt timer_failed\n");
	destroy_workqueue(heaven_motion_dev.tilt_wq);
err_motion_gyro_wq:
	printk("##  sensor: gyro timer_failed\n");
	destroy_workqueue(heaven_motion_dev.gyro_wq);
err_motion_compass_wq:
	printk("##  sensor: compass timer_failed\n");
	destroy_workqueue(heaven_motion_dev.compass_wq);
err_motion_composite_wq:
	printk("##  sensor: composite timer_failed\n");
	destroy_workqueue(heaven_motion_dev.composite_wq);	
	return err;

}
static int heaven_motion_remove(struct platform_device *pdev)
{
	//struct device *dev = &pdev->dev;   
	input_unregister_device(heaven_motion_dev.input_dev);
	
	i2c_del_driver(&gyro_i2c_driver);
	i2c_del_driver(&accel_i2c_driver);

	if (heaven_motion_dev.tilt_wq)
		destroy_workqueue(heaven_motion_dev.tilt_wq);
	if (heaven_motion_dev.accel_wq)
		destroy_workqueue(heaven_motion_dev.accel_wq);
	   
    return 0;
}

static int heaven_motion_suspend(struct platform_device *pdev)
{
	atomic_set(&suspend_flag, 1);
	return 0;
}

static int heaven_motion_resume(struct platform_device *pdev)
{
	atomic_set(&suspend_flag, 0);
	return 0;
}

#ifdef CONFIG_HAS_EARLYSUSPEND
static void mpu3050_early_suspend(struct early_suspend *h)
{
	//printk("%s\n", __func__);
	heaven_motion_suspend(NULL);

	#if 1
	//timer removed
	if(atomic_read(&accel_flag)){
		hrtimer_cancel(&heaven_motion_dev.timer[0]);
	}
	if(atomic_read(&tilt_flag)){
		hrtimer_cancel(&heaven_motion_dev.timer[1]);
	}
	if(atomic_read(&gyro_flag)){
		hrtimer_cancel(&heaven_motion_dev.timer[2]);
	}	
	if(atomic_read(&compass_flag)){
		hrtimer_cancel(&heaven_motion_dev.timer[3]);
	}
	if(atomic_read(&composite_flag)){
		hrtimer_cancel(&heaven_motion_dev.timer[4]);
	}
	#endif
	
	return;
}

static void mpu3050_late_resume(struct early_suspend *h)
{
	unsigned long current_delay;

	//printk("%s\n", __func__);
	heaven_motion_resume(NULL);
	
	#if 1
	//timer restart
	if(atomic_read(&accel_flag)){
		current_delay = atomic_read(&accel_delay);
		hrtimer_start(&heaven_motion_dev.timer[0], ktime_set(0, current_delay * 1000000), HRTIMER_MODE_REL);
	}
	if(atomic_read(&tilt_flag)){
		current_delay = atomic_read(&tilt_delay);
		hrtimer_start(&heaven_motion_dev.timer[1], ktime_set(0, current_delay * 1000000), HRTIMER_MODE_REL);
	}
	if(atomic_read(&gyro_flag)){
		current_delay = atomic_read(&gyro_delay);
		hrtimer_start(&heaven_motion_dev.timer[2], ktime_set(0, current_delay * 1000000), HRTIMER_MODE_REL);
	}	
	if(atomic_read(&compass_flag)){
		current_delay = atomic_read(&compass_delay);
		hrtimer_start(&heaven_motion_dev.timer[3], ktime_set(0, current_delay * 1000000), HRTIMER_MODE_REL);
	}
	if(atomic_read(&composite_flag)){
		current_delay = atomic_read(&composite_delay);
		hrtimer_start(&heaven_motion_dev.timer[4], ktime_set(0, current_delay * 1000000), HRTIMER_MODE_REL);
	}	
	#endif
	
	return;	
}
#endif

static struct platform_driver heaven_motion_driver = {
	.probe      = heaven_motion_probe,
	.remove   =  heaven_motion_remove,
	.suspend  =  heaven_motion_suspend,
	.resume   =  heaven_motion_resume,
	.driver =  {
		       .name = HEAVEN_MOTION_SENSOR_NAME,
		       .owner = THIS_MODULE,
	  },
};
static int __init heaven_motion_init(void)
{	
     int err;

     err = platform_driver_register(&heaven_motion_driver);
	
     return 0;
}
static void __exit heaven_motion_exit(void)
{
      printk(KERN_INFO "lge heaven_motion_exit was unloaded!\nHave a nice day!\n");
	platform_driver_unregister(&heaven_motion_driver);	  
      return;
}

module_init(heaven_motion_init);
module_exit(heaven_motion_exit);

MODULE_AUTHOR("LG Electronics");
MODULE_DESCRIPTION("HEAVEN MOTION Driver for heaven");
MODULE_LICENSE("GPL");

