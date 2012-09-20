/**
 * Copyright (C) 2006 Koninklijke Philips Electronics N.V., All Rights Reserved.
 * This source code and any compilation or derivative thereof is the proprietary
 * information of Koninklijke Philips Electronics N.V. and is confidential in
 * nature. Under no circumstances is this software to be  exposed to or placed
 * under an Open Source License of any type without the expressed written
 * permission of Koninklijke Philips Electronics N.V.
 *
 * \file          tmbslHdmiTx_2.c
 *
 * \version       $Revision: 29 $
 *
 * \date          $Date: 29/10/07 14:11 $
 *
 * \brief         BSL driver component for the TDA9983 HDMI Transmitter
 *
 * \section refs  Reference Documents
 * HDMI Driver - Outline Architecture.doc
 * HDMI Driver - tmbslHdmiTx - SCS.doc
 *
 * \section info  Change Information
 *
 * \verbatim
   $History: tmbslHdmiTx_2.c $
 *
 * *******************  Version 29  *****************
 * User: B.Vereecke     Date: 29/10/07   Time: 14:11
 * Updated in $/Source/tmbslHdmiTx/src
 * PR852 : remove external library dependancy
 *
 * * *****************  Version 28  *****************
 * User: B.Vereecke     Date: 17/10/07   Time: 14:11
 * Updated in $/Source/tmbslHdmiTx/src
 * PR872 : add new formats, 1080p24/25/30
 *
 * * *****************  Version 27  *****************
 * User: J. Lamotte      Date: 01/06/07   Time: 12:00
 * Updated in $/Source/tmbslHdmiTx/src
 * PR359 (PR176) : Modify SetHwRegisters call in
 *				tmbslHdmiTxMatrixSetConversion API 
 *
 * * *****************  Version 26  *****************
 * User: J. Lamotte      Date: 14/05/07   Time: 10:30
 * Updated in $/Source/tmbslHdmiTx/src
 * PR322 (PR176) : Remove 3 DLL compilation warnings
 *                 (calculateCheckusm, getEdidBlock, 
 *                 tmbslHdmiTxpktSetVsInfoframe)
 *
 * * *****************  Version 25  *****************
 * User: Burnouf         Date: 18/04/07   Time: 13:25
 * Updated in $/Source/tmbslHdmiTx/src
 * PR50 : Send event EV_SINKON for TDA9981
 *
 * *****************  Version 24  *****************
 * User: Burnouf      Date: 29/11/06   Time: 17:06
 * Updated in $/Source/tmbslHdmiTx/Src
 * PNF79 and PR11 allow driver to go in state connected if EDID checksum
 * is bad 
 * 
 * *****************  Version 23  *****************
 * User: Mayhew       Date: 23/11/06   Time: 15:06
 * Updated in $/Source/tmbslHdmiTx/Src
 * PNF78 Ensure that DDC is disconnected from I2C after bad EDID read
 * 
 * *****************  Version 21  *****************
 * User: Mayhew       Date: 10/11/06   Time: 10:11
 * Updated in $/Source/tmbslHdmiTx/Src
 * PNF68 RETIF_REG_FAIL macro replaces RETIF checks after register set/get
 * calls, and compiled out to save code space
 * 
 * *****************  Version 19  *****************
 * User: Mayhew       Date: 2/11/06    Time: 16:54
 * Updated in $/Source/tmbslHdmiTx/Src
 * Remove N3 support
 * 
 * *****************  Version 17  *****************
 * User: Mayhew       Date: 21/09/06   Time: 15:49
 * Updated in $/Source/tmbslHdmiTx/Src
 * Cut code size in demo by using RETIF_BADPARAM
 * 
 * *****************  Version 15  *****************
 * User: Djw          Date: 22/08/06   Time: 9:56
 * Updated in $/Source/tmbslHdmiTx/Src
 * Updated file configuration info.
 * 
 * *****************  Version 14  *****************
 * User: Mayhew       Date: 10/07/06   Time: 13:09
 * Updated in $/Source/tmbslHdmiTx/Src
 * Fix file header comment
 * 
 * *****************  Version 12  *****************
 * User: Mayhew       Date: 30/06/06   Time: 13:26
 * Updated in $/Source/tmbslHdmiTx/Src
 * EdidSinkType replaced by SinkType. Set full colourspace for VGA format
 * 1 as well as for PC formats.
 * 
 * *****************  Version 10  *****************
 * User: Djw          Date: 16/06/06   Time: 12:04
 * Updated in $/Source/tmbslHdmiTx/Src
 * Added use of alternate i2c address for EDID.  More conditional
 * compilation for demoboard build.
 * 
 * *****************  Version 8  *****************
 * User: Mayhew       Date: 5/06/06    Time: 16:39
 * Updated in $/Source/tmbslHdmiTx/Src
 * Save code space by replacing API unit checks with checkUnitSetDis.
 * 
 * *****************  Version 7  *****************
 * User: Djw          Date: 24/05/06   Time: 15:30
 * Updated in $/Source/tmbslHdmiTx/Src
 * Minor change to force EDID reads of block 0 or 1 to occur without
 * segment pointer - thus keeping compatibility with older EDIDs.
 * 
 * *****************  Version 5  *****************
 * User: Djw          Date: 24/05/06   Time: 11:19
 * Updated in $/Source/tmbslHdmiTx/Src
 * Added conditional compilation for demoboard build.  Added N4 EDID
 * capability with Seg Ptr writing.  Added all InfoFrames with N4 version
 * checking; InfoFrames use page 10h on N4.
 * 
 * *****************  Version 4  *****************
 * User: Mayhew       Date: 10/05/06   Time: 17:08
 * Updated in $/Source/tmbslHdmiTx/Src
 * Rename local E_ enums to public HDMITX_ enums
 * 
 * *****************  Version 3  *****************
 * User: Djw          Date: 20/04/06   Time: 18:03
 * Updated in $/Source/tmbslHdmiTx/Src
 * Fixed logic problem with EDID block parsing on block 1 upwards
 * (checksum test).  Added workaround for EDID ghost register problem on
 * n3 device.
 * 
 * *****************  Version 2  *****************
 * User: Mayhew       Date: 11/04/06   Time: 14:09
 * Updated in $/Source/tmbslHdmiTx/Src
 * Fixed Bad Parameter error in MatrixSetConversion and fixed wrong block
 * version in video infoframe.
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

/* Defining this symbol on the compiler command line excludes unused code */
/* #define DEMO_BUILD */

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
/*                   TYPE DEFINITIONS                                         */
/*============================================================================*/



