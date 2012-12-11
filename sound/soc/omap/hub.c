// prime@sdcmicro.com Reworked for 2.6.35 [START]
#include <linux/clk.h>
#include <linux/platform_device.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>

#include <asm/mach-types.h>
#include <mach/hardware.h>
#include <mach/gpio.h>
#include <plat/mcbsp.h>
#include <plat/mux.h>
#include <plat/omap-pm.h>

#include "omap-mcbsp.h"
#include "omap-pcm.h"
/* Register descriptions for twl4030 codec part */
#include <linux/mfd/twl4030-codec.h>


#include "../../../arch/arm/mach-omap2/mux.h"
/* LGE_CHANGE_S [iggikim@lge.com] 2009-09-05, evb b board */
#if defined(CONFIG_HUB_AMP_WM9093)
#include <mach/wm9093.h>
#endif
/* LGE_CHANGE_E [iggikim@lge.com] */

#define MODULE_NAME		"SOC-AUDIO(B)"


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

#define OMAP_MCBSP_MASTER_MODE	0

voice_mode_enum cur_voice_mode=0;

extern int twl4030_set_ext_clock(struct snd_soc_codec *codec, int enable);
unsigned int mp3_playing=0;
EXPORT_SYMBOL(mp3_playing);
#if defined(CONFIG_BACKLIGHT_AAT2870)
extern int check_bl_shutdown;
#endif

static int hub_i2s_hw_params(struct snd_pcm_substream *substream,
				struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *codec_dai = rtd->codec_dai;
	struct snd_soc_dai *cpu_dai = rtd->cpu_dai;
	int ret;
	DBG("\n");

#if 1 // Applied path with justin/black..
	// McBSP2 SLAVE MODE mux settings
	omap_mux_init_signal("mcbsp2_fsx.mcbsp2_fsx",OMAP_PIN_INPUT);
	omap_mux_init_signal("mcbsp2_clkx.mcbsp2_clkx",OMAP_PIN_INPUT);
	omap_mux_init_signal("mcbsp2_dr.mcbsp2_dr",OMAP_PIN_INPUT);
	omap_mux_init_signal("mcbsp2_dx.mcbsp2_dx",OMAP_PIN_OUTPUT | OMAP_PULL_ENA);
#endif
	/* Set codec DAI configuration */
	ret = snd_soc_dai_set_fmt(codec_dai,
				  SND_SOC_DAIFMT_I2S |
				  SND_SOC_DAIFMT_NB_NF |
				  SND_SOC_DAIFMT_CBM_CFM);
	if (ret < 0) {
		printk(KERN_ERR "can't set codec DAI configuration\n");
		return ret;
	}

	/* Set cpu DAI configuration */
	ret = snd_soc_dai_set_fmt(cpu_dai,
				  SND_SOC_DAIFMT_I2S |
				  SND_SOC_DAIFMT_NB_NF |
				  SND_SOC_DAIFMT_CBM_CFM);
	if (ret < 0) {
		printk(KERN_ERR "can't set cpu DAI configuration\n");
		return ret;
	}

	/* Set the codec system clock for DAC and ADC */
	ret = snd_soc_dai_set_sysclk(codec_dai, 0, 26000000,
					SND_SOC_CLOCK_IN);
	if (ret < 0) {
		printk(KERN_ERR "can't set codec system clock\n");
		return ret;
	}

	/* enable 256 FS clk for HDMI */

	 ret = twl4030_set_ext_clock(codec_dai->codec, 1);
	 if (ret < 0) {
		printk(KERN_ERR "can't set 256 FS clock\n");
		return ret;
	 }

	/* Use external clock for mcBSP2 */
	ret = snd_soc_dai_set_sysclk(cpu_dai, OMAP_MCBSP_SYSCLK_CLKS_EXT,
			0, SND_SOC_CLOCK_OUT);

	if (ret < 0) {
		printk(KERN_ERR "can't set cpu_dai system clock\n");
		return ret;
	}
	/*
	 * Set headset EXTMUTE signal to ON to make sure we
	 * get correct headset status
	 */
#if 0//defined(CONFIG_BACKLIGHT_AAT2870)//DELETED BY NATTING
	mp3_playing=1; 
#endif

	return 0;
}

int hub_i2s_hw_free(struct snd_pcm_substream *substream)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *cpu_dai = rtd->cpu_dai;
	int ret;

	DBG("\n");
	/* Use function clock for mcBSP2 */
	ret = snd_soc_dai_set_sysclk(cpu_dai, OMAP_MCBSP_SYSCLK_CLKS_FCLK,
			0, SND_SOC_CLOCK_OUT);
#if 0//defined(CONFIG_BACKLIGHT_AAT2870)//DELETED BY NATTING			
	if (check_bl_shutdown!=1)		
	mp3_playing=0;
#endif	
	return 0;
}


