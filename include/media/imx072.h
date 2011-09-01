/*
 * drivers/media/video/imx072.h
 *
 * Register definitions for the IMX072 Sensor.
 *
 *
 * Copyright (C) 2010 MM Solutions 
 *
 * This file is licensed under the terms of the GNU General Public License
 * version 2. This program is licensed "as is" without any warranty of any
 * kind, whether express or implied.
 */

#ifndef IMX072
#define IMX072

#define IMX072_I2C_ADDR		0x1A

/* The ID values we are looking for */
#define IMX072_MOD_ID			0x0045

#define IMX072_MFR_ID			0x000B

#define VAUX_2_8_V		0x09
#define VAUX_1_8_V		0x05
#define VAUX_DEV_GRP_P1	0x20
#define VAUX_DEV_GRP_NONE	0x00

/* IMX072 has 8/16/32 I2C registers */
#define I2C_8BIT			1
#define I2C_16BIT			2
#define I2C_32BIT			4

/* Terminating list entry for reg */
#define I2C_REG_TERM		0xFFFF
/* Terminating list entry for val */
#define I2C_VAL_TERM		0xFFFFFFFF
/* Terminating list entry for len */
#define I2C_LEN_TERM		0xFFFF

/* terminating token for reg list */
#define IMX072_TOK_TERM 		0xFF

/* delay token for reg list */
#define IMX072_TOK_DELAY		100

/* Sensor specific GPIO signals */
#define IMX072_RESET_GPIO  	98
#define IMX072_STANDBY_GPIO	58

/* CSI2 Virtual ID */
#define IMX072_CSI2_VIRTUAL_ID	0x0

#define IMX072_CLKRC			0x11

/* Average black level */
#define IMX072_BLACK_LEVEL_AVG	64

/* Used registers */
#define IMX072_REG_MODEL_ID				0x0000
#define IMX072_REG_REV_NUMBER			0x0002
#define IMX072_REG_MFR_ID				0x0003

#define IMX072_REG_MODE_SELECT			0x0100
#define IMX072_REG_IMAGE_ORIENTATION	0x0101
#define IMX072_REG_SW_RESET				0x0103
#define IMX072_REG_GROUPED_PAR_HOLD		0x0104
#define IMX072_REG_CCP2_CHANNEL_ID		0x0110

#define IMX072_REG_FINE_INT_TIME		0x0200
#define IMX072_REG_COARSE_INT_TIME		0x0202

#define IMX072_REG_ANALOG_GAIN_GLOBAL	0x0204
#define IMX072_REG_ANALOG_GAIN_GREENR	0x0206
#define IMX072_REG_ANALOG_GAIN_RED		0x0208
#define IMX072_REG_ANALOG_GAIN_BLUE		0x020A
#define IMX072_REG_ANALOG_GAIN_GREENB	0x020C
#define IMX072_REG_DIGITAL_GAIN_GREENR	0x020E
#define IMX072_REG_DIGITAL_GAIN_RED		0x0210
#define IMX072_REG_DIGITAL_GAIN_BLUE	0x0212
#define IMX072_REG_DIGITAL_GAIN_GREENB	0x0214

#define IMX072_REG_VT_PIX_CLK_DIV		0x0300
#define IMX072_REG_VT_SYS_CLK_DIV		0x0302
#define IMX072_REG_PRE_PLL_CLK_DIV		0x0304
#define IMX072_REG_PLL_MULTIPLIER		0x0306
#define IMX072_REG_OP_PIX_CLK_DIV		0x0308
#define IMX072_REG_OP_SYS_CLK_DIV		0x030A

#define IMX072_REG_FRAME_LEN_LINES		0x0340
#define IMX072_REG_LINE_LEN_PCK			0x0342

#define IMX072_REG_X_ADDR_START			0x0344
#define IMX072_REG_Y_ADDR_START			0x0346
#define IMX072_REG_X_ADDR_END			0x0348
#define IMX072_REG_Y_ADDR_END			0x034A
#define IMX072_REG_X_OUTPUT_SIZE		0x034C
#define IMX072_REG_Y_OUTPUT_SIZE		0x034E
#define IMX072_REG_X_EVEN_INC			0x0380
#define IMX072_REG_X_ODD_INC			0x0382
#define IMX072_REG_Y_EVEN_INC			0x0384
#define IMX072_REG_Y_ODD_INC			0x0386

