/**
 * Copyright (C) 2006 NXP N.V., All Rights Reserved.
 * This source code and any compilation or derivative thereof is the proprietary
 * information of NXP N.V. and is confidential in nature. Under no circumstances
 * is this software to be  exposed to or placed under an Open Source License of
 * any type without the expressed written permission of NXP N.V.
 *
 * Version       Revision: 1.0
 *
 * Date          Date: 27/10/09
 *
 * Brief         API for the TDA1998x HDMI Transmitters
 *
 **/

#include <linux/types.h>

#ifndef __tx_ioctl__
#define __tx_ioctl__

//#ifdef __tx_h__

#define TRANS_TYPE 1

#if TRANS_TYPE

#define EXAMPLE_MAX_SVD 30

/*
  trans-type
*/
typedef tmSWVersion_t tda_version;
typedef tmPowerState_t tda_power;
typedef tmdlHdmiTxInstanceSetupInfo_t tda_setup;
typedef tmdlHdmiTxCapabilities_t tda_capabilities;
typedef tmdlHdmiTxVideoOutConfig_t tda_video_out;
typedef tmdlHdmiTxVideoInConfig_t tda_video_in;
typedef tmdlHdmiTxSinkType_t tda_sink;
typedef tmdlHdmiTxAudioInConfig_t tda_audio_in;
typedef tmdlHdmiTxEdidAudioDesc_t tda_edid_audio_desc;
typedef tmdlHdmiTxShortVidDesc_t tda_edid_video_desc;
typedef tmdlHdmiTxEvent_t tda_event;
typedef tmdlHdmiTxInstanceSetupInfo_t tda_setup_info;
typedef tmdlHdmiTxEdidVideoTimings_t tda_edid_video_timings;
typedef tmdlHdmiTxPictAspectRatio_t tda_edid_tv_aspect_ratio;
typedef tmdlHdmiTxHdcpCheck_t tda_hdcp_status;
typedef tmdlHdmiTxHdcpStatus_t tda_hdcp_fail;
#ifdef TMFL_TDA19989
typedef tmdlHdmiTxEdidLatency_t tda_edid_latency;
#endif
// LGE Update - changhoony.lee@lge.com
typedef tmdlHdmiTxHotPlug_t tda_hpd_status;

// kyw2029
typedef PLAY_MODE_t play_mode;
typedef tmdlHdmiTxVidFmt_t tda_current_res;
// kyw2029

typedef struct {
   Bool enable;
   tmdlHdmiTxGamutData_t data;
} tda_gammut;

typedef struct {
   Bool enable;
   tmdlHdmiTxVsPktData_t data;
} tda_vs_infoframe;

typedef struct {
   Bool enable;
   tmdlHdmiTxSpdIfData_t data;
} tda_spd_infoframe;

typedef struct {
   Bool enable;
   tmdlHdmiTxMpsIfData_t data;
} tda_mps_infoframe;

typedef struct {
   Bool enable;
   tmdlHdmiTxIsrc1PktData_t data;
} tda_isrc1;

typedef struct {
   Bool enable;
   tmdlHdmiTxIsrc2PktData_t data;
} tda_isrc2;

typedef struct {
   Bool enable;
   tmdlHdmiTxAcpPktData_t data;
} tda_acp;

typedef struct {
   Bool enable;
   tmdlHdmiTxGcpPktData_t data;
} tda_gcp;

typedef struct {
   Bool enable;
   tmdlHdmiTxAviIfData_t data;
} tda_video_infoframe;

typedef struct {
   Bool enable;
   tmdlHdmiTxAudIfData_t data;
} tda_audio_infoframe;

typedef struct {
   tmdlHdmiTxVidFmt_t id;
   tmdlHdmiTxVidFmtSpecs_t spec;
} tda_video_format;

typedef struct {
   tda_video_in video_in;
   tda_video_out video_out;
   tda_audio_in audio_in; /* Mind tda_set_audio_in if you change this */
   tda_sink sink; /* Mind tda_set_audio_in if you change this */
} tda_set_in_out;

typedef struct {
   tda_audio_in audio_in;
   tda_sink sink;
} tda_set_audio_in;

typedef struct {
   tda_edid_audio_desc desc[EXAMPLE_MAX_SVD];
   unsigned int max;
   unsigned int written;
   unsigned char flags;
} tda_edid_audio_caps;

typedef struct {
   tda_edid_video_desc desc[EXAMPLE_MAX_SVD];
   unsigned int max;
   unsigned int written;
   unsigned char flags;
} tda_edid_video_caps;

typedef struct {
   tmdlHdmiTxEdidStatus_t status;
   unsigned char block_count;
} tda_edid;

typedef struct {
   tmdlHdmiTxEdidVideoTimings_t desc[EXAMPLE_MAX_SVD];
   unsigned char max;
   unsigned char written;
} tda_edid_dtd;

typedef struct {
   tmdlHdmiTxEdidFirstMD_t desc1[EXAMPLE_MAX_SVD];
   tmdlHdmiTxEdidSecondMD_t desc2[EXAMPLE_MAX_SVD];
   tmdlHdmiTxEdidOtherMD_t other[EXAMPLE_MAX_SVD];
   unsigned char max;
   unsigned char written;
} tda_edid_md;

#else 

#error do not compiled this !

