/*
 * ALSA SoC TWL4030 codec driver
 *
 * Author:      Steve Sakoman, <steve@sakoman.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 */

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/pm.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/i2c/twl.h>
#include <linux/slab.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>
#include <sound/initval.h>
#include <sound/tlv.h>
#if defined(CONFIG_PRODUCT_LGE_LU6800) || defined(CONFIG_PRODUCT_LGE_KU5900) || defined (CONFIG_PRODUCT_LGE_P970) // 20100618 junyeop.kim@lge.com, audio path [START_LGE]
#include <mach/hub_headset_det.h>
#endif /* LGE_CHANGE_E [iggikim@lge.com]*/
#if defined(CONFIG_HUB_AMP_WM9093)
#include <mach/wm9093.h>
#endif

/* iwlee_temp */
#include <asm/gpio.h>

/* Register descriptions are here */
#include <linux/mfd/twl4030-codec.h>

/* Shadow register used by the audio driver */
#define TWL4030_REG_SW_SHADOW		0x4A
#define TWL4030_CACHEREGNUM	(TWL4030_REG_SW_SHADOW + 1)

/* TWL4030_REG_SW_SHADOW (0x4A) Fields */
#define TWL4030_HFL_EN			0x01
#define TWL4030_HFR_EN			0x02
#define MODULE_NAME		"twl4030"

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

#if defined(CONFIG_PRODUCT_LGE_LU6800) // 20101214 seungdae.goh@lge.com justin gpio adjust
#define MIC_SEL_GPIO  63
#elif defined(CONFIG_PRODUCT_LGE_KU5900) || defined (CONFIG_PRODUCT_LGE_P970)
#define MIC_SEL_GPIO  87
#elif defined(CONFIG_PRODUCT_LGE_HUB)
// Hub not used the Mic2
#endif
#if defined(CONFIG_PRODUCT_LGE_HUB)
unsigned int dmb_status = 0;	//junyeop.kim@lge.com
#endif //#if defined(CONFIG_PRODUCT_LGE_HUB)

unsigned int twl4030_status = 1;	//0 : sleep, 1 : resume
unsigned int call_headset_ramp = 0;

#if defined(CONFIG_PRODUCT_LGE_LU6800) || defined(CONFIG_PRODUCT_LGE_KU5900) || defined (CONFIG_PRODUCT_LGE_P970)
//20101225 inbang.park@lge.com Wake lock for FM Radio[START]
extern unsigned int cur_fmradio_mode;
unsigned int fmradio_suspend_user = 0;
unsigned fmradio_is_on = 0;
//20101225 inbang.park@lge.com Wake lock for FM Radio[END]
//unsigned int dmb_status = 0;	//junyeop.kim@lge.com
#endif // #if defined(CONFIG_PRODUCT_LGE_LU6800) || defined(CONFIG_PRODUCT_LGE_KU5900) || defined (CONFIG_PRODUCT_LGE_P970)

/*
 * twl4030 register cache & default register settings
 */
#if defined(CONFIG_PRODUCT_LGE_LU6800)  // 20101231 seungdae.goh@lge.com HW_TUNNING


static const u8 twl4030_reg[TWL4030_CACHEREGNUM] = {
	0x00, /* this register not used		*/
	0x91, /* REG_CODEC_MODE		(0x1)	*/
	0xc3, /* REG_OPTION		(0x2)	*/
	0x00, /* REG_UNKNOWN		(0x3)	*/
	0x00, /* REG_MICBIAS_CTL	(0x4)	*/
	0x20, /* REG_ANAMICL		(0x5)	*/
	0x00, /* REG_ANAMICR		(0x6)	*/
	0x08, /* REG_AVADC_CTL		(0x7)	*/
	0x00, /* REG_ADCMICSEL		(0x8)	*/
	0x00, /* REG_DIGMIXING		(0x9)	*/
	0x0c, /* REG_ATXL1PGA		(0xA)	*/
	0x0c, /* REG_ATXR1PGA		(0xB)	*/
	0x00, /* REG_AVTXL2PGA		(0xC)	*/
	0x00, /* REG_AVTXR2PGA		(0xD)	*/
	0x01, /* REG_AUDIO_IF		(0xE)	*/
	0x04, /* REG_VOICE_IF		(0xF)	*/
	0x00, /* REG_ARXR1PGA		(0x10)	*/
	0x00, /* REG_ARXL1PGA		(0x11)	*/
	0x3c, /* REG_ARXR2PGA		(0x12)	*/
	0x3c, /* REG_ARXL2PGA		(0x13)	*/
	0x00, /* REG_VRXPGA		(0x14)	*/
	0x00, /* REG_VSTPGA		(0x15)	*/
	0x00, /* REG_VRX2ARXPGA		(0x16)	*/
	0x0c, /* REG_AVDAC_CTL		(0x17)	*/
	0x00, /* REG_ARX2VTXPGA		(0x18)	*/
	0x00, /* REG_ARXL1_APGA_CTL	(0x19)	*/
	0x00, /* REG_ARXR1_APGA_CTL	(0x1A)	*/
	0x23, /* REG_ARXL2_APGA_CTL	(0x1B)	*/
	0x23, /* REG_ARXR2_APGA_CTL	(0x1C)	*/
	0x00, /* REG_ATX2ARXPGA		(0x1D)	*/
	0x00, /* REG_BT_IF		(0x1E)	*/
	0x00, /* REG_BTPGA		(0x1F)	*/
	0x00, /* REG_BTSTPGA		(0x20)	*/
	0x00, /* REG_EAR_CTL		(0x21)	*/
	0x24, /* REG_HS_SEL		(0x22)	*/
	0x0a, /* REG_HS_GAIN_SET	(0x23)	*/
	0x00, /* REG_HS_POPN_SET	(0x24)	*/
	0x24, /* REG_PREDL_CTL		(0x25)	*/
	0x24, /* REG_PREDR_CTL		(0x26)	*/
	0x00, /* REG_PRECKL_CTL		(0x27)	*/
	0x00, /* REG_PRECKR_CTL		(0x28)	*/
	0x00, /* REG_HFL_CTL		(0x29)	*/
	0x00, /* REG_HFR_CTL		(0x2A)	*/
	0x00, /* REG_ALC_CTL		(0x2B)	*/
	0x00, /* REG_ALC_SET1		(0x2C)	*/
	0x00, /* REG_ALC_SET2		(0x2D)	*/
	0x00, /* REG_BOOST_CTL		(0x2E)	*/
	0x00, /* REG_SOFTVOL_CTL	(0x2F)	*/
	0x00, /* REG_DTMF_FREQSEL	(0x30)	*/
	0x00, /* REG_DTMF_TONEXT1H	(0x31)	*/
	0x00, /* REG_DTMF_TONEXT1L	(0x32)	*/
	0x00, /* REG_DTMF_TONEXT2H	(0x33)	*/
	0x00, /* REG_DTMF_TONEXT2L	(0x34)	*/
	0x00, /* REG_DTMF_TONOFF	(0x35)	*/
	0x00, /* REG_DTMF_WANONOFF	(0x36)	*/
	0x00, /* REG_I2S_RX_SCRAMBLE_H	(0x37)	*/
	0x00, /* REG_I2S_RX_SCRAMBLE_M	(0x38)	*/
	0x00, /* REG_I2S_RX_SCRAMBLE_L	(0x39)	*/
	0x16, /* REG_APLL_CTL		(0x3A)	*/
	0x00, /* REG_DTMF_CTL		(0x3B)	*/
	0x00, /* REG_DTMF_PGA_CTL2	(0x3C)	*/
	0x00, /* REG_DTMF_PGA_CTL1	(0x3D)	*/
	0x00, /* REG_MISC_SET_1		(0x3E)	*/
	0x00, /* REG_PCMBTMUX		(0x3F)	*/
	0x00, /* not used		(0x40)	*/
	0x00, /* not used		(0x41)	*/
	0x00, /* not used		(0x42)	*/
	0x00, /* REG_RX_PATH_SEL	(0x43)	*/
	0x00, /* REG_VDL_APGA_CTL	(0x44)	*/
	0x00, /* REG_VIBRA_CTL		(0x45)	*/
	0x00, /* REG_VIBRA_SET		(0x46)	*/
	0x00, /* REG_VIBRA_PWM_SET	(0x47)	*/
	0x00, /* REG_ANAMIC_GAIN	(0x48)	*/
	0x00, /* REG_MISC_SET_2		(0x49)	*/
	0x00, /* REG_SW_SHADOW		(0x4A)	- Shadow, non HW register */
};
#elif defined(CONFIG_PRODUCT_LGE_KU5900) || defined (CONFIG_PRODUCT_LGE_P970)
static const u8 twl4030_reg[TWL4030_CACHEREGNUM] = {
	0x00, /* this register not used		*/
    // wooho.jeong@lge.com 2012.10.05
    // MOD : for 44.1K audio sample rate
	0x91, /* REG_CODEC_MODE		(0x1)	*/
	0xc3, /* REG_OPTION		(0x2)	*/
	0x00, /* REG_UNKNOWN		(0x3)	*/
	0x00, /* REG_MICBIAS_CTL	(0x4)	*/
	0x20, /* REG_ANAMICL		(0x5)	*/
	0x00, /* REG_ANAMICR		(0x6)	*/
	0x08, /* REG_AVADC_CTL		(0x7)	*/
	0x00, /* REG_ADCMICSEL		(0x8)	*/
	0x00, /* REG_DIGMIXING		(0x9)	*/
	
	//0x0c, /* REG_ATXL1PGA		(0xA)	*/
	0x00, /* REG_ATXL1PGA		(0xA)	*/ // 20121030 bs.lim@lge.com
	//0x0c, /* REG_ATXR1PGA		(0xB)	*/
	0x00, /* REG_ATXR1PGA		(0xB)	*/ //20121030 bs.lim@lge.com
	
	0x00, /* REG_AVTXL2PGA		(0xC)	*/	// 20100920 junyeop.kim@lge.com, call tuning for call(HW require) [START_LGE]
	0x00, /* REG_AVTXR2PGA		(0xD)	*/	// 20100820 junyeop.kim@lge.com, call tuning for call(HW require) [START_LGE]
	0x01, /* REG_AUDIO_IF		(0xE)	*/
	0x04, /* REG_VOICE_IF		(0xF)	*/
	0x00, /* REG_ARXR1PGA		(0x10)	*/
	0x00, /* REG_ARXL1PGA		(0x11)	*/
	0x3f, /* REG_ARXR2PGA		(0x12)	*/	// 20100828 junyeop.kim@lge.com, vol tuning for media(HW require) [START_LGE]
	0x3f, /* REG_ARXL2PGA		(0x13)	*/	// 20100828 junyeop.kim@lge.com, vol tuning for media(HW require) [END_LGE]
	0x00, /* REG_VRXPGA		(0x14)	*/
	0x00, /* REG_VSTPGA		(0x15)	*/
	0x00, /* REG_VRX2ARXPGA		(0x16)	*/
	0x0c, /* REG_AVDAC_CTL		(0x17)	*/
	0x00, /* REG_ARX2VTXPGA		(0x18)	*/
	0x00, /* REG_ARXL1_APGA_CTL	(0x19)	*/
	0x00, /* REG_ARXR1_APGA_CTL	(0x1A)	*/
	0x2b, /* REG_ARXL2_APGA_CTL	(0x1B)	*/	// 20100828 junyeop.kim@lge.com, vol tuning for media(HW require) [START_LGE]
	0x2b, /* REG_ARXR2_APGA_CTL	(0x1C)	*/	// 20100828 junyeop.kim@lge.com, vol tuning for media(HW require) [END_LGE]
	0x00, /* REG_ATX2ARXPGA		(0x1D)	*/
	0x00, /* REG_BT_IF		(0x1E)	*/
	0x00, /* REG_BTPGA		(0x1F)	*/
	0x00, /* REG_BTSTPGA		(0x20)	*/
	0x00, /* REG_EAR_CTL		(0x21)	*/ /* LGE_CHANGE [iggikim@lge.com] 2009-07-7, audio path */
	0x24, /* REG_HS_SEL		(0x22)	*/
	0x0a, /* REG_HS_GAIN_SET	(0x23)	*/
	0x00, /* REG_HS_POPN_SET	(0x24)	*/
	0x20, /* REG_PREDL_CTL		(0x25)	*/ // 20100520 junyeop.kim@lge.com, headset tuning for media(HW require) [START_LGE]
	0x20, /* REG_PREDR_CTL		(0x26)	*/ // 20100520 junyeop.kim@lge.com, headset tuning for media(HW require) [START_LGE]
	0x00, /* REG_PRECKL_CTL		(0x27)	*/
	0x00, /* REG_PRECKR_CTL		(0x28)	*/
	0x00, /* REG_HFL_CTL		(0x29)	*/
	0x00, /* REG_HFR_CTL		(0x2A)	*/
	0x00, /* REG_ALC_CTL		(0x2B)	*/
	0x00, /* REG_ALC_SET1		(0x2C)	*/
	0x00, /* REG_ALC_SET2		(0x2D)	*/
	0x00, /* REG_BOOST_CTL		(0x2E)	*/
	0x00, /* REG_SOFTVOL_CTL	(0x2F)	*/
	0x00, /* REG_DTMF_FREQSEL	(0x30)	*/
	0x00, /* REG_DTMF_TONEXT1H	(0x31)	*/
	0x00, /* REG_DTMF_TONEXT1L	(0x32)	*/
	0x00, /* REG_DTMF_TONEXT2H	(0x33)	*/
	0x00, /* REG_DTMF_TONEXT2L	(0x34)	*/
	0x00, /* REG_DTMF_TONOFF	(0x35)	*/
	0x00, /* REG_DTMF_WANONOFF	(0x36)	*/
	0x00, /* REG_I2S_RX_SCRAMBLE_H	(0x37)	*/
	0x00, /* REG_I2S_RX_SCRAMBLE_M	(0x38)	*/
	0x00, /* REG_I2S_RX_SCRAMBLE_L	(0x39)	*/
	0x16, /* REG_APLL_CTL		(0x3A)	*/
	0x00, /* REG_DTMF_CTL		(0x3B)	*/
	0x00, /* REG_DTMF_PGA_CTL2	(0x3C)	*/
	0x00, /* REG_DTMF_PGA_CTL1	(0x3D)	*/
	0x00, /* REG_MISC_SET_1		(0x3E)	*/
	0x00, /* REG_PCMBTMUX		(0x3F)	*/
	0x00, /* not used		(0x40)	*/
	0x00, /* not used		(0x41)	*/
	0x00, /* not used		(0x42)	*/
	0x00, /* REG_RX_PATH_SEL	(0x43)	*/
	0x00, /* REG_VDL_APGA_CTL	(0x44)	*/
	0x00, /* REG_VIBRA_CTL		(0x45)	*/
	0x00, /* REG_VIBRA_SET		(0x46)	*/
	0x00, /* REG_VIBRA_PWM_SET	(0x47)	*/
	0x00, /* REG_ANAMIC_GAIN	(0x48)	*/
	0x00, /* REG_MISC_SET_2		(0x49)	*/
	0x00, /* REG_SW_SHADOW		(0x4A)	- Shadow, non HW register */
};
#else
static const u8 twl4030_reg[TWL4030_CACHEREGNUM] = {
	0x00, /* this register not used		*/
	0x00, /* REG_CODEC_MODE		(0x1)	*/
	0x00, /* REG_OPTION		(0x2)	*/
	0x00, /* REG_UNKNOWN		(0x3)	*/
	0x00, /* REG_MICBIAS_CTL	(0x4)	*/
	0x00, /* REG_ANAMICL		(0x5)	*/
	0x00, /* REG_ANAMICR		(0x6)	*/
	0x00, /* REG_AVADC_CTL		(0x7)	*/
	0x00, /* REG_ADCMICSEL		(0x8)	*/
	0x00, /* REG_DIGMIXING		(0x9)	*/
	0x0f, /* REG_ATXL1PGA		(0xA)	*/
	0x0f, /* REG_ATXR1PGA		(0xB)	*/
	0x0f, /* REG_AVTXL2PGA		(0xC)	*/
	0x0f, /* REG_AVTXR2PGA		(0xD)	*/
	0x00, /* REG_AUDIO_IF		(0xE)	*/
	0x00, /* REG_VOICE_IF		(0xF)	*/
	0x3f, /* REG_ARXR1PGA		(0x10)	*/
	0x3f, /* REG_ARXL1PGA		(0x11)	*/
	0x3f, /* REG_ARXR2PGA		(0x12)	*/
	0x3f, /* REG_ARXL2PGA		(0x13)	*/
	0x25, /* REG_VRXPGA		(0x14)	*/
	0x00, /* REG_VSTPGA		(0x15)	*/
	0x00, /* REG_VRX2ARXPGA		(0x16)	*/
	0x00, /* REG_AVDAC_CTL		(0x17)	*/
	0x00, /* REG_ARX2VTXPGA		(0x18)	*/
	0x32, /* REG_ARXL1_APGA_CTL	(0x19)	*/
	0x32, /* REG_ARXR1_APGA_CTL	(0x1A)	*/
	0x32, /* REG_ARXL2_APGA_CTL	(0x1B)	*/
	0x32, /* REG_ARXR2_APGA_CTL	(0x1C)	*/
	0x00, /* REG_ATX2ARXPGA		(0x1D)	*/
	0x00, /* REG_BT_IF		(0x1E)	*/
	0x55, /* REG_BTPGA		(0x1F)	*/
	0x00, /* REG_BTSTPGA		(0x20)	*/
	0x00, /* REG_EAR_CTL		(0x21)	*/
	0x00, /* REG_HS_SEL		(0x22)	*/
	0x00, /* REG_HS_GAIN_SET	(0x23)	*/
	0x00, /* REG_HS_POPN_SET	(0x24)	*/
	0x00, /* REG_PREDL_CTL		(0x25)	*/
	0x00, /* REG_PREDR_CTL		(0x26)	*/
	0x00, /* REG_PRECKL_CTL		(0x27)	*/
	0x00, /* REG_PRECKR_CTL		(0x28)	*/
	0x00, /* REG_HFL_CTL		(0x29)	*/
	0x00, /* REG_HFR_CTL		(0x2A)	*/
	0x05, /* REG_ALC_CTL		(0x2B)	*/
	0x00, /* REG_ALC_SET1		(0x2C)	*/
	0x00, /* REG_ALC_SET2		(0x2D)	*/
	0x00, /* REG_BOOST_CTL		(0x2E)	*/
	0x00, /* REG_SOFTVOL_CTL	(0x2F)	*/
	0x13, /* REG_DTMF_FREQSEL	(0x30)	*/
	0x00, /* REG_DTMF_TONEXT1H	(0x31)	*/
	0x00, /* REG_DTMF_TONEXT1L	(0x32)	*/
	0x00, /* REG_DTMF_TONEXT2H	(0x33)	*/
	0x00, /* REG_DTMF_TONEXT2L	(0x34)	*/
	0x79, /* REG_DTMF_TONOFF	(0x35)	*/
	0x11, /* REG_DTMF_WANONOFF	(0x36)	*/
	0x00, /* REG_I2S_RX_SCRAMBLE_H	(0x37)	*/
	0x00, /* REG_I2S_RX_SCRAMBLE_M	(0x38)	*/
	0x00, /* REG_I2S_RX_SCRAMBLE_L	(0x39)	*/
	0x06, /* REG_APLL_CTL		(0x3A)	*/
	0x00, /* REG_DTMF_CTL		(0x3B)	*/
	0x44, /* REG_DTMF_PGA_CTL2	(0x3C)	*/
	0x69, /* REG_DTMF_PGA_CTL1	(0x3D)	*/
	0x00, /* REG_MISC_SET_1		(0x3E)	*/
	0x00, /* REG_PCMBTMUX		(0x3F)	*/
	0x00, /* not used		(0x40)	*/
	0x00, /* not used		(0x41)	*/
	0x00, /* not used		(0x42)	*/
	0x00, /* REG_RX_PATH_SEL	(0x43)	*/
	0x32, /* REG_VDL_APGA_CTL	(0x44)	*/
	0x00, /* REG_VIBRA_CTL		(0x45)	*/
	0x00, /* REG_VIBRA_SET		(0x46)	*/
	0x00, /* REG_VIBRA_PWM_SET	(0x47)	*/
	0x00, /* REG_ANAMIC_GAIN	(0x48)	*/
	0x00, /* REG_MISC_SET_2		(0x49)	*/
	0x00, /* REG_SW_SHADOW		(0x4A)	- Shadow, non HW register */
};
#endif

struct substream_item {
	struct list_head started;
	struct list_head configured;
	struct snd_pcm_substream *substream;
	int use256FS;
};

extern unsigned int get_headset_type(void);
// [bsnoh@ubiquix][20110528] : Added code from froyo hub [Start]
extern int get_dmb_status(void);
// [bsnoh@ubiquix][20110528] : Added code from froyo hub [End]

// prime@sdcmicro.com Added function to get the codec instance [START]
extern struct snd_soc_codec *snd_soc_get_codec(const char *name);
// prime@sdcmicro.com Added function to get the codec instance [END]
// [bsnoh@ubiquix][20110528] : Added code from froyo Justin [Start]
#if defined(CONFIG_PRODUCT_LGE_LU6800) || defined(CONFIG_PRODUCT_LGE_KU5900) || defined (CONFIG_PRODUCT_LGE_P970)
static const twl_reg_type twl_vt_receiver_call_tab[] =
{
	/*reg_control_type, address, data  */
	{TWL4030_CMD ,TWL4030_REG_CODEC_MODE, 0x92},
	{TWL4030_CMD ,TWL4030_REG_OPTION, 0xdf},
	{TWL4030_CMD ,TWL4030_REG_ANAMIC_GAIN, 0x12},//20110128 2d->24  //20110315 seungdae.goh@lge.com  NXP_EC_MCBSP3
	{TWL4030_CMD ,TWL4030_REG_VOICE_IF, 0x61},
	{TWL4030_CMD ,TWL4030_REG_ADCMICSEL, 0x00},
	{TWL4030_CMD ,TWL4030_REG_MICBIAS_CTL, 0x01},
	{TWL4030_CMD ,TWL4030_REG_ANAMICL, 0x71},
	{TWL4030_CMD ,TWL4030_REG_AVADC_CTL, 0x08},	//20100916 junyeop.kim@lge.com, only ADC L enable
	{TWL4030_CMD ,TWL4030_REG_AVDAC_CTL, 0x1c},
	{TWL4030_CMD ,TWL4030_REG_VSTPGA, 0x00},
	{TWL4030_CMD ,TWL4030_REG_VDL_APGA_CTL, 0x33},	//20110315 seungdae.goh@lge.com  NXP_EC_MCBSP3
	{TWL4030_CMD ,TWL4030_REG_ARXL2_APGA_CTL, 0x2b},	//20100908, junyeop.kim@lge.com, workaround for call playback sound
	{TWL4030_CMD ,TWL4030_REG_ARXR2_APGA_CTL, 0x2b},	//20100908, junyeop.kim@lge.com, workaround for call playback sound
	{TWL4030_CMD ,TWL4030_REG_AVTXL2PGA, 0x00},	//20110128 00->03  //20110315 seungdae.goh@lge.com  NXP_EC_MCBSP3
	{TWL4030_CMD ,TWL4030_REG_AVTXR2PGA, 0x03},	//20110128 00->03
	{TWL4030_CMD ,TWL4030_REG_PREDL_CTL, 0x00},	//20100916 junyeop.kim@lge.com, don't use reg. shut down
	{TWL4030_CMD ,TWL4030_REG_PREDR_CTL, 0x00},	//20100916 junyeop.kim@lge.com, don't use reg. shut down
	{TWL4030_CMD ,TWL4030_REG_HS_SEL, 0x00},
	{TWL4030_CMD ,TWL4030_REG_EAR_CTL, 0x15},
	{TWL4030_END_SEQ,0x00,0x00}
};

static const twl_reg_type twl_vt_headset_call_tab[] =
{
	{TWL4030_CMD ,TWL4030_REG_CODEC_MODE, 0x92},
	{TWL4030_CMD ,TWL4030_REG_OPTION, 0xdf},
	{TWL4030_CMD ,TWL4030_REG_ANAMIC_GAIN, 0x1B},	//  2d --> 24 H/W changes	//20110315 seungdae.goh@lge.com  NXP_EC_MCBSP3
	{TWL4030_CMD ,TWL4030_REG_VOICE_IF, 0x61},
	{TWL4030_CMD ,TWL4030_REG_ADCMICSEL, 0x00},
	{TWL4030_CMD ,TWL4030_REG_MICBIAS_CTL, 0x02},
	{TWL4030_CMD ,TWL4030_REG_ANAMICL, 0x72},
	{TWL4030_CMD ,TWL4030_REG_AVADC_CTL, 0x08},
	{TWL4030_CMD ,TWL4030_REG_AVDAC_CTL, 0x1c},
	{TWL4030_CMD ,TWL4030_REG_VSTPGA, 0x00},
	{TWL4030_CMD ,TWL4030_REG_VDL_APGA_CTL, 0x2b},	// 20101011 junyeop.kim@lge.com,voice call tuning (HW request) [START_LGE]  //20110315 seungdae.goh@lge.com  NXP_EC_MCBSP3
	{TWL4030_CMD ,TWL4030_REG_EAR_CTL, 0x30},
	{TWL4030_CMD ,TWL4030_REG_ARXL2_APGA_CTL, 0x2b},	//20100908, junyeop.kim@lge.com, workaround for call playback sound
	{TWL4030_CMD ,TWL4030_REG_ARXR2_APGA_CTL, 0x2b},	//20100908, junyeop.kim@lge.com, workaround for call playback sound
	{TWL4030_CMD ,TWL4030_REG_AVTXL2PGA, 0x04},		//0x05 --> 0x03 --> 0x00 H/W Changes  //20110315 seungdae.goh@lge.com  NXP_EC_MCBSP3
	{TWL4030_CMD ,TWL4030_REG_AVTXR2PGA, 0x00},		//0x05 --> 0x03 --> 0x00 H/W Changes
	{TWL4030_CMD ,TWL4030_REG_PREDL_CTL, 0x00},	//20100916 junyeop.kim@lge.com, don't use reg. shut down
	{TWL4030_CMD ,TWL4030_REG_PREDR_CTL, 0x00},	//20100916 junyeop.kim@lge.com, don't use reg. shut down
	{TWL4030_CMD ,TWL4030_REG_HS_SEL, 0x29},
	{TWL4030_CMD ,TWL4030_REG_HS_GAIN_SET, 0x05},
	{TWL4030_END_SEQ,0x00,0x00}
};

static const twl_reg_type twl_vt_speaker_call_tab[] =
{
	{TWL4030_CMD ,TWL4030_REG_CODEC_MODE, 0x92},
	{TWL4030_CMD ,TWL4030_REG_OPTION, 0xdf},
	{TWL4030_CMD ,TWL4030_REG_ANAMIC_GAIN, 0x24},	// 24 ( 20110128 - H/W Changes)
	{TWL4030_CMD ,TWL4030_REG_VOICE_IF, 0x61},
	{TWL4030_CMD ,TWL4030_REG_ADCMICSEL, 0x00},
	{TWL4030_CMD ,TWL4030_REG_MICBIAS_CTL, 0x02},
	{TWL4030_CMD ,TWL4030_REG_ANAMICL, 0x71},
	{TWL4030_CMD ,TWL4030_REG_AVADC_CTL, 0x08},	//20100824 junyeop.kim@lge.com, only ADC L enable
	{TWL4030_CMD ,TWL4030_REG_AVDAC_CTL, 0x1c},
	{TWL4030_CMD ,TWL4030_REG_VSTPGA, 0x00},
	{TWL4030_CMD ,TWL4030_REG_VDL_APGA_CTL, 0x2b},	//20110418 seungdae.goh@lge.com  0x2b --> 0x23 --> 0x2b

	{TWL4030_CMD ,TWL4030_REG_ARXL2_APGA_CTL, 0x23},	// 1b --> 23 ( 20110128 - H/W Changes)
	{TWL4030_CMD ,TWL4030_REG_ARXR2_APGA_CTL, 0x33},	// 23(4dB) --> 33(0dB) ( 20110217 - H/W Changes)
	{TWL4030_CMD ,TWL4030_REG_AVTXL2PGA, 0x01},	//20110315 seungdae.goh@lge.com  NXP_EC_MCBSP3
	{TWL4030_CMD ,TWL4030_REG_AVTXR2PGA, 0x00},		//0x00 --> 0x03 --> 0x00( 20110128 - H/W Changes)
	{TWL4030_CMD ,TWL4030_REG_ARXR2PGA, 0x3f},
	{TWL4030_CMD ,TWL4030_REG_ARXL2PGA, 0x3f},
	{TWL4030_CMD ,TWL4030_REG_EAR_CTL, 0x10},
	{TWL4030_CMD ,TWL4030_REG_HS_SEL, 0x09},
	{TWL4030_CMD ,TWL4030_REG_PREDL_CTL, 0x15},
	{TWL4030_CMD ,TWL4030_REG_PREDR_CTL, 0x15},
	{TWL4030_END_SEQ,0x00,0x00}
};

static const twl_reg_type twl_vt_headphone_call_tab[] =
{
	{TWL4030_CMD ,TWL4030_REG_CODEC_MODE, 0x92},
	{TWL4030_CMD ,TWL4030_REG_OPTION, 0xdf},
	{TWL4030_CMD ,TWL4030_REG_ANAMIC_GAIN, 0x1b},//tx 	: => 20111021 : request from H/W(ds.lee@lge.com)
	{TWL4030_CMD ,TWL4030_REG_VOICE_IF, 0x61},
	{TWL4030_CMD ,TWL4030_REG_ADCMICSEL, 0x00},
	{TWL4030_CMD ,TWL4030_REG_MICBIAS_CTL, 0x03},
	{TWL4030_CMD ,TWL4030_REG_ANAMICL, 0x71},
	{TWL4030_CMD ,TWL4030_REG_AVADC_CTL, 0x08},	//20100824 junyeop.kim@lge.com, only ADC L enable
	{TWL4030_CMD ,TWL4030_REG_AVDAC_CTL, 0x1c},
	{TWL4030_CMD ,TWL4030_REG_VSTPGA, 0x00},
	{TWL4030_CMD ,TWL4030_REG_VDL_APGA_CTL, 0x2b},	//0x0b(10dB)->0x33(0dB) => => 20111021 : request from H/W(ds.lee@lge.com)
	{TWL4030_CMD ,TWL4030_REG_ARXL2_APGA_CTL, 0x2b},	//20100908, junyeop.kim@lge.com, workaround for call playback sound
	{TWL4030_CMD ,TWL4030_REG_ARXR2_APGA_CTL, 0x2b},	//20100908, junyeop.kim@lge.com, workaround for call playback sound
	{TWL4030_CMD ,TWL4030_REG_AVTXL2PGA, 0x04},	//20110127 jisun.kwon => 20111021 : request from H/W(ds.lee@lge.com)
	{TWL4030_CMD ,TWL4030_REG_AVTXR2PGA, 0x00},	//20110127 jisun.kwon
	{TWL4030_CMD ,TWL4030_REG_EAR_CTL, 0x30},	//20100912, junyeop.kim@lge.com, workaround for call playback sound
	{TWL4030_CMD ,TWL4030_REG_PREDL_CTL, 0x00},	//20100916 junyeop.kim@lge.com, don't use reg. shut down
	{TWL4030_CMD ,TWL4030_REG_PREDR_CTL, 0x00},	//20100916 junyeop.kim@lge.com, don't use reg. shut down
	{TWL4030_CMD ,TWL4030_REG_HS_SEL, 0x29},
	{TWL4030_CMD ,TWL4030_REG_HS_GAIN_SET, 0x05},
	{TWL4030_END_SEQ,0x00,0x00}
};


static const twl_reg_type twl_vt_bt_call_tab[] =
{
	{TWL4030_CMD ,TWL4030_REG_CODEC_MODE, 0x90},
	{TWL4030_CMD ,TWL4030_REG_VOICE_IF, 0x84},
	{TWL4030_END_SEQ,0x00,0x00}
};
#endif // #if defined(CONFIG_PRODUCT_LGE_LU6800) || defined(CONFIG_PRODUCT_LGE_KU5900) || defined (CONFIG_PRODUCT_LGE_P970)

