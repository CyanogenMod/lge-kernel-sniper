/**
 * Copyright (C) 2006 NXP N.V., All Rights Reserved.
 * This source code and any compilation or derivative thereof is the proprietary
 * information of NXP N.V. and is confidential in nature. Under no circumstances
 * is this software to be  exposed to or placed under an Open Source License of
 * any type without the expressed written permission of NXP N.V.
 *
 * \file          tmbslTDA9984_misc.c
 *
 * \version       Revision: 26
 *
 * \date          Date: 05/02/08
 *
 * \brief         BSL driver component API for the TDA9984 HDMI Transmitter
 *
 * \section refs  Reference Documents
 *
 * \section info  Change Information
 *
 * \verbatim

   History: tmbslTDA9974_misc.c
 *
 * **************** Version 26  ******************
 * User: G.Burnouf     Date: 05/02/08
 * Updated in $/Source/tmbslTDA9984/Src
 * PR1251 : init pDis EDIDBasicDisplayParam
 *
 * **************** Version 25  ******************
 * User: G.Burnouf     Date: 22/01/08
 * Updated in $/Source/tmbslTDA9984/Src
 * PR1271 : remove qmore warning
 *
 * **************** Version 24  ******************
 * User: G.Burnouf     Date: 04/01/07
 * Updated in $/Source/tmbslTDA9984/Src
 * PR1164 : remove occurences on alternate EDID
 *
 * **************** Version 23  ******************
 * User: G.Burnouf     Date: 10/12/07   Time: 08:30
 * Updated in $/Source/tmbslTDA9984/Src
 * PR1145 : return DTD and monitor description
 *
 * *****************  Version 22  ****************
 * User: G.Burnouf     Date: 30/11/07   Time: 11:50
 * Updated in $/Source/tmbslTDA9984/Src
 * PR1038 : Apply same types for all projects
 *
 * *****************  Version 21  ****************
 * User: J. Lamotte    Date: 29/11/07   Time: 17:51
 * Updated in $/Source/tmbslTDA9984/Src
 * PR1108 : Change default setting of pDis
 *          enaAudioPortTableBackup, 
 *          gndAudioPortTableBackup,
 *          enaAudioClockPortTableBackup,
 *          gndAudioClockPortTableBackup
 *
 * *****************  Version 20  ****************
 * User: G.Burnouf     Date: 28/11/07   Time: 14:00
 * Updated in $/Source/tmbslTDA9984/Src
 * PR1038 : Apply same API for all projects
 *
 * *****************  Version 19  ****************
 * User: B.Vereecke     Date: 31/10/07   Time: 10:51
 * Updated in $/Source/tmbslTDA9984/Src
 * PR848 : Remove qmore errors
 *
 * *****************  Version 18  *****************
 * User: B.Vereecke     Date: 26/10/07   Time: 15:50
 * Updated in $/Source/tmbslTDA9984/Src
 * PR850 : remove external library dependancy
 * 
 * *****************  Version 17  ****************
 * User: B.Vereecke     Date: 26/10/07   Time: 14:51
 * Updated in $/Source/tmbslTDA9984/Src
 * PR951 : HPD Workaround in tmbslTDA9984HwHandleInterrupt
 *
 * *****************  Version 16  ****************
 * User: B.Vereecke     Date: 25/10/07   Time: 14:51
 * Updated in $/Source/tmbslTDA9984/Src
 * PR707 : Workaround in tmbslTDA9984AudioOutSetMute
 *
 * *****************  Version 15  ****************
 * User: G.Burnouf      Date: 25/10/07   Time: 10:07
 * Updated in $/Source/tmbslTDA9984/Src
 * PR780 : change state machine for stand by mode
 *
 * *****************  Version 14  ****************
 * User: G.Burnouf      Date: 23/10/07   Time: 14:51
 * Updated in $/Source/tmbslTDA9984/Src
 * PR926 : Add callback for IT VS
 *
 * *****************  Version 13  ****************
 * User: B.Vereecke     Date: 11/10/07   Time: 14:51
 * Updated in $/Source/tmbslTDA9984/Src
 * PR814 : Remove compilation warnings
 *
 * *****************  Version 12  *****************
 * User: B. Vereecke    Date: 09/10/07   Time: 09:26
 * Updated in $/Source/tmbslTDA9984/Src
 * PR 623 : EDID absence detection
 *
 * *****************  Version 11  *****************
 * User: B. Vereecke    Date: 08/10/07   Time: 09:51
 * Updated in $/Source/tmbslTDA9984/Src
 * PR 798 : Compilation error with NO_HDCP flag
 *
 * *****************  Version 10  *****************
 * User: G. Burnouf    Date: 05/10/07   Time: 15:32
 * Updated in $/Source/tmbslTDA9984/Src
 * PR579 : remove test of sysFuncEdidRead pointer 
 *
 * *****************  Version 9  *****************
 * User: G. Burnouf    Date: 05/10/07   Time: 15:32
 * Updated in $/Source/tmbslTDA9984/Src
 * PR824 : Change type of kITCallbackPriority
 *
 * *****************  Version 8  *****************
 * User: G. Burnouf    Date: 05/10/07   Time: 15:32
 * Updated in $/Source/tmbslTDA9984/Src
 * PR809 : Change Interrupt priority
 *
 * *****************  Version 7  *****************
 * User: B. Vereecke    Date: 03/10/07   Time: 15:32
 * Updated in $/Source/tmbslTDA9984/Src
 * PR778 : EDID workaround (soft reset while HPD 
 *          is low)
 *
 * *****************  Version 6  *****************
 * User: B.Vereecke     Date: 02/10/07   Time: 15:39
 * Updated in $/Source/tmbslHdmiTx/src
 * PR628 : init the clock speed of the DDC channel 
 * 
 * *****************  Version 5  *****************
 * User: B.Vereecke     Date: 28/09/07   Time: 15:39
 * Updated in $/Source/tmbslHdmiTx/src
 * PR766 : Rename pseudo API functions
 * 
 * *****************  Version 4  *****************
 * User: J.Turpin     Date: 13/09/07   Time: 14:32
 * Updated in $/Source/tmbslHdmiTx/src
 * PR693 : add black pattern functionality
 * 
 * *****************  Version 3  *****************
 * User: G.Burnouf     Date: 07/08/07   Time: 10:30
 * Updated in $/Source/tmbslHdmiTx/src
 * PR551 : add blue pattern functionality
 * 
 * *****************  Version 2  *****************
 * User: G. Burnouf    Date: 05/07/07   Time: 17:00
 * Updated in $/Source/tmbslTDA9984/Src
 * PR 570 : -Clear Edid request on hot plug
 *          -Enable Edid IT
 *          -Disable HotPlug and RxSense IT on 
 *           call of function tmbslTDA9984HwHandleInterrupt
 *           by function tmbslTDA9984Init
 *
 * *****************  Version 1  *****************
 * User: G. Burnouf    Date: 05/07/07   Time: 17:00
 * Updated in $/Source/tmbslTDA9984/Src
 * PR 414 : Add new edid management
 *

   \endverbatim
 *
*/

/*============================================================================*/
/*                       INCLUDE FILES                                        */
/*============================================================================*/

#include "tmbslHdmiTx_types.h"
#include "tmbslTDA9984_Functions.h"
#include "tmbslTDA9984_local.h"
#include "tmbslTDA9984_State_l.h"
#include "tmbslTDA9984_InOut_l.h"
#include "tmbslTDA9984_HDCP_l.h"
#include "tmbslTDA9984_Edid_l.h"
#include "tmbslTDA9984_Misc_l.h"

/*============================================================================*/
/*                     TYPES DECLARATIONS                                     */
/*============================================================================*/

/*============================================================================*/
/*                       CONSTANTS DECLARATIONS EXPORTED                      */
/*============================================================================*/

/**
 * Lookup table of input port control registers and their swap and mirror masks
 */
CONST_DAT tmbslTDA9984RegVip
 kRegVip[HDMITX_VIN_PORT_MAP_TABLE_LEN] =
{
    {E_REG_P00_VIP_CNTRL_0_W,
        E_MASKREG_P00_VIP_CNTRL_0_swap_a,
        E_MASKREG_P00_VIP_CNTRL_0_mirr_a
    }, /* Port group 0 */
    {E_REG_P00_VIP_CNTRL_0_W,
        E_MASKREG_P00_VIP_CNTRL_0_swap_b,
        E_MASKREG_P00_VIP_CNTRL_0_mirr_b
    }, /* Port group 1 */
    {E_REG_P00_VIP_CNTRL_1_W,
        E_MASKREG_P00_VIP_CNTRL_1_swap_c,
        E_MASKREG_P00_VIP_CNTRL_1_mirr_c
    }, /* Port group 2 */
    {E_REG_P00_VIP_CNTRL_1_W,
        E_MASKREG_P00_VIP_CNTRL_1_swap_d,
        E_MASKREG_P00_VIP_CNTRL_1_mirr_d
    }, /* Port group 3 */
    {E_REG_P00_VIP_CNTRL_2_W,
        E_MASKREG_P00_VIP_CNTRL_2_swap_e,
        E_MASKREG_P00_VIP_CNTRL_2_mirr_e
    }, /* Port group 4 */
    {E_REG_P00_VIP_CNTRL_2_W,
        E_MASKREG_P00_VIP_CNTRL_2_swap_f,
        E_MASKREG_P00_VIP_CNTRL_2_mirr_f
    }  /* Port group 5 */
};


/*============================================================================*/
/*                       CONSTANTS DECLARATIONS                               */
/*============================================================================*/

