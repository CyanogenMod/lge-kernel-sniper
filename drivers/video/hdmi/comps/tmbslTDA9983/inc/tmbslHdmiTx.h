/**
 * Copyright (C) 2006 Koninklijke Philips Electronics N.V., All Rights Reserved.
 * This source code and any compilation or derivative thereof is the proprietary
 * information of Koninklijke Philips Electronics N.V. and is confidential in
 * nature. Under no circumstances is this software to be  exposed to or placed
 * under an Open Source License of any type without the expressed written
 * permission of Koninklijke Philips Electronics N.V.
 *
 * \file          tmbslHdmiTx.h
 *
 * \version       $Revision: 41 $
 *
 * \date          $Date: 17/10/07 14:11 $
 *
 * \brief         BSL driver component API for the TDA998x HDMI Transmitter
 *
 * \section refs  Reference Documents
 * HDMI Driver - Outline Architecture.doc,
 * HDMI Driver - tmbslHdmiTx - SCS.doc
 *
 * \section info  Change Information
 *
 * \verbatim

   $History: tmbslHdmiTx.h $
 *
 * *****************  Version 41  ****************
 * User: B.Vereecke     Date: 17/10/07   Time: 14:11
 * Updated in $/Source/tmbslHdmiTx/src
 * PR872 : add new formats, 1080p24/25/30
 *
 * *****************  Version 40  *****************
 * User: B.Vereecke      Date: 17/07/07   Time: 10:30
 * Updated in $/Source/tmbslHdmiTx/Inc
 * PR217 - Add a new Pattern type in tmbslHdmiTxTestPattern_t
 *			it is used for set the bluescreen
 *
 * *****************  Version 39  *****************
 * User: J. Lamotte      Date: 13/06/07   Time: 12:00
 * Updated in $/Source/tmbslHdmiTx/Inc
 * PR397 - Merge with PR322
 *       add 9981 API definitions for 9983 
 *       (return NOT_SUPPORTED)
 *
 * *****************  Version 38  *****************
 * User: G. BURNOUF      Date: 08/06/07   Time: 10:40
 * Updated in $/Source/tmbslHdmiTx/Inc
 * PR347 - Add new formatPC
 *
 * *****************  Version 37  *****************
 * User: J/ Lamotte         Date: 24/04/07   Time: 10:40
 * Updated in $/Source/tmbslHdmiTx/Inc
 * PR50 - Change comment for tmbslHdmiTxSetAudioPortConfig
 *        and tmbslHdmiTxSetVideoPortConfig functions (TDA9981)
 *
 * *****************  Version 36  *****************
 * User: Burnouf         Date: 16/04/07   Time: 11:30
 * Updated in $/Source/tmbslHdmiTx/Inc
 * PR50 - add new API tmbslHdmiTxRxSenseGetStatus for TDA9981
 *      - add new API only for debug tmbslHdmiTxFlagSwInt for TDA9981
 *      - add new index for the new callback interrupt of TDA9981
 *
 * *****************  Version 35  *****************
 * User: J. Lamotte      Date: 16/04/07   Time: 11:30
 * Updated in $/Source/tmbslHdmiTx/Inc
 * PR50 - add video and audio port configuration for TDA9981 
 *        tmbslHdmiTxSetAudioPortConfig and 
 *        tmbslHdmiTxSetVideoPortConfig
 *      - define HDMITX_UNITS_MAX (in tmbslHdmiTx_local.h before)
 *
 * *****************  Version 34  *****************
 * User: J. Lamotte    Date: 25/04/07   Time: 14:40
 * Updated in $/Source/tmbslHdmiTx/Inc
 * PR273 - add PLL configuration before soft reset
 *         in function tmbslHdmiTxInit (2 more parameters)
 * 
 * *****************  Version 33  *****************
 * User: C. Logiou      Date: 08/03/07   Time: 16:52
 * Updated in $/Source/tmbslHdmiTx/Inc
 * PR214 - add new input format repeated 480i/576i
 * 
 * *****************  Version 32  *****************
 * User: burnouf      Date: 06/02/07   Time: 16:52
 * Updated in $/Source/tmbslHdmiTx/Inc
 * PR49 - add PC Formats
 * 
 * *****************  Version 31  *****************
 * User: burnouf      Date: 08/01/07   Time: 15:33
 * Updated in $/Source/tmbslHdmiTx/Inc
 * PR72 - add function tmbslHdmiTxHwGetCapabilities
 * 
 * *****************  Version 30  *****************
 * User: Djw          Date: 17/11/06   Time: 18:48
 * Updated in $/Source/tmbslHdmiTx/Inc
 * PNF72 - Corrections to doxygen info for
 * tmbslHdmiTxAudioOutSetChanStatus
 * 
 * *****************  Version 28  *****************
 * User: Mayhew       Date: 10/11/06   Time: 10:35
 * Updated in $/Source/tmbslHdmiTx/Inc
 * PNF68 Add note to tmbslHdmiTxHdcpConfigure for disabling authentication
 * checking
 * 
 * *****************  Version 27  *****************
 * User: Mayhew       Date: 10/11/06   Time: 10:03
 * Updated in $/Source/tmbslHdmiTx/Inc
 * PNF68 HdcpCheck params thisCheck and maxChecks replaced by
 * uTimeSinceLastCallMs, and Check_t values reordered
 * PNF68 Params uCheckIntervalMs and uChecksToDo added to
 * tmbslHdmiTxHdcpConfigure
 * PNF68 tmbslHdmiTxHwHandleTimer prototype removed
 * 
 * *****************  Version 26  *****************
 * User: Djw          Date: 9/11/06    Time: 18:05
 * Updated in $/Source/tmbslHdmiTx/Inc
 * PNF69 Add tmbslHdmiTxAudioOutSetChanStatus and
 * tmbslHdmiTxAudioOutSetChanStatusMapping
 * 
 * *****************  Version 25  *****************
 * User: Mayhew       Date: 6/11/06    Time: 17:46
 * Updated in $/Source/tmbslHdmiTx/Inc
 * PNF68 Add tmbslHdmiTxHwHandleTimer
 * 
 * *****************  Version 23  *****************
 * User: Djw          Date: 25/10/06   Time: 13:41
 * Updated in $/Source/tmbslHdmiTx/Inc
 * PNF58 Added vinMode parameter to tmbslHdmiTxVideoInSetSyncAuto
 * 
 * *****************  Version 21  *****************
 * User: Mayhew       Date: 13/10/06   Time: 11:01
 * Updated in $/Source/tmbslHdmiTx/Inc
 * PNF37 Move arguments from tmbslHdmiTxVideoInSetConfig,
 * tmbslHdmiTxVideoOutSetSync and tmbslHdmiTxVideoOutSetFine (deleted)
 * to new APIs tmbslHdmiTxVideoInSetSyncAuto/Manual
 * 
 * *****************  Version 19  *****************
 * User: Mayhew       Date: 15/09/06   Time: 15:54
 * Updated in $/Source/tmbslHdmiTx/Inc
 * PNF22 Add HdcpCheck API
 * PNF23 Add GetOtp API
 * 
 * *****************  Version 17  *****************
 * User: Mayhew       Date: 4/09/06    Time: 10:03
 * Updated in $/Source/tmbslHdmiTx/Inc
 * PNF20: Add HDMITX_HDCP_OPTION_FORCE_VSLOW_DDC
 * 
 * *****************  Version 15  *****************
 * User: Djw          Date: 23/08/06   Time: 18:18
 * Updated in $/Source/tmbslHdmiTx/Inc
 * Updated comments for tmbslHdmiTxctsRef_t enum.
 * 
 * *****************  Version 14  *****************
 * User: Djw          Date: 21/08/06   Time: 14:02
 * Updated in $/Source/tmbslHdmiTx/Inc
 * Correction to tmbslHdmiTxTmdsOut_t enum.
 * 
 * *****************  Version 13  *****************
 * User: Mayhew       Date: 10/07/06   Time: 12:31
 * Updated in $/Source/tmbslHdmiTx/Inc
 * Fix Doxygen comment warnings
 * 
 * *****************  Version 11  *****************
 * User: Mayhew       Date: 30/06/06   Time: 12:40
 * Updated in $/Source/tmbslHdmiTx/Inc
 * HDMITX_EDID_SINK_ type replaced with HDMITX_SINK_ type for clarity
 * 
 * *****************  Version 9  *****************
 * User: Djw          Date: 16/06/06   Time: 12:05
 * Updated in $/Source/tmbslHdmiTx/Inc
 * Added flag to Init to support use of alternate i2c address for EDID.
 * 
 * *****************  Version 7  *****************
 * User: Mayhew       Date: 5/06/06    Time: 14:28
 * Updated in $/Source/tmbslHdmiTx/Inc
 * Add tmbslHdmiTxHdcpGetT0FailState, tmbslHdmiTxSysTimerWait. Move error
 * codes from _local.h.
 * 
 * *****************  Version 5  *****************
 * User: Djw          Date: 24/05/06   Time: 11:15
 * Updated in $/Source/tmbslHdmiTx/Inc
 * Added Infoframe packet APIs
 * 
 * *****************  Version 4  *****************
 * User: Mayhew       Date: 15/05/06   Time: 15:56
 * Updated in $/Source/tmbslHdmiTx/Inc
 * Correct the order of values in tmbslHdmiTxVoutDbits_t. Add mask to
 * tmbslHdmiTxHdcpOptions_t.
 * 
 * *****************  Version 3  *****************
 * User: Mayhew       Date: 10/05/06   Time: 16:58
 * Updated in $/Source/tmbslHdmiTx/Inc
 * Added HDCP APIs, ScalerGetMode and HwStartup
 * 
 * *****************  Version 2  *****************
 * User: Mayhew       Date: 11/04/06   Time: 14:20
 * Updated in $/Source/tmbslHdmiTx/Inc
 * Add HDMITX_UPSAMPLE_AUTO
 * 
 * *****************  Version 1  *****************
 * User: Mayhew       Date: 4/04/06    Time: 16:27
 * Created in $/Source/tmbslHdmiTx/Inc
 * Driver API phase 2
 
   \endverbatim
 *
*/

#ifndef TMBSLHDMITX_H
#define TMBSLHDMITX_H

/*============================================================================*/
/*                       COMPILER COMMAND LINE BUILD OPTIONS                  */
/*                                                                            */
/* #define          PC Build            8051 Build                            */
/* CONST_DAT        const               code                                  */
/* RAM_DAT          (blank)             xdata                                 */
/* FUNC_PTR         (blank)             code                                  */
/*============================================================================*/


/*============================================================================*/
/*                       INCLUDE FILES                                        */
/*============================================================================*/
#include "tmNxCompId.h"

#ifdef __cplusplus
extern "C" {
#endif

/*============================================================================*/
/*                       MACRO DEFINITIONS                                    */
/*============================================================================*/

/**
 * The maximum number of supported HDMI Transmitter units
 */
#define HDMITX_UNITS_MAX       2
	
/** \name Errors
 *  The group of error codes returned by all API and internal functions
 */
/*@{*/
/** The base offset for all error codes.
 *  This needs defining as non-zero if this component is integrated with others
 *  and all component error ranges are to be kept separate.
 */
#define TMBSL_ERR_HDMI_BASE	0x00

/** Define the OK code if not defined already */
#ifndef TM_OK
#define TM_OK   0
#endif

/** SW interface compatibility error */
#define TMBSL_ERR_HDMI_COMPATIBILITY            (TMBSL_ERR_HDMI_BASE + 0x001U)

/** SW major version error */
#define TMBSL_ERR_HDMI_MAJOR_VERSION            (TMBSL_ERR_HDMI_BASE + 0x002U)

/** SW component version error */
#define TMBSL_ERR_HDMI_COMP_VERSION             (TMBSL_ERR_HDMI_BASE + 0x003U)

/** Invalid device unit number */
#define TMBSL_ERR_HDMI_BAD_UNIT_NUMBER          (TMBSL_ERR_HDMI_BASE + 0x005U)

/** Invalid input parameter other than unit number */
#define TMBSL_ERR_HDMI_BAD_PARAMETER            (TMBSL_ERR_HDMI_BASE + 0x009U)

/** Inconsistent input parameters */
#define TMBSL_ERR_HDMI_INCONSISTENT_PARAMS      (TMBSL_ERR_HDMI_BASE + 0x010U)

/** Component is not initialized */
#define TMBSL_ERR_HDMI_NOT_INITIALIZED          (TMBSL_ERR_HDMI_BASE + 0x011U)

/** Command not supported for current device */
#define TMBSL_ERR_HDMI_NOT_SUPPORTED            (TMBSL_ERR_HDMI_BASE + 0x013U)

/** Initialization failed */
#define TMBSL_ERR_HDMI_INIT_FAILED              (TMBSL_ERR_HDMI_BASE + 0x014U)

/** Component is busy and cannot do a new operation */
#define TMBSL_ERR_HDMI_BUSY                     (TMBSL_ERR_HDMI_BASE + 0x015U)

/** I2C read error */
#define TMBSL_ERR_HDMI_I2C_READ                 (TMBSL_ERR_HDMI_BASE + 0x017U)

/** I2C write error */
#define TMBSL_ERR_HDMI_I2C_WRITE                (TMBSL_ERR_HDMI_BASE + 0x018U)

/** Assertion failure */
#define TMBSL_ERR_HDMI_ASSERTION                (TMBSL_ERR_HDMI_BASE + 0x049U)

/** Bad EDID block checksum */
#define TMBSL_ERR_HDMI_INVALID_STATE            (TMBSL_ERR_HDMI_BASE + 0x066U)
#define TMBSL_ERR_HDMI_INVALID_CHECKSUM         TMBSL_ERR_HDMI_INVALID_STATE

/** No connection to HPD pin */
#define TMBSL_ERR_HDMI_NULL_CONNECTION          (TMBSL_ERR_HDMI_BASE + 0x067U)

/** Not allowed in DVI mode */
#define TMBSL_ERR_HDMI_OPERATION_NOT_PERMITTED  (TMBSL_ERR_HDMI_BASE + 0x068U)

/* Maximum error code defined */
#define TMBSL_ERR_HDMI_MAX              TMBSL_ERR_HDMI_OPERATION_NOT_PERMITTED

/*@}*/


/** size descriptor block of monitor descriptor */
#define EDID_MONITOR_DESCRIPTOR_SIZE   13

#define NUMBER_DTD_STORED       10



/*============================================================================*/
/*                       ENUM OR TYPE DEFINITIONS                             */
/*============================================================================*/


/** Sink category */
typedef enum
{
    HDMITX_SINK_CAT_NOT_REPEATER    = 0, /**< Not repeater  */
    HDMITX_SINK_CAT_REPEATER        = 1, /**< repeater      */   
    HDMITX_SINK_CAT_INVALID         = 3  /**< Invalid       */
} tmbslHdmiTxSinkCategory_t;

/*============================================================================*/
/**
 * basic display parameters structure
 */
typedef struct _tmbslHdmiTxEdidBDParam_t
{
    UInt8   uVideoInputDef;      /**< Video Input Definition                       */
    UInt8   uMaxHorizontalSize;  /**< Max. Horizontal Image Size in cm             */
    UInt8   uMaxVerticalSize;    /**< Max. Vertical Image Size in cm               */
    UInt8   uGamma;              /**< Gamma                                        */
    UInt8   uFeatureSupport;     /**< Feature support                              */
} tmbslHdmiTxEdidBDParam_t;


/*============================================================================*/
/**
 * First monitor descriptor structure
 */
typedef struct _tmbslHdmiTxEdidFirstMD_t
{   
    Bool    bDescRecord;                                    /**< True when parameters of struct are available   */
    UInt8   uMonitorName[EDID_MONITOR_DESCRIPTOR_SIZE];     /**< Monitor Name                                   */
} tmbslHdmiTxEdidFirstMD_t;

/*============================================================================*/
/**
 * Second monitor descriptor structure
 */
typedef struct _tmbslHdmiTxEdidSecondMD_t
{
    Bool    bDescRecord;                                    /**< True when parameters of struct are available   */
    UInt8   uMinVerticalRate;                               /**< Min vertical rate in Hz                        */
    UInt8   uMaxVerticalRate;                               /**< Max vertical rate in Hz                        */
    UInt8   uMinHorizontalRate;                             /**< Min horizontal rate in Hz                      */
    UInt8   uMaxHorizontalRate;                             /**< Max horizontal rate in Hz                      */
    UInt8   uMaxSupportedPixelClk;                          /**< Max suuported pixel clock rate in MHz          */
} tmbslHdmiTxEdidSecondMD_t;


/*============================================================================*/
/**
 * Other monitor descriptor structure
 */
typedef struct _tmbslHdmiTxEdidOtherMD_t
{
    Bool    bDescRecord;                                    /**< True when parameters of struct are available   */
    UInt8   uOtherDescriptor[EDID_MONITOR_DESCRIPTOR_SIZE]; /**< Other monitor Descriptor                       */
} tmbslHdmiTxEdidOtherMD_t;














/**
 * \brief System function pointer type, to call user I2C read/write functions
 * \param slaveAddr     The I2C slave address
 * \param firstRegister The first device register address to read or write 
 * \param lenData       Length of data to read or write (i.e. no. of registers)
 * \param pData         Pointer to data to write, or to buffer to receive data
 * \return              The call result:
 *                      - TM_OK: the call was successful
 *                      - TMBSL_ERR_HDMI_I2C_WRITE: failed when writing
 *                      - TMBSL_ERR_HDMI_I2C_READ:  failed when reading
 */
typedef struct _tmbslHdmiTxSysArgs_t
{
    UInt8 slaveAddr;
    UInt8 firstRegister;
    UInt8 lenData;
    UInt8 *pData;
} tmbslHdmiTxSysArgs_t;
typedef tmErrorCode_t (FUNC_PTR * ptmbslHdmiTxSysFunc_t)
                                 (tmbslHdmiTxSysArgs_t *pSysArgs);

/**
 * \brief System function pointer type, to call user I2C EDID read function
 * \param segPtrAddr    The EDID segment pointer address 0 to 7Fh
 * \param segPtr        The EDID segment pointer 0 to 7Fh
 * \param dataRegAddr   The EDID data register address 0 to 7Fh
 * \param wordOffset    The first word offset 0 to FFh to read
 * \param lenData       Length of data to read (i.e. number of registers),
                        1 to max starting at wordOffset
 * \param pData         Pointer to buffer to receive lenData data bytes
 * \return              The call result:
 *                      - TM_OK: the call was successful
 *                      - TMBSL_ERR_HDMI_I2C_WRITE: failed when writing
 *                      - TMBSL_ERR_HDMI_I2C_READ:  failed when reading
 */
typedef struct _tmbslHdmiTxSysArgsEdid_t
{
    UInt8 segPtrAddr;
    UInt8 segPtr;
    UInt8 dataRegAddr;
    UInt8 wordOffset;
    UInt8 lenData;
    UInt8 *pData;
} tmbslHdmiTxSysArgsEdid_t;
typedef tmErrorCode_t (FUNC_PTR * ptmbslHdmiTxSysFuncEdid_t)
                                 (tmbslHdmiTxSysArgsEdid_t *pSysArgs);

/*============================================================================*/
/*
 * Timer function pointer type, to call an application timer
 * Parameter ms: Delay in milliseconds required
 */
typedef Void (FUNC_PTR * ptmbslHdmiTxSysFuncTimer_t) (UInt16 ms);

/*============================================================================*/
/*
 * Callback function pointer type, to call a user interrupt handler
 * Parameter txUnit: The transmitter unit that interrupted, 0 to max
 */
typedef Void (FUNC_PTR * ptmbslHdmiTxCallback_t) (tmUnitSelect_t txUnit);

/*============================================================================*/
/**
 * EIA/CEA-861B video format type
 */
typedef enum
{
    HDMITX_VFMT_NULL               = 0,     /**< Not a valid format...        */
    HDMITX_VFMT_NO_CHANGE          = 0,     /**< ...or no change required     */
    HDMITX_VFMT_MIN                = 1,     /**< Lowest valid format          */
    HDMITX_VFMT_TV_MIN             = 1,     /**< Lowest valid TV format       */
    HDMITX_VFMT_01_640x480p_60Hz   = 1,     /**< Format 01 640  x 480p  60Hz  */
    HDMITX_VFMT_02_720x480p_60Hz   = 2,     /**< Format 02 720  x 480p  60Hz  */
    HDMITX_VFMT_03_720x480p_60Hz   = 3,     /**< Format 03 720  x 480p  60Hz  */
    HDMITX_VFMT_04_1280x720p_60Hz  = 4,     /**< Format 04 1280 x 720p  60Hz  */
    HDMITX_VFMT_05_1920x1080i_60Hz = 5,     /**< Format 05 1920 x 1080i 60Hz  */
    HDMITX_VFMT_06_720x480i_60Hz   = 6,     /**< Format 06 720  x 480i  60Hz  */
    HDMITX_VFMT_07_720x480i_60Hz   = 7,     /**< Format 07 720  x 480i  60Hz  */
    HDMITX_VFMT_08_720x240p_60Hz   = 8,     /**< Format 08 720  x 240p  60Hz  */
    HDMITX_VFMT_09_720x240p_60Hz   = 9,     /**< Format 09 720  x 240p  60Hz  */
    HDMITX_VFMT_10_720x480i_60Hz   = 10,    /**< Format 10 720  x 480i  60Hz  */
    HDMITX_VFMT_11_720x480i_60Hz   = 11,    /**< Format 11 720  x 480i  60Hz  */
    HDMITX_VFMT_12_720x240p_60Hz   = 12,    /**< Format 12 720  x 240p  60Hz  */
    HDMITX_VFMT_13_720x240p_60Hz   = 13,    /**< Format 13 720  x 240p  60Hz  */
    HDMITX_VFMT_14_1440x480p_60Hz  = 14,    /**< Format 14 1440 x 480p  60Hz  */
    HDMITX_VFMT_15_1440x480p_60Hz  = 15,    /**< Format 15 1440 x 480p  60Hz  */
    HDMITX_VFMT_16_1920x1080p_60Hz = 16,    /**< Format 16 1920 x 1080p 60Hz  */
    HDMITX_VFMT_17_720x576p_50Hz   = 17,    /**< Format 17 720  x 576p  50Hz  */
    HDMITX_VFMT_18_720x576p_50Hz   = 18,    /**< Format 18 720  x 576p  50Hz  */
    HDMITX_VFMT_19_1280x720p_50Hz  = 19,    /**< Format 19 1280 x 720p  50Hz  */
    HDMITX_VFMT_20_1920x1080i_50Hz = 20,    /**< Format 20 1920 x 1080i 50Hz  */
    HDMITX_VFMT_21_720x576i_50Hz   = 21,    /**< Format 21 720  x 576i  50Hz  */
    HDMITX_VFMT_22_720x576i_50Hz   = 22,    /**< Format 22 720  x 576i  50Hz  */
    HDMITX_VFMT_23_720x288p_50Hz   = 23,    /**< Format 23 720  x 288p  50Hz  */
    HDMITX_VFMT_24_720x288p_50Hz   = 24,    /**< Format 24 720  x 288p  50Hz  */
    HDMITX_VFMT_25_720x576i_50Hz   = 25,    /**< Format 25 720  x 576i  50Hz  */
    HDMITX_VFMT_26_720x576i_50Hz   = 26,    /**< Format 26 720  x 576i  50Hz  */
    HDMITX_VFMT_27_720x288p_50Hz   = 27,    /**< Format 27 720  x 288p  50Hz  */
    HDMITX_VFMT_28_720x288p_50Hz   = 28,    /**< Format 28 720  x 288p  50Hz  */
    HDMITX_VFMT_29_1440x576p_50Hz  = 29,    /**< Format 29 1440 x 576p  50Hz  */
    HDMITX_VFMT_30_1440x576p_50Hz  = 30,    /**< Format 30 1440 x 576p  50Hz  */
    HDMITX_VFMT_31_1920x1080p_50Hz = 31,    /**< Format 31 1920 x 1080p 50Hz  */
	HDMITX_VFMT_32_1920x1080p_24Hz = 32,    /**< Format 32 1920 x 1080p 24Hz  */
	HDMITX_VFMT_33_1920x1080p_25Hz = 33,    /**< Format 33 1920 x 1080p 25Hz  */
	HDMITX_VFMT_34_1920x1080p_30Hz = 34,    /**< Format 34 1920 x 1080p 30Hz  */
    HDMITX_VFMT_TV_MAX             = 34,    /**< Highest valid TV format      */
	HDMITX_VFMT_TV_NO_REG_MIN	   = 32,	/**< Lowest TV format without prefetched table */
    HDMITX_VFMT_TV_NUM             = 35,    /**< Number of TV formats & null  */
    HDMITX_VFMT_PC_MIN             = 128,   /**< Lowest valid PC format       */
    HDMITX_VFMT_PC_640x480p_60Hz   = 128,   /**< PC format 128                */
    HDMITX_VFMT_PC_800x600p_60Hz   = 129,   /**< PC format 129                */
    HDMITX_VFMT_PC_1152x960p_60Hz  = 130,   /**< PC format 130                */
    HDMITX_VFMT_PC_1024x768p_60Hz  = 131,   /**< PC format 131                */
    HDMITX_VFMT_PC_1280x768p_60Hz  = 132,   /**< PC format 132                */
    HDMITX_VFMT_PC_1280x1024p_60Hz = 133,   /**< PC format 133                */
    HDMITX_VFMT_PC_1360x768p_60Hz  = 134,   /**< PC format 134                */
    HDMITX_VFMT_PC_1400x1050p_60Hz  = 135,  /**< PC format 135                */
    HDMITX_VFMT_PC_1600x1200p_60Hz = 136,   /**< PC format 136                */
    HDMITX_VFMT_PC_1024x768p_70Hz  = 137,   /**< PC format 137                */
    HDMITX_VFMT_PC_640x480p_72Hz   = 138,   /**< PC format 138                */
    HDMITX_VFMT_PC_800x600p_72Hz   = 139,   /**< PC format 139                */
    HDMITX_VFMT_PC_640x480p_75Hz   = 140,   /**< PC format 140                */
    HDMITX_VFMT_PC_1024x768p_75Hz  = 141,   /**< PC format 141                */
    HDMITX_VFMT_PC_800x600p_75Hz   = 142,   /**< PC format 142                */
    HDMITX_VFMT_PC_1024x864p_75Hz  = 143,   /**< PC format 143                */
    HDMITX_VFMT_PC_1280x1024p_75Hz = 144,   /**< PC format 144                */
    HDMITX_VFMT_PC_640x350p_85Hz   = 145,   /**< PC format 145                */
    HDMITX_VFMT_PC_640x400p_85Hz   = 146,   /**< PC format 146                */
    HDMITX_VFMT_PC_720x400p_85Hz   = 147,   /**< PC format 147                */
    HDMITX_VFMT_PC_640x480p_85Hz   = 148,   /**< PC format 148                */
    HDMITX_VFMT_PC_800x600p_85Hz   = 149,   /**< PC format 149                */
    HDMITX_VFMT_PC_1024x768p_85Hz  = 150,   /**< PC format 150                */
    HDMITX_VFMT_PC_1152x864p_85Hz  = 151,   /**< PC format 151                */
    HDMITX_VFMT_PC_1280x960p_85Hz  = 152,   /**< PC format 152                */
    HDMITX_VFMT_PC_1280x1024p_85Hz = 153,   /**< PC format 153                */
    HDMITX_VFMT_PC_1024x768i_87Hz  = 154,   /**< PC format 154                */
    HDMITX_VFMT_PC_MAX             = 154,   /**< Highest valid PC format      */
    HDMITX_VFMT_PC_NUM     = (1+154-128)    /**< Number of PC formats         */
} tmbslHdmiTxVidFmt_t;

/*============================================================================*/
/*                       EXTERN DATA DEFINITIONS                              */
/*============================================================================*/

typedef enum
{
     BSLHDMITX_UNKNOWN = 0x00, /**< IC/IP is not recognized */
     BSLHDMITX_TDA9984,        /**< IC is a TDA9984         */
     BSLHDMITX_TDA9989,        /**< IC is a TDA9989         */
     BSLHDMITX_LIPP4200,       /**< IP is a LIPP4200        */
     BSLHDMITX_TDA9981,        /**< IC is a TDA9981         */
     BSLHDMITX_TDA9983         /**< IC is a TDA9983         */
} tmbslHdmiTxVersion_t;


/*============================================================================*/
/*                       EXTERN FUNCTION PROTOTYPES                           */
/*============================================================================*/

/*============================================================================*/
/**
    \brief      Reset the Clock Time Stamp generator in HDMI mode only

    \param[in]  txUnit      Transmitter unit number

    \return     The call result:
                - TM_OK: the call was successful
                - Else a problem has been detected:
                  - TMBSL_ERR_HDMI_BAD_UNIT_NUMBER: bad transmitter unit number
                  - TMBSL_ERR_HDMI_NOT_INITIALIZED: transmitter not initialized
                  - TMBSL_ERR_HDMI_I2C_WRITE: failed when writing the I2C bus
                  - TMBSL_ERR_HDMI_OPERATION_NOT_PERMITTED: in DVI mode
 */
tmErrorCode_t
tmbslHdmiTxAudioInResetCts
(
    tmUnitSelect_t      txUnit
);


/*============================================================================*/
/**
 * tmbslHdmiTxAudioInSetConfig() parameter types
 */
/** Audio input formats */
typedef enum
{
    HDMITX_AFMT_SPDIF      = 0,         /**< SPDIF */
    HDMITX_AFMT_I2S        = 1,         /**< I2S   */
    HDMITX_AFMT_OBA        = 2,         /**< One bit audio */
    HDMITX_AFMT_DSD        = 3,         /**< DSD           */
    HDMITX_AFMT_INVALID    = 4          /**< Invalid format*/
} tmbslHdmiTxaFmt_t;

/** DSD clock polarities */
typedef enum
{
    HDMITX_CLKPOLDSD_ACLK       = 0,    /**< Same as ACLK */
    HDMITX_CLKPOLDSD_NACLK      = 1,    /**< Not ACLK, i.e. inverted */
    HDMITX_CLKPOLDSD_NO_CHANGE  = 2,    /**< No change    */
    HDMITX_CLKPOLDSD_INVALID    = 3     /**< Invalid      */
} tmbslHdmiTxClkPolDsd_t;

/** DSD data swap values */
typedef enum
{
    HDMITX_SWAPDSD_OFF          = 0,    /**< No swap   */
    HDMITX_SWAPDSD_ON           = 1,    /**< Swap      */
    HDMITX_SWAPDSD_NO_CHANGE    = 2,    /**< No change */
    HDMITX_SWAPDSD_INVALID      = 3     /**< Invalid   */
} tmbslHdmiTxSwapDsd_t;

/** I2S and DSD channel allocation values */
enum _tmbslHdmiTxChan
{
    HDMITX_CHAN_MIN        = 0,
    HDMITX_CHAN_MAX        = 31,
    HDMITX_CHAN_NO_CHANGE  = 32,
    HDMITX_CHAN_INVALID    = 33
};

/** Audio layout values */
enum _tmbslHdmiTxLayout
{
    HDMITX_LAYOUT_MIN       = 0,
    HDMITX_LAYOUT_MAX       = 1,
    HDMITX_LAYOUT_NO_CHANGE = 2,
    HDMITX_LAYOUT_INVALID   = 3
};

/** Audio FIFO read latency values */
enum _tmbslHdmiTxlatency_rd
{
    HDMITX_LATENCY_MIN         = 0x000,
    HDMITX_LATENCY_MAX         = 0x0FF,
    HDMITX_LATENCY_NO_CHANGE   = 0x100,
    HDMITX_LATENCY_INVALID     = 0x101
};

/**
    \brief      Set audio input configuration in HDMI mode only

    \param[in]  txUnit      Transmitter unit number
    \param[in]  aFmt        Audio input format
    \param[in]  chanI2s     I2S channel allocation
    \param[in]  chanDsd     DSD channel allocation
    \param[in]  clkPolDsd   DSD clock polarity
    \param[in]  swapDsd     DSD data swap
    \param[in]  layout      Sample layout
    \param[in]  latency_rd  Audio FIFO read latency

    \return     The call result:
                - TM_OK: the call was successful
                - Else a problem has been detected:
                  - TMBSL_ERR_HDMI_BAD_UNIT_NUMBER: bad transmitter unit number
                  - TMBSL_ERR_HDMI_BAD_PARAMETER: a parameter was out of range
                  - TMBSL_ERR_HDMI_NOT_INITIALIZED: transmitter not initialized
                  - TMBSL_ERR_HDMI_I2C_WRITE: failed when writing to the I2C bus
                  - TMBSL_ERR_HDMI_OPERATION_NOT_PERMITTED: in DVI mode
 */
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
    UInt16                   latency_rd
);


