
#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/fb.h>
#include <linux/i2c.h>
#include <linux/string.h>
#include <asm/gpio.h>
#include <asm/system.h>
#include <mach/wm9093.h>
#include <mach/hub_headset_det.h>
#include <linux/workqueue.h>	//junyeop.kim@lge.com


struct wm9093_i2c_device *wm9093_i2c_dev = NULL;
struct wm9093_device *wm9093_amp_dev = NULL;

#include <linux/i2c/twl.h>
#include <linux/regulator/consumer.h>

extern int voice_get_curmode(void);

#define VPLL2_DEV_GRP       0x33  //junyeop.kim@lge.com, vpll2 power domain grp

#define MODULE_NAME		"wm9093"


#ifndef DEBUG
//#define DEBUG
//#undef DEBUG
#endif

#ifdef DEBUG
#define DBG(fmt, args...) 				\
	printk(KERN_DEBUG "[%s] [wm9093.c] %s(),  (%d): " 		\
		fmt, MODULE_NAME, __func__, __LINE__, ## args);
#else	/* DEBUG */
#define DBG(...)
#endif


static struct regulator *wm9093_reg;
#if defined(CONFIG_PRODUCT_LGE_LU6800) || defined(CONFIG_PRODUCT_LGE_KU5900) || defined (CONFIG_PRODUCT_LGE_P970)
static wm9093_fmvolume_enum s_volume =  LEVEL_4;
//20101222 inbang.park@lge.com Wake lock for  FM Radio [START]
static wm9093_fmvolume_enum s_modes = LEVEL_OFF;
unsigned int cur_fmradio_mode = 0;
//20101222 inbang.park@lge.com Wake lock for  FM Radio [END]
#endif // #if defined(CONFIG_PRODUCT_LGE_LU6800) || defined(CONFIG_PRODUCT_LGE_KU5900) || defined (CONFIG_PRODUCT_LGE_P970)

int wm9093_call_status = 1;		//20101209 junyeop.kim@lge.com, reduce the outgoing call noise (0 : incoming call, 1 :other case) [START_LGE]
#if defined(CONFIG_PRODUCT_LGE_LU6800)  // 20101231 seungdae.goh@lge.com HW_TUNNING

static const wm9093_reg_type wm9093_in1_to_out_tab[] =
{
  	{WM9093_CMD ,0x01, 0x000B},
	{WM9093_CMD ,0x02, 0x60C0},
	{WM9093_CMD ,0x16, 0x0001},
	{WM9093_CMD ,0x18, 0x0002},
	{WM9093_CMD ,0x19, 0x0002},
	{WM9093_CMD ,0x18, 0x0102},
	{WM9093_CMD ,0x36, 0x0050},
	{WM9093_CMD ,0x03, 0x0008},
	{WM9093_CMD ,0x22, 0x0050},
	{WM9093_CMD ,0x03, 0x0108},
	{WM9093_CMD ,0x25, 0x0160},
	{WM9093_CMD ,0x16, 0x0000},
	{WM9093_CMD ,0x01, 0x100B},
    {WM9093_END_SEQ,0x00,0x00}
};

static const wm9093_reg_type wm9093_in1_to_hp_tab[] =
{

	{WM9093_DELAY,0x39, 0x0050},
	{WM9093_CMD ,0x39, 0x000D},
	{WM9093_CMD ,0x01, 0x000B},
	{WM9093_CMD ,0x02, 0x60C0},
	{WM9093_CMD ,0x16, 0x0001},
	{WM9093_CMD ,0x18, 0x0100},	// 20100517 junyeop.kim@lge.com, headset tuning for media(HW require) [START_LGE]
	{WM9093_CMD ,0x19, 0x0100},	// 20100517 junyeop.kim@lge.com, headset tuning for media(HW require) [START_LGE]
	{WM9093_CMD ,0x2D, 0x0040},
	{WM9093_CMD ,0x2E, 0x0010},
	{WM9093_CMD ,0x03, 0x0030},
	{WM9093_CMD ,0x2F, 0x0000},
	{WM9093_CMD ,0x30, 0x0000},
	{WM9093_CMD ,0x16, 0x0000},
	{WM9093_CMD ,0x1C, 0x0133},	// 20100517 junyeop.kim@lge.com, headset tuning for media(HW require) [START_LGE]
	{WM9093_CMD ,0x1D, 0x0133},	// 20100517 junyeop.kim@lge.com, headset tuning for media(HW require) [START_LGE]
	{WM9093_CMD ,0x1C, 0x0133},	// 20100517 junyeop.kim@lge.com, headset tuning for media(HW require) [START_LGE]
	{WM9093_CMD ,0x46, 0x0100},
	{WM9093_CMD ,0x49, 0x0100},
    {WM9093_END_SEQ,0x00,0x00}
};
//20101205 inbang.park@lge.com Add STREAM  for  FM Radio [START]
static const wm9093_reg_type wm9093_in1_to_FMhp_tab[] =
{
       {WM9093_DELAY,0x39, 0x0050},
	{WM9093_CMD ,0x39, 0x000D},
	{WM9093_CMD ,0x01, 0x000B},
	{WM9093_CMD ,0x02, 0x60C0},
	{WM9093_CMD ,0x16, 0x0001},
	{WM9093_CMD ,0x18, 0x0100},
	{WM9093_CMD ,0x19, 0x0100},
	{WM9093_CMD ,0x2D, 0x0040},
	{WM9093_CMD ,0x2E, 0x0010},
	{WM9093_CMD ,0x03, 0x0030},
	{WM9093_CMD ,0x2F, 0x0000},
	{WM9093_CMD ,0x30, 0x0000},
	{WM9093_CMD ,0x16, 0x0000},
	//{WM9093_CMD ,0x1C, 0x011C},	//initial value
	//{WM9093_CMD ,0x1D, 0x011C},	//initial value
	//{WM9093_CMD ,0x1C, 0x0135},
	{WM9093_CMD ,0x46, 0x0100},
	{WM9093_CMD ,0x49, 0x0100},
       {WM9093_END_SEQ,0x00,0x00}
};
static const wm9093_reg_type wm9093_in1_to_FMout_tab[] =
{
  	{WM9093_CMD ,0x39, 0x000D},
	{WM9093_CMD ,0x01, 0x000B},
	{WM9093_CMD ,0x02, 0x6030},
	{WM9093_CMD ,0x17, 0x0001},
	{WM9093_CMD ,0x1A, 0x0100},
	{WM9093_CMD ,0x1B, 0x0100},
	{WM9093_CMD ,0x36, 0x0005},
	{WM9093_CMD ,0x03, 0x0008},
	{WM9093_CMD ,0x22, 0x0000},
	{WM9093_CMD ,0x03, 0x0108},
	{WM9093_CMD ,0x25, 0x0178},
	{WM9093_CMD ,0x24, 0x0010},
	//{WM9093_CMD ,0x26, 0x011C},	//initial value
	{WM9093_CMD ,0x17, 0x0000},
	{WM9093_CMD ,0x01, 0x100B},
    {WM9093_END_SEQ,0x00,0x00}
};
//20101205 inbang.park@lge.com Add STREAM  for  FM Radio [END]
//20101120 junyeop.kim@lge.com, voip call tuning[START_LGE]
static const wm9093_reg_type wm9093_in1_to_hp_voip_tab[] =
{
	{WM9093_DELAY,0x39, 0x0050},
	{WM9093_CMD ,0x39, 0x000D},
	{WM9093_CMD ,0x01, 0x000B},
	{WM9093_CMD ,0x02, 0x60C0},
	{WM9093_CMD ,0x16, 0x0001},
	{WM9093_CMD ,0x18, 0x0100},	// 20100517 junyeop.kim@lge.com, headset tuning for media(HW require) [START_LGE]
	{WM9093_CMD ,0x19, 0x0100},	// 20100517 junyeop.kim@lge.com, headset tuning for media(HW require) [START_LGE]
	{WM9093_CMD ,0x2D, 0x0040},
	{WM9093_CMD ,0x2E, 0x0010},
	{WM9093_CMD ,0x03, 0x0030},
	{WM9093_CMD ,0x2F, 0x0000},
	{WM9093_CMD ,0x30, 0x0000},
	{WM9093_CMD ,0x16, 0x0000},
	{WM9093_CMD ,0x1C, 0x0133},	// 20100517 junyeop.kim@lge.com, headset tuning for media(HW require) [START_LGE]
	{WM9093_CMD ,0x1D, 0x0133},	// 20100517 junyeop.kim@lge.com, headset tuning for media(HW require) [START_LGE]
	{WM9093_CMD ,0x1C, 0x0133},	// 20100517 junyeop.kim@lge.com, headset tuning for media(HW require) [START_LGE]
	{WM9093_CMD ,0x46, 0x0100},
	{WM9093_CMD ,0x49, 0x0100},
    {WM9093_END_SEQ,0x00,0x00}
};
//20101120 junyeop.kim@lge.com, voip call tuning[END_LGE]


static const wm9093_reg_type wm9093_in2_to_out_hp_tab[] =
{

 	{WM9093_CMD ,0x39, 0x000D},
    {WM9093_CMD ,0x01, 0x130B},
	{WM9093_CMD ,0x02, 0x60f0},
    {WM9093_CMD ,0x16, 0x0001},
	{WM9093_CMD ,0x18, 0x0002},	// 20100517 junyeop.kim@lge.com, headset/spk tuning for media(HW require) [START_LGE]
	{WM9093_CMD ,0x19, 0x0002},	// 20100517 junyeop.kim@lge.com, headset/spk tuning for media(HW require) [START_LGE]
	{WM9093_CMD ,0x18, 0x0102},
	{WM9093_CMD ,0x17, 0x0001},
	{WM9093_CMD ,0x1A, 0x0002},
	{WM9093_CMD ,0x1B, 0x0002},
	{WM9093_CMD ,0x1A, 0x0102},
    {WM9093_CMD ,0x36, 0x0005},
	{WM9093_CMD ,0x22, 0x0005},
	{WM9093_CMD ,0x25, 0x0178},
	{WM9093_CMD ,0x2D, 0x0040},
	{WM9093_CMD ,0x2E, 0x0010},
    {WM9093_CMD ,0x03, 0x0138},
//	{WM9093_CMD ,0x1C, 0x0023},	// 20101011 junyeop.kim@lge.com, dual path tuning for call(HW require) [START_LGE]
//	{WM9093_CMD ,0x1D, 0x0123},	// 20101011 junyeop.kim@lge.com, dual path tuning for call(HW require) [START_LGE]
//	{WM9093_CMD ,0x1C, 0x0123},	// 20101011 junyeop.kim@lge.com, dual path tuning for call(HW require) [START_LGE]
    {WM9093_CMD ,0x1C, 0x0017}, // 20120810 gt.kim      Dual Path tuning...
    {WM9093_CMD ,0x1D, 0x0117}, // 20120810 gt.kim      Dual Path tuning...
    {WM9093_CMD ,0x1C, 0x0117}, // 20120810 gt.kim     Dual Path tuning...
	{WM9093_CMD ,0x2F, 0x0000},
	{WM9093_CMD ,0x30, 0x0000},
    {WM9093_CMD ,0x16, 0x0000},
	{WM9093_CMD ,0x17, 0x0000},
    {WM9093_CMD ,0x46, 0x0100},
    {WM9093_CMD ,0x49, 0x0100},
    {WM9093_END_SEQ,0x00,0x00}



};


static const wm9093_reg_type wm9093_in2_to_out_call_tab[] =	//spk call
{
	{WM9093_CMD ,0x39, 0x000D},		//20100720 junyeop.kim@lge.com, fix the call mute
 	{WM9093_CMD ,0x01, 0x000B},
	{WM9093_CMD ,0x02, 0x6020},
//	{WM9093_CMD ,0x1A, 0x0000},
//	{WM9093_CMD ,0x1B, 0x0000},
	{WM9093_CMD ,0x1A, 0x0102},
	{WM9093_CMD ,0x1B, 0x0102},		// 20100816 junyeop.kim@lge.com, spk tuning for call(HW require) [START_LGE]
	{WM9093_CMD ,0x36, 0x0004},
	{WM9093_CMD ,0x03, 0x0008},
	{WM9093_CMD ,0x22, 0x0000},		// 20100816 junyeop.kim@lge.com, spk tuning for call(HW require) [START_LGE]
	{WM9093_CMD ,0x03, 0x0108},
	{WM9093_CMD ,0x24, 0x0010},
	{WM9093_CMD ,0x25, 0x0178},		// 20100816 junyeop.kim@lge.com, spk tuning for call(HW require) [START_LGE]
	{WM9093_CMD ,0x26, 0x013F},		// 20100816 junyeop.kim@lge.com, spk tuning for call(HW require) [START_LGE]
	{WM9093_CMD ,0x17, 0x0000},
	{WM9093_CMD ,0x01, 0x100B},
    {WM9093_END_SEQ,0x00,0x00}
};
//20101120 junyeop.kim@lge.com, voip call tuning[START_LGE]
static const wm9093_reg_type wm9093_in2_to_out_voip_tab[] =
{
	{WM9093_CMD ,0x39, 0x000D},
	{WM9093_CMD ,0x01, 0x000B},
	{WM9093_CMD ,0x02, 0x6030},
	{WM9093_CMD ,0x17, 0x0001},
	{WM9093_CMD ,0x1A, 0x0100},		// 20100604 junyeop.kim@lge.com, spk tuning for media(wolfson require) [START_LGE]
	{WM9093_CMD ,0x1B, 0x0100},		// 20100604 junyeop.kim@lge.com, spk tuning for media(wolfson require) [START_LGE]
	{WM9093_CMD ,0x36, 0x0005},
	{WM9093_CMD ,0x03, 0x0008},
	{WM9093_CMD ,0x22, 0x0000},		// 20100709 junyeop.kim@lge.com, spk tuning for media(HW require) [START_LGE]
	{WM9093_CMD ,0x03, 0x0108},
	{WM9093_CMD ,0x25, 0x0178},		// 20100604 junyeop.kim@lge.com, spk tuning for media(PL require) [START_LGE]
	{WM9093_CMD ,0x24, 0x0010},		// 20100604 junyeop.kim@lge.com, spk tuning for media(PL require) [START_LGE]
//	{WM9093_CMD ,0x26, 0x003F},		// 20100604 junyeop.kim@lge.com, spk tuning for media(PL require) [START_LGE]
	{WM9093_CMD ,0x1C, 0x0134},	// 20100517 junyeop.kim@lge.com, headset tuning for media(HW require) [START_LGE]
	{WM9093_CMD ,0x1D, 0x0134},	// 20100517 junyeop.kim@lge.com, headset tuning for media(HW require) [START_LGE]
	{WM9093_CMD ,0x1C, 0x0134},	// 20100517 junyeop.kim@lge.com, headset tuning for media(HW require) [START_LGE]

	{WM9093_CMD ,0x17, 0x0000},
	{WM9093_CMD ,0x01, 0x100B},
    {WM9093_END_SEQ,0x00,0x00}
};
//20101120 junyeop.kim@lge.com, voip call tuning[END_LGE]

static const wm9093_reg_type wm9093_in2_to_out_tab[] =
{
	{WM9093_CMD ,0x39, 0x000D},
	{WM9093_CMD ,0x01, 0x000B},
	{WM9093_CMD ,0x02, 0x6030},
	{WM9093_CMD ,0x17, 0x0001},
	{WM9093_CMD ,0x1A, 0x0100},		// 20100604 junyeop.kim@lge.com, spk tuning for media(wolfson require) [START_LGE]
	{WM9093_CMD ,0x1B, 0x0100},		// 20100604 junyeop.kim@lge.com, spk tuning for media(wolfson require) [START_LGE]
	{WM9093_CMD ,0x36, 0x0005},
	{WM9093_CMD ,0x03, 0x0008},
	{WM9093_CMD ,0x22, 0x0000},		// 20100709 junyeop.kim@lge.com, spk tuning for media(HW require) [START_LGE]
	{WM9093_CMD ,0x03, 0x0108},
	{WM9093_CMD ,0x25, 0x0178},		// 20100604 junyeop.kim@lge.com, spk tuning for media(PL require) [START_LGE]
	{WM9093_CMD ,0x24, 0x0010},		// 20100604 junyeop.kim@lge.com, spk tuning for media(PL require) [START_LGE]
	{WM9093_CMD ,0x26, 0x0039},
	{WM9093_CMD ,0x17, 0x0000},
	{WM9093_CMD ,0x01, 0x100B},
    {WM9093_END_SEQ,0x00,0x00}
};

static const wm9093_reg_type wm9093_test_tab[] =
{
	{WM9093_DELAY,0x39, 0x0050},
	{WM9093_CMD ,0x39, 0x000D},
	{WM9093_CMD ,0x01, 0x000B},
	{WM9093_DELAY,0x00, 50},
	{WM9093_CMD ,0x02, 0x6030},
	{WM9093_CMD ,0x17, 0x0001},
	{WM9093_CMD ,0x1A, 0x0100},
	{WM9093_CMD ,0x1B, 0x0100},
	{WM9093_CMD ,0x2D, 0x0004},
	{WM9093_CMD ,0x2E, 0x0001},
	{WM9093_CMD ,0x03, 0x0030},
	{WM9093_CMD ,0x2F, 0x0000},
	{WM9093_CMD ,0x30, 0x0000},
	{WM9093_CMD ,0x17, 0x0000},
	{WM9093_CMD ,0x1C, 0x0135},
	{WM9093_CMD ,0x1D, 0x0135},
	{WM9093_CMD ,0x1C, 0x0135},
	{WM9093_CMD ,0x46, 0x0100},
	{WM9093_CMD ,0x49, 0x0100},
    {WM9093_END_SEQ,0x00,0x00}

};

/* LGE_CHANGE_S [iggikim@lge.com] 2009-11-10, rev a headset */
static const wm9093_reg_type wm9093_in2_to_hp_tab[] =	// headset call
{

    {WM9093_CMD ,0x39, 0x000D},
    {WM9093_CMD ,0x01, 0x000B},
	{WM9093_CMD ,0x02, 0x6040},
	{WM9093_CMD ,0x16, 0x0001},
	{WM9093_CMD ,0x18, 0x0100},	// 20100910 junyeop.kim@lge.com, headset tuning for call(HW require) [START_LGE]
	{WM9093_CMD ,0x19, 0x0100},	// 20100910 junyeop.kim@lge.com, headset tuning for call(HW require) [START_LGE]
	{WM9093_CMD ,0x2D, 0x0010},
	{WM9093_CMD ,0x2E, 0x0010},
	{WM9093_CMD ,0x03, 0x0030},
    {WM9093_CMD ,0x2F, 0x0000},	// 20100816 junyeop.kim@lge.com, headset tuning for call(HW require) [START_LGE]
    {WM9093_CMD ,0x30, 0x0000},	// 20100816 junyeop.kim@lge.com, headset tuning for call(HW require) [START_LGE]
	{WM9093_CMD ,0x16, 0x0000},
	{WM9093_CMD ,0x1C, 0x0133},	// 20101011 junyeop.kim@lge.com, headset tuning for call(HW require) [START_LGE]
	{WM9093_CMD ,0x1D, 0x0133},	// 20101011 junyeop.kim@lge.com, headset tuning for call(HW require) [START_LGE]
	{WM9093_CMD ,0x1C, 0x0133},	// 20100910 junyeop.kim@lge.com, headset tuning for call(HW require) [START_LGE]
	{WM9093_CMD ,0x46, 0x0100},
	{WM9093_CMD ,0x49, 0x0100},
    {WM9093_END_SEQ,0x00,0x00}

};
/* LGE_CHANGE_E [iggikim@lge.com] */
static const wm9093_reg_type wm9093_in3_to_out_tab[] =
{
	{WM9093_CMD ,0x39, 0x000D},		//20100720 junyeop.kim@lge.com, fix the call mute
	{WM9093_CMD ,0x01, 0x000B},
	{WM9093_CMD ,0x22, 0x1100},
	{WM9093_CMD ,0x24, 0x0020},
	{WM9093_CMD ,0x15, 0x0000},
//	{WM9093_CMD ,0x25, 0x01e0},     //20100912 junyeop.kim@lge.com, receiver tuning (HW request)
	{WM9093_CMD ,0x25, 0x01d8},     //20100912 junyeop.kim@lge.com, receiver tuning (HW request)
	{WM9093_CMD ,0x01, 0x100B},
    {WM9093_END_SEQ,0x00,0x00}
};

//20101120 junyeop.kim@lge.com, voip call tuning[START_LGE]
static const wm9093_reg_type wm9093_in3_to_out_voip_tab[] =
{
	{WM9093_CMD ,0x39, 0x000D},		//20100720 junyeop.kim@lge.com, fix the call mute
	{WM9093_CMD ,0x01, 0x000B},
	{WM9093_CMD ,0x22, 0x1100},
	{WM9093_CMD ,0x24, 0x0020},
	{WM9093_CMD ,0x15, 0x0000},
	{WM9093_CMD ,0x25, 0x01d8},
	{WM9093_CMD ,0x01, 0x100B},
    {WM9093_END_SEQ,0x00,0x00}
};
//20101120 junyeop.kim@lge.com, voip call tuning[END_LGE]

static const wm9093_reg_type wm9093_pwroff_tab[] =
{
#if 1 	//shut down sequence
    {WM9093_CMD ,0x46, 0x0100},
    {WM9093_CMD ,0x49, 0x0110},
    {WM9093_CMD ,0x00, 0x0000},
    {WM9093_CMD ,0x02, 0x6000},
    {WM9093_CMD ,0x39, 0x0000},
//    {WM9093_DELAY,0x39, 0x0200},    //added by jykim
#else
    {WM9093_CMD ,0x00, 0x9093},
#endif
    {WM9093_END_SEQ,0x00,0x00}};





#elif defined(CONFIG_PRODUCT_LGE_KU5900) || defined (CONFIG_PRODUCT_LGE_P970)    /*  defined(CONFIG_MACH_LGE_JUSTIN) */

static const wm9093_reg_type wm9093_in1_to_out_tab[] =
{
  	{WM9093_CMD ,0x01, 0x000B},
	{WM9093_CMD ,0x02, 0x60C0},
	{WM9093_CMD ,0x16, 0x0001},
	{WM9093_CMD ,0x18, 0x0002},
	{WM9093_CMD ,0x19, 0x0002},
	{WM9093_CMD ,0x18, 0x0102},
	{WM9093_CMD ,0x36, 0x0050},
	{WM9093_CMD ,0x03, 0x0008},
	{WM9093_CMD ,0x22, 0x0050},
	{WM9093_CMD ,0x03, 0x0108},
	{WM9093_CMD ,0x25, 0x0160},
	{WM9093_CMD ,0x16, 0x0000},
	{WM9093_CMD ,0x01, 0x100B},
    {WM9093_END_SEQ,0x00,0x00}
};

static const wm9093_reg_type wm9093_in1_to_hp_tab[] =
{
	{WM9093_DELAY,0x39, 0x0050},
	{WM9093_CMD ,0x39, 0x000D},
	{WM9093_CMD ,0x01, 0x000B},
	{WM9093_DELAY,0x00, 50}, //jongik2.kim 20101216 headset pop noise
	{WM9093_CMD ,0x02, 0x60C0},
	{WM9093_CMD ,0x16, 0x0001},
	{WM9093_CMD ,0x18, 0x0100},	// 20100517 junyeop.kim@lge.com, headset tuning for media(HW require) [START_LGE]
	{WM9093_CMD ,0x19, 0x0100},	// 20100517 junyeop.kim@lge.com, headset tuning for media(HW require) [START_LGE]
	{WM9093_CMD ,0x2D, 0x0040},
	{WM9093_CMD ,0x2E, 0x0010},
	{WM9093_CMD ,0x03, 0x0030},
	{WM9093_CMD ,0x2F, 0x0000},
	{WM9093_CMD ,0x30, 0x0000},
	{WM9093_CMD ,0x16, 0x0000},
// jk6884.lee@lge.com 20121022 P970 Audio parameter // 20121026 133 -> 135
#ifdef CONFIG_PRODUCT_LGE_P970
	{WM9093_CMD ,0x1C, 0x0134},
	{WM9093_CMD ,0x1D, 0x0134},
	{WM9093_CMD ,0x1C, 0x0134},
#else
	{WM9093_CMD ,0x1C, 0x0135},	// 20110224 mikyoung.chang@lge.com hw request
	{WM9093_CMD ,0x1D, 0x0135},	// 20110224 mikyoung.chang@lge.com hw request
	{WM9093_CMD ,0x1C, 0x0135},	// 20110224 mikyoung.chang@lge.com hw request
#endif
	{WM9093_CMD ,0x46, 0x0100},
	{WM9093_CMD ,0x49, 0x0100},
    {WM9093_END_SEQ,0x00,0x00}
};
//20101205 inbang.park@lge.com Add STREAM  for  FM Radio [START]
static const wm9093_reg_type wm9093_in1_to_FMhp_tab[] =
{
       {WM9093_DELAY,0x39, 0x0050},
	{WM9093_CMD ,0x39, 0x000D},
	{WM9093_CMD ,0x01, 0x000B},
	{WM9093_DELAY,0x00, 50},
	{WM9093_CMD ,0x02, 0x60C0},
	{WM9093_CMD ,0x16, 0x0001},
	{WM9093_CMD ,0x18, 0x0102},
	{WM9093_CMD ,0x19, 0x0102},
	{WM9093_CMD ,0x2D, 0x0040},
	{WM9093_CMD ,0x2E, 0x0010},
	{WM9093_CMD ,0x03, 0x0030},
	{WM9093_CMD ,0x2F, 0x0000},
	{WM9093_CMD ,0x30, 0x0000},
	{WM9093_CMD ,0x16, 0x0000},
	{WM9093_CMD ,0x2f, 0x0000},
	{WM9093_CMD ,0x30, 0x0000},
	{WM9093_CMD ,0x1C, 0x0100},	//initial value
	{WM9093_CMD ,0x1D, 0x0100},	//initial value
	{WM9093_CMD ,0x1C, 0x0100},
	{WM9093_CMD ,0x46, 0x0100},
	{WM9093_CMD ,0x49, 0x0100},
       {WM9093_END_SEQ,0x00,0x00}
};
static const wm9093_reg_type wm9093_in1_to_FMout_tab[] =
{
  	{WM9093_CMD ,0x39, 0x000D},
	{WM9093_CMD ,0x01, 0x000B},
	{WM9093_CMD ,0x02, 0x6030},
	{WM9093_CMD ,0x17, 0x0001},
	{WM9093_CMD ,0x1A, 0x0101},
	{WM9093_CMD ,0x1B, 0x0101},
	{WM9093_CMD ,0x36, 0x0005},
	{WM9093_CMD ,0x03, 0x0008},
	{WM9093_CMD ,0x22, 0x0000},
	{WM9093_CMD ,0x03, 0x0108},
	{WM9093_CMD ,0x25, 0x0178},
	{WM9093_CMD ,0x24, 0x0010},
	{WM9093_CMD ,0x2f, 0x0000},
	{WM9093_CMD ,0x30, 0x0000},
	{WM9093_CMD ,0x26, 0x0100},	//initial value
	{WM9093_CMD ,0x17, 0x0000},
	{WM9093_CMD ,0x01, 0x100B},
    {WM9093_END_SEQ,0x00,0x00}
};
//20101205 inbang.park@lge.com Add STREAM  for  FM Radio [END]
//20101120 junyeop.kim@lge.com, voip call tuning[START_LGE]
static const wm9093_reg_type wm9093_in1_to_hp_voip_tab[] =
{
	{WM9093_DELAY,0x39, 0x0050},
	{WM9093_CMD ,0x39, 0x000D},
	{WM9093_CMD ,0x01, 0x000B},
	{WM9093_CMD ,0x02, 0x60C0},
	{WM9093_CMD ,0x16, 0x0001},
	{WM9093_CMD ,0x18, 0x0100},	// 20100517 junyeop.kim@lge.com, headset tuning for media(HW require) [START_LGE]
	{WM9093_CMD ,0x19, 0x0100},	// 20100517 junyeop.kim@lge.com, headset tuning for media(HW require) [START_LGE]
	{WM9093_CMD ,0x2D, 0x0040},
	{WM9093_CMD ,0x2E, 0x0010},
	{WM9093_CMD ,0x03, 0x0030},
	{WM9093_CMD ,0x2F, 0x0000},
	{WM9093_CMD ,0x30, 0x0000},
	{WM9093_CMD ,0x16, 0x0000},
	{WM9093_CMD ,0x1C, 0x0131},	// 20100517 junyeop.kim@lge.com, headset tuning for media(HW require) [START_LGE]
	{WM9093_CMD ,0x1D, 0x0131},	// 20100517 junyeop.kim@lge.com, headset tuning for media(HW require) [START_LGE]
	{WM9093_CMD ,0x1C, 0x0131},	// 20100517 junyeop.kim@lge.com, headset tuning for media(HW require) [START_LGE]
	{WM9093_CMD ,0x46, 0x0100},
	{WM9093_CMD ,0x49, 0x0100},
    {WM9093_END_SEQ,0x00,0x00}
};
//20101120 junyeop.kim@lge.com, voip call tuning[END_LGE]


static const wm9093_reg_type wm9093_in2_to_out_hp_tab[] =
{
 	{WM9093_CMD ,0x39, 0x000D},
    {WM9093_CMD ,0x01, 0x130B},
//	{WM9093_DELAY,0x00, 50}, //jiwon.seo@lge.com 20101228 dual path pop noise
	{WM9093_CMD ,0x02, 0x60f0},
    {WM9093_CMD ,0x16, 0x0001},
	{WM9093_CMD ,0x18, 0x0002},	// 20100517 junyeop.kim@lge.com, headset/spk tuning for media(HW require) [START_LGE]
	{WM9093_CMD ,0x19, 0x0002},	// 20100517 junyeop.kim@lge.com, headset/spk tuning for media(HW require) [START_LGE]
	{WM9093_CMD ,0x18, 0x0102},
	{WM9093_CMD ,0x17, 0x0001},
	{WM9093_CMD ,0x1A, 0x0002},
	{WM9093_CMD ,0x1B, 0x0002},
	{WM9093_CMD ,0x1A, 0x0102},
    {WM9093_CMD ,0x36, 0x0005},
	{WM9093_CMD ,0x22, 0x0005},
	{WM9093_CMD ,0x25, 0x0178},
	{WM9093_CMD ,0x2D, 0x0040},
	{WM9093_CMD ,0x2E, 0x0010},
    {WM9093_CMD ,0x03, 0x0138},
	{WM9093_CMD ,0x1C, 0x001b},	// 20110224 mikyoung.chang@lge.com hw request // 20121029 bs.lim@lge.com 23->1b
	{WM9093_CMD ,0x1D, 0x011b},	// 20101011 junyeop.kim@lge.com, dual path tuning for call(HW require) [START_LGE] // 20121029 bs.lim@lge.com 23->1b
	{WM9093_CMD ,0x1C, 0x011b},	// 20101011 junyeop.kim@lge.com, dual path tuning for call(HW require) [START_LGE] // 20121029 bs.lim@lge.com 23->1b
	{WM9093_CMD ,0x2F, 0x0000},
	{WM9093_CMD ,0x30, 0x0000},
    {WM9093_CMD ,0x16, 0x0000},
	{WM9093_CMD ,0x17, 0x0000},
    {WM9093_CMD ,0x46, 0x0100},
    {WM9093_CMD ,0x49, 0x0100},
    {WM9093_END_SEQ,0x00,0x00}
};


static const wm9093_reg_type wm9093_in2_to_out_call_tab[] =	//spk call
{
	{WM9093_CMD ,0x39, 0x000D},		//20100720 junyeop.kim@lge.com, fix the call mute
 	{WM9093_CMD ,0x01, 0x000B},
	{WM9093_CMD ,0x02, 0x6020},
// jk6884.lee@lge.com 20121022 P970 Audio parameter
#ifdef CONFIG_PRODUCT_LGE_P970
	{WM9093_CMD ,0x1A, 0x0102},
	{WM9093_CMD ,0x1B, 0x0102},
#else
	{WM9093_CMD ,0x1A, 0x0100},		// 20110126 mikyoung.chang@lge.com, spk tuning for call(HW require) [START_LGE]
	{WM9093_CMD ,0x1B, 0x0100},		// 20110126 mikyoung.chang@lge.com, spk tuning for call(HW require) [START_LGE]
#endif
	{WM9093_CMD ,0x36, 0x0004},
	{WM9093_CMD ,0x03, 0x0008},
	{WM9093_CMD ,0x22, 0x0000},		// 20100816 junyeop.kim@lge.com, spk tuning for call(HW require) [START_LGE]
	{WM9093_CMD ,0x03, 0x0108},
	{WM9093_CMD ,0x24, 0x0010},
	{WM9093_CMD ,0x25, 0x0178},		// 20100816 junyeop.kim@lge.com, spk tuning for call(HW require) [START_LGE]
	{WM9093_CMD ,0x26, 0x013F /* 0x013A*/},		/* jiwon.seo@lge.com 20100121 : spk tuning 0x13F-> 0x13A */
	{WM9093_CMD ,0x17, 0x0000},
	{WM9093_CMD ,0x01, 0x100B},
    {WM9093_END_SEQ,0x00,0x00}
};
//20101120 junyeop.kim@lge.com, voip call tuning[START_LGE]
static const wm9093_reg_type wm9093_in2_to_out_voip_tab[] =
{
	{WM9093_CMD ,0x39, 0x000D},
	{WM9093_CMD ,0x01, 0x000B},
	{WM9093_CMD ,0x02, 0x6030},
	{WM9093_CMD ,0x17, 0x0001},
	{WM9093_CMD ,0x1A, 0x0100},		// 20100604 junyeop.kim@lge.com, spk tuning for media(wolfson require) [START_LGE]
	{WM9093_CMD ,0x1B, 0x0100},		// 20100604 junyeop.kim@lge.com, spk tuning for media(wolfson require) [START_LGE]
	{WM9093_CMD ,0x36, 0x0005},
	{WM9093_CMD ,0x03, 0x0008},
	{WM9093_CMD ,0x22, 0x0000},		// 20100709 junyeop.kim@lge.com, spk tuning for media(HW require) [START_LGE]
	{WM9093_CMD ,0x03, 0x0108},
	{WM9093_CMD ,0x25, 0x0178},		// 20100604 junyeop.kim@lge.com, spk tuning for media(PL require) [START_LGE]
	{WM9093_CMD ,0x24, 0x0010},		// 20100604 junyeop.kim@lge.com, spk tuning for media(PL require) [START_LGE]
//	{WM9093_CMD ,0x26, 0x003F},		// 20100604 junyeop.kim@lge.com, spk tuning for media(PL require) [START_LGE]
	{WM9093_CMD ,0x17, 0x0000},
	{WM9093_CMD ,0x01, 0x100B},
    {WM9093_END_SEQ,0x00,0x00}
};
//20101120 junyeop.kim@lge.com, voip call tuning[END_LGE]

static const wm9093_reg_type wm9093_in2_to_out_tab[] =
{
	{WM9093_CMD ,0x39, 0x000D},
	{WM9093_CMD ,0x01, 0x000B},
	{WM9093_CMD ,0x02, 0x6030},
	{WM9093_CMD ,0x17, 0x0001},
	//{WM9093_CMD ,0x1A, 0x0102},		// 20100604 junyeop.kim@lge.com, spk tuning for media(wolfson require) [START_LGE]
	//{WM9093_CMD ,0x1B, 0x0102},		// 20100604 junyeop.kim@lge.com, spk tuning for media(wolfson require) [START_LGE]
	{WM9093_CMD ,0x1A, 0x0100}, 	// 20100604 junyeop.kim@lge.com, spk tuning for media(wolfson require) [START_LGE]
	{WM9093_CMD ,0x1B, 0x0100}, 	// 20100604 junyeop.kim@lge.com, spk tuning for media(wolfson require) [START_LGE]

	{WM9093_CMD ,0x36, 0x0005},
	{WM9093_CMD ,0x03, 0x0008},
	{WM9093_CMD ,0x22, 0x0000},		// 20100709 junyeop.kim@lge.com, spk tuning for media(HW require) [START_LGE]
	{WM9093_CMD ,0x03, 0x0108},
	{WM9093_CMD ,0x25, 0x0178},		// 20100604 junyeop.kim@lge.com, spk tuning for media(PL require) [START_LGE]
	{WM9093_CMD ,0x24, 0x0010},		// 20100604 junyeop.kim@lge.com, spk tuning for media(PL require) [START_LGE]
	//{WM9093_CMD ,0x26, 0x0132}, 	// 20100604 junyeop.kim@lge.com, spk tuning for media(PL require) [START_LGE]
// jk6884.lee@lge.com 20121022 P970 Audio parameter
#ifdef CONFIG_PRODUCT_LGE_P970
	{WM9093_CMD ,0x26, 0x0138},
#else
	{WM9093_CMD ,0x26, 0x0137},
#endif


	{WM9093_CMD ,0x17, 0x0000},
	{WM9093_CMD ,0x01, 0x100B},
    {WM9093_END_SEQ,0x00,0x00}
};

static const wm9093_reg_type wm9093_test_tab[] =
{
	{WM9093_DELAY,0x39, 0x0050},
	{WM9093_CMD ,0x39, 0x000D},
	{WM9093_CMD ,0x01, 0x000B},
	{WM9093_DELAY,0x00, 50},
	{WM9093_CMD ,0x02, 0x6030},
	{WM9093_CMD ,0x17, 0x0001},
	{WM9093_CMD ,0x1A, 0x0100},
	{WM9093_CMD ,0x1B, 0x0100},
	{WM9093_CMD ,0x2D, 0x0004},
	{WM9093_CMD ,0x2E, 0x0001},
	{WM9093_CMD ,0x03, 0x0030},
	{WM9093_CMD ,0x2F, 0x0000},
	{WM9093_CMD ,0x30, 0x0000},
	{WM9093_CMD ,0x17, 0x0000},
	{WM9093_CMD ,0x1C, 0x0135},
	{WM9093_CMD ,0x1D, 0x0135},
	{WM9093_CMD ,0x1C, 0x0135},
	{WM9093_CMD ,0x46, 0x0100},
	{WM9093_CMD ,0x49, 0x0100},
    {WM9093_END_SEQ,0x00,0x00}

};


/* LGE_CHANGE_S [iggikim@lge.com] 2009-11-10, rev a headset */
static const wm9093_reg_type wm9093_in2_to_hp_tab[] =	// headset call
{
#if 0	// 20100528 junyeop.kim@lge.com, remove the pop noise - wolfson path 4 [START_LGE]  //stereo
    {WM9093_CMD ,0x39, 0x000D},
    {WM9093_CMD ,0x01, 0x000B},
	{WM9093_CMD ,0x02, 0x60C0},
	{WM9093_CMD ,0x16, 0x0001},
	{WM9093_CMD ,0x18, 0x0101},	// 20100910 junyeop.kim@lge.com, headset tuning for call(HW require) [START_LGE]
	{WM9093_CMD ,0x19, 0x0101},	// 20100910 junyeop.kim@lge.com, headset tuning for call(HW require) [START_LGE]
	{WM9093_CMD ,0x2D, 0x0040},
	{WM9093_CMD ,0x2E, 0x0010},
    {WM9093_CMD ,0x03, 0x0030},
    {WM9093_CMD ,0x2F, 0x0040},	// 20100816 junyeop.kim@lge.com, headset tuning for call(HW require) [START_LGE]
    {WM9093_CMD ,0x30, 0x0010},	// 20100816 junyeop.kim@lge.com, headset tuning for call(HW require) [START_LGE]
    {WM9093_CMD ,0x16, 0x0000},
	{WM9093_CMD ,0x1C, 0x002f},	// 20100910 junyeop.kim@lge.com, headset tuning for call(HW require) [START_LGE]
	{WM9093_CMD ,0x1D, 0x002f},	// 20100910 junyeop.kim@lge.com, headset tuning for call(HW require) [START_LGE]
	{WM9093_CMD ,0x1C, 0x012f},	// 20100910 junyeop.kim@lge.com, headset tuning for call(HW require) [START_LGE]
    {WM9093_CMD ,0x46, 0x0100},
    {WM9093_CMD ,0x49, 0x0100},
    {WM9093_END_SEQ,0x00,0x00}
#else	// 20100920 junyeop.kim@lge.com, remove the pop noise - wolfson path 1 [START_LGE]  //mono
    {WM9093_CMD ,0x39, 0x000D},
    {WM9093_CMD ,0x01, 0x000B},
	{WM9093_CMD ,0x02, 0x6040},
	{WM9093_CMD ,0x16, 0x0001},
	{WM9093_CMD ,0x18, 0x0100},	// 20100910 junyeop.kim@lge.com, headset tuning for call(HW require) [START_LGE]
	{WM9093_CMD ,0x19, 0x0100},	// 20100910 junyeop.kim@lge.com, headset tuning for call(HW require) [START_LGE]
	{WM9093_CMD ,0x2D, 0x0010},
	{WM9093_CMD ,0x2E, 0x0010},
	{WM9093_CMD ,0x03, 0x0030},
    {WM9093_CMD ,0x2F, 0x0000},	// 20100816 junyeop.kim@lge.com, headset tuning for call(HW require) [START_LGE]
    {WM9093_CMD ,0x30, 0x0000},	// 20100816 junyeop.kim@lge.com, headset tuning for call(HW require) [START_LGE]
	{WM9093_CMD ,0x16, 0x0000},
	
// jk6884.lee@lge.com 20121022 P970 Audio parameter // 20121026 133 -> 135
#ifdef CONFIG_PRODUCT_LGE_P970
	{WM9093_CMD ,0x1C, 0x0131}, // 20121030 bs.lim@lge.com 35->31->30->31
	{WM9093_CMD ,0x1D, 0x0131}, // 20121030 bs.lim@lge.com 35->31->30->31
	{WM9093_CMD ,0x1C, 0x0131}, // 20121030 bs.lim@lge.com 35->31->30->31
#else
	{WM9093_CMD ,0x1C, 0x0133},	// 20110224 mikyoung.chang@lge.com hw request
	{WM9093_CMD ,0x1D, 0x0133},	// 20110224 mikyoung.chang@lge.com hw request
	{WM9093_CMD ,0x1C, 0x0133},	// 20110224 mikyoung.chang@lge.com hw request
#endif
	{WM9093_CMD ,0x46, 0x0100},
	{WM9093_CMD ,0x49, 0x0100},
    {WM9093_END_SEQ,0x00,0x00}
#endif
};
/* LGE_CHANGE_E [iggikim@lge.com] */
static const wm9093_reg_type wm9093_in3_to_out_tab[] =
{
	{WM9093_CMD ,0x39, 0x000D},		//20100720 junyeop.kim@lge.com, fix the call mute
	{WM9093_CMD ,0x01, 0x000B},
	{WM9093_CMD ,0x22, 0x1100},
	{WM9093_CMD ,0x24, 0x0020},
	{WM9093_CMD ,0x15, 0x0000},
//	{WM9093_CMD ,0x25, 0x01e0},     //20100912 junyeop.kim@lge.com, receiver tuning (HW request)
	{WM9093_CMD ,0x25, 0x01d8},     //20100912 junyeop.kim@lge.com, receiver tuning (HW request)
	{WM9093_CMD ,0x01, 0x100B},
    {WM9093_END_SEQ,0x00,0x00}
};

//20101120 junyeop.kim@lge.com, voip call tuning[START_LGE]
static const wm9093_reg_type wm9093_in3_to_out_voip_tab[] =
{
	{WM9093_CMD ,0x39, 0x000D},		//20100720 junyeop.kim@lge.com, fix the call mute
	{WM9093_CMD ,0x01, 0x000B},
	{WM9093_CMD ,0x22, 0x1100},
	{WM9093_CMD ,0x24, 0x0020},
	{WM9093_CMD ,0x15, 0x0000},
	{WM9093_CMD ,0x25, 0x01d8},
	{WM9093_CMD ,0x01, 0x100B},
    {WM9093_END_SEQ,0x00,0x00}
};
//20101120 junyeop.kim@lge.com, voip call tuning[END_LGE]

static const wm9093_reg_type wm9093_pwroff_tab[] =
{
#if 1 	//shut down sequence
    {WM9093_CMD ,0x46, 0x0100},
    {WM9093_CMD ,0x49, 0x0110},
    {WM9093_CMD ,0x00, 0x0000},
    {WM9093_CMD ,0x02, 0x6000},
    {WM9093_CMD ,0x39, 0x0000},
//    {WM9093_DELAY,0x39, 0x0200},    //added by jykim
#else
    {WM9093_CMD ,0x00, 0x9093},
#endif
    {WM9093_END_SEQ,0x00,0x00}
};

#else			/* CONFIG_PRODUCT_LGE_HUB */

static const wm9093_reg_type wm9093_in1_to_out_tab[] =
{
  	{WM9093_CMD ,0x01, 0x000B},
	{WM9093_CMD ,0x02, 0x60C0},
	{WM9093_CMD ,0x16, 0x0001},
	{WM9093_CMD ,0x18, 0x0002},
	{WM9093_CMD ,0x19, 0x0002},
	{WM9093_CMD ,0x18, 0x0102},
	{WM9093_CMD ,0x36, 0x0050},
	{WM9093_CMD ,0x03, 0x0008},
	{WM9093_CMD ,0x22, 0x0050},
	{WM9093_CMD ,0x03, 0x0108},
	{WM9093_CMD ,0x25, 0x0160},
	{WM9093_CMD ,0x16, 0x0000},
	{WM9093_CMD ,0x01, 0x100B},
    {WM9093_END_SEQ,0x00,0x00}
};

static const wm9093_reg_type wm9093_in1_to_hp_tab[] =
{
#if 1	// 20100528 junyeop.kim@lge.com, remove the pop noise - wolfson path 4 [START_LGE]
	{WM9093_DELAY,0x39, 0x0050},
	{WM9093_CMD ,0x39, 0x000D},
	{WM9093_CMD ,0x01, 0x000B},
	{WM9093_CMD ,0x02, 0x60C0},
	{WM9093_CMD ,0x16, 0x0001},
	{WM9093_CMD ,0x18, 0x0100},	// 20100517 junyeop.kim@lge.com, headset tuning for media(HW require) [START_LGE]
	{WM9093_CMD ,0x19, 0x0100},	// 20100517 junyeop.kim@lge.com, headset tuning for media(HW require) [START_LGE]
	{WM9093_CMD ,0x2D, 0x0040},
	{WM9093_CMD ,0x2E, 0x0010},
	{WM9093_CMD ,0x03, 0x0030},
	{WM9093_CMD ,0x2F, 0x0000},
	{WM9093_CMD ,0x30, 0x0000},
	{WM9093_CMD ,0x1C, 0x0032},	// 20100517 junyeop.kim@lge.com, headset tuning for media(HW require) [START_LGE]
	{WM9093_CMD ,0x1D, 0x0032},	// 20100517 junyeop.kim@lge.com, headset tuning for media(HW require) [START_LGE]
	{WM9093_CMD ,0x1C, 0x0132},	// 20100517 junyeop.kim@lge.com, headset tuning for media(HW require) [START_LGE]
	{WM9093_CMD ,0x16, 0x0000},
	{WM9093_DELAY,0x39, 0x0010},
	{WM9093_CMD ,0x46, 0x0100},
	{WM9093_CMD ,0x49, 0x0100},
    {WM9093_END_SEQ,0x00,0x00}
#else
    {WM9093_CMD ,0x01, 0x000B},
	{WM9093_CMD ,0x02, 0x60C0},
	{WM9093_CMD ,0x16, 0x0001},
	{WM9093_CMD ,0x18, 0x0000},	// 20100517 junyeop.kim@lge.com, headset tuning for media(HW require) [START_LGE]
	{WM9093_CMD ,0x19, 0x0000},	// 20100517 junyeop.kim@lge.com, headset tuning for media(HW require) [START_LGE]
	{WM9093_CMD ,0x18, 0x0101},	// 20100517 junyeop.kim@lge.com, headset tuning for media(HW require) [START_LGE]
	{WM9093_CMD ,0x2D, 0x0040},
	{WM9093_CMD ,0x2E, 0x0010},
	{WM9093_CMD ,0x03, 0x0030},
	{WM9093_CMD ,0x2F, 0x0000},
	{WM9093_CMD ,0x30, 0x0000},
	{WM9093_CMD ,0x16, 0x0000},
	{WM9093_CMD ,0x1C, 0x0036},	// 20100517 junyeop.kim@lge.com, headset tuning for media(HW require) [START_LGE]
	{WM9093_CMD ,0x1D, 0x0036},	// 20100517 junyeop.kim@lge.com, headset tuning for media(HW require) [START_LGE]
	{WM9093_CMD ,0x1C, 0x0136},	// 20100517 junyeop.kim@lge.com, headset tuning for media(HW require) [START_LGE]
	{WM9093_CMD ,0x46, 0x0100},
	{WM9093_CMD ,0x49, 0x0100},
    {WM9093_END_SEQ,0x00,0x00}
#endif
};

//20101120 junyeop.kim@lge.com, voip call tuning[START_LGE]
static const wm9093_reg_type wm9093_in1_to_hp_voip_tab[] =
{
	{WM9093_DELAY,0x39, 0x0050},
	{WM9093_CMD ,0x39, 0x000D},
	{WM9093_CMD ,0x01, 0x000B},
	{WM9093_CMD ,0x02, 0x60C0},
	{WM9093_CMD ,0x16, 0x0001},
	{WM9093_CMD ,0x18, 0x0100},	// 20100517 junyeop.kim@lge.com, headset tuning for media(HW require) [START_LGE]
	{WM9093_CMD ,0x19, 0x0100},	// 20100517 junyeop.kim@lge.com, headset tuning for media(HW require) [START_LGE]
	{WM9093_CMD ,0x2D, 0x0040},
	{WM9093_CMD ,0x2E, 0x0010},
	{WM9093_CMD ,0x03, 0x0030},
	{WM9093_CMD ,0x2F, 0x0000},
	{WM9093_CMD ,0x30, 0x0000},
	{WM9093_CMD ,0x16, 0x0000},
	{WM9093_CMD ,0x1C, 0x0033},	// 20100517 junyeop.kim@lge.com, headset tuning for media(HW require) [START_LGE]
	{WM9093_CMD ,0x1D, 0x0033},	// 20100517 junyeop.kim@lge.com, headset tuning for media(HW require) [START_LGE]
	{WM9093_CMD ,0x1C, 0x0133},	// 20100517 junyeop.kim@lge.com, headset tuning for media(HW require) [START_LGE]
	{WM9093_CMD ,0x46, 0x0100},
	{WM9093_CMD ,0x49, 0x0100},
    {WM9093_END_SEQ,0x00,0x00}
};
//20101120 junyeop.kim@lge.com, voip call tuning[END_LGE]


static const wm9093_reg_type wm9093_in2_to_out_hp_tab[] =
{
#if 0	// 20100531 junyeop.kim@lge.com, remove the pop noise - wolfson path 12[START_LGE]
	{WM9093_CMD ,0x39, 0x000D},
	{WM9093_CMD ,0x01, 0x100B},
	{WM9093_CMD ,0x02, 0x6030},
	{WM9093_CMD ,0x17, 0x0001},
	{WM9093_CMD ,0x1A, 0x0002},
	{WM9093_CMD ,0x1B, 0x0002},
	{WM9093_CMD ,0x1A, 0x0102},
	{WM9093_CMD ,0x36, 0x0005},
	{WM9093_CMD ,0x22, 0x0005},
	{WM9093_CMD ,0x25, 0x0178},
//	{WM9093_CMD ,0x26, 0x003a},		// 20100604 junyeop.kim@lge.com, spk tuning for media(PL require) [START_LGE]
	{WM9093_CMD ,0x2D, 0x0004},
	{WM9093_CMD ,0x2E, 0x0001},
	{WM9093_CMD ,0x03, 0x0138},
	{WM9093_CMD ,0x1C, 0x001c},	// 20101011 junyeop.kim@lge.com, dual path tuning for call(HW require) [START_LGE]
	{WM9093_CMD ,0x1D, 0x011c},	// 20101011 junyeop.kim@lge.com, dual path tuning for call(HW require) [START_LGE]
	{WM9093_CMD ,0x1C, 0x011c},	// 20101011 junyeop.kim@lge.com, dual path tuning for call(HW require) [START_LGE]
	{WM9093_CMD ,0x2F, 0x0000},
	{WM9093_CMD ,0x30, 0x0000},
	{WM9093_CMD ,0x17, 0x0000},
	{WM9093_CMD ,0x46, 0x0100},
	{WM9093_CMD ,0x49, 0x0100},
    {WM9093_END_SEQ,0x00,0x00}
#else
#if 0	//20101018 junyeop.kim@lge.com, dual path success - wolfson path 6[START_LGE]
 	{WM9093_CMD ,0x39, 0x000D},
    {WM9093_CMD ,0x01, 0x130B},
	{WM9093_CMD ,0x02, 0x60c0},
    {WM9093_CMD ,0x16, 0x0001},
	{WM9093_CMD ,0x18, 0x0002},	// 20100517 junyeop.kim@lge.com, headset/spk tuning for media(HW require) [START_LGE]
	{WM9093_CMD ,0x19, 0x0002},	// 20100517 junyeop.kim@lge.com, headset/spk tuning for media(HW require) [START_LGE]
	{WM9093_CMD ,0x18, 0x0102},
    {WM9093_CMD ,0x36, 0x0050},
	{WM9093_CMD ,0x22, 0x0050},
	{WM9093_CMD ,0x25, 0x0178},  // 20100522 junyeop.kim@lge.com, headset/spk tuning for media(HW require) [START_LGE]
	{WM9093_CMD ,0x2D, 0x0040},
	{WM9093_CMD ,0x2E, 0x0010},
    {WM9093_CMD ,0x03, 0x0138},
	{WM9093_CMD ,0x1C, 0x001c},	// 20101011 junyeop.kim@lge.com, dual path tuning for call(HW require) [START_LGE]
	{WM9093_CMD ,0x1D, 0x011c},	// 20101011 junyeop.kim@lge.com, dual path tuning for call(HW require) [START_LGE]
	{WM9093_CMD ,0x1C, 0x011c},	// 20101011 junyeop.kim@lge.com, dual path tuning for call(HW require) [START_LGE]
	{WM9093_CMD ,0x2F, 0x0000},
	{WM9093_CMD ,0x30, 0x0000},
    {WM9093_CMD ,0x16, 0x0000},
	{WM9093_CMD ,0x46, 0x0100},
	{WM9093_CMD ,0x49, 0x0100},
    {WM9093_END_SEQ,0x00,0x00}
#else	//dual path in1 -> headset, in2->spk
 	{WM9093_CMD ,0x39, 0x000D},
    {WM9093_CMD ,0x01, 0x130B},
	{WM9093_CMD ,0x02, 0x60f0},
    {WM9093_CMD ,0x16, 0x0001},
	{WM9093_CMD ,0x18, 0x0002},	// 20100517 junyeop.kim@lge.com, headset/spk tuning for media(HW require) [START_LGE]
	{WM9093_CMD ,0x19, 0x0002},	// 20100517 junyeop.kim@lge.com, headset/spk tuning for media(HW require) [START_LGE]
	{WM9093_CMD ,0x18, 0x0102},
	{WM9093_CMD ,0x17, 0x0001},
	{WM9093_CMD ,0x1A, 0x0002},
	{WM9093_CMD ,0x1B, 0x0002},
	{WM9093_CMD ,0x1A, 0x0102},
    {WM9093_CMD ,0x36, 0x0005},
	{WM9093_CMD ,0x22, 0x0005},
	{WM9093_CMD ,0x25, 0x0178},
	{WM9093_CMD ,0x2D, 0x0040},
	{WM9093_CMD ,0x2E, 0x0010},
    {WM9093_CMD ,0x03, 0x0138},
	{WM9093_CMD ,0x1C, 0x0023},	// 20101011 junyeop.kim@lge.com, dual path tuning for call(HW require) [START_LGE]
	{WM9093_CMD ,0x1D, 0x0123},	// 20101011 junyeop.kim@lge.com, dual path tuning for call(HW require) [START_LGE]
	{WM9093_CMD ,0x1C, 0x0123},	// 20101011 junyeop.kim@lge.com, dual path tuning for call(HW require) [START_LGE]
	{WM9093_CMD ,0x2F, 0x0000},
	{WM9093_CMD ,0x30, 0x0000},
    {WM9093_CMD ,0x16, 0x0000},
	{WM9093_CMD ,0x17, 0x0000},
    {WM9093_CMD ,0x46, 0x0100},
    {WM9093_CMD ,0x49, 0x0100},
    {WM9093_END_SEQ,0x00,0x00}

#endif
#endif
};


static const wm9093_reg_type wm9093_in2_to_out_call_tab[] =	//spk call
{
#if 1	//spk call mono, wolfson path 8 & modify the mono, single-ended
	{WM9093_CMD ,0x39, 0x000D},		//20100720 junyeop.kim@lge.com, fix the call mute
 	{WM9093_CMD ,0x01, 0x000B},
	{WM9093_CMD ,0x02, 0x6020},
//	{WM9093_CMD ,0x1A, 0x0000},
//	{WM9093_CMD ,0x1B, 0x0000},
	{WM9093_CMD ,0x1A, 0x0102},
	{WM9093_CMD ,0x1B, 0x0102},		// 20100816 junyeop.kim@lge.com, spk tuning for call(HW require) [START_LGE]
	{WM9093_CMD ,0x36, 0x0004},
	{WM9093_CMD ,0x03, 0x0008},
	{WM9093_CMD ,0x22, 0x0000},		// 20100816 junyeop.kim@lge.com, spk tuning for call(HW require) [START_LGE]
	{WM9093_CMD ,0x03, 0x0108},
	{WM9093_CMD ,0x24, 0x0010},
	{WM9093_CMD ,0x25, 0x0178},		// 20100816 junyeop.kim@lge.com, spk tuning for call(HW require) [START_LGE]
	{WM9093_CMD ,0x26, 0x013f},		// 20100816 junyeop.kim@lge.com, spk tuning for call(HW require) [START_LGE]
	{WM9093_CMD ,0x17, 0x0000},
	{WM9093_CMD ,0x01, 0x100B},
    {WM9093_END_SEQ,0x00,0x00}
#else	//spk call stereo
	{WM9093_CMD ,0x39, 0x000D},		//20100720 junyeop.kim@lge.com, fix the call mute
 	{WM9093_CMD ,0x01, 0x000B},
	{WM9093_CMD ,0x02, 0x6030},
	{WM9093_CMD ,0x1A, 0x0005},		// 20100816 junyeop.kim@lge.com, spk tuning for call(HW require) [START_LGE]
	{WM9093_CMD ,0x1B, 0x0005},		// 20100816 junyeop.kim@lge.com, spk tuning for call(HW require) [START_LGE]
	{WM9093_CMD ,0x1A, 0x0105},		// 20100816 junyeop.kim@lge.com, spk tuning for call(HW require) [START_LGE]
	{WM9093_CMD ,0x1B, 0x0105},		// 20100816 junyeop.kim@lge.com, spk tuning for call(HW require) [START_LGE]
	{WM9093_CMD ,0x36, 0x0005},
	{WM9093_CMD ,0x03, 0x0008},
	{WM9093_CMD ,0x22, 0x0011},		// 20100816 junyeop.kim@lge.com, spk tuning for call(HW require) [START_LGE]
	{WM9093_CMD ,0x03, 0x0108},
	{WM9093_CMD ,0x24, 0x0010},
	{WM9093_CMD ,0x25, 0x0170},		// 20100816 junyeop.kim@lge.com, spk tuning for call(HW require) [START_LGE]
	{WM9093_CMD ,0x26, 0x0039},		// 20100816 junyeop.kim@lge.com, spk tuning for call(HW require) [START_LGE]
	{WM9093_CMD ,0x17, 0x0000},
	{WM9093_CMD ,0x01, 0x100B},
    {WM9093_END_SEQ,0x00,0x00}

#endif
};
//20101120 junyeop.kim@lge.com, voip call tuning[START_LGE]
static const wm9093_reg_type wm9093_in2_to_out_voip_tab[] =
{
	{WM9093_CMD ,0x39, 0x000D},
	{WM9093_CMD ,0x01, 0x000B},
	{WM9093_CMD ,0x02, 0x6030},
	{WM9093_CMD ,0x17, 0x0001},
	{WM9093_CMD ,0x1A, 0x0100},		// 20100604 junyeop.kim@lge.com, spk tuning for media(wolfson require) [START_LGE]
	{WM9093_CMD ,0x1B, 0x0100},		// 20100604 junyeop.kim@lge.com, spk tuning for media(wolfson require) [START_LGE]
	{WM9093_CMD ,0x36, 0x0005},
	{WM9093_CMD ,0x03, 0x0008},
	{WM9093_CMD ,0x22, 0x0000},		// 20100709 junyeop.kim@lge.com, spk tuning for media(HW require) [START_LGE]
	{WM9093_CMD ,0x03, 0x0108},
	{WM9093_CMD ,0x25, 0x0178},		// 20100604 junyeop.kim@lge.com, spk tuning for media(PL require) [START_LGE]
	{WM9093_CMD ,0x24, 0x0010},		// 20100604 junyeop.kim@lge.com, spk tuning for media(PL require) [START_LGE]
//	{WM9093_CMD ,0x26, 0x003F},		// 20100604 junyeop.kim@lge.com, spk tuning for media(PL require) [START_LGE]
	{WM9093_CMD ,0x17, 0x0000},
	{WM9093_CMD ,0x01, 0x100B},
    {WM9093_END_SEQ,0x00,0x00}
};
//20101120 junyeop.kim@lge.com, voip call tuning[END_LGE]

static const wm9093_reg_type wm9093_in2_to_out_tab[] =
{
#if 1 // 20100531 junyeop.kim@lge.com, remove the pop noise - wolfson path 11[START_LGE]
	{WM9093_CMD ,0x39, 0x000D},
	{WM9093_CMD ,0x01, 0x000B},
	{WM9093_CMD ,0x02, 0x6030},
	{WM9093_CMD ,0x17, 0x0001},
	{WM9093_CMD ,0x1A, 0x0100},		// 20100604 junyeop.kim@lge.com, spk tuning for media(wolfson require) [START_LGE]
	{WM9093_CMD ,0x1B, 0x0100},		// 20100604 junyeop.kim@lge.com, spk tuning for media(wolfson require) [START_LGE]
	{WM9093_CMD ,0x36, 0x0005},
	{WM9093_CMD ,0x03, 0x0008},
	{WM9093_CMD ,0x22, 0x0000},		// 20100709 junyeop.kim@lge.com, spk tuning for media(HW require) [START_LGE]
	{WM9093_CMD ,0x03, 0x0108},
	{WM9093_CMD ,0x25, 0x0178},		// 20100604 junyeop.kim@lge.com, spk tuning for media(PL require) [START_LGE]
	{WM9093_CMD ,0x24, 0x0010},		// 20100604 junyeop.kim@lge.com, spk tuning for media(PL require) [START_LGE]
//	{WM9093_CMD ,0x26, 0x003F},		// 20100604 junyeop.kim@lge.com, spk tuning for media(PL require) [START_LGE]
	{WM9093_CMD ,0x17, 0x0000},
	{WM9093_CMD ,0x01, 0x100B},
    {WM9093_END_SEQ,0x00,0x00}
#else
 	{WM9093_CMD ,0x01, 0x000B},
	{WM9093_CMD ,0x02, 0x6030},
	{WM9093_CMD ,0x1A, 0x0000},		// 20100515 junyeop.kim@lge.com, spk tuning for media(HW require) [START_LGE]
	{WM9093_CMD ,0x1B, 0x0000},		// 20100515 junyeop.kim@lge.com, spk tuning for media(HW require) [START_LGE]
	{WM9093_CMD ,0x1A, 0x0100},		// 20100515 junyeop.kim@lge.com, spk tuning for media(HW require) [START_LGE]
	{WM9093_CMD ,0x1B, 0x0100},		// 20100515 junyeop.kim@lge.com, spk tuning for media(HW require) [START_LGE]
	{WM9093_CMD ,0x36, 0x0005},
	{WM9093_CMD ,0x03, 0x0008},
	{WM9093_CMD ,0x22, 0x0000},		// 20100515 junyeop.kim@lge.com, spk tuning for media(HW require) [START_LGE]
	{WM9093_CMD ,0x03, 0x0108},
	{WM9093_CMD ,0x24, 0x0010},
	{WM9093_CMD ,0x25, 0x0170},		// 20100515 junyeop.kim@lge.com, spk tuning for media(HW require) [START_LGE]
	{WM9093_CMD ,0x26, 0x0039},		// 20100515 junyeop.kim@lge.com, spk tuning for media(HW require) [START_LGE]
	{WM9093_CMD ,0x17, 0x0000},
	{WM9093_CMD ,0x01, 0x100B},
    {WM9093_END_SEQ,0x00,0x00}
#endif
};

//--[[ LGE_UBIQUIX_MODIFIED_START : bsnoh@ubiquix.com - add from justin_froyo
static const wm9093_reg_type wm9093_test_tab[] =
{
	{WM9093_DELAY,0x39, 0x0050},
	{WM9093_CMD ,0x39, 0x000D},
	{WM9093_CMD ,0x01, 0x000B},
	{WM9093_DELAY,0x00, 50},
	{WM9093_CMD ,0x02, 0x6030},
	{WM9093_CMD ,0x17, 0x0001},
	{WM9093_CMD ,0x1A, 0x0100},
	{WM9093_CMD ,0x1B, 0x0100},
	{WM9093_CMD ,0x2D, 0x0004},
	{WM9093_CMD ,0x2E, 0x0001},
	{WM9093_CMD ,0x03, 0x0030},
	{WM9093_CMD ,0x2F, 0x0000},
	{WM9093_CMD ,0x30, 0x0000},
	{WM9093_CMD ,0x17, 0x0000},
	{WM9093_CMD ,0x1C, 0x0135},
	{WM9093_CMD ,0x1D, 0x0135},
	{WM9093_CMD ,0x1C, 0x0135},
	{WM9093_CMD ,0x46, 0x0100},
	{WM9093_CMD ,0x49, 0x0100},
    {WM9093_END_SEQ,0x00,0x00}

};
//--]] LGE_UBIQUIX_MODIFIED_END : bsnoh@ubiquix.com - add from justin_froyo


/* LGE_CHANGE_S [iggikim@lge.com] 2009-11-10, rev a headset */
static const wm9093_reg_type wm9093_in2_to_hp_tab[] =	// headset call
{
#if 0	// 20100528 junyeop.kim@lge.com, remove the pop noise - wolfson path 4 [START_LGE]  //stereo
    {WM9093_CMD ,0x39, 0x000D},
    {WM9093_CMD ,0x01, 0x000B},
	{WM9093_CMD ,0x02, 0x60C0},
	{WM9093_CMD ,0x16, 0x0001},
	{WM9093_CMD ,0x18, 0x0101},	// 20100910 junyeop.kim@lge.com, headset tuning for call(HW require) [START_LGE]
	{WM9093_CMD ,0x19, 0x0101},	// 20100910 junyeop.kim@lge.com, headset tuning for call(HW require) [START_LGE]
	{WM9093_CMD ,0x2D, 0x0040},
	{WM9093_CMD ,0x2E, 0x0010},
    {WM9093_CMD ,0x03, 0x0030},
    {WM9093_CMD ,0x2F, 0x0040},	// 20100816 junyeop.kim@lge.com, headset tuning for call(HW require) [START_LGE]
    {WM9093_CMD ,0x30, 0x0010},	// 20100816 junyeop.kim@lge.com, headset tuning for call(HW require) [START_LGE]
    {WM9093_CMD ,0x16, 0x0000},
	{WM9093_CMD ,0x1C, 0x002f},	// 20100910 junyeop.kim@lge.com, headset tuning for call(HW require) [START_LGE]
	{WM9093_CMD ,0x1D, 0x002f},	// 20100910 junyeop.kim@lge.com, headset tuning for call(HW require) [START_LGE]
	{WM9093_CMD ,0x1C, 0x012f},	// 20100910 junyeop.kim@lge.com, headset tuning for call(HW require) [START_LGE]
    {WM9093_CMD ,0x46, 0x0100},
    {WM9093_CMD ,0x49, 0x0100},
    {WM9093_END_SEQ,0x00,0x00}
#else	// 20100920 junyeop.kim@lge.com, remove the pop noise - wolfson path 1 [START_LGE]  //mono
    {WM9093_CMD ,0x39, 0x000D},
    {WM9093_CMD ,0x01, 0x000B},
	{WM9093_CMD ,0x02, 0x6040},
	{WM9093_CMD ,0x16, 0x0001},
	{WM9093_CMD ,0x18, 0x0100},	// 20100910 junyeop.kim@lge.com, headset tuning for call(HW require) [START_LGE]
	{WM9093_CMD ,0x19, 0x0100},	// 20100910 junyeop.kim@lge.com, headset tuning for call(HW require) [START_LGE]
	{WM9093_CMD ,0x2D, 0x0010},
	{WM9093_CMD ,0x2E, 0x0010},
	{WM9093_CMD ,0x03, 0x0030},
    {WM9093_CMD ,0x2F, 0x0000},	// 20100816 junyeop.kim@lge.com, headset tuning for call(HW require) [START_LGE]
    {WM9093_CMD ,0x30, 0x0000},	// 20100816 junyeop.kim@lge.com, headset tuning for call(HW require) [START_LGE]
	{WM9093_CMD ,0x16, 0x0000},
	{WM9093_CMD ,0x1C, 0x0034},	// 20101011 junyeop.kim@lge.com, headset tuning for call(HW require) [START_LGE]
	{WM9093_CMD ,0x1D, 0x0134},	// 20101011 junyeop.kim@lge.com, headset tuning for call(HW require) [START_LGE]
	{WM9093_CMD ,0x1C, 0x0134},	// 20100910 junyeop.kim@lge.com, headset tuning for call(HW require) [START_LGE]
	{WM9093_CMD ,0x46, 0x0100},
	{WM9093_CMD ,0x49, 0x0100},
    {WM9093_END_SEQ,0x00,0x00}
#endif
};
/* LGE_CHANGE_E [iggikim@lge.com] */
static const wm9093_reg_type wm9093_in3_to_out_tab[] =
{
	{WM9093_CMD ,0x39, 0x000D},		//20100720 junyeop.kim@lge.com, fix the call mute
	{WM9093_CMD ,0x01, 0x000B},
	{WM9093_CMD ,0x22, 0x1100},
	{WM9093_CMD ,0x24, 0x0020},
	{WM9093_CMD ,0x15, 0x0000},
//	{WM9093_CMD ,0x25, 0x01e0},     //20100912 junyeop.kim@lge.com, receiver tuning (HW request)
	{WM9093_CMD ,0x25, 0x01d8},     //20100912 junyeop.kim@lge.com, receiver tuning (HW request)
	{WM9093_CMD ,0x01, 0x100B},
    {WM9093_END_SEQ,0x00,0x00}
};

//20101120 junyeop.kim@lge.com, voip call tuning[START_LGE]
static const wm9093_reg_type wm9093_in3_to_out_voip_tab[] =
{
	{WM9093_CMD ,0x39, 0x000D},		//20100720 junyeop.kim@lge.com, fix the call mute
	{WM9093_CMD ,0x01, 0x000B},
	{WM9093_CMD ,0x22, 0x1100},
	{WM9093_CMD ,0x24, 0x0020},
	{WM9093_CMD ,0x15, 0x0000},
	{WM9093_CMD ,0x25, 0x01d8},
	{WM9093_CMD ,0x01, 0x100B},
    {WM9093_END_SEQ,0x00,0x00}
};
//20101120 junyeop.kim@lge.com, voip call tuning[END_LGE]

static const wm9093_reg_type wm9093_pwroff_tab[] =
{
#if 1 	//shut down sequence
    {WM9093_CMD ,0x46, 0x0100},
    {WM9093_CMD ,0x49, 0x0110},
    {WM9093_CMD ,0x00, 0x0000},
    {WM9093_CMD ,0x02, 0x6000},
    {WM9093_CMD ,0x39, 0x0000},
//    {WM9093_DELAY,0x39, 0x0200},    //added by jykim
#else
    {WM9093_CMD ,0x00, 0x9093},
#endif
    {WM9093_END_SEQ,0x00,0x00}
};

#endif /* Justin or Black or Hub */

static unsigned int wm9093_read_reg(struct i2c_client *client, unsigned char reg)
{
	struct i2c_msg xfer[2];
	u16 data = 0xffff;
	int ret;
	DBG("\n");

	/* Write register */
	xfer[0].addr = client->addr;
	xfer[0].flags = 0;
	xfer[0].len = 1;
	xfer[0].buf = &reg;

	/* Read data */
	xfer[1].addr = client->addr;
	xfer[1].flags = I2C_M_RD;
	xfer[1].len = 2;
	xfer[1].buf = (u8 *)&data;

	ret = i2c_transfer(client->adapter, xfer, 2);

	return (data >> 8) | ((data & 0xff) << 8);
}

static int wm9093_write_reg(struct i2c_client *client, u8 reg, int val)
{
#if defined(CONFIG_PRODUCT_LGE_LU6800) || defined(CONFIG_PRODUCT_LGE_KU5900) || defined (CONFIG_PRODUCT_LGE_P970)	// dajin.kim temp code
	int ret;
	int retry = 3;

	DBG("\n");
	val = ((val & 0xFF00) >> 8) | ((val & 0xFF) << 8);

	while (retry--) {
		ret = i2c_smbus_write_word_data(client, reg, val);

		if (ret < 0) {
			dev_err(&client->dev, "%s: err %d\n", __func__, ret);
		} else {
			break;
		}
	}

	return ret;

#else	// original code /* CONFIG_PRODUCT_LGE_HUB */
	int err;
	DBG("\n");

	struct i2c_msg	msg;
        u8 buf[3];

	msg.addr = (u16)client->addr;
	msg.flags =0;
	msg.len =3;

    buf[0]= reg;
	buf[1]= (val & 0xFF00) >> 8;
    buf[2]= val & 0x00FF;

	msg.buf = &buf[0];

	if ((err = i2c_transfer(client->adapter, &msg, 1)) < 0) {
		dev_err(&client->dev, "i2c write error\n");
	}
	DBG("\n");

	return err;
#endif
}

void wm_delay_msec(int msec)
{
	unsigned long start;
	start = jiffies;
	DBG("\n");
	while (time_before(jiffies, start + (msec * HZ) / 1000)) {
		udelay(10);
	}
}

void wm9093_write_table(wm9093_reg_type* table)
{
	int i;
	int result =0;
	DBG("\n");

	for (i = 0; table[i].irc != WM9093_END_SEQ; i++) {
		if (table[i].irc == WM9093_DELAY) {
			wm_delay_msec(table[i].data);
		} else {
			if (wm9093_i2c_dev != NULL)
				result= wm9093_write_reg(wm9093_i2c_dev->client, table[i].address, table[i].data);
			else
				printk(KERN_ERR "wm9093 i2c_dev is null");
		}

		if (result < 0) break;
	}
}

int pre_voice_curmode = 0x00;	//audio mode

#if 1	//junyeop.kim@lge.com, call initial noise workaround
static void callmode_set_work(struct work_struct *work)
{
	DBG("\n");
	printk("[LUCKYJUN77] CALLMODE_SET_WORK\n");

	switch(wm9093_amp_dev->wm9093_mode) {
	case RECEIVER_CALL_MODE:
		wm9093_write_table((wm9093_reg_type*)&wm9093_in3_to_out_tab[0]);
		break;

	case SPEAKER_CALL_MODE:
		wm9093_write_table((wm9093_reg_type*)&wm9093_in2_to_out_call_tab[0]);
		break;

	case HEADSET_CALL_MODE:
		wm9093_write_table((wm9093_reg_type*)&wm9093_in2_to_hp_tab[0]);
		break;
	default:
		break;
	}
}
#endif

int wm9093_get_curmode(void)
{
	DBG("\n");

	return (int)wm9093_amp_dev->wm9093_mode;
}

int boot_cnt =0;
void wm9093_configure_path(wm9093_mode_enum mode)
{
	printk("[wm9093.c] func:[%s()], line:(%d), mode( %d)\n", __func__,__LINE__,mode);

	if (wm9093_amp_dev->wm9093_mode == mode)
		return;

#if 0
//20100705 junyeop.kim@lge.com, amp control on/off[START_LGE]
	if(wm9093_call_status == 1)
	{
		printk("[LUCKYJUN77] wm9093_control_status : %d\n", wm9093_call_status);
		return;
	}
//20100705 junyeop.kim@lge.com, amp control on/off[END_LGE]
#endif

#if 0
	if(mode == OFF_MODE){
		if(wm9093_amp_dev->wm9093_pstatus > 0 )
            wm9093_amp_dev->wm9093_pstatus = wm9093_amp_dev->wm9093_pstatus - 1;
	}
	else{
        wm9093_amp_dev->wm9093_pstatus = wm9093_amp_dev->wm9093_pstatus + 1;
	}
#endif

// 20100812 junyeop.kim@lge.com, reduce the pop noise when changing devices[START_LGE]
	   	if(wm9093_amp_dev->wm9093_mode != OFF_MODE && mode != OFF_MODE)
            wm9093_write_table((wm9093_reg_type*)&wm9093_pwroff_tab[0]);
// 20100812 junyeop.kim@lge.com, reduce the pop noise when changing devices[END_LGE]

    switch(mode){
#if 0
        case OFF_MODE : if(wm9093_amp_dev->wm9093_pstatus == 0){
			                wm9093_write_table((wm9093_reg_type*)&wm9093_pwroff_tab[0]);
						wm9093_amp_dev->wm9093_mode = OFF_MODE;
        	            }
						break;
#else
	case OFF_MODE:
		wm9093_write_table((wm9093_reg_type*)&wm9093_pwroff_tab[0]);
		wm9093_amp_dev->wm9093_mode = OFF_MODE;
		wm_delay_msec(20);		// 20100601 junyeop.kim@lge.com, remove the pop noise in the shut down[START_LGE]
		break;
#endif
	case HEADSET_AUDIO_MODE:
		wm9093_write_table((wm9093_reg_type*)&wm9093_in1_to_hp_tab[0]);
		wm9093_amp_dev->wm9093_mode = HEADSET_AUDIO_MODE;
		break;
		//--[[ LGE_UBIQUIX_MODIFIED_START : bsnoh@ubiquix.com - add from justin_froyo
	    case SPEAKER_AUDIO_MODE :
							if(boot_cnt < 10)
							{
								boot_cnt++;
//								printk(KERN_INFO "@@WM9093@@ BOOT SOUND HEADSET CNT value : %d\r\n", boot_cnt);
							}
							if((get_headset_type() != 0) && (boot_cnt < 3))
							{
								wm9093_write_table((wm9093_reg_type*)&wm9093_test_tab[0]);
//								printk(KERN_INFO "@@WM9093@@ BOOT SOUND HEADSET\r\n");
							}
							else
							{
								wm9093_write_table((wm9093_reg_type*)&wm9093_in2_to_out_tab[0]);
							}
	                        wm9093_amp_dev->wm9093_mode = SPEAKER_AUDIO_MODE;
							break;
		//--]] LGE_UBIQUIX_MODIFIED_END : bsnoh@ubiquix.com - add from justin_froyo
	    case SPEAKER_HEADSET_DUAL_AUDIO_MODE : wm9093_write_table((wm9093_reg_type*)&wm9093_in2_to_out_hp_tab[0]);
						 wm9093_amp_dev->wm9093_mode = SPEAKER_HEADSET_DUAL_AUDIO_MODE;
			             break;
#if defined(CONFIG_PRODUCT_LGE_LU6800) || defined(CONFIG_PRODUCT_LGE_KU5900) || defined (CONFIG_PRODUCT_LGE_P970)
	    case RECEIVER_CALL_MODE : wm9093_write_table((wm9093_reg_type*)&wm9093_in3_to_out_tab[0]);
						   wm9093_amp_dev->wm9093_mode = RECEIVER_CALL_MODE;
						   break;
		case SPEAKER_CALL_MODE : wm9093_write_table((wm9093_reg_type*)&wm9093_in2_to_out_call_tab[0]);
						   wm9093_amp_dev->wm9093_mode = SPEAKER_CALL_MODE;
						   break;
		case HEADSET_CALL_MODE : wm9093_write_table((wm9093_reg_type*)&wm9093_in2_to_hp_tab[0]);
						   wm9093_amp_dev->wm9093_mode = HEADSET_CALL_MODE;
						   break;
#else				/* CONFIG_PRODUCT_LGE_HUB */
	    case RECEIVER_CALL_MODE :
						   if(pre_voice_curmode == 0 && voice_get_curmode() != 0x00 && wm9093_call_status != 0)	//first call setting
						   {
  		    				   printk("[LUCKYJUN77] RECEIVER_CALL_MODE first call setting\n");
							   wm9093_amp_dev->wm9093_mode = RECEIVER_CALL_MODE;
					 		   schedule_delayed_work(&wm9093_i2c_dev->delayed_work,	msecs_to_jiffies(2000));
						   }
						   else
						   {
  		    				   printk("[LUCKYJUN77] RECEIVER_CALL_MODE not initial\n");
		    				   wm9093_write_table((wm9093_reg_type*)&wm9093_in3_to_out_tab[0]);
							   wm9093_amp_dev->wm9093_mode = RECEIVER_CALL_MODE;
						   }
						   break;
		case SPEAKER_CALL_MODE :
						   if(pre_voice_curmode == 0 && voice_get_curmode() != 0x00 && wm9093_call_status != 0)	//first call setting
						   {
  		    				   printk("[LUCKYJUN77] SPEAKER_CALL_MODE first call setting\n");
							   wm9093_amp_dev->wm9093_mode = SPEAKER_CALL_MODE;
					 		   schedule_delayed_work(&wm9093_i2c_dev->delayed_work,	msecs_to_jiffies(2000));
						   }
						   else
						   {
  		    				   printk("[LUCKYJUN77] SPEAKER_CALL_MODE not initial\n");
							   wm9093_write_table((wm9093_reg_type*)&wm9093_in2_to_out_call_tab[0]);
							   wm9093_amp_dev->wm9093_mode = SPEAKER_CALL_MODE;
						   }
						   break;
		case HEADSET_CALL_MODE :
						   if(pre_voice_curmode == 0 && voice_get_curmode() != 0x00 && wm9093_call_status != 0)	//first call setting
						   {
  		    				   printk("[LUCKYJUN77] HEADSET_CALL_MODE first call setting\n");
							   wm9093_amp_dev->wm9093_mode = HEADSET_CALL_MODE;
					 		   schedule_delayed_work(&wm9093_i2c_dev->delayed_work,	msecs_to_jiffies(2000));
						   }
						   else
						   {
  		    				   printk("[LUCKYJUN77] HEADSET_CALL_MODE not initial\n");
							   wm9093_write_table((wm9093_reg_type*)&wm9093_in2_to_hp_tab[0]);
							   wm9093_amp_dev->wm9093_mode = HEADSET_CALL_MODE;
						   }
						   break;
#endif
	case RECEIVER_VOIP_MODE:
		wm9093_write_table((wm9093_reg_type*)&wm9093_in3_to_out_voip_tab[0]);
		wm9093_amp_dev->wm9093_mode = RECEIVER_VOIP_MODE;
		break;

	case SPEAKER_VOIP_MODE:
		wm9093_write_table((wm9093_reg_type*)&wm9093_in2_to_out_voip_tab[0]);
		wm9093_amp_dev->wm9093_mode = SPEAKER_VOIP_MODE;
		break;

	case HEADSET_VOIP_MODE:
		wm9093_write_table((wm9093_reg_type*)&wm9093_in1_to_hp_voip_tab[0]);
		wm9093_amp_dev->wm9093_mode = HEADSET_VOIP_MODE;
		break;
#if defined(CONFIG_PRODUCT_LGE_LU6800) || defined(CONFIG_PRODUCT_LGE_KU5900) || defined (CONFIG_PRODUCT_LGE_P970)
		//20101205 inbang.park@lge.com Add STREAM  for  FM Radio [START]
	case SPEAKER_FMR_MODE:
		wm9093_write_table((wm9093_reg_type*)&wm9093_in1_to_FMout_tab[0]);
		wm_delay_msec(50);
		wm9093_fmradio_volume(s_volume);
		wm9093_amp_dev->wm9093_mode = SPEAKER_FMR_MODE;
		break;

	case HEADSET_FMR_MODE:
		wm9093_write_table((wm9093_reg_type*)&wm9093_in1_to_FMhp_tab[0]);
		wm_delay_msec(50);
		wm9093_fmradio_volume(s_volume);
		wm9093_amp_dev->wm9093_mode = HEADSET_FMR_MODE;
		break;
	      //20101205 inbang.park@lge.com Add STREAM  for  FM Radio [END]
#endif // #if defined(CONFIG_PRODUCT_LGE_LU6800) || defined(CONFIG_PRODUCT_LGE_KU5900) || defined (CONFIG_PRODUCT_LGE_P970)
	default:
		break;
	}
	pre_voice_curmode = voice_get_curmode();
}

#if defined(CONFIG_PRODUCT_LGE_LU6800) || defined(CONFIG_PRODUCT_LGE_KU5900) || defined (CONFIG_PRODUCT_LGE_P970)
//20101205 inbang.park@lge.com Add STREAM  for  FM Radio [START]
#define SET_FM_VOL(headphone, speaker) do { \
			wm9093_write_reg(wm9093_i2c_dev->client, 0x1C, headphone); \
			wm9093_write_reg(wm9093_i2c_dev->client, 0x1D, headphone); \
			wm9093_write_reg(wm9093_i2c_dev->client, 0x1C, headphone); \
			wm9093_write_reg(wm9093_i2c_dev->client, 0x26, speaker); \
			} while (0)

