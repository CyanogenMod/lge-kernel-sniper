/*
 * Hub DSI command mode panel
 *
 * Author: Kyungtae.oh <kyungtae.oh@lge.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*#define DEBUG*/

#include <linux/module.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/jiffies.h>
#include <linux/sched.h>
#include <linux/backlight.h>
#include <linux/fb.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>
#include <linux/workqueue.h>
#include <linux/slab.h>
#include <linux/mutex.h>

#include <plat/display.h>

#define MODULE_NAME		"B-LCD"

#define DSI_DISPLAY_SUSPENDED 0 
#define DSI_DISPLAY_DISABLED  1

#ifndef DEBUG
//#define DEBUG
//#undef DEBUG
#endif

#ifdef DEBUG
#define DBG(fmt, args...) 				\
	printk(KERN_DEBUG "[%s] %s(%d): " 		\
		fmt, MODULE_NAME, __func__, __LINE__, ## args); 
#else	/* DEBUG */
#define DBG(...) 
#endif

#define GPIO_LCD_RESET_N		34
#define GPIO_LCD_CS			54
#define GPIO_LCD_MAKER_ID		55

static int hub_dcs_read_1(enum omap_dsi_index ix, u8 dcs_cmd, u8 *data);
#ifdef DEBUG
static void read_status_reg(enum omap_dsi_index ix, u8* msg)
{
	u8 data[6];
	hub_dcs_read_1(ix, 0x0a, &data[0]);
	hub_dcs_read_1(ix, 0x0b, &data[1]);
	hub_dcs_read_1(ix, 0x0c, &data[2]);
	hub_dcs_read_1(ix, 0x0d, &data[3]);
	hub_dcs_read_1(ix, 0x0e, &data[4]);
	hub_dcs_read_1(ix, 0x0f, &data[5]);
	DBG("%s a:%02x b:%02x c:%02x d:%02x e:%02x f:%02x\n",msg, data[0], data[1], data[2], data[3], data[4], data[5]);
}
#else
#define	read_status_reg(ix, msg) while(0)
#endif

/* DSI Virtual channel. Hardcoded for now. */
#define TCH 				0

#define DCS_RESET			0x01
#define DCS_READ_NUM_ERRORS		0x05
#define DCS_READ_POWER_MODE		0x0a
#define DCS_READ_MADCTL			0x0b
#define DCS_READ_PIXEL_FORMAT		0x0c
#define DCS_RDDSDR			0x0f
#define DCS_SLEEP_IN			0x10
#define DCS_SLEEP_OUT			0x11
#define DCS_DISPLAY_OFF			0x28
#define DCS_DISPLAY_ON			0x29
#define DCS_COLUMN_ADDR			0x2a
#define DCS_PAGE_ADDR			0x2b
#define DCS_MEMORY_WRITE		0x2c
#define DCS_TEAR_OFF			0x34
#define DCS_TEAR_ON			0x35
#define DCS_MEM_ACC_CTRL		0x36
#define DCS_PIXEL_FORMAT		0x3a
#define DCS_BRIGHTNESS			0x51
#define DCS_CTRL_DISPLAY		0x53
#define DCS_WRITE_CABC			0x55
#define DCS_READ_CABC			0x56
#define DCS_GET_ID1			0xf8	
/* #define HUB_USE_ESD_CHECK */
#define HUB_ESD_CHECK_PERIOD	msecs_to_jiffies(5000)

extern int dsi_vc_write(enum omap_dsi_index ix, int channel, u8 cmd, u8 *data, int len);
static irqreturn_t hub_te_isr(int irq, void *data);
static void hub_te_timeout_work_callback(struct work_struct *work);
static int _hub_enable_te(struct omap_dss_device *dssdev, bool enable);
#ifdef HUB_USE_ESD_CHECK
static void hub_esd_work(struct work_struct *work);
#endif

#if 1////
static int lcd_boot_status=1;
#endif
#if 1
int lcd_off_boot=0;
EXPORT_SYMBOL(lcd_off_boot);

static int __init nolcd_setup(char *unused)
{

	lcd_off_boot = 1;
	return 1;
}
__setup("nolcd", nolcd_setup);
#endif

static int no_lcd_flag = 0; 

enum {
	CMD_GATESET_8_1,
	CMD_GATESET_8_2,
	CMD_SET_PARTIAL_AREA,
	CMD_SET_SCROLL_AREA,
	CMD_SET_ADDRESS_MODE,
	CMD_SET_SCROLL_START,
	CMD_SET_PIXEL_FOMAT,
//	CMD_SET_TEAR_SCANLINE,////
	CMD_EX_VSYNC_EN,
	CMD_VCSEL,
	CMD_SETVGMPM,
	CMD_RBIAS1,
	CMD_RBIAS2,
	CMD_SET_DDVDHP,
	CMD_SET_DDVDHM,
	CMD_SET_VGH,
	CMD_SET_VGL,
	CMD_SET_VCL,
	CMD_NUMBER_OF_LINES,
	CMD_NUMBER_OF_FP_LINES,
	CMD_GATESET_1,
	CMD_1H_PERIOD,
	CMD_SOURCE_PRECHARGE,
	CMD_SOURCE_PRECHARGE_TIMING,
	CMD_SOURCE_LEVEL,
	CMD_NUMBER_OF_BP_LINES,
	CMD_GATESET_2,
	CMD_GATESET_3,
	CMD_GATESET_4,
	CMD_GATESET_5,
	CMD_GATESET_6,
	CMD_GATESET_7,
	CMD_GATESET_9,
	CMD_FLHW,
	CMD_VCKHW,
	CMD_FLT,
	CMD_TCTRL,
	CMD_DOTINV,
	CMD_ON_OFF_SEQUENCE_PERIOD,
	CMD_PONSEQA,
	CMD_PONSEQB,
	CMD_PONSEQC,
	CMD_POFSEQA,
	CMD_POFSEQB,
	CMD_POFSEQC,
	CMD_MANUAL_BRIGHTNESS,
	CMD_MINIMUM_BRIGHTNESS,
	CMD_BACKLIGHTCONTROL,
	CMD_CABC_PWM,
	CMD_CABC_1,
	CMD_CABC_BRIGHTNESS_1,
	CMD_CABC_BRIGHTNESS_2,
	CMD_CABC_2,
	CMD_BRT_REV,
	CMD_TE,
	CMD_HIGH_SPEED_RAM,
	CMD_GAMMA_SETTING_R_POS,
	CMD_GAMMA_SETTING_R_NEG,
	CMD_GAMMA_SETTING_G_POS,
	CMD_GAMMA_SETTING_G_NEG,
	CMD_GAMMA_SETTING_B_POS,
	CMD_GAMMA_SETTING_B_NEG,
	NUM_CMD,
	CMD_SET_TEAR_SCANLINE,
};

u8 lcd_command_for_mipi[][22] = {
#if 0
	[STAR01                     ] = {0x15,0x02,0xBD,0x00,},               
	[STAR02                     ] = {0x15,0x02,0xBD,0x00,},               
	[STAR03                     ] = {0x15,0x02,0xBC,0x1A,},               
	[STAR04                     ] = {0x15,0x02,0xBD,0x08,},               
	[STAR05                     ] = {0x39,0x05,0x03,0x00,0xBC,0xFF,0xFF,},
	[STAR06                     ] = {0x15,0x02,0xBD,0x0B,},               
	[STAR07                     ] = {0x15,0x02,0xBC,0xCC,},               
	[STAR08                     ] = {0x15,0x02,0xBD,0x11,},               
	[STAR09                     ] = {0x15,0x02,0xBC,0xFF,},               
#endif
	[CMD_GATESET_8_1            ] = {0x23,0x02,0xCF,0x30,},                                                                           
	[CMD_GATESET_8_2            ] = {0x23,0x02,0xCF,0x30,},                                                                           
	[CMD_SET_PARTIAL_AREA       ] = {0x39,0x05,0x30,0x00,0x00,0x03,0x1F,},                                                            
	[CMD_SET_SCROLL_AREA        ] = {0x39,0x07,0x33,0x00,0x00,0x03,0x20,0x00,0x00,},                                                  
	[CMD_SET_ADDRESS_MODE       ] = {0x15,0x02,0x36,0x0A,},                                                                           
	[CMD_SET_SCROLL_START       ] = {0x39,0x03,0x37,0x00,0x00,}, 
	[CMD_SET_PIXEL_FOMAT        ] = {0x15,0x02,0x3A,0x07,}, 
//	[CMD_SET_TEAR_SCANLINE      ] = {0x39,0x03,0x44,0x00,0x00,},////
	[CMD_EX_VSYNC_EN            ] = {0x23,0x02,0x71,0x00,},
	[CMD_VCSEL                  ] = {0x23,0x02,0xB2,0x00,},
	[CMD_SETVGMPM               ] = {0x23,0x02,0xB4,0xAA,},                                                                           
	[CMD_RBIAS1                 ] = {0x23,0x02,0xB5,0x33,},                                                                           
	[CMD_RBIAS2                 ] = {0x23,0x02,0xB6,0x03,},                                                                           
	[CMD_SET_DDVDHP             ] = {0x29,0x11,0xB7,0x1A,0x33,0x03,0x03,0x03,0x00,0x00,0x01,0x02,0x00,0x00,0x00,0x00,0x01,0x01,0x01,},
	[CMD_SET_DDVDHM             ] = {0x29,0x0E,0xB8,0x1C,0x53,0x03,0x03,0x00,0x01,0x02,0x00,0x00,0x04,0x00,0x01,0x01,},               
	[CMD_SET_VGH                ] = {0x29,0x0B,0xB9,0x0C,0x01,0x01,0x00,0x00,0x00,0x02,0x00,0x02,0x01,},                              
	[CMD_SET_VGL                ] = {0x29,0x0B,0xBA,0x0D,0x01,0x01,0x00,0x00,0x00,0x02,0x00,0x02,0x01,},                              
	[CMD_SET_VCL                ] = {0x29,0x08,0xBB,0x00,0x00,0x00,0x00,0x01,0x02,0x01,},                                             
	[CMD_NUMBER_OF_LINES        ] = {0x23,0x02,0xC1,0x01,},                                                                           
	[CMD_NUMBER_OF_FP_LINES     ] = {0x23,0x02,0xC2,0x08,},                                                                           
	[CMD_GATESET_1              ] = {0x23,0x02,0xC3,0x04,},                                                                           
	[CMD_1H_PERIOD              ] = {0x23,0x02,0xC4,0x4C,},                                                                           
	[CMD_SOURCE_PRECHARGE       ] = {0x23,0x02,0xC5,0x07,},                                                                           
	[CMD_SOURCE_PRECHARGE_TIMING] = {0x29,0x03,0xC6,0xC4,0x04,},                                                                      
	[CMD_SOURCE_LEVEL           ] = {0x23,0x02,0xC7,0x00,},                                                                           
	[CMD_NUMBER_OF_BP_LINES     ] = {0x23,0x02,0xC8,0x02,},                                                                           
	[CMD_GATESET_2              ] = {0x23,0x02,0xC9,0x10,},                                                                           
	[CMD_GATESET_3              ] = {0x29,0x03,0xCA,0x04,0x04,},                                                                      
	[CMD_GATESET_4              ] = {0x23,0x02,0xCB,0x03,},                                                                           
	[CMD_GATESET_5              ] = {0x23,0x02,0xCC,0x12,},                                                                           
	[CMD_GATESET_6              ] = {0x23,0x02,0xCD,0x12,},                                                                           
	[CMD_GATESET_7              ] = {0x23,0x02,0xCE,0x30,},                                                                           
	[CMD_GATESET_9              ] = {0x23,0x02,0xD0,0x40,},                                                                           
	[CMD_FLHW                   ] = {0x23,0x02,0xD1,0x22,},                                                                           
	[CMD_VCKHW                  ] = {0x23,0x02,0xD2,0x22,},                                                                           
	[CMD_FLT                    ] = {0x23,0x02,0xD3,0x04,},                                                                           
	[CMD_TCTRL                  ] = {0x23,0x02,0xD4,0x14,},                                                                           
	[CMD_DOTINV                 ] = {0x23,0x02,0xD6,0x01,},                                                                           
	[CMD_ON_OFF_SEQUENCE_PERIOD ] = {0x23,0x02,0xD7,0x00,},                                                                           
	[CMD_PONSEQA                ] = {0x29,0x0A,0xD8,0x01,0x05,0x06,0x0D,0x18,0x09,0x22,0x23,0x00,},                                   
	[CMD_PONSEQB                ] = {0x29,0x03,0xD9,0x24,0x01,},                                                                      
	[CMD_PONSEQC                ] = {0x29,0x06,0xDE,0x09,0x0F,0x21,0x12,0x04,},                                                       
	[CMD_POFSEQA                ] = {0x29,0x07,0xDF,0x02,0x06,0x06,0x06,0x06,0x00,},                                                  
	[CMD_POFSEQB                ] = {0x23,0x02,0xE0,0x01,},                                                                           
	[CMD_POFSEQC                ] = {0x29,0x06,0xE1,0x00,0x00,0x00,0x00,0x00,},                                                       
	[CMD_MANUAL_BRIGHTNESS      ] = {0x23,0x02,0x51,0xFF,},                                                                           
	[CMD_MINIMUM_BRIGHTNESS     ] = {0x23,0x02,0x52,0x00,},                                                                           
	[CMD_BACKLIGHTCONTROL       ] = {0x23,0x02,0x53,0x40,},                                                                           
	[CMD_CABC_PWM               ] = {0x29,0x03,0xE2,0x00,0x00,},                                                                      
	[CMD_CABC_1                 ] = {0x23,0x02,0xE3,0x03,},                                                                           
	[CMD_CABC_BRIGHTNESS_1      ] = {0x29,0x09,0xE4,0x66,0x7B,0x90,0xA5,0xBB,0xC7,0xE1,0xE5,},                                        
	[CMD_CABC_BRIGHTNESS_2      ] = {0x29,0x09,0xE5,0xC5,0xC5,0xC9,0xC9,0xD1,0xE1,0xF1,0xFE,},                                        
	[CMD_CABC_2                 ] = {0x23,0x02,0xE7,0x2A,},                                                                           
	[CMD_BRT_REV                ] = {0x23,0x02,0xE8,0x00,},                                                                           
	[CMD_TE                     ] = {0x23,0x02,0xE9,0x00,},                                                                           
	[CMD_HIGH_SPEED_RAM         ] = {0x23,0x02,0xEA,0x01,},                                                                           
	[CMD_GAMMA_SETTING_R_POS    ] = {0x29,0x09,0xEB,0x00,0x33,0x12,0x10,0x98,0x88,0x87,0x0B,},                                        
	[CMD_GAMMA_SETTING_R_NEG    ] = {0x29,0x09,0xEC,0x00,0x33,0x12,0x10,0x98,0x88,0x87,0x0B,},                                        
	[CMD_GAMMA_SETTING_G_POS    ] = {0x29,0x09,0xED,0x00,0x33,0x12,0x10,0x98,0x88,0x87,0x0B,},                                        
	[CMD_GAMMA_SETTING_G_NEG    ] = {0x29,0x09,0xEE,0x00,0x33,0x12,0x10,0x98,0x88,0x87,0x0B,},                                        
	[CMD_GAMMA_SETTING_B_POS    ] = {0x29,0x09,0xEF,0x00,0x33,0x12,0x10,0x98,0x88,0x87,0x0B,},                                        
	[CMD_GAMMA_SETTING_B_NEG    ] = {0x29,0x09,0xF0,0x00,0x33,0x12,0x10,0x98,0x88,0x87,0x0B,},                                        
	[NUM_CMD]                    = {0,},
	[CMD_SET_TEAR_SCANLINE      ] = {0x39,0x03,0x44,0x00,0x00,},
};

#define LCD_CMD(a)	(lcd_command_for_mipi[(a)][0])
#define LCD_DAT(a)	(&lcd_command_for_mipi[(a)][2])
#define LCD_LEN(a)	(lcd_command_for_mipi[(a)][1])

struct hub_data {
	struct mutex lock;

	struct backlight_device *bldev;

	unsigned long	hw_guard_end;	/* next value of jiffies when we can
					 * issue the next sleep in/out command
					 */
	unsigned long	hw_guard_wait;	/* max guard time in jiffies */

	struct omap_dss_device *dssdev;

	bool enabled;
	u8 rotate;
	bool mirror;

	bool te_enabled;

	atomic_t do_update;
	struct {
		u16 x;
		u16 y;
		u16 w;
		u16 h;
	} update_region;
	struct delayed_work te_timeout_work;

	bool use_ext_te;
	bool use_dsi_bl;

	bool cabc_broken;
	unsigned cabc_mode;

	bool intro_printed;

#ifdef HUB_USE_ESD_CHECK
	struct workqueue_struct *esd_wq;
	struct delayed_work esd_work;
#endif
	u8 gpio_lcd_reset_n;
	u8 gpio_lcd_cs;
	u8 gpio_lcd_maker_id;
};

int check_no_lcd(void)
{
    //printk("check_no_lcd for test mode = %d \n",no_lcd_flag);
    return no_lcd_flag;
}
EXPORT_SYMBOL(check_no_lcd);

static int lcd_status_check(enum omap_dsi_index ix)
{
	u8 data;
	hub_dcs_read_1(ix, 0x0a, &data);

	if (data==0x94)
	{
		DBG("LCD is Normal\n");
		return 0;
	}
	else
	{
		DBG("LCD is Abnormal\n");
		return 1;
	}
}

static void hw_guard_start(struct hub_data *td, int guard_msec)
{
	td->hw_guard_wait = msecs_to_jiffies(guard_msec);
	td->hw_guard_end = jiffies + td->hw_guard_wait;
}

static void hw_guard_wait(struct hub_data *td)
{
	unsigned long wait = td->hw_guard_end - jiffies;

	if ((long)wait > 0 && wait <= td->hw_guard_wait) {
		set_current_state(TASK_UNINTERRUPTIBLE);
		schedule_timeout(wait);
	}
}

static int hub_dcs_read_1(enum omap_dsi_index ix, u8 dcs_cmd, u8 *data)
{
	int r;
	u8 buf[1];
	
	if (lcd_off_boot == 1)
		return 0;
	r = dsi_vc_dcs_read(ix, TCH, dcs_cmd, buf, 1);

	if (r < 0)
		return r;

	*data = buf[0];

	return 0;
}

static int hub_dcs_write_0(enum omap_dsi_index ix, u8 dcs_cmd)
{
	if (lcd_off_boot ==1)
		return 0;
	return dsi_vc_dcs_write(ix, TCH, &dcs_cmd, 1);
}

static int hub_dcs_write_1(enum omap_dsi_index ix, u8 dcs_cmd, u8 param)
{
	u8 buf[2];
	buf[0] = dcs_cmd;
	buf[1] = param;
	
	if (lcd_off_boot == 1)
		return 0;
	return dsi_vc_dcs_write(ix, TCH, buf, 2);
}

static int hub_sleep_in(enum omap_dsi_index ix, struct hub_data *td)
{
	u8 cmd;
	int r;

	hw_guard_wait(td);

	cmd = DCS_SLEEP_IN;
	r = dsi_vc_dcs_write_nosync(ix, TCH, &cmd, 1);
	if (r)
		return r;

	hw_guard_start(td, 120);

	msleep(50);

	return 0;
}

static int hub_sleep_out(enum omap_dsi_index ix, struct hub_data *td)
{
	int r;

	hw_guard_wait(td);

	r = hub_dcs_write_0(ix, DCS_SLEEP_OUT);
	if (r)
		return r;

	hw_guard_start(td, 120);

	msleep(100);

	return 0;
}

static int hub_get_id(enum omap_dsi_index ix, u8 *id1)
{
	int r;

	r = hub_dcs_read_1(ix, DCS_GET_ID1, id1);
	if (r)
		return r;

	return 0;
}

static int hub_set_addr_mode(enum omap_dsi_index ix, u8 rotate, bool mirror)
{
	int r;
	u8 mode;
	int b5, b6, b7;

	r = hub_dcs_read_1(ix, DCS_READ_MADCTL, &mode);
	if (r)
		return r;

	switch (rotate) {
	default:
	case 0:
		b7 = 0;
		b6 = 0;
		b5 = 0;
		break;
	case 1:
		b7 = 0;
		b6 = 1;
		b5 = 1;
		break;
	case 2:
		b7 = 1;
		b6 = 1;
		b5 = 0;
		break;
	case 3:
		b7 = 1;
		b6 = 0;
		b5 = 1;
		break;
	}

	if (mirror)
		b6 = !b6;

	mode &= ~((1<<7) | (1<<6) | (1<<5));
	mode |= (b7 << 7) | (b6 << 6) | (b5 << 5);

	return hub_dcs_write_1(ix, DCS_MEM_ACC_CTRL, mode);
}

static int hub_set_update_window(enum omap_dsi_index ix,
	u16 x, u16 y, u16 w, u16 h)
{
	int r = 0;
	u16 x1 = x;
	u16 x2 = x + w - 1;
	u16 y1 = y;
	u16 y2 = y + h - 1;

	u8 buf[5];
	buf[0] = DCS_COLUMN_ADDR;
	buf[1] = (x1 >> 8) & 0xff;
	buf[2] = (x1 >> 0) & 0xff;
	buf[3] = (x2 >> 8) & 0xff;
	buf[4] = (x2 >> 0) & 0xff;

	r = dsi_vc_dcs_write_nosync(ix, TCH, buf, sizeof(buf));
	if (r)
		return r;

	buf[0] = DCS_PAGE_ADDR;
	buf[1] = (y1 >> 8) & 0xff;
	buf[2] = (y1 >> 0) & 0xff;
	buf[3] = (y2 >> 8) & 0xff;
	buf[4] = (y2 >> 0) & 0xff;

	r = dsi_vc_dcs_write_nosync(ix, TCH, buf, sizeof(buf));
	if (r)
		return r;

	dsi_vc_send_bta_sync(ix, TCH);

	return r;
}

static int hub_bl_update_status(struct backlight_device *dev)
{
	struct omap_dss_device *dssdev = dev_get_drvdata(&dev->dev);
	struct hub_data *td = dev_get_drvdata(&dssdev->dev);
	int r;
	int level;
	enum omap_dsi_index ix;

	ix = (dssdev->channel == OMAP_DSS_CHANNEL_LCD) ? DSI1 : DSI2;

	if (dev->props.fb_blank == FB_BLANK_UNBLANK &&
			dev->props.power == FB_BLANK_UNBLANK)
		level = dev->props.brightness;
	else
		level = 0;

	dev_dbg(&dssdev->dev, "update brightness to %d\n", level);

	//mutex_lock(&td->lock);

	if (td->use_dsi_bl) {
		if (td->enabled) {
			dsi_bus_lock(ix);
			r = hub_dcs_write_1(ix, DCS_BRIGHTNESS, level);
			dsi_bus_unlock(ix);
		} else {
			r = 0;
		}
	} else {
		if (!dssdev->set_backlight)
			r = -EINVAL;
		else
			r = dssdev->set_backlight(dssdev, level);
	}

	//mutex_unlock(&td->lock);

	return r;
}

static int hub_bl_get_intensity(struct backlight_device *dev)
{
	if (dev->props.fb_blank == FB_BLANK_UNBLANK &&
			dev->props.power == FB_BLANK_UNBLANK)
		return dev->props.brightness;

	return 0;
}

static struct backlight_ops hub_bl_ops = {
	.get_brightness = hub_bl_get_intensity,
	.update_status  = hub_bl_update_status,
};

static void hub_get_timings(struct omap_dss_device *dssdev,
			struct omap_video_timings *timings)
{
	*timings = dssdev->panel.timings;
}

static void hub_set_timings(struct omap_dss_device *dssdev,
			struct omap_video_timings *timings)
{
	dssdev->panel.timings.x_res = timings->x_res;
	dssdev->panel.timings.y_res = timings->y_res;
	dssdev->panel.timings.pixel_clock = timings->pixel_clock;
	dssdev->panel.timings.hsw = timings->hsw;
	dssdev->panel.timings.hfp = timings->hfp;
	dssdev->panel.timings.hbp = timings->hbp;
	dssdev->panel.timings.vsw = timings->vsw;
	dssdev->panel.timings.vfp = timings->vfp;
	dssdev->panel.timings.vbp = timings->vbp;
}

static int hub_check_timings(struct omap_dss_device *dssdev,
			struct omap_video_timings *timings)
{
	if (timings->x_res != 480 || timings->y_res != 800)
		return -EINVAL;

	return 0;
}

static void hub_get_resolution(struct omap_dss_device *dssdev,
		u16 *xres, u16 *yres)
{
	struct hub_data *td = dev_get_drvdata(&dssdev->dev);

	if (td->rotate == 0 || td->rotate == 2) {
		*xres = dssdev->panel.timings.x_res;
		*yres = dssdev->panel.timings.y_res;
	} else {
		*yres = dssdev->panel.timings.x_res;
		*xres = dssdev->panel.timings.y_res;
	}
}

static ssize_t hub_num_errors_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct omap_dss_device *dssdev = to_dss_device(dev);
	struct hub_data *td = dev_get_drvdata(&dssdev->dev);
	u8 errors;
	int r;
	enum omap_dsi_index ix;

	ix = (dssdev->channel == OMAP_DSS_CHANNEL_LCD) ? DSI1 : DSI2;

	mutex_lock(&td->lock);

	if (td->enabled) {
		dsi_bus_lock(ix);
		r = hub_dcs_read_1(ix, DCS_READ_NUM_ERRORS, &errors);
		dsi_bus_unlock(ix);
	} else {
		r = -ENODEV;
	}

	mutex_unlock(&td->lock);

	if (r)
		return r;

	return snprintf(buf, PAGE_SIZE, "%d\n", errors);
}