/** Preset default values for an object instance */
static CONST_DAT tmHdmiTxobject_t kHdmiTxInstanceDefault
=
{
    ST_UNINITIALIZED,                   /* state */
    0,                                  /* nIgnoredEvents */
    tmUnit0,                            /* txUnit */
    0,                                  /* uHwAddress */
    (ptmbslHdmiTxSysFunc_t)0,           /* sysFuncWrite */
    (ptmbslHdmiTxSysFunc_t)0,           /* sysFuncRead */
    (ptmbslHdmiTxSysFuncEdid_t)0,       /* sysFuncEdidRead */
    (ptmbslHdmiTxSysFuncTimer_t)0,      /* sysFuncTimer */
    {                                   /* funcIntCallbacks[] */
        (ptmbslHdmiTxCallback_t)0,
        (ptmbslHdmiTxCallback_t)0,
        (ptmbslHdmiTxCallback_t)0,
        (ptmbslHdmiTxCallback_t)0,
        (ptmbslHdmiTxCallback_t)0,
        (ptmbslHdmiTxCallback_t)0,
        (ptmbslHdmiTxCallback_t)0,
        (ptmbslHdmiTxCallback_t)0,
        (ptmbslHdmiTxCallback_t)0,
        (ptmbslHdmiTxCallback_t)0,
        (ptmbslHdmiTxCallback_t)0,
        (ptmbslHdmiTxCallback_t)0,
        (ptmbslHdmiTxCallback_t)0
    },
    0,                                  /* InterruptsEnable */
    {                                   /* uSupportedVersions[] */
        E_DEV_VERSION_N1,
        E_DEV_VERSION_LIST_END
    },
    E_DEV_VERSION_LIST_END,             /* uDeviceVersion */
    E_DEV_VERSION_LIST_END,             /* uDeviceFeatures */
    tmPowerOn,                         /* ePowerState */
    HDMITX_SINK_DVI,                    /* sinkType */
    HDMITX_SINK_DVI,                    /* EdidSinkType */
    False,                              /* EdidSinkAi */
    0,                                  /* EdidCeaFlags */
    0,                                  /* EdidCeaXVYCCFlags */
    {
     False,                              /* latency_available */
     False,                              /* Ilatency_available */
     0,                                  /* Edidvideo_latency */
     0,                                  /* Edidaudio_latency */
     0,                                  /* EdidIvideo_latency */
     0},                                  /* EdidIaudio_latency */

    {
        0,                                          /* maximum supported TMDS clock */
        0,                                          /* content type Graphics (text) */
        0,                                          /* content type Photo */
        0,                                          /* content type Cinema */
        0,                                          /* content type Game */
        0,                                          /* additional video format */
        0,                                          /* 3D support by the HDMI Sink */
        0,                                          /* 3D multi strctures present */
        0,                                          /* additional info for the values in the image size area */
        0,                                          /* total length of 3D video formats */
        0,                                          /* total length of extended video formats */
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0} /* max_len-10, ie: 31-10=21 */
    },
    
    HDMITX_EDID_NOT_READ,               /* EdidStatus */
    0,                                  /* NbDTDStored */
    {                                   /* EdidDTD: */ /* * NUMBER_DTD_STORED */
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*1 */
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*2 */  
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*3 */
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*4 */
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*5 */
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*6 */
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*7 */
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*8 */
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*9 */
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0}  /*10*/
    },
    {                               /* EdidMonitorDescriptor */
        False,                      /* bDescRecord */
        {0,0,0,0,0,0,0,0,0,0,0,0,0} /* uMonitorName[EDID_MONITOR_DESCRIPTOR_SIZE]       */
    },  
    {
        False,                      /* bDescRecord */
        0,                          /* uMinVerticalRate                                 */
        0,                          /* uMaxVerticalRate                                 */
        0,                          /* uMinHorizontalRate                               */
        0,                          /* uMaxHorizontalRate                               */
        0                           /* uMaxSupportedPixelClk                            */
    },
    {
        False,                      /* bDescRecord */
        {0,0,0,0,0,0,0,0,0,0,0,0,0} /* uOtherDescriptor[EDID_MONITOR_DESCRIPTOR_SIZE]   */
    },
    {                                   /* EdidVFmts[] */
        HDMITX_VFMT_NULL
    },
    0,                                  /* EdidSvdCnt */
    {                                   /* EdidAFmts[]. */
        {0,0,0},                             /* {ModeChans, Freqs, Byte3} */
        {0,0,0},                             /* {ModeChans, Freqs, Byte3} */
        {0,0,0},                             /* {ModeChans, Freqs, Byte3} */
        {0,0,0},                             /* {ModeChans, Freqs, Byte3} */
        {0,0,0},                             /* {ModeChans, Freqs, Byte3} */
        {0,0,0},                             /* {ModeChans, Freqs, Byte3} */
        {0,0,0},                             /* {ModeChans, Freqs, Byte3} */
        {0,0,0},                             /* {ModeChans, Freqs, Byte3} */
        {0,0,0},                             /* {ModeChans, Freqs, Byte3} */
        {0,0,0}                             /* {ModeChans, Freqs, Byte3} */
    },
    0,                                  /* EdidSadCnt */
    {
        0                               /* EdidBlock[ ] */
    },
    0,                                  /* EdidBlockCnt */
    0,                                  /* EdidSourceAddress */
    0,                                  /* EdidBlockRequested */
    False,                              /* EdidReadStarted */
    {                                   /* EdidToApp */
        0,                              /* pRawEdid */
        0                               /* numBlocks */
    },
    {                                   /* EDIDBasicDisplayParam */
        0,                              /* uVideoInputDef */
        0,                              /* uMaxHorizontalSize */
        0,                              /* uMaxVerticalSize */
        0,                              /* uGamma */
        0,                              /* uFeatureSupport */
    },
#ifndef NO_HDCP
    False,                              /* HDCPIgnoreEncrypt */
    0,                                  /* HdcpPortAddress */
    HDMITX_HDCP_TXMODE_NOT_SET,         /* HdcpTxMode */
    HDMITX_HDCP_OPTION_DEFAULT,         /* HdcpOptions */
    0,                                  /* HdcpBcaps */
    0,                                  /* HdcpBstatus */
    0,                                  /* HdcpRi */
    0,                                  /* HdcpFsmState */
    0,                                  /* HdcpT0FailState */
    0,                                  /* HdcpSeed */
    {0, 0, 0, 0, 0},                    /* HdcpAksv */
    (ptmHdmiTxFunc_t)0,                 /* HdcpFuncScheduled */
    0,                                  /* HdcpFuncRemainingMs */
    0,                                  /* HdcpCheckIntervalMs */
    0,                                  /* HdcpCheckRemainingMs */
    0,                                  /* HdcpCheckNum */
    0,                                  /* HdcpChecksToDo */
#endif /* NO_HDCP */
    HDMITX_VFMT_NULL,                   /* vinFmt */
    HDMITX_VFMT_NULL,                   /* voutFmt */
    HDMITX_PIXRATE_DOUBLE,              /* pixRate */
    HDMITX_VINMODE_RGB444,              /* vinMode */
    HDMITX_VOUTMODE_RGB444,             /* voutMode */
    HDMITX_VFREQ_INVALID,               /* voutFreq */
    HDMITX_SCAMODE_OFF,                 /* scaMode */
    HDMITX_UPSAMPLE_AUTO,               /* upsampleMode */
    HDMITX_PIXREP_MIN,                  /* pixelRepeatCount */
    HDMITX_HOTPLUG_INVALID,             /* hotPlugStatus */
    HDMITX_RX_SENSE_INVALID,            /* rxSenseStatus */
    E_PAGE_INVALID,                     /* curRegPage */
    {
        /* These match power-up defaults.  shadowReg[]: */
        0x00,                           /* E_SP00_MAIN_CNTRL0 */
        0x00,                           /* E_SP00_INT_FLAGS_0 */
        0x00,                           /* E_SP00_INT_FLAGS_1 */
        0x00,                           /* E_SP00_INT_FLAGS_2 */
        0x01,                           /* E_SP00_VIP_CNTRL_0 */
        0x24,                           /* E_SP00_VIP_CNTRL_1 */
        0x56,                           /* E_SP00_VIP_CNTRL_2 */
        0x17,                           /* E_SP00_VIP_CNTRL_3 */
        0x01,                           /* E_SP00_VIP_CNTRL_4 */
        0x00,                           /* E_SP00_VIP_CNTRL_5 */
        0x05,                           /* E_SP00_MAT_CONTRL  */
        0x00,                           /* E_SP00_TBG_CNTRL_0 */
        0x00,                           /* E_SP00_TBG_CNTRL_1 */
        0x00,                           /* E_SP00_HVF_CNTRL_0 */
        0x00,                           /* E_SP00_HVF_CNTRL_1 */
        0x00,                           /* E_SP00_TIMER_H     */
        0x00,                           /* E_SP00_DEBUG_PROBE */
        0x00                            /* E_SP00_AIP_CLKSEL  */
        ,0x00                           /* E_SP01_SC_VIDFORMAT*/
        ,0x00                           /* E_SP01_SC_CNTRL    */
        ,0x00                           /* E_SP01_TBG_CNTRL_0 */
#ifndef NO_HDCP
        ,0x00                           /* E_SP12_HDCP_CTRL   */
        ,0x00                           /* E_SP12_HDCP_BCAPS  */
#endif /* NO_HDCP */
    },
    False,                              /* Init prevFilterPattern to false */
    False,                              /* Init prevPattern to false */
    False                              /* bInitialized */
};