#define IMX072_REG_HMODEADD				0x3001
#define HMODEADD_SHIFT					7
#define HMODEADD_MASK  					(0x1 << HMODEADD_SHIFT)
#define IMX072_REG_OPB_CTRL				0x300C
#define IMX072_REG_Y_OPBADDR_START_DI	0x3014
#define IMX072_REG_Y_OPBADDR_END_DI		0x3015
#define IMX072_REG_PGACUR_VMODEADD		0x3016
#define VMODEADD_SHIFT					6
#define VMODEADD_MASK  					(0x1 << VMODEADD_SHIFT)
#define IMX072_REG_CHCODE_OUTCHSINGLE	0x3017
#define IMX072_REG_OUTIF				0x301C
#define IMX072_REG_RGPLTD_RGCLKEN		0x3022
#define RGPLTD_MASK						0x3
#define IMX072_REG_RGPOF_RGPOFV2		0x3031
#define IMX072_REG_CPCKAUTOEN			0x3040
#define IMX072_REG_RGCPVFB				0x3041
#define IMX072_REG_RGAZPDRV				0x3051
#define IMX072_REG_RGAZTEST				0x3053
#define IMX072_REG_RGVSUNLV				0x3055
#define IMX072_REG_CLPOWER				0x3060
#define IMX072_REG_CLPOWERSP			0x3065
#define IMX072_REG_ACLDIRV_TVADDCLP		0x30AA
#define IMX072_REG_TESTDI				0x30E5
#define IMX072_REG_HADDAVE				0x30E8
#define HADDAVE_SHIFT					7
#define HADDAVE_MASK  					(0x1 << HADDAVE_SHIFT)

#define IMX072_REG_RAW10CH2V2P_LO		0x31A4
#define IMX072_REG_RAW10CH2V2D_LO		0x31A6
#define IMX072_REG_COMP8CH1V2P_LO		0x31AC
#define IMX072_REG_COMP8CH1V2D_LO		0x31AE
#define IMX072_REG_RAW10CH1V2P_LO		0x31B4
#define IMX072_REG_RAW10CH1V2D_LO		0x31B6

#define IMX072_REG_RGOUTSEL1			0x3300
#define IMX072_REG_RGLANESEL			0x3301
#define IMX072_REG_RGTLPX 				0x3304
#define IMX072_REG_RGTCLKPREPARE 		0x3305
#define IMX072_REG_RGTCLKZERO 			0x3306
#define IMX072_REG_RGTCLKPRE 			0x3307
#define IMX072_REG_RGTCLKPOST 			0x3308
#define IMX072_REG_RGTCLKTRAIL 			0x3309
#define IMX072_REG_RGTHSEXIT 			0x330A
#define IMX072_REG_RGTHSPREPARE			0x330B
#define IMX072_REG_RGTHSZERO			0x330C
#define IMX072_REG_RGTHSTRAIL 			0x330D


/*
 * The nominal xclk input frequency of the IMX072 is 24MHz, maximum
 * frequency is 27MHz, and minimum frequency is 6MHz.
 */
#define IMX072_XCLK_MIN   	6000000
#define IMX072_XCLK_MAX   	27000000
#define IMX072_XCLK_NOM_1 	24000000
#define IMX072_XCLK_NOM_2 	18000000

/* FPS Capabilities */
#define IMX072_MIN_FPS		5
#define IMX072_DEF_FPS		15
#define IMX072_MAX_FPS		30

#define I2C_RETRY_COUNT		5

/* Still capture 5 MP */
#define IMX072_IMAGE_WIDTH_MAX	2608
#define IMX072_IMAGE_HEIGHT_MAX	1960

/* Analog gain values */
#define IMX072_MIN_GAIN		(256*1)
#define IMX072_MAX_GAIN		(256*8)
#define IMX072_DEF_GAIN		(256*2)
#define IMX072_DEF_EV_GAIN	15
#define IMX072_GAIN_STEP		1

/* Exposure time values */
#define IMX072_MIN_EXPOSURE		24
#define IMX072_MAX_EXPOSURE		200000
#define IMX072_DEF_EXPOSURE	    33000
#define IMX072_EXPOSURE_STEP		50

