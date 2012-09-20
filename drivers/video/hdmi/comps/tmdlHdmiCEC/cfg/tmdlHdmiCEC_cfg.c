/**
 * Copyright (C) 2006 NXP N.V., All Rights Reserved.
 * This source code and any compilation or derivative thereof is the proprietary
 * information of NXP N.V. and is confidential in nature. Under no circumstances
 * is this software to be  exposed to or placed under an Open Source License of
 * any type without the expressed written permission of NXP N.V.
 *
 * \file          tmdlHdmiCEC_cfg.c
 *
 * \version       Revision: 1
 *
 * \date          Date: 
 *
 * \brief         devlib driver component API for the CEC message
 *
 * \section refs  Reference Documents
 * 
 *
 * \section info  Change Information
 *
 * \verbatim

   History:       tmdlHdmiCEC_cfg.c
 *

   \endverbatim
 *
*/

/*============================================================================*/
/*                             INCLUDE FILES                                  */
/*============================================================================*/
#include "tmdlHdmiCEC_IW.h"
#include "tmNxTypes.h"
#include "tmdlHdmiCEC.h"
#include "tmdlHdmiCEC_cfg.h"


#ifdef   TMFL_OS_WINDOWS    /* OS Windows */
#include "infra_i2c.h"
#else                       /* OS ARM7 */
#include "I2C.h"
#include <LPC21xx.H>   
#endif                      /* endif TMFL_OS_WINDOWS */


/******************************************************************************
******************************************************************************
*                 THIS PART CAN BE MODIFIED BY CUSTOMER                      *
******************************************************************************
*****************************************************************************/
/*============================================================================*/
/*                          INTERNAL PROTOTYPE                                */
/*============================================================================*/

#ifdef TMFL_OS_WINDOWS      /* OS Windows */ 
tmErrorCode_t windowsI2cReadFunction(tmbslHdmiTxSysArgs_t *pSysArgs);
tmErrorCode_t windowsI2cWriteFunction(tmbslHdmiTxSysArgs_t *pSysArgs);
#else                       /* OS ARM7 */
tmErrorCode_t RtxI2cReadFunction(tmdlHdmiCecSysArgs_t *pSysArgs);
tmErrorCode_t RtxI2cWriteFunction(tmdlHdmiCecSysArgs_t *pSysArgs);
#endif                      /* endif TMFL_OS_WINDOWS */

/*============================================================================*/
/*                                MACRO                                       */
/*============================================================================*/

/* macro for quick error handling */
#define RETIF(cond, rslt) if ((cond)){return (rslt);}

/*============================================================================*/
/*                          TYPES DECLARATIONS                                */
/*============================================================================*/

/*============================================================================*/
/*                       CONSTANTS DECLARATIONS                               */
/*============================================================================*/

/* Configuration for unit 0 *************************** */
#ifdef TMFL_OS_WINDOWS         /* OS Windows */
#define COMMAND_TASK_PRIORITY_0  THREAD_PRIORITY_HIGHEST
#else                          /* OS ARM7 */
#define COMMAND_TASK_PRIORITY_0  250
#endif                        /* endif TMFL_OS_WINDOWS */
/* stack size of the command tasks */
#define COMMAND_TASK_STACKSIZE_0 128
/* size of the message queues for command tasks */
#define COMMAND_TASK_QUEUESIZE_0 8
/* I2C adress of the unit */
#ifdef TMFL_TDA9996
	#define UNIT_I2C_ADDRESS_0 0x60 /* I2C Address of TDA9950 */
#else
	#define UNIT_I2C_ADDRESS_0 0x34 /* I2C Address of TDA9950 */
#endif

/* Configuration for unit 1 *************************** */
/* priority of the command tasks */
#ifdef TMFL_OS_WINDOWS         /* OS Windows */
#define COMMAND_TASK_PRIORITY_1  THREAD_PRIORITY_HIGHEST
#else                          /* OS ARM7 */
#define COMMAND_TASK_PRIORITY_1  250
#endif                        /* endif TMFL_OS_WINDOWS */
/* stack size of the command tasks */
#define COMMAND_TASK_STACKSIZE_1 128
/* size of the message queues for command tasks */
#define COMMAND_TASK_QUEUESIZE_1 8
/* I2C adress of the unit */
#ifdef TMFL_TDA9996
	#define UNIT_I2C_ADDRESS_1 0x60 /* I2C Address of TDA9950 */
#else
	#define UNIT_I2C_ADDRESS_1 0x34 /* I2C Address of TDA9950 */
#endif


/*============================================================================*/
/*                        DEFINES DECLARATIONS                                */
/*============================================================================*/

/*============================================================================*/
/*                       VARIABLES DECLARATIONS                               */
/*============================================================================*/

/**
 * \brief List of the capabilities to be enabled by the device library
 */ 
tmdlHdmiCecCapabilities_t CeccapabilitiesList = {TMDL_HDMICEC_DEVICE_UNKNOWN, CEC_VERSION_1_3a};

