/**
 * Copyright (C) 2006 NXP N.V., All Rights Reserved.
 * This source code and any compilation or derivative thereof is the proprietary
 * information of NXP N.V. and is confidential in nature. Under no circumstances
 * is this software to be  exposed to or placed under an Open Source License of
 * any type without the expressed written permission of NXP N.V.
 *
 * \file          tmdlHdmiTx_cfg.c
 *
 * \version       Revision: 1
 *
 * \date          Date: 08/08/07 11:00
 *
 * \brief         devlib driver component API for the TDA998x HDMI Transmitters
 *
 * \section refs  Reference Documents
 * HDMI Tx Driver - FRS.doc,
 *
 * \section info  Change Information
 *
 * \verbatim

   History:       tmdlHdmiTx_cfg.c
 *
 * *****************  Version 1  *****************
 * User: J. Lamotte Date: 08/08/07  Time: 11:00
 * initial version
 *

   \endverbatim
 *
*/

/*============================================================================*/
/*                             INCLUDE FILES                                  */
/*============================================================================*/
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/types.h>
#include <linux/i2c.h>
#include <linux/delay.h>

#include "tmdlHdmiTx_IW.h"
#include "tmNxTypes.h"
#include "tmdlHdmiTx.h"
#include "tmdlHdmiTx_cfg.h"

#include "tmbslHdmiTx.h"

#define I2C_M_WR 0

struct i2c_client *GetThisI2cClient(void);
unsigned char  my_i2c_data[255];

/*============================================================================*/
/*                          INTERNAL PROTOTYPE                                */
/*============================================================================*/

tmErrorCode_t TxI2cReadFunction(tmbslHdmiTxSysArgs_t *pSysArgs);
tmErrorCode_t TxI2cWriteFunction(tmbslHdmiTxSysArgs_t *pSysArgs);
tmErrorCode_t tda9983_edid_read(tmbslHdmiTxSysArgsEdid_t* pArg);

/*============================================================================*/
/*                                MACRO                                       */
/*============================================================================*/

/* macro for quick error handling */
#define RETIF(cond, rslt) if ((cond)){return (rslt);}

/*============================================================================*/
/*                          TYPES DECLARATIONS                                */
/*============================================================================*/


/* I2C adress of the unit */
#define UNIT_I2C_ADDRESS_0 0x70


/* I2C adress of the unit */
#define UNIT_I2C_ADDRESS_1 0x71

/* Resolution supported */
#ifndef FORMAT_PC
#define RESOLUTION_NB   34
#else
#define RESOLUTION_NB   61
#endif /* FORMAT_PC */

/* HDCP key */
#define KEY_SEED    0x1234

/* Audio port configuration, bitn = 1 to enable port n, = 0 to disable port n */
#define ENABLE_ALL_AUDIO_PORT       0xFF

/* Audio clock port configuration */
#define ENABLE_AUDIO_CLOCK_PORT     True
#define DISABLE_AUDIO_CLOCK_PORT    False

/* Audio port configuration, bitn = 1 to pulldown port n, = 0 to pulldown port n */
#define DISABLE_ALL_AUDIO_PORT_PULLDOWN 0x00

/* Audio clock port pulldown configuration */
#define ENABLE_AUDIO_CLOCK_PORT_PULLDOWN    True
#define DISABLE_AUDIO_CLOCK_PORT_PULLDOWN   False

/*============================================================================*/
/*                        DEFINES DECLARATIONS                                */
/*============================================================================*/

/*============================================================================*/
/*                       VARIABLES DECLARATIONS                               */
/*============================================================================*/

/**
 * \brief List of the resolution to be detected by the device library
 */