/**
 * Table of shadow registers, as packed Shad/Page/Addr codes.
 * This allows shadow index values to be searched for using register page
 * and address values.
 */
static CONST_DAT UInt16 kShadowReg[E_SNUM] =
{/* Shadow Index                  Packed Shad/Page/Addr */
    E_REG_P00_MAIN_CNTRL0_W ,    /* E_SP00_MAIN_CNTRL0  */
    E_REG_P00_INT_FLAGS_0_RW,    /* E_SP00_INT_FLAGS_0  */
    E_REG_P00_INT_FLAGS_1_RW,    /* E_SP00_INT_FLAGS_1  */
    E_REG_P00_INT_FLAGS_2_RW,    /* E_SP00_INT_FLAGS_2  */
    E_REG_P00_VIP_CNTRL_0_W ,    /* E_SP00_VIP_CNTRL_0  */
    E_REG_P00_VIP_CNTRL_1_W ,    /* E_SP00_VIP_CNTRL_1  */
    E_REG_P00_VIP_CNTRL_2_W ,    /* E_SP00_VIP_CNTRL_2  */
    E_REG_P00_VIP_CNTRL_3_W ,    /* E_SP00_VIP_CNTRL_3  */
    E_REG_P00_VIP_CNTRL_4_W ,    /* E_SP00_VIP_CNTRL_4  */
    E_REG_P00_VIP_CNTRL_5_W ,    /* E_SP00_VIP_CNTRL_5  */
    E_REG_P00_MAT_CONTRL_W  ,    /* E_SP00_MAT_CONTRL   */
    E_REG_P00_TBG_CNTRL_0_W ,    /* E_SP00_TBG_CNTRL_0  */
    E_REG_P00_TBG_CNTRL_1_W ,    /* E_SP00_TBG_CNTRL_1  */
    E_REG_P00_HVF_CNTRL_0_W ,    /* E_SP00_HVF_CNTRL_0  */
    E_REG_P00_HVF_CNTRL_1_W ,    /* E_SP00_HVF_CNTRL_1  */
    E_REG_P00_TIMER_H_W     ,    /* E_SP00_TIMER_H      */
    E_REG_P00_DEBUG_PROBE_W ,    /* E_SP00_DEBUG_PROBE  */
    E_REG_P00_AIP_CLKSEL_W,      /* E_SP00_AIP_CLKSEL   */
    E_REG_P01_SC_VIDFORMAT_W,    /* E_SP01_SC_VIDFORMAT */
    E_REG_P01_SC_CNTRL_W,        /* E_SP01_SC_CNTRL     */
    E_REG_P01_TBG_CNTRL_0_W      /* E_SP01_TBG_CNTRL_0  */
#ifndef NO_HDCP
    ,E_REG_P12_HDCP_CTRL_W       /* E_SP12_HDCP_CTRL    */
    ,E_REG_P12_HDCP_BCAPS_W      /* E_SP12_HDCP_BCAPS   */
#endif /* NO_HDCP */
};


/**
 * Table of registers to switch to low power (standby)
 */
static CONST_DAT tmHdmiTxRegMaskVal_t kPowerOff[] =
{
    {E_REG_P02_TEST2_RW,        E_MASKREG_P02_TEST2_pwd1v8,         1},
    {E_REG_P02_PLL_SCG1_RW,     E_MASKREG_P02_PLL_SCG1_scg_fdn,     1},
    {E_REG_P02_PLL_SERIAL_1_RW, E_MASKREG_P02_PLL_SERIAL_1_srl_fdn, 1},
    {E_REG_P02_PLL_DE_RW,       E_MASKREG_P02_PLL_DE_pllde_fdn,     1},
    {E_REG_P02_BUFFER_OUT_RW,   E_MASKREG_P02_BUFFER_OUT_srl_force, 2},
    {E_REG_P02_SEL_CLK_RW,      E_MASKREG_P02_SEL_CLK_ena_sc_clk,   0},
    {E_REG_P00_CCLK_ON_RW,      E_MASKREG_P00_CCLK_ON_cclk_on,      0},
    {0,0,0}
};

/**
 * Table of registers to switch to normal power (resume)
 */
static CONST_DAT tmHdmiTxRegMaskVal_t kPowerOn[] =
{
    {E_REG_P02_TEST2_RW,        E_MASKREG_P02_TEST2_pwd1v8,         0},
    {E_REG_P02_PLL_SERIAL_1_RW, E_MASKREG_P02_PLL_SERIAL_1_srl_fdn, 0},
    {E_REG_P02_PLL_DE_RW,       E_MASKREG_P02_PLL_DE_pllde_fdn,     0},
    {E_REG_P02_PLL_SCG1_RW,     E_MASKREG_P02_PLL_SCG1_scg_fdn,     0},
    {E_REG_P02_SEL_CLK_RW,      E_MASKREG_P02_SEL_CLK_ena_sc_clk,   1},
    {E_REG_P02_BUFFER_OUT_RW,   E_MASKREG_P02_BUFFER_OUT_srl_force, 0},
    {E_REG_P00_TBG_CNTRL_0_W,   E_MASKREG_P00_TBG_CNTRL_0_sync_once,0},
    {E_REG_P00_CCLK_ON_RW,      E_MASKREG_P00_CCLK_ON_cclk_on,      1},
    {0,0,0}
};

static CONST_DAT tmbslHdmiTxCallbackInt_t kITCallbackPriority[HDMITX_CALLBACK_INT_NUM] =
{
    HDMITX_CALLBACK_INT_R0,             /**< R0 interrupt                     */
    HDMITX_CALLBACK_INT_ENCRYPT,        /**< HDCP encryption switched off     */
    HDMITX_CALLBACK_INT_HPD,            /**< Transition on HPD input          */
    HDMITX_CALLBACK_INT_T0,             /**< HDCP state machine in state T0   */
    HDMITX_CALLBACK_INT_BCAPS,          /**< BCAPS available                  */
    HDMITX_CALLBACK_INT_BSTATUS,        /**< BSTATUS available                */
    HDMITX_CALLBACK_INT_SHA_1,          /**< sha-1(ksv,bstatus,m0)=V'         */
    HDMITX_CALLBACK_INT_PJ,             /**< pj=pj' check fails               */
    HDMITX_CALLBACK_INT_SW_INT,         /**< SW DEBUG interrupt               */
    HDMITX_CALLBACK_INT_RX_SENSE,       /**< RX SENSE interrupt               */
    HDMITX_CALLBACK_INT_EDID_BLK_READ,  /**< EDID BLK READ interrupt          */
    HDMITX_CALLBACK_INT_VS_RPT,         /**< VS interrupt                     */
    HDMITX_CALLBACK_INT_PLL_LOCK        /** PLL LOCK not present on TDA9984   */
};


/*============================================================================*/
/*                       DEFINES DECLARATIONS                               */
/*============================================================================*/


/*============================================================================*/
/*                       VARIABLES DECLARATIONS                               */
/*============================================================================*/

static Bool gMiscInterruptHpdRxEnable = False; /* Enable HPD and RX sense IT after */
                                               /* first call done by init function */

/*============================================================================*/
/*                       FUNCTION PROTOTYPES                                  */
/*============================================================================*/

/*============================================================================*/
/* tmbslTDA9984Deinit                                                          */
/*============================================================================*/
tmErrorCode_t
tmbslTDA9984Deinit
(
    tmUnitSelect_t txUnit
)
{
    tmHdmiTxobject_t *pDis;     /* Pointer to Device Instance Structure */
    tmErrorCode_t     err;      /* Error code */

    /* Check unit parameter and point to its object */
    err = checkUnitSetDis(txUnit, &pDis);
    RETIF(err != TM_OK, err)

    /* Hold the device in reset to disable it */
    (void)setHwRegisterField(pDis, E_REG_P00_MAIN_CNTRL0_W,
                             E_MASKREG_P00_MAIN_CNTRL0_sr, 1);

    /* Clear the Initialized flag to destroy the device instance */
    pDis->bInitialized = False;

    setState(pDis, EV_DEINIT);
    return TM_OK;
}

/*============================================================================*/
/* tmbslTDA9984HotPlugGetStatus                                                */
/*============================================================================*/
tmErrorCode_t
tmbslTDA9984HotPlugGetStatus
(
    tmUnitSelect_t        txUnit,
    tmbslHdmiTxHotPlug_t *pHotPlugStatus,
    Bool                  client    /* not used */
)
{
    tmHdmiTxobject_t *pDis;     /* Pointer to Device Instance Structure */
    tmErrorCode_t     err;      /* Error code */

    /* Check unit parameter and point to its object */
    err = checkUnitSetDis(txUnit, &pDis);
    RETIF(err != TM_OK, err)

    /* Check remaining parameters */
    RETIF_BADPARAM(pHotPlugStatus == (tmbslHdmiTxHotPlug_t *)0)

    /* Read the hot plug status flag register last read at interrupt */
    *pHotPlugStatus = pDis->hotPlugStatus;

    return TM_OK;
}

/*============================================================================*/
/* tmbslTDA9984RxSenseGetStatus                                                */
/*============================================================================*/
tmErrorCode_t
tmbslTDA9984RxSenseGetStatus
(
    tmUnitSelect_t        txUnit,
    tmbslHdmiTxRxSense_t *pRxSenseStatus,
    Bool                  client    /* not used */
)
{
    tmHdmiTxobject_t *pDis;     /* Pointer to Device Instance Structure */
    tmErrorCode_t     err;      /* Error code */

    /* Check unit parameter and point to its object */
    err = checkUnitSetDis(txUnit, &pDis);
    RETIF(err != TM_OK, err)

    /* Check remaining parameters */
    RETIF_BADPARAM(pRxSenseStatus == (tmbslHdmiTxRxSense_t *)0)

    /* Read the Rx sense status flag register last read at interrupt */
    *pRxSenseStatus = pDis->rxSenseStatus;

    return TM_OK;
}

