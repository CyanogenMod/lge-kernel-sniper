/**
 * Copyright (C) 2006 NXP N.V., All Rights Reserved.
 * This source code and any compilation or derivative thereof is the proprietary
 * information of NXP N.V. and is confidential in nature. Under no circumstances
 * is this software to be  exposed to or placed under an Open Source License of
 * any type without the expressed written permission of NXP N.V.
 *
 * \file          tmbslTDA9984_edid.c
 *
 * \version       Revision: 11
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

   History: tmbslTDA9984_edid.c
 *
 * **************** Version 12  ******************
 * User: G.Burnouf     Date: 06/03/08
 * Updated in $/Source/tmbslTDA9984/Src
 * PR1415 : Solve problem DTD parsing of block 1
 *
 * **************** Version 11  ******************
 * User: G.Burnouf     Date: 05/02/08
 * Updated in $/Source/tmbslTDA9984/Src
 * PR1251 : add function tmbslTDA9984EdidGetBasicDisplayParam
 *
 * **************** Version 10  ******************
 * User: G.Burnouf     Date: 22/01/08
 * Updated in $/Source/tmbslTDA9984/Src
 * PR1271 : remove qmore warning
 *
 * **************** Version 9  ******************
 * User: G.Burnouf     Date: 10/12/07   Time: 08:30
 * Updated in $/Source/tmbslTDA9984/Src
 * PR1145 : return DTD and monitor descriptor
 *
 * *****************  Version 8  ****************
 * User: G.Burnouf     Date: 13/11/07   Time: 10:15
 * Updated in $/Source/tmbslTDA9984/Src
 * PR1008 : Update function tmbslTDA9984HwGetCapabilities
 *
 * *****************  Version 7  ****************
 * User: B.Vereecke     Date: 31/10/07   Time: 10:51
 * Updated in $/Source/tmbslTDA9984/Src
 * PR848 : Remove qmore errors
 *
 * *****************  Version 6  *****************
 * User: B.Vereecke     Date: 26/10/07   Time: 15:50
 * Updated in $/Source/tmbslTDA9984/Src
 * PR850 : Remove external library dependancy
 * 
 * *****************  Version 5  ****************
 * User: B.Vereecke     Date: 11/10/07   Time: 14:51
 * Updated in $/Source/tmbslTDA9984/Src
 * PR814 : Remove compilation warnings
 *
 * *****************  Version 4  *****************
 * User: B. Vereecke    Date: 09/10/07   Time: 09:26
 * Updated in $/Source/tmbslTDA9984/Src
 * PR 623 : EDID absence detection
 *
 * *****************  Version 3  *****************
 * User: B. Vereecke    Date: 28/09/07   Time: 15:32
 * Updated in $/Source/tmbslTDA9984/Src
 * PR 766 : Rename pseudo API functions
 *
 * *****************  Version 2  *****************
 * User: G. Burnouf    Date: 06/08/07   Time: 17:00
 * Updated in $/Source/tmbslTDA9984/Src
 * PR 570 : New local function to stop an EDID read
 *          ClearEdidRequest
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
#include "tmbslTDA9984_Edid_l.h"


/*============================================================================*/
/*                     TYPES DECLARATIONS                                     */
/*============================================================================*/


/*============================================================================*/
/*                       CONSTANTS DECLARATIONS                               */
/*============================================================================*/

#define EDID_NUMBER_MAX_DTD_BLK_1           6
/** EDID block 0 parse start point */
#define EDID_BLK0_BASE_DTD                  0x36

#define EDID_BLK1_OFFSET_BASE_DTD           2

/** EDID block 0 extension block count */
#define EDID_BLK0_EXT_CNT                   0x7E

/** EDID extension block parse start point */
#define EDID_BLK_EXT_BASE                   0x04

/** CEA extension block type */
#define EDID_CEA_EXTENSION                  0x02

/** CEA Block Map */
#define EDID_BLOCK_MAP                      0xF0

/** NB Max of descriptor DTD or monitor in block 0 */
#define EDID_NB_MAX_DESCRIP_BLK_IN_BLK_0    4

#define EDID_MONITOR_NAME_DESC_DATA_TYPE    252

#define EDID_MONITOR_RANGE_DESC_DATA_TYPE   253

/*============================================================================*/
/*                       DEFINES DECLARATIONS                               */
/*============================================================================*/


/*============================================================================*/
/*                       VARIABLES DECLARATIONS                               */
/*============================================================================*/

/*============================================================================*/
/*                       FUNCTION PROTOTYPES                                  */
/*============================================================================*/

static tmErrorCode_t    requestEdidBlock(tmHdmiTxobject_t   *pDis);

static tmErrorCode_t    parseEdidBlock (tmHdmiTxobject_t *pDis, 
                                        Int blockNumber);

static Bool             storeDtdBlock (tmHdmiTxobject_t    *pDis,
                                        UInt8               blockPtr);

static Bool             storeMonitorDescriptor (tmHdmiTxobject_t    *pDis,
                                                UInt8                blockPtr);


/*============================================================================*/
/* tmbslTDA9984HwGetCapabilities                                                    */
/*============================================================================*/

tmErrorCode_t
tmbslTDA9984HwGetCapabilities
(
    tmUnitSelect_t          txUnit,
    tmbslHdmiTxHwFeature_t  deviceCapability,
    Bool                    *pFeatureSupported
)

{
    tmHdmiTxobject_t *pDis;                 /* Pointer to Device Instance Structure */
    tmErrorCode_t    err;                   /* Error code */

    err = TM_OK;

    /* Check unit parameter and point to its object */
    err = checkUnitSetDis(txUnit, &pDis);
    RETIF(err != TM_OK, err)

    RETIF_BADPARAM(pFeatureSupported == Null)

    *pFeatureSupported = False;

    switch (deviceCapability)
    {
        case HDMITX_FEATURE_HW_HDCP:
            if((pDis->uDeviceFeatures & E_MASKREG_P00_VERSION_not_h) == 0)
            {
                *pFeatureSupported = True;
            }
            break;
        case HDMITX_FEATURE_HW_SCALER:
            if((pDis->uDeviceFeatures & E_MASKREG_P00_VERSION_not_s) == 0)
            {
                *pFeatureSupported = True;
            }
            break;
        case HDMITX_FEATURE_HW_AUDIO_OBA:
                *pFeatureSupported = True;
            break;
        case HDMITX_FEATURE_HW_AUDIO_DST:
                *pFeatureSupported = False;
            break;
        case HDMITX_FEATURE_HW_AUDIO_HBR:
                *pFeatureSupported = True;
            break;
        case HDMITX_FEATURE_HW_HDMI_1_1:
                *pFeatureSupported = True;
            break;
        case HDMITX_FEATURE_HW_HDMI_1_2A:
                *pFeatureSupported = True;
            break;
        case HDMITX_FEATURE_HW_HDMI_1_3A:
                *pFeatureSupported = False;
            break;

        case HDMITX_FEATURE_HW_DEEP_COLOR_30:
                *pFeatureSupported = False;
            break;

        case HDMITX_FEATURE_HW_DEEP_COLOR_36:
                *pFeatureSupported = False;
            break;

        case HDMITX_FEATURE_HW_DEEP_COLOR_48:
                *pFeatureSupported = False;
            break;

        case HDMITX_FEATURE_HW_UPSAMPLER:
                *pFeatureSupported = True;
            break;

        case HDMITX_FEATURE_HW_DOWNSAMPLER:
                *pFeatureSupported = True;
            break;

        case HDMITX_FEATURE_HW_COLOR_CONVERSION:
                *pFeatureSupported = True;
            break;

        default:
            *pFeatureSupported = False;
            break;
    }

    return err;
}