/*============================================================================*/
/**
 * tmbslHdmiTxAudioInSetCts() parameter types
 */
/** Clock Time Stamp reference source */
typedef enum
{
    HDMITX_CTSREF_ACLK      = 0,    /**< Clock input pin for I2S       */
    HDMITX_CTSREF_MCLK      = 1,    /**< Clock input pin for EXTREF    */
    HDMITX_CTSREF_FS64SPDIF = 2,    /**< 64xsample rate, for SPDIF     */
    HDMITX_CTSREF_INVALID   = 3     /**< Invalid value                 */
} tmbslHdmiTxctsRef_t;

/** Audio sample rate kHz indexes */
typedef enum
{
    HDMITX_AFS_32k           = 0,     /**< 32kHz    */
    HDMITX_AFS_44_1k         = 1,     /**< 44.1kHz  */
    HDMITX_AFS_48K           = 2,     /**< 48kHz    */
    HDMITX_AFS_88_2K         = 3,     /**< 88.2kHz  */
    HDMITX_AFS_96K           = 4,     /**< 96kHz    */
    HDMITX_AFS_176_4K        = 5,     /**< 176.4kHz */
    HDMITX_AFS_192K          = 6,     /**< 192kHz   */
    HDMITX_AFS_NOT_INDICATED = 7,     /**< Not Indicated (Channel Status) */
    HDMITX_AFS_INVALID       = 7,     /**< Invalid  */
    HDMITX_AFS_NUM           = 7      /**< # rates  */
} tmbslHdmiTxafs_t;

/** Vertical output frequencies */
typedef enum
{
    HDMITX_VFREQ_24Hz      = 0,     /**< 24Hz          */
	HDMITX_VFREQ_25Hz      = 1,     /**< 25Hz          */
	HDMITX_VFREQ_30Hz      = 2,     /**< 30Hz          */
	HDMITX_VFREQ_50Hz      = 3,     /**< 50Hz         0 */
    HDMITX_VFREQ_59Hz      = 4,     /**< 59.94Hz      1 */
    HDMITX_VFREQ_60Hz      = 5,     /**< 60Hz         2 */
#ifndef FORMAT_PC
    HDMITX_VFREQ_INVALID   = 6,     /**< Invalid       */
    HDMITX_VFREQ_NUM       = 6      /**< No. of values */
#else /* FORMAT_PC */
    HDMITX_VFREQ_70Hz      = 6,     /**< 70Hz          */
    HDMITX_VFREQ_72Hz      = 7,     /**< 72Hz          */
    HDMITX_VFREQ_75Hz      = 8,     /**< 75Hz          */
    HDMITX_VFREQ_85Hz      = 9,     /**< 85Hz          */
    HDMITX_VFREQ_87Hz      = 10,     /**< 87Hz          */
    HDMITX_VFREQ_INVALID   = 11,     /**< Invalid       */
    HDMITX_VFREQ_NUM       = 11      /**< No. of values */
#endif /* FORMAT_PC */
} tmbslHdmiTxVfreq_t;

/** Clock Time Stamp predivider - scales N */
typedef enum
{
    HDMITX_CTSK1           = 0,     /**< k=1 */
    HDMITX_CTSK2           = 1,     /**< k=2 */
    HDMITX_CTSK3           = 2,     /**< k=3 */
    HDMITX_CTSK4           = 3,     /**< k=4 */
    HDMITX_CTSK8           = 4,     /**< k=8 */
    HDMITX_CTSK_USE_CTSX   = 5,     /**< Calculate from ctsX factor */
    HDMITX_CTSK_INVALID    = 6      /**< Invalid */
} tmbslHdmiTxctsK_t;

/** Clock Time Stamp postdivider measured time stamp */
typedef enum
{
    HDMITX_CTSMTS          = 0,     /**< =mts   */
    HDMITX_CTSMTS2         = 1,     /**< =mts%2 */
    HDMITX_CTSMTS4         = 2,     /**< =mts%4 */
    HDMITX_CTSMTS8         = 3,     /**< =mts%8 */
    HDMITX_CTSMTS_USE_CTSX = 4,     /**< Calculate from ctsX factor */
    HDMITX_CTSMTS_INVALID  = 5      /**< Invalid */
} tmbslHdmiTxctsM_t;

/** Cycle Time Stamp values */
enum _tmbslHdmiTxCts
{
    HDMITX_CTS_AUTO        = 0,
    HDMITX_CTS_MIN         = 0x000001
};

/** Cycle Time Stamp X factors */ 
enum _tmbslHdmiTxCtsX
{
    HDMITX_CTSX_16         = 0,
    HDMITX_CTSX_32         = 1,
    HDMITX_CTSX_48         = 2,
    HDMITX_CTSX_64         = 3,
    HDMITX_CTSX_128        = 4,
    HDMITX_CTSX_NUM        = 5,
    HDMITX_CTSX_UNUSED     = 5,     /**< CTX value unused when K and Mts used */
    HDMITX_CTSX_INVALID    = 6
};

/**
    \brief      Set the Clock Time Stamp generator in HDMI mode only

    \param[in]  txUnit      Transmitter unit number
    \param[in]  ctsRef      Clock Time Stamp reference source
    \param[in]  afs         Audio input sample frequency
    \param[in]  voutFmt     Video output format
    \param[in]  voutFreq    Vertical output frequency
    \param[in]  uCts        Manual Cycle Time Stamp
    \param[in]  uCtsX       Clock Time Stamp factor x
    \param[in]  ctsK        Clock Time Stamp predivider k
    \param[in]  ctsM        Clock Time Stamp postdivider m

    \return     The call result:
                - TM_OK: the call was successful
                - Else a problem has been detected:
                  - TMBSL_ERR_HDMI_BAD_UNIT_NUMBER: bad transmitter unit number
                  - TMBSL_ERR_HDMI_BAD_PARAMETER: a parameter was out of range
                  - TMBSL_ERR_HDMI_NOT_INITIALIZED: transmitter not initialized
                  - TMBSL_ERR_HDMI_I2C_WRITE: failed when writing to the I2C bus
                  - TMBSL_ERR_HDMI_OPERATION_NOT_PERMITTED: in DVI mode
 */
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
);


/*============================================================================*/
/**
 * tmbslHdmiTxAudioOutSetChanStatus() parameter types
 */
/** BYTE 0: Channel Status Format information */
typedef enum
{
    HDMITX_CSFI_PCM_2CHAN_NO_PRE    = 0,  /**< PCM 2 channels without pre-emphasis           */
    HDMITX_CSFI_PCM_2CHAN_PRE       = 1,  /**< PCM 2 channels with 50us/15us pre-emphasis    */
    HDMITX_CSFI_PCM_2CHAN_PRE_RSVD1 = 2,  /**< PCM Reserved for 2 channels with pre-emphasis */
    HDMITX_CSFI_PCM_2CHAN_PRE_RSVD2 = 3,  /**< PCM Reserved for 2 channels with pre-emphasis */
    HDMITX_CSFI_NOTPCM_DEFAULT      = 4,  /**< Non-PCM Default state                         */
    HDMITX_CSFI_INVALID             = 5   /**< Invalid value                                 */
} tmbslHdmiTxCSformatInfo_t;

/** BYTE 0: Channel Status Copyright assertion */
typedef enum
{
    HDMITX_CSCOPYRIGHT_PROTECTED   = 0,  /**< Copyright protected     */
    HDMITX_CSCOPYRIGHT_UNPROTECTED = 1,  /**< Not copyright protected */
    HDMITX_CSCOPYRIGHT_INVALID     = 2   /**< Invalid value           */
} tmbslHdmiTxCScopyright_t;

/** BYTE 3: Channel Status Clock Accuracy */
typedef enum
{
    HDMITX_CSCLK_LEVEL_II    = 0,     /**< Level II                     */
    HDMITX_CSCLK_LEVEL_I     = 1,     /**< Level I                      */
    HDMITX_CSCLK_LEVEL_III   = 2,     /**< Level III                    */
    HDMITX_CSCLK_NOT_MATCHED = 3,     /**< Not matched to sample freq.  */
    HDMITX_CSCLK_INVALID     = 4      /**< Invalid                      */
} tmbslHdmiTxCSclkAcc_t;

/** BYTE 4: Channel Status Maximum sample word length */
typedef enum
{
    HDMITX_CSMAX_LENGTH_20   = 0,  /**< Max word length is 20 bits   */
    HDMITX_CSMAX_LENGTH_24   = 1,  /**< Max word length is 24 bits   */
    HDMITX_CSMAX_INVALID     = 2   /**< Invalid value                */
} tmbslHdmiTxCSmaxWordLength_t;


/** BYTE 4: Channel Status Sample word length */
typedef enum
{
    HDMITX_CSWORD_DEFAULT          = 0,     /**< Word length is not indicated                    */
    HDMITX_CSWORD_20_OF_24         = 1,     /**< Sample length is 20 bits out of max 24 possible */
    HDMITX_CSWORD_16_OF_20         = 1,     /**< Sample length is 16 bits out of max 20 possible */
    HDMITX_CSWORD_22_OF_24         = 2,     /**< Sample length is 22 bits out of max 24 possible */
    HDMITX_CSWORD_18_OF_20         = 2,     /**< Sample length is 18 bits out of max 20 possible */
    HDMITX_CSWORD_RESVD            = 3,     /**< Reserved - shall not be used */
    HDMITX_CSWORD_23_OF_24         = 4,     /**< Sample length is 23 bits out of max 24 possible */
    HDMITX_CSWORD_19_OF_20         = 4,     /**< Sample length is 19 bits out of max 20 possible */
    HDMITX_CSWORD_24_OF_24         = 5,     /**< Sample length is 24 bits out of max 24 possible */
    HDMITX_CSWORD_20_OF_20         = 5,     /**< Sample length is 20 bits out of max 20 possible */
    HDMITX_CSWORD_21_OF_24         = 6,     /**< Sample length is 21 bits out of max 24 possible */
    HDMITX_CSWORD_17_OF_20         = 6,     /**< Sample length is 17 bits out of max 20 possible */
    HDMITX_CSWORD_INVALID          = 7      /**< Invalid */
} tmbslHdmiTxCSwordLength_t;

/** BYTE 4: Channel Status Original sample frequency */
typedef enum
{
    HDMITX_CSOFREQ_NOT_INDICATED = 0,   /**< Not Indicated */
    HDMITX_CSOFREQ_192k          = 1,   /**< 192kHz        */
    HDMITX_CSOFREQ_12k           = 2,   /**< 12kHz         */
    HDMITX_CSOFREQ_176_4k        = 3,   /**< 176.4kHz      */
    HDMITX_CSOFREQ_RSVD1         = 4,   /**< Reserved      */
    HDMITX_CSOFREQ_96k           = 5,   /**< 96kHz         */
    HDMITX_CSOFREQ_8k            = 6,   /**< 8kHz          */
    HDMITX_CSOFREQ_88_2k         = 7,   /**< 88.2kHz       */
    HDMITX_CSOFREQ_16k           = 8,   /**< 16kHz         */
    HDMITX_CSOFREQ_24k           = 9,   /**< 24kHz         */
    HDMITX_CSOFREQ_11_025k       = 10,  /**< 11.025kHz     */
    HDMITX_CSOFREQ_22_05k        = 11,  /**< 22.05kHz      */
    HDMITX_CSOFREQ_32k           = 12,  /**< 32kHz         */
    HDMITX_CSOFREQ_48k           = 13,  /**< 48kHz         */
    HDMITX_CSOFREQ_RSVD2         = 14,  /**< Reserved      */
    HDMITX_CSOFREQ_44_1k         = 15,  /**< 44.1kHz       */
    HDMITX_CSAFS_INVALID         = 16   /**< Invalid value */
} tmbslHdmiTxCSorigAfs_t;

/**
    \brief      Set the Channel Status Bytes 0,1,3 & 4

    \param[in]  txUnit              Transmitter unit number
    \param[in]  copyright           Byte 0 Copyright bit (bit2)
    \param[in]  formatInfo          Byte 0 Audio sample format (bit1) and additional info (bit345)
    \param[in]  categoryCode        Byte 1 Category code (bits8-15)
    \param[in]  sampleFreq          Byte 3 Sample Frequency (bits24-27)
    \param[in]  clockAccuracy       Byte 3 Clock Accuracy (bits38-31)
    \param[in]  maxWordLength       Byte 4 Maximum word length (bit32)
    \param[in]  wordLength          Byte 4 Word length (bits33-35)
    \param[in]  origSampleFreq      Byte 4 Original Sample Frequency (bits36-39)

    \return     The call result:
                - TM_OK: the call was successful
                - Else a problem has been detected:
                  - TMBSL_ERR_HDMI_BAD_UNIT_NUMBER: bad transmitter unit number
                  - TMBSL_ERR_HDMI_BAD_PARAMETER: a parameter was out of range
                  - TMBSL_ERR_HDMI_NOT_INITIALIZED: transmitter not initialized
                  - TMBSL_ERR_HDMI_I2C_WRITE: failed when writing to the I2C bus
                  - TMBSL_ERR_HDMI_OPERATION_NOT_PERMITTED: in DVI mode

    \note       The consumer use bit (bit0) and Mode bits (bits6-7) are forced to zero.
                Use tmbslHdmiTxAudioOutSetChanStatusMapping to set CS Byte 2.

 */
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
);


/*============================================================================*/
/**
 * tmbslHdmiTxAudioOutSetChanStatusMapping() parameter types
 */
/** Channel Status source/channel number limits */
enum _tmbslHdmiTxChanStatusChanLimits
{
    HDMITX_CS_CHANNELS_MAX  = 0x0F,
    HDMITX_CS_SOURCES_MAX   = 0x0F
};

