 /**
 * Copyright (C) 2007 NXP N.V., All Rights Reserved.
 * This source code and any compilation or derivative thereof is the proprietary
 * information of NXP N.V. and is confidential in nature. Under no circumstances
 * is this software to be  exposed to or placed under an Open Source License of
 * any type without the expressed written permission of NXP N.V.
 *
 * \file          tmdlHdmiTx_Types.h
 *
 * \version       $Revision: 1 $
 *
 * \date          $Date: 02/08/07 08:32 $
 *
 * \brief         devlib driver component API for the TDA998x HDMI Transmitters
 *
 * \section refs  Reference Documents
 * HDMI Tx Driver - FRS.doc,
 * HDMI Tx Driver - tmdlHdmiTx - SCS.doc
 *
 * \section info  Change Information
 *
 * \verbatim

   $History: tmdlHdmiTx_Types.h $
 *
 * *****************  Version 1  *****************
 * User: Demoment     Date: 02/08/07   Time: 08:32
 * Updated in $/Source/tmdlHdmiTx/inc
 * initial version

   \endverbatim
 *
*/

#ifndef TMDLHDMITX_TYPES_H
#define TMDLHDMITX_TYPES_H

/*============================================================================*/
/*                       INCLUDE FILES                                        */
/*============================================================================*/

#include "tmNxTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

/*============================================================================*/
/*                       MACRO DEFINITIONS                                    */
/*============================================================================*/

/*============================================================================*/
/*                                DEFINES                                     */
/*============================================================================*/

/**< Error Codes */
#define TMDL_ERR_DLHDMITX_BASE                      CID_DL_HDMITX
#define TMDL_ERR_DLHDMITX_COMPATIBILITY             (TMDL_ERR_DLHDMITX_BASE + TM_ERR_COMPATIBILITY)             /**< SW Interface compatibility   */
#define TMDL_ERR_DLHDMITX_MAJOR_VERSION             (TMDL_ERR_DLHDMITX_BASE + TM_ERR_MAJOR_VERSION)             /**< SW Major Version error       */
#define TMDL_ERR_DLHDMITX_COMP_VERSION              (TMDL_ERR_DLHDMITX_BASE + TM_ERR_COMP_VERSION)              /**< SW component version error   */
#define TMDL_ERR_DLHDMITX_BAD_UNIT_NUMBER           (TMDL_ERR_DLHDMITX_BASE + TM_ERR_BAD_UNIT_NUMBER)           /**< Invalid device unit number   */
#define TMDL_ERR_DLHDMITX_BAD_INSTANCE              (TMDL_ERR_DLHDMITX_BASE + TM_ERR_BAD_INSTANCE)              /**< Bad input instance value     */
#define TMDL_ERR_DLHDMITX_BAD_HANDLE                (TMDL_ERR_DLHDMITX_BASE + TM_ERR_BAD_HANDLE)                /**< Bad input handle             */
#define TMDL_ERR_DLHDMITX_BAD_PARAMETER             (TMDL_ERR_DLHDMITX_BASE + TM_ERR_BAD_PARAMETER)             /**< Invalid input parameter      */
#define TMDL_ERR_DLHDMITX_NO_RESOURCES              (TMDL_ERR_DLHDMITX_BASE + TM_ERR_NO_RESOURCES)              /**< Resource is not available    */
#define TMDL_ERR_DLHDMITX_RESOURCE_OWNED            (TMDL_ERR_DLHDMITX_BASE + TM_ERR_RESOURCE_OWNED)            /**< Resource is already in use   */
#define TMDL_ERR_DLHDMITX_RESOURCE_NOT_OWNED        (TMDL_ERR_DLHDMITX_BASE + TM_ERR_RESOURCE_NOT_OWNED)        /**< Caller does not own resource */
#define TMDL_ERR_DLHDMITX_INCONSISTENT_PARAMS       (TMDL_ERR_DLHDMITX_BASE + TM_ERR_INCONSISTENT_PARAMS)       /**< Inconsistent input params    */
#define TMDL_ERR_DLHDMITX_NOT_INITIALIZED           (TMDL_ERR_DLHDMITX_BASE + TM_ERR_NOT_INITIALIZED)           /**< Component is not initialized */
#define TMDL_ERR_DLHDMITX_NOT_SUPPORTED             (TMDL_ERR_DLHDMITX_BASE + TM_ERR_NOT_SUPPORTED)             /**< Function is not supported    */
#define TMDL_ERR_DLHDMITX_INIT_FAILED               (TMDL_ERR_DLHDMITX_BASE + TM_ERR_INIT_FAILED)               /**< Initialization failed        */
#define TMDL_ERR_DLHDMITX_BUSY                      (TMDL_ERR_DLHDMITX_BASE + TM_ERR_BUSY)                      /**< Component is busy            */
#define TMDL_ERR_DLHDMITX_I2C_READ                  (TMDL_ERR_DLHDMITX_BASE + TM_ERR_READ)                      /**< Read error                   */
#define TMDL_ERR_DLHDMITX_I2C_WRITE                 (TMDL_ERR_DLHDMITX_BASE + TM_ERR_WRITE)                     /**< Write error                  */
#define TMDL_ERR_DLHDMITX_FULL                      (TMDL_ERR_DLHDMITX_BASE + TM_ERR_FULL)                      /**< Queue is full                */
#define TMDL_ERR_DLHDMITX_NOT_STARTED               (TMDL_ERR_DLHDMITX_BASE + TM_ERR_NOT_STARTED)               /**< Function is not started      */
#define TMDL_ERR_DLHDMITX_ALREADY_STARTED           (TMDL_ERR_DLHDMITX_BASE + TM_ERR_ALREADY_STARTED)           /**< Function is already started  */
#define TMDL_ERR_DLHDMITX_ASSERTION                 (TMDL_ERR_DLHDMITX_BASE + TM_ERR_ASSERTION)                 /**< Assertion failure            */
#define TMDL_ERR_DLHDMITX_INVALID_STATE             (TMDL_ERR_DLHDMITX_BASE + TM_ERR_INVALID_STATE)             /**< Invalid state for function   */
#define TMDL_ERR_DLHDMITX_OPERATION_NOT_PERMITTED   (TMDL_ERR_DLHDMITX_BASE + TM_ERR_OPERATION_NOT_PERMITTED)   /**< Corresponds to posix EPERM   */
#define TMDL_ERR_DLHDMITX_RESOLUTION_UNKNOWN        (TMDL_ERR_DLHDMITX_BASE + TM_ERR_BAD_FORMAT)                /**< Bad format                   */