/*============================================================================*/
/*                   PUBLIC VARIABLE DEFINITIONS                              */
/*============================================================================*/


/*============================================================================*/
/*                   STATIC VARIABLE DECLARATIONS                             */
/*============================================================================*/

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
 *  The co-ordinates into this array are tmbslHdmiTxColourspace_t enums.
 *  The value of -1 is returned for matching input/output colourspaces.
 */
static CONST_DAT Int kMatrixIndex[HDMITX_CS_NUM][HDMITX_CS_NUM] =
{
    {-1, E_MATRIX_RGBF_2_RGBL, E_MATRIX_RGBF_2_BT601, E_MATRIX_RGBF_2_BT709},
    {E_MATRIX_RGBL_2_RGBF, -1, E_MATRIX_RGBL_2_BT601, E_MATRIX_RGBL_2_BT709},
    {E_MATRIX_BT601_2_RGBF, E_MATRIX_BT601_2_RGBL, -1, E_MATRIX_BT601_2_BT709},
    {E_MATRIX_BT709_2_RGBF, E_MATRIX_BT709_2_RGBL, E_MATRIX_BT709_2_BT601, -1}
};

/*============================================================================*/
/*                   STATIC FUNCTION DECLARATIONS                             */
/*============================================================================*/
static UInt8            calculateChecksum (UInt8 *pData, Int numBytes);
static tmErrorCode_t    getEdidBlock (tmHdmiTxobject_t *pDis, 
                                      Int blockNumber);
static tmErrorCode_t    parseEdidBlock (tmHdmiTxobject_t *pDis, 
                                        Int blockNumber);

static Bool             storeDtdBlock (tmHdmiTxobject_t    *pDis,
                                        UInt8               blockPtr);

static Bool             storeMonitorDescriptor (tmHdmiTxobject_t    *pDis,
                                                UInt8                blockPtr);



/*============================================================================*/
/*                   PUBLIC FUNCTION DEFINITIONS                              */
/*============================================================================*/