void wm9093_fmradio_volume(wm9093_fmvolume_enum volume)
{
	s_volume = volume;
	s_modes = volume;//20101225 inbang.park@lge.com Wake lock for FM radio

	DBG("\n");
	if (wm9093_i2c_dev != NULL) {
		switch (volume) {
		case LEVEL_15: SET_FM_VOL(0x12C, 0x134); break;
		case LEVEL_14: SET_FM_VOL(0x129, 0x132); break;
		case LEVEL_13: SET_FM_VOL(0x126, 0x130); break;
		case LEVEL_12: SET_FM_VOL(0x123, 0x12E); break;
		case LEVEL_11: SET_FM_VOL(0x120, 0x12C); break;
		case LEVEL_10: SET_FM_VOL(0x11D, 0x12A); break;
		case LEVEL_9 : SET_FM_VOL(0x11A, 0x128); break;
		case LEVEL_8 : SET_FM_VOL(0x117, 0x126); break;
		case LEVEL_7 : SET_FM_VOL(0x114, 0x124); break;
		case LEVEL_6 : SET_FM_VOL(0x111, 0x122); break;
		case LEVEL_5 : SET_FM_VOL(0x10E, 0x120); break;
		case LEVEL_4 : SET_FM_VOL(0x10B, 0x11E); break;
		case LEVEL_3 : SET_FM_VOL(0x108, 0x11C); break;
		case LEVEL_2 : SET_FM_VOL(0x105, 0x11A); break;
		case LEVEL_1 : SET_FM_VOL(0x102, 0x118); break;
		case LEVEL_OFF:SET_FM_VOL(0x140, 0x140); break;
		case LEVEL_reset: break;
		}
 		printk(KERN_INFO "FMvolume_%d\n", volume);
	} else {
		printk(KERN_ERR "wm9093 i2c_dev is null");
	}
}
//20101205 inbang.park@lge.com Add STREAM  for  FM Radio [END]
#endif // #if defined(CONFIG_PRODUCT_LGE_LU6800) || defined(CONFIG_PRODUCT_LGE_KU5900) || defined (CONFIG_PRODUCT_LGE_P970)

