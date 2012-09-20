/**
 * Copyright (C) 2006 Koninklijke Philips Electronics N.V., All Rights Reserved.
 * This source code and any compilation or derivative thereof is the proprietary
 * information of Koninklijke Philips Electronics N.V. and is confidential in
 * nature. Under no circumstances is this software to be  exposed to or placed
 * under an Open Source License of any type without the expressed written
 * permission of Koninklijke Philips Electronics N.V.
 *
 * \file          tmbslHdmiTx_local.h
 *
 * \version       $Revision: 66 $
 *
 * \date          $Date: 29/10/07 14:11 $
 *
 * \brief         BSL driver component local definitions for the TDA998x
 *                HDMI Transmitter.
 *
 * \section refs  Reference Documents
 * HDMI Driver - Outline Architecture.doc,
 * HDMI Driver - tmbslHdmiTx - SCS.doc
 *
 * \section info  Change Information
 *
 * \verbatim
   $History: tmbslHdmiTx_local.h $
 *
 * *****************  Version 66  ****************
 * User: B.Vereecke     Date: 29/10/07   Time: 14:11
 * Updated in $/Source/tmbslHdmiTx/src
 * PR852 : remove external library dependancy
 *
 * *****************  Version 65  ****************
 * User: B.Vereecke     Date: 17/10/07   Time: 14:11
 * Updated in $/Source/tmbslHdmiTx/src
 * PR872 : add new formats, 1080p24/25/30
 *
 * *****************  Version 64  *****************
 * User: B.Vereecke    Date: 11/09/07   Time: 15:15
 * Updated in $/Source/tmbslHdmiTx/Src
 * PR679: Update version to 3.9
 *
 * *****************  Version 63  *****************
 * User: B.Vereecke    Date: 07/09/07   Time: 16:23
 * Updated in $/Source/tmbslHdmiTx/Src
 * PR670 : Add PC Format : 1280x1024@75Hz
 *
 * *****************  Version 62  *****************
 * User: B.Vereecke    Date: 27/07/07   Time: 11:23
 * Updated in $/Source/tmbslHdmiTx/Src
 * PR536 : Update version to 2.69
 *
 * *****************  Version 61  *****************
 * User: B.Vereecke    Date: 17/07/07   Time: 10:30
 * Updated in $/Source/tmbslHdmiTx/Src
 * PR 217 : Add a new flag in tmHdmiTxobject_t
 *
 * *****************  Version 60  *****************
 * User: J. Lamotte    Date: 26/06/07   Time: 11:37
 * Updated in $/Source/tmbslHdmiTx/Src
 * PR 459 : Update version to 3.7
 *
 * *****************  Version 59  *****************
 * User: J. Lamotte    Date: 13/06/07   Time: 17:00
 * Updated in $/Source/tmbslHdmiTx/Src
 * PR 401 : Update version to 3.6
 *
 * *****************  Version 58  *****************
 * User: J. Lamotte    Date: 13/06/07   Time: 12:00
 * Updated in $/Source/tmbslHdmiTx/Src
 * PR 397 : Merge with PR 391 (ST_CONNECTED rename
 *          to ST_SINK_CONNECTED)
 * 
 * *****************  Version 57  *****************
 * User: Burnouf       Date: 08/06/07   Time: 15:50
 * Updated in $/Source/tmbslHdmiTx/Src
 * PR 347 : Add new PC formats
 * 
 * *****************  Version 56  *****************
 * User: J. Lamotte    Date: 24/05/07   Time: 14:45
 * Updated in $/Source/tmbslHdmiTx/Src
 * PR 50 : Update version to 3.5
 *
 * *****************  Version 55  *****************
 * User: J. Lamotte    Date: 03/05/07   Time: 13:55
 * Updated in $/Source/tmbslHdmiTx/Src
 * PR 50 : Update E_DEV_VERSION_N4 and E_DEV_VERSION_LIST_END
 *         in _eDevVersion enum for TDA9981 chip detection
 *
 * *****************  Version 54  *****************
 * User: Burnouf       Date: 18/04/07   Time: 15:50
 * Updated in $/Source/tmbslHdmiTx/Src
 * PR 50 : Add new state ST_AWAIT_RX_SENSE and
 *         add new events EV_SINKON and EV_SINKOFF for TDA9981
 * 
 * *****************  Version 53  *****************
 * User: J. Lamotte      Date: 17/04/07   Time: 13:00
 * Updated in $/Source/tmbslHdmiTx/src
 * PR50 - disable scaler for TDA9981
 *      - undefined page 1 register for TDA9981
 * Driver 3.2
 * 
 * *****************  Version 52  *****************
 * User: J. Lamotte      Date: 16/04/07   Time: 11:30
 * Updated in $/Source/tmbslHdmiTx/src
 * PR50 - move define of HDMITX_UNITS_MAX in
 *        tmblsHdmiTx.h
 * Driver 3.2
 * 
 * *****************  Version 51  *****************
 * User: J. Lamotte     Date: 25/04/07   Time: 14:50
 * Updated in $/Source/tmbslHdmiTx/src
 * PR273 - add PLL configuration before soft reset
 *         in function tmbslHdmiTxInit
 * Driver 3.4
 * 
 * *****************  Version 50  *****************
 * User: J. Lamotte      Date: 13/04/07   Time: 17:30
 * Updated in $/Source/tmbslHdmiTx/src
 * PR50 - add registers for TDA9981
 * Driver 3.2
 * 
 * *****************  Version 49  *****************
 * User: C. Logiou      Date: 08/03/07   Time: 16:52
 * Updated in $/Source/tmbslHdmiTx/src
 * PR214 - add new input format repeated 480i/576i
 * Driver 3.2
 * 
 * *****************  Version 48  *****************
 * User: Burnouf      Date: 01/03/07   Time: 16:15
 * Updated in $/Source/tmbslHdmiTx/Src
 * PR207
 * Driver 3.1
 *  
 * *****************  Version 48  *****************
 * User: Burnouf      Date: 06/02/07   Time: 16:15
 * Updated in $/Source/tmbslHdmiTx/Src
 * PR49 : add PC Formats
 * Driver 3.0
 *  
 * *****************  Version 47  *****************
 * User: Burnouf      Date: 29/01/07   Time: 16:15
 * Updated in $/Source/tmbslHdmiTx/Src
 * Driver 2.9
 *  
 * *****************  Version 46  *****************
 * User: Burnouf      Date: 08/01/07   Time: 16:15
 * Updated in $/Source/tmbslHdmiTx/Src
 * Driver 2.8
 *  
 * *****************  Version 45  *****************
 * User: Burnouf      Date: 14/12/06   Time: 10:03
 * Updated in $/Source/tmbslHdmiTx/Src
 * Driver 2.7
 *  
 * *****************  Version 44  *****************
 * User: Burnouf      Date: 11/12/06   Time: 11:08
 * Updated in $/Source/tmbslHdmiTx/Src
 * Driver 2.6
 *  
 * *****************  Version 43  *****************
 * User: Burnouf      Date: 07/12/06   Time: 14:20
 * Updated in $/Source/tmbslHdmiTx/Src
 * Driver 2.5
 *  
 * *****************  Version 42  *****************
 * User: Burnouf      Date: 29/11/06   Time: 17:07
 * Updated in $/Source/tmbslHdmiTx/Src
 * Driver 2.4
 *  
 * *****************  Version 41  *****************
 * User: Mayhew       Date: 23/11/06   Time: 15:07
 * Updated in $/Source/tmbslHdmiTx/Src
 * Driver 2.3
 * 
 * *****************  Version 39  *****************
 * User: Djw          Date: 22/11/06   Time: 11:37
 * Updated in $/Source/tmbslHdmiTx/Src
 * PNF74 Changes to serial clock divider values in setPixelRepeat
 * 
 * *****************  Version 37  *****************
 * User: Mayhew       Date: 10/11/06   Time: 10:14
 * Updated in $/Source/tmbslHdmiTx/Src
 * PNF68 RETIF_REG_FAIL macro added, controlled by compiler command line
 * symbol NO_RETIF_REG_FAIL
 * PNF68 DIS members funcScheduled, uFuncScheduledMs replaced by HdcpFunc*
 * and HdcpCheck* members
 * 
 * *****************  Version 36  *****************
 * User: Mayhew       Date: 6/11/06    Time: 17:49
 * Updated in $/Source/tmbslHdmiTx/Src
 * PNF68 Add funcScheduled and uFuncScheduledMs to DIS
 * 
 * *****************  Version 34  *****************
 * User: Mayhew       Date: 2/11/06    Time: 17:05
 * Updated in $/Source/tmbslHdmiTx/Src
 * Version 2.1
 * 
 * *****************  Version 32  *****************
 * User: Mayhew       Date: 31/10/06   Time: 16:23
 * Updated in $/Source/tmbslHdmiTx/Src
 * Version 2.0
 * 
 * *****************  Version 30  *****************
 * User: Mayhew       Date: 27/10/06   Time: 12:33
 * Updated in $/Source/tmbslHdmiTx/Src
 * Version 1.9
 * 
 * *****************  Version 28  *****************
 * User: Mayhew       Date: 23/10/06   Time: 16:41
 * Updated in $/Source/tmbslHdmiTx/Src
 * Driver version 1.8
 * 
 * *****************  Version 26  *****************
 * User: Mayhew       Date: 13/10/06   Time: 11:19
 * Updated in $/Source/tmbslHdmiTx/Src
 * Version 1.7. PNF40 Rename P11_CH_STAT registers correctly
 * 
 * *****************  Version 24  *****************
 * User: Mayhew       Date: 21/09/06   Time: 15:50
 * Updated in $/Source/tmbslHdmiTx/Src
 * Version 1.6. Cut code size in demo by using RETIF_BADPARAM.
 * 
 * *****************  Version 22  *****************
 * User: Mayhew       Date: 15/09/06   Time: 16:10
 * Updated in $/Source/tmbslHdmiTx/Src
 * PNF22 Add HdcpRi & HdcpFsmState to DIS
 * Version 1.5. Fix QAC warning.
 * 
 * *****************  Version 20  *****************
 * User: Mayhew       Date: 7/09/06    Time: 9:43
 * Updated in $/Source/tmbslHdmiTx/Src
 * Minor version now 4
 * 
 * *****************  Version 18  *****************
 * User: Djw          Date: 24/08/06   Time: 12:12
 * Updated in $/Source/tmbslHdmiTx/Src
 * PNF8 Use TX33 register in NO_HDCP build.
 * PNF14 Add DIS.pixelRepeatCount.
 * 
 * *****************  Version 17  *****************
 * User: Djw          Date: 22/08/06   Time: 9:58
 * Updated in $/Source/tmbslHdmiTx/Src
 * NO_HDCP modifications
 * 
 * *****************  Version 16  *****************
 * User: Mayhew       Date: 10/07/06   Time: 13:18
 * Updated in $/Source/tmbslHdmiTx/Src
 * Minor version now 2. Add #define BCAPS_REPEATER.
 * Add N5 registers and feature flag. Fix Doxygen warnings.
 * 
 * *****************  Version 14  *****************
 * User: Mayhew       Date: 30/06/06   Time: 12:55
 * Updated in $/Source/tmbslHdmiTx/Src
 * Minor version now 1. BKSV_SECURE event renamed to BKSV_REPEAT.
 * HDCP_BCAPS register is now shadowed and has new _repeater bit.
 * Add N4 CH_STAT registers. EdidSinkType replaced by SinkType.
 * Add DIS.HdcpAksv.
 * 
 * *****************  Version 12  *****************
 * User: Djw          Date: 16/06/06   Time: 12:04
 * Updated in $/Source/tmbslHdmiTx/Src
 * Added flag to DIS to support use of alternate i2c address for EDID.
 * 
 * *****************  Version 10  *****************
 * User: Mayhew       Date: 6/06/06    Time: 13:41
 * Updated in $/Source/tmbslHdmiTx/Src
 * Add checkUnitSetDis. Remove erroneous NOT_SUPPORTED definition.
 * 
 * *****************  Version 9  *****************
 * User: Mayhew       Date: 5/06/06    Time: 15:06
 * Updated in $/Source/tmbslHdmiTx/Src
 * Move error code definitions to tmbslHdmiTx.h, replace with #if checks
 * Add DIS.HdcpT0FailState.
 * 
 * *****************  Version 7  *****************
 * User: Djw          Date: 24/05/06   Time: 11:23
 * Updated in $/Source/tmbslHdmiTx/Src
 * Added new registers for N4 (page 10h Infoframes).  Added 2nd Ghost
 * register.  Reset names for ISRC1 and ACP packet registers for N4
 * compatibility.
 * 
 * *****************  Version 5  *****************
 * User: Mayhew       Date: 22/05/06   Time: 15:58
 * Updated in $/Source/tmbslHdmiTx/Src
 * Add DIS.uDeviceFeatures. Add N4 to version list. Add version register
 * masks for N4.
 * 
 * *****************  Version 4  *****************
 * User: Mayhew       Date: 10/05/06   Time: 17:10
 * Updated in $/Source/tmbslHdmiTx/Src
 * Add HDCP state handling and DIS members, move EDID status enum to .h
 * 
 * *****************  Version 3  *****************
 * User: Djw          Date: 20/04/06   Time: 17:31
 * Updated in $/Source/tmbslHdmiTx/Src
 * Minor comment correction for EDID DTD in DIS.
 * 
 * *****************  Version 2  *****************
 * User: Mayhew       Date: 11/04/06   Time: 14:19
 * Updated in $/Source/tmbslHdmiTx/Src
 * Add upsampleMode to Device Instance Structure
 * 
 * *****************  Version 1  *****************
 * User: Mayhew       Date: 4/04/06    Time: 16:30
 * Created in $/Source/tmbslHdmiTx/Src
 * Driver local API phase 2

   \endverbatim
 *
*/

#ifndef TMBSLHDMITX_LOCAL_H
#define TMBSLHDMITX_LOCAL_H

/*============================================================================*/
/*                       INCLUDE FILES                                        */
/*============================================================================*/

