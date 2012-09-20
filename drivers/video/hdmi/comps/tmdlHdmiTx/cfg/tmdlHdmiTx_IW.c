/*
 * Copyright (C) 2007 NXP N.V., All Rights Reserved.
 * This source code and any compilation or derivative thereof is the proprietary
 * information of NXP N.V. and is confidential in nature. Under no circumstances
 * is this software to be  exposed to or placed under an Open Source License of
 * any type without the expressed written permission of NXP N.V.
 *
 * \file          tmdlHdmiTx_IW.c
 *
 * \version       $Revision: 1 $
 *
 * \date          $Date: 07/08/07 16:00 $
 *
 * \brief         devlib driver component API for the TDA998x HDMI Transmitters
 *                  
 * \section refs  Reference Documents
 * TDA998x Driver - FRS.doc,
 * TDA998x Driver - tmdlHdmiTx - SCS.doc
 *
 * \section info  Change Information
 *
 * \verbatim

   $History: tmdlHdmiTx_IW.c $
 *
 * *****************  Version 1  *****************
 * User: J. Lamotte Date: 07/08/07   Time: 16:00
 * Updated in $/Source/tmdlHdmiTx/inc
 * initial version
 *

   \endverbatim
 *
*/

/*============================================================================*/
/*                       INCLUDE FILES                                        */
/*============================================================================*/

#define _WIN32_WINNT 0x0500

#ifndef WINDOWS_QMORE
#include "windows.h"
#endif


#include "tmNxTypes.h"
#include "tmNxCompId.h"
#include "tmdlHdmiTx_Types.h"
#include "tmdlHdmiTx_cfg.h"
#include "tmdlHdmiTx_IW.h"

