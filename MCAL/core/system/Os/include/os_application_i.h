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

#ifndef APPLICATION_H_
#define APPLICATION_H_

/*
 * IMPLEMENTATION NOTES:
 *
 *  OS448:  The  Operating  System  module  shall  prevent  access  of
 *  OS-Applications, trusted or non-trusted, to objects not belonging
 *  to this OS-Application, except access rights for such objects are
 *  explicitly granted by configuration.
 *
 *  OS509:  If  a  service  call  is  made  on  an  Operating  System
 *  object  that  is  owned  by another OS-Application without state
 *  APPLICATION_ACCESSIBLE, then the Operating System module shall return E_OS_ACCESS.
 *
 *  OS056: If an OS-object identifier is the parameter of an Operating System module�s
 *  system service, and no sufficient access rights have been assigned to this OS-object
 *  at  configuration  time  (Parameter  Os[...]AccessingApplication)  to  the  calling
 *  Task/Category  2  ISR,  the  Operating  System  module�s  system  service  shall  return
 *  E_OS_ACCESS.
 *
 *  OS311: If OsScalabilityClass is SC3 or SC4 AND a Task OR Category 2 ISR OR
 *  Resources OR Counters OR Alarms OR Schedule tables does not belong to exactly
 *  one OS-Application the consistency check shall issue an error.
 *
 *  Page 52:
 *  It is assumed that the Operating System module itself is trusted.
 *
 *  Sooo, that gives us:
 *  1. For each
 *
 *
 *  1. App1, NT
 *     Task11
 *  2. App2, NT
 *     Task21
 *  3. App3, T
 *     Task31
 *  4. App4, T
 *     Task41
 *
 *  * App2->App1: ActivateTask(Task11)
 *    This is OK as long as Task11 have granted access to it during configuration
 *  * App4->App3: ActivateTask(Task31)
 *    This is OK as long as Task31 have granted access to it during configuration
 *  * App1->App4: ActivateTask(Task41)
 *    It's not really clear if the OS automagically exports all services..
 *    But this could also be CallTrustedFunction(ServiceId_AcivateTask,???)
 *
 */


#define APPL_ID_TO_MASK(_x)   (1U<<(_x))

#define	Os_ApplGetState(_applId) 			Os_AppVar[_applId].state
#define Os_ApplSetState(_applId, _state)	Os_AppVar[_applId].state = _state;


typedef struct OsAppHooks {
    void (*startup)( void );
    void (*shutdown)( Std_ReturnType Error );
    void (*error)( Std_ReturnType Error );
} OsAppHooksType;


typedef struct OsAppVar {
    _Bool trusted;					/* 0 - Non-trusted application
                                             * 1 - Trusted application */
    ApplicationStateType state;		/* The current state of the application */
} OsAppVarType;

/* NON standard type.
 * Used for ROM based parameters....
 */
typedef struct OsApplication {
    uint32 	appId;			/* The ID of the application */
    const char 	*name;		/* Name of the application */
    _Bool	trusted;		/* Trusted or not, 0 - Non-trusted */ /** @req OS446 */
    uint8_t core;			/* The core number on which this application runs */

    /* hooks, the names are StartupHook_<name>(), etc. */
    void (*StartupHook)( void );   /** @req OS543 Available in SC3 and SC4 (and other SCs (OS240)) */
    void (*ShutdownHook)( StatusType Error ); /** @req OS545 Available in SC3 and SC4 (and other SCs (OS240)) */
    void (*ErrorHook)( StatusType Error ); /** @req OS544 Available in SC3 and SC4 (and other SCs (OS240)) */

    int 	restartTaskId;
} OsAppConstType;

#if ( OS_SC1 == STD_ON ) || ( OS_SC4 == STD_ON )
typedef void ( * trusted_func_t)( TrustedFunctionIndexType , TrustedFunctionParameterRefType );
#endif

#if OS_APPLICATION_CNT!=0
extern OsAppVarType Os_AppVar[OS_APPLICATION_CNT];

static inline OsAppVarType *Os_ApplGet(ApplicationType id) {
    return &Os_AppVar[id];
}


extern const OsAppConstType Os_AppConst[OS_APPLICATION_CNT];

static inline const OsAppConstType *Os_ApplGetConst(ApplicationType id) {
    return &Os_AppConst[id];
}
#endif

extern GEN_APPLICATION_HEAD;


static inline uint8_t Os_ApplGetCore( ApplicationType appl )
{
    return Os_AppConst[appl].core;
}

#define OS_APP_CALL_ERRORHOOKS( x ) \
    for(uint32 i=0;i<OS_APPLICATION_CNT;i++) { \
        if( Os_AppConst[i].ErrorHook != NULL ) { \
            Os_AppConst[i].ErrorHook(x); \
        } \
    }

/**
 *
 * @param mask  Target accessing application mask
 * @return
 */
static inline StatusType Os_ApplHaveAccess( uint32_t mask ) {

    /* @req OS056 */
    if( (APPL_ID_TO_MASK(OS_SYS_PTR->currApplId) & mask) == 0 ) {
        return E_OS_ACCESS;
    }

    /* @req OS504/ActivateTask
     * The Operating System module shall deny access to Operating System
     * objects from other OS-Applications to an OS-Application which is not in state
     * APPLICATION_ACCESSIBLE.
     * */

    /* We are activating a task in another application */
    if( !( Os_ApplGetState(OS_SYS_PTR->currApplId) == APPLICATION_ACCESSIBLE)) {
        return E_OS_ACCESS;
    }

    return E_OK;
}

void Os_ApplStart( void );


#endif /* APPLICATION_H_ */
