/**
 * Copyright (C) 2006 Koninklijke Philips Electronics N.V., All Rights Reserved.
 * This source code and any compilation or derivative thereof is the proprietary
 * information of Koninklijke Philips Electronics N.V. and is confidential in
 * nature. Under no circumstances is this software to be  exposed to or placed
 * under an Open Source License of any type without the expressed written
 * permission of Koninklijke Philips Electronics N.V.
 *
 * \file          tmbslHdmiTx_app.c
 *
 * \version       $Revision: 65 $
 *
 * \date          $Date: 18/02/08 $
 *
 * \brief         Optional top-level application functions to simplify the
 *                use of the TDA998x HDMI Transmitter BSL driver component.
 *                This module corresponds to the DVP 'device' layer.
 *                This code can be used directly in user applications, with
 *                areas of customisation indicated by 'todo' comments.
 *
 * \section refs  Reference Documents
 * HDMI Driver - Outline Architecture.doc
 * HDMI Driver - tmbslHdmiTx - SCS.doc
 *
 * \section info  Change Information
 *
 * \verbatim
   $History: tmbslHdmiTx_app.c $
 *
 ******************  Version 65  ****************
 * User: G. Burnouf     Date: 18/02/08
 * Updated in $/Source/tmbslHdmiTx/src
 * PR1355 : Set audio channel allocation
 *
 ******************  Version 64  ****************
 * User: G. Burnouf     Date: 03/01/08
 * Updated in $/Source/tmbslHdmiTx/src
 * PR982 : replace memset by lAppMemset
 *
 * *****************  Version 63  ****************
 * User: B.Vereecke     Date: 29/10/07   Time: 14:11
 * Updated in $/Source/tmbslHdmiTx/src
 * PR833 : update appHdmiTx_setAudio error codes
 *
 * *****************  Version 62  ****************
 * User: B.Vereecke     Date: 17/10/07   Time: 14:11
 * Updated in $/Source/tmbslHdmiTx/src
 * PR872 : add new formats, 1080p24/25/30
 *
 * ****************  Version 61  ****************
 * User: B.Vereecke     Date: 10/10/07   Time: 11:11
 * Updated in $/Source/tmbslHdmiTx/src
 * PR815 : Update bluescreen infoframes
 *         According to the output mode
 *
 * * **************  Version 60  *****************
 * User: B.Vereecke   Date: 19/09/07   Time: 09:44
 * Updated in $/Source/tmbslHdmiTx/src
 * PR721   - Fix Bug in appHdmiTx_setInputOutput()
 *           when sink parameter is HDMITX_SINK_EDID
 *
 * * **************  Version 59  *****************
 * User: B.Vereecke   Date: 07/09/07   Time: 18:11
 * Updated in $/Source/tmbslHdmiTx/src
 * PR673   - Fix bad parameter error in Pc format
 *           in function setVideoInfoframe()
 *
 * * **************  Version 58  *****************
 * User: B.Vereecke   Date: 03/09/07   Time: 11:11
 * Updated in $/Source/tmbslHdmiTx/src
 * PR647   - Fix bad_parameter error in DVI mode
 *           Update in appHdmiTx_setInputOutput()
 *
 * * **************  Version 57  *****************
 * User: B.Vereecke   Date: 30/08/07   Time: 14:45
 * Updated in $/Source/tmbslHdmiTx/src
 * PR4   - add new appHdmiTx_ReadEdidAtPowerOn()
 *         function, calling result after resuming 
 *         from power_down is a EDID reading.
 *
 * * **************  Version 56  *****************
 * User: B.Vereecke   Date: 23/08/07   Time: 09:45
 * Updated in $/Source/tmbslHdmiTx/Inc
 * PR609 - in appHdmiTx_handleSHA_1() for TDA9981 
 *         only:Fix problem in repeater mode with 
 *         devcount=0
 *
 * * **************  Version 55  *****************
 * User: B.Vereecke   Date: 23/07/07   Time: 13:18
 * Updated in $/Source/tmbslHdmiTx/Inc
 * PR502 - Add new appHdmiTx_setAudio() function
 *         for change Audio parameters without update
 *         video input/output
 *
 *****************  Version 54  ******************
 * User: B.Vereecke   Date: 19/07/07   Time: 10:30
 * Updated in $/Source/tmbslHdmiTx/Inc
 * PR511 - add new PixelEdge argument in 
 *          appHdmiTx_setInputOutput
 *
 *****************  Version 53  ******************
 * User: B.Vereecke   Date: 17/07/07   Time: 10:30
 * Updated in $/Source/tmbslHdmiTx/Inc
 * PR217 - Add Pattern type parameter to
 *         appHdmiTx_test_pattern_on API in order
 *         to add blue pattern functionality
 *
 *****************  Version 52  ******************
 * User: J. Lamotte   Date: 29/06/07   Time: 15:50
 * Updated in $/Source/tmbslHdmiTx/Inc
 * PR210 - Add video output format parameter to
 *         appHdmiTx_test_pattern_on API in order
 *         to update AVI info frame for color bar.
 *
 *****************  Version 51  ******************
 * User: J. Lamotte   Date: 27/06/07   Time: 11:23
 * Updated in $/Source/tmbslHdmiTx/Inc
 * PR456 - For TDA9981 only, change the serializer
 *         register configuration, srl_phase3 = 8
 *         and srl_phase2 = 4.
 *         (tmbslHdmiTxTmdsSetSerializer parameter)
 *
 *****************  Version 50  ******************
 * User: J. Lamotte   Date: 26/06/07   Time: 11:21
 * Updated in $/Source/tmbslHdmiTx/Inc
 * PR457 - In appHdmiTx_handleSHA_1 function,
 *         check bksv list for top level only 
 *
 *****************  Version 49  ******************
 * User: J. Lamotte   Date: 22/06/07   Time: 16:00
 * Updated in $/Source/tmbslHdmiTx/Inc
 * PR448 : Make the initialisation of shortVinFmt and
 *         shortVoutFmt in getReflineRefPix function
 *         only for TV format to prevent buffer overflow
 * PR385 : Execute tmbslHdmiTxHdcpHandleBKSVResult
 *         even if bCheckRequired is false to
 *         continue HDCP authentification for 
 *        repeater in appHdmiTx_handleBCAPS
 *
 *****************  Version 48  ******************
 * User: burnouf      Date: 08/06/07   Time: 17:03
 * Updated in $/Source/tmbslHdmiTx/Inc
 * PR347 - add new PC Formats
 *
 ******************  Version 47  ******************
 * User: J. Lamotte      Date: 08/06/07   Time: 09:30
 * Updated in $/Source/tmbslHdmiTx/src
 * PR377 - Reset fifo applied before audio configuration
 *
 *******************  Version 46  *****************
 * User: J. Lamotte      Date: 16/04/07   Time: 11:30
 * Updated in $/Source/tmbslHdmiTx/src
 * PR50 - add configuration file for TDA9981
 *      - add support of TDA9981 in appHdmiTx_setInputOutput 
 *
 *******************  Version 45  *****************
 * User: C. Logiou      Date: 08/03/07   Time: 16:52
 * Updated in $/Source/tmbslHdmiTx/src
 * PR214 - add new input format repeated 480i/576i 
 *
 * *****************  Version 44  *****************
 * User: burnouf      Date: 06/02/07   Time: 17:03
 * Updated in $/Source/tmbslHdmiTx/Inc
 * PR49 - add PC Formats
 *
 * *****************  Version 43  *****************
 * User: Burnouf      Date: 14/12/06   Time: 09:54
 * Updated in $/Source/tmbslHdmiTx/Src
 * PR48 Update value Refpix and Refline
 * 
 * *****************  Version 42  *****************
 * User: Burnouf      Date: 07/12/06   Time: 16:30
 * Updated in $/Source/tmbslHdmiTx/Src
 * PR46 Change error number
 * 
 * *****************  Version 41  *****************
 * User: Burnouf      Date: 07/12/06   Time: 11:17
 * Updated in $/Source/tmbslHdmiTx/Src
 * PR46 set serial phase
 *
 * *****************  Version 40  *****************
 * User: Mayhew       Date: 23/11/06   Time: 15:07
 * Updated in $/Source/tmbslHdmiTx/Src
 * PNF77 Wait 500ms before reading EDID when hotplug=on, to allow DDC
 * connection to settle and avoid EDID checksum errors
 *
 * *****************  Version 38  *****************
 * User: Djw          Date: 10/11/06   Time: 14:00
 * Updated in $/Source/tmbslHdmiTx/Src
 * PNF69 Add call to tmbslHdmiTxAudioOutSetChanStatus in
 * appHdmiTx_setInputOutput.
 * 
 * *****************  Version 37  *****************
 * User: Mayhew       Date: 10/11/06   Time: 10:12
 * Updated in $/Source/tmbslHdmiTx/Src
 * PNF68 HDCP check parameters HDCP_CHECK_INTERVAL_MS and HDCP_NUM_CHECKS
 * added and passed to tmbslHdmiTxHdcpConfigure
 * 
 * *****************  Version 35  *****************
 * User: Mayhew       Date: 31/10/06   Time: 16:23
 * Updated in $/Source/tmbslHdmiTx/Src
 * PNF65 setVideoInfoframe ignores OPERATION_NOT_PERMITTED, to work in DVI
 * mode
 * PNF66 New kRefpixRefline values for CCIR656+EMB
 * 
 * *****************  Version 33  *****************
 * User: Mayhew       Date: 27/10/06   Time: 12:31
 * Updated in $/Source/tmbslHdmiTx/Src
 * PNF59 Test pattern APi split into _on and _off APIs
 * PNF59 appHdmiTx_handleBCAPS has new pbBksvSecure parameter
 * Comments added to highlight user customisation
 * appHdmiTx_Hdcp_On no longer resets EdidVFmtsAvail
 * 
 * *****************  Version 31  *****************
 * User: Djw          Date: 25/10/06   Time: 13:36
 * Updated in $/Source/tmbslHdmiTx/Src
 * PNF58 Added vinMode to call to tmbslHdmiTxVideoInSetSyncAuto
 * 
 * *****************  Version 29  *****************
 * User: Djw          Date: 25/10/06   Time: 11:52
 * Updated in $/Source/tmbslHdmiTx/Src
 * PNF57 Change the handleBCAPS timer from 100ms to 10ms
 * 
 * *****************  Version 27  *****************
 * User: Djw          Date: 24/10/06   Time: 17:53
 * Updated in $/Source/tmbslHdmiTx/Src
 * PNF2 (part) New RefPix table values for scaled 480p/576p
 * 
 * *****************  Version 25  *****************
 * User: Mayhew       Date: 23/10/06   Time: 16:41
 * Updated in $/Source/tmbslHdmiTx/Src
 * PNF37 Refpix table size reduced to save code space
 * PNF53 Output field order corrected for p->i conversion
 * 
 * *****************  Version 23  *****************
 * User: Mayhew       Date: 13/10/06   Time: 11:17
 * Updated in $/Source/tmbslHdmiTx/Src
 * PNF37 Pass syncSource not syncIn. Comment out non-scaler RefPix table
 * combinations. Use uRefPix = 0 to do new VideoInSetSyncAuto algorithm.
 * PNF43 Scaler field order for 1080i
 * PNF47 Audio latency
 * 
 * *****************  Version 21  *****************
 * User: Mayhew       Date: 21/09/06   Time: 15:45
 * Updated in $/Source/tmbslHdmiTx/Src
 * PNF35 Only set channel count in audio info frame
 * PNF36 576p->1080i,DE,EMB,CCIR656 now verified
 * 
 * *****************  Version 19  *****************
 * User: Mayhew       Date: 15/09/06   Time: 16:07
 * Updated in $/Source/tmbslHdmiTx/Src
 * PNF19 Add i2sQualifier arg to setInputOutput, set AIF
 * PNF25 Add pEdidVidFlags arg to handleHPD
 * Fix Raisonance and QAC warnings
 * 
 * *****************  Version 17  *****************
 * User: Mayhew       Date: 7/09/06    Time: 9:43
 * Updated in $/Source/tmbslHdmiTx/Src
 * New refpix table
 * 
 * *****************  Version 15  *****************
 * User: Djw          Date: 24/08/06   Time: 12:07
 * Updated in $/Source/tmbslHdmiTx/Src
 * Add new refpix/refline values.
 * PNF13 TdmsSetOutputs uses FORCED0 not FORCED1.
 * PNF14 Set correct pixel repetition.
 * PNF16 Set correct cts values for I2S.
 * 
 * *****************  Version 14  *****************
 * User: Djw          Date: 22/08/06   Time: 9:59
 * Updated in $/Source/tmbslHdmiTx/Src
 * NO_HDCP modifications. Change Request 13.
 * 
 * *****************  Version 13  *****************
 * User: Mayhew       Date: 10/07/06   Time: 13:15
 * Updated in $/Source/tmbslHdmiTx/Src
 * Fix file header comment. Aspect ratio codes now 1 not 0 for 4:3. 
 * Fix VGA format lookup. 
 * New refpix/refline values. Add Verified value for each combination.
 * Sync Once values now based on scaler mode not output format.
 * Reduce delay before HandleBKSV for reliability. Fix GCC warnings.
 * 
 * *****************  Version 11  *****************
 * User: Mayhew       Date: 30/06/06   Time: 12:44
 * Updated in $/Source/tmbslHdmiTx/Src
 * Add audioFmt parameter to appHdmiTx_setInputOutput.
 * New kRefpixRefline values, based on pixRate not voutMode.
 * 
 * *****************  Version 10  *****************
 * User: Mayhew       Date: 29/06/06   Time: 16:35
 * Updated in $/Source/tmbslHdmiTx/Src
 * New refpix/refline values. Add audioFmt audio format argument. Collect
 * all input sync settings under syncIn argument. Add RGB swapTable
 * values.
 * 
 * *****************  Version 8  *****************
 * User: Mayhew       Date: 5/06/06    Time: 15:02
 * Updated in $/Source/tmbslHdmiTx/Src
 * Rename syncSrc to syncIn. Remove 2nd param from handleBCAPS. 
 * Add pVidFmtNew param to appHdmiTx_nextEdidVidFmt.
 * Input port mapping depends on vinMode.
 * Several settings differ for CCIR656 rather than set as constants.
 * Use table of refpix/refline values for combinations of input/output.
 * Use table fo aspect ratios per vouFmt
 * 
 * *****************  Version 6  *****************
 * User: Mayhew       Date: 22/05/06   Time: 15:56
 * Updated in $/Source/tmbslHdmiTx/Src
 * Add pixRate to appHdmiTx_setInputOutput. Add appHdmiTx_nextEdidVidFmt.
 * 
 * *****************  Version 4  *****************
 * User: Mayhew       Date: 19/05/06   Time: 11:33
 * Updated in $/Source/tmbslHdmiTx/Src
 * Add options parameter to appHdmiTx_Hdcp_On
 * 
 * *****************  Version 3  *****************
 * User: Mayhew       Date: 10/05/06   Time: 17:09
 * Updated in $/Source/tmbslHdmiTx/Src
 * New APIs for HDCP and parameterised format setting
 * 
 * *****************  Version 2  *****************
 * User: Mayhew       Date: 11/04/06   Time: 14:18
 * Updated in $/Source/tmbslHdmiTx/Src
 * Fixed bad colours in phase 2 demo: MATMODE_AUTO not _OFF, UPSAMPLE_AUTO
 * not _COPY. 
 * ScalerSetFine values changed from script for 1080i.
 * SD output format uses 59Hz not 60Hz.
 * For cleaner image changes, disabled scaler input instead of output
 * frame.
   \endverbatim
 *
*/