int wm9093_ext_suspend()
{
	DBG("\n");

	if (wm9093_amp_dev->wm9093_mode != 0)
		wm9093_write_table((wm9093_reg_type*)&wm9093_pwroff_tab[0]);

	wm9093_amp_dev->wm9093_mode = OFF_MODE; //add - jaewoo1.park@lge.com:to configure path after resume

	return 0;
}

int wm9093_ext_resume()
{
	DBG("\n");

	switch (wm9093_amp_dev->wm9093_mode) {
	case OFF_MODE:
		#if (defined(CONFIG_PRODUCT_LGE_HUB))
			wm9093_write_table((wm9093_reg_type*)&wm9093_pwroff_tab[0]);
		#endif // #if (defined(CONFIG_PRODUCT_LGE_HUB)
		break;
	case HEADSET_AUDIO_MODE: wm9093_write_table((wm9093_reg_type*)&wm9093_in1_to_hp_tab[0]);
		break;
	case SPEAKER_AUDIO_MODE: wm9093_write_table((wm9093_reg_type*)&wm9093_in2_to_out_tab[0]);
		break;
	case SPEAKER_HEADSET_DUAL_AUDIO_MODE: wm9093_write_table((wm9093_reg_type*)&wm9093_in2_to_out_hp_tab[0]);
		break;
	case RECEIVER_CALL_MODE: wm9093_write_table((wm9093_reg_type*)&wm9093_in3_to_out_tab[0]);
		break;
	case SPEAKER_CALL_MODE: wm9093_write_table((wm9093_reg_type*)&wm9093_in2_to_out_call_tab[0]);
		break;
	case HEADSET_CALL_MODE: wm9093_write_table((wm9093_reg_type*)&wm9093_in2_to_hp_tab[0]);
		break;
	case RECEIVER_VOIP_MODE: wm9093_write_table((wm9093_reg_type*)&wm9093_in3_to_out_voip_tab[0]);
		break;
	case SPEAKER_VOIP_MODE: wm9093_write_table((wm9093_reg_type*)&wm9093_in2_to_out_voip_tab[0]);
		break;
	case HEADSET_VOIP_MODE: wm9093_write_table((wm9093_reg_type*)&wm9093_in1_to_hp_voip_tab[0]);
		break;
#if defined(CONFIG_PRODUCT_LGE_LU6800) || defined(CONFIG_PRODUCT_LGE_KU5900) || defined (CONFIG_PRODUCT_LGE_P970)
	     //20101205 inbang.park@lge.com Add STREAM  for  FM Radio [START]
	case SPEAKER_FMR_MODE: wm9093_write_table((wm9093_reg_type*)&wm9093_in1_to_FMout_tab[0]);
		wm_delay_msec(50);
		wm9093_fmradio_volume(s_volume);
		break;
	case HEADSET_FMR_MODE: wm9093_write_table((wm9093_reg_type*)&wm9093_in1_to_FMhp_tab[0]);
		wm_delay_msec(50);
		wm9093_fmradio_volume(s_volume);
		break;
	      //20101205 inbang.park@lge.com Add STREAM  for  FM Radio [END]
#endif // #if defined(CONFIG_PRODUCT_LGE_LU6800) || defined(CONFIG_PRODUCT_LGE_KU5900) || defined (CONFIG_PRODUCT_LGE_P970)
	default :
	     break;
	}
	return 0;
}

