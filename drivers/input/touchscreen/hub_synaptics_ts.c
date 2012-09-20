/* drivers/input/touchscreen/hub_synaptics_ts.c
 *
 * Copyright (C) 2011 LG Electronics, Inc.
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
 *
 * 2011-02-14 Kim Sunghee  <seven.kim@lge.com>
 *
 * 2011-03-02 Choi Daewan  <ntdeaewan.choi@lge.com>
 *
 */

#include <linux/module.h>
#include <linux/delay.h>
#include <linux/earlysuspend.h>
#include <linux/hrtimer.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#include <mach/gpio.h>
#include <linux/jiffies.h>

#include <linux/workqueue.h>
#include <linux/delay.h>
#include <linux/wakelock.h>

#include <linux/slab.h>

//--[[ LGE_UBIQUIX_MODIFIED_START : shyun@ubiquix.com [2012.03.20] - Not included
/* 20110331 sookyoung.kim@lge.com LG-DVFS [START_LGE] */
//#include <linux/dvs_suite.h>
/* 20110331 sookyoung.kim@lge.com LG-DVFS [END_LGE] */
//--]] LGE_UBIQUIX_MODIFIED_END : shyun@ubiquix.com [2012.03.20]- Not included

#include "synaptics_ts_firmware.h"
#include "synaptics_ts_firmware_lgit.h"
#define SYNAPTICS_SUPPORT_FW_UPGRADE

