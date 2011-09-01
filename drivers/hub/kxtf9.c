/*
 *  kxtf9.c 
 *
 *  KXTF9 Accelerometer Driver   
 * 
 *  Copyright (C) 2009 LGE Inc.
 *
 *  2009/09/05 : created by <chanhee.park@lge.com>
 *
 */

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
#include <linux/slab.h>
#include <mach/hardware.h>
#include <asm/uaccess.h>
#include <linux/ioctl.h>
#include <linux/miscdevice.h>
#include <linux/regulator/consumer.h>
#ifdef CONFIG_ARCH_OMAP
#include <mach/gpio.h>
#endif
#include "kxtf9_if.h"
#include "kxtf9.h"
#include "kionix_accel_api.h"

  
#define  MISC_DYNAMIC_MINOR		 	255
 
#define  MAX_MOTION_DATA_LENGTH	 	10
 
#define  MIN_MOTION_POLLING_TIME    10
 
#define  ACCEL_INT_GPIO  			42   /* hub model */


#define DEBUG   0
#define NEW_GESTURE 1
#define NO_COMPASS_SUPPORT 1

int tap_onoff_count = 0;
int flip_onoff_count = 0;
int snap_onoff_count = 0;
int shake_onoff_count = 0;

#if DEBUG
u32 kxtf_reg_index;
u32 kxtf_reg_value;
#endif
static struct i2c_client *this_client;


struct kxtf9_data {
	 struct input_dev			  *input_dev;	 /* sensor HAL */
	 struct hrtimer 			  timer;	 
	 
	 struct work_struct 		  shake_work;
	 struct work_struct 		  isr_work;
     struct work_struct           snap_work;
	 
	 struct workqueue_struct	 *timer_wq;
	 struct workqueue_struct	 *isr_wq;
     struct workqueue_struct     *snap_wq;
	 
	 int  irq;
	 int  use_irq;

     int snap_detect;
     int snap_detect_counter;
 };
 
/*------------------------------------------------*/
static struct regulator	*kxtf9_accel_reg;
static struct regulator	*hub_gyro_vio_reg;
/*-------------------------------------------------*/
 
static atomic_t		tap_flag;
static atomic_t		shake_flag;
static atomic_t		snap_flag;
static atomic_t		flip_flag;
static atomic_t		poweronoff_flag;
 
/*LGE_CHAGE_S [taehwan.kim@lge.com] 2010-03-31*/
int is_tap_enabled(void)
{
    return atomic_read(&tap_flag);
}
//EXPORT_SYMBOL(is_tap_enabled);
int is_snap_enabled(void)
{
    return atomic_read(&snap_flag);
}
int is_flip_enabled(void)
{
    return atomic_read(&flip_flag);
}

/*LGE_CHAGE_E [taehwan.kim@lge.com] 2010-03-31*/

 /*---------------------------------------------------------------------------
	  kxtf9_power_on/off
	---------------------------------------------------------------------------*/		   
 void kxtf9_power_on(void)
 {
/*	 if(!atomic_read(&poweronoff_flag)){	 	
		 regulator_enable(kxtf9_accel_reg);
		 msleep(1);  	
		 regulator_enable(hub_gyro_vio_reg); 
		 msleep(1);
		 atomic_set(&poweronoff_flag, 1);
	 }*/
 }
 void kxtf9_power_off(void)
 {
/*	 if(atomic_read(&poweronoff_flag)){
	    regulator_disable(hub_gyro_vio_reg); 
	    regulator_disable(kxtf9_accel_reg);
	   
	    atomic_set(&poweronoff_flag, 0);
	 }*/
 }
 /*---------------------------------------------------------------------------
	kxtf9_read_reg_in_burst
   ---------------------------------------------------------------------------*/		
int kxtf9_read_reg_in_burst(struct i2c_client *client, unsigned char reg,unsigned char *buf,int length)
{
	int err;
	unsigned char reg_val = reg;
	
	struct i2c_msg msg[2] = { 
		{ client->addr, 0, 1,&reg_val },
		{ client->addr, I2C_M_RD, length, buf}
	};

	if ((err = i2c_transfer(client->adapter, msg, 2)) < 0) 
	{
		dev_err(&client->dev, "i2c read error\n");
		return -EIO;
	}
	
	return 0;
}

/*---------------------------------------------------------------------------
	kxtf9_write_reg_in_burst
   ---------------------------------------------------------------------------*/		
int kxtf9_write_reg_in_burst(struct i2c_client *client, unsigned char *value,int length)
{
	unsigned char buf[length];   
       int err;		
	struct i2c_msg msg ={
		.addr = client->addr,
		.flags = 0,
		.len   = sizeof(buf),
		.buf   = buf};

	memcpy(buf,value,length);
	
	if ((err = i2c_transfer(client->adapter, &msg, 1)) < 0) 
	{
		dev_err(&client->dev, "i2c write error\n");
		return -EIO;
	}
	
	return 0;
	
}

