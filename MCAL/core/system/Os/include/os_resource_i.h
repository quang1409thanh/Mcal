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

#ifndef RESOURCE_I_H_
#define RESOURCE_I_H_

/**
 * STD container: OsResource
 * OsResourceProperty:				1    INTERNAL, LINKED, STANDARD
 * OsResourceAccessingApplication:  0..*
 * OsResourceLinkedResourceRef:     0..1
 */

typedef enum {
    /* External resource */
    RESOURCE_TYPE_STANDARD,//!< RESOURCE_TYPE_STANDARD
    /* ?? */
    RESOURCE_TYPE_LINKED,  //!< RESOURCE_TYPE_LINKED
    /* Internal resource */
    RESOURCE_TYPE_INTERNAL //!< RESOURCE_TYPE_INTERNAL
} OsResourcePropertyType;

typedef struct OsResource {
    char id[16];

    int 	nr;					/* The running number, starting at RES_SCHEDULER=0 */
    uint32 	ceiling_priority;	/* The calculated ceiling prio */
    uint32 	old_task_prio;		/* Stored prio of the owner oi the resource */
    TaskType owner;				/* Owner of the resource */

    OsResourcePropertyType type;

#if	(OS_USE_APPLICATIONS == STD_ON)
    ApplicationType applOwnerId;		/* Application that owns this task */
    uint32			accessingApplMask;	/* Applications that may access task
                                         * when state is APPLICATION_ACCESSIBLE */
#endif
    TAILQ_ENTRY(OsResource) listEntry;	/* List of resources for each task. */

} OsResourceType;


typedef enum {
    LOCK_TYPE_RESOURCE,
    LOCK_TYPE_INTERRUPT
} OsLocktypeType;

typedef struct OsLockingtime {
    OsLocktypeType type;
    union {
        struct {
            ResourceType id;
            uint64 time;
        } resource;

        struct {
            uint64 all;
            uint64 os;
        } interrupt;
    } u;
} OsLockingtimeType;

typedef struct OsTimingProtection {
    uint64 executionBudget;			/* worst case execution budget in ns */
    uint64 timeFrame;				/* the frame in ns that timelimit may execute in. */
    uint64 timeLimit;				/* time in ns that the task/isr may with a timeframe */
    OsLockingtimeType *lockingTime;	/* ROM, resource/interrupt locktimes */
} OsTimingProtectionType;

#if OS_RESOURCE_CNT!=0
extern GEN_RESOURCE_HEAD;
#endif

static inline OsResourceType *Os_ResourceGet( ResourceType resource ) {
#if OS_RESOURCE_CNT!=0
    return &resource_list[resource];
#else
    (void)resource;
    return NULL;
#endif
}

static inline ApplicationType Os_ResourceGetApplicationOwner( ResourceType id ) {
    /** @req OS274 */
    ApplicationType rv = INVALID_OSAPPLICATION;
#if (OS_RESOURCE_CNT!=0)
    if( id < OS_RESOURCE_CNT ) {
        rv = Os_ResourceGet(id)->applOwnerId;
    }
#else
    (void)id;
#endif
    return rv;
}

extern struct OsResource resScheduler;

static inline _Bool Os_SchedulerResourceIsFree( void ) {
    return (OS_SYS_PTR->resScheduler->owner == NO_TASK_OWNER );
}

void Os_ResourceGetInternal(void );
void Os_ResourceReleaseInternal( void );
void Os_ResourceInit( void );



#endif /* RESOURCE_I_H_ */