const tmdlHdmiTxCfgResolution_t resolutionInfoTx[RESOLUTION_NB] = {
    /* TV Formats */
    /* 60 HZ */
    {TMDL_HDMITX_VFMT_01_640x480p_60Hz,     640,   480,    False,  TMDL_HDMITX_VFREQ_59Hz, TMDL_HDMITX_P_ASPECT_RATIO_4_3},
    {TMDL_HDMITX_VFMT_02_720x480p_60Hz,     720,   480,    False,  TMDL_HDMITX_VFREQ_59Hz, TMDL_HDMITX_P_ASPECT_RATIO_4_3},
    {TMDL_HDMITX_VFMT_03_720x480p_60Hz,     720,   480,    False,  TMDL_HDMITX_VFREQ_59Hz, TMDL_HDMITX_P_ASPECT_RATIO_16_9},
    {TMDL_HDMITX_VFMT_04_1280x720p_60Hz,    1280,  720,    False,  TMDL_HDMITX_VFREQ_60Hz, TMDL_HDMITX_P_ASPECT_RATIO_16_9},
    {TMDL_HDMITX_VFMT_05_1920x1080i_60Hz,   1920,  1080,   True,   TMDL_HDMITX_VFREQ_60Hz, TMDL_HDMITX_P_ASPECT_RATIO_16_9},
    {TMDL_HDMITX_VFMT_06_720x480i_60Hz,     720,   480,    True,   TMDL_HDMITX_VFREQ_59Hz, TMDL_HDMITX_P_ASPECT_RATIO_4_3},
    {TMDL_HDMITX_VFMT_07_720x480i_60Hz,     720,   480,    True,   TMDL_HDMITX_VFREQ_59Hz, TMDL_HDMITX_P_ASPECT_RATIO_16_9},
    {TMDL_HDMITX_VFMT_08_720x240p_60Hz,     720,   240,    False,  TMDL_HDMITX_VFREQ_59Hz, TMDL_HDMITX_P_ASPECT_RATIO_4_3},
    {TMDL_HDMITX_VFMT_09_720x240p_60Hz,     720,   240,    False,  TMDL_HDMITX_VFREQ_59Hz, TMDL_HDMITX_P_ASPECT_RATIO_16_9},
    {TMDL_HDMITX_VFMT_10_720x480i_60Hz,     720,   480,    True,   TMDL_HDMITX_VFREQ_59Hz, TMDL_HDMITX_P_ASPECT_RATIO_4_3},
    {TMDL_HDMITX_VFMT_11_720x480i_60Hz,     720,   480,    True,   TMDL_HDMITX_VFREQ_59Hz, TMDL_HDMITX_P_ASPECT_RATIO_16_9},
    {TMDL_HDMITX_VFMT_12_720x240p_60Hz,     720,   240,    False,  TMDL_HDMITX_VFREQ_59Hz, TMDL_HDMITX_P_ASPECT_RATIO_4_3},
    {TMDL_HDMITX_VFMT_13_720x240p_60Hz,     720,   240,    False,  TMDL_HDMITX_VFREQ_59Hz, TMDL_HDMITX_P_ASPECT_RATIO_16_9},
    {TMDL_HDMITX_VFMT_14_1440x480p_60Hz,    1440,  480,    False,  TMDL_HDMITX_VFREQ_59Hz, TMDL_HDMITX_P_ASPECT_RATIO_4_3},
    {TMDL_HDMITX_VFMT_15_1440x480p_60Hz,    1440,  480,    False,  TMDL_HDMITX_VFREQ_59Hz, TMDL_HDMITX_P_ASPECT_RATIO_16_9},
    {TMDL_HDMITX_VFMT_16_1920x1080p_60Hz,   1920,  1080,   False,  TMDL_HDMITX_VFREQ_60Hz, TMDL_HDMITX_P_ASPECT_RATIO_16_9},
    /* 50 HZ */
    {TMDL_HDMITX_VFMT_17_720x576p_50Hz,     720,   576,    False,  TMDL_HDMITX_VFREQ_50Hz, TMDL_HDMITX_P_ASPECT_RATIO_4_3},
    {TMDL_HDMITX_VFMT_18_720x576p_50Hz,     720,   576,    False,  TMDL_HDMITX_VFREQ_50Hz, TMDL_HDMITX_P_ASPECT_RATIO_16_9},
    {TMDL_HDMITX_VFMT_19_1280x720p_50Hz,    1280,  720,    False,  TMDL_HDMITX_VFREQ_50Hz, TMDL_HDMITX_P_ASPECT_RATIO_16_9},
    {TMDL_HDMITX_VFMT_20_1920x1080i_50Hz,   1920,  1080,   True,   TMDL_HDMITX_VFREQ_50Hz, TMDL_HDMITX_P_ASPECT_RATIO_16_9},
    {TMDL_HDMITX_VFMT_21_720x576i_50Hz,     720,   576,    True,   TMDL_HDMITX_VFREQ_50Hz, TMDL_HDMITX_P_ASPECT_RATIO_4_3},
    {TMDL_HDMITX_VFMT_22_720x576i_50Hz,     720,   576,    True,   TMDL_HDMITX_VFREQ_50Hz, TMDL_HDMITX_P_ASPECT_RATIO_16_9},
    {TMDL_HDMITX_VFMT_23_720x288p_50Hz,     720,   288,    False,  TMDL_HDMITX_VFREQ_50Hz, TMDL_HDMITX_P_ASPECT_RATIO_4_3},
    {TMDL_HDMITX_VFMT_24_720x288p_50Hz,     720,   288,    False,  TMDL_HDMITX_VFREQ_50Hz, TMDL_HDMITX_P_ASPECT_RATIO_16_9},
    {TMDL_HDMITX_VFMT_25_720x576i_50Hz,     720,   576,    True,   TMDL_HDMITX_VFREQ_50Hz, TMDL_HDMITX_P_ASPECT_RATIO_4_3},
    {TMDL_HDMITX_VFMT_26_720x576i_50Hz,     720,   576,    True,   TMDL_HDMITX_VFREQ_50Hz, TMDL_HDMITX_P_ASPECT_RATIO_16_9},
    {TMDL_HDMITX_VFMT_27_720x288p_50Hz,     720,   288,    False,  TMDL_HDMITX_VFREQ_50Hz, TMDL_HDMITX_P_ASPECT_RATIO_4_3},
    {TMDL_HDMITX_VFMT_28_720x288p_50Hz,     720,   288,    False,  TMDL_HDMITX_VFREQ_50Hz, TMDL_HDMITX_P_ASPECT_RATIO_16_9},
    {TMDL_HDMITX_VFMT_29_1440x576p_50Hz,    1440,  576,    False,  TMDL_HDMITX_VFREQ_50Hz, TMDL_HDMITX_P_ASPECT_RATIO_4_3},
    {TMDL_HDMITX_VFMT_30_1440x576p_50Hz,    1440,  576,    False,  TMDL_HDMITX_VFREQ_50Hz, TMDL_HDMITX_P_ASPECT_RATIO_16_9},
    {TMDL_HDMITX_VFMT_31_1920x1080p_50Hz,   1920,  1080,   False,  TMDL_HDMITX_VFREQ_50Hz, TMDL_HDMITX_P_ASPECT_RATIO_16_9},
    {TMDL_HDMITX_VFMT_32_1920x1080p_24Hz,   1920,  1080,   False,  TMDL_HDMITX_VFREQ_24Hz, TMDL_HDMITX_P_ASPECT_RATIO_16_9},
    {TMDL_HDMITX_VFMT_33_1920x1080p_25Hz,   1920,  1080,   False,  TMDL_HDMITX_VFREQ_25Hz, TMDL_HDMITX_P_ASPECT_RATIO_16_9},
    {TMDL_HDMITX_VFMT_34_1920x1080p_30Hz,   1920,  1080,   False,  TMDL_HDMITX_VFREQ_30Hz, TMDL_HDMITX_P_ASPECT_RATIO_16_9}
#ifdef FORMAT_PC
    /* PC Formats */
    /* 60 HZ */
    ,{TMDL_HDMITX_VFMT_PC_640x480p_60Hz,     640,   480,    False,  TMDL_HDMITX_VFREQ_60Hz, TMDL_HDMITX_P_ASPECT_RATIO_4_3},
    {TMDL_HDMITX_VFMT_PC_800x600p_60Hz,     800,   600,    False,  TMDL_HDMITX_VFREQ_60Hz, TMDL_HDMITX_P_ASPECT_RATIO_4_3},
    {TMDL_HDMITX_VFMT_PC_1152x960p_60Hz,    1152,  960,    False,  TMDL_HDMITX_VFREQ_60Hz, TMDL_HDMITX_P_ASPECT_RATIO_6_5},
    {TMDL_HDMITX_VFMT_PC_1024x768p_60Hz,    1024,  768,    False,  TMDL_HDMITX_VFREQ_60Hz, TMDL_HDMITX_P_ASPECT_RATIO_4_3},
    {TMDL_HDMITX_VFMT_PC_1280x768p_60Hz,    1280,  768,    False,  TMDL_HDMITX_VFREQ_60Hz, TMDL_HDMITX_P_ASPECT_RATIO_5_3},
    {TMDL_HDMITX_VFMT_PC_1280x1024p_60Hz,   1280,  1024,   False,  TMDL_HDMITX_VFREQ_60Hz, TMDL_HDMITX_P_ASPECT_RATIO_5_4},
    {TMDL_HDMITX_VFMT_PC_1360x768p_60Hz,    1360,  768,    False,  TMDL_HDMITX_VFREQ_60Hz, TMDL_HDMITX_P_ASPECT_RATIO_16_9},
    {TMDL_HDMITX_VFMT_PC_1400x1050p_60Hz,   1400,  1050,   False,  TMDL_HDMITX_VFREQ_60Hz, TMDL_HDMITX_P_ASPECT_RATIO_4_3},
    {TMDL_HDMITX_VFMT_PC_1600x1200p_60Hz,   1600,  1200,   False,  TMDL_HDMITX_VFREQ_60Hz, TMDL_HDMITX_P_ASPECT_RATIO_4_3},
    /* 70 HZ */
    {TMDL_HDMITX_VFMT_PC_1024x768p_70Hz,    1024,  768,    False,  TMDL_HDMITX_VFREQ_70Hz, TMDL_HDMITX_P_ASPECT_RATIO_4_3},
    /* 72 HZ */
    {TMDL_HDMITX_VFMT_PC_640x480p_72Hz,     640,   480,    False,  TMDL_HDMITX_VFREQ_72Hz, TMDL_HDMITX_P_ASPECT_RATIO_4_3},
    {TMDL_HDMITX_VFMT_PC_800x600p_72Hz,     800,   600,    False,  TMDL_HDMITX_VFREQ_72Hz, TMDL_HDMITX_P_ASPECT_RATIO_4_3},
    /* 75 HZ */
    {TMDL_HDMITX_VFMT_PC_640x480p_75Hz,     640,   480,    False,  TMDL_HDMITX_VFREQ_75Hz, TMDL_HDMITX_P_ASPECT_RATIO_4_3},
    {TMDL_HDMITX_VFMT_PC_1024x768p_75Hz,    1024,  768,    False,  TMDL_HDMITX_VFREQ_75Hz, TMDL_HDMITX_P_ASPECT_RATIO_4_3},
    {TMDL_HDMITX_VFMT_PC_800x600p_75Hz,     800,   600,    False,  TMDL_HDMITX_VFREQ_75Hz, TMDL_HDMITX_P_ASPECT_RATIO_4_3},
    {TMDL_HDMITX_VFMT_PC_1024x864p_75Hz,    1024,  864,    False,  TMDL_HDMITX_VFREQ_75Hz, TMDL_HDMITX_P_ASPECT_RATIO_UNDEFINED},
    {TMDL_HDMITX_VFMT_PC_1280x1024p_75Hz,   1280,  1024,   False,  TMDL_HDMITX_VFREQ_75Hz, TMDL_HDMITX_P_ASPECT_RATIO_5_4},
    /* 85 HZ */
    {TMDL_HDMITX_VFMT_PC_640x350p_85Hz,     640,   350,    False,  TMDL_HDMITX_VFREQ_85Hz, TMDL_HDMITX_P_ASPECT_RATIO_UNDEFINED},
    {TMDL_HDMITX_VFMT_PC_640x400p_85Hz,     640,   400,    False,  TMDL_HDMITX_VFREQ_85Hz, TMDL_HDMITX_P_ASPECT_RATIO_16_10},
    {TMDL_HDMITX_VFMT_PC_720x400p_85Hz,     720,   400,    False,  TMDL_HDMITX_VFREQ_85Hz, TMDL_HDMITX_P_ASPECT_RATIO_9_5},
    {TMDL_HDMITX_VFMT_PC_640x480p_85Hz,     640,   480,    False,  TMDL_HDMITX_VFREQ_85Hz, TMDL_HDMITX_P_ASPECT_RATIO_4_3},
    {TMDL_HDMITX_VFMT_PC_800x600p_85Hz,     800,   600,    False,  TMDL_HDMITX_VFREQ_85Hz, TMDL_HDMITX_P_ASPECT_RATIO_4_3},
    {TMDL_HDMITX_VFMT_PC_1024x768p_85Hz,    1024,  768,    False,  TMDL_HDMITX_VFREQ_85Hz, TMDL_HDMITX_P_ASPECT_RATIO_4_3},
    {TMDL_HDMITX_VFMT_PC_1152x864p_85Hz,    1152,  864,    False,  TMDL_HDMITX_VFREQ_85Hz, TMDL_HDMITX_P_ASPECT_RATIO_4_3},
    {TMDL_HDMITX_VFMT_PC_1280x960p_85Hz,    1280,  960,    False,  TMDL_HDMITX_VFREQ_85Hz, TMDL_HDMITX_P_ASPECT_RATIO_4_3},
    {TMDL_HDMITX_VFMT_PC_1280x1024p_85Hz,   1280,  1024,   False,  TMDL_HDMITX_VFREQ_85Hz, TMDL_HDMITX_P_ASPECT_RATIO_5_4},
    /* 87 HZ */
    {TMDL_HDMITX_VFMT_PC_1024x768i_87Hz,    1024,  768,    True,   TMDL_HDMITX_VFREQ_87Hz, TMDL_HDMITX_P_ASPECT_RATIO_4_3}
#endif /* FORMAT_PC */
};



