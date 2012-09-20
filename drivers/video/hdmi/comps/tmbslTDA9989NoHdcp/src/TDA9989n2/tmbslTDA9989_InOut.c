/**
 * Copyright (C) 2009 NXP N.V., All Rights Reserved.
 * This source code and any compilation or derivative thereof is the proprietary
 * information of NXP N.V. and is confidential in nature. Under no circumstances
 * is this software to be  exposed to or placed under an Open Source License of
 * any type without the expressed written permission of NXP N.V.
 *
 * \file          tmbslTDA9989_InOut.c
 *
 * \version       %version: 5 %
 *
 *
*/

/*============================================================================*/
/*                       INCLUDE FILES                                        */
/*============================================================================*/

#include "tmbslHdmiTx_types.h"
#include "tmbslTDA9989_Functions.h"
#include "tmbslTDA9989_local.h"

#include "tmbslTDA9989_State_l.h"
#include "tmbslTDA9989_Misc_l.h"
#include "tmbslTDA9989_InOut_l.h"
/*============================================================================*/
/*                     TYPES DECLARATIONS                                     */
/*============================================================================*/

#define SSD_UNUSED_VALUE 0xF0

#ifdef FORMAT_PC
#define DEPTH_COLOR_PC 1  /* PC_FORMAT only 8 bits available */
#endif /* FORMAT_PC */

#define REG_VAL_SEL_AIP_SPDIF   0
#define REG_VAL_SEL_AIP_I2S     1
#define REG_VAL_SEL_AIP_OBA     2
#define REG_VAL_SEL_AIP_DST     3
#define REG_VAL_SEL_AIP_HBR     5

/*============================================================================*/
/*                       CONSTANTS DECLARATIONS EXPORTED                      */
/*============================================================================*/

extern CONST_DAT tmHdmiTxRegMaskVal_t kVoutHdcpOff[];
extern CONST_DAT tmHdmiTxRegMaskVal_t kVoutHdcpOn[];


/**
 * Table of PLL settings registers to configure for all video input format (vinFmt)
 */
CONST_DAT tmHdmiTxRegMaskVal_t kCommonPllCfg[] =
{
    {E_REG_P02_PLL_SERIAL_1_RW, E_MASKREG_ALL,  0x00},
    {E_REG_P02_PLL_SERIAL_2_RW, E_MASKREG_ALL,  0x01},
    {E_REG_P02_PLL_SERIAL_3_RW, E_MASKREG_ALL,  0x00},
    {E_REG_P02_SERIALIZER_RW,   E_MASKREG_ALL,  0x00},
    {E_REG_P02_BUFFER_OUT_RW,   E_MASKREG_ALL,  0x00},
    {E_REG_P02_PLL_SCG1_RW,     E_MASKREG_ALL,  0x00},
    {E_REG_P02_AUDIO_DIV_RW,    E_MASKREG_ALL,  0x03},
    /*{E_REG_P02_TEST2_RW,        E_MASKREG_ALL,  0x00},*/
    {E_REG_P02_SEL_CLK_RW,      E_MASKREG_ALL,  0x09},
    {0,0,0}
};

/**
 * Table of PLL settings registers to configure double mode pixel rate,
 * vinFmt other than 480i or 576i
 */
CONST_DAT tmHdmiTxRegMaskVal_t kDoublePrateVfmtOtherPllCfg[] =
{
    {E_REG_P02_PLL_SCG2_RW,     E_MASKREG_ALL,  0x00},
    {0,0,0}
};

/**
 * Table of PLL settings registers to configure for single mode pixel rate,
 * vinFmt 480i or 576i only
 */
CONST_DAT tmHdmiTxRegMaskVal_t kSinglePrateVfmt480i576iPllCfg[] =
{
    {E_REG_P02_PLL_SCG2_RW,     E_MASKREG_ALL,  0x11},
    {0,0,0}
};

/**
 * Table of PLL settings registers to configure single mode pixel rate,
 * vinFmt other than 480i or 576i
 */
CONST_DAT tmHdmiTxRegMaskVal_t kSinglePrateVfmtOtherPllCfg[] =
{
    {E_REG_P02_PLL_SCG2_RW,     E_MASKREG_ALL,  0x10},
    {0,0,0}
};

/**
 * Table of PLL settings registers to configure for single repeated mode pixel rate,
 * vinFmt 480i or 576i only
 */
CONST_DAT tmHdmiTxRegMaskVal_t kSrepeatedPrateVfmt480i576iPllCfg[] =
{
    {E_REG_P02_PLL_SCG2_RW,     E_MASKREG_ALL,  0x01},
    {0,0,0}
};

/**
 * Table of PLL settings registers to configure for 480i and 576i vinFmt
 */
CONST_DAT tmHdmiTxRegMaskVal_t kVfmt480i576iPllCfg[] =
{
    {E_REG_P02_PLL_SCGN1_RW,    E_MASKREG_ALL,  0x14},
    {E_REG_P02_PLL_SCGN2_RW,    E_MASKREG_ALL,  0x00},
    {E_REG_P02_PLL_SCGR1_RW,    E_MASKREG_ALL,  0x0A},
    {E_REG_P02_PLL_SCGR2_RW,    E_MASKREG_ALL,  0x00},
    {0,0,0}
};

/**
 * Table of PLL settings registers to configure for other vinFmt than 480i and 576i
 */
CONST_DAT tmHdmiTxRegMaskVal_t kVfmtOtherPllCfg[] =
{
    {E_REG_P02_PLL_SCGN1_RW,    E_MASKREG_ALL,  0xFA},
    {E_REG_P02_PLL_SCGN2_RW,    E_MASKREG_ALL,  0x00},
    {E_REG_P02_PLL_SCGR1_RW,    E_MASKREG_ALL,  0x5B},
    {E_REG_P02_PLL_SCGR2_RW,    E_MASKREG_ALL,  0x00},
    {0,0,0}
};

/**
 * Lookup table to convert from EIA/CEA TV video formats used in the EDID and
 * in API parameters to pixel clock frequencies, according to SCS Table
 * "HDMI Pixel Clock Frequencies per EIA/CEA-861B Video Output Format".
 * The other index is the veritical frame frequency.
 */
 
CONST_DAT UInt8 kVfmtToPixClk_TV[HDMITX_VFMT_TV_NUM][HDMITX_VFREQ_NUM] =
{
  /* HDMITX_VFREQ_24Hz HDMITX_VFREQ_25Hz HDMITX_VFREQ_30Hz  HDMITX_VFREQ_50Hz HDMITX_VFREQ_59Hz HDMITX_VFREQ_60Hz */
    {E_PIXCLK_INVALID, E_PIXCLK_INVALID, E_PIXCLK_INVALID, E_PIXCLK_INVALID, E_PIXCLK_INVALID, E_PIXCLK_INVALID},   /* HDMITX_VFMT_NULL               */
    {E_PIXCLK_INVALID, E_PIXCLK_INVALID, E_PIXCLK_INVALID, E_PIXCLK_INVALID, E_PIXCLK_25175,   E_PIXCLK_25200},     /* HDMITX_VFMT_01_640x480p_60Hz   */
    {E_PIXCLK_INVALID, E_PIXCLK_INVALID, E_PIXCLK_INVALID, E_PIXCLK_INVALID, E_PIXCLK_27000,   E_PIXCLK_27027},     /* HDMITX_VFMT_02_720x480p_60Hz   */
    {E_PIXCLK_INVALID, E_PIXCLK_INVALID, E_PIXCLK_INVALID, E_PIXCLK_INVALID, E_PIXCLK_27000,   E_PIXCLK_27027},     /* HDMITX_VFMT_03_720x480p_60Hz   */
    {E_PIXCLK_INVALID, E_PIXCLK_INVALID, E_PIXCLK_INVALID, E_PIXCLK_INVALID, E_PIXCLK_74175,   E_PIXCLK_74250},     /* HDMITX_VFMT_04_1280x720p_60Hz  */
    {E_PIXCLK_INVALID, E_PIXCLK_INVALID, E_PIXCLK_INVALID, E_PIXCLK_INVALID, E_PIXCLK_74175,   E_PIXCLK_74250},     /* HDMITX_VFMT_05_1920x1080i_60Hz */
    {E_PIXCLK_INVALID, E_PIXCLK_INVALID, E_PIXCLK_INVALID, E_PIXCLK_INVALID, E_PIXCLK_27000,   E_PIXCLK_27027},     /* HDMITX_VFMT_06_720x480i_60Hz   */
    {E_PIXCLK_INVALID, E_PIXCLK_INVALID, E_PIXCLK_INVALID, E_PIXCLK_INVALID, E_PIXCLK_27000,   E_PIXCLK_27027},     /* HDMITX_VFMT_07_720x480i_60Hz   */
    {E_PIXCLK_INVALID, E_PIXCLK_INVALID, E_PIXCLK_INVALID, E_PIXCLK_INVALID, E_PIXCLK_27000,   E_PIXCLK_27027},     /* HDMITX_VFMT_08_720x240p_60Hz   */
    {E_PIXCLK_INVALID, E_PIXCLK_INVALID, E_PIXCLK_INVALID, E_PIXCLK_INVALID, E_PIXCLK_27000,   E_PIXCLK_27027},     /* HDMITX_VFMT_09_720x240p_60Hz   */
    {E_PIXCLK_INVALID, E_PIXCLK_INVALID, E_PIXCLK_INVALID, E_PIXCLK_INVALID, E_PIXCLK_54000,   E_PIXCLK_54054},     /* HDMITX_VFMT_10_720x480i_60Hz   */
    {E_PIXCLK_INVALID, E_PIXCLK_INVALID, E_PIXCLK_INVALID, E_PIXCLK_INVALID, E_PIXCLK_54000,   E_PIXCLK_54054},     /* HDMITX_VFMT_11_720x480i_60Hz   */
    {E_PIXCLK_INVALID, E_PIXCLK_INVALID, E_PIXCLK_INVALID, E_PIXCLK_INVALID, E_PIXCLK_54000,   E_PIXCLK_54054},     /* HDMITX_VFMT_12_720x240p_60Hz   */
    {E_PIXCLK_INVALID, E_PIXCLK_INVALID, E_PIXCLK_INVALID, E_PIXCLK_INVALID, E_PIXCLK_54000,   E_PIXCLK_54054},     /* HDMITX_VFMT_13_720x240p_60Hz   */
    {E_PIXCLK_INVALID, E_PIXCLK_INVALID, E_PIXCLK_INVALID, E_PIXCLK_INVALID, E_PIXCLK_54000,   E_PIXCLK_54054},     /* HDMITX_VFMT_14_1440x480p_60Hz  */
    {E_PIXCLK_INVALID, E_PIXCLK_INVALID, E_PIXCLK_INVALID, E_PIXCLK_INVALID, E_PIXCLK_54000,   E_PIXCLK_54054},     /* HDMITX_VFMT_15_1440x480p_60Hz  */
    {E_PIXCLK_INVALID, E_PIXCLK_INVALID, E_PIXCLK_INVALID, E_PIXCLK_INVALID, E_PIXCLK_148350,  E_PIXCLK_148500},    /* HDMITX_VFMT_16_1920x1080p_60Hz */
    {E_PIXCLK_INVALID, E_PIXCLK_INVALID, E_PIXCLK_INVALID, E_PIXCLK_27000,   E_PIXCLK_INVALID, E_PIXCLK_INVALID},   /* HDMITX_VFMT_17_720x576p_50Hz   */
    {E_PIXCLK_INVALID, E_PIXCLK_INVALID, E_PIXCLK_INVALID, E_PIXCLK_27000,   E_PIXCLK_INVALID, E_PIXCLK_INVALID},   /* HDMITX_VFMT_18_720x576p_50Hz   */
    {E_PIXCLK_INVALID, E_PIXCLK_INVALID, E_PIXCLK_INVALID, E_PIXCLK_74250,   E_PIXCLK_INVALID, E_PIXCLK_INVALID},   /* HDMITX_VFMT_19_1280x720p_50Hz  */
    {E_PIXCLK_INVALID, E_PIXCLK_INVALID, E_PIXCLK_INVALID, E_PIXCLK_74250,   E_PIXCLK_INVALID, E_PIXCLK_INVALID},   /* HDMITX_VFMT_20_1920x1080i_50Hz */
    {E_PIXCLK_INVALID, E_PIXCLK_INVALID, E_PIXCLK_INVALID, E_PIXCLK_27000,   E_PIXCLK_INVALID, E_PIXCLK_INVALID},   /* HDMITX_VFMT_21_720x576i_50Hz   */
    {E_PIXCLK_INVALID, E_PIXCLK_INVALID, E_PIXCLK_INVALID, E_PIXCLK_27000,   E_PIXCLK_INVALID, E_PIXCLK_INVALID},   /* HDMITX_VFMT_22_720x576i_50Hz   */
    {E_PIXCLK_INVALID, E_PIXCLK_INVALID, E_PIXCLK_INVALID, E_PIXCLK_27000,   E_PIXCLK_INVALID, E_PIXCLK_INVALID},   /* HDMITX_VFMT_23_720x288p_50Hz   */
    {E_PIXCLK_INVALID, E_PIXCLK_INVALID, E_PIXCLK_INVALID, E_PIXCLK_27000,   E_PIXCLK_INVALID, E_PIXCLK_INVALID},   /* HDMITX_VFMT_24_720x288p_50Hz   */
    {E_PIXCLK_INVALID, E_PIXCLK_INVALID, E_PIXCLK_INVALID, E_PIXCLK_54000,   E_PIXCLK_INVALID, E_PIXCLK_INVALID},   /* HDMITX_VFMT_25_720x576i_50Hz   */
    {E_PIXCLK_INVALID, E_PIXCLK_INVALID, E_PIXCLK_INVALID, E_PIXCLK_54000,   E_PIXCLK_INVALID, E_PIXCLK_INVALID},   /* HDMITX_VFMT_26_720x576i_50Hz   */
    {E_PIXCLK_INVALID, E_PIXCLK_INVALID, E_PIXCLK_INVALID, E_PIXCLK_54000,   E_PIXCLK_INVALID, E_PIXCLK_INVALID},   /* HDMITX_VFMT_27_720x288p_50Hz   */
    {E_PIXCLK_INVALID, E_PIXCLK_INVALID, E_PIXCLK_INVALID, E_PIXCLK_54000,   E_PIXCLK_INVALID, E_PIXCLK_INVALID},   /* HDMITX_VFMT_28_720x288p_50Hz   */
    {E_PIXCLK_INVALID, E_PIXCLK_INVALID, E_PIXCLK_INVALID, E_PIXCLK_54000,   E_PIXCLK_INVALID, E_PIXCLK_INVALID},   /* HDMITX_VFMT_29_1440x576p_50Hz  */
    {E_PIXCLK_INVALID, E_PIXCLK_INVALID, E_PIXCLK_INVALID, E_PIXCLK_54000,   E_PIXCLK_INVALID, E_PIXCLK_INVALID},   /* HDMITX_VFMT_30_1440x576p_50Hz  */
    {E_PIXCLK_INVALID, E_PIXCLK_INVALID, E_PIXCLK_INVALID, E_PIXCLK_148500,  E_PIXCLK_INVALID, E_PIXCLK_INVALID},   /* HDMITX_VFMT_31_1920x1080p_50Hz */
    {E_PIXCLK_74250,   E_PIXCLK_INVALID, E_PIXCLK_INVALID, E_PIXCLK_INVALID, E_PIXCLK_INVALID, E_PIXCLK_INVALID},   /* HDMITX_VFMT_32_1920x1080p_24Hz */
    {E_PIXCLK_INVALID, E_PIXCLK_74250,   E_PIXCLK_INVALID, E_PIXCLK_INVALID, E_PIXCLK_INVALID, E_PIXCLK_INVALID},   /* HDMITX_VFMT_33_1920x1080p_25Hz */
    {E_PIXCLK_INVALID, E_PIXCLK_INVALID, E_PIXCLK_74250,   E_PIXCLK_INVALID, E_PIXCLK_INVALID, E_PIXCLK_INVALID}    /* HDMITX_VFMT_34_1920x1080p_30Hz */
};


/*============================================================================*/
/*                       CONSTANTS DECLARATIONS                               */
/*============================================================================*/


/**
 * Lookup table to convert PC formats used in API parameters to pixel clock 
 * frequencies.
 * The other index is the veritical frame frequency.
 */
#ifdef FORMAT_PC
CONST_DAT UInt8 kVfmtToPixClk_PC[HDMITX_VFMT_TV_NUM] =
{
  /* HDMITX_VFREQ_60Hz HDMITX_VFREQ_70Hz HDMITX_VFREQ_72Hz HDMITX_VFREQ_75Hz HDMITX_VFREQ_85Hz HDMITX_VFREQ_87Hz*/
    E_PIXCLK_INVALID,   /* HDMITX_VFMT_NULL                */
    E_PIXCLK_25175  ,   /* HDMITX_VFMT_PC_640x480p_60Hz   */
    E_PIXCLK_40000  ,   /* HDMITX_VFMT_PC_800x600p_60Hz   */
    E_PIXCLK_INVALID,   /* HDMITX_VFMT_PC_1152x960p_60Hz  */
    E_PIXCLK_65000  ,   /* HDMITX_VFMT_PC_1024x768p_60Hz */
    E_PIXCLK_79500  ,   /* HDMITX_VFMT_PC_1280x768p_60Hz */
    E_PIXCLK_108000 ,   /* HDMITX_VFMT_PC_1280x1024p_60Hz */
    E_PIXCLK_85500  ,   /* HDMITX_VFMT_PC_1360x768p_60Hz  */
    E_PIXCLK_121750 ,   /* HDMITX_VFMT_PC_1400x1050p_60Hz */
    E_PIXCLK_162000 ,   /* HDMITX_VFMT_PC_1600x1200p_60Hz */
    E_PIXCLK_75000  ,   /* HDMITX_VFMT_PC_1024x768p_70Hz  */
    E_PIXCLK_31500  ,   /* HDMITX_VFMT_PC_640x480p_72Hz   */
    E_PIXCLK_50000  ,   /* HDMITX_VFMT_PC_800x600p_72Hz   */
    E_PIXCLK_31500  ,   /* HDMITX_VFMT_PC_640x480p_75Hz   */
    E_PIXCLK_78750  ,   /* HDMITX_VFMT_PC_1024x768p_75Hz  */
    E_PIXCLK_49500  ,   /* HDMITX_VFMT_PC_800x600p_75Hz   */
    E_PIXCLK_INVALID,   /* HDMITX_VFMT_PC_1024x864p_75Hz  */
    E_PIXCLK_INVALID,   /* HDMITX_VFMT_PC_1280x1024p_75Hz */
    E_PIXCLK_INVALID,   /* HDMITX_VFMT_PC_640x350p_85Hz   */
    E_PIXCLK_INVALID,   /* HDMITX_VFMT_PC_640x400p_85Hz   */
    E_PIXCLK_INVALID,   /* HDMITX_VFMT_PC_720x400p_85Hz   */
    E_PIXCLK_36000  ,   /* HDMITX_VFMT_PC_640x480p_85Hz   */
    E_PIXCLK_56250  ,   /* HDMITX_VFMT_PC_800x600p_85Hz   */
    E_PIXCLK_INVALID,   /* HDMITX_VFMT_PC_1024x768p_85Hz  */
    E_PIXCLK_INVALID,   /* HDMITX_VFMT_PC_1152x864p_85Hz  */
    E_PIXCLK_INVALID,   /* HDMITX_VFMT_PC_1280x960p_85Hz  */
    E_PIXCLK_157500,    /* HDMITX_VFMT_PC_1280x1024p_85Hz */
    E_PIXCLK_INVALID    /* HDMITX_VFMT_PC_1024x768i_87Hz  */
};
#endif /* FORMAT_PC */

/**
 * Lookup table to convert from EIA/CEA TV video formats used in the EDID and in
 * API parameters to the format used in the E_REG_P00_VIDFORMAT_W register
 */

#ifndef FORMAT_PC
static CONST_DAT UInt8 kVfmtToRegvfmt_TV[HDMITX_VFMT_TV_NUM] =
#else /* FORMAT_PC */
static CONST_DAT UInt8 kVfmtToRegvfmt_TV[HDMITX_VFMT_TV_NUM + HDMITX_VFMT_PC_NUM] =
#endif /* FORMAT_PC */
{
    E_REGVFMT_INVALID,              /* HDMITX_VFMT_NULL               */
    E_REGVFMT_640x480p_60Hz,        /* HDMITX_VFMT_01_640x480p_60Hz   */
    E_REGVFMT_720x480p_60Hz,        /* HDMITX_VFMT_02_720x480p_60Hz   */
    E_REGVFMT_720x480p_60Hz,        /* HDMITX_VFMT_03_720x480p_60Hz   */
    E_REGVFMT_1280x720p_60Hz,       /* HDMITX_VFMT_04_1280x720p_60Hz  */
    E_REGVFMT_1920x1080i_60Hz,      /* HDMITX_VFMT_05_1920x1080i_60Hz */
    E_REGVFMT_720x480i_60Hz,        /* HDMITX_VFMT_06_720x480i_60Hz   */
    E_REGVFMT_720x480i_60Hz,        /* HDMITX_VFMT_07_720x480i_60Hz   */
    E_REGVFMT_720x240p_60Hz,        /* HDMITX_VFMT_08_720x240p_60Hz   */
    E_REGVFMT_720x240p_60Hz,        /* HDMITX_VFMT_09_720x240p_60Hz   */
    E_REGVFMT_INVALID,              /* HDMITX_VFMT_10_720x480i_60Hz   */
    E_REGVFMT_INVALID,              /* HDMITX_VFMT_11_720x480i_60Hz   */
    E_REGVFMT_INVALID,              /* HDMITX_VFMT_12_720x240p_60Hz   */
    E_REGVFMT_INVALID,              /* HDMITX_VFMT_13_720x240p_60Hz   */
    E_REGVFMT_INVALID,              /* HDMITX_VFMT_14_1440x480p_60Hz  */
    E_REGVFMT_INVALID,              /* HDMITX_VFMT_15_1440x480p_60Hz  */
    E_REGVFMT_1920x1080p_60Hz,      /* HDMITX_VFMT_16_1920x1080p_60Hz */
    E_REGVFMT_720x576p_50Hz,        /* HDMITX_VFMT_17_720x576p_50Hz   */
    E_REGVFMT_720x576p_50Hz,        /* HDMITX_VFMT_18_720x576p_50Hz   */
    E_REGVFMT_1280x720p_50Hz,       /* HDMITX_VFMT_19_1280x720p_50Hz  */
    E_REGVFMT_1920x1080i_50Hz,      /* HDMITX_VFMT_20_1920x1080i_50Hz */
    E_REGVFMT_720x576i_50Hz,        /* HDMITX_VFMT_21_720x576i_50Hz   */
    E_REGVFMT_720x576i_50Hz,        /* HDMITX_VFMT_22_720x576i_50Hz   */
    E_REGVFMT_720x288p_50Hz,        /* HDMITX_VFMT_23_720x288p_50Hz   */
    E_REGVFMT_720x288p_50Hz,        /* HDMITX_VFMT_24_720x288p_50Hz   */
    E_REGVFMT_INVALID,              /* HDMITX_VFMT_25_720x576i_50Hz   */
    E_REGVFMT_INVALID,              /* HDMITX_VFMT_26_720x576i_50Hz   */
    E_REGVFMT_INVALID,              /* HDMITX_VFMT_27_720x288p_50Hz   */
    E_REGVFMT_INVALID,              /* HDMITX_VFMT_28_720x288p_50Hz   */
    E_REGVFMT_INVALID,              /* HDMITX_VFMT_29_1440x576p_50Hz  */
    E_REGVFMT_INVALID,              /* HDMITX_VFMT_30_1440x576p_50Hz  */
    E_REGVFMT_1920x1080p_50Hz,       /* HDMITX_VFMT_31_1920x1080p_50Hz */
    E_REGVFMT_1920x1080p_24Hz,      /* HDMITX_VFMT_32_1920x1080p_24Hz */
    E_REGVFMT_1920x1080p_25Hz,      /* HDMITX_VFMT_33_1920x1080p_25Hz */
    E_REGVFMT_1920x1080p_30Hz,      /* HDMITX_VFMT_34_1920x1080p_30Hz */
    E_REGVFMT_720x480p_60Hz,        /* HDMITX_VFMT_35_2880x480p_60Hz */
    E_REGVFMT_720x480p_60Hz,        /* HDMITX_VFMT_36_2880x480p_60Hz */
    E_REGVFMT_720x576p_50Hz,        /* HDMITX_VFMT_37_2880x576p_50Hz */
    E_REGVFMT_720x576p_50Hz,        /* HDMITX_VFMT_38_2880x576p_50Hz */
#ifdef FORMAT_PC    
    E_REGVFMT_640x480p_60Hz,        /* HDMITX_VFMT_PC_640x480p_60Hz  */
    E_REGVFMT_800x600p_60Hz,        /* HDMITX_VFMT_PC_800x600p_60Hz  */
    E_REGVFMT_INVALID,              /* HDMITX_VFMT_PC_1152x960p_60Hz  */
    E_REGVFMT_1024x768p_60Hz,       /* HDMITX_VFMT_PC_1024x768p_60Hz  */
    E_REGVFMT_1280x768p_60Hz,       /* HDMITX_VFMT_PC_1280x768p_60Hz */
    E_REGVFMT_1280x1024p_60Hz,      /* HDMITX_VFMT_PC_1280x1024p_60Hz  */
    E_REGVFMT_1360x768p_60Hz,       /* HDMITX_VFMT_PC_1360x768p_60Hz */
    E_REGVFMT_1400x1050p_60Hz,      /* HDMITX_VFMT_PC_1400x1050p_60Hz */
    E_REGVFMT_1600x1200p_60Hz,      /* HDMITX_VFMT_PC_1600x1200p_60Hz  */
    E_REGVFMT_1024x768p_70Hz,       /* HDMITX_VFMT_PC_1024x768p_70Hz  */
    E_REGVFMT_640x480p_72Hz,        /* HDMITX_VFMT_PC_640x480p_72Hz  */
    E_REGVFMT_800x600p_72Hz,        /* HDMITX_VFMT_PC_800x600p_72Hz  */
    E_REGVFMT_640x480p_75Hz,        /* HDMITX_VFMT_PC_640x480p_75Hz  */
    E_REGVFMT_1024x768p_75Hz,       /* HDMITX_VFMT_PC_1024x768p_75Hz  */
    E_REGVFMT_800x600p_75Hz,        /* HDMITX_VFMT_PC_800x600p_75Hz  */
    E_REGVFMT_INVALID,              /* HDMITX_VFMT_PC_1024x864p_75Hz  */
    E_REGVFMT_INVALID,              /* HDMITX_VFMT_PC_1280x1024p_75Hz  */
    E_REGVFMT_INVALID,              /* HDMITX_VFMT_PC_640x350p_85Hz  */
    E_REGVFMT_INVALID,              /* HDMITX_VFMT_PC_640x400p_85Hz  */
    E_REGVFMT_INVALID,              /* HDMITX_VFMT_PC_720x400p_85Hz  */
    E_REGVFMT_640x480p_85Hz,        /* HDMITX_VFMT_PC_640x480p_85Hz  */
    E_REGVFMT_800x600p_85Hz,        /* HDMITX_VFMT_PC_800x600p_85Hz  */
    E_REGVFMT_INVALID,              /* HDMITX_VFMT_PC_1024x768p_85Hz  */
    E_REGVFMT_INVALID,              /* HDMITX_VFMT_PC_1152x864p_85Hz  */
    E_REGVFMT_INVALID,              /* HDMITX_VFMT_PC_1280x960p_85Hz  */
    E_REGVFMT_1280x1024p_85Hz,      /* HDMITX_VFMT_PC_1280x1024p_85Hz */
    E_REGVFMT_INVALID,              /* HDMITX_VFMT_PC_1024x768i_87Hz  */
#endif /* FORMAT PC */ 
    
};

/**
 * Lookup table to convert from EIA/CEA TV video formats used in API
 * parameters to the vid_format values used in the 
 * E_REG_P01_VIDFORMAT_RW register
 */
static CONST_DAT UInt8 kVfmtToRegvfmtVfi_TV[HDMITX_VFMT_TV_NUM] =
{
    E_REGVFMT_VFIN_INVALID,         /* HDMITX_VFMT_NULL               */
    E_REGVFMT_VFIN_480p_60Hz,       /* HDMITX_VFMT_01_640x480p_60Hz   */
    E_REGVFMT_VFIN_480p_60Hz,       /* HDMITX_VFMT_02_720x480p_60Hz   */
    E_REGVFMT_VFIN_480p_60Hz,       /* HDMITX_VFMT_03_720x480p_60Hz   */
    E_REGVFMT_VFIN_720p_60Hz,       /* HDMITX_VFMT_04_1280x720p_60Hz  */
    E_REGVFMT_VFIN_1080i_60Hz,      /* HDMITX_VFMT_05_1920x1080i_60Hz */
    E_REGVFMT_VFIN_480i_60Hz,       /* HDMITX_VFMT_06_720x480i_60Hz   */
    E_REGVFMT_VFIN_480i_60Hz,       /* HDMITX_VFMT_07_720x480i_60Hz   */
    E_REGVFMT_VFIN_INVALID,         /* HDMITX_VFMT_08_720x240p_60Hz   */
    E_REGVFMT_VFIN_INVALID,         /* HDMITX_VFMT_09_720x240p_60Hz   */
    E_REGVFMT_VFIN_480i_60Hz,       /* HDMITX_VFMT_10_720x480i_60Hz   */
    E_REGVFMT_VFIN_480i_60Hz,       /* HDMITX_VFMT_11_720x480i_60Hz   */
    E_REGVFMT_VFIN_INVALID,         /* HDMITX_VFMT_12_720x240p_60Hz   */
    E_REGVFMT_VFIN_INVALID,         /* HDMITX_VFMT_13_720x240p_60Hz   */
    E_REGVFMT_VFIN_480p_60Hz,       /* HDMITX_VFMT_14_1440x480p_60Hz  */
    E_REGVFMT_VFIN_480p_60Hz,       /* HDMITX_VFMT_15_1440x480p_60Hz  */
    E_REGVFMT_VFIN_INVALID,         /* HDMITX_VFMT_16_1920x1080p_60Hz */
    E_REGVFMT_VFIN_576p_50Hz,       /* HDMITX_VFMT_17_720x576p_50Hz   */
    E_REGVFMT_VFIN_576p_50Hz,       /* HDMITX_VFMT_18_720x576p_50Hz   */
    E_REGVFMT_VFIN_720p_50Hz,       /* HDMITX_VFMT_19_1280x720p_50Hz  */
    E_REGVFMT_VFIN_1080i_50Hz,      /* HDMITX_VFMT_20_1920x1080i_50Hz */
    E_REGVFMT_VFIN_576i_50Hz,       /* HDMITX_VFMT_21_720x576i_50Hz   */
    E_REGVFMT_VFIN_576i_50Hz,       /* HDMITX_VFMT_22_720x576i_50Hz   */
    E_REGVFMT_VFIN_INVALID,         /* HDMITX_VFMT_23_720x288p_50Hz   */
    E_REGVFMT_VFIN_INVALID,         /* HDMITX_VFMT_24_720x288p_50Hz   */
    E_REGVFMT_VFIN_576i_50Hz,       /* HDMITX_VFMT_25_720x576i_50Hz   */
    E_REGVFMT_VFIN_576i_50Hz,       /* HDMITX_VFMT_26_720x576i_50Hz   */
    E_REGVFMT_VFIN_INVALID,         /* HDMITX_VFMT_27_720x288p_50Hz   */
    E_REGVFMT_VFIN_INVALID,         /* HDMITX_VFMT_28_720x288p_50Hz   */
    E_REGVFMT_VFIN_576p_50Hz,       /* HDMITX_VFMT_29_1440x576p_50Hz  */
    E_REGVFMT_VFIN_576p_50Hz,       /* HDMITX_VFMT_30_1440x576p_50Hz  */
    E_REGVFMT_VFIN_INVALID          /* HDMITX_VFMT_31_1920x1080p_50Hz */
};

/**
 * Lookup table to convert from EIA/CEA TV video formats used in API
 * parameters to the vid_format_i and vid_format_o values used in the 
 * E_REG_P01_SC_VIDFORMAT_W register
 */
#define SCIO(scin,scout)            (((scin)<<4)|(scout))
#define SCIO2SCIN(scio)             (UInt8)(((scio)>>4)&0xF)
#define SCIO2SCOUT(scio)            (UInt8)((scio)&0xF)

static CONST_DAT UInt8 kVfmtToRegvfmtScio_TV[HDMITX_VFMT_TV_NUM] =
{
    SCIO(E_REGVFMT_SCIN_INVALID,        E_REGVFMT_SCOUT_INVALID),         /* HDMITX_VFMT_NULL               */
    SCIO(E_REGVFMT_SCIN_480p_60Hz,      E_REGVFMT_SCOUT_480p_60Hz),       /* HDMITX_VFMT_01_640x480p_60Hz   */
    SCIO(E_REGVFMT_SCIN_480p_60Hz,      E_REGVFMT_SCOUT_480p_60Hz),       /* HDMITX_VFMT_02_720x480p_60Hz   */
    SCIO(E_REGVFMT_SCIN_480p_60Hz,      E_REGVFMT_SCOUT_480p_60Hz),       /* HDMITX_VFMT_03_720x480p_60Hz   */
    SCIO(E_REGVFMT_SCIN_720p_50Hz_60Hz, E_REGVFMT_SCOUT_720p_50Hz_60Hz),  /* HDMITX_VFMT_04_1280x720p_60Hz  */
    SCIO(E_REGVFMT_SCIN_1080i_50Hz_60Hz,E_REGVFMT_SCOUT_1080i_50Hz_60Hz), /* HDMITX_VFMT_05_1920x1080i_60Hz */
    SCIO(E_REGVFMT_SCIN_480i_60Hz,      E_REGVFMT_SCOUT_INVALID),         /* HDMITX_VFMT_06_720x480i_60Hz   */
    SCIO(E_REGVFMT_SCIN_480i_60Hz,      E_REGVFMT_SCOUT_INVALID),         /* HDMITX_VFMT_07_720x480i_60Hz   */
    SCIO(E_REGVFMT_SCIN_INVALID,        E_REGVFMT_SCOUT_INVALID),         /* HDMITX_VFMT_08_720x240p_60Hz   */
    SCIO(E_REGVFMT_SCIN_INVALID,        E_REGVFMT_SCOUT_INVALID),         /* HDMITX_VFMT_09_720x240p_60Hz   */
    SCIO(E_REGVFMT_SCIN_480i_60Hz,      E_REGVFMT_SCOUT_INVALID),         /* HDMITX_VFMT_10_720x480i_60Hz   */
    SCIO(E_REGVFMT_SCIN_480i_60Hz,      E_REGVFMT_SCOUT_INVALID),         /* HDMITX_VFMT_11_720x480i_60Hz   */
    SCIO(E_REGVFMT_SCIN_INVALID,        E_REGVFMT_SCOUT_INVALID),         /* HDMITX_VFMT_12_720x240p_60Hz   */
    SCIO(E_REGVFMT_SCIN_INVALID,        E_REGVFMT_SCOUT_INVALID),         /* HDMITX_VFMT_13_720x240p_60Hz   */
    SCIO(E_REGVFMT_SCIN_480p_60Hz,      E_REGVFMT_SCOUT_INVALID),         /* HDMITX_VFMT_14_1440x480p_60Hz  */
    SCIO(E_REGVFMT_SCIN_480p_60Hz,      E_REGVFMT_SCOUT_INVALID),         /* HDMITX_VFMT_15_1440x480p_60Hz  */
    SCIO(E_REGVFMT_SCIN_INVALID,        E_REGVFMT_SCOUT_1080p_50Hz_60Hz), /* HDMITX_VFMT_16_1920x1080p_60Hz */
    SCIO(E_REGVFMT_SCIN_576p_50Hz,      E_REGVFMT_SCOUT_576ip_50Hz),      /* HDMITX_VFMT_17_720x576p_50Hz   */
    SCIO(E_REGVFMT_SCIN_576p_50Hz,      E_REGVFMT_SCOUT_576ip_50Hz),      /* HDMITX_VFMT_18_720x576p_50Hz   */
    SCIO(E_REGVFMT_SCIN_720p_50Hz_60Hz, E_REGVFMT_SCOUT_720p_50Hz_60Hz),  /* HDMITX_VFMT_19_1280x720p_50Hz  */
    SCIO(E_REGVFMT_SCIN_1080i_50Hz_60Hz,E_REGVFMT_SCOUT_1080i_50Hz_60Hz), /* HDMITX_VFMT_20_1920x1080i_50Hz */
    SCIO(E_REGVFMT_SCIN_576i_50Hz,      E_REGVFMT_SCOUT_576ip_50Hz),      /* HDMITX_VFMT_21_720x576i_50Hz   */
    SCIO(E_REGVFMT_SCIN_576i_50Hz,      E_REGVFMT_SCOUT_576ip_50Hz),      /* HDMITX_VFMT_22_720x576i_50Hz   */
    SCIO(E_REGVFMT_SCIN_INVALID,        E_REGVFMT_SCOUT_INVALID),         /* HDMITX_VFMT_23_720x288p_50Hz   */
    SCIO(E_REGVFMT_SCIN_INVALID,        E_REGVFMT_SCOUT_INVALID),         /* HDMITX_VFMT_24_720x288p_50Hz   */
    SCIO(E_REGVFMT_SCIN_576i_50Hz,      E_REGVFMT_SCOUT_INVALID),         /* HDMITX_VFMT_25_720x576i_50Hz   */
    SCIO(E_REGVFMT_SCIN_576i_50Hz,      E_REGVFMT_SCOUT_INVALID),         /* HDMITX_VFMT_26_720x576i_50Hz   */
    SCIO(E_REGVFMT_SCIN_INVALID,        E_REGVFMT_SCOUT_INVALID),         /* HDMITX_VFMT_27_720x288p_50Hz   */
    SCIO(E_REGVFMT_SCIN_INVALID,        E_REGVFMT_SCOUT_INVALID),         /* HDMITX_VFMT_28_720x288p_50Hz   */
    SCIO(E_REGVFMT_SCIN_576p_50Hz,      E_REGVFMT_SCOUT_INVALID),         /* HDMITX_VFMT_29_1440x576p_50Hz  */
    SCIO(E_REGVFMT_SCIN_576p_50Hz,      E_REGVFMT_SCOUT_INVALID),         /* HDMITX_VFMT_30_1440x576p_50Hz  */
    SCIO(E_REGVFMT_SCIN_INVALID,        E_REGVFMT_SCOUT_1080p_50Hz_60Hz),  /* HDMITX_VFMT_31_1920x1080p_50Hz */
    SCIO(E_REGVFMT_SCIN_INVALID,        E_REGVFMT_SCOUT_INVALID),         /* HDMITX_VFMT_32_1920x1080p_24Hz */
    SCIO(E_REGVFMT_SCIN_INVALID,        E_REGVFMT_SCOUT_INVALID),         /* HDMITX_VFMT_33_1920x1080p_25Hz */
    SCIO(E_REGVFMT_SCIN_INVALID,        E_REGVFMT_SCOUT_INVALID)          /* HDMITX_VFMT_34_1920x1080p_30Hz */

        
};