#if 0
#define pr_debug(fmt, ...) \
	printk(KERN_DEBUG pr_fmt(fmt), ##__VA_ARGS__)
#endif

#if 0
#define FEATURE_LGE_TOUCH_MOVING_IMPROVE
#define FEATURE_LGE_TOUCH_JITTERING_IMPROVE
#endif
#define FEATURE_LGE_TOUCH_GHOST_FINGER_IMPROVE
#define FEATURE_LGE_TOUCH_GRIP_SUPPRESSION

#define FEATURE_LGE_TOUCH_REAL_TIME_WORK_QUEUE
#define FEATURE_LGE_TOUCH_ESD_DETECT
/*===========================================================================
                DEFINITIONS AND DECLARATIONS FOR MODULE

This section contains definitions for constants, macros, types, variables
and other items needed by this module.
===========================================================================*/

static struct workqueue_struct *synaptics_wq;
static struct i2c_client *hub_ts_client = NULL;

struct synaptics_ts_priv {
	uint16_t addr;
	struct i2c_client *client;
	struct input_dev *input_dev;
	int use_irq;
	bool has_relative_report;
	struct hrtimer timer;
	struct work_struct  work;
	uint16_t max[2];

	uint32_t flags;
	int reported_finger_count;
	int8_t sensitivity_adjust;
	int (*power)(int on);

	unsigned int count;
	int x_lastpt;
	int y_lastpt;

	struct early_suspend early_suspend;

	unsigned int interval;
	struct delayed_work init_delayed_work;
	unsigned char product_value; //product_value=0:misung panel  product_value=1 : LGIT panel
};
struct synaptics_ts_priv *p_ts;

enum key_leds {
	MENU,
	HOME,
	BACK,
	SEARCH,
};

static int init_stabled = -1;
//--[[ LGE_UBIQUIX_MODIFIED_START : shyun@ubiquix.com [2012.03.20] - TBD
int lcd_off_boot = 0;
//--]] LGE_UBIQUIX_MODIFIED_END : shyun@ubiquix.com [2012.03.20]- TBD

#ifdef CONFIG_HAS_EARLYSUSPEND
static void synaptics_ts_early_suspend(struct early_suspend *h);
static void synaptics_ts_late_resume(struct early_suspend *h);
#endif

#define TOUCH_INT_N_GPIO						35

extern void bd2802_touch_on(void);
extern void bd2802_touch_timer_restart(void);
extern u8 key_led_flag;

/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/*                                                                         */
/*                                 Macros                                  */
/*                                                                         */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

#define TS_SNTS_GET_FINGER_STATE_0(finger_status_reg) \
		(finger_status_reg&0x03)
#define TS_SNTS_GET_FINGER_STATE_1(finger_status_reg) \
		((finger_status_reg&0x0C)>>2)
#define TS_SNTS_GET_FINGER_STATE_2(finger_status_reg) \
		((finger_status_reg&0x30)>>4)
#define TS_SNTS_GET_FINGER_STATE_3(finger_status_reg) \
      ((finger_status_reg&0xC0)>>6)
#define TS_SNTS_GET_FINGER_STATE_4(finger_status_reg) \
      (finger_status_reg&0x03)

#define TS_SNTS_GET_X_POSITION(high_reg, low_reg) \
		((int)(high_reg*0x10) + (int)(low_reg&0x0F))
#define TS_SNTS_GET_Y_POSITION(high_reg, low_reg) \
		((int)(high_reg*0x10) + (int)((low_reg&0xF0)/0x10))

#define TS_SNTS_HAS_PINCH(gesture_reg) \
		((gesture_reg&0x40)>>6)
#define TS_SNTS_HAS_FLICK(gesture_reg) \
		((gesture_reg&0x10)>>4)
#define TS_SNTS_HAS_DOUBLE_TAP(gesture_reg) \
		((gesture_reg&0x04)>>2)

#define TS_SNTS_GET_REPORT_RATE(device_control_reg) \
		((device_control_reg&0x40)>>6)
// 1st bit : '0' - Allow sleep mode, '1' - Full power without sleeping
// 2nd and 3rd bit : 0x00 - Normal Operation, 0x01 - Sensor Sleep
#define TS_SNTS_GET_SLEEP_MODE(device_control_reg) \
		(device_control_reg&0x07)


/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/*                                                                         */
/*                       CONSTANTS DATA DEFINITIONS                        */
/*                                                                         */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

#define TOUCH_EVENT_NULL						0
#define TOUCH_EVENT_BUTTON						1
#define TOUCH_EVENT_ABS							2



#define SYNAPTICS_FINGER_MAX					5


#define SYNAPTICS_TM1576_PRODUCT_ID				"TM1576"
#define SYNAPTICS_TM1576_RESOLUTION_X			1036
#define SYNAPTICS_TM1576_RESOLUTION_Y			1976
#define SYNAPTICS_TM1576_LCD_ACTIVE_AREA		1728
#define SYNAPTICS_TM1576_BUTTON_ACTIVE_AREA		1828

#define SYNAPTICS_TM1702_PRODUCT_ID				"TM1702"
#define SYNAPTICS_TM1702_RESOLUTION_X			1036
#define SYNAPTICS_TM1702_RESOLUTION_Y			1896
#define SYNAPTICS_TM1702_LCD_ACTIVE_AREA		1728
#define SYNAPTICS_TM1702_BUTTON_ACTIVE_AREA		1805

#define SYNAPTICS_TM1738_PRODUCT_ID				"TM1738"
#define SYNAPTICS_TM1738_RESOLUTION_X			1036
#define SYNAPTICS_TM1738_RESOLUTION_Y			1896
#define SYNAPTICS_TM1738_LCD_ACTIVE_AREA		1728
#define SYNAPTICS_TM1738_BUTTON_ACTIVE_AREA		1805



/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/*                                                                         */
/*                    REGISTER ADDR & SETTING VALUE                        */
/*                                                                         */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

#define SYNAPTICS_FLASH_CONTROL_REG				0x12
#define SYNAPTICS_DATA_BASE_REG					0x13
#define SYNAPTICS_INT_STATUS_REG				0x14

#define SYNAPTICS_CONTROL_REG					0x4C
#define SYNAPTICS_RIM_CONTROL_INTERRUPT_ENABLE	0x4D

#define SYNAPTICS_DELTA_X_THRES_REG				0x50
#define SYNAPTICS_DELTA_Y_THRES_REG				0x51

#define SYNAPTICS_2D_GESTURE_ENABLES_2			0x59

#define SYNAPTICS_FW_REVISION_REG				0xAD

#define SYNAPTICS_RMI_QUERY_BASE_REG			0xE3
#define SYNAPTICS_RMI_CMD_BASE_REG				0xE4
#define SYNAPTICS_FLASH_QUERY_BASE_REG			0xE9
#define SYNAPTICS_FLASH_DATA_BASE_REG			0xEC

#define SYNAPTICS_INT_FLASH						1<<0
#define SYNAPTICS_INT_STATUS					1<<1
#define SYNAPTICS_INT_ABS0						1<<2

#define SYNAPTICS_CONTROL_SLEEP					1<<0
#define SYNAPTICS_CONTROL_NOSLEEP				1<<2

#ifdef FEATURE_LGE_TOUCH_ESD_DETECT
#define SYNAPTICS_CONTROL_CONFIGURED			1<<7
#define SYNAPTICS_RIM_DEVICE_RESET				1<<0
#endif /*FEATURE_LGE_TOUCH_ESD_DETECT*/

#ifdef SYNAPTICS_SUPPORT_FW_UPGRADE
#define SYNAPTICS_FLASH_CMD_FW_CRC				0x01
#define SYNAPTICS_FLASH_CMD_FW_WRITE			0x02
#define SYNAPTICS_FLASH_CMD_ERASEALL			0x03
#define SYNAPTICS_FLASH_CMD_CONFIG_READ			0x05
#define SYNAPTICS_FLASH_CMD_CONFIG_WRITE		0x06
#define SYNAPTICS_FLASH_CMD_CONFIG_ERASE		0x07
#define SYNAPTICS_FLASH_CMD_ENABLE				0x0F
#define SYNAPTICS_FLASH_NORMAL_RESULT			0x80
#define FW_IMAGE_SIZE 	28929
unsigned char SynapticsFirmware[FW_IMAGE_SIZE];
#endif /* SYNAPTICS_SUPPORT_FW_UPGRADE */

#define SYNAPTICS_TS_SENSITYVITY_REG		0x9B
#define SYNAPTICS_TS_SENSITYVITY_VALUE		0x00

/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/*                                                                         */
/*                         DATA DEFINITIONS                                */
/*                                                                         */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

typedef struct {
	unsigned char m_QueryBase;
	unsigned char m_CommandBase;
	unsigned char m_ControlBase;
	unsigned char m_DataBase;
	unsigned char m_IntSourceCount;
	unsigned char m_FunctionExists;
} T_RMI4FuncDescriptor;

typedef struct
{
	unsigned char device_status_reg;						//0x13
	unsigned char interrupt_status_reg;						//0x14
	unsigned char finger_state_reg[3];						//0x15~0x17

	unsigned char fingers_data[SYNAPTICS_FINGER_MAX][5];	//0x18 ~ 0x49
	/* 5 data per 1 finger, support 10 fingers data
	fingers_data[x][0] : xth finger's X high position
	fingers_data[x][1] : xth finger's Y high position
	fingers_data[x][2] : xth finger's XY low position
	fingers_data[x][3] : xth finger's XY width
	fingers_data[x][4] : xth finger's Z (pressure)
	*/
	// Etc...
	unsigned char gesture_flag0;							//0x4A
	unsigned char gesture_flag1;							//0x4B
#if 0
	unsigned char pinch_motion_X_flick_distance;			//0x4C
	unsigned char rotation_motion_Y_flick_distance;		//0x4D
	unsigned char finger_separation_flick_time;			//0x4E
#endif
} ts_sensor_data;

typedef struct {
	unsigned char touch_status[SYNAPTICS_FINGER_MAX];
	unsigned int X_position[SYNAPTICS_FINGER_MAX];
	unsigned int Y_position[SYNAPTICS_FINGER_MAX];
	unsigned char width[SYNAPTICS_FINGER_MAX];
	unsigned char pressure[SYNAPTICS_FINGER_MAX];
} ts_finger_data;


static ts_sensor_data ts_reg_data;
static ts_finger_data prev_ts_data;
static ts_finger_data curr_ts_data;

static uint8_t curr_event_type = TOUCH_EVENT_NULL;
static uint8_t prev_event_type = TOUCH_EVENT_NULL;

static uint16_t pressed_button_type = KEY_REJECT;

static uint16_t SYNAPTICS_PANEL_MAX_X;
static uint16_t SYNAPTICS_PANEL_MAX_Y;
static uint16_t SYNAPTICS_PANEL_LCD_MAX_Y;
static uint16_t SYNAPTICS_PANEL_BUTTON_MIN_Y;

unsigned char  touch_fw_version = 0;

struct wake_lock ts_wake_lock;

/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/*                                                                         */
/*                           Local Functions                               */
/*                                                                         */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

#ifdef FEATURE_LGE_TOUCH_MOVING_IMPROVE
#define ADJUST_VAL				4
#define ADJUST_BASIS_VALUE		20
#define Square(x) ((x) * (x))

static void touch_adjust_position(int finger_num)
{
	if(prev_ts_data.touch_status[finger_num])
	{
		if( (Square(prev_ts_data.X_position[finger_num] - curr_ts_data.X_position[finger_num]) + Square(prev_ts_data.Y_position[finger_num] - curr_ts_data.Y_position[finger_num])) <= Square(ADJUST_BASIS_VALUE) )
		{
			curr_ts_data.X_position[finger_num] = (prev_ts_data.X_position[finger_num] * ADJUST_VAL + curr_ts_data.X_position[finger_num]) / (ADJUST_VAL + 1);
			curr_ts_data.Y_position[finger_num] = (prev_ts_data.Y_position[finger_num] * ADJUST_VAL + curr_ts_data.Y_position[finger_num]) / (ADJUST_VAL + 1);
		}
	}
}
#endif /* FEATURE_LGE_TOUCH_MOVING_IMPROVE */

#ifdef FEATURE_LGE_TOUCH_GHOST_FINGER_IMPROVE
#define MELT_CONTROL			0xF0
#define NO_MELT 				0x00
#define MELT					0x01
#define AUTO_MELT				0x10

static u8 melt_mode = 0;
static u8 melt_flag = 0;
//static int ts_pre_state = 0; /* for checking the touch state */
static u8 ghost_finger_1 = 0; // remove for ghost finger
static u8 ghost_finger_2 = 0;
static u8 pressed = 0;
static unsigned long pressed_time;
#endif

#ifdef FEATURE_LGE_TOUCH_ESD_DETECT
extern int aat2870_ldo_write(u8 reg , u8 val);
static bool  synatics_ts_touch_recovery(void);
#endif /*FEATURE_LGE_TOUCH_ESD_DETECT*/

//extern int lcd_off_boot;

static void synaptics_ts_initialize(void)
{
	int ret;
	printk("%s : %d\n",__func__,__LINE__);
	
	if(!gpio_get_value(TOUCH_INT_N_GPIO))
	{
		ret = i2c_smbus_read_i2c_block_data(p_ts->client, SYNAPTICS_DATA_BASE_REG, sizeof(ts_reg_data), (u8 *)&ts_reg_data);
		if(ret<0)
		{
			printk("[touch] [fail] SYNAPTICS_DATA_BASE_REG READ\n");
		}
	}

	ret = i2c_smbus_write_byte_data(p_ts->client, SYNAPTICS_RIM_CONTROL_INTERRUPT_ENABLE, 0x00); //interrupt disable
	if(ret<0)
	{
		printk("[touch] [fail] SYNAPTICS_RIM_CONTROL_INTERRUPT_ENABLE\n");
	}
	disable_irq(p_ts->client->irq);

	ret = i2c_smbus_write_byte_data(p_ts->client, SYNAPTICS_CONTROL_REG, SYNAPTICS_CONTROL_NOSLEEP);
	if(ret<0)
	{
		printk("[touch] [fail] SYNAPTICS_CONTROL_CONFIGURED, SYNAPTICS_CONTROL_NOSLEEP\n");
	}
	
//--[[ LGE_UBIQUIX_MODIFIED_START : shyun@ubiquix.com [2011.09.09] - Merge from Black_Froyo MR Ver.
#if 0
	ret = i2c_smbus_write_byte_data(p_ts->client, SYNAPTICS_2D_GESTURE_ENABLES_2, 0x00);
	if(ret<0)
	{
		printk("[touch] [fail] SYNAPTICS_2D_GESTURE_ENABLES_2\n");
	}
#endif	
//--]] LGE_UBIQUIX_MODIFIED_END : shyun@ubiquix.com [2011.09.09]- Merge from Black_Froyo MR Ver.
	
	ret = i2c_smbus_read_i2c_block_data(p_ts->client, SYNAPTICS_DATA_BASE_REG, sizeof(ts_reg_data), (u8 *)&ts_reg_data);
	if(ret<0)
	{
		printk("[touch] [fail] SYNAPTICS_DATA_BASE_REG READ\n");
	}
		
	ret = i2c_smbus_write_byte_data(p_ts->client, SYNAPTICS_TS_SENSITYVITY_REG, SYNAPTICS_TS_SENSITYVITY_VALUE);
	if(ret<0)
	{
		printk("[touch] [fail] SYNAPTICS_TS_SENSITYVITY_VALUE\n");
	}

	ret = i2c_smbus_write_byte_data(p_ts->client, SYNAPTICS_DELTA_X_THRES_REG, 0x03);
	if(ret<0)
	{
		printk("[touch] [fail] SYNAPTICS_DELTA_X_THRES_REG\n");
	}
	
	ret = i2c_smbus_write_byte_data(p_ts->client, SYNAPTICS_DELTA_Y_THRES_REG, 0x03);
	if(ret<0)
	{
		printk("[touch] [fail] SYNAPTICS_DELTA_Y_THRES_REG\n");
	}
	
#ifdef FEATURE_LGE_TOUCH_GHOST_FINGER_IMPROVE
	ret = i2c_smbus_write_byte_data(p_ts->client, MELT_CONTROL, MELT);
	if(ret<0)
	{
		printk("[touch] [fail] MELT_CONTROL, MELT\n");
	}
#endif

	enable_irq(p_ts->client->irq);
//--[[ LGE_UBIQUIX_MODIFIED_START : shyun@ubiquix.com [2011.09.08] - Prevent duplication of enable_irq api calling.(Remove the warning log)
	init_stabled = 1;
//--]] LGE_UBIQUIX_MODIFIED_END : shyun@ubiquix.com [2011.09.08]- Prevent duplication of enable_irq api calling.(Remove the warning log)
	ret = i2c_smbus_write_byte_data(p_ts->client, SYNAPTICS_RIM_CONTROL_INTERRUPT_ENABLE, 0x07); //interrupt enable
	if(ret<0)
	{
		printk("[touch] [fail] SYNAPTICS_RIM_CONTROL_INTERRUPT_ENABLE\n");
		synatics_ts_touch_recovery();
		mdelay(20);
		i2c_smbus_write_byte_data(p_ts->client, SYNAPTICS_RIM_CONTROL_INTERRUPT_ENABLE, 0x07);
	}
	
	return;
}

static u8 dummy;
static u8 ts_esd_detect_flag=0;
static u8 ts_esd_reset=0;
static u8 ts_recovery_count=0;
static u8 ts_keyled_count = 0;

static void synaptics_ts_init_delayed_work(struct work_struct *work)
{
	int ts_delay_work_flag, ret;

	local_irq_save(ts_delay_work_flag);

	if(ts_esd_detect_flag)
	{
//--[[ LGE_UBIQUIX_MODIFIED_START : shyun@ubiquix.com [2011.09.09] - Merge from Black_Froyo MR Ver.
		//synatics_ts_touch_recovery();
//--]] LGE_UBIQUIX_MODIFIED_END : shyun@ubiquix.com [2011.09.09]- Merge from Black_Froyo MR Ver.
		ts_esd_detect_flag=0;
		ts_esd_reset=0;
		printk("[touch] ESD detect && recovery !!\n");
	}

	if(init_stabled==-1)	enable_irq(p_ts->client->irq);
	
	melt_flag=1;
	init_stabled = 1;
	local_irq_restore(ts_delay_work_flag);
//--[[ LGE_UBIQUIX_MODIFIED_START : shyun@ubiquix.com [2011.09.09] - Merge from Black_Froyo MR Ver.
	//p_ts->interval = msecs_to_jiffies(500);
   	//schedule_delayed_work(&p_ts->init_delayed_work, p_ts->interval);
//--]] LGE_UBIQUIX_MODIFIED_END : shyun@ubiquix.com [2011.09.09]- Merge from Black_Froyo MR Ver.
}

#ifdef FEATURE_LGE_TOUCH_GRIP_SUPPRESSION
static int g_gripIgnoreRangeValue = 0;
static int g_receivedPixelValue = 0;

static int touch_ConvertPixelToRawData(int pixel)
{
	int result = 0;

	result = (SYNAPTICS_PANEL_MAX_X * pixel) /480;

	return result;
}

void synaptics_ts_ldo_write(u8 reg , u8 val)
{
  int ret;

  ret = i2c_smbus_write_byte_data(hub_ts_client, reg , val);

  return;
}

ssize_t touch_gripsuppression_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	sprintf(buf, "%d\n", g_receivedPixelValue);
	printk("[kernel] [touch] SHOW (%d) \n", g_receivedPixelValue);

	return (ssize_t)(strlen(buf)+1);

}

