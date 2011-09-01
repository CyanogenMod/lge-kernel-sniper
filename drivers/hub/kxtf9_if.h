/* 
 * Copyright (C) 2010 LGE Inc.
 * 
 * KXTF9 Accelerometer Interface header file.
 * 
 */

#ifndef __KXTF9_ACCEL_IF_H__
#define __KXTF9_ACCEL_IF_H__

 
#include "kxtf9.h"
 
 /* tapping type */
enum{
	  ACCEL_TAP_MODE_SINGLE = 1,
	  ACCEL_TAP_MODE_DOUBLE 	
};

/* tapping direction */
enum{
	  ACCEL_TAP_UP  = 1,		 
	  ACCEL_TAP_DOWN,	   
	  ACCEL_TAP_LEFT,
	  ACCEL_TAP_RIGHT,
	  ACCEL_TAP_FRONT,
	  ACCEL_TAP_BACK
};

/* flip type */
enum{
	  ACCEL_FLIP_UPSIDE_DOWN   = 1,
	  ACCEL_FLIP_DOWNSIDE_UP
};

/* snap type */
enum{
	  ACCEL_SNAP_PICTH_UP = 1,
	  ACCEL_SNAP_ROLL_RIGHT,
      ACCEL_SNAP_ROLL_LEFT,
      ACCEL_SNAP_PICTH_DOWN
};
 
	
 typedef void (*accel_send_tap_cb_fn_type)(int type,int direction);
 typedef void (*accel_send_shake_cb_fn_type)(int value);
 typedef void (*accel_send_flip_cb_fn_type)(int value);
 typedef void (*accel_send_snap_cb_fn_type)(int value);
 

 
 /*--------------------------------------------------------------------------------------------------------------------
	 Functions
	--------------------------------------------------------------------------------------------------------------------*/	 
 int	kxtf9_accel_init(void);
 void   kxtf9_accel_register_i2c_func(void);
 int	kxtf9_accel_service_isr(void);
 int    kxtf9_read_current_position(void);
 int	kxtf9_accel_read_raw_data(unsigned char *buf);
 int	kxtf9_accel_read_data_api(int *buf);
 int	kxtf9_accel_enable_tap(void);
 int	kxtf9_accel_disable_tap(void);
 int	kxtf9_accel_enable_shake(void);
 int	kxtf9_accel_detect_shake(void);
 int 	kxtf9_accel_detect_flip(int value);
 int 	kxtf9_accel_enable_snap(void);
 int 	kxtf9_accel_disable_snap(void);
 void  	kxtf9_accel_send_tap_event(int type,int direction);
 void  	kxtf9_accel_send_shake_event(int value);
 void  	kxtf9_accel_send_screen_rotation_event(int value, int pre_value);
 void  	kxtf9_accel_tap_register_callback(void (*func_ptr)(int type,int direction));
 void  	kxtf9_accel_snap_register_callback(void (*func_ptr)(int value));
 void  	kxtf9_accel_shake_register_callback(void (*func_ptr)(int value));
 void  	kxtf9_accel_flip_register_callback(void (*func_ptr)(int value));
 int	kxtf9_accel_api_read_data(int *buf);
 int	kxtf9_accel_api_set_sample_rate(int index);
 int	kxtf9_accel_api_get_sample_rate(int *index);
 int	kxtf9_accel_api_get_g_range(int *range);
 int	kxtf9_accel_api_get_resolution(int *resolution);
/*LGE_CHAGE_S [taehwan.kim@lge.com] 2010-03-31*/
 int is_tap_enabled(void);
 int is_snap_enabled(void);
 int is_flip_enabled(void);
/*LGE_CHAGE_E [taehwan.kim@lge.com] 2010-03-31*/
#endif //__KXTF9_ACCEL_IF_H__