#if defined(CONFIG_PRODUCT_LGE_LU6800)

static const twl_reg_type twl_receiver_call_tab[] =
{
	{TWL4030_CMD ,TWL4030_REG_CODEC_MODE, 0x92},
	{TWL4030_CMD ,TWL4030_REG_OPTION, 0xdf},
	{TWL4030_CMD ,TWL4030_REG_ANAMIC_GAIN, 0x12},

	{TWL4030_CMD ,TWL4030_REG_AUDIO_IF, 0x03},		// gt.kim@lge.com 20120713 voice call state PCM Line Open

	{TWL4030_CMD ,TWL4030_REG_VOICE_IF, 0x61},
	{TWL4030_CMD ,TWL4030_REG_ADCMICSEL, 0x00},
	{TWL4030_CMD ,TWL4030_REG_MICBIAS_CTL, 0x01},
	{TWL4030_CMD ,TWL4030_REG_ANAMICL, 0x71},
	//    {TWL4030_CMD ,TWL4030_REG_AVADC_CTL, 0x0a},
	{TWL4030_CMD ,TWL4030_REG_AVADC_CTL, 0x08},	//20100916 junyeop.kim@lge.com, only ADC L enable
	{TWL4030_CMD ,TWL4030_REG_AVDAC_CTL, 0x1c},
	{TWL4030_CMD ,TWL4030_REG_VSTPGA, 0x00},
	{TWL4030_CMD ,TWL4030_REG_VRXPGA, 0x25},
	{TWL4030_CMD ,TWL4030_REG_VDL_APGA_CTL, 0x33},
	//    {TWL4030_CMD ,TWL4030_REG_VDL_APGA_CTL, 0x1b},	// 20100924 junyeop.kim@lge.com,voice call tuning (HW request) [START_LGE]
	{TWL4030_CMD ,TWL4030_REG_ARXL2_APGA_CTL, 0x2b},	//20100908, junyeop.kim@lge.com, workaround for call playback sound
	{TWL4030_CMD ,TWL4030_REG_ARXR2_APGA_CTL, 0x2b},	//20100908, junyeop.kim@lge.com, workaround for call playback sound
	{TWL4030_CMD ,TWL4030_REG_AVTXL2PGA, 0x00},	//20100912, junyeop.kim@lge.com, voice call tuning
	{TWL4030_CMD ,TWL4030_REG_AVTXR2PGA, 0x00},	//20100912, junyeop.kim@lge.com, voice call tuning

	{TWL4030_CMD ,TWL4030_REG_ARXR2PGA, 0x3f},	// gt.kim@lge.com 20120713 voice call state PCM Line Open
	{TWL4030_CMD ,TWL4030_REG_ARXL2PGA, 0x3f},  // gt.kim@lge.com 20120713 voice call state PCM Line Open

	//    {TWL4030_CMD ,TWL4030_REG_PREDL_CTL, 0x10},	//20100916 junyeop.kim@lge.com, don't use reg.
	//    {TWL4030_CMD ,TWL4030_REG_PREDR_CTL, 0x10},	//20100916 junyeop.kim@lge.com, don't use reg.
	{TWL4030_CMD ,TWL4030_REG_PREDL_CTL, 0x00},	//20100916 junyeop.kim@lge.com, don't use reg. shut down
	{TWL4030_CMD ,TWL4030_REG_PREDR_CTL, 0x00},	//20100916 junyeop.kim@lge.com, don't use reg. shut down
	{TWL4030_CMD ,TWL4030_REG_HS_SEL, 0x00},
	{TWL4030_CMD ,TWL4030_REG_EAR_CTL, 0x15},
	{TWL4030_END_SEQ,0x00,0x00}
};

static const twl_reg_type twl_receiver_call_dmb_tab[] =
{
	{TWL4030_CMD ,TWL4030_REG_CODEC_MODE, 0x92},
	{TWL4030_CMD ,TWL4030_REG_OPTION, 0xdf},
	{TWL4030_CMD ,TWL4030_REG_ANAMIC_GAIN, 0x1b},
	{TWL4030_CMD ,TWL4030_REG_VOICE_IF, 0x61},
	{TWL4030_CMD ,TWL4030_REG_ADCMICSEL, 0x00},
	{TWL4030_CMD ,TWL4030_REG_MICBIAS_CTL, 0x03},	 // 20101231 seungdae.goh@lge.com  MICBIAS2 disable  3 --> 1 --> 3
	{TWL4030_CMD ,TWL4030_REG_ANAMICL, 0x71},
	//    {TWL4030_CMD ,TWL4030_REG_AVADC_CTL, 0x0a},
	{TWL4030_CMD ,TWL4030_REG_AVADC_CTL, 0x08},	//20100916 junyeop.kim@lge.com, only ADC L enable
	{TWL4030_CMD ,TWL4030_REG_AVDAC_CTL, 0x1c},
	{TWL4030_CMD ,TWL4030_REG_VSTPGA, 0x00},
	//    {TWL4030_CMD ,TWL4030_REG_VRXPGA, 0x19},	// 20100912 junyeop.kim@lge.com,voice call tuning (HW request) [START_LGE]
	//    {TWL4030_CMD ,TWL4030_REG_VDL_APGA_CTL, 0x33},
	{TWL4030_CMD ,TWL4030_REG_VDL_APGA_CTL, 0x1b},	// 20100920 junyeop.kim@lge.com,voice call tuning (HW request) [START_LGE]
	{TWL4030_CMD ,TWL4030_REG_ARXL2_APGA_CTL, 0x2b},	//20100908, junyeop.kim@lge.com, workaround for call playback sound
	{TWL4030_CMD ,TWL4030_REG_ARXR2_APGA_CTL, 0x2b},	//20100908, junyeop.kim@lge.com, workaround for call playback sound
	{TWL4030_CMD ,TWL4030_REG_AVTXL2PGA, 0x00},	//20100912, junyeop.kim@lge.com, voice call tuning
	{TWL4030_CMD ,TWL4030_REG_AVTXR2PGA, 0x00},	//20100912, junyeop.kim@lge.com, voice call tuning
	//    {TWL4030_CMD ,TWL4030_REG_PREDL_CTL, 0x10},	//20100916 junyeop.kim@lge.com, don't use reg.
	//    {TWL4030_CMD ,TWL4030_REG_PREDR_CTL, 0x10},	//20100916 junyeop.kim@lge.com, don't use reg.
	{TWL4030_CMD ,TWL4030_REG_PREDL_CTL, 0x00},	//20100916 junyeop.kim@lge.com, don't use reg. shut down
	{TWL4030_CMD ,TWL4030_REG_PREDR_CTL, 0x00},	//20100916 junyeop.kim@lge.com, don't use reg. shut down
	{TWL4030_CMD ,TWL4030_REG_HS_SEL, 0x00},
	{TWL4030_CMD ,TWL4030_REG_EAR_CTL, 0x15},
	{TWL4030_END_SEQ,0x00,0x00}
};


static const twl_reg_type twl_headset_call_tab[] =
{
	{TWL4030_CMD ,TWL4030_REG_CODEC_MODE, 0x92},
	{TWL4030_CMD ,TWL4030_REG_OPTION, 0xdf},
	{TWL4030_CMD ,TWL4030_REG_ANAMIC_GAIN, 0x1b}, //gsd_mp_test

	{TWL4030_CMD ,TWL4030_REG_AUDIO_IF, 0x03},		// gt.kim@lge.com 20120713 voice call state PCM Line Open

	{TWL4030_CMD ,TWL4030_REG_VOICE_IF, 0x61},
	{TWL4030_CMD ,TWL4030_REG_ADCMICSEL, 0x00},
	{TWL4030_CMD ,TWL4030_REG_MICBIAS_CTL, 0x02}, // 20101231 seungdae.goh@lge.com  MICBIAS2 disable  2 --> 0 --> 2
	{TWL4030_CMD ,TWL4030_REG_ANAMICL, 0x72},
	{TWL4030_CMD ,TWL4030_REG_AVADC_CTL, 0x08},
	{TWL4030_CMD ,TWL4030_REG_AVDAC_CTL, 0x1c},
	{TWL4030_CMD ,TWL4030_REG_VSTPGA, 0x00},
//	{TWL4030_CMD ,TWL4030_REG_VRXPGA, 0x22},	// 20120714 gt.kim@lge.com Voice Volume settint register remove

	{TWL4030_CMD ,TWL4030_REG_VDL_APGA_CTL, 0x2b},

	{TWL4030_CMD ,TWL4030_REG_EAR_CTL, 0x30},
	{TWL4030_CMD ,TWL4030_REG_ARXL2_APGA_CTL, 0x2b},
	{TWL4030_CMD ,TWL4030_REG_ARXR2_APGA_CTL, 0x2b},	// 20110404  seungdae.goh@lge.com error fix
	//{TWL4030_CMD ,TWL4030_REG_ATX2ARXPGA, 0x34},	//20110317 seungdae.goh


	{TWL4030_CMD ,TWL4030_REG_AVTXL2PGA, 0x04}, //gsd_mp_test
	{TWL4030_CMD ,TWL4030_REG_AVTXR2PGA, 0x05},

	{TWL4030_CMD ,TWL4030_REG_ARXR2PGA, 0x3f},	// gt.kim@lge.com 20120713 voice call state PCM Line Open
	{TWL4030_CMD ,TWL4030_REG_ARXL2PGA, 0x3f},  // gt.kim@lge.com 20120713 voice call state PCM Line Open

	{TWL4030_CMD ,TWL4030_REG_PREDL_CTL, 0x00},
	{TWL4030_CMD ,TWL4030_REG_PREDR_CTL, 0x00},
	{TWL4030_CMD ,TWL4030_REG_HS_SEL, 0x29},
	{TWL4030_CMD ,TWL4030_REG_HS_GAIN_SET, 0x05},

	{TWL4030_END_SEQ,0x00,0x00}
};

static const twl_reg_type twl_speaker_call_tab[] =
{
	{TWL4030_CMD ,TWL4030_REG_CODEC_MODE, 0x92},
	{TWL4030_CMD ,TWL4030_REG_OPTION, 0xdf},
	{TWL4030_CMD ,TWL4030_REG_ANAMIC_GAIN, 0x24},

	{TWL4030_CMD ,TWL4030_REG_AUDIO_IF, 0x03},		// gt.kim@lge.com 20120713 voice call state PCM Line Open

	{TWL4030_CMD ,TWL4030_REG_VOICE_IF, 0x61},
	{TWL4030_CMD ,TWL4030_REG_ADCMICSEL, 0x00},
	{TWL4030_CMD ,TWL4030_REG_MICBIAS_CTL, 0x02},
	{TWL4030_CMD ,TWL4030_REG_ANAMICL, 0x71},

	{TWL4030_CMD ,TWL4030_REG_AVADC_CTL, 0x08},	//20100824 junyeop.kim@lge.com, only ADC L enable
	{TWL4030_CMD ,TWL4030_REG_AVDAC_CTL, 0x1c},
	{TWL4030_CMD ,TWL4030_REG_VSTPGA, 0x00},
//	{TWL4030_CMD ,TWL4030_REG_VRXPGA, 0x26},	// 20120714 gt.kim@lge.com Voice Volume settint register remove
	{TWL4030_CMD ,TWL4030_REG_VDL_APGA_CTL, 0x2b}, //20110418 seungdae.goh@lge.com  0x2b --> 0x23 --> 0x2b

	{TWL4030_CMD ,TWL4030_REG_ARXL2_APGA_CTL, 0x1b},
	{TWL4030_CMD ,TWL4030_REG_ARXR2_APGA_CTL, 0x1b},
	{TWL4030_CMD ,TWL4030_REG_AVTXL2PGA, 0x01},
	{TWL4030_CMD ,TWL4030_REG_AVTXR2PGA, 0x00},
	{TWL4030_CMD ,TWL4030_REG_ARXR2PGA, 0x3f},
	{TWL4030_CMD ,TWL4030_REG_ARXL2PGA, 0x3f},
	{TWL4030_CMD ,TWL4030_REG_EAR_CTL, 0x10},
	{TWL4030_CMD ,TWL4030_REG_HS_SEL, 0x09},
	{TWL4030_CMD ,TWL4030_REG_PREDL_CTL, 0x15},
	{TWL4030_CMD ,TWL4030_REG_PREDR_CTL, 0x15},
	{TWL4030_END_SEQ,0x00,0x00}
};

static const twl_reg_type twl_speaker_call_headset_det_tab[] =
{
	{TWL4030_CMD ,TWL4030_REG_CODEC_MODE, 0x92},
	{TWL4030_CMD ,TWL4030_REG_OPTION, 0xdf},
	{TWL4030_CMD ,TWL4030_REG_ANAMIC_GAIN, 0x24},
	{TWL4030_CMD ,TWL4030_REG_VOICE_IF, 0x61},
	{TWL4030_CMD ,TWL4030_REG_ADCMICSEL, 0x00},
	{TWL4030_CMD ,TWL4030_REG_MICBIAS_CTL, 0x03},	// 20101231 seungdae.goh@lge.com  MICBIAS2 disable   3 --> 1 --> 3
	{TWL4030_CMD ,TWL4030_REG_ANAMICL, 0x71},
	//    {TWL4030_CMD ,TWL4030_REG_AVADC_CTL, 0x0a},
	{TWL4030_CMD ,TWL4030_REG_AVADC_CTL, 0x08},	//20100824 junyeop.kim@lge.com, only ADC L enable
	{TWL4030_CMD ,TWL4030_REG_AVDAC_CTL, 0x1c},
	{TWL4030_CMD ,TWL4030_REG_VSTPGA, 0x00},
	//    {TWL4030_CMD ,TWL4030_REG_VRXPGA, 0x19},	// 20100912 junyeop.kim@lge.com,voice call tuning (HW request) [START_LGE]
	{TWL4030_CMD ,TWL4030_REG_VDL_APGA_CTL, 0x33},
	//    {TWL4030_CMD ,TWL4030_REG_VDL_APGA_CTL, 0x23},	// 20100920 junyeop.kim@lge.com,voice call tuning (HW request) [START_LGE]
	{TWL4030_CMD ,TWL4030_REG_ARXL2_APGA_CTL, 0x2b},	//20100908, junyeop.kim@lge.com, workaround for call playback sound
	{TWL4030_CMD ,TWL4030_REG_ARXR2_APGA_CTL, 0x2b},	//20100908, junyeop.kim@lge.com, workaround for call playback sound
	{TWL4030_CMD ,TWL4030_REG_AVTXL2PGA, 0x00},		//20100912, junyeop.kim@lge.com, voice call tuning
	{TWL4030_CMD ,TWL4030_REG_AVTXR2PGA, 0x00},		//20100912, junyeop.kim@lge.com, voice call tuning
	{TWL4030_CMD ,TWL4030_REG_EAR_CTL, 0x10},
	{TWL4030_CMD ,TWL4030_REG_HS_SEL, 0x09},
	{TWL4030_CMD ,TWL4030_REG_PREDL_CTL, 0x25},
	{TWL4030_CMD ,TWL4030_REG_PREDR_CTL, 0x25},
	{TWL4030_END_SEQ,0x00,0x00}
};

static const twl_reg_type twl_headphone_call_tab[] =
{
	{TWL4030_CMD ,TWL4030_REG_CODEC_MODE, 0x92},
	{TWL4030_CMD ,TWL4030_REG_OPTION, 0xdf},
	{TWL4030_CMD ,TWL4030_REG_ANAMIC_GAIN, 0x1b},
	{TWL4030_CMD ,TWL4030_REG_VOICE_IF, 0x61},
	{TWL4030_CMD ,TWL4030_REG_ADCMICSEL, 0x00},
	{TWL4030_CMD ,TWL4030_REG_MICBIAS_CTL, 0x03},  // 20101231 seungdae.goh@lge.com  MICBIAS2 disable 3-->1 -->3
	{TWL4030_CMD ,TWL4030_REG_ANAMICL, 0x71},
	//    {TWL4030_CMD ,TWL4030_REG_AVADC_CTL, 0x0a},
	{TWL4030_CMD ,TWL4030_REG_AVADC_CTL, 0x08},	//20100824 junyeop.kim@lge.com, only ADC L enable
	{TWL4030_CMD ,TWL4030_REG_AVDAC_CTL, 0x1c},
	{TWL4030_CMD ,TWL4030_REG_VSTPGA, 0x00},
	//    {TWL4030_CMD ,TWL4030_REG_VRXPGA, 0x19},	// 20100912 junyeop.kim@lge.com,voice call tuning (HW request) [START_LGE]
	//    {TWL4030_CMD ,TWL4030_REG_VDL_APGA_CTL, 0x33},
	{TWL4030_CMD ,TWL4030_REG_VDL_APGA_CTL, 0x2b},
	{TWL4030_CMD ,TWL4030_REG_ARXL2_APGA_CTL, 0x2b},	//20100908, junyeop.kim@lge.com, workaround for call playback sound
	{TWL4030_CMD ,TWL4030_REG_ARXR2_APGA_CTL, 0x2b},	//20100908, junyeop.kim@lge.com, workaround for call playback sound
	{TWL4030_CMD ,TWL4030_REG_AVTXL2PGA, 0x04},	//20110227  seungdae.goh@lge.com
	{TWL4030_CMD ,TWL4030_REG_AVTXR2PGA, 0x00},	//20100912, junyeop.kim@lge.com, voice call tuning
	{TWL4030_CMD ,TWL4030_REG_EAR_CTL, 0x30},	//20100912, junyeop.kim@lge.com, workaround for call playback sound
	//    {TWL4030_CMD ,TWL4030_REG_PREDL_CTL, 0x10},	//20100916 junyeop.kim@lge.com, don't use reg.
	//    {TWL4030_CMD ,TWL4030_REG_PREDR_CTL, 0x10},	//20100916 junyeop.kim@lge.com, don't use reg.
	{TWL4030_CMD ,TWL4030_REG_PREDL_CTL, 0x00},	//20100916 junyeop.kim@lge.com, don't use reg. shut down
	{TWL4030_CMD ,TWL4030_REG_PREDR_CTL, 0x00},	//20100916 junyeop.kim@lge.com, don't use reg. shut down
	{TWL4030_CMD ,TWL4030_REG_HS_SEL, 0x29},
	{TWL4030_CMD ,TWL4030_REG_HS_GAIN_SET, 0x05},
	//    {TWL4030_CMD ,TWL4030_REG_HS_POPN_SET, 0x42},
	//    {TWL4030_CMD ,TWL4030_REG_HS_POPN_SET, 0x00}, 	//20101017 junyeop.kim@lge.com, modify the headset call
	{TWL4030_END_SEQ,0x00,0x00}
};

#elif defined (CONFIG_PRODUCT_LGE_P970)
static const twl_reg_type twl_receiver_call_tab[] =
{
	{TWL4030_CMD ,TWL4030_REG_CODEC_MODE, 0x92},
	{TWL4030_CMD ,TWL4030_REG_OPTION, 0xdf},
	{TWL4030_CMD ,TWL4030_REG_ANAMIC_GAIN, 0x2d}, // 20121030 bs.lim@lge.com

	{TWL4030_CMD ,TWL4030_REG_AUDIO_IF, 0x03},		// gt.kim@lge.com 20120713 voice call state PCM Line Open

	{TWL4030_CMD ,TWL4030_REG_VOICE_IF, 0x61},
	{TWL4030_CMD ,TWL4030_REG_ADCMICSEL, 0x00},
	{TWL4030_CMD ,TWL4030_REG_MICBIAS_CTL, 0x01},

	{TWL4030_CMD ,TWL4030_REG_ANAMICR, 0x00}, // 20121030 bs.lim@lge.com
	
	{TWL4030_CMD ,TWL4030_REG_ANAMICL, 0x71},
	//    {TWL4030_CMD ,TWL4030_REG_AVADC_CTL, 0x0a},
	{TWL4030_CMD ,TWL4030_REG_AVADC_CTL, 0x08},	//20100916 junyeop.kim@lge.com, only ADC L enable
	{TWL4030_CMD ,TWL4030_REG_AVDAC_CTL, 0x1c},
	{TWL4030_CMD ,TWL4030_REG_VSTPGA, 0x00},
	{TWL4030_CMD ,TWL4030_REG_VRXPGA, 0x25},
	{TWL4030_CMD ,TWL4030_REG_VDL_APGA_CTL, 0x2b}, // 20121030 bs.lim@lge.com
	//    {TWL4030_CMD ,TWL4030_REG_VDL_APGA_CTL, 0x1b},	// 20100924 junyeop.kim@lge.com,voice call tuning (HW request) [START_LGE]
	{TWL4030_CMD ,TWL4030_REG_ARXL2_APGA_CTL, 0x2b},	//20100908, junyeop.kim@lge.com, workaround for call playback sound
	{TWL4030_CMD ,TWL4030_REG_ARXR2_APGA_CTL, 0x2b},	//20100908, junyeop.kim@lge.com, workaround for call playback sound
	{TWL4030_CMD ,TWL4030_REG_AVTXL2PGA, 0x00},	//20100912, junyeop.kim@lge.com, voice call tuning
	{TWL4030_CMD ,TWL4030_REG_AVTXR2PGA, 0x00},	//20100912, junyeop.kim@lge.com, voice call tuning

	{TWL4030_CMD ,TWL4030_REG_ARXR2PGA, 0x3f},	// gt.kim@lge.com 20120713 voice call state PCM Line Open
	{TWL4030_CMD ,TWL4030_REG_ARXL2PGA, 0x3f},  // gt.kim@lge.com 20120713 voice call state PCM Line Open

	//    {TWL4030_CMD ,TWL4030_REG_PREDL_CTL, 0x10},	//20100916 junyeop.kim@lge.com, don't use reg.
	//    {TWL4030_CMD ,TWL4030_REG_PREDR_CTL, 0x10},	//20100916 junyeop.kim@lge.com, don't use reg.
	{TWL4030_CMD ,TWL4030_REG_PREDL_CTL, 0x00},	//20100916 junyeop.kim@lge.com, don't use reg. shut down
	{TWL4030_CMD ,TWL4030_REG_PREDR_CTL, 0x00},	//20100916 junyeop.kim@lge.com, don't use reg. shut down
	{TWL4030_CMD ,TWL4030_REG_HS_SEL, 0x00},
	{TWL4030_CMD ,TWL4030_REG_EAR_CTL, 0x15},
	{TWL4030_END_SEQ,0x00,0x00}
};

static const twl_reg_type twl_receiver_call_dmb_tab[] =
{
	{TWL4030_CMD ,TWL4030_REG_CODEC_MODE, 0x92},
	{TWL4030_CMD ,TWL4030_REG_OPTION, 0xdf},
	{TWL4030_CMD ,TWL4030_REG_ANAMIC_GAIN, 0x1b},
	{TWL4030_CMD ,TWL4030_REG_VOICE_IF, 0x61},
	{TWL4030_CMD ,TWL4030_REG_ADCMICSEL, 0x00},
	{TWL4030_CMD ,TWL4030_REG_MICBIAS_CTL, 0x03},	 // 20101231 seungdae.goh@lge.com  MICBIAS2 disable  3 --> 1 --> 3
	{TWL4030_CMD ,TWL4030_REG_ANAMICL, 0x71},
	//    {TWL4030_CMD ,TWL4030_REG_AVADC_CTL, 0x0a},
	{TWL4030_CMD ,TWL4030_REG_AVADC_CTL, 0x08},	//20100916 junyeop.kim@lge.com, only ADC L enable
	{TWL4030_CMD ,TWL4030_REG_AVDAC_CTL, 0x1c},
	{TWL4030_CMD ,TWL4030_REG_VSTPGA, 0x00},
	//    {TWL4030_CMD ,TWL4030_REG_VRXPGA, 0x19},	// 20100912 junyeop.kim@lge.com,voice call tuning (HW request) [START_LGE]
	//    {TWL4030_CMD ,TWL4030_REG_VDL_APGA_CTL, 0x33},
	{TWL4030_CMD ,TWL4030_REG_VDL_APGA_CTL, 0x1b},	// 20100920 junyeop.kim@lge.com,voice call tuning (HW request) [START_LGE]
	{TWL4030_CMD ,TWL4030_REG_ARXL2_APGA_CTL, 0x2b},	//20100908, junyeop.kim@lge.com, workaround for call playback sound
	{TWL4030_CMD ,TWL4030_REG_ARXR2_APGA_CTL, 0x2b},	//20100908, junyeop.kim@lge.com, workaround for call playback sound
	{TWL4030_CMD ,TWL4030_REG_AVTXL2PGA, 0x00},	//20100912, junyeop.kim@lge.com, voice call tuning
	{TWL4030_CMD ,TWL4030_REG_AVTXR2PGA, 0x00},	//20100912, junyeop.kim@lge.com, voice call tuning
	//    {TWL4030_CMD ,TWL4030_REG_PREDL_CTL, 0x10},	//20100916 junyeop.kim@lge.com, don't use reg.
	//    {TWL4030_CMD ,TWL4030_REG_PREDR_CTL, 0x10},	//20100916 junyeop.kim@lge.com, don't use reg.
	{TWL4030_CMD ,TWL4030_REG_PREDL_CTL, 0x00},	//20100916 junyeop.kim@lge.com, don't use reg. shut down
	{TWL4030_CMD ,TWL4030_REG_PREDR_CTL, 0x00},	//20100916 junyeop.kim@lge.com, don't use reg. shut down
	{TWL4030_CMD ,TWL4030_REG_HS_SEL, 0x00},
	{TWL4030_CMD ,TWL4030_REG_EAR_CTL, 0x15},
	{TWL4030_END_SEQ,0x00,0x00}
};

static const twl_reg_type twl_headset_call_tab[] =
{
	{TWL4030_CMD ,TWL4030_REG_CODEC_MODE, 0x92},
	{TWL4030_CMD ,TWL4030_REG_OPTION, 0xdf},
	{TWL4030_CMD ,TWL4030_REG_ANAMIC_GAIN, 0x24}, // 20121030 bs.lim@lge.com

	{TWL4030_CMD ,TWL4030_REG_AUDIO_IF, 0x03},		// gt.kim@lge.com 20120713 voice call state PCM Line Open

	{TWL4030_CMD ,TWL4030_REG_VOICE_IF, 0x61},
	{TWL4030_CMD ,TWL4030_REG_ADCMICSEL, 0x00},
	{TWL4030_CMD ,TWL4030_REG_MICBIAS_CTL, 0x02}, // 20101231 seungdae.goh@lge.com  MICBIAS2 disable  2 --> 0 --> 2
	{TWL4030_CMD ,TWL4030_REG_ANAMICL, 0x72},
	{TWL4030_CMD ,TWL4030_REG_AVADC_CTL, 0x08},
	{TWL4030_CMD ,TWL4030_REG_AVDAC_CTL, 0x1c},
	{TWL4030_CMD ,TWL4030_REG_VSTPGA, 0x00},

	{TWL4030_CMD ,TWL4030_REG_ANAMICR, 0x00}, // 20121030 bs.lim@lge.com
	{TWL4030_CMD ,TWL4030_REG_ATXL1PGA, 0x00}, // 20121030 bs.lim@lge.com
	{TWL4030_CMD ,TWL4030_REG_ATXR1PGA, 0x00}, // 20121030 bs.lim@lge.com
	
//	{TWL4030_CMD ,TWL4030_REG_VRXPGA, 0x22},	// 20120714 gt.kim@lge.com Voice Volume settint register remove

	{TWL4030_CMD ,TWL4030_REG_VDL_APGA_CTL, 0x33}, // 20121030 bs.lim@lge.com

	{TWL4030_CMD ,TWL4030_REG_EAR_CTL, 0x30},
	{TWL4030_CMD ,TWL4030_REG_ARXL2_APGA_CTL, 0x2b},
	{TWL4030_CMD ,TWL4030_REG_ARXR2_APGA_CTL, 0x2b},	// 20110404  seungdae.goh@lge.com error fix
	//{TWL4030_CMD ,TWL4030_REG_ATX2ARXPGA, 0x34},	//20110317 seungdae.goh


	{TWL4030_CMD ,TWL4030_REG_AVTXL2PGA, 0x05}, // 20121030 bs.lim@lge.com
	{TWL4030_CMD ,TWL4030_REG_AVTXR2PGA, 0x05},

	{TWL4030_CMD ,TWL4030_REG_ARXR2PGA, 0x3f},	// gt.kim@lge.com 20120713 voice call state PCM Line Open
	{TWL4030_CMD ,TWL4030_REG_ARXL2PGA, 0x3f},  // gt.kim@lge.com 20120713 voice call state PCM Line Open

	{TWL4030_CMD ,TWL4030_REG_PREDL_CTL, 0x00},
	{TWL4030_CMD ,TWL4030_REG_PREDR_CTL, 0x00},
	{TWL4030_CMD ,TWL4030_REG_HS_SEL, 0x29},
	{TWL4030_CMD ,TWL4030_REG_HS_GAIN_SET, 0x05},

	{TWL4030_END_SEQ,0x00,0x00}
};

static const twl_reg_type twl_headphone_call_tab[] =
{
	{TWL4030_CMD ,TWL4030_REG_CODEC_MODE, 0x92},
	{TWL4030_CMD ,TWL4030_REG_OPTION, 0xdf},
	{TWL4030_CMD ,TWL4030_REG_ANAMIC_GAIN, 0x24}, // 20121030 bs.lim@lge.com
	
	{TWL4030_CMD ,TWL4030_REG_VOICE_IF, 0x61},
	{TWL4030_CMD ,TWL4030_REG_ADCMICSEL, 0x00},
	{TWL4030_CMD ,TWL4030_REG_MICBIAS_CTL, 0x03},  // 20101231 seungdae.goh@lge.com  MICBIAS2 disable 3-->1 -->3
	{TWL4030_CMD ,TWL4030_REG_ANAMICL, 0x71},
	//    {TWL4030_CMD ,TWL4030_REG_AVADC_CTL, 0x0a},
	{TWL4030_CMD ,TWL4030_REG_AVADC_CTL, 0x08},	//20100824 junyeop.kim@lge.com, only ADC L enable
	{TWL4030_CMD ,TWL4030_REG_AVDAC_CTL, 0x1c},
	{TWL4030_CMD ,TWL4030_REG_VSTPGA, 0x00},

	{TWL4030_CMD ,TWL4030_REG_ANAMICR, 0x00}, // 20121030 bs.lim@lge.com
	
	//    {TWL4030_CMD ,TWL4030_REG_VRXPGA, 0x19},	// 20100912 junyeop.kim@lge.com,voice call tuning (HW request) [START_LGE]
	//    {TWL4030_CMD ,TWL4030_REG_VDL_APGA_CTL, 0x33},
	{TWL4030_CMD ,TWL4030_REG_VDL_APGA_CTL, 0x33}, // 20121030 bs.lim@lge.com
	
	{TWL4030_CMD ,TWL4030_REG_ARXL2_APGA_CTL, 0x2b},	//20100908, junyeop.kim@lge.com, workaround for call playback sound
	{TWL4030_CMD ,TWL4030_REG_ARXR2_APGA_CTL, 0x2b},	//20100908, junyeop.kim@lge.com, workaround for call playback sound
	{TWL4030_CMD ,TWL4030_REG_AVTXL2PGA, 0x04},	//20110227  seungdae.goh@lge.com
	{TWL4030_CMD ,TWL4030_REG_AVTXR2PGA, 0x00},	//20100912, junyeop.kim@lge.com, voice call tuning
	{TWL4030_CMD ,TWL4030_REG_EAR_CTL, 0x30},	//20100912, junyeop.kim@lge.com, workaround for call playback sound
	//    {TWL4030_CMD ,TWL4030_REG_PREDL_CTL, 0x10},	//20100916 junyeop.kim@lge.com, don't use reg.
	//    {TWL4030_CMD ,TWL4030_REG_PREDR_CTL, 0x10},	//20100916 junyeop.kim@lge.com, don't use reg.
	{TWL4030_CMD ,TWL4030_REG_PREDL_CTL, 0x00},	//20100916 junyeop.kim@lge.com, don't use reg. shut down
	{TWL4030_CMD ,TWL4030_REG_PREDR_CTL, 0x00},	//20100916 junyeop.kim@lge.com, don't use reg. shut down
	{TWL4030_CMD ,TWL4030_REG_HS_SEL, 0x29},
	{TWL4030_CMD ,TWL4030_REG_HS_GAIN_SET, 0x05},
	//    {TWL4030_CMD ,TWL4030_REG_HS_POPN_SET, 0x42},
	//    {TWL4030_CMD ,TWL4030_REG_HS_POPN_SET, 0x00}, 	//20101017 junyeop.kim@lge.com, modify the headset call
	{TWL4030_END_SEQ,0x00,0x00}
};