ssize_t touch_gripsuppression_store(struct device *dev, struct device_attribute *attr, const char *buffer, size_t count)
{
	sscanf(buffer, "%d", &g_receivedPixelValue);
	g_gripIgnoreRangeValue = touch_ConvertPixelToRawData(g_receivedPixelValue);
	printk("[kernel] [touch] STORE  pixel(%d) convet (%d) \n", g_receivedPixelValue, g_gripIgnoreRangeValue);

	return count;
}

DEVICE_ATTR(gripsuppression, 0666, touch_gripsuppression_show, touch_gripsuppression_store);
#endif /* FEATURE_LGE_TOUCH_GRIP_SUPPRESSION */

static u8 ts_i=0;
static u8 finger_count=0;
static u8 ts_ret=0;
static u8 ts_finger=0;
static u8 ghost_count=0;
static int ts_check=0;
static u8 ts_esd_detect_count=0;

static void synaptics_ts_work_func(struct work_struct *work)
{
	p_ts = container_of(work, struct synaptics_ts_priv, work);

//--[[ LGE_UBIQUIX_MODIFIED_START : shyun@ubiquix.com [2011.09.09] - Merge from Black_Froyo MR Ver.
#if 0
do
{
#endif
//--]] LGE_UBIQUIX_MODIFIED_END : shyun@ubiquix.com [2011.09.09]- Merge from Black_Froyo MR Ver.
	ts_ret = i2c_smbus_read_i2c_block_data(p_ts->client, SYNAPTICS_DATA_BASE_REG, sizeof(ts_reg_data), (u8 *)&ts_reg_data);

		for(ts_i = 0; ts_i < SYNAPTICS_FINGER_MAX; ts_i++)
		{
			ts_check = 1 << ((ts_i%4)*2);
			ts_finger = (u8)(ts_i/4);

			if((ts_reg_data.finger_state_reg[ts_finger] & ts_check) == ts_check)
			{
				curr_ts_data.X_position[ts_i] = (int)TS_SNTS_GET_X_POSITION(ts_reg_data.fingers_data[ts_i][0], ts_reg_data.fingers_data[ts_i][2]);
				curr_ts_data.Y_position[ts_i] = (int)TS_SNTS_GET_Y_POSITION(ts_reg_data.fingers_data[ts_i][1], ts_reg_data.fingers_data[ts_i][2]);
#ifdef FEATURE_LGE_TOUCH_MOVING_IMPROVE
				touch_adjust_position(ts_i);
#endif

#ifdef FEATURE_LGE_TOUCH_JITTERING_IMPROVE
				if(!(abs(curr_ts_data.X_position[ts_i]-prev_ts_data.X_position[ts_i]) > 1 && abs(curr_ts_data.Y_position[ts_i]-prev_ts_data.Y_position[ts_i]) > 1))
				{
					curr_ts_data.X_position[ts_i] = prev_ts_data.X_position[ts_i];
					curr_ts_data.Y_position[ts_i] = prev_ts_data.Y_position[ts_i];
				}
#endif

#ifdef FEATURE_LGE_TOUCH_GRIP_SUPPRESSION
				if ( (g_gripIgnoreRangeValue > 0) && ( (curr_ts_data.X_position[ts_i] <= g_gripIgnoreRangeValue ) ||
															(curr_ts_data.X_position[ts_i] >= (SYNAPTICS_PANEL_MAX_X - g_gripIgnoreRangeValue) )) )
				{
					//printk("[touch] Girp Region Pressed. IGNORE!!! value : %d\n",g_gripIgnoreRangeValue);
				}
				else
				{
#endif
					if ((((ts_reg_data.fingers_data[ts_i][3] & 0xf0) >> 4) - (ts_reg_data.fingers_data[ts_i][3] & 0x0f)) > 0)
						curr_ts_data.width[ts_i] = (ts_reg_data.fingers_data[ts_i][3] & 0xf0) >> 4;
					else
						curr_ts_data.width[ts_i] = ts_reg_data.fingers_data[ts_i][3] & 0x0f;

					curr_ts_data.pressure[ts_i] = ts_reg_data.fingers_data[ts_i][4];
					curr_ts_data.touch_status[ts_i] = 1;
					finger_count++;
				}
			}
			else
			{
				curr_ts_data.touch_status[ts_i] = 0;
			}

			if(curr_ts_data.touch_status[ts_i])
			{
				if(finger_count == 1 && !ts_i)
				{
#ifdef FEATURE_LGE_TOUCH_GHOST_FINGER_IMPROVE
					 if(!pressed) {
					 	if(curr_ts_data.X_position[0]>60 && curr_ts_data.X_position[0]<970)
					 	{
							pressed_time = jiffies;
							ghost_finger_1 = 1;
					 		pressed++;
					 	}
					 }
#endif
					if((curr_ts_data.Y_position[ts_i] < SYNAPTICS_PANEL_LCD_MAX_Y && prev_event_type == TOUCH_EVENT_NULL) || prev_event_type == TOUCH_EVENT_ABS)
						curr_event_type = TOUCH_EVENT_ABS;
					else if((curr_ts_data.Y_position[ts_i] >= SYNAPTICS_PANEL_LCD_MAX_Y && prev_event_type == TOUCH_EVENT_NULL) || prev_event_type == TOUCH_EVENT_BUTTON)
						curr_event_type = TOUCH_EVENT_BUTTON;

					if(curr_event_type == TOUCH_EVENT_ABS)
					{
						if(curr_ts_data.Y_position[ts_i] < SYNAPTICS_PANEL_LCD_MAX_Y)
						{
							input_report_abs(p_ts->input_dev, ABS_MT_POSITION_X, curr_ts_data.X_position[ts_i]);
							input_report_abs(p_ts->input_dev, ABS_MT_POSITION_Y, curr_ts_data.Y_position[ts_i]);
							input_report_abs(p_ts->input_dev, ABS_MT_TOUCH_MAJOR, curr_ts_data.pressure[ts_i]);
							input_report_abs(p_ts->input_dev, ABS_MT_WIDTH_MAJOR, curr_ts_data.width[ts_i]);
							input_report_key(p_ts->input_dev, BTN_TOUCH, true);
							input_mt_sync(p_ts->input_dev);
							//printk("[touch] (%d, %d),  melt mode=%d\n", curr_ts_data.X_position[0], curr_ts_data.Y_position[0],melt_mode);
						}
					}
					else if(curr_event_type == TOUCH_EVENT_BUTTON)
					{
						if(curr_ts_data.Y_position[ts_i] > SYNAPTICS_PANEL_BUTTON_MIN_Y)
						{
							if(curr_ts_data.X_position[ts_i] > 35 && curr_ts_data.X_position[ts_i] < 245) //center 75
							{
								if(!prev_ts_data.touch_status[ts_i])
								{
									printk("[key-touch] [KEY_MENU]\n");
									input_report_key(p_ts->input_dev, KEY_MENU, 1); //seven blocked for key drag action
									pressed_button_type = KEY_MENU;
								}
								else
								{
									if(pressed_button_type != KEY_MENU && pressed_button_type != KEY_REJECT)
									{
										printk("[key-touch] [KEY_MENU]\n");
										input_report_key(p_ts->input_dev, KEY_REJECT, 1);
										input_report_key(p_ts->input_dev, KEY_REJECT, 0);
										input_report_key(p_ts->input_dev, pressed_button_type, 0);
										pressed_button_type = KEY_REJECT;
									}
								}
							}
							else if(curr_ts_data.X_position[ts_i] > 287 && curr_ts_data.X_position[ts_i] < 497) //center 185
							{
								if(!prev_ts_data.touch_status[ts_i])
								{
									printk("[key-touch] [KEY_HOME]\n");
									input_report_key(p_ts->input_dev, KEY_HOME, 1); //seven blocked for key drag action
									pressed_button_type = KEY_HOME;
								}
								else
								{
									if(pressed_button_type != KEY_HOME && pressed_button_type != KEY_REJECT)
									{
										printk("[key-touch] [KEY_HOME]\n");
										input_report_key(p_ts->input_dev, KEY_REJECT, 1);
										input_report_key(p_ts->input_dev, KEY_REJECT, 0);
										input_report_key(p_ts->input_dev, pressed_button_type, 0);
										pressed_button_type = KEY_REJECT;
									}
								}
							}
							else if(curr_ts_data.X_position[ts_i] > 539 && curr_ts_data.X_position[ts_i] < 749) //center 295
							{
								if(!prev_ts_data.touch_status[ts_i])
								{
									printk("[key-touch] [KEY_BACK]\n");
									input_report_key(p_ts->input_dev, KEY_BACK, 1); //seven blocked for key drag action
									pressed_button_type = KEY_BACK;
								}
								else
								{
									if(pressed_button_type != KEY_BACK && pressed_button_type != KEY_REJECT)
									{
										printk("[key-touch] [KEY_BACK]\n");
										input_report_key(p_ts->input_dev, KEY_REJECT, 1);
										input_report_key(p_ts->input_dev, KEY_REJECT, 0);
										input_report_key(p_ts->input_dev, pressed_button_type, 0);
										pressed_button_type = KEY_REJECT;
									}
								}
							}
							else if(curr_ts_data.X_position[ts_i] > 791 && curr_ts_data.X_position[ts_i] < 1001) //center 405
							{
								if(!prev_ts_data.touch_status[ts_i])
								{
									printk("[key-touch] [KEY_SERACH]\n");
									input_report_key(p_ts->input_dev, KEY_SEARCH, 1); //seven blocked for key drag action
									pressed_button_type = KEY_SEARCH;
								}
								else
								{
									if(pressed_button_type != KEY_SEARCH && pressed_button_type != KEY_REJECT)
									{
										printk("[key-touch] [KEY_SEARCH]\n");
										input_report_key(p_ts->input_dev, KEY_REJECT, 1);
										input_report_key(p_ts->input_dev, KEY_REJECT, 0);
										input_report_key(p_ts->input_dev, pressed_button_type, 0);
										pressed_button_type = KEY_REJECT;
									}
								}
							}
							else
							{
								if(!prev_ts_data.touch_status[ts_i])
								{
									pressed_button_type = KEY_REJECT;
								}
								else
								{
									if(pressed_button_type != KEY_REJECT)
									{
										input_report_key(p_ts->input_dev, KEY_REJECT, 1);
										input_report_key(p_ts->input_dev, KEY_REJECT, 0);
										input_report_key(p_ts->input_dev, pressed_button_type, 0);
										pressed_button_type = KEY_REJECT;
									}
								}
							}
						}
						else
						{
							if(!prev_ts_data.touch_status[ts_i])
							{
								pressed_button_type = KEY_REJECT;
							}
							else
							{
								if(pressed_button_type != KEY_REJECT)
								{
									input_report_key(p_ts->input_dev, KEY_REJECT, 1);
									input_report_key(p_ts->input_dev, KEY_REJECT, 0);
									input_report_key(p_ts->input_dev, pressed_button_type, 0);
									pressed_button_type = KEY_REJECT;
								}
							}

								input_report_abs(p_ts->input_dev, ABS_MT_POSITION_X, curr_ts_data.X_position[ts_i]);
								input_report_abs(p_ts->input_dev, ABS_MT_POSITION_Y, curr_ts_data.Y_position[ts_i]);
								input_report_abs(p_ts->input_dev, ABS_MT_TOUCH_MAJOR, curr_ts_data.pressure[ts_i]);
								input_report_abs(p_ts->input_dev, ABS_MT_WIDTH_MAJOR, curr_ts_data.width[ts_i]);
								input_report_key(p_ts->input_dev, BTN_TOUCH, true);								

								input_mt_sync(p_ts->input_dev);

								curr_event_type = TOUCH_EVENT_ABS;
						}
					}
					else
					{
						curr_event_type = TOUCH_EVENT_NULL;
						pressed_button_type = KEY_REJECT;
					}
				}
				else // multi-finger
				{
					curr_event_type = TOUCH_EVENT_ABS;

					if(pressed_button_type != KEY_REJECT)
					{
						input_report_key(p_ts->input_dev, KEY_REJECT, 1);
						input_report_key(p_ts->input_dev, KEY_REJECT, 0);
						input_report_key(p_ts->input_dev, pressed_button_type, 0);
						pressed_button_type = KEY_REJECT;
					}

					if(curr_ts_data.Y_position[ts_i] < SYNAPTICS_PANEL_LCD_MAX_Y)
					{
						input_report_abs(p_ts->input_dev, ABS_MT_POSITION_X, curr_ts_data.X_position[ts_i]);
						input_report_abs(p_ts->input_dev, ABS_MT_POSITION_Y, curr_ts_data.Y_position[ts_i]);
						input_report_abs(p_ts->input_dev, ABS_MT_TOUCH_MAJOR, curr_ts_data.pressure[ts_i]);
						input_report_abs(p_ts->input_dev, ABS_MT_WIDTH_MAJOR, curr_ts_data.width[ts_i]);
						input_report_key(p_ts->input_dev, BTN_TOUCH, true);						

						input_mt_sync(p_ts->input_dev);
						
						if(curr_ts_data.X_position[1] || curr_ts_data.Y_position[1]) ghost_count=0;
					}
				}
			}
			else
			{
				if(pressed_button_type != KEY_REJECT && !ts_i)
				{
					input_report_key(p_ts->input_dev, pressed_button_type, 0);
					pressed_button_type = KEY_REJECT;
				}
			}
			
			prev_ts_data.touch_status[ts_i] = curr_ts_data.touch_status[ts_i];
			prev_ts_data.X_position[ts_i] = curr_ts_data.X_position[ts_i];
			prev_ts_data.Y_position[ts_i] = curr_ts_data.Y_position[ts_i];
			prev_ts_data.width[ts_i] = curr_ts_data.width[ts_i];
			prev_ts_data.pressure[ts_i] = curr_ts_data.pressure[ts_i];
		}

		if(!finger_count)
		{
			prev_event_type = TOUCH_EVENT_NULL;
		}
		else
		{
			prev_event_type = curr_event_type;
		}

#ifdef FEATURE_LGE_TOUCH_GHOST_FINGER_IMPROVE
		if(!melt_mode && melt_flag) 
		{
			if(pressed) 
			{
				if( TS_SNTS_GET_FINGER_STATE_1(ts_reg_data.finger_state_reg[0]) == 1 ||
					TS_SNTS_GET_FINGER_STATE_2(ts_reg_data.finger_state_reg[0]) == 1 ||
					TS_SNTS_GET_FINGER_STATE_3(ts_reg_data.finger_state_reg[0]) == 1 ||
					TS_SNTS_GET_FINGER_STATE_0(ts_reg_data.finger_state_reg[1]) == 1 ||
					TS_SNTS_GET_FINGER_STATE_1(ts_reg_data.finger_state_reg[1]) == 1 ||
					TS_SNTS_GET_FINGER_STATE_2(ts_reg_data.finger_state_reg[1]) == 1 ||
					TS_SNTS_GET_FINGER_STATE_3(ts_reg_data.finger_state_reg[1]) == 1 ||
					TS_SNTS_GET_FINGER_STATE_0(ts_reg_data.finger_state_reg[2]) == 1 ||
					TS_SNTS_GET_FINGER_STATE_1(ts_reg_data.finger_state_reg[2]) == 1 ) 
				{
					ghost_finger_2 = 1;
				}
			}
            if((TS_SNTS_GET_FINGER_STATE_0(ts_reg_data.finger_state_reg[0]) == 0) && ghost_finger_1 == 1 && ghost_finger_2 == 0 && pressed == 1)
            {
	            if(jiffies - pressed_time < 2 * HZ) 
	            {
                	ghost_count++;
            
                    if(ghost_count > 3) 
                    {
                    	mdelay(30);
                    	i2c_smbus_write_byte_data(p_ts->client, MELT_CONTROL, NO_MELT);
                        ghost_count = 0;
                        melt_mode++;
                        printk("[touch] NO_MELT MODE\n");
                    }
				}
            	ghost_finger_1 = 0;
            	pressed = 0;
			}
			if( !TS_SNTS_GET_FINGER_STATE_0(ts_reg_data.finger_state_reg[0]) &&
				!TS_SNTS_GET_FINGER_STATE_1(ts_reg_data.finger_state_reg[0]) &&
				!TS_SNTS_GET_FINGER_STATE_2(ts_reg_data.finger_state_reg[0]) &&
				!TS_SNTS_GET_FINGER_STATE_3(ts_reg_data.finger_state_reg[0]) &&
				!TS_SNTS_GET_FINGER_STATE_0(ts_reg_data.finger_state_reg[1]) &&
				!TS_SNTS_GET_FINGER_STATE_1(ts_reg_data.finger_state_reg[1]) &&
				!TS_SNTS_GET_FINGER_STATE_2(ts_reg_data.finger_state_reg[1]) &&
				!TS_SNTS_GET_FINGER_STATE_3(ts_reg_data.finger_state_reg[1]) &&
				!TS_SNTS_GET_FINGER_STATE_0(ts_reg_data.finger_state_reg[2]) &&
				!TS_SNTS_GET_FINGER_STATE_1(ts_reg_data.finger_state_reg[2])) 
			{
				ghost_finger_1 = 0;
				ghost_finger_2 = 0;
				pressed = 0;
			}
		}
#endif
	finger_count=0;
	input_mt_sync(p_ts->input_dev);
	input_sync(p_ts->input_dev);

	//printk("[touch](%d, %d),(%d, %d) melt_mode=%d, esd=%d\n", curr_ts_data.X_position[0], curr_ts_data.Y_position[0],curr_ts_data.X_position[1], curr_ts_data.Y_position[1],!melt_mode,ts_esd_reset);
//--[[ LGE_UBIQUIX_MODIFIED_START : shyun@ubiquix.com [2011.09.09] - Merge from Black_Froyo MR Ver.
#if 0
//--]] LGE_UBIQUIX_MODIFIED_END : shyun@ubiquix.com [2011.09.09]- Merge from Black_Froyo MR Ver.
	if(!ts_reg_data.fingers_data[0][4])
	{
		if(!ts_esd_detect_flag && init_stabled==1 && !curr_ts_data.X_position[0] && !curr_ts_data.Y_position[0] && !curr_ts_data.X_position[1] && !curr_ts_data.Y_position[1])
		{
			ts_esd_detect_count++;
			if(ts_esd_detect_count>2 && init_stabled==1)
			{
				ts_esd_detect_flag=1;
				ts_esd_detect_count=0;
			}
		}
		else
			ts_esd_detect_count=0;
	}
	else
		ts_esd_detect_count=0;
#endif
//--[[ LGE_UBIQUIX_MODIFIED_START : shyun@ubiquix.com [2012.03.20] - TBD
//	if(init_stabled && !key_led_flag)	bd2802_touch_on();
//--]] LGE_UBIQUIX_MODIFIED_END : shyun@ubiquix.com [2012.03.20]- TBD
	
//--[[ LGE_UBIQUIX_MODIFIED_START : shyun@ubiquix.com [2011.09.09] - Merge from Black_Froyo MR Ver.
	for(ts_ret=0;ts_ret<100;ts_ret++)
	{
		if(!gpio_get_value(TOUCH_INT_N_GPIO))	 i2c_smbus_read_i2c_block_data(p_ts->client, 0x14, 1, &dummy);
		else									 break;
	}
	if(ts_ret>=100)	printk("[touch] TOUCH_INT_N_GPIO is LOW. touch lock up!!\n");
//--]] LGE_UBIQUIX_MODIFIED_END : shyun@ubiquix.com [2011.09.09]- Merge from Black_Froyo MR Ver.

	memset(&curr_ts_data, 0x0, sizeof(ts_finger_data));
//--[[ LGE_UBIQUIX_MODIFIED_START : shyun@ubiquix.com [2011.09.09] - Merge from Black_Froyo MR Ver.
#if 0
}
while(!gpio_get_value(TOUCH_INT_N_GPIO));
#endif
//--]] LGE_UBIQUIX_MODIFIED_END : shyun@ubiquix.com [2011.09.09]- Merge from Black_Froyo MR Ver.

SYNAPTICS_TS_IDLE:
	if (p_ts->use_irq) {
		enable_irq(p_ts->client->irq);
	}
}