/*============================================================================*/
/*                   STANDARD INCLUDE FILES                                   */
/*============================================================================*/



/*============================================================================*/
/*                   PROJECT INCLUDE FILES                                    */
/* Note that tmbslHdmiTx_local.h is NOT included, because local driver data   */
/* should NOT be accessed from application code.                              */
/*============================================================================*/
#include "tmbslHdmiTx.h"
#include "tmbslHdmiTx_app.h"

#ifdef TMFL_TDA9981_SUPPORT
#include "tmbslHdmiTx_cfg.h"
#endif /* TMFL_TDA9981_SUPPORT */

/*============================================================================*/
/*                   MACRO DEFINITIONS                                        */
/*============================================================================*/

/**
 * A macro to check a condition and if true return a result
 */
#define RETIF(cond, rslt)       if ((cond)){return (rslt);}

/* Arbitrary short TV format values */
#define TV_INVALID      0
#define TV_VGA_60Hz     1
#define TV_240p_60Hz    2
#define TV_480p_60Hz    3
#define TV_480i_60Hz    4
#define TV_720p_60Hz    5
#define TV_1080p_60Hz   6
#define TV_1080i_60Hz   7
#define TV_288p_50Hz    8
#define TV_576p_50Hz    9
#define TV_576i_50Hz    10
#define TV_720p_50Hz    11
#define TV_1080p_50Hz   12
#define TV_1080i_50Hz   13


/* Shorthands for vinMode values in tmbslHdmiTx.h */
#define iINVALID HDMITX_VINMODE_INVALID
#define iCCIR656 HDMITX_VINMODE_CCIR656
#define iRGB444  HDMITX_VINMODE_RGB444 
#define iYUV444  HDMITX_VINMODE_YUV444 
#define iYUV422  HDMITX_VINMODE_YUV422 

/* Shorthands for input sync */
#define EMB      1
#define EXT      0

/* Shorthands for single/double pixel rate in tmbslHdmiTx.h */
#define SINGLE   HDMITX_PIXRATE_SINGLE
#define DOUBLE   HDMITX_PIXRATE_DOUBLE

/* Shorthands for sampling frequency in appHdmiTx_setAudio API */
#define AIF_SF_REFER_TO_STREAM_HEADER   0
#define AIF_SF_32K                      1
#define AIF_SF_44K                      2
#define AIF_SF_48K                      3
#define AIF_SF_88K                      4
#define AIF_SF_96K                      5
#define AIF_SF_176K                     6
#define AIF_SF_192K                     7

/* HDCP check interval in milliseconds */
#define HDCP_CHECK_INTERVAL_MS 2500

/* Number of HDCP checks to carry out after HDCP is started */
#define HDCP_NUM_CHECKS 5

/*============================================================================*/
/*                   TYPE DEFINITIONS                                         */
/*============================================================================*/


/*============================================================================*/
/*                   PUBLIC VARIABLE DEFINITIONS                              */
/*============================================================================*/


/*============================================================================*/
/*                   STATIC CONSTANT DECLARATIONS                             */
/*============================================================================*/

#define APP_HDMITX_CHANNELALLOC_LUT_SIZE 32

static CONST_DAT UInt8 kChanAllocChanNum[APP_HDMITX_CHANNELALLOC_LUT_SIZE] = \
{2,3,3,4,3,4,4,5,4,5,5,6,5,6,6,7,6,7,7,8,4,5,5,6,5,6,6,7,6,7,7,8 };

/**
 * Lookup table to convert from EIA/CEA TV video format to
 * aspect ratio used in video infoframe:
 * Aspect ratio 1=4:3, 2=16:9
 */
#ifndef FORMAT_PC
static CONST_DAT UInt8 kVfmtToAspect_TV[HDMITX_VFMT_TV_NUM] =
#else /* FORMAT_PC */
static CONST_DAT UInt8 kVfmtToAspect_TV[HDMITX_VFMT_TV_NUM + HDMITX_VFMT_PC_NUM] =
#endif /* FORMAT_PC */
{
    0,   /* HDMITX_VFMT_NULL               */
    1,   /* HDMITX_VFMT_01_640x480p_60Hz   */
    1,   /* HDMITX_VFMT_02_720x480p_60Hz   */
    2,   /* HDMITX_VFMT_03_720x480p_60Hz   */
    2,   /* HDMITX_VFMT_04_1280x720p_60Hz  */
    2,   /* HDMITX_VFMT_05_1920x1080i_60Hz */
    1,   /* HDMITX_VFMT_06_720x480i_60Hz   */
    2,   /* HDMITX_VFMT_07_720x480i_60Hz   */
    1,   /* HDMITX_VFMT_08_720x240p_60Hz   */
    2,   /* HDMITX_VFMT_09_720x240p_60Hz   */
    1,   /* HDMITX_VFMT_10_720x480i_60Hz   */
    2,   /* HDMITX_VFMT_11_720x480i_60Hz   */
    1,   /* HDMITX_VFMT_12_720x240p_60Hz   */
    2,   /* HDMITX_VFMT_13_720x240p_60Hz   */
    1,   /* HDMITX_VFMT_14_1440x480p_60Hz  */
    2,   /* HDMITX_VFMT_15_1440x480p_60Hz  */
    2,   /* HDMITX_VFMT_16_1920x1080p_60Hz */
    1,   /* HDMITX_VFMT_17_720x576p_50Hz   */
    2,   /* HDMITX_VFMT_18_720x576p_50Hz   */
    2,   /* HDMITX_VFMT_19_1280x720p_50Hz  */
    2,   /* HDMITX_VFMT_20_1920x1080i_50Hz */
    1,   /* HDMITX_VFMT_21_720x576i_50Hz   */
    2,   /* HDMITX_VFMT_22_720x576i_50Hz   */
    1,   /* HDMITX_VFMT_23_720x288p_50Hz   */
    2,   /* HDMITX_VFMT_24_720x288p_50Hz   */
    1,   /* HDMITX_VFMT_25_720x576i_50Hz   */
    2,   /* HDMITX_VFMT_26_720x576i_50Hz   */
    1,   /* HDMITX_VFMT_27_720x288p_50Hz   */
    2,   /* HDMITX_VFMT_28_720x288p_50Hz   */
    1,   /* HDMITX_VFMT_29_1440x576p_50Hz  */
    2,   /* HDMITX_VFMT_30_1440x576p_50Hz  */
    2,   /* HDMITX_VFMT_31_1920x1080p_50Hz */
    2,   /* HDMITX_VFMT_32_1920x1080p_24Hz */
    2,   /* HDMITX_VFMT_33_1920x1080p_25Hz */
    2    /* HDMITX_VFMT_34_1920x1080p_30Hz */
#ifdef FORMAT_PC
   ,1,   /* HDMITX_VFMT_PC_640x480p_60Hz  */
    1,   /* HDMITX_VFMT_PC_800x600p_60Hz  */
    1,   /* HDMITX_VFMT_PC_1152x960p_60Hz  */
    1,   /* HDMITX_VFMT_PC_1024x768p_60Hz  */
    1,   /* HDMITX_VFMT_PC_1280x768p_60Hz  */
    1,   /* HDMITX_VFMT_PC_1280x1024p_60Hz  */
    1,   /* HDMITX_VFMT_PC_1360x768p_60Hz */
    1,   /* HDMITX_VFMT_PC_1400x1050p_60Hz */
    1,   /* HDMITX_VFMT_PC_1600x1200p_60Hz  */
    1,   /* HDMITX_VFMT_PC_1024x768p_70Hz  */
    1,   /* HDMITX_VFMT_PC_640x480p_72Hz  */
    1,   /* HDMITX_VFMT_PC_800x600p_72Hz  */
    1,   /* HDMITX_VFMT_PC_640x480p_75Hz  */
    1,   /* HDMITX_VFMT_PC_1024x768p_75Hz  */
    1,   /* HDMITX_VFMT_PC_800x600p_75Hz  */
    1,   /* HDMITX_VFMT_PC_1024x864p_75Hz  */
    1,   /* HDMITX_VFMT_PC_1280x1024p_75Hz  */
    1,   /* HDMITX_VFMT_PC_640x350p_85Hz  */
    1,   /* HDMITX_VFMT_PC_640x400p_85Hz  */
    1,   /* HDMITX_VFMT_PC_720x400p_85Hz  */
    1,   /* HDMITX_VFMT_PC_640x480p_85Hz  */
    1,   /* HDMITX_VFMT_PC_800x600p_85Hz  */
    1,   /* HDMITX_VFMT_PC_1024x768p_85Hz  */
    1,   /* HDMITX_VFMT_PC_1152x864p_85Hz  */
    1,   /* HDMITX_VFMT_PC_1280x960p_85Hz  */
    1,   /* HDMITX_VFMT_PC_1280x1024p_85Hz */
    1    /* HDMITX_VFMT_PC_1024x768i_87Hz  */
#endif /* FORMAT_PC */
};

/**
 * Lookup table to convert from EIA/CEA TV video format to
 * the short format of resolution/interlace/frequency
 */
static CONST_DAT UInt8 kVfmtToShortFmt_TV[HDMITX_VFMT_TV_NUM] =
{
    TV_INVALID,         /* HDMITX_VFMT_NULL               */
    TV_VGA_60Hz,        /* HDMITX_VFMT_01_640x480p_60Hz   */
    TV_480p_60Hz,       /* HDMITX_VFMT_02_720x480p_60Hz   */
    TV_480p_60Hz,       /* HDMITX_VFMT_03_720x480p_60Hz   */
    TV_720p_60Hz,       /* HDMITX_VFMT_04_1280x720p_60Hz  */
    TV_1080i_60Hz,      /* HDMITX_VFMT_05_1920x1080i_60Hz */
    TV_480i_60Hz,       /* HDMITX_VFMT_06_720x480i_60Hz   */
    TV_480i_60Hz,       /* HDMITX_VFMT_07_720x480i_60Hz   */
    TV_240p_60Hz,       /* HDMITX_VFMT_08_720x240p_60Hz   */
    TV_240p_60Hz,       /* HDMITX_VFMT_09_720x240p_60Hz   */
    TV_480i_60Hz,       /* HDMITX_VFMT_10_720x480i_60Hz   */
    TV_480i_60Hz,       /* HDMITX_VFMT_11_720x480i_60Hz   */
    TV_240p_60Hz,       /* HDMITX_VFMT_12_720x240p_60Hz   */
    TV_240p_60Hz,       /* HDMITX_VFMT_13_720x240p_60Hz   */
    TV_480p_60Hz,       /* HDMITX_VFMT_14_1440x480p_60Hz  */
    TV_480p_60Hz,       /* HDMITX_VFMT_15_1440x480p_60Hz  */
    TV_1080p_60Hz,      /* HDMITX_VFMT_16_1920x1080p_60Hz */
    TV_576p_50Hz,       /* HDMITX_VFMT_17_720x576p_50Hz   */
    TV_576p_50Hz,       /* HDMITX_VFMT_18_720x576p_50Hz   */
    TV_720p_50Hz,       /* HDMITX_VFMT_19_1280x720p_50Hz  */
    TV_1080i_50Hz,      /* HDMITX_VFMT_20_1920x1080i_50Hz */
    TV_576i_50Hz,       /* HDMITX_VFMT_21_720x576i_50Hz   */
    TV_576i_50Hz,       /* HDMITX_VFMT_22_720x576i_50Hz   */
    TV_288p_50Hz,       /* HDMITX_VFMT_23_720x288p_50Hz   */
    TV_288p_50Hz,       /* HDMITX_VFMT_24_720x288p_50Hz   */
    TV_576i_50Hz,       /* HDMITX_VFMT_25_720x576i_50Hz   */
    TV_576i_50Hz,       /* HDMITX_VFMT_26_720x576i_50Hz   */
    TV_288p_50Hz,       /* HDMITX_VFMT_27_720x288p_50Hz   */
    TV_288p_50Hz,       /* HDMITX_VFMT_28_720x288p_50Hz   */
    TV_576p_50Hz,       /* HDMITX_VFMT_29_1440x576p_50Hz  */
    TV_576p_50Hz,       /* HDMITX_VFMT_30_1440x576p_50Hz  */
    TV_1080p_50Hz,      /* HDMITX_VFMT_31_1920x1080p_50Hz */
};

