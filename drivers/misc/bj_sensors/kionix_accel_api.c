 /*
  * KIONIX Accelerometer API 
  *
  * Copyright (C) 2009 LGE, Inc.
  *
  *  2009/11/10 : 
  *
  */

#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/device.h>

#include "kionix_accel_api.h"
#include "kxtf9_if.h"
//--[[ LGE_UBIQUIX_MODIFIED_START : shyun@ubiquix.com [2011.07.14] - Header file name is modified.
//#include "hub_motion.h"
#include "bj_motion.h"
//--]] LGE_UBIQUIX_MODIFIED_END : shyun@ubiquix.com [2011.07.14]- Header file name is modified.

//static int 		  KIONIX_ACCEL_g_range = 2000; 	/* KIONIX_ACCEL factory default G range in milli g */
static int 		   device = 1;						/* flag to indicate device KXTE9=0, KXTF9=1 */ 	

accel_i2c_read_fn  kionix_i2c_read = NULL;
accel_i2c_write_fn kionix_i2c_write = NULL;

#define DEBUG 0

#if 1 // YJ
extern int oca_enabled;
extern int oca[3];

extern int axis_map_x;
extern int axis_map_y;
extern int axis_map_z;

extern int negate_x;
extern int negate_y;
extern int negate_z;
#endif


void KIONIX_ACCEL_i2c_register_cb(void)
{
	kionix_i2c_read  = &kxtf9_i2c_read;
	kionix_i2c_write = &kxtf9_i2c_write;
}

/*==================================================================================================
FUNCTION: 		KIONIX_ACCEL_read_bytes
DESCRIPTION:    	This function reads data from the Kionix accelerometer in bytes.
ARGUMENTS PASSED:	register address, data pointer, length in number of bytes
RETURN VALUE:		0 = pass; 1 = fail
PRE-CONDITIONS:		KIONIX_ACCEL_init() has been called
POST-CONDITIONS:	None
IMPORTANT NOTES:	Using the i2c_smbus functions requires the inclusion of the i2c-dev.h file 
			available in the lm-sensors package.
==================================================================================================*/
int KIONIX_ACCEL_read_bytes(int reg, unsigned char* data, int length)
{
	int res=0;

	if(kionix_i2c_read)	
		kionix_i2c_read((unsigned char)reg,data,length);
	else
		printk("[KIONIX_ACCEL]unregister i2c read function !!\n");

	return res;
}
/*==================================================================================================
FUNCTION: 		KIONIX_ACCEL_write_byte
DESCRIPTION:		This function writes a byte of data to the Kionix accelerometer.
ARGUMENTS PASSED:	register address, data variable
RETURN VALUE:   	0 = pass; 1 = fail
PRE-CONDITIONS:   	KIONIX_ACCEL_init() has been called
POST-CONDITIONS:   	None
IMPORTANT NOTES:   	Using the i2c_smbus_write_byte_data function requires the inclusion of
			the i2c-dev.h file available in the lm-sensors package.
==================================================================================================*/
int KIONIX_ACCEL_write_byte(int reg, int data)
{
	int res=0;

	if(kionix_i2c_write)
		kionix_i2c_write((unsigned char)reg,(unsigned char)data);
	else		
		printk("[KIONIX_ACCEL]unregister i2c write function !!\n");

	return res;
	
}
/*==================================================================================================
FUNCTION: 		KIONIX_ACCEL_isr
DESCRIPTION:    	This function is the interrupt service routine for the accelerometer. 
ARGUMENTS PASSED:   	None
RETURN VALUE:   	None
PRE-CONDITIONS:   	None
POST-CONDITIONS:   	None
IMPORTANT NOTES:	Called from interrupt context, so do NOT do any i2c operations!
==================================================================================================*/
void KIONIX_ACCEL_isr(void)
{
#if 0
	extern SU_TASK_HANDLE accel_task_handle;
	// disable accelerometer interrupt first
	KIONIX_ACCEL_disable_interrupt();
	// Set event to handle interrupt
	suSetEventMask(accel_task_handle, ACCEL_EVENT_INTERRUPT, NULL);
	// enable accelerometer interrupt again
	KIONIX_ACCEL_enable_interrupt();
	// disable accelerometer interrupt first
	KIONIX_ACCEL_disable_interrupt();
	// Set event to handle interrupt
	rex_set_sigs( &accel_tcb, ACCEL_ISR_SIG ); /* Signal a queue event */
#endif

}
/*==================================================================================================
FUNCTION: 		KIONIX_ACCEL_enable_interrupt
DESCRIPTION:    	This function enables the interrupt for the accelerometer. 
ARGUMENTS PASSED:   	None
RETURN VALUE:   	None
PRE-CONDITIONS:   	None
POST-CONDITIONS:   	None
IMPORTANT NOTES:	Called from interrupt context, so do NOT do any i2c operations!
==================================================================================================*/
void KIONIX_ACCEL_enable_interrupt(void)
{
	KIONIX_ACCEL_enable_int();
}
/*==================================================================================================
FUNCTION: 		KIONIX_ACCEL_disable_interrupt
DESCRIPTION:    	This function disables the interrupt for the accelerometer. 
ARGUMENTS PASSED:   	None
RETURN VALUE:   	None
PRE-CONDITIONS:   	None
POST-CONDITIONS:   	None
IMPORTANT NOTES:	Called from interrupt context, so do NOT do any i2c operations!
==================================================================================================*/
void KIONIX_ACCEL_disable_interrupt(void)
{
	KIONIX_ACCEL_disable_int();
}