static struct snd_soc_ops hub_i2s_ops = {
	.hw_params = hub_i2s_hw_params,
	.hw_free = hub_i2s_hw_free,
};
#if 1
static int hub_pcm_hw_params(struct snd_pcm_substream *substream,
	struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *codec_dai = rtd->codec_dai;
	struct snd_soc_dai *cpu_dai = rtd->cpu_dai;
	int ret;

	DBG("\n");

#if 0 // Applied path with justin/black..
	// McBSP2 SLAVE MODE mux settings
	omap_mux_init_signal("mcbsp2_fsx.mcbsp2_fsx",OMAP_PIN_INPUT);
	omap_mux_init_signal("mcbsp2_clkx.mcbsp2_clkx",OMAP_PIN_INPUT);
	omap_mux_init_signal("mcbsp2_dr.mcbsp2_dr",OMAP_PIN_INPUT);
	omap_mux_init_signal("mcbsp2_dx.mcbsp2_dx",OMAP_PIN_OUTPUT | OMAP_PULL_ENA);
#endif
	
#if OMAP_MCBSP_MASTER_MODE
	struct snd_soc_dai *codec_dai = rtd->dai->codec_dai;
	int divisor;

//	omap3_mux_config("OMAP_MCBSP3_MASTER");
	omap_mux_init_signal("mcbsp_clks.mcbsp_clks",OMAP_PIN_INPUT);
	omap_mux_init_signal("mcbsp3_fsx.mcbsp3_fsx",OMAP_PIN_OUTPUT);
	omap_mux_init_signal("mcbsp3_clkx.mcbsp3_clkx",OMAP_PIN_OUTPUT);
	omap_mux_init_signal("mcbsp3_dr.mcbsp3_dr",OMAP_PIN_INPUT_PULLDOWN);
	omap_mux_init_signal("mcbsp3_dx.mcbsp3_dx",OMAP_PIN_OUTPUT);


	/* Set codec DAI configuration */
	ret = snd_soc_dai_set_fmt(codec_dai,
				  SND_SOC_DAIFMT_DSP_B |
				  SND_SOC_DAIFMT_NB_NF |
				  SND_SOC_DAIFMT_CBS_CFS);
	if (ret < 0) {
		printk(KERN_ERR "can't set codec DAI configuration\n");
		return ret;
	}

	/* Set cpu DAI configuration */
	ret = snd_soc_dai_set_fmt(cpu_dai,
				  SND_SOC_DAIFMT_DSP_B |
				  SND_SOC_DAIFMT_NB_NF |
				  SND_SOC_DAIFMT_CBS_CFS);
	if (ret < 0) {
		printk(KERN_ERR "can't set cpu DAI configuration\n");
		return ret;
	}

	/* Set the codec system clock for DAC and ADC */
	ret = snd_soc_dai_set_sysclk(codec_dai, 0, 26000000,
			SND_SOC_CLOCK_IN);
	if (ret < 0) {
		printk(KERN_ERR "can't set codec system clock\n");
		return ret;
	}

	ret = twl4030_set_rate(codec_dai->codec, params);

	/* Use external (CLK256FS) clock for mcBSP3 */
	ret = snd_soc_dai_set_sysclk(cpu_dai, OMAP_MCBSP_SYSCLK_CLKS_EXT,
			0, SND_SOC_CLOCK_OUT);
	if (ret < 0) {
		printk(KERN_ERR "can't set mcBSP3 to external clock\n");
		return ret;
	}

	divisor = twl4030_get_clock_divisor(codec_dai->codec, params);

	ret = snd_soc_dai_set_clkdiv(cpu_dai, OMAP_MCBSP_CLKGDV, divisor);
	if (ret < 0) {
		printk(KERN_ERR "can't set codec clock divisor\n");
		return ret;
	}
#else
	
//	omap3_mux_config("OMAP_MCBSP3_SLAVE");
	omap_mux_init_signal("mcbsp3_fsx.mcbsp3_fsx",OMAP_PIN_INPUT_PULLDOWN);
	omap_mux_init_signal("mcbsp3_clkx.mcbsp3_clkx",OMAP_PIN_INPUT_PULLDOWN);
	omap_mux_init_signal("mcbsp3_dr.mcbsp3_dr",OMAP_PIN_INPUT_PULLDOWN);
	omap_mux_init_signal("mcbsp3_dx.mcbsp3_dx",OMAP_PIN_OUTPUT);


	/* Set cpu DAI configuration */
	ret = snd_soc_dai_set_fmt(cpu_dai,
				  SND_SOC_DAIFMT_DSP_B |
					  SND_SOC_DAIFMT_IB_IF |
				  SND_SOC_DAIFMT_CBM_CFM);
	if (ret < 0) {
		printk(KERN_ERR "can't set cpu DAI configuration\n");
		return ret;
	}

#endif

	return 0;
}