/**
 * Macro to pack vinMode(0-5), pixRate(0-1), syncIn(0-1) and bVerified(0-1)
 * into a byte
 */
#define PKBYTE(mode,rate,sync,verf) (((rate)<<7)|((sync)<<6)|((verf)<<5)|((mode)&0x1F))

/**
 * Macros to unpack vinMode(0-5), pixRate(0-1), syncIn(0-1) and bVerified(0-1)
 * from a byte
 */
#define UNPKRATE(byte) (((byte)>>7)&1)
#define UNPKSYNC(byte) (((byte)>>6)&1)
#define UNPKVERF(byte) (((byte)>>5)&1)
#define UNPKMODE(byte) ((byte)&0x1F)

/**
 * Lookup table to match main video settings and look up sets of
 * Refpix and Refline values
 */
static CONST_DAT struct
{
    /* Values to match */
    UInt8   modeRateSyncVerf;   /* Packed vinMode, pixRate, syncIn, bVerified */
    UInt8   shortVinFmt;
    UInt8   shortVoutFmt;
    /* Values to look up */
    UInt16  refPix;             /* Output values */
    UInt16  refLine;
    UInt16  scRefPix;           /* Scaler values */
    UInt16  scRefLine;
} kRefpixRefline [] =
{
  /*************************************************************/
  /** Rows formatted in "Refpix_Refline.xls" and pasted here  **/
  /** DO NOT DELETE ANY ROWS, to keep all scaler combinations **/
  /*************************************************************/
  /*        mode_____Rate___Sync_Verf  shortVinFmt     shortVoutFmt    refPix  refLine  scRefPix  scRefLine  Test ID  */
    {PKBYTE(iCCIR656,SINGLE,EMB, 1),   TV_480i_60Hz,   TV_480p_60Hz,   0x08b,  0x024,   0x078,    0x017}, /* VID_F_04 */
    {PKBYTE(iCCIR656,SINGLE,EMB, 1),   TV_480i_60Hz,   TV_720p_60Hz,   0x08b,  0x012,   0x078,    0x017}, /* VID_F_04 */
    {PKBYTE(iCCIR656,SINGLE,EMB, 1),   TV_480i_60Hz,   TV_1080i_60Hz,  0x08b,  0x00e,   0x078,    0x017}, /* VID_F_04 */
    {PKBYTE(iCCIR656,SINGLE,EMB, 1),   TV_480p_60Hz,   TV_720p_60Hz,   0x08b,  0x017,   0x078,    0x02c}, /* VID_F_01 */
    {PKBYTE(iCCIR656,SINGLE,EMB, 1),   TV_480p_60Hz,   TV_1080i_60Hz,  0x08b,  0x013,   0x078,    0x02c}, /* VID_F_01 */
    {PKBYTE(iCCIR656,SINGLE,EMB, 1),   TV_576i_50Hz,   TV_576p_50Hz,   0x091,  0x026,   0x085,    0x018}, /* VID_F_09 */
    {PKBYTE(iCCIR656,SINGLE,EMB, 1),   TV_576i_50Hz,   TV_720p_50Hz,   0x091,  0x013,   0x085,    0x018}, /* VID_F_09 */
    {PKBYTE(iCCIR656,SINGLE,EMB, 1),   TV_576i_50Hz,   TV_1080i_50Hz,  0x091,  0x00f,   0x085,    0x018}, /* VID_F_09 */
    {PKBYTE(iCCIR656,SINGLE,EMB, 1),   TV_576p_50Hz,   TV_720p_50Hz,   0x091,  0x019,   0x085,    0x02e}, /* VID_F_06 */
    {PKBYTE(iCCIR656,SINGLE,EMB, 1),   TV_576p_50Hz,   TV_1080i_50Hz,  0x091,  0x014,   0x085,    0x02e}, /* VID_F_06 */
    {PKBYTE(iCCIR656,SINGLE,EXT, 1),   TV_480i_60Hz,   TV_480p_60Hz,   0x014,  0x20d,   0x359,    0x004}, /* VID_F_04 */
    {PKBYTE(iCCIR656,SINGLE,EXT, 1),   TV_480i_60Hz,   TV_720p_60Hz,   0x014,  0x2cb,   0x359,    0x004}, /* VID_F_04 */
    {PKBYTE(iCCIR656,SINGLE,EXT, 1),   TV_480i_60Hz,   TV_1080i_60Hz,  0x014,  0x44c,   0x359,    0x004}, /* VID_F_04 */
    {PKBYTE(iCCIR656,SINGLE,EXT, 1),   TV_480p_60Hz,   TV_720p_60Hz,   0x011,  0x2d3,   0x358,    0x007}, /* VID_F_01 */
    {PKBYTE(iCCIR656,SINGLE,EXT, 1),   TV_480p_60Hz,   TV_1080i_60Hz,  0x011,  0x452,   0x358,    0x007}, /* VID_F_01 */
    {PKBYTE(iCCIR656,SINGLE,EXT, 1),   TV_576i_50Hz,   TV_576p_50Hz,   0x00d,  0x26b,   0x35f,    0x001}, /* VID_F_09 */
    {PKBYTE(iCCIR656,SINGLE,EXT, 1),   TV_576i_50Hz,   TV_720p_50Hz,   0x00d,  0x2cb,   0x35f,    0x001}, /* VID_F_09 */
    {PKBYTE(iCCIR656,SINGLE,EXT, 1),   TV_576i_50Hz,   TV_1080i_50Hz,  0x00d,  0x44b,   0x35f,    0x001}, /* VID_F_09 */
    {PKBYTE(iCCIR656,SINGLE,EXT, 1),   TV_576p_50Hz,   TV_720p_50Hz,   0x00d,  0x2d1,   0x35f,    0x001}, /* VID_F_06 */
    {PKBYTE(iCCIR656,SINGLE,EXT, 1),   TV_576p_50Hz,   TV_1080i_50Hz,  0x00d,  0x451,   0x35f,    0x001}, /* VID_F_06 */
    {PKBYTE(iCCIR656,DOUBLE,EMB, 1),   TV_480i_60Hz,   TV_480p_60Hz,   0x08b,  0x024,   0x078,    0x017}, /* VID_F_04 */
    {PKBYTE(iCCIR656,DOUBLE,EMB, 1),   TV_480i_60Hz,   TV_720p_60Hz,   0x08b,  0x012,   0x078,    0x017}, /* VID_F_04 */
    {PKBYTE(iCCIR656,DOUBLE,EMB, 1),   TV_480i_60Hz,   TV_1080i_60Hz,  0x08b,  0x00e,   0x078,    0x017}, /* VID_F_04 */
    {PKBYTE(iCCIR656,DOUBLE,EMB, 1),   TV_480p_60Hz,   TV_720p_60Hz,   0x08b,  0x017,   0x078,    0x02c}, /* VID_F_01 */
    {PKBYTE(iCCIR656,DOUBLE,EMB, 1),   TV_480p_60Hz,   TV_1080i_60Hz,  0x08b,  0x013,   0x078,    0x02c}, /* VID_F_01 */
    {PKBYTE(iCCIR656,DOUBLE,EMB, 1),   TV_576i_50Hz,   TV_576p_50Hz,   0x091,  0x026,   0x085,    0x018}, /* VID_F_09 */
    {PKBYTE(iCCIR656,DOUBLE,EMB, 1),   TV_576i_50Hz,   TV_720p_50Hz,   0x091,  0x013,   0x085,    0x018}, /* VID_F_09 */
    {PKBYTE(iCCIR656,DOUBLE,EMB, 1),   TV_576i_50Hz,   TV_1080i_50Hz,  0x091,  0x00f,   0x085,    0x018}, /* VID_F_09 */
    {PKBYTE(iCCIR656,DOUBLE,EMB, 1),   TV_576p_50Hz,   TV_720p_50Hz,   0x091,  0x019,   0x085,    0x02e}, /* VID_F_06 */
    {PKBYTE(iCCIR656,DOUBLE,EMB, 1),   TV_576p_50Hz,   TV_1080i_50Hz,  0x091,  0x014,   0x085,    0x02e}, /* VID_F_06 */
    {PKBYTE(iCCIR656,DOUBLE,EXT, 1),   TV_480i_60Hz,   TV_480p_60Hz,   0x014,  0x20d,   0x359,    0x004}, /* VID_F_04 */
    {PKBYTE(iCCIR656,DOUBLE,EXT, 1),   TV_480i_60Hz,   TV_720p_60Hz,   0x014,  0x2cb,   0x359,    0x004}, /* VID_F_04 */
    {PKBYTE(iCCIR656,DOUBLE,EXT, 1),   TV_480i_60Hz,   TV_1080i_60Hz,  0x014,  0x44c,   0x359,    0x004}, /* VID_F_04 */
    {PKBYTE(iCCIR656,DOUBLE,EXT, 1),   TV_480p_60Hz,   TV_720p_60Hz,   0x011,  0x2d3,   0x358,    0x007}, /* VID_F_01 */
    {PKBYTE(iCCIR656,DOUBLE,EXT, 1),   TV_480p_60Hz,   TV_1080i_60Hz,  0x011,  0x452,   0x358,    0x007}, /* VID_F_01 */
    {PKBYTE(iCCIR656,DOUBLE,EXT, 1),   TV_576i_50Hz,   TV_576p_50Hz,   0x00d,  0x26b,   0x35f,    0x001}, /* VID_F_09 */
    {PKBYTE(iCCIR656,DOUBLE,EXT, 1),   TV_576i_50Hz,   TV_720p_50Hz,   0x00d,  0x2cb,   0x35f,    0x001}, /* VID_F_09 */
    {PKBYTE(iCCIR656,DOUBLE,EXT, 1),   TV_576i_50Hz,   TV_1080i_50Hz,  0x00d,  0x44b,   0x35f,    0x001}, /* VID_F_09 */
    {PKBYTE(iCCIR656,DOUBLE,EXT, 1),   TV_576p_50Hz,   TV_720p_50Hz,   0x00d,  0x2d1,   0x35f,    0x001}, /* VID_F_06 */
    {PKBYTE(iCCIR656,DOUBLE,EXT, 1),   TV_576p_50Hz,   TV_1080i_50Hz,  0x00d,  0x451,   0x35f,    0x001}, /* VID_F_06 */
    {PKBYTE(iYUV422, SINGLE,EMB, 1),   TV_480i_60Hz,   TV_480p_60Hz,   0x08d,  0x028,   0x078,    0x017}, /* VID_F_04 */
    {PKBYTE(iYUV422, SINGLE,EMB, 1),   TV_480i_60Hz,   TV_720p_60Hz,   0x08d,  0x014,   0x078,    0x017}, /* VID_F_04 */
    {PKBYTE(iYUV422, SINGLE,EMB, 1),   TV_480i_60Hz,   TV_1080i_60Hz,  0x08d,  0x010,   0x078,    0x017}, /* VID_F_04 */
    {PKBYTE(iYUV422, SINGLE,EMB, 1),   TV_480p_60Hz,   TV_720p_60Hz,   0x08d,  0x017,   0x078,    0x02c}, /* VID_F_01 */
    {PKBYTE(iYUV422, SINGLE,EMB, 1),   TV_480p_60Hz,   TV_1080i_60Hz,  0x08d,  0x014,   0x078,    0x02c}, /* VID_F_01 */
    {PKBYTE(iYUV422, SINGLE,EMB, 1),   TV_576i_50Hz,   TV_576p_50Hz,   0x093,  0x02a,   0x085,    0x018}, /* VID_F_09 */
    {PKBYTE(iYUV422, SINGLE,EMB, 1),   TV_576i_50Hz,   TV_720p_50Hz,   0x093,  0x013,   0x085,    0x018}, /* VID_F_09 */
    {PKBYTE(iYUV422, SINGLE,EMB, 1),   TV_576i_50Hz,   TV_1080i_50Hz,  0x093,  0x00e,   0x085,    0x018}, /* VID_F_09 */
    {PKBYTE(iYUV422, SINGLE,EMB, 1),   TV_576p_50Hz,   TV_720p_50Hz,   0x093,  0x019,   0x085,    0x02e}, /* VID_F_06 */
    {PKBYTE(iYUV422, SINGLE,EMB, 1),   TV_576p_50Hz,   TV_1080i_50Hz,  0x093,  0x014,   0x085,    0x02e}, /* VID_F_06 */
    {PKBYTE(iYUV422, SINGLE,EXT, 1),   TV_480i_60Hz,   TV_480p_60Hz,   0x016,  0x20d,   0x359,    0x004}, /* VID_F_04 */
    {PKBYTE(iYUV422, SINGLE,EXT, 1),   TV_480i_60Hz,   TV_720p_60Hz,   0x016,  0x2cb,   0x359,    0x004}, /* VID_F_04 */
    {PKBYTE(iYUV422, SINGLE,EXT, 1),   TV_480i_60Hz,   TV_1080i_60Hz,  0x016,  0x44c,   0x359,    0x004}, /* VID_F_04 */
    {PKBYTE(iYUV422, SINGLE,EXT, 1),   TV_480p_60Hz,   TV_720p_60Hz,   0x013,  0x2d3,   0x358,    0x007}, /* VID_F_01 */
    {PKBYTE(iYUV422, SINGLE,EXT, 1),   TV_480p_60Hz,   TV_1080i_60Hz,  0x013,  0x452,   0x358,    0x007}, /* VID_F_01 */
    {PKBYTE(iYUV422, SINGLE,EXT, 1),   TV_576i_50Hz,   TV_576p_50Hz,   0x00f,  0x26b,   0x35f,    0x001}, /* VID_F_09 */
    {PKBYTE(iYUV422, SINGLE,EXT, 1),   TV_576i_50Hz,   TV_720p_50Hz,   0x00f,  0x2cb,   0x35f,    0x001}, /* VID_F_09 */
    {PKBYTE(iYUV422, SINGLE,EXT, 1),   TV_576i_50Hz,   TV_1080i_50Hz,  0x00f,  0x44b,   0x35f,    0x001}, /* VID_F_09 */
    {PKBYTE(iYUV422, SINGLE,EXT, 1),   TV_576p_50Hz,   TV_720p_50Hz,   0x00f,  0x2d1,   0x35f,    0x001}, /* VID_F_06 */
    {PKBYTE(iYUV422, SINGLE,EXT, 1),   TV_576p_50Hz,   TV_1080i_50Hz,  0x00f,  0x451,   0x35f,    0x001}, /* VID_F_06 */
    {PKBYTE(iYUV422, DOUBLE,EMB, 0),   TV_480i_60Hz,   TV_480p_60Hz,   0x000,  0x000,   0x000,    0x000}, /* VID_F_04 */
    {PKBYTE(iYUV422, DOUBLE,EMB, 0),   TV_480i_60Hz,   TV_720p_60Hz,   0x000,  0x000,   0x000,    0x000}, /* VID_F_04 */
    {PKBYTE(iYUV422, DOUBLE,EMB, 0),   TV_480i_60Hz,   TV_1080i_60Hz,  0x000,  0x000,   0x000,    0x000}, /* VID_F_04 */
    {PKBYTE(iYUV422, DOUBLE,EMB, 0),   TV_480p_60Hz,   TV_720p_60Hz,   0x000,  0x000,   0x000,    0x000}, /* VID_F_01 */
    {PKBYTE(iYUV422, DOUBLE,EMB, 0),   TV_480p_60Hz,   TV_1080i_60Hz,  0x000,  0x000,   0x000,    0x000}, /* VID_F_01 */
    {PKBYTE(iYUV422, DOUBLE,EMB, 0),   TV_576i_50Hz,   TV_576p_50Hz,   0x000,  0x000,   0x000,    0x000}, /* VID_F_09 */
    {PKBYTE(iYUV422, DOUBLE,EMB, 0),   TV_576i_50Hz,   TV_720p_50Hz,   0x000,  0x000,   0x000,    0x000}, /* VID_F_09 */
    {PKBYTE(iYUV422, DOUBLE,EMB, 0),   TV_576i_50Hz,   TV_1080i_50Hz,  0x000,  0x000,   0x000,    0x000}, /* VID_F_09 */
    {PKBYTE(iYUV422, DOUBLE,EMB, 0),   TV_576p_50Hz,   TV_720p_50Hz,   0x000,  0x000,   0x000,    0x000}, /* VID_F_06 */
    {PKBYTE(iYUV422, DOUBLE,EMB, 0),   TV_576p_50Hz,   TV_1080i_50Hz,  0x000,  0x000,   0x000,    0x000}, /* VID_F_06 */
    {PKBYTE(iYUV422, DOUBLE,EXT, 0),   TV_480i_60Hz,   TV_480p_60Hz,   0x000,  0x000,   0x000,    0x000}, /* VID_F_04 */
    {PKBYTE(iYUV422, DOUBLE,EXT, 0),   TV_480i_60Hz,   TV_720p_60Hz,   0x000,  0x000,   0x000,    0x000}, /* VID_F_04 */
    {PKBYTE(iYUV422, DOUBLE,EXT, 0),   TV_480i_60Hz,   TV_1080i_60Hz,  0x000,  0x000,   0x000,    0x000}, /* VID_F_04 */
    {PKBYTE(iYUV422, DOUBLE,EXT, 0),   TV_480p_60Hz,   TV_720p_60Hz,   0x000,  0x000,   0x000,    0x000}, /* VID_F_01 */
    {PKBYTE(iYUV422, DOUBLE,EXT, 0),   TV_480p_60Hz,   TV_1080i_60Hz,  0x000,  0x000,   0x000,    0x000}, /* VID_F_01 */
    {PKBYTE(iYUV422, DOUBLE,EXT, 0),   TV_576i_50Hz,   TV_576p_50Hz,   0x000,  0x000,   0x000,    0x000}, /* VID_F_09 */
    {PKBYTE(iYUV422, DOUBLE,EXT, 0),   TV_576i_50Hz,   TV_720p_50Hz,   0x000,  0x000,   0x000,    0x000}, /* VID_F_09 */
    {PKBYTE(iYUV422, DOUBLE,EXT, 0),   TV_576i_50Hz,   TV_1080i_50Hz,  0x000,  0x000,   0x000,    0x000}, /* VID_F_09 */
    {PKBYTE(iYUV422, DOUBLE,EXT, 0),   TV_576p_50Hz,   TV_720p_50Hz,   0x000,  0x000,   0x000,    0x000}, /* VID_F_06 */
    {PKBYTE(iYUV422, DOUBLE,EXT, 0),   TV_576p_50Hz,   TV_1080i_50Hz,  0x000,  0x000,   0x000,    0x000}, /* VID_F_06 */
    {PKBYTE(iYUV444, SINGLE,EMB, 0),   TV_480i_60Hz,   TV_480p_60Hz,   0x000,  0x000,   0x000,    0x000}, /* VID_F_04 */
    {PKBYTE(iYUV444, SINGLE,EMB, 0),   TV_480i_60Hz,   TV_720p_60Hz,   0x000,  0x000,   0x000,    0x000}, /* VID_F_04 */
    {PKBYTE(iYUV444, SINGLE,EMB, 0),   TV_480i_60Hz,   TV_1080i_60Hz,  0x000,  0x000,   0x000,    0x000}, /* VID_F_04 */
    {PKBYTE(iYUV444, SINGLE,EMB, 0),   TV_480p_60Hz,   TV_720p_60Hz,   0x000,  0x000,   0x000,    0x000}, /* VID_F_01 */
    {PKBYTE(iYUV444, SINGLE,EMB, 0),   TV_480p_60Hz,   TV_1080i_60Hz,  0x000,  0x000,   0x000,    0x000}, /* VID_F_01 */
    {PKBYTE(iYUV444, SINGLE,EMB, 0),   TV_576i_50Hz,   TV_576p_50Hz,   0x000,  0x000,   0x000,    0x000}, /* VID_F_09 */
    {PKBYTE(iYUV444, SINGLE,EMB, 0),   TV_576i_50Hz,   TV_720p_50Hz,   0x000,  0x000,   0x000,    0x000}, /* VID_F_09 */
    {PKBYTE(iYUV444, SINGLE,EMB, 0),   TV_576i_50Hz,   TV_1080i_50Hz,  0x000,  0x000,   0x000,    0x000}, /* VID_F_09 */
    {PKBYTE(iYUV444, SINGLE,EMB, 0),   TV_576p_50Hz,   TV_720p_50Hz,   0x000,  0x000,   0x000,    0x000}, /* VID_F_06 */
    {PKBYTE(iYUV444, SINGLE,EMB, 0),   TV_576p_50Hz,   TV_1080i_50Hz,  0x000,  0x000,   0x000,    0x000}, /* VID_F_06 */
    {PKBYTE(iYUV444, SINGLE,EXT, 0),   TV_480i_60Hz,   TV_480p_60Hz,   0x000,  0x000,   0x000,    0x000}, /* VID_F_04 */
    {PKBYTE(iYUV444, SINGLE,EXT, 0),   TV_480i_60Hz,   TV_720p_60Hz,   0x000,  0x000,   0x000,    0x000}, /* VID_F_04 */
    {PKBYTE(iYUV444, SINGLE,EXT, 0),   TV_480i_60Hz,   TV_1080i_60Hz,  0x000,  0x000,   0x000,    0x000}, /* VID_F_04 */
    {PKBYTE(iYUV444, SINGLE,EXT, 0),   TV_480p_60Hz,   TV_720p_60Hz,   0x000,  0x000,   0x000,    0x000}, /* VID_F_01 */
    {PKBYTE(iYUV444, SINGLE,EXT, 0),   TV_480p_60Hz,   TV_1080i_60Hz,  0x000,  0x000,   0x000,    0x000}, /* VID_F_01 */
    {PKBYTE(iYUV444, SINGLE,EXT, 0),   TV_576i_50Hz,   TV_576p_50Hz,   0x000,  0x000,   0x000,    0x000}, /* VID_F_09 */
    {PKBYTE(iYUV444, SINGLE,EXT, 0),   TV_576i_50Hz,   TV_720p_50Hz,   0x000,  0x000,   0x000,    0x000}, /* VID_F_09 */
    {PKBYTE(iYUV444, SINGLE,EXT, 0),   TV_576i_50Hz,   TV_1080i_50Hz,  0x000,  0x000,   0x000,    0x000}, /* VID_F_09 */
    {PKBYTE(iYUV444, SINGLE,EXT, 0),   TV_576p_50Hz,   TV_720p_50Hz,   0x000,  0x000,   0x000,    0x000}, /* VID_F_06 */
    {PKBYTE(iYUV444, SINGLE,EXT, 0),   TV_576p_50Hz,   TV_1080i_50Hz,  0x000,  0x000,   0x000,    0x000}, /* VID_F_06 */
    {PKBYTE(iYUV444, DOUBLE,EMB, 0),   TV_480i_60Hz,   TV_480p_60Hz,   0x000,  0x000,   0x000,    0x000}, /* VID_F_04 */
    {PKBYTE(iYUV444, DOUBLE,EMB, 0),   TV_480i_60Hz,   TV_720p_60Hz,   0x000,  0x000,   0x000,    0x000}, /* VID_F_04 */
    {PKBYTE(iYUV444, DOUBLE,EMB, 0),   TV_480i_60Hz,   TV_1080i_60Hz,  0x000,  0x000,   0x000,    0x000}, /* VID_F_04 */
    {PKBYTE(iYUV444, DOUBLE,EMB, 0),   TV_480p_60Hz,   TV_720p_60Hz,   0x000,  0x000,   0x000,    0x000}, /* VID_F_01 */
    {PKBYTE(iYUV444, DOUBLE,EMB, 0),   TV_480p_60Hz,   TV_1080i_60Hz,  0x000,  0x000,   0x000,    0x000}, /* VID_F_01 */
    {PKBYTE(iYUV444, DOUBLE,EMB, 0),   TV_576i_50Hz,   TV_576p_50Hz,   0x000,  0x000,   0x000,    0x000}, /* VID_F_09 */
    {PKBYTE(iYUV444, DOUBLE,EMB, 0),   TV_576i_50Hz,   TV_720p_50Hz,   0x000,  0x000,   0x000,    0x000}, /* VID_F_09 */
    {PKBYTE(iYUV444, DOUBLE,EMB, 0),   TV_576i_50Hz,   TV_1080i_50Hz,  0x000,  0x000,   0x000,    0x000}, /* VID_F_09 */
    {PKBYTE(iYUV444, DOUBLE,EMB, 0),   TV_576p_50Hz,   TV_720p_50Hz,   0x000,  0x000,   0x000,    0x000}, /* VID_F_06 */
    {PKBYTE(iYUV444, DOUBLE,EMB, 0),   TV_576p_50Hz,   TV_1080i_50Hz,  0x000,  0x000,   0x000,    0x000}, /* VID_F_06 */
    {PKBYTE(iYUV444, DOUBLE,EXT, 0),   TV_480i_60Hz,   TV_480p_60Hz,   0x000,  0x000,   0x000,    0x000}, /* VID_F_04 */
    {PKBYTE(iYUV444, DOUBLE,EXT, 0),   TV_480i_60Hz,   TV_720p_60Hz,   0x000,  0x000,   0x000,    0x000}, /* VID_F_04 */
    {PKBYTE(iYUV444, DOUBLE,EXT, 0),   TV_480i_60Hz,   TV_1080i_60Hz,  0x000,  0x000,   0x000,    0x000}, /* VID_F_04 */
    {PKBYTE(iYUV444, DOUBLE,EXT, 0),   TV_480p_60Hz,   TV_720p_60Hz,   0x000,  0x000,   0x000,    0x000}, /* VID_F_01 */
    {PKBYTE(iYUV444, DOUBLE,EXT, 0),   TV_480p_60Hz,   TV_1080i_60Hz,  0x000,  0x000,   0x000,    0x000}, /* VID_F_01 */
    {PKBYTE(iYUV444, DOUBLE,EXT, 0),   TV_576i_50Hz,   TV_576p_50Hz,   0x000,  0x000,   0x000,    0x000}, /* VID_F_09 */
    {PKBYTE(iYUV444, DOUBLE,EXT, 0),   TV_576i_50Hz,   TV_720p_50Hz,   0x000,  0x000,   0x000,    0x000}, /* VID_F_09 */
    {PKBYTE(iYUV444, DOUBLE,EXT, 0),   TV_576i_50Hz,   TV_1080i_50Hz,  0x000,  0x000,   0x000,    0x000}, /* VID_F_09 */
    {PKBYTE(iYUV444, DOUBLE,EXT, 0),   TV_576p_50Hz,   TV_720p_50Hz,   0x000,  0x000,   0x000,    0x000}, /* VID_F_06 */
    {PKBYTE(iYUV444, DOUBLE,EXT, 0),   TV_576p_50Hz,   TV_1080i_50Hz,  0x000,  0x000,   0x000,    0x000}, /* VID_F_06 */
    {PKBYTE(iRGB444, SINGLE,EMB, 0),   TV_480i_60Hz,   TV_480p_60Hz,   0x000,  0x000,   0x000,    0x000}, /* VID_F_04 */
    {PKBYTE(iRGB444, SINGLE,EMB, 0),   TV_480i_60Hz,   TV_720p_60Hz,   0x000,  0x000,   0x000,    0x000}, /* VID_F_04 */
    {PKBYTE(iRGB444, SINGLE,EMB, 0),   TV_480i_60Hz,   TV_1080i_60Hz,  0x000,  0x000,   0x000,    0x000}, /* VID_F_04 */
    {PKBYTE(iRGB444, SINGLE,EMB, 0),   TV_480p_60Hz,   TV_720p_60Hz,   0x000,  0x000,   0x000,    0x000}, /* VID_F_01 */
    {PKBYTE(iRGB444, SINGLE,EMB, 0),   TV_480p_60Hz,   TV_1080i_60Hz,  0x000,  0x000,   0x000,    0x000}, /* VID_F_01 */
    {PKBYTE(iRGB444, SINGLE,EMB, 0),   TV_576i_50Hz,   TV_576p_50Hz,   0x000,  0x000,   0x000,    0x000}, /* VID_F_09 */
    {PKBYTE(iRGB444, SINGLE,EMB, 0),   TV_576i_50Hz,   TV_720p_50Hz,   0x000,  0x000,   0x000,    0x000}, /* VID_F_09 */
    {PKBYTE(iRGB444, SINGLE,EMB, 0),   TV_576i_50Hz,   TV_1080i_50Hz,  0x000,  0x000,   0x000,    0x000}, /* VID_F_09 */
    {PKBYTE(iRGB444, SINGLE,EMB, 0),   TV_576p_50Hz,   TV_720p_50Hz,   0x000,  0x000,   0x000,    0x000}, /* VID_F_06 */
    {PKBYTE(iRGB444, SINGLE,EMB, 0),   TV_576p_50Hz,   TV_1080i_50Hz,  0x000,  0x000,   0x000,    0x000}, /* VID_F_06 */
    {PKBYTE(iRGB444, SINGLE,EXT, 0),   TV_480i_60Hz,   TV_480p_60Hz,   0x000,  0x000,   0x000,    0x000}, /* VID_F_04 */
    {PKBYTE(iRGB444, SINGLE,EXT, 0),   TV_480i_60Hz,   TV_720p_60Hz,   0x000,  0x000,   0x000,    0x000}, /* VID_F_04 */
    {PKBYTE(iRGB444, SINGLE,EXT, 0),   TV_480i_60Hz,   TV_1080i_60Hz,  0x000,  0x000,   0x000,    0x000}, /* VID_F_04 */
    {PKBYTE(iRGB444, SINGLE,EXT, 0),   TV_480p_60Hz,   TV_720p_60Hz,   0x000,  0x000,   0x000,    0x000}, /* VID_F_01 */
    {PKBYTE(iRGB444, SINGLE,EXT, 0),   TV_480p_60Hz,   TV_1080i_60Hz,  0x000,  0x000,   0x000,    0x000}, /* VID_F_01 */
    {PKBYTE(iRGB444, SINGLE,EXT, 0),   TV_480p_60Hz,   TV_VGA_60Hz,    0x000,  0x000,   0x000,    0x000}, /* VID_F_01 */
    {PKBYTE(iRGB444, SINGLE,EXT, 0),   TV_576i_50Hz,   TV_576p_50Hz,   0x000,  0x000,   0x000,    0x000}, /* VID_F_09 */
    {PKBYTE(iRGB444, SINGLE,EXT, 0),   TV_576i_50Hz,   TV_720p_50Hz,   0x000,  0x000,   0x000,    0x000}, /* VID_F_09 */
    {PKBYTE(iRGB444, SINGLE,EXT, 0),   TV_576i_50Hz,   TV_1080i_50Hz,  0x000,  0x000,   0x000,    0x000}, /* VID_F_09 */
    {PKBYTE(iRGB444, SINGLE,EXT, 0),   TV_576p_50Hz,   TV_720p_50Hz,   0x000,  0x000,   0x000,    0x000}, /* VID_F_06 */
    {PKBYTE(iRGB444, SINGLE,EXT, 0),   TV_576p_50Hz,   TV_1080i_50Hz,  0x000,  0x000,   0x000,    0x000}, /* VID_F_06 */
    {PKBYTE(iRGB444, DOUBLE,EMB, 0),   TV_480i_60Hz,   TV_480p_60Hz,   0x000,  0x000,   0x000,    0x000}, /* VID_F_04 */
    {PKBYTE(iRGB444, DOUBLE,EMB, 0),   TV_480i_60Hz,   TV_720p_60Hz,   0x000,  0x000,   0x000,    0x000}, /* VID_F_04 */
    {PKBYTE(iRGB444, DOUBLE,EMB, 0),   TV_480i_60Hz,   TV_1080i_60Hz,  0x000,  0x000,   0x000,    0x000}, /* VID_F_04 */
    {PKBYTE(iRGB444, DOUBLE,EMB, 0),   TV_480p_60Hz,   TV_720p_60Hz,   0x000,  0x000,   0x000,    0x000}, /* VID_F_01 */
    {PKBYTE(iRGB444, DOUBLE,EMB, 0),   TV_480p_60Hz,   TV_1080i_60Hz,  0x000,  0x000,   0x000,    0x000}, /* VID_F_01 */
    {PKBYTE(iRGB444, DOUBLE,EMB, 0),   TV_576i_50Hz,   TV_576p_50Hz,   0x000,  0x000,   0x000,    0x000}, /* VID_F_09 */
    {PKBYTE(iRGB444, DOUBLE,EMB, 0),   TV_576i_50Hz,   TV_720p_50Hz,   0x000,  0x000,   0x000,    0x000}, /* VID_F_09 */
    {PKBYTE(iRGB444, DOUBLE,EMB, 0),   TV_576i_50Hz,   TV_1080i_50Hz,  0x000,  0x000,   0x000,    0x000}, /* VID_F_09 */
    {PKBYTE(iRGB444, DOUBLE,EMB, 0),   TV_576p_50Hz,   TV_720p_50Hz,   0x000,  0x000,   0x000,    0x000}, /* VID_F_06 */
    {PKBYTE(iRGB444, DOUBLE,EMB, 0),   TV_576p_50Hz,   TV_1080i_50Hz,  0x000,  0x000,   0x000,    0x000}, /* VID_F_06 */
    {PKBYTE(iRGB444, DOUBLE,EXT, 0),   TV_480i_60Hz,   TV_480p_60Hz,   0x000,  0x000,   0x000,    0x000}, /* VID_F_04 */
    {PKBYTE(iRGB444, DOUBLE,EXT, 0),   TV_480i_60Hz,   TV_720p_60Hz,   0x000,  0x000,   0x000,    0x000}, /* VID_F_04 */
    {PKBYTE(iRGB444, DOUBLE,EXT, 0),   TV_480i_60Hz,   TV_1080i_60Hz,  0x000,  0x000,   0x000,    0x000}, /* VID_F_04 */
    {PKBYTE(iRGB444, DOUBLE,EXT, 0),   TV_480p_60Hz,   TV_720p_60Hz,   0x000,  0x000,   0x000,    0x000}, /* VID_F_01 */
    {PKBYTE(iRGB444, DOUBLE,EXT, 0),   TV_480p_60Hz,   TV_1080i_60Hz,  0x000,  0x000,   0x000,    0x000}, /* VID_F_01 */
    {PKBYTE(iRGB444, DOUBLE,EXT, 0),   TV_576i_50Hz,   TV_576p_50Hz,   0x000,  0x000,   0x000,    0x000}, /* VID_F_09 */
    {PKBYTE(iRGB444, DOUBLE,EXT, 0),   TV_576i_50Hz,   TV_720p_50Hz,   0x000,  0x000,   0x000,    0x000}, /* VID_F_09 */
    {PKBYTE(iRGB444, DOUBLE,EXT, 0),   TV_576i_50Hz,   TV_1080i_50Hz,  0x000,  0x000,   0x000,    0x000}, /* VID_F_09 */
    {PKBYTE(iRGB444, DOUBLE,EXT, 0),   TV_576p_50Hz,   TV_720p_50Hz,   0x000,  0x000,   0x000,    0x000}, /* VID_F_06 */
    {PKBYTE(iRGB444, DOUBLE,EXT, 0),   TV_576p_50Hz,   TV_1080i_50Hz,  0x000,  0x000,   0x000,    0x000}, /* VID_F_06 */
    {PKBYTE(iINVALID,DOUBLE,EMB, 0),   TV_INVALID,     TV_INVALID,     0x000,  0x000,   0x000,    0x000}  /* EndTable */
};