static enum hrtimer_restart synaptics_ts_timer_func(struct hrtimer *timer)
{
	struct synaptics_ts_priv *ts = container_of(timer, struct synaptics_ts_priv, timer);

	queue_work(synaptics_wq, &ts->work);
	hrtimer_start(&ts->timer, ktime_set(0, 12500000), HRTIMER_MODE_REL); /* 12.5 msec */

	return HRTIMER_NORESTART;
}

static irqreturn_t synaptics_ts_irq_handler(int irq, void *dev_id)
{
	//pr_info("LGE: synaptics_ts_irq_handler\n");
	disable_irq_nosync(p_ts->client->irq);

/* 20110331 sookyoung.kim@lge.com LG-DVFS [START_LGE] */
/* Move this code later to somewhere common, such as the irq entry point.
 */
//--[[ LGE_UBIQUIX_MODIFIED_START : shyun@ubiquix.com [2012.03.20] - This feature is disabled.
#if 0
	if(ds_status.flag_run_dvs == 1){
        ds_status.flag_touch_timeout_count = DS_TOUCH_TIMEOUT_COUNT_MAX;    // = 6
        if(ds_status.touch_timeout_sec == 0){
            if(ds_counter.elapsed_usec + DS_TOUCH_TIMEOUT < 1000000){
                ds_status.touch_timeout_sec = ds_counter.elapsed_sec;
                ds_status.touch_timeout_usec = ds_counter.elapsed_usec + DS_TOUCH_TIMEOUT;
            }
            else{
                ds_status.touch_timeout_sec = ds_counter.elapsed_sec + 1;
                ds_status.touch_timeout_usec = (ds_counter.elapsed_usec + DS_TOUCH_TIMEOUT) - 1000000;
            }
        }
    }
#endif
//--]] LGE_UBIQUIX_MODIFIED_END : shyun@ubiquix.com [2012.03.20]- This feature is disabled.
/* 20110331 sookyoung.kim@lge.com LG-DVFS [END_LGE] */


	queue_work(synaptics_wq, &p_ts->work);
	return IRQ_HANDLED;
}


