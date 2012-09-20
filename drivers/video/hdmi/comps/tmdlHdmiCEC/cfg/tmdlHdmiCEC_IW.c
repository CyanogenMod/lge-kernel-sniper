/*
 * Copyright (C) 2007 NXP N.V., All Rights Reserved.
 * This source code and any compilation or derivative thereof is the proprietary
 * information of NXP N.V. and is confidential in nature. Under no circumstances
 * is this software to be  exposed to or placed under an Open Source License of
 * any type without the expressed written permission of NXP N.V.
 *
 * \file          tmdlHdmiCEC_IW.c
 *
 * \version       $Revision: 1 $
 *
 * \date          $Date: 06/02/07 8:32 $
 *
 * \brief         devlib driver component API for the CEC Messages

 * \section refs  Reference Documents

 *
 * \section info  Change Information
 *
 * \verbatim

   $History: tmdlHdmiCEC_IW.c $
 *
   \endverbatim
 *
*/


/*============================================================================*/
/*                       INCLUDE FILES                                        */
/*============================================================================*/

#ifdef TMFL_OS_WINDOWS
#define _WIN32_WINNT 0x0500
#include "windows.h"
#else
#include "RTL.h"
#endif

#include "tmNxTypes.h"
#include "tmNxCompId.h"
#include "tmdlHdmiCEC_Types.h"
#include "tmdlHdmiCEC_cfg.h"
#include "tmdlHdmiCEC_IW.h"