/**
 * Macro to pack output format flags as bits in a UInt16 for the following table
 */
#define PKOPF(p1,p2,p3,p4,p5,p6,p7,p8,p9,p10,p11,p12,p13,p14,p15,p16,p17) \
    ((p1)|((p2)<<1)|((p3)<<2)|((p4)<<3)|((p5)<<4)|((p6)<<5)|((p7)<<6)|((p8)<<7)| \
    ((p9)<<8)|((p10)<<9)|((p11)<<10)|((p12)<<11)|((p13)<<12)|((p14)<<13)|((p15)<<14)| \
    ((p16)<<15)|((p17)<<16))

/* Macro to test for input and output format = '1' in following table */
#define CAN_FMTS_SCALE(fin,fout) (kCanSclInToOut[(fin)]&(1<<(fout)))

/**
 * Lookup table to determine if an input format can be scaled to an
 * output format, including scaler off conditions (in=out)
 * Indexed by [input format][output format]
 */
static CONST_DAT UInt16 kCanSclInToOut[E_REGVFMT_INVALID+1] =
{   /* -----OUTPUT FORMATS------ */
    /* E_REGVFMT_640x480p_60Hz   */
    /*    E_REGVFMT_720x480p_60Hz   */
    /*       E_REGVFMT_1280x720p_60Hz  */
    /*          E_REGVFMT_1920x1080i_60Hz */
    /*             E_REGVFMT_720x480i_60Hz   */
    /*                E_REGVFMT_720x240p_60Hz   */
    /*                   E_REGVFMT_1920x1080p_60Hz */
    /*                      E_REGVFMT_720x576p_50Hz   */
    /*                         E_REGVFMT_1280x720p_50Hz  */
    /*                            E_REGVFMT_1920x1080i_50Hz */
    /*                               E_REGVFMT_720x576i_50Hz   */
    /*                                  E_REGVFMT_720x288p_50Hz   */
    /*                                     E_REGVFMT_1920x1080p_50Hz */
    /*                                        E_REGVFMT_1920x1080p_24Hz */
    /*                                           E_REGVFMT_1920x1080p_25Hz */
    /*                                              E_REGVFMT_1920x1080p_30Hz */    
    /*                                        E_REGVFMT_INVALID         */
                                                 /* ------INPUT FORMATS------ */
PKOPF( 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),/* E_REGVFMT_640x480p_60Hz   */
PKOPF( 0, 1, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),/* E_REGVFMT_720x480p_60Hz   */
PKOPF( 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),/* E_REGVFMT_1280x720p_60Hz  */
PKOPF( 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),/* E_REGVFMT_1920x1080i_60Hz */
PKOPF( 0, 1, 1, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),/* E_REGVFMT_720x480i_60Hz   */
PKOPF( 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),/* E_REGVFMT_720x240p_60Hz   */
PKOPF( 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),/* E_REGVFMT_1920x1080p_60Hz */
PKOPF( 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 0, 0),/* E_REGVFMT_720x576p_50Hz   */
PKOPF( 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0),/* E_REGVFMT_1280x720p_50Hz  */
PKOPF( 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0),/* E_REGVFMT_1920x1080i_50Hz */
PKOPF( 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 1, 0, 0, 0, 0),/* E_REGVFMT_720x576i_50Hz   */
PKOPF( 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0),/* E_REGVFMT_720x288p_50Hz   */
PKOPF( 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0),/* E_REGVFMT_1920x1080p_50Hz */
PKOPF( 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0),/* E_REGVFMT_1920x1080p_24Hz */
PKOPF( 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0),/* E_REGVFMT_1920x1080p_25Hz */
PKOPF( 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0),/* E_REGVFMT_1920x1080p_30Hz */
PKOPF( 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) /* E_REGVFMT_INVALID         */
};

/**
 * Lookup tables to convert scaler input and output modes
 * to PLL scaling values
 */
static CONST_DAT UInt16 kSclModeToScgN[E_REGVFMT_SCIN_NUM][E_REGVFMT_SCOUT_NUM] =
{ /* E_REGVFMT_SCOUT_480p_60Hz */
  /*        E_REGVFMT_SCOUT_576ip_50Hz */
  /*                E_REGVFMT_SCOUT_720p_50Hz_60Hz */
  /*                        E_REGVFMT_SCOUT_1080i_50Hz_60Hz */
  /*                               E_REGVFMT_SCOUT_1080p_50Hz_60Hz */
    { 20,     0,    500,    500,   1000},   /* E_REGVFMT_SCIN_480i_60Hz */
    {  0,    20,     88,     88,    176},   /* E_REGVFMT_SCIN_576i_50Hz */
    {  0,     0,    250,    250,    500},   /* E_REGVFMT_SCIN_480p_60Hz */
    {  0,     0,     44,     44,     88},   /* E_REGVFMT_SCIN_576p_50Hz */
    {  0,     0,      0,      0,     20},   /* E_REGVFMT_SCIN_720p_50Hz_60Hz */
    {  0,     0,      0,      0,     20}   /* E_REGVFMT_SCIN_1080i_50Hz_60Hz */
};
static CONST_DAT UInt16 kSclModeToScgR[E_REGVFMT_SCIN_NUM][E_REGVFMT_SCOUT_NUM] =
{ /* E_REGVFMT_SCOUT_480p_60Hz */
  /*        E_REGVFMT_SCOUT_576ip_50Hz */
  /*                E_REGVFMT_SCOUT_720p_50Hz_60Hz */
  /*                        E_REGVFMT_SCOUT_1080i_50Hz_60Hz */
    { 10,     0,     91,     91,    91},   /* E_REGVFMT_SCIN_480i_60Hz */
    {  0,    10,     16,     16,    16},   /* E_REGVFMT_SCIN_576i_50Hz */
    {  0,     0,     91,     91,    91},   /* E_REGVFMT_SCIN_480p_60Hz */
    {  0,     0,     16,     16,    16},   /* E_REGVFMT_SCIN_576p_50Hz */
    {  0,     0,      0,      0,    10},   /* E_REGVFMT_SCIN_720p_50Hz_60Hz */
    {  0,     0,      0,      0,    10}   /* E_REGVFMT_SCIN_1080i_50Hz_60Hz */
};


/**
 * Lookup table to convert from video format codes used in the 
 * E_REG_P00_VIDFORMAT_W register to corresponding VS_PIX_STRT_2
 * register values, to correct the output window for interlaced
 * output formats, with or without the scaler.
 *
 * The correction is VS_PIX_STRT_2=VS_PIX_STRT_2+VS_PIX_STRT_1.
 * The same value is also applied to VS_PIX_END_2.
 */
static CONST_DAT UInt16 kRegvfmtToVs2[E_REGVFMT_NUM] =
{
    0,            /* E_REGVFMT_640x480p_60Hz   */
    0,            /* E_REGVFMT_720x480p_60Hz   */
    0,            /* E_REGVFMT_1280x720p_60Hz  */
    1100 + 88,    /* E_REGVFMT_1920x1080i_60Hz */
    429  + 19,    /* E_REGVFMT_720x480i_60Hz   */
    0,            /* E_REGVFMT_720x240p_60Hz   */
    0,            /* E_REGVFMT_1920x1080p_60Hz */
    0,            /* E_REGVFMT_720x576p_50Hz   */
    0,            /* E_REGVFMT_1280x720p_50Hz  */
    1320 + 528,   /* E_REGVFMT_1920x1080i_50Hz */
    432  + 12,    /* E_REGVFMT_720x576i_50Hz   */
    0,            /* E_REGVFMT_720x288p_50Hz   */
    0,            /* E_REGVFMT_1920x1080p_50Hz */
    0,            /* E_REGVFMT_1920x1080p_24Hz */
    0,            /* E_REGVFMT_1920x1080p_25Hz */
    0             /* E_REGVFMT_1920x1080p_30Hz */
#ifdef FORMAT_PC
   ,0, /* E_REGVFMT_640x480p_72Hz   */
    0, /* E_REGVFMT_640x480p_75Hz   */
    0, /* E_REGVFMT_640x480p_85Hz   */
    0, /* E_REGVFMT_800x600p_60Hz   */
    0, /* E_REGVFMT_800x600p_72Hz   */
    0, /* E_REGVFMT_800x600p_75Hz   */
    0, /* E_REGVFMT_800x600p_85Hz   */
    0, /* E_REGVFMT_1024x768p_60Hz  */
    0, /* E_REGVFMT_1024x768p_70Hz  */
    0, /* E_REGVFMT_1024x768p_75Hz  */
    0, /* E_REGVFMT_1280x768p_60Hz  */
    0, /* E_REGVFMT_1280x1024p_60Hz */
    0, /* E_REGVFMT_1360x768p_60Hz  */
    0, /* E_REGVFMT_1400x1050p_60Hz */
    0, /* E_REGVFMT_1600x1200p_60Hz */
    0  /* E_REGVFMT_1280x1024p_85Hz */
#endif /* FORMAT_PC */
};

/**
 * Lookup table to convert from video format codes used in the 
 * E_REG_P00_VIDFORMAT_W register to corresponding 
 * pixel repetition values in the PLL_SERIAL_2 register.
 * 0=no repetition (pixel sent once)
 * 1=one repetition (pixel sent twice) etc
 */
static CONST_DAT UInt8 kRegvfmtToPixRep[E_REGVFMT_NUM] =
{
    0, /* E_REGVFMT_640x480p_60Hz   */
    0, /* E_REGVFMT_720x480p_60Hz   */
    0, /* E_REGVFMT_1280x720p_60Hz  */
    0, /* E_REGVFMT_1920x1080i_60Hz */
    1, /* E_REGVFMT_720x480i_60Hz   */
    1, /* E_REGVFMT_720x240p_60Hz   */
    0, /* E_REGVFMT_1920x1080p_60Hz */
    0, /* E_REGVFMT_720x576p_50Hz   */
    0, /* E_REGVFMT_1280x720p_50Hz  */
    0, /* E_REGVFMT_1920x1080i_50Hz */
    1, /* E_REGVFMT_720x576i_50Hz   */
    1, /* E_REGVFMT_720x288p_50Hz   */
    0, /* E_REGVFMT_1920x1080p_50Hz */
    0, /* E_REGVFMT_1920x1080p_24Hz */
    0, /* E_REGVFMT_1920x1080p_25Hz */
    0  /* E_REGVFMT_1920x1080p_30Hz */
#ifdef FORMAT_PC
   ,0, /* E_REGVFMT_640x480p_72Hz   */
    0, /* E_REGVFMT_640x480p_75Hz   */
    0, /* E_REGVFMT_640x480p_85Hz   */
    0, /* E_REGVFMT_800x600p_60Hz   */
    0, /* E_REGVFMT_800x600p_72Hz   */
    0, /* E_REGVFMT_800x600p_75Hz   */
    0, /* E_REGVFMT_800x600p_85Hz   */
    0, /* E_REGVFMT_1024x768p_60Hz  */
    0, /* E_REGVFMT_1024x768p_70Hz  */
    0, /* E_REGVFMT_1024x768p_75Hz  */
    0, /* E_REGVFMT_1280x768p_60Hz  */
    0, /* E_REGVFMT_1280x1024p_60Hz */
    0, /* E_REGVFMT_1360x768p_60Hz  */
    0, /* E_REGVFMT_1400x1050p_60Hz */
    0, /* E_REGVFMT_1600x1200p_60Hz */
    0  /* E_REGVFMT_1280x1024p_85Hz */
#endif /* FORMAT_PC */

};

/**
 * Lookup table to convert from video format codes used in the 
 * E_REG_P00_VIDFORMAT_W register to corresponding 
 * trios of 2-bit values in the srl_nosc, scg_nosc and de_nosc
 * PLL control registers
 */
#define SSD(srl,de) (((srl)<<4)|(de))
#define SSD2SRL(ssd)    (UInt8)(((ssd)>>4)&3)
#define SSD2DE(ssd)     (UInt8)((ssd)&3)


static CONST_DAT struct
{
    UInt8 ScaOffCcirOffDblEdgeOff[E_REGVFMT_NUM_TV];
                
    UInt8 ScaOffCcirOnDblEdgeOff[E_REGVFMT_NUM_TV];

    UInt8 ScaOffCcirOnDblEdgeOn[E_REGVFMT_NUM_TV];

    UInt8 ScaOnCcirOffDblEdgeOffInterlace[E_REGVFMT_NUM_TV + 1];
        
    UInt8 ScaOnCcirOnDblEdgeOffInterlace[E_REGVFMT_NUM_TV];

    UInt8 ScaOnCcirOnDblEdgeOnInterlace[E_REGVFMT_NUM_TV];

    UInt8 ScaOnCcirOffDblEdgeOffProgressif[E_REGVFMT_NUM_TV + 1];

    UInt8 ScaOnCcirOnDblEdgeOffProgressif[E_REGVFMT_NUM_TV];

    UInt8 ScaOnCcirOnDblEdgeOnProgressif[E_REGVFMT_NUM_TV];

#ifdef FORMAT_PC
    UInt8 SettingsFormatPC[E_REGVFMT_NUM_PC];
#endif /* FORMAT_PC */
    
}kRegvfmtToPllSsd = {

    {   /*ScaOffCCIROffDblEdgeOff*/
        /* SRL,DE */
        SSD(2,2), /* E_REGVFMT_640x480p_60Hz   */
        SSD(2,2), /* E_REGVFMT_720x480p_60Hz   */
        SSD(1,1), /* E_REGVFMT_1280x720p_60Hz  */
        SSD(1,1), /* E_REGVFMT_1920x1080i_60Hz */
        SSD(3,3), /* E_REGVFMT_720x480i_60Hz   */
        SSD(0xF,0), /* E_REGVFMT_720x240p_60Hz   */ /** \todo Need nosc PLL value */
        SSD(0,0), /* E_REGVFMT_1920x1080p_60Hz */
        SSD(2,2), /* E_REGVFMT_720x576p_50Hz   */
        SSD(1,1), /* E_REGVFMT_1280x720p_50Hz  */
        SSD(1,1), /* E_REGVFMT_1920x1080i_50Hz */
        SSD(3,3), /* E_REGVFMT_720x576i_50Hz   */
        SSD(0xF,0), /* E_REGVFMT_720x288p_50Hz   */ /** \todo Need nosc PLL value */
        SSD(0,0),  /* E_REGVFMT_1920x1080p_50Hz */
        SSD(1,1),  /* E_REGVFMT_1920x1080p_24Hz */
        SSD(1,1),  /* E_REGVFMT_1920x1080p_25Hz */
        SSD(1,1)   /* E_REGVFMT_1920x1080p_30Hz */
            
    },

    {   /*ScaOffCCIROnDblEdgeOff*/
        /* SRL,DE */
        SSD(0xF,0), /* E_REGVFMT_640x480p_60Hz   */
        SSD(2,1), /* E_REGVFMT_720x480p_60Hz   */
        SSD(0xF,0), /* E_REGVFMT_1280x720p_60Hz  */
        SSD(0xF,0), /* E_REGVFMT_1920x1080i_60Hz */
        SSD(2,2), /* E_REGVFMT_720x480i_60Hz   */
        SSD(0xF,0), /* E_REGVFMT_720x240p_60Hz   */ /** \todo Need nosc PLL value */
        SSD(0xF,0), /* E_REGVFMT_1920x1080p_60Hz */
        SSD(2,1), /* E_REGVFMT_720x576p_50Hz   */
        SSD(0xF,0), /* E_REGVFMT_1280x720p_50Hz  */
        SSD(0xF,0), /* E_REGVFMT_1920x1080i_50Hz */
        SSD(2,2), /* E_REGVFMT_720x576i_50Hz   */
        SSD(0xF,0), /* E_REGVFMT_720x288p_50Hz   */ /** \todo Need nosc PLL value */
        SSD(0xF,0), /* E_REGVFMT_1920x1080p_50Hz */
        SSD(0xF,0),  /* E_REGVFMT_1920x1080p_24Hz */
        SSD(0xF,0),  /* E_REGVFMT_1920x1080p_25Hz */
        SSD(0xF,0)   /* E_REGVFMT_1920x1080p_30Hz */        
    },

    {   /*ScaOffCcirONDblEdgeOn*/
        /* SRL,DE */
        SSD(0xF,0), /* E_REGVFMT_640x480p_60Hz   */
        SSD(2,1), /* E_REGVFMT_720x480p_60Hz   */
        SSD(1,1), /* E_REGVFMT_1280x720p_60Hz  */ /*To be Update for TDA9989*/
        SSD(1,1), /* E_REGVFMT_1920x1080i_60Hz */ /*To be Update for TDA9989*/
        SSD(2,2), /* E_REGVFMT_720x480i_60Hz   */
        SSD(0xF,0), /* E_REGVFMT_720x240p_60Hz   */ /** \todo Need nosc PLL value */
        SSD(1,1), /* E_REGVFMT_1920x1080p_60Hz */ /*To be Update for TDA9989*/
        SSD(2,1), /* E_REGVFMT_720x576p_50Hz   */
        SSD(1,1), /* E_REGVFMT_1280x720p_50Hz  */ /*To be Update for TDA9989*/
        SSD(1,1), /* E_REGVFMT_1920x1080i_50Hz */ /*To be Update for TDA9989*/
        SSD(2,2), /* E_REGVFMT_720x576i_50Hz   */
        SSD(0xF,0), /* E_REGVFMT_720x288p_50Hz   */ /** \todo Need nosc PLL value */
        SSD(1,1),  /* E_REGVFMT_1920x1080p_50Hz */ /*To be Update for TDA9989*/
        SSD(0xF,0),  /* E_REGVFMT_1920x1080p_24Hz */
        SSD(0xF,0),  /* E_REGVFMT_1920x1080p_25Hz */
        SSD(0xF,0)   /* E_REGVFMT_1920x1080p_30Hz */
    },

    {   /* ScaOnCcirOffDblEdgeOffInterlace */
        /* SRL,DE */
        SSD(0xF,0), /* E_REGVFMT_640x480p_60Hz   */
        SSD(2,3), /* E_REGVFMT_720x480p_60Hz   */
        SSD(1,3), /* E_REGVFMT_1280x720p_60Hz  */
        SSD(1,3), /* E_REGVFMT_1920x1080i_60Hz */
        SSD(0xF,0), /* E_REGVFMT_720x480i_60Hz   */
        SSD(0xF,0), /* E_REGVFMT_720x240p_60Hz   */ /** \todo Need nosc PLL value */
        SSD(0,3), /* E_REGVFMT_1920x1080p_60Hz */
        SSD(2,3), /* E_REGVFMT_720x576p_50Hz   */
        SSD(1,3), /* E_REGVFMT_1280x720p_50Hz  */
        SSD(1,3), /* E_REGVFMT_1920x1080i_50Hz */
        SSD(0xF,0), /* E_REGVFMT_720x576i_50Hz   */
        SSD(0xF,0), /* E_REGVFMT_720x288p_50Hz   */ /** \todo Need nosc PLL value */
        SSD(0,3),  /* E_REGVFMT_1920x1080p_50Hz */
        SSD(0,1),  /* E_REGVFMT_1920x1080p_50Hz_60Hz vinfmt = 720p or 1080i */
        SSD(1,3),  /* E_REGVFMT_1920x1080p_24Hz */
        SSD(1,3),  /* E_REGVFMT_1920x1080p_25Hz */
        SSD(1,3)   /* E_REGVFMT_1920x1080p_30Hz */            
    },

    {   /* ScaOnCcirONDblEdgeOffInterlace */
        /* SRL,DE */
        SSD(0xF,0), /* E_REGVFMT_640x480p_60Hz   */
        SSD(2,2), /* E_REGVFMT_720x480p_60Hz   */
        SSD(1,2), /* E_REGVFMT_1280x720p_60Hz  */
        SSD(1,2), /* E_REGVFMT_1920x1080i_60Hz */
        SSD(0xF,0), /* E_REGVFMT_720x480i_60Hz   */
        SSD(0xF,0), /* E_REGVFMT_720x240p_60Hz   */ /** \todo Need nosc PLL value */
        SSD(0,2), /* E_REGVFMT_1920x1080p_60Hz */
        SSD(2,2), /* E_REGVFMT_720x576p_50Hz   */
        SSD(1,2), /* E_REGVFMT_1280x720p_50Hz  */
        SSD(1,2), /* E_REGVFMT_1920x1080i_50Hz */
        SSD(0xF,0), /* E_REGVFMT_720x576i_50Hz   */
        SSD(0xF,0), /* E_REGVFMT_720x288p_50Hz   */ /** \todo Need nosc PLL value */
        SSD(0,2), /* E_REGVFMT_1920x1080p_50Hz */
        SSD(1,2),  /* E_REGVFMT_1920x1080p_24Hz */
        SSD(1,2),  /* E_REGVFMT_1920x1080p_25Hz */
        SSD(1,2)  /* E_REGVFMT_1920x1080p_30Hz */        
    },

    {  /* ScaOnCcirONDblEdgeOnInterlace */
        /* SRL,DE */
        SSD(0xF,0), /* E_REGVFMT_640x480p_60Hz   */
        SSD(2,2), /* E_REGVFMT_720x480p_60Hz   */
        SSD(1,2), /* E_REGVFMT_1280x720p_60Hz  */
        SSD(1,2), /* E_REGVFMT_1920x1080i_60Hz */
        SSD(0xF,0), /* E_REGVFMT_720x480i_60Hz   */
        SSD(0xF,0), /* E_REGVFMT_720x240p_60Hz   */ /** \todo Need nosc PLL value */
        SSD(0,2), /* E_REGVFMT_1920x1080p_60Hz */
        SSD(2,2), /* E_REGVFMT_720x576p_50Hz   */
        SSD(1,2), /* E_REGVFMT_1280x720p_50Hz  */
        SSD(1,2), /* E_REGVFMT_1920x1080i_50Hz */
        SSD(0xF,0), /* E_REGVFMT_720x576i_50Hz   */
        SSD(0xF,0), /* E_REGVFMT_720x288p_50Hz   */ /** \todo Need nosc PLL value */
        SSD(0,2), /* E_REGVFMT_1920x1080p_50Hz */
        SSD(1,2),  /* E_REGVFMT_1920x1080p_24Hz */
        SSD(1,2),  /* E_REGVFMT_1920x1080p_25Hz */
        SSD(1,2)  /* E_REGVFMT_1920x1080p_30Hz */            
    },

    { /* ScaOnCcirOffDblEdgeOffProgressif */
        /* SRL,DE */
        SSD(0xF,0), /* E_REGVFMT_640x480p_60Hz   */
        SSD(0xF,0), /* E_REGVFMT_720x480p_60Hz   */
        SSD(1,2), /* E_REGVFMT_1280x720p_60Hz  */
        SSD(1,2), /* E_REGVFMT_1920x1080i_60Hz */
        SSD(0xF,0), /* E_REGVFMT_720x480i_60Hz   */
        SSD(0xF,0), /* E_REGVFMT_720x240p_60Hz   */ /** \todo Need nosc PLL value */
        SSD(0,2), /* E_REGVFMT_1920x1080p_60Hz */
        SSD(0xF,0), /* E_REGVFMT_720x576p_50Hz   */
        SSD(1,2), /* E_REGVFMT_1280x720p_50Hz  */
        SSD(1,2), /* E_REGVFMT_1920x1080i_50Hz */
        SSD(0xF,0), /* E_REGVFMT_720x576i_50Hz   */
        SSD(0xF,0), /* E_REGVFMT_720x288p_50Hz   */ /** \todo Need nosc PLL value */
        SSD(0,2),  /* E_REGVFMT_1920x1080p_50Hz */
        SSD(0,1),  /* E_REGVFMT_1920x1080p_50Hz_60Hz vinfmt = 720p or 1080i */
        SSD(1,2),  /* E_REGVFMT_1920x1080p_24Hz */
        SSD(1,2),  /* E_REGVFMT_1920x1080p_25Hz */
        SSD(1,2)  /* E_REGVFMT_1920x1080p_30Hz */        
    },

    {   /* ScaOnCcirOnDblEdgeOffProgressif */
        /* SRL,DE */
        SSD(0xF,0), /* E_REGVFMT_640x480p_60Hz   */
        SSD(0xF,0), /* E_REGVFMT_720x480p_60Hz   */
        SSD(1,1), /* E_REGVFMT_1280x720p_60Hz  */
        SSD(1,1), /* E_REGVFMT_1920x1080i_60Hz */
        SSD(0xF,0), /* E_REGVFMT_720x480i_60Hz   */
        SSD(0xF,0), /* E_REGVFMT_720x240p_60Hz   */ /** \todo Need nosc PLL value */
        SSD(0,1), /* E_REGVFMT_1920x1080p_60Hz */
        SSD(0xF,0), /* E_REGVFMT_720x576p_50Hz   */
        SSD(1,1), /* E_REGVFMT_1280x720p_50Hz  */
        SSD(1,1), /* E_REGVFMT_1920x1080i_50Hz */
        SSD(0xF,0), /* E_REGVFMT_720x576i_50Hz   */
        SSD(0xF,0), /* E_REGVFMT_720x288p_50Hz   */ /** \todo Need nosc PLL value */
        SSD(0,1), /* E_REGVFMT_1920x1080p_50Hz */
        SSD(1,1),  /* E_REGVFMT_1920x1080p_24Hz */
        SSD(1,1),  /* E_REGVFMT_1920x1080p_25Hz */
        SSD(1,1)  /* E_REGVFMT_1920x1080p_30Hz */        
    },

    {   /* ScaOnCcirONDblEdgeOnProgressif */
        /* SRL,DE */
        SSD(0xF,0), /* E_REGVFMT_640x480p_60Hz   */
        SSD(0xF,0), /* E_REGVFMT_720x480p_60Hz   */
        SSD(1,1), /* E_REGVFMT_1280x720p_60Hz  */
        SSD(1,1), /* E_REGVFMT_1920x1080i_60Hz */
        SSD(0xF,0), /* E_REGVFMT_720x480i_60Hz   */
        SSD(0xF,0), /* E_REGVFMT_720x240p_60Hz   */ /** \todo Need nosc PLL value */
        SSD(0,1), /* E_REGVFMT_1920x1080p_60Hz */
        SSD(0xF,0), /* E_REGVFMT_720x576p_50Hz   */
        SSD(1,1), /* E_REGVFMT_1280x720p_50Hz  */
        SSD(1,1), /* E_REGVFMT_1920x1080i_50Hz */
        SSD(0xF,0), /* E_REGVFMT_720x576i_50Hz   */
        SSD(0xF,0), /* E_REGVFMT_720x288p_50Hz   */ /** \todo Need nosc PLL value */
        SSD(0,1),  /* E_REGVFMT_1920x1080p_50Hz */
        SSD(1,1),  /* E_REGVFMT_1920x1080p_24Hz */
        SSD(1,1),  /* E_REGVFMT_1920x1080p_25Hz */
        SSD(1,1)  /* E_REGVFMT_1920x1080p_30Hz */            
    }
    
#ifdef FORMAT_PC
  ,{
        /* SRL,DE */
        SSD(2,0),   /* E_REGVFMT_640x480p_72Hz   */
        SSD(2,0),   /* E_REGVFMT_640x480p_75Hz   */
        SSD(2,0),   /* E_REGVFMT_640x480p_85Hz   */
        SSD(1,0),   /* E_REGVFMT_800x600p_60Hz   */
        SSD(1,0),   /* E_REGVFMT_800x600p_72Hz   */
        SSD(1,0),   /* E_REGVFMT_800x600p_75Hz   */
        SSD(1,0),   /* E_REGVFMT_800x600p_85Hz   */
        SSD(1,0),   /* E_REGVFMT_1024x768p_60Hz  */
        SSD(1,0),   /* E_REGVFMT_1024x768p_70Hz  */
        SSD(1,0),   /* E_REGVFMT_1024x768p_75Hz  */
        SSD(0,0),   /* E_REGVFMT_1280x768p_60Hz  */
        SSD(0,0),   /* E_REGVFMT_1280x1024p_60Hz */
        SSD(0,0),   /* E_REGVFMT_1360x768p_60Hz  */
        SSD(0,0),   /* E_REGVFMT_1400x1050p_60Hz */
        SSD(0,0)    /* E_REGVFMT_1600x1200p_60Hz */
  }
#endif /* FORMAT_PC */

    

};

/**
 * Lookup table to convert from video format codes used in the 
 * E_REG_P00_VIDFORMAT_W register to corresponding 
 * V_toggle values for Vs external sync.
 * 0=no toggle
 * 1=toggle
 */
static CONST_DAT UInt8 kRegvfmtToVToggle[E_REGVFMT_NUM] =
{
    1, /* E_REGVFMT_640x480p_60Hz   */
    1, /* E_REGVFMT_720x480p_60Hz   */
    0, /* E_REGVFMT_1280x720p_60Hz  */
    0, /* E_REGVFMT_1920x1080i_60Hz */
    1, /* E_REGVFMT_720x480i_60Hz   */
    1, /* E_REGVFMT_720x240p_60Hz   */
    0, /* E_REGVFMT_1920x1080p_60Hz */
    1, /* E_REGVFMT_720x576p_50Hz   */
    0, /* E_REGVFMT_1280x720p_50Hz  */
    0, /* E_REGVFMT_1920x1080i_50Hz */
    1, /* E_REGVFMT_720x576i_50Hz   */
    1, /* E_REGVFMT_720x288p_50Hz   */
    0,  /* E_REGVFMT_1920x1080p_50Hz */
    0, /* E_REGVFMT_1920x1080p_24Hz */
    0, /* E_REGVFMT_1920x1080p_25Hz */
    0  /* E_REGVFMT_1920x1080p_30Hz */
#ifdef FORMAT_PC
   ,1, /* E_REGVFMT_640x480p_72Hz   */
    1, /* E_REGVFMT_640x480p_75Hz   */
    1, /* E_REGVFMT_640x480p_85Hz   */
    0, /* E_REGVFMT_800x600p_60Hz   */
    0, /* E_REGVFMT_800x600p_72Hz   */
    0, /* E_REGVFMT_800x600p_75Hz   */
    0, /* E_REGVFMT_800x600p_85Hz   */
    1, /* E_REGVFMT_1024x768p_60Hz  */
    1, /* E_REGVFMT_1024x768p_70Hz  */
    0, /* E_REGVFMT_1024x768p_75Hz  */
    0, /* E_REGVFMT_1280x768p_60Hz  */
    0, /* E_REGVFMT_1280x1024p_60Hz */
    0, /* E_REGVFMT_1360x768p_60Hz  */
    0, /* E_REGVFMT_1400x1050p_60Hz */
    0, /* E_REGVFMT_1600x1200p_60Hz */
    0  /* E_REGVFMT_1280x1024p_85Hz */
#endif /* FORMAT_PC */
};

/**
 * Lookup table to convert from video format codes used in the 
 * E_REG_P00_VIDFORMAT_W register to corresponding 
 * H_toggle values for Vs external sync.
 * 0=no toggle
 * 1=toggle
 */
static CONST_DAT UInt8 kRegvfmtToHToggle[E_REGVFMT_NUM] =
{
    1, /* E_REGVFMT_640x480p_60Hz   */
    1, /* E_REGVFMT_720x480p_60Hz   */
    0, /* E_REGVFMT_1280x720p_60Hz  */
    0, /* E_REGVFMT_1920x1080i_60Hz */
    1, /* E_REGVFMT_720x480i_60Hz   */
    1, /* E_REGVFMT_720x240p_60Hz   */
    0, /* E_REGVFMT_1920x1080p_60Hz */
    1, /* E_REGVFMT_720x576p_50Hz   */
    0, /* E_REGVFMT_1280x720p_50Hz  */
    0, /* E_REGVFMT_1920x1080i_50Hz */
    1, /* E_REGVFMT_720x576i_50Hz   */
    1, /* E_REGVFMT_720x288p_50Hz   */
    0,  /* E_REGVFMT_1920x1080p_50Hz */
    0, /* E_REGVFMT_1920x1080p_24Hz */
    0, /* E_REGVFMT_1920x1080p_25Hz */
    0  /* E_REGVFMT_1920x1080p_30Hz */
#ifdef FORMAT_PC
   ,1, /* E_REGVFMT_640x480p_72Hz   */
    1, /* E_REGVFMT_640x480p_75Hz   */
    1, /* E_REGVFMT_640x480p_85Hz   */
    0, /* E_REGVFMT_800x600p_60Hz   */
    0, /* E_REGVFMT_800x600p_72Hz   */
    0, /* E_REGVFMT_800x600p_75Hz   */
    0, /* E_REGVFMT_800x600p_85Hz   */
    1, /* E_REGVFMT_1024x768p_60Hz  */
    1, /* E_REGVFMT_1024x768p_70Hz  */
    0, /* E_REGVFMT_1024x768p_75Hz  */
    1, /* E_REGVFMT_1280x768p_60Hz  */
    0, /* E_REGVFMT_1280x1024p_60Hz */
    0, /* E_REGVFMT_1360x768p_60Hz  */
    1, /* E_REGVFMT_1400x1050p_60Hz */
    0, /* E_REGVFMT_1600x1200p_60Hz */
    0  /* E_REGVFMT_1280x1024p_85Hz */
#endif /* FORMAT_PC */
};

/**
 * Lokup table to convert from video format codes used in the 
 * E_REG_P00_VIDFORMAT_W register to RefPix and RefLine values
 * according to sync source
 */
static CONST_DAT struct
{
    UInt16  refPixVsSync;    /* Output values for Vs/Hs input sync */
    UInt16  refLineVsSync;
    UInt16  refPixOtherSync; /* Output values for all other input sync sources */
    UInt16  refLineOtherSync;
} kVidFmtToRefpixRefline[E_REGVFMT_NUM] =
{
/*  PixVs   LineVs  PixOther    LineOther   vinFmt */
    {17,    2,      161,        36},     /* E_REGVFMT_640x480p_60Hz   */
    {17,    8,      139,        43},     /* E_REGVFMT_720x480p_60Hz   */
    {111,   2,      371,        26},     /* E_REGVFMT_1280x720p_60Hz  */
    {89,    2,      281,        21},     /* E_REGVFMT_1920x1080i_60Hz */
    {20,    5,      139,        22},     /* E_REGVFMT_720x480i_60Hz   */
    {20,    5,      139,        22},     /* E_REGVFMT_720x240p_60Hz   */
    {89,    2,      281,        42},     /* E_REGVFMT_1920x1080p_60Hz */
    {13,    2,      145,        45},     /* E_REGVFMT_720x576p_50Hz   */
    {441,   2,      701,        26},     /* E_REGVFMT_1280x720p_50Hz  */
    {529,   2,      721,        21},     /* E_REGVFMT_1920x1080i_50Hz */
    {13,    2,      145,        23},     /* E_REGVFMT_720x576i_50Hz   */
    {13,    2,      145,        23},     /* E_REGVFMT_720x288p_50Hz   */
    {529,   2,      721,        42},      /* E_REGVFMT_1920x1080p_50Hz */
    {639,   2,      831,        42},     /* E_REGVFMT_1920x1080p_24Hz */
    {529,   2,      721,        42},     /* E_REGVFMT_1920x1080p_25Hz */
    {89,    2,      281,        42}      /* E_REGVFMT_1920x1080p_30Hz */    
#ifdef FORMAT_PC
   ,{25,    2,      195,        32}, /* E_REGVFMT_640x480p_72Hz   */
    {17,    2,      203,        20}, /* E_REGVFMT_640x480p_75Hz   */
    {57,    2,      195,        29}, /* E_REGVFMT_640x480p_85Hz   */
    {41,    2,      259,        28}, /* E_REGVFMT_800x600p_60Hz   */
    {57,    2,      243,        30}, /* E_REGVFMT_800x600p_72Hz   */
    {17,    2,      259,        25}, /* E_REGVFMT_800x600p_75Hz   */
    {33,    2,      251,        31}, /* E_REGVFMT_800x600p_85Hz   */
    {25,    2,      323,        36}, /* E_REGVFMT_1024x768p_60Hz  */
    {25,    2,      307,        36}, /* E_REGVFMT_1024x768p_70Hz  */
    {17,    2,      291,        32}, /* E_REGVFMT_1024x768p_75Hz  */
    {65,    2,      387,        28}, /* E_REGVFMT_1280x768p_60Hz */
    {49,    2,      411,        42}, /* E_REGVFMT_1280x1024p_60Hz */
    {65,    2,      435,        25}, /* E_REGVFMT_1360x768p_60Hz */
    {89,    2,      467,        37}, /* E_REGVFMT_1400x1050p_60Hz */
    {65,    2,      563,        50}, /* E_REGVFMT_1600x1200p_60Hz */
    {65,    2,      451,        48}  /* E_REGVFMT_1280x1024p_85Hz */
#endif/* FORMAT_PC */
};