typedef enum
{
    TDA_HDCP_ACTIVE         = 0,    /**< HDCP encryption status switched to active */
    TDA_HDCP_INACTIVE       = 1,    /**< HDCP encryption status switched to inactive */
    TDA_HPD_ACTIVE          = 2,    /**< Hotplug status switched to active */
    TDA_HPD_INACTIVE        = 3,    /**< Hotplug status switched to inactive */
    TDA_RX_KEYS_RECEIVED    = 4,    /**< Receiver(s) key(s) received */
    TDA_RX_DEVICE_ACTIVE    = 5,    /**< Rx device is connected and active */
    TDA_RX_DEVICE_INACTIVE  = 6,    /**< Rx device is connected but inactive (standby) */
    TDA_EDID_RECEIVED       = 7,    /**< EDID has been received */
    TDA_VS_RPT_RECEIVED     = 8,    /**< VS interrupt has been received */              
#ifdef HDMI_TX_REPEATER_ISR_MODE
    TDA_B_STATUS            = 9,    /**< TX received BStatus */
#endif /* HDMI_TX_REPEATER_ISR_MODE */
    TDA_DEBUG_EVENT_1       = 10     /**< This is a debug event */
} tda_event;

typedef struct {
    unsigned char format;         /* EIA/CEA861 mode */
    unsigned char channels;       /* number of channels */
    unsigned char supportedFreqs; /* bitmask of supported frequencies */
    unsigned char supportedRes;   /* bitmask of supported resolutions (LPCM only) */
    unsigned char maxBitrate;     /* Maximum bitrate divided by 8KHz (compressed formats only) */
} tda_edid_audio_desc;

typedef enum {
    TDA_EDID_READ                = 0,   /**< All blocks read OK */
    TDA_EDID_READ_INCOMPLETE     = 1,   /**< All blocks read OK but buffer too small to return all of them */
    TDA_EDID_ERROR_CHK_BLOCK_0   = 2,   /**< Block 0 checksum error */
    TDA_EDID_ERROR_CHK           = 3,   /**< Block 0 OK, checksum error in one or more other blocks */
    TDA_EDID_NOT_READ            = 4,   /**< EDID not read */
    TDA_EDID_STATUS_INVALID      = 5    /**< Invalid   */
} tda_edid_status;

typedef struct {
   int HBR;              /**< High Bitrate Audio packet */
   int DST;              /**< Direct Stream Transport audio packet */
   int oneBitAudio;      /**< One Bit Audio sample packet */
} tda_audio_packet;

typedef enum {
    TDA_AFMT_SPDIF      = 0, /**< SPDIF */
    TDA_AFMT_I2S        = 1, /**< I2S */
    TDA_AFMT_OBA        = 2, /**< One bit audio / DSD */
    TDA_AFMT_DST        = 3, /**< DST */
    TDA_AFMT_HBR        = 4  /**< HBR */
} tda_audio_format;

typedef enum {
    TDA_AFS_32K           = 0, /**< 32kHz    */
    TDA_AFS_44K           = 1, /**< 44.1kHz  */
    TDA_AFS_48K           = 2, /**< 48kHz    */
    TDA_AFS_88K           = 3, /**< 88.2kHz  */
    TDA_AFS_96K           = 4, /**< 96kHz    */
    TDA_AFS_176K          = 5, /**< 176.4kHz */
    TDA_AFS_192K          = 6  /**< 192kHz   */
} tda_audio_rate;

typedef enum {
    TDA_I2SQ_16BITS       = 16, /**< 16 bits */
    TDA_I2SQ_32BITS       = 32, /**< 32 bits */
    TDA_I2SQ_OTHERS       = 0   /**< for SPDIF and DSD */
} tda_audio_I2S_qualifier;

typedef enum {
    TDA_I2SFOR_PHILIPS_L   = 0,	/**< Philips like format */
    TDA_I2SFOR_OTH_L       = 2,	/**< Other non Philips left justified */
    TDA_I2SFOR_OTH_R	   = 3,	/**< Other non Philips right justified */
    TDA_I2SFOR_INVALID     = 4	/**< Invalid format */
} tda_audio_I2S_format;

typedef enum {
    TDA_DSTRATE_SINGLE  = 0,    /**< Single transfer rate */
    TDA_DSTRATE_DOUBLE  = 1     /**< Double data rate */
} tda_dst_rate;

typedef struct {
    int    simplayHd;          /**< Enable simplayHD support */
    int    repeaterEnable;     /**< Enable repeater mode */
    unsigned char   *pEdidBuffer;       /**< Pointer to raw EDID data */
    unsigned long  edidBufferSize;     /**< Size of buffer for raw EDID data */
} tda_instance_setup_info;