/*============================================================================*/
/* tmbslTDA9984HwGetRegisters                                                  */
/*============================================================================*/
tmErrorCode_t
tmbslTDA9984HwGetRegisters
(
    tmUnitSelect_t  txUnit,
    Int             regPage,
    Int             regAddr,
    UInt8          *pRegBuf,
    Int             nRegs
)
{
    tmHdmiTxobject_t *pDis;         /* Pointer to Device Instance Structure */
    tmErrorCode_t   err;            /* Error code */
    Int             i;              /* Loop index */
    UInt8           newRegPage;     /* The register's new page number */
    UInt8           regShad;        /* Index to the register's shadow copy */
    UInt16          regShadPageAddr;/* Packed shadowindex/page/address */
    tmbslHdmiTxSysArgs_t sysArgs;   /* Arguments passed to system function     */

    /* Check unit parameter and point to its object */
    err = checkUnitSetDis(txUnit, &pDis);
    RETIF(err != TM_OK, err)

    /* Check remaining parameters */
    RETIF_BADPARAM((regPage < kPageIndexToPage[E_PAGE_00]) 
      || ((regPage > kPageIndexToPage[E_PAGE_02]) 
       && (regPage < kPageIndexToPage[E_PAGE_09]))
      || ((regPage > kPageIndexToPage[E_PAGE_09]) 
       && (regPage < kPageIndexToPage[E_PAGE_11]))
      ||  (regPage > kPageIndexToPage[E_PAGE_12]))
    RETIF_BADPARAM((regAddr < E_REG_MIN_ADR) || (regAddr >= E_REG_CURPAGE_ADR_W))
    RETIF_BADPARAM(pRegBuf == (pUInt8)0)
    RETIF_BADPARAM((nRegs < 1) || ((nRegs + regAddr) > E_REG_CURPAGE_ADR_W))

    /* Set page register if required */
    newRegPage = (UInt8)regPage;
    if (pDis->curRegPage != newRegPage)
    {
        /* All non-OK results are errors */
        sysArgs.slaveAddr       = pDis->uHwAddress;
        sysArgs.firstRegister   = E_REG_CURPAGE_ADR_W;
        sysArgs.lenData         = 1;
        sysArgs.pData           = &newRegPage;
        err = pDis->sysFuncWrite(&sysArgs);
        RETIF(err != TM_OK, TMBSL_ERR_HDMI_I2C_WRITE)
        pDis->curRegPage = newRegPage;
    }

    /* Read each register in the range. nRegs must start at 1 or more */
    for ( ; nRegs > 0; pRegBuf++, regAddr++, nRegs--)
    {
        /* Find shadow register index.
         * This loop is not very efficient, but it is assumed that this API
         * will not be used often. The alternative is to use a huge sparse
         * array indexed by page and address and containing the shadow index.
         */
        regShad = E_SNONE;
        for (i = 0; i < E_SNUM; i++)
        {
            /* Check lookup table for match with page and address */
            regShadPageAddr = kShadowReg[i];
            if ((SPA2PAGE(regShadPageAddr) == newRegPage)
            &&  (SPA2ADDR(regShadPageAddr) == regAddr))
            {
                /* Found page and address - look up the shadow index */
                regShad = SPA2SHAD(regShadPageAddr);
                break;
            }
        }
        /* Read the shadow register if available, as device registers that
         * are shadowed cannot be read directly */
        if (regShad != E_SNONE)
        {
            *pRegBuf = pDis->shadowReg[regShad];
        }
        else
        {
            /* Read the device register - all non-OK results are errors.
             * Note that some non-shadowed registers are also write-only and
             * cannot be read. */
            sysArgs.slaveAddr       = pDis->uHwAddress;
            sysArgs.firstRegister   = (UInt8)regAddr;
            sysArgs.lenData         = 1;
            sysArgs.pData           = pRegBuf;
            err = pDis->sysFuncRead(&sysArgs);
            RETIF(err != TM_OK, TMBSL_ERR_HDMI_I2C_READ)
        }
    }

    return TM_OK;
}

/*============================================================================*/
/* tmbslTDA9984HwGetVersion                                                    */
/*============================================================================*/
#ifndef DEMO_BUILD
tmErrorCode_t
tmbslTDA9984HwGetVersion
(
    tmUnitSelect_t  txUnit,
    pUInt8          pHwVersion
)
{
    tmHdmiTxobject_t *pDis;       /* Pointer to Device Instance Structure */
    tmErrorCode_t   err;            /* Error code */

    /* Check unit parameter and point to its object */
    err = checkUnitSetDis(txUnit, &pDis);
    RETIF(err != TM_OK, err)

    /* Check remaining parameters */
    RETIF_BADPARAM(pHwVersion == (pUInt8)0)


    *pHwVersion = BSLHDMITX_TDA9984;

    return TM_OK;
}
#endif /* DEMO_BUILD */


