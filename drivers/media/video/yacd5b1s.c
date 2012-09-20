//--[[ LGE_UBIQUIX_MODIFIED_START : ymjun@mnbt.co.kr [2011.07.26] - CAM : from black froyo
/*
 * drivers/media/video/yacd5b1s.c
 *
 * yacd5b1s sensor driver
 *
 *
 * Copyright (C) 2010 MM Solutions AD
 *
 *
 * This file is licensed under the terms of the GNU General Public License
 * version 2. This program is licensed "as is" without any warranty of any
 * kind, whether express or implied.
 */

#include <linux/i2c.h>
#include <linux/delay.h>

#include <media/v4l2-int-device.h>

#include "yacd5b1s.h"
#include "yacd5b1s_regs.h"
#include "omap34xxcam.h"
#include "isp/isp.h"
#include "isp/ispcsi2.h"
#include <linux/videodev2.h>


#define YACD5B1S_DRIVER_NAME  "yacd5b1s"
#define YACD5B1S_MOD_NAME "YACD5B1S: "

#define I2C_M_WR 0

/**
 * struct yacd5b1s_sensor - main structure for storage of sensor information
 * @pdata: access functions and data for platform level information
 * @v4l2_int_device: V4L2 device structure structure
 * @i2c_client: iic client device structure
 * @pix: V4L2 pixel format information structure
 * @timeperframe: time per frame expressed as V4L fraction
 * @scaler:
 * @ver: yacd5b1s chip version
 * @fps: frames per second value
 */
struct yacd5b1s_sensor {
	const struct yacd5b1s_platform_data *pdata;
	struct v4l2_int_device *v4l2_int_device;
	struct i2c_client *i2c_client;
	struct v4l2_pix_format pix;
	struct v4l2_fract timeperframe;
	int scaler;
	int ver;
	int fps;
	bool resuming;
	bool reset_camera;
	bool vt_mode;
	bool capture_mode;
};

static struct yacd5b1s_sensor yacd5b1s;
static struct i2c_driver yacd5b1ssensor_i2c_driver;
static unsigned long xclk_current = YACD5B1S_XCLK_NOM_1;
static enum yacd5b1s_image_size isize_current = YACD5B1S_2MP;

/* list of image formats supported by yacd5b1s sensor */
const static struct v4l2_fmtdesc yacd5b1s_formats[] = {
	{
		.description = "UYVY (YUV 4:2:2), packed",
		.pixelformat = V4L2_PIX_FMT_UYVY,
	}
};

const static struct yacd5b1s_reg *config_regs_table[YACD5B1S_CFG_MODE][YACD5B1S_SIZE] = {
	{ // preview mode
		set_preview_qcif_config,
		set_preview_qvga_config,
		set_preview_vga_config,
		sensor_core_dummy,
		sensor_core_dummy
	},
	{ // return to preview mode
		set_return_to_preview_qcif_config,
		set_return_to_preview_qvga_config,
		set_return_to_preview_vga_config,
		sensor_core_dummy,
		sensor_core_dummy,
	},
	{ // capture mode
		set_capture_qcif_config,
		set_capture_qvga_config,
		set_capture_vga_config,
		set_capture_1mpix_config,
		set_capture_2mpix_config
	},
};

#define NUM_CAPTURE_FORMATS ARRAY_SIZE(yacd5b1s_formats)

static enum v4l2_power current_power_state;

static struct yacd5b1s_clock_freq current_clk;

struct i2c_list {
	struct i2c_msg *reg_list;
	unsigned int list_size;
};

/**
 * struct vcontrol - Video controls
 * @v4l2_queryctrl: V4L2 VIDIOC_QUERYCTRL ioctl structure
 * @current_value: current value of this control
 */
static struct vcontrol {
	struct v4l2_queryctrl qc;
	int current_value;
} yacd5b1ssensor_video_control[] = {
	{
		{
			.id = V4L2_CID_EXPOSURE,
			.type = V4L2_CTRL_TYPE_INTEGER,
			.name = "Exposure",
			.minimum = YACD5B1S_MIN_EXPOSURE,
			.maximum = YACD5B1S_MAX_EXPOSURE,
			.step = YACD5B1S_EXPOSURE_STEP,
			.default_value = YACD5B1S_DEF_EXPOSURE,
		},
		.current_value = YACD5B1S_DEF_EXPOSURE,
	},
	{
		{
			.id = V4L2_CID_GAIN,
			.type = V4L2_CTRL_TYPE_INTEGER,
			.name = "Analog Gain",
			.minimum = YACD5B1S_MIN_GAIN,
			.maximum = YACD5B1S_MAX_GAIN,
			.step = YACD5B1S_GAIN_STEP,
			.default_value = YACD5B1S_DEF_GAIN,
		},
		.current_value = YACD5B1S_DEF_GAIN,
	},

	{
		{
			.id = V4L2_CID_BRIGHTNESS,
			.type = V4L2_CTRL_TYPE_INTEGER,
			.name = "brightness",
			.minimum = YACD5B1S_MIN_BRIGHTNESS,
			.maximum = YACD5B1S_MAX_BRIGHTNESS,
			.step = YACD5B1S_BRIGHTNESS_STEP,
			.default_value = YACD5B1S_DEF_BRIGHTNESS,
		},
		.current_value = YACD5B1S_DEF_BRIGHTNESS,
	},
	{
		{
			.id = V4L2_CID_POWER_LINE_FREQUENCY,
			.type = V4L2_CTRL_TYPE_INTEGER,
			.name = "antibanding",
			.minimum = YACD5B1S_MIN_FLICKER,
			.maximum = YACD5B1S_MAX_FLICKER,
			.step = YACD5B1S_FLICKER_STEP,
			.default_value = YACD5B1S_DEF_FLICKER,
		},
		.current_value = YACD5B1S_DEF_FLICKER,
	},
	{
		{
			.id = V4L2_CID_COLORFX,
			.type = V4L2_CTRL_TYPE_INTEGER,
			.name = "effect",
			.minimum = YACD5B1S_MIN_COLORFX,
			.maximum = YACD5B1S_MAX_COLORFX,
			.step = YACD5B1S_COLORFX_STEP,
			.default_value = YACD5B1S_DEF_COLORFX,
		},
		.current_value = YACD5B1S_DEF_COLORFX,
	},
	{
		{
			.id = V4L2_CID_AUTO_WHITE_BALANCE,
			.type = V4L2_CTRL_TYPE_INTEGER,
			.name = "whitebalance",
			.minimum = YACD5B1S_MIN_WB,
			.maximum = YACD5B1S_MAX_WB,
			.step = YACD5B1S_WB_STEP,
			.default_value = YACD5B1S_DEF_WB,
		},
		.current_value = YACD5B1S_DEF_WB,
	},

	{
		{
			.id = V4L2_CID_ROTATE,
			.type = V4L2_CTRL_TYPE_INTEGER,
			.name = "mirror",
			.minimum = YACD5B1S_MIN_MIRROR,
			.maximum = YACD5B1S_MAX_MIRROR,
			.step = YACD5B1S_MIRROR_STEP,
			.default_value = YACD5B1S_DEF_MIRROR,
		},
		.current_value = YACD5B1S_DEF_MIRROR,
	},
	{
		{
			.id = V4L2_CID_HCENTER,//TBD
			.type = V4L2_CTRL_TYPE_INTEGER,
			.name = "meter-mode",
			.minimum = YACD5B1S_MIN_METERING,
			.maximum = YACD5B1S_MAX_METERING,
			.step = YACD5B1S_METERING_STEP,
			.default_value = YACD5B1S_DEF_METERING,
		},
		.current_value = YACD5B1S_DEF_METERING,
	},
	{
		{
			.id = V4L2_CID_PRIVATE_OMAP3ISP_HYNIX_SMART_CAMERA,
			.type = V4L2_CTRL_TYPE_BOOLEAN,
			.name = "reset-camera-module",
			.minimum = 0,
			.maximum = 1,
			.step = 1,
			.default_value = 0,
		},
		.current_value = 0,
	},
	{
		{
			.id = V4L2_CID_VCENTER,
			.type = V4L2_CTRL_TYPE_INTEGER,
			.name = "night-mode",
			.minimum = 0,
			.maximum = 1,
			.step = 1,
			.default_value = 0,
		},
		.current_value = 0,
	},
	{
		{
			.id = V4L2_CID_PRIVATE_OMAP3ISP_HYNIX_SMART_CAMERA_VT,
			.type = V4L2_CTRL_TYPE_BOOLEAN,
			.name = "VT-mode",
			.minimum = 0,
			.maximum = 1,
			.step = 1,
			.default_value = 0,
		},
		.current_value = 0,
	},


};

