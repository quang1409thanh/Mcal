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

/* Globally fulfilled requirements */
/** @req COMM863 */
/** @req COMM051 */
/** @req COMM845 */
/** @req COMM846 */
/** @req COMM881 */
/** @req COMM880 */
/** @req COMM686 */
/** @req COMM51.partially */
/** @req COMM191 */
/** @req COMM301 */
/** @req COMM488 */
/** @req COMM599.partially */
/** @req COMM459 */
/** @req COMM462 */
/** @req COMM549.bswbuilder */
/** @req COMM464 */
/** @req ComM910 *//* Pnc exists only if ComMPncSupport is enabled */
/** @req ComM909 *//* One Pnc SM per PNC supported*/
/** @req ComM920 *//* 48 PNC SM supported */
/** @req ComM994 *//* BusNm config does not restrict ComM user assignment to PNC */
/** @req ComM996 *//* Redundant configuration of a User mapping to PNC and Channel is not allowed */
/** @req ComM912 */ /* Configurable No. of ComMUsers can be mapped to one or more Pncs */
/* @req ComM981 */ /* IF ComMPncGatewayEnabled is True, COMM_GATEWAY_TYPE_ACTIVE can be a default setting. Both ERA and EIRA are present */
/* @req ComM919 */ /* More than one Com signal can be assigned to a PNC */
/* @req ComM964 *//* Active gateway handled according to SM */

//lint -emacro(904,COMM_VALIDATE_PARAMETER_NORV,COMM_VALIDATE_INIT_NORV,COMM_VALIDATE_INIT,COMM_VALIDATE_PARAMETER) //904 PC-Lint exception to MISRA 14.7 (validate DET macros).

#include <string.h>
#include "ComM.h" /** @req COMM463.partially */
#include "ComM_Dcm.h" /** @req COMM463.partially */
#include "ComM_EcuM.h"
#if (COMM_DEV_ERROR_DETECT == STD_ON)
#include "Det.h"
#endif

#if defined(USE_DEM)
#include "Dem.h"
#endif

#include "ComM_BusSM.h" /** @req COMM463.partially */
#include "ComM_Internal.h"

/** !req COMM506.partially *//* Com.h, SchM_ComM.h, NvM.h etc. are missing */
#if defined(USE_CANSM)
#include "CanSM.h"
#endif

#if defined(USE_LINSM)
#include "LinSM.h"
#endif
#if defined(USE_ETHSM)
#include "EthSM.h" 
#endif
#if defined(USE_FRSM)
#include "FrSM.h"
#endif
#if defined(USE_NM)
#include "Nm.h"
#endif

#if defined(USE_BSWM)
#include "BswM.h"
#endif

#if defined(USE_DCM)
#include "Dcm_Cbk.h"
#endif

#include "SchM_ComM.h"
static ComM_InternalType ComM_Internal = {
        .InitStatus = COMM_UNINIT,
        .InhibitCounter = 0,
#if (COMM_NO_COM == STD_ON)
        .NoCommunication = TRUE,
#else
        .NoCommunication = FALSE,
#endif
#if (COMM_PNC_SUPPORT == STD_ON)
        .newEIRARxSignal = FALSE,
#endif
};

static const ComM_ConfigType * ComM_ConfigPtr;
#if (COMM_PNC_SUPPORT == STD_ON)
#define PNC_FIRST_CHNL_REF 0u
static boolean ComM_PncStateTrans_ThisCycle;
#endif /* (COMM_PNC_SUPPORT == STD_ON) */

// ----------------------------------------------------------------------------
// Internal functions
// ----------------------------------------------------------------------------
#if (COMM_PNC_SUPPORT == STD_ON)
static void ComM_Internal_Monitor_RdySlpToPrpSlpTrns(void);
#if (COMM_PNC_GATEWAY_ENABLED == STD_ON)
static void ComM_Internal_Monitor_GwPassiveChnl(void);
static boolean externalPncRelease(uint8 pncCount,uint8 byteNo,uint8 bitNo);
static boolean externalPncRequest(uint8 pncCount,uint8 channelIndex);
#endif
#endif
/**
 * @brief Check FullCom Min duration
 * @param ChannelConf
 * @param ChannelInternal
 * @return
 */
static inline boolean ComM_Internal_FullComMinTime_AllowsExit(const ComM_ChannelType* ChannelConf, const ComM_Internal_ChannelType* ChannelInternal) {
    boolean rv;
    if ((ChannelConf->NmVariant == COMM_NM_VARIANT_LIGHT) ||
        (ChannelConf->NmVariant == COMM_NM_VARIANT_NONE)){
        rv = (ChannelInternal->FullComMinDurationTimeLeft == 0);
    } else {
        rv = TRUE;
    }
    return rv;
}


/** @req COMM073  @req COMM071
 *  @req COMM069  @req COMM402 */
/**
 * @brief Propagates channel mode to BusSM
 * @param ChannelConf
 * @param ComMode
 * @return
 */
static Std_ReturnType ComM_Internal_PropagateComMode( const ComM_ChannelType* ChannelConf, ComM_ModeType ComMode ){

    Std_ReturnType busSMStatus = E_OK;
    switch (ChannelConf->BusType) {
#if defined(USE_CANSM)
        case COMM_BUS_TYPE_CAN:
            busSMStatus = CanSM_RequestComMode(ChannelConf->ComMChannelId, ComMode); /** @req COMM854 */
            break;
#endif
#if defined(USE_LINSM)
        case COMM_BUS_TYPE_LIN:
            busSMStatus = LinSM_RequestComMode(ChannelConf->ComMChannelId, ComMode); /** @req COMM856 */
            break;
#endif
#if defined(USE_ETHSM)
        case COMM_BUS_TYPE_ETH:
            busSMStatus = EthSM_RequestComMode(ChannelConf->ComMChannelId, ComMode); /** @req COMM859 */
            break;
#endif
#if defined (USE_FRSM)
        case COMM_BUS_TYPE_FR:
            busSMStatus = FrSM_RequestComMode(ChannelConf->ComMChannelId, ComMode); /** @req ComM852 */
            break;
#endif
        default:
            busSMStatus = E_NOT_OK;
            break;
    }

    if (E_OK == busSMStatus)
    {
        ComM_Internal.Channels[ChannelConf->ComMChannelId].requestPending = TRUE;
    }
    return busSMStatus;
}


/** @req COMM602  @req COMM261 */
/**
 * @brief Requests/releases Nm network according to channel mode
 * @param ChannelConf
 * @return
 */
static Std_ReturnType ComM_Internal_NotifyNm( const ComM_ChannelType* ChannelConf){
    ComM_Internal_ChannelType* ChannelInternal = &ComM_Internal.Channels[ChannelConf->ComMChannelId];

    Std_ReturnType status = E_OK;

    if (ChannelConf->NmVariant == COMM_NM_VARIANT_FULL)
    {
#if defined(USE_NM)
        if (ChannelInternal->Mode == COMM_FULL_COMMUNICATION)
        {
            if (ChannelInternal->SubMode == COMM_FULL_COM_NETWORK_REQUESTED)
            {
                if (TRUE == ChannelInternal->EcuMWkUpIndication) {

                    status = Nm_PassiveStartUp(ChannelConf->NmChannelHandle); /* @req COMM665 */
                    ChannelInternal->EcuMWkUpIndication = FALSE;
                }
#if (COMM_PNC_SUPPORT == STD_ON)
#if (COMM_PNC_GATEWAY_ENABLED == STD_OFF)

                /* UserRequestMask is checked before calling the Nm_NetworkRequest(), UserRequestMask request ensures
                 * network request is made by the user/application */
                else if (ChannelInternal->UserRequestMask != 0) {

                    status = Nm_NetworkRequest(ChannelConf->NmChannelHandle);  /** @req COMM869 */ /** @req COMM870 */
                } else {
                    /* To avoid compiler message */
                }
#else
                else {
                    status = Nm_NetworkRequest(ChannelConf->NmChannelHandle);
                }
#endif
#else
                /* UserRequestMask is checked before calling the Nm_NetworkRequest(), UserRequestMask request ensures
                 * network request is made by the user/application */

                else if (ChannelInternal->UserRequestMask != 0) {
                    status = Nm_NetworkRequest(ChannelConf->NmChannelHandle);
                } else {
                    /* To avoid compiler message */
                }
#endif
            }
            else if (ChannelInternal->SubMode == COMM_FULL_COM_READY_SLEEP)
            {
                status = Nm_NetworkRelease(ChannelConf->NmChannelHandle);  /**< @req COMM133 */
            }else{
                /* Do nothing */
            }
        }
#else
        status = E_NOT_OK;
#endif
    }

    if ((ChannelConf->NmVariant == COMM_NM_VARIANT_NONE) || (ChannelConf->NmVariant == COMM_NM_VARIANT_LIGHT))
    {
        if (ChannelInternal->Mode == COMM_FULL_COMMUNICATION)
        {
            if (ChannelInternal->SubMode == COMM_FULL_COM_NETWORK_REQUESTED)
            {
                /* start timer */ /* cancelling timer by restarting it */
                ChannelInternal->FullComMinDurationTimeLeft = COMM_T_MIN_FULL_COM_MODE_DURATION; /** @req COMM886 */ /** @req COMM887 */
            }
            else if (ChannelInternal->SubMode == COMM_FULL_COM_READY_SLEEP)
            {
                ChannelInternal->LightTimeoutTimeLeft = ChannelConf->LightTimeout; /** @req COMM891 */
            }else{
                /* Do nothing */
            }
        }
    }

    return status;
}


/**
 * @brief Enter No Com
 * @param ChannelConf
 * @param ChannelInternal
 * @return
 */
static inline Std_ReturnType ComM_Internal_Enter_NoCom(const ComM_ChannelType* ChannelConf,
                                                    ComM_Internal_ChannelType* ChannelInternal) {
    ChannelInternal->RequestedSubMode = COMM_NO_COM_NO_PENDING_REQUEST;
    return ComM_Internal_PropagateComMode(ChannelConf, COMM_NO_COMMUNICATION);
}


/**
 * @brief Enter Silent Com
 * @param ChannelConf
 * @param ChannelInternal
 * @return
 */
static inline Std_ReturnType ComM_Internal_Enter_SilentCom(const ComM_ChannelType* ChannelConf,
                                                        ComM_Internal_ChannelType* ChannelInternal) {
    ChannelInternal->RequestedSubMode = COMM_SILENT_COM;
    return ComM_Internal_PropagateComMode(ChannelConf, COMM_SILENT_COMMUNICATION);
}

/**
 * @brief Enter Network requested phase
 * @param ChannelConf
 * @param ChannelInternal
 * @return
 */
static inline Std_ReturnType ComM_Internal_Enter_NetworkRequested(const ComM_ChannelType* ChannelConf,
                                                                ComM_Internal_ChannelType* ChannelInternal) {
    Std_ReturnType status = E_OK;

    if (ChannelInternal->SubMode == COMM_NO_COM_NO_PENDING_REQUEST)
    {
        ChannelInternal->SubMode = COMM_NO_COM_REQUEST_PENDING; /** @req COMM875 */ /** @req COMM894 */ /** @req COMM876 */
    }

    if (ChannelInternal->CommunicationAllowed == TRUE)
    {
        /* @req COMM895 */
        /* !req COMM896 *//* Or do we only end up here in COMM_NO_COM_REQUEST_PENDING. */
        status = ComM_Internal_PropagateComMode(ChannelConf, COMM_FULL_COMMUNICATION);
        if(status == E_OK) {
            ChannelInternal->RequestedSubMode = COMM_FULL_COM_NETWORK_REQUESTED;
        }
    }

    return status;
}

