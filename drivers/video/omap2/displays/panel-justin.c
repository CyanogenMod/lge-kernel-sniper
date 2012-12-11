/*
 * lh430wv4_panel  DSI Video Mode Panel Driver
 *
 * modified from panel-justin.c
 * kyungtae.oh@lge.com 
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

//#define DEBUG

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
#include <video/omapdss.h>

#include <video/lge-dsi-panel.h>

/* DSI Virtual channel. Hardcoded for now. */
#define TCH 0

#define DCS_READ_NUM_ERRORS	0x05
#define DCS_READ_POWER_MODE	0x0a
#define DCS_READ_MADCTL		0x0b
#define DCS_READ_PIXEL_FORMAT	0x0c
#define DCS_RDDSDR		0x0f
#define DCS_SLEEP_IN		0x10
#define DCS_SLEEP_OUT		0x11
#define DCS_DISPLAY_OFF		0x28
#define DCS_DISPLAY_ON		0x29
#define DCS_COLUMN_ADDR		0x2a
#define DCS_PAGE_ADDR		0x2b
#define DCS_MEMORY_WRITE	0x2c
#define DCS_TEAR_OFF		0x34
#define DCS_TEAR_ON		0x35
#define DCS_MEM_ACC_CTRL	0x36
#define DCS_PIXEL_FORMAT	0x3a
#define DCS_BRIGHTNESS		0x51
#define DCS_CTRL_DISPLAY	0x53
#define DCS_WRITE_CABC		0x55
#define DCS_READ_CABC		0x56
#define DCS_DEEP_STANDBY_IN		0xC1
#define DCS_GET_ID			0xf8
#define DSI_DT_DCS_SHORT_WRITE_0	0x05
#define DSI_DT_DCS_SHORT_WRITE_1	0x15
#define DSI_DT_DCS_READ			0x06
#define DSI_DT_SET_MAX_RET_PKG_SIZE	0x37
#define DSI_DT_NULL_PACKET		0x09
#define DSI_DT_DCS_LONG_WRITE		0x39

#define DSI_GEN_SHORTWRITE_NOPARAM 0x3
#define DSI_GEN_SHORTWRITE_1PARAM 0x13
#define DSI_GEN_SHORTWRITE_2PARAM 0x23
#define DSI_GEN_LONGWRITE 	  0x29

#define LONG_CMD_MIPI	0
#define SHORT_CMD_MIPI	1
#define END_OF_COMMAND	2
#define MIPI_DELAY_CMD  3

// S [mannsik.chung@lge.com] LCD off boot.
int lcd_off_boot=0;
EXPORT_SYMBOL(lcd_off_boot);

static int __init nolcd_setup(char *unused)
{
    lcd_off_boot = 1;
    return 1;
}
__setup("nolcd", nolcd_setup);
// E [mannsik.chung@lge.com] LCD off boot. 

/* S[, 20120922, mannsik.chung@lge.com, PM from froyo. */
#if defined(CONFIG_PRODUCT_LGE_LU6800)
static int lcd_boot_status=1;
extern u32 doing_wakeup;
u32 te_cpu_idle_block=0;
EXPORT_SYMBOL(te_cpu_idle_block);
#endif
/* E], 20120922, mannsik.chung@lge.com, PM from froyo. */

#ifdef CONFIG_OMAP2_DSS_HDMI // goochang.jeong@ge.com switch to lcd
extern bool HDMI_finalizing;
extern int nDisabledGFX;
int hdmitolcd_count = 0;
extern void lm3528_set_backlight_for_hdmi(int onoff);
#endif
extern int dsi_vc_write(struct omap_dss_device *dssdev, int channel, u8 cmd, u8 *data, int len);
static irqreturn_t justin_te_isr(int irq, void *data);
static void justin_te_timeout_work_callback(struct work_struct *work);
static int _justin_enable_te(struct omap_dss_device *dssdev, bool enable);

static int justin_panel_reset(struct omap_dss_device *dssdev);

// LGE_CHANGE [daewung.kim@lge.com] LCD panel sync problem WA
static int is_update = 0;

/**
 * struct panel_config - panel configuration
 * @name: panel name
 * @type: panel type
 * @timings: panel resolution
 * @sleep: various panel specific delays, passed to msleep() if non-zero
 * @reset_sequence: reset sequence timings, passed to udelay() if non-zero
 * @regulators: array of panel regulators
 * @num_regulators: number of regulators in the array
 */
struct panel_config {
	const char *name;
	int type;

	struct omap_video_timings timings;

	struct {
		unsigned int sleep_in;
		unsigned int sleep_out;
		unsigned int hw_reset;
		unsigned int enable_te;
	} sleep;

	struct {
		unsigned int high;
		unsigned int low;
	} reset_sequence;

	struct panel_regulator *regulators;
	int num_regulators;
};

enum {
	PANEL_JUSTIN,
};

static struct panel_config panel_configs[] = {
	{
		.name		= "justin_panel",
		.type		= PANEL_JUSTIN,
		.timings	= {
			.x_res		= 480,
			.y_res		= 800,
			.pixel_clock      =   23800,  //28800,  // add pixel clock goochang test  //23800,
			.hsw = 51,
			.hfp = 14,
			.hbp = 19,
			.vsw = 8,
			.vfp = 0,
			.vbp = 22,
		},
		.sleep		= {
			.sleep_in	= 10,
			.sleep_out	= 5,
			.hw_reset	= 10,
			.enable_te	= 50, /* possible panel bug */
		},
		.reset_sequence	= {
			.high		= 10000,
			.low		= 10000,
		},
	},
};

struct justin_data {
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
	int channel;

	struct delayed_work te_timeout_work;

	bool use_dsi_bl;

	bool cabc_broken;
	unsigned cabc_mode;

	bool intro_printed;

	struct workqueue_struct *workqueue;

	struct delayed_work esd_work;
	unsigned esd_interval;

	bool ulps_enabled;
	unsigned ulps_timeout;
	struct delayed_work ulps_work;

	struct panel_config *panel_config;
};

