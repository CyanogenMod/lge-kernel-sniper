/*
 * zoom2.c  --  SoC audio for Zoom2
 *
 * Author: Misael Lopez Cruz <x0052729@ti.com>
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

#include <linux/clk.h>
#include <linux/platform_device.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>

#include <asm/mach-types.h>
#include <mach/hardware.h>
#include <mach/gpio.h>
#include <mach/board-zoom.h>
#include <plat/mcbsp.h>

/* Register descriptions for twl4030 codec part */
#include <linux/mfd/twl4030-codec.h>

#include "omap-mcbsp.h"
#include "omap-pcm.h"

#define ZOOM2_BT_MCBSP_GPIO		164
#define ZOOM2_HEADSET_MUX_GPIO		(OMAP_MAX_GPIO_LINES + 15)
#define ZOOM2_HEADSET_EXTMUTE_GPIO      153

static struct snd_soc_dai_link zoom2_dai[];
static int zoom2_hifi_playback_state;
static int zoom2_voice_state;
static int zoom2_capture_state;

struct snd_soc_pcm_runtime *g_rtd;

extern int twl4030_set_ext_clock(struct snd_soc_codec *codec, int enable);

static int zoom2_hw_params(struct snd_pcm_substream *substream,
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

	/*
	 * Set headset EXTMUTE signal to ON to make sure we
	 * get correct headset status
	 */
	gpio_direction_output(ZOOM2_HEADSET_EXTMUTE_GPIO, 1);

	return 0;
}

static struct snd_soc_ops zoom2_ops = {
	.hw_params = zoom2_hw_params,
};

/* Audio Sampling frequences supported by Triton */
static const char *audio_sample_rates_txt[] = {
	"8000", "11025", "12000", "16000", "22050",
	"24000", "32000", "44100", "48000", "96000"
	};

/*
 * APLL_RATE defined in CODEC_MODE register, which corresponds
 * to the sampling rates defined above
 */
static const unsigned int audio_sample_rates_apll[] = {
	0x0, 0x1, 0x2, 0x4, 0x5,
	0x6, 0x8, 0x9, 0xa, 0xe
	};

/* Voice Sampling rates supported by Triton */
static const char *voice_sample_rates_txt[] = {
	"8000", "16000"
	};

/*
 * SEL_16K defined in CODEC_MODE register, which corresponds
 * to the voice sample rates defined above
 */
static const unsigned int voice_sample_rates_sel_16k[] = {
	0x0, 0x1
	};

static const struct soc_enum twl4030_audio_sample_rates_enum =
	SOC_VALUE_ENUM_SINGLE(TWL4030_REG_CODEC_MODE, 4, 0xf,
			ARRAY_SIZE(audio_sample_rates_txt),
			audio_sample_rates_txt, audio_sample_rates_apll);

static const struct soc_enum twl4030_voice_sample_rates_enum =
	SOC_VALUE_ENUM_SINGLE(TWL4030_REG_CODEC_MODE, 3, 0x1,
			ARRAY_SIZE(voice_sample_rates_txt),
			voice_sample_rates_txt, voice_sample_rates_sel_16k);