/*==================================================================================================
FUNCTION: 		KIONIX_ACCEL_enable_outputs
DESCRIPTION:    	This function enables accelerometer outputs. 
ARGUMENTS PASSED:   	None
RETURN VALUE:   	0 = pass; 1 = fail
PRE-CONDITIONS:   	KIONIX_ACCEL_init() has been called
POST-CONDITIONS:   	Accelerometer outputs enabled
==================================================================================================*/
int KIONIX_ACCEL_enable_outputs(void)
{
	char ctlreg_1 = 0;
	int status = 0;
	if (KIONIX_ACCEL_read_bytes(KIONIX_ACCEL_I2C_CTRL_REG1, &ctlreg_1, 1) == 0){
		SET_REG_BIT(ctlreg_1, CTRL_REG1_PC1); /* sets PC1 bit to be in power up state */
		KIONIX_ACCEL_write_byte(KIONIX_ACCEL_I2C_CTRL_REG1, ctlreg_1);
	}
	else status = 1;

	return status;
}
int KIONIX_ACCEL_disable_outputs(void)
{
	char ctlreg_1 = 0;
	int status = 0;
	if (KIONIX_ACCEL_read_bytes(KIONIX_ACCEL_I2C_CTRL_REG1, &ctlreg_1, 1) == 0){
		UNSET_REG_BIT(ctlreg_1, CTRL_REG1_PC1); /* sets PC1 bit to be in power up state */
		KIONIX_ACCEL_write_byte(KIONIX_ACCEL_I2C_CTRL_REG1, ctlreg_1);
	}
	else status = 1;

	return status;

}
/*==================================================================================================
FUNCTION: 		KIONIX_ACCEL_enable_tilt_function
DESCRIPTION:    	This function enables the tilt position function. 
ARGUMENTS PASSED:   	None
RETURN VALUE:   	0 = pass; 1 = fail
PRE-CONDITIONS:   	KIONIX_ACCEL_init() has been called
POST-CONDITIONS:   	Tilt position function is enabled
==================================================================================================*/
int KIONIX_ACCEL_enable_tilt_function(void)
{
	char ctlreg_1 = 0;
	int status = 0;
	if (KIONIX_ACCEL_read_bytes(KIONIX_ACCEL_I2C_CTRL_REG1, &ctlreg_1, 1) == 0){
		SET_REG_BIT(ctlreg_1, CTRL_REG1_TPE); /* sets TPE bit to enable tilt position function*/
		KIONIX_ACCEL_write_byte(KIONIX_ACCEL_I2C_CTRL_REG1, ctlreg_1);
	}
	else status = 1;

	return status;

}
/*==================================================================================================
FUNCTION: 		KIONIX_ACCEL_disable_tilt_function
DESCRIPTION:    	This function disables the tilt position function. 
ARGUMENTS PASSED:   	None
RETURN VALUE:   	0 = pass; 1 = fail
PRE-CONDITIONS:   	KIONIX_ACCEL_init() has been called
POST-CONDITIONS:   	Tilt position function is disabled
==================================================================================================*/
int KIONIX_ACCEL_disable_tilt_function(void)
{
	char ctlreg_1 = 0;
	int status = 0;
	if (KIONIX_ACCEL_read_bytes(KIONIX_ACCEL_I2C_CTRL_REG1, &ctlreg_1, 1) == 0){
		UNSET_REG_BIT(ctlreg_1, CTRL_REG1_TPE); /* unset TPE bit to disable tilt position function */
		KIONIX_ACCEL_write_byte(KIONIX_ACCEL_I2C_CTRL_REG1, ctlreg_1);
	}
	else status = 1;

	return status;

}
/*==================================================================================================
FUNCTION: 		KIONIX_ACCEL_enable_wake_up_function
DESCRIPTION:    	This function enables the wake up function. 
ARGUMENTS PASSED:   	None
RETURN VALUE:   	0 = pass; 1 = fail
PRE-CONDITIONS:   	KIONIX_ACCEL_init() has been called
POST-CONDITIONS:   	Wake up function is enabled
==================================================================================================*/
int KIONIX_ACCEL_enable_wake_up_function(void)
{
	char ctlreg_1 = 0;
	int status = 0;
	if (KIONIX_ACCEL_read_bytes(KIONIX_ACCEL_I2C_CTRL_REG1, &ctlreg_1, 1) == 0){
		SET_REG_BIT(ctlreg_1, CTRL_REG1_WUFE); /* set WUFE bit to enable the wake up function */
		KIONIX_ACCEL_write_byte(KIONIX_ACCEL_I2C_CTRL_REG1, ctlreg_1);
	}
	else status = 1;

	return status;

}
/*==================================================================================================
FUNCTION: 		KIONIX_ACCEL_disable_wake_up_function
DESCRIPTION:    	This function disables the wake up function. 
ARGUMENTS PASSED:   	None
RETURN VALUE:   	0 = pass; 1 = fail
PRE-CONDITIONS:   	KIONIX_ACCEL_init() has been called
POST-CONDITIONS:   	Wake up function is disabled
==================================================================================================*/
int KIONIX_ACCEL_disable_wake_up_function(void)
{
	char ctlreg_1 = 0;
	int status = 0;
	if (KIONIX_ACCEL_read_bytes(KIONIX_ACCEL_I2C_CTRL_REG1, &ctlreg_1, 1) == 0){
		UNSET_REG_BIT(ctlreg_1, CTRL_REG1_WUFE); /* unset the WUFE bit to disable the wake up function */
		KIONIX_ACCEL_write_byte(KIONIX_ACCEL_I2C_CTRL_REG1, ctlreg_1);
	}
	else status = 1;

	return status;

}
/*==================================================================================================
FUNCTION: 		KIONIX_ACCEL_enable_all
DESCRIPTION:    	This function enables all engines. 
ARGUMENTS PASSED:   	None
RETURN VALUE:   	0 = pass; 1 = fail
PRE-CONDITIONS:   	KIONIX_ACCEL_init() has been called
POST-CONDITIONS:   	All engines enabled
==================================================================================================*/
int KIONIX_ACCEL_enable_all(void)
{
	char ctlreg_1 = 0;
	int status = 0;
	if (KIONIX_ACCEL_read_bytes(KIONIX_ACCEL_I2C_CTRL_REG1, &ctlreg_1, 1) == 0){
		SET_REG_BIT(ctlreg_1, CTRL_REG1_PC1); /* set PC1 to enable the accelerometer outputs */
		SET_REG_BIT(ctlreg_1, CTRL_REG1_TPE); /* set TPE bit to enable the tilt function */
		SET_REG_BIT(ctlreg_1, CTRL_REG1_WUFE); /* set WUFE to enable the wake up function */
		if (device == 0){	//KXTE9
			SET_REG_BIT(ctlreg_1, CTRL_REG1_B2SE); /* set B2SE to enable back to sleep function on KXTE9 */
		}
		else if (device == 1){	//KXTF9
			SET_REG_BIT(ctlreg_1, CTRL_REG1_TDTE); /* set TDTE to enable tap function on KXTF9 */
		}		
		KIONIX_ACCEL_write_byte(KIONIX_ACCEL_I2C_CTRL_REG1, ctlreg_1);
	}
	else status = 1;

	return status;
}
/*==================================================================================================
FUNCTION: 		KIONIX_ACCEL_disable_all
DESCRIPTION:    	This function disables all engines. 
ARGUMENTS PASSED:   	None
RETURN VALUE:   	0 = pass; 1 = fail
PRE-CONDITIONS:   	KIONIX_ACCEL_init() has been called
POST-CONDITIONS:   	All engines disabled
==================================================================================================*/
int KIONIX_ACCEL_disable_all(void)
{
	char ctlreg_1 = 0;
	int status = 0;
	if (KIONIX_ACCEL_read_bytes(KIONIX_ACCEL_I2C_CTRL_REG1, &ctlreg_1, 1) == 0){
		UNSET_REG_BIT(ctlreg_1, CTRL_REG1_PC1); /* unset the PC1 bit to disable the accelerometer outputs */
		UNSET_REG_BIT(ctlreg_1, CTRL_REG1_TPE); /* unset the TPE bit to disable the tilt function */
		UNSET_REG_BIT(ctlreg_1, CTRL_REG1_WUFE); /* unset WUFE to disable wake up function */
		if (device == 0){	//KXTE9
			UNSET_REG_BIT(ctlreg_1, CTRL_REG1_B2SE); /* set B2SE to enable back to sleep function on KXTE9 */
		}
		else if (device == 1){	//KXTF9
			UNSET_REG_BIT(ctlreg_1, CTRL_REG1_TDTE); /* set TDTE to enable tap function on KXTF9 */
		}
		KIONIX_ACCEL_write_byte(KIONIX_ACCEL_I2C_CTRL_REG1, ctlreg_1);
	}
	else status = 1;

	return status;

}
/*==================================================================================================
FUNCTION: 		KIONIX_ACCEL_sleep
DESCRIPTION:    	This function places the accelerometer into a standby state while retaining 
			current register values. 
ARGUMENTS PASSED:   	None
RETURN VALUE:   	0 = pass; 1 = fail
PRE-CONDITIONS:   	KIONIX_ACCEL_init() has been called
POST-CONDITIONS:   	Device is in sleep mode
==================================================================================================*/
int KIONIX_ACCEL_sleep(void)
{
	char ctlreg_1 = 0;
	int status = 0;
	if (KIONIX_ACCEL_read_bytes(KIONIX_ACCEL_I2C_CTRL_REG1, &ctlreg_1, 1) == 0 ){
		UNSET_REG_BIT(ctlreg_1, CTRL_REG1_PC1); /* unset the PC1 bit to disable the accelerometer */
		KIONIX_ACCEL_write_byte(KIONIX_ACCEL_I2C_CTRL_REG1, ctlreg_1);
	}
	else status = 1;

	return status;

}
/*==================================================================================================
FUNCTION: 		KIONIX_ACCEL_read_interrupt_status
DESCRIPTION:    	This function reads the physical pin interrupt status. 
ARGUMENTS PASSED:   	None
RETURN VALUE:   	0 = interrupt active; 1 = interrupt inactive
PRE-CONDITIONS:   	KIONIX_ACCEL_init() has been called
POST-CONDITIONS:   	None
==================================================================================================*/
int KIONIX_ACCEL_read_interrupt_status(void)
{
	int interrupt_status; 
	char status_reg;
	if (KIONIX_ACCEL_read_bytes(KIONIX_ACCEL_I2C_STATUS_REG, &status_reg, 1) == 0){
		if ((status_reg & 0x10) == 0x00){
			interrupt_status = 1;
		}
		else	interrupt_status = 0;
	}
	else	 {
		interrupt_status = 1;
	}
	return interrupt_status;
}
/*==================================================================================================
FUNCTION: 		KIONIX_ACCEL_read_interrupt_source
DESCRIPTION:    	This function reads the Interrupt Source 2 register. 
ARGUMENTS PASSED:   	None
RETURN VALUE:   	0 = interrupt active; 1 = interrupt inactive
PRE-CONDITIONS:   	KIONIX_ACCEL_init() has been called
POST-CONDITIONS:   	interrupt_source contains the byte read from Interrupt Source Register 2
==================================================================================================*/
int KIONIX_ACCEL_read_interrupt_source(char* interrupt_source)
{
	int interrupt_status =0;
	
	if (device == 0){	//KXTE9
		if (KIONIX_ACCEL_read_bytes(KXTE9_I2C_INT_SRC_REG2, interrupt_source, 1) == 0){
			if (interrupt_source != 0x00){
				interrupt_status = 0;
			}
			else	interrupt_status = 1;
		}
	}
	else if (device == 1){	//KXTF9
		if (KIONIX_ACCEL_read_bytes(KXTF9_I2C_INT_SRC_REG2, interrupt_source, 1) == 0){
			if (interrupt_source != 0x00){
				interrupt_status = 0;
			}
			else	interrupt_status = 1;
		}
	}
	else	interrupt_status = 1;
	return interrupt_status;
}
/*==================================================================================================
FUNCTION: 		KIONIX_ACCEL_read_previous_position
DESCRIPTION:    	This function reads the previous tilt position register. 
ARGUMENTS PASSED:   	previous_position pointer
RETURN VALUE:   	0 = pass; 1 = fail
PRE-CONDITIONS:   	KIONIX_ACCEL_init() has been called
POST-CONDITIONS:   	previous_position is assigned
==================================================================================================*/
int KIONIX_ACCEL_read_previous_position(char* previous_position)
{
	int status;
	status = KIONIX_ACCEL_read_bytes(KIONIX_ACCEL_I2C_TILT_POS_PRE, previous_position, 1);
	return status;
}
/*==================================================================================================
FUNCTION: 		KIONIX_ACCEL_read_current_position
DESCRIPTION:    	This function reads the current tilt position register. 
ARGUMENTS PASSED:   	current_position pointer
RETURN VALUE:   	0 = pass; 1 = fail
PRE-CONDITIONS:   	KIONIX_ACCEL_init() has been called
POST-CONDITIONS:   	current_position is assigned
==================================================================================================*/
int KIONIX_ACCEL_read_current_position(char* current_position)
{
	int status;
	status = KIONIX_ACCEL_read_bytes(KIONIX_ACCEL_I2C_TILT_POS_CUR, current_position, 1);
	return status;
}
/*==================================================================================================
FUNCTION: 		KIONIX_ACCEL_reset
DESCRIPTION:    	This function issues a software reset to the Kionix accelerometer. 
ARGUMENTS PASSED:   	None
RETURN VALUE:   	0 = pass; 1 = fail
PRE-CONDITIONS:   	KIONIX_ACCEL_init() has been called
POST-CONDITIONS:   	Accelerometer is reset (will have to re-initialize)
==================================================================================================*/
int KIONIX_ACCEL_reset(void)
{
	char ctrl_reg3 = 0;
	int status = 0;
	if (KIONIX_ACCEL_read_bytes(KIONIX_ACCEL_I2C_CTRL_REG3, &ctrl_reg3, 1) == 0){
		SET_REG_BIT(ctrl_reg3, CTRL_REG3_SRST);
		KIONIX_ACCEL_write_byte(KIONIX_ACCEL_I2C_CTRL_REG3, ctrl_reg3);
	}
	else status = 1;

	return status;

}
/*==================================================================================================
FUNCTION: 		KIONIX_ACCEL_tilt_timer
DESCRIPTION:    	This function sets the number of tilt debounce samples. 
ARGUMENTS PASSED:   	tilt_timer; 0-255
RETURN VALUE:   	0 = pass; 1 = fail
PRE-CONDITIONS:   	KIONIX_ACCEL_init() has been called
POST-CONDITIONS:   	Tile debounce set according to tilt_timer
==================================================================================================*/
int KIONIX_ACCEL_tilt_timer(int tilt_timer)
{
	int status;
	if (KIONIX_ACCEL_write_byte(KIONIX_ACCEL_I2C_TILT_TIMER, tilt_timer) == 0){
		status = 0;
	}
	else status = 1;

	return status;

}
/*==================================================================================================
FUNCTION: 		KIONIX_ACCEL_wuf_timer
DESCRIPTION:    	This function sets the number of wake-up debounce samples. 
ARGUMENTS PASSED:   	wuf_timer; 0-255
RETURN VALUE:   	0 = pass; 1 = fail
PRE-CONDITIONS:   	KIONIX_ACCEL_init() has been called
POST-CONDITIONS:   	Wake-up-function debounce set according to wuf_timer
==================================================================================================*/
int KIONIX_ACCEL_wuf_timer(int wuf_timer)
{
	int status;
	if (KIONIX_ACCEL_write_byte(KIONIX_ACCEL_I2C_WUF_TIMER, wuf_timer) == 0){
		status = 0;
	}
	else status = 1;

	return status;

}
/*==================================================================================================
FUNCTION: 		KIONIX_ACCEL_wuf_thresh
DESCRIPTION:    	This function defines the threshold for general motion detection. 
ARGUMENTS PASSED:   	wuf_thresh; 0-255
RETURN VALUE:   	0 = pass; 1 = fail
PRE-CONDITIONS:   	KIONIX_ACCEL_init() has been called
POST-CONDITIONS:   	Wake up function threshold set according to wuf_thresh
IMPORTANT NOTES:   	Default: 0.5g (0x20h)
==================================================================================================*/
int KIONIX_ACCEL_wuf_thresh(int wuf_thresh)
{
	int status;
	if (KIONIX_ACCEL_write_byte(KIONIX_ACCEL_I2C_WUF_THRESH, wuf_thresh) == 0){
		status = 0;
	}
	else status = 1;

	return status;

}
/*==================================================================================================
FUNCTION: 		KIONIX_ACCEL_position_mask_z
DESCRIPTION:    	This function masks Z-axis from the activity engine. 
ARGUMENTS PASSED:   	None
RETURN VALUE:   	0 = pass; 1 = fail
PRE-CONDITIONS:   	KIONIX_ACCEL_init() has been called
POST-CONDITIONS:   	Z-axis masked
==================================================================================================*/
int KIONIX_ACCEL_motion_mask_z(void)
{
	char int_ctrl_reg2 = 0;
	int status = 0;
	if (KIONIX_ACCEL_read_bytes(KIONIX_ACCEL_I2C_INT_CTRL_REG2, &int_ctrl_reg2, 1) == 0){
		SET_REG_BIT(int_ctrl_reg2, INT_CTRL_REG2_ZBW);
		KIONIX_ACCEL_write_byte(KIONIX_ACCEL_I2C_INT_CTRL_REG2, int_ctrl_reg2);
	}
	else status = 1;

	return status;

}
/*==================================================================================================
FUNCTION: 		KIONIX_ACCEL_position_unmask_z
DESCRIPTION:    	This function unmasks Z-axis from the activity engine. 
ARGUMENTS PASSED:   	None
RETURN VALUE:   	0 = pass; 1 = fail
PRE-CONDITIONS:   	KIONIX_ACCEL_init() has been called
POST-CONDITIONS:   	Z-axis unmasked
==================================================================================================*/
int KIONIX_ACCEL_motion_unmask_z(void)
{
	char int_ctrl_reg2 = 0;
	int status = 0;
	if (KIONIX_ACCEL_read_bytes(KIONIX_ACCEL_I2C_INT_CTRL_REG2, &int_ctrl_reg2, 1) == 0){
		UNSET_REG_BIT(int_ctrl_reg2, INT_CTRL_REG2_ZBW);
		KIONIX_ACCEL_write_byte(KIONIX_ACCEL_I2C_INT_CTRL_REG2, int_ctrl_reg2);
	}
	else status = 1;

	return status;

}
/*==================================================================================================
FUNCTION: 		KIONIX_ACCEL_position_mask_y
DESCRIPTION:    	This function masks Y-axis from the activity engine. 
ARGUMENTS PASSED:   	None
RETURN VALUE:   	0 = pass; 1 = fail
PRE-CONDITIONS:   	KIONIX_ACCEL_init() has been called
POST-CONDITIONS:   	Y-axis masked
==================================================================================================*/
int KIONIX_ACCEL_motion_mask_y(void)
{
	char int_ctrl_reg2 = 0;
	int status = 0;
	if (KIONIX_ACCEL_read_bytes(KIONIX_ACCEL_I2C_INT_CTRL_REG2, &int_ctrl_reg2, 1) == 0){
		SET_REG_BIT(int_ctrl_reg2, INT_CTRL_REG2_YBW);
		KIONIX_ACCEL_write_byte(KIONIX_ACCEL_I2C_INT_CTRL_REG2, int_ctrl_reg2);
	}
	else status = 1;

	return status;

}
/*==================================================================================================
FUNCTION: 		KIONIX_ACCEL_position_unmask_y
DESCRIPTION:    	This function unmasks Y-axis from the activity engine. 
ARGUMENTS PASSED:   	None
RETURN VALUE:   	0 = pass; 1 = fail
PRE-CONDITIONS:   	KIONIX_ACCEL_init() has been called
POST-CONDITIONS:   	Y-axis unmasked
==================================================================================================*/
int KIONIX_ACCEL_motion_unmask_y(void)
{
	char int_ctrl_reg2 = 0;
	int status = 0;
	if (KIONIX_ACCEL_read_bytes(KIONIX_ACCEL_I2C_INT_CTRL_REG2, &int_ctrl_reg2, 1) == 0){
		UNSET_REG_BIT(int_ctrl_reg2, INT_CTRL_REG2_YBW);
		KIONIX_ACCEL_write_byte(KIONIX_ACCEL_I2C_INT_CTRL_REG2, int_ctrl_reg2);
	}
	else status = 1;

	return status;

}
/*==================================================================================================
FUNCTION: 		KIONIX_ACCEL_position_mask_x
DESCRIPTION:    	This function masks X-axis from the activity engine. 
ARGUMENTS PASSED:   	None
RETURN VALUE:   	0 = pass; 1 = fail
PRE-CONDITIONS:   	KIONIX_ACCEL_init() has been called
POST-CONDITIONS:   	X-axis masked
==================================================================================================*/
int KIONIX_ACCEL_motion_mask_x(void)
{
	char int_ctrl_reg2 = 0;
	int status = 0;
	if (KIONIX_ACCEL_read_bytes(KIONIX_ACCEL_I2C_INT_CTRL_REG2, &int_ctrl_reg2, 1) == 0){
		SET_REG_BIT(int_ctrl_reg2, INT_CTRL_REG2_XBW);
		KIONIX_ACCEL_write_byte(KIONIX_ACCEL_I2C_INT_CTRL_REG2, int_ctrl_reg2);
	}
	else status = 1;

	return status;

}
/*==================================================================================================
FUNCTION: 		KIONIX_ACCEL_position_unmask_x
DESCRIPTION:    	This function unmasks X-axis from the activity engine. 
ARGUMENTS PASSED:   	None
RETURN VALUE:   	0 = pass; 1 = fail
PRE-CONDITIONS:   	KIONIX_ACCEL_init() has been called
POST-CONDITIONS:   	X-axis unmasked
==================================================================================================*/
int KIONIX_ACCEL_motion_unmask_x(void)
{
	char int_ctrl_reg2 = 0;
	int status = 0;
	if (KIONIX_ACCEL_read_bytes(KIONIX_ACCEL_I2C_INT_CTRL_REG2, &int_ctrl_reg2, 1) == 0){
		UNSET_REG_BIT(int_ctrl_reg2, INT_CTRL_REG2_XBW);
		KIONIX_ACCEL_write_byte(KIONIX_ACCEL_I2C_INT_CTRL_REG2, int_ctrl_reg2);
	}
	else status = 1;

	return status;

}
/*==================================================================================================
FUNCTION: 		KIONIX_ACCEL_position_mask_fu
DESCRIPTION:    	This function masks face-up state in the screen rotation function. 
ARGUMENTS PASSED:   	None
RETURN VALUE:   	0 = pass; 1 = fail
PRE-CONDITIONS:   	KIONIX_ACCEL_init() has been called
POST-CONDITIONS:   	Face up state masked
==================================================================================================*/
int KIONIX_ACCEL_position_mask_fu(void)
{
	char ctrl_reg2 = 0;
	int status = 0;
	if (KIONIX_ACCEL_read_bytes(KIONIX_ACCEL_I2C_CTRL_REG2, &ctrl_reg2, 1) == 0){
		SET_REG_BIT(ctrl_reg2, CTRL_REG2_FUM);
		KIONIX_ACCEL_write_byte(KIONIX_ACCEL_I2C_CTRL_REG2, ctrl_reg2);
	}
	else status = 1;

	return status;

}
/*==================================================================================================
FUNCTION: 		KIONIX_ACCEL_position_mask_fd
DESCRIPTION:    	This function masks face-down state in the screen rotation function. 
ARGUMENTS PASSED:   	None
RETURN VALUE:   	0 = pass; 1 = fail
PRE-CONDITIONS:   	KIONIX_ACCEL_init() has been called
POST-CONDITIONS:   	Face down state masked
==================================================================================================*/
int KIONIX_ACCEL_position_mask_fd(void)
{
	char ctrl_reg2 = 0;
	int status = 0;
	if (KIONIX_ACCEL_read_bytes(KIONIX_ACCEL_I2C_CTRL_REG2, &ctrl_reg2, 1) == 0){
		SET_REG_BIT(ctrl_reg2, CTRL_REG2_FDM);
		KIONIX_ACCEL_write_byte(KIONIX_ACCEL_I2C_CTRL_REG2, ctrl_reg2);
	}
	else status = 1;

	return status;

}
/*==================================================================================================
FUNCTION: 		KIONIX_ACCEL_position_mask_up
DESCRIPTION:    	This function masks up state in the screen rotation function. 
ARGUMENTS PASSED:   	None
RETURN VALUE:   	0 = pass; 1 = fail
PRE-CONDITIONS:   	KIONIX_ACCEL_init() has been called
POST-CONDITIONS:   	Up state masked
==================================================================================================*/
int KIONIX_ACCEL_position_mask_up(void)
{
	char ctrl_reg2 = 0;
	int status = 0;
	if (KIONIX_ACCEL_read_bytes(KIONIX_ACCEL_I2C_CTRL_REG2, &ctrl_reg2, 1) == 0){
		SET_REG_BIT(ctrl_reg2, CTRL_REG2_UPM);
		KIONIX_ACCEL_write_byte(KIONIX_ACCEL_I2C_CTRL_REG2, ctrl_reg2);
	}
	else status = 1;

	return status;

}
/*==================================================================================================
FUNCTION: 		KIONIX_ACCEL_position_mask_do
DESCRIPTION:    	This function masks down state in the screen rotation function. 
ARGUMENTS PASSED:   	None
RETURN VALUE:   	0 = pass; 1 = fail
PRE-CONDITIONS:   	KIONIX_ACCEL_init() has been called
POST-CONDITIONS:   	Down state masked
==================================================================================================*/
int KIONIX_ACCEL_position_mask_do(void)
{
	char ctrl_reg2 = 0;
	int status = 0;
	if (KIONIX_ACCEL_read_bytes(KIONIX_ACCEL_I2C_CTRL_REG2, &ctrl_reg2, 1) == 0){
		SET_REG_BIT(ctrl_reg2, CTRL_REG2_DOM);
		KIONIX_ACCEL_write_byte(KIONIX_ACCEL_I2C_CTRL_REG2, ctrl_reg2);
	}
	else status = 1;

	return status;

}
/*==================================================================================================
FUNCTION: 		KIONIX_ACCEL_position_mask_ri
DESCRIPTION:    	This function masks right state in the screen rotation function.
ARGUMENTS PASSED:   	None
RETURN VALUE:   	0 = pass; 1 = fail
PRE-CONDITIONS:   	KIONIX_ACCEL_init() has been called
POST-CONDITIONS:   	Right state masked
==================================================================================================*/
int KIONIX_ACCEL_position_mask_ri(void)
{
	char ctrl_reg2 = 0;
	int status = 0;
	if (KIONIX_ACCEL_read_bytes(KIONIX_ACCEL_I2C_CTRL_REG2, &ctrl_reg2, 1) == 0){
		SET_REG_BIT(ctrl_reg2, CTRL_REG2_RIM);
		KIONIX_ACCEL_write_byte(KIONIX_ACCEL_I2C_CTRL_REG2, ctrl_reg2);
	}
	else status = 1;

	return status;

}
/*==================================================================================================
FUNCTION: 		KIONIX_ACCEL_position_mask_le
DESCRIPTION:    	This function masks left state in the screen rotation function. 
ARGUMENTS PASSED:   	None
RETURN VALUE:   	0 = pass; 1 = fail
PRE-CONDITIONS:   	KIONIX_ACCEL_init() has been called
POST-CONDITIONS:   	Left state masked
==================================================================================================*/
int KIONIX_ACCEL_position_mask_le(void)
{
	char ctrl_reg2 = 0;
	int status = 0;
	if (KIONIX_ACCEL_read_bytes(KIONIX_ACCEL_I2C_CTRL_REG2, &ctrl_reg2, 1) == 0){
		SET_REG_BIT(ctrl_reg2, CTRL_REG2_LEM);
		KIONIX_ACCEL_write_byte(KIONIX_ACCEL_I2C_CTRL_REG2, ctrl_reg2);
	}
	else status = 1;

	return status;

}
/*==================================================================================================
FUNCTION: 		KIONIX_ACCEL_position_unmask_fu
DESCRIPTION:    	This function unmasks face-up state in the screen rotation function. 
ARGUMENTS PASSED:   	None
RETURN VALUE:   	0 = pass; 1 = fail
PRE-CONDITIONS:   	KIONIX_ACCEL_init() has been called
POST-CONDITIONS:   	Face up state unmasked
==================================================================================================*/
int KIONIX_ACCEL_position_unmask_fu(void)
{
	char ctrl_reg2 = 0;
	int status = 0;
	if (KIONIX_ACCEL_read_bytes(KIONIX_ACCEL_I2C_CTRL_REG2, &ctrl_reg2, 1) == 0){
		UNSET_REG_BIT(ctrl_reg2, CTRL_REG2_FUM);
		KIONIX_ACCEL_write_byte(KIONIX_ACCEL_I2C_CTRL_REG2, ctrl_reg2);
	}
	else status = 1;

	return status;

}
/*==================================================================================================
FUNCTION: 		KIONIX_ACCEL_position_unmask_fd
DESCRIPTION:    	This function unmasks face-down state in the screen rotation function. 
ARGUMENTS PASSED:   	None
RETURN VALUE:   	0 = pass; 1 = fail
PRE-CONDITIONS:   	KIONIX_ACCEL_init() has been called
POST-CONDITIONS:   	Face down state unmasked
==================================================================================================*/
int KIONIX_ACCEL_position_unmask_fd(void)
{
	char ctrl_reg2 = 0;
	int status = 0;
	if (KIONIX_ACCEL_read_bytes(KIONIX_ACCEL_I2C_CTRL_REG2, &ctrl_reg2, 1) == 0){
		UNSET_REG_BIT(ctrl_reg2, CTRL_REG2_FDM);
		KIONIX_ACCEL_write_byte(KIONIX_ACCEL_I2C_CTRL_REG2, ctrl_reg2);
	}
	else status = 1;

	return status;

}
/*==================================================================================================
FUNCTION: 		KIONIX_ACCEL_position_unmask_up
DESCRIPTION:    	This function unmasks up state in the screen rotation function. 
ARGUMENTS PASSED:   	None
RETURN VALUE:   	0 = pass; 1 = fail
PRE-CONDITIONS:   	KIONIX_ACCEL_init() has been called
POST-CONDITIONS:   	Up state unmasked
==================================================================================================*/
int KIONIX_ACCEL_position_unmask_up(void)
{
	char ctrl_reg2 = 0;
	int status = 0;
	if (KIONIX_ACCEL_read_bytes(KIONIX_ACCEL_I2C_CTRL_REG2, &ctrl_reg2, 1) == 0){
		UNSET_REG_BIT(ctrl_reg2, CTRL_REG2_UPM);
		KIONIX_ACCEL_write_byte(KIONIX_ACCEL_I2C_CTRL_REG2, ctrl_reg2);
	}
	else status = 1;

	return status;

}
/*==================================================================================================
FUNCTION: 		KIONIX_ACCEL_position_unmask_do
DESCRIPTION:    	This function unmasks down state in the screen rotation function. 
ARGUMENTS PASSED:   	None
RETURN VALUE:   	0 = pass; 1 = fail
PRE-CONDITIONS:   	KIONIX_ACCEL_init() has been called
POST-CONDITIONS:   	Down state unmasked
==================================================================================================*/
int KIONIX_ACCEL_position_unmask_do(void)
{
	char ctrl_reg2 = 0;
	int status = 0;
	if (KIONIX_ACCEL_read_bytes(KIONIX_ACCEL_I2C_CTRL_REG2, &ctrl_reg2, 1) == 0){
		UNSET_REG_BIT(ctrl_reg2, CTRL_REG2_DOM);
		KIONIX_ACCEL_write_byte(KIONIX_ACCEL_I2C_CTRL_REG2, ctrl_reg2);
	}
	else status = 1;

	return status;

}
/*==================================================================================================
FUNCTION: 		KIONIX_ACCEL_position_unmask_ri
DESCRIPTION:    	This function unmasks right state in the screen rotation function. 
ARGUMENTS PASSED:   	None
RETURN VALUE:   	0 = pass; 1 = fail
PRE-CONDITIONS:   	KIONIX_ACCEL_init() has been called
POST-CONDITIONS:   	Right state unmasked
==================================================================================================*/
int KIONIX_ACCEL_position_unmask_ri(void)
{
	char ctrl_reg2 = 0;
	int status = 0;
	if (KIONIX_ACCEL_read_bytes(KIONIX_ACCEL_I2C_CTRL_REG2, &ctrl_reg2, 1) == 0){
		UNSET_REG_BIT(ctrl_reg2, CTRL_REG2_RIM);
		KIONIX_ACCEL_write_byte(KIONIX_ACCEL_I2C_CTRL_REG2, ctrl_reg2);
	}
	else status = 1;

	return status;

}
/*==================================================================================================
FUNCTION: 		KIONIX_ACCEL_position_unmask_le
DESCRIPTION:    	This function unmasks left state in the screen rotation function. 
ARGUMENTS PASSED:   	None
RETURN VALUE:   	0 = pass; 1 = fail
PRE-CONDITIONS:   	KIONIX_ACCEL_init() has been called
POST-CONDITIONS:   	Left state unmasked
==================================================================================================*/
int KIONIX_ACCEL_position_unmask_le(void)
{
	char ctrl_reg2 = 0;
	int status = 0;
	if (KIONIX_ACCEL_read_bytes(KIONIX_ACCEL_I2C_CTRL_REG2, &ctrl_reg2, 1) == 0){
		UNSET_REG_BIT(ctrl_reg2, CTRL_REG2_LEM);
		KIONIX_ACCEL_write_byte(KIONIX_ACCEL_I2C_CTRL_REG2, ctrl_reg2);
	}
	else status = 1;

	return status;

}
/*==================================================================================================
FUNCTION: 		KIONIX_ACCEL_set_odr_motion
DESCRIPTION:    	This function sets the ODR frequency.
ARGUMENTS PASSED:   	frequency variable; 1, 3, 10, or 40 for KXTE9; 25, 50, 100, or 200 for KXTF9
RETURN VALUE:   	0 = pass; 1 = fail
PRE-CONDITIONS:   	KIONIX_ACCEL_init() has been called
POST-CONDITIONS:   	ODR is set according to frequency
==================================================================================================*/
int KIONIX_ACCEL_set_odr_motion(int frequency)
{
	char ctlreg_1 = 0;
	char ctlreg_3 = 0;	
	if (device == 0){	//KXTE9
		if (KIONIX_ACCEL_read_bytes(KIONIX_ACCEL_I2C_CTRL_REG1, &ctlreg_1, 1) != 0){
			return 1;
		} 
		if (KIONIX_ACCEL_read_bytes(KIONIX_ACCEL_I2C_CTRL_REG3, &ctlreg_3, 1) != 0){
			return 1;
		}
		switch (frequency){
		case 1:		/* set all ODR's to 1Hz */
			UNSET_REG_BIT(ctlreg_1, CTRL_REG1_ODRA);
			UNSET_REG_BIT(ctlreg_1, CTRL_REG1_ODRB);
			UNSET_REG_BIT(ctlreg_3, CTRL_REG3_OWUFA);
			UNSET_REG_BIT(ctlreg_3, CTRL_REG3_OWUFB);
			UNSET_REG_BIT(ctlreg_3, CTRL_REG3_OB2SA);
			UNSET_REG_BIT(ctlreg_3, CTRL_REG3_OB2SB);
			break;
		case 3:		/* set all ODR's to 3Hz */
			UNSET_REG_BIT(ctlreg_1, CTRL_REG1_ODRA);
			SET_REG_BIT(ctlreg_1, CTRL_REG1_ODRB);
			UNSET_REG_BIT(ctlreg_3, CTRL_REG3_OWUFA);
			SET_REG_BIT(ctlreg_3, CTRL_REG3_OWUFB);
			UNSET_REG_BIT(ctlreg_3, CTRL_REG3_OB2SA);
			SET_REG_BIT(ctlreg_3, CTRL_REG3_OB2SB);
			break;
		case 10:	/* set all ODR's to 10Hz */
			SET_REG_BIT(ctlreg_1, CTRL_REG1_ODRA);
			UNSET_REG_BIT(ctlreg_1, CTRL_REG1_ODRB);
			SET_REG_BIT(ctlreg_3, CTRL_REG3_OWUFA);
			UNSET_REG_BIT(ctlreg_3, CTRL_REG3_OWUFB);
			SET_REG_BIT(ctlreg_3, CTRL_REG3_OB2SA);
			UNSET_REG_BIT(ctlreg_3, CTRL_REG3_OB2SB);
			break;
		case 40:	/* set all ODR's to 40Hz */
			SET_REG_BIT(ctlreg_1, CTRL_REG1_ODRA);
			SET_REG_BIT(ctlreg_1, CTRL_REG1_ODRB);
			SET_REG_BIT(ctlreg_3, CTRL_REG3_OWUFA);
			SET_REG_BIT(ctlreg_3, CTRL_REG3_OWUFB);
			SET_REG_BIT(ctlreg_3, CTRL_REG3_OB2SA);
			SET_REG_BIT(ctlreg_3, CTRL_REG3_OB2SB);
			break;
		default:
			return 1;
		}
		KIONIX_ACCEL_write_byte(KIONIX_ACCEL_I2C_CTRL_REG1, ctlreg_1);
		KIONIX_ACCEL_write_byte(KIONIX_ACCEL_I2C_CTRL_REG3, ctlreg_3);
	}	
	else if (device == 1){	//KXTF9
		if (KIONIX_ACCEL_read_bytes(KIONIX_ACCEL_I2C_CTRL_REG3, &ctlreg_3, 1) != 0){
			return 1;
		}
		switch (frequency){
		case 25:	/* set ODR to 25Hz */
			UNSET_REG_BIT(ctlreg_3, CTRL_REG3_OWUFA);
			UNSET_REG_BIT(ctlreg_3, CTRL_REG3_OWUFB);
			break;
		case 50:	/* set ODR t0 50 Hz */
			UNSET_REG_BIT(ctlreg_3, CTRL_REG3_OWUFA);
			SET_REG_BIT(ctlreg_3, CTRL_REG3_OWUFB);
			break;
		case 100:	/* set ODR to 100 Hz */
			SET_REG_BIT(ctlreg_3, CTRL_REG3_OWUFA);
			UNSET_REG_BIT(ctlreg_3, CTRL_REG3_OWUFB);
			break;
		case 200:	/* set ODR to 200 Hz */
			SET_REG_BIT(ctlreg_3, CTRL_REG3_OWUFA);
			SET_REG_BIT(ctlreg_3, CTRL_REG3_OWUFB);
			break;
		default:
			return 1;
		}
		KIONIX_ACCEL_write_byte(KIONIX_ACCEL_I2C_CTRL_REG3, ctlreg_3);
	}
	return 0;
}