#if defined(CONFIG_PRODUCT_LGE_LU6800) || defined(CONFIG_PRODUCT_LGE_KU5900) || defined (CONFIG_PRODUCT_LGE_P970)
//20101222 inbang.park@lge.com Wake lock for  FM Radio [START]
void fmradio_configure_path(wm9093_fmvolume_enum mode)
{
	DBG("\n");
	printk("[inbangpark] :fmradio_configure_path %d\n",mode);
	switch (mode) {
	case LEVEL_reset:
		cur_fmradio_mode= 0;
		printk("[inbangpark] :fmradio_configure_path %d\n", cur_fmradio_mode);
		break;
	default:
		cur_fmradio_mode= 1;
		printk("[inbangpark] :fmradio_configure_path %d\n", cur_fmradio_mode);
		break;
	}
}
//20101222 inbang.park@lge.com Wake lock for  FM Radio [END]
//20101222 inbang.park@lge.com Wake lock for  FM Radio [START]
int fmradio_get_curmode()
{
	DBG("\n");
	return cur_fmradio_mode;
}
//20101222 inbang.park@lge.com Wake lock for  FM Radio [END]
#endif // #if defined(CONFIG_PRODUCT_LGE_LU6800) || defined(CONFIG_PRODUCT_LGE_KU5900) || defined (CONFIG_PRODUCT_LGE_P970)
EXPORT_SYMBOL(wm9093_get_curmode);
EXPORT_SYMBOL(wm9093_configure_path);
EXPORT_SYMBOL(wm9093_ext_suspend);
EXPORT_SYMBOL(wm9093_ext_resume);
#if defined(CONFIG_PRODUCT_LGE_LU6800) || defined(CONFIG_PRODUCT_LGE_KU5900) || defined (CONFIG_PRODUCT_LGE_P970)
//20101205 inbang.park@lge.com Add STREAM  for  FM Radio [START]
EXPORT_SYMBOL(wm9093_fmradio_volume);
//20101205 inbang.park@lge.com Add STREAM  for  FM Radio [END]
//20101222 inbang.park@lge.com Wake lock for  FM Radio [START]
EXPORT_SYMBOL(fmradio_configure_path);
EXPORT_SYMBOL(fmradio_get_curmode);
//20101222 inbang.park@lge.com Wake lock for  FM Radio [END]
#endif // #if defined(CONFIG_PRODUCT_LGE_LU6800) || defined(CONFIG_PRODUCT_LGE_KU5900) || defined (CONFIG_PRODUCT_LGE_P970)