static unsigned char synaptics_ts_check_fwver(struct i2c_client *client)
{
	unsigned char RMI_Query_BaseAddr;
	unsigned char FWVersion_Addr;

	unsigned char SynapticsFirmVersion;

	RMI_Query_BaseAddr = i2c_smbus_read_byte_data(client, SYNAPTICS_RMI_QUERY_BASE_REG);
	FWVersion_Addr = RMI_Query_BaseAddr+3;

	SynapticsFirmVersion = i2c_smbus_read_byte_data(client, FWVersion_Addr);
	printk("[touch] touch controller firmware version = %x\n", SynapticsFirmVersion);

	return SynapticsFirmVersion;
}


static ssize_t hub_ts_FW_show(struct device *dev,  struct device_attribute *attr,  char *buf)
{
	int r;

	r = snprintf(buf, PAGE_SIZE,"%d\n", touch_fw_version);

	return r;
}

static DEVICE_ATTR(fw, 0666, hub_ts_FW_show, NULL);




#ifdef SYNAPTICS_SUPPORT_FW_UPGRADE
static unsigned long ExtractLongFromHeader(const unsigned char *SynaImage)  // Endian agnostic
{
  return((unsigned long)SynaImage[0] +
         (unsigned long)SynaImage[1]*0x100 +
         (unsigned long)SynaImage[2]*0x10000 +
         (unsigned long)SynaImage[3]*0x1000000);
}

static void CalculateChecksum(uint16_t *data, uint16_t len, uint32_t *dataBlock)
{
  unsigned long temp = *data++;
  unsigned long sum1;
  unsigned long sum2;

  *dataBlock = 0xffffffff;

  sum1 = *dataBlock & 0xFFFF;
  sum2 = *dataBlock >> 16;

  while (len--)
  {
    sum1 += temp;
    sum2 += sum1;
    sum1 = (sum1 & 0xffff) + (sum1 >> 16);
    sum2 = (sum2 & 0xffff) + (sum2 >> 16);
  }

  *dataBlock = sum2 << 16 | sum1;
}

static void SpecialCopyEndianAgnostic(uint8_t *dest, uint16_t src)
{
  dest[0] = src%0x100;  //Endian agnostic method
  dest[1] = src/0x100;
}