u8 lcd_command_for_mipi[][30] = {

#if 1 //KS_DEBUG
	{SHORT_CMD_MIPI,DSI_GEN_SHORTWRITE_2PARAM,0x02,0x03,0x00},												/* MIPI DSI config */
#endif

	{SHORT_CMD_MIPI,DSI_GEN_SHORTWRITE_1PARAM,0x01,0x20,},												/* Display Inversion */
	{SHORT_CMD_MIPI,DSI_GEN_SHORTWRITE_1PARAM,0x01,0x35,},		/* tearing effect  */
	{LONG_CMD_MIPI, DSI_GEN_LONGWRITE,0x03,0xB2, 0x00, 0xC8,},											/* Panel Characteristics Setting */
//	{MIPI_DELAY_CMD,5,}, //delay ms
	{SHORT_CMD_MIPI,DSI_GEN_SHORTWRITE_2PARAM, 0x02, 0xB3, 0x00,},									/* Panel Drive Setting */
	{SHORT_CMD_MIPI,DSI_GEN_SHORTWRITE_2PARAM, 0x02, 0xB4, 0x04,},									/* Display Mode Control */
	{LONG_CMD_MIPI, DSI_GEN_LONGWRITE,0x06, 0xB5, 0x42, 0x10, 0x10, 0x00, 0x20 ,},						/* Display Control 1 */
#if 0	
	{LONG_CMD_MIPI, DSI_GEN_LONGWRITE,0x07, 0xB6, 0x0B, 0x0F, 0x03C, 0x18, 0x18, 0xE8,}, 				/* Display Control 2 */
#else	//HW Tuning TEST : natting 2010.1230
	{LONG_CMD_MIPI, DSI_GEN_LONGWRITE,0x07, 0xB6, 0x0B, 0x0F, 0x03C, 0x13, 0x13, 0xE8,}, 				/* Display Control 2 */
#endif	
	{LONG_CMD_MIPI, DSI_GEN_LONGWRITE,0x06, 0xB7, /*0x59*/0x49, 0x06, 0x1F, 0x00, 0x00 ,},	//jkr test, Send BTA
//	{MIPI_DELAY_CMD,5,}, //delay ms
	{LONG_CMD_MIPI, DSI_GEN_LONGWRITE,0x03, 0xC0, 0x01, 0x11,}, 					/* oscilator setting */
	{LONG_CMD_MIPI, DSI_GEN_LONGWRITE,0x06, 0xC3, 0x07, 0x03, 0x04, 0x04, 0x04 ,},						/* Power Control 3 */
//	{MIPI_DELAY_CMD,100,}, //delay ms
#if 0
	{LONG_CMD_MIPI, DSI_GEN_LONGWRITE,0x07, 0xC4, 0x12, 0x24, 0x18, 0x18, 0x00, 0x49,}, 				/* Power Control 4 */
#else	//HW Tuning TEST : natting 2010.1230
  {LONG_CMD_MIPI, DSI_GEN_LONGWRITE,0x07, 0xC4, 0x12, 0x24, 0x18, 0x18, 0x05 /* 0x04 -> 0x05 */, 0x49,}, 				/* Power Control 4 */
#endif
//	{MIPI_DELAY_CMD,100,}, //delay ms
	{SHORT_CMD_MIPI,DSI_GEN_SHORTWRITE_2PARAM, 0x02, 0xC5, 0x70 /* 0x63 -> 0x6B -> 0x70 */,},									/* Power Control 5 */
	{MIPI_DELAY_CMD,100,}, //delay ms
	{LONG_CMD_MIPI, DSI_GEN_LONGWRITE,0x03,0xC6, 0x41, 0x63,},									/* Power Control 6 */
//	{MIPI_DELAY_CMD,100,}, //delay ms
#if 0
  {LONG_CMD_MIPI, DSI_GEN_LONGWRITE,0x0A,0xD0, 0x03, 0x07, 0x73, 0x35, 0x00, 0x01, 0x20, 0x00, 0x03,}, /* Positive Gamma Curve for Red */
  {LONG_CMD_MIPI, DSI_GEN_LONGWRITE,0x0A,0xD1, 0x03, 0x07, 0x73, 0x35, 0x00, 0x01, 0x20, 0x00, 0x03,}, /* Negative Gamma Curve for Red */
  {LONG_CMD_MIPI, DSI_GEN_LONGWRITE,0x0A,0xD2, 0x03, 0x07, 0x73, 0x35, 0x00, 0x01, 0x20, 0x00, 0x03,}, /* Positive Gamma Curve for Green */
  {LONG_CMD_MIPI, DSI_GEN_LONGWRITE,0x0A,0xD3, 0x03, 0x07, 0x73, 0x35, 0x00, 0x01, 0x20, 0x00, 0x03,}, /* Negative Gamma Curve for Green */
  {LONG_CMD_MIPI, DSI_GEN_LONGWRITE,0x0A,0xD4, 0x03, 0x07, 0x73, 0x35, 0x00, 0x01, 0x20, 0x00, 0x03,}, /* Positive Gamma Curve for Blue */
  {LONG_CMD_MIPI, DSI_GEN_LONGWRITE,0x0A,0xD5, 0x03, 0x07, 0x73, 0x35, 0x00, 0x01, 0x20, 0x00, 0x03,}, /* Negative Gamma Curve for Blue */
#else	//HW Tuning TEST : natting 2010.1230
	{LONG_CMD_MIPI, DSI_GEN_LONGWRITE,0x0A,0xD0, 0x00, 0x16, 0x62, 0x35, 0x02, 0x00, 0x30, 0x00, 0x03,}, /* Positive Gamma Curve for Red */
	{LONG_CMD_MIPI, DSI_GEN_LONGWRITE,0x0A,0xD1, 0x00, 0x16, 0x62, 0x35, 0x02, 0x00, 0x30, 0x00, 0x03,}, /* Negative Gamma Curve for Red */
	{LONG_CMD_MIPI, DSI_GEN_LONGWRITE,0x0A,0xD2, 0x00, 0x16, 0x62, 0x35, 0x02, 0x00, 0x30, 0x00, 0x03,}, /* Positive Gamma Curve for Green */
	{LONG_CMD_MIPI, DSI_GEN_LONGWRITE,0x0A,0xD3, 0x00, 0x16, 0x62, 0x35, 0x02, 0x00, 0x30, 0x00, 0x03,}, /* Negative Gamma Curve for Green */
	{LONG_CMD_MIPI, DSI_GEN_LONGWRITE,0x0A,0xD4, 0x00, 0x16, 0x62, 0x35, 0x02, 0x00, 0x30, 0x00, 0x03,}, /* Positive Gamma Curve for Blue */
	{LONG_CMD_MIPI, DSI_GEN_LONGWRITE,0x0A,0xD5, 0x00, 0x16, 0x62, 0x35, 0x02, 0x00, 0x30, 0x00, 0x03,}, /* Negative Gamma Curve for Blue */
#endif	
	{END_OF_COMMAND, },
};

static inline struct lge_dsi_panel_data
*get_panel_data(const struct omap_dss_device *dssdev)
{
	return (struct lge_dsi_panel_data *) dssdev->data;
}

static void justin_esd_work(struct work_struct *work);
static void justin_ulps_work(struct work_struct *work);

void dsi_phy_config_4_bta(struct omap_dss_device *dssdev);
#if 0//NATTING_TEST : i dun need u
static void hw_guard_start(struct justin_data *td, int guard_msec)
{
	td->hw_guard_wait = msecs_to_jiffies(guard_msec);
	td->hw_guard_end = jiffies + td->hw_guard_wait;
}

static void hw_guard_wait(struct justin_data *td)
{
	unsigned long wait = td->hw_guard_end - jiffies;

	if ((long)wait > 0 && wait <= td->hw_guard_wait) {
		set_current_state(TASK_UNINTERRUPTIBLE);
		schedule_timeout(wait);
	}
}
#endif

static int justin_dcs_read_1(struct justin_data *td, u8 dcs_cmd, u8 *data)
{
	int r;
	u8 buf[1];

	// S, [mannsik.chung@lge.com] LCD off boot. 
	if (lcd_off_boot == 1)
		return 0;
	// E, [mannsik.chung@lge.com] LCD off boot.

	r = dsi_vc_dcs_read(td->dssdev, td->channel, dcs_cmd, buf, 1);

	if (r < 0)
		return r;

	*data = buf[0];

	return 0;
}

static int justin_dcs_write_0(struct justin_data *td, u8 dcs_cmd)
{
	// S, [mannsik.chung@lge.com] LCD off boot. 
	if (lcd_off_boot == 1)
		return 0;
	// E, [mannsik.chung@lge.com] LCD off boot.

	return dsi_vc_dcs_write(td->dssdev, td->channel, &dcs_cmd, 1);
}

static int justin_dcs_write_1(struct justin_data *td, u8 dcs_cmd, u8 param)
{
	u8 buf[2];
	buf[0] = dcs_cmd;
	buf[1] = param;

	// S, [mannsik.chung@lge.com] LCD off boot. 
	if (lcd_off_boot == 1)
		return 0;
	// E, [mannsik.chung@lge.com] LCD off boot.

	return dsi_vc_dcs_write(td->dssdev, td->channel, buf, 2);
}

static int justin_sleep_in(struct justin_data *td)

{
	u8 cmd;
	int r;

#if 0//NATTING_TEST : i dun need u
	hw_guard_wait(td);
#endif

	cmd = DCS_SLEEP_IN;
	r = dsi_vc_dcs_write_nosync(td->dssdev, td->channel, &cmd, 1);
	if (r)
		return r;
#if 0  // sangki.hyun@lge.com lcd on/off ctl
	cmd = DCS_DEEP_STANDBY_IN;
	r = dsi_vc_dcs_write_nosync(td->dssdev, td->channel, &cmd, 1);
	if (r)
		return r;
#endif

#if 0//NATTING_TEST : i dun need u
	hw_guard_start(td, 120);
#endif

	if (td->panel_config->sleep.sleep_in)
		msleep(td->panel_config->sleep.sleep_in);

	return 0;
}