/*=============================================================================

FUNCTION
  KIONIX_ACCEL_PROCESS_screen_rotation

DESCRIPTION
  Process accelation sensor operation in Landscape mode

DEPENDENCIES

RETURN VALUE

SIDE EFFECTS

=============================================================================*/
void KIONIX_ACCEL_PROCESS_screen_rotation(unsigned char tilt_pos_pre, unsigned char tilt_pos_cur)
{
#if 0 //For_Resume_Speed
	switch(tilt_pos_cur)
	{
	case CTRL_REG2_RIM :  // X+
		printk("[%s:%d] ---I--- X+  Screen Roation : Landscape LEFT %x\n",__FUNCTION__,__LINE__,tilt_pos_cur);
		break;
	case CTRL_REG2_LEM :  // X-
		printk("[%s:%d] ---I--- X-  Screen Roation : Landscape RIGHT %x\n",__FUNCTION__,__LINE__,tilt_pos_cur);
		break;
	case CTRL_REG2_UPM : // Y+    
		printk("[%s:%d] ---I--- Y+  Screen Roation : Potrait UP %x\n",__FUNCTION__,__LINE__,tilt_pos_cur);
		break;
	case CTRL_REG2_DOM : // Y-  // ACCEL_SNAP_ROLL_RIGHT
		printk("[%s:%d] ---I--- Y-  Screen Roation : Potrait DOWN %x\n",__FUNCTION__,__LINE__,tilt_pos_cur);
		break;
	case CTRL_REG2_FUM : // Z+
		printk("[%s:%d] ---I--- Z+  Screen Roation : Face Down %x\n",__FUNCTION__,__LINE__,tilt_pos_cur);
		break;
	case CTRL_REG2_FDM : // Z- // ACCEL_SNAP_PITCH_DOWN
		printk("[%s:%d] ---I--- Z-  Screen Roation : Face Up %x\n",__FUNCTION__,__LINE__,tilt_pos_cur);
		break;
	}
#endif

// LGE_JUSTIN_S 20110119 nttaejun.cho@lge.com, motion sensor
//--[[ LGE_UBIQUIX_MODIFIED_START : shyun@ubiquix.com [2012.03.30] - CONFIG_PRODUCT_LGE_JUSTIN -> CONFIG_PRODUCT_LGE_LU6800
//#if defined(CONFIG_PRODUCT_LGE_JUSTIN)
#if defined(CONFIG_PRODUCT_LGE_LU6800)
//--]] LGE_UBIQUIX_MODIFIED_END : shyun@ubiquix.com [2012.03.30]- CONFIG_PRODUCT_LGE_JUSTIN -> CONFIG_PRODUCT_LGE_LU6800

#if 1
	if (tilt_pos_cur == 1)
 	{
 	   tilt_pos_cur = 2;
 	}
	else if (tilt_pos_cur == 2)
 	{
 	   tilt_pos_cur = 1;
 	}	
    else if (tilt_pos_cur == 4)
 	{
 	   tilt_pos_cur = 8;
 	}
	else if (tilt_pos_cur == 8)
 	{
 	   tilt_pos_cur = 4;
 	}

	if (tilt_pos_pre == 1)
 	{
 	   tilt_pos_pre = 2;
 	}
	else if (tilt_pos_pre == 2)
 	{
 	   tilt_pos_pre = 1;
 	}	
    else if (tilt_pos_pre == 4)
 	{
 	   tilt_pos_pre = 8;
 	}
	else if (tilt_pos_pre == 8)
 	{
 	   tilt_pos_pre = 4;
 	}

#endif 

#if 1 //For_Resume_Speed
		switch(tilt_pos_cur)
		{
		case CTRL_REG2_RIM :  // X+
			printk("[%s:%d] ---I--- X+	Screen Roation : Landscape LEFT %x %x\n",__FUNCTION__,__LINE__,tilt_pos_cur, tilt_pos_pre);
			break;
		case CTRL_REG2_LEM :  // X-
			printk("[%s:%d] ---I--- X-	Screen Roation : Landscape RIGHT %x %x\n",__FUNCTION__,__LINE__,tilt_pos_cur, tilt_pos_pre);
			break;
		case CTRL_REG2_UPM : // Y+	  
			printk("[%s:%d] ---I--- Y+	Screen Roation : Potrait UP %x %x\n",__FUNCTION__,__LINE__,tilt_pos_cur, tilt_pos_pre);
			break;
		case CTRL_REG2_DOM : // Y-	// ACCEL_SNAP_ROLL_RIGHT
			printk("[%s:%d] ---I--- Y-	Screen Roation : Potrait DOWN %x %x\n",__FUNCTION__,__LINE__,tilt_pos_cur, tilt_pos_pre);
			break;
		case CTRL_REG2_FUM : // Z+
			printk("[%s:%d] ---I--- Z+	Screen Roation : Face Down %x %x\n",__FUNCTION__,__LINE__,tilt_pos_cur, tilt_pos_pre);
			break;
		case CTRL_REG2_FDM : // Z- // ACCEL_SNAP_PITCH_DOWN
			printk("[%s:%d] ---I--- Z-	Screen Roation : Face Up %x %x\n",__FUNCTION__,__LINE__,tilt_pos_cur, tilt_pos_pre);
			break;
		}
#endif

#endif
// LGE_JUSTIN_E 20110119 nttaejun.cho@lge.com, motion sensor
	//if(tilt_pos_pre!=tilt_pos_cur) /*LGE_CHANGE_S_E [taehwan.kim@lge.com] 2010-04-01 */
	//{
		// TODO :  send event to application
		kxtf9_accel_send_screen_rotation_event((int)tilt_pos_cur, (int)tilt_pos_pre);
	//}
	
	return ;
}

/*=============================================================================

FUNCTION
  KIONIX_ACCEL_PROCESS_directional_tap

DESCRIPTION
  Process accelation sensor operation in directionaltap mode

DEPENDENCIES

RETURN VALUE

SIDE EFFECTS

=============================================================================*/
void KIONIX_ACCEL_PROCESS_directional_tap(unsigned char tap_mode, unsigned char tap_direction)
{
	int type =0,direction =0;

	if(tap_mode == 2)
	     type = ACCEL_TAP_MODE_DOUBLE; 
	else
	     type = ACCEL_TAP_MODE_SINGLE; 	

// LGE_JUSTIN_S 20101216 nttaejun.cho@lge.com, motion sensor
//#if defined(CONFIG_PRODUCT_LGE_JUSTIN)
#if defined(CONFIG_PRODUCT_LGE_LU6800)
	switch(tap_direction)
	{
	case INT_CTRL_REG3_TFDM : // Z-	
		direction = ACCEL_TAP_FRONT;//ACCEL_TAP_BACK;

		if(tap_mode == 2)		printk("[%s:%d] ---I--- Z+	Directional-Tap  : (2) Front \n",__FUNCTION__,__LINE__);  
		else					printk("[%s:%d] ---I--- Z+	Directional-Tap  : (1) Front \n",__FUNCTION__,__LINE__);	
		break;
	case INT_CTRL_REG3_TFUM : // Z+
		   direction = ACCEL_TAP_BACK;//ACCEL_TAP_FRONT;

		if(tap_mode == 2)		printk("[%s:%d] ---I--- Z-	Directional-Tap  : (2) Back \n",__FUNCTION__,__LINE__);
		else				    printk("[%s:%d] ---I--- Z-	Directional-Tap  : (1) Back \n",__FUNCTION__,__LINE__); 
		break;
	case INT_CTRL_REG3_TDOM : // Y-	
		direction = ACCEL_TAP_LEFT;

		if(tap_mode == 2)		printk("[%s:%d] ---I--- Y+	Directional-Tap  : (2) Left \n",__FUNCTION__,__LINE__); 
		else					printk("[%s:%d] ---I--- Y+	Directional-Tap  : (1) Left \n",__FUNCTION__,__LINE__);
		break;
	case INT_CTRL_REG3_TUPM : // Y+
		direction = ACCEL_TAP_RIGHT;

		if(tap_mode == 2)		printk("[%s:%d] ---I--- Y-	Directional-Tap  : (2) Right \n",__FUNCTION__,__LINE__);  
		else					printk("[%s:%d] ---I--- Y-	Directional-Tap  : (1) Right  \n",__FUNCTION__,__LINE__);  
		break;
	case INT_CTRL_REG3_TLEM : // X-
		direction = ACCEL_TAP_UP;//ACCEL_TAP_DOWN;		
		if(tap_mode == 2)		printk("[%s:%d] ---I--- X+	Directional-Tap  : (2) Down \n",__FUNCTION__,__LINE__);  
		else					printk("[%s:%d] ---I--- X+	Directional-Tap  : (1) Down \n",__FUNCTION__,__LINE__);  
		break;
	case INT_CTRL_REG3_TRIM : // X+	
		direction = ACCEL_TAP_DOWN;//ACCEL_TAP_UP;
		if(tap_mode == 2)		printk("[%s:%d] ---I--- X-	Directional-Tap  : (2) Up \n",__FUNCTION__,__LINE__);  
		else					printk("[%s:%d] ---I--- X-	Directional-Tap  : (1) Up \n",__FUNCTION__,__LINE__);  
		break;
	}

#else

	switch(tap_direction)
	{
	case INT_CTRL_REG3_TFUM : // Z+
		direction = ACCEL_TAP_FRONT;//ACCEL_TAP_BACK;

		if(tap_mode == 2)		printk("[%s:%d] ---I--- Z+	Directional-Tap  : (2) Front \n",__FUNCTION__,__LINE__);  
		else					printk("[%s:%d] ---I--- Z+	Directional-Tap  : (1) Front \n",__FUNCTION__,__LINE__); 	
		break;
	case INT_CTRL_REG3_TFDM : // Z-
		   direction = ACCEL_TAP_BACK;//ACCEL_TAP_FRONT;

		if(tap_mode == 2)		printk("[%s:%d] ---I--- Z-	Directional-Tap  : (2) Back \n",__FUNCTION__,__LINE__);
		else				printk("[%s:%d] ---I--- Z-	Directional-Tap  : (1) Back \n",__FUNCTION__,__LINE__); 
		break;
	case INT_CTRL_REG3_TUPM : // Y+
		direction = ACCEL_TAP_LEFT;

		if(tap_mode == 2)		printk("[%s:%d] ---I--- Y+	Directional-Tap  : (2) Left \n",__FUNCTION__,__LINE__); 
		else					printk("[%s:%d] ---I--- Y+	Directional-Tap  : (1) Left \n",__FUNCTION__,__LINE__);
		break;
	case INT_CTRL_REG3_TDOM : // Y-
		direction = ACCEL_TAP_RIGHT;

		if(tap_mode == 2)		printk("[%s:%d] ---I--- Y-	Directional-Tap  : (2) Right \n",__FUNCTION__,__LINE__);  
		else					printk("[%s:%d] ---I--- Y-	Directional-Tap  : (1) Right  \n",__FUNCTION__,__LINE__);  
		break;
	case INT_CTRL_REG3_TRIM : // X+
		direction = ACCEL_TAP_DOWN;//ACCEL_TAP_UP;

		if(tap_mode == 2)		printk("[%s:%d] ---I--- X+	Directional-Tap  : (2) Down \n",__FUNCTION__,__LINE__);  
		else					printk("[%s:%d] ---I--- X+	Directional-Tap  : (1) Down \n",__FUNCTION__,__LINE__);  
		break;
	case INT_CTRL_REG3_TLEM : // X-
		direction = ACCEL_TAP_UP;//ACCEL_TAP_DOWN;

		if(tap_mode == 2)		printk("[%s:%d] ---I--- X-	Directional-Tap  : (2) Up \n",__FUNCTION__,__LINE__);  
		else					printk("[%s:%d] ---I--- X-	Directional-Tap  : (1) Up \n",__FUNCTION__,__LINE__);  
		break;
	}
#endif 
// LGE_JUSTIN_E 20101216 nttaejun.cho@lge.com, motion sensor

	#if 1 //magoo
	motion_send_tap_detection(type, direction);
	#else
	// TODO : send event to application
	kxtf9_accel_send_tap_event(type,direction);
	#endif
	return ;
	
}



/*==================================================================================================
FUNCTION: 		KIONIX_ACCEL_service_interrupt
DESCRIPTION:    	This function clears the interrupt request status. 
ARGUMENTS PASSED:   	source_of_interrupt pointer
RETURN VALUE:   	0 = interrupt was pending; 1 = interrupt was not pending
PRE-CONDITIONS:   	KIONIX_ACCEL_init() has been called
POST-CONDITIONS:   	source_of_interrupt is assigned
			Interrupt pending bit (MOTI in REGA) will be cleared
IMPORTANT NOTES:	Do not call this from interrupt context since it accesses i2c.
==================================================================================================*/
int  KIONIX_ACCEL_service_interrupt(void)
{
	unsigned char status_reg=0, int_rel=0;
	unsigned char int_src_reg1, int_src_reg2;
	unsigned char tilt_pos_cur, tilt_pos_pre;

	// disable accelerometer interrupt first
	KIONIX_ACCEL_disable_interrupt();

	KIONIX_ACCEL_read_bytes(KIONIX_ACCEL_I2C_STATUS_REG, &status_reg, 1);
#if DEBUG
    printk("KIONIX_ACCEL_I2C_STATUS_REG = %x ",status_reg);
#endif //DEBUG
#ifndef BIT
#	define BIT(x)	(1<<x)
#endif
	if(!(status_reg & BIT(4)) ) {
		goto RELEASE_INT;
	}

	KIONIX_ACCEL_read_bytes(KXTF9_I2C_INT_SRC_REG1, &int_src_reg1, 1);
	KIONIX_ACCEL_read_bytes(KXTF9_I2C_INT_SRC_REG2, &int_src_reg2, 1);
#if DEBUG
    printk("KXTF9_I2C_INT_SRC_REG1 = %x ",int_src_reg1);
    printk("KXTF9_I2C_INT_SRC_REG2 = %x \n",int_src_reg2);
#endif //DEBUG
	if((int_src_reg2 & (0x3<<2)) && is_tap_enabled() ) { // Direction tap
		unsigned char tap_mode ;
		tap_mode = ((int_src_reg2&(0x3<<2))>>2);
				
		//   Processing Directional-Tap 
		KIONIX_ACCEL_PROCESS_directional_tap(tap_mode, int_src_reg1);
		
	}
    if((int_src_reg2 & BIT(0)) && (is_flip_enabled() || is_snap_enabled())) { // TPS : Screen Rotation
		KIONIX_ACCEL_read_bytes(KIONIX_ACCEL_I2C_TILT_POS_CUR, &tilt_pos_cur, 1);
		KIONIX_ACCEL_read_bytes(KIONIX_ACCEL_I2C_TILT_POS_PRE, &tilt_pos_pre, 1);
#if DEBUG
		printk("KIONIX_ACCEL_I2C_TILT_POS_CUR = %x KIONIX_ACCEL_I2C_TILT_POS_PRE = %x \n",tilt_pos_cur,tilt_pos_pre); 
#endif
	    KIONIX_ACCEL_PROCESS_screen_rotation(tilt_pos_pre, tilt_pos_cur);
	}

	if(int_src_reg2 & BIT(1)) { // Motion Detection
		// TODO:
	}

RELEASE_INT:
       KIONIX_ACCEL_read_bytes(KIONIX_ACCEL_I2C_INT_REL, &int_rel, 1);
       // enable accelerometer interrupt again
       KIONIX_ACCEL_enable_interrupt();

	return 0; 
	
}