/**
 * find_vctrl - Finds the requested ID in the video control structure array
 * @id: ID of control to search the video control array for
 *
 * Returns the index of the requested ID from the control structure array
 */
static int
find_vctrl(int id)
{
	int i;

	if (id < V4L2_CID_BASE)
		return -EDOM;

	for (i = (ARRAY_SIZE(yacd5b1ssensor_video_control) - 1); i >= 0; i--)
		if (yacd5b1ssensor_video_control[i].qc.id == id)
			break;
	if (i < 0)
		i = -EINVAL;
	return i;
}

/**
 * yacd5b1s_read_reg - Read a value from a register in an yacd5b1s sensor device
 * @client: i2c driver client structure
 * @data_length: length of data to be read
 * @reg: register address / offset
 * @val: stores the value that gets read
 *
 * Read a value from a register in an yacd5b1s sensor device.
 * The value is returned in 'val'.
 * Returns zero if successful, or non-zero otherwise.
 */
static int
yacd5b1s_read_reg(struct i2c_client *client, u16 data_length, u16 reg, u32 *val)
{
	int err;
	struct i2c_msg msg[1];
	unsigned char data[4];

	if (!client->adapter)
		return -ENODEV;
	if (data_length != I2C_8BIT && data_length != I2C_16BIT
			&& data_length != I2C_32BIT)
		return -EINVAL;

	msg->addr = client->addr;
	msg->flags = 0;
	msg->len = 2;
	msg->buf = data;

	/* Write addr - high byte goes out first */
	data[0] = (u8) (reg >> 8);;
	data[1] = (u8) (reg & 0xff);
	err = i2c_transfer(client->adapter, msg, 1);
	udelay(2);
	/* Read back data */
	if (err >= 0) {
		msg->len = data_length;
		msg->flags = I2C_M_RD;
		err = i2c_transfer(client->adapter, msg, 1);
	}
	else
	{
		printk(KERN_ERR "-yacd5b1s_read_reg write error\n");
	}
	if (err >= 0) {
		*val = 0;
		/* high byte comes first */
		if (data_length == I2C_8BIT)
			*val = data[0];
		else if (data_length == I2C_16BIT)
			*val = data[1] + (data[0] << 8);
		else
			*val = data[3] + (data[2] << 8) +
				(data[1] << 16) + (data[0] << 24);
		return 0;
	}
	v4l_err(client, "read from offset 0x%x error %d\n", reg, err);
    printk(KERN_ERR "-yacd5b1s_read_reg, -1\n");
	return err;
}

/**
 * Write a value to a register in yacd5b1s sensor device.
 * @client: i2c driver client structure.
 * @reg: Address of the register to read value from.
 * @val: Value to be written to a specific register.
 * Returns zero if successful, or non-zero otherwise.
 */
static int yacd5b1s_write_reg(struct i2c_client *client, u16 reg,
						u32 val, u16 data_length)
{
	int err = 0;
	struct i2c_msg msg[1];
	unsigned char data[6];
	int retries = 0;

	if (!client->adapter)
		return -ENODEV;

	if (data_length != I2C_8BIT && data_length != I2C_16BIT
			&& data_length != I2C_32BIT)
		return -EINVAL;

retry:
	msg->addr = client->addr;
	msg->flags = I2C_M_WR;
	msg->len = data_length+1;  /* add address bytes */
	msg->buf = data;

	/* high byte goes out first */
	data[0] = (u8) (reg & 0xff);
	if (data_length == I2C_8BIT) {
		data[1] = val & 0xff;
	} else if (data_length == I2C_16BIT) {
		data[1] = (val >> 8) & 0xff;
		data[2] = val & 0xff;
	} else {
		data[1] = (val >> 24) & 0xff;
		data[2] = (val >> 16) & 0xff;
		data[3] = (val >> 8) & 0xff;
		data[4] = val & 0xff;
	}

	if (data_length == 1)
		dev_dbg(&client->dev, "YACD5B1S Wrt:[0x%04X]=0x%02X\n",
				reg, val);
	else if (data_length == 2)
		dev_dbg(&client->dev, "YACD5B1S Wrt:[0x%04X]=0x%04X\n",
				reg, val);

	err = i2c_transfer(client->adapter, msg, 1);
	udelay(50);

	if (err >= 0)
		return 0;

	if (retries <= 5) {
		v4l_info(client, "Retrying I2C... %d", retries);
		retries++;
		mdelay(20);
		goto retry;
	}

	return err;
}

/**
 * Initialize a list of yacd5b1s registers.
 * The list of registers is terminated by the pair of values
 * {OV3640_REG_TERM, OV3640_VAL_TERM}.
 * @client: i2c driver client structure.
 * @reglist[]: List of address of the registers to write data.
 * Returns zero if successful, or non-zero otherwise.
 */
static int yacd5b1s_write_regs(struct i2c_client *client,
					const struct yacd5b1s_reg reglist[])
{
	int err = 0;
	const struct yacd5b1s_reg *list = reglist;

	while (!((list->reg == I2C_REG_TERM)
		&& (list->val == I2C_VAL_TERM))) {
		err = yacd5b1s_write_reg(client, list->reg,
				list->val, list->length);
		if (err)
			return err;
		list++;
	}
	return 0;
}

/**
 * yacd5b1s_find_size - Find the best match for a requested image capture size
 * @width: requested image width in pixels
 * @height: requested image height in pixels
 *
 * Find the best match for a requested image capture size.  The best match
 * is chosen as the nearest match that has the same number or fewer pixels
 * as the requested size, or the smallest image size if the requested size
 * has fewer pixels than the smallest image.
 * Since the available sizes are subsampled in the vertical direction only,
 * the routine will find the size with a height that is equal to or less
 * than the requested height.
 */
static enum yacd5b1s_image_size yacd5b1s_find_size(unsigned int width,
							unsigned int height)
{
	enum yacd5b1s_image_size isize;

	for (isize = YACD5B1S_QCIF; isize <= YACD5B1S_2MP; isize++) {
		if ((yacd5b1s_sizes[isize].height >= height) &&
			(yacd5b1s_sizes[isize].width >= width)) {
			break;
		}
	}

	printk(KERN_DEBUG "yacd5b1s_find_size: Req Size=%dx%d, "
			"Calc Size=%dx%d\n",
			width, height, (int)yacd5b1s_sizes[isize].width,
			(int)yacd5b1s_sizes[isize].height);

	return isize;
}

#if 0
/**
 * Set CSI2 Virtual ID.
 * @client: i2c client driver structure
 * @id: Virtual channel ID.
 *
 * Sets the channel ID which identifies data packets sent by this device
 * on the CSI2 bus.
 **/