#ifdef __cplusplus
extern "C" {
#endif

/*============================================================================*/
/*                       DEFINES                                              */
/*============================================================================*/

/* maximum number of tasks that can be handled by the wrapper */
#define MAX_TASKS  5
/* maximum number of message queues that can be handled by the wrapper */
#define MAX_QUEUES 5

/*============================================================================*/
/*                                MACRO                                       */
/*============================================================================*/

/* macro for quick error handling */
#define RETIF(cond, rslt) if ((cond)){return (rslt);}

/*============================================================================*/
/*                       TYPE DEFINITIONS                                     */
/*============================================================================*/

/* structure describing each task handled by the wrapper */
typedef struct
{
    Bool                    created;
    Bool                    started;
    UInt8                   priority;
    UInt16                  stackSize;
    DWORD                   threadID;
    HANDLE                  threadHandle;
    LPTHREAD_START_ROUTINE  associatedThread;
    tmdlHdmiTxIWFuncPtr_t   associatedTask;
} iwTcb_t;

/* structure describing each message queue handled by the wrapper */
typedef struct
{
    Bool    created;
    HANDLE  accessSemaphore;
    HANDLE  countSemaphore;
    UInt16  queueFullness;
    UInt16  queueSize;
    UInt16  writePointer;
    UInt16  readPointer;
    UInt8  *queue;
} iwQueue_t;

/*============================================================================*/
/*                       FUNCTION PROTOTYPES                                  */
/*============================================================================*/

DWORD WINAPI ThreadProc0(LPVOID lpParameter);
DWORD WINAPI ThreadProc1(LPVOID lpParameter);
DWORD WINAPI ThreadProc2(LPVOID lpParameter);
DWORD WINAPI ThreadProc3(LPVOID lpParameter);
DWORD WINAPI ThreadProc4(LPVOID lpParameter);

/*============================================================================*/
/*                       VARIABLES                                            */
/*============================================================================*/

/* table storing all tasks descriptions */
iwTcb_t taskTable[MAX_TASKS] =
{
    {False, False, 0, 0, 0, Null, ThreadProc0},
    {False, False, 0, 0, 0, Null, ThreadProc1},
    {False, False, 0, 0, 0, Null, ThreadProc2},
    {False, False, 0, 0, 0, Null, ThreadProc3},
    {False, False, 0, 0, 0, Null, ThreadProc4}
};

/* table storing all message queues descriptions */
iwQueue_t queueTable[MAX_QUEUES] =
{
    {False, 0, 0, 0, 0, 0, 0, Null},
    {False, 0, 0, 0, 0, 0, 0, Null},
    {False, 0, 0, 0, 0, 0, 0, Null},
    {False, 0, 0, 0, 0, 0, 0, Null},
    {False, 0, 0, 0, 0, 0, 0, Null}
};

/*============================================================================*/
/*                                  FUNCTION                                  */
/*============================================================================*/

/******************************************************************************
    \brief  This function creates a task and allocates all the necessary resources.
            Note that creating a task do not start it automatically, 
            an explicit call to tmdlHdmiTxIWTaskStart must be made.

    \param  pFunc        Pointer to the function that will be executed in the task context.
    \param  Priority     Priority of the task. The minimum priority is 0, the maximum is 255.
    \param  StackSize    Size of the stack to allocate for this task.
    \param  pHandle      Pointer to the handle buffer.

    \return The call result:
            - TM_OK: the call was successful
            - TMDL_ERR_DLHDMITX_NO_RESOURCES: the resource is not available
            - TMDL_ERR_DLHDMITX_INCONSISTENT_PARAMS: an input parameter is
              inconsistent

******************************************************************************/
tmErrorCode_t tmdlHdmiTxIWTaskCreate
(
    tmdlHdmiTxIWFuncPtr_t       pFunc,
    UInt8                       priority,
    UInt16                      stackSize,
    tmdlHdmiTxIWTaskHandle_t    *pHandle
)
{
    UInt32 i;

    /* check that input pointer is not NULL */
    RETIF(pFunc == Null, TMDL_ERR_DLHDMITX_INCONSISTENT_PARAMS)

    /* check that input pointer is not NULL */
    RETIF(pHandle == Null, TMDL_ERR_DLHDMITX_INCONSISTENT_PARAMS)

    /* search for available task slot */
    for(i = 0; i < MAX_TASKS; i++)
    {
        if (taskTable[i].created == False)
            break;

    }
    RETIF(i >= MAX_TASKS, TMDL_ERR_DLHDMITX_NO_RESOURCES)

    /* store task parameters into the dedicated structure */
    taskTable[i].priority = priority;
    taskTable[i].stackSize = stackSize;
    taskTable[i].associatedTask = pFunc;
    taskTable[i].created = True;

    *pHandle = (tmdlHdmiTxIWTaskHandle_t)i;

    return(TM_OK);
}

/******************************************************************************
    \brief  This function destroys an existing task and frees resources used by it.

    \param  Handle  Handle of the task to be destroyed, as returned by 
                    tmdlHdmiTxIWTaskCreate.

    \return The call result:
            - TM_OK: the call was successful
            - TMDL_ERR_DLHDMITX_RESOURCE_NOT_OWNED: the caller does not own 
              the resource
            - TMDL_ERR_DLHDMITX_BAD_HANDLE: the handle number is wrong

******************************************************************************/
tmErrorCode_t tmdlHdmiTxIWTaskDestroy
(
    tmdlHdmiTxIWTaskHandle_t handle
)
{
    /* check if handle number is in range */
    RETIF((handle < 0) || (handle >= MAX_TASKS), TMDL_ERR_DLHDMITX_BAD_HANDLE)

    /* check if handle corresponding to task is created */
    RETIF(taskTable[handle].created == False, TMDL_ERR_DLHDMITX_RESOURCE_NOT_OWNED)

    if (taskTable[handle].started == True)
    {
        TerminateThread(taskTable[handle].threadHandle, 0);
        taskTable[handle].started = False;
    }
    taskTable[handle].created = False;
    CloseHandle(taskTable[handle].threadHandle);

    return(TM_OK);
}

/******************************************************************************
    \brief  This function start an existing task.

    \param  Handle  Handle of the task to be started.

    \return The call result:
            - TM_OK: the call was successful
            - TMDL_ERR_DLHDMITX_ALREADY_STARTED: the function is already started
            - TMDL_ERR_DLHDMITX_NOT_STARTED: the function is not started
            - TMDL_ERR_DLHDMITX_BAD_HANDLE: the handle number is wrong

******************************************************************************/
tmErrorCode_t tmdlHdmiTxIWTaskStart
(
    tmdlHdmiTxIWTaskHandle_t handle
)
{
    HANDLE threadHandle;

    /* check if handle number is in range */
    RETIF((handle < 0) || (handle >= MAX_TASKS), TMDL_ERR_DLHDMITX_BAD_HANDLE)

    /* check if task is already started */
    RETIF(taskTable[handle].started == True, TMDL_ERR_DLHDMITX_ALREADY_STARTED)

    /* start thread associated to the task */
    threadHandle = CreateThread(NULL,
                                (SIZE_T)taskTable[handle].stackSize,
                                taskTable[handle].associatedThread,
                                NULL,
                                0,
                                &(taskTable[handle].threadID));

    /* check return code for errors */
    RETIF(!threadHandle, TMDL_ERR_DLHDMITX_NOT_STARTED)

    /* set the priority task */
    SetThreadPriority(threadHandle,(int)taskTable[handle].priority);

    /* update task status */
    taskTable[handle].threadHandle = threadHandle;
    taskTable[handle].started = True;

    return(TM_OK);
}

/******************************************************************************
    \brief  This function blocks the current task for the specified amount time. 
            This is a passive wait.

    \param  Duration    Duration of the task blocking in milliseconds.

    \return The call result:
            - TM_OK: the call was successful
            - TMDL_ERR_DLHDMITX_NO_RESOURCES: the resource is not available

******************************************************************************/
tmErrorCode_t tmdlHdmiTxIWWait
(
    UInt16 duration
)
{
    HANDLE        timerHandle;
    LARGE_INTEGER time;

    timerHandle = CreateWaitableTimer(NULL, TRUE, NULL);
    RETIF(timerHandle == NULL, TMDL_ERR_DLHDMITX_NO_RESOURCES)

    time.QuadPart = -10000 * (long)duration;
    SetWaitableTimer(timerHandle, &time, 0, NULL, NULL, FALSE);
    WaitForSingleObject(timerHandle, INFINITE);

    CloseHandle(timerHandle);

    return(TM_OK);
}

/******************************************************************************
    \brief  This function creates a message queue.

    \param  QueueSize   Maximum number of messages in the message queue.
    \param  pHandle     Pointer to the handle buffer.
    
    \return The call result:
            - TM_OK: the call was successful
            - TMDL_ERR_DLHDMITX_INCONSISTENT_PARAMS: an input parameter is
              inconsistent
            - TMDL_ERR_DLHDMITX_NO_RESOURCES: the resource is not available

******************************************************************************/
tmErrorCode_t tmdlHdmiTxIWQueueCreate
(
    UInt8                       queueSize,
    tmdlHdmiTxIWQueueHandle_t   *pHandle
)
{
    UInt32 i;

    /* check that input pointer is not NULL */
    RETIF(pHandle == Null, TMDL_ERR_DLHDMITX_INCONSISTENT_PARAMS)

    /* search for available queue slot */
    for(i = 0; i < MAX_QUEUES; i++)
    {
        if (queueTable[i].created == False)
            break;

    }
    
    RETIF(i >= MAX_QUEUES, TMDL_ERR_DLHDMITX_NO_RESOURCES)

    /* allocate memory for the queue */
    queueTable[i].queue = (UInt8 *)GlobalAlloc(GMEM_FIXED, queueSize);
    RETIF(queueTable[i].queue == NULL, TMDL_ERR_DLHDMITX_NO_RESOURCES)

    /* allocate semaphores for the queue */
    queueTable[i].countSemaphore = CreateSemaphore(NULL, 0, queueSize, NULL);
    RETIF(queueTable[i].countSemaphore == NULL, TMDL_ERR_DLHDMITX_NO_RESOURCES)

    queueTable[i].accessSemaphore = CreateSemaphore(NULL, 1, 1, NULL);
    RETIF(queueTable[i].accessSemaphore == NULL, TMDL_ERR_DLHDMITX_NO_RESOURCES)

    /* update status of the queue table */
    queueTable[i].created = True;
    queueTable[i].queueSize = queueSize;
    *pHandle = (tmdlHdmiTxIWQueueHandle_t)i;

    return(TM_OK);
}

/******************************************************************************
    \brief  This function destroys an existing message queue.

    \param  Handle  Handle of the queue to be destroyed.

    \return The call result:
            - TM_OK: the call was successful
            - TMDL_ERR_DLHDMITX_BAD_HANDLE: the handle number is wrong
            - TMDL_ERR_DLHDMITX_RESOURCE_NOT_OWNED: the caller does not own 
              the resource

******************************************************************************/
tmErrorCode_t tmdlHdmiTxIWQueueDestroy
(
    tmdlHdmiTxIWQueueHandle_t handle
)
{
    RETIF(handle > MAX_QUEUES, TMDL_ERR_DLHDMITX_BAD_HANDLE)

    RETIF(queueTable[handle].created == False, TMDL_ERR_DLHDMITX_RESOURCE_NOT_OWNED)

    GlobalFree((LPVOID)queueTable[handle].queue);
    CloseHandle(queueTable[handle].countSemaphore);
    CloseHandle(queueTable[handle].accessSemaphore);
    queueTable[handle].created = False;

    return(TM_OK);
}

/******************************************************************************
    \brief  This function sends a message into the specified message queue.

    \param  Handle  Handle of the queue that will receive the message.
    \param  Message Message to be sent.
    
    \return The call result:
            - TM_OK: the call was successful
            - TMDL_ERR_DLHDMITX_BAD_HANDLE: the handle number is wrong
            - TMDL_ERR_DLHDMITX_RESOURCE_NOT_OWNED: the caller does not own 
              the resource
            - TMDL_ERR_DLHDMITX_FULL: the queue is full

******************************************************************************/
tmErrorCode_t tmdlHdmiTxIWQueueSend
(
    tmdlHdmiTxIWQueueHandle_t   handle,
    UInt8                       message
)
{
    tmErrorCode_t errorCode = TM_OK;

    /* check that handle is correct */
    RETIF(handle > MAX_QUEUES, TMDL_ERR_DLHDMITX_BAD_HANDLE)

    RETIF(queueTable[handle].created != True, TMDL_ERR_DLHDMITX_RESOURCE_NOT_OWNED)

    /* ask for exclusive access to this queue */
    WaitForSingleObject(queueTable[handle].accessSemaphore, INFINITE);

    if(queueTable[handle].queueFullness < (queueTable[handle].queueSize - 1))
    {
        queueTable[handle].queue[queueTable[handle].writePointer] = message;
        queueTable[handle].queueFullness++;
        queueTable[handle].writePointer++;
        if(queueTable[handle].writePointer == queueTable[handle].queueSize)
        {
            queueTable[handle].writePointer = 0;
        }
        ReleaseSemaphore(queueTable[handle].countSemaphore, 1, NULL);
    }
    else
    {
        errorCode = TMDL_ERR_DLHDMITX_FULL;
    }

    /* release access to this queue */
    ReleaseSemaphore(queueTable[handle].accessSemaphore, 1, NULL);

    return(TM_OK);
}

/******************************************************************************
    \brief  This function reads a message from the specified message queue.

    \param  Handle      Handle of the queue from which to read the message.
    \param  pMessage    Pointer to the message buffer.

    \return The call result:
            - TM_OK: the call was successful
            - TMDL_ERR_DLHDMITX_BAD_HANDLE: the handle number is wrong
            - TMDL_ERR_DLHDMITX_RESOURCE_NOT_OWNED: the caller does not own 
              the resource
            - TMDL_ERR_DLHDMITX_INCONSISTENT_PARAMS: an input parameter is
              inconsistent

******************************************************************************/
tmErrorCode_t tmdlHdmiTxIWQueueReceive
(
    tmdlHdmiTxIWQueueHandle_t handle, UInt8 *pMessage
)
{
    /* check that handle is correct */
    RETIF(handle > MAX_QUEUES, TMDL_ERR_DLHDMITX_BAD_HANDLE)
    
    RETIF(queueTable[handle].created != True, TMDL_ERR_DLHDMITX_RESOURCE_NOT_OWNED)

    /* check that input pointer is not NULL */
    RETIF(pMessage == Null, TMDL_ERR_DLHDMITX_INCONSISTENT_PARAMS)

    /* ask for a new message by acquiring the counting semaphore */
    WaitForSingleObject(queueTable[handle].countSemaphore, INFINITE);

    /* if we reach this point, this means that we got a message */
    /* ask for exclusive access to this queue */
    WaitForSingleObject(queueTable[handle].accessSemaphore, INFINITE);

    *pMessage = queueTable[handle].queue[queueTable[handle].readPointer];
    queueTable[handle].queueFullness--;
    queueTable[handle].readPointer++;
    if(queueTable[handle].readPointer == queueTable[handle].queueSize)
    {
        queueTable[handle].readPointer = 0;
    }

    /* release access to this queue */
    ReleaseSemaphore(queueTable[handle].accessSemaphore, 1, NULL);

    return(TM_OK);
}

/******************************************************************************
    \brief  This function creates a semaphore.

    \param  pHandle Pointer to the handle buffer.

    \return The call result:
            - TM_OK: the call was successful
            - TMDL_ERR_DLHDMITX_NO_RESOURCES: the resource is not available
            - TMDL_ERR_DLHDMITX_INCONSISTENT_PARAMS: an input parameter is
              inconsistent

******************************************************************************/
tmErrorCode_t tmdlHdmiTxIWSemaphoreCreate
(
    tmdlHdmiTxIWSemHandle_t *pHandle
)
{
    /* check that input pointer is not NULL */
    RETIF(pHandle == Null, TMDL_ERR_DLHDMITX_INCONSISTENT_PARAMS)

    *pHandle = (tmdlHdmiTxIWSemHandle_t)CreateSemaphore(NULL, 1, 1, NULL);

    RETIF((*pHandle) == NULL, TMDL_ERR_DLHDMITX_NO_RESOURCES)

    return(TM_OK);
}

/******************************************************************************
    \brief  This function destroys an existing semaphore.

    \param  Handle  Handle of the semaphore to be destroyed.

    \return The call result:
            - TM_OK: the call was successful
            - TMDL_ERR_DLHDMITX_BAD_HANDLE: the handle number is wrong

******************************************************************************/
tmErrorCode_t tmdlHdmiTxIWSemaphoreDestroy
(
    tmdlHdmiTxIWSemHandle_t handle
)
{
    RETIF(CloseHandle(handle) == False, TMDL_ERR_DLHDMITX_BAD_HANDLE)

    return(TM_OK);
}

/******************************************************************************
    \brief  This function acquires the specified semaphore.

    \param  Handle  Handle of the semaphore to be acquired.

    \return The call result:
            - TM_OK: the call was successful
            - TMDL_ERR_DLHDMITX_BAD_HANDLE: the handle number is wrong

******************************************************************************/
tmErrorCode_t tmdlHdmiTxIWSemaphoreP
(
    tmdlHdmiTxIWSemHandle_t handle
)
{
    RETIF(WaitForSingleObject(handle, INFINITE) != WAIT_OBJECT_0, TMDL_ERR_DLHDMITX_BAD_HANDLE)

    return(TM_OK);
}

/******************************************************************************
    \brief  This function releases the specified semaphore.

    \param  Handle  Handle of the semaphore to be released.

    \return The call result:
            - TM_OK: the call was successful
            - TMDL_ERR_DLHDMITX_BAD_HANDLE: the handle number is wrong

******************************************************************************/
tmErrorCode_t tmdlHdmiTxIWSemaphoreV
(
    tmdlHdmiTxIWSemHandle_t handle
)
{
    RETIF(ReleaseSemaphore(handle, 1, NULL) == 0, TMDL_ERR_DLHDMITX_BAD_HANDLE)

    return(TM_OK);
}

/******************************************************************************
    \brief  This function disables the interrupts for a specific device.

    \param

    \return The call result:
            - TM_OK: the call was successful

******************************************************************************/
void tmdlHdmiTxIWDisableInterrupts(tmdlHdmiIWDeviceInterrupt_t device)
{
    device;
}

/******************************************************************************
    \brief  This function enables the interrupts for a specific device.

    \param

    \return The call result:
            - TM_OK: the call was successful

******************************************************************************/
void tmdlHdmiTxIWEnableInterrupts(tmdlHdmiIWDeviceInterrupt_t device)
{
    device;
}

#ifdef __cplusplus
}
#endif