#else
static int hub_pcm_hw_params(struct snd_pcm_substream *substream,
	struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *cpu_dai = rtd->cpu_dai;
	int ret;

	struct snd_soc_dai *codec_dai = rtd->codec_dai;
	int divisor;

	omap3_mux_config("OMAP_MCBSP3_MASTER");

	/* Set cpu DAI configuration */
	ret = snd_soc_dai_set_fmt(cpu_dai,
				  SND_SOC_DAIFMT_DSP_A |
				  SND_SOC_DAIFMT_IB_IF |
				  SND_SOC_DAIFMT_CBS_CFS);
	if (ret < 0) {
		printk(KERN_ERR "can't set cpu DAI configuration\n");
		return ret;
	}

	/* Use external (CLK256FS) clock for mcBSP3 */
	ret = snd_soc_dai_set_sysclk(cpu_dai, OMAP_MCBSP_SYSCLK_CLKS_FCLK, //iggikim 20091012 bs300 mic
			0, SND_SOC_CLOCK_OUT);
	if (ret < 0) {
		printk(KERN_ERR "can't set mcBSP3 to external clock\n");
		return ret;
	}

	ret = snd_soc_dai_set_clkdiv(cpu_dai, OMAP_MCBSP_CLKGDV, 75); //iggikim 20091012 bs300 mic
	if (ret < 0) {
		printk(KERN_ERR "can't set codec clock divisor\n");
		return ret;
	}

	return 0;
}


#endif /* LGE_CHANGE_E [iggikim@lge.com]*/

int hub_pcm_hw_free(struct snd_pcm_substream *substream)
{
    DBG("cur_voice_mode=%d \n",cur_voice_mode);

	if ((cur_voice_mode==10))//&&(capture_running_flag != 3) && (playback_running_flag != 3))
	{
//		omap3_mux_config("OMAP_MCBSP3_TRISTATE");
		omap_mux_init_signal("mcbsp3_fsx.safe_mode",OMAP_PIN_INPUT);
		omap_mux_init_signal("mcbsp3_clkx.safe_mode",OMAP_PIN_INPUT);
		omap_mux_init_signal("mcbsp3_dr.safe_mode",OMAP_PIN_INPUT);
		omap_mux_init_signal("mcbsp3_dx.safe_mode",OMAP_PIN_INPUT);

#if 1//20110126 jisun.kwon : need to merge
//20110128 jisun.kwon blocking	voice_configure_path(0); 
		wm9093_configure_path(0);
#endif
	}
	return 0;
}

static struct snd_soc_ops hub_pcm_ops = {
	.hw_params = hub_pcm_hw_params,
	.hw_free = hub_pcm_hw_free,
};

#if defined(CONFIG_PRODUCT_LGE_LU6800) || defined(CONFIG_PRODUCT_LGE_KU5900) || defined (CONFIG_PRODUCT_LGE_P970) || defined(CONFIG_PRODUCT_LGE_HUB)/* LGE_CHANGE_S [iggikim@lge.com] 2009-11-04, call path */
extern void hub_set_call_mode(struct snd_pcm_substream *substream, int mode);

static int hub_call_hw_params(struct snd_pcm_substream *substream,
	struct snd_pcm_hw_params *params)
{
    DBG("\n");
	hub_set_call_mode(substream, 1);
//	omap3_mux_config("OMAP_MCBSP3_TRISTATE");
	omap_mux_init_signal("mcbsp3_fsx.safe_mode",OMAP_PIN_INPUT);
	omap_mux_init_signal("mcbsp3_clkx.safe_mode",OMAP_PIN_INPUT);
	omap_mux_init_signal("mcbsp3_dr.safe_mode",OMAP_PIN_INPUT);
	omap_mux_init_signal("mcbsp3_dx.safe_mode",OMAP_PIN_INPUT);
	
	return 0;
}

int hub_call_hw_free(struct snd_pcm_substream *substream)
{
    DBG("\n");
	hub_set_call_mode(substream, 0);
//    omap3_mux_config("OMAP_MCBSP3_TRISTATE");
	omap_mux_init_signal("mcbsp3_fsx.safe_mode",OMAP_PIN_INPUT);
	omap_mux_init_signal("mcbsp3_clkx.safe_mode",OMAP_PIN_INPUT);
	omap_mux_init_signal("mcbsp3_dr.safe_mode",OMAP_PIN_INPUT);
	omap_mux_init_signal("mcbsp3_dx.safe_mode",OMAP_PIN_INPUT);

	return 0;
}


static struct snd_soc_ops hub_call_ops = {
	.hw_params = hub_call_hw_params,
	.hw_free = hub_call_hw_free,
};
#endif