static int yacd5b1s_set_virtual_id(struct i2c_client *client, u32 id)
{
	return 0;
}


/**
 * yacd5b1s_set_framerate - Sets framerate by adjusting frame_length_lines reg.
 * @s: pointer to standard V4L2 device structure
 * @fper: frame period numerator and denominator in seconds
 *
 * The maximum exposure time is also updated since it is affected by the
 * frame rate.
 **/
static int yacd5b1s_set_framerate(struct v4l2_int_device *s,
						struct v4l2_fract *fper)
{
	int err = 0;
	return err;
}
#endif

/**
 * yacd5b1ssensor_calc_xclk - Calculate the required xclk frequency
 *
 * Xclk is not determined from framerate for the YACD5B1S
 */
static unsigned long yacd5b1ssensor_calc_xclk(void)
{
	xclk_current = YACD5B1S_XCLK_NOM_1;

	return xclk_current;
}

#if 0
/**
 * Sets the correct orientation based on the sensor version.
 *   IU046F2-Z   version=2  orientation=3
 *   IU046F4-2D  version>2  orientation=0
 */
static int yacd5b1s_set_orientation(struct i2c_client *client, u32 ver)
{
	int err = 0;
	return err;
}
#endif

/**
 * yacd5b1ssensor_set_exposure_time - sets exposure time per input value
 * @exp_time: exposure time to be set on device (in usec)
 * @s: pointer to standard V4L2 device structure
 * @lvc: pointer to V4L2 exposure entry in yacd5b1ssensor_video_controls array
 *
 * If the requested exposure time is within the allowed limits, the HW
 * is configured to use the new exposure time, and the
 * yacd5b1ssensor_video_control[] array is updated with the new current value.
 * The function returns 0 upon success.  Otherwise an error code is
 * returned.
 */
int yacd5b1ssensor_set_exposure_time(u32 exp_time, struct v4l2_int_device *s,
							struct vcontrol *lvc)
{
	int err = 0;
	return err;
}

/**
 * yacd5b1ssensor_set_gain - sets sensor analog gain per input value
 * @gain: analog gain value to be set on device
 * @s: pointer to standard V4L2 device structure
 * @lvc: pointer to V4L2 analog gain entry in yacd5b1ssensor_video_control array
 *
 * If the requested analog gain is within the allowed limits, the HW
 * is configured to use the new gain value, and the yacd5b1ssensor_video_control
 * array is updated with the new current value.
 * The function returns 0 upon success.  Otherwise an error code is
 * returned.
 */
int yacd5b1ssensor_set_gain(u16 lineargain, struct v4l2_int_device *s,
							struct vcontrol *lvc)
{
	int err = 0;
	return err;
}


int yacd5b1ssensor_set_brightness(u16 value, struct v4l2_int_device *s,
							struct vcontrol *lvc)
{
	int err = 0;
	struct yacd5b1s_sensor *sensor = s->priv;
	struct i2c_client *client = sensor->i2c_client;
	printk(KERN_ERR "yacd5b1ssensor_set_brightness %d\n",value);
	switch (value) {
	case  0:
		yacd5b1s_write_regs(client, Brightness1);
	break;
	case  1:
		yacd5b1s_write_regs(client, Brightness2);
	break;
	case  2:
		yacd5b1s_write_regs(client, Brightness3);
	break;
	case  3:
		yacd5b1s_write_regs(client, Brightness4);
	break;
	case  4:
		yacd5b1s_write_regs(client, Brightness5);
	break;
	case  5:
		yacd5b1s_write_regs(client, Brightness6);
	break;
	case  6:
		yacd5b1s_write_regs(client, Brightness7);
	break;
	case  7:
		yacd5b1s_write_regs(client, Brightness8);
	break;
	case  8:
		yacd5b1s_write_regs(client, Brightness9);
	break;
	case  9:
		yacd5b1s_write_regs(client, Brightness10);
	break;
	case  10:
		yacd5b1s_write_regs(client, Brightness11);
	break;
	case  11:
		yacd5b1s_write_regs(client, Brightness12);
	break;
	case  12:
		yacd5b1s_write_regs(client, Brightness13);
	break;

	default:
		return -EINVAL;
	}
	return err;
}

int yacd5b1ssensor_set_flicker(u16 value, struct v4l2_int_device *s,
							struct vcontrol *lvc)
{
	int err = 0;
	struct yacd5b1s_sensor *sensor = s->priv;
	struct i2c_client *client = sensor->i2c_client;
	u32 val1 = 0;

	printk(KERN_ERR "yacd5b1ssensor_set_flicker %d\n",value);
	switch (value) {
	case  0:
		yacd5b1s_write_reg(client, 0x03, 0x20, I2C_8BIT);
		yacd5b1s_read_reg(client, I2C_8BIT, 0x10, &val1);
		yacd5b1s_write_reg(client, 0x10, ((val1 | 0x10 ) & 0xBF), I2C_8BIT);
		
	break;
	case  1:
		yacd5b1s_write_reg(client, 0x03, 0x20, I2C_8BIT);
		yacd5b1s_read_reg(client, I2C_8BIT, 0x10, &val1);
		yacd5b1s_write_reg(client, 0x10, (val1  & 0xAF), I2C_8BIT);
	break;

	default:
		return -EINVAL;
	}
	return err;
}

int yacd5b1ssensor_set_effect(u16 value, struct v4l2_int_device *s,
							struct vcontrol *lvc)
{
	int err = 0;
	struct yacd5b1s_sensor *sensor = s->priv;
	struct i2c_client *client = sensor->i2c_client;


	printk(KERN_ERR "yacd5b1ssensor_set_effect %d\n",value);
	switch (value) {
	case  0:
		yacd5b1s_write_regs(client, Color_Normal);
	break;
	case  1:
		yacd5b1s_write_regs(client, Color_Mono);
	break;
	case  2:
		yacd5b1s_write_regs(client, Color_Sepia);
	break;
	case  3:
		yacd5b1s_write_regs(client, Color_Negative);
	break;
	case  4:
		yacd5b1s_write_regs(client, Color_Solarize);
	break;
	case  5:
		yacd5b1s_write_regs(client, Color_Emboss);
	break;
	default:
		return -EINVAL;
	}

	return err;
}

int yacd5b1ssensor_set_wb(u16 value, struct v4l2_int_device *s,
							struct vcontrol *lvc)
{
	int err = 0;
	struct yacd5b1s_sensor *sensor = s->priv;
	struct i2c_client *client = sensor->i2c_client;
	printk(KERN_ERR "yacd5b1ssensor_set_wb %d\n",value);
	switch (value) {
	case  0:
		yacd5b1s_write_regs(client, AWB_auto);
	break;
	case  1:
		yacd5b1s_write_regs(client, AWB_sun);
	break;
	case  2:
		yacd5b1s_write_regs(client, AWB_tunsten);
	break;
	case  3:
		yacd5b1s_write_regs(client, AWB_fluorescent);
	break;
	case  4:
		yacd5b1s_write_regs(client, AWB_cloudy);
	break;
	default:
		return -EINVAL;
	}
	return err;
}

int yacd5b1ssensor_set_mirror(u16 value, struct v4l2_int_device *s,
							struct vcontrol *lvc)
{
	int err = 0;
	struct yacd5b1s_sensor *sensor = s->priv;
	struct i2c_client *client = sensor->i2c_client;
	printk(KERN_ERR "yacd5b1ssensor_set_mirror %d\n",value);
	switch (value) {
	case  0:
		yacd5b1s_write_regs(client, Mirror_Normal);
	break;
	case  1:
		yacd5b1s_write_regs(client, Horizontal_Mirror);
	break;
	case  2:
		yacd5b1s_write_regs(client, Vertical_Flip);
	break;
	case  3:
		yacd5b1s_write_regs(client, Mirror_and_Flip);
	break;

	default:
		return -EINVAL;
	}
	return err;
}

