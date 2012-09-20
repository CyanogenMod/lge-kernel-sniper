/**
 * Copyright (C) 2006 Koninklijke Philips Electronics N.V., All Rights Reserved.
 * This source code and any compilation or derivative thereof is the proprietary
 * information of Koninklijke Philips Electronics N.V. and is confidential in
 * nature. Under no circumstances is this software to be  exposed to or placed
 * under an Open Source License of any type without the expressed written
 * permission of Koninklijke Philips Electronics N.V.
 *
 * \file          tmbslTDA9984_Edid_l.h
 *
 * \version       $Revision: 3 $
 *
 * \date          $Date: 28/09/07 15:39 $
 *
 * \brief         BSL driver component local definitions for the TDA9984
 *                HDMI Transmitter.
 *
 * \section refs  Reference Documents
 *
 * \section info  Change Information
 *
 * \verbatim
 * $History: tmbslTDA9984_Edid_l.h $
 *
 * *****************  Version 3  *****************
 * User: B. Vereecke    Date: 28/09/07   Time: 15:39
 * Updated in $/Source/tmbslTDA9984/Src
 * PR 766 : Rename pseudo API functions
 *
 * *****************  Version 2  *****************
 * User: G. Burnouf    Date: 06/08/07   Time: 17:00
 * Updated in $/Source/tmbslTDA9984/Src
 * PR 570 : New local function to stop an EDID read
 *          tmbslTDA9984ClearEdidRequest
 *
 * *****************  Version 1  *****************
 * User: G. Burnouf    Date: 04/07/07    Time: 16:30
 * Created in $/Source/tmbslTDA9984/Src
 * Driver local API component tmbslTDA9984_Edid.c

   \endverbatim
 *
*/

#ifndef TMBSLTDA9984_EDID_L_H
#define TMBSLTDA9984_EDID_L_H

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
/*                       EXTERN DATA DEFINITION                               */
/*============================================================================*/

extern tmErrorCode_t
EdidBlockAvailable (tmUnitSelect_t txUnit, Bool * pSendEDIDCallback);

extern tmErrorCode_t
ClearEdidRequest (tmUnitSelect_t txUnit);

/*============================================================================*/
/*                       EXTERN FUNCTION PROTOTYPES                           */
/*============================================================================*/



#ifdef __cplusplus
}
#endif

#endif /* TMBSLTDA9984_EDID_L_H */
/*============================================================================*/
/*                            END OF FILE                                     */
/*============================================================================*/
