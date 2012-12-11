//--[[ LGE_UBIQUIX_MODIFIED_START : ymjun@mnbt.co.kr [2011.07.26] - CAM : from justin froyo
/*
 * drivers/media/video/mt9v113.c
 *
 * mt9v113 sensor driver
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

#include "mt9v113.h"
#include "mt9v113_regs.h"
#include "omap34xxcam.h"
#include "isp/isp.h"
#include "isp/ispcsi2.h"


#define MT9V113_DRIVER_NAME  "mt9v113"
#define MT9V113_MOD_NAME "MT9V113: "

#define I2C_M_WR 0

#define CTX_PWR_OFF     0
#define CTX_PWR_ON      1
#define CTX_PWR_DEFAULT CTX_PWR_OFF //OFF

#define CTX_BR_DEFAULT  6	  // 20120823 jungyeal@lge.com modify default br 7->6
#define CTX_WB_DEFAULT  0
#define CTX_EF_DEFAULT  0

#define CTX_NM_OFF      0
#define CTX_NM_ON       1
#define CTX_NM_DEFAULT  CTX_NM_OFF //OFF

struct mt9v113_status_ctx{
  u32 ctx_pwrOnState;
  u32 ctx_br;
  u32 ctx_wb;
  u32 ctx_ef;
  u32 ctx_nm;
};

/**
 * struct mt9v113_sensor - main structure for storage of sensor information
 * @pdata: access functions and data for platform level information
 * @v4l2_int_device: V4L2 device structure structure
 * @i2c_client: iic client device structure
 * @pix: V4L2 pixel format information structure
 * @timeperframe: time per frame expressed as V4L fraction
 * @scaler:
 * @ver: mt9v113 chip version
 * @fps: frames per second value
 */
struct mt9v113_sensor {
  const struct mt9v113_platform_data *pdata;
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
  bool sub_vr_mode;

  struct mt9v113_status_ctx ctx;
};

static struct mt9v113_sensor mt9v113;
static struct i2c_driver mt9v113sensor_i2c_driver;
static unsigned long xclk_current = MT9V113_XCLK_NOM_1;
static enum mt9v113_image_size isize_current = MT9V113_VGA;
//KJK. Skip size chage in case of same size
static enum mt9v113_image_size preSize = MT9V113_VGA;

/* list of image formats supported by mt9v113 sensor */
const static struct v4l2_fmtdesc mt9v113_formats[] = {
  {
    //.description = "YUYV (YUV 4:2:2), packed",
    //.pixelformat = V4L2_PIX_FMT_YUYV,
    .description = "UYVY (YUV 4:2:2), packed",		//change by MMS
    .pixelformat = V4L2_PIX_FMT_UYVY,
  }
};

#define NUM_CAPTURE_FORMATS ARRAY_SIZE(mt9v113_formats)

static enum v4l2_power current_power_state;