#if 1 // yj

int kxtf9_get_hwaccel_data(int *xyz)
{
	int err;
	u8 acc_data[6]; // xyz data bytes from hardware
	char buf, Res = 0, G_range = 0;
	int range = 0, sensitivity;
	int x_sign, y_sign, z_sign;
	int hw_d[3]; // count values
	int xyz_cnt[3]; // count values  e.g. range is 0~4095 @ 2g, 12-bit

	err = KIONIX_ACCEL_read_bytes(KIONIX_ACCEL_I2C_CTRL_REG1, &buf, 1);
	if(err < 0 ) {
		printk("can't get acceleration data, err=%d\n", err);
		return err;
	}
	G_range = (buf & 0x18) >> 3;
	switch(G_range)
	{
	case 0:
		range = 2;
		break;
	case 1:
		range = 4;
		break;
	case 2:
		range = 8;
		break;
	default:
		break;
	}

	err = KIONIX_ACCEL_read_bytes(KXTF9_I2C_XOUT_L, &acc_data[0], 6);
	if(err < 0){
		printk("can't get acceleration data, err=%d\n", err);
		return err;
	}

	Res = buf & 0x40;
	switch(Res)
	{
	case 0x00: // 8-bit : low resolution state
		xyz_cnt[0] = hw_d[0] = ((int)acc_data[1]);
		xyz_cnt[0] = (xyz_cnt[0]>>7) ? (xyz_cnt[0]-128) : (xyz_cnt[0]+128) ;
		xyz_cnt[0] = (negate_x) ? (256 - xyz_cnt[0]) : (xyz_cnt[0]);
		x_sign = hw_d[0] >> 7;	//1 = negative; 0 = positive
		if (x_sign == 1){
			hw_d[0] = ((~(hw_d[0]) + 0x01) & 0x0FF);
			hw_d[0] = -(hw_d[0]);
		}
		xyz_cnt[1] = hw_d[1] = ((int)acc_data[3]);
		xyz_cnt[1] = (xyz_cnt[1]>>7) ? (xyz_cnt[1]-128) : (xyz_cnt[1]+128) ;
		xyz_cnt[1] = (negate_y) ? (256 - xyz_cnt[1]) : (xyz_cnt[1]);
		y_sign = hw_d[1] >> 7;	//1 = negative; 0 = positive
		if (y_sign == 1){
			hw_d[1] = ((~(hw_d[1]) + 0x01) & 0x0FF);	//2's complement
			hw_d[1] = -(hw_d[1]);
		}
		xyz_cnt[2] = hw_d[2] = ((int)acc_data[5]);
		xyz_cnt[2] = (xyz_cnt[2]>>7) ? (xyz_cnt[2]-128) : (xyz_cnt[2]+128) ;
		xyz_cnt[2] = (negate_z) ? (256 - xyz_cnt[2]) : (xyz_cnt[2]);
		z_sign = hw_d[2] >> 7;	//1 = negative; 0 = positive
		if (z_sign == 1){
			hw_d[2] = ((~(hw_d[2]) + 0x01) & 0x0FF);	//2's complement
			hw_d[2] = -(hw_d[2]);						
		}
		sensitivity = (256)/(2*range);
		/* calculate milli-G's */
		hw_d[0] = 1000 * (hw_d[0]) / sensitivity; 
		hw_d[1] = 1000 * (hw_d[1]) / sensitivity; 
		hw_d[2] = 1000 * (hw_d[2]) / sensitivity;
		break;
	case 0x40: // 12-bit : high-resolution state
		hw_d[0] = ((int)acc_data[0]) >> 4;
		xyz_cnt[0] = hw_d[0] = hw_d[0] + (((int)acc_data[1]) << 4);
		xyz_cnt[0] = (xyz_cnt[0]>>11) ? (xyz_cnt[0]-2048) : (xyz_cnt[0]+2048) ;
		xyz_cnt[0] = (negate_x) ? (4096 - xyz_cnt[0]) : (xyz_cnt[0]);
		x_sign = hw_d[0] >> 11; 	//1 = negative; 0 = positive
		if (x_sign == 1){
			hw_d[0] = ((~(hw_d[0]) + 0x01) & 0x0FFF);	//2's complement
			hw_d[0] = -(hw_d[0]);
		}
		hw_d[1] = ((int)acc_data[2]) >> 4;
		xyz_cnt[1] = hw_d[1]  = hw_d[1]  + (((int)acc_data[3]) << 4);
		xyz_cnt[1] = (xyz_cnt[1]>>11) ? (xyz_cnt[1]-2048) : (xyz_cnt[1]+2048) ;
		xyz_cnt[1] = (negate_y) ? (4096 - xyz_cnt[1]) : (xyz_cnt[1]);
		y_sign = hw_d[1]  >> 11; 	//1 = negative; 0 = positive
		if (y_sign == 1){
			hw_d[1]  = ((~(hw_d[1] ) + 0x01) & 0x0FFF);	//2's complement
			hw_d[1]  = -(hw_d[1] );
		}
		hw_d[2] = ((int)acc_data[4]) >> 4;
		xyz_cnt[2] = hw_d[2] = hw_d[2] + (((int)acc_data[5]) << 4);
		xyz_cnt[2] = (xyz_cnt[2]>>11) ? (xyz_cnt[2]-2048) : (xyz_cnt[2]+2048) ;
		xyz_cnt[2] = (negate_z) ? (4096 - xyz_cnt[2]) : (xyz_cnt[2]);
		z_sign = hw_d[2] >> 11; 	//1 = negative; 0 = positive
		if (z_sign == 1){
			hw_d[2] = ((~(hw_d[2]) + 0x01) & 0x0FFF);	//2's complement
			hw_d[2] = -(hw_d[2]);
		}
		sensitivity = (4096)/(2*range);
		/* calculate milli-G's */
		hw_d[0] = 1000 * (hw_d[0]) / sensitivity; 
		hw_d[1] = 1000 * (hw_d[1]) / sensitivity; 
		hw_d[2] = 1000 * (hw_d[2]) / sensitivity;
		break;
	}

#if 0 // mg values
	xyz[0] = ((tf9->pdata->negate_x) ? (-hw_d[tf9->pdata->axis_map_x]) : (hw_d[tf9->pdata->axis_map_x]));
	xyz[1] = ((tf9->pdata->negate_y) ? (-hw_d[tf9->pdata->axis_map_y]) : (hw_d[tf9->pdata->axis_map_y]));
	xyz[2] = ((tf9->pdata->negate_z) ? (-hw_d[tf9->pdata->axis_map_z]) : (hw_d[tf9->pdata->axis_map_z]));

	//dev_info(&tf9->client->dev, "x:%d y:%d z:%d \n", xyz[0], xyz[1], xyz[2] );
#else // count value
	xyz[0] = xyz_cnt[axis_map_x];
	xyz[1] = xyz_cnt[axis_map_y];
	xyz[2] = xyz_cnt[axis_map_z];

	//dev_info(&tf9->client->dev, "x:%d y:%d z:%d  \n", xyz[0], xyz[1], xyz[2] );
#endif
	return err;
}

int kxtf9_set_calibration(void)
{
	int err;
	int i;
	char buf, Res = 0, G_range = 0;
	int range = 0, sensitivity;

	long Xaccum=0, Yaccum=0, Zaccum=0;
	int xyz[3] = {0 };

	err = KIONIX_ACCEL_read_bytes(KIONIX_ACCEL_I2C_CTRL_REG1, &buf, 1);
	if(err < 0 ) {
		printk("can't get acceleration data, err=%d\n", err);
		return err;
	}
	G_range = (buf & 0x18) >> 3;
	switch(G_range)
	{
	case 0:		range = 2;		break;
	case 1:		range = 4;		break;
	case 2:		range = 8;		break;
	default:		break;
	}
	Res = buf & 0x40;
	switch(Res)
	{
	case 0x00: // 8-bit : low resolution state
		sensitivity = (256)/(2*range);
		break;
	case 0x40: // 12-bit : high-resolution state
		sensitivity = (4096)/(2*range);
		break;
	}

	// Average 100 samples.
	for(i=0; i<100; i++)
	{
		err = kxtf9_get_hwaccel_data(&xyz[0]);
		if(err < 0)
			return err;

		Xaccum += xyz[axis_map_x];
		Yaccum += xyz[axis_map_y];
		Zaccum += xyz[axis_map_z];
	}

	//printk("[%d] Xaccum = %ld , Yaccum = %ld, Zaccum = %ld, Res=%x, sensitivity=%d \n", __LINE__,
	//		Xaccum, Yaccum, Zaccum , Res, sensitivity);

	oca[0] = (Res==0x40 ? 2048 : 256) - (Xaccum/100);
	oca[1] = (Res==0x40 ? 2048 : 256) - (Yaccum/100);
	oca[2] = ((Res==0x40 ? 2048 : 256)+sensitivity) - (Zaccum/100);

	oca_enabled = 1;
	printk("-YJ- oca_enabled = %d, OCAX:%d OCAY:%d OCAZ:%d  \n", oca_enabled, oca[0], oca[1], oca[2] );

	return err;
}

int kxtf9_get_xyz(int *xyz)
{
	int err;

	err = kxtf9_get_hwaccel_data(&xyz[0]);
	if(err < 0)
		return err;

	if(oca_enabled == 1 )
	{
		xyz[0] += oca[0];
		xyz[1] += oca[1];
		xyz[2] += oca[2];
	}

	return 0;
}
#endif
/*==================================================================================================
FUNCTION: 		KIONIX_ACCEL_read_LPF_cnt
DESCRIPTION:    	This function reads the number of counts on the X, Y, and Z axes.
ARGUMENTS PASSED:   	x, y, and z pointers
RETURN VALUE:   	0 = pass; 1 = fail
PRE-CONDITIONS:   	KIONIX_ACCEL_init() has been called
POST-CONDITIONS:   	x, y, and z are assigned
==================================================================================================*/
int KIONIX_ACCEL_read_LPF_cnt(int* x, int* y, int* z)
{
	int status =0, x_sign, y_sign, z_sign;
	char Res;
	char ret[3] = {0, 0, 0};
	unsigned char xyz[6] = {0, 0, 0, 0, 0, 0};
	
	if (device == 0){	//KXTE9	
		status = KIONIX_ACCEL_read_bytes(KXTE9_I2C_XOUT, ret, 3);
		if(status == 0){
			*x = (int)(ret[0]) >> 2;
			*y = (int)(ret[1]) >> 2;
			*z = (int)(ret[2]) >> 2;
		}
		else status = 1;
	}
	else if (device == 1){	//KXTF9
		if (KIONIX_ACCEL_read_bytes(KIONIX_ACCEL_I2C_CTRL_REG1, &Res, 1) == 0){
			Res = Res & 0x40;
			switch(Res){
				case 0x00:	//low-resolution state
					if ((status = KIONIX_ACCEL_read_bytes(KXTF9_I2C_XOUT_L, xyz, 6)) == 0){
						*x = ((int)xyz[1]);
						x_sign = *x >> 7;	//1 = negative; 0 = positive
						if (x_sign == 1){
							*x = ((~(*x) + 0x01) & 0x0FF);
							*x = -(*x);
						}
						*y = ((int)xyz[3]);
						y_sign = *y >> 7;	//1 = negative; 0 = positive
						if (y_sign == 1){
							*y = ((~(*y) + 0x01) & 0x0FF);	//2's complement
							*y = -(*y);
						}
						*z = ((int)xyz[5]);
						z_sign = *z >> 7;	//1 = negative; 0 = positive
						if (z_sign == 1){
							*z = ((~(*z) + 0x01) & 0x0FF);	//2's complement
							*z = -(*z);						
						}
					}
					break;
				case 0x40:	//high-resolution state
					if ((status = KIONIX_ACCEL_read_bytes(KXTF9_I2C_XOUT_L, xyz, 6)) == 0){				
						*x = ((int)xyz[0]) >> 4;
						*x = *x + (((int)xyz[1]) << 4);
						x_sign = *x >> 11; 	//1 = negative; 0 = positive
						if (x_sign == 1){
							*x = ((~(*x) + 0x01) & 0x0FFF);	//2's complement
							*x = -(*x);
						}
						*y = ((int)xyz[2]) >> 4;
						*y = *y + (((int)xyz[3]) << 4);
						y_sign = *y >> 11; 	//1 = negative; 0 = positive
						if (y_sign == 1){
							*y = ((~(*y) + 0x01) & 0x0FFF);	//2's complement
							*y = -(*y);
						}
						*z = ((int)xyz[4]) >> 4;
						*z = *z + (((int)xyz[5]) << 4);
						z_sign = *z >> 11; 	//1 = negative; 0 = positive
						if (z_sign == 1){
							*z = ((~(*z) + 0x01) & 0x0FFF);	//2's complement
							*z = -(*z);
						}
					}
					break;
				default:
					printk("KIONIX_ACCEL_read_LPF_cnt: NO TYPE\n");
					break;
			}		
		}
	}
	else status = 1;

	return status;

}
/*==================================================================================================
FUNCTION: 		KIONIX_ACCEL_read_LPF_g
DESCRIPTION:    	This function reads the G(gravity force) values on the X, Y, and Z axes.
			The units used are milli-g's, or 1/1000*G. 
ARGUMENTS PASSED:   	gx, gy, and gz pointers
RETURN VALUE:   	0 = pass; 1 = fail
PRE-CONDITIONS:   	KIONIX_ACCEL_init() has been called
POST-CONDITIONS:   	gx, gy, and gz are assigned
==================================================================================================*/
int KIONIX_ACCEL_read_LPF_g(int* gx, int* gy, int* gz)
{
	int status =0, sensitivity;
	int x = 0;
	int y = 0;
	int z = 0;
	int x_sign, y_sign, z_sign;
	char xyz[6] = {0, 0, 0, 0, 0, 0};
	char Res = 0;
	char G_range = 0;
	int range = 0;
	if (device == 0){	//KXTE9	
		sensitivity = BIT_SENSITIVITY_2_G;		
		if ((status = KIONIX_ACCEL_read_LPF_cnt(&x, &y, &z)) == 0){
			/* calculate milli-G's */
			*gx = 1000 * (x - ZERO_G_OFFSET) / sensitivity; 
			*gy = 1000 * (y - ZERO_G_OFFSET) / sensitivity; 
			*gz = 1000 * (z - ZERO_G_OFFSET) / sensitivity;
		}
		else status = 1;
	}
	else if (device == 1){	//KXTF9
		//determine if in the low resolution or high resolution state
		if (KIONIX_ACCEL_read_bytes(KIONIX_ACCEL_I2C_CTRL_REG1, &Res, 1) == 0){
			if (KIONIX_ACCEL_read_bytes(KIONIX_ACCEL_I2C_CTRL_REG1, &G_range, 1) == 0){
				G_range = G_range & 0x18;
				G_range = G_range >> 3;
				switch(G_range){
					case 0:
						range = 2;
						break;
					case 1:
						range = 4;
						break;
					case 2:
						range = 8;
						break;
					default:
						break;
				}
				Res = Res & 0x40;
				switch(Res){
					case 0x00:	//low-resolution state
						if ((status = KIONIX_ACCEL_read_bytes(KXTF9_I2C_XOUT_L, xyz, 6)) == 0){
							x = ((int)xyz[1]);
							x_sign = x >> 7;	//1 = negative; 0 = positive
							if (x_sign == 1){
								x = ((~(x) + 0x01) & 0x0FF);
								x = -(x);
							}
							y = ((int)xyz[3]);
							y_sign = y >> 7;	//1 = negative; 0 = positive
							if (y_sign == 1){
								y = ((~(y) + 0x01) & 0x0FF);	//2's complement
								y = -(y);
							}
							z = ((int)xyz[5]);
							z_sign = z >> 7;	//1 = negative; 0 = positive
							if (z_sign == 1){
								z = ((~(z) + 0x01) & 0x0FF);	//2's complement
								z = -(z);						
							}
							sensitivity = (256)/(2*range);
							/* calculate milli-G's */
							*gx = 1000 * (x) / sensitivity; 
							*gy = 1000 * (y) / sensitivity; 
							*gz = 1000 * (z) / sensitivity;
						}
						break;
					case 0x40:	//high-resolution state
						if ((status = KIONIX_ACCEL_read_bytes(KXTF9_I2C_XOUT_L, xyz, 6)) == 0){
							x = ((int)xyz[0]) >> 4;
							x = x + (((int)xyz[1]) << 4);
							x_sign = x >> 11; 	//1 = negative; 0 = positive
							if (x_sign == 1){
								x = ((~(x) + 0x01) & 0x0FFF);	//2's complement
								x = -(x);
							}
							y = ((int)xyz[2]) >> 4;
							y = y + (((int)xyz[3]) << 4);
							y_sign = y >> 11;	//1 = negative; 0 = positive
							if (y_sign == 1){
								y = ((~(y) + 0x01) & 0x0FFF);	//2's complement
								y = -(y);
							}
							z = ((int)xyz[4]) >> 4;
							z = z + (((int)xyz[5]) << 4);
							z_sign = z >> 11;	//1 = negative; 0 = positive
							if (z_sign == 1){
								z = ((~(z) + 0x01) & 0x0FFF);	//2's complement
								z = -(z);
							}
							sensitivity = (4096)/(2*range);

							#if 1   // original test
							/* calculate milli-G's */
							*gx = 1000 * (x) / sensitivity; 
							*gy = 1000 * (y) / sensitivity; 
							*gz = 1000 * (z) / sensitivity;
							#endif  

							#if 0
							 /* for JSR256 API - chanhee.park@lge.com */
							*gx = x;  
							*gy = y;  
							*gz = z; 
							#endif
							
						}
						break;
					default:
						break;
				}
			}
			else	 
				status = 1;
		}
		else	 
			status = 1;
	}
	else	 
		status = 1;
	return status;
}

