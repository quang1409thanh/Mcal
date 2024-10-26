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

/* ----------------------------[includes]------------------------------------*/

#include "Os.h"
#include "timer.h"
#include "arc.h"
#if defined(CFG_SHELL)
#include "shell.h"
#endif

#include <stdio.h>
#include <string.h>

//#define USE_LDEBUG_PRINTF
#include "debug.h"

/* ----------------------------[private define]------------------------------*/
#define MAX_FUNCTION_CNT    16uL
#define MAX_NAME_CHAR_LEN   32uL
/* ----------------------------[private macro]-------------------------------*/
/* ----------------------------[private typedef]-----------------------------*/

typedef struct Perf_Info_S {
    /* Calculated load, 0% to 100% */
    uint8_t load;
    /* Number of times the function have been called */
    uint32_t invokedCnt;

    /* Max time in us that this task/isr have executed */
    TickType timeMax_us;
    TickType timeMin_us;
    /* The total time in us this task/isr have executed */
    TickType timeTotal_us;

    char name[MAX_NAME_CHAR_LEN];
//	char *name;

    /*
     * INTERNAL
     */

    TickType timePeriodTotal_us;

    /* For each entry 1 is added. For each exit 1 is subtracted
     * Normally this should be 0 or 1 */
    int8_t called;

    TickType timeStart;
} Perf_InfoType;

/* ----------------------------[private function prototypes]-----------------*/

#if defined(CFG_SHELL)
static int shellCmdTop(int argc, char *argv[] );
#endif

/* ----------------------------[private variables]---------------------------*/

Perf_InfoType Perf_TaskTimers[OS_TASK_CNT];
Perf_InfoType Perf_IsrTimers[OS_ISR_MAX_CNT];
Perf_InfoType Perf_FunctionTimers[MAX_FUNCTION_CNT];
Perf_InfoType Perf_KernelTimers;

uint32 nesting = 0;
static boolean initCalled = false;

#if defined(CFG_SHELL)
static ShellCmdT topCmdInfo = {
    shellCmdTop,
    0,1,
    "top",
    "top",
    "List CPU load\n",
    {	NULL,NULL}
};
#endif

/* ----------------------------[private functions]---------------------------*/

#if defined(CFG_SHELL)
/**
 *
 * @param argc
 * @param argv
 * @return
 */
static int shellCmdTop(int argc, char *argv[] ) {
    Arc_PcbType pcb;

    if(argc == 1 ) {
        puts("Task name         %load    invCnt max[us] \n");
        for (int i = 0; i < OS_TASK_CNT; i++) {
            printf("  %-16s %3d %8d %8d\n",Perf_TaskTimers[i].name,
                    Perf_TaskTimers[i].load,
                    Perf_TaskTimers[i].invokedCnt,
                    Perf_TaskTimers[i].timeMax_us );
        }

        puts("\n");
        puts("ISR name          %load    invCnt max[us] \n");

        for (int i = 0; i < Os_Arc_GetIsrCount(); i++) {

            if( Perf_IsrTimers[i].name[0] == '\0' ) {
                Os_Arc_GetIsrInfo(&pcb,i);
                memcpy(Perf_IsrTimers[i].name,pcb.name,MAX_NAME_CHAR_LEN);
            }
            printf("  %-16s %3d %8d %8d\n",Perf_IsrTimers[i].name,
                    Perf_IsrTimers[i].load,
                    Perf_IsrTimers[i].invokedCnt,
                    Perf_IsrTimers[i].timeMax_us );

        }

        puts("\nKernel\n");
        printf( "  %%load  : %-3d\n"
                "  max[us]: %-8d\n\n",
                Perf_KernelTimers.load,
                Perf_KernelTimers.timeMax_us);

    } else {
    }

    return 0;
}
#endif

/* ----------------------------[public functions]----------------------------*/

/**
 *
 */
void Perf_Init(void) {
    Arc_PcbType pcb;
uint8 k = 0;
    if (initCalled) {
        return;
    }
#if defined(CFG_SHELL)
    SHELL_AddCmd(&topCmdInfo);
#endif
    Timer_Init();
    initCalled = true;

    for (int i = 0; i < OS_TASK_CNT; i++) {
        Os_Arc_GetTaskInfo(&pcb,i);
        //Perf_TaskTimers[i].name = pcb.name;
        for (k = 0; k < 16; k++) {
            Perf_TaskTimers[i].name[k] = pcb.name[k];
        }

    }

    for (int i = 0; i < Os_Arc_GetIsrCount(); i++) {
        Os_Arc_GetIsrInfo(&pcb,i);
//		Perf_IsrTimers[i].name = pcb.name;
        for (k = 0; k < 16; k++) {
            Perf_IsrTimers[i].name[k] = pcb.name[k];
        }
    }
}

/**
 *
 */