/*============================================================================*/
/* tmbslTDA9984HwHandleInterrupt                                               */
/* RETIF_REG_FAIL NOT USED HERE AS ALL ERRORS SHOULD BE TRAPPED IN ALL BUILDS */
/*============================================================================*/
tmErrorCode_t
tmbslTDA9984HwHandleInterrupt
(
    tmUnitSelect_t      txUnit
)
{
    tmHdmiTxobject_t        *pDis;             /* Pointer to Device Instance Structure */
    tmErrorCode_t           err;              /* Error code */
#ifndef NO_HDCP
    UInt8                   regVal;           /* Register value */
#endif
    UInt16                  fInterruptStatus; /* Interrupt flags */
    UInt16                  fInterruptMask;   /* Mask to test each interrupt bit */
    tmbslHdmiTxRxSense_t    newRxs_fil;       /* Latest copy of rx_sense */
    Int                     i;                /* Loop counter */
    tmbslHdmiTxHotPlug_t    newHpdIn;      /* Latest copy of hpd input */
    Bool                    sendEdidCallback;

    UInt8                   tabIntFlagsVal[4];       

    /* Check unit parameter and point to its object */
    err = checkUnitSetDis(txUnit, &pDis);
    RETIF(err != TM_OK, err)

    fInterruptStatus = 0;
    sendEdidCallback = False;

    /* Read the main interrupt flags register to determine the source(s) 
     * of the interrupt. (The device resets these register flags after they 
     * have been read.)
     */
    err = getHwRegisters(pDis, E_REG_P00_INT_FLAGS_0_RW, tabIntFlagsVal, 4);
    RETIF(err != TM_OK, err)

    if ((tabIntFlagsVal[0] & E_MASKREG_P00_INT_FLAGS_0_encrypt) != 0)
    {
        fInterruptStatus = fInterruptStatus | (1 << HDMITX_CALLBACK_INT_ENCRYPT); 
    }

    if ((tabIntFlagsVal[0] & E_MASKREG_P00_INT_FLAGS_0_hpd) != 0)
    {
        fInterruptStatus = fInterruptStatus | (1 << HDMITX_CALLBACK_INT_HPD); 
    }

    if ((tabIntFlagsVal[0] & E_MASKREG_P00_INT_FLAGS_0_t0) != 0)
    {
        fInterruptStatus = fInterruptStatus | (1 << HDMITX_CALLBACK_INT_T0); 
    }

    if ((tabIntFlagsVal[0] & E_MASKREG_P00_INT_FLAGS_0_bcaps) != 0)
    {
        fInterruptStatus = fInterruptStatus | (1 << HDMITX_CALLBACK_INT_BCAPS); 
    }

    if ((tabIntFlagsVal[0] & E_MASKREG_P00_INT_FLAGS_0_bstatus) != 0)
    {
        fInterruptStatus = fInterruptStatus | (1 << HDMITX_CALLBACK_INT_BSTATUS); 
    }

    if ((tabIntFlagsVal[0] & E_MASKREG_P00_INT_FLAGS_0_sha_1) != 0)
    {
        fInterruptStatus = fInterruptStatus | (1 << HDMITX_CALLBACK_INT_SHA_1); 
    }

    if ((tabIntFlagsVal[0] & E_MASKREG_P00_INT_FLAGS_0_pj) != 0)
    {
        fInterruptStatus = fInterruptStatus | (1 << HDMITX_CALLBACK_INT_PJ); 
    }

    if ((tabIntFlagsVal[0] & E_MASKREG_P00_INT_FLAGS_0_r0) != 0)
    {
        fInterruptStatus = fInterruptStatus | (1 << HDMITX_CALLBACK_INT_R0); 
    }

    /* Read Hot Plug input status to unit object 
       DJW do this after reading INT flags so we get the actual
       level that caused the interrupt */
    //err = getHwRegister(pDis, E_REG_P00_INT_FLAGS_1_RW, &regVal);
    //RETIF(err != TM_OK, err)
    newHpdIn = (tabIntFlagsVal[1] & E_MASKREG_P00_INT_FLAGS_1_hpd_in) ?
                HDMITX_HOTPLUG_ACTIVE : HDMITX_HOTPLUG_INACTIVE;

    /* Read the software interrupt flag */
    if ((tabIntFlagsVal[1] & E_MASKREG_P00_INT_FLAGS_1_sw_int) != 0)
    {
        fInterruptStatus = fInterruptStatus | (1 << HDMITX_CALLBACK_INT_SW_INT); 
    }

    /* Read the software interrupt flag */
    if (((pDis->InterruptsEnable & E_MASKREG_P00_INT_FLAGS_1_vs_rpt) != 0) && 
        ((tabIntFlagsVal[1] & E_MASKREG_P00_INT_FLAGS_1_vs_rpt) != 0)
       )
    {
        fInterruptStatus = fInterruptStatus | (1 << HDMITX_CALLBACK_INT_VS_RPT); 
    }

    /* Read INT_FLAGS_2 interrupt flag register. 
     *(The device resets these register flags after they 
     * have been read.) */
    //err = getHwRegister(pDis, E_REG_P00_INT_FLAGS_2_RW, &regVal);
    //RETIF(err != TM_OK, err)

    /* Has the rx_sense interrupt occurs */
    if ((tabIntFlagsVal[2] & E_MASKREG_P00_INT_FLAGS_2_rx_sense) != 0)
    {
        fInterruptStatus = fInterruptStatus | (1 << HDMITX_CALLBACK_INT_RX_SENSE); 
    }

    /* Has the EDID_blk_rd interrupt occurs */
    if ((tabIntFlagsVal[2] & E_MASKREG_P00_INT_FLAGS_2_edid_blk_rd) != 0)
    {
        fInterruptStatus = fInterruptStatus | (1 << HDMITX_CALLBACK_INT_EDID_BLK_READ);
    }

    if (newHpdIn == pDis->hotPlugStatus)
    {
        if (fInterruptStatus & (1 << HDMITX_CALLBACK_INT_HPD))
        {
            /* Short HPD */
            /* A short HPD has been missed send ev unplugged  */
            if (pDis->hotPlugStatus == HDMITX_HOTPLUG_ACTIVE)
            {
                /* Reset EDID status */
                err = ClearEdidRequest(txUnit);
                pDis->hotPlugStatus = HDMITX_HOTPLUG_INACTIVE; 
                setState(pDis, EV_UNPLUGGED);
                pDis->funcIntCallbacks[HDMITX_CALLBACK_INT_HPD](txUnit);
            }
        }
        else
        {
            if (fInterruptStatus & (1 << HDMITX_CALLBACK_INT_EDID_BLK_READ))
            {
                err = EdidBlockAvailable(txUnit,&sendEdidCallback);
                RETIF(err != TM_OK, err)
                if (sendEdidCallback == False)
                {
                    /* Read EDID not finished clear callback */
                    fInterruptStatus = (UInt16) (fInterruptStatus & (~(1 << HDMITX_CALLBACK_INT_EDID_BLK_READ)));
                }
            }
        }

    }

    if (newHpdIn != pDis->hotPlugStatus)
    {
        /* Callback disable on first tmbslTDA9984HwHandleInterrupt call */
        if(gMiscInterruptHpdRxEnable)
        {
            /* Yes: save new HPD level */
            pDis->hotPlugStatus = newHpdIn;

            /* Reset EDID status */
            err = ClearEdidRequest(txUnit);

            /* Set HPD flag to 1 although it certainly already done just a security */
            fInterruptStatus |= (1 << HDMITX_CALLBACK_INT_HPD);
            /* Reset all simultaneous HDCP interrupts on hot plug,
            * preserving only the high-priority hpd interrupt rx_sense and sw interrupt for debug*/
            fInterruptStatus &= (1 << HDMITX_CALLBACK_INT_HPD) |
                                (1 << HDMITX_CALLBACK_INT_RX_SENSE) |
                                (1 << HDMITX_CALLBACK_INT_SW_INT);

            if (pDis->hotPlugStatus == HDMITX_HOTPLUG_ACTIVE)
            {
                /* EDID workaround (PR778) :Soft reset required to be able to read EDID */
                err = tmbslTDA9984Reset(txUnit);
                RETIF(err != TM_OK, err)
                /* EDID workaround (PR778) : Restore chip after the soft reset */
				err = hotPlugRestore(txUnit);
                RETIF(err != TM_OK, err)

                setState(pDis, EV_PLUGGEDIN);
            }
            else
            {
                setState(pDis, EV_UNPLUGGED);
            }
        }
    }

    /* Read INT_FLAGS_3 interrupt flag register. */
    //err = getHwRegister(pDis, E_REG_P00_INT_FLAGS_3_R, &regVal);
    //RETIF(err != TM_OK, err)

    /*  Read RXS_FIL status to know the actual
        level that caused the interrupt */
    newRxs_fil = (tabIntFlagsVal[3] & E_MASKREG_P00_INT_FLAGS_3_rxs_fil) ?
                 HDMITX_RX_SENSE_ACTIVE : HDMITX_RX_SENSE_INACTIVE;

    /* Has the Rxs Fil changed? (Ignore the RxSense interrupt status flag in 
     * fInterruptStatus as this may have latched a on/off glitch) */
    if (newRxs_fil != pDis->rxSenseStatus)
    {
        /* Callback disable on first tmbslTDA9984HwHandleInterrupt call */
        if(gMiscInterruptHpdRxEnable)
        {
            /* Yes: save new rxSense level */
            pDis->rxSenseStatus = newRxs_fil;

            /* Set RX_Sense flag to 1 although it certainly already done just a security */
            fInterruptStatus |= ( 1 << HDMITX_CALLBACK_INT_RX_SENSE);

            fInterruptStatus &= (1 << HDMITX_CALLBACK_INT_HPD) |
                                (1 << HDMITX_CALLBACK_INT_RX_SENSE) |
                                (1 << HDMITX_CALLBACK_INT_SW_INT);

            if (pDis->rxSenseStatus == HDMITX_RX_SENSE_ACTIVE)
            {
                setState(pDis, EV_SINKON);
            }
            else
            {
                setState(pDis, EV_SINKOFF);
            }
        }
    }
    else
    {
        /* Clear RX_sense IT if level has not changed */
        fInterruptStatus = (UInt16) (fInterruptStatus & (~(1 << HDMITX_CALLBACK_INT_RX_SENSE)));
    }

    /* Ignore other simultaneous HDCP interrupts if T0 interrupt,
     * preserving any hpd interrupt */
    if (fInterruptStatus & (1 << HDMITX_CALLBACK_INT_T0))
    {
        if (pDis->EdidReadStarted)
        {
#ifndef NO_HDCP
			err = getHwRegister(pDis, E_REG_P12_HDCP_TX0_RW, &regVal);
			RETIF(err != TM_OK, err)
			/* EDID read failure */
			if ((regVal & E_MASKREG_P12_HDCP_TX0_sr_hdcp) != 0)
			{
#endif
            /* Reset EDID status */
            err = ClearEdidRequest(txUnit);
				RETIF(err != TM_OK, err)
				/* enable EDID callback */
				fInterruptStatus = (UInt16) (fInterruptStatus & (~(1 << HDMITX_CALLBACK_INT_T0)));
				fInterruptStatus = fInterruptStatus | (1 << HDMITX_CALLBACK_INT_EDID_BLK_READ);
#ifndef NO_HDCP
        	}
#endif
        }
        else
        {
            fInterruptStatus &= 
                (
                (1 << HDMITX_CALLBACK_INT_HPD) |
                (1 << HDMITX_CALLBACK_INT_T0)
                |(1 << HDMITX_CALLBACK_INT_RX_SENSE)
                |(1 << HDMITX_CALLBACK_INT_SW_INT)
                );
        }
    }
#ifndef NO_HDCP
    if (fInterruptStatus & (1 << HDMITX_CALLBACK_INT_R0))
    {
        err = tmbslTDA9984handleBKSVResultSecure(txUnit);
        RETIF(err != TM_OK, err)
    }
#endif /* NO_HDCP */
    /* For each interrupt flag that is set, check the corresponding registered 
     * callback function pointer in the Device Instance Structure 
     * funcIntCallbacks array.
     */

    for (i = 0; i < HDMITX_CALLBACK_INT_NUM; i++)
    {
        if ( i != HDMITX_CALLBACK_INT_PLL_LOCK) /* PLL LOCK not present on TDA9984 */
        {
            fInterruptMask = 1;
            fInterruptMask  = fInterruptMask << ((UInt16)kITCallbackPriority[i]);
    
            if (fInterruptStatus & fInterruptMask)
            {
                /* IF a registered callback pointer is non-null THEN call it. */
                if (pDis->funcIntCallbacks[kITCallbackPriority[i]] != (ptmbslHdmiTxCallback_t)0)
                {
                    pDis->funcIntCallbacks[kITCallbackPriority[i]](txUnit);
                }
            }
        }
    }

    return TM_OK;
}

/*============================================================================*/
/* tmbslTDA9984FlagSwInt                                                       */
/* Use only for debug to flag the software debug interrupt                    */
/*============================================================================*/
tmErrorCode_t
tmbslTDA9984FlagSwInt
(
    tmUnitSelect_t              txUnit,
    UInt32                      uSwInt
)
{
    tmHdmiTxobject_t *pDis;     /* Pointer to Device Instance Structure */
    tmErrorCode_t     err;      /* Error code */

    /* Check unit parameter and point to its object */
    err = checkUnitSetDis(txUnit, &pDis);
    RETIF(err != TM_OK, err)

    err = setHwRegister(pDis, E_REG_P00_SW_INT_W, 
                        E_MASKREG_P00_SW_INT_sw_int);

    DUMMY_ACCESS(uSwInt);

    return err;
}