static int hub_fm_hw_params(struct snd_pcm_substream *substream,
	struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *cpu_dai = rtd->cpu_dai;	// 20110106 prime@sdcmicro.com
	int ret;
    DBG("\n");
#if OMAP_MCBSP_MASTER_MODE
	struct snd_soc_dai *codec_dai = rtd->dai->codec_dai;
	int divisor;

	/* Set codec DAI configuration */
	ret = snd_soc_dai_set_fmt(codec_dai,
				  SND_SOC_DAIFMT_I2S |
				  SND_SOC_DAIFMT_NB_NF |
				  SND_SOC_DAIFMT_CBS_CFS);
	if (ret < 0) {
		printk(KERN_ERR "can't set codec DAI configuration\n");
		return ret;
	}

	/* Set cpu DAI configuration */
	ret = snd_soc_dai_set_fmt(cpu_dai,
				  SND_SOC_DAIFMT_I2S |
				  SND_SOC_DAIFMT_NB_NF |
				  SND_SOC_DAIFMT_CBS_CFS);
	if (ret < 0) {
		printk(KERN_ERR "can't set cpu DAI configuration\n");
		return ret;
	}

	/* Set the codec system clock for DAC and ADC */
	ret = snd_soc_dai_set_sysclk(codec_dai, 0, 26000000,
			SND_SOC_CLOCK_IN);
	if (ret < 0) {
		printk(KERN_ERR "can't set codec system clock\n");
		return ret;
	}

	ret = twl4030_set_rate(codec_dai->codec, params);

	/* Use external (CLK256FS) clock for mcBSP4 */
	ret = snd_soc_dai_set_sysclk(cpu_dai, OMAP_MCBSP_SYSCLK_CLKS_EXT,
			0, SND_SOC_CLOCK_OUT);
	if (ret < 0) {
		printk(KERN_ERR "can't set mcBSP4 to external clock\n");
		return ret;
	}

	divisor = twl4030_get_clock_divisor(codec_dai->codec, params);

	ret = snd_soc_dai_set_clkdiv(cpu_dai, OMAP_MCBSP_CLKGDV, divisor);
	if (ret < 0) {
		printk(KERN_ERR "can't set codec clock divisor\n");
		return ret;
	}
#else
	/* Set cpu DAI configuration */
	ret = snd_soc_dai_set_fmt(cpu_dai,
				  SND_SOC_DAIFMT_I2S |
				  SND_SOC_DAIFMT_NB_NF |
				  SND_SOC_DAIFMT_CBM_CFM);
	if (ret < 0) {
		printk(KERN_ERR "can't set cpu DAI configuration\n");
		return ret;
	}
#endif
	return 0;
}

static struct snd_soc_ops hub_fm_ops = {
	.hw_params = hub_fm_hw_params,
};

/* LGE_CHANGE_S [iggikim@lge.com] 2009-09-05, evb b board */
#if defined(CONFIG_AUDIO_AMP_MAX9877)
static int ext_amp_get_mode(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	ucontrol->value.integer.value[0] = max9877_get_curmode();
	return 0;
}

static int ext_amp_set_mode(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	max9877_configure_path((max9877_mode_enum)ucontrol->value.integer.value[0]);
	
	return 1;
}
#elif defined(CONFIG_HUB_AMP_WM9093)
static int ext_amp_get_mode(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	DBG("\n");
	ucontrol->value.integer.value[0] = wm9093_get_curmode();
	return 0;
}

static int ext_amp_set_mode(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
//#if defined(CONFIG_PRODUCT_LGE_KU5900)
#if defined(CONFIG_PRODUCT_LGE_KU5900) || defined(CONFIG_PRODUCT_LGE_P970) /* [LGE_CHANGE] 20120908 pyocool.cho@lge.com "for p970" */
	int ext_amp_mode = (wm9093_mode_enum)ucontrol->value.integer.value[0];
	DBG("SET cur_ext_amp_mode(%d) codec_status(%d) wm9093_get_curmode(%d)\n",ext_amp_mode,get_twl4030_headset_spk_codec_status(),wm9093_get_curmode());

	/* Quickly Amp off */
	if(ext_amp_mode == OFF_MODE){
	    wm9093_configure_path(ext_amp_mode);
	}

    // wooho.jeong@lge.com 2012.09.25
    // ADD : Audio for FM Radio
	if( ( (OFF_MODE <= ext_amp_mode) && (ext_amp_mode <= SPEAKER_HEADSET_DUAL_AUDIO_MODE ) ) 
        || ( ( ext_amp_mode == SPEAKER_FMR_MODE ) || ( ext_amp_mode == HEADSET_FMR_MODE ) )
        )
    {
	    set_ext_amp_mode(ext_amp_mode);

	    if( ((get_twl4030_headset_spk_codec_status() == 1) ||(wm9093_get_curmode() != OFF_MODE && ext_amp_mode != OFF_MODE))
	        ||((get_twl4030_apll_state() == 1) && (wm9093_get_curmode() == OFF_MODE && ext_amp_mode == SPEAKER_AUDIO_MODE)) )
	        wm9093_configure_path(ext_amp_mode);
        else if(( ext_amp_mode == SPEAKER_FMR_MODE ) || ( ext_amp_mode == HEADSET_FMR_MODE ))
            wm9093_configure_path(ext_amp_mode);
	}
	else
	    wm9093_configure_path(ext_amp_mode);
#else
	DBG("\n");
	wm9093_configure_path((wm9093_mode_enum)ucontrol->value.integer.value[0]);
#endif
       return 1;
}
#else
static int ext_amp_get_mode(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	
	return 0;
}

static int ext_amp_set_mode(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
		
	return 1;
}

#endif
#if defined(CONFIG_PRODUCT_LGE_HUB)
static const char *ext_amp_mode_texts[] =
		{"OFF", "Headset", "Speaker", "Dual", "Bypass", "Spkcall", "Headsetcall", "BypassVOIP", "SpkcallVOIP", "HeadsetcallVOIP"};
static const struct soc_enum ext_amp_mode_soc_enum =
	SOC_ENUM_SINGLE_EXT(10, ext_amp_mode_texts);