#ifdef __cplusplus
extern "C" {
#endif

/*============================================================================*/
/*                       DEFINES                                              */
/*============================================================================*/

/* maximum number of tasks that can be handled by the wrapper */
#define MAX_TASKS  4
/* maximum number of message queues that can be handled by the wrapper */
#define MAX_QUEUES 4
/* maximum number of message queues that can be handled by the wrapper */
#define MAX_SEMA   4

/*============================================================================*/
/*                                MACRO                                       */
/*============================================================================*/

/* macro for quick error handling */
#define RETIF(cond, rslt) if ((cond)){return (rslt);}

/*============================================================================*/
/*                       TYPE DEFINITIONS                                     */
/*============================================================================*/

#ifdef TMFL_OS_WINDOWS
/* structure describing each task handled by the wrapper */
typedef struct
{
    Bool                   created;
    Bool                   started;
    UInt8                  priority;
    UInt16                 stackSize;
    DWORD                  threadID;
    HANDLE                 threadHandle;
    LPTHREAD_START_ROUTINE associatedThread;
    tmdlHdmiTxIWFuncPtr_t            associatedTask;
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

/*============================================================================*/
/*                       VARIABLES                                            */
/*============================================================================*/

/* table storing all tasks descriptions */
iwTcb_t taskTable[MAX_TASKS] =
{
    {False, False, 0, 0, 0, Null, ThreadProc0},
    {False, False, 0, 0, 0, Null, ThreadProc1},
    {False, False, 0, 0, 0, Null, ThreadProc2},
    {False, False, 0, 0, 0, Null, ThreadProc3}
};

/* table storing all message queues descriptions */
iwQueue_t queueTable[MAX_QUEUES] =
{
    {False, 0, 0, 0, 0, 0, 0, Null},
    {False, 0, 0, 0, 0, 0, 0, Null},
    {False, 0, 0, 0, 0, 0, 0, Null},
    {False, 0, 0, 0, 0, 0, 0, Null}
};

/*============================================================================*/
/*                       FUNCTION                                                */
/*============================================================================*/

/**
    \brief This function creates a task and allocates all the necessary resources. Note that creating a task do not start it automatically, an explicit call to IWTaskStart must be made.

    \param pFunc        Pointer to the function that will be executed in the task context.
    \param Priority     Priority of the task. The minimum priority is 0, the maximum is 255.
    \param StackSize    Size of the stack to allocate for this task.
    \param pHandle      Pointer to the handle buffer.

    \return The call result:
            - TM_OK: the call was successful
            - TMDL_ERR_DLHDMICEC_NO_RESOURCES: the resource is not available
            - TMDL_ERR_DLHDMICEC_INCONSISTENT_PARAMS: an input parameter is
              inconsistent

******************************************************************************/
tmErrorCode_t tmdlHdmiTxIWTaskCreate
(
    tmdlHdmiTxIWFuncPtr_t     pFunc,
    UInt8           priority,
    UInt16          stackSize,
    tmdlHdmiTxIWTaskHandle_t  *pHandle
)
{
    UInt32 i;

    /* check that input pointer is not NULL */
    RETIF(pFunc == Null, TMDL_ERR_DLHDMICEC_INCONSISTENT_PARAMS)

    /* check that input pointer is not NULL */
    RETIF(pHandle == Null, TMDL_ERR_DLHDMICEC_INCONSISTENT_PARAMS)

    /* search for available task slot */
    for(i = 0; i < MAX_TASKS; i++)
    {
        if (taskTable[i].created == False)
            break;

    }
    RETIF(i >= MAX_TASKS, TMDL_ERR_DLHDMICEC_NO_RESOURCES)

    /* store task parameters into the dedicated structure */
    taskTable[i].priority = priority;
    taskTable[i].stackSize = stackSize;
    taskTable[i].associatedTask = pFunc;
    taskTable[i].created = True;

    *pHandle = (tmdlHdmiTxIWTaskHandle_t)i;

    return(TM_OK);
}

/*============================================================================*/

/**
    \brief    This function destroys an existing task and frees resources used by it.

    \param Handle        Handle of the task to be destroyed, as returned by IWTaskCreate.

    \return The call result:
            - TM_OK: the call was successful
            - TMDL_ERR_DLHDMICEC_RESOURCE_NOT_OWNED: the caller does not own 
              the resource
            - TMDL_ERR_DLHDMICEC_BAD_HANDLE: the handle number is wrong

******************************************************************************/
tmErrorCode_t tmdlHdmiTxIWTaskDestroy
(
    tmdlHdmiTxIWTaskHandle_t handle
)
{
    /* check if handle number is in range */
    RETIF((handle < 0) || (handle >= MAX_TASKS), TMDL_ERR_DLHDMICEC_BAD_HANDLE)

    /* check if handle corresponding to task is created */
    RETIF(taskTable[handle].created == False, TMDL_ERR_DLHDMICEC_RESOURCE_NOT_OWNED)

    if (taskTable[handle].started == True)
    {
        TerminateThread(taskTable[handle].threadHandle, 0);
        taskTable[handle].started = False;
    }
    taskTable[handle].created = False;
    CloseHandle(taskTable[handle].threadHandle);

    return(TM_OK);
}

/*============================================================================*/

/**
    \brief This function start an existing task.

    \param Handle        Handle of the task to be started.

    \return The call result:
            - TM_OK: the call was successful
            - TMDL_ERR_DLHDMICEC_ALREADY_STARTED: the function is already started
            - TMDL_ERR_DLHDMICEC_NOT_STARTED: the function is not started
            - TMDL_ERR_DLHDMICEC_BAD_HANDLE: the handle number is wrong

******************************************************************************/
tmErrorCode_t tmdlHdmiTxIWTaskStart(tmdlHdmiTxIWTaskHandle_t handle)
{
    HANDLE threadHandle;

    /* check if handle number is in range */
    RETIF((handle < 0) || (handle >= MAX_TASKS), TMDL_ERR_DLHDMICEC_BAD_HANDLE)

    /* check if task is already started */
    RETIF(taskTable[handle].started == True, TMDL_ERR_DLHDMICEC_ALREADY_STARTED)

    /* start thread associated to the task */
    threadHandle = CreateThread(NULL,
                                (SIZE_T)taskTable[handle].stackSize,
                                taskTable[handle].associatedThread,
                                NULL,
                                0,
                                &(taskTable[handle].threadID));

    /* check return code for errors */
    RETIF(!threadHandle, TMDL_ERR_DLHDMICEC_NOT_STARTED)
    
    /* update task status */
    taskTable[handle].threadHandle = threadHandle;
    taskTable[handle].started = True;

    return(TM_OK);
}

/*============================================================================*/

/**
    \brief This function blocks the current task for the specified amount time. This is a passive wait.

    \param Duration    Duration of the task blocking in milliseconds.

    \return The call result:
            - TM_OK: the call was successful
            - TMDL_ERR_DLHDMICEC_NO_RESOURCES: the resource is not available

******************************************************************************/
tmErrorCode_t tmdlHdmiTxIWWait(UInt16 duration)
{
    HANDLE        timerHandle;
    LARGE_INTEGER time;

    timerHandle = CreateWaitableTimer(NULL, TRUE, NULL);
    RETIF(timerHandle == NULL, TMDL_ERR_DLHDMICEC_NO_RESOURCES)

    time.QuadPart = -10000 * (long)duration;
    SetWaitableTimer(timerHandle, &time, 0, NULL, NULL, FALSE);
    WaitForSingleObject(timerHandle, INFINITE);

    return(TM_OK);
}

/*============================================================================*/

/**
    \brief This function creates a message queue.

    \param QueueSize    Maximum number of messages in the message queue.
    \param pHandle        Pointer to the handle buffer.
    
    \return The call result:
            - TM_OK: the call was successful
            - TMDL_ERR_DLHDMICEC_INCONSISTENT_PARAMS: an input parameter is
              inconsistent
            - TMDL_ERR_DLHDMICEC_NO_RESOURCES: the resource is not available

******************************************************************************/
tmErrorCode_t tmdlHdmiTxIWQueueCreate(UInt8 queueSize, tmdlHdmiTxIWQueueHandle_t *pHandle)
{
    UInt32 i;

    /* check that input pointer is not NULL */
    RETIF(pHandle == Null, TMDL_ERR_DLHDMICEC_INCONSISTENT_PARAMS)

    /* search for available queue slot */
    for(i = 0; i < MAX_QUEUES; i++)
    {
        if (queueTable[i].created == False)
            break;

    }
    
    RETIF(i >= MAX_QUEUES, TMDL_ERR_DLHDMICEC_NO_RESOURCES)

    /* allocate memory for the queue */
    queueTable[i].queue = (UInt8 *)GlobalAlloc(GMEM_FIXED, queueSize);
    RETIF(queueTable[i].queue == NULL, TMDL_ERR_DLHDMICEC_NO_RESOURCES)

    /* allocate semaphores for the queue */
    queueTable[i].countSemaphore = CreateSemaphore(NULL, 0, queueSize, NULL);
    RETIF(queueTable[i].countSemaphore == NULL, TMDL_ERR_DLHDMICEC_NO_RESOURCES)

    queueTable[i].accessSemaphore = CreateSemaphore(NULL, 1, 1, NULL);
    RETIF(queueTable[i].accessSemaphore == NULL, TMDL_ERR_DLHDMICEC_NO_RESOURCES)

    /* update status of the queue table */
    queueTable[i].created = True;
    queueTable[i].queueSize = queueSize;
    *pHandle = (tmdlHdmiTxIWQueueHandle_t)i;

    return(TM_OK);
}

/*============================================================================*/

/**
    \brief This function destroys an existing message queue.

    \param    Handle        Handle of the queue to be destroyed.

    \return The call result:
            - TM_OK: the call was successful
            - TMDL_ERR_DLHDMICEC_BAD_HANDLE: the handle number is wrong
            - TMDL_ERR_DLHDMICEC_RESOURCE_NOT_OWNED: the caller does not own 
              the resource

******************************************************************************/
tmErrorCode_t tmdlHdmiTxIWQueueDestroy(tmdlHdmiTxIWQueueHandle_t handle)
{
    RETIF(handle > MAX_QUEUES, TMDL_ERR_DLHDMICEC_BAD_HANDLE)

    RETIF(queueTable[handle].created == False, TMDL_ERR_DLHDMICEC_RESOURCE_NOT_OWNED)

    GlobalFree((LPVOID)queueTable[handle].queue);
    CloseHandle(queueTable[handle].countSemaphore);
    CloseHandle(queueTable[handle].accessSemaphore);
    queueTable[handle].created = False;

    return(TM_OK);
}

/*============================================================================*/

/**
    \brief This function sends a message into the specified message queue.

    \param Handle    Handle of the queue that will receive the message.
    \param Message   Message to be sent.
    
    \return The call result:
            - TM_OK: the call was successful
            - TMDL_ERR_DLHDMICEC_BAD_HANDLE: the handle number is wrong
            - TMDL_ERR_DLHDMICEC_RESOURCE_NOT_OWNED: the caller does not own 
              the resource
            - TMDL_ERR_DLHDMICEC_FULL: the queue is full

******************************************************************************/
tmErrorCode_t tmdlHdmiTxIWQueueSend(tmdlHdmiTxIWQueueHandle_t handle, UInt8 message)
{
    tmErrorCode_t errorCode = TM_OK;

    /* check that handle is correct */
    RETIF(handle > MAX_QUEUES, TMDL_ERR_DLHDMICEC_BAD_HANDLE)

    RETIF(queueTable[handle].created != True, TMDL_ERR_DLHDMICEC_RESOURCE_NOT_OWNED)

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
        errorCode = TMDL_ERR_DLHDMICEC_FULL;
    }

    /* release access to this queue */
    ReleaseSemaphore(queueTable[handle].accessSemaphore, 1, NULL);

    return(TM_OK);
}
/*============================================================================*/

/**
    \brief This function reads a message from the specified message queue.

    \param    Handle        Handle of the queue from which to read the message.
    \param    pMessage      Pointer to the message buffer.

    \return The call result:
            - TM_OK: the call was successful
            - TMDL_ERR_DLHDMICEC_BAD_HANDLE: the handle number is wrong
            - TMDL_ERR_DLHDMICEC_RESOURCE_NOT_OWNED: the caller does not own 
              the resource
            - TMDL_ERR_DLHDMICEC_INCONSISTENT_PARAMS: an input parameter is
              inconsistent

******************************************************************************/
tmErrorCode_t tmdlHdmiTxIWQueueReceive(tmdlHdmiTxIWQueueHandle_t handle, UInt8 *pMessage)
{
    /* check that handle is correct */
    RETIF(handle > MAX_QUEUES, TMDL_ERR_DLHDMICEC_BAD_HANDLE)
    
    RETIF(queueTable[handle].created != True, TMDL_ERR_DLHDMICEC_RESOURCE_NOT_OWNED)

    /* check that input pointer is not NULL */
    RETIF(pMessage == Null, TMDL_ERR_DLHDMICEC_INCONSISTENT_PARAMS)

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

/*============================================================================*/

/**
    \brief This function creates a semaphore.

    \param     pHandle    Pointer to the handle buffer.

    \return The call result:
            - TM_OK: the call was successful
            - TMDL_ERR_DLHDMICEC_NO_RESOURCES: the resource is not available
            - TMDL_ERR_DLHDMICEC_INCONSISTENT_PARAMS: an input parameter is
              inconsistent

******************************************************************************/
tmErrorCode_t tmdlHdmiTxIWSemaphoreCreate(tmdlHdmiTxIWSemHandle_t *pHandle)
{
    /* check that input pointer is not NULL */
    RETIF(pHandle == Null, TMDL_ERR_DLHDMICEC_INCONSISTENT_PARAMS)

    *pHandle = (tmdlHdmiTxIWSemHandle_t)CreateSemaphore(NULL, 1, 1, NULL);

    RETIF((*pHandle) == NULL, TMDL_ERR_DLHDMICEC_NO_RESOURCES)

    return(TM_OK);
}

/*============================================================================*/

/**
    \brief This function destroys an existing semaphore.

    \param    Handle        Handle of the semaphore to be destroyed.

    \return The call result:
            - TM_OK: the call was successful
            - TMDL_ERR_DLHDMICEC_BAD_HANDLE: the handle number is wrong

******************************************************************************/
tmErrorCode_t tmdlHdmiTxIWSemaphoreDestroy(tmdlHdmiTxIWSemHandle_t handle)
{
    RETIF(CloseHandle(handle) == False, TMDL_ERR_DLHDMICEC_BAD_HANDLE)

    return(TM_OK);
}

/*============================================================================*/

/**
    \brief This function acquires the specified semaphore.

    \param Handle        Handle of the semaphore to be acquired.

    \return The call result:
            - TM_OK: the call was successful
            - TMDL_ERR_DLHDMICEC_BAD_HANDLE: the handle number is wrong

******************************************************************************/
tmErrorCode_t tmdlHdmiTxIWSemaphoreP(tmdlHdmiTxIWSemHandle_t handle)
{
    RETIF(WaitForSingleObject(handle, INFINITE) != WAIT_OBJECT_0, TMDL_ERR_DLHDMICEC_BAD_HANDLE)

    return(TM_OK);
}

/*============================================================================*/

/**
    \brief This function releases the specified semaphore.

    \param    Handle        Handle of the semaphore to be released.

    \return The call result:
            - TM_OK: the call was successful
            - TMDL_ERR_DLHDMICEC_BAD_HANDLE: the handle number is wrong

******************************************************************************/
tmErrorCode_t tmdlHdmiTxIWSemaphoreV(tmdlHdmiTxIWSemHandle_t handle)
{
    RETIF(ReleaseSemaphore(handle, 1, NULL) == 0, TMDL_ERR_DLHDMICEC_BAD_HANDLE)

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
#else

/*============================================================================*/
/*                       TYPE DEFINITIONS                                     */
/*============================================================================*/

/* structure describing each task handled by the wrapper */
typedef struct
{
    Bool                   created;
    Bool                   started;
    UInt8                  priority;
    UInt16                 stackSize;
    UInt                   threadHandle;
    tmdlHdmiTxIWFuncPtr_t            associatedTask;
} iwTcb_t;

/* structure describing each message queue handled by the wrapper */
#define Queue_default_Size 128

os_mbx_declare(mbox0, Queue_default_Size);
os_mbx_declare(mbox1, Queue_default_Size);
os_mbx_declare(mbox2, Queue_default_Size);
os_mbx_declare(mbox3, Queue_default_Size);


_declare_box (mpool0,sizeof(UInt8),Queue_default_Size);
_declare_box (mpool1,sizeof(UInt8),Queue_default_Size);
_declare_box (mpool2,sizeof(UInt8),Queue_default_Size);
_declare_box (mpool3,sizeof(UInt8),Queue_default_Size);



typedef struct
{
    Bool            created;
    UInt16          queueSize;
    void *          mbox;
    void *          mpool;
} iwQueue_t;

/* structure describing each task handled by the wrapper */
typedef struct
{
    Bool                    created;
    OS_SEM                  handle;
} iwSem_t;
/*============================================================================*/
/*                       FUNCTION PROTOTYPES                                  */
/*============================================================================*/

/*============================================================================*/
/*                       VARIABLES                                            */
/*============================================================================*/

/* table storing all tasks descriptions */
iwTcb_t taskTable[MAX_TASKS] =
{
    {False, False, 0, 0, 0, Null},
    {False, False, 0, 0, 0, Null},
    {False, False, 0, 0, 0, Null},
    {False, False, 0, 0, 0, Null}
};

/* table storing all message queues descriptions */
iwQueue_t queueTable[MAX_QUEUES] =
{
    {False, 0, mbox0, mpool0},
    {False, 0, mbox1, mpool1},
    {False, 0, mbox2, mpool2},
    {False, 0, mbox3, mpool3}
};


/* table storing all message queues descriptions */
iwSem_t semTable[MAX_SEMA] =
{
    {False, 0},
    {False, 0},
    {False, 0},
    {False, 0},
};
/*============================================================================*/
/*                       FUNCTION                                                */
/*============================================================================*/

/**
    \brief This function creates a task and allocates all the necessary resources. Note that creating a task do not start it automatically, an explicit call to IWTaskStart must be made.
Parameters:

    \param pSWVersion    Pointer to the version structure
    \param pFunc        Pointer to the function that will be executed in the task context.
    \param Priority        Priority of the task. The minimum priority is 0, the maximum is 255.
    \param StackSize    Size of the stack to allocate for this task.
    \param pHandle        Pointer to the handle buffer.

    \return The call result:
            - TM_OK        If call is successful.
            - TM_NOK    If call failed.

******************************************************************************/
tmErrorCode_t tmdlHdmiTxIWTaskCreate(tmdlHdmiTxIWFuncPtr_t     pFunc,
                                UInt8           Priority,
                                UInt16          StackSize,
                                tmdlHdmiTxIWTaskHandle_t *pHandle)
{
    UInt32 i;

    /* search for available task slot */
    for(i = 0; i < MAX_TASKS; i++)
    {
        if (taskTable[i].created == False)
            break;

    }
    if(i >= MAX_TASKS) return(TM_ERR_NO_RESOURCES);

    /* store task parameters into the dedicated structure */
    taskTable[i].priority = Priority;
    taskTable[i].stackSize = StackSize;
    taskTable[i].associatedTask = pFunc;
    taskTable[i].created = True;

    *pHandle = (tmdlHdmiTxIWTaskHandle_t)i;

    return(TM_OK);
}

/*============================================================================*/

/**
    \brief    This function destroys an existing task and frees resources used by it.

    \param Handle        Handle of the task to be destroyed, as returned by TxIWTaskCreate.

    \return The call result:
            - TM_OK    If call is successful.
            - ~TM_OK    If call failed.

******************************************************************************/
tmErrorCode_t tmdlHdmiTxIWTaskDestroy(tmdlHdmiTxIWTaskHandle_t Handle)
{
    if (taskTable[Handle].started == True)
    {
        if (os_tsk_delete(taskTable[Handle].threadHandle) == OS_R_OK)
        {
            taskTable[Handle].started = False;
        }
        else
        {
            return ~TM_OK;
        }
    }
    taskTable[Handle].created = False;

    return(TM_OK);
}

/*============================================================================*/

/**
    \brief This function start an existing task.

    \param Handle        Handle of the task to be started.

    \return The call result:
            - TM_OK    If call is successful.
            - ~TM_OK    If call failed.

******************************************************************************/
tmErrorCode_t tmdlHdmiTxIWTaskStart(tmdlHdmiTxIWTaskHandle_t Handle)
{
    UInt threadHandle;

    /* check if task is already started */
    if (taskTable[Handle].started == True)
        return(TM_ERR_ALREADY_STARTED);

    /* start thread associated to the task */
    threadHandle = os_tsk_create(taskTable[Handle].associatedTask,
                                 taskTable[Handle].priority);

    /* check return code for errors */
    if (!threadHandle )
    {
        return(TM_ERR_NOT_STARTED);
    }

    /* update task status */
    taskTable[Handle].threadHandle = threadHandle;
    taskTable[Handle].started = True;

    return(TM_OK);
}

/*============================================================================*/

/**
    \brief This function blocks the current task for the specified amount time. This is a passive wait.

    \param Duration    Duration of the task blocking in milliseconds.

    \return The call result:
            - TM_OK    If call is successful.
            - ~TM_OK    If call failed.

******************************************************************************/
tmErrorCode_t tmdlHdmiTxIWWait(UInt16 Duration)
{
    //TODO perform conversion with Tick system in order to consider ms
    os_dly_wait(Duration/10);

    return(TM_OK);
}

/*============================================================================*/

/**
    \brief This function creates a message queue.

    \param QueueSize    Maximum number of messages in the message queue.
    \param pHandle        Pointer to the handle buffer.
    \return The call result:
            - TM_OK    If call is successful.
            - ~TM_OK    If call failed.

******************************************************************************/
tmErrorCode_t tmdlHdmiTxIWQueueCreate(UInt8 QueueSize, tmdlHdmiTxIWQueueHandle_t *pHandle)
{
    UInt8 i;

    if (QueueSize > Queue_default_Size) return TM_ERR_BAD_PARAMETER;


    /* search for available queue slot */
    for(i = 0; i < MAX_QUEUES; i++)
    {
        if (queueTable[i].created == False)
            break;

    }
    if(i >= MAX_QUEUES) return(TM_ERR_NO_RESOURCES);

    *pHandle = i;

    /* Initialisation of queue object */
    //TODO check if we can reuse deleted queue
    _init_box (queueTable[i].mpool, sizeof(queueTable[i].mpool),sizeof(UInt8));
    os_mbx_init(queueTable[i].mbox, sizeof(queueTable[i].mbox));

    /* update status of the queue table */
    queueTable[i].created = True;
    queueTable[i].queueSize = QueueSize;

    return(TM_OK);
}

/*============================================================================*/

/**
    \brief This function destroys an existing message queue.

    \param    Handle        Handle of the queue to be destroyed.

    \return The call result:
            - TM_OK    If call is successful.
            - ~TM_OK    If call failed.

******************************************************************************/
tmErrorCode_t tmdlHdmiTxIWQueueDestroy(tmdlHdmiTxIWQueueHandle_t Handle)
{
    void *msg;

    RETIF(Handle > MAX_QUEUES, TM_ERR_BAD_HANDLE);
    RETIF(queueTable[Handle].created == False, TM_ERR_BAD_HANDLE);

    while(os_mbx_check(queueTable[Handle].mbox) != queueTable[Handle].queueSize)
    {
        RETIF(os_mbx_wait(queueTable[Handle].mbox, &msg, 10) == OS_R_TMO, TM_ERR_TIMEOUT);
    }

    queueTable[Handle].created = False;

    return(TM_OK);
}

/*============================================================================*/

/**
    \brief This function sends a message into the specified message queue.

    \param Handle    Handle of the queue that will receive the message.
                    Message    Message to be sent.
    \return The call result:
            - TM_OK    If call is successful.
            - ~TM_OK    If call failed.

******************************************************************************/
tmErrorCode_t tmdlHdmiTxIWQueueSend(tmdlHdmiTxIWQueueHandle_t Handle, UInt8 Message)
{
    UInt8 * msg;

    /* check that handle is correct */
    RETIF(Handle > MAX_QUEUES, TM_ERR_BAD_HANDLE);
    RETIF(queueTable[Handle].created != True, TM_ERR_BAD_HANDLE);

    msg = _alloc_box (queueTable[Handle].mpool);
    msg = (UInt8 *) Message;

    if(os_mbx_check(queueTable[Handle].mbox) != 0)
    {
        RETIF(os_mbx_send(queueTable[Handle].mbox, msg, 0xffff) == OS_R_TMO, TM_ERR_TIMEOUT);
    }
    else
    {
        return TM_ERR_FULL;
    }

    return(TM_OK);
}
/*============================================================================*/

/**
    \brief This function reads a message from the specified message queue.

    \param    Handle        Handle of the queue from which to read the message.
            pMessage    Pointer to the message buffer.

    \return The call result:
            - TM_OK    If call is successful.
            - ~TM_OK    If call failed.

******************************************************************************/
tmErrorCode_t tmdlHdmiTxIWQueueReceive(tmdlHdmiTxIWQueueHandle_t Handle, UInt8 *pMessage)
{
    UInt8 * msg;
    /* check that handle is correct */
    RETIF(Handle > MAX_QUEUES, TM_ERR_BAD_HANDLE);
    RETIF(queueTable[Handle].created != True, TM_ERR_BAD_HANDLE);

    /* if we reach this point, this means that we got a message */
    /* ask for exclusive access to this queue */
    RETIF(os_mbx_wait(queueTable[Handle].mbox , (void **) &msg, 0xffff) == OS_R_TMO, TM_ERR_TIMEOUT);
    *pMessage = (UInt) msg;
    _free_box (queueTable[Handle].mpool, msg);

    return(TM_OK);
}

/*============================================================================*/

/**
    \brief This function creates a semaphore.

    \param     pHandle    Pointer to the handle buffer.

    \return The call result:
            - TM_OK    If call is successful.
            - ~TM_OK    If call failed.
******************************************************************************/
tmErrorCode_t tmdlHdmiTxIWSemaphoreCreate(tmdlHdmiTxIWSemHandle_t *pHandle)
{
    UInt8 i;

    /* search for available queue slot */
    for(i = 0; i < MAX_SEMA; i++)
    {
        if (semTable[i].created == False)
            break;

    }
    if(i >= MAX_SEMA) return(TM_ERR_NO_RESOURCES);



    os_sem_init(semTable[i].handle, 1);

    *pHandle = (tmdlHdmiTxIWSemHandle_t) i;

    semTable[i].created = True;

    return(TM_OK);
}

/*============================================================================*/

/**
    \brief This function destroys an existing semaphore.

    \param    Handle        Handle of the semaphore to be destroyed.

    \return The call result:
            - TM_OK     If call is successful.
            - ~TM_OK If call failed.
******************************************************************************/
tmErrorCode_t tmdlHdmiTxIWSemaphoreDestroy(tmdlHdmiTxIWSemHandle_t Handle)
{
    //TODO check if we can reuse

    return(TM_OK);
}

/*============================================================================*/

/**
    \brief This function acquires the specified semaphore.

    \param Handle        Handle of the semaphore to be acquired.

    \return The call result:
            - TM_OK    If call is successful.
            - ~TM_OK    If call failed.

******************************************************************************/
tmErrorCode_t tmdlHdmiTxIWSemaphoreP(tmdlHdmiTxIWSemHandle_t Handle)
{
    if (semTable[Handle].created==False) return TM_ERR_BAD_HANDLE;

    RETIF(os_sem_wait(semTable[Handle].handle, 0xffff) == OS_R_TMO, TM_ERR_TIMEOUT);

    return(TM_OK);
}

/*============================================================================*/

/**
    \brief This function releases the specified semaphore.

    \param    Handle        Handle of the semaphore to be released.

    \return The call result:
            - TM_OK    If call is successful.
            - ~TM_OK    If call failed.

******************************************************************************/
tmErrorCode_t tmdlHdmiTxIWSemaphoreV(tmdlHdmiTxIWSemHandle_t Handle)
{
    if (semTable[Handle].created==False) return TM_ERR_BAD_HANDLE;

    RETIF(os_sem_send(semTable[Handle].handle) == OS_R_NOK, TM_ERR_BAD_HANDLE);

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
}

/******************************************************************************
    \brief  This function enables the interrupts for a specific device.

    \param

    \return The call result:
            - TM_OK: the call was successful

******************************************************************************/
void tmdlHdmiTxIWEnableInterrupts(tmdlHdmiIWDeviceInterrupt_t device)
{
}

#endif

#ifdef __cplusplus
extern "C" {
#endif

/*============================================================================*/
/*                            END OF FILE                                     */
/*============================================================================*/

