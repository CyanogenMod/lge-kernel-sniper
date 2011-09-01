/*
 *  mpu3050.c
 *
 *  MPU3050 Gyroscope Sensor Driver  
 * 
 * Copyright (C) 2009 LGE Inc.
 *
 *    chpark96@lge.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
 
#include <linux/i2c.h>
//#include <mach/resource.h>
#include <mach/hardware.h>
#include <asm/uaccess.h>
#include <linux/device.h>

 #include "mpu3050.h"
// #include "kr3dh.h"


 
struct i2c_client	*mpu3050_i2c_client;


void  mpu3050_reg_i2c_client(struct i2c_client *client)
{
	mpu3050_i2c_client = client;	
}
int mpu3050_read_reg_in_burst(struct i2c_client *client, unsigned char reg,unsigned char *buf,int length)
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
int mpu3050_write_reg_in_burst(struct i2c_client *client, unsigned char *buffer,int length)
{
	int err;	 
#if 1
	struct i2c_msg msg ={
		.addr = client->addr,
		.flags = 0,
		.len	= length,
		.buf	= buffer};
#else
	unsigned char buf[length];   
	struct i2c_msg msg ={
		.addr = client->addr,
		.flags = 0,
		.len	= sizeof(buf),
		.buf	= buf};
		 
	memcpy(buf,buffer,length);
#endif		 
	if ((err = i2c_transfer(client->adapter, &msg, 1)) < 0) 
	{
		dev_err(&client->dev, "i2c write error\n");
		return -EIO;
	}
			 
	return 0;
	 
}
int mpu3050_read_reg(struct i2c_client *client, unsigned char reg, unsigned char *val)
{
	int err;
	unsigned char buf = reg;
	 
	struct i2c_msg msg[2] = { 
		{ client->addr, 0, 1, &buf },
		{ client->addr, I2C_M_RD, 1, val}
	};
	 
	if ((err = i2c_transfer(client->adapter, msg, 2)) < 0) 
	{
		dev_err(&client->dev, "i2c read error\n");
		return -1;
	}
		 
	return 0;
}

 int mpu3050_write_reg(struct i2c_client *client, unsigned char *buffer)
 {
	unsigned char buf[2];	 
	int err;
	
	struct i2c_msg msg ={
			 .addr = client->addr,
			 .flags = 0,
			 .len	= 2,
			 .buf	= buf};
	 
	buf[0] = buffer[0];
	buf[1] = buffer[1];
		 
	if ((err = i2c_transfer(client->adapter, &msg, 1)) < 0) 
	{
		dev_err(&client->dev, "i2c write error\n");
	}
		 
	return 0;	 
		 
}

void mpu3050_sleep_mode(void)
{
	unsigned char value = 0;
	unsigned char buf[5] = {0,};

	 mpu3050_read_reg(mpu3050_i2c_client,MPU3050_GYRO_I2C_PWR_MGM,&value);

         /*
		Bit 6   SLEEP        
		Bit 5   STBY_XG    
		Bit 4   STBY_YG
		Bit 3   STBY_ZG
	   */

	 if(!(value & 0x78))
	 {
	 	value|= 0x78;
		
		buf[0] = MPU3050_GYRO_I2C_PWR_MGM;
		buf[1] = value;    
		
		mpu3050_write_reg(mpu3050_i2c_client,buf);
		
	 }
}
void mpu3050_sleep_wake_up(void)
{
	unsigned char value = 0;
	unsigned char buf[5] = {0,};
	
	mpu3050_read_reg(mpu3050_i2c_client,MPU3050_GYRO_I2C_PWR_MGM,&value);
	
	if(value & 0x78)
	{
	     value&= ~0x78;

	    buf[0] = MPU3050_GYRO_I2C_PWR_MGM;
	    buf[1] = value;	
	
	    mpu3050_write_reg(mpu3050_i2c_client,buf);	 
	}
}
void mpu3050_i2c_through_pass(int benable)
{
	unsigned char value;
	unsigned char buf[3]={0,};
	int status = 0;
		
	value = 0;
       status = mpu3050_read_reg(mpu3050_i2c_client,MPU3050_GYRO_I2C_USER_CTRL,&value);    
	if(status <0)
	{
	     printk("[mpu3050] MPU3050_GYRO_I2C_USER_CTRL. i2c ERROR: 0x%x................................\n",value);
      	     return;
	}

	printk("[mpu3050].........................-------------------------------------------------------------------------------.......\n");
      	     	
      	if(benable ==MPU3050_BYPASS_MODE_ON)
      	{
      	     if(value & 0x20)
      	           value&=~(0x20);	
      	}
      	else  // bypass off
      	{
      		printk("[mpu3050] bypass off.....................................\n");
      	      if(!(value & 0x20))
      	           value|= 0x20;      
      	}
      	
      	if(!(value & 0x08))
      	       value|=0x08;	
      	       
      	buf[0] = MPU3050_GYRO_I2C_USER_CTRL;
      	buf[1] = value;
      	status = mpu3050_write_reg(mpu3050_i2c_client,buf);	
      	
}