static const twl_reg_type twl_speaker_call_headset_det_tab[] =
{
	{TWL4030_CMD ,TWL4030_REG_CODEC_MODE, 0x92},
	{TWL4030_CMD ,TWL4030_REG_OPTION, 0xdf},
	{TWL4030_CMD ,TWL4030_REG_ANAMIC_GAIN, 0x24},
	{TWL4030_CMD ,TWL4030_REG_VOICE_IF, 0x61},
	{TWL4030_CMD ,TWL4030_REG_ADCMICSEL, 0x00},
	{TWL4030_CMD ,TWL4030_REG_MICBIAS_CTL, 0x03},	// 20101231 seungdae.goh@lge.com  MICBIAS2 disable   3 --> 1 --> 3
	{TWL4030_CMD ,TWL4030_REG_ANAMICL, 0x71},
	//    {TWL4030_CMD ,TWL4030_REG_AVADC_CTL, 0x0a},
	{TWL4030_CMD ,TWL4030_REG_AVADC_CTL, 0x08},	//20100824 junyeop.kim@lge.com, only ADC L enable
	{TWL4030_CMD ,TWL4030_REG_AVDAC_CTL, 0x1c},
	{TWL4030_CMD ,TWL4030_REG_VSTPGA, 0x00},
	//    {TWL4030_CMD ,TWL4030_REG_VRXPGA, 0x19},	// 20100912 junyeop.kim@lge.com,voice call tuning (HW request) [START_LGE]
	{TWL4030_CMD ,TWL4030_REG_VDL_APGA_CTL, 0x33},
	//    {TWL4030_CMD ,TWL4030_REG_VDL_APGA_CTL, 0x23},	// 20100920 junyeop.kim@lge.com,voice call tuning (HW request) [START_LGE]
	{TWL4030_CMD ,TWL4030_REG_ARXL2_APGA_CTL, 0x2b},	//20100908, junyeop.kim@lge.com, workaround for call playback sound
	{TWL4030_CMD ,TWL4030_REG_ARXR2_APGA_CTL, 0x2b},	//20100908, junyeop.kim@lge.com, workaround for call playback sound
	{TWL4030_CMD ,TWL4030_REG_AVTXL2PGA, 0x00},		//20100912, junyeop.kim@lge.com, voice call tuning
	{TWL4030_CMD ,TWL4030_REG_AVTXR2PGA, 0x00},		//20100912, junyeop.kim@lge.com, voice call tuning
	{TWL4030_CMD ,TWL4030_REG_EAR_CTL, 0x10},
	{TWL4030_CMD ,TWL4030_REG_HS_SEL, 0x09},
	{TWL4030_CMD ,TWL4030_REG_PREDL_CTL, 0x25},
	{TWL4030_CMD ,TWL4030_REG_PREDR_CTL, 0x25},
	{TWL4030_END_SEQ,0x00,0x00}
};

static const twl_reg_type twl_speaker_call_tab[] =
{
	{TWL4030_CMD ,TWL4030_REG_CODEC_MODE, 0x92},
	{TWL4030_CMD ,TWL4030_REG_OPTION, 0xdf},
	
	{TWL4030_CMD ,TWL4030_REG_ANAMIC_GAIN, 0x1b}, // 20121030 bs.lim@lge.com

	{TWL4030_CMD ,TWL4030_REG_AUDIO_IF, 0x03},		// gt.kim@lge.com 20120713 voice call state PCM Line Open

	{TWL4030_CMD ,TWL4030_REG_VOICE_IF, 0x61},
	{TWL4030_CMD ,TWL4030_REG_ADCMICSEL, 0x00},
	{TWL4030_CMD ,TWL4030_REG_MICBIAS_CTL, 0x02},

	{TWL4030_CMD ,TWL4030_REG_ANAMICR, 0x00}, // 20121030 bs.lim@lge.com
	
	{TWL4030_CMD ,TWL4030_REG_ANAMICL, 0x71},

	{TWL4030_CMD ,TWL4030_REG_AVADC_CTL, 0x08},	//20100824 junyeop.kim@lge.com, only ADC L enable
	{TWL4030_CMD ,TWL4030_REG_AVDAC_CTL, 0x1c},
	{TWL4030_CMD ,TWL4030_REG_VSTPGA, 0x00},
	
	{TWL4030_CMD ,TWL4030_REG_VRXPGA, 0x26},	// 20121030 bs.lim@lge.com
	
	{TWL4030_CMD ,TWL4030_REG_VDL_APGA_CTL, 0x33}, // 20121030 bs.lim@lge.com

	{TWL4030_CMD ,TWL4030_REG_ATXL1PGA, 0x00}, // 20121030 bs.lim@lge.com
	{TWL4030_CMD ,TWL4030_REG_AVTXL2PGA, 0x00}, // 20121030 bs.lim@lge.com
	
	{TWL4030_CMD ,TWL4030_REG_ARXL2_APGA_CTL, 0x2b}, // 20121030 bs.lim@lge.com
	{TWL4030_CMD ,TWL4030_REG_ARXR2_APGA_CTL, 0x2b}, // 20121030 bs.lim@lge.com
	
	{TWL4030_CMD ,TWL4030_REG_AVTXL2PGA, 0x01},
	{TWL4030_CMD ,TWL4030_REG_AVTXR2PGA, 0x00},
	{TWL4030_CMD ,TWL4030_REG_ARXR2PGA, 0x3f},
	{TWL4030_CMD ,TWL4030_REG_ARXL2PGA, 0x3f},
	{TWL4030_CMD ,TWL4030_REG_EAR_CTL, 0x10},
	{TWL4030_CMD ,TWL4030_REG_HS_SEL, 0x09},
	
	{TWL4030_CMD ,TWL4030_REG_PREDL_CTL, 0x25}, // 20121030 bs.lim@lge.com
	{TWL4030_CMD ,TWL4030_REG_PREDR_CTL, 0x25}, // 20121030 bs.lim@lge.com
	{TWL4030_END_SEQ,0x00,0x00}
};

#elif defined(CONFIG_PRODUCT_LGE_KU5900) //|| defined (CONFIG_PRODUCT_LGE_P970)
static const twl_reg_type twl_receiver_call_tab[] =
{
    // wooho.jeong@lge.com 2012.10.05
    // MOD : for 44.1K audio sample rate
	//{TWL4030_CMD ,TWL4030_REG_CODEC_MODE, 0xa2},
    {TWL4030_CMD ,TWL4030_REG_CODEC_MODE, 0x92},        
	{TWL4030_CMD ,TWL4030_REG_OPTION, 0xdf},
	{TWL4030_CMD ,TWL4030_REG_ANAMIC_GAIN, 0x2d},
	{TWL4030_CMD ,TWL4030_REG_VOICE_IF, 0x61},
	{TWL4030_CMD ,TWL4030_REG_AUDIO_IF, 0x03},
	{TWL4030_CMD ,TWL4030_REG_ADCMICSEL, 0x00},
	{TWL4030_CMD ,TWL4030_REG_MICBIAS_CTL, 0x01},
	{TWL4030_CMD ,TWL4030_REG_ANAMICL, 0x71},
	{TWL4030_CMD ,TWL4030_REG_AVADC_CTL, 0x08},	//20100916 junyeop.kim@lge.com, only ADC L enable
	{TWL4030_CMD ,TWL4030_REG_AVDAC_CTL, 0x1c},
	{TWL4030_CMD ,TWL4030_REG_VSTPGA, 0x00},
	{TWL4030_CMD ,TWL4030_REG_VDL_APGA_CTL, 0x2b},
	{TWL4030_CMD ,TWL4030_REG_ARXL2_APGA_CTL, 0x2b},	//20100908, junyeop.kim@lge.com, workaround for call playback sound
	{TWL4030_CMD ,TWL4030_REG_ARXR2_APGA_CTL, 0x2b},	//20100908, junyeop.kim@lge.com, workaround for call playback sound
	{TWL4030_CMD ,TWL4030_REG_AVTXL2PGA, 0x00},	//20100912, junyeop.kim@lge.com, voice call tuning
	{TWL4030_CMD ,TWL4030_REG_AVTXR2PGA, 0x00},	//20100912, junyeop.kim@lge.com, voice call tuning
	{TWL4030_CMD ,TWL4030_REG_PREDL_CTL, 0x00},	//20100916 junyeop.kim@lge.com, don't use reg. shut down
	{TWL4030_CMD ,TWL4030_REG_PREDR_CTL, 0x00},	//20100916 junyeop.kim@lge.com, don't use reg. shut down
	{TWL4030_CMD ,TWL4030_REG_HS_SEL, 0x00},
	{TWL4030_CMD ,TWL4030_REG_EAR_CTL, 0x15},
	{TWL4030_END_SEQ,0x00,0x00}
};

static const twl_reg_type twl_receiver_call_dmb_tab[] =
{
    // wooho.jeong@lge.com 2012.10.05
    // MOD : for 44.1K audio sample rate
	//{TWL4030_CMD ,TWL4030_REG_CODEC_MODE, 0xa2},
    {TWL4030_CMD ,TWL4030_REG_CODEC_MODE, 0x92},        
	{TWL4030_CMD ,TWL4030_REG_OPTION, 0xdf},
	{TWL4030_CMD ,TWL4030_REG_ANAMIC_GAIN, 0x1b},
	{TWL4030_CMD ,TWL4030_REG_VOICE_IF, 0x61},
	{TWL4030_CMD ,TWL4030_REG_ADCMICSEL, 0x00},
	{TWL4030_CMD ,TWL4030_REG_MICBIAS_CTL, 0x03},	//different to original receiver tab
	{TWL4030_CMD ,TWL4030_REG_ANAMICL, 0x71},
	{TWL4030_CMD ,TWL4030_REG_AVADC_CTL, 0x08},	//20100916 junyeop.kim@lge.com, only ADC L enable
	{TWL4030_CMD ,TWL4030_REG_AVDAC_CTL, 0x1c},
	{TWL4030_CMD ,TWL4030_REG_VSTPGA, 0x00},
	{TWL4030_CMD ,TWL4030_REG_VDL_APGA_CTL, 0x1b},	// 20100920 junyeop.kim@lge.com,voice call tuning (HW request) [START_LGE]
	{TWL4030_CMD ,TWL4030_REG_ARXL2_APGA_CTL, 0x2b},	//20100908, junyeop.kim@lge.com, workaround for call playback sound
	{TWL4030_CMD ,TWL4030_REG_ARXR2_APGA_CTL, 0x2b},	//20100908, junyeop.kim@lge.com, workaround for call playback sound
	{TWL4030_CMD ,TWL4030_REG_AVTXL2PGA, 0x00},	//20100912, junyeop.kim@lge.com, voice call tuning
	{TWL4030_CMD ,TWL4030_REG_AVTXR2PGA, 0x00},	//20100912, junyeop.kim@lge.com, voice call tuning
	{TWL4030_CMD ,TWL4030_REG_PREDL_CTL, 0x00},	//20100916 junyeop.kim@lge.com, don't use reg. shut down
	{TWL4030_CMD ,TWL4030_REG_PREDR_CTL, 0x00},	//20100916 junyeop.kim@lge.com, don't use reg. shut down
	{TWL4030_CMD ,TWL4030_REG_HS_SEL, 0x00},
	{TWL4030_CMD ,TWL4030_REG_EAR_CTL, 0x15},
	{TWL4030_END_SEQ,0x00,0x00}
};


static const twl_reg_type twl_headset_call_tab[] =
{
    // wooho.jeong@lge.com 2012.10.05
    // MOD : for 44.1K audio sample rate
	//{TWL4030_CMD ,TWL4030_REG_CODEC_MODE, 0xa2},
    {TWL4030_CMD ,TWL4030_REG_CODEC_MODE, 0x92},         
	{TWL4030_CMD ,TWL4030_REG_OPTION, 0xdf},
	{TWL4030_CMD ,TWL4030_REG_ANAMIC_GAIN, 0x24},	//20110320 seungdae.goh@lge.com
	{TWL4030_CMD ,TWL4030_REG_VOICE_IF, 0x61},
	{TWL4030_CMD ,TWL4030_REG_AUDIO_IF, 0x03},
	{TWL4030_CMD ,TWL4030_REG_ADCMICSEL, 0x00},
	{TWL4030_CMD ,TWL4030_REG_MICBIAS_CTL, 0x02},
	{TWL4030_CMD ,TWL4030_REG_ANAMICL, 0x72},
	{TWL4030_CMD ,TWL4030_REG_AVADC_CTL, 0x08},
	{TWL4030_CMD ,TWL4030_REG_AVDAC_CTL, 0x1c},
	{TWL4030_CMD ,TWL4030_REG_VSTPGA, 0x00},
	{TWL4030_CMD ,TWL4030_REG_VDL_APGA_CTL, 0x33},	//20110320 seungdae.goh@lge.com 0x33 --> 0x2b
	{TWL4030_CMD ,TWL4030_REG_EAR_CTL, 0x30},
	{TWL4030_CMD ,TWL4030_REG_ARXL2_APGA_CTL, 0x2b},	//20100908, junyeop.kim@lge.com, workaround for call playback sound
	{TWL4030_CMD ,TWL4030_REG_ARXR2_APGA_CTL, 0x2b},	//20100908, junyeop.kim@lge.com, workaround for call playback sound
	{TWL4030_CMD ,TWL4030_REG_AVTXL2PGA, 0x05},		//20110320 seungdae.goh@lge.com  5-->4
	{TWL4030_CMD ,TWL4030_REG_AVTXR2PGA, 0x05},		//20100912, junyeop.kim@lge.com, voice call tuning
	{TWL4030_CMD ,TWL4030_REG_PREDL_CTL, 0x00},
	{TWL4030_CMD ,TWL4030_REG_PREDR_CTL, 0x00},
	{TWL4030_CMD ,TWL4030_REG_HS_SEL, 0x29},
	{TWL4030_CMD ,TWL4030_REG_HS_GAIN_SET, 0x05},
	{TWL4030_END_SEQ,0x00,0x00}
};

static const twl_reg_type twl_speaker_call_tab[] =
{
    // wooho.jeong@lge.com 2012.10.05
    // MOD : for 44.1K audio sample rate
	//{TWL4030_CMD ,TWL4030_REG_CODEC_MODE, 0xa2},
    {TWL4030_CMD ,TWL4030_REG_CODEC_MODE, 0x92},          
	{TWL4030_CMD ,TWL4030_REG_OPTION, 0xdf},
	{TWL4030_CMD ,TWL4030_REG_ANAMIC_GAIN, 0x1b/*0x24*/},	// 20100927 junyeop.kim@lge.com,voice call tuning (HW request) [START_LGE]
	{TWL4030_CMD ,TWL4030_REG_VOICE_IF, 0x61},
	{TWL4030_CMD ,TWL4030_REG_AUDIO_IF, 0x03},
	{TWL4030_CMD ,TWL4030_REG_ADCMICSEL, 0x00},
	{TWL4030_CMD ,TWL4030_REG_MICBIAS_CTL, 0x02},
	{TWL4030_CMD ,TWL4030_REG_ANAMICL, 0x71},
	{TWL4030_CMD ,TWL4030_REG_AVADC_CTL, 0x08},	//20100824 junyeop.kim@lge.com, only ADC L enable
	{TWL4030_CMD ,TWL4030_REG_AVDAC_CTL, 0x1c},
	{TWL4030_CMD ,TWL4030_REG_VSTPGA, 0x00},
	{TWL4030_CMD ,TWL4030_REG_VDL_APGA_CTL, 0x33},
	{TWL4030_CMD ,TWL4030_REG_ARXL2_APGA_CTL, 0x1b},	//20110224, mikyoung.chang@lge.com HW request
	{TWL4030_CMD ,TWL4030_REG_ARXR2_APGA_CTL, 0x1b},	//20110224, mikyoung.chang@lge.com HW request
	{TWL4030_CMD ,TWL4030_REG_AVTXL2PGA, 0x00},		//20100912, junyeop.kim@lge.com, voice call tuning
	{TWL4030_CMD ,TWL4030_REG_AVTXR2PGA, 0x00},		//20100912, junyeop.kim@lge.com, voice call tuning
	{TWL4030_CMD ,TWL4030_REG_ARXR2PGA, 0x3f},
	{TWL4030_CMD ,TWL4030_REG_ARXL2PGA, 0x3f},
	{TWL4030_CMD ,TWL4030_REG_EAR_CTL, 0x10},
	{TWL4030_CMD ,TWL4030_REG_HS_SEL, 0x09},
	{TWL4030_CMD ,TWL4030_REG_PREDL_CTL, 0x15},//20110224, mikyoung.chang@lge.com HW request
	{TWL4030_CMD ,TWL4030_REG_PREDR_CTL, 0x15},//20110224, mikyoung.chang@lge.com HW request
	{TWL4030_END_SEQ,0x00,0x00}
};

static const twl_reg_type twl_speaker_call_headset_det_tab[] =
{
    // wooho.jeong@lge.com 2012.10.05
    // MOD : for 44.1K audio sample rate
	//{TWL4030_CMD ,TWL4030_REG_CODEC_MODE, 0xa2},
    {TWL4030_CMD ,TWL4030_REG_CODEC_MODE, 0x92},        
	{TWL4030_CMD ,TWL4030_REG_OPTION, 0xdf},
	{TWL4030_CMD ,TWL4030_REG_ANAMIC_GAIN, 0x24},
	{TWL4030_CMD ,TWL4030_REG_VOICE_IF, 0x61},
	{TWL4030_CMD ,TWL4030_REG_ADCMICSEL, 0x00},
	{TWL4030_CMD ,TWL4030_REG_MICBIAS_CTL, 0x03},	//differ from general spk call tab
	{TWL4030_CMD ,TWL4030_REG_ANAMICL, 0x71},
	{TWL4030_CMD ,TWL4030_REG_AVADC_CTL, 0x08},	//20100824 junyeop.kim@lge.com, only ADC L enable
	{TWL4030_CMD ,TWL4030_REG_AVDAC_CTL, 0x1c},
	{TWL4030_CMD ,TWL4030_REG_VSTPGA, 0x00},
	{TWL4030_CMD ,TWL4030_REG_VDL_APGA_CTL, 0x33},
	{TWL4030_CMD ,TWL4030_REG_ARXL2_APGA_CTL, 0x2b},	//20100908, junyeop.kim@lge.com, workaround for call playback sound
	{TWL4030_CMD ,TWL4030_REG_ARXR2_APGA_CTL, 0x2b},	//20100908, junyeop.kim@lge.com, workaround for call playback sound
	{TWL4030_CMD ,TWL4030_REG_AVTXL2PGA, 0x00},		//20100912, junyeop.kim@lge.com, voice call tuning
	{TWL4030_CMD ,TWL4030_REG_AVTXR2PGA, 0x00},		//20100912, junyeop.kim@lge.com, voice call tuning
	{TWL4030_CMD ,TWL4030_REG_EAR_CTL, 0x10},
	{TWL4030_CMD ,TWL4030_REG_HS_SEL, 0x09},
	{TWL4030_CMD ,TWL4030_REG_PREDL_CTL, 0x25},
	{TWL4030_CMD ,TWL4030_REG_PREDR_CTL, 0x25},
	{TWL4030_END_SEQ,0x00,0x00}
};

static const twl_reg_type twl_headphone_call_tab[] =
{
    // wooho.jeong@lge.com 2012.10.05
    // MOD : for 44.1K audio sample rate
	//{TWL4030_CMD ,TWL4030_REG_CODEC_MODE, 0xa2},
    {TWL4030_CMD ,TWL4030_REG_CODEC_MODE, 0x92},        
	{TWL4030_CMD ,TWL4030_REG_OPTION, 0xdf},
	{TWL4030_CMD ,TWL4030_REG_ANAMIC_GAIN, 0x1b},
	{TWL4030_CMD ,TWL4030_REG_VOICE_IF, 0x61},
	{TWL4030_CMD ,TWL4030_REG_ADCMICSEL, 0x00},
	{TWL4030_CMD ,TWL4030_REG_MICBIAS_CTL, 0x03},
	{TWL4030_CMD ,TWL4030_REG_ANAMICL, 0x71},
	{TWL4030_CMD ,TWL4030_REG_AVADC_CTL, 0x08},	//20100824 junyeop.kim@lge.com, only ADC L enable
	{TWL4030_CMD ,TWL4030_REG_AVDAC_CTL, 0x1c},
	{TWL4030_CMD ,TWL4030_REG_VSTPGA, 0x00},
	{TWL4030_CMD ,TWL4030_REG_VDL_APGA_CTL, 0x33},	// 20110126  mikyoung.chang@lge.com
	{TWL4030_CMD ,TWL4030_REG_ARXL2_APGA_CTL, 0x2b},	//20100908, junyeop.kim@lge.com, workaround for call playback sound
	{TWL4030_CMD ,TWL4030_REG_ARXR2_APGA_CTL, 0x2b},	//20100908, junyeop.kim@lge.com, workaround for call playback sound
	{TWL4030_CMD ,TWL4030_REG_AVTXL2PGA, 0x05},	//20100912, junyeop.kim@lge.com, voice call tuning
	{TWL4030_CMD ,TWL4030_REG_AVTXR2PGA, 0x00},	//20100912, junyeop.kim@lge.com, voice call tuning
	{TWL4030_CMD ,TWL4030_REG_EAR_CTL, 0x30},	//20100912, junyeop.kim@lge.com, workaround for call playback sound
	{TWL4030_CMD ,TWL4030_REG_PREDL_CTL, 0x00},	//20100916 junyeop.kim@lge.com, don't use reg. shut down
	{TWL4030_CMD ,TWL4030_REG_PREDR_CTL, 0x00},	//20100916 junyeop.kim@lge.com, don't use reg. shut down
	{TWL4030_CMD ,TWL4030_REG_HS_SEL, 0x29},
	{TWL4030_CMD ,TWL4030_REG_HS_GAIN_SET, 0x05},
	{TWL4030_END_SEQ,0x00,0x00}
};
#endif /* LGE_CHANGE_E [iggikim@lge.com]*/

/* 20110106 jiwon.seo@lge.com : speaker phone on pop nosie [START] */
static const twl_reg_type twl_mic1bias_tab[] =
{//20110206 jisun.kwon : due to pop noise when incoming call is received
	{TWL4030_CMD ,TWL4030_REG_MICBIAS_CTL, 0x01},      //original 0x7x
	{TWL4030_CMD ,TWL4030_REG_VOICE_IF, 0x00},
	{TWL4030_END_SEQ,0x00,0x00}
};
static const twl_reg_type twl_mic2bias_tab[] =
{//20110206 jisun.kwon : due to pop noise when incoming call is received
	{TWL4030_CMD ,TWL4030_REG_MICBIAS_CTL, 0x02},      //original 0x7x
	{TWL4030_CMD ,TWL4030_REG_VOICE_IF, 0x00},      //original 0x7x
	{TWL4030_END_SEQ,0x00,0x00}
};
/* 20110106 jiwon.seo@lge.com : speaker phone on pop nosie  [END] */
// [bsnoh@ubiquix][20110528] : Added code from froyo Justin [End]


// 20100426 junyeop.kim@lge.com Add the mic mute [START_LGE]
static const twl_reg_type twl_mic_off_tab[] =
{
	{TWL4030_CMD ,TWL4030_REG_VOICE_IF, 0x61},	// original 0x61
	{TWL4030_CMD ,TWL4030_REG_ANAMICL, 0x70}, 	//original 0x7x
	{TWL4030_END_SEQ,0x00,0x00}
};

static const twl_reg_type twl_headset_mic_tab[] =
{
	{TWL4030_CMD ,TWL4030_REG_VOICE_IF, 0x61},
	{TWL4030_CMD ,TWL4030_REG_ANAMICL, 0x72},
	{TWL4030_CMD ,TWL4030_REG_HS_GAIN_SET, 0x05},
	{TWL4030_END_SEQ,0x00,0x00}
};

static const twl_reg_type twl_speaker_mic_tab[] =
{
	{TWL4030_CMD ,TWL4030_REG_VOICE_IF, 0x61},
	{TWL4030_CMD ,TWL4030_REG_ANAMICL, 0x71},
	{TWL4030_END_SEQ,0x00,0x00}
};

static const twl_reg_type twl_receiver_mic_tab[] =
{
	{TWL4030_CMD ,TWL4030_REG_VOICE_IF, 0x61},
	{TWL4030_CMD ,TWL4030_REG_ANAMICL, 0x71},
	{TWL4030_END_SEQ,0x00,0x00}
};
// 20100426 junyeop.kim@lge.com Add the mic mute [END_LGE]

// 20100521 junyeop.kim@lge.com call recording path [START_LGE]
static const twl_reg_type twl_headset_callrec_tab[] =
{
	{TWL4030_CMD ,TWL4030_REG_OPTION, 0xd3},
	{TWL4030_END_SEQ,0x00,0x00}
};

static const twl_reg_type twl_builtin_callrec_tab[] =
{
	{TWL4030_CMD ,TWL4030_REG_OPTION, 0xd3},
	{TWL4030_END_SEQ,0x00,0x00}
};
// 20100521 junyeop.kim@lge.com call recording path [END_LGE]


/* codec private data */
struct twl4030_priv {
	struct mutex mutex;
	unsigned int extClock;
	struct snd_soc_codec codec;

	unsigned int codec_powered;

	/* reference counts of AIF/APLL users */
	unsigned int apll_enabled;

	struct snd_pcm_substream *master_substream;
	struct snd_pcm_substream *slave_substream;

	unsigned int configured;
	unsigned int rate;
	unsigned int sample_bits;
	unsigned int channels;

	unsigned int sysclk;

	/* Output (with associated amp) states */
	u8 hsl_enabled, hsr_enabled;
	u8 earpiece_enabled;
	u8 predrivel_enabled, predriver_enabled;
	u8 carkitl_enabled, carkitr_enabled;

	/* Delay needed after enabling the digimic interface */
	unsigned int digimic_delay;
#if defined(CONFIG_PRODUCT_LGE_LU6800) || defined(CONFIG_PRODUCT_LGE_KU5900) || defined (CONFIG_PRODUCT_LGE_P970) // 20100618 junyeop.kim@lge.com, call path [START_LGE}
	unsigned int is_calling;
#endif
};

//#if defined(CONFIG_PRODUCT_LGE_KU5900)||defined(CONFIG_PRODUCT_LGE_LU6800)	
/*[LGE_CHANGE] 20120908 pyocool.cho@lge.com "for p970" */
#if defined(CONFIG_PRODUCT_LGE_KU5900)||defined(CONFIG_PRODUCT_LGE_LU6800) || defined (CONFIG_PRODUCT_LGE_P970)	

int get_twl4030_apll_state(void)
{
    struct snd_soc_codec* codec = snd_soc_get_codec("twl4030-codec");
    if(codec == NULL)
    {
        printk(KERN_ERR "snd_soc_get_codec twl4030-codec :: Null \n");
        return -1;
    }
    struct twl4030_priv *twl4030 = snd_soc_codec_get_drvdata(codec);

    if(twl4030->apll_enabled)
        return 1;
     else
        return 0;
}

EXPORT_SYMBOL_GPL(get_twl4030_apll_state);

//#if defined(CONFIG_PRODUCT_LGE_KU5900)
/* [LGE_CHANGE] 20120908 pyocool.cho@lge.com "for p970" */
#if defined(CONFIG_PRODUCT_LGE_KU5900) || defined(CONFIG_PRODUCT_LGE_P970)
wm9093_mode_enum cur_ext_amp_mode = OFF_MODE;
void set_ext_amp_mode(int mode)
{
    cur_ext_amp_mode = (wm9093_mode_enum)mode;
}

int get_twl4030_headset_spk_codec_status(void)
{
    struct snd_soc_codec* codec = snd_soc_get_codec("twl4030-codec");
    if(codec == NULL)
    {
        printk(KERN_ERR "snd_soc_get_codec twl4030-codec :: Null \n");
        return -1;
    }
    struct twl4030_priv *twl4030 = snd_soc_codec_get_drvdata(codec);

     if( twl4030->hsr_enabled || twl4030->hsl_enabled                           /* CHECK HEADSET */
        || twl4030->predriver_enabled || twl4030->predrivel_enabled /* CHECK SPEKER   */
       )
        return 1;
    else
        return 0;
}

EXPORT_SYMBOL_GPL(set_ext_amp_mode);
EXPORT_SYMBOL_GPL(get_twl4030_headset_spk_codec_status);
#endif
#endif

#ifdef DEBUG
static char * reg_log_string(unsigned int nTemp_enum);
static char *voice_mode_log_str(unsigned int nTemp_enum);