/**
    \brief      Set the Channel Status Byte2 for Audio Port 0

    \param[in]  txUnit              Transmitter unit number
    \param[in]  sourceLeft          L Source Number: 0 don't take into account, 1-15
    \param[in]  channelLeft         L Channel Number: 0 don't take into account, 1-15
    \param[in]  sourceRight         R Source Number: 0 don't take into account, 1-15
    \param[in]  channelRight        R Channel Number: 0 don't take into account, 1-15
    \return     The call result:
                - TM_OK: the call was successful
                - Else a problem has been detected:
                  - TMBSL_ERR_HDMI_BAD_UNIT_NUMBER: bad transmitter unit number
                  - TMBSL_ERR_HDMI_BAD_PARAMETER: a parameter was out of range
                  - TMBSL_ERR_HDMI_NOT_INITIALIZED: transmitter not initialized
                  - TMBSL_ERR_HDMI_I2C_WRITE: failed when writing to the I2C bus
                  - TMBSL_ERR_HDMI_OPERATION_NOT_PERMITTED: in DVI mode

    \note       Use tmbslHdmiTxAudioOutSetChanStatus to set all other CS bytes
                This function only sets the mapping for Audio Port 0.

 */
tmErrorCode_t
tmbslHdmiTxAudioOutSetChanStatusMapping
(
    tmUnitSelect_t  txUnit,
    UInt8           sourceLeft,
    UInt8           channelLeft,
    UInt8           sourceRight,
    UInt8           channelRight
);


/*============================================================================*/
/**
 * tmbslHdmiTxAudioOutSetMute() parameter type
 */
typedef enum
{
    HDMITX_AMUTE_OFF       = 0,    /**< Mute off */
    HDMITX_AMUTE_ON        = 1,    /**< Mute on  */
    HDMITX_AMUTE_INVALID   = 2     /**< Invalid  */
} tmbslHdmiTxaMute_t;

/**
    \brief      Mute or un-mute the audio output by controlling the audio FIFO,
                in HDMI mode only

    \param[in]  txUnit      Transmitter unit number
    \param[in]  aMute       Audio mute: On, Off

    \return     The call result:
                - TM_OK: the call was successful
                - Else a problem has been detected:
                  - TMBSL_ERR_HDMI_BAD_UNIT_NUMBER: bad transmitter unit number
                  - TMBSL_ERR_HDMI_BAD_PARAMETER: a parameter was out of range
                  - TMBSL_ERR_HDMI_NOT_INITIALIZED: transmitter not initialized
                  - TMBSL_ERR_HDMI_I2C_WRITE: failed when writing to the I2C bus
                  - TMBSL_ERR_HDMI_OPERATION_NOT_PERMITTED: in DVI mode

    \note       tmbslHdmiTxPktSetGeneralCntrl must be used to control the audio
                mute in outgoing data island packets

 */
tmErrorCode_t
tmbslHdmiTxAudioOutSetMute
(
    tmUnitSelect_t      txUnit,
    tmbslHdmiTxaMute_t aMute
);


/*============================================================================*/
/**
    \brief      Disable an HDMI Transmitter output and destroy its driver 
                instance

    \param[in]  txUnit Transmitter unit number

    \return     The call result:
                - TM_OK: the call was successful
                - Else a problem has been detected:
                  - TMBSL_ERR_HDMI_BAD_UNIT_NUMBER: bad transmitter unit number 
                  - TMBSL_ERR_HDMI_NOT_INITIALIZED: the unit is not initialized
                  - TMBSL_ERR_HDMI_I2C_WRITE: failed when writing to the I2C bus
 */
tmErrorCode_t 
tmbslHdmiTxDeinit
(
    tmUnitSelect_t      txUnit
);


/*============================================================================*/
/**
 * \brief The tmbslHdmiTxEdidGetAudioCapabilities() parameter type
 */
typedef struct _tmbslHdmiTxEdidSad_t
{
    UInt8   ModeChans;  /* Bits[6:3]: EIA/CEA861 mode; Bits[2:0]: channels */
    UInt8   Freqs;      /* Bits for each supported frequency */
    UInt8   Byte3;      /* EIA/CEA861B p83: data depending on audio mode */
}tmbslHdmiTxEdidSad_t;

/** Number of 3 byte Short Audio Descriptors stored in pEdidAFmts */
#define HDMI_TX_SAD_MAX_CNT     10

/**
    \brief      Get supported audio format(s) from previously read EDID

    \param[in]  txUnit      Transmitter unit number
    \param[out] pEdidAFmts  Pointer to the array of structures to receive the 
                            supported Short Audio Descriptors
    \param[in]  aFmtLength  Number of SADs supported in buffer pEdidAFmts,
                            up to HDMI_TX_SAD_MAX_CNT
    \param[out] pAFmtsAvail Pointer to receive the number of SADs available
    \param[out] pAudioFlags Pointer to the byte to receive the Audio Capability
                            Flags

    \return     The call result:
                - TM_OK: the call was successful
                - Else a problem has been detected:
                  - TMBSL_ERR_HDMI_BAD_PARAMETER: a parameter was out of range
                  - TMBSL_ERR_HDMI_BAD_UNIT_NUMBER: bad transmitter unit number
                  - TMBSL_ERR_HDMI_I2C_READ: failed when reading the I2C bus
                  - TMBSL_ERR_HDMI_INVALID_STATE: EDID checksum failure
                  - TMBSL_ERR_HDMI_NOT_INITIALIZED: transmitter not initialized
                  - TMBSL_ERR_HDMI_NULL_CONNECTION: HPD pin is inactive
    
    \note \verbatim
                Supported Short Audio Descriptors array:
                EdidAFmts[0].ModeChans      SAD 1  - Mode byte
                EdidAFmts[0].Freqs	        SAD 1  - Frequencies byte
                EdidAFmts[0].Byte3          SAD 1  - Byte 3
                ...
                EdidAFmts[n-1].ModeChans    SAD n  - Mode byte
                EdidAFmts[n-1].Freqs	    SAD n  - Frequencies byte
                EdidAFmts[n-1].Byte3        SAD n  - Byte 3
                (Where n is the smaller of aFmtLength and pAFmtAvail)
    \endverbatim
 */
tmErrorCode_t
tmbslHdmiTxEdidGetAudioCapabilities
(
    tmUnitSelect_t          txUnit,
    tmbslHdmiTxEdidSad_t    *pEdidAFmts,
    UInt                    aFmtLength,
    UInt                    *pAFmtsAvail,
    UInt8                   *pAudioFlags
);

/*============================================================================*/
/**
    \brief      Get the EDID block count

    \param[in]  txUnit              Transmitter unit number
    \param[out] puEdidBlockCount    Pointer to data byte in which to return the
                                    block count

    \return     The call result:
                - TM_OK: the call was successful
                - Else a problem has been detected:
                  - TMBSL_ERR_HDMI_BAD_PARAMETER: a parameter was out of range
                  - TMBSL_ERR_HDMI_BAD_UNIT_NUMBER: bad transmitter unit number
                  - TMBSL_ERR_HDMI_I2C_READ: failed when reading the I2C bus
                  - TMBSL_ERR_HDMI_INVALID_STATE: EDID checksum failure
                  - TMBSL_ERR_HDMI_NOT_INITIALIZED: transmitter not initialized
                  - TMBSL_ERR_HDMI_NULL_CONNECTION: HPD pin is inactive
 */
tmErrorCode_t
tmbslHdmiTxEdidGetBlockCount
(
    tmUnitSelect_t  txUnit,
    UInt8           *puEdidBlockCount
);

/*============================================================================*/
/**
 * tmbslHdmiTxEdidGetBlockData() parameter types
 */
/** An enum to represent the current EDID status */
enum _tmbslHdmiTxEdidSta_t
{
    HDMITX_EDID_READ                = 0,    /* All blocks read OK */
    HDMITX_EDID_READ_INCOMPLETE     = 1,    /* All blocks read OK but buffer too
                                               small to return all of them */
    HDMITX_EDID_ERROR               = 2,    /* Block 0 OK, checksum error in one
                                               or more other blocks */
    HDMITX_EDID_ERROR_INCOMPLETE    = 3,    /* Block 0 OK, checksum error in one
                                               or more other blocks and buffer
                                               too small to return all of them*/
    HDMITX_EDID_NOT_READ            = 4     /* EDID not read or checksum error
                                               in block 0 */
};
 
/**
    \brief      Get raw EDID blocks from the sink device via DDC

    \param[in]  txUnit      Transmitter unit number
    \param[out] pRawEdid    Pointer to a buffer supplied by the caller to accept
                            the raw EDID data
    \param[in]  numBlocks   Number of blocks to read
    \param[in]  lenRawEdid  Length in bytes of the supplied buffer
    \param[out] pEdidStatus Pointer to status value E_EDID_READ or E_EDID_ERROR
                            valid only when the return value is TM_OK

    \return     The call result:
                - TM_OK: the call was successful
                - Else a problem has been detected:
                  - TMBSL_ERR_HDMI_BAD_PARAMETER: a parameter was out of range
                  - TMBSL_ERR_HDMI_BAD_UNIT_NUMBER: bad transmitter unit number
                  - TMBSL_ERR_HDMI_I2C_READ: failed when reading the I2C bus
                  - TMBSL_ERR_HDMI_INVALID_STATE: EDID checksum failure
                  - TMBSL_ERR_HDMI_NOT_INITIALIZED: transmitter not initialized
                  - TMBSL_ERR_HDMI_NULL_CONNECTION: HPD pin is inactive

    \note       NA

    \sa         tmbslHdmiTxEdidGetVideoPreferred, 
                tmbslHdmiTxEdidGetVideoCapabilities
 */
tmErrorCode_t
tmbslHdmiTxEdidGetBlockData
(
    tmUnitSelect_t          txUnit,
    UInt8                   *pRawEdid, 
    Int                     numBlocks,
    Int                     lenRawEdid,
    UInt8                   *pEdidStatus
);

/*============================================================================*/
/**
 * tmbslHdmiTxEdidGetSinkType() parameter types
 */
/** Sink device type */
typedef enum
{
    HDMITX_SINK_DVI                = 0,    /**< DVI  */
    HDMITX_SINK_HDMI               = 1,    /**< HDMI */
    HDMITX_SINK_EDID               = 2,    /**< As currently defined in EDID */
    HDMITX_SINK_INVALID            = 3     /**< Invalid   */
} tmbslHdmiTxSinkType_t;

/**
    \brief      Get Sink Type by analysis of EDID content

    \param[in]  txUnit      Transmitter unit number
    \param[out] pSinkType   Pointer to returned Sink Type: DVI or HDMI

    \return     The call result:
                - TM_OK: the call was successful
                - Else a problem has been detected:
                  - TMBSL_ERR_HDMI_BAD_PARAMETER: a parameter was out of range
                  - TMBSL_ERR_HDMI_BAD_UNIT_NUMBER: bad transmitter unit number
                  - TMBSL_ERR_HDMI_I2C_READ: failed when reading the I2C bus
                  - TMBSL_ERR_HDMI_INVALID_STATE: EDID checksum failure
                  - TMBSL_ERR_HDMI_NOT_INITIALIZED: transmitter not initialized
                  - TMBSL_ERR_HDMI_NULL_CONNECTION: HPD pin is inactive
    
    \sa         tmbslHdmiTxEdidGetBlockData
 */
tmErrorCode_t
tmbslHdmiTxEdidGetSinkType
(
    tmUnitSelect_t              txUnit,
    tmbslHdmiTxSinkType_t      *pSinkType 
);

/*============================================================================*/
/**
    \brief      Get Source Physical Address by analysis of EDID content

    \param[in]  txUnit          Transmitter unit number
    \param[out] pSourceAddress  Pointer to returned Source Physical Address (ABCDh)

    \return     The call result:
                - TM_OK: the call was successful
                - Else a problem has been detected:
                  - TMBSL_ERR_HDMI_BAD_PARAMETER: a parameter was out of range
                  - TMBSL_ERR_HDMI_BAD_UNIT_NUMBER: bad transmitter unit number
                  - TMBSL_ERR_HDMI_I2C_READ: failed when reading the I2C bus
                  - TMBSL_ERR_HDMI_INVALID_STATE: EDID checksum failure
                  - TMBSL_ERR_HDMI_NOT_INITIALIZED: transmitter not initialized
                  - TMBSL_ERR_HDMI_NULL_CONNECTION: HPD pin is inactive
    
    \sa         tmbslHdmiTxEdidGetBlockData
 */
tmErrorCode_t
tmbslHdmiTxEdidGetSourceAddress
(
    tmUnitSelect_t  txUnit,
    UInt16          *pSourceAddress 
);

/*============================================================================*/
/**
 *  tmbslHdmiTxEdidGetVideoCapabilities() parameter types
 */
/** Number of 1 byte Short Video Descriptors stored in pEdidVFmts */
#define HDMI_TX_SVD_MAX_CNT     30

/** Flag set in Short Video Descriptor to indicate native format */
#define HDMI_TX_SVD_NATIVE_MASK 0x80
#define HDMI_TX_SVD_NATIVE_NOT  0x7F

/** Video capability flags */
enum _tmbslHdmiTxVidCap_t
{
    HDMITX_VIDCAP_UNDERSCAN   = 0x80,   /**< Underscan supported */
    HDMITX_VIDCAP_YUV444      = 0x40,   /**< YCbCr 4:4:4 supported */
    HDMITX_VIDCAP_YUV422      = 0x20,   /**< YCbCr 4:2:2 supported */
    HDMITX_VIDCAP_UNUSED      = 0x1F    /**< Unused flags */
};

/**
    \brief      Get supported video format(s) from previously read EDID

    \param[in]  txUnit      Transmitter unit number
    \param[out] pEdidVFmts  Pointer to the array to receive the supported Short
                            Video Descriptors
    \param[in]  vFmtLength  Number of SVDs supported in buffer pEdidVFmts,
                            up to HDMI_TX_SVD_MAX_CNT
    \param[out] pVFmtsAvail Pointer to receive the number of SVDs available
    \param[out] pVidFlags   Ptr to the byte to receive Video Capability Flags
                            b7: underscan supported
                            b6: YCbCr 4:4:4 supported
                            b5: YCbCr 4:2:2 supported

    \return     The call result:
                - TM_OK: the call was successful
                - Else a problem has been detected:
                  - TMBSL_ERR_HDMI_BAD_PARAMETER: a parameter was out of range
                  - TMBSL_ERR_HDMI_BAD_UNIT_NUMBER: bad transmitter unit number
                  - TMBSL_ERR_HDMI_I2C_READ: failed when reading the I2C bus
                  - TMBSL_ERR_HDMI_INVALID_STATE: EDID checksum failure
                  - TMBSL_ERR_HDMI_NOT_INITIALIZED: transmitter not initialized
                  - TMBSL_ERR_HDMI_NULL_CONNECTION: HPD pin is inactive
    
    \note \verbatim
                Supported Short Video Descriptors array:
                    (HDMI_TX_SVD_NATIVE_MASK bit set to indicate native format)
                EdidVFmts[0]   EIA/CEA Short Video Descriptor 1, or 0
                ...
                EdidVFmts[n-1]  EIA/CEA Short Video Descriptor 32, or 0
                (Where n is the smaller of vFmtLength and pVFmtAvail)
    \endverbatim
    \sa         tmbslHdmiTxEdidGetBlockData
 */
tmErrorCode_t
tmbslHdmiTxEdidGetVideoCapabilities
(
    tmUnitSelect_t  txUnit,
    UInt8           *pEdidVFmts,
    UInt            vFmtLength,
    UInt            *pVFmtsAvail,
    UInt8           *pVidFlags
);

/*============================================================================*/
/**
 * \brief The tmbslHdmiTxEdidGetVideoPreferred() parameter type
 */
typedef struct _tmbslHdmiTxEdidDtd_t
{
    UInt16  uPixelClock;        /**< Pixel Clock/10,000         */
    UInt16  uHActivePixels;     /**< Horizontal Active Pixels   */
    UInt16  uHBlankPixels;      /**< Horizontal Blanking Pixels */
    UInt16  uVActiveLines;      /**< Vertical Active Lines      */
    UInt16  uVBlankLines;       /**< Vertical Blanking Lines    */
    UInt16  uHSyncOffset;       /**< Horizontal Sync Offset     */
    UInt16  uHSyncWidth;        /**< Horiz. Sync Pulse Width    */
    UInt16  uVSyncOffset;       /**< Vertical Sync Offset       */
    UInt16  uVSyncWidth;        /**< Vertical Sync Pulse Width  */
    UInt16  uHImageSize;        /**< Horizontal Image Size      */
    UInt16  uVImageSize;        /**< Vertical Image Size        */
    UInt16  uHBorderPixels;     /**< Horizontal Border          */
    UInt16  uVBorderPixels;     /**< Vertical Border            */
    UInt8   Flags;              /**< Interlace/sync info        */
} tmbslHdmiTxEdidDtd_t;

/**
    \brief      Get preferred video format from previously read EDID

    \param[in]  txUnit      Transmitter unit number
    \param[out] pEdidDTD    Pointer to the structure to receive the Detailed
                            Timing Descriptor parameters of the preferred video
                            format

    \return     The call result:
                - TM_OK: the call was successful
                - Else a problem has been detected:
                  - TMBSL_ERR_HDMI_BAD_PARAMETER: a parameter was out of range
                  - TMBSL_ERR_HDMI_BAD_UNIT_NUMBER: bad transmitter unit number
                  - TMBSL_ERR_HDMI_I2C_READ: failed when reading the I2C bus
                  - TMBSL_ERR_HDMI_INVALID_STATE: EDID checksum failure
                  - TMBSL_ERR_HDMI_NOT_INITIALIZED: transmitter not initialized
                  - TMBSL_ERR_HDMI_NULL_CONNECTION: HPD pin is inactive

    \note \verbatim      
                Detailed Timing Descriptor parameters output structure:
                UInt16 uPixelClock      Pixel Clock (MHz/10,000)
                UInt16 uHActivePixels   Horizontal Active Pixels
                UInt16 uHBlankPixels    Horizontal Blanking Pixels
                UInt16 uVActiveLines    Vertical Active Lines
                UInt16 uVBlankLines     Vertical Blanking Lines
                UInt16 uHSyncOffset     Horizontal Sync Offset (Pixels)
                UInt16 uHSyncWidth      Horizontal Sync Pulse Width (Pixels)
                UInt16 uVSyncOffset     Vertical Sync Offset (Lines)
                UInt16 uVSyncWidth      Vertical Sync Pulse Width (Lines)
                UInt16 uHImageSize      Horizontal Image Size (mm)
                UInt16 uVImageSize      Vertical Image Size (mm)
                UInt16 uHBorderPixels   Horizontal Border (Pixels)
                UInt16 uVborderPixels   Vertical Border (Pixels)
                UInt8 Flags             Interlace/sync info
    \endverbatim
    \sa         tmbslHdmiTxEdidGetBlockData
 */
tmErrorCode_t
tmbslHdmiTxEdidGetVideoPreferred
(
    tmUnitSelect_t          txUnit,
    tmbslHdmiTxEdidDtd_t    *pEdidDTD
);

/**
    \brief      Get detailed timing descriptor from previously read EDID

    \param[in]  txUnit      Transmitter unit number
    \param[out] pEdidDTD    Pointer to the array to receive the Detailed timing descriptor

    \param[in]  nb_size     Number of DTD supported in buffer pEdidDTD

    \param[out] pDTDAvail Pointer to receive the number of DTD available

    \return     The call result:
                - TM_OK: the call was successful
                - Else a problem has been detected:
                  - TMBSL_ERR_HDMI_BAD_PARAMETER: a parameter was out of range
                  - TMBSL_ERR_HDMI_BAD_UNIT_NUMBER: bad transmitter unit number
                  - TMBSL_ERR_HDMI_INVALID_STATE: EDID not read
                  - TMBSL_ERR_HDMI_NOT_INITIALIZED: transmitter not initialized
*/
tmErrorCode_t
tmbslHdmiTxEdidGetDetailedTimingDescriptors
(
    tmUnitSelect_t          txUnit,
    tmbslHdmiTxEdidDtd_t    *pEdidDTD,
    UInt8                   nb_size,
    UInt8                   *pDTDAvail
);



/*============================================================================*/
/**
 * tmbslHdmiTxHdcpCheck() parameter type
 */
/** HDCP check result */
typedef enum
{
    HDMITX_HDCP_CHECK_NOT_STARTED       = 0,    /**< Check not started */
    HDMITX_HDCP_CHECK_IN_PROGRESS       = 1,    /**< No failures, more to do */
    HDMITX_HDCP_CHECK_PASS              = 2,    /**< Final check has passed */
    HDMITX_HDCP_CHECK_FAIL_FIRST        = 3,    /**< First check failure code */
    HDMITX_HDCP_CHECK_FAIL_DRIVER_STATE = 3,    /**< Driver not AUTHENTICATED */
    HDMITX_HDCP_CHECK_FAIL_DEVICE_T0    = 4,    /**< A T0 interrupt occurred */
    HDMITX_HDCP_CHECK_FAIL_DEVICE_RI    = 5,    /**< Device RI changed */
    HDMITX_HDCP_CHECK_FAIL_DEVICE_FSM   = 6,    /**< Device FSM not 10h */
    HDMITX_HDCP_CHECK_NUM               = 7     /**< Number of check results */
}tmbslHdmiTxHdcpCheck_t;

/**
    \brief      Check the result of an HDCP encryption attempt, called at
                intervals (set by uTimeSinceLastCallMs) after tmbslHdmiTxHdcpRun

    \param[in]  txUnit                  Transmitter unit number
    \param[in]  uTimeSinceLastCallMs    Time in ms since this was last called
    \param[out] pResult                 The outcome of the check

    \return     The call result:
                - TM_OK: the call was successful
                - Else a problem has been detected:
                  - TMBSL_ERR_HDMI_BAD_UNIT_NUMBER: bad transmitter unit number
                  - TMBSL_ERR_HDMI_BAD_PARAMETER: a parameter was out of range
                  - TMBSL_ERR_HDMI_NOT_INITIALIZED: transmitter not initialized
                  - TMBSL_ERR_HDMI_I2C_READ: failed when reading the I2C bus
                  - TMBSL_ERR_HDMI_I2C_WRITE: failed when writing the I2C bus
                  - TMBSL_ERR_HDMI_NOT_SUPPORTED: device does not support HDCP
 */
tmErrorCode_t
tmbslHdmiTxHdcpCheck
(
    tmUnitSelect_t	        txUnit,
    UInt16                  uTimeSinceLastCallMs,
    tmbslHdmiTxHdcpCheck_t  *pResult
);

/*============================================================================*/
/**
 * tmbslHdmiTxHdcpConfigure() parameter type
 */
/** HDCP DDC slave addresses */
enum _tmbslHdmiTxHdcpSlaveAddress
{
    HDMITX_HDCP_SLAVE_PRIMARY    = 0x74,
    HDMITX_HDCP_SLAVE_SECONDARY  = 0x76
};

/** HDCP transmitter modes */
typedef enum
{
    HDMITX_HDCP_TXMODE_NOT_SET   = 0,
    HDMITX_HDCP_TXMODE_REPEATER  = 1,
    HDMITX_HDCP_TXMODE_TOP_LEVEL = 2,
    HDMITX_HDCP_TXMODE_MAX       = 2
}tmbslHdmiTxHdcpTxMode_t;