int yacd5b1ssensor_set_metering(u16 value, struct v4l2_int_device *s,
							struct vcontrol *lvc)
{
	int err = 0;
	struct yacd5b1s_sensor *sensor = s->priv;
	struct i2c_client *client = sensor->i2c_client;
	printk(KERN_ERR "yacd5b1ssensor_set_metering %d\n",value);
	switch (value) {
	case  0:
		yacd5b1s_write_regs(client, Exposure_mode_auto);
	break;
	case  1:
		yacd5b1s_write_regs(client, Exposure_mode_average);
	break;
	case  2:
		yacd5b1s_write_regs(client, Exposure_mode_multi);
	break;
	case  3:
		yacd5b1s_write_regs(client, Exposure_mode_cen);
	break;

	default:
		return -EINVAL;
	}
	return err;
}


int yacd5b1ssensor_set_nightmode(u16 value, struct v4l2_int_device *s,
							struct vcontrol *lvc)
{
	int err = 0;
	struct yacd5b1s_sensor *sensor = s->priv;
	struct i2c_client *client = sensor->i2c_client;
	printk(KERN_ERR "yacd5b1ssensor_set_nightmode %d\n",value);
	switch (value) {
	case  0:
		yacd5b1s_write_regs(client, Night_mode_off);
	break;
	case  1:
		yacd5b1s_write_regs(client, Night_mode_on);
	break;

	default:
		return -EINVAL;
	}
	return err;
}


/**
 * yacd5b1s_update_clocks - calcs sensor clocks based on sensor settings.
 * @isize: image size enum
 */
int yacd5b1s_update_clocks(struct v4l2_int_device *s, u32 xclk, enum yacd5b1s_image_size isize)
{

	return 0;
}

/**
 * yacd5b1s_setup_pll - initializes sensor PLL registers.
 * @c: i2c client driver structure
 * @isize: image size enum
 */
int yacd5b1s_setup_pll(struct i2c_client *client, enum yacd5b1s_image_size isize)
{
#if 0
	yacd5b1s_write_regs(client, pll_settings);
#endif

	return 0;
}

/**
 * yacd5b1s_setup_mipi - initializes sensor & isp MIPI registers.
 * @c: i2c client driver structure
 * @isize: image size enum
 */
int yacd5b1s_setup_mipi(struct v4l2_int_device *s,
			enum yacd5b1s_image_size isize)
{
	return 0;
}

/**
 * yacd5b1s_configure_frame - initializes image frame registers
 * @c: i2c client driver structure
 * @isize: image size enum
 */
int yacd5b1s_configure_frame(struct i2c_client *client,
			enum yacd5b1s_image_size isize)
{
	return 0;
}

/**
 * yacd5b1s_configure - Configure the yacd5b1s for the specified image mode
 * @s: pointer to standard V4L2 device structure
 *
 * Configure the yacd5b1s for a specified image size, pixel format, and frame
 * period.  xclk is the frequency (in Hz) of the xclk input to the yacd5b1s.
 * fper is the frame period (in seconds) expressed as a fraction.
 * Returns zero if successful, or non-zero otherwise.
 * The actual frame period is returned in fper.
 */
static int yacd5b1s_configure(struct v4l2_int_device *s)
{
	const struct yacd5b1s_reg *cfg_table = NULL;
	struct yacd5b1s_sensor *sensor = s->priv;
	struct v4l2_pix_format *pix = &sensor->pix;
	struct i2c_client *client = sensor->i2c_client;
	enum yacd5b1s_image_size isize;
	enum yacd5b1s_cfg_mode config_mode = YACD5B1S_CFG_PREVIEW;
	int err = 0;

	isize = yacd5b1s_find_size(pix->width, pix->height);
	isize_current = isize;

	printk(KERN_ERR "+yacd5b1s_configure size = [%d], reset_camera(%d), capture_mode(%d)\n", \
			isize_current, sensor->reset_camera, sensor->capture_mode);

	if (!sensor->reset_camera){
		if (!sensor->vt_mode){
			/* set dinamic frame rate 10 ~ 22 fps */
			yacd5b1s_write_regs(client, sensor_core_settings);
		}else{
			/* set fixed frame rate 15fps */
			yacd5b1s_write_regs(client, VT_sensor_core_settings);
		}
		config_mode = YACD5B1S_CFG_PREVIEW; // preview mode
	} else {
		if (!sensor->capture_mode) {
			config_mode = YACD5B1S_CFG_RETPREVIEW; // return to preview mode
		} else {
			config_mode = YACD5B1S_CFG_CAPTURE; // capture mode
		}
	}

	cfg_table = config_regs_table[config_mode][isize];

	yacd5b1s_write_regs(client, cfg_table);
	
	mdelay(30);
		     
	return err;
}

/**
 * yacd5b1s_detect - Detect if an yacd5b1s is present, and if so which revision
 * @client: pointer to the i2c client driver structure
 * Detect if an yacd5b1s is present, and if so which revision.
 * A device is considered to be detected if the manufacturer ID (MIDH and MIDL)
 * and the product ID (PID) registers match the expected values.
 * Any value of the version ID (VER) register is accepted.
 * Returns a negative error number if no device is detected, or the
 * non-negative value of the version ID register if a device is detected.
 */
static int
yacd5b1s_detect(struct i2c_client *client)
{
	u32 model_id,  rev = 0;
	struct yacd5b1s_sensor *sensor;
	dev_err(&client->dev, "client.addr = 0x%x \n", client->addr);

	if (!client)
		return -ENODEV;

	sensor = i2c_get_clientdata(client);

	yacd5b1s_write_reg(client, 0x03, 0x00, I2C_8BIT);


	if (yacd5b1s_read_reg(client, I2C_8BIT, YACD5B1S_REG_MODEL_ID, &model_id))
	    return -ENODEV;

	
	dev_info(&client->dev, "model id detected, 0x%x\n", model_id);

	if (model_id != YACD5B1S_FW_SENSOR_ID) {
		
		dev_warn(&client->dev, "model id mismatch, 0x%x\n", model_id);

		return -ENODEV;
	}

	return rev;
}

/**
 * ioctl_queryctrl - V4L2 sensor interface handler for VIDIOC_QUERYCTRL ioctl
 * @s: pointer to standard V4L2 device structure
 * @qc: standard V4L2 VIDIOC_QUERYCTRL ioctl structure
 *
 * If the requested control is supported, returns the control information
 * from the yacd5b1ssensor_video_control[] array.
 * Otherwise, returns -EINVAL if the control is not supported.
 */
static int ioctl_queryctrl(struct v4l2_int_device *s,
				struct v4l2_queryctrl *qc)
{
	int i;

    printk(KERN_ERR "+ioctl_queryctrl\n");
	i = find_vctrl(qc->id);
	if (i == -EINVAL)
		qc->flags = V4L2_CTRL_FLAG_DISABLED;

	if (i < 0)
		return -EINVAL;

	*qc = yacd5b1ssensor_video_control[i].qc;
	return 0;
}

/**
 * ioctl_g_ctrl - V4L2 sensor interface handler for VIDIOC_G_CTRL ioctl
 * @s: pointer to standard V4L2 device structure
 * @vc: standard V4L2 VIDIOC_G_CTRL ioctl structure
 *
 * If the requested control is supported, returns the control's current
 * value from the yacd5b1ssensor_video_control[] array.
 * Otherwise, returns -EINVAL if the control is not supported.
 */