/*============================================================================*/
/* tmbslTDA9984EdidGetAudioCapabilities                                        */
/*============================================================================*/
#ifndef DEMO_BUILD
tmErrorCode_t
tmbslTDA9984EdidGetAudioCapabilities
(
    tmUnitSelect_t          txUnit,
    tmbslHdmiTxEdidSad_t    *pEdidAFmts,
    UInt                    aFmtLength,
    UInt                    *pAFmtsAvail,
    UInt8                   *pAudioFlags
)
{
    tmHdmiTxobject_t    *pDis;      /* Pointer to Device Instance Structure */
    tmErrorCode_t       err;        /* Error code */
    UInt                i;          /* Loop index */

    /* Check unit parameter and point to TX unit object */
    err = checkUnitSetDis(txUnit, &pDis);
    RETIF(err != TM_OK, err)

    /* Check remaining parameter(s) */
    RETIF_BADPARAM(pEdidAFmts == Null)
    RETIF_BADPARAM(aFmtLength < 1)
    RETIF_BADPARAM(pAFmtsAvail == Null)
    RETIF_BADPARAM(pAudioFlags == Null)

    if ((pDis->EdidStatus == HDMITX_EDID_READ) ||
        (pDis->EdidStatus == HDMITX_EDID_ERROR_CHK))
    {
        /* allow if edid are read or if there are a chk error on an other block than block 0 */

        /* Copy the Device Instance Structure EdidAFmts descriptors to
        * pEdidAFmts until we run out or no more space in structure.
        */
        if (pDis->EdidSadCnt > 0)
        {
            for (i = 0; (i < (UInt)pDis->EdidSadCnt) && (i < aFmtLength); i++)
            {
                pEdidAFmts[i].ModeChans = pDis->EdidAFmts[i].ModeChans;
                pEdidAFmts[i].Freqs     = pDis->EdidAFmts[i].Freqs;
                pEdidAFmts[i].Byte3     = pDis->EdidAFmts[i].Byte3;
            }
        }
        else
        {
            /* No pEdidAFmts to copy so set a zero format to be safe */
            pEdidAFmts[0].ModeChans = 0;
            pEdidAFmts[0].Freqs     = 0;
            pEdidAFmts[0].Byte3     = 0;
        }

        /* Fill Audio Flags parameter */
        *pAudioFlags = ((pDis->EdidCeaFlags & 0x40) << 1);  /* Basic audio */
        if (pDis->EdidSinkAi == True)
        {
            *pAudioFlags += 0x40;                           /* Mask in AI support */
        }

        /* Fill number of SADs available parameter */
        *pAFmtsAvail = pDis->EdidSadCnt;
    }
    else
    {
        /* Not allowed if EdidStatus value is not valid */
        err = TMBSL_ERR_HDMI_RESOURCE_NOT_AVAILABLE; 
    }
    return err;
}
#endif /* DEMO_BUILD */

/*============================================================================*/
/* tmbslTDA9984EdidGetBlockCount                                               */
/*============================================================================*/
tmErrorCode_t
tmbslTDA9984EdidGetBlockCount
(
    tmUnitSelect_t      txUnit,
    UInt8               *puEdidBlockCount
)
{
    tmHdmiTxobject_t    *pDis;      /* Pointer to Device Instance Structure */
    tmErrorCode_t       err;        /* Error code */

    /* Check unit parameter and point to TX unit object */
    err = checkUnitSetDis(txUnit, &pDis);
    RETIF(err != TM_OK, err)

    /* Check remaining parameter(s) */
    RETIF_BADPARAM(puEdidBlockCount == Null)

    if ((pDis->EdidStatus == HDMITX_EDID_READ) ||
        (pDis->EdidStatus == HDMITX_EDID_ERROR_CHK))
    {
        /* allow if edid are read or if there are a chk error on an other block than block 0 */
        *puEdidBlockCount = pDis->EdidBlockCnt;
    }
    else
    {
        /* Not allowed if EdidStatus value is not valid */
        err = TMBSL_ERR_HDMI_RESOURCE_NOT_AVAILABLE; 
    }

    return err;
}

/*============================================================================*/
/* tmbslTDA9984EdidGetStatus                                                  */
/*============================================================================*/
tmErrorCode_t
tmbslTDA9984EdidGetStatus
(
    tmUnitSelect_t  txUnit,
    UInt8           *puEdidStatus
)
{
    tmHdmiTxobject_t    *pDis;      /* Pointer to Device Instance Structure */
    tmErrorCode_t       err;        /* Error code */

    /* Check unit parameter and point to TX unit object */
    err = checkUnitSetDis(txUnit, &pDis);
    RETIF(err != TM_OK, err)

    /* Check remaining parameter(s) */
    RETIF_BADPARAM(puEdidStatus == Null)

    if (puEdidStatus)
    {
        *puEdidStatus = pDis->EdidStatus;
    }
 
    return err;
}

/*============================================================================*/
/* tmbslTDA9984EdidRequestBlockData                                           */
/*============================================================================*/
tmErrorCode_t
tmbslTDA9984EdidRequestBlockData
(
    tmUnitSelect_t  txUnit,
    UInt8           *pRawEdid, 
    Int             numBlocks,  /* Only relevant if pRawEdid valid */
    Int             lenRawEdid  /* Only relevant if pRawEdid valid */
)
{
    tmHdmiTxobject_t    *pDis;      /* Pointer to Device Instance Structure */
    UInt8               regval;     /* Byte value write to register */
    tmErrorCode_t       err;        /* Error code */

    /* Check unit parameter and point to TX unit object */
    err = checkUnitSetDis(txUnit, &pDis);
    RETIF(err != TM_OK, err)

    /* Check remaining parameter(s)
     * We do allow a null pRawEdid pointer, in which case buffer length is 
     * irrelevant. If pRawEdid pointer is valid, there is no point in
     * continuing if insufficient space for at least one block.
     */
    RETIF_BADPARAM((pRawEdid != Null) && (lenRawEdid < EDID_BLOCK_SIZE))
    /* Sensible value of numBlocks? */
    RETIF((pRawEdid != Null) && ((numBlocks < 1) || (numBlocks > 255)),
          TMBSL_ERR_HDMI_INCONSISTENT_PARAMS)
    /* Enough space for the data requested? */
    RETIF((pRawEdid != Null) && (lenRawEdid < (numBlocks * EDID_BLOCK_SIZE)),
          TMBSL_ERR_HDMI_INCONSISTENT_PARAMS)

    /* Read the HPD pin via the hpd_in flag in the first interrupt status
    * register and return a TMBSL_ERR_HDMI_NULL_CONNECTION error if it is
    * not set.
    * We must use the flag in the Device Instance Structure to avoid 
    * clearing pending interrupt flags.
    */
    RETIF(pDis->hotPlugStatus != HDMITX_HOTPLUG_ACTIVE,
        TMBSL_ERR_HDMI_NULL_CONNECTION)

    if (pDis->EdidReadStarted == False)
    {

        /* Reset the EdidStatus in the Device Instance Structure */
        pDis->EdidStatus = HDMITX_EDID_NOT_READ;

        pDis->EdidReadStarted = True;

        /* Reset stored parameters from EDID in the Device Instance Structure */
        pDis->EdidSinkType = HDMITX_SINK_DVI;
        pDis->EdidSinkAi = False;
        pDis->EdidCeaFlags = 0;
        pDis->EdidCeaXVYCCFlags = 0;
        pDis->EdidSvdCnt = 0;
        pDis->EdidSadCnt = 0;
        pDis->EdidSourceAddress = 0;                /* 0.0.0.0 */
        pDis->NbDTDStored = 0;
        pDis->EdidFirstMonitorDescriptor.bDescRecord = False;
        pDis->EdidSecondMonitorDescriptor.bDescRecord = False;
        pDis->EdidOtherMonitorDescriptor.bDescRecord = False;

        pDis->EdidLatency.latency_available = False;
        pDis->EdidLatency.Ilatency_available = False;

        pDis->EdidExtraVsdbData.hdmiVideoPresent = False;


        pDis->EdidToApp.pRawEdid = pRawEdid;
        pDis->EdidToApp.numBlocks = numBlocks;

        /* Enable the T0 interrupt for detecting the Read_EDID failure */
        regval =    
            E_MASKREG_P00_INT_FLAGS_0_hpd   |
            E_MASKREG_P00_INT_FLAGS_0_t0 ;
        err = setHwRegister(pDis, E_REG_P00_INT_FLAGS_0_RW, regval);
        RETIF(err != TM_OK, err);

        /* Launch the read of first EDID block into Device Instance workspace */
        pDis->EdidBlockRequested = 0;
        err = requestEdidBlock(pDis);
    }
    else
    {
        /* Not allowed if read edid is on going */
        err = TMBSL_ERR_HDMI_RESOURCE_NOT_AVAILABLE; 
    }

    return err;
}


