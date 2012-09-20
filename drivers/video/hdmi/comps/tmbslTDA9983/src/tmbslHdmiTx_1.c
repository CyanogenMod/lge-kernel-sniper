/**
 * Copyright (C) 2006 Koninklijke Philips Electronics N.V., All Rights Reserved.
 * This source code and any compilation or derivative thereof is the proprietary
 * information of Koninklijke Philips Electronics N.V. and is confidential in
 * nature. Under no circumstances is this software to be  exposed to or placed
 * under an Open Source License of any type without the expressed written
 * permission of Koninklijke Philips Electronics N.V.
 *
 * \file          tmbslHdmiTx_1.c
 *
 * \version       $Revision: 85 $
 *
 * \date          $Date: 21/01/08 $
 *
 * \brief         BSL driver component for the TDA998x HDMI Transmitter
 *
 * \section refs  Reference Documents
 * HDMI Driver - Outline Architecture.doc
 * HDMI Driver - tmbslHdmiTx - SCS.doc
 *
 * \section info  Change Information
 *
 * \verbatim
   $History: tmbslHdmiTx_1.c $
 *
 * *****************  Version 85  ****************
 * User: G.Burnouf     Date: 21/01/08
 * Updated in $/Source/tmbslTDA9984/Src
 * PR1262 : Enable IT HPD during HDCP
 *
 * *****************  Version 84  ****************
 * User: G.Burnouf     Date: 07/01/08
 * Updated in $/Source/tmbslHdmiTx/src
 * PR985 : Set quantization parameter
 *
 * *****************  Version 83  ****************
 * User: B.Vereecke     Date: 29/10/07   Time: 15:11
 * Updated in $/Source/tmbslHdmiTx/src
 * PR935 : remove compilation error when "const" and 
 *         FORMAT_PC are defined
 *
 * *****************  Version 82  ****************
 * User: B.Vereecke     Date: 29/10/07   Time: 14:11
 * Updated in $/Source/tmbslHdmiTx/src
 * PR852 : remove external library dependancy
 *
 * *****************  Version 81  ****************
 * User: B.Vereecke     Date: 17/10/07   Time: 14:11
 * Updated in $/Source/tmbslHdmiTx/src
 * PR872 : add new formats, 1080p24/25/30
 *
 * *****************  Version 80  ****************
 * User: B.Vereecke     Date: 10/10/07   Time: 11:11
 * Updated in $/Source/tmbslHdmiTx/src
 * PR815 : Update bluescreen infoframes
 *         According to the output mode
 *
 * *****************  Version 79  *****************
 * User: B.Vereecke     Date: 14/09/07   Time: 14:00
 * Updated in $/Source/tmbslHdmiTx/src
 * PR696 : Fix pointer problem in case of invalid 
 *         parameters in setpixelrepeat()
 *
 * *****************  Version 78  *****************
 * User: B.Vereecke     Date: 07/09/07   Time: 16:53
 * Updated in $/Source/tmbslHdmiTx/src
 * PR670 :  Add PC Format : 1280x1024@75Hz
 *
 * *****************  Version 77  *****************
 * User: B.Vereecke     Date: 30/08/07   Time: 16:53
 * Updated in $/Source/tmbslHdmiTx/src
 * PR626 :  Update tmbslHdmiTxTestSetPattern and          
 *			tmbslHdmiTxVideoSetInOut API to save 
 *			kBypassColourProc registers each time 
 *			the video output mode is changed and
 *			restore thoses registers correctly after 
 *			pattern off. 
 *
 * *****************  Version 76  *****************
 * User: B.Vereecke     Date: 21/08/07   Time: 13:46
 * Updated in $/Source/tmbslHdmiTx/src
 * PR595 : (9981) remove timer for R0
 * 
 * *****************  Version 75  *****************
 * User: B.Vereecke     Date: 26/07/07   Time: 16:46
 * Updated in $/Source/tmbslHdmiTx/src
 * PR83 : artefact seen on screen in Scaler mode
 *		update in setScalerFormat() and 
 *		tmbslHdmiTxTestSetPattern()
 * 
 * *****************  Version 74  *****************
 * User: B.Vereecke     Date: 24/07/07   Time: 16:46
 * Updated in $/Source/tmbslHdmiTx/src
 * PR526 : in API tmbslHdmiTxHdcpCheck()
 *			Hdcp Check process is now reset after a fail.
 * 
 * *****************  Version 73  *****************
 * User: B.Vereecke     Date: 17/07/07   Time: 10:30
 * Updated in $/Source/tmbslHdmiTx/src
 * PR217 : add blue pattern functionality for 
 *			tmbslHdmiTxTestSetPattern() api
 * 
 * *****************  Version 72  *****************
 * User: J. Lamotte      Date: 22/06/07   Time: 16:00
 * Updated in $/Source/tmbslHdmiTx/src
 * PR418 : (For TDA9981 only)Enable DDC channel 
 *         only when HDCP runs
 * PR446 : kVfmtToPixClk_PC length is changed for 
 *         HDMITX_VFMT_PC_NUM+1
 * PR385 : Read Bksv for top level and repeater
 *         in tmbslHdmiTxHdcpHandleBKSV
 *         Read pnKsvDevices and BKSV list for
 *         top level and repeater (but not tested)
 *         in tmbslHdmiTxHdcpHandleSHA_1
 * 
 * *****************  Version 71  *****************
 * User: J. Lamotte      Date: 13/06/07   Time: 12:00
 * Updated in $/Source/tmbslHdmiTx/src
 * PR397 (merge with PR322) : 
 *       - Remove DLL compilation warnings 
 *       - Add definition of new 9981 API for 9983
 *         to solve .def problem
 *       (return TMBSL_ERR_HDMI_NOT_SUPPORTED for 9983)
 * 
 * *****************  Version 70  *****************
 * User: J. Lamotte      Date: 12/06/07   Time: 17:30
 * Updated in $/Source/tmbslHdmiTx/src
 * PR387 : Solve bug in setPixelRepeat
 *
 * *****************  Version 69  *****************
 * User: Burnouf         Date: 08/06/07   Time: 13:25
 * Updated in $/Source/tmbslHdmiTx/src
 * PR347 : Add new PC formats
 *
 * *****************  Version 68  *****************
 * User: J. Lamotte      Date: 21/05/07   Time: 11:09
 * Updated in $/Source/tmbslHdmiTx/src
 * PR345 (PR50) : Only for TDA9981
 *        In tmHdmiTxRegMaskVal_t kPowerOn[], set
 *        E_REG_P00_CCLK_ON_RW cclk_on bit with value 1
 *        In tmHdmiTxRegMaskVal_t kPowerOff[], set
 *        E_REG_P00_CCLK_ON_RW cclk_on bit with value 0
 *
 * *****************  Version 67  *****************
 * User: J. Lamotte      Date: 03/05/07   Time: 16:50
 * Updated in $/Source/tmbslHdmiTx/src
 * PR50 : - API tmbslHdmiTxInit modified to support
 *        TDA9981 version detection 
 *        (version = 0 for TDA9981)
 *        - Set ddc_dis to '0' after soft reset
 *
 * *****************  Version 66  *****************
 * User: Burnouf         Date: 18/04/07   Time: 13:25
 * Updated in $/Source/tmbslHdmiTx/src
 * PR50 : manage new interrupts for TDA9981
 *
 * *****************  Version 65  *****************
 * User: J. Lamotte      Date: 17/04/07   Time: 13:30
 * Updated in $/Source/tmbslHdmiTx/src
 * PR50 - disable DDC_MASTER_WORKAROUND for TDA9981 
 * 
 * *****************  Version 64  *****************
 * User: J. Lamotte      Date: 17/04/07   Time: 13:00
 * Updated in $/Source/tmbslHdmiTx/src
 * PR50 - disable scaler for TDA9981 
 *      - undefined page 1 registers for TDA9981
 * 
 * *****************  Version 63  *****************
 * User: J. Lamotte      Date: 16/04/07   Time: 11:30
 * Updated in $/Source/tmbslHdmiTx/src
 * PR50 - add video and audio port configuration for TDA9981 
 *        tmbslHdmiTxSetAudioPortConfig and 
 *        tmbslHdmiTxSetVideoPortConfig
 * 
 * *****************  Version 62  *****************
 * User: J. Lamotte   Date: 25/04/07   Time: 14:40
 * Updated in $/Source/tmbslHdmiTx/src
 * PR273 - add PLL configuration before soft reset
 *         in function tmbslHdmiTxInit
 * 
 * *****************  Version 61  *****************
 * User: J. Lamotte      Date: 13/04/07   Time: 17:30
 * Updated in $/Source/tmbslHdmiTx/src
 * PR50 - add shadow register E_REG_P00_INT_FLAGS_2_RW for TDA9981 
 * 
 * *****************  Version 60  *****************
 * User: C. Logiou      Date: 08/03/07   Time: 16:52
 * Updated in $/Source/tmbslHdmiTx/src
 * PR214 - add new input format repeated 480i/576i 
 * 
 * *****************  Version 59  *****************
 * User: burnouf      Date: 01/03/07   Time: 17:03
 * Updated in $/Source/tmbslHdmiTx/Inc
 * PR207 -call function setPixelRepeat before write 
 *        in register E_REG_P00_VIDFORMAT_W
 *  * *****************  Version 58  *****************
 * User: burnouf      Date: 06/02/07   Time: 17:03
 * Updated in $/Source/tmbslHdmiTx/Inc
 * PR49 - add PC Formats
 * 
 * *****************  Version 57  *****************
 * User: Burnouf       Date: 29/01/07   Time: 16:15
 * Updated in $/Source/tmbslHdmiTx/Src
 * PR97 Modify the PLL settings
 * 
 * *****************  Version 56  *****************
 * User: Burnouf       Date: 08/01/07   Time: 15:32
 * Updated in $/Source/tmbslHdmiTx/Src
 * PR72 Add function tmbslHdmiTxHwGetCapabilities
 * 
 * *****************  Version 55  *****************
 * User: Burnouf       Date: 11/12/06   Time: 11:07
 * Updated in $/Source/tmbslHdmiTx/Src
 * PR46 remove demo_build flag around fct tmbslHdmiTxTmdsSetSerializer
 * 
 * *****************  Version 54  *****************
 * User: Burnouf       Date: 07/12/06   Time: 14:21
 * Updated in $/Source/tmbslHdmiTx/Src
 * PR47 Decrease time of soft reset
 * 
 * *****************  Version 53  *****************
 * User: Mayhew       Date: 23/11/06   Time: 15:05
 * Updated in $/Source/tmbslHdmiTx/Src
 * PNF77 Don't detect hotplug using latched hpd status, to avoid
 * connection glitches
 * PNF78 Ensure that DDC is not connected to I2C at start
 * 
 * *****************  Version 51  *****************
 * User: Djw          Date: 22/11/06   Time: 13:43
 * Updated in $/Source/tmbslHdmiTx/Src
 * PNF75 tmbslHdmiTxPowerSetState PowerOn modifications
 * 
 * *****************  Version 50  *****************
 * User: Djw          Date: 22/11/06   Time: 11:37
 * Updated in $/Source/tmbslHdmiTx/Src
 * PNF74 Changes to serial clock divider values in setPixelRepeat
 * 
 * *****************  Version 48  *****************
 * User: Djw          Date: 10/11/06   Time: 14:00
 * Updated in $/Source/tmbslHdmiTx/Src
 * PNF69 Add tmbslHdmiTxAudioOutSetChanStatus and
 * tmbslHdmiTxAudioOutSetChanStatusMapping. Removed write to Channel
 * Status Byte 3 from tmbslHdmiTxAudioInSetCts (replaced with a call to
 * tmbslHdmiTxAudioOutSetChanStatus in appHdmiTx_setInputOutput).
 * 
 * *****************  Version 47  *****************
 * User: Mayhew       Date: 10/11/06   Time: 10:36
 * Updated in $/Source/tmbslHdmiTx/Src
 * PNF68 Allow HDCP checks to be disabled by setting uChecksToDo=0
 * 
 * *****************  Version 46  *****************
 * User: Mayhew       Date: 10/11/06   Time: 10:11
 * Updated in $/Source/tmbslHdmiTx/Src
 * PNF68 RETIF_REG_FAIL macro replaces RETIF checks after register set/get
 * calls, and compiled out to save code space
 * PNF68 DIS members funcScheduled, uFuncScheduledMs replaced by HdcpFunc*
 * and HdcpCheck* members
 * PNF68 tmbslHdmiTxHdcpCheck reworked to be called at known interval to
 * handle function scheduling to avoid in-line delay in HandleBKSVResult
 * and to handle authentication check timing
 * PNF68 HDCP check timer started by tmbslHdmiTxHdcpRun and stopped by
 * tmbslHdmiTxHdcpStop and tmbslHdmiTxReset
 * PNF68 New DIS members saved by tmbslHdmiTxHdcpConfigure
 * PNF68 tmbslHdmiTxHwHandleTimer removed
 * kChStatI2sLayout0[] dimension corrected
 * 
 * *****************  Version 45  *****************
 * User: Mayhew       Date: 6/11/06    Time: 17:49
 * Updated in $/Source/tmbslHdmiTx/Src
 * PNF68 Split handleBKSVResultSecure out from
 * tmbslHdmiTxHdcpHandleBKSVResult
 * Add tmbslHdmiTxHwHandleTimer
 * 
 * *****************  Version 44  *****************
 * User: Djw          Date: 6/11/06    Time: 16:39
 * Updated in $/Source/tmbslHdmiTx/Src
 * PNF68 Fixed BKSVResult polling, modified HPD reading in
 * HwHandleInterrupt
 * 
 * *****************  Version 42  *****************
 * User: Mayhew       Date: 2/11/06    Time: 16:53
 * Updated in $/Source/tmbslHdmiTx/Src
 * PNF68 Remove N3 support. Add HPD polling in HandleBKSVResult.
 * 
 * *****************  Version 40  *****************
 * User: Mayhew       Date: 31/10/06   Time: 16:21
 * Updated in $/Source/tmbslHdmiTx/Src
 * PNF61 tmbslHdmiTxVideoInSetSyncAuto adjusts uRefPix when vinMode !=
 * CCIR656
 * PNF67 kChStatI2sLayout0 has new value 10 for 96kHz
 * 
 * *****************  Version 38  *****************
 * User: Mayhew       Date: 27/10/06   Time: 12:20
 * Updated in $/Source/tmbslHdmiTx/Src
 * PNF57 Single-entry register tables replaces by API calls
 * PNF59 HDCP colour bar interlock flag implemented
 * PNF59 Disabling colour bar now restores image colour
 * N2 device no longer supported, to save code space
 * 
 * *****************  Version 36  *****************
 * User: Djw          Date: 25/10/06   Time: 13:38
 * Updated in $/Source/tmbslHdmiTx/Src
 * PNF38 HDCP timing
 * PNF57 HDCP modifcations
 * PNF58 Modification of refpix algorithm in tmbslHdmiTxVideoInSetSyncAuto
 * 
 * *****************  Version 34  *****************
 * User: Mayhew       Date: 23/10/06   Time: 16:39
 * Updated in $/Source/tmbslHdmiTx/Src
 * PNF52 Replaced AKSV read taken out in PNF44
 * PNF54 VS_PIX_END_2 corrected for interlaced outputs
 * 
 * *****************  Version 32  *****************
 * User: Mayhew       Date: 13/10/06   Time: 11:09
 * Updated in $/Source/tmbslHdmiTx/Src
 * PNF37,43,44,45,46,47,48,49
 * 
 * *****************  Version 30  *****************
 * User: Mayhew       Date: 21/09/06   Time: 15:48
 * Updated in $/Source/tmbslHdmiTx/Src
 * PNF36 Set scaler twice to work round 576i -> 720p issue
 * Cut size of kCanSclInToOut table. 
 * Cut code size in demo by using RETIF_BADPARAM
 * 
 * *****************  Version 28  *****************
 * User: Mayhew       Date: 15/09/06   Time: 16:03
 * Updated in $/Source/tmbslHdmiTx/Src
 * PNF22 Add HdcpRi & HdcpFsmState to DIS default. Add HdcpCheck API.
 * PNF23 Add GetOtp API
 * PNF33 Set RPT_CNTRL
 * Fix QAC warnings, exclude unused code from demo
 * 
 * *****************  Version 26  *****************
 * User: Mayhew       Date: 7/09/06    Time: 9:42
 * Updated in $/Source/tmbslHdmiTx/Src
 * PNF27 Unmute audio FIFO in HDMI mode.
 * PNF29 Avoid having to set video format twice by calling setPixelRepeat
 * before setScalerFormat; write P00_VIDFORMAT once at end of
 * setScalerFormat.
 * 
 * *****************  Version 24  *****************
 * User: Mayhew       Date: 4/09/06    Time: 10:12
 * Updated in $/Source/tmbslHdmiTx/Src
 * PNF17 Change _DEFAULT pixel repeat count to _MIN
 * PNF18 Add kPowerOn, kPowerOff register settings
 * PNF21 Change 100ms HDCP time to 150ms
 * 
 * *****************  Version 22  *****************
 * User: Djw          Date: 24/08/06   Time: 11:56
 * Updated in $/Source/tmbslHdmiTx/Src
 * PNF8 Fix NO_HDCP output by once again setting TX33 register.
 * PNF14 Set pixel repetition clock vrf_clk using pixelRepeatCount.
 * 
 * *****************  Version 21  *****************
 * User: Djw          Date: 22/08/06   Time: 10:02
 * Updated in $/Source/tmbslHdmiTx/Src
 * NO_HDCP modifications. Problem Report 6. Change Request 9.
 * 
 * *****************  Version 20  *****************
 * User: Mayhew       Date: 10/07/06   Time: 13:08
 * Updated in $/Source/tmbslHdmiTx/Src
 * Enclose BCAPS Repeater and DDC Master Workaround mods with #defines.
 * Enclose N4 registers with version checks. Detect N5 version.
 * Fix GCC switch statement warning. Reset sp_cnt field when not used.
 * 
 * *****************  Version 18  *****************
 * User: Mayhew       Date: 30/06/06   Time: 13:36
 * Updated in $/Source/tmbslHdmiTx/Src
 * EdidSinkType replaced by SinkType. Add HdcpAksv. Set new BCAPS Repeater
 * bit.
 * Add VGA input. Set I2S channel status. Relaunch HDCP at T0.
 * Add DDC master workaround for N4. 
 * Check BCAPS repeater during BKSV handling to fix non-repeater
 * authentication.
 * Return NOT_SUPPORTED or INCONSISTENT errors if can't scale.
 * 
 * *****************  Version 16  *****************
 * User: Djw          Date: 16/06/06   Time: 12:02
 * Updated in $/Source/tmbslHdmiTx/Src
 * Added flag to Init to support use of alternate i2c address for EDID.
 * Added feature checking for devices with no Scaler and no HDCP.
 * 
 * *****************  Version 14  *****************
 * User: Mayhew       Date: 6/06/06    Time: 13:38
 * Updated in $/Source/tmbslHdmiTx/Src
 * Use checkUnitSetDis to reduce code size
 * 
 * *****************  Version 13  *****************
 * User: Mayhew       Date: 5/06/06    Time: 14:53
 * Updated in $/Source/tmbslHdmiTx/Src
 * Combine ScIn_TV and ScOut_TV tables to save code space.
 * Add missing ScgN and ScgR values.
 * Set _nosc and ctl_code registers. Read FAILSTATE register.
 * Add tmbslHdmiTxHdcpGetT0FailState and tmbslHdmiTxSysTimerWait.
 * Treat CCIR656 input as YUV422 for up/downsampling.
 * 
 * *****************  Version 11  *****************
 * User: Djw          Date: 24/05/06   Time: 11:17
 * Updated in $/Source/tmbslHdmiTx/Src
 * Added conditional compilation for demoboard build.
 * 
 * *****************  Version 9  *****************
 * User: Mayhew       Date: 22/05/06   Time: 15:59
 * Updated in $/Source/tmbslHdmiTx/Src
 * Add N4 to default version list. Add N4 version register handling. Mark
 * todo table values.
 * 
 * *****************  Version 7  *****************
 * User: Mayhew       Date: 19/05/06   Time: 11:33
 * Updated in $/Source/tmbslHdmiTx/Src
 * Use setHwRegisterFieldTable to reduce code size.
 * Reinstate scaIn/OutFmt in tmbslHdmiTxVideoSetInOut to match SCS.
 * Fix minor QAC code warnings.
 * 
 * *****************  Version 5  *****************
 * User: Mayhew       Date: 11/05/06   Time: 15:35
 * Updated in $/Source/tmbslHdmiTx/Src
 * Read KSV list 5 bytes at a time and increment the index registers
 * 
 * *****************  Version 4  *****************
 * User: Mayhew       Date: 10/05/06   Time: 17:06
 * Updated in $/Source/tmbslHdmiTx/Src
 * Add HDCP APIs and interrupt priorities, add ScalerGetMode and HwStartup
 * 
 * *****************  Version 3  *****************
 * User: Djw          Date: 20/04/06   Time: 17:33
 * Updated in $/Source/tmbslHdmiTx/Src
 * Improved hotplug handling.
 * 
 * *****************  Version 2  *****************
 * User: Mayhew       Date: 11/04/06   Time: 14:05
 * Updated in $/Source/tmbslHdmiTx/Src
 * Updated to match TX_fct_TST_ATC_4.URM script and SCS v1r11 for
 * TxVideoSetInOut, TxReset, ScalerSetSync, TmdsSetOutputs. Old local
 * functions removed.
 * 
 * *****************  Version 1  *****************
 * User: Mayhew       Date: 4/04/06    Time: 16:28
 * Created in $/Source/tmbslHdmiTx/Src
 * Driver phase 2
   \endverbatim
 *
*/

/*============================================================================*/
/*                   FILE CONFIGURATION                                       */
/*============================================================================*/

/* Defining this symbol on the compiler command line excludes some API checks */
/* #define NO_RETIF_BADPARAM */

/* Defining this symbol on the compiler command line excludes some API checks */
/* #define NO_RETIF_REG_FAIL */

/* Defining this symbol on the compiler command line excludes HDCP code */
/* #define NO_HDCP */

/* Defining this symbol on the compiler command line excludes unused code */
/* #define DEMO_BUILD */

/* Defining this symbol on the compiler command line adapts code for testing */
/* #define UNIT_TEST */

#ifndef NO_HDCP

/* Defining this symbol here enables the BCAPS Repeater mod for N4 onwards */
#define BCAPS_REPEATER

#ifndef TMFL_TDA9981_SUPPORT
/* Defining this symbol here enables the DDC Master workaround for N4 only */
#define DDC_MASTER_WORKAROUND
#endif /* TMFL_TDA9981_SUPPORT */ 

#endif /* NO_HDCP */

/*============================================================================*/
/*                   STANDARD INCLUDE FILES                                   */
/*============================================================================*/


/*============================================================================*/
/*                   PROJECT INCLUDE FILES                                    */
/*============================================================================*/
#include "tmbslHdmiTx.h"
#include "tmbslHdmiTx_local.h"

/*============================================================================*/
/*                   MACRO DEFINITIONS                                        */
/*============================================================================*/


/*============================================================================*/
/*                   TYPE DEFINITIONS                                         */
/*============================================================================*/

#define SSD_UNUSED_VALUE 0xF0

#ifdef FORMAT_PC
#define DEPTH_COLOR_PC 1  /* PC_FORMAT only 8 bits available */
#endif /* FORMAT_PC */

/*============================================================================*/
/*                   PUBLIC VARIABLE DEFINITIONS                              */
/*============================================================================*/


/*============================================================================*/
/*                   STATIC CONSTANT DECLARATIONS                             */
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
        (ptmbslHdmiTxCallback_t)0
    },
    {                                   /* uSupportedVersions[] */
        E_DEV_VERSION_N4, /* <=== Applies to N5 also */
        E_DEV_VERSION_LIST_END
    },
    E_DEV_VERSION_LIST_END,             /* uDeviceVersion */
    E_DEV_VERSION_LIST_END,             /* uDeviceFeatures */
    tmPowerOn,                         /* ePowerState */
    False,                              /* EdidAlternateAddr */
    HDMITX_SINK_DVI,                    /* sinkType */
    HDMITX_SINK_DVI,                    /* EdidSinkType */
    False,                              /* EdidSinkAi */
    0,                                  /* EdidCeaFlags */
    HDMITX_EDID_NOT_READ,               /* EdidStatus */
	0,                                  /* NbDTDStored */
    {                                   /* EdidDTD: */
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /* 1 */                     
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*2 */  
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*3 */
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*4 */
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*5 */
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*6 */
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*7 */
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*8 */
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*9 */
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0}   /*10*/
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
        {0,0,0}                             /* {ModeChans, Freqs, Byte3} */
    },
    0,                                  /* EdidSadCnt */
    {
        0                               /* EdidBlock[ ] */
    },
    0,                                  /* EdidBlockCnt */
    0,                                  /* EdidSourceAddress */
	    {                                   /* EDIDBasicDisplayParam */
        0,                              /* uVideoInputDef */
        0,                              /* uMaxHorizontalSize */
        0,                              /* uMaxVerticalSize */
        0,                              /* uGamma */
        0,                              /* uFeatureSupport */
    },