/* Defined here for tmdlHdmiTx_CmpTst */
#define TMDL_ERR_DLHDMITX_COMP                      TMDL_ERR_DLHDMITX_BASE | TM_ERR_COMP_UNIQUE_START
#define TMDL_DLHDMITX_HDCP_SECURE                   (TMDL_ERR_DLHDMITX_COMP + 0x0001) /**< Revocation list is secure */
#define TMDL_DLHDMITX_HDCP_NOT_SECURE               (TMDL_ERR_DLHDMITX_COMP + 0x0002) /**< Revocation list is NOT secure */

/*============================================================================*/
/*                       ENUM OR TYPE DEFINITIONS                             */
/*============================================================================*/

/**
 * \brief Enum listing all events that can be signalled to application
 */
typedef enum
{
    TMDL_HDMITX_HDCP_ACTIVE         = 0,    /**< HDCP encryption status switched to active */
    TMDL_HDMITX_HDCP_INACTIVE       = 1,    /**< HDCP encryption status switched to inactive */
    TMDL_HDMITX_HPD_ACTIVE          = 2,    /**< Hotplug status switched to active */
    TMDL_HDMITX_HPD_INACTIVE        = 3,    /**< Hotplug status switched to inactive */
    TMDL_HDMITX_RX_KEYS_RECEIVED    = 4,    /**< Receiver(s) key(s) received */
    TMDL_HDMITX_RX_DEVICE_ACTIVE    = 5,    /**< Rx device is connected and active */
    TMDL_HDMITX_RX_DEVICE_INACTIVE  = 6,    /**< Rx device is connected but inactive (standby) */
    TMDL_HDMITX_EDID_RECEIVED       = 7,    /**< EDID has been received */
    TMDL_HDMITX_VS_RPT_RECEIVED     = 8     /**< VS interrupt has been received */              
} tmdlHdmiTxEvent_t;

/**
 * \brief Enum listing all available event status
 */
typedef enum
{
    TMDL_HDMITX_EVENT_ENABLED,  /**< Event is enabled */
    TMDL_HDMITX_EVENT_DISABLED  /**< Event is disabled */
} tmdlHdmiTxEventStatus_t;

/**
 * \brief Callback function pointer type, used to allow driver to callback
          application when activity status is changing at input.
 * \param Event Identifier of the source event.
 */
typedef void (*ptmdlHdmiTxCallback_t) (tmdlHdmiTxEvent_t event);

/**
 * \brief Enum listing all supported device versions
 */
 typedef enum
 {
     TMDL_HDMITX_DEVICE_UNKNOWN,  /**< HW device is unknown */
     TMDL_HDMITX_DEVICE_TDA9984,  /**< HW device is IC TDA9984 */
     TMDL_HDMITX_DEVICE_TDA9989,  /**< HW device is IC TDA9989 */
     TMDL_HDMITX_DEVICE_LIPP4200, /**< HW device is IP LIPP4200 */
     TMDL_HDMITX_DEVICE_TDA9981,  /**< HW device is IC TDA9981 */
     TMDL_HDMITX_DEVICE_TDA9983   /**< HW device is IC TDA9983 */
 } tmdlHdmiTxDeviceVersion_t;

/**
 * \brief Enum defining the supported HDMI standard version
 */
typedef enum
{
   TMDL_HDMITX_HDMI_VERSION_UNKNOWN, /**< Unknown   */
   TMDL_HDMITX_HDMI_VERSION_1_1,     /**< HDMI 1.1  */
   TMDL_HDMITX_HDMI_VERSION_1_2a,    /**< HDMI 1.2a */
   TMDL_HDMITX_HDMI_VERSION_1_3a     /**< HDMI 1.3  */
} tmdlHdmiTxHdmiVersion_t;

/**
 * \brief Enum listing all color depth (8 bits/color, 10 bits/color, etc.)
 */
typedef enum
{
    TMDL_HDMITX_COLORDEPTH_24   = 0,    /**< 8 bits per color */
    TMDL_HDMITX_COLORDEPTH_30   = 1,    /**< 10 bits per color */
    TMDL_HDMITX_COLORDEPTH_36   = 2,    /**< 12 bits per color */
    TMDL_HDMITX_COLORDEPTH_48   = 3,    /**< 16 bits per color */
} tmdlHdmiTxColorDepth_t;

/**
 * \brief Enum defining the EDID Status
 */
typedef enum
{
    TMDL_HDMITX_EDID_READ                = 0,   /**< All blocks read OK */
    TMDL_HDMITX_EDID_READ_INCOMPLETE     = 1,   /**< All blocks read OK but buffer too small to return all of them */
    TMDL_HDMITX_EDID_ERROR_CHK_BLOCK_0   = 2,   /**< Block 0 checksum error */
    TMDL_HDMITX_EDID_ERROR_CHK           = 3,   /**< Block 0 OK, checksum error in one or more other blocks */
    TMDL_HDMITX_EDID_NOT_READ            = 4,   /**< EDID not read */
    TMDL_HDMITX_EDID_STATUS_INVALID      = 5    /**< Invalid   */
} tmdlHdmiTxEdidStatus_t;

/**
 * \brief Structure defining the supported audio packets
 */
typedef struct
{
   Bool HBR;              /**< High Bitrate Audio packet */
   Bool DST;              /**< Direct Stream Transport audio packet */
   Bool oneBitAudio;      /**< One Bit Audio sample packet */
} tmdlHdmiTxAudioPacket_t;