static CONST_DAT tmHdmiTxVidReg_t kVidFmtFormatParam[E_REGVFMT_NUM_TV_NO_REG] =
{
/*  NPIX    NLINE  VsLineStart  VsPixStart  VsLineEnd   VsPixEnd    HsStart     HsEnd   vWinStart   vWinEnd DeStart DeEnd */
    {2750,  1125,     1,          638,         6,           638,        638,    682,    41,         1121,   830,   2750},/* E_REGVFMT_1920x1080p_24Hz */
    {2640,  1125,     1,          528,         6,           528,        528,    572,    41,         1121,   720,   2640},/* E_REGVFMT_1920x1080p_25Hz */
    {2200,  1125,     1,          88,          6,           88,         88,     132,    41,         1121,   280,   2200},/* E_REGVFMT_1920x1080p_30Hz */

};

#ifdef FORMAT_PC
static CONST_DAT tmHdmiTxVidReg_t kVidFmtToPCFormatParam[E_REGVFMT_NUM_PC] =
{
/*  NPIX    NLINE  VsLineStart  VsPixStart  VsLineEnd   VsPixEnd    HsStart HsEnd   vWinStart   vWinEnd DeStart DeEnd */
    {832,   520,      1,          24,          4,           24,         24,     64,     31,         511,    192,    832},/* E_REGVFMT_640x480p_72Hz   */
    {840,   500,      1,          16,          4,           16,         16,     80,     19,         499,    200,    840},/* E_REGVFMT_640x480p_75Hz   */
    {832,   509,      1,          56,          4,           56,         56,     112,    28,         508,    192,    832},/* E_REGVFMT_640x480p_85Hz   */
    {1056,  628,      1,          40,          5,           40,         40,     168,    27,         627,    256,   1056},/* E_REGVFMT_800x600p_60Hz   */
    {1040,  666,      1,          56,          7,           56,         56,     176,    29,         619,    240,   1040},/* E_REGVFMT_800x600p_72Hz   */
    {1056,  625,      1,          16,          4,           16,         16,     96,     24,         624,    256,   1056},/* E_REGVFMT_800x600p_75Hz   */
    {1048,  631,      1,          32,          4,           32,         32,     96,     30,         630,    248,   1048},/* E_REGVFMT_800x600p_85Hz   */
    {1344,  806,      1,          24,          7,           24,         24,     160,    35,         803,    320,   1344},/* E_REGVFMT_1024x768p_60Hz  */
    {1328,  806,      1,          24,          7,           24,         24,     160,    35,         803,    304,   1328},/* E_REGVFMT_1024x768p_70Hz  */
    {1312,  800,      1,          16,          4,           16,         16,     112,    31,         799,    288,   1312},/* E_REGVFMT_1024x768p_75Hz  */
    {1664,  798,      1,          64,          8,           64,         64,     192,    27,         795,    384,   1664},/* E_REGVFMT_1280x768p_60Hz  */
    {1688,  1066,     1,          48,          4,           48,         48,     160,    41,         1065,   408,   1688},/* E_REGVFMT_1280x1024p_60Hz */
    {1792,  795,      1,          64,          7,           64,         64,     176,    24,         792,    432,   1792},/* E_REGVFMT_1360x768p_60Hz  */
    {1864,  1089,     1,          88,          5,           88,         88,     232,    36,         1086,   464,   1864},/* E_REGVFMT_1400x1050p_60Hz */
    {2160,  1250,     1,          64,          4,           64,         64,     256,    49,         1249,   560,   2160},/* E_REGVFMT_1600x1200p_60Hz */
    {1728,  1072,     1,          64,          4,           64,         64,     224,    47,         1071,   448,   1728} /* E_REGVFMT_1280x1024p_85Hz */
};
#endif/* FORMAT_PC */

 /**
 *  Lookup table for each pixel clock frequency's CTS value in kHz
 *  according to SCS table "Audio Clock Recovery CTS Values"
 */
static CONST_DAT UInt32 kPixClkToAcrCts[E_PIXCLK_NUM][HDMITX_AFS_NUM] =
{
 /* HDMITX_AFS_32k  _AFS_48K       _AFS_96K        _AFS_192K */
 /*         _AFS_44_1k      _AFS_88_2K      _AFS_176_4K       */
   { 28125,  31250,  28125,  31250,  28125,  31250,  28125}, /* E_PIXCLK_25175 */
   { 25200,  28000,  25200,  28000,  25200,  28000,  25200}, /* E_PIXCLK_25200 */
   { 27000,  30000,  27000,  30000,  27000,  30000,  27000}, /* E_PIXCLK_27000 */
   { 27027,  30030,  27027,  30030,  27027,  30030,  27027}, /* E_PIXCLK_27027 */
   { 54000,  60000,  54000,  60000,  54000,  60000,  54000}, /* E_PIXCLK_54000 */
   { 54054,  60060,  54054,  60060,  54054,  60060,  54054}, /* E_PIXCLK_54054 */
   {210937, 234375, 140625, 234375, 140625, 234375, 140625}, /* E_PIXCLK_74175 */
   { 74250,  82500,  74250,  82500,  74250,  82500,  74250}, /* E_PIXCLK_74250 */
   {421875, 234375, 140625, 234375, 140625, 234375, 140625}, /* E_PIXCLK_148350*/
   {148500, 165000, 148500, 165000, 148500, 165000, 148500}  /* E_PIXCLK_148500*/
#ifdef FORMAT_PC
  ,{ 31500,  35000,  31500,  35000,  31500,  35000,  31500}, /* E_PIXCLK_31500 */
   { 36000,  40000,  36000,  40000,  36000,  40000,  36000}, /* E_PIXCLK_36000 */
   { 40000,  44444,  40000,  44444,  40000,  44444,  40000}, /* E_PIXCLK_40000 */
   { 49500,  55000,  49500,  55000,  49500,  55000,  49500}, /* E_PIXCLK_49500 */
   { 50000,  55556,  50000,  55556,  50000,  55556,  50000}, /* E_PIXCLK_50000 */
   { 56250,  62500,  56250,  62500,  56250,  62500,  56250}, /* E_PIXCLK_56250 */
   { 65000,  72222,  65000,  72222,  65000,  72222,  65000}, /* E_PIXCLK_65000 */
   { 75000,  83333,  75000,  83333,  75000,  83333,  75000}, /* E_PIXCLK_75000 */
   { 78750,  87500,  78750,  87500,  78750,  87500,  78750}, /* E_PIXCLK_78750 */
   {162000, 180000, 162000, 180000, 162000, 180000, 162000}, /* E_PIXCLK_162000*/
   {157500, 175000, 157500, 175000, 157500, 175000, 157500}  /* E_PIXCLK_157500 */
#endif /* FORMAT_PC */
};

/**
 *  Lookup table for each pixel clock frequency's Audio Clock Regeneration N,
 *  according to SCS Table "Audio Clock Recovery N Values"
 */
static CONST_DAT UInt32 kPixClkToAcrN[E_PIXCLK_NUM][HDMITX_AFS_NUM] =
{
 /* HDMITX_AFS_32k  _AFS_48K       _AFS_96K        _AFS_192K */
 /*         _AFS_44_1k      _AFS_88_2K      _AFS_176_4K       */
   { 4576,   7007,   6864,  14014,  13728,  28028,  27456}, /* E_PIXCLK_25175 */
   { 4096,   6272,   6144,  12544,  12288,  25088,  24576}, /* E_PIXCLK_25200 */
   { 4096,   6272,   6144,  12544,  12288,  25088,  24576}, /* E_PIXCLK_27000 */
   { 4096,   6272,   6144,  12544,  12288,  25088,  24576}, /* E_PIXCLK_27027 */
   { 4096,   6272,   6144,  12544,  12288,  25088,  24576}, /* E_PIXCLK_54000 */
   { 4096,   6272,   6144,  12544,  12288,  25088,  24576}, /* E_PIXCLK_54054 */
   {11648,  17836,  11648,  35672,  23296,  71344,  46592}, /* E_PIXCLK_74175 */
   { 4096,   6272,   6144,  12544,  12288,  25088,  24576}, /* E_PIXCLK_74250 */
   {11648,   8918,   5824,  17836,  11648,  35672,  23296}, /* E_PIXCLK_148350*/
   { 4096,   6272,   6144,  12544,  12288,  25088,  24576}  /* E_PIXCLK_148500*/
#ifdef FORMAT_PC
  ,{ 4096,  6272,  6144,  12544,  12288,  25088,  24576}, /* E_PIXCLK_31500 */
   { 4096,  6272,  6144,  12544,  12288,  25088,  24576}, /* E_PIXCLK_36000 */
   { 4096,  6272,  6144,  12544,  12288,  25088,  24576}, /* E_PIXCLK_40000 */
   { 4096,  6272,  6144,  12544,  12288,  25088,  24576}, /* E_PIXCLK_49500 */
   { 4096,  6272,  6144,  12544,  12288,  25088,  24576}, /* E_PIXCLK_50000 */
   { 4096,  6272,  6144,  12544,  12288,  25088,  24576}, /* E_PIXCLK_56250 */
   { 4096,  6272,  6144,  12544,  12288,  25088,  24576}, /* E_PIXCLK_65000 */
   { 4096,  6272,  6144,  12544,  12288,  25088,  24576}, /* E_PIXCLK_75000 */
   { 4096,  6272,  6144,  12544,  12288,  25088,  24576}, /* E_PIXCLK_78750 */
   { 4096,  6272,  6144,  12544,  12288,  25088,  24576},  /* E_PIXCLK_162000*/
   { 4096,  6272,  6144,  12544,  12288,  25088,  24576}  /* E_PIXCLK_157500*/
#endif /* FORMAT_PC */
};

/**
 *  Lookup table for each pixel clock frequency's Audio Divider, according to
 *  SCS Table "Audio Clock Recovery Divider Values"
 */
static CONST_DAT UInt8 kPixClkToAdiv[E_PIXCLK_NUM][HDMITX_AFS_NUM] =
{
 /* HDMITX_AFS_32k  _AFS_48K       _AFS_96K        _AFS_192K */
 /*         _AFS_44_1k      _AFS_88_2K      _AFS_176_4K       */
   {2,      2,      2,      1,      1,      0,      0},     /* E_PIXCLK_25175 */
   {2,      2,      2,      1,      1,      0,      0},     /* E_PIXCLK_25200 */
   {2,      2,      2,      1,      1,      0,      0},     /* E_PIXCLK_27000 */
   {2,      2,      2,      1,      1,      0,      0},     /* E_PIXCLK_27027 */
   {3,      3,      3,      2,      2,      1,      1},     /* E_PIXCLK_54000 */
   {3,      3,      3,      2,      2,      1,      1},     /* E_PIXCLK_54054 */
   {4,      3,      3,      2,      2,      1,      1},     /* E_PIXCLK_74175 */
   {4,      3,      3,      2,      2,      1,      1},     /* E_PIXCLK_74250 */
   {5,      4,      4,      3,      3,      2,      2},     /* E_PIXCLK_148350 */
   {5,      4,      4,      3,      3,      2,      2}      /* E_PIXCLK_148500 */
#ifdef FORMAT_PC
  ,{2,      2,      2,      1,      1,      0,      0}, /* E_PIXCLK_31500  */
   {3,      2,      2,      1,      1,      0,      0}, /* E_PIXCLK_36000  */
   {3,      2,      2,      1,      1,      0,      0}, /* E_PIXCLK_40000  */
   {3,      3,      3,      2,      2,      1,      1}, /* E_PIXCLK_49500  */
   {3,      3,      3,      2,      2,      1,      1}, /* E_PIXCLK_50000  */
   {3,      3,      3,      2,      2,      1,      1}, /* E_PIXCLK_56250  */
   {4,      3,      3,      2,      2,      1,      1}, /* E_PIXCLK_65000  */
   {4,      3,      3,      2,      2,      1,      1}, /* E_PIXCLK_75000  */
   {4,      3,      3,      2,      2,      1,      1}, /* E_PIXCLK_78750  */
   {5,      4,      4,      3,      3,      2,      2}, /* E_PIXCLK_162000 */
   {5,      4,      4,      3,      3,      2,      2}  /* E_PIXCLK_157500 */
#endif /* FORMAT_PC */

};

/**
 *  Lookup table for converting a sampling frequency into the values
 *  required in channel status byte 3 according to IEC60958-3
 */
static CONST_DAT UInt8 kAfsToCSbyte3[HDMITX_AFS_NUM+1] =
{
    3,      /* HDMITX_AFS_32k */
    0,      /* HDMITX_AFS_44_1k */
    2,      /* HDMITX_AFS_48k */
    8,      /* HDMITX_AFS_88_2k */
    10,     /* HDMITX_AFS_96k */
    12,     /* HDMITX_AFS_176_4k */
    14,     /* HDMITX_AFS_192k */
    9,      /* HDMITX_AFS_768k */
    1,      /* HDMITX_AFS_NOT_INDICATED */
};



/**
 *  Lookup table for each CTS X factor's k and m register values
 */
static CONST_DAT UInt8 kCtsXToMK[HDMITX_CTSX_NUM][2] =
{
/*   Register values    Actual values */
/*   m  k               m, k */ 
    {3, 0},          /* 8, 1 */
    {3, 1},          /* 8, 2 */
    {3, 2},          /* 8, 3 */
    {3, 3},          /* 8, 4 */
    {0, 0}           /* 1, 1 */
};

/**
 * Table of registers to switch scaler off
 */
static CONST_DAT tmHdmiTxRegMaskVal_t kScalerOff[] =
{
    /* Scaler is bypassed */
    {E_REG_P00_MAIN_CNTRL0_RW,   E_MASKREG_P00_MAIN_CNTRL0_scaler,   0},
    /* Bypass PLL scaling */
    {E_REG_P02_PLL_SCG2_RW,     E_MASKREG_P02_PLL_SCG2_bypass_scg,  1},
    /* Disable scaler clock 
    {E_REG_P02_SEL_CLK_RW,      E_MASKREG_P02_SEL_CLK_ena_sc_clk,   0},*/
    /* PLL loop open (standby) */
    {E_REG_P02_PLL_SCG1_RW,     E_MASKREG_P02_PLL_SCG1_scg_fdn,     1},
    {0,0,0}
};

/**
 * Table of registers to switch scaler on
 */
static CONST_DAT tmHdmiTxRegMaskVal_t kScalerOn[] =
{
    /* Scaler is on */
    {E_REG_P00_MAIN_CNTRL0_RW,   E_MASKREG_P00_MAIN_CNTRL0_scaler,   1},
    /* Don't bypass PLL scaling */
    {E_REG_P02_PLL_SCG2_RW,     E_MASKREG_P02_PLL_SCG2_bypass_scg,  0},
    /* Enable scaler clock 
    {E_REG_P02_SEL_CLK_RW,      E_MASKREG_P02_SEL_CLK_ena_sc_clk,   1},*/
    /* PLL loop active */
    {E_REG_P02_PLL_SCG1_RW,     E_MASKREG_P02_PLL_SCG1_scg_fdn,     0},
    {0,0,0}
};

/**
 * Table of registers to reset and release the CTS generator
 */
static CONST_DAT tmHdmiTxRegMaskVal_t kResetCtsGenerator[] =
{
    {E_REG_P11_AIP_CNTRL_0_RW,  E_MASKREG_P11_AIP_CNTRL_0_rst_cts,  1},
    {E_REG_P11_AIP_CNTRL_0_RW,  E_MASKREG_P11_AIP_CNTRL_0_rst_cts,  0},
    {0,0,0}
};

/**
 * Table of registers to bypass colour processing (up/down sampler & matrix)
 */
static CONST_DAT tmHdmiTxRegMaskVal_t kBypassColourProc[] =
{
    /* Bypass upsampler for RGB colourbars */
    {E_REG_P00_HVF_CNTRL_0_W,   E_MASKREG_P00_HVF_CNTRL_0_intpol,   0},
    /* Bypass matrix for RGB colourbars */
    {E_REG_P00_MAT_CONTRL_W,    E_MASKREG_P00_MAT_CONTRL_mat_bp,    1},
    /* Bypass downsampler for RGB colourbars */
    {E_REG_P00_HVF_CNTRL_1_W,   E_MASKREG_P00_HVF_CNTRL_1_for,      0},
    {0,0,0}
};

/**
 * Table of registers to configure video input mode CCIR656*/
static CONST_DAT tmHdmiTxRegMaskVal_t kVinModeCCIR656[] =
{
    {E_REG_P00_VIP_CNTRL_4_W,   E_MASKREG_P00_VIP_CNTRL_4_ccir656,      1},
    {E_REG_P00_HVF_CNTRL_1_W,   E_MASKREG_P00_HVF_CNTRL_1_semi_planar,  1},
    /*{E_REG_P02_SEL_CLK_RW,      E_MASKREG_P02_SEL_CLK_sel_clk1,         1},*/
    {E_REG_P02_PLL_SERIAL_3_RW, E_MASKREG_P02_PLL_SERIAL_3_srl_ccir,    1},
    {E_REG_P02_SEL_CLK_RW,      E_MASKREG_P02_SEL_CLK_sel_vrf_clk,      1},
    {0,0,0}
};

 /* Table of registers to configure video input mode for CCIR656 DDR with 1280*720p and 1920*1080i formats*/
static CONST_DAT tmHdmiTxRegMaskVal_t kVinModeCCIR656_DDR_above720p[] =
{
    {E_REG_P00_VIP_CNTRL_4_W,   E_MASKREG_P00_VIP_CNTRL_4_ccir656,      1},
    {E_REG_P00_HVF_CNTRL_1_W,   E_MASKREG_P00_HVF_CNTRL_1_semi_planar,  1},/*To be defined*/
    /*{E_REG_P02_SEL_CLK_RW,      E_MASKREG_P02_SEL_CLK_sel_clk1,         0},To be defined*/
    {E_REG_P02_PLL_SERIAL_3_RW, E_MASKREG_P02_PLL_SERIAL_3_srl_ccir,    0},
    {E_REG_P02_SEL_CLK_RW,      E_MASKREG_P02_SEL_CLK_sel_vrf_clk,      0},
    {0,0,0}
};
/**
 * Table of registers to configure video input mode RGB444 or YUV444
 */
static CONST_DAT tmHdmiTxRegMaskVal_t kVinMode444[] =
{
    {E_REG_P00_VIP_CNTRL_4_W,   E_MASKREG_P00_VIP_CNTRL_4_ccir656,      0},
    {E_REG_P00_HVF_CNTRL_1_W,   E_MASKREG_P00_HVF_CNTRL_1_semi_planar,  0},
   /* {E_REG_P02_SEL_CLK_RW,      E_MASKREG_P02_SEL_CLK_sel_clk1,         0},*/
    {E_REG_P02_PLL_SERIAL_3_RW, E_MASKREG_P02_PLL_SERIAL_3_srl_ccir,    0},
    {E_REG_P02_SEL_CLK_RW,      E_MASKREG_P02_SEL_CLK_sel_vrf_clk,      0},
    {0,0,0}
};

/**
 * Table of registers to configure video input mode YUV422
 */
static CONST_DAT tmHdmiTxRegMaskVal_t kVinModeYUV422[] =
{
    {E_REG_P00_VIP_CNTRL_4_W,   E_MASKREG_P00_VIP_CNTRL_4_ccir656,      0},
    {E_REG_P00_HVF_CNTRL_1_W,   E_MASKREG_P00_HVF_CNTRL_1_semi_planar,  1},
    /*{E_REG_P02_SEL_CLK_RW,      E_MASKREG_P02_SEL_CLK_sel_clk1,         0},*/
    {E_REG_P02_PLL_SERIAL_3_RW, E_MASKREG_P02_PLL_SERIAL_3_srl_ccir,    0},
    {E_REG_P02_SEL_CLK_RW,      E_MASKREG_P02_SEL_CLK_sel_vrf_clk,      0},
    {0,0,0}
};


/**
 *  Lookup table for colour space conversion matrix register sets.
 *  Each array consists of 31 register values from MAT_CONTROL through
 *  to MAT_OO3_LSB
 */
static CONST_DAT UInt8 kMatrixPreset[MATRIX_PRESET_QTY][MATRIX_PRESET_SIZE] =
{
    {0x2, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x3, 0x6F, 0x0, 0x0, 0x0,
     0x0, 0x0, 0x0, 0x3, 0x6F, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x3,
     0x6F, 0x0, 0x40, 0x0, 0x40, 0x0, 0x40
    },  /* RGB Full to RGB Limited */

    {0x2, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x2, 0x4, 0x1, 0x7, 0x0,
     0x64, 0x6, 0x88, 0x1, 0xC2, 0x7, 0xB7, 0x6, 0xD6, 0x7, 0x68, 0x1,
     0xC2, 0x0, 0x40, 0x2, 0x0, 0x2, 0x0
    },  /* RGB Full to BT601 */

    {0x2, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x2, 0x75, 0x0, 0xBB, 0x0,
     0x3F, 0x6, 0x68, 0x1, 0xC2, 0x7, 0xD7, 0x6, 0xA6, 0x7, 0x99, 0x1,
     0xC2, 0x0, 0x40, 0x2, 0x0, 0x2, 0x0
    },  /* RGB Full to BT709 */

    {0x1, 0x7, 0xC0, 0x7, 0xC0, 0x7, 0xC0, 0x2, 0x54, 0x0, 0x0, 0x0,
     0x0, 0x0, 0x0, 0x2, 0x54, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x2,
     0x54, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0
    },  /* RGB Limited to RGB Full */

    {0x2, 0x7, 0xC0, 0x7, 0xC0, 0x7, 0xC0, 0x2, 0x59, 0x1, 0x32, 0x0,
     0x75, 0x6, 0x4A, 0x2, 0x0C, 0x7, 0xAB, 0x6, 0xA5, 0x7, 0x4F, 0x2,
     0x0C, 0x0, 0x40, 0x2, 0x0, 0x2, 0x0
    },  /* RGB Limited to BT601 */

    {0x2, 0x7, 0xC0, 0x7, 0xC0, 0x7, 0xC0, 0x2, 0xDC, 0x0, 0xDA, 0x0,
     0x4A, 0x6, 0x24, 0x2, 0x0C, 0x7, 0xD0, 0x6, 0x6C, 0x7, 0x88, 0x2,
     0x0C, 0x0, 0x40, 0x2, 0x0, 0x2, 0x0
    },  /* RGB Limited to BT709 */

    {0x0, 0x7, 0xC0, 0x6, 0x0, 0x6, 0x0, 0x1, 0x2A, 0x7, 0x30, 0x7,
     0x9C, 0x1, 0x2A, 0x1, 0x99, 0x0, 0x0, 0x1, 0x2A, 0x0, 0x0, 0x2,
     0x5, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0
    },  /* BT601 to RGB Full */

    {0x1, 0x7, 0xC0, 0x6, 0x0, 0x6, 0x0, 0x2, 0x0, 0x6, 0x9A, 0x7,
     0x54, 0x2, 0x0, 0x2, 0xBE, 0x0, 0x0, 0x2, 0x0, 0x0, 0x0, 0x3,
     0x77, 0x0, 0x40, 0x0, 0x40, 0x0, 0x40
    },  /* BT601 to RGB Limited */

    {0x1, 0x7, 0xC0, 0x6, 0x0, 0x6, 0x0, 0x2, 0x0, 0x7, 0x96, 0x7,
     0xC5, 0x0, 0x0, 0x2, 0x0D, 0x0, 0x26, 0x0, 0x0, 0x0, 0x3B, 0x2,
     0x0A, 0x0, 0x40, 0x2, 0x0, 0x2, 0x0
    },  /* BT601 to BT709 */

    {0x0, 0x7, 0xC0, 0x6, 0x0, 0x6, 0x0, 0x1, 0x2A, 0x7, 0x77, 0x7,
     0xC9, 0x1, 0x2A, 0x1, 0xCB, 0x0, 0x0, 0x1, 0x2A, 0x0, 0x0, 0x2,
     0x1D, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0
    },  /* BT709 to RGB Full */

    {0x1, 0x7, 0xC0, 0x6, 0x0, 0x6, 0x0, 0x2, 0x0, 0x7, 0x16, 0x7,
     0xA2, 0x2, 0x0, 0x3, 0x14, 0x0, 0x0, 0x2, 0x0, 0x0, 0x0, 0x3,
     0xA1, 0x0, 0x40, 0x0, 0x40, 0x0, 0x40
    },  /* BT709 to RGB Limited */

    {0x1, 0x7, 0xC0, 0x6, 0x0, 0x6, 0x0, 0x2, 0x0, 0x0, 0x62, 0x0,
     0x33, 0x0, 0x0, 0x1, 0xF7, 0x7, 0xDB, 0x0, 0x0, 0x7, 0xC7, 0x1,
     0xFB, 0x0, 0x40, 0x2, 0x0, 0x2, 0x0
    }  /* BT709 to BT601 */
}; 

/**
 *  This table gives us the index into the kMatrixPreset array, based
 *  on the input and output colourspaces.
 *  The co-ordinates into this array are tmbslTDA9989Colourspace_t enums.
 *  The value of -1 is returned for matching input/output colourspaces.
 */
static CONST_DAT Int kMatrixIndex[HDMITX_CS_NUM][HDMITX_CS_NUM] =
{
    {-1, E_MATRIX_RGBF_2_RGBL, E_MATRIX_RGBF_2_BT601, E_MATRIX_RGBF_2_BT709},
    {E_MATRIX_RGBL_2_RGBF, -1, E_MATRIX_RGBL_2_BT601, E_MATRIX_RGBL_2_BT709},
    {E_MATRIX_BT601_2_RGBF, E_MATRIX_BT601_2_RGBL, -1, E_MATRIX_BT601_2_BT709},
    {E_MATRIX_BT709_2_RGBF, E_MATRIX_BT709_2_RGBL, E_MATRIX_BT709_2_BT601, -1}
};

/**
 *  Blue filter Lookup table for colour space conversion.
 *  Each array consists of 31 register values from MAT_CONTROL through
 *  to MAT_OO3_LSB
 */
static CONST_DAT UInt8 MatrixCoeffBlueScreen[][MATRIX_PRESET_SIZE] =
{
    {0x1, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
     0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
     0x0, 0x0, 0x0, 0x0, 0x0, 0x2, 0x0
    },  /* blue screen for RGB output color space */

    {0x1, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
     0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
     0x0, 0x0, 0x0, 0x1, 0x0, 0x3, 0x0
    },  /* blue screen for YCbCr422 output color space */

    {0x1, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
     0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
     0x0, 0x0, 0x0, 0x1, 0x0, 0x3, 0x0
    },  /* blue screen for YCbCr444 output color space */
};

/**
 *  Black filter Lookup table for colour space conversion.
 *  Each array consists of 31 register values from MAT_CONTROL through
 *  to MAT_OO3_LSB
 */
static CONST_DAT UInt8 MatrixCoeffBlackScreen[][MATRIX_PRESET_SIZE] =
{
    {0x1, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
     0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
     0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0
    },  /* black screen for RGB output color space */

    {0x1, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
     0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
     0x0, 0x0, 0x0, 0x2, 0x0, 0x2, 0x0
    },  /* black screen for YCbCr422 output color space */

    {0x1, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
     0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
     0x0, 0x0, 0x0, 0x2, 0x0, 0x2, 0x0
    },  /* black screen for YCbCr444 output color space */
};


/*============================================================================*/
/*                       DEFINES DECLARATIONS                               */
/*============================================================================*/
#define HDMITX_LAT_SCO_MAX_VAL 40
#define HDMITX_LAT_SCO_MIN_VAL 34

/*============================================================================*/
/*                       VARIABLES DECLARATIONS                               */
/*============================================================================*/

/* Register values per device to restore colour processing after test pattern */
static RAM_DAT  UInt8   gMatContrl[HDMITX_UNITS_MAX];
static RAM_DAT  UInt8   gHvfCntrl0[HDMITX_UNITS_MAX];
static RAM_DAT  UInt8   gHvfCntrl1[HDMITX_UNITS_MAX];

/*============================================================================*/
/*                       FUNCTION PROTOTYPES                                  */
/*============================================================================*/

static tmErrorCode_t    setDeVs        (tmHdmiTxobject_t *pDis,
                                           tmbslHdmiTxVidFmt_t voutFmt);

static tmErrorCode_t    setPixelRepeat (tmHdmiTxobject_t *pDis, 
                                           tmbslHdmiTxVidFmt_t voutFmt,
                                           UInt8 uPixelRepeat);

static tmErrorCode_t    setSampling    (tmHdmiTxobject_t *pDis);

static tmErrorCode_t    setScalerFormat(tmHdmiTxobject_t *pDis,
                                           tmbslHdmiTxVidFmt_t vinFmt,
                                           tmbslHdmiTxVidFmt_t voutFmt,
                                           UInt8 pixelRepeat);

static UInt8            calculateChecksum (UInt8 *pData, Int numBytes);

static tmErrorCode_t    InputConfig (tmHdmiTxobject_t           *pDis,
                                     tmbslHdmiTxVinMode_t       vinMode,
                                     tmbslHdmiTxPixEdge_t       sampleEdge,
                                     tmbslHdmiTxPixRate_t       pixRate,
                                     tmbslHdmiTxUpsampleMode_t  upsampleMode, 
                                     UInt8                      uPixelRepeat,
                                     tmbslHdmiTxVidFmt_t          voutFmt);
/*============================================================================*/
/* tmbslTDA9989AudioInResetCts                                                 */
/*============================================================================*/
tmErrorCode_t
tmbslTDA9989AudioInResetCts
(
    tmUnitSelect_t              txUnit
)
{
    tmHdmiTxobject_t *pDis;     /* Pointer to Device Instance Structure */
    tmErrorCode_t     err;      /* Error code */

    /* Check unit parameter and point to its object */
    err = checkUnitSetDis(txUnit, &pDis);
    RETIF(err != TM_OK, err)

    /* Return if sink is not an HDMI device */
    RETIF(pDis->sinkType != HDMITX_SINK_HDMI, 
          TMBSL_ERR_HDMI_OPERATION_NOT_PERMITTED)

    /* Reset and release the CTS generator */
    err = setHwRegisterFieldTable(pDis, &kResetCtsGenerator[0]);
    return err;
}


/*============================================================================*/
/* tmbslTDA9989AudioInSetConfig                                                */
/*============================================================================*/
tmErrorCode_t
tmbslTDA9989AudioInSetConfig
(
    tmUnitSelect_t           txUnit,
    tmbslHdmiTxaFmt_t        aFmt,
    tmbslHdmiTxI2sFor_t      i2sFormat,
    UInt8                    chanI2s,
    UInt8                    chanDsd,
    tmbslHdmiTxClkPolDsd_t   clkPolDsd,
    tmbslHdmiTxSwapDsd_t     swapDsd,
    UInt8                    layout,
    UInt16                   uLatency_rd,
    tmbslHdmiTxDstRate_t     dstRate
)
{
    tmHdmiTxobject_t *pDis;     /* Pointer to Device Instance Structure */
    tmErrorCode_t     err;      /* Error code */
    UInt8             regVal;   /* Value to write in register */

    DUMMY_ACCESS(dstRate);

    /* Check unit parameter and point to its object */
    err = checkUnitSetDis(txUnit, &pDis);
    RETIF(err != TM_OK, err)

    /* Return if sink is not an HDMI device */
    RETIF(pDis->sinkType != HDMITX_SINK_HDMI, 
          TMBSL_ERR_HDMI_OPERATION_NOT_PERMITTED)

    /* Check remaining parameters */
#ifdef TMFL_HBR_SUPPORT
    RETIF_BADPARAM((aFmt != HDMITX_AFMT_SPDIF) &&
                   (aFmt != HDMITX_AFMT_I2S)   &&
                   (aFmt != HDMITX_AFMT_OBA)   &&
                   (aFmt != HDMITX_AFMT_HBR))

#else /* TMFL_HBR_SUPPORT */
    RETIF_BADPARAM((aFmt != HDMITX_AFMT_SPDIF) &&
                   (aFmt != HDMITX_AFMT_I2S)   &&
                   (aFmt != HDMITX_AFMT_OBA))


#endif /* TMFL_HBR_SUPPORT */

    RETIF_BADPARAM(chanI2s     >= HDMITX_CHAN_INVALID)
    RETIF_BADPARAM(chanDsd     >= HDMITX_CHAN_INVALID)
    RETIF_BADPARAM(clkPolDsd   >= HDMITX_CLKPOLDSD_INVALID)
    RETIF_BADPARAM(swapDsd     >= HDMITX_SWAPDSD_INVALID)
    RETIF_BADPARAM(layout      >= HDMITX_LAYOUT_INVALID)
    RETIF_BADPARAM(uLatency_rd >= HDMITX_LATENCY_INVALID)

    if ((aFmt == HDMITX_AFMT_I2S) 
#ifdef TMFL_HBR_SUPPORT 
        || (aFmt == HDMITX_AFMT_HBR)
#endif /* TMFL_HBR_SUPPORT */
        )
    {
        RETIF_BADPARAM((i2sFormat != HDMITX_I2SFOR_PHILIPS_L) &&
                       (i2sFormat != HDMITX_I2SFOR_OTH_L)     &&
                       (i2sFormat != HDMITX_I2SFOR_OTH_R)
                       )
    }


    switch (aFmt)
    {
        case HDMITX_AFMT_SPDIF:
            regVal = (UInt8)REG_VAL_SEL_AIP_SPDIF;


#ifdef TMFL_TDA19989

            /* configure MUX_AP */
            err = setHwRegister(pDis, E_REG_P00_MUX_AP_RW, TDA19989_MUX_AP_SELECT_SPDIF);
            RETIF_REG_FAIL(err)

#else /* TMFL_TDA19989 */
            /* configure MUX_AP */
            err = setHwRegister(pDis, E_REG_P00_MUX_AP_RW, MUX_AP_SELECT_SPDIF);
            RETIF_REG_FAIL(err)
#endif/* TMFL_TDA19989 */

            break;

        case HDMITX_AFMT_I2S:
            regVal = (UInt8)REG_VAL_SEL_AIP_I2S;

#ifdef TMFL_TDA19989

            /* configure MUX_AP */
            err = setHwRegister(pDis, E_REG_P00_MUX_AP_RW, TDA19989_MUX_AP_SELECT_I2S);
            RETIF_REG_FAIL(err)


#else /* TMFL_TDA19989 */
            /* configure MUX_AP */
            err = setHwRegister(pDis, E_REG_P00_MUX_AP_RW, MUX_AP_SELECT_I2S);
            RETIF_REG_FAIL(err)
#endif/* TMFL_TDA19989 */
            break;

        case HDMITX_AFMT_OBA:
            regVal = (UInt8)REG_VAL_SEL_AIP_OBA;
            break;

        case HDMITX_AFMT_HBR:
            regVal = (UInt8)REG_VAL_SEL_AIP_HBR;
            break;
            
        default:
            return TMBSL_ERR_HDMI_BAD_PARAMETER;
            break;
    }


    /* Set the audio input processor format to aFmt. */
    err = setHwRegisterField(pDis,
                             E_REG_P00_AIP_CLKSEL_W,
                             E_MASKREG_P00_AIP_CLKSEL_sel_aip,
                             regVal);
    RETIF_REG_FAIL(err)

    /* Channel status on 1 channel  */
    err = setHwRegisterField(pDis,
                             E_REG_P11_CA_I2S_RW,
                             E_MASKREG_P11_CA_I2S_hbr_chstat_4,
                             0);
    RETIF_REG_FAIL(err)

    /* Select the audio format */
    if ((aFmt == HDMITX_AFMT_I2S) 
#ifdef TMFL_HBR_SUPPORT
        || (aFmt == HDMITX_AFMT_HBR)
#endif /* TMFL_HBR_SUPPORT */
       )
    {
        if (chanI2s != HDMITX_CHAN_NO_CHANGE)
        {
            err = setHwRegisterField(pDis,
                  E_REG_P11_CA_I2S_RW,
                  E_MASKREG_P11_CA_I2S_ca_i2s,
                  (UInt8)chanI2s);
        }

        /* Select the I2S format */
        err = setHwRegisterField(pDis,
                                 E_REG_P00_I2S_FORMAT_RW,
                                 E_MASKREG_P00_I2S_FORMAT_i2s_format,
                                 (UInt8)i2sFormat);
        RETIF_REG_FAIL(err)

//#ifdef TMFL_HBR_SUPPORT 
      //if (aFmt == HDMITX_AFMT_HBR)
      // {
            /* Channel status on 1 channel  */
      //      err = setHwRegisterField(pDis,
      //                       E_REG_P11_CA_I2S_RW,
      //                       E_MASKREG_P11_CA_I2S_hbr_chstat_4,
      //                       1);
      //      RETIF_REG_FAIL(err)
      //  }
//#endif /* TMFL_HBR_SUPPORT */
    }
    else if (aFmt == HDMITX_AFMT_OBA)
    {
        if (chanDsd != HDMITX_CHAN_NO_CHANGE)
        {
            err = setHwRegister(pDis, E_REG_P11_CA_DSD_RW, chanDsd);
            RETIF_REG_FAIL(err)
        }
        if (clkPolDsd != HDMITX_CLKPOLDSD_NO_CHANGE)
        {
            err = setHwRegisterField(pDis,
                                     E_REG_P00_AIP_CLKSEL_W,
                                     E_MASKREG_P00_AIP_CLKSEL_sel_pol_clk,
                                     (UInt8)clkPolDsd);
            RETIF_REG_FAIL(err)
        }
        if (swapDsd != HDMITX_SWAPDSD_NO_CHANGE)
        {
            err = setHwRegisterField(pDis,
                                     E_REG_P11_AIP_CNTRL_0_RW,
                                     E_MASKREG_P11_AIP_CNTRL_0_swap,
                                     (UInt8)swapDsd);
            RETIF_REG_FAIL(err)
        }
    }

    /* Set layout and latency */
    if (layout != HDMITX_LAYOUT_NO_CHANGE)
    {
        err = setHwRegisterField(pDis,
                                 E_REG_P11_AIP_CNTRL_0_RW,
                                 E_MASKREG_P11_AIP_CNTRL_0_layout,
                                 layout);
        RETIF_REG_FAIL(err)
    }
    if (uLatency_rd != HDMITX_LATENCY_NO_CHANGE)
    {
        err = setHwRegister(pDis, E_REG_P11_LATENCY_RD_RW, (UInt8)uLatency_rd);
        RETIF_REG_FAIL(err)
    }
    return TM_OK;
}