/**
 * @brief Enter ready sleep phase
 * @param ChannelConf
 * @param ChannelInternal
 * @return
 */
static inline Std_ReturnType ComM_Internal_Enter_ReadySleep(const ComM_ChannelType* ChannelConf,
                                                            ComM_Internal_ChannelType* ChannelInternal) {
    Std_ReturnType status = E_OK;

    if (ChannelInternal->Mode != COMM_FULL_COMMUNICATION)
    {
        ChannelInternal->RequestedSubMode = COMM_FULL_COM_READY_SLEEP;
        status = ComM_Internal_PropagateComMode(ChannelConf, COMM_FULL_COMMUNICATION);
    }
    else
    {
        ChannelInternal->SubMode = COMM_FULL_COM_READY_SLEEP;
        status = ComM_Internal_NotifyNm(ChannelConf);
    }

    return status;
}

/**
 * @brief Transition from No Com
 * @param ChannelConf
 * @param ChannelInternal
 * @param isRequest
 * @return
 */
static inline Std_ReturnType ComM_Internal_UpdateFromNoCom(const ComM_ChannelType* ChannelConf,
                    ComM_Internal_ChannelType* ChannelInternal, boolean isRequest) {
    Std_ReturnType status = E_OK;
    if ((ChannelInternal->NmIndicationMask & COMM_NM_INDICATION_RESTART) == COMM_NM_INDICATION_RESTART ) {

        /* restart indication  */
        status = ComM_Internal_Enter_NetworkRequested(ChannelConf, ChannelInternal);  /**< @req COMM583 */
        ChannelInternal->NmIndicationMask &= ~(COMM_NM_INDICATION_RESTART);
    } else if (TRUE == ChannelInternal->EcuMWkUpIndication) {
        /* EcuM wake up indication */
        status = ComM_Internal_Enter_NetworkRequested(ChannelConf, ChannelInternal);

    } else {
        if ((((ChannelInternal->InhibitionStatus & COMM_INHIBITION_STATUS_NO_COMMUNICATION) == COMM_INHIBITION_STATUS_NO_COMMUNICATION) ||
            ((ChannelInternal->InhibitionStatus & COMM_INHIBITION_STATUS_WAKE_UP) == COMM_INHIBITION_STATUS_WAKE_UP) ||
            (ComM_Internal.NoCommunication == TRUE)) && (ChannelInternal->DCM_Requested == FALSE)) { /** @req COMM182 */
            // Inhibition is active
            /** @req COMM302  @req COMM218  @req COMM219  @req COMM215.3 */
            if (TRUE == isRequest) {
                /* @req COMM142 */
                /* IMPROVEMENT: Should only increment if it is a request for
                 * COMM_FULL_COMMUNICATION.
                 * And we should return COMM_E_MODE_LIMITATION. */
                if (ComM_Internal.InhibitCounter < 65535) {
                    ComM_Internal.InhibitCounter++;
                }
            }
        } else {
            if ((ChannelInternal->UserRequestMask != 0) || (ChannelInternal->DCM_Requested == TRUE)) {
                // Channel is requested
                status = ComM_Internal_Enter_NetworkRequested(ChannelConf, ChannelInternal);
            } else {
                // Channel is not requested
                ChannelInternal->SubMode = COMM_NO_COM_NO_PENDING_REQUEST; /** @req COMM897 */
            }
        }
    }
    return status;
}

/**
 * @brief Transition from Silent Com
 * @param ChannelConf
 * @param ChannelInternal
 * @param isRequest
 * @return
 */
static inline Std_ReturnType ComM_Internal_UpdateFromSilentCom(const ComM_ChannelType* ChannelConf,
                    ComM_Internal_ChannelType* ChannelInternal, boolean isRequest) {
    Std_ReturnType status = E_OK;
    if ((ChannelInternal->NmIndicationMask & COMM_NM_INDICATION_RESTART) == COMM_NM_INDICATION_RESTART ) {
        // "restart" indication
        status = ComM_Internal_Enter_ReadySleep(ChannelConf, ChannelInternal);  /**< @req COMM296.1 */
        ChannelInternal->NmIndicationMask &= ~(COMM_NM_INDICATION_RESTART);
    } else if ((ChannelInternal->NmIndicationMask & COMM_NM_INDICATION_BUS_SLEEP)==COMM_NM_INDICATION_BUS_SLEEP) {
        // "bus sleep" indication
        status = ComM_Internal_Enter_NoCom(ChannelConf, ChannelInternal);  /**< @req COMM295 */
        ChannelInternal->NmIndicationMask &= ~(COMM_NM_INDICATION_BUS_SLEEP);
    } else if ((ChannelInternal->NmIndicationMask & COMM_NM_INDICATION_NETWORK_MODE)==COMM_NM_INDICATION_NETWORK_MODE) {
        // "network mode" indication
        status = ComM_Internal_Enter_ReadySleep(ChannelConf, ChannelInternal);  /**< @req COMM296.2 */
        ChannelInternal->NmIndicationMask &= ~(COMM_NM_INDICATION_NETWORK_MODE);
    } else {
        if ((((ChannelInternal->InhibitionStatus & COMM_INHIBITION_STATUS_NO_COMMUNICATION)==COMM_INHIBITION_STATUS_NO_COMMUNICATION) ||
            (ComM_Internal.NoCommunication == TRUE)) && (ChannelInternal->DCM_Requested == FALSE)) { /** @req COMM182 */
            // Inhibition is active
            /** @req COMM215.2 */
            if (TRUE == isRequest) {
                /* @req COMM142 */
                /* IMPROVEMENT: Should only increment if it is a request for
                 * COMM_FULL_COMMUNICATION.
                 * And we should return COMM_E_MODE_LIMITATION. */
                if (ComM_Internal.InhibitCounter < 65535) {
                    ComM_Internal.InhibitCounter++;
                }
            }
        } else {
            if ((ChannelInternal->UserRequestMask != 0) || (ChannelInternal->DCM_Requested == TRUE)) { /** @req COMM878 *//** @req COMM877 */
                // Channel is requested
                status = ComM_Internal_Enter_NetworkRequested(ChannelConf, ChannelInternal);
            } else {
                // Stay in SILENT
            }
        }
    }
    return status;
}



/**
 * @brief Transition from Full Com
 * @param ChannelConf
 * @param ChannelInternal
 * @param isRequest
 * @return
 */
static inline Std_ReturnType ComM_Internal_UpdateFromFullCom(const ComM_ChannelType* ChannelConf,
                    ComM_Internal_ChannelType* ChannelInternal, boolean isRequest) {
    Std_ReturnType status = E_OK;
    if ((ChannelInternal->NmIndicationMask & COMM_NM_INDICATION_BUS_SLEEP)==COMM_NM_INDICATION_BUS_SLEEP) {
        // "bus sleep" indication
        status = ComM_Internal_Enter_NoCom(ChannelConf, ChannelInternal);  /**< @req COMM637 */
        ChannelInternal->NmIndicationMask &= ~(COMM_NM_INDICATION_BUS_SLEEP);
    } else if (((ChannelInternal->NmIndicationMask & COMM_NM_INDICATION_PREPARE_BUS_SLEEP)==COMM_NM_INDICATION_PREPARE_BUS_SLEEP) &&
                (((ChannelInternal->SubMode == COMM_FULL_COM_NETWORK_REQUESTED) && (ChannelConf->NmVariant == COMM_NM_VARIANT_PASSIVE)) || (ChannelInternal->SubMode == COMM_FULL_COM_READY_SLEEP))) {
        // "prepare bus sleep" indication
        status = ComM_Internal_Enter_SilentCom(ChannelConf, ChannelInternal);  /**< @req COMM299.partially */ /** @req COMM900 */
        ChannelInternal->NmIndicationMask &= ~(COMM_NM_INDICATION_PREPARE_BUS_SLEEP);
    } else {
        if ((((ChannelInternal->InhibitionStatus & COMM_INHIBITION_STATUS_NO_COMMUNICATION)==COMM_INHIBITION_STATUS_NO_COMMUNICATION) ||
            (ComM_Internal.NoCommunication == TRUE)) && (ChannelInternal->DCM_Requested == FALSE)) { /** @req COMM182 */
            // Inhibition is active
            if (TRUE == ComM_Internal_FullComMinTime_AllowsExit(ChannelConf, ChannelInternal)) {
                if (ChannelInternal->SubMode == COMM_FULL_COM_READY_SLEEP) {
                    if ((ChannelConf->NmVariant == COMM_NM_VARIANT_LIGHT) &&
                        (ChannelInternal->LightTimeoutTimeLeft == 0)) {
                        status = ComM_Internal_Enter_NoCom(ChannelConf, ChannelInternal);  /**< @req COMM610 */
                    }
                } else {
                    /** @req COMM303  @req COMM215.1 */
                    status = ComM_Internal_Enter_ReadySleep(ChannelConf, ChannelInternal);
                }
            }
            if (TRUE == isRequest){
                /* @req COMM142 */
                /* IMPROVEMENT: Should only increment if it is a request for
                 * COMM_FULL_COMMUNICATION.
                 * And we should return COMM_E_MODE_LIMITATION. */
                if (ComM_Internal.InhibitCounter < 65535) {
                    ComM_Internal.InhibitCounter++;
                }
            }
        } else {
            if ((ChannelInternal->UserRequestMask == 0) && (ChannelInternal->DCM_Requested == FALSE)) {
                // Channel no longer requested
                if (TRUE == ComM_Internal_FullComMinTime_AllowsExit(ChannelConf, ChannelInternal)) {
                    if (ChannelInternal->SubMode == COMM_FULL_COM_READY_SLEEP) {
                        if ((ChannelConf->NmVariant == COMM_NM_VARIANT_LIGHT) &&
                            (ChannelInternal->LightTimeoutTimeLeft == 0)) {
                            status = ComM_Internal_Enter_NoCom(ChannelConf, ChannelInternal);  /**< @req COMM610 */
                        }
                    } else {
                        status = ComM_Internal_Enter_ReadySleep(ChannelConf, ChannelInternal); /** @req COMM889 */ /** @req COMM888 */ /** @req COMM890 */
                    }
                }
            } else {
                // Channel is requested
                if (ChannelInternal->SubMode != COMM_FULL_COM_NETWORK_REQUESTED) { /** @req COMM842 */
                    status = ComM_Internal_Enter_NetworkRequested(ChannelConf, ChannelInternal);  /** @req COMM882 */ /* @req COMM883 */
                }
            }
        }
    }
    return status;
}


/**
 * @brief Processes all requests stored for a channel. and makes state machine transitions accordingly
 * @param ChannelConf
 * @param isRequest
 * @return
 */
static Std_ReturnType ComM_Internal_UpdateChannelState( const ComM_ChannelType* ChannelConf, boolean isRequest ) {
    ComM_Internal_ChannelType* ChannelInternal = &ComM_Internal.Channels[ChannelConf->ComMChannelId];
    Std_ReturnType status = E_OK;
    ChannelInternal->requestPending = FALSE; /* Avoid any pending request, treat this as a new Channel request */
    switch (ChannelInternal->Mode) {
        case COMM_NO_COMMUNICATION:
            status = ComM_Internal_UpdateFromNoCom(ChannelConf, ChannelInternal, isRequest);
            break;
        case COMM_SILENT_COMMUNICATION:
            status = ComM_Internal_UpdateFromSilentCom(ChannelConf, ChannelInternal, isRequest);
            break;
        case COMM_FULL_COMMUNICATION:
            status = ComM_Internal_UpdateFromFullCom(ChannelConf, ChannelInternal, isRequest);
            break;
        default:
            status = E_NOT_OK;
            break;
    }

    return status;
}