/*============================================================================*/
/*                   STATIC VARIABLE DECLARATIONS                             */
/* To conserve RAM, these variables do NOT support multiple device instances  */
/** \todo Users can turn them into arrays with dimension HDMITX_UNITS_MAX     */
/*============================================================================*/

/* EDID short video descriptor list and total available */
static UInt8   EdidVFmts[HDMI_TX_SVD_MAX_CNT];
static UInt    EdidVFmtsAvail;

#ifndef NO_HDCP
/* KSV list and number of devices read from B sink during SHA-1 interrupt */
static UInt8   HdcpKsvList[HDMITX_KSV_BYTES_PER_DEVICE *
                           HDMITX_KSV_LIST_MAX_DEVICES];
static UInt8   HdcpKsvDevices;
static tmbslHdmiTxHdcpTxMode_t  txMode;

/* BKSV read from B sink */
static UInt8   HdcpBksv[HDMITX_KSV_BYTES_PER_DEVICE];
#endif /* NO_HDCP */

static UInt8 AudioChannelAllocation = 0;

static UInt8 * ptrHDCPRevocationList = Null;
static UInt32  HDCPRevocationListSize = 0;

/*============================================================================*/
/*                   STATIC FUNCTION DECLARATIONS                             */
/*============================================================================*/