/*============================================================================*/
/* tmbslTDA9989AudioInSetCts                                                   */
/*============================================================================*/
tmErrorCode_t
tmbslTDA9989AudioInSetCts
(
    tmUnitSelect_t       txUnit,
    tmbslHdmiTxctsRef_t  ctsRef,
    tmbslHdmiTxafs_t     afs, 
    tmbslHdmiTxVidFmt_t  voutFmt, 
    tmbslHdmiTxVfreq_t   voutFreq, 
    UInt32               uCts, 
    UInt16               uCtsX,
    tmbslHdmiTxctsK_t    ctsK,
    tmbslHdmiTxctsM_t    ctsM,
    tmbslHdmiTxDstRate_t dstRate
)
{
    tmHdmiTxobject_t *pDis;     /* Pointer to Device Instance Structure */
    tmErrorCode_t   err;        /* Error code */
    UInt8           regVal;     /* Register value */
    UInt8           pixClk;     /* Pixel clock index */
    UInt32          acrN;       /* Audio clock recovery N */

    DUMMY_ACCESS(dstRate);

    /* Check unit parameter and point to its object */
    err = checkUnitSetDis(txUnit, &pDis);
    RETIF(err != TM_OK, err)

    /* Return if sink is not an HDMI device */
    RETIF(pDis->sinkType != HDMITX_SINK_HDMI, 
          TMBSL_ERR_HDMI_OPERATION_NOT_PERMITTED)

    /* Check remaining parameters */
    RETIF_BADPARAM(ctsRef   >= HDMITX_CTSREF_INVALID)
    RETIF_BADPARAM(afs      >= HDMITX_AFS_INVALID)
    
#ifdef FORMAT_PC
    RETIF_BADPARAM(voutFmt       < HDMITX_VFMT_TV_MIN)
    RETIF_BADPARAM((voutFmt > HDMITX_VFMT_TV_MAX) && (voutFmt < HDMITX_VFMT_PC_MIN))
    RETIF_BADPARAM(voutFmt       > HDMITX_VFMT_PC_MAX)
#else /* FORMAT_PC */
    /*FORMAT TV only*/
    RETIF_BADPARAM(voutFmt       < HDMITX_VFMT_TV_MIN)
    RETIF_BADPARAM(voutFmt       > HDMITX_VFMT_TV_MAX)
#endif /* FORMAT_PC */

    RETIF_BADPARAM(voutFreq >= HDMITX_VFREQ_INVALID)
    RETIF_BADPARAM(uCtsX    >= HDMITX_CTSX_INVALID)
    RETIF_BADPARAM(ctsK     >= HDMITX_CTSK_INVALID)
    RETIF_BADPARAM(ctsM     >= HDMITX_CTSMTS_INVALID)
   
    
    if ((voutFmt  >=  HDMITX_VFMT_TV_MIN) && (voutFmt  <=  HDMITX_VFMT_TV_MAX))
    {
        if (voutFreq == HDMITX_VFREQ_50Hz)
        {
            RETIF(((voutFmt < HDMITX_VFMT_17_720x576p_50Hz) || (voutFmt > HDMITX_VFMT_31_1920x1080p_50Hz)),
                                        TMBSL_ERR_HDMI_INCONSISTENT_PARAMS)
        }
        else if (voutFreq == HDMITX_VFREQ_24Hz)
        {
            RETIF(voutFmt != HDMITX_VFMT_32_1920x1080p_24Hz,
                                        TMBSL_ERR_HDMI_INCONSISTENT_PARAMS)
        }
        else if (voutFreq == HDMITX_VFREQ_25Hz)
        {
            RETIF(voutFmt != HDMITX_VFMT_33_1920x1080p_25Hz,
                                        TMBSL_ERR_HDMI_INCONSISTENT_PARAMS)
        }
        else if (voutFreq == HDMITX_VFREQ_30Hz)
        {
            RETIF(voutFmt != HDMITX_VFMT_34_1920x1080p_30Hz,
                                        TMBSL_ERR_HDMI_INCONSISTENT_PARAMS)
        }
        else
        {
            RETIF(voutFmt >= HDMITX_VFMT_17_720x576p_50Hz,
                                       TMBSL_ERR_HDMI_INCONSISTENT_PARAMS)
        }
            
    }

#ifdef FORMAT_PC
    if ((voutFmt  >=  HDMITX_VFMT_PC_MIN) && (voutFmt  <=  HDMITX_VFMT_PC_MAX))
    {
        if (voutFreq == HDMITX_VFREQ_60Hz)
        {
            RETIF(voutFmt > HDMITX_VFMT_PC_1600x1200p_60Hz,
                                       TMBSL_ERR_HDMI_INCONSISTENT_PARAMS)
        }
        else if (voutFreq == HDMITX_VFREQ_70Hz)
        {
            RETIF(voutFmt != HDMITX_VFMT_PC_1024x768p_70Hz,
                                       TMBSL_ERR_HDMI_INCONSISTENT_PARAMS)
        }
        else if (voutFreq == HDMITX_VFREQ_72Hz)
        {
            RETIF( ((voutFmt < HDMITX_VFMT_PC_640x480p_72Hz) ||
                    (voutFmt > HDMITX_VFMT_PC_800x600p_72Hz)),
                                       TMBSL_ERR_HDMI_INCONSISTENT_PARAMS)
        }
        else if (voutFreq == HDMITX_VFREQ_75Hz)
        {
            RETIF( ((voutFmt < HDMITX_VFMT_PC_640x480p_75Hz) ||
                    (voutFmt > HDMITX_VFMT_PC_1280x1024p_75Hz)),
                                       TMBSL_ERR_HDMI_INCONSISTENT_PARAMS)
        }
        else if (voutFreq == HDMITX_VFREQ_85Hz)
        {
            RETIF( ((voutFmt < HDMITX_VFMT_PC_640x350p_85Hz) ||
                    (voutFmt > HDMITX_VFMT_PC_1280x1024p_85Hz)),
                                       TMBSL_ERR_HDMI_INCONSISTENT_PARAMS)
        }
        else
        {
            RETIF(voutFmt != HDMITX_VFMT_PC_1024x768i_87Hz,
                                       TMBSL_ERR_HDMI_INCONSISTENT_PARAMS)
        }
    }
#endif /* FORMAT_PC */

    /* Check for auto or manual CTS */
    if (uCts == HDMITX_CTS_AUTO)
    {
        /* Auto */
        err = setHwRegisterField(pDis,
                                 E_REG_P11_AIP_CNTRL_0_RW,
                                 E_MASKREG_P11_AIP_CNTRL_0_acr_man,
                                 0);
        RETIF_REG_FAIL(err)
    }
    else
    {
        /* Manual */
        err = setHwRegisterField(pDis,
                                 E_REG_P11_AIP_CNTRL_0_RW,
                                 E_MASKREG_P11_AIP_CNTRL_0_acr_man,
                                 1);
        RETIF_REG_FAIL(err)
    }

    /* Derive M and K from X? */
    if ((ctsM == HDMITX_CTSMTS_USE_CTSX) || (ctsK == HDMITX_CTSK_USE_CTSX))
    {
        RETIF_BADPARAM(uCtsX == HDMITX_CTSX_UNUSED)
        ctsM = (tmbslHdmiTxctsM_t) kCtsXToMK[uCtsX][0];
        ctsK = (tmbslHdmiTxctsK_t)kCtsXToMK[uCtsX][1];
    }

    /* Set the Post-divider measured timestamp factor */
    regVal = (UInt8)ctsM;
    err = setHwRegisterField(pDis,
                             E_REG_P11_CTS_N_RW,
                             E_MASKREG_P11_CTS_N_m_sel,
                             regVal);
    RETIF_REG_FAIL(err)

    /* Set the Pre-divider scale */
    regVal = (UInt8)ctsK;
    err = setHwRegisterField(pDis,
                             E_REG_P11_CTS_N_RW,
                             E_MASKREG_P11_CTS_N_k_sel,
                             regVal);
    RETIF_REG_FAIL(err)

    /* Use voutFmt and voutFreq to index into a lookup table to get
     * the current pixel clock value. */

#ifdef FORMAT_PC
    if ((voutFmt  >=  HDMITX_VFMT_PC_MIN) && (voutFmt  <=  HDMITX_VFMT_PC_MAX))
    {
        pixClk = kVfmtToPixClk_PC[(voutFmt - HDMITX_VFMT_PC_MIN + 1)];
    }
    else
    {
#endif /* FORMAT_PC */
        pixClk = kVfmtToPixClk_TV[voutFmt][voutFreq];
#ifdef FORMAT_PC
    }
#endif /* FORMAT_PC */

    if (pixClk != E_PIXCLK_INVALID)
    {
        /* Set the Audio Clock Recovery N multiplier based on the audio sample
         * frequency afs and current pixel clock. */
        acrN = kPixClkToAcrN[pixClk][afs];

        /* Set ACR N multiplier [19 to 16] */
        regVal = (UInt8)(acrN >> 16);
        err = setHwRegister(pDis, E_REG_P11_ACR_N_2_RW, regVal);
        RETIF_REG_FAIL(err)
        /* Set ACR N multiplier [15 to 8] */
        regVal = (UInt8)(acrN >> 8);
        err = setHwRegister(pDis, E_REG_P11_ACR_N_1_RW, regVal);
        RETIF_REG_FAIL(err)
        /* Set ACR N multiplier [7 to 0] */
        regVal = (UInt8)acrN;
        err = setHwRegister(pDis, E_REG_P11_ACR_N_0_RW, regVal);
        RETIF_REG_FAIL(err)

        /* Set the CDC Audio Divider register based on the audio sample frequency
         * afs and current pixel clock. */
        regVal = kPixClkToAdiv[pixClk][afs];
        err = setHwRegister(pDis, E_REG_P02_AUDIO_DIV_RW, regVal);
        RETIF_REG_FAIL(err)

        /* If auto CTS, get CTS value based on the audio sample
         * frequency afs and current pixel clock. */
        if (uCts == HDMITX_CTS_AUTO)
        {
            uCts = kPixClkToAcrCts[pixClk][afs];
        }
    }

    /* Set manual or pixel clock CTS */
    if (uCts != HDMITX_CTS_AUTO)
    {
        /* Set manual ACR CTS [19 to 16 */
        regVal = (UInt8)(uCts >> 16);
        err = setHwRegister(pDis, E_REG_P11_ACR_CTS_2_RW, regVal);
        RETIF_REG_FAIL(err)
        /* Set manual ACR CTS [15 to 8] */
        regVal = (UInt8)(uCts >> 8);
        err = setHwRegister(pDis, E_REG_P11_ACR_CTS_1_RW, regVal);
        RETIF_REG_FAIL(err)
        /* Set manual ACR CTS [7 to 0] */
        regVal = (UInt8)uCts;
        err = setHwRegister(pDis, E_REG_P11_ACR_CTS_0_RW, regVal);
        RETIF_REG_FAIL(err)
    }

    /* Set the CTS clock reference register according to ctsRef */
    regVal = (UInt8)ctsRef;
    err = setHwRegisterField(pDis,
                             E_REG_P00_AIP_CLKSEL_W,
                             E_MASKREG_P00_AIP_CLKSEL_sel_fs,
                             regVal);
    RETIF_REG_FAIL(err)

    /* Reset and release the CTS generator */
    err = setHwRegisterFieldTable(pDis, &kResetCtsGenerator[0]);
    return err;
}

/*============================================================================*/
/* tmbslTDA9989AudioOutSetChanStatus                                           */
/*============================================================================*/
tmErrorCode_t
tmbslTDA9989AudioOutSetChanStatus
(
    tmUnitSelect_t               txUnit,
    tmbslHdmiTxAudioData_t       pcmIdentification,
    tmbslHdmiTxCSformatInfo_t    formatInfo,
    tmbslHdmiTxCScopyright_t     copyright,
    UInt8                        categoryCode,
    tmbslHdmiTxafs_t             sampleFreq,
    tmbslHdmiTxCSclkAcc_t        clockAccuracy,
    tmbslHdmiTxCSmaxWordLength_t maxWordLength,
    tmbslHdmiTxCSwordLength_t    wordLength,
    tmbslHdmiTxCSorigAfs_t       origSampleFreq
)
{
    tmHdmiTxobject_t *pDis;     /* Pointer to Device Instance Structure */
    tmErrorCode_t     err;      /* Error code */
    UInt8             buf[4];   /* Buffer to hold channel status data */

    /* Check unit parameter and point to its object */
    err = checkUnitSetDis(txUnit, &pDis);
    RETIF(err != TM_OK, err)

    /* Return if sink is not an HDMI device */
    RETIF(pDis->sinkType != HDMITX_SINK_HDMI, 
          TMBSL_ERR_HDMI_OPERATION_NOT_PERMITTED)

    /* Check remaining parameters */
    RETIF_BADPARAM(formatInfo     >= HDMITX_CSFI_INVALID)
    RETIF_BADPARAM(copyright      >= HDMITX_CSCOPYRIGHT_INVALID)
    RETIF_BADPARAM(sampleFreq     >  HDMITX_AFS_NOT_INDICATED)
    RETIF_BADPARAM(clockAccuracy  >= HDMITX_CSCLK_INVALID)
    RETIF_BADPARAM(maxWordLength  >= HDMITX_CSMAX_INVALID)
    RETIF_BADPARAM(wordLength     >= HDMITX_CSWORD_INVALID)
    RETIF_BADPARAM(wordLength     == HDMITX_CSWORD_RESVD)
    RETIF_BADPARAM(origSampleFreq >= HDMITX_CSAFS_INVALID)
    RETIF_BADPARAM(pcmIdentification >=HDMITX_AUDIO_DATA_INVALID)


    /* Prepare Byte 0 */
    buf[0] = ((UInt8)formatInfo << 3) | ((UInt8)copyright << 2) | ((UInt8)pcmIdentification<< 1);


    /* Prepare Byte 1 */
    buf[1] = categoryCode;

    /* Prepare Byte 3  - note Byte 2 not in sequence in TDA9983 register map */
    buf[2] = ((UInt8)clockAccuracy << 4) | kAfsToCSbyte3[sampleFreq];

    /* Prepare Byte 4 */
    buf[3] = ((UInt8)origSampleFreq << 4) | ((UInt8)wordLength << 1) |
              (UInt8)maxWordLength;

    /* Write 4 Channel Status bytes */
    err = setHwRegisters(pDis, E_REG_P11_CH_STAT_B_0_RW, &buf[0], 4);
    return err;
}

/*============================================================================*/
/* tmbslTDA9989AudioOutSetChanStatusMapping                                    */
/*============================================================================*/

tmErrorCode_t
tmbslTDA9989AudioOutSetChanStatusMapping
(
    tmUnitSelect_t  txUnit,
    UInt8           sourceLeft[4],
    UInt8           channelLeft[4],
    UInt8           sourceRight[4],
    UInt8           channelRight[4]
)
{
    tmHdmiTxobject_t *pDis;     /* Pointer to Device Instance Structure */
    tmErrorCode_t     err;      /* Error code */
    UInt8             buf[2];   /* Buffer to hold channel status data */

    /* Check unit parameter and point to its object */
    err = checkUnitSetDis(txUnit, &pDis);
    RETIF(err != TM_OK, err)

    /* Return if sink is not an HDMI device */
    RETIF(pDis->sinkType != HDMITX_SINK_HDMI, 
          TMBSL_ERR_HDMI_OPERATION_NOT_PERMITTED)

    /* Check remaining parameters */
    RETIF_BADPARAM(sourceLeft[0]   > HDMITX_CS_SOURCES_MAX)
    RETIF_BADPARAM(channelLeft[0]  > HDMITX_CS_CHANNELS_MAX)
    RETIF_BADPARAM(sourceRight[0]  > HDMITX_CS_SOURCES_MAX)
    RETIF_BADPARAM(channelRight[0] > HDMITX_CS_CHANNELS_MAX)

    /* Prepare Left byte */
    buf[0] = ((UInt8)channelLeft[0] << 4) | (UInt8)sourceLeft[0];

    /* Prepare Right byte */
    buf[1] = ((UInt8)channelRight[0] << 4) | (UInt8)sourceRight[0];

    /* Write 2 Channel Status bytes */
    err = setHwRegisters(pDis, E_REG_P11_CH_STAT_B_2_ap0_l_RW, &buf[0], 2);
    return err;
}


/*============================================================================*/
/* tmbslTDA9989AudioOutSetMute                                                 */
/*============================================================================*/
tmErrorCode_t
tmbslTDA9989AudioOutSetMute
(
    tmUnitSelect_t      txUnit,
    tmbslHdmiTxaMute_t  aMute
)
{
    tmHdmiTxobject_t *pDis;             /* Pointer to Device Instance Structure */
    tmErrorCode_t     err;              /* Error code */

    /* Check unit parameter and point to its object */
    err = checkUnitSetDis(txUnit, &pDis);
    RETIF(err != TM_OK, err)

    /* Return if sink is not an HDMI device */
    RETIF(pDis->sinkType != HDMITX_SINK_HDMI, 
          TMBSL_ERR_HDMI_OPERATION_NOT_PERMITTED)

    /* Check remaining parameters */
    RETIF_BADPARAM(aMute >= HDMITX_AMUTE_INVALID)

    /* audio mute workaround, un-map audio input before muting */
    if (aMute == HDMITX_AMUTE_ON)
    {
        err = setHwRegisterField(pDis,
                                E_REG_P00_SR_REG_W,
                                E_MASKREG_P00_SR_REG_sr_audio,
                                (UInt8)aMute);
        RETIF(err != TM_OK, err)


        err = setHwRegisterField(pDis,
                                E_REG_P00_SR_REG_W,
                                E_MASKREG_P00_SR_REG_sr_audio,
                                (UInt8) !aMute);
        RETIF(err != TM_OK, err)

    }

    /* Reset the audio FIFO to mute audio */
    err = setHwRegisterField(pDis,
                             E_REG_P11_AIP_CNTRL_0_RW,
                             E_MASKREG_P11_AIP_CNTRL_0_rst_fifo,
                             (UInt8)aMute);
    RETIF(err != TM_OK, err)


    return TM_OK;

}

/*============================================================================*/
/* tmbslTDA9989ScalerGet                                                       */
/*============================================================================*/

tmErrorCode_t
tmbslTDA9989ScalerGet
(
    tmUnitSelect_t            txUnit,
    tmbslHdmiTxScalerDiag_t  *pScalerDiag
)
{
    tmHdmiTxobject_t *pDis;     /* Pointer to Device Instance Structure */
    tmErrorCode_t   err;        /* Error code */
    UInt8           regVal = 0;     /* Register value */

    /* Check unit parameter and point to its object */
    err = checkUnitSetDis(txUnit, &pDis);
    RETIF(err != TM_OK, err)

#ifdef TMFL_SCALER_SUPPORT
    /* Check if this device has a Scaler */
    RETIF(pDis->uDeviceFeatures & E_MASKREG_P00_VERSION_not_s,
          TMBSL_ERR_HDMI_NOT_SUPPORTED)

    /* Check remaining parameters */
    RETIF_BADPARAM(pScalerDiag == (tmbslHdmiTxScalerDiag_t *)0)

    /* Read from dummy register to update MAX_BUFFILL_P registers */
    err = getHwRegister(pDis, E_REG_P01_SC_SAMPLE_BUFFILL_R, &regVal);
    RETIF_REG_FAIL(err)

    /* Read the 16-bit MAX_BUFFILL_P register pair and copy to pScalerDiag */
    err = getHwRegister(pDis, E_REG_P01_SC_MAX_BUFFILL_P_0_R, &regVal);
    RETIF_REG_FAIL(err)
    pScalerDiag->maxBuffill_p = regVal;
    err = getHwRegister(pDis, E_REG_P01_SC_MAX_BUFFILL_P_1_R, &regVal);
    RETIF_REG_FAIL(err)
    pScalerDiag->maxBuffill_p |= ((UInt16)regVal << 8);

    /* Read the 16-bit MAX_BUFFILL_D register pair and copy to pScalerDiag */
    err = getHwRegister(pDis, E_REG_P01_SC_MAX_BUFFILL_D_0_R, &regVal);
    RETIF_REG_FAIL(err)
    pScalerDiag->maxBuffill_d = regVal;
    err = getHwRegister(pDis, E_REG_P01_SC_MAX_BUFFILL_D_1_R, &regVal);
    RETIF_REG_FAIL(err)
    pScalerDiag->maxBuffill_d |= ((UInt16)regVal << 8);

    /* Read from dummy register to update MAX_FIFOFILL registers */
    err = getHwRegister(pDis, E_REG_P01_SC_SAMPLE_FIFOFILL_R, &regVal);
    RETIF_REG_FAIL(err)

    /* Read the 8-bit FIFOFILL registers directly to pScalerDiag */
    err = getHwRegisters(pDis, E_REG_P01_SC_MAX_FIFOFILL_PI_R, 
                        &pScalerDiag->maxFifofill_pi, 7);
    return err;

#else /* TMFL_SCALER_SUPPORT */
    DUMMY_ACCESS(pScalerDiag); /* else not referenced */
    regVal = regVal; /* else is declared but not used */
    return TMBSL_ERR_HDMI_NOT_SUPPORTED;
#endif /* TMFL_SCALER_SUPPORT */

}


/*============================================================================*/
/* tmbslTDA9989ScalerGetMode                                                   */
/*============================================================================*/
tmErrorCode_t
tmbslTDA9989ScalerGetMode
(
    tmUnitSelect_t            txUnit,
    tmbslHdmiTxScaMode_t      *pScalerMode
)
{
    tmHdmiTxobject_t *pDis;       /* Pointer to Device Instance Structure */
    tmErrorCode_t   err;          /* Error code */

    /* Check unit parameter and point to TX unit object */
    err = checkUnitSetDis(txUnit, &pDis);
    RETIF(err != TM_OK, err)

#ifdef TMFL_SCALER_SUPPORT
    /* Check if this device has a Scaler */
    RETIF(pDis->uDeviceFeatures & E_MASKREG_P00_VERSION_not_s,
          TMBSL_ERR_HDMI_NOT_SUPPORTED)

    /* Check parameters */
    RETIF_BADPARAM(pScalerMode == Null)

    /* Set output parameter */
    *pScalerMode = pDis->scaMode;
    return TM_OK;

#else /* TMFL_SCALER_SUPPORT */
    DUMMY_ACCESS(pDis);/* else is declared but not used */
    DUMMY_ACCESS(err); /* else is declared but not used */
    DUMMY_ACCESS(pScalerMode); /* else is declared but not used */
    return TMBSL_ERR_HDMI_NOT_SUPPORTED;

#endif /* TMFL_SCALER_SUPPORT */
}

/*============================================================================*/
/* tmbslTDA9989ScalerInDisable                                                 */
/*============================================================================*/
tmErrorCode_t
tmbslTDA9989ScalerInDisable
(
    tmUnitSelect_t  txUnit,
    Bool            bDisable
)
{
    tmHdmiTxobject_t *pDis;       /* Pointer to Device Instance Structure */
    tmErrorCode_t     err;        /* Error code */


    /* Check unit parameter and point to TX unit object */
    err = checkUnitSetDis(txUnit, &pDis);
    RETIF(err != TM_OK, err)

#ifdef TMFL_SCALER_SUPPORT
    /* Check if this device has a Scaler */
    RETIF(pDis->uDeviceFeatures & E_MASKREG_P00_VERSION_not_s,
          TMBSL_ERR_HDMI_NOT_SUPPORTED)

    /* Check parameters */
    RETIF_BADPARAM(bDisable > True)

    /* Set or clear frame_dis in the scaler Timebase Control 0 register 
     * according to bDisable
     */
    err = setHwRegisterField(pDis,
                             E_REG_P01_TBG_CNTRL_0_W,
                             E_MASKREG_P01_TBG_CNTRL_0_frame_dis,
                             (UInt8)bDisable);
    return err;

#else /* TMFL_SCALER_SUPPORT */
    DUMMY_ACCESS(pDis); /* else is declared but not used */
    DUMMY_ACCESS(err); /* else is declared but not used */
    DUMMY_ACCESS(bDisable);
    return TMBSL_ERR_HDMI_NOT_SUPPORTED;

#endif /* TMFL_SCALER_SUPPORT */
}

/*============================================================================*/
/* tmbslTDA9989ScalerSetCoeffs                                                 */
/*============================================================================*/

tmErrorCode_t
tmbslTDA9989ScalerSetCoeffs
(
    tmUnitSelect_t        txUnit,
    tmbslHdmiTxScaLut_t   lutSel,
    UInt8                *pVsLut
)
{
    tmHdmiTxobject_t *pDis;       /* Pointer to Device Instance Structure */
    tmErrorCode_t     err;        /* Error code */

    /* Check unit parameter and point to TX unit object */
    err = checkUnitSetDis(txUnit, &pDis);
    RETIF(err != TM_OK, err)

#ifdef TMFL_SCALER_SUPPORT
    /* Check if this device has a Scaler */
    RETIF(pDis->uDeviceFeatures & E_MASKREG_P00_VERSION_not_s,
          TMBSL_ERR_HDMI_NOT_SUPPORTED)

    /* Check parameters */
    RETIF_BADPARAM(lutSel >= HDMITX_SCALUT_INVALID)

    if (lutSel == HDMITX_SCALUT_USE_VSLUT)
    {
        /* Table pointer must be valid here */
        RETIF(pVsLut == Null, TMBSL_ERR_HDMI_INCONSISTENT_PARAMS)

        /* Set LUT coefficient table */
        err = setHwRegisters(pDis,
                             E_REG_P01_SC_VS_LUT_0_W,
                             pVsLut,
                             HDMITX_VSLUT_COEFF_NUM);
        RETIF_REG_FAIL(err)
    }
    else
    {
        err = setHwRegisterField(pDis,
                                 E_REG_P01_SC_VIDFORMAT_W,
                                 E_MASKREG_P01_SC_VIDFORMAT_lut_sel,
                                 (UInt8)lutSel);
        RETIF_REG_FAIL(err)
    }
    return TM_OK;

#else /* TMFL_SCALER_SUPPORT */
    DUMMY_ACCESS(pDis);/* else is declared but not used */
    DUMMY_ACCESS(err); /* else is declared but not used */
    DUMMY_ACCESS(lutSel); /* else is declared but not used */
    DUMMY_ACCESS(pVsLut); /* else is declared but not used */
    return TMBSL_ERR_HDMI_NOT_SUPPORTED;

#endif /* TMFL_SCALER_SUPPORT */
}


/*============================================================================*/
/* tmbslTDA9989ScalerSetFieldOrder                                             */
/*============================================================================*/
tmErrorCode_t   
tmbslTDA9989ScalerSetFieldOrder 
(
    tmUnitSelect_t      txUnit,
    tmbslHdmiTxIntExt_t topExt,
    tmbslHdmiTxIntExt_t deExt,
    tmbslHdmiTxTopSel_t topSel,
    tmbslHdmiTxTopTgl_t topTgl 
)
{
    tmHdmiTxobject_t *pDis;       /* Pointer to Device Instance Structure */
    tmErrorCode_t     err;        /* Error code */

    /* Check unit parameter and point to TX unit object */
    err = checkUnitSetDis(txUnit, &pDis);
    RETIF(err != TM_OK, err)

#ifdef TMFL_SCALER_SUPPORT
    /* Check if this device has a Scaler */
    RETIF(pDis->uDeviceFeatures & E_MASKREG_P00_VERSION_not_s,
          TMBSL_ERR_HDMI_NOT_SUPPORTED)

    /* Check parameters */
    RETIF_BADPARAM(topExt > HDMITX_INTEXT_NO_CHANGE)
    RETIF_BADPARAM(deExt  > HDMITX_INTEXT_NO_CHANGE)
    RETIF_BADPARAM(topSel > HDMITX_TOPSEL_NO_CHANGE)
    RETIF_BADPARAM(topTgl > HDMITX_TOPTGL_NO_CHANGE)

    /* Set each optional parameter */
    if (topExt != HDMITX_INTEXT_NO_CHANGE)
    {
        err = setHwRegisterField(pDis,
                                 E_REG_P01_TBG_CNTRL_0_W,
                                 E_MASKREG_P01_TBG_CNTRL_0_top_ext,
                                 (UInt8)topExt);
        RETIF_REG_FAIL(err)
    }
    if (deExt != HDMITX_INTEXT_NO_CHANGE)
    {
        err = setHwRegisterField(pDis,
                                 E_REG_P01_TBG_CNTRL_0_W,
                                 E_MASKREG_P01_TBG_CNTRL_0_de_ext,
                                 (UInt8)deExt);
        RETIF_REG_FAIL(err)
    }
    if (topSel != HDMITX_TOPSEL_NO_CHANGE)
    {
        err = setHwRegisterField(pDis,
                                 E_REG_P01_TBG_CNTRL_0_W,
                                 E_MASKREG_P01_TBG_CNTRL_0_top_sel,
                                 (UInt8)topSel);
        RETIF_REG_FAIL(err)
    }
    if (topTgl != HDMITX_TOPTGL_NO_CHANGE)
    {
        err = setHwRegisterField(pDis,
                                 E_REG_P01_TBG_CNTRL_0_W,
                                 E_MASKREG_P01_TBG_CNTRL_0_top_tgl,
                                 (UInt8)topTgl);
        RETIF_REG_FAIL(err)
    }
    return TM_OK;

#else /* TMFL_SCALER_SUPPORT */
    DUMMY_ACCESS(pDis);/* else is declared but not used */
    DUMMY_ACCESS(err); /* else is declared but not used */
    DUMMY_ACCESS(deExt); /* else is declared but not used */
    DUMMY_ACCESS(topExt); /* else is declared but not used */
    DUMMY_ACCESS(topSel); /* else is declared but not used */
    DUMMY_ACCESS(topTgl); /* else is declared but not used */
    return TMBSL_ERR_HDMI_NOT_SUPPORTED;

#endif /* TMFL_SCALER_SUPPORT */
}

/*============================================================================*/
/* tmbslTDA9989ScalerSetPhase                                                 */
/*============================================================================*/
tmErrorCode_t   
tmbslTDA9989ScalerSetPhase
(
    tmUnitSelect_t        txUnit,
    tmbslHdmiTxHPhases_t  horizontalPhases   
)
{
    tmHdmiTxobject_t *pDis;       /* Pointer to Device Instance Structure */
    tmErrorCode_t     err;        /* Error code */

    /* Check unit parameter and point to TX unit object */
    err = checkUnitSetDis(txUnit, &pDis);
    RETIF(err != TM_OK, err)

#ifdef TMFL_SCALER_SUPPORT
    /* Check if this device has a Scaler */
    RETIF(pDis->uDeviceFeatures & E_MASKREG_P00_VERSION_not_s,
          TMBSL_ERR_HDMI_NOT_SUPPORTED)

    /* Check parameters */
    RETIF_BADPARAM(horizontalPhases  >= HDMITX_H_PHASES_INVALID)

    err = setHwRegisterField(pDis,
                            E_REG_P01_SC_CNTRL_W,
                            E_MASKREG_P01_SC_CNTRL_phases_h,
                            (UInt8)horizontalPhases);

    RETIF_REG_FAIL(err)
    return TM_OK;

#else /* TMFL_SCALER_SUPPORT */
    DUMMY_ACCESS(pDis);/* else is declared but not used */
    DUMMY_ACCESS(err); /* else is declared but not used */
    DUMMY_ACCESS(horizontalPhases); /* else is declared but not used */
    return TMBSL_ERR_HDMI_NOT_SUPPORTED;

#endif /* TMFL_SCALER_SUPPORT */

}

/*============================================================================*/
/* tmbslTDA9989ScalerSetLatency                                                 */
/*============================================================================*/
tmErrorCode_t   
tmbslTDA9989ScalerSetLatency
(
    tmUnitSelect_t        txUnit,
    UInt8                 scaler_latency
)
{
    tmHdmiTxobject_t *pDis;       /* Pointer to Device Instance Structure */
    tmErrorCode_t     err;        /* Error code */

    /* Check unit parameter and point to TX unit object */
    err = checkUnitSetDis(txUnit, &pDis);
    RETIF(err != TM_OK, err)

#ifdef TMFL_SCALER_SUPPORT
    /* Check if this device has a Scaler */
    RETIF(pDis->uDeviceFeatures & E_MASKREG_P00_VERSION_not_s,
          TMBSL_ERR_HDMI_NOT_SUPPORTED)

    /* Check parameters */
    RETIF_BADPARAM(scaler_latency > HDMITX_LAT_SCO_MAX_VAL)
    RETIF_BADPARAM(scaler_latency < HDMITX_LAT_SCO_MIN_VAL)
    RETIF_BADPARAM(scaler_latency%2)

    err = setHwRegister(pDis,
                        E_REG_P01_SC_LAT_SCO_RW,
                        scaler_latency);

    RETIF_REG_FAIL(err)
    return TM_OK;

#else /* TMFL_SCALER_SUPPORT */
    DUMMY_ACCESS(pDis);/* else is declared but not used */
    DUMMY_ACCESS(err); /* else is declared but not used */
    DUMMY_ACCESS(scaler_latency); /* else is declared but not used */
    return TMBSL_ERR_HDMI_NOT_SUPPORTED;

#endif /* TMFL_SCALER_SUPPORT */
 
}

/*============================================================================*/
/* tmbslTDA9989ScalerSetFine                                                   */
/*============================================================================*/
tmErrorCode_t   
tmbslTDA9989ScalerSetFine
(
    tmUnitSelect_t  txUnit,
    UInt16          uRefPix,    
    UInt16          uRefLine   
)
{
    tmHdmiTxobject_t *pDis;       /* Pointer to Device Instance Structure */
    tmErrorCode_t     err;        /* Error code */

    /* Check unit parameter and point to TX unit object */
    err = checkUnitSetDis(txUnit, &pDis);
    RETIF(err != TM_OK, err)

#ifdef TMFL_SCALER_SUPPORT
    /* Check if this device has a Scaler */
    RETIF(pDis->uDeviceFeatures & E_MASKREG_P00_VERSION_not_s,
          TMBSL_ERR_HDMI_NOT_SUPPORTED)

    /* Check parameters */
    RETIF_BADPARAM(uRefPix  >= HDMITX_SCALER_FINE_PIXEL_INVALID)
    RETIF_BADPARAM(uRefLine >= HDMITX_SCALER_FINE_LINE_INVALID)

    if (uRefPix < HDMITX_SCALER_FINE_PIXEL_NO_CHANGE)
    {
        err = setHwRegisterMsbLsb(pDis, E_REG_P01_REFPIX_MSB_W, uRefPix);
        RETIF_REG_FAIL(err)
    }
    if (uRefLine < HDMITX_SCALER_FINE_LINE_NO_CHANGE)
    {
        err = setHwRegisterMsbLsb(pDis, E_REG_P01_REFLINE_MSB_W, uRefLine);
        RETIF_REG_FAIL(err)
    }
    return TM_OK;

#else /* TMFL_SCALER_SUPPORT */
    DUMMY_ACCESS(pDis);/* else is declared but not used */
    DUMMY_ACCESS(err); /* else is declared but not used */
    DUMMY_ACCESS(uRefPix); /* else is declared but not used */
    DUMMY_ACCESS(uRefLine); /* else is declared but not used */
    return TMBSL_ERR_HDMI_NOT_SUPPORTED;

#endif /* TMFL_SCALER_SUPPORT */
}