/*============================================================================*/
/* EdidBlockAvailable                                                         */
/*============================================================================*/

tmErrorCode_t
EdidBlockAvailable (tmUnitSelect_t txUnit, Bool * pSendEDIDCallback)
{

    tmErrorCode_t       err;        /* Error code */
    UInt8               chksum;     /* Checksum value */
    UInt8               LoopIndex;  /* Loop index */
    UInt8               extBlockCnt;
    tmHdmiTxobject_t    *pDis;      /* Pointer to Device Instance Structure */

    err = TM_OK;
    /* Check remaining parameter(s) */
    RETIF_BADPARAM(pSendEDIDCallback == Null)

    /* Check unit parameter and point to its object */
    err = checkUnitSetDis(txUnit, &pDis);
    RETIF(err != TM_OK, err)

    if (pDis->EdidReadStarted == True)
    {

        err = getHwRegisters(pDis, E_REG_P09_EDID_DATA_0_R, pDis->EdidBlock,
                                EDID_BLOCK_SIZE);
        RETIF(err != TM_OK, err)

        if(pSendEDIDCallback)
        {
            *pSendEDIDCallback = False;
        }

        if (pDis->EdidStatus == HDMITX_EDID_NOT_READ)
        {

            /* Add up all the values of the EDID block bytes, including the
            * checksum byte
            */
            chksum = 0;
            for (LoopIndex = 0; LoopIndex < EDID_BLOCK_SIZE; LoopIndex++)
            {
                chksum = chksum + pDis->EdidBlock[LoopIndex];
            }

            /* IF the EDID block does not yield a checksum of zero
            */
            if(chksum != 0)
            {
                if (pDis->EdidBlockRequested == 0)
                {
                    /* THEN return a HDMITX_EDID_ERROR error.*/
                    pDis->EdidStatus = HDMITX_EDID_ERROR_CHK_BLOCK_0;
                }
                else
                {
                    /* THEN return a HDMITX_EDID_ERROR_CHK error.*/
                    pDis->EdidStatus = HDMITX_EDID_ERROR_CHK;
                }
            }
        }

        if (pDis->EdidStatus == HDMITX_EDID_ERROR_CHK_BLOCK_0)
        {
            /* PR11 : On i2c error or bad checksum in block 0 */
            /* allow driver to go in state CONNECTED */
            /* On the other block, we also accept INVALID_CHECKSUM which means
            * there was a checksum error */

            if(pSendEDIDCallback)
            {
                *pSendEDIDCallback = True;
            }

            setState(pDis, EV_GETBLOCKDATA);
            if (pDis->rxSenseStatus == HDMITX_RX_SENSE_ACTIVE)
            {
                setState(pDis, EV_SINKON);
            }
            pDis->EdidReadStarted = False;
            return err;
        }

        /* Check if block 0 */
        if (pDis->EdidBlockRequested == 0)
        {
            /* Could check block 0 header (0x00,6 x 0xFF,0x00) here but not
            * certain to be future proof [CEA861C A.2.3]
            */

            /* Read block count from penultimate byte of block and store in DIS */
            extBlockCnt = pDis->EdidBlock[EDID_BLK0_EXT_CNT];
            
            pDis->EdidBlockCnt = extBlockCnt + 1;   /* Total = Block 0 + extensions */

        }

        /* If pointer was supplied, copy block from DIS to buffer */
        if (pDis->EdidToApp.pRawEdid != Null)
        {
            /* Check if we've copied as many as requested yet? */
            if (pDis->EdidBlockRequested < pDis->EdidToApp.numBlocks) 
            {
                lmemcpy(pDis->EdidToApp.pRawEdid + (pDis->EdidBlockRequested * EDID_BLOCK_SIZE),
                    pDis->EdidBlock,
                    EDID_BLOCK_SIZE);
            }
        }
        parseEdidBlock(pDis, (Int16)pDis->EdidBlockRequested);

        /* If extension blocks are present, process them */
        if ( (pDis->EdidBlockRequested + 1) < pDis->EdidBlockCnt)
        {
            pDis->EdidBlockRequested = pDis->EdidBlockRequested + 1;
            /* Launch an edid block read */
            err = requestEdidBlock(pDis);
        }
        else
        {
            if (pDis->EdidStatus == HDMITX_EDID_NOT_READ)
            {
                pDis->EdidStatus = HDMITX_EDID_READ;
            }

            if(pSendEDIDCallback)
            {
                *pSendEDIDCallback = True;
            }

            /* Disable T0 interrupt by only enabling Hot Plug Detect */
            err = setHwRegister(pDis, E_REG_P00_INT_FLAGS_0_RW, E_MASKREG_P00_INT_FLAGS_0_hpd);
            RETIF(err != TM_OK, err);

            setState(pDis, EV_GETBLOCKDATA);

            if (pDis->rxSenseStatus == HDMITX_RX_SENSE_ACTIVE)
            {
                setState(pDis, EV_SINKON);
            }
            pDis->EdidReadStarted = False;
        }
    }
    else
    {
        /* function called in an invalid state */
        err = TMBSL_ERR_HDMI_RESOURCE_NOT_AVAILABLE; 
    }

    return err;

}

/*============================================================================*/
/* ClearEdidRequest                                               */
/*============================================================================*/