/** HDCP option flags */
typedef enum
{
    HDMITX_HDCP_OPTION_FORCE_PJ_IGNORED = 0x01,/* Not set: obey PJ result     */
    HDMITX_HDCP_OPTION_FORCE_SLOW_DDC   = 0x02,/* Not set: obey BCAPS setting */
    HDMITX_HDCP_OPTION_FORCE_NO_1_1     = 0x04,/* Not set: obey BCAPS setting */
    HDMITX_HDCP_OPTION_FORCE_REPEATER   = 0x08,/* Not set: obey BCAPS setting */
    HDMITX_HDCP_OPTION_FORCE_NO_REPEATER= 0x10,/* Not set: obey BCAPS setting */
    HDMITX_HDCP_OPTION_FORCE_V_EQU_VBAR = 0x20,/* Not set: obey V=V' result   */
    HDMITX_HDCP_OPTION_FORCE_VSLOW_DDC  = 0x40,/* Set: 50kHz DDC */
    HDMITX_HDCP_OPTION_DEFAULT          = 0x00,/* All the above Not Set vals */
    HDMITX_HDCP_OPTION_MASK             = 0x7F,/* Only these bits are allowed */
    HDMITX_HDCP_OPTION_MASK_BAD         = 0x80 /* These bits are not allowed  */
}tmbslHdmiTxHdcpOptions_t;

/**
    \brief      Configure various HDCP parameters

    \param[in]  txUnit          Transmitter unit number
    \param[in]  slaveAddress    DDC I2C slave address
    \param[in]  txMode          Mode of our transmitter device
    \param[in]  options         Options flags to control behaviour of HDCP
    \param[in]  uCheckIntervalMs HDCP check interval in milliseconds
    \param[in]  uChecksToDo     Number of HDCP checks to do after HDCP starts
                                A value of 2 or more is valid for checking
                                May be set to 0 to disabling checking

    \return     The call result:
                - TM_OK: the call was successful
                - Else a problem has been detected:
                  - TMBSL_ERR_HDMI_BAD_UNIT_NUMBER: bad transmitter unit number
                  - TMBSL_ERR_HDMI_BAD_PARAMETER: a parameter was out of range
                  - TMBSL_ERR_HDMI_NOT_INITIALIZED: transmitter not initialized
                  - TMBSL_ERR_HDMI_I2C_READ: failed when reading the I2C bus
                  - TMBSL_ERR_HDMI_I2C_WRITE: failed when writing the I2C bus
                  - TMBSL_ERR_HDMI_NOT_SUPPORTED: device does not support HDCP

    \note       Must be called before all other HDCP APIs
 */
tmErrorCode_t
tmbslHdmiTxHdcpConfigure
(
    tmUnitSelect_t	         txUnit,
    UInt8                    slaveAddress,
    tmbslHdmiTxHdcpTxMode_t  txMode,
    tmbslHdmiTxHdcpOptions_t options,
    UInt16                   uCheckIntervalMs,
    UInt8                    uChecksToDo
);

/*============================================================================*/
/**
 * tmbslHdmiTxHdcpDownloadKeys() parameter type
 */
/** HDCP decryption mode */
typedef enum
{
    HDMITX_HDCP_DECRYPT_DISABLE  = 0,
    HDMITX_HDCP_DECRYPT_ENABLE   = 1,
    HDMITX_HDCP_DECRYPT_MAX      = 1
}tmbslHdmiTxDecrypt_t;

/**
    \brief      Download keys and AKSV data from OTP memory to the device

    \param[in]  txUnit          Transmitter unit number
    \param[in]  seed            Seed value
    \param[in]  keyDecryption   State of key decryption 0 to 1 (disabled, enabled)

    \return     The call result:
                - TM_OK: the call was successful
                - Else a problem has been detected:
                  - TMBSL_ERR_HDMI_BAD_UNIT_NUMBER: bad transmitter unit number
                  - TMBSL_ERR_HDMI_BAD_PARAMETER: a parameter was out of range
                  - TMBSL_ERR_HDMI_NOT_INITIALIZED: transmitter not initialized
                  - TMBSL_ERR_HDMI_I2C_READ: failed when reading the I2C bus
                  - TMBSL_ERR_HDMI_I2C_WRITE: failed when writing the I2C bus
                  - TMBSL_ERR_HDMI_NOT_SUPPORTED: device does not support HDCP
 */
tmErrorCode_t
tmbslHdmiTxHdcpDownloadKeys
(
    tmUnitSelect_t	        txUnit,
    UInt16                  seed,
    tmbslHdmiTxDecrypt_t    keyDecryption
);

/*============================================================================*/
/**
    \brief      Switch HDCP encryption on or off without disturbing Infoframes
                (Not normally used)

    \param[in]  txUnit  Transmitter unit number
    \param[in]  bOn     Encryption state: 1=on, 0=off

    \return     The call result:
                - TM_OK: the call was successful
                - Else a problem has been detected:
                  - TMBSL_ERR_HDMI_BAD_UNIT_NUMBER: bad transmitter unit number
                  - TMBSL_ERR_HDMI_BAD_PARAMETER: a parameter was out of range
                  - TMBSL_ERR_HDMI_NOT_INITIALIZED: transmitter not initialized
                  - TMBSL_ERR_HDMI_I2C_WRITE: failed when writing the I2C bus
                  - TMBSL_ERR_HDMI_NOT_SUPPORTED: device does not support HDCP
*/
tmErrorCode_t
tmbslHdmiTxHdcpEncryptionOn
(
    tmUnitSelect_t	txUnit,
    Bool            bOn
);

/*============================================================================*/
/**
    \brief      Get HDCP OTP registers

    \param[in]  txUnit      Transmitter unit number
    \param[in]  otpAddress  OTP start address 0-FF
    \param[out] pOtpData    Ptr to a three-byte array to hold the data read:
                            [0] = OTP_DATA_MSB
                            [1] = OTP_DATA_ISB
                            [2] = OTP_DATA_LSB

    \return     The call result:
                - TM_OK: the call was successful
                - Else a problem has been detected:
                  - TMBSL_ERR_HDMI_BAD_UNIT_NUMBER: bad transmitter unit number
                  - TMBSL_ERR_HDMI_BAD_PARAMETER: a parameter was out of range
                  - TMBSL_ERR_HDMI_NOT_INITIALIZED: transmitter not initialized
                  - TMBSL_ERR_HDMI_I2C_WRITE: failed when writing the I2C bus
                  - TMBSL_ERR_HDMI_NOT_SUPPORTED: device does not support HDCP
*/
tmErrorCode_t
tmbslHdmiTxHdcpGetOtp
(
    tmUnitSelect_t	        txUnit,
    UInt8                   otpAddress,
    UInt8                   *pOtpData
);

/*============================================================================*/
/**
    \brief      Return the failure state that caused the last T0 interrupt

    \param[in]  txUnit      Transmitter unit number
    \param[out] pFailState  Ptr to the unit's last T0 fail state

    \return     The call result:
                - TM_OK: the call was successful
                - Else a problem has been detected:
                  - TMBSL_ERR_HDMI_BAD_UNIT_NUMBER: bad transmitter unit number
                  - TMBSL_ERR_HDMI_BAD_PARAMETER: a parameter was out of range
                  - TMBSL_ERR_HDMI_NOT_INITIALIZED: transmitter not initialized
                  - TMBSL_ERR_HDMI_NOT_SUPPORTED: device does not support HDCP
*/
tmErrorCode_t
tmbslHdmiTxHdcpGetT0FailState
(
    tmUnitSelect_t	txUnit,
    UInt8           *pFailState
);

/*============================================================================*/
/**
    \brief      Handle BCAPS interrupt

    \param[in]  txUnit  Transmitter unit number

    \return     The call result:
                - TM_OK: the call was successful
                - Else a problem has been detected:
                  - TMBSL_ERR_HDMI_BAD_UNIT_NUMBER: bad transmitter unit number
                  - TMBSL_ERR_HDMI_NOT_INITIALIZED: transmitter not initialized
                  - TMBSL_ERR_HDMI_I2C_READ: failed when reading the I2C bus
                  - TMBSL_ERR_HDMI_I2C_WRITE: failed when writing the I2C bus

    \note       The user BCAPS interrupt handler (registered with 
                tmbslHdmiTxInit) calls this API before calling 
                tmbslHdmiTxHdcpHandleBKSV
*/
tmErrorCode_t
tmbslHdmiTxHdcpHandleBCAPS
(
    tmUnitSelect_t	txUnit
);

/*============================================================================*/
/**
    \brief      Read BKSV registers

    \param[in]  txUnit          Transmitter unit number
    \param[out] pBksv           Pointer to 5-byte BKSV array returned to caller
                                (1st byte is MSB)
    \param[out] pbCheckRequired Pointer to a result variable to tell the caller
                                whether to check for BKSV in a revocation list:
                                0 or 1 (check not required, check required)

    \return     The call result:
                - TM_OK: the call was successful
                - Else a problem has been detected:
                  - TMBSL_ERR_HDMI_BAD_UNIT_NUMBER: bad transmitter unit number
                  - TMBSL_ERR_HDMI_BAD_PARAMETER: a parameter was out of range
                  - TMBSL_ERR_HDMI_NOT_INITIALIZED: transmitter not initialized
                  - TMBSL_ERR_HDMI_I2C_READ: failed when reading the I2C bus
                  - TMBSL_ERR_HDMI_I2C_WRITE: failed when writing the I2C bus

    \note       The user BCAPS interrupt handler (registered with 
                tmbslHdmiTxInit) calls this API after calling 
                tmbslHdmiTxHdcpHandleBCAPS
*/
tmErrorCode_t
tmbslHdmiTxHdcpHandleBKSV
(
    tmUnitSelect_t	txUnit,
    UInt8           *pBksv,
    Bool            *pbCheckRequired  
);

/*============================================================================*/
/**
    \brief      Declare BKSV result to be secure or not secure

    \param[in]  txUnit          Transmitter unit number
    \param[in]  bSecure         Result of user's check of BKSV against a 
                                revocation list:
                                0 (not secure: BKSV found in revocation list)
                                1 (secure: BKSV not found in revocation list)

    \return     The call result:
                - TM_OK: the call was successful
                - Else a problem has been detected:
                  - TMBSL_ERR_HDMI_BAD_UNIT_NUMBER: bad transmitter unit number
                  - TMBSL_ERR_HDMI_BAD_PARAMETER: a parameter was out of range
                  - TMBSL_ERR_HDMI_NOT_INITIALIZED: transmitter not initialized
                  - TMBSL_ERR_HDMI_I2C_WRITE: failed when writing the I2C bus

    \note       The user BCAPS interrupt handler (registered with 
                tmbslHdmiTxInit) calls this API after calling 
                tmbslHdmiTxHdcpHandleBKSV
*/
tmErrorCode_t
tmbslHdmiTxHdcpHandleBKSVResult
(
    tmUnitSelect_t	txUnit,
    Bool            bSecure
);

/*============================================================================*/
/**
 * tmbslHdmiTxHdcpHandleBSTATUS() parameter type
 */
/** BSTATUS bit fields */
enum _tmbslHdmiTxHdcpHandleBSTATUS
{
    HDMITX_HDCP_BSTATUS_HDMI_MODE               = 0x1000,
    HDMITX_HDCP_BSTATUS_MAX_CASCADE_EXCEEDED    = 0x0800,
    HDMITX_HDCP_BSTATUS_CASCADE_DEPTH           = 0x0700,
    HDMITX_HDCP_BSTATUS_MAX_DEVS_EXCEEDED       = 0x0080,
    HDMITX_HDCP_BSTATUS_DEVICE_COUNT            = 0x007F
};

/**
    \brief      Handle BSTATUS interrupt

    \param[in]  txUnit      Transmitter unit number
    \param[out] pBstatus    Pointer to 16-bit BSTATUS value returned to caller

    \return     The call result:
                - TM_OK: the call was successful
                - Else a problem has been detected:
                  - TMBSL_ERR_HDMI_BAD_UNIT_NUMBER: bad transmitter unit number
                  - TMBSL_ERR_HDMI_BAD_PARAMETER: a parameter was out of range
                  - TMBSL_ERR_HDMI_NOT_INITIALIZED: transmitter not initialized
                  - TMBSL_ERR_HDMI_I2C_READ: failed when reading the I2C bus
                  - TMBSL_ERR_HDMI_I2C_WRITE: failed when writing the I2C bus

    \note       Called by user's BSTATUS interrupt handler registered with 
                tmbslHdmiTxInit
*/
tmErrorCode_t
tmbslHdmiTxHdcpHandleBSTATUS
(
    tmUnitSelect_t	txUnit,
    UInt16          *pBstatus
);

/*============================================================================*/
/**
    \brief      Handle ENCRYPT interrupt

    \param[in]  txUnit  Transmitter unit number

    \return     The call result:
                - TM_OK: the call was successful
                - Else a problem has been detected:
                  - TMBSL_ERR_HDMI_BAD_UNIT_NUMBER: bad transmitter unit number
                  - TMBSL_ERR_HDMI_NOT_INITIALIZED: transmitter not initialized
                  - TMBSL_ERR_HDMI_I2C_READ: failed when reading the I2C bus
                  - TMBSL_ERR_HDMI_I2C_WRITE: failed when writing the I2C bus

    \note       Called by user's ENCRYPT interrupt handler registered with 
                tmbslHdmiTxInit
*/
tmErrorCode_t
tmbslHdmiTxHdcpHandleENCRYPT
(
    tmUnitSelect_t	txUnit
);

/*============================================================================*/
/**
    \brief      Handle PJ interrupt

    \param[in]  txUnit  Transmitter unit number

    \return     The call result:
                - TM_OK: the call was successful
                - Else a problem has been detected:
                  - TMBSL_ERR_HDMI_BAD_UNIT_NUMBER: bad transmitter unit number
                  - TMBSL_ERR_HDMI_NOT_INITIALIZED: transmitter not initialized
                  - TMBSL_ERR_HDMI_I2C_READ: failed when reading the I2C bus
                  - TMBSL_ERR_HDMI_I2C_WRITE: failed when writing the I2C bus

    \note       Called by user's PJ interrupt handler registered with 
                tmbslHdmiTxInit
*/
tmErrorCode_t
tmbslHdmiTxHdcpHandlePJ
(
    tmUnitSelect_t	txUnit
);

/*============================================================================*/
/**
 * tmbslHdmiTxHdcpHandleSHA_1() parameter types
 */

/** KSV list sizes */
enum _tmbslHdmiTxHdcpHandleSHA_1
{
    HDMITX_KSV_LIST_MAX_DEVICES = 128,
    HDMITX_KSV_BYTES_PER_DEVICE = 5
};

/**
    \brief      Handle SHA-1 interrupt

    \param[in]  txUnit          Transmitter unit number
    \param[in]  maxKsvDevices   Maximum number of 5-byte devices that will fit
                                in *pKsvList: 0 to 128 devices
                                If 0, no KSV read is done and it is treated as
                                secure
    \param[out] pKsvList        Pointer to KSV list array supplied by caller:
                                Sets of 5-byte KSVs, 1 per device, 1st byte is
                                LSB of 1st device
                                May be null if maxKsvDevices is 0
    \param[out] pnKsvDevices    Pointer to number of KSV devices copied to
                                *pKsvList: 0 to 128
                                If 0, no KSV check is needed and it is treated
                                as secure
                                May be null if maxKsvDevices is 0

    \param[out] pDepth          Connection tree depth

    \return     The call result:
                - TM_OK: the call was successful
                - Else a problem has been detected:
                  - TMBSL_ERR_HDMI_BAD_UNIT_NUMBER: bad transmitter unit number
                  - TMBSL_ERR_HDMI_BAD_PARAMETER: a parameter was out of range
                  - TMBSL_ERR_HDMI_NOT_INITIALIZED: transmitter not initialized
                  - TMBSL_ERR_HDMI_INCONSISTENT_PARAMS: two parameters disagree
                  - TMBSL_ERR_HDMI_I2C_READ: failed when reading the I2C bus
                  - TMBSL_ERR_HDMI_I2C_WRITE: failed when writing the I2C bus

    \note       Called by user's SHA-1 interrupt handler registered with 
                tmbslHdmiTxInit
*/
tmErrorCode_t
tmbslHdmiTxHdcpHandleSHA_1
(
    tmUnitSelect_t	txUnit,
    UInt8           maxKsvDevices,
    UInt8           *pKsvList,
    UInt8           *pnKsvDevices,
    UInt8           *pDepth             /* Connection tree depth returned with KSV list */
);

/*============================================================================*/
/**
    \brief      Declare KSV list result to be secure or not secure

    \param[in]  txUnit          Transmitter unit number
    \param[in]  bSecure         Result of user's check of KSV list against a 
                                revocation list:
                                0 (not secure: one or more KSVs are in r.list)
                                1 (secure: no KSV found in revocation list)

    \return     The call result:
                - TM_OK: the call was successful
                - Else a problem has been detected:
                  - TMBSL_ERR_HDMI_BAD_UNIT_NUMBER: bad transmitter unit number
                  - TMBSL_ERR_HDMI_BAD_PARAMETER: a parameter was out of range
                  - TMBSL_ERR_HDMI_NOT_INITIALIZED: transmitter not initialized
                  - TMBSL_ERR_HDMI_I2C_WRITE: failed when writing the I2C bus

    \note       The user SHA_1 interrupt handler (registered with 
                tmbslHdmiTxInit) calls this API after calling 
                tmbslHdmiTxHdcpHandleSHA_1
*/
tmErrorCode_t
tmbslHdmiTxHdcpHandleSHA_1Result
(
    tmUnitSelect_t	txUnit,
    Bool            bSecure
);

/*============================================================================*/
/**
    \brief      Handle T0 interrupt

    \param[in]  txUnit      Transmitter unit number

    \return     The call result:
                - TM_OK: the call was successful
                - Else a problem has been detected:
                  - TMBSL_ERR_HDMI_BAD_UNIT_NUMBER: bad transmitter unit number
                  - TMBSL_ERR_HDMI_NOT_INITIALIZED: transmitter not initialized
                  - TMBSL_ERR_HDMI_I2C_WRITE: failed when writing the I2C bus

    \note       Called by user's T0 interrupt handler registered with 
                tmbslHdmiTxInit
*/
tmErrorCode_t
tmbslHdmiTxHdcpHandleT0
(
    tmUnitSelect_t	txUnit
);

/*============================================================================*/
/**
    \brief      Prepare for HDCP operation

    \param[in]  txUnit      Transmitter unit number
    \param[in]  voutFmt     Video output format
    \param[in]  voutFreq    Vertical output frequency

    \return     The call result:
                - TM_OK: the call was successful
                - Else a problem has been detected:
                  - TMBSL_ERR_HDMI_BAD_UNIT_NUMBER: bad transmitter unit number
                  - TMBSL_ERR_HDMI_BAD_PARAMETER: a parameter was out of range
                  - TMBSL_ERR_HDMI_NOT_INITIALIZED: transmitter not initialized
                  - TMBSL_ERR_HDMI_I2C_WRITE: failed when writing the I2C bus
                  - TMBSL_ERR_HDMI_NOT_SUPPORTED: device does not support HDCP

    \note       Must be called before tmbslHdmiTxHdcpRun
*/
tmErrorCode_t
tmbslHdmiTxHdcpInit
(
    tmUnitSelect_t	    txUnit,
    tmbslHdmiTxVidFmt_t voutFmt,
    tmbslHdmiTxVfreq_t  voutFreq
);

/*============================================================================*/
/**
    \brief      Start HDCP operation

    \param[in]  txUnit  Transmitter unit number

    \return     The call result:
                - TM_OK: the call was successful
                - Else a problem has been detected:
                  - TMBSL_ERR_HDMI_BAD_UNIT_NUMBER: bad transmitter unit number
                  - TMBSL_ERR_HDMI_BAD_PARAMETER: a parameter was out of range
                  - TMBSL_ERR_HDMI_NOT_INITIALIZED: transmitter not initialized
                  - TMBSL_ERR_HDMI_I2C_WRITE: failed when writing the I2C bus
                  - TMBSL_ERR_HDMI_NOT_SUPPORTED: device does not support HDCP

    \note       Must be called after tmbslHdmiTxHdcpInit
*/
tmErrorCode_t
tmbslHdmiTxHdcpRun
(
    tmUnitSelect_t	txUnit
);

/*============================================================================*/
/**
    \brief      Stop HDCP operation, and cease encrypting the output

    \param[in]  txUnit  Transmitter unit number

    \return     The call result:
                - TM_OK: the call was successful
                - Else a problem has been detected:
                  - TMBSL_ERR_HDMI_BAD_UNIT_NUMBER: bad transmitter unit number
                  - TMBSL_ERR_HDMI_BAD_PARAMETER: a parameter was out of range
                  - TMBSL_ERR_HDMI_NOT_INITIALIZED: transmitter not initialized
                  - TMBSL_ERR_HDMI_I2C_WRITE: failed when writing the I2C bus
                  - TMBSL_ERR_HDMI_NOT_SUPPORTED: device does not support HDCP

    \note       This will trigger an Encrypt interrupt
*/
tmErrorCode_t
tmbslHdmiTxHdcpStop
(
    tmUnitSelect_t	txUnit
);

/*============================================================================*/
/**
 * tmbslHdmiTxHotPlugGetStatus() parameter type
 */
typedef enum
{
    HDMITX_HOTPLUG_INACTIVE    = 0,    /**< Hotplug inactive */
    HDMITX_HOTPLUG_ACTIVE      = 1,    /**< Hotplug active   */
    HDMITX_HOTPLUG_INVALID     = 2     /**< Invalid Hotplug  */
} tmbslHdmiTxHotPlug_t;

/**
    \brief      Get the hot plug input status last read by tmbslHdmiTxInit
                or tmbslHdmiTxHwHandleInterrupt

    \param[in]  txUnit          Transmitter unit number
    \param[out] pHotPlugStatus  Pointer to returned Hot Plug Detect status

    \return     The call result:
                - TM_OK: the call was successful
                - Else a problem has been detected:
                  - TMBSL_ERR_HDMI_BAD_UNIT_NUMBER: bad transmitter unit number
                  - TMBSL_ERR_HDMI_BAD_PARAMETER: a parameter was out of range
                  - TMBSL_ERR_HDMI_NOT_INITIALIZED: transmitter not initialized
 */
tmErrorCode_t
tmbslHdmiTxHotPlugGetStatus
(
    tmUnitSelect_t        txUnit,
    tmbslHdmiTxHotPlug_t *pHotPlugStatus
);

/*============================================================================*/
/**
 * tmbslHdmiTxRxSenseGetStatus() parameter type
 */
typedef enum
{
    HDMITX_RX_SENSE_INACTIVE    = 0,    /**< RxSense inactive */
    HDMITX_RX_SENSE_ACTIVE      = 1,    /**< RxSense active   */
    HDMITX_RX_SENSE_INVALID     = 2     /**< Invalid RxSense  */
} tmbslHdmiTxRxSense_t;

/**
    \brief      Get the rx sense input status last read by tmbslHdmiTxInit
                or tmbslHdmiTxHwHandleInterrupt

    \param[in]  txUnit          Transmitter unit number
    \param[out] pRxSenseStatus  Pointer to returned Rx Sense Detect status

    \return     The call result:
                - TM_OK: the call was successful
                - Else a problem has been detected:
                  - TMBSL_ERR_HDMI_BAD_UNIT_NUMBER: bad transmitter unit number
                  - TMBSL_ERR_HDMI_BAD_PARAMETER: a parameter was out of range
                  - TMBSL_ERR_HDMI_NOT_INITIALIZED: transmitter not initialized
 */
tmErrorCode_t
tmbslHdmiTxRxSenseGetStatus
(
    tmUnitSelect_t        txUnit,
    tmbslHdmiTxRxSense_t *pRxSenseStatus
);