/*============================================================================*/
/* tmbslTDA9989ScalerSetSync                                                   */
/*============================================================================*/
tmErrorCode_t   
tmbslTDA9989ScalerSetSync
(
    tmUnitSelect_t      txUnit,
    tmbslHdmiTxVsMeth_t method,
    tmbslHdmiTxVsOnce_t once
)
{
    tmHdmiTxobject_t *pDis;       /* Pointer to Device Instance Structure */
    tmErrorCode_t     err;        /* Error code */

    /* Check unit parameter and point to TX unit object */
    err = checkUnitSetDis(txUnit, &pDis);
    RETIF(err != TM_OK, err)

#ifdef TMFL_SCALER_SUPPORT
    /* Check if this device has a Scaler */
    RETIF(pDis->uDeviceFeatures & E_MASKREG_P00_VERSION_not_s,
          TMBSL_ERR_HDMI_NOT_SUPPORTED)

    /* Check parameters */
    RETIF_BADPARAM(method >= HDMITX_VSMETH_INVALID)
    RETIF_BADPARAM(once   >= HDMITX_VSONCE_INVALID)

    /* Set each optional parameter */
    if (method != HDMITX_VSMETH_NO_CHANGE)
    {
        err = setHwRegisterField(pDis,
                                 E_REG_P01_TBG_CNTRL_0_W,
                                 E_MASKREG_P01_TBG_CNTRL_0_sync_mthd,
                                 (UInt8)method);
        RETIF_REG_FAIL(err)
    }
    if (once != HDMITX_VSONCE_NO_CHANGE)
    {
        /* Must be last register set */
        err = setHwRegisterField(pDis,
                                 E_REG_P01_TBG_CNTRL_0_W,
                                 E_MASKREG_P01_TBG_CNTRL_0_sync_once,
                                 (UInt8)once);
        RETIF_REG_FAIL(err)
        if (once == HDMITX_VSONCE_ONCE)
        {
            /* Toggle output Sync Once flag for settings to take effect */
            err = setHwRegisterField(pDis,
                                     E_REG_P01_TBG_CNTRL_0_W,
                                     E_MASKREG_P01_TBG_CNTRL_0_sync_once,
                                     (UInt8)HDMITX_VSONCE_EACH_FRAME);
            RETIF_REG_FAIL(err)
            err = setHwRegisterField(pDis,
                                     E_REG_P01_TBG_CNTRL_0_W,
                                     E_MASKREG_P01_TBG_CNTRL_0_sync_once,
                                     (UInt8)HDMITX_VSONCE_ONCE);
            RETIF_REG_FAIL(err)
        }
    }
    return TM_OK;

#else /* TMFL_SCALER_SUPPORT */
    DUMMY_ACCESS(pDis);/* else is declared but not used */
    DUMMY_ACCESS(err); /* else is declared but not used */
    DUMMY_ACCESS(method); /* else is declared but not used */
    DUMMY_ACCESS(once); /* else is declared but not used */
    return TMBSL_ERR_HDMI_NOT_SUPPORTED;

#endif /* TMFL_SCALER_SUPPORT */
}

/*============================================================================*/
/* tmbslTDA9989TmdsSetOutputs                                                  */
/*============================================================================*/
tmErrorCode_t   
tmbslTDA9989TmdsSetOutputs 
(
    tmUnitSelect_t          txUnit,
    tmbslHdmiTxTmdsOut_t    tmdsOut
)
{
    tmHdmiTxobject_t *pDis;       /* Pointer to Device Instance Structure */
    tmErrorCode_t     err;        /* Error code */

    /* Check unit parameter and point to TX unit object */
    err = checkUnitSetDis(txUnit, &pDis);
    RETIF(err != TM_OK, err)

    /* Check parameters */
    RETIF_BADPARAM(tmdsOut >= HDMITX_TMDSOUT_INVALID)

    /* Set the TMDS output mode */
    err = setHwRegisterField(pDis,
                             E_REG_P02_BUFFER_OUT_RW,
                             E_MASKREG_P02_BUFFER_OUT_srl_force,
                             (UInt8)tmdsOut);
    return err;
}

/*============================================================================*/
/* tmbslTDA9989TmdsSetSerializer                                               */
/*============================================================================*/
tmErrorCode_t   
tmbslTDA9989TmdsSetSerializer 
(
    tmUnitSelect_t  txUnit,
    UInt8           uPhase2,
    UInt8           uPhase3
)
{

    DUMMY_ACCESS(txUnit);
    DUMMY_ACCESS(uPhase2);
    DUMMY_ACCESS(uPhase3);

    /*
    tmHdmiTxobject_t *pDis;       * Pointer to Device Instance Structure *
    tmErrorCode_t     err;        * Error code *

    * Check unit parameter and point to TX unit object 
    err = checkUnitSetDis(txUnit, &pDis);
    RETIF(err != TM_OK, err)

    * Check parameters 
    RETIF_BADPARAM(uPhase2 >= HDMITX_TMDSPHASE_INVALID)
    RETIF_BADPARAM(uPhase3 >= HDMITX_TMDSPHASE_INVALID)

     Set the serializer phase 2 & 3 counts 
    err = setHwRegisterField(pDis,
                             E_REG_P02_SERIALIZER_RW,
                             E_MASKREG_P02_SERIALIZER_srl_phase2,
                             uPhase2);
    RETIF_REG_FAIL(err)
    err = setHwRegisterField(pDis,
                             E_REG_P02_SERIALIZER_RW,
                             E_MASKREG_P02_SERIALIZER_srl_phase3,
                             uPhase3);
    */
    return TMBSL_ERR_HDMI_NOT_SUPPORTED;
}

/*============================================================================*/
/* tmbslTDA9989TestSetPattern                                                  */
/*============================================================================*/
tmErrorCode_t   
tmbslTDA9989TestSetPattern
(
    tmUnitSelect_t            txUnit,
    tmbslHdmiTxTestPattern_t pattern 
)
{
    tmHdmiTxobject_t *pDis;						/* Pointer to Device Instance Structure */
    tmErrorCode_t     err;						/* Error code */
    UInt8             serviceMode;				/* Register value */
    UInt8             bars8;					/* Register value */
	UInt8			  buf[MATRIX_PRESET_SIZE];	/* Temp buffer */
	UInt8			  i;						/* Loop index */
	UInt8			  *MatrixCoeff=Null;    

    /* Check unit parameter and point to TX unit object */
    err = checkUnitSetDis(txUnit, &pDis);
    RETIF(err != TM_OK, err)

    /* Check pattern parameters */
    switch (pattern)
    {
    case HDMITX_PATTERN_CBAR4:
        serviceMode = 1;
        bars8 = 0;
        break;
	case HDMITX_PATTERN_BLUE:
		MatrixCoeff = (UInt8*)&MatrixCoeffBlueScreen[pDis->voutMode][0]; //point to the blue matrix 
        serviceMode = 1;
        bars8 = 1;
        break;
	case HDMITX_PATTERN_BLACK:
		MatrixCoeff = (UInt8*)&MatrixCoeffBlackScreen[pDis->voutMode][0]; //point to the black matrix 
    case HDMITX_PATTERN_CBAR8:
        serviceMode = 1;
        bars8 = 1;
        break;
    case HDMITX_PATTERN_OFF:
        serviceMode = 0;
        bars8 = 0;
        break;
    default:
        return TMBSL_ERR_HDMI_BAD_PARAMETER;
    }

	if (serviceMode)
	{
		if (!pDis->prevPattern) /* if a pattern is on, registers are already saved */
		{
			/* The kBypassColourProc registers are saved in tmbslTDA9989VideoSetInOut API */
			/* Bypass up/down sampler and matrix for RGB colourbars */
			setHwRegisterFieldTable(pDis, &kBypassColourProc[0]);
		}
		if (( pattern == HDMITX_PATTERN_BLUE )||( pattern == HDMITX_PATTERN_BLACK )) /* blue or black screen pattern */
		{

			/* To create blue or black screen, we use the internal color bar 8 on which we apply a matrix to change it to blue or black */
			/* Set the first block byte separately, as it is shadowed and can't
			* be set by setHwRegisters */

			/* Set the first block byte separately, as it is shadowed and can't
			* be set by setHwRegisters */
			err = setHwRegister(pDis,					
								E_REG_P00_MAT_CONTRL_W,
								MatrixCoeff[0]);
			RETIF_REG_FAIL(err)

			for (i = 0; i < MATRIX_PRESET_SIZE; i++)
			{
				buf[i] = MatrixCoeff[i];
			}
			
			/* Set the rest of the block */
			err = setHwRegisters(pDis,
								E_REG_P00_MAT_OI1_MSB_W,
								&buf[1],
								MATRIX_PRESET_SIZE - 1);
			RETIF_REG_FAIL(err)
			pDis->prevFilterPattern = True;
		}
		else /* colour bars patterns */
		{
			/* Set number of colour bars */
			err = setHwRegisterField(pDis, 
									E_REG_P00_HVF_CNTRL_0_W,
									E_MASKREG_P00_HVF_CNTRL_0_rwb,
									bars8);
			RETIF_REG_FAIL(err)
			
			/* Bypass up/down sampler and matrix for RGB colourbars */
			setHwRegisterFieldTable(pDis, &kBypassColourProc[0]);
		}
		pDis->prevPattern = True;
	}
	else /* serviceMode == 0 */
	{
		if (pDis->prevFilterPattern)
		{
			/* Restore the previous Matrix when pattern goes off */
			err = tmbslTDA9989MatrixSetConversion ( txUnit, pDis->vinFmt, pDis->vinMode, pDis->voutFmt, pDis->voutMode,pDis->dviVqr);
			RETIF_REG_FAIL(err)

			pDis->prevFilterPattern = False;
		}
		/* Restore kBypassColourProc registers when pattern goes off */
		setHwRegister(pDis, E_REG_P00_MAT_CONTRL_W,  gMatContrl[txUnit]);
		setHwRegister(pDis, E_REG_P00_HVF_CNTRL_0_W, gHvfCntrl0[txUnit]);
		setHwRegister(pDis, E_REG_P00_HVF_CNTRL_1_W, gHvfCntrl1[txUnit]);
		pDis->prevPattern = False;
	}

    /* Set Service Mode on or off */
    err = setHwRegisterField(pDis, 
                             E_REG_P00_HVF_CNTRL_0_W,
                             E_MASKREG_P00_HVF_CNTRL_0_sm,
                             serviceMode);
#ifdef TMFL_HDCP_SUPPORT
   pDis->HDCPIgnoreEncrypt = True;  /* Skip the next encrypt IT */
#endif /* TMFL_HDCP_SUPPORT */

    return err;
}

/*============================================================================*/
/* tmbslTDA9989VideoInSetBlanking                                              */
/*============================================================================*/
tmErrorCode_t
tmbslTDA9989VideoInSetBlanking
(
    tmUnitSelect_t         txUnit,
    tmbslHdmiTxBlnkSrc_t   blankitSource,
    tmbslHdmiTxBlnkCode_t  blankingCodes
)
{
    tmHdmiTxobject_t *pDis;       /* Pointer to Device Instance Structure */
    tmErrorCode_t     err;        /* Error code */

    /* Check unit parameter and point to TX unit object */
    err = checkUnitSetDis(txUnit, &pDis);
    RETIF(err != TM_OK, err)

    /* Check parameters */
    RETIF_BADPARAM(blankitSource >= HDMITX_BLNKSRC_INVALID)
    RETIF_BADPARAM(blankingCodes >= HDMITX_BLNKCODE_INVALID)

    /* For each parameter that is not No Change, set its register */
    if (blankitSource != HDMITX_BLNKSRC_NO_CHANGE)
    {
        err = setHwRegisterField(pDis,
                                 E_REG_P00_VIP_CNTRL_4_W,
                                 E_MASKREG_P00_VIP_CNTRL_4_blankit,
                                 (UInt8)blankitSource);
        RETIF_REG_FAIL(err)
    }
    if (blankingCodes != HDMITX_BLNKCODE_NO_CHANGE)
    {
        err = setHwRegisterField(pDis,
                                 E_REG_P00_VIP_CNTRL_4_W,
                                 E_MASKREG_P00_VIP_CNTRL_4_blc,
                                 (UInt8)blankingCodes);
        RETIF_REG_FAIL(err)
    }

    return TM_OK;
}

/*============================================================================*/
/* tmbslTDA9989VideoInSetConfig                                                */
/*============================================================================*/
tmErrorCode_t
tmbslTDA9989VideoInSetConfig
(
    tmUnitSelect_t             txUnit,
    tmbslHdmiTxVinMode_t       vinMode,
    tmbslHdmiTxVidFmt_t        voutFmt,
    tmbslHdmiTxPixEdge_t       sampleEdge,
    tmbslHdmiTxPixRate_t       pixRate,
    tmbslHdmiTxUpsampleMode_t  upsampleMode 
)
{
    tmHdmiTxobject_t *pDis;       /* Pointer to Device Instance Structure */
    tmErrorCode_t     err;        /* Error code */

    /* Check unit parameter and point to TX unit object */
    err = checkUnitSetDis(txUnit, &pDis);
    RETIF(err != TM_OK, err)

    /* Check parameters */
    RETIF_BADPARAM(vinMode      >= HDMITX_VINMODE_INVALID)
    RETIF_BADPARAM(sampleEdge   >= HDMITX_PIXEDGE_INVALID)
    RETIF_BADPARAM(pixRate      >= HDMITX_PIXRATE_INVALID)
    RETIF_BADPARAM(upsampleMode >= HDMITX_UPSAMPLE_INVALID)

    err = InputConfig(pDis,
                       vinMode,
                       sampleEdge,
                       pixRate,
                       upsampleMode, 
                       HDMITX_PIXREP_NO_CHANGE,
                       voutFmt
                       );
    RETIF_REG_FAIL(err)

    return TM_OK;
}
/*============================================================================*/
/* tmbslTDA9989VideoInSetFine                                                  */
/*============================================================================*/
tmErrorCode_t
tmbslTDA9989VideoInSetFine
(
    tmUnitSelect_t            txUnit,
    tmbslHdmiTxPixSubpkt_t    subpacketCount,
    tmbslHdmiTxPixTogl_t      toggleClk1 
)
{
    tmHdmiTxobject_t *pDis;       /* Pointer to Device Instance Structure */
    tmErrorCode_t     err;        /* Error code */

    /* Check unit parameter and point to TX unit object */
    err = checkUnitSetDis(txUnit, &pDis);
    RETIF(err != TM_OK, err)

    /* Check parameters */
    RETIF_BADPARAM(subpacketCount >= HDMITX_PIXSUBPKT_INVALID)
    RETIF_BADPARAM(toggleClk1     >= HDMITX_PIXTOGL_INVALID)

    /* IF subpacketCount is Fix at 0/1/2/3 THEN set subpacket count register
     * to 0/1/2/3 and set subpacket sync register to 3
     */
    if (subpacketCount <= HDMITX_PIXSUBPKT_FIX_3)
    {
        err = setHwRegisterField(pDis,
                                 E_REG_P00_VIP_CNTRL_5_W,
                                 E_MASKREG_P00_VIP_CNTRL_5_sp_cnt,
                                 (UInt8)subpacketCount);
        RETIF_REG_FAIL(err)
        err = setHwRegisterField(pDis,
                                 E_REG_P00_VIP_CNTRL_3_W,
                                 E_MASKREG_P00_VIP_CNTRL_3_sp_sync,
                                 HDMITX_PIXSUBPKT_SYNC_FIXED);
        RETIF_REG_FAIL(err)
    }
    /* ELSE IF subpacketCount is Sync by Hemb/ Sync by Rising Edge DE/ 
     * Sync by Rising Edge HS THEN set the unused subpacket count to zero and
     * set subpacket sync register to 0/1/2
     */
    else if (subpacketCount != HDMITX_PIXSUBPKT_NO_CHANGE)
    {
        err = setHwRegisterField(pDis,
                                 E_REG_P00_VIP_CNTRL_5_W,
                                 E_MASKREG_P00_VIP_CNTRL_5_sp_cnt,
                                 HDMITX_PIXSUBPKT_FIX_0);
        RETIF_REG_FAIL(err)
       
        err = setHwRegisterField(pDis,
                                 E_REG_P00_VIP_CNTRL_3_W,
                                 E_MASKREG_P00_VIP_CNTRL_3_sp_sync,
                         (UInt8)(subpacketCount - HDMITX_PIXSUBPKT_SYNC_FIRST));
        RETIF_REG_FAIL(err)
    }

    /* IF toggleClk1 is not No Change THEN set ckcase bitfield */
    if (toggleClk1 != HDMITX_PIXTOGL_NO_CHANGE)
    {
        err = setHwRegisterField(pDis,
                                 E_REG_P00_VIP_CNTRL_5_W,
                                 E_MASKREG_P00_VIP_CNTRL_5_ckcase,
                                 (UInt8)toggleClk1);
        RETIF_REG_FAIL(err)
    }
    return TM_OK;
}

/*============================================================================*/
/* tmbslTDA9989VideoInSetMapping                                               */
/*============================================================================*/
tmErrorCode_t
tmbslTDA9989VideoInSetMapping
(
    tmUnitSelect_t  txUnit,
    UInt8           *pSwapTable,
    UInt8           *pMirrorTable
)
{
    tmHdmiTxobject_t *pDis;       /* Pointer to Device Instance Structure */
    tmErrorCode_t     err;        /* Error code */
    Int               i;          /* Loop counter */

    /* Check unit parameter and point to TX unit object */
    err = checkUnitSetDis(txUnit, &pDis);
    RETIF(err != TM_OK, err)

    /* Check parameters */
    RETIF_BADPARAM(pSwapTable == Null)
    RETIF_BADPARAM(pMirrorTable == Null)
    for (i = 0; i < HDMITX_VIN_PORT_MAP_TABLE_LEN; i++)
    {
        RETIF_BADPARAM(pSwapTable[i] >= HDMITX_VIN_PORT_SWAP_INVALID)
        RETIF_BADPARAM(pMirrorTable[i] >= HDMITX_VIN_PORT_MIRROR_INVALID)
    }

    /* IF pswapTable[n] is not No Change THEN set the port swap registers from
     * pswapTable[n]
     */
    for (i = 0; i < HDMITX_VIN_PORT_MAP_TABLE_LEN; i++)
    {
        if (pSwapTable[0] < HDMITX_VIN_PORT_SWAP_NO_CHANGE)
        {
            err = setHwRegisterField(pDis,
                                     kRegVip[i].Register,
                                     kRegVip[i].MaskSwap,
                                     pSwapTable[i]);
            RETIF_REG_FAIL(err)
        }
    }

    /* IF pMirrorTable[n] is not No Change THEN set the port mirror registers
     * from pMirrorTable[n]
     */
    for (i = 0; i < HDMITX_VIN_PORT_MAP_TABLE_LEN; i++)
    {
        if (pMirrorTable[0] < HDMITX_VIN_PORT_MIRROR_NO_CHANGE)
        {
            err = setHwRegisterField(pDis,
                                     kRegVip[i].Register,
                                     kRegVip[i].MaskMirror,
                                     pMirrorTable[i]);
            RETIF_REG_FAIL(err)
        }
    }

    return TM_OK;
}

/*============================================================================*/
/* tmbslTDA9989SetVideoPortConfig                                              */
/*============================================================================*/
tmErrorCode_t
tmbslTDA9989SetVideoPortConfig
(
    tmUnitSelect_t  txUnit,
    UInt8           *pEnaVideoPortTable,
    UInt8           *pGndVideoPortTable
)
{
    tmHdmiTxobject_t *pDis;       /* Pointer to Device Instance Structure */
    tmErrorCode_t     err;        /* Error code */

    /* Check unit parameter and point to TX unit object */
    err = checkUnitSetDis(txUnit, &pDis);
    RETIF(err != TM_OK, err)

    /* Check parameters */
    RETIF_BADPARAM(pEnaVideoPortTable == Null)
    RETIF_BADPARAM(pGndVideoPortTable == Null)

    err = setHwRegister(pDis,
                        E_REG_P00_ENA_VP_0_RW,
                        pEnaVideoPortTable[0]);
    RETIF_REG_FAIL(err)

    err = setHwRegister(pDis,
                        E_REG_P00_ENA_VP_1_RW,
                        pEnaVideoPortTable[1]);
    RETIF_REG_FAIL(err)

    err = setHwRegister(pDis,
                        E_REG_P00_ENA_VP_2_RW,
                        pEnaVideoPortTable[2]);
    RETIF_REG_FAIL(err)

 /*   err = setHwRegister(pDis,
                        E_REG_P00_GND_VP_0_RW,
                        pGndVideoPortTable[0]);
    RETIF_REG_FAIL(err)*/

 /*  err = setHwRegister(pDis,
                        E_REG_P00_GND_VP_1_RW,
                        pGndVideoPortTable[1]);
    RETIF_REG_FAIL(err)*/

 /* err = setHwRegister(pDis,
                        E_REG_P00_GND_VP_2_RW,
                        pGndVideoPortTable[2]);
    RETIF_REG_FAIL(err)*/

    return TM_OK;
}

/*============================================================================*/
/* tmbslTDA9989SetAudioPortConfig                                              */
/*============================================================================*/
tmErrorCode_t
tmbslTDA9989SetAudioPortConfig
(
    tmUnitSelect_t  txUnit,
    UInt8           *pEnaAudioPortTable,
    UInt8           *pGndAudioPortTable
)
{
    tmHdmiTxobject_t *pDis;       /* Pointer to Device Instance Structure */
    tmErrorCode_t     err;        /* Error code */

    /* Check unit parameter and point to TX unit object */
    err = checkUnitSetDis(txUnit, &pDis);
    RETIF(err != TM_OK, err)

    /* Check parameters */
    RETIF_BADPARAM(pEnaAudioPortTable == Null)
    RETIF_BADPARAM(pGndAudioPortTable == Null)

    err = setHwRegister(pDis,
                        E_REG_P00_ENA_AP_RW,
                        pEnaAudioPortTable[0]);
    RETIF_REG_FAIL(err)

 /*   err = setHwRegister(pDis,
                        E_REG_P00_GND_AP_RW,
                        pGndAudioPortTable[0]);
    RETIF_REG_FAIL(err)*/

    return TM_OK;
}

/*============================================================================*/
/* tmbslTDA9989SetAudioClockPortConfig                                              */
/*============================================================================*/
tmErrorCode_t
tmbslTDA9989SetAudioClockPortConfig
(
    tmUnitSelect_t  txUnit,
    UInt8           *pEnaAudioClockPortTable,
    UInt8           *pGndAudioClockPortTable
)
{
    tmHdmiTxobject_t *pDis;       /* Pointer to Device Instance Structure */
    tmErrorCode_t     err;        /* Error code */

    /* Check unit parameter and point to TX unit object */
    err = checkUnitSetDis(txUnit, &pDis);
    RETIF(err != TM_OK, err)

    /* Check parameters */
    RETIF_BADPARAM(pEnaAudioClockPortTable == Null)
    RETIF_BADPARAM(pGndAudioClockPortTable == Null)

    err = setHwRegister(pDis,
                        E_REG_P00_ENA_ACLK_RW,
                        pEnaAudioClockPortTable[0]);
    RETIF_REG_FAIL(err)

    /*err = setHwRegister(pDis,
                        E_REG_P00_GND_ACLK_RW,
                        pGndAudioClockPortTable[0]);
    RETIF_REG_FAIL(err)*/

    return TM_OK;
}

/*============================================================================*/
/* tmbslTDA9989VideoInSetSyncAuto                                              */
/*============================================================================*/
tmErrorCode_t
tmbslTDA9989VideoInSetSyncAuto
(
    tmUnitSelect_t            txUnit,
    tmbslHdmiTxSyncSource_t   syncSource,
    tmbslHdmiTxVidFmt_t       vinFmt,
    tmbslHdmiTxVinMode_t      vinMode
)
{
    tmHdmiTxobject_t *pDis;       /* Pointer to Device Instance Structure */
    tmErrorCode_t     err;        /* Error code */
    UInt8             regVinFmt;  /* Video i/p fmt value used for comparison */
    UInt8             embedded;   /* Register value */
    UInt8             syncMethod; /* Sync method */
    UInt8             toggleV;    /* V toggle */
    UInt8             toggleH;    /* H toggle */
    UInt8             toggleX;    /* X toggle */
    UInt16            uRefPix;    /* Output refpix */
    UInt16            uRefLine;   /* Output refline */
    tmbslHdmiTxVidFmt_t  vinFmtIndex; /* Index in tab kVfmtToRegvfmt_TV*/
    UInt8             regVinFmtNoReg;    
    UInt8             regVal;
#ifdef FORMAT_PC
    UInt8             regVinFmtPC;
#endif /* FORMAT_PC */

    /* Check unit parameter and point to TX unit object */
    err = checkUnitSetDis(txUnit, &pDis);
    RETIF(err != TM_OK, err)

    /* Check parameters - syncSource must be specified */
    RETIF_BADPARAM(syncSource   >= HDMITX_SYNCSRC_NO_CHANGE)
    
#ifdef FORMAT_PC
    RETIF_BADPARAM(vinFmt       < HDMITX_VFMT_TV_MIN)
    RETIF_BADPARAM((vinFmt > HDMITX_VFMT_TV_MAX) && (vinFmt < HDMITX_VFMT_PC_MIN))
    RETIF_BADPARAM(vinFmt       > HDMITX_VFMT_PC_MAX)
#else /* FORMAT_PC */
    /*FORMAT TV only*/
    RETIF_BADPARAM(vinFmt       < HDMITX_VFMT_TV_MIN)
    RETIF_BADPARAM(vinFmt       > HDMITX_VFMT_TV_MAX)
#endif /* FORMAT_PC */
    
    vinFmtIndex = vinFmt;
#ifdef FORMAT_PC
    if (vinFmtIndex > HDMITX_VFMT_PC_MIN)
    {
        vinFmtIndex = (tmbslHdmiTxVidFmt_t)(vinFmtIndex - HDMITX_VFMT_PC_MIN + HDMITX_VFMT_TV_MAX + (tmbslHdmiTxVidFmt_t)1);

        /* CBE 1600x1200 WorkAround */
        err = setHwRegister(pDis, E_REG_P00_VIDFORMAT_W, E_REGVFMT_INVALID);
        RETIF_REG_FAIL(err)
    }



#endif /* FORMAT_PC */

    /* Look up the VIDFORMAT register format from the register format table */
    regVinFmt = kVfmtToRegvfmt_TV[vinFmtIndex];
    /* Quit if the input format does not map to the register format */
    RETIF_BADPARAM(regVinFmt == E_REGVFMT_INVALID)

    /* Select values according to sync source */
    embedded = 0;
    switch (syncSource)
    {
    case HDMITX_SYNCSRC_EXT_VS:
        syncMethod = 0;
        toggleV    = kRegvfmtToVToggle[regVinFmt];
        toggleH    = kRegvfmtToHToggle[regVinFmt];
        toggleX    = 0;
        uRefPix    = kVidFmtToRefpixRefline[regVinFmt].refPixVsSync;
        uRefLine   = kVidFmtToRefpixRefline[regVinFmt].refLineVsSync;
        break;
    case HDMITX_SYNCSRC_EMBEDDED:
        embedded++;
        /* fall thru */
    case HDMITX_SYNCSRC_EXT_VREF:
    default:
        syncMethod = 1;
        toggleV    = 1;
        toggleH    = 1;
        toggleX    = 1;
        uRefPix    = kVidFmtToRefpixRefline[regVinFmt].refPixOtherSync;
        uRefLine   = kVidFmtToRefpixRefline[regVinFmt].refLineOtherSync;
        break;
    }
    /* Table has +1 added to refpix values which are not needed in 
       RGB444, YUV444 and YUV422 modes, but +2 is required in those cases */
    if (vinMode != HDMITX_VINMODE_CCIR656) 
    {
        uRefPix = uRefPix + 2;
    }

    /* ---------------------------------------------------------- */
    /* Synchronicity software workaround issue number 106         */
    /* ---------------------------------------------------------- */
    if (vinMode == HDMITX_VINMODE_CCIR656) {
        if (syncSource == HDMITX_SYNCSRC_EXT_VS) {
            if (pDis->pixRate == HDMITX_PIXRATE_DOUBLE) {

                switch (regVinFmt) {
                case E_REGVFMT_720x480p_60Hz:
                case E_REGVFMT_720x480i_60Hz:
                case E_REGVFMT_720x576p_50Hz:
                case E_REGVFMT_720x576i_50Hz:
                    uRefPix = uRefPix + 1;
                break;
                default:
                /* do nothing */
                break;
                }

            }
        }
    }



    /* Set embedded sync */
    err = setHwRegisterField(pDis,
                             E_REG_P00_VIP_CNTRL_3_W,
                             E_MASKREG_P00_VIP_CNTRL_3_emb,
                             embedded);
    RETIF_REG_FAIL(err)

    /* Set sync method */
    err = setHwRegisterField(pDis,
                             E_REG_P00_TBG_CNTRL_0_W,
                             E_MASKREG_P00_TBG_CNTRL_0_sync_mthd,
                             syncMethod);
    RETIF_REG_FAIL(err)

    /* Set VH toggle */
    err = setHwRegisterField(pDis,
                             E_REG_P00_VIP_CNTRL_3_W,
                             E_MASKREG_P00_VIP_CNTRL_3_v_tgl,
                             toggleV);
    RETIF_REG_FAIL(err)
    err = setHwRegisterField(pDis,
                             E_REG_P00_VIP_CNTRL_3_W,
                             E_MASKREG_P00_VIP_CNTRL_3_h_tgl,
                             toggleH);
    RETIF_REG_FAIL(err)

    /* Set X toggle */
    err = setHwRegisterField(pDis,
                             E_REG_P00_VIP_CNTRL_3_W,
                             E_MASKREG_P00_VIP_CNTRL_3_x_tgl,
                             toggleX);
    RETIF_REG_FAIL(err)

    if ((vinFmt >= HDMITX_VFMT_TV_NO_REG_MIN)
		&& (vinFmt < HDMITX_VFMT_PC_MIN))
    {
        RETIF_BADPARAM( regVinFmt > E_REGVFMT_NUM_TV )
     
        /* Check parameters */
		regVinFmtNoReg = (UInt8)(regVinFmt - E_REGVFMT_FIRST_TV_NO_REG);

        regVal = (UInt8)kVidFmtFormatParam[regVinFmtNoReg].nPix;
        err = setHwRegister(pDis, E_REG_P00_NPIX_LSB_W, regVal);
        RETIF_REG_FAIL(err);

        regVal = (UInt8)(kVidFmtFormatParam[regVinFmtNoReg].nPix>>8);
        err = setHwRegister(pDis, E_REG_P00_NPIX_MSB_W, regVal);
        RETIF_REG_FAIL(err);

        regVal = (UInt8)kVidFmtFormatParam[regVinFmtNoReg].nLine;
        err = setHwRegister(pDis, E_REG_P00_NLINE_LSB_W, regVal);
        RETIF_REG_FAIL(err);

        regVal = (UInt8)(kVidFmtFormatParam[regVinFmtNoReg].nLine>>8);
        err = setHwRegister(pDis, E_REG_P00_NLINE_MSB_W, regVal);
        RETIF_REG_FAIL(err);
        
        regVal = (UInt8)kVidFmtFormatParam[regVinFmtNoReg].VsLineStart;
        err = setHwRegister(pDis, E_REG_P00_VS_LINE_STRT_1_LSB_W, regVal);
        RETIF_REG_FAIL(err);

        regVal = (UInt8)kVidFmtFormatParam[regVinFmtNoReg].VsPixStart;
        err = setHwRegister(pDis, E_REG_P00_VS_PIX_STRT_1_LSB_W, regVal);
        RETIF_REG_FAIL(err);

        regVal = (UInt8)(kVidFmtFormatParam[regVinFmtNoReg].VsPixStart>>8);
        err = setHwRegister(pDis, E_REG_P00_VS_PIX_STRT_1_MSB_W, regVal);
        RETIF_REG_FAIL(err);
        
        regVal = (UInt8)kVidFmtFormatParam[regVinFmtNoReg].VsLineEnd;
        err = setHwRegister(pDis, E_REG_P00_VS_LINE_END_1_LSB_W, regVal);
        RETIF_REG_FAIL(err);

        regVal = (UInt8)kVidFmtFormatParam[regVinFmtNoReg].VsPixEnd;
        err = setHwRegister(pDis, E_REG_P00_VS_PIX_END_1_LSB_W, regVal);
        RETIF_REG_FAIL(err);

        regVal = (UInt8)(kVidFmtFormatParam[regVinFmtNoReg].VsPixEnd>>8);
        err = setHwRegister(pDis, E_REG_P00_VS_PIX_END_1_MSB_W, regVal);
        RETIF_REG_FAIL(err);

        regVal = (UInt8)kVidFmtFormatParam[regVinFmtNoReg].HsStart;
        err = setHwRegister(pDis, E_REG_P00_HS_PIX_START_LSB_W, regVal);
        RETIF_REG_FAIL(err);

        regVal = (UInt8)(kVidFmtFormatParam[regVinFmtNoReg].HsStart>>8);
        err = setHwRegister(pDis, E_REG_P00_HS_PIX_START_MSB_W, regVal);
        RETIF_REG_FAIL(err);

        regVal = (UInt8)kVidFmtFormatParam[regVinFmtNoReg].HsEnd;
        err = setHwRegister(pDis, E_REG_P00_HS_PIX_STOP_LSB_W, regVal);
        RETIF_REG_FAIL(err);

        regVal = (UInt8)(kVidFmtFormatParam[regVinFmtNoReg].HsEnd>>8);
        err = setHwRegister(pDis, E_REG_P00_HS_PIX_STOP_MSB_W, regVal);
        RETIF_REG_FAIL(err);

        regVal = (UInt8)kVidFmtFormatParam[regVinFmtNoReg].vWinStart;
        err = setHwRegister(pDis, E_REG_P00_VWIN_START_1_LSB_W, regVal);
        RETIF_REG_FAIL(err);

        regVal = (UInt8)kVidFmtFormatParam[regVinFmtNoReg].vWinEnd;
        err = setHwRegister(pDis, E_REG_P00_VWIN_END_1_LSB_W, regVal);
        RETIF_REG_FAIL(err);

        regVal = (UInt8)(kVidFmtFormatParam[regVinFmtNoReg].vWinEnd>>8);
        err = setHwRegister(pDis, E_REG_P00_VWIN_END_1_MSB_W, regVal);
        RETIF_REG_FAIL(err);

        regVal = (UInt8)kVidFmtFormatParam[regVinFmtNoReg].DeStart;
        err = setHwRegister(pDis, E_REG_P00_DE_START_LSB_W, regVal);
        RETIF_REG_FAIL(err);

        regVal = (UInt8)(kVidFmtFormatParam[regVinFmtNoReg].DeStart>>8);
        err = setHwRegister(pDis, E_REG_P00_DE_START_MSB_W, regVal);
        RETIF_REG_FAIL(err);

        regVal = (UInt8)kVidFmtFormatParam[regVinFmtNoReg].DeEnd;
        err = setHwRegister(pDis, E_REG_P00_DE_STOP_LSB_W, regVal);
        RETIF_REG_FAIL(err);

        regVal = (UInt8)(kVidFmtFormatParam[regVinFmtNoReg].DeEnd>>8);
        err = setHwRegister(pDis, E_REG_P00_DE_STOP_MSB_W, regVal);
        RETIF_REG_FAIL(err);
    }

                             
#ifdef FORMAT_PC

    if (vinFmt > HDMITX_VFMT_PC_MIN)
    {

        /* Check parameters */
        RETIF_BADPARAM( regVinFmt < E_REGVFMT_NUM_TV )
        
	regVinFmtPC = (UInt8) (regVinFmt - E_REGVFMT_FIRST_PC_FORMAT);

        regVal = (UInt8)kVidFmtToPCFormatParam[regVinFmtPC].nPix;
        err = setHwRegister(pDis, E_REG_P00_NPIX_LSB_W, regVal);
        RETIF_REG_FAIL(err);

        regVal = (UInt8)(kVidFmtToPCFormatParam[regVinFmtPC].nPix>>8);
        err = setHwRegister(pDis, E_REG_P00_NPIX_MSB_W, regVal);
        RETIF_REG_FAIL(err);

        regVal = (UInt8)kVidFmtToPCFormatParam[regVinFmtPC].nLine;
        err = setHwRegister(pDis, E_REG_P00_NLINE_LSB_W, regVal);
        RETIF_REG_FAIL(err);

        regVal = (UInt8)(kVidFmtToPCFormatParam[regVinFmtPC].nLine>>8);
        err = setHwRegister(pDis, E_REG_P00_NLINE_MSB_W, regVal);
        RETIF_REG_FAIL(err);
        
        regVal = (UInt8)kVidFmtToPCFormatParam[regVinFmtPC].VsLineStart;
        err = setHwRegister(pDis, E_REG_P00_VS_LINE_STRT_1_LSB_W, regVal);
        RETIF_REG_FAIL(err);

        regVal = (UInt8)kVidFmtToPCFormatParam[regVinFmtPC].VsPixStart;
        err = setHwRegister(pDis, E_REG_P00_VS_PIX_STRT_1_LSB_W, regVal);
        RETIF_REG_FAIL(err);
        
        regVal = (UInt8)kVidFmtToPCFormatParam[regVinFmtPC].VsLineEnd;
        err = setHwRegister(pDis, E_REG_P00_VS_LINE_END_1_LSB_W, regVal);
        RETIF_REG_FAIL(err);

        regVal = (UInt8)kVidFmtToPCFormatParam[regVinFmtPC].VsPixEnd;
        err = setHwRegister(pDis, E_REG_P00_VS_PIX_END_1_LSB_W, regVal);
        RETIF_REG_FAIL(err);

        regVal = (UInt8)kVidFmtToPCFormatParam[regVinFmtPC].HsStart;
        err = setHwRegister(pDis, E_REG_P00_HS_PIX_START_LSB_W, regVal);
        RETIF_REG_FAIL(err);

        regVal = (UInt8)kVidFmtToPCFormatParam[regVinFmtPC].HsEnd;
        err = setHwRegister(pDis, E_REG_P00_HS_PIX_STOP_LSB_W, regVal);
        RETIF_REG_FAIL(err);

        regVal = (UInt8)(kVidFmtToPCFormatParam[regVinFmtPC].HsEnd>>8);
        err = setHwRegister(pDis, E_REG_P00_HS_PIX_STOP_MSB_W, regVal);
        RETIF_REG_FAIL(err);

        regVal = (UInt8)kVidFmtToPCFormatParam[regVinFmtPC].vWinStart;
        err = setHwRegister(pDis, E_REG_P00_VWIN_START_1_LSB_W, regVal);
        RETIF_REG_FAIL(err);

        regVal = (UInt8)kVidFmtToPCFormatParam[regVinFmtPC].vWinEnd;
        err = setHwRegister(pDis, E_REG_P00_VWIN_END_1_LSB_W, regVal);
        RETIF_REG_FAIL(err);

        regVal = (UInt8)(kVidFmtToPCFormatParam[regVinFmtPC].vWinEnd>>8);
        err = setHwRegister(pDis, E_REG_P00_VWIN_END_1_MSB_W, regVal);
        RETIF_REG_FAIL(err);

        regVal = (UInt8)kVidFmtToPCFormatParam[regVinFmtPC].DeStart;
        err = setHwRegister(pDis, E_REG_P00_DE_START_LSB_W, regVal);
        RETIF_REG_FAIL(err);

        regVal = (UInt8)(kVidFmtToPCFormatParam[regVinFmtPC].DeStart>>8);
        err = setHwRegister(pDis, E_REG_P00_DE_START_MSB_W, regVal);
        RETIF_REG_FAIL(err);

        regVal = (UInt8)kVidFmtToPCFormatParam[regVinFmtPC].DeEnd;
        err = setHwRegister(pDis, E_REG_P00_DE_STOP_LSB_W, regVal);
        RETIF_REG_FAIL(err);

        regVal = (UInt8)(kVidFmtToPCFormatParam[regVinFmtPC].DeEnd>>8);
        err = setHwRegister(pDis, E_REG_P00_DE_STOP_MSB_W, regVal);
        RETIF_REG_FAIL(err);

        regVal = DEPTH_COLOR_PC;
        err = setHwRegisterField(pDis,
                                E_REG_P00_HVF_CNTRL_1_W,
                                E_MASKREG_P00_HVF_CNTRL_1_pad,
                                regVal);

        RETIF_REG_FAIL(err);
    }
#endif /* FORMAT_PC */

    /* Set refpix, refline */
    err = setHwRegisterMsbLsb(pDis, E_REG_P00_REFPIX_MSB_W, uRefPix);
    RETIF_REG_FAIL(err)
    err = setHwRegisterMsbLsb(pDis, E_REG_P00_REFLINE_MSB_W, uRefLine);
    return err;
}