/**
 * @brief Tick 'Min full com duration' timeout, and update state if needed
 * @param ChannelConf
 * @param ChannelInternal
 * @return
 */
static inline Std_ReturnType ComM_Internal_TickFullComMinTime(const ComM_ChannelType* ChannelConf, ComM_Internal_ChannelType* ChannelInternal) {
    Std_ReturnType status = E_OK;
    if (ChannelInternal->Mode == COMM_FULL_COMMUNICATION) {
        if (ChannelConf->MainFunctionPeriod >= ChannelInternal->FullComMinDurationTimeLeft) {
            ChannelInternal->FullComMinDurationTimeLeft = 0;
            status = ComM_Internal_UpdateChannelState(ChannelConf, FALSE);
        } else {
            ChannelInternal->FullComMinDurationTimeLeft -= ChannelConf->MainFunctionPeriod;
        }
    }
    return status;
}

/**
 * @brief  Tick 'Light nm' timeout, and update state if needed
 * @param ChannelConf
 * @param ChannelInternal
 * @return
 */
static inline Std_ReturnType ComM_Internal_TickLightTime(const ComM_ChannelType* ChannelConf, ComM_Internal_ChannelType* ChannelInternal) {
    Std_ReturnType status = E_OK;
    if ((ChannelInternal->Mode == COMM_FULL_COMMUNICATION) &&
        (ChannelInternal->SubMode == COMM_FULL_COM_READY_SLEEP)) {
        if (ChannelConf->MainFunctionPeriod >= ChannelInternal->LightTimeoutTimeLeft) {
            ChannelInternal->LightTimeoutTimeLeft = 0;
            status = ComM_Internal_UpdateChannelState(ChannelConf, FALSE);
        } else {
            ChannelInternal->LightTimeoutTimeLeft -= ChannelConf->MainFunctionPeriod;
        }
    }
    return status;
}


/**
 * @brief Propagate query to channel Bus SMs. Collect overall mode and status
 * @param User
 * @param ComMode
 * @return
 */
static Std_ReturnType ComM_Internal_PropagateGetCurrentComMode( ComM_UserHandleType User, ComM_ModeType* ComMode ){
    const ComM_UserType* UserConfig = &ComM_ConfigPtr->Users[User];
    const ComM_ChannelType* Channel;
    ComM_ModeType requestMode = COMM_FULL_COMMUNICATION;
    Std_ReturnType totalStatus = E_OK;
    uint8 loopCount;

#if (COMM_PNC_SUPPORT == STD_ON)
    /* Check if PNC is enabled and user references any PNC */
    if((TRUE == ComM_ConfigPtr->ComMPncConfig->ComMPncEnabled) && (UserConfig->PncChnlCount !=0)) {
        loopCount = UserConfig->ChannelCount + UserConfig->PncChnlCount;

    } else {
        loopCount = UserConfig->ChannelCount;
    }

#else
    loopCount = UserConfig->ChannelCount;
#endif
    /* Go through users channels. Relay to SMs. Collect overall mode and success status */
    for (uint8 i = 0; i < loopCount; ++i) {
#if (COMM_PNC_SUPPORT == STD_ON)
        if((TRUE == ComM_ConfigPtr->ComMPncConfig->ComMPncEnabled) && (UserConfig->PncChnlCount !=0)) {
            if (i >= UserConfig->ChannelCount){
                Channel = UserConfig->PncChnlList[i-UserConfig->ChannelCount];/** @req COMM798 */

            } else {
                Channel = UserConfig->ChannelList[i]; /** @req COMM798 */
            }

        } else {
            Channel = UserConfig->ChannelList[i]; /** @req COMM798 */
        }
#else
        Channel = UserConfig->ChannelList[i]; /** @req COMM798 */
#endif
        Std_ReturnType status = E_OK;
        ComM_ModeType mode = COMM_FULL_COMMUNICATION;
        switch (Channel->BusType) {
#if defined(USE_CANSM)
            case COMM_BUS_TYPE_CAN:
                status = CanSM_GetCurrentComMode(Channel->ComMChannelId, &mode); /** @req COMM855 */
                break;
#endif
#if defined(USE_LINSM)
        case COMM_BUS_TYPE_LIN:
            status = LinSM_GetCurrentComMode(Channel->ComMChannelId, &mode); /** @req COMM857 */
            break;
#endif
#if defined(USE_ETHSM)
        case COMM_BUS_TYPE_ETH:
            status = EthSM_GetCurrentComMode(Channel->ComMChannelId, &mode); /** @req COMM860 */
            break;
#endif
#if defined (USE_FRSM)
        case COMM_BUS_TYPE_FR:
            status = FrSM_GetCurrentComMode(Channel->ComMChannelId, &mode); /** @req ComM853 */
            break;
#endif
            default:
                status = E_NOT_OK;
                break;
        }
        if (status == E_OK) {
            if (mode < requestMode) {   /** @req COMM176 */
                requestMode = mode;
            }
        } else {
            totalStatus = status;
        }
    }
    *ComMode = requestMode;
    return totalStatus;
}

/**
 * @brief Delegate request to users channels and call ComM_Internal_UpdateChannelState
 * @param User
 * @param ComMode
 * @return
 */
static Std_ReturnType ComM_Internal_RequestComMode(
                ComM_UserHandleType User, ComM_ModeType ComMode ){

    const ComM_UserType* UserConfig = &ComM_ConfigPtr->Users[User]; /** @req COMM795 */
    ComM_Internal_UserType* UserInternal = &ComM_Internal.Users[User];
    const ComM_ChannelType* Channel;
    uint8 loopCount;
    uint8 chnlId = 0;

#if (COMM_PNC_SUPPORT == STD_ON)
    /* Check if PNC is enabled and user references any PNC */
    if((TRUE == ComM_ConfigPtr->ComMPncConfig->ComMPncEnabled) && (UserConfig->PncChnlCount !=0)) {
        loopCount = UserConfig->ChannelCount + UserConfig->PncChnlCount;

    } else {
        loopCount = UserConfig->ChannelCount;
    }

#else
    loopCount = UserConfig->ChannelCount;
#endif
    UserInternal->RequestedMode = ComMode;  /**<  @req COMM500  @req COMM92 */
    uint32 userMask = (1LU << User);

    Std_ReturnType requestStatus = E_OK;

    /* Go through users channels. Relay to SMs. Collect overall success status */
    for (uint8 i = 0; i < loopCount; ++i) {
#if (COMM_PNC_SUPPORT == STD_ON)
        if((TRUE == ComM_ConfigPtr->ComMPncConfig->ComMPncEnabled) && (UserConfig->PncChnlCount !=0)) {
            if (i >= UserConfig->ChannelCount){
                Channel = UserConfig->PncChnlList[i-UserConfig->ChannelCount];

            } else {
                Channel = UserConfig->ChannelList[i];
            }

        } else {
            Channel = UserConfig->ChannelList[i];
        }
#else
        Channel = UserConfig->ChannelList[i];
#endif
        ComM_Internal_ChannelType* ChannelInternal = &ComM_Internal.Channels[Channel->ComMChannelId];

        /** @req COMM625 */
        // Put user request into mask
        if (ComMode == COMM_NO_COMMUNICATION) {
            ChannelInternal->UserRequestMask &= ~(userMask);/* @req COMM840 */
        } else if (ComMode == COMM_FULL_COMMUNICATION) {
            ChannelInternal->UserRequestMask |= userMask; /** @req COMM839 */
        } else {
            /*Nothing to be done.*/
        }

#if (COMM_PNC_SUPPORT == STD_ON)
        if (ComMode == COMM_NO_COMMUNICATION){
            /* This will be handled by ComM_Internal_HandlePncStateChange() */
            /* i.e Only If all users mapped to a PNC requests a NO_COM then PNC state changes to PNC_READY_SLEEP.
             * When PNC_READY_SLEEP is entered Nm channels are released.
             * Wait for ComM_Nm_BusSleepMode() & ComM_Nm_PrepareBusSleepMode(). This call back will trigger
             * either ComM_Internal_Enter_NoCom() or ComM_Internal_Enter_SilentCom() respectively.
             * In nut shell all Channels are requested to No_Com only depending on the Nm state indications after all PNCs have released
             * Assumption all ComMChannels are of type ComMNetworkManagement
             */

            /* Check if this ComMChannels is related to any PNC
             * Shortcut would be to see if there exists a ComM_TxComSignals[] at an index of this ComMChannelId
             * Note: configuration ComM_TxComSignals[] is generated in the same order as ComM_Channels[]
             */
            chnlId = Channel->ComMChannelId;
            if (ComM_ConfigPtr->ComMPncConfig->ComMPncTxComSigIdRef[chnlId].ComMPncSignalId != INVALID_SIGNAL_HANDLE) {
                continue; /* Continue with next iteration */
            }
        }
#else
        chnlId = 0;
        (void)chnlId;
#endif
        // take request -> new state
        Std_ReturnType status = ComM_Internal_UpdateChannelState(Channel, TRUE);
        if (status > requestStatus){
            requestStatus = status;
        }

    }

    return requestStatus;
}

#if (COMM_PNC_SUPPORT == STD_ON)

/**
 * @brief Register new user requests for PN
 * @param User
 * @param ComMode
 */
static void ComM_Internal_UpdatePncChangeRequests(ComM_UserHandleType User,ComM_ModeType ComMode){
    const ComM_PNConfigType * pncCfg;
    uint8 pncCount;
    uint8 usrCnt;

    pncCfg  =   ComM_ConfigPtr->ComMPncConfig;

    /** check all PNCs mapped to this user */
    for (pncCount=0;(pncCount <  pncCfg->ComMPncNum);pncCount++) {

        if (pncCfg->ComMPnc[pncCount].ComMPncUserRefNum !=0) {
            /** Scan all  users associated with the PNC */
            for (usrCnt=0; usrCnt < pncCfg->ComMPnc[pncCount].ComMPncUserRefNum; usrCnt++) {
                if (pncCfg->ComMPnc[pncCount].ComMPncUserRef[usrCnt] == User) {
                    ComM_Internal.pncRunTimeData[pncCount].pncRequestedState   = ComMode;
                    ComM_Internal.pncRunTimeData[pncCount].pncNewUserRequest = TRUE;
                }
            }
        }
    }
}

/**
 * @brief Save the trigger for new EIRA reception
 */
void ComM_Arc_IndicateNewRxEIRA(void) {
    ComM_Internal.newEIRARxSignal = TRUE;
}

/**
 * @brief update bits in TxComSignal
 * @param byteIndex
 * @param bitIndex
 * @param set
 */
static inline void ComM_Internal_updateTxComSignal(uint8 chnlIdx, uint8 byteIndex,uint8 bitIndex,boolean set) {

    uint8 mask;

    mask =(uint8)(1u << bitIndex);

    if (FALSE == set) {
        mask = ~mask;
        ComM_Internal.Channels[chnlIdx].TxComSignal.bytes[byteIndex] &= mask;
    } else {
        ComM_Internal.Channels[chnlIdx].TxComSignal.bytes[byteIndex] |= mask;
    }

}