/*============================================================================*/
/**
    \brief      Get one or more hardware I2C register values

    \param[in]  txUnit      Transmitter unit number
    \param[in]  regPage     The device register's page: 00h, 01h, 02h, 11h, 12h
    \param[in]  regAddr     The starting register address on the page: 0 to FFh
    \param[out] pRegBuf     Pointer to buffer to receive the register data
    \param[in]  nRegs       Number of contiguous registers to read: 1 to 254

    \return     The call result:
                - TM_OK: the call was successful
                - Else a problem has been detected:
                  - TMBSL_ERR_HDMI_BAD_UNIT_NUMBER: bad transmitter unit number
                  - TMBSL_ERR_HDMI_BAD_PARAMETER: a parameter was out of range
                  - TMBSL_ERR_HDMI_NOT_INITIALIZED: transmitter not initialized
                  - TMBSL_ERR_HDMI_I2C_READ: failed when reading the I2C bus
                  - TMBSL_ERR_HDMI_I2C_WRITE: failed when writing the I2C bus
 */
tmErrorCode_t
tmbslHdmiTxHwGetRegisters
(
    tmUnitSelect_t	txUnit,
    Int             regPage,
    Int             regAddr,
    UInt8          *pRegBuf,
    Int             nRegs
);

/*============================================================================*/
/**
    \brief      Get the transmitter device version read at initialization

    \param[in]  txUnit          Transmitter unit number
    \param[out] puDeviceVersion Pointer to returned hardware version

    \return     The call result:
                - TM_OK: the call was successful
                - Else a problem has been detected:
                  - TMBSL_ERR_HDMI_BAD_UNIT_NUMBER: bad transmitter unit number
                  - TMBSL_ERR_HDMI_BAD_PARAMETER: a parameter was out of range
                  - TMBSL_ERR_HDMI_NOT_INITIALIZED: transmitter not initialized
 */
tmErrorCode_t
tmbslHdmiTxHwGetVersion
(
    tmUnitSelect_t  txUnit,
    UInt8          *puDeviceVersion
);

/*============================================================================*/
/**
 * tmbslHdmiTxHwGetCapabilities() parameter type
 */
typedef enum
{
    tmbslHdmiTxHwNone       = 0,    /**< None           feature */
    tmbslHdmiTxHwHDCP       = 1,    /**< HDCP           feature */
    tmbslHdmiTxHwScaler     = 2,    /**< Scaler         feature */
    tmbslHdmiTxHwHDCPScaler = 3,    /**< HDCP & Scaler  feature  */
    tmbslHdmiTxHwAll        = 3     /**< All            feature  */
} tmbslHdmiTxHwFeature_t;

/**
    \brief      Get the transmitter device feature read at initialization

    \param[in]  txUnit          Transmitter unit number
    \param[out] pDeviceFeature  Pointer to returned hardware feature

    \return     The call result:
                - TM_OK: the call was successful
                - Else a problem has been detected:
                  - TMBSL_ERR_HDMI_BAD_UNIT_NUMBER: bad transmitter unit number
                  - TMBSL_ERR_HDMI_BAD_PARAMETER: a parameter was out of range
                  - TMBSL_ERR_HDMI_NOT_INITIALIZED: transmitter not initialized
 */
tmErrorCode_t
tmbslHdmiTxHwGetCapabilities
(
    tmUnitSelect_t          txUnit,
    tmbslHdmiTxHwFeature_t  *pDeviceCapabilities
);



/*============================================================================*/
/**
    \brief      Handle all hardware interrupts from a transmitter unit

    \param[in]  txUnit      Transmitter unit number

    \return     The call result:
                - TM_OK: the call was successful
                - Else a problem has been detected:
                  - TMBSL_ERR_HDMI_BAD_UNIT_NUMBER: bad transmitter unit number
                  - TMBSL_ERR_HDMI_BAD_PARAMETER: a parameter was out of range
                  - TMBSL_ERR_HDMI_NOT_INITIALIZED: transmitter not initialized
                  - TMBSL_ERR_HDMI_I2C_READ: failed when reading the I2C bus
    \note       This function must be called at task level not interrupt level,
                as I2C access is required
 */
tmErrorCode_t
tmbslHdmiTxHwHandleInterrupt
(
    tmUnitSelect_t      txUnit
);


/*============================================================================*/
/**
    \brief      Set one or more hardware I2C registers

    \param[in]  txUnit      Transmitter unit number
    \param[in]  regPage     The device register's page: 00h, 01h, 02h, 11h, 12h
    \param[in]  regAddr     The starting register address on the page: 0 to FFh
    \param[in]  pRegBuf     Ptr to buffer from which to write the register data
    \param[in]  nRegs       Number of contiguous registers to write: 0 to 254.
                            The page register (255) may not be written - it is
                            written to automatically here. If nRegs is 0, the
                            page register is the only register written.

    \return     The call result:
                - TM_OK: the call was successful
                - Else a problem has been detected:
                  - TMBSL_ERR_HDMI_BAD_UNIT_NUMBER: bad transmitter unit number
                  - TMBSL_ERR_HDMI_BAD_PARAMETER: a parameter was out of range
                  - TMBSL_ERR_HDMI_NOT_INITIALIZED: transmitter not initialized
                  - TMBSL_ERR_HDMI_I2C_WRITE: failed when writing to the I2C bus
 */
tmErrorCode_t
tmbslHdmiTxHwSetRegisters 
(
    tmUnitSelect_t	txUnit,
    Int             regPage, 
    Int             regAddr, 
    UInt8          *pRegBuf, 
    Int             nRegs
);


/*============================================================================*/
/**
    \brief      Handle hardware startup by resetting Device Instance Data
 */
void
tmbslHdmiTxHwStartup
(
    void
);


/*============================================================================*/
/**
 * tmbslHdmiTxInit() parameter types
 */
/**
 * Supported range of I2C slave addresses
 */
enum _tmbslHdmiTxSlaveAddress
{
    HDMITX_SLAVE_ADDRESS_MIN    = 1,
    HDMITX_SLAVE_ADDRESS_MAX    = 127
};

/**
 * Indexes into the funcCallback[] array of interrupt callback function pointers
 */
enum _tmbslHdmiTxCallbackInt
{
    HDMITX_CALLBACK_INT_ENCRYPT  = 0,  /**< HDCP encryption switched off     */
    HDMITX_CALLBACK_INT_HPD      = 1,  /**< Transition on HPD input          */
    HDMITX_CALLBACK_INT_T0       = 2,  /**< HDCP state machine in state T0   */
    HDMITX_CALLBACK_INT_BCAPS    = 3,  /**< BCAPS available                  */
    HDMITX_CALLBACK_INT_BSTATUS  = 4,  /**< BSTATUS available                */
    HDMITX_CALLBACK_INT_SHA_1    = 5,  /**< sha-1(ksv,bstatus,m0)=V'         */
    HDMITX_CALLBACK_INT_PJ       = 6,  /**< pj=pj' check fails               */
#ifndef TMFL_TDA9981_SUPPORT
    HDMITX_CALLBACK_INT_UNUSED7  = 7,  /**< Unused interrupt                 */
	HDMITX_CALLBACK_INT_NUM      = 8   /**< Number of callbacks              */
#else /* TMFL_TDA9981_SUPPORT */
	HDMITX_CALLBACK_INT_R0	     = 7,  /**< R0 interrupt                     */
	HDMITX_CALLBACK_INT_SW_INT   = 8,  /**< SW DEBUG interrupt               */
#ifdef TMFL_RX_SENSE_ON
	HDMITX_CALLBACK_INT_RX_SENSE = 9,  /**< RX SENSE interrupt               */
	HDMITX_CALLBACK_INT_NUM      = 10   /**< Number of callbacks             */
#else /* TMFL_RX_SENSE_ON */
	HDMITX_CALLBACK_INT_NUM      = 9   /**< Number of callbacks             */
#endif /* TMFL_RX_SENSE_ON */
#endif /* TMFL_TDA9981_SUPPORT */
};

/** Pixel rate */
typedef enum
{
    HDMITX_PIXRATE_DOUBLE               = 0,        /**< Double pixel rate */
    HDMITX_PIXRATE_SINGLE               = 1,        /**< Single pixel rate */
    HDMITX_PIXRATE_SINGLE_REPEATED      = 2,        /**< Single pixel repeated */
    HDMITX_PIXRATE_NO_CHANGE            = 3,        /**< No Change */
    HDMITX_PIXRATE_INVALID              = 4         /**< Invalid   */
} tmbslHdmiTxPixRate_t;

/**
 * \brief The tmbslHdmiTxInit() parameter structure
 */
typedef struct _tmbslHdmiTxCallbackList_t
{
    /** Interrupt callback function pointers (each ptr if null = not used) */
    ptmbslHdmiTxCallback_t funcCallback[HDMITX_CALLBACK_INT_NUM];

} tmbslHdmiTxCallbackList_t;

/**
    \brief      Create an instance of an HDMI Transmitter: initialize the 
                driver, reset the transmitter device and get the current
                Hot Plug state

    \param[in]  txUnit           Transmitter unit number
    \param[in]  uHwAddress       Device I2C slave address
    \param[in]  sysFuncWrite     System function to write I2C
    \param[in]  sysFuncRead      System function to read I2C
    \param[in]  sysFuncEdidRead  System function to read EDID blocks via I2C
    \param[in]  sysFuncTimer     System function to run a timer
    \param[in]  funcIntCallbacks Pointer to interrupt callback function list
                                 The list pointer is null for no callbacks;
                                 each pointer in the list may also be null.
    \param[in]  bEdidAltAddr     Use alternative i2c address for EDID data
                                 register between Driver and TDA9983/2:
                                 0 - use default address (A0)
                                 1 - use alternative address (A2)
    \param[in]  vinFmt           EIA/CEA Video input format: 1 to 31, 0 = No Change
    \param[in]  pixRate          Single data (repeated or not) or double data rate

    \return     The call result:
                - TM_OK: the call was successful
                - Else a problem has been detected:
                  - TMBSL_ERR_HDMI_BAD_UNIT_NUMBER: the unit number is wrong or
                    the transmitter instance is already initialised
                  - TMBSL_ERR_HDMI_BAD_PARAMETER: a parameter is invalid or out 
                    of range
                  - TMBSL_ERR_HDMI_INIT_FAILED: the unit instance is already 
                    initialised
                  - TMBSL_ERR_HDMI_COMPATIBILITY: the driver is not compatiable 
                    with the internal device version code
                  - TMBSL_ERR_HDMI_I2C_WRITE: failed when writing to the I2C bus
                  - TMBSL_ERR_HDMI_I2C_READ: failed when reading the I2C bus
 */
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
);


/*============================================================================*/
/**
 * tmbslHdmiTxMatrixSetCoeffs() parameter type
 */
/** Parameter structure array size */
enum _tmbslHdmiTxMatCoeff
{
    HDMITX_MAT_COEFF_NUM = 9
};

/** \brief The tmbslHdmiTxMatrixSetCoeffs() parameter structure */
typedef struct _tmbslHdmiTxMatCoeff_t
{
    /** Array of coefficients (values -1024 to +1023) */
    Int16 Coeff[HDMITX_MAT_COEFF_NUM];
} tmbslHdmiTxMatCoeff_t;
 
/**
    \brief      Set colour space converter matrix coefficients

    \param[in]  txUnit      Transmitter unit number
    \param[in]  pMatCoeff   Pointer to Matrix Coefficient structure

    \return     The call result:
                - TM_OK: the call was successful
                - Else a problem has been detected:
                  - TMBSL_ERR_HDMI_BAD_UNIT_NUMBER: bad transmitter unit number
                  - TMBSL_ERR_HDMI_BAD_PARAMETER: a parameter was out of range
                  - TMBSL_ERR_HDMI_NOT_INITIALIZED: transmitter not initialized
                  - TMBSL_ERR_HDMI_I2C_WRITE: failed when writing to the I2C bus

    \note       Matrix Coefficient parameter structure:
                Int16 Coeff[9]: Array of coefficients (values -1024 to +1023)
 */
tmErrorCode_t
tmbslHdmiTxMatrixSetCoeffs
(
    tmUnitSelect_t          txUnit,
    tmbslHdmiTxMatCoeff_t   *pMatCoeff
);

/*============================================================================*/
/**
 * tmbslHdmiTxMatrixSetConversion() parameter type
 */
/** Video input mode */
typedef enum
{
    HDMITX_VINMODE_CCIR656     = 0,    /**< ccir656  */
    HDMITX_VINMODE_RGB444      = 1,    /**< RGB444    */
    HDMITX_VINMODE_YUV444      = 2,    /**< YUV444    */
    HDMITX_VINMODE_YUV422      = 3,    /**< YUV422    */
    HDMITX_VINMODE_NO_CHANGE   = 4,    /**< No change */
    HDMITX_VINMODE_INVALID     = 5     /**< Invalid   */
} tmbslHdmiTxVinMode_t;

/** Video output mode */
typedef enum
{
    HDMITX_VOUTMODE_RGB444      = 0,    /**< RGB444    */
    HDMITX_VOUTMODE_YUV422      = 1,    /**< YUV422    */
    HDMITX_VOUTMODE_YUV444      = 2,    /**< YUV444    */
    HDMITX_VOUTMODE_NO_CHANGE   = 3,    /**< No change */
    HDMITX_VOUTMODE_INVALID     = 4     /**< Invalid   */
} tmbslHdmiTxVoutMode_t;

/**
 * \brief Enum defining possible quantization range
 */
typedef enum
{
    HDMITX_VQR_DEFAULT = 0, /* Follow HDMI spec. */
    HDMITX_RGB_FULL    = 1, /* Force RGB FULL , DVI only */
    HDMITX_RGB_LIMITED = 2  /* Force RGB LIMITED , DVI only */
} tmbslHdmiTxVQR_t;

/**
    \brief      Set colour space conversion using preset values

    \param[in]  txUnit          Transmitter unit number
    \param[in]  vinFmt          Input video format
    \param[in]  vinMode         Input video mode
    \param[in]  voutFmt         Output video format
    \param[in]  voutMode        Output video mode

    \return     The call result:
                - TM_OK: the call was successful
                - Else a problem has been detected:
                  - TMBSL_ERR_HDMI_BAD_PARAMETER: a parameter was out of range
                  - TMBSL_ERR_HDMI_BAD_UNIT_NUMBER: bad transmitter unit number
                  - TMBSL_ERR_HDMI_I2C_WRITE: failed when writing to the I2C bus
                  - TMBSL_ERR_HDMI_NOT_INITIALIZED: transmitter not initialized
 */
tmErrorCode_t
tmbslHdmiTxMatrixSetConversion
(
    tmUnitSelect_t          txUnit,
    tmbslHdmiTxVidFmt_t     vinFmt,
    tmbslHdmiTxVinMode_t    vinMode,
    tmbslHdmiTxVidFmt_t     voutFmt,
    tmbslHdmiTxVoutMode_t   voutMode,
    tmbslHdmiTxVQR_t        dviVqr
);

/*============================================================================*/
/**
 * tmbslHdmiTxMatrixSetInputOffset() parameter type
 */
/** Parameter structure array size */
enum _tmbslHdmiTxMatOffset
{
    HDMITX_MAT_OFFSET_NUM = 3
};

/** \brief The tmbslHdmiTxMatrixSetInputOffset() parameter structure */
typedef struct _tmbslHdmiTxMatOffset_t
{
    /** Offset array  (values -1024 to +1023) */
    Int16 Offset[HDMITX_MAT_OFFSET_NUM];
} tmbslHdmiTxMatOffset_t;

/** Matrix numeric limits */
enum _tmbslHdmiTxMatLimits
{
    HDMITX_MAT_OFFSET_MIN      = -1024,
    HDMITX_MAT_OFFSET_MAX      = 1023
};
 
/**
    \brief      Set colour space converter matrix offset at input

    \param[in]  txUnit      Transmitter unit number
    \param[in]  pMatOffset  Pointer to Matrix Offset structure

    \return     The call result:
                - TM_OK: the call was successful
                - Else a problem has been detected:
                  - TMBSL_ERR_HDMI_BAD_PARAMETER: a parameter was out of range
                  - TMBSL_ERR_HDMI_BAD_UNIT_NUMBER: bad transmitter unit number
                  - TMBSL_ERR_HDMI_I2C_WRITE: failed when writing to the I2C bus
                  - TMBSL_ERR_HDMI_NOT_INITIALIZED: transmitter not initialized

    \note       Matrix Offset structure parameter structure:
                Int16 Offset[3]: Offset array (values -1024 to +1023)
 */
tmErrorCode_t
tmbslHdmiTxMatrixSetInputOffset
(
    tmUnitSelect_t          txUnit,
    tmbslHdmiTxMatOffset_t  *pMatOffset
);


/*============================================================================*/
/**
 * tmbslHdmiTxMatrixSetMode() parameter types
 */
/** Matrix  control values */
typedef enum
{
    HDMITX_MCNTRL_ON        = 0,   /**< Matrix on  */
    HDMITX_MCNTRL_OFF       = 1,   /**< Matrix off */
    HDMITX_MCNTRL_NO_CHANGE = 2,   /**< Matrix unchanged */
    HDMITX_MCNTRL_MAX       = 2,   /**< Max value  */
    HDMITX_MCNTRL_INVALID   = 3    /**< Invalid    */
} tmbslHdmiTxmCntrl_t;

/** Matrix  scale values */
typedef enum
{
    HDMITX_MSCALE_256       = 0,   /**< Factor 1/256  */
    HDMITX_MSCALE_512       = 1,   /**< Factor 1/512  */
    HDMITX_MSCALE_1024      = 2,   /**< Factor 1/1024 */
    HDMITX_MSCALE_NO_CHANGE = 3,   /**< Factor unchanged */
    HDMITX_MSCALE_MAX       = 3,   /**< Max value     */
    HDMITX_MSCALE_INVALID   = 4    /**< Invalid value */
} tmbslHdmiTxmScale_t;

/**
    \brief      Set colour space converter matrix mode

    \param[in]  txUnit      Transmitter unit number
    \param[in]  mControl    Matrix Control: On, Off, No change
    \param[in]  mScale      Matrix Scale Factor: 1/256, 1/512, 1/1024, No change

    \return     The call result:
                - TM_OK: the call was successful
                - Else a problem has been detected:
                  - TMBSL_ERR_HDMI_BAD_PARAMETER: a parameter was out of range
                  - TMBSL_ERR_HDMI_BAD_UNIT_NUMBER: bad transmitter unit number
                  - TMBSL_ERR_HDMI_I2C_WRITE: failed when writing to the I2C bus
                  - TMBSL_ERR_HDMI_NOT_INITIALIZED: transmitter not initialized

    \note       NA

    \sa         NA
 */
tmErrorCode_t
tmbslHdmiTxMatrixSetMode
(
    tmUnitSelect_t      txUnit,
    tmbslHdmiTxmCntrl_t mControl,
    tmbslHdmiTxmScale_t mScale
);

/*============================================================================*/
/**
    \brief      Set colour space converter matrix offset at output

    \param[in]  txUnit      Transmitter unit number
    \param[in]  pMatOffset  Pointer to Matrix Offset structure

    \return     The call result:
                - TM_OK: the call was successful
                - Else a problem has been detected:
                  - TMBSL_ERR_HDMI_BAD_PARAMETER: a parameter was out of range
                  - TMBSL_ERR_HDMI_BAD_UNIT_NUMBER: bad transmitter unit number
                  - TMBSL_ERR_HDMI_I2C_WRITE: failed when writing to the I2C bus
                  - TMBSL_ERR_HDMI_NOT_INITIALIZED: transmitter not initialized

    \note       Matrix Offset parameter structure:
                nt16 Offset[3]: Offset array (values -1024 to +1023)
 */
tmErrorCode_t
tmbslHdmiTxMatrixSetOutputOffset
(
    tmUnitSelect_t          txUnit,
    tmbslHdmiTxMatOffset_t  *pMatOffset
);

/*============================================================================*/
/**
    \brief      Enable audio clock recovery packet insertion 

    \param[in]  txUnit      Transmitter unit number
    \param[in]  bEnable     Enable or disable packet insertion

    \return     The call result:
                - TM_OK: the call was successful
                - Else a problem has been detected:
                  - TMBSL_ERR_HDMI_BAD_PARAMETER: a parameter was out of range
                  - TMBSL_ERR_HDMI_BAD_UNIT_NUMBER: bad transmitter unit number
                  - TMBSL_ERR_HDMI_I2C_WRITE: failed when writing to the I2C bus
                  - TMBSL_ERR_HDMI_NOT_INITIALIZED: transmitter not initialized
                  - TMBSL_ERR_HDMI_OPERATION_NOT_PERMITTED: when in DVI mode

    \note       tmbslHdmiTxAudioInSetCts sets CTS and N values
 */
tmErrorCode_t
tmbslHdmiTxPktSetAclkRecovery
(
    tmUnitSelect_t	txUnit,
    Bool            bEnable
);

/*============================================================================*/
/**
 * Data Island Packet structure
 */
/** Parameter structure array sizes */
enum _tmbslHdmiTxPkt
{
    HDMITX_PKT_DATA_BYTE_CNT = 28
};

/** \brief The parameter structure for tmbslHdmiTxPkt*() APIs */
typedef struct _tmbslHdmiTxPkt_t
{
    UInt8 dataByte[HDMITX_PKT_DATA_BYTE_CNT];       /**< Packet Data   */
} tmbslHdmiTxPkt_t;

/**
    \brief      Set audio content protection packet & enable/disable packet 
                insertion

    \param[in]  txUnit      Transmitter unit number
    \param[in]  pPkt        Pointer to Data Island Packet structure
    \param[in]  byteCnt     Packet buffer byte count
    \param[in]  uAcpType    Content protection type
    \param[in]  bEnable     Enable or disable packet insertion

    \return     The call result:
                - TM_OK: the call was successful
                - Else a problem has been detected:
                  - TMBSL_ERR_HDMI_BAD_PARAMETER: a parameter was out of range
                  - TMBSL_ERR_HDMI_BAD_UNIT_NUMBER: bad transmitter unit number
                  - TMBSL_ERR_HDMI_I2C_WRITE: failed when writing to the I2C bus
                  - TMBSL_ERR_HDMI_INCONSISTENT_PARAMS: pointer suppied with byte count of zero
                  - TMBSL_ERR_HDMI_NOT_INITIALIZED: transmitter not initialized
                  - TMBSL_ERR_HDMI_NOT_SUPPORTED: not possible with this device
                  - TMBSL_ERR_HDMI_OPERATION_NOT_PERMITTED: not allowed in DVI mode

    \note       Data Island Packet parameter structure:
                UInt8 dataByte[28]      Packet Data

    \sa         NA
 */
tmErrorCode_t
tmbslHdmiTxPktSetAcp
(
    tmUnitSelect_t      txUnit,
    tmbslHdmiTxPkt_t    *pPkt,
    UInt                byteCnt,
    UInt8               uAcpType,
    Bool                bEnable
);

/*============================================================================*/
/**
 * \brief The Audio Infoframe Parameter structure
 */
typedef struct _tmbslHdmiTxPktAif_t
{
    UInt8 CodingType;       /**< Coding Type 0 to 0Fh */
    UInt8 ChannelCount;     /**< Channel Count 0 to 07h */
    UInt8 SampleFreq;       /**< Sample Frequency 0 to 07h */
    UInt8 SampleSize;       /**< Sample Size 0 to 03h */
    UInt8 ChannelAlloc;     /**< Channel Allocation 0 to FFh */
    Bool  DownMixInhibit;   /**< Downmix inhibit flag 0/1 */
    UInt8 LevelShift;       /**< Level Shift 0 to 0Fh */
} tmbslHdmiTxPktAif_t;