static char * reg_log_string(unsigned int nTemp_enum)
{
	switch(nTemp_enum)
	{
                case TWL4030_REG_CODEC_MODE:    return "TWL4030_REG_CODEC_MODE";
                case TWL4030_REG_OPTION:    return "TWL4030_REG_OPTION";
                case TWL4030_REG_UNKNOWN:    return "TWL4030_REG_UNKNOWN";
                case TWL4030_REG_MICBIAS_CTL:    return "TWL4030_REG_MICBIAS_CTL";
                case TWL4030_REG_ANAMICL:    return "TWL4030_REG_ANAMICL";
                case TWL4030_REG_ANAMICR:    return "TWL4030_REG_ANAMICR";
                case TWL4030_REG_AVADC_CTL:    return "TWL4030_REG_AVADC_CTL";
                case TWL4030_REG_ADCMICSEL:    return "TWL4030_REG_ADCMICSEL";
                case TWL4030_REG_DIGMIXING:    return "TWL4030_REG_DIGMIXING";
                case TWL4030_REG_ATXL1PGA:    return "TWL4030_REG_ATXL1PGA";
                case TWL4030_REG_ATXR1PGA:    return "TWL4030_REG_ATXR1PGA";
                case TWL4030_REG_AVTXL2PGA:    return "TWL4030_REG_AVTXL2PGA";
                case TWL4030_REG_AVTXR2PGA:    return "TWL4030_REG_AVTXR2PGA";
                case TWL4030_REG_AUDIO_IF:    return "TWL4030_REG_AUDIO_IF";
                case TWL4030_REG_VOICE_IF:    return "TWL4030_REG_VOICE_IF";
                case TWL4030_REG_ARXR1PGA:    return "TWL4030_REG_ARXR1PGA";
                case TWL4030_REG_ARXL1PGA:    return "TWL4030_REG_ARXL1PGA";
                case TWL4030_REG_ARXR2PGA:    return "TWL4030_REG_ARXR2PGA";
                case TWL4030_REG_ARXL2PGA:    return "TWL4030_REG_ARXL2PGA";
                case TWL4030_REG_VRXPGA:    return "TWL4030_REG_VRXPGA";
                case TWL4030_REG_VSTPGA:    return "TWL4030_REG_VSTPGA";
                case TWL4030_REG_VRX2ARXPGA:    return "TWL4030_REG_VRX2ARXPGA";
                case TWL4030_REG_AVDAC_CTL:    return "TWL4030_REG_AVDAC_CTL";
                case TWL4030_REG_ARX2VTXPGA:    return "TWL4030_REG_ARX2VTXPGA";
                case TWL4030_REG_ARXL1_APGA_CTL:    return "TWL4030_REG_ARXL1_APGA_CTL";
                case TWL4030_REG_ARXR1_APGA_CTL:    return "TWL4030_REG_ARXR1_APGA_CTL";
                case TWL4030_REG_ARXL2_APGA_CTL:    return "TWL4030_REG_ARXL2_APGA_CTL";
                case TWL4030_REG_ARXR2_APGA_CTL:    return "TWL4030_REG_ARXR2_APGA_CTL";
                case TWL4030_REG_ATX2ARXPGA:    return "TWL4030_REG_ATX2ARXPGA";
                case TWL4030_REG_BT_IF:    return "TWL4030_REG_BT_IF";
                case TWL4030_REG_BTPGA:    return "TWL4030_REG_BTPGA";
                case TWL4030_REG_BTSTPGA:    return "TWL4030_REG_BTSTPGA";
                case TWL4030_REG_EAR_CTL:    return "TWL4030_REG_EAR_CTL";
                case TWL4030_REG_HS_SEL:    return "TWL4030_REG_HS_SEL";
                case TWL4030_REG_HS_GAIN_SET:    return "TWL4030_REG_HS_GAIN_SET";
                case TWL4030_REG_HS_POPN_SET:    return "TWL4030_REG_HS_POPN_SET";
                case TWL4030_REG_PREDL_CTL:    return "TWL4030_REG_PREDL_CTL";
                case TWL4030_REG_PREDR_CTL:    return "TWL4030_REG_PREDR_CTL";
                case TWL4030_REG_PRECKL_CTL:    return "TWL4030_REG_PRECKL_CTL";
                case TWL4030_REG_PRECKR_CTL:    return "TWL4030_REG_PRECKR_CTL";
                case TWL4030_REG_HFL_CTL:    return "TWL4030_REG_HFL_CTL";
                case TWL4030_REG_HFR_CTL:    return "TWL4030_REG_HFR_CTL";
                case TWL4030_REG_ALC_CTL:    return "TWL4030_REG_ALC_CTL";
                case TWL4030_REG_ALC_SET1:    return "TWL4030_REG_ALC_SET1";
                case TWL4030_REG_ALC_SET2:    return "TWL4030_REG_ALC_SET2";
                case TWL4030_REG_BOOST_CTL:    return "TWL4030_REG_BOOST_CTL";
                case TWL4030_REG_SOFTVOL_CTL:    return "TWL4030_REG_SOFTVOL_CTL";
                case TWL4030_REG_DTMF_FREQSEL:    return "TWL4030_REG_DTMF_FREQSEL";
                case TWL4030_REG_DTMF_TONEXT1H:    return "TWL4030_REG_DTMF_TONEXT1H";
                case TWL4030_REG_DTMF_TONEXT1L:    return "TWL4030_REG_DTMF_TONEXT1L";
                case TWL4030_REG_DTMF_TONEXT2H:    return "TWL4030_REG_DTMF_TONEXT2H";
                case TWL4030_REG_DTMF_TONEXT2L:    return "TWL4030_REG_DTMF_TONEXT2L";
                case TWL4030_REG_DTMF_TONOFF:    return "TWL4030_REG_DTMF_TONOFF";
                case TWL4030_REG_DTMF_WANONOFF:    return "TWL4030_REG_DTMF_WANONOFF";
                case TWL4030_REG_I2S_RX_SCRAMBLE_H:    return "TWL4030_REG_I2S_RX_SCRAMBLE_H";
                case TWL4030_REG_I2S_RX_SCRAMBLE_M:    return "TWL4030_REG_I2S_RX_SCRAMBLE_M";
                case TWL4030_REG_I2S_RX_SCRAMBLE_L:    return "TWL4030_REG_I2S_RX_SCRAMBLE_L";
                case TWL4030_REG_APLL_CTL:    return "TWL4030_REG_APLL_CTL";
                case TWL4030_REG_DTMF_CTL:    return "TWL4030_REG_DTMF_CTL";
                case TWL4030_REG_DTMF_PGA_CTL2:    return "TWL4030_REG_DTMF_PGA_CTL2";
                case TWL4030_REG_DTMF_PGA_CTL1:    return "TWL4030_REG_DTMF_PGA_CTL1";
                case TWL4030_REG_MISC_SET_1:    return "TWL4030_REG_MISC_SET_1";
                case TWL4030_REG_PCMBTMUX:    return "TWL4030_REG_PCMBTMUX";
                case TWL4030_REG_RX_PATH_SEL:    return "TWL4030_REG_RX_PATH_SEL";
                case TWL4030_REG_VDL_APGA_CTL:    return "TWL4030_REG_VDL_APGA_CTL";
                case TWL4030_REG_VIBRA_CTL:    return "TWL4030_REG_VIBRA_CTL";
                case TWL4030_REG_VIBRA_SET:    return "TWL4030_REG_VIBRA_SET";
                case TWL4030_REG_VIBRA_PWM_SET:    return "TWL4030_REG_VIBRA_PWM_SET";
                case TWL4030_REG_ANAMIC_GAIN:    return "TWL4030_REG_ANAMIC_GAIN";
                case TWL4030_REG_MISC_SET_2:    return "TWL4030_REG_MISC_SET_2";
                case TWL4030_REG_SW_SHADOW :   return "TWL4030_REG_SW_SHADOW";
                default:    return "Error Don't know REG";
	}
}

static char *voice_mode_log_str(unsigned int nTemp_enum)
{
	switch(nTemp_enum)
	{
		case TWL4030_AUDIO_MODE:    return "TWL4030_AUDIO_MODE";
		case TWL4030_HEADSET_CALL_MODE:    return "TWL4030_HEADSET_CALL_MODE";
		case TWL4030_SPEAKER_CALL_MODE:    return "TWL4030_SPEAKER_CALL_MODE";
		case TWL4030_RECEIVER_CALL_MODE:    return "TWL4030_RECEIVER_CALL_MODE";
		case TWL4030_HEADPHONE_CALL_MODE:    return "TWL4030_HEADPHONE_CALL_MODE";
		case TWL4030_VT_BT_CALL_MODE:    return "TWL4030_VT_BT_CALL_MODE";
		case TWL4030_VT_HEADSET_CALL_MODE:    return "TWL4030_VT_HEADSET_CALL_MODE";
		case TWL4030_VT_SPEAKER_CALL_MODE:    return "TWL4030_VT_SPEAKER_CALL_MODE";
		case TWL4030_VT_RECEIVER_CALL_MODE:    return "TWL4030_VT_RECEIVER_CALL_MODE";
		case TWL4030_VT_HEADPHONE_CALL_MODE:    return "TWL4030_VT_HEADPHONE_CALL_MODE";
		case TWL4030_VT_CALL_END_MODE:    return "TWL4030_VT_CALL_END_MODE";
		case TWL4030_VOIP_CALL_END_MODE:    return "TWL4030_VOIP_CALL_END_MODE";
		default:    return "Error Voice_mode";
	}
}
#endif

/*
 * read twl4030 register cache
 */
static inline unsigned int twl4030_read_reg_cache(struct snd_soc_codec *codec,
		unsigned int reg)
{
	u8 *cache = codec->reg_cache;

	if (reg >= TWL4030_CACHEREGNUM)
		return -EIO;

	return cache[reg];
}

/*
 * write twl4030 register cache
 */
static inline void twl4030_write_reg_cache(struct snd_soc_codec *codec,
		u8 reg, u8 value)
{
	u8 *cache = codec->reg_cache;

	if (reg >= TWL4030_CACHEREGNUM)
		return;
	cache[reg] = value;
}

/*
 * write to the twl4030 register space
 */
static int twl4030_write(struct snd_soc_codec *codec,
		unsigned int reg, unsigned int value)
{
	struct twl4030_priv *twl4030 = snd_soc_codec_get_drvdata(codec);
	int write_to_reg = 0;

/* LGE_CHANGE_S :  2012-09-26, gt.kim@lge.com, Desc: Headset Music Play-> VT Call->Call end Issue..    */
    if( ( reg == TWL4030_REG_ARXL2_APGA_CTL || reg == TWL4030_REG_ARXR2_APGA_CTL )
            && ( ! ( value & 0x0001 ) )
            && ( twl4030->is_calling) )
        {
            int twl_mode = voice_get_curmode();
            if(twl_mode >= TWL4030_VT_BT_CALL_MODE && twl_mode <= TWL4030_VT_HEADPHONE_CALL_MODE )
            {
//                WARN_ON(1);
#ifdef DEBUG
                printk("[%s:%d] func:: Return !!  reg_log_string(%s), VALUE(0x%x) twl_mode[%d]\n", __func__, __LINE__, reg_log_string(reg), value, twl_mode);
#endif
                return 0;
            }
        }
/* LGE_CHANGE_E :  2012-09-26, gt.kim@lge.com,     */
	// prime@sdcmicro.com LGE specific codec control [START]
#if defined(CONFIG_PRODUCT_LGE_LU6800) || defined(CONFIG_PRODUCT_LGE_KU5900) || defined (CONFIG_PRODUCT_LGE_P970)  /* 20100618 junyeop.kim@lge.com, call path */
#if defined(CONFIG_PRODUCT_LGE_HUB)    // 20100515 junyeop.kim@lge.com,voice call tuning (HW request) [START_LGE]
	if((twl4030->is_calling) && (reg != TWL4030_REG_VRXPGA)) //except volume reg
		return 0;
#else
	if((twl4030->is_calling) && (reg != TWL4030_REG_VDL_APGA_CTL) && (reg != TWL4030_REG_VRXPGA)
		&& (reg != TWL4030_REG_AVTXL2PGA) && (reg != TWL4030_REG_AVTXR2PGA) && (reg != TWL4030_REG_ARXR2PGA) && (reg != TWL4030_REG_ARXL2PGA) &&
		(reg != TWL4030_REG_PREDL_CTL) && (reg !=TWL4030_REG_PREDR_CTL) && (reg !=TWL4030_REG_ARXL2_APGA_CTL) && (reg !=TWL4030_REG_ARXR2_APGA_CTL))
	{
#ifdef DEBUG
		printk("[Twl4030] func:: twl4030_write  Return !!     reg_log_string(%s), VALUE(0x%x)\n", reg_log_string(reg), value);
#endif
		return 0;
	}
#endif    // 20100515 junyeop.kim@lge.com,voice call tuning (HW request) [END_LGE]

	if((twl4030->is_calling) && (reg == TWL4030_REG_VDL_APGA_CTL))	//junyeop.kim@lge.com, temp code for volume reg, why?
		value |= 0x03;

	// 20100525 junyeop.kim@lge.com, external mic bias always enable when headset detect [START_LGE]
	if((reg == TWL4030_REG_MICBIAS_CTL) && get_headset_type() == HUB_HEADSET)	//initial recording issue(external mic bias removal when initail record)
		value |=0x02;

#if defined(CONFIG_PRODUCT_LGE_HUB)
	if((reg == TWL4030_REG_MICBIAS_CTL) && dmb_status == 1)	//DMB ant detect case
		value |=0x02;
#endif

#if 0
	if(reg == 0x04 && get_headset_type() == HUB_NONE)
		value = 0x03;
#endif
	if(reg == TWL4030_REG_AVADC_CTL)	//junyeop.kim@lge.com, remove the initial adc setting noise
		value = 0x0a;

#endif
	// prime@sdcmicro.com LGE specific codec control [END]


#if 0
	twl4030_write_reg_cache(codec, reg, value);
	if (likely(reg < TWL4030_REG_SW_SHADOW)) {
		/* Decide if the given register can be written */
		switch (reg) {
			case TWL4030_REG_EAR_CTL:
				if (twl4030->earpiece_enabled)
					write_to_reg = 1;
				break;
			case TWL4030_REG_PREDL_CTL:
				if (twl4030->predrivel_enabled)
					write_to_reg = 1;
				break;
			case TWL4030_REG_PREDR_CTL:
				if (twl4030->predriver_enabled)
					write_to_reg = 1;
				break;
			case TWL4030_REG_PRECKL_CTL:
				if (twl4030->carkitl_enabled)
					write_to_reg = 1;
				break;
			case TWL4030_REG_PRECKR_CTL:
				if (twl4030->carkitr_enabled)
					write_to_reg = 1;
				break;
			case TWL4030_REG_HS_GAIN_SET:
				if (twl4030->hsl_enabled || twl4030->hsr_enabled)
					write_to_reg = 1;
				break;
			default:
				/* All other register can be written */
				write_to_reg = 1;
				break;
		}
		if (write_to_reg)
			return twl_i2c_write_u8(TWL4030_MODULE_AUDIO_VOICE,
					value, reg);
	}
	return 0;
#else
#ifdef DEBUG
	printk("[Twl4030] twl4030_write : reg_log_string(%s), VALUE(0x%x)\n", reg_log_string(reg), value);
#endif
	twl4030_write_reg_cache(codec, reg, value);
	if (likely(reg < TWL4030_REG_SW_SHADOW))
		return twl_i2c_write_u8(TWL4030_MODULE_AUDIO_VOICE, value,
				reg);
	else
		return 0;
#endif
}

static inline void twl4030_wait_ms(int time)
{
	if (time < 60) {
		time *= 1000;
		usleep_range(time, time + 500);
	} else {
		msleep(time);
	}
}

static void twl4030_codec_enable(struct snd_soc_codec *codec, int enable)
{
	struct twl4030_priv *twl4030 = snd_soc_codec_get_drvdata(codec);
	int mode;

	DBG("\n");

	if (enable == twl4030->codec_powered)
		return;

	if (enable)
		mode = twl4030_codec_enable_resource(TWL4030_CODEC_RES_POWER);
	else
		mode = twl4030_codec_disable_resource(TWL4030_CODEC_RES_POWER);

	if (mode >= 0) {
		twl4030_write_reg_cache(codec, TWL4030_REG_CODEC_MODE, mode);
		twl4030->codec_powered = enable;
	}

	/* REVISIT: this delay is present in TI sample drivers */
	/* but there seems to be no TRM requirement for it     */
	udelay(10);
}

static inline void twl4030_check_defaults(struct snd_soc_codec *codec)
{
	int i, difference = 0;
	u8 val;


	DBG("\n");

	dev_dbg(codec->dev, "Checking TWL audio default configuration\n");
	for (i = 1; i <= TWL4030_REG_MISC_SET_2; i++) {
		twl_i2c_read_u8(TWL4030_MODULE_AUDIO_VOICE, &val, i);
		if (val != twl4030_reg[i]) {
			difference++;
			dev_dbg(codec->dev,
					"Reg 0x%02x: chip: 0x%02x driver: 0x%02x\n",
					i, val, twl4030_reg[i]);
		}
	}
	dev_dbg(codec->dev, "Found %d non-matching registers. %s\n",
			difference, difference ? "Not OK" : "OK");
}

static inline void twl4030_reset_registers(struct snd_soc_codec *codec)
{
	int i;
	DBG("\n");

	/* set all audio section registers to reasonable defaults */
	for (i = TWL4030_REG_OPTION; i <= TWL4030_REG_MISC_SET_2; i++)
		if (i != TWL4030_REG_APLL_CTL)
			twl4030_write(codec, i, twl4030_reg[i]);

}

static void twl4030_init_chip(struct snd_soc_codec *codec)
{
	struct twl4030_codec_audio_data *pdata = dev_get_platdata(codec->dev);
	struct twl4030_priv *twl4030 = snd_soc_codec_get_drvdata(codec);
	u8 reg, byte;
	int i = 0;
	DBG("\n");

	/* Check defaults, if instructed before anything else */
	if (pdata && pdata->check_defaults)
		twl4030_check_defaults(codec);

	/* Reset registers, if no setup data or if instructed to do so */
	if (!pdata || (pdata && pdata->reset_registers))
		twl4030_reset_registers(codec);

	/* Refresh APLL_CTL register from HW */
	twl_i2c_read_u8(TWL4030_MODULE_AUDIO_VOICE, &byte,
			TWL4030_REG_APLL_CTL);
	twl4030_write_reg_cache(codec, TWL4030_REG_APLL_CTL, byte);

	/* anti-pop when changing analog gain */
	reg = twl4030_read_reg_cache(codec, TWL4030_REG_MISC_SET_1);
	twl4030_write(codec, TWL4030_REG_MISC_SET_1,
			reg | TWL4030_SMOOTH_ANAVOL_EN);

	twl4030_write(codec, TWL4030_REG_OPTION,
			TWL4030_ATXL1_EN | TWL4030_ATXR1_EN |
			TWL4030_ARXL2_EN | TWL4030_ARXR2_EN);

	/* REG_ARXR2_APGA_CTL reset according to the TRM: 0dB, DA_EN */
	twl4030_write(codec, TWL4030_REG_ARXR2_APGA_CTL, 0x2b); // // 20121030 bs.lim@lge.com 32 -> 2b

	/* Machine dependent setup */
	if (!pdata)
		return;

	twl4030->digimic_delay = pdata->digimic_delay;

	reg = twl4030_read_reg_cache(codec, TWL4030_REG_HS_POPN_SET);
	reg &= ~TWL4030_RAMP_DELAY;
	reg |= (pdata->ramp_delay_value << 2);
	twl4030_write_reg_cache(codec, TWL4030_REG_HS_POPN_SET, reg);

	/* initiate offset cancellation */
	twl4030_codec_enable(codec, 1);

	reg = twl4030_read_reg_cache(codec, TWL4030_REG_ANAMICL);
	reg &= ~TWL4030_OFFSET_CNCL_SEL;
	reg |= pdata->offset_cncl_path;
	twl4030_write(codec, TWL4030_REG_ANAMICL,
			reg | TWL4030_CNCL_OFFSET_START);

	/*
	 * Wait for offset cancellation to complete.
	 * Since this takes a while, do not slam the i2c.
	 * Start polling the status after ~20ms.
	 */
	msleep(20);
	do {
		usleep_range(1000, 2000);
		twl_i2c_read_u8(TWL4030_MODULE_AUDIO_VOICE, &byte,
				TWL4030_REG_ANAMICL);
	} while ((i++ < 100) &&
			((byte & TWL4030_CNCL_OFFSET_START) ==
			 TWL4030_CNCL_OFFSET_START));

	/* Make sure that the reg_cache has the same value as the HW */
	twl4030_write_reg_cache(codec, TWL4030_REG_ANAMICL, byte);

	twl4030_codec_enable(codec, 0);
}

static void twl4030_apll_enable(struct snd_soc_codec *codec, int enable)
{
	struct twl4030_priv *twl4030 = snd_soc_codec_get_drvdata(codec);
	int status = -1;
	DBG("\n");

	if (enable) {
		twl4030->apll_enabled++;
		if (twl4030->apll_enabled == 1)
			status = twl4030_codec_enable_resource(
					TWL4030_CODEC_RES_APLL);
	} else {
		twl4030->apll_enabled--;
		if (!twl4030->apll_enabled)
			status = twl4030_codec_disable_resource(
					TWL4030_CODEC_RES_APLL);
	}

	if (status >= 0)
		twl4030_write_reg_cache(codec, TWL4030_REG_APLL_CTL, status);
}

/* Earpiece */
static const struct snd_kcontrol_new twl4030_dapm_earpiece_controls[] = {
	SOC_DAPM_SINGLE("Voice", TWL4030_REG_EAR_CTL, 0, 1, 0),
	SOC_DAPM_SINGLE("AudioL1", TWL4030_REG_EAR_CTL, 1, 1, 0),
	SOC_DAPM_SINGLE("AudioL2", TWL4030_REG_EAR_CTL, 2, 1, 0),
	SOC_DAPM_SINGLE("AudioR1", TWL4030_REG_EAR_CTL, 3, 1, 0),
};

/* PreDrive Left */
static const struct snd_kcontrol_new twl4030_dapm_predrivel_controls[] = {
	SOC_DAPM_SINGLE("Voice", TWL4030_REG_PREDL_CTL, 0, 1, 0),
	SOC_DAPM_SINGLE("AudioL1", TWL4030_REG_PREDL_CTL, 1, 1, 0),
	SOC_DAPM_SINGLE("AudioL2", TWL4030_REG_PREDL_CTL, 2, 1, 0),
	SOC_DAPM_SINGLE("AudioR2", TWL4030_REG_PREDL_CTL, 3, 1, 0),
};

/* PreDrive Right */
static const struct snd_kcontrol_new twl4030_dapm_predriver_controls[] = {
	SOC_DAPM_SINGLE("Voice", TWL4030_REG_PREDR_CTL, 0, 1, 0),
	SOC_DAPM_SINGLE("AudioR1", TWL4030_REG_PREDR_CTL, 1, 1, 0),
	SOC_DAPM_SINGLE("AudioR2", TWL4030_REG_PREDR_CTL, 2, 1, 0),
	SOC_DAPM_SINGLE("AudioL2", TWL4030_REG_PREDR_CTL, 3, 1, 0),
};

/* Headset Left */
static const struct snd_kcontrol_new twl4030_dapm_hsol_controls[] = {
	SOC_DAPM_SINGLE("Voice", TWL4030_REG_HS_SEL, 0, 1, 0),
	SOC_DAPM_SINGLE("AudioL1", TWL4030_REG_HS_SEL, 1, 1, 0),
	SOC_DAPM_SINGLE("AudioL2", TWL4030_REG_HS_SEL, 2, 1, 0),
};

/* Headset Right */
static const struct snd_kcontrol_new twl4030_dapm_hsor_controls[] = {
	SOC_DAPM_SINGLE("Voice", TWL4030_REG_HS_SEL, 3, 1, 0),
	SOC_DAPM_SINGLE("AudioR1", TWL4030_REG_HS_SEL, 4, 1, 0),
	SOC_DAPM_SINGLE("AudioR2", TWL4030_REG_HS_SEL, 5, 1, 0),
};

/* Carkit Left */
static const struct snd_kcontrol_new twl4030_dapm_carkitl_controls[] = {
	SOC_DAPM_SINGLE("Voice", TWL4030_REG_PRECKL_CTL, 0, 1, 0),
	SOC_DAPM_SINGLE("AudioL1", TWL4030_REG_PRECKL_CTL, 1, 1, 0),
	SOC_DAPM_SINGLE("AudioL2", TWL4030_REG_PRECKL_CTL, 2, 1, 0),
};

/* Carkit Right */
static const struct snd_kcontrol_new twl4030_dapm_carkitr_controls[] = {
	SOC_DAPM_SINGLE("Voice", TWL4030_REG_PRECKR_CTL, 0, 1, 0),
	SOC_DAPM_SINGLE("AudioR1", TWL4030_REG_PRECKR_CTL, 1, 1, 0),
	SOC_DAPM_SINGLE("AudioR2", TWL4030_REG_PRECKR_CTL, 2, 1, 0),
};

/* Handsfree Left */
static const char *twl4030_handsfreel_texts[] =
{"Voice", "AudioL1", "AudioL2", "AudioR2"};

static const struct soc_enum twl4030_handsfreel_enum =
SOC_ENUM_SINGLE(TWL4030_REG_HFL_CTL, 0,
		ARRAY_SIZE(twl4030_handsfreel_texts),
		twl4030_handsfreel_texts);

static const struct snd_kcontrol_new twl4030_dapm_handsfreel_control =
SOC_DAPM_ENUM("Route", twl4030_handsfreel_enum);

/* Handsfree Left virtual mute */
static const struct snd_kcontrol_new twl4030_dapm_handsfreelmute_control =
SOC_DAPM_SINGLE("Switch", TWL4030_REG_SW_SHADOW, 0, 1, 0);

/* Handsfree Right */
static const char *twl4030_handsfreer_texts[] =
{"Voice", "AudioR1", "AudioR2", "AudioL2"};

static const struct soc_enum twl4030_handsfreer_enum =
SOC_ENUM_SINGLE(TWL4030_REG_HFR_CTL, 0,
		ARRAY_SIZE(twl4030_handsfreer_texts),
		twl4030_handsfreer_texts);

static const struct snd_kcontrol_new twl4030_dapm_handsfreer_control =
SOC_DAPM_ENUM("Route", twl4030_handsfreer_enum);

/* Handsfree Right virtual mute */
static const struct snd_kcontrol_new twl4030_dapm_handsfreermute_control =
SOC_DAPM_SINGLE("Switch", TWL4030_REG_SW_SHADOW, 1, 1, 0);

/* Vibra */
/* Vibra audio path selection */
static const char *twl4030_vibra_texts[] =
{"AudioL1", "AudioR1", "AudioL2", "AudioR2"};

static const struct soc_enum twl4030_vibra_enum =
SOC_ENUM_SINGLE(TWL4030_REG_VIBRA_CTL, 2,
		ARRAY_SIZE(twl4030_vibra_texts),
		twl4030_vibra_texts);

static const struct snd_kcontrol_new twl4030_dapm_vibra_control =
SOC_DAPM_ENUM("Route", twl4030_vibra_enum);

/* Vibra path selection: local vibrator (PWM) or audio driven */
static const char *twl4030_vibrapath_texts[] =
{"Local vibrator", "Audio"};

static const struct soc_enum twl4030_vibrapath_enum =
SOC_ENUM_SINGLE(TWL4030_REG_VIBRA_CTL, 4,
		ARRAY_SIZE(twl4030_vibrapath_texts),
		twl4030_vibrapath_texts);

static const struct snd_kcontrol_new twl4030_dapm_vibrapath_control =
SOC_DAPM_ENUM("Route", twl4030_vibrapath_enum);

/* Left analog microphone selection */
static const struct snd_kcontrol_new twl4030_dapm_analoglmic_controls[] = {
	SOC_DAPM_SINGLE("Main Mic Capture Switch",
			TWL4030_REG_ANAMICL, 0, 1, 0),
	SOC_DAPM_SINGLE("Headset Mic Capture Switch",
			TWL4030_REG_ANAMICL, 1, 1, 0),
	SOC_DAPM_SINGLE("AUXL Capture Switch",
			TWL4030_REG_ANAMICL, 2, 1, 0),
	SOC_DAPM_SINGLE("Carkit Mic Capture Switch",
			TWL4030_REG_ANAMICL, 3, 1, 0),
};

/* Right analog microphone selection */
static const struct snd_kcontrol_new twl4030_dapm_analogrmic_controls[] = {
	SOC_DAPM_SINGLE("Sub Mic Capture Switch", TWL4030_REG_ANAMICR, 0, 1, 0),
	SOC_DAPM_SINGLE("AUXR Capture Switch", TWL4030_REG_ANAMICR, 2, 1, 0),
};

/* TX1 L/R Analog/Digital microphone selection */
static const char *twl4030_micpathtx1_texts[] =
{"Analog", "Digimic0"};

static const struct soc_enum twl4030_micpathtx1_enum =
SOC_ENUM_SINGLE(TWL4030_REG_ADCMICSEL, 0,
		ARRAY_SIZE(twl4030_micpathtx1_texts),
		twl4030_micpathtx1_texts);

static const struct snd_kcontrol_new twl4030_dapm_micpathtx1_control =
SOC_DAPM_ENUM("Route", twl4030_micpathtx1_enum);

/* TX2 L/R Analog/Digital microphone selection */
static const char *twl4030_micpathtx2_texts[] =
{"Analog", "Digimic1"};

static const struct soc_enum twl4030_micpathtx2_enum =
SOC_ENUM_SINGLE(TWL4030_REG_ADCMICSEL, 2,
		ARRAY_SIZE(twl4030_micpathtx2_texts),
		twl4030_micpathtx2_texts);

static const struct snd_kcontrol_new twl4030_dapm_micpathtx2_control =
SOC_DAPM_ENUM("Route", twl4030_micpathtx2_enum);

/* Analog bypass for AudioR1 */
static const struct snd_kcontrol_new twl4030_dapm_abypassr1_control =
SOC_DAPM_SINGLE("Switch", TWL4030_REG_ARXR1_APGA_CTL, 2, 1, 0);

/* Analog bypass for AudioL1 */
static const struct snd_kcontrol_new twl4030_dapm_abypassl1_control =
SOC_DAPM_SINGLE("Switch", TWL4030_REG_ARXL1_APGA_CTL, 2, 1, 0);

/* Analog bypass for AudioR2 */
static const struct snd_kcontrol_new twl4030_dapm_abypassr2_control =
SOC_DAPM_SINGLE("Switch", TWL4030_REG_ARXR2_APGA_CTL, 2, 1, 0);

/* Analog bypass for AudioL2 */
static const struct snd_kcontrol_new twl4030_dapm_abypassl2_control =
SOC_DAPM_SINGLE("Switch", TWL4030_REG_ARXL2_APGA_CTL, 2, 1, 0);

/* Analog bypass for Voice */
static const struct snd_kcontrol_new twl4030_dapm_abypassv_control =
SOC_DAPM_SINGLE("Switch", TWL4030_REG_VDL_APGA_CTL, 2, 1, 0);

/* Digital bypass gain, mute instead of -30dB */
static const unsigned int twl4030_dapm_dbypass_tlv[] = {
	TLV_DB_RANGE_HEAD(3),
	0, 1, TLV_DB_SCALE_ITEM(-3000, 600, 1),
	2, 3, TLV_DB_SCALE_ITEM(-2400, 0, 0),
	4, 7, TLV_DB_SCALE_ITEM(-1800, 600, 0),
};

/* Digital bypass left (TX1L -> RX2L) */
static const struct snd_kcontrol_new twl4030_dapm_dbypassl_control =
SOC_DAPM_SINGLE_TLV("Volume",
		TWL4030_REG_ATX2ARXPGA, 3, 7, 0,
		twl4030_dapm_dbypass_tlv);

/* Digital bypass right (TX1R -> RX2R) */
static const struct snd_kcontrol_new twl4030_dapm_dbypassr_control =
SOC_DAPM_SINGLE_TLV("Volume",
		TWL4030_REG_ATX2ARXPGA, 0, 7, 0,
		twl4030_dapm_dbypass_tlv);

/*
 * Voice Sidetone GAIN volume control:
 * from -51 to -10 dB in 1 dB steps (mute instead of -51 dB)
 */
static DECLARE_TLV_DB_SCALE(twl4030_dapm_dbypassv_tlv, -5100, 100, 1);

/* Digital bypass voice: sidetone (VUL -> VDL)*/
static const struct snd_kcontrol_new twl4030_dapm_dbypassv_control =
SOC_DAPM_SINGLE_TLV("Volume",
		TWL4030_REG_VSTPGA, 0, 0x29, 0,
		twl4030_dapm_dbypassv_tlv);

/*
 * Output PGA builder:
 * Handle the muting and unmuting of the given output (turning off the
 * amplifier associated with the output pin)
 * On mute bypass the reg_cache and write 0 to the register
 * On unmute: restore the register content from the reg_cache
 * Outputs handled in this way:  Earpiece, PreDrivL/R, CarkitL/R
 */
#define TWL4030_OUTPUT_PGA(pin_name, reg, mask)				\
	static int pin_name##pga_event(struct snd_soc_dapm_widget *w,		\
			struct snd_kcontrol *kcontrol, int event)		\
{									\
	struct twl4030_priv *twl4030 = snd_soc_codec_get_drvdata(w->codec); \
	DBG("\n");								\
	switch (event) {						\
		case SND_SOC_DAPM_POST_PMU:					\
										twl4030->pin_name##_enabled = 1;			\
		twl4030_write(w->codec, reg,				\
				twl4030_read_reg_cache(w->codec, reg));		\
		break;							\
		case SND_SOC_DAPM_POST_PMD:					\
										twl4030->pin_name##_enabled = 0;			\
		twl_i2c_write_u8(TWL4030_MODULE_AUDIO_VOICE,		\
				0, reg);			\
		break;							\
	}								\
	return 0;							\
}

TWL4030_OUTPUT_PGA(earpiece, TWL4030_REG_EAR_CTL, TWL4030_EAR_GAIN);
TWL4030_OUTPUT_PGA(predrivel, TWL4030_REG_PREDL_CTL, TWL4030_PREDL_GAIN);
TWL4030_OUTPUT_PGA(predriver, TWL4030_REG_PREDR_CTL, TWL4030_PREDR_GAIN);
TWL4030_OUTPUT_PGA(carkitl, TWL4030_REG_PRECKL_CTL, TWL4030_PRECKL_GAIN);
TWL4030_OUTPUT_PGA(carkitr, TWL4030_REG_PRECKR_CTL, TWL4030_PRECKR_GAIN);