static bool synaptics_ts_fw_upgrade(struct i2c_client *client)
{
	int i;
	int j;

	uint8_t FlashQueryBaseAddr, FlashDataBaseAddr;
	uint8_t RMICommandBaseAddr;

	uint8_t BootloaderIDAddr;
	uint8_t BlockSizeAddr;
	uint8_t FirmwareBlockCountAddr;
	uint8_t ConfigBlockCountAddr;

	uint8_t BlockNumAddr;
	uint8_t BlockDataStartAddr;

	uint8_t current_fw_ver;

	uint8_t bootloader_id[2];

	uint8_t temp_array[2], temp_data, flashValue, m_firmwareImgVersion;
	uint8_t checkSumCode;

	uint16_t ts_block_size, ts_config_block_count, ts_fw_block_count;
	uint16_t m_bootloadImgID;

	uint32_t ts_config_img_size;
	uint32_t ts_fw_img_size;
	uint32_t pinValue, m_fileSize, m_firmwareImgSize, m_configImgSize, m_FirmwareImgFile_checkSum;

	////////////////////////////

	printk("[Touch Driver] synaptics_upgrade firmware [START]\n");
/*
	if(!(synaptics_ts_check_fwver(client) < SynapticsFirmware[0x1F]))
	{
		// Firmware Upgrade does not necessary!!!!
		pr_debug("[Touch Driver] Synaptics_UpgradeFirmware does not necessary!!!!\n");
		return true;
	}
*/
	if (p_ts->product_value==1)
	{
		memcpy(SynapticsFirmware, SynapticsFirmware_misung, sizeof(SynapticsFirmware_misung));
		current_fw_ver = synaptics_ts_check_fwver(client);
		if((current_fw_ver >= 0x64 && SynapticsFirmware[0x1F] >= 0x64) || (current_fw_ver < 0x64 && SynapticsFirmware[0x1F] < 0x64))
		{
#if 0
			if(!(current_fw_ver < SynapticsFirmware[0x1F]))
#else
			if(!(current_fw_ver != SynapticsFirmware[0x1F]))
#endif
			{
				// Firmware Upgrade does not necessary!!!!
				printk("[Touch Driver] synaptics_upgrade firmware does not necessary!!!!\n");
				return true;
			}
		}
	}
	else if (p_ts->product_value==2)
	{
			memcpy(SynapticsFirmware, SynapticsFirmware_lgit, sizeof(SynapticsFirmware_lgit));
			current_fw_ver = synaptics_ts_check_fwver(client);
			if((current_fw_ver >= 0x01 && SynapticsFirmware[0x1F] >= 0x01) || (current_fw_ver < 0x01 && SynapticsFirmware[0x1F] < 0x01))
			{
#if 0
				if(!(current_fw_ver < SynapticsFirmware[0x1F]))
#else
				if(!(current_fw_ver != SynapticsFirmware[0x1F]))
#endif
				{
					// Firmware Upgrade does not necessary!!!!
					printk("[Touch Driver] synaptics_upgrade firmware does not necessary!!!!\n");
					return true;
				}
			}
	}
	else
		return true;

	// Address Configuration
	FlashQueryBaseAddr = i2c_smbus_read_byte_data(client, SYNAPTICS_FLASH_QUERY_BASE_REG);

	BootloaderIDAddr = FlashQueryBaseAddr;
	BlockSizeAddr = FlashQueryBaseAddr + 3;
	FirmwareBlockCountAddr = FlashQueryBaseAddr + 5;
	ConfigBlockCountAddr = FlashQueryBaseAddr + 7;


	FlashDataBaseAddr = i2c_smbus_read_byte_data(client, SYNAPTICS_FLASH_DATA_BASE_REG);

	BlockNumAddr = FlashDataBaseAddr;
	BlockDataStartAddr = FlashDataBaseAddr + 2;

	// Get New Firmware Information from Header
	m_fileSize = sizeof(SynapticsFirmware) -1;

	checkSumCode         = ExtractLongFromHeader(&(SynapticsFirmware[0]));
	m_bootloadImgID      = (unsigned int)SynapticsFirmware[4] + (unsigned int)SynapticsFirmware[5]*0x100;
	m_firmwareImgVersion = SynapticsFirmware[7];
	m_firmwareImgSize    = ExtractLongFromHeader(&(SynapticsFirmware[8]));
	m_configImgSize      = ExtractLongFromHeader(&(SynapticsFirmware[12]));

	CalculateChecksum((uint16_t*)&(SynapticsFirmware[4]), (uint16_t)(m_fileSize-4)>>1, &m_FirmwareImgFile_checkSum);

	// Get Current Firmware Information
	i2c_smbus_read_i2c_block_data(client, BlockSizeAddr, sizeof(temp_array), (u8 *)&temp_array[0]);
	ts_block_size = temp_array[0] + (temp_array[1] << 8);

	i2c_smbus_read_i2c_block_data(client, FirmwareBlockCountAddr, sizeof(temp_array), (u8 *)&temp_array[0]);
	ts_fw_block_count = temp_array[0] + (temp_array[1] << 8);
	ts_fw_img_size = ts_block_size * ts_fw_block_count;

	i2c_smbus_read_i2c_block_data(client, ConfigBlockCountAddr, sizeof(temp_array), (u8 *)&temp_array[0]);
	ts_config_block_count = temp_array[0] + (temp_array[1] << 8);
	ts_config_img_size = ts_block_size * ts_config_block_count;

	i2c_smbus_read_i2c_block_data(client, BootloaderIDAddr, sizeof(bootloader_id), (u8 *)&bootloader_id[0]);
	printk("[touch] synaptics_upgrade firmware :: bootloaderID %02x %02x\n", bootloader_id[0], bootloader_id[1]);

	// Compare
	if (m_fileSize != (0x100+m_firmwareImgSize+m_configImgSize))
	{
		printk("[touch] synaptics_upgrade firmware :: error : invalid file size\n");
		return true;
	}

	if (m_firmwareImgSize != ts_fw_img_size)
	{
		pr_debug("[touch] synaptics_upgrade firmware :: error : invalid firmware image size\n");
		return true;
	}

	if (m_configImgSize != ts_config_img_size)
	{
		printk("[touch] synaptics_upgrade firmware :: error : invalid config image size\n");
		return true;
	}

	// Flash Write Ready - Flash Command Enable & Erase
	//i2c_smbus_write_block_data(client, BlockDataStartAddr, sizeof(bootloader_id), &bootloader_id[0]);
	// How can i use 'i2c_smbus_write_block_data'
	for(i = 0; i < sizeof(bootloader_id); i++)
	{
		if(i2c_smbus_write_byte_data(client, BlockDataStartAddr+i, bootloader_id[i]))
			printk("[touch] synaptics_upgrade firmware :: address %02x, value %02x\n", BlockDataStartAddr+i, bootloader_id[i]);
	}

	do
	{
		flashValue = i2c_smbus_read_byte_data(client, SYNAPTICS_FLASH_CONTROL_REG);
		temp_data = i2c_smbus_read_byte_data(client, SYNAPTICS_INT_STATUS_REG);
	} while((flashValue & 0x0f) != 0x00);

	i2c_smbus_write_byte_data(client, SYNAPTICS_FLASH_CONTROL_REG, SYNAPTICS_FLASH_CMD_ENABLE);

	do
	{
		pinValue = gpio_get_value(TOUCH_INT_N_GPIO);
		mdelay(1);
	} while(pinValue);
	do
	{
		flashValue = i2c_smbus_read_byte_data(client, SYNAPTICS_FLASH_CONTROL_REG);
		temp_data = i2c_smbus_read_byte_data(client, SYNAPTICS_INT_STATUS_REG);
	} while(flashValue != 0x80);
	flashValue = i2c_smbus_read_byte_data(client, SYNAPTICS_FLASH_CONTROL_REG);

	printk("[touch] synaptics_upgrade firmware :: flash program enable setup complete\n");

	//i2c_smbus_write_block_data(client, BlockDataStartAddr, sizeof(bootloader_id), &bootloader_id[0]);
	// How can i use 'i2c_smbus_write_block_data'
	for(i = 0; i < sizeof(bootloader_id); i++)
	{
		if(i2c_smbus_write_byte_data(client, BlockDataStartAddr+i, bootloader_id[i]))
			printk("[touch] synaptics_upgrade firmware :: address %02x, value %02x\n", BlockDataStartAddr+i, bootloader_id[i]);
	}

	if(m_firmwareImgVersion == 0 && ((unsigned int)bootloader_id[0] + (unsigned int)bootloader_id[1]*0x100) != m_bootloadImgID)
	{
		printk("[touch] synaptics_upgrade firmware :: error : invalid bootload Image\n");
		return true;
	}

	i2c_smbus_write_byte_data(client, SYNAPTICS_FLASH_CONTROL_REG, SYNAPTICS_FLASH_CMD_ERASEALL);

	printk("[touch] synaptics_upgrade firmware :: SYNAPTICS_FLASH_CMD_ERASEALL\n");

	do
	{
		pinValue = gpio_get_value(TOUCH_INT_N_GPIO);
		mdelay(1);
	} while(pinValue);
	do
	{
		flashValue = i2c_smbus_read_byte_data(client, SYNAPTICS_FLASH_CONTROL_REG);
		temp_data = i2c_smbus_read_byte_data(client, SYNAPTICS_INT_STATUS_REG);
	} while(flashValue != 0x80);

	printk("[touch] synaptics_upgrade firmware :: flash erase complete\n");

	// Flash Firmware Data Write
	for(i = 0; i < ts_fw_block_count; ++i)
	{
		temp_array[0] = i & 0xff;
		temp_array[1] = (i & 0xff00) >> 8;

		// Write Block Number
		//i2c_smbus_write_block_data(client, BlockNumAddr, sizeof(temp_array), &temp_array[0]);
		// How can i use 'i2c_smbus_write_block_data'
		for(j = 0; j < sizeof(temp_array); j++)
		{
			i2c_smbus_write_byte_data(client, BlockNumAddr+j, temp_array[j]);
		}

		// Write Data Block&SynapticsFirmware[0]
		//i2c_smbus_write_block_data(client, BlockDataStartAddr, ts_block_size, &SynapticsFirmware[0x100+i*ts_block_size]);
		// How can i use 'i2c_smbus_write_block_data'
		for(j = 0; j < ts_block_size; j++)
		{
			i2c_smbus_write_byte_data(client, BlockDataStartAddr+j, SynapticsFirmware[0x100+i*ts_block_size+j]);
		}

		// Issue Write Firmware Block command
		i2c_smbus_write_byte_data(client, SYNAPTICS_FLASH_CONTROL_REG, SYNAPTICS_FLASH_CMD_FW_WRITE);
		do
		{
			pinValue = gpio_get_value(TOUCH_INT_N_GPIO);
			mdelay(1);
		} while(pinValue);
		do
		{
			flashValue = i2c_smbus_read_byte_data(client, SYNAPTICS_FLASH_CONTROL_REG);
			temp_data = i2c_smbus_read_byte_data(client, SYNAPTICS_INT_STATUS_REG);
		} while(flashValue != 0x80);
	} //for

	printk("[touch] synaptics_upgrade firmware :: flash firmware write complete\n");

	// Flash Firmware Config Write
	for(i = 0; i < ts_config_block_count; i++)
	{
		SpecialCopyEndianAgnostic(&temp_array[0], i);

		// Write Configuration Block Number
		i2c_smbus_write_block_data(client, BlockNumAddr, sizeof(temp_array), &temp_array[0]);
		// How can i use 'i2c_smbus_write_block_data'
		for(j = 0; j < sizeof(temp_array); j++)
		{
			i2c_smbus_write_byte_data(client, BlockNumAddr+j, temp_array[j]);
		}

		// Write Data Block
		//i2c_smbus_write_block_data(client, BlockDataStartAddr, ts_block_size, &SynapticsFirmware[0x100+m_firmwareImgSize+i*ts_block_size]);
		// How can i use 'i2c_smbus_write_block_data'
		for(j = 0; j < ts_block_size; j++)
		{
			i2c_smbus_write_byte_data(client, BlockDataStartAddr+j, SynapticsFirmware[0x100+m_firmwareImgSize+i*ts_block_size+j]);
		}

		// Issue Write Configuration Block command to flash command register
		i2c_smbus_write_byte_data(client, SYNAPTICS_FLASH_CONTROL_REG, SYNAPTICS_FLASH_CMD_CONFIG_WRITE);
		do
		{
			pinValue = gpio_get_value(TOUCH_INT_N_GPIO);
			mdelay(1);
		} while(pinValue);
		do
		{
			flashValue = i2c_smbus_read_byte_data(client, SYNAPTICS_FLASH_CONTROL_REG);
			temp_data = i2c_smbus_read_byte_data(client, SYNAPTICS_INT_STATUS_REG);
		} while(flashValue != 0x80);
	}

	printk("[touch] synaptics_upgrade firmware :: flash config write complete\n");


	RMICommandBaseAddr = i2c_smbus_read_byte_data(client, SYNAPTICS_RMI_CMD_BASE_REG);
	i2c_smbus_write_byte_data(client, RMICommandBaseAddr, 0x01);
	mdelay(100);

	do
	{
		pinValue = gpio_get_value(TOUCH_INT_N_GPIO);
		mdelay(1);
	} while(pinValue);
	do
	{
		flashValue = i2c_smbus_read_byte_data(client, SYNAPTICS_FLASH_CONTROL_REG);
		temp_data = i2c_smbus_read_byte_data(client, SYNAPTICS_INT_STATUS_REG);
	} while((flashValue & 0x0f) != 0x00);

	// Clear the attention assertion by reading the interrupt status register
	temp_data = i2c_smbus_read_byte_data(client, SYNAPTICS_INT_STATUS_REG);

	// Read F01 Status flash prog, ensure the 6th bit is '0'
	do
	{
		temp_data = i2c_smbus_read_byte_data(client, SYNAPTICS_DATA_BASE_REG);
	} while((temp_data & 0x40) != 0);

	return true;
}
#endif

