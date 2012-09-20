/**
 * Copyright (C) 2008 NXP N.V., All Rights Reserved.
 * This source code and any compilation or derivative thereof is the proprietary
 * information of NXP N.V. and is confidential in nature. Under no circumstances
 * is this software to be  exposed to or placed under an Open Source License of
 * any type without the expressed written permission of NXP N.V.
 *
 * \file          tmbslHdmiTx_funcMapping.h
 *
 * \version       $Revision: 1 $
 *
 * \date          $Date: 06-May-2008 $
 *
 * \brief         BSL functions wrapper
 *
*/



#ifndef TMDLHDMITXTDA9984_CFG_H
#define TMDLHDMITXTDA9984_CFG_H

#include "tmbslTDA9984_Functions.h"

#ifdef __cplusplus
extern "C" {
#endif

#define tmbslHdmiTxInit                             tmbslTDA9984Init
#define tmbslHdmiTxEdidRequestBlockData             tmbslTDA9984EdidRequestBlockData
#define tmbslHdmiTxVideoOutSetConfig                tmbslTDA9984VideoOutSetConfig
#define tmbslHdmiTxAudioInResetCts                  tmbslTDA9984AudioInResetCts
#define tmbslHdmiTxAudioInSetConfig                 tmbslTDA9984AudioInSetConfig
#define tmbslHdmiTxAudioInSetCts                    tmbslTDA9984AudioInSetCts
#define tmbslHdmiTxAudioOutSetChanStatus            tmbslTDA9984AudioOutSetChanStatus
#define tmbslHdmiTxAudioOutSetChanStatusMapping     tmbslTDA9984AudioOutSetChanStatusMapping
#define tmbslHdmiTxAudioOutSetMute                  tmbslTDA9984AudioOutSetMute
#define tmbslHdmiTxDeinit                           tmbslTDA9984Deinit
#define tmbslHdmiTxEdidGetAudioCapabilities         tmbslTDA9984EdidGetAudioCapabilities
#define tmbslHdmiTxEdidGetBlockCount                tmbslTDA9984EdidGetBlockCount
#define tmbslHdmiTxEdidGetStatus                    tmbslTDA9984EdidGetStatus
#define tmbslHdmiTxEdidGetSinkType                  tmbslTDA9984EdidGetSinkType
#define tmbslHdmiTxEdidSetSinkType                  tmbslTDA9984EdidSetSinkType
#define tmbslHdmiTxEdidGetSourceAddress             tmbslTDA9984EdidGetSourceAddress
#define tmbslHdmiTxEdidGetVideoCapabilities         tmbslTDA9984EdidGetVideoCapabilities
#define tmbslHdmiTxEdidGetVideoPreferred            tmbslTDA9984EdidGetVideoPreferred
#define tmbslHdmiTxHdcpCheck                        tmbslTDA9984HdcpCheck
#define tmbslHdmiTxHdcpConfigure                    tmbslTDA9984HdcpConfigure
#define tmbslHdmiTxHdcpDownloadKeys                 tmbslTDA9984HdcpDownloadKeys
#define tmbslHdmiTxHdcpEncryptionOn                 tmbslTDA9984HdcpEncryptionOn
#define tmbslHdmiTxHdcpGetOtp                       tmbslTDA9984HdcpGetOtp
#define tmbslHdmiTxHdcpGetT0FailState               tmbslTDA9984HdcpGetT0FailState
#define tmbslHdmiTxHdcpHandleBCAPS                  tmbslTDA9984HdcpHandleBCAPS
#define tmbslHdmiTxHdcpHandleBKSV                   tmbslTDA9984HdcpHandleBKSV
#define tmbslHdmiTxHdcpHandleBKSVResult             tmbslTDA9984HdcpHandleBKSVResult
#define tmbslHdmiTxHdcpHandleBSTATUS                tmbslTDA9984HdcpHandleBSTATUS
#define tmbslHdmiTxHdcpHandleENCRYPT                tmbslTDA9984HdcpHandleENCRYPT
#define tmbslHdmiTxHdcpHandlePJ                     tmbslTDA9984HdcpHandlePJ
#define tmbslHdmiTxHdcpHandleSHA_1                  tmbslTDA9984HdcpHandleSHA_1
#define tmbslHdmiTxHdcpHandleSHA_1Result            tmbslTDA9984HdcpHandleSHA_1Result
#define tmbslHdmiTxHdcpHandleT0                     tmbslTDA9984HdcpHandleT0
#define tmbslHdmiTxHdcpInit                         tmbslTDA9984HdcpInit
#define tmbslHdmiTxHdcpRun                          tmbslTDA9984HdcpRun
#define tmbslHdmiTxHdcpStop                         tmbslTDA9984HdcpStop
#define tmbslHdmiTxHotPlugGetStatus                 tmbslTDA9984HotPlugGetStatus
#define tmbslHdmiTxRxSenseGetStatus                 tmbslTDA9984RxSenseGetStatus
#define tmbslHdmiTxHwGetRegisters                   tmbslTDA9984HwGetRegisters
#define tmbslHdmiTxHwGetVersion                     tmbslTDA9984HwGetVersion
#define tmbslHdmiTxHwGetCapabilities                tmbslTDA9984HwGetCapabilities
#define tmbslHdmiTxHwHandleInterrupt                tmbslTDA9984HwHandleInterrupt
#define tmbslHdmiTxHwSetRegisters                   tmbslTDA9984HwSetRegisters
#define tmbslHdmiTxHwStartup                        tmbslTDA9984HwStartup
#define tmbslHdmiTxMatrixSetCoeffs                  tmbslTDA9984MatrixSetCoeffs
#define tmbslHdmiTxMatrixSetConversion              tmbslTDA9984MatrixSetConversion
#define tmbslHdmiTxMatrixSetInputOffset             tmbslTDA9984MatrixSetInputOffset
#define tmbslHdmiTxMatrixSetMode                    tmbslTDA9984MatrixSetMode
#define tmbslHdmiTxMatrixSetOutputOffset            tmbslTDA9984MatrixSetOutputOffset
#define tmbslHdmiTxPktSetAclkRecovery               tmbslTDA9984PktSetAclkRecovery
#define tmbslHdmiTxPktSetAcp                        tmbslTDA9984PktSetAcp
#define tmbslHdmiTxPktSetAudioInfoframe             tmbslTDA9984PktSetAudioInfoframe
#define tmbslHdmiTxPktSetGeneralCntrl               tmbslTDA9984PktSetGeneralCntrl
#define tmbslHdmiTxPktSetIsrc1                      tmbslTDA9984PktSetIsrc1
#define tmbslHdmiTxPktSetIsrc2                      tmbslTDA9984PktSetIsrc2
#define tmbslHdmiTxPktSetMpegInfoframe              tmbslTDA9984PktSetMpegInfoframe
#define tmbslHdmiTxPktSetNullInsert                 tmbslTDA9984PktSetNullInsert
#define tmbslHdmiTxPktSetNullSingle                 tmbslTDA9984PktSetNullSingle
#define tmbslHdmiTxPktSetSpdInfoframe               tmbslTDA9984PktSetSpdInfoframe
#define tmbslHdmiTxPktSetVideoInfoframe             tmbslTDA9984PktSetVideoInfoframe
#define tmbslHdmiTxPktSetVsInfoframe                tmbslTDA9984PktSetVsInfoframe
#define tmbslHdmiTxPktSetRawVideoInfoframe          tmbslTDA9984PktSetRawVideoInfoframe
#define tmbslHdmiTxPowerGetState                    tmbslTDA9984PowerGetState
#define tmbslHdmiTxPowerSetState                    tmbslTDA9984PowerSetState
#define tmbslHdmiTxReset                            tmbslTDA9984Reset
#define tmbslHdmiTxScalerGet                        tmbslTDA9984ScalerGet
#define tmbslHdmiTxScalerGetMode                    tmbslTDA9984ScalerGetMode
#define tmbslHdmiTxScalerInDisable                  tmbslTDA9984ScalerInDisable
#define tmbslHdmiTxScalerSetCoeffs                  tmbslTDA9984ScalerSetCoeffs
#define tmbslHdmiTxScalerSetFieldOrder              tmbslTDA9984ScalerSetFieldOrder
#define tmbslHdmiTxScalerSetFine                    tmbslTDA9984ScalerSetFine
#define tmbslHdmiTxScalerSetPhase                   tmbslTDA9984ScalerSetPhase
#define tmbslHdmiTxScalerSetLatency                 tmbslTDA9984ScalerSetLatency
#define tmbslHdmiTxScalerSetSync                    tmbslTDA9984ScalerSetSync
#define tmbslHdmiTxSwGetVersion                     tmbslTDA9984SwGetVersion
#define tmbslHdmiTxSysTimerWait                     tmbslTDA9984SysTimerWait
#define tmbslHdmiTxTmdsSetOutputs                   tmbslTDA9984TmdsSetOutputs
#define tmbslHdmiTxTmdsSetSerializer                tmbslTDA9984TmdsSetSerializer
#define tmbslHdmiTxTestSetPattern                   tmbslTDA9984TestSetPattern
#define tmbslHdmiTxTestSetMode                      tmbslTDA9984TestSetMode
#define tmbslHdmiTxVideoInSetBlanking               tmbslTDA9984VideoInSetBlanking
#define tmbslHdmiTxVideoInSetConfig                 tmbslTDA9984VideoInSetConfig
#define tmbslHdmiTxVideoInSetFine                   tmbslTDA9984VideoInSetFine
#define tmbslHdmiTxVideoInSetMapping                tmbslTDA9984VideoInSetMapping
#define tmbslHdmiTxSetVideoPortConfig               tmbslTDA9984SetVideoPortConfig
#define tmbslHdmiTxSetAudioPortConfig               tmbslTDA9984SetAudioPortConfig
#define tmbslHdmiTxSetAudioClockPortConfig          tmbslTDA9984SetAudioClockPortConfig
#define tmbslHdmiTxVideoInSetSyncAuto               tmbslTDA9984VideoInSetSyncAuto
#define tmbslHdmiTxVideoInSetSyncManual             tmbslTDA9984VideoInSetSyncManual
#define tmbslHdmiTxVideoOutDisable                  tmbslTDA9984VideoOutDisable
#define tmbslHdmiTxVideoOutSetSync                  tmbslTDA9984VideoOutSetSync
#define tmbslHdmiTxVideoSetInOut                    tmbslTDA9984VideoSetInOut
#define tmbslHdmiTxFlagSwInt                        tmbslTDA9984FlagSwInt
#define tmbslHdmiTxSet5vpower                       tmbslTDA9984Set5vpower
#define tmbslHdmiTxEnableCallback                   tmbslTDA9984EnableCallback
#define tmbslHdmiTxSetColorDepth                    tmbslTDA9984SetColorDepth
#define tmbslHdmiTxSetDefaultPhase                  tmbslTDA9984SetDefaultPhase
#define tmbslHdmiTxPktFillGamut                     tmbslTDA9984PktFillGamut
#define tmbslHdmiTxPktSendGamut                     tmbslTDA9984PktSendGamut
#define tmbslHdmiTxEdidGetMonitorDescriptors        tmbslTDA9984EdidGetMonitorDescriptors
#define tmbslHdmiTxEdidGetDetailedTimingDescriptors tmbslTDA9984EdidGetDetailedTimingDescriptors
#define tmbslHdmiTxEdidGetBasicDisplayParam         tmbslTDA9984EdidGetBasicDisplayParam
#define tmbslHdmiTxHdcpGetSinkCategory              tmbslTDA9984HdcpGetSinkCategory
#define tmbslHdmiTxEdidGetLatencyInfo               tmbslTDA9984EdidGetLatencyInfo
#define tmbslHdmiTxEdidGetExtraVsdbData             tmbslTDA9984EdidGetExtraVsdbData

#ifdef __cplusplus
}
#endif

#endif /* TMDLHDMITXTDA9984_CFG_H */

/*============================================================================*/
/*                               END OF FILE                                  */
/*============================================================================*/