tmErrorCode_t
ClearEdidRequest (tmUnitSelect_t txUnit)
{

    tmErrorCode_t       err;        /* Error code */
    tmHdmiTxobject_t    *pDis;      /* Pointer to Device Instance Structure */

    err = TM_OK;

    /* Check unit parameter and point to its object */
    err = checkUnitSetDis(txUnit, &pDis);
    RETIF(err != TM_OK, err)


    /* Disable T0 interrupt by only enabling Hot Plug Detect */
    err = setHwRegister(pDis, E_REG_P00_INT_FLAGS_0_RW, E_MASKREG_P00_INT_FLAGS_0_hpd);
    RETIF(err != TM_OK, err);


    /* Reset the EdidStatus in the Device Instance Structure */
    pDis->EdidStatus = HDMITX_EDID_NOT_READ;

    pDis->EdidReadStarted = False;

    /* Reset stored parameters from EDID in the Device Instance Structure */
    pDis->EdidSinkType = HDMITX_SINK_DVI;
    pDis->EdidSinkAi = False;

    pDis->EdidCeaFlags = 0;
    pDis->EdidCeaXVYCCFlags = 0;
    pDis->EdidSvdCnt = 0;
    pDis->EdidSadCnt = 0;
    pDis->EdidSourceAddress = 0;                /* 0.0.0.0 */
    pDis->NbDTDStored = 0;
    pDis->EdidFirstMonitorDescriptor.bDescRecord = False;
    pDis->EdidSecondMonitorDescriptor.bDescRecord = False;
    pDis->EdidOtherMonitorDescriptor.bDescRecord = False;

    pDis->EdidLatency.latency_available = False;
    pDis->EdidLatency.Ilatency_available = False;

    pDis->EdidExtraVsdbData.hdmiVideoPresent = False;

    setState(pDis, EV_GETBLOCKDATA);

    if (pDis->rxSenseStatus == HDMITX_RX_SENSE_ACTIVE)
    {
        setState(pDis, EV_SINKON);
    }

    /* Launch the read of first EDID block into Device Instance workspace */
    pDis->EdidBlockRequested = 0;


    return err;
}

/*============================================================================*/
/* tmbslTDA9984EdidGetSinkType                                                 */
/*============================================================================*/
tmErrorCode_t
tmbslTDA9984EdidGetSinkType
(
    tmUnitSelect_t              txUnit,
    tmbslHdmiTxSinkType_t      *pSinkType 
)
{
    tmHdmiTxobject_t    *pDis;      /* Pointer to Device Instance Structure */
    tmErrorCode_t       err;        /* Error code */

    /* Check unit parameter and point to TX unit object */
    err = checkUnitSetDis(txUnit, &pDis);
    RETIF(err != TM_OK, err)

    /* Check remaining parameter(s) */
    RETIF_BADPARAM(pSinkType == Null)

    if ((pDis->EdidStatus == HDMITX_EDID_READ) ||
        (pDis->EdidStatus == HDMITX_EDID_ERROR_CHK))
    {
        /* allow if edid are read or if there are a chk error on an other block than block 0 */

        *pSinkType = pDis->EdidSinkType;
    }
    else
    {
        /* Not allowed if EdidStatus value is not valid */
        err = TMBSL_ERR_HDMI_RESOURCE_NOT_AVAILABLE; 
    }
    return err;

}





/*============================================================================*/
/* tmbslTDA9984EdidSetSinkType                                                 */
/*============================================================================*/
tmErrorCode_t
tmbslTDA9984EdidSetSinkType
(
    tmUnitSelect_t              txUnit,
    tmbslHdmiTxSinkType_t       sinkType 
)
{
    tmHdmiTxobject_t    *pDis;      /* Pointer to Device Instance Structure */
    tmErrorCode_t       err;        /* Error code */

    /* Check unit parameter and point to TX unit object */
    err = checkUnitSetDis(txUnit, &pDis);
    RETIF(err != TM_OK, err)

    /* Check remaining parameter(s) */
    RETIF_BADPARAM(sinkType >= HDMITX_SINK_INVALID)



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

    return err;

}












/*============================================================================*/
/* tmbslTDA9984EdidGetSourceAddress                                            */
/*============================================================================*/
#ifndef DEMO_BUILD
tmErrorCode_t
tmbslTDA9984EdidGetSourceAddress
(
    tmUnitSelect_t  txUnit,
    UInt16          *pSourceAddress 
)
{
    tmHdmiTxobject_t    *pDis;      /* Pointer to Device Instance Structure */
    tmErrorCode_t       err;        /* Error code */

    /* Check unit parameter and point to TX unit object */
    err = checkUnitSetDis(txUnit, &pDis);
    RETIF(err != TM_OK, err)

    /* Check remaining parameter(s) */
    RETIF_BADPARAM(pSourceAddress == Null)

    if ((pDis->EdidStatus == HDMITX_EDID_READ) ||
        (pDis->EdidStatus == HDMITX_EDID_ERROR_CHK))
    {
        /* allow if edid are read or if there are a chk error on an other block than block 0 */

        *pSourceAddress = pDis->EdidSourceAddress;
    }
    else
    {
        /* Not allowed if EdidStatus value is not valid */
        err = TMBSL_ERR_HDMI_RESOURCE_NOT_AVAILABLE; 
    }
    return err;

}
#endif /* DEMO_BUILD */

/*============================================================================*/
/* tmbslTDA9984EdidGetDetailedTimingDescriptors                               */
/*============================================================================*/
tmErrorCode_t
tmbslTDA9984EdidGetDetailedTimingDescriptors
(
    tmUnitSelect_t          txUnit,
    tmbslHdmiTxEdidDtd_t    *pEdidDTD,
    UInt8                   nb_size,
    UInt8                   *pDTDAvail
)
{

    tmHdmiTxobject_t    *pDis;      /* Pointer to Device Instance Structure */
    tmErrorCode_t       err;        /* Error code */

    /* Check unit parameter and point to TX unit object */
    err = checkUnitSetDis(txUnit, &pDis);
    RETIF(err != TM_OK, err)

    /* Check remaining parameter(s) */
    RETIF_BADPARAM(pEdidDTD == Null)
    RETIF_BADPARAM(pDTDAvail == Null)
    RETIF_BADPARAM(nb_size == 0)

    if ((pDis->EdidStatus == HDMITX_EDID_READ) ||
        (pDis->EdidStatus == HDMITX_EDID_ERROR_CHK))
    {
        /* allow if edid are read or if there are a chk error on an other block than block 0 */
        if (nb_size > pDis->NbDTDStored)
        {
            *pDTDAvail = pDis->NbDTDStored;
        }
        else
        {
            *pDTDAvail = nb_size;
        }

        lmemcpy(pEdidDTD, pDis->EdidDTD, sizeof(tmbslHdmiTxEdidDtd_t) * (*pDTDAvail));
    }
    else
    {
        /* Not allowed if EdidStatus value is not valid */
        err = TMBSL_ERR_HDMI_RESOURCE_NOT_AVAILABLE; 
    }

    return TM_OK;
}

/*============================================================================*/
/* tmbslTDA9984EdidGetMonitorDescriptors                                      */
/*============================================================================*/
tmErrorCode_t
tmbslTDA9984EdidGetMonitorDescriptors
(
    tmUnitSelect_t              txUnit,
    tmbslHdmiTxEdidFirstMD_t    *pEdidFirstMD,
    tmbslHdmiTxEdidSecondMD_t   *pEdidSecondMD,
    tmbslHdmiTxEdidOtherMD_t    *pEdidOtherMD,
    UInt8                       sizeOtherMD,
    UInt8                       *pOtherMDAvail
)
{

    tmHdmiTxobject_t    *pDis;      /* Pointer to Device Instance Structure */
    tmErrorCode_t       err;        /* Error code */

    /* Check unit parameter and point to TX unit object */
    err = checkUnitSetDis(txUnit, &pDis);
    RETIF(err != TM_OK, err)

    /* Check remaining parameter(s) */
    RETIF_BADPARAM(pEdidFirstMD == Null)
    RETIF_BADPARAM(pEdidSecondMD == Null)
    RETIF_BADPARAM(pEdidOtherMD == Null)

    DUMMY_ACCESS(pOtherMDAvail);
    DUMMY_ACCESS(sizeOtherMD);

    if ((pDis->EdidStatus == HDMITX_EDID_READ) ||
        (pDis->EdidStatus == HDMITX_EDID_ERROR_CHK))
    {
        lmemcpy(pEdidFirstMD, &(pDis->EdidFirstMonitorDescriptor), sizeof(tmbslHdmiTxEdidFirstMD_t));
        lmemcpy(pEdidSecondMD, &(pDis->EdidSecondMonitorDescriptor), sizeof(tmbslHdmiTxEdidSecondMD_t));
        lmemcpy(pEdidOtherMD, &(pDis->EdidOtherMonitorDescriptor), sizeof(tmbslHdmiTxEdidOtherMD_t));
    }
    else
    {
        /* Not allowed if EdidStatus value is not valid */
        err = TMBSL_ERR_HDMI_RESOURCE_NOT_AVAILABLE; 
    }

    return TM_OK;

}