/*==================================================================================================
FUNCTION: 		KIONIX_ACCEL_read_current_odr_motion
DESCRIPTION:    	This function reads the current ODR of the general motion function. 
ARGUMENTS PASSED:   	ODR_rate_motion pointer
RETURN VALUE:   	0 = ODR set correctly; 1 = ODR invalid
PRE-CONDITIONS:   	KIONIX_ACCEL_init() has been called
POST-CONDITIONS:   	ODR_rate_motion is assigned
==================================================================================================*/
int KIONIX_ACCEL_read_current_odr_motion(double* ODR_rate_motion)
{
	int status;
	char status_reg, ctrl_reg;
	if (device == 0){	//KXTE9	
		if (KIONIX_ACCEL_read_bytes(KIONIX_ACCEL_I2C_STATUS_REG, &status_reg, 1) == 0){
			status_reg &= 0x0C;
			status_reg >>= 2;
			switch (status_reg){
			case 0:
				*ODR_rate_motion = 1;
				status = 0;
				break;
			case 1:
				*ODR_rate_motion = 3;
				status = 0;
				break;
			case 2:
				*ODR_rate_motion = 10;
				status = 0;
				break;
			case 3:
				*ODR_rate_motion = 40;
				status = 0;
				break;
			default:
				status = 1;
				break;
			}
		}
		else	 {
			status = 1;
		}
	}
	else if (device == 1){	//KXTF9
		if (KIONIX_ACCEL_read_bytes(KIONIX_ACCEL_I2C_CTRL_REG3, &ctrl_reg, 1) == 0){
			ctrl_reg &= 0x03;
			ctrl_reg >>= 2;
			switch (ctrl_reg){
			case 0:
				*ODR_rate_motion = 25;
				status = 0;
				break;
			case 1:
				*ODR_rate_motion = 50;
				status = 0;
				break;
			case 2:
				*ODR_rate_motion = 100;
				status = 0;
				break;
			case 3:
				*ODR_rate_motion = 200;
				status = 0;
				break;
			default:
				status = 1;
				break;
			}
		}
		else	 {
			status = 1;
		}
	}
	else	 {
		status = 1;
	}
	return status;
}
/*==================================================================================================
FUNCTION: 		KIONIX_ACCEL_read_position_status
DESCRIPTION:    	This function reads INT_SRC_REG to determine if there was a change in tilt. 
ARGUMENTS PASSED:   	None
RETURN VALUE:   	0 = tilt occurred; 1 = no tilt occurred
PRE-CONDITIONS:   	KIONIX_ACCEL_init() has been called
POST-CONDITIONS:   	None
==================================================================================================*/
int KIONIX_ACCEL_read_position_status(void)
{
	int position_status; 
	char src_reg1, src_reg2;
	if (device == 0){	//KXTE9
		if (KIONIX_ACCEL_read_bytes(KXTE9_I2C_INT_SRC_REG1, &src_reg1, 1) == 0){
			if((src_reg1 & 0x01) == 1){
				position_status = 0;
			}
			else	position_status = 1;
		}
		else	position_status = 1;
	}
	else if (device == 1){	//KXTF9
		if (KIONIX_ACCEL_read_bytes(KXTF9_I2C_INT_SRC_REG2, &src_reg2, 1) == 0){
			if((src_reg2 & 0x01) == 1){
				position_status = 0;
			}
			else	position_status = 1;
		}
		else	position_status = 1;
	}
	else position_status = 1;
	return position_status;
}
/*==================================================================================================
FUNCTION: 		KIONIX_ACCEL_read_wuf_status
DESCRIPTION:    	This function reads INT_SRC_REG1 to determine if wake up occurred. 
ARGUMENTS PASSED:   	None
RETURN VALUE:   	0 = wake up occurred; 1 = no wake up occurred
PRE-CONDITIONS:   	KIONIX_ACCEL_init() has been called
POST-CONDITIONS:   	None
==================================================================================================*/
int KIONIX_ACCEL_read_wuf_status(void)
{
	int wuf_status; 
	char src_reg1, src_reg2;
	if (device == 0){	//KXTE9	
		if (KIONIX_ACCEL_read_bytes(KXTE9_I2C_INT_SRC_REG1, &src_reg1, 1) == 0){
			if((src_reg1 & 0x02) == 0x02){
				wuf_status = 0;
			}
			else	wuf_status = 1;
		}
		else	wuf_status = 1;
	}
	else if (device == 1){	//KXTF9	
		if (KIONIX_ACCEL_read_bytes(KXTF9_I2C_INT_SRC_REG2, &src_reg2, 1) == 0){
			if((src_reg2 & 0x02) == 0x02){
				wuf_status = 0;
			}
			else	wuf_status = 1;
		}
		else	wuf_status = 1;
	}
	else wuf_status = 1;
	return wuf_status;
}
/*==================================================================================================
FUNCTION: 		KIONIX_ACCEL_enable_int
DESCRIPTION:    	This function enables the physical interrupt. 
ARGUMENTS PASSED:   	None
RETURN VALUE:   	0 = pass; 1 = fail
PRE-CONDITIONS:   	KIONIX_ACCEL_init() has been called
POST-CONDITIONS:   	Physical interrupt enabled
==================================================================================================*/
int KIONIX_ACCEL_enable_int(void)
{
	char int_ctrl_reg1 = 0;
	int status = 0;
	
	if (device == 0){	//KXTE9
		if (KIONIX_ACCEL_read_bytes(KXTE9_I2C_INT_CTRL_REG1, &int_ctrl_reg1, 1) == 0){
			SET_REG_BIT(int_ctrl_reg1, KXTE9_INT_CTRL_REG1_IEN);
			KIONIX_ACCEL_write_byte(KXTE9_I2C_INT_CTRL_REG1, int_ctrl_reg1);
		}
		else	 {
			status = 1;
		}
	}
	else if (device == 1){	//KXTF9
		if (KIONIX_ACCEL_read_bytes(KXTF9_I2C_INT_CTRL_REG1, &int_ctrl_reg1, 1) == 0){
			SET_REG_BIT(int_ctrl_reg1, KXTF9_INT_CTRL_REG1_IEN);
			KIONIX_ACCEL_write_byte(KXTF9_I2C_INT_CTRL_REG1, int_ctrl_reg1);
		}
		else	 {
			status = 1;
		}
	}
	else status = 1;

	return status;
}
/*==================================================================================================
FUNCTION: 		KIONIX_ACCEL_disable_int
DESCRIPTION:    	This function disables the physical interrupt. 
ARGUMENTS PASSED:   	None
RETURN VALUE:   	0 = pass; 1 = fail
PRE-CONDITIONS:   	KIONIX_ACCEL_init() has been called
POST-CONDITIONS:   	Physical interrupt disabled
==================================================================================================*/
int KIONIX_ACCEL_disable_int(void)
{
	char int_ctrl_reg1 = 0;
	int status = 0;
	
	if (device == 0){	//KXTE9	
		if (KIONIX_ACCEL_read_bytes(KXTE9_I2C_INT_CTRL_REG1, &int_ctrl_reg1, 1) == 0){
			UNSET_REG_BIT(int_ctrl_reg1, KXTE9_INT_CTRL_REG1_IEN);
			KIONIX_ACCEL_write_byte(KXTE9_I2C_INT_CTRL_REG1, int_ctrl_reg1);
		}
		else	 {
			status = 1;
		}
	}
	else if (device == 1){	//KXTF9
		if (KIONIX_ACCEL_read_bytes(KXTF9_I2C_INT_CTRL_REG1, &int_ctrl_reg1, 1) == 0){
			UNSET_REG_BIT(int_ctrl_reg1, KXTF9_INT_CTRL_REG1_IEN);
			KIONIX_ACCEL_write_byte(KXTF9_I2C_INT_CTRL_REG1, int_ctrl_reg1);
		}
		else	 {
			status = 1;
		}
	}
	else status = 1;	

	return status;
}
/*==================================================================================================
FUNCTION: 		KIONIX_ACCEL_int_activeh
DESCRIPTION:    	This function sets the polarity of physical interrupt pin to active high. 
ARGUMENTS PASSED:   	None
RETURN VALUE:   	0 = pass; 1 = fail
PRE-CONDITIONS:   	KIONIX_ACCEL_init() has been called
POST-CONDITIONS:   	Physical interrupt set to active high
==================================================================================================*/
int KIONIX_ACCEL_int_activeh(void)
{
	char int_ctrl_reg1 = 0;
	int status = 0;
	if (device == 0){	//KXTE9	
		if (KIONIX_ACCEL_read_bytes(KXTE9_I2C_INT_CTRL_REG1, &int_ctrl_reg1, 1) == 0){
			SET_REG_BIT(int_ctrl_reg1, KXTE9_INT_CTRL_REG1_IEA);
			KIONIX_ACCEL_write_byte(KXTE9_I2C_INT_CTRL_REG1, int_ctrl_reg1);
		}
		else	 {
			status = 1;
		}
	}
	else if (device == 1){	//KXTF9
		if (KIONIX_ACCEL_read_bytes(KXTF9_I2C_INT_CTRL_REG1, &int_ctrl_reg1, 1) == 0){
			SET_REG_BIT(int_ctrl_reg1, KXTF9_INT_CTRL_REG1_IEA);
			KIONIX_ACCEL_write_byte(KXTF9_I2C_INT_CTRL_REG1, int_ctrl_reg1);
		}
		else	 {
			status = 1;
		}
	}
	else status = 1;	
	
	return status;
	
}
/*==================================================================================================
FUNCTION: 		KIONIX_ACCEL_int_activel
DESCRIPTION:    	This function sets the polarity of physical interrupt pin to active low. 
ARGUMENTS PASSED:   	None
RETURN VALUE:   	0 = pass; 1 = fail
PRE-CONDITIONS:   	KIONIX_ACCEL_init() has been called
POST-CONDITIONS:   	Physical interrupt set to active low
==================================================================================================*/
int KIONIX_ACCEL_int_activel(void)
{
	char int_ctrl_reg1 = 0;
	int status = 0;
	if (device == 0){	//KXTE9	
		if (KIONIX_ACCEL_read_bytes(KXTE9_I2C_INT_CTRL_REG1, &int_ctrl_reg1, 1) == 0){
			UNSET_REG_BIT(int_ctrl_reg1, KXTE9_INT_CTRL_REG1_IEA);
			KIONIX_ACCEL_write_byte(KXTE9_I2C_INT_CTRL_REG1, int_ctrl_reg1);
		}
		else	 {
			status = 1;
		}
	}
	else if (device == 1){	//KXTF9
		if (KIONIX_ACCEL_read_bytes(KXTF9_I2C_INT_CTRL_REG1, &int_ctrl_reg1, 1) == 0){
			UNSET_REG_BIT(int_ctrl_reg1, KXTF9_INT_CTRL_REG1_IEA);
			KIONIX_ACCEL_write_byte(KXTF9_I2C_INT_CTRL_REG1, int_ctrl_reg1);
		}
		else	 {
			status = 1;
		}
	}
	else status = 1;	
	return status;
}
/*==================================================================================================
FUNCTION: 		KIONIX_ACCEL_int_latch
DESCRIPTION:    	This function sets the physical interrupt to a latch state. 
ARGUMENTS PASSED:   	None
RETURN VALUE:   	0 = pass; 1 = fail
PRE-CONDITIONS:   	KIONIX_ACCEL_init() has been called
POST-CONDITIONS:   	Physical interrupt set to latched response
==================================================================================================*/
int KIONIX_ACCEL_int_latch(void)
{
	char int_ctrl_reg1 = 0;
	int status = 0;
	if (device == 0){	//KXTE9	
		if (KIONIX_ACCEL_read_bytes(KXTE9_I2C_INT_CTRL_REG1, &int_ctrl_reg1, 1) == 0){
			UNSET_REG_BIT(int_ctrl_reg1, KXTE9_INT_CTRL_REG1_IEL);
			KIONIX_ACCEL_write_byte(KXTE9_I2C_INT_CTRL_REG1, int_ctrl_reg1);
		}
		else	 {
			status = 1;
		}
	}
	else if (device == 1){	//KXTF9
		if (KIONIX_ACCEL_read_bytes(KXTF9_I2C_INT_CTRL_REG1, &int_ctrl_reg1, 1) == 0){
			UNSET_REG_BIT(int_ctrl_reg1, KXTF9_INT_CTRL_REG1_IEL);
			KIONIX_ACCEL_write_byte(KXTF9_I2C_INT_CTRL_REG1, int_ctrl_reg1);
		}
		else	 {
			status = 1;
		}
	}
	else status = 1;	
	return status;
}
/*==================================================================================================
FUNCTION: 		KIONIX_ACCEL_int_pulse
DESCRIPTION:    	This function sets the physical interrupt to a pulse state. 
ARGUMENTS PASSED:   	None
RETURN VALUE:   	0 = pass; 1 = fail
PRE-CONDITIONS:   	KIONIX_ACCEL_init() has been called
POST-CONDITIONS:   	Physical interrupt set to pulse response
==================================================================================================*/
int KIONIX_ACCEL_int_pulse(void)
{
	char int_ctrl_reg1 = 0;
	int status = 0;
	if (device == 0){	//KXTE9	
		if (KIONIX_ACCEL_read_bytes(KXTE9_I2C_INT_CTRL_REG1, &int_ctrl_reg1, 1) == 0){
			SET_REG_BIT(int_ctrl_reg1, KXTE9_INT_CTRL_REG1_IEL);
			KIONIX_ACCEL_write_byte(KXTE9_I2C_INT_CTRL_REG1, int_ctrl_reg1);
		}
		else	 {
			status = 1;
		}
	}
	else if (device == 1){	//KXTF9
		if (KIONIX_ACCEL_read_bytes(KXTF9_I2C_INT_CTRL_REG1, &int_ctrl_reg1, 1) == 0){
			SET_REG_BIT(int_ctrl_reg1, KXTF9_INT_CTRL_REG1_IEL);
			KIONIX_ACCEL_write_byte(KXTF9_I2C_INT_CTRL_REG1, int_ctrl_reg1);
		}
		else	 {
			status = 1;
		}
	}
	else status = 1;	
	return status;
}
/*==================================================================================================
					KXTE9-SPECIFIC FUNCTIONS
==================================================================================================*/
/*==================================================================================================
FUNCTION: 		KIONIX_ACCEL_enable_back_to_sleep
DESCRIPTION:    	This function enables the back to sleep function. 
ARGUMENTS PASSED:   	None
RETURN VALUE:   	0 = pass; 1 = fail
PRE-CONDITIONS:   	KIONIX_ACCEL_init() has been called
POST-CONDITIONS:   	Back to sleep is enabled
==================================================================================================*/
int KIONIX_ACCEL_enable_back_to_sleep(void)
{
	char ctlreg_1 = 0;
	int status = 0;
	if (KIONIX_ACCEL_read_bytes(KIONIX_ACCEL_I2C_CTRL_REG1, &ctlreg_1, 1) == 0){
		SET_REG_BIT(ctlreg_1, CTRL_REG1_B2SE); /* set the B2SE bit to enable back to sleep function */
		KIONIX_ACCEL_write_byte(KIONIX_ACCEL_I2C_CTRL_REG1, ctlreg_1);
	}
	else	 {
		status = 1;
	}
	return status;
}
/*==================================================================================================
FUNCTION: 		KIONIX_ACCEL_disable_back_to_sleep
DESCRIPTION:    	This function disables the back to sleep function. 
ARGUMENTS PASSED:   	None
RETURN VALUE:   	0 = pass; 1 = fail
PRE-CONDITIONS:   	KIONIX_ACCEL_init() has been called
POST-CONDITIONS:   	Back to sleep is disabled
==================================================================================================*/
int KIONIX_ACCEL_disable_back_to_sleep(void)
{
	char ctlreg_1 = 0;
	int status = 0;
	if (KIONIX_ACCEL_read_bytes(KIONIX_ACCEL_I2C_CTRL_REG1, &ctlreg_1, 1) == 0){
		UNSET_REG_BIT(ctlreg_1, CTRL_REG1_B2SE); /* unset the B2SE bit to disable back to sleep function */
		KIONIX_ACCEL_write_byte(KIONIX_ACCEL_I2C_CTRL_REG1, ctlreg_1);
	}
	else	 {
		status = 1;
	}
	return status;
}
/*==================================================================================================
FUNCTION: 		KXTE9_read_b2s_status
DESCRIPTION:    	This function reads INT_SRC_REG1 to determine if back to sleep occurred. 
ARGUMENTS PASSED:   	None
RETURN VALUE:   	0 = back to sleep occurred; 1 = back to sleep did not occur
PRE-CONDITIONS:   	KIONIX_ACCEL_init() has been called
POST-CONDITIONS:   	None
==================================================================================================*/
int KXTE9_read_b2s_status(void)
{
	int wuf_status; 
	char src_reg1;
	if (device == 1){	//KXTF9
		printk("KXTE9 function (KXTE9_read_b2s_status) called for KXTF9 device\n");
		return 1;
	}	
	if (KIONIX_ACCEL_read_bytes(KXTE9_I2C_INT_SRC_REG1, &src_reg1, 1) == 0){
		if((src_reg1 & 0x04) == 0x04){
			wuf_status = 0;
		}
		else	wuf_status = 1;
	}
	else	wuf_status = 1;
	return wuf_status;
}
/*==================================================================================================
FUNCTION: 		KXTE9_b2s_timer
DESCRIPTION:    	This function sets the number of back-to-sleep debounce samples. 
ARGUMENTS PASSED:   	b2s_timer; 0-255
RETURN VALUE:   	0 = pass; 1 = fail
PRE-CONDITIONS:   	KIONIX_ACCEL_init() has been called
POST-CONDITIONS:   	Back-to-sleep debounce set according to b2s_timer
==================================================================================================*/
int KXTE9_b2s_timer(int b2s_timer)
{
	int status;
	if (device == 1){	//KXTF9
		printk("KXTE9 function (KXTE9_b2s_timer) called for KXTF9 device\n");
		return 1;
	}
	if (KIONIX_ACCEL_write_byte(KXTE9_I2C_B2S_TIMER, b2s_timer) == 0){
		status = 0;
	}
	else	status = 1;
	return status;
}
/*==================================================================================================
					KXTF9-SPECIFIC FUNCTIONS
==================================================================================================*/
/*==================================================================================================
FUNCTION: 		KXTF9_enable_tap_detection
DESCRIPTION:    	This function enables the tap detection function. 
ARGUMENTS PASSED:   	None
RETURN VALUE:   	0 = pass; 1 = fail
PRE-CONDITIONS:   	KIONIX_ACCEL_init() has been called
POST-CONDITIONS:   	Tap detection is enabled
==================================================================================================*/
int KXTF9_enable_tap_detection(void)
{
	char ctlreg_1 = 0;
	int status = 0;
	if (device == 0){	//KXTE9
		printk("KXTF9 function (KXTF9_enable_tap_detection) called for KXTE9 device\n");
		return 1;
	}
	if (KIONIX_ACCEL_read_bytes(KIONIX_ACCEL_I2C_CTRL_REG1, &ctlreg_1, 1) == 0){
		SET_REG_BIT(ctlreg_1, CTRL_REG1_TDTE); /* set TDTE bit to enable tap function */
		KIONIX_ACCEL_write_byte(KIONIX_ACCEL_I2C_CTRL_REG1, ctlreg_1);
	}
	else	 {
		status = 1;
	}
	return status;
}
/*==================================================================================================
FUNCTION: 		KXTF9_disable_tap_detection
DESCRIPTION:    	This function disables the tap detection function. 
ARGUMENTS PASSED:   	None
RETURN VALUE:   	0 = pass; 1 = fail
PRE-CONDITIONS:   	KIONIX_ACCEL_init() has been called
POST-CONDITIONS:   	Tap detection is disabled
==================================================================================================*/
int KXTF9_disable_tap_detection(void)
{
	char ctlreg_1 = 0;
	int status = 0;
	if (device == 0){	//KXTE9
		printk("KXTF9 function (KXTF9_disable_tap_detection) called for KXTE9 device\n");
		return 1;
	}
	if (KIONIX_ACCEL_read_bytes(KIONIX_ACCEL_I2C_CTRL_REG1, &ctlreg_1, 1) == 0){
		UNSET_REG_BIT(ctlreg_1, CTRL_REG1_TDTE); /* unset the TDTE bit to disable tap function */
		KIONIX_ACCEL_write_byte(KIONIX_ACCEL_I2C_CTRL_REG1, ctlreg_1);
	}
	else	 {
		status = 1;
	}
	return status;
}
/*==================================================================================================
FUNCTION: 		KXTF9_read_single_tap_status
DESCRIPTION:    	This function reads INT_SRC_REG2 to determine whether a single tap event
			occurred. 
ARGUMENTS PASSED:   	None
RETURN VALUE:   	0 = single tap occurred; 1 = single tap did not occur
PRE-CONDITIONS:   	KIONIX_ACCEL_init() has been called
POST-CONDITIONS:   	None
==================================================================================================*/
int KXTF9_read_single_tap_status(void)
{
	int single_tap; 
	char src_reg2;
	if (device == 0){	//KXTE9
		printk("KXTF9 function (KXTF9_read_single_tap_status) called for KXTE9 device\n");
		return 1;
	}
	if (KIONIX_ACCEL_read_bytes(KXTF9_I2C_INT_SRC_REG2, &src_reg2, 1) == 0){
		if((src_reg2 & 0x0C) == 0x04){
			single_tap = 0;
		}
		else	single_tap = 1;
	}
	else	single_tap = 1;
	return single_tap;
}
/*==================================================================================================
FUNCTION: 		KXTF9_read_double_tap_status
DESCRIPTION:    	This function reads INT_SRC_REG2 to determine whether a double tap event
			occurred.
ARGUMENTS PASSED:   	None
RETURN VALUE:   	0 = double tap occurred; 1 = double tap did not occur
PRE-CONDITIONS:   	KIONIX_ACCEL_init() has been called
POST-CONDITIONS:   	None
==================================================================================================*/
int KXTF9_read_double_tap_status(void)
{
	int double_tap; 
	char src_reg2;
	if (device == 0){	//KXTE9
		printk("KXTF9 function (KXTF9_read_double_tap_status) called for KXTE9 device\n");
		return 1;
	}
	if (KIONIX_ACCEL_read_bytes(KXTF9_I2C_INT_SRC_REG2, &src_reg2, 1) == 0){
		if((src_reg2 & 0x0C) == 0x08){
			double_tap = 0;
		}
		else	double_tap = 1;
	}
	else	double_tap = 1;
	return double_tap;
}
/*==================================================================================================
FUNCTION: 		KXTF9_set_odr_tilt
DESCRIPTION:    	This function sets the ODR frequency for the tilt position function. 
ARGUMENTS PASSED:   	frequency variable; 1, 6, 12, or 50
RETURN VALUE:   	0 = pass; 1 = fail
PRE-CONDITIONS:   	KIONIX_ACCEL_init() has been called
POST-CONDITIONS:   	ODR is set for tilt function according to frequency
==================================================================================================*/
int KXTF9_set_odr_tilt(int frequency)
{
	char ctlreg_3 = 0;
	if (device == 0){	//KXTE9
		printk("KXTF9 function (KXTF9_set_odr_tilt) called for KXTE9 device\n");
		return 1;
	}
	if (KIONIX_ACCEL_read_bytes(KIONIX_ACCEL_I2C_CTRL_REG3, &ctlreg_3, 1) != 0){
		return 1;
	}
	switch (frequency){
	case 1:		/* set all ODR's to 1.6Hz */
		UNSET_REG_BIT(ctlreg_3, CTRL_REG3_OTPA);
		UNSET_REG_BIT(ctlreg_3, CTRL_REG3_OTPB);
		break;
	case 6:		/* set all ODR's to 6.3Hz */
		UNSET_REG_BIT(ctlreg_3, CTRL_REG3_OTPA);
		SET_REG_BIT(ctlreg_3, CTRL_REG3_OTPB);
		break;
	case 12:	/* set all ODR's to 12.5Hz */
		SET_REG_BIT(ctlreg_3, CTRL_REG3_OTPA);
		UNSET_REG_BIT(ctlreg_3, CTRL_REG3_OTPB);
		break;
	case 50:	/* set all ODR's to 50Hz */
		SET_REG_BIT(ctlreg_3, CTRL_REG3_OTPA);
		SET_REG_BIT(ctlreg_3, CTRL_REG3_OTPB);
		break;
	default:
		return 1;
	}
	KIONIX_ACCEL_write_byte(KIONIX_ACCEL_I2C_CTRL_REG3, ctlreg_3);
	return 0;
}
/*==================================================================================================
FUNCTION: 		KXTF9_set_G_range
DESCRIPTION:    	This function sets the accelerometer G range. 
ARGUMENTS PASSED:   	range variable; 2, 4, or 8
RETURN VALUE:   	0 = pass; 1 = fail
PRE-CONDITIONS:   	KIONIX_ACCEL_init() has been called
POST-CONDITIONS:   	G range is set according to range
==================================================================================================*/
int KXTF9_set_G_range(int range)
{
	char ctlreg_1 = 0;
	if (device == 0){	//KXTE9
		printk("KXTF9 function (KXTF9_set_G_range) called for KXTE9 device\n");
		return 1;
	}
	if (KIONIX_ACCEL_read_bytes(KIONIX_ACCEL_I2C_CTRL_REG1, &ctlreg_1, 1) != 0){
		return 1;
	}
	switch (range){
	case 2:		/* set G-range to 2g */
		UNSET_REG_BIT(ctlreg_1, CTRL_REG1_GSEL1);
		UNSET_REG_BIT(ctlreg_1, CTRL_REG1_GSEL0);
		break;
	case 4:		/* set G-range to 4g */
		UNSET_REG_BIT(ctlreg_1, CTRL_REG1_GSEL1);
		SET_REG_BIT(ctlreg_1, CTRL_REG1_GSEL0);
		break;
	case 8:		/* set G-range to 8g */
		SET_REG_BIT(ctlreg_1, CTRL_REG1_GSEL1);
		UNSET_REG_BIT(ctlreg_1, CTRL_REG1_GSEL0);
		break;
	default:
		return 1;
	}
	KIONIX_ACCEL_write_byte(KIONIX_ACCEL_I2C_CTRL_REG1, ctlreg_1);
	return 0;
}
/*==================================================================================================
FUNCTION: 		KXTF9_read_HPF_cnt
DESCRIPTION:    	This function reads the high pass filtered number of counts on the X, Y, 
			and Z axes. 
ARGUMENTS PASSED:   	x, y, and z pointers
RETURN VALUE:   	0 = pass; 1 = fail
PRE-CONDITIONS:   	KIONIX_ACCEL_init() has been called
POST-CONDITIONS:   	x, y, and z are assigned
==================================================================================================*/
int KXTF9_read_HPF_cnt(int* x, int* y, int* z)
{
	int status =0, x_sign, y_sign, z_sign;
	char Res = 0;	
	char xyz[6] = {0, 0, 0, 0, 0, 0};
	if (device == 0){	//KXTE9
		printk("KXTF9 function (KXTF9_read_HPF_cnt) called for KXTE9 device\n");
		return 1;
	}
	if (KIONIX_ACCEL_read_bytes(KIONIX_ACCEL_I2C_CTRL_REG1, &Res, 1) == 0){
			Res = Res & 0x40;
			switch(Res){
				case 0x00:	//low-resolution state
					if ((status = KIONIX_ACCEL_read_bytes(KXTF9_I2C_XOUT_HPF_L, xyz, 6)) == 0){
						*x = ((int)xyz[1]);
						x_sign = *x >> 7;	//1 = negative; 0 = positive
						if (x_sign == 1){
							*x = ((~(*x) + 0x01) & 0x0FF);
							*x = -(*x);
						}
						*y = ((int)xyz[3]);
						y_sign = *y >> 7;	//1 = negative; 0 = positive
						if (y_sign == 1){
							*y = ((~(*y) + 0x01) & 0x0FF);	//2's complement
							*y = -(*y);
						}
						*z = ((int)xyz[5]);
						z_sign = *z >> 7;	//1 = negative; 0 = positive
						if (z_sign == 1){
							*z = ((~(*z) + 0x01) & 0x0FF);	//2's complement
							*z = -(*z);						
						}
					}
					break;
				case 0x40:	//high-resolution state
					if ((status = KIONIX_ACCEL_read_bytes(KXTF9_I2C_XOUT_HPF_L, xyz, 6)) == 0){
						*x = ((int)xyz[0]) >> 4;
						*x = *x + (((int)xyz[1]) << 4);
						x_sign = *x >> 11; 	//1 = negative; 0 = positive
						if (x_sign == 1){
							*x = ((~(*x) + 0x01) & 0x0FFF);	//2's complement
							*x = -(*x);
						}
						*y = ((int)xyz[2]) >> 4;
						*y = *y + (((int)xyz[3]) << 4);
						y_sign = *y >> 11; 	//1 = negative; 0 = positive
						if (y_sign == 1){
							*y = ((~(*y) + 0x01) & 0x0FFF);	//2's complement
							*y = -(*y);
						}
						*z = ((int)xyz[4]) >> 4;
						*z = *z + (((int)xyz[5]) << 4);
						z_sign = *z >> 11; 	//1 = negative; 0 = positive
						if (z_sign == 1){
							*z = ((~(*z) + 0x01) & 0x0FFF);	//2's complement
							*z = -(*z);
						}
					}
					break;
			}		
	}
	else	 {
		status = 1;
	}
	return status;
}
/*==================================================================================================
FUNCTION: 		KXTF9_read_HPF_g
DESCRIPTION:    	This function reads the G(gravity force) values on the X, Y, and Z axes.
			The units used are milli-g's, or 1/1000*G. 
ARGUMENTS PASSED:   	gx, gy, and gz pointers
RETURN VALUE:   	0 = pass; 1 = fail
PRE-CONDITIONS:   	KIONIX_ACCEL_init() has been called
POST-CONDITIONS:   	gx, gy, and gz are assigned
==================================================================================================*/
int KXTF9_read_HPF_g(int* gx, int* gy, int* gz)
{
	int status = 0;
	int x,y,z;
	int x_sign, y_sign, z_sign;
	int sensitivity;
	char xyz[6] = {0, 0, 0, 0, 0, 0};
	char Res = 0;
	char G_range = 0;
	int range = 0;
	if (device == 0){	//KXTE9
		printk("KXTF9 function (KXTF9_read_HPF_g) called for KXTE9 device\n");
		return 1;
	}
	//determine if in the low resolution or high resolution state
	if (KIONIX_ACCEL_read_bytes(KIONIX_ACCEL_I2C_CTRL_REG1, &Res, 1) == 0){
		if (KIONIX_ACCEL_read_bytes(KIONIX_ACCEL_I2C_CTRL_REG1, &G_range, 1) == 0){
			G_range = G_range & 0x18;
			G_range = G_range >> 3;
			switch(G_range){
				case 0:
					range = 2;
					break;
				case 1:
					range = 4;
					break;
				case 2:
					range = 8;
					break;
				default:
					break;
			}
			Res = Res & 0x40; 
			switch(Res){
				case 0x00:	//low-resolution state
					if ((status = KIONIX_ACCEL_read_bytes(KXTF9_I2C_XOUT_HPF_L, xyz, 6)) == 0){
						x = ((int)xyz[1]);
						x_sign = x >> 7;	//1 = negative; 0 = positive
						if (x_sign == 1){
							x = ((~(x) + 0x01) & 0x0FF);
							x = -(x);
						}
						y = ((int)xyz[3]);
						y_sign = y >> 7;	//1 = negative; 0 = positive
						if (y_sign == 1){
							y = ((~(y) + 0x01) & 0x0FF);	//2's complement
							y = -(y);
						}
						z = ((int)xyz[5]);
						z_sign = z >> 7;	//1 = negative; 0 = positive
						if (z_sign == 1){
							z = ((~(z) + 0x01) & 0x0FF);	//2's complement
							z = -(z);						
						}						
						sensitivity = (256)/(2*range);
						/* calculate milli-G's */
						*gx = 1000 * (x) / sensitivity; 
						*gy = 1000 * (y) / sensitivity; 
						*gz = 1000 * (z) / sensitivity;
					}
					break;
				case 0x40:	//high-resolution state
					if ((status = KIONIX_ACCEL_read_bytes(KXTF9_I2C_XOUT_HPF_L, xyz, 6)) == 0){
						x = ((int)xyz[0]) >> 4;
						x = x + (((int)xyz[1]) << 4);
						x_sign = x >> 11; 	//1 = negative; 0 = positive
						if (x_sign == 1){
							x = ((~(x) + 0x01) & 0x0FFF);	//2's complement
							x = -(x);
						}
						y = ((int)xyz[2]) >> 4;
						y = y + (((int)xyz[3]) << 4);
						y_sign = y >> 11; 	//1 = negative; 0 = positive
						if (y_sign == 1){
							y = ((~(y) + 0x01) & 0x0FFF);	//2's complement
							y = -(y);
						}
						z = ((int)xyz[4]) >> 4;
						z = z + (((int)xyz[5]) << 4);
						z_sign = z >> 11; 	//1 = negative; 0 = positive
						if (z_sign == 1){
							z = ((~(z) + 0x01) & 0x0FFF);	//2's complement
							z = -(z);
						}
						sensitivity = (4096)/(2*range);
						/* calculate milli-G's */
						*gx = 1000 * (x) / sensitivity; 
						*gy = 1000 * (y) / sensitivity; 
						*gz = 1000 * (z) / sensitivity;
					}
					break;
				default:
					break;
			}
		}
		else	 {
			status = 1;
		}
	}
	else	 {
		status = 1;
	}
	return status;
}
/*==================================================================================================
FUNCTION: 		KXTF9_read_current_odr_tilt
DESCRIPTION:    	This function reads the current ODR of the tilt function. 
ARGUMENTS PASSED:   	ODR_rate_tilt pointer
RETURN VALUE:   	0 = ODR set correctly; 1 = ODR invalid
PRE-CONDITIONS:   	KIONIX_ACCEL_init() has been called
POST-CONDITIONS:   	ODR_rate_tilt is assigned
==================================================================================================*/
int KXTF9_read_current_odr_tilt(double* ODR_rate_tilt)
{
	int status;
	char ctrl_reg;
	if (device == 0){	//KXTE9
		printk("KXTF9 function (KXTF9_read_current_odr_tilt) called for KXTE9 device\n");
		return 1;
	}
	if (KIONIX_ACCEL_read_bytes(KIONIX_ACCEL_I2C_CTRL_REG3, &ctrl_reg, 1) == 0){
		ctrl_reg &= 0x60;
		ctrl_reg >>= 5;
		switch (ctrl_reg){
		case 0:
			*ODR_rate_tilt = 1.6;
			status = 0;
			break;
		case 1:
			*ODR_rate_tilt = 6.3;
			status = 0;
			break;
		case 2:
			*ODR_rate_tilt = 12.5;
			status = 0;
			break;
		case 3:
			*ODR_rate_tilt = 50;
			status = 0;
			break;
		default:
			status = 1;
			break;
		}
	}
	else	 {
		status = 1;
	}
	return status;
}
/*==================================================================================================
FUNCTION: 		KXTF9_read_current_odr_tap
DESCRIPTION:    	This function reads the current ODR of the tap double tap function. 
ARGUMENTS PASSED:   	ODR_rate_tap pointer
RETURN VALUE:   	0 = ODR set correctly; 1 = ODR invalid
PRE-CONDITIONS:   	KIONIX_ACCEL_init() has been called
POST-CONDITIONS:   	ODR_rate_tap is assigned
==================================================================================================*/
int KXTF9_read_current_odr_tap(double* ODR_rate_tap)
{
	int status;
	char ctrl_reg;
	if (device == 0){	//KXTE9
		printk("KXTF9 function (KXTF9_read_current_odr_tap) called for KXTE9 device\n");
		return 1;
	}
	if (KIONIX_ACCEL_read_bytes(KIONIX_ACCEL_I2C_CTRL_REG3, &ctrl_reg, 1) == 0){
		ctrl_reg &= 0x0C;
		ctrl_reg >>= 2;
		switch (ctrl_reg){
		case 0:
			*ODR_rate_tap = 50;
			status = 0;
			break;
		case 1:
			*ODR_rate_tap = 100;
			status = 0;
			break;
		case 2:
			*ODR_rate_tap = 200;
			status = 0;
			break;
		case 3:
			*ODR_rate_tap = 400;
			status = 0;
			break;
		default:
			status = 1;
			break;
		}
	}
	else	 {
		status = 1;
	}
	return status;
}
/*==================================================================================================
FUNCTION: 		KXTF9_read_tap_direction
DESCRIPTION:    	This function reads INT_SRC_REG1 to determine which axis and in which direction
			a tap or double tap event occurred. 
ARGUMENTS PASSED:   	int_src_reg1 pointer
RETURN VALUE:   	0 = pass; 1 = fail
PRE-CONDITIONS:   	KIONIX_ACCEL_init() has been called
POST-CONDITIONS:   	int_src_reg1 is assigned
==================================================================================================*/
int KXTF9_read_tap_direction(char* int_src_reg1)
{
	int status;
	if (device == 0){	//KXTE9
		printk("KXTF9 function (KXTF9_read_tap_direction) called for KXTE9 device\n");
		return 1;
	}
	status = KIONIX_ACCEL_read_bytes(KXTF9_I2C_INT_SRC_REG1, int_src_reg1, 1);
	return status;
}
/*==================================================================================================
FUNCTION: 		KXTF9_int_alt_disable
DESCRIPTION:    	This function disables the alternate unlatched response for the physical 
			interrupt pin. 
ARGUMENTS PASSED:   	None
RETURN VALUE:   	0 = pass; 1 = fail
PRE-CONDITIONS:   	KIONIX_ACCEL_init() has been called
POST-CONDITIONS:   	Alternate unlatched response for physical interrupt disabled
==================================================================================================*/
int KXTF9_int_alt_disable(void)
{
	char int_ctrl_reg1 = 0;
	int status = 0;
	if (device == 0){	//KXTE9
		printk("KXTF9 function (KXTF9_int_alt_disable) called for KXTE9 device\n");
		return 1;
	}
	if (KIONIX_ACCEL_read_bytes(KXTF9_I2C_INT_CTRL_REG1, &int_ctrl_reg1, 1) == 0){
		UNSET_REG_BIT(int_ctrl_reg1, KXTF9_INT_CTRL_REG1_ULMB);
		KIONIX_ACCEL_write_byte(KXTF9_I2C_INT_CTRL_REG1, int_ctrl_reg1);
	}
	else	 {
		status = 1;
	}
	return status;
}
/*==================================================================================================
FUNCTION: 		KXTF9_int_alt_enable
DESCRIPTION:    	This function enables the alternate unlatched response for the physical 
			interrupt pin. 
ARGUMENTS PASSED:   	None
RETURN VALUE:   	0 = pass; 1 = fail
PRE-CONDITIONS:   	KIONIX_ACCEL_init() has been called
POST-CONDITIONS:   	Alternate unlatched response for physical interrupt enabled
==================================================================================================*/
int KXTF9_int_alt_enable(void)
{
	char int_ctrl_reg1 = 0;
	int status = 0;
	if (device == 0){	//KXTE9
		printk("KXTF9 function (KXTF9_int_alt_enable) called for KXTE9 device\n");
		return 1;
	}
	if (KIONIX_ACCEL_read_bytes(KXTF9_I2C_INT_CTRL_REG1, &int_ctrl_reg1, 1) == 0){
		SET_REG_BIT(int_ctrl_reg1, KXTF9_INT_CTRL_REG1_ULMB);
		KIONIX_ACCEL_write_byte(KXTF9_I2C_INT_CTRL_REG1, int_ctrl_reg1);
	}
	else	 {
		status = 1;
	}
	return status;
}
/*==================================================================================================
FUNCTION: 		KXTF9_tdt_timer
DESCRIPTION:    	This function defines the minimum separation between the first and second
			taps in a double tap event. 
ARGUMENTS PASSED:   	tdt_timer; 0-255
RETURN VALUE:   	0 = pass; 1 = fail
PRE-CONDITIONS:   	KIONIX_ACCEL_init() has been called
POST-CONDITIONS:   	Double tap minimum separation set according to tdt_timer
IMPORTANT NOTES:   	Default: 0.3s (0x78h)
==================================================================================================*/
int KXTF9_tdt_timer(int tdt_timer)
{
	int status;
	if (device == 0){	//KXTE9
		printk("KXTF9 function (KXTF9_tdt_timer) called for KXTE9 device\n");
		return 1;
	}
	if (KIONIX_ACCEL_write_byte(KXTF9_I2C_TDT_TIMER, tdt_timer) == 0){
		status = 0;
	}
	else	 {
		status = 1;
	}
	return status;
}
/*==================================================================================================
FUNCTION: 		KXTF9_tdt_h_thresh
DESCRIPTION:    	This function defines the upper limit for the jerk threshold. 
ARGUMENTS PASSED:   	tdt_h_thresh; 0-255
RETURN VALUE:   	0 = pass; 1 = fail
PRE-CONDITIONS:   	KIONIX_ACCEL_init() has been called
POST-CONDITIONS:   	Upper tap threshold set according to tdt_h_thresh
IMPORTANT NOTES:   	Default: 14j (0xB6h)
==================================================================================================*/
int KXTF9_tdt_h_thresh(int tdt_h_thresh)
{
	int status;
	if (device == 0){	//KXTE9
		printk("KXTF9 function (KXTF9_tdt_h_thresh) called for KXTE9 device\n");
		return 1;
	}
	if (KIONIX_ACCEL_write_byte(KXTF9_I2C_TDT_H_THRESH, tdt_h_thresh) == 0){
		status = 0;
	}
	else	 {
		status = 1;
	}
	return status;
}
/*==================================================================================================
FUNCTION: 		KXTF9_tdt_l_thresh
DESCRIPTION:    	This function defines the lower limit for the jerk threshold. 
ARGUMENTS PASSED:   	tdt_l_thresh; 0-255
RETURN VALUE:   	0 = pass; 1 = fail
PRE-CONDITIONS:   	KIONIX_ACCEL_init() has been called
POST-CONDITIONS:   	Lower tap threshold set according to tdt_l_thresh
IMPORTANT NOTES:   	Default: 1j (0x1Ah)
==================================================================================================*/
int KXTF9_tdt_l_thresh(int tdt_l_thresh)
{
	int status;
	if (device == 0){	//KXTE9
		printk("KXTF9 function (KXTF9_tdt_l_thresh) called for KXTE9 device\n");
		return 1;
	}
	if (KIONIX_ACCEL_write_byte(KXTF9_I2C_TDT_L_THRESH, tdt_l_thresh) == 0){
		status = 0;
	}
	else	 {
		status = 1;
	}
	return status;
}
/*==================================================================================================
FUNCTION: 		KXTF9_tdt_tap_timer
DESCRIPTION:    	This function defines the minimum and maximum pulse width for the tap event. 
ARGUMENTS PASSED:   	tdt_tap_timer; 0-255
RETURN VALUE:   	0 = pass; 1 = fail
PRE-CONDITIONS:   	KIONIX_ACCEL_init() has been called
POST-CONDITIONS:   	Tap timer set according to tdt_tap_timer
IMPORTANT NOTES:   	Default: 0.005s lower limit, 0.05s upper limit (0xA2h)
==================================================================================================*/
int KXTF9_tdt_tap_timer(int tdt_tap_timer)
{
	int status;
	if (device == 0){	//KXTE9
		printk("KXTF9 function (KXTF9_tdt_tap_timer) called for KXTE9 device\n");
		return 1;
	}
	if (KIONIX_ACCEL_write_byte(KXTF9_I2C_TDT_TAP_TIMER, tdt_tap_timer) == 0){
		status = 0;
	}
	else	 {
		status = 1;
	}
	return status;
}
/*==================================================================================================
FUNCTION: 		KXTF9_tdt_total_timer
DESCRIPTION:    	This function defines the amount of time that two taps in a double tap event 
			can be avove the PI threshold. 
ARGUMENTS PASSED:   	tdt_total_timer; 0-255
RETURN VALUE:   	0 = pass; 1 = fail
PRE-CONDITIONS:   	KIONIX_ACCEL_init() has been called
POST-CONDITIONS:   	Total timer set according to tdt_total_timer
IMPORTANT NOTES:   	Default: 0.09s (0x24h)
==================================================================================================*/
int KXTF9_tdt_total_timer(int tdt_total_timer)
{
	int status;
	if (device == 0){	//KXTE9
		printk("KXTF9 function (KXTF9_tdt_total_timer) called for KXTE9 device\n");
		return 1;
	}
	if (KIONIX_ACCEL_write_byte(KXTF9_I2C_TDT_TOTAL_TIMER, tdt_total_timer) == 0){
		status = 0;
	}
	else	 {
		status = 1;
	}
	return status;
}
/*==================================================================================================
FUNCTION: 		KXTF9_tdt_latency_timer
DESCRIPTION:    	This function defines the total amount of time that the tap algorithm will 
			count samples that are above the PI threshold. 
ARGUMENTS PASSED:   	tdt_latency_timer; 0-255
RETURN VALUE:   	0 = pass; 1 = fail
PRE-CONDITIONS:   	KIONIX_ACCEL_init() has been called
POST-CONDITIONS:   	Latency timer set according to tdt_latency_timer
IMPORTANT NOTES:   	Default: 0.1s (0x28h)
==================================================================================================*/
int KXTF9_tdt_latency_timer(int tdt_latency_timer)
{
	int status;
	if (device == 0){	//KXTE9
		printk("KXTF9 function (KXTF9_tdt_latency_timer) called for KXTE9 device\n");
		return 1;
	}
	if (KIONIX_ACCEL_write_byte(KXTF9_I2C_TDT_LATENCY_TIMER, tdt_latency_timer) == 0){
		status = 0;
	}
	else	 {
		status = 1;
	}
	return status;
}
/*==================================================================================================
FUNCTION: 		KXTF9_tdt_window_timer
DESCRIPTION:    	This function defines the time window for the entire tap event, 
			single or double, to occur. 
ARGUMENTS PASSED:   	tdt_window_timer; 0-255
RETURN VALUE:   	0 = pass; 1 = fail
PRE-CONDITIONS:   	KIONIX_ACCEL_init() has been called
POST-CONDITIONS:   	Window timer set according to tdt_window_timer
IMPORTANT NOTES:   	Default: 0.4s (0xA0h)
==================================================================================================*/
int KXTF9_tdt_window_timer(int tdt_window_timer)
{
	int status;
	if (device == 0){	//KXTE9
		printk("KXTF9 function (KXTF9_tdt_window_timer) called for KXTE9 device\n");
		return 1;
	}
	if (KIONIX_ACCEL_write_byte(KXTF9_I2C_TDT_WINDOW_TIMER, tdt_window_timer) == 0){
		status = 0;
	}
	else	 {
		status = 1;
	}
	return status;
}
/*==================================================================================================
FUNCTION: 		KXTF9_tap_mask_TFU
DESCRIPTION:    	This function masks Z+ (face up) tap. 
ARGUMENTS PASSED:   	None
RETURN VALUE:   	0 = pass; 1 = fail
PRE-CONDITIONS:   	KIONIX_ACCEL_init() has been called
POST-CONDITIONS:   	Z+ tap masked
==================================================================================================*/
int KXTF9_tap_mask_TFU(void)
{
	char ctrl_reg3 = 0;
	int status = 0;
	if (device == 0){	//KXTE9
		printk("KXTF9 function (KXTF9_tap_mask_TFU) called for KXTE9 device\n");
		return 1;
	}
	if (KIONIX_ACCEL_read_bytes(KXTF9_I2C_INT_CTRL_REG3, &ctrl_reg3, 1) == 0){
		SET_REG_BIT(ctrl_reg3, INT_CTRL_REG3_TFUM);
		KIONIX_ACCEL_write_byte(KXTF9_I2C_INT_CTRL_REG3, ctrl_reg3);
	}
	else	 {
		status = 1;
	}
	return status;
}
/*==================================================================================================
FUNCTION: 		KXTF9_tap_unmask_TFU
DESCRIPTION:    	This function unmasks Z+ (face up) tap. 
ARGUMENTS PASSED:   	None
RETURN VALUE:   	0 = pass; 1 = fail
PRE-CONDITIONS:   	KIONIX_ACCEL_init() has been called
POST-CONDITIONS:   	Z+ tap unmasked
==================================================================================================*/
int KXTF9_tap_unmask_TFU(void)
{
	char ctrl_reg3 = 0;
	int status = 0;
	if (device == 0){	//KXTE9
		printk("KXTF9 function (KXTF9_tap_unmask_TFU) called for KXTE9 device\n");
		return 1;
	}
	if (KIONIX_ACCEL_read_bytes(KXTF9_I2C_INT_CTRL_REG3, &ctrl_reg3, 1) == 0){
		UNSET_REG_BIT(ctrl_reg3, INT_CTRL_REG3_TFUM);
		KIONIX_ACCEL_write_byte(KXTF9_I2C_INT_CTRL_REG3, ctrl_reg3);
	}
	else	 {
		status = 1;
	}
	return status;
}
/*==================================================================================================
FUNCTION: 		KXTF9_tap_mask_TFD
DESCRIPTION:    	This function masks Z- (face down) tap. 
ARGUMENTS PASSED:   	None
RETURN VALUE:   	0 = pass; 1 = fail
PRE-CONDITIONS:   	KIONIX_ACCEL_init() has been called
POST-CONDITIONS:   	Z- tap masked
==================================================================================================*/
int KXTF9_tap_mask_TFD(void)
{
	char ctrl_reg3 = 0;
	int status = 0;
	if (device == 0){	//KXTE9
		printk("KXTF9 function (KXTF9_tap_mask_TFD) called for KXTE9 device\n");
		return 1;
	}
	if (KIONIX_ACCEL_read_bytes(KXTF9_I2C_INT_CTRL_REG3, &ctrl_reg3, 1) == 0){
		SET_REG_BIT(ctrl_reg3, INT_CTRL_REG3_TFDM);
		KIONIX_ACCEL_write_byte(KXTF9_I2C_INT_CTRL_REG3, ctrl_reg3);
	}
	else	 {
		status = 1;
	}
	return status;
}
/*==================================================================================================
FUNCTION: 		KXTF9_tap_unmask_TFD
DESCRIPTION:    	This function unmasks Z- (face down) tap. 
ARGUMENTS PASSED:   	None
RETURN VALUE:   	0 = pass; 1 = fail
PRE-CONDITIONS:   	KIONIX_ACCEL_init() has been called
POST-CONDITIONS:   	Z- tap unmasked
==================================================================================================*/
int KXTF9_tap_unmask_TFD(void)
{
	char ctrl_reg3 = 0;
	int status = 0;
	if (device == 0){	//KXTE9
		printk("KXTF9 function (KXTF9_tap_unmask_TFD) called for KXTE9 device\n");
		return 1;
	}
	if (KIONIX_ACCEL_read_bytes(KXTF9_I2C_INT_CTRL_REG3, &ctrl_reg3, 1) == 0){
		UNSET_REG_BIT(ctrl_reg3, INT_CTRL_REG3_TFDM);
		KIONIX_ACCEL_write_byte(KXTF9_I2C_INT_CTRL_REG3, ctrl_reg3);
	}
	else	 {
		status = 1;
	}
	return status;
}
/*==================================================================================================
FUNCTION: 		KXTF9_tap_mask_TUP
DESCRIPTION:    	This function masks Y+ (up) tap. 
ARGUMENTS PASSED:   	None
RETURN VALUE:   	0 = pass; 1 = fail
PRE-CONDITIONS:   	KIONIX_ACCEL_init() has been called
POST-CONDITIONS:   	Y+ tap masked
==================================================================================================*/
int KXTF9_tap_mask_TUP(void)
{
	char ctrl_reg3 = 0;
	int status = 0;
	if (device == 0){	//KXTE9
		printk("KXTF9 function (KXTF9_tap_mask_TUP) called for KXTE9 device\n");
		return 1;
	}
	if (KIONIX_ACCEL_read_bytes(KXTF9_I2C_INT_CTRL_REG3, &ctrl_reg3, 1) == 0){
		SET_REG_BIT(ctrl_reg3, INT_CTRL_REG3_TUPM);
		KIONIX_ACCEL_write_byte(KXTF9_I2C_INT_CTRL_REG3, ctrl_reg3);
	}
	else	 {
		status = 1;
	}
	return status;
}
/*==================================================================================================
FUNCTION: 		KXTF9_tap_unmask_TUP
DESCRIPTION:    	This function unmasks Y+ (up) tap. 
ARGUMENTS PASSED:   	None
RETURN VALUE:   	0 = pass; 1 = fail
PRE-CONDITIONS:   	KIONIX_ACCEL_init() has been called
POST-CONDITIONS:   	Y+ tap unmasked
==================================================================================================*/
int KXTF9_tap_unmask_TUP(void)
{
	char ctrl_reg3 = 0;
	int status = 0;
	if (device == 0){	//KXTE9
		printk("KXTF9 function (KXTF9_tap_unmask_TUP) called for KXTE9 device\n");
		return 1;
	}
	if (KIONIX_ACCEL_read_bytes(KXTF9_I2C_INT_CTRL_REG3, &ctrl_reg3, 1) == 0){
		UNSET_REG_BIT(ctrl_reg3, INT_CTRL_REG3_TUPM);
		KIONIX_ACCEL_write_byte(KXTF9_I2C_INT_CTRL_REG3, ctrl_reg3);
	}
	else	 {
		status = 1;
	}
	return status;
}
/*==================================================================================================
FUNCTION: 		KXTF9_tap_mask_TDO
DESCRIPTION:    	This function masks Y- (down) tap. 
ARGUMENTS PASSED:   	None
RETURN VALUE:   	0 = pass; 1 = fail
PRE-CONDITIONS:   	KIONIX_ACCEL_init() has been called
POST-CONDITIONS:   	Y- tap masked
==================================================================================================*/
int KXTF9_tap_mask_TDO(void)
{
	char ctrl_reg3 = 0;
	int status = 0;
	if (device == 0){	//KXTE9
		printk("KXTF9 function (KXTF9_tap_mask_TDO) called for KXTE9 device\n");
		return 1;
	}
	if (KIONIX_ACCEL_read_bytes(KXTF9_I2C_INT_CTRL_REG3, &ctrl_reg3, 1) == 0){
		SET_REG_BIT(ctrl_reg3, INT_CTRL_REG3_TDOM);
		KIONIX_ACCEL_write_byte(KXTF9_I2C_INT_CTRL_REG3, ctrl_reg3);
	}
	else	 {
		status = 1;
	}
	return status;
}
/*==================================================================================================
FUNCTION: 		KXTF9_tap_unmask_TDO
DESCRIPTION:    	This function unmasks Y- (down) tap. 
ARGUMENTS PASSED:   	None
RETURN VALUE:   	0 = pass; 1 = fail
PRE-CONDITIONS:   	KIONIX_ACCEL_init() has been called
POST-CONDITIONS:   	Y- tap unmasked
==================================================================================================*/
int KXTF9_tap_unmask_TDO(void)
{
	char ctrl_reg3 = 0;
	int status = 0;
	if (device == 0){	//KXTE9
		printk("KXTF9 function (KXTF9_tap_unmask_TDO) called for KXTE9 device\n");
		return 1;
	}
	if (KIONIX_ACCEL_read_bytes(KXTF9_I2C_INT_CTRL_REG3, &ctrl_reg3, 1) == 0){
		UNSET_REG_BIT(ctrl_reg3, INT_CTRL_REG3_TDOM);
		KIONIX_ACCEL_write_byte(KXTF9_I2C_INT_CTRL_REG3, ctrl_reg3);
	}
	else	 {
		status = 1;
	}
	return status;
}
/*==================================================================================================
FUNCTION: 		KXTF9_tap_mask_TRI
DESCRIPTION:    	This function masks X+ (right) tap. 
ARGUMENTS PASSED:   	None
RETURN VALUE:   	0 = pass; 1 = fail
PRE-CONDITIONS:   	KIONIX_ACCEL_init() has been called
POST-CONDITIONS:   	X+ tap masked
==================================================================================================*/
int KXTF9_tap_mask_TRI(void)
{
	char ctrl_reg3 = 0;
	int status = 0;
	if (device == 0){	//KXTE9
		printk("KXTF9 function (KXTF9_tap_mask_TRI) called for KXTE9 device\n");
		return 1;
	}
	if (KIONIX_ACCEL_read_bytes(KXTF9_I2C_INT_CTRL_REG3, &ctrl_reg3, 1) == 0){
		SET_REG_BIT(ctrl_reg3, INT_CTRL_REG3_TRIM);
		KIONIX_ACCEL_write_byte(KXTF9_I2C_INT_CTRL_REG3, ctrl_reg3);
	}
	else	 {
		status = 1;
	}
	return status;
}
/*==================================================================================================
FUNCTION: 		KXTF9_tap_unmask_TRI
DESCRIPTION:    	This function unmasks X+ (right) tap. 
ARGUMENTS PASSED:   	None
RETURN VALUE:   	0 = pass; 1 = fail
PRE-CONDITIONS:   	KIONIX_ACCEL_init() has been called
POST-CONDITIONS:   	X+ tap unmasked
==================================================================================================*/
int KXTF9_tap_unmask_TRI(void)
{
	char ctrl_reg3 = 0;
	int status = 0;
	if (device == 0){	//KXTE9
		printk("KXTF9 function (KXTF9_tap_unmask_TRI) called for KXTE9 device\n");
		return 1;
	}
	if (KIONIX_ACCEL_read_bytes(KXTF9_I2C_INT_CTRL_REG3, &ctrl_reg3, 1) == 0){
		UNSET_REG_BIT(ctrl_reg3, INT_CTRL_REG3_TRIM);
		KIONIX_ACCEL_write_byte(KXTF9_I2C_INT_CTRL_REG3, ctrl_reg3);
	}
	else	 {
		status = 1;
	}
	return status;
}
/*==================================================================================================
FUNCTION: 		KXTF9_tap_mask_TLE
DESCRIPTION:    	This function masks X- (left) tap. 
ARGUMENTS PASSED:   	None
RETURN VALUE:   	0 = pass; 1 = fail
PRE-CONDITIONS:   	KIONIX_ACCEL_init() has been called
POST-CONDITIONS:   	X- tap masked
==================================================================================================*/
int KXTF9_tap_mask_TLE(void)
{
	char ctrl_reg3 = 0;
	int status = 0;
	if (device == 0){	//KXTE9
		printk("KXTF9 function (KXTF9_tap_mask_TLE) called for KXTE9 device\n");
		return 1;
	}
	if (KIONIX_ACCEL_read_bytes(KXTF9_I2C_INT_CTRL_REG3, &ctrl_reg3, 1) == 0){
		SET_REG_BIT(ctrl_reg3, INT_CTRL_REG3_TLEM);
		KIONIX_ACCEL_write_byte(KXTF9_I2C_INT_CTRL_REG3, ctrl_reg3);
	}
	else	 {
		status = 1;
	}
	return status;
}
/*==================================================================================================
FUNCTION: 		KXTF9_tap_unmask_TLE
DESCRIPTION:    	This function unmasks X- (left) tap. 
ARGUMENTS PASSED:   	None
RETURN VALUE:   	0 = pass; 1 = fail
PRE-CONDITIONS:   	KIONIX_ACCEL_init() has been called
POST-CONDITIONS:   	X- tap unmasked
==================================================================================================*/
int KXTF9_tap_unmask_TLE(void)
{
	char ctrl_reg3 = 0;
	int status = 0;
	if (device == 0){	//KXTE9
		printk("KXTF9 function (KXTF9_tap_unmask_TLE) called for KXTE9 device\n");
		return 1;
	}
	if (KIONIX_ACCEL_read_bytes(KXTF9_I2C_INT_CTRL_REG3, &ctrl_reg3, 1) == 0){
		UNSET_REG_BIT(ctrl_reg3, INT_CTRL_REG3_TLEM);
		KIONIX_ACCEL_write_byte(KXTF9_I2C_INT_CTRL_REG3, ctrl_reg3);
	}
	else	 {
		status = 1;
	}
	return status;
}
/*==================================================================================================
FUNCTION: 		KXTF9_tap_mask_all_direction
DESCRIPTION:    	
ARGUMENTS PASSED:   	None
RETURN VALUE:   	0 = pass; 1 = fail
PRE-CONDITIONS:   	KIONIX_ACCEL_init() has been called
POST-CONDITIONS:   	All direction tap masked
==================================================================================================*/
int KXTF9_tap_mask_all_direction(void)
{
	int res=0;

	res |= KXTF9_tap_mask_TFU();  // Z+
	res |= KXTF9_tap_mask_TFD(); // Z-
	res |= KXTF9_tap_mask_TUP(); // Y+
	res |= KXTF9_tap_mask_TDO(); // Y-
	res |= KXTF9_tap_mask_TRI(); // X+
	res |= KXTF9_tap_mask_TLE(); // X-

	return res;
}
/*==================================================================================================
FUNCTION: 		KXTF9_tap_unmask_all_direction
DESCRIPTION:    	
ARGUMENTS PASSED:   	None
RETURN VALUE:   	0 = pass; 1 = fail
PRE-CONDITIONS:   	KIONIX_ACCEL_init() has been called
POST-CONDITIONS:   	All direction tap unmasked
==================================================================================================*/
int KXTF9_tap_unmask_all_direction(void)
{
	int res=0;

	res |= KXTF9_tap_unmask_TFU();  // Z+
	res |= KXTF9_tap_unmask_TFD(); // Z-
	res |= KXTF9_tap_unmask_TUP(); // Y+
	res |= KXTF9_tap_unmask_TDO(); // Y-
	res |= KXTF9_tap_unmask_TRI(); // X+
	res |= KXTF9_tap_unmask_TLE(); // X-

	return res;
}
/*==================================================================================================
FUNCTION: 		KXTF9_set_odr_tap
DESCRIPTION:    	This function sets the ODR frequency for the Direction Tap function. 
ARGUMENTS PASSED:   	frequency variable; 50, 100, 200 or 400
RETURN VALUE:   	0 = pass; 1 = fail
PRE-CONDITIONS:   	KIONIX_ACCEL_init() has been called
POST-CONDITIONS:   	ODR is set for Direction Tap function according to frequency
==================================================================================================*/
int KXTF9_set_odr_tap(int frequency)
{
	char ctlreg_3 = 0;
	if (device == 0){	//KXTE9
		printk("KXTF9 function (KXTF9_set_odr_tap) called for KXTF9 device\n");
		return 1;
	}
	if (KIONIX_ACCEL_read_bytes(KIONIX_ACCEL_I2C_CTRL_REG3, &ctlreg_3, 1) != 0){
		return 1;
	}
	switch (frequency){
	case 50:		/* set all ODR's to 50Hz */
		UNSET_REG_BIT(ctlreg_3, CTRL_REG3_OTDTA);
		UNSET_REG_BIT(ctlreg_3, CTRL_REG3_OTDTB);
		break;
	case 100:		/* set all ODR's to 100Hz */
		UNSET_REG_BIT(ctlreg_3, CTRL_REG3_OTDTA);
		SET_REG_BIT(ctlreg_3, CTRL_REG3_OTDTB);
		break;
	case 200:	/* set all ODR's to 200Hz */
		SET_REG_BIT(ctlreg_3, CTRL_REG3_OTDTA);
		UNSET_REG_BIT(ctlreg_3, CTRL_REG3_OTDTB);
		break;
	case 400:	/* set all ODR's to 400Hz */
		SET_REG_BIT(ctlreg_3, CTRL_REG3_OTDTA);
		SET_REG_BIT(ctlreg_3, CTRL_REG3_OTDTB);
		break;
	default:
		return 1;
	}
	KIONIX_ACCEL_write_byte(KIONIX_ACCEL_I2C_CTRL_REG3, ctlreg_3);
	return 0;
}