void mpu3050_initialize(void)
{
	unsigned char buf[3]={0,};
	unsigned char value = 0;
	int status = 0;

	//  WHO AM I  
	 value = 0;
	 status = mpu3050_read_reg(mpu3050_i2c_client,MPU3050_GYRO_I2C_WHO_AM_I,&value);
	printk("[MPU3050] MPU3050_GYRO_I2C_WHO_AM_I : %x\n",value);
	
	// Read Product ID
	value = 0;
	status = mpu3050_read_reg(mpu3050_i2c_client,MPU3050_GYRO_I2C_PRODUCT_ID,&value); 
	printk("[MPU3050] MPU3050_GYRO_I2C_PRODUCT_ID : %x\n",value);

      	//  Reset the MPU-3000
      	buf[0] = MPU3050_GYRO_I2C_PWR_MGM;
       buf[1] = 0x80;
      	status = mpu3050_write_reg(mpu3050_i2c_client,buf);
	printk(" [MPU3050]Reset the MPU-3000 : %x\n",value);
 
   	// Set ACCEL. slave address   
      	buf[0] = MPU3050_GYRO_I2C_ACCEL_SLAVE_ADDR;
      	buf[1] = 0x0F;//KR3DH_ACCEL_I2C_SLAVE_ADDR;
      	status = mpu3050_write_reg(mpu3050_i2c_client,buf);
	printk("[MPU3050] Set ACCEL. slave address \n");	
       
      	// Reset IME interface
      	value = 0;
      	status = mpu3050_read_reg(mpu3050_i2c_client,MPU3050_GYRO_I2C_USER_CTRL,&value);
      	
      	if(!(value & 0x08))
      	      value |= 0x08;
      	buf[0] = MPU3050_GYRO_I2C_USER_CTRL;
      	buf[1] = value;
      	status = mpu3050_write_reg(mpu3050_i2c_client,buf);
	printk("[MPU3050] Reset IME interface \n");	
			   
	  #if 0
        //klp  -- disable passthrough mode
       value = 0;
      	status = mpu3050_read_reg(mpu3050_i2c_client,MPU3050_GYRO_I2C_USER_CTRL,&value);
       buf[0] = MPU3050_GYRO_I2C_USER_CTRL;
       buf[1] = value | 0x20;
       mpu3050_write_reg(mpu3050_i2c_client,buf); 
       #endif
		
	// Disable ISR
      	buf[0] = MPU3050_GYRO_I2C_INT_CFG;
      	buf[1] = 0x00;//0x83;    //0xC1;//0xE1;   //0x81 or 0x83;    // Enable ISR
      	status = mpu3050_write_reg(mpu3050_i2c_client,buf);  
	printk("[MPU3050] Disable ISR :  MPU3050_GYRO_I2C_INT_CFG\n"); 
				
      	// Cfg Sampling MPU
      	buf[0] = MPU3050_GYRO_I2C_SMPLR_DIV;
      	buf[1] = 0x04;    
      	status = mpu3050_write_reg(mpu3050_i2c_client,buf);  
	printk("[MPU3050] MPU3050_GYRO_I2C_SMPLR_DIV\n");	
					
      	buf[0] = MPU3050_GYRO_I2C_DLPF_FS_SYNC;
      	buf[1] =((0x3<<3)|0x3);    
      	status = mpu3050_write_reg(mpu3050_i2c_client,buf);  
	printk("[MPU3050] MPU3050_GYRO_I2C_DLPF_FS_SYNC\n"); 	
      	
      	/*----------------------------------CLKSource------------------------------------------*/
      	  value = 0;   
      	  status = mpu3050_read_reg(mpu3050_i2c_client,MPU3050_GYRO_I2C_PWR_MGM,&value);

         value &= ~(0x07);   //clear clk_sel bits
         value |= 0x03;        //set pll gyro z clk source
         buf[0] = MPU3050_GYRO_I2C_PWR_MGM;
         buf[1] = value;  
         status = mpu3050_write_reg(mpu3050_i2c_client,buf);
	  printk("[MPU3050] MPU3050_GYRO_I2C_PWR_MGM --- clk source \n"); 	
       	 

      /*------------------------------ Set Accelerometer --------------------------------------*/  
	  // Accel Data Start
	buf[0] = MPU3050_GYRO_I2C_ACCEL_BURST_ADDR;
	buf[1] = 0x28; /*0xA8*/;
	status = mpu3050_write_reg(mpu3050_i2c_client,buf);
	printk("[MPU3050] MPU3050_GYRO_I2C_ACCEL_BURST_ADDR --- clk source \n");

	printk("[MPU3050] MPU3050_BYPASS_MODE_ON --- \n");

	mpu3050_i2c_through_pass(MPU3050_BYPASS_MODE_ON);

     /*-----------------------------------------------------------------------------------------*/	  
#if 0
       value = 0;
       status = mpu3050_read_reg(mpu3050_i2c_client,MPU3050_GYRO_I2C_ACCEL_BURST_ADDR,value);
	printk("[MPU3050_GYRO_I2C_ACCEL_BURST_ADDR] read value [0x%x]\n",value);

	if(value & 0x80)
	{
		value &= ~(0x80);
	}
	else
	{
		value |= 0x80;
	}
	
	printk("[MPU3050_GYRO_I2C_ACCEL_BURST_ADDR] setting value [0x%x]\n",value);
	
	buf[0] = MPU3050_GYRO_I2C_ACCEL_BURST_ADDR;
	buf[1] = value;	
	status = mpu3050_write_reg(mpu3050_i2c_client,buf);

	mpu3050_i2c_through_pass(MPU3050_BYPASS_MODE_ON);
#endif

#if 1  
        // ACCEL i2c pass through on
 //	 mpu3050_i2c_through_pass(MPU3050_BYPASS_MODE_ON);

//    	kr3dh_initialize();
    	
      	//ACCEL i2c pass through off
//       mpu3050_i2c_through_pass(MPU3050_BYPASS_MODE_OFF);

      /*
	value = 0;
      	status = mpu3050_read_reg(mpu3050_i2c_client,MPU3050_GYRO_I2C_USER_CTRL,&value); 	       
      	//value &= ~(0x09);
      	  if(!(value & 0x10))
      	       value|=0x10;
      	      
	buf[0] = MPU3050_GYRO_I2C_USER_CTRL;
      	//buf[1] = value;
      	buf[1] = 0x30;
      	status = mpu3050_write_reg(mpu3050_i2c_client,buf);
      	*/
#endif

}

void  mpu3050_read_gyro_xyz(unsigned char *data_xyz)
{
	int status = 0;

	status = mpu3050_read_reg_in_burst(mpu3050_i2c_client,MPU3050_GYRO_I2C_GYRO_XOUT_H,&data_xyz[0],6);

}

/*----------------------------------------------------------------------------*/
int mpu3050_i2c_read(unsigned char reg,unsigned char *buf,int length)
{
	int status = 0;

	status = mpu3050_read_reg_in_burst(mpu3050_i2c_client,reg,buf,length);

	return status;
}

int  mpu3050_i2c_write(unsigned char *buffer,int length)
{
	int status = 0;

	status = mpu3050_write_reg_in_burst(mpu3050_i2c_client,buffer,length);

	return status;
	
}