const tmdlHdmiTxCfgVideoSignal444 videoPortMapping_YUV444[MAX_UNITS][6] = {
    {
        TMDL_HDMITX_VID444_BU_0_TO_3,   /* Signals connected to VPA[0..3] */
        TMDL_HDMITX_VID444_BU_4_TO_7,   /* Signals connected to VPA[4..7] */
        TMDL_HDMITX_VID444_GY_0_TO_3,   /* Signals connected to VPB[0..3] */
        TMDL_HDMITX_VID444_GY_4_TO_7,   /* Signals connected to VPB[4..7] */
        TMDL_HDMITX_VID444_VR_0_TO_3,   /* Signals connected to VPC[0..3] */
        TMDL_HDMITX_VID444_VR_4_TO_7    /* Signals connected to VPC[4..7] */
    }
};

const tmdlHdmiTxCfgVideoSignal444 videoPortMapping_RGB444[MAX_UNITS][6] = {
    {
        TMDL_HDMITX_VID444_BU_0_TO_3,   /* Signals connected to VPA[0..3] */
        TMDL_HDMITX_VID444_BU_4_TO_7,   /* Signals connected to VPA[4..7] */
        TMDL_HDMITX_VID444_GY_0_TO_3,   /* Signals connected to VPB[0..3] */
        TMDL_HDMITX_VID444_GY_4_TO_7,   /* Signals connected to VPB[4..7] */
        TMDL_HDMITX_VID444_VR_0_TO_3,   /* Signals connected to VPC[0..3] */
        TMDL_HDMITX_VID444_VR_4_TO_7    /* Signals connected to VPC[4..7] */
    }
};

