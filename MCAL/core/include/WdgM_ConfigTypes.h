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


#ifndef WDGM_TYPES_H
#define WDGM_TYPES_H

#include "Std_Types.h"
/** @req WDGM025 */
#include "WdgM_Cfg.h"
#include "WdgIf.h"
#if defined(USE_DEM)
#include "Dem.h"
#endif
#include "Os.h"


typedef uint8 WdgM_Substate;
#define WDGM_SUBSTATE_INCORRECT 			0x00
#define WDGM_SUBSTATE_CORRECT				0x01


#define WDGM_LOCAL_STATUS_OK			0x00
#define WDGM_LOCAL_STATUS_FAILED		0x01
#define WDGM_LOCAL_STATUS_EXPIRED		0x02
#define WDGM_LOCAL_STATUS_DEACTIVATED	0x04

#define WDGM_GLOBAL_STATUS_OK			0x00
#define WDGM_GLOBAL_STATUS_FAILED		0x01
#define WDGM_GLOBAL_STATUS_EXPIRED		0x02
#define WDGM_GLOBAL_STATUS_STOPPED		0x03
#define WDGM_GLOBAL_STATUS_DEACTIVATED	0x04


typedef struct
{
    const uint16			TriggerConditionValue;
    const WdgIf_ModeType    WatchdogMode;		/** @req WDGM181 */
    const uint8			    WatchdogId;
} WdgM_Trigger;


typedef struct
{
    const uint16 		CheckpointId;
    const uint16		ExpectedAliveIndications;
    const uint8			MinMargin;
    const uint8			MaxMargin;
    const uint16		SupervisionReferenceCycle;
} WdgM_AliveSupervision;

typedef struct
{
    const uint16 			CheckpointIdStart;
    const uint16 			CheckpointIdFinish;
    const uint32			DeadlineMin;
    const uint32			DeadlineMax;
} WdgM_DeadlineSupervision;

typedef struct
{
    const uint16			CheckpointIdSource;
    const uint16			CheckpointIdDestination;
} WdgM_InternalTransition;

typedef struct
{
    const uint16					Id;

    const uint16					*CheckpointIds;
    const uint16					Length_CheckpointIds; /* only 65535 possible */

    const WdgM_InternalTransition	*Transitions;
    const uint16					Length_Transitions; /* only 65535 possible */

    const uint16					*StartCheckpointIds;
    const uint16					Length_StartCheckpointIds; /* only 65535 possible */

    const uint16					*FinalCheckpointIds;
    const uint16					Length_FinalCheckpointIds; /* only 65535 possible */

    const boolean					isOsApplicationRefSet;
    const ApplicationType			OsApplicationRef;
} WdgM_SupervisedEntity;


typedef struct
{
    const uint16							SupervisedEntityId;		/** @req WDGM282 */

    const boolean							CheckInternalLogic; 	/** @req WDGM212.partially */ /* is true when either internal logic or external logic shall be checked (only internal is activated when this flag is true and Length_ExternalLogicalSupervisions == 0) */

    const WdgM_AliveSupervision				*AliveSupervisions;
    const uint16							Length_AliveSupervisions; /* only 65535 possible */

    const WdgM_DeadlineSupervision			*DeadlineSupervisions;
    const uint16							Length_DeadlineSupervisions; /* only 65535 possible */

    const uint8								FailedAliveSupervisionReferenceCycleTol;
} WdgM_SupervisedEntityConfiguration;

typedef struct
{
    const uint8									Id;
    const uint16								ExpiredSupervisionCycleTol;
    const uint32								SupervisionCycle;

    const WdgM_SupervisedEntityConfiguration    *SEConfigurations;		/** @req WDGM283 */ /* only activated SEs are configured */
    const uint16								Length_SEConfigurations; /* only 65535 possible */

    const WdgM_Trigger							*Triggers;		/** @req WDGM178 */
    const uint8									Length_Triggers; /* only 255 possible */

} WdgM_Mode;

typedef struct
{
    const uint16		*allowedCallerIds;
    const uint8			Length_allowedCallerIds; /* only 255 possible */
} WdgM_CallerIds;

typedef struct
{
    const uint8		WatchdogId;
    const uint8		WatchdogDeviceId;
} WdgM_Watchdog;

#if defined(USE_DEM)
typedef struct
{
    const Dem_EventIdType		ImproperCaller;
    const Dem_EventIdType		Monitoring;
    const Dem_EventIdType		SetMode;
} WdgM_DEMEventIdRefs;
#endif

typedef struct
{
    const WdgM_CallerIds		CallerIds;

    const WdgM_SupervisedEntity *SupervisedEntities;
    const uint16				Length_SupervisedEntities; /* only 65535 possible */

    const WdgM_Watchdog			*Watchdogs;
    const uint8					Length_Watchdogs; /* only 255 possible */
} WdgM_General;

typedef struct
{
#if defined(USE_DEM)
    const WdgM_DEMEventIdRefs		    DemEventIdRefs;
#endif
    const uint8							initialModeId;
    const WdgM_Mode						*Modes;
    const uint8							Length_Modes; /* only 255 possible */
} WdgM_ConfigSet;

typedef struct
{
    const WdgM_General	    General;
    const WdgM_ConfigSet    ConfigSet;
} WdgM_ConfigType;

extern const WdgM_ConfigType WdgMConfig;

#endif