#ifndef NO_HDCP
    0,                                  /* HdcpPortAddress */
    HDMITX_HDCP_TXMODE_NOT_SET,         /* HdcpTxMode */
    HDMITX_HDCP_OPTION_DEFAULT,         /* HdcpOptions */
    0,                                  /* HdcpBcaps */
    0,                                  /* HdcpBstatus */
    0,                                  /* HdcpRi */
    0,                                  /* HdcpFsmState */
    0,                                  /* HdcpT0FailState */
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
#ifdef TMFL_TDA9981_SUPPORT
#ifdef TMFL_RX_SENSE_ON
    HDMITX_RX_SENSE_INVALID,            /* rxSenseStatus */
#endif /* TMFL_RX_SENSE_ON */
#endif /* TMFL_TDA9981_SUPPORT */
    E_PAGE_INVALID,                     /* curRegPage */
    {
        /* These match power-up defaults.  shadowReg[]: */
        0x00,                           /* E_SP00_MAIN_CNTRL0 */
        0x00,                           /* E_SP00_INT_FLAGS_0 */
        0x00,                           /* E_SP00_INT_FLAGS_1 */
#ifdef TMFL_TDA9981_SUPPORT
        0x00,                           /* E_SP00_INT_FLAGS_2 */
#endif /* TMFL_TDA9981_SUPPORT */
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
#ifndef TMFL_TDA9981_SUPPORT
        ,0x00                           /* E_SP01_SC_VIDFORMAT*/
        ,0x00                           /* E_SP01_SC_CNTRL    */
        ,0x00                           /* E_SP01_TBG_CNTRL_0 */
#endif /* TMFL_TDA9981_SUPPORT */
#ifndef NO_HDCP
        ,0x00                           /* E_SP12_HDCP_CTRL   */
#ifdef BCAPS_REPEATER
        ,0x00                           /* E_SP12_HDCP_BCAPS  */
#endif /* BCAPS_REPEATER */
#endif /* NO_HDCP */
    },
	False,								/* Init prevBluescreen to false */
	False,								/* Init prevPattern to false */
    False                              /* bInitialized */
};

/**
 * Lookup table to convert from EIA/CEA TV video formats used in the EDID and
 * in API parameters to pixel clock frequencies, according to SCS Table
 * "HDMI Pixel Clock Frequencies per EIA/CEA-861B Video Output Format".
 * The other index is the veritical frame frequency.
 */
 
static CONST_DAT UInt8 kVfmtToPixClk_TV[HDMITX_VFMT_TV_NUM][HDMITX_VFREQ_NUM] =
{
  /* HDMITX_VFREQ_24Hz HDMITX_VFREQ_25Hz HDMITX_VFREQ_30Hz HDMITX_VFREQ_50Hz HDMITX_VFREQ_59Hz HDMITX_VFREQ_60Hz */
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

/**
 * Lookup table to convert PC formats used in API parameters to pixel clock 
 * frequencies.
 * The other index is the veritical frame frequency.
 */
#ifdef FORMAT_PC
static CONST_DAT UInt8 kVfmtToPixClk_PC[HDMITX_VFMT_PC_NUM+1] =
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
    E_PIXCLK_135000 ,   /* HDMITX_VFMT_PC_1280x1024p_75Hz */
    E_PIXCLK_INVALID,   /* HDMITX_VFMT_PC_640x350p_85Hz   */
    E_PIXCLK_INVALID,   /* HDMITX_VFMT_PC_640x400p_85Hz   */
    E_PIXCLK_INVALID,   /* HDMITX_VFMT_PC_720x400p_85Hz   */
    E_PIXCLK_36000  ,   /* HDMITX_VFMT_PC_640x480p_85Hz   */
    E_PIXCLK_56250  ,   /* HDMITX_VFMT_PC_800x600p_85Hz   */
    E_PIXCLK_INVALID,   /* HDMITX_VFMT_PC_1024x768p_85Hz  */
    E_PIXCLK_INVALID,   /* HDMITX_VFMT_PC_1152x864p_85Hz  */
    E_PIXCLK_INVALID,   /* HDMITX_VFMT_PC_1280x960p_85Hz  */
    E_PIXCLK_INVALID,   /* HDMITX_VFMT_PC_1280x1024p_85Hz */
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
    E_REGVFMT_1920x1080p_50Hz,      /* HDMITX_VFMT_31_1920x1080p_50Hz */
    E_REGVFMT_1920x1080p_24Hz,      /* HDMITX_VFMT_32_1920x1080p_24Hz */
    E_REGVFMT_1920x1080p_25Hz,      /* HDMITX_VFMT_33_1920x1080p_25Hz */
    E_REGVFMT_1920x1080p_30Hz       /* HDMITX_VFMT_34_1920x1080p_30Hz */
#ifdef FORMAT_PC    
   ,E_REGVFMT_640x480p_60Hz,        /* HDMITX_VFMT_PC_640x480p_60Hz  */
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
    E_REGVFMT_1280x1024p_75Hz,      /* HDMITX_VFMT_PC_1280x1024p_75Hz  */
    E_REGVFMT_INVALID,              /* HDMITX_VFMT_PC_640x350p_85Hz  */
    E_REGVFMT_INVALID,              /* HDMITX_VFMT_PC_640x400p_85Hz  */
    E_REGVFMT_INVALID,              /* HDMITX_VFMT_PC_720x400p_85Hz  */
    E_REGVFMT_640x480p_85Hz,        /* HDMITX_VFMT_PC_640x480p_85Hz  */
    E_REGVFMT_800x600p_85Hz,        /* HDMITX_VFMT_PC_800x600p_85Hz  */
    E_REGVFMT_INVALID,              /* HDMITX_VFMT_PC_1024x768p_85Hz  */
    E_REGVFMT_INVALID,              /* HDMITX_VFMT_PC_1152x864p_85Hz  */
    E_REGVFMT_INVALID,              /* HDMITX_VFMT_PC_1280x960p_85Hz  */
    E_REGVFMT_INVALID,              /* HDMITX_VFMT_PC_1280x1024p_85Hz */
    E_REGVFMT_INVALID,              /* HDMITX_VFMT_PC_1024x768i_87Hz  */
#endif /* FORMAT PC */ 
    
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
    SCIO(E_REGVFMT_SCIN_INVALID,   E_REGVFMT_SCOUT_INVALID),         /* HDMITX_VFMT_NULL               */
    SCIO(E_REGVFMT_SCIN_480p_60Hz, E_REGVFMT_SCOUT_480p_60Hz),       /* HDMITX_VFMT_01_640x480p_60Hz   */
    SCIO(E_REGVFMT_SCIN_480p_60Hz, E_REGVFMT_SCOUT_480p_60Hz),       /* HDMITX_VFMT_02_720x480p_60Hz   */
    SCIO(E_REGVFMT_SCIN_480p_60Hz, E_REGVFMT_SCOUT_480p_60Hz),       /* HDMITX_VFMT_03_720x480p_60Hz   */
    SCIO(E_REGVFMT_SCIN_INVALID,   E_REGVFMT_SCOUT_720p_50Hz_60Hz),  /* HDMITX_VFMT_04_1280x720p_60Hz  */
    SCIO(E_REGVFMT_SCIN_INVALID,   E_REGVFMT_SCOUT_1080i_50Hz_60Hz), /* HDMITX_VFMT_05_1920x1080i_60Hz */
    SCIO(E_REGVFMT_SCIN_480i_60Hz, E_REGVFMT_SCOUT_INVALID),         /* HDMITX_VFMT_06_720x480i_60Hz   */
    SCIO(E_REGVFMT_SCIN_480i_60Hz, E_REGVFMT_SCOUT_INVALID),         /* HDMITX_VFMT_07_720x480i_60Hz   */
    SCIO(E_REGVFMT_SCIN_INVALID,   E_REGVFMT_SCOUT_INVALID),         /* HDMITX_VFMT_08_720x240p_60Hz   */
    SCIO(E_REGVFMT_SCIN_INVALID,   E_REGVFMT_SCOUT_INVALID),         /* HDMITX_VFMT_09_720x240p_60Hz   */
    SCIO(E_REGVFMT_SCIN_480i_60Hz, E_REGVFMT_SCOUT_INVALID),         /* HDMITX_VFMT_10_720x480i_60Hz   */
    SCIO(E_REGVFMT_SCIN_480i_60Hz, E_REGVFMT_SCOUT_INVALID),         /* HDMITX_VFMT_11_720x480i_60Hz   */
    SCIO(E_REGVFMT_SCIN_INVALID,   E_REGVFMT_SCOUT_INVALID),         /* HDMITX_VFMT_12_720x240p_60Hz   */
    SCIO(E_REGVFMT_SCIN_INVALID,   E_REGVFMT_SCOUT_INVALID),         /* HDMITX_VFMT_13_720x240p_60Hz   */
    SCIO(E_REGVFMT_SCIN_480p_60Hz, E_REGVFMT_SCOUT_INVALID),         /* HDMITX_VFMT_14_1440x480p_60Hz  */
    SCIO(E_REGVFMT_SCIN_480p_60Hz, E_REGVFMT_SCOUT_INVALID),         /* HDMITX_VFMT_15_1440x480p_60Hz  */
    SCIO(E_REGVFMT_SCIN_INVALID,   E_REGVFMT_SCOUT_INVALID),         /* HDMITX_VFMT_16_1920x1080p_60Hz */
    SCIO(E_REGVFMT_SCIN_576p_50Hz, E_REGVFMT_SCOUT_576ip_50Hz),      /* HDMITX_VFMT_17_720x576p_50Hz   */
    SCIO(E_REGVFMT_SCIN_576p_50Hz, E_REGVFMT_SCOUT_576ip_50Hz),      /* HDMITX_VFMT_18_720x576p_50Hz   */
    SCIO(E_REGVFMT_SCIN_INVALID,   E_REGVFMT_SCOUT_720p_50Hz_60Hz),  /* HDMITX_VFMT_19_1280x720p_50Hz  */
    SCIO(E_REGVFMT_SCIN_INVALID,   E_REGVFMT_SCOUT_1080i_50Hz_60Hz), /* HDMITX_VFMT_20_1920x1080i_50Hz */
    SCIO(E_REGVFMT_SCIN_576i_50Hz, E_REGVFMT_SCOUT_576ip_50Hz),      /* HDMITX_VFMT_21_720x576i_50Hz   */
    SCIO(E_REGVFMT_SCIN_576i_50Hz, E_REGVFMT_SCOUT_576ip_50Hz),      /* HDMITX_VFMT_22_720x576i_50Hz   */
    SCIO(E_REGVFMT_SCIN_INVALID,   E_REGVFMT_SCOUT_INVALID),         /* HDMITX_VFMT_23_720x288p_50Hz   */
    SCIO(E_REGVFMT_SCIN_INVALID,   E_REGVFMT_SCOUT_INVALID),         /* HDMITX_VFMT_24_720x288p_50Hz   */
    SCIO(E_REGVFMT_SCIN_576i_50Hz, E_REGVFMT_SCOUT_INVALID),         /* HDMITX_VFMT_25_720x576i_50Hz   */
    SCIO(E_REGVFMT_SCIN_576i_50Hz, E_REGVFMT_SCOUT_INVALID),         /* HDMITX_VFMT_26_720x576i_50Hz   */
    SCIO(E_REGVFMT_SCIN_INVALID,   E_REGVFMT_SCOUT_INVALID),         /* HDMITX_VFMT_27_720x288p_50Hz   */
    SCIO(E_REGVFMT_SCIN_INVALID,   E_REGVFMT_SCOUT_INVALID),         /* HDMITX_VFMT_28_720x288p_50Hz   */
    SCIO(E_REGVFMT_SCIN_576p_50Hz, E_REGVFMT_SCOUT_INVALID),         /* HDMITX_VFMT_29_1440x576p_50Hz  */
    SCIO(E_REGVFMT_SCIN_576p_50Hz, E_REGVFMT_SCOUT_INVALID),         /* HDMITX_VFMT_30_1440x576p_50Hz  */
    SCIO(E_REGVFMT_SCIN_INVALID,   E_REGVFMT_SCOUT_INVALID),         /* HDMITX_VFMT_31_1920x1080p_50Hz */
	SCIO(E_REGVFMT_SCIN_INVALID,   E_REGVFMT_SCOUT_INVALID),         /* HDMITX_VFMT_32_1920x1080p_24Hz */
	SCIO(E_REGVFMT_SCIN_INVALID,   E_REGVFMT_SCOUT_INVALID),         /* HDMITX_VFMT_33_1920x1080p_25Hz */
	SCIO(E_REGVFMT_SCIN_INVALID,   E_REGVFMT_SCOUT_INVALID)          /* HDMITX_VFMT_34_1920x1080p_30Hz */
};

/**
 * Macro to pack output format flags as bits in a UInt16 for the following table
 */
#define PKOPF(p1,p2,p3,p4,p5,p6,p7,p8,p9,p10,p11,p12,p13,p14,p15,p16,p17) \
    ((p1)|((p2)<<1)|((p3)<<2)|((p4)<<3)|((p5)<<4)|((p6)<<5)|((p7)<<6)|((p8)<<7)| \
    ((p9)<<8)|((p10)<<9)|((p11)<<10)|((p12)<<11)|((p13)<<12)|((p14)<<13)|((p15)<<14)|((p16)<<15)|((p17)<<16))

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
	/*										  E_REGVFMT_1920x1080p_24Hz */
	/*											 E_REGVFMT_1920x1080p_25Hz */
	/*												E_REGVFMT_1920x1080p_30Hz */
    /*												   E_REGVFMT_INVALID		   */
														  /* ------INPUT FORMATS------ */
PKOPF( 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),/* E_REGVFMT_640x480p_60Hz   */
PKOPF( 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),/* E_REGVFMT_720x480p_60Hz   */
PKOPF( 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),/* E_REGVFMT_1280x720p_60Hz  */
PKOPF( 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),/* E_REGVFMT_1920x1080i_60Hz */
PKOPF( 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),/* E_REGVFMT_720x480i_60Hz   */
PKOPF( 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),/* E_REGVFMT_720x240p_60Hz   */
PKOPF( 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),/* E_REGVFMT_1920x1080p_60Hz */
PKOPF( 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0),/* E_REGVFMT_720x576p_50Hz   */
PKOPF( 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0),/* E_REGVFMT_1280x720p_50Hz  */
PKOPF( 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0),/* E_REGVFMT_1920x1080i_50Hz */
PKOPF( 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0),/* E_REGVFMT_720x576i_50Hz   */
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
    { 20,     0,    500,    500},   /* E_REGVFMT_SCIN_480i_60Hz */
    {  0,    20,     88,     88},   /* E_REGVFMT_SCIN_576i_50Hz */
    {  0,     0,    250,    250},   /* E_REGVFMT_SCIN_480p_60Hz */
    {  0,     0,     44,     44},   /* E_REGVFMT_SCIN_576p_50Hz */
};
static CONST_DAT UInt16 kSclModeToScgR[E_REGVFMT_SCIN_NUM][E_REGVFMT_SCOUT_NUM] =
{ /* E_REGVFMT_SCOUT_480p_60Hz */
  /*        E_REGVFMT_SCOUT_576ip_50Hz */
  /*                E_REGVFMT_SCOUT_720p_50Hz_60Hz */
  /*                        E_REGVFMT_SCOUT_1080i_50Hz_60Hz */
    { 10,     0,     91,     91},   /* E_REGVFMT_SCIN_480i_60Hz */
    {  0,    10,     16,     16},   /* E_REGVFMT_SCIN_576i_50Hz */
    {  0,     0,     91,     91},   /* E_REGVFMT_SCIN_480p_60Hz */
    {  0,     0,     16,     16},   /* E_REGVFMT_SCIN_576p_50Hz */
};


/**
 * Lookup tables to hide the scaler issue on side effect
 * by adding blanking pixel on right and left side of the screen
 */
static CONST_DAT UInt8 kSclAddBlkPix[E_REGVFMT_SCIN_NUM][E_REGVFMT_SCOUT_NUM] =
{ /* E_REGVFMT_SCOUT_480p_60Hz */
  /*        E_REGVFMT_SCOUT_576ip_50Hz */
  /*                E_REGVFMT_SCOUT_720p_50Hz_60Hz */
  /*                        E_REGVFMT_SCOUT_1080i_50Hz_60Hz */
    {  0,     0,      4,      8},   /* E_REGVFMT_SCIN_480i_60Hz */
    {  0,     0,      4,      8},   /* E_REGVFMT_SCIN_576i_50Hz */
    {  0,     0,      4,      8},   /* E_REGVFMT_SCIN_480p_60Hz */
    {  0,     0,      4,      8},   /* E_REGVFMT_SCIN_576p_50Hz */
};
static CONST_DAT UInt8 kSclClearBlkPix[] =
{                     
    0,    /* HDMITX_VOUTMODE_RGB444 */
    1,    /* HDMITX_VOUTMODE_YUV422 */
    1,    /* HDMITX_VOUTMODE_YUV444 */
    0,    /* HDMITX_VOUTMODE_NO_CHANGE */
    0     /* HDMITX_VOUTMODE_INVALID */
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
	0, /* E_REGVFMT_1280x1024p_75Hz */
    0, /* E_REGVFMT_1360x768p_60Hz  */
    0, /* E_REGVFMT_1400x1050p_60Hz */
    0, /* E_REGVFMT_1600x1200p_60Hz */
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
    0, /* E_REGVFMT_1280x1024p_75Hz */
    0, /* E_REGVFMT_1360x768p_60Hz  */
    0, /* E_REGVFMT_1400x1050p_60Hz */
    0, /* E_REGVFMT_1600x1200p_60Hz */
#endif /* FORMAT_PC */

};

/**
 * Lookup table to convert from video format codes used in the 
 * E_REG_P00_VIDFORMAT_W register to corresponding 
 * trios of 2-bit values in the srl_nosc, scg_nosc and de_nosc
 * PLL control registers
 */
#define SSD(srl,scg,de) (((srl)<<4)|((scg)<<2)|(de))
#define SSD2SRL(ssd)    (UInt8)(((ssd)>>4)&3)
#define SSD2SCG(ssd)    (UInt8)(((ssd)>>2)&3)
#define SSD2DE(ssd)     (UInt8)((ssd)&3)