/*==================================================================================================
FUNCTION: 		KXTF9_set_hpf_odr
DESCRIPTION:    	This function sets the high pass filter roll off frequency for the accelerometer outputs. 
ARGUMENTS PASSED:   	frequency, where roll_off_frequency = ODR/alpha; alpha = (50, 100, 200, 400)
RETURN VALUE:   	0 = pass; 1 = fail
PRE-CONDITIONS:   	KIONIX_ACCEL_init() has been called
POST-CONDITIONS:   	High pass filter roll off is set according to frequency
==================================================================================================*/
int KXTF9_set_hpf_odr(int frequency)
{
	char data_ctrl_reg = 0;
	if (device == 0){	//KXTE9
		printk("KXTF9 function (KXTF9_set_odr_tap) called for KXTE9 device\n");
		return 1;
	}
	if (KIONIX_ACCEL_read_bytes(KXTF9_I2C_DATA_CTRL_REG, &data_ctrl_reg, 1) != 0){
		return 1;
	}
	switch (frequency){
	case 50:	/* set tap ODR to 50Hz */
		UNSET_REG_BIT(data_ctrl_reg, DATA_CTRL_REG_HPFROA);
		UNSET_REG_BIT(data_ctrl_reg, DATA_CTRL_REG_HPFROB);
		break;
	case 100:	/* set tap ODR to 100 Hz */
		UNSET_REG_BIT(data_ctrl_reg, DATA_CTRL_REG_HPFROA);
		SET_REG_BIT(data_ctrl_reg, DATA_CTRL_REG_HPFROB);
		break;
	case 200:	/* set tap ODR to 200 Hz */
		SET_REG_BIT(data_ctrl_reg, DATA_CTRL_REG_HPFROA);
		UNSET_REG_BIT(data_ctrl_reg, DATA_CTRL_REG_HPFROB);
		break;
	case 400:	/* set tap ODR to 400 Hz */
		SET_REG_BIT(data_ctrl_reg, DATA_CTRL_REG_HPFROA);
		SET_REG_BIT(data_ctrl_reg, DATA_CTRL_REG_HPFROB);
		break;
	default:
		return 1;
	}
	KIONIX_ACCEL_write_byte(KXTF9_I2C_DATA_CTRL_REG, data_ctrl_reg);
	return 0;
}
/*==================================================================================================
FUNCTION: 		KXTF9_set_lpf_odr
DESCRIPTION:    	This function sets the low pass filter roll off for the accelerometer outputs. 
ARGUMENTS PASSED:   	roll off frequency (6, 12, 25, 50, 100, 200, 400)
RETURN VALUE:   	0 = pass; 1 = fail
PRE-CONDITIONS:   	KIONIX_ACCEL_init() has been called
POST-CONDITIONS:   	Low pass filter roll off is set according to frequency
==================================================================================================*/
int KXTF9_set_lpf_odr(int frequency)
{
	char data_ctrl_reg = 0;
	if (device == 0){	//KXTE9
		printk("KXTF9 function (KXTF9_set_lpf_odr) called for KXTE9 device\n");
		return 1;
	}
	if (KIONIX_ACCEL_read_bytes(KXTF9_I2C_DATA_CTRL_REG, &data_ctrl_reg, 1) != 0){
		return 1;
	}
	switch (frequency){
	case 6:		/* set LPF rolloff to 6.25Hz */
		UNSET_REG_BIT(data_ctrl_reg, DATA_CTRL_REG_OSAA);
		UNSET_REG_BIT(data_ctrl_reg, DATA_CTRL_REG_OSAB);
		UNSET_REG_BIT(data_ctrl_reg, DATA_CTRL_REG_OSAC);
		break;
	case 12:	/* set LPF rolloff to 12.5Hz */
		UNSET_REG_BIT(data_ctrl_reg, DATA_CTRL_REG_OSAA);
		UNSET_REG_BIT(data_ctrl_reg, DATA_CTRL_REG_OSAB);
		SET_REG_BIT(data_ctrl_reg, DATA_CTRL_REG_OSAC);
		break;
	case 25:	/* set LPF rolloff to 25Hz */
		UNSET_REG_BIT(data_ctrl_reg, DATA_CTRL_REG_OSAA);
		SET_REG_BIT(data_ctrl_reg, DATA_CTRL_REG_OSAB);
		UNSET_REG_BIT(data_ctrl_reg, DATA_CTRL_REG_OSAC);
		break;
	case 50:	/* set LPF rolloff to 50Hz */
		UNSET_REG_BIT(data_ctrl_reg, DATA_CTRL_REG_OSAA);
		SET_REG_BIT(data_ctrl_reg, DATA_CTRL_REG_OSAB);
		SET_REG_BIT(data_ctrl_reg, DATA_CTRL_REG_OSAC);
		break;
	case 100:	/* set LPF rolloff to 100Hz */
		SET_REG_BIT(data_ctrl_reg, DATA_CTRL_REG_OSAA);
		UNSET_REG_BIT(data_ctrl_reg, DATA_CTRL_REG_OSAB);
		UNSET_REG_BIT(data_ctrl_reg, DATA_CTRL_REG_OSAC);
		break;
	case 200:	/* set LPF rolloff to 200 Hz */
		SET_REG_BIT(data_ctrl_reg, DATA_CTRL_REG_OSAA);
		UNSET_REG_BIT(data_ctrl_reg, DATA_CTRL_REG_OSAB);
		SET_REG_BIT(data_ctrl_reg, DATA_CTRL_REG_OSAC);
		break;
	case 400:	/* set LPF rolloff to 400 Hz */
		SET_REG_BIT(data_ctrl_reg, DATA_CTRL_REG_OSAA);
		SET_REG_BIT(data_ctrl_reg, DATA_CTRL_REG_OSAB);
		UNSET_REG_BIT(data_ctrl_reg, DATA_CTRL_REG_OSAC);
		break;
	default:
		return 1;
	}
	KIONIX_ACCEL_write_byte(KXTF9_I2C_DATA_CTRL_REG, data_ctrl_reg);
	return 0;
}
/*==================================================================================================
FUNCTION: 		KXTF9_set_resolution
DESCRIPTION:    	This function sets the resolution of the accelerometer outputs. 
ARGUMENTS PASSED:   	resolution (8-bit or 12-bit)
RETURN VALUE:   	0 = pass; 1 = fail
PRE-CONDITIONS:   	KIONIX_ACCEL_init() has been called
POST-CONDITIONS:   	Accelerometer resolution is set according to resolution
==================================================================================================*/
int KXTF9_set_resolution(int resolution)
{
	char ctrl_reg1 = 0;	
	if (device == 0){	//KXTE9
		printk("KXTF9 function (KXTF9_set_resolution) called for KXTE9 device\n");
		return 1;
	}
	if (KIONIX_ACCEL_read_bytes(KIONIX_ACCEL_I2C_CTRL_REG1, &ctrl_reg1, 1) != 0){
		return 1;
	}
	switch (resolution){
	case 8:		/* set resolution to 8 bits */
		UNSET_REG_BIT(ctrl_reg1, CTRL_REG1_RES);		
		break;
	case 12:	/* set resolution to 12 bits */
		SET_REG_BIT(ctrl_reg1, CTRL_REG1_RES);
		break;
	default:
		return 1;
	}
	KIONIX_ACCEL_write_byte(KIONIX_ACCEL_I2C_CTRL_REG1, ctrl_reg1);
	return 0;
}