/*---------------------------------------------------------------------------
	kxtf9_read_reg
   ---------------------------------------------------------------------------*/		
int kxtf9_read_reg(struct i2c_client *client, unsigned char reg, unsigned char *value)
{
	int err;
	unsigned char buf = reg;

	struct i2c_msg msg[2] = { 
		{ client->addr, 0, 1, &buf },
		{ client->addr, I2C_M_RD, 1, value}
	};

	if ((err = i2c_transfer(client->adapter, msg, 2)) < 0) 
	{
		dev_err(&client->dev, "i2c read error\n");
		return -EIO;
	}
	
	return 0;
}

/*---------------------------------------------------------------------------
	kxtf9_write_reg
   ---------------------------------------------------------------------------*/		
int kxtf9_write_reg(struct i2c_client *client, unsigned char *buffer)
{	
	unsigned char buf[2];   
       int err;
	struct i2c_msg msg ={
		.addr = client->addr,
		.flags = 0,
		.len   = 2,
		.buf   = buf};

	buf[0] = buffer[0];
	buf[1] = buffer[1];
	
	if ((err = i2c_transfer(client->adapter, &msg, 1)) < 0) 
	{
	       dev_err(&client->dev, "i2c write error\n");
		return -EIO;
	}
	
	return 0;
	
}

 /*---------------------------------------------------------------------------
	 kxtf9_i2c_read
	---------------------------------------------------------------------------*/		 
 int kxtf9_i2c_read(unsigned char reg,unsigned char *data,int length)
 {
	 int status = 0;
 
	 status = kxtf9_read_reg_in_burst(this_client,reg,data,length);
 
	 return status;
 }
 
 /*---------------------------------------------------------------------------
	 kxtf9_i2c_write - 1 byte register
	---------------------------------------------------------------------------*/		 
 int  kxtf9_i2c_write(unsigned char reg,unsigned char data)
 {
 	 unsigned char buffer[2];
	 int status = 0;
	 buffer[0] = reg;
	 buffer[1] = data;
 
	 status = kxtf9_write_reg(this_client,buffer);
 
	 return status;
 }

 int  kxtf9_i2c_write_burst(unsigned char *buffer,int length)
 {
 	 int status = 0;

	 status = kxtf9_write_reg_in_burst(this_client,buffer,length);
 
	 return status;
 }
 
 /*---------------------------------------------------------------------------
	  kxtf9_accel_enable/disable_irq
	---------------------------------------------------------------------------*/		   
 void kxtf9_enable_irq(void)
 {
  	struct kxtf9_data *data = i2c_get_clientdata(this_client);
  
	 if(data->use_irq == 0)
	 {
		 printk("[kxtf9_enable_irq] data->use_irq == 0\n");
		 enable_irq(data->irq);
		 data->use_irq = 1;
	 }
 }
 void kxtf9_disable_irq(void)
 {
 	struct kxtf9_data *data = i2c_get_clientdata(this_client);
 
	 if(data->use_irq == 1)
	 {
		 printk("[kxtf9_disable_irq] hub_motion_dev.use_irq  == 1\n");
		 disable_irq_nosync(data->irq); //antispoon0630
		 data->use_irq	= 0;
	 }
 }
 /*---------------------------------------------------------------------------
	  kxtf9 send event function
	---------------------------------------------------------------------------*/	  
 void kxtf9_send_tap_detection(int type,int direction)
 {
 	struct kxtf9_data *data = i2c_get_clientdata(this_client);
 
	 if(atomic_read(&tap_flag))
	 {
		 input_report_rel(data->input_dev,REL_X,type); 
		 input_report_rel(data->input_dev,REL_Y,direction); 
		 
		 input_sync(data->input_dev);
	 }
 }

 void kxtf9_send_shake_detection(int value)
 {
 	struct kxtf9_data *data = i2c_get_clientdata(this_client);

	if(atomic_read(&shake_flag))
	{
		input_report_rel(data->input_dev,REL_HWHEEL,value); 
		input_sync(data->input_dev);
	}
 }
 void kxtf9_send_snap_detection(int value)
 {
 	struct kxtf9_data *data = i2c_get_clientdata(this_client);
 
	 if(atomic_read(&snap_flag))
	 {
#if NEW_GESTURE
         if (data->snap_detect == 0) 
         {
             data->snap_detect = value;      
             data->snap_detect_counter = 0;
             hrtimer_start(&data->timer,ktime_set(0,200000000),HRTIMER_MODE_REL);
         }
#else
         input_report_rel(data->input_dev,REL_DIAL,value); 	
		 input_sync(data->input_dev);
#endif
	 }
 }
 
 void kxtf9_send_flip_detection(int value)
 {
 	struct kxtf9_data *data = i2c_get_clientdata(this_client);
	
	 if(atomic_read(&flip_flag))
	 { 
		 input_report_rel(data->input_dev,REL_WHEEL,value); 
		 input_sync(data->input_dev);
	 }
 }
 
 /*---------------------------------------------------------------------------
	  kxtf9 enable gesture type
---------------------------------------------------------------------------*/	
 int kxtf9_enable_gesture_status(void)
 {
 	 int status = 0;

	 if(atomic_read(&tap_flag))
     {
		status |= KXTF9_TAPPING_ENABLE;
	 }
	 
	 if(atomic_read(&shake_flag))
	 {
		status |= KXTF9_SHAKE_ENABLE;
	 }

	 if(atomic_read(&flip_flag))
	 {
	 	status |= KXTF9_FLIP_ENABLE;
	 }

	 if(atomic_read(&snap_flag))
	 {
	 	status |= KXTF9_SNAP_ENABLE;
	 }

	 return status;
	 
 }
 
 /*---------------------------------------------------------------------------
	  kxtf9_accel_driver_init
	---------------------------------------------------------------------------*/		   
 void kxtf9_accel_driver_init(void)
 {
	 static int accel_callback_register_flag = 0;
 
	 if(accel_callback_register_flag == 0)
	 {
		 printk("[motion_accel_init] accel_callback_register_flag \n");
		 kxtf9_accel_tap_register_callback(kxtf9_send_tap_detection);
		 kxtf9_accel_shake_register_callback(kxtf9_send_shake_detection);
		 kxtf9_accel_flip_register_callback(kxtf9_send_flip_detection);
		 kxtf9_accel_snap_register_callback(kxtf9_send_snap_detection);
 
		 accel_callback_register_flag = 1;
	 }
	 
	 //kxtf9_accel_init();	 
	 
 }
 
 /*---------------------------------------------------------------------------
	  work function
	---------------------------------------------------------------------------*/		   