ssize_t wm9093_show_level(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct wm9093_i2c_device *wm9093_dev;
	int r = 0;

	DBG("\n");

	wm9093_dev = dev_get_drvdata(dev);

	r += sprintf(buf + r, "wm9093 mode is : %d\n", wm9093_dev->wm9093_dev->wm9093_mode);

	return r;

}

ssize_t wm9093_store_level(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	int level;

	DBG("\n");

	level = simple_strtoul(buf, NULL, 10);

	switch (level) {
	//turn off
	case 0: wm9093_configure_path(OFF_MODE);
		break;
	//set spk mode
	case 1: wm9093_configure_path(SPEAKER_AUDIO_MODE);
		break;
	//set headset mode
	case 2: wm9093_configure_path(HEADSET_AUDIO_MODE);
		break;
	//set bypass mode
	case 3: wm9093_configure_path(RECEIVER_CALL_MODE);
		break;
	//set dual mode
	case 4: wm9093_configure_path(SPEAKER_HEADSET_DUAL_AUDIO_MODE);
		break;
	case 5: wm9093_configure_path(SPEAKER_CALL_MODE);
		break;
	case 6: wm9093_configure_path(HEADSET_CALL_MODE);
		break;
	default:
		break;
	}
	return count;
}

DEVICE_ATTR(wm9093_path, 0664, wm9093_show_level, wm9093_store_level);

