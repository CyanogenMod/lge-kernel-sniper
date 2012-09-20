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
#include "tmdlHdmiTx_IW.h"
#include "tmNxTypes.h"
#include "tmdlHdmiTx.h"
#include "tmdlHdmiTx_cfg.h"

#ifdef TMFL_OS_WINDOWS      /* OS Windows */
#include "infra_i2c.h"
#else                       /* OS ARM7 */
#include "I2C.h"
#include <LPC21xx.H>   
#endif                      /* endif TMFL_OS_WINDOWS */

#include "tmbslHdmiTx.h"

/******************************************************************************
 ******************************************************************************
 *                 THIS PART CAN BE MODIFIED BY CUSTOMER                      *
 ******************************************************************************
 *****************************************************************************/

/*============================================================================*/
/*                          INTERNAL PROTOTYPE                                */
/*============================================================================*/
#ifdef TMFL_OS_WINDOWS  /* OS Windows */ 
tmErrorCode_t windowsI2cReadFunction(tmbslHdmiTxSysArgs_t *pSysArgs);
tmErrorCode_t windowsI2cWriteFunction(tmbslHdmiTxSysArgs_t *pSysArgs);
#else                   /* OS ARM7 */
tmErrorCode_t RtxTxI2cReadFunction(tmbslHdmiTxSysArgs_t *pSysArgs);
tmErrorCode_t RtxTxI2cWriteFunction(tmbslHdmiTxSysArgs_t *pSysArgs);
#endif                  /* endif TMFL_OS_WINDOWS */

tmErrorCode_t edidReadFunction (tmbslHdmiTxSysArgsEdid_t *pSysArgs);

/*============================================================================*/
/*                                MACRO                                       */
/*============================================================================*/

/* macro for quick error handling */
#define RETIF(cond, rslt) if ((cond)){return (rslt);}

/*============================================================================*/
/*                          TYPES DECLARATIONS                                */
/*============================================================================*/

/*============================================================================*/
/*                       CONSTANTS DECLARATIONS                               */
/*============================================================================*/

/* Configuration for unit 0 *************************** */
/* priority of the command tasks */
#ifdef TMFL_OS_WINDOWS          /* OS Windows */
#define COMMAND_TASK_PRIORITY_0  THREAD_PRIORITY_HIGHEST
#else                           /* OS ARM7 */
#define COMMAND_TASK_PRIORITY_0  250
#endif                          /* endif TMFL_OS_WINDOWS */
/* stack size of the command tasks */
#define COMMAND_TASK_STACKSIZE_0 128
/* size of the message queues for command tasks */
#define COMMAND_TASK_QUEUESIZE_0 128

/* priority of the hdcp check tasks */
#ifdef TMFL_OS_WINDOWS          /* OS Windows */
#define HDCP_CHECK_TASK_PRIORITY_0  THREAD_PRIORITY_HIGHEST
#else                           /* OS ARM7 */
#define HDCP_CHECK_TASK_PRIORITY_0  250
#endif                          /* endif TMFL_OS_WINDOWS */

/* stack size of the hdcp check tasks */
#define HDCP_CHECK_TASK_STACKSIZE_0 128
/* I2C adress of the unit */
#define UNIT_I2C_ADDRESS_0 0x70

/* Configuration for unit 1 *************************** */
/* priority of the command tasks */
#ifdef TMFL_OS_WINDOWS          /* OS Windows */
#define COMMAND_TASK_PRIORITY_1  THREAD_PRIORITY_HIGHEST
#else                           /* OS ARM7 */
#define COMMAND_TASK_PRIORITY_1  250
#endif                          /* endif TMFL_OS_WINDOWS */
/* stack size of the command tasks */
#define COMMAND_TASK_STACKSIZE_1 128
/* size of the message queues for command tasks */
#define COMMAND_TASK_QUEUESIZE_1 128

/* priority of the hdcp check tasks */
#ifdef TMFL_OS_WINDOWS          /* OS Windows */
#define HDCP_CHECK_TASK_PRIORITY_1  THREAD_PRIORITY_HIGHEST
#else                           /* OS ARM7 */
#define HDCP_CHECK_TASK_PRIORITY_1  250
#endif                          /* endif TMFL_OS_WINDOWS */

/* stack size of the hdcp check tasks */
#define HDCP_CHECK_TASK_STACKSIZE_1 128
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