static void kxtf9_shake_work_func(struct work_struct *work)						  
{	
	kxtf9_accel_detect_shake(); 	
}
static void kxtf9_isr_work_func(struct work_struct *work) 						
{
	 struct kxtf9_data *data = i2c_get_clientdata(this_client);

	 kxtf9_accel_service_isr();
#if DEBUG     
     printk("kxtf9 enable_irq \n");
#endif
	 enable_irq(data->irq); 
}
static void kxtf9_snap_work_func(struct work_struct *work)
{
    struct kxtf9_data *data = i2c_get_clientdata(this_client);
    int res;
    char snap_pos_cur, snap_pos_pre;
   
#if DEBUG     
    printk("kxtf9_isr_work_func \n");
#endif
    data->snap_detect_counter++;
    if (data->snap_detect_counter < 4)
    {
#if DEBUG
        printk("kxtf9_snap_timer_func snap_detect_counter = %d \n",data->snap_detect_counter);
#endif
        if(data->snap_detect!=0)   
        {
            KIONIX_ACCEL_read_bytes(KIONIX_ACCEL_I2C_TILT_POS_CUR, &snap_pos_cur, 1);
#if DEBUG
           printk("kxtf9_snap_timer_func cur_position = %x res = %d \n",snap_pos_cur,res);
#endif
            if ( (snap_pos_cur == 0x01)
                    ||((snap_pos_cur == 0x04)&&((data->snap_detect==1)||(data->snap_detect==4)))
                    ||((snap_pos_cur == 0x20)&&((data->snap_detect==2)||(data->snap_detect==3))) )
            {
                printk("===[[SNAP_EVENT!!]] = [%x] ====\n",data->snap_detect);
                input_report_rel(data->input_dev,REL_DIAL,data->snap_detect);
                input_sync(data->input_dev);
                data->snap_detect_counter = 0;
                data->snap_detect = 0;
                hrtimer_cancel(&data->timer);
            }
            else if ( ( (data->snap_detect == 1)&&(snap_pos_cur == 0x10) )
                    ||( (data->snap_detect == 2)&&(snap_pos_cur == 0x08) )
                    ||( (data->snap_detect == 3)&&(snap_pos_cur == 0x04) )
                    ||( (data->snap_detect == 4)&&(snap_pos_cur == 0x20) ))
            {
#if DEBUG
                printk("hrtimer_start \n");
#endif
                hrtimer_start(&data->timer,ktime_set(0,200000000),HRTIMER_MODE_REL);  // 200 ms  
            }
            else
            {
#if DEBUG
                printk("cancel_snap_detect \n");                    
#endif
                data->snap_detect_counter = 0;
                data->snap_detect = 0;
                hrtimer_cancel(&data->timer);
            }        
        }
        else
        {
            data->snap_detect_counter = 0;               
            data->snap_detect = 0;                                
            hrtimer_cancel(&data->timer);
        }
    }
    else 
    {
        data->snap_detect_counter = 0;                            
        data->snap_detect = 0;
        hrtimer_cancel(&data->timer);
    }
}
/*---------------------------------------------------------------------------
	  kxtf9 interrupt handler 
---------------------------------------------------------------------------*/		   
 static irqreturn_t kxtf9_int_handler(int irq, void *dev_id)						   
 {
 	struct kxtf9_data *data = dev_id;
#if DEBUG
    printk("kxtf9 interrupt occur! ");
#endif
	 disable_irq_nosync(data->irq);
	 queue_work(data->isr_wq,&data->isr_work);
		 
	 return IRQ_HANDLED;												   
 }	