static int justin_sleep_out(struct justin_data *td)
{
	int r;
	
#if 0//NATTING_TEST : i dun need u
	hw_guard_wait(td);
#endif

	r = justin_dcs_write_0(td, DCS_SLEEP_OUT);
	if (r)
		return r;

#if 0//NATTING_TEST : we dun need u
	hw_guard_start(td, 120);
#endif

	if (td->panel_config->sleep.sleep_out)
		msleep(td->panel_config->sleep.sleep_out);

	return 0;
}

static int justin_set_addr_mode(struct justin_data *td, u8 rotate, bool mirror)
{
	int r;
	u8 mode;
	int b5, b6, b7;

	r = justin_dcs_read_1(td, DCS_READ_MADCTL, &mode);
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

	return justin_dcs_write_1(td, DCS_MEM_ACC_CTRL, mode);
}


static int justin_set_update_window(struct justin_data *td,
		u16 x, u16 y, u16 w, u16 h)
{
	int r;
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

	r = dsi_vc_dcs_write_nosync(td->dssdev, td->channel, buf, sizeof(buf));
	if (r)
		return r;

	buf[0] = DCS_PAGE_ADDR;
	buf[1] = (y1 >> 8) & 0xff;
	buf[2] = (y1 >> 0) & 0xff;
	buf[3] = (y2 >> 8) & 0xff;
	buf[4] = (y2 >> 0) & 0xff;

	r = dsi_vc_dcs_write_nosync(td->dssdev, td->channel, buf, sizeof(buf));
	if (r)
		return r;
	
#if	0  // sangki.hyun@lge.com
	dsi_vc_send_bta_sync(td->dssdev, td->channel);
#endif
	return r;
}

static void justin_queue_esd_work(struct omap_dss_device *dssdev)
{
	struct justin_data *td = dev_get_drvdata(&dssdev->dev);

	if (td->esd_interval > 0)
		queue_delayed_work(td->workqueue, &td->esd_work,
				msecs_to_jiffies(td->esd_interval));
}

static void justin_cancel_esd_work(struct omap_dss_device *dssdev)
{
	struct justin_data *td = dev_get_drvdata(&dssdev->dev);

	if (td->esd_interval > 0)
		cancel_delayed_work(&td->esd_work);
}

static void justin_queue_ulps_work(struct omap_dss_device *dssdev)
{
	struct justin_data *td = dev_get_drvdata(&dssdev->dev);

	if (td->ulps_timeout > 0)
		queue_delayed_work(td->workqueue, &td->ulps_work,
				msecs_to_jiffies(td->ulps_timeout));
}

static void justin_cancel_ulps_work(struct omap_dss_device *dssdev)
{
	struct justin_data *td = dev_get_drvdata(&dssdev->dev);

	if (td->ulps_timeout > 0)
		cancel_delayed_work(&td->ulps_work);
}

static int justin_enter_ulps(struct omap_dss_device *dssdev)
{
	struct justin_data *td = dev_get_drvdata(&dssdev->dev);
	struct lge_dsi_panel_data *panel_data = get_panel_data(dssdev);
	int r;

	if (td->ulps_enabled)
		return 0;

	justin_cancel_ulps_work(dssdev);

	r = _justin_enable_te(dssdev, false);
	if (r)
		goto err;

	disable_irq(gpio_to_irq(panel_data->ext_te_gpio));

	omapdss_dsi_display_disable(dssdev, false, true);

	td->ulps_enabled = true;

	return 0;

err:
	dev_err(&dssdev->dev, "enter ULPS failed");
	justin_panel_reset(dssdev);

	td->ulps_enabled = false;

	justin_queue_ulps_work(dssdev);

	return r;
}

static int justin_exit_ulps(struct omap_dss_device *dssdev)
{
	struct justin_data *td = dev_get_drvdata(&dssdev->dev);
	struct lge_dsi_panel_data *panel_data = get_panel_data(dssdev);
	int r;

	if (!td->ulps_enabled)
		return 0;

	r = omapdss_dsi_display_enable(dssdev);
	if (r) {
		dev_err(&dssdev->dev, "failed to enable DSI\n");
		goto err1;
	}

	omapdss_dsi_vc_enable_hs(dssdev, td->channel, true);

	r = _justin_enable_te(dssdev, true);
	if (r) {
		dev_err(&dssdev->dev, "failed to re-enable TE");
		goto err2;
	}

	enable_irq(gpio_to_irq(panel_data->ext_te_gpio));

	justin_queue_ulps_work(dssdev);

	td->ulps_enabled = false;

	return 0;

err2:
	dev_err(&dssdev->dev, "failed to exit ULPS");

	r = justin_panel_reset(dssdev);
	if (!r) {
		enable_irq(gpio_to_irq(panel_data->ext_te_gpio));
		td->ulps_enabled = false;
	}
err1:
	justin_queue_ulps_work(dssdev);

	return r;
}

static int justin_wake_up(struct omap_dss_device *dssdev)
{
	struct justin_data *td = dev_get_drvdata(&dssdev->dev);

	if (td->ulps_enabled)
		return justin_exit_ulps(dssdev);

	justin_cancel_ulps_work(dssdev);
	justin_queue_ulps_work(dssdev);
	return 0;
}

static int justin_bl_update_status(struct backlight_device *dev)
{
	struct omap_dss_device *dssdev = dev_get_drvdata(&dev->dev);
	struct justin_data *td = dev_get_drvdata(&dssdev->dev);
	struct lge_dsi_panel_data *panel_data = get_panel_data(dssdev);
	int r;
	int level;

	if (dev->props.fb_blank == FB_BLANK_UNBLANK &&
			dev->props.power == FB_BLANK_UNBLANK)
		level = dev->props.brightness;
	else
		level = 0;

	dev_dbg(&dssdev->dev, "update brightness to %d\n", level);

	mutex_lock(&td->lock);

	if (td->use_dsi_bl) {
		if (td->enabled) {
			dsi_bus_lock(dssdev);

			r = justin_wake_up(dssdev);
			if (!r)
				r = justin_dcs_write_1(td, DCS_BRIGHTNESS, level);

			dsi_bus_unlock(dssdev);
		} else {
			r = 0;
		}
	} else {
		if (!panel_data->set_backlight)
			r = -EINVAL;
		else
			r = panel_data->set_backlight(dssdev, level);
	}

	mutex_unlock(&td->lock);

	return r;
}

static int justin_bl_get_intensity(struct backlight_device *dev)
{
	if (dev->props.fb_blank == FB_BLANK_UNBLANK &&
			dev->props.power == FB_BLANK_UNBLANK)
		return dev->props.brightness;

	return 0;
}

static const struct backlight_ops justin_bl_ops = {
	.get_brightness = justin_bl_get_intensity,
	.update_status  = justin_bl_update_status,
};

static void justin_get_timings(struct omap_dss_device *dssdev,
			struct omap_video_timings *timings)
{
	*timings = dssdev->panel.timings;
}