/*============================================================================*/
/* tmbslTDA9984HwSetRegisters                                                  */
/*============================================================================*/
#ifndef DEMO_BUILD
tmErrorCode_t
tmbslTDA9984HwSetRegisters 
(
    tmUnitSelect_t  txUnit,
    Int             regPage, 
    Int             regAddr, 
    UInt8          *pRegBuf, 
    Int             nRegs
)
{
    tmHdmiTxobject_t *pDis;         /* Pointer to Device Instance Structure */
    tmErrorCode_t   err;            /* Error code */
    Int             i;              /* Loop index */
    UInt8           newRegPage;     /* The register's new page number */
    UInt8           regShad;        /* Index to the register's shadow copy */
    UInt16          regShadPageAddr;/* Packed shadowindex/page/address */
    tmbslHdmiTxSysArgs_t sysArgs;   /* Arguments passed to system function */

    /* Check unit parameter and point to its object */
    err = checkUnitSetDis(txUnit, &pDis);
    RETIF(err != TM_OK, err)

    /* Check remaining parameters */
    RETIF_BADPARAM((regPage < kPageIndexToPage[E_PAGE_00])
      || ((regPage > kPageIndexToPage[E_PAGE_02])
       && (regPage < kPageIndexToPage[E_PAGE_11]))
      ||  (regPage > kPageIndexToPage[E_PAGE_12]))
    RETIF_BADPARAM((regAddr < E_REG_MIN_ADR) || (regAddr >= E_REG_CURPAGE_ADR_W))
    RETIF_BADPARAM(pRegBuf == (pUInt8)0)
    RETIF_BADPARAM((nRegs < 0) || ((nRegs + regAddr) > E_REG_CURPAGE_ADR_W))

    /* Set page register if required */
    newRegPage = (UInt8)regPage;
    if (pDis->curRegPage != newRegPage)
    {
        /* All non-OK results are errors */
        sysArgs.slaveAddr       = pDis->uHwAddress;
        sysArgs.firstRegister   = E_REG_CURPAGE_ADR_W;
        sysArgs.lenData         = 1;
        sysArgs.pData           = &newRegPage;
        err = pDis->sysFuncWrite(&sysArgs);
        RETIF(err != TM_OK, TMBSL_ERR_HDMI_I2C_WRITE)
        pDis->curRegPage = newRegPage;
    }

    /* Write each register in the range. nRegs = 0 is ok, to allow only
     * the page register to be written if required (above)
     */
    for ( ; nRegs > 0; pRegBuf++, regAddr++, nRegs--)
    {
        /* Find shadow register index.
         * This loop is not very efficient, but it is assumed that this API
         * will not be used often. The alternative is to use a huge sparse
         * array indexed by page and address and containing the shadow index.
         */
        for (i = 0; i < E_SNUM; i++)
        {
            /* Check lookup table for match with page and address */
            regShadPageAddr = kShadowReg[i];
            if ((SPA2PAGE(regShadPageAddr) == newRegPage)
            &&  (SPA2ADDR(regShadPageAddr) == regAddr))
            {
                /* Found index - write the shadow register */
                regShad = SPA2SHAD(regShadPageAddr);
                pDis->shadowReg[regShad] = *pRegBuf;
                break;
            }
        }
        /* Write the device register - all non-OK results are errors */
        sysArgs.slaveAddr       = pDis->uHwAddress;
        sysArgs.firstRegister   = (UInt8)regAddr;
        sysArgs.lenData         = 1;
        sysArgs.pData           = pRegBuf;
        err = pDis->sysFuncWrite(&sysArgs);
        RETIF(err != TM_OK, TMBSL_ERR_HDMI_I2C_WRITE)
    }

    return TM_OK;
}
#endif /* DEMO_BUILD */

/*============================================================================*/
/* tmbslTDA9984HwStartup                                                       */
/*============================================================================*/
void
tmbslTDA9984HwStartup
(
    void
)
{
    /* Reset device instance data for when compiler doesn't do it */
    lmemset(&gHdmiTxInstance, 0, sizeof(gHdmiTxInstance));
}

/*============================================================================*/
/* tmbslTDA9984Init                                                            */
/* RETIF_REG_FAIL NOT USED HERE AS ALL ERRORS SHOULD BE TRAPPED IN ALL BUILDS */
/*============================================================================*/
tmErrorCode_t
tmbslTDA9984Init
(
    tmUnitSelect_t             txUnit,
    UInt8                      uHwAddress,
    ptmbslHdmiTxSysFunc_t      sysFuncWrite,
    ptmbslHdmiTxSysFunc_t      sysFuncRead,
    ptmbslHdmiTxSysFuncEdid_t  sysFuncEdidRead,
    ptmbslHdmiTxSysFuncTimer_t sysFuncTimer,
    tmbslHdmiTxCallbackList_t *funcIntCallbacks,
    Bool                       bEdidAltAddr,
    tmbslHdmiTxVidFmt_t        vinFmt,
    tmbslHdmiTxPixRate_t       pixRate
)
{
    tmHdmiTxobject_t *pDis;     /* Pointer to Device Instance Structure */
    tmErrorCode_t     err;      /* Error code */
    Int               i;        /* Loop index */
    Bool              bFound;   /* T=found, F=not found */

    DUMMY_ACCESS(bEdidAltAddr); /* Parameter not relevant */

    /* Check unit parameter and point to its object */
    RETIF(txUnit < tmUnit0, TMBSL_ERR_HDMI_BAD_UNIT_NUMBER) 
    RETIF(txUnit >= HDMITX_UNITS_MAX, TMBSL_ERR_HDMI_BAD_UNIT_NUMBER)
    pDis = &gHdmiTxInstance[txUnit];

    /* IF the bInitialized flag is set THEN return (only Init does this) */
    RETIF(pDis->bInitialized, TMBSL_ERR_HDMI_INIT_FAILED)

    /* Check remaining parameters */
    RETIF_BADPARAM(uHwAddress < HDMITX_SLAVE_ADDRESS_MIN)
    RETIF_BADPARAM(uHwAddress > HDMITX_SLAVE_ADDRESS_MAX)
    RETIF_BADPARAM(sysFuncWrite == (ptmbslHdmiTxSysFunc_t)0)
    RETIF_BADPARAM(sysFuncRead == (ptmbslHdmiTxSysFunc_t)0)
    RETIF_BADPARAM(sysFuncTimer == (ptmbslHdmiTxSysFuncTimer_t)0)
#ifdef FORMAT_PC
    RETIF_BADPARAM(vinFmt  < HDMITX_VFMT_TV_MIN)
    RETIF_BADPARAM((vinFmt > HDMITX_VFMT_TV_MAX) && (vinFmt < HDMITX_VFMT_PC_MIN))
    RETIF_BADPARAM(vinFmt  > HDMITX_VFMT_PC_MAX)
#else /* FORMAT_PC */
    /* FORMAT TV only */
    RETIF_BADPARAM(vinFmt  < HDMITX_VFMT_TV_MIN)
    RETIF_BADPARAM(vinFmt  > HDMITX_VFMT_TV_MAX)
#endif /* FORMAT_PC */
    RETIF_BADPARAM(pixRate >= HDMITX_PIXRATE_INVALID)

    /* Set all Device Instance Structure members to default values */
    lmemcpy(pDis, &kHdmiTxInstanceDefault, sizeof(*pDis));

    /* Copy txUnit, uHwAddress, sysFuncWrite and sysFuncRead values to
     * the defaulted Device Instance Structure BEFORE FIRST DEVICE ACCESS.
     */
    pDis->txUnit          = txUnit;
#ifdef UNIT_TEST
    /* Unit test build can't support 127 device sets of dummy registers, so use
     * smaller range instead, indexed by unit number not I2C address */
    pDis->uHwAddress      = (UInt8)txUnit;
#else
    /* Store actual I2C address */
    pDis->uHwAddress      = uHwAddress;
#endif
    pDis->sysFuncWrite    = sysFuncWrite;
    pDis->sysFuncRead     = sysFuncRead;
    pDis->sysFuncEdidRead = sysFuncEdidRead;
    pDis->sysFuncTimer    = sysFuncTimer;

    /* Read the device version register to uDeviceVersion in the
     * Device Instance Structure
     */
    err = getHwRegister(pDis, E_REG_P00_VERSION_R, &pDis->uDeviceVersion);
    RETIF(err != TM_OK, err)
    /* Copy N4 features bits to DIS */
    pDis->uDeviceFeatures = pDis->uDeviceVersion & 
        (E_MASKREG_P00_VERSION_not_h | E_MASKREG_P00_VERSION_not_s);
    /* Mask off N4 features bits in version */
    pDis->uDeviceVersion &= (UInt8)~pDis->uDeviceFeatures;

    /* Quit if version reads zero */
    RETIF(pDis->uDeviceVersion == E_DEV_VERSION_LIST_END,
          TMBSL_ERR_HDMI_COMPATIBILITY)

    /* Search for the device version in the Supported Version 
     * List in the Device Instance Structure.
     */
    for (i = 0, bFound = False; i < E_DEV_VERSION_LIST_NUM; i++)
    {
        if (pDis->uDeviceVersion == pDis->uSupportedVersions[i])
        {
            bFound = True;
        }
    }

    /* IF the device version is not found in the Supported Version List THEN 
     * this driver component is not compatible with the device.
     */
    RETIF(!bFound, TMBSL_ERR_HDMI_COMPATIBILITY);
    /* IF the funcIntCallbacks array pointer is defined
     * THEN for each funcIntCallbacks pointer that is not null:
     * - Copy the pointer to the Device Instance Structure
     *   funcIntCallbacks array.
     */
    for (i = 0; i < HDMITX_CALLBACK_INT_NUM; i++)
    {
        if ((funcIntCallbacks != (tmbslHdmiTxCallbackList_t *)0)
            && (funcIntCallbacks->funcCallback[i] != (ptmbslHdmiTxCallback_t)0))
        {
            pDis->funcIntCallbacks[i] = funcIntCallbacks->funcCallback[i];
        }
        else
        {
            pDis->funcIntCallbacks[i] = (ptmbslHdmiTxCallback_t)0;
        }
    }

    /* Set the bInitialized flag to enable other APIs */
    pDis->bInitialized = True;

    /* Set the PLL before resetting the device */    
    /* PLL registers common configuration */
    err = setHwRegisterFieldTable(pDis, &kCommonPllCfg[0]);
    RETIF_REG_FAIL(err)

    switch (vinFmt)
    {
    /* 480i or 576i video input format */
    case HDMITX_VFMT_06_720x480i_60Hz:
    case HDMITX_VFMT_07_720x480i_60Hz:
    case HDMITX_VFMT_21_720x576i_50Hz:
    case HDMITX_VFMT_22_720x576i_50Hz:
        err = setHwRegisterFieldTable(pDis, &kVfmt480i576iPllCfg[0]);
        RETIF_REG_FAIL(err)

        switch (pixRate)
        {
        case HDMITX_PIXRATE_SINGLE:
            /* Single edge mode, vinFmt 480i or 576i */
            err = setHwRegisterFieldTable(pDis, &kSinglePrateVfmt480i576iPllCfg[0]);
            RETIF_REG_FAIL(err)
        break;
        case HDMITX_PIXRATE_SINGLE_REPEATED:
            /* Single repeated edge mode, vinFmt 480i or 576i */
            err = setHwRegisterFieldTable(pDis, &kSrepeatedPrateVfmt480i576iPllCfg[0]);
            RETIF_REG_FAIL(err)
        break;
        default:
            /* Double edge mode doesn't exist for vinFmt 480i or 576i */
            return(TMBSL_ERR_HDMI_INCONSISTENT_PARAMS);
        }        

        break;

    /* Others video input format */
    default:
        err = setHwRegisterFieldTable(pDis, &kVfmtOtherPllCfg[0]);
        RETIF_REG_FAIL(err)
        
        switch (pixRate)
        {
        case HDMITX_PIXRATE_SINGLE:
            /* Single edge mode, vinFmt other than 480i or 576i */
            err = setHwRegisterFieldTable(pDis, &kSinglePrateVfmtOtherPllCfg[0]);
            RETIF_REG_FAIL(err)
        break;
        case HDMITX_PIXRATE_DOUBLE:
            /* Double edge mode, vinFmt other than 480i or 576i */
            err = setHwRegisterFieldTable(pDis, &kDoublePrateVfmtOtherPllCfg[0]);
            RETIF_REG_FAIL(err)
        break;
        default:
            /* Single repeated edge mode doesn't exist for other vinFmt */
            return(TMBSL_ERR_HDMI_INCONSISTENT_PARAMS);
        }        

        break;
    }

    /* Reset the device */
    err = tmbslTDA9984Reset(txUnit);
    RETIF(err != TM_OK, err)

    /* DDC interface is disable for TDA9984 after reset, enable it */
    err = setHwRegister(pDis, E_REG_P00_DDC_DISABLE_RW, 0x00);
    RETIF(err != TM_OK, err)

    /* Set clock speed of the DDC channel */ 
    err = setHwRegister(pDis, E_REG_P00_TIMER_H_W, E_MASKREG_P00_TIMER_H_im_clksel);
    RETIF(err != TM_OK, err)
    err = setHwRegister(pDis, E_REG_P12_TX3_RW, 100);
    RETIF(err != TM_OK, err)

    /* The DIS hotplug status is HDMITX_HOTPLUG_INVALID, so call the main 
     * interrupt handler to read the current Hot Plug status and run any
     * registered HPD callback before interrupts are enabled below */
    err = tmbslTDA9984HwHandleInterrupt(txUnit);
    RETIF(err != TM_OK, err)

    /* enable sw _interrupt for debug */
    err = setHwRegister(pDis, E_REG_P00_INT_FLAGS_1_RW, 
                        E_MASKREG_P00_INT_FLAGS_1_sw_int);

    /* enable rx_sense and edid read */
    err = setHwRegister(pDis, E_REG_P00_INT_FLAGS_2_RW, 
                        E_MASKREG_P00_INT_FLAGS_2_rx_sense
                        |E_MASKREG_P00_INT_FLAGS_2_edid_blk_rd);

    /* Enable only the Hot Plug detect interrupt */
    err = setHwRegister(pDis, E_REG_P00_INT_FLAGS_0_RW, 
                        E_MASKREG_P00_INT_FLAGS_0_hpd);

    /* Enable HPD and RX sense IT after first call done by init function */
    gMiscInterruptHpdRxEnable = True;

    return err;
}