typedef enum {
    TDA_VFMT_NULL               = 0,    /**< Not a valid format...        */
    TDA_VFMT_NO_CHANGE          = 0,    /**< ...or no change required     */
    TDA_VFMT_MIN                = 1,    /**< Lowest valid format          */
    TDA_VFMT_TV_MIN             = 1,    /**< Lowest valid TV format       */
    TDA_VFMT_01_640x480p_60Hz   = 1,    /**< Format 01 640  x 480p  60Hz  */
    TDA_VFMT_02_720x480p_60Hz   = 2,    /**< Format 02 720  x 480p  60Hz  */
    TDA_VFMT_03_720x480p_60Hz   = 3,    /**< Format 03 720  x 480p  60Hz  */
    TDA_VFMT_04_1280x720p_60Hz  = 4,    /**< Format 04 1280 x 720p  60Hz  */
    TDA_VFMT_05_1920x1080i_60Hz = 5,    /**< Format 05 1920 x 1080i 60Hz  */
    TDA_VFMT_06_720x480i_60Hz   = 6,    /**< Format 06 720  x 480i  60Hz  */
    TDA_VFMT_07_720x480i_60Hz   = 7,    /**< Format 07 720  x 480i  60Hz  */
    TDA_VFMT_08_720x240p_60Hz   = 8,    /**< Format 08 720  x 240p  60Hz  */
    TDA_VFMT_09_720x240p_60Hz   = 9,    /**< Format 09 720  x 240p  60Hz  */
    TDA_VFMT_10_720x480i_60Hz   = 10,   /**< Format 10 720  x 480i  60Hz  */
    TDA_VFMT_11_720x480i_60Hz   = 11,   /**< Format 11 720  x 480i  60Hz  */
    TDA_VFMT_12_720x240p_60Hz   = 12,   /**< Format 12 720  x 240p  60Hz  */
    TDA_VFMT_13_720x240p_60Hz   = 13,   /**< Format 13 720  x 240p  60Hz  */
    TDA_VFMT_14_1440x480p_60Hz  = 14,   /**< Format 14 1440 x 480p  60Hz  */
    TDA_VFMT_15_1440x480p_60Hz  = 15,   /**< Format 15 1440 x 480p  60Hz  */
    TDA_VFMT_16_1920x1080p_60Hz = 16,   /**< Format 16 1920 x 1080p 60Hz  */
    TDA_VFMT_17_720x576p_50Hz   = 17,   /**< Format 17 720  x 576p  50Hz  */
    TDA_VFMT_18_720x576p_50Hz   = 18,   /**< Format 18 720  x 576p  50Hz  */
    TDA_VFMT_19_1280x720p_50Hz  = 19,   /**< Format 19 1280 x 720p  50Hz  */
    TDA_VFMT_20_1920x1080i_50Hz = 20,   /**< Format 20 1920 x 1080i 50Hz  */
    TDA_VFMT_21_720x576i_50Hz   = 21,   /**< Format 21 720  x 576i  50Hz  */
    TDA_VFMT_22_720x576i_50Hz   = 22,   /**< Format 22 720  x 576i  50Hz  */
    TDA_VFMT_23_720x288p_50Hz   = 23,   /**< Format 23 720  x 288p  50Hz  */
    TDA_VFMT_24_720x288p_50Hz   = 24,   /**< Format 24 720  x 288p  50Hz  */
    TDA_VFMT_25_720x576i_50Hz   = 25,   /**< Format 25 720  x 576i  50Hz  */
    TDA_VFMT_26_720x576i_50Hz   = 26,   /**< Format 26 720  x 576i  50Hz  */
    TDA_VFMT_27_720x288p_50Hz   = 27,   /**< Format 27 720  x 288p  50Hz  */
    TDA_VFMT_28_720x288p_50Hz   = 28,   /**< Format 28 720  x 288p  50Hz  */
    TDA_VFMT_29_1440x576p_50Hz  = 29,   /**< Format 29 1440 x 576p  50Hz  */
    TDA_VFMT_30_1440x576p_50Hz  = 30,   /**< Format 30 1440 x 576p  50Hz  */
    TDA_VFMT_31_1920x1080p_50Hz = 31,   /**< Format 31 1920 x 1080p 50Hz  */
    TDA_VFMT_32_1920x1080p_24Hz = 32,   /**< Format 32 1920 x 1080p 24Hz  */
    TDA_VFMT_33_1920x1080p_25Hz = 33,   /**< Format 33 1920 x 1080p 25Hz  */
    TDA_VFMT_34_1920x1080p_30Hz = 34,   /**< Format 34 1920 x 1080p 30Hz  */
    TDA_VFMT_TV_MAX             = 34,   /**< Highest valid TV format      */
    TDA_VFMT_TV_NO_REG_MIN      = 32,   /**< Lowest TV format without prefetched table */
    TDA_VFMT_TV_NUM             = 35,   /**< Number of TV formats & null  */
    TDA_VFMT_PC_MIN             = 128,  /**< Lowest valid PC format       */
    TDA_VFMT_PC_640x480p_60Hz   = 128,  /**< PC format 128                */
    TDA_VFMT_PC_800x600p_60Hz   = 129,  /**< PC format 129                */
    TDA_VFMT_PC_1152x960p_60Hz  = 130,  /**< PC format 130                */
    TDA_VFMT_PC_1024x768p_60Hz  = 131,  /**< PC format 131                */
    TDA_VFMT_PC_1280x768p_60Hz  = 132,  /**< PC format 132                */
    TDA_VFMT_PC_1280x1024p_60Hz = 133,  /**< PC format 133                */
    TDA_VFMT_PC_1360x768p_60Hz  = 134,  /**< PC format 134                */
    TDA_VFMT_PC_1400x1050p_60Hz = 135,  /**< PC format 135                */
    TDA_VFMT_PC_1600x1200p_60Hz = 136,  /**< PC format 136                */
    TDA_VFMT_PC_1024x768p_70Hz  = 137,  /**< PC format 137                */
    TDA_VFMT_PC_640x480p_72Hz   = 138,  /**< PC format 138                */
    TDA_VFMT_PC_800x600p_72Hz   = 139,  /**< PC format 139                */
    TDA_VFMT_PC_640x480p_75Hz   = 140,  /**< PC format 140                */
    TDA_VFMT_PC_1024x768p_75Hz  = 141,  /**< PC format 141                */
    TDA_VFMT_PC_800x600p_75Hz   = 142,  /**< PC format 142                */
    TDA_VFMT_PC_1024x864p_75Hz  = 143,  /**< PC format 143                */
    TDA_VFMT_PC_1280x1024p_75Hz = 144,  /**< PC format 144                */
    TDA_VFMT_PC_640x350p_85Hz   = 145,  /**< PC format 145                */
    TDA_VFMT_PC_640x400p_85Hz   = 146,  /**< PC format 146                */
    TDA_VFMT_PC_720x400p_85Hz   = 147,  /**< PC format 147                */
    TDA_VFMT_PC_640x480p_85Hz   = 148,  /**< PC format 148                */
    TDA_VFMT_PC_800x600p_85Hz   = 149,  /**< PC format 149                */
    TDA_VFMT_PC_1024x768p_85Hz  = 150,  /**< PC format 150                */
    TDA_VFMT_PC_1152x864p_85Hz  = 151,  /**< PC format 151                */
    TDA_VFMT_PC_1280x960p_85Hz  = 152,  /**< PC format 152                */
    TDA_VFMT_PC_1280x1024p_85Hz = 153,  /**< PC format 153                */
    TDA_VFMT_PC_1024x768i_87Hz  = 154,  /**< PC format 154                */
    TDA_VFMT_PC_MAX             = 154,  /**< Highest valid PC format      */
    TDA_VFMT_PC_NUM             = (1+154-128)   /**< Number of PC formats         */
} tda_video_fmt_id;

