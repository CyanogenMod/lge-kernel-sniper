/**
 * Copyright (C) 2006 NXP N.V., All Rights Reserved.
 * This source code and any compilation or derivative thereof is the proprietary
 * information of NXP N.V. and is confidential in nature. Under no circumstances
 * is this software to be  exposed to or placed under an Open Source License of
 * any type without the expressed written permission of NXP N.V.
 *
 * \file          tmdlHdmiTx_WinCfg.c
 *
 * \version       Revision: 1
 *
 * \date          Date: 22/02/08
 *
 * \brief         devlib driver component API for the TDA998x HDMI Transmitters
 *
 * \section refs  Reference Documents
 * HDMI Tx Driver - FRS.doc,
 *
 * \section info  Change Information
 *
 * \verbatim

   History:       tmdlHdmiTx_WinCfg.c
 *
 * *****************  Version 1  *****************
 * User: G. Burnouf Date: 22/02/08
 * initial version
 *

   \endverbatim
 *
*/

#include "tmdlHdmiTx_IW.h"
#include "tmNxTypes.h"
#include "tmdlHdmiTx.h"
#include "tmdlHdmiTx_cfg.h"
#include "tmbslTDA9984_Functions.h"

/******************************************************************************
 ******************************************************************************
 *                 THIS PART CAN BE MODIFIED BY CUSTOMER                      *
 ******************************************************************************
 *****************************************************************************/

/* The two following includes are used by I2C access function for ARM7. If    */
/* you need to rewrite these functions for your own SW infrastructure, then   */
/* it can be removed                                                          */
#include "infra_i2c.h"


/* I2C adress of the unit                                                     */
/* Put there the I2C slave adress of the Tx transmitter IC                    */
#define UNIT_I2C_ADDRESS_0 0x70

/* Priority of the command task                                               */
/* Command task is an internal task that handles incoming event from the IC   */
/* put there a value that will ensure a response time of ~20ms in your system */
#define COMMAND_TASK_PRIORITY_0  (UInt8)THREAD_PRIORITY_HIGHEST

/* Priority of the hdcp check tasks */
/* HDCP task is an internal task that handles periodical HDCP processing      */
/* put there a value that will ensure a response time of ~20ms in your system */
#define HDCP_CHECK_TASK_PRIORITY_0  (UInt8)THREAD_PRIORITY_HIGHEST

/* Stack size of the command tasks */
/* This value depends of the type of CPU used, and also from the length of    */
/* the customer callbacks. Increase this value if you are making a lot of     */
/* processing (function calls & local variables) and that you experience      */
/* stack overflows                                                            */
#define COMMAND_TASK_STACKSIZE_0 128

/* stack size of the hdcp check tasks */
/* This value depends of the type of CPU used, default value should be enough */
/* for all configuration                                                      */
#define HDCP_CHECK_TASK_STACKSIZE_0 128

/* Size of the message queues for command tasks                               */
/* This value defines the size of the message queue used to link the          */
/* the tmdlHdmiTxHandleInterrupt function and the command task. The default   */
/* value below should fit any configuration                                   */
#define COMMAND_TASK_QUEUESIZE_0 128

/* HDCP key seed                                                              */
/* HDCP key are stored encrypted into the IC, this value allows the IC to     */
/* decrypt them. This value is provided to the customer by NXP customer       */
/* support team.                                                              */
#define KEY_SEED 0x1234

/* Video port configuration for YUV444 input                                  */
/* You can specify in this table how are connected video ports in case of     */
/* YUV444 input signal. Each line of the array corresponds to a quartet of    */
/* pins of one video port (see comment on the left to identify them). Just    */
/* change the enum to specify which signal you connected to it. See file      */
/* tmdlHdmiTx_cfg.h to get the list of possible values                        */
tmdlHdmiTxCfgVideoSignal444 videoPortMapping_YUV444[MAX_UNITS][6] = {
    {
        TMDL_HDMITX_VID444_BU_0_TO_3,   /* Signals connected to VPA[0..3] */
        TMDL_HDMITX_VID444_BU_4_TO_7,   /* Signals connected to VPA[4..7] */
        TMDL_HDMITX_VID444_GY_0_TO_3,   /* Signals connected to VPB[0..3] */
        TMDL_HDMITX_VID444_GY_4_TO_7,   /* Signals connected to VPB[4..7] */
        TMDL_HDMITX_VID444_VR_0_TO_3,   /* Signals connected to VPC[0..3] */
        TMDL_HDMITX_VID444_VR_4_TO_7    /* Signals connected to VPC[4..7] */
    }
};

