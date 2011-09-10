/*
 * drivers/media/video/yacd5b1s.h
 *
 * Register definitions for the yacd5b1s Sensor.
 *
 * This file is licensed under the terms of the GNU General Public License
 * version 2. This program is licensed "as is" without any warranty of any
 * kind, whether express or implied.
 */

#ifndef YACD5B1S_H
#define YACD5B1S_H

#define YACD5B1S_I2C_ADDR		0x20

#define VAUX_2_8_V				0x09
#define VAUX_1_8_V				0x05
#define VAUX_DEV_GRP_P1	        0x20
#define VAUX_DEV_GRP_NONE		0x00

/* yacd5b1s has 8/16/32 I2C registers */
#define I2C_8BIT			1
#define I2C_16BIT			2
#define I2C_32BIT			4

/* Terminating list entry for reg */
#define I2C_REG_TERM		0xFFFF
/* Terminating list entry for val */
#define I2C_VAL_TERM		0xFFFFFFFF
/* Terminating list entry for len */
#define I2C_LEN_TERM		0xFFFF

/* Average black level */
#define YACD5B1S_BLACK_LEVEL_AVG	64

/*============================================================================
                  SENSOR REGISTER DEFINES
=============================================================================*/
#define YACD5B1S_REG_MODEL_ID   	0x04

#define YACD5B1S_FW_SENSOR_ID       0x51



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
 * The nominal xclk input frequency of the yacd5b1s is 24MHz, maximum
 * frequency is 45MHz, and minimum frequency is 6MHz.
 */
#define YACD5B1S_XCLK_MIN   	6000000
#define YACD5B1S_XCLK_MAX   	45000000
#define YACD5B1S_XCLK_NOM_1 	24000000
#define YACD5B1S_XCLK_NOM_2 	18000000

/* FPS Capabilities */
#define YACD5B1S_MIN_FPS		7
#define YACD5B1S_DEF_FPS		30
#define YACD5B1S_MAX_FPS		30

#define I2C_RETRY_COUNT			5


#define YACD5B1S_IMAGE_WIDTH_MAX	        1600
#define YACD5B1S_IMAGE_HEIGHT_MAX			1200

/* Analog gain values */
#define YACD5B1S_MIN_GAIN		(0)
#define YACD5B1S_MAX_GAIN		(40)
#define YACD5B1S_DEF_GAIN		(0)
#define YACD5B1S_GAIN_STEP		0x1

/* Exposure time values */
#define YACD5B1S_MIN_EXPOSURE		1000
#define YACD5B1S_MAX_EXPOSURE		128000
#define YACD5B1S_DEF_EXPOSURE	    33000
#define YACD5B1S_EXPOSURE_STEP		50

#define SENSOR_DETECTED		1
#define SENSOR_NOT_DETECTED	0

/**
 * struct yacd5b1s_reg - yacd5b1s register format
 * @reg: 16-bit offset to register
 * @val: 8/16/32-bit register value
 * @length: length of the register
 *
 * Define a structure for yacd5b1s register initialization values
 */
struct yacd5b1s_reg {
	u16 	reg;
	u32 	val;
	u16	length;
};

enum yacd5b1s_image_size {
	YACD5B1S_QCIF,  /*QCIF Resolution*/
	YACD5B1S_QVGA,  /*QVGA Resolution*/
	YACD5B1S_VGA,  /*VGA Resolution*/
	YACD5B1S_1MP,  /*1MP Resolution*/
	YACD5B1S_2MP, /*2MP Full Resolution*/	
	YACD5B1S_SIZE
};

/* Configuration mode for yacd5b1s */
enum yacd5b1s_cfg_mode {
	YACD5B1S_CFG_PREVIEW,
	YACD5B1S_CFG_RETPREVIEW,
	YACD5B1S_CFG_CAPTURE,
	YACD5B1S_CFG_MODE
};

#define NUM_IMAGE_SIZES ARRAY_SIZE              (yacd5b1s_sizes)

/**
 * struct yacd5b1s_capture_size - image capture size information
 * @width: image width in pixels
 * @height: image height in pixels
 */
struct yacd5b1s_capture_size {
	unsigned long width;
	unsigned long height;
};

/**
 * struct struct clk_settings - struct for storage of sensor
 * clock settings
 */
struct yacd5b1s_clk_settings {
	u16	pre_pll_div;
	u16	pll_mult;
	u16  post_pll_div;
	u16	vt_pix_clk_div;
	u16	vt_sys_clk_div;
};

/**
 * struct struct frame_settings - struct for storage of sensor
 * frame settings
 */
struct yacd5b1s_frame_settings {
	u16	frame_len_lines_min;
	u16	frame_len_lines;
	u16	line_len_pck;
	u16	x_addr_start;
	u16	x_addr_end;
	u16	y_addr_start;
	u16	y_addr_end;
	u16	x_output_size;
	u16	y_output_size;
	u16	x_even_inc;
	u16	x_odd_inc;
	u16	y_even_inc;
	u16	y_odd_inc;
	u16	v_mode_add;
	u16	h_mode_add;
	u16	h_add_ave;
};


/**
 * struct yacd5b1s_platform_data - platform data values and access functions
 * @power_set: Power state access function, zero is off, non-zero is on.
 * @default_regs: Default registers written after power-on or reset.
 * @ifparm: Interface parameters access function
 * @priv_data_set: device private data (pointer) access function
 */
struct yacd5b1s_platform_data {

	int (*power_set)(struct v4l2_int_device *s, enum v4l2_power power);
	int (*ifparm)(struct v4l2_ifparm *p);
	int (*priv_data_set)(struct v4l2_int_device *s, void *);
	u32 (*set_xclk)(struct v4l2_int_device *s, u32 xclkfreq);
	int (*cfg_interface_bridge)(u32);
};

/**
 * struct yacd5b1s_sensor_settings - struct for storage of
 * sensor settings.
 */
struct yacd5b1s_sensor_settings {
	struct yacd5b1s_clk_settings clk;
	struct yacd5b1s_frame_settings frame;
};

/**
 * struct struct yacd5b1s_clock_freq - struct for storage of sensor
 * clock frequencies
 */
struct yacd5b1s_clock_freq {
	u32 vco_clk;
	u32 vt_pix_clk;
};

/*
 * Array of image sizes supported by yacd5b1s.  These must be ordered from
 * smallest image size to largest.
 */
const static struct yacd5b1s_capture_size yacd5b1s_sizes[] = {
	{176, 144 },  /* QCIF - Preview Resolution */
	{320, 240 },  /* QVGA - Preview Resolution */
	{640, 480 },  /* VGA - Preview Resolution */
	{1280, 960 },/* 1MP - Full Resolution */
	{1600, 1200 },/* 2MP - Full Resolution */		
};

/* PLL settings for yacd5b1s */
enum yacd5b1s_pll_type {
	YACD5B1S_PLL_QCIF = 0,
	YACD5B1S_PLL_QVGA,
	YACD5B1S_PLL_VGA,
	YACD5B1S_PLL_1MP,
	YACD5B1S_PLL_2MP,
};

#endif /* ifndef yacd5b1s_H */