/*============================================================================*/
/* tmbslHdmiTxEdidGetAudioCapabilities                                        */
/*============================================================================*/
#ifndef DEMO_BUILD
tmErrorCode_t
tmbslHdmiTxEdidGetAudioCapabilities
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
    UInt8               edidResult;

    /* Check unit parameter and point to TX unit object */
    err = checkUnitSetDis(txUnit, &pDis);
    RETIF(err != TM_OK, err)

    /* Check remaining parameter(s) */
    RETIF_BADPARAM(pEdidAFmts == Null)
    RETIF_BADPARAM(aFmtLength < 1)
    RETIF_BADPARAM(pAFmtsAvail == Null)
    RETIF_BADPARAM(pAudioFlags == Null)

    /* IF the EdidStatus value is not valid in the Device Instance Structure
     * THEN call tmbslHdmiTxEdidGetBlockData.
     */
    if (pDis->EdidStatus == HDMITX_EDID_NOT_READ)
    {
        err = tmbslHdmiTxEdidGetBlockData(txUnit, Null, 0, 0, &edidResult);
        /* IF the result is not TM_OK THEN return it. */
        RETIF(err != TM_OK, err)
    }

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

    return TM_OK;
}
#endif /* DEMO_BUILD */

/*============================================================================*/
/* tmbslHdmiTxEdidGetBlockCount                                               */
/*============================================================================*/
tmErrorCode_t
tmbslHdmiTxEdidGetBlockCount
(
    tmUnitSelect_t      txUnit,
    UInt8               *puEdidBlockCount
)
{
    tmHdmiTxobject_t    *pDis;      /* Pointer to Device Instance Structure */
    tmErrorCode_t       err;        /* Error code */
    UInt8               edidResult;

    /* Check unit parameter and point to TX unit object */
    err = checkUnitSetDis(txUnit, &pDis);
    RETIF(err != TM_OK, err)

    /* Check remaining parameter(s) */
    RETIF_BADPARAM(puEdidBlockCount == Null)

    /* IF the EdidStatus value is not valid in the Device Instance Structure
     * THEN call tmbslHdmiTxEdidGetBlockData.
     */
    if (pDis->EdidStatus == HDMITX_EDID_NOT_READ)
    {
        err = tmbslHdmiTxEdidGetBlockData(txUnit, Null, 0, 0, &edidResult);
        /* IF the result is not TM_OK THEN return it. */
        RETIF(err != TM_OK, err)
    }

    *puEdidBlockCount = pDis->EdidBlockCnt;

    return TM_OK;
}

