
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
#include "../../../arch/arm/mach-omap2/mux.h"

#if defined(CONFIG_HUB_AMP_WM9093)
#include <mach/wm9093.h>
#endif


/* Register descriptions for twl4030 codec part */
#include <linux/mfd/twl4030-codec.h>
voice_mode_enum cur_voice_mode=0; 

extern int twl4030_set_ext_clock(struct snd_soc_codec *codec, int enable);

static int hub_i2s_hw_params(struct snd_pcm_substream *substream,
				struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *codec_dai = rtd->codec_dai;
	struct snd_soc_dai *cpu_dai = rtd->cpu_dai;
	int ret;

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

	return 0;
}

int hub_i2s_hw_free(struct snd_pcm_substream *substream)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *cpu_dai = rtd->cpu_dai;
	int ret;

	printk("[LUCKYJUN77] hub_i2s_hw_free\n");

	/* Use function clock for mcBSP2 */
	ret = snd_soc_dai_set_sysclk(cpu_dai, OMAP_MCBSP_SYSCLK_CLKS_FCLK,
			0, SND_SOC_CLOCK_OUT);
	return 0;
}

static struct snd_soc_ops hub_i2s_ops = {
	.hw_params = hub_i2s_hw_params,
	.hw_free = hub_i2s_hw_free,
};

static int hub_pcm_hw_params(struct snd_pcm_substream *substream,
	struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *codec_dai = rtd->codec_dai;
	struct snd_soc_dai *cpu_dai = rtd->cpu_dai;
	int ret;

	/* Set codec DAI configuration */
	ret = snd_soc_dai_set_fmt(codec_dai,
				SND_SOC_DAIFMT_DSP_A |
				SND_SOC_DAIFMT_IB_NF |
				SND_SOC_DAIFMT_CBM_CFM);
	if (ret) {
		printk(KERN_ERR "can't set codec DAI configuration\n");
		return ret;
	}

	/* Set cpu DAI configuration */
	ret = snd_soc_dai_set_fmt(cpu_dai,
				SND_SOC_DAIFMT_DSP_A |
				SND_SOC_DAIFMT_IB_NF |
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

	return 0;
}

static struct snd_soc_ops hub_pcm_ops = {
	.hw_params = hub_pcm_hw_params,
};

#if defined(CONFIG_MACH_LGE_HUB)
extern void hub_set_call_mode(struct snd_pcm_substream *substream, int mode);

static int hub_call_hw_params(struct snd_pcm_substream *substream,
	struct snd_pcm_hw_params *params)
{
	hub_set_call_mode(substream, 1);

	return 0;
}

int hub_call_hw_free(struct snd_pcm_substream *substream)
{
	hub_set_call_mode(substream, 0);

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
//	struct snd_soc_dai *cpu_dai = rtd->dai->cpu_dai;
	struct snd_soc_dai *cpu_dai = rtd->cpu_dai;	
	int ret;

	/* Set cpu DAI configuration */
	ret = snd_soc_dai_set_fmt(cpu_dai,
				  SND_SOC_DAIFMT_I2S |
				  SND_SOC_DAIFMT_NB_NF |
				  SND_SOC_DAIFMT_CBM_CFM);
	if (ret < 0) {
		printk(KERN_ERR "can't set cpu DAI configuration\n");
		return ret;
	}
	return 0;
}

static struct snd_soc_ops hub_fm_ops = {
	.hw_params = hub_fm_hw_params,
};


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
	ucontrol->value.integer.value[0] = wm9093_get_curmode();
	return 0;
}