/*---------------------------------------------------------------------------
	  kxtf9 polling timer
	---------------------------------------------------------------------------*/		   
#if NEW_GESTURE
static enum hrtimer_restart kxtf9_snap_timer_func(struct hrtimer *timer)
 {
 	 struct kxtf9_data *data = container_of(timer, struct kxtf9_data, timer);
#if DEBUG
    printk("kxtf9_snap_timer_func snap_evet = %x \n",data->snap_detect);
#endif
    if(atomic_read(&snap_flag))
    {
        queue_work(data->snap_wq, &data->snap_work);
    }
    
    return HRTIMER_NORESTART;
 }

#else
static enum hrtimer_restart kxtf9_shake_timer_func(struct hrtimer *timer)
 {
 	 struct kxtf9_data *data = container_of(timer, struct kxtf9_data, timer);

	 if(atomic_read(&shake_flag))   
	 { 
		 queue_work(data->timer_wq, &data->shake_work);
		 hrtimer_start(&data->timer,ktime_set(0,20000000),HRTIMER_MODE_REL);  // 20 ms  
	 }	
	return HRTIMER_NORESTART;
 }
#endif
 
 /*---------------------------------------------------------------------------
	  sensor enable/disable (Sensor HAL)
	---------------------------------------------------------------------------*/	  
 static ssize_t kxtf9_tap_onoff_store(struct device *dev,struct device_attribute *attr,const char *buf,size_t count)
 {
	 u32	val;
     char test[3]; 
#if DEBUG
     kxtf9_i2c_read(0x1B,&test[0],1);
     kxtf9_i2c_read(0x1C,&test[1],1);
     kxtf9_i2c_read(0x1D,&test[2],1);
	 printk("REG 0x1B = %x, 0x1C = %x, 0x1D = %x \n", test[0],test[1],test[2]);
#endif
	 val = simple_strtoul(buf, NULL, 10);
    
     if (val>0) tap_onoff_count++;
     else tap_onoff_count--;
#if DEBUG
     printk("enalbe tap register %d > current tap_onoff_count is %d\n",val,tap_onoff_count);
#endif
     if (tap_onoff_count <0) { 
         printk("WANING! Broken balanced tap onoff register listener ! \n");
        tap_onoff_count = 0;
     }
	 printk("[motion_set_tap_onoff_store] tap.... flag [%d]\n",val);
 
	 if(tap_onoff_count>0)
	 {
#if 0//magoo	 
         kxtf9_accel_enable_tap();
		 kxtf9_enable_irq();
#endif		 
		 atomic_set(&tap_flag, 1);	 

	 }
	 else
	 {	 
#if 0//magoo	 
		 if((!atomic_read(&flip_flag)) && (!atomic_read(&snap_flag)))
		 {	 
             kxtf9_accel_disable_tap();
			 kxtf9_disable_irq();
			 printk("[motion_tap_onoff_store] flip_flag == 0 , snap_flag == 0\n");
		 }
#endif
 
		 atomic_set(&tap_flag, 0);
	 }
 
	 return count;
	 
 }
 
 static ssize_t kxtf9_shake_onoff_store(struct device *dev,struct device_attribute *attr,const char *buf,size_t count)
 {
 	struct kxtf9_data *data = i2c_get_clientdata(this_client);
	u32	val;
	char test[3]; 
#if DEBUG
    kxtf9_i2c_read(0x1B,&test[0],1);
    kxtf9_i2c_read(0x1C,&test[1],1);
    kxtf9_i2c_read(0x1D,&test[2],1);
    printk("REG 0x1B = %x, 0x1C = %x, 0x1D = %x \n", test[0],test[1],test[2]);
#endif
    val = simple_strtoul(buf, NULL, 10);
 
	 printk("[motion_set_shake_onoff_store]  flag [%d]\n",val);
#if NEW_GESTURE 
     printk("shake function is not supported \n");
#else
	 if(val)
	 {
		 atomic_set(&shake_flag, 1); 
		 
		 hrtimer_cancel(&data->timer);
		 kxtf9_accel_enable_shake(); 
		 hrtimer_start(&data->timer, ktime_set(0,2000000000), HRTIMER_MODE_REL);	// 2000 ms 
	 }
	 else
	 {
		 hrtimer_cancel(&data->timer);
		 atomic_set(&shake_flag, 0);
	 }
#endif
	 return count;
	 
 }

  static ssize_t kxtf9_flip_onoff_store(struct device *dev,struct device_attribute *attr,const char *buf,size_t count)
 {
	 u32	val;
     char test[3];
#if DEBUG
     kxtf9_i2c_read(0x1B,&test[0],1);
     kxtf9_i2c_read(0x1C,&test[1],1);
     kxtf9_i2c_read(0x1D,&test[2],1);
     printk("REG 0x1B = %x, 0x1C = %x, 0x1D = %x \n", test[0],test[1],test[2]);
#endif
     val = simple_strtoul(buf, NULL, 10);
     if (val>0) flip_onoff_count++;
     else flip_onoff_count--;
#if 1//DEBUG
     printk("enalbe tap register %d > current flip_onoff_count is %d\n",val,tap_onoff_count);
#endif
     if (flip_onoff_count <0) printk("WANING! Broken balanced flip onoff register listener ! \n");
	 printk("[motion_set_flip_onoff_store]	flag [%d]\n",val);
 
	 if(flip_onoff_count)
	 {
#if 0 //magoo
         KIONIX_ACCEL_position_mask_fu(); 
         KIONIX_ACCEL_position_mask_fd();   // snap  --  snap_pitch_up
         
         KIONIX_ACCEL_enable_tilt_function(); //antispoon 0326

		 atomic_set(&flip_flag, 1);
		 kxtf9_enable_irq();
#else
		atomic_set(&flip_flag, 1);
#endif
	 }
	 else
	 {
#if 0 //magoo	
		 if((!atomic_read(&tap_flag)) && (!atomic_read(&snap_flag)))
		 {

             //KIONIX_ACCEL_position_unmask_fu(); 
             KIONIX_ACCEL_position_unmask_fd();   // snap  --  snap_pitch_up
             
             KIONIX_ACCEL_disable_tilt_function(); //antispoon 0326

			 printk("[motion_tap_onoff_store] flip_flag == 0 , snap_flag == 0\n");
			 kxtf9_disable_irq();
		 
		 }
#endif	
		 
		 atomic_set(&flip_flag, 0);  
 
	 }
 
	 return count;
	 
 }
  
 static ssize_t kxtf9_snap_onoff_store(struct device *dev,struct device_attribute *attr,const char *buf, size_t count)
 {
 	 u32  val;
     char test[3]; 
#if DEBUG
     kxtf9_i2c_read(0x1B,&test[0],1);
     kxtf9_i2c_read(0x1C,&test[1],1);
     kxtf9_i2c_read(0x1D,&test[2],1);
     printk("REG 0x1B = %x, 0x1C = %x, 0x1D = %x \n", test[0],test[1],test[2]);
#endif
	 val = simple_strtoul(buf, NULL, 10);
     if (val>0) snap_onoff_count++;
     else snap_onoff_count--;
#if DEBUG
     printk("enalbe snap register %d > current snap_onoff_count is %d\n",val,tap_onoff_count);
#endif
     if (snap_onoff_count <0) printk("WANING! Broken balanced snap onoff register listener ! \n");
	 printk("[motion_set_snap_onoff_store]	flag [%d]\n",val);
 
	 if(snap_onoff_count>0)
	 {
         KIONIX_ACCEL_position_mask_fu(); /*LGE_CHAGE [taehwan.kim@lge.com]*/
         KIONIX_ACCEL_position_mask_ri();
         KIONIX_ACCEL_position_mask_le();
         KIONIX_ACCEL_position_mask_up();   // flip
         KIONIX_ACCEL_position_mask_do();   // flip --- snap_roll_right

         KIONIX_ACCEL_enable_tilt_function();
         atomic_set(&snap_flag, 1);  
		 kxtf9_enable_irq();
	 }
	 else
	 {
		 if((!atomic_read(&tap_flag)) && (!atomic_read(&flip_flag)))
		 {
             KIONIX_ACCEL_position_unmask_ri();
             KIONIX_ACCEL_position_unmask_le();
             KIONIX_ACCEL_position_unmask_up();   // flip
             KIONIX_ACCEL_position_unmask_do();   // flip --- snap_roll_right

             KIONIX_ACCEL_disable_tilt_function();
             printk("[motion_tap_onoff_store] flip_flag == 0 , flip_flag == 0\n");
			 kxtf9_disable_irq();
		 }
		 
		 atomic_set(&snap_flag, 0);		 
	 }
 
	 return count;
	 
 }