static void handsfree_ramp(struct snd_soc_codec *codec, int reg, int ramp)
{
	unsigned char hs_ctl;

	hs_ctl = twl4030_read_reg_cache(codec, reg);
	DBG("\n");

	if (ramp) {
		/* HF ramp-up */
		hs_ctl |= TWL4030_HF_CTL_REF_EN;
		twl4030_write(codec, reg, hs_ctl);
		udelay(10);
		hs_ctl |= TWL4030_HF_CTL_RAMP_EN;
		twl4030_write(codec, reg, hs_ctl);
		udelay(40);
		hs_ctl |= TWL4030_HF_CTL_LOOP_EN;
		hs_ctl |= TWL4030_HF_CTL_HB_EN;
		twl4030_write(codec, reg, hs_ctl);
	} else {
		/* HF ramp-down */
		hs_ctl &= ~TWL4030_HF_CTL_LOOP_EN;
		hs_ctl &= ~TWL4030_HF_CTL_HB_EN;
		twl4030_write(codec, reg, hs_ctl);
		hs_ctl &= ~TWL4030_HF_CTL_RAMP_EN;
		twl4030_write(codec, reg, hs_ctl);
		udelay(40);
		hs_ctl &= ~TWL4030_HF_CTL_REF_EN;
		twl4030_write(codec, reg, hs_ctl);
	}
}

static int handsfreelpga_event(struct snd_soc_dapm_widget *w,
		struct snd_kcontrol *kcontrol, int event)
{
	DBG("\n");

	switch (event) {
		case SND_SOC_DAPM_POST_PMU:
			handsfree_ramp(w->codec, TWL4030_REG_HFL_CTL, 1);
			break;
		case SND_SOC_DAPM_POST_PMD:
			handsfree_ramp(w->codec, TWL4030_REG_HFL_CTL, 0);
			break;
	}
	return 0;
}

static int handsfreerpga_event(struct snd_soc_dapm_widget *w,
		struct snd_kcontrol *kcontrol, int event)
{
	DBG("\n");

	switch (event) {
		case SND_SOC_DAPM_POST_PMU:
			handsfree_ramp(w->codec, TWL4030_REG_HFR_CTL, 1);
			break;
		case SND_SOC_DAPM_POST_PMD:
			handsfree_ramp(w->codec, TWL4030_REG_HFR_CTL, 0);
			break;
	}
	return 0;
}

static int vibramux_event(struct snd_soc_dapm_widget *w,
		struct snd_kcontrol *kcontrol, int event)
{
	DBG("\n");

	twl4030_write(w->codec, TWL4030_REG_VIBRA_SET, 0xff);
	return 0;
}

static int apll_event(struct snd_soc_dapm_widget *w,
		struct snd_kcontrol *kcontrol, int event)
{
	DBG("\n");

	switch (event) {
		case SND_SOC_DAPM_PRE_PMU:
			twl4030_apll_enable(w->codec, 1);
			break;
		case SND_SOC_DAPM_POST_PMD:
			twl4030_apll_enable(w->codec, 0);
			break;
	}
	return 0;
}

static int aif_event(struct snd_soc_dapm_widget *w,
		struct snd_kcontrol *kcontrol, int event)
{
	u8 audio_if;

	audio_if = twl4030_read_reg_cache(w->codec, TWL4030_REG_AUDIO_IF);

//#if defined(CONFIG_PRODUCT_LGE_KU5900)||defined(CONFIG_PRODUCT_LGE_LU6800)
/*[LGE_CHANGE] 20120908 pyocool.cho@lge.com "for p970" */
#if defined(CONFIG_PRODUCT_LGE_KU5900) ||defined(CONFIG_PRODUCT_LGE_LU6800) ||defined(CONFIG_PRODUCT_LGE_P970)
	struct twl4030_priv *twl4030 = snd_soc_codec_get_drvdata(w->codec);

    if((twl4030->is_calling)&&(get_twl4030_apll_state()))
    {
        printk("[Twl4030] aif_event(%d) is_calling TRUE ::::: RETURN \n",event);
        return;
    }
//#if defined(CONFIG_PRODUCT_LGE_KU5900) 	
/*[LGE_CHANGE] 20120908 pyocool.cho@lge.com "for p970" */
#if defined(CONFIG_PRODUCT_LGE_KU5900) || defined(CONFIG_PRODUCT_LGE_P970)
	if( (twl4030->is_calling == 0) && (cur_ext_amp_mode == OFF_MODE)){
		twl4030->predrivel_enabled = 0;
		twl4030->predriver_enabled = 0;
		wm9093_configure_path(cur_ext_amp_mode);
        }
#endif
#endif
	DBG("\n");

	switch (event) {
		case SND_SOC_DAPM_PRE_PMU:
			/* Enable AIF */
			/* enable the PLL before we use it to clock the DAI */
			twl4030_apll_enable(w->codec, 1);

			twl4030_write(w->codec, TWL4030_REG_AUDIO_IF,
					audio_if | TWL4030_AIF_EN);
			break;
		case SND_SOC_DAPM_POST_PMD:
			/* disable the DAI before we stop it's source PLL */
			twl4030_write(w->codec, TWL4030_REG_AUDIO_IF,
					audio_if &  ~TWL4030_AIF_EN);
			twl4030_apll_enable(w->codec, 0);
			break;
	}

//#if defined(CONFIG_PRODUCT_LGE_KU5900) 	
/*[LGE_CHANGE] 20120908 pyocool.cho@lge.com "for p970" */
#if defined(CONFIG_PRODUCT_LGE_KU5900) || defined(CONFIG_PRODUCT_LGE_P970)

        if((twl4030->is_calling == 0) && (cur_ext_amp_mode == SPEAKER_AUDIO_MODE)){
            twl4030->predrivel_enabled = 1;
            twl4030->predriver_enabled = 1;
            wm9093_configure_path(cur_ext_amp_mode);
        }
#endif

	return 0;
}

static void headset_ramp(struct snd_soc_codec *codec, int ramp)
{
	struct twl4030_codec_audio_data *pdata = codec->dev->platform_data;
	unsigned char hs_gain, hs_pop;
	struct twl4030_priv *twl4030 = snd_soc_codec_get_drvdata(codec);
	/* Base values for ramp delay calculation: 2^19 - 2^26 */
	unsigned int ramp_base[] = {524288, 1048576, 2097152, 4194304,
		8388608, 16777216, 33554432, 67108864};
	unsigned int delay;
    
    // wooho.jeong@lge.com 2012.09.25
    // ADD : Audio for FM Radio
	printk("[LUCKYJUN77] headset_ramp ramp = %d\n",ramp);
	call_headset_ramp = ramp;

	hs_gain = twl4030_read_reg_cache(codec, TWL4030_REG_HS_GAIN_SET);
	hs_pop = twl4030_read_reg_cache(codec, TWL4030_REG_HS_POPN_SET);
	delay = (ramp_base[(hs_pop & TWL4030_RAMP_DELAY) >> 2] /
			twl4030->sysclk) + 1;

	/* Enable external mute control, this dramatically reduces
	 * the pop-noise */
	if (pdata && pdata->hs_extmute) {
		if (pdata->set_hs_extmute) {
			pdata->set_hs_extmute(1);
		} else {
			hs_pop |= TWL4030_EXTMUTE;
#if 0
			twl4030_write(codec, TWL4030_REG_HS_POPN_SET, hs_pop);
#else
			twl4030_write_reg_cache(codec, TWL4030_REG_HS_POPN_SET, hs_pop);
			twl_i2c_write_u8(TWL4030_MODULE_AUDIO_VOICE, hs_pop, TWL4030_REG_HS_POPN_SET);
#endif

		}
	}

	if (ramp) {
		/* Headset ramp-up according to the TRM */
		hs_pop |= TWL4030_VMID_EN;
#if 0	//junyeop.kim@lge.com
		twl4030_write(codec, TWL4030_REG_HS_POPN_SET, hs_pop);
		/* Actually write to the register */
		twl_i2c_write_u8(TWL4030_MODULE_AUDIO_VOICE,
				hs_gain,
				TWL4030_REG_HS_GAIN_SET);
#else
		twl4030_write_reg_cache(codec, TWL4030_REG_HS_POPN_SET, hs_pop);
		twl4030_write_reg_cache(codec, TWL4030_REG_HS_GAIN_SET, hs_gain);
		twl_i2c_write_u8(TWL4030_MODULE_AUDIO_VOICE, hs_pop, TWL4030_REG_HS_POPN_SET);
		twl_i2c_write_u8(TWL4030_MODULE_AUDIO_VOICE, hs_gain, TWL4030_REG_HS_GAIN_SET);
#endif
		hs_pop |= TWL4030_RAMP_EN;
#if 0	//junyeop.kim@lge.com
		twl4030_write(codec, TWL4030_REG_HS_POPN_SET, hs_pop);
#else
		twl4030_write_reg_cache(codec, TWL4030_REG_HS_POPN_SET, hs_pop);
		twl_i2c_write_u8(TWL4030_MODULE_AUDIO_VOICE, hs_pop, TWL4030_REG_HS_POPN_SET);
#endif
		/* Wait ramp delay time + 1, so the VMID can settle */
		twl4030_wait_ms(delay);
	} else {
		/* Headset ramp-down _not_ according to
		 * the TRM, but in a way that it is working */
		hs_pop &= ~TWL4030_RAMP_EN;
#if 0
		twl4030_write(codec, TWL4030_REG_HS_POPN_SET, hs_pop);
#else
		twl4030_write_reg_cache(codec, TWL4030_REG_HS_POPN_SET, hs_pop);
		twl_i2c_write_u8(TWL4030_MODULE_AUDIO_VOICE, hs_pop, TWL4030_REG_HS_POPN_SET);
#endif
		/* Wait ramp delay time + 1, so the VMID can settle */
		twl4030_wait_ms(delay);
		/* Bypass the reg_cache to mute the headset */
		twl_i2c_write_u8(TWL4030_MODULE_AUDIO_VOICE,
				hs_gain & (~0x0f),
				TWL4030_REG_HS_GAIN_SET);

		hs_pop &= ~TWL4030_VMID_EN;
		twl4030_write(codec, TWL4030_REG_HS_POPN_SET, hs_pop);
	}

	/* Disable external mute */
	if (pdata && pdata->hs_extmute) {
		if (pdata->set_hs_extmute) {
			pdata->set_hs_extmute(0);
		} else {
			hs_pop &= ~TWL4030_EXTMUTE;
#if 0
			twl4030_write(codec, TWL4030_REG_HS_POPN_SET, hs_pop);
#else
			twl4030_write_reg_cache(codec, TWL4030_REG_HS_POPN_SET, hs_pop);
			twl_i2c_write_u8(TWL4030_MODULE_AUDIO_VOICE, hs_pop, TWL4030_REG_HS_POPN_SET);
#endif
		}
	}
}

static int headsetlpga_event(struct snd_soc_dapm_widget *w,
		struct snd_kcontrol *kcontrol, int event)
{
	struct twl4030_priv *twl4030 = snd_soc_codec_get_drvdata(w->codec);
	
	DBG("event %d\n",event);
	

#if defined(CONFIG_PRODUCT_LGE_HUB) /* 20100913 junyeop.kim@lge.com, fix the headset call path */
	if(twl4030->is_calling)
		return 0;
#elif defined(CONFIG_PRODUCT_LGE_LU6800) || defined(CONFIG_PRODUCT_LGE_KU5900) || defined (CONFIG_PRODUCT_LGE_P970)/* 20100913 junyeop.kim@lge.com, fix the headset call path */
	if(twl4030->is_calling|| fmradio_is_on == 1)
	{
	    printk(KERN_ERR "fmradio_is_on %d\n", fmradio_is_on);
		return 0;
	}
#else
	//nothing
#endif
//#if defined(CONFIG_PRODUCT_LGE_KU5900) 	
/*[LGE_CHANGE] 20120908 pyocool.cho@lge.com "for p970" */
#if defined(CONFIG_PRODUCT_LGE_KU5900) || defined(CONFIG_PRODUCT_LGE_P970)
	if(cur_ext_amp_mode == OFF_MODE)
		wm9093_configure_path(cur_ext_amp_mode);
#endif
	switch (event) {
		case SND_SOC_DAPM_POST_PMU:
			/* Do the ramp-up only once */
			if (!twl4030->hsr_enabled)
				headset_ramp(w->codec, 1);

			twl4030->hsl_enabled = 1;
			break;
		case SND_SOC_DAPM_POST_PMD:
			/* Do the ramp-down only if both headsetL/R is disabled */
			if (!twl4030->hsr_enabled)
				headset_ramp(w->codec, 0);

			twl4030->hsl_enabled = 0;
			break;
	}
	return 0;
}

static int headsetrpga_event(struct snd_soc_dapm_widget *w,
		struct snd_kcontrol *kcontrol, int event)
{
	struct twl4030_priv *twl4030 = snd_soc_codec_get_drvdata(w->codec);
	DBG("event %d\n",event);

#if defined(CONFIG_PRODUCT_LGE_HUB) /* 20100913 junyeop.kim@lge.com, fix the headset call path */
	if(twl4030->is_calling)
		return 0;
#elif defined(CONFIG_PRODUCT_LGE_LU6800) || defined(CONFIG_PRODUCT_LGE_KU5900) || defined (CONFIG_PRODUCT_LGE_P970)/* 20100913 junyeop.kim@lge.com, fix the headset call path */
	if(twl4030->is_calling || fmradio_is_on == 1)
	{
	    printk(KERN_ERR "fmradio_is_on %d\n", fmradio_is_on);
		return 0;
	}
#else
	//nothing
#endif
//#if defined(CONFIG_PRODUCT_LGE_KU5900) 	
/*[LGE_CHANGE] 20120908 pyocool.cho@lge.com "for p970" */
#if defined(CONFIG_PRODUCT_LGE_KU5900) || defined(CONFIG_PRODUCT_LGE_P970)
         if(cur_ext_amp_mode == OFF_MODE){
             wm9093_configure_path(cur_ext_amp_mode);
             twl4030->predriver_enabled = 0;
             twl4030->predrivel_enabled = 0;
        }
#endif
	switch (event) {
		case SND_SOC_DAPM_POST_PMU:
			/* Do the ramp-up only once */
			if (!twl4030->hsl_enabled)
				headset_ramp(w->codec, 1);

			twl4030->hsr_enabled = 1;
			break;
		case SND_SOC_DAPM_POST_PMD:
			/* Do the ramp-down only if both headsetL/R is disabled */
			if (!twl4030->hsl_enabled)
				headset_ramp(w->codec, 0);

			twl4030->hsr_enabled = 0;
			break;
	}
//#if defined(CONFIG_PRODUCT_LGE_KU5900)	
/*[LGE_CHANGE] 20120908 pyocool.cho@lge.com "for p970" */
#if defined(CONFIG_PRODUCT_LGE_KU5900) || defined(CONFIG_PRODUCT_LGE_P970)
//        msleep(10);
        // wooho.jeong@lge.com 2012.09.25
        // ADD : Audio for FM Radio
//        if(cur_ext_amp_mode == HEADSET_AUDIO_MODE || cur_ext_amp_mode == SPEAKER_HEADSET_DUAL_AUDIO_MODE || cur_ext_amp_mode == HEADSET_FMR_MODE){
        if(cur_ext_amp_mode == HEADSET_AUDIO_MODE || cur_ext_amp_mode == SPEAKER_HEADSET_DUAL_AUDIO_MODE){
             wm9093_configure_path(cur_ext_amp_mode);
             if(cur_ext_amp_mode == SPEAKER_HEADSET_DUAL_AUDIO_MODE){
                twl4030->predrivel_enabled = 1;
                twl4030->predriver_enabled = 1;
            }
        }
#endif
	return 0;
}

static int digimic_event(struct snd_soc_dapm_widget *w,
		struct snd_kcontrol *kcontrol, int event)
{
	struct twl4030_priv *twl4030 = snd_soc_codec_get_drvdata(w->codec);
	DBG("\n");

	if (twl4030->digimic_delay)
		twl4030_wait_ms(twl4030->digimic_delay);
	return 0;
}

/*
 * Some of the gain controls in TWL (mostly those which are associated with
 * the outputs) are implemented in an interesting way:
 * 0x0 : Power down (mute)
 * 0x1 : 6dB
 * 0x2 : 0 dB
 * 0x3 : -6 dB
 * Inverting not going to help with these.
 * Custom volsw and volsw_2r get/put functions to handle these gain bits.
 */
#define SOC_DOUBLE_TLV_TWL4030(xname, xreg, shift_left, shift_right, xmax,\
		xinvert, tlv_array) \
{	.iface = SNDRV_CTL_ELEM_IFACE_MIXER, .name = (xname),\
	.access = SNDRV_CTL_ELEM_ACCESS_TLV_READ |\
	SNDRV_CTL_ELEM_ACCESS_READWRITE,\
	.tlv.p = (tlv_array), \
	.info = snd_soc_info_volsw, \
	.get = snd_soc_get_volsw_twl4030, \
	.put = snd_soc_put_volsw_twl4030, \
	.private_value = (unsigned long)&(struct soc_mixer_control) \
	{.reg = xreg, .shift = shift_left, .rshift = shift_right,\
		.max = xmax, .invert = xinvert} }
#define SOC_DOUBLE_R_TLV_TWL4030(xname, reg_left, reg_right, xshift, xmax,\
		xinvert, tlv_array) \
{	.iface = SNDRV_CTL_ELEM_IFACE_MIXER, .name = (xname),\
	.access = SNDRV_CTL_ELEM_ACCESS_TLV_READ |\
	SNDRV_CTL_ELEM_ACCESS_READWRITE,\
	.tlv.p = (tlv_array), \
	.info = snd_soc_info_volsw_2r, \
	.get = snd_soc_get_volsw_r2_twl4030,\
	.put = snd_soc_put_volsw_r2_twl4030, \
	.private_value = (unsigned long)&(struct soc_mixer_control) \
	{.reg = reg_left, .rreg = reg_right, .shift = xshift, \
		.rshift = xshift, .max = xmax, .invert = xinvert} }
#define SOC_SINGLE_TLV_TWL4030(xname, xreg, xshift, xmax, xinvert, tlv_array) \
	SOC_DOUBLE_TLV_TWL4030(xname, xreg, xshift, xshift, xmax, \
			xinvert, tlv_array)

static int snd_soc_get_volsw_twl4030(struct snd_kcontrol *kcontrol,
		struct snd_ctl_elem_value *ucontrol)
{
	struct soc_mixer_control *mc =
		(struct soc_mixer_control *)kcontrol->private_value;
	struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol);
	unsigned int reg = mc->reg;
	unsigned int shift = mc->shift;
	unsigned int rshift = mc->rshift;
	int max = mc->max;
	int mask = (1 << fls(max)) - 1;

	ucontrol->value.integer.value[0] =
		(snd_soc_read(codec, reg) >> shift) & mask;
	if (ucontrol->value.integer.value[0])
		ucontrol->value.integer.value[0] =
			max + 1 - ucontrol->value.integer.value[0];

	if (shift != rshift) {
		ucontrol->value.integer.value[1] =
			(snd_soc_read(codec, reg) >> rshift) & mask;
		if (ucontrol->value.integer.value[1])
			ucontrol->value.integer.value[1] =
				max + 1 - ucontrol->value.integer.value[1];
	}

	return 0;
}

static int snd_soc_put_volsw_twl4030(struct snd_kcontrol *kcontrol,
		struct snd_ctl_elem_value *ucontrol)
{
	struct soc_mixer_control *mc =
		(struct soc_mixer_control *)kcontrol->private_value;
	struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol);
	unsigned int reg = mc->reg;
	unsigned int shift = mc->shift;
	unsigned int rshift = mc->rshift;
	int max = mc->max;
	int mask = (1 << fls(max)) - 1;
	unsigned short val, val2, val_mask;
	DBG("\n");

	val = (ucontrol->value.integer.value[0] & mask);

	val_mask = mask << shift;
	if (val)
		val = max + 1 - val;
	val = val << shift;
	if (shift != rshift) {
		val2 = (ucontrol->value.integer.value[1] & mask);
		val_mask |= mask << rshift;
		if (val2)
			val2 = max + 1 - val2;
		val |= val2 << rshift;
	}
	return snd_soc_update_bits(codec, reg, val_mask, val);
}

static int snd_soc_get_volsw_r2_twl4030(struct snd_kcontrol *kcontrol,
		struct snd_ctl_elem_value *ucontrol)
{
	struct soc_mixer_control *mc =
		(struct soc_mixer_control *)kcontrol->private_value;
	struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol);
	unsigned int reg = mc->reg;
	unsigned int reg2 = mc->rreg;
	unsigned int shift = mc->shift;
	int max = mc->max;
	int mask = (1<<fls(max))-1;
	DBG("\n");

	ucontrol->value.integer.value[0] =
		(snd_soc_read(codec, reg) >> shift) & mask;
	ucontrol->value.integer.value[1] =
		(snd_soc_read(codec, reg2) >> shift) & mask;

	if (ucontrol->value.integer.value[0])
		ucontrol->value.integer.value[0] =
			max + 1 - ucontrol->value.integer.value[0];
	if (ucontrol->value.integer.value[1])
		ucontrol->value.integer.value[1] =
			max + 1 - ucontrol->value.integer.value[1];

	return 0;
}

static int snd_soc_put_volsw_r2_twl4030(struct snd_kcontrol *kcontrol,
		struct snd_ctl_elem_value *ucontrol)
{
	struct soc_mixer_control *mc =
		(struct soc_mixer_control *)kcontrol->private_value;
	struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol);
	unsigned int reg = mc->reg;
	unsigned int reg2 = mc->rreg;
	unsigned int shift = mc->shift;
	int max = mc->max;
	int mask = (1 << fls(max)) - 1;
	int err;
	unsigned short val, val2, val_mask;

	val_mask = mask << shift;
	val = (ucontrol->value.integer.value[0] & mask);
	val2 = (ucontrol->value.integer.value[1] & mask);
	DBG("\n");

	if (val)
		val = max + 1 - val;
	if (val2)
		val2 = max + 1 - val2;

	val = val << shift;
	val2 = val2 << shift;

	err = snd_soc_update_bits(codec, reg, val_mask, val);
	if (err < 0)
		return err;

	err = snd_soc_update_bits(codec, reg2, val_mask, val2);
	return err;
}

/* Codec operation modes */
static const char *twl4030_op_modes_texts[] = {
	"Option 2 (voice/audio)", "Option 1 (audio)"
};

static const struct soc_enum twl4030_op_modes_enum =
SOC_ENUM_SINGLE(TWL4030_REG_CODEC_MODE, 0,
		ARRAY_SIZE(twl4030_op_modes_texts),
		twl4030_op_modes_texts);

static int snd_soc_put_twl4030_opmode_enum_double(struct snd_kcontrol *kcontrol,
		struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol);
	struct twl4030_priv *twl4030 = snd_soc_codec_get_drvdata(codec);
	struct soc_enum *e = (struct soc_enum *)kcontrol->private_value;
	unsigned short val;
	unsigned short mask, bitmask;
	DBG("\n");

	if (twl4030->configured) {
		printk(KERN_ERR "twl4030 operation mode cannot be "
				"changed on-the-fly\n");
		return -EBUSY;
	}

	for (bitmask = 1; bitmask < e->max; bitmask <<= 1)
		;
	if (ucontrol->value.enumerated.item[0] > e->max - 1)
		return -EINVAL;

	val = ucontrol->value.enumerated.item[0] << e->shift_l;
	mask = (bitmask - 1) << e->shift_l;
	if (e->shift_l != e->shift_r) {
		if (ucontrol->value.enumerated.item[1] > e->max - 1)
			return -EINVAL;
		val |= ucontrol->value.enumerated.item[1] << e->shift_r;
		mask |= (bitmask - 1) << e->shift_r;
	}

	return snd_soc_update_bits(codec, e->reg, mask, val);
}

/*
 * FGAIN volume control:
 * from -62 to 0 dB in 1 dB steps (mute instead of -63 dB)
 */
static DECLARE_TLV_DB_SCALE(digital_fine_tlv, -6300, 100, 1);

/*
 * CGAIN volume control:
 * 0 dB to 12 dB in 6 dB steps
 * value 2 and 3 means 12 dB
 */
static DECLARE_TLV_DB_SCALE(digital_coarse_tlv, 0, 600, 0);

/*
 * Voice Downlink GAIN volume control:
 * from -37 to 12 dB in 1 dB steps (mute instead of -37 dB)
 */
static DECLARE_TLV_DB_SCALE(digital_voice_downlink_tlv, -3700, 100, 1);

/*
 * Analog playback gain
 * -24 dB to 12 dB in 2 dB steps
 */
static DECLARE_TLV_DB_SCALE(analog_tlv, -2400, 200, 0);

/*
 * Gain controls tied to outputs
 * -6 dB to 6 dB in 6 dB steps (mute instead of -12)
 */
static DECLARE_TLV_DB_SCALE(output_tvl, -1200, 600, 1);

/*
 * Gain control for earpiece amplifier
 * 0 dB to 12 dB in 6 dB steps (mute instead of -6)
 */
static DECLARE_TLV_DB_SCALE(output_ear_tvl, -600, 600, 1);

/*
 * Capture gain after the ADCs
 * from 0 dB to 31 dB in 1 dB steps
 */
static DECLARE_TLV_DB_SCALE(digital_capture_tlv, 0, 100, 0);

/*
 * Gain control for input amplifiers
 * 0 dB to 30 dB in 6 dB steps
 */
static DECLARE_TLV_DB_SCALE(input_gain_tlv, 0, 600, 0);

/* AVADC clock priority */
static const char *twl4030_avadc_clk_priority_texts[] = {
	"Voice high priority", "HiFi high priority"
};

static const struct soc_enum twl4030_avadc_clk_priority_enum =
SOC_ENUM_SINGLE(TWL4030_REG_AVADC_CTL, 2,
		ARRAY_SIZE(twl4030_avadc_clk_priority_texts),
		twl4030_avadc_clk_priority_texts);

static const char *twl4030_rampdelay_texts[] = {
	"27/20/14 ms", "55/40/27 ms", "109/81/55 ms", "218/161/109 ms",
	"437/323/218 ms", "874/645/437 ms", "1748/1291/874 ms",
	"3495/2581/1748 ms"
};

static const struct soc_enum twl4030_rampdelay_enum =
SOC_ENUM_SINGLE(TWL4030_REG_HS_POPN_SET, 2,
		ARRAY_SIZE(twl4030_rampdelay_texts),
		twl4030_rampdelay_texts);

/* Vibra H-bridge direction mode */
static const char *twl4030_vibradirmode_texts[] = {
	"Vibra H-bridge direction", "Audio data MSB",
};

static const struct soc_enum twl4030_vibradirmode_enum =
SOC_ENUM_SINGLE(TWL4030_REG_VIBRA_CTL, 5,
		ARRAY_SIZE(twl4030_vibradirmode_texts),
		twl4030_vibradirmode_texts);

/* Vibra H-bridge direction */
static const char *twl4030_vibradir_texts[] = {
	"Positive polarity", "Negative polarity",
};

static const struct soc_enum twl4030_vibradir_enum =
SOC_ENUM_SINGLE(TWL4030_REG_VIBRA_CTL, 1,
		ARRAY_SIZE(twl4030_vibradir_texts),
		twl4030_vibradir_texts);

/* Digimic Left and right swapping */
static const char *twl4030_digimicswap_texts[] = {
	"Not swapped", "Swapped",
};

static const struct soc_enum twl4030_digimicswap_enum =
SOC_ENUM_SINGLE(TWL4030_REG_MISC_SET_1, 0,
		ARRAY_SIZE(twl4030_digimicswap_texts),
		twl4030_digimicswap_texts);

static const struct snd_kcontrol_new twl4030_snd_controls[] = {
	/* Codec operation mode control */
	SOC_ENUM_EXT("Codec Operation Mode", twl4030_op_modes_enum,
			snd_soc_get_enum_double,
			snd_soc_put_twl4030_opmode_enum_double),

	/* Common playback gain controls */
	SOC_DOUBLE_R_TLV("DAC1 Digital Fine Playback Volume",
			TWL4030_REG_ARXL1PGA, TWL4030_REG_ARXR1PGA,
			0, 0x3f, 0, digital_fine_tlv),
	SOC_DOUBLE_R_TLV("DAC2 Digital Fine Playback Volume",
			TWL4030_REG_ARXL2PGA, TWL4030_REG_ARXR2PGA,
			0, 0x3f, 0, digital_fine_tlv),

	SOC_DOUBLE_R_TLV("DAC1 Digital Coarse Playback Volume",
			TWL4030_REG_ARXL1PGA, TWL4030_REG_ARXR1PGA,
			6, 0x2, 0, digital_coarse_tlv),
	SOC_DOUBLE_R_TLV("DAC2 Digital Coarse Playback Volume",
			TWL4030_REG_ARXL2PGA, TWL4030_REG_ARXR2PGA,
			6, 0x2, 0, digital_coarse_tlv),

	SOC_DOUBLE_R_TLV("DAC1 Analog Playback Volume",
			TWL4030_REG_ARXL1_APGA_CTL, TWL4030_REG_ARXR1_APGA_CTL,
			3, 0x12, 1, analog_tlv),
	SOC_DOUBLE_R_TLV("DAC2 Analog Playback Volume",
			TWL4030_REG_ARXL2_APGA_CTL, TWL4030_REG_ARXR2_APGA_CTL,
			3, 0x12, 1, analog_tlv),
	SOC_DOUBLE_R("DAC1 Analog Playback Switch",
			TWL4030_REG_ARXL1_APGA_CTL, TWL4030_REG_ARXR1_APGA_CTL,
			1, 1, 0),
	SOC_DOUBLE_R("DAC2 Analog Playback Switch",
			TWL4030_REG_ARXL2_APGA_CTL, TWL4030_REG_ARXR2_APGA_CTL,
			1, 1, 0),

	/* Common voice downlink gain controls */
	SOC_SINGLE_TLV("DAC Voice Digital Downlink Volume",
			TWL4030_REG_VRXPGA, 0, 0x31, 0, digital_voice_downlink_tlv),

	SOC_SINGLE_TLV("DAC Voice Analog Downlink Volume",
			TWL4030_REG_VDL_APGA_CTL, 3, 0x12, 1, analog_tlv),

	SOC_SINGLE("DAC Voice Analog Downlink Switch",
			TWL4030_REG_VDL_APGA_CTL, 1, 1, 0),

	/* Separate output gain controls */
	SOC_DOUBLE_R_TLV_TWL4030("PreDriv Playback Volume",
			TWL4030_REG_PREDL_CTL, TWL4030_REG_PREDR_CTL,
			4, 3, 0, output_tvl),

	SOC_DOUBLE_TLV_TWL4030("Headset Playback Volume",
			TWL4030_REG_HS_GAIN_SET, 0, 2, 3, 0, output_tvl),

	SOC_DOUBLE_R_TLV_TWL4030("Carkit Playback Volume",
			TWL4030_REG_PRECKL_CTL, TWL4030_REG_PRECKR_CTL,
			4, 3, 0, output_tvl),

	SOC_SINGLE_TLV_TWL4030("Earpiece Playback Volume",
			TWL4030_REG_EAR_CTL, 4, 3, 0, output_ear_tvl),

	/* Common capture gain controls */
	SOC_DOUBLE_R_TLV("TX1 Digital Capture Volume",
			TWL4030_REG_ATXL1PGA, TWL4030_REG_ATXR1PGA,
			0, 0x1f, 0, digital_capture_tlv),
	SOC_DOUBLE_R_TLV("TX2 Digital Capture Volume",
			TWL4030_REG_AVTXL2PGA, TWL4030_REG_AVTXR2PGA,
			0, 0x1f, 0, digital_capture_tlv),

	SOC_DOUBLE_TLV("Analog Capture Volume", TWL4030_REG_ANAMIC_GAIN,
			0, 3, 5, 0, input_gain_tlv),

	// 20100429 junyeop.kim@lge.com FM radio audio gain setting [START_LGE]
#if 0
	SOC_DOUBLE_TLV("DAC2 AnalogL Playback Volume", TWL4030_REG_ARXL2_APGA_CTL,
			3, 3, 0x12, 0, analog_tlv),

	SOC_DOUBLE_TLV("DAC2 AnalogR Playback Volume", TWL4030_REG_ARXR2_APGA_CTL,
			3, 3, 0x12, 0, analog_tlv),
#else
	SOC_SINGLE_TLV("DAC2 AnalogL Playback Volume", TWL4030_REG_ARXL2_APGA_CTL,
			3, 0x12, 0, analog_tlv),

	SOC_SINGLE_TLV("DAC2 AnalogR Playback Volume", TWL4030_REG_ARXR2_APGA_CTL,
			3, 0x12, 0, analog_tlv),
#endif
	// 20100426 junyeop.kim@lge.com FM audio gain setting [END_LGE]

	SOC_ENUM("AVADC Clock Priority", twl4030_avadc_clk_priority_enum),

	SOC_ENUM("HS ramp delay", twl4030_rampdelay_enum),

	SOC_ENUM("Vibra H-bridge mode", twl4030_vibradirmode_enum),
	SOC_ENUM("Vibra H-bridge direction", twl4030_vibradir_enum),

	SOC_ENUM("Digimic LR Swap", twl4030_digimicswap_enum),
};