/*============================================================================*/
/* tmbslHdmiTxEdidGetBlockData                                                */
/*============================================================================*/
tmErrorCode_t
tmbslHdmiTxEdidGetBlockData
(
    tmUnitSelect_t  txUnit,
    UInt8           *pRawEdid, 
    Int             numBlocks,  /* Only relevant if pRawEdid valid */
    Int             lenRawEdid, /* Only relevant if pRawEdid valid */
    UInt8           *pEdidStatus
)
{
    tmHdmiTxobject_t    *pDis;      /* Pointer to Device Instance Structure */
    tmErrorCode_t       err;        /* Error code */
    Int                 i;          /* Loop index */
    UInt8               extBlockCnt;   /* Count of extension blocks */

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
    RETIF_BADPARAM(pEdidStatus == Null)

    /* Reset the EdidStatus in the Device Instance Structure */
    pDis->EdidStatus = HDMITX_EDID_NOT_READ;

    /* Reset stored parameters from EDID in the Device Instance Structure */
    pDis->EdidSinkType = HDMITX_SINK_DVI;
    pDis->EdidSinkAi = False;
    pDis->EdidCeaFlags = 0;
    pDis->EdidSvdCnt = 0;
    pDis->EdidSadCnt = 0;
    pDis->EdidSourceAddress = 0;                /* 0.0.0.0 */
    pDis->NbDTDStored = 0;
    pDis->EdidFirstMonitorDescriptor.bDescRecord = False;
    pDis->EdidSecondMonitorDescriptor.bDescRecord = False;
    pDis->EdidOtherMonitorDescriptor.bDescRecord = False;

    


    /* Read the HPD pin via the hpd_in flag in the first interrupt status
     * register and return a TMBSL_ERR_HDMI_NULL_CONNECTION error if it is
     * not set.
     * We must use the flag in the Device Instance Structure to avoid 
     * clearing pending interrupt flags.
     */
    RETIF(pDis->hotPlugStatus != HDMITX_HOTPLUG_ACTIVE,
          TMBSL_ERR_HDMI_NULL_CONNECTION)
                           
    /* Get the first EDID block into Device Instance workspace */
    err = getEdidBlock(pDis, 0);
    /* PR11 : On i2c error or bad checksum in block 0 */
    /* allow driver to go in state CONNECTED */
    
    if (err != TM_OK)
    {
        setState(pDis, EV_GETBLOCKDATA);
#ifdef TMFL_TDA9981_SUPPORT
#ifdef TMFL_RX_SENSE_ON
        if (pDis->rxSenseStatus == HDMITX_RX_SENSE_ACTIVE)
        {
            setState(pDis, EV_SINKON);
        }
#endif /* TMFL_RX_SENSE_ON */
#endif /* TMFL_TDA9981_SUPPORT */
       return err;
    }
    
    pDis->EdidStatus = HDMITX_EDID_READ;  /* Indicate that status OK so far */

    /* If pointer present, copy block from workspace.  We know from the 
     * paramenter checking on entry that at least one block is required
     * and we have the space for it.
     */
    if (pRawEdid != Null)
    {
        lmemcpy(pRawEdid, pDis->EdidBlock, EDID_BLOCK_SIZE);
    }

    /* Could check block 0 header (0x00,6 x 0xFF,0x00) here but not
     * certain to be future proof [CEA861C A.2.3]
     */

    /* Read block count from penultimate byte of block and store in DIS */
    extBlockCnt = pDis->EdidBlock[EDID_BLK0_EXT_CNT];
    /* (For N3, used to have to limit extBlockCnt to one block) */
    pDis->EdidBlockCnt = extBlockCnt + 1;   /* Total = Block 0 + extensions */

    /* Parse block 0 */
    parseEdidBlock(pDis, 0);

    /* If extension blocks are present, process them */
    if (extBlockCnt > 0)
    {
        for (i = 0; i < extBlockCnt; i++)
        {
            /* read block */
            err = getEdidBlock(pDis, i+1);
            /* On this occasion, we also accept INVALID_STATE which means
             * there was a checksum error
             */
             if ((err != TM_OK) && (err != TMBSL_ERR_HDMI_INVALID_STATE))
             {
                 /* PR11 : allow driver to go in state CONNECTED */
                 setState(pDis, EV_GETBLOCKDATA);
#ifdef TMFL_TDA9981_SUPPORT
#ifdef TMFL_RX_SENSE_ON
                 if (pDis->rxSenseStatus == HDMITX_RX_SENSE_ACTIVE)
                 {
                     setState(pDis, EV_SINKON);
                 }
#endif /* TMFL_RX_SENSE_ON */
#endif /* TMFL_TDA9981_SUPPORT */
                 return err;
             }
                        
            /* If pointer was supplied, copy block from DIS to buffer */
            if (pRawEdid != Null)
            {
                /* Check if we've copied as many as requested yet? */
                if ((i + 2) <= numBlocks) 
                {
                    lmemcpy(pRawEdid + ((i + 1) * EDID_BLOCK_SIZE),
                           pDis->EdidBlock,
                           EDID_BLOCK_SIZE);
                    if (err == TMBSL_ERR_HDMI_INVALID_STATE)
                    {   /* Note checksum error in EdidStatus */
                        pDis->EdidStatus = HDMITX_EDID_ERROR;  
                    }
                }
                else    /* Fewer blocks requested than EDID contains, warn */
                {
                   if (pDis->EdidStatus == HDMITX_EDID_ERROR)
                   {
                      pDis->EdidStatus = HDMITX_EDID_ERROR_INCOMPLETE;
                   }
                }
            }
            /* If the checksum was OK, we can parse the block */
            if (err == TM_OK)
            {
                parseEdidBlock(pDis, i+1);
            }
        }
    }

    /* Copy return value from EdidStatus */
    *pEdidStatus = pDis->EdidStatus;

    /* Filter out buffer status from the EdidStatus value in the
     * Device Instance Structure
     */
    if ((pDis->EdidStatus == HDMITX_EDID_ERROR) ||
        (pDis->EdidStatus == HDMITX_EDID_ERROR_INCOMPLETE))
    {
       pDis->EdidStatus = HDMITX_EDID_ERROR;
    }    

    setState(pDis, EV_GETBLOCKDATA);

#ifdef TMFL_TDA9981_SUPPORT
#ifdef TMFL_RX_SENSE_ON
    if (pDis->rxSenseStatus == HDMITX_RX_SENSE_ACTIVE)
    {
        setState(pDis, EV_SINKON);
    }
#endif /* TMFL_RX_SENSE_ON */
#endif /* TMFL_TDA9981_SUPPORT */

    return TM_OK;
}