static ssize_t hub_hw_revision_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct omap_dss_device *dssdev = to_dss_device(dev);
	struct hub_data *td = dev_get_drvdata(&dssdev->dev);
	u8 id1;
	int r;
	enum omap_dsi_index ix;

	ix = (dssdev->channel == OMAP_DSS_CHANNEL_LCD) ? DSI1 : DSI2;

	mutex_lock(&td->lock);

	if (td->enabled) {
		dsi_bus_lock(ix);
		r = hub_get_id(ix, &id1);
		dsi_bus_unlock(ix);
	} else {
		r = -ENODEV;
	}

	mutex_unlock(&td->lock);

	if (r)
		return r;

	return snprintf(buf, PAGE_SIZE, "%02x\n", id1);
}

static const char *cabc_modes[] = {
	"off",		/* used also always when CABC is not supported */
	"ui",
	"still-image",
	"moving-image",
};

static ssize_t show_cabc_mode(struct device *dev,
		struct device_attribute *attr,
		char *buf)
{
	struct omap_dss_device *dssdev = to_dss_device(dev);
	struct hub_data *td = dev_get_drvdata(&dssdev->dev);
	const char *mode_str;
	int mode;
	int len;

	mode = td->cabc_mode;

	mode_str = "unknown";
	if (mode >= 0 && mode < ARRAY_SIZE(cabc_modes))
		mode_str = cabc_modes[mode];
	len = snprintf(buf, PAGE_SIZE, "%s\n", mode_str);

	return len < PAGE_SIZE - 1 ? len : PAGE_SIZE - 1;
}

