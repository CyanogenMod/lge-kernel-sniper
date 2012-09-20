/**
 * Copyright (C) 2006 NXP N.V., All Rights Reserved.
 * This source code and any compilation or derivative thereof is the proprietary
 * information of NXP N.V. and is confidential in nature. Under no circumstances
 * is this software to be  exposed to or placed under an Open Source License of
 * any type without the expressed written permission of NXP N.V.
 *
 * \file          tmbslTDA9984_HDCP.c
 *
 * \version       Revision: 20
 *
 * \date          Date: 01/04/08
 *
 * \brief         BSL driver component API for the TDA9984 HDMI Transmitter
 *
 * \section refs  Reference Documents
 *
 * \section info  Change Information
 *
 * \verbatim

 * History: tmbslTDA9984_HDCP.c
 *
 * **************** Version 15  ******************
 * User: G.Burnouf     Date: 01/04/08
 * Updated in $/Source/tmbslTDA9984/Src
 * PR1468 : add new function tmbslTDA9984GetSinkCategory
 *

   \endverbatim
 *
*/

/*============================================================================*/
/*                   FILE CONFIGURATION                                       */
/*============================================================================*/


/*============================================================================*/
/*                       INCLUDE FILES                                        */
/*============================================================================*/

#include "tmbslHdmiTx_types.h"
#include "tmbslTDA9984_Functions.h"
#include "tmbslTDA9984_local.h"
#include "tmbslTDA9984_State_l.h"
#include "tmbslTDA9984_InOut_l.h"
#include "tmbslTDA9984_HDCP_l.h"

/*============================================================================*/
/*                     TYPES DECLARATIONS                                     */
/*============================================================================*/

/*============================================================================*/
/*                       CONSTANTS DECLARATIONS EXPORTED                      */
/*============================================================================*/

/**
 * Table of registers to switch HDMI HDCP mode off for DVI
 */

CONST_DAT tmHdmiTxRegMaskVal_t kVoutHdcpOff[] =
{
    {E_REG_P00_TBG_CNTRL_1_W,   E_MASKREG_P00_TBG_CNTRL_1_dwin_dis, 1},
    {E_REG_P12_TX33_RW,    E_MASKREG_P12_TX33_hdmi,       0},
    {0,0,0}
};

/**
 * Table of registers to switch HDMI HDCP mode on for HDMI
 */
CONST_DAT tmHdmiTxRegMaskVal_t kVoutHdcpOn[] =
{
    {E_REG_P00_TBG_CNTRL_1_W,   E_MASKREG_P00_TBG_CNTRL_1_dwin_dis, 0},
    {E_REG_P11_ENC_CNTRL_RW,    E_MASKREG_P11_ENC_CNTRL_ctl_code,   1},
    {E_REG_P12_TX33_RW,    E_MASKREG_P12_TX33_hdmi,       1},
    {0,0,0}
};


/*============================================================================*/
/* tmbslTDA9984HdcpCheck                                                      */
/*============================================================================*/
tmErrorCode_t
tmbslTDA9984HdcpCheck
(
    tmUnitSelect_t          txUnit,
    UInt16                  uTimeSinceLastCallMs,
    tmbslHdmiTxHdcpCheck_t  *pResult
)
{
    return TMBSL_ERR_HDMI_NOT_SUPPORTED;

}

/*============================================================================*/
/* tmbslTDA9984HdcpConfigure                                                   */
/*============================================================================*/
tmErrorCode_t
tmbslTDA9984HdcpConfigure
(
    tmUnitSelect_t           txUnit,
    UInt8                    slaveAddress,
    tmbslHdmiTxHdcpTxMode_t  txMode,
    tmbslHdmiTxHdcpOptions_t options,
    UInt16                   uCheckIntervalMs,
    UInt8                    uChecksToDo
)
{
    return TMBSL_ERR_HDMI_NOT_SUPPORTED;
}

/*============================================================================*/
/* tmbslTDA9984HdcpDownloadKeys                                                */
/*============================================================================*/
tmErrorCode_t
tmbslTDA9984HdcpDownloadKeys
(
    tmUnitSelect_t          txUnit,
    UInt16                  seed,
    tmbslHdmiTxDecrypt_t    keyDecryption
)
{
    return TMBSL_ERR_HDMI_NOT_SUPPORTED;
}

/*============================================================================*/
/* tmbslTDA9984HdcpEncryptionOn                                                */
/*============================================================================*/

tmErrorCode_t
tmbslTDA9984HdcpEncryptionOn
(
    tmUnitSelect_t  txUnit,
    Bool            bOn
)
{
    return TMBSL_ERR_HDMI_NOT_SUPPORTED;
}


/*============================================================================*/
/* tmbslTDA9984HdcpGetOtp                                                      */
/*============================================================================*/
tmErrorCode_t
tmbslTDA9984HdcpGetOtp
(
    tmUnitSelect_t          txUnit,
    UInt8                   otpAddress,
    UInt8                   *pOtpData
)
{
    return TMBSL_ERR_HDMI_NOT_SUPPORTED;
}

/*============================================================================*/
/* tmbslTDA9984HdcpGetT0FailState                                              */
/*============================================================================*/
tmErrorCode_t
tmbslTDA9984HdcpGetT0FailState
(
    tmUnitSelect_t  txUnit,
    UInt8           *pFailState
)
{
    return TMBSL_ERR_HDMI_NOT_SUPPORTED;
}