static void justin_get_resolution(struct omap_dss_device *dssdev,
		u16 *xres, u16 *yres)
{
	struct justin_data *td = dev_get_drvdata(&dssdev->dev);

	if (td->rotate == 0 || td->rotate == 2) {
		*xres = dssdev->panel.timings.x_res;
		*yres = dssdev->panel.timings.y_res;
	} else {
		*yres = dssdev->panel.timings.x_res;
		*xres = dssdev->panel.timings.y_res;
	}
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
	struct justin_data *td = dev_get_drvdata(&dssdev->dev);
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
	struct justin_data *td = dev_get_drvdata(&dssdev->dev);
	int i;
	int r;

	for (i = 0; i < ARRAY_SIZE(cabc_modes); i++) {
		if (sysfs_streq(cabc_modes[i], buf))
			break;
	}

	if (i == ARRAY_SIZE(cabc_modes))
		return -EINVAL;

	mutex_lock(&td->lock);

	if (td->enabled) {
		dsi_bus_lock(dssdev);

		if (!td->cabc_broken) {
			r = justin_wake_up(dssdev);
			if (r)
				goto err;

			r = justin_dcs_write_1(td, DCS_WRITE_CABC, i);
			if (r)
				goto err;
		}

		dsi_bus_unlock(dssdev);
	}

	td->cabc_mode = i;

	mutex_unlock(&td->lock);

	return count;
err:
	dsi_bus_unlock(dssdev);
	mutex_unlock(&td->lock);
	return r;
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

static ssize_t justin_store_esd_interval(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{
	struct omap_dss_device *dssdev = to_dss_device(dev);
	struct justin_data *td = dev_get_drvdata(&dssdev->dev);

	unsigned long t;
	int r;

	r = strict_strtoul(buf, 10, &t);
	if (r)
		return r;

	mutex_lock(&td->lock);
	justin_cancel_esd_work(dssdev);
	td->esd_interval = t;
	if (td->enabled)
		justin_queue_esd_work(dssdev);
	mutex_unlock(&td->lock);

	return count;
}

static ssize_t justin_show_esd_interval(struct device *dev,
		struct device_attribute *attr,
		char *buf)
{
	struct omap_dss_device *dssdev = to_dss_device(dev);
	struct justin_data *td = dev_get_drvdata(&dssdev->dev);
	unsigned t;

	mutex_lock(&td->lock);
	t = td->esd_interval;
	mutex_unlock(&td->lock);

	return snprintf(buf, PAGE_SIZE, "%u\n", t);
}

static ssize_t justin_store_ulps(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{
	struct omap_dss_device *dssdev = to_dss_device(dev);
	struct justin_data *td = dev_get_drvdata(&dssdev->dev);
	unsigned long t;
	int r;

	r = strict_strtoul(buf, 10, &t);
	if (r)
		return r;

	mutex_lock(&td->lock);

	if (td->enabled) {
		dsi_bus_lock(dssdev);

		if (t)
			r = justin_enter_ulps(dssdev);
		else
			r = justin_wake_up(dssdev);

		dsi_bus_unlock(dssdev);
	}

	mutex_unlock(&td->lock);

	if (r)
		return r;

	return count;
}

static ssize_t justin_show_ulps(struct device *dev,
		struct device_attribute *attr,
		char *buf)
{
	struct omap_dss_device *dssdev = to_dss_device(dev);
	struct justin_data *td = dev_get_drvdata(&dssdev->dev);
	unsigned t;

	mutex_lock(&td->lock);
	t = td->ulps_enabled;
	mutex_unlock(&td->lock);

	return snprintf(buf, PAGE_SIZE, "%u\n", t);
}

static ssize_t justin_store_ulps_timeout(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{
	struct omap_dss_device *dssdev = to_dss_device(dev);
	struct justin_data *td = dev_get_drvdata(&dssdev->dev);
	unsigned long t;
	int r;

	r = strict_strtoul(buf, 10, &t);
	if (r)
		return r;

	mutex_lock(&td->lock);
	td->ulps_timeout = t;

	if (td->enabled) {
		/* justin_wake_up will restart the timer */
		dsi_bus_lock(dssdev);
		r = justin_wake_up(dssdev);
		dsi_bus_unlock(dssdev);
	}

	mutex_unlock(&td->lock);

	if (r)
		return r;

	return count;
}

static ssize_t justin_show_ulps_timeout(struct device *dev,
		struct device_attribute *attr,
		char *buf)
{
	struct omap_dss_device *dssdev = to_dss_device(dev);
	struct justin_data *td = dev_get_drvdata(&dssdev->dev);
	unsigned t;

	mutex_lock(&td->lock);
	t = td->ulps_timeout;
	mutex_unlock(&td->lock);

	return snprintf(buf, PAGE_SIZE, "%u\n", t);
}

static DEVICE_ATTR(cabc_mode, S_IRUGO | S_IWUSR,
		show_cabc_mode, store_cabc_mode);
static DEVICE_ATTR(cabc_available_modes, S_IRUGO,
		show_cabc_available_modes, NULL);
static DEVICE_ATTR(esd_interval, S_IRUGO | S_IWUSR,
		justin_show_esd_interval, justin_store_esd_interval);
static DEVICE_ATTR(ulps, S_IRUGO | S_IWUSR,
		justin_show_ulps, justin_store_ulps);
static DEVICE_ATTR(ulps_timeout, S_IRUGO | S_IWUSR,
		justin_show_ulps_timeout, justin_store_ulps_timeout);

static struct attribute *justin_attrs[] = {
	&dev_attr_cabc_mode.attr,
	&dev_attr_cabc_available_modes.attr,
	&dev_attr_esd_interval.attr,
	&dev_attr_ulps.attr,
	&dev_attr_ulps_timeout.attr,
	NULL,
};

static struct attribute_group justin_attr_group = {
	.attrs = justin_attrs,
};

extern int lcd_backlight_status;
extern int lcd_status_backup;
extern  int lm3258_power_switch(int val);
static void justin_hw_reset(struct omap_dss_device *dssdev)
{
	struct justin_data *td = dev_get_drvdata(&dssdev->dev);
	struct lge_dsi_panel_data *panel_data = get_panel_data(dssdev);

	if (panel_data->reset_gpio == -1)
		return;

	gpio_request(panel_data->reset_gpio, "lcd_reset");
	gpio_direction_output(panel_data->reset_gpio, 1);

	gpio_set_value(panel_data->reset_gpio, 1);
	if (td->panel_config->reset_sequence.high)
		udelay(td->panel_config->reset_sequence.high);
	/* reset the panel */
	gpio_set_value(panel_data->reset_gpio, 0);
	/* assert reset */
	if (td->panel_config->reset_sequence.low)
		udelay(td->panel_config->reset_sequence.low);
	gpio_set_value(panel_data->reset_gpio, 1);
	/* wait after releasing reset */
	if (td->panel_config->sleep.hw_reset)
		msleep(td->panel_config->sleep.hw_reset);
}

static int justin_probe(struct omap_dss_device *dssdev)
{
	struct backlight_properties props;
	struct justin_data *td;
	struct backlight_device *bldev;
	struct lge_dsi_panel_data *panel_data = get_panel_data(dssdev);
	struct panel_config *panel_config = NULL;
	int r, i;

	dev_dbg(&dssdev->dev, "probe\n");

	if (!panel_data || !panel_data->name) {
		r = -EINVAL;
		goto err;
	}

	for (i = 0; i < ARRAY_SIZE(panel_configs); i++) {
		if (strcmp(panel_data->name, panel_configs[i].name) == 0) {
			panel_config = &panel_configs[i];
			break;
		}
	}

	if (!panel_config) {
		r = -EINVAL;
		goto err;
	}

	dssdev->panel.config = OMAP_DSS_LCD_TFT | OMAP_DSS_LCD_RF | OMAP_DSS_LCD_ONOFF;

	dssdev->panel.timings = panel_config->timings;
	dssdev->ctrl.pixel_size = 24;

	/* Since some android application use physical dimension, that information should be set here */
	dssdev->panel.width_in_um = 52000; /* physical dimension in um */
	dssdev->panel.height_in_um = 86000; /* physical dimension in um */ 
	td = kzalloc(sizeof(*td), GFP_KERNEL);
	if (!td) {
		r = -ENOMEM;
		goto err;
	}
	td->dssdev = dssdev;
	td->panel_config = panel_config;
	td->esd_interval = panel_data->esd_interval;
	td->ulps_enabled = false;
	td->ulps_timeout = panel_data->ulps_timeout;

	mutex_init(&td->lock);

	atomic_set(&td->do_update, 0);

	td->workqueue = create_singlethread_workqueue("justin_esd");
	if (td->workqueue == NULL) {
		dev_err(&dssdev->dev, "can't create ESD workqueue\n");
		r = -ENOMEM;
		goto err_reg;
	}
	INIT_DELAYED_WORK_DEFERRABLE(&td->esd_work, justin_esd_work);
	INIT_DELAYED_WORK(&td->ulps_work, justin_ulps_work);

	dev_set_drvdata(&dssdev->dev, td);

	//justin_hw_reset(dssdev);

	/* if no platform set_backlight() defined, presume DSI backlight
	 * control */
	memset(&props, 0, sizeof(struct backlight_properties));

	/* P940 dose not use dsi blacklight control */
	td->use_dsi_bl = false; 

	//if (!panel_data->set_backlight)
	//	td->use_dsi_bl = true;

	if (td->use_dsi_bl)
		props.max_brightness = 255;
	else
		props.max_brightness = 127;

	props.type = BACKLIGHT_RAW;
	bldev = backlight_device_register(dev_name(&dssdev->dev), &dssdev->dev,
					dssdev, &justin_bl_ops, &props);
	if (IS_ERR(bldev)) {
		r = PTR_ERR(bldev);
		goto err_bl;
	}

	td->bldev = bldev;

	bldev->props.fb_blank = FB_BLANK_UNBLANK;
	bldev->props.power = FB_BLANK_UNBLANK;
	if (td->use_dsi_bl)
		bldev->props.brightness = 255;
	else
		bldev->props.brightness = 127;

	justin_bl_update_status(bldev);

	if (panel_data->use_ext_te) {
		int gpio = panel_data->ext_te_gpio;

		r = gpio_request(gpio, "justin irq");
		if (r) {
			dev_err(&dssdev->dev, "GPIO request failed\n");
			goto err_gpio;
		}

		gpio_direction_input(gpio);

		r = request_irq(gpio_to_irq(gpio), justin_te_isr,
				IRQF_DISABLED | IRQF_TRIGGER_RISING,
				"justin vsync", dssdev);

		if (r) {
			dev_err(&dssdev->dev, "IRQ request failed\n");
			gpio_free(gpio);
			goto err_irq;
		}

		INIT_DELAYED_WORK_DEFERRABLE(&td->te_timeout_work,
					justin_te_timeout_work_callback);

		dev_dbg(&dssdev->dev, "Using GPIO TE\n");
	}
	r = omap_dsi_request_vc(dssdev, &td->channel);
	if (r) {
		dev_err(&dssdev->dev, "failed to get virtual channel\n");
		goto err_req_vc;
	}

	r = omap_dsi_set_vc_id(dssdev, td->channel, TCH);
	if (r) {
		dev_err(&dssdev->dev, "failed to set VC_ID\n");
		goto err_vc_id;
	}
	r = sysfs_create_group(&dssdev->dev.kobj, &justin_attr_group);
	if (r) {
		dev_err(&dssdev->dev, "failed to create sysfs files\n");
		goto err_vc_id;
	}
	
#ifdef CONFIG_FB_OMAP_BOOTLOADER_INIT
		if (&dssdev->channel == OMAP_DSS_CHANNEL_LCD) {
			td->enabled = 1;
		}
#endif

	return 0;

err_vc_id:
	omap_dsi_release_vc(dssdev, td->channel);
err_req_vc:
	if (panel_data->use_ext_te)
		free_irq(gpio_to_irq(panel_data->ext_te_gpio), dssdev);
err_irq:
	if (panel_data->use_ext_te)
		gpio_free(panel_data->ext_te_gpio);
err_gpio:
	backlight_device_unregister(bldev);
err_bl:
	destroy_workqueue(td->workqueue);
err_reg:
	kfree(td);
err:
	return r;
}

static void __exit justin_remove(struct omap_dss_device *dssdev)
{
	struct justin_data *td = dev_get_drvdata(&dssdev->dev);
	struct lge_dsi_panel_data *panel_data = get_panel_data(dssdev);
	struct backlight_device *bldev;

	dev_dbg(&dssdev->dev, "remove\n");

	sysfs_remove_group(&dssdev->dev.kobj, &justin_attr_group);
	omap_dsi_release_vc(dssdev, td->channel);

	if (panel_data->use_ext_te) {
		int gpio = panel_data->ext_te_gpio;
		free_irq(gpio_to_irq(gpio), dssdev);
		gpio_free(gpio);
	}

	bldev = td->bldev;
	bldev->props.power = FB_BLANK_POWERDOWN;
	justin_bl_update_status(bldev);
	backlight_device_unregister(bldev);

	justin_cancel_ulps_work(dssdev);
	justin_cancel_esd_work(dssdev);
	destroy_workqueue(td->workqueue);

	/* reset, to be sure that the panel is in a valid state */
	justin_hw_reset(dssdev);

	kfree(td);
}

static int justin_power_on(struct omap_dss_device *dssdev)
{
	struct justin_data *td = dev_get_drvdata(&dssdev->dev);
	int i,r;

	/* At power on the first vsync has not been received yet */
        dssdev->first_vsync = false;

/* S[, 20120922, mannsik.chung@lge.com, PM from froyo. */
#if defined(CONFIG_PRODUCT_LGE_LU6800)
	if((doing_wakeup == 0)&&(lcd_boot_status==0))
	{
		doing_wakeup = 1;
	}
#endif
/* E], 20120922, mannsik.chung@lge.com, PM from froyo. */

	lm3258_power_switch(1);
	lcd_status_backup = 1;
	r = omapdss_dsi_display_enable(dssdev);
	if (r) {
		dev_err(&dssdev->dev, "failed to enable DSI\n");
		goto err0;
	}

#if 0  // sangki.hyun@lge.com
	if (dssdev->platform_enable) {
		r = dssdev->platform_enable(dssdev);		// justin_panel_enable_lcd()
		if (r)
			return r;
	}

	justin_hw_reset(dssdev);
#endif
	omapdss_dsi_vc_enable_hs(dssdev, td->channel, false);

	mdelay(5);	

/* S[, 20120922, mannsik.chung@lge.com, PM from froyo. */
#if defined(CONFIG_PRODUCT_LGE_LU6800)
	if(lcd_boot_status== 1)
	{
		lcd_boot_status = 0;
	}
#endif
/* E], 20120922, mannsik.chung@lge.com, PM from froyo. */



#if 0  // sangki.hyun@lge.com
	for (i = 0; lgd_lcd_command_for_mipi[i][0] != END_OF_COMMAND; i++) {
		dsi_vc_dcs_write(dssdev, td->channel, &lgd_lcd_command_for_mipi[i][3], lgd_lcd_command_for_mipi[i][2]);
	}
#else
	for (i = 0; lcd_command_for_mipi[i][0] != END_OF_COMMAND; i++) 
	{
		if(lcd_command_for_mipi[i][0]==MIPI_DELAY_CMD)
		{
//				mdelay(lcd_command_for_mipi[i][1]);
		}
		else
		{
#if 1 // KS_DEBUG
			if(i==0)
			{	
				dsi_vc_write(dssdev,td->channel,lcd_command_for_mipi[i][1],&lcd_command_for_mipi[i][3], lcd_command_for_mipi[i][2]);
			}
			else	
#endif
			{

				dsi_vc_dcs_write(dssdev, td->channel, &lcd_command_for_mipi[i][3], lcd_command_for_mipi[i][2]);
			}
		}
	}
 	dsi_phy_config_4_bta(dssdev);
#endif
	r = justin_sleep_out(td);
	if (r)
		goto err;

#if 0
	r = justin_dcs_write_1(td, DCS_BRIGHTNESS, 0xff);
	if (r)
		goto err;
	r = justin_dcs_write_1(td, DCS_CTRL_DISPLAY,
			(1<<2) | (1<<5));	/* BL | BCTRL */
	if (r)
		goto err;

	r = justin_dcs_write_1(td, DCS_PIXEL_FORMAT, 0x7); /* 24bit/pixel */
	if (r)
		goto err;

	r = justin_set_addr_mode(td, td->rotate, td->mirror);
	if (r)
		goto err;

	if (!td->cabc_broken) {
		r = justin_dcs_write_1(td, DCS_WRITE_CABC, td->cabc_mode);
		if (r)
			goto err;
	}
#endif
	r = justin_dcs_write_0(td, DCS_DISPLAY_ON);
	if (r)
		goto err;

	r = _justin_enable_te(dssdev, td->te_enabled);
	if (r)
		goto err;

	td->enabled = 1;
#if 0
	if (!td->intro_printed) {
		dev_info(&dssdev->dev, "%s panel revision %02x.%02x.%02x\n",
			td->panel_config->name, id1, id2, id3);
		if (td->cabc_broken)
			dev_info(&dssdev->dev,
					"old Taal version, CABC disabled\n");
		td->intro_printed = true;
	}
#endif
	omapdss_dsi_vc_enable_hs(dssdev, td->channel, true);

	return 0;
err:
	dev_err(&dssdev->dev, "error while enabling panel, issuing HW reset\n");

	justin_hw_reset(dssdev);

	omapdss_dsi_display_disable(dssdev, true, false);
err0:
	return r;
}

static void justin_power_off(struct omap_dss_device *dssdev)
{
	struct justin_data *td = dev_get_drvdata(&dssdev->dev);
	struct lge_dsi_panel_data *panel_data = get_panel_data(dssdev);
	int r;
#if 1 // sangki.hyun@lge.com
	if (!td->enabled)
		return;
#endif
   _justin_enable_te(dssdev, 0);
	r = justin_dcs_write_0(td, DCS_DISPLAY_OFF);
	if (!r) {
		msleep(35);  // sangki.hyun@lge.com
		r = justin_sleep_in(td);
		/* HACK: wait a bit so that the message goes through */
		msleep(10);
	}

	if (r) {
		dev_err(&dssdev->dev,
				"error disabling panel, issuing HW reset\n");
		justin_hw_reset(dssdev);
	}

#if 1  //2012.07.21 goochang.jeong@lge.com sleep current fix
	/* reset  the panel */
    // wooho.jeong@lge.com 2012.07.26
    // MOD : for LCD broken then Power Key.
    if( nDisabledGFX == 0 )
    {
    	if (panel_data->reset_gpio)
    		gpio_set_value(panel_data->reset_gpio, 0);

    	/* disable lcd ldo */
    	if (dssdev->platform_disable)
    		dssdev->platform_disable(dssdev);    
    }
#endif
	/* if we try to turn off dsi regulator (VCXIO), system will be halt  */
	/* So below funtion's sencod args should set as false  */
	omapdss_dsi_display_disable(dssdev, false, false);

	td->enabled = 0;
}

static int justin_panel_reset(struct omap_dss_device *dssdev)
{
	dev_err(&dssdev->dev, "performing LCD reset\n");

	justin_power_off(dssdev);
	justin_hw_reset(dssdev);
	return justin_power_on(dssdev);
}

static int justin_enable(struct omap_dss_device *dssdev)
{
	struct justin_data *td = dev_get_drvdata(&dssdev->dev);
	int r;

	dev_dbg(&dssdev->dev, "enable\n");
#ifdef CONFIG_OMAP2_DSS_HDMI // goochang.jeong@lge.com switch to lcd
    // shouldn't reset LCD while LCD ON(BL ON)
    if (HDMI_finalizing){
		printk("%s::HDMI is finalizing ::dun have to init LCD again..MUST NOT actually \n", __func__);
		lm3528_set_backlight_for_hdmi(0);
		gpio_set_value(34, 0);
		mdelay(20);
		gpio_set_value(34, 1);
		mdelay(20);
		hdmitolcd_count = 0;
    }
#endif

	mutex_lock(&td->lock);

	if (dssdev->state != OMAP_DSS_DISPLAY_DISABLED) {
		r = -EINVAL;
		goto err;
	}

	dsi_bus_lock(dssdev);

	r = justin_power_on(dssdev);

	dsi_bus_unlock(dssdev);

	if (r)
		goto err;

	justin_queue_esd_work(dssdev);

	dssdev->state = OMAP_DSS_DISPLAY_ACTIVE;

	mutex_unlock(&td->lock);

	return 0;
err:
	dev_dbg(&dssdev->dev, "enable failed\n");
	mutex_unlock(&td->lock);
	return r;
}

static void justin_disable(struct omap_dss_device *dssdev)
{
	struct justin_data *td = dev_get_drvdata(&dssdev->dev);
#ifdef CONFIG_OMAP2_DSS_HDMI  // goochang.jeong@lge.com LCD ON when video HDMI display
	if( nDisabledGFX==2 && HDMI_finalizing==0 ) 
	{
		dssdev->state = OMAP_DSS_DISPLAY_DISABLED;
		return;
	}
	else if( nDisabledGFX==2 && HDMI_finalizing==1 )
	{
		dssdev->state = OMAP_DSS_DISPLAY_ACTIVE;
	}
#endif
	dev_dbg(&dssdev->dev, "disable\n");

	mutex_lock(&td->lock);

	justin_cancel_ulps_work(dssdev);
	justin_cancel_esd_work(dssdev);

	dsi_bus_lock(dssdev);

	if (dssdev->state == OMAP_DSS_DISPLAY_ACTIVE) {
		int r;

		r = justin_wake_up(dssdev);
		if (!r)
			justin_power_off(dssdev);
	}

	dsi_bus_unlock(dssdev);

	dssdev->state = OMAP_DSS_DISPLAY_DISABLED;

	mutex_unlock(&td->lock);
}

static int justin_suspend(struct omap_dss_device *dssdev)
{
	struct justin_data *td = dev_get_drvdata(&dssdev->dev);
	int r;

	dev_dbg(&dssdev->dev, "suspend\n");

	mutex_lock(&td->lock);

	if (dssdev->state != OMAP_DSS_DISPLAY_ACTIVE) {
		r = -EINVAL;
		goto err;
	}

	justin_cancel_ulps_work(dssdev);
	justin_cancel_esd_work(dssdev);

	dsi_bus_lock(dssdev);

	r = justin_wake_up(dssdev);
	if (!r)
		justin_power_off(dssdev);

	dsi_bus_unlock(dssdev);

	dssdev->state = OMAP_DSS_DISPLAY_SUSPENDED;

	mutex_unlock(&td->lock);
	if(lcd_backlight_status == 0)
	{
		lm3258_power_switch(0);
	}
	lcd_status_backup = 0;
	return 0;
err:
	mutex_unlock(&td->lock);
	return r;
}

static int justin_resume(struct omap_dss_device *dssdev)
{
	struct justin_data *td = dev_get_drvdata(&dssdev->dev);
	int r;

	dev_dbg(&dssdev->dev, "resume\n");

	lm3258_power_switch(1);
	mutex_lock(&td->lock);

	if (dssdev->state != OMAP_DSS_DISPLAY_SUSPENDED) {
		r = -EINVAL;
		goto err;
	}

	dsi_bus_lock(dssdev);
	justin_hw_reset(dssdev); /* 20110813, mschung@ubiquix.com, LCD need to do hw_reset at resume. Some board's LCD does not wake-up without hw_reset. */	
	r = justin_power_on(dssdev);

	dsi_bus_unlock(dssdev);

	if (r) {
		dssdev->state = OMAP_DSS_DISPLAY_DISABLED;
	} else {
		dssdev->state = OMAP_DSS_DISPLAY_ACTIVE;
		justin_queue_esd_work(dssdev);
	}

	mutex_unlock(&td->lock);

	return r;
err:
	mutex_unlock(&td->lock);
	return r;
}

static void justin_framedone_cb(int err, void *data)
{
	struct omap_dss_device *dssdev = data;
	dev_dbg(&dssdev->dev, "framedone, err %d\n", err);

// LGE_CHANGE_S [daewung.kim@lge.com] LCD panel sync problem WA
//	dsi_bus_unlock(dssdev);		// original
	if (is_update)
	{
		is_update = 0;
		dsi_bus_unlock(dssdev);
	}
// LGE_CHANGE_E [daewung.kim@lge.com] LCD panel sync problem WA
}

static irqreturn_t justin_te_isr(int irq, void *data)
{
	struct omap_dss_device *dssdev = data;
	struct justin_data *td = dev_get_drvdata(&dssdev->dev);
	int old;
	int r;

	old = atomic_cmpxchg(&td->do_update, 1, 0);

	if (old) {
		cancel_delayed_work(&td->te_timeout_work);

/* S[, 20120922, mannsik.chung@lge.com, PM from froyo. */
#if defined(CONFIG_PRODUCT_LGE_LU6800)
		te_cpu_idle_block = 0;
#endif
/* E], 20120922, mannsik.chung@lge.com, PM from froyo. */

		r = omap_dsi_update(dssdev, td->channel,
				td->update_region.x,
				td->update_region.y,
				td->update_region.w,
				td->update_region.h,
				justin_framedone_cb, dssdev);
		if (r)
			goto err;
	}

	return IRQ_HANDLED;
err:
	dev_err(&dssdev->dev, "start update failed\n");

// LGE_CHANGE_S [daewung.kim@lge.com] LCD panel sync problem WA
//	dsi_bus_unlock(dssdev);		// original
	if (is_update)
	{
		is_update = 0;
		dsi_bus_unlock(dssdev);
	}
// LGE_CHANGE_E [daewung.kim@lge.com] LCD panel sync problem WA
	return IRQ_HANDLED;
}

static void justin_te_timeout_work_callback(struct work_struct *work)
{
	struct justin_data *td = container_of(work, struct justin_data,
					te_timeout_work.work);
	struct omap_dss_device *dssdev = td->dssdev;

	dev_err(&dssdev->dev, "TE not received for 250ms!\n");

	atomic_set(&td->do_update, 0);
// LGE_CHANGE_S [daewung.kim@lge.com] LCD panel sync problem WA
//	dsi_bus_unlock(dssdev);  // original
	if (is_update)
	{
		is_update = 0;
		dsi_bus_unlock(dssdev);
	}
// LGE_CHANGE_E [daewung.kim@lge.com] LCD panel sync problem WA
}

static int justin_update(struct omap_dss_device *dssdev,
				    u16 x, u16 y, u16 w, u16 h)
{
	struct justin_data *td = dev_get_drvdata(&dssdev->dev);
	struct lge_dsi_panel_data *panel_data = get_panel_data(dssdev);
	int r;


	// S, [mannsik.chung@lge.com] LCD off boot. 
	if (lcd_off_boot == 1)
		return 0;
	// E, [mannsik.chung@lge.com] LCD off boot.

	dev_dbg(&dssdev->dev, "update %d, %d, %d x %d\n", x, y, w, h);

	mutex_lock(&td->lock);
	dsi_bus_lock(dssdev);

#ifdef CONFIG_OMAP2_DSS_HDMI // goochang.jeong@lge.com switch to lcd
	if(HDMI_finalizing)
		hdmitolcd_count++;
#endif
	r = justin_wake_up(dssdev);
	if (r)
		goto err;

	if (!td->enabled) {
		r = 0;
		goto err;
	}

	r = omap_dsi_prepare_update(dssdev, &x, &y, &w, &h, true);
	if (r)
		goto err;

	r = justin_set_update_window(td, x, y, w, h);
	if (r)
		goto err;

// LGE_CHANGE [daewung.kim@lge.com] LCD panel sync problem WA
	is_update = 1;//+DEJA

	if (td->te_enabled && panel_data->use_ext_te) {

/* S[, 20120922, mannsik.chung@lge.com, PM from froyo. */
#if defined(CONFIG_PRODUCT_LGE_LU6800)
		te_cpu_idle_block = 1;
#endif
/* E], 20120922, mannsik.chung@lge.com, PM from froyo. */

		td->update_region.x = x;
		td->update_region.y = y;
		td->update_region.w = w;
		td->update_region.h = h;
		barrier();
		schedule_delayed_work(&td->te_timeout_work,
				msecs_to_jiffies(250));
#ifdef CONFIG_OMAP2_DSS_HDMI  // goochang.jeong@lge.com LCD ON when video HDMI display
        /* wooho.jeong@lge.com ADD : for overlay0 display & HDMI Disconnected problem fixed */
		if( nDisabledGFX == 3) // && HDMI_finalizing == 0 ) 
		{
			atomic_set(&td->do_update, 0);
		}
		else
		{
			atomic_set(&td->do_update, 1);
		}
#else
		atomic_set(&td->do_update, 1);
#endif
	} else {
		r = omap_dsi_update(dssdev, td->channel, x, y, w, h,
				justin_framedone_cb, dssdev);
		if (r)
			goto err;
	}

	/* note: no bus_unlock here. unlock is in framedone_cb */
	mutex_unlock(&td->lock);
#ifdef CONFIG_OMAP2_DSS_HDMI // goochang.jeong@lge.com switch to lcd
	if ((hdmitolcd_count >= 5) && HDMI_finalizing)
	{
		lm3528_set_backlight_for_hdmi(1);
		HDMI_finalizing = 0;
		hdmitolcd_count = 0;
	}
#endif
	return 0;
err:
// LGE_CHANGE [daewung.kim@lge.com] LCD panel sync problem WA
	is_update = 0;//+DEJA

	dsi_bus_unlock(dssdev);
	mutex_unlock(&td->lock);
#ifdef CONFIG_OMAP2_DSS_HDMI // goochang.jeong@lge.com switch to lcd
	HDMI_finalizing = 0;
#endif
	dev_err(&dssdev->dev, "update error \n");
	return r;
}

static int justin_sync(struct omap_dss_device *dssdev)
{
	struct justin_data *td = dev_get_drvdata(&dssdev->dev);

	dev_dbg(&dssdev->dev, "sync\n");
#if 0  // sangki.hyun@lge.com
	mutex_lock(&td->lock);
	dsi_bus_lock(dssdev);
	dsi_bus_unlock(dssdev);
	mutex_unlock(&td->lock);
#endif
	dev_dbg(&dssdev->dev, "sync done\n");

	return 0;
}

static int _justin_enable_te(struct omap_dss_device *dssdev, bool enable)
{
	struct justin_data *td = dev_get_drvdata(&dssdev->dev);
	struct lge_dsi_panel_data *panel_data = get_panel_data(dssdev);
	int r;

	if (enable)
	{
#define _LCD_CMD()	(tear_scanline[0])
#define _LCD_DAT()	(&tear_scanline[2])
#define _LCD_LEN()	(tear_scanline[1])
		u8 tear_scanline[] = {0x39,0x03,0x44,0x00,0x00,};
		r = dsi_vc_write(dssdev, TCH, _LCD_CMD(), _LCD_DAT(), _LCD_LEN());
		if(r)
			while(1);

		r = justin_dcs_write_1(td, DCS_TEAR_ON, 0);
	}
	else
		r = justin_dcs_write_0(td, DCS_TEAR_OFF);

	if (!panel_data->use_ext_te)
		omapdss_dsi_enable_te(dssdev, enable);

	if (td->panel_config->sleep.enable_te)
		msleep(td->panel_config->sleep.enable_te);

	return r;
}

static int justin_enable_te(struct omap_dss_device *dssdev, bool enable)
{
	struct justin_data *td = dev_get_drvdata(&dssdev->dev);
	int r;

	mutex_lock(&td->lock);

	if (td->te_enabled == enable)
		goto end;

	dsi_bus_lock(dssdev);

	if (td->enabled) {
		r = justin_wake_up(dssdev);
		if (r)
			goto err;

		r = _justin_enable_te(dssdev, enable);
		if (r)
			goto err;
	}

	td->te_enabled = enable;

	dsi_bus_unlock(dssdev);
end:
	mutex_unlock(&td->lock);

	return 0;
err:
	dsi_bus_unlock(dssdev);
	mutex_unlock(&td->lock);

	return r;
}

static int justin_get_te(struct omap_dss_device *dssdev)
{
	struct justin_data *td = dev_get_drvdata(&dssdev->dev);
	int r;

	mutex_lock(&td->lock);
	r = td->te_enabled;
	mutex_unlock(&td->lock);

	return r;
}

static int justin_rotate(struct omap_dss_device *dssdev, u8 rotate)
{
	struct justin_data *td = dev_get_drvdata(&dssdev->dev);
	int r;

	dev_dbg(&dssdev->dev, "rotate %d\n", rotate);

	mutex_lock(&td->lock);

	if (td->rotate == rotate)
		goto end;

	dsi_bus_lock(dssdev);

	if (td->enabled) {
		r = justin_wake_up(dssdev);
		if (r)
			goto err;

		r = justin_set_addr_mode(td, rotate, td->mirror);
		if (r)
			goto err;
	}

	td->rotate = rotate;

	dsi_bus_unlock(dssdev);
end:
	mutex_unlock(&td->lock);
	return 0;
err:
	dsi_bus_unlock(dssdev);
	mutex_unlock(&td->lock);
	return r;
}

static u8 justin_get_rotate(struct omap_dss_device *dssdev)
{
	struct justin_data *td = dev_get_drvdata(&dssdev->dev);
	int r;

	mutex_lock(&td->lock);
	r = td->rotate;
	mutex_unlock(&td->lock);

	return r;
}

static int justin_mirror(struct omap_dss_device *dssdev, bool enable)
{
	struct justin_data *td = dev_get_drvdata(&dssdev->dev);
	int r;

	dev_dbg(&dssdev->dev, "mirror %d\n", enable);

	mutex_lock(&td->lock);

	if (td->mirror == enable)
		goto end;

	dsi_bus_lock(dssdev);
	if (td->enabled) {
		r = justin_wake_up(dssdev);
		if (r)
			goto err;

		r = justin_set_addr_mode(td, td->rotate, enable);
		if (r)
			goto err;
	}

	td->mirror = enable;

	dsi_bus_unlock(dssdev);
end:
	mutex_unlock(&td->lock);
	return 0;
err:
	dsi_bus_unlock(dssdev);
	mutex_unlock(&td->lock);
	return r;
}

static bool justin_get_mirror(struct omap_dss_device *dssdev)
{
	struct justin_data *td = dev_get_drvdata(&dssdev->dev);
	int r;

	mutex_lock(&td->lock);
	r = td->mirror;
	mutex_unlock(&td->lock);

	return r;
}

static int justin_run_test(struct omap_dss_device *dssdev, int test_num)
{
	struct justin_data *td = dev_get_drvdata(&dssdev->dev);
	int r;
#if 1  // sangki.hyun@lge.com
	return 1;
#endif
	mutex_lock(&td->lock);

	if (!td->enabled) {
		r = -ENODEV;
		goto err1;
	}

	dsi_bus_lock(dssdev);

	r = justin_wake_up(dssdev);
	if (r)
		goto err2;

	dsi_bus_unlock(dssdev);
	mutex_unlock(&td->lock);
	return 0;
err2:
	dsi_bus_unlock(dssdev);
err1:
	mutex_unlock(&td->lock);
	return r;
}

static int justin_memory_read(struct omap_dss_device *dssdev,
		void *buf, size_t size,
		u16 x, u16 y, u16 w, u16 h)
{
	int r;
	int first = 1;
	int plen;
	unsigned buf_used = 0;
	struct justin_data *td = dev_get_drvdata(&dssdev->dev);

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

	dsi_bus_lock(dssdev);

	r = justin_wake_up(dssdev);
	if (r)
		goto err2;

	/* plen 1 or 2 goes into short packet. until checksum error is fixed,
	 * use short packets. plen 32 works, but bigger packets seem to cause
	 * an error. */
	if (size % 2)
		plen = 1;
	else
		plen = 2;

	justin_set_update_window(td, x, y, w, h);

	r = dsi_vc_set_max_rx_packet_size(dssdev, td->channel, plen);
	if (r)
		goto err2;

	while (buf_used < size) {
		u8 dcs_cmd = first ? 0x2e : 0x3e;
		first = 0;

		r = dsi_vc_dcs_read(dssdev, td->channel, dcs_cmd,
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
	dsi_vc_set_max_rx_packet_size(dssdev, td->channel, 1);
err2:
	dsi_bus_unlock(dssdev);
err1:
	mutex_unlock(&td->lock);
	return r;
}

static void justin_ulps_work(struct work_struct *work)
{
	struct justin_data *td = container_of(work, struct justin_data,
			ulps_work.work);
	struct omap_dss_device *dssdev = td->dssdev;

	mutex_lock(&td->lock);

	if (dssdev->state != OMAP_DSS_DISPLAY_ACTIVE || !td->enabled) {
		mutex_unlock(&td->lock);
		return;
	}

	dsi_bus_lock(dssdev);

	justin_enter_ulps(dssdev);

	dsi_bus_unlock(dssdev);
	mutex_unlock(&td->lock);
}

static void justin_esd_work(struct work_struct *work)
{
	struct justin_data *td = container_of(work, struct justin_data,
			esd_work.work);
	struct omap_dss_device *dssdev = td->dssdev;
	struct lge_dsi_panel_data *panel_data = get_panel_data(dssdev);
	u8 state1, state2;
	int r;

	mutex_lock(&td->lock);

	if (!td->enabled) {
		mutex_unlock(&td->lock);
		return;
	}

	dsi_bus_lock(dssdev);

	r = justin_wake_up(dssdev);
	if (r) {
		dev_err(&dssdev->dev, "failed to exit ULPS\n");
		goto err;
	}

	r = justin_dcs_read_1(td, DCS_RDDSDR, &state1);
	if (r) {
		dev_err(&dssdev->dev, "failed to read Taal status\n");
		goto err;
	}

	/* Run self diagnostics */
	r = justin_sleep_out(td);
	if (r) {
		dev_err(&dssdev->dev, "failed to run Taal self-diagnostics\n");
		goto err;
	}

	r = justin_dcs_read_1(td, DCS_RDDSDR, &state2);
	if (r) {
		dev_err(&dssdev->dev, "failed to read Taal status\n");
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
	if (td->te_enabled && panel_data->use_ext_te) {
		r = justin_dcs_write_1(td, DCS_TEAR_ON, 0);
		if (r)
			goto err;
	}

	dsi_bus_unlock(dssdev);

	justin_queue_esd_work(dssdev);

	mutex_unlock(&td->lock);
	return;
err:
	dev_err(&dssdev->dev, "performing LCD reset\n");

	justin_panel_reset(dssdev);

	dsi_bus_unlock(dssdev);

	justin_queue_esd_work(dssdev);

	mutex_unlock(&td->lock);
}

static int justin_set_update_mode(struct omap_dss_device *dssdev,
		enum omap_dss_update_mode mode)
{
	if (mode != OMAP_DSS_UPDATE_MANUAL)
		return -EINVAL;
	return 0;
}

static enum omap_dss_update_mode justin_get_update_mode(
		struct omap_dss_device *dssdev)
{
	return OMAP_DSS_UPDATE_MANUAL;
}

static struct omap_dss_driver justin_driver = {
	.probe		= justin_probe,
	.remove		= __exit_p(justin_remove),

	.enable		= justin_enable,
	.disable	= justin_disable,
	.suspend	= justin_suspend,
	.resume		= justin_resume,

	.set_update_mode = justin_set_update_mode,
	.get_update_mode = justin_get_update_mode,

	.update		= justin_update,
	.sync		= justin_sync,

	.get_resolution	= justin_get_resolution,
	.get_recommended_bpp = omapdss_default_get_recommended_bpp,

	.enable_te	= justin_enable_te,
	.get_te		= justin_get_te,

	.set_rotate	= justin_rotate,
	.get_rotate	= justin_get_rotate,
	.set_mirror	= justin_mirror,
	.get_mirror	= justin_get_mirror,
	.run_test	= justin_run_test,
	.memory_read	= justin_memory_read,

	.get_timings	= justin_get_timings,

	.driver         = {
		.name   = "justin_panel",
		.owner  = THIS_MODULE,
	},
};


// S, [mannsik.chung@lge.com] LCD off boot. 
int check_no_lcd(void)
{
	return lcd_off_boot;
}
EXPORT_SYMBOL(check_no_lcd);
// E, [mannsik.chung@lge.com] LCD off boot.

static int __init justin_init(void)
{
	omap_dss_register_driver(&justin_driver);

	return 0;
}

static void __exit justin_exit(void)
{
	omap_dss_unregister_driver(&justin_driver);
}

module_init(justin_init);
module_exit(justin_exit);

MODULE_AUTHOR("kyungtae Oh <kyungtae.oh@lge.com>");
MODULE_DESCRIPTION("justin Driver");
MODULE_LICENSE("GPL");