#elif defined(CONFIG_PRODUCT_LGE_LU6800) || defined(CONFIG_PRODUCT_LGE_KU5900) || defined (CONFIG_PRODUCT_LGE_P970)
//20101205 inbang.park@lge.com Add STREAM  for  FM Radio [START] 
static const char *ext_amp_mode_texts[] =
		{"OFF", "Headset", "Speaker", "Dual", "Bypass", "Spkcall", "Headsetcall", "BypassVOIP", "SpkcallVOIP", "HeadsetcallVOIP","SpkFMR","HeadsetFMR"};
//20101205 inbang.park@lge.com Add STREAM  for  FM Radio [END] 
static const struct soc_enum ext_amp_mode_soc_enum =
	SOC_ENUM_SINGLE_EXT(12, ext_amp_mode_texts);
#endif

static const struct snd_kcontrol_new ext_amp_mode_control =
	SOC_ENUM_EXT("Route", ext_amp_mode_soc_enum, ext_amp_get_mode, ext_amp_set_mode);
#if defined(CONFIG_CALL_USES_BS300)

static int bs300_2_mic_get_mode(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	ucontrol->value.integer.value[0] = bs300_get_curmode();
	return 0;
}

static int bs300_2_mic_set_mode(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	bs300_set_mode((bs300_mode_enum)ucontrol->value.integer.value[0]);
	
	return 1;
}

static const char *bs300_mode_texts[] =
		{"Deepsleep", "Lineout", "Bypass", "call", "cam"};

static const struct soc_enum bs300_mode_soc_enum =
	SOC_ENUM_SINGLE_EXT(5, bs300_mode_texts);

static const struct snd_kcontrol_new bs300_mode_control =
SOC_ENUM_EXT("Route", bs300_mode_soc_enum, bs300_2_mic_get_mode, bs300_2_mic_set_mode);
#endif

/* LGE_CHANGE_S [iggikim@lge.com] 2009-08-06, audio path */
static int voice_get_mode(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	ucontrol->value.integer.value[0] = voice_get_curmode();
	return 0;
}

#ifndef HARDLINE_TEST
int cur_mode_check;
#endif
static int voice_set_mode(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
    DBG("mode=%d\n",(voice_mode_enum)ucontrol->value.integer.value[0]);
	voice_configure_path((voice_mode_enum)ucontrol->value.integer.value[0]);
	cur_voice_mode=(voice_mode_enum)ucontrol->value.integer.value[0];
	#ifndef HARDLINE_TEST
	cur_mode_check=cur_voice_mode;
	#endif
	
	return 1;
}

#if defined(CONFIG_PRODUCT_LGE_LU6800) || defined(CONFIG_PRODUCT_LGE_KU5900) || defined (CONFIG_PRODUCT_LGE_P970)//20110126 jisun.kwon vt_bt_audiopath
static const char *voice_mode_texts[] =
		{"OFF", "Headset", "Speaker", "Receiver", "Headphone","VT_SCO_Headset","VT_Headset", "VT_Speaker","VT_Receiver","VT_Headphone","VT_Call_End","VOIP_End"};

static const struct soc_enum voice_mode_soc_enum =
	SOC_ENUM_SINGLE_EXT(12, voice_mode_texts);

#elif defined(CONFIG_PRODUCT_LGE_HUB)
static const char *voice_mode_texts[] =
		{"OFF", "Headset", "Speaker", "Receiver", "Headphone"};

static const struct soc_enum voice_mode_soc_enum =
	SOC_ENUM_SINGLE_EXT(5, voice_mode_texts);
#endif 
static const struct snd_kcontrol_new voice_mode_control =
	SOC_ENUM_EXT("Route", voice_mode_soc_enum, voice_get_mode, voice_set_mode);
/* LGE_CHANGE_E [iggikim@lge.com]*/

// 20100426 junyeop.kim@lge.com Add the mic mute [START_LGE]
static int mic_get_mode(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
    DBG("\n");
	ucontrol->value.integer.value[0] = mic_get_curmode();
	return 0;
}

static int mic_set_mode(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
    DBG("\n");
	mic_configure_path((mic_mode_enum)ucontrol->value.integer.value[0]);
	
	return 1;
}
static const char *mic_mode_texts[] =
		{"OFF", "Restore"};

static const struct soc_enum mic_mode_soc_enum =
	SOC_ENUM_SINGLE_EXT(2, mic_mode_texts);

static const struct snd_kcontrol_new mic_mode_control =
SOC_ENUM_EXT("Route", mic_mode_soc_enum, mic_get_mode, mic_set_mode);

// 20100426 junyeop.kim@lge.com Add the mic mute [END_LGE]

// 20100521 junyeop.kim@lge.com call recording path [START_LGE]
static int callrec_get_mode(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
    DBG("\n");
	ucontrol->value.integer.value[0] = callrec_get_curmode();
	return 0;
}

static int callrec_set_mode(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	DBG("callrec_set_mode : %d\n", (callrec_mode_enum)ucontrol->value.integer.value[0]);
	
	callrec_configure_path((callrec_mode_enum)ucontrol->value.integer.value[0]);
	
	return 1;
}
static const char *callrec_mode_texts[] =
		{"BuiltIn", "Headset", "OFF"};

static const struct soc_enum callrec_mode_soc_enum =
	SOC_ENUM_SINGLE_EXT(3, callrec_mode_texts);