const tmdlHdmiTxCfgVideoSignal422 videoPortMapping_YUV422[MAX_UNITS][6] = {
   
  {
        TMDL_HDMITX_VID422_Y_4_TO_7,           /* Signals connected to VPA[0..3] */    
        TMDL_HDMITX_VID422_Y_8_TO_11,          /* Signals connected to VPA[4..7] */    
        TMDL_HDMITX_VID422_UV_4_TO_7,          /* Signals connected to VPB[0..3] */
        TMDL_HDMITX_VID422_UV_8_TO_11,         /* Signals connected to VPB[4..7] */
        TMDL_HDMITX_VID422_NOT_CONNECTED,      /* Signals connected to VPC[0..3] */
        TMDL_HDMITX_VID422_NOT_CONNECTED       /* Signals connected to VPC[4..7] */
  }
 };

const tmdlHdmiTxCfgVideoSignalCCIR656 videoPortMapping_CCIR656[MAX_UNITS][6] = {
    { 
        TMDL_HDMITX_VIDCCIR_4_TO_7,         /* Signals connected to VPA[0..3] */
        TMDL_HDMITX_VIDCCIR_8_TO_11,        /* Signals connected to VPA[4..7] */
        TMDL_HDMITX_VIDCCIR_NOT_CONNECTED,  /* Signals connected to VPB[0..3] */
        TMDL_HDMITX_VIDCCIR_NOT_CONNECTED,  /* Signals connected to VPB[4..7] */
        TMDL_HDMITX_VIDCCIR_NOT_CONNECTED,  /* Signals connected to VPC[0..3] */
        TMDL_HDMITX_VIDCCIR_NOT_CONNECTED   /* Signals connected to VPC[4..7] */
    }
};






/* Audio port configuration for SPDIF                                         */
/* Here you can specify the audio port routing configuration for SPDIF input. */
/* enableAudioPortSPDIF and groundAudioPortSPDIF should be filled with a      */
/* value build as follows : each bit represent an audio port, LSB is port 0.  */
/* enableAudioClockPortSPDIF and groundAudioClockPortSPDIF can be configured  */
/* with the corresponding enums (See file tmdlHdmiTx_cfg.h for more details). */
UInt8 enableAudioPortSPDIF[MAX_UNITS]      = {0x40};
UInt8 enableAudioClockPortSPDIF[MAX_UNITS] = {DISABLE_AUDIO_CLOCK_PORT};
UInt8 groundAudioPortSPDIF[MAX_UNITS]      = {0xBF};
UInt8 groundAudioClockPortSPDIF[MAX_UNITS] = {ENABLE_AUDIO_CLOCK_PORT_PULLDOWN};

/* Audio port configuration for I2S                                           */
/* Here you can specify the audio port routing configuration for SPDIF input. */
/* enableAudioPortI2S and groundAudioPortI2S should be filled with a          */
/* value build as follows : each bit represent an audio port, LSB is port 0.  */
/* enableAudioClockPortI2S and groundAudioClockPortI2S can be configured      */
/* with the corresponding enums (See file tmdlHdmiTx_cfg.h for more details). */
UInt8 enableAudioPortI2S[MAX_UNITS]        = {0x03};
UInt8 enableAudioPortI2S8C[MAX_UNITS]      = {0x1F};
UInt8 enableAudioClockPortI2S[MAX_UNITS]   = {ENABLE_AUDIO_CLOCK_PORT};
UInt8 groundAudioPortI2S[MAX_UNITS]        = {0xFC};
UInt8 groundAudioPortI2S8C[MAX_UNITS]      = {0xE0};
UInt8 groundAudioClockPortI2S[MAX_UNITS]   = {DISABLE_AUDIO_CLOCK_PORT_PULLDOWN};

/* Audio port configuration for OBA                                           */
/* Here you can specify the audio port routing configuration for SPDIF input. */
/* enableAudioPortOBA and groundAudioPortOBA should be filled with a          */
/* value build as follows : each bit represent an audio port, LSB is port 0.  */
/* enableAudioClockPortOBA and groundAudioClockPortOBA can be configured      */
/* with the corresponding enums (See file tmdlHdmiTx_cfg.h for more details). */
UInt8 enableAudioPortOBA[MAX_UNITS]        = {0xFF};
UInt8 enableAudioClockPortOBA[MAX_UNITS]   = {ENABLE_AUDIO_CLOCK_PORT};
UInt8 groundAudioPortOBA[MAX_UNITS]        = {0x00};
UInt8 groundAudioClockPortOBA[MAX_UNITS]   = {DISABLE_AUDIO_CLOCK_PORT_PULLDOWN};

/* Audio port configuration for DST                                           */
/* Here you can specify the audio port routing configuration for SPDIF input. */
/* enableAudioPortDST and groundAudioPortDST should be filled with a          */
/* value build as follows : each bit represent an audio port, LSB is port 0.  */
/* enableAudioClockPortDST and groundAudioClockPortDST can be configured      */
/* with the corresponding enums (See file tmdlHdmiTx_cfg.h for more details). */
UInt8 enableAudioPortDST[MAX_UNITS]        = {0xFF};
UInt8 enableAudioClockPortDST[MAX_UNITS]   = {ENABLE_AUDIO_CLOCK_PORT};
UInt8 groundAudioPortDST[MAX_UNITS]        = {0x00};
UInt8 groundAudioClockPortDST[MAX_UNITS]   = {DISABLE_AUDIO_CLOCK_PORT_PULLDOWN};