#if DEBUG
static ssize_t kxindex_store(struct device *dev,struct device_attribute *attr,const char *buf, size_t count)
{
    char val[2],reg_addr;
    int ret;
    
    kxtf_reg_index = simple_strtoul(buf, NULL, 10);
    reg_addr = kxtf_reg_index;
    ret = kxtf9_i2c_read(kxtf_reg_index,&val[0],1);
    printk("kxtf read reg[%x] = %x \n",kxtf_reg_index,val[0]);

    return count;
}
static ssize_t kxvalue_store(struct device *dev,struct device_attribute *attr,const char *buf, size_t count)
{
    char val[2],reg_addr,reg_val;
    int ret;

    kxtf_reg_value = simple_strtoul(buf, NULL, 10);
    reg_addr = kxtf_reg_index;
    reg_val = kxtf_reg_value;
    //printk("num %x, val %x %d \n",reg_addr,reg_val,reg_val);
    ret = kxtf9_i2c_write(reg_addr,reg_val);
    printk("kxtf write reg[%x] = %x \n",kxtf_reg_index,reg_val);

    return count;
}

#endif

 static DEVICE_ATTR(tap_onoff,0666,NULL,kxtf9_tap_onoff_store);
 static DEVICE_ATTR(shake_onoff,0666,NULL,kxtf9_shake_onoff_store);
 static DEVICE_ATTR(flip_onoff,0666,NULL,kxtf9_flip_onoff_store);
 static DEVICE_ATTR(snap_onoff,0666,NULL,kxtf9_snap_onoff_store);