static const struct snd_kcontrol_new callrec_mode_control =
	SOC_ENUM_EXT("Route", callrec_mode_soc_enum, callrec_get_mode, callrec_set_mode);
// 20100521 junyeop.kim@lge.com call recording path [END_LGE]
#if defined(CONFIG_PRODUCT_LGE_LU6800) || defined(CONFIG_PRODUCT_LGE_KU5900) || defined (CONFIG_PRODUCT_LGE_P970)
//20101205 inbang.park@lge.com Add STREAM  for  FM Radio [START] 
static int fmradio_set_volume(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol)
{
    DBG("\n");
       fmradio_configure_path((wm9093_fmvolume_enum)ucontrol->value.integer.value[0]);//20101222 inbang.park@lge.com Wake lock for  FM Radio
       wm9093_fmradio_volume((wm9093_fmvolume_enum)ucontrol->value.integer.value[0]);
	return 1;
}
static int fmradio_get_volume(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol)
{
         return 0;
}
static const char *FMradio_volume_texts[]={
  "LEVEL_OFF",
  "LEVEL_1",
  "LEVEL_2",
  "LEVEL_3",
  "LEVEL_4",
  "LEVEL_5",
  "LEVEL_6",
  "LEVEL_7",
  "LEVEL_8",
  "LEVEL_9",
  "LEVEL_10",
  "LEVEL_11",
  "LEVEL_12",
  "LEVEL_13",
  "LEVEL_14",
  "LEVEL_15",
  "LEVEL_reset"
};
static const struct soc_enum FMradio_volume_soc_enum =
SOC_ENUM_SINGLE_EXT(17, FMradio_volume_texts);
static const struct snd_kcontrol_new FM_volume_control =
SOC_ENUM_EXT("Route", FMradio_volume_soc_enum, fmradio_get_volume, fmradio_set_volume);
//20101205 inbang.park@lge.com Add STREAM  for  FM Radio [END] 
//jongik2.kim 20101220 add mic2 control [start]
static int set_mic_switch(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol)
{
	DBG("\n");
       twl4030_set_mic_switch((int)ucontrol->value.integer.value[0]);
	   return 1;
}

