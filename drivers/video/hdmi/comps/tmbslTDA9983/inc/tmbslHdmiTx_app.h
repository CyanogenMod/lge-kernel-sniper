/**
 * Copyright (C) 2006 Koninklijke Philips Electronics N.V., All Rights Reserved.
 * This source code and any compilation or derivative thereof is the proprietary
 * information of Koninklijke Philips Electronics N.V. and is confidential in
 * nature. Under no circumstances is this software to be  exposed to or placed
 * under an Open Source License of any type without the expressed written
 * permission of Koninklijke Philips Electronics N.V.
 *
 * \file          tmbslHdmiTx_app.h
 *
 * \version       $Revision: 23 $
 *
 * \date          $Date: 10/10/07 11:11 $
 *
 * \brief         Application-level API for BSL driver component for the 
 *                TDA998x HDMI Transmitter
 *
 * \section refs  Reference Documents
 * HDMI Driver - Outline Architecture.doc,
 * HDMI Driver - tmbslHdmiTx - SCS.doc
 *
 * \section info  Change Information
 *
 * \verbatim

   $History: tmbslHdmiTx_app.h $
 *
 ******************  Version 23  ****************
 * User: G. Burnouf     Date: 18/02/08
 * Updated in $/Source/tmbslHdmiTx/Inc
 * PR1355 : Set audio channel allocation
 *
 * ****************  Version 22  ****************
 * User: B.Vereecke     Date: 10/10/07   Time: 11:11
 * Updated in $/Source/tmbslHdmiTx/Inc
 * PR815 : Update bluescreen infoframes
 *         According to the output mode
 *
 * * **************  Version 21  *****************
 * User: B.Vereecke   Date: 30/08/07   Time: 14:45
 * Updated in $/Source/tmbslHdmiTx/Inc
 * PR4   - add new appHdmiTx_ReadEdidAtPowerOn()
 *		   function, calling result after resuming 
 *		   from power_down is an EDID reading.
 *
 * * **************  Version 20  *****************
 * User: B.Vereecke   Date: 20/07/07   Time: 17:30
 * Updated in $/Source/tmbslHdmiTx/Inc
 * PR502 - Add new appHdmiTx_setAudio() function
 *		   for change Audio parameters without update
 *		   video input/output
 *
 *****************  Version 19  ******************
 * User: B.Vereecke   Date: 19/07/07   Time: 10:30
 * Updated in $/Source/tmbslHdmiTx/Inc
 * PR511 - add new PixelEdge argument in 
 *			appHdmiTx_setInputOutput
 *
 * *****************  Version 18  ******************
 * User: B.Vereecke   Date: 17/07/07   Time: 10:30
 * Updated in $/Source/tmbslHdmiTx/Inc
 * PR217 - Add Pattern type parameter to
 *         appHdmiTx_test_pattern_on API in order
 *         to add blue pattern functionality
 *
 * *****************  Version 17  ******************
 * User: J. Lamotte   Date: 29/06/07   Time: 15:50
 * Updated in $/Source/tmbslHdmiTx/Inc
 * PR210 - Add video output format parameter to
 *         appHdmiTx_test_pattern_on API in order
 *         to update AVI info frame for color bar.
 *
 * *****************  Version 16  *****************
 * User: Mayhew       Date: 27/10/06   Time: 12:34
 * Updated in $/Source/tmbslHdmiTx/Inc
 * PNF59 Test pattern API split into _on and _off APIs
 * PNF59 appHdmiTx_handleBCAPS has new pbBksvSecure parameter
 * 
 * *****************  Version 14  *****************
 * User: Mayhew       Date: 13/10/06   Time: 11:02
 * Updated in $/Source/tmbslHdmiTx/Inc
 * PNF37 appHdmiTx_setInputOutput syncIn arg. is now syncSource
 * 
 * *****************  Version 12  *****************
 * User: Mayhew       Date: 15/09/06   Time: 15:56
 * Updated in $/Source/tmbslHdmiTx/Inc
 * PNF19 Add i2sQualifier arg to setInputOutput
 * PNF25 Add pEdidVidFlags arg to handleHPD
 * 
 * *****************  Version 11  *****************
 * User: Mayhew       Date: 10/07/06   Time: 12:33
 * Updated in $/Source/tmbslHdmiTx/Inc
 * Add pbVerified parameter to _setInputOutput. Fix Doxygen comment
 * warnings.
 * 
 * *****************  Version 9  *****************
 * User: Mayhew       Date: 30/06/06   Time: 12:42
 * Updated in $/Source/tmbslHdmiTx/Inc
 * Add audioFmt parameter to appHdmiTx_setInputOutput
 * 
 * *****************  Version 7  *****************
 * User: Mayhew       Date: 5/06/06    Time: 14:38
 * Updated in $/Source/tmbslHdmiTx/Inc
 * Rename syncSrc to syncIn. Remove 2nd param from handleBCAPS. Add
 * pVidFmtNew param to appHdmiTx_nextEdidVidFmt.
 * 
 * *****************  Version 5  *****************
 * User: Mayhew       Date: 22/05/06   Time: 15:55
 * Updated in $/Source/tmbslHdmiTx/Inc
 * Add pixRate to appHdmiTx_setInputOutput. Add appHdmiTx_nextEdidVidFmt.
 * 
 * *****************  Version 3  *****************
 * User: Mayhew       Date: 19/05/06   Time: 11:29
 * Updated in $/Source/tmbslHdmiTx/Inc
 * Add options parameter to appHdmiTx_Hdcp_On
 * 
 * *****************  Version 2  *****************
 * User: Mayhew       Date: 10/05/06   Time: 17:01
 * Updated in $/Source/tmbslHdmiTx/Inc
 * New APIs for HDCP and parameterised format setting
 * 
 * *****************  Version 1  *****************
 * User: Mayhew       Date: 4/04/06    Time: 16:27
 * Created in $/Source/tmbslHdmiTx/Inc
 * Driver demo app API phase 2
 
   \endverbatim
 *
*/