static tmbslHdmiTxVfreq_t
fmtToFreq
(
    tmbslHdmiTxVidFmt_t fmt
);

static Bool
getReflineRefpix
(
    tmbslHdmiTxVidFmt_t     vinFmt,    
    tmbslHdmiTxVinMode_t    vinMode,   
    tmbslHdmiTxVidFmt_t     voutFmt,   
    UInt8                   syncIn,
    tmbslHdmiTxPixRate_t    pixRate,
    UInt16                  *pRefPix,  
    UInt16                  *pRefLine, 
    UInt16                  *pScRefPix,
    UInt16                  *pScRefLine,
    Bool                    *pbVerified
);

static tmErrorCode_t
setVideoInfoframe
(
    tmUnitSelect_t          txUnit,
    tmbslHdmiTxVidFmt_t     voutFmt,
    tmbslHdmiTxVoutMode_t   voutMode
);

static void
lAppMemset
(
    void *pTable1,
    const UInt8 value,
    UInt Size
);


static void lAppCheckHdcpBksv
(
UInt8 * pHdcpBksvTested,
Bool  * pbBksvSecure,
Bool    bBigEndian
);





/*============================================================================*/
/*                   PUBLIC FUNCTION DEFINITIONS                              */
/*============================================================================*/


/*============================================================================*/
/* Initialise demo application */
void
appHdmiTx_init(void)
{
    EdidVFmtsAvail = 0;
    AudioChannelAllocation = 0;

#ifndef NO_HDCP
    HdcpKsvDevices = 0;
    lAppMemset(HdcpBksv, 0, sizeof(HdcpBksv));
    ptrHDCPRevocationList = Null;
    HDCPRevocationListSize = 0;

#endif /* NO_HDCP */
}

/*============================================================================*/
/* Set colourbar test pattern on with RGB infoframe */
tmErrorCode_t
appHdmiTx_test_pattern_on
(
    tmUnitSelect_t              txUnit,
    tmbslHdmiTxVidFmt_t         voutFmt,
    tmbslHdmiTxVoutMode_t       voutMode,
    tmbslHdmiTxTestPattern_t    pattern
)
{
    tmErrorCode_t           err;
    tmbslHdmiTxVoutMode_t   infoFrameMode;

    err = tmbslHdmiTxTestSetPattern(txUnit, pattern);
    RETIF(err != TM_OK, err + 0x100);

    /* set infoframe in RGB when pattern is colorbar */
    if (pattern>HDMITX_PATTERN_CBAR8)
    {
        infoFrameMode = voutMode;
    }
    else
    {
        infoFrameMode = HDMITX_VOUTMODE_RGB444;
    }
    /* update videoinfoframe */
    err = setVideoInfoframe(
                            txUnit,
                            voutFmt,
                            infoFrameMode
                            );
    return TM_OK;
}

/*============================================================================*/
/* Set colourbar test pattern off with previous infoframe */
tmErrorCode_t
appHdmiTx_test_pattern_off
(
    tmUnitSelect_t          txUnit,
    tmbslHdmiTxVidFmt_t     voutFmt,
    tmbslHdmiTxVoutMode_t   voutMode
)
{
    tmErrorCode_t err;

    err = tmbslHdmiTxTestSetPattern(txUnit, HDMITX_PATTERN_OFF);
    RETIF(err != TM_OK, err + 0x100);

    /* Restore vif */
    err = setVideoInfoframe(txUnit, voutFmt, voutMode);
    RETIF(err != TM_OK, err + 0x200);

    return TM_OK;
}