/* Audio port configuration for HBR                                           */
/* Here you can specify the audio port routing configuration for SPDIF input. */
/* enableAudioPortHBR and groundAudioPortHBR should be filled with a          */
/* value build as follows : each bit represent an audio port, LSB is port 0.  */
/* enableAudioClockPortHBR and groundAudioClockPortHBR can be configured      */
/* with the corresponding enums (See file tmdlHdmiTx_cfg.h for more details). */
UInt8 enableAudioPortHBR[MAX_UNITS]        = {0x1F};
UInt8 enableAudioClockPortHBR[MAX_UNITS]   = {ENABLE_AUDIO_CLOCK_PORT};
UInt8 groundAudioPortHBR[MAX_UNITS]        = {0xE0};
UInt8 groundAudioClockPortHBR[MAX_UNITS]   = {DISABLE_AUDIO_CLOCK_PORT_PULLDOWN};



/* Do not modify, those tables are filled dynamically by dlHdmiTxGenerateCfgVideoPortTables API */
UInt8 mirrorTableCCIR656[MAX_UNITS][6] = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};
UInt8 swapTableCCIR656[MAX_UNITS][6] = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};
UInt8 enableVideoPortCCIR656[MAX_UNITS][3] = {{0x00, 0x00, 0x00}};
UInt8 groundVideoPortCCIR656[MAX_UNITS][3] = {{0xFF, 0xFF, 0xFF}};
UInt8 mirrorTableYUV422[MAX_UNITS][6] = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};
UInt8 swapTableYUV422[MAX_UNITS][6] = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};
UInt8 enableVideoPortYUV422[MAX_UNITS][3] = {{0x00, 0x00, 0x00}};
UInt8 groundVideoPortYUV422[MAX_UNITS][3] = {{0xFF, 0xFF, 0xFF}};
UInt8 mirrorTableYUV444[MAX_UNITS][6] = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};
UInt8 swapTableYUV444[MAX_UNITS][6] = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};
UInt8 enableVideoPortYUV444[MAX_UNITS][3] = {{0x00, 0x00, 0x00}};
UInt8 groundVideoPortYUV444[MAX_UNITS][3] = {{0xFF, 0xFF, 0xFF}};
UInt8 mirrorTableRGB444[MAX_UNITS][6] = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};
UInt8 swapTableRGB444[MAX_UNITS][6] = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};
UInt8 enableVideoPortRGB444[MAX_UNITS][3] = {{0x00, 0x00, 0x00}};
UInt8 groundVideoPortRGB444[MAX_UNITS][3] = {{0xFF, 0xFF, 0xFF}};

/**
 * \brief Configuration Tables. This table can be modified by the customer 
            to choose its prefered configuration.
 */
tmdlHdmiTxDriverConfigTable_t driverConfigTableTx[MAX_UNITS] = {
    {
       250 /* COMMAND_TASK_PRIORITY_0 */,
       128 /* COMMAND_TASK_STACKSIZE_0 */,
       128 /* COMMAND_TASK_QUEUESIZE_0 */,
       250 /* HDCP_CHECK_TASK_PRIORITY_0 */,
       128 /* HDCP_CHECK_TASK_STACKSIZE_0 */,
        UNIT_I2C_ADDRESS_0,
        TxI2cReadFunction,
        TxI2cWriteFunction,
        tda9983_edid_read,
        Null,                           /* filled dynamically, do not modify */
        RESOLUTION_NB,
        &mirrorTableCCIR656[0][0],      /* filled dynamically, do not modify */
        &swapTableCCIR656[0][0],        /* filled dynamically, do not modify */
        &enableVideoPortCCIR656[0][0],  /* filled dynamically, do not modify */
        &groundVideoPortCCIR656[0][0],  /* filled dynamically, do not modify */
        &mirrorTableYUV422[0][0],       /* filled dynamically, do not modify */
        &swapTableYUV422[0][0],         /* filled dynamically, do not modify */
        &enableVideoPortYUV422[0][0],   /* filled dynamically, do not modify */
        &groundVideoPortYUV422[0][0],   /* filled dynamically, do not modify */
        &mirrorTableYUV444[0][0],       /* filled dynamically, do not modify */
        &swapTableYUV444[0][0],         /* filled dynamically, do not modify */
        &enableVideoPortYUV444[0][0],   /* filled dynamically, do not modify */
        &groundVideoPortYUV444[0][0],   /* filled dynamically, do not modify */
        &mirrorTableRGB444[0][0],       /* filled dynamically, do not modify */
        &swapTableRGB444[0][0],         /* filled dynamically, do not modify */
        &enableVideoPortRGB444[0][0],   /* filled dynamically, do not modify */
        &groundVideoPortRGB444[0][0],   /* filled dynamically, do not modify */
        &enableAudioPortSPDIF[0],
        &groundAudioPortSPDIF[0],
        &enableAudioClockPortSPDIF[0],
        &groundAudioClockPortSPDIF[0],
        &enableAudioPortI2S[0],
        &groundAudioPortI2S[0],
        &enableAudioPortI2S8C[0],
        &groundAudioPortI2S8C[0],
        &enableAudioClockPortI2S[0],
        &groundAudioClockPortI2S[0],
        &enableAudioPortOBA[0],
        &groundAudioPortOBA[0],
        &enableAudioClockPortOBA[0],
        &groundAudioClockPortOBA[0],
        &enableAudioPortDST[0],
        &groundAudioPortDST[0],
        &enableAudioClockPortDST[0],
        &groundAudioClockPortDST[0],
        &enableAudioPortHBR[0],
        &groundAudioPortHBR[0],
        &enableAudioClockPortHBR[0],
        &groundAudioClockPortHBR[0],
        KEY_SEED,
        TMDL_HDMITX_PATTERN_BLUE
    }
};

/*
 *
 * Linux wrapping starts here...............................
 *
 */

/* 
 *  Write a bloc to a register in Tx device.
 */