typedef struct {
    tda_video_fmt_id videoFormat;            /**< Video format as defined by EIA/CEA 861-D */
    int              nativeVideoFormat;   /**< True if format is the preferred video format */
} tda_edid_video_desc;

typedef struct {
    tda_video_fmt_id videoFormat;            /**< Video format as defined by EIA/CEA 861-D */
    int              nativeVideoFormat;   /**< True if format is the preferred video format */
} tda_short_video_desc;

typedef enum {
    TDA_P_ASPECT_RATIO_UNDEFINED    = 0,    /**< Undefined picture aspect ratio */
    TDA_P_ASPECT_RATIO_6_5          = 1,    /**< 6:5 picture aspect ratio (PAR) */
    TDA_P_ASPECT_RATIO_5_4          = 2,    /**< 5:4 PAR */
    TDA_P_ASPECT_RATIO_4_3          = 3,    /**< 4:3 PAR */
    TDA_P_ASPECT_RATIO_16_10        = 4,    /**< 16:10 PAR */
    TDA_P_ASPECT_RATIO_5_3          = 5,    /**< 5:3 PAR */
    TDA_P_ASPECT_RATIO_16_9         = 6,    /**< 16:9 PAR */
    TDA_P_ASPECT_RATIO_9_5          = 7     /**< 9:5 PAR */
} tda_pict_aspect_ratio;

typedef enum {
    TDA_VFREQ_24Hz      = 0,    /**< 24Hz          */
    TDA_VFREQ_25Hz      = 1,    /**< 25Hz          */
    TDA_VFREQ_30Hz      = 2,    /**< 30Hz          */
    TDA_VFREQ_50Hz      = 3,    /**< 50Hz          */
    TDA_VFREQ_59Hz      = 4,    /**< 59.94Hz       */
    TDA_VFREQ_60Hz      = 5,    /**< 60Hz          */
    TDA_VFREQ_70Hz      = 6,    /**< 70Hz          */
    TDA_VFREQ_72Hz      = 7,    /**< 72Hz          */
    TDA_VFREQ_75Hz      = 8,    /**< 75Hz          */
    TDA_VFREQ_85Hz      = 9,    /**< 85Hz          */
    TDA_VFREQ_87Hz      = 10,   /**< 87Hz          */
    TDA_VFREQ_INVALID   = 11,   /**< Invalid       */
    TDA_VFREQ_NUM       = 11    /**< No. of values */
} tda_vfreq;

typedef struct {
    unsigned short                      width;         /**< Width of the frame in pixels */
    unsigned short                      height;        /**< Height of the frame in pixels */
    int                        interlaced;    /**< Interlaced mode (True/False) */
    tda_vfreq           vfrequency;    /**< Vertical frequency in Hz */
    tda_pict_aspect_ratio aspectRatio;   /**< Picture aspect ratio (H:V) */
} tda_video_fmt_specs;

typedef enum {
    TDA_VINMODE_CCIR656     = 0,    /**< CCIR656 */
    TDA_VINMODE_RGB444      = 1,    /**< RGB444  */
    TDA_VINMODE_YUV444      = 2,    /**< YUV444  */
    TDA_VINMODE_YUV422      = 3,    /**< YUV422  */
    TDA_VINMODE_NO_CHANGE   = 4,    /**< No change */
    TDA_VINMODE_INVALID     = 5     /**< Invalid */
} tda_vinmode;