#ifndef TMBSLHDMITX_APP_H
#define TMBSLHDMITX_APP_H

/*============================================================================*/
/*                       INCLUDE FILES                                        */
/*============================================================================*/

#ifdef __cplusplus
extern "C" {
#endif

/*============================================================================*/
/*                       MACRO DEFINITIONS                                    */
/*============================================================================*/


/*============================================================================*/
/*                       ENUM OR TYPE DEFINITIONS                             */
/*============================================================================*/


/*============================================================================*/
/*                       EXTERN DATA DEFINITIONS                              */
/*============================================================================*/


/*============================================================================*/
/*                       EXTERN FUNCTION PROTOTYPES                           */
/*============================================================================*/


/*============================================================================*/
/**
    \brief      Initialise demo application
 */
void
appHdmiTx_init(void);

/*============================================================================*/
/**
    \brief      Set colourbar test pattern on with RGB infoframe

    \param[in]  txUnit      Transmitter unit number
    \param[in]  voutFmt     Video output format
	\param[in]  pattern		type of pattern

    \return     The call result:
                - TM_OK: the call was successful
                - Else a problem has been detected
 */
tmErrorCode_t
appHdmiTx_test_pattern_on
(
    tmUnitSelect_t				txUnit,
    tmbslHdmiTxVidFmt_t			voutFmt,
    tmbslHdmiTxVoutMode_t       voutMode,
	tmbslHdmiTxTestPattern_t	pattern
);

/*============================================================================*/
/**
    \brief      Set colourbar test pattern off with previous infoframe

    \param[in]  txUnit       Transmitter unit number
    \param[in]  voutFmt      Video output format
    \param[in]  voutMode     Video output mode

    \return     The call result:
                - TM_OK: the call was successful
                - Else a problem has been detected
 */
tmErrorCode_t
appHdmiTx_test_pattern_off
(
    tmUnitSelect_t          txUnit,
    tmbslHdmiTxVidFmt_t     voutFmt,
    tmbslHdmiTxVoutMode_t   voutMode
);

/*============================================================================*/
/**
    \brief      Set input and output formats, modes, sync source, sink type
                and audio rate

    \param[in]  txUnit       Transmitter unit number
    \param[in]  vinFmt       Video input format
    \param[in]  vinMode      Video input mode
    \param[in]  voutFmt      Video output format
    \param[in]  voutMode     Video output mode
    \param[in]  syncSource   Video input sync source
    \param[in]  sinkType     Downstream receiver sink type
    \param[in]  audioFmt     Audio format
    \param[in]  audioRate    Audio sample rate
    \param[in]  i2sQualifier Audio I2S qualifier: 8=channels; 16,32=bits
    \param[in]  pixRate      Pixel rate
	\param[in]  pixelEdge    Pixel edge
    \param[out] pbVerified   Pointer to flag set when the requested combination
                             of formats and modes has previously been verified
                             during testing

    \return     The call result:
                - TM_OK: the call was successful
                - Else a problem has been detected
 */
tmErrorCode_t
appHdmiTx_setInputOutput
(
    tmUnitSelect_t          txUnit,
    tmbslHdmiTxVidFmt_t     vinFmt,  
    tmbslHdmiTxVinMode_t    vinMode, 
    tmbslHdmiTxVidFmt_t     voutFmt, 
    tmbslHdmiTxVoutMode_t   voutMode,
    tmbslHdmiTxSyncSource_t syncSource,
    tmbslHdmiTxSinkType_t   sinkType,
    tmbslHdmiTxaFmt_t       audioFmt,  
    tmbslHdmiTxafs_t        audioRate,
    UInt8                   i2sQualifier,
    tmbslHdmiTxPixRate_t    pixRate,
    tmbslHdmiTxPixEdge_t    pixelEdge,
    Bool                    *pbVerified /* Returns True if requested combination
                                         * has been verified */
);

/*============================================================================*/
/**
    \brief      Set audio format and audio rate

    \param[in]  txUnit       Transmitter unit number

    \param[in]  sinkType     Downstream receiver sink type
    \param[in]  audioFmt     Audio format
    \param[in]  audioRate    Audio sample rate
    \param[in]  i2sQualifier Audio I2S qualifier: 8=channels; 16,32=bits

    \return     The call result:
                - TM_OK: the call was successful
                - Else a problem has been detected
 */
tmErrorCode_t
appHdmiTx_setAudio
(
    tmUnitSelect_t          txUnit,
    tmbslHdmiTxVidFmt_t     voutFmt,
    tmbslHdmiTxSinkType_t   sinkType,
    tmbslHdmiTxaFmt_t       audioFmt,  
    tmbslHdmiTxafs_t        audioRate,
    UInt8                   i2sQualifier
);

/*============================================================================*/
/**
    \brief      Set audio channel allocation

    \param[in]  ChannelAllocation     audio channel allocation

    \return     The call result:
                - void
 */
void
appHdmiTx_setAudioChannelAllocation
(
	UInt8					ChannelAllocation
);

/*============================================================================*/
/**
    \brief      Switch on HDCP

    \param[in]  txUnit       Transmitter unit number
    \param[in]  voutFmt      Current CEA output format
    \param[in]  options      HDCP options (HDMITX_HDCP_OPTION_FORCE_ values)

    \return     The call result:
                - TM_OK: the call was successful
                - Else a problem has been detected
 */
tmErrorCode_t
appHdmiTx_Hdcp_On
(
    tmUnitSelect_t           txUnit,
    tmbslHdmiTxVidFmt_t      voutFmt,
    tmbslHdmiTxHdcpOptions_t options
);

/*============================================================================*/
/**
    \brief      Switch off HDCP

    \param[in]  txUnit       Transmitter unit number

    \return     The call result:
                - TM_OK: the call was successful
                - Else a problem has been detected
 */
tmErrorCode_t
appHdmiTx_Hdcp_Off
(
    tmUnitSelect_t      txUnit
);

/*============================================================================*/
/**
    \brief      Handle HDCP BCAPS interrupt as a callback

    \param[in]  txUnit       Transmitter unit number
    \param[out] pbBksvSecure BKSV check result

    \return     The call result:
                - TM_OK: the call was successful
                - Else a problem has been detected
 */
tmErrorCode_t
appHdmiTx_handleBCAPS
(
    tmUnitSelect_t txUnit,
    Bool *pbBksvSecure
);

/*============================================================================*/
/**
    \brief      Handle HDCP BSTATUS interrupt as a callback

    \param[in]  txUnit       Transmitter unit number

    \return     The call result:
                - TM_OK: the call was successful
                - Else a problem has been detected
 */
tmErrorCode_t
appHdmiTx_handleBSTATUS
(
    tmUnitSelect_t  txUnit
);

/*============================================================================*/
/**
    \brief      Handle HDCP ENCRYPT interrupt as a callback

    \param[in]  txUnit       Transmitter unit number

    \return     The call result:
                - TM_OK: the call was successful
                - Else a problem has been detected
 */
tmErrorCode_t
appHdmiTx_handleENCRYPT
(
    tmUnitSelect_t  txUnit
);

/*============================================================================*/
/**
    \brief      Handle HDCP PJ interrupt as a callback

    \param[in]  txUnit       Transmitter unit number

    \return     The call result:
                - TM_OK: the call was successful
                - Else a problem has been detected
 */
tmErrorCode_t
appHdmiTx_handlePJ
(
    tmUnitSelect_t  txUnit
);

/*============================================================================*/
/**
    \brief      Handle HDCP SHA_1 interrupt as a callback

    \param[in]  txUnit       Transmitter unit number

    \return     The call result:
                - TM_OK: the call was successful
                - Else a problem has been detected
 */
tmErrorCode_t
appHdmiTx_handleSHA_1
(
    tmUnitSelect_t  txUnit
);

/*============================================================================*/
/**
    \brief      Handle HDCP T0 interrupt as a callback

    \param[in]  txUnit      Transmitter unit number

    \return     The call result:
                - TM_OK: the call was successful
                - Else a problem has been detected
 */
tmErrorCode_t
appHdmiTx_handleT0
(
    tmUnitSelect_t  txUnit
);

/*============================================================================*/
/**
    \brief      Handle Hot Plug Detect interrupt as a callback

    \param[in]  txUnit          Transmitter unit number
    \param[out] pHotPlugStatus  Pointer to hot plug status
    \param[out] pEdidStatus     Copy of pDis->EdidStatus
    \param[out] pEdidVidFlags   Ptr to video capability flags
                                See enum _tmbslHdmiTxVidCap_t

    \return     The call result:
                - TM_OK: the call was successful
                - Else a problem has been detected
 */
tmErrorCode_t
appHdmiTx_handleHPD
(
    tmUnitSelect_t          txUnit,
    tmbslHdmiTxHotPlug_t    *pHotPlugStatus,
    UInt8                   *pEdidStatus,
    UInt8                   *pEdidVidFlags
);

/*============================================================================*/
/**
    \brief      Get the pointer to the KSV list used by the App unit

    \param[in]  ppKsvList      Pointer to pointer to KSV list
    \param[in]  pKsvDevices    Pointer to device count 0 to 128

    \return     None
 */
void 
appHdmiTx_getKsvList
(
    UInt8          **ppKsvList,
    UInt8          *pKsvDevices
);

/*============================================================================*/
/**
    \brief      Get the pointer to the BKSV used by the App unit

    \param[in]  ppBksv      Pointer to pointer to BKSV

    \return     None
 */
void 
appHdmiTx_getBksv
(
    UInt8          **ppBksv
);

/*============================================================================*/
/**
    \brief      Check a video format against the Short Video
                Descriptors last read from EDID

    \param[in]  vidFmt  Video format to check

    \return     The call result:
                - True:  vidFmt is in SVD list
                - False: not in list
 */
Bool
appHdmiTx_checkVidFmt
(
    tmbslHdmiTxVidFmt_t     vidFmt
);

/*============================================================================*/
/**
    \brief      Find the next video format in the Short Video Descriptors list
                last read from the EDID

    \param[in]  vidFmtOld      Video format whose successor must be found
    \param[in]  *pVidFmtNew    Ptr to a variable to receive the next higher
                               video format or the first format in SVD list

    \return     The call result:
                - True:  a format was found
                - False: no format was found because the list was empty
 */
Bool
appHdmiTx_nextEdidVidFmt
(
    tmbslHdmiTxVidFmt_t     vidFmtOld,
    tmbslHdmiTxVidFmt_t     *pVidFmtNew
);

/*============================================================================*/
/**
    \brief      Read Edid after PowerOn

    \param[in]  txUnit          Transmitter unit number
    \param[out] pEdidStatus     Copy of pDis->EdidStatus
    \param[out] pEdidVidFlags   Ptr to video capability flags
                                See enum _tmbslHdmiTxVidCap_t

    \return     The call result:
                - TM_OK: the call was successful
                - Else a problem has been detected
 */
tmErrorCode_t 
appHdmiTx_ReadEdidAtPowerOn
(
    tmUnitSelect_t          txUnit,
    UInt8                   *pEdidStatus,
    UInt8                   *pEdidVidFlags
);




/*============================================================================*/
/**
    \brief      set revocation list

    \param[in]  listPtr                 pointer on revocation list
    \param[in]  Length                  number of bksv in revocation list
*/
tmErrorCode_t 
appHdmiTx_SetHDCPRevocationList
(
    void           *listPtr,
    UInt32          Length
);


#ifdef __cplusplus
}
#endif

#endif /* TMBSLHDMITX_APP_H */
/*============================================================================*/
/*                            END OF FILE                                     */
/*============================================================================*/