/******************************************************************************/
DWORD WINAPI ThreadProc0(LPVOID lpParameter)
{
    /* dummy reference to avoid compilation warning C4100 */
    lpParameter;

    /* call the registered task */
    taskTable[0].associatedTask();
    /* if we reach this point, the task is terminated, so update its status */
    taskTable[0].started = False;

    return(0);
}

/******************************************************************************/
DWORD WINAPI ThreadProc1(LPVOID lpParameter)
{
    /* dummy reference to avoid compilation warning C4100 */
    lpParameter;

    /* call the registered task */
    taskTable[1].associatedTask();
    /* if we reach this point, the task is terminated, so update its status */
    taskTable[1].started = False;

    return(0);
}

/******************************************************************************/
DWORD WINAPI ThreadProc2(LPVOID lpParameter)
{
    /* dummy reference to avoid compilation warning C4100 */
    lpParameter;

    /* call the registered task */
    taskTable[2].associatedTask();
    /* if we reach this point, the task is terminated, so update its status */
    taskTable[2].started = False;

    return(0);
}

/******************************************************************************/
DWORD WINAPI ThreadProc3(LPVOID lpParameter)
{
    /* dummy reference to avoid compilation warning C4100 */
    lpParameter;

    /* call the registered task */
    taskTable[3].associatedTask();
    /* if we reach this point, the task is terminated, so update its status */
    taskTable[3].started = False;

    return(0);
}


DWORD WINAPI ThreadProc4(LPVOID lpParameter)
{
    /* dummy reference to avoid compilation warning C4100 */
    lpParameter;

    /* call the registered task */
    taskTable[4].associatedTask();
    /* if we reach this point, the task is terminated, so update its status */
    taskTable[4].started = False;

    return(0);
}

/*============================================================================*/
/*                            END OF FILE                                     */
/*============================================================================*/