typedef enum {
    TDA_SYNCSRC_EMBEDDED = 0, /**< Embedded sync */
    TDA_SYNCSRC_EXT_VREF = 1, /**< External sync Vref, Href, Fref */
    TDA_SYNCSRC_EXT_VS   = 2  /**< External sync Vs, Hs */
} tda_sync_source;

typedef enum {
    TDA_PIXRATE_DOUBLE          = 0,        /**< Double pixel rate */
    TDA_PIXRATE_SINGLE          = 1,        /**< Single pixel rate */
    TDA_PIXRATE_SINGLE_REPEATED = 2         /**< Single pixel repeated */
} tda_pix_rate;

typedef struct {
   tda_video_fmt_id       format;     /**< Video format as defined by EIA/CEA 861-D */
   tda_vinmode      mode;       /**< Video mode (CCIR, RGB, YUV, etc.) */
   tda_sync_source   syncSource; /**< Sync source type */
   tda_pix_rate      pixelRate;  /**< Pixel rate */
} tda_video_in;

typedef enum {
    TDA_VOUTMODE_RGB444     = 0,    /**< RGB444    */
    TDA_VOUTMODE_YUV422     = 1,    /**< YUV422    */
    TDA_VOUTMODE_YUV444     = 2     /**< YUV444    */
} tda_vout_mode;

typedef enum {
    TDA_VQR_DEFAULT = 0, /* Follow HDMI spec. */
    TDA_RGB_FULL    = 1, /* Force RGB FULL , DVI only */
    TDA_RGB_LIMITED = 2  /* Force RGB LIMITED , DVI only */
} tda_vqr;

typedef enum {
    TDA_COLORDEPTH_24   = 0,    /**< 8 bits per color */
    TDA_COLORDEPTH_30   = 1,    /**< 10 bits per color */
    TDA_COLORDEPTH_36   = 2,    /**< 12 bits per color */
    TDA_COLORDEPTH_48   = 3     /**< 16 bits per color */
} tda_color_depth;

typedef struct {
   tda_video_fmt_id       format;     /**< Video format as defined by EIA/CEA 861-D */
   tda_vout_mode     mode;       /**< Video mode (CCIR, RGB, YUV, etc.) */
   tda_color_depth  colorDepth; /**< Color depth */
   tda_vqr          dviVqr;     /**< VQR applied in DVI mode */ 
} tda_video_out;

typedef struct {
   tda_audio_format          format;             /**< Audio format (I2S, SPDIF, etc.) */
   tda_audio_rate            rate;               /**< Audio sampling rate */
   tda_audio_I2S_format       i2sFormat;          /**< I2S format of the audio input */
   tda_audio_I2S_qualifier    i2sQualifier;       /**< I2S qualifier of the audio input (8,16,32 bits) */
   tda_dst_rate              dstRate;            /**< DST data transfer rate */
   unsigned char                            channelAllocation;  /**< Ref to CEA-861D p85 */
} tda_audio_in;

typedef enum {
    TDA_SINK_DVI  = 0, /**< DVI  */
    TDA_SINK_HDMI = 1, /**< HDMI */
    TDA_SINK_EDID = 2  /**< As currently defined in EDID */
} tda_sink;

 typedef enum {
     TDA_DEVICE_UNKNOWN,   /**< HW device is unknown */
     TDA_DEVICE_TDA9984,   /**< HW device is IC TDA9984 */
     TDA_DEVICE_TDA9989,   /**< HW device is IC TDA9989 */
     TDA_DEVICE_TDA9981,   /**< HW device is IC TDA9981 */
     TDA_DEVICE_TDA9983,   /**< HW device is IC TDA9983 */
     TDA_DEVICE_TDA19989   /**< HW device is IC TDA19989 */
 } tda_device_version;

typedef enum {
   TDA_HDMI_VERSION_UNKNOWN, /**< Unknown   */
   TDA_HDMI_VERSION_1_1,     /**< HDMI 1.1  */
   TDA_HDMI_VERSION_1_2a,    /**< HDMI 1.2a */
   TDA_HDMI_VERSION_1_3a     /**< HDMI 1.3  */
} tda_hdmi_version;
 
typedef struct {
   int HBR;              /**< High Bitrate Audio packet */
   int DST;              /**< Direct Stream Transport audio packet */
   int oneBitAudio;      /**< One Bit Audio sample packet */
} tda_audio_packet;

typedef enum {
    TDA_COLORDEPTH_24   = 0,    /**< 8 bits per color */
    TDA_COLORDEPTH_30   = 1,    /**< 10 bits per color */
    TDA_COLORDEPTH_36   = 2,    /**< 12 bits per color */
    TDA_COLORDEPTH_48   = 3     /**< 16 bits per color */
} tda_color_depth;

typedef struct {
    tda_device_version deviceVersion;  /**< HW device version */
    tda_hdmi_version hdmiVersion;    /**< Supported HDMI standard version  */
    tda_audio_packet audioPacket;    /**< Supported audio packets */
    tda_color_depth colorDepth;     /**< Supported color depth */
    int hdcp;           /**< Supported Hdcp encryption (True/False) */
   int scaler;         /**< Supported scaler (True/False) */
} tda_capabilities;

