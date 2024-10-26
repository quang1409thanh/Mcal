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



// Structs and types used internal in the module

#ifndef _ECUM_INTERNALS_H_
#define _ECUM_INTERNALS_H_

#include "EcuM_Generated_Types.h"

#if  ( ECUM_DEV_ERROR_DETECT == STD_ON )
#if defined(USE_DET)
#include "Det.h"
#endif
#define VALIDATE(_exp,_api,_err ) \
        if(!(_exp) ) { \
          (void)Det_ReportError(MODULE_ID_ECUM, 0, _api, _err); \
        }
/* Lint Exceptions:- Unpermitted operand to operator '!' (9027) */
/*lint -emacro(9027,VALIDATE_RV)*/
/*lint -emacro(904,VALIDATE_RV)*/
#define VALIDATE_RV(_exp,_api,_err,_rv ) \
        if(!(_exp)) { \
          (void)Det_ReportError(MODULE_ID_ECUM, 0, _api, _err); \
          return _rv; \
        }

#define VALIDATE_NO_RV(_exp,_api,_err) \
        if(!(_exp)){ \
          (void)Det_ReportError(MODULE_ID_ECUM, 0, _api, _err); \
          return; \
        }

// MISRA 2012 14.4 : Inhibit lint error 9036 for macro ended with while(0) condition
//lint -emacro(9036,DET_REPORT_ERROR)
#define DET_REPORT_ERROR(_api, _err)                   \
        (void)Det_ReportError(MODULE_ID_ECUM, 0, _api, _err); \


#else
#define VALIDATE(_exp,_api,_err )
#define VALIDATE_RV(_exp,_api,_err,_rv )
#define VALIDATE_NO_RV(_exp,_api,_err )
#define DET_REPORT_ERROR(_api, _error)
#endif

#define SchM_Enter_EcuM(_area) SchM_Enter_EcuM_##_area)
#define SchM_Exit_EcuM(_area) SchM_Exit_EcuM_##_area)


typedef struct
{
    boolean                   initiated;
    EcuM_ConfigType *         config;
    EcuM_StateType            shutdown_target;
#if (defined(USE_ECUM_FLEXIBLE))
    EcuM_ShutdownCauseType    shutdown_cause;
#endif
    uint8                     sleep_mode;
    AppModeType               app_mode;
    EcuM_StateType            current_state;
#if defined(USE_COMM) || (defined(USE_ECUM_COMM) && (ECUM_AR_VERSION < 40000))
    uint32 				run_comm_requests;
#endif
    uint32 				run_requests;
    uint32 				postrun_requests;
    /* Events set by EcuM_SetWakeupEvent */
    uint32 wakeupEvents;

    uint32 wakeupTimer;
    uint32 validationTimer;
    uint32 nvmReadAllTimer;
    /* Events set by EcuM_ValidateWakeupEvent */
    uint32 validEvents;
    boolean killAllRequest;
} EcuM_GlobalType;

extern EcuM_GlobalType EcuM_World;
typedef enum
{
    ALL,
    ALL_WO_LIN,
    ONLY_LIN
} EcuM_ComMCommunicationGroupsType;


void EcuM_enter_run_mode(void);

void SetCurrentState(EcuM_StateType state);

/* @req EcuM2905 */ /* @req EcuMf2905 */
/* @req EcuM2906 */ /* @req EcuMf2906 */
/* @req EcuM2907 */ /* @req EcuMf2907 */
/* @req EcuMf2908 */
/* @req EcuMf2909 */

//#if defined(USE_LDEBUG_PRINTF)
// MISRA 2004 6.3, 2012 4.6 : Inhibit lint error 970 for function GetMainStateAsString
// to allow usage of type char, as this function uses string constants.
char *GetMainStateAsString( EcuM_StateType state ); //lint !e970
//#endif


#define DEBUG_ECUM_STATE(_state)						LDEBUG_PRINTF("STATE: %s\n",GetMainStateAsString(_state))
#define DEBUG_ECUM_CALLOUT(_call)    					LDEBUG_FPUTS( "  CALLOUT->: " _call "\n");
#define DEBUG_ECUM_CALLOUT_W_ARG(_call,_farg0,_arg0)    LDEBUG_PRINTF("  CALLOUT->: " _call " "_farg0 "\n",_arg0)
#define DEBUG_ECUM_CALLIN_W_ARG(_call,_farg0,_arg0)     LDEBUG_PRINTF("  <-CALLIN : " _call " "_farg0 "\n",_arg0)


#endif /*_ECUM_INTERNALS_H_*/
