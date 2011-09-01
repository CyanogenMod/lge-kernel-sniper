/* 
 * Copyright (C) 2009 LGE Inc.
 * 
 * KXTF9 Accelerometer header file.
 * 
 */
 
 #ifndef __KXTF9_H__
#define  __KXTF9_H__
 
 
 /*==================================================================================================
				 LGE Definitions
 ==================================================================================================*/
 
#define  KXTF9_DRIVER_NAME   	    "kxtf9"     	 /* i2c device*/
 
#define  KXTF9_IOCTL_NAME      		"kxtf9_accel"    /* for JSR256 API */

#define  KXTF9_GESTURE_INPUT_NAME   "lge_gesture"  /* for Sensor HAL */


 enum
 {
	 KXTF9_ACCEL_POWER_OFF = 0,
	 KXTF9_ACCEL_POWER_ON	 
 };

 enum
 {
 	KXTF9_TAPPING_ENABLE = 0x01,
	KXTF9_SHAKE_ENABLE = 0x02,
	KXTF9_FLIP_ENABLE = 0x04,
	KXTF9_SNAP_ENABLE = 0x08
 };
 
 /*-------------------------------------------------------------------*/
 /* 					 		IOCTL									   */
 /*-------------------------------------------------------------------*/
  
#define KXTF9_IOC_MAGIC			      0xA2
 
#define KXTF9_IOCTL_SET_POWER_ONOFF   _IOWR(KXTF9_IOC_MAGIC, 0x01, int)
#define KXTF9_IOCTL_GET_POWER_ONOFF   _IOWR(KXTF9_IOC_MAGIC, 0x02, int)
#define KXTF9_IOCTL_GET_ACCEL_RAW     _IOWR(KXTF9_IOC_MAGIC, 0x03, unsigned char[10])
#define KXTF9_IOCTL_GET_DATA    	  _IOWR(KXTF9_IOC_MAGIC, 0x04, int[3])
#define KXTF9_IOCTL_SET_SAMPLE_RATE   _IOWR(KXTF9_IOC_MAGIC, 0x05, int)
#define KXTF9_IOCTL_GET_SAMPLE_RATE   _IOWR(KXTF9_IOC_MAGIC, 0x06, int)
#define KXTF9_IOCTL_GET_G_RANGE    	  _IOWR(KXTF9_IOC_MAGIC, 0x07, int)
#define KXTF9_IOCTL_GET_RESOLUTION    _IOWR(KXTF9_IOC_MAGIC, 0x08, int)


#define  ACCEL_I2C_SLAVE_ADDR      0x0F  // KXTF9


/*-------------------------------------------------------------------*/
/* 					 		FUNCTION								 */
/*------------------------------------------------------------------*/

int kxtf9_i2c_read(unsigned char reg,unsigned char *data,int length); 
int kxtf9_i2c_write_burst(unsigned char *buffer,int length);
int kxtf9_i2c_write(unsigned char reg,unsigned char data);
int kxtf9_enable_gesture_status(void);
void kxtf9_enable_irq(void);
void kxtf9_disable_irq(void);
 
#endif //__KXTF9_H__
 