#ifdef TMFL_OS_WINDOWS          /* OS Windows */
tmdlHdmiTxCfgResolution_t resolutionInfoTx[RESOLUTION_NB] = {
#else                           /* OS ARM7 */
const tmdlHdmiTxCfgResolution_t resolutionInfoTx[RESOLUTION_NB] = {
#endif                          /* endif TMFL_OS_WINDOWS */
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



#ifdef TMFL_OS_WINDOWS          /* OS Windows */
tmdlHdmiTxCfgVideoSignal444 videoPortMapping_YUV444[MAX_UNITS][6] = {
#else                           /* OS ARM7 */
const tmdlHdmiTxCfgVideoSignal444 videoPortMapping_YUV444[MAX_UNITS][6] = {
#endif                          /* endif TMFL_OS_WINDOWS */
    {
        TMDL_HDMITX_VID444_BU_0_TO_3,   /* Signals connected to VPA[0..3] */
        TMDL_HDMITX_VID444_BU_4_TO_7,   /* Signals connected to VPA[4..7] */
        TMDL_HDMITX_VID444_GY_0_TO_3,   /* Signals connected to VPB[0..3] */
        TMDL_HDMITX_VID444_GY_4_TO_7,   /* Signals connected to VPB[4..7] */
        TMDL_HDMITX_VID444_VR_0_TO_3,   /* Signals connected to VPC[0..3] */
        TMDL_HDMITX_VID444_VR_4_TO_7    /* Signals connected to VPC[4..7] */
    }
};

#ifdef TMFL_OS_WINDOWS          /* OS Windows */
tmdlHdmiTxCfgVideoSignal444 videoPortMapping_RGB444[MAX_UNITS][6] = {
#else                           /* OS ARM7 */
const tmdlHdmiTxCfgVideoSignal444 videoPortMapping_RGB444[MAX_UNITS][6] = {
#endif                          /* endif TMFL_OS_WINDOWS */
    {
        TMDL_HDMITX_VID444_BU_0_TO_3,   /* Signals connected to VPA[0..3] */
        TMDL_HDMITX_VID444_BU_4_TO_7,   /* Signals connected to VPA[4..7] */
        TMDL_HDMITX_VID444_GY_0_TO_3,   /* Signals connected to VPB[0..3] */
        TMDL_HDMITX_VID444_GY_4_TO_7,   /* Signals connected to VPB[4..7] */
        TMDL_HDMITX_VID444_VR_0_TO_3,   /* Signals connected to VPC[0..3] */
        TMDL_HDMITX_VID444_VR_4_TO_7    /* Signals connected to VPC[4..7] */
    }
};

#ifdef TMFL_OS_WINDOWS          /* OS Windows */
tmdlHdmiTxCfgVideoSignal422 videoPortMapping_YUV422[MAX_UNITS][6] = {
#else                           /* OS ARM7 */
const tmdlHdmiTxCfgVideoSignal422 videoPortMapping_YUV422[MAX_UNITS][6] = {
#endif                          /* endif TMFL_OS_WINDOWS */
   
  {
        TMDL_HDMITX_VID422_Y_4_TO_7,           /* Signals connected to VPA[0..3] */    
        TMDL_HDMITX_VID422_Y_8_TO_11,          /* Signals connected to VPA[4..7] */    
        TMDL_HDMITX_VID422_UV_4_TO_7,          /* Signals connected to VPB[0..3] */
        TMDL_HDMITX_VID422_UV_8_TO_11,         /* Signals connected to VPB[4..7] */
        TMDL_HDMITX_VID422_NOT_CONNECTED,      /* Signals connected to VPC[0..3] */
        TMDL_HDMITX_VID422_NOT_CONNECTED       /* Signals connected to VPC[4..7] */
  }
 };

#ifdef TMFL_OS_WINDOWS          /* OS Windows */
tmdlHdmiTxCfgVideoSignalCCIR656 videoPortMapping_CCIR656[MAX_UNITS][6] = {
#else                           /* OS ARM7 */
const tmdlHdmiTxCfgVideoSignalCCIR656 videoPortMapping_CCIR656[MAX_UNITS][6] = {
#endif                          /* endif TMFL_OS_WINDOWS */
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
#ifdef TMFL_OS_WINDOWS         /* OS Windows */
tmdlHdmiTxDriverConfigTable_t driverConfigTableTx[MAX_UNITS] = {
    {
        COMMAND_TASK_PRIORITY_0,
        COMMAND_TASK_STACKSIZE_0,
        COMMAND_TASK_QUEUESIZE_0,
        HDCP_CHECK_TASK_PRIORITY_0,
        HDCP_CHECK_TASK_STACKSIZE_0,
        UNIT_I2C_ADDRESS_0,
        windowsI2cReadFunction,
        windowsI2cWriteFunction,
        edidReadFunction,
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
        TMDL_HDMITX_PATTERN_BLUE,
        1                               /* by default, the DE signal is considered available */
    }
};
#else            /* OS ARM7 */
tmdlHdmiTxDriverConfigTable_t driverConfigTableTx[MAX_UNITS] = {
    {
        COMMAND_TASK_PRIORITY_0,
        COMMAND_TASK_STACKSIZE_0,
        COMMAND_TASK_QUEUESIZE_0,
        HDCP_CHECK_TASK_PRIORITY_0,
        HDCP_CHECK_TASK_STACKSIZE_0,
        UNIT_I2C_ADDRESS_0,
        RtxTxI2cReadFunction,
        RtxTxI2cWriteFunction,
        edidReadFunction,
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
        TMDL_HDMITX_PATTERN_BLUE,
        1                               /* by default, the DE signal is considered available */
    }
};
#endif              /* endif TMFL_OS_WINDOWS */

/*============================================================================*/
/*                              FUNCTIONS                                     */
/*============================================================================*/
#ifdef TMFL_OS_WINDOWS          /* OS Windows */
/******************************************************************************
    \brief Read to BSL driver through I2C bus

    \param pSysArgs Pointer to the I2C read structure

    \return The call result:
            - TM_OK: the call was successful

******************************************************************************/
tmErrorCode_t windowsI2cReadFunction
(
    tmbslHdmiTxSysArgs_t    *pSysArgs
)
{
    tmErrorCode_t   errCode;

    errCode = i2cRead(reg_TDA998X, (tmbslHdmiSysArgs_t *) pSysArgs);

    return errCode;
}

/******************************************************************************
    \brief Write to BSL driver through I2C bus

    \param pSysArgs Pointer to the I2C write structure

    \return The call result:
            - TM_OK: the call was successful

******************************************************************************/
tmErrorCode_t windowsI2cWriteFunction
(
    tmbslHdmiTxSysArgs_t    *pSysArgs
)
{
    tmErrorCode_t   errCode;

    errCode = i2cWrite(reg_TDA998X, (tmbslHdmiSysArgs_t *) pSysArgs);

    return errCode;
}

#else         /* OS ARM7 */
/*============================================================================*/

/******************************************************************************
    \brief Write to BSL driver through I2C bus

    \param pSysArgs Pointer to the I2C read structure

    \return The call result:
            - TM_OK: the call was successful

******************************************************************************/
tmErrorCode_t
RtxTxI2cReadFunction
(
    tmbslHdmiTxSysArgs_t        *pSysArgs
)
{
    tmErrorCode_t               errCode;

    errCode = i2cRead(reg_TDA998X, (tmbslHdmiSysArgs_t *) pSysArgs);

    return errCode;
}

/******************************************************************************
    \brief Write to BSL driver through I2C bus

    \param pSysArgs Pointer to the I2C write structure

    \return The call result:
            - TM_OK: the call was successful

******************************************************************************/
tmErrorCode_t
RtxTxI2cWriteFunction
(
    tmbslHdmiTxSysArgs_t *pSysArgs
)
{
    tmErrorCode_t               errCode;

    errCode = i2cWrite(reg_TDA998X,(tmbslHdmiSysArgs_t *) pSysArgs);

    return errCode;
}

#endif      /* endif TMFL_OS_WINDOWS */

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



/******************************************************************************
    \brief Driver callback function for i2c EDID read (REQUIRED SW INTERFACE)

    \param 
    \param 

    \return The call result:
            TM_OK: the call was successfull 
            TMBSL_ERR_HDMI_I2C_READ: failed when reading the I2C bus
             

******************************************************************************/
tmErrorCode_t edidReadFunction (tmbslHdmiTxSysArgsEdid_t *pSysArgs)
{
    UInt8 seg_address = pSysArgs->segPtrAddr;
    UInt8 segptr = pSysArgs->segPtr;
    UInt8 data_address = pSysArgs->dataRegAddr;
    UInt8 offset = pSysArgs->wordOffset;
    UInt8 *ptr = pSysArgs->pData;
    UInt8 nb_char = pSysArgs->lenData;
    UInt8 i=0;
    tmErrorCode_t       Err;


    do
    {
        Err = (i2cReadEdid(seg_address, segptr, data_address, offset, nb_char, ptr))? TMBSL_ERR_HDMI_I2C_READ : TM_OK;
        i++;
    } while ( (Err != TM_OK) && (i<3) );


    return Err;
}



/*============================================================================*/
/*                            END OF FILE                                     */
/*============================================================================*/