/*============================================================================*/
/* tmbslTDA9989VideoInSetSyncManual                                            */
/*============================================================================*/
tmErrorCode_t
tmbslTDA9989VideoInSetSyncManual
(
    tmUnitSelect_t            txUnit,
    tmbslHdmiTxSyncSource_t   syncSource,
    tmbslHdmiTxVsMeth_t       syncMethod,
    tmbslHdmiTxPixTogl_t      toggleV,
    tmbslHdmiTxPixTogl_t      toggleH,
    tmbslHdmiTxPixTogl_t      toggleX,
    UInt16                    uRefPix,    
    UInt16                    uRefLine   
)
{
    tmHdmiTxobject_t *pDis;       /* Pointer to Device Instance Structure */
    tmErrorCode_t     err;        /* Error code */
    UInt8             embedded;   /* Register value */

    /* Check unit parameter and point to TX unit object */
    err = checkUnitSetDis(txUnit, &pDis);
    RETIF(err != TM_OK, err)

    /* Check parameters */
    RETIF_BADPARAM(syncSource   >= HDMITX_SYNCSRC_INVALID)
    RETIF_BADPARAM(syncMethod   >= HDMITX_VSMETH_INVALID)
    RETIF_BADPARAM(toggleV      >= HDMITX_PIXTOGL_INVALID)
    RETIF_BADPARAM(toggleH      >= HDMITX_PIXTOGL_INVALID)
    RETIF_BADPARAM(toggleX      >= HDMITX_PIXTOGL_INVALID)
    RETIF_BADPARAM(uRefPix  >= HDMITX_VOUT_FINE_PIXEL_INVALID)
    RETIF_BADPARAM(uRefLine >= HDMITX_VOUT_FINE_LINE_INVALID)

    if (syncSource != HDMITX_SYNCSRC_NO_CHANGE)
    {
        if (syncSource == HDMITX_SYNCSRC_EMBEDDED)
        {
            embedded = 1;
        }
        else
        {
            embedded = 0;
        }
        err = setHwRegisterField(pDis,
                                 E_REG_P00_VIP_CNTRL_3_W,
                                 E_MASKREG_P00_VIP_CNTRL_3_emb,
                                 embedded);
        RETIF_REG_FAIL(err)
    }
    if (syncMethod != HDMITX_VSMETH_NO_CHANGE)
    {
        err = setHwRegisterField(pDis,
                                 E_REG_P00_TBG_CNTRL_0_W,
                                 E_MASKREG_P00_TBG_CNTRL_0_sync_mthd,
                                 (UInt8)syncMethod);
        RETIF_REG_FAIL(err)
    }
    if (toggleV != HDMITX_PIXTOGL_NO_CHANGE)
    {
        err = setHwRegisterField(pDis,
                                 E_REG_P00_VIP_CNTRL_3_W,
                                 E_MASKREG_P00_VIP_CNTRL_3_v_tgl,
                                 (UInt8)toggleV);
        RETIF_REG_FAIL(err)
    }
    if (toggleH != HDMITX_PIXTOGL_NO_CHANGE)
    {
        err = setHwRegisterField(pDis,
                                 E_REG_P00_VIP_CNTRL_3_W,
                                 E_MASKREG_P00_VIP_CNTRL_3_h_tgl,
                                 (UInt8)toggleH);
        RETIF_REG_FAIL(err)
    }
    if (toggleX != HDMITX_PIXTOGL_NO_CHANGE)
    {
        err = setHwRegisterField(pDis,
                                 E_REG_P00_VIP_CNTRL_3_W,
                                 E_MASKREG_P00_VIP_CNTRL_3_x_tgl,
                                 (UInt8)toggleX);
        RETIF_REG_FAIL(err)
    }
    if (uRefPix < HDMITX_VOUT_FINE_PIXEL_NO_CHANGE)
    {
        err = setHwRegisterMsbLsb(pDis, E_REG_P00_REFPIX_MSB_W, uRefPix);
        RETIF_REG_FAIL(err)
    }
    if (uRefLine < HDMITX_VOUT_FINE_LINE_NO_CHANGE)
    {
        err = setHwRegisterMsbLsb(pDis, E_REG_P00_REFLINE_MSB_W, uRefLine);
        RETIF_REG_FAIL(err)
    }

    return TM_OK;
}

/*============================================================================*/
/* tmbslTDA9989VideoOutDisable                                                 */
/*============================================================================*/

tmErrorCode_t
tmbslTDA9989VideoOutDisable
(
    tmUnitSelect_t  txUnit,
    Bool            bDisable
)
{
    tmHdmiTxobject_t *pDis;       /* Pointer to Device Instance Structure */
    tmErrorCode_t     err;        /* Error code */

    /* Check unit parameter and point to TX unit object */
    err = checkUnitSetDis(txUnit, &pDis);
    RETIF(err != TM_OK, err)

    /* Check parameters */
    RETIF_BADPARAM(bDisable > True)

    /* Set or clear frame_dis in the scaler Timebase Control 0 register 
     * according to bDisable
     */
    err = setHwRegisterField(pDis,
                             E_REG_P00_TBG_CNTRL_0_W,
                             E_MASKREG_P00_TBG_CNTRL_0_frame_dis,
                             (UInt8)bDisable);
    if (bDisable)
    {
        setState(pDis, EV_OUTDISABLE);
    }
    return err;
}


/*============================================================================*/
/* tmbslTDA9989VideoOutSetConfig                                               */
/*============================================================================*/
tmErrorCode_t
tmbslTDA9989VideoOutSetConfig
(
    tmUnitSelect_t            txUnit,
    tmbslHdmiTxSinkType_t     sinkType,
    tmbslHdmiTxVoutMode_t     voutMode,
    tmbslHdmiTxVoutPrefil_t   preFilter,
    tmbslHdmiTxVoutYuvBlnk_t  yuvBlank,
    tmbslHdmiTxVoutQrange_t   quantization 
)
{
    tmHdmiTxobject_t *pDis;       /* Pointer to Device Instance Structure */
    tmErrorCode_t     err;        /* Error code */
    UInt8             regVal;     /* Register value */

    /* Check unit parameter and point to TX unit object */
    err = checkUnitSetDis(txUnit, &pDis);
    RETIF(err != TM_OK, err)

    /* Check parameters */
    RETIF_BADPARAM(sinkType     >= HDMITX_SINK_INVALID)
    RETIF_BADPARAM(voutMode     >= HDMITX_VOUTMODE_INVALID)
    RETIF_BADPARAM(preFilter    >= HDMITX_VOUT_PREFIL_INVALID)
    RETIF_BADPARAM(yuvBlank     >= HDMITX_VOUT_YUV_BLNK_INVALID)
    RETIF_BADPARAM(quantization >= HDMITX_VOUT_QRANGE_INVALID)

    if (sinkType == HDMITX_SINK_EDID)
    {
        if (pDis->EdidStatus == HDMITX_EDID_NOT_READ)
        {
            /* EDID has not been read so assume simplest sink */
            pDis->sinkType = HDMITX_SINK_DVI;
        }
        else
        {
            /* EDID has been read so set sink to the type that was read */
            pDis->sinkType = pDis->EdidSinkType;
        }
    }
    else
    {
        /* Set demanded sink type */
        pDis->sinkType = sinkType;
    }

    /* Is DVI sink required? */
    if (pDis->sinkType == HDMITX_SINK_DVI)
    {
        /* Mute the audio FIFO */
        err = setHwRegisterField(pDis,
                                 E_REG_P11_AIP_CNTRL_0_RW,
                                 E_MASKREG_P11_AIP_CNTRL_0_rst_fifo,
                                 1);
        RETIF_REG_FAIL(err)

        /* Force RGB mode for DVI sink */
        voutMode = HDMITX_VOUTMODE_RGB444;

        /* Set HDMI HDCP mode off for DVI */
        err = setHwRegisterFieldTable(pDis, &kVoutHdcpOff[0]);
        RETIF_REG_FAIL(err)

#ifdef TMFL_HDCP_SUPPORT
#ifdef __LINUX_ARM_ARCH__
         {
            int rej_f2(tmHdmiTxobject_t *pDis);
            regVal = rej_f2(pDis);
         }
#endif

#else
         regVal = 0;
#endif /*TMFL_HDCP_SUPPORT*/
        err = setHwRegisterField(pDis,
                                 E_REG_P11_ENC_CNTRL_RW,
                                 E_MASKREG_P11_ENC_CNTRL_ctl_code,
                                 regVal);
        RETIF_REG_FAIL(err)
    }
    else
    {
        /* Unmute the audio FIFO */
        err = setHwRegisterField(pDis,
                                 E_REG_P11_AIP_CNTRL_0_RW,
                                 E_MASKREG_P11_AIP_CNTRL_0_rst_fifo,
                                 0);
        RETIF_REG_FAIL(err)

        /* Set HDMI HDCP mode on for HDMI */
        /* Also sets E_MASKREG_P11_ENC_CNTRL_ctl_code */
        err = setHwRegisterFieldTable(pDis, &kVoutHdcpOn[0]);
        RETIF_REG_FAIL(err)
    }

    /* For each parameter that is not No Change, set its register */
    if (voutMode != HDMITX_VOUTMODE_NO_CHANGE)
    {
        /* Save the output mode for later use by the matrix & downsampler */
        pDis->voutMode = voutMode;
    }
    if (preFilter < HDMITX_VOUT_PREFIL_NO_CHANGE)
    {
        err = setHwRegisterField(pDis, 
                                 E_REG_P00_HVF_CNTRL_0_W, 
                                 E_MASKREG_P00_HVF_CNTRL_0_prefil,
                                 (UInt8)preFilter);
        RETIF_REG_FAIL(err)
    }
    if (yuvBlank < HDMITX_VOUT_YUV_BLNK_NO_CHANGE)
    {
        err = setHwRegisterField(pDis, 
                                 E_REG_P00_HVF_CNTRL_1_W, 
                                 E_MASKREG_P00_HVF_CNTRL_1_yuvblk,
                                 (UInt8)yuvBlank);
        RETIF_REG_FAIL(err)
    }
    return TM_OK;
}

/*============================================================================*/
/* tmbslTDA9989VideoOutSetSync                                                 */
/*============================================================================*/
tmErrorCode_t
tmbslTDA9989VideoOutSetSync
(
    tmUnitSelect_t          txUnit,
    tmbslHdmiTxVsSrc_t      srcH,
    tmbslHdmiTxVsSrc_t      srcV,
    tmbslHdmiTxVsSrc_t      srcX,
    tmbslHdmiTxVsTgl_t      toggle,
    tmbslHdmiTxVsOnce_t     once
)
{
    tmHdmiTxobject_t *pDis;       /* Pointer to Device Instance Structure */
    tmErrorCode_t     err;        /* Error code */

    /* Check unit parameter and point to TX unit object */
    err = checkUnitSetDis(txUnit, &pDis);
    RETIF(err != TM_OK, err)

    /* Check parameters */
    RETIF_BADPARAM(srcH   >= HDMITX_VSSRC_INVALID)
    RETIF_BADPARAM(srcV   >= HDMITX_VSSRC_INVALID)
    RETIF_BADPARAM(srcX   >= HDMITX_VSSRC_INVALID)
    RETIF_BADPARAM(toggle >= HDMITX_VSTGL_INVALID)
    RETIF_BADPARAM(once   >= HDMITX_VSONCE_INVALID)

    /* For each parameter that is not No Change, set its register */
    if (srcH != HDMITX_VSSRC_NO_CHANGE)
    {
        err = setHwRegisterField(pDis,
                                 E_REG_P00_TBG_CNTRL_1_W,
                                 E_MASKREG_P00_TBG_CNTRL_1_vhx_ext_hs,
                                 (UInt8)srcH);
        RETIF_REG_FAIL(err)
    }
    if (srcV != HDMITX_VSSRC_NO_CHANGE)
    {
        err = setHwRegisterField(pDis,
                                 E_REG_P00_TBG_CNTRL_1_W,
                                 E_MASKREG_P00_TBG_CNTRL_1_vhx_ext_vs,
                                 (UInt8)srcV);
        RETIF_REG_FAIL(err)
    }
    if (srcX != HDMITX_VSSRC_NO_CHANGE)
    {
        err = setHwRegisterField(pDis,
                                 E_REG_P00_TBG_CNTRL_1_W,
                                 E_MASKREG_P00_TBG_CNTRL_1_vhx_ext_de,
                                 (UInt8)srcX);
        RETIF_REG_FAIL(err)
    }
    if (toggle != HDMITX_VSTGL_NO_CHANGE)
    {
        err = setHwRegisterField(pDis,
                                 E_REG_P00_TBG_CNTRL_1_W,
                                 E_MASKREG_P00_TBG_CNTRL_1_vh_tgl,
                                 (UInt8)toggle);
        RETIF_REG_FAIL(err)
    }
    if (once != HDMITX_VSONCE_NO_CHANGE)
    {
        /* Must be last register set */
        err = setHwRegisterField(pDis,
                                 E_REG_P00_TBG_CNTRL_0_W,
                                 E_MASKREG_P00_TBG_CNTRL_0_sync_once,
                                 (UInt8)once);
        RETIF_REG_FAIL(err)
    }

    /* Toggle TMDS serialiser force flags - stability fix */
    err = setHwRegisterField(pDis,
                             E_REG_P02_BUFFER_OUT_RW,
                             E_MASKREG_P02_BUFFER_OUT_srl_force,
                             (UInt8)HDMITX_TMDSOUT_FORCED0);
    RETIF_REG_FAIL(err)
    err = setHwRegisterField(pDis,
                             E_REG_P02_BUFFER_OUT_RW,
                             E_MASKREG_P02_BUFFER_OUT_srl_force,
                             (UInt8)HDMITX_TMDSOUT_NORMAL);
    RETIF_REG_FAIL(err)


    if (once == HDMITX_VSONCE_ONCE)
    {
        /* Toggle output Sync Once flag for settings to take effect */
        err = setHwRegisterField(pDis,
                                 E_REG_P00_TBG_CNTRL_0_W,
                                 E_MASKREG_P00_TBG_CNTRL_0_sync_once,
                                 (UInt8)HDMITX_VSONCE_EACH_FRAME);
        RETIF_REG_FAIL(err)
        err = setHwRegisterField(pDis,
                                 E_REG_P00_TBG_CNTRL_0_W,
                                 E_MASKREG_P00_TBG_CNTRL_0_sync_once,
                                 (UInt8)HDMITX_VSONCE_ONCE);
        RETIF_REG_FAIL(err)
    }
    return TM_OK;
}

/*============================================================================*/
/* tmbslTDA9989VideoSetInOut                                                   */
/*============================================================================*/
tmErrorCode_t
tmbslTDA9989VideoSetInOut
(
    tmUnitSelect_t          txUnit,
    tmbslHdmiTxVidFmt_t     vinFmt,
    tmbslHdmiTxScaMode_t    scaModeRequest,
    tmbslHdmiTxVidFmt_t     voutFmt,
    UInt8                   uPixelRepeat,
    tmbslHdmiTxMatMode_t    matMode,
    tmbslHdmiTxVoutDbits_t  datapathBits,
    tmbslHdmiTxVQR_t        dviVqr
)
{
    tmHdmiTxobject_t        *pDis;       /* Pointer to Device Instance Structure */
    tmErrorCode_t           err;         /* Error code */
    tmbslHdmiTxVidFmt_t     scaInFmt;    /* Scaler input format */
    tmbslHdmiTxVidFmt_t     scaOutFmt;   /* Scaler output format */
    tmbslHdmiTxScaMode_t    scaMode;     /* Scaler mode */
    
    /* Check unit parameter and point to TX unit object */
    err = checkUnitSetDis(txUnit, &pDis);
    RETIF(err != TM_OK, err)

    /* Check parameters */
#ifdef FORMAT_PC
    RETIF_BADPARAM(vinFmt        < HDMITX_VFMT_TV_MIN)
    RETIF_BADPARAM(voutFmt       < HDMITX_VFMT_TV_MIN)
    RETIF_BADPARAM((vinFmt  > HDMITX_VFMT_TV_MAX) && (vinFmt < HDMITX_VFMT_PC_MIN))
    RETIF_BADPARAM((voutFmt > HDMITX_VFMT_TV_MAX) && (voutFmt < HDMITX_VFMT_PC_MIN))
    RETIF_BADPARAM(vinFmt        > HDMITX_VFMT_PC_MAX)
    RETIF_BADPARAM(voutFmt       > HDMITX_VFMT_PC_MAX)
#else /* FORMAT_PC */
    /*FORMAT TV only*/
    RETIF_BADPARAM(vinFmt        < HDMITX_VFMT_TV_MIN)
    RETIF_BADPARAM(voutFmt       < HDMITX_VFMT_TV_MIN)
    RETIF_BADPARAM(vinFmt        > HDMITX_VFMT_TV_MAX)
    RETIF_BADPARAM(voutFmt       > HDMITX_VFMT_TV_MAX)
#endif /* FORMAT_PC */

    RETIF_BADPARAM(scaModeRequest >= HDMITX_SCAMODE_INVALID)
    RETIF_BADPARAM(uPixelRepeat   >= HDMITX_PIXREP_INVALID)
    RETIF_BADPARAM(matMode        >= HDMITX_MATMODE_INVALID)
    RETIF_BADPARAM(datapathBits   >= HDMITX_VOUT_DBITS_INVALID)

#ifdef TMFL_SCALER_SUPPORT
    scaMode = scaModeRequest;
#else  /* TMFL_SCALER_SUPPORT */
    scaMode = HDMITX_SCAMODE_OFF;
    pDis->scaMode = HDMITX_SCAMODE_OFF;
#endif /* TMFL_SCALER_SUPPORT */

    if (voutFmt >= HDMITX_VFMT_TV_NO_REG_MIN)
    {
        /* Disable Scaler mode for PC_FORMAT and for 24/25/30Hz formats */
        scaMode = HDMITX_SCAMODE_OFF;
    }

    /* Get current input format if it must not change */
    if (vinFmt == HDMITX_VFMT_NO_CHANGE)
    {
        RETIF(pDis->vinFmt == HDMITX_VFMT_NULL,
              TMBSL_ERR_HDMI_INCONSISTENT_PARAMS)
        vinFmt = pDis->vinFmt;
    }
    else
    {
        pDis->vinFmt = vinFmt;

#ifdef TMFL_TDA9989_PIXEL_CLOCK_ON_DDC

     if ( pDis->vinFmt <= HDMITX_VFMT_TV_MAX ) {

         err = setHwRegister(pDis, E_REG_P00_TIMER_H_W, 0);
         RETIF(err != TM_OK, err)

         err = setHwRegister(pDis, E_REG_P00_NDIV_IM_W, kndiv_im[vinFmt]);
         RETIF(err != TM_OK, err)

         err = setHwRegister(pDis, E_REG_P12_TX3_RW, kclk_div[vinFmt]);
         RETIF(err != TM_OK, err)

        }
        else if (pDis->vinFmt > HDMITX_VFMT_TV_MAX) {

            err = setHwRegister(pDis, E_REG_P00_TIMER_H_W, E_MASKREG_P00_TIMER_H_im_clksel);
            RETIF(err != TM_OK, err)
            err = setHwRegister(pDis, E_REG_P12_TX3_RW, 17);
            RETIF(err != TM_OK, err)
        }
#endif /* TMFL_TDA9989_PIXEL_CLOCK_ON_DDC */


    }

    /* Get current output format if it must not change */
    if (voutFmt == HDMITX_VFMT_NO_CHANGE)
    {
        RETIF(pDis->voutFmt == HDMITX_VFMT_NULL,
              TMBSL_ERR_HDMI_INCONSISTENT_PARAMS)
        voutFmt = pDis->voutFmt;
    }
    else
    {
        pDis->voutFmt = voutFmt;
    }
    /* Force RGB mode for VGA output format */
/*     if (voutFmt == HDMITX_VFMT_TV_MIN) */
/*     { */
/*         pDis->voutMode = HDMITX_VOUTMODE_RGB444; */
/*     } */
    
    if (pDis->voutMode == HDMITX_VOUTMODE_RGB444)
    {
        if ((pDis->voutFmt >= HDMITX_VFMT_02_720x480p_60Hz) && (pDis->voutFmt <= HDMITX_VFMT_TV_MAX))
        {
            err = setHwRegisterField(pDis, 
                                    E_REG_P00_HVF_CNTRL_1_W, 
                                    E_MASKREG_P00_HVF_CNTRL_1_vqr,
                                    (UInt8) HDMITX_VOUT_QRANGE_RGB_YUV);
            RETIF_REG_FAIL(err)
        }
        else /*Format PC or VGA*/
        {
            err = setHwRegisterField(pDis, 
                                    E_REG_P00_HVF_CNTRL_1_W, 
                                    E_MASKREG_P00_HVF_CNTRL_1_vqr,
                                    (UInt8) HDMITX_VOUT_QRANGE_FS);
            RETIF_REG_FAIL(err)
        }
    }
    else
    {
        err = setHwRegisterField(pDis, 
                                 E_REG_P00_HVF_CNTRL_1_W, 
                                 E_MASKREG_P00_HVF_CNTRL_1_vqr,
                                 (UInt8) HDMITX_VOUT_QRANGE_YUV);
        RETIF_REG_FAIL(err)
    }

    /* If scaler mode is auto then set mode based on input and output format */
    if (scaMode != HDMITX_SCAMODE_NO_CHANGE)
    {
        if (scaMode == HDMITX_SCAMODE_AUTO)
        {
            /* If both formats map to the same device output format then
             * the scaler can be switched off */
            if (kVfmtToRegvfmt_TV[voutFmt] == kVfmtToRegvfmt_TV[vinFmt])
            {
                scaMode = HDMITX_SCAMODE_OFF;
            }
            else
            {
                scaMode = HDMITX_SCAMODE_ON;
            }
        }
        scaInFmt  = vinFmt;
        if (scaMode == HDMITX_SCAMODE_ON)
        {
            scaOutFmt = voutFmt;
        }
        else
        {
            scaOutFmt = vinFmt;
        }
        err = setScalerFormat(pDis, scaInFmt, scaOutFmt, uPixelRepeat);
        RETIF(err != TM_OK, err)
    }
    else
    {
        /* Set pixel repetition - sets pixelRepeatCount, used by setScalerFormat */
        err = setPixelRepeat(pDis, voutFmt, uPixelRepeat);
        RETIF(err != TM_OK, err)
    }

    
    /* Set VS and optional DE */
    err = setDeVs(pDis, voutFmt);
    RETIF(err != TM_OK, err)

    /* If matrix mode is auto then set mode based on input and output format */
    if (matMode != HDMITX_MATMODE_NO_CHANGE)
    {
        if (matMode == HDMITX_MATMODE_AUTO)
        {
            err = tmbslTDA9989MatrixSetConversion(txUnit, vinFmt,
          pDis->vinMode, voutFmt, pDis->voutMode,pDis->dviVqr);
        }
        else
        {
            err = tmbslTDA9989MatrixSetMode(txUnit, HDMITX_MCNTRL_OFF, 
                  HDMITX_MSCALE_NO_CHANGE);
        }
        RETIF(err != TM_OK, err)
    }

    /* Set upsampler and downsampler */
    err = setSampling(pDis);
    RETIF(err != TM_OK, err)

    /* Set colour component bit depth */
    if (datapathBits != HDMITX_VOUT_DBITS_NO_CHANGE)
    {
        err = setHwRegisterField(pDis, 
                                 E_REG_P00_HVF_CNTRL_1_W, 
                                 E_MASKREG_P00_HVF_CNTRL_1_pad,
                                 (UInt8)datapathBits);
        RETIF_REG_FAIL(err)
    }

	/* Save kBypassColourProc registers before pattern goes on */
	getHwRegister(pDis, E_REG_P00_MAT_CONTRL_W,  &gMatContrl[txUnit]);
	getHwRegister(pDis, E_REG_P00_HVF_CNTRL_0_W, &gHvfCntrl0[txUnit]);
	getHwRegister(pDis, E_REG_P00_HVF_CNTRL_1_W, &gHvfCntrl1[txUnit]);

    setState(pDis, EV_SETINOUT);
    return TM_OK;
}

/*============================================================================*/
/* tmbslTDA9989MatrixSetCoeffs                                                 */
/*============================================================================*/

tmErrorCode_t
tmbslTDA9989MatrixSetCoeffs
(
    tmUnitSelect_t         txUnit,
    tmbslHdmiTxMatCoeff_t *pMatCoeff
)
{
    tmHdmiTxobject_t    *pDis;  /* Pointer to Device Instance Structure */
    tmErrorCode_t       err;    /* Error code */
    Int                 i;      /* Loop index */
    UInt8               buf[HDMITX_MAT_COEFF_NUM * 2];     /* Temp buffer */

    /* Check unit parameter and point to TX unit object */
    err = checkUnitSetDis(txUnit, &pDis);
    RETIF(err != TM_OK, err)

    /* Check remaining parameter(s) */
    RETIF_BADPARAM(pMatCoeff == (tmbslHdmiTxMatCoeff_t *)0)
    for (i = 0; i < HDMITX_MAT_COEFF_NUM; i++)
    {
        RETIF_BADPARAM((pMatCoeff->Coeff[i] < HDMITX_MAT_OFFSET_MIN) || 
              (pMatCoeff->Coeff[i] > HDMITX_MAT_OFFSET_MAX))
    }

    /* Convert signed 11 bit values from Coeff array to pairs of MSB-LSB
     * register values, and write to register pairs
     */
    for (i = 0; i < HDMITX_MAT_COEFF_NUM; i++)
    {
        /* Mask & copy MSB */
        buf[i*2] = (UInt8)(((UInt16)pMatCoeff->Coeff[i] & 0x0700) >> 8);
        /* Copy LSB */
        buf[(i*2)+1] = (UInt8)((UInt16)pMatCoeff->Coeff[i] & 0x00FF);
    }
    err = setHwRegisters(pDis,
                         E_REG_P00_MAT_P11_MSB_W,
                         &buf[0],
                         HDMITX_MAT_COEFF_NUM * 2);
    return err;
}


/*============================================================================*/
/* tmbslTDA9989MatrixSetConversion                                             */
/*============================================================================*/
tmErrorCode_t
tmbslTDA9989MatrixSetConversion
(
    tmUnitSelect_t          txUnit,
    tmbslHdmiTxVidFmt_t     vinFmt,
    tmbslHdmiTxVinMode_t    vinMode,
    tmbslHdmiTxVidFmt_t     voutFmt,
    tmbslHdmiTxVoutMode_t   voutMode,
    tmbslHdmiTxVQR_t        dviVqr
)
{
    tmHdmiTxobject_t            *pDis;  /* Ptr to Device Instance Structure */
    tmErrorCode_t               err;        /* Error code */
    tmbslTDA9989Colourspace_t    cspace_in;  /* Input colourspaces */
    tmbslTDA9989Colourspace_t    cspace_out; /* Output colourspaces */
    Int                         matrixIndex;/* Index into matrix preset array */
    UInt8                       buf[MATRIX_PRESET_SIZE]; /* Temp buffer */
    UInt8                       i; /* Loop index */

    /* Check unit parameter and point to TX unit object */
    err = checkUnitSetDis(txUnit, &pDis);
    RETIF(err != TM_OK, err)

    /* Check remaining parameter(s) */
    RETIF_BADPARAM(  ((vinFmt < HDMITX_VFMT_TV_MIN) || (vinFmt > HDMITX_VFMT_PC_MAX))
         || ((vinFmt > HDMITX_VFMT_TV_MAX) && (vinFmt < HDMITX_VFMT_PC_MIN)))
    /* NB: NO_CHANGE is not valid for this function, so limit to actual values*/
    RETIF_BADPARAM(vinMode >= HDMITX_VINMODE_NO_CHANGE)
    RETIF_BADPARAM(  ((voutFmt < HDMITX_VFMT_TV_MIN) || (voutFmt > HDMITX_VFMT_PC_MAX))
         || ((voutFmt > HDMITX_VFMT_TV_MAX) && (voutFmt < HDMITX_VFMT_PC_MIN)))
    /* NB: NO_CHANGE is not valid for this function, so limit to actual values*/
    RETIF_BADPARAM(voutMode >= HDMITX_VOUTMODE_NO_CHANGE)

    /* Since vinMode and voutMode are different types, we don't use a local
       function to do this and use inline code twice */
     

    /* Calculate input colour space */
 /*    if(vinFmt >= HDMITX_VFMT_PC_MIN) */
/*     { */
        /* Catch the PC formats */
        /* cspace_in = HDMITX_CS_RGB_FULL; */
/*     } */
/*     else */
/*     { */
        switch (vinFmt)
        {       /* Catch the HD modes */
        case HDMITX_VFMT_04_1280x720p_60Hz:
        case HDMITX_VFMT_05_1920x1080i_60Hz:
        case HDMITX_VFMT_16_1920x1080p_60Hz:
        case HDMITX_VFMT_19_1280x720p_50Hz:
        case HDMITX_VFMT_20_1920x1080i_50Hz:
        case HDMITX_VFMT_31_1920x1080p_50Hz:
        case HDMITX_VFMT_32_1920x1080p_24Hz:
        case HDMITX_VFMT_33_1920x1080p_25Hz:
        case HDMITX_VFMT_34_1920x1080p_30Hz:
    
            if(vinMode == HDMITX_VINMODE_RGB444)    /* RGB */
            {
                cspace_in = HDMITX_CS_RGB_LIMITED;
            }
            else                                    /* CCIR656, YUV444, YU422 */
            {
                cspace_in = HDMITX_CS_YUV_ITU_BT709;
            }
            break;
        default:    /* Now all the SD modes */
            if(vinMode == HDMITX_VINMODE_RGB444)    /* we're RGB */
            {
                cspace_in = HDMITX_CS_RGB_LIMITED;
            }
            else                                    /* CCIR656, YUV444, YU422 */
            {
                cspace_in = HDMITX_CS_YUV_ITU_BT601;
            }
            break;
        }

/*     } */

    /* Calculate output colour space */
    if(voutFmt >= HDMITX_VFMT_PC_MIN)
    {
        /* Catch the PC formats */
        cspace_out = HDMITX_CS_RGB_FULL;
    }
    else
    {
        switch (voutFmt)
        {       /* Catch the HD modes */
        case HDMITX_VFMT_04_1280x720p_60Hz:
        case HDMITX_VFMT_05_1920x1080i_60Hz:
        case HDMITX_VFMT_16_1920x1080p_60Hz:
        case HDMITX_VFMT_19_1280x720p_50Hz:
        case HDMITX_VFMT_20_1920x1080i_50Hz:
        case HDMITX_VFMT_31_1920x1080p_50Hz:
		case HDMITX_VFMT_32_1920x1080p_24Hz:
		case HDMITX_VFMT_33_1920x1080p_25Hz:
		case HDMITX_VFMT_34_1920x1080p_30Hz:
            
            if(voutMode == HDMITX_VOUTMODE_RGB444)  /* RGB */
            {
                cspace_out = HDMITX_CS_RGB_LIMITED;
            }
            else                                    /* YUV444 or YUV422 */
            {
                cspace_out = HDMITX_CS_YUV_ITU_BT709;
            }
            break;
        default:    /* Now all the SD modes */
            if(voutMode == HDMITX_VOUTMODE_RGB444)  /* RGB */
            {
                cspace_out = HDMITX_CS_RGB_LIMITED;
            }
            else                                    /* YUV444 or YUV422 */
            {
                cspace_out = HDMITX_CS_YUV_ITU_BT601;
            }
            break;
        }
    }

    if (cspace_in == cspace_out)
    {
        /* Switch off colour matrix by setting bypass flag */
        err = setHwRegisterField(pDis, 
                                 E_REG_P00_MAT_CONTRL_W,
                                 E_MASKREG_P00_MAT_CONTRL_mat_bp,
                                 1);
    }
    else
    {
        /* Load appropriate values into matrix  - we have preset blocks of
         * 31 register vales in a table, just need to work out which set to use
         */
        matrixIndex = kMatrixIndex[cspace_in][cspace_out];

        /* Set the first block byte separately, as it is shadowed and can't
         * be set by setHwRegisters */
        err = setHwRegister(pDis,
                             E_REG_P00_MAT_CONTRL_W,
                             kMatrixPreset[matrixIndex][0]);
        RETIF_REG_FAIL(err)
        
        for (i = 0; i < MATRIX_PRESET_SIZE; i++)
        {
            buf[i] = kMatrixPreset[matrixIndex][i];
        }
        
        /* Set the rest of the block */
        err = setHwRegisters(pDis,
                             E_REG_P00_MAT_OI1_MSB_W,
                             &buf[1],
                             MATRIX_PRESET_SIZE - 1);
    }
    return err;
}

/*============================================================================*/
/* tmbslTDA9989MatrixSetInputOffset                                            */
/*============================================================================*/

tmErrorCode_t
tmbslTDA9989MatrixSetInputOffset
(
    tmUnitSelect_t          txUnit,
    tmbslHdmiTxMatOffset_t  *pMatOffset
)
{
    tmHdmiTxobject_t    *pDis;  /* Pointer to Device Instance Structure */
    tmErrorCode_t       err;    /* Error code */
    Int                 i;      /* Loop index */
    UInt8               buf[HDMITX_MAT_OFFSET_NUM * 2];    /* Temp buffer */

    /* Check unit parameter and point to TX unit object */
    err = checkUnitSetDis(txUnit, &pDis);
    RETIF(err != TM_OK, err)

    /* Check remaining parameter(s) */
    RETIF_BADPARAM(pMatOffset == (tmbslHdmiTxMatOffset_t *)0)
    for (i = 0; i < HDMITX_MAT_OFFSET_NUM; i++)
    {
        RETIF_BADPARAM((pMatOffset->Offset[i] < HDMITX_MAT_OFFSET_MIN) || 
              (pMatOffset->Offset[i] > HDMITX_MAT_OFFSET_MAX))
    }

    /* Convert signed 11 bit values from Offset array to pairs of MSB-LSB
     * register values, and write to register pairs
     */
    for (i = 0; i < HDMITX_MAT_OFFSET_NUM; i++)
    {
        /* Mask & copy MSB */
        buf[i*2] = (UInt8)(((UInt16)pMatOffset->Offset[i] & 0x0700) >> 8);
        /* Copy LSB */
        buf[(i*2)+1] = (UInt8)((UInt16)pMatOffset->Offset[i] & 0x00FF);
    }
    err = setHwRegisters(pDis,
                         E_REG_P00_MAT_OI1_MSB_W,
                         &buf[0],
                         HDMITX_MAT_OFFSET_NUM * 2);
    return err;
}