/**
    \brief      Set audio info frame packet & enable/disable packet insertion

    \param[in]  txUnit  Transmitter unit number
    \param[in]  pPkt    Pointer to Audio Infoframe structure
    \param[in]  bEnable Enable or disable packet insertion

    \return     The call result:
                - TM_OK: the call was successful
                - Else a problem has been detected:
                  - TMBSL_ERR_HDMI_BAD_PARAMETER: a parameter was out of range
                  - TMBSL_ERR_HDMI_BAD_UNIT_NUMBER: bad transmitter unit number
                  - TMBSL_ERR_HDMI_I2C_WRITE: failed when writing to the I2C bus
                  - TMBSL_ERR_HDMI_NOT_INITIALIZED: transmitter not initialized
                  - TMBSL_ERR_HDMI_OPERATION_NOT_PERMITTED: not allowed in DVI mode

    \note       Audio Infoframe structure:
                UInt8 CodingType
                UInt8 ChannelCount
                UInt8 SampleFreq
                UInt8 SampleSize
                UInt8 ChannelAlloc
                Bool DownMixInhibit
                UInt8 LevelShift
 */
tmErrorCode_t
tmbslHdmiTxPktSetAudioInfoframe
(
    tmUnitSelect_t      txUnit,
    tmbslHdmiTxPktAif_t *pPkt,
    Bool                bEnable
);

/*============================================================================*/
/**
    \brief      Set contents of general control packet & enable/disable 
                packet insertion

    \param[in]  txUnit      Transmitter unit number
    \param[in]  paMute      Pointer to Audio Mute; if Null, no change to packet
                            contents is made
    \param[in]  bEnable     Enable or disable packet insertion

    \return     The call result:
                - TM_OK: the call was successful
                - Else a problem has been detected:
                  - TMBSL_ERR_HDMI_BAD_PARAMETER: a parameter was out of range
                  - TMBSL_ERR_HDMI_BAD_UNIT_NUMBER: bad transmitter unit number
                  - TMBSL_ERR_HDMI_I2C_WRITE: failed when writing to the I2C bus
                  - TMBSL_ERR_HDMI_NOT_INITIALIZED: transmitter not initialized
                  - TMBSL_ERR_HDMI_OPERATION_NOT_PERMITTED: not allowed in DVI mode

    \note       tmbslHdmiTxAudioOutSetMute must be used to mute the audio output
 */
tmErrorCode_t
tmbslHdmiTxPktSetGeneralCntrl
(
    tmUnitSelect_t      txUnit,
    tmbslHdmiTxaMute_t  *paMute,
    Bool                bEnable
);

/*============================================================================*/
/**
    \brief      Set ISRC1 packet & enable/disable packet insertion

    \param[in]  txUnit      Transmitter unit number
    \param[in]  pPkt        Pointer to Data Island Packet structure
    \param[in]  byteCnt     Packet buffer byte count
    \param[in]  bIsrcCont   ISRC continuation flag
    \param[in]  bIsrcValid  ISRC valid flag
    \param[in]  uIsrcStatus ISRC Status
    \param[in]  bEnable     Enable or disable packet insertion

    \return     The call result:
                - TM_OK: the call was successful
                - Else a problem has been detected:
                  - TMBSL_ERR_HDMI_BAD_PARAMETER: a parameter was out of range
                  - TMBSL_ERR_HDMI_BAD_UNIT_NUMBER: bad transmitter unit number
                  - TMBSL_ERR_HDMI_I2C_WRITE: failed when writing to the I2C bus
                  - TMBSL_ERR_HDMI_INCONSISTENT_PARAMS: pointer suppied with byte count of zero
                  - TMBSL_ERR_HDMI_NOT_INITIALIZED: transmitter not initialized
                  - TMBSL_ERR_HDMI_NOT_SUPPORTED: not possible with this device
                  - TMBSL_ERR_HDMI_OPERATION_NOT_PERMITTED: not allowed in DVI mode

    \note       Data Island Packet parameter structure:
                UInt8 dataByte[28]  Packet Data

    \sa         NA
 */
tmErrorCode_t
tmbslHdmiTxPktSetIsrc1
(
    tmUnitSelect_t      txUnit,
    tmbslHdmiTxPkt_t    *pPkt,
    UInt                byteCnt,
    Bool                bIsrcCont,
    Bool                bIsrcValid,
    UInt8               uIsrcStatus,
    Bool                bEnable
);

/*============================================================================*/
/**
    \brief      Set ISRC2 packet & enable/disable packet insertion

    \param[in]  txUnit      Transmitter unit number
    \param[in]  pPkt        Pointer to Data Island Packet structure
    \param[in]  byteCnt     Packet buffer byte count
    \param[in]  bEnable     Enable or disable packet insertion

    \return     The call result:
                - TM_OK: the call was successful
                - Else a problem has been detected:
                  - TMBSL_ERR_HDMI_BAD_PARAMETER: a parameter was out of range
                  - TMBSL_ERR_HDMI_BAD_UNIT_NUMBER: bad transmitter unit number
                  - TMBSL_ERR_HDMI_I2C_WRITE: failed when writing to the I2C bus
                  - TMBSL_ERR_HDMI_INCONSISTENT_PARAMS: pointer suppied with byte count of zero
                  - TMBSL_ERR_HDMI_NOT_INITIALIZED: transmitter not initialized
                  - TMBSL_ERR_HDMI_NOT_SUPPORTED: not possible with this device
                  - TMBSL_ERR_HDMI_OPERATION_NOT_PERMITTED: not allowed in DVI mode

    \note       Data Island Packet parameter structure:
                UInt8 dataByte[28]      Packet Data

    \sa         NA
 */
tmErrorCode_t
tmbslHdmiTxPktSetIsrc2
(
    tmUnitSelect_t      txUnit,
    tmbslHdmiTxPkt_t    *pPkt,
    UInt                byteCnt,
    Bool                bEnable
);

/*============================================================================*/
/**
 * tmbslHdmiTxPktSetMpegInfoframe() parameter types
 */
/** MPEG frame types */
typedef enum
{
    HDMITX_MPEG_FRAME_UNKNOWN   = 0,    /**< Unknown  */
    HDMITX_MPEG_FRAME_I         = 1,    /**< i-frame   */
    HDMITX_MPEG_FRAME_B         = 2,    /**< b-frame */
    HDMITX_MPEG_FRAME_P         = 3,    /**< p-frame */
    HDMITX_MPEG_FRAME_INVALID   = 4     /**< Invalid   */
} tmbslHdmiTxMpegFrame_t;

/** \brief The MPEG Infoframe Parameter structure */
typedef struct _tmbslHdmiTxPktMpeg_t
{
    UInt32                  bitRate;        /**< MPEG bit rate in Hz */
    tmbslHdmiTxMpegFrame_t  frameType;      /**< MPEG frame type */
    Bool                    bFieldRepeat;   /**< 0: new field, 1:repeated field */
}tmbslHdmiTxPktMpeg_t;

/**
    \brief      Set MPEG infoframe packet & enable/disable packet insertion

    \param[in]  txUnit          Transmitter unit number
    \param[in]  pPkt            Pointer to MPEG Infoframe structure
    \param[in]  bEnable         Enable or disable packet insertion

    \return     The call result:
                - TM_OK: the call was successful
                - Else a problem has been detected:
                  - TMBSL_ERR_HDMI_BAD_PARAMETER: a parameter was out of range
                  - TMBSL_ERR_HDMI_BAD_UNIT_NUMBER: bad transmitter unit number
                  - TMBSL_ERR_HDMI_I2C_WRITE: failed when writing to the I2C bus
                  - TMBSL_ERR_HDMI_NOT_INITIALIZED: transmitter not initialized
                  - TMBSL_ERR_HDMI_NOT_SUPPORTED: not possible with this device
                  - TMBSL_ERR_HDMI_OPERATION_NOT_PERMITTED: not allowed in DVI mode

    \note       MPEG Infoframe structure:
                UInt32                  bitRate
                tmbslHdmiTxMpegFrame_t  frameType
                Bool                    bFieldRepeat

    \sa         NA
 */
tmErrorCode_t
tmbslHdmiTxPktSetMpegInfoframe
(
    tmUnitSelect_t          txUnit,
    tmbslHdmiTxPktMpeg_t    *pPkt,
    Bool                    bEnable
);

/*============================================================================*/
/**
    \brief      Enable NULL packet insertion

    \param[in]  txUnit      Transmitter unit number
    \param[in]  bEnable     Enable or disable packet insertion

    \return     The call result:
                - TM_OK: the call was successful
                - Else a problem has been detected:
                  - TMBSL_ERR_HDMI_BAD_PARAMETER: a parameter was out of range
                  - TMBSL_ERR_HDMI_BAD_UNIT_NUMBER: bad transmitter unit number
                  - TMBSL_ERR_HDMI_I2C_WRITE: failed when writing to the I2C bus
                  - TMBSL_ERR_HDMI_NOT_INITIALIZED: transmitter not initialized
                  - TMBSL_ERR_HDMI_OPERATION_NOT_PERMITTED: not allowed in DVI mode
 */
tmErrorCode_t
tmbslHdmiTxPktSetNullInsert
(
    tmUnitSelect_t  txUnit,
    Bool            bEnable
);

/*============================================================================*/
/**
    \brief      Set single Null packet insertion (flag auto-resets after 
                transmission)

    \param[in]  txUnit      Transmitter unit number

    \return     The call result:
                - TM_OK: the call was successful
                - Else a problem has been detected:
                  - TMBSL_ERR_HDMI_BAD_PARAMETER: a parameter was out of range
                  - TMBSL_ERR_HDMI_BAD_UNIT_NUMBER: bad transmitter unit number
                  - TMBSL_ERR_HDMI_I2C_WRITE: failed when writing to the I2C bus
                  - TMBSL_ERR_HDMI_NOT_INITIALIZED: transmitter not initialized
                  - TMBSL_ERR_HDMI_OPERATION_NOT_PERMITTED: not allowed in DVI mode

    \note       Operation resets after single transmission
 */
tmErrorCode_t
tmbslHdmiTxPktSetNullSingle
(
    tmUnitSelect_t  txUnit
);

/*============================================================================*/
/**
  * Source Product Description Infoframe Parameter types
  */
/** SDI frame types */
typedef enum
{
    HDMITX_SPD_INFO_UNKNOWN     = 0,
    HDMITX_SPD_INFO_DIGITAL_STB = 1,
    HDMITX_SPD_INFO_DVD         = 2,
    HDMITX_SPD_INFO_DVHS        = 3,
    HDMITX_SPD_INFO_HDD_VIDEO   = 4,
    HDMITX_SPD_INFO_DVC         = 5,
    HDMITX_SPD_INFO_DSC         = 6,
    HDMITX_SPD_INFO_VIDEO_CD    = 7,
    HDMITX_SPD_INFO_GAME        = 8,
    HDMITX_SPD_INFO_PC          = 9,
    HDMITX_SPD_INFO_INVALID     = 10
} tmbslHdmiTxSourceDev_t;

#define HDMI_TX_SPD_VENDOR_SIZE 8
#define HDMI_TX_SPD_DESCR_SIZE  16
#define HDMI_TX_SPD_LENGTH      25
 
/** \brief The Source Product Description Infoframe Parameter structure */
typedef struct _tmbslHdmiTxPktSpd_t
{
    UInt8                   VendorName[HDMI_TX_SPD_VENDOR_SIZE]; /**< Vendor name */
    UInt8                   ProdDescr[HDMI_TX_SPD_DESCR_SIZE]; /**< Product Description */
    tmbslHdmiTxSourceDev_t  SourceDevInfo;                     /**< Source Device Info */
} tmbslHdmiTxPktSpd_t;

/**
    \brief      Set audio info frame packet & enable/disable packet insertion

    \param[in]  txUnit  Transmitter unit number
    \param[in]  pPkt    Pointer to Audio Infoframe structure
    \param[in]  bEnable Enable or disable packet insertion

    \return     The call result:
                - TM_OK: the call was successful
                - Else a problem has been detected:
                  - TMBSL_ERR_HDMI_BAD_PARAMETER: a parameter was out of range
                  - TMBSL_ERR_HDMI_BAD_UNIT_NUMBER: bad transmitter unit number
                  - TMBSL_ERR_HDMI_I2C_WRITE: failed when writing to the I2C bus
                  - TMBSL_ERR_HDMI_NOT_INITIALIZED: transmitter not initialized
                  - TMBSL_ERR_HDMI_NOT_SUPPORTED: not possible with this device
                  - TMBSL_ERR_HDMI_OPERATION_NOT_PERMITTED: not allowed in DVI mode

    \note       Audio Infoframe structure:
                UInt8                   VendorName[8]
                UInt8                   ProdDescr[16]
                tmbslHdmiTxSourceDev_t  SourceDevInfo
 */
tmErrorCode_t
tmbslHdmiTxPktSetSpdInfoframe
(
    tmUnitSelect_t      txUnit,
    tmbslHdmiTxPktSpd_t *pPkt,
    Bool                bEnable
);

/*============================================================================*/
/**
  * \brief The Video Infoframe Parameter structure
  */
typedef struct _tmbslHdmiTxPktVif_t
{
    UInt8  Colour;               /**< 0 to 03h */
    Bool   ActiveInfo;           /**< 0/1 */
    UInt8  BarInfo;              /**< 0 to 03h */
    UInt8  ScanInfo;             /**< 0 to 03h */
    UInt8  Colorimetry;          /**< 0 to 03h */
    UInt8  PictureAspectRatio;   /**< 0 to 03h */
    UInt8  ActiveFormatRatio;    /**< 0 to 0Fh */
    UInt8  Scaling;              /**< 0 to 03h */
    UInt8  VidFormat;            /**< 0 to 7Fh */
    UInt8  PixelRepeat;          /**< 0 to 0Fh */
    UInt16 EndTopBarLine;       
    UInt16 StartBottomBarLine;  
    UInt16 EndLeftBarPixel;
    UInt16 StartRightBarPixel;
} tmbslHdmiTxPktVif_t;

/**
    \brief      Set video infoframe packet & enable/disable packet insertion

    \param[in]  txUnit  Transmitter unit number
    \param[in]  pPkt    Pointer to Video Infoframe structure
    \param[in]  bEnable Enable or disable packet insertion

    \return     The call result:
                - TM_OK: the call was successful
                - Else a problem has been detected:
                  - TMBSL_ERR_HDMI_BAD_PARAMETER: a parameter was out of range
                  - TMBSL_ERR_HDMI_BAD_UNIT_NUMBER: bad transmitter unit number
                  - TMBSL_ERR_HDMI_I2C_WRITE: failed when writing to the I2C bus
                  - TMBSL_ERR_HDMI_NOT_INITIALIZED: transmitter not initialized
                  - TMBSL_ERR_HDMI_OPERATION_NOT_PERMITTED: not allowed in DVI mode

    \note       Video Infoframe structure:
                UInt8 Colour
                Bool ActiveInfo
                UInt8 BarInfo
                UInt8 ScanInfo
                UInt8 Colorimetry
                UInt8 PictureAspectRatio
                UInt8 ActiveFormatRatio
                UInt8 Scaling
                UInt8 VidFormat
                UInt8 PixelRepeat
                UInt16 EndTopBarLine
                UInt16 StartBottomBarLine
                UInt16 EndLeftBarPixel
                UInt16 StartRightBarPixel   (incorrectly named in [HDMI1.2])
 */
tmErrorCode_t
tmbslHdmiTxPktSetVideoInfoframe
(
    tmUnitSelect_t      txUnit,
    tmbslHdmiTxPktVif_t *pPkt,
    Bool                bEnable
);

/*============================================================================*/
/**
    \brief      Set Vendor Specific Infoframe packet & enable/disable packet insertion

    \param[in]  txUnit      Transmitter unit number
    \param[in]  pPkt        Pointer to Data Island Packet structure
    \param[in]  byteCnt     Packet buffer byte count
    \param[in]  uVersion    Version number for packet header
    \param[in]  bEnable     Enable or disable packet insertion

    \return     The call result:
                - TM_OK: the call was successful
                - Else a problem has been detected:
                  - TMBSL_ERR_HDMI_BAD_PARAMETER: a parameter was out of range
                  - TMBSL_ERR_HDMI_BAD_UNIT_NUMBER: bad transmitter unit number
                  - TMBSL_ERR_HDMI_I2C_WRITE: failed when writing to the I2C bus
                  - TMBSL_ERR_HDMI_INCONSISTENT_PARAMS: pointer suppied with byte count of zero
                  - TMBSL_ERR_HDMI_NOT_INITIALIZED: transmitter not initialized
                  - TMBSL_ERR_HDMI_NOT_SUPPORTED: not possible with this device
                  - TMBSL_ERR_HDMI_OPERATION_NOT_PERMITTED: not allowed in DVI mode

    \note       Data Island Packet parameter structure:
                UInt8 dataByte[28]      Packet Data (only use 27 bytes max)

    \sa         NA
 */
tmErrorCode_t
tmbslHdmiTxPktSetVsInfoframe
(
    tmUnitSelect_t      txUnit,
    tmbslHdmiTxPkt_t    *pPkt,
    UInt                byteCnt,
    UInt8               uVersion,
    Bool                bEnable
);

/*============================================================================*/
/**
    \brief      Get the power state of the transmitter

    \param[in]  txUnit       Transmitter unit number
    \param[out] pePowerState Pointer to the power state of the device now

    \return     The call result:
                - TM_OK: the call was successful
                - Else a problem has been detected:
                  - TMBSL_ERR_HDMI_BAD_UNIT_NUMBER: bad transmitter unit number
                  - TMBSL_ERR_HDMI_BAD_PARAMETER: a parameter was out of range
                  - TMBSL_ERR_HDMI_NOT_INITIALIZED: transmitter not initialized

    \note       Power states:
                - tmPowerOn
                - tmPowerStandby
 */
tmErrorCode_t
tmbslHdmiTxPowerGetState
(
    tmUnitSelect_t      txUnit,
	ptmPowerState_t		pePowerState
);

/*============================================================================*/
/**
    \brief      Set the power state of the transmitter

    \param[in]  txUnit      Transmitter unit number
    \param[in]  ePowerState Power state to set

    \return     The call result:
                - TM_OK: the call was successful
                - Else a problem has been detected:
                  - TMBSL_ERR_HDMI_BAD_UNIT_NUMBER: bad transmitter unit number 
                  - TMBSL_ERR_HDMI_BAD_PARAMETER: a parameter was out of range
                  - TMBSL_ERR_HDMI_NOT_INITIALIZED: transmitter not initialized
                  - TMBSL_ERR_HDMI_I2C_WRITE: failed when writing to the I2C bus

    \note       Power states (Off and Suspend are treated the same as Standby):
                - tmPowerOn
                - tmPowerStandby
                - tmPowerSuspend
                - tmPowerOff
 */
tmErrorCode_t
tmbslHdmiTxPowerSetState
(
    tmUnitSelect_t      txUnit,
	tmPowerState_t      ePowerState
);


/*============================================================================*/
/**
    \brief      Reset the HDMI transmitter

    \param[in]  txUnit      Transmitter unit number

    \return     The call result:
                - TM_OK: the call was successful
                - Else a problem has been detected:
                  - TMBSL_ERR_HDMI_BAD_UNIT_NUMBER: bad transmitter unit number
                  - TMBSL_ERR_HDMI_BAD_PARAMETER: a parameter was out of range
                  - TMBSL_ERR_HDMI_NOT_INITIALIZED: transmitter not initialized
                  - TMBSL_ERR_HDMI_I2C_WRITE: failed when writing to the I2C bus

    \note       NA

    \sa         tmbslHdmiTxInit
 */
tmErrorCode_t
tmbslHdmiTxReset
(
    tmUnitSelect_t      txUnit
);


/*============================================================================*/
/**
 * \brief The tmbslHdmiTxScalerGet() parameter type
 */
typedef struct _tmbslHdmiTxScalerDiag_t
{
    UInt16 maxBuffill_p;     /**< Filling primary video buffer           */
    UInt16 maxBuffill_d;     /**< Filling video deinterlaced buffer      */
    UInt8 maxFifofill_pi;    /**< Filling primary video input FIFO       */
    UInt8 minFifofill_po1;   /**< Filling primary video output FIFO #1   */
    UInt8 minFifofill_po2;   /**< Filling primary video output FIFO #2   */
    UInt8 minFifofill_po3;   /**< Filling primary video output FIFO #3   */
    UInt8 minFifofill_po4;   /**< Filling primary video output FIFO #4   */
    UInt8 maxFifofill_di;    /**< Filling deinterlaced video input FIFO  */
    UInt8 maxFifofill_do;    /**< Filling deinterlaced video output FIFO */
} tmbslHdmiTxScalerDiag_t;

/**
    \brief      Get diagnostic counters from the scaler

    \param[in]  txUnit      Transmitter unit number
    \param[out] pScalerDiag Pointer to structure to receive scaler diagnostic
                            registers

    \return     The call result:
                - TM_OK: the call was successful
                - Else a problem has been detected:
                  - TMBSL_ERR_HDMI_BAD_UNIT_NUMBER: bad transmitter unit number
                  - TMBSL_ERR_HDMI_BAD_PARAMETER: a parameter was out of range
                  - TMBSL_ERR_HDMI_NOT_INITIALIZED: transmitter not initialized
                  - TMBSL_ERR_HDMI_I2C_READ: failed when reading the I2C bus

    \note       scaler diagnostic registers structure:
                UInt16 maxBuffill_p     Filling primary video buffer
                UInt16 maxBuffill_d     Filling video deinterlaced buffer
                UInt8  maxFifofill_pi   Filling primary video input FIFO
                UInt8  minFifofill_po1  Filling primary video output FIFO #1
                UInt8  minFifofill_po2  Filling primary video output FIFO #2
                UInt8  minFifofill_po3  Filling primary video output FIFO #3
                UInt8  minFifofill_po4  Filling primary video output FIFO #4
                UInt8  maxFifofill_di   Filling deinterlaced video input FIFO
                UInt8  maxFifofill_do   Filling deinterlaced video output FIFO
 */
tmErrorCode_t
tmbslHdmiTxScalerGet
(
    tmUnitSelect_t           txUnit,
    tmbslHdmiTxScalerDiag_t *pScalerDiag
);


/*============================================================================*/
/**
 * tmbslHdmiTxScalerGetMode() parameter types
 */
/** Scaler modes */
typedef enum
{
    HDMITX_SCAMODE_OFF       = 0,    /**< Off  */
    HDMITX_SCAMODE_ON        = 1,    /**< On   */
    HDMITX_SCAMODE_AUTO      = 2,    /**< Auto */
    HDMITX_SCAMODE_NO_CHANGE = 3,    /**< No change */
    HDMITX_SCAMODE_INVALID   = 4     /**< Invalid   */
} tmbslHdmiTxScaMode_t;
 
/**
    \brief      Get the current scaler mode

    \param[in]  txUnit      Transmitter unit number
    \param[out] pScalerMode Pointer to variable to receive scaler mode

    \return     The call result:
                - TM_OK: the call was successful
                - Else a problem has been detected:
                  - TMBSL_ERR_HDMI_BAD_UNIT_NUMBER: bad transmitter unit number
                  - TMBSL_ERR_HDMI_BAD_PARAMETER: a parameter was out of range
                  - TMBSL_ERR_HDMI_NOT_INITIALIZED: transmitter not initialized
*/
tmErrorCode_t
tmbslHdmiTxScalerGetMode
(
    tmUnitSelect_t		      txUnit,
    tmbslHdmiTxScaMode_t      *pScalerMode
);