/**
 * \brief Enum listing all possible audio input formats
 */
typedef enum
{
    TMDL_HDMITX_AFMT_SPDIF      = 0, /**< SPDIF */
    TMDL_HDMITX_AFMT_I2S        = 1, /**< I2S */
    TMDL_HDMITX_AFMT_OBA        = 2, /**< One bit audio / DSD */
    TMDL_HDMITX_AFMT_DST        = 3, /**< DST */
    TMDL_HDMITX_AFMT_HBR        = 4  /**< HBR */
} tmdlHdmiTxAudioFormat_t;

/**
 * \brief Enum listing all possible audio input sample rates
 */
typedef enum
{
    TMDL_HDMITX_AFS_32K           = 0, /**< 32kHz    */
    TMDL_HDMITX_AFS_44K           = 1, /**< 44.1kHz  */
    TMDL_HDMITX_AFS_48K           = 2, /**< 48kHz    */
    TMDL_HDMITX_AFS_88K           = 3, /**< 88.2kHz  */
    TMDL_HDMITX_AFS_96K           = 4, /**< 96kHz    */
    TMDL_HDMITX_AFS_176K          = 5, /**< 176.4kHz */
    TMDL_HDMITX_AFS_192K          = 6  /**< 192kHz   */
} tmdlHdmiTxAudioRate_t;

/**
 * \brief Enum listing all possible audio input sample rates
 */
typedef enum
{
    TMDL_HDMITX_I2SQ_16BITS       = 16, /**< 16 bits */
    TMDL_HDMITX_I2SQ_32BITS       = 32, /**< 32 bits */
    TMDL_HDMITX_I2SQ_OTHERS       = 0,  /**< for SPDIF and DSD */
} tmdlHdmiTxAudioI2SQualifier_t;

/**
 * \brief Enum listing all possible audio I2S formats
 */
typedef enum
{
    TMDL_HDMITX_I2SFOR_PHILIPS_L   = 0,	/**< Philips like format */
    TMDL_HDMITX_I2SFOR_OTH_L       = 2,	/**< Other non Philips left justified */
    TMDL_HDMITX_I2SFOR_OTH_R	   = 3,	/**< Other non Philips right justified */
    TMDL_HDMITX_I2SFOR_INVALID     = 4	/**< Invalid format */
} tmdlHdmiTxAudioI2SFormat_t;

/**
 * \brief Enum listing all possible DST data transfer rates 
 */
typedef enum
{
    TMDL_HDMITX_DSTRATE_SINGLE  = 0,    /**< Single transfer rate */
    TMDL_HDMITX_DSTRATE_DOUBLE  = 1     /**< Double data rate */
} tmdlHdmiTxDstRate_t;

/**
 * \brief Structure describing unit capabilities
 */
typedef struct
{
    tmdlHdmiTxDeviceVersion_t   deviceVersion;  /**< HW device version */
    tmdlHdmiTxHdmiVersion_t     hdmiVersion;    /**< Supported HDMI standard version  */
    tmdlHdmiTxAudioPacket_t     audioPacket;    /**< Supported audio packets */
    tmdlHdmiTxColorDepth_t      colorDepth;     /**< Supported color depth */
    Bool                        hdcp;           /**< Supported Hdcp encryption (True/False) */
	Bool                        scaler;         /**< Supported scaler (True/False) */
} tmdlHdmiTxCapabilities_t;

/**
 * \brief Structure gathering all instance setup parameters
 */
typedef struct
{
    Bool    simplayHd;          /**< Enable simplayHD support */
    Bool    repeaterEnable;     /**< Enable repeater mode */
    UInt8   *pEdidBuffer;       /**< Pointer to raw EDID data */
    UInt32  edidBufferSize;     /**< Size of buffer for raw EDID data */
} tmdlHdmiTxInstanceSetupInfo_t;

/**
 * \brief Enum listing all IA/CEA 861-D video formats
 */