static int ioctl_g_ctrl(struct v4l2_int_device *s,
			     struct v4l2_control *vc)
{
	struct vcontrol *lvc;
	int i;

	i = find_vctrl(vc->id);
	if (i < 0)
		return -EINVAL;
	lvc = &yacd5b1ssensor_video_control[i];

	switch (vc->id) {
	case  V4L2_CID_EXPOSURE:
		vc->value = lvc->current_value;
		break;
	case V4L2_CID_GAIN:
		vc->value = lvc->current_value;
		break;
	case V4L2_CID_BRIGHTNESS:
		vc->value = lvc->current_value;
		break;
	case V4L2_CID_POWER_LINE_FREQUENCY://flicker
		vc->value = lvc->current_value;
		break;
	case V4L2_CID_COLORFX://effect
		vc->value = lvc->current_value;
		break;
	case V4L2_CID_AUTO_WHITE_BALANCE://manual white balance
		vc->value = lvc->current_value;
		break;
	case V4L2_CID_ROTATE://mirror
		vc->value = lvc->current_value;
		break;
	case V4L2_CID_HCENTER://metering
		vc->value = lvc->current_value;
		break;
	case V4L2_CID_PRIVATE_OMAP3ISP_HYNIX_SMART_CAMERA:
		vc->value = lvc->current_value;
		break;
	case V4L2_CID_VCENTER: //night mode
		vc->value = lvc->current_value;
		break;
	case V4L2_CID_PRIVATE_OMAP3ISP_HYNIX_SMART_CAMERA_VT:
		vc->value = lvc->current_value;
		break;
	}

	return 0;
}

/**
 * ioctl_s_ctrl - V4L2 sensor interface handler for VIDIOC_S_CTRL ioctl
 * @s: pointer to standard V4L2 device structure
 * @vc: standard V4L2 VIDIOC_S_CTRL ioctl structure
 *
 * If the requested control is supported, sets the control's current
 * value in HW (and updates the yacd5b1ssensor_video_control[] array).
 * Otherwise, * returns -EINVAL if the control is not supported.
 */
static int ioctl_s_ctrl(struct v4l2_int_device *s,
			     struct v4l2_control *vc)
{
	struct yacd5b1s_sensor *sensor = s->priv;
	int retval = -EINVAL;
	int i;
	struct vcontrol *lvc;
	//printk(KERN_DEBUG "[yacd5b1s] ioctl_s_ctrl(id: %d, value: %d)\n", vc->id, vc->value);

	i = find_vctrl(vc->id);
	if (i < 0)
		return -EINVAL;
	lvc = &yacd5b1ssensor_video_control[i];

	switch (vc->id) {
	case V4L2_CID_EXPOSURE:
		retval = yacd5b1ssensor_set_exposure_time(vc->value, s, lvc);
		break;
	case V4L2_CID_GAIN:
		retval = yacd5b1ssensor_set_gain(vc->value, s, lvc);
		break;
	case V4L2_CID_BRIGHTNESS:
		retval = yacd5b1ssensor_set_brightness(vc->value, s, lvc);
		break;
	case V4L2_CID_POWER_LINE_FREQUENCY://flicker
		retval = yacd5b1ssensor_set_flicker(vc->value, s, lvc);
		break;
	case V4L2_CID_COLORFX://effect
		retval = yacd5b1ssensor_set_effect(vc->value, s, lvc);
		break;
	case V4L2_CID_AUTO_WHITE_BALANCE://manual white balance
		retval = yacd5b1ssensor_set_wb(vc->value, s, lvc);
		break;
	case V4L2_CID_ROTATE://mirror
		retval = yacd5b1ssensor_set_mirror(vc->value, s, lvc);
		break;
	case V4L2_CID_HCENTER://metering
		retval = yacd5b1ssensor_set_metering(vc->value, s, lvc);
		break;
	case V4L2_CID_PRIVATE_OMAP3ISP_HYNIX_SMART_CAMERA:
		if(vc->value) {
			sensor->reset_camera = true;
			sensor->capture_mode = true;
		} else {
			sensor->reset_camera = false;
		}
		lvc->current_value = vc->value;
		retval = 0;
		break;
	case V4L2_CID_VCENTER: //night mode
		retval = yacd5b1ssensor_set_nightmode(vc->value, s, lvc);
		break;
	case V4L2_CID_PRIVATE_OMAP3ISP_HYNIX_SMART_CAMERA_VT:
		sensor->vt_mode = vc->value ? true : false;
		lvc->current_value = vc->value;
		retval = 0;
		break;

	}

	return retval;
}

/**
 * ioctl_enum_fmt_cap - Implement the CAPTURE buffer VIDIOC_ENUM_FMT ioctl
 * @s: pointer to standard V4L2 device structure
 * @fmt: standard V4L2 VIDIOC_ENUM_FMT ioctl structure
 *
 * Implement the VIDIOC_ENUM_FMT ioctl for the CAPTURE buffer type.
 */
static int ioctl_enum_fmt_cap(struct v4l2_int_device *s,
				   struct v4l2_fmtdesc *fmt)
{
	int index = fmt->index;
	enum v4l2_buf_type type = fmt->type;

	memset(fmt, 0, sizeof(*fmt));
	fmt->index = index;
	fmt->type = type;

	switch (fmt->type) {
	case V4L2_BUF_TYPE_VIDEO_CAPTURE:
		if (index >= NUM_CAPTURE_FORMATS)
			return -EINVAL;
	break;
	default:
		return -EINVAL;
	}

	fmt->flags = yacd5b1s_formats[index].flags;
	strlcpy(fmt->description, yacd5b1s_formats[index].description,
					sizeof(fmt->description));
	fmt->pixelformat = yacd5b1s_formats[index].pixelformat;

	return 0;
}

/**
 * ioctl_try_fmt_cap - Implement the CAPTURE buffer VIDIOC_TRY_FMT ioctl
 * @s: pointer to standard V4L2 device structure
 * @f: pointer to standard V4L2 VIDIOC_TRY_FMT ioctl structure
 *
 * Implement the VIDIOC_TRY_FMT ioctl for the CAPTURE buffer type.  This
 * ioctl is used to negotiate the image capture size and pixel format
 * without actually making it take effect.
 */
static int ioctl_try_fmt_cap(struct v4l2_int_device *s,
			     struct v4l2_format *f)
{
	enum yacd5b1s_image_size isize;
	int ifmt;
	struct v4l2_pix_format *pix = &f->fmt.pix;
	struct yacd5b1s_sensor *sensor = s->priv;
	struct v4l2_pix_format *pix2 = &sensor->pix;

	isize = yacd5b1s_find_size(pix->width, pix->height);
	isize_current = isize;

	pix->width = yacd5b1s_sizes[isize].width;
	pix->height = yacd5b1s_sizes[isize].height;
	for (ifmt = 0; ifmt < NUM_CAPTURE_FORMATS; ifmt++) {
		if (pix->pixelformat == yacd5b1s_formats[ifmt].pixelformat)
			break;
	}
	if (ifmt == NUM_CAPTURE_FORMATS)
		ifmt = 0;
	pix->pixelformat = yacd5b1s_formats[ifmt].pixelformat;
	pix->field = V4L2_FIELD_NONE;
	pix->bytesperline = pix->width * 2;
	pix->sizeimage = pix->bytesperline * pix->height;
	pix->priv = 0;
	pix->colorspace = V4L2_COLORSPACE_JPEG;
	*pix2 = *pix;