/*============================================================================*/
/* tmbslHdmiTxEdidGetSinkType                                                 */
/*============================================================================*/
tmErrorCode_t
tmbslHdmiTxEdidGetSinkType
(
    tmUnitSelect_t              txUnit,
    tmbslHdmiTxSinkType_t      *pSinkType 
)
{
    tmHdmiTxobject_t    *pDis;      /* Pointer to Device Instance Structure */
    tmErrorCode_t       err;        /* Error code */
    UInt8               edidResult;

    /* Check unit parameter and point to TX unit object */
    err = checkUnitSetDis(txUnit, &pDis);
    RETIF(err != TM_OK, err)

    /* Check remaining parameter(s) */
    RETIF_BADPARAM(pSinkType == Null)

    /* IF the EdidStatus value is not valid in the Device Instance Structure
     * THEN call tmbslHdmiTxEdidGetBlockData.
     */
    if (pDis->EdidStatus == HDMITX_EDID_NOT_READ)
    {
        err = tmbslHdmiTxEdidGetBlockData(txUnit, Null, 0, 0, &edidResult);
        /* IF the result is not TM_OK THEN return it. */
        RETIF(err != TM_OK, err)
    }

    *pSinkType = pDis->EdidSinkType;

    return TM_OK;
}

/*============================================================================*/
/* tmbslHdmiTxEdidGetSourceAddress                                            */
/*============================================================================*/
#ifndef DEMO_BUILD
tmErrorCode_t
tmbslHdmiTxEdidGetSourceAddress
(
    tmUnitSelect_t  txUnit,
    UInt16          *pSourceAddress 
)
{
    tmHdmiTxobject_t    *pDis;      /* Pointer to Device Instance Structure */
    tmErrorCode_t       err;        /* Error code */
    UInt8               edidResult;

    /* Check unit parameter and point to TX unit object */
    err = checkUnitSetDis(txUnit, &pDis);
    RETIF(err != TM_OK, err)

    /* Check remaining parameter(s) */
    RETIF_BADPARAM(pSourceAddress == Null)

    /* IF the EdidStatus value is not valid in the Device Instance Structure
     * THEN call tmbslHdmiTxEdidGetBlockData.
     */
    if (pDis->EdidStatus == HDMITX_EDID_NOT_READ)
    {
        err = tmbslHdmiTxEdidGetBlockData(txUnit, Null, 0, 0, &edidResult);
        /* IF the result is not TM_OK THEN return it. */
        RETIF(err != TM_OK, err)
    }

    *pSourceAddress = pDis->EdidSourceAddress;

    return TM_OK;
}
#endif /* DEMO_BUILD */

/*============================================================================*/
/* tmbslHdmiTxEdidGetVideoCapabilities                                        */
/*============================================================================*/
tmErrorCode_t
tmbslHdmiTxEdidGetVideoCapabilities
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
    UInt8               edidResult;

    /* Check unit parameter and point to TX unit object */
    err = checkUnitSetDis(txUnit, &pDis);
    RETIF(err != TM_OK, err)

    /* Check remaining parameter(s) */
    RETIF_BADPARAM(pEdidVFmts == Null)
    RETIF_BADPARAM(vFmtLength < 1)
    RETIF_BADPARAM(pVFmtsAvail == Null)
    RETIF_BADPARAM(pVidFlags == Null)

    /* IF the EdidStatus value is not valid in the Device Instance Structure
     * THEN call tmbslHdmiTxEdidGetBlockData.
     */
    if (pDis->EdidStatus == HDMITX_EDID_NOT_READ)
    {
        err = tmbslHdmiTxEdidGetBlockData(txUnit, Null, 0, 0, &edidResult);
        /* IF the result is not TM_OK THEN return it. */
        RETIF(err != TM_OK, err)
    }

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
    *pVidFlags = ((pDis->EdidCeaFlags & 0x80) |         /* Underscan */
                 ((pDis->EdidCeaFlags & 0x30) << 1) );  /* YUV444, YUV422 */

    /* Fill number of SVDs available parameter */
    *pVFmtsAvail = pDis->EdidSvdCnt;

    return TM_OK;
}