#ifdef FEATURE_LGE_TOUCH_ESD_DETECT
static bool  synatics_ts_touch_recovery()
{
	disable_irq(p_ts->client->irq);
	//touch power down reset for ESD recovery
	mdelay(30);
//	aat2870_ldo_write(0x25 , 0x00);	//SHYUN_TBD
	mdelay(100);
	//touch power on reset for ESD recovery
//	aat2870_ldo_write(0x25 , 0x4C);	//SHYUN_TBD
	mdelay(70);
	
#ifdef FEATURE_LGE_TOUCH_GHOST_FINGER_IMPROVE
	melt_mode = 0;
	ghost_finger_1 = 0;
	ghost_finger_2 = 0;
	pressed = 0;
	ghost_count=0;
	melt_flag=0;
#endif
	enable_irq(p_ts->client->irq);
	
	return true;
}
#endif /*FEATURE_LGE_TOUCH_ESD_DETECT*/

static void synatics_ts_power_on(void)
{
	printk("%s : %d\n",__func__,__LINE__);
	aat2870_ldo_write(0x25 , 0x00);
	mdelay(30);
	aat2870_ldo_write(0x25 , 0x4C);
	return;
}

static void synatics_ts_power_off(void)
{
	printk("%s : %d\n",__func__,__LINE__);
	aat2870_ldo_write(0x25 , 0x00);
	mdelay(50);
	return;
}

/*************************************************************************************************
 * 1. Set interrupt configuration
 * 2. Disable interrupt
 * 3. Power up
 * 4. Read RMI Version
 * 5. Read Firmware version & Upgrade firmware automatically
 * 6. Read Data To Initialization Touch IC
 * 7. Set some register
 * 8. Enable interrupt
*************************************************************************************************/
static int synaptics_ts_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int ret = 0;
	uint16_t max_x;
	uint16_t max_y;
	uint8_t max_pressure;
	uint8_t max_width;

	char product_id[6];
	uint8_t product_id_addr;

	if(lcd_off_boot ==1) 	
	{
		printk("[touch] No Device LCD\n");
		ret = -ENODEV;
		return ret;
	}

	printk("%s() -- start\n\n\n", __func__);

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		pr_err("synaptics_ts_probe: need I2C_FUNC_I2C\n");
		ret = -ENODEV;
		goto err_check_functionality_failed;
	}

	p_ts = kzalloc(sizeof(*p_ts), GFP_KERNEL);
	if (p_ts == NULL) {
		ret = -ENOMEM;
		goto err_alloc_data_failed;
	}

	p_ts->client = client;
	hub_ts_client = client;
	i2c_set_clientdata(client, p_ts);

	INIT_WORK(&p_ts->work, synaptics_ts_work_func);
	INIT_DELAYED_WORK(&p_ts->init_delayed_work, synaptics_ts_init_delayed_work);

#ifdef FEATURE_LGE_TOUCH_ESD_DETECT
	wake_lock_init(&ts_wake_lock, WAKE_LOCK_SUSPEND, "ts_upgrade");
#endif /*FEATURE_LGE_TOUCH_ESD_DETECT*/

#ifdef FEATURE_LGE_TOUCH_GRIP_SUPPRESSION
	ret = device_create_file(&client->dev, &dev_attr_gripsuppression);
	if (ret) {
		pr_err("synaptics_ts_probe: grip suppression device_create_file failed\n");
		goto err_check_functionality_failed;
	}