/*============================================================================*/
/* Set input and output formats, modes, sync source, sink type and audio rate */
tmErrorCode_t
appHdmiTx_setInputOutput
(
    tmUnitSelect_t          txUnit,     /* Defaults: */
    tmbslHdmiTxVidFmt_t     vinFmt,     /* HDMITX_VFMT_02_720x480p_60Hz */
    tmbslHdmiTxVinMode_t    vinMode,    /* HDMITX_VINMODE_YUV422 */
    tmbslHdmiTxVidFmt_t     voutFmt,    /* HDMITX_VFMT_04_1280x720p_60Hz etc */
    tmbslHdmiTxVoutMode_t   voutMode,   /* HDMITX_VOUTMODE_YUV422 */
    tmbslHdmiTxSyncSource_t syncSource, /* HDMITX_SYNCSRC_EMBEDDED */
    tmbslHdmiTxSinkType_t   sinkType,   /* HDMITX_SINK_HDMI */
    tmbslHdmiTxaFmt_t       audioFmt,   /* HDMITX_AFMT_SPDIF */
    tmbslHdmiTxafs_t        audioRate,  /* HDMITX_AFS_48K */
    UInt8                   i2sQualifier,/* 0 */
    tmbslHdmiTxPixRate_t    pixRate,    /* HDMITX_PIXRATE_SINGLE */
    tmbslHdmiTxPixEdge_t    pixelEdge,  /* Pixel sampling edge */
    Bool                    *pbVerified /* Returns True if requested combination
                                         * has been verified during testing */
)
{
    tmErrorCode_t           err;        /* Error code from APIs */
    UInt8                   swapTable[HDMITX_VIN_PORT_MAP_TABLE_LEN];
    UInt8                   mirrorTable[HDMITX_VIN_PORT_MAP_TABLE_LEN];
    UInt16                  uRefPix;    /* REFPIX for output */
    UInt16                  uRefLine;   /* REFLINE for output */
    UInt16                  uScRefPix;  /* REFPIX for scaler */
    UInt16                  uScRefLine; /* REFLINE for scaler */
    tmbslHdmiTxVsOnce_t     once;       /* Line/pixel counters sync */
    tmbslHdmiTxScaMode_t    scalerMode; /* Current scaler mode */
    tmbslHdmiTxBlnkSrc_t    blankit;    /* Blanking */
    tmbslHdmiTxPixTogl_t    toggle;     /* Toggling */
    tmbslHdmiTxVoutDbits_t  pathBits;   /* Data path bit width */
    tmbslHdmiTxPixSubpkt_t  spSync;     /* Subpacket sync */
    UInt8                   pixRepeat;  /* Pixel repetition */
    tmbslHdmiTxVsMeth_t     syncMethod; /* Sync method */
    tmbslHdmiTxTopSel_t     topSel;     /* Adjustment for interlaced output */
    UInt8                   syncIn;     /* Embedded or external */
    tmbslHdmiTxPixRate_t    pixRateSingleDouble;    /* HDMITX_PIXRATE_SINGLE */
#ifdef TMFL_TDA9981_SUPPORT
    UInt8                   enaVideoPortTable[HDMITX_ENABLE_VP_TABLE_LEN];
    UInt8                   gndVideoPortTable[HDMITX_GROUND_VP_TABLE_LEN];
    UInt8                   enaAudioPortCfg;
    UInt8                   gndAudioPortCfg;
#endif /* TMFL_TDA9981_SUPPORT */

    /* Script: api_output_frame */
    /* CHANGE FROM SCRIPT: CR13: Use tmbslHdmiTxTmdsSetOutputs instead */
    err = tmbslHdmiTxTmdsSetOutputs(txUnit, HDMITX_TMDSOUT_FORCED0);
    RETIF(err != TM_OK, err + 0x100);

#ifdef TMFL_TDA9981_SUPPORT
    /* PR456 Modification - use phase2 : 4 - Phase3 : 8 */
    err =tmbslHdmiTxTmdsSetSerializer(txUnit,4,8);
    RETIF(err != TM_OK, err + 0x200);
#else /* TMFL_TDA9981_SUPPORT */
    /* PR46 Modification - use phase2 : 8 - Phase3 : 4 */
    err =tmbslHdmiTxTmdsSetSerializer(txUnit,8,4);
    RETIF(err != TM_OK, err + 0x200);
#endif /* TMFL_TDA9981_SUPPORT */

    if (sinkType == HDMITX_SINK_EDID)
    {
        /* change sink Type with the currently defined in EDID */
#ifdef __RC51__
#pragma nowarning(C207)
#endif
        err = tmbslHdmiTxEdidGetSinkType(txUnit, &sinkType);
#ifdef __RC51__
#pragma warning(C207)
#endif
        RETIF(err != TM_OK, err + 0x300);

    }

    /* Script: api_output_config */
    err = tmbslHdmiTxVideoOutSetConfig(txUnit, sinkType, voutMode, 
                                HDMITX_VOUT_PREFIL_OFF, HDMITX_VOUT_YUV_BLNK_16,
                                HDMITX_VOUT_QRANGE_FS);
    RETIF(err != TM_OK, err + 0x400);

    /* Set default config */
    pixRepeat = HDMITX_PIXREP_DEFAULT;
    pathBits  = HDMITX_VOUT_DBITS_12;
    syncMethod = HDMITX_VSMETH_V_H;
    toggle    = HDMITX_PIXTOGL_ENABLE;
    
    /* Set sync details */
    if (syncSource == HDMITX_SYNCSRC_EMBEDDED)
    {
        /* Embedded sync */
        syncIn    = EMB;
        spSync    = HDMITX_PIXSUBPKT_SYNC_HEMB;
        blankit   = HDMITX_BLNKSRC_VS_HEMB_VEMB;
        /*PR48 mod: change syncMethod value since it is syncSource dependent*/
        syncMethod = HDMITX_VSMETH_V_XDE;
    }
    else
    {
        /* External sync */
        syncIn    = EXT;
        spSync    = HDMITX_PIXSUBPKT_SYNC_DE;
        blankit   = HDMITX_BLNKSRC_NOT_DE;
    }

#ifdef TMFL_TDA9981_SUPPORT
    /* Get the configuration file information */
    /* Port swap table */
    err = tmbslHdmiTxGetSwapTableConfig(txUnit, swapTable, vinMode);
    RETIF(err != TM_OK, err + 0x410);
    
    if(vinMode == HDMITX_VINMODE_CCIR656)
        pathBits = HDMITX_VOUT_DBITS_8;
    
    /* Port mirror Table */
    err = tmbslHdmiTxGetMirrorTableConfig(txUnit, mirrorTable);
    RETIF(err != TM_OK, err + 0x420);
    /* Video port enable and pulldown table */
    err = tmbslHdmiTxGetVideoPortConfig(txUnit, enaVideoPortTable, gndVideoPortTable);
    RETIF(err != TM_OK, err + 0x430);
    /* Audio port enable and pulldown table */
    err = tmbslHdmiTxGetAudioPortConfig(txUnit, &enaAudioPortCfg, &gndAudioPortCfg);
    RETIF(err != TM_OK, err + 0x440);
    
    /* Set the audio and video input port configuration */
    err = tmbslHdmiTxSetVideoPortConfig(txUnit, enaVideoPortTable, gndVideoPortTable);
    RETIF(err != TM_OK, err + 0x450);
    err = tmbslHdmiTxSetAudioPortConfig(txUnit, &enaAudioPortCfg, &gndAudioPortCfg);
    RETIF(err != TM_OK, err + 0x460);
#else 
    /* Script: api_set_INPUT_port */
    switch (vinMode)
    {
    case HDMITX_VINMODE_YUV422:
        swapTable[0] = 2;
        swapTable[1] = 3;
        swapTable[2] = 0;
        swapTable[3] = 0;
        swapTable[4] = 1;
        swapTable[5] = 0;
        break;
    case HDMITX_VINMODE_CCIR656:
        swapTable[0] = 4;
        swapTable[1] = 5;
        swapTable[2] = 0;
        swapTable[3] = 0;
        swapTable[4] = 0;
        swapTable[5] = 0;
        pathBits = HDMITX_VOUT_DBITS_8;
        break;
    case HDMITX_VINMODE_RGB444:
    case HDMITX_VINMODE_YUV444:
    default:
        swapTable[0] = 2;
        swapTable[1] = 3;
        swapTable[2] = 4;
        swapTable[3] = 5;
        swapTable[4] = 0;
        swapTable[5] = 1;
        break;
    }
    mirrorTable[0] = 0;
    mirrorTable[1] = 0;
    mirrorTable[2] = 0;
    mirrorTable[3] = 0;
    mirrorTable[4] = 0;
    mirrorTable[5] = 0;

#endif /* TMFL_TDA9981_SUPPORT */
    
    err = tmbslHdmiTxVideoInSetMapping(txUnit, swapTable, mirrorTable);
    RETIF(err != TM_OK, err + 0x500);
    err = tmbslHdmiTxVideoInSetFine(txUnit, spSync, HDMITX_PIXTOGL_NO_ACTION);
    RETIF(err != TM_OK, err + 0x600);

    /* Script: api_input_blanking */
    err = tmbslHdmiTxVideoInSetBlanking(txUnit,
          blankit, HDMITX_BLNKCODE_ALL_0);
    RETIF(err != TM_OK, err + 0x700);

    /* Script: api_input_config */
    err = tmbslHdmiTxVideoInSetConfig(txUnit, vinMode, pixelEdge,
                        pixRate, HDMITX_UPSAMPLE_AUTO);
    RETIF(err != TM_OK, err + 0x800);

    /* Only set audio for HDMI, not DVI */
    if (sinkType == HDMITX_SINK_HDMI)
    {
        /* Set audio parameters -> script in */
        err = appHdmiTx_setAudio(txUnit, voutFmt, sinkType, audioFmt, audioRate, i2sQualifier);
        RETIF(err != TM_OK, err + 0x900);
    }

    /* Script: API_output fine adj */
    pixRateSingleDouble = pixRate;
    if (pixRate == HDMITX_PIXRATE_SINGLE_REPEATED)
    {
        pixRateSingleDouble = HDMITX_PIXRATE_SINGLE;
    }

        /* Script: api_input_output  - may give NOT_SUPPORTED error */
    err = tmbslHdmiTxVideoSetInOut(txUnit, vinFmt, HDMITX_SCAMODE_AUTO,
                             voutFmt, pixRepeat, 
				   HDMITX_MATMODE_AUTO, pathBits,HDMITX_VQR_DEFAULT);
    RETIF(err != TM_OK, err + 0xC00);
    
    if (getReflineRefpix(vinFmt, vinMode, voutFmt, syncIn, pixRateSingleDouble,
                 &uRefPix, &uRefLine, &uScRefPix, &uScRefLine, pbVerified) > 0)
    {
        /* Combination found in table for scaler: configure input manually */
        err = tmbslHdmiTxVideoInSetSyncManual(txUnit, syncSource, syncMethod,
                     toggle, toggle, toggle, uRefPix, uRefLine);
        RETIF(err != TM_OK, err + 0xA00);
    }
    else
    {
        /* Not found so assume non-scaler and auto-configure input */
        err = tmbslHdmiTxVideoInSetSyncAuto(txUnit, syncSource, vinFmt, vinMode);
        RETIF(err != TM_OK, err + 0xB00);
    }



    /* Only set infoframes for HDMI, not DVI */
    if (sinkType == HDMITX_SINK_HDMI)
    {
        /* Script: api_set_avi_infoframe */
        err = setVideoInfoframe(txUnit, voutFmt, voutMode);
        RETIF(err != TM_OK, err + 0xD00);
    }

    /* Set scaler details if it has been switched on by previous APIs */
#ifdef __RC51__
#pragma nowarning(C207) /* Hide Raisonance "suspicious pointer conversion" */
#endif
    err = tmbslHdmiTxScalerGetMode(txUnit, &scalerMode);
#ifdef __RC51__
#pragma warning(C207)
#endif
    /* Ignore scaler TMBSL_ERR_HDMI_NOT_SUPPORTED error */
    if ((err == TM_OK) && (scalerMode == HDMITX_SCAMODE_ON))
    {
        /* Script: api_scaler_frame */
        err = tmbslHdmiTxScalerInDisable(txUnit, False);
        RETIF(err != TM_OK, err + 0xE00);

        /* Correction to interlace */
        topSel = HDMITX_TOPSEL_INTERNAL;
        if ((voutFmt == HDMITX_VFMT_05_1920x1080i_60Hz)
        ||  (voutFmt == HDMITX_VFMT_20_1920x1080i_50Hz))
        {
            /* vinFmt is range-checked by tmbslHdmiTxVideoSetInOut above */
            if ((kVfmtToShortFmt_TV[vinFmt] == TV_480p_60Hz)
            ||  (kVfmtToShortFmt_TV[vinFmt] == TV_576p_50Hz))
            {
                /* Correct for 1080i output for p->i conversion only */
                topSel = HDMITX_TOPSEL_VRF;
            }
        }
        err = tmbslHdmiTxScalerSetFieldOrder(txUnit,
            HDMITX_INTEXT_NO_CHANGE, HDMITX_INTEXT_NO_CHANGE,
            topSel, HDMITX_TOPTGL_NO_CHANGE);
        RETIF(err != TM_OK, err + 0xF00);

        /* Script: api_scaler_fineadj */
        err = tmbslHdmiTxScalerSetFine(txUnit, uScRefPix, uScRefLine);
        RETIF(err != TM_OK, err + 0x1000);

        /* Script: api_scaler_sync */
        err = tmbslHdmiTxScalerSetSync(txUnit, syncMethod,
                                       HDMITX_VSONCE_EACH_FRAME);
        RETIF(err != TM_OK, err + 0x1100);

        /* With scaler, use Only Once setting for tmbslHdmiTxVideoOutSetSync */
        once = HDMITX_VSONCE_ONCE;
    }
    else
    {
        once = HDMITX_VSONCE_EACH_FRAME;
    }

    /* Script: api_output_sync */
    err = tmbslHdmiTxVideoOutSetSync(txUnit, HDMITX_VSSRC_INTERNAL,
                               HDMITX_VSSRC_INTERNAL, HDMITX_VSSRC_INTERNAL,
                               HDMITX_VSTGL_TABLE, once);
    RETIF(err != TM_OK, err + 0x1200);
    
    /* tmbslHdmiTxTmdsSetOutputs not needed here as srl_force "toggle"
       in tmbslHdmiTxVideoOutSetSync resets NORMAL mode */

    return TM_OK;
}
/*============================================================================*/
/* Set audio format and audio rate */
tmErrorCode_t
appHdmiTx_setAudio
(
    tmUnitSelect_t          txUnit,     /* Defaults: */
    tmbslHdmiTxVidFmt_t     voutFmt,    /* HDMITX_VFMT_04_1280x720p_60Hz etc */
    tmbslHdmiTxSinkType_t   sinkType,   /* HDMITX_SINK_HDMI */
    tmbslHdmiTxaFmt_t       audioFmt,   /* HDMITX_AFMT_SPDIF */
    tmbslHdmiTxafs_t        audioRate,  /* HDMITX_AFS_48K */
    UInt8                   i2sQualifier
)
{
    tmErrorCode_t           err;        /* Error code from APIs */
    tmbslHdmiTxVfreq_t      vOutFreq;   /* Vertical output frequency */
    UInt16                  uCtsX;      /* CtsX value */
    UInt8                   layout;     /* 0 or 1 */
    tmbslHdmiTxctsRef_t     ctsRef;     /* CTS ref source */
    UInt8                   aifChannelCountCode = 0; /* audio info frame channels */
    tmbslHdmiTxPktAif_t     pktAif;     /* Audio infoframe packet */

    /* Only set audio for HDMI, not DVI */
    if (sinkType == HDMITX_SINK_HDMI)
    {
        if (AudioChannelAllocation == 0)
        {
            /* 2 channels */
            layout = 0;
        }
        else
        {
            /* Multi channels */
            layout = 1;
        }

        err = tmbslHdmiTxAudioInSetConfig(txUnit,
                        audioFmt, AudioChannelAllocation,
                        HDMITX_CHAN_NO_CHANGE, HDMITX_CLKPOLDSD_NO_CHANGE,
                        HDMITX_SWAPDSD_NO_CHANGE, layout,
                        0x80); /* uLatency_rd */
        RETIF(err != TM_OK, err + 0x010);

        vOutFreq = fmtToFreq(voutFmt);
        if ((audioFmt == HDMITX_AFMT_SPDIF) || (audioFmt == HDMITX_AFMT_DSD))
        {
            ctsRef = HDMITX_CTSREF_FS64SPDIF;
            uCtsX = HDMITX_CTSX_64;
        }
        else /* I2S */
        {
            ctsRef = HDMITX_CTSREF_ACLK;
            if (i2sQualifier == 32)
            {
                uCtsX = HDMITX_CTSX_64;
            }
            else
            {
                uCtsX = HDMITX_CTSX_32;
            }
        }
        err = tmbslHdmiTxAudioInSetCts(txUnit, ctsRef,
                        audioRate, voutFmt, vOutFreq, HDMITX_CTS_AUTO,
                        uCtsX, HDMITX_CTSK_USE_CTSX, HDMITX_CTSMTS_USE_CTSX);
        RETIF(err != TM_OK, err + 0x020);

        /* Set Channel Status registers
           No need to call tmbslHdmiTxAudioOutSetChanStatusMapping, since default Byte 2
           values of "Do not take into account" are adequate */
        err = tmbslHdmiTxAudioOutSetChanStatus(txUnit, HDMITX_CSFI_PCM_2CHAN_NO_PRE,
                        HDMITX_CSCOPYRIGHT_PROTECTED, 0x00, audioRate, 
                        HDMITX_CSCLK_LEVEL_II, HDMITX_CSMAX_LENGTH_20, HDMITX_CSWORD_DEFAULT,
                        HDMITX_CSOFREQ_NOT_INDICATED);
        RETIF(err != TM_OK, err + 0x030);

        /* Set reset_fifo to 1 */
        err = tmbslHdmiTxAudioOutSetMute(txUnit, HDMITX_AMUTE_ON);
        RETIF(err != TM_OK, err + 0x040);
        /* Wait for 20 ms */
        (void)tmbslHdmiTxSysTimerWait(txUnit, 20);
        /* Set reset_fifo to 0 */
        err = tmbslHdmiTxAudioOutSetMute(txUnit, HDMITX_AMUTE_OFF);
        RETIF(err != TM_OK, err + 0x050);

        aifChannelCountCode = kChanAllocChanNum[AudioChannelAllocation] - 1;

        /* Script: api_set_audio_infoframe */
        pktAif.ChannelCount       = aifChannelCountCode;
        pktAif.CodingType         = 0; /* refer to stream header */
        pktAif.SampleSize         = 0; /* refer to stream header */
        pktAif.ChannelAlloc       = AudioChannelAllocation;
        pktAif.LevelShift         = 0; /* 0dB level shift */
        pktAif.DownMixInhibit     = 0; /* down-mix stereo permitted */
        pktAif.SampleFreq		= AIF_SF_REFER_TO_STREAM_HEADER; /* refer to stream header */
        err = tmbslHdmiTxPktSetAudioInfoframe(txUnit, &pktAif, True);
        RETIF(err != TM_OK, err + 0x060);
    }
    else
    {
        err = TMBSL_ERR_HDMI_BAD_PARAMETER;
    }
    return err;
}