/**
 * \brief Configuration Tables. This table can be modified by the customer 
            to choose its prefered configuration
 */

#ifdef TMFL_OS_WINDOWS 
tmdlHdmiCecDriverConfigTable_t CecdriverConfigTable[MAX_UNITS] = {
    {
    COMMAND_TASK_PRIORITY_0,
    COMMAND_TASK_STACKSIZE_0,
    COMMAND_TASK_QUEUESIZE_0,
    UNIT_I2C_ADDRESS_0,
    windowsI2cReadFunction,
    windowsI2cWriteFunction,
    &CeccapabilitiesList
    }
};
#else
tmdlHdmiCecDriverConfigTable_t CecdriverConfigTable[MAX_UNITS] = {
    {
    COMMAND_TASK_PRIORITY_0,
    COMMAND_TASK_STACKSIZE_0,
    COMMAND_TASK_QUEUESIZE_0,
    UNIT_I2C_ADDRESS_0,
    RtxI2cReadFunction,
    RtxI2cWriteFunction,
    &CeccapabilitiesList
    }
};
#endif

#ifdef TMFL_OS_WINDOWS 
/*============================================================================*/
/*                              FUNCTIONS                                     */
/*============================================================================*/


/**
    \brief Write to BSL driver through I2C bus

    \param pSysArgs Pointer to the I2C read structure

    \return The call result:
            - TM_OK: the call was successful

******************************************************************************/
tmErrorCode_t
windowsI2cReadFunction
(
    tmdlHdmiCecSysArgs_t        *pSysArgs
)
{
    tmErrorCode_t  errCode;

	#ifdef TMFL_TDA9996
		errCode = i2cRead(reg_TDA9996, (tmbslHdmiSysArgs_t *) pSysArgs);
	#else
		errCode = i2cRead(reg_TDA9950, (tmbslHdmiSysArgs_t *) pSysArgs);
	#endif

    return errCode;
}
/**
    \brief Write to BSL driver through I2C bus

    \param pSysArgs Pointer to the I2C write structure

    \return The call result:
            - TM_OK: the call was successful

******************************************************************************/
tmErrorCode_t
windowsI2cWriteFunction
(
    tmdlHdmiCecSysArgs_t *pSysArgs
)
{
    tmErrorCode_t   errCode;

    #ifdef TMFL_TDA9996
		errCode = i2cWrite(reg_TDA9996,(tmbslHdmiSysArgs_t *) pSysArgs);
	#else
		errCode = i2cWrite(reg_TDA9950,(tmbslHdmiSysArgs_t *) pSysArgs);
	#endif


    return errCode;
}
#else

/*============================================================================*/

/**
    \brief Write to BSL driver through I2C bus

    \param pSysArgs Pointer to the I2C read structure

    \return The call result:
            - TM_OK: the call was successful

******************************************************************************/
tmErrorCode_t
RtxI2cReadFunction
(
    tmdlHdmiCecSysArgs_t        *pSysArgs
)
{
    tmErrorCode_t               errCode;

    errCode = i2cRead(reg_TDA9950, (tmbslHdmiSysArgs_t *) pSysArgs);

    return errCode;
}
/**
    \brief Write to BSL driver through I2C bus

    \param pSysArgs Pointer to the I2C write structure

    \return The call result:
            - TM_OK: the call was successful

******************************************************************************/
tmErrorCode_t
RtxI2cWriteFunction
(
    tmdlHdmiCecSysArgs_t *pSysArgs
)
{
    tmErrorCode_t               errCode;

    errCode = i2cWrite(reg_TDA9950,(tmbslHdmiSysArgs_t *) pSysArgs);

    return errCode;
}

#endif

/******************************************************************************
******************************************************************************
*                THIS PART MUST NOT BE MODIFIED BY CUSTOMER                  *
******************************************************************************
*****************************************************************************/

/**
    \brief This function allows to the main driver to retrieve its
           configuration parameters.

    \param pConfig Pointer to the config structure

    \return The call result:
            - TM_OK: the call was successful
            - TMDL_ERR_DLHDMICEC_BAD_UNIT_NUMBER: the unit number is wrong or
              the receiver instance is not initialised
            - TMDL_ERR_DLHDMICEC_INCONSISTENT_PARAMS: an input parameter is
              inconsistent

******************************************************************************/
tmErrorCode_t tmdlHdmiCecCfgGetConfig
(
    tmUnitSelect_t                 unit,
    tmdlHdmiCecDriverConfigTable_t *pConfig
)
{
    /* check if unit number is in range */
    RETIF((unit < 0) || (unit >= MAX_UNITS), TMDL_ERR_DLHDMICEC_BAD_UNIT_NUMBER)

    /* check if pointer is Null */
    RETIF(pConfig == Null, TMDL_ERR_DLHDMICEC_INCONSISTENT_PARAMS)

    *pConfig = CecdriverConfigTable[unit];

    return(TM_OK);
};
/*============================================================================*/
/*                            END OF FILE                                     */
/*============================================================================*/