/* Video port configuration for RGB444 input                                  */
/* You can specify in this table how are connected video ports in case of     */
/* RGB444 input signal. Each line of the array corresponds to a quartet of    */
/* pins of one video port (see comment on the left to identify them). Just    */
/* change the enum to specify which signal you connected to it. See file      */
/* tmdlHdmiTx_cfg.h to get the list of possible values                        */
tmdlHdmiTxCfgVideoSignal444 videoPortMapping_RGB444[MAX_UNITS][6] = {
    {
        TMDL_HDMITX_VID444_BU_0_TO_3,   /* Signals connected to VPA[0..3] */
        TMDL_HDMITX_VID444_BU_4_TO_7,   /* Signals connected to VPA[4..7] */
        TMDL_HDMITX_VID444_GY_0_TO_3,   /* Signals connected to VPB[0..3] */
        TMDL_HDMITX_VID444_GY_4_TO_7,   /* Signals connected to VPB[4..7] */
        TMDL_HDMITX_VID444_VR_0_TO_3,   /* Signals connected to VPC[0..3] */
        TMDL_HDMITX_VID444_VR_4_TO_7    /* Signals connected to VPC[4..7] */
    }
};

/* Video port configuration for YUV422 input                                  */
/* You can specify in this table how are connected video ports in case of     */
/* YUV422 input signal. Each line of the array corresponds to a quartet of    */
/* pins of one video port (see comment on the left to identify them). Just    */
/* change the enum to specify which signal you connected to it. See file      */
/* tmdlHdmiTx_cfg.h to get the list of possible values                        */
tmdlHdmiTxCfgVideoSignal422 videoPortMapping_YUV422[MAX_UNITS][6] = {
    {
        TMDL_HDMITX_VID422_Y_4_TO_7,           /* Signals connected to VPA[0..3] */    
        TMDL_HDMITX_VID422_Y_8_TO_11,          /* Signals connected to VPA[4..7] */    
        TMDL_HDMITX_VID422_UV_4_TO_7,          /* Signals connected to VPB[0..3] */
        TMDL_HDMITX_VID422_UV_8_TO_11,         /* Signals connected to VPB[4..7] */
        TMDL_HDMITX_VID422_NOT_CONNECTED,      /* Signals connected to VPC[0..3] */
        TMDL_HDMITX_VID422_NOT_CONNECTED       /* Signals connected to VPC[4..7] */
    }
};

/* Video port configuration for CCIR656 input                                 */
/* You can specify in this table how are connected video ports in case of     */
/* CCIR656 input signal. Each line of the array corresponds to a quartet of   */
/* pins of one video port (see comment on the left to identify them). Just    */
/* change the enum to specify which signal you connected to it. See file      */
/* tmdlHdmiTx_cfg.h to get the list of possible values                        */
tmdlHdmiTxCfgVideoSignalCCIR656 videoPortMapping_CCIR656[MAX_UNITS][6] = {
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
UInt8 groundAudioPortSPDIF[MAX_UNITS]      = {0xbf};
UInt8 groundAudioClockPortSPDIF[MAX_UNITS] = {ENABLE_AUDIO_CLOCK_PORT_PULLDOWN};

/* Audio port configuration for I2S                                           */
/* Here you can specify the audio port routing configuration for SPDIF input. */
/* enableAudioPortI2S and groundAudioPortI2S should be filled with a          */
/* value build as follows : each bit represent an audio port, LSB is port 0.  */
/* enableAudioClockPortI2S and groundAudioClockPortI2S can be configured      */
/* with the corresponding enums (See file tmdlHdmiTx_cfg.h for more details). */
UInt8 enableAudioPortI2S[MAX_UNITS]        = {0x03};
UInt8 enableAudioPortI2S8C[MAX_UNITS]      = {0x1f};
UInt8 enableAudioClockPortI2S[MAX_UNITS]   = {ENABLE_AUDIO_CLOCK_PORT};
UInt8 groundAudioPortI2S[MAX_UNITS]        = {0xfc};
UInt8 groundAudioPortI2S8C[MAX_UNITS]      = {0xe0};
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
UInt8 enableAudioPortHBR[MAX_UNITS]        = {0x1f};
UInt8 enableAudioClockPortHBR[MAX_UNITS]   = {ENABLE_AUDIO_CLOCK_PORT};
UInt8 groundAudioPortHBR[MAX_UNITS]        = {0xe0};
UInt8 groundAudioClockPortHBR[MAX_UNITS]   = {DISABLE_AUDIO_CLOCK_PORT_PULLDOWN};

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

/*****************************************************************************
******************************************************************************
*                THIS PART MUST NOT BE MODIFIED BY CUSTOMER                  *
******************************************************************************
*****************************************************************************/

/* DO NOT MODIFY, those tables are filled dynamically by                      */
/* dlHdmiTxGenerateCfgVideoPortTables API                                     */
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

/* DO NOT MODIFY, this table is used for transmission of the configuration to */
/* the core driver                                                            */
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
        Null,                           /* filled dynamically, do not modify */
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
        1                               /* DE signal is available */
    }
 };


/*============================================================================*/
/*                            END OF FILE                                     */
/*============================================================================*/