/**
 *
 * @param pncCount
 * @param trig - Set Internal request or release
 * @param gwType - Type of gw channel to be selected for send operation
 */
static void comTransmitSignal(uint8 pncCount, ComM_Internal_TriggerSendType trig, ComM_PncGatewayType gwType)
{
    uint8 i;
    uint8 chnlIdx;
    uint8 pncId;
    uint8 byteNo;
    uint8 bitNo;
    uint8 mask;

    pncId = ComM_ConfigPtr->ComMPncConfig->ComMPnc[pncCount].ComMPncId;
    byteNo = pncId/8;
    bitNo =  pncId%8;
    Com_SignalIdType sigId;

    for (i = 0;i < ComM_ConfigPtr->ComMPncConfig->ComMPnc[pncCount].ComMPncChnlRefNum; i++)
    {
        chnlIdx  = ComM_ConfigPtr->ComMPncConfig->ComMPnc[pncCount].ComMPncChnlRef[i];

#if (COMM_PNC_GATEWAY_ENABLED == STD_ON)
        if ((gwType != ComM_ConfigPtr->Channels[chnlIdx].PncGatewayType) && (gwType != COMM_GATEWAY_TYPE_NONE))
        {
            continue; /* Only ComMChannels with active/passive gateway is requested */
            /*gwType is COMM_GATEWAY_TYPE_NONE, both acitve and passive channel Tx signals are sent */
        }
#else
        (void)gwType;
#endif
        mask =(uint8)(1u << bitNo);
        if (TRIGGER_SEND_PNC_VAL_0 == trig) {
            if ((ComM_Internal.Channels[chnlIdx].TxComSignal.bytes[byteNo] & mask) != 0) {
                    ComM_Internal_updateTxComSignal(chnlIdx, byteNo, bitNo, FALSE); /** @req ComM960 */
            } else {
                continue; /* No re-transmission required */
            }
        }
        else {
            if ((ComM_Internal.Channels[chnlIdx].TxComSignal.bytes[byteNo] & mask) == 0) {
                ComM_Internal_updateTxComSignal(chnlIdx, byteNo, bitNo, TRUE);     /** @req ComM930 */
            } else {
                continue; /* No re-transmission required */
            }
        }
        sigId = ComM_ConfigPtr->ComMPncConfig->ComMPncTxComSigIdRef[chnlIdx].ComMPncSignalId;
        /** @req ComM975 */
        if ( sigId != INVALID_SIGNAL_HANDLE) {
            (void)Com_SendSignal(sigId, ComM_Internal.Channels[chnlIdx].TxComSignal.bytes);
        }
    }
}

/**
 * @brief Request all Channels to Enter Full Com
 * @param pncCount
 */
static inline void reqAllPncComMChlsFullCom(uint8 pncCount)  {
    uint8 chnlId;
    uint8 i;
    Std_ReturnType status;
    const ComM_ChannelType* ChannelConf;
    ComM_Internal_ChannelType* ChannelInternal;
    for (i=0;i < ComM_ConfigPtr->ComMPncConfig->ComMPnc[pncCount].ComMPncChnlRefNum; i++)
       {
           chnlId = ComM_ConfigPtr->ComMPncConfig->ComMPnc[pncCount].ComMPncChnlRef[i]; /* Get the channel handle */
           ChannelConf = &ComM_ConfigPtr->Channels[chnlId];
           ChannelInternal = &ComM_Internal.Channels[chnlId];

           status =  ComM_Internal_PropagateComMode(ChannelConf, COMM_FULL_COMMUNICATION);/* Expect that this returns E_OK. Raise DET if fails */

           if (status == E_OK) {
                  ChannelInternal->RequestedSubMode = COMM_FULL_COM_NETWORK_REQUESTED;
                  /* Nm Network will be requested on receipt of ComM_BusSM_ModeIndication() */
           }
       }
}


/**
 * @brief Request all Nm Channels to release Nw in READY_SLEEP state because nothing to transmit (TxSignals are zero)
 * @param pncCount
 */
static inline void reqAllPncNmChannelsRelease(uint8 pncCount)  {

    /* This function is called whenever PNC State READY_SLEEP is entered.
     * The ComMChannels are not requested NO_COMMUNICATION because that would
     * cause all Channels to become disabled and we do not receive any further EIRA or ERAs.
     * Also ComM929 relies on the fact that if PNC is in a main state (PNC_FULL_COMMUNICATION)
     * i.e (READY_SLEEP, REQUESTED, PREPARE_SLEEP) all ComMChannels are in FULL_COM.
     */
    uint8 chnlId;
    uint8 i;
    const ComM_ChannelType* ChannelConf;

    for (i=0;i < ComM_ConfigPtr->ComMPncConfig->ComMPnc[pncCount].ComMPncChnlRefNum; i++)
       {
           chnlId = ComM_ConfigPtr->ComMPncConfig->ComMPnc[pncCount].ComMPncChnlRef[i]; /* Get the channel handle */
           ChannelConf = &ComM_ConfigPtr->Channels[chnlId];

           (void)Nm_NetworkRelease(ChannelConf->NmChannelHandle);
       }
}
/**
 * @brief update bits in EIRA
 * @param byteIndex
 * @param bitIndex
 * @param set
 */
static inline void ComM_Internal_updateEIRAPNCBit(uint8 byteIndex,uint8 bitIndex,boolean set) {

    uint8 mask;

    mask = (uint8)(1u << bitIndex);

    if (FALSE == set) {
        mask = ~mask;
        ComM_Internal.pnEIRA.bytes[byteIndex] &= mask;
    } else {
        ComM_Internal.pnEIRA.bytes[byteIndex] |= mask;
    }

}


#define LSBIT_MASK 0x1u

/**
 * @brief Update ComM PNC states for new EIRA received
 * @return Indicate if the state has change to PNC_PREPARE_SLEEP or not
 */
static void ComM_Internal_EIRARxSignalUpdate(void) {
    uint8 newEIRA[COMM_PNC_COMSIGNAL_VECTOR_LEN];
    uint8 byteNo;
    uint8 bitNo;
    uint8 set;
    PNCHandleType pnc;
    uint8 pncCount;
    uint8 ret;
    boolean isStateChanged;
    uint8 i;

    ComM_Internal.newEIRARxSignal = FALSE;

    if (ComM_ConfigPtr->ComMPncConfig->ComMPncEIRARxComSigIdRef == NULL) {
        /*nothing*/
    } else {

        /** @req ComM984 */
        ret = Com_ReceiveSignal(ComM_ConfigPtr->ComMPncConfig->ComMPncEIRARxComSigIdRef->ComMPncSignalId,newEIRA);

        for (byteNo=0;(byteNo< COMM_PNC_COMSIGNAL_VECTOR_LEN) && (ret == E_OK);byteNo++) {

            if (ComM_Internal.pnEIRA.bytes[byteNo] != newEIRA[byteNo]) {

                for (bitNo=0;bitNo<8;bitNo++) {
                    set =   ((newEIRA[byteNo] >> bitNo)&LSBIT_MASK);

                    if (((ComM_Internal.pnEIRA.bytes[byteNo] >> bitNo)&LSBIT_MASK) != set) {

                        ComM_Internal_updateEIRAPNCBit(byteNo,bitNo,(boolean)set);
                        /*lint -e{734} *//* the number of pnc is limited to 48, no loss of precision */
                        pnc         =   (byteNo*8)+bitNo;
                        for (i=0;i <ComM_ConfigPtr->ComMPncConfig->ComMPncNum; i++){
                            if (ComM_ConfigPtr->ComMPncConfig->ComMPnc[i].ComMPncId == pnc) {
                                break; /* Found */
                            }
                        }
                        if (i == ComM_ConfigPtr->ComMPncConfig->ComMPncNum) {
                            continue; /* Not found */
                        }
                        pncCount    =   ComM_ConfigPtr->ComMPncConfig->ComMPncIdToPncCountVal[pnc];
                        isStateChanged = FALSE;
                        if (set==1) {
                            /** @req ComM944 */
                            if (PNC_PREPARE_SLEEP == ComM_Internal.pncRunTimeData[pncCount].pncSubState) {
                                /** @req ComM950 */
                                ComM_Internal.pncRunTimeData[pncCount].pncSubState = PNC_READY_SLEEP;
                                isStateChanged = TRUE;
                            } else if (PNC_NO_COMMUNICATION == ComM_Internal.pncRunTimeData[pncCount].pncSubState) {
                                /** @req ComM933 */
                                ComM_Internal.pncRunTimeData[pncCount].pncSubState = PNC_READY_SLEEP;
                                ComM_Internal.pncRunTimeData[pncCount].pncState    = PNC_FULL_COMMUNICATION_STATE;
                                isStateChanged = TRUE;
                                /* @adma No need to fulfill ComM929 here, if channels are requested FULL_COM, it leads
                                 * to Nm network request which is not intended in READY_SLEEP state.
                                 * Also because of the reason we are in FULL_COM we have received this EIRA indication from CanNm
                                 */
                            } else {
                                /* Do nothing */
                            }

                        } else {
                            if (PNC_READY_SLEEP == ComM_Internal.pncRunTimeData[pncCount].pncSubState) {
                                /** @req ComM940 */
                                ComM_Internal.pncRunTimeData[pncCount].pncSubState      = PNC_PREPARE_SLEEP;
                                /** @req ComM952 */
                                ComM_Internal.pncRunTimeData[pncCount].prepareSleepTimer    = COMM_T_PNC_PREPARE_SLEEP;
                                isStateChanged = TRUE;
                            }
                        }
                        if (TRUE == isStateChanged) {
                            ComM_PncStateTrans_ThisCycle = TRUE;
#if defined(USE_BSWM)

                            /** @req ComM908 */
                            /** @req COMM976 */
                            BswM_ComM_CurrentPNCMode(pnc,ComM_Internal.pncRunTimeData[pncCount].pncSubState);
#endif
                        }
                    }
                }
            }
        }
    }
}

/** @req ComM988 *//* Pnc SM description */
/**
 * @brief Handle PNC state change
 * @param idx index of PNC
 * @param req flag to indicate if EIRA has to be updated or not
 * @param Channel Related ComM channel
 * @return
 */