static int zoom2_hw_voice_params(struct snd_pcm_substream *substream,
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

static struct snd_soc_ops zoom2_voice_ops = {
	.hw_params = zoom2_hw_voice_params,
};

/* Zoom2 machine DAPM */
static const struct snd_soc_dapm_widget zoom2_twl4030_dapm_widgets[] = {
	SND_SOC_DAPM_MIC("Ext Mic", NULL),
	SND_SOC_DAPM_SPK("Ext Spk", NULL),
	SND_SOC_DAPM_MIC("Headset Mic", NULL),
	SND_SOC_DAPM_HP("Headset Stereophone", NULL),
	SND_SOC_DAPM_LINE("Aux In", NULL),
};

static const struct snd_soc_dapm_route audio_map[] = {
	/* External Mics: MAINMIC, SUBMIC with bias*/
	{"MAINMIC", NULL, "Mic Bias 1"},
	{"SUBMIC", NULL, "Mic Bias 2"},
	{"Mic Bias 1", NULL, "Ext Mic"},
	{"Mic Bias 2", NULL, "Ext Mic"},

	/* External Speakers: HFL, HFR */
	{"Ext Spk", NULL, "HFL"},
	{"Ext Spk", NULL, "HFR"},

	/* Headset Stereophone:  HSOL, HSOR */
	{"Headset Stereophone", NULL, "HSOL"},
	{"Headset Stereophone", NULL, "HSOR"},

	/* Headset Mic: HSMIC with bias */
	{"HSMIC", NULL, "Headset Mic Bias"},
	{"Headset Mic Bias", NULL, "Headset Mic"},

	/* Aux In: AUXL, AUXR */
	{"Aux In", NULL, "AUXL"},
	{"Aux In", NULL, "AUXR"},
};

static int zoom2_get_hifi_playback_state(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	ucontrol->value.integer.value[0] = zoom2_hifi_playback_state;
	return 0;
}

static int zoom2_set_hifi_playback_state(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_dapm_widget *widget = snd_kcontrol_chip(kcontrol);
	int ret;

	if (zoom2_hifi_playback_state == ucontrol->value.integer.value[0])
		return 0;

	if (ucontrol->value.integer.value[0]) {
		/* this info is in the twl4030_dai ... */
		ret = snd_soc_dapm_stream_event(g_rtd, 1, "HiFi Playback",
			SND_SOC_DAPM_STREAM_START);
	} else {
	       ret = snd_soc_dapm_stream_event(g_rtd, 1, "HiFi Playback",
			SND_SOC_DAPM_STREAM_STOP);
	}

	if (ret != 0) {
		printk(KERN_ERR "failed to set hifi playback state\n");
		return 0;
	}

	zoom2_hifi_playback_state = ucontrol->value.integer.value[0];
	return 1;
}

static int zoom2_get_voice_state(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	ucontrol->value.integer.value[0] = zoom2_voice_state;
	return 0;
}

static int zoom2_set_voice_state(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	int ret;
	struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol);

	if (zoom2_voice_state == ucontrol->value.integer.value[0])
		return 0;

	if (ucontrol->value.integer.value[0]) {
		ret = snd_soc_dapm_stream_event(g_rtd, 1, "Voice Playback",
			SND_SOC_DAPM_STREAM_START);

		/* Enable voice digital filters */
		snd_soc_update_bits(codec, TWL4030_REG_OPTION,
			TWL4030_ARXL1_VRX_EN, 0x10);
	} else {
	       ret = snd_soc_dapm_stream_event(g_rtd, 1, "Voice Playback",
			SND_SOC_DAPM_STREAM_STOP);

		/* Disable voice digital filters */
		snd_soc_update_bits(codec, TWL4030_REG_OPTION,
			TWL4030_ARXL1_VRX_EN, 0x0);
	}

	if (ret != 0) {
		printk(KERN_ERR "failed to set voice playback state\n");
		return 0;
	}

	zoom2_voice_state = ucontrol->value.integer.value[0];
	return 1;
}

static int zoom2_get_capture_state(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	ucontrol->value.integer.value[0] = zoom2_capture_state;
	return 0;
}

static int zoom2_set_capture_state(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	int ret;

	if (zoom2_capture_state == ucontrol->value.integer.value[0])
		return 0;

	if (ucontrol->value.integer.value[0]) {
		ret = snd_soc_dapm_stream_event(g_rtd, 1, "Capture",
			SND_SOC_DAPM_STREAM_START);
	} else {
	       ret = snd_soc_dapm_stream_event(g_rtd, 1, "Capture",
			SND_SOC_DAPM_STREAM_STOP);
	}

	if (ret != 0) {
		printk(KERN_ERR "failed to set capture state\n");
		return 0;
	}

	zoom2_capture_state = ucontrol->value.integer.value[0];
	return 1;
}