/*============================================================================*/
/* tmbslTDA9984PowerGetState                                                   */
/*============================================================================*/
tmErrorCode_t
tmbslTDA9984PowerGetState
(
    tmUnitSelect_t      txUnit,
    ptmPowerState_t     pePowerState
)
{
    tmHdmiTxobject_t *pDis;   /* Pointer to Device Instance Structure */
    tmErrorCode_t     err;    /* Error code */

    /* Check unit parameter and point to its object */
    err = checkUnitSetDis(txUnit, &pDis);
    RETIF(err != TM_OK, err)

    /* Check remaining parameters */
    RETIF_BADPARAM(pePowerState == (ptmPowerState_t)0)

    /* Read the device power status and set the pePowerState
     * return parameter 
     */
    *pePowerState = pDis->ePowerState;

    return TM_OK;
}

/*============================================================================*/
/* tmbslTDA9984PowerSetState                                                   */
/*============================================================================*/
tmErrorCode_t
tmbslTDA9984PowerSetState
(
    tmUnitSelect_t      txUnit,
    tmPowerState_t      ePowerState
)
{
    tmHdmiTxobject_t *pDis;   /* Pointer to Device Instance Structure */
    tmErrorCode_t     err;    /* Error code */

    /* Check unit parameter and point to its object */
    err = checkUnitSetDis(txUnit, &pDis);
    RETIF(err != TM_OK, err)

    /* Check remaining parameters */
    RETIF_BADPARAM(ePowerState > tmPowerOff)

    /* Treat Off and Suspend the same as Standby */
    if ((ePowerState == tmPowerOff) || (ePowerState == tmPowerSuspend))
    {
        ePowerState = tmPowerStandby;
    }

    /* Set the device power control register to the requested ePowerState 
     * state 
     */
    if (ePowerState == tmPowerOn)
    {
        /** Set power On state control registers */
        err = setHwRegisterFieldTable(pDis, &kPowerOn[0]);
        RETIF_REG_FAIL(err)

        if (pDis->ePowerState == tmPowerStandby)
        {
            if (pDis->hotPlugStatus == HDMITX_HOTPLUG_ACTIVE)
            {
                if (pDis->EdidStatus == HDMITX_EDID_NOT_READ)
                {
                    setState(pDis, EV_RESUME_AWAIT_EDID);                    
                }
                else
                {
                    setState(pDis, EV_RESUME_PLUGGEDIN);
                }
            }
            else
            {
                setState(pDis, EV_RESUME_UNPLUGGED);
            }
        }
    }
    else
    {
        /** Set power standby state control registers */
        err = setHwRegisterFieldTable(pDis, &kPowerOff[0]);
        RETIF_REG_FAIL(err)

        setState(pDis, EV_STANDBY);
    }

    /* Save the ePowerState value in the Device Instance Structure */
    pDis->ePowerState = ePowerState;

    return TM_OK;
}

/*============================================================================*/
/* tmbslTDA9984Reset                                                           */
/*============================================================================*/
tmErrorCode_t
tmbslTDA9984Reset
(
    tmUnitSelect_t      txUnit
)
{
    tmHdmiTxobject_t *pDis;       /* Pointer to Device Instance Structure */
    tmErrorCode_t     err;        /* Error code */

    /* Check unit parameter and point to its object */
    err = checkUnitSetDis(txUnit, &pDis);
    RETIF(err != TM_OK, err)

    /* Write to the transmitter to do a soft reset. Don't abort after any
     * error here, to ensure full reset.
     */
    (void)setHwRegisterField(pDis, E_REG_P00_MAIN_CNTRL0_W,
                             E_MASKREG_P00_MAIN_CNTRL0_sr, 1);
    pDis->sysFuncTimer(50); /* ms */
    (void)setHwRegisterField(pDis, E_REG_P00_MAIN_CNTRL0_W,
                             E_MASKREG_P00_MAIN_CNTRL0_sr, 0);
    
    /* Clear any colourbars */
    (void)setHwRegisterField(pDis, E_REG_P00_HVF_CNTRL_0_W,
                             E_MASKREG_P00_HVF_CNTRL_0_sm, 0);

#ifndef NO_HDCP
    /* Disable any scheduled function and HDCP check timer */
    pDis->HdcpFuncRemainingMs = 0;
    pDis->HdcpCheckNum = 0;
#endif /* NO_HDCP */

    /* Transmitter is now inactive so treat as if sink has been unplugged */
    setState(pDis, EV_UNPLUGGED);
    return TM_OK;
}