static ssize_t store_cabc_mode(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{
	struct omap_dss_device *dssdev = to_dss_device(dev);
	struct hub_data *td = dev_get_drvdata(&dssdev->dev);
	int i;
	enum omap_dsi_index ix;

	ix = (dssdev->channel == OMAP_DSS_CHANNEL_LCD) ? DSI1 : DSI2;

	for (i = 0; i < ARRAY_SIZE(cabc_modes); i++) {
		if (sysfs_streq(cabc_modes[i], buf))
			break;
	}

	if (i == ARRAY_SIZE(cabc_modes))
		return -EINVAL;

	mutex_lock(&td->lock);

	if (td->enabled) {
		dsi_bus_lock(ix);
		if (!td->cabc_broken)
			hub_dcs_write_1(ix, DCS_WRITE_CABC, i);
		dsi_bus_unlock(ix);
	}

	td->cabc_mode = i;

	mutex_unlock(&td->lock);

	return count;
}

static ssize_t show_cabc_available_modes(struct device *dev,
		struct device_attribute *attr,
		char *buf)
{
	int len;
	int i;

	for (i = 0, len = 0;
	     len < PAGE_SIZE && i < ARRAY_SIZE(cabc_modes); i++)
		len += snprintf(&buf[len], PAGE_SIZE - len, "%s%s%s",
			i ? " " : "", cabc_modes[i],
			i == ARRAY_SIZE(cabc_modes) - 1 ? "\n" : "");

	return len < PAGE_SIZE ? len : PAGE_SIZE - 1;
}

static DEVICE_ATTR(num_dsi_errors, S_IRUGO, hub_num_errors_show, NULL);
static DEVICE_ATTR(hw_revision, S_IRUGO, hub_hw_revision_show, NULL);
static DEVICE_ATTR(cabc_mode, S_IRUGO | S_IWUSR,
		show_cabc_mode, store_cabc_mode);
static DEVICE_ATTR(cabc_available_modes, S_IRUGO,
		show_cabc_available_modes, NULL);

static struct attribute *hub_attrs[] = {
	&dev_attr_num_dsi_errors.attr,
	&dev_attr_hw_revision.attr,
	&dev_attr_cabc_mode.attr,
	&dev_attr_cabc_available_modes.attr,
	NULL,
};

static struct attribute_group hub_attr_group = {
	.attrs = hub_attrs,
};

static void hub_hw_reset(struct omap_dss_device *dssdev)
{
	struct hub_data *td = dev_get_drvdata(&dssdev->dev);

	gpio_set_value(td->gpio_lcd_reset_n, 1);
	mdelay(5);
	gpio_set_value(td->gpio_lcd_reset_n, 0);
	mdelay(20);
	gpio_set_value(td->gpio_lcd_reset_n, 1);
	mdelay(5);
}

static void hub_panel_init_lcd(struct omap_dss_device *dssdev)
{
	struct hub_data *td = dev_get_drvdata(&dssdev->dev);

#ifdef CONFIG_FB_OMAP_BOOTLOADER_INIT
	if(gpio_request(td->gpio_lcd_cs, "lcd cs") < 0) {
		return;
	}
	gpio_direction_output(td->gpio_lcd_cs, 1);
#else
	if(gpio_request(td->gpio_lcd_reset_n, "lcd gpio") < 0) {
		return;
	}
	gpio_direction_output(td->gpio_lcd_reset_n, 1);
	gpio_set_value(td->gpio_lcd_reset_n, 0);

	if(gpio_request(td->gpio_lcd_cs, "lcd cs") < 0) {
		return;
	}
	gpio_direction_output(gpio_lcd_cs, 1);
#endif
}

static void hub_get_dimension(struct omap_dss_device *dssdev,
		u32 *width, u32 *height)
{
	*width = dssdev->panel.width_in_mm;
	*height= dssdev->panel.height_in_mm;
}

static int hub_probe(struct omap_dss_device *dssdev)
{
	struct backlight_properties props;
	struct hub_data *td;
	struct backlight_device *bldev;
	int r;
	
	dev_dbg(&dssdev->dev, "probe\n");

	td = kzalloc(sizeof(*td), GFP_KERNEL);
	if (!td) {
		r = -ENOMEM;
		goto err;
	}
	td->dssdev = dssdev;

	td->gpio_lcd_reset_n	= GPIO_LCD_RESET_N;
	td->gpio_lcd_cs		= GPIO_LCD_CS;
	td->gpio_lcd_maker_id	= GPIO_LCD_MAKER_ID;

	mutex_init(&td->lock);

#ifdef HUB_USE_ESD_CHECK
	td->esd_wq = create_singlethread_workqueue("hub_esd");
	if (td->esd_wq == NULL) {
		dev_err(&dssdev->dev, "can't create ESD workqueue\n");
		r = -ENOMEM;
		goto err_wq;
	}
	INIT_DELAYED_WORK_DEFERRABLE(&td->esd_work, hub_esd_work);
#endif

	dev_set_drvdata(&dssdev->dev, td);

	hub_panel_init_lcd(dssdev);

	/* if no platform set_backlight() defined, presume DSI backlight
	 * control */
	memset(&props, 0, sizeof(struct backlight_properties));
	if (!dssdev->set_backlight) {
		td->use_dsi_bl = true;
	}

	if (td->use_dsi_bl) {
		props.max_brightness = 255;
	}
	else {
		props.max_brightness = 127;
	}

	bldev = backlight_device_register(dssdev->name,
		       			  &dssdev->dev, 
					  dssdev, 
					  &hub_bl_ops, 
					  &props);

	if (IS_ERR(bldev)) {
		r = PTR_ERR(bldev);
		goto err_bl;
	}

	td->bldev = bldev;

	bldev->props.fb_blank	= FB_BLANK_UNBLANK;
	bldev->props.power	= FB_BLANK_UNBLANK;
	if (td->use_dsi_bl) {
		bldev->props.max_brightness = 255;
		bldev->props.brightness = 255;
	}
	else {
		bldev->props.max_brightness = 127;
		bldev->props.brightness = 127;
	}

	hub_bl_update_status(bldev);

	if (dssdev->phy.dsi.ext_te) {
		int gpio = dssdev->phy.dsi.ext_te_gpio;

		r = gpio_request(gpio, "hub irq");
		if (r) {
			dev_err(&dssdev->dev, "GPIO request failed\n");
			goto err_gpio;
		}

		gpio_direction_input(gpio);

		r = request_irq(OMAP_GPIO_IRQ(gpio), hub_te_isr, 
				IRQF_TRIGGER_RISING, "hub vsync", dssdev);

		if (r) {
			dev_err(&dssdev->dev, "IRQ request failed\n");
			gpio_free(gpio);
			goto err_irq;
		}

		INIT_DELAYED_WORK_DEFERRABLE(&td->te_timeout_work,
				hub_te_timeout_work_callback);

		dev_dbg(&dssdev->dev, "Using GPIO TE\n");

		td->use_ext_te = true;
	}

	r = sysfs_create_group(&dssdev->dev.kobj, &hub_attr_group);
	if (r) {
		dev_err(&dssdev->dev, "failed to create sysfs files\n");
		goto err_sysfs;
	}

/*
 * the code location of enabled status must be at the end of function.*/
#ifdef CONFIG_FB_OMAP_BOOTLOADER_INIT
	if (dssdev->channel == OMAP_DSS_CHANNEL_LCD) {
		td->enabled = 1;
	}
#endif

	return 0;

err_sysfs:
	if (td->use_ext_te) {
		int gpio = dssdev->phy.dsi.ext_te_gpio;
		free_irq(gpio_to_irq(gpio), dssdev);
	}
err_irq:
	if (td->use_ext_te) {
		int gpio = dssdev->phy.dsi.ext_te_gpio;
		gpio_free(gpio);
	}
err_gpio:
	backlight_device_unregister(bldev);
err_bl:
#ifdef HUB_USE_ESD_CHECK
	destroy_workqueue(td->esd_wq);
err_wq:
#endif
	kfree(td);
err:
	return r;
}

static void hub_remove(struct omap_dss_device *dssdev)
{
	struct hub_data *td = dev_get_drvdata(&dssdev->dev);
	struct backlight_device *bldev;

	dev_dbg(&dssdev->dev, "remove\n");

	sysfs_remove_group(&dssdev->dev.kobj, &hub_attr_group);

	if (td->use_ext_te) {
		int gpio = dssdev->phy.dsi.ext_te_gpio;
		free_irq(gpio_to_irq(gpio), dssdev);
		gpio_free(gpio);
	}

	bldev = td->bldev;
	bldev->props.power = FB_BLANK_POWERDOWN;
	hub_bl_update_status(bldev);
	backlight_device_unregister(bldev);

#ifdef HUB_USE_ESD_CHECK
	cancel_delayed_work_sync(&td->esd_work);
	destroy_workqueue(td->esd_wq);
#endif

	/* reset, to be sure that the panel is in a valid state */
	hub_hw_reset(dssdev);

	kfree(td);
}

static int hub_power_on(struct omap_dss_device *dssdev)
{
	struct hub_data *td = dev_get_drvdata(&dssdev->dev);
	int i, r;
	enum omap_dsi_index ix;

	ix = (dssdev->channel == OMAP_DSS_CHANNEL_LCD) ? DSI1 : DSI2;

//	if (dssdev->platform_enable) {
	if (!td->enabled && dssdev->platform_enable) {
 		r = dssdev->platform_enable(dssdev);
		if (r)
			return r;
	}

	r = omapdss_dsi_display_enable(dssdev);
	if (r) {
		dev_err(&dssdev->dev, "failed to enable DSI\n");
		goto err0;
	}

	if(lcd_off_boot==1)
		goto err;

 #if 0
	hub_hw_reset(dssdev);
#else
	if (!td->enabled)
		hub_hw_reset(dssdev);
#endif
 
	read_status_reg(ix, "after hw reset");

	/* it seems we have to wait a bit until cosmo_panel is ready */
	mdelay(5);	

	omapdss_dsi_vc_enable_hs(ix, TCH, false);

	r = hub_sleep_out(ix, td);
	if (r)
		goto err;

	// initialize device
	for (i = 0; i < NUM_CMD; i++) {
		dsi_vc_write(ix, TCH, LCD_CMD(i), LCD_DAT(i), LCD_LEN(i));
	}

	hub_dcs_write_0(ix, DCS_DISPLAY_ON);

	read_status_reg(ix, "after disp on");

	printk("DISPLAY ON & start tearing ON\n");
//HARDLINE	mdelay(30); //after 1 frame

#ifdef HUB_USE_ESD_CHECK
	queue_delayed_work(td->esd_wq, &td->esd_work, HUB_ESD_CHECK_PERIOD);
#endif

	r = _hub_enable_te(dssdev, td->te_enabled);
	if (r)
		goto err;

	td->enabled = 1;

#if 0////
	if (!td->intro_printed) {
		dev_info(&dssdev->dev, "revision %02x\n",
				id1);
		if (td->cabc_broken)
			dev_info(&dssdev->dev,
					"old Hub version, CABC disabled\n");
		td->intro_printed = true;
	}
#endif

	omapdss_dsi_vc_enable_hs(ix, TCH, true);

	return 0;
err:
	dev_err(&dssdev->dev, "error while enabling panel, issuing HW reset\n");

	//lcd_off_boot =1;

	if( lcd_off_boot == 0){
		hub_hw_reset(dssdev);
		omapdss_dsi_display_disable(dssdev);
	}
err0:
	return r;
}

static void hub_power_off(struct omap_dss_device *dssdev)
{
	struct hub_data *td = dev_get_drvdata(&dssdev->dev);
	int r;
	enum omap_dsi_index ix;

	ix = (dssdev->channel == OMAP_DSS_CHANNEL_LCD) ? DSI1 : DSI2;

	if (!td->enabled)
		return;

	read_status_reg(ix, "after disable");

	r = hub_dcs_write_0(ix, DCS_DISPLAY_OFF);
	if (!r) {
		msleep(35);//B-prj
		r = hub_sleep_in(ix, td);
		/* HACK: wait a bit so that the message goes through */
		msleep(10);
	}

	if (r) {
		dev_err(&dssdev->dev,
				"error disabling panel, issuing HW reset\n");
		hub_hw_reset(dssdev);
	}

	omapdss_dsi_display_disable(dssdev);

#if 1
	if (dssdev->platform_disable) {
		dssdev->platform_disable(dssdev);
	}
#endif
 
	td->enabled = 0;
}

static int hub_start(struct omap_dss_device *dssdev)
{
#ifdef HUB_USE_ESD_CHECK
	struct hub_data *td = dev_get_drvdata(&dssdev->dev);
#endif
	int r = 0;
	enum omap_dsi_index ix;

	ix = (dssdev->channel == OMAP_DSS_CHANNEL_LCD) ? DSI1 : DSI2;

	dsi_bus_lock(ix);

	r = hub_power_on(dssdev);

	dsi_bus_unlock(ix);

	if (r) {
		dev_dbg(&dssdev->dev, "enable failed\n");
		dssdev->state = OMAP_DSS_DISPLAY_DISABLED;
	} else {
		dssdev->state = OMAP_DSS_DISPLAY_ACTIVE;
#ifdef HUB_USE_ESD_CHECK
		queue_delayed_work(td->esd_wq, &td->esd_work, HUB_ESD_CHECK_PERIOD);
#endif
	}

	return r;
}

static void hub_stop(struct omap_dss_device *dssdev)
{
#ifdef HUB_USE_ESD_CHECK
	struct hub_data *td = dev_get_drvdata(&dssdev->dev);
#endif
	enum omap_dsi_index ix;

	ix = (dssdev->channel == OMAP_DSS_CHANNEL_LCD) ? DSI1 : DSI2;

#ifdef HUB_USE_ESD_CHECK
	cancel_delayed_work(&td->esd_work);
#endif

	dsi_bus_lock(ix);

	hub_power_off(dssdev);

	dsi_bus_unlock(ix);
}

static void hub_disable(struct omap_dss_device *dssdev)
{
	struct hub_data *td = dev_get_drvdata(&dssdev->dev);

	dev_dbg(&dssdev->dev, "disable\n");

	mutex_lock(&td->lock);

	if (dssdev->state == OMAP_DSS_DISPLAY_ACTIVE ||
	    dssdev->state == OMAP_DSS_DISPLAY_TRANSITION)
		hub_stop(dssdev);

	dssdev->state = OMAP_DSS_DISPLAY_DISABLED;

	mutex_unlock(&td->lock);
}

static int hub_suspend(struct omap_dss_device *dssdev)
{
	struct hub_data *td = dev_get_drvdata(&dssdev->dev);
	struct backlight_device *bldev = td->bldev;
	int r = 0;

	dev_dbg(&dssdev->dev, "suspend\n");

	mutex_lock(&td->lock);

	bldev->props.power = FB_BLANK_POWERDOWN;
	hub_bl_update_status(bldev);
	if (dssdev->state != OMAP_DSS_DISPLAY_ACTIVE) {
		r = -EINVAL;
		goto err;
	}
	hub_stop(dssdev);

	dssdev->state = OMAP_DSS_DISPLAY_SUSPENDED;
err:
	mutex_unlock(&td->lock);
	return r;
}

static int hub_enable(struct omap_dss_device *dssdev)
{
	struct hub_data *td = dev_get_drvdata(&dssdev->dev);
	int r = 0;

	dev_dbg(&dssdev->dev, "enable\n");

	mutex_lock(&td->lock);

	if (dssdev->state != OMAP_DSS_DISPLAY_DISABLED) {
		r = -EINVAL;
		goto err;
	}

	r = hub_start(dssdev);
err:
	mutex_unlock(&td->lock);
	return r;
}

static int hub_resume(struct omap_dss_device *dssdev)
{
	struct hub_data *td = dev_get_drvdata(&dssdev->dev);
	struct backlight_device *bldev = td->bldev;
	int r = 0;

	dev_dbg(&dssdev->dev, "resume\n");

	mutex_lock(&td->lock);

	bldev->props.power = FB_BLANK_UNBLANK;
	hub_bl_update_status(bldev);
	if (dssdev->state != OMAP_DSS_DISPLAY_SUSPENDED) {
		r = -EINVAL;
		goto err;
	}
	r = hub_start(dssdev);
err:
	mutex_unlock(&td->lock);
	return r;
}


static void hub_framedone_cb(int err, void *data)
{
	struct omap_dss_device *dssdev = data;
	enum omap_dsi_index ix;

	ix = (dssdev->channel == OMAP_DSS_CHANNEL_LCD) ? DSI1 : DSI2;

	dev_dbg(&dssdev->dev, "framedone, err %d\n", err);
	dsi_bus_unlock(ix);
}

static irqreturn_t hub_te_isr(int irq, void *data)
{
	struct omap_dss_device *dssdev = data;
	struct hub_data *td = dev_get_drvdata(&dssdev->dev);
	int old;
	int r;

	old = atomic_cmpxchg(&td->do_update, 1, 0);

	if (old) {
		cancel_delayed_work(&td->te_timeout_work);

		r = omap_dsi_update(dssdev, TCH,
				td->update_region.x,
				td->update_region.y,
				td->update_region.w,
				td->update_region.h,
				hub_framedone_cb, dssdev);
		if (r)
			goto err;
	}

	return IRQ_HANDLED;
err:
	dev_err(&dssdev->dev, "start update failed\n");
	//dsi_bus_unlock(DSI1);
	return IRQ_HANDLED;
}

static void hub_te_timeout_work_callback(struct work_struct *work)
{
	struct hub_data *td = container_of(work, struct hub_data,
					te_timeout_work.work);
	struct omap_dss_device *dssdev = td->dssdev;
	enum omap_dsi_index ix;

	ix = (dssdev->channel == OMAP_DSS_CHANNEL_LCD) ? DSI1 : DSI2;

	dev_err(&dssdev->dev, "TE not received for 250ms!\n");

	atomic_set(&td->do_update, 0);
	//dsi_bus_unlock(ix);
}

static int hub_update_locked(struct omap_dss_device *dssdev,
				    u16 x, u16 y, u16 w, u16 h)
{
	struct hub_data *td = dev_get_drvdata(&dssdev->dev);
	int r;
	enum omap_dsi_index ix;

	ix = (dssdev->channel == OMAP_DSS_CHANNEL_LCD) ? DSI1 : DSI2;

	WARN_ON(!dsi_bus_is_locked(ix));

	if (!td->enabled) {
		r = 0;
		goto err;
	}

	r = omap_dsi_prepare_update(dssdev, &x, &y, &w, &h, true);
	if (r)
		goto err;

	r = hub_set_update_window(ix, x, y, w, h);
	if (r)
		goto err;

#if 1 
	if (td->te_enabled && td->use_ext_te) {
		td->update_region.x = x;
		td->update_region.y = y;
		td->update_region.w = w;
		td->update_region.h = h;
		barrier();
		schedule_delayed_work(&td->te_timeout_work,
				msecs_to_jiffies(250));
		atomic_set(&td->do_update, 1);
	} else {
		/* We use VC(1) for VideoPort Data and VC(0) for L4 data */
		if (cpu_is_omap44xx())
			r = omap_dsi_update(dssdev, 1, x, y, w, h,
				hub_framedone_cb, dssdev);
		else
			r = omap_dsi_update(dssdev, TCH, x, y, w, h,
				hub_framedone_cb, dssdev);
		if (r)
			goto err;
	}
#else
	r = omap_dsi_update(dssdev, TCH, x, y, w, h,
		hub_framedone_cb, dssdev);
	if (r)
		goto err;
#endif

	/* note: no bus_unlock here. unlock is in framedone_cb */
	mutex_unlock(&td->lock);
	return 0;
err:
	dsi_bus_unlock(ix);
	mutex_unlock(&td->lock);
	return r;
}

static int hub_update(struct omap_dss_device *dssdev,
				    u16 x, u16 y, u16 w, u16 h)
{
	enum omap_dsi_index ix;
	struct hub_data *td = dev_get_drvdata(&dssdev->dev);

	dev_dbg(&dssdev->dev, "update %d, %d, %d x %d\n", x, y, w, h);
	ix = (dssdev->channel == OMAP_DSS_CHANNEL_LCD) ? DSI1 : DSI2;

	mutex_lock(&td->lock);

	/* mark while waiting on bus so delayed update will not call update */
	dssdev->sched_update.waiting = true;
	dsi_bus_lock(ix);
	dssdev->sched_update.waiting = false;

	return hub_update_locked(dssdev, x, y, w, h);
}

static int hub_sched_update(struct omap_dss_device *dssdev,
					u16 x, u16 y, u16 w, u16 h)
{
	struct hub_data *td = dev_get_drvdata(&dssdev->dev);
	int r;

 	extern int get_fb_state(void);
	if (!get_fb_state()) return 0;
 
	if (mutex_trylock(&td->lock)) {
		r = omap_dsi_sched_update_lock(dssdev, x, y, w, h, false);

		if (!r) {
			/* start the update now */
			r = hub_update_locked(dssdev, x, y, w, h);

			return r;
		}

		if (r == -EBUSY)
			r = 0;

		mutex_unlock(&td->lock);

	} else {
		/* this locks dsi bus if it can and returns 0 */
		r = omap_dsi_sched_update_lock(dssdev, x, y, w, h, true);

		if (r == -EBUSY)
			r = 0;
	}

	return r;
}

static int hub_sync(struct omap_dss_device *dssdev)
{
	struct hub_data *td = dev_get_drvdata(&dssdev->dev);
	enum omap_dsi_index ix;

	ix = (dssdev->channel == OMAP_DSS_CHANNEL_LCD) ? DSI1 : DSI2;

	dev_dbg(&dssdev->dev, "sync\n");

  	mutex_lock(&td->lock);
	dsi_bus_lock(ix);
	dsi_bus_unlock(ix);
	mutex_unlock(&td->lock);

	dev_dbg(&dssdev->dev, "sync done\n");

	return 0;
}

static int _hub_enable_te(struct omap_dss_device *dssdev, bool enable)
{
	struct hub_data *td = dev_get_drvdata(&dssdev->dev);
	int r = 0;
	enum omap_dsi_index ix;

	ix = (dssdev->channel == OMAP_DSS_CHANNEL_LCD) ? DSI1 : DSI2;

#if 0
	if (enable) {
		if (!td->te_enabled && td->enabled) {
			r = hub_dcs_write_1(ix, DCS_TEAR_ON, 1);
			dsi_vc_write(ix, TCH, LCD_CMD(CMD_SET_TEAR_SCANLINE), LCD_DAT(CMD_SET_TEAR_SCANLINE), LCD_LEN(CMD_SET_TEAR_SCANLINE));
//			r = hub_dcs_write_1(ix, DCS_TEAR_ON, 1);
			r = hub_dcs_write_1(ix, DCS_TEAR_ON, 0);
		}
	} else {
		if (td->te_enabled)
			r = hub_dcs_write_0(ix, DCS_TEAR_OFF);
	}
#else  

	if (enable)
	{
		r = hub_dcs_write_1(ix, DCS_TEAR_ON, 1); 
		dsi_vc_write(ix, TCH, LCD_CMD(CMD_SET_TEAR_SCANLINE), LCD_DAT(CMD_SET_TEAR_SCANLINE), LCD_LEN(CMD_SET_TEAR_SCANLINE));
		r = hub_dcs_write_1(ix, DCS_TEAR_ON, 0);
		
	}
	else
		r = hub_dcs_write_0(ix, DCS_TEAR_OFF);

#endif

	if (!td->use_ext_te)
		omapdss_dsi_enable_te(dssdev, enable);

	msleep(50);

	return r;
}

static int hub_enable_te(struct omap_dss_device *dssdev, bool enable)
{
	struct hub_data *td = dev_get_drvdata(&dssdev->dev);
	int r;
	enum omap_dsi_index ix;

	ix = (dssdev->channel == OMAP_DSS_CHANNEL_LCD) ? DSI1 : DSI2;

	mutex_lock(&td->lock);

	if (td->te_enabled == enable)
		goto end;

	dsi_bus_lock(ix);

	if (td->enabled) {
		r = _hub_enable_te(dssdev, enable);
		if (r)
			goto err;
	}

	td->te_enabled = enable;

	dsi_bus_unlock(ix);
end:
	mutex_unlock(&td->lock);

	return 0;
err:
	dsi_bus_unlock(ix);
	mutex_unlock(&td->lock);

	return r;
}

static int hub_get_te(struct omap_dss_device *dssdev)
{
	struct hub_data *td = dev_get_drvdata(&dssdev->dev);
	int r;

	mutex_lock(&td->lock);
	r = td->te_enabled;
	mutex_unlock(&td->lock);

	return r;
}

static int hub_rotate(struct omap_dss_device *dssdev, u8 rotate)
{
	struct hub_data *td = dev_get_drvdata(&dssdev->dev);
	int r;
	enum omap_dsi_index ix;

	ix = (dssdev->channel == OMAP_DSS_CHANNEL_LCD) ? DSI1 : DSI2;

	dev_dbg(&dssdev->dev, "rotate %d\n", rotate);

	mutex_lock(&td->lock);

	if (td->rotate == rotate)
		goto end;

	dsi_bus_lock(ix);

	if (td->enabled) {
		r = hub_set_addr_mode(ix, rotate, td->mirror);

		if (r)
			goto err;
	}

	td->rotate = rotate;

	dsi_bus_unlock(ix);
end:
	mutex_unlock(&td->lock);
	return 0;
err:
	dsi_bus_unlock(ix);
	mutex_unlock(&td->lock);
	return r;
}

static u8 hub_get_rotate(struct omap_dss_device *dssdev)
{
	struct hub_data *td = dev_get_drvdata(&dssdev->dev);
	int r;

	mutex_lock(&td->lock);
	r = td->rotate;
	mutex_unlock(&td->lock);

	return r;
}

static int hub_mirror(struct omap_dss_device *dssdev, bool enable)
{
	struct hub_data *td = dev_get_drvdata(&dssdev->dev);
	int r;
	enum omap_dsi_index ix;

	ix = (dssdev->channel == OMAP_DSS_CHANNEL_LCD) ? DSI1 : DSI2;

	dev_dbg(&dssdev->dev, "mirror %d\n", enable);

	mutex_lock(&td->lock);

	if (td->mirror == enable)
		goto end;

	dsi_bus_lock(ix);
	if (td->enabled) {
		r = hub_set_addr_mode(ix, td->rotate, enable);
		if (r)
			goto err;
	}

	td->mirror = enable;

	dsi_bus_unlock(ix);
end:
	mutex_unlock(&td->lock);
	return 0;
err:
	dsi_bus_unlock(ix);
	mutex_unlock(&td->lock);
	return r;
}

static bool hub_get_mirror(struct omap_dss_device *dssdev)
{
	struct hub_data *td = dev_get_drvdata(&dssdev->dev);
	int r;

	mutex_lock(&td->lock);
	r = td->mirror;
	mutex_unlock(&td->lock);

	return r;
}

static int hub_run_test(struct omap_dss_device *dssdev, int test_num)
{
	struct hub_data *td = dev_get_drvdata(&dssdev->dev);
	u8 id1;
	int r;
	enum omap_dsi_index ix;

	ix = (dssdev->channel == OMAP_DSS_CHANNEL_LCD) ? DSI1 : DSI2;

	mutex_lock(&td->lock);

	if (!td->enabled) {
		r = -ENODEV;
		goto err1;
	}

	dsi_bus_lock(ix);

	r = hub_dcs_read_1(ix, DCS_GET_ID1, &id1);
	if (r)
		goto err2;

	dsi_bus_unlock(ix);
	mutex_unlock(&td->lock);
	return 0;
err2:
	dsi_bus_unlock(ix);
err1:
	mutex_unlock(&td->lock);
	return r;
}

static int hub_memory_read(struct omap_dss_device *dssdev,
		void *buf, size_t size,
		u16 x, u16 y, u16 w, u16 h)
{
	int r;
	int first = 1;
	int plen;
	unsigned buf_used = 0;
	struct hub_data *td = dev_get_drvdata(&dssdev->dev);
	enum omap_dsi_index ix;

	ix = (dssdev->channel == OMAP_DSS_CHANNEL_LCD) ? DSI1 : DSI2;

	if (size < w * h * 3)
		return -ENOMEM;

	mutex_lock(&td->lock);

	if (!td->enabled) {
		r = -ENODEV;
		goto err1;
	}

	size = min(w * h * 3,
			dssdev->panel.timings.x_res *
			dssdev->panel.timings.y_res * 3);

	dsi_bus_lock(ix);

	/* plen 1 or 2 goes into short packet. until checksum error is fixed,
	 * use short packets. plen 32 works, but bigger packets seem to cause
	 * an error. */
	if (size % 2)
		plen = 1;
	else
		plen = 2;

	hub_set_update_window(ix, x, y, w, h);

	r = dsi_vc_set_max_rx_packet_size(ix, TCH, plen);
	if (r)
		goto err2;

	while (buf_used < size) {
		u8 dcs_cmd = first ? 0x2e : 0x3e;
		first = 0;

		r = dsi_vc_dcs_read(ix, TCH, dcs_cmd,
				buf + buf_used, size - buf_used);

		if (r < 0) {
			dev_err(&dssdev->dev, "read error\n");
			goto err3;
		}

		buf_used += r;

		if (r < plen) {
			dev_err(&dssdev->dev, "short read\n");
			break;
		}

		if (signal_pending(current)) {
			dev_err(&dssdev->dev, "signal pending, "
					"aborting memory read\n");
			r = -ERESTARTSYS;
			goto err3;
		}
	}

	r = buf_used;

err3:
	dsi_vc_set_max_rx_packet_size(ix, TCH, 1);
err2:
	dsi_bus_unlock(ix);
err1:
	mutex_unlock(&td->lock);
	return r;
}

#ifdef HUB_USE_ESD_CHECK
static void hub_esd_work(struct work_struct *work)
{
	struct hub_data *td = container_of(work, struct hub_data,
			esd_work.work);
	struct omap_dss_device *dssdev = td->dssdev;
	u8 state1, state2;
	int r;
	enum omap_dsi_index ix;
	ix = (dssdev->channel == OMAP_DSS_CHANNEL_LCD) ? DSI1 : DSI2;

	mutex_lock(&td->lock);

	if (!td->enabled) {
		mutex_unlock(&td->lock);
		return;
	}

	dsi_bus_lock(ix);

	r = hub_dcs_read_1(ix, DCS_RDDSDR, &state1);
	if (r) {
		dev_err(&dssdev->dev, "failed to read Hub status\n");
		goto err;
	}

	/* Run self diagnostics */
	r = hub_sleep_out(ix, td);
	if (r) {
		dev_err(&dssdev->dev, "failed to run Hub self-diagnostics\n");
		goto err;
	}

	r = hub_dcs_read_1(ix, DCS_RDDSDR, &state2);
	if (r) {
		dev_err(&dssdev->dev, "failed to read Hub status\n");
		goto err;
	}

	/* Each sleep out command will trigger a self diagnostic and flip
	 * Bit6 if the test passes.
	 */
	if (!((state1 ^ state2) & (1 << 6))) {
		dev_err(&dssdev->dev, "LCD self diagnostics failed\n");
		goto err;
	}
	/* Self-diagnostics result is also shown on TE GPIO line. We need
	 * to re-enable TE after self diagnostics */
	if (td->use_ext_te && td->te_enabled) {
		r = _hub_enable_te(dssdev, true);
		if (r)
			goto err;
	}

	dsi_bus_unlock(ix);

	queue_delayed_work(td->esd_wq, &td->esd_work, HUB_ESD_CHECK_PERIOD);

	mutex_unlock(&td->lock);
	return;
err:
	dev_err(&dssdev->dev, "performing LCD reset\n");

	hub_power_off(dssdev);
	hub_hw_reset(dssdev);
	hub_power_on(dssdev);

	dsi_bus_unlock(ix);

	queue_delayed_work(td->esd_wq, &td->esd_work, HUB_ESD_CHECK_PERIOD);

	mutex_unlock(&td->lock);
}
#endif

static int hub_set_update_mode(struct omap_dss_device *dssdev,
			       enum omap_dss_update_mode mode)
{
	if (mode != OMAP_DSS_UPDATE_MANUAL) {
		return -EINVAL;
	}

	return 0;
}

static enum omap_dss_update_mode 
hub_get_update_mode(struct omap_dss_device *dssdev)
{
	return OMAP_DSS_UPDATE_MANUAL;
}

static struct omap_dss_driver hub_driver = {
	.probe		= hub_probe,
	.remove		= hub_remove,

	.enable		= hub_enable,
	.disable	= hub_disable,
	.suspend	= hub_suspend,
	.resume		= hub_resume,

	.set_update_mode = hub_set_update_mode,
	.get_update_mode = hub_get_update_mode,

	.update		= hub_update,
	.sched_update	= hub_sched_update,
	.sync		= hub_sync,

	.get_resolution	= hub_get_resolution,

	.enable_te	= hub_enable_te,
	.get_te		= hub_get_te,

	.set_rotate	= hub_rotate,
	.get_rotate	= hub_get_rotate,
	.set_mirror	= hub_mirror,
	.get_mirror	= hub_get_mirror,
	.run_test	= hub_run_test,
	.memory_read	= hub_memory_read,

	.get_timings	= hub_get_timings,
	.set_timings	= hub_set_timings,
	.check_timings	= hub_check_timings,
 	.get_dimension = hub_get_dimension,  

	.driver         = {
		.name   = "hub_panel",
		.owner  = THIS_MODULE,
	},
};

#if 0
static struct omap_dss_driver hub_driver2 = {
	.probe		= hub_probe,
	.remove		= hub_remove,

	.enable		= hub_enable,
	.disable	= hub_disable,
	.suspend	= hub_suspend,
	.resume		= hub_resume,

	.set_update_mode = hub_set_update_mode,
	.get_update_mode = hub_get_update_mode,

	.update		= hub_update,
	.sched_update	= hub_sched_update,
	.sync		= hub_sync,

	.get_resolution	= hub_get_resolution,

	.enable_te	= hub_enable_te,
	.get_te		= hub_get_te,

	.set_rotate	= hub_rotate,
	.get_rotate	= hub_get_rotate,
	.set_mirror	= hub_mirror,
	.get_mirror	= hub_get_mirror,
	.run_test	= hub_run_test,
	.memory_read	= hub_memory_read,

	.get_timings	= hub_get_timings,
	.set_timings	= hub_set_timings,
	.check_timings	= hub_check_timings,

	.driver         = {
		.name   = "hub_panel2",
		.owner  = THIS_MODULE,
	},
};
#endif

static int __init hub_init(void)
{
	omap_dss_register_driver(&hub_driver);
#if 0
	omap_dss_register_driver(&hub_driver2);
#endif

	return 0;
}

static void __exit hub_exit(void)
{
	omap_dss_unregister_driver(&hub_driver);
#if 0
	omap_dss_unregister_driver(&hub_driver2);
#endif
}

module_init(hub_init);
module_exit(hub_exit);

MODULE_AUTHOR("kyungtae Oh ");
MODULE_DESCRIPTION("HUB Driver");
MODULE_LICENSE("GPL");