#define IMX072_MAX_FRAME_LENGTH_LINES	0xFFFF
#define IMX072_MIN_EXPOSURE_LINES	0x1

#define SENSOR_DETECTED		1
#define SENSOR_NOT_DETECTED	0

/**
 * struct imx072_reg - imx072 register format
 * @reg: 16-bit offset to register
 * @val: 8/16/32-bit register value
 * @length: length of the register
 *
 * Define a structure for IMX072 register initialization values
 */
struct imx072_reg {
	u16 	reg;
	u32 	val;
	u16	length;
};

enum imx072_image_size {
	VT_0_2,
	MP_0_3,
	MP_1_2,
	MP_FIVE,
	MP_FIVE_CC
};

/**
 * struct imx072_capture_size - image capture size information
 * @width: image width in pixels
 * @height: image height in pixels
 */
struct imx072_capture_size {
	unsigned long width;
	unsigned long height;
};

/**
 * struct imx072_platform_data - platform data values and access functions
 * @power_set: Power state access function, zero is off, non-zero is on.
 * @default_regs: Default registers written after power-on or reset.
 * @ifparm: Interface parameters access function
 * @priv_data_set: device private data (pointer) access function
 */
struct imx072_platform_data {

	int (*power_set)(struct v4l2_int_device *s, enum v4l2_power power);
	int (*ifparm)(struct v4l2_ifparm *p);
	int (*priv_data_set)(struct v4l2_int_device *s, void *);
	u32 (*set_xclk)(struct v4l2_int_device *s, u32 xclkfreq);
	int (*cfg_interface_bridge)(u32);
	int (*csi2_lane_count)(struct v4l2_int_device *s, int count);
	int (*csi2_cfg_vp_out_ctrl)(struct v4l2_int_device *s, u8 vp_out_ctrl);
	int (*csi2_ctrl_update)(struct v4l2_int_device *s, bool);
	int (*csi2_cfg_virtual_id)(struct v4l2_int_device *s, u8 ctx, u8 id);
	int (*csi2_ctx_update)(struct v4l2_int_device *s, u8 ctx, bool);
	int (*csi2_calc_phy_cfg0)(struct v4l2_int_device *s, u32, u32, u32);

};

/**
 * struct struct clk_settings - struct for storage of sensor
 * clock settings
 */
struct imx072_clk_settings {
	u16	pre_pll_div;
	u16	pll_mult;
	u16  post_pll_div;
	u16	vt_pix_clk_div;
	u16	vt_sys_clk_div;
};

/**
 * struct struct mipi_settings - struct for storage of sensor
 * mipi settings
 */
struct imx072_mipi_settings {
	u16	data_lanes;
	u16	ths_prepare;
	u16	ths_zero;
	u16	ths_settle_lower;
	u16	ths_settle_upper;
};

/**
 * struct struct frame_settings - struct for storage of sensor
 * frame settings
 */
struct imx072_frame_settings {
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
 * struct struct imx072_sensor_settings - struct for storage of
 * sensor settings.
 */
struct imx072_sensor_settings {
	struct imx072_clk_settings clk;
	struct imx072_mipi_settings mipi;
	struct imx072_frame_settings frame;
};

/**
 * struct struct imx072_clock_freq - struct for storage of sensor
 * clock frequencies
 */
struct imx072_clock_freq {
	u32 vco_clk;
	u32 mipi_clk;
	u32 ddr_clk;
	u32 vt_pix_clk;
};

/**
 * Array of image sizes supported by IMX072.  These must be ordered from
 * smallest image size to largest.
 */
const static struct imx072_capture_size imx072_sizes[] = {
	{ 576, 490 },	/* 0.2Mp - 4X Horizontal & Vertical Elim. */
	{ 652, 490 },	/* 0.3Mp - 4X Horizontal & Vertical Elim. */
	{ 1304, 980 },	/* 0.8Mp - 2X Horz & 3X Vert */
	{ 2608, 1960},	/* 5.1MP - Full Resolution */
	{ 2608, 1960},	/* 5.1MP - Full Resolution Continuous Capture */
};

/* PLL settings for imx072 */
enum imx072_pll_type {
	PLL_VT_0_2MP = 0,
	PLL_0_3MP,
	PLL_1_2MP,
	PLL_5_1MP,
	PLL_5_1MP_CC,
};

#endif /* ifndef IMX072 */