/*============================================================================*/
/* Set audio channel allocation */
void
appHdmiTx_setAudioChannelAllocation
(
    UInt8                   channelAllocation
)
{

    AudioChannelAllocation = channelAllocation;

}

#ifndef NO_HDCP
/*============================================================================*/
/* Set HDCP On */
tmErrorCode_t
appHdmiTx_Hdcp_On
(
    tmUnitSelect_t           txUnit,
    tmbslHdmiTxVidFmt_t      voutFmt,
    tmbslHdmiTxHdcpOptions_t options
)
{
    tmErrorCode_t            err;
    tmbslHdmiTxVfreq_t       voutFreq;
    UInt8                    slaveAddress;

    /* Reset HDCP app data to ensure that new values are used */
    HdcpKsvDevices = 0;
    lAppMemset(HdcpBksv, 0, sizeof(HdcpBksv));

    /* Derive output vertical frequency from output format */
    voutFreq = fmtToFreq(voutFmt);

    /* Configure HDCP */
    slaveAddress = HDMITX_HDCP_SLAVE_PRIMARY;
#ifdef DEMO_REPEATER
    txMode       = HDMITX_HDCP_TXMODE_REPEATER;
#else /* DEMO_REPEATER */
    txMode       = HDMITX_HDCP_TXMODE_TOP_LEVEL;
#endif /* DEMO_REPEATER */
    err = tmbslHdmiTxHdcpConfigure(txUnit, slaveAddress, txMode, options,
        HDCP_CHECK_INTERVAL_MS, HDCP_NUM_CHECKS);
    RETIF(err != TM_OK, err + 0x100);

    /* Start HDCP */
    err = tmbslHdmiTxHdcpInit(txUnit, voutFmt, voutFreq);
    RETIF(err != TM_OK, err + 0x200);
    err = tmbslHdmiTxHdcpRun(txUnit);
    return err;
}

/*============================================================================*/
/* Set HDCP Off */
tmErrorCode_t
appHdmiTx_Hdcp_Off
(
    tmUnitSelect_t txUnit
)
{
    return tmbslHdmiTxHdcpStop(txUnit);
}

/*============================================================================*/
/* HDCP ENCRYPT interrupt handler */
tmErrorCode_t 
appHdmiTx_handleENCRYPT
(
    tmUnitSelect_t txUnit
)
{
    return tmbslHdmiTxHdcpHandleENCRYPT(txUnit);
}

/*============================================================================*/
/* HDCP T0 interrupt handler */
tmErrorCode_t
appHdmiTx_handleT0
(
    tmUnitSelect_t txUnit
)
{
    return tmbslHdmiTxHdcpHandleT0(txUnit);
}

/*============================================================================*/
/* HDCP BCAPS interrupt handler */
tmErrorCode_t 
appHdmiTx_handleBCAPS
(
    tmUnitSelect_t txUnit,
    Bool *pbBksvSecure
)
{
    Bool  bCheckRequired;
    tmErrorCode_t  err;

    /* Handle BCAPS interrupt immediately */
    err = tmbslHdmiTxHdcpHandleBCAPS(txUnit);
    RETIF(err != TM_OK, err + 0x100);

    /* Wait for TDA9983 to read BKSV from B device */
    (void)tmbslHdmiTxSysTimerWait(txUnit, 10); /* ms */

    /* Handle BKSV read */
    err = tmbslHdmiTxHdcpHandleBKSV(txUnit, HdcpBksv, &bCheckRequired);
    RETIF(err != TM_OK, err + 0x200);
    if (bCheckRequired)
    {
        /* Check HdcpBksv against a revocation list */
        *pbBksvSecure = True;
        lAppCheckHdcpBksv (HdcpBksv,pbBksvSecure, True);
    }
    else
    {
        /* Result is always secure if no check required */
        *pbBksvSecure = True;
    }

    /* Handle BKSV result */
    err = tmbslHdmiTxHdcpHandleBKSVResult(txUnit, *pbBksvSecure);
    RETIF(err != TM_OK, err + 0x300);

    return TM_OK;
}

/*============================================================================*/
/* HDCP BSTATUS interrupt handler */
tmErrorCode_t 
appHdmiTx_handleBSTATUS
(
    tmUnitSelect_t txUnit
)
{
    UInt16 bstatus;
    
    return tmbslHdmiTxHdcpHandleBSTATUS(txUnit, &bstatus);
}

/*============================================================================*/
/* HDCP SHA_1 interrupt handler */
tmErrorCode_t 
appHdmiTx_handleSHA_1
(
    tmUnitSelect_t txUnit
)
{
    Bool  bKsvSecure;
    tmErrorCode_t  err;
    UInt8 indexKSVList;
    UInt8 depth;


    err = tmbslHdmiTxHdcpHandleSHA_1(txUnit, HDMITX_KSV_LIST_MAX_DEVICES,
                                     HdcpKsvList, &HdcpKsvDevices,&depth);
    RETIF(err != TM_OK, err + 0x100);
#ifndef TMFL_TDA9981_SUPPORT
    if (HdcpKsvDevices > 0)
    {
#endif /* TMFL_TDA9981_SUPPORT */
        if (txMode == HDMITX_HDCP_TXMODE_TOP_LEVEL)
        {
            /* Check HdcpKsvList against revocation list */
            bKsvSecure = True;
   
            indexKSVList = 0;
            while (( indexKSVList <  HDMITX_KSV_LIST_MAX_DEVICES ) &&
                   ( indexKSVList <  HdcpKsvDevices ) && 
                   (bKsvSecure == True))
            {
               lAppCheckHdcpBksv (&(HdcpKsvList[indexKSVList * HDMITX_KSV_BYTES_PER_DEVICE]),&bKsvSecure, False);
               indexKSVList++;
            }

        }
        else
        {
            /* Forece bKsvSecure in repeater mode */
            bKsvSecure = True;
        }

        /* Handle SHA_1 result */
        err = tmbslHdmiTxHdcpHandleSHA_1Result(txUnit, bKsvSecure);
        RETIF(err != TM_OK, err + 0x200);
#ifndef TMFL_TDA9981_SUPPORT
    }
#endif /* TMFL_TDA9981_SUPPORT */
    return TM_OK;
}

/*============================================================================*/
/* HDCP PJ interrupt handler */
tmErrorCode_t 
appHdmiTx_handlePJ
(
    tmUnitSelect_t txUnit
)
{
    return tmbslHdmiTxHdcpHandlePJ(txUnit);
}
#endif /* NO_HDCP */

/*============================================================================*/
/* Hot Plug Detect interrupt handler */
tmErrorCode_t 
appHdmiTx_handleHPD
(
    tmUnitSelect_t          txUnit,
    tmbslHdmiTxHotPlug_t    *pHotPlugStatus,
    UInt8                   *pEdidStatus,
    UInt8                   *pEdidVidFlags
)
{
    tmErrorCode_t  err;

    /* Get Hot Plug status */
    err = tmbslHdmiTxHotPlugGetStatus(txUnit, pHotPlugStatus);
    RETIF(err != TM_OK, err + 0x100);

    /* Has hot plug changed to Active? */
    if (*pHotPlugStatus == HDMITX_HOTPLUG_ACTIVE)
    {
        /* Yes: Wait for DDC line to settle before reading EDID */
        (void)tmbslHdmiTxSysTimerWait(txUnit, 500); /* ms */

        /* Read and parse sink's EDID and get latest EDID status */
        err = tmbslHdmiTxEdidGetBlockData(txUnit, Null, 0, 0, pEdidStatus);
        RETIF(err != TM_OK, err + 0x200);

        /* Get video capabilities to help decide on available formats */
        err = tmbslHdmiTxEdidGetVideoCapabilities(txUnit, EdidVFmts,
              sizeof(EdidVFmts), &EdidVFmtsAvail, pEdidVidFlags);
        RETIF(err != TM_OK, err + 0x300);
    }
    else
    {
        /* No: no EDID video formats are available */
        EdidVFmtsAvail = 0;
    }
    return TM_OK;
}

#ifndef NO_HDCP
/*============================================================================*/
/* Get pointer to KSV list and number of devices */
void 
appHdmiTx_getKsvList
(
    UInt8          **ppKsvList,
    UInt8          *pKsvDevices
)
{
    *ppKsvList   = &HdcpKsvList[0];
    *pKsvDevices = HdcpKsvDevices;
}

/*============================================================================*/
/* Get BKSV */
void 
appHdmiTx_getBksv
(
    UInt8          **ppBksv
)
{
    *ppBksv = &HdcpBksv[0];
}
#endif /* NO_HDCP */

/*============================================================================*/
/* Check a video format against Short Video Descriptors last read from EDID */
Bool
appHdmiTx_checkVidFmt
(
    tmbslHdmiTxVidFmt_t     vidFmt
)
{
    UInt i;

    /* Check for vidFmt in EDID SVD list, masking out the Native format bit */
    for (i = 0; i < EdidVFmtsAvail; i++)
    {
        if (vidFmt == (tmbslHdmiTxVidFmt_t)
                      (EdidVFmts[i] & HDMI_TX_SVD_NATIVE_NOT))
        {
            return True; /* vidFmt found */
        }
    }
    return False; /* vidFmt not found */
}