/*============================================================================*/
/* tmbslTDA9984EdidGetBasicDisplayParam                                       */
/*============================================================================*/
tmErrorCode_t
tmbslTDA9984EdidGetBasicDisplayParam
(
    tmUnitSelect_t              txUnit,
    tmbslHdmiTxEdidBDParam_t    *pEdidBDParam
)
{

    tmHdmiTxobject_t    *pDis;      /* Pointer to Device Instance Structure */
    tmErrorCode_t       err;        /* Error code */

    /* Check unit parameter and point to TX unit object */
    err = checkUnitSetDis(txUnit, &pDis);
    RETIF(err != TM_OK, err)

    /* Check remaining parameter(s) */
    RETIF_BADPARAM(pEdidBDParam == Null)

    if ((pDis->EdidStatus == HDMITX_EDID_READ) ||
        (pDis->EdidStatus == HDMITX_EDID_ERROR_CHK))
    {
        lmemcpy(pEdidBDParam, &(pDis->EDIDBasicDisplayParam), sizeof(tmbslHdmiTxEdidBDParam_t));
    }
    else
    {
        /* Not allowed if EdidStatus value is not valid */
        err = TMBSL_ERR_HDMI_RESOURCE_NOT_AVAILABLE; 
    }

    return TM_OK;
}

/*============================================================================*/
/* tmbslTDA9984EdidGetVideoCapabilities                                        */
/*============================================================================*/
tmErrorCode_t
tmbslTDA9984EdidGetVideoCapabilities
(
    tmUnitSelect_t          txUnit,
    UInt8                   *pEdidVFmts,
    UInt                    vFmtLength,
    UInt                    *pVFmtsAvail,
    UInt8                   *pVidFlags
)
{
    tmHdmiTxobject_t    *pDis;      /* Pointer to Device Instance Structure */
    tmErrorCode_t       err;        /* Error code */
    UInt                i;          /* Loop index */

    /* Check unit parameter and point to TX unit object */
    err = checkUnitSetDis(txUnit, &pDis);
    RETIF(err != TM_OK, err)

    /* Check remaining parameter(s) */
    RETIF_BADPARAM(pEdidVFmts == Null)
    RETIF_BADPARAM(vFmtLength < 1)
    RETIF_BADPARAM(pVFmtsAvail == Null)
    RETIF_BADPARAM(pVidFlags == Null)

    if ((pDis->EdidStatus == HDMITX_EDID_READ) ||
        (pDis->EdidStatus == HDMITX_EDID_ERROR_CHK))
    {
        /* allow if edid are read or if there are a chk error on an other block than block 0 */

        /* Copy the Device Instance Structure EdidVFmts descriptors to
        * pEdidVFmts until we run out or no more space in structure.
        */
        if (pDis->EdidSvdCnt > 0)
        {
            for (i = 0; (i < (UInt)pDis->EdidSvdCnt) && (i < vFmtLength); i++)
            {
                pEdidVFmts[i] = pDis->EdidVFmts[i];
            }
        }
        else
        {
            /* No pEdidVFmts to copy so set a zero format to be safe */
            pEdidVFmts[0] = HDMITX_VFMT_NULL;
        }

        /* Fill Video Flags parameter */
        *pVidFlags = (UInt8)((pDis->EdidCeaFlags & 0x80) |         /* Underscan */
                    ((pDis->EdidCeaFlags & 0x30) << 1) );  /* YUV444, YUV422 */


        /* Add info regarding xvYCC support */
        *pVidFlags = *pVidFlags | (pDis->EdidCeaXVYCCFlags & 0x03);


        /* Fill number of SVDs available parameter */
        *pVFmtsAvail = pDis->EdidSvdCnt;
    }
    else
    {
        /* Not allowed if EdidStatus value is not valid */
        err = TMBSL_ERR_HDMI_RESOURCE_NOT_AVAILABLE; 
    }

    return err;
}

/*============================================================================*/
/* tmbslTDA9984EdidGetVideoPreferred                                           */
/*============================================================================*/
#ifndef DEMO_BUILD
tmErrorCode_t
tmbslTDA9984EdidGetVideoPreferred
(
    tmUnitSelect_t         txUnit,
    tmbslHdmiTxEdidDtd_t *pEdidDTD
)
{
    tmHdmiTxobject_t    *pDis;      /* Pointer to Device Instance Structure */
    tmErrorCode_t       err;        /* Error code */

    /* Check unit parameter and point to TX unit object */
    err = checkUnitSetDis(txUnit, &pDis);
    RETIF(err != TM_OK, err)

    /* Check remaining parameter(s) */
    RETIF_BADPARAM(pEdidDTD == Null)

    if ((pDis->EdidStatus == HDMITX_EDID_READ) ||
        (pDis->EdidStatus == HDMITX_EDID_ERROR_CHK))
    {
        /* allow if edid are read or if there are a chk error on an other block than block 0 */

        /* Populate the Detailed Timing Descriptor structure pEdidDTD from
        * EdidDtd in the Device Instance Structure.
        */
        lmemcpy(pEdidDTD, &pDis->EdidDTD, sizeof(tmbslHdmiTxEdidDtd_t));
    }
    else
    {
        /* Not allowed if EdidStatus value is not valid */
        err = TMBSL_ERR_HDMI_RESOURCE_NOT_AVAILABLE; 
    }
    return err;

}
#endif /* DEMO_BUILD */

/*============================================================================*/
/*                          STATIC FUNCTION                                   */
/*============================================================================*/

/*============================================================================*/
/* requestEdidBlock - reads an entire edid block                              */
/*============================================================================*/
static tmErrorCode_t
requestEdidBlock
(   
    tmHdmiTxobject_t    *pDis      /* Device instance strucure to use */
    )
{
    tmErrorCode_t   err;        /* Error code */
    UInt8           segptr;     /* Segment ptr value */
    UInt8           offset;     /* Word offset value */

    /* Check block number is valid [CEA861C A.2.1] */
    RETIF_BADPARAM(pDis->EdidBlockRequested >= 255)

    err = setHwRegister(pDis, E_REG_P09_DDC_ADDR_RW, DDC_EDID_ADDRESS);
    RETIF_REG_FAIL(err)

    /* For even blocks we need an offset of 0, odd blocks we need 128 */
    offset = (UInt8)((((UInt8)pDis->EdidBlockRequested & 1) == 1) ? 128 : 0);

    err = setHwRegister(pDis, E_REG_P09_DDC_OFFS_RW, offset);
    RETIF_REG_FAIL(err)

    err = setHwRegister(pDis, E_REG_P09_DDC_SEGM_ADDR_RW, DDC_SGMT_PTR_ADDRESS);
    RETIF_REG_FAIL(err)

    /* Calculate which segment of the EDID we need (2 blocks per segment) */
    segptr = (UInt8)pDis->EdidBlockRequested / 2;

    err = setHwRegister(pDis, E_REG_P09_DDC_SEGM_RW, segptr);
    RETIF_REG_FAIL(err)

    /* Enable reading EDID */
    err = setHwRegister(pDis, E_REG_P09_EDID_CTRL_RW, 0x1);
    RETIF_REG_FAIL(err)

    /* The flag to start the EDID reading must cleared by software*/
    err = setHwRegister(pDis, E_REG_P09_EDID_CTRL_RW, 0x0);
    RETIF_REG_FAIL(err)

    return err;
}