#if DEBUG
 static DEVICE_ATTR(num,0666,NULL,kxindex_store);
 static DEVICE_ATTR(value,0666,NULL,kxvalue_store);
#endif
 
 static struct attribute* kxtf9_attributes[] = {
	 &dev_attr_tap_onoff.attr,
	 &dev_attr_shake_onoff.attr,
	 &dev_attr_flip_onoff.attr,  
	 &dev_attr_snap_onoff.attr,
#if DEBUG
	 &dev_attr_num.attr,
	 &dev_attr_value.attr,
#endif
	 NULL
 };
 
 static const struct attribute_group kxtf9_group = {
	 .attrs = kxtf9_attributes,
 };
 
 /*---------------------------------------------------------------------------
	  ioctl command API
   ---------------------------------------------------------------------------*/	 
 static int kxtf9_open(struct inode *inode, struct file *file)
 {	   
	   int status = 0;
		
	   return status;
 }
 static int kxtf9_release(struct inode *inode, struct file *file)
 {
	 printk("kxtf9_release\n");
 	 
	 return 0;
 }
 static int kxtf9_ioctl(struct inode *inode, struct file *file, unsigned int cmd,unsigned long arg)
 {
	 void __user *argp = (void __user *)arg;
	 int  data[3],data_temp = 0;
	 int  sample_index = 0, sensor_power_onoff = KXTF9_ACCEL_POWER_OFF;
	 int  resolution = 0, g_range = 0;
	 unsigned char data_raw[10];
	 
	 switch (cmd) 
	 {
	 case KXTF9_IOCTL_SET_POWER_ONOFF:
		 if (copy_from_user(&sensor_power_onoff, argp, sizeof(sensor_power_onoff))){
			 return -EFAULT;
		 }
		 
		 printk("ACCEL_IOCTL_SET_POWER_ONOFF: [%d]\n",sensor_power_onoff);
 
		 if(sensor_power_onoff == KXTF9_ACCEL_POWER_OFF){
			 kxtf9_power_off();
		 }
		 else if(sensor_power_onoff == KXTF9_ACCEL_POWER_ON){		 
			 kxtf9_power_on();
		 }
		 else{
			 printk("Not supported power on/off type.......\n");
		 }
		 break;
	 case KXTF9_IOCTL_GET_POWER_ONOFF:	 
		  sensor_power_onoff = atomic_read(&poweronoff_flag);
		  if (copy_to_user(argp, &sensor_power_onoff, sizeof(sensor_power_onoff)))
		  {
			  return -EFAULT;
		  }
		 break;
	case KXTF9_IOCTL_GET_ACCEL_RAW:
		 kxtf9_accel_read_raw_data(&data_raw[0]);

		 if (copy_to_user(argp, &data_raw, sizeof(data_raw)))
		 {
		 	 return -EFAULT;
		 }
		 break;
	 case KXTF9_IOCTL_GET_DATA :
		  kxtf9_accel_api_read_data(&data[0]);
		  
		  /* swap x,y-axis and change a z-axis sign in hub model */
		  data_temp = data[0];
		  data[0]= data[1];
		  data[1]= data_temp;
		  data[2]*=(-1);
		  
		  if (copy_to_user(argp, &data, sizeof(data)))
		  {
			  return -EFAULT;
		  }
		  break;
 
	  /*	KXTF9 output data rate
		*
		*	  0 : 12.5 Hz			1 :  25 Hz
		*	  2:	  50 Hz 		  3: 100 Hz
		  * 	4:	  200 Hz		   5: 400 Hz
		  * 	6:	  800 Hz
		*/	 
	 case KXTF9_IOCTL_SET_SAMPLE_RATE:
		 if (copy_from_user(&sample_index, argp, sizeof(sample_index)))
		 {
			 return -EFAULT;
		 }
 
		 kxtf9_accel_api_set_sample_rate(sample_index);
		 printk("ACCEL_IOCTL_SET_SAMPLE_RATE : index [%d]\n",sample_index);
		 
		 break;
	 case KXTF9_IOCTL_GET_SAMPLE_RATE:
		 kxtf9_accel_api_get_sample_rate(&sample_index);
		 printk("ACCEL_IOCTL_GET_SAMPLE_RATE--- current sample rate[%d]\n",sample_index);
		 
		 if (copy_to_user(argp, &sample_index, sizeof(sample_index)))
		  {
			  return -EFAULT;
		  }
		 break;
	 case KXTF9_IOCTL_GET_G_RANGE:
		  kxtf9_accel_api_get_g_range(&g_range);
		  if (copy_to_user(argp, &g_range, sizeof(g_range)))
		  {
			  return -EFAULT;
		  }
		  break;
	 case KXTF9_IOCTL_GET_RESOLUTION:	 
		  kxtf9_accel_api_get_resolution(&resolution);
		  if (copy_to_user(argp, &resolution, sizeof(resolution)))
		  {
			  return -EFAULT;
		  }
		  break; 
	 }
 
	 return 0;
	 
 }
  
 static struct file_operations	kxtf9_fops = {
	 .owner    = THIS_MODULE,
	 .open	   = kxtf9_open,
	 .release  = kxtf9_release,
	 .ioctl    = kxtf9_ioctl,
 };
 
 static struct miscdevice  kxtf9_misc_device = {
	 .minor    = MISC_DYNAMIC_MINOR,
	 .name	   = KXTF9_IOCTL_NAME,
	 .fops	   = &kxtf9_fops,
 };

 /*---------------------------------------------------------------------------
	  i2c driver
	---------------------------------------------------------------------------*/		   
 static int __init kxtf9_probe(struct i2c_client *client, const struct i2c_device_id *id)
 {
	struct kxtf9_data *kxtf9 = NULL;	
	struct device *dev = &client->dev;
	int ret = 0;

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		  ret = -ENOTSUPP;
		  goto probe_failure;
	}

	kxtf9 = kzalloc(sizeof(struct kxtf9_data), GFP_KERNEL);
	if (kxtf9 == NULL) {
		ret = -ENOMEM;
		goto exit_alloc_data_failed;
	}
	
	i2c_set_clientdata(client,kxtf9);
	this_client = client;
	   
    /* i2c interface kionix_accel_api*/
	kxtf9_accel_register_i2c_func();
	 /*---------------------------------------------------------------------------
		register misc device
	  ---------------------------------------------------------------------------*/   
	 ret = misc_register(&kxtf9_misc_device);
	 if (ret) {
		 printk(KERN_ERR"hub_motion_misc_device register failed\n");
		 goto exit_misc_device_register_failed;
	 }  

	 /*---------------------------------------------------------------------------
		register input device
	  ---------------------------------------------------------------------------*/	  
	 kxtf9->input_dev = input_allocate_device();
	 if(kxtf9->input_dev == NULL)
	 {
		 printk(KERN_ERR"hub_motion_sesnor_probe: input_allocate_device (1) failed\n");
		 goto err_input_allocate1; 	 
	 }  
 
	 kxtf9->input_dev->name = KXTF9_GESTURE_INPUT_NAME;

	#if 1
	 set_bit(EV_SYN,kxtf9->input_dev->evbit);	   
	 set_bit(EV_REL,kxtf9->input_dev->evbit);  


	 set_bit(REL_HWHEEL,kxtf9->input_dev->relbit); // SHAKE
	 set_bit(REL_DIAL,kxtf9->input_dev->relbit);   // SNAP 

	#else
	 set_bit(REL_X,kxtf9->input_dev->relbit);  // TAP - Type 
	 set_bit(REL_Y,kxtf9->input_dev->relbit);  // TAP - DIRECTION
	 set_bit(REL_WHEEL,kxtf9->input_dev->relbit);  // FLIP	 
	#endif 
	   
	 ret = input_register_device(kxtf9->input_dev);
	 if(ret){
		 printk(KERN_ERR"kxtf9: input_allocate_device (1) failed \n");
		 goto err_input_allocate1; 	 
	 }
	 /*---------------------------------------------------------------------------
		 init. sysfs
	 ---------------------------------------------------------------------------*/	 
	 if ((ret = sysfs_create_group(&dev->kobj,&kxtf9_group)))
	 {
		 printk("[motion_sensor] sysfs_create_group FAIL \n");
		 goto err_sysfs_create;
	 }
	 /*---------------------------------------------------------------------------
		 INIT_WORK 
	 ---------------------------------------------------------------------------*/		   
	 INIT_WORK(&kxtf9->shake_work,kxtf9_shake_work_func);
	 INIT_WORK(&kxtf9->isr_work,kxtf9_isr_work_func);
     INIT_WORK(&kxtf9->snap_work,kxtf9_snap_work_func); 
	 /*---------------------------------------------------------------------------
		 init. workqueue 
	 ---------------------------------------------------------------------------*/		   
			 
	 kxtf9->timer_wq = create_singlethread_workqueue("kxtf9_timer_wq");
	 if (!kxtf9->timer_wq) {
		 printk("[motion_sensor] couldn't create timer queue\n");
		 goto err_accel_timer_wq;
	 }
 
	 kxtf9->isr_wq = create_singlethread_workqueue("kxtf9_isr_wq");
	 if (!kxtf9->isr_wq) {
		 printk("[motion_sensor] couldn't create ISR queue\n");
		 goto err_accel_isr_wq;
	 }
 
     kxtf9->snap_wq = create_singlethread_workqueue("kxtf9_snap_wq");
     if (!kxtf9->isr_wq) {
         printk("[motion_sensor] couldn't create snap queue\n");
         goto err_accel_snap_wq;
     }
	/*---------------------------------------------------------------------------
		 init. timer
	 ---------------------------------------------------------------------------*/	 
	 // Shake POLLING TIMER	  
	 hrtimer_init(&kxtf9->timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
#if NEW_GESTURE
	 kxtf9->timer.function = kxtf9_snap_timer_func;
#else
	 kxtf9->timer.function = kxtf9_shake_timer_func;
#endif

	 /*---------------------------------------------------------------------------
		init. IRQ  
       ---------------------------------------------------------------------------*/   
     if (gpio_request(ACCEL_INT_GPIO,KXTF9_DRIVER_NAME) < 0) {
		 printk("[ERROR!!] hub_ACCEL_INT_GPIO\n");		 
		 goto err_request_irq;
	 }
		 
	 ret = gpio_direction_input(ACCEL_INT_GPIO);
	 if(ret <0)
	 {
		 printk("[ERROR!!] gpio_direction_input\n");
		 return -1;
	 }
 
	 kxtf9->irq = OMAP_GPIO_IRQ(ACCEL_INT_GPIO);

	 printk("[kxtf9_probe] OMAP_GPIO_IRQ : kxtf9->irq[%d]\n",kxtf9->irq);
			
	 ret = request_irq(kxtf9->irq,kxtf9_int_handler,IRQF_TRIGGER_RISING,KXTF9_DRIVER_NAME, kxtf9);   
	 if(ret <0)
	 {
		 printk("[ERROR!!] request_irq\n");
		 return -1;
	 }
	 
	 disable_irq_nosync(kxtf9->irq);
	 kxtf9->use_irq	= 0;
 
	/*---------------------------------------------------------------------------
		power 
	  ---------------------------------------------------------------------------*/  
	  hub_gyro_vio_reg = regulator_get(dev, "vdac");
	  if (hub_gyro_vio_reg == NULL) {
			 printk(KERN_ERR": Failed to get motion power resources !! \n");
			 return -ENODEV;
	  }

 
	  kxtf9_accel_reg = regulator_get(dev, "vaux2");
	   if (kxtf9_accel_reg == NULL) {
			 printk(KERN_ERR": Failed to get motion power resources !! \n");
			 return -ENODEV;
	   } 
	   
	   kxtf9_power_on();
       kxtf9->snap_detect = 0; //initialize snap detection flag
	   kxtf9_accel_driver_init();
#if NO_COMPASS_SUPPORT       
       atomic_set(&snap_flag, 1); 
       //kxtf9_enable_irq();
#endif	   

       return 0; 

err_input_allocate1:	
   input_unregister_device(kxtf9->input_dev);	
err_request_irq:
   gpio_free(ACCEL_INT_GPIO);
err_accel_timer_wq:
   destroy_workqueue(kxtf9->timer_wq);
err_accel_isr_wq:
   destroy_workqueue(kxtf9->isr_wq);
err_accel_snap_wq:
      destroy_workqueue(kxtf9->snap_wq);
probe_failure:  
err_sysfs_create:   
exit_misc_device_register_failed:
exit_alloc_data_failed:
   kfree(kxtf9);
   return ret; 
 
 }
 static int kxtf9_remove(struct i2c_client *client)
 {
 	struct kxtf9_data *data = i2c_get_clientdata(client);
 
 	input_unregister_device(data->input_dev);
	 
	if (data->timer_wq)
		 destroy_workqueue(data->timer_wq);
	if (data->isr_wq)
		 destroy_workqueue(data->isr_wq);
    if (data->isr_wq)
        destroy_workqueue(data->snap_wq);
	i2c_set_clientdata(client, NULL);
	kfree(data);

	gpio_free(ACCEL_INT_GPIO);

	return 0;
}
 
static int kxtf9_suspend(struct i2c_client *client, pm_message_t mesg)
{
	return 0;
}

static int kxtf9_resume(struct i2c_client *client)
{
	return 0;
}

 static const struct i2c_device_id kxtf9_id[] = {
	 { KXTF9_DRIVER_NAME, 0 },
	 { }
 };

 static struct i2c_driver kxtf9_driver = {
	 .probe    = kxtf9_probe,
	 .remove   = kxtf9_remove,
	 .suspend  = kxtf9_suspend,
	 .resume   = kxtf9_resume,
	 .id_table = kxtf9_id,
	 .driver   =	{
		.name  = KXTF9_DRIVER_NAME,
	  },
 };

 static int __init kxtf9_init(void)
 {	  
 	return i2c_add_driver(&kxtf9_driver);
 }
 static void __exit kxtf9_exit(void)
 {
	i2c_del_driver(&kxtf9_driver); 
 }

 module_init(kxtf9_init);
 module_exit(kxtf9_exit);
 
 MODULE_AUTHOR("LG Electronics");
 MODULE_DESCRIPTION("KXTF9 Acceleromter driver");
 MODULE_LICENSE("GPL");
 