static int get_mic_switch(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol)
{
       return 0;
}
static const char *mic_switch_texts[]={ "MIC1", "MIC2" };
static const struct soc_enum mic_switch_soc_enum =
SOC_ENUM_SINGLE_EXT(2, mic_switch_texts);
static const struct snd_kcontrol_new mic_switch_control =
SOC_ENUM_EXT("Route", mic_switch_soc_enum, get_mic_switch, set_mic_switch);
//jongik2.kim 20101220 add mic2 control [end]
#endif // #if defined(CONFIG_PRODUCT_LGE_LU6800) || defined(CONFIG_PRODUCT_LGE_KU5900) || defined (CONFIG_PRODUCT_LGE_P970)
static const struct snd_soc_dapm_widget hub_twl4030_dapm_widgets[] = {
#if 0  //jongik2.kim@lge.com
	SND_SOC_DAPM_MIC("Ext Mic", NULL),
#else
	SND_SOC_DAPM_MIC("Ext Mic1", NULL),
	SND_SOC_DAPM_MIC("Ext Mic2", NULL),
#endif
	SND_SOC_DAPM_SPK("Ext Spk", NULL),
	SND_SOC_DAPM_SPK("Receiver", NULL),
	SND_SOC_DAPM_HP("Headset Jack", NULL),
	SND_SOC_DAPM_MIC("Ext Headset Mic", NULL),   //jongik2.kim@lge.com
	SND_SOC_DAPM_LINE("Aux In", NULL),
	SND_SOC_DAPM_MUX("ExtAmp", SND_SOC_NOPM, 0, 0, &ext_amp_mode_control),
	SND_SOC_DAPM_MUX("Voice", SND_SOC_NOPM, 0, 0, &voice_mode_control), /* LGE_CHANGE_S [iggikim@lge.com] 2009-08-06, audio path */
	SND_SOC_DAPM_MUX("Mic", SND_SOC_NOPM, 0, 0, &mic_mode_control),		// 20100426 junyeop.kim@lge.com Add the mic mute [START_LGE]
	SND_SOC_DAPM_MUX("CallRec", SND_SOC_NOPM, 0, 0, &callrec_mode_control),		// 20100521 junyeop.kim@lge.com call recording path [START_LGE]
#if defined(CONFIG_PRODUCT_LGE_LU6800) || defined(CONFIG_PRODUCT_LGE_KU5900) || defined (CONFIG_PRODUCT_LGE_P970)
//20101205 inbang.park@lge.com Add STREAM  for  FM Radio [START] 
       SND_SOC_DAPM_MUX("FMradio", SND_SOC_NOPM, 0, 0, &FM_volume_control),
//20101205 inbang.park@lge.com Add STREAM  for  FM Radio [END] 
    SND_SOC_DAPM_MUX("MIC Switch", SND_SOC_NOPM, 0, 0, &mic_switch_control),  //jongik2.kim 20101220 add mic2 control
    #if defined(CONFIG_CALL_USES_BS300)
    SND_SOC_DAPM_MUX("Bs300", SND_SOC_NOPM, 0, 0, &bs300_mode_control), 
	#endif
#endif // #if defined(CONFIG_PRODUCT_LGE_LU6800) || defined(CONFIG_PRODUCT_LGE_KU5900) || defined (CONFIG_PRODUCT_LGE_P970)
};
#if defined(CONFIG_CALL_USES_BS300)
static const struct snd_soc_dapm_route audio_map[] = {
	/* External Mics: MAINMIC, SUBMIC with bias*/
	{"MAINMIC", NULL, "Mic Bias 1"},
	{"SUBMIC", NULL, "Mic Bias 2"},
	{"Mic Bias 1", "cam", "Bs300"},
	{"Mic Bias 2", "cam", "Bs300"},
	{"Bs300", NULL, "Ext Mic"},
	{"Bs300", NULL, "Ext Mic"},
	/* receiver : EARPIECE */
    {"ExtAmp", "Bypass", "EARPIECE"},
    {"Receiver", NULL, "ExtAmp"},
	/* External Speakers: HFL, HFR */
	{"ExtAmp", "Speaker", "PREDRIVEL"},
	{"ExtAmp", "Speaker", "PREDRIVER"},
    {"Ext Spk", NULL, "ExtAmp"},
	{"Ext Spk", NULL, "ExtAmp"},
	/* Headset: HSMIC (with bias), HSOL, HSOR */
	{"ExtAmp", "Headset", "HSOL"},
	{"ExtAmp", "Headset", "HSOR"},
	{"Headset Jack", NULL, "ExtAmp"},
	{"Headset Jack", NULL, "ExtAmp"},
	{"HSMIC", NULL, "Headset Mic Bias"},
	{"Headset Mic Bias", NULL, "Headset Jack"},

	/* Aux In: AUXL, AUXR */
	{"Aux In", NULL, "AUXL"},
	{"Aux In", NULL, "AUXR"},
};
#else
static const struct snd_soc_dapm_route audio_map[] = {
	/* External Mics: MAINMIC, SUBMIC with bias*/
	{"MAINMIC", NULL, "Mic Bias 1"},
	{"SUBMIC", NULL, "Mic Bias 2"},
#if 0 //jongik2.kim@lge.com
	{"Mic Bias 1", NULL, "Ext Mic"},
	{"Mic Bias 2", NULL, "Ext Mic"},
#else
	{"Mic Bias 1", NULL, "Ext Mic1"},
	{"Mic Bias 2", NULL, "Ext Mic2"},
#endif
	{"ExtAmp", "Bypass", "EARPIECE"},
	{"Receiver", NULL, "ExtAmp"},
	/* External Speakers: HFL, HFR */
	{"ExtAmp", "Speaker", "PREDRIVEL"},
	{"ExtAmp", "Speaker", "PREDRIVER"},
	{"Ext Spk", NULL, "ExtAmp"},
	{"Ext Spk", NULL, "ExtAmp"},
	/* Headset: HSMIC (with bias), HSOL, HSOR */
	{"ExtAmp", "Headset", "HSOL"},
	{"ExtAmp", "Headset", "HSOR"},
	{"Headset Jack", NULL, "ExtAmp"},
	{"Headset Jack", NULL, "ExtAmp"},
#if 0 //jongik2.kim@lge.com
	{"HSMIC", NULL, "Headset Mic Bias"},
	{"Headset Mic Bias", NULL, "Headset Jack"},
#else
	{"HSMIC", NULL, "Mic Bias 2"},
	{"Mic Bias 2", NULL, "Ext Headset Mic"},
#endif
	/* Aux In: AUXL, AUXR */
	{"Aux In", NULL, "AUXL"},
	{"Aux In", NULL, "AUXR"},
};
#endif
static int hub_twl4030_init(struct snd_soc_pcm_runtime *rtd)
{
	struct snd_soc_codec *codec = rtd->codec;
	int ret;
	DBG("\n");

	/* Add hub specific widgets */
	ret = snd_soc_dapm_new_controls(&codec->dapm, hub_twl4030_dapm_widgets,
				ARRAY_SIZE(hub_twl4030_dapm_widgets));
	if (ret)
		return ret;

	/* Set up hub specific audio path audio_map */
	snd_soc_dapm_add_routes(&codec->dapm, audio_map, ARRAY_SIZE(audio_map));

	/* hub connected pins */
#if 0 //jongik2.kim@lge.com
	snd_soc_dapm_enable_pin(&codec->dapm, "Ext Mic");
#else
	snd_soc_dapm_enable_pin(&codec->dapm, "Ext Mic1");
	snd_soc_dapm_enable_pin(&codec->dapm, "Ext Mic2");
	snd_soc_dapm_enable_pin(&codec->dapm, "Ext Headset Mic");	
#endif
	snd_soc_dapm_enable_pin(&codec->dapm, "Ext Spk");
	snd_soc_dapm_enable_pin(&codec->dapm, "Receiver");
	snd_soc_dapm_enable_pin(&codec->dapm, "Headset Jack");
	snd_soc_dapm_enable_pin(&codec->dapm, "Aux In");

	/* TWL4030 not connected pins */
	snd_soc_dapm_nc_pin(&codec->dapm, "CARKITMIC");
	snd_soc_dapm_nc_pin(&codec->dapm, "DIGIMIC0");
	snd_soc_dapm_nc_pin(&codec->dapm, "DIGIMIC1");

	snd_soc_dapm_nc_pin(&codec->dapm, "OUTL");
	snd_soc_dapm_nc_pin(&codec->dapm, "OUTR");
#if 0 /* LGE_CHANGE_S [iggikim@lge.com] 2009-07-7, audio path */
	snd_soc_dapm_nc_pin(&codec->dapm, "EARPIECE");
	snd_soc_dapm_nc_pin(&codec->dapm, "PREDRIVEL");
	snd_soc_dapm_nc_pin(&codec->dapm, "PREDRIVER");
#else
	snd_soc_dapm_nc_pin(&codec->dapm, "HFL");
	snd_soc_dapm_nc_pin(&codec->dapm, "HFR");
#endif /* LGE_CHANGE_E [iggikim@lge.com]*/
	snd_soc_dapm_nc_pin(&codec->dapm, "CARKITL");
	snd_soc_dapm_nc_pin(&codec->dapm, "CARKITR");

	ret = snd_soc_dapm_sync(&codec->dapm);

	return ret;
}