/*============================================================================*/
/* tmbslTDA9989MatrixSetMode                                                   */
/*============================================================================*/
tmErrorCode_t
tmbslTDA9989MatrixSetMode
(
    tmUnitSelect_t       txUnit,
    tmbslHdmiTxmCntrl_t mControl,
    tmbslHdmiTxmScale_t mScale
)
{
    tmHdmiTxobject_t    *pDis;  /* Pointer to Device Instance Structure */
    tmErrorCode_t       err;    /* Error code */

    /* Check unit parameter and point to TX unit object */
    err = checkUnitSetDis(txUnit, &pDis);
    RETIF(err != TM_OK, err)

    /* Check remaining parameter(s) */
    RETIF_BADPARAM((mControl > HDMITX_MCNTRL_MAX) ||
          (mScale > HDMITX_MSCALE_MAX))

    /* For each value that is not NoChange, update the appropriate register */
    if (mControl != HDMITX_MCNTRL_NO_CHANGE)
    {
        err = setHwRegisterField(pDis, 
                                 E_REG_P00_MAT_CONTRL_W,
                                 E_MASKREG_P00_MAT_CONTRL_mat_bp,
                                 (UInt8)mControl);
        RETIF_REG_FAIL(err)
    }

    if (mScale != HDMITX_MSCALE_NO_CHANGE)
    {
        err = setHwRegisterField(pDis, 
                                 E_REG_P00_MAT_CONTRL_W,
                                 E_MASKREG_P00_MAT_CONTRL_mat_sc,
                                 (UInt8)mScale);
        RETIF_REG_FAIL(err)
    }

    return TM_OK;
}

/*============================================================================*/
/* tmbslTDA9989MatrixSetOutputOffset                                           */
/*============================================================================*/

tmErrorCode_t
tmbslTDA9989MatrixSetOutputOffset
(
    tmUnitSelect_t          txUnit,
    tmbslHdmiTxMatOffset_t  *pMatOffset
)
{
    tmHdmiTxobject_t    *pDis;  /* Pointer to Device Instance Structure */
    tmErrorCode_t       err;    /* Error code */
    Int                 i;      /* Loop index */
    UInt8               buf[HDMITX_MAT_OFFSET_NUM * 2];   /* Temp buffer */

    /* Check unit parameter and point to TX unit object */
    err = checkUnitSetDis(txUnit, &pDis);
    RETIF(err != TM_OK, err)

    /* Check remaining parameter(s) */
    RETIF_BADPARAM(pMatOffset == (tmbslHdmiTxMatOffset_t *)0)
    for (i = 0; i < HDMITX_MAT_OFFSET_NUM; i++)
    {
        RETIF_BADPARAM((pMatOffset->Offset[i] < HDMITX_MAT_OFFSET_MIN) || 
              (pMatOffset->Offset[i] > HDMITX_MAT_OFFSET_MAX))
    }

    /* Convert signed 11 bit values from Offset array to pairs of MSB-LSB
     * register values, and write to register pairs
     */
    for (i = 0; i < HDMITX_MAT_OFFSET_NUM; i++)
    {
        /* Mask & copy MSB */
        buf[i*2] = (UInt8)(((UInt16)pMatOffset->Offset[i] & 0x0700) >> 8);
        /* Copy LSB */
        buf[(i*2)+1] = (UInt8)((UInt16)pMatOffset->Offset[i] & 0x00FF);
    }
    err = setHwRegisters(pDis,
                         E_REG_P00_MAT_OO1_MSB_W,
                         &buf[0],
                         HDMITX_MAT_OFFSET_NUM * 2);
    return err;
}


/*============================================================================*/
/* tmbslTDA9989PktSetAclkRecovery                                              */
/*============================================================================*/

tmErrorCode_t
tmbslTDA9989PktSetAclkRecovery
(
    tmUnitSelect_t  txUnit,
    Bool            bEnable
)
{
    tmHdmiTxobject_t    *pDis;  /* Pointer to Device Instance Structure */
    tmErrorCode_t       err;    /* Error code */

    /* Check unit parameter and point to TX unit object */
    err = checkUnitSetDis(txUnit, &pDis);
    RETIF(err != TM_OK, err)

    /* Return TMBSL_ERR_HDMI_OPERATION_NOT_PERMITTED error if the 
     * sinkType is not HDMI
     */ 
    RETIF(pDis->sinkType != HDMITX_SINK_HDMI,
          TMBSL_ERR_HDMI_OPERATION_NOT_PERMITTED)

    /* Check remaining parameter(s) */
    RETIF_BADPARAM((bEnable != True) && (bEnable != False))
    
    /* Write the ACR packet insertion flag */
    err = setHwRegisterField(pDis, 
                             E_REG_P11_DIP_FLAGS_RW,
                             E_MASKREG_P11_DIP_FLAGS_acr,
                             (UInt8)bEnable);
    return err;
}

/*============================================================================*/
/* tmbslTDA9989PktSetAcp                                                       */
/*============================================================================*/
tmErrorCode_t
tmbslTDA9989PktSetAcp
(
    tmUnitSelect_t      txUnit,
    tmbslHdmiTxPkt_t    *pPkt,
    UInt                byteCnt,
    UInt8               uAcpType,
    Bool                bEnable
)
{
    tmHdmiTxobject_t    *pDis;  /* Pointer to Device Instance Structure */
    tmErrorCode_t       err;    /* Error code */
    UInt8               buf[3]; /* Temp buffer to hold header bytes */

    /* Check unit parameter and point to TX unit object */
    err = checkUnitSetDis(txUnit, &pDis);
    RETIF(err != TM_OK, err)

    /* Return TMBSL_ERR_HDMI_OPERATION_NOT_PERMITTED error if the 
     * sinkType is not HDMI
     */ 
    RETIF(pDis->sinkType != HDMITX_SINK_HDMI,
          TMBSL_ERR_HDMI_OPERATION_NOT_PERMITTED)

    /* Only supported for device N4 or later */ 

    /* Check remaining parameter(s) - NULL pointer allowed */
    RETIF_BADPARAM((bEnable != True) && (bEnable != False))

    if(pPkt != Null)
    {
        /* Pointer to structure provided so check parameters */
        RETIF_BADPARAM(byteCnt > HDMITX_PKT_DATA_BYTE_CNT)
        RETIF(byteCnt == 0, TMBSL_ERR_HDMI_INCONSISTENT_PARAMS)

        /* Data to change, start by clearing ACP packet insertion flag */
        err = setHwRegisterField(pDis, 
                                 E_REG_P11_DIP_FLAGS_RW,
                                 E_MASKREG_P11_DIP_FLAGS_acp,
                                 0x00);
        RETIF_REG_FAIL(err)

        /* Prepare ACP header */
        buf[0] = 0x04;      /* ACP packet */
        buf[1] = uAcpType;
        buf[2] = 0;         /* Reserved [HDMI 1.2] */


        /* Write 3 header bytes to registers */
        err = setHwRegisters(pDis,
                             E_REG_P11_ACP_HB0_RW,
                             &buf[0],
                             3);
        RETIF_REG_FAIL(err)

        /* Write "byteCnt" bytes of data to registers */
        err = setHwRegisters(pDis,
                             E_REG_P11_ACP_PB0_RW,
                             &pPkt->dataByte[0],
                             (UInt16)byteCnt);
        RETIF_REG_FAIL(err)
    }

    /* Write the ACP packet insertion flag */
    err = setHwRegisterField(pDis, 
                             E_REG_P11_DIP_FLAGS_RW,
                             E_MASKREG_P11_DIP_FLAGS_acp,
                             (UInt8)bEnable);
    return err;
}


/*============================================================================*/
/* tmbslTDA9989PktSetAudioInfoframe                                            */
/*============================================================================*/
tmErrorCode_t
tmbslTDA9989PktSetAudioInfoframe
(
    tmUnitSelect_t      txUnit,
    tmbslHdmiTxPktAif_t *pPkt,
    Bool                bEnable
)
{
    tmHdmiTxobject_t    *pDis;  /* Pointer to Device Instance Structure */
    tmErrorCode_t       err;    /* Error code */
    UInt8               buf[9]; /* Temp buffer to hold header/packet bytes */
    UInt16              bufReg; /* Base register used for writing InfoFrame*/
    UInt16              flagReg;/* Flag register to be used */
    UInt8               flagMask;/* Mask used for writing flag register */

    /* Check unit parameter and point to TX unit object */
    err = checkUnitSetDis(txUnit, &pDis);
    RETIF(err != TM_OK, err)

    /* Return TMBSL_ERR_HDMI_OPERATION_NOT_PERMITTED error if the 
     * sinkType is not HDMI
     */ 
    RETIF(pDis->sinkType != HDMITX_SINK_HDMI,
          TMBSL_ERR_HDMI_OPERATION_NOT_PERMITTED)

    /* Check remaining parameter(s) - NULL pointer allowed */
    RETIF_BADPARAM((bEnable != True) && (bEnable != False))
    if(pPkt != Null)
    {
        /* Pointer to structure provided so check parameters */
        RETIF_BADPARAM(pPkt->CodingType   > 0x0F)
        RETIF_BADPARAM(pPkt->ChannelCount > 0x07)
        RETIF_BADPARAM(pPkt->SampleFreq   > 0x07)
        RETIF_BADPARAM(pPkt->SampleSize   > 0x03)
        /* No need to check ChannelAlloc - all values are allowed */
        RETIF_BADPARAM((pPkt->DownMixInhibit != True) &&
              (pPkt->DownMixInhibit != False))
        RETIF_BADPARAM(pPkt->LevelShift   > 0x0F)
    }

    /* Only supported for device N4 or later */ 

    /* We're using n4 or later, use IF4 buffer for Audio InfoFrame */
    bufReg = E_REG_P10_IF4_HB0_RW; 
    flagReg = E_REG_P11_DIP_IF_FLAGS_RW;
    flagMask = E_MASKREG_P11_DIP_IF_FLAGS_if4;

    if(pPkt != Null)
    {
        /* Data to change, start by clearing AIF packet insertion flag */
        err = setHwRegisterField(pDis, 
                                 flagReg,
                                 flagMask,
                                 0x00);
        RETIF_REG_FAIL(err)

        /* Prepare AIF header */
        buf[0] = 0x84;     /* Audio InfoFrame */
        buf[1] = 0x01;     /* Version 1 [HDMI 1.2] */
        buf[2] = 0x0A;     /* Length [HDMI 1.2] */

        /* Prepare AIF packet (byte numbers offset by 3) */
        buf[0+3] = 0;     /* Preset checksum to zero so calculation works! */
        buf[1+3] = ((pPkt->CodingType & 0x0F) << 4) |
                     (pPkt->ChannelCount & 0x07);        /* CT3-0, CC2-0 */
        buf[2+3] = ((pPkt->SampleFreq & 0x07) << 2) |
                     (pPkt->SampleSize & 0x03);          /* SF2-0, SS1-0 */
        buf[3+3] = 0;                                      /* [HDMI 1.2] */
        buf[4+3] = pPkt->ChannelAlloc;                  /* CA7-0 */
        buf[5+3] = ((pPkt->LevelShift & 0x0F) << 3);    /* LS3-0 */
        if(pPkt->DownMixInhibit == True)
        {
            buf[5+3] += 0x80;                              /* DMI bit */
        }

        /* Calculate checksum - this is worked out on "Length" bytes of the
         * packet, the checksum (which we've preset to zero), and the three
         * header bytes.  We exclude bytes PB6 to PB10 (which we
         * are not writing) since they are zero.
         */
        buf[0+3] = calculateChecksum(&buf[0], 0x0A+1+3-5);

        /* Write header and packet bytes in one operation */
        err = setHwRegisters(pDis,
                             bufReg,
                             &buf[0],
                             9);
        RETIF_REG_FAIL(err)
    }

    /* Write AIF packet insertion flag */
    err = setHwRegisterField(pDis, 
                             flagReg,
                             flagMask,
                             (UInt8)bEnable);
    return err;
}

/*============================================================================*/
/* tmbslTDA9989PktSetGeneralCntrl                                              */
/*============================================================================*/

tmErrorCode_t
tmbslTDA9989PktSetGeneralCntrl
(
    tmUnitSelect_t      txUnit,
    tmbslHdmiTxaMute_t  *paMute,
    Bool                bEnable
)
{
    tmHdmiTxobject_t    *pDis;  /* Pointer to Device Instance Structure */
    tmErrorCode_t       err;    /* Error code */

    /* Check unit parameter and point to TX unit object */
    err = checkUnitSetDis(txUnit, &pDis);
    RETIF(err != TM_OK, err)

    /* Return TMBSL_ERR_HDMI_OPERATION_NOT_PERMITTED error if the 
     * sinkType is not HDMI
     */ 
    RETIF(pDis->sinkType != HDMITX_SINK_HDMI,
          TMBSL_ERR_HDMI_OPERATION_NOT_PERMITTED)

    /* Check remaining parameter(s) - NULL pointer allowed */
    RETIF_BADPARAM((bEnable != True) && (bEnable != False))

    if(paMute != Null)
    {
        RETIF_BADPARAM((*paMute != HDMITX_AMUTE_OFF) && (*paMute != HDMITX_AMUTE_ON))

        if (*paMute == HDMITX_AMUTE_ON)
        {
            err = setHwRegister(pDis, E_REG_P11_GC_AVMUTE_RW, 0x02);
            RETIF_REG_FAIL(err)
        }
        else
        {
            err = setHwRegister(pDis, E_REG_P11_GC_AVMUTE_RW, 0x01);
            RETIF_REG_FAIL(err)
        }
    }

    /* Set or clear GC packet insertion flag */
    err = setHwRegisterField(pDis, 
                             E_REG_P11_DIP_FLAGS_RW,
                             E_MASKREG_P11_DIP_FLAGS_gc,
                             (UInt8)bEnable);
    return err;
}

/*============================================================================*/
/* tmbslTDA9989PktSetIsrc1                                                     */
/*============================================================================*/
tmErrorCode_t
tmbslTDA9989PktSetIsrc1
(
    tmUnitSelect_t      txUnit,
    tmbslHdmiTxPkt_t    *pPkt,
    UInt                byteCnt,
    Bool                bIsrcCont,
    Bool                bIsrcValid,
    UInt8               uIsrcStatus,
    Bool                bEnable
)
{
    tmHdmiTxobject_t    *pDis;  /* Pointer to Device Instance Structure */
    tmErrorCode_t       err;    /* Error code */
    UInt8               buf[3]; /* Temp buffer to hold header bytes */

    /* Check unit parameter and point to TX unit object */
    err = checkUnitSetDis(txUnit, &pDis);
    RETIF(err != TM_OK, err)

    /* Return TMBSL_ERR_HDMI_OPERATION_NOT_PERMITTED error if the 
     * sinkType is not HDMI
     */ 
    RETIF(pDis->sinkType != HDMITX_SINK_HDMI,
          TMBSL_ERR_HDMI_OPERATION_NOT_PERMITTED)

    /* Only supported for device N4 or later */ 

    /* Check remaining parameter(s) - NULL pointer allowed */
    RETIF_BADPARAM((bEnable != True) && (bEnable != False))

    if(pPkt != Null)
    {
        /* Pointer to structure provided so check parameters */
        RETIF_BADPARAM((bIsrcCont != True) && (bIsrcCont != False))
        RETIF_BADPARAM((bIsrcValid != True) && (bIsrcValid != False))
        RETIF_BADPARAM(uIsrcStatus > 7)    /* 3 bits */
        RETIF_BADPARAM(byteCnt > HDMITX_PKT_DATA_BYTE_CNT)
        RETIF(byteCnt == 0, TMBSL_ERR_HDMI_INCONSISTENT_PARAMS)

        /* Data to change, start by clearing ISRC1 packet insertion flag */
        err = setHwRegisterField(pDis, 
                                 E_REG_P11_DIP_FLAGS_RW,
                                 E_MASKREG_P11_DIP_FLAGS_isrc1,
                                 0x00);
        RETIF_REG_FAIL(err)

        /* Prepare ISRC1 header */
        buf[0] = 0x05;      /* ISRC1 packet */
        buf[1] = (uIsrcStatus & 0x07);
        if(bIsrcValid == True)
        {
            buf[1] += 0x40;
        }
        if(bIsrcCont == True)
        {
            buf[1] += 0x80;
        }
        buf[2] = 0;         /* Reserved [HDMI 1.2] */

        /* Write 3 header bytes to registers */
        err = setHwRegisters(pDis,
                             E_REG_P11_ISRC1_HB0_RW,
                             &buf[0],
                             3);
        RETIF_REG_FAIL(err)

        /* Write "byteCnt" bytes of data to registers */
        err = setHwRegisters(pDis,
                             E_REG_P11_ISRC1_PB0_RW,
                             &pPkt->dataByte[0],
                             (UInt16)byteCnt);
        RETIF_REG_FAIL(err)
    }

    /* Write the ISRC1 packet insertion flag */
    err = setHwRegisterField(pDis, 
                             E_REG_P11_DIP_FLAGS_RW,
                             E_MASKREG_P11_DIP_FLAGS_isrc1,
                             (UInt8)bEnable);
    return err;
}

/*============================================================================*/
/* tmbslTDA9989PktSetIsrc2                                                     */
/*============================================================================*/
tmErrorCode_t
tmbslTDA9989PktSetIsrc2
(
    tmUnitSelect_t      txUnit,
    tmbslHdmiTxPkt_t    *pPkt,
    UInt                byteCnt,
    Bool                bEnable
)
{
    tmHdmiTxobject_t    *pDis;  /* Pointer to Device Instance Structure */
    tmErrorCode_t       err;    /* Error code */
    UInt8               buf[3]; /* Temp buffer to hold header bytes */

    /* Check unit parameter and point to TX unit object */
    err = checkUnitSetDis(txUnit, &pDis);
    RETIF(err != TM_OK, err)

    /* Return TMBSL_ERR_HDMI_OPERATION_NOT_PERMITTED error if the 
     * sinkType is not HDMI
     */ 
    RETIF(pDis->sinkType != HDMITX_SINK_HDMI,
          TMBSL_ERR_HDMI_OPERATION_NOT_PERMITTED)

    /* Only supported for device N4 or later */ 

    /* Check remaining parameter(s) - NULL pointer allowed */
    RETIF_BADPARAM((bEnable != True) && (bEnable != False))

    if(pPkt != Null)
    {
        /* Pointer to structure provided so check parameters */
        RETIF_BADPARAM(byteCnt > HDMITX_PKT_DATA_BYTE_CNT)
        RETIF(byteCnt == 0, TMBSL_ERR_HDMI_INCONSISTENT_PARAMS)

        /* Data to change, start by clearing ISRC2 packet insertion flag */
        err = setHwRegisterField(pDis, 
                                 E_REG_P11_DIP_FLAGS_RW,
                                 E_MASKREG_P11_DIP_FLAGS_isrc2,
                                 0x00);
        RETIF_REG_FAIL(err)

        /* Prepare ISRC2 header */
        buf[0] = 0x06;      /* ISRC2 packet */
        buf[1] = 0;         /* Reserved [HDMI 1.2] */
        buf[2] = 0;         /* Reserved [HDMI 1.2] */

        /* Write 3 header bytes to registers */
        err = setHwRegisters(pDis,
                             E_REG_P11_ISRC2_HB0_RW,
                             &buf[0],
                             3);
        RETIF_REG_FAIL(err)

        /* Write "byteCnt" bytes of data to registers */
        err = setHwRegisters(pDis,
                             E_REG_P11_ISRC2_PB0_RW,
                             &pPkt->dataByte[0],
                             (UInt16)byteCnt);
        RETIF_REG_FAIL(err)
    }

    /* Write the ISRC2 packet insertion flag */
    err = setHwRegisterField(pDis, 
                             E_REG_P11_DIP_FLAGS_RW,
                             E_MASKREG_P11_DIP_FLAGS_isrc2,
                             (UInt8)bEnable);
    return err;
}

/*============================================================================*/
/* tmbslTDA9989PktSetMpegInfoframe                                             */
/*============================================================================*/
tmErrorCode_t
tmbslTDA9989PktSetMpegInfoframe
(
    tmUnitSelect_t          txUnit,
    tmbslHdmiTxPktMpeg_t    *pPkt,
    Bool                    bEnable
)
{
    tmHdmiTxobject_t    *pDis;  /* Pointer to Device Instance Structure */
    tmErrorCode_t       err;    /* Error code */
    UInt8               buf[9]; /* Temp buffer to hold packet */

    /* Check unit parameter and point to TX unit object */
    err = checkUnitSetDis(txUnit, &pDis);
    RETIF(err != TM_OK, err)

    /* Return TMBSL_ERR_HDMI_OPERATION_NOT_PERMITTED error if the 
     * sinkType is not HDMI
     */ 
    RETIF(pDis->sinkType != HDMITX_SINK_HDMI,
          TMBSL_ERR_HDMI_OPERATION_NOT_PERMITTED)

    /* Only supported for device N4 or later */ 

    /* Check remaining parameter(s) */
    RETIF_BADPARAM((bEnable != True) && (bEnable != False))

    if(pPkt != Null)
    {
        /* Pointer to structure provided so check parameters */
        RETIF_BADPARAM((pPkt->bFieldRepeat != True) && (pPkt->bFieldRepeat != False))
        RETIF_BADPARAM(pPkt->frameType >= HDMITX_MPEG_FRAME_INVALID)

        /* Data to change, start by clearing MPEG packet insertion flag */
        err = setHwRegisterField(pDis, 
                                 E_REG_P11_DIP_IF_FLAGS_RW,
                                 E_MASKREG_P11_DIP_IF_FLAGS_if5,
                                 0x00);
        RETIF_REG_FAIL(err)

        /* Prepare MPEG header */
        buf[0] = 0x85;      /* MPEG Source InfoFrame */
        buf[1] = 0x01;      /* Version 1 [HDMI 1.2] */
        buf[2] = 0x0A;      /* Length [HDMI 1.2] */

        /* Prepare MPEG packet (byte numbers offset by 3) */
        buf[0+3] = 0;     /* Preset checksum to zero so calculation works! */
        buf[1+3] = (UInt8)(pPkt->bitRate & 0x000000FF);
        buf[2+3] = (UInt8)((pPkt->bitRate & 0x0000FF00) >> 8);
        buf[3+3] = (UInt8)((pPkt->bitRate & 0x00FF0000) >> 16);
        buf[4+3] = (UInt8)((pPkt->bitRate & 0xFF000000) >> 24);
        buf[5+3] = pPkt->frameType;                         /* MF1-0 */
        if(pPkt->bFieldRepeat == True)
        {
            buf[5+3] += 0x10;                               /* FR0 bit */
        }

        /* Calculate checksum - this is worked out on "Length" bytes of the
         * packet, the checksum (which we've preset to zero), and the three
         * header bytes.  We exclude bytes PB6 to PB10 (which we
         * are not writing) since they are zero.
         */
        buf[0+3] = calculateChecksum(&buf[0], 0x0A+1+3-5);

        /* Write header and packet bytes in one operation */
        err = setHwRegisters(pDis,
                             E_REG_P10_IF5_HB0_RW,
                             &buf[0],
                             9);
        RETIF_REG_FAIL(err)
    }

    /* Write the MPEG packet insertion flag */
    err = setHwRegisterField(pDis, 
                             E_REG_P11_DIP_IF_FLAGS_RW,
                             E_MASKREG_P11_DIP_IF_FLAGS_if5,
                             (UInt8)bEnable);
    return err;
}
/*============================================================================*/
/* tmbslTDA9989PktSetNullInsert                                                */
/*============================================================================*/
tmErrorCode_t
tmbslTDA9989PktSetNullInsert
(
    tmUnitSelect_t  txUnit,
    Bool            bEnable
)
{
    tmHdmiTxobject_t    *pDis;  /* Pointer to Device Instance Structure */
    tmErrorCode_t       err;    /* Error code */

    /* Check unit parameter and point to TX unit object */
    err = checkUnitSetDis(txUnit, &pDis);
    RETIF(err != TM_OK, err)

    /* Return TMBSL_ERR_HDMI_OPERATION_NOT_PERMITTED error if the 
     * sinkType is not HDMI
     */ 
    RETIF(pDis->sinkType != HDMITX_SINK_HDMI,
          TMBSL_ERR_HDMI_OPERATION_NOT_PERMITTED)

    /* Check remaining parameter(s) */
    RETIF_BADPARAM((bEnable != True) && (bEnable != False))

    /* Set or clear FORCE_NULL packet insertion flag */
    err = setHwRegisterField(pDis, 
                             E_REG_P11_DIP_FLAGS_RW,
                             E_MASKREG_P11_DIP_FLAGS_force_null,
                             (UInt8)bEnable);
    return err;
}

/*============================================================================*/
/* tmbslTDA9989PktSetNullSingle                                                */
/*============================================================================*/
tmErrorCode_t
tmbslTDA9989PktSetNullSingle
(
    tmUnitSelect_t  txUnit
)
{
    tmHdmiTxobject_t    *pDis;  /* Pointer to Device Instance Structure */
    tmErrorCode_t       err;    /* Error code */

    /* Check unit parameter and point to TX unit object */
    err = checkUnitSetDis(txUnit, &pDis);
    RETIF(err != TM_OK, err)

    /* Return TMBSL_ERR_HDMI_OPERATION_NOT_PERMITTED error if the 
     * sinkType is not HDMI
     */ 
    RETIF(pDis->sinkType != HDMITX_SINK_HDMI,
          TMBSL_ERR_HDMI_OPERATION_NOT_PERMITTED)

    /* Set NULL packet insertion flag */
    err = setHwRegisterField(pDis, 
                             E_REG_P11_DIP_FLAGS_RW,
                             E_MASKREG_P11_DIP_FLAGS_null,
                             0x01);
    return err;
}

/*============================================================================*/
/* tmbslTDA9989PktSetSpdInfoframe                                              */
/*============================================================================*/
tmErrorCode_t
tmbslTDA9989PktSetSpdInfoframe
(
    tmUnitSelect_t      txUnit,
    tmbslHdmiTxPktSpd_t *pPkt,
    Bool                bEnable
)
{
    tmHdmiTxobject_t    *pDis;  /* Pointer to Device Instance Structure */
    tmErrorCode_t       err;    /* Error code */
    UInt8               buf[29];/* Temp buffer to hold packet */

    /* Check unit parameter and point to TX unit object */
    err = checkUnitSetDis(txUnit, &pDis);
    RETIF(err != TM_OK, err)

    /* Return TMBSL_ERR_HDMI_OPERATION_NOT_PERMITTED error if the 
     * sinkType is not HDMI
     */ 
    RETIF(pDis->sinkType != HDMITX_SINK_HDMI,
          TMBSL_ERR_HDMI_OPERATION_NOT_PERMITTED)

    /* Only supported for device N4 or later */ 

    /* Check remaining parameter(s) */
    RETIF_BADPARAM((bEnable != True) && (bEnable != False))

    if(pPkt != Null)
    {
        /* Pointer to structure provided so check parameters */
        RETIF_BADPARAM(pPkt->SourceDevInfo >= HDMITX_SPD_INFO_INVALID)

        /* Data to change, start by clearing SPD packet insertion flag */
        err = setHwRegisterField(pDis, 
                                 E_REG_P11_DIP_IF_FLAGS_RW,
                                 E_MASKREG_P11_DIP_IF_FLAGS_if3,
                                 0x00);
        RETIF_REG_FAIL(err)

        /* Prepare SPD header */
        buf[0] = 0x83;      /* Source. Product Descriptor InfoFrame */
        buf[1] = 0x01;      /* Version 1 [CEA 861B] */
        buf[2] = 0x19;      /* Length [HDMI 1.2] */

        /* Prepare SPD packet (byte numbers offset by 3) */
        buf[0+3] = 0;     /* Preset checksum to zero so calculation works! */
        lmemcpy(&buf[1+3], &pPkt->VendorName[0], HDMI_TX_SPD_VENDOR_SIZE);
        lmemcpy(&buf[1+3+HDMI_TX_SPD_VENDOR_SIZE], &pPkt->ProdDescr[0],
               HDMI_TX_SPD_DESCR_SIZE);


        buf[HDMI_TX_SPD_LENGTH+3] = pPkt->SourceDevInfo;

        /* Calculate checksum - this is worked out on "Length" bytes of the
         * packet, the checksum (which we've preset to zero), and the three
         * header bytes.  
         */
        buf[0+3] = calculateChecksum(&buf[0], HDMI_TX_SPD_LENGTH+1+3);

        /* Write header and packet bytes in one operation */
        err = setHwRegisters(pDis,
                             E_REG_P10_IF3_HB0_RW,
                             &buf[0],
                             29);
        RETIF_REG_FAIL(err)
    }

    /* Write the SPD packet insertion flag */
    err = setHwRegisterField(pDis, 
                             E_REG_P11_DIP_IF_FLAGS_RW,
                             E_MASKREG_P11_DIP_IF_FLAGS_if3,
                             (UInt8)bEnable);
    return err;
}


/*============================================================================*/
/* tmbslTDA9989PktSetVideoInfoframe                                            */
/*============================================================================*/
tmErrorCode_t
tmbslTDA9989PktSetVideoInfoframe
(
    tmUnitSelect_t      txUnit,
    tmbslHdmiTxPktVif_t *pPkt,
    Bool                bEnable
)
{
    tmHdmiTxobject_t    *pDis;  /* Pointer to Device Instance Structure */
    tmErrorCode_t       err;    /* Error code */
    UInt8               buf[17];/* Temp buffer to hold header/packet bytes */
    UInt16              bufReg; /* Base register used for writing InfoFrame*/
    UInt16              flagReg;/* Flag register to be used */
    UInt8               flagMask;/* Mask used for writing flag register */

    /* Check unit parameter and point to TX unit object */
    err = checkUnitSetDis(txUnit, &pDis);
    RETIF(err != TM_OK, err)

    /* Return TMBSL_ERR_HDMI_OPERATION_NOT_PERMITTED error if the 
     * sinkType is not HDMI
     */ 
    RETIF(pDis->sinkType != HDMITX_SINK_HDMI,
          TMBSL_ERR_HDMI_OPERATION_NOT_PERMITTED)

    /* Check remaining parameter(s) - NULL pointer allowed */
    RETIF_BADPARAM((bEnable != True) && (bEnable != False))
    if(pPkt != Null)
    {
        /* Pointer to structure provided so check parameters */
        RETIF_BADPARAM(pPkt->Colour             > 0x03)
        RETIF_BADPARAM((pPkt->ActiveInfo != True) && (pPkt->ActiveInfo != False))
        RETIF_BADPARAM(pPkt->BarInfo            > 0x03)
        RETIF_BADPARAM(pPkt->ScanInfo           > 0x03)
        RETIF_BADPARAM(pPkt->Colorimetry        > 0x03)
        RETIF_BADPARAM(pPkt->PictureAspectRatio > 0x03)
        RETIF_BADPARAM(pPkt->ActiveFormatRatio  > 0x0F)
        RETIF_BADPARAM(pPkt->Scaling            > 0x03)
        RETIF_BADPARAM(pPkt->VidFormat          > 0x7F)
        RETIF_BADPARAM(pPkt->PixelRepeat        > 0x0F)
    }

    /* Only supported for device N4 or later */ 

    /* We're using n4 or later, use IF2 buffer for Video InfoFrame */
    bufReg = E_REG_P10_IF2_HB0_RW; 
    flagReg = E_REG_P11_DIP_IF_FLAGS_RW;
    flagMask = E_MASKREG_P11_DIP_IF_FLAGS_if2;

    if(pPkt != Null)
    {
        /* Data to change, start by clearing VIF packet insertion flag */
        err = setHwRegisterField(pDis, 
                                 flagReg,
                                 flagMask,
                                 0x00);
        RETIF_REG_FAIL(err)

        /* Prepare VIF header */
        buf[0] = 0x82;     /* Video InfoFrame */
        buf[1] = 0x02;     /* Version 2 [HDMI 1.2] */
        buf[2] = 0x0D;     /* Length [HDMI 1.2] */

        /* Prepare VIF packet (byte numbers offset by 3) */
        buf[0+3] = 0;     /* Preset checksum to zero so calculation works! */
        buf[1+3] = ((pPkt->Colour & 0x03) << 5) |       /* Y1-0, B1-0,S1-0 */
                    ((pPkt->BarInfo & 0x03) << 2) |
                     (pPkt->ScanInfo & 0x03);
        if(pPkt->ActiveInfo == True)
        {
            buf[1+3] += 0x10;                              /* AI bit */
        }
        buf[2+3] = ((pPkt->Colorimetry & 0x03) << 6) |  /* C1-0, M1-0, R3-0 */
                    ((pPkt->PictureAspectRatio & 0x03) << 4) |
                     (pPkt->ActiveFormatRatio & 0x0F);
        buf[3+3] = (pPkt->Scaling & 0x03);              /* SC1-0 */                                    /* [HDMI 1.2] */
        buf[4+3] = (pPkt->VidFormat & 0x7F);            /* VIC6-0 */
        buf[5+3] = (pPkt->PixelRepeat & 0x0F);          /* PR3-0 */
        buf[6+3] = (UInt8)(pPkt->EndTopBarLine & 0x00FF);
        buf[7+3] = (UInt8)((pPkt->EndTopBarLine & 0xFF00) >> 8);
        buf[8+3] = (UInt8)(pPkt->StartBottomBarLine & 0x00FF);
        buf[9+3] = (UInt8)((pPkt->StartBottomBarLine & 0xFF00) >> 8);
        buf[10+3] = (UInt8)(pPkt->EndLeftBarPixel & 0x00FF);
        buf[11+3] = (UInt8)((pPkt->EndLeftBarPixel & 0xFF00) >> 8);
        buf[12+3] = (UInt8)(pPkt->StartRightBarPixel & 0x00FF);
        buf[13+3] = (UInt8)((pPkt->StartRightBarPixel & 0xFF00) >> 8);

        /* Calculate checksum - this is worked out on "Length" bytes of the
         * packet, the checksum (which we've preset to zero), and the three
         * header bytes.
         */
        buf[0+3] = calculateChecksum(&buf[0], 0x0D+1+3);

        /* Write header and packet bytes in one operation */
        err = setHwRegisters(pDis,
                             bufReg,
                             &buf[0],
                             17);
        RETIF_REG_FAIL(err)
    }

    /* Write VIF packet insertion flag */
    err = setHwRegisterField(pDis, 
                             flagReg,
                             flagMask,
                             (UInt8)bEnable);
    return err;
}

/*============================================================================*/
/* tmbslTDA9989PktSetRawVideoInfoframe                                       */
/*============================================================================*/
tmErrorCode_t tmbslTDA9989PktSetRawVideoInfoframe
(
    tmUnitSelect_t          txUnit,
    tmbslHdmiTxPktRawAvi_t *pPkt,
    Bool                    bEnable
)
{


    tmHdmiTxobject_t    *pDis;  /* Pointer to Device Instance Structure */
    tmErrorCode_t       err;    /* Error code */

    /* Check unit parameter and point to TX unit object */
    err = checkUnitSetDis(txUnit, &pDis);
    RETIF(err != TM_OK, err)

    /* Return TMBSL_ERR_HDMI_OPERATION_NOT_PERMITTED error if the 
     * sinkType is not HDMI
     */ 
    RETIF(pDis->sinkType != HDMITX_SINK_HDMI,
          TMBSL_ERR_HDMI_OPERATION_NOT_PERMITTED)

    /* Check remaining parameter(s) - NULL pointer allowed */
    RETIF_BADPARAM((bEnable != True) && (bEnable != False))
    
    /* use IF2 buffer */
    if(pPkt != Null)
    {
        /* Data to change, start by clearing VIF packet insertion flag */
        err = setHwRegisterField(pDis, 
                                 E_REG_P11_DIP_IF_FLAGS_RW,
                                 E_MASKREG_P11_DIP_IF_FLAGS_if2,
                                 0x00);
        RETIF_REG_FAIL(err)


        /* Write VIF raw header bytes 0-2 */
        err = setHwRegisters(pDis,
                             E_REG_P10_IF2_HB0_RW,
                             pPkt->HB,
                             3);
        RETIF_REG_FAIL(err)

        /* Write VIF raw payload bytes 0-27 */
        err = setHwRegisters(pDis,
                             E_REG_P10_IF2_PB0_RW,
                             pPkt->PB,
                             28);

        RETIF_REG_FAIL(err)

    }

    /* Write VIF packet insertion flag */
    err = setHwRegisterField(pDis, 
                             E_REG_P11_DIP_IF_FLAGS_RW,
                             E_MASKREG_P11_DIP_IF_FLAGS_if2,
                             (UInt8)bEnable);
    
    return err;
}

/*============================================================================*/
/* tmbslTDA9989PktSetVsInfoframe                                               */
/*============================================================================*/