static CONST_DAT struct
{
    UInt8 ScaOffCcirOffDblEdgeOff[E_REGVFMT_NUM_TV];
                
    UInt8 ScaOffCcirOnDblEdgeOff[E_REGVFMT_NUM_TV];

    UInt8 ScaOffCcirOnDblEdgeOn[E_REGVFMT_NUM_TV];

    UInt8 ScaOnCcirOffDblEdgeOffInterlace[E_REGVFMT_NUM_TV];
        
    UInt8 ScaOnCcirOnDblEdgeOffInterlace[E_REGVFMT_NUM_TV];

    UInt8 ScaOnCcirOnDblEdgeOnInterlace[E_REGVFMT_NUM_TV];

    UInt8 ScaOnCcirOffDblEdgeOffProgressif[E_REGVFMT_NUM_TV];

    UInt8 ScaOnCcirOnDblEdgeOffProgressif[E_REGVFMT_NUM_TV];

    UInt8 ScaOnCcirOnDblEdgeOnProgressif[E_REGVFMT_NUM_TV];

#ifdef FORMAT_PC
    UInt8 SettingsFormatPC[E_REGVFMT_NUM_PC];
#endif /* FORMAT_PC */
    
}kRegvfmtToPllSsd = {

    {   /*ScaOffCCIROffDblEdgeOff*/
        /* SRL,SCG,DE */
        SSD(2,2,2), /* E_REGVFMT_640x480p_60Hz   */
        SSD(2,2,2), /* E_REGVFMT_720x480p_60Hz   */
        SSD(1,1,1), /* E_REGVFMT_1280x720p_60Hz  */
        SSD(1,1,1), /* E_REGVFMT_1920x1080i_60Hz */
        SSD(3,3,3), /* E_REGVFMT_720x480i_60Hz   */
        SSD(0xF,0,0), /* E_REGVFMT_720x240p_60Hz   */ /** \todo Need nosc PLL value */
        SSD(0,0,0), /* E_REGVFMT_1920x1080p_60Hz */
        SSD(2,2,2), /* E_REGVFMT_720x576p_50Hz   */
        SSD(1,1,1), /* E_REGVFMT_1280x720p_50Hz  */
        SSD(1,1,1), /* E_REGVFMT_1920x1080i_50Hz */
        SSD(3,3,3), /* E_REGVFMT_720x576i_50Hz   */
        SSD(0xF,0,0), /* E_REGVFMT_720x288p_50Hz   */ /** \todo Need nosc PLL value */
        SSD(0,0,0),  /* E_REGVFMT_1920x1080p_50Hz */
		SSD(1,1,1),  /* E_REGVFMT_1920x1080p_24Hz */
		SSD(1,1,1),  /* E_REGVFMT_1920x1080p_25Hz */
		SSD(1,1,1)  /* E_REGVFMT_1920x1080p_30Hz */
    },

    {   /*ScaOffCCIROnDblEdgeOff*/
        /* SRL,SCG,DE */
        SSD(0xF,0,0), /* E_REGVFMT_640x480p_60Hz   */
        SSD(2,1,1), /* E_REGVFMT_720x480p_60Hz   */
        SSD(0xF,0,0), /* E_REGVFMT_1280x720p_60Hz  */
        SSD(0xF,0,0), /* E_REGVFMT_1920x1080i_60Hz */
        SSD(2,2,2), /* E_REGVFMT_720x480i_60Hz   */
        SSD(0xF,0,0), /* E_REGVFMT_720x240p_60Hz   */ /** \todo Need nosc PLL value */
        SSD(0xF,0,0), /* E_REGVFMT_1920x1080p_60Hz */
        SSD(2,1,1), /* E_REGVFMT_720x576p_50Hz   */
        SSD(0xF,0,0), /* E_REGVFMT_1280x720p_50Hz  */
        SSD(0xF,0,0), /* E_REGVFMT_1920x1080i_50Hz */
        SSD(2,2,2), /* E_REGVFMT_720x576i_50Hz   */
        SSD(0xF,0,0), /* E_REGVFMT_720x288p_50Hz   */ /** \todo Need nosc PLL value */
        SSD(0xF,0,0),  /* E_REGVFMT_1920x1080p_50Hz */
		SSD(0xF,0,0),  /* E_REGVFMT_1920x1080p_24Hz */
		SSD(0xF,0,0),  /* E_REGVFMT_1920x1080p_25Hz */
		SSD(0xF,0,0)  /* E_REGVFMT_1920x1080p_30Hz */
    },

    {   /*ScaOffCcirONDblEdgeOn*/
        /* SRL,SCG,DE */
        SSD(0xF,0,0), /* E_REGVFMT_640x480p_60Hz   */
        SSD(2,1,1), /* E_REGVFMT_720x480p_60Hz   */
        SSD(0xF,0,0), /* E_REGVFMT_1280x720p_60Hz  */
        SSD(0xF,0,0), /* E_REGVFMT_1920x1080i_60Hz */
        SSD(2,2,2), /* E_REGVFMT_720x480i_60Hz   */
        SSD(0xF,0,0), /* E_REGVFMT_720x240p_60Hz   */ /** \todo Need nosc PLL value */
        SSD(0xF,0,0), /* E_REGVFMT_1920x1080p_60Hz */
        SSD(2,1,1), /* E_REGVFMT_720x576p_50Hz   */
        SSD(0xF,0,0), /* E_REGVFMT_1280x720p_50Hz  */
        SSD(0xF,0,0), /* E_REGVFMT_1920x1080i_50Hz */
        SSD(2,2,2), /* E_REGVFMT_720x576i_50Hz   */
        SSD(0xF,0,0), /* E_REGVFMT_720x288p_50Hz   */ /** \todo Need nosc PLL value */
        SSD(0xF,0,0),  /* E_REGVFMT_1920x1080p_50Hz */
		SSD(0xF,0,0),  /* E_REGVFMT_1920x1080p_24Hz */
		SSD(0xF,0,0),  /* E_REGVFMT_1920x1080p_25Hz */
		SSD(0xF,0,0)  /* E_REGVFMT_1920x1080p_30Hz */
    },

    {   /* ScaOnCcirOffDblEdgeOffInterlace */
        /* SRL,SCG,DE */
        SSD(0xF,0,0), /* E_REGVFMT_640x480p_60Hz   */
        SSD(2,2,3), /* E_REGVFMT_720x480p_60Hz   */
        SSD(1,1,3), /* E_REGVFMT_1280x720p_60Hz  */
        SSD(1,1,3), /* E_REGVFMT_1920x1080i_60Hz */
        SSD(0xF,0,0), /* E_REGVFMT_720x480i_60Hz   */
        SSD(0xF,0,0), /* E_REGVFMT_720x240p_60Hz   */ /** \todo Need nosc PLL value */
        SSD(0xF,0,0), /* E_REGVFMT_1920x1080p_60Hz */
        SSD(2,2,3), /* E_REGVFMT_720x576p_50Hz   */
        SSD(1,1,3), /* E_REGVFMT_1280x720p_50Hz  */
        SSD(1,1,3), /* E_REGVFMT_1920x1080i_50Hz */
        SSD(0xF,0,0), /* E_REGVFMT_720x576i_50Hz   */
        SSD(0xF,0,0), /* E_REGVFMT_720x288p_50Hz   */ /** \todo Need nosc PLL value */
        SSD(0xF,0,0),  /* E_REGVFMT_1920x1080p_50Hz */
		SSD(1,1,3),  /* E_REGVFMT_1920x1080p_24Hz */
		SSD(1,1,3),  /* E_REGVFMT_1920x1080p_25Hz */
		SSD(1,1,3)  /* E_REGVFMT_1920x1080p_30Hz */
    },

    {   /* ScaOnCcirONDblEdgeOffInterlace */
        /* SRL,SCG,DE */
        SSD(0xF,0,0), /* E_REGVFMT_640x480p_60Hz   */
        SSD(2,1,2), /* E_REGVFMT_720x480p_60Hz   */
        SSD(1,0,2), /* E_REGVFMT_1280x720p_60Hz  */
        SSD(1,0,2), /* E_REGVFMT_1920x1080i_60Hz */
        SSD(0xF,0,0), /* E_REGVFMT_720x480i_60Hz   */
        SSD(0xF,0,0), /* E_REGVFMT_720x240p_60Hz   */ /** \todo Need nosc PLL value */
        SSD(0xF,0,0), /* E_REGVFMT_1920x1080p_60Hz */
        SSD(2,1,2), /* E_REGVFMT_720x576p_50Hz   */
        SSD(1,0,2), /* E_REGVFMT_1280x720p_50Hz  */
        SSD(1,0,2), /* E_REGVFMT_1920x1080i_50Hz */
        SSD(0xF,0,0), /* E_REGVFMT_720x576i_50Hz   */
        SSD(0xF,0,0), /* E_REGVFMT_720x288p_50Hz   */ /** \todo Need nosc PLL value */
        SSD(0xF,0,0),  /* E_REGVFMT_1920x1080p_50Hz */
		SSD(1,0,2),  /* E_REGVFMT_1920x1080p_24Hz */
		SSD(1,0,2),  /* E_REGVFMT_1920x1080p_25Hz */
		SSD(1,0,2)  /* E_REGVFMT_1920x1080p_30Hz */
    },

    {  /* ScaOnCcirONDblEdgeOnInterlace */
        /* SRL,SCG,DE */
        SSD(0xF,0,0), /* E_REGVFMT_640x480p_60Hz   */
        SSD(2,1,2), /* E_REGVFMT_720x480p_60Hz   */
        SSD(1,0,2), /* E_REGVFMT_1280x720p_60Hz  */
        SSD(1,0,2), /* E_REGVFMT_1920x1080i_60Hz */
        SSD(0xF,0,0), /* E_REGVFMT_720x480i_60Hz   */
        SSD(0xF,0,0), /* E_REGVFMT_720x240p_60Hz   */ /** \todo Need nosc PLL value */
        SSD(0xF,0,0), /* E_REGVFMT_1920x1080p_60Hz */
        SSD(2,1,2), /* E_REGVFMT_720x576p_50Hz   */
        SSD(1,0,2), /* E_REGVFMT_1280x720p_50Hz  */
        SSD(1,0,2), /* E_REGVFMT_1920x1080i_50Hz */
        SSD(0xF,0,0), /* E_REGVFMT_720x576i_50Hz   */
        SSD(0xF,0,0), /* E_REGVFMT_720x288p_50Hz   */ /** \todo Need nosc PLL value */
        SSD(0xF,0,0),  /* E_REGVFMT_1920x1080p_50Hz */
		SSD(1,0,2),  /* E_REGVFMT_1920x1080p_24Hz */
		SSD(1,0,2),  /* E_REGVFMT_1920x1080p_25Hz */
		SSD(1,0,2)  /* E_REGVFMT_1920x1080p_30Hz */
    },

    { /* ScaOnCcirOffDblEdgeOffProgressif */
        /* SRL,SCG,DE */
        SSD(0xF,0,0), /* E_REGVFMT_640x480p_60Hz   */
        SSD(0xF,0,0), /* E_REGVFMT_720x480p_60Hz   */
        SSD(1,1,2), /* E_REGVFMT_1280x720p_60Hz  */
        SSD(1,1,2), /* E_REGVFMT_1920x1080i_60Hz */
        SSD(0xF,0,0), /* E_REGVFMT_720x480i_60Hz   */
        SSD(0xF,0,0), /* E_REGVFMT_720x240p_60Hz   */ /** \todo Need nosc PLL value */
        SSD(0xF,0,0), /* E_REGVFMT_1920x1080p_60Hz */
        SSD(0xF,0,0), /* E_REGVFMT_720x576p_50Hz   */
        SSD(1,1,2), /* E_REGVFMT_1280x720p_50Hz  */
        SSD(1,1,2), /* E_REGVFMT_1920x1080i_50Hz */
        SSD(0xF,0,0), /* E_REGVFMT_720x576i_50Hz   */
        SSD(0xF,0,0), /* E_REGVFMT_720x288p_50Hz   */ /** \todo Need nosc PLL value */
        SSD(0xF,0,0),  /* E_REGVFMT_1920x1080p_50Hz */
		SSD(1,1,2),  /* E_REGVFMT_1920x1080p_24Hz */
		SSD(1,1,2),  /* E_REGVFMT_1920x1080p_25Hz */
		SSD(1,1,2)  /* E_REGVFMT_1920x1080p_30Hz */
    },

    {   /* ScaOnCcirOnDblEdgeOffProgressif */
        /* SRL,SCG,DE */
        SSD(0xF,0,0), /* E_REGVFMT_640x480p_60Hz   */
        SSD(0xF,0,0), /* E_REGVFMT_720x480p_60Hz   */
        SSD(1,0,1), /* E_REGVFMT_1280x720p_60Hz  */
        SSD(1,0,1), /* E_REGVFMT_1920x1080i_60Hz */
        SSD(0xF,0,0), /* E_REGVFMT_720x480i_60Hz   */
        SSD(0xF,0,0), /* E_REGVFMT_720x240p_60Hz   */ /** \todo Need nosc PLL value */
        SSD(0xF,0,0), /* E_REGVFMT_1920x1080p_60Hz */
        SSD(0xF,0,0), /* E_REGVFMT_720x576p_50Hz   */
        SSD(1,0,1), /* E_REGVFMT_1280x720p_50Hz  */
        SSD(1,0,1), /* E_REGVFMT_1920x1080i_50Hz */
        SSD(0xF,0,0), /* E_REGVFMT_720x576i_50Hz   */
        SSD(0xF,0,0), /* E_REGVFMT_720x288p_50Hz   */ /** \todo Need nosc PLL value */
        SSD(0xF,0,0),  /* E_REGVFMT_1920x1080p_50Hz */
		SSD(1,0,1),  /* E_REGVFMT_1920x1080p_24Hz */
		SSD(1,0,1),  /* E_REGVFMT_1920x1080p_25Hz */
		SSD(1,0,1)  /* E_REGVFMT_1920x1080p_30Hz */
    },

    {   /* ScaOnCcirONDblEdgeOnProgressif */
        /* SRL,SCG,DE */
        SSD(0xF,0,0), /* E_REGVFMT_640x480p_60Hz   */
        SSD(0xF,0,0), /* E_REGVFMT_720x480p_60Hz   */
        SSD(1,0,1), /* E_REGVFMT_1280x720p_60Hz  */
        SSD(1,0,1), /* E_REGVFMT_1920x1080i_60Hz */
        SSD(0xF,0,0), /* E_REGVFMT_720x480i_60Hz   */
        SSD(0xF,0,0), /* E_REGVFMT_720x240p_60Hz   */ /** \todo Need nosc PLL value */
        SSD(0xF,0,0), /* E_REGVFMT_1920x1080p_60Hz */
        SSD(0xF,0,0), /* E_REGVFMT_720x576p_50Hz   */
        SSD(1,0,1), /* E_REGVFMT_1280x720p_50Hz  */
        SSD(1,0,1), /* E_REGVFMT_1920x1080i_50Hz */
        SSD(0xF,0,0), /* E_REGVFMT_720x576i_50Hz   */
        SSD(0xF,0,0), /* E_REGVFMT_720x288p_50Hz   */ /** \todo Need nosc PLL value */
        SSD(0xF,0,0),  /* E_REGVFMT_1920x1080p_50Hz */
		SSD(1,0,1),  /* E_REGVFMT_1920x1080p_24Hz */
		SSD(1,0,1),  /* E_REGVFMT_1920x1080p_25Hz */
		SSD(1,0,1)  /* E_REGVFMT_1920x1080p_30Hz */
    }
    
#ifdef FORMAT_PC
  ,{
        /* SRL,SCG,DE */
        SSD(2,0,0),   /* E_REGVFMT_640x480p_72Hz   */
        SSD(2,0,0),   /* E_REGVFMT_640x480p_75Hz   */
        SSD(2,0,0),   /* E_REGVFMT_640x480p_85Hz   */
        SSD(1,0,0),   /* E_REGVFMT_800x600p_60Hz   */
        SSD(1,0,0),   /* E_REGVFMT_800x600p_72Hz   */
        SSD(1,0,0),   /* E_REGVFMT_800x600p_75Hz   */
        SSD(1,0,0),   /* E_REGVFMT_800x600p_85Hz   */
        SSD(1,0,0),   /* E_REGVFMT_1024x768p_60Hz  */
        SSD(1,0,0),   /* E_REGVFMT_1024x768p_70Hz  */
        SSD(1,0,0),   /* E_REGVFMT_1024x768p_75Hz  */
        SSD(0,0,0),   /* E_REGVFMT_1280x768p_60Hz  */
        SSD(0,0,0),   /* E_REGVFMT_1280x1024p_60Hz */
        SSD(0,0,0),   /* E_REGVFMT_1280x1024p_75Hz */
        SSD(0,0,0),   /* E_REGVFMT_1360x768p_60Hz  */
        SSD(0,0,0),   /* E_REGVFMT_1400x1050p_60Hz */
        SSD(0,0,0)    /* E_REGVFMT_1600x1200p_60Hz */
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
    0, /* E_REGVFMT_1920x1080p_50Hz */
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
    0, /* E_REGVFMT_1280x1024p_75Hz */
    0, /* E_REGVFMT_1360x768p_60Hz  */
    0, /* E_REGVFMT_1400x1050p_60Hz */
    0, /* E_REGVFMT_1600x1200p_60Hz */
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
    0, /* E_REGVFMT_1920x1080p_50Hz */
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
    0, /* E_REGVFMT_1280x1024p_75Hz */
    0, /* E_REGVFMT_1360x768p_60Hz  */
    1, /* E_REGVFMT_1400x1050p_60Hz */
    0, /* E_REGVFMT_1600x1200p_60Hz */
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
    {529,   2,      721,        42},     /* E_REGVFMT_1920x1080p_50Hz */
	{639,   2,		831,        42},     /* E_REGVFMT_1920x1080p_24Hz */
	{529,   2,		721,        42},     /* E_REGVFMT_1920x1080p_25Hz */
	{89,    2,		281,        42}      /* E_REGVFMT_1920x1080p_30Hz */
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
    {49,    2,      501,        42}, /* E_REGVFMT_1280x1024p_60Hz */
    {17,    2,      411,        42}, /* E_REGVFMT_1280x1024p_75Hz */
    {65,    2,      435,        25}, /* E_REGVFMT_1360x768p_60Hz */
    {89,    2,      467,        37}, /* E_REGVFMT_1400x1050p_60Hz */
    {65,	2,		563,		50}  /* E_REGVFMT_1600x1200p_60Hz */
#endif/* FORMAT_PC */
};

static CONST_DAT tmHdmiTxVidReg_t kVidFmtFormatParam[E_REGVFMT_NUM_TV_NO_REG] =
{
/*  NPIX    NLINE  VsLineStart	VsPixStart	VsLineEnd	VsPixEnd	HsStart		HsEnd	vWinStart	vWinEnd	DeStart	DeEnd */
    {2750,	1125,	  1,	      638,	       6,	        638,        638,    682,    41,         1121,   830,   2750},/* E_REGVFMT_1920x1080p_24Hz */
    {2640,	1125,	  1,	      528,	       6,	        528,	    528,	572,    41,         1121,   720,   2640},/* E_REGVFMT_1920x1080p_25Hz */
    {2200,	1125,	  1,	      88,	       6,	        88,	        88,	    132,    41,         1121,   280,   2200},/* E_REGVFMT_1920x1080p_30Hz */

};

#ifdef FORMAT_PC
static CONST_DAT tmHdmiTxVidReg_t kVidFmtToPCFormatParam[E_REGVFMT_NUM_PC] =
{
/*  NPIX    NLINE  VsLineStart	VsPixStart	VsLineEnd	VsPixEnd	HsStart		HsEnd	vWinStart	vWinEnd	DeStart	DeEnd */
    {832,	520,      1,          24,	       4,	        24,	        24,	    64,	    31,	        511,    192,    832},/* E_REGVFMT_640x480p_72Hz   */
    {840,	500,      1,	      16,	       4,	        16,         16,	    80,	    19,	        499,    200,    840},/* E_REGVFMT_640x480p_75Hz   */
    {832,	509,      1,	      56,	       4,	        56,	        56,	    112,    28,         508,    192,    832},/* E_REGVFMT_640x480p_85Hz   */
    {1056,	628,	  1,	      40,	       5,	        40,	        40,    	168,    27,         627,    256,   1056},/* E_REGVFMT_800x600p_60Hz   */
    {1040,	666,	  1,	      56,	       7,	        56,	        56,	    176,    29,         619,    240,   1040},/* E_REGVFMT_800x600p_72Hz   */
    {1056,	625,	  1,	      16,	       4,           16,	        16,	    96,     24,         624,    256,   1056},/* E_REGVFMT_800x600p_75Hz   */
    {1048,	631,	  1,	      32,	       4,	        32,	        32,	    96,     30,         630,    248,   1048},/* E_REGVFMT_800x600p_85Hz   */
    {1344,	806,	  1,	      24,	       7,	        24,	        24,	    160,    35,         803,    320,   1344},/* E_REGVFMT_1024x768p_60Hz  */
    {1328,	806,	  1,	      24,	       7,	        24,	        24,	    160,    35,         803,    304,   1328},/* E_REGVFMT_1024x768p_70Hz  */
    {1312,	800,	  1,	      16,	       4,	        16,	        16,	    112,    31,         799,    288,   1312},/* E_REGVFMT_1024x768p_75Hz  */
    {1664,	798,	  1,	      64,	       8,	        64,	        64,	    192,    27,         795,    384,   1664},/* E_REGVFMT_1280x768p_60Hz  */
    {1688,	1066,	  1,	      48,	       4,	        48,	        48,	    160,    41,         1065,   408,   1688},/* E_REGVFMT_1280x1024p_60Hz */
    {1688,	1066,	  1,	      16,	       4,	        16,	        16,	    160,    41,         1065,   408,   1688},/* E_REGVFMT_1280x1024p_75Hz */
    {1792,	795,	  1,	      64,	       7,	        64,	        64,	    176,    24,         792,    432,   1792},/* E_REGVFMT_1360x768p_60Hz  */
    {1864,	1089,	  1,	      88,	       5,	        88,	        88,	    232,    36,         1086,   464,   1864},/* E_REGVFMT_1400x1050p_60Hz */
    {2160,	1250,	  1,	      64,	       4,	        64,	        64,	    256,    49,         1249,   560,   2160}/* E_REGVFMT_1600x1200p_60Hz */
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
   {135000, 150000, 135000, 150000, 135000, 150000, 135000}, /* E_PIXCLK_135000 */
   {162000, 180000, 162000, 180000, 162000, 180000, 162000}  /* E_PIXCLK_162000*/
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
   { 4096,  6272,  6144,  12544,  12288,  25088,  24576}, /* E_PIXCLK_135000*/
   { 4096,  6272,  6144,  12544,  12288,  25088,  24576}  /* E_PIXCLK_162000*/
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
   {5,      4,      4,      3,      3,      2,      2},     /* E_PIXCLK_148350*/
   {5,      4,      4,      3,      3,      2,      2}      /* E_PIXCLK_148500*/
#ifdef FORMAT_PC
  ,{2,      2,      2,      1,      1,      0,      0}, /* E_PIXCLK_31500 */
   {3,      2,      2,      1,      1,      0,      0}, /* E_PIXCLK_36000 */
   {3,      2,      2,      1,      1,      0,      0}, /* E_PIXCLK_40000 */
   {3,      3,      3,      2,      2,      1,      1}, /* E_PIXCLK_49500 */
   {3,      3,      3,      2,      2,      1,      1}, /* E_PIXCLK_50000 */
   {3,      3,      3,      2,      2,      1,      1}, /* E_PIXCLK_56250 */
   {4,      3,      3,      2,      2,      1,      1}, /* E_PIXCLK_65000 */
   {4,      3,      3,      2,      2,      1,      1}, /* E_PIXCLK_75000 */
   {4,      3,      3,      2,      2,      1,      1}, /* E_PIXCLK_78750 */
   {5,      4,      4,      3,      3,      2,      2}, /* E_PIXCLK_135000*/
   {5,      4,      4,      3,      3,      2,      2}  /* E_PIXCLK_162000*/
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
    1,      /* HDMITX_AFS_NOT_INDICATED */
};

/**
 *  Lookup table for each pixel clock frequency index's nearest MHz value
 */
static CONST_DAT UInt8 kPixClkToMHz[E_PIXCLK_NUM] =
{
   25,     /* E_PIXCLK_25175 */
   25,     /* E_PIXCLK_25200 */
   27,     /* E_PIXCLK_27000 */
   27,     /* E_PIXCLK_27027 */
   54,     /* E_PIXCLK_54000 */
   54,     /* E_PIXCLK_54054 */
   74,     /* E_PIXCLK_74175 */
   74,     /* E_PIXCLK_74250 */
   148,    /* E_PIXCLK_148350*/
   148     /* E_PIXCLK_148500*/
#ifdef FORMAT_PC
  ,31, /* E_PIXCLK_31500 */
   36, /* E_PIXCLK_36000 */
   40, /* E_PIXCLK_40000 */
   49, /* E_PIXCLK_49500 */
   50, /* E_PIXCLK_50000 */
   56, /* E_PIXCLK_56250 */
   65, /* E_PIXCLK_65000 */
   75, /* E_PIXCLK_75000 */
   79, /* E_PIXCLK_78750 */
   79, /* E_PIXCLK_79500 */
   85, /* E_PIXCLK_85500 */
   108, /* E_PIXCLK_108000 */
   122, /* E_PIXCLK_121750 */
   135, /* E_PIXCLK_135000 */
   162  /* E_PIXCLK_162000 */
#endif /* FORMAT_PC */   
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
 * Table of shadow registers, as packed Shad/Page/Addr codes.
 * This allows shadow index values to be searched for using register page
 * and address values.
 */
static CONST_DAT UInt16 kShadowReg[E_SNUM] =
{/* Shadow Index                  Packed Shad/Page/Addr */
    E_REG_P00_MAIN_CNTRL0_W ,    /* E_SP00_MAIN_CNTRL0  */
    E_REG_P00_INT_FLAGS_0_RW,    /* E_SP00_INT_FLAGS_0  */
    E_REG_P00_INT_FLAGS_1_RW,    /* E_SP00_INT_FLAGS_1  */
#ifdef TMFL_TDA9981_SUPPORT
    E_REG_P00_INT_FLAGS_2_RW,    /* E_SP00_INT_FLAGS_2  */
#endif /* TMFL_TDA9981_SUPPORT */
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
    E_REG_P00_AIP_CLKSEL_W       /* E_SP00_AIP_CLKSEL   */
#ifndef TMFL_TDA9981_SUPPORT
    ,E_REG_P01_SC_VIDFORMAT_W    /* E_SP01_SC_VIDFORMAT */
    ,E_REG_P01_SC_CNTRL_W        /* E_SP01_SC_CNTRL     */
    ,E_REG_P01_TBG_CNTRL_0_W     /* E_SP01_TBG_CNTRL_0  */
#endif /* TMFL_TDA9981_SUPPORT */
#ifndef NO_HDCP
    ,E_REG_P12_HDCP_CTRL_W       /* E_SP12_HDCP_CTRL    */
#ifdef BCAPS_REPEATER
    ,E_REG_P12_HDCP_BCAPS_W      /* E_SP12_HDCP_BCAPS   */
#endif /* BCAPS_REPEATER */
#endif /* NO_HDCP */
};

/**
 * Lookup table of input port control registers and their swap and mirror masks
 */
static CONST_DAT struct _tmbslHdmiTxRegVip
{
    UInt16 Register;
    UInt8  MaskSwap;
    UInt8  MaskMirror;
} kRegVip[HDMITX_VIN_PORT_MAP_TABLE_LEN] =
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

/**
 * Table of registers to switch scaler off
 */
static CONST_DAT tmHdmiTxRegMaskVal_t kScalerOff[] =
{
    /* Scaler is bypassed */
    {E_REG_P00_MAIN_CNTRL0_W,   E_MASKREG_P00_MAIN_CNTRL0_scaler,   0},
    /* Bypass PLL scaling */
    {E_REG_P02_PLL_SCG2_RW,     E_MASKREG_P02_PLL_SCG2_bypass_scg,  1},
    /* Disable scaler clock */
    {E_REG_P02_SEL_CLK_RW,      E_MASKREG_P02_SEL_CLK_ena_sc_clk,   0},
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
    {E_REG_P00_MAIN_CNTRL0_W,   E_MASKREG_P00_MAIN_CNTRL0_scaler,   1},
    /* Don't bypass PLL scaling */
    {E_REG_P02_PLL_SCG2_RW,     E_MASKREG_P02_PLL_SCG2_bypass_scg,  0},
    /* Enable scaler clock */
    {E_REG_P02_SEL_CLK_RW,      E_MASKREG_P02_SEL_CLK_ena_sc_clk,   1},
    /* PLL loop active */
    {E_REG_P02_PLL_SCG1_RW,     E_MASKREG_P02_PLL_SCG1_scg_fdn,     0},
    {0,0,0}
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
#ifdef TMFL_TDA9981_SUPPORT
	{E_REG_P00_CCLK_ON_RW,		E_MASKREG_P00_CCLK_ON_cclk_on,		0},
#endif /* TMFL_TDA9981_SUPPORT */
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
#ifdef TMFL_TDA9981_SUPPORT
	{E_REG_P00_CCLK_ON_RW,		E_MASKREG_P00_CCLK_ON_cclk_on,		1},
#endif /* TMFL_TDA9981_SUPPORT */
    {0,0,0}
};

/**
 * Table of registers to switch HDMI HDCP mode off for DVI
 */
static CONST_DAT tmHdmiTxRegMaskVal_t kVoutHdcpOff[] =
{
    {E_REG_P00_TBG_CNTRL_1_W,   E_MASKREG_P00_TBG_CNTRL_1_dwin_dis, 1},
    {E_REG_P12_HDCP_TX33_RW,    E_MASKREG_P12_HDCP_TX33_hdmi,       0},
    {E_REG_P12_HDCP_TX33_RW,    E_MASKREG_P12_HDCP_TX33_eess,       0},
    {E_REG_P12_HDCP_TX33_RW,    E_MASKREG_P12_HDCP_TX33_ctl_sel,    0},
    {0,0,0}
};

/**
 * Table of registers to switch HDMI HDCP mode on for HDMI
 */
static CONST_DAT tmHdmiTxRegMaskVal_t kVoutHdcpOn[] =
{
    {E_REG_P00_TBG_CNTRL_1_W,   E_MASKREG_P00_TBG_CNTRL_1_dwin_dis, 0},
    {E_REG_P11_ENC_CNTRL_RW,    E_MASKREG_P11_ENC_CNTRL_ctl_code,   1},
    {E_REG_P12_HDCP_TX33_RW,    E_MASKREG_P12_HDCP_TX33_hdmi,       1},
    {E_REG_P12_HDCP_TX33_RW,    E_MASKREG_P12_HDCP_TX33_eess,       1},
    {E_REG_P12_HDCP_TX33_RW,    E_MASKREG_P12_HDCP_TX33_ctl_sel,    1},
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
 * Table of registers to set 5s watchdog timer
 */
static CONST_DAT tmHdmiTxRegMaskVal_t kSet5secWatchdog[] =
{
    /* Watchdog timer reference = CCLK divide by 32 / 100 = 10 kHz */
    /* PNF57 {E_REG_P00_NDIV_PF_W,       E_MASKREG_ALL,  100}, */
    /* CCLK divide by 32 = 1MHz */
    {E_REG_P00_TIMER_H_W,       E_MASKREG_P00_TIMER_H_wd_clksel,    1},
    /* Set 5s watchdog (registers set to 100,000) */
    {E_REG_P00_TIMER_H_W,       E_MASKREG_P00_TIMER_H_tim_h,        0x02},
    {E_REG_P00_TIMER_M_W,       E_MASKREG_ALL,                      0xb0},
    {E_REG_P00_TIMER_L_W,       E_MASKREG_ALL,                      0x00},
    {0,0,0}
};

/**
 * Table of registers to set 120ms watchdog timer based on HDMI clock
 */
static CONST_DAT tmHdmiTxRegMaskVal_t kSet120msWatchdog[] =
{
    /* Set HDMI clock as watchdog */
    {E_REG_P00_TIMER_H_W,       E_MASKREG_P00_TIMER_H_wd_clksel,    0},
    /* Set 120ms watchdog (registers set to 120,000us) */
    {E_REG_P00_TIMER_H_W,       E_MASKREG_P00_TIMER_H_tim_h,        0x01},
    {E_REG_P00_TIMER_M_W,       E_MASKREG_ALL,                      0xD4},
    {E_REG_P00_TIMER_L_W,       E_MASKREG_ALL,                      0xC0},
    {0,0,0}
};

/**
 * Table of registers to set 1s watchdog timer based on HDMI clock use for phase3
 */
static CONST_DAT tmHdmiTxRegMaskVal_t kSet1sWatchdog[] =
{
    /* PNF57 Set HDMI clock as watchdog */
    {E_REG_P00_TIMER_H_W,       E_MASKREG_P00_TIMER_H_wd_clksel,    0},
    /* Set 600ms watchdog for link check */
    {E_REG_P00_TIMER_H_W,       E_MASKREG_P00_TIMER_H_tim_h,        0x00},
    {E_REG_P00_TIMER_M_W,       E_MASKREG_ALL,                      0x75},
    {E_REG_P00_TIMER_L_W,       E_MASKREG_ALL,                      0x30},
    {0,0,0}
};

#ifndef NO_HDCP
/**
 * Table of registers to declare that BKSV is secure
 */
static CONST_DAT tmHdmiTxRegMaskVal_t kBksvSecure[] =
{
    /* BKSV is secure */
    {E_REG_P12_HDCP_CTRL_W,     E_MASKREG_P12_HDCP_CTRL_hdcp_ena,   1},
    {E_REG_P12_HDCP_TX1_RW,     E_MASKREG_P12_HDCP_TX1_bksv_nok,    0},
    {E_REG_P12_HDCP_TX1_RW,     E_MASKREG_P12_HDCP_TX1_bksv_ok,     1},
    {E_REG_P12_HDCP_TX1_RW,     E_MASKREG_P12_HDCP_TX1_ksvl_ok,     0},

    {E_REG_P12_HDCP_TX1_RW,     E_MASKREG_P12_HDCP_TX1_ksvl_nok,    0},

    /* Disable BCAPS interrupt: must not handle it while awaiting BSTATUS */
    {E_REG_P00_INT_FLAGS_0_RW,  E_MASKREG_P00_INT_FLAGS_0_bcaps,    0},
    {0,0,0}
};

/**
 * Table of registers to declare that BKSV is not secure
 */
static CONST_DAT tmHdmiTxRegMaskVal_t kBksvNotSecure[] =
{
    /* BKSV is not secure */
    {E_REG_P12_HDCP_CTRL_W,     E_MASKREG_P12_HDCP_CTRL_hdcp_ena,   1},
    {E_REG_P12_HDCP_TX1_RW,     E_MASKREG_P12_HDCP_TX1_bksv_ok,     0},
    {E_REG_P12_HDCP_TX1_RW,     E_MASKREG_P12_HDCP_TX1_bksv_nok,    1},
    {E_REG_P12_HDCP_TX1_RW,     E_MASKREG_P12_HDCP_TX1_ksvl_ok,     0},

    {E_REG_P12_HDCP_TX1_RW,     E_MASKREG_P12_HDCP_TX1_ksvl_nok,    0},

    /* Re-enable BCAPS interrupt */
    {E_REG_P00_INT_FLAGS_0_RW,  E_MASKREG_P00_INT_FLAGS_0_bcaps,    1},
    {0,0,0}
};



/**

 * Table of registers to declare that KSV list is secure

 */

static CONST_DAT tmHdmiTxRegMaskVal_t kKsvLSecure[] =

{

    /* KSV list is secure */

    {E_REG_P12_HDCP_CTRL_W,     E_MASKREG_P12_HDCP_CTRL_hdcp_ena,   1},

    {E_REG_P12_HDCP_TX1_RW,     E_MASKREG_P12_HDCP_TX1_bksv_nok,    0},

    {E_REG_P12_HDCP_TX1_RW,     E_MASKREG_P12_HDCP_TX1_bksv_ok,     0},

    {E_REG_P12_HDCP_TX1_RW,     E_MASKREG_P12_HDCP_TX1_ksvl_ok,     1},

    {E_REG_P12_HDCP_TX1_RW,     E_MASKREG_P12_HDCP_TX1_ksvl_nok,    0},

    {0,0,0}

};



/**

 * Table of registers to declare that BKSV is not secure

 */

static CONST_DAT tmHdmiTxRegMaskVal_t kKsvLNotSecure[] =

{

    /* BKSV is not secure */

    {E_REG_P12_HDCP_CTRL_W,     E_MASKREG_P12_HDCP_CTRL_hdcp_ena,   1},

    {E_REG_P12_HDCP_TX1_RW,     E_MASKREG_P12_HDCP_TX1_bksv_ok,     0},

    {E_REG_P12_HDCP_TX1_RW,     E_MASKREG_P12_HDCP_TX1_bksv_nok,    0},

    {E_REG_P12_HDCP_TX1_RW,     E_MASKREG_P12_HDCP_TX1_ksvl_ok,     0},

    {E_REG_P12_HDCP_TX1_RW,     E_MASKREG_P12_HDCP_TX1_ksvl_nok,    1},

    {0,0,0}

};












/**
 * Table of registers to recover from HDCP T0 interrupt
 */
static CONST_DAT tmHdmiTxRegMaskVal_t kT0Recovery[] =
{
    /* Stop HDCP */
    {E_REG_P12_HDCP_TX0_RW,     E_MASKREG_P12_HDCP_TX0_sr_hdcp,     1},
    /* TX advance cipher not supported */
    {E_REG_P12_HDCP_BCAPS_W,    E_MASKREG_P12_HDCP_BCAPS_1_1,       0},
    /* Init BKSV no specific action */
    {E_REG_P12_HDCP_TX1_RW,     E_MASKREG_ALL,                      0},
    /* Wait for BCAPS check */
    {E_REG_P12_HDCP_TX0_RW,     E_MASKREG_P12_HDCP_TX0_ainfo_rdy,   0},
    /* Disable XOR encryption */
    {E_REG_P12_HDCP_CTRL_W,     E_MASKREG_P12_HDCP_CTRL_hdcp_ena,   0},
    {0,0,0}
};

/**
 * Table of registers to initialise HDCP (part 1)
 */
static CONST_DAT tmHdmiTxRegMaskVal_t kHdcpInit1[] =
{
    /* Added since GS HDCP 2.0 */
    {E_REG_P12_HDCP_TX33_RW,    E_MASKREG_P12_HDCP_TX33_sys_req,    1},
    /* Must be set before HDCP */
    {E_REG_P12_HDCP_TX0_RW,     E_MASKREG_P12_HDCP_TX0_sr_hdcp,     1},
    /* Enable DDC master */
    {E_REG_P12_HDCP_TX0_RW,     E_MASKREG_P12_HDCP_TX0_mst_en,      1},
    {0,0,0}
};

/**
 * Table of registers to initialise HDCP (part 2)
 * (Not N4)
 */
static CONST_DAT tmHdmiTxRegMaskVal_t kHdcpInit2[] =
{
    /* Disable XOR encryption */
    {E_REG_P12_HDCP_CTRL_W,     E_MASKREG_P12_HDCP_CTRL_hdcp_ena,   0},
    /* DDC clock = cclk/3 = 10MHz */
    {E_REG_P00_TIMER_H_W,       E_MASKREG_P00_TIMER_H_im_clksel,    1},
    {0,0,0}
};

#ifdef DDC_MASTER_WORKAROUND
/**
 * Table of registers to initialise HDCP (part 2a)
 * (N4 DDC MASTER WORKAROUND)
 */
static CONST_DAT tmHdmiTxRegMaskVal_t kHdcpInit2a[] =
{
    /* Disable XOR encryption */
    {E_REG_P12_HDCP_CTRL_W,     E_MASKREG_P12_HDCP_CTRL_hdcp_ena,   0},
    /* DDC clock = clocked by hdmi_clk/(ndiv_im + 1) */
    {E_REG_P00_TIMER_H_W,       E_MASKREG_P00_TIMER_H_im_clksel,    0},
    /* DDC master clock divider */
    {E_REG_P00_NDIV_IM_W,       E_MASKREG_ALL,                      9},
    {0,0,0}
};
#endif

/**
 * Table of registers to initialise HDCP (part 3)
 */
static CONST_DAT tmHdmiTxRegMaskVal_t kHdcpInit3[] =
{
    /* TX advance cipher not supported */
    {E_REG_P12_HDCP_BCAPS_W,    E_MASKREG_P12_HDCP_BCAPS_1_1,       0},
    /* Init BKSV no specific action */
    {E_REG_P12_HDCP_TX1_RW,     E_MASKREG_ALL,                      0},
    /* Wait for BCAPS check */
    {E_REG_P12_HDCP_TX0_RW,     E_MASKREG_P12_HDCP_TX0_ainfo_rdy,   0},
    {0,0,0}
};
#endif /* NO_HDCP */

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
 * Table of registers to configure video input mode CCIR656
 */
static CONST_DAT tmHdmiTxRegMaskVal_t kVinModeCCIR656[] =
{
    {E_REG_P00_VIP_CNTRL_4_W,   E_MASKREG_P00_VIP_CNTRL_4_ccir656,      1},
    {E_REG_P00_HVF_CNTRL_1_W,   E_MASKREG_P00_HVF_CNTRL_1_semi_planar,  1},
    {E_REG_P02_SEL_CLK_RW,      E_MASKREG_P02_SEL_CLK_sel_clk1,         1},
    {E_REG_P02_PLL_SERIAL_3_RW, E_MASKREG_P02_PLL_SERIAL_3_srl_ccir,    1},
    {0,0,0}
};

/**
 * Table of registers to configure video input mode RGB444 or YUV444
 */
static CONST_DAT tmHdmiTxRegMaskVal_t kVinMode444[] =
{
    {E_REG_P00_VIP_CNTRL_4_W,   E_MASKREG_P00_VIP_CNTRL_4_ccir656,      0},
    {E_REG_P00_HVF_CNTRL_1_W,   E_MASKREG_P00_HVF_CNTRL_1_semi_planar,  0},
    {E_REG_P02_SEL_CLK_RW,      E_MASKREG_P02_SEL_CLK_sel_clk1,         0},
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
    {E_REG_P02_SEL_CLK_RW,      E_MASKREG_P02_SEL_CLK_sel_clk1,         0},
    {E_REG_P02_PLL_SERIAL_3_RW, E_MASKREG_P02_PLL_SERIAL_3_srl_ccir,    0},
    {E_REG_P02_SEL_CLK_RW,      E_MASKREG_P02_SEL_CLK_sel_vrf_clk,      0},
    {0,0,0}
};

/**
 * Table of PLL settings registers to configure for all video input format (vinFmt)
 */
static CONST_DAT tmHdmiTxRegMaskVal_t kCommonPllCfg[] =
{
    {E_REG_P02_PLL_SERIAL_1_RW, E_MASKREG_ALL,  0x00},
    {E_REG_P02_PLL_SERIAL_2_RW, E_MASKREG_ALL,  0x01},
    {E_REG_P02_PLL_SERIAL_3_RW, E_MASKREG_ALL,  0x01},
    {E_REG_P02_SERIALIZER_RW,   E_MASKREG_ALL,  0x00},
    {E_REG_P02_BUFFER_OUT_RW,   E_MASKREG_ALL,  0x00},
    {E_REG_P02_CCIR_DIV_RW,     E_MASKREG_ALL,  0x00},
	{E_REG_P02_PLL_SCG1_RW,     E_MASKREG_ALL,  0x00},
	{E_REG_P02_AUDIO_DIV_RW,    E_MASKREG_ALL,  0x03},
	{E_REG_P02_TEST2_RW,        E_MASKREG_ALL,  0x00},
	{E_REG_P02_SEL_CLK_RW,      E_MASKREG_ALL,  0x09},
    {0,0,0}
};

/**
 * Table of PLL settings registers to configure for 480i and 576i vinFmt
 */
static CONST_DAT tmHdmiTxRegMaskVal_t kVfmt480i576iPllCfg[] =
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
static CONST_DAT tmHdmiTxRegMaskVal_t kVfmtOtherPllCfg[] =
{
    {E_REG_P02_PLL_SCGN1_RW,    E_MASKREG_ALL,  0xFA},
    {E_REG_P02_PLL_SCGN2_RW,    E_MASKREG_ALL,  0x00},
    {E_REG_P02_PLL_SCGR1_RW,    E_MASKREG_ALL,  0x5B},
    {E_REG_P02_PLL_SCGR2_RW,    E_MASKREG_ALL,  0x00},
    {0,0,0}
};

/**
 * Table of PLL settings registers to configure for single mode pixel rate,
 * vinFmt 480i or 576i only
 */
static CONST_DAT tmHdmiTxRegMaskVal_t kSinglePrateVfmt480i576iPllCfg[] =
{
    {E_REG_P02_PLL_SCG2_RW,     E_MASKREG_ALL,  0x11},
    {E_REG_P02_PLL_DE_RW,       E_MASKREG_ALL,  0xA1},
    {0,0,0}
};

/**
 * Table of PLL settings registers to configure for single repeated mode pixel rate,
 * vinFmt 480i or 576i only
 */
static CONST_DAT tmHdmiTxRegMaskVal_t kSrepeatedPrateVfmt480i576iPllCfg[] =
{
    {E_REG_P02_PLL_SCG2_RW,     E_MASKREG_ALL,  0x01},
    {E_REG_P02_PLL_DE_RW,       E_MASKREG_ALL,  0xA1},
    {E_REG_P02_CCIR_DIV_RW,     E_MASKREG_ALL,  0x01},
    {0,0,0}
};

/**
 * Table of PLL settings registers to configure single mode pixel rate,
 * vinFmt other than 480i or 576i
 */
static CONST_DAT tmHdmiTxRegMaskVal_t kSinglePrateVfmtOtherPllCfg[] =
{
    {E_REG_P02_PLL_SCG2_RW,     E_MASKREG_ALL,  0x10},
    {E_REG_P02_PLL_DE_RW,       E_MASKREG_ALL,  0x91},
    {0,0,0}
};

/**
 * Table of PLL settings registers to configure double mode pixel rate,
 * vinFmt other than 480i or 576i
 */
static CONST_DAT tmHdmiTxRegMaskVal_t kDoublePrateVfmtOtherPllCfg[] =
{
    {E_REG_P02_PLL_SCG2_RW,     E_MASKREG_ALL,  0x00},
    {E_REG_P02_PLL_DE_RW,       E_MASKREG_ALL,  0x10},
    {0,0,0}
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


/*============================================================================*/
/*                   STATIC VARIABLE DECLARATIONS                             */
/*============================================================================*/

/* Flag per device used to ignore 1st Encrypt interrupt after HdcpInit */
static RAM_DAT  Bool    gIgnoreEncrypt[HDMITX_UNITS_MAX];

/* Register values per device to restore colour processing after test pattern */
static RAM_DAT  UInt8   gMatContrl[HDMITX_UNITS_MAX];
static RAM_DAT  UInt8   gHvfCntrl0[HDMITX_UNITS_MAX];
static RAM_DAT  UInt8   gHvfCntrl1[HDMITX_UNITS_MAX];

/*============================================================================*/
/*                   STATIC FUNCTION DECLARATIONS                             */
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
static tmErrorCode_t    handleBKSVResultSecure(tmUnitSelect_t txUnit);

/*============================================================================*/
/*                   PUBLIC FUNCTION DEFINITIONS                              */
/*============================================================================*/

/*============================================================================*/
/* tmbslHdmiTxAudioInResetCts                                                 */
/*============================================================================*/
#ifndef DEMO_BUILD
tmErrorCode_t
tmbslHdmiTxAudioInResetCts
(
    tmUnitSelect_t	            txUnit
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
#endif /* DEMO_BUILD */

/*============================================================================*/
/* tmbslHdmiTxAudioInSetConfig                                                */
/*============================================================================*/
tmErrorCode_t
tmbslHdmiTxAudioInSetConfig
(
    tmUnitSelect_t           txUnit,
    tmbslHdmiTxaFmt_t        aFmt,
    UInt8                    chanI2s,
    UInt8                    chanDsd,
    tmbslHdmiTxClkPolDsd_t   clkPolDsd,
    tmbslHdmiTxSwapDsd_t     swapDsd,
    UInt8                    layout,
    UInt16                   uLatency_rd
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

    /* Check remaining parameters */
    RETIF_BADPARAM(aFmt        >= HDMITX_AFMT_INVALID)
    RETIF_BADPARAM(chanI2s     >= HDMITX_CHAN_INVALID)
    RETIF_BADPARAM(chanDsd     >= HDMITX_CHAN_INVALID)
    RETIF_BADPARAM(clkPolDsd   >= HDMITX_CLKPOLDSD_INVALID)
    RETIF_BADPARAM(swapDsd     >= HDMITX_SWAPDSD_INVALID)
    RETIF_BADPARAM(layout      >= HDMITX_LAYOUT_INVALID)
    RETIF_BADPARAM(uLatency_rd >= HDMITX_LATENCY_INVALID)

    /* Fold OBA onto DSD */
    if (aFmt == HDMITX_AFMT_OBA)
    {
        aFmt = HDMITX_AFMT_DSD;
    }

    /* Set the audio input processor format to aFmt. */
    err = setHwRegisterField(pDis,
                             E_REG_P00_AIP_CLKSEL_W,
                             E_MASKREG_P00_AIP_CLKSEL_sel_aip,
                             (UInt8)aFmt);
    RETIF_REG_FAIL(err)

    /* Select the audio format */
    if (aFmt == HDMITX_AFMT_I2S)
    {
        if (chanI2s != HDMITX_CHAN_NO_CHANGE)
        {
            err = setHwRegister(pDis, E_REG_P11_CA_I2S_RW, chanI2s);
            RETIF_REG_FAIL(err)
        }
    }
    else if (aFmt == HDMITX_AFMT_DSD)
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
/* tmbslHdmiTxAudioInSetCts                                                   */
/*============================================================================*/
tmErrorCode_t
tmbslHdmiTxAudioInSetCts
(
    tmUnitSelect_t       txUnit,
    tmbslHdmiTxctsRef_t  ctsRef,
    tmbslHdmiTxafs_t     afs, 
    tmbslHdmiTxVidFmt_t  voutFmt,
    tmbslHdmiTxVfreq_t   voutFreq, 
    UInt32               uCts, 
    UInt16               uCtsX,
    tmbslHdmiTxctsK_t    ctsK,
    tmbslHdmiTxctsM_t    ctsM
)
{
    tmHdmiTxobject_t *pDis;     /* Pointer to Device Instance Structure */
    tmErrorCode_t   err;        /* Error code */
    UInt8           regVal;     /* Register value */
    UInt8           pixClk;     /* Pixel clock index */
    UInt32          acrN;       /* Audio clock recovery N */

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
        ctsM = (tmbslHdmiTxctsM_t)kCtsXToMK[uCtsX][0];
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
/* tmbslHdmiTxAudioOutSetChanStatus                                           */
/*============================================================================*/
tmErrorCode_t
tmbslHdmiTxAudioOutSetChanStatus
(
    tmUnitSelect_t               txUnit,
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

    /* Prepare Byte 0 */
    if(formatInfo == HDMITX_CSFI_NOTPCM_DEFAULT)
    {
        buf[0] = ((UInt8)copyright << 2) | 2;   /* Set NOT_PCM bit2 */
    }
    else
    {
        buf[0] = ((UInt8)formatInfo << 3) | ((UInt8)copyright << 2);
    }

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
/* tmbslHdmiTxAudioOutSetChanStatusMapping                                    */
/*============================================================================*/
#ifndef DEMO_BUILD
tmErrorCode_t
tmbslHdmiTxAudioOutSetChanStatusMapping
(
    tmUnitSelect_t  txUnit,
    UInt8           sourceLeft,
    UInt8           channelLeft,
    UInt8           sourceRight,
    UInt8           channelRight
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
    RETIF_BADPARAM(sourceLeft   > HDMITX_CS_SOURCES_MAX)
    RETIF_BADPARAM(channelLeft  > HDMITX_CS_CHANNELS_MAX)
    RETIF_BADPARAM(sourceRight  > HDMITX_CS_SOURCES_MAX)
    RETIF_BADPARAM(channelRight > HDMITX_CS_CHANNELS_MAX)

    /* Prepare Left byte */
    buf[0] = ((UInt8)channelLeft << 4) | (UInt8)sourceLeft;

    /* Prepare Right byte */
    buf[1] = ((UInt8)channelRight << 4) | (UInt8)sourceRight;

    /* Write 2 Channel Status bytes */
    err = setHwRegisters(pDis, E_REG_P11_CH_STAT_B_2_ap0_l_RW, &buf[0], 2);
    return err;
}
#endif /* DEMO_BUILD */

/*============================================================================*/
/* tmbslHdmiTxAudioOutSetMute                                                 */
/*============================================================================*/
tmErrorCode_t
tmbslHdmiTxAudioOutSetMute
(
    tmUnitSelect_t      txUnit,
    tmbslHdmiTxaMute_t  aMute
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

    /* Check remaining parameters */
    RETIF_BADPARAM(aMute >= HDMITX_AMUTE_INVALID)

    /* Reset the audio FIFO to mute audio */
    err = setHwRegisterField(pDis,
                             E_REG_P11_AIP_CNTRL_0_RW,
                             E_MASKREG_P11_AIP_CNTRL_0_rst_fifo,
                             (UInt8)aMute);
    return err;
}

/*============================================================================*/
/* tmbslHdmiTxDeinit                                                          */
/*============================================================================*/
tmErrorCode_t
tmbslHdmiTxDeinit
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

#ifndef NO_HDCP
/*============================================================================*/
/* tmbslHdmiTxHdcpCheck                                                       */
/*============================================================================*/
tmErrorCode_t
tmbslHdmiTxHdcpCheck
(
    tmUnitSelect_t	        txUnit,
    UInt16                  uTimeSinceLastCallMs,
    tmbslHdmiTxHdcpCheck_t  *pResult
)
{
    tmHdmiTxobject_t *pDis;     /* Pointer to Device Instance Structure */
    tmErrorCode_t     err;      /* Error code */
    UInt8             regVal;   /* Byte value read from register */
    UInt16            regRi;    /* RI value read from registers */

    /* Check unit parameter and point to its object */
    err = checkUnitSetDis(txUnit, &pDis);
    RETIF(err != TM_OK, err)

    /* Check if this device supports HDCP */
    RETIF(pDis->uDeviceFeatures & E_MASKREG_P00_VERSION_not_h,
          TMBSL_ERR_HDMI_NOT_SUPPORTED)

    /* See if a function has been scheduled */
    if (pDis->HdcpFuncRemainingMs > 0)
    {
        /* Decrement scheduled period by the call interval */
        if (pDis->HdcpFuncRemainingMs > uTimeSinceLastCallMs)
        {
            /* Not yet time to schedule a function */
            pDis->HdcpFuncRemainingMs = pDis->HdcpFuncRemainingMs - uTimeSinceLastCallMs;
        }
        else
        {
            /* Time now to call the scheduled function - reset the
             * timer to ensure only one schedule call */
            pDis->HdcpFuncRemainingMs = 0;
            /* See if a scheduled function is defined */
            if (pDis->HdcpFuncScheduled != (ptmHdmiTxFunc_t)0)
            {
                /* Call the scheduled function */
                err = pDis->HdcpFuncScheduled(txUnit);
                RETIF(err != TM_OK, err)
            }
        }
    }

    /* See if there are HDCP checks to do */
    if (pDis->HdcpCheckNum == 0)
    {
        /* No checks because not started yet, or HDCP was stopped */
        *pResult = HDMITX_HDCP_CHECK_NOT_STARTED;
    }
    /* Decrement check period by the call interval */
    else if (pDis->HdcpCheckRemainingMs > uTimeSinceLastCallMs)
    {
        /* Not yet time to do a check */
        pDis->HdcpCheckRemainingMs = pDis->HdcpCheckRemainingMs - uTimeSinceLastCallMs;
    }
    else
    {
        /* Time to do a check - restart the timer for next check */
        pDis->HdcpCheckRemainingMs = pDis->HdcpCheckIntervalMs;

        /* Increment check number if not yet passed or failed */
        if (pDis->HdcpCheckNum < pDis->HdcpChecksToDo)
        {
            pDis->HdcpCheckNum++;
        }
        /* else no increment to prevent wrap to 0 and disabling the test */

        /* Test driver state at every check */
        if (pDis->state != ST_HDCP_AUTHENTICATED)
        {
            *pResult = HDMITX_HDCP_CHECK_FAIL_DRIVER_STATE;
        }
        /* Test for T0 interrupts at every check */
		else if (pDis->HdcpT0FailState != 0)
        {
            *pResult = HDMITX_HDCP_CHECK_FAIL_DEVICE_T0;
			/* reset hdcp check process */
			/* clear T0 interrupt error */
			pDis->HdcpT0FailState = 0;
			/* reset CheckNum */
			pDis->HdcpCheckNum = 1;
			/* reset CheckRemainingMs */
			pDis->HdcpCheckRemainingMs = pDis->HdcpCheckIntervalMs;

        }
        else
        {
            /* In progress until pass or fail */
            *pResult = HDMITX_HDCP_CHECK_IN_PROGRESS;

            /* Read RI LSB and MSB registers every check interval */
            err = getHwRegister(pDis, E_REG_P12_HDCP_RI0_R, &regVal);
            RETIF_REG_FAIL(err)
            regRi = regVal;
            err = getHwRegister(pDis, E_REG_P12_HDCP_RI1_R, &regVal);
            RETIF_REG_FAIL(err)
            regRi |= ((UInt16)regVal << 8);

            /* Read FSM State register every check interval */
            err = getHwRegister(pDis, E_REG_P12_HDCP_FSM_STATE_R, &regVal);
            RETIF_REG_FAIL(err)

            /* Only do history and completion checks after first time */
            if (pDis->HdcpCheckNum > 1)
            {
                if (regRi == pDis->HdcpRi)
                {
                    *pResult = HDMITX_HDCP_CHECK_FAIL_DEVICE_RI;
                }
                else if ((regVal == 0x10) && (pDis->HdcpFsmState == 0x10))
                {
                    *pResult = HDMITX_HDCP_CHECK_FAIL_DEVICE_FSM;
                }
                else if (pDis->HdcpCheckNum >= pDis->HdcpChecksToDo)
                {
                    *pResult = HDMITX_HDCP_CHECK_PASS;
                }
            }

            /* Save register values for next time */
            pDis->HdcpRi = regRi;
            pDis->HdcpFsmState = regVal;
        }
    }

    return TM_OK;
}

/*============================================================================*/
/* tmbslHdmiTxHdcpConfigure                                                   */
/*============================================================================*/
tmErrorCode_t
tmbslHdmiTxHdcpConfigure
(
    tmUnitSelect_t	         txUnit,
    UInt8                    slaveAddress,
    tmbslHdmiTxHdcpTxMode_t  txMode,
    tmbslHdmiTxHdcpOptions_t options,
    UInt16                   uCheckIntervalMs,
    UInt8                    uChecksToDo
)
{
    tmHdmiTxobject_t *pDis;     /* Pointer to Device Instance Structure */
    tmErrorCode_t     err;      /* Error code */
    UInt8             bitVal;   /* Register bit value */

    /* Check unit parameter and point to its object */
    err = checkUnitSetDis(txUnit, &pDis);
    RETIF(err != TM_OK, err)

    /* Check if this device supports HDCP */
    RETIF(pDis->uDeviceFeatures & E_MASKREG_P00_VERSION_not_h,
          TMBSL_ERR_HDMI_NOT_SUPPORTED)

    /* Check other arguments */
    RETIF_BADPARAM(txMode > HDMITX_HDCP_TXMODE_MAX)
    RETIF_BADPARAM((options & HDMITX_HDCP_OPTION_MASK_BAD) > 0)
    if (slaveAddress == HDMITX_HDCP_SLAVE_PRIMARY)
    {
        bitVal = 0;
    }
    else if (slaveAddress == HDMITX_HDCP_SLAVE_SECONDARY)
    {
        bitVal = 1;
    }
    else
    {
       return TMBSL_ERR_HDMI_BAD_PARAMETER;
    }

    /* Set slave address flag */
    err = setHwRegisterField(pDis, E_REG_P12_HDCP_TX2_RW,
                             E_MASKREG_P12_HDCP_TX2_slvaddr, bitVal);

    /* Copy aruments to Device Instance Structure */
    pDis->HdcpSlaveAddress = slaveAddress;
    pDis->HdcpTxMode       = txMode;
    pDis->HdcpOptions      = options;
    pDis->HdcpCheckIntervalMs = uCheckIntervalMs;
    pDis->HdcpChecksToDo   = uChecksToDo;

    return err;
}

/*============================================================================*/
/* tmbslHdmiTxHdcpDownloadKeys                                                */
/*============================================================================*/
tmErrorCode_t
tmbslHdmiTxHdcpDownloadKeys
(
    tmUnitSelect_t	        txUnit,
    UInt16                  seed,
    tmbslHdmiTxDecrypt_t    keyDecryption
)
{
    tmHdmiTxobject_t *pDis;     /* Pointer to Device Instance Structure */
    tmErrorCode_t     err;      /* Error code */
    UInt8             otp[3];   /* Data read from OTP */

    /* Check unit parameter and point to its object */
    err = checkUnitSetDis(txUnit, &pDis);
    RETIF(err != TM_OK, err)

    /* Check if this device supports HDCP */
    RETIF(pDis->uDeviceFeatures & E_MASKREG_P00_VERSION_not_h,
          TMBSL_ERR_HDMI_NOT_SUPPORTED)

    /* Note: Supported for N4 or later (N3 needed AKSV download from OTP
     * public space and is not supported)
     */

    /* Check other arguments */
    RETIF_BADPARAM(keyDecryption > HDMITX_HDCP_DECRYPT_MAX)

    /* Must be set before download */
    err = setHwRegisterField(pDis, E_REG_P12_HDCP_TX0_RW,
                             E_MASKREG_P12_HDCP_TX0_sr_hdcp, 1);
    RETIF_REG_FAIL(err)

    /* Set key decryption */
    err = setHwRegisterField(pDis, E_REG_P12_HDCP_CTRL_W,
                             E_MASKREG_P12_HDCP_CTRL_keyd, 
                             (UInt8)keyDecryption);
    RETIF_REG_FAIL(err)

    /* Set seed value */
    err = setHwRegisterMsbLsb(pDis, E_REG_P12_HDCP_KDS_HI_W, seed);
    RETIF_REG_FAIL(err)

    /* Reset the key index */
    err = setHwRegister(pDis, E_REG_P12_HDCP_KIDX_W, 0);
    RETIF_REG_FAIL(err)

    /* Pulse high-low to start download */
    err = setHwRegisterField(pDis, E_REG_P12_OTP_CNTRL_W,
                             E_MASKREG_P12_OTP_CNTRL_start_dl, 1);
    RETIF_REG_FAIL(err)
    err = setHwRegisterField(pDis, E_REG_P12_OTP_CNTRL_W,
                             E_MASKREG_P12_OTP_CNTRL_start_dl, 0);
    RETIF_REG_FAIL(err)

    /* Reset control register */
    err = setHwRegister(pDis, E_REG_P12_HDCP_CTRL_W, 0);
    RETIF_REG_FAIL(err)

    /* Read HDCP_AKSV4,HDCP_AKSV3 using API */
    err = tmbslHdmiTxHdcpGetOtp(txUnit, 0x7E, otp);
    RETIF(err != TM_OK, err)
    pDis->HdcpAksv[0] = otp[1]; /* OTP_DATA_ISB */
    pDis->HdcpAksv[1] = otp[2]; /* OTP_DATA_LSB */

    /* Read HDCP_AKSV2,HDCP_AKSV1,HDCP_AKSV0 using API */
    err = tmbslHdmiTxHdcpGetOtp(txUnit, 0x7F, otp);
    pDis->HdcpAksv[2] = otp[0]; /* OTP_DATA_MSB */
    pDis->HdcpAksv[3] = otp[1]; /* OTP_DATA_ISB */
    pDis->HdcpAksv[4] = otp[2]; /* OTP_DATA_LSB */

    return err;
}

/*============================================================================*/
/* tmbslHdmiTxHdcpEncryptionOn                                                */
/*============================================================================*/
#ifndef DEMO_BUILD
tmErrorCode_t
tmbslHdmiTxHdcpEncryptionOn
(
    tmUnitSelect_t	txUnit,
    Bool            bOn
)
{
    tmHdmiTxobject_t *pDis;     /* Pointer to Device Instance Structure */
    tmErrorCode_t     err;      /* Error code */

    /* Check unit parameter and point to its object */
    err = checkUnitSetDis(txUnit, &pDis);
    RETIF(err != TM_OK, err)

    /* Check if this device supports HDCP */
    RETIF(pDis->uDeviceFeatures & E_MASKREG_P00_VERSION_not_h,
          TMBSL_ERR_HDMI_NOT_SUPPORTED)

    /* Check other arguments */
    RETIF_BADPARAM(bOn > 1)

    err = setHwRegisterField(pDis, E_REG_P12_HDCP_TX33_RW,
                             E_MASKREG_P12_HDCP_TX33_ac_not, (UInt8)bOn);
    return err;
}
#endif /* DEMO_BUILD */

/*============================================================================*/
/* tmbslHdmiTxHdcpGetOtp                                                      */
/*============================================================================*/
tmErrorCode_t
tmbslHdmiTxHdcpGetOtp
(
    tmUnitSelect_t	        txUnit,
    UInt8                   otpAddress,
    UInt8                   *pOtpData
)
{
    tmHdmiTxobject_t *pDis;     /* Pointer to Device Instance Structure */
    tmErrorCode_t     err;      /* Error code */

    /* Check unit parameter and point to its object */
    err = checkUnitSetDis(txUnit, &pDis);
    RETIF(err != TM_OK, err)

    /* Check if this device supports HDCP */
    RETIF(pDis->uDeviceFeatures & E_MASKREG_P00_VERSION_not_h,
          TMBSL_ERR_HDMI_NOT_SUPPORTED)

    /* Check other arguments */
    RETIF_BADPARAM(!pOtpData)

    /* Set OTP address */
    err = setHwRegister(pDis, E_REG_P12_OTP_ADDRESS_W, otpAddress);
    RETIF_REG_FAIL(err)

    /* Pulse high-low to start read */
    err = setHwRegisterField(pDis, E_REG_P12_OTP_CNTRL_W,
                             E_MASKREG_P12_OTP_CNTRL_start_rd, 1);
    RETIF_REG_FAIL(err)
    err = setHwRegisterField(pDis, E_REG_P12_OTP_CNTRL_W,
                             E_MASKREG_P12_OTP_CNTRL_start_rd, 0);
    RETIF_REG_FAIL(err)

    /* Read OTP data registers */
    err = getHwRegister(pDis, E_REG_P12_OTP_DATA_MSB_RW, &pOtpData[0]);
    RETIF_REG_FAIL(err)
    err = getHwRegister(pDis, E_REG_P12_OTP_DATA_ISB_RW, &pOtpData[1]);
    RETIF_REG_FAIL(err)
    err = getHwRegister(pDis, E_REG_P12_OTP_DATA_LSB_RW, &pOtpData[2]);
    RETIF_REG_FAIL(err)

    return err;
}

/*============================================================================*/
/* tmbslHdmiTxHdcpGetT0FailState                                              */
/*============================================================================*/
tmErrorCode_t
tmbslHdmiTxHdcpGetT0FailState
(
    tmUnitSelect_t	txUnit,
    UInt8           *pFailState
)
{
    tmHdmiTxobject_t *pDis;     /* Pointer to Device Instance Structure */
    tmErrorCode_t     err;      /* Error code */

    /* Check unit parameter and point to its object */
    err = checkUnitSetDis(txUnit, &pDis);
    RETIF(err != TM_OK, err)

    /* Check if this device supports HDCP */
    RETIF(pDis->uDeviceFeatures & E_MASKREG_P00_VERSION_not_h,
          TMBSL_ERR_HDMI_NOT_SUPPORTED)

    /* Return if pointer is null */
    RETIF_BADPARAM(!pFailState)

    /* Return the value */
    *pFailState = pDis->HdcpT0FailState;
    return TM_OK;
}

/*============================================================================*/
/* tmbslHdmiTxHdcpHandleBCAPS                                                 */
/*============================================================================*/
tmErrorCode_t
tmbslHdmiTxHdcpHandleBCAPS
(
    tmUnitSelect_t	txUnit
)
{
    tmHdmiTxobject_t *pDis;     /* Pointer to Device Instance Structure */
    tmErrorCode_t     err;      /* Error code */

    /* Check unit parameter and point to its object */
    err = checkUnitSetDis(txUnit, &pDis);
    RETIF(err != TM_OK, err)

    /* Read sink BCAPS from DDC */
    err = getHwRegister(pDis, E_REG_P12_HDCP_BCAPS_RX_R, &pDis->HdcpBcaps);
    RETIF_REG_FAIL(err)

    /* HDCP receiver repeater options */
    if ((pDis->HdcpOptions & HDMITX_HDCP_OPTION_FORCE_NO_REPEATER) > 0)
    {
        /* Forced as not a repeater */
        err = setHwRegisterField(pDis, E_REG_P12_HDCP_TX2_RW,
                                 E_MASKREG_P12_HDCP_TX2_selrptsrc |
                                 E_MASKREG_P12_HDCP_TX2_rpt_force, 2);
        RETIF_REG_FAIL(err)
#ifdef BCAPS_REPEATER
        /* BCAPS:repeater bit forced to 0 (N4 and above) */
        err = setHwRegisterField(pDis, E_REG_P12_HDCP_BCAPS_W,
                                 E_MASKREG_P12_HDCP_BCAPS_repeater, 0);
        RETIF_REG_FAIL(err)
#endif
    }
    else if ((pDis->HdcpOptions & HDMITX_HDCP_OPTION_FORCE_REPEATER) > 0)
    {
        /* Forced as a repeater */
        err = setHwRegisterField(pDis, E_REG_P12_HDCP_TX2_RW,
                                 E_MASKREG_P12_HDCP_TX2_selrptsrc |
                                 E_MASKREG_P12_HDCP_TX2_rpt_force, 3);
        RETIF_REG_FAIL(err)
#ifdef BCAPS_REPEATER
        /* BCAPS:repeater bit follows BCAPS_RX (N4 and above) */
        err = setHwRegisterField(pDis, E_REG_P12_HDCP_BCAPS_W,
                                 E_MASKREG_P12_HDCP_BCAPS_repeater, 1);
        RETIF_REG_FAIL(err)
#endif
    }
    else
    {
        /* Obey BCAPS bit 6 */
        err = setHwRegisterField(pDis, E_REG_P12_HDCP_TX2_RW,
                                 E_MASKREG_P12_HDCP_TX2_selrptsrc |
                                 E_MASKREG_P12_HDCP_TX2_rpt_force, 0);
        RETIF_REG_FAIL(err)
#ifdef BCAPS_REPEATER
        /* BCAPS:repeater bit follows BCAPS_RX (N4 and above) */
        err = setHwRegisterField(pDis, E_REG_P12_HDCP_BCAPS_W,
                                 E_MASKREG_P12_HDCP_BCAPS_repeater,
                    (UInt8)(((pDis->HdcpBcaps &
                    E_MASKREG_P12_HDCP_BCAPS_RX_repeater) > 0) ? 1 : 0));
        RETIF_REG_FAIL(err)
#endif
    }


    /* HDCP receiver does not support 1.1 enhanced encryption, or is forced */
    if (((pDis->HdcpBcaps & E_MASKREG_P12_HDCP_BCAPS_RX_1_1) == 0)
     || ((pDis->HdcpOptions & HDMITX_HDCP_OPTION_FORCE_NO_1_1) > 0))
    {
        err = setHwRegisterField(pDis, E_REG_P12_HDCP_BCAPS_W,
                                 E_MASKREG_P12_HDCP_BCAPS_1_1, 0);
        RETIF_REG_FAIL(err)
        err = setHwRegisterField(pDis, E_REG_P12_HDCP_TX0_RW,
                                 E_MASKREG_P12_HDCP_TX0_ainfo_1, 0);
        RETIF_REG_FAIL(err)
    }
    else
    {
        err = setHwRegisterField(pDis, E_REG_P12_HDCP_BCAPS_W,
                                 E_MASKREG_P12_HDCP_BCAPS_1_1, 1);
        RETIF_REG_FAIL(err)
        err = setHwRegisterField(pDis, E_REG_P12_HDCP_TX0_RW,
                                 E_MASKREG_P12_HDCP_TX0_ainfo_1, 1);
        RETIF_REG_FAIL(err)
    }

    /** \todo Handle other BCAPS bits and HDMITX_HDCP_OPTION_FORCE_SLOW_DDC */

    /* Signal that BCAPS was read */
    err = setHwRegisterField(pDis, E_REG_P12_HDCP_TX0_RW,
                             E_MASKREG_P12_HDCP_TX0_ainfo_rdy, 1);
    RETIF_REG_FAIL(err)
    err = setHwRegisterField(pDis, E_REG_P12_HDCP_TX0_RW,
                             E_MASKREG_P12_HDCP_TX0_ainfo_rdy, 0);
    return err;
}

/*============================================================================*/
/* tmbslHdmiTxHdcpHandleBKSV                                                  */
/*============================================================================*/
tmErrorCode_t
tmbslHdmiTxHdcpHandleBKSV
(
    tmUnitSelect_t	txUnit,
    UInt8           *pBksv,
    Bool            *pbCheckRequired  /* May be null, but only for testing */
)
{
    tmHdmiTxobject_t *pDis;     /* Pointer to Device Instance Structure */
    tmErrorCode_t     err;      /* Error code */

    /* Check unit parameter and point to its object */
    err = checkUnitSetDis(txUnit, &pDis);
    RETIF(err != TM_OK, err)

    /* Return if pointer is not assigned; pbCheckRequired can be Null */
    RETIF_BADPARAM(pBksv == Null)

    /* Top level and repeater :
	 * Read BKSV, MSB first: bksv[0]=MSB, bksv[4]=LSB */
    err = getHwRegisters(pDis, E_REG_P12_HDCP_BKSV4_R, pBksv,
                         HDMITX_KSV_BYTES_PER_DEVICE);

	/* Is this driver operating as part of a Repeater? */
    if (pDis->HdcpTxMode == HDMITX_HDCP_TXMODE_REPEATER)
    {
        /* Repeater: Set 5s watchdog timer - now suppressed (PNF57) */
        /*err = setHwRegisterFieldTable(pDis, &kSet5secWatchdog[0]);
        RETIF_REG_FAIL(err)*/

        /* No revocation list check needed for a TX repeater */
    }
    /* Not Repeater: Is this driver part of a top level transmitter? */
    else if (pDis->HdcpTxMode == HDMITX_HDCP_TXMODE_TOP_LEVEL)
    {
        /* Top level: Read BKSV, MSB first: bksv[0]=MSB, bksv[4]=LSB */
		/* PR385 : now made backward */
        /*err = getHwRegisters(pDis, E_REG_P12_HDCP_BKSV4_R, pBksv,
                             HDMITX_KSV_BYTES_PER_DEVICE);*/

        /* CALLER MUST NOW CHECK BKSV AGAINST REVOCATION LIST AND CALL
         * tmbslHdmiTxHdcpHandleBKSVResult, which will set state from result */
        if (pbCheckRequired)
        {
            /* Always return True for an application; can skip for testing */
            *pbCheckRequired = True;
        }
        return err; /* State is set later by tmbslHdmiTxHdcpHandleBKSVResult */
    }
    /* else No revocation list check needed for this TX */

    /* BKSV is secure, disable BCAPS interrupt */
    err = setHwRegisterFieldTable(pDis, &kBksvSecure[0]);
    RETIF_REG_FAIL(err)

    /* Check if receiver is a repeater */
    if ((pDis->HdcpBcaps & E_MASKREG_P12_HDCP_BCAPS_RX_repeater) > 0)
    {
        /* Go to next part of authentication for a repeater */
        setState(pDis, EV_HDCP_BKSV_REPEAT);
    }
    else
    {
        /* A secure non-repeater has authenticated */
        setState(pDis, EV_HDCP_BKSV_NREPEAT);
    }
    
    /* User revocation test is not required, pBksv is not set */
    if (pbCheckRequired)
    {
        *pbCheckRequired = False;
    }
    return TM_OK;
}

/*============================================================================*/
/* tmbslHdmiTxHdcpHandleBKSVResult                                            */
/*============================================================================*/
tmErrorCode_t
tmbslHdmiTxHdcpHandleBKSVResult
(
    tmUnitSelect_t	txUnit,
    Bool            bSecure
)
{
    tmHdmiTxobject_t *pDis;     /* Pointer to Device Instance Structure */
    tmErrorCode_t     err;      /* Error code */

    /* Check unit parameter and point to its object */
    err = checkUnitSetDis(txUnit, &pDis);
    RETIF(err != TM_OK, err)

    /* Check other parameters */
    RETIF_BADPARAM(bSecure > 1)

    if (bSecure)
    {
        /* PNF57 Set 1s watchdog based on HDMI clock */
        err = setHwRegisterField(pDis, E_REG_P12_HDCP_CTRL_W,
                                 E_MASKREG_P12_HDCP_CTRL_hdcp_ena, 1);
    	RETIF_REG_FAIL(err)

#ifndef TMFL_TDA9981_SUPPORT
        /* PNF68 Arm the DIS tick handler to call handleBKSVResultSecure 
         * later */
        pDis->HdcpFuncScheduled = handleBKSVResultSecure;
        pDis->HdcpFuncRemainingMs = 160; /* millisecond delay */
#endif /* TMFL_TDA9981_SUPPORT */
    }
    else
    {
        /* BKSV is not secure, re-enable BCAPS interrupt */
        err = setHwRegisterFieldTable(pDis, &kBksvNotSecure[0]);
        RETIF_REG_FAIL(err)

        /* Authentication has failed */
        setState(pDis, EV_HDCP_BKSV_NSECURE);
    }
    return TM_OK;
}

/*============================================================================*/
/* tmbslHdmiTxHdcpHandleBSTATUS                                               */
/*============================================================================*/
tmErrorCode_t
tmbslHdmiTxHdcpHandleBSTATUS
(
    tmUnitSelect_t	txUnit,
    UInt16          *pBstatus   /* May be null */
)
{
    tmHdmiTxobject_t *pDis;     /* Pointer to Device Instance Structure */
    tmErrorCode_t     err;      /* Error code */
    UInt8             bstatus[2];

    /* Check unit parameter and point to its object */
    err = checkUnitSetDis(txUnit, &pDis);
    RETIF(err != TM_OK, err)

    /* Read BSTATUS MSB & LSB into bstatus[0] and [1] */
    err = getHwRegisters(pDis, E_REG_P12_HDCP_BSTATUS_1_R, bstatus, 2);

    /* Check result of read */
    if (err != TM_OK)
    {
        /* No T0 interrupt from device after a bad BSTATUS so do it ourselves */
        err = tmbslHdmiTxHdcpHandleT0(txUnit);/* Does a T0 state change */
        pDis->HdcpBstatus = 0;
    }
    else
    {
        /* Save BSTATUS to Device Instance Structure */
        pDis->HdcpBstatus = ((UInt16)bstatus[0] << 8) | bstatus[1];

        /* Check bstatus topology error indicators */
        if (((pDis->HdcpBstatus & HDMITX_HDCP_BSTATUS_MAX_CASCADE_EXCEEDED) > 0)
        ||  ((pDis->HdcpBstatus & HDMITX_HDCP_BSTATUS_MAX_DEVS_EXCEEDED) > 0))
        {
            /* No T0 interrupt from device after a bad BSTATUS so do it here */
            err = tmbslHdmiTxHdcpHandleT0(txUnit);/* ->T0 state change */
        }
        else
        {
            err = TM_OK;
            setState(pDis, EV_HDCP_BSTATUS_GOOD);
        }
    }

    /* Copy to optional output parameter */
    if (pBstatus)
    {
        *pBstatus = pDis->HdcpBstatus;
    }
    return err;
}

/*============================================================================*/
/* tmbslHdmiTxHdcpHandleENCRYPT                                               */
/*============================================================================*/
tmErrorCode_t
tmbslHdmiTxHdcpHandleENCRYPT
(
    tmUnitSelect_t	txUnit
)
{
    tmHdmiTxobject_t *pDis;     /* Pointer to Device Instance Structure */
    tmErrorCode_t     err;      /* Error code */
    
    /* Check unit parameter and point to its object */
    err = checkUnitSetDis(txUnit, &pDis);
    RETIF(err != TM_OK, err)

    if (gIgnoreEncrypt[txUnit])
    {
        /* Ignore 1st encrypt after HdcpInit */
        gIgnoreEncrypt[txUnit] = False;
        return TM_OK;
    }
    else
    {
        /* Reset HDCP */
        return tmbslHdmiTxHdcpHandleT0(txUnit); /* Does T0 state change */
    }
}

/*============================================================================*/
/* tmbslHdmiTxHdcpHandlePJ                                                    */
/*============================================================================*/
tmErrorCode_t
tmbslHdmiTxHdcpHandlePJ
(
    tmUnitSelect_t	txUnit
)
{
    tmHdmiTxobject_t *pDis;     /* Pointer to Device Instance Structure */
    tmErrorCode_t     err;      /* Error code */
    
    /* Check unit parameter and point to its object */
    err = checkUnitSetDis(txUnit, &pDis);
    RETIF(err != TM_OK, err)

    if ((pDis->HdcpOptions & HDMITX_HDCP_OPTION_FORCE_PJ_IGNORED) == 0)
    {
        return tmbslHdmiTxHdcpHandleT0(txUnit); /* Does T0 state change */
    }
    return TM_OK;
}

/*============================================================================*/
/* tmbslHdmiTxHdcpHandleSHA_1                                                 */
/*============================================================================*/
tmErrorCode_t
tmbslHdmiTxHdcpHandleSHA_1
(
    tmUnitSelect_t	txUnit,
    UInt8           maxKsvDevices,
    UInt8           *pKsvList,          /* May be null if maxKsvDevices is 0 */
    UInt8           *pnKsvDevices,       /* May be null if maxKsvDevices is 0 */
    UInt8           *pDepth             /* Connection tree depth returned with KSV list */
)
{
    tmHdmiTxobject_t *pDis;             /* Pointer to Device Instance Struct. */
    tmErrorCode_t     err;              /* Error code */
    UInt8             v_equ_vbar;       /* Register value for V=V' */
    UInt8             bstatusDevices;   /* No. of devices defined in BSTATUS */
    UInt16            ksvIndex;         /* Byte index into KSV list */
    UInt16            i;                /* Loop counter */

    /* Check unit parameter and point to its object */
    err = checkUnitSetDis(txUnit, &pDis);
    RETIF(err != TM_OK, err)

    /* Check other parameters */
    RETIF_BADPARAM(maxKsvDevices > HDMITX_KSV_LIST_MAX_DEVICES)
    if (maxKsvDevices > 0)
    {
        RETIF((pKsvList == Null) || (pnKsvDevices == Null),
              TMBSL_ERR_HDMI_INCONSISTENT_PARAMS)
    }

   RETIF(pDepth == Null, TMBSL_ERR_HDMI_INCONSISTENT_PARAMS)


    /* Force V=V' or read it from transmitter */
    if ((pDis->HdcpOptions & HDMITX_HDCP_OPTION_FORCE_V_EQU_VBAR) > 0)
    {
        v_equ_vbar = 1;
    }
    else
    {
        err = getHwRegister(pDis, E_REG_P12_HDCP_KSV_check_R, &v_equ_vbar);
        if (err != TM_OK)
        {
            /* Force bad value if supposed to read it but can't */
            v_equ_vbar = 0;
        }
    }

    err = TM_OK;
    if (!v_equ_vbar)
    {
        /* V=V' could not be read or is not set: reset HDCP */
        err = tmbslHdmiTxHdcpHandleT0(txUnit); /* Does T0 state change */
        /*V != V' error HDCP*/
        err = TMBSL_ERR_HDMI_INCONSISTENT_PARAMS;
        if (pnKsvDevices)
        {
            *pnKsvDevices = 0;
        }
    }
    else if (maxKsvDevices == 0)
    {
        /* No KSV check requested by caller */
        if (pnKsvDevices)
        {
            *pnKsvDevices = 0;
        }

        /* Treat KSV as secure */
        setState(pDis, EV_HDCP_KSV_SECURE);
    }
    /* PR385 : pnKsvDevices should be read to continue HDCP authentification
	 * even if the BKSV list is not tested (revocation list)  	
	else if (pDis->HdcpTxMode != HDMITX_HDCP_TXMODE_TOP_LEVEL)
    {
        // No KSV required when not configured as a top-level transmitter 
        *pnKsvDevices = 0; // Can set because not null when maxKsvDevices > 0 

        // Treat KSV as secure 
        setState(pDis, EV_HDCP_KSV_SECURE);
    }*/
    else
    {
        /* The KsvList length is limited by the smaller of the list array
         * length and the number of devices returned in BSTATUS */
        bstatusDevices = 
            (UInt8)(pDis->HdcpBstatus & HDMITX_HDCP_BSTATUS_DEVICE_COUNT);
        if (maxKsvDevices < bstatusDevices)
        {
            *pnKsvDevices = maxKsvDevices;
        }
        else
        {
            *pnKsvDevices = bstatusDevices;
        }

        /* Read the KSV list if there are device KSVs to read */
        if (*pnKsvDevices > 0)
        {
            /* Read KSV list, 5 bytes per device, up to the array length or the
             * number of devices in BSTATUS whichever is smaller */
            for (i = 0; i < *pnKsvDevices; i++)
            {
                /* Set KSV index */
                ksvIndex = (UInt16)(i * HDMITX_KSV_BYTES_PER_DEVICE);
                err = setHwRegister(pDis, E_REG_P12_HDCP_KSVPTR_LSB_W,
                                   (UInt8)ksvIndex);
                RETIF_REG_FAIL(err)
                err = setHwRegister(pDis, E_REG_P12_HDCP_KSVPTR_MSB_W,
                                   (UInt8)(ksvIndex >> 8));
                RETIF_REG_FAIL(err)

                /* Read one device's ksv bytes */
                err = getHwRegisters(pDis, E_REG_P12_HDCP_KSV0_R,
                                    pKsvList + ksvIndex,
                                   (UInt8)HDMITX_KSV_BYTES_PER_DEVICE);
            }
            /* CALLER MUST NOW CHECK KSV LIST AGAINST REVOCATION LIST
             * BASED ON *pnKsvDevices > 0; IF REVOCATION LIST CHECK FAILS, 
             * CALLER MUST CALL tmbslHdmiTxHdcpHandleT0, WHICH WILL DO A T0
             * STATE CHANGE */
        }
        else
        {
            /* No device KSVs to read so treat KSV as secure */
            setState(pDis, EV_HDCP_KSV_SECURE);
        }
    }

    /* read connection tree depth */
    *pDepth = (UInt8)((pDis->HdcpBstatus & HDMITX_HDCP_BSTATUS_CASCADE_DEPTH)>>8);

    return err;
}

/*============================================================================*/
/* tmbslHdmiTxHdcpHandleSHA_1Result                                           */
/*============================================================================*/
tmErrorCode_t
tmbslHdmiTxHdcpHandleSHA_1Result
(
    tmUnitSelect_t	txUnit,
    Bool            bSecure
)
{
    tmHdmiTxobject_t *pDis;     /* Pointer to Device Instance Structure */
    tmErrorCode_t     err;      /* Error code */
    UInt8           pixClk;     /* Pixel clock index */
    UInt8           regVal;     /* Register value */

    /* Check unit parameter and point to its object */
    err = checkUnitSetDis(txUnit, &pDis);
    RETIF(err != TM_OK, err)

    /* Check other parameters */
    RETIF_BADPARAM(bSecure > 1)

    if (bSecure)
    {
        /* Initialize pixClk */
        pixClk = E_PIXCLK_INVALID;
        /* KSV list is secure */
        /* PNF57  add the timer 1s here */
        if ((pDis->voutFmt < HDMITX_VFMT_TV_NUM)
            && (pDis->voutFreq < HDMITX_VFREQ_NUM))
        {
            pixClk = kVfmtToPixClk_TV[pDis->voutFmt][pDis->voutFreq];
        }

#ifdef FORMAT_PC
        if ((pDis->voutFmt >= HDMITX_VFMT_PC_MIN)
            && (pDis->voutFmt <= HDMITX_VFMT_PC_MAX))
        {
            pixClk = kVfmtToPixClk_PC[pDis->voutFmt - HDMITX_VFMT_PC_MIN + 1];
        }
#endif /* FORMAT_PC */

        if (pixClk != E_PIXCLK_INVALID)
        {
            /* Set watchdog timer reference to nearest pixel clock MHz */
            regVal = kPixClkToMHz[pixClk];
            err = setHwRegister(pDis, E_REG_P00_NDIV_PF_W, regVal);
            RETIF_REG_FAIL(err)

            err = setHwRegisterFieldTable(pDis, &kSet1sWatchdog[0]);
    	    RETIF_REG_FAIL(err)
        }

        
        err = setHwRegisterFieldTable(pDis, &kKsvLSecure[0]);

        RETIF_REG_FAIL(err)


        setState(pDis, EV_HDCP_KSV_SECURE);
    }
    else
    {

        /* BKSV is not secure */

        err = setHwRegisterFieldTable(pDis, &kKsvLNotSecure[0]);


      
        RETIF_REG_FAIL(err)


    }
    return err;
}

/*============================================================================*/
/* tmbslHdmiTxHdcpHandleT0                                                    */
/*============================================================================*/
tmErrorCode_t
tmbslHdmiTxHdcpHandleT0
(
    tmUnitSelect_t	txUnit
)
{
    tmHdmiTxobject_t *pDis;     /* Pointer to Device Instance Structure */
    UInt8           pixClk;     /* Pixel clock index */
    UInt8           regVal;     /* Register value */
    tmErrorCode_t     err;      /* Error code */

    /* Check unit parameter and point to its object */
    err = checkUnitSetDis(txUnit, &pDis);
    RETIF(err != TM_OK, err)

    /* Get device HDCP failure state that caused this interrupt.
     * This is an N4 register, but will return 0 for earlier versions. */
    err = getHwRegister(pDis, E_REG_P12_HDCP_FAILSTATE_R,
                        &pDis->HdcpT0FailState);
    RETIF_REG_FAIL(err)

    /* Use voutFmt and voutFreq to index into a lookup table to get
     * the current HDMI pixel clock value. */
     
    /* Initialize pixClk */
    pixClk = E_PIXCLK_INVALID;

    if ((pDis->voutFmt < HDMITX_VFMT_TV_NUM)
        && (pDis->voutFreq < HDMITX_VFREQ_NUM))
    {
        pixClk = kVfmtToPixClk_TV[pDis->voutFmt][pDis->voutFreq];
    }

#ifdef FORMAT_PC
    if ((pDis->voutFmt >= HDMITX_VFMT_PC_MIN)
        && (pDis->voutFmt <= HDMITX_VFMT_PC_MAX))
    {
        pixClk = kVfmtToPixClk_PC[pDis->voutFmt - HDMITX_VFMT_PC_MIN + 1];
    }
#endif /* FORMAT_PC */
    
    if (pixClk != E_PIXCLK_INVALID)
    {
        /* Set watchdog timer reference to nearest pixel clock MHz */
        regVal = kPixClkToMHz[pixClk];
        err = setHwRegister(pDis, E_REG_P00_NDIV_PF_W, regVal);
        RETIF_REG_FAIL(err)

       /* Set 120ms watchdog based on HDMI clock */
       err = setHwRegisterFieldTable(pDis, &kSet120msWatchdog[0]);
       RETIF_REG_FAIL(err)
    }

    
    /* Recover from HDCP T0 interrupt */
    err = setHwRegisterFieldTable(pDis, &kT0Recovery[0]);
    RETIF_REG_FAIL(err)

    /* Enable all HDCP interrupts */
    regVal =
        E_MASKREG_P00_INT_FLAGS_0_sha_1   |
        E_MASKREG_P00_INT_FLAGS_0_bstatus |
        E_MASKREG_P00_INT_FLAGS_0_bcaps   |
        E_MASKREG_P00_INT_FLAGS_0_t0      |
#ifdef TMFL_TDA9981_SUPPORT
		E_MASKREG_P00_INT_FLAGS_0_r0      |
#endif /* TMFL_TDA9981_SUPPORT */
        E_MASKREG_P00_INT_FLAGS_0_encrypt;
    if ((pDis->HdcpOptions & HDMITX_HDCP_OPTION_FORCE_PJ_IGNORED) == 0)
    {
        /* Include Pj */
        regVal |= E_MASKREG_P00_INT_FLAGS_0_pj;
    }
    err = setHwRegisterField(pDis, E_REG_P00_INT_FLAGS_0_RW, regVal , regVal);
    RETIF_REG_FAIL(err)

    /* Relaunch HDCP algorithm after interrupts enabled, to avoid missing any */
    err = setHwRegisterField(pDis, E_REG_P12_HDCP_TX0_RW,
                             E_MASKREG_P12_HDCP_TX0_sr_hdcp, 0);
    RETIF_REG_FAIL(err)

    /* Disable any scheduled function, leave HDCP check timer running */
    pDis->HdcpFuncRemainingMs = 0;

    setState(pDis, EV_HDCP_T0);
    return TM_OK;
}

/*============================================================================*/
/* tmbslHdmiTxHdcpInit                                                        */
/* RETIF_REG_FAIL NOT USED HERE AS ALL ERRORS SHOULD BE TRAPPED IN ALL BUILDS */
/*============================================================================*/
tmErrorCode_t
tmbslHdmiTxHdcpInit
(
    tmUnitSelect_t	    txUnit,
    tmbslHdmiTxVidFmt_t voutFmt,
    tmbslHdmiTxVfreq_t  voutFreq
)
{
    tmHdmiTxobject_t *pDis;     /* Pointer to Device Instance Structure */
    tmErrorCode_t     err;      /* Error code */
    UInt8             pixClk;   /* Pixel clock index */
    UInt8             regVal;   /* Register value */

    /* Check unit parameter and point to its object */
    err = checkUnitSetDis(txUnit, &pDis);
    RETIF(err != TM_OK, err)

    /* Check if this device supports HDCP */
    RETIF(pDis->uDeviceFeatures & E_MASKREG_P00_VERSION_not_h,
          TMBSL_ERR_HDMI_NOT_SUPPORTED)

    /* Check other arguments */
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
    
#ifdef FORMAT_PC
    if ( voutFmt < HDMITX_VFMT_PC_MIN )
    {
#endif /* FORMAT_PC */
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
#ifdef FORMAT_PC
    }
    else
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

    /* Use voutFmt and voutFreq to index into a lookup table to get
     * the current pixel clock value. */
     
    /* Initialize pixClk */
    pixClk = E_PIXCLK_INVALID;
     
    if ((voutFmt < HDMITX_VFMT_TV_NUM)
        && (voutFreq < HDMITX_VFREQ_NUM))
    {
        pixClk = kVfmtToPixClk_TV[voutFmt][voutFreq];
    }

#ifdef FORMAT_PC
    if ((voutFmt >= HDMITX_VFMT_PC_MIN)
        && (voutFmt <= HDMITX_VFMT_PC_MAX))
    {
        pixClk = kVfmtToPixClk_PC[voutFmt - HDMITX_VFMT_PC_MIN + 1];
    }
#endif /* FORMAT_PC */
    

    RETIF(pixClk == E_PIXCLK_INVALID, TMBSL_ERR_HDMI_INCONSISTENT_PARAMS)

    /* Save vertical output frequency for use later by HandleT0 */
    pDis->voutFreq = voutFreq;

    /* Initialise HDCP (part 1) */
    err = setHwRegisterFieldTable(pDis, &kHdcpInit1[0]);
    RETIF(err != TM_OK, err)

    /* Set watchdog timer reference to nearest pixel clock MHz */
    regVal = kPixClkToMHz[pixClk];
    err = setHwRegister(pDis, E_REG_P00_NDIV_PF_W, regVal);
    RETIF(err != TM_OK, err)

    /* Set 120ms watchdog based on HDMI clock */
    err = setHwRegisterFieldTable(pDis, &kSet120msWatchdog[0]);
    RETIF(err != TM_OK, err)

#ifdef DDC_MASTER_WORKAROUND
    /* Check for N4 (N5 uses same version code, so check feature flags also) */
    if ((pDis->uDeviceVersion == E_DEV_VERSION_N4)
    && ((pDis->uDeviceFeatures & E_MASKREG_P00_FEATURE_N5) == 0))
    {
        /* Initialise HDCP (part 2a)    N4 DDC MASTER WORKAROUND */
        err = setHwRegisterFieldTable(pDis, &kHdcpInit2a[0]);
        RETIF(err != TM_OK, err)
    }
    else
#endif
    {
        /* Initialise HDCP (part 2) */
        err = setHwRegisterFieldTable(pDis, &kHdcpInit2[0]);
        RETIF(err != TM_OK, err)
    }

    /* Set DDC speed */
    if ((pDis->HdcpOptions & HDMITX_HDCP_OPTION_FORCE_VSLOW_DDC) > 0)
    {
        regVal = 100; /* 50kHz */
    }
    else
    {
        regVal = 50; /* 100kHz */
    }
    err = setHwRegister(pDis, E_REG_P12_HDCP_TX3_RW, regVal);
    RETIF(err != TM_OK, err)

    /* Initialise HDCP (part 3) */
    err = setHwRegisterFieldTable(pDis, &kHdcpInit3[0]);
    RETIF(err != TM_OK, err)

    /* Obey sink type - done in tmbslHdmiTxVideoOutSetConfig */

    /* Set flag so 1st Encrypt interrupt after HdcpInit is ignored */
    gIgnoreEncrypt[txUnit] = True;

    /* Enable all HDCP interrupts */
    regVal =
        E_MASKREG_P00_INT_FLAGS_0_sha_1   |
        E_MASKREG_P00_INT_FLAGS_0_bstatus |
        E_MASKREG_P00_INT_FLAGS_0_bcaps   |
        E_MASKREG_P00_INT_FLAGS_0_t0      |
#ifdef TMFL_TDA9981_SUPPORT
		E_MASKREG_P00_INT_FLAGS_0_r0      |
#endif
        E_MASKREG_P00_INT_FLAGS_0_encrypt;
    if ((pDis->HdcpOptions & HDMITX_HDCP_OPTION_FORCE_PJ_IGNORED) == 0)
    {
        /* Include Pj */
        regVal |= E_MASKREG_P00_INT_FLAGS_0_pj;
    }
    err = setHwRegisterField(pDis, E_REG_P00_INT_FLAGS_0_RW, regVal, regVal);
    return err;
}

/*============================================================================*/
/* tmbslHdmiTxHdcpRun                                                         */
/*============================================================================*/
tmErrorCode_t
tmbslHdmiTxHdcpRun
(
    tmUnitSelect_t	txUnit
)
{
    tmHdmiTxobject_t *pDis;     /* Pointer to Device Instance Structure */
    tmErrorCode_t     err;      /* Error code */

    /* Check unit parameter and point to its object */
    err = checkUnitSetDis(txUnit, &pDis);
    RETIF(err != TM_OK, err)

    /* Check if this device supports HDCP */
    RETIF(pDis->uDeviceFeatures & E_MASKREG_P00_VERSION_not_h,
          TMBSL_ERR_HDMI_NOT_SUPPORTED)

#ifdef TMFL_TDA9981_SUPPORT
    /* DDC interface is disable for TDA9981 after reset and in 
       tmbslHdmiTxHdcpStop function, enable it */
    err = setHwRegister(pDis, E_REG_P00_DDC_DISABLE_RW, 0x00);
    RETIF(err != TM_OK, err)
#endif /* TMFL_TDA9981_SUPPORT */

    /* Reset device HDCP data in DIS to ensure that new values are used */
    pDis->HdcpBcaps = 0;
    pDis->HdcpBstatus = 0;
    pDis->HdcpT0FailState = 0;
    pDis->HdcpFsmState = 0;
    pDis->HdcpRi = 0;

    /* Launch HDCP algorithm */
    err = setHwRegisterField(pDis, E_REG_P12_HDCP_TX0_RW,
                             E_MASKREG_P12_HDCP_TX0_sr_hdcp, 0);
    if (err == TM_OK)
    {
        /* Start the HDCP check timer if checking is configured */
        if (pDis->HdcpChecksToDo > 1)
        {
            pDis->HdcpCheckRemainingMs = pDis->HdcpCheckIntervalMs;
            pDis->HdcpCheckNum = 1;
        }

        setState(pDis, EV_HDCP_RUN);
    }
    return err;
}

/*============================================================================*/
/* tmbslHdmiTxHdcpStop                                                        */
/*============================================================================*/
tmErrorCode_t
tmbslHdmiTxHdcpStop
(
    tmUnitSelect_t	txUnit
)
{
    tmHdmiTxobject_t *pDis;     /* Pointer to Device Instance Structure */
    tmErrorCode_t     err;      /* Error code */

    /* Check unit parameter and point to its object */
    err = checkUnitSetDis(txUnit, &pDis);
    RETIF(err != TM_OK, err)

    /* Check if this device supports HDCP */
    RETIF(pDis->uDeviceFeatures & E_MASKREG_P00_VERSION_not_h,
          TMBSL_ERR_HDMI_NOT_SUPPORTED)

    /* Disable all HDCP interrupts by only enabling Hot Plug Detect */
    (void)setHwRegister(pDis, E_REG_P00_INT_FLAGS_0_RW, 
                        E_MASKREG_P00_INT_FLAGS_0_hpd);

    /* Stop HDCP algorithm */
    err = setHwRegisterField(pDis, E_REG_P12_HDCP_TX0_RW,
                             E_MASKREG_P12_HDCP_TX0_sr_hdcp, 1);

    /* Disable any scheduled function and HDCP check timer */
    pDis->HdcpFuncRemainingMs = 0;
    pDis->HdcpCheckNum = 0;

    setState(pDis, EV_HDCP_STOP);

#ifdef TMFL_TDA9981_SUPPORT
    /* DDC interface is enable in tmbslHdmiTxHdcpRun function, disable it */
    err += setHwRegister(pDis, E_REG_P00_DDC_DISABLE_RW, 0x01);
#endif /* TMFL_TDA9981_SUPPORT */

    return err;
}
#endif /* NO_HDCP */

/*============================================================================*/
/* tmbslHdmiTxHotPlugGetStatus                                                */
/*============================================================================*/
tmErrorCode_t
tmbslHdmiTxHotPlugGetStatus
(
    tmUnitSelect_t        txUnit,
    tmbslHdmiTxHotPlug_t *pHotPlugStatus
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
/* tmbslHdmiTxRxSenseGetStatus                                                */
/*============================================================================*/
tmErrorCode_t
tmbslHdmiTxRxSenseGetStatus
(
    tmUnitSelect_t        txUnit,
    tmbslHdmiTxRxSense_t *pRxSenseStatus
)
{
    tmHdmiTxobject_t *pDis;     /* Pointer to Device Instance Structure */
    tmErrorCode_t     err;      /* Error code */

    /* Check unit parameter and point to its object */
    err = checkUnitSetDis(txUnit, &pDis);
    RETIF(err != TM_OK, err)

    /* Check remaining parameters */
    RETIF_BADPARAM(pRxSenseStatus == (tmbslHdmiTxRxSense_t *)0)

#if defined (TMFL_TDA9981_SUPPORT) && defined(TMFL_RX_SENSE_ON)
    /* Read the Rx sense status flag register last read at interrupt */
    *pRxSenseStatus = pDis->rxSenseStatus;
	return TM_OK;
#else /* TMFL_TDA9981_SUPPORT && TMFL_RX_SENSE_ON */
    (void)*pRxSenseStatus; /* else not referenced */
	return TMBSL_ERR_HDMI_NOT_SUPPORTED;
#endif /* TMFL_TDA9981_SUPPORT && TMFL_RX_SENSE_ON */
}

/*============================================================================*/
/* tmbslHdmiTxHwGetRegisters                                                  */
/*============================================================================*/
tmErrorCode_t
tmbslHdmiTxHwGetRegisters
(
    tmUnitSelect_t	txUnit,
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
/* tmbslHdmiTxHwGetVersion                                                    */
/*============================================================================*/
#ifndef DEMO_BUILD
tmErrorCode_t
tmbslHdmiTxHwGetVersion
(
    tmUnitSelect_t	txUnit,
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


#ifdef TMFL_TDA9981_SUPPORT

      *pHwVersion = (UInt8) BSLHDMITX_TDA9981;

#else

    *pHwVersion = (UInt8) BSLHDMITX_TDA9983;

#endif /* TMFL_TDA9981_SUPPORT */

    return TM_OK;
}
#endif /* DEMO_BUILD */

/*============================================================================*/
/* tmbslHdmiTxHwGetCapabilities                                                    */
/*============================================================================*/
tmErrorCode_t
tmbslHdmiTxHwGetCapabilities
(
    tmUnitSelect_t          txUnit,
    tmbslHdmiTxHwFeature_t  *pDeviceCapabilities
)

{
	tmHdmiTxobject_t *pDis;       /* Pointer to Device Instance Structure */
    tmErrorCode_t   err;            /* Error code */

    /* Check unit parameter and point to its object */
    err = checkUnitSetDis(txUnit, &pDis);
	RETIF(err != TM_OK, err)

    /* Check remaining parameters */
    RETIF_BADPARAM(pDeviceCapabilities == (tmbslHdmiTxHwFeature_t *)0)

	*pDeviceCapabilities = tmbslHdmiTxHwNone;

	if((pDis->uDeviceFeatures & E_MASKREG_P00_VERSION_not_h) == 0)
	{
            *pDeviceCapabilities = (tmbslHdmiTxHwFeature_t) (*pDeviceCapabilities + tmbslHdmiTxHwHDCP);
	}

	if((pDis->uDeviceFeatures & E_MASKREG_P00_VERSION_not_s) == 0)
	{
            *pDeviceCapabilities = (tmbslHdmiTxHwFeature_t) (*pDeviceCapabilities + tmbslHdmiTxHwScaler);
	}

    return TM_OK;
}

/*============================================================================*/
/* tmbslHdmiTxHwHandleInterrupt                                               */
/* RETIF_REG_FAIL NOT USED HERE AS ALL ERRORS SHOULD BE TRAPPED IN ALL BUILDS */
/*============================================================================*/
tmErrorCode_t
tmbslHdmiTxHwHandleInterrupt
(
    tmUnitSelect_t		txUnit
)
{
    tmHdmiTxobject_t *pDis;             /* Pointer to Device Instance Structure */
    tmErrorCode_t     err;              /* Error code */
    UInt8             regVal;           /* Register value */
#ifdef TMFL_TDA9981_SUPPORT
    UInt16            fInterruptStatus; /* Interrupt flags */
    UInt16            fInterruptMask;   /* Mask to test each interrupt bit */
#ifdef TMFL_RX_SENSE_ON
	tmbslHdmiTxRxSense_t  newRxs_fil;       /* Latest copy of rx_sense */
#endif /* TMFL_RX_SENSE_ON */
#else /* TMFL_TDA9981_SUPPORT */
    UInt8             fInterruptStatus; /* Interrupt flags */
    UInt8             fInterruptMask;   /* Mask to test each interrupt bit */
#endif /* TMFL_TDA9981_SUPPORT */
    Int               i;                /* Loop counter */
    tmbslHdmiTxHotPlug_t newHpdIn;      /* Latest copy of hpd input */

    /* Check unit parameter and point to its object */
    err = checkUnitSetDis(txUnit, &pDis);
    RETIF(err != TM_OK, err)

	fInterruptStatus = 0;

    /* Read the main interrupt flags register to determine the source(s) 
     * of the interrupt. (The device resets these register flags after they 
     * have been read.)
     */
#ifdef TMFL_TDA9981_SUPPORT
    err = getHwRegister(pDis, E_REG_P00_INT_FLAGS_0_RW, &regVal);
    RETIF(err != TM_OK, err)
    fInterruptStatus = regVal;

#else /* TMFL_TDA9981_SUPPORT */
    err = getHwRegister(pDis, E_REG_P00_INT_FLAGS_0_RW, &fInterruptStatus);
    RETIF(err != TM_OK, err)
#endif /* TMFL_TDA9981_SUPPORT */

    /* Read Hot Plug input status to unit object 
       DJW do this after reading INT flags so we get the actual
       level that caused the interrupt */
    err = getHwRegister(pDis, E_REG_P00_INT_FLAGS_1_RW, &regVal);
    RETIF(err != TM_OK, err)
    newHpdIn = (regVal & E_MASKREG_P00_INT_FLAGS_1_hpd_in) ?
                HDMITX_HOTPLUG_ACTIVE : HDMITX_HOTPLUG_INACTIVE;

#ifdef TMFL_TDA9981_SUPPORT
	/* Read the software interrupt flag */
	if ((regVal & E_MASKREG_P00_INT_FLAGS_1_sw_int) != 0)
	{
		fInterruptStatus = fInterruptStatus | (1 << HDMITX_CALLBACK_INT_SW_INT); 
	}

#ifdef TMFL_RX_SENSE_ON
    /* Read INT_FLAGS_2 interrupt flag register. 
	 *(The device resets these register flags after they 
     * have been read.) */
    err = getHwRegister(pDis, E_REG_P00_INT_FLAGS_2_RW, &regVal);
    RETIF(err != TM_OK, err)

	/* Has the rx_sense interrupt occurs */
    if ((regVal & E_MASKREG_P00_INT_FLAGS_2_rx_sense) != 0)
    {
		fInterruptStatus = fInterruptStatus | (1 << HDMITX_CALLBACK_INT_RX_SENSE); 
	}
#endif /* TMFL_RX_SENSE_ON */
#endif /* TMFL_TDA9981_SUPPORT */

    /* Has the hpd input changed? (Ignore the HPD interrupt status flag in 
     * fInterruptStatus as this may have latched a connect/disconnect glitch) */
    if (newHpdIn != pDis->hotPlugStatus)
    {
        /* Yes: save new HPD level */
        pDis->hotPlugStatus = newHpdIn;

        /* Reset EDID status */
        pDis->EdidStatus = HDMITX_EDID_NOT_READ;

#ifdef TMFL_TDA9981_SUPPORT
		/* Set HPD flag to 1 although it certainly already done just a security */
		fInterruptStatus |= E_MASKREG_P00_INT_FLAGS_0_hpd;
		/* Reset all simultaneous HDCP interrupts on hot plug,
		 * preserving only the high-priority hpd interrupt rx_sense and sw interrupt for debug*/
		fInterruptStatus &= E_MASKREG_P00_INT_FLAGS_0_hpd |
#ifdef TMFL_RX_SENSE_ON
							(1 << HDMITX_CALLBACK_INT_RX_SENSE) |
#endif /* TMFL_RX_SENSE_ON */
							(1 << HDMITX_CALLBACK_INT_SW_INT);
#else
        /* Reset all simultaneous HDCP interrupts on hot plug/unplug,
         * preserving only the high-priority hpd interrupt */
        fInterruptStatus = E_MASKREG_P00_INT_FLAGS_0_hpd;
#endif /* TMFL_TDA9981_SUPPORT */

        if (pDis->hotPlugStatus == HDMITX_HOTPLUG_ACTIVE)
        {
            setState(pDis, EV_PLUGGEDIN);
        }
        else
        {
            setState(pDis, EV_UNPLUGGED);
        }
    }
    else
    {
        /* Clear HPD status if level has not changed */
        fInterruptStatus &= ~E_MASKREG_P00_INT_FLAGS_0_hpd;
    }

#ifdef TMFL_TDA9981_SUPPORT
#ifdef TMFL_RX_SENSE_ON

	/* Read INT_FLAGS_3 interrupt flag register. */
	err = getHwRegister(pDis, E_REG_P00_INT_FLAGS_3_R, &regVal);
	RETIF(err != TM_OK, err)

	/*	Read RXS_FIL status to know the actual
		level that caused the interrupt */
	newRxs_fil = (regVal & E_MASKREG_P00_INT_FLAGS_3_rxs_fil) ?
				 HDMITX_RX_SENSE_ACTIVE : HDMITX_RX_SENSE_INACTIVE;

	/* Has the Rxs Fil changed? (Ignore the RxSense interrupt status flag in 
	 * fInterruptStatus as this may have latched a on/off glitch) */
	if (newRxs_fil != pDis->rxSenseStatus)
	{
		/* Yes: save new rxSense level */
		pDis->rxSenseStatus = newRxs_fil;

		/* Set RX_Sense flag to 1 although it certainly already done just a security */
		fInterruptStatus |= ( 1 << HDMITX_CALLBACK_INT_RX_SENSE);

		fInterruptStatus &= E_MASKREG_P00_INT_FLAGS_0_hpd |
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
	else
	{
		/* Clear RX_sense IT if level has not changed */
		fInterruptStatus &= ~(1 << HDMITX_CALLBACK_INT_RX_SENSE);
	}
#endif /* TMFL_RX_SENSE_ON */
#endif /* TMFL_TDA9981_SUPPORT */

    /* Ignore other simultaneous HDCP interrupts if T0 interrupt,
     * preserving any hpd interrupt */
    if (fInterruptStatus & E_MASKREG_P00_INT_FLAGS_0_t0)
    {
        fInterruptStatus &= 
            (
            E_MASKREG_P00_INT_FLAGS_0_hpd |
            E_MASKREG_P00_INT_FLAGS_0_t0
#ifdef TMFL_TDA9981_SUPPORT
#ifdef TMFL_RX_SENSE_ON
			|(1 << HDMITX_CALLBACK_INT_RX_SENSE)
#endif /* TMFL_RX_SENSE_ON */
			|(1 << HDMITX_CALLBACK_INT_SW_INT)
#endif /* TMFL_TDA9981_SUPPORT */
            );
    }

#ifdef TMFL_TDA9981_SUPPORT
    if (fInterruptStatus & E_MASKREG_P00_INT_FLAGS_0_r0)
    {
        err = handleBKSVResultSecure(txUnit);
        RETIF(err != TM_OK, err)
    }
#endif /* TMFL_RX_SENSE_ON */

    /* For each interrupt flag that is set, check the corresponding registered 
     * callback function pointer in the Device Instance Structure 
     * funcIntCallbacks array.
     */
    fInterruptMask = 1;
    for (i = 0; i < HDMITX_CALLBACK_INT_NUM; i++)
    {
        if (fInterruptStatus & fInterruptMask)
        {
            /* IF a registered callback pointer is non-null THEN call it. */
            if (pDis->funcIntCallbacks[i] != (ptmbslHdmiTxCallback_t)0)
            {
                pDis->funcIntCallbacks[i](txUnit);
            }
        }
        fInterruptMask <<= 1;
    }

    return TM_OK;
}

/*============================================================================*/
/* tmbslHdmiTxFlagSwInt                                                       */
/* Use only for debug to flag the software debug interrupt                    */
/*============================================================================*/
tmErrorCode_t
tmbslHdmiTxFlagSwInt
(
    tmUnitSelect_t	            txUnit
)
{
    tmHdmiTxobject_t *pDis;     /* Pointer to Device Instance Structure */
    tmErrorCode_t     err;      /* Error code */

    /* Check unit parameter and point to its object */
    err = checkUnitSetDis(txUnit, &pDis);
    RETIF(err != TM_OK, err)

#ifdef TMFL_TDA9981_SUPPORT
    err = setHwRegister(pDis, E_REG_P00_SW_INT_W, 
                        E_MASKREG_P00_SW_INT_sw_int);

	return err;
#else /* TMFL_TDA9981_SUPPORT */
    return TMBSL_ERR_HDMI_NOT_SUPPORTED;
#endif /* TMFL_TDA9981_SUPPORT */
}

/*============================================================================*/
/* tmbslHdmiTxHwSetRegisters                                                  */
/*============================================================================*/
#ifndef DEMO_BUILD
tmErrorCode_t
tmbslHdmiTxHwSetRegisters 
(
    tmUnitSelect_t	txUnit,
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
/* tmbslHdmiTxHwStartup                                                       */
/*============================================================================*/
void
tmbslHdmiTxHwStartup
(
    void
)
{
	/* Reset device instance data for when compiler doesn't do it */
	lmemset(&gHdmiTxInstance, 0, sizeof(gHdmiTxInstance));
}

/*============================================================================*/
/* tmbslHdmiTxInit                                                            */
/* RETIF_REG_FAIL NOT USED HERE AS ALL ERRORS SHOULD BE TRAPPED IN ALL BUILDS */
/*============================================================================*/
tmErrorCode_t
tmbslHdmiTxInit
(
    tmUnitSelect_t	           txUnit,
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
    UInt8             regVal;   /* Value read from register */

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
    RETIF_BADPARAM(sysFuncEdidRead == (ptmbslHdmiTxSysFuncEdid_t)0)
    RETIF_BADPARAM(sysFuncTimer == (ptmbslHdmiTxSysFuncTimer_t)0)
    RETIF_BADPARAM((bEdidAltAddr != True) && (bEdidAltAddr != False))
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

    /* Reset Hdcp interrupt flag */
    gIgnoreEncrypt[txUnit] = False;

    /* Ensure that DDC is not connected to I2C at start */
    err = setHwRegister(pDis, E_REG_P00_GHOST_ADDR_W, 0x01);
    RETIF(err != TM_OK, err)

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

#ifdef TMFL_TDA9981_SUPPORT
    /* If version reads zero, test not_s to '1' for TDA9981 */
    if (pDis->uDeviceVersion == E_DEV_VERSION_N4)
    {
		if ((pDis->uDeviceFeatures & E_MASKREG_P00_VERSION_not_s) !=
             E_MASKREG_P00_VERSION_not_s)
		{
            pDis->uDeviceVersion = E_DEV_VERSION_LIST_END;
		}
	}
#endif /* TMFL_TDA9981_SUPPORT */

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

    /* If version is N4 then detect N5 device (both have same version) */
    if (pDis->uDeviceVersion == E_DEV_VERSION_N4)
    {
        /* Read N5-specific registers to detect N5 */
        err = getHwRegister(pDis, E_REG_P00_DWIN_RE_DE_W, &regVal);
        RETIF(err != TM_OK, err)
        if (regVal == 0x11)
        {
            /* Default value for N5 */
            err = getHwRegister(pDis, E_REG_P00_DWIN_FE_DE_W, &regVal);
            RETIF(err != TM_OK, err)
            if (regVal == 0x7A)
            {
                /* Both N5 registers have correct default value for N5,
                 * so set the N5 'feature' flag */
                pDis->uDeviceFeatures |= E_MASKREG_P00_FEATURE_N5;
            }
        }
    }

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

    /* Set the EDID alternate address flag if needed*/
    pDis->bEdidAlternateAddr = bEdidAltAddr;

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
        break;
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
        break;
        }        

        break;
    }

    /* Reset the device */
    err = tmbslHdmiTxReset(txUnit);
    RETIF(err != TM_OK, err)

    /* The DIS hotplug status is HDMITX_HOTPLUG_INVALID, so call the main 
     * interrupt handler to read the current Hot Plug status and run any
     * registered HPD callback before interrupts are enabled below */
    err = tmbslHdmiTxHwHandleInterrupt(txUnit);
    RETIF(err != TM_OK, err)

#ifdef TMFL_TDA9981_SUPPORT
    /* enable sw _interrupt for debug */
    err = setHwRegister(pDis, E_REG_P00_INT_FLAGS_1_RW, 
                        E_MASKREG_P00_INT_FLAGS_1_sw_int);

#ifdef TMFL_RX_SENSE_ON
	/* enable sw _interrupt for debug */
    err = setHwRegister(pDis, E_REG_P00_INT_FLAGS_2_RW, 
                        E_MASKREG_P00_INT_FLAGS_2_rx_sense);
#endif /* TMFL_RX_SENSE_ON */
#endif /* TMFL_TDA9981_SUPPORT */

    /* Enable only the Hot Plug detect interrupt */
    err = setHwRegister(pDis, E_REG_P00_INT_FLAGS_0_RW, 
                        E_MASKREG_P00_INT_FLAGS_0_hpd);
    return err;
}

/*============================================================================*/
/* tmbslHdmiTxPowerGetState                                                   */
/*============================================================================*/
#ifndef DEMO_BUILD
tmErrorCode_t
tmbslHdmiTxPowerGetState
(
    tmUnitSelect_t		txUnit,
	ptmPowerState_t		pePowerState
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
#endif /* DEMO_BUILD */

/*============================================================================*/
/* tmbslHdmiTxPowerSetState                                                   */
/*============================================================================*/
tmErrorCode_t
tmbslHdmiTxPowerSetState
(
    tmUnitSelect_t		txUnit,
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
                setState(pDis, EV_RESUME_PLUGGEDIN);
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
/* tmbslHdmiTxReset                                                           */
/*============================================================================*/
tmErrorCode_t
tmbslHdmiTxReset
(
    tmUnitSelect_t		txUnit
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
/* tmbslHdmiTxScalerGet                                                       */
/*============================================================================*/
#ifndef DEMO_BUILD
tmErrorCode_t
tmbslHdmiTxScalerGet
(
    tmUnitSelect_t		      txUnit,
    tmbslHdmiTxScalerDiag_t  *pScalerDiag
)
{
    tmHdmiTxobject_t *pDis;     /* Pointer to Device Instance Structure */
    tmErrorCode_t   err;        /* Error code */
    UInt8           regVal;     /* Register value */

    /* Check unit parameter and point to its object */
    err = checkUnitSetDis(txUnit, &pDis);
    RETIF(err != TM_OK, err)

#ifdef TMFL_TDA9981_SUPPORT
    (void)pScalerDiag; /* else not referenced */
    (void)regVal; /* else is declared but not used */
    return TMBSL_ERR_HDMI_NOT_SUPPORTED;
#else
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
#endif /* TMFL_TDA9981_SUPPORT */
}
#endif /* DEMO_BUILD */

/*============================================================================*/
/* tmbslHdmiTxScalerGetMode                                                   */
/*============================================================================*/
tmErrorCode_t
tmbslHdmiTxScalerGetMode
(
    tmUnitSelect_t		      txUnit,
    tmbslHdmiTxScaMode_t      *pScalerMode
)
{
    tmHdmiTxobject_t *pDis;       /* Pointer to Device Instance Structure */
    tmErrorCode_t   err;          /* Error code */

    /* Check unit parameter and point to TX unit object */
    err = checkUnitSetDis(txUnit, &pDis);
    RETIF(err != TM_OK, err)

#ifdef TMFL_TDA9981_SUPPORT
    (void)pScalerMode; /* else is declared but not used */
    return TMBSL_ERR_HDMI_NOT_SUPPORTED;
#else
    /* Check if this device has a Scaler */
    RETIF(pDis->uDeviceFeatures & E_MASKREG_P00_VERSION_not_s,
          TMBSL_ERR_HDMI_NOT_SUPPORTED)

    /* Check parameters */
    RETIF_BADPARAM(pScalerMode == Null)

    /* Set output parameter */
    *pScalerMode = pDis->scaMode;
    return TM_OK;
#endif /* TMFL_TDA9981_SUPPORT */
}

/*============================================================================*/
/* tmbslHdmiTxScalerInDisable                                                 */
/*============================================================================*/
tmErrorCode_t
tmbslHdmiTxScalerInDisable
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

#ifdef TMFL_TDA9981_SUPPORT
    (void)bDisable; /* else is declared but not used */
    return TMBSL_ERR_HDMI_NOT_SUPPORTED;
#else
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
#endif /* TMFL_TDA9981_SUPPORT */
}

/*============================================================================*/
/* tmbslHdmiTxScalerSetCoeffs                                                 */
/*============================================================================*/
#ifndef DEMO_BUILD
tmErrorCode_t
tmbslHdmiTxScalerSetCoeffs
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

#ifdef TMFL_TDA9981_SUPPORT
    (void)lutSel; /* else not referenced */
    (void)pVsLut; /* else not referenced */
    return TMBSL_ERR_HDMI_NOT_SUPPORTED;
#else
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
#endif /* TMFL_TDA9981_SUPPORT */
}
#endif /* DEMO_BUILD */

/*============================================================================*/
/* tmbslHdmiTxScalerSetFieldOrder                                             */
/*============================================================================*/
tmErrorCode_t   
tmbslHdmiTxScalerSetFieldOrder 
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

#ifdef TMFL_TDA9981_SUPPORT
    (void)topExt; /* else is declared but not used */
    (void)deExt;  /* else is declared but not used */
    (void)topSel; /* else is declared but not used */
    (void)topTgl; /* else is declared but not used */
    return TMBSL_ERR_HDMI_NOT_SUPPORTED;
#else
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
#endif /* TMFL_TDA9981_SUPPORT */
}

/*============================================================================*/
/* tmbslHdmiTxScalerSetFine                                                   */
/*============================================================================*/
tmErrorCode_t   
tmbslHdmiTxScalerSetFine
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

#ifdef TMFL_TDA9981_SUPPORT
    (void)uRefPix;  /* else is declared but not used */
    (void)uRefLine; /* else is declared but not used */
	return TMBSL_ERR_HDMI_NOT_SUPPORTED;
#else
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
#endif /* TMFL_TDA9981_SUPPORT */
}

/*============================================================================*/
/* tmbslHdmiTxScalerSetSync                                                   */
/*============================================================================*/
tmErrorCode_t   
tmbslHdmiTxScalerSetSync
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

#ifdef TMFL_TDA9981_SUPPORT
    (void)method; /* else is declared but not used */
    (void)once;   /* else is declared but not used */
    return TMBSL_ERR_HDMI_NOT_SUPPORTED;
#else
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
#endif /* TMFL_TDA9981_SUPPORT */
}

/*============================================================================*/
/* tmbslHdmiTxSwGetVersion                                                    */
/*============================================================================*/
tmErrorCode_t   
tmbslHdmiTxSwGetVersion 
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
/* tmbslHdmiTxSysTimerWait                                                    */
/*============================================================================*/
tmErrorCode_t   
tmbslHdmiTxSysTimerWait 
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
/* tmbslHdmiTxTmdsSetOutputs                                                  */
/*============================================================================*/
tmErrorCode_t   
tmbslHdmiTxTmdsSetOutputs 
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
/* tmbslHdmiTxTmdsSetSerializer                                               */
/*============================================================================*/
tmErrorCode_t   
tmbslHdmiTxTmdsSetSerializer 
(
    tmUnitSelect_t  txUnit,
    UInt8           uPhase2,
    UInt8           uPhase3
)
{
    tmHdmiTxobject_t *pDis;       /* Pointer to Device Instance Structure */
    tmErrorCode_t     err;        /* Error code */

    /* Check unit parameter and point to TX unit object */
    err = checkUnitSetDis(txUnit, &pDis);
    RETIF(err != TM_OK, err)

    /* Check parameters */
    RETIF_BADPARAM(uPhase2 >= HDMITX_TMDSPHASE_INVALID)
    RETIF_BADPARAM(uPhase3 >= HDMITX_TMDSPHASE_INVALID)

    /* Set the serializer phase 2 & 3 counts */
    err = setHwRegisterField(pDis,
                             E_REG_P02_SERIALIZER_RW,
                             E_MASKREG_P02_SERIALIZER_srl_phase2,
                             uPhase2);
    RETIF_REG_FAIL(err)
    err = setHwRegisterField(pDis,
                             E_REG_P02_SERIALIZER_RW,
                             E_MASKREG_P02_SERIALIZER_srl_phase3,
                             uPhase3);
    return err;
}

/*============================================================================*/
/* tmbslHdmiTxTestSetPattern                                                  */
/*============================================================================*/
tmErrorCode_t   
tmbslHdmiTxTestSetPattern
(
    tmUnitSelect_t           txUnit,
    tmbslHdmiTxTestPattern_t pattern 
)
{
    tmHdmiTxobject_t *pDis;						/* Pointer to Device Instance Structure */
    tmErrorCode_t     err;						/* Error code */
    UInt8             serviceMode;				/* Register value */
    UInt8             bars8;					/* Register value */
	UInt8			  buf[MATRIX_PRESET_SIZE];	/* Temp buffer */
	UInt8			  i;						/* Loop index */
    UInt8			  hblOff;					/* FB: define hbl offset*/
    UInt8			  scInFmt = 0;				/* Scaler input format */
    UInt8			  scOutFmt = 0;				/* Scaler output format */			

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
		if (!pDis->prevPattern) /* if pattern is on, register is already saved */
		{
			/*  The kBypassColourProc registers are saved in tmbslTDA9984VideoSetInOut API */
			/* Bypass up/down sampler and matrix for RGB colourbars */
			setHwRegisterFieldTable(pDis, &kBypassColourProc[0]);

			/* disable scaler blanking if necessary */
			if (pDis->scaMode == HDMITX_SCAMODE_ON)
			{				
				/* remove blanking */
				err = setHwRegister(pDis, E_REG_P00_HBL_OFFSET_START_W, 0);
				RETIF_REG_FAIL(err)
			}
		}
		if ( pattern == HDMITX_PATTERN_BLUE ) /* blue screen pattern */
		{

			/* To create blue screen, we use the internal color bar 8 on which we apply a matrix to change it to blue */
			/* Set the first block byte separately, as it is shadowed and can't
			* be set by setHwRegisters */

			/* Set the first block byte separately, as it is shadowed and can't
			* be set by setHwRegisters */
			err = setHwRegister(pDis,					
								E_REG_P00_MAT_CONTRL_W,
								MatrixCoeffBlueScreen[pDis->voutMode][0]);
			RETIF_REG_FAIL(err)

			for (i = 0; i < MATRIX_PRESET_SIZE; i++)
			{
				buf[i] = MatrixCoeffBlueScreen[pDis->voutMode][i];
			}
			
			/* Set the rest of the block */
			err = setHwRegisters(pDis,
								E_REG_P00_MAT_OI1_MSB_W,
								&buf[1],
								MATRIX_PRESET_SIZE - 1);
			RETIF_REG_FAIL(err)

            if (pDis->voutMode == HDMITX_VOUTMODE_YUV422)
            {
                /* pattern is 444 -> convert to 422 */
                err = setHwRegisterField(pDis, 
                                        E_REG_P00_HVF_CNTRL_1_W, 
                                        E_MASKREG_P00_HVF_CNTRL_1_for,
                                        1);
            }

			pDis->prevBluescreen = True;
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
		if (pDis->prevBluescreen)
		{
			/* Restore the previous Matrix when pattern goes off */
			err = tmbslHdmiTxMatrixSetConversion ( txUnit, pDis->vinFmt, pDis->vinMode, pDis->voutFmt, pDis->voutMode, pDis->dviVqr);
			RETIF_REG_FAIL(err)

			/* Restore the correct output sampler mode */
			err = setSampling(pDis);
			RETIF(err != TM_OK, err)

			pDis->prevBluescreen = False;
		}

		/* Restore kBypassColourProc registers when pattern goes off */
		setHwRegister(pDis, E_REG_P00_MAT_CONTRL_W,  gMatContrl[txUnit]);
		setHwRegister(pDis, E_REG_P00_HVF_CNTRL_0_W, gHvfCntrl0[txUnit]);
		setHwRegister(pDis, E_REG_P00_HVF_CNTRL_1_W, gHvfCntrl1[txUnit]);
		pDis->prevPattern = False;

		/* restore scaler blanking if necessary */
		if (pDis->scaMode == HDMITX_SCAMODE_ON)
		{
			/* Look up scaler register formats from table */
			scInFmt = SCIO2SCIN(kVfmtToRegvfmtScio_TV[pDis->vinFmt]);
			scOutFmt = SCIO2SCOUT(kVfmtToRegvfmtScio_TV[pDis->voutFmt]);
			
			/* Look-up blanking re-insertion */
			hblOff = kSclAddBlkPix[scInFmt][scOutFmt];
			err = setHwRegister(pDis, E_REG_P00_HBL_OFFSET_START_W, hblOff);
			RETIF_REG_FAIL(err)
			/* clean green lines for none RGB output */
			err = setHwRegisterField(pDis, 
									E_REG_P00_HVF_CNTRL_1_W, 
									E_MASKREG_P00_HVF_CNTRL_1_yuvblk,
									kSclClearBlkPix[pDis->voutMode]);
			RETIF_REG_FAIL(err)
		}
	}

    /* Set Service Mode on or off */
    err = setHwRegisterField(pDis, 
                             E_REG_P00_HVF_CNTRL_0_W,
                             E_MASKREG_P00_HVF_CNTRL_0_sm,
                             serviceMode);

    return err;
}

/*============================================================================*/
/* tmbslHdmiTxTestSetMode                                                     */
/*============================================================================*/
#ifndef DEMO_BUILD
tmErrorCode_t
tmbslHdmiTxTestSetMode
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
        break;
    }
    err = setHwRegisterField(pDis, testReg, testMask, (UInt8)testState);
    return err;
}
#endif /* DEMO_BUILD */

/*============================================================================*/
/* tmbslHdmiTxVideoInSetBlanking                                              */
/*============================================================================*/
tmErrorCode_t
tmbslHdmiTxVideoInSetBlanking
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
/* tmbslHdmiTxVideoInSetConfig                                                */
/*============================================================================*/
tmErrorCode_t
tmbslHdmiTxVideoInSetConfig
(
    tmUnitSelect_t            txUnit,
    tmbslHdmiTxVinMode_t      vinMode,
    tmbslHdmiTxPixEdge_t      sampleEdge,
    tmbslHdmiTxPixRate_t      pixRate,
    tmbslHdmiTxUpsampleMode_t upsampleMode 
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

    if (vinMode != HDMITX_VINMODE_NO_CHANGE)
    {
        switch (vinMode)
        {
        case HDMITX_VINMODE_CCIR656:
            err = setHwRegisterFieldTable(pDis, &kVinModeCCIR656[0]);
            RETIF_REG_FAIL(err)
            break;
        case HDMITX_VINMODE_YUV422:           
            err = setHwRegisterFieldTable(pDis, &kVinModeYUV422[0]);
            RETIF_REG_FAIL(err)
            break;
        case HDMITX_VINMODE_RGB444:
        case HDMITX_VINMODE_YUV444:           
        default:
            err = setHwRegisterFieldTable(pDis, &kVinMode444[0]);
            RETIF_REG_FAIL(err)
            break;
        }
        err = setHwRegisterField(pDis,
                                 E_REG_P02_CCIR_DIV_RW,
                                 E_MASKREG_P02_CCIR_DIV_refdiv2,
                                 0);
        RETIF_REG_FAIL(err)
        pDis->vinMode = vinMode;
    }
    if (sampleEdge != HDMITX_PIXEDGE_NO_CHANGE)
    {
        err = setHwRegisterField(pDis,
                                 E_REG_P00_VIP_CNTRL_3_W,
                                 E_MASKREG_P00_VIP_CNTRL_3_edge,
                                 (UInt8)sampleEdge);
        RETIF_REG_FAIL(err)
    }
    if (pixRate != HDMITX_PIXRATE_NO_CHANGE)
    {
        pDis->pixRate = pixRate;

	if (pixRate == HDMITX_PIXRATE_SINGLE_REPEATED)
	{

	    err = setHwRegisterField(pDis,
                         E_REG_P02_CCIR_DIV_RW,
                         E_MASKREG_P02_CCIR_DIV_refdiv2,
                         1);
	    RETIF_REG_FAIL(err)

	    err = setHwRegisterField(pDis,
                         E_REG_P02_PLL_SCG2_RW,
                         E_MASKREG_P02_PLL_SCG2_selpllclkin,
                         0);
	    RETIF_REG_FAIL(err)

	    err = setHwRegisterField(pDis,
                         E_REG_P02_PLL_DE_RW,
                         E_MASKREG_P02_PLL_DE_pllde_fdn,
                         1);
	    RETIF_REG_FAIL(err)

	    err = setHwRegisterField(pDis,
                         E_REG_P02_PLL_DE_RW,
                         E_MASKREG_P02_PLL_DE_bypass_pllde,
                         1);
	    RETIF_REG_FAIL(err)
	}
	else
	{
        
	    err = setHwRegisterField(pDis,
                                 E_REG_P02_PLL_SCG2_RW,
                                 E_MASKREG_P02_PLL_SCG2_selpllclkin,
                                 (UInt8)pixRate);
	    RETIF_REG_FAIL(err)
	    err = setHwRegisterField(pDis,
                                 E_REG_P02_PLL_DE_RW,
                                 E_MASKREG_P02_PLL_DE_bypass_pllde,
                                 (UInt8)pixRate);
	    RETIF_REG_FAIL(err)
	    err = setHwRegisterField(pDis,
                                 E_REG_P02_PLL_DE_RW,
                                 E_MASKREG_P02_PLL_DE_pllde_fdn,
                                 (UInt8)pixRate);
	    RETIF_REG_FAIL(err)
	}
    }
    if (upsampleMode != HDMITX_UPSAMPLE_NO_CHANGE)
    {
        pDis->upsampleMode = upsampleMode;
    }
    return TM_OK;
}

/*============================================================================*/
/* tmbslHdmiTxVideoInSetFine                                                  */
/*============================================================================*/
tmErrorCode_t
tmbslHdmiTxVideoInSetFine
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
/* tmbslHdmiTxVideoInSetMapping                                               */
/*============================================================================*/
tmErrorCode_t
tmbslHdmiTxVideoInSetMapping
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
/* tmbslHdmiTxSetVideoPortConfig                                              */
/*============================================================================*/
tmErrorCode_t
tmbslHdmiTxSetVideoPortConfig
(
    tmUnitSelect_t  txUnit,
 	UInt8			*pEnaVideoPortTable,
	UInt8			*pGndVideoPortTable
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

#ifdef TMFL_TDA9981_SUPPORT
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

    err = setHwRegister(pDis,
                        E_REG_P00_GND_VP_0_RW,
						pGndVideoPortTable[0]);
    RETIF_REG_FAIL(err)

    err = setHwRegister(pDis,
                        E_REG_P00_GND_VP_1_RW,
						pGndVideoPortTable[1]);
    RETIF_REG_FAIL(err)

    err = setHwRegister(pDis,
                        E_REG_P00_GND_VP_2_RW,
						pGndVideoPortTable[2]);
    RETIF_REG_FAIL(err)

    return TM_OK;
#else /* TMFL_TDA9981_SUPPORT */
    (void)pEnaVideoPortTable;
	(void)pGndVideoPortTable;
    return TMBSL_ERR_HDMI_NOT_SUPPORTED;
#endif /* TMFL_TDA9981_SUPPORT */
}

/*============================================================================*/
/* tmbslHdmiTxSetAudioPortConfig                                              */
/*============================================================================*/
tmErrorCode_t
tmbslHdmiTxSetAudioPortConfig
(
    tmUnitSelect_t  txUnit,
 	UInt8			*pEnaAudioPortTable,
	UInt8			*pGndAudioPortTable
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

#ifdef TMFL_TDA9981_SUPPORT
    err = setHwRegister(pDis,
                        E_REG_P00_ENA_AP_RW,
						pEnaAudioPortTable[0]);
    RETIF_REG_FAIL(err)

	err = setHwRegister(pDis,
                        E_REG_P00_GND_AP_RW,
						pGndAudioPortTable[0]);
    RETIF_REG_FAIL(err)

    return TM_OK;
#else /* TMFL_TDA9981_SUPPORT */
    (void)pEnaAudioPortTable;
    (void)pGndAudioPortTable;
    return TMBSL_ERR_HDMI_NOT_SUPPORTED;
#endif /* TMFL_TDA9981_SUPPORT */
}

/*============================================================================*/
/* tmbslHdmiTxVideoInSetSyncAuto                                              */
/*============================================================================*/
tmErrorCode_t
tmbslHdmiTxVideoInSetSyncAuto
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
        vinFmtIndex =(tmbslHdmiTxVidFmt_t)(vinFmtIndex - HDMITX_VFMT_PC_MIN + HDMITX_VFMT_TV_MAX + 1);
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
        
        regVinFmtPC = (UInt8)(regVinFmt - E_REGVFMT_FIRST_PC_FORMAT);

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
/* tmbslHdmiTxVideoInSetSyncManual                                            */
/*============================================================================*/

tmErrorCode_t
tmbslHdmiTxVideoInSetSyncManual
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
/* tmbslHdmiTxVideoOutDisable                                                 */
/*============================================================================*/
#ifndef DEMO_BUILD
tmErrorCode_t
tmbslHdmiTxVideoOutDisable
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
#endif /* DEMO_BUILD */

/*============================================================================*/
/* tmbslHdmiTxVideoOutSetConfig                                               */
/*============================================================================*/
tmErrorCode_t
tmbslHdmiTxVideoOutSetConfig
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

       (void)DUMMY_ACCESS(quantization);

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

#ifndef NO_HDCP
        /* Enable EESS if HDCP 1.1 receiver and if 1.1 not disabled */
        if (((pDis->HdcpBcaps & E_MASKREG_P12_HDCP_BCAPS_RX_1_1) > 0)
        && ((pDis->HdcpOptions & HDMITX_HDCP_OPTION_FORCE_NO_1_1) == 0))
        {
            regVal = 1;
        }
        else
        {
            regVal = 0;
        }
#else
            regVal = 0;
#endif /* NO_HDCP */
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
/* tmbslHdmiTxVideoOutSetSync                                                 */
/*============================================================================*/
tmErrorCode_t
tmbslHdmiTxVideoOutSetSync
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
/* tmbslHdmiTxVideoSetInOut                                                   */
/*============================================================================*/
tmErrorCode_t
tmbslHdmiTxVideoSetInOut
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
   
    scaMode = scaModeRequest;

#ifdef TMFL_TDA9981_SUPPORT
        scaMode = HDMITX_SCAMODE_OFF;
#else /* TMFL_TDA9981_SUPPORT */
    if (voutFmt >= HDMITX_VFMT_TV_NO_REG_MIN)
    {
        /* Disable Scaler mode for PC_FORMAT and for 24/25/30Hz formats */
        scaMode = HDMITX_SCAMODE_OFF;
    }
#endif /* TMFL_TDA9981_SUPPORT */


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
    if (voutFmt == HDMITX_VFMT_TV_MIN)
    {
        pDis->voutMode = HDMITX_VOUTMODE_RGB444;
    }

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
            err = tmbslHdmiTxMatrixSetConversion(txUnit, vinFmt,
	      pDis->vinMode, voutFmt, pDis->voutMode, pDis->dviVqr);
        }
        else
        {
            err = tmbslHdmiTxMatrixSetMode(txUnit, HDMITX_MCNTRL_OFF, 
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
/*                   STATIC FUNCTION DEFINTIONS                               */
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
        voutFmtIndex = (tmbslHdmiTxVidFmt_t)(voutFmtIndex - HDMITX_VFMT_PC_MIN + HDMITX_VFMT_TV_MAX + 1);
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
    UInt8         regVfmt;      /* Video format value used for register */
    UInt8         ssd;          /* Packed srl, scg and de */
    tmbslHdmiTxVidFmt_t  voutFmtIndex; /* Index in tab kVfmtToRegvfmt_TV*/
    UInt8         *TabSettingPllSsd = Null; /* Pointer on a tab of kRegvfmtToPllSsd*/
    Bool          bIsTvFormat;  /* True if TV_FORMAT with internal chip confiration  found (default) */
    
    bIsTvFormat = True;

    /* IF voutFmt = No Change THEN return TM_OK */
    RETIF(voutFmt == HDMITX_VFMT_NO_CHANGE, TM_OK)

    voutFmtIndex = voutFmt;
#ifdef FORMAT_PC
    if (voutFmtIndex > HDMITX_VFMT_PC_MIN)
    {
        voutFmtIndex = (tmbslHdmiTxVidFmt_t)(voutFmtIndex - HDMITX_VFMT_PC_MIN + HDMITX_VFMT_TV_MAX + 1);
        bIsTvFormat = False; /* PC_FORMAT found */
    }
#endif /* FORMAT_PC */
    
        /* Quit if the output format does not map to the register format */
    regVfmt = kVfmtToRegvfmt_TV[voutFmtIndex];
    RETIF_BADPARAM(regVfmt == E_REGVFMT_INVALID)

#ifdef FORMAT_PC
    if (bIsTvFormat)
    {
#endif /* FORMAT_PC */
        switch (pDis->scaMode)
        {
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
							return TMBSL_ERR_HDMI_INCONSISTENT_PARAMS;
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
                                    TabSettingPllSsd = (UInt8 *)kRegvfmtToPllSsd.ScaOnCcirOffDblEdgeOffInterlace;
                                break;
                                
                                case HDMITX_VFMT_02_720x480p_60Hz:
                                case HDMITX_VFMT_03_720x480p_60Hz:
                                case HDMITX_VFMT_17_720x576p_50Hz:
                                case HDMITX_VFMT_18_720x576p_50Hz:
                                    TabSettingPllSsd = (UInt8 *)kRegvfmtToPllSsd.ScaOnCcirOffDblEdgeOffProgressif;
                                break;
                                default:
				    return TMBSL_ERR_HDMI_BAD_PARAMETER;
                            }
                        
                        break;
                        case HDMITX_PIXRATE_DOUBLE:
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

        /* Check parameters */
        RETIF_BADPARAM( regVfmt >= E_REGVFMT_NUM_TV )
        
		/* Table pointer must be valid here */
		if (TabSettingPllSsd)
		{
			/* Set PLLs based on output format */
			ssd = TabSettingPllSsd[regVfmt];
		}
		else 
		{
			return TMBSL_ERR_HDMI_ASSERTION;
        }

#ifdef FORMAT_PC
    }
    else
    {
        TabSettingPllSsd = (UInt8 *)kRegvfmtToPllSsd.SettingsFormatPC;
        
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
        RETIF_REG_FAIL(err)
        err = setHwRegisterField(pDis, E_REG_P02_PLL_SCG2_RW,
                             E_MASKREG_P02_PLL_SCG2_scg_nosc,
                             SSD2SCG(ssd));
        RETIF_REG_FAIL(err)
        err = setHwRegisterField(pDis, E_REG_P02_PLL_DE_RW,
                             E_MASKREG_P02_PLL_DE_pllde_nosc,
                             SSD2DE(ssd));
    }
    
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

    /* Check upsample mode saved by tmbslHdmiTxVideoInSetConfig */
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
#ifndef TMFL_TDA9981_SUPPORT
    UInt8         scInFmt = 0;  /* Scaler input format */
    UInt8         scOutFmt = 0; /* Scaler output format */
    UInt16        pllScgN;      /* PLL scaling values */
    UInt16        pllScgR;
    UInt8		  hblOff;		/* FB: define hbl offset*/

#endif /* TMFL_TDA9981_SUPPORT */

#ifdef FORMAT_PC
    RETIF_BADPARAM(vinFmt        > HDMITX_VFMT_PC_MAX)
    RETIF_BADPARAM(voutFmt        > HDMITX_VFMT_PC_MAX)
#else
    RETIF_BADPARAM(vinFmt        > HDMITX_VFMT_TV_MAX)
    RETIF_BADPARAM(voutFmt        > HDMITX_VFMT_TV_MAX)
#endif
    
    /* Look up the VIDFORMAT register formats from the register format table */
    regVinFmt = kVfmtToRegvfmt_TV[vinFmt];
    regVoutFmt = kVfmtToRegvfmt_TV[voutFmt];
    /* Quit if the output format does not map to the register format */
    RETIF_BADPARAM(regVoutFmt == E_REGVFMT_INVALID)

    if ((vinFmt > HDMITX_VFMT_TV_MAX)
    ||  (voutFmt > HDMITX_VFMT_TV_MAX)
    ||  (regVinFmt == regVoutFmt))
    {
		/* Disable scaler for PC formats or if same input and output formats */
        err = setHwRegisterFieldTable(pDis, &kScalerOff[0]);
        RETIF_REG_FAIL(err)

#ifndef TMFL_TDA9981_SUPPORT	
		if (pDis->scaMode == HDMITX_SCAMODE_ON)
		{			
			/* remove blanking */
			err = setHwRegister(pDis, E_REG_P00_HBL_OFFSET_START_W, 0);
			RETIF_REG_FAIL(err)
		}
#endif	/* TMFL_TDA9981_SUPPORT */

        pDis->scaMode = HDMITX_SCAMODE_OFF;
    }
    else
    {
#ifdef TMFL_TDA9981_SUPPORT
        return TMBSL_ERR_HDMI_NOT_SUPPORTED;
#else
        /* Need to scale: quit if no scaler */
        RETIF(pDis->uDeviceFeatures & E_MASKREG_P00_VERSION_not_s,
              TMBSL_ERR_HDMI_NOT_SUPPORTED)

        /* Look up scaler register formats from table */
        scInFmt = SCIO2SCIN(kVfmtToRegvfmtScio_TV[vinFmt]);
        scOutFmt = SCIO2SCOUT(kVfmtToRegvfmtScio_TV[voutFmt]);

        /* Do these formats individually support scaling? */
        RETIF(scInFmt  == E_REGVFMT_SCIN_INVALID,
            TMBSL_ERR_HDMI_INCONSISTENT_PARAMS)
        RETIF(scOutFmt == E_REGVFMT_SCOUT_INVALID,
            TMBSL_ERR_HDMI_INCONSISTENT_PARAMS)

        /* Can the i/p register format be scaled to the o/p register format? */
        RETIF(CAN_FMTS_SCALE(regVinFmt,regVoutFmt) == 0,
            TMBSL_ERR_HDMI_INCONSISTENT_PARAMS)

        /* Scaling is possible: Enable scaler */
        err = setHwRegisterFieldTable(pDis, &kScalerOn[0]);
        RETIF_REG_FAIL(err)

        /* Set scaler input format */
        err = setHwRegister(pDis, E_REG_P01_VIDFORMAT_W, scInFmt);
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

        /* Look-up blanking insertion */
        hblOff = kSclAddBlkPix[scInFmt][scOutFmt];
        err = setHwRegister(pDis, E_REG_P00_HBL_OFFSET_START_W, hblOff);
        RETIF_REG_FAIL(err)
		/* clean green lines for none RGB output */
		err = setHwRegisterField(pDis, 
								E_REG_P00_HVF_CNTRL_1_W, 
								E_MASKREG_P00_HVF_CNTRL_1_yuvblk,
								kSclClearBlkPix[pDis->voutMode]);
		RETIF_REG_FAIL(err)
#endif /* TMFL_TDA9981_SUPPORT */
    }
    
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

#ifndef TMFL_TDA9981_SUPPORT
    if (pDis->scaMode == HDMITX_SCAMODE_ON)
    {
        /* Set scaler input format again */
        /* MUST BE AFTER SCALER CLOCK or sometimes the 9983 won't take value */
        err = setHwRegister(pDis, E_REG_P01_VIDFORMAT_W, scInFmt);
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
#endif /* TMFL_TDA9981_SUPPORT */
    return err;
}

#ifndef NO_HDCP
/*============================================================================*/
/* handleBKSVResultSecure                                                     */
/*============================================================================*/
static tmErrorCode_t
handleBKSVResultSecure
(
    tmUnitSelect_t	txUnit
)
{
    tmHdmiTxobject_t *pDis;     /* Pointer to Device Instance Structure */
    tmErrorCode_t     err;      /* Error code */

    /* Check unit parameter and point to its object */
    err = checkUnitSetDis(txUnit, &pDis);
    RETIF(err != TM_OK, err)

    /* BKSV is secure: check if receiver is a repeater */
    if ((pDis->HdcpBcaps & E_MASKREG_P12_HDCP_BCAPS_RX_repeater) > 0)
    {
        err = setHwRegisterFieldTable(pDis, &kSet5secWatchdog[0]);
    	RETIF_REG_FAIL(err) 
        /* Go to next part of authentication for a repeater */
        setState(pDis, EV_HDCP_BKSV_REPEAT);
    }
    else
    {
        err = setHwRegisterFieldTable(pDis, &kSet1sWatchdog[0]);
        RETIF_REG_FAIL(err)
        /* A secure non-repeater has authenticated */
        setState(pDis, EV_HDCP_BKSV_NREPEAT);
    }
    /* BKSV is secure, disable BCAPS interrupt */
    err = setHwRegisterFieldTable(pDis, &kBksvSecure[0]);
    return err;
}


/*============================================================================*/
/* tmbslHdmiTxHdcpGetSinkCategory                                             */
/*============================================================================*/
tmErrorCode_t
tmbslHdmiTxHdcpGetSinkCategory
(
    tmUnitSelect_t              txUnit,
    tmbslHdmiTxSinkCategory_t   *category
)
{
    tmHdmiTxobject_t *pDis;     /* Pointer to Device Instance Structure */
    tmErrorCode_t     err;      /* Error code */

    /* Check unit parameter and point to its object */
    err = checkUnitSetDis(txUnit, &pDis);
    RETIF(err != TM_OK, err)

    RETIF(category == Null, TMBSL_ERR_HDMI_INCONSISTENT_PARAMS)

    if ((pDis->state == ST_HDCP_WAIT_RX)
        ||
        (pDis->state == ST_HDCP_WAIT_BSTATUS)
        ||
        (pDis->state == ST_HDCP_WAIT_SHA_1)
        ||
        (pDis->state == ST_HDCP_AUTHENTICATED)
       )
    {
        /* check if receiver is a repeater */
        if ((pDis->HdcpBcaps & E_MASKREG_P12_HDCP_BCAPS_RX_repeater) > 0)
        {
            *category = HDMITX_SINK_CAT_REPEATER;
        }
        else
        {
            *category = HDMITX_SINK_CAT_NOT_REPEATER;
        }
    }
    else
    {
        err = TMBSL_ERR_HDMI_OPERATION_NOT_PERMITTED;
    }

    return err;
}

#endif /* NO_HDCP */
/*============================================================================*/
/*                     END OF FILE                                            */
/*============================================================================*/