//*************************************************************
//	KIONIX_SHAKE_Init
//		- initializes the shake detection engine
//	params
//		- shake_data* data = engine data
//	return
//		- none
//*************************************************************
void KIONIX_SHAKE_Init(shake_data* data)
{
    // init thresholds (convert ms to counts)
    data->maxDuration = CONFIG_DURATION / (1000 / CONFIG_RATE);
    data->maxDelay    = CONFIG_DELAY / (1000 / CONFIG_RATE);
    data->maxTimeout  = CONFIG_TIMEOUT / (1000 / CONFIG_RATE);

    // init timers
    data->cntDuration = 0;
    data->cntDelay    = 0;
    data->cntTimeout  = 0;

    // init counters
    data->cntShake    = 0;
    data->cntInvalid  = 0;
}


//*************************************************************
//	KIONIX_SHAKE_Update
//		- updates the shake detection engine
//      - maintains current shake count
//      - NOTE: must be called at a fixed interval
//	params
//		- shake_data* data = engine data
//      - long val = (x^2 + y^2 + z^2) / 1000
//	return
//		- long = current shake count
//*************************************************************
long KIONIX_SHAKE_Update(shake_data* data, long val)
{
    // possible shake...
    if (val > CONFIG_THRESHOLD)
    {
        // if the delay timer has started & 
        // not yet expired -> flag invalid
        if (data->cntDelay > 0)
            data->cntInvalid = 1;

        // inc duration
        data->cntDuration += 1;                
        
        // reset delay & timeout
        data->cntDelay   = data->maxDelay;
        data->cntTimeout = data->maxTimeout;
        
        return 0;
    }

    // shake detected...
    if ((data->cntDuration >= 2) && 
        (data->cntDuration <= data->maxDuration))
    {
        // add valid shakes to the count
        if (data->cntInvalid == 0)
            data->cntShake += 1;
    }    

    // flag valid & reset duration
    data->cntInvalid  = 0;    
    data->cntDuration = 0;

    // dec delay & timeout (if necessary)
    data->cntDelay   -= (data->cntDelay > 0 ? 1 : 0);
    data->cntTimeout -= (data->cntTimeout > 0 ? 1 : 0);
    
    // reset shake count after timeout
    if (data->cntTimeout <= 0)
    {
        data->cntShake   = 0;
        data->cntDelay   = 0;
        data->cntTimeout = 0;
    }

    return data->cntShake;
}