/*============================================================================*/
/* parseEdidBlock                                                             */
/*============================================================================*/
static tmErrorCode_t
parseEdidBlock
(   
    tmHdmiTxobject_t    *pDis,      /* Device instance strucure holding block */
    Int                 blockNumber /* Block number */
    )
{
    UInt8           i;                  /* Loop index */
    UInt8           blockPtr, endPtr;   /* Parsing pointers */       
    UInt8           blockType, blockLength;
    Bool            dtdFound;
    UInt8           NbBlkRead, offset3D=0;

    /* Check block number is valid [CEA861C A.2.1] */
    RETIF_BADPARAM(blockNumber >= 255)

    NbBlkRead = 0;
    dtdFound = True;
    blockPtr = 0;

    if (blockNumber == 0)
    {
        pDis->EDIDBasicDisplayParam.uVideoInputDef = pDis->EdidBlock[0x14];
        pDis->EDIDBasicDisplayParam.uMaxHorizontalSize = pDis->EdidBlock[0x15];
        pDis->EDIDBasicDisplayParam.uMaxVerticalSize = pDis->EdidBlock[0x16];
        pDis->EDIDBasicDisplayParam.uGamma = pDis->EdidBlock[0x17];
        pDis->EDIDBasicDisplayParam.uFeatureSupport = pDis->EdidBlock[0x18];
        
        /* Block 0 - contains DTDs but no video data block (SVDs) */
        for (i = 0; (i < 2) && (dtdFound); i++) /* search 2 possible DTD blocks in block 0 */
        {   
            blockPtr = (UInt8)(EDID_BLK0_BASE_DTD + (i * EDID_DTD_BLK_SIZE));
            if ((blockPtr + EDID_DTD_BLK_SIZE - 1) < EDID_BLOCK_SIZE)
            {
                dtdFound = storeDtdBlock(pDis, blockPtr);
                if (dtdFound)
                {
                    NbBlkRead++;
                }
            }
        }

        dtdFound = True;

        /* Parse monitor descriptor */
        for (i = NbBlkRead; (i < EDID_NB_MAX_DESCRIP_BLK_IN_BLK_0) && (dtdFound); i++)
        {
            blockPtr = (UInt8)(EDID_BLK0_BASE_DTD + (i * EDID_DTD_BLK_SIZE));
            if ((blockPtr + EDID_DTD_BLK_SIZE - 1) < EDID_BLOCK_SIZE)
            {
                dtdFound = storeMonitorDescriptor(pDis, blockPtr);
            }
        }
    }
    else if (blockNumber >= 1)
    {   
       switch (pDis->EdidBlock[0])
       {
          /* CEA EXTENSION */
          case EDID_CEA_EXTENSION:
             /* Read CEA flag bits here - lockout when read once??? */
             pDis->EdidCeaFlags = pDis->EdidBlock[3];
             
             blockPtr = EDID_BLK_EXT_BASE;   /* data block start always fixed */
             endPtr = pDis->EdidBlock[2];    /* byte after end of data blocks */
             if (endPtr >= (EDID_BLK_EXT_BASE + 2))
                /* Only try reading if data blocks take up 2 bytes or more, since 
                 * a video data block must be at least 2 bytes
                 */
             {
                while (blockPtr < endPtr)
                {
                   blockType   = (UInt8)((pDis->EdidBlock[blockPtr] & 0xE0) >> 5);
                   blockLength = (pDis->EdidBlock[blockPtr] & 0x1F);
                   
                   switch((Int)blockType)
                   {
                      case E_CEA_VIDEO_BLOCK:  /* We have a video data block */
                         for (i = 1; i <= blockLength; i++)
                         {
                            /* If space, store non-zero SVDs */
                            if ((pDis->EdidBlock[blockPtr + i] != 0) &&
                                (pDis->EdidSvdCnt < HDMI_TX_SVD_MAX_CNT))
                            {
                               pDis->EdidVFmts[pDis->EdidSvdCnt] =
                                  pDis->EdidBlock[blockPtr + i];
                               pDis->EdidSvdCnt++;
                            }
                         }
                         break;
                      case E_CEA_AUDIO_BLOCK:  /* We have an audio data block */
                         for (i = 1; (i + 2) <= blockLength; i += 3) 
                         {   /* Must loop in steps of 3 (SAD size) */
                            /* If space, store non-zero SADs */
                            if (((pDis->EdidBlock[blockPtr + i] & 0x78) != 0) &&
                                (pDis->EdidSadCnt < HDMI_TX_SAD_MAX_CNT))
                            {
                               pDis->EdidAFmts[pDis->EdidSadCnt].ModeChans =
                                  pDis->EdidBlock[blockPtr + i];
                               pDis->EdidAFmts[pDis->EdidSadCnt].Freqs =
                                  pDis->EdidBlock[blockPtr + i + 1];
                               pDis->EdidAFmts[pDis->EdidSadCnt].Byte3 =
                                  pDis->EdidBlock[blockPtr + i + 2];
                               pDis->EdidSadCnt++;
                            }
                         }
                         break;
                      case E_CEA_VSDB:         /* We have a VSDB */
                         /* 5 bytes expected, but this is EDID land so double check*/
                         if (blockLength >= 5)
                         {
                            if ((pDis->EdidBlock[blockPtr + 1] == 0x03) &&
                                (pDis->EdidBlock[blockPtr + 2] == 0x0C) &&
                                (pDis->EdidBlock[blockPtr + 3] == 0x00))
                            {
                               pDis->EdidSinkType = HDMITX_SINK_HDMI;
                               pDis->EdidSourceAddress = 
                                  ((UInt16)pDis->EdidBlock[blockPtr + 4] << 8) +
                                  pDis->EdidBlock[blockPtr + 5];
                            }
                            else
                            {
                               pDis->EdidSinkType = HDMITX_SINK_DVI;
                            }
                         }
                         if (blockLength >= 6)   /* Space for byte with AI flag */
                         {   /* Mask AI bit */
                            if((pDis->EdidBlock[blockPtr + 6] & 0x80) == 0x80)
                            {
                               pDis->EdidSinkAi = True;
                            }
                         }

                         /* Read Max_TMDS_Clock */
                         if (blockLength >= 7)  
                             pDis->EdidExtraVsdbData.maxTmdsClock = pDis->EdidBlock[blockPtr + 7];
                         else
                             pDis->EdidExtraVsdbData.maxTmdsClock = 0;

                         /* latency, HDMI Video present and content type fields */
                         if (blockLength >= 8) {

                             /* Read CNC0~3 */
                             pDis->EdidExtraVsdbData.cnc0 = pDis->EdidBlock[blockPtr + 8] & 0x01;  /* 1=True, 0=False */
                             pDis->EdidExtraVsdbData.cnc1 = (pDis->EdidBlock[blockPtr + 8] & 0x02) >> 1;
                             pDis->EdidExtraVsdbData.cnc2 = (pDis->EdidBlock[blockPtr + 8] & 0x04) >> 2;
                             pDis->EdidExtraVsdbData.cnc3 = (pDis->EdidBlock[blockPtr + 8] & 0x08) >> 3;
                                                          
                             if( (pDis->EdidBlock[blockPtr + 8] & 0xC0) == 0xC0 ) {
                                 /* Read video_latency, audio_latency, I_video_latency, I_audio_latency */

                                 pDis->EdidLatency.Edidvideo_latency = pDis->EdidBlock[blockPtr + 9];
                                 pDis->EdidLatency.Edidaudio_latency = pDis->EdidBlock[blockPtr + 10];
                                 pDis->EdidLatency.EdidIvideo_latency = pDis->EdidBlock[blockPtr + 11];
                                 pDis->EdidLatency.EdidIaudio_latency = pDis->EdidBlock[blockPtr + 12];

                                 pDis->EdidLatency.latency_available = True;
                                 pDis->EdidLatency.Ilatency_available = True;

                                 offset3D = 13;  /* offset to the '3D_present' field */
                             }
                             else if ((pDis->EdidBlock[blockPtr + 8] & 0x80) == 0x80) {
                                 /* Read video_latency, audio_latency */

                                 pDis->EdidLatency.Edidvideo_latency = pDis->EdidBlock[blockPtr + 9];
                                 pDis->EdidLatency.Edidaudio_latency = pDis->EdidBlock[blockPtr + 10];

                                 pDis->EdidLatency.latency_available = True;

                                 offset3D = 11;
                             }
                             else {
                                pDis->EdidLatency.latency_available = False;
                                pDis->EdidLatency.Ilatency_available = False;
                                offset3D = 9;
                             }

                             /* Read HDMI_Video_present */
                             pDis->EdidExtraVsdbData.hdmiVideoPresent = (pDis->EdidBlock[blockPtr + 8] & 0x20) >> 5;

                         }
                         else {
                             pDis->EdidLatency.latency_available      = False;
                             pDis->EdidLatency.Ilatency_available     = False;
                             pDis->EdidExtraVsdbData.hdmiVideoPresent = False;
                             pDis->EdidExtraVsdbData.cnc0 = False;
                             pDis->EdidExtraVsdbData.cnc1 = False;
                             pDis->EdidExtraVsdbData.cnc2 = False;
                             pDis->EdidExtraVsdbData.cnc3 = False;
                         }


                         /* 3D data fields according to HDMI 1.4a standard */
                         if (pDis->EdidExtraVsdbData.hdmiVideoPresent) {

                             /* read 3D_present */
                             pDis->EdidExtraVsdbData.h3DPresent = (pDis->EdidBlock[blockPtr + offset3D] & 0x80) >> 7;
                             /* read 3D_Multi_present */
                             pDis->EdidExtraVsdbData.h3DMultiPresent = (pDis->EdidBlock[blockPtr + offset3D] & 0x60) >> 5;
                             /* read image_Size */
                             pDis->EdidExtraVsdbData.imageSize = (pDis->EdidBlock[blockPtr + offset3D] & 0x18) >> 3;
                             
                             /* read HDMI_3D_LEN and HDMI_XX_LEN */
                             offset3D += 1;
                             pDis->EdidExtraVsdbData.hdmi3DLen  = pDis->EdidBlock[blockPtr + offset3D] & 0x1F;
                             pDis->EdidExtraVsdbData.hdmiVicLen = (pDis->EdidBlock[blockPtr + offset3D] & 0xE0) >> 5;

                             if((pDis->EdidExtraVsdbData.hdmi3DLen + pDis->EdidExtraVsdbData.hdmiVicLen) > 0)
                             {
                                 /* copy the rest of the bytes*/
                                 lmemcpy(pDis->EdidExtraVsdbData.ext3DData, &(pDis->EdidBlock[blockPtr + offset3D + 1]), blockLength-offset3D);
                             }
                         }
                         else {
                             pDis->EdidExtraVsdbData.h3DPresent       = False;
                             pDis->EdidExtraVsdbData.h3DMultiPresent  = 0;
                             pDis->EdidExtraVsdbData.imageSize        = 0;
                             pDis->EdidExtraVsdbData.hdmi3DLen        = 0;
                             pDis->EdidExtraVsdbData.hdmiVicLen       = 0;
                         }
                         
                         break;

                      case E_CEA_EXTENDED: /* Use extended Tag */

                        /* we need to read the extended tag code */


                          switch ( pDis->EdidBlock[blockPtr + 1]) 
                          {
                            case EXT_CEA_COLORIMETRY_DB:

                                /* look at xvYCC709 and xvYCC601 support */
                                pDis->EdidCeaXVYCCFlags = pDis->EdidBlock[blockPtr + 2];

                            break;
                          }

                        break;  /* E_CEA_EXTENDED */

                      default:
                         break;
                   }
                   blockPtr += (blockLength + 1);  /* Point to next block */
                }
             }
             dtdFound = True;

             for (i = 0; (i < EDID_NUMBER_MAX_DTD_BLK_1) && (dtdFound); i++) /* search possible DTD blocks in block 1 */
             {   
                blockPtr = ((UInt8)pDis->EdidBlock[EDID_BLK1_OFFSET_BASE_DTD]) + ((UInt8)(i * EDID_DTD_BLK_SIZE));
                if ((blockPtr + EDID_DTD_BLK_SIZE - 1) < EDID_BLOCK_SIZE)
                {
                   dtdFound = storeDtdBlock(pDis, blockPtr);
                }
             }

             break;
            

          case EDID_BLOCK_MAP:
             /* BLOCK MAP */

              if (pDis->EdidBlockCnt > 1) {
                  if (pDis->EdidBlock[pDis->EdidBlockCnt - 1] == EDID_CEA_EXTENSION) {
                   /* Some devices have been incorrectly designed so that the block map is not counted in the */
                   /* extension count. Design of compliant devices should take compatibility with those non-compliant */
                   /* devices into consideration. */
                      pDis->EdidBlockCnt = pDis->EdidBlockCnt + 1;
                  }
              }


             break;
             

       }
       
    }
    
    return TM_OK;
}