/*============================================================================*/
/**
    \brief      Enable or disable scaler input frame

    \param[in]  txUnit      Transmitter unit number
    \param[in]  bDisable    Enable or disable scaler input

    \return     The call result:
                - TM_OK: the call was successful
                - Else a problem has been detected:
                  - TMBSL_ERR_HDMI_BAD_UNIT_NUMBER: bad transmitter unit number
                  - TMBSL_ERR_HDMI_BAD_PARAMETER: a parameter was out of range
                  - TMBSL_ERR_HDMI_NOT_INITIALIZED: transmitter not initialized
                  - TMBSL_ERR_HDMI_I2C_WRITE: failed when writing to the I2C bus
 */
tmErrorCode_t
tmbslHdmiTxScalerInDisable
(
    tmUnitSelect_t  txUnit,
    Bool            bDisable
);


/*============================================================================*/
/**
 * tmbslHdmiTxScalerSetCoeffs() parameter types
 */
/** Scaler lookup table selection */
typedef enum
{
    HDMITX_SCALUT_DEFAULT_TAB1     = 0,    /**< Use default table 1 */
    HDMITX_SCALUT_DEFAULT_TAB2     = 1,    /**< Use default table 2 */
    HDMITX_SCALUT_USE_VSLUT        = 2,    /**< Use vsLut parameter */
    HDMITX_SCALUT_INVALID          = 3     /**< Invalid value       */
} tmbslHdmiTxScaLut_t;
 
/** Scaler control parameter structure array size */
enum _tmbslHdmiTxvsLut
{
    HDMITX_VSLUT_COEFF_NUM         = 45
};

/**
    \brief      Set the active coefficient lookup table for the vertical scaler 

    \param[in]  txUnit      Transmitter unit number
    \param[in]  lutSel      Coefficient lookup table selection
    \param[in]  pVsLut      Table of HDMITX_VSLUT_COEFF_NUM coefficient values
                            (may be null if lutSel not HDMITX_SCALUT_USE_VSLUT)

    \return     The call result:
                - TM_OK: the call was successful
                - Else a problem has been detected:
                  - TMBSL_ERR_HDMI_BAD_UNIT_NUMBER: bad transmitter unit number
                  - TMBSL_ERR_HDMI_BAD_PARAMETER: a parameter was out of range
                  - TMBSL_ERR_HDMI_INCONSISTENT_PARAMS: two parameters disagree
                  - TMBSL_ERR_HDMI_NOT_INITIALIZED: transmitter not initialized
                  - TMBSL_ERR_HDMI_I2C_WRITE: failed when writing to the I2C bus
 */
tmErrorCode_t
tmbslHdmiTxScalerSetCoeffs
(
    tmUnitSelect_t        txUnit,
    tmbslHdmiTxScaLut_t   lutSel,
    UInt8                *pVsLut
);


/*============================================================================*/
/**
 * tmbslHdmiTxScalerSetFieldOrder() parameter types
 */
/** IntExt values */
typedef enum
{
    HDMITX_INTEXT_INTERNAL         = 0,    /**< Internal  */
    HDMITX_INTEXT_EXTERNAL         = 1,    /**< External  */
    HDMITX_INTEXT_NO_CHANGE        = 2,    /**< No change */
    HDMITX_INTEXT_INVALID          = 3     /**< Invalid   */
} tmbslHdmiTxIntExt_t;

/** TopSel values */
typedef enum
{
    HDMITX_TOPSEL_INTERNAL         = 0,    /**< Internal  */
    HDMITX_TOPSEL_VRF              = 1,    /**< VRF       */
    HDMITX_TOPSEL_NO_CHANGE        = 2,    /**< No change */
    HDMITX_TOPSEL_INVALID          = 3     /**< Invalid   */
} tmbslHdmiTxTopSel_t;

/** TopTgl values */
typedef enum
{
    HDMITX_TOPTGL_NO_ACTION        = 0,    /**< NO action */
    HDMITX_TOPTGL_TOGGLE           = 1,    /**< Toggle    */
    HDMITX_TOPTGL_NO_CHANGE        = 2,    /**< No change */
    HDMITX_TOPTGL_INVALID          = 3     /**< Invalid   */
} tmbslHdmiTxTopTgl_t;

/**
    \brief      Set scaler field positions 

    \param[in]  txUnit      Transmitter unit number
    \param[in]  topExt      Internal, External, No Change
    \param[in]  deExt       Internal, External, No Change
    \param[in]  topSel      Internal, VRF, No Change
    \param[in]  topTgl      No Action, Toggle, No Change

    \return     The call result:
                - TM_OK: the call was successful
                - Else a problem has been detected:
                  - TMBSL_ERR_HDMI_BAD_UNIT_NUMBER: bad transmitter unit number
                  - TMBSL_ERR_HDMI_BAD_PARAMETER: a parameter was out of range
                  - TMBSL_ERR_HDMI_NOT_INITIALIZED: transmitter not initialized
                  - TMBSL_ERR_HDMI_I2C_WRITE: failed when writing to the I2C bus
 */
tmErrorCode_t   
tmbslHdmiTxScalerSetFieldOrder 
(
    tmUnitSelect_t      txUnit,
    tmbslHdmiTxIntExt_t topExt,
    tmbslHdmiTxIntExt_t deExt,
    tmbslHdmiTxTopSel_t topSel,
    tmbslHdmiTxTopTgl_t topTgl 
);


/*============================================================================*/
/**
 * tmbslHdmiTxScalerSetFine() parameter types
 */
/** Reference pixel values */
enum _tmbslHdmiTxScalerFinePixelLimits
{
    HDMITX_SCALER_FINE_PIXEL_MIN        = 0x0000,
    HDMITX_SCALER_FINE_PIXEL_MAX        = 0x1FFF,
    HDMITX_SCALER_FINE_PIXEL_NO_CHANGE  = 0x2000,
    HDMITX_SCALER_FINE_PIXEL_INVALID    = 0x2001
};

/** Reference line values */
enum _tmbslHdmiTxScalerFineLineLimits
{
    HDMITX_SCALER_FINE_LINE_MIN         = 0x0000,
    HDMITX_SCALER_FINE_LINE_MAX         = 0x07FF,
    HDMITX_SCALER_FINE_LINE_NO_CHANGE   = 0x0800,
    HDMITX_SCALER_FINE_LINE_INVALID     = 0x0801
};

/**
    \brief      Set scaler fine adjustment options

    \param[in]  txUnit       Transmitter unit number
    \param[in]  uRefPix      Ref. pixel preset 0 to 1FFFh (2000h = No Change)
    \param[in]  uRefLine     Ref. line preset 0 to 7FFh (800h = No Change)

    \return     The call result:
                - TM_OK: the call was successful
                - Else a problem has been detected:
                  - TMBSL_ERR_HDMI_BAD_UNIT_NUMBER: bad transmitter unit number
                  - TMBSL_ERR_HDMI_BAD_PARAMETER: a parameter was out of range
                  - TMBSL_ERR_HDMI_NOT_INITIALIZED: transmitter not initialized
                  - TMBSL_ERR_HDMI_I2C_WRITE: failed when writing to the I2C bus
 */
tmErrorCode_t
tmbslHdmiTxScalerSetFine
(
    tmUnitSelect_t  txUnit,
    UInt16          uRefPix,    
    UInt16          uRefLine   
);


/*============================================================================*/
/**
 * tmbslHdmiTxScalerSetSync() parameter types
 */
/** Video sync method */
typedef enum
{
    HDMITX_VSMETH_V_H          = 0,    /**< V and H    */
    HDMITX_VSMETH_V_XDE        = 1,    /**< V and X-DE */
    HDMITX_VSMETH_NO_CHANGE    = 2,    /**< No change  */
    HDMITX_VSMETH_INVALID      = 3     /**< Invalid    */
} tmbslHdmiTxVsMeth_t;
 
/** Line/pixel counters sync */
typedef enum
{
    HDMITX_VSONCE_EACH_FRAME   = 0,    /**< Sync on each frame */
    HDMITX_VSONCE_ONCE         = 1,    /**< Sync once only     */
    HDMITX_VSONCE_NO_CHANGE    = 2,    /**< No change  */
    HDMITX_VSONCE_INVALID      = 3     /**< Invalid    */
} tmbslHdmiTxVsOnce_t;
 

/**
    \brief      Set scaler synchronization options

    \param[in]  txUnit       Transmitter unit number
    \param[in]  method       Sync. combination method
    \param[in]  once         Line/pixel counters sync once or each frame

    \return     The call result:
                - TM_OK: the call was successful
                - Else a problem has been detected:
                  - TMBSL_ERR_HDMI_BAD_UNIT_NUMBER: bad transmitter unit number
                  - TMBSL_ERR_HDMI_BAD_PARAMETER: a parameter was out of range
                  - TMBSL_ERR_HDMI_NOT_INITIALIZED: transmitter not initialized
                  - TMBSL_ERR_HDMI_I2C_WRITE: failed when writing to the I2C bus
 */
tmErrorCode_t
tmbslHdmiTxScalerSetSync
(
    tmUnitSelect_t      txUnit,
    tmbslHdmiTxVsMeth_t method,
    tmbslHdmiTxVsOnce_t once
);


/*============================================================================*/
/**
    \brief      Get the driver software version and compatibility numbers 

    \param[out] pSWVersion   Pointer to the software version structure returned

    \return     The call result:
                - TM_OK: the call was successful
                - Else a problem has been detected:
                  - TMBSL_ERR_HDMI_BAD_UNIT_NUMBER: bad transmitter unit number
                  - TMBSL_ERR_HDMI_BAD_PARAMETER: a parameter was out of range
                  - TMBSL_ERR_HDMI_NOT_INITIALIZED: transmitter not initialized
 */
tmErrorCode_t   
tmbslHdmiTxSwGetVersion 
(
    ptmSWVersion_t      pSWVersion
);


/*============================================================================*/
/**
    \brief      Get the driver software version and compatibility numbers 

    \param[in]  txUnit       Transmitter unit number
    \param[in]  waitMs       Period in milliseconds to wait

    \return     The call result:
                - TM_OK: the call was successful
                - Else a problem has been detected:
                  - TMBSL_ERR_HDMI_BAD_UNIT_NUMBER: bad transmitter unit number
                  - TMBSL_ERR_HDMI_NOT_INITIALIZED: transmitter not initialized
 */
tmErrorCode_t   
tmbslHdmiTxSysTimerWait 
(
    tmUnitSelect_t      txUnit,
    UInt16              waitMs
);


/*============================================================================*/
/**
 * tmbslHdmiTxTmdsSetOutputs() parameter types
 */
/** TMDS output mode */
typedef enum
{
    HDMITX_TMDSOUT_NORMAL      = 0,    /**< Normal outputs   */
    HDMITX_TMDSOUT_NORMAL1     = 1,    /**< Normal outputs, same as 0  */
    HDMITX_TMDSOUT_FORCED0     = 2,    /**< Forced 0 outputs */
    HDMITX_TMDSOUT_FORCED1     = 3,    /**< Forced 1 outputs */
    HDMITX_TMDSOUT_INVALID     = 4     /**< Invalid          */
} tmbslHdmiTxTmdsOut_t;
 
/**
    \brief      Set the TMDS outputs to normal active operation or to a forced
                state 

    \param[in]  txUnit      Transmitter unit number
    \param[in]  tmdsOut     TMDS output mode

    \return     The call result:
                - TM_OK: the call was successful
                - Else a problem has been detected:
                  - TMBSL_ERR_HDMI_BAD_UNIT_NUMBER: bad transmitter unit number
                  - TMBSL_ERR_HDMI_BAD_PARAMETER: a parameter was out of range
                  - TMBSL_ERR_HDMI_NOT_INITIALIZED: transmitter not initialized
 */
tmErrorCode_t   
tmbslHdmiTxTmdsSetOutputs 
(
    tmUnitSelect_t          txUnit,
    tmbslHdmiTxTmdsOut_t    tmdsOut
);


/*============================================================================*/
/**
 * tmbslHdmiTxTmdsSetSerializer() parameter types
 */
/** Serializer phase limits */
enum _tmbslHdmiTxTmdsPhase
{
    HDMITX_TMDSPHASE_MIN       = 0,
    HDMITX_TMDSPHASE_MAX       = 15,
    HDMITX_TMDSPHASE_INVALID   = 16
};
 
/**
    \brief      Fine-tune the TMDS serializer 

    \param[in]  txUnit      Transmitter unit number
    \param[in]  uPhase2     Serializer phase 2
    \param[in]  uPhase3     Serializer phase 3

    \return     The call result:
                - TM_OK: the call was successful
                - Else a problem has been detected:
                  - TMBSL_ERR_HDMI_BAD_UNIT_NUMBER: bad transmitter unit number
                  - TMBSL_ERR_HDMI_BAD_PARAMETER: a parameter was out of range
                  - TMBSL_ERR_HDMI_NOT_INITIALIZED: transmitter not initialized
 */
tmErrorCode_t   
tmbslHdmiTxTmdsSetSerializer 
(
    tmUnitSelect_t  txUnit,
    UInt8           uPhase2,
    UInt8           uPhase3
);


/*============================================================================*/
/**
 * tmbslHdmiTxTestSetPattern() parameter types
 */
/** Test pattern types */
typedef enum
{
    HDMITX_PATTERN_OFF     = 0, /**< Insert test pattern     */
    HDMITX_PATTERN_CBAR4   = 1, /**< Insert 4-bar colour bar */
    HDMITX_PATTERN_CBAR8   = 2, /**< Insert 8-bar colour bar */
	HDMITX_PATTERN_BLUE	   = 3, /**< Insert Blue screen		 */
    HDMITX_PATTERN_INVALID = 4  /**< Invalid pattern		 */
} tmbslHdmiTxTestPattern_t;

/*
    \brief      Set a colour bar test pattern

    \param[in]  txUnit     Transmitter unit number
    \param[in]  pattern    Test pattern

    \return     The call result:
                - TM_OK: the call was successful
                - Else a problem has been detected:
                  - TMBSL_ERR_HDMI_BAD_UNIT_NUMBER: bad transmitter unit number
                  - TMBSL_ERR_HDMI_BAD_PARAMETER: a parameter was out of range
                  - TMBSL_ERR_HDMI_NOT_INITIALIZED: transmitter not initialized
                  - TMBSL_ERR_HDMI_I2C_WRITE: failed when writing to the I2C bus
 */
tmErrorCode_t
tmbslHdmiTxTestSetPattern
(
    tmUnitSelect_t           txUnit,
    tmbslHdmiTxTestPattern_t pattern 
);


/*============================================================================*/
/**
 * tmbslHdmiTxTestSetMode() parameter types
 */
/** Test modes */
typedef enum
{
    HDMITX_TESTMODE_PAT     = 0,/**< Insert test pattern                    */
    HDMITX_TESTMODE_656     = 1,/**< Inject CCIR-656 video via audio port   */
    HDMITX_TESTMODE_SERPHOE = 2,/**< Activate srl_tst_ph2_o & srl_tst_ph3_o */
    HDMITX_TESTMODE_NOSC    = 3,/**< Input nosc predivider = PLL-ref input  */
    HDMITX_TESTMODE_HVP     = 4,/**< Test high voltage protection cells     */
    HDMITX_TESTMODE_PWD     = 5,/**< Test PLLs in sleep mode                */
    HDMITX_TESTMODE_DIVOE   = 6,/**< Enable scaler PLL divider test output  */
    HDMITX_TESTMODE_INVALID = 7 /**< Invalid test							*/
} tmbslHdmiTxTestMode_t;

/** Test states */
typedef enum
{
    HDMITX_TESTSTATE_OFF     = 0,  /**< Disable the selected test */
    HDMITX_TESTSTATE_ON      = 1,  /**< Enable the selected test  */
    HDMITX_TESTSTATE_INVALID = 2   /**< Invalid value */
} tmbslHdmiTxTestState_t;

/**
    \brief      Set or clear one or more simultaneous test modes

    \param[in]  txUnit      Transmitter unit number
    \param[in]  testMode    Mode: tst_pat, tst_656, tst_serphoe, tst_nosc,
                            tst_hvp, tst_pwd, tst_divoe
    \param[in]  testState   State: 1=On, 0=Off

    \return     The call result:
                - TM_OK: the call was successful
                - Else a problem has been detected:
                  - TMBSL_ERR_HDMI_BAD_UNIT_NUMBER: bad transmitter unit number
                  - TMBSL_ERR_HDMI_BAD_PARAMETER: a parameter was out of range
                  - TMBSL_ERR_HDMI_NOT_INITIALIZED: transmitter not initialized
                  - TMBSL_ERR_HDMI_I2C_WRITE: failed when writing to the I2C bus
 */
tmErrorCode_t
tmbslHdmiTxTestSetMode
(
    tmUnitSelect_t          txUnit,
    tmbslHdmiTxTestMode_t   testMode,
    tmbslHdmiTxTestState_t  testState
);


/*============================================================================*/
/**
 * tmbslHdmiTxVideoInSetBlanking() parameter types
 */
/** Blankit Source */
typedef enum
{
    HDMITX_BLNKSRC_NOT_DE       = 0,       /**< Source=Not DE        */
    HDMITX_BLNKSRC_VS_HS        = 1,       /**< Source=VS And HS     */
    HDMITX_BLNKSRC_VS_NOT_HS    = 2,       /**< Source=VS And Not HS */
    HDMITX_BLNKSRC_VS_HEMB_VEMB = 3,       /**< Source=Hemb And Vemb */
    HDMITX_BLNKSRC_NO_CHANGE    = 4,       /**< No change */
    HDMITX_BLNKSRC_INVALID      = 5        /**< Invalid   */
} tmbslHdmiTxBlnkSrc_t;
 
/** Blanking Codes */
typedef enum
{
    HDMITX_BLNKCODE_ALL_0       = 0,       /**< Code=All Zero */
    HDMITX_BLNKCODE_RGB444      = 1,       /**< Code=RGB444   */
    HDMITX_BLNKCODE_YUV444      = 2,       /**< Code=YUV444   */
    HDMITX_BLNKCODE_YUV422      = 3,       /**< Code=YUV422   */
    HDMITX_BLNKCODE_NO_CHANGE   = 4,       /**< No change */
    HDMITX_BLNKCODE_INVALID     = 5        /**< Invalid   */
} tmbslHdmiTxBlnkCode_t;
 
/**
    \brief      Enable blanking between active data

    \param[in]  txUnit          Transmitter unit number
    \param[in]  blankitSource   Blankit Source: Not DE, VS And HS, 
                                VS And Not HS, Hemb And Vemb, No Change
    \param[in]  blankingCodes   Blanking Codes: All Zero, RGB444, YUV444,
                                YUV422, No Change

    \return     The call result:
                - TM_OK: the call was successful
                - Else a problem has been detected:
                  - TMBSL_ERR_HDMI_BAD_UNIT_NUMBER: bad transmitter unit number
                  - TMBSL_ERR_HDMI_BAD_PARAMETER: a parameter was out of range
                  - TMBSL_ERR_HDMI_NOT_INITIALIZED: transmitter not initialized
                  - TMBSL_ERR_HDMI_I2C_WRITE: failed when writing to the I2C bus

    \note       NA

    \sa         NA
 */
tmErrorCode_t
tmbslHdmiTxVideoInSetBlanking
(
    tmUnitSelect_t         txUnit,
    tmbslHdmiTxBlnkSrc_t   blankitSource,
    tmbslHdmiTxBlnkCode_t  blankingCodes
);


/*============================================================================*/
/**
 * tmbslHdmiTxVideoInSetConfig() parameter types
 */
/** Sample edge */
typedef enum
{
    HDMITX_PIXEDGE_CLK_POS      = 0,        /**< Pixel Clock Positive Edge */
    HDMITX_PIXEDGE_CLK_NEG      = 1,        /**< Pixel Clock Negative Edge */
    HDMITX_PIXEDGE_NO_CHANGE    = 2,        /**< No Change */
    HDMITX_PIXEDGE_INVALID      = 3         /**< Invalid   */
} tmbslHdmiTxPixEdge_t;

/** Upsample modes */
typedef enum
{
    HDMITX_UPSAMPLE_BYPASS      = 0,        /**< Bypass */
    HDMITX_UPSAMPLE_COPY        = 1,        /**< Copy */
    HDMITX_UPSAMPLE_INTERPOLATE = 2,        /**< Interpolate */
    HDMITX_UPSAMPLE_AUTO        = 3,        /**< Auto: driver chooses best value */
    HDMITX_UPSAMPLE_NO_CHANGE   = 4,        /**< No Change */
    HDMITX_UPSAMPLE_INVALID     = 5         /**< Invalid   */
} tmbslHdmiTxUpsampleMode_t;

/**
    \brief      Configure video input options and control the upsampler

    \param[in]  txUnit          Transmitter unit number
    \param[in]  vinMode         Video input mode
    \param[in]  sampleEdge      Sample edge:
                                Pixel Clock Positive Edge, 
                                Pixel Clock Negative Edge, No Change
    \param[in]  pixRate         Single data or double data rate
    \param[in]  upsampleMode    Upsample mode

    \return     The call result:
                - TM_OK: the call was successful
                - Else a problem has been detected:
                  - TMBSL_ERR_HDMI_BAD_UNIT_NUMBER: bad transmitter unit number
                  - TMBSL_ERR_HDMI_BAD_PARAMETER: a parameter was out of range
                  - TMBSL_ERR_HDMI_NOT_INITIALIZED: transmitter not initialized
                  - TMBSL_ERR_HDMI_I2C_WRITE: failed when writing to the I2C bus
 */
tmErrorCode_t
tmbslHdmiTxVideoInSetConfig
(
    tmUnitSelect_t            txUnit,
    tmbslHdmiTxVinMode_t      vinMode,
    tmbslHdmiTxPixEdge_t      sampleEdge,
    tmbslHdmiTxPixRate_t      pixRate,
    tmbslHdmiTxUpsampleMode_t upsampleMode 
);


/*============================================================================*/
/**
 * tmbslHdmiTxVideoInSetFine() parameter types
 */
/** Subpacket count */
typedef enum
{
    HDMITX_PIXSUBPKT_FIX_0      = 0,        /**< Fix At 0 */
    HDMITX_PIXSUBPKT_FIX_1      = 1,        /**< Fix At 1 */
    HDMITX_PIXSUBPKT_FIX_2      = 2,        /**< Fix At 2 */
    HDMITX_PIXSUBPKT_FIX_3      = 3,        /**< Fix At 3 */
    HDMITX_PIXSUBPKT_SYNC_FIRST = 4,        /**< First Sync value */
    HDMITX_PIXSUBPKT_SYNC_HEMB  = 4,        /**< Sync By Hemb */
    HDMITX_PIXSUBPKT_SYNC_DE    = 5,        /**< Sync By Rising Edge DE */
    HDMITX_PIXSUBPKT_SYNC_HS    = 6,        /**< Sync By Rising Edge HS */
    HDMITX_PIXSUBPKT_NO_CHANGE  = 7,        /**< No Change */
    HDMITX_PIXSUBPKT_INVALID    = 8,        /**< Invalid   */
    HDMITX_PIXSUBPKT_SYNC_FIXED = 3         /**< Not used as a parameter value,
                                             *  but used internally when 
                                             *  Fix at 0/1/2/3 values are set */
} tmbslHdmiTxPixSubpkt_t;