int blockwrite_reg(struct i2c_client *client,
		   u8 reg, u16 alength, u8 *val)
{
   int err = 0, i;
   struct i2c_msg msg[1];
    
   if (!client->adapter) {
      dev_err(&client->dev, "<%s> ERROR: No HDMI Device\n", __func__);
      return -ENODEV;
   }
   
   msg->addr = client->addr;
   msg->flags = I2C_M_WR;
   msg->len = alength+1;
   msg->buf = my_i2c_data;
   
   msg->buf[0] = reg;   
   for (i=1; i<=alength; i++) msg->buf[i] = (*val++);
   
   err = i2c_transfer(client->adapter, msg, 1);
   udelay(50);


/*    printk(KERN_INFO "DBG blockwrite_reg addr:%x reg:%d data:%x %s\n",msg->addr,reg,val,(err<0?"ERROR":"")); */

/*    dev_dbg(&client->dev, "<%s> i2c Block write at 0x%x, " */
/*            "*val=%d flags=%d byte[%d] err=%d\n", */
/*            __func__, data[0], data[1], msg->flags, i, err); */
   return (err < 0?err:0);
}

/* 
 *  Read a bloc to a register in Tx device.
 */
int blockread_reg(struct i2c_client *client,
		  u8 reg, u16 alength, u8 *val)
{
   int err = 0;
   struct i2c_msg msg[1];
   u8 data[2];

   if (!client->adapter) {
      dev_err(&client->dev, "<%s> ERROR: No HDMI Device\n", __func__);
      return -ENODEV;
   }

   msg->addr = client->addr;
   msg->flags = I2C_M_WR;
   msg->len = 1;
   msg->buf = data;
   data[0] = reg; /* High byte goes out first */
   err = i2c_transfer(client->adapter, msg, 1);
/*    printk(KERN_INFO "DBG blockread_reg #1 addr:%x len:%d buf:%02x%02x%02x%02x %s\n",msg->addr,msg->len,\ */
/*           msg->buf[0],msg->buf[1],msg->buf[2],msg->buf[3],(err<0?"ERROR":"")); */
   if (err<0) goto BLOCK_READ_OUPS;

   msg->flags = I2C_M_RD;
   msg->len = alength; 
   msg->buf = val;
   err = i2c_transfer(client->adapter, msg, 1);
/*    printk(KERN_INFO "DBG blockread_reg #2 addr:%x len:%d buf:%02x%02x%02x%02x %s\n",msg->addr,msg->len,\ */
/*           msg->buf[0],msg->buf[1],msg->buf[2],msg->buf[3],(err<0?"ERROR":"")); */

   if (err<0) goto BLOCK_READ_OUPS;

   return 0;
   
 BLOCK_READ_OUPS:
   dev_err(&client->dev, "<%s> ERROR:  i2c Read at 0x%x, "
           "*val=%d flags=%d bytes err=%d\n",
           __func__, reg, *val, msg->flags, err);
   return err;
}

/* 
 *  Write a byte to a register in Tx device.
 */
int write_reg(struct i2c_client *client, u8 reg, u8 val)
{
   int err = 0;
   struct i2c_msg msg[1];
   u8 data[2];
   int retries = 0;

   if (!client->adapter) {
      dev_err(&client->dev, "<%s> ERROR: No HDMI Device\n", __func__);
      return -ENODEV;
   }

 retry:
   msg->addr = client->addr;
   msg->flags = I2C_M_WR;
   msg->len = 2;
   msg->buf = data;

   data[0] = reg;
   data[1] = val;

   err = i2c_transfer(client->adapter, msg, 1);
   dev_dbg(&client->dev, "<%s> i2c write at=%x "
	   "val=%x flags=%d err=%d\n",
	   __func__, data[0], data[1], msg->flags, err);
   udelay(50);

/*    printk(KERN_INFO "DBG write_reg addr:%x reg:%d data:%x %s\n",msg->addr,reg,val,(err<0?"ERROR":"")); */

   if (err >= 0)
      return 0;

   dev_err(&client->dev, "<%s> ERROR: i2c write at=%x "
	   "val=%x flags=%d err=%d\n",
	   __func__, data[0], data[1], msg->flags, err);
   if (retries <= 5) {
      dev_info(&client->dev, "Retrying I2C... %d\n", retries);
      retries++;
      set_current_state(TASK_UNINTERRUPTIBLE);
      schedule_timeout(msecs_to_jiffies(20));
      goto retry;
   }
   return err;
}

/*
 *  Read a byte from a register in Tx device.
 */
int read_reg(struct i2c_client *client, u16 data_length, u8 reg, u8 *val)
{
   int err = 0;
   struct i2c_msg msg[1];
   u8 data[2];

   if (!client->adapter) {
      dev_err(&client->dev, "<%s> ERROR: No HDMI Device\n", __func__);
      return -ENODEV;
   }

   msg->addr = client->addr;
   msg->flags = I2C_M_WR;
   msg->len = 1;
   msg->buf = data;

   data[0] = reg;
   err = i2c_transfer(client->adapter, msg, 1);
   dev_dbg(&client->dev, "<%s> i2c Read1 reg=%x val=%d "
	   "flags=%d err=%d\n",
	   __func__, reg, data[1], msg->flags, err);

   if (err >= 0) {
      mdelay(3);
      msg->flags = I2C_M_RD;
      msg->len = data_length;
      err = i2c_transfer(client->adapter, msg, 1);
   }

   if (err >= 0) {
      *val = 0;
      if (data_length == 1)
	 *val = data[0];
      else if (data_length == 2)
	 *val = data[1] + (data[0] << 8);
      dev_dbg(&client->dev, "<%s> i2c Read2 at 0x%x, *val=%d "
	      "flags=%d err=%d\n",
	      __func__, reg, *val, msg->flags, err);
      return 0;
   }

   dev_err(&client->dev, "<%s> ERROR: i2c Read at 0x%x, "
	   "*val=%d flags=%d err=%d\n",
	   __func__, reg, *val, msg->flags, err);
   return err;
}