static ssize_t wm9093_reg_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int r =0;
	int r_data;

	DBG("\n");

	r_data = wm9093_read_reg(wm9093_i2c_dev->client, 0);
	r += sprintf(buf+r, "wm9093 reg 0 : 0x%4x\n",r_data);

	r_data = wm9093_read_reg(wm9093_i2c_dev->client, 1);
	r += sprintf(buf+r, "wm9093 reg 1 : 0x%4x\n",r_data);

	r_data = wm9093_read_reg(wm9093_i2c_dev->client, 2);
	r += sprintf(buf+r, "wm9093 reg 2 : 0x%4x\n",r_data);

	r_data = wm9093_read_reg(wm9093_i2c_dev->client, 3);
	r += sprintf(buf+r, "wm9093 reg 3 : 0x%4x\n",r_data);

	r_data = wm9093_read_reg(wm9093_i2c_dev->client, 6);
	r += sprintf(buf+r, "wm9093 reg 6 : 0x%4x\n",r_data);

	r_data = wm9093_read_reg(wm9093_i2c_dev->client, 21);
	r += sprintf(buf+r, "wm9093 reg 15 : 0x%4x\n",r_data);

	r_data = wm9093_read_reg(wm9093_i2c_dev->client, 22);
	r += sprintf(buf+r, "wm9093 reg 16 : 0x%4x\n",r_data);

	r_data = wm9093_read_reg(wm9093_i2c_dev->client, 23);
	r += sprintf(buf+r, "wm9093 reg 17 : 0x%4x\n",r_data);

	r_data = wm9093_read_reg(wm9093_i2c_dev->client, 24);
	r += sprintf(buf+r, "wm9093 reg 18 : 0x%4x\n",r_data);

	r_data = wm9093_read_reg(wm9093_i2c_dev->client, 25);
	r += sprintf(buf+r, "wm9093 reg 19 : 0x%4x\n",r_data);

	r_data = wm9093_read_reg(wm9093_i2c_dev->client, 26);
	r += sprintf(buf+r, "wm9093 reg 1a : 0x%4x\n",r_data);

	r_data = wm9093_read_reg(wm9093_i2c_dev->client, 27);
	r += sprintf(buf+r, "wm9093 reg 1b : 0x%4x\n",r_data);

	r_data = wm9093_read_reg(wm9093_i2c_dev->client, 28);
	r += sprintf(buf+r, "wm9093 reg 1c : 0x%4x\n",r_data);

	r_data = wm9093_read_reg(wm9093_i2c_dev->client, 29);
	r += sprintf(buf+r, "wm9093 reg 1d : 0x%4x\n",r_data);

	r_data = wm9093_read_reg(wm9093_i2c_dev->client, 34);
	r += sprintf(buf+r, "wm9093 reg 22 : 0x%4x\n",r_data);

	r_data = wm9093_read_reg(wm9093_i2c_dev->client, 35);
	r += sprintf(buf+r, "wm9093 reg 23 : 0x%4x\n",r_data);

	r_data = wm9093_read_reg(wm9093_i2c_dev->client, 36);
	r += sprintf(buf+r, "wm9093 reg 24 : 0x%4x\n",r_data);

	r_data = wm9093_read_reg(wm9093_i2c_dev->client, 37);
	r += sprintf(buf+r, "wm9093 reg 25 : 0x%4x\n",r_data);

	r_data = wm9093_read_reg(wm9093_i2c_dev->client, 38);
	r += sprintf(buf+r, "wm9093 reg 26 : 0x%4x\n",r_data);

	r_data = wm9093_read_reg(wm9093_i2c_dev->client, 40);
	r += sprintf(buf+r, "wm9093 reg 28 : 0x%4x\n",r_data);

	r_data = wm9093_read_reg(wm9093_i2c_dev->client, 45);
	r += sprintf(buf+r, "wm9093 reg 2d : 0x%4x\n",r_data);

	r_data = wm9093_read_reg(wm9093_i2c_dev->client, 46);
	r += sprintf(buf+r, "wm9093 reg 2e : 0x%4x\n",r_data);

	r_data = wm9093_read_reg(wm9093_i2c_dev->client, 47);
	r += sprintf(buf+r, "wm9093 reg 2f : 0x%4x\n",r_data);

	r_data = wm9093_read_reg(wm9093_i2c_dev->client, 48);
	r += sprintf(buf+r, "wm9093 reg 30 : 0x%4x\n",r_data);

	r_data = wm9093_read_reg(wm9093_i2c_dev->client, 54);
	r += sprintf(buf+r, "wm9093 reg 36 : 0x%4x\n",r_data);