/*============================================================================*/
/* tmbslHdmiTxEdidGetVideoPreferred                                           */
/*============================================================================*/
#ifndef DEMO_BUILD
tmErrorCode_t
tmbslHdmiTxEdidGetVideoPreferred
(
    tmUnitSelect_t		   txUnit,
    tmbslHdmiTxEdidDtd_t *pEdidDTD
)
{
    tmHdmiTxobject_t    *pDis;      /* Pointer to Device Instance Structure */
    tmErrorCode_t       err;        /* Error code */
    UInt8               edidResult;

    /* Check unit parameter and point to TX unit object */
    err = checkUnitSetDis(txUnit, &pDis);
    RETIF(err != TM_OK, err)

    /* Check remaining parameter(s) */
    RETIF_BADPARAM(pEdidDTD == Null)

    /* IF the EdidStatus value is not valid in the Device Instance Structure
     * THEN call tmbslHdmiTxEdidGetBlockData.
     */
    if (pDis->EdidStatus == HDMITX_EDID_NOT_READ)
    {
        err = tmbslHdmiTxEdidGetBlockData(txUnit, Null, 0, 0, &edidResult);
        /* IF the result is not TM_OK THEN return it. */
        RETIF(err != TM_OK, err)
    }

    /* Populate the Detailed Timing Descriptor structure pEdidDTD from
     * EdidDtd in the Device Instance Structure.
     */
    lmemcpy(pEdidDTD, &pDis->EdidDTD, sizeof(*pEdidDTD));

    return TM_OK;
}
#endif /* DEMO_BUILD */

#ifndef DEMO_BUILD
tmErrorCode_t
tmbslHdmiTxEdidGetDetailedTimingDescriptors
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

    if ((pDis->EdidStatus == HDMITX_EDID_READ)  ||
        (pDis->EdidStatus == HDMITX_EDID_ERROR) ||
        (pDis->EdidStatus == HDMITX_EDID_ERROR_INCOMPLETE) )
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
        err = TMBSL_ERR_HDMI_INVALID_STATE; 
    }

    return TM_OK;

}
#endif  /* DEMO_BUILD */