typedef enum
{
    TMDL_HDMITX_VFMT_NULL               = 0,    /**< Not a valid format...        */
    TMDL_HDMITX_VFMT_NO_CHANGE          = 0,    /**< ...or no change required     */
    TMDL_HDMITX_VFMT_MIN                = 1,    /**< Lowest valid format          */
    TMDL_HDMITX_VFMT_TV_MIN             = 1,    /**< Lowest valid TV format       */
    TMDL_HDMITX_VFMT_01_640x480p_60Hz   = 1,    /**< Format 01 640  x 480p  60Hz  */
    TMDL_HDMITX_VFMT_02_720x480p_60Hz   = 2,    /**< Format 02 720  x 480p  60Hz  */
    TMDL_HDMITX_VFMT_03_720x480p_60Hz   = 3,    /**< Format 03 720  x 480p  60Hz  */
    TMDL_HDMITX_VFMT_04_1280x720p_60Hz  = 4,    /**< Format 04 1280 x 720p  60Hz  */
    TMDL_HDMITX_VFMT_05_1920x1080i_60Hz = 5,    /**< Format 05 1920 x 1080i 60Hz  */
    TMDL_HDMITX_VFMT_06_720x480i_60Hz   = 6,    /**< Format 06 720  x 480i  60Hz  */
    TMDL_HDMITX_VFMT_07_720x480i_60Hz   = 7,    /**< Format 07 720  x 480i  60Hz  */
    TMDL_HDMITX_VFMT_08_720x240p_60Hz   = 8,    /**< Format 08 720  x 240p  60Hz  */
    TMDL_HDMITX_VFMT_09_720x240p_60Hz   = 9,    /**< Format 09 720  x 240p  60Hz  */
    TMDL_HDMITX_VFMT_10_720x480i_60Hz   = 10,   /**< Format 10 720  x 480i  60Hz  */
    TMDL_HDMITX_VFMT_11_720x480i_60Hz   = 11,   /**< Format 11 720  x 480i  60Hz  */
    TMDL_HDMITX_VFMT_12_720x240p_60Hz   = 12,   /**< Format 12 720  x 240p  60Hz  */
    TMDL_HDMITX_VFMT_13_720x240p_60Hz   = 13,   /**< Format 13 720  x 240p  60Hz  */
    TMDL_HDMITX_VFMT_14_1440x480p_60Hz  = 14,   /**< Format 14 1440 x 480p  60Hz  */
    TMDL_HDMITX_VFMT_15_1440x480p_60Hz  = 15,   /**< Format 15 1440 x 480p  60Hz  */
    TMDL_HDMITX_VFMT_16_1920x1080p_60Hz = 16,   /**< Format 16 1920 x 1080p 60Hz  */
    TMDL_HDMITX_VFMT_17_720x576p_50Hz   = 17,   /**< Format 17 720  x 576p  50Hz  */
    TMDL_HDMITX_VFMT_18_720x576p_50Hz   = 18,   /**< Format 18 720  x 576p  50Hz  */
    TMDL_HDMITX_VFMT_19_1280x720p_50Hz  = 19,   /**< Format 19 1280 x 720p  50Hz  */
    TMDL_HDMITX_VFMT_20_1920x1080i_50Hz = 20,   /**< Format 20 1920 x 1080i 50Hz  */
    TMDL_HDMITX_VFMT_21_720x576i_50Hz   = 21,   /**< Format 21 720  x 576i  50Hz  */
    TMDL_HDMITX_VFMT_22_720x576i_50Hz   = 22,   /**< Format 22 720  x 576i  50Hz  */
    TMDL_HDMITX_VFMT_23_720x288p_50Hz   = 23,   /**< Format 23 720  x 288p  50Hz  */
    TMDL_HDMITX_VFMT_24_720x288p_50Hz   = 24,   /**< Format 24 720  x 288p  50Hz  */
    TMDL_HDMITX_VFMT_25_720x576i_50Hz   = 25,   /**< Format 25 720  x 576i  50Hz  */
    TMDL_HDMITX_VFMT_26_720x576i_50Hz   = 26,   /**< Format 26 720  x 576i  50Hz  */
    TMDL_HDMITX_VFMT_27_720x288p_50Hz   = 27,   /**< Format 27 720  x 288p  50Hz  */
    TMDL_HDMITX_VFMT_28_720x288p_50Hz   = 28,   /**< Format 28 720  x 288p  50Hz  */
    TMDL_HDMITX_VFMT_29_1440x576p_50Hz  = 29,   /**< Format 29 1440 x 576p  50Hz  */
    TMDL_HDMITX_VFMT_30_1440x576p_50Hz  = 30,   /**< Format 30 1440 x 576p  50Hz  */
    TMDL_HDMITX_VFMT_31_1920x1080p_50Hz = 31,   /**< Format 31 1920 x 1080p 50Hz  */
    TMDL_HDMITX_VFMT_32_1920x1080p_24Hz = 32,   /**< Format 32 1920 x 1080p 24Hz  */
    TMDL_HDMITX_VFMT_33_1920x1080p_25Hz = 33,   /**< Format 33 1920 x 1080p 25Hz  */
    TMDL_HDMITX_VFMT_34_1920x1080p_30Hz = 34,   /**< Format 34 1920 x 1080p 30Hz  */
    TMDL_HDMITX_VFMT_TV_MAX             = 34,   /**< Highest valid TV format      */
    TMDL_HDMITX_VFMT_TV_NO_REG_MIN      = 32,   /**< Lowest TV format without prefetched table */
    TMDL_HDMITX_VFMT_TV_NUM             = 35,   /**< Number of TV formats & null  */
    TMDL_HDMITX_VFMT_PC_MIN             = 128,  /**< Lowest valid PC format       */
    TMDL_HDMITX_VFMT_PC_640x480p_60Hz   = 128,  /**< PC format 128                */
    TMDL_HDMITX_VFMT_PC_800x600p_60Hz   = 129,  /**< PC format 129                */
    TMDL_HDMITX_VFMT_PC_1152x960p_60Hz  = 130,  /**< PC format 130                */
    TMDL_HDMITX_VFMT_PC_1024x768p_60Hz  = 131,  /**< PC format 131                */
    TMDL_HDMITX_VFMT_PC_1280x768p_60Hz  = 132,  /**< PC format 132                */
    TMDL_HDMITX_VFMT_PC_1280x1024p_60Hz = 133,  /**< PC format 133                */
    TMDL_HDMITX_VFMT_PC_1360x768p_60Hz  = 134,  /**< PC format 134                */
    TMDL_HDMITX_VFMT_PC_1400x1050p_60Hz = 135,  /**< PC format 135                */
    TMDL_HDMITX_VFMT_PC_1600x1200p_60Hz = 136,  /**< PC format 136                */
    TMDL_HDMITX_VFMT_PC_1024x768p_70Hz  = 137,  /**< PC format 137                */
    TMDL_HDMITX_VFMT_PC_640x480p_72Hz   = 138,  /**< PC format 138                */
    TMDL_HDMITX_VFMT_PC_800x600p_72Hz   = 139,  /**< PC format 139                */
    TMDL_HDMITX_VFMT_PC_640x480p_75Hz   = 140,  /**< PC format 140                */
    TMDL_HDMITX_VFMT_PC_1024x768p_75Hz  = 141,  /**< PC format 141                */
    TMDL_HDMITX_VFMT_PC_800x600p_75Hz   = 142,  /**< PC format 142                */
    TMDL_HDMITX_VFMT_PC_1024x864p_75Hz  = 143,  /**< PC format 143                */
    TMDL_HDMITX_VFMT_PC_1280x1024p_75Hz = 144,  /**< PC format 144                */
    TMDL_HDMITX_VFMT_PC_640x350p_85Hz   = 145,  /**< PC format 145                */
    TMDL_HDMITX_VFMT_PC_640x400p_85Hz   = 146,  /**< PC format 146                */
    TMDL_HDMITX_VFMT_PC_720x400p_85Hz   = 147,  /**< PC format 147                */
    TMDL_HDMITX_VFMT_PC_640x480p_85Hz   = 148,  /**< PC format 148                */
    TMDL_HDMITX_VFMT_PC_800x600p_85Hz   = 149,  /**< PC format 149                */
    TMDL_HDMITX_VFMT_PC_1024x768p_85Hz  = 150,  /**< PC format 150                */
    TMDL_HDMITX_VFMT_PC_1152x864p_85Hz  = 151,  /**< PC format 151                */
    TMDL_HDMITX_VFMT_PC_1280x960p_85Hz  = 152,  /**< PC format 152                */
    TMDL_HDMITX_VFMT_PC_1280x1024p_85Hz = 153,  /**< PC format 153                */
    TMDL_HDMITX_VFMT_PC_1024x768i_87Hz  = 154,  /**< PC format 154                */
    TMDL_HDMITX_VFMT_PC_MAX             = 154,  /**< Highest valid PC format      */
    TMDL_HDMITX_VFMT_PC_NUM             = (1+154-128)   /**< Number of PC formats         */
} tmdlHdmiTxVidFmt_t;