typedef struct {
    unsigned long compatibilityNr;        // Interface compatibility number
    unsigned long majorVersionNr;         // Interface major version number
    unsigned long minorVersionNr;         // Interface minor version number
} tda_version;

typedef enum
{
    PowerOn,                          // Device powered on      (D0 state)
    PowerStandby,                     // Device power standby   (D1 state)
    PowerSuspend,                     // Device power suspended (D2 state)
    PowerOff                          // Device powered off     (D3 state)
} tda_powerXXX;

typedef struct {
    unsigned int simplayHd;          /**< Enable simplayHD support */
    unsigned int repeaterEnable;     /**< Enable repeater mode */
    unsigned char *pEdidBuffer;       /**< Pointer to raw EDID data */
    unsigned long edidBufferSize;     /**< Size of buffer for raw EDID data */
} tda_setup;

typedef struct {
   tda_video_fmt_id id;
   tda_video_fmt_specs spec;
} tda_video_format;

typedef struct {
   tda_video_in video_in;
   tda_video_out video_out;
   tda_audio_in audio_in;
} tda_set_in_out;

typedef struct {
   tda_edid_audio_desc desc;
   unsigned int max;
   unsigned int written;
   unsigned char flags;
} tda_edid_audio_caps;

typedef struct {
   tda_edid_video_desc desc;
   unsigned int max;
   unsigned int written;
   unsigned char flags;
} tda_edid_video_caps;

typedef struct {
   tda_edid_status status;
   unsigned char block_count;
} tda_edid;

#endif

#define TDA_IOCTL_BASE 0x40
#define RELEASE 0xFF

enum {
   /* driver specific */
   TDA_VERBOSE_ON_CMD = 0,
   TDA_VERBOSE_OFF_CMD,
   TDA_BYEBYE_CMD,
   /* HDMI Tx */
   TDA_GET_SW_VERSION_CMD,
   TDA_SET_POWER_CMD,
   TDA_GET_POWER_CMD,
   TDA_SETUP_CMD,
   TDA_GET_SETUP_CMD,
   TDA_WAIT_EVENT_CMD,
   TDA_ENABLE_EVENT_CMD,
   TDA_DISABLE_EVENT_CMD,
   TDA_GET_VIDEO_SPEC_CMD,
   TDA_SET_INPUT_OUTPUT_CMD,
   TDA_SET_AUDIO_INPUT_CMD,
   TDA_SET_VIDEO_INFOFRAME_CMD,
   TDA_SET_AUDIO_INFOFRAME_CMD,
   TDA_SET_ACP_CMD,
   TDA_SET_GCP_CMD,
   TDA_SET_ISRC1_CMD,
   TDA_SET_ISRC2_CMD,
   TDA_SET_MPS_INFOFRAME_CMD,
   TDA_SET_SPD_INFOFRAME_CMD,
   TDA_SET_VS_INFOFRAME_CMD,
   TDA_SET_AUDIO_MUTE_CMD,
   TDA_RESET_AUDIO_CTS_CMD,
   TDA_GET_EDID_STATUS_CMD,
   TDA_GET_EDID_AUDIO_CAPS_CMD,
   TDA_GET_EDID_VIDEO_CAPS_CMD,
   TDA_GET_EDID_VIDEO_PREF_CMD,
   TDA_GET_EDID_SINK_TYPE_CMD,
   TDA_GET_EDID_SOURCE_ADDRESS_CMD,
   TDA_SET_GAMMUT_CMD,
   TDA_GET_EDID_DTD_CMD,
   TDA_GET_EDID_MD_CMD,
   TDA_GET_EDID_TV_ASPECT_RATIO_CMD,
   TDA_GET_EDID_LATENCY_CMD,
   TDA_GET_CABLE_STATUS_CMD,	/*LGE Update - changhoony.lee@lge.com*/
   TDA_SET_HDMI_INIT_CMD,		/*LGE Update - changhoony.lee@lge.com*/
   
   TDA_SET_HDMI_ENABLE,
   TDA_SET_HDMI_DISABLE,
   TDA_SET_HDMI_MODE,
   TDA_GET_CURRENT_RES,

   TDA_SET_HDCP_CMD,
   TDA_GET_HDCP_STATUS_CMD,
};


