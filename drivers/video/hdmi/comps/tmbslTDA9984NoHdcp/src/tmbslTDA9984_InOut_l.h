/**
 * Copyright (C) 2006 Koninklijke Philips Electronics N.V., All Rights Reserved.
 * This source code and any compilation or derivative thereof is the proprietary
 * information of Koninklijke Philips Electronics N.V. and is confidential in
 * nature. Under no circumstances is this software to be  exposed to or placed
 * under an Open Source License of any type without the expressed written
 * permission of Koninklijke Philips Electronics N.V.
 *
 * \file          tmbslTDA9984_InOut_l.h
 *
 * \version       $Revision: 2 $
 *
 * \date          $Date: 20/06907 10:30 $
 *
 * \brief         BSL driver component local definitions for the TDA998x
 *                HDMI Transmitter.
 *
 * \section refs  Reference Documents
 *
 * \section info  Change Information
 *
 * \verbatim
 * $History: tmbslTDA9984_InOut_l.h $
 *
 * *****************  Version 2  *****************
 * User: B.Vereecke      Date: 20/09/07    Time: 10:30
 * Created in $/Source/tmbslTDA9984/Src
 * PR734 : fix compilation errors when FORMAT_PC flag 
 *         is defined
 * PR447 : change length of kVfmtToPixClk_PC table from 
 *         HDMITX_VFMT_TV_NUM to HDMITX_VFMT_TV_NUM + 1
 *
 * *****************  Version 1  *****************
 * User: G. Burnouf      Date: 19/06/07    Time: 16:30
 * Created in $/Source/tmbslTDA9984/Src
 * Driver local API component tmbslTDA9984_InOut

   \endverbatim
 *
*/

#ifndef TMBSLTDA9984_INOUT_L_H
#define TMBSLTDA9984_INOUT_L_H

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

extern CONST_DAT tmHdmiTxRegMaskVal_t kCommonPllCfg[];

/**
 * Table of PLL settings registers to configure for 480i and 576i vinFmt
 */
extern CONST_DAT tmHdmiTxRegMaskVal_t kVfmt480i576iPllCfg[];

/**
 * Table of PLL settings registers to configure for single mode pixel rate,
 * vinFmt 480i or 576i only
 */
extern CONST_DAT tmHdmiTxRegMaskVal_t kSinglePrateVfmt480i576iPllCfg[];

/**
 * Table of PLL settings registers to configure for single repeated mode pixel rate,
 * vinFmt 480i or 576i only
 */
extern CONST_DAT tmHdmiTxRegMaskVal_t kSrepeatedPrateVfmt480i576iPllCfg[];

/**
 * Table of PLL settings registers to configure for other vinFmt than 480i and 576i
 */
extern CONST_DAT tmHdmiTxRegMaskVal_t kVfmtOtherPllCfg[];

/**
 * Table of PLL settings registers to configure single mode pixel rate,
 * vinFmt other than 480i or 576i
 */
extern CONST_DAT tmHdmiTxRegMaskVal_t kSinglePrateVfmtOtherPllCfg[];

/**
 * Table of PLL settings registers to configure double mode pixel rate,
 * vinFmt other than 480i or 576i
 */
extern CONST_DAT tmHdmiTxRegMaskVal_t kDoublePrateVfmtOtherPllCfg[];

/**
 * Lookup table to convert from EIA/CEA TV video formats used in the EDID and
 * in API parameters to pixel clock frequencies, according to SCS Table
 * "HDMI Pixel Clock Frequencies per EIA/CEA-861B Video Output Format".
 * The other index is the veritical frame frequency.
 */
 
extern CONST_DAT UInt8 kVfmtToPixClk_TV[HDMITX_VFMT_TV_NUM][HDMITX_VFREQ_NUM];

/**
 * Lookup table to convert PC formats used in API parameters to pixel clock 
 * frequencies.
 * The other index is the vertical frame frequency.
 */

extern CONST_DAT UInt8 kVfmtToPixClk_PC[HDMITX_VFMT_PC_NUM+1];

/*============================================================================*/
/*                       EXTERN FUNCTION PROTOTYPES                           */
/*============================================================================*/



#ifdef __cplusplus
}
#endif

#endif /* TMBSLTDA9984_INOUT_L_H */
/*============================================================================*/
/*                            END OF FILE                                     */
/*============================================================================*/