/**
 * \brief Structure defining the EDID short video descriptor
 */
typedef struct
{
    tmdlHdmiTxVidFmt_t  videoFormat;            /**< Video format as defined by EIA/CEA 861-D */
    Bool                nativeVideoFormat;   /**< True if format is the preferred video format */
} tmdlHdmiTxShortVidDesc_t;

/**
 * \brief Enum listing all picture aspect ratio (H:V) (4:3, 16:9)
 */
typedef enum
{
    TMDL_HDMITX_P_ASPECT_RATIO_UNDEFINED    = 0,    /**< Undefined picture aspect ratio */
    TMDL_HDMITX_P_ASPECT_RATIO_6_5          = 1,    /**< 6:5 picture aspect ratio (PAR) */
    TMDL_HDMITX_P_ASPECT_RATIO_5_4          = 2,    /**< 5:4 PAR */
    TMDL_HDMITX_P_ASPECT_RATIO_4_3          = 3,    /**< 4:3 PAR */
    TMDL_HDMITX_P_ASPECT_RATIO_16_10        = 4,    /**< 16:10 PAR */
    TMDL_HDMITX_P_ASPECT_RATIO_5_3          = 5,    /**< 5:3 PAR */
    TMDL_HDMITX_P_ASPECT_RATIO_16_9         = 6,    /**< 16:9 PAR */
    TMDL_HDMITX_P_ASPECT_RATIO_9_5          = 7,    /**< 9:5 PAR */
} tmdlHdmiTxPictAspectRatio_t;

/**
 * \brief Enum listing all vertical frequency
 */
typedef enum
{
    TMDL_HDMITX_VFREQ_24Hz      = 0,    /**< 24Hz          */
    TMDL_HDMITX_VFREQ_25Hz      = 1,    /**< 25Hz          */
    TMDL_HDMITX_VFREQ_30Hz      = 2,    /**< 30Hz          */
    TMDL_HDMITX_VFREQ_50Hz      = 3,    /**< 50Hz          */
    TMDL_HDMITX_VFREQ_59Hz      = 4,    /**< 59.94Hz       */
    TMDL_HDMITX_VFREQ_60Hz      = 5,    /**< 60Hz          */
#ifndef FORMAT_PC
    TMDL_HDMITX_VFREQ_INVALID   = 6,    /**< Invalid       */
    TMDL_HDMITX_VFREQ_NUM       = 6     /**< No. of values */
#else /* FORMAT_PC */
    TMDL_HDMITX_VFREQ_70Hz      = 6,    /**< 70Hz          */
    TMDL_HDMITX_VFREQ_72Hz      = 7,    /**< 72Hz          */
    TMDL_HDMITX_VFREQ_75Hz      = 8,    /**< 75Hz          */
    TMDL_HDMITX_VFREQ_85Hz      = 9,    /**< 85Hz          */
    TMDL_HDMITX_VFREQ_87Hz      = 10,   /**< 87Hz          */
    TMDL_HDMITX_VFREQ_INVALID   = 11,   /**< Invalid       */
    TMDL_HDMITX_VFREQ_NUM       = 11    /**< No. of values */
#endif /* FORMAT_PC */
} tmdlHdmiTxVfreq_t;

/**
 * \brief Structure storing specifications of a video resolution
 */
typedef struct
{
    UInt16                      width;         /**< Width of the frame in pixels */
    UInt16                      height;        /**< Height of the frame in pixels */
    Bool                        interlaced;    /**< Interlaced mode (True/False) */
    tmdlHdmiTxVfreq_t           vfrequency;    /**< Vertical frequency in Hz */
    tmdlHdmiTxPictAspectRatio_t aspectRatio;   /**< Picture aspect ratio (H:V) */
} tmdlHdmiTxVidFmtSpecs_t;

/**
 * \brief Enum listing all video input modes (CCIR, RGB, etc.)
 */
typedef enum
{
    TMDL_HDMITX_VINMODE_CCIR656     = 0,    /**< CCIR656 */
    TMDL_HDMITX_VINMODE_RGB444      = 1,    /**< RGB444  */
    TMDL_HDMITX_VINMODE_YUV444      = 2,    /**< YUV444  */
    TMDL_HDMITX_VINMODE_YUV422      = 3,    /**< YUV422  */
    TMDL_HDMITX_VINMODE_NO_CHANGE   = 4,    /**< No change */
    TMDL_HDMITX_VINMODE_INVALID     = 5     /**< Invalid */
} tmdlHdmiTxVinMode_t;

/**
 * \brief Enum listing all possible sync sources
 */
typedef enum
{
    TMDL_HDMITX_SYNCSRC_EMBEDDED = 0, /**< Embedded sync */
    TMDL_HDMITX_SYNCSRC_EXT_VREF = 1, /**< External sync Vref, Href, Fref */
    TMDL_HDMITX_SYNCSRC_EXT_VS   = 2, /**< External sync Vs, Hs */
} tmdlHdmiTxSyncSource_t;

