/**
 * Copyright (C) 2006 Koninklijke Philips Electronics N.V., All Rights Reserved.
 * This source code and any compilation or derivative thereof is the proprietary
 * information of Koninklijke Philips Electronics N.V. and is confidential in
 * nature. Under no circumstances is this software to be  exposed to or placed
 * under an Open Source License of any type without the expressed written
 * permission of Koninklijke Philips Electronics N.V.
 *
 * \file          tmbslTDA9984_Misc_l.h
 *
 * \version       $Revision: 2 $
 *
 * \date          $Date: 03/10/07 15:32 $
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
 * *****************  Version 2  *****************
 * User: B.Vereecke    Date: 03/10/07   Time: 15:32
 * Updated in $/Source/tmbslTDA9984/Src
 * PR778 : EDID workaround (soft reset while HPD 
 *          is low)
 *
 * *****************  Version 1  *****************
 * User: Mayhew       Date: 19/06/07    Time: 16:30
 * Created in $/Source/tmbslTDA9984/Src
 * Driver local API component tmbslTDA9984_Misc.c

   \endverbatim
 *
*/

#ifndef TMBSLTDA9984_MISC_L_H
#define TMBSLTDA9984_MISC_L_H

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

typedef struct 
{
    UInt16 Register;
    UInt8  MaskSwap;
    UInt8  MaskMirror;
} tmbslTDA9984RegVip;

/*============================================================================*/
/*                       EXTERN DATA DEFINITION                               */
/*============================================================================*/

/**
 * Lookup table of input port control registers and their swap and mirror masks
 */
extern CONST_DAT tmbslTDA9984RegVip kRegVip[HDMITX_VIN_PORT_MAP_TABLE_LEN];


/*============================================================================*/
/*                       EXTERN FUNCTION PROTOTYPES                           */
/*============================================================================*/

/**
 * Routine of chip reinitialisation after hotplug (EDID work around, PR778)
 */

tmErrorCode_t hotPlugRestore ( tmUnitSelect_t  txUnit );

#ifdef __cplusplus
}
#endif

#endif /* TMBSLTDA9984_MISC_L_H */
/*============================================================================*/
/*                            END OF FILE                                     */
/*============================================================================*/