/*==================================================================================================
FUNCTION: 		KIONIX_ACCEL_deinit
DESCRIPTION:    	This function de-initializes the Kionix accelerometer. 
ARGUMENTS PASSED:   	None
RETURN VALUE:   	0 = pass; 1 = fail
PRE-CONDITIONS:   	None
POST-CONDITIONS:   	Acceleration data outputs are disabled
==================================================================================================*/
int KIONIX_ACCEL_deinit(void)
{
	int res=0;
	
	res = KIONIX_ACCEL_disable_outputs();
	KIONIX_ACCEL_disable_interrupt();
	res |= KIONIX_ACCEL_sleep();


	return res;
}
/*==================================================================================================
FUNCTION: 		KIONIX_ACCEL_init
DESCRIPTION:    	This function initializes the Kionix accelerometer. 
ARGUMENTS PASSED:   	None
RETURN VALUE:   	0 = pass; 1 = fail
PRE-CONDITIONS:   	None
POST-CONDITIONS:   	Acceleration data outputs are enabled
==================================================================================================*/
int KIONIX_ACCEL_init(void)
{
	int status = 0;
//	unsigned char ctrl_reg1=0, ctrl_reg2=0, ctrl_reg3=0, int_ctrl_reg1=0, int_ctrl_reg3=0;
	unsigned char who_am_i=0, value=0;

// Need 50ms until the RAM load is finished after Power-up      
	msleep(50);

	//determine what hardware is included by reading the WHO_AM_I register
	if (KIONIX_ACCEL_read_bytes(KIONIX_ACCEL_I2C_WHO_AM_I, &who_am_i, 1) == 0){
		switch(who_am_i){
			//KXTF9 initialization
			case 0x01:
			case 0x4E:
				printk("[%s:%d] -I- Initializing device: KXTF9 (0x%x) \n", __FUNCTION__,__LINE__, who_am_i);
				device = 1;

				// Read testing for i2c corretly wokring check...
				KIONIX_ACCEL_read_bytes(KIONIX_ACCEL_I2C_CTRL_REG1, &value, 1);
					printk("-CTRL_REG1(%x)     = 0x%x (default = 0x00)\n", KIONIX_ACCEL_I2C_CTRL_REG1, value );
				KIONIX_ACCEL_read_bytes(KIONIX_ACCEL_I2C_CTRL_REG2, &value, 1);
					printk("-CTRL_REG2(%x)     = 0x%x (default = 0x3f)\n", KIONIX_ACCEL_I2C_CTRL_REG2,value);
				KIONIX_ACCEL_read_bytes(KIONIX_ACCEL_I2C_CTRL_REG3, &value, 1);
					printk("-CTRL_REG3(%x)     = 0x%x (default = 0x4d)\n", KIONIX_ACCEL_I2C_CTRL_REG3,value );
				KIONIX_ACCEL_read_bytes(KXTF9_I2C_INT_CTRL_REG1, &value, 1);
					printk("-INT_CTRL_REG1(%x) = 0x%x (default = 0x10)\n",KXTF9_I2C_INT_CTRL_REG1, value );
				KIONIX_ACCEL_read_bytes(KXTF9_I2C_INT_CTRL_REG3, &value, 1);
					printk("-INT_CTRL_REG3(%x) = 0x%x (default = 0x3f)\n", KXTF9_I2C_INT_CTRL_REG3,value);


				status |= KXTF9_set_G_range(2);
				status |= KXTF9_set_resolution(12);
				
				status |= KXTF9_set_odr_tap(400);
				status |= KXTF9_set_odr_tilt(12);
				status |= KIONIX_ACCEL_set_odr_motion(50);

				status |= KIONIX_ACCEL_int_activel();
				status |= KIONIX_ACCEL_int_latch();
				status |= KIONIX_ACCEL_disable_all();    // test
				break;

			//KXTE9 initialization
			case 0x00:
				printk("[%s:%d] -I- Initializing device: KXTE9 (0x%x) \n", __FUNCTION__,__LINE__, who_am_i);
				device = 0;

				status = 0;
				break;
			default:
				status = 1;				
				break;
		}
	}
	else {
		printk("[%s:%d] -ERR- Not found KXT_9 Accel. device (0x%x) \n",__FUNCTION__,__LINE__, who_am_i);
		status = 1;
	}

	return status;
	
}

