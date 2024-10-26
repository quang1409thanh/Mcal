/*-------------------------------- Arctic Core ------------------------------
 * Copyright (C) 2013, ArcCore AB, Sweden, www.arccore.com.
 * Contact: <contact@arccore.com>
 * 
 * You may ONLY use this file:
 * 1)if you have a valid commercial ArcCore license and then in accordance with  
 * the terms contained in the written license agreement between you and ArcCore, 
 * or alternatively
 * 2)if you follow the terms found in GNU General Public License version 2 as 
 * published by the Free Software Foundation and appearing in the file 
 * LICENSE.GPL included in the packaging of this file or here 
 * <http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt>
 *-------------------------------- Arctic Core -----------------------------*/

#include "os_i.h"


#define COUNTER_STD_END 	\
        goto ok;		\
    err:				\
        ERRORHOOK(rv);	\
    ok:					\
        return rv // Expecting calling function to provide the ending semicolon



#define IsCounterValid(_counterId)   ((CounterType)(_counterId) < (CounterType)OS_COUNTER_CNT)

/**
 *
 * @param counter_id
 * @return
 */

/** @req OS530 IncrementCounter() available in all Scalability Classes. */
/** @req OS399 */
StatusType IncrementCounter( CounterType counter_id ) {
    StatusType rv = E_OK;
    OsCounterType *cPtr;
    imask_t flags;
#if (OS_SC3==STD_ON) || (OS_SC4==STD_ON)
    OsTaskVarType *currPcbPtr;
#endif

    OS_VALIDATE( (Os_SysIntAnyDisabled() == FALSE) , E_OS_DISABLEDINT );   /* @req 4.1.2/SWS_Os_00093 */

    /** @req OS285 Return E_OS_ID if the counter id is not valid */
    OS_VALIDATE(IsCounterValid(counter_id), E_OS_ID);
    cPtr = Os_CounterGet(counter_id);

    /** @req OS285 Return E_OS_ID if the counter is a HW counter */
    OS_VALIDATE( COUNTER_TYPE_SOFT == cPtr->type, E_OS_ID );

#if (OS_SC3==STD_ON) || (OS_SC4==STD_ON)
    currPcbPtr = Os_SysTaskGetCurr();

    if( currPcbPtr->constPtr->applOwnerId != cPtr->applOwnerId ) {
        /* @req SWS_Os_00056 */
        APPL_CHECK_STATE(cPtr->applOwnerId);
        APPL_CHECK_ACCESS(currPcbPtr->constPtr->applOwnerId, cPtr->accessingApplMask);


#if (OS_NUM_CORES > 1)
        OS_EXT_VALIDATE( Os_ApplGetCore(cPtr->applOwnerId) != GetCoreID(), E_OS_ACCESS );
#endif
    }
#endif

    Irq_Save(flags);

    /** @req OS286 */
    cPtr->val = Os_CounterAdd( cPtr->val, Os_CounterGetMaxValue(cPtr), 1 );

#if OS_ALARM_CNT!=0
    Os_AlarmCheck(cPtr);
#endif
#if OS_SCHTBL_CNT!=0
    Os_SchTblCheck(cPtr);
#endif

    Irq_Restore(flags);

    /** @req OS321 */
    OS_STD_END_1(OSServiceId_IncrementCounter,counter_id);
}


/** @req OS532 GetCounterValue() available in all Scalability Classes. */
/** @req OS383 */
StatusType GetCounterValue( CounterType counter_id , TickRefType tick_ref)
{
    StatusType rv = E_OK;
    OsCounterType *cPtr;
#if (OS_SC3==STD_ON) || (OS_SC4==STD_ON)
    OsTaskVarType *currPcbPtr;
#endif

    OS_VALIDATE( (Os_SysIntAnyDisabled() == FALSE) , E_OS_DISABLEDINT );   /* @req 4.1.2/SWS_Os_00093 */
    OS_VALIDATE(IsCounterValid(counter_id),E_OS_ID);    /* @req 4.1.2/SWS_Os_00376 *//** @req OS376 */

    cPtr = Os_CounterGet(counter_id);


#if (OS_SC3==STD_ON) || (OS_SC4==STD_ON)
    currPcbPtr = Os_SysTaskGetCurr();

    if( currPcbPtr->constPtr->applOwnerId != cPtr->applOwnerId ) {
        /* @req SWS_Os_00056 */
        APPL_CHECK_STATE(cPtr->applOwnerId);
        APPL_CHECK_ACCESS(currPcbPtr->constPtr->applOwnerId, cPtr->accessingApplMask);


#if	(OS_NUM_CORES > 1)
        if (Os_ApplGetCore(cPtr->applOwnerId) != GetCoreID()) {
            StatusType status = Os_NotifyCore(Os_ApplGetCore(cPtr->applOwnerId),
                                              OSServiceId_GetCounterValue,
                                              counter_id,
                                              (uint32_t)tick_ref,
                                              0);
            return status;
        }
#endif
    }
#endif

    /** @req OS377 */
    if( cPtr->type == COUNTER_TYPE_HARD ) {
        if( cPtr->driver == NULL ) {
            /* It's OSINTERNAL */
            *tick_ref = OS_SYS_PTR->tick;
        } else {
#if 0
        /** !req OS531 */ /* Return the real time value */
        /* We support only GPT for now */
        *tick_ref  = (TickType)Gpt_GetTimeElapsed(cPtr->driver.OsGptChannelRef);
#endif

        }
    } else {
        *tick_ref = cPtr->val;
    }

    OS_STD_END_2(OSServiceId_GetCounterValue,counter_id,tick_ref);
}