/* driver specific */
#define TDA_IOCTL_VERBOSE_ON     _IO(TDA_IOCTL_BASE, TDA_VERBOSE_ON_CMD)
#define TDA_IOCTL_VERBOSE_OFF     _IO(TDA_IOCTL_BASE, TDA_VERBOSE_OFF_CMD)
#define TDA_IOCTL_BYEBYE     _IO(TDA_IOCTL_BASE, TDA_BYEBYE_CMD)
/* HDMI Tx */
#define TDA_IOCTL_GET_SW_VERSION     _IOWR(TDA_IOCTL_BASE, TDA_GET_SW_VERSION_CMD,tda_version)
#define TDA_IOCTL_SET_POWER     _IOWR(TDA_IOCTL_BASE, TDA_SET_POWER_CMD,tda_power)
#define TDA_IOCTL_GET_POWER     _IOWR(TDA_IOCTL_BASE, TDA_GET_POWER_CMD,tda_power)
#define TDA_IOCTL_SETUP     _IOWR(TDA_IOCTL_BASE, TDA_SETUP_CMD,tda_setup_info)
#define TDA_IOCTL_GET_SETUP     _IOWR(TDA_IOCTL_BASE, TDA_GET_SETUP_CMD,tda_setup_info)
#define TDA_IOCTL_WAIT_EVENT     _IOWR(TDA_IOCTL_BASE, TDA_WAIT_EVENT_CMD,tda_event)
#define TDA_IOCTL_ENABLE_EVENT     _IOWR(TDA_IOCTL_BASE, TDA_ENABLE_EVENT_CMD,tda_event)
#define TDA_IOCTL_DISABLE_EVENT     _IOWR(TDA_IOCTL_BASE, TDA_DISABLE_EVENT_CMD,tda_event)
#define TDA_IOCTL_GET_VIDEO_SPEC     _IOWR(TDA_IOCTL_BASE, TDA_GET_VIDEO_SPEC_CMD,tda_video_format)
#define TDA_IOCTL_SET_INPUT_OUTPUT     _IOWR(TDA_IOCTL_BASE, TDA_SET_INPUT_OUTPUT_CMD,tda_set_in_out)
#define TDA_IOCTL_SET_AUDIO_INPUT     _IOWR(TDA_IOCTL_BASE, TDA_SET_AUDIO_INPUT_CMD,tda_audio_in)
#define TDA_IOCTL_SET_VIDEO_INFOFRAME     _IOWR(TDA_IOCTL_BASE, TDA_SET_VIDEO_INFOFRAME_CMD,tda_video_infoframe)
#define TDA_IOCTL_SET_AUDIO_INFOFRAME     _IOWR(TDA_IOCTL_BASE, TDA_SET_AUDIO_INFOFRAME_CMD,tda_audio_infoframe)
#define TDA_IOCTL_SET_ACP     _IOWR(TDA_IOCTL_BASE, TDA_SET_ACP_CMD,tda_acp)
#define TDA_IOCTL_SET_GCP     _IOWR(TDA_IOCTL_BASE, TDA_SET_GCP_CMD,tda_gcp)
#define TDA_IOCTL_SET_ISRC1     _IOWR(TDA_IOCTL_BASE, TDA_SET_ISRC1_CMD,tda_isrc1)
#define TDA_IOCTL_SET_ISRC2     _IOWR(TDA_IOCTL_BASE, TDA_SET_ISRC2_CMD,tda_isrc2)
#define TDA_IOCTL_SET_MPS_INFOFRAME     _IOWR(TDA_IOCTL_BASE, TDA_SET_MPS_INFOFRAME_CMD,tda_mps_infoframe)
#define TDA_IOCTL_SET_SPD_INFOFRAME     _IOWR(TDA_IOCTL_BASE, TDA_SET_SPD_INFOFRAME_CMD,tda_spd_infoframe)
#define TDA_IOCTL_SET_VS_INFOFRAME     _IOWR(TDA_IOCTL_BASE, TDA_SET_VS_INFOFRAME_CMD,tda_vs_infoframe)
#define TDA_IOCTL_SET_AUDIO_MUTE     _IOWR(TDA_IOCTL_BASE, TDA_SET_AUDIO_MUTE_CMD,bool)
#define TDA_IOCTL_RESET_AUDIO_CTS     _IO(TDA_IOCTL_BASE, TDA_RESET_AUDIO_CTS_CMD)
#define TDA_IOCTL_GET_EDID_STATUS     _IOWR(TDA_IOCTL_BASE, TDA_GET_EDID_STATUS_CMD,tda_edid)
#define TDA_IOCTL_GET_EDID_AUDIO_CAPS     _IOWR(TDA_IOCTL_BASE, TDA_GET_EDID_AUDIO_CAPS_CMD,tda_edid_audio_caps)
#define TDA_IOCTL_GET_EDID_VIDEO_CAPS     _IOWR(TDA_IOCTL_BASE, TDA_GET_EDID_VIDEO_CAPS_CMD,tda_edid_video_caps)
#define TDA_IOCTL_GET_EDID_VIDEO_PREF     _IOWR(TDA_IOCTL_BASE, TDA_GET_EDID_VIDEO_PREF_CMD,tda_edid_video_timings)
#define TDA_IOCTL_GET_EDID_SINK_TYPE     _IOWR(TDA_IOCTL_BASE, TDA_GET_EDID_SINK_TYPE_CMD,tda_sink)
#define TDA_IOCTL_GET_EDID_SOURCE_ADDRESS     _IOWR(TDA_IOCTL_BASE, TDA_GET_EDID_SOURCE_ADDRESS_CMD,unsigned short)
#define TDA_IOCTL_SET_GAMMUT     _IOWR(TDA_IOCTL_BASE, TDA_SET_GAMMUT_CMD,tda_gammut)
#define TDA_IOCTL_GET_EDID_DTD     _IOWR(TDA_IOCTL_BASE, TDA_GET_EDID_DTD_CMD,tda_edid_dtd)
#define TDA_IOCTL_GET_EDID_MD     _IOWR(TDA_IOCTL_BASE, TDA_GET_EDID_MD_CMD,tda_edid_md)
#define TDA_IOCTL_GET_EDID_TV_ASPECT_RATIO     _IOWR(TDA_IOCTL_BASE, TDA_GET_EDID_TV_ASPECT_RATIO_CMD,tda_edid_tv_aspect_ratio)
#ifdef TMFL_TDA19989
#define TDA_IOCTL_GET_EDID_LATENCY     _IOWR(TDA_IOCTL_BASE, TDA_GET_EDID_LATENCY_CMD,tda_edid_latency)
#define TDA_IOCTL_SET_HDCP     _IOWR(TDA_IOCTL_BASE, TDA_SET_HDCP_CMD,bool)
#define TDA_IOCTL_GET_HDCP_STATUS     _IOWR(TDA_IOCTL_BASE, TDA_GET_HDCP_STATUS_CMD,tda_hdcp_status)
#endif
//LGE Update - jy.choi@lge.com
#define TDA_IOCTL_GET_CABLE_STATUS     _IOWR(TDA_IOCTL_BASE, TDA_GET_CABLE_STATUS_CMD,tda_hpd_status)
#define TDA_IOCTL_SET_HDMI_INIT     _IO(TDA_IOCTL_BASE, TDA_SET_HDMI_INIT_CMD)
#define TDA_IOCTL_SET_HDMI_ENABLE     _IO(TDA_IOCTL_BASE, TDA_SET_HDMI_ENABLE)
#define TDA_IOCTL_SET_HDMI_DISABLE     _IO(TDA_IOCTL_BASE, TDA_SET_HDMI_DISABLE)
#define TDA_IOCTL_SET_HDMI_MODE     _IOWR(TDA_IOCTL_BASE, TDA_SET_HDMI_MODE,play_mode)
#define TDA_IOCTL_GET_CURRENT_RES     _IOWR(TDA_IOCTL_BASE, TDA_GET_CURRENT_RES,tda_current_res)