static const struct snd_soc_dapm_widget twl4030_dapm_widgets[] = {
	/* Left channel inputs */
	SND_SOC_DAPM_INPUT("MAINMIC"),
	SND_SOC_DAPM_INPUT("HSMIC"),
	SND_SOC_DAPM_INPUT("AUXL"),
	SND_SOC_DAPM_INPUT("CARKITMIC"),
	/* Right channel inputs */
	SND_SOC_DAPM_INPUT("SUBMIC"),
	SND_SOC_DAPM_INPUT("AUXR"),
	/* Digital microphones (Stereo) */
	SND_SOC_DAPM_INPUT("DIGIMIC0"),
	SND_SOC_DAPM_INPUT("DIGIMIC1"),

	/* Outputs */
	SND_SOC_DAPM_OUTPUT("EARPIECE"),
	SND_SOC_DAPM_OUTPUT("PREDRIVEL"),
	SND_SOC_DAPM_OUTPUT("PREDRIVER"),
	SND_SOC_DAPM_OUTPUT("HSOL"),
	SND_SOC_DAPM_OUTPUT("HSOR"),
	SND_SOC_DAPM_OUTPUT("CARKITL"),
	SND_SOC_DAPM_OUTPUT("CARKITR"),
	SND_SOC_DAPM_OUTPUT("HFL"),
	SND_SOC_DAPM_OUTPUT("HFR"),
	SND_SOC_DAPM_OUTPUT("VIBRA"),

	/* AIF and APLL clocks for running DAIs (including loopback) */
	SND_SOC_DAPM_OUTPUT("Virtual HiFi OUT"),
	SND_SOC_DAPM_INPUT("Virtual HiFi IN"),
	SND_SOC_DAPM_OUTPUT("Virtual Voice OUT"),

	/* DACs */
	SND_SOC_DAPM_DAC("DAC Right1", "Right Front HiFi Playback",
			SND_SOC_NOPM, 0, 0),
	SND_SOC_DAPM_DAC("DAC Left1", "Left Front HiFi Playback",
			SND_SOC_NOPM, 0, 0),
	SND_SOC_DAPM_DAC("DAC Right2", "Right Rear HiFi Playback",
			SND_SOC_NOPM, 0, 0),
	SND_SOC_DAPM_DAC("DAC Left2", "Left Rear HiFi Playback",
			SND_SOC_NOPM, 0, 0),
	SND_SOC_DAPM_DAC("DAC Voice", "Voice Playback",
			SND_SOC_NOPM, 0, 0),

	/* Analog bypasses */
	SND_SOC_DAPM_SWITCH("Right1 Analog Loopback", SND_SOC_NOPM, 0, 0,
			&twl4030_dapm_abypassr1_control),
	SND_SOC_DAPM_SWITCH("Left1 Analog Loopback", SND_SOC_NOPM, 0, 0,
			&twl4030_dapm_abypassl1_control),
	SND_SOC_DAPM_SWITCH("Right2 Analog Loopback", SND_SOC_NOPM, 0, 0,
			&twl4030_dapm_abypassr2_control),
	SND_SOC_DAPM_SWITCH("Left2 Analog Loopback", SND_SOC_NOPM, 0, 0,
			&twl4030_dapm_abypassl2_control),
	SND_SOC_DAPM_SWITCH("Voice Analog Loopback", SND_SOC_NOPM, 0, 0,
			&twl4030_dapm_abypassv_control),

	/* Master analog loopback switch */
	SND_SOC_DAPM_SUPPLY("FM Loop Enable", TWL4030_REG_MISC_SET_1, 5, 0,
			NULL, 0),

	/* Digital bypasses */
	SND_SOC_DAPM_SWITCH("Left Digital Loopback", SND_SOC_NOPM, 0, 0,
			&twl4030_dapm_dbypassl_control),
	SND_SOC_DAPM_SWITCH("Right Digital Loopback", SND_SOC_NOPM, 0, 0,
			&twl4030_dapm_dbypassr_control),
	SND_SOC_DAPM_SWITCH("Voice Digital Loopback", SND_SOC_NOPM, 0, 0,
			&twl4030_dapm_dbypassv_control),

	/* Digital mixers, power control for the physical DACs */
	SND_SOC_DAPM_MIXER("Digital R1 Playback Mixer",
			TWL4030_REG_AVDAC_CTL, 0, 0, NULL, 0),
	SND_SOC_DAPM_MIXER("Digital L1 Playback Mixer",
			TWL4030_REG_AVDAC_CTL, 1, 0, NULL, 0),
	SND_SOC_DAPM_MIXER("Digital R2 Playback Mixer",
			TWL4030_REG_AVDAC_CTL, 2, 0, NULL, 0),
	SND_SOC_DAPM_MIXER("Digital L2 Playback Mixer",
			TWL4030_REG_AVDAC_CTL, 3, 0, NULL, 0),
	SND_SOC_DAPM_MIXER("Digital Voice Playback Mixer",
			TWL4030_REG_AVDAC_CTL, 4, 0, NULL, 0),

	/* Analog mixers, power control for the physical PGAs */
	SND_SOC_DAPM_MIXER("Analog R1 Playback Mixer",
			TWL4030_REG_ARXR1_APGA_CTL, 0, 0, NULL, 0),
	SND_SOC_DAPM_MIXER("Analog L1 Playback Mixer",
			TWL4030_REG_ARXL1_APGA_CTL, 0, 0, NULL, 0),
	SND_SOC_DAPM_MIXER("Analog R2 Playback Mixer",
			TWL4030_REG_ARXR2_APGA_CTL, 0, 0, NULL, 0),
	SND_SOC_DAPM_MIXER("Analog L2 Playback Mixer",
			TWL4030_REG_ARXL2_APGA_CTL, 0, 0, NULL, 0),
	SND_SOC_DAPM_MIXER("Analog Voice Playback Mixer",
			TWL4030_REG_VDL_APGA_CTL, 0, 0, NULL, 0),

	SND_SOC_DAPM_SUPPLY("APLL Enable", SND_SOC_NOPM, 0, 0, apll_event,
			SND_SOC_DAPM_PRE_PMU|SND_SOC_DAPM_POST_PMD),

	SND_SOC_DAPM_SUPPLY("AIF Enable", SND_SOC_NOPM, 0, 0, aif_event,
			SND_SOC_DAPM_PRE_PMU|SND_SOC_DAPM_POST_PMD),

	/* Output MIXER controls */
	/* Earpiece */
	SND_SOC_DAPM_MIXER("Earpiece Mixer", SND_SOC_NOPM, 0, 0,
			&twl4030_dapm_earpiece_controls[0],
			ARRAY_SIZE(twl4030_dapm_earpiece_controls)),
	SND_SOC_DAPM_PGA_E("Earpiece PGA", SND_SOC_NOPM,
			0, 0, NULL, 0, earpiecepga_event,
			SND_SOC_DAPM_POST_PMU|SND_SOC_DAPM_POST_PMD),
	/* PreDrivL/R */
	SND_SOC_DAPM_MIXER("PredriveL Mixer", SND_SOC_NOPM, 0, 0,
			&twl4030_dapm_predrivel_controls[0],
			ARRAY_SIZE(twl4030_dapm_predrivel_controls)),
	SND_SOC_DAPM_PGA_E("PredriveL PGA", SND_SOC_NOPM,
			0, 0, NULL, 0, predrivelpga_event,
			SND_SOC_DAPM_POST_PMU|SND_SOC_DAPM_POST_PMD),
	SND_SOC_DAPM_MIXER("PredriveR Mixer", SND_SOC_NOPM, 0, 0,
			&twl4030_dapm_predriver_controls[0],
			ARRAY_SIZE(twl4030_dapm_predriver_controls)),
	SND_SOC_DAPM_PGA_E("PredriveR PGA", SND_SOC_NOPM,
			0, 0, NULL, 0, predriverpga_event,
			SND_SOC_DAPM_POST_PMU|SND_SOC_DAPM_POST_PMD),
	/* HeadsetL/R */
	SND_SOC_DAPM_MIXER("HeadsetL Mixer", SND_SOC_NOPM, 0, 0,
			&twl4030_dapm_hsol_controls[0],
			ARRAY_SIZE(twl4030_dapm_hsol_controls)),
	SND_SOC_DAPM_PGA_E("HeadsetL PGA", SND_SOC_NOPM,
			0, 0, NULL, 0, headsetlpga_event,
			SND_SOC_DAPM_POST_PMU|SND_SOC_DAPM_POST_PMD),
	SND_SOC_DAPM_MIXER("HeadsetR Mixer", SND_SOC_NOPM, 0, 0,
			&twl4030_dapm_hsor_controls[0],
			ARRAY_SIZE(twl4030_dapm_hsor_controls)),
	SND_SOC_DAPM_PGA_E("HeadsetR PGA", SND_SOC_NOPM,
			0, 0, NULL, 0, headsetrpga_event,
			SND_SOC_DAPM_POST_PMU|SND_SOC_DAPM_POST_PMD),
	/* CarkitL/R */
	SND_SOC_DAPM_MIXER("CarkitL Mixer", SND_SOC_NOPM, 0, 0,
			&twl4030_dapm_carkitl_controls[0],
			ARRAY_SIZE(twl4030_dapm_carkitl_controls)),
	SND_SOC_DAPM_PGA_E("CarkitL PGA", SND_SOC_NOPM,
			0, 0, NULL, 0, carkitlpga_event,
			SND_SOC_DAPM_POST_PMU|SND_SOC_DAPM_POST_PMD),
	SND_SOC_DAPM_MIXER("CarkitR Mixer", SND_SOC_NOPM, 0, 0,
			&twl4030_dapm_carkitr_controls[0],
			ARRAY_SIZE(twl4030_dapm_carkitr_controls)),
	SND_SOC_DAPM_PGA_E("CarkitR PGA", SND_SOC_NOPM,
			0, 0, NULL, 0, carkitrpga_event,
			SND_SOC_DAPM_POST_PMU|SND_SOC_DAPM_POST_PMD),

	/* Output MUX controls */
	/* HandsfreeL/R */
	SND_SOC_DAPM_MUX("HandsfreeL Mux", SND_SOC_NOPM, 0, 0,
			&twl4030_dapm_handsfreel_control),
	SND_SOC_DAPM_SWITCH("HandsfreeL", SND_SOC_NOPM, 0, 0,
			&twl4030_dapm_handsfreelmute_control),
	SND_SOC_DAPM_PGA_E("HandsfreeL PGA", SND_SOC_NOPM,
			0, 0, NULL, 0, handsfreelpga_event,
			SND_SOC_DAPM_POST_PMU|SND_SOC_DAPM_POST_PMD),
	SND_SOC_DAPM_MUX("HandsfreeR Mux", SND_SOC_NOPM, 5, 0,
			&twl4030_dapm_handsfreer_control),
	SND_SOC_DAPM_SWITCH("HandsfreeR", SND_SOC_NOPM, 0, 0,
			&twl4030_dapm_handsfreermute_control),
	SND_SOC_DAPM_PGA_E("HandsfreeR PGA", SND_SOC_NOPM,
			0, 0, NULL, 0, handsfreerpga_event,
			SND_SOC_DAPM_POST_PMU|SND_SOC_DAPM_POST_PMD),
	/* Vibra */
	SND_SOC_DAPM_MUX_E("Vibra Mux", TWL4030_REG_VIBRA_CTL, 0, 0,
			&twl4030_dapm_vibra_control, vibramux_event,
			SND_SOC_DAPM_PRE_PMU),
	SND_SOC_DAPM_MUX("Vibra Route", SND_SOC_NOPM, 0, 0,
			&twl4030_dapm_vibrapath_control),

	/* Introducing four virtual ADC, since TWL4030 have four channel for
	   capture */
	SND_SOC_DAPM_ADC("ADC Virtual Left1", "Left Front Capture",
			SND_SOC_NOPM, 0, 0),
	SND_SOC_DAPM_ADC("ADC Virtual Right1", "Right Front Capture",
			SND_SOC_NOPM, 0, 0),
	SND_SOC_DAPM_ADC("ADC Virtual Left2", "Left Rear Capture",
			SND_SOC_NOPM, 0, 0),
	SND_SOC_DAPM_ADC("ADC Virtual Right2", "Right Rear Capture",
			SND_SOC_NOPM, 0, 0),

	/* Analog/Digital mic path selection.
	   TX1 Left/Right: either analog Left/Right or Digimic0
	   TX2 Left/Right: either analog Left/Right or Digimic1 */
	SND_SOC_DAPM_MUX("TX1 Capture Route", SND_SOC_NOPM, 0, 0,
			&twl4030_dapm_micpathtx1_control),
	SND_SOC_DAPM_MUX("TX2 Capture Route", SND_SOC_NOPM, 0, 0,
			&twl4030_dapm_micpathtx2_control),

	/* Analog input mixers for the capture amplifiers */
	SND_SOC_DAPM_MIXER("Analog Left",
			TWL4030_REG_ANAMICL, 4, 0,
			&twl4030_dapm_analoglmic_controls[0],
			ARRAY_SIZE(twl4030_dapm_analoglmic_controls)),
	SND_SOC_DAPM_MIXER("Analog Right",
			TWL4030_REG_ANAMICR, 4, 0,
			&twl4030_dapm_analogrmic_controls[0],
			ARRAY_SIZE(twl4030_dapm_analogrmic_controls)),

	SND_SOC_DAPM_PGA("ADC Physical Left",
			TWL4030_REG_AVADC_CTL, 3, 0, NULL, 0),
	SND_SOC_DAPM_PGA("ADC Physical Right",
			TWL4030_REG_AVADC_CTL, 1, 0, NULL, 0),

	SND_SOC_DAPM_PGA_E("Digimic0 Enable",
			TWL4030_REG_ADCMICSEL, 1, 0, NULL, 0,
			digimic_event, SND_SOC_DAPM_POST_PMU),
	SND_SOC_DAPM_PGA_E("Digimic1 Enable",
			TWL4030_REG_ADCMICSEL, 3, 0, NULL, 0,
			digimic_event, SND_SOC_DAPM_POST_PMU),

	SND_SOC_DAPM_SUPPLY("micbias1 select", TWL4030_REG_MICBIAS_CTL, 5, 0,
			NULL, 0),
	SND_SOC_DAPM_SUPPLY("micbias2 select", TWL4030_REG_MICBIAS_CTL, 6, 0,
			NULL, 0),

	SND_SOC_DAPM_MICBIAS("Mic Bias 1", TWL4030_REG_MICBIAS_CTL, 0, 0),
	SND_SOC_DAPM_MICBIAS("Mic Bias 2", TWL4030_REG_MICBIAS_CTL, 1, 0),
	SND_SOC_DAPM_MICBIAS("Headset Mic Bias", TWL4030_REG_MICBIAS_CTL, 2, 0),

};

static const struct snd_soc_dapm_route intercon[] = {
	/* sink,                            control,                source      */
	{"Digital L1 Playback Mixer", NULL, "DAC Left1"},
	{"Digital R1 Playback Mixer", NULL, "DAC Right1"},
	{"Digital L2 Playback Mixer", NULL, "DAC Left2"},
	{"Digital R2 Playback Mixer", NULL, "DAC Right2"},
	{"Digital Voice Playback Mixer", NULL, "DAC Voice"},

	/* Supply for the digital part (APLL) */
	{"Digital Voice Playback Mixer", NULL, "APLL Enable"},

	{"DAC Left1", NULL, "AIF Enable"},
	{"DAC Right1", NULL, "AIF Enable"},
	{"DAC Left2", NULL, "AIF Enable"},
	{"DAC Right1", NULL, "AIF Enable"},

	{"Digital R2 Playback Mixer", NULL, "AIF Enable"},
	{"Digital L2 Playback Mixer", NULL, "AIF Enable"},

	{"Analog L1 Playback Mixer", NULL, "Digital L1 Playback Mixer"},
	{"Analog R1 Playback Mixer", NULL, "Digital R1 Playback Mixer"},
	{"Analog L2 Playback Mixer", NULL, "Digital L2 Playback Mixer"},
	{"Analog R2 Playback Mixer", NULL, "Digital R2 Playback Mixer"},
	{"Analog Voice Playback Mixer", NULL, "Digital Voice Playback Mixer"},

	/* Internal playback routings */
	/* Earpiece */
	{"Earpiece Mixer", "Voice", "Analog Voice Playback Mixer"},
	{"Earpiece Mixer", "AudioL1", "Analog L1 Playback Mixer"},
	{"Earpiece Mixer", "AudioL2", "Analog L2 Playback Mixer"},
	{"Earpiece Mixer", "AudioR1", "Analog R1 Playback Mixer"},
	{"Earpiece PGA", NULL, "Earpiece Mixer"},
	/* PreDrivL */
	{"PredriveL Mixer", "Voice", "Analog Voice Playback Mixer"},
	{"PredriveL Mixer", "AudioL1", "Analog L1 Playback Mixer"},
	{"PredriveL Mixer", "AudioL2", "Analog L2 Playback Mixer"},
	{"PredriveL Mixer", "AudioR2", "Analog R2 Playback Mixer"},
	{"PredriveL PGA", NULL, "PredriveL Mixer"},
	/* PreDrivR */
	{"PredriveR Mixer", "Voice", "Analog Voice Playback Mixer"},
	{"PredriveR Mixer", "AudioR1", "Analog R1 Playback Mixer"},
	{"PredriveR Mixer", "AudioR2", "Analog R2 Playback Mixer"},
	{"PredriveR Mixer", "AudioL2", "Analog L2 Playback Mixer"},
	{"PredriveR PGA", NULL, "PredriveR Mixer"},
	/* HeadsetL */
	{"HeadsetL Mixer", "Voice", "Analog Voice Playback Mixer"},
	{"HeadsetL Mixer", "AudioL1", "Analog L1 Playback Mixer"},
	{"HeadsetL Mixer", "AudioL2", "Analog L2 Playback Mixer"},
	{"HeadsetL PGA", NULL, "HeadsetL Mixer"},
	/* HeadsetR */
	{"HeadsetR Mixer", "Voice", "Analog Voice Playback Mixer"},
	{"HeadsetR Mixer", "AudioR1", "Analog R1 Playback Mixer"},
	{"HeadsetR Mixer", "AudioR2", "Analog R2 Playback Mixer"},
	{"HeadsetR PGA", NULL, "HeadsetR Mixer"},
	/* CarkitL */
	{"CarkitL Mixer", "Voice", "Analog Voice Playback Mixer"},
	{"CarkitL Mixer", "AudioL1", "Analog L1 Playback Mixer"},
	{"CarkitL Mixer", "AudioL2", "Analog L2 Playback Mixer"},
	{"CarkitL PGA", NULL, "CarkitL Mixer"},
	/* CarkitR */
	{"CarkitR Mixer", "Voice", "Analog Voice Playback Mixer"},
	{"CarkitR Mixer", "AudioR1", "Analog R1 Playback Mixer"},
	{"CarkitR Mixer", "AudioR2", "Analog R2 Playback Mixer"},
	{"CarkitR PGA", NULL, "CarkitR Mixer"},
	/* HandsfreeL */
	{"HandsfreeL Mux", "Voice", "Analog Voice Playback Mixer"},
	{"HandsfreeL Mux", "AudioL1", "Analog L1 Playback Mixer"},
	{"HandsfreeL Mux", "AudioL2", "Analog L2 Playback Mixer"},
	{"HandsfreeL Mux", "AudioR2", "Analog R2 Playback Mixer"},
	{"HandsfreeL", "Switch", "HandsfreeL Mux"},
	{"HandsfreeL PGA", NULL, "HandsfreeL"},
	/* HandsfreeR */
	{"HandsfreeR Mux", "Voice", "Analog Voice Playback Mixer"},
	{"HandsfreeR Mux", "AudioR1", "Analog R1 Playback Mixer"},
	{"HandsfreeR Mux", "AudioR2", "Analog R2 Playback Mixer"},
	{"HandsfreeR Mux", "AudioL2", "Analog L2 Playback Mixer"},
	{"HandsfreeR", "Switch", "HandsfreeR Mux"},
	{"HandsfreeR PGA", NULL, "HandsfreeR"},
	/* Vibra */
	{"Vibra Mux", "AudioL1", "DAC Left1"},
	{"Vibra Mux", "AudioR1", "DAC Right1"},
	{"Vibra Mux", "AudioL2", "DAC Left2"},
	{"Vibra Mux", "AudioR2", "DAC Right2"},

	/* outputs */
	/* Must be always connected (for AIF and APLL) */
	{"Virtual HiFi OUT", NULL, "DAC Left1"},
	{"Virtual HiFi OUT", NULL, "DAC Right1"},
	{"Virtual HiFi OUT", NULL, "DAC Left2"},
	{"Virtual HiFi OUT", NULL, "DAC Right2"},
	/* Must be always connected (for APLL) */
	{"Virtual Voice OUT", NULL, "Digital Voice Playback Mixer"},
	/* Physical outputs */
	//#if 0  //junyeop.kim@lge.com
#if 0  //bsnoh@ubiquix : refference from GB version of Black
	{"EARPIECE", NULL, "Earpiece PGA"},
	{"PREDRIVEL", NULL, "PredriveL PGA"},
	{"PREDRIVER", NULL, "PredriveR PGA"},
#else
	{"EARPIECE", NULL, "Earpiece Mixer"},
	{"PREDRIVEL", NULL, "PredriveL Mixer"},
	{"PREDRIVER", NULL, "PredriveR Mixer"},
#endif
	{"HSOL", NULL, "HeadsetL PGA"},
	{"HSOR", NULL, "HeadsetR PGA"},
	//#if 0  //junyeop.kim@lge.com
#if 0  //bsnoh@ubiquix : refference from GB version of Black
	{"CARKITL", NULL, "CarkitL PGA"},
	{"CARKITR", NULL, "CarkitR PGA"},
#else
	{"CARKITL", NULL, "CarkitL Mixer"},
	{"CARKITR", NULL, "CarkitR Mixer"},
#endif
	{"HFL", NULL, "HandsfreeL PGA"},
	{"HFR", NULL, "HandsfreeR PGA"},
	{"Vibra Route", "Audio", "Vibra Mux"},
	{"VIBRA", NULL, "Vibra Route"},

	/* Capture path */
	/* Must be always connected (for AIF and APLL) */
	{"ADC Virtual Left1", NULL, "Virtual HiFi IN"},
	{"ADC Virtual Right1", NULL, "Virtual HiFi IN"},
	{"ADC Virtual Left2", NULL, "Virtual HiFi IN"},
	{"ADC Virtual Right2", NULL, "Virtual HiFi IN"},
	/* Physical inputs */
	{"Analog Left", "Main Mic Capture Switch", "MAINMIC"},
	{"Analog Left", "Headset Mic Capture Switch", "HSMIC"},
	{"Analog Left", "AUXL Capture Switch", "AUXL"},
	{"Analog Left", "Carkit Mic Capture Switch", "CARKITMIC"},

	{"Analog Right", "Sub Mic Capture Switch", "SUBMIC"},
	{"Analog Right", "AUXR Capture Switch", "AUXR"},

	{"ADC Physical Left", NULL, "Analog Left"},
	{"ADC Physical Right", NULL, "Analog Right"},

	{"Digimic0 Enable", NULL, "DIGIMIC0"},
	{"Digimic1 Enable", NULL, "DIGIMIC1"},

	{"DIGIMIC0", NULL, "micbias1 select"},
	{"DIGIMIC1", NULL, "micbias2 select"},

	/* TX1 Left capture path */
	{"TX1 Capture Route", "Analog", "ADC Physical Left"},
	{"TX1 Capture Route", "Digimic0", "Digimic0 Enable"},
	/* TX1 Right capture path */
	{"TX1 Capture Route", "Analog", "ADC Physical Right"},
	{"TX1 Capture Route", "Digimic0", "Digimic0 Enable"},
	/* TX2 Left capture path */
	{"TX2 Capture Route", "Analog", "ADC Physical Left"},
	{"TX2 Capture Route", "Digimic1", "Digimic1 Enable"},
	/* TX2 Right capture path */
	{"TX2 Capture Route", "Analog", "ADC Physical Right"},
	{"TX2 Capture Route", "Digimic1", "Digimic1 Enable"},

	{"ADC Virtual Left1", NULL, "TX1 Capture Route"},
	{"ADC Virtual Right1", NULL, "TX1 Capture Route"},
	{"ADC Virtual Left2", NULL, "TX2 Capture Route"},
	{"ADC Virtual Right2", NULL, "TX2 Capture Route"},

	{"ADC Virtual Left1", NULL, "AIF Enable"},
	{"ADC Virtual Right1", NULL, "AIF Enable"},
	{"ADC Virtual Left2", NULL, "AIF Enable"},
	{"ADC Virtual Right2", NULL, "AIF Enable"},

	/* Analog bypass routes */
	{"Right1 Analog Loopback", "Switch", "Analog Right"},
	{"Left1 Analog Loopback", "Switch", "Analog Left"},
	{"Right2 Analog Loopback", "Switch", "Analog Right"},
	{"Left2 Analog Loopback", "Switch", "Analog Left"},
	{"Voice Analog Loopback", "Switch", "Analog Left"},

	/* Supply for the Analog loopbacks */
	{"Right1 Analog Loopback", NULL, "FM Loop Enable"},
	{"Left1 Analog Loopback", NULL, "FM Loop Enable"},
	{"Right2 Analog Loopback", NULL, "FM Loop Enable"},
	{"Left2 Analog Loopback", NULL, "FM Loop Enable"},
	{"Voice Analog Loopback", NULL, "FM Loop Enable"},

	{"Analog R1 Playback Mixer", NULL, "Right1 Analog Loopback"},
	{"Analog L1 Playback Mixer", NULL, "Left1 Analog Loopback"},
	{"Analog R2 Playback Mixer", NULL, "Right2 Analog Loopback"},
	{"Analog L2 Playback Mixer", NULL, "Left2 Analog Loopback"},
	{"Analog Voice Playback Mixer", NULL, "Voice Analog Loopback"},

	/* Digital bypass routes */
	{"Right Digital Loopback", "Volume", "TX1 Capture Route"},
	{"Left Digital Loopback", "Volume", "TX1 Capture Route"},
	{"Voice Digital Loopback", "Volume", "TX2 Capture Route"},

	{"Digital R2 Playback Mixer", NULL, "Right Digital Loopback"},
	{"Digital L2 Playback Mixer", NULL, "Left Digital Loopback"},
	{"Digital Voice Playback Mixer", NULL, "Voice Digital Loopback"},

};

static int twl4030_add_widgets(struct snd_soc_codec *codec)
{
	struct snd_soc_dapm_context *dapm = &codec->dapm;

	DBG("\n");

	snd_soc_dapm_new_controls(dapm, twl4030_dapm_widgets,
			ARRAY_SIZE(twl4030_dapm_widgets));

	snd_soc_dapm_add_routes(dapm, intercon, ARRAY_SIZE(intercon));

	return 0;
}

static int twl4030_set_bias_level(struct snd_soc_codec *codec,
		enum snd_soc_bias_level level)
{
	struct twl4030_priv *twl4030 = snd_soc_codec_get_drvdata(codec);
	DBG("\n");

	// prime@sdcmicro.com Merge with 2.6.32 [START]
#if defined(CONFIG_PRODUCT_LGE_HUB)
	if (twl4030->is_calling)
		level = SND_SOC_BIAS_ON;

	if (codec->dapm.bias_level == level)
		return 0;
#if defined(CONFIG_HUB_AMP_WM9093)
	wm9093_ext_suspend();
#endif
	// prime@sdcmicro.com Merge with 2.6.32 [END]
#elif defined(CONFIG_PRODUCT_LGE_LU6800) || defined(CONFIG_PRODUCT_LGE_KU5900) || defined (CONFIG_PRODUCT_LGE_P970)
	//20101222 inbang.park@lge.com Wake lock for  FM Radio [START]
	DBG("\n");
	if(cur_fmradio_mode == 1)
		fmradio_is_on = 1;
	if(twl4030->is_calling ||fmradio_is_on == 1)
	{
		level = SND_SOC_BIAS_ON;
	}

	if(codec->dapm.bias_level == level || fmradio_is_on)
		//20101222 inbang.park@lge.com Wake lock for  FM Radio [END]
		return 0;

	//    DBG("state : %d is_calling : %d, fmradio_is_on: %d \n", twl4030->bypass_state,twl4030->is_calling, fmradio_is_on);
#if defined(CONFIG_HUB_AMP_WM9093)
	wm9093_ext_suspend();
#endif
#endif // #elif defined(CONFIG_PRODUCT_LGE_LU6800) || defined(CONFIG_PRODUCT_LGE_KU5900) || defined (CONFIG_PRODUCT_LGE_P970)

	switch (level) {
		case SND_SOC_BIAS_ON:
			break;
		case SND_SOC_BIAS_PREPARE:
			break;
		case SND_SOC_BIAS_STANDBY:
			if (codec->dapm.bias_level == SND_SOC_BIAS_OFF)
				twl4030_codec_enable(codec, 1);
			break;
		case SND_SOC_BIAS_OFF:
			twl4030_codec_enable(codec, 0);
			break;
	}
	codec->dapm.bias_level = level;

	return 0;
}

static void twl4030_constraints(struct twl4030_priv *twl4030,
		struct snd_pcm_substream *mst_substream)
{
	struct snd_pcm_substream *slv_substream;
	DBG("\n");

	/* Pick the stream, which need to be constrained */
	if (mst_substream == twl4030->master_substream)
		slv_substream = twl4030->slave_substream;
	else if (mst_substream == twl4030->slave_substream)
		slv_substream = twl4030->master_substream;
	else /* This should not happen.. */
		return;

	/* Set the constraints according to the already configured stream */
	snd_pcm_hw_constraint_minmax(slv_substream->runtime,
			SNDRV_PCM_HW_PARAM_RATE,
			twl4030->rate,
			twl4030->rate);

	snd_pcm_hw_constraint_minmax(slv_substream->runtime,
			SNDRV_PCM_HW_PARAM_SAMPLE_BITS,
			twl4030->sample_bits,
			twl4030->sample_bits);
	//--[[ LGE_UBIQUIX_MODIFIED_START : bsnoh@ubiquix.com - add from DCM_GB
#if 0 //I knew we need to set alsa contraints. //soyoung77.park(mono recording)
	snd_pcm_hw_constraint_minmax(slv_substream->runtime,
			SNDRV_PCM_HW_PARAM_CHANNELS,
			twl4030->channels,
			twl4030->channels);
#endif
	//--]] LGE_UBIQUIX_MODIFIED_END : bsnoh@ubiquix.com - add from DCM_GB
}

/* In case of 4 channel mode, the RX1 L/R for playback and the TX2 L/R for
 * capture has to be enabled/disabled. */
