#ifndef _LINUX_WM9093_I2C_H
#define _LINUX_WM9093_I2C_H

#define WM9093_I2C_NAME "wm9093"
#define WM9093_I2C_ADDR	0x6E

typedef enum {
    OFF_MODE,
    HEADSET_AUDIO_MODE,
    SPEAKER_AUDIO_MODE,
    SPEAKER_HEADSET_DUAL_AUDIO_MODE,
    RECEIVER_CALL_MODE,
    SPEAKER_CALL_MODE,
    HEADSET_CALL_MODE,
    RECEIVER_VOIP_MODE,		//20101120 junyeop.kim@lge.com, voip call tuning[START_LGE]
    SPEAKER_VOIP_MODE,
    HEADSET_VOIP_MODE,//20101120 junyeop.kim@lge.com, voip call tuning[END_LGE]
#if defined(CONFIG_PRODUCT_LGE_LU6800) || defined(CONFIG_PRODUCT_LGE_KU5900) || defined (CONFIG_PRODUCT_LGE_P970)
//20101205 inbang.park@lge.com Add STREAM  for  FM Radio [START] 
    SPEAKER_FMR_MODE,
    HEADSET_FMR_MODE
//20101205 inbang.park@lge.com Add STREAM  for  FM Radio [END] 
#endif // defined(CONFIG_PRODUCT_LGE_LU6800) || defined(CONFIG_PRODUCT_LGE_KU5900) || defined (CONFIG_PRODUCT_LGE_P970)
}wm9093_mode_enum;

#if defined(CONFIG_PRODUCT_LGE_LU6800) || defined(CONFIG_PRODUCT_LGE_KU5900) || defined (CONFIG_PRODUCT_LGE_P970)
//20101205 inbang.park@lge.com Add STREAM  for  FM Radio [START] 
typedef enum{
  LEVEL_OFF,//201022 inbang.park@lge.com Wake lock for FM radio
  LEVEL_1,
  LEVEL_2,
  LEVEL_3,
  LEVEL_4,
  LEVEL_5,
  LEVEL_6,
  LEVEL_7,
  LEVEL_8,
  LEVEL_9,
  LEVEL_10,
  LEVEL_11,
  LEVEL_12,
  LEVEL_13,
  LEVEL_14,
  LEVEL_15,
  LEVEL_reset
}wm9093_fmvolume_enum;
//20101205 inbang.park@lge.com Add STREAM  for  FM Radio [END] 
#endif	// defined(CONFIG_PRODUCT_LGE_LU6800) || defined(CONFIG_PRODUCT_LGE_KU5900) || defined (CONFIG_PRODUCT_LGE_P970)
typedef enum
{
    WM9093_CMD,
    WM9093_DELAY,
    WM9093_END_SEQ
} wm9093_reg_control_type;

typedef struct
{
  wm9093_reg_control_type irc;
  u8 address;
  int data;
} wm9093_reg_type;

struct wm9093_device {
	wm9093_mode_enum wm9093_mode;
	int wm9093_pstatus;
	struct device dev;
};

struct wm9093_i2c_device {
	struct i2c_client *client;
	struct wm9093_device *wm9093_dev;
	struct delayed_work delayed_work;		//junyeop.kim@lge.com, call initial noise workaround
};

extern void wm9093_configure_path(wm9093_mode_enum mode);
#if defined(CONFIG_PRODUCT_LGE_LU6800) || defined(CONFIG_PRODUCT_LGE_KU5900) || defined (CONFIG_PRODUCT_LGE_P970)
//20101205 inbang.park@lge.com Add STREAM  for  FM Radio [START] 
extern void wm9093_fmradio_volume(wm9093_fmvolume_enum volume);
//20101222 inbang.park@lge.com Wake lock for  FM Radio [START]
extern void fmradio_configure_path(wm9093_fmvolume_enum mode);
extern int fmradio_get_curmode(void);
//20101222 inbang.park@lge.com Wake lock for  FM Radio [END]
//20101205 inbang.park@lge.com Add STREAM  for  FM Radio [END] 
#endif // defined(CONFIG_PRODUCT_LGE_LU6800) || defined(CONFIG_PRODUCT_LGE_KU5900) || defined (CONFIG_PRODUCT_LGE_P970)
extern int wm9093_get_curmode(void);
extern int wm9093_ext_suspend(void);
extern int wm9093_ext_resume(void);

#endif