static struct mt9v113_clock_freq current_clk;

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
} mt9v113sensor_video_control[] = {
  {
    {
      .id = V4L2_CID_EXPOSURE,
      .type = V4L2_CTRL_TYPE_INTEGER,
      .name = "Exposure",
      .minimum = MT9V113_MIN_EXPOSURE,
      .maximum = MT9V113_MAX_EXPOSURE,
      .step = MT9V113_EXPOSURE_STEP,
      .default_value = MT9V113_DEF_EXPOSURE,
    },
    .current_value = MT9V113_DEF_EXPOSURE,
  },
  {
    {
      .id = V4L2_CID_GAIN,
      .type = V4L2_CTRL_TYPE_INTEGER,
      .name = "Analog Gain",
      .minimum = MT9V113_MIN_GAIN,
      .maximum = MT9V113_MAX_GAIN,
      .step = MT9V113_GAIN_STEP,
      .default_value = MT9V113_DEF_GAIN,
    },
    .current_value = MT9V113_DEF_GAIN,
  },

  {
    {
      .id = V4L2_CID_BRIGHTNESS,
      .type = V4L2_CTRL_TYPE_INTEGER,
      .name = "brightness",
      .minimum = MT9V113_MIN_BRIGHTNESS,
      .maximum = MT9V113_MAX_BRIGHTNESS,
      .step = MT9V113_BRIGHTNESS_STEP,
      .default_value = MT9V113_DEF_BRIGHTNESS,
    },
    .current_value = MT9V113_DEF_BRIGHTNESS,
  },
  {
    {
      .id = V4L2_CID_POWER_LINE_FREQUENCY,
      .type = V4L2_CTRL_TYPE_INTEGER,
      .name = "antibanding",
      .minimum = MT9V113_MIN_FLICKER,
      .maximum = MT9V113_MAX_FLICKER,
      .step = MT9V113_FLICKER_STEP,
      .default_value = MT9V113_DEF_FLICKER,
    },
    .current_value = MT9V113_DEF_FLICKER,
  },
  {
    {
      .id = V4L2_CID_COLORFX,
      .type = V4L2_CTRL_TYPE_INTEGER,
      .name = "effect",
      .minimum = MT9V113_MIN_COLORFX,
      .maximum = MT9V113_MAX_COLORFX,
      .step = MT9V113_COLORFX_STEP,
      .default_value = MT9V113_DEF_COLORFX,
    },
    .current_value = MT9V113_DEF_COLORFX,
  },
  {
    {
      .id = V4L2_CID_AUTO_WHITE_BALANCE,
      .type = V4L2_CTRL_TYPE_INTEGER,
      .name = "whitebalance",
      .minimum = MT9V113_MIN_WB,
      .maximum = MT9V113_MAX_WB,
      .step = MT9V113_WB_STEP,
      .default_value = MT9V113_DEF_WB,
    },
    .current_value = MT9V113_DEF_WB,
  },

  {
    {
      .id = V4L2_CID_ROTATE,
      .type = V4L2_CTRL_TYPE_INTEGER,
      .name = "mirror",
      .minimum = MT9V113_MIN_MIRROR,
      .maximum = MT9V113_MAX_MIRROR,
      .step = MT9V113_MIRROR_STEP,
      .default_value = MT9V113_DEF_MIRROR,
    },
    .current_value = MT9V113_DEF_MIRROR,
  },
  {
    {
      .id = V4L2_CID_HCENTER,//TBD
      .type = V4L2_CTRL_TYPE_INTEGER,
      .name = "meter-mode",
      .minimum = MT9V113_MIN_METERING,
      .maximum = MT9V113_MAX_METERING,
      .step = MT9V113_METERING_STEP,
      .default_value = MT9V113_DEF_METERING,
    },
    .current_value = MT9V113_DEF_METERING,
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
  {
    {
      .id = V4L2_CID_PRIVATE_OMAP3ISP_APTINA_SUB_VR,
      .type = V4L2_CTRL_TYPE_BOOLEAN,
      .name = "SubVR-mode",
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

  for (i = (ARRAY_SIZE(mt9v113sensor_video_control) - 1); i >= 0; i--)
  {
    //MT9V113_LOGD("i %d | qc.id 0x%x | id 0x%x \n",i, mt9v113sensor_video_control[i].qc.id, id);
    if (mt9v113sensor_video_control[i].qc.id == id)
      break;
  }
  if (i < 0)
    i = -EINVAL;
  return i;
}

/**
 * mt9v113_read_reg - Read a value from a register in an mt9v113 sensor device
 * @client: i2c driver client structure
 * @data_length: length of data to be read
 * @reg: register address / offset
 * @val: stores the value that gets read
 *
 * Read a value from a register in an mt9v113 sensor device.
 * The value is returned in 'val'.
 * Returns zero if successful, or non-zero otherwise.
 */
static int
mt9v113_read_reg(struct i2c_client *client, u16 data_length, u16 reg, u32 *val)
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
    MT9V113_LOGD("-mt9v113_read_reg write error\n");
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
  MT9V113_LOGD("read from offset 0x%x error %d\n", reg, err);
  MT9V113_LOGD("-mt9v113_read_reg, -1\n");
  return err;
}

/**
 * Write a value to a register in mt9v113 sensor device.
 * @client: i2c driver client structure.
 * @reg: Address of the register to read value from.
 * @val: Value to be written to a specific register.
 * Returns zero if successful, or non-zero otherwise.
 */
static int mt9v113_write_reg(struct i2c_client *client, u16 reg,
            u32 val, u16 data_length)
{
  int err = 0;
  struct i2c_msg msg[1];
  unsigned char data[6];
  int retries = 0;

  if (!client->adapter)
    return -ENODEV;

  if (data_length != I2C_16BIT)
    return -EINVAL;

retry:
  msg->addr = client->addr;
  msg->flags = I2C_M_WR;
  msg->len = data_length+2;  /* add address bytes */
  msg->buf = data;

  /* high byte goes out first */
  data[0] = (u8) (reg >> 8);
  data[1] = (u8) (reg & 0xff);
  
  data[2] = (val >> 8) & 0xff;
  data[3] = val & 0xff;

  err = i2c_transfer(client->adapter, msg, 1);

  //MT9V113_LOGD("i2c_transfer retV: %d \n",err);
  
  udelay(50);

  if (err >= 0)
    return 0;

  if (retries <= 5) {
    MT9V113_LOGD("Retrying I2C... %d \n", retries);
    retries++;
    msleep(20);
    goto retry;
  }

  return err;
}

/**
 * Initialize a list of mt9v113 registers.
 * The list of registers is terminated by the pair of values
 * {OV3640_REG_TERM, OV3640_VAL_TERM}.
 * @client: i2c driver client structure.
 * @reglist[]: List of address of the registers to write data.
 * Returns zero if successful, or non-zero otherwise.
 */
static int mt9v113_write_regs(struct i2c_client *client,
          const struct mt9v113_reg reglist[])
{
  int err = 0;
  const struct mt9v113_reg *list = reglist;

  MT9V113_FUNC_IN;
  
  while (!((list->reg == I2C_REG_TERM) && (list->val == I2C_VAL_TERM))) 
  {
    // _lkh test
    if(list->reg == 0xFFFE)
    {
        msleep(list->val);
        list++;
        continue;
    }

    err = mt9v113_write_reg(client, list->reg, list->val, list->length);

    if (err)
    {
      MT9V113_LOGD("mt9v113_write_reg ERROR %d \n", err);
      MT9V113_FUNC_OUT;
      return err;
    }
    list++;
  }

  MT9V113_FUNC_OUT;
  return 0;
}
/**
 * mt9v113_chk_modeChange - Check Mode Change
 */
int mt9v113_chk_modeChange(struct i2c_client *client, u32 reg_val)
{
  u32 mode_mon = 0xFFFFFFFF;
  int i   = 0;
  int err = 0;
  u32 local_val = reg_val;
  struct i2c_client *local_client = client;

  msleep(10);

  do
  {                           
    mt9v113_write_reg(local_client, 0x098C, local_val, REG_VAL_DATA_LEN); // MCU_ADDRESS
    mt9v113_read_reg (local_client, REG_VAL_DATA_LEN, 0x0990, &mode_mon); // SEQ_STATE
    
    if(mode_mon == 0)
    {
      MT9V113_LOGD("=====> VGA MODE Change...OK loop[%d]\n",i);
      break;
    }
    //MT9V113_LOGD("VGA MODE Change. FAIL RETRY result [%d]!!!!!\n", mode_mon);
    i++;
    msleep(10);
  }while(i < 30);

 return err;
}
/**
 * mt9v113_find_size - Find the best match for a requested image capture size
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
static enum mt9v113_image_size mt9v113_find_size(unsigned int width,
              unsigned int height)
{
  enum mt9v113_image_size isize;

  //MT9V113_LOGD(" width = %d, height = %d\n",width, height);

  for (isize = MT9V113_QCIF; isize <= MT9V113_VGA; isize++) {
    if ((mt9v113_sizes[isize].height >= height) &&
      (mt9v113_sizes[isize].width >= width)) {
      break;
    }
  }

  MT9V113_LOGD(" Req Size=%dx%d, "
      "Calc Size=%dx%d\n",
      width, height, (int)mt9v113_sizes[isize].width,
      (int)mt9v113_sizes[isize].height);

  return isize;
}

/**
 * mt9v113sensor_calc_xclk - Calculate the required xclk frequency
 *
 * Xclk is not determined from framerate for the MT9V113
 */
static unsigned long mt9v113sensor_calc_xclk(void)
{
  xclk_current = MT9V113_XCLK_NOM_1;

  return xclk_current;
}

/**
 * mt9v113sensor_set_exposure_time - sets exposure time per input value
 * @exp_time: exposure time to be set on device (in usec)
 * @s: pointer to standard V4L2 device structure
 * @lvc: pointer to V4L2 exposure entry in mt9v113sensor_video_controls array
 *
 * If the requested exposure time is within the allowed limits, the HW
 * is configured to use the new exposure time, and the
 * mt9v113sensor_video_control[] array is updated with the new current value.
 * The function returns 0 upon success.  Otherwise an error code is
 * returned.
 */
int mt9v113sensor_set_exposure_time(u32 exp_time, struct v4l2_int_device *s,
              struct vcontrol *lvc)
{
  int err = 0;
  return err;
}

/**
 * mt9v113sensor_set_gain - sets sensor analog gain per input value
 * @gain: analog gain value to be set on device
 * @s: pointer to standard V4L2 device structure
 * @lvc: pointer to V4L2 analog gain entry in mt9v113sensor_video_control array
 *
 * If the requested analog gain is within the allowed limits, the HW
 * is configured to use the new gain value, and the mt9v113sensor_video_control
 * array is updated with the new current value.
 * The function returns 0 upon success.  Otherwise an error code is
 * returned.
 */
int mt9v113sensor_set_gain(u16 lineargain, struct v4l2_int_device *s,
              struct vcontrol *lvc)
{
  int err = 0;
  return err;
}


int mt9v113sensor_set_brightness(u16 value, struct v4l2_int_device *s,
              struct vcontrol *lvc)
{
  int err = 0;
  struct mt9v113_sensor *sensor = s->priv;
  struct i2c_client *client = sensor->i2c_client;
  MT9V113_LOGD("Cur %d | Req %d ",sensor->ctx.ctx_br,value);


   // 20120823 jungyeal@lge.com check mode code moved this code here
   // Org code here...  
   
  if(sensor->ctx.ctx_br == value)
  {
    MT9V113_LOGD("SAME ! Return \n");
    return err;
  }
  else
  {
    sensor->ctx.ctx_br = value;
    MT9V113_LOGD("Different ! Chage BR %d\n",sensor->ctx.ctx_br);
  }

  // 20120823 jungyeal@lge.com check mode code moved this code here
  mt9v113_write_reg(client, 0x098C, 0xA103, REG_VAL_DATA_LEN);
  mt9v113_write_reg(client, 0x0990, 0x0005, REG_VAL_DATA_LEN);
  mt9v113_chk_modeChange(client, 0xA103);
  
  msleep(10);

  mt9v113_write_reg(client, 0x098C, 0xA103, REG_VAL_DATA_LEN);  
  mt9v113_write_reg(client, 0x0990, 0x0006, REG_VAL_DATA_LEN);  
  mt9v113_chk_modeChange(client, 0xA103);

  msleep(10);


  switch (value) {
  case  0:
    mt9v113_write_regs(client, Brightness1);
  break;
  case  1:
    mt9v113_write_regs(client, Brightness2);
  break;
  case  2:
    mt9v113_write_regs(client, Brightness3);
  break;
  case  3:
    mt9v113_write_regs(client, Brightness4);
  break;
  case  4:
    mt9v113_write_regs(client, Brightness5);
  break;
  case  5:
    mt9v113_write_regs(client, Brightness6);
  break;
  case  6:
    mt9v113_write_regs(client, Brightness7);
  break;
  case  7:
    mt9v113_write_regs(client, Brightness8);
  break;
  case  8:
    mt9v113_write_regs(client, Brightness9);
  break;
  case  9:
    mt9v113_write_regs(client, Brightness10);
  break;
  case  10:
    mt9v113_write_regs(client, Brightness11);
  break;
  case  11:
    mt9v113_write_regs(client, Brightness12);
  break;
  case  12:
    mt9v113_write_regs(client, Brightness13);
  break;

  default:
    return -EINVAL;
  }
/* Not Needed
  mt9v113_write_reg(client, 0x098C, 0xA103, REG_VAL_DATA_LEN);
  mt9v113_write_reg(client, 0x0990, 0x0006, REG_VAL_DATA_LEN);
  mt9v113_chk_modeChange(client, 0xA103);
  
  msleep(10);

  mt9v113_write_reg(client, 0x098C, 0xA103, REG_VAL_DATA_LEN);  
  mt9v113_write_reg(client, 0x0990, 0x0005, REG_VAL_DATA_LEN);  
  mt9v113_chk_modeChange(client, 0xA103);

  msleep(10);
*/
  msleep(10);
  return err;
}

int mt9v113sensor_set_flicker(u16 value, struct v4l2_int_device *s,
              struct vcontrol *lvc)
{
  int err = 0;
  struct mt9v113_sensor *sensor = s->priv;
  struct i2c_client *client = sensor->i2c_client;

  MT9V113_LOGD(" %d\n",value);
  switch (value) {
  case  0:
    mt9v113_write_regs(client, Flicker_50Hz);
  break;
  case  1:
    mt9v113_write_regs(client, Flicker_60Hz);
  break;

  default:
    return -EINVAL;
  }
  return err;
}

int mt9v113sensor_set_effect(u16 value, struct v4l2_int_device *s,
              struct vcontrol *lvc)
{
  int err = 0;
  struct mt9v113_sensor *sensor = s->priv;
  struct i2c_client *client = sensor->i2c_client;

  MT9V113_LOGD("Cur %d | Req %d ",sensor->ctx.ctx_ef,value);

  if(sensor->ctx.ctx_ef == value)
  {
    MT9V113_LOGD("SAME ! Return \n");
    return err;
  }
  else
  {
    sensor->ctx.ctx_ef = value;
    MT9V113_LOGD("Different ! Chage EF %d\n",sensor->ctx.ctx_ef);
  }

  switch (value) {
  case  0:
    mt9v113_write_regs(client, Color_Normal);
  break;
  case  1:
    mt9v113_write_regs(client, Color_Mono);
  break;
  case  2:
    mt9v113_write_regs(client, Color_Sepia);
  break;
  case  3:
    mt9v113_write_regs(client, Color_Negative);
  break;
  case  4:
    mt9v113_write_regs(client, Color_Solarize);
  break;
  default:
    return -EINVAL;
  }

  mt9v113_write_reg(client, 0x098C, 0xA103, REG_VAL_DATA_LEN);
  mt9v113_write_reg(client, 0x0990, 0x0006, REG_VAL_DATA_LEN);
  mt9v113_chk_modeChange(client, 0xA103);
  
  msleep(10);

  mt9v113_write_reg(client, 0x098C, 0xA103, REG_VAL_DATA_LEN);  
  mt9v113_write_reg(client, 0x0990, 0x0005, REG_VAL_DATA_LEN);  
  mt9v113_chk_modeChange(client, 0xA103);

  msleep(10);
  
  return err;
}

int mt9v113sensor_set_wb(u16 value, struct v4l2_int_device *s,
              struct vcontrol *lvc)
{
  int err = 0;
  struct mt9v113_sensor *sensor = s->priv;
  struct i2c_client *client = sensor->i2c_client;
  
  MT9V113_LOGD("Cur %d | Req %d ",sensor->ctx.ctx_wb,value);

  if(sensor->ctx.ctx_wb == value)
  {
    MT9V113_LOGD("SAME ! BUT KEEP GOING \n"); //if it is not, BLACK SCREEN occurs
    //return err;
  }
  else
  {
    sensor->ctx.ctx_wb = value;
    MT9V113_LOGD("Different ! Chage WB %d\n",sensor->ctx.ctx_wb);
  }

  switch (value) {
  case  0:
    mt9v113_write_regs(client, AWB_auto);
    MT9V113_LOGD("CAMERA_WB_AUTO = %d \n", value);
  break;
  case  1:
    mt9v113_write_regs(client, AWB_sun);
    MT9V113_LOGD("CAMERA_WB_DAYLIGHT = %d \n", value);
  break;
  case  2:
    mt9v113_write_regs(client, AWB_glow);
    MT9V113_LOGD("CAMERA_WB_INCANDESCENT = %d \n", value);
  break;
  case  3:
    mt9v113_write_regs(client, AWB_fluorescent);
    MT9V113_LOGD("CAMERA_WB_FLUORESCENT = %d \n", value);
  break;
  case  4:
    mt9v113_write_regs(client, AWB_cloudy);
    MT9V113_LOGD("CAMERA_WB_CLOUDY_DAYLIGHT = %d \n", value);
  break;
  default:
    return -EINVAL;
  }

  mt9v113_write_reg(client, 0x098C, 0xA103, REG_VAL_DATA_LEN);
  mt9v113_write_reg(client, 0x0990, 0x0006, REG_VAL_DATA_LEN);

  mt9v113_chk_modeChange(client, 0xA103);
  
  msleep(10);

  mt9v113_write_reg(client, 0x098C, 0xA103, REG_VAL_DATA_LEN);  
  mt9v113_write_reg(client, 0x0990, 0x0005, REG_VAL_DATA_LEN);  

  mt9v113_chk_modeChange(client, 0xA103);

  msleep(10);
  return err;
}

int mt9v113sensor_set_mirror(u16 value, struct v4l2_int_device *s,
              struct vcontrol *lvc)
{
  int err = 0;
  struct mt9v113_sensor *sensor = s->priv;
  struct i2c_client *client = sensor->i2c_client;
  MT9V113_LOGD(" %d\n",value);

  switch (value) {
  case  0:
    mt9v113_write_regs(client, Mirror_Normal);
  break;
  case  1:
    mt9v113_write_regs(client, Horizontal_Mirror);
  break;
  case  2:
    mt9v113_write_regs(client, Vertical_Flip);
  break;
  case  3:
    mt9v113_write_regs(client, Mirror_and_Flip);
  break;

  default:
    return -EINVAL;
  }

  mt9v113_write_reg(client, 0x098C, 0xA103, REG_VAL_DATA_LEN);
  mt9v113_write_reg(client, 0x0990, 0x0006, REG_VAL_DATA_LEN);

  mt9v113_chk_modeChange(client, 0xA103);

  mt9v113_write_reg(client, 0x098C, 0xA103, REG_VAL_DATA_LEN);  
  mt9v113_write_reg(client, 0x0990, 0x0005, REG_VAL_DATA_LEN);  

  mt9v113_chk_modeChange(client, 0xA103);
  msleep(100);
  return err;
}

int mt9v113sensor_set_metering(u16 value, struct v4l2_int_device *s,
              struct vcontrol *lvc)
{
  int err = 0;
  struct mt9v113_sensor *sensor = s->priv;
  struct i2c_client *client = sensor->i2c_client;
  MT9V113_LOGD(" %d\n",value);

  switch (value) {
  case  0:
    mt9v113_write_regs(client, Exposure_mode_multi);
  break;
  case  1:
    mt9v113_write_regs(client, Exposure_mode_cen);
  break;

  default:
    return -EINVAL;
  }
  return err;
}

int mt9v113sensor_set_nightmode(u16 value, struct v4l2_int_device *s,
              struct vcontrol *lvc)
{
  int err = 0;
  struct mt9v113_sensor *sensor = s->priv;
  struct i2c_client *client = sensor->i2c_client;

  if(sensor->sub_vr_mode) // if VR, go out.
    goto nightmode_out;

  MT9V113_LOGD("Cur %d | Req %d ",sensor->ctx.ctx_nm,value);

  if(sensor->ctx.ctx_nm == value)
  {
    MT9V113_LOGD("SAME ! Return \n");
    return err;
  }
  else
  {
    sensor->ctx.ctx_nm = value;
    MT9V113_LOGD("Different ! Chage NM %d \n",sensor->ctx.ctx_nm);
  }
  
  switch (value) {
  case  0:
    if (sensor->vt_mode){
      mt9v113_write_regs(client, VT_Night_mode_off); 
      MT9V113_LOGD( "VT MODE PLL\n");
    }
    else{
      mt9v113_write_regs(client, Night_mode_off);
      MT9V113_LOGD( "Normal MODE PLL\n");
    }

  break;
  case  1:
    if (sensor->vt_mode){
      mt9v113_write_regs(client, VT_Night_mode_on);
      MT9V113_LOGD( "VT night MODE PLL\n");
    }
    else{
      mt9v113_write_regs(client, Night_mode_on);
      MT9V113_LOGD( "Normal night MODE PLL\n");
    }
  break;

  default:
    return -EINVAL;
  }
  //Hansung Recommend 5->6 Just Night mode setting
  mt9v113_write_reg(client, 0x098C, 0xA103, REG_VAL_DATA_LEN);
  mt9v113_write_reg(client, 0x0990, 0x0005, REG_VAL_DATA_LEN);
  mt9v113_chk_modeChange(client, 0xA103);
  
  msleep(10);

  mt9v113_write_reg(client, 0x098C, 0xA103, REG_VAL_DATA_LEN);  
  mt9v113_write_reg(client, 0x0990, 0x0006, REG_VAL_DATA_LEN);  
  mt9v113_chk_modeChange(client, 0xA103);

  msleep(10);

nightmode_out:

  return err;
}

/**
 * mt9v113_update_clocks - calcs sensor clocks based on sensor settings.
 * @isize: image size enum
 */
int mt9v113_update_clocks(struct v4l2_int_device *s, u32 xclk, enum mt9v113_image_size isize)
{
  return 0;
}

/**
 * mt9v113_setup_pll - initializes sensor PLL registers.
 * @c: i2c client driver structure
 * @isize: image size enum
 */
int mt9v113_setup_pll(struct i2c_client *client, enum mt9v113_image_size isize)
{

  mt9v113_write_regs(client, pll_settings);
  MT9V113_LOGD( "PLL Setting\n");

  return 0;
}

/**
 * mt9v113_setup_mipi - initializes sensor & isp MIPI registers.
 * @c: i2c client driver structure
 * @isize: image size enum
 */
int mt9v113_setup_mipi(struct v4l2_int_device *s,
      enum mt9v113_image_size isize)
{
  return 0;
}

/**
 * mt9v113_configure_frame - initializes image frame registers
 * @c: i2c client driver structure
 * @isize: image size enum
 */
int mt9v113_configure_frame(struct i2c_client *client,
      enum mt9v113_image_size isize)
{
  return 0;
}

/**
 * mt9v113_init_sensor 
 *
 *
 */
int mt9v113_init_sensor(struct i2c_client *client)
{
  u32 mode_mon = 0xffffffff;
  u32 i;
  int ret = 0;

  ret = mt9v113_write_reg(client, 0x301A, 0x1218, REG_VAL_DATA_LEN);
  if (ret < 0)
    return ret;
  
  msleep(100);
  mt9v113_write_reg(client, 0x301A, 0x121C, REG_VAL_DATA_LEN);
  mt9v113_write_reg(client, 0x0018, 0x4028, REG_VAL_DATA_LEN);
  mt9v113_write_reg(client, 0x0018, 0x4028, REG_VAL_DATA_LEN); // STANDBY_CONTROL           

  //POLL_REG=0x0018, 0x4000, !=0, DELAY=10, TIMEOUT=100  
  mode_mon = 0xffffffff;
  ret = -1; //KJK for Fail Case Reset 2011.03.08
  
  //for (i=0; i<100; i++)
  for (i=0; i<10; i++)
  {               
    msleep(10);
    mt9v113_read_reg(client, REG_VAL_DATA_LEN, 0x0018, &mode_mon); // SEQ_STATE

    if(!(mode_mon & 0x4000))
    {
      MT9V113_LOGD("=====> VGA MODE CHANGE...SUCCESS loop [%d]\n",i);
      ret = 0;
      break;
    }
    MT9V113_LOGD( "=====> INIT NEXT LOOP____VGA MODE CHANGE : 0x%x !!!\n", mode_mon);
  }

  //KJK for Fail Case Reset 2011.03.08
  if(ret < 0)
  {
    MT9V113_LOGD("=====> STANDBY_CONTROL FAIL OUT !!!!!! [%d]\n",i);
    return ret;
  }

  //POLL_REG=0x301A, 0x0004, !=1, DELAY=50, TIMEOUT=20 // Verify streaming bit is high  
  mode_mon = 0xffffffff;

  for (i=0; i<20; i++)
  {               
    msleep(50);
    mt9v113_read_reg(client, REG_VAL_DATA_LEN, 0x301A, &mode_mon); // SEQ_STATE

    if((mode_mon & 0x0004))
    {
      MT9V113_LOGD("=====> VGA MODE CHANGE...SUCCESS loop [%d]\n",i);
      break;
    }
    MT9V113_LOGD( "=====> INIT NEXT LOOP____VGA MODE CHANGE : %d !!!\n", mode_mon);
  }

  mt9v113_write_reg(client, 0x001A, 0x0013, REG_VAL_DATA_LEN); // RESET_AND_MISC_CONTROL                                                                                                                                                                   
  msleep(10);
  mt9v113_write_reg(client, 0x001A, 0x0010, REG_VAL_DATA_LEN); // RESET_AND_MISC_CONTROL                                                                                                                                                                   
  msleep(10);
  mt9v113_write_reg(client, 0x0018, 0x4028, REG_VAL_DATA_LEN); // STANDBY_CONTROL                                                                                                                                                                          

  //POLL_REG=0x0018, 0x4000, !=0, DELAY=10, TIMEOUT=100 
  mode_mon = 0xffffffff;

  for (i=0; i<100; i++)
  {               
    msleep(10);
    mt9v113_read_reg(client, REG_VAL_DATA_LEN, 0x0018, &mode_mon); // SEQ_STATE

    if(!(mode_mon & 0x4000))
    {
      MT9V113_LOGD("=====> VGA MODE CHANGE...SUCCESS loop [%d]\n",i);
      break;
    }
    MT9V113_LOGD( "=====> INIT NEXT LOOP____VGA MODE CHANGE : %d !!!\n", mode_mon);
  }   

  //POLL_REG=0x301A, 0x0004, !=1, DELAY=50, TIMEOUT=20 // Verify streaming bit is high                                                                                                                             
  mode_mon = 0xffffffff;

  for (i=0; i<20; i++)
  {               
    msleep(10);
    mt9v113_read_reg(client, REG_VAL_DATA_LEN, 0x301A, &mode_mon); // SEQ_STATE

    if((mode_mon & 0x0004))
    {
      MT9V113_LOGD("=====> VGA MODE CHANGE...SUCCESS loop [%d]\n",i);
      break;
    }
    MT9V113_LOGD( "=====> INIT NEXT LOOP____VGA MODE CHANGE : %d !!!\n", mode_mon);

  } 

  //reduce_IO_current
  mt9v113_write_reg(client, 0x098C, 0x02F0, REG_VAL_DATA_LEN);  // MCU_ADDRESS
  mt9v113_write_reg(client, 0x0990, 0x0000, REG_VAL_DATA_LEN);  // MCU_DATA_0
  mt9v113_write_reg(client, 0x098C, 0x02F2, REG_VAL_DATA_LEN);  // MCU_ADDRESS
  mt9v113_write_reg(client, 0x0990, 0x0210, REG_VAL_DATA_LEN);  // MCU_DATA_0
  mt9v113_write_reg(client, 0x098C, 0x02F4, REG_VAL_DATA_LEN);  // MCU_ADDRESS
  mt9v113_write_reg(client, 0x0990, 0x001A, REG_VAL_DATA_LEN);  // MCU_DATA_0
  mt9v113_write_reg(client, 0x098C, 0x2145, REG_VAL_DATA_LEN);  // MCU_ADDRESS
  mt9v113_write_reg(client, 0x0990, 0x02F4, REG_VAL_DATA_LEN);  // MCU_DATA_0
  mt9v113_write_reg(client, 0x098C, 0xA134, REG_VAL_DATA_LEN);  // MCU_ADDRESS
  mt9v113_write_reg(client, 0x0990, 0x0001, REG_VAL_DATA_LEN);  // MCU_DATA_0
  mt9v113_write_reg(client, 0x31E0, 0x0001, REG_VAL_DATA_LEN);  // RESERVED_CORE_31E0
  mt9v113_write_reg(client, 0x31E0, 0x0001, REG_VAL_DATA_LEN);  // BITFIELD= 0x31E0 , 2, 0 // core only tags defects. SOC will correct them.                                                                                                                                      
  mt9v113_write_reg(client, 0x001A, 0x0210, REG_VAL_DATA_LEN);  // RESET_AND_MISC_CONTROL
  ret = mt9v113_write_reg(client, 0x0016, 0x42DF, REG_VAL_DATA_LEN);  // CLOCKS_CONTROL
  if (ret < 0)
    return ret;
  
  return 0;
}


/**
 * mt9v113_configure - Configure the mt9v113 for the specified image mode
 * @s: pointer to standard V4L2 device structure
 *
 * Configure the mt9v113 for a specified image size, pixel format, and frame
 * period.  xclk is the frequency (in Hz) of the xclk input to the mt9v113.
 * fper is the frame period (in seconds) expressed as a fraction.
 * Returns zero if successful, or non-zero otherwise.
 * The actual frame period is returned in fper.
 */
static int mt9v113_configure(struct v4l2_int_device *s)
{
  struct mt9v113_sensor *sensor = s->priv;
  struct v4l2_pix_format *pix = &sensor->pix;
  struct i2c_client *client = sensor->i2c_client;
  enum mt9v113_image_size isize;
  int err = 0;
  u32 temp = 0, k = 0;	//insert by MMS

  u32 w_set = 0x280;//640;
  u32 h_set = 0x1E0;//480;
  
  isize = mt9v113_find_size(pix->width, pix->height);
  isize_current = isize;
  preSize = isize;

  //KJK for STANDBY_CONTROL fail
#if 0
  while(1)
  {
    err = mt9v113_init_sensor(client);

    if (err >= 0)
    {
      MT9V113_LOGD(" ==================> mt9v113_init_sensor Success I'm Happy\n");
      err = 0;
      break;
    }
    else
    {
      MT9V113_LOGD(" ==================> mt9v113_init_sensor Fail Keep Going until die\n");
      mt9v113_power_reset(s);
    }
  }
#else
  err = mt9v113_init_sensor(client);

  if (err < 0) {
    MT9V113_LOGD(" ==================> mt9v113_init_sensor Fail Keep Going until die ^^, No Just One\n");
    mt9v113_power_reset(s);
    err = mt9v113_init_sensor(client);
  }else {
    MT9V113_LOGD(" ==================> mt9v113_init_sensor Success I'm Happy\n");
    err = 0;
  }
#endif
  mt9v113_setup_pll(client, isize);

  if(sensor->sub_vr_mode){
    mt9v113_write_regs(client, VR_core_settings);
    MT9V113_LOGD( "===== VR MODE PLL =====\n");
  }else if(sensor->vt_mode){
    mt9v113_write_regs(client, VT_core_settings);
    MT9V113_LOGD( "===== VT MODE PLL =====\n");
  }else{
    mt9v113_write_regs(client, Normal_core_settings);
    MT9V113_LOGD( "===== NORMAL MODE PLL =====\n");
  }


  if(0 == isize_current)
  {
    mt9v113_write_regs(client, set_preview_qcif_config);
    w_set = 0x00B0; //176
    h_set = 0x0090; //144;
    MT9V113_LOGD( "QCIF SET 176 144 !!!\n");
  }
  else if (1 == isize_current)
  {
    mt9v113_write_regs(client, set_preview_qvga_config);	//insert by MMS
    w_set = 0x0140; //320;
    h_set = 0x00F0; //240;
    MT9V113_LOGD( "QVGA 320 240 SET !!!\n");
  }
  else if (2 == isize_current)
  {
    mt9v113_write_regs(client, set_preview_vga_config);		//insert by MMS
    w_set = 0x0280; //640;
    h_set = 0x01E0; //480;
    MT9V113_LOGD( "VGA 640 480 SET !!!\n");
  }
  else
  {
    mt9v113_write_regs(client, set_preview_vga_config);		//insert by MMS
    MT9V113_LOGD( "Configure Mode Not Supported : %d !!!\n", isize_current);
    msleep(30);
  }
    msleep(30);		//insert by MMS
#if 0	//Remove by MMS
  mt9v113_write_reg(client, 0x098C, 0x2703, REG_VAL_DATA_LEN);  // Output Width (A) 
  mt9v113_write_reg(client, 0x0990, w_set,  REG_VAL_DATA_LEN);  //      = 640 
  mt9v113_write_reg(client, 0x098C, 0x2705, REG_VAL_DATA_LEN);  // Output Height 
  mt9v113_write_reg(client, 0x0990, h_set,  REG_VAL_DATA_LEN);  //      = 480
  
  mt9v113_write_reg(client, 0x098C, 0x2707, REG_VAL_DATA_LEN);  // Output Width (B) 
  mt9v113_write_reg(client, 0x0990, w_set,  REG_VAL_DATA_LEN);  //      = 640 
  mt9v113_write_reg(client, 0x098C, 0x2709, REG_VAL_DATA_LEN);  // Output Height 
  mt9v113_write_reg(client, 0x0990, h_set,  REG_VAL_DATA_LEN);  //      = 480
 
//  mt9v113_write_reg(client, 0x098C, 0xA103, REG_VAL_DATA_LEN); 
//  mt9v113_write_reg(client, 0x0990, 0x0001, REG_VAL_DATA_LEN); // Go to Preview Mode
//  MT9V113_LOGD( "Go to Preveiw Mode\n");

  mt9v113_write_reg(client, 0x098C, 0xA103, REG_VAL_DATA_LEN); 
  mt9v113_write_reg(client, 0x098C, 0x0006, REG_VAL_DATA_LEN);
  mt9v113_chk_modeChange(client, 0xA103);
  msleep(10);  

  mt9v113_write_reg(client, 0x098C, 0xA103, REG_VAL_DATA_LEN); 
  mt9v113_write_reg(client, 0x098C, 0x0005, REG_VAL_DATA_LEN);
  mt9v113_chk_modeChange(client, 0xA103);
  //msleep(150);
  msleep(10);
#endif
  return err;
}
/**
 * mt9v113_mode_change - Configure the mt9v113 for capture size change
 * @s: pointer to standard V4L2 device structure
 *
 * 
 * 
 * 
 * 
 * 
 */
static int mt9v113_mode_change(struct v4l2_int_device *s)
{
  struct mt9v113_sensor *sensor = s->priv;
  struct v4l2_pix_format *pix = &sensor->pix;
  struct i2c_client *client = sensor->i2c_client;
  enum mt9v113_image_size isize;
  int err = 0;
  u32 w_set = 0x280;//640;
  u32 h_set = 0x1E0;//480;

  isize = mt9v113_find_size(pix->width, pix->height);

  if(preSize == isize) //If same, go return, not change
  {
    MT9V113_LOGD("Previous %d == Reqest %d, DONT Change Size\n",preSize,isize);
    return err;
  }
  isize_current = isize;
  preSize = isize;

  if(0 == isize_current)
  {
    mt9v113_write_regs(client, set_preview_qcif_config);	//insert by MMS
    w_set = 0x00B0; //176
    h_set = 0x0090; //144;
    MT9V113_LOGD( "QCIF SET 176 144 !!!\n");
  }
  else if (1 == isize_current)
  {
    mt9v113_write_regs(client, set_preview_qvga_config);	//insert by MMS
    w_set = 0x0140; //320;
    h_set = 0x00F0; //240;
    MT9V113_LOGD( "QVGA 320 240 SET !!!\n");
  }
  else if (2 == isize_current)
  {
    mt9v113_write_regs(client, set_preview_vga_config);	//insert by MMS
    w_set = 0x0280; //640;
    h_set = 0x01E0; //480;
    MT9V113_LOGD( "VGA 640 480 SET !!!\n");
  }
  else
  {
    mt9v113_write_regs(client, set_preview_vga_config);	//insert by MMS
    MT9V113_LOGD( "Configure Mode Not Supported : %d !!!\n", isize_current);
    msleep(30);
  }
    msleep(30);	//insert by MMS
#if 0	//Remove by MMS
  mt9v113_write_reg(client, 0x098C, 0x2703, REG_VAL_DATA_LEN);  // Output Width (A) 
  mt9v113_write_reg(client, 0x0990, w_set,  REG_VAL_DATA_LEN);  //      = 640 
  mt9v113_write_reg(client, 0x098C, 0x2705, REG_VAL_DATA_LEN);  // Output Height 
  mt9v113_write_reg(client, 0x0990, h_set,  REG_VAL_DATA_LEN);  //      = 480
  
  mt9v113_write_reg(client, 0x098C, 0x2707, REG_VAL_DATA_LEN);  // Output Width (B) 
  mt9v113_write_reg(client, 0x0990, w_set,  REG_VAL_DATA_LEN);  //      = 640 
  mt9v113_write_reg(client, 0x098C, 0x2709, REG_VAL_DATA_LEN);  // Output Height 
  mt9v113_write_reg(client, 0x0990, h_set,  REG_VAL_DATA_LEN);  //      = 480
 
  MT9V113_LOGD( "Change Preview Size\n");

  mt9v113_write_reg(client, 0x098C, 0xA103, REG_VAL_DATA_LEN);
  mt9v113_write_reg(client, 0x0990, 0x0006, REG_VAL_DATA_LEN);
  mt9v113_chk_modeChange(client, 0xA103);
  msleep(10);

  mt9v113_write_reg(client, 0x098C, 0xA103, REG_VAL_DATA_LEN);  
  mt9v113_write_reg(client, 0x0990, 0x0005, REG_VAL_DATA_LEN);  
  mt9v113_chk_modeChange(client, 0xA103);
  msleep(10);
#endif 
  return err;
}
/**
 * mt9v113_detect - Detect if an mt9v113 is present, and if so which revision
 * @client: pointer to the i2c client driver structure
 * Detect if an mt9v113 is present, and if so which revision.
 * A device is considered to be detected if the manufacturer ID (MIDH and MIDL)
 * and the product ID (PID) registers match the expected values.
 * Any value of the version ID (VER) register is accepted.
 * Returns a negative error number if no device is detected, or the
 * non-negative value of the version ID register if a device is detected.
 */
static int mt9v113_detect(struct i2c_client *client)
{
  //u32 model_id = 0;  
  u32 rev = 0;
  struct mt9v113_sensor *sensor;
  dev_err(&client->dev, "client.addr = 0x%x \n", client->addr);

  if (!client)
    return -ENODEV;

  sensor = i2c_get_clientdata(client);
#if 0
  mt9v113_write_reg(client, 0x0028, 0x7000, I2C_16BIT);
  mt9v113_write_reg(client, 0x002A, MT9V113_REG_MODEL_ID, I2C_16BIT);

  if (mt9v113_read_reg(client, I2C_16BIT, 0x0F12, &model_id))
      return -ENODEV;

  
  dev_info(&client->dev, "model id detected, 0x%x\n", model_id);

  if (model_id != MT9V113_FW_SENSOR_ID) {
    
    dev_warn(&client->dev, "model id mismatch, 0x%x\n", model_id);

    return -ENODEV;
  }
#endif

  return rev;
}

#if 0 // Remove Warning
/**
 * Sets the correct orientation based on the sensor version.
 *   IU046F2-Z   version=2  orientation=3
 *   IU046F4-2D  version>2  orientation=0
 */
static int mt9v113_set_orientation(struct i2c_client *client, u32 ver)
{
  int err = 0;
  return err;
}

/**
 * Set CSI2 Virtual ID.
 * @client: i2c client driver structure
 * @id: Virtual channel ID.
 *
 * Sets the channel ID which identifies data packets sent by this device
 * on the CSI2 bus.
 **/
static int mt9v113_set_virtual_id(struct i2c_client *client, u32 id)
{
  return 0;
}


/**
 * mt9v113_set_framerate - Sets framerate by adjusting frame_length_lines reg.
 * @s: pointer to standard V4L2 device structure
 * @fper: frame period numerator and denominator in seconds
 *
 * The maximum exposure time is also updated since it is affected by the
 * frame rate.
 **/
static int mt9v113_set_framerate(struct v4l2_int_device *s,
            struct v4l2_fract *fper)
{
  int err = 0;
  return err;
}
#endif
/**
 * ioctl_queryctrl - V4L2 sensor interface handler for VIDIOC_QUERYCTRL ioctl
 * @s: pointer to standard V4L2 device structure
 * @qc: standard V4L2 VIDIOC_QUERYCTRL ioctl structure
 *
 * If the requested control is supported, returns the control information
 * from the mt9v113sensor_video_control[] array.
 * Otherwise, returns -EINVAL if the control is not supported.
 */
static int ioctl_queryctrl(struct v4l2_int_device *s,
        struct v4l2_queryctrl *qc)
{
  int i;

  MT9V113_LOGD("+ioctl_queryctrl\n");
  
  i = find_vctrl(qc->id);
  if (i == -EINVAL)
    qc->flags = V4L2_CTRL_FLAG_DISABLED;

  if (i < 0)
    return -EINVAL;

  *qc = mt9v113sensor_video_control[i].qc;
  return 0;
}

/**
 * ioctl_g_ctrl - V4L2 sensor interface handler for VIDIOC_G_CTRL ioctl
 * @s: pointer to standard V4L2 device structure
 * @vc: standard V4L2 VIDIOC_G_CTRL ioctl structure
 *
 * If the requested control is supported, returns the control's current
 * value from the mt9v113sensor_video_control[] array.
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
  
  lvc = &mt9v113sensor_video_control[i];

  switch (vc->id) 
  {
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
    case V4L2_CID_PRIVATE_OMAP3ISP_APTINA_SUB_VR:
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
 * value in HW (and updates the mt9v113sensor_video_control[] array).
 * Otherwise, * returns -EINVAL if the control is not supported.
 */
static int ioctl_s_ctrl(struct v4l2_int_device *s,
           struct v4l2_control *vc)
{
  struct mt9v113_sensor *sensor = s->priv;
  int retval = -EINVAL;
  int i;
  struct vcontrol *lvc;

  MT9V113_FUNC_IN;

  i = find_vctrl(vc->id);

  //MT9V113_LOGD("vd->id 0x%x | find_vctrl %d \n", vc->id, i);
  
  if (i < 0)
    return -EINVAL;

  /* 20120605 jungyeal@lge.com Workaround I2C write error when cam power state off [START] */	
  if (sensor->ctx.ctx_pwrOnState  == CTX_PWR_OFF && 
  	vc->id != V4L2_CID_PRIVATE_OMAP3ISP_HYNIX_SMART_CAMERA_VT && 
	vc->id != V4L2_CID_PRIVATE_OMAP3ISP_HYNIX_SMART_CAMERA && 
	vc->id != V4L2_CID_PRIVATE_OMAP3ISP_APTINA_SUB_VR 
  )
  {
  	MT9V113_LOGD("Warning Cam power state is OFF  !!!\n");
	return -EINVAL;
  }
  /* 20120605 jungyeal@lge.com Workaround I2C write error when cam power state off  [START] */
  
  lvc = &mt9v113sensor_video_control[i];

  switch (vc->id) 
  {
    case V4L2_CID_EXPOSURE:
      retval = mt9v113sensor_set_exposure_time(vc->value, s, lvc);
      break;
    case V4L2_CID_GAIN:
      retval = mt9v113sensor_set_gain(vc->value, s, lvc);
      break;
    case V4L2_CID_BRIGHTNESS:
      retval = mt9v113sensor_set_brightness(vc->value, s, lvc);
      break;
    case V4L2_CID_POWER_LINE_FREQUENCY://flicker
      retval = mt9v113sensor_set_flicker(vc->value, s, lvc);
      break;
    case V4L2_CID_COLORFX://effect
      retval = mt9v113sensor_set_effect(vc->value, s, lvc);
      break;
    case V4L2_CID_AUTO_WHITE_BALANCE://manual white balance
      retval = mt9v113sensor_set_wb(vc->value, s, lvc);
      break;
    case V4L2_CID_ROTATE://mirror
      retval = mt9v113sensor_set_mirror(vc->value, s, lvc);
      break;
    case V4L2_CID_HCENTER://metering
      retval = mt9v113sensor_set_metering(vc->value, s, lvc);
      break;
    case V4L2_CID_PRIVATE_OMAP3ISP_HYNIX_SMART_CAMERA:
      sensor->reset_camera = vc->value ? true : false;
      lvc->current_value = vc->value;
      retval = 0;
      break;
    case V4L2_CID_VCENTER: //night mode
      retval = mt9v113sensor_set_nightmode(vc->value, s, lvc);
      break;
    case V4L2_CID_PRIVATE_OMAP3ISP_HYNIX_SMART_CAMERA_VT:
      sensor->vt_mode = vc->value ? true : false;
      lvc->current_value = vc->value;
      retval = 0;

      sensor->sub_vr_mode = false; //KJK mode reset 
      break;
    case V4L2_CID_PRIVATE_OMAP3ISP_APTINA_SUB_VR: //Sub VR/Fix 15FPS
      sensor->sub_vr_mode = vc->value ? true : false;
      lvc->current_value = vc->value;
      retval = 0;

      sensor->vt_mode = false; //KJK mode reset
      break;
    default:
      MT9V113_LOGD("Not Supported V4L2 CID!!!! ID: %d \n",vc->id);
      break;
  }
  
  MT9V113_FUNC_OUT;
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

  switch (fmt->type) 
  {
    case V4L2_BUF_TYPE_VIDEO_CAPTURE:
      if (index >= NUM_CAPTURE_FORMATS)
        return -EINVAL;
    break;
    default:
      return -EINVAL;
  }

  fmt->flags = mt9v113_formats[index].flags;
  strlcpy(fmt->description, mt9v113_formats[index].description,
          sizeof(fmt->description));
  fmt->pixelformat = mt9v113_formats[index].pixelformat;

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
  enum mt9v113_image_size isize;
  int ifmt;
  struct v4l2_pix_format *pix = &f->fmt.pix;
  struct mt9v113_sensor *sensor = s->priv;
  struct v4l2_pix_format *pix2 = &sensor->pix;

  isize = mt9v113_find_size(pix->width, pix->height);
  isize_current = isize;

  pix->width = mt9v113_sizes[isize].width;
  pix->height = mt9v113_sizes[isize].height;
  MT9V113_LOGD("ioctl_try_fmt_cap w = %d, h = %d\n", pix->width, pix->height);

  for (ifmt = 0; ifmt < NUM_CAPTURE_FORMATS; ifmt++) 
  {
    if (pix->pixelformat == mt9v113_formats[ifmt].pixelformat)
      break;
  }

  if (ifmt == NUM_CAPTURE_FORMATS)
    ifmt = 0;

  pix->pixelformat = mt9v113_formats[ifmt].pixelformat;
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
  struct mt9v113_sensor *sensor = s->priv;
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
  struct mt9v113_sensor *sensor = s->priv;
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
  //struct mt9v113_sensor *sensor = s->priv;

  //pix->width = sensor->pix.width;
  //pix->height = sensor->pix.height;
  if(0 == isize_current){
    pix->left   = 0;
    pix->top    = 0;
    pix->width  = 176;
    pix->height = 144;
  }else if (1 == isize_current){
    pix->left   = 0;
    pix->top    = 0;
    pix->width  = 320;
    pix->height = 240;
  }else{
    pix->left    = 0;
    pix->top    = 0;
    pix->width  = 640;
    pix->height = 480;
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
  pix->left = 0;
  pix->top  = 0;
  pix->width  = mt9v113_sizes[NUM_IMAGE_SIZES - 1].width;
  pix->height = mt9v113_sizes[NUM_IMAGE_SIZES - 1].height;
  return 0;
}

/**
 * ioctl_g_pixelsize - V4L2 sensor interface handler for ioctl_g_pixelsize
 * @s: pointer to standard V4L2 device structure
 * @pix: pointer to standard V4L2 v4l2_pix_format structure
 *
 * Returns the sensor's configure pixel size. Just For Sub VT Rotation
 */
static int ioctl_priv_g_pixelsize(struct v4l2_int_device *s,
          struct v4l2_rect *pix)
{
  //struct yacd5b1s_frame_settings *frm;

  if(0 == isize_current){
    pix->left   = 0;
    pix->top    = 0;
    pix->width  = 2;
    pix->height = 2;
  }else if (1 == isize_current){
    pix->left   = 0;
    pix->top    = 0;
    pix->width  = 2;
    pix->height = 2;
  }else if (2 == isize_current){
    pix->left   = 0;
    pix->top    = 0;
    pix->width  = 2;
    pix->height = 2;
  }else if (3 == isize_current){
    pix->left   = 0;
    pix->top    = 0;
    pix->width  = 1;
    pix->height = 1;
  }else{
    pix->left   = 0;
    pix->top    = 0;
    pix->width  = 1;
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
  struct mt9v113_sensor *sensor = s->priv;
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
  struct mt9v113_sensor *sensor = s->priv;
  struct v4l2_fract *timeperframe = &a->parm.capture.timeperframe;
  int err = 0;

  sensor->timeperframe = *timeperframe;
  mt9v113sensor_calc_xclk();
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
  struct mt9v113_sensor *sensor = s->priv;

  return sensor->pdata->priv_data_set(s, p);

}

static int __mt9v113_power_off_standby(struct v4l2_int_device *s,
              enum v4l2_power on)
{
  struct mt9v113_sensor *sensor = s->priv;
  //struct i2c_client *client = sensor->i2c_client;
  int rval;

  rval = sensor->pdata->power_set(s, on);
  if (rval < 0) {
    MT9V113_LOGD(">>>>>Unable to set the power state: STANDBY\n");
    return rval;
  }

  sensor->pdata->set_xclk(s, 0);
  return 0;
}

static int mt9v113_power_off(struct v4l2_int_device *s)
{
  return __mt9v113_power_off_standby(s, V4L2_POWER_OFF);
}

static int mt9v113_power_standby(struct v4l2_int_device *s)
{
  return 0; //Compile Waring Remove
  //return __mt9v113_power_off_standby(s, V4L2_POWER_STANDBY);
}

static int mt9v113_power_on(struct v4l2_int_device *s)
{
  struct mt9v113_sensor *sensor = s->priv;
  //struct i2c_client *client = sensor->i2c_client;
  int rval;

  MT9V113_FUNC_IN;
  sensor->pdata->set_xclk(s, xclk_current);

  rval = sensor->pdata->power_set(s, V4L2_POWER_ON);
  
  if (rval < 0) 
  {
    MT9V113_LOGD(">>>>>>Unable to set the power state: ON\n");

    sensor->pdata->set_xclk(s, 0);

    MT9V113_FUNC_OUT;
    return rval;
  }
  MT9V113_FUNC_OUT;
  return 0;
}
//KJK Add Reset Function 2011.03.08
static int mt9v113_power_reset(struct v4l2_int_device *s)
{
  int retV = 0;

  MT9V113_FUNC_IN;

  msleep(10);
  retV = mt9v113_power_off(s);
  if (retV < 0)
  {
    MT9V113_LOGD("power reset off seq. fail \n");
  }

  msleep(10);
  retV = mt9v113_power_on(s);
  if (retV < 0)
  {
    MT9V113_LOGD("power reset on seq. fail \n");
  }

  msleep(10);
  MT9V113_FUNC_OUT;
  return retV;
}
/**
 * ioctl_s_power - V4L2 sensor interface handler for vidioc_int_s_power_num
 * @s: pointer to standard V4L2 device structure
 * @on: power state to which device is to be set
 *
 * Sets devices power state to requrested state, if possible.
 */
static int ioctl_s_power(struct v4l2_int_device *s, enum v4l2_power on)
#if 0
{
  struct mt9v113_sensor *sensor = s->priv;
  struct i2c_client *client = sensor->i2c_client;
  struct omap34xxcam_hw_config hw_config;
  struct vcontrol *lvc;
  int i;

  MT9V113_FUNC_IN;
  
  switch (on) 
  {
    case V4L2_POWER_ON:
      if (!sensor->reset_camera){
        MT9V113_LOGD("[PWR ON] reset_camera is false\n");
        mt9v113_power_on(s);
        mt9v113_configure(s);
      }
      else
        MT9V113_LOGD("[PWR ON] reset_camera is TRUE SKIP!!!!\n");
        
      if (current_power_state == V4L2_POWER_STANDBY){
        sensor->resuming = true;
        mt9v113_mode_change(s);
        MT9V113_LOGD("[PWR ON] Previous is V4L2_POWER_STANDBY | Just Change Size.\n");
      }

      break;
    case V4L2_POWER_OFF:
      //if (!sensor->reset_camera){ //KJK for sleep
      if(1){
        MT9V113_LOGD("[PWR OFF] reset_camera is false : DO PWR OFF\n");
        mt9v113_power_off(s);
      }
      else
        MT9V113_LOGD("[PWR OFF] reset_camera is true  : DO NOT PWR OFF\n");

      /* Reset defaults for controls */
      i = find_vctrl(V4L2_CID_GAIN);
      if (i >= 0) {
        lvc = &mt9v113sensor_video_control[i];
        lvc->current_value = MT9V113_DEF_GAIN;
      }
      
      i = find_vctrl(V4L2_CID_EXPOSURE);
      if (i >= 0) {
        lvc = &mt9v113sensor_video_control[i];
        lvc->current_value = MT9V113_DEF_EXPOSURE;
      }
      break;
    case V4L2_POWER_STANDBY:
      if (!sensor->reset_camera){ 
        MT9V113_LOGD("[PWR STANDBY] reset_camera is false : DO STANDBY\n");
        mt9v113_power_standby(s);
      }
      else{
        MT9V113_LOGD("[PWR STANDBY] reset_camera is true  : DO NOT STANDBY\n");
      }
      break;
  }

  sensor->resuming = false;
  current_power_state = on;

  MT9V113_FUNC_OUT;
  return 0;
}
#else
{
  struct mt9v113_sensor *sensor = s->priv;
  //struct i2c_client *client = sensor->i2c_client;
  //struct omap34xxcam_hw_config hw_config;
  struct vcontrol *lvc;
  int i;

  MT9V113_FUNC_IN;
  
  switch (on) 
  {
    case V4L2_POWER_ON:
      if (sensor->ctx.ctx_pwrOnState == CTX_PWR_OFF){
        MT9V113_LOGD("[PWR ON] \n");
        mt9v113_power_on(s);
        mt9v113_configure(s);
  msleep(300);      
        sensor->ctx.ctx_pwrOnState = CTX_PWR_ON;
        sensor->ctx.ctx_br = CTX_BR_DEFAULT;
        sensor->ctx.ctx_wb = CTX_WB_DEFAULT;
        sensor->ctx.ctx_ef = CTX_EF_DEFAULT;
        sensor->ctx.ctx_nm = CTX_NM_DEFAULT;
      }
      else
        MT9V113_LOGD("[PWR ON] NOW ON, Skip.\n");
        
      if (current_power_state == V4L2_POWER_STANDBY){
        sensor->resuming = true;
        mt9v113_mode_change(s);
        MT9V113_LOGD("[PWR ON] Previous is V4L2_POWER_STANDBY | Just Change Size.\n");
      }

      break;
    case V4L2_POWER_OFF:
      MT9V113_LOGD("[PWR OFF] DO PWR OFF\n");
      mt9v113_power_off(s);
      
      sensor->ctx.ctx_pwrOnState = CTX_PWR_OFF;
      sensor->ctx.ctx_br = CTX_BR_DEFAULT;
      sensor->ctx.ctx_wb = CTX_WB_DEFAULT;
      sensor->ctx.ctx_ef = CTX_EF_DEFAULT;
      sensor->ctx.ctx_nm = CTX_NM_DEFAULT;
      
      /* Reset defaults for controls */
      i = find_vctrl(V4L2_CID_GAIN);
      if (i >= 0) {
        lvc = &mt9v113sensor_video_control[i];
        lvc->current_value = MT9V113_DEF_GAIN;
      }
      
      i = find_vctrl(V4L2_CID_EXPOSURE);
      if (i >= 0) {
        lvc = &mt9v113sensor_video_control[i];
        lvc->current_value = MT9V113_DEF_EXPOSURE;
      }
      break;
    case V4L2_POWER_STANDBY:
      mt9v113_power_standby(s);
      MT9V113_LOGD("[PWR STANDBY] Nothing \n");
      break;
  }

  sensor->resuming = false;
  current_power_state = on;
  MT9V113_LOGD("Change current power state [%d] => [%d]\n",current_power_state,on);

  MT9V113_FUNC_OUT;
  return 0;
}
#endif
/**
 * ioctl_init - V4L2 sensor interface handler for VIDIOC_INT_INIT
 * @s: pointer to standard V4L2 device structure
 *
 * Initialize the sensor device (call mt9v113_configure())
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
 * mt9v113 device could be found, otherwise returns appropriate error.
 */
static int ioctl_dev_init(struct v4l2_int_device *s)
{
  struct mt9v113_sensor *sensor = s->priv;
  struct i2c_client *client = sensor->i2c_client;
  int err;

  err = mt9v113_power_on(s);
  if (err)
    return -ENODEV;

  err = mt9v113_detect(client);
  if (err < 0) {
    MT9V113_LOGD("Unable to detect sensor\n");

    /*
     * Turn power off before leaving the function.
     * If not, CAM Pwrdm will be ON which is not needed
     * as there is no sensor detected.
     */
    mt9v113_power_off(s);

    return err;
  }
  sensor->ver = err;
  MT9V113_LOGD("chip version 0x%02x detected\n", sensor->ver);

  err = mt9v113_power_off(s);
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
    if (frms->pixel_format == mt9v113_formats[ifmt].pixelformat)
      break;
  }
  /* Is requested pixelformat not found on sensor? */
    MT9V113_LOGD("ioctl_enum_framesizesm ifmt = %d, frms->index = %d\n",ifmt,frms->index );
  if (ifmt == NUM_CAPTURE_FORMATS)
    return -EINVAL;

  /* Check that the index we are being asked for is not
     out of bounds. */
  if (frms->index >= ARRAY_SIZE(mt9v113_sizes))
    return -EINVAL;

  frms->type = V4L2_FRMSIZE_TYPE_DISCRETE;
  frms->discrete.width = mt9v113_sizes[frms->index].width;
  frms->discrete.height = mt9v113_sizes[frms->index].height;

  return 0;
}

const struct v4l2_fract mt9v113_frameintervals[] = {
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
    if (frmi->pixel_format == mt9v113_formats[ifmt].pixelformat)
      break;
  }

  if (ifmt == NUM_CAPTURE_FORMATS)
    return -EINVAL;

  /* Check that the index we are being asked for is not
     out of bounds. */
  if (frmi->index >= ARRAY_SIZE(mt9v113_frameintervals))
    return -EINVAL;

  /* Make sure that the 2MP size reports a max of 10fps */
  if (frmi->width == 1600 && frmi->height == 1200) {
    if (frmi->index != 0)
      return -EINVAL;
  }

  frmi->type = V4L2_FRMIVAL_TYPE_DISCRETE;
  frmi->discrete.numerator =
        mt9v113_frameintervals[frmi->index].numerator;
  frmi->discrete.denominator =
        mt9v113_frameintervals[frmi->index].denominator;

  return 0;
}

static struct v4l2_int_ioctl_desc mt9v113_ioctl_desc[] = {
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

static struct v4l2_int_slave mt9v113_slave = {
  .ioctls = mt9v113_ioctl_desc,
  .num_ioctls = ARRAY_SIZE(mt9v113_ioctl_desc),
};

static struct v4l2_int_device mt9v113_int_device = {
  .module = THIS_MODULE,
  .name = MT9V113_DRIVER_NAME,
  .priv = &mt9v113,
  .type = v4l2_int_type_slave,
  .u = {
    .slave = &mt9v113_slave,
  },
};

/**
 * mt9v113_probe - sensor driver i2c probe handler
 * @client: i2c driver client device structure
 *
 * Register sensor as an i2c client device and V4L2
 * device.
 */
static int __devinit mt9v113_probe(struct i2c_client *client,
           const struct i2c_device_id *id)
{
  struct mt9v113_sensor *sensor = &mt9v113;
  int err;

  MT9V113_FUNC_IN;

  if (i2c_get_clientdata(client))
  {
    MT9V113_LOGD("i2c_get_clientdata ERROR\n");
    MT9V113_FUNC_OUT;
    return -EBUSY;
  }
  sensor->pdata = client->dev.platform_data;

  if (!sensor->pdata) {
    MT9V113_LOGD("no platform data?\n");
    MT9V113_FUNC_OUT;
    return -ENODEV;
  }

  sensor->v4l2_int_device = &mt9v113_int_device;
  sensor->i2c_client = client;

  i2c_set_clientdata(client, sensor);

  /* Make the default capture format QCIF V4L2_PIX_FMT_UYVY */
  sensor->pix.width = MT9V113_IMAGE_WIDTH_MAX;
  sensor->pix.height = MT9V113_IMAGE_HEIGHT_MAX;
  //sensor->pix.pixelformat = V4L2_PIX_FMT_YUYV;
  sensor->pix.pixelformat = V4L2_PIX_FMT_UYVY;

  /* Enable reset secondary camera default */
  sensor->reset_camera = false;
  sensor->vt_mode      = false;
  sensor->sub_vr_mode  = false;

  // KJK. CTX Init.
  sensor->ctx.ctx_pwrOnState = CTX_PWR_DEFAULT;
  sensor->ctx.ctx_br = CTX_BR_DEFAULT;
  sensor->ctx.ctx_wb = CTX_WB_DEFAULT;
  sensor->ctx.ctx_ef = CTX_EF_DEFAULT;
  sensor->ctx.ctx_nm = CTX_NM_DEFAULT;
  
  err = v4l2_int_device_register(sensor->v4l2_int_device);

  if (err)
  {
    MT9V113_LOGD("mt9v113_probe error\n");
    i2c_set_clientdata(client, NULL);
  }

  MT9V113_LOGD("mt9v113_probe success\n");

  MT9V113_FUNC_OUT;
  return 0;
}

/**
 * mt9v113_remove - sensor driver i2c remove handler
 * @client: i2c driver client device structure
 *
 * Unregister sensor as an i2c client device and V4L2
 * device.  Complement of mt9v113_probe().
 */
static int __exit
mt9v113_remove(struct i2c_client *client)
{
  struct mt9v113_sensor *sensor = i2c_get_clientdata(client);

  if (!client->adapter)
    return -ENODEV; /* our client isn't attached */

  v4l2_int_device_unregister(sensor->v4l2_int_device);
  i2c_set_clientdata(client, NULL);

  return 0;
}

static const struct i2c_device_id mt9v113_id[] = {
  { MT9V113_DRIVER_NAME, 0 },
  { },
};
MODULE_DEVICE_TABLE(i2c, mt9v113_id);

static struct i2c_driver mt9v113sensor_i2c_driver = {
  .driver = {
    .name = MT9V113_DRIVER_NAME,
    .owner = THIS_MODULE,
  },
  .probe = mt9v113_probe,
  .remove = __exit_p(mt9v113_remove),
  .id_table = mt9v113_id,
};

static struct mt9v113_sensor mt9v113 = {
  .timeperframe = {
    .numerator = 1,
    .denominator = 30,
  },
};

/**
 * mt9v113sensor_init - sensor driver module_init handler
 *
 * Registers driver as an i2c client driver.  Returns 0 on success,
 * error code otherwise.
 */
//extern int mt9v113_sensor_power_set(enum v4l2_power);
static int __init mt9v113sensor_init(void)
{
  int err;

  MT9V113_FUNC_IN;

  err = i2c_add_driver(&mt9v113sensor_i2c_driver);

  MT9V113_LOGD("i2c_add_driver return %d\n",err);
  
  if (err) {
    MT9V113_LOGD( "Failed to register\n");
    return err;
  }

  MT9V113_FUNC_OUT;
  return 0;
}
late_initcall(mt9v113sensor_init);

/**
 * mt9v113sensor_cleanup - sensor driver module_exit handler
 *
 * Unregisters/deletes driver as an i2c client driver.
 * Complement of mt9v113sensor_init.
 */
static void __exit mt9v113sensor_cleanup(void)
{
  i2c_del_driver(&mt9v113sensor_i2c_driver);
}
module_exit(mt9v113sensor_cleanup);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Vasko Kalanoski vkalanoski@mm-sol.com");
MODULE_DESCRIPTION("mt9v113 camera sensor driver");
//--]] LGE_UBIQUIX_MODIFIED_END : ymjun@mnbt.co.kr [2011.07.26] - CAM : from justin froyo