void Perf_Trigger(void) {
    TickType perfDiff_us;
    static TickType perfTimerLast = 0;

    if (perfTimerLast == 0) {
        perfTimerLast = Timer_GetTicks();
        return;
    }

    perfDiff_us = TIMER_TICK2US(Timer_GetTicks() - perfTimerLast);
    perfTimerLast = Timer_GetTicks();

    for (int i = 0; i < OS_TASK_CNT; i++) {
        LDEBUG_PRINTF("%2u load:%d\n", (unsigned)i, Perf_TaskTimers[i].timePeriodTotal_us * 100 / PERIOD_IN_US );
        /* clear period times */
        Perf_TaskTimers[i].load = Perf_TaskTimers[i].timePeriodTotal_us * 100 / perfDiff_us;
        Perf_TaskTimers[i].timePeriodTotal_us = 0;
    }

    for (int i = 0; i < Os_Arc_GetIsrCount(); i++) {
        LDEBUG_PRINTF("%2u load:%d\n", (unsigned)i, Perf_IsrTimers[i].timePeriodTotal_us * 100 / PERIOD_IN_US );
        /* clear period times */
        Perf_IsrTimers[i].load = Perf_IsrTimers[i].timePeriodTotal_us * 100 / perfDiff_us;
        Perf_IsrTimers[i].timePeriodTotal_us = 0;
    }

    for (int i = 0; i < MAX_FUNCTION_CNT; i++) {
        LDEBUG_PRINTF("%2u load:%d\n", (unsigned)i, Perf_FunctionTimers[i].timePeriodTotal_us * 100 / PERIOD_IN_US );
        /* clear period times */
        Perf_FunctionTimers[i].load = Perf_FunctionTimers[i].timePeriodTotal_us * 100 / perfDiff_us;
        Perf_FunctionTimers[i].timePeriodTotal_us = 0;
    }

    /* Kernel */
    Perf_KernelTimers.load = Perf_KernelTimers.timePeriodTotal_us * 100 / perfDiff_us;
    Perf_KernelTimers.timePeriodTotal_us = 0;

}

#if !defined(CFG_T1_ENABLE)
/**
 *
 * @param isr
 */
void Os_PreIsrHook(ISRType isr) {
    Perf_IsrTimers[isr].invokedCnt++;
    Perf_IsrTimers[isr].called++;
    Perf_IsrTimers[isr].timeStart = Timer_GetTicks();

}

/**
 *
 * @param isr
 */
void Os_PostIsrHook(ISRType isr) {
    TickType diff;

    diff = TIMER_TICK2US(Timer_GetTicks() - Perf_IsrTimers[isr].timeStart);
    if (diff > Perf_IsrTimers[isr].timeMax_us) {
        Perf_IsrTimers[isr].timeMax_us = diff;
    }
    Perf_IsrTimers[isr].timeTotal_us += diff;
    Perf_IsrTimers[isr].timePeriodTotal_us += diff;
    Perf_IsrTimers[isr].called--;
}
#else
#warning Perf.c cannot trace the interrupts when T1 is used.
#endif
/**
 *
 */
void PreTaskHook(void) {
    TaskType task;
    TickType diff;

    GetTaskID(&task);

    /* Kernel */
    if (Perf_KernelTimers.timeStart != 0) {
        diff = TIMER_TICK2US(Timer_GetTicks() - Perf_KernelTimers.timeStart);
        if (diff > Perf_KernelTimers.timeMax_us) {
            Perf_KernelTimers.timeMax_us = diff;
        }
        Perf_KernelTimers.timeTotal_us += diff;
        Perf_KernelTimers.timePeriodTotal_us += diff;
    }

    /* Task */
    Perf_TaskTimers[task].invokedCnt++;
    Perf_TaskTimers[task].called++;
    Perf_TaskTimers[task].timeStart = Timer_GetTicks();

}

/**
 *
 */
void PostTaskHook(void) {
    TaskType task;
    TickType diff;

    GetTaskID(&task);
    Perf_TaskTimers[task].called--;
    diff = TIMER_TICK2US(Timer_GetTicks() - Perf_TaskTimers[task].timeStart);
    if (diff > Perf_TaskTimers[task].timeMax_us) {
        Perf_TaskTimers[task].timeMax_us = diff;
    }
    Perf_TaskTimers[task].timeTotal_us += diff;
    Perf_TaskTimers[task].timePeriodTotal_us += diff;

    /* Kernel time */
    Perf_KernelTimers.timeStart = Timer_GetTicks();
}

/**
 * Optional API, add a name to the Perf_InfoType for usability.
 * @param PerfFuncIdx
 * @param PerfNamePtr
 * @param PerfNameLen
 */
void Perf_InstallFunctionName(uint8 PerfFuncIdx, char *PerfNamePtr, uint8 PerfNameLen) {

    if((PerfNameLen < MAX_NAME_CHAR_LEN) &&
            (PerfFuncIdx < MAX_FUNCTION_CNT))
    {
        for (uint8 i = 0; i < PerfNameLen; i++) {
            Perf_FunctionTimers[PerfFuncIdx].name[i] = PerfNamePtr[i];
        }
    }
}

/**
 * Call before the functions for measurement is called.
 * @param PerfFuncIdx
 */
void Perf_PreFunctionHook(uint8 PerfFuncIdx) {

    if (PerfFuncIdx < MAX_FUNCTION_CNT) {
        Perf_FunctionTimers[PerfFuncIdx].invokedCnt++;
        Perf_FunctionTimers[PerfFuncIdx].called++;
        Perf_FunctionTimers[PerfFuncIdx].timeStart = Timer_GetTicks();
    }
}

/**
 * Call after the function for measurement has been executed.
 * @param PerfFuncIdx
 */
void Perf_PostFunctionHook(uint8 PerfFuncIdx) {
    TickType diff;

    if (PerfFuncIdx < MAX_FUNCTION_CNT) {

        diff = TIMER_TICK2US(Timer_GetTicks() - Perf_FunctionTimers[PerfFuncIdx].timeStart);
        if (diff > Perf_FunctionTimers[PerfFuncIdx].timeMax_us) {
            Perf_FunctionTimers[PerfFuncIdx].timeMax_us = diff;
        }
        Perf_FunctionTimers[PerfFuncIdx].timeTotal_us += diff;
        Perf_FunctionTimers[PerfFuncIdx].timePeriodTotal_us += diff;
        Perf_FunctionTimers[PerfFuncIdx].called--;
    }
}