/*============================================================================*/
/* tmbslTDA9984SwGetVersion                                                    */
/*============================================================================*/
tmErrorCode_t   
tmbslTDA9984SwGetVersion 
(
    ptmSWVersion_t      pSWVersion
)
{
    /* Check parameters */
    RETIF_BADPARAM(pSWVersion == (ptmSWVersion_t)0)

    /* Get the version details of the component. */
    pSWVersion->compatibilityNr = HDMITX_BSL_COMP_NUM;
    pSWVersion->majorVersionNr  = HDMITX_BSL_MAJOR_VER;
    pSWVersion->minorVersionNr  = HDMITX_BSL_MINOR_VER;

    return TM_OK;
}

/*============================================================================*/
/* tmbslTDA9984SysTimerWait                                                    */
/*============================================================================*/
tmErrorCode_t   
tmbslTDA9984SysTimerWait 
(
    tmUnitSelect_t      txUnit,
    UInt16              waitMs
)
{
    tmHdmiTxobject_t *pDis;       /* Pointer to Device Instance Structure */
    tmErrorCode_t     err;        /* Error code */

    /* Check unit parameter and point to TX unit object */
    err = checkUnitSetDis(txUnit, &pDis);
    RETIF(err != TM_OK, err)

    /* Return if this device timer is not set up */
    RETIF(!pDis->sysFuncTimer, TMBSL_ERR_HDMI_NOT_INITIALIZED)

    /* Wait for the requested time */
    pDis->sysFuncTimer(waitMs);

    return TM_OK;
}

/*============================================================================*/
/* tmbslTDA9984TestSetMode                                                     */
/*============================================================================*/
#ifndef DEMO_BUILD
tmErrorCode_t
tmbslTDA9984TestSetMode
(
    tmUnitSelect_t          txUnit,
    tmbslHdmiTxTestMode_t   testMode,
    tmbslHdmiTxTestState_t  testState
)
{
    tmHdmiTxobject_t *pDis;       /* Pointer to Device Instance Structure */
    tmErrorCode_t     err;        /* Error code */
    /* Register used to activate a test */
    UInt16            testReg = E_REG_P00_VIP_CNTRL_4_W; 
    /* Register bitfield mask used */
    UInt8             testMask = E_MASKREG_P00_VIP_CNTRL_4_tst_pat; 

    /* Check unit parameter and point to TX unit object */
    err = checkUnitSetDis(txUnit, &pDis);
    RETIF(err != TM_OK, err)

    /* Check parameters */
    RETIF_BADPARAM(testMode  >= HDMITX_TESTMODE_INVALID)
    RETIF_BADPARAM(testState >= HDMITX_TESTSTATE_INVALID)

    /* Set the mode selected by testMode to the state indicated by testState */
    switch (testMode)
    {
    case HDMITX_TESTMODE_PAT:
        testReg  = E_REG_P00_VIP_CNTRL_4_W;
        testMask = E_MASKREG_P00_VIP_CNTRL_4_tst_pat;
        break;
    case HDMITX_TESTMODE_656:
        testReg  = E_REG_P00_VIP_CNTRL_4_W;
        testMask = E_MASKREG_P00_VIP_CNTRL_4_tst_656;
        break;
    case HDMITX_TESTMODE_SERPHOE:
        testReg  = E_REG_P02_TEST1_RW;
        testMask = E_MASKREG_P02_TEST1_tstserphoe;
        break;
    case HDMITX_TESTMODE_NOSC:
        testReg  = E_REG_P02_TEST1_RW;
        testMask = E_MASKREG_P02_TEST1_tst_nosc;
        break;
    case HDMITX_TESTMODE_HVP:
        testReg  = E_REG_P02_TEST1_RW;
        testMask = E_MASKREG_P02_TEST1_tst_hvp;
        break;
    case HDMITX_TESTMODE_PWD:
        testReg  = E_REG_P02_TEST2_RW;
        testMask = E_MASKREG_P02_TEST2_pwd1v8;
        break;
    case HDMITX_TESTMODE_DIVOE:
        testReg  = E_REG_P02_TEST2_RW;
        testMask = E_MASKREG_P02_TEST2_divtestoe;
        break;
    case HDMITX_TESTMODE_INVALID:
    default:
        break;
    }
    err = setHwRegisterField(pDis, testReg, testMask, (UInt8)testState);
    return err;
}
#endif /* DEMO_BUILD */

/*============================================================================*/
/* hotPlugRestore                                                   */
/*============================================================================*/
tmErrorCode_t
hotPlugRestore 
(
    tmUnitSelect_t txUnit
)
{
    tmHdmiTxobject_t *pDis;       /* Pointer to Device Instance Structure */
    tmErrorCode_t     err;        /* Error code */

    /* Check unit parameter and point to TX unit object */
    err = checkUnitSetDis(txUnit, &pDis);
    RETIF(err != TM_OK, err)

    /* Set clock speed of the DDC channel */ 
    err = setHwRegister(pDis, E_REG_P00_TIMER_H_W, E_MASKREG_P00_TIMER_H_im_clksel);
    RETIF(err != TM_OK, err)
    err = setHwRegister(pDis, E_REG_P12_TX3_RW, 100);
    RETIF(err != TM_OK, err)

    /* DDC interface is disable for TDA9984 after reset, enable it */
    err = setHwRegister(pDis, E_REG_P00_DDC_DISABLE_RW, 0x01);
    RETIF(err != TM_OK, err)
    /* DDC interface is disable for TDA9984 after reset, enable it */
    err = setHwRegister(pDis, E_REG_P00_DDC_DISABLE_RW, 0x00);
    RETIF(err != TM_OK, err)

    /* enable sw _interrupt for debug */
    err = setHwRegister(pDis, E_REG_P00_INT_FLAGS_1_RW, 
                        E_MASKREG_P00_INT_FLAGS_1_sw_int);
    RETIF(err != TM_OK, err)

    /* enable rx_sense and edid read */
    err = setHwRegister(pDis, E_REG_P00_INT_FLAGS_2_RW, 
                        E_MASKREG_P00_INT_FLAGS_2_rx_sense
                        |E_MASKREG_P00_INT_FLAGS_2_edid_blk_rd);
    RETIF(err != TM_OK, err)

    /* Enable only the Hot Plug detect interrupt */
    err = setHwRegister(pDis, E_REG_P00_INT_FLAGS_0_RW, 
                        E_MASKREG_P00_INT_FLAGS_0_hpd);
    RETIF(err != TM_OK, err)
#ifndef NO_HDCP
    /* While HPD is low after soft reset is done, need to download the keys again */
	if (pDis->HdcpSeed)
	{
		err = tmbslTDA9984HdcpDownloadKeys(txUnit, pDis->HdcpSeed, HDMITX_HDCP_DECRYPT_ENABLE);
	}
#endif /* NO_HDCP */


    if (pDis->ePowerState == tmPowerOff)
    {
        /** Set power standby state control registers */
        err = setHwRegisterFieldTable(pDis, &kPowerOff[0]);
        RETIF_REG_FAIL(err)
    }

    return err;
}

/*============================================================================*/
/* tmbslTDA9984EnableCallback                                                 */
/*============================================================================*/
tmErrorCode_t
tmbslTDA9984EnableCallback
(
    tmUnitSelect_t           txUnit,
    tmbslHdmiTxCallbackInt_t callbackSource,
    Bool                     enable
)
{
    tmHdmiTxobject_t *pDis;           /* Pointer to Device Instance Structure */
    tmErrorCode_t     err = TM_OK;    /* Error code */

    /* Check unit parameter and point to TX unit object */
    err = checkUnitSetDis(txUnit, &pDis);
    RETIF(err != TM_OK, err)

    /* Check parameters */
    RETIF_BADPARAM( callbackSource >= HDMITX_CALLBACK_INT_NUM )

    switch (callbackSource)
    {
        case HDMITX_CALLBACK_INT_VS_RPT:
            /* Enable or disable VS Interrupt */
            err = setHwRegisterField(pDis, 
                                     E_REG_P00_INT_FLAGS_1_RW,
                                     E_MASKREG_P00_INT_FLAGS_1_vs_rpt,
                                     (UInt8)enable);
            if (enable)
            {
                pDis->InterruptsEnable |= (1 << callbackSource);
            }
            else
            {
                pDis->InterruptsEnable &= ~(1 << callbackSource);
            }
            break;
        default:                      
            err = TMBSL_ERR_HDMI_NOT_SUPPORTED;
            break;
    }
    
    return err;
}

/*============================================================================*/
/* tmbslTDA9984Set5vpower                                                     */
/*============================================================================*/
tmErrorCode_t
tmbslTDA9984Set5vpower
(
    tmUnitSelect_t txUnit,
    Bool           pwrEnable
)
{
    tmHdmiTxobject_t *pDis;           /* Pointer to Device Instance Structure */
    tmErrorCode_t     err;            /* Error code */

    /* Check unit parameter and point to TX unit object */
    err = checkUnitSetDis(txUnit, &pDis);
    RETIF(err != TM_OK, err)

    DUMMY_ACCESS(pwrEnable);

    return TMBSL_ERR_HDMI_NOT_SUPPORTED;
}

/*============================================================================*/
/* tmbslDebugWriteFakeRegPage                                                 */
/*============================================================================*/
tmErrorCode_t tmbslDebugWriteFakeRegPage( tmUnitSelect_t txUnit )
{
    tmHdmiTxobject_t *pDis;           /* Pointer to Device Instance Structure */
    tmErrorCode_t     err;            /* Error code */

    err = checkUnitSetDis(txUnit, &pDis);

    pDis->curRegPage = 0x20; 

    return err;
}


/*============================================================================*/
/*                            END OF FILE                                     */
/*============================================================================*/