/* The following function must be rewritten by the customer to fit its own    */
/* SW infrastructure. This function allows reading through I2C bus.           */
/* tmbslHdmiTxSysArgs_t definition is located into tmbslHdmiTx_type.h file.   */
tmErrorCode_t TxI2cReadFunction(tmbslHdmiTxSysArgs_t *pSysArgs)
{

   tmErrorCode_t errCode = TM_OK;
   struct i2c_client *client=GetThisI2cClient();
   u32 client_main_addr=client->addr;

   /* DevLib needs address control, so let it be */ 
   client->addr=pSysArgs->slaveAddr;

   if (pSysArgs->lenData == 1) {
      /* single byte */
      errCode = read_reg(GetThisI2cClient(),1,pSysArgs->firstRegister,pSysArgs->pData);
   }
   else {
      /* block */
      errCode = blockread_reg(GetThisI2cClient(), \
			      pSysArgs->firstRegister, \
			      pSysArgs->lenData, \
			      pSysArgs->pData);
   }
   
   /* restore default client address */
   client->addr=client_main_addr;

   return errCode;
}

/* The following function must be rewritten by the customer to fit its own    */
/* SW infrastructure. This function allows writing through I2C bus.           */
/* tmbslHdmiTxSysArgs_t definition is located into tmbslHdmiTx_type.h file.   */
tmErrorCode_t TxI2cWriteFunction(tmbslHdmiTxSysArgs_t *pSysArgs)
{

   tmErrorCode_t errCode = TM_OK;
   struct i2c_client *client=GetThisI2cClient();
   u32 client_main_addr=client->addr;

   /* DevLib needs address control, so let it be */ 
   client->addr=pSysArgs->slaveAddr;
   
   if (pSysArgs->lenData == 1) {
      /* single byte */
      errCode = write_reg(GetThisI2cClient(),pSysArgs->firstRegister,*pSysArgs->pData);
   }
   else {
      /* block */
      errCode = blockwrite_reg(GetThisI2cClient(),  \
                               pSysArgs->firstRegister, \
                               pSysArgs->lenData,       \
                               pSysArgs->pData);
   }
   
   /* restore default client address */
   client->addr=client_main_addr;

   return errCode;
}

tmErrorCode_t tda9983_edid_read(tmbslHdmiTxSysArgsEdid_t* pArg)
{
   struct i2c_client *client=GetThisI2cClient();
   struct i2c_msg msg[3];
   int index= 0;

    if (pArg->segPtrAddr != 0)
    {
        msg[index].addr=(pArg->segPtrAddr >> 1);
        msg[index].flags=0;
        msg[index].len=1;
        pArg->segPtr |= 0x01;
        msg[index].buf=&pArg->segPtr;
        index++;
    }

    msg[index].addr = (pArg->dataRegAddr >> 1);
    msg[index].flags = 0;
    msg[index].len=1;
    msg[index].buf=&pArg->wordOffset;
    index++;

    msg[index].addr = (pArg->dataRegAddr >> 1);
    msg[index].flags=I2C_M_RD;
    msg[index].len=pArg->lenData;
    msg[index].buf=pArg->pData;
    index++;
    i2c_transfer(client->adapter, msg, index);

    return TM_OK;
}

/******************************************************************************
    \brief  This function blocks the current task for the specified amount time. 
            This is a passive wait.

    \param  Duration    Duration of the task blocking in milliseconds.

    \return The call result:
            - TM_OK: the call was successful
            - TMDL_ERR_DLHDMITX_NO_RESOURCES: the resource is not available

******************************************************************************/
tmErrorCode_t tmdlHdmiTxIWWait
(
    UInt16 duration
)
{

    mdelay((unsigned long)duration);

    return(TM_OK);
}

/******************************************************************************
    \brief  This function creates a semaphore.

    \param  pHandle Pointer to the handle buffer.

    \return The call result:
            - TM_OK: the call was successful
            - TMDL_ERR_DLHDMITX_NO_RESOURCES: the resource is not available
            - TMDL_ERR_DLHDMITX_INCONSISTENT_PARAMS: an input parameter is
              inconsistent

******************************************************************************/
tmErrorCode_t tmdlHdmiTxIWSemaphoreCreate
(
    tmdlHdmiTxIWSemHandle_t *pHandle
)
{
    struct semaphore * mutex;
    
    /* check that input pointer is not NULL */
    RETIF(pHandle == Null, TMDL_ERR_DLHDMITX_INCONSISTENT_PARAMS)

    mutex = (struct semaphore *)kmalloc(sizeof(struct semaphore),GFP_KERNEL);
    if (!mutex) {
       printk(KERN_ERR "malloc failed in %s\n",__func__);
       return TMDL_ERR_DLHDMITX_NO_RESOURCES;
    }
    
    init_MUTEX(mutex);
    *pHandle = (tmdlHdmiTxIWSemHandle_t)mutex;

    RETIF(pHandle == NULL, TMDL_ERR_DLHDMITX_NO_RESOURCES)

    return(TM_OK);
}

/******************************************************************************
    \brief  This function destroys an existing semaphore.

    \param  Handle  Handle of the semaphore to be destroyed.

    \return The call result:
            - TM_OK: the call was successful
            - TMDL_ERR_DLHDMITX_BAD_HANDLE: the handle number is wrong

******************************************************************************/
tmErrorCode_t tmdlHdmiTxIWSemaphoreDestroy
(
    tmdlHdmiTxIWSemHandle_t handle
)
{
   RETIF(handle == False, TMDL_ERR_DLHDMITX_BAD_HANDLE);
   
   if (atomic_read((atomic_t*)&((struct semaphore *)handle)->count) < 1) {
      printk(KERN_ERR "release catched semaphore");
   }
   
   kfree((void*)handle);
   
   return(TM_OK);
}

/******************************************************************************
    \brief  This function acquires the specified semaphore.

    \param  Handle  Handle of the semaphore to be acquired.

    \return The call result:
            - TM_OK: the call was successful
            - TMDL_ERR_DLHDMITX_BAD_HANDLE: the handle number is wrong

******************************************************************************/
tmErrorCode_t tmdlHdmiTxIWSemaphoreP
(
    tmdlHdmiTxIWSemHandle_t handle
)
{

    down((struct semaphore *)handle);

    return(TM_OK);
}