/**
 * \brief Enum listing all output pixel rate (Single, Double, etc.)
 */
typedef enum
{
    TMDL_HDMITX_PIXRATE_DOUBLE          = 0,        /**< Double pixel rate */
    TMDL_HDMITX_PIXRATE_SINGLE          = 1,        /**< Single pixel rate */
    TMDL_HDMITX_PIXRATE_SINGLE_REPEATED = 2,        /**< Single pixel repeated */
} tmdlHdmiTxPixRate_t;

/**
 * \brief Structure defining the video input configuration
 */
typedef struct
{
   tmdlHdmiTxVidFmt_t       format;     /**< Video format as defined by EIA/CEA 861-D */
   tmdlHdmiTxVinMode_t      mode;       /**< Video mode (CCIR, RGB, YUV, etc.) */
   tmdlHdmiTxSyncSource_t   syncSource; /**< Sync source type */
   tmdlHdmiTxPixRate_t      pixelRate;  /**< Pixel rate */
} tmdlHdmiTxVideoInConfig_t;

/**
 * \brief Enum listing all video output modes (YUV, RGB, etc.)
 */
typedef enum
{
    TMDL_HDMITX_VOUTMODE_RGB444     = 0,    /**< RGB444    */
    TMDL_HDMITX_VOUTMODE_YUV422     = 1,    /**< YUV422    */
    TMDL_HDMITX_VOUTMODE_YUV444     = 2,    /**< YUV444    */
} tmdlHdmiTxVoutMode_t;

/**
 * \brief Enum defining possible quantization range
 */
typedef enum
{
    TMDL_HDMITX_VQR_DEFAULT = 0, /* Follow HDMI spec. */
    TMDL_HDMITX_RGB_FULL    = 1, /* Force RGB FULL , DVI only */
    TMDL_HDMITX_RGB_LIMITED = 2  /* Force RGB LIMITED , DVI only */
} tmdlHdmiTxVQR_t;

/**
 * \brief Structure defining the video output configuration
 */
typedef struct
{
   tmdlHdmiTxVidFmt_t       format;     /**< Video format as defined by EIA/CEA 861-D */
   tmdlHdmiTxVoutMode_t     mode;       /**< Video mode (CCIR, RGB, YUV, etc.) */
   tmdlHdmiTxColorDepth_t   colorDepth; /**< Color depth */
   tmdlHdmiTxVQR_t          dviVqr;     /**< VQR applied in DVI mode */ 
} tmdlHdmiTxVideoOutConfig_t;

/**
 * \brief Structure defining the audio input configuration
 */
typedef struct
{
   tmdlHdmiTxAudioFormat_t          format;         /**< Audio format (I2S, SPDIF, etc.) */
   tmdlHdmiTxAudioRate_t            rate;           /**< Audio sampling rate */
   tmdlHdmiTxAudioI2SFormat_t       i2sFormat;      /**< I2S format of the audio input */
   tmdlHdmiTxAudioI2SQualifier_t    i2sQualifier;   /**< I2S qualifier of the audio input (8,16,32 bits) */
   tmdlHdmiTxDstRate_t              dstRate;        /**< DST data transfer rate */
   UInt8                            channelAllocation;  /**< Ref to CEA-861D p85 */
} tmdlHdmiTxAudioInConfig_t;

/**
 * \brief Enum listing all the type of sunk
 */
typedef enum
{
    TMDL_HDMITX_SINK_DVI  = 0, /**< DVI  */
    TMDL_HDMITX_SINK_HDMI = 1, /**< HDMI */
    TMDL_HDMITX_SINK_EDID = 2, /**< As currently defined in EDID */
} tmdlHdmiTxSinkType_t;

/**
 * \brief Structure defining the content of a gamut packet
 */
typedef struct
{
    Bool   nextField;           /**< Gamut relevant for field following packet insertion */
    UInt8  GBD_Profile;         /**< Profile of the gamut packet : 0 = P0, 1 = P1 */
    UInt8  affectedGamutSeqNum; /**< Gamut sequence number of the field that have to be affected by this gamut packet */
    Bool   noCurrentGBD;        /**< Current field not using specific gamut */
    UInt8  currentGamutSeqNum;  /**< Gamut sequence number of the current field */
    UInt8  packetSequence;      /**< Sequence of the packet inside a multiple packet gamut */
    UInt8  payload[28];         /**< Payload of the gamut packet */
} tmdlHdmiTxGamutData_t;

/**
 * \brief Type defining the content of a generic packet
 */
typedef UInt8 tmdlHdmiTxGenericPacket[28];

/**
 * \brief Structure defining the content of an ACP packet
 */
typedef struct
{
    UInt8 acpType;
    UInt8 acpData[28];
} tmdlHdmiTxAcpPktData_t;

/**
 * \brief Structure defining the content of an AVI infoframe
 */
typedef struct
{
    UInt8  colorIndicator;                /**< RGB or YCbCr indicator. See CEA-861-B table 8 for details */
    UInt8  activeInfoPresent;             /**< Active information present. Indicates if activeFormatAspectRatio field is valid */
    UInt8  barInformationDataValid;       /**< Bar information data valid */
    UInt8  scanInformation;               /**< Scan information. See CEA-861-B table 8 for details */
    UInt8  colorimetry;                   /**< Colorimetry. See CEA-861-B table 9 for details */
    UInt8  pictureAspectRatio;            /**< Picture aspect ratio. See CEA-861-B table 9 for details */
    UInt8  activeFormatAspectRatio;       /**< Active Format aspect ratio. See CEA-861-B table 10 and Annex H for details */
    UInt8  nonUniformPictureScaling;      /**< Non-uniform picture scaling. See CEA-861-B table 11 for details */
    UInt8  videoFormatIdentificationCode; /**< Video format indentification code. See CEA-861-B section 6.3 for details */
    UInt8  pixelRepetitionFactor;         /**< Pixel repetition factor. See CEA-861-B table 11 for details */
    UInt16 lineNumberEndTopBar;
    UInt16 lineNumberStartBottomBar;
    UInt16 lineNumberEndLeftBar;
    UInt16 lineNumberStartRightBar;
} tmdlHdmiTxAviIfData_t;