static void ComM_Internal_HandlePncStateChange(uint8 pncCount, ComM_Internal_TriggerSendType *req) {

    uint8 byteIndex;
    uint8 bitIndex;
    PNCHandleType pnc;
    uint8 i;
    uint8 usr;
    uint8 channelIndex;
    boolean isStateChanged;
    boolean status;
    status = TRUE;

    pnc = ComM_ConfigPtr->ComMPncConfig->ComMPnc[pncCount].ComMPncId;
    isStateChanged= FALSE;

    if(COMM_NO_COMMUNICATION == ComM_Internal.pncRunTimeData[pncCount].pncRequestedState) {
        if (PNC_REQUESTED == ComM_Internal.pncRunTimeData[pncCount].pncSubState) {
             /* If current state is PNC_REQUESTED check whether any other user is requesting this PNC */
            for (i=0;i < ComM_ConfigPtr->ComMPncConfig->ComMPnc[pncCount].ComMPncUserRefNum; i++)
            {
                usr = ComM_ConfigPtr->ComMPncConfig->ComMPnc[pncCount].ComMPncUserRef[i]; /* Get the user handle */
                if (ComM_Internal.Users[usr].RequestedMode == COMM_FULL_COMMUNICATION) {
                    /** @req ComM936 */
                    status = FALSE;
                    break; /* No need of PNC state change. At least one user requesting COMM_FULL_COMMUNICATION */
                }
            }
#if (COMM_PNC_GATEWAY_ENABLED == STD_OFF)
            /** @req ComM938 */
            if(status == TRUE){
                ComM_Internal.pncRunTimeData[pncCount].pncSubState = PNC_READY_SLEEP;
                *req = TRIGGER_SEND_PNC_VAL_0;
                isStateChanged = TRUE;
                reqAllPncNmChannelsRelease(pncCount); /* @req 4.2.2SWS_ComM_00961 */ /* This is required to make CanNm enter Ready sleep state*/
                channelIndex = 0;
                /*lint -e{530} */
                (void)channelIndex;
                byteIndex = 0;
                bitIndex = 0;
                (void)byteIndex;
                (void)bitIndex;
            }
        }
#else /* COMM_PNC_GATEWAY_ENABLED == STD_ON */
            if(status == TRUE){
                byteIndex = pnc/8;
                bitIndex  = pnc%8;
                for (channelIndex = 0; channelIndex < COMM_CHANNEL_COUNT; channelIndex++) {
                    if ((ComM_Internal.Channels[channelIndex].pnERA.bytes[byteIndex] & (1u << bitIndex)) != 0) {
                        status = FALSE;
                        break; /* PNC in all ERA is not released */
                    }
                }
                if (status == TRUE) {
                    /** @req ComM991 */
                    ComM_Internal.pncRunTimeData[pncCount].pncSubState = PNC_READY_SLEEP;
                    *req = TRIGGER_SEND_PNC_VAL_0;
                    isStateChanged = TRUE;
                    reqAllPncNmChannelsRelease(pncCount); /* @req 4.2.2SWS_ComM_00961 */ /* This is required to make CanNm enter Ready sleep state*/
                }
            }
        }
#endif /* COMM_PNC_GATEWAY_ENABLED */
    } else {
        if (ComM_Internal.pncRunTimeData[pncCount].pncSubState !=  PNC_REQUESTED) {
            /** @req ComM932 */ /** @req ComM948 */
            ComM_Internal.pncRunTimeData[pncCount].pncSubState = PNC_REQUESTED;
            ComM_Internal.pncRunTimeData[pncCount].pncState    = PNC_FULL_COMMUNICATION_STATE;
            reqAllPncComMChlsFullCom(pncCount); /* @req ComM993 */
            *req = TRIGGER_SEND_PNC_VAL_1;    /** @req ComM992 */ /* @req 4.2.2 SWS_ComM_00164 */
            isStateChanged = TRUE;
        }
        *req = TRIGGER_SEND_PNC_VAL_1;    /** @req ComM992 */ /* @req 4.2.2 SWS_ComM_00164 */
    }

    if ((status == TRUE) && (TRUE == isStateChanged)) {
        ComM_PncStateTrans_ThisCycle = TRUE;
#if defined(USE_BSWM)
        /** @req ComM908 */
        /** @req COMM976 */
        BswM_ComM_CurrentPNCMode(pnc,ComM_Internal.pncRunTimeData[pncCount].pncSubState);
#endif
    }
    (void)pnc; // Lint: Symbol not accessed
}


/**
 * @brief Process new user requests
 * @param Channel
 * @return indicates if Channel is requested to change state or not
 */
static void ComM_Internal_ProcessPNReqMode(NetworkHandleType Channel) {

    uint8 pncCount;
    uint8 chnlIdx;
    ComM_Internal_TriggerSendType trigVal;

    trigVal= INVALID_TRIGGER_SEND;
    for (pncCount=0;pncCount<ComM_ConfigPtr->ComMPncConfig->ComMPncNum;pncCount++) {

        for(chnlIdx=0;chnlIdx < ComM_ConfigPtr->ComMPncConfig->ComMPnc[pncCount].ComMPncChnlRefNum;chnlIdx++) {
            /* Find any matching channel for the current iteration PNC */
            if (Channel == ComM_ConfigPtr->ComMPncConfig->ComMPnc[pncCount].ComMPncChnlRef[chnlIdx]) {

                if (TRUE == ComM_Internal.pncRunTimeData[pncCount].pncNewUserRequest){
                    ComM_Internal_HandlePncStateChange(pncCount,&trigVal);
                    ComM_Internal.pncRunTimeData[pncCount].pncNewUserRequest = FALSE;
                    if (trigVal != INVALID_TRIGGER_SEND ) {
                        comTransmitSignal(pncCount,trigVal, COMM_GATEWAY_TYPE_NONE);
                    }
                    break; /* channel found */
                }
            }
        }
    }
}


/**
 * @brief Tick prepare sleep timer for PNC
 * @param ChannelConf
 */
static inline void ComM_Internal_TickPncPrepareSleepTime(const ComM_ChannelType* ChannelConf) {

    uint8 pncCount;
    PNCHandleType pncId = 0;

    for (pncCount=0;pncCount<ComM_ConfigPtr->ComMPncConfig->ComMPncNum;pncCount++) {
        /** @req ComM943 */
        if (PNC_PREPARE_SLEEP == ComM_Internal.pncRunTimeData[pncCount].pncSubState ) {
            /* Decrement PN sleep timers for 1st main function of the PNC only */
            if (ChannelConf->ComMChannelId == ComM_ConfigPtr->ComMPncConfig->ComMPnc[pncCount].ComMPncChnlRef[PNC_FIRST_CHNL_REF]) {

                if (ChannelConf->MainFunctionPeriod > ComM_Internal.pncRunTimeData[pncCount].prepareSleepTimer) {
                    ComM_Internal.pncRunTimeData[pncCount].prepareSleepTimer = 0;

                    pncId      = ComM_ConfigPtr->ComMPncConfig->ComMPnc[pncCount].ComMPncId;
                    /** @req ComM947 */
                    ComM_Internal.pncRunTimeData[pncCount].pncSubState  =   PNC_NO_COMMUNICATION;
                    ComM_Internal.pncRunTimeData[pncCount].pncState     =   PNC_NO_COMMUNICATION_STATE;
                    ComM_PncStateTrans_ThisCycle = TRUE;
#if defined(USE_BSWM)
                    /** @req ComM908 */
                    /** @req COMM976 */
                    BswM_ComM_CurrentPNCMode(pncId,ComM_Internal.pncRunTimeData[pncCount].pncSubState);
#endif /* defined(USE_BSWM) */

                } else {
                    ComM_Internal.pncRunTimeData[pncCount].prepareSleepTimer -= ChannelConf->MainFunctionPeriod;
                }
            }
        }
    }
    (void)pncId; // Lint: Symbol not accessed
}

/**
 * @brief  All pnc in ready sleep is checked whether a transition to prepare sleep is required
 */
static void ComM_Internal_Monitor_RdySlpToPrpSlpTrns(void)
{

    uint8 byteNo;
    uint8 bitNo;
    uint8 pncCount;
    PNCHandleType pncId;
    uint8 mask;

    /** @req ComM942 */ /* Retain this state until no new Internal requests*/
    for(pncCount=0;pncCount<ComM_ConfigPtr->ComMPncConfig->ComMPncNum;pncCount++) {

        if (PNC_READY_SLEEP == ComM_Internal.pncRunTimeData[pncCount].pncSubState) {

            pncId  = ComM_ConfigPtr->ComMPncConfig->ComMPnc[pncCount].ComMPncId;
            byteNo = pncId /8;
            bitNo  = pncId %8;
            mask   = (uint8)(1u << bitNo);

            if(	(ComM_Internal.pnEIRA.bytes[byteNo] & mask) == 0)
            {
                /** @req ComM940 */
                ComM_Internal.pncRunTimeData[pncCount].pncSubState      = PNC_PREPARE_SLEEP;
                /** @req ComM952 */
                ComM_Internal.pncRunTimeData[pncCount].prepareSleepTimer    = COMM_T_PNC_PREPARE_SLEEP;
                ComM_PncStateTrans_ThisCycle = TRUE;
#if defined(USE_BSWM)
                /** @req ComM908 */
                /** @req COMM976 */
                BswM_ComM_CurrentPNCMode(pncId,ComM_Internal.pncRunTimeData[pncCount].pncSubState);
#endif /* defined(USE_BSWM) */
            }
        }
    }
}

#if (HOST_TEST == STD_ON)
uint64 * readEira(void) {
  return  &ComM_Internal.pnEIRA.data;
}

ComM_PncStateType checkPncStatus(uint8 idx) {
    return ComM_Internal.pncRunTimeData[idx].pncState;
}

ComM_PncModeType checkPncMode(uint8 idx) {
    return ComM_Internal.pncRunTimeData[idx].pncSubState;
}

#endif /* (HOST_TEST == STD_ON) */

#if (COMM_PNC_GATEWAY_ENABLED == STD_ON)
/**
 * @brief Save the trigger for new ERA reception
 */
void ComM_Arc_IndicateNewRxERA(uint8 channelIndex) {
    ComM_Internal.Channels[channelIndex].newERARxSignal = TRUE;
}

/**
 * @brief update bits in RA
 * @param byteIndex
 * @param bitIndex
 * @param set
 */
/*lint -e{9018} No issue using union as per the logic */
static inline void ComM_Internal_updateRAPNCBit(ComM_Internal_RAType *RA,
        uint8 byteIndex, uint8 bitIndex, boolean set) {

    if (FALSE == set) {
        RA->bytes[byteIndex] &=(uint8) ~(1u << bitIndex);
    } else {
        RA->bytes[byteIndex] |=(uint8)(1u << bitIndex);
    }
}

/**
 * @brief Process external PNC request i.e Rx ERA with PNC bit set to 1
 * @param pncCount
 * @param channelIndex
 * @return
 */
static boolean externalPncRequest(uint8 pncCount,uint8 channelIndex){
    boolean isStateChanged;
    boolean doReqestStateTrans;

    isStateChanged = FALSE;
    doReqestStateTrans = TRUE;

    /** @req ComM945 */
    if (PNC_PREPARE_SLEEP == ComM_Internal.pncRunTimeData[pncCount].pncSubState) {
        /** @req ComM951 */
        ComM_Internal.pncRunTimeData[pncCount].pncSubState = PNC_REQUESTED;
        reqAllPncComMChlsFullCom(pncCount); /* @req ComM993 */
        isStateChanged  = TRUE;
    } else if (PNC_NO_COMMUNICATION == ComM_Internal.pncRunTimeData[pncCount].pncSubState) {
        /** @req ComM934 */
        ComM_Internal.pncRunTimeData[pncCount].pncSubState = PNC_REQUESTED;
        ComM_Internal.pncRunTimeData[pncCount].pncState    = PNC_FULL_COMMUNICATION_STATE;
        /** @req ComM929 */
        reqAllPncComMChlsFullCom(pncCount); /* @req ComM993 */
        isStateChanged  = TRUE;
    } else if (PNC_READY_SLEEP == ComM_Internal.pncRunTimeData[pncCount].pncSubState) {
        ComM_Internal.pncRunTimeData[pncCount].pncSubState = PNC_REQUESTED;
        reqAllPncComMChlsFullCom(pncCount); /* @req ComM993 */
        isStateChanged  = TRUE;
    } else {
        doReqestStateTrans = FALSE;
    }
    if (COMM_GATEWAY_TYPE_ACTIVE == ComM_ConfigPtr->Channels[channelIndex].PncGatewayType )
    {
        comTransmitSignal(pncCount, TRIGGER_SEND_PNC_VAL_1,COMM_GATEWAY_TYPE_NONE);     /** @req ComM992 */ /* @req 4.2.2 SWS_ComM_00164 */
    } else if (TRUE == doReqestStateTrans) {
        comTransmitSignal(pncCount, TRIGGER_SEND_PNC_VAL_1, COMM_GATEWAY_TYPE_ACTIVE);     /** @req ComM992 */
    } else {
        /* Do nothing */
    }
    return isStateChanged;
}