#if 1
	r_data = wm9093_read_reg(wm9093_i2c_dev->client, 57);
	r += sprintf(buf+r, "wm9093 reg 39 : 0x%4x\n",r_data);

	r_data = wm9093_read_reg(wm9093_i2c_dev->client, 76);
	r += sprintf(buf+r, "wm9093 reg 4c : 0x%4x\n",r_data);

	r_data = wm9093_read_reg(wm9093_i2c_dev->client, 84);
	r += sprintf(buf+r, "wm9093 reg 54 : 0x%4x\n",r_data);

	r_data = wm9093_read_reg(wm9093_i2c_dev->client, 96);
	r += sprintf(buf+r, "wm9093 reg 60 : 0x%4x\n",r_data);

	r_data = wm9093_read_reg(wm9093_i2c_dev->client, 98);
	r += sprintf(buf+r, "wm9093 reg 62 : 0x%4x\n",r_data);

	r_data = wm9093_read_reg(wm9093_i2c_dev->client, 99);
	r += sprintf(buf+r, "wm9093 reg 63 : 0x%4x\n",r_data);

#endif

	return r;
}

ssize_t wm9093_reg_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	int reg, data;
	char *r, *d;

	DBG("\n");
	printk("[LUCKYJUN77] wm9093_reg_store : %s\n", buf);

	r= &buf[0];
	printk("[LUCKYJUN77] r : %s\n", r);

	d= &buf[5];
	printk("[LUCKYJUN77] d : %s\n", d);

	reg = simple_strtoul(r, NULL, 16);
	data = simple_strtoul(d, NULL, 16);

	printk("[LUCKYJUN77] reg: %d, data : %d\n", reg, data);

	wm9093_write_reg(wm9093_i2c_dev->client, (u8)reg, data);

	return count;
}