tmErrorCode_t
tmbslTDA9989PktSetVsInfoframe
(
    tmUnitSelect_t      txUnit,
    tmbslHdmiTxPkt_t    *pPkt,
    UInt                byteCnt,
    UInt8               uVersion,
    Bool                bEnable
)
{
    tmHdmiTxobject_t    *pDis;  /* Pointer to Device Instance Structure */
    tmErrorCode_t       err;    /* Error code */
    UInt8               buf[31];/* Temp buffer to hold packet */

    /* Check unit parameter and point to TX unit object */
    err = checkUnitSetDis(txUnit, &pDis);
    RETIF(err != TM_OK, err)

    /* Return TMBSL_ERR_HDMI_OPERATION_NOT_PERMITTED error if the 
     * sinkType is not HDMI
     */ 
    RETIF(pDis->sinkType != HDMITX_SINK_HDMI,
          TMBSL_ERR_HDMI_OPERATION_NOT_PERMITTED)

    /* Only supported for device N4 or later */ 

    /* Check remaining parameter(s) - NULL pointer allowed */
    RETIF_BADPARAM((bEnable != True) && (bEnable != False))

    if(pPkt != Null)
    {
        /* Pointer to structure provided so check parameters */
        /* InfoFrame needs a checksum, so 1 usable byte less than full pkt */
        RETIF_BADPARAM(byteCnt > (HDMITX_PKT_DATA_BYTE_CNT-1))
        RETIF(byteCnt == 0, TMBSL_ERR_HDMI_INCONSISTENT_PARAMS)

        /* Data to change, start by clearing VS_IF packet insertion flag */
        err = setHwRegisterField(pDis, 
                                 E_REG_P11_DIP_IF_FLAGS_RW,
                                 E_MASKREG_P11_DIP_IF_FLAGS_if1,
                                 0x00);
        RETIF_REG_FAIL(err)

        /* Prepare VS_IF header */
        lmemset(&buf[0], 0, 31); /* Clear buffer as user may vary length used */
        buf[0] = 0x81;          /* Vendor Specific InfoFrame */
        buf[1] = uVersion;      /* Vendor defined version */
        buf[2] = (UInt8)byteCnt;       /* Length [HDMI 1.2] */

        /* Prepare VS_IF packet (byte numbers offset by 3) */
        buf[0+3] = 0;     /* Preset checksum to zero so calculation works! */
        lmemcpy(&buf[1+3], &pPkt->dataByte[0], byteCnt);

        /* Calculate checksum - this is worked out on "Length" bytes of the
         * packet, the checksum (which we've preset to zero), and the three
         * header bytes.  
         */
        buf[0+3] = calculateChecksum(&buf[0], byteCnt+1+3);

        /* Write header and packet bytes in one operation  - write entire 
         * buffer even though we may not be using it all so that zeros
         * are placed in the unused registers. */
        err = setHwRegisters(pDis,
                             E_REG_P10_IF1_HB0_RW,
                             &buf[0],
                             31);
        RETIF_REG_FAIL(err)
    }

    /* Write the VS_IF packet insertion flag */
    err = setHwRegisterField(pDis, 
                             E_REG_P11_DIP_IF_FLAGS_RW,
                             E_MASKREG_P11_DIP_IF_FLAGS_if1,
                             (UInt8)bEnable);
    return err;
}




/*============================================================================*/
/*                              STATIC FUNCTIONS                              */
/*============================================================================*/

/*============================================================================*/
/* setDeVs                                                                    */
/*============================================================================*/
static tmErrorCode_t
setDeVs
(
    tmHdmiTxobject_t    *pDis,
    tmbslHdmiTxVidFmt_t  voutFmt
)
{
    tmErrorCode_t   err;        /* Error code */
    UInt16          vsPixStrt2; /* VS pixel number for start pulse in field 2 */
    UInt8           regVfmt;    /* Video format value used for register */
    tmbslHdmiTxVidFmt_t  voutFmtIndex; /* Index in tab kVfmtToRegvfmt_TV*/
    
    /* Parameters already checked by caller */
    
    /* IF voutFmt = No Change THEN return TM_OK */
    RETIF(voutFmt == HDMITX_VFMT_NO_CHANGE, TM_OK)

    voutFmtIndex = voutFmt;
#ifdef FORMAT_PC
    if (voutFmtIndex > HDMITX_VFMT_PC_MIN)
    {
        voutFmtIndex = (tmbslHdmiTxVidFmt_t)(voutFmtIndex - HDMITX_VFMT_PC_MIN + HDMITX_VFMT_TV_MAX + (tmbslHdmiTxVidFmt_t)1);
    }
#endif /* FORMAT_PC */
    
    
    /* Quit if the output format does not map to the register format */
    regVfmt = kVfmtToRegvfmt_TV[voutFmtIndex];
    RETIF_BADPARAM(regVfmt == E_REGVFMT_INVALID)

    /* DE_START & DE_STOP no longer set because N2 device no longer supported */

    /* Adjust VS_PIX_STRT_2 and VS_PIX_END_2 for interlaced output formats */
    vsPixStrt2 = kRegvfmtToVs2[regVfmt];
    err = setHwRegisterMsbLsb(pDis, E_REG_P00_VS_PIX_STRT_2_MSB_W, vsPixStrt2);
    RETIF_REG_FAIL(err)
    err = setHwRegisterMsbLsb(pDis, E_REG_P00_VS_PIX_END_2_MSB_W, vsPixStrt2);

    return err;
}

/*============================================================================*/
/* setPixelRepeat                                                             */
/*============================================================================*/
static tmErrorCode_t
setPixelRepeat
(
    tmHdmiTxobject_t    *pDis,
    tmbslHdmiTxVidFmt_t voutFmt,
    UInt8               uPixelRepeat
)
{
    tmErrorCode_t err = TM_OK;  /* Error code */

    RETIF(voutFmt == HDMITX_VFMT_NO_CHANGE, TM_OK)

    err = InputConfig(pDis,
                      HDMITX_VINMODE_NO_CHANGE,
                      HDMITX_PIXEDGE_NO_CHANGE,
                      HDMITX_PIXRATE_NO_CHANGE,
                      HDMITX_UPSAMPLE_NO_CHANGE, 
                      uPixelRepeat,
                      voutFmt
                      );

   return err;
}
/*============================================================================*/
/* setSampling                                                                */
/*============================================================================*/
static tmErrorCode_t
setSampling
(
    tmHdmiTxobject_t   *pDis
)
{
    tmErrorCode_t err;          /* Error code */
    UInt8         upSample;     /* 1 if upsampler must be enabled */
    UInt8         downSample;   /* 1 if downsampler must be enabled */
    UInt8         matrixBypass; /*>0 if matrix has been bypassed */

    if ((pDis->vinMode == HDMITX_VINMODE_YUV422)
    ||  (pDis->vinMode == HDMITX_VINMODE_CCIR656))
    {
        if (pDis->voutMode == HDMITX_VOUTMODE_YUV422)
        {
            /* Input 422/656, output 422 */
            err = getHwRegister(pDis, E_REG_P00_MAT_CONTRL_W, &matrixBypass);
            RETIF_REG_FAIL(err)
            matrixBypass &= E_MASKREG_P00_MAT_CONTRL_mat_bp;
            /* Has matrix been bypassed? */
            if (matrixBypass > 0)
            {
                upSample = 0;
                downSample = 0;
            }
            else
            {
                upSample = 1;
                downSample = 1;
            }
        }
        else
        {
            /* Input 422/656, output not 422 */
            upSample = 1;
            downSample = 0;
        }
    }
    else
    {
        if (pDis->voutMode == HDMITX_VOUTMODE_YUV422)
        {
            /* Input not 422/656, output 422 */
            upSample = 0;
            downSample = 1;
        }
        else
        {
            /* Input not 422/656, output not 422 */
            upSample = 0;
            downSample = 0;
        }
    }

    /* Check upsample mode saved by tmbslTDA9989VideoInSetConfig */
    if (pDis->upsampleMode != HDMITX_UPSAMPLE_AUTO)
    {
        /* Saved upsample mode overrides local one */
        upSample = pDis->upsampleMode;
    }

    /* Set upsampler */
    err = setHwRegisterField(pDis, 
                             E_REG_P00_HVF_CNTRL_0_W, 
                             E_MASKREG_P00_HVF_CNTRL_0_intpol,
                             upSample);
    RETIF_REG_FAIL(err)

    /* Set downsampler */
    err = setHwRegisterField(pDis, 
                             E_REG_P00_HVF_CNTRL_1_W, 
                             E_MASKREG_P00_HVF_CNTRL_1_for,
                             downSample);
    return err;
}

/*============================================================================*/
/* setScalerFormat                                                            */
/*============================================================================*/
static tmErrorCode_t
setScalerFormat
(
    tmHdmiTxobject_t   *pDis,
    tmbslHdmiTxVidFmt_t vinFmt,
    tmbslHdmiTxVidFmt_t voutFmt,
    UInt8 pixelRepeat
)
{
    tmErrorCode_t err;          /* Error code */
    UInt8         regVoutFmt;   /* Video o/p format value used for register */
    UInt8         regVinFmt;    /* Video i/p format value used for comparison */
    UInt8         regVal;       /* Register value */
    UInt8         scInFmt = 0;  /* Scaler input format */
    UInt8         scOutFmt = 0; /* Scaler output format */
    UInt8         vfInFmt = 0;  /* Video input format */
    UInt16        pllScgN = 0;      /* PLL scaling values */
    UInt16        pllScgR = 0;

#ifdef FORMAT_PC
    RETIF_BADPARAM(vinFmt        > HDMITX_VFMT_PC_MAX)
    RETIF_BADPARAM(voutFmt        > HDMITX_VFMT_PC_MAX)
#else /*FORMAT_PC*/
    RETIF_BADPARAM(vinFmt        > HDMITX_VFMT_TV_MAX)
    RETIF_BADPARAM(voutFmt        > HDMITX_VFMT_TV_MAX)
#endif /*FORMAT_PC*/
    
    /* Look up the VIDFORMAT register formats from the register format table */
    regVinFmt = kVfmtToRegvfmt_TV[vinFmt];
    regVoutFmt = kVfmtToRegvfmt_TV[voutFmt];
    /* Quit if the output format does not map to the register format */
    RETIF_BADPARAM(regVoutFmt == E_REGVFMT_INVALID)

#ifdef TMFL_SCALER_SUPPORT
    if ((vinFmt > HDMITX_VFMT_TV_MAX)
    ||  (voutFmt > HDMITX_VFMT_TV_MAX)
    ||  (regVinFmt == regVoutFmt))
    {
        /* Disable scaler for PC formats or if same input and output formats */
        err = setHwRegisterFieldTable(pDis, &kScalerOff[0]);
        RETIF_REG_FAIL(err)

        pDis->scaMode = HDMITX_SCAMODE_OFF;
    }
    else
    {
        /* Need to scale: quit if no scaler */
        RETIF(pDis->uDeviceFeatures & E_MASKREG_P00_VERSION_not_s,
              TMBSL_ERR_HDMI_NOT_SUPPORTED)

        /* Look up scaler register formats from table */
        scInFmt = SCIO2SCIN(kVfmtToRegvfmtScio_TV[vinFmt]);
        scOutFmt = SCIO2SCOUT(kVfmtToRegvfmtScio_TV[voutFmt]);
        vfInFmt = kVfmtToRegvfmtVfi_TV[vinFmt];

        /* Do these formats individually support scaling? */
        RETIF(scInFmt  == E_REGVFMT_SCIN_INVALID,
            TMBSL_ERR_HDMI_INCONSISTENT_PARAMS)
        RETIF(scOutFmt == E_REGVFMT_SCOUT_INVALID,
            TMBSL_ERR_HDMI_INCONSISTENT_PARAMS)
        RETIF(vfInFmt == E_REGVFMT_VFIN_INVALID,
            TMBSL_ERR_HDMI_INCONSISTENT_PARAMS)


        /* Can the i/p register format be scaled to the o/p register format? */
        RETIF(CAN_FMTS_SCALE(regVinFmt,regVoutFmt) == 0,
            TMBSL_ERR_HDMI_INCONSISTENT_PARAMS)

        /* Scaling is possible: Enable scaler */
        err = setHwRegisterFieldTable(pDis, &kScalerOn[0]);
        RETIF_REG_FAIL(err)

        /* Set scaler input format */
        err = setHwRegister(pDis, E_REG_P01_VIDFORMAT_W, vfInFmt);
        RETIF_REG_FAIL(err)
        err = setHwRegisterField(pDis, 
                                 E_REG_P01_SC_VIDFORMAT_W, 
                                 E_MASKREG_P01_SC_VIDFORMAT_vid_format_i,
                                 scInFmt);
        RETIF_REG_FAIL(err)

        /* Set scaler output format */
        err = setHwRegisterField(pDis, 
                                 E_REG_P01_SC_VIDFORMAT_W, 
                                 E_MASKREG_P01_SC_VIDFORMAT_vid_format_o,
                                 scOutFmt);
        RETIF_REG_FAIL(err)
            
        pDis->scaMode = HDMITX_SCAMODE_ON;

        /* Look up PLL scaling */
        pllScgN = kSclModeToScgN[scInFmt][scOutFmt];

        if ((scOutFmt == E_REGVFMT_SCOUT_1080p_50Hz_60Hz) && (pDis->vinMode == HDMITX_VINMODE_CCIR656))
        {
            pllScgN = pllScgN / 2;
        }
        
        /* Set bits [10 to 8] */
        regVal = (UInt8)(pllScgN >> 8) & 7;
        err = setHwRegister(pDis, E_REG_P02_PLL_SCGN2_RW, regVal);
        RETIF_REG_FAIL(err)
        /* Set bits [7 to 0] */
        regVal = (UInt8)pllScgN;
        err = setHwRegister(pDis, E_REG_P02_PLL_SCGN1_RW, regVal);
        RETIF_REG_FAIL(err)
        
        /* Look up PLL scaling */
        pllScgR = kSclModeToScgR[scInFmt][scOutFmt];
        
        /* Set bit [8] */
        regVal = (UInt8)(pllScgR >> 8) & 1;
        err = setHwRegister(pDis, E_REG_P02_PLL_SCGR2_RW, regVal);
        RETIF_REG_FAIL(err)
        /* Set bits [7 to 0] */
        regVal = (UInt8)pllScgR;
        err = setHwRegister(pDis, E_REG_P02_PLL_SCGR1_RW, regVal);
        RETIF_REG_FAIL(err)
    }
#endif /*TMFL_SCALER_SUPPORT*/
    
    /* PR 207  call function setPixelRepeat before wite in E_REG_P00_VIDFORMAT_W*/
    /* Set pixel repetition - sets pixelRepeatCount, used by setScalerFormat */
    err = setPixelRepeat(pDis, pDis->voutFmt, pixelRepeat);
    RETIF(err != TM_OK, err)

    /* Set scaler clock */
    regVal = 0;
    if ((pDis->pixelRepeatCount > HDMITX_PIXREP_MIN) && 
        (pDis->pixelRepeatCount <= HDMITX_PIXREP_MAX))
    {
        regVal = 2;
    }
    else if (pDis->vinMode == HDMITX_VINMODE_CCIR656)
    {
        regVal = (UInt8)((pDis->scaMode == HDMITX_SCAMODE_ON) ? 0 : 1);

	if (pDis->pixRate == HDMITX_PIXRATE_DOUBLE)
	  {
		regVal = 0;
	  }
    }

    err = setHwRegisterField(pDis, 
                             E_REG_P02_SEL_CLK_RW, 
                             E_MASKREG_P02_SEL_CLK_sel_vrf_clk,
                             regVal);
    RETIF_REG_FAIL(err)

    /* Set format register for the selected output format voutFmt */
    /* MUST BE AFTER SCALER CLOCK or sometimes the 9983 won't take the value */
    err = setHwRegister(pDis, E_REG_P00_VIDFORMAT_W, regVoutFmt);
    RETIF_REG_FAIL(err)

    /** \todo Evaluation of customer fix for 576i -> 720 p issue *************/

#ifdef TMFL_SCALER_SUPPORT
    if (pDis->scaMode == HDMITX_SCAMODE_ON)
    {
        /* Set scaler input format again */
        /* MUST BE AFTER SCALER CLOCK or sometimes the 9983 won't take value */
        err = setHwRegister(pDis, E_REG_P01_VIDFORMAT_W, vfInFmt);
        RETIF_REG_FAIL(err)
        err = setHwRegisterField(pDis,
                                E_REG_P01_SC_VIDFORMAT_W,
                                E_MASKREG_P01_SC_VIDFORMAT_vid_format_i,
                                scInFmt);
        RETIF_REG_FAIL(err)

        /* Set scaler output format again */
        /* MUST BE AFTER SCALER CLOCK or sometimes the 9983 won't take value */
        err = setHwRegisterField(pDis,
                                E_REG_P01_SC_VIDFORMAT_W,
                                E_MASKREG_P01_SC_VIDFORMAT_vid_format_o,
                                scOutFmt);
        RETIF_REG_FAIL(err)
    }

#else /* TMFL_SCALER_SUPPORT */
    vfInFmt = vfInFmt;
    regVinFmt = regVinFmt;
    scInFmt = scInFmt;
    scOutFmt = scOutFmt;
    pllScgN = pllScgN;
    pllScgR = pllScgR;
#endif /* TMFL_SCALER_SUPPORT */
    return err;
}

/*============================================================================*/
/* calculateChecksum - returns the byte needed to yield a checksum of zero    */
/*============================================================================*/
static UInt8
calculateChecksum
(   
    UInt8       *pData,     /* Pointer to checksum data */
    Int         numBytes    /* Number of bytes over which to calculate */
    )
{
    UInt8       checksum = 0;   /* Working checksum calculation */
    UInt8       result = 0;     /* Value to be returned */
    Int         i;

    if((pData != Null) && (numBytes > 0))
    {
        for (i = 0; i < numBytes; i++)
        {
            checksum = checksum + (*(pData + i));
        }
        result = (255 - checksum) + 1;
    }
    return result;          /* returns 0 in the case of null ptr or 0 bytes */
}

/*============================================================================*/
/* InputConfig                                                                */
/*============================================================================*/
static tmErrorCode_t
InputConfig
(
    tmHdmiTxobject_t           *pDis,
    tmbslHdmiTxVinMode_t       vinMode,
    tmbslHdmiTxPixEdge_t       sampleEdge,
    tmbslHdmiTxPixRate_t       pixRate,
    tmbslHdmiTxUpsampleMode_t  upsampleMode, 
    UInt8                      uPixelRepeat,
    tmbslHdmiTxVidFmt_t        voutFmt
)
{
    tmErrorCode_t err = TM_OK;  /* Error code */
    UInt8         regVfmt;      /* Video format value used for register */
    UInt8         regVfmtTvFormat; /* used as index in tab pllssd for TV*/
    UInt8         ssd;          /* Packed srl, scg and de */
    tmbslHdmiTxVidFmt_t  voutFmtIndex; /* Index in tab kVfmtToRegvfmt_TV*/
    UInt8         *TabSettingPllSsd = Null; /* Pointer on a tab of kRegvfmtToPllSsd*/

#ifdef FORMAT_PC
    Bool          bIsTvFormat;  /* True if TV_FORMAT found (default) */
    bIsTvFormat = True;
#endif /* FORMAT_PC */

    /****************Check Parameters********************/
    /* Check parameters */
    RETIF_BADPARAM(vinMode      >= HDMITX_VINMODE_INVALID)
    RETIF_BADPARAM(sampleEdge   >= HDMITX_PIXEDGE_INVALID)
    RETIF_BADPARAM(pixRate      >= HDMITX_PIXRATE_INVALID)
    RETIF_BADPARAM(upsampleMode >= HDMITX_UPSAMPLE_INVALID)

    RETIF(voutFmt == HDMITX_VFMT_NO_CHANGE, TM_OK)
#ifdef FORMAT_PC
    RETIF_BADPARAM(voutFmt       < HDMITX_VFMT_TV_MIN)
    RETIF_BADPARAM((voutFmt > HDMITX_VFMT_TV_MAX) && (voutFmt < HDMITX_VFMT_PC_MIN))
    RETIF_BADPARAM(voutFmt       > HDMITX_VFMT_PC_MAX)
#else /* FORMAT_PC */
    /*FORMAT TV only*/
    RETIF_BADPARAM(voutFmt       < HDMITX_VFMT_TV_MIN)
    RETIF_BADPARAM(voutFmt       > HDMITX_VFMT_TV_MAX)
#endif /* FORMAT_PC */
    voutFmtIndex = voutFmt;

#ifdef FORMAT_PC
    if (voutFmtIndex > HDMITX_VFMT_PC_MIN)
    {
        voutFmtIndex = (tmbslHdmiTxVidFmt_t)(voutFmtIndex - HDMITX_VFMT_PC_MIN + HDMITX_VFMT_TV_MAX + (tmbslHdmiTxVidFmt_t)1);
        bIsTvFormat = False; /* PC_FORMAT found */
    }
#endif /* FORMAT_PC */
    
        /* Quit if the output format does not map to the register format */
    regVfmt = kVfmtToRegvfmt_TV[voutFmtIndex];
    RETIF_BADPARAM(regVfmt == E_REGVFMT_INVALID)

/****************Set the VinMode************************
- P00_VIP_CNTRL_4_ccir656
- P00_HVF_CNTRL_1_semi_planar
- P02_PLL_SERIAL_3_srl_ccir
- P02_SEL_CLK_sel_vrf_clk
*/
    if (vinMode != HDMITX_VINMODE_NO_CHANGE)
    {
        pDis->vinMode = vinMode;
    }
/****************Set the sampleEdge***********************
-P00_VIP_CNTRL_3_edge*/

    if (sampleEdge != HDMITX_PIXEDGE_NO_CHANGE)
    {
        err = setHwRegisterField(pDis,
                                 E_REG_P00_VIP_CNTRL_3_W,
                                 E_MASKREG_P00_VIP_CNTRL_3_edge,
                                 (UInt8)sampleEdge);
        RETIF_REG_FAIL(err)
    }

/****************Set the Pixel Rate***********************
-P02_CCIR_DIV_refdiv2
-P02_P02_PLL_SCG2_selpllclkin
-P02_P02_PLL_DE_bypass_pllde
-P00_VIP_CNTRL_4_656_alt*/
if (pixRate != HDMITX_PIXRATE_NO_CHANGE)
    {
    pDis->pixRate = pixRate;
    }

if ((pixRate != HDMITX_PIXRATE_NO_CHANGE)||(vinMode != HDMITX_VINMODE_NO_CHANGE))
{
    switch (pDis->vinMode)
    {
    case HDMITX_VINMODE_RGB444:
    case HDMITX_VINMODE_YUV444:    
    
    if (pDis->pixRate == HDMITX_PIXRATE_SINGLE)
        {
            err = setHwRegisterFieldTable(pDis, &kVinMode444[0]);

            RETIF_REG_FAIL(err)

            

            err = setHwRegisterField(pDis, 
                            E_REG_P00_VIP_CNTRL_4_W,
                            E_MASKREG_P00_VIP_CNTRL_4_656_alt,
                            0);
            RETIF_REG_FAIL(err)

        }
        else if (pDis->pixRate == HDMITX_PIXRATE_SINGLE_REPEATED)
        {
           err = setHwRegisterFieldTable(pDis, &kVinMode444[0]);

            RETIF_REG_FAIL(err)

            

            err = setHwRegisterField(pDis, 
                            E_REG_P00_VIP_CNTRL_4_W,
                            E_MASKREG_P00_VIP_CNTRL_4_656_alt,
                            0);
            RETIF_REG_FAIL(err)

        }
        else 
        {
            /* Not supported*/
        }
    break;
    case HDMITX_VINMODE_YUV422: 
    if (pDis->pixRate == HDMITX_PIXRATE_SINGLE)
        {
            err = setHwRegisterFieldTable(pDis, &kVinModeYUV422[0]);

            RETIF_REG_FAIL(err)

            

            err = setHwRegisterField(pDis, 
                            E_REG_P00_VIP_CNTRL_4_W,
                            E_MASKREG_P00_VIP_CNTRL_4_656_alt,
                            0);
            RETIF_REG_FAIL(err)

        }
    else if (pDis->pixRate == HDMITX_PIXRATE_SINGLE_REPEATED)
        {
            err = setHwRegisterFieldTable(pDis, &kVinModeYUV422[0]);

            RETIF_REG_FAIL(err)

            

            err = setHwRegisterField(pDis, 
                            E_REG_P00_VIP_CNTRL_4_W,
                            E_MASKREG_P00_VIP_CNTRL_4_656_alt,
                            0);
            RETIF_REG_FAIL(err)

        }
        else 
        {
            /* Not supported*/
            return TMBSL_ERR_HDMI_BAD_PARAMETER;
        }
    break;
    case HDMITX_VINMODE_CCIR656:
        if(pDis->pixRate == HDMITX_PIXRATE_SINGLE)
        {

            err = setHwRegisterFieldTable(pDis, &kVinModeCCIR656[0]);

            RETIF_REG_FAIL(err)

            

            err = setHwRegisterField(pDis, 
                            E_REG_P00_VIP_CNTRL_4_W,
                            E_MASKREG_P00_VIP_CNTRL_4_656_alt,
                            0);
            RETIF_REG_FAIL(err)

        }
        else if (pDis->pixRate == HDMITX_PIXRATE_SINGLE_REPEATED)
        {
            err = setHwRegisterFieldTable(pDis, &kVinModeCCIR656[0]);

            RETIF_REG_FAIL(err)

            

            err = setHwRegisterField(pDis, 
                            E_REG_P00_VIP_CNTRL_4_W,
                            E_MASKREG_P00_VIP_CNTRL_4_656_alt,
                            0);
            RETIF_REG_FAIL(err)

        }
        else if (pDis->pixRate == HDMITX_PIXRATE_DOUBLE)
        {
            err = setHwRegisterFieldTable(pDis, &kVinModeCCIR656_DDR_above720p[0]);

            RETIF_REG_FAIL(err)

            


            err = setHwRegisterField(pDis, 
                                     E_REG_P00_VIP_CNTRL_4_W,
                                     E_MASKREG_P00_VIP_CNTRL_4_656_alt,
                                     1);
            RETIF_REG_FAIL(err)

        }
     break;
     default:
            err = setHwRegisterFieldTable(pDis, &kVinMode444[0]);

            RETIF_REG_FAIL(err)
    break;
    }

}
/****************Update the Sample Mode***********************/

    if (upsampleMode != HDMITX_UPSAMPLE_NO_CHANGE)
    {
        pDis->upsampleMode = upsampleMode;
    }

/****************Set the Pixel repeat PLL Value ***********************
- P02_PLL_SERIAL_2_srl_nosc
- P02_PLL_DE_pllde_nosc */

#ifdef FORMAT_PC
    if (bIsTvFormat)
    {
#endif /* FORMAT_PC */

        /* Check parameters */
        RETIF_BADPARAM( regVfmt >= E_REGVFMT_NUM_TV )

        regVfmtTvFormat = regVfmt;

        switch (pDis->scaMode)
        {
        /*Scaler Mode OFF*/
        case HDMITX_SCAMODE_OFF:
    
            switch (pDis->vinMode)
            {
                case HDMITX_VINMODE_RGB444:
                case HDMITX_VINMODE_YUV444:
                case HDMITX_VINMODE_YUV422:
           
                    switch (pDis->pixRate)
                    {
                        case HDMITX_PIXRATE_SINGLE:
                        case HDMITX_PIXRATE_SINGLE_REPEATED:
                            TabSettingPllSsd = (UInt8 *)kRegvfmtToPllSsd.ScaOffCcirOffDblEdgeOff;
                           
                        break;
                        
                        case HDMITX_PIXRATE_DOUBLE:
                            return TMBSL_ERR_HDMI_BAD_PARAMETER;
                        break;             
                        
                        default:
                            return TMBSL_ERR_HDMI_BAD_PARAMETER;
                    }
                        
                break;
                
                case HDMITX_VINMODE_CCIR656:
                
                    switch (pDis->pixRate)
                    {
                        case HDMITX_PIXRATE_SINGLE:
                        case HDMITX_PIXRATE_SINGLE_REPEATED:
                            TabSettingPllSsd = (UInt8 *)kRegvfmtToPllSsd.ScaOffCcirOnDblEdgeOff;
                             
                        break;

                        case HDMITX_PIXRATE_DOUBLE:
                            TabSettingPllSsd = (UInt8 *)kRegvfmtToPllSsd.ScaOffCcirOnDblEdgeOn;
                       
                        break;
                        
                        default:
                            return TMBSL_ERR_HDMI_BAD_PARAMETER;
                        
                    }
                break;
                
                default:
                return TMBSL_ERR_HDMI_BAD_PARAMETER;
            }
        break;
          /*Scaler Mode ON*/
        case HDMITX_SCAMODE_ON:
           switch (pDis->vinMode)
            {
                case HDMITX_VINMODE_RGB444:
                case HDMITX_VINMODE_YUV444:
                case HDMITX_VINMODE_YUV422:
        
                    switch (pDis->pixRate)
                    {
                        case HDMITX_PIXRATE_SINGLE:
                        case HDMITX_PIXRATE_SINGLE_REPEATED:
                        /*Reset 656_Alt bit in VIP_CONTROL_4 Register*/
                        err = setHwRegisterField( pDis, E_REG_P00_VIP_CNTRL_4_W, E_MASKREG_P00_VIP_CNTRL_4_656_alt, 0);
                            switch (pDis->vinFmt)
                            {
                                case HDMITX_VFMT_05_1920x1080i_60Hz:
                                case HDMITX_VFMT_06_720x480i_60Hz:
                                case HDMITX_VFMT_07_720x480i_60Hz:
                                case HDMITX_VFMT_10_720x480i_60Hz:
                                case HDMITX_VFMT_11_720x480i_60Hz:
                                case HDMITX_VFMT_20_1920x1080i_50Hz:
                                case HDMITX_VFMT_21_720x576i_50Hz:
                                case HDMITX_VFMT_22_720x576i_50Hz:
                                case HDMITX_VFMT_25_720x576i_50Hz:
                                case HDMITX_VFMT_26_720x576i_50Hz:
                                    TabSettingPllSsd = (UInt8 *)kRegvfmtToPllSsd.ScaOnCcirOffDblEdgeOffInterlace;
                                break;
                                
                                case HDMITX_VFMT_02_720x480p_60Hz:
                                case HDMITX_VFMT_03_720x480p_60Hz:
                                case HDMITX_VFMT_04_1280x720p_60Hz:
                                case HDMITX_VFMT_17_720x576p_50Hz:
                                case HDMITX_VFMT_18_720x576p_50Hz:
                                case HDMITX_VFMT_19_1280x720p_50Hz:
                                    TabSettingPllSsd = (UInt8 *)kRegvfmtToPllSsd.ScaOnCcirOffDblEdgeOffProgressif;
                                break;
                                default:
                                     return TMBSL_ERR_HDMI_BAD_PARAMETER;
                            }
                        
                            if ( (((pDis->vinFmt == HDMITX_VFMT_05_1920x1080i_60Hz) ||
                                   (pDis->vinFmt == HDMITX_VFMT_20_1920x1080i_50Hz) ||
                                   (pDis->vinFmt == HDMITX_VFMT_04_1280x720p_60Hz)  ||
                                   (pDis->vinFmt == HDMITX_VFMT_19_1280x720p_50Hz))&& 
                                  ((pDis->voutFmt == HDMITX_VFMT_16_1920x1080p_60Hz)||
                                   (pDis->voutFmt == HDMITX_VFMT_31_1920x1080p_50Hz))) ||
                                   (pDis->voutFmt == HDMITX_VFMT_32_1920x1080p_24Hz) ||
                                   (pDis->voutFmt == HDMITX_VFMT_33_1920x1080p_25Hz) ||
                                   (pDis->voutFmt == HDMITX_VFMT_34_1920x1080p_30Hz))

                            {
                                regVfmtTvFormat = regVfmtTvFormat + 1;
                            }

                        break;
                        case HDMITX_PIXRATE_DOUBLE:
                        /*Reset 656_Alt bit in VIP_CONTROL_4 Register*/
                        err = setHwRegisterField( pDis, E_REG_P00_VIP_CNTRL_4_W, E_MASKREG_P00_VIP_CNTRL_4_656_alt, 0);
                        break;             
                        default:
                        return TMBSL_ERR_HDMI_BAD_PARAMETER;
                    }
                        
                break;
                
                case HDMITX_VINMODE_CCIR656:
                
                    switch (pDis->pixRate)
                    {
                        case HDMITX_PIXRATE_SINGLE:
                        case HDMITX_PIXRATE_SINGLE_REPEATED:
                        /*Reset 656_Alt bit in VIP_CONTROL_4 Register*/
                        err = setHwRegisterField( pDis, E_REG_P00_VIP_CNTRL_4_W, E_MASKREG_P00_VIP_CNTRL_4_656_alt, 0);
                            switch (pDis->vinFmt)
                            {
                                case HDMITX_VFMT_06_720x480i_60Hz:
                                case HDMITX_VFMT_07_720x480i_60Hz:
                                case HDMITX_VFMT_10_720x480i_60Hz:
                                case HDMITX_VFMT_11_720x480i_60Hz:
                                case HDMITX_VFMT_21_720x576i_50Hz:
                                case HDMITX_VFMT_22_720x576i_50Hz:
                                case HDMITX_VFMT_25_720x576i_50Hz:
                                case HDMITX_VFMT_26_720x576i_50Hz:
                                    TabSettingPllSsd = (UInt8 *)kRegvfmtToPllSsd.ScaOnCcirOnDblEdgeOffInterlace;
                                break;
                                
                                case HDMITX_VFMT_02_720x480p_60Hz:
                                case HDMITX_VFMT_03_720x480p_60Hz:
                                case HDMITX_VFMT_17_720x576p_50Hz:
                                case HDMITX_VFMT_18_720x576p_50Hz:
                                    TabSettingPllSsd = (UInt8 *)kRegvfmtToPllSsd.ScaOnCcirOnDblEdgeOffProgressif;
                                break;
                                            
                                default:
                                return TMBSL_ERR_HDMI_BAD_PARAMETER;
                            }

                        
                        break;
                        
                        case HDMITX_PIXRATE_DOUBLE:
                        /*Set 656_Alt bit in VIP_CONTROL_4 Register*/
                        err = setHwRegisterField( pDis, E_REG_P00_VIP_CNTRL_4_W, E_MASKREG_P00_VIP_CNTRL_4_656_alt, 1);
                            switch (pDis->vinFmt)
                            {
                                case HDMITX_VFMT_06_720x480i_60Hz:
                                case HDMITX_VFMT_07_720x480i_60Hz:
                                case HDMITX_VFMT_10_720x480i_60Hz:
                                case HDMITX_VFMT_11_720x480i_60Hz:
                                case HDMITX_VFMT_21_720x576i_50Hz:
                                case HDMITX_VFMT_22_720x576i_50Hz:
                                case HDMITX_VFMT_25_720x576i_50Hz:
                                case HDMITX_VFMT_26_720x576i_50Hz:
                                    TabSettingPllSsd = (UInt8 *)kRegvfmtToPllSsd.ScaOnCcirOnDblEdgeOnInterlace;
                                break;
                                
                                case HDMITX_VFMT_02_720x480p_60Hz:
                                case HDMITX_VFMT_03_720x480p_60Hz:
                                case HDMITX_VFMT_17_720x576p_50Hz:
                                case HDMITX_VFMT_18_720x576p_50Hz:
                                    TabSettingPllSsd = (UInt8 *)kRegvfmtToPllSsd.ScaOnCcirOnDblEdgeOnProgressif;
                                break;
                                            
                                default:
                                return TMBSL_ERR_HDMI_BAD_PARAMETER;
                            }
                        
                        break;
                        
                        default:
                        return TMBSL_ERR_HDMI_BAD_PARAMETER;
                    }
                
                break;
                
                default:
                return TMBSL_ERR_HDMI_BAD_PARAMETER;
            }
        
        break;

        default:
        return TMBSL_ERR_HDMI_BAD_PARAMETER;
        }

         if (TabSettingPllSsd) {

            /* Set PLLs based on output format */
            ssd = TabSettingPllSsd[regVfmtTvFormat];

         } else
            {
               return TMBSL_ERR_HDMI_BAD_PARAMETER;
            }

#ifdef FORMAT_PC
    }
    else
    {
        TabSettingPllSsd = (UInt8 *) kRegvfmtToPllSsd.SettingsFormatPC;
        
        /* Check parameters */
        RETIF_BADPARAM( regVfmt < E_REGVFMT_NUM_TV )
        
        ssd = TabSettingPllSsd[regVfmt - E_REGVFMT_NUM_TV];
    }
        
#endif /* FORMAT_PC */

    
    if ( ssd < SSD_UNUSED_VALUE)
    {
        
        err = setHwRegisterField(pDis, E_REG_P02_PLL_SERIAL_2_RW,
                             E_MASKREG_P02_PLL_SERIAL_2_srl_nosc,
                             SSD2SRL(ssd));
    }

/*****************Set the Pixel Repetition***********************
- P02_PLL_SERIAL_2_srl_pr*/

    /* Set pixel repetition */
    if (uPixelRepeat != HDMITX_PIXREP_NO_CHANGE)
    {
        if (uPixelRepeat == HDMITX_PIXREP_DEFAULT)
        {
            /* Look up default pixel repeat value for this output format */
            uPixelRepeat = kRegvfmtToPixRep[regVfmt];
        }

        /* Update current pixel repetition count */
        pDis->pixelRepeatCount = uPixelRepeat;

        err = setHwRegisterField(pDis, 
                                 E_REG_P02_PLL_SERIAL_2_RW, 
                                 E_MASKREG_P02_PLL_SERIAL_2_srl_pr,
                                 uPixelRepeat);
        RETIF_REG_FAIL(err)
        /* Set pixel repetition count for Repetitor module */
        err = setHwRegister(pDis, E_REG_P00_RPT_CNTRL_W, uPixelRepeat);
    }

/*******************Fixe other settings*********************
- P02_PLL_SERIAL_1_srl_man_iz     = 0
- P02_PLL_SERIAL_3_srl_de         = 0
- Pol Clk Sel = P02_SERIALIZER_RW = 0
- P02_BUFFER_OUT_srl_force        = 0 
- P02_BUFFER_OUT_srl_clk          = 0
- P02_PLL_DE_pllde_iz             = 0
*/


    err = setHwRegisterField(pDis, 
                            E_REG_P02_PLL_SERIAL_1_RW, 
                            E_MASKREG_P02_PLL_SERIAL_1_srl_man_iz,
                            0);


RETIF_REG_FAIL(err)

err = setHwRegisterField(pDis, 
                                 E_REG_P02_PLL_SERIAL_3_RW, 
                                 E_MASKREG_P02_PLL_SERIAL_3_srl_de,
                                 0);
RETIF_REG_FAIL(err)

err = setHwRegister(pDis, E_REG_P02_SERIALIZER_RW, 0);
RETIF_REG_FAIL(err)

return err;
}

/*============================================================================*/
/*                            END OF FILE                                     */
/*============================================================================*/