static const char *path_control[] = {"Off", "On"};

static const struct soc_enum zoom2_enum[] = {
	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(path_control), path_control),
};

static const struct snd_kcontrol_new zoom2_controls[] = {
	SOC_ENUM_EXT("HIFI Playback Control", zoom2_enum[0],
		zoom2_get_hifi_playback_state, zoom2_set_hifi_playback_state),
	SOC_ENUM_EXT("Voice Control", zoom2_enum[0],
		zoom2_get_voice_state, zoom2_set_voice_state),
	SOC_ENUM_EXT("Capture Control", zoom2_enum[0],
		zoom2_get_capture_state, zoom2_set_capture_state),
	SOC_ENUM_EXT("Audio Sample Rate", twl4030_audio_sample_rates_enum,
		snd_soc_get_value_enum_double, snd_soc_put_value_enum_double),
	SOC_ENUM_EXT("Voice Sample Rate", twl4030_voice_sample_rates_enum,
		snd_soc_get_value_enum_double, snd_soc_put_value_enum_double),
	SOC_SINGLE("256FS CLK Control Switch", TWL4030_REG_AUDIO_IF, 1, 1, 0),
};

static int zoom2_twl4030_init(struct snd_soc_pcm_runtime *rtd)
{
	struct snd_soc_codec *codec = rtd->codec;
	int ret;
	unsigned short reg;

	g_rtd = rtd;

	/* Add ZOOM2 specific controls */
	ret = snd_soc_add_controls(codec, zoom2_controls,
			ARRAY_SIZE(zoom2_controls));
	if (ret)
		return ret;

	/* Add Zoom2 specific widgets */
	ret = snd_soc_dapm_new_controls(codec->dapm, zoom2_twl4030_dapm_widgets,
				ARRAY_SIZE(zoom2_twl4030_dapm_widgets));
	if (ret)
		return ret;

	/* Set up Zoom2 specific audio path audio_map */
	snd_soc_dapm_add_routes(codec->dapm, audio_map, ARRAY_SIZE(audio_map));

	reg = codec->driver->read(codec, TWL4030_REG_VOICE_IF);
	reg |= TWL4030_VIF_DIN_EN | TWL4030_VIF_DOUT_EN | TWL4030_VIF_EN;
	codec->driver->write(codec, TWL4030_REG_VOICE_IF, reg);

	/* Zoom2 connected pins */
	snd_soc_dapm_enable_pin(codec->dapm, "Ext Mic");
	snd_soc_dapm_enable_pin(codec->dapm, "Ext Spk");
	snd_soc_dapm_enable_pin(codec->dapm, "Headset Mic");
	snd_soc_dapm_enable_pin(codec->dapm, "Headset Stereophone");
	snd_soc_dapm_enable_pin(codec->dapm, "Aux In");

	/* TWL4030 not connected pins */
	snd_soc_dapm_nc_pin(codec->dapm, "CARKITMIC");
	snd_soc_dapm_nc_pin(codec->dapm, "DIGIMIC0");
	snd_soc_dapm_nc_pin(codec->dapm, "DIGIMIC1");

	snd_soc_dapm_nc_pin(codec->dapm, "OUTL");
	snd_soc_dapm_nc_pin(codec->dapm, "OUTR");
	snd_soc_dapm_nc_pin(codec->dapm, "EARPIECE");
	snd_soc_dapm_nc_pin(codec->dapm, "PREDRIVEL");
	snd_soc_dapm_nc_pin(codec->dapm, "PREDRIVER");
	snd_soc_dapm_nc_pin(codec->dapm, "CARKITL");
	snd_soc_dapm_nc_pin(codec->dapm, "CARKITR");

	ret = snd_soc_dapm_sync(codec->dapm);

	return ret;
}