/*============================================================================*/
/* tmbslTDA9984HdcpHandleBCAPS                                                 */
/*============================================================================*/
tmErrorCode_t
tmbslTDA9984HdcpHandleBCAPS
(
    tmUnitSelect_t  txUnit
)
{
    return TMBSL_ERR_HDMI_NOT_SUPPORTED;

}

/*============================================================================*/
/* tmbslTDA9984HdcpHandleBKSV                                                  */
/*============================================================================*/
tmErrorCode_t
tmbslTDA9984HdcpHandleBKSV
(
    tmUnitSelect_t  txUnit,
    UInt8           *pBksv,
    Bool            *pbCheckRequired  /* May be null, but only for testing */
)
{
    return TMBSL_ERR_HDMI_NOT_SUPPORTED;

}

/*============================================================================*/
/* tmbslTDA9984HdcpHandleBKSVResult                                            */
/*============================================================================*/
tmErrorCode_t
tmbslTDA9984HdcpHandleBKSVResult
(
    tmUnitSelect_t  txUnit,
    Bool            bSecure
)
{
    return TMBSL_ERR_HDMI_NOT_SUPPORTED;

}

/*============================================================================*/
/* tmbslTDA9984HdcpHandleBSTATUS                                               */
/*============================================================================*/
tmErrorCode_t
tmbslTDA9984HdcpHandleBSTATUS
(
    tmUnitSelect_t  txUnit,
    UInt16          *pBstatus   /* May be null */
)
{
    return TMBSL_ERR_HDMI_NOT_SUPPORTED;

}

/*============================================================================*/
/* tmbslTDA9984HdcpHandleENCRYPT                                               */
/*============================================================================*/
tmErrorCode_t
tmbslTDA9984HdcpHandleENCRYPT
(
    tmUnitSelect_t  txUnit
)
{
    return TMBSL_ERR_HDMI_NOT_SUPPORTED;

}

/*============================================================================*/
/* tmbslTDA9984HdcpHandlePJ                                                    */
/*============================================================================*/
tmErrorCode_t
tmbslTDA9984HdcpHandlePJ
(
    tmUnitSelect_t  txUnit
)
{
    return TMBSL_ERR_HDMI_NOT_SUPPORTED;

}

/*============================================================================*/
/* tmbslTDA9984HdcpHandleSHA_1                                                 */
/*============================================================================*/
tmErrorCode_t
tmbslTDA9984HdcpHandleSHA_1
(
    tmUnitSelect_t  txUnit,
    UInt8           maxKsvDevices,
    UInt8           *pKsvList,          /* May be null if maxKsvDevices is 0 */
    UInt8           *pnKsvDevices,      /* May be null if maxKsvDevices is 0 */
    UInt8           *pDepth             /* Connection tree depth returned with KSV list */
)
{
    return TMBSL_ERR_HDMI_NOT_SUPPORTED;
}

/*============================================================================*/
/* tmbslTDA9984HdcpHandleSHA_1Result                                           */
/*============================================================================*/
tmErrorCode_t
tmbslTDA9984HdcpHandleSHA_1Result
(
    tmUnitSelect_t  txUnit,
    Bool            bSecure
)
{
    return TMBSL_ERR_HDMI_NOT_SUPPORTED;
}

/*============================================================================*/
/* tmbslTDA9984HdcpHandleT0                                                    */
/*============================================================================*/
tmErrorCode_t
tmbslTDA9984HdcpHandleT0
(
    tmUnitSelect_t  txUnit
)
{
    return TMBSL_ERR_HDMI_NOT_SUPPORTED;
}

/*============================================================================*/
/* tmbslTDA9984HdcpInit                                                        */
/* RETIF_REG_FAIL NOT USED HERE AS ALL ERRORS SHOULD BE TRAPPED IN ALL BUILDS */
/*============================================================================*/
tmErrorCode_t
tmbslTDA9984HdcpInit
(
    tmUnitSelect_t      txUnit,
    tmbslHdmiTxVidFmt_t voutFmt,
    tmbslHdmiTxVfreq_t  voutFreq
)
{
    return TMBSL_ERR_HDMI_NOT_SUPPORTED;
}

/*============================================================================*/
/* tmbslTDA9984HdcpRun                                                         */
/*============================================================================*/
tmErrorCode_t
tmbslTDA9984HdcpRun
(
    tmUnitSelect_t  txUnit
)
{
    return TMBSL_ERR_HDMI_NOT_SUPPORTED;
}

/*============================================================================*/
/* tmbslTDA9984HdcpStop                                                        */
/*============================================================================*/
tmErrorCode_t
tmbslTDA9984HdcpStop
(
    tmUnitSelect_t  txUnit
)
{
    return TMBSL_ERR_HDMI_NOT_SUPPORTED;
}

/*
 * internal
 */

/*============================================================================*/
/* tmbslTDA9984HdcpGetSinkCategory                                            */
/*============================================================================*/
tmErrorCode_t
tmbslTDA9984HdcpGetSinkCategory
(
    tmUnitSelect_t              txUnit,
    tmbslHdmiTxSinkCategory_t   *category
)
{
    return TMBSL_ERR_HDMI_NOT_SUPPORTED;
}




/*============================================================================*/
/* tmbslTDA9984handleBKSVResultSecure                                         */
/*============================================================================*/
tmErrorCode_t
tmbslTDA9984handleBKSVResultSecure
(
    tmUnitSelect_t  txUnit
)
{
    return TMBSL_ERR_HDMI_NOT_SUPPORTED;
}

/*============================================================================*/
/*                            END OF FILE                                     */
/*============================================================================*/