static void twl4030_tdm_enable(struct snd_soc_codec *codec, int direction,
		int enable)
{
	u8 reg, mask;

	reg = twl4030_read_reg_cache(codec, TWL4030_REG_OPTION);
	DBG("\n");

	if (direction == SNDRV_PCM_STREAM_PLAYBACK)
		mask = TWL4030_ARXL1_VRX_EN | TWL4030_ARXR1_EN;
	else
		mask = TWL4030_ATXL2_VTXL_EN | TWL4030_ATXR2_VTXR_EN;

	if (enable)
		reg |= mask;
	else
		reg &= ~mask;

	twl4030_write(codec, TWL4030_REG_OPTION, reg);
}


#if defined(CONFIG_PRODUCT_LGE_LU6800) || defined(CONFIG_PRODUCT_LGE_KU5900) || defined (CONFIG_PRODUCT_LGE_P970) /* LGE_CHANGE_S [iggikim@lge.com] 2009-08-06, audio path */
voice_mode_enum cur_twl_mode = TWL4030_AUDIO_MODE;
mic_mode_enum cur_mic_mode = TWL4030_RESTORE_MIC_MODE;	// 20100426 junyeop.kim@lge.com Add the mic mute [START_LGE]
callrec_mode_enum cur_callrec_mode = TWL4030_CALLREC_OFF_MODE;	// 20100521 junyeop.kim@lge.com call recording path [START_LGE]

int voice_get_curmode(void)
{
	DBG("\n");

	return (int)cur_twl_mode;
}

void twl4030_restore_reg_cache(void)
{
	struct snd_soc_codec* codec = snd_soc_get_codec("twl4030-codec");
	u8 *cache = codec ? codec->reg_cache : 0;
	int i;
	DBG("\n");

	if (!codec)
		return;

	for (i = TWL4030_REG_CODEC_MODE; i <= TWL4030_REG_MISC_SET_2; i++)
		twl_i2c_write_u8(TWL4030_MODULE_AUDIO_VOICE, cache[i], i);
}

void codec_delay_msec(int msec)
{
	unsigned long start;
	start = jiffies;

	while (time_before(jiffies, start+(msec*HZ)/1000))
	{
		udelay(5);
	}
}

void set_voice_table(twl_reg_type* table)
{
	int i;
	DBG("\n");
	for(i=0; table[i].irc!= TWL4030_END_SEQ; i++) {
		if(table[i].irc == TWL4030_DELAY){
			codec_delay_msec(table[i].data);
		}
		else{
			twl_i2c_write_u8(TWL4030_MODULE_AUDIO_VOICE, (u8)table[i].data, table[i].address);
		}
	}
}

#if 0	//wake lock for call
int wake_lock_on = 0;
#endif
#if defined(CONFIG_PRODUCT_LGE_LU6800) || defined(CONFIG_PRODUCT_LGE_KU5900) || defined (CONFIG_PRODUCT_LGE_P970)
//jongik2.kim 20101220 add mic2 control [start]
void twl4030_set_mic_switch(int mic)
{
	DBG("BSNOH :::::mic=%d\n",mic);

	if(mic ==  0)
	{
		set_voice_table((twl_reg_type*)&twl_mic1bias_tab[0]); /* jiwon.seo@lge.com 20100109 : ARS start noise */
		codec_delay_msec(10);  /* jiwon.seo@lge.com 20100109 : ASR start noise */
		gpio_direction_output( MIC_SEL_GPIO, 0);
		// bsnoh@ubiquix.com : 20110924
		// Changed delay time for CTS test of testAudioRecordOP(200 => 60).
		// Need to check ASR Test
		//--[[ LGE_UBIQUIX_MODIFIED_START : shyun@ubiquix.com [2011.10.06] - Confirm ASR & CTS Test.
		codec_delay_msec(50);  /* jiwon.seo@lge.com 20100109 : ASR start noise */
		//		codec_delay_msec(60);  /* jiwon.seo@lge.com 20100109 : ASR start noise */
		//--]] LGE_UBIQUIX_MODIFIED_END : shyun@ubiquix.com [2011.10.06]- Confirm ASR & CTS Test.
		//		codec_delay_msec(200);  /* jiwon.seo@lge.com 20100109 : ASR start noise */
	}
	else
		//--[[ LGE_UBIQUIX_MODIFIED_START : bsnoh@ubiquix.com - add from DCM_GB
	{//20110330 jisun.kwon
		//set_voice_table((twl_reg_type*)&twl_mic2bias_tab[0]);
		//codec_delay_msec(10);
		gpio_direction_output( MIC_SEL_GPIO, 1);
		//codec_delay_msec(10);
	}
	//--]] LGE_UBIQUIX_MODIFIED_END : bsnoh@ubiquix.com - add from DCM_GB
	return;
}
//jongik2.kim 20101220 add mic2 control [end]
#endif // #if defined(CONFIG_PRODUCT_LGE_LU6800) || defined(CONFIG_PRODUCT_LGE_KU5900) || defined (CONFIG_PRODUCT_LGE_P970)

void voice_configure_path(voice_mode_enum mode)
{
	struct snd_soc_codec* codec = snd_soc_get_codec("twl4030-codec");
	struct twl4030_priv *twl4030 = snd_soc_codec_get_drvdata(codec);

#ifdef DEBUG
	printk(KERN_INFO"voice_configure_path(at twl4030).....path = %s/ cur_twl_mode = %s\n",voice_mode_log_str(mode), voice_mode_log_str(cur_twl_mode));
#else
	printk(KERN_INFO"voice_configure_path(at twl4030).....path = %d, cur_twl_mode = %d\n",mode, cur_twl_mode);
#endif
	if(cur_twl_mode == mode)
		return;

	#if 0 /* LGE_CHANGE [jongik2.kim@lge.com] 2010-01-04, eclair sound path */
	if(mode == TWL4030_HEADSET_CALL_MODE)
	    if(get_headset_type() == 1)
			mode = TWL4030_HEADPHONE_CALL_MODE;
	#endif

//#if defined(CONFIG_PRODUCT_LGE_KU5900)	
/*[LGE_CHANGE] 20120908 pyocool.cho@lge.com "for p970" */
#if defined(CONFIG_PRODUCT_LGE_KU5900) || defined(CONFIG_PRODUCT_LGE_P970)
	if(mode != TWL4030_AUDIO_MODE)
	{
	    u8 audio_if;
	    audio_if = twl4030_read_reg_cache(codec, TWL4030_REG_AUDIO_IF);
	    twl4030_write_reg_cache(codec, TWL4030_REG_AUDIO_IF, audio_if | TWL4030_AIF_EN);
	}
#endif

    switch(mode)
    {
        case TWL4030_AUDIO_MODE :
			printk("[TWL4030]voice_configure_path::TWL4030_AUDIO_MODE..\n");
			wm9093_configure_path(0); //220110313 jisun.kwon :anti-pop noise
			twl4030_restore_reg_cache();
#if defined(CONFIG_PRODUCT_LGE_LU6800) || defined(CONFIG_PRODUCT_LGE_KU5900) || defined (CONFIG_PRODUCT_LGE_P970)
			gpio_direction_output(MIC_SEL_GPIO, 0);
#endif // #if defined(CONFIG_PRODUCT_LGE_LU6800) || defined(CONFIG_PRODUCT_LGE_KU5900) || defined (CONFIG_PRODUCT_LGE_P970)

//			headset_ramp(twl4030_socdev->card->codec, 0);		//junyeop.kim@lge.com, headset ramp disable
						     break;
	    case TWL4030_HEADSET_CALL_MODE :
			printk("[TWL4030]voice_configure_path::TWL4030_HEADSET_CALL_MODE..\n");
	    	if(call_headset_ramp == 0)
			headset_ramp(codec, 1);						/* jung.chanmin@lge.com - change headset ramp enable */
	    	set_voice_table((twl_reg_type*)&twl_headset_call_tab[0]);
#if defined(CONFIG_PRODUCT_LGE_LU6800) || defined(CONFIG_PRODUCT_LGE_KU5900) || defined (CONFIG_PRODUCT_LGE_P970)
			gpio_direction_output(MIC_SEL_GPIO, 0);
#endif // #if defined(CONFIG_PRODUCT_LGE_LU6800) || defined(CONFIG_PRODUCT_LGE_KU5900) || defined (CONFIG_PRODUCT_LGE_P970)


							 break;
	    case TWL4030_SPEAKER_CALL_MODE :
			printk("[TWL4030]voice_configure_path::TWL4030_SPEAKER_CALL_MODE..\n");
	    	if(call_headset_ramp == 1)
				headset_ramp(codec, 0);						/* jung.chanmin@lge.com - change headset ramp enable */
#if defined(CONFIG_PRODUCT_LGE_LU6800) || defined(CONFIG_PRODUCT_LGE_KU5900) || defined (CONFIG_PRODUCT_LGE_P970)
			/* 20110106 jiwon.seo@lge.com : speaker phone on pop nosie [START] */
			set_voice_table((twl_reg_type*)&twl_mic2bias_tab[0]);
			codec_delay_msec(10);
	    	gpio_direction_output(MIC_SEL_GPIO, 1);
			codec_delay_msec(10); //20110206 jisun.kwon : due to pop noise when incoming call is received 100->10
			/* 20110106 jiwon.seo@lge.com : speaker phone on pop nosie [END] */
#endif // #if defined(CONFIG_PRODUCT_LGE_LU6800) || defined(CONFIG_PRODUCT_LGE_KU5900) || defined (CONFIG_PRODUCT_LGE_P970)


#if defined(CONFIG_PRODUCT_LGE_LU6800) || defined(CONFIG_PRODUCT_LGE_KU5900) || defined (CONFIG_PRODUCT_LGE_P970)
	    	if(get_headset_type() == HUB_NONE /*&& get_dmb_status() == 0*/)
#elif defined(CONFIG_PRODUCT_LGE_HUB)
			if(get_headset_type() == HUB_NONE && get_dmb_status() == 0)
#endif
		    	set_voice_table((twl_reg_type*)&twl_speaker_call_tab[0]);
	    	else
		    	set_voice_table((twl_reg_type*)&twl_speaker_call_headset_det_tab[0]);
							 break;
	    case TWL4030_RECEIVER_CALL_MODE :
			printk("[TWL4030]voice_configure_path::TWL4030_RECEIVER_CALL_MODE..\n");
	    	if(call_headset_ramp == 1)
				headset_ramp(codec, 0);						/* jung.chanmin@lge.com - change headset ramp enable */
#if defined(CONFIG_PRODUCT_LGE_LU6800) || defined(CONFIG_PRODUCT_LGE_KU5900) || defined (CONFIG_PRODUCT_LGE_P970)
			/* 201101016 jiwon.seo@lge.com : speaker phone on pop nosie [START]  */
			set_voice_table((twl_reg_type*)&twl_mic1bias_tab[0]);
			codec_delay_msec(10);
			gpio_direction_output(MIC_SEL_GPIO, 0);
			codec_delay_msec(10);//20110206 jisun.kwon : due to pop noise when incoming call is received 100->10
			/* 20110106 jiwon.seo@lge.com : speaker phone on pop nosie  [END] */
#endif // #if defined(CONFIG_PRODUCT_LGE_LU6800) || defined(CONFIG_PRODUCT_LGE_KU5900) || defined (CONFIG_PRODUCT_LGE_P970)


#if defined(CONFIG_PRODUCT_LGE_LU6800) || defined(CONFIG_PRODUCT_LGE_KU5900) || defined (CONFIG_PRODUCT_LGE_P970)
	    	if(get_headset_type() == HUB_NONE /*&& get_dmb_status() == 0*/)
#elif defined(CONFIG_PRODUCT_LGE_HUB)
	    	if(get_headset_type() == HUB_NONE && get_dmb_status() == 0)
#endif // #if defined(CONFIG_PRODUCT_LGE_LU6800) || defined(CONFIG_PRODUCT_LGE_KU5900) || defined (CONFIG_PRODUCT_LGE_P970)
		    	set_voice_table((twl_reg_type*)&twl_receiver_call_tab[0]);
	    	else
		    	set_voice_table((twl_reg_type*)&twl_receiver_call_dmb_tab[0]);


			                 break;
	    case TWL4030_HEADPHONE_CALL_MODE :
			printk("[TWL4030]voice_configure_path::TWL4030_HEADPHONE_CALL_MODE..\n");
	       	if(call_headset_ramp == 0)
			headset_ramp(codec, 1);						/* jung.chanmin@lge.com - change headset ramp enable */
	    	set_voice_table((twl_reg_type*)&twl_headphone_call_tab[0]);
#if defined(CONFIG_PRODUCT_LGE_LU6800) || defined(CONFIG_PRODUCT_LGE_KU5900) || defined (CONFIG_PRODUCT_LGE_P970)
			gpio_direction_output(MIC_SEL_GPIO, 0);
#endif // #if defined(CONFIG_PRODUCT_LGE_LU6800) || defined(CONFIG_PRODUCT_LGE_KU5900) || defined (CONFIG_PRODUCT_LGE_P970)


						     break;
#if defined(CONFIG_PRODUCT_LGE_LU6800) || defined(CONFIG_PRODUCT_LGE_KU5900) || defined (CONFIG_PRODUCT_LGE_P970)
		case TWL4030_VT_HEADSET_CALL_MODE :
			printk("[TWL4030]voice_configure_path::TWL4030_VT_HEADSET_CALL_MODE..\n");
	    	if(call_headset_ramp == 0)
			headset_ramp(codec, 1);		//junyeop.kim@lge.com, headset ramp enable
	    	set_voice_table((twl_reg_type*)&twl_vt_headset_call_tab[0]);
			gpio_direction_output(MIC_SEL_GPIO, 0);

			DBG("TWL4030_VT_HEADSET_CALL_MODE \n");

		break;
		case TWL4030_VT_SPEAKER_CALL_MODE :
			printk("[TWL4030]voice_configure_path::TWL4030_VT_SPEAKER_CALL_MODE..\n");
			set_voice_table((twl_reg_type*)&twl_mic2bias_tab[0]);
			codec_delay_msec(10);
	    	gpio_direction_output(MIC_SEL_GPIO, 1);
			codec_delay_msec(10);
			set_voice_table((twl_reg_type*)&twl_vt_speaker_call_tab[0]);
			DBG("TWL4030_VT_SPEAKER_CALL_MODE \n");
		break;
		case TWL4030_VT_RECEIVER_CALL_MODE :
			printk("[TWL4030]voice_configure_path::TWL4030_VT_RECEIVER_CALL_MODE..\n");
			set_voice_table((twl_reg_type*)&twl_mic1bias_tab[0]);
			codec_delay_msec(10);
			gpio_direction_output(MIC_SEL_GPIO, 0);
			codec_delay_msec(10);
			set_voice_table((twl_reg_type*)&twl_vt_receiver_call_tab[0]);
			DBG("TWL4030_VT_RECEIVER_CALL_MODE \n");
		break;


	    case TWL4030_VT_HEADPHONE_CALL_MODE :
			printk("[TWL4030]voice_configure_path::TWL4030_VT_HEADPHONE_CALL_MODE..\n");
	       	if(call_headset_ramp == 0)
			headset_ramp(codec, 1);		//junyeop.kim@lge.com, headset ramp enable
	    	set_voice_table((twl_reg_type*)&twl_vt_headphone_call_tab[0]);
			gpio_direction_output(MIC_SEL_GPIO, 0);		//need to check scenario
			DBG("TWL4030_VT_HEADPHONE_CALL_MODE \n");
			 break;



		case TWL4030_VT_BT_CALL_MODE://jisun.kwon
			printk("[TWL4030]voice_configure_path::TWL4030_VT_BT_CALL_MODE..\n");
			set_voice_table((twl_reg_type*)&twl_vt_bt_call_tab[0]);
			DBG("TWL4030_VT_BT_CALL_MODE \n");
		break;
#endif // #if defined(CONFIG_PRODUCT_LGE_LU6800) || defined(CONFIG_PRODUCT_LGE_KU5900) || defined (CONFIG_PRODUCT_LGE_P970)
		default :
			     break;
    }

    cur_twl_mode = mode;

	if(cur_mic_mode == TWL4030_OFF_MIC_MODE)
	{
		DBG("wake unlock : twl4030");
		set_voice_table((twl_reg_type*)&twl_mic_off_tab[0]);
	}

	if(cur_twl_mode == TWL4030_AUDIO_MODE)
		twl4030->is_calling =0;
	else
		twl4030->is_calling =1;
}

EXPORT_SYMBOL_GPL(voice_get_curmode);
EXPORT_SYMBOL_GPL(voice_configure_path);
#if defined(CONFIG_PRODUCT_LGE_LU6800) || defined(CONFIG_PRODUCT_LGE_KU5900) || defined (CONFIG_PRODUCT_LGE_P970)
EXPORT_SYMBOL_GPL(twl4030_set_mic_switch); //jongik2.kim 20101220 add mic2 control
#endif // #if defined(CONFIG_PRODUCT_LGE_LU6800) || defined(CONFIG_PRODUCT_LGE_KU5900) || defined (CONFIG_PRODUCT_LGE_P970)
#else
static struct snd_soc_device *twl4030_socdev;
#endif /* LGE_CHANGE_E [iggikim@lge.com]*/


// 20100426 junyeop.kim@lge.com Add the mic mute [START_LGE]
int mic_get_curmode(void)
{
	DBG("\n");

    return (int)cur_mic_mode;
}

void mic_configure_path(voice_mode_enum mode)
{
	DBG("mode=%d cur_mode=%d\n", mode, cur_twl_mode);
//    struct twl4030_priv *twl4030 = twl4030_socdev->codec->private_data;
    if(cur_mic_mode == mode)
		return;

    switch(mode){
        case TWL4030_OFF_MIC_MODE :	set_voice_table((twl_reg_type*)&twl_mic_off_tab[0]);
						     break;
	    case TWL4030_RESTORE_MIC_MODE :
	    	switch(cur_twl_mode) {
		        case TWL4030_AUDIO_MODE :      twl4030_restore_reg_cache();
							     break;
		    	case TWL4030_HEADSET_CALL_MODE :  set_voice_table((twl_reg_type*)&twl_headset_mic_tab[0]);
								 break;
			    case TWL4030_SPEAKER_CALL_MODE :  set_voice_table((twl_reg_type*)&twl_speaker_mic_tab[0]);
					#if (defined(CONFIG_PRODUCT_LGE_LU6800) || defined(CONFIG_PRODUCT_LGE_KU5900) || defined (CONFIG_PRODUCT_LGE_P970))
			    	if(get_headset_type() == HUB_NONE /*&& get_dmb_status() == 0*/)
						twl_i2c_write_u8(TWL4030_MODULE_AUDIO_VOICE, 0x02, 0x04); //mic bias on, only main
				    #else	// HUB
			    	if(get_headset_type() == HUB_NONE && get_dmb_status() == 0)
				         twl_i2c_write_u8(TWL4030_MODULE_AUDIO_VOICE, 0x01, 0x04); //mic bias on, only main
				    #endif
			    	else
				         twl_i2c_write_u8(TWL4030_MODULE_AUDIO_VOICE, 0x03, 0x04); //mic bias on, external & main
								 break;
			    case TWL4030_RECEIVER_CALL_MODE : set_voice_table((twl_reg_type*)&twl_receiver_mic_tab[0]);
					#if (defined(CONFIG_PRODUCT_LGE_LU6800) || defined(CONFIG_PRODUCT_LGE_KU5900) || defined (CONFIG_PRODUCT_LGE_P970))
			    	if(get_headset_type() == HUB_NONE /*&& get_dmb_status() == 0*/)
				         twl_i2c_write_u8(TWL4030_MODULE_AUDIO_VOICE, 0x01, 0x04); //mic bias on, only main
				    #else
			    	if(get_headset_type() == HUB_NONE && get_dmb_status() == 0)
				         twl_i2c_write_u8(TWL4030_MODULE_AUDIO_VOICE, 0x01, 0x04); //mic bias on, only main
					#endif
			    	else
				         twl_i2c_write_u8(TWL4030_MODULE_AUDIO_VOICE, 0x03, 0x04); //mic bias on, external & main
				                 break;
			    case TWL4030_HEADPHONE_CALL_MODE : set_voice_table((twl_reg_type*)&twl_receiver_mic_tab[0]);	//use the internal mic
							     break;
#if defined(CONFIG_PRODUCT_LGE_LU6800) || defined(CONFIG_PRODUCT_LGE_KU5900) || defined (CONFIG_PRODUCT_LGE_P970)
				case TWL4030_VT_BT_CALL_MODE :
						break;
				case TWL4030_VT_HEADSET_CALL_MODE :  set_voice_table((twl_reg_type*)&twl_headset_mic_tab[0]);
				break;
				case TWL4030_VT_SPEAKER_CALL_MODE : set_voice_table((twl_reg_type*)&twl_speaker_mic_tab[0]);
					if(get_headset_type() == HUB_NONE /*&& get_dmb_status() == 0*/)
				         twl_i2c_write_u8(TWL4030_MODULE_AUDIO_VOICE, 0x02, 0x04); //mic bias on, only main
					else
				         twl_i2c_write_u8(TWL4030_MODULE_AUDIO_VOICE, 0x03, 0x04); //mic bias on, external & main

				break;

				case TWL4030_VT_RECEIVER_CALL_MODE :set_voice_table((twl_reg_type*)&twl_receiver_mic_tab[0]);
					if(get_headset_type() == HUB_NONE /*&& get_dmb_status() == 0*/)
				         twl_i2c_write_u8(TWL4030_MODULE_AUDIO_VOICE, 0x01, 0x04); //mic bias on, only main
					else
				         twl_i2c_write_u8(TWL4030_MODULE_AUDIO_VOICE, 0x03, 0x04); //mic bias on, external & main
						break;
				case TWL4030_VT_HEADPHONE_CALL_MODE :set_voice_table((twl_reg_type*)&twl_receiver_mic_tab[0]);	//use the internal mic
						break;

#endif // #if defined(CONFIG_PRODUCT_LGE_LU6800) || defined(CONFIG_PRODUCT_LGE_KU5900) || defined (CONFIG_PRODUCT_LGE_P970)
				default :
				     break;
	    	}
			 break;
#if 0
	    case TWL4030_SPEAKER_MIC_MODE :  set_voice_table((twl_reg_type*)&twl_speaker_mic_tab[0]);
							 break;
	    case TWL4030_HANDSET_MIC_MODE :  set_voice_table((twl_reg_type*)&twl_receiver_mic_tab[0]);
							 break;
#endif
		default :
			     break;
    }
    cur_mic_mode = mode;
#if 0
	if(cur_twl_mode == TWL4030_AUDIO_MODE)
		twl4030->is_calling =0;
	else
		twl4030->is_calling =1;
#endif
}

EXPORT_SYMBOL_GPL(mic_get_curmode);
EXPORT_SYMBOL_GPL(mic_configure_path);

// 20100426 junyeop.kim@lge.com Add the mic mute [END_LGE]

// 20100521 junyeop.kim@lge.com call recording path [START_LGE]
int callrec_get_curmode(void)
{
	DBG("\n");

    return (int)cur_callrec_mode;
}

void callrec_configure_path(callrec_mode_enum mode)
{
	DBG("\n");
#if 0	//junyeop.kim@lge.com, not use call recording
	printk("[LUCKYJUN77]callrec_configure_path : %d\n", mode);
//    struct twl4030_priv *twl4030 = twl4030_socdev->codec->private_data;
    if(cur_callrec_mode == mode)
		return;

    switch(mode){
        case TWL4030_CALLREC_BUILTIN_MODE : set_voice_table((twl_reg_type*)&twl_builtin_callrec_tab[0]);
        		break;
        case TWL4030_CALLREC_HEADSET_MODE :  set_voice_table((twl_reg_type*)&twl_headset_callrec_tab[0]);
        		break;

	    case TWL4030_CALLREC_OFF_MODE :
	    	switch(cur_twl_mode) {
		        case TWL4030_AUDIO_MODE :      twl4030_restore_reg_cache();
							     break;
		    	case TWL4030_HEADSET_CALL_MODE :  set_voice_table((twl_reg_type*)&twl_headset_call_tab[0]);
								 break;
			    case TWL4030_SPEAKER_CALL_MODE :  set_voice_table((twl_reg_type*)&twl_speaker_call_tab[0]);
								 break;
			    case TWL4030_RECEIVER_CALL_MODE : set_voice_table((twl_reg_type*)&twl_receiver_call_tab[0]);
				                 break;
			    case TWL4030_HEADPHONE_CALL_MODE : set_voice_table((twl_reg_type*)&twl_headphone_call_tab[0]);
							     break;
				default :
				     break;
	    	}
			 break;
		default :
			     break;
    }
    cur_callrec_mode = mode;
#endif
}

EXPORT_SYMBOL_GPL(callrec_get_curmode);
EXPORT_SYMBOL_GPL(callrec_configure_path);
// 20100521 junyeop.kim@lge.com call recording path [END_LGE]

static int twl4030_startup(struct snd_pcm_substream *substream,
			   struct snd_soc_dai *dai)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_codec *codec = rtd->codec;
	struct twl4030_priv *twl4030 = snd_soc_codec_get_drvdata(codec);
	DBG("\n");

	int use256FS = 0;

	substream->use256FS = use256FS;
	twl4030->extClock += substream->use256FS;


	snd_pcm_hw_constraint_msbits(substream->runtime, 0, 32, 24);
	if (twl4030->master_substream) {
		twl4030->slave_substream = substream;
		/* The DAI has one configuration for playback and capture, so
		 * if the DAI has been already configured then constrain this
		 * substream to match it. */
		if (twl4030->configured)
			twl4030_constraints(twl4030, twl4030->master_substream);
	} else {
		if (!(twl4030_read_reg_cache(codec, TWL4030_REG_CODEC_MODE) &
			TWL4030_OPTION_1)) {
			/* In option2 4 channel is not supported, set the
			 * constraint for the first stream for channels, the
			 * second stream will 'inherit' this cosntraint */
			snd_pcm_hw_constraint_minmax(substream->runtime,
						SNDRV_PCM_HW_PARAM_CHANNELS,
//--[[ LGE_UBIQUIX_MODIFIED_START : bsnoh@ubiquix.com - add from DCM_GB
						1, 2);
//--]] LGE_UBIQUIX_MODIFIED_END : bsnoh@ubiquix.com - add from DCM_GB
		}
		twl4030->master_substream = substream;
	}

	return 0;
}

static void twl4030_shutdown(struct snd_pcm_substream *substream,
				struct snd_soc_dai *dai)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_codec *codec = rtd->codec;
	struct twl4030_priv *twl4030 = snd_soc_codec_get_drvdata(codec);
	DBG("\n");

	if (twl4030->master_substream == substream)
		twl4030->master_substream = twl4030->slave_substream;

	twl4030->slave_substream = NULL;

	/* If all streams are closed, or the remaining stream has not yet
	 * been configured than set the DAI as not configured. */
	if (!twl4030->master_substream)
		twl4030->configured = 0;
	 else if (!twl4030->master_substream->runtime->channels)
		twl4030->configured = 0;

	 /* If the closing substream had 4 channel, do the necessary cleanup */
	if (substream->runtime->channels == 4)
		twl4030_tdm_enable(codec, substream->stream, 0);
}

static int twl4030_hw_params(struct snd_pcm_substream *substream,
								   struct snd_pcm_hw_params *params,
								   struct snd_soc_dai *dai)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_codec *codec = rtd->codec;
	struct twl4030_priv *twl4030 = snd_soc_codec_get_drvdata(codec);
	u8 mode, old_mode, format, old_format;

	unsigned int f_headset_ramp=0x0;	/* LGE_CHANGE_S :  2012-07-16, gt.kim@lge.com  TD115914 Issue patch. 	*/
	DBG("\n");

	 /* If the substream has 4 channel, do the necessary setup */
	if (params_channels(params) == 4) {
		format = twl4030_read_reg_cache(codec, TWL4030_REG_AUDIO_IF);
		mode = twl4030_read_reg_cache(codec, TWL4030_REG_CODEC_MODE);

		/* Safety check: are we in the correct operating mode and
		 * the interface is in TDM mode? */
		if ((mode & TWL4030_OPTION_1) &&
		    ((format & TWL4030_AIF_FORMAT) == TWL4030_AIF_FORMAT_TDM))
			twl4030_tdm_enable(codec, substream->stream, 1);
		else
			return -EINVAL;
	}

	if (twl4030->configured)
		/* Ignoring hw_params for already configured DAI */
		return 0;

	/* bit rate */
	old_mode = twl4030_read_reg_cache(codec,
			TWL4030_REG_CODEC_MODE) & ~TWL4030_CODECPDZ;
	mode = old_mode & ~TWL4030_APLL_RATE;

	switch (params_rate(params)) {
	case 8000:
		mode |= TWL4030_APLL_RATE_8000;
		break;
	case 11025:
		mode |= TWL4030_APLL_RATE_11025;
		break;
	case 12000:
		mode |= TWL4030_APLL_RATE_12000;
		break;
	case 16000:
		mode |= TWL4030_APLL_RATE_16000;
		break;
	case 22050:
		mode |= TWL4030_APLL_RATE_22050;
		break;
	case 24000:
		mode |= TWL4030_APLL_RATE_24000;
		break;
	case 32000:
		mode |= TWL4030_APLL_RATE_32000;
		break;
	case 44100:
		mode |= TWL4030_APLL_RATE_44100;
		break;
	case 48000:
		mode |= TWL4030_APLL_RATE_48000;
		break;
	case 96000:
		mode |= TWL4030_APLL_RATE_96000;
		break;
	default:
		printk(KERN_ERR "TWL4030 hw params: unknown rate %d\n",
			params_rate(params));
		return -EINVAL;
	}

	/* sample size */
	old_format = twl4030_read_reg_cache(codec, TWL4030_REG_AUDIO_IF);
	format = old_format;
	format &= ~TWL4030_DATA_WIDTH;
    	//DBG("\n");

	switch (params_format(params)) {
	case SNDRV_PCM_FORMAT_S16_LE:
		format |= TWL4030_DATA_WIDTH_16S_16W;
		break;
	case SNDRV_PCM_FORMAT_S24_LE:
		format |= TWL4030_DATA_WIDTH_32S_24W;
		break;
	default:
		printk(KERN_ERR "TWL4030 hw params: unknown format %d\n",
			params_format(params));
		return -EINVAL;
	}

	if (format != old_format || mode != old_mode) {
// prime@sdcmicro.com Protection against noise when headset is plugged in/out [START]
#if defined(CONFIG_PRODUCT_LGE_LU6800) || defined(CONFIG_PRODUCT_LGE_KU5900) || defined (CONFIG_PRODUCT_LGE_P970)

/* LGE_CHANGE_S :  2012-07-16, gt.kim@lge.com  TD115914 Issue patch. 	*/
#if 1
		if(twl4030->is_calling)
			f_headset_ramp=call_headset_ramp;

		if (twl4030->codec_powered && f_headset_ramp == 0)
#else	// original code..
		if (twl4030->codec_powered && call_headset_ramp == 0)
#endif
/* LGE_CHANGE_E :  2012-07-16, gt.kim@lge.com,     */

#else
		if (twl4030->codec_powered)
#endif
		{
// prime@sdcmicro.com Protection against noise when headset is plugged in/out [END]
			/*
			 * If the codec is powered, than we need to toggle the
			 * codec power.
			 */
			twl4030_codec_enable(codec, 0);
			twl4030_write(codec, TWL4030_REG_CODEC_MODE, mode);
			twl4030_write(codec, TWL4030_REG_AUDIO_IF, format);
			twl4030_codec_enable(codec, 1);
		} else {
			twl4030_write(codec, TWL4030_REG_CODEC_MODE, mode);
			twl4030_write(codec, TWL4030_REG_AUDIO_IF, format);
		}
	}

	/* Store the important parameters for the DAI configuration and set
	 * the DAI as configured */
	twl4030->configured = 1;
	twl4030->rate = params_rate(params);
	twl4030->sample_bits = hw_param_interval(params,
					SNDRV_PCM_HW_PARAM_SAMPLE_BITS)->min;
	twl4030->channels = params_channels(params);

	/* If both playback and capture streams are open, and one of them
	 * is setting the hw parameters right now (since we are here), set
	 * constraints to the other stream to match the current one. */
	if (twl4030->slave_substream)
		twl4030_constraints(twl4030, substream);

	return 0;
}