/**
 *
 * @param counter_id		The counter to be read
 * @param val[in,out]		in,  The previously read tick value of the counter
 * 							out, Contains the current tick value of the counter.
 * @param elapsed_val[out]  The difference
 * @return
 */

/** @req 4.1.2/SWS_Os_00392 @req OS392 OS provides the service GetElapsedValue() */
/** @req OS534 GetElapsedValue() available in all Scalability Classes. */
StatusType GetElapsedValue ( CounterType counter_id, TickRefType val, TickRefType elapsed_val)
{
    StatusType rv = E_OK;
    OsCounterType *cPtr;
    TickType currTick = 0;
    TickType max;

    OS_VALIDATE( (Os_SysIntAnyDisabled() == FALSE) , E_OS_DISABLEDINT );   /* @req 4.1.2/SWS_Os_00093 */

    /** @req SWS_Os_00381 */ /** @req OS381 */
    OS_VALIDATE(IsCounterValid(counter_id),E_OS_ID);

    cPtr = Os_CounterGet(counter_id);
    max = Os_CounterGetMaxValue(cPtr);

    /** @req SWS_Os_00391 */ /** @req OS391 */
    OS_VALIDATE( *val <= max,E_OS_VALUE );

#if	(OS_APPLICATION_CNT > 1) && (OS_NUM_CORES > 1)
    if (Os_ApplGetCore(cPtr->applOwnerId) != GetCoreID()) {
        StatusType status = Os_NotifyCore(Os_ApplGetCore(cPtr->applOwnerId),
                                          OSServiceId_GetElapsedValue,
                                          counter_id,
                                          (int32_t)val,
                                          (int32_t)elapsed_val);
        return status;
    }
#endif

    (void)GetCounterValue(counter_id,&currTick);

    /** @req OS382 */
    *elapsed_val = Os_CounterDiff(currTick,*val,max);

    /** @req OS460 */
    *val = currTick;

    OS_STD_END_3(OSServiceId_GetElapsedValue, counter_id, val,elapsed_val );
}

/*
 * The OsTick():
 * 1. The Decrementer is setup by Os_SysTickStart(period_ticks)
 * 2. Os_SysTickInit() setup INTC[7] to trigger OsTick
 * 3. OsTick() then increment counter os_tick_counter if used
 */

/*
 * Non-Autosar stuff
 */

#if 0
/* NOTE: Fix this once the whole compiler / multicore combo gets figured out */

/* The id of the counter driven by the os tick, or -1 if not used.
 * Using weak linking to set default value -1 if not set by config.
 */
#if defined(__DCC__)
#pragma weak Os_Arc_OsTickCounter
#endif
#if defined(__ICCHCS12__)
extern CounterType Os_Arc_OsTickCounter;
#else
CounterType Os_Arc_OsTickCounter __attribute__((weak)) = -1;
#endif
#endif

extern const CounterType Os_Arc_OsTickCounter[];

ISR(OsTick) {
    // if not used, os_tick_counter < 0
    if (Os_Arc_OsTickCounter[GetCoreID()] >= 0) {

        OsCounterType *cPtr = Os_CounterGet(Os_Arc_OsTickCounter[GetCoreID()]);
#if defined(CFG_KERNEL_EXTRA)
        OsTaskVarType *pcbPtr;
        imask_t flags;
#endif

        OS_SYS_PTR->tick++;

        cPtr->val = Os_CounterAdd( cPtr->val, Os_CounterGetMaxValue(cPtr), 1 );

#if defined(CFG_KERNEL_EXTRA)
        Irq_Save(flags);
        /* Check tasks in the timer queue (here from Sleep() or WaitSemaphore() ) */
        TAILQ_FOREACH(pcbPtr, &OS_SYS_PTR->timerHead, timerEntry ) {
            --pcbPtr->timerDec;
            if( pcbPtr->timerDec <= 0 ) {
                /* Remove from the timer queue */
                TAILQ_REMOVE(&OS_SYS_PTR->timerHead, pcbPtr, timerEntry);
                /* ... and add to the ready queue */
                Os_TaskMakeReady(pcbPtr);
            }
        }
        Irq_Restore(flags);
#endif
#if OS_ALARM_CNT!=0
        Os_AlarmCheck(cPtr);
#endif
#if OS_SCHTBL_CNT!=0
        Os_SchTblCheck(cPtr);
#endif
    }
}

TickType GetOsTick( void ) {
    return OS_SYS_PTR->tick;
}


/**
 * Initialize alarms and schedule-tables for the counters
 */
void Os_CounterInit( void ) {
#if OS_ALARM_CNT!=0
    {
        OsCounterType *cPtr;
        OsAlarmType *aPtr;

        /* Add the alarms to counters */
        for (AlarmType i = 0; i < OS_ALARM_CNT; i++) {
            aPtr = Os_AlarmGet(i);
            cPtr = aPtr->counter;
            SLIST_INSERT_HEAD(&cPtr->alarm_head, aPtr, alarm_list);
        }
    }
#endif

#if OS_SCHTBL_CNT!=0
    {
        OsCounterType *cPtr;
        OsSchTblType *sPtr;

        /* Add the schedule tables to counters */
        for(ScheduleTableType i = 0; i < OS_SCHTBL_CNT; i++ ) {

            sPtr = Os_SchTblGet(i);
            cPtr = sPtr->counter;
            SLIST_INSERT_HEAD(&cPtr->sched_head, sPtr, sched_list);
        }
    }
#endif
}