/*============================================================================*/
/* Find next video format in Short Video Descriptors last read from EDID */
Bool
appHdmiTx_nextEdidVidFmt
(
    tmbslHdmiTxVidFmt_t     vidFmtOld,
    tmbslHdmiTxVidFmt_t     *pVidFmtNew
)
{
    UInt i;
    Bool bFound;

    /* Can't find an EDID SVD if nothing in SVD list */
    if (EdidVFmtsAvail == 0)
    {
        return False; /* *pVidFmtNew unchanged */
    }

    /* Find index to vidFmt or to next higher format in EDID SVD list,
     * masking out the Native format bit as we go
     * THIS ASSUMES EdidVFmts IS IN ASCENDING ORDER */
    for (bFound = False, i = 0; i < EdidVFmtsAvail; i++)
    {
        if (vidFmtOld == 
            (tmbslHdmiTxVidFmt_t)
            (EdidVFmts[i] & HDMI_TX_SVD_NATIVE_NOT))
        {
            i++; /* SVD matched, pick next one */
            bFound = True; /* Index to next vidFmt found */
            break;
        }
        else if (vidFmtOld < 
            (tmbslHdmiTxVidFmt_t)
            (EdidVFmts[i] & HDMI_TX_SVD_NATIVE_NOT))
        {
            bFound = True; /* Index to next vidFmt found */
            break;
        }
    }
    if (bFound)
    {
        /* Return the next SVD in the list, or wrap to the first */
        if (i >= EdidVFmtsAvail)
        {
            i = 0;
        }
    }
    else
    {
        /* vidFmt is higher than the highest SVD so return the first */
        i = 0;
    }
    *pVidFmtNew = EdidVFmts[i] & HDMI_TX_SVD_NATIVE_NOT;
    
    return True;
}



/*============================================================================*/
/* Set revocation list                                                        */
tmErrorCode_t 
appHdmiTx_SetHDCPRevocationList
(
    void           *listPtr,
    UInt32          Length
)
{
    tmErrorCode_t errCode = TM_OK;

    ptrHDCPRevocationList  = (UInt8*)listPtr;
    HDCPRevocationListSize = Length;

    return errCode;

}

/*============================================================================*/
/*                   STATIC FUNCTION DEFINITIONS                              */
/*============================================================================*/


/*============================================================================*/
/* Convert an output format to its vertical frequency */
static tmbslHdmiTxVfreq_t
fmtToFreq
(
    tmbslHdmiTxVidFmt_t fmt
)
{
    tmbslHdmiTxVfreq_t vFreq;

    if ((fmt >= HDMITX_VFMT_17_720x576p_50Hz) && (fmt < HDMITX_VFMT_TV_NO_REG_MIN))
    {
        /* 50Hz formats */
        vFreq = HDMITX_VFREQ_50Hz;
    }
    else if ((fmt == HDMITX_VFMT_04_1280x720p_60Hz)
        ||   (fmt == HDMITX_VFMT_05_1920x1080i_60Hz)
        ||   (fmt == HDMITX_VFMT_16_1920x1080p_60Hz)
#ifdef FORMAT_PC
        ||   ((fmt >= HDMITX_VFMT_PC_640x480p_60Hz) &&
              (fmt <= HDMITX_VFMT_PC_1600x1200p_60Hz))
#endif
        )
    {
        /* 60Hz HD formats */
        vFreq = HDMITX_VFREQ_60Hz;
    }
    else if (fmt == HDMITX_VFMT_32_1920x1080p_24Hz)
    {
        vFreq = HDMITX_VFREQ_24Hz;
    }
    else if (fmt == HDMITX_VFMT_33_1920x1080p_25Hz)
    {
        vFreq = HDMITX_VFREQ_25Hz;
    }
    else if (fmt == HDMITX_VFMT_34_1920x1080p_30Hz)
    {
        vFreq = HDMITX_VFREQ_30Hz;
    }
#ifdef FORMAT_PC
    else if ((fmt == HDMITX_VFMT_PC_1024x768p_70Hz))
    {
        vFreq = HDMITX_VFREQ_70Hz;
    }
    else if ((fmt >= HDMITX_VFMT_PC_640x480p_72Hz) &&
             (fmt <= HDMITX_VFMT_PC_800x600p_72Hz))
    {
        vFreq = HDMITX_VFREQ_72Hz;
    }
    else if ((fmt >= HDMITX_VFMT_PC_640x480p_75Hz) &&
             (fmt <= HDMITX_VFMT_PC_1280x1024p_75Hz))
    {
        vFreq = HDMITX_VFREQ_75Hz;
    }
    else if ((fmt >= HDMITX_VFMT_PC_640x350p_85Hz) &&
             (fmt <= HDMITX_VFMT_PC_1280x1024p_85Hz))
    {
        vFreq = HDMITX_VFREQ_85Hz;
    }
    else if ((fmt == HDMITX_VFMT_PC_1024x768i_87Hz))
    {
        vFreq = HDMITX_VFREQ_87Hz;
    }
#endif /* FORMAT_PC */
    else
    {
        /* 60Hz SD formats */
        vFreq = HDMITX_VFREQ_59Hz;
    }
    return vFreq;
}

/*============================================================================*/
/* Get the REFPIX and REFLINE for output and scaler for the current settings */
static Bool
getReflineRefpix
(
    tmbslHdmiTxVidFmt_t     vinFmt,     /* HDMITX_VFMT_ */
    tmbslHdmiTxVinMode_t    vinMode,    /* HDMITX_VINMODE_ */
    tmbslHdmiTxVidFmt_t     voutFmt,    /* HDMITX_VFMT_ */
    UInt8                   syncIn,     /* EMB or EXT */
    tmbslHdmiTxPixRate_t    pixRate,    /* HDMITX_PIXRATE_ */
    UInt16                  *pRefPix,   /* REFPIX for output */
    UInt16                  *pRefLine,  /* REFLINE for output */
    UInt16                  *pScRefPix, /* REFPIX for scaler */
    UInt16                  *pScRefLine,/* REFLINE for scaler */
    Bool                     *pbVerified /* True: combination tested ok */
)
{
    UInt8 shortVinFmt;
    UInt8 shortVoutFmt;
    int i;
    Bool bFound;

    /* Search for all values to match in table, until table end is reached
     * when both refPix values are zero */
    *pRefPix    = 0;
    *pRefLine   = 0;
    *pScRefPix  = 0;
    *pScRefLine = 0;
    /* If match is not found in table, we can assume a verified non-scaler 
     * combination */
    *pbVerified = 1;
    bFound = 0;
    if (voutFmt  < HDMITX_VFMT_TV_NO_REG_MIN)
    {
        shortVinFmt  = kVfmtToShortFmt_TV[vinFmt];
        shortVoutFmt = kVfmtToShortFmt_TV[voutFmt];

        for (i = 0; kRefpixRefline[i].shortVinFmt != TV_INVALID; i++)
        {
            if ((kRefpixRefline[i].shortVinFmt == shortVinFmt)
                &&  (UNPKMODE(kRefpixRefline[i].modeRateSyncVerf) == vinMode)
            &&  (kRefpixRefline[i].shortVoutFmt == shortVoutFmt)
            &&  (UNPKRATE(kRefpixRefline[i].modeRateSyncVerf) == pixRate)
            &&  (UNPKSYNC(kRefpixRefline[i].modeRateSyncVerf) == syncIn))
            {
                *pRefPix    = kRefpixRefline[i].refPix;
                *pRefLine   = kRefpixRefline[i].refLine;
                *pScRefPix  = kRefpixRefline[i].scRefPix;
                *pScRefLine = kRefpixRefline[i].scRefLine;
                *pbVerified = UNPKVERF(kRefpixRefline[i].modeRateSyncVerf);
                bFound = 1;
                break;
            }
        }
    }
    return bFound;
}

/*============================================================================*/
/* Set the video infoframe */
static tmErrorCode_t
setVideoInfoframe
(
    tmUnitSelect_t          txUnit,
    tmbslHdmiTxVidFmt_t     voutFmt,
    tmbslHdmiTxVoutMode_t   voutMode
)
{
    tmErrorCode_t       err;    /* Error code */
    tmbslHdmiTxPktVif_t pktVif;
    tmbslHdmiTxVidFmt_t vifInfoFrame;

    pktVif.Colour             = voutMode; /* 3rd api_set_avi_infoframe param */
    pktVif.ActiveInfo         = 0;
    pktVif.BarInfo            = 0;
    pktVif.ScanInfo           = 0;
    pktVif.Colorimetry        = 1;
#ifndef FORMAT_PC
    pktVif.PictureAspectRatio = kVfmtToAspect_TV[voutFmt];
#else /* FORMAT_PC */
    if ((voutFmt >= HDMITX_VFMT_PC_MIN)
        && (voutFmt <= HDMITX_VFMT_PC_MAX))
    {
        pktVif.PictureAspectRatio = kVfmtToAspect_TV[voutFmt - HDMITX_VFMT_PC_MIN + 1];
    }
    else 
        pktVif.PictureAspectRatio = kVfmtToAspect_TV[voutFmt];
#endif /* FORMAT_PC */
    pktVif.ActiveFormatRatio  = 8;
    pktVif.Scaling            = 0;
    
#ifdef FORMAT_PC
    if (voutFmt >= HDMITX_VFMT_PC_MIN)
    {
        if (voutFmt == HDMITX_VFMT_PC_640x480p_60Hz)
        {
            vifInfoFrame = HDMITX_VFMT_01_640x480p_60Hz;
        }
        else
        {
            vifInfoFrame = HDMITX_VFMT_NULL; /*Format PC not Valid in eia861b*/
        }
    }
    else
    {
#endif /* FORMAT_PC */

        vifInfoFrame = voutFmt;
        
#ifdef FORMAT_PC
    }
#endif /* FORMAT_PC */
    
    pktVif.VidFormat          = vifInfoFrame;
    if (((voutFmt >= HDMITX_VFMT_06_720x480i_60Hz) && (voutFmt <= HDMITX_VFMT_09_720x240p_60Hz))
        || ((voutFmt >= HDMITX_VFMT_21_720x576i_50Hz) && (voutFmt <= HDMITX_VFMT_24_720x288p_50Hz)))
    {
        pktVif.PixelRepeat    = 1; /* force pixel repeat for formats where it's mandatory */
    }
    else
    {
        pktVif.PixelRepeat    = HDMITX_PIXREP_NONE; /* default to no repeat for all other formats */
    }
    pktVif.EndTopBarLine      = 0;
    pktVif.StartBottomBarLine = 0;
    pktVif.EndLeftBarPixel    = 0;
    pktVif.StartRightBarPixel = 0;
    err = tmbslHdmiTxPktSetVideoInfoframe(txUnit, &pktVif, True);
    /* Ignore infoframe interlock in DVI mode */
    if (err == TMBSL_ERR_HDMI_OPERATION_NOT_PERMITTED)
    {
        err = TM_OK;
    }
    return err;
}
/*============================================================================*/
/* Read Edid After Power On */
tmErrorCode_t 
appHdmiTx_ReadEdidAtPowerOn
(
    tmUnitSelect_t          txUnit,
    UInt8                   *pEdidStatus,
    UInt8                   *pEdidVidFlags
)
{
    tmbslHdmiTxHotPlug_t    HotPlugStatus = HDMITX_HOTPLUG_INACTIVE;
    tmErrorCode_t  err;

    /* Get Hot Plug status */
#ifdef __RC51__
#pragma nowarning(C207)
#endif
    err = tmbslHdmiTxHotPlugGetStatus(txUnit, &HotPlugStatus);
#ifdef __RC51__
#pragma warning(C207)
#endif
    RETIF(err != TM_OK, err + 0x100);

    /* Has hot plug changed to Active? */
    if (HotPlugStatus == HDMITX_HOTPLUG_ACTIVE)
    {
        /* Read and parse sink's EDID and get latest EDID status */
        err = tmbslHdmiTxEdidGetBlockData(txUnit, Null, 0, 0, pEdidStatus);
        RETIF(err != TM_OK, err + 0x200);

        /* Get video capabilities to help decide on available formats */
        err = tmbslHdmiTxEdidGetVideoCapabilities(txUnit, EdidVFmts,
              sizeof(EdidVFmts), &EdidVFmtsAvail, pEdidVidFlags);
        RETIF(err != TM_OK, err + 0x300);
    }
    else
    {
        /* No: no EDID video formats are available */
        EdidVFmtsAvail = 0;
    }
    return TM_OK;
}

/*============================================================================*/
/* lAppMemset                                                                     */
/*============================================================================*/
static void
lAppMemset
(
    void *pTable1,
    const UInt8 value,
    UInt Size
)
{
    char *ptrSource = (char*) pTable1;
    char *endSource = (char*)pTable1 + Size;

    if (pTable1 != Null)
    {
        while (endSource > ptrSource)
        {
            *(ptrSource++) = value;
        }
    }
}




/******************************************************************************
    \brief lAppCheckHdcpBksv .

    \param pHdcpBksvTested  HdcpBksv To test.
    \param pbBksvSecure     Test result.

    \return NA.

******************************************************************************/
static void lAppCheckHdcpBksv
(
UInt8 * pHdcpBksvTested,
Bool  * pbBksvSecure,
Bool    bBigEndian
)
{

    UInt32 NbInRevocationList = 0;
    UInt32 ListIndex;
    Bool   KsvSecure;
    UInt8 i;
    
    

    if (ptrHDCPRevocationList != Null)
    {
        while ((*pbBksvSecure == True) && (NbInRevocationList < HDCPRevocationListSize))
        {
           ListIndex = NbInRevocationList * HDMITX_KSV_BYTES_PER_DEVICE;

           KsvSecure = False;
           

           for (i = 0; i < HDMITX_KSV_BYTES_PER_DEVICE; i++)
           {
              if (bBigEndian)
              {
                 if (pHdcpBksvTested[i] != ptrHDCPRevocationList[i + ListIndex])
                 {
                    KsvSecure = True;
                 }
              }
              else /* little endian */
              {
                if (pHdcpBksvTested[HDMITX_KSV_BYTES_PER_DEVICE - i - 1] != ptrHDCPRevocationList[i + ListIndex])
                 {
                    KsvSecure = True;
                 }
              }
           }
           
           *pbBksvSecure = KsvSecure; 
           
              
           NbInRevocationList++;
        }
    }
}

/*============================================================================*/
/*                     END OF FILE                                            */
/*============================================================================*/