/**
 * @brief Release any external PNC request (i.e Rx ERA with PNC bit set to 0)
 * @param pncCount
 * @param byteNo
 * @param bitNo
 * @return
 */
static boolean externalPncRelease(uint8 pncCount,uint8 byteNo,uint8 bitNo) {
    boolean isStateChanged;
    boolean doReadySleepTrans;
    uint8 usrHandle;
    uint8 usrCnt;
    uint8 channelId;

    isStateChanged = FALSE;
    doReadySleepTrans = TRUE;
    if (PNC_REQUESTED == ComM_Internal.pncRunTimeData[pncCount].pncSubState) {
         /* If current state is PNC_REQUESTED check whether any other user is requesting this PNC */
        for (usrCnt=0;usrCnt< ComM_ConfigPtr->ComMPncConfig->ComMPnc[pncCount].ComMPncUserRefNum; usrCnt++)
        {
            usrHandle = ComM_ConfigPtr->ComMPncConfig->ComMPnc[pncCount].ComMPncUserRef[usrCnt]; /* Get the user handle */
            if (ComM_Internal.Users[usrHandle].RequestedMode == COMM_FULL_COMMUNICATION) {
                doReadySleepTrans = FALSE;
                break;/* @req ComM936 */
            }
        }
        if (TRUE == doReadySleepTrans) { /* All user is requesting No Communication */
            for (channelId = 0; channelId < COMM_CHANNEL_COUNT; channelId++) {
                if ((ComM_Internal.Channels[channelId].pnERA.bytes[byteNo] & (1u << bitNo)) != 0) {
                    doReadySleepTrans = FALSE;
                    break; /* No need of PNC state change */ /* @req ComM937 */
                }
            }
            if (TRUE == doReadySleepTrans) { /* All ERAn for this PNC is relased go to Ready Sleep*/
                /** @req ComM991 */
                ComM_Internal.pncRunTimeData[pncCount].pncSubState      = PNC_READY_SLEEP;
                /** @req ComM960 */
                comTransmitSignal(pncCount, TRIGGER_SEND_PNC_VAL_0, COMM_GATEWAY_TYPE_NONE);
                isStateChanged  = TRUE;
                reqAllPncNmChannelsRelease(pncCount); /* @req 4.2.2SWS_ComM_00961 */ /* This is required to make CanNm enter Ready sleep state*/
            }
        }
    }
    return isStateChanged;
}
/**
 * @brief Update ComM PNC states for new ERA received
 */
static void ComM_Internal_ERARxSignalUpdate(uint8 channelIndex) {

    uint8 newERA[COMM_PNC_COMSIGNAL_VECTOR_LEN];
    uint8 byteNo;
    uint8 bitNo;
    uint8 set;
    PNCHandleType pnc;
    uint8 pncCount;
    uint8 ret;
    uint8 i;
    Com_SignalIdType rxSignalId;
    boolean isStateChanged;

    ComM_Internal.Channels[channelIndex].newERARxSignal = FALSE;

    rxSignalId = ComM_ConfigPtr->ComMPncConfig->ComMPncERARxComSigIdRefs[channelIndex].ComMPncSignalId;

    if ( rxSignalId != INVALID_SIGNAL_HANDLE) {
        /** @req ComM984 */
        ret = Com_ReceiveSignal(rxSignalId, newERA);

        if (ret == E_OK)
        {
            for (byteNo = 0; byteNo < COMM_PNC_COMSIGNAL_VECTOR_LEN; byteNo++) {

                if (ComM_Internal.Channels[channelIndex].pnERA.bytes[byteNo] != newERA[byteNo]) {

                    for (bitNo = 0; bitNo < 8; bitNo++) {
                        set = ((newERA[byteNo] >> bitNo) & LSBIT_MASK);

                        if (((ComM_Internal.Channels[channelIndex].pnERA.bytes[byteNo] >> bitNo) & LSBIT_MASK) != set) {
                            /* @req COMM945 */ /* We update all ERA's irrespective of gateway type */
                            ComM_Internal_updateRAPNCBit(&ComM_Internal.Channels[channelIndex].pnERA,
                                    byteNo, bitNo, (boolean) set);

                            /*lint -e{734} *//* the number of pnc is limited to 48, no loss of precision */
                            pnc         =   (byteNo * 8) + bitNo;
                            for (i=0;i <ComM_ConfigPtr->ComMPncConfig->ComMPncNum; i++){
                                if (ComM_ConfigPtr->ComMPncConfig->ComMPnc[i].ComMPncId == pnc) {
                                    break; /* Found */
                                }
                            }
                            /*lint -e{539} Did not expect positive indentation from line. OK, false positive. */
                            if (i == ComM_ConfigPtr->ComMPncConfig->ComMPncNum) {
                                continue; /* Not found */
                            }

                            pncCount    =   ComM_ConfigPtr->ComMPncConfig->ComMPncIdToPncCountVal[pnc];
                            isStateChanged = FALSE;
                            if (set == 1) {
                                isStateChanged = externalPncRequest(pncCount, channelIndex);
                            } else {
                                isStateChanged = externalPncRelease(pncCount, byteNo, bitNo);
                            }
                            if (isStateChanged  == TRUE) {
                                ComM_PncStateTrans_ThisCycle = TRUE;
    #if defined(USE_BSWM)

                                /** @req ComM908 */
                                /** @req COMM976 */
                                BswM_ComM_CurrentPNCMode(pnc,ComM_Internal.pncRunTimeData[pncCount].pncSubState);
    #endif
                            }
                        }
                    }
                }
            }
        }
    }
}

/**
 * @brief Monitors the ComMChannel of type COMM_GATEWAY_TYPE_PASSIVE when PNC is in requested mode
 */
static void ComM_Internal_Monitor_GwPassiveChnl(void)
{
    uint8 pncCount;
    uint8 i;
    uint8 usr;
    boolean sendZeroPncVal;
    uint8 chnlIdx;
    uint8 mask;
    uint8 byteNo;
    uint8 bitNo;
    PNCHandleType pncId;

    for(pncCount=0;pncCount<ComM_ConfigPtr->ComMPncConfig->ComMPncNum;pncCount++) {

        if (PNC_REQUESTED == ComM_Internal.pncRunTimeData[pncCount].pncSubState) {

            sendZeroPncVal = TRUE;

            /* If current state is PNC_REQUESTED check whether any other user is requesting this PNC */
            for (i=0;i < ComM_ConfigPtr->ComMPncConfig->ComMPnc[pncCount].ComMPncUserRefNum; i++)
            {
                usr = ComM_ConfigPtr->ComMPncConfig->ComMPnc[pncCount].ComMPncUserRef[i]; /* Get the user handle */
                if (ComM_Internal.Users[usr].RequestedMode == COMM_FULL_COMMUNICATION) {
                    sendZeroPncVal = FALSE;
                    break;/* @req ComM936 */
                }
            }

            if (TRUE == sendZeroPncVal) {

                pncId  = ComM_ConfigPtr->ComMPncConfig->ComMPnc[pncCount].ComMPncId;
                byteNo = pncId /8;
                bitNo  = pncId %8;
                mask   = (uint8)(1u << bitNo);

                for (i = 0;i < ComM_ConfigPtr->ComMPncConfig->ComMPnc[pncCount].ComMPncChnlRefNum; i++)
                {
                    chnlIdx  = ComM_ConfigPtr->ComMPncConfig->ComMPnc[pncCount].ComMPncChnlRef[i];
                    if (COMM_GATEWAY_TYPE_ACTIVE == ComM_ConfigPtr->Channels[chnlIdx].PncGatewayType)
                    {
                        if( (ComM_Internal.Channels[chnlIdx].pnERA.bytes[byteNo] & mask) != 0) {
                            sendZeroPncVal = FALSE;
                            break;
                        }
                    }
                }
                if (TRUE == sendZeroPncVal) {
                    /* @req ComM966 */
                    /* @req ComM955 */
                    /* @req ComM959 */
                    /* @req ComM946 */
                    comTransmitSignal(pncCount, TRIGGER_SEND_PNC_VAL_0, COMM_GATEWAY_TYPE_PASSIVE);
                }
            }
        }
    }
}

#endif/* COMM_PNC_GATEWAY_ENABLED == STD_ON */

#endif /* (COMM_PNC_SUPPORT == STD_ON) */


void ComM_Init(const ComM_ConfigType * Config ){
    uint8 i;
    COMM_VALIDATE_PARAMETER_NORV( (Config != NULL), COMM_SERVICEID_INIT);
    COMM_VALIDATE_PARAMETER_NORV( (Config->Channels != NULL), COMM_SERVICEID_INIT);
    COMM_VALIDATE_PARAMETER_NORV( (Config->Users != NULL), COMM_SERVICEID_INIT);

    ComM_ConfigPtr = Config;

    for (i = 0; i < COMM_CHANNEL_COUNT; ++i) {
        ComM_Internal.Channels[i].Mode = COMM_NO_COMMUNICATION;  /**< @req COMM485 */
        ComM_Internal.Channels[i].SubMode = COMM_NO_COM_NO_PENDING_REQUEST; /** @req COMM898 */
        ComM_Internal.Channels[i].RequestedSubMode = COMM_NO_COM_NO_PENDING_REQUEST;
        ComM_Internal.Channels[i].UserRequestMask = 0;
        ComM_Internal.Channels[i].DCM_Requested = FALSE;
        ComM_Internal.Channels[i].InhibitionStatus = COMM_INHIBITION_STATUS_NONE;
        ComM_Internal.Channels[i].NmIndicationMask = COMM_NM_INDICATION_NONE;
        ComM_Internal.Channels[i].CommunicationAllowed = FALSE; /** @req COMM884 */
        ComM_Internal.Channels[i].requestPending = FALSE;
        ComM_Internal.Channels[i].EcuMWkUpIndication = FALSE;
#if (COMM_PNC_SUPPORT == STD_ON)
        ComM_Internal.Channels[i].TxComSignal.data = 0; /* Reset all Tx data */
#if (COMM_PNC_GATEWAY_ENABLED == STD_ON)
        ComM_Internal.Channels[i].pnERA.data = 0;
        ComM_Internal.Channels[i].newERARxSignal = FALSE;
#endif
#endif
    }

    for (i = 0; i < COMM_USER_COUNT; ++i) {
        ComM_Internal.Users[i].RequestedMode = COMM_NO_COMMUNICATION;
    }
#if (COMM_PNC_SUPPORT == STD_ON)

    for (i = 0; ((i < ComM_ConfigPtr->ComMPncConfig->ComMPncNum) && (TRUE == ComM_ConfigPtr->ComMPncConfig->ComMPncEnabled)); ++i) {
        /** @req ComM925 */
        /** @req ComM926 */
        /** @req ComM927 */
        ComM_Internal.pncRunTimeData[i].pncState            =   PNC_NO_COMMUNICATION_STATE;
        ComM_Internal.pncRunTimeData[i].pncSubState         =   PNC_NO_COMMUNICATION;
        ComM_Internal.pncRunTimeData[i].pncRequestedState   =   COMM_NO_COMMUNICATION;
        ComM_Internal.pncRunTimeData[i].prepareSleepTimer   =   0;
        ComM_Internal.pncRunTimeData[i].pncNewUserRequest   =   FALSE;
    }
    ComM_Internal.pnEIRA.data                           =   0;
    ComM_Internal.newEIRARxSignal                       =   FALSE;
#endif

    ComM_Internal.InhibitCounter = 0;
    ComM_Internal.InitStatus = COMM_INIT;
    /** @req COMM313 */
}