static int zoom2_twl4030_voice_init(struct snd_soc_pcm_runtime *rtd)
{
	struct snd_soc_codec *codec = rtd->codec;
	unsigned short reg;
	int     ret = 0;

	/* Enable voice interface */
	reg = codec->driver->read(codec, TWL4030_REG_VOICE_IF);
	reg |= TWL4030_VIF_DIN_EN | TWL4030_VIF_DOUT_EN | TWL4030_VIF_EN;
	codec->driver->write(codec, TWL4030_REG_VOICE_IF, reg);

	/* hifi state */

	ret = snd_soc_dapm_stream_event(rtd,
			1,
			"Capture",
			SND_SOC_DAPM_STREAM_START);

	/* voice state */
	/* this is not dynamic, so ignore dir */
	ret = snd_soc_dapm_stream_event(rtd,
			1,
			"TWL4030 Voice",
			SND_SOC_DAPM_STREAM_START);

		/* Enable voice digital filters */
	snd_soc_update_bits(codec, TWL4030_REG_OPTION,
			TWL4030_ARXL1_VRX_EN, 0x10);

	ret = snd_soc_dapm_stream_event(rtd,
			1,
			"TWL4030 Voice",
			SND_SOC_DAPM_STREAM_START);

	/* reinit defaults */

	snd_soc_update_bits(codec, 1, 1, 0);
	snd_soc_update_bits(codec, 17, 63, 50);
	snd_soc_update_bits(codec, 16, 63, 50);

	snd_soc_update_bits(codec, 19, 63, 45);
	snd_soc_update_bits(codec, 18, 63, 45);

	snd_soc_update_bits(codec, 17, 192, 128);
	snd_soc_update_bits(codec, 16, 192, 128);
	snd_soc_update_bits(codec, 19, 192, 128);
	snd_soc_update_bits(codec, 18, 192, 128);

	snd_soc_update_bits(codec, 25, 248, 0);
	snd_soc_update_bits(codec, 26, 248, 0);

	snd_soc_update_bits(codec, 27, 248, 32);
	snd_soc_update_bits(codec, 28, 248, 32);

	snd_soc_update_bits(codec, 25, 2, 0);
	snd_soc_update_bits(codec, 26, 2, 0);
	snd_soc_update_bits(codec, 27, 2, 2);
	snd_soc_update_bits(codec, 28, 2, 2);

	snd_soc_update_bits(codec, 20, 63, 36);

	snd_soc_update_bits(codec, 68, 2, 0);

	snd_soc_update_bits(codec, 37, 48, 16);
	snd_soc_update_bits(codec, 37, 48, 16);

	snd_soc_update_bits(codec, 38, 48, 16);

	snd_soc_update_bits(codec, 35, 15, 5);

	snd_soc_update_bits(codec, 39, 48, 16);
	snd_soc_update_bits(codec, 40, 48, 16);

	snd_soc_update_bits(codec, 33, 48, 16);

	snd_soc_update_bits(codec, 10, 31, 23);
	snd_soc_update_bits(codec, 11, 31, 23);
	snd_soc_update_bits(codec, 12, 31, 23);
	snd_soc_update_bits(codec, 13, 31, 23);

	snd_soc_update_bits(codec, 72, 63, 27);

	snd_soc_update_bits(codec, 7, 4, 0);

	snd_soc_update_bits(codec, 36, 28, 12);

	snd_soc_update_bits(codec, 69, 32, 0);
	snd_soc_update_bits(codec, 69, 2, 0);

	return 0;
}