#if 1	// 20100429 junyeop.kim@lge.com [START_LGE]
static DEVICE_ATTR(wm9093_data, 0644, wm9093_reg_show, wm9093_reg_store);
#else
static DEVICE_ATTR(wm9093_data, 0666, wm9093_reg_show, wm9093_reg_store);
#endif	// 20100426 junyeop.kim@lge.com [END_LGE]

//20101209 junyeop.kim@lge.com, reduce the outgoing call noise (0 : incoming call, 1 :other case) [START_LGE]
ssize_t wm9093_show_status(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct wm9093_i2c_device *wm9093_dev;
	int r = 0;
	DBG("\n");

	wm9093_dev = dev_get_drvdata(dev);

	r += sprintf(buf + r, "wm9093 status is : %d\n", wm9093_call_status);

	return r;
}

ssize_t wm9093_store_status(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	int status;

	status = simple_strtoul(buf, NULL, 10);

	DBG("\n");
	printk("[LUCKYJUN77] status: %d\n", status);

	if (status == 0 || status == 1) {
		wm9093_call_status = status;
	}

	return count;
}

DEVICE_ATTR(wm9093_status, 0664, wm9093_show_status, wm9093_store_status);
//20101209 junyeop.kim@lge.com, reduce the outgoing call noise (0 : incoming call, 1 :other case) [END_LGE]

static void wm9093_init(struct i2c_client *client)
{
	DBG("\n");

	wm9093_write_table((wm9093_reg_type*)&wm9093_pwroff_tab[0]);
	wm9093_amp_dev->wm9093_mode = OFF_MODE;
	wm9093_amp_dev->wm9093_pstatus = 0;
	return;
}

// [kibum.lee@lge.com] Section mismatch WARNING fix
//static int __init wm9093_probe(struct i2c_client *client, const struct i2c_device_id *id)
static int wm9093_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int err;
	DBG("\n");
	if (i2c_get_clientdata(client))
		return -EBUSY;

	wm9093_i2c_dev = kzalloc(sizeof(struct wm9093_i2c_device), GFP_KERNEL);
	wm9093_amp_dev = kzalloc(sizeof(struct wm9093_device), GFP_KERNEL);

	wm9093_i2c_dev->client = client;
	wm9093_i2c_dev->wm9093_dev = wm9093_amp_dev;

//junyeop.kim@lge.com, codec power enable for vpll2 [START_LGE]
	struct device *wm9093_dev = &client->dev;
	wm9093_reg = regulator_get(wm9093_dev,"vpll2");
	if (wm9093_reg == NULL) {
		printk("wm9093 : Failed to get amp power regulator !! \n");
	}

	regulator_enable(wm9093_reg);
//junyeop.kim@lge.com, codec power enable for vpll2 [END_LGE]

	i2c_set_clientdata(client, wm9093_i2c_dev);

	wm9093_init(client);

	err = device_create_file(&client->dev, &dev_attr_wm9093_path);
	err = device_create_file(&client->dev, &dev_attr_wm9093_data);
	err = device_create_file(&client->dev, &dev_attr_wm9093_status);	//20100705 junyeop.kim@lge.com, amp control on/off[START_LGE]

#if 1	//junyeop.kim@lge.com, call initial noise workaround
	INIT_DELAYED_WORK(&wm9093_i2c_dev->delayed_work, callmode_set_work);
#endif
	DBG("\n");

	return 0;
}

static int wm9093_remove(struct i2c_client *client)
{
	DBG("\n");

	i2c_set_clientdata(client, NULL);
	device_remove_file(&client->dev, &dev_attr_wm9093_path);
	device_remove_file(&client->dev, &dev_attr_wm9093_data);
	device_remove_file(&client->dev, &dev_attr_wm9093_status);		//20100705 junyeop.kim@lge.com, amp control on/off[START_LGE]

	kfree(wm9093_amp_dev);
	kfree(wm9093_i2c_dev);
	return 0;
}

//junyeop.kim@lge.com, codec power enable for vpll2 [START_LGE]
static int wm9093_suspend(struct i2c_client *client, pm_message_t mesg)
{
	DBG("\n");

//For_Resume_Speed	printk("[LUCKYJUN77]wm9093_suspend\n");
	if (wm9093_amp_dev->wm9093_mode == OFF_MODE) {	//junyeop.kim@lge.com
//		printk("[IBPARK]WM9093 Sleep State of FMradio\n");
		regulator_disable(wm9093_reg);
	} else {
		twl_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER, 0xee, VPLL2_DEV_GRP );   //junyeop.kim@lge.com, 20100826 change vpll2 power grp [START_LGE]
		twl_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER, 0x0e, 0x35 );
//		printk("[IBPARK]WM9093 Wake State of FMradio\n");
	}
	return 0;
}

static int wm9093_resume(struct i2c_client *client)
{
	DBG("\n");

//For_Resume_Speed	printk("[LUCKYJUN77]wm9093_resume\n");
	if(wm9093_amp_dev->wm9093_mode == OFF_MODE)	//junyeop.kim@lge.com, test
		regulator_enable(wm9093_reg);

	return 0;
}
//junyeop.kim@lge.com, codec power enable for vpll2 [END_LGE]

//junyeop.kim@lge.com, 20100826 get codec status [START_LGE]
unsigned int get_wm9093_mode(void)
{
	DBG("\n");
	return (unsigned int)wm9093_amp_dev->wm9093_mode;
}
EXPORT_SYMBOL_GPL(get_wm9093_mode);
//junyeop.kim@lge.com, 20100826 get codec status [END_LGE]

static const struct i2c_device_id wm9093_ids[] = {
	{ WM9093_I2C_NAME, 0 },	/*wm9093*/
	{ /* end of list */ },
};

static struct i2c_driver wm9093_i2c_driver = {
	.probe = wm9093_probe,
	.remove = wm9093_remove,
//junyeop.kim@lge.com, codec power enable for vpll2 [START_LGE]
	.suspend = wm9093_suspend,
	.resume = wm9093_resume,
//junyeop.kim@lge.com, codec power enable for vpll2 [END_LGE]
	.id_table = wm9093_ids,
	.driver = {
		.name = WM9093_I2C_NAME,
		.owner = THIS_MODULE,
	},
};

static int __init ext_amp_wm9093_init(void)
{
	int ret;
	DBG("\n");
	ret = i2c_add_driver(&wm9093_i2c_driver);
	if (ret != 0)
		printk(KERN_ERR "can't add i2c driver");
	return ret;
}

static void __exit ext_amp_wm9093_exit(void)
{
	DBG("\n");

	i2c_del_driver(&wm9093_i2c_driver);
}

module_init(ext_amp_wm9093_init);
module_exit(ext_amp_wm9093_exit);

MODULE_AUTHOR("LG Electronics");
MODULE_DESCRIPTION("wm9093 audio ext amp Driver");
MODULE_LICENSE("GPL");