/**
 * \brief Structure defining the content of an ACP packet
 */
typedef struct
{
    Bool avMute;
} tmdlHdmiTxGcpPktData_t;

/**
 * \brief Structure defining the content of an AUD infoframe
 */
typedef struct
{
    UInt8 codingType;        /**< Coding type (always set to zero) */
    UInt8 channelCount;      /**< Channel count. See CEA-861-B table 17 for details */
    UInt8 samplefrequency;   /**< Sample frequency. See CEA-861-B table 18 for details */
    UInt8 sampleSize;        /**< Sample frequency. See CEA-861-B table 18 for details */
    UInt8 channelAllocation; /**< Channel allocation. See CEA-861-B section 6.3.2 for details */
    Bool  downmixInhibit;    /**< Downmix inhibit. See CEA-861-B section 6.3.2 for details */
    UInt8 levelShiftValue;   /**< level shift value for downmixing. See CEA-861-B section 6.3.2 and table 23 for details */
} tmdlHdmiTxAudIfData_t;

/**
 * \brief Structure defining the content of an ISRC1 packet
 */
typedef struct
{
    Bool  isrcCont;         /**< ISRC packet continued in next packet */
    Bool  isrcValid;        /**< Set to one when ISRCStatus and UPC_EAN_ISRC_xx are valid */
    UInt8 isrcStatus;       /**< ISRC status */
    UInt8 UPC_EAN_ISRC[16]; /**< ISRC packet data */
} tmdlHdmiTxIsrc1PktData_t;

/**
 * \brief Structure defining the content of an ISRC2 packet
 */
typedef struct
{
    UInt8 UPC_EAN_ISRC[16];  /**< ISRC packet data */
} tmdlHdmiTxIsrc2PktData_t;

/**
 * \brief Structure defining the content of an MPS infoframe
 */
typedef struct
{
    UInt32 bitRate;         /**< MPEG bit rate in Hz */
    UInt32 frameType;       /**< MPEG frame type */
    Bool   fieldRepeat;     /**< 0: new field, 1:repeated field */
} tmdlHdmiTxMpsIfData_t;

/**
 * \brief Structure defining the content of an SPD infoframe
 */
typedef struct
{
    UInt8   vendorName[8];   /**< Vendor name */
    UInt8   productDesc[16]; /**< Product Description */
    UInt32  sourceDevInfo;   /**< Source Device Info */
} tmdlHdmiTxSpdIfData_t;

/**
 * \brief Structure defining the content of a VS packet
 */
typedef struct
{
    UInt8 version;
    UInt8 vsData[27];
} tmdlHdmiTxVsPktData_t;

/**
 * \brief Structure defining the Edid audio descriptor
 */
typedef struct
{
    UInt8 format;         /* EIA/CEA861 mode */
    UInt8 channels;       /* number of channels */
    UInt8 supportedFreqs; /* bitmask of supported frequencies */
    UInt8 supportedRes;   /* bitmask of supported resolutions (LPCM only) */
    UInt8 maxBitrate;     /* Maximum bitrate divided by 8KHz (compressed formats only) */
} tmdlHdmiTxEdidAudioDesc_t;

/**
 * \brief Structure defining detailed timings of a video format
 */
typedef struct
{
    UInt16  pixelClock;        /**< Pixel Clock/10 000         */
    UInt16  hActivePixels;     /**< Horizontal Active Pixels   */
    UInt16  hBlankPixels;      /**< Horizontal Blanking Pixels */
    UInt16  vActiveLines;      /**< Vertical Active Lines      */
    UInt16  vBlankLines;       /**< Vertical Blanking Lines    */
    UInt16  hSyncOffset;       /**< Horizontal Sync Offset     */
    UInt16  hSyncWidth;        /**< Horiz. Sync Pulse Width    */
    UInt16  vSyncOffset;       /**< Vertical Sync Offset       */
    UInt16  vSyncWidth;        /**< Vertical Sync Pulse Width  */
    UInt16  hImageSize;        /**< Horizontal Image Size      */
    UInt16  vImageSize;        /**< Vertical Image Size        */
    UInt16  hBorderPixels;     /**< Horizontal Border          */
    UInt16  vBorderPixels;     /**< Vertical Border            */
    UInt8   flags;             /**< Interlace/sync info        */
} tmdlHdmiTxEdidVideoTimings_t;

/** size descriptor block of monitor descriptor */
#define EDID_MONITOR_DESCRIPTOR_SIZE   13

/**
 * \brief Structure defining the first monitor descriptor
 */
typedef struct
{   
    Bool    descRecord;                                 /**< True when parameters of struct are available   */
    UInt8   monitorName[EDID_MONITOR_DESCRIPTOR_SIZE];  /**< Monitor Name                                   */
} tmdlHdmiTxEdidFirstMD_t;

/**
 * \brief Structure defining the second monitor descriptor
 */
typedef struct
{
    Bool    descRecord;             /**< True when parameters of struct are available   */
    UInt8   minVerticalRate;        /**< Min vertical rate in Hz                        */
    UInt8   maxVerticalRate;        /**< Max vertical rate in Hz                        */
    UInt8   minHorizontalRate;      /**< Min horizontal rate in Hz                      */
    UInt8   maxHorizontalRate;      /**< Max horizontal rate in Hz                      */
    UInt8   maxSupportedPixelClk;   /**< Max suuported pixel clock rate in MHz          */
} tmdlHdmiTxEdidSecondMD_t;

/**
 * \brief Structure defining the other monitor descriptor
 */
