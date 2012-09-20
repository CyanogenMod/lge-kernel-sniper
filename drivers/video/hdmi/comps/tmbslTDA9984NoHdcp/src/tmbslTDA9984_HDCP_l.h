/**
 * Copyright (C) 2006 Koninklijke Philips Electronics N.V., All Rights Reserved.
 * This source code and any compilation or derivative thereof is the proprietary
 * information of Koninklijke Philips Electronics N.V. and is confidential in
 * nature. Under no circumstances is this software to be  exposed to or placed
 * under an Open Source License of any type without the expressed written
 * permission of Koninklijke Philips Electronics N.V.
 *
 * \file          tmbslTDA9984_HDCP_l.h
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
 * Driver local API component tmbslTDA9984_HDCP.c

   \endverbatim
 *
*/

#ifndef TMBSLTDA9984_HDCP_L_H
#define TMBSLTDA9984_HDCP_L_H

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

/**
 * Table of registers to switch HDMI HDCP mode off for DVI
 */
extern CONST_DAT tmHdmiTxRegMaskVal_t kVoutHdcpOff[];

/**
 * Table of registers to switch HDMI HDCP mode on for HDMI
 */
extern CONST_DAT tmHdmiTxRegMaskVal_t kVoutHdcpOn[];

/*============================================================================*/
/*                       EXTERN FUNCTION PROTOTYPES                           */
/*============================================================================*/
#ifndef NO_HDCP
extern tmErrorCode_t tmbslTDA9984handleBKSVResultSecure(tmUnitSelect_t txUnit);
#endif /* NO_HDCP */

#ifdef __cplusplus
}
#endif

#endif /* TMBSLTDA9984_HDCP_L_H */
/*============================================================================*/
/*                            END OF FILE                                     */
/*============================================================================*/