/*============================================================================*/
/* storeDtdBlock                                                              */
/*============================================================================*/
static Bool
storeDtdBlock
(   
    tmHdmiTxobject_t    *pDis,      /* Device instance strucure holding block */
    UInt8               blockPtr                     
)
{

    Bool    dtdFound = False;

    /* First, select blocks that are DTDs [CEA861C A.2.10] */
    if (((pDis->EdidBlock[blockPtr+0] != 0) ||
         (pDis->EdidBlock[blockPtr+1] != 0) ||
         (pDis->EdidBlock[blockPtr+2] != 0) ||
         (pDis->EdidBlock[blockPtr+4] != 0))
        &&
         (pDis->NbDTDStored < NUMBER_DTD_STORED))
    {   /* Store the first DTD we find, others will be skipped */
        pDis->EdidDTD[pDis->NbDTDStored].uPixelClock =
            ((UInt16)pDis->EdidBlock[blockPtr+1] << 8) |
                (UInt16)pDis->EdidBlock[blockPtr+0];

        pDis->EdidDTD[pDis->NbDTDStored].uHActivePixels =
            (((UInt16)pDis->EdidBlock[blockPtr+4] & 0x00F0) << 4) |
                (UInt16)pDis->EdidBlock[blockPtr+2];

        pDis->EdidDTD[pDis->NbDTDStored].uHBlankPixels = 
            (((UInt16)pDis->EdidBlock[blockPtr+4] & 0x000F) << 8) |
                (UInt16)pDis->EdidBlock[blockPtr+3];

        pDis->EdidDTD[pDis->NbDTDStored].uVActiveLines =
            (((UInt16)pDis->EdidBlock[blockPtr+7] & 0x00F0) << 4) |
                (UInt16)pDis->EdidBlock[blockPtr+5];

        pDis->EdidDTD[pDis->NbDTDStored].uVBlankLines =
            (((UInt16)pDis->EdidBlock[blockPtr+7] & 0x000F) << 8) |
                (UInt16)pDis->EdidBlock[blockPtr+6];

        pDis->EdidDTD[pDis->NbDTDStored].uHSyncOffset =
            (((UInt16)pDis->EdidBlock[blockPtr+11] & 0x00C0) << 2) |
                (UInt16)pDis->EdidBlock[blockPtr+8];

        pDis->EdidDTD[pDis->NbDTDStored].uHSyncWidth =
            (((UInt16)pDis->EdidBlock[blockPtr+11] & 0x0030) << 4) |
                (UInt16)pDis->EdidBlock[blockPtr+9];

        pDis->EdidDTD[pDis->NbDTDStored].uVSyncOffset =
            (((UInt16)pDis->EdidBlock[blockPtr+11] & 0x000C) << 2) |
            (((UInt16)pDis->EdidBlock[blockPtr+10] & 0x00F0) >> 4);

        pDis->EdidDTD[pDis->NbDTDStored].uVSyncWidth =
            (((UInt16)pDis->EdidBlock[blockPtr+11] & 0x0003) << 4) |
                ((UInt16)pDis->EdidBlock[blockPtr+10] & 0x000F);

        pDis->EdidDTD[pDis->NbDTDStored].uHImageSize =
            (((UInt16)pDis->EdidBlock[blockPtr+14] & 0x00F0) << 4) |
                (UInt16)pDis->EdidBlock[blockPtr+12];

        pDis->EdidDTD[pDis->NbDTDStored].uVImageSize = 
            (((UInt16)pDis->EdidBlock[blockPtr+14] & 0x000F) << 8) |
                (UInt16)pDis->EdidBlock[blockPtr+13];

        pDis->EdidDTD[pDis->NbDTDStored].uHBorderPixels =
            (UInt16)pDis->EdidBlock[blockPtr+15];

        pDis->EdidDTD[pDis->NbDTDStored].uVBorderPixels =
            (UInt16)pDis->EdidBlock[blockPtr+16];

        pDis->EdidDTD[pDis->NbDTDStored].Flags = pDis->EdidBlock[blockPtr+17];

        pDis->NbDTDStored++;

        dtdFound = True;    /* Stop any more DTDs being parsed */
    }

    return (dtdFound);
}