	return 0;
}

/**
 * ioctl_s_fmt_cap - V4L2 sensor interface handler for VIDIOC_S_FMT ioctl
 * @s: pointer to standard V4L2 device structure
 * @f: pointer to standard V4L2 VIDIOC_S_FMT ioctl structure
 *
 * If the requested format is supported, configures the HW to use that
 * format, returns error code if format not supported or HW can't be
 * correctly configured.
 */
static int ioctl_s_fmt_cap(struct v4l2_int_device *s,
				struct v4l2_format *f)
{
	struct yacd5b1s_sensor *sensor = s->priv;
	struct v4l2_pix_format *pix = &f->fmt.pix;
	int rval;

	rval = ioctl_try_fmt_cap(s, f);
	if (rval)
		return rval;
	else
		sensor->pix = *pix;


	return rval;
}

/**
 * ioctl_g_fmt_cap - V4L2 sensor interface handler for ioctl_g_fmt_cap
 * @s: pointer to standard V4L2 device structure
 * @f: pointer to standard V4L2 v4l2_format structure
 *
 * Returns the sensor's current pixel format in the v4l2_format
 * parameter.
 */
static int ioctl_g_fmt_cap(struct v4l2_int_device *s,
				struct v4l2_format *f)
{
	struct yacd5b1s_sensor *sensor = s->priv;
	f->fmt.pix = sensor->pix;
	return 0;
}

/**
 * ioctl_g_pixclk - V4L2 sensor interface handler for ioctl_g_pixclk
 * @s: pointer to standard V4L2 device structure
 * @pixclk: pointer to unsigned 32 var to store pixelclk in HZ
 *
 * Returns the sensor's current pixel clock in HZ
 */
static int ioctl_priv_g_pixclk(struct v4l2_int_device *s, u32 *pixclk)
{
	*pixclk = current_clk.vt_pix_clk;

	return 0;
}

/**
 * ioctl_g_activesize - V4L2 sensor interface handler for ioctl_g_activesize
 * @s: pointer to standard V4L2 device structure
 * @pix: pointer to standard V4L2 v4l2_pix_format structure
 *
 * Returns the sensor's current active image basesize.
 */
static int ioctl_priv_g_activesize(struct v4l2_int_device *s,
				   struct v4l2_rect *pix)
{
//	struct yacd5b1s_frame_settings *frm;

	if(0 == isize_current){
		pix->left = 0;
		pix->top = 0;
		pix->width = 176;
		pix->height = 144;
	}else if (1 == isize_current){
		pix->left = 0;
		pix->top = 0;
		pix->width = 320;
		pix->height = 240;
	}else if (2 == isize_current){
		pix->left = 0;
		pix->top = 0;
		pix->width = 640;
		pix->height = 480;
	}else if (3 == isize_current){
		pix->left = 0;
		pix->top = 0;
		pix->width = 1280;
		pix->height = 960;
	}else{
		pix->left = 0;
		pix->top = 0;
		pix->width = 1600;
		pix->height = 1200;
	}

	return 0;
}

/**
 * ioctl_g_fullsize - V4L2 sensor interface handler for ioctl_g_fullsize
 * @s: pointer to standard V4L2 device structure
 * @pix: pointer to standard V4L2 v4l2_pix_format structure
 *
 * Returns the sensor's biggest image basesize.
 */
static int ioctl_priv_g_fullsize(struct v4l2_int_device *s,
				 struct v4l2_rect *pix)
{

//	struct yacd5b1s_frame_settings *frm;

	pix->left = 0;
	pix->top = 0;
	pix->width = 1600;
	pix->height = 1200;

	return 0;
}

/**
 * ioctl_g_pixelsize - V4L2 sensor interface handler for ioctl_g_pixelsize
 * @s: pointer to standard V4L2 device structure
 * @pix: pointer to standard V4L2 v4l2_pix_format structure
 *
 * Returns the sensor's configure pixel size.
 */
static int ioctl_priv_g_pixelsize(struct v4l2_int_device *s,
				  struct v4l2_rect *pix)
{
//	struct yacd5b1s_frame_settings *frm;

	if(0 == isize_current){
		pix->left = 0;
		pix->top = 0;
		pix->width = 2;
		pix->height = 2;
	}else if (1 == isize_current){
		pix->left = 0;
		pix->top = 0;
		pix->width = 2;
		pix->height = 2;
	}else if (2 == isize_current){
		pix->left = 0;
		pix->top = 0;
		pix->width = 2;
		pix->height = 2;
	}else if (3 == isize_current){
		pix->left = 0;
		pix->top = 0;
		pix->width = 1;
		pix->height = 1;
	}else{
		pix->left = 0;
		pix->top = 0;
		pix->width = 1;
		pix->height = 1;
	}

	return 0;
}

/**
 * ioctl_g_parm - V4L2 sensor interface handler for VIDIOC_G_PARM ioctl
 * @s: pointer to standard V4L2 device structure
 * @a: pointer to standard V4L2 VIDIOC_G_PARM ioctl structure
 *
 * Returns the sensor's video CAPTURE parameters.
 */
static int ioctl_g_parm(struct v4l2_int_device *s,
			     struct v4l2_streamparm *a)
{
	struct yacd5b1s_sensor *sensor = s->priv;
	struct v4l2_captureparm *cparm = &a->parm.capture;

	if (a->type != V4L2_BUF_TYPE_VIDEO_CAPTURE)
		return -EINVAL;

	memset(a, 0, sizeof(*a));
	a->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	cparm->capability = V4L2_CAP_TIMEPERFRAME;
	cparm->timeperframe = sensor->timeperframe;


	return 0;
}

/**
 * ioctl_s_parm - V4L2 sensor interface handler for VIDIOC_S_PARM ioctl
 * @s: pointer to standard V4L2 device structure
 * @a: pointer to standard V4L2 VIDIOC_S_PARM ioctl structure
 *
 * Configures the sensor to use the input parameters, if possible.  If
 * not possible, reverts to the old parameters and returns the
 * appropriate error code.
 */
static int ioctl_s_parm(struct v4l2_int_device *s,
			     struct v4l2_streamparm *a)
{
	struct yacd5b1s_sensor *sensor = s->priv;
	struct v4l2_fract *timeperframe = &a->parm.capture.timeperframe;
	int err = 0;

	sensor->timeperframe = *timeperframe;
	yacd5b1ssensor_calc_xclk();
	*timeperframe = sensor->timeperframe;

	return err;
}


/**
 * ioctl_g_priv - V4L2 sensor interface handler for vidioc_int_g_priv_num
 * @s: pointer to standard V4L2 device structure
 * @p: void pointer to hold sensor's private data address
 *
 * Returns device's (sensor's) private data area address in p parameter
 */
static int ioctl_g_priv(struct v4l2_int_device *s, void *p)
{
	struct yacd5b1s_sensor *sensor = s->priv;

	return sensor->pdata->priv_data_set(s, p);

}

static int __yacd5b1s_power_off_standby(struct v4l2_int_device *s,
				      enum v4l2_power on)
{
	struct yacd5b1s_sensor *sensor = s->priv;
	struct i2c_client *client = sensor->i2c_client;
	int rval;

	rval = sensor->pdata->power_set(s, on);
	if (rval < 0) {
		v4l_err(client, ">>>>>Unable to set the power state: "
			YACD5B1S_DRIVER_NAME " sensor\n");
		return rval;
	}

	sensor->pdata->set_xclk(s, 0);
	return 0;
}

static int yacd5b1s_power_off(struct v4l2_int_device *s)
{
	return __yacd5b1s_power_off_standby(s, V4L2_POWER_OFF);
}