/******************************************************************************
    \brief  This function releases the specified semaphore.

    \param  Handle  Handle of the semaphore to be released.

    \return The call result:
            - TM_OK: the call was successful
            - TMDL_ERR_DLHDMITX_BAD_HANDLE: the handle number is wrong

******************************************************************************/
tmErrorCode_t tmdlHdmiTxIWSemaphoreV
(
    tmdlHdmiTxIWSemHandle_t handle
)
{
    up((struct semaphore *)handle);

    return(TM_OK);
}

/*============================================================================*/
/*                              FUNCTIONS                                     */
/*============================================================================*/

/*****************************************************************************
******************************************************************************
*                THIS PART MUST NOT BE MODIFIED BY CUSTOMER                  *
******************************************************************************
*****************************************************************************/

/******************************************************************************
    \brief  Generate swap and mirror tables in function 
            of video port mapping tables.

    \param unit     Unit identifier.

    \return NA.

******************************************************************************/
static void dlHdmiTxGenerateCfgVideoPortTables
(
    tmUnitSelect_t                  unit,
    tmdlHdmiTxDriverConfigTable_t   *pConfig
)
{
    UInt8   i;
   
    for (i=0; i<6; i++)
    {
        /* CCIR656 */
        if (videoPortMapping_CCIR656[unit][i] != TMDL_HDMITX_VIDCCIR_NOT_CONNECTED)
        {
            pConfig->pSwapTableCCIR656[videoPortMapping_CCIR656[unit][i] & 0x07F] = 5-i;
            pConfig->pMirrorTableCCIR656[videoPortMapping_CCIR656[unit][i] & 0x07F] = (UInt8)(videoPortMapping_CCIR656[unit][i] >> 7);
            /* Enable port and disable ground port */
            if (((5-i) % 2) == 0)
            {
                pConfig->pEnableVideoPortCCIR656[i/2] |= 0x0F;
                pConfig->pGroundVideoPortCCIR656[i/2] &= 0xF0;
            }
            else
            {
                pConfig->pEnableVideoPortCCIR656[i/2] |= 0xF0;
                pConfig->pGroundVideoPortCCIR656[i/2] &= 0x0F;
            }
        }

        /* YUV422 */
        if (videoPortMapping_YUV422[unit][i] != TMDL_HDMITX_VID422_NOT_CONNECTED)
        {
            pConfig->pSwapTableYUV422[videoPortMapping_YUV422[unit][i] & 0x07F] = 5-i;
            pConfig->pMirrorTableYUV422[videoPortMapping_YUV422[unit][i] & 0x07F] = (UInt8)(videoPortMapping_YUV422[unit][i] >> 7);
            /* Enable port and disable ground port */
            if (((5-i) % 2) == 0)
            {
                pConfig->pEnableVideoPortYUV422[i/2] |= 0x0F;
                pConfig->pGroundVideoPortYUV422[i/2] &= 0xF0;
            }
            else
            {
                pConfig->pEnableVideoPortYUV422[i/2] |= 0xF0;
                pConfig->pGroundVideoPortYUV422[i/2] &= 0x0F;
            }
        }

        /* YUV444 */
        if (videoPortMapping_YUV444[unit][i] != TMDL_HDMITX_VID444_NOT_CONNECTED)
        {
            pConfig->pSwapTableYUV444[videoPortMapping_YUV444[unit][i] & 0x07F] = 5-i;
            pConfig->pMirrorTableYUV444[videoPortMapping_YUV444[unit][i] & 0x07F] = (UInt8)(videoPortMapping_YUV444[unit][i] >> 7);
            /* Enable port and disable ground port */
            if (((5-i) % 2) == 0)
            {
                pConfig->pEnableVideoPortYUV444[i/2] |= 0x0F;
                pConfig->pGroundVideoPortYUV444[i/2] &= 0xF0;
            }
            else
            {
                pConfig->pEnableVideoPortYUV444[i/2] |= 0xF0;
                pConfig->pGroundVideoPortYUV444[i/2] &= 0x0F;
            }
        }

        /* RGB444 */
        if (videoPortMapping_RGB444[unit][i] != TMDL_HDMITX_VID444_NOT_CONNECTED)
        {
            pConfig->pSwapTableRGB444[videoPortMapping_RGB444[unit][i] & 0x07F] = 5-i;
            pConfig->pMirrorTableRGB444[videoPortMapping_RGB444[unit][i] & 0x07F] = (UInt8)(videoPortMapping_RGB444[unit][i] >> 7);
            /* Enable port and disable ground port */
            if (((5-i) % 2) == 0)
            {
                pConfig->pEnableVideoPortRGB444[i/2] |= 0x0F;
                pConfig->pGroundVideoPortRGB444[i/2] &= 0xF0;
            }
            else
            {
                pConfig->pEnableVideoPortRGB444[i/2] |= 0xF0;
                pConfig->pGroundVideoPortRGB444[i/2] &= 0x0F;
            }
        }
    }
}

/******************************************************************************
    \brief This function allows to the main driver to retrieve its
           configuration parameters.

    \param pConfig Pointer to the config structure

    \return The call result:
            - TM_OK: the call was successful
            - TMDL_ERR_DLHDMITX_BAD_UNIT_NUMBER: the unit number is wrong or
              the receiver instance is not initialised
            - TMDL_ERR_DLHDMITX_INCONSISTENT_PARAMS: an input parameter is
              inconsistent

******************************************************************************/
tmErrorCode_t tmdlHdmiTxCfgGetConfig
(
    tmUnitSelect_t                  unit,
    tmdlHdmiTxDriverConfigTable_t   *pConfig
)
{
    /* Check if unit number is in range */
    RETIF((unit < 0) || (unit >= MAX_UNITS), TMDL_ERR_DLHDMITX_BAD_UNIT_NUMBER)

    /* Check if pointer is Null */
    RETIF(pConfig == Null, TMDL_ERR_DLHDMITX_INCONSISTENT_PARAMS)

    *pConfig = driverConfigTableTx[unit];

    /* Done here because of const declaration of tables in ARM7 case */   
    pConfig->pResolutionInfo = (ptmdlHdmiTxCfgResolution_t)resolutionInfoTx;
    

    /* Generate swap and mirror tables in function of video port mapping tables */
    dlHdmiTxGenerateCfgVideoPortTables(unit, pConfig);

    return TM_OK;
}



/*============================================================================*/
/*                            END OF FILE                                     */
/*============================================================================*/