/*============================================================================*/
/* storeMonitorBlock                                                          */
/*============================================================================*/
static Bool
storeMonitorDescriptor
(   
    tmHdmiTxobject_t    *pDis,      /* Device instance strucure holding block */
    UInt8               blockPtr                     
)
{

    Bool    dtdFound = False;

    /* First, select blocks that are DTDs [CEA861C A.2.10] */
    if ((pDis->EdidBlock[blockPtr+0] == 0) &&
        (pDis->EdidBlock[blockPtr+1] == 0) &&
        (pDis->EdidBlock[blockPtr+2] == 0)
       )
    {
        if (pDis->EdidBlock[blockPtr+3] == EDID_MONITOR_NAME_DESC_DATA_TYPE)
        {
            if (pDis->EdidFirstMonitorDescriptor.bDescRecord == False)
            {
                pDis->EdidFirstMonitorDescriptor.bDescRecord = True;
                lmemcpy(&(pDis->EdidFirstMonitorDescriptor.uMonitorName) , 
                        &(pDis->EdidBlock[blockPtr+5]), EDID_MONITOR_DESCRIPTOR_SIZE);
                dtdFound = True;
            }
            else if ((pDis->EdidOtherMonitorDescriptor.bDescRecord == False))
            {
                pDis->EdidOtherMonitorDescriptor.bDescRecord = True;
                lmemcpy(&(pDis->EdidOtherMonitorDescriptor.uOtherDescriptor) , 
                        &(pDis->EdidBlock[blockPtr+5]), EDID_MONITOR_DESCRIPTOR_SIZE);
                dtdFound = True;
            }
        }
        else if (pDis->EdidBlock[blockPtr+3] == EDID_MONITOR_RANGE_DESC_DATA_TYPE)
        {
            if (pDis->EdidSecondMonitorDescriptor.bDescRecord == False)
            {
                pDis->EdidSecondMonitorDescriptor.bDescRecord = True;
                pDis->EdidSecondMonitorDescriptor.uMinVerticalRate = pDis->EdidBlock[blockPtr+5];
                pDis->EdidSecondMonitorDescriptor.uMaxVerticalRate = pDis->EdidBlock[blockPtr+6];
                pDis->EdidSecondMonitorDescriptor.uMinHorizontalRate = pDis->EdidBlock[blockPtr+7];
                pDis->EdidSecondMonitorDescriptor.uMaxHorizontalRate = pDis->EdidBlock[blockPtr+8];
                pDis->EdidSecondMonitorDescriptor.uMaxSupportedPixelClk = pDis->EdidBlock[blockPtr+9];
                dtdFound = True;
            }
        }
    }

    return (dtdFound);

}


/*============================================================================*/
/* tmbslTDA9984EdidGetLatencyInfo                                             */
/*============================================================================*/
tmErrorCode_t
tmbslTDA9984EdidGetLatencyInfo
(
    tmUnitSelect_t  txUnit,
    tmbslHdmiTxEdidLatency_t * pEdidLatency
)
{
    tmHdmiTxobject_t    *pDis;      /* Pointer to Device Instance Structure */
    tmErrorCode_t       err;        /* Error code */

    /* Check unit parameter and point to TX unit object */
    err = checkUnitSetDis(txUnit, &pDis);
    RETIF(err != TM_OK, err)

    /* Check remaining parameter(s) */
    RETIF_BADPARAM(pEdidLatency == Null)

    if ((pDis->EdidStatus == HDMITX_EDID_READ) ||
        (pDis->EdidStatus == HDMITX_EDID_ERROR_CHK))
    {
        /* allow if edid are read or if there are a chk error on an other block than block 0 */

        *pEdidLatency = pDis->EdidLatency;
    }
    else
    {
        /* Not allowed if EdidStatus value is not valid */
        err = TMBSL_ERR_HDMI_RESOURCE_NOT_AVAILABLE; 
    }
    return err;

}

/*============================================================================*/
/* tmbslTDA9984EdidGetExtraVsdbData                                             */
/*============================================================================*/
tmErrorCode_t tmbslTDA9984EdidGetExtraVsdbData
(
    tmUnitSelect_t                  txUnit,
    tmbslHdmiTxEdidExtraVsdbData_t  **pExtraVsdbData
)
{
    tmHdmiTxobject_t    *pDis;      /* Pointer to Device Instance Structure */
    tmErrorCode_t       err;        /* Error code */

    /* Check unit parameter and point to TX unit object */
    err = checkUnitSetDis(txUnit, &pDis);
    RETIF(err != TM_OK, err)

    /* Check remaining parameter(s) */
    RETIF_BADPARAM(pExtraVsdbData == Null)

    if ((pDis->EdidStatus == HDMITX_EDID_READ) ||
        (pDis->EdidStatus == HDMITX_EDID_ERROR_CHK))
    {
        /* allow if edid are read or if there are a chk error on an other block than block 0 */
        *pExtraVsdbData = &(pDis->EdidExtraVsdbData);
    }
    else
    {
        /* Not allowed if EdidStatus value is not valid */
        err = TMBSL_ERR_HDMI_RESOURCE_NOT_AVAILABLE; 
    }
    return err;
}

/*============================================================================*/
/*                            END OF FILE                                     */
/*============================================================================*/