static int yacd5b1s_power_standby(struct v4l2_int_device *s)
{
	return __yacd5b1s_power_off_standby(s, V4L2_POWER_STANDBY);
}

static int yacd5b1s_power_on(struct v4l2_int_device *s)
{
	struct yacd5b1s_sensor *sensor = s->priv;
	struct i2c_client *client = sensor->i2c_client;
	int rval;

	sensor->pdata->set_xclk(s, xclk_current);

	rval = sensor->pdata->power_set(s, V4L2_POWER_ON);
	if (rval < 0) {
		v4l_err(client, ">>>>>>Unable to set the power state: "
			YACD5B1S_DRIVER_NAME " sensor\n");
		sensor->pdata->set_xclk(s, 0);
		return rval;
	}

	return 0;
}

/**
 * ioctl_s_power - V4L2 sensor interface handler for vidioc_int_s_power_num
 * @s: pointer to standard V4L2 device structure
 * @on: power state to which device is to be set
 *
 * Sets devices power state to requrested state, if possible.
 */
static int ioctl_s_power(struct v4l2_int_device *s, enum v4l2_power on)
{
	struct yacd5b1s_sensor *sensor = s->priv;
//	struct i2c_client *client = sensor->i2c_client;
//	struct omap34xxcam_hw_config hw_config;
	struct vcontrol *lvc;
	int i;

	//printk(KERN_DEBUG "[_lkh] yacd5b1s: ioctl_s_power(%d ==> %d, reset: %d)(cnt: %d)\n", current_power_state, on, sensor->reset_camera, sensor->capture_mode);

	switch (on) {
	case V4L2_POWER_ON:
		if (!sensor->reset_camera) {
			if (sensor->capture_mode == false)
				yacd5b1s_power_on(s);
			else
				sensor->capture_mode = false;
		}
		yacd5b1s_configure(s);
		break;
	case V4L2_POWER_OFF:
		if(sensor->reset_camera)
			yacd5b1s_power_standby(s);
		yacd5b1s_power_off(s);
		
		sensor->capture_mode = false;

		/* Reset defaults for controls */
		i = find_vctrl(V4L2_CID_GAIN);
		if (i >= 0) {
			lvc = &yacd5b1ssensor_video_control[i];
			lvc->current_value = YACD5B1S_DEF_GAIN;
		}
		i = find_vctrl(V4L2_CID_EXPOSURE);
		if (i >= 0) {
			lvc = &yacd5b1ssensor_video_control[i];
			lvc->current_value = YACD5B1S_DEF_EXPOSURE;
		}
		break;
	case V4L2_POWER_STANDBY:
		if(!sensor->reset_camera)
			yacd5b1s_power_standby(s);
		break;
	}
	sensor->resuming = false;
	current_power_state = on;
	return 0;
}

/**
 * ioctl_init - V4L2 sensor interface handler for VIDIOC_INT_INIT
 * @s: pointer to standard V4L2 device structure
 *
 * Initialize the sensor device (call yacd5b1s_configure())
 */
static int ioctl_init(struct v4l2_int_device *s)
{
	return 0;
}

/**
 * ioctl_dev_exit - V4L2 sensor interface handler for vidioc_int_dev_exit_num
 * @s: pointer to standard V4L2 device structure
 *
 * Delinitialise the dev. at slave detach.  The complement of ioctl_dev_init.
 */
static int ioctl_dev_exit(struct v4l2_int_device *s)
{
	return 0;
}

/**
 * ioctl_dev_init - V4L2 sensor interface handler for vidioc_int_dev_init_num
 * @s: pointer to standard V4L2 device structure
 *
 * Initialise the device when slave attaches to the master.  Returns 0 if
 * yacd5b1s device could be found, otherwise returns appropriate error.
 */
static int ioctl_dev_init(struct v4l2_int_device *s)
{
	struct yacd5b1s_sensor *sensor = s->priv;
	struct i2c_client *client = sensor->i2c_client;
	int err;

	err = yacd5b1s_power_on(s);
	if (err)
		return -ENODEV;

	err = yacd5b1s_detect(client);
	if (err < 0) {
		v4l_err(client, "Unable to detect "
				YACD5B1S_DRIVER_NAME " sensor\n");

		/*
		 * Turn power off before leaving the function.
		 * If not, CAM Pwrdm will be ON which is not needed
		 * as there is no sensor detected.
		 */
		yacd5b1s_power_off(s);

		return err;
	}
	sensor->ver = err;
	v4l_info(client, YACD5B1S_DRIVER_NAME
		" chip version 0x%02x detected\n", sensor->ver);

	err = yacd5b1s_power_off(s);
	if (err)
		return -ENODEV;

	return 0;
}

/**
 * ioctl_enum_framesizes - V4L2 sensor if handler for vidioc_int_enum_framesizes
 * @s: pointer to standard V4L2 device structure
 * @frms: pointer to standard V4L2 framesizes enumeration structure
 *
 * Returns possible framesizes depending on choosen pixel format
 **/
static int ioctl_enum_framesizes(struct v4l2_int_device *s,
					struct v4l2_frmsizeenum *frms)
{
	int ifmt;

	for (ifmt = 0; ifmt < NUM_CAPTURE_FORMATS; ifmt++) {
		if (frms->pixel_format == yacd5b1s_formats[ifmt].pixelformat)
			break;
	}
	/* Is requested pixelformat not found on sensor? */
	if (ifmt == NUM_CAPTURE_FORMATS)
		return -EINVAL;

	/* Check that the index we are being asked for is not
	   out of bounds. */
	if (frms->index >= ARRAY_SIZE(yacd5b1s_sizes))
		return -EINVAL;

	frms->type = V4L2_FRMSIZE_TYPE_DISCRETE;
	frms->discrete.width = yacd5b1s_sizes[frms->index].width;
	frms->discrete.height = yacd5b1s_sizes[frms->index].height;

	return 0;
}

const struct v4l2_fract yacd5b1s_frameintervals[] = {
	{ .numerator = 1, .denominator = 30 },
	{ .numerator = 1, .denominator = 20 },
	{ .numerator = 1, .denominator = 15 },
	{ .numerator = 1, .denominator = 10 },
	{ .numerator = 1, .denominator = 7 },
};

static int ioctl_enum_frameintervals(struct v4l2_int_device *s,
					struct v4l2_frmivalenum *frmi)
{
	int ifmt;

	/* Check that the requested format is one we support */
	for (ifmt = 0; ifmt < NUM_CAPTURE_FORMATS; ifmt++) {
		if (frmi->pixel_format == yacd5b1s_formats[ifmt].pixelformat)
			break;
	}

	if (ifmt == NUM_CAPTURE_FORMATS)
		return -EINVAL;

	/* Check that the index we are being asked for is not
	   out of bounds. */
	if (frmi->index >= ARRAY_SIZE(yacd5b1s_frameintervals))
		return -EINVAL;

	/* Make sure that the 2MP size reports a max of 10fps */
	if (frmi->width == 1600 && frmi->height == 1200) {
		if (frmi->index != 0)
			return -EINVAL;
	}

	frmi->type = V4L2_FRMIVAL_TYPE_DISCRETE;
	frmi->discrete.numerator =
				yacd5b1s_frameintervals[frmi->index].numerator;
	frmi->discrete.denominator =
				yacd5b1s_frameintervals[frmi->index].denominator;

	return 0;
}

