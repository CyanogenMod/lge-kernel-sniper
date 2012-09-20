//--[[ LGE_UBIQUIX_MODIFIED_START : ymjun@mnbt.co.kr [2011.07.26] - CAM : from justin froyo
/*
 * drivers/media/video/mt9v113.h
 *
 * Register definitions for the mt9v113 Sensor.
 *
 * This file is licensed under the terms of the GNU General Public License
 * version 2. This program is licensed "as is" without any warranty of any
 * kind, whether express or implied.
 */

#ifndef MT9V113_H
#define MT9V113_H

#define MT9V113_I2C_ADDR    0x7A>>1 //0x20 //0x78>>1

#define VAUX_2_8_V          0x09
#define VAUX_1_8_V          0x05
#define VAUX_DEV_GRP_P1     0x20
#define VAUX_DEV_GRP_NONE   0x00

/* mt9v113 has 8/16/32 I2C registers */
#define I2C_8BIT      1
#define I2C_16BIT     2
#define I2C_32BIT     4

/* Terminating list entry for reg */
#define I2C_REG_TERM    0xFFFF
/* Terminating list entry for val */
#define I2C_VAL_TERM    0xFFFFFFFF
/* Terminating list entry for len */
#define I2C_LEN_TERM    0xFFFF

/* Average black level */
#define MT9V113_BLACK_LEVEL_AVG 64

/*============================================================================
                  SENSOR REGISTER DEFINES
=============================================================================*/
#define MT9V113_REG_MODEL_ID  0x0152

#define MT9V113_FW_SENSOR_ID  0xF018

#define REG_VAL_DATA_LEN 2

#define BIT0 0x00000001
#define BIT1 0x00000002
#define BIT2 0x00000004
#define BIT3 0x00000008
#define BIT4 0x00000010
#define BIT5 0x00000020
#define BIT6 0x00000040
#define BIT7 0x00000080
#define BIT8 0x00000100
#define BIT9 0x00000200

/*
 * The nominal xclk input frequency of the mt9v113 is 24MHz, maximum
 * frequency is 45MHz, and minimum frequency is 6MHz.
 */
#define MT9V113_XCLK_MIN    6000000
#define MT9V113_XCLK_MAX    45000000
#define MT9V113_XCLK_NOM_1  24000000
//#define MT9V113_XCLK_NOM_1  28000000 //24->28Mhz Change
#define MT9V113_XCLK_NOM_2  18000000

/* FPS Capabilities */
#define MT9V113_MIN_FPS   7
#define MT9V113_DEF_FPS   30
#define MT9V113_MAX_FPS   30

#define I2C_RETRY_COUNT   5

#define MT9V113_IMAGE_WIDTH_MAX       640
#define MT9V113_IMAGE_HEIGHT_MAX      480

/* Analog gain values */
#define MT9V113_MIN_GAIN    (0)
#define MT9V113_MAX_GAIN    (40)
#define MT9V113_DEF_GAIN    (0)
#define MT9V113_GAIN_STEP   0x1

/* Exposure time values */
#define MT9V113_MIN_EXPOSURE    1000
#define MT9V113_MAX_EXPOSURE    128000
#define MT9V113_DEF_EXPOSURE    33000
#define MT9V113_EXPOSURE_STEP   50

#define SENSOR_DETECTED     1
#define SENSOR_NOT_DETECTED 0