/* --- Full list --- */

/* legend: */
/* ------- */
/* [ ] : not supported */
/* [x] : IOCTL */
/* [i] : open, init... */

/* [x] tmdlHdmiTxGetSWVersion */
/* [ ] tmdlHdmiTxGetNumberOfUnits */
/* [i] tmdlHdmiTxGetCapabilities */
/* [ ] tmdlHdmiTxGetCapabilitiesM */
/* [i] tmdlHdmiTxOpen */
/* [ ] tmdlHdmiTxOpenM */
/* [i] tmdlHdmiTxClose */
/* [x] tmdlHdmiTxSetPowerState */
/* [x] tmdlHdmiTxGetPowerState */
/* [ ] tmdlHdmiTxInstanceConfig */
/* [xi] tmdlHdmiTxInstanceSetup */
/* [x] tmdlHdmiTxGetInstanceSetup */
/* [x] tmdlHdmiTxHandleInterrupt see IOCTL_WAIT_EVENT */
/* [i] tmdlHdmiTxRegisterCallbacks */
/* [x] tmdlHdmiTxEnableEvent */
/* [x] tmdlHdmiTxDisableEvent */
/* [x] tmdlHdmiTxGetVideoFormatSpecs */
/* [x] tmdlHdmiTxSetInputOutput */
/* [x] tmdlHdmiTxSetAudioInput */
/* [x] tmdlHdmiTxSetVideoInfoframe */
/* [x] tmdlHdmiTxSetAudioInfoframe */
/* [x] tmdlHdmiTxSetACPPacket */
/* [x] tmdlHdmiTxSetGeneralControlPacket */
/* [x] tmdlHdmiTxSetISRC1Packet */
/* [x] tmdlHdmiTxSetISRC2Packet */
/* [x] tmdlHdmiTxSetMPSInfoframe */
/* [x] tmdlHdmiTxSetSpdInfoframe */
/* [x] tmdlHdmiTxSetVsInfoframe */
/* [ ] tmdlHdmiTxDebugSetNullPacket */
/* [ ] tmdlHdmiTxDebugSetSingleNullPacket */
/* [x] tmdlHdmiTxSetAudioMute */
/* [x] tmdlHdmiTxResetAudioCts */
/* [x] tmdlHdmiTxGetEdidStatus */
/* [x] tmdlHdmiTxGetEdidAudioCaps */
/* [x] tmdlHdmiTxGetEdidVideoCaps */
/* [x] tmdlHdmiTxGetEdidVideoPreferred */
/* [x] tmdlHdmiTxGetEdidSinkType */
/* [x] tmdlHdmiTxGetEdidSourceAddress */
/* [ ] tmdlHdmiTxGetKsvList */
/* [ ] tmdlHdmiTxGetDepth */
/* [ ] tmdlHdmiTxGeneSHA_1_IT */
/* [ ] tmdlHdmiTxSetHdcp */
/* [ ] tmdlHdmiTxGetHdcpState */
/* [ ] tmdlHdmiTxHdcpCheck */
/* [x] tmdlHdmiTxSetGamutPacket */
/* [x] tmdlHdmiTxGetEdidDetailledTimingDescriptors */
/* [x] tmdlHdmiTxGetEdidMonitorDescriptors */
/* [x] tmdlHdmiTxGetEdidTVPictureRatio */
/* [ ] tmdlHdmiTxSetHDCPRevocationList */
/* [ ] tmdlHdmiTxGetHdcpFailStatus */
/* [x] tmdlHdmiTxGetEdidLatencyInfo */
/* [ ] tmdlHdmiTxSetBScreen */
/* [ ] tmdlHdmiTxRemoveBScreen */


//#endif /* __tx_h__ */
#endif /* __tx_ioctl__ */