#ifdef __cplusplus
extern "C" {
#endif

/*============================================================================*/
/*                       MACRO DEFINITIONS                                    */
/*============================================================================*/
/* for remove warning */
#define DUMMY_ACCESS(x) x

/* Defining this symbol here enables the BCAPS Repeater mod for N4 onwards */
#ifndef BCAPS_REPEATER
#define BCAPS_REPEATER
#endif

/** \name Versions
 *  A group of macros to set the software component number and version
 */
/*@{*/
/** Compatibility number */
#define HDMITX_BSL_COMP_NUM             1

/** Major software version 1 to 255 */
#define HDMITX_BSL_MAJOR_VER            4

/** Minor software version 0 to 9 */
#define HDMITX_BSL_MINOR_VER            18
/*@}*/

/** \name ErrorChecks
 *  A group of checks ensuring that public error codes match DVP standard errors
 */
/*@{*/
/** SW interface compatibility error */
#if TMBSL_ERR_HDMI_COMPATIBILITY != \
(TMBSL_ERR_HDMI_BASE + TM_ERR_COMPATIBILITY)
#error
#endif

/** SW major version error */
#if TMBSL_ERR_HDMI_MAJOR_VERSION != \
(TMBSL_ERR_HDMI_BASE + TM_ERR_MAJOR_VERSION)
#error
#endif

/** SW component version error */
#if TMBSL_ERR_HDMI_COMP_VERSION != \
(TMBSL_ERR_HDMI_BASE + TM_ERR_COMP_VERSION)
#error
#endif

/** Invalid device unit number */
#if TMBSL_ERR_HDMI_BAD_UNIT_NUMBER != \
(TMBSL_ERR_HDMI_BASE + TM_ERR_BAD_UNIT_NUMBER)
#error
#endif

/** Invalid input parameter other than unit number */
#if TMBSL_ERR_HDMI_BAD_PARAMETER != \
(TMBSL_ERR_HDMI_BASE + TM_ERR_BAD_PARAMETER)
#error
#endif

/** Inconsistent input parameters */
#if TMBSL_ERR_HDMI_INCONSISTENT_PARAMS != \
(TMBSL_ERR_HDMI_BASE + TM_ERR_INCONSISTENT_PARAMS)
#error
#endif

/** Component is not initialized */
#if TMBSL_ERR_HDMI_NOT_INITIALIZED != \
(TMBSL_ERR_HDMI_BASE + TM_ERR_NOT_INITIALIZED)
#error
#endif

/** Command not supported for current device */
#if TMBSL_ERR_HDMI_NOT_SUPPORTED != \
(TMBSL_ERR_HDMI_BASE + TM_ERR_NOT_SUPPORTED)
#error
#endif

/** Initialization failed */
#if TMBSL_ERR_HDMI_INIT_FAILED != \
(TMBSL_ERR_HDMI_BASE + TM_ERR_INIT_FAILED)
#error
#endif

/** Component is busy and cannot do a new operation */
#if TMBSL_ERR_HDMI_BUSY != \
(TMBSL_ERR_HDMI_BASE + TM_ERR_BUSY)
#error
#endif

/** I2C read error */
#if TMBSL_ERR_HDMI_I2C_READ != \
(TMBSL_ERR_HDMI_BASE + TM_ERR_READ)
#error
#endif

/** I2C write error */
#if TMBSL_ERR_HDMI_I2C_WRITE != \
(TMBSL_ERR_HDMI_BASE + TM_ERR_WRITE)
#error
#endif

/** Assertion failure */
#if TMBSL_ERR_HDMI_ASSERTION != \
(TMBSL_ERR_HDMI_BASE + TM_ERR_ASSERTION)
#error
#endif

/** Bad EDID block checksum */
#if TMBSL_ERR_HDMI_INVALID_CHECKSUM != \
(TMBSL_ERR_HDMI_BASE + TM_ERR_INVALID_STATE)
#error
#endif

/** No connection to HPD pin */
#if TMBSL_ERR_HDMI_NULL_CONNECTION != \
(TMBSL_ERR_HDMI_BASE + TM_ERR_NULL_CONNECTION)
#error
#endif

/** Not allowed in DVI mode */
#if TMBSL_ERR_HDMI_OPERATION_NOT_PERMITTED != \
(TMBSL_ERR_HDMI_BASE + TM_ERR_OPERATION_NOT_PERMITTED)
#error
#endif
/*@}*/

/**
 * A macro to check a condition and if true return a result
 */
#define RETIF(cond, rslt)       if ((cond)){return (rslt);}

/**
 * A macro to check a condition and if true return 
 * TMBSL_ERR_HDMI_BAD_PARAMETER.
 * To save code space, it can be compiled out by defining NO_RETIF_BADPARAM on
 * the compiler command line.
 */
#ifdef NO_RETIF_BADPARAM
#define RETIF_BADPARAM(cond)
#else
#define RETIF_BADPARAM(cond)  if ((cond)){return TMBSL_ERR_HDMI_BAD_PARAMETER;}
#endif

/**
 * A macro to check the result of a register API and if not TM_OK to return it.
 * To save code space, it can be compiled out by defining NO_RETIF_REG_FAIL on
 * the compiler command line.
 */
#ifdef NO_RETIF_REG_FAIL
#define RETIF_REG_FAIL(result)
#else
#define RETIF_REG_FAIL(result)  if ((result) != TM_OK){return (result);}
#endif

/*============================================================================*/
/*                       ENUM OR TYPE DEFINITIONS                             */
/*============================================================================*/

/**
 * Driver events and states used for diagnosis
 */
typedef enum _tmbslHdmiTxEvent
{
    EV_DEINIT               = 0,
    EV_UNPLUGGED            = 1,
    EV_PLUGGEDIN            = 2,
    EV_STANDBY              = 3,
    EV_RESUME_UNPLUGGED     = 4,
    EV_RESUME_PLUGGEDIN     = 5,
    EV_GETBLOCKDATA         = 6,
    EV_SETINOUT             = 7,
    EV_OUTDISABLE           = 8,
    EV_HDCP_RUN             = 9,
    EV_HDCP_BKSV_NREPEAT    = 10,
    EV_HDCP_BKSV_NSECURE    = 11,
    EV_HDCP_BKSV_REPEAT     = 12,
    EV_HDCP_BSTATUS_GOOD    = 13,
    EV_HDCP_KSV_SECURE      = 14,
    EV_HDCP_T0              = 15,
    EV_HDCP_STOP            = 16,
#if defined (TMFL_TDA9981_SUPPORT) && defined(TMFL_RX_SENSE_ON)
    EV_SINKON               = 17,
    EV_SINKOFF              = 18,
    EV_INVALID              = 19
#else /* TMFL_TDA9981_SUPPORT && TMFL_RX_SENSE_ON */
    EV_INVALID              = 17
#endif /* TMFL_TDA9981_SUPPORT && TMFL_RX_SENSE_ON */
} tmbslHdmiTxEvent_t;

typedef enum _tmbslHdmiTxState
{
    ST_UNINITIALIZED        = 0,
    ST_DISCONNECTED         = 1,
    ST_AWAIT_EDID           = 2,
    ST_SINK_CONNECTED       = 3,
    ST_VIDEO_NO_HDCP        = 4,
    ST_STANDBY              = 5,
    ST_HDCP_WAIT_RX         = 6,
    ST_HDCP_WAIT_BSTATUS    = 7,
    ST_HDCP_WAIT_SHA_1      = 8,
    ST_HDCP_AUTHENTICATED   = 9,
#if defined (TMFL_TDA9981_SUPPORT) && defined(TMFL_RX_SENSE_ON)
    ST_AWAIT_RX_SENSE       = 10,
    ST_INVALID              = 11,
    ST_NUM                  = 11
#else /* TMFL_TDA9981_SUPPORT && TMFL_RX_SENSE_ON */
    ST_INVALID              = 10,
    ST_NUM                  = 10
#endif /* TMFL_TDA9981_SUPPORT && TMFL_RX_SENSE_ON */
} tmbslHdmiTxState_t;

/**
 * An enum to index into the Device Instance Data shadowReg array
 */
enum _eShad
{
#ifdef TMFL_TDA9981_SUPPORT
    E_SP00_MAIN_CNTRL0  = 0,
    E_SP00_INT_FLAGS_0  = 1,
    E_SP00_INT_FLAGS_1  = 2,
    E_SP00_INT_FLAGS_2  = 3,
    E_SP00_VIP_CNTRL_0  = 4,
    E_SP00_VIP_CNTRL_1  = 5,
    E_SP00_VIP_CNTRL_2  = 6,
    E_SP00_VIP_CNTRL_3  = 7,
    E_SP00_VIP_CNTRL_4  = 8,
    E_SP00_VIP_CNTRL_5  = 9,
    E_SP00_MAT_CONTRL   = 10,
    E_SP00_TBG_CNTRL_0  = 11,
    E_SP00_TBG_CNTRL_1  = 12,
    E_SP00_HVF_CNTRL_0  = 13,
    E_SP00_HVF_CNTRL_1  = 14,
    E_SP00_TIMER_H      = 15,
    E_SP00_DEBUG_PROBE  = 16,
    E_SP00_AIP_CLKSEL   = 17,
#ifndef NO_HDCP
    E_SP12_HDCP_CTRL    = 18,
#ifdef BCAPS_REPEATER
    E_SP12_HDCP_BCAPS   = 19,
    E_SNUM              = 20,   /* Number of shadow registers */
    E_SNONE             = 20    /* Index value indicating no shadow register */
#else
    E_SNUM              = 19,   /* Number of shadow registers */
    E_SNONE             = 19    /* Index value indicating no shadow register */
#endif /* BCAPS_REPEATER */
#else
    E_SNUM              = 18,   /* Number of shadow registers */
    E_SNONE             = 18    /* Index value indicating no shadow register */
#endif /* NO_HDCP */
#else
    E_SP00_MAIN_CNTRL0  = 0,
    E_SP00_INT_FLAGS_0  = 1,
    E_SP00_INT_FLAGS_1  = 2,
    E_SP00_VIP_CNTRL_0  = 3,
    E_SP00_VIP_CNTRL_1  = 4,
    E_SP00_VIP_CNTRL_2  = 5,
    E_SP00_VIP_CNTRL_3  = 6,
    E_SP00_VIP_CNTRL_4  = 7,
    E_SP00_VIP_CNTRL_5  = 8,
    E_SP00_MAT_CONTRL   = 9,
    E_SP00_TBG_CNTRL_0  = 10,
    E_SP00_TBG_CNTRL_1  = 11,
    E_SP00_HVF_CNTRL_0  = 12,
    E_SP00_HVF_CNTRL_1  = 13,
    E_SP00_TIMER_H      = 14,
    E_SP00_DEBUG_PROBE  = 15,
    E_SP00_AIP_CLKSEL   = 16,
    E_SP01_SC_VIDFORMAT = 17,
    E_SP01_SC_CNTRL     = 18,
    E_SP01_TBG_CNTRL_0  = 19,
#ifndef NO_HDCP
    E_SP12_HDCP_CTRL    = 20,
#ifdef BCAPS_REPEATER
    E_SP12_HDCP_BCAPS   = 21,
    E_SNUM              = 22,   /* Number of shadow registers */
    E_SNONE             = 22    /* Index value indicating no shadow register */
#else
    E_SNUM              = 21,   /* Number of shadow registers */
    E_SNONE             = 21    /* Index value indicating no shadow register */
#endif /* BCAPS_REPEATER */
#else
    E_SNUM              = 20,   /* Number of shadow registers */
    E_SNONE             = 20    /* Index value indicating no shadow register */
#endif /* NO_HDCP */
#endif /* TMFL_TDA9981_SUPPORT */
};

/**
 * Page list
 * These are indexes to the allowed register page numbers
 */
enum _ePage
{
    E_PAGE_00      = 0,
    E_PAGE_01      = 1,
    E_PAGE_02      = 2,
    E_PAGE_10      = 3,         /* New for N4 */
    E_PAGE_11      = 4,
    E_PAGE_12      = 5,
    E_PAGE_NUM     = 6,         /* Number of pages */
    E_PAGE_INVALID = 6          /* Index value indicating invalid page */
};

/**
 * Macros to initialize and access the following register list enum _eReg
 */
/* Pack shadow index s, page index p and register address a into UInt16 */
#define SPA(s,p,a)       (UInt16)(((s)<<11)|((p)<<8)|(a))
/* Unpacks shadow index s from UInt16 */
#define SPA2SHAD(spa)    (UInt8)((spa)>>11)
/* Unpacks page index p from UInt16 */
#define SPA2PAGE(spa)    (UInt8)(((spa)>>8)&0x0007)
/* Unpacks register address a from UInt16 */
#define SPA2ADDR(spa)    (UInt8)((spa)&0x00FF)

/**
 * Register list
 *
 * Each register symbol has these fields: E_REG_page_register_access
 *
 * The symbols have a 16-bit value as follows, including an index to
 * the Device Instance Data shadowReg[] array:
 *
 * +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
 * |15 |14 |13 |12 |11 |10 | 9 | 8 | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |
 * +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
 * |  Shadow Index     |Page Index |       Register Address        |
 * +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
 *
 */
enum _eReg
{
    /*************************************************************************/
    /** Rows formatted in "HDMI Driver - Register List.xls" and pasted here **/
    /*************************************************************************/
    E_REG_MIN_ADR                   = 0x00, /* First register on all pages   */
    E_REG_CURPAGE_ADR_W             = 0xFF, /* Address register on all pages */
                                            
    E_REG_P00_VERSION_R             = SPA(E_SNONE            , E_PAGE_00, 0x00),
    E_REG_P00_MAIN_CNTRL0_W         = SPA(E_SP00_MAIN_CNTRL0 , E_PAGE_00, 0x01),
#ifdef TMFL_TDA9981_SUPPORT
    E_REG_P00_SR_REG_W              = SPA(E_SNONE            , E_PAGE_00, 0x0A),
    E_REG_P00_DDC_DISABLE_RW        = SPA(E_SNONE            , E_PAGE_00, 0x0B),
    E_REG_P00_CCLK_ON_RW            = SPA(E_SNONE            , E_PAGE_00, 0x0C),
#endif /* TMFL_TDA9981_SUPPORT */
    E_REG_P00_INT_FLAGS_0_RW        = SPA(E_SP00_INT_FLAGS_0 , E_PAGE_00, 0x0F),
    E_REG_P00_INT_FLAGS_1_RW        = SPA(E_SP00_INT_FLAGS_1 , E_PAGE_00, 0x10),
#ifdef TMFL_TDA9981_SUPPORT
    E_REG_P00_INT_FLAGS_2_RW        = SPA(E_SP00_INT_FLAGS_2 , E_PAGE_00, 0x11),
    E_REG_P00_INT_FLAGS_3_R         = SPA(E_SNONE            , E_PAGE_00, 0x12),
    E_REG_P00_SW_INT_W              = SPA(E_SNONE            , E_PAGE_00, 0x15),
    E_REG_P00_ENA_VP_0_RW           = SPA(E_SNONE            , E_PAGE_00, 0x18),
    E_REG_P00_ENA_VP_1_RW           = SPA(E_SNONE            , E_PAGE_00, 0x19),
    E_REG_P00_ENA_VP_2_RW           = SPA(E_SNONE            , E_PAGE_00, 0x1A),
    E_REG_P00_GND_VP_0_RW           = SPA(E_SNONE            , E_PAGE_00, 0x1B),
    E_REG_P00_GND_VP_1_RW           = SPA(E_SNONE            , E_PAGE_00, 0x1C),
    E_REG_P00_GND_VP_2_RW           = SPA(E_SNONE            , E_PAGE_00, 0x1D),
    E_REG_P00_ENA_AP_RW             = SPA(E_SNONE            , E_PAGE_00, 0x1E),
    E_REG_P00_GND_AP_RW             = SPA(E_SNONE            , E_PAGE_00, 0x1F),
#endif /* TMFL_TDA9981_SUPPORT */
    E_REG_P00_VIP_CNTRL_0_W         = SPA(E_SP00_VIP_CNTRL_0 , E_PAGE_00, 0x20),
    E_REG_P00_VIP_CNTRL_1_W         = SPA(E_SP00_VIP_CNTRL_1 , E_PAGE_00, 0x21),
    E_REG_P00_VIP_CNTRL_2_W         = SPA(E_SP00_VIP_CNTRL_2 , E_PAGE_00, 0x22),
    E_REG_P00_VIP_CNTRL_3_W         = SPA(E_SP00_VIP_CNTRL_3 , E_PAGE_00, 0x23),
    E_REG_P00_VIP_CNTRL_4_W         = SPA(E_SP00_VIP_CNTRL_4 , E_PAGE_00, 0x24),
    E_REG_P00_VIP_CNTRL_5_W         = SPA(E_SP00_VIP_CNTRL_5 , E_PAGE_00, 0x25),
    E_REG_P00_MAT_CONTRL_W          = SPA(E_SP00_MAT_CONTRL  , E_PAGE_00, 0x80),
    E_REG_P00_MAT_OI1_MSB_W         = SPA(E_SNONE            , E_PAGE_00, 0x81),
    E_REG_P00_MAT_OI1_LSB_W         = SPA(E_SNONE            , E_PAGE_00, 0x82),
    E_REG_P00_MAT_OI2_MSB_W         = SPA(E_SNONE            , E_PAGE_00, 0x83),
    E_REG_P00_MAT_OI2_LSB_W         = SPA(E_SNONE            , E_PAGE_00, 0x84),
    E_REG_P00_MAT_OI3_MSB_W         = SPA(E_SNONE            , E_PAGE_00, 0x85),
    E_REG_P00_MAT_OI3_LSB_W         = SPA(E_SNONE            , E_PAGE_00, 0x86),
    E_REG_P00_MAT_P11_MSB_W         = SPA(E_SNONE            , E_PAGE_00, 0x87),
    E_REG_P00_MAT_P11_LSB_W         = SPA(E_SNONE            , E_PAGE_00, 0x88),
    E_REG_P00_MAT_P12_MSB_W         = SPA(E_SNONE            , E_PAGE_00, 0x89),
    E_REG_P00_MAT_P12_LSB_W         = SPA(E_SNONE            , E_PAGE_00, 0x8A),
    E_REG_P00_MAT_P13_MSB_W         = SPA(E_SNONE            , E_PAGE_00, 0x8B),
    E_REG_P00_MAT_P13_LSB_W         = SPA(E_SNONE            , E_PAGE_00, 0x8C),
    E_REG_P00_MAT_P21_MSB_W         = SPA(E_SNONE            , E_PAGE_00, 0x8D),
    E_REG_P00_MAT_P21_LSB_W         = SPA(E_SNONE            , E_PAGE_00, 0x8E),
    E_REG_P00_MAT_P22_MSB_W         = SPA(E_SNONE            , E_PAGE_00, 0x8F),
    E_REG_P00_MAT_P22_LSB_W         = SPA(E_SNONE            , E_PAGE_00, 0x90),
    E_REG_P00_MAT_P23_MSB_W         = SPA(E_SNONE            , E_PAGE_00, 0x91),
    E_REG_P00_MAT_P23_LSB_W         = SPA(E_SNONE            , E_PAGE_00, 0x92),
    E_REG_P00_MAT_P31_MSB_W         = SPA(E_SNONE            , E_PAGE_00, 0x93),
    E_REG_P00_MAT_P31_LSB_W         = SPA(E_SNONE            , E_PAGE_00, 0x94),
    E_REG_P00_MAT_P32_MSB_W         = SPA(E_SNONE            , E_PAGE_00, 0x95),
    E_REG_P00_MAT_P32_LSB_W         = SPA(E_SNONE            , E_PAGE_00, 0x96),
    E_REG_P00_MAT_P33_MSB_W         = SPA(E_SNONE            , E_PAGE_00, 0x97),
    E_REG_P00_MAT_P33_LSB_W         = SPA(E_SNONE            , E_PAGE_00, 0x98),
    E_REG_P00_MAT_OO1_MSB_W         = SPA(E_SNONE            , E_PAGE_00, 0x99),
    E_REG_P00_MAT_OO1_LSB_W         = SPA(E_SNONE            , E_PAGE_00, 0x9A),
    E_REG_P00_MAT_OO2_MSB_W         = SPA(E_SNONE            , E_PAGE_00, 0x9B),
    E_REG_P00_MAT_OO2_LSB_W         = SPA(E_SNONE            , E_PAGE_00, 0x9C),
    E_REG_P00_MAT_OO3_MSB_W         = SPA(E_SNONE            , E_PAGE_00, 0x9D),
    E_REG_P00_MAT_OO3_LSB_W         = SPA(E_SNONE            , E_PAGE_00, 0x9E),
    E_REG_P00_VIDFORMAT_W           = SPA(E_SNONE            , E_PAGE_00, 0xA0),
    E_REG_P00_REFPIX_MSB_W          = SPA(E_SNONE            , E_PAGE_00, 0xA1),
    E_REG_P00_REFPIX_LSB_W          = SPA(E_SNONE            , E_PAGE_00, 0xA2),
    E_REG_P00_REFLINE_MSB_W         = SPA(E_SNONE            , E_PAGE_00, 0xA3),
    E_REG_P00_REFLINE_LSB_W         = SPA(E_SNONE            , E_PAGE_00, 0xA4),
    E_REG_P00_NPIX_MSB_W            = SPA(E_SNONE            , E_PAGE_00, 0xA5),
    E_REG_P00_NPIX_LSB_W            = SPA(E_SNONE            , E_PAGE_00, 0xA6),
    E_REG_P00_NLINE_MSB_W           = SPA(E_SNONE            , E_PAGE_00, 0xA7),
    E_REG_P00_NLINE_LSB_W           = SPA(E_SNONE            , E_PAGE_00, 0xA8),
    E_REG_P00_VS_LINE_STRT_1_MSB_W  = SPA(E_SNONE            , E_PAGE_00, 0xA9),
    E_REG_P00_VS_LINE_STRT_1_LSB_W  = SPA(E_SNONE            , E_PAGE_00, 0xAA),
    E_REG_P00_VS_PIX_STRT_1_MSB_W   = SPA(E_SNONE            , E_PAGE_00, 0xAB),
    E_REG_P00_VS_PIX_STRT_1_LSB_W   = SPA(E_SNONE            , E_PAGE_00, 0xAC),
    E_REG_P00_VS_LINE_END_1_MSB_W   = SPA(E_SNONE            , E_PAGE_00, 0xAD),
    E_REG_P00_VS_LINE_END_1_LSB_W   = SPA(E_SNONE            , E_PAGE_00, 0xAE),
    E_REG_P00_VS_PIX_END_1_MSB_W    = SPA(E_SNONE            , E_PAGE_00, 0xAF),
    E_REG_P00_VS_PIX_END_1_LSB_W    = SPA(E_SNONE            , E_PAGE_00, 0xB0),
    E_REG_P00_VS_LINE_STRT_2_MSB_W  = SPA(E_SNONE            , E_PAGE_00, 0xB1),
    E_REG_P00_VS_LINE_STRT_2_LSB_W  = SPA(E_SNONE            , E_PAGE_00, 0xB2),
    E_REG_P00_VS_PIX_STRT_2_MSB_W   = SPA(E_SNONE            , E_PAGE_00, 0xB3),
    E_REG_P00_VS_PIX_STRT_2_LSB_W   = SPA(E_SNONE            , E_PAGE_00, 0xB4),
    E_REG_P00_VS_LINE_END_2_MSB_W   = SPA(E_SNONE            , E_PAGE_00, 0xB5),
    E_REG_P00_VS_LINE_END_2_LSB_W   = SPA(E_SNONE            , E_PAGE_00, 0xB6),
    E_REG_P00_VS_PIX_END_2_MSB_W    = SPA(E_SNONE            , E_PAGE_00, 0xB7),
    E_REG_P00_VS_PIX_END_2_LSB_W    = SPA(E_SNONE            , E_PAGE_00, 0xB8),
    E_REG_P00_HS_PIX_START_MSB_W    = SPA(E_SNONE            , E_PAGE_00, 0xB9),
    E_REG_P00_HS_PIX_START_LSB_W    = SPA(E_SNONE            , E_PAGE_00, 0xBA),
    E_REG_P00_HS_PIX_STOP_MSB_W     = SPA(E_SNONE            , E_PAGE_00, 0xBB),
    E_REG_P00_HS_PIX_STOP_LSB_W     = SPA(E_SNONE            , E_PAGE_00, 0xBC),
    E_REG_P00_VWIN_START_1_MSB_W    = SPA(E_SNONE            , E_PAGE_00, 0xBD),
    E_REG_P00_VWIN_START_1_LSB_W    = SPA(E_SNONE            , E_PAGE_00, 0xBE),
    E_REG_P00_VWIN_END_1_MSB_W      = SPA(E_SNONE            , E_PAGE_00, 0xBF),
    E_REG_P00_VWIN_END_1_LSB_W      = SPA(E_SNONE            , E_PAGE_00, 0xC0),
    E_REG_P00_VWIN_START_2_MSB_W    = SPA(E_SNONE            , E_PAGE_00, 0xC1),
    E_REG_P00_VWIN_START_2_LSB_W    = SPA(E_SNONE            , E_PAGE_00, 0xC2),
    E_REG_P00_VWIN_END_2_MSB_W      = SPA(E_SNONE            , E_PAGE_00, 0xC3),
    E_REG_P00_VWIN_END_2_LSB_W      = SPA(E_SNONE            , E_PAGE_00, 0xC4),
    E_REG_P00_DE_START_MSB_W        = SPA(E_SNONE            , E_PAGE_00, 0xC5),
    E_REG_P00_DE_START_LSB_W        = SPA(E_SNONE            , E_PAGE_00, 0xC6),
    E_REG_P00_DE_STOP_MSB_W         = SPA(E_SNONE            , E_PAGE_00, 0xC7),
    E_REG_P00_DE_STOP_LSB_W         = SPA(E_SNONE            , E_PAGE_00, 0xC8),
    E_REG_P00_COLBAR_WIDTH_W        = SPA(E_SNONE            , E_PAGE_00, 0xC9),
    E_REG_P00_TBG_CNTRL_0_W         = SPA(E_SP00_TBG_CNTRL_0 , E_PAGE_00, 0xCA),
    E_REG_P00_TBG_CNTRL_1_W         = SPA(E_SP00_TBG_CNTRL_1 , E_PAGE_00, 0xCB),
    E_REG_P00_VBL_OFFSET_START_W    = SPA(E_SNONE            , E_PAGE_00, 0xCC),
    E_REG_P00_VBL_OFFSET_END_W      = SPA(E_SNONE            , E_PAGE_00, 0xCD),
    E_REG_P00_HBL_OFFSET_START_W    = SPA(E_SNONE            , E_PAGE_00, 0xCE),
    E_REG_P00_HBL_OFFSET_END_W      = SPA(E_SNONE            , E_PAGE_00, 0xCF),
    E_REG_P00_DWIN_RE_DE_W          = SPA(E_SNONE            , E_PAGE_00, 0xD0),
    E_REG_P00_DWIN_FE_DE_W          = SPA(E_SNONE            , E_PAGE_00, 0xD1),
    E_REG_P00_HVF_CNTRL_0_W         = SPA(E_SP00_HVF_CNTRL_0 , E_PAGE_00, 0xE4),
    E_REG_P00_HVF_CNTRL_1_W         = SPA(E_SP00_HVF_CNTRL_1 , E_PAGE_00, 0xE5),
    E_REG_P00_TIMER_H_W             = SPA(E_SP00_TIMER_H     , E_PAGE_00, 0xE8),
    E_REG_P00_TIMER_M_W             = SPA(E_SNONE            , E_PAGE_00, 0xE9),
    E_REG_P00_TIMER_L_W             = SPA(E_SNONE            , E_PAGE_00, 0xEA),
#ifdef TMFL_TDA9981_SUPPORT
    E_REG_P00_TIMER_2SEC_W          = SPA(E_SNONE            , E_PAGE_00, 0xEB),
    E_REG_P00_TIMER_5SEC_W          = SPA(E_SNONE            , E_PAGE_00, 0xEC),
#endif /* TMFL_TDA9981_SUPPORT */
    E_REG_P00_NDIV_IM_W             = SPA(E_SNONE            , E_PAGE_00, 0xEE),
    E_REG_P00_NDIV_PF_W             = SPA(E_SNONE            , E_PAGE_00, 0xEF),
    E_REG_P00_RPT_CNTRL_W           = SPA(E_SNONE            , E_PAGE_00, 0xF0),
    E_REG_P00_LEAD_OFF_W            = SPA(E_SNONE            , E_PAGE_00, 0xF1),
    E_REG_P00_TRAIL_OFF_W           = SPA(E_SNONE            , E_PAGE_00, 0xF2),
    E_REG_P00_DEBUG_PROBE_W         = SPA(E_SP00_DEBUG_PROBE , E_PAGE_00, 0xF8),
    E_REG_P00_GHOST_XADDR_W         = SPA(E_SNONE            , E_PAGE_00, 0xF9),
    E_REG_P00_AIP_CLKSEL_W          = SPA(E_SP00_AIP_CLKSEL  , E_PAGE_00, 0xFD),
    E_REG_P00_GHOST_ADDR_W          = SPA(E_SNONE            , E_PAGE_00, 0xFE),
#ifndef TMFL_TDA9981_SUPPORT
	E_REG_P01_SC_VIDFORMAT_W        = SPA(E_SP01_SC_VIDFORMAT, E_PAGE_01, 0x00),
    E_REG_P01_SC_CNTRL_W            = SPA(E_SP01_SC_CNTRL    , E_PAGE_01, 0x01),
    E_REG_P01_SC_DELTA_PHASE_V_W    = SPA(E_SNONE            , E_PAGE_01, 0x02),
    E_REG_P01_SC_DELTA_PHASE_H_W    = SPA(E_SNONE            , E_PAGE_01, 0x03),
    E_REG_P01_SC_START_PHASE_H_W    = SPA(E_SNONE            , E_PAGE_01, 0x04),
    E_REG_P01_SC_NPIX_IN_LSB_W      = SPA(E_SNONE            , E_PAGE_01, 0x05),
    E_REG_P01_SC_NPIX_IN_MSB_W      = SPA(E_SNONE            , E_PAGE_01, 0x06),
    E_REG_P01_SC_NPIX_OUT_LSB_W     = SPA(E_SNONE            , E_PAGE_01, 0x07),
    E_REG_P01_SC_NPIX_OUT_MSB_W     = SPA(E_SNONE            , E_PAGE_01, 0x08),
    E_REG_P01_SC_NLINE_IN_LSB_W     = SPA(E_SNONE            , E_PAGE_01, 0x09),
    E_REG_P01_SC_NLINE_IN_MSB_W     = SPA(E_SNONE            , E_PAGE_01, 0x0A),
    E_REG_P01_SC_NLINE_OUT_LSB_W    = SPA(E_SNONE            , E_PAGE_01, 0x0B),
    E_REG_P01_SC_NLINE_OUT_MSB_W    = SPA(E_SNONE            , E_PAGE_01, 0x0C),
    E_REG_P01_SC_NLINE_SKIP_W       = SPA(E_SNONE            , E_PAGE_01, 0x0D),
    E_REG_P01_SC_SAMPLE_BUFFILL_R   = SPA(E_SNONE            , E_PAGE_01, 0x0E),
    E_REG_P01_SC_MAX_BUFFILL_P_0_R  = SPA(E_SNONE            , E_PAGE_01, 0x0F),
    E_REG_P01_SC_MAX_BUFFILL_P_1_R  = SPA(E_SNONE            , E_PAGE_01, 0x10),
    E_REG_P01_SC_MAX_BUFFILL_D_0_R  = SPA(E_SNONE            , E_PAGE_01, 0x11),
    E_REG_P01_SC_MAX_BUFFILL_D_1_R  = SPA(E_SNONE            , E_PAGE_01, 0x12),
    E_REG_P01_SC_SAMPLE_FIFOFILL_R  = SPA(E_SNONE            , E_PAGE_01, 0x13),
    E_REG_P01_SC_MAX_FIFOFILL_PI_R  = SPA(E_SNONE            , E_PAGE_01, 0x14),
    E_REG_P01_SC_MIN_FIFOFILL_PO1_R = SPA(E_SNONE            , E_PAGE_01, 0x15),
    E_REG_P01_SC_MIN_FIFOFILL_PO2_R = SPA(E_SNONE            , E_PAGE_01, 0x16),
    E_REG_P01_SC_MIN_FIFOFILL_PO3_R = SPA(E_SNONE            , E_PAGE_01, 0x17),
    E_REG_P01_SC_MIN_FIFOFILL_PO4_R = SPA(E_SNONE            , E_PAGE_01, 0x18),
    E_REG_P01_SC_MAX_FIFOFILL_DI_R  = SPA(E_SNONE            , E_PAGE_01, 0x19),
    E_REG_P01_SC_MAX_FIFOFILL_DO_R  = SPA(E_SNONE            , E_PAGE_01, 0x1A),
    E_REG_P01_SC_VS_LUT_0_W         = SPA(E_SNONE            , E_PAGE_01, 0x1B),
    E_REG_P01_SC_VS_LUT_1_W         = SPA(E_SNONE            , E_PAGE_01, 0x1C),
    E_REG_P01_SC_VS_LUT_2_W         = SPA(E_SNONE            , E_PAGE_01, 0x1D),
    E_REG_P01_SC_VS_LUT_3_W         = SPA(E_SNONE            , E_PAGE_01, 0x1E),
    E_REG_P01_SC_VS_LUT_4_W         = SPA(E_SNONE            , E_PAGE_01, 0x1F),
    E_REG_P01_SC_VS_LUT_5_W         = SPA(E_SNONE            , E_PAGE_01, 0x20),
    E_REG_P01_SC_VS_LUT_6_W         = SPA(E_SNONE            , E_PAGE_01, 0x21),
    E_REG_P01_SC_VS_LUT_7_W         = SPA(E_SNONE            , E_PAGE_01, 0x22),
    E_REG_P01_SC_VS_LUT_8_W         = SPA(E_SNONE            , E_PAGE_01, 0x23),
    E_REG_P01_SC_VS_LUT_9_W         = SPA(E_SNONE            , E_PAGE_01, 0x24),
    E_REG_P01_SC_VS_LUT_10_W        = SPA(E_SNONE            , E_PAGE_01, 0x25),
    E_REG_P01_SC_VS_LUT_11_W        = SPA(E_SNONE            , E_PAGE_01, 0x26),
    E_REG_P01_SC_VS_LUT_12_W        = SPA(E_SNONE            , E_PAGE_01, 0x27),
    E_REG_P01_SC_VS_LUT_13_W        = SPA(E_SNONE            , E_PAGE_01, 0x28),
    E_REG_P01_SC_VS_LUT_14_W        = SPA(E_SNONE            , E_PAGE_01, 0x29),
    E_REG_P01_SC_VS_LUT_15_W        = SPA(E_SNONE            , E_PAGE_01, 0x2A),
    E_REG_P01_SC_VS_LUT_16_W        = SPA(E_SNONE            , E_PAGE_01, 0x2B),
    E_REG_P01_SC_VS_LUT_17_W        = SPA(E_SNONE            , E_PAGE_01, 0x2C),
    E_REG_P01_SC_VS_LUT_18_W        = SPA(E_SNONE            , E_PAGE_01, 0x2D),
    E_REG_P01_SC_VS_LUT_19_W        = SPA(E_SNONE            , E_PAGE_01, 0x2E),
    E_REG_P01_SC_VS_LUT_20_W        = SPA(E_SNONE            , E_PAGE_01, 0x2F),
    E_REG_P01_SC_VS_LUT_21_W        = SPA(E_SNONE            , E_PAGE_01, 0x30),
    E_REG_P01_SC_VS_LUT_22_W        = SPA(E_SNONE            , E_PAGE_01, 0x31),
    E_REG_P01_SC_VS_LUT_23_W        = SPA(E_SNONE            , E_PAGE_01, 0x32),
    E_REG_P01_SC_VS_LUT_24_W        = SPA(E_SNONE            , E_PAGE_01, 0x33),
    E_REG_P01_SC_VS_LUT_25_W        = SPA(E_SNONE            , E_PAGE_01, 0x34),
    E_REG_P01_SC_VS_LUT_26_W        = SPA(E_SNONE            , E_PAGE_01, 0x35),
    E_REG_P01_SC_VS_LUT_27_W        = SPA(E_SNONE            , E_PAGE_01, 0x36),
    E_REG_P01_SC_VS_LUT_28_W        = SPA(E_SNONE            , E_PAGE_01, 0x37),
    E_REG_P01_SC_VS_LUT_29_W        = SPA(E_SNONE            , E_PAGE_01, 0x38),
    E_REG_P01_SC_VS_LUT_30_W        = SPA(E_SNONE            , E_PAGE_01, 0x39),
    E_REG_P01_SC_VS_LUT_31_W        = SPA(E_SNONE            , E_PAGE_01, 0x3A),
    E_REG_P01_SC_VS_LUT_32_W        = SPA(E_SNONE            , E_PAGE_01, 0x3B),
    E_REG_P01_SC_VS_LUT_33_W        = SPA(E_SNONE            , E_PAGE_01, 0x3C),
    E_REG_P01_SC_VS_LUT_34_W        = SPA(E_SNONE            , E_PAGE_01, 0x3D),
    E_REG_P01_SC_VS_LUT_35_W        = SPA(E_SNONE            , E_PAGE_01, 0x3E),
    E_REG_P01_SC_VS_LUT_36_W        = SPA(E_SNONE            , E_PAGE_01, 0x3F),
    E_REG_P01_SC_VS_LUT_37_W        = SPA(E_SNONE            , E_PAGE_01, 0x40),
    E_REG_P01_SC_VS_LUT_38_W        = SPA(E_SNONE            , E_PAGE_01, 0x41),
    E_REG_P01_SC_VS_LUT_39_W        = SPA(E_SNONE            , E_PAGE_01, 0x42),
    E_REG_P01_SC_VS_LUT_40_W        = SPA(E_SNONE            , E_PAGE_01, 0x43),
    E_REG_P01_SC_VS_LUT_41_W        = SPA(E_SNONE            , E_PAGE_01, 0x44),
    E_REG_P01_SC_VS_LUT_42_W        = SPA(E_SNONE            , E_PAGE_01, 0x45),
    E_REG_P01_SC_VS_LUT_43_W        = SPA(E_SNONE            , E_PAGE_01, 0x46),
    E_REG_P01_SC_VS_LUT_44_W        = SPA(E_SNONE            , E_PAGE_01, 0x47),
    E_REG_P01_VIDFORMAT_W           = SPA(E_SNONE            , E_PAGE_01, 0xA0),
    E_REG_P01_REFPIX_MSB_W          = SPA(E_SNONE            , E_PAGE_01, 0xA1),
    E_REG_P01_REFPIX_LSB_W          = SPA(E_SNONE            , E_PAGE_01, 0xA2),
    E_REG_P01_REFLINE_MSB_W         = SPA(E_SNONE            , E_PAGE_01, 0xA3),
    E_REG_P01_REFLINE_LSB_W         = SPA(E_SNONE            , E_PAGE_01, 0xA4),
    E_REG_P01_NPIX_MSB_W            = SPA(E_SNONE            , E_PAGE_01, 0xA5),
    E_REG_P01_NPIX_LSB_W            = SPA(E_SNONE            , E_PAGE_01, 0xA6),
    E_REG_P01_NLINE_MSB_W           = SPA(E_SNONE            , E_PAGE_01, 0xA7),
    E_REG_P01_NLINE_LSB_W           = SPA(E_SNONE            , E_PAGE_01, 0xA8),
    E_REG_P01_VWIN_START_1_MSB_W    = SPA(E_SNONE            , E_PAGE_01, 0xBD),
    E_REG_P01_VWIN_START_1_LSB_W    = SPA(E_SNONE            , E_PAGE_01, 0xBE),
    E_REG_P01_VWIN_END_1_MSB_W      = SPA(E_SNONE            , E_PAGE_01, 0xBF),
    E_REG_P01_VWIN_END_1_LSB_W      = SPA(E_SNONE            , E_PAGE_01, 0xC0),
    E_REG_P01_VWIN_START_2_MSB_W    = SPA(E_SNONE            , E_PAGE_01, 0xC1),
    E_REG_P01_VWIN_START_2_LSB_W    = SPA(E_SNONE            , E_PAGE_01, 0xC2),
    E_REG_P01_VWIN_END_2_MSB_W      = SPA(E_SNONE            , E_PAGE_01, 0xC3),
    E_REG_P01_VWIN_END_2_LSB_W      = SPA(E_SNONE            , E_PAGE_01, 0xC4),
    E_REG_P01_DE_START_MSB_W        = SPA(E_SNONE            , E_PAGE_01, 0xC5),
    E_REG_P01_DE_START_LSB_W        = SPA(E_SNONE            , E_PAGE_01, 0xC6),
    E_REG_P01_DE_STOP_MSB_W         = SPA(E_SNONE            , E_PAGE_01, 0xC7),
    E_REG_P01_DE_STOP_LSB_W         = SPA(E_SNONE            , E_PAGE_01, 0xC8),
    E_REG_P01_TBG_CNTRL_0_W         = SPA(E_SP01_TBG_CNTRL_0 , E_PAGE_01, 0xCA),
#endif /* TMFL_TDA9981_SUPPORT */
	E_REG_P02_PLL_SERIAL_1_RW       = SPA(E_SNONE            , E_PAGE_02, 0x00),
    E_REG_P02_PLL_SERIAL_2_RW       = SPA(E_SNONE            , E_PAGE_02, 0x01),
    E_REG_P02_PLL_SERIAL_3_RW       = SPA(E_SNONE            , E_PAGE_02, 0x02),
    E_REG_P02_SERIALIZER_RW         = SPA(E_SNONE            , E_PAGE_02, 0x03),
    E_REG_P02_BUFFER_OUT_RW         = SPA(E_SNONE            , E_PAGE_02, 0x04),
    E_REG_P02_PLL_SCG1_RW           = SPA(E_SNONE            , E_PAGE_02, 0x05),
    E_REG_P02_PLL_SCG2_RW           = SPA(E_SNONE            , E_PAGE_02, 0x06),
    E_REG_P02_PLL_SCGN1_RW          = SPA(E_SNONE            , E_PAGE_02, 0x07),
    E_REG_P02_PLL_SCGN2_RW          = SPA(E_SNONE            , E_PAGE_02, 0x08),
    E_REG_P02_PLL_SCGR1_RW          = SPA(E_SNONE            , E_PAGE_02, 0x09),
    E_REG_P02_PLL_SCGR2_RW          = SPA(E_SNONE            , E_PAGE_02, 0x0A),
    E_REG_P02_PLL_DE_RW             = SPA(E_SNONE            , E_PAGE_02, 0x0B),
    E_REG_P02_CCIR_DIV_RW           = SPA(E_SNONE            , E_PAGE_02, 0x0C),
    E_REG_P02_VAI_PLL_R             = SPA(E_SNONE            , E_PAGE_02, 0x0D),
    E_REG_P02_AUDIO_DIV_RW          = SPA(E_SNONE            , E_PAGE_02, 0x0E),
    E_REG_P02_TEST1_RW              = SPA(E_SNONE            , E_PAGE_02, 0x0F),
    E_REG_P02_TEST2_RW              = SPA(E_SNONE            , E_PAGE_02, 0x10),
    E_REG_P02_SEL_CLK_RW            = SPA(E_SNONE            , E_PAGE_02, 0x11),
    E_REG_P10_IF1_HB0_RW            = SPA(E_SNONE            , E_PAGE_10, 0x20),
    E_REG_P10_IF1_HB1_RW            = SPA(E_SNONE            , E_PAGE_10, 0x21),
    E_REG_P10_IF1_HB2_RW            = SPA(E_SNONE            , E_PAGE_10, 0x22),
    E_REG_P10_IF1_PB0_RW            = SPA(E_SNONE            , E_PAGE_10, 0x23),
    E_REG_P10_IF1_PB1_RW            = SPA(E_SNONE            , E_PAGE_10, 0x24),
    E_REG_P10_IF1_PB2_RW            = SPA(E_SNONE            , E_PAGE_10, 0x25),
    E_REG_P10_IF1_PB3_RW            = SPA(E_SNONE            , E_PAGE_10, 0x26),
    E_REG_P10_IF1_PB4_RW            = SPA(E_SNONE            , E_PAGE_10, 0x27),
    E_REG_P10_IF1_PB5_RW            = SPA(E_SNONE            , E_PAGE_10, 0x28),
    E_REG_P10_IF1_PB6_RW            = SPA(E_SNONE            , E_PAGE_10, 0x29),
    E_REG_P10_IF1_PB7_RW            = SPA(E_SNONE            , E_PAGE_10, 0x2A),
    E_REG_P10_IF1_PB8_RW            = SPA(E_SNONE            , E_PAGE_10, 0x2B),
    E_REG_P10_IF1_PB9_RW            = SPA(E_SNONE            , E_PAGE_10, 0x2C),
    E_REG_P10_IF1_PB10_RW           = SPA(E_SNONE            , E_PAGE_10, 0x2D),
    E_REG_P10_IF1_PB11_RW           = SPA(E_SNONE            , E_PAGE_10, 0x2E),
    E_REG_P10_IF1_PB12_RW           = SPA(E_SNONE            , E_PAGE_10, 0x2F),
    E_REG_P10_IF1_PB13_RW           = SPA(E_SNONE            , E_PAGE_10, 0x30),
    E_REG_P10_IF1_PB14_RW           = SPA(E_SNONE            , E_PAGE_10, 0x31),
    E_REG_P10_IF1_PB15_RW           = SPA(E_SNONE            , E_PAGE_10, 0x32),
    E_REG_P10_IF1_PB16_RW           = SPA(E_SNONE            , E_PAGE_10, 0x33),
    E_REG_P10_IF1_PB17_RW           = SPA(E_SNONE            , E_PAGE_10, 0x34),
    E_REG_P10_IF1_PB18_RW           = SPA(E_SNONE            , E_PAGE_10, 0x35),
    E_REG_P10_IF1_PB19_RW           = SPA(E_SNONE            , E_PAGE_10, 0x36),
    E_REG_P10_IF1_PB20_RW           = SPA(E_SNONE            , E_PAGE_10, 0x37),
    E_REG_P10_IF1_PB21_RW           = SPA(E_SNONE            , E_PAGE_10, 0x38),
    E_REG_P10_IF1_PB22_RW           = SPA(E_SNONE            , E_PAGE_10, 0x39),
    E_REG_P10_IF1_PB23_RW           = SPA(E_SNONE            , E_PAGE_10, 0x3A),
    E_REG_P10_IF1_PB24_RW           = SPA(E_SNONE            , E_PAGE_10, 0x3B),
    E_REG_P10_IF1_PB25_RW           = SPA(E_SNONE            , E_PAGE_10, 0x3C),
    E_REG_P10_IF1_PB26_RW           = SPA(E_SNONE            , E_PAGE_10, 0x3D),
    E_REG_P10_IF1_PB27_RW           = SPA(E_SNONE            , E_PAGE_10, 0x3E),
    E_REG_P10_IF2_HB0_RW            = SPA(E_SNONE            , E_PAGE_10, 0x40),
    E_REG_P10_IF2_HB1_RW            = SPA(E_SNONE            , E_PAGE_10, 0x41),
    E_REG_P10_IF2_HB2_RW            = SPA(E_SNONE            , E_PAGE_10, 0x42),
    E_REG_P10_IF2_PB0_RW            = SPA(E_SNONE            , E_PAGE_10, 0x43),
    E_REG_P10_IF2_PB1_RW            = SPA(E_SNONE            , E_PAGE_10, 0x44),
    E_REG_P10_IF2_PB2_RW            = SPA(E_SNONE            , E_PAGE_10, 0x45),
    E_REG_P10_IF2_PB3_RW            = SPA(E_SNONE            , E_PAGE_10, 0x46),
    E_REG_P10_IF2_PB4_RW            = SPA(E_SNONE            , E_PAGE_10, 0x47),
    E_REG_P10_IF2_PB5_RW            = SPA(E_SNONE            , E_PAGE_10, 0x48),
    E_REG_P10_IF2_PB6_RW            = SPA(E_SNONE            , E_PAGE_10, 0x49),
    E_REG_P10_IF2_PB7_RW            = SPA(E_SNONE            , E_PAGE_10, 0x4A),
    E_REG_P10_IF2_PB8_RW            = SPA(E_SNONE            , E_PAGE_10, 0x4B),
    E_REG_P10_IF2_PB9_RW            = SPA(E_SNONE            , E_PAGE_10, 0x4C),
    E_REG_P10_IF2_PB10_RW           = SPA(E_SNONE            , E_PAGE_10, 0x4D),
    E_REG_P10_IF2_PB11_RW           = SPA(E_SNONE            , E_PAGE_10, 0x4E),
    E_REG_P10_IF2_PB12_RW           = SPA(E_SNONE            , E_PAGE_10, 0x4F),
    E_REG_P10_IF2_PB13_RW           = SPA(E_SNONE            , E_PAGE_10, 0x50),
    E_REG_P10_IF2_PB14_RW           = SPA(E_SNONE            , E_PAGE_10, 0x51),
    E_REG_P10_IF2_PB15_RW           = SPA(E_SNONE            , E_PAGE_10, 0x52),
    E_REG_P10_IF2_PB16_RW           = SPA(E_SNONE            , E_PAGE_10, 0x53),
    E_REG_P10_IF2_PB17_RW           = SPA(E_SNONE            , E_PAGE_10, 0x54),
    E_REG_P10_IF2_PB18_RW           = SPA(E_SNONE            , E_PAGE_10, 0x55),
    E_REG_P10_IF2_PB19_RW           = SPA(E_SNONE            , E_PAGE_10, 0x56),
    E_REG_P10_IF2_PB20_RW           = SPA(E_SNONE            , E_PAGE_10, 0x57),
    E_REG_P10_IF2_PB21_RW           = SPA(E_SNONE            , E_PAGE_10, 0x58),
    E_REG_P10_IF2_PB22_RW           = SPA(E_SNONE            , E_PAGE_10, 0x59),
    E_REG_P10_IF2_PB23_RW           = SPA(E_SNONE            , E_PAGE_10, 0x5A),
    E_REG_P10_IF2_PB24_RW           = SPA(E_SNONE            , E_PAGE_10, 0x5B),
    E_REG_P10_IF2_PB25_RW           = SPA(E_SNONE            , E_PAGE_10, 0x5C),
    E_REG_P10_IF2_PB26_RW           = SPA(E_SNONE            , E_PAGE_10, 0x5D),
    E_REG_P10_IF2_PB27_RW           = SPA(E_SNONE            , E_PAGE_10, 0x5E),
    E_REG_P10_IF3_HB0_RW            = SPA(E_SNONE            , E_PAGE_10, 0x60),
    E_REG_P10_IF3_HB1_RW            = SPA(E_SNONE            , E_PAGE_10, 0x61),
    E_REG_P10_IF3_HB2_RW            = SPA(E_SNONE            , E_PAGE_10, 0x62),
    E_REG_P10_IF3_PB0_RW            = SPA(E_SNONE            , E_PAGE_10, 0x63),
    E_REG_P10_IF3_PB1_RW            = SPA(E_SNONE            , E_PAGE_10, 0x64),
    E_REG_P10_IF3_PB2_RW            = SPA(E_SNONE            , E_PAGE_10, 0x65),
    E_REG_P10_IF3_PB3_RW            = SPA(E_SNONE            , E_PAGE_10, 0x66),
    E_REG_P10_IF3_PB4_RW            = SPA(E_SNONE            , E_PAGE_10, 0x67),
    E_REG_P10_IF3_PB5_RW            = SPA(E_SNONE            , E_PAGE_10, 0x68),
    E_REG_P10_IF3_PB6_RW            = SPA(E_SNONE            , E_PAGE_10, 0x69),
    E_REG_P10_IF3_PB7_RW            = SPA(E_SNONE            , E_PAGE_10, 0x6A),
    E_REG_P10_IF3_PB8_RW            = SPA(E_SNONE            , E_PAGE_10, 0x6B),
    E_REG_P10_IF3_PB9_RW            = SPA(E_SNONE            , E_PAGE_10, 0x6C),
    E_REG_P10_IF3_PB10_RW           = SPA(E_SNONE            , E_PAGE_10, 0x6D),
    E_REG_P10_IF3_PB11_RW           = SPA(E_SNONE            , E_PAGE_10, 0x6E),
    E_REG_P10_IF3_PB12_RW           = SPA(E_SNONE            , E_PAGE_10, 0x6F),
    E_REG_P10_IF3_PB13_RW           = SPA(E_SNONE            , E_PAGE_10, 0x70),
    E_REG_P10_IF3_PB14_RW           = SPA(E_SNONE            , E_PAGE_10, 0x71),
    E_REG_P10_IF3_PB15_RW           = SPA(E_SNONE            , E_PAGE_10, 0x72),
    E_REG_P10_IF3_PB16_RW           = SPA(E_SNONE            , E_PAGE_10, 0x73),
    E_REG_P10_IF3_PB17_RW           = SPA(E_SNONE            , E_PAGE_10, 0x74),
    E_REG_P10_IF3_PB18_RW           = SPA(E_SNONE            , E_PAGE_10, 0x75),
    E_REG_P10_IF3_PB19_RW           = SPA(E_SNONE            , E_PAGE_10, 0x76),
    E_REG_P10_IF3_PB20_RW           = SPA(E_SNONE            , E_PAGE_10, 0x77),
    E_REG_P10_IF3_PB21_RW           = SPA(E_SNONE            , E_PAGE_10, 0x78),
    E_REG_P10_IF3_PB22_RW           = SPA(E_SNONE            , E_PAGE_10, 0x79),
    E_REG_P10_IF3_PB23_RW           = SPA(E_SNONE            , E_PAGE_10, 0x7A),
    E_REG_P10_IF3_PB24_RW           = SPA(E_SNONE            , E_PAGE_10, 0x7B),
    E_REG_P10_IF3_PB25_RW           = SPA(E_SNONE            , E_PAGE_10, 0x7C),
    E_REG_P10_IF3_PB26_RW           = SPA(E_SNONE            , E_PAGE_10, 0x7D),
    E_REG_P10_IF3_PB27_RW           = SPA(E_SNONE            , E_PAGE_10, 0x7E),
    E_REG_P10_IF4_HB0_RW            = SPA(E_SNONE            , E_PAGE_10, 0x80),
    E_REG_P10_IF4_HB1_RW            = SPA(E_SNONE            , E_PAGE_10, 0x81),
    E_REG_P10_IF4_HB2_RW            = SPA(E_SNONE            , E_PAGE_10, 0x82),
    E_REG_P10_IF4_PB0_RW            = SPA(E_SNONE            , E_PAGE_10, 0x83),
    E_REG_P10_IF4_PB1_RW            = SPA(E_SNONE            , E_PAGE_10, 0x84),
    E_REG_P10_IF4_PB2_RW            = SPA(E_SNONE            , E_PAGE_10, 0x85),
    E_REG_P10_IF4_PB3_RW            = SPA(E_SNONE            , E_PAGE_10, 0x86),
    E_REG_P10_IF4_PB4_RW            = SPA(E_SNONE            , E_PAGE_10, 0x87),
    E_REG_P10_IF4_PB5_RW            = SPA(E_SNONE            , E_PAGE_10, 0x88),
    E_REG_P10_IF4_PB6_RW            = SPA(E_SNONE            , E_PAGE_10, 0x89),
    E_REG_P10_IF4_PB7_RW            = SPA(E_SNONE            , E_PAGE_10, 0x8A),
    E_REG_P10_IF4_PB8_RW            = SPA(E_SNONE            , E_PAGE_10, 0x8B),
    E_REG_P10_IF4_PB9_RW            = SPA(E_SNONE            , E_PAGE_10, 0x8C),
    E_REG_P10_IF4_PB10_RW           = SPA(E_SNONE            , E_PAGE_10, 0x8D),
    E_REG_P10_IF4_PB11_RW           = SPA(E_SNONE            , E_PAGE_10, 0x8E),
    E_REG_P10_IF4_PB12_RW           = SPA(E_SNONE            , E_PAGE_10, 0x8F),
    E_REG_P10_IF4_PB13_RW           = SPA(E_SNONE            , E_PAGE_10, 0x90),
    E_REG_P10_IF4_PB14_RW           = SPA(E_SNONE            , E_PAGE_10, 0x91),
    E_REG_P10_IF4_PB15_RW           = SPA(E_SNONE            , E_PAGE_10, 0x92),
    E_REG_P10_IF4_PB16_RW           = SPA(E_SNONE            , E_PAGE_10, 0x93),
    E_REG_P10_IF4_PB17_RW           = SPA(E_SNONE            , E_PAGE_10, 0x94),
    E_REG_P10_IF4_PB18_RW           = SPA(E_SNONE            , E_PAGE_10, 0x95),
    E_REG_P10_IF4_PB19_RW           = SPA(E_SNONE            , E_PAGE_10, 0x96),
    E_REG_P10_IF4_PB20_RW           = SPA(E_SNONE            , E_PAGE_10, 0x97),
    E_REG_P10_IF4_PB21_RW           = SPA(E_SNONE            , E_PAGE_10, 0x98),
    E_REG_P10_IF4_PB22_RW           = SPA(E_SNONE            , E_PAGE_10, 0x99),
    E_REG_P10_IF4_PB23_RW           = SPA(E_SNONE            , E_PAGE_10, 0x9A),
    E_REG_P10_IF4_PB24_RW           = SPA(E_SNONE            , E_PAGE_10, 0x9B),
    E_REG_P10_IF4_PB25_RW           = SPA(E_SNONE            , E_PAGE_10, 0x9C),
    E_REG_P10_IF4_PB26_RW           = SPA(E_SNONE            , E_PAGE_10, 0x9D),
    E_REG_P10_IF4_PB27_RW           = SPA(E_SNONE            , E_PAGE_10, 0x9E),
    E_REG_P10_IF5_HB0_RW            = SPA(E_SNONE            , E_PAGE_10, 0xA0),
    E_REG_P10_IF5_HB1_RW            = SPA(E_SNONE            , E_PAGE_10, 0xA1),
    E_REG_P10_IF5_HB2_RW            = SPA(E_SNONE            , E_PAGE_10, 0xA2),
    E_REG_P10_IF5_PB0_RW            = SPA(E_SNONE            , E_PAGE_10, 0xA3),
    E_REG_P10_IF5_PB1_RW            = SPA(E_SNONE            , E_PAGE_10, 0xA4),
    E_REG_P10_IF5_PB2_RW            = SPA(E_SNONE            , E_PAGE_10, 0xA5),
    E_REG_P10_IF5_PB3_RW            = SPA(E_SNONE            , E_PAGE_10, 0xA6),
    E_REG_P10_IF5_PB4_RW            = SPA(E_SNONE            , E_PAGE_10, 0xA7),
    E_REG_P10_IF5_PB5_RW            = SPA(E_SNONE            , E_PAGE_10, 0xA8),
    E_REG_P10_IF5_PB6_RW            = SPA(E_SNONE            , E_PAGE_10, 0xA9),
    E_REG_P10_IF5_PB7_RW            = SPA(E_SNONE            , E_PAGE_10, 0xAA),
    E_REG_P10_IF5_PB8_RW            = SPA(E_SNONE            , E_PAGE_10, 0xAB),
    E_REG_P10_IF5_PB9_RW            = SPA(E_SNONE            , E_PAGE_10, 0xAC),
    E_REG_P10_IF5_PB10_RW           = SPA(E_SNONE            , E_PAGE_10, 0xAD),
    E_REG_P10_IF5_PB11_RW           = SPA(E_SNONE            , E_PAGE_10, 0xAE),
    E_REG_P10_IF5_PB12_RW           = SPA(E_SNONE            , E_PAGE_10, 0xAF),
    E_REG_P10_IF5_PB13_RW           = SPA(E_SNONE            , E_PAGE_10, 0xB0),
    E_REG_P10_IF5_PB14_RW           = SPA(E_SNONE            , E_PAGE_10, 0xB1),
    E_REG_P10_IF5_PB15_RW           = SPA(E_SNONE            , E_PAGE_10, 0xB2),
    E_REG_P10_IF5_PB16_RW           = SPA(E_SNONE            , E_PAGE_10, 0xB3),
    E_REG_P10_IF5_PB17_RW           = SPA(E_SNONE            , E_PAGE_10, 0xB4),
    E_REG_P10_IF5_PB18_RW           = SPA(E_SNONE            , E_PAGE_10, 0xB5),
    E_REG_P10_IF5_PB19_RW           = SPA(E_SNONE            , E_PAGE_10, 0xB6),
    E_REG_P10_IF5_PB20_RW           = SPA(E_SNONE            , E_PAGE_10, 0xB7),
    E_REG_P10_IF5_PB21_RW           = SPA(E_SNONE            , E_PAGE_10, 0xB8),
    E_REG_P10_IF5_PB22_RW           = SPA(E_SNONE            , E_PAGE_10, 0xB9),
    E_REG_P10_IF5_PB23_RW           = SPA(E_SNONE            , E_PAGE_10, 0xBA),
    E_REG_P10_IF5_PB24_RW           = SPA(E_SNONE            , E_PAGE_10, 0xBB),
    E_REG_P10_IF5_PB25_RW           = SPA(E_SNONE            , E_PAGE_10, 0xBC),
    E_REG_P10_IF5_PB26_RW           = SPA(E_SNONE            , E_PAGE_10, 0xBD),
    E_REG_P10_IF5_PB27_RW           = SPA(E_SNONE            , E_PAGE_10, 0xBE),
    E_REG_P11_AIP_CNTRL_0_RW        = SPA(E_SNONE            , E_PAGE_11, 0x00),
    E_REG_P11_CA_I2S_RW             = SPA(E_SNONE            , E_PAGE_11, 0x01),
    E_REG_P11_CA_DSD_RW             = SPA(E_SNONE            , E_PAGE_11, 0x02),
    E_REG_P11_OBA_PH_RW             = SPA(E_SNONE            , E_PAGE_11, 0x03),
    E_REG_P11_LATENCY_RD_RW         = SPA(E_SNONE            , E_PAGE_11, 0x04),
    E_REG_P11_ACR_CTS_0_RW          = SPA(E_SNONE            , E_PAGE_11, 0x05),
    E_REG_P11_ACR_CTS_1_RW          = SPA(E_SNONE            , E_PAGE_11, 0x06),
    E_REG_P11_ACR_CTS_2_RW          = SPA(E_SNONE            , E_PAGE_11, 0x07),
    E_REG_P11_ACR_N_0_RW            = SPA(E_SNONE            , E_PAGE_11, 0x08),
    E_REG_P11_ACR_N_1_RW            = SPA(E_SNONE            , E_PAGE_11, 0x09),
    E_REG_P11_ACR_N_2_RW            = SPA(E_SNONE            , E_PAGE_11, 0x0A),
    E_REG_P11_GC_AVMUTE_RW          = SPA(E_SNONE            , E_PAGE_11, 0x0B),
    E_REG_P11_CTS_N_RW              = SPA(E_SNONE            , E_PAGE_11, 0x0C),
    E_REG_P11_ENC_CNTRL_RW          = SPA(E_SNONE            , E_PAGE_11, 0x0D),
    E_REG_P11_DIP_FLAGS_RW          = SPA(E_SNONE            , E_PAGE_11, 0x0E),
    E_REG_P11_DIP_IF_FLAGS_RW       = SPA(E_SNONE            , E_PAGE_11, 0x0F),
    E_REG_P11_CH_STAT_B_0_RW        = SPA(E_SNONE            , E_PAGE_11, 0x14),
    E_REG_P11_CH_STAT_B_1_RW        = SPA(E_SNONE            , E_PAGE_11, 0x15),
    E_REG_P11_CH_STAT_B_3_RW        = SPA(E_SNONE            , E_PAGE_11, 0x16),
    E_REG_P11_CH_STAT_B_4_RW        = SPA(E_SNONE            , E_PAGE_11, 0x17),
    E_REG_P11_CH_STAT_B_2_ap0_l_RW  = SPA(E_SNONE            , E_PAGE_11, 0x18),
    E_REG_P11_CH_STAT_B_2_ap0_r_RW  = SPA(E_SNONE            , E_PAGE_11, 0x19),
    E_REG_P11_CH_STAT_B_2_ap1_l_RW  = SPA(E_SNONE            , E_PAGE_11, 0x1A),
    E_REG_P11_CH_STAT_B_2_ap1_r_RW  = SPA(E_SNONE            , E_PAGE_11, 0x1B),
    E_REG_P11_CH_STAT_B_2_ap2_l_RW  = SPA(E_SNONE            , E_PAGE_11, 0x1C),
    E_REG_P11_CH_STAT_B_2_ap2_r_RW  = SPA(E_SNONE            , E_PAGE_11, 0x1D),
    E_REG_P11_CH_STAT_B_2_ap3_l_RW  = SPA(E_SNONE            , E_PAGE_11, 0x1E),
    E_REG_P11_CH_STAT_B_2_ap3_r_RW  = SPA(E_SNONE            , E_PAGE_11, 0x1F),
    E_REG_P11_ISRC1_HB0_RW          = SPA(E_SNONE            , E_PAGE_11, 0x20), /* 20-3E: n3 uses this area for VideoInfoFrame */
    E_REG_P11_ISRC1_HB1_RW          = SPA(E_SNONE            , E_PAGE_11, 0x21),
    E_REG_P11_ISRC1_HB2_RW          = SPA(E_SNONE            , E_PAGE_11, 0x22),
    E_REG_P11_ISRC1_PB0_RW          = SPA(E_SNONE            , E_PAGE_11, 0x23),
    E_REG_P11_ISRC1_PB1_RW          = SPA(E_SNONE            , E_PAGE_11, 0x24),
    E_REG_P11_ISRC1_PB2_RW          = SPA(E_SNONE            , E_PAGE_11, 0x25),
    E_REG_P11_ISRC1_PB3_RW          = SPA(E_SNONE            , E_PAGE_11, 0x26),
    E_REG_P11_ISRC1_PB4_RW          = SPA(E_SNONE            , E_PAGE_11, 0x27),
    E_REG_P11_ISRC1_PB5_RW          = SPA(E_SNONE            , E_PAGE_11, 0x28),
    E_REG_P11_ISRC1_PB6_RW          = SPA(E_SNONE            , E_PAGE_11, 0x29),
    E_REG_P11_ISRC1_PB7_RW          = SPA(E_SNONE            , E_PAGE_11, 0x2A),
    E_REG_P11_ISRC1_PB8_RW          = SPA(E_SNONE            , E_PAGE_11, 0x2B),
    E_REG_P11_ISRC1_PB9_RW          = SPA(E_SNONE            , E_PAGE_11, 0x2C),
    E_REG_P11_ISRC1_PB10_RW         = SPA(E_SNONE            , E_PAGE_11, 0x2D),
    E_REG_P11_ISRC1_PB11_RW         = SPA(E_SNONE            , E_PAGE_11, 0x2E),
    E_REG_P11_ISRC1_PB12_RW         = SPA(E_SNONE            , E_PAGE_11, 0x2F),
    E_REG_P11_ISRC1_PB13_RW         = SPA(E_SNONE            , E_PAGE_11, 0x30),
    E_REG_P11_ISRC1_PB14_RW         = SPA(E_SNONE            , E_PAGE_11, 0x31),
    E_REG_P11_ISRC1_PB15_RW         = SPA(E_SNONE            , E_PAGE_11, 0x32),
    E_REG_P11_ISRC1_PB16_RW         = SPA(E_SNONE            , E_PAGE_11, 0x33),
    E_REG_P11_ISRC1_PB17_RW         = SPA(E_SNONE            , E_PAGE_11, 0x34),
    E_REG_P11_ISRC1_PB18_RW         = SPA(E_SNONE            , E_PAGE_11, 0x35),
    E_REG_P11_ISRC1_PB19_RW         = SPA(E_SNONE            , E_PAGE_11, 0x36),
    E_REG_P11_ISRC1_PB20_RW         = SPA(E_SNONE            , E_PAGE_11, 0x37),
    E_REG_P11_ISRC1_PB21_RW         = SPA(E_SNONE            , E_PAGE_11, 0x38),
    E_REG_P11_ISRC1_PB22_RW         = SPA(E_SNONE            , E_PAGE_11, 0x39),
    E_REG_P11_ISRC1_PB23_RW         = SPA(E_SNONE            , E_PAGE_11, 0x3A),
    E_REG_P11_ISRC1_PB24_RW         = SPA(E_SNONE            , E_PAGE_11, 0x3B),
    E_REG_P11_ISRC1_PB25_RW         = SPA(E_SNONE            , E_PAGE_11, 0x3C),
    E_REG_P11_ISRC1_PB26_RW         = SPA(E_SNONE            , E_PAGE_11, 0x3D),
    E_REG_P11_ISRC1_PB27_RW         = SPA(E_SNONE            , E_PAGE_11, 0x3E),
    E_REG_P11_ISRC2_HB0_RW          = SPA(E_SNONE            , E_PAGE_11, 0x40),
    E_REG_P11_ISRC2_HB1_RW          = SPA(E_SNONE            , E_PAGE_11, 0x41),
    E_REG_P11_ISRC2_HB2_RW          = SPA(E_SNONE            , E_PAGE_11, 0x42),
    E_REG_P11_ISRC2_PB0_RW          = SPA(E_SNONE            , E_PAGE_11, 0x43),
    E_REG_P11_ISRC2_PB1_RW          = SPA(E_SNONE            , E_PAGE_11, 0x44),
    E_REG_P11_ISRC2_PB2_RW          = SPA(E_SNONE            , E_PAGE_11, 0x45),
    E_REG_P11_ISRC2_PB3_RW          = SPA(E_SNONE            , E_PAGE_11, 0x46),
    E_REG_P11_ISRC2_PB4_RW          = SPA(E_SNONE            , E_PAGE_11, 0x47),
    E_REG_P11_ISRC2_PB5_RW          = SPA(E_SNONE            , E_PAGE_11, 0x48),
    E_REG_P11_ISRC2_PB6_RW          = SPA(E_SNONE            , E_PAGE_11, 0x49),
    E_REG_P11_ISRC2_PB7_RW          = SPA(E_SNONE            , E_PAGE_11, 0x4A),
    E_REG_P11_ISRC2_PB8_RW          = SPA(E_SNONE            , E_PAGE_11, 0x4B),
    E_REG_P11_ISRC2_PB9_RW          = SPA(E_SNONE            , E_PAGE_11, 0x4C),
    E_REG_P11_ISRC2_PB10_RW         = SPA(E_SNONE            , E_PAGE_11, 0x4D),
    E_REG_P11_ISRC2_PB11_RW         = SPA(E_SNONE            , E_PAGE_11, 0x4E),
    E_REG_P11_ISRC2_PB12_RW         = SPA(E_SNONE            , E_PAGE_11, 0x4F),
    E_REG_P11_ISRC2_PB13_RW         = SPA(E_SNONE            , E_PAGE_11, 0x50),
    E_REG_P11_ISRC2_PB14_RW         = SPA(E_SNONE            , E_PAGE_11, 0x51),
    E_REG_P11_ISRC2_PB15_RW         = SPA(E_SNONE            , E_PAGE_11, 0x52),
    E_REG_P11_ISRC2_PB16_RW         = SPA(E_SNONE            , E_PAGE_11, 0x53),
    E_REG_P11_ISRC2_PB17_RW         = SPA(E_SNONE            , E_PAGE_11, 0x54),
    E_REG_P11_ISRC2_PB18_RW         = SPA(E_SNONE            , E_PAGE_11, 0x55),
    E_REG_P11_ISRC2_PB19_RW         = SPA(E_SNONE            , E_PAGE_11, 0x56),
    E_REG_P11_ISRC2_PB20_RW         = SPA(E_SNONE            , E_PAGE_11, 0x57),
    E_REG_P11_ISRC2_PB21_RW         = SPA(E_SNONE            , E_PAGE_11, 0x58),
    E_REG_P11_ISRC2_PB22_RW         = SPA(E_SNONE            , E_PAGE_11, 0x59),
    E_REG_P11_ISRC2_PB23_RW         = SPA(E_SNONE            , E_PAGE_11, 0x5A),
    E_REG_P11_ISRC2_PB24_RW         = SPA(E_SNONE            , E_PAGE_11, 0x5B),
    E_REG_P11_ISRC2_PB25_RW         = SPA(E_SNONE            , E_PAGE_11, 0x5C),
    E_REG_P11_ISRC2_PB26_RW         = SPA(E_SNONE            , E_PAGE_11, 0x5D),
    E_REG_P11_ISRC2_PB27_RW         = SPA(E_SNONE            , E_PAGE_11, 0x5E),
    E_REG_P11_ACP_HB0_RW            = SPA(E_SNONE            , E_PAGE_11, 0x60), /* 60-7E: n3 uses this area for AudioInfoFrame */
    E_REG_P11_ACP_HB1_RW            = SPA(E_SNONE            , E_PAGE_11, 0x61),
    E_REG_P11_ACP_HB2_RW            = SPA(E_SNONE            , E_PAGE_11, 0x62),
    E_REG_P11_ACP_PB0_RW            = SPA(E_SNONE            , E_PAGE_11, 0x63),
    E_REG_P11_ACP_PB1_RW            = SPA(E_SNONE            , E_PAGE_11, 0x64),
    E_REG_P11_ACP_PB2_RW            = SPA(E_SNONE            , E_PAGE_11, 0x65),
    E_REG_P11_ACP_PB3_RW            = SPA(E_SNONE            , E_PAGE_11, 0x66),
    E_REG_P11_ACP_PB4_RW            = SPA(E_SNONE            , E_PAGE_11, 0x67),
    E_REG_P11_ACP_PB5_RW            = SPA(E_SNONE            , E_PAGE_11, 0x68),
    E_REG_P11_ACP_PB6_RW            = SPA(E_SNONE            , E_PAGE_11, 0x69),
    E_REG_P11_ACP_PB7_RW            = SPA(E_SNONE            , E_PAGE_11, 0x6A),
    E_REG_P11_ACP_PB8_RW            = SPA(E_SNONE            , E_PAGE_11, 0x6B),
    E_REG_P11_ACP_PB9_RW            = SPA(E_SNONE            , E_PAGE_11, 0x6C),
    E_REG_P11_ACP_PB10_RW           = SPA(E_SNONE            , E_PAGE_11, 0x6D),
    E_REG_P11_ACP_PB11_RW           = SPA(E_SNONE            , E_PAGE_11, 0x6E),
    E_REG_P11_ACP_PB12_RW           = SPA(E_SNONE            , E_PAGE_11, 0x6F),
    E_REG_P11_ACP_PB13_RW           = SPA(E_SNONE            , E_PAGE_11, 0x70),
    E_REG_P11_ACP_PB14_RW           = SPA(E_SNONE            , E_PAGE_11, 0x71),
    E_REG_P11_ACP_PB15_RW           = SPA(E_SNONE            , E_PAGE_11, 0x72),
    E_REG_P11_ACP_PB16_RW           = SPA(E_SNONE            , E_PAGE_11, 0x73),
    E_REG_P11_ACP_PB17_RW           = SPA(E_SNONE            , E_PAGE_11, 0x74),
    E_REG_P11_ACP_PB18_RW           = SPA(E_SNONE            , E_PAGE_11, 0x75),
    E_REG_P11_ACP_PB19_RW           = SPA(E_SNONE            , E_PAGE_11, 0x76),
    E_REG_P11_ACP_PB20_RW           = SPA(E_SNONE            , E_PAGE_11, 0x77),
    E_REG_P11_ACP_PB21_RW           = SPA(E_SNONE            , E_PAGE_11, 0x78),
    E_REG_P11_ACP_PB22_RW           = SPA(E_SNONE            , E_PAGE_11, 0x79),
    E_REG_P11_ACP_PB23_RW           = SPA(E_SNONE            , E_PAGE_11, 0x7A),
    E_REG_P11_ACP_PB24_RW           = SPA(E_SNONE            , E_PAGE_11, 0x7B),
    E_REG_P11_ACP_PB25_RW           = SPA(E_SNONE            , E_PAGE_11, 0x7C),
    E_REG_P11_ACP_PB26_RW           = SPA(E_SNONE            , E_PAGE_11, 0x7D),
    E_REG_P11_ACP_PB27_RW           = SPA(E_SNONE            , E_PAGE_11, 0x7E), /* 80-9E: deleted according to tda9983n3_i2c_GSmodifications_2.xls 27/3/2006 */
    E_REG_P12_OTP_ADDRESS_W         = SPA(E_SNONE            , E_PAGE_12, 0x30),
    E_REG_P12_OTP_DATA_MSB_RW       = SPA(E_SNONE            , E_PAGE_12, 0x31),
    E_REG_P12_OTP_DATA_ISB_RW       = SPA(E_SNONE            , E_PAGE_12, 0x32),
    E_REG_P12_OTP_DATA_LSB_RW       = SPA(E_SNONE            , E_PAGE_12, 0x33),
    E_REG_P12_OTP_CNTRL_W           = SPA(E_SNONE            , E_PAGE_12, 0x34),
    E_REG_P12_OTP_STATUS_R          = SPA(E_SNONE            , E_PAGE_12, 0x35),
    E_REG_P12_VPP_TIMER_W           = SPA(E_SNONE            , E_PAGE_12, 0x36),
    E_REG_P12_WRITE_TIMER_W         = SPA(E_SNONE            , E_PAGE_12, 0x37),
    E_REG_P12_DISCHARGE_TIMER_W     = SPA(E_SNONE            , E_PAGE_12, 0x38),
    E_REG_P12_READ_TIMER_W          = SPA(E_SNONE            , E_PAGE_12, 0x39),
#ifndef NO_HDCP
    E_REG_P12_HDCP_CTRL_W           = SPA(E_SP12_HDCP_CTRL   , E_PAGE_12, 0x40),
    E_REG_P12_HDCP_KDS_HI_W         = SPA(E_SNONE            , E_PAGE_12, 0x42),
    E_REG_P12_HDCP_KDS_LO_W         = SPA(E_SNONE            , E_PAGE_12, 0x43),
#ifdef BCAPS_REPEATER
    E_REG_P12_HDCP_BCAPS_W          = SPA(E_SP12_HDCP_BCAPS  , E_PAGE_12, 0x49),
#else
    E_REG_P12_HDCP_BCAPS_W          = SPA(E_SNONE            , E_PAGE_12, 0x49),
#endif /* BCAPS_REPEATER */
    E_REG_P12_HDCP_AKSV4_W          = SPA(E_SNONE            , E_PAGE_12, 0x4B),
    E_REG_P12_HDCP_AKSV3_W          = SPA(E_SNONE            , E_PAGE_12, 0x4C),
    E_REG_P12_HDCP_AKSV2_W          = SPA(E_SNONE            , E_PAGE_12, 0x4D),
    E_REG_P12_HDCP_AKSV1_W          = SPA(E_SNONE            , E_PAGE_12, 0x4E),
    E_REG_P12_HDCP_AKSV0_W          = SPA(E_SNONE            , E_PAGE_12, 0x4F),
    E_REG_P12_HDCP_KIDX_W           = SPA(E_SNONE            , E_PAGE_12, 0x50),
    E_REG_P12_HDCP_KEY6_W           = SPA(E_SNONE            , E_PAGE_12, 0x51),
    E_REG_P12_HDCP_KEY5_W           = SPA(E_SNONE            , E_PAGE_12, 0x52),
    E_REG_P12_HDCP_KEY4_W           = SPA(E_SNONE            , E_PAGE_12, 0x53),
    E_REG_P12_HDCP_KEY3_W           = SPA(E_SNONE            , E_PAGE_12, 0x54),
    E_REG_P12_HDCP_KEY2_W           = SPA(E_SNONE            , E_PAGE_12, 0x55),
    E_REG_P12_HDCP_KEY1_W           = SPA(E_SNONE            , E_PAGE_12, 0x56),
    E_REG_P12_HDCP_KEY0_W           = SPA(E_SNONE            , E_PAGE_12, 0x57),
    E_REG_P12_HDCP_RI1_R            = SPA(E_SNONE            , E_PAGE_12, 0x60),
    E_REG_P12_HDCP_RI0_R            = SPA(E_SNONE            , E_PAGE_12, 0x61),
    E_REG_P12_HDCP_FSM_STATE_R      = SPA(E_SNONE            , E_PAGE_12, 0x62),
    E_REG_P12_HDCP_BKSV4_R          = SPA(E_SNONE            , E_PAGE_12, 0x63),
    E_REG_P12_HDCP_BKSV3_R          = SPA(E_SNONE            , E_PAGE_12, 0x64),
    E_REG_P12_HDCP_BKSV2_R          = SPA(E_SNONE            , E_PAGE_12, 0x65),
    E_REG_P12_HDCP_BKSV1_R          = SPA(E_SNONE            , E_PAGE_12, 0x66),
    E_REG_P12_HDCP_BKSV0_R          = SPA(E_SNONE            , E_PAGE_12, 0x67),
    E_REG_P12_HDCP_AN7_R            = SPA(E_SNONE            , E_PAGE_12, 0x68),
    E_REG_P12_HDCP_AN6_R            = SPA(E_SNONE            , E_PAGE_12, 0x69),
    E_REG_P12_HDCP_AN5_R            = SPA(E_SNONE            , E_PAGE_12, 0x6A),
    E_REG_P12_HDCP_AN4_R            = SPA(E_SNONE            , E_PAGE_12, 0x6B),
    E_REG_P12_HDCP_AN3_R            = SPA(E_SNONE            , E_PAGE_12, 0x6C),
    E_REG_P12_HDCP_AN2_R            = SPA(E_SNONE            , E_PAGE_12, 0x6D),
    E_REG_P12_HDCP_AN1_R            = SPA(E_SNONE            , E_PAGE_12, 0x6E),
    E_REG_P12_HDCP_AN0_R            = SPA(E_SNONE            , E_PAGE_12, 0x6F),
    E_REG_P12_HDCP_PJ_R             = SPA(E_SNONE            , E_PAGE_12, 0x70),
    E_REG_P12_HDCP_AINFO_R          = SPA(E_SNONE            , E_PAGE_12, 0x71),
    E_REG_P12_HDCP_KSVPTR_LSB_W     = SPA(E_SNONE            , E_PAGE_12, 0x90),
    E_REG_P12_HDCP_KSVPTR_MSB_W     = SPA(E_SNONE            , E_PAGE_12, 0x91),
    E_REG_P12_HDCP_KSV0_R           = SPA(E_SNONE            , E_PAGE_12, 0x92),
    E_REG_P12_HDCP_KSV1_R           = SPA(E_SNONE            , E_PAGE_12, 0x93),
    E_REG_P12_HDCP_KSV2_R           = SPA(E_SNONE            , E_PAGE_12, 0x94),
    E_REG_P12_HDCP_KSV3_R           = SPA(E_SNONE            , E_PAGE_12, 0x95),
    E_REG_P12_HDCP_KSV4_R           = SPA(E_SNONE            , E_PAGE_12, 0x96),
    E_REG_P12_HDCP_TX0_RW           = SPA(E_SNONE            , E_PAGE_12, 0x97),
    E_REG_P12_HDCP_TX1_RW           = SPA(E_SNONE            , E_PAGE_12, 0x98),
    E_REG_P12_HDCP_TX2_RW           = SPA(E_SNONE            , E_PAGE_12, 0x99),
    E_REG_P12_HDCP_TX3_RW           = SPA(E_SNONE            , E_PAGE_12, 0x9A),
    E_REG_P12_HDCP_KSV_check_R      = SPA(E_SNONE            , E_PAGE_12, 0xA0),
    E_REG_P12_HDCP_BSTATUS_1_R      = SPA(E_SNONE            , E_PAGE_12, 0xA1),
    E_REG_P12_HDCP_BSTATUS_0_R      = SPA(E_SNONE            , E_PAGE_12, 0xA2),
    E_REG_P12_HDCP_BCAPS_RX_R       = SPA(E_SNONE            , E_PAGE_12, 0xA3),
    E_REG_P12_HDCP_V0_0_R           = SPA(E_SNONE            , E_PAGE_12, 0xA4),
    E_REG_P12_HDCP_V0_1_R           = SPA(E_SNONE            , E_PAGE_12, 0xA5),
    E_REG_P12_HDCP_V0_2_R           = SPA(E_SNONE            , E_PAGE_12, 0xA6),
    E_REG_P12_HDCP_V0_3_R           = SPA(E_SNONE            , E_PAGE_12, 0xA7),
    E_REG_P12_HDCP_V1_0_R           = SPA(E_SNONE            , E_PAGE_12, 0xA8),
    E_REG_P12_HDCP_V1_1_R           = SPA(E_SNONE            , E_PAGE_12, 0xA9),
    E_REG_P12_HDCP_V1_2_R           = SPA(E_SNONE            , E_PAGE_12, 0xAA),
    E_REG_P12_HDCP_V1_3_R           = SPA(E_SNONE            , E_PAGE_12, 0xAB),
    E_REG_P12_HDCP_V2_0_R           = SPA(E_SNONE            , E_PAGE_12, 0xAC),
    E_REG_P12_HDCP_V2_1_R           = SPA(E_SNONE            , E_PAGE_12, 0xAD),
    E_REG_P12_HDCP_V2_2_R           = SPA(E_SNONE            , E_PAGE_12, 0xAE),
    E_REG_P12_HDCP_V2_3_R           = SPA(E_SNONE            , E_PAGE_12, 0xAF),
    E_REG_P12_HDCP_V3_0_R           = SPA(E_SNONE            , E_PAGE_12, 0xB0),
    E_REG_P12_HDCP_V3_1_R           = SPA(E_SNONE            , E_PAGE_12, 0xB1),
    E_REG_P12_HDCP_V3_2_R           = SPA(E_SNONE            , E_PAGE_12, 0xB2),
    E_REG_P12_HDCP_V3_3_R           = SPA(E_SNONE            , E_PAGE_12, 0xB3),
    E_REG_P12_HDCP_V4_0_R           = SPA(E_SNONE            , E_PAGE_12, 0xB4),
    E_REG_P12_HDCP_V4_1_R           = SPA(E_SNONE            , E_PAGE_12, 0xB5),
    E_REG_P12_HDCP_V4_2_R           = SPA(E_SNONE            , E_PAGE_12, 0xB6),
    E_REG_P12_HDCP_V4_3_R           = SPA(E_SNONE            , E_PAGE_12, 0xB7),
#endif /* NO_HDCP */
    E_REG_P12_HDCP_TX33_RW          = SPA(E_SNONE            , E_PAGE_12, 0xB8)
#ifndef NO_HDCP
    ,E_REG_P12_HDCP_FAILSTATE_R     = SPA(E_SNONE            , E_PAGE_12, 0xB9)
#endif /* NO_HDCP */
};
#undef SPR

/**
 * Register bitfield masks, with a macro to allow binary initializers.
 * Enum names are derived directly from TDA998x register and bitfield names.
 */
#define BINARY(d7,d6,d5,d4,d3,d2,d1,d0) \
    (((d7)<<7)|((d6)<<6)|((d5)<<5)|((d4)<<4)|((d3)<<3)|((d2)<<2)|((d1)<<1)|(d0))

enum _eMaskReg
{
    E_MASKREG_NONE                          = BINARY(0,0,0,0, 0,0,0,0),
    E_MASKREG_ALL                           = BINARY(1,1,1,1, 1,1,1,1),

    /* N4 features flags read from version register:
     * not_h = no HDCP support
     * not_s = no scaler support
     *
     * N5 = a flag that is not a register bit, but is derived by the
     * driver from the new N5 registers DWIN_RE_DE and DWIN_FE_DE,
     * because the N5 device still uses the N4 version register value.
     * This bit position would clash with version register, so is not
     * present in the driver's copy (uDeviceVersion) of the version 
     * register, but only in the driver's features byte (uDeviceFeatures).
     */
    E_MASKREG_P00_VERSION_not_h             = BINARY(0,0,1,0, 0,0,0,0),
    E_MASKREG_P00_VERSION_not_s             = BINARY(0,0,0,1, 0,0,0,0),
    E_MASKREG_P00_FEATURE_N5                = BINARY(0,0,0,0, 0,0,0,1),

    E_MASKREG_P00_MAIN_CNTRL0_scaler        = BINARY(1,0,0,0, 0,0,0,0),
    E_MASKREG_P00_MAIN_CNTRL0_cehs          = BINARY(0,0,0,1, 0,0,0,0),
    E_MASKREG_P00_MAIN_CNTRL0_cecs          = BINARY(0,0,0,0, 1,0,0,0),
    E_MASKREG_P00_MAIN_CNTRL0_dehs          = BINARY(0,0,0,0, 0,1,0,0),
    E_MASKREG_P00_MAIN_CNTRL0_decs          = BINARY(0,0,0,0, 0,0,1,0),
    E_MASKREG_P00_MAIN_CNTRL0_sr            = BINARY(0,0,0,0, 0,0,0,1),

#ifdef TMFL_TDA9981_SUPPORT
	E_MASKREG_P00_SR_REG_sr_i2c_ms			= BINARY(0,0,0,0, 0,0,1,0),
	E_MASKREG_P00_SR_REG_sr_audio			= BINARY(0,0,0,0, 0,0,0,1),

	E_MASKREG_P00_DDC_DISABLE_ddc_dis		= BINARY(0,0,0,0, 0,0,0,1),

	E_MASKREG_P00_CCLK_ON_cclk_on			= BINARY(0,0,0,0, 0,0,0,1),

	E_MASKREG_P00_INT_FLAGS_0_r0			= BINARY(1,0,0,0, 0,0,0,0),
#endif /* TMFL_TDA9981_SUPPORT */
    E_MASKREG_P00_INT_FLAGS_0_pj            = BINARY(0,1,0,0, 0,0,0,0),
    E_MASKREG_P00_INT_FLAGS_0_sha_1         = BINARY(0,0,1,0, 0,0,0,0),
    E_MASKREG_P00_INT_FLAGS_0_bstatus       = BINARY(0,0,0,1, 0,0,0,0),
    E_MASKREG_P00_INT_FLAGS_0_bcaps         = BINARY(0,0,0,0, 1,0,0,0),
    E_MASKREG_P00_INT_FLAGS_0_t0            = BINARY(0,0,0,0, 0,1,0,0),
    E_MASKREG_P00_INT_FLAGS_0_hpd           = BINARY(0,0,0,0, 0,0,1,0),
    E_MASKREG_P00_INT_FLAGS_0_encrypt       = BINARY(0,0,0,0, 0,0,0,1),

    E_MASKREG_P00_INT_FLAGS_1_hpd_in 		= BINARY(1,0,0,0, 0,0,0,0),
#ifdef TMFL_TDA9981_SUPPORT
	E_MASKREG_P00_INT_FLAGS_1_sw_int		= BINARY(0,1,0,0, 0,0,0,0),
#endif /* TMFL_TDA9981_SUPPORT */
    E_MASKREG_P00_INT_FLAGS_1_sc_deil		= BINARY(0,0,1,0, 0,0,0,0),
    E_MASKREG_P00_INT_FLAGS_1_sc_vid		= BINARY(0,0,0,1, 0,0,0,0),
    E_MASKREG_P00_INT_FLAGS_1_sc_out		= BINARY(0,0,0,0, 1,0,0,0),
    E_MASKREG_P00_INT_FLAGS_1_sc_in			= BINARY(0,0,0,0, 0,1,0,0),
    E_MASKREG_P00_INT_FLAGS_1_otp			= BINARY(0,0,0,0, 0,0,1,0),
    E_MASKREG_P00_INT_FLAGS_1_vs_rpt		= BINARY(0,0,0,0, 0,0,0,1),
#ifdef TMFL_TDA9981_SUPPORT
	E_MASKREG_P00_INT_FLAGS_2_rx_sense		= BINARY(0,0,0,0, 0,0,0,1),

	E_MASKREG_P00_INT_FLAGS_3_rxs_fil		= BINARY(0,0,0,0, 0,0,0,1),
	
	E_MASKREG_P00_SW_INT_sw_int				= BINARY(0,0,0,0, 0,0,0,1),
	
	E_MASKREG_P00_ENA_VP_0_ena_vp7			= BINARY(1,0,0,0, 0,0,0,0),
	E_MASKREG_P00_ENA_VP_0_ena_vp6			= BINARY(0,1,0,0, 0,0,0,0),
	E_MASKREG_P00_ENA_VP_0_ena_vp5			= BINARY(0,0,1,0, 0,0,0,0),
	E_MASKREG_P00_ENA_VP_0_ena_vp4			= BINARY(0,0,0,1, 0,0,0,0),
	E_MASKREG_P00_ENA_VP_0_ena_vp3			= BINARY(0,0,0,0, 1,0,0,0),
	E_MASKREG_P00_ENA_VP_0_ena_vp2			= BINARY(0,0,0,0, 0,1,0,0),
	E_MASKREG_P00_ENA_VP_0_ena_vp1			= BINARY(0,0,0,0, 0,0,1,0),
	E_MASKREG_P00_ENA_VP_0_ena_vp0			= BINARY(0,0,0,0, 0,0,0,1),
	
	E_MASKREG_P00_ENA_VP_1_ena_vp15			= BINARY(1,0,0,0, 0,0,0,0),
	E_MASKREG_P00_ENA_VP_1_ena_vp14			= BINARY(0,1,0,0, 0,0,0,0),
	E_MASKREG_P00_ENA_VP_1_ena_vp13			= BINARY(0,0,1,0, 0,0,0,0),
	E_MASKREG_P00_ENA_VP_1_ena_vp12			= BINARY(0,0,0,1, 0,0,0,0),
	E_MASKREG_P00_ENA_VP_1_ena_vp11			= BINARY(0,0,0,0, 1,0,0,0),
	E_MASKREG_P00_ENA_VP_1_ena_vp10			= BINARY(0,0,0,0, 0,1,0,0),
	E_MASKREG_P00_ENA_VP_1_ena_vp9			= BINARY(0,0,0,0, 0,0,1,0),
	E_MASKREG_P00_ENA_VP_1_ena_vp8			= BINARY(0,0,0,0, 0,0,0,1),
	
	E_MASKREG_P00_ENA_VP_2_ena_vp23			= BINARY(1,0,0,0, 0,0,0,0),
	E_MASKREG_P00_ENA_VP_2_ena_vp22			= BINARY(0,1,0,0, 0,0,0,0),
	E_MASKREG_P00_ENA_VP_2_ena_vp21			= BINARY(0,0,1,0, 0,0,0,0),
	E_MASKREG_P00_ENA_VP_2_ena_vp20			= BINARY(0,0,0,1, 0,0,0,0),
	E_MASKREG_P00_ENA_VP_2_ena_vp19			= BINARY(0,0,0,0, 1,0,0,0),
	E_MASKREG_P00_ENA_VP_2_ena_vp18			= BINARY(0,0,0,0, 0,1,0,0),
	E_MASKREG_P00_ENA_VP_2_ena_vp17			= BINARY(0,0,0,0, 0,0,1,0),
	E_MASKREG_P00_ENA_VP_2_ena_vp16			= BINARY(0,0,0,0, 0,0,0,1),
	
	E_MASKREG_P00_GND_VP_0_ena_vp7			= BINARY(1,0,0,0, 0,0,0,0),
	E_MASKREG_P00_GND_VP_0_ena_vp6			= BINARY(0,1,0,0, 0,0,0,0),
	E_MASKREG_P00_GND_VP_0_ena_vp5			= BINARY(0,0,1,0, 0,0,0,0),
	E_MASKREG_P00_GND_VP_0_ena_vp4			= BINARY(0,0,0,1, 0,0,0,0),
	E_MASKREG_P00_GND_VP_0_ena_vp3			= BINARY(0,0,0,0, 1,0,0,0),
	E_MASKREG_P00_GND_VP_0_ena_vp2			= BINARY(0,0,0,0, 0,1,0,0),
	E_MASKREG_P00_GND_VP_0_ena_vp1			= BINARY(0,0,0,0, 0,0,1,0),
	E_MASKREG_P00_GND_VP_0_ena_vp0			= BINARY(0,0,0,0, 0,0,0,1),
	
	E_MASKREG_P00_GND_VP_1_ena_vp15			= BINARY(1,0,0,0, 0,0,0,0),
	E_MASKREG_P00_GND_VP_1_ena_vp14			= BINARY(0,1,0,0, 0,0,0,0),
	E_MASKREG_P00_GND_VP_1_ena_vp13			= BINARY(0,0,1,0, 0,0,0,0),
	E_MASKREG_P00_GND_VP_1_ena_vp12			= BINARY(0,0,0,1, 0,0,0,0),
	E_MASKREG_P00_GND_VP_1_ena_vp11			= BINARY(0,0,0,0, 1,0,0,0),
	E_MASKREG_P00_GND_VP_1_ena_vp10			= BINARY(0,0,0,0, 0,1,0,0),
	E_MASKREG_P00_GND_VP_1_ena_vp9			= BINARY(0,0,0,0, 0,0,1,0),
	E_MASKREG_P00_GND_VP_1_ena_vp8			= BINARY(0,0,0,0, 0,0,0,1),
	
	E_MASKREG_P00_GND_VP_2_ena_vp23			= BINARY(1,0,0,0, 0,0,0,0),
	E_MASKREG_P00_GND_VP_2_ena_vp22			= BINARY(0,1,0,0, 0,0,0,0),
	E_MASKREG_P00_GND_VP_2_ena_vp21			= BINARY(0,0,1,0, 0,0,0,0),
	E_MASKREG_P00_GND_VP_2_ena_vp20			= BINARY(0,0,0,1, 0,0,0,0),
	E_MASKREG_P00_GND_VP_2_ena_vp19			= BINARY(0,0,0,0, 1,0,0,0),
	E_MASKREG_P00_GND_VP_2_ena_vp18			= BINARY(0,0,0,0, 0,1,0,0),
	E_MASKREG_P00_GND_VP_2_ena_vp17			= BINARY(0,0,0,0, 0,0,1,0),
	E_MASKREG_P00_GND_VP_2_ena_vp16			= BINARY(0,0,0,0, 0,0,0,1),
	
	E_MASKREG_P00_ENA_AP_ena_ap7			= BINARY(1,0,0,0, 0,0,0,0),
	E_MASKREG_P00_ENA_AP_ena_ap6			= BINARY(0,1,0,0, 0,0,0,0),
	E_MASKREG_P00_ENA_AP_ena_ap5			= BINARY(0,0,1,0, 0,0,0,0),
	E_MASKREG_P00_ENA_AP_ena_ap4			= BINARY(0,0,0,1, 0,0,0,0),
	E_MASKREG_P00_ENA_AP_ena_ap3			= BINARY(0,0,0,0, 1,0,0,0),
	E_MASKREG_P00_ENA_AP_ena_ap2			= BINARY(0,0,0,0, 0,1,0,0),
	E_MASKREG_P00_ENA_AP_ena_ap1			= BINARY(0,0,0,0, 0,0,1,0),
	E_MASKREG_P00_ENA_AP_ena_ap0			= BINARY(0,0,0,0, 0,0,0,1),
	
	E_MASKREG_P00_GND_AP_gnd_ap7			= BINARY(1,0,0,0, 0,0,0,0),
	E_MASKREG_P00_GND_AP_gnd_ap6			= BINARY(0,1,0,0, 0,0,0,0),
	E_MASKREG_P00_GND_AP_gnd_ap5			= BINARY(0,0,1,0, 0,0,0,0),
	E_MASKREG_P00_GND_AP_gnd_ap4			= BINARY(0,0,0,1, 0,0,0,0),
	E_MASKREG_P00_GND_AP_gnd_ap3			= BINARY(0,0,0,0, 1,0,0,0),
	E_MASKREG_P00_GND_AP_gnd_ap2			= BINARY(0,0,0,0, 0,1,0,0),
	E_MASKREG_P00_GND_AP_gnd_ap1			= BINARY(0,0,0,0, 0,0,1,0),
	E_MASKREG_P00_GND_AP_gnd_ap0			= BINARY(0,0,0,0, 0,0,0,1),
#endif /* TMFL_TDA9981_SUPPORT */
   
    E_MASKREG_P00_VIP_CNTRL_0_mirr_a        = BINARY(1,0,0,0, 0,0,0,0),
    E_MASKREG_P00_VIP_CNTRL_0_swap_a        = BINARY(0,1,1,1, 0,0,0,0),
    E_MASKREG_P00_VIP_CNTRL_0_mirr_b        = BINARY(0,0,0,0, 1,0,0,0),
    E_MASKREG_P00_VIP_CNTRL_0_swap_b        = BINARY(0,0,0,0, 0,1,1,1),

    E_MASKREG_P00_VIP_CNTRL_1_mirr_c        = BINARY(1,0,0,0, 0,0,0,0),
    E_MASKREG_P00_VIP_CNTRL_1_swap_c        = BINARY(0,1,1,1, 0,0,0,0),
    E_MASKREG_P00_VIP_CNTRL_1_mirr_d        = BINARY(0,0,0,0, 1,0,0,0),
    E_MASKREG_P00_VIP_CNTRL_1_swap_d        = BINARY(0,0,0,0, 0,1,1,1),

    E_MASKREG_P00_VIP_CNTRL_2_mirr_e        = BINARY(1,0,0,0, 0,0,0,0),
    E_MASKREG_P00_VIP_CNTRL_2_swap_e        = BINARY(0,1,1,1, 0,0,0,0),
    E_MASKREG_P00_VIP_CNTRL_2_mirr_f        = BINARY(0,0,0,0, 1,0,0,0),
    E_MASKREG_P00_VIP_CNTRL_2_swap_f        = BINARY(0,0,0,0, 0,1,1,1),

    E_MASKREG_P00_VIP_CNTRL_3_edge          = BINARY(1,0,0,0, 0,0,0,0),
    E_MASKREG_P00_VIP_CNTRL_3_sp_sync       = BINARY(0,0,1,1, 0,0,0,0),
    E_MASKREG_P00_VIP_CNTRL_3_emb           = BINARY(0,0,0,0, 1,0,0,0),
    E_MASKREG_P00_VIP_CNTRL_3_v_tgl         = BINARY(0,0,0,0, 0,1,0,0),
    E_MASKREG_P00_VIP_CNTRL_3_h_tgl         = BINARY(0,0,0,0, 0,0,1,0),
    E_MASKREG_P00_VIP_CNTRL_3_x_tgl         = BINARY(0,0,0,0, 0,0,0,1),

    E_MASKREG_P00_VIP_CNTRL_4_tst_pat       = BINARY(1,0,0,0, 0,0,0,0),
    E_MASKREG_P00_VIP_CNTRL_4_tst_656       = BINARY(0,1,0,0, 0,0,0,0),
    E_MASKREG_P00_VIP_CNTRL_4_ccir656       = BINARY(0,0,0,1, 0,0,0,0),
    E_MASKREG_P00_VIP_CNTRL_4_blankit       = BINARY(0,0,0,0, 1,1,0,0),
    E_MASKREG_P00_VIP_CNTRL_4_blc           = BINARY(0,0,0,0, 0,0,1,1),

    E_MASKREG_P00_VIP_CNTRL_5_sp_cnt        = BINARY(0,0,0,0, 0,1,1,0),
    E_MASKREG_P00_VIP_CNTRL_5_ckcase        = BINARY(0,0,0,0, 0,0,0,1),

    E_MASKREG_P00_MAT_CONTRL_mat_bp         = BINARY(0,0,0,0, 0,1,0,0),
    E_MASKREG_P00_MAT_CONTRL_mat_sc         = BINARY(0,0,0,0, 0,0,1,1),

    E_MASKREG_P00_VIDFORMAT_vidformat       = BINARY(0,0,0,1, 1,1,1,1),

    E_MASKREG_P00_TBG_CNTRL_0_sync_once     = BINARY(1,0,0,0, 0,0,0,0),
    E_MASKREG_P00_TBG_CNTRL_0_sync_mthd     = BINARY(0,1,0,0, 0,0,0,0),
    E_MASKREG_P00_TBG_CNTRL_0_frame_dis     = BINARY(0,0,1,0, 0,0,0,0),

    E_MASKREG_P00_TBG_CNTRL_1_dwin_dis      = BINARY(0,1,0,0, 0,0,0,0),
    E_MASKREG_P00_TBG_CNTRL_1_vhx_ext       = BINARY(0,0,1,1, 1,0,0,0),
    E_MASKREG_P00_TBG_CNTRL_1_vhx_ext_vs    = BINARY(0,0,1,0, 0,0,0,0),
    E_MASKREG_P00_TBG_CNTRL_1_vhx_ext_hs    = BINARY(0,0,0,1, 0,0,0,0),
    E_MASKREG_P00_TBG_CNTRL_1_vhx_ext_de    = BINARY(0,0,0,0, 1,0,0,0),
    E_MASKREG_P00_TBG_CNTRL_1_vh_tgl        = BINARY(0,0,0,0, 0,1,1,1),
    E_MASKREG_P00_TBG_CNTRL_1_vh_tgl_2      = BINARY(0,0,0,0, 0,1,0,0),
    E_MASKREG_P00_TBG_CNTRL_1_vh_tgl_1      = BINARY(0,0,0,0, 0,0,1,0),
    E_MASKREG_P00_TBG_CNTRL_1_vh_tgl_0      = BINARY(0,0,0,0, 0,0,0,1),

    E_MASKREG_P00_HVF_CNTRL_0_sm            = BINARY(1,0,0,0, 0,0,0,0),
    E_MASKREG_P00_HVF_CNTRL_0_rwb           = BINARY(0,1,0,0, 0,0,0,0),
    E_MASKREG_P00_HVF_CNTRL_0_prefil        = BINARY(0,0,0,0, 1,1,0,0),
    E_MASKREG_P00_HVF_CNTRL_0_intpol        = BINARY(0,0,0,0, 0,0,1,1),

    E_MASKREG_P00_HVF_CNTRL_1_semi_planar   = BINARY(0,1,0,0, 0,0,0,0),
    E_MASKREG_P00_HVF_CNTRL_1_pad           = BINARY(0,0,1,1, 0,0,0,0),
    E_MASKREG_P00_HVF_CNTRL_1_vqr           = BINARY(0,0,0,0, 1,1,0,0),
    E_MASKREG_P00_HVF_CNTRL_1_yuvblk        = BINARY(0,0,0,0, 0,0,1,0),
    E_MASKREG_P00_HVF_CNTRL_1_for           = BINARY(0,0,0,0, 0,0,0,1),

    E_MASKREG_P00_TIMER_H_im_clksel         = BINARY(1,0,0,0, 0,0,0,0),
    E_MASKREG_P00_TIMER_H_wd_clksel         = BINARY(0,1,0,0, 0,0,0,0),
    E_MASKREG_P00_TIMER_H_tim_h             = BINARY(0,0,0,0, 0,0,1,1),

    E_MASKREG_P00_DEBUG_PROBE_bypass        = BINARY(0,0,0,0, 1,0,0,0),
    E_MASKREG_P00_DEBUG_PROBE_vid_de        = BINARY(0,0,0,0, 0,1,0,0),
    E_MASKREG_P00_DEBUG_PROBE_di_de         = BINARY(0,0,0,0, 0,0,1,0),
    E_MASKREG_P00_DEBUG_PROBE_woo_en        = BINARY(0,0,0,0, 0,0,0,1),

    E_MASKREG_P00_AIP_CLKSEL_sel_aip_SHIFT  = 3,
    E_MASKREG_P00_AIP_CLKSEL_sel_aip        = BINARY(0,0,0,1, 1,0,0,0),
    E_MASKREG_P00_AIP_CLKSEL_sel_pol_clk    = BINARY(0,0,0,0, 0,1,0,0),
    E_MASKREG_P00_AIP_CLKSEL_sel_fs         = BINARY(0,0,0,0, 0,0,1,1),
#ifndef TMFL_TDA9981_SUPPORT
    E_MASKREG_P01_SC_VIDFORMAT_lut_sel      = BINARY(1,1,0,0, 0,0,0,0),
    E_MASKREG_P01_SC_VIDFORMAT_vid_format_o = BINARY(0,0,1,1, 1,0,0,0),
    E_MASKREG_P01_SC_VIDFORMAT_vid_format_i = BINARY(0,0,0,0, 0,1,1,1),

    E_MASKREG_P01_SC_CNTRL_il_out_on        = BINARY(0,0,0,0, 1,0,0,0),
    E_MASKREG_P01_SC_CNTRL_phases_v         = BINARY(0,0,0,0, 0,1,0,0),
    E_MASKREG_P01_SC_CNTRL_vs_on            = BINARY(0,0,0,0, 0,0,1,0),
    E_MASKREG_P01_SC_CNTRL_deil_on          = BINARY(0,0,0,0, 0,0,0,1),

    E_MASKREG_P01_VIDFORMAT_vidformat       = BINARY(0,0,0,0, 0,1,1,1),

    E_MASKREG_P01_TBG_CNTRL_0_sync_once     = BINARY(1,0,0,0, 0,0,0,0),
    E_MASKREG_P01_TBG_CNTRL_0_sync_mthd     = BINARY(0,1,0,0, 0,0,0,0),
    E_MASKREG_P01_TBG_CNTRL_0_frame_dis     = BINARY(0,0,1,0, 0,0,0,0),
    E_MASKREG_P01_TBG_CNTRL_0_top_ext       = BINARY(0,0,0,0, 1,0,0,0),
    E_MASKREG_P01_TBG_CNTRL_0_de_ext        = BINARY(0,0,0,0, 0,1,0,0),
    E_MASKREG_P01_TBG_CNTRL_0_top_sel       = BINARY(0,0,0,0, 0,0,1,0),
    E_MASKREG_P01_TBG_CNTRL_0_top_tgl       = BINARY(0,0,0,0, 0,0,0,1),
#endif /* TMFL_TDA9981_SUPPORT */
    E_MASKREG_P02_PLL_SERIAL_1_srl_man_ip   = BINARY(0,1,0,0, 0,0,0,0),
    E_MASKREG_P02_PLL_SERIAL_1_srl_reg_ip   = BINARY(0,0,1,1, 1,0,0,0),
    E_MASKREG_P02_PLL_SERIAL_1_srl_iz       = BINARY(0,0,0,0, 0,1,1,0),
    E_MASKREG_P02_PLL_SERIAL_1_srl_fdn      = BINARY(0,0,0,0, 0,0,0,1),

    E_MASKREG_P02_PLL_SERIAL_2_srl_pr       = BINARY(1,1,1,1, 0,0,0,0),
    E_MASKREG_P02_PLL_SERIAL_2_srl_nosc     = BINARY(0,0,0,0, 0,0,1,1),

    E_MASKREG_P02_PLL_SERIAL_3_srl_pxin_sel = BINARY(0,0,0,1, 0,0,0,0),
    E_MASKREG_P02_PLL_SERIAL_3_srl_de       = BINARY(0,0,0,0, 0,0,1,0),
    E_MASKREG_P02_PLL_SERIAL_3_srl_ccir     = BINARY(0,0,0,0, 0,0,0,1),

    E_MASKREG_P02_SERIALIZER_srl_phase3     = BINARY(1,1,1,1, 0,0,0,0),
    E_MASKREG_P02_SERIALIZER_srl_phase2     = BINARY(0,0,0,0, 1,1,1,1),

    E_MASKREG_P02_BUFFER_OUT_srl_force      = BINARY(0,0,0,0, 1,1,0,0),
    E_MASKREG_P02_BUFFER_OUT_srl_clk        = BINARY(0,0,0,0, 0,0,1,1),

    E_MASKREG_P02_PLL_SCG1_scg_fdn          = BINARY(0,0,0,0, 0,0,0,1),

    E_MASKREG_P02_PLL_SCG2_bypass_scg       = BINARY(1,0,0,0, 0,0,0,0),
    E_MASKREG_P02_PLL_SCG2_selpllclkin      = BINARY(0,0,0,1, 0,0,0,0),
    E_MASKREG_P02_PLL_SCG2_scg_nosc         = BINARY(0,0,0,0, 0,0,1,1),

    E_MASKREG_P02_PLL_DE_bypass_pllde       = BINARY(1,0,0,0, 0,0,0,0),
    E_MASKREG_P02_PLL_DE_pllde_nosc         = BINARY(0,0,1,1, 0,0,0,0),
    E_MASKREG_P02_PLL_DE_pllde_iz           = BINARY(0,0,0,0, 0,1,1,0),
    E_MASKREG_P02_PLL_DE_pllde_fdn          = BINARY(0,0,0,0, 0,0,0,1),

    E_MASKREG_P02_CCIR_DIV_refdiv2          = BINARY(0,0,0,0, 0,0,0,1),

    E_MASKREG_P02_VAI_PLL_pllde_hvp         = BINARY(0,1,0,0, 0,0,0,0),
    E_MASKREG_P02_VAI_PLL_pllscg_hvp        = BINARY(0,0,1,0, 0,0,0,0),
    E_MASKREG_P02_VAI_PLL_pllsrl_hvp        = BINARY(0,0,0,1, 0,0,0,0),
    E_MASKREG_P02_VAI_PLL_pllde_lock        = BINARY(0,0,0,0, 0,1,0,0),
    E_MASKREG_P02_VAI_PLL_pllscg_lock       = BINARY(0,0,0,0, 0,0,1,0),
    E_MASKREG_P02_VAI_PLL_pllsrl_lock       = BINARY(0,0,0,0, 0,0,0,1),

    E_MASKREG_P02_AUDIO_DIV_audio_div       = BINARY(0,0,0,0, 0,1,1,1),

    E_MASKREG_P02_TEST1_tstserphoe          = BINARY(0,0,0,1, 0,0,0,0),
    E_MASKREG_P02_TEST1_tst_nosc            = BINARY(0,0,0,0, 0,0,1,0),
    E_MASKREG_P02_TEST1_tst_hvp             = BINARY(0,0,0,0, 0,0,0,1),

    E_MASKREG_P02_TEST2_pwd1v8              = BINARY(0,0,0,0, 0,0,1,0),
    E_MASKREG_P02_TEST2_divtestoe           = BINARY(0,0,0,0, 0,0,0,1),

    E_MASKREG_P02_SEL_CLK_ena_sc_clk        = BINARY(0,0,0,0, 1,0,0,0),
    E_MASKREG_P02_SEL_CLK_sel_vrf_clk       = BINARY(0,0,0,0, 0,1,1,0),
    E_MASKREG_P02_SEL_CLK_sel_clk1          = BINARY(0,0,0,0, 0,0,0,1),

    E_MASKREG_P11_AIP_CNTRL_0_rst_cts       = BINARY(0,1,0,0, 0,0,0,0),
    E_MASKREG_P11_AIP_CNTRL_0_acr_man       = BINARY(0,0,1,0, 0,0,0,0),
    E_MASKREG_P11_AIP_CNTRL_0_layout        = BINARY(0,0,0,0, 0,1,0,0),
    E_MASKREG_P11_AIP_CNTRL_0_swap          = BINARY(0,0,0,0, 0,0,1,0),
    E_MASKREG_P11_AIP_CNTRL_0_rst_fifo      = BINARY(0,0,0,0, 0,0,0,1),

    E_MASKREG_P11_GC_AVMUTE_set_mute        = BINARY(0,0,0,0, 0,0,1,0),
    E_MASKREG_P11_GC_AVMUTE_clr_mute        = BINARY(0,0,0,0, 0,0,0,1),
    E_MASKREG_P11_GC_AVMUTE_setclr_mute     = BINARY(0,0,0,0, 0,0,1,1),

    E_MASKREG_P11_CTS_N_m_sel               = BINARY(0,0,1,1, 0,0,0,0),
    E_MASKREG_P11_CTS_N_k_sel               = BINARY(0,0,0,0, 0,1,1,1),

    E_MASKREG_P11_ENC_CNTRL_ctl_code        = BINARY(0,0,0,0, 1,1,0,0),
    E_MASKREG_P11_ENC_CNTRL_rst_sel         = BINARY(0,0,0,0, 0,0,1,0),
    E_MASKREG_P11_ENC_CNTRL_rst_enc         = BINARY(0,0,0,0, 0,0,0,1),
    E_MASKREG_P11_DIP_FLAGS_force_null      = BINARY(1,0,0,0, 0,0,0,0),
    E_MASKREG_P11_DIP_FLAGS_null            = BINARY(0,1,0,0, 0,0,0,0),
    E_MASKREG_P11_DIP_FLAGS_acp             = BINARY(0,0,0,1, 0,0,0,0),
    E_MASKREG_P11_DIP_FLAGS_isrc2           = BINARY(0,0,0,0, 1,0,0,0),
    E_MASKREG_P11_DIP_FLAGS_isrc1           = BINARY(0,0,0,0, 0,1,0,0),
    E_MASKREG_P11_DIP_FLAGS_gc              = BINARY(0,0,0,0, 0,0,1,0),
    E_MASKREG_P11_DIP_FLAGS_acr             = BINARY(0,0,0,0, 0,0,0,1),

    E_MASKREG_P11_DIP_IF_FLAGS_if5          = BINARY(0,0,1,0, 0,0,0,0),
    E_MASKREG_P11_DIP_IF_FLAGS_if4          = BINARY(0,0,0,1, 0,0,0,0),
    E_MASKREG_P11_DIP_IF_FLAGS_if3          = BINARY(0,0,0,0, 1,0,0,0),
    E_MASKREG_P11_DIP_IF_FLAGS_if2          = BINARY(0,0,0,0, 0,1,0,0),
    E_MASKREG_P11_DIP_IF_FLAGS_if1          = BINARY(0,0,0,0, 0,0,1,0),

    E_MASKREG_P12_OTP_CNTRL_start_rd        = BINARY(0,0,0,0, 1,0,0,0),
    E_MASKREG_P12_OTP_CNTRL_start_dl        = BINARY(0,0,0,0, 0,1,0,0),
    E_MASKREG_P12_OTP_CNTRL_bch_act         = BINARY(0,0,0,0, 0,0,1,0),
    E_MASKREG_P12_OTP_CNTRL_start_pc        = BINARY(0,0,0,0, 0,0,0,1),

    E_MASKREG_P12_OTP_STATUS_rd_d_status    = BINARY(1,1,0,0, 0,0,0,0),
    E_MASKREG_P12_OTP_STATUS_rd_c_status    = BINARY(0,0,1,1, 0,0,0,0),
    E_MASKREG_P12_OTP_STATUS_rd_b_status    = BINARY(0,0,0,0, 1,1,0,0),
    E_MASKREG_P12_OTP_STATUS_rd_a_status    = BINARY(0,0,0,0, 0,0,1,1),

#ifndef NO_HDCP
    E_MASKREG_P12_HDCP_CTRL_ri_rate         = BINARY(1,1,0,0, 0,0,0,0),
    E_MASKREG_P12_HDCP_CTRL_keyd            = BINARY(0,0,0,0, 0,0,1,0),
    E_MASKREG_P12_HDCP_CTRL_hdcp_ena        = BINARY(0,0,0,0, 0,0,0,1),

#ifdef BCAPS_REPEATER
    E_MASKREG_P12_HDCP_BCAPS_repeater       = BINARY(0,1,0,0, 0,0,0,0),
#endif /* BCAPS_REPEATER */
    E_MASKREG_P12_HDCP_BCAPS_1_1            = BINARY(0,0,0,0, 0,0,1,0),

    E_MASKREG_P12_HDCP_TX0_mst_en           = BINARY(0,0,0,0, 1,0,0,0),
    E_MASKREG_P12_HDCP_TX0_ainfo_1          = BINARY(0,0,0,0, 0,1,0,0),
    E_MASKREG_P12_HDCP_TX0_ainfo_rdy        = BINARY(0,0,0,0, 0,0,1,0),
    E_MASKREG_P12_HDCP_TX0_sr_hdcp          = BINARY(0,0,0,0, 0,0,0,1),

    E_MASKREG_P12_HDCP_TX1_ksvl_nok         = BINARY(0,0,0,0, 1,0,0,0),
    E_MASKREG_P12_HDCP_TX1_ksvl_ok          = BINARY(0,0,0,0, 0,1,0,0),
    E_MASKREG_P12_HDCP_TX1_bksv_nok         = BINARY(0,0,0,0, 0,0,1,0),
    E_MASKREG_P12_HDCP_TX1_bksv_ok          = BINARY(0,0,0,0, 0,0,0,1),

    E_MASKREG_P12_HDCP_TX2_slvaddr          = BINARY(0,0,0,0, 0,1,0,0),
    E_MASKREG_P12_HDCP_TX2_selrptsrc        = BINARY(0,0,0,0, 0,0,1,0),
    E_MASKREG_P12_HDCP_TX2_rpt_force        = BINARY(0,0,0,0, 0,0,0,1),

    E_MASKREG_P12_HDCP_BCAPS_RX_hdmi        = BINARY(1,0,0,0, 0,0,0,0),
    E_MASKREG_P12_HDCP_BCAPS_RX_repeater    = BINARY(0,1,0,0, 0,0,0,0),
    E_MASKREG_P12_HDCP_BCAPS_RX_ready       = BINARY(0,0,1,0, 0,0,0,0),
    E_MASKREG_P12_HDCP_BCAPS_RX_fast        = BINARY(0,0,0,1, 0,0,0,0),
    E_MASKREG_P12_HDCP_BCAPS_RX_bit3        = BINARY(0,0,0,0, 1,0,0,0),
    E_MASKREG_P12_HDCP_BCAPS_RX_bit2        = BINARY(0,0,0,0, 0,1,0,0),
    E_MASKREG_P12_HDCP_BCAPS_RX_1_1         = BINARY(0,0,0,0, 0,0,1,0),
    E_MASKREG_P12_HDCP_BCAPS_RX_fast_ra     = BINARY(0,0,0,0, 0,0,0,1),
#endif /* NO_HDCP */

    E_MASKREG_P12_HDCP_TX33_ac_not          = BINARY(0,0,1,0, 0,0,0,0),
    E_MASKREG_P12_HDCP_TX33_ctl_sel         = BINARY(0,0,0,1, 0,0,0,0),
    E_MASKREG_P12_HDCP_TX33_sys_req         = BINARY(0,0,0,0, 1,0,0,0),
    E_MASKREG_P12_HDCP_TX33_eess            = BINARY(0,0,0,0, 0,1,0,0),
    E_MASKREG_P12_HDCP_TX33_hdmi            = BINARY(0,0,0,0, 0,0,1,0),
    E_MASKREG_P12_HDCP_TX33_avmute          = BINARY(0,0,0,0, 0,0,0,1)
};
#undef BINARY

/**
 * An enum for the video formats used in the E_REG_P00_VIDFORMAT_W register
 */
enum _eRegVfmt
{
    E_REGVFMT_640x480p_60Hz     = 0,
    E_REGVFMT_720x480p_60Hz     ,
    E_REGVFMT_1280x720p_60Hz    ,
    E_REGVFMT_1920x1080i_60Hz   ,
    E_REGVFMT_720x480i_60Hz     ,
    E_REGVFMT_720x240p_60Hz     ,
    E_REGVFMT_1920x1080p_60Hz   ,
    E_REGVFMT_720x576p_50Hz     ,
    E_REGVFMT_1280x720p_50Hz    ,
    E_REGVFMT_1920x1080i_50Hz   ,
    E_REGVFMT_720x576i_50Hz     ,
    E_REGVFMT_720x288p_50Hz     ,
    E_REGVFMT_1920x1080p_50Hz   ,
	E_REGVFMT_1920x1080p_24Hz   ,
	E_REGVFMT_1920x1080p_25Hz   ,
	E_REGVFMT_1920x1080p_30Hz   ,
#ifdef FORMAT_PC
    E_REGVFMT_640x480p_72Hz     ,
    E_REGVFMT_640x480p_75Hz     ,
    E_REGVFMT_640x480p_85Hz     ,
    E_REGVFMT_800x600p_60Hz     ,
    E_REGVFMT_800x600p_72Hz     ,
    E_REGVFMT_800x600p_75Hz     ,
    E_REGVFMT_800x600p_85Hz     ,
    E_REGVFMT_1024x768p_60Hz    ,
    E_REGVFMT_1024x768p_70Hz    ,
    E_REGVFMT_1024x768p_75Hz    ,
    E_REGVFMT_1280x768p_60Hz    ,
    E_REGVFMT_1280x1024p_60Hz   ,
	E_REGVFMT_1280x1024p_75Hz   ,
    E_REGVFMT_1360x768p_60Hz    ,
    E_REGVFMT_1400x1050p_60Hz   ,
    E_REGVFMT_1600x1200p_60Hz   ,
#endif /*FORMAT_PC*/

    E_REGVFMT_INVALID           ,
    E_REGVFMT_NUM_TV            = E_REGVFMT_1920x1080p_30Hz + 1,
	E_REGVFMT_NUM_TV_NO_REG		= E_REGVFMT_NUM_TV - E_REGVFMT_1920x1080p_24Hz,
	E_REGVFMT_FIRST_TV_NO_REG	= E_REGVFMT_1920x1080p_24Hz,
    E_REGVFMT_NUM               = E_REGVFMT_INVALID
    
#ifdef FORMAT_PC
   ,E_REGVFMT_FIRST_PC_FORMAT = E_REGVFMT_640x480p_72Hz,
    E_REGVFMT_NUM_PC          = E_REGVFMT_1600x1200p_60Hz - E_REGVFMT_NUM_TV + 1

#endif

};

/**
 * An enum for the video input formats used in the E_REG_P01_SC_VIDFORMAT_W
 * register
 */
enum _eRegVfmtScIn
{
    E_REGVFMT_SCIN_480i_60Hz    = 0,
    E_REGVFMT_SCIN_576i_50Hz    = 1,
    E_REGVFMT_SCIN_480p_60Hz    = 2,
    E_REGVFMT_SCIN_576p_50Hz    = 3,
    E_REGVFMT_SCIN_MAX          = 3,
    E_REGVFMT_SCIN_NUM          = 4,
    E_REGVFMT_SCIN_INVALID      = 4
};
 
/**
 * An enum for the video output formats used in the E_REG_P01_SC_VIDFORMAT_W
 * register
 */
enum _eRegVfmtScOut
{
    E_REGVFMT_SCOUT_480p_60Hz       = 0,
    E_REGVFMT_SCOUT_576ip_50Hz      = 1,
    E_REGVFMT_SCOUT_720p_50Hz_60Hz  = 2,
    E_REGVFMT_SCOUT_1080i_50Hz_60Hz = 3,
    E_REGVFMT_SCOUT_MAX             = 3,
    E_REGVFMT_SCOUT_NUM             = 4,
    E_REGVFMT_SCOUT_INVALID         = 4
};
 
/**
 * An enum to list all supported pixel clock frequencies in kHz
 */
enum _ePixClk
{
    E_PIXCLK_25175      = 0,
    E_PIXCLK_25200      = 1,
    E_PIXCLK_27000      = 2,
    E_PIXCLK_27027      = 3,
    E_PIXCLK_54000      = 4,
    E_PIXCLK_54054      = 5,
    E_PIXCLK_74175      = 6,
    E_PIXCLK_74250      = 7,
    E_PIXCLK_148350     = 8,
    E_PIXCLK_148500     = 9,
#ifndef FORMAT_PC
    E_PIXCLK_MAX        = 9,
    E_PIXCLK_INVALID    = 10,
    E_PIXCLK_NUM        = 10
#else /* FORMAT_PC */
    E_PIXCLK_31500      = 10,
    E_PIXCLK_36000      = 11,
    E_PIXCLK_40000      = 12,
    E_PIXCLK_49500      = 13,
    E_PIXCLK_50000      = 14,
    E_PIXCLK_56250      = 15,
    E_PIXCLK_65000      = 16,
    E_PIXCLK_75000      = 17,
    E_PIXCLK_78750      = 18,
    E_PIXCLK_79500      = 19,
    E_PIXCLK_85500      = 20,
    E_PIXCLK_108000     = 21,
    E_PIXCLK_121750     = 22,
	E_PIXCLK_135000		= 23,
    E_PIXCLK_162000     = 24,
    E_PIXCLK_MAX        = 24,
    E_PIXCLK_INVALID    = 25,
    E_PIXCLK_NUM        = 25
#endif /* FORMAT_PC */
};

/**
 * An enum to list all device version codes supported by this driver.
 * The values form a list, with non-zero version codes first in any order.
 * The E_DEV_VERSION_END_LIST must be the last value in the list.
 */
enum _eDevVersion
{
    E_DEV_VERSION_N2            = 0x40,
    E_DEV_VERSION_N3            = 0x41,
#ifdef TMFL_TDA9981_SUPPORT
    E_DEV_VERSION_N4            = 0x00,
    E_DEV_VERSION_LIST_END      = 0xCF,
#else /* TMFL_TDA9981_SUPPORT */
    E_DEV_VERSION_N4            = 0x42,
    E_DEV_VERSION_LIST_END      = 0x00,
#endif /* TMFL_TDA9981_SUPPORT */
    E_DEV_VERSION_LIST_NUM      = 4         /**< Number of items in list */
};

/**
 * An enum to list all CEA Data Block Tag Codes we may find in EDID.
 */
enum _eCeaBlockTags
{
    E_CEA_RESERVED_0     = 0x00,
    E_CEA_AUDIO_BLOCK    = 0x01,
    E_CEA_VIDEO_BLOCK    = 0x02,
    E_CEA_VSDB           = 0x03,
    E_CEA_SPEAKER_ALLOC  = 0x04,
    E_CEA_VESA_DTC       = 0x05,
    E_CEA_RESERVED_6     = 0x06,
    E_CEA_EXTENDED       = 0x07
};

/** A typedef for colourspace values */
typedef enum
{
    HDMITX_CS_RGB_FULL              = 0,    /**< RGB Full (PC) */
    HDMITX_CS_RGB_LIMITED           = 1,    /**< RGB Limited (TV) */
    HDMITX_CS_YUV_ITU_BT601         = 2,    /**< YUV ITUBT601 (SDTV) */
    HDMITX_CS_YUV_ITU_BT709         = 3,    /**< YUV ITUBT709 (HDTV) */
    HDMITX_CS_NUM                   = 4     /**< Number Cspaces we support */
} tmbslHdmiTxColourspace_t;

/** Matrix register block size */
#define MATRIX_PRESET_SIZE		    31

/** Matrix register block size */
#define MATRIX_PRESET_QTY		    12

/** The enum that vectors us into the MatrixPreset table */
enum _eMatrixPresetIndex
{
    E_MATRIX_RGBF_2_RGBL    = 0,
    E_MATRIX_RGBF_2_BT601   = 1,
    E_MATRIX_RGBF_2_BT709   = 2,
    E_MATRIX_RGBL_2_RGBF    = 3,
    E_MATRIX_RGBL_2_BT601   = 4,
    E_MATRIX_RGBL_2_BT709   = 5,
    E_MATRIX_BT601_2_RGBF   = 6,
    E_MATRIX_BT601_2_RGBL   = 7,
    E_MATRIX_BT601_2_BT709  = 8,
    E_MATRIX_BT709_2_RGBF   = 9,
    E_MATRIX_BT709_2_RGBL   = 10,
    E_MATRIX_BT709_2_BT601  = 11
};   

/** EDID block size */
#define EDID_BLOCK_SIZE		    128

/** number of detailed timing descriptor stored in BSL */
#define NUMBER_DTD_STORED       10

/** EDID DTD block descriptor size */
#define EDID_DTD_BLK_SIZE       0x12


/** EDID i2c address */
#define DDC_EDID_ADDRESS        0xA0

/** EDID alternate i2c address */
#define DDC_EDID_ADDRESS_ALT    0xA2

/** EDID Segment Pointer address */
#define DDC_SGMT_PTR_ADDRESS    0x60

/** EDID block 0 parse start point */
#define EDID_BLK0_BASE          0x36

/** EDID block 0 descriptor size */
#define EDID_BLK0_SIZE          0x12

/** EDID block 0 extension block count */
#define EDID_BLK0_EXT_CNT       0x7E

/** EDID extension block parse start point */
#define EDID_BLK_EXT_BASE       0x04

/** CEA extension block type */
#define EDID_CEA_EXTENSION      0x02

/**
 * \brief A structure type to form arrays that hold a series of registers and
 * values
 */
typedef struct _tmHdmiTxRegVal_t
{
    UInt16 Reg;
    UInt8  Val;
} tmHdmiTxRegVal_t;

/**
 * \brief A structure type to form arrays that hold a series of registers,
 * bitfield masks and bitfield values
 */
typedef struct _tmHdmiTxRegMaskVal_t
{
    UInt16 Reg;
    UInt8  Mask;
    UInt8  Val;
} tmHdmiTxRegMaskVal_t;

/**
 * \brief A function pointer type to call a function and return a result
 */
typedef tmErrorCode_t (FUNC_PTR * ptmHdmiTxFunc_t) (tmUnitSelect_t txUnit);

/**
 * \brief The structure of a TM998x object, one per device unit
 ****************************************************************************
 ** Copy changes to kTestDisNames tab in "HDMI Driver - Register List.xls" **
 ****************************************************************************
 */
typedef struct _tmHdmiTxobject_t
{
    /** Component State */
    tmbslHdmiTxState_t state;

    /** Count of events ignored by setState() */
    UInt8 nIgnoredEvents;

    /** Device unit number */
    tmUnitSelect_t txUnit;

    /** Device I2C slave address */
    UInt8 uHwAddress;

    /** System function to write to the I2C driver */
    ptmbslHdmiTxSysFunc_t sysFuncWrite;

    /** System function to read from the I2C driver */
    ptmbslHdmiTxSysFunc_t sysFuncRead;

    /** System function to read EDID blocks via the I2C driver */
    ptmbslHdmiTxSysFuncEdid_t sysFuncEdidRead;

    /** System function to run a timer */
    ptmbslHdmiTxSysFuncTimer_t sysFuncTimer;

    /** Array of registered interrupt handler callback functions */
    ptmbslHdmiTxCallback_t funcIntCallbacks[HDMITX_CALLBACK_INT_NUM];

    /** Device version(s) supported by this component */
    UInt8 uSupportedVersions[E_DEV_VERSION_LIST_NUM];

    /** Device version read from register, with features flags masked out */
    UInt8 uDeviceVersion;

    /** Device features flags read from version register */
    UInt8 uDeviceFeatures;

    /** The device's power state */
    tmPowerState_t ePowerState;

    /*=== E D I D ===*/

    /** EDID Use alternative i2c address flag */
    Bool bEdidAlternateAddr;

    /** The sink type set by the user (may or may not match EdidSinkType) */
    tmbslHdmiTxSinkType_t sinkType;

    /** EDID Sink Type for receiver */
    tmbslHdmiTxSinkType_t EdidSinkType;

    /** EDID AI_Support from HDMI VSDB */
    Bool EdidSinkAi;

    /** EDID CEA flags from extension block */
    UInt8 EdidCeaFlags;

    /** EDID Read Status */
    UInt8 EdidStatus;

    /** NB DTD stored in EdidDTD */
    UInt8 NbDTDStored;


    /** EDID  Detailed Timing Descriptor */
    tmbslHdmiTxEdidDtd_t EdidDTD[NUMBER_DTD_STORED];

    /** EDID First Moniteur descriptor */
    tmbslHdmiTxEdidFirstMD_t EdidFirstMonitorDescriptor;

    /** EDID Second Moniteur descriptor */
    tmbslHdmiTxEdidSecondMD_t EdidSecondMonitorDescriptor;

    /** EDID Other Moniteur descriptor */
    tmbslHdmiTxEdidOtherMD_t EdidOtherMonitorDescriptor;



    /** EDID supported Short Video Descriptors */
    UInt8 EdidVFmts[HDMI_TX_SVD_MAX_CNT];

    /** Counter for supported short video descriptors */
    UInt8 EdidSvdCnt;

    /** EDID supported Short Audio Descriptors */
    tmbslHdmiTxEdidSad_t EdidAFmts[HDMI_TX_SAD_MAX_CNT];

    /** Counter for supported short audio descriptors */
    UInt8 EdidSadCnt;

    /** EDID block workspace */
    UInt8 EdidBlock[EDID_BLOCK_SIZE];

    /** EDID Block Count */
    UInt8 EdidBlockCnt;

    /** CEC Source Address read from EDID as "A.B.C.D" nibbles */
    UInt16 EdidSourceAddress;

    /** EDID Basic Display Parameters */
    tmbslHdmiTxEdidBDParam_t EDIDBasicDisplayParam;


#ifndef NO_HDCP
    /*=== H D C P === */

    /** Configured DDC I2C slave address */
    UInt8 HdcpSlaveAddress;

    /** Configured mode of our transmitter device */
    tmbslHdmiTxHdcpTxMode_t HdcpTxMode;

    /** Configured HDCP options */
    tmbslHdmiTxHdcpOptions_t HdcpOptions;

    /** BCAPS read from sink */
    UInt8 HdcpBcaps;

    /** BSTATUS read from sink */
    UInt16 HdcpBstatus;

    /** Device value generated for Ri=Ri' comparison */
    UInt16 HdcpRi;

    /** Device HDCP FSM state */
    UInt8 HdcpFsmState;

    /** Device failure state that caused T0 interrupt */
    UInt8 HdcpT0FailState;

    /* Key Selection Vector for transmitter */
    UInt8 HdcpAksv[HDMITX_KSV_BYTES_PER_DEVICE];

    /** Local callback scheduled to be called after HdcpFuncRemainingMs */
    ptmHdmiTxFunc_t HdcpFuncScheduled;

    /** Period in ms after which to call HdcpFuncScheduled; 0=disabled */
    UInt16 HdcpFuncRemainingMs;

    /** Configured period in ms after which to do HDCP check */
    UInt16 HdcpCheckIntervalMs;

    /** Period in ms until next HDCP check */
    UInt16 HdcpCheckRemainingMs;

    /** Number of the HDCP check since HDCP was started; 0=disabled */
    UInt8 HdcpCheckNum;

    /** Configured number of HDCP checks to do after HDCP is started */
    UInt8 HdcpChecksToDo;
#endif /* NO_HDCP */

    /*=== V I D E O ===*/

    /** Current EIA/CEA video input format */
    tmbslHdmiTxVidFmt_t vinFmt;

    /** Current EIA/CEA video output format */
    tmbslHdmiTxVidFmt_t voutFmt;
    
    /** Current pix Rate*/
    tmbslHdmiTxPixRate_t pixRate;

    /** Video input mode */
    tmbslHdmiTxVinMode_t vinMode;

    /** Video output mode */
    tmbslHdmiTxVoutMode_t voutMode;

    /** Vertical output frequency */
    tmbslHdmiTxVfreq_t voutFreq;

    /** Current scaler mode */
    tmbslHdmiTxScaMode_t scaMode;

    /** Current upsampler mode */
    tmbslHdmiTxUpsampleMode_t upsampleMode;

    /** Current pixel repetition count */
    UInt8 pixelRepeatCount;

    /** Status of hot plug detect pin last read at interrupt */
    tmbslHdmiTxHotPlug_t hotPlugStatus;

#ifdef TMFL_TDA9981_SUPPORT
#ifdef TMFL_RX_SENSE_ON
    /** Status of rx sense detect pin last read at interrupt */
	tmbslHdmiTxRxSense_t rxSenseStatus;
#endif /* TMFL_RX_SENSE_ON */
#endif /* TMFL_TDA9981_SUPPORT */

    /** Current register page */
    UInt8 curRegPage;

    /** Shadow copies of write-only registers with bitfields */
    UInt8 shadowReg[E_SNUM];

    /** TRUE: Blue screen is the previous test pattern ; FALSE: is not */
    Bool prevBluescreen;

	/** TRUE: last screen is test pattern ; FALSE: is not */
	Bool prevPattern;

    /** TRUE: Unit has been initialized; FALSE: not initialized */
    Bool bInitialized;

    tmbslHdmiTxVQR_t        dviVqr;

} tmHdmiTxobject_t;

/**
 * \The structure of registers for video format , 
 *  used by PC_formats and chip_unknown formats
 */

typedef struct _tmHdmiTxVidReg_t
{
    UInt16  nPix;
    UInt16  nLine;
    UInt8   VsLineStart;
    UInt16  VsPixStart;
    UInt8   VsLineEnd;
    UInt16  VsPixEnd;
    UInt16  HsStart;
    UInt16  HsEnd;
    UInt8   vWinStart;
    UInt16  vWinEnd;
    UInt16  DeStart;
    UInt16  DeEnd;
} tmHdmiTxVidReg_t;

/*============================================================================*/
/*                       EXTERN DATA DEFINITION                               */
/*============================================================================*/

extern RAM_DAT  tmHdmiTxobject_t gHdmiTxInstance[HDMITX_UNITS_MAX];
extern CONST_DAT UInt8 kPageIndexToPage[E_PAGE_NUM];

/*============================================================================*/
/*                       EXTERN FUNCTION PROTOTYPES                           */
/*============================================================================*/

tmErrorCode_t    checkUnitSetDis (tmUnitSelect_t txUnit,
                                  tmHdmiTxobject_t **ppDis);
tmErrorCode_t    getHwRegisters (tmHdmiTxobject_t *pDis, 
                                 UInt16 regShadPageAddr,
                                 UInt8 *pData, UInt16 lenData);
tmErrorCode_t    getHwRegister (tmHdmiTxobject_t *pDis, 
                                UInt16 regShadPageAddr,
                                UInt8 *pRegValue);
tmErrorCode_t    setHwRegisters (tmHdmiTxobject_t *pDis, 
                                 UInt16 regShadPageAddr,
                                 UInt8 *pData, UInt16 lenData);
tmErrorCode_t    setHwRegisterMsbLsb (tmHdmiTxobject_t *pDis, 
                                      UInt16 regShadPageAddr, 
                                      UInt16 regWord);
tmErrorCode_t    setHwRegister (tmHdmiTxobject_t *pDis, 
                                UInt16 regShadPageAddr,
                                UInt8 regValue);
tmErrorCode_t    setHwRegisterField (tmHdmiTxobject_t *pDis,
                                     UInt16 regShadPageAddr,
                                     UInt8 fieldMask, UInt8 fieldValue);
tmErrorCode_t    setHwRegisterFieldTable(tmHdmiTxobject_t *pDis,
                                     const tmHdmiTxRegMaskVal_t *pTable);

tmErrorCode_t    setState (tmHdmiTxobject_t *pDis, tmbslHdmiTxEvent_t event);

tmErrorCode_t    lmemcpy (void *pTable1,
                        const void *pTable2,
                        UInt Size);
tmErrorCode_t    lmemset (void *pTable1,
                        const UInt8 value,
                        UInt Size);
#ifdef __cplusplus
}
#endif

#endif /* TMBSLHDMITX_LOCAL_H */
/*============================================================================*/
/*                            END OF FILE                                     */
/*============================================================================*/