/* Digital audio interface glue - connects codec <--> CPU */
static struct snd_soc_dai_link zoom2_dai[] = {
	{
		.name = "TWL4030 I2S",
		.stream_name = "TWL4030 Audio",
		.cpu_dai_name = "omap-mcbsp-dai.1",
		.codec_dai_name = "twl4030-hifi",
		.platform_name = "omap-pcm-audio",
		.codec_name = "twl4030-codec",
		.init = zoom2_twl4030_init,
		.ops = &zoom2_ops,
	},
	{
		.name = "TWL4030 VOICE",
		.stream_name = "TWL4030 Voice",
		.cpu_dai_name = "omap-mcbsp-dai.2",
		.codec_dai_name = "twl4030-voice",
		.platform_name = "omap-pcm-audio",
		.codec_name = "twl4030-codec",
		.init = zoom2_twl4030_voice_init,
		.ops = &zoom2_voice_ops,
	},
	{
		.name = "TWL4030_PCM",
		.stream_name = "TWL4030 Clock",
		.cpu_dai_name = "omap-mcbsp-dai.3",
		.codec_dai_name = "twl4030-clock",
		.platform_name = "omap-pcm-audio",
		.codec_name = "twl4030-codec",
		.init = zoom2_twl4030_voice_init,
		.ops = &zoom2_voice_ops,
	},
};

/* Audio machine driver */
static struct snd_soc_card snd_soc_zoom2 = {
	.name = "Zoom2",
	.long_name = "Zoom2 (twl4030)",
	.dai_link = zoom2_dai,
	.num_links = ARRAY_SIZE(zoom2_dai),
};

/* EXTMUTE callback function */
void zoom2_set_hs_extmute(int mute)
{
	gpio_set_value(ZOOM2_HEADSET_EXTMUTE_GPIO, mute);
}

struct twl4030_setup_data {
	unsigned int ramp_delay_value;
	unsigned int sysclk;
	unsigned int hs_extmute:1;
	void (*set_hs_extmute)(int mute);
};

/* twl4030 setup */
static struct twl4030_setup_data twl4030_setup = {
	.ramp_delay_value = 3,  /* 161 ms */
	.sysclk = 26000,
	/*.hs_extmute = 1, */
	.hs_extmute = 0,
	.set_hs_extmute = zoom2_set_hs_extmute,
};

static struct platform_device *zoom2_snd_device;

static int __init zoom2_soc_init(void)
{
	int ret;

	if (!machine_is_omap_zoom2() && !machine_is_omap_zoom3()) {
		pr_debug("Not Zoom2/3!\n");
		return -ENODEV;
	}
	printk(KERN_INFO "Zoom2 SoC init\n");

	zoom2_snd_device = platform_device_alloc("soc-audio", -1);
	if (!zoom2_snd_device) {
		printk(KERN_ERR "Platform device allocation failed\n");
		return -ENOMEM;
	}

	platform_set_drvdata(zoom2_snd_device, &snd_soc_zoom2);
	ret = platform_device_add(zoom2_snd_device);
	if (ret)
		goto err1;

	BUG_ON(gpio_request(ZOOM2_HEADSET_MUX_GPIO, "hs_mux") < 0);
	gpio_direction_output(ZOOM2_HEADSET_MUX_GPIO, 0);

	BUG_ON(gpio_request(ZOOM2_HEADSET_EXTMUTE_GPIO, "ext_mute") < 0);
	gpio_direction_output(ZOOM2_HEADSET_EXTMUTE_GPIO, 0);

	return 0;

err1:
	printk(KERN_ERR "Unable to add platform device\n");
	platform_device_put(zoom2_snd_device);

	return ret;
}
module_init(zoom2_soc_init);

static void __exit zoom2_soc_exit(void)
{
	gpio_free(ZOOM2_HEADSET_MUX_GPIO);
	gpio_free(ZOOM2_HEADSET_EXTMUTE_GPIO);

	platform_device_unregister(zoom2_snd_device);
}
module_exit(zoom2_soc_exit);

MODULE_AUTHOR("Misael Lopez Cruz <x0052729@ti.com>");
MODULE_DESCRIPTION("ALSA SoC Zoom2");
MODULE_LICENSE("GPL");