//#define MT9V113_LOGD(format, arg...) printk(KERN_DEBUG "[MT9V113] %s : %d: %s()" format ,__FILE__, __LINE__, __FUNCTION__, ## arg )
#define MT9V113_LOGD(format, arg...) printk(KERN_DEBUG "[MT9V113] L:%04d: %s() " format , __LINE__, __FUNCTION__, ## arg )
#if 0
#define MT9V113_FUNC_IN  printk(KERN_DEBUG "[KJK_MT9V113] %s : %d: %s() IN  <--\n", __FILE__, __LINE__, __FUNCTION__)
#define MT9V113_FUNC_OUT printk(KERN_DEBUG "[KJK_MT9V113] %s : %d: %s() OUT -->\n", __FILE__, __LINE__, __FUNCTION__)
#else
#define MT9V113_FUNC_IN
#define MT9V113_FUNC_OUT
#endif
#define MT9V113_DEV_LOG(dev, format, arg...)  \
  printk(KERN_DEBUG "%s %s: " format , dev_driver_string(dev) , \
         dev_name(dev) , ## arg)
         
#if 0 // Ref Code. KJK
//KJK START : Debug Msg
#define kjk_printk(format, arg...)  \
  printk(KERN_DEBUG "[KJK] %s(%d): " format ,__func__ , __LINE__ , ## arg)

#define KJK_FUNC_IN \
  printk(KERN_DEBUG "[KJK] %s(%d): <-----\n" ,__func__ , __LINE__)

#define KJK_FUNC_OUT  \
  printk(KERN_DEBUG "[KJK] %s(%d): ----->\n" ,__func__ , __LINE__)


#define dev_printk(level, dev, format, arg...)  \
  printk(level "[KJK] %s %s %s(%d): " format , dev_driver_string(dev) , \
         dev_name(dev), __func__ , __LINE__ , ## arg)

#define dev_dbg(dev, format, arg...)    \
  dev_printk(KERN_DEBUG , dev , format , ## arg)
//KJK END
#endif 

/**
 * struct mt9v113_reg - mt9v113 register format
 * @reg: 16-bit offset to register
 * @val: 8/16/32-bit register value
 * @length: length of the register
 *
 * Define a structure for mt9v113 register initialization values
 */
struct mt9v113_reg {
  u16   reg;
  u32   val;
  u16 length;
};

enum mt9v113_image_size {
  MT9V113_QCIF,  /*QCIF Resolution*/
  MT9V113_QVGA,  /*QVGA Resolution*/
  MT9V113_VGA,   /*VGA Resolution*/
};
#define NUM_IMAGE_SIZES ARRAY_SIZE (mt9v113_sizes)

/**
 * struct mt9v113_capture_size - image capture size information
 * @width: image width in pixels
 * @height: image height in pixels
 */
struct mt9v113_capture_size {
  unsigned long width;
  unsigned long height;
};

/**
 * struct struct clk_settings - struct for storage of sensor
 * clock settings
 */
struct mt9v113_clk_settings {
  u16 pre_pll_div;
  u16 pll_mult;
  u16  post_pll_div;
  u16 vt_pix_clk_div;
  u16 vt_sys_clk_div;
};

/**
 * struct struct frame_settings - struct for storage of sensor
 * frame settings
 */
struct mt9v113_frame_settings {
  u16 frame_len_lines_min;
  u16 frame_len_lines;
  u16 line_len_pck;
  u16 x_addr_start;
  u16 x_addr_end;
  u16 y_addr_start;
  u16 y_addr_end;
  u16 x_output_size;
  u16 y_output_size;
  u16 x_even_inc;
  u16 x_odd_inc;
  u16 y_even_inc;
  u16 y_odd_inc;
  u16 v_mode_add;
  u16 h_mode_add;
  u16 h_add_ave;
};


/**
 * struct mt9v113_platform_data - platform data values and access functions
 * @power_set: Power state access function, zero is off, non-zero is on.
 * @default_regs: Default registers written after power-on or reset.
 * @ifparm: Interface parameters access function
 * @priv_data_set: device private data (pointer) access function
 */
struct mt9v113_platform_data {

  int (*power_set)(struct v4l2_int_device *s, enum v4l2_power power);
  int (*ifparm)(struct v4l2_ifparm *p);
  int (*priv_data_set)(struct v4l2_int_device *s, void *);
  u32 (*set_xclk)(struct v4l2_int_device *s, u32 xclkfreq);
  int (*cfg_interface_bridge)(u32);
};

/**
 * struct mt9v113_sensor_settings - struct for storage of
 * sensor settings.
 */
struct mt9v113_sensor_settings {
  struct mt9v113_clk_settings clk;
  struct mt9v113_frame_settings frame;
};

/**
 * struct struct mt9v113_clock_freq - struct for storage of sensor
 * clock frequencies
 */
struct mt9v113_clock_freq {
  u32 vco_clk;
  u32 vt_pix_clk;
};

/*
 * Array of image sizes supported by mt9v113.  These must be ordered from
 * smallest image size to largest.
 */

const static struct mt9v113_capture_size mt9v113_sizes[] = {
  {176, 144 },  /* QCIF - Preview Resolution */
  {320, 240 },  /* QVGA - Preview Resolution */
  {640, 480 }   /* VGA  - Preview Resolution */
};

/* PLL settings for mt9v113 */
enum mt9v113_pll_type {
  MT9V113_PLL_QCIF = 0,
  MT9V113_PLL_QVGA,
  MT9V113_PLL_VGA
};

//KJK 2011.03.08
static int mt9v113_power_reset(struct v4l2_int_device *s);


#endif /* ifndef mt9v113_H */
//--]] LGE_UBIQUIX_MODIFIED_END : ymjun@mnbt.co.kr [2011.07.26] - CAM : from justin froyo