static struct v4l2_int_ioctl_desc yacd5b1s_ioctl_desc[] = {
	{ .num = vidioc_int_enum_framesizes_num,
	  .func = (v4l2_int_ioctl_func *)ioctl_enum_framesizes},
	{ .num = vidioc_int_enum_frameintervals_num,
	  .func = (v4l2_int_ioctl_func *)ioctl_enum_frameintervals},
	{ .num = vidioc_int_dev_init_num,
	  .func = (v4l2_int_ioctl_func *)ioctl_dev_init},
	{ .num = vidioc_int_dev_exit_num,
	  .func = (v4l2_int_ioctl_func *)ioctl_dev_exit},
	{ .num = vidioc_int_s_power_num,
	  .func = (v4l2_int_ioctl_func *)ioctl_s_power },
	{ .num = vidioc_int_g_priv_num,
	  .func = (v4l2_int_ioctl_func *)ioctl_g_priv },
	{ .num = vidioc_int_init_num,
	  .func = (v4l2_int_ioctl_func *)ioctl_init },
	{ .num = vidioc_int_enum_fmt_cap_num,
	  .func = (v4l2_int_ioctl_func *)ioctl_enum_fmt_cap },
	{ .num = vidioc_int_try_fmt_cap_num,
	  .func = (v4l2_int_ioctl_func *)ioctl_try_fmt_cap },
	{ .num = vidioc_int_g_fmt_cap_num,
	  .func = (v4l2_int_ioctl_func *)ioctl_g_fmt_cap },
	{ .num = vidioc_int_s_fmt_cap_num,
	  .func = (v4l2_int_ioctl_func *)ioctl_s_fmt_cap },
	{ .num = vidioc_int_g_parm_num,
	  .func = (v4l2_int_ioctl_func *)ioctl_g_parm },
	{ .num = vidioc_int_s_parm_num,
	  .func = (v4l2_int_ioctl_func *)ioctl_s_parm },
	{ .num = vidioc_int_queryctrl_num,
	  .func = (v4l2_int_ioctl_func *)ioctl_queryctrl },
	{ .num = vidioc_int_g_ctrl_num,
	  .func = (v4l2_int_ioctl_func *)ioctl_g_ctrl },
	{ .num = vidioc_int_s_ctrl_num,
	  .func = (v4l2_int_ioctl_func *)ioctl_s_ctrl },
	{ .num = vidioc_int_priv_g_pixclk_num,
	  .func = (v4l2_int_ioctl_func *)ioctl_priv_g_pixclk },
	{ .num = vidioc_int_priv_g_activesize_num,
	  .func = (v4l2_int_ioctl_func *)ioctl_priv_g_activesize },
	{ .num = vidioc_int_priv_g_fullsize_num,
	  .func = (v4l2_int_ioctl_func *)ioctl_priv_g_fullsize },
	{ .num = vidioc_int_priv_g_pixelsize_num,
	  .func = (v4l2_int_ioctl_func *)ioctl_priv_g_pixelsize },	  
	  	
};

static struct v4l2_int_slave yacd5b1s_slave = {
	.ioctls = yacd5b1s_ioctl_desc,
	.num_ioctls = ARRAY_SIZE(yacd5b1s_ioctl_desc),
};

static struct v4l2_int_device yacd5b1s_int_device = {
	.module = THIS_MODULE,
	.name = YACD5B1S_DRIVER_NAME,
	.priv = &yacd5b1s,
	.type = v4l2_int_type_slave,
	.u = {
		.slave = &yacd5b1s_slave,
	},
};

/**
 * yacd5b1s_probe - sensor driver i2c probe handler
 * @client: i2c driver client device structure
 *
 * Register sensor as an i2c client device and V4L2
 * device.
 */
static int __devinit yacd5b1s_probe(struct i2c_client *client,
				   const struct i2c_device_id *id)
{
	struct yacd5b1s_sensor *sensor = &yacd5b1s;
	int err;

	if (i2c_get_clientdata(client))
		return -EBUSY;

	sensor->pdata = client->dev.platform_data;

	if (!sensor->pdata) {
		v4l_err(client, "no platform data?\n");
		return -ENODEV;
	}

	sensor->v4l2_int_device = &yacd5b1s_int_device;
	sensor->i2c_client = client;

	i2c_set_clientdata(client, sensor);

	/* Make the default capture format QCIF V4L2_PIX_FMT_UYVY */
	sensor->pix.width = YACD5B1S_IMAGE_WIDTH_MAX;
	sensor->pix.height = YACD5B1S_IMAGE_HEIGHT_MAX;
	sensor->pix.pixelformat = V4L2_PIX_FMT_UYVY;

	/* Enable reset secondary camera default */
	sensor->reset_camera = false;
	sensor->vt_mode = false;

	sensor->capture_mode = false;

	err = v4l2_int_device_register(sensor->v4l2_int_device);
	if (err)
		i2c_set_clientdata(client, NULL);

	return 0;
}

/**
 * yacd5b1s_remove - sensor driver i2c remove handler
 * @client: i2c driver client device structure
 *
 * Unregister sensor as an i2c client device and V4L2
 * device.  Complement of yacd5b1s_probe().
 */
static int __exit
yacd5b1s_remove(struct i2c_client *client)
{
	struct yacd5b1s_sensor *sensor = i2c_get_clientdata(client);

	if (!client->adapter)
		return -ENODEV;	/* our client isn't attached */

	v4l2_int_device_unregister(sensor->v4l2_int_device);
	i2c_set_clientdata(client, NULL);

	return 0;
}

static const struct i2c_device_id yacd5b1s_id[] = {
	{ YACD5B1S_DRIVER_NAME, 0 },
	{ },
};
MODULE_DEVICE_TABLE(i2c, yacd5b1s_id);

static struct i2c_driver yacd5b1ssensor_i2c_driver = {
	.driver = {
		.name = YACD5B1S_DRIVER_NAME,
		.owner = THIS_MODULE,
	},
	.probe = yacd5b1s_probe,
	.remove = __exit_p(yacd5b1s_remove),
	.id_table = yacd5b1s_id,
};

static struct yacd5b1s_sensor yacd5b1s = {
	.timeperframe = {
		.numerator = 1,
		.denominator = 30,
	},
};

/**
 * yacd5b1ssensor_init - sensor driver module_init handler
 *
 * Registers driver as an i2c client driver.  Returns 0 on success,
 * error code otherwise.
 */
//extern int yacd5b1s_sensor_power_set(enum v4l2_power);
static int __init yacd5b1ssensor_init(void)
{
	int err;
//LGE_CHANGE_S [sunggyun.yu@lge.com] 2011-01-20, for factory process without sub-board
//	extern int lcd_off_boot;
//	if (lcd_off_boot) return -ENODEV;
//LGE_CHANGE_E [sunggyun.yu@lge.com] 2011-01-20, for factory process without sub-board

	err = i2c_add_driver(&yacd5b1ssensor_i2c_driver);
	if (err) {
		printk(KERN_ERR "Failed to register" YACD5B1S_DRIVER_NAME ".\n");
		return err;
	}
	return 0;
}
late_initcall(yacd5b1ssensor_init);

/**
 * yacd5b1ssensor_cleanup - sensor driver module_exit handler
 *
 * Unregisters/deletes driver as an i2c client driver.
 * Complement of yacd5b1ssensor_init.
 */
static void __exit yacd5b1ssensor_cleanup(void)
{
	i2c_del_driver(&yacd5b1ssensor_i2c_driver);
}
module_exit(yacd5b1ssensor_cleanup);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Vasko Kalanoski vkalanoski@mm-sol.com");
MODULE_DESCRIPTION("yacd5b1s camera sensor driver");
//--]] LGE_UBIQUIX_MODIFIED_END : ymjun@mnbt.co.kr [2011.07.26] - CAM : from black froyo