void ComM_DeInit(){
    /* !req COMM794 */
    COMM_VALIDATE_INIT_NORV(COMM_SERVICEID_DEINIT);

    ComM_Internal.InitStatus = COMM_UNINIT;
}

/* @req COMM872 */
Std_ReturnType ComM_GetState(NetworkHandleType Channel, ComM_StateType *State)
{
    COMM_VALIDATE_INIT(COMM_SERVICEID_GETSTATE);

    const ComM_Internal_ChannelType* ChannelInternal = &ComM_Internal.Channels[Channel];
    *State = ChannelInternal->SubMode;

    return E_OK;
}

Std_ReturnType ComM_GetStatus( ComM_InitStatusType* Status ){
    COMM_VALIDATE_PARAMETER( (Status != NULL), COMM_SERVICEID_GETSTATUS);

    *Status = ComM_Internal.InitStatus;
    return E_OK;
}

Std_ReturnType ComM_GetInhibitionStatus( NetworkHandleType Channel, ComM_InhibitionStatusType* Status ){
    COMM_VALIDATE_INIT(COMM_SERVICEID_GETINHIBITIONSTATUS);

    const ComM_Internal_ChannelType* ChannelInternal = &ComM_Internal.Channels[Channel];
    *Status = ChannelInternal->InhibitionStatus;
    return E_OK;
}


Std_ReturnType ComM_RequestComMode( ComM_UserHandleType User, ComM_ModeType ComMode ){
    COMM_VALIDATE_INIT(COMM_SERVICEID_REQUESTCOMMODE);
    COMM_VALIDATE_USER(User, COMM_SERVICEID_REQUESTCOMMODE);
    /** @req COMM151 */
    COMM_VALIDATE_PARAMETER((ComMode != COMM_SILENT_COMMUNICATION), COMM_SERVICEID_REQUESTCOMMODE);

#if (COMM_PNC_SUPPORT == STD_ON)
    /* Pnc related action is performed first and channel related action later */
    /** @req ComM953 */
    /** @req ComM979 */
    if (TRUE == ComM_ConfigPtr->ComMPncConfig->ComMPncEnabled) {
        ComM_Internal_UpdatePncChangeRequests(User,ComMode);
    }
#endif

    /** @req COMM500 */
    return ComM_Internal_RequestComMode(User, ComMode);

}


/* !req COMM085 */
Std_ReturnType ComM_GetMaxComMode( ComM_UserHandleType User, ComM_ModeType* ComMode ){
    /* !req COMM796  */
    COMM_VALIDATE_INIT(COMM_SERVICEID_GETMAXCOMMODE);
    COMM_VALIDATE_USER(User, COMM_SERVICEID_GETMAXCOMMODE);
    // Not implemented
    //lint -estring(920,pointer)  /* cast to void */
    (void)ComMode;
    //lint +estring(920,pointer)  /* cast to void */
    return E_NOT_OK;
}

Std_ReturnType ComM_GetRequestedComMode( ComM_UserHandleType User, ComM_ModeType* ComMode ){
    COMM_VALIDATE_INIT(COMM_SERVICEID_GETREQUESTEDCOMMODE);
    COMM_VALIDATE_USER(User, COMM_SERVICEID_GETREQUESTEDCOMMODE);

    /** @req COMM797 */
    const ComM_Internal_UserType* UserInternal = &ComM_Internal.Users[User];
    *ComMode = UserInternal->RequestedMode;
    return E_OK;
}

/** @req COMM084 */
Std_ReturnType ComM_GetCurrentComMode( ComM_UserHandleType User, ComM_ModeType* ComMode ){
    COMM_VALIDATE_INIT(COMM_SERVICEID_GETCURRENTCOMMODE);
    COMM_VALIDATE_USER(User, COMM_SERVICEID_GETCURRENTCOMMODE);

    return ComM_Internal_PropagateGetCurrentComMode(User, ComMode);
}

/* !req COMM799 */
Std_ReturnType ComM_PreventWakeUp( NetworkHandleType Channel, boolean Status ){
    COMM_VALIDATE_INIT(COMM_SERVICEID_PREVENTWAKEUP);
    COMM_VALIDATE_CHANNEL(Channel, COMM_SERVICEID_PREVENTWAKEUP);
#if (COMM_MODE_LIMITATION_ENABLED == STD_ON)
    ComM_Internal_ChannelType* ChannelInternal = &ComM_Internal.Channels[Channel];
    if (TRUE == Status) {
        ChannelInternal->InhibitionStatus |=  (COMM_INHIBITION_STATUS_WAKE_UP);
    } else {
        ChannelInternal->InhibitionStatus &= ~(COMM_INHIBITION_STATUS_WAKE_UP);
    }
    return E_OK;
#else
    (void)Status; /* Avoid compiler warning  */
    return E_NOT_OK;
#endif
}

/** !req COMM800.partially */
Std_ReturnType ComM_LimitChannelToNoComMode( NetworkHandleType Channel, boolean Status ){
    COMM_VALIDATE_INIT(COMM_SERVICEID_LIMITCHANNELTONOCOMMODE);
    COMM_VALIDATE_CHANNEL(Channel, COMM_SERVICEID_LIMITCHANNELTONOCOMMODE);
#if (COMM_MODE_LIMITATION_ENABLED == STD_ON)
    const ComM_ChannelType* ChannelConf = &ComM_ConfigPtr->Channels[Channel];
    ComM_Internal_ChannelType* ChannelInternal = &ComM_Internal.Channels[Channel];
    if (TRUE == Status) {
        ChannelInternal->InhibitionStatus |=  (COMM_INHIBITION_STATUS_NO_COMMUNICATION);
    } else {
        ChannelInternal->InhibitionStatus &= ~(COMM_INHIBITION_STATUS_NO_COMMUNICATION);
    }
    return ComM_Internal_UpdateChannelState(ChannelConf, FALSE);
#else
    (void)Status; /* Avoid compiler warning */
    return E_NOT_OK;
#endif
}

/** !req COMM801.partially */
Std_ReturnType ComM_LimitECUToNoComMode( boolean Status ){
    COMM_VALIDATE_INIT(COMM_SERVICEID_LIMITECUTONOCOMMODE);
#if (COMM_MODE_LIMITATION_ENABLED == STD_ON)
    ComM_Internal.NoCommunication = Status;
    uint8 Channel;
    Std_ReturnType totalStatus = E_OK;
    for (Channel = 0; Channel < COMM_CHANNEL_COUNT; Channel++) {
        const ComM_ChannelType* ChannelConf = &ComM_ConfigPtr->Channels[Channel];
        Std_ReturnType status = ComM_Internal_UpdateChannelState(ChannelConf, FALSE);
        if (status > totalStatus) {
            totalStatus = status;
        }
    }
    return totalStatus;
#else
    (void)Status;  /* Avoid compiler warning */
    return E_NOT_OK;
#endif
}

/** @req COMM143  !req COMM802 */
Std_ReturnType ComM_ReadInhibitCounter( uint16* CounterValue ){
    COMM_VALIDATE_INIT(COMM_SERVICEID_READINHIBITCOUNTER);
#if (COMM_MODE_LIMITATION_ENABLED == STD_ON)
    *CounterValue = ComM_Internal.InhibitCounter;
    return E_OK;
#else
    /*lint -e{920} Intentional */
    (void)(CounterValue);
    return E_NOT_OK;
#endif
}

/** !req COMM803 */
Std_ReturnType ComM_ResetInhibitCounter(){
    COMM_VALIDATE_INIT(COMM_SERVICEID_RESETINHIBITCOUNTER);
#if (COMM_MODE_LIMITATION_ENABLED == STD_ON)
    ComM_Internal.InhibitCounter = 0;
    return E_OK;
#else
    return E_NOT_OK;
#endif
}

Std_ReturnType ComM_SetECUGroupClassification( ComM_InhibitionStatusType Status ){
    COMM_VALIDATE_INIT(COMM_SERVICEID_SETECUGROUPCLASSIFICATION);
    // Not implemented
    (void)Status;
    return E_NOT_OK;
}


// Network Management Interface Callbacks
// --------------------------------------
/** @req COMM383 */
void ComM_Nm_NetworkStartIndication( NetworkHandleType Channel ){
    COMM_VALIDATE_INIT_NORV(COMM_SERVICEID_NM_NETWORKSTARTINDICATION);
    COMM_VALIDATE_CHANNEL_NORV(Channel, COMM_SERVICEID_NM_NETWORKSTARTINDICATION);
    const ComM_ChannelType* ChannelConf = &ComM_ConfigPtr->Channels[Channel];
    ComM_Internal_ChannelType* ChannelInternal = &ComM_Internal.Channels[Channel];

    // Used to simulate Wake-up
    ChannelInternal->NmIndicationMask |= COMM_NM_INDICATION_RESTART;
    Std_ReturnType status = ComM_Internal_UpdateChannelState(ChannelConf, FALSE);
    if (status != E_OK) {
        // IMPROVMENT: Add DET error
    }
}

/** @req COMM390 */
void ComM_Nm_NetworkMode( NetworkHandleType Channel ){
    COMM_VALIDATE_INIT_NORV(COMM_SERVICEID_NM_NETWORKMODE);
    COMM_VALIDATE_CHANNEL_NORV(Channel, COMM_SERVICEID_NM_NETWORKMODE);
    const ComM_ChannelType* ChannelConf = &ComM_ConfigPtr->Channels[Channel];
    ComM_Internal_ChannelType* ChannelInternal = &ComM_Internal.Channels[Channel];

    ChannelInternal->NmIndicationMask |= COMM_NM_INDICATION_NETWORK_MODE;
	if(FALSE == ChannelInternal->requestPending){
		Std_ReturnType status = ComM_Internal_UpdateChannelState(ChannelConf, FALSE);
		if (status != E_OK) {
			// IMPROVEMENT: Report DET error?
			}
		ChannelInternal->NmIndicationMask &= ~(COMM_NM_INDICATION_NETWORK_MODE); /* reset for all the situations not remembering the FSM */
	}
}

/** @req COMM391 */
void ComM_Nm_PrepareBusSleepMode( NetworkHandleType Channel ){
    COMM_VALIDATE_INIT_NORV(COMM_SERVICEID_NM_PREPAREBUSSLEEPMODE);
    COMM_VALIDATE_CHANNEL_NORV(Channel, COMM_SERVICEID_NM_PREPAREBUSSLEEPMODE);
    const ComM_ChannelType* ChannelConf = &ComM_ConfigPtr->Channels[Channel];
    ComM_Internal_ChannelType* ChannelInternal = &ComM_Internal.Channels[Channel];

    ChannelInternal->NmIndicationMask |= COMM_NM_INDICATION_PREPARE_BUS_SLEEP;
    Std_ReturnType status = ComM_Internal_UpdateChannelState(ChannelConf, FALSE);
    if (status != E_OK) {
        // IMPROVEMENT: Report DET error?
    }
}