/*============================================================================*/
/* tmbslHdmiTxMatrixSetCoeffs                                                 */
/*============================================================================*/
#ifndef DEMO_BUILD
tmErrorCode_t
tmbslHdmiTxMatrixSetCoeffs
(
    tmUnitSelect_t		   txUnit,
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
#endif /* DEMO_BUILD */

/*============================================================================*/
/* tmbslHdmiTxMatrixSetConversion                                             */
/*============================================================================*/
tmErrorCode_t
tmbslHdmiTxMatrixSetConversion
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
    tmbslHdmiTxColourspace_t    cspace_in;  /* Input colourspaces */
    tmbslHdmiTxColourspace_t    cspace_out; /* Output colourspaces */
    Int                         matrixIndex;/* Index into matrix preset array */
	UInt8						buf[MATRIX_PRESET_SIZE]; /* Temp buffer */
	UInt8						i; /* Loop index */

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
    if((vinFmt == HDMITX_VFMT_TV_MIN) || (vinFmt >= HDMITX_VFMT_PC_MIN))
    {
        /* Catch the VGA or PC formats */
        cspace_in = HDMITX_CS_RGB_FULL;
    }
    else
    {
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
    }

    /* Calculate output colour space */
    if((voutFmt == HDMITX_VFMT_TV_MIN) || (voutFmt >= HDMITX_VFMT_PC_MIN))
    {
        /* Catch the VGA or PC formats */
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

    if (pDis->sinkType == HDMITX_SINK_DVI) {

        switch (dviVqr) {
        case HDMITX_VQR_DEFAULT :
           /* do nothing */
            break;

        case HDMITX_RGB_FULL :
            cspace_out = HDMITX_CS_RGB_FULL;
            break;

        case HDMITX_RGB_LIMITED :
            cspace_out = HDMITX_CS_RGB_LIMITED;
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
/* tmbslHdmiTxMatrixSetInputOffset                                            */
/*============================================================================*/
#ifndef DEMO_BUILD
tmErrorCode_t
tmbslHdmiTxMatrixSetInputOffset
(
    tmUnitSelect_t		    txUnit,
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
#endif /* DEMO_BUILD */

/*============================================================================*/
/* tmbslHdmiTxMatrixSetMode                                                   */
/*============================================================================*/
tmErrorCode_t
tmbslHdmiTxMatrixSetMode
(
    tmUnitSelect_t		 txUnit,
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
/* tmbslHdmiTxMatrixSetOutputOffset                                           */
/*============================================================================*/
#ifndef DEMO_BUILD
tmErrorCode_t
tmbslHdmiTxMatrixSetOutputOffset
(
    tmUnitSelect_t		    txUnit,
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
#endif /* DEMO_BUILD */

/*============================================================================*/
/* tmbslHdmiTxPktSetAclkRecovery                                              */
/*============================================================================*/
#ifndef DEMO_BUILD
tmErrorCode_t
tmbslHdmiTxPktSetAclkRecovery
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
/* tmbslHdmiTxPktSetAcp                                                       */
/*============================================================================*/
tmErrorCode_t
tmbslHdmiTxPktSetAcp
(
    tmUnitSelect_t		txUnit,
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
#endif /* DEMO_BUILD */

/*============================================================================*/
/* tmbslHdmiTxPktSetAudioInfoframe                                            */
/*============================================================================*/
tmErrorCode_t
tmbslHdmiTxPktSetAudioInfoframe
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
/* tmbslHdmiTxPktSetGeneralCntrl                                              */
/*============================================================================*/
#ifndef DEMO_BUILD
tmErrorCode_t
tmbslHdmiTxPktSetGeneralCntrl
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
/* tmbslHdmiTxPktSetIsrc1                                                     */
/*============================================================================*/
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
/* tmbslHdmiTxPktSetIsrc2                                                     */
/*============================================================================*/
tmErrorCode_t
tmbslHdmiTxPktSetIsrc2
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
/* tmbslHdmiTxPktSetMpegInfoframe                                             */
/*============================================================================*/
tmErrorCode_t
tmbslHdmiTxPktSetMpegInfoframe
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
/* tmbslHdmiTxPktSetNullInsert                                                */
/*============================================================================*/
tmErrorCode_t
tmbslHdmiTxPktSetNullInsert
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
/* tmbslHdmiTxPktSetNullSingle                                                */
/*============================================================================*/
tmErrorCode_t
tmbslHdmiTxPktSetNullSingle
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
/* tmbslHdmiTxPktSetSpdInfoframe                                              */
/*============================================================================*/
tmErrorCode_t
tmbslHdmiTxPktSetSpdInfoframe
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
#endif /* DEMO_BUILD */

/*============================================================================*/
/* tmbslHdmiTxPktSetVideoInfoframe                                            */
/*============================================================================*/
tmErrorCode_t
tmbslHdmiTxPktSetVideoInfoframe
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
/* tmbslHdmiTxPktSetVsInfoframe                                               */
/*============================================================================*/
#ifndef DEMO_BUILD
tmErrorCode_t
tmbslHdmiTxPktSetVsInfoframe
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
#endif /* DEMO_BUILD */

/*============================================================================*/
/*                   STATIC FUNCTION DEFINTIONS                               */
/*============================================================================*/

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
/* getEdidBlock - reads an entire edid block                                  */
/*============================================================================*/
static tmErrorCode_t
getEdidBlock
(   
    tmHdmiTxobject_t    *pDis,      /* Device instance strucure to use */
    Int                 blockNumber /* Block number to read */
    )
{
    tmErrorCode_t   err;        /* Error code */
    Int             i;          /* Loop index */
    UInt8           chksum;     /* Checksum value */
    UInt8           segptr;     /* Segment ptr value */
    UInt8           offset;     /* Word offset value */
    tmbslHdmiTxSysArgsEdid_t sysArgs; /* Arguments passed to system function */

    /* Check block number is valid [CEA861C A.2.1] */
    RETIF_BADPARAM(blockNumber >= 255)

    /* Enable the EDID ghost device to allow I2C write-through */
    /* Write the segment pointer address into the ghost regsiter */
    /* (For N3, we did't use the segptr and did't support alternate addr) */
    /* Load both Ghost registers  - we load whole register so no shadowing 
       to give control of lsbs */


    if (pDis->bEdidAlternateAddr == True)
    {   /* Use alternate address and set  a0_zero bit (lsb) in GHOST_XADDR */
        err = setHwRegister(pDis, E_REG_P00_GHOST_XADDR_W,
                            DDC_SGMT_PTR_ADDRESS + 1);
        RETIF_REG_FAIL(err)
        err = setHwRegister(pDis, E_REG_P00_GHOST_ADDR_W,
                            DDC_EDID_ADDRESS_ALT);
        RETIF_REG_FAIL(err)
    }
    else
    {   /* Use normal address and don't set a0_zero bit */
        err = setHwRegister(pDis, E_REG_P00_GHOST_XADDR_W,
                            DDC_SGMT_PTR_ADDRESS);
        RETIF_REG_FAIL(err)
        err = setHwRegister(pDis, E_REG_P00_GHOST_ADDR_W,
                            DDC_EDID_ADDRESS);
        RETIF_REG_FAIL(err)
    }

	


    /* Calculate which segment of the EDID we need (2 blocks per segment) */
    segptr = (UInt8)blockNumber / 2;
    /* For even blocks we need an offset of 0, odd blocks we need 128 */
    offset = (((UInt8)blockNumber & 1) == 1) ? 128 : 0;

    /* If we're reading blocks 0 or 1, we don't use segptr, as sink may not
       support it.  We also never use for n3 or earlier, as it is not
       supported.
     */
    if (blockNumber < 2)
    {   /* Null SegPtrAddress used to indicate seg ptr should be skipped */
        sysArgs.segPtrAddr  = 0;
    }
    else
    {   
        sysArgs.segPtrAddr  = DDC_SGMT_PTR_ADDRESS;
    }

    if (pDis->bEdidAlternateAddr == True)
    {   /* Use alternate address */
        sysArgs.dataRegAddr = DDC_EDID_ADDRESS_ALT;
    }
    else
    {   /* Use default address */
        sysArgs.dataRegAddr = DDC_EDID_ADDRESS;
    }

    /* Read EDID block: THIS CAN FAIL IF DDC CONNECTION IS BROKEN */
    sysArgs.segPtr      = segptr;
    sysArgs.wordOffset  = offset;
    sysArgs.lenData     = EDID_BLOCK_SIZE;
    sysArgs.pData       = pDis->EdidBlock;
    err = pDis->sysFuncEdidRead(&sysArgs);



    /* Do not quit yet on I2C error: must always set GHOST_ADDR here */
    setHwRegister(pDis, E_REG_P00_GHOST_ADDR_W, 0x01);



    /* Ignore last error, but return the more important EDID Read error */
    RETIF(err != TM_OK, TMBSL_ERR_HDMI_I2C_READ)

    /* Add up all the values of the EDID block bytes, including the
     * checksum byte
     */
    chksum=0;
    for (i = 0; i < EDID_BLOCK_SIZE; i++)
    {
        chksum = chksum + pDis->EdidBlock[i];
    }

    /* IF the EDID block does not yield a checksum of zero
     * THEN return a TMBSL_ERR_HDMI_INVALID_STATE error.
     */
    return (chksum == 0) ? TM_OK : TMBSL_ERR_HDMI_INVALID_STATE;
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
    UInt8           NbBlkRead;

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
#ifndef DEMO_BUILD
                dtdFound = storeMonitorDescriptor(pDis, blockPtr);
#endif /* DEMO_BUILD */
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
                         break;
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
             /* Nothing special to do */
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

#ifndef DEMO_BUILD

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
#endif /* DEMO_BUILD */

    return (dtdFound);
}





/*============================================================================*/
/* storeMonitorDescriptor                                                     */
/*============================================================================*/
#ifndef DEMO_BUILD
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
#endif /* DEMO_BUILD */

/*============================================================================*/
/* tmbslDebugWriteFakeRegPage                                                 */
/*============================================================================*/
#ifndef DEMO_BUILD
tmErrorCode_t tmbslDebugWriteFakeRegPage( tmUnitSelect_t txUnit )
{
    tmHdmiTxobject_t *pDis;           /* Pointer to Device Instance Structure */
    tmErrorCode_t     err;            /* Error code */

    err = checkUnitSetDis(txUnit, &pDis);

    pDis->curRegPage = 0x20; 

    return err;
}
#endif  /* DEMO_BUILD */

/*============================================================================*/
/*                     END OF FILE                                            */
/*============================================================================*/