static int ext_amp_set_mode(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	wm9093_configure_path((wm9093_mode_enum)ucontrol->value.integer.value[0]);
	
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

static const char *ext_amp_mode_texts[] =
		{"OFF", "Headset", "Speaker", "Dual", "Bypass", "Spkcall", "Headsetcall", "BypassVOIP", "SpkcallVOIP", "HeadsetcallVOIP","SpkFMR","HeadsetFMR"};


static const struct soc_enum ext_amp_mode_soc_enum =
	SOC_ENUM_SINGLE_EXT(12, ext_amp_mode_texts);

static const struct snd_kcontrol_new ext_amp_mode_control =
	SOC_ENUM_EXT("Route", ext_amp_mode_soc_enum, ext_amp_get_mode, ext_amp_set_mode);


static int voice_get_mode(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	ucontrol->value.integer.value[0] = voice_get_curmode();
	return 0;
}


int cur_mode_check;
static int voice_set_mode(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
//    printk("[BIZZ]voice set mode=%d\n",(voice_mode_enum)ucontrol->value.integer.value[0]);
	voice_configure_path((voice_mode_enum)ucontrol->value.integer.value[0]);
	cur_voice_mode=(voice_mode_enum)ucontrol->value.integer.value[0];
	cur_mode_check=cur_voice_mode;
	
	return 1;
}


static const char *voice_mode_texts[] =
		{"OFF", "Headset", "Speaker", "Receiver", "Headphone"};

static const struct soc_enum voice_mode_soc_enum =
	SOC_ENUM_SINGLE_EXT(5, voice_mode_texts);

static const struct snd_kcontrol_new voice_mode_control =
	SOC_ENUM_EXT("Route", voice_mode_soc_enum, voice_get_mode, voice_set_mode);



static int mic_get_mode(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	ucontrol->value.integer.value[0] = mic_get_curmode();
	return 0;
}

static int mic_set_mode(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	mic_configure_path((mic_mode_enum)ucontrol->value.integer.value[0]);
	
	return 1;
}

static const char *mic_mode_texts[] =
		{"OFF", "Restore"};

static const struct soc_enum mic_mode_soc_enum =
	SOC_ENUM_SINGLE_EXT(2, mic_mode_texts);

static const struct snd_kcontrol_new mic_mode_control =
	SOC_ENUM_EXT("Route", mic_mode_soc_enum, mic_get_mode, mic_set_mode);



static int callrec_get_mode(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	ucontrol->value.integer.value[0] = callrec_get_curmode();
	return 0;
}

static int callrec_set_mode(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	printk("[LUCKYJUN77] callrec_set_mode : %d\n", (callrec_mode_enum)ucontrol->value.integer.value[0]);
	
	callrec_configure_path((callrec_mode_enum)ucontrol->value.integer.value[0]);
	
	return 1;
}

static const char *callrec_mode_texts[] = {"BuiltIn", "Headset", "OFF"};

static const struct soc_enum callrec_mode_soc_enum =
	SOC_ENUM_SINGLE_EXT(3, callrec_mode_texts);

static const struct snd_kcontrol_new callrec_mode_control =
	SOC_ENUM_EXT("Route", callrec_mode_soc_enum, callrec_get_mode, callrec_set_mode);



static int fmradio_set_volume(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol)
{
       wm9093_fmradio_volume((wm9093_fmvolume_enum)ucontrol->value.integer.value[0]);
	return 1;
}
static int fmradio_get_volume(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol)
{
         return 0;
}
static const char *FMradio_volume_texts[]={
  "OFF",
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
 


static int set_mic_switch(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol)
{
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

#if defined (CONFIG_LGE_LAB3_BOARD) //[LG_FW_AUDIO_TTY Start] - jungsoo1221.lee
static int set_tty_mode(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol)
{
       twl4030_set_tty_mode((int)ucontrol->value.integer.value[0]);
	return 1;
}

static int get_tty_mode(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol)
{
         return 0;
}  
static const char *Tty_mode_texts[]={
  "TTY_OFF",
  "TTY_VCO",
  "TTY_HCO",
  "TTY_FULL"
};
static const struct soc_enum Tty_mode_soc_enum =
SOC_ENUM_SINGLE_EXT(4, Tty_mode_texts);
static const struct snd_kcontrol_new tty_mode_control =
SOC_ENUM_EXT("Route", Tty_mode_soc_enum, get_tty_mode, set_tty_mode);
#endif //[LG_FW_AUDIO_TTY End] 

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
//20101205 inbang.park@lge.com Add STREAM  for  FM Radio [START] 
	SND_SOC_DAPM_MUX("FMradio", SND_SOC_NOPM, 0, 0, &FM_volume_control),
//20101205 inbang.park@lge.com Add STREAM  for  FM Radio [END] 
	SND_SOC_DAPM_MUX("MIC Switch", SND_SOC_NOPM, 0, 0, &mic_switch_control),  //jongik2.kim 20101220 add mic2 control
#if defined (CONFIG_LGE_LAB3_BOARD) //[LG_FW_AUDIO_TTY] - jungsoo1221.lee
	SND_SOC_DAPM_MUX("TTY Mode", SND_SOC_NOPM, 0, 0, &tty_mode_control),
#endif
};

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
#if 0 
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

static int hub_twl4030_init(struct snd_soc_pcm_runtime *rtd)
{
	struct snd_soc_codec *codec = rtd->codec;
	int ret;

	/* Add hub specific widgets */
	ret = snd_soc_dapm_new_controls(codec->dapm, hub_twl4030_dapm_widgets,
				ARRAY_SIZE(hub_twl4030_dapm_widgets));
	if (ret)
		return ret;

	/* Set up hub specific audio path audio_map */
	snd_soc_dapm_add_routes(codec->dapm, audio_map, ARRAY_SIZE(audio_map));

	/* hub connected pins */
#if 0 
	snd_soc_dapm_enable_pin(codec->dapm, "Ext Mic");
#else
	snd_soc_dapm_enable_pin(codec->dapm, "Ext Mic1");
	snd_soc_dapm_enable_pin(codec->dapm, "Ext Mic2");
	snd_soc_dapm_enable_pin(codec->dapm, "Ext Headset Mic");	
#endif
	snd_soc_dapm_enable_pin(codec->dapm, "Ext Spk");
	snd_soc_dapm_enable_pin(codec->dapm, "Receiver");
	snd_soc_dapm_enable_pin(codec->dapm, "Headset Jack");
	snd_soc_dapm_enable_pin(codec->dapm, "Aux In");

	/* TWL4030 not connected pins */
	snd_soc_dapm_nc_pin(codec->dapm, "CARKITMIC");
	snd_soc_dapm_nc_pin(codec->dapm, "DIGIMIC0");
	snd_soc_dapm_nc_pin(codec->dapm, "DIGIMIC1");

	snd_soc_dapm_nc_pin(codec->dapm, "OUTL");
	snd_soc_dapm_nc_pin(codec->dapm, "OUTR");
#if 0 
	snd_soc_dapm_nc_pin(codec->dapm, "EARPIECE");
	snd_soc_dapm_nc_pin(codec->dapm, "PREDRIVEL");
	snd_soc_dapm_nc_pin(codec->dapm, "PREDRIVER");
#else
	snd_soc_dapm_nc_pin(codec->dapm, "HFL");
	snd_soc_dapm_nc_pin(codec->dapm, "HFR");
#endif
	snd_soc_dapm_nc_pin(codec->dapm, "CARKITL");
	snd_soc_dapm_nc_pin(codec->dapm, "CARKITR");

	ret = snd_soc_dapm_sync(codec->dapm);

	return ret;
}

static int hub_twl4030_voice_init(struct snd_soc_pcm_runtime *rtd)
{
	struct snd_soc_codec *codec = rtd->codec;
	unsigned short reg;

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
#if defined(CONFIG_MACH_LGE_HUB) 
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

	return 0;

err1:
	printk(KERN_ERR "Unable to add platform device\n");
	platform_device_put(hub_snd_device);

	return ret;
}

static void __exit hub_soc_exit(void)
{
	platform_device_unregister(hub_snd_device);
}

module_init(hub_soc_init);
module_exit(hub_soc_exit);

MODULE_AUTHOR("LG Electronics");
MODULE_DESCRIPTION("ALSA SoC HUB");
MODULE_LICENSE("GPL");