static int hub_twl4030_voice_init(struct snd_soc_pcm_runtime *rtd)
{
	struct snd_soc_codec *codec = rtd->codec;
	unsigned short reg;
	DBG("\n");

	/* Enable voice interface */
	reg = codec->driver->read(codec, TWL4030_REG_VOICE_IF);
	reg |= TWL4030_VIF_DIN_EN | TWL4030_VIF_DOUT_EN | TWL4030_VIF_EN;
	codec->driver->write(codec, TWL4030_REG_VOICE_IF, reg);

	return 0;
}

/* Digital audio interface glue - connects codec <--> CPU */
static struct snd_soc_dai_link hub_dai[] = {
	{
		.name = "TWL4030_I2S",
		.stream_name = "TWL4030_I2S",
		.cpu_dai_name = "omap-mcbsp-dai.1",
		.codec_dai_name = "twl4030-hifi",
		.platform_name = "omap-pcm-audio",
		.codec_name = "twl4030-codec",
		.init = hub_twl4030_init,
		.ops = &hub_i2s_ops,
	},
	{
		.name = "TWL4030_PCM",
		.stream_name = "TWL4030 Voice",
		.cpu_dai_name = "omap-mcbsp-dai.2",
		.codec_dai_name = "twl4030-voice",
		.platform_name = "omap-pcm-audio",
		.codec_name = "twl4030-codec",
//		.init = hub_twl4030_voice_init,
		.ops = &hub_pcm_ops,
	},
	{
		.name = "TWL4030_FM",
		.stream_name = "TWL4030_FM",
		.cpu_dai_name = "omap-mcbsp-dai.3",
		.codec_dai_name = "null",
		.platform_name = "omap-pcm-audio",
		.codec_name = "twl4030-codec",
		.ops = &hub_fm_ops,
	},
#if defined(CONFIG_PRODUCT_LGE_LU6800) || defined(CONFIG_PRODUCT_LGE_KU5900) || defined (CONFIG_PRODUCT_LGE_P970) || defined(CONFIG_PRODUCT_LGE_HUB) /* LGE_CHANGE_S [iggikim@lge.com] 2009-11-04, call path */
	{
		.name = "IFX_CALL",
		.stream_name = "IFX_CALL",
		.cpu_dai_name = "omap-mcbsp-dai.4",
		.codec_dai_name = "null",
		.platform_name = "omap-pcm-audio",
		.codec_name = "twl4030-codec",
		.ops = &hub_call_ops,
	},
#endif
};

/* Audio machine driver */
static struct snd_soc_card snd_soc_hub = {
	.name = "hub",
	.long_name = "hub (twl4030)",
	.dai_link = hub_dai,
	.num_links = ARRAY_SIZE(hub_dai),
};

static struct platform_device *hub_snd_device;

static int __init hub_soc_init(void)
{
	int ret;

	printk(KERN_INFO "hub SoC init\n");

	hub_snd_device = platform_device_alloc("soc-audio", -1);
	if (!hub_snd_device) {
		printk(KERN_ERR "Platform device allocation failed\n");
		return -ENOMEM;
	}

	platform_set_drvdata(hub_snd_device, &snd_soc_hub);
	ret = platform_device_add(hub_snd_device);
	if (ret)
		goto err1;
	DBG("\n");

	return 0;

err1:
	printk(KERN_ERR "Unable to add platform device\n");
	platform_device_put(hub_snd_device);

	return ret;
}

static void __exit hub_soc_exit(void)
{
	DBG("\n");

	platform_device_unregister(hub_snd_device);
}

module_init(hub_soc_init);
module_exit(hub_soc_exit);

MODULE_AUTHOR("LG Electronics");
MODULE_DESCRIPTION("ALSA SoC HUB");
MODULE_LICENSE("GPL");
// prime@sdcmicro.com Reworked for 2.6.35 [END]