#endif /* FEATURE_LGE_TOUCH_GRIP_SUPPRESSION */

  	memset(&ts_reg_data, 0x0, sizeof(ts_sensor_data));
	memset(&prev_ts_data, 0x0, sizeof(ts_finger_data));
  	memset(&curr_ts_data, 0x0, sizeof(ts_finger_data));

	// touch firmware ID check
	product_id_addr = (i2c_smbus_read_byte_data(hub_ts_client, SYNAPTICS_RMI_QUERY_BASE_REG)) + 11;
	
	ret=i2c_smbus_read_i2c_block_data(hub_ts_client, product_id_addr, sizeof(product_id), (u8 *)&product_id[0]);
	
	if (ret<0) {
		pr_err("synaptics_ts_probe: need I2C_DEV\n");
		ret = -ENODEV;
		goto err_check_functionality_failed;
	}
	
		if(strncmp(product_id, SYNAPTICS_TM1576_PRODUCT_ID, 6) == 0)
		{
			pr_err("synaptics_ts_probe: product ID : TM1576\n");
			SYNAPTICS_PANEL_MAX_X = SYNAPTICS_TM1576_RESOLUTION_X;
			SYNAPTICS_PANEL_MAX_Y = SYNAPTICS_TM1576_RESOLUTION_Y;
			SYNAPTICS_PANEL_LCD_MAX_Y = SYNAPTICS_TM1576_LCD_ACTIVE_AREA;
			SYNAPTICS_PANEL_BUTTON_MIN_Y = SYNAPTICS_TM1576_BUTTON_ACTIVE_AREA;
			p_ts->product_value=3; //other
		}
		else if(strncmp(product_id, SYNAPTICS_TM1702_PRODUCT_ID, 6) == 0)
		{
			pr_err("synaptics_ts_probe: product ID : TM1702\n");
			SYNAPTICS_PANEL_MAX_X = SYNAPTICS_TM1702_RESOLUTION_X;
			SYNAPTICS_PANEL_MAX_Y = SYNAPTICS_TM1702_RESOLUTION_Y;
			SYNAPTICS_PANEL_LCD_MAX_Y = SYNAPTICS_TM1702_LCD_ACTIVE_AREA;
			SYNAPTICS_PANEL_BUTTON_MIN_Y = SYNAPTICS_TM1702_BUTTON_ACTIVE_AREA;
			p_ts->product_value=1; //misung panel

			synaptics_ts_fw_upgrade(hub_ts_client);
		}
		else if(strncmp(product_id, SYNAPTICS_TM1738_PRODUCT_ID, 6) == 0)
		{
			pr_err("synaptics_ts_probe: product ID : TM1738\n");
			SYNAPTICS_PANEL_MAX_X = SYNAPTICS_TM1738_RESOLUTION_X;
			SYNAPTICS_PANEL_MAX_Y = SYNAPTICS_TM1738_RESOLUTION_Y;
			SYNAPTICS_PANEL_LCD_MAX_Y = SYNAPTICS_TM1738_LCD_ACTIVE_AREA;
			SYNAPTICS_PANEL_BUTTON_MIN_Y = SYNAPTICS_TM1738_BUTTON_ACTIVE_AREA;
			p_ts->product_value=2; //lgit panel

			if(ret>=0) synaptics_ts_fw_upgrade(hub_ts_client);
		}
		else
		{
			pr_err("synaptics_ts_probe: product ID : error\n");
			SYNAPTICS_PANEL_MAX_X = SYNAPTICS_TM1738_RESOLUTION_X;
			SYNAPTICS_PANEL_MAX_Y = SYNAPTICS_TM1738_RESOLUTION_Y;
			SYNAPTICS_PANEL_LCD_MAX_Y = SYNAPTICS_TM1738_LCD_ACTIVE_AREA;
			SYNAPTICS_PANEL_BUTTON_MIN_Y = SYNAPTICS_TM1738_BUTTON_ACTIVE_AREA;
			p_ts->product_value=2; //nothing
		}

		if(ret>=0) touch_fw_version=synaptics_ts_check_fwver(hub_ts_client);


	p_ts->input_dev = input_allocate_device();
	if (p_ts->input_dev == NULL) {
		ret = -ENOMEM;
		pr_err("synaptics_ts_probe: Failed to allocate input device\n");
		goto err_input_dev_alloc_failed;
	}
	p_ts->input_dev->name = "hub_synaptics_touch";

	set_bit(EV_SYN, p_ts->input_dev->evbit);
	set_bit(EV_KEY, p_ts->input_dev->evbit);
	set_bit(BTN_TOUCH, p_ts->input_dev->keybit);
	set_bit(EV_ABS, p_ts->input_dev->evbit);

	// button
	set_bit(KEY_MENU, p_ts->input_dev->keybit);
	set_bit(KEY_HOME, p_ts->input_dev->keybit);
	set_bit(KEY_BACK, p_ts->input_dev->keybit);
	set_bit(KEY_SEARCH, p_ts->input_dev->keybit);
	set_bit(KEY_REJECT, p_ts->input_dev->keybit);

	max_x = SYNAPTICS_PANEL_MAX_X;
	max_y = SYNAPTICS_PANEL_LCD_MAX_Y;
	max_pressure = 0xFF;
	max_width = 0x0F;

	input_set_abs_params(p_ts->input_dev, ABS_MT_POSITION_X, 0, max_x, 0, 0);
	input_set_abs_params(p_ts->input_dev, ABS_MT_POSITION_Y, 0, max_y, 0, 0);
	input_set_abs_params(p_ts->input_dev, ABS_MT_TOUCH_MAJOR, 0, max_pressure, 0, 0);
	input_set_abs_params(p_ts->input_dev, ABS_MT_WIDTH_MAJOR, 0, max_width, 0, 0);

	pr_info("synaptics_ts_probe: max_x %d, max_y %d\n", max_x, max_y);

	ret = input_register_device(p_ts->input_dev);
	if (ret) {
		pr_err("synaptics_ts_probe: Unable to register %s input device\n", p_ts->input_dev->name);
		goto err_input_register_device_failed;
	}

	pr_debug("########## irq [%d], irqflags[0x%x]\n", client->irq, IRQF_TRIGGER_FALLING);

	i2c_smbus_write_byte_data(hub_ts_client, SYNAPTICS_RIM_CONTROL_INTERRUPT_ENABLE, 0x00); //interrupt disable

	if (client->irq) {
		ret = request_irq(client->irq, synaptics_ts_irq_handler, IRQF_TRIGGER_FALLING, client->name, p_ts);

		if (ret == 0) {
			p_ts->use_irq = 1;
			pr_warning("request_irq\n");
			}
		else
			dev_err(&client->dev, "request_irq failed\n");
	}
	if (!p_ts->use_irq) {
		hrtimer_init(&p_ts->timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
		p_ts->timer.function = synaptics_ts_timer_func;
		hrtimer_start(&p_ts->timer, ktime_set(1, 0), HRTIMER_MODE_REL);
	}

	ret = device_create_file(&client->dev, &dev_attr_fw);
	if (ret) {
		pr_debug( "Hub-touch screen : touch screen_probe: Fail\n");
		device_remove_file(&client->dev, &dev_attr_fw);
		return ret;
	}

#ifdef CONFIG_HAS_EARLYSUSPEND
	p_ts->early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN - 1;
	p_ts->early_suspend.suspend = synaptics_ts_early_suspend;
	p_ts->early_suspend.resume = synaptics_ts_late_resume;
	register_early_suspend(&p_ts->early_suspend);
#endif

	melt_mode = 0;
	ghost_finger_1 = 0;
	ghost_finger_2 = 0;
	pressed = 0;
	ghost_count=0;
	melt_flag=0;

	synaptics_ts_initialize();
	schedule_delayed_work(&p_ts->init_delayed_work, msecs_to_jiffies(30000));

	printk("synaptics_ts_probe: Start touchscreen %s in %s mode\n", p_ts->input_dev->name, p_ts->use_irq ? "interrupt" : "polling");
	return 0;

err_input_register_device_failed:
	input_free_device(p_ts->input_dev);

err_input_dev_alloc_failed:
err_alloc_data_failed:
err_check_functionality_failed:
	return ret;
}

void synaptics_ts_disable_irq()
{
	printk("\n%s : %d\n",__func__,__LINE__);
	
	if(lcd_off_boot==1)	return;

	disable_irq(hub_ts_client->irq);
}
EXPORT_SYMBOL(synaptics_ts_disable_irq);

static int synaptics_ts_remove(struct i2c_client *client)
{
#ifdef FEATURE_LGE_TOUCH_GRIP_SUPPRESSION
	device_remove_file(&client->dev, &dev_attr_gripsuppression);
#endif

	unregister_early_suspend(&p_ts->early_suspend);
	if (p_ts->use_irq)
		free_irq(client->irq, p_ts);
	else
		hrtimer_cancel(&p_ts->timer);
	input_unregister_device(p_ts->input_dev);
	kfree(p_ts);
	return 0;
}

static int synaptics_ts_suspend(struct i2c_client *client, pm_message_t mesg)
{
	int ret;

	init_stabled = 0;

	if (p_ts->use_irq)
		disable_irq(client->irq);
	else
		hrtimer_cancel(&p_ts->timer);

	ret = cancel_work_sync(&p_ts->work);
	cancel_delayed_work_sync(&p_ts->init_delayed_work);

	//if (ret && p_ts->use_irq) /* if work was pending disable-count is now 2 */
	//	enable_irq(client->irq);

#ifdef FEATURE_LGE_TOUCH_GHOST_FINGER_IMPROVE
	melt_mode = 0;
	ghost_finger_1 = 0;
	ghost_finger_2 = 0;
	pressed = 0;
	ghost_count=0;
	melt_flag=0;
	ts_esd_detect_flag=0;
	ts_esd_reset=0;
#endif
	//synatics_ts_power_off();

	return 0;
}

static int synaptics_ts_resume(struct i2c_client *client)
{
	printk("%s : %d\n",__func__,__LINE__);
	if (p_ts->use_irq)
		enable_irq(client->irq);

	if (!p_ts->use_irq)
		hrtimer_start(&p_ts->timer, ktime_set(1, 0), HRTIMER_MODE_REL);

	memset(&ts_reg_data, 0x0, sizeof(ts_sensor_data));
	memset(&prev_ts_data, 0x0, sizeof(ts_finger_data));
  	memset(&curr_ts_data, 0x0, sizeof(ts_finger_data));

	synaptics_ts_initialize();
	schedule_delayed_work(&p_ts->init_delayed_work, msecs_to_jiffies(4000));

	return 0;
}

#ifdef CONFIG_HAS_EARLYSUSPEND
static void synaptics_ts_early_suspend(struct early_suspend *h)
{
	printk("%s : %d\n",__func__,__LINE__);
	p_ts = container_of(h, struct synaptics_ts_priv, early_suspend);
	synaptics_ts_suspend(p_ts->client, PMSG_SUSPEND);
}

static void synaptics_ts_late_resume(struct early_suspend *h)
{
	printk("%s : %d\n",__func__,__LINE__);
	p_ts = container_of(h, struct synaptics_ts_priv, early_suspend);
	synaptics_ts_resume(p_ts->client);
}
#endif

static const struct i2c_device_id synaptics_ts_id[] = {
	{ "hub_synaptics_ts", 0 },
	{ },
};

static struct i2c_driver synaptics_ts_driver = {
	.probe		= synaptics_ts_probe,
	.remove		= synaptics_ts_remove,
#ifndef CONFIG_HAS_EARLYSUSPEND
	.suspend	= synaptics_ts_suspend,
	.resume		= synaptics_ts_resume,
#endif
	.id_table	= synaptics_ts_id,
	.driver = {
		.name	= "hub_synaptics_ts",
		.owner = THIS_MODULE,
	},
};

static int __devinit synaptics_ts_init(void)
{
#ifdef FEATURE_LGE_TOUCH_REAL_TIME_WORK_QUEUE
//	synaptics_wq = create_rt_workqueue("synaptics_wq");
	synaptics_wq = alloc_workqueue("synaptics_wq", WQ_MEM_RECLAIM | WQ_HIGHPRI | WQ_CPU_INTENSIVE, 1);
#else
	synaptics_wq = create_singlethread_workqueue("synaptics_wq");
#endif

   	pr_warning("LGE: Synaptics ts_init\n");
	if (!synaptics_wq)
		return -ENOMEM;
	return i2c_add_driver(&synaptics_ts_driver);
}

static void __exit synaptics_ts_exit(void)
{
	i2c_del_driver(&synaptics_ts_driver);

	if (synaptics_wq)
		destroy_workqueue(synaptics_wq);
}

module_init(synaptics_ts_init);
module_exit(synaptics_ts_exit);

MODULE_DESCRIPTION("Synaptics Touchscreen Driver");
MODULE_AUTHOR("Choi Daewan <ntdeaewan.choi@lge.com>");
MODULE_LICENSE("GPL");