/** Toggling */
typedef enum
{
    HDMITX_PIXTOGL_NO_ACTION    = 0,        /**< No Action  */
    HDMITX_PIXTOGL_ENABLE       = 1,        /**< Toggle     */
    HDMITX_PIXTOGL_NO_CHANGE    = 2,        /**< No Change  */
    HDMITX_PIXTOGL_INVALID      = 3         /**< Invalid    */
} tmbslHdmiTxPixTogl_t;

/**
    \brief      Set fine image position

    \param[in]  txUnit          Transmitter unit number
    \param[in]  subpacketCount  Subpacket Count fixed values and sync options
    \param[in]  toggleClk1      Toggle clock 1 phase w.r.t. clock 2

    \return     The call result:
                - TM_OK: the call was successful
                - Else a problem has been detected:
                  - TMBSL_ERR_HDMI_BAD_UNIT_NUMBER: bad transmitter unit number
                  - TMBSL_ERR_HDMI_BAD_PARAMETER: a parameter was out of range
                  - TMBSL_ERR_HDMI_NOT_INITIALIZED: transmitter not initialized
                  - TMBSL_ERR_HDMI_I2C_WRITE: failed when writing to the I2C bus

    \note       NA     

    \sa         NA
 */
tmErrorCode_t
tmbslHdmiTxVideoInSetFine
(
    tmUnitSelect_t            txUnit,
    tmbslHdmiTxPixSubpkt_t    subpacketCount,
    tmbslHdmiTxPixTogl_t      toggleClk1 
);


/*============================================================================*/
/**
 * tmbslHdmiTxVideoInSetMapping() parameter types
 */
/** Video input port parameter structure array size and limits */
enum _tmbslHdmiTxVinPortMap
{
    HDMITX_VIN_PORT_MAP_TABLE_LEN    = 6,

    HDMITX_VIN_PORT_SWAP_NO_CHANGE   = 6,
    HDMITX_VIN_PORT_SWAP_INVALID     = 7,

    HDMITX_VIN_PORT_MIRROR_NO_CHANGE = 2,
    HDMITX_VIN_PORT_MIRROR_INVALID   = 3
};

/**
    \brief      Set video input port swapping and mirroring

    \param[in]  txUnit          Transmitter unit number
    \param[in]  pSwapTable      Pointer to 6-byte port swap table
    \param[in]  pMirrorTable    Pointer to 6-byte port mirror table  

    \return     The call result:
                - TM_OK: the call was successful
                - Else a problem has been detected:
                  - TMBSL_ERR_HDMI_BAD_UNIT_NUMBER: bad transmitter unit number
                  - TMBSL_ERR_HDMI_BAD_PARAMETER: a parameter was out of range
                  - TMBSL_ERR_HDMI_NOT_INITIALIZED: transmitter not initialized
                  - TMBSL_ERR_HDMI_I2C_WRITE: failed when writing to the I2C bus

    \note       UInt8 pSwapTable[6]
                
                Each table position 0 to 5 represents a group of 4 port bits:
                [0]=23:20, [1]=16:19, [2]=15:12, [3]=11:8, [4]=4:7, [5]=0:3
                Table position values are 0 to 6, denoting the group of 4 port
                bits to swap to: 0=23:20, 1=16:19, 2=15:12, 3=11:8, 4=4:7, 5=0:3.
                For example, to swap port bits 15:12 to bits 4:7: pSwapTable[2]=4

                UInt8 pMirrorTable[6]

                Each table position 0 to 5 represents a group of 4 port bits:
                [0]=23:20, [1]=16:19, [2]=15:12, [3]=11:8, [4]=4:7, [5]=0:3.
                Cell values are 0 to 2 (Not Mirrored, Mirrored, No Change).
                For example, to mirror port bits 11:8 to bits 8:11:
                pMirrorTable[3]=1.
 */
tmErrorCode_t
tmbslHdmiTxVideoInSetMapping
(
    tmUnitSelect_t  txUnit,
    UInt8           *pSwapTable,
    UInt8           *pMirrorTable
);

/*============================================================================*/

#define HDMITX_ENABLE_VP_TABLE_LEN	3
#define HDMITX_GROUND_VP_TABLE_LEN	3

/**
    \brief      Set video input port (enable, ground)

    \param[in]  txUnit                Transmitter unit number
    \param[in]  pEnaVideoPortTable    Pointer to 3-byte video port enable table
    \param[in]  pGndVideoPortTable    Pointer to 3-byte video port ground table  

    \return     The call result:
                - TM_OK: the call was successful
                - Else a problem has been detected:
                  - TMBSL_ERR_HDMI_BAD_UNIT_NUMBER: bad transmitter unit number
                  - TMBSL_ERR_HDMI_BAD_PARAMETER: a parameter was out of range
                  - TMBSL_ERR_HDMI_NOT_INITIALIZED: transmitter not initialized
                  - TMBSL_ERR_HDMI_I2C_WRITE: failed when writing to the I2C bus

    \note       UInt8 pEnaVideoPortTable[3]
                
                Each table position 0 to 2 represents a group of 8 port bits:
                [0]=7:0, [1]=15:8, [2]=23:16
                bitn = '1' means enable port n
				bitn = '0' means disable port n
				For example, to enable port 0 to 7 only : pEnaVideoPortTable[0]= 0xFF
                pEnaVideoPortTable[1]= 0x00, pEnaVideoPortTable[2]= 0x00    
                
				UInt8 pGndVideoPortTable[3]

                Each table position 0 to 2 represents a group of 8 port bits:
                [0]=7:0, [1]=15:8, [2]=23:16
                bitn = '1' means pulldown port n
				bitn = '0' means not pulldown port n
				For example, to pulldown port 8 to 15 only : pEnaVideoPortTable[0]= 0x00
                pEnaVideoPortTable[1]= 0xFF, pEnaVideoPortTable[2]= 0x00
 */
tmErrorCode_t
tmbslHdmiTxSetVideoPortConfig
(
    tmUnitSelect_t  txUnit,
	UInt8			*pEnaVideoPortTable,
	UInt8			*pGndVideoPortTable
);

/*============================================================================*/
/**
    \brief      Set audio input port (enable, ground)

    \param[in]  txUnit                Transmitter unit number
    \param[in]  pEnaAudioPortTable    Pointer to 1-byte audio port enable configuration
    \param[in]  pGndAudioPortTable    Pointer to 1-byte audio port ground configuration  

    \return     The call result:
                - TM_OK: the call was successful
                - Else a problem has been detected:
                  - TMBSL_ERR_HDMI_BAD_UNIT_NUMBER: bad transmitter unit number
                  - TMBSL_ERR_HDMI_BAD_PARAMETER: a parameter was out of range
                  - TMBSL_ERR_HDMI_NOT_INITIALIZED: transmitter not initialized
                  - TMBSL_ERR_HDMI_I2C_WRITE: failed when writing to the I2C bus

    \note       UInt8 pEnaAudioPortTable[1]
                bitn = '1' means enable port n
				bitn = '0' means disable port n
				For example, to enable all audio port (0:7) : pEnaAudioPortTable[0]= 0xFF
                
				UInt8 pGndAudioPortTable[1]
                bitn = '1' means pulldown port n
				bitn = '0' means not pulldown port n
				For example, to pulldown audio port (0:7) : pEnaAudioPortTable[0]= 0xFF
*/
tmErrorCode_t
tmbslHdmiTxSetAudioPortConfig
(
    tmUnitSelect_t  txUnit,
	UInt8			*pEnaAudioPortTable,
	UInt8			*pGndAudioPortTable
);

/*============================================================================*/
/**
 * tmbslHdmiTxVideoInSetSyncAuto() parameter types
 */
/** Sync source - was Embedded sync HDMITX_PIXEMBSYNC_ */
typedef enum
{
    HDMITX_SYNCSRC_EMBEDDED  = 0,        /**< Embedded sync */
    HDMITX_SYNCSRC_EXT_VREF  = 1,        /**< External sync Vref, Href, Fref */
    HDMITX_SYNCSRC_EXT_VS    = 2,        /**< External sync Vs, Hs */
    HDMITX_SYNCSRC_NO_CHANGE = 3,        /**< No Change     */
    HDMITX_SYNCSRC_INVALID   = 4         /**< Invalid       */
} tmbslHdmiTxSyncSource_t;

/**
    \brief      Configure video input sync automatically

    \param[in]  txUnit       Transmitter unit number
    \param[in]  syncSource   Sync Source:
                             Embedded, External Vref, External Vs
                             No Change
    \param[in]  vinFmt       EIA/CEA Video input format: 1 to 31, 0 = No Change
    \param[in]  vinMode      Input video mode

    \return     The call result:
                - TM_OK: the call was successful
                - Else a problem has been detected:
                  - TMBSL_ERR_HDMI_BAD_UNIT_NUMBER: bad transmitter unit number
                  - TMBSL_ERR_HDMI_BAD_PARAMETER: a parameter was out of range
                  - TMBSL_ERR_HDMI_NOT_INITIALIZED: transmitter not initialized
                  - TMBSL_ERR_HDMI_I2C_WRITE: failed when writing to the I2C bus
 */
tmErrorCode_t
tmbslHdmiTxVideoInSetSyncAuto
(
    tmUnitSelect_t            txUnit,
    tmbslHdmiTxSyncSource_t   syncSource,
    tmbslHdmiTxVidFmt_t       vinFmt,
    tmbslHdmiTxVinMode_t      vinMode

);


/*============================================================================*/
/**
 * tmbslHdmiTxVideoInSetSyncManual() parameter types
 */
/** Video output frame pixel values */
enum _tmbslHdmiTxVoutFinePixelLimits
{
    HDMITX_VOUT_FINE_PIXEL_MIN        = 0x0000,
    HDMITX_VOUT_FINE_PIXEL_MAX        = 0x1FFF,
    HDMITX_VOUT_FINE_PIXEL_NO_CHANGE  = 0x2000,
    HDMITX_VOUT_FINE_PIXEL_INVALID    = 0x2001
};

/** Video output frame line values */
enum _tmbslHdmiTxVoutFineLineLimits
{
    HDMITX_VOUT_FINE_LINE_MIN         = 0x0000,
    HDMITX_VOUT_FINE_LINE_MAX         = 0x07FF,
    HDMITX_VOUT_FINE_LINE_NO_CHANGE   = 0x0800,
    HDMITX_VOUT_FINE_LINE_INVALID     = 0x0801
};
/**
    \brief      Configure video input sync with manual parameters

    \param[in]  txUnit       Transmitter unit number
    \param[in]  syncSource   Sync Source:
                             Embedded, External Vref, External Vs
                             No Change
    \param[in]  syncMethod   Sync method: V And H, V And X-DE, No Change
    \param[in]  toggleV      VS Toggle:
                             No Action, Toggle VS/Vref, No Change
    \param[in]  toggleH      HS Toggle:
                             No Action, Toggle HS/Href, No Change
    \param[in]  toggleX      DE/FREF Toggle:
                             No Action, Toggle DE/Fref, No Change
    \param[in]  uRefPix      Ref. pixel preset 0 to 1FFFh (2000h = No Change)
    \param[in]  uRefLine     Ref. line preset 0 to 7FFh (800h = No Change)

    \return     The call result:
                - TM_OK: the call was successful
                - Else a problem has been detected:
                  - TMBSL_ERR_HDMI_BAD_UNIT_NUMBER: bad transmitter unit number
                  - TMBSL_ERR_HDMI_BAD_PARAMETER: a parameter was out of range
                  - TMBSL_ERR_HDMI_NOT_INITIALIZED: transmitter not initialized
                  - TMBSL_ERR_HDMI_I2C_WRITE: failed when writing to the I2C bus
 */
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
);


/*============================================================================*/
/**
    \brief      Enable or disable output video frame

    \param[in]  txUnit      Transmitter unit number
    \param[in]  bDisable    Enable or disable scaler input

    \return     The call result:
                - TM_OK: the call was successful
                - Else a problem has been detected:
                  - TMBSL_ERR_HDMI_BAD_UNIT_NUMBER: bad transmitter unit number
                  - TMBSL_ERR_HDMI_BAD_PARAMETER: a parameter was out of range
                  - TMBSL_ERR_HDMI_NOT_INITIALIZED: transmitter not initialized
                  - TMBSL_ERR_HDMI_I2C_WRITE: failed when writing to the I2C bus
 */
tmErrorCode_t
tmbslHdmiTxVideoOutDisable
(
    tmUnitSelect_t  txUnit,
    Bool            bDisable
);


/*============================================================================*/
/**
 * tmbslHdmiTxVideoOutSetConfig() parameter types
 */
/** Prefilter */
typedef enum
{
    HDMITX_VOUT_PREFIL_OFF         = 0,    /**< Off */
    HDMITX_VOUT_PREFIL_121         = 1,    /**< 121 */
    HDMITX_VOUT_PREFIL_109         = 2,    /**< 109 */
    HDMITX_VOUT_PREFIL_CCIR601     = 3,    /**< CCIR601   */
    HDMITX_VOUT_PREFIL_NO_CHANGE   = 4,    /**< No Change */
    HDMITX_VOUT_PREFIL_INVALID     = 5     /**< Invalid   */
} tmbslHdmiTxVoutPrefil_t;

/** YUV blanking */
typedef enum
{
    HDMITX_VOUT_YUV_BLNK_16        = 0,    /**< 16 */
    HDMITX_VOUT_YUV_BLNK_0         = 1,    /**< 0  */
    HDMITX_VOUT_YUV_BLNK_NO_CHANGE = 2,    /**< No Change */
    HDMITX_VOUT_YUV_BLNK_INVALID   = 3     /**< Invalid   */
} tmbslHdmiTxVoutYuvBlnk_t;

/** Video quantization range */
typedef enum
{
    HDMITX_VOUT_QRANGE_FS          = 0,    /**< Full Scale */
    HDMITX_VOUT_QRANGE_RGB_YUV     = 1,    /**< RGB Or YUV */
    HDMITX_VOUT_QRANGE_YUV         = 2,    /**< YUV        */
    HDMITX_VOUT_QRANGE_NO_CHANGE   = 3,    /**< No Change  */
    HDMITX_VOUT_QRANGE_INVALID     = 4     /**< Invalid    */
} tmbslHdmiTxVoutQrange_t;

/**
    \brief      Configure sink type, configure video output colour and
                quantization, control the downsampler, and force RGB output
                and mute audio in DVI mode

    \param[in]  txUnit          Transmitter unit number:
    \param[in]  sinkType        Sink device type: DVI or HDMI or copy from EDID
    \param[in]  voutMode        Video output mode
    \param[in]  preFilter       Prefilter: Off, 121, 109, CCIR601, No Change
    \param[in]  yuvBlank        YUV blanking: 16, 0, No Change
    \param[in]  quantization    Video quantization range:
                                Full Scale, RGB Or YUV, YUV, No Change


    \return     The call result:
                - TM_OK: the call was successful
                - Else a problem has been detected:
                  - TMBSL_ERR_HDMI_BAD_UNIT_NUMBER: bad transmitter unit number
                  - TMBSL_ERR_HDMI_BAD_PARAMETER: a parameter was out of range
                  - TMBSL_ERR_HDMI_NOT_INITIALIZED: transmitter not initialized
                  - TMBSL_ERR_HDMI_I2C_WRITE: failed when writing to the I2C bus
 */
tmErrorCode_t
tmbslHdmiTxVideoOutSetConfig
(
    tmUnitSelect_t            txUnit,
    tmbslHdmiTxSinkType_t     sinkType,
    tmbslHdmiTxVoutMode_t     voutMode,
    tmbslHdmiTxVoutPrefil_t   preFilter,
    tmbslHdmiTxVoutYuvBlnk_t  yuvBlank,
    tmbslHdmiTxVoutQrange_t   quantization 
);


/*============================================================================*/
/**
 * tmbslHdmiTxVideoOutSetSync() parameter types
 */
/** Video sync source */
typedef enum
{
    HDMITX_VSSRC_INTERNAL      = 0,    /**< Internal  */
    HDMITX_VSSRC_EXTERNAL      = 1,    /**< External  */
    HDMITX_VSSRC_NO_CHANGE     = 2,    /**< No change */
    HDMITX_VSSRC_INVALID       = 3     /**< Invalid   */
} tmbslHdmiTxVsSrc_t;
 
/** Video sync toggle */
typedef enum
{
    HDMITX_VSTGL_TABLE         = 0,    /**< Vs/Hs polarity from table */
    HDMITX_VSTGL_UNUSED_1      = 1,    /**< Unused          */
    HDMITX_VSTGL_UNUSED_2      = 2,    /**< Unused          */
    HDMITX_VSTGL_UNUSED_3      = 3,    /**< Unused          */
    HDMITX_VSTGL_NO_ACTION     = 4,    /**< No toggle       */
    HDMITX_VSTGL_HS            = 5,    /**< Toggle Hs       */
    HDMITX_VSTGL_VS            = 6,    /**< Toggle Vs       */
    HDMITX_VSTGL_HS_VS         = 7,    /**< Toggle Hs & Vs  */
    HDMITX_VSTGL_NO_CHANGE     = 8,    /**< No change       */
    HDMITX_VSTGL_INVALID       = 9     /**< Invalid         */
} tmbslHdmiTxVsTgl_t;

/**
    \brief      Set video synchronization

    \param[in]  txUnit      Transmitter unit number
    \param[in]  srcH        Horizontal sync source: Internal, Exter'l, No Change
    \param[in]  srcV        Vertical sync source: Internal, Exter'l, No Change
    \param[in]  srcX        X sync source: Internal, Exter'l, No Change
    \param[in]  toggle      Sync toggle: Hs, Vs, Off, No Change
    \param[in]  once        Line/pixel counters sync once or each frame

    \return     The call result:
                - TM_OK: the call was successful
                - Else a problem has been detected:
                  - TMBSL_ERR_HDMI_BAD_UNIT_NUMBER: bad transmitter unit number
                  - TMBSL_ERR_HDMI_BAD_PARAMETER: a parameter was out of range
                  - TMBSL_ERR_HDMI_NOT_INITIALIZED: transmitter not initialized
                  - TMBSL_ERR_HDMI_I2C_WRITE: failed when writing to the I2C bus
 */
tmErrorCode_t
tmbslHdmiTxVideoOutSetSync
(
    tmUnitSelect_t          txUnit,
    tmbslHdmiTxVsSrc_t      srcH,
    tmbslHdmiTxVsSrc_t      srcV,
    tmbslHdmiTxVsSrc_t      srcX,
    tmbslHdmiTxVsTgl_t      toggle,
    tmbslHdmiTxVsOnce_t     once
);


/*============================================================================*/
/**
 * tmbslHdmiTxVideoSetInOut() parameter types
 */
/** Pixel repetition values */
enum _tmbslHdmiTxPixRepeat
{
    HDMITX_PIXREP_NONE       = 0,    /**< No repetition  */
    HDMITX_PIXREP_MIN        = 0,    /**< 1 repetition   */
    HDMITX_PIXREP_MAX        = 9,    /**< 10 repetitions */
    HDMITX_PIXREP_DEFAULT    = 10,   /**< Default repetitions for output format */
    HDMITX_PIXREP_NO_CHANGE  = 11,   /**< No change */
    HDMITX_PIXREP_INVALID    = 12    /**< Invalid   */
};

/** Matrix modes */
typedef enum
{
    HDMITX_MATMODE_OFF       = 0,    /**< Off  */
    HDMITX_MATMODE_AUTO      = 1,    /**< Auto */
    HDMITX_MATMODE_NO_CHANGE = 2,    /**< No change */
    HDMITX_MATMODE_INVALID   = 3     /**< Invalid   */
} tmbslHdmiTxMatMode_t;
 
/** Datapath bitwidth */
typedef enum
{
    HDMITX_VOUT_DBITS_12           = 0,    /**< 12 bits */
    HDMITX_VOUT_DBITS_8            = 1,    /**< 8 bits  */
    HDMITX_VOUT_DBITS_10           = 2,    /**< 10 bits */
    HDMITX_VOUT_DBITS_NO_CHANGE    = 3,    /**< No change */
    HDMITX_VOUT_DBITS_INVALID      = 4     /**< Invalid   */
} tmbslHdmiTxVoutDbits_t;

/**
    \brief      Set main video input and output parameters

    \param[in]  txUnit       Transmitter unit number
    \param[in]  vinFmt       EIA/CEA Video input format: 1 to 31, 0 = No Change
    \param[in]  scaMode      Scaler mode: Off, On, Auto, No Change
    \param[in]  voutFmt      EIA/CEA Video output format: 1 to 31, 0 = No Change
    \param[in]  uPixelRepeat Pixel repetition factor: 0 to 9, 10 = default,
                             11 = no change
    \param[in]  matMode      Matrix mode: 0 = off, 1 = auto
    \param[in]  datapathBits Datapath bitwidth: 0 to 3 (8, 10, 12, No Change)

    \return     The call result:
                - TM_OK: the call was successful
                - Else a problem has been detected:
                  - TMBSL_ERR_HDMI_BAD_UNIT_NUMBER: bad transmitter unit number
                  - TMBSL_ERR_HDMI_BAD_PARAMETER: a parameter was out of range
                  - TMBSL_ERR_HDMI_NOT_INITIALIZED: transmitter not initialized
                  - TMBSL_ERR_HDMI_INCONSISTENT_PARAMS: params are inconsistent
                  - TMBSL_ERR_HDMI_I2C_WRITE: failed when writing to the I2C bus
 */
tmErrorCode_t
tmbslHdmiTxVideoSetInOut
(
    tmUnitSelect_t          txUnit,
    tmbslHdmiTxVidFmt_t     vinFmt,
    tmbslHdmiTxScaMode_t    scaMode,
    tmbslHdmiTxVidFmt_t     voutFmt,
    UInt8                   uPixelRepeat,
    tmbslHdmiTxMatMode_t    matMode,
    tmbslHdmiTxVoutDbits_t  datapathBits,
    tmbslHdmiTxVQR_t        dviVqr 
);

/**
    \brief      Use only for debug to flag the software debug interrupt

    \param[in]  txUnit       Transmitter unit number

    \return     The call result:
                - TM_OK: the call was successful
                - Else a problem has been detected:
                  - TMBSL_ERR_HDMI_BAD_UNIT_NUMBER: bad transmitter unit number
                  - TMBSL_ERR_HDMI_NOT_INITIALIZED: transmitter not initialized
                  - TMBSL_ERR_HDMI_I2C_WRITE: failed when writing to the I2C bus
 */
tmErrorCode_t
tmbslHdmiTxFlagSwInt
(
    tmUnitSelect_t	            txUnit
)
;


/**
    \brief Return the category of equipement connected

    \param txUnit   Transmitter unit number
    \param category return category type

    \return The call result:
            - TM_OK: the call was successful
            - TMBSL_ERR_HDMI_BAD_UNIT_NUMBER: the unit number is wrong or
              the receiver instance is not initialised
            - TMBSL_ERR_HDMI_INCONSISTENT_PARAMS: params are inconsistent
            - TMBSL_ERR_HDMI_OPERATION_NOT_PERMITTED hdcp not started

*/
tmErrorCode_t
tmbslHdmiTxHdcpGetSinkCategory
(
    tmUnitSelect_t              txUnit,
    tmbslHdmiTxSinkCategory_t   *category
    );


#ifdef __cplusplus
}
#endif

#endif /* TMBSLHDMITX_H */
/*============================================================================*/
/*                            END OF FILE                                     */
/*============================================================================*/

