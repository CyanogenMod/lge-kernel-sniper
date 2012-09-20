/**
 * Copyright (C) 2006 Koninklijke Philips Electronics N.V., All Rights Reserved.
 * This source code and any compilation or derivative thereof is the proprietary
 * information of Koninklijke Philips Electronics N.V. and is confidential in
 * nature. Under no circumstances is this software to be  exposed to or placed
 * under an Open Source License of any type without the expressed written
 * permission of Koninklijke Philips Electronics N.V.
 *
 * \file          tmbslTDA9984_State.h
 *
 * \version       $Revision: 59 $
 *
 * \date          $Date: 13/06/07 17:00 $
 *
 * \brief         BSL driver component local definitions for the TDA998x
 *                HDMI Transmitter.
 *
 * \section refs  Reference Documents
 *
 * \section info  Change Information
 *
 * \verbatim
 * $History: tmbslTDA9984_State.h $
 *
 * *****************  Version 1  *****************
 * User: Mayhew       Date: 19/06/07    Time: 16:30
 * Created in $/Source/tmbslTDA9984/Src
 * Driver local API component tmbslTDA9984_State

   \endverbatim
 *
*/

#ifndef TMBSLTDA9984_STATE_L_H
#define TMBSLTDA9984_STATE_L_H

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


/*============================================================================*/
/*                       EXTERN FUNCTION PROTOTYPES                           */
/*============================================================================*/

tmErrorCode_t    setState (tmHdmiTxobject_t *pDis, tmbslTDA9984Event_t event);

#ifdef __cplusplus
}
#endif

#endif /* TMBSLTDA9984_STATE_L_H */
/*============================================================================*/
/*                            END OF FILE                                     */
/*============================================================================*/