/** @req COMM392 */
void ComM_Nm_BusSleepMode( NetworkHandleType Channel ){
    COMM_VALIDATE_INIT_NORV(COMM_SERVICEID_NM_BUSSLEEPMODE);
    COMM_VALIDATE_CHANNEL_NORV(Channel, COMM_SERVICEID_NM_BUSSLEEPMODE);
    const ComM_ChannelType* ChannelConf = &ComM_ConfigPtr->Channels[Channel];
    ComM_Internal_ChannelType* ChannelInternal = &ComM_Internal.Channels[Channel];

    ChannelInternal->NmIndicationMask |= COMM_NM_INDICATION_BUS_SLEEP;
    Std_ReturnType status = ComM_Internal_UpdateChannelState(ChannelConf, FALSE);
    if (status != E_OK) {
        // IMPROVEMENT: Report DET error?
    }
}

/** @req COMM792 */
void ComM_Nm_RestartIndication( NetworkHandleType Channel ){
    COMM_VALIDATE_INIT_NORV(COMM_SERVICEID_NM_RESTARTINDICATION);
    COMM_VALIDATE_CHANNEL_NORV(Channel, COMM_SERVICEID_NM_RESTARTINDICATION);
    const ComM_ChannelType* ChannelConf = &ComM_ConfigPtr->Channels[Channel];
    ComM_Internal_ChannelType* ChannelInternal = &ComM_Internal.Channels[Channel];

    ChannelInternal->NmIndicationMask |= COMM_NM_INDICATION_RESTART;
    Std_ReturnType status = ComM_Internal_UpdateChannelState(ChannelConf, TRUE);
    if (status != E_OK) {
        // IMPROVEMENT: Report DET error?
    }

}


// ECU State Manager Callbacks
// ---------------------------
/* @req COMM275 */
void ComM_EcuM_WakeUpIndication( NetworkHandleType Channel ){
    /* @req COMM814 */
    COMM_VALIDATE_INIT_NORV(COMM_SERVICEID_ECUM_WAKEUPINDICATION);
    COMM_VALIDATE_CHANNEL_NORV(Channel, COMM_SERVICEID_ECUM_WAKEUPINDICATION);
	if ((ComM_Internal.Channels[Channel].Mode == COMM_NO_COMMUNICATION) ||
		((ComM_Internal.Channels[Channel].SubMode == COMM_SILENT_COM) && /* during silent communication */
	     (ComM_Internal.Channels[Channel].RequestedSubMode == COMM_NO_COM_NO_PENDING_REQUEST))){ /* wake up arrival during a boundary condition where ComM unsync with SM */
		ComM_Internal.Channels[Channel].EcuMWkUpIndication = TRUE;
	}
}


// Diagnostic Communication Manager Callbacks
// ------------------------------------------

void ComM_DCM_ActiveDiagnostic(NetworkHandleType Channel){
    COMM_VALIDATE_INIT_NORV(COMM_SERVICEID_DCM_ACTIVEDIAGNOSTIC);
    COMM_VALIDATE_CHANNEL_NORV(Channel, COMM_SERVICEID_DCM_ACTIVEDIAGNOSTIC);

    const ComM_ChannelType* ChannelConf = &ComM_ConfigPtr->Channels[Channel];
    ComM_Internal.Channels[Channel].DCM_Requested = TRUE;

    /* IMPROVEMENT: Report DET error if returning error */
    (void)ComM_Internal_UpdateChannelState(ChannelConf, FALSE); /** @req COMM866 */
}

void ComM_DCM_InactiveDiagnostic(NetworkHandleType Channel){
    COMM_VALIDATE_INIT_NORV(COMM_SERVICEID_DCM_INACTIVEDIAGNOSTIC);
    COMM_VALIDATE_CHANNEL_NORV(Channel, COMM_SERVICEID_DCM_INACTIVEDIAGNOSTIC);

    const ComM_ChannelType* ChannelConf = &ComM_ConfigPtr->Channels[Channel];
    ComM_Internal.Channels[Channel].DCM_Requested = FALSE;

    /* IMPROVEMENT: Report DET error if returning error */
    (void)ComM_Internal_UpdateChannelState(ChannelConf, FALSE);
}


// Bus State Manager Callbacks
// ---------------------------
/* @req COMM675 */
void ComM_BusSM_ModeIndication( NetworkHandleType Channel, ComM_ModeType *Mode ){
    /* !req COMM288: Should release network on entering NO_COMMUNICATION when Nm variant is FULL.
     * Seems strange since COMM133 says it should be released when entering ready sleep. */
    COMM_VALIDATE_INIT_NORV(COMM_SERVICEID_BUSSM_MODEINDICATION);
    COMM_VALIDATE_CHANNEL_NORV(Channel, COMM_SERVICEID_BUSSM_MODEINDICATION);
    ComM_ModeType ComMode;
    ComMode = *Mode;
    ComM_Internal_ChannelType* ChannelInternal = &ComM_Internal.Channels[Channel];
    const ComM_ChannelType* ChannelConf = &ComM_ConfigPtr->Channels[Channel];

    SchM_Enter_ComM_EA_0();
#if defined(USE_BSWM) || defined(USE_DCM)
    /* !req COMM472 *//* Should report to users */
    if( ChannelInternal->Mode != ComMode) {
#if defined(USE_BSWM)
        BswM_ComM_CurrentMode(Channel, ComMode); /* @req COMM976 */
#endif
#if defined(USE_DCM)
        /* @req COMM266 */
        /* !req COMM693 */
        switch(ComMode)  {
            case COMM_NO_COMMUNICATION:
                Dcm_ComM_NoComModeEntered(Channel);
                break;
            case COMM_SILENT_COMMUNICATION:
                Dcm_ComM_SilentComModeEntered(Channel);
                break;
            case COMM_FULL_COMMUNICATION:
                Dcm_ComM_FullComModeEntered(Channel);
                break;
            default:
                break;
        }
#endif
    }
#endif

    ChannelInternal->Mode = ComMode;
    ChannelInternal->SubMode = ChannelInternal->RequestedSubMode; /** @req COMM898 */ /** @req COMM899 */ /** @req COMM883 */
    ChannelInternal->requestPending = FALSE;

    switch(ComMode)
    {
        case COMM_NO_COMMUNICATION:
            break;
        case COMM_SILENT_COMMUNICATION:
            break;
        case COMM_FULL_COMMUNICATION:
            /* IMPROVMENT: What to do if return error */
            (void)ComM_Internal_NotifyNm(ChannelConf);
            break;
        default:
                /* Do Nothing */
            break;
    }
    if((ChannelInternal->NmIndicationMask & COMM_NM_INDICATION_NETWORK_MODE) == COMM_NM_INDICATION_NETWORK_MODE){
    	/* NetworkMode indication is asynchronous,uninterrupted and critical */
    	Std_ReturnType status = ComM_Internal_UpdateChannelState(ChannelConf, FALSE);
		if (status != E_OK) {
			// IMPROVEMENT: Report DET error?
		}
		ChannelInternal->NmIndicationMask &= ~(COMM_NM_INDICATION_NETWORK_MODE); /* reset for all the situations not remembering the FSM */
    }
    SchM_Exit_ComM_EA_0();
    /*lint -e818 Diclaring Mode as const ComM_ModeType* will cause deviation in ASR API prototype */
}




// Scheduled main function
// -----------------------

// Prototype right here because this function should not be exposed
void ComM_MainFunction(NetworkHandleType Channel);

/** @req COMM429 */
void ComM_MainFunction(NetworkHandleType Channel){

    const ComM_ChannelType* ChannelConf = &ComM_ConfigPtr->Channels[Channel];
    ComM_Internal_ChannelType* ChannelInternal = &ComM_Internal.Channels[Channel];


#if (COMM_PNC_SUPPORT == STD_ON)
    /** @req ComM978 */
    /** @req ComM979 */
    /** @req ComM953 */
    SchM_Enter_ComM_EA_0(); /* Disable interrupts  (Required for no new newEIRARxSignal) */
    ComM_PncStateTrans_ThisCycle = FALSE;

    if(TRUE == ComM_ConfigPtr->ComMPncConfig->ComMPncEnabled) {
         ComM_Internal_TickPncPrepareSleepTime(ChannelConf); /* Tick Prepare sleep timer */

        /** @req ComM987 */
        ComM_Internal_ProcessPNReqMode(Channel); /* User requesting PN */

#if (COMM_PNC_GATEWAY_ENABLED == STD_ON)
        if (TRUE == ChannelInternal->newERARxSignal) {
            ComM_Internal_ERARxSignalUpdate(Channel); /* new ERA received */
        }

        ComM_Internal_Monitor_GwPassiveChnl(); /*Monitor Passive Gateway channels when PNC in PNC_REQUESTED state */
#endif
        if (TRUE == ComM_Internal.newEIRARxSignal) {
            ComM_Internal_EIRARxSignalUpdate();	/* new EIRA received */
        }
        if (ComM_PncStateTrans_ThisCycle != TRUE) { /* To avoid multiple transition in one mainf function */
            ComM_Internal_Monitor_RdySlpToPrpSlpTrns(); /* Monitor ready sleep to prepare sleep transitions*/
        }
    }
    SchM_Exit_ComM_EA_0(); /* Enable interrupts */

#endif

    if ((ChannelConf->NmVariant == COMM_NM_VARIANT_NONE) ||
        (ChannelConf->NmVariant == COMM_NM_VARIANT_LIGHT)) {
        Std_ReturnType status = ComM_Internal_TickFullComMinTime(ChannelConf, ChannelInternal);
        if (status != E_OK) {
            // IMPROVEMENT: Report DET error?
        }
    }
    if (ChannelConf->NmVariant == COMM_NM_VARIANT_LIGHT) {
        Std_ReturnType status = ComM_Internal_TickLightTime(ChannelConf, ChannelInternal);
        if (status != E_OK) {
            // IMPROVEMENT: Report DET error?
        }
    }

    /* If wake up indication is set by EcuM */
    if (ChannelInternal->EcuMWkUpIndication == TRUE) {
		if ((ChannelInternal->SubMode == COMM_NO_COM_NO_PENDING_REQUEST ) && (ChannelInternal->requestPending == FALSE)){
			ChannelInternal->SubMode = COMM_NO_COM_REQUEST_PENDING; /* @req ComM893 */
		}
	}

    if ((ChannelInternal->Mode == COMM_NO_COMMUNICATION) &&
		(ChannelInternal->SubMode == COMM_NO_COM_REQUEST_PENDING) ){
			Std_ReturnType status = ComM_Internal_UpdateChannelState(ChannelConf, FALSE);
			if (status != E_OK) {
				// IMPROVEMENT: Report DET error?
			}
	}
}


void ComM_CommunicationAllowed( NetworkHandleType Channel, boolean Allowed) {
    COMM_VALIDATE_INIT_NORV(COMM_SERVICEID_COMMUNICATIONALLOWED);
    COMM_VALIDATE_CHANNEL_NORV(Channel, COMM_SERVICEID_COMMUNICATIONALLOWED);

    ComM_Internal_ChannelType* ChannelInternal = &ComM_Internal.Channels[Channel];
    ChannelInternal->CommunicationAllowed = Allowed; /** @req COMM885 */
}