typedef struct
{
    Bool    descRecord;                                     /**< True when parameters of struct are available   */
    UInt8   otherDescriptor[EDID_MONITOR_DESCRIPTOR_SIZE];  /**< Other monitor Descriptor                       */
} tmdlHdmiTxEdidOtherMD_t;

/**
 * \brief Test pattern types
 */
typedef enum
{
    TMDL_HDMITX_PATTERN_OFF     = 0, /**< Insert test pattern       */
    TMDL_HDMITX_PATTERN_CBAR4   = 1, /**< Insert 4-bar colour bar   */
    TMDL_HDMITX_PATTERN_CBAR8   = 2, /**< Insert 8-bar colour bar   */
    TMDL_HDMITX_PATTERN_BLUE	= 3, /**< Insert Blue screen        */
    TMDL_HDMITX_PATTERN_BLACK   = 4, /**< Insert Black screen       */
    TMDL_HDMITX_PATTERN_INVALID = 5  /**< Invalid pattern		    */
} tmdlHdmiTxTestPattern_t;

/**
 * \brief Enum listing all hdcp state
 */
typedef enum
{
    TMDL_HDMITX_HDCP_CHECK_NOT_STARTED       = 0,    /**< Check not started */
    TMDL_HDMITX_HDCP_CHECK_IN_PROGRESS       = 1,    /**< No failures, more to do */
    TMDL_HDMITX_HDCP_CHECK_PASS              = 2,    /**< Final check has passed */
    TMDL_HDMITX_HDCP_CHECK_FAIL_FIRST        = 3,    /**< First check failure code */
    TMDL_HDMITX_HDCP_CHECK_FAIL_DRIVER_STATE = 3,    /**< Driver not AUTHENTICATED */
    TMDL_HDMITX_HDCP_CHECK_FAIL_DEVICE_T0    = 4,    /**< A T0 interrupt occurred */
    TMDL_HDMITX_HDCP_CHECK_FAIL_DEVICE_RI    = 5,    /**< Device RI changed */
    TMDL_HDMITX_HDCP_CHECK_FAIL_DEVICE_FSM   = 6,    /**< Device FSM not 10h */
    TMDL_HDMITX_HDCP_CHECK_NUM               = 7     /**< Number of check results */
}tmdlHdmiTxHdcpCheck_t;

/**
 * \brief Enum listing all hdcp option flags
 */
typedef enum
{
    TMDL_HDMITX_HDCP_OPTION_FORCE_PJ_IGNORED    = 0x01, /* Not set: obey PJ result     */
    TMDL_HDMITX_HDCP_OPTION_FORCE_SLOW_DDC      = 0x02, /* Not set: obey BCAPS setting */
    TMDL_HDMITX_HDCP_OPTION_FORCE_NO_1_1        = 0x04, /* Not set: obey BCAPS setting */
    TMDL_HDMITX_HDCP_OPTION_FORCE_REPEATER      = 0x08, /* Not set: obey BCAPS setting */
    TMDL_HDMITX_HDCP_OPTION_FORCE_NO_REPEATER   = 0x10, /* Not set: obey BCAPS setting */
    TMDL_HDMITX_HDCP_OPTION_FORCE_V_EQU_VBAR    = 0x20, /* Not set: obey V=V' result   */
    TMDL_HDMITX_HDCP_OPTION_FORCE_VSLOW_DDC     = 0x40, /* Set: 50kHz DDC */
    TMDL_HDMITX_HDCP_OPTION_DEFAULT             = 0x00, /* All the above Not Set vals */
    TMDL_HDMITX_HDCP_OPTION_MASK                = 0x7F, /* Only these bits are allowed */
    TMDL_HDMITX_HDCP_OPTION_MASK_BAD            = 0x80  /* These bits are not allowed  */
}tmdlHdmiTxHdcpOptions_t;

#ifndef NO_HDCP
/** KSV list sizes */
typedef enum
{
    TMDL_HDMITX_KSV_LIST_MAX_DEVICES = 128,
    TMDL_HDMITX_KSV_BYTES_PER_DEVICE = 5
} tmdlHdmiTxHdcpHandleSHA_1;

/**
 * \brief Structure defining information about hdcp
 */
typedef struct
{
    tmdlHdmiTxHdcpCheck_t   hdcpCheckState;                                 /* Hdcp check state */
    UInt8                   hdcpErrorState;                                 /* Error State when T0 occured */
    Bool                    bKsvSecure;                                     /* BKSV is secured */
    UInt8                   hdcpBksv[TMDL_HDMITX_KSV_BYTES_PER_DEVICE];     /* BKSV read from B sink */
    UInt8                   hdcpKsvList[TMDL_HDMITX_KSV_BYTES_PER_DEVICE *
                                        TMDL_HDMITX_KSV_LIST_MAX_DEVICES];  /* KSV list read from B sink during
                                                                            SHA-1 interrupt */
    UInt8                   hdcpKsvDevices;                                 /* Number of devices read from
                                                                            B sink during SHA-1 interrupt */
    UInt8                   hdcpDeviceDepth;                               /* Connection tree depth */
    Bool                    hdcpMaxCascExceeded;
    Bool                    hdcpMaxDevsExceeded;
} tmdlHdmiTxHdcpInfo_t;
#endif /* NO_HDCP */

typedef enum
{
    TMDL_HDMITX_HOTPLUG_INACTIVE    = 0,    /**< Hotplug inactive */
    TMDL_HDMITX_HOTPLUG_ACTIVE      = 1,    /**< Hotplug active   */
    TMDL_HDMITX_HOTPLUG_INVALID     = 2     /**< Invalid Hotplug  */
} tmdlHdmiTxHotPlug_t;


// kyw2029
typedef enum
{
	IMAGE_GALLERY_MODE =0,
	VIDEO_PLAY_MODE = 1,
	NORMAL_MODE = 2
}PLAY_MODE_t;
// kyw2029

#ifdef __cplusplus
}
#endif

#endif /* TMDLHDMITX_TYPES_H */

/*============================================================================*/
/*                            END OF FILE                                     */
/*============================================================================*/