/* tbd add twl4030_hw_prepare */

static int twl4030_set_dai_sysclk(struct snd_soc_dai *codec_dai,
		int clk_id, unsigned int freq, int dir)
{
	struct snd_soc_codec *codec = codec_dai->codec;
	struct twl4030_priv *twl4030 = snd_soc_codec_get_drvdata(codec);
	DBG("\n");

	switch (freq) {
	case 19200000:
	case 26000000:
	case 38400000:
		break;
	default:
		dev_err(codec->dev, "Unsupported APLL mclk: %u\n", freq);
		return -EINVAL;
	}

	if ((freq / 1000) != twl4030->sysclk) {
		dev_err(codec->dev,
			"Mismatch in APLL mclk: %u (configured: %u)\n",
			freq, twl4030->sysclk * 1000);
		return -EINVAL;
	}

	return 0;
}

int twl4030_set_ext_clock(struct snd_soc_codec *codec, int enable)
{

	u8 old_format, format;

	/* get format */
	old_format = twl4030_read_reg_cache(codec, TWL4030_REG_AUDIO_IF);
	DBG("\n");

	if (enable)
		format = old_format | TWL4030_CLK256FS_EN;
	else
		format = old_format & ~TWL4030_CLK256FS_EN;

#if 0	//junyeop.kim@lge.com, ti original
	if (format != old_format) {

		/* clear CODECPDZ before changing format (codec requirement) */
		twl4030_codec_enable(codec, 0);

		/* change format */
		twl4030_write(codec, TWL4030_REG_AUDIO_IF, format);

		/* set CODECPDZ afterwards */
		twl4030_codec_enable(codec, 1);
	}
#else
	if (format != old_format) {
		if(call_headset_ramp == 1)
		{
			/* change format */
			twl4030_write(codec, TWL4030_REG_AUDIO_IF, format);
		}
		else
		{
			/* clear CODECPDZ before changing format (codec requirement) */
			twl4030_codec_enable(codec, 0);

			/* change format */
			twl4030_write(codec, TWL4030_REG_AUDIO_IF, format);

			/* set CODECPDZ afterwards */
			twl4030_codec_enable(codec, 1);
		}
	}


#endif

	return 0;
}
EXPORT_SYMBOL_GPL(twl4030_set_ext_clock);

static int twl4030_set_dai_fmt(struct snd_soc_dai *codec_dai,
			     unsigned int fmt)
{
	struct snd_soc_codec *codec = codec_dai->codec;
	struct twl4030_priv *twl4030 = snd_soc_codec_get_drvdata(codec);
	u8 old_format, format;
	int use256FS = 0;

	/* get format */
	old_format = twl4030_read_reg_cache(codec, TWL4030_REG_AUDIO_IF);
	format = old_format;
	DBG("\n");

	/* set master/slave audio interface */
	switch (fmt & SND_SOC_DAIFMT_MASTER_MASK) {
	case SND_SOC_DAIFMT_CBM_CFM:
		format &= ~(TWL4030_AIF_SLAVE_EN);
		format &= ~(TWL4030_CLK256FS_EN);
		break;
	case SND_SOC_DAIFMT_CBS_CFS:
		format |= TWL4030_AIF_SLAVE_EN;
		format |= TWL4030_CLK256FS_EN;
		use256FS = 1;
		break;
	default:
		return -EINVAL;
	}

	/* interface format */
	format &= ~TWL4030_AIF_FORMAT;
	switch (fmt & SND_SOC_DAIFMT_FORMAT_MASK) {
	case SND_SOC_DAIFMT_I2S:
		format |= TWL4030_AIF_FORMAT_CODEC;
		break;
	case SND_SOC_DAIFMT_DSP_A:
		format |= TWL4030_AIF_FORMAT_TDM;
		break;
	default:
		return -EINVAL;
	}

	if (format != old_format) {
// prime@sdcmicro.com Protection against noise when headset is plugged in/out [START]
#if defined(CONFIG_PRODUCT_LGE_LU6800) || defined(CONFIG_PRODUCT_LGE_KU5900) || defined (CONFIG_PRODUCT_LGE_P970)
		if (twl4030->codec_powered && call_headset_ramp == 0) {
#else
		if (twl4030->codec_powered) {
#endif
// prime@sdcmicro.com Protection against noise when headset is plugged in/out [END]
			/*
			 * If the codec is powered, than we need to toggle the
			 * codec power.
			 */
			twl4030_codec_enable(codec, 0);
			twl4030_write(codec, TWL4030_REG_AUDIO_IF, format);
			twl4030_codec_enable(codec, 1);
		} else {
			twl4030_write(codec, TWL4030_REG_AUDIO_IF, format);
		}
	}
	return twl4030_set_ext_clock(codec, use256FS | twl4030->extClock);

}

static int twl4030_clock_startup(struct snd_pcm_substream *substream,
		struct snd_soc_dai *dai)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_codec *codec = rtd->codec;
	struct twl4030_priv *twl4030 = snd_soc_codec_get_drvdata(codec);

	int use256FS = 1;
	DBG("\n");

	/* item->use256FS = use256FS; */
	substream->use256FS = use256FS;

	/*mutex_lock(&twl4030->mutex); */
	/* list_add_tail(&item->started, &twl4030->started_list); */
	twl4030->extClock += substream->use256FS;

	if (twl4030->master_substream) {
		twl4030->slave_substream = substream;
		/* The DAI has one configuration for playback and capture, so
		 * if the DAI has been already configured then constrain this
		 * substream to match it. */
		if (twl4030->configured)
			twl4030_constraints(twl4030, twl4030->master_substream);
	} else {
		if (!(twl4030_read_reg_cache(codec, TWL4030_REG_CODEC_MODE) &
		TWL4030_OPTION_1)) {
			/* In option2 4 channel is not supported, set the
			 * constraint for the first stream for channels, the
			 * second stream will 'inherit' this cosntraint */
			snd_pcm_hw_constraint_minmax(substream->runtime,
					SNDRV_PCM_HW_PARAM_CHANNELS,
					2, 2);
		}
		twl4030->master_substream = substream;
	}


#ifdef DOCUMENTATION
	return twl4030_new_substream(twl4030, substream, 1);
#endif
	return 0;
}

int twl4030_get_clock_divisor(struct snd_soc_codec *codec,
		   struct snd_pcm_hw_params *params)
{
	/* struct twl4030_priv *twl4030 = codec->private_data; */
	struct twl4030_priv *twl4030 = snd_soc_codec_get_drvdata(codec);
	int clock, divisor;

	/* clock = params_rate(&twl4030->params) * 256; */
	clock = twl4030->rate * 256;
	divisor = clock / params_rate(params);
	divisor /= params_channels(params);
	DBG("\n");

	switch (params_format(params)) {
	case SNDRV_PCM_FORMAT_U8:
	case SNDRV_PCM_FORMAT_S8:
		divisor /= 8;
		break;
	case SNDRV_PCM_FORMAT_S16_LE:
		divisor /= 16;
		break;
	case SNDRV_PCM_FORMAT_S24_LE:
		divisor /= 24;
		break;
	default:
		printk(KERN_ERR "TWL4030 get_clock_divisor: unknown format %d\n",
			params_format(params));
		return -EINVAL;
	}

	return divisor;
}
EXPORT_SYMBOL_GPL(twl4030_get_clock_divisor);


static int twl4030_clock_hw_params(struct snd_pcm_substream *substream,
		struct snd_pcm_hw_params *params, struct snd_soc_dai *dai)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_codec *codec = rtd->codec;
	struct twl4030_priv *twl4030 = snd_soc_codec_get_drvdata(codec);

	int rval = 0;

	u8 old_mode, mode;
	DBG("\n");

	mutex_lock(&twl4030->mutex);

	/* rval = twl4030_set_rate(codec, params); */

	/* bit rate */
	old_mode = twl4030_read_reg_cache(codec,
			TWL4030_REG_CODEC_MODE) & ~TWL4030_CODECPDZ;
	mode = old_mode & ~TWL4030_APLL_RATE;

	switch (params_rate(params)) {
	case 8000:
		mode |= TWL4030_APLL_RATE_8000;
		break;
	case 11025:
		mode |= TWL4030_APLL_RATE_11025;
		break;
	case 12000:
		mode |= TWL4030_APLL_RATE_12000;
		break;
	case 16000:
		mode |= TWL4030_APLL_RATE_16000;
		break;
	case 22050:
		mode |= TWL4030_APLL_RATE_22050;
		break;
	case 24000:
		mode |= TWL4030_APLL_RATE_24000;
		break;
	case 32000:
		mode |= TWL4030_APLL_RATE_32000;
		break;
	case 44100:
		mode |= TWL4030_APLL_RATE_44100;
		break;
	case 48000:
		mode |= TWL4030_APLL_RATE_48000;
		break;
	case 96000:
		mode |= TWL4030_APLL_RATE_96000;
		break;
	default:
		printk(KERN_ERR "TWL4030 hw params: unknown rate %d\n",
			params_rate(params));
		rval = -EINVAL;
	}

	/* See if we are a multiple of the current FS. If so, then still OK. */
	if (rval) {
		/* find the current FS ..! */

		int divisor = twl4030_get_clock_divisor(codec, params);
		/* int clock = params_rate(&twl4030->params) * 256; */
		int clock = twl4030->rate  * 256;
		int remainder = clock % params_rate(params);

		if (remainder == 0 && divisor <= 256)
			rval = 0;
	}

	/* If any other streams are currently open, and one of them
	 * is setting the hw parameters right now (since we are here), set
	 * constraints to the other stream(s) to match the current one. */
	/* twl4030_constraints(twl4030); */

	twl4030_constraints(twl4030, substream);

	mutex_unlock(&twl4030->mutex);

	return rval;
}

static int twl4030_clock_set_dai_fmt(struct snd_soc_dai *codec_dai,
		unsigned int fmt)
{
	struct snd_soc_codec *codec = codec_dai->codec;
	DBG("\n");

	return twl4030_set_ext_clock(codec, 1);
}

static int twl4030_set_tristate(struct snd_soc_dai *dai, int tristate)
{
	struct snd_soc_codec *codec = dai->codec;
	u8 reg = twl4030_read_reg_cache(codec, TWL4030_REG_AUDIO_IF);
	DBG("\n");

	if (tristate)
		reg |= TWL4030_AIF_TRI_EN;
	else
		reg &= ~TWL4030_AIF_TRI_EN;

	return twl4030_write(codec, TWL4030_REG_AUDIO_IF, reg);
}

/* In case of voice mode, the RX1 L(VRX) for downlink and the TX2 L/R
 * (VTXL, VTXR) for uplink has to be enabled/disabled. */
static void twl4030_voice_enable(struct snd_soc_codec *codec, int direction,
				int enable)
{
	u8 reg, mask;
	printk("[TWL4030]twl4030_voice_enable(%d)..\n",direction);

	reg = twl4030_read_reg_cache(codec, TWL4030_REG_OPTION);
	DBG("\n");

	if (direction == SNDRV_PCM_STREAM_PLAYBACK)
		mask = TWL4030_ARXL1_VRX_EN;
	else
		mask = TWL4030_ATXL2_VTXL_EN | TWL4030_ATXR2_VTXR_EN;

	if (enable)
		reg |= mask;
	else
		reg &= ~mask;

	twl4030_write(codec, TWL4030_REG_OPTION, reg);
}

static int twl4030_voice_startup(struct snd_pcm_substream *substream,
		struct snd_soc_dai *dai)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_codec *codec = rtd->codec;
	struct twl4030_priv *twl4030 = snd_soc_codec_get_drvdata(codec);
	u8 mode;
	DBG("\n");

	/* If the system master clock is not 26MHz, the voice PCM interface is
	 * not available.
	 */
	if (twl4030->sysclk != 26000) {
		dev_err(codec->dev, "The board is configured for %u Hz, while"
			"the Voice interface needs 26MHz APLL mclk\n",
			twl4030->sysclk * 1000);
		return -EINVAL;
	}

	/* If the codec mode is not option2, the voice PCM interface is not
	 * available.
	 */
	mode = twl4030_read_reg_cache(codec, TWL4030_REG_CODEC_MODE)
		& TWL4030_OPT_MODE;

	if (mode != TWL4030_OPTION_2) {
		printk(KERN_ERR "TWL4030 voice startup: "
			"the codec mode is not option2\n");
		return -EINVAL;
	}

	return 0;
}

static void twl4030_voice_shutdown(struct snd_pcm_substream *substream,
				struct snd_soc_dai *dai)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_codec *codec = rtd->codec;
	DBG("\n");

	/* Enable voice digital filters */
	twl4030_voice_enable(codec, substream->stream, 0);
}

static int twl4030_voice_hw_params(struct snd_pcm_substream *substream,
		struct snd_pcm_hw_params *params, struct snd_soc_dai *dai)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_codec *codec = rtd->codec;
	struct twl4030_priv *twl4030 = snd_soc_codec_get_drvdata(codec);
	u8 old_mode, mode;
	DBG("\n");

	/* Enable voice digital filters */
	twl4030_voice_enable(codec, substream->stream, 1);

	/* bit rate */
	old_mode = twl4030_read_reg_cache(codec, TWL4030_REG_CODEC_MODE)
		& ~(TWL4030_CODECPDZ);
	mode = old_mode;

	switch (params_rate(params)) {
	case 8000:
		mode &= ~(TWL4030_SEL_16K);
		break;
	case 16000:
		mode |= TWL4030_SEL_16K;
		break;
	default:
		printk(KERN_ERR "TWL4030 voice hw params: unknown rate %d\n",
			params_rate(params));
		return -EINVAL;
	}

	if (mode != old_mode) {
		if (twl4030->codec_powered) {
			/*
			 * If the codec is powered, than we need to toggle the
			 * codec power.
			 */
			twl4030_codec_enable(codec, 0);
			twl4030_write(codec, TWL4030_REG_CODEC_MODE, mode);
			twl4030_codec_enable(codec, 1);
		} else {
			twl4030_write(codec, TWL4030_REG_CODEC_MODE, mode);
		}
	}

	return 0;
}

static int twl4030_voice_set_dai_sysclk(struct snd_soc_dai *codec_dai,
		int clk_id, unsigned int freq, int dir)
{
	struct snd_soc_codec *codec = codec_dai->codec;
	struct twl4030_priv *twl4030 = snd_soc_codec_get_drvdata(codec);
	DBG("\n");

	if (freq != 26000000) {
		dev_err(codec->dev, "Unsupported APLL mclk: %u, the Voice"
			"interface needs 26MHz APLL mclk\n", freq);
		return -EINVAL;
	}
	if ((freq / 1000) != twl4030->sysclk) {
		dev_err(codec->dev,
			"Mismatch in APLL mclk: %u (configured: %u)\n",
			freq, twl4030->sysclk * 1000);
		return -EINVAL;
	}
	return 0;
}

static int twl4030_voice_set_dai_fmt(struct snd_soc_dai *codec_dai,
		unsigned int fmt)
{
	struct snd_soc_codec *codec = codec_dai->codec;
	struct twl4030_priv *twl4030 = snd_soc_codec_get_drvdata(codec);
	int use256FS = 0;
	u8 old_format, format;
	DBG("\n");

	/* get format */
	old_format = twl4030_read_reg_cache(codec, TWL4030_REG_VOICE_IF);
	format = old_format;

	/* set master/slave audio interface */
	switch (fmt & SND_SOC_DAIFMT_MASTER_MASK) {
	case SND_SOC_DAIFMT_CBM_CFM:
		format &= ~(TWL4030_VIF_SLAVE_EN);
		break;
	case SND_SOC_DAIFMT_CBS_CFS:
		format |= TWL4030_VIF_SLAVE_EN;
		use256FS = 1;
		break;
	default:
		return -EINVAL;
	}

	/* clock inversion */
	switch (fmt & SND_SOC_DAIFMT_INV_MASK) {
	case SND_SOC_DAIFMT_IB_NF:
		format &= ~(TWL4030_VIF_FORMAT);
		break;
	case SND_SOC_DAIFMT_NB_IF:
		format |= TWL4030_VIF_FORMAT;
		break;
	default:
		return -EINVAL;
	}

	if (format != old_format) {
		if (twl4030->codec_powered) {
			/*
			 * If the codec is powered, than we need to toggle the
			 * codec power.
			 */
			twl4030_codec_enable(codec, 0);
			twl4030_write(codec, TWL4030_REG_VOICE_IF, format);
			twl4030_codec_enable(codec, 1);
		} else {
			twl4030_write(codec, TWL4030_REG_VOICE_IF, format);
		}
	}

	return twl4030_set_ext_clock(codec, use256FS | twl4030->extClock);
}

static int twl4030_voice_set_tristate(struct snd_soc_dai *dai, int tristate)
{
	struct snd_soc_codec *codec = dai->codec;
	u8 reg = twl4030_read_reg_cache(codec, TWL4030_REG_VOICE_IF);
	DBG("\n");

	if (tristate)
		reg |= TWL4030_VIF_TRI_EN;
	else
		reg &= ~TWL4030_VIF_TRI_EN;

	return twl4030_write(codec, TWL4030_REG_VOICE_IF, reg);
}

static int twl4030_hw_free(struct snd_pcm_substream *substream,
		struct snd_soc_dai *dai)
{
	DBG("\n");

	/* the original code assumes config lists, so for now,
	   just return */
	return 0;
}


#define TWL4030_RATES	 (SNDRV_PCM_RATE_8000_48000)
#define TWL4030_FORMATS	 (SNDRV_PCM_FMTBIT_S16_LE | SNDRV_PCM_FORMAT_S24_LE)

static struct snd_soc_dai_ops twl4030_dai_hifi_ops = {
	.startup	= twl4030_startup,
	.shutdown	= twl4030_shutdown,
	.hw_params	= twl4030_hw_params,
	.set_sysclk	= twl4030_set_dai_sysclk,
	.set_fmt	= twl4030_set_dai_fmt,
	.set_tristate	= twl4030_set_tristate,
};

static struct snd_soc_dai_ops twl4030_dai_voice_ops = {
	.startup	= twl4030_voice_startup,
	.shutdown	= twl4030_voice_shutdown,
	.hw_params	= twl4030_voice_hw_params,
	.set_sysclk	= twl4030_voice_set_dai_sysclk,
	.set_fmt	= twl4030_voice_set_dai_fmt,
	.set_tristate	= twl4030_voice_set_tristate,
};

static struct snd_soc_dai_ops twl4030_dai_clock_ops = {
	.startup = twl4030_clock_startup,
	.shutdown = twl4030_shutdown,
	.hw_params = twl4030_clock_hw_params,
	.hw_free = twl4030_hw_free,
	.set_sysclk = twl4030_set_dai_sysclk,
	.set_fmt = twl4030_clock_set_dai_fmt,
};

static struct snd_soc_dai_driver twl4030_dai[] = {
// prime@sdcmicro.com Added null dai for FM and IFX call [START]
{
	.name = "null",
	.playback = {
		.stream_name = "Playback",
		.channels_min = 1,
		.channels_max = 4,
		.rates = SNDRV_PCM_RATE_8000_96000,
		.formats = SNDRV_PCM_FMTBIT_S16_LE | SNDRV_PCM_FORMAT_S24_LE,},
	.capture = {
		.stream_name = "Capture",
		.channels_min = 1,
		.channels_max = 4,
		.rates = SNDRV_PCM_RATE_8000_96000,
		.formats = SNDRV_PCM_FMTBIT_S16_LE | SNDRV_PCM_FORMAT_S24_LE,},
},
// prime@sdcmicro.com Added null dai for FM and IFX call [END]
{
	.name = "twl4030-hifi",
	.playback = {
		.stream_name = "HiFi Playback",
		.channels_min = 2,
		.channels_max = 4,
		.rates = TWL4030_RATES | SNDRV_PCM_RATE_96000,
		.formats = TWL4030_FORMATS,},
	.capture = {
		.stream_name = "Capture",
#if 1 // TI Patch (Audio Broken issue)
#if 1 // gt.kim@lge.com justin/black same appliy ASR Patch...
//#ifdef CONFIG_PRODUCT_LGE_KU5900
		.channels_min = 1,
		.channels_max = 2,
		.rates = TWL4030_RATES,
#else
		.channels_min = 2,
		.channels_max = 4,
		.rates = TWL4030_RATES,
#endif
#else // old
//--[[ LGE_UBIQUIX_MODIFIED_START : bsnoh@ubiquix.com - add from DCM_GB
		.channels_min = 1, //soyoung77.park(mono recording)
		.channels_max = 2, //soyoung77.park(mono recording)
//--]] LGE_UBIQUIX_MODIFIED_END : bsnoh@ubiquix.com - add from DCM_GB
//      .rates = TWL4030_RATES, //junyeop.kim@lge.com, test
        .rates = SNDRV_PCM_RATE_44100,
#endif
		.formats = TWL4030_FORMATS,},
	.ops = &twl4030_dai_hifi_ops,
},
{
	.name = "twl4030-voice",
	.playback = {
		.stream_name = "Voice Playback",
		.channels_min = 1,
        //--[[ LGE_UBIQUIX_MODIFIED_START : bsnoh@ubiquix.com - add from DCM_GB
		//.channels_max = 1,
		.channels_max = 2,
        //--]] LGE_UBIQUIX_MODIFIED_END : bsnoh@ubiquix.com - add from DCM_GB
		.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000,
		.formats = SNDRV_PCM_FMTBIT_S16_LE,},
	.capture = {
		.stream_name = "Capture",
		.channels_min = 1,
		.channels_max = 2,
		.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000,
		.formats = SNDRV_PCM_FMTBIT_S16_LE,},
	.ops = &twl4030_dai_voice_ops,
},
{
	.name = "twl4030-clock",
	.playback = {
		.stream_name = "Playback",
		.channels_min = 1,
		.channels_max = 2,
		.rates = TWL4030_RATES,
		.formats = SNDRV_PCM_FMTBIT_U8 | TWL4030_FORMATS,},
	.capture = {
		.stream_name = "Capture",
		.channels_min = 1,
		.channels_max = 2,
		.rates = TWL4030_RATES,
		.formats = SNDRV_PCM_FMTBIT_U8 | TWL4030_FORMATS,},
	.ops = &twl4030_dai_clock_ops,
},
};

// 20100603 junyeop.kim@lge.com, headset suspend/resume [START_LGE]
unsigned int get_twl4030_status(void)
{
    return (unsigned int)twl4030_status;
}
EXPORT_SYMBOL_GPL(get_twl4030_status);
// 20100603 junyeop.kim@lge.com, headset suspend/resume [END_LGE]

#if defined(CONFIG_PRODUCT_LGE_HUB)
// 20101128 junyeop.kim@lge.com, dmb ant status[START_LGE]
void set_dmb_status(int state)
{
    dmb_status = (unsigned int)state;
}

EXPORT_SYMBOL_GPL(set_dmb_status);
// 20101128 junyeop.kim@lge.com, dmb ant status[END_LGE]
#endif	//#if defined(CONFIG_PRODUCT_LGE_HUB)
static int twl4030_soc_suspend(struct snd_soc_codec *codec, pm_message_t state)
{
	DBG("\n");

	twl4030_set_bias_level(codec, SND_SOC_BIAS_OFF);
#if defined(CONFIG_PRODUCT_LGE_HUB)
	twl4030_status = 0;	// 20100603 junyeop.kim@lge.com, headset suspend/resume [START_LGE]
#elif defined(CONFIG_PRODUCT_LGE_LU6800) || defined(CONFIG_PRODUCT_LGE_KU5900) || defined (CONFIG_PRODUCT_LGE_P970)
//20101222 inbang.park@lge.com Wake lock for  FM Radio [START]
	if(fmradio_is_on != 1)
       twl4030_status = 0;	// 20100603 junyeop.kim@lge.com, headset suspend/resume [START_LGE]
  //20101222 inbang.park@lge.com Wake lock for  FM Radio [END]
#endif
	return 0;
}

static int twl4030_soc_resume(struct snd_soc_codec *codec)
{
	DBG("\n");

	twl4030_set_bias_level(codec, SND_SOC_BIAS_STANDBY);
	twl4030_status = 1;	// 20100603 junyeop.kim@lge.com, headset suspend/resume [START_LGE]
#if defined(CONFIG_PRODUCT_LGE_LU6800) || defined(CONFIG_PRODUCT_LGE_KU5900) || defined (CONFIG_PRODUCT_LGE_P970)
	//20101222 inbang.park@lge.com Wake lock for  FM Radio [START]
	fmradio_is_on = 0;
	//20101222 inbang.park@lge.com Wake lock for  FM Radio [END]
#endif
	return 0;
}

#if defined(CONFIG_PRODUCT_LGE_LU6800) || defined(CONFIG_PRODUCT_LGE_KU5900) || defined (CONFIG_PRODUCT_LGE_P970) || defined(CONFIG_PRODUCT_LGE_HUB) /* LGE_CHANGE_S [iggikim@lge.com] 2009-11-04, call path */
void hub_set_call_mode(struct snd_pcm_substream *substream, int mode)
{
	struct snd_soc_codec* codec = snd_soc_get_codec("twl4030-codec");
	struct twl4030_priv *twl4030 = snd_soc_codec_get_drvdata(codec);
DBG("\n");
	if(mode == 1){
		//twl_i2c_write_u8(TWL4030_MODULE_AUDIO_VOICE, 0x03, TWL4030_REG_MICBIAS_CTL); //mic bias on
		twl_i2c_write_u8(TWL4030_MODULE_AUDIO_VOICE, 0x01, TWL4030_REG_MICBIAS_CTL); //mic bias on // bs.lim 03 -> 01
		twl_i2c_write_u8(TWL4030_MODULE_AUDIO_VOICE, 0x00, TWL4030_REG_PREDL_CTL); //spk off
		twl4030->is_calling = 1;
	}
	else{
		twl_i2c_write_u8(TWL4030_MODULE_AUDIO_VOICE, 0x00, TWL4030_REG_MICBIAS_CTL);
		twl_i2c_write_u8(TWL4030_MODULE_AUDIO_VOICE, 0x10, TWL4030_REG_PREDL_CTL);
		twl4030->is_calling =0;
	}
}
/* LGE_CHANGE_S [iggikim@lge.com] 2009-11-10, rev a headset */
void hub_headsetdet_bias(int bias)
{
	struct snd_soc_codec* codec = snd_soc_get_codec("twl4030-codec");
	u8 *cache = codec ? codec->reg_cache : 0;    	// 20100608 junyeop.kim@lge.com, update the codec_reg table for ext mic  [START_LGE]

#if 0	/* LGE_CHANGE_S, [junyeop.kim@lge.com] 2010-04-01, mic bias enable on/off for external mic*/
	if(bias == 1){
		twl_i2c_write_u8(TWL4030_MODULE_AUDIO_VOICE, 0x02, 0x04); //mic bias2 on
	}
	else{
		twl_i2c_write_u8(TWL4030_MODULE_AUDIO_VOICE, 0x00, 0x04); //mic bias2 off
	}
#else
	u8 tmp=0;
	DBG("\n");

	twl_i2c_read_u8(TWL4030_MODULE_AUDIO_VOICE, &tmp, 0x04); //junyeop.kim@lge.com

	if(bias == 1)
		tmp |= 0x02;
	else
		tmp &= ~0x02;   //junyeop.kim@lge.com

	twl_i2c_write_u8(TWL4030_MODULE_AUDIO_VOICE, tmp, 0x04); //mic bias2 on
#endif

	if (cache)
		cache[TWL4030_REG_MICBIAS_CTL] = tmp;    	// 20100608 junyeop.kim@lge.com, update the codec_reg table for ext mic  [START_LGE]
}

EXPORT_SYMBOL(hub_set_call_mode);
EXPORT_SYMBOL(hub_headsetdet_bias);
/* LGE_CHANGE_E [iggikim@lge.com] */
#endif

static int twl4030_soc_probe(struct snd_soc_codec *codec)
{
	struct twl4030_priv *twl4030;
	DBG("\n");

	twl4030 = kzalloc(sizeof(struct twl4030_priv), GFP_KERNEL);
	if (twl4030 == NULL) {
		printk(KERN_ERR "Can not allocate memroy\n");
		return -ENOMEM;
	}

	snd_soc_codec_set_drvdata(codec, twl4030); //(struct device *)(codec->dev)->p->driver_data = twl4030

	/* Set the defaults, and power up the codec */
	twl4030->sysclk = twl4030_codec_get_mclk() / 1000;
	codec->dapm.bias_level = SND_SOC_BIAS_OFF;
	codec->dapm.idle_bias_off = 1;

#if defined(CONFIG_PRODUCT_LGE_LU6800) || defined(CONFIG_PRODUCT_LGE_KU5900) || defined (CONFIG_PRODUCT_LGE_P970) || defined(CONFIG_PRODUCT_LGE_HUB) /* LGE_CHANGE_S [iggikim@lge.com] 2009-11-04, call path */
	twl4030->is_calling = 0;
#endif

	twl4030_init_chip(codec);

	snd_soc_add_controls(codec, twl4030_snd_controls,
				ARRAY_SIZE(twl4030_snd_controls));
	twl4030_add_widgets(codec);
	twl4030_status = 1;	// 20100603 junyeop.kim@lge.com, headset suspend/resume [START_LGE]

	return 0;
}

static int twl4030_soc_remove(struct snd_soc_codec *codec)
{
	struct twl4030_priv *twl4030 = snd_soc_codec_get_drvdata(codec);
	DBG("\n");

	/* Reset registers to their chip default before leaving */
	twl4030_reset_registers(codec);
	twl4030_set_bias_level(codec, SND_SOC_BIAS_OFF);
	kfree(twl4030);
	return 0;
}

static struct snd_soc_codec_driver soc_codec_dev_twl4030 = {
	.probe = twl4030_soc_probe,
	.remove = twl4030_soc_remove,
	.suspend = twl4030_soc_suspend,
	.resume = twl4030_soc_resume,
	.read = twl4030_read_reg_cache,
	.write = twl4030_write,
	.set_bias_level = twl4030_set_bias_level,
	.reg_cache_size = sizeof(twl4030_reg),
	.reg_word_size = sizeof(u8),
	.reg_cache_default = twl4030_reg,
};

static int __devinit twl4030_codec_probe(struct platform_device *pdev)
{
	struct twl4030_codec_audio_data *pdata = pdev->dev.platform_data;
	DBG("\n");

	if (!pdata) {
		dev_err(&pdev->dev, "platform_data is missing\n");
		return -EINVAL;
	}

#if defined(CONFIG_PRODUCT_LGE_LU6800) || defined(CONFIG_PRODUCT_LGE_KU5900) || defined (CONFIG_PRODUCT_LGE_P970)
         gpio_request(MIC_SEL_GPIO, pdev->name);
	gpio_direction_output(MIC_SEL_GPIO, 0);
#endif

	return snd_soc_register_codec(&pdev->dev, &soc_codec_dev_twl4030,
			twl4030_dai, ARRAY_SIZE(twl4030_dai));
}

static int __devexit twl4030_codec_remove(struct platform_device *pdev)
{
	struct twl4030_priv *twl4030 = dev_get_drvdata(&pdev->dev);
	DBG("\n");

	snd_soc_unregister_codec(&pdev->dev);
	kfree(twl4030);
	return 0;
}

MODULE_ALIAS("platform:twl4030-codec");

static struct platform_driver twl4030_codec_driver = {
	.probe		= twl4030_codec_probe,
	.remove		= __devexit_p(twl4030_codec_remove),
	.driver		= {
		.name	= "twl4030-codec",
		.owner	= THIS_MODULE,
	},
};

static int __init twl4030_modinit(void)
{
	return platform_driver_register(&twl4030_codec_driver);
}
module_init(twl4030_modinit);

static void __exit twl4030_exit(void)
{
	platform_driver_unregister(&twl4030_codec_driver);
}
module_exit(twl4030_exit);

MODULE_DESCRIPTION("ASoC TWL4030 codec driver");
MODULE_AUTHOR("Steve Sakoman");
MODULE_LICENSE("GPL");


