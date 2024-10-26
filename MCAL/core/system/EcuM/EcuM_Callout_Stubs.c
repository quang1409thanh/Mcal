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

/* @req EcuM2990*/
/* @req EcuMf2990*/

/* ----------------------------[includes]------------------------------------*/
//lint -emacro(9036,VALIDATE_STATE)

#include "EcuM.h"
#include "EcuM_Generated_Types.h"
#if defined(USE_DET)
#include "Det.h"
#endif
#if defined(USE_DEM)
#include "Dem.h"
#endif
#if defined(USE_MCU)
#include "Mcu.h"
#endif
#if defined(USE_GPT)
#include "Gpt.h"
#endif
#if defined(USE_CAN)
#include "Can.h"
#endif
#if defined(USE_FR)
#include "Fr.h"
#endif
#if defined(USE_DIO)
#include "Dio.h"
#endif
#if defined(USE_CANIF)
#include "CanIf.h"
#endif
#if defined(USE_XCP)
#include "Xcp.h"
#endif
#if defined(USE_PDUR)
#include "PduR.h"
#endif
#if defined(USE_COM)
#include "Com.h"
#endif
#if defined(USE_CANTP)
#include "CanTp.h"
#endif
#if defined(USE_FRTP)
#include "FrTp.h"
#endif
#if defined(USE_J1939TP)
#include "J1939Tp.h"
#endif
#if defined(USE_TCPIP)
#include "TcpIp.h"
#endif
#if defined(USE_SD)
#include "SD.h"
#endif
#if defined(USE_SOAD)
#include "SoAd.h"
#endif
#if defined(USE_LDCOM)
#include "LdCom.h"
#endif

#if defined(USE_DCM)
#include "Dcm.h"
#endif
#if defined(USE_PWM)
#include "Pwm.h"
#endif
#if defined (USE_OCU)
#include "Ocu.h"
#endif
#if defined (USE_ICU)
#include "Icu.h"
#endif
#if defined(USE_IOHWAB)
#include "IoHwAb.h"
#endif
#if defined(USE_FLS)
#include "Fls.h"
#endif
#if defined(USE_EEP)
#include "Eep.h"
#endif
#if defined(USE_FEE)
#include "Fee.h"
#endif
#if defined(USE_EA)
#include "Ea.h"
#endif
#if defined(USE_ETHIF)
#include "EthIf.h"
#endif
#if defined(USE_ETH)
#include "Eth.h"
#endif
#if defined(USE_NVM)
#include "NvM.h"
#endif
#if defined(USE_COMM)
#include "ComM.h"
#endif
#if defined(USE_NM)
#include "Nm.h"
#endif
#if defined(USE_CANNM)
#include "CanNm.h"
#endif
#if defined(USE_FRNM)
#include "FrNm.h"
#endif
#if defined(USE_CANSM)
#include "CanSM.h"
#endif
#if defined(USE_FRSM)
#include "FrSM.h"
#endif
#if defined(USE_UDPNM)
#include "UdpNm.h"
#endif
#if defined(USE_LINSM)
#include "LinSM.h"
#endif
#if defined(USE_ETHIF)
#include "EthIf.h"
#endif
#if defined(USE_ETHSM)
#include "EthSM.h"
#endif
#if defined(USE_ETHTSYN)
#include "EthTSyn.h"
#endif
#if defined(USE_STBM)
#include "StbM.h"
#endif
#if defined(USE_SPI)
#include "Spi.h"
#endif
#if defined(USE_WDG)
#include "Wdg.h"
#endif
#if defined(USE_WDGM)
#include "WdgM.h"
#endif
#if defined(USE_BSWM)
#include "BswM.h"
#endif
#if defined(USE_STBM)
#include "StbM.h"
#endif
#if defined(USE_DLT)
#include "Dlt.h"
#endif
#if defined(USE_SOMEIPXF)
#include "SomeIpXf.h"
#endif

#if defined(USE_E2EXF)
#include "E2EXf.h"
#endif

#if defined(USE_SECOC)
#include "SecOC.h"
#endif

#if defined(CFG_SHELL)
#include "shell.h"
#endif

#if defined(CFG_T1_ENABLE)
#include "Arc_T1_int.h"
#endif

#include "isr.h"

/* ----------------------------[private define]------------------------------*/
/* Does not run functions that are not drivers */
#define NO_DRIVER(_func)  \
  if( (EcuM_DriverRestart==FALSE) ) { \
    _func; \
 }

#if defined(USE_ECUM_FIXED)
#define VALIDATE_STATE(_state) \
    do { \
        EcuM_StateType ecuMState;  \
        if (E_OK != EcuM_GetState(&ecuMState)) { \
            ASSERT(0); \
        } \
        ASSERT(ecuMState == (_state) ); \
    } while(0)
#else
#define VALIDATE_STATE(_state)
#endif

/* ----------------------------[private macro]-------------------------------*/

/* ----------------------------[private typedef]-----------------------------*/
/* ----------------------------[private function prototypes]-----------------*/
/* ----------------------------[private variables]---------------------------*/
/*lint --e{843} this is only info  */
static boolean EcuM_DriverRestart = FALSE;
/* ----------------------------[private functions]---------------------------*/
/* ----------------------------[public functions]----------------------------*/


/**
 * Called if there is something very wrong.
 *
 * Checks for ECUM_E_RAM_CHECK_FAILED (bad hash at wakeup) or
 * ECUM_E_CONFIGURATION_DATA_INCONSISTENT (bad post-build data)
 *
 * This function should NEVER return
 *
 * @param reason
 */
void EcuM_ErrorHook(uint16 reason) {
    /* @req 4.0.3|3.1.5/EcuM2904  */

	/* Go to defined state Reset, Fail-Safe etc. */
    (void)reason;
#if (MCU_PERFORM_RESET_API == STD_ON)
    Mcu_PerformReset();
#else
    ASSERT(0);
#endif

#if defined(USE_DET)
#endif
}

extern const EcuM_ConfigType EcuMConfig;

//lint -esym(522, _dummy) MISRA exception. _dummy is used to prevent other false positive warnings.
static void _dummy( void ) {

}

struct EcuM_ConfigS* EcuM_DeterminePbConfiguration(void) {
	NO_DRIVER(_dummy());	// Keep compiler silent
	return (EcuM_ConfigType*)&EcuMConfig; /*lint !e9005 !e929*/
}
#if defined(ECUM_BACKWARD_COMPATIBLE)
/**
 * First callout from EcuM_Init(). May contain
 * all kind of initialization code.
 *
 * Got here by main()->EcuM_Init()
 *
 * Part of STARTUP I
 */
void EcuM_AL_DriverInitZero(void)
{
//	VALIDATE_STATE( ECUM_STATE_STARTUP_ONE );
#if defined(CFG_T1_ENABLE)
    Arc_T1_Init();
#endif

#if defined(USE_DET)
    Det_Init();  /** @req EcuM2783 */ /** @req EcuMf2783 */
    Det_Start(); /** @req EcuM2634 *//** @req EcuMf2634 */
#endif
}

/**
 * Part of STARTUP I
 *
 * @param ConfigPtr
 */
void EcuM_AL_DriverInitOne(const EcuM_ConfigType *ConfigPtr)
{
    (void)ConfigPtr; //lint !e920 MISRA False positive. Allowed to cast pointer to void here.

#if defined(USE_MCU)
    Mcu_Init(ConfigPtr->McuConfigPtr);

    /* Set up default clock (Mcu_InitClock requires initRun==1) */
    /* Ignoring return value */
    (void) Mcu_InitClock(ConfigPtr->McuConfigPtr->McuDefaultClockSettings);

    // Wait for PLL to sync.
    while (Mcu_GetPllStatus() != MCU_PLL_LOCKED) {
        ;
    }

    /* Ignoring return value. IMPROVEMENT Handle return value. */
    (void) Mcu_DistributePllClock();
#endif

#if defined(USE_DEM)
    // Preinitialize DEM
    NO_DRIVER(Dem_PreInit(ConfigPtr->DemConfigPtr));
#endif

#if defined(USE_PORT)
    // Setup Port
    Port_Init(ConfigPtr->PortConfigPtr);
#endif

#if defined(USE_DIO)
    // Setup Dio
    Dio_Init(ConfigPtr->DioConfigPtr);
#endif

#if defined(USE_GPT)
    // Setup the GPT
    Gpt_Init(ConfigPtr->GptConfigPtr);
#endif

    // Setup watchdog
#if defined(USE_WDG)
    Wdg_Init(ConfigPtr->WdgConfigPtr);
#endif
#if defined(USE_WDGM)
    NO_DRIVER(WdgM_Init(ConfigPtr->WdgMConfigPtr));
#endif

#if defined(USE_DMA)
    // Setup DMA
    Dma_Init(ConfigPtr->DmaConfigPtr);
#endif

#if defined(USE_ADC)
    // Setup ADC
    Adc_Init(ConfigPtr->AdcConfigPtr);
#endif

#if defined(USE_BSWM)
    // Setup BSWM
    BswM_Init(ConfigPtr->BswMConfigPtr);
#endif

#if defined(USE_STBM)
    // Setup STBM /* @req SWS_StbM_00250 */
    StbM_Init(NULL_PTR);
#endif
    // Setup ICU
#if defined(USE_ICU)
     Icu_Init(ConfigPtr->IcuConfigPtr);
#endif

    // Setup PWM
#if defined(USE_PWM)
    // Setup PWM
    Pwm_Init(ConfigPtr->PwmConfigPtr);
#endif

    // Setup OCU
#if defined(USE_OCU)
     Ocu_Init(ConfigPtr->OcuConfigPtr);
#endif

#if defined(CFG_SHELL)
    SHELL_Init();
#endif
}
#endif

#if defined(USE_ECUM_FIXED)
/**
 * At this point OS and essential drivers have started.
 * Start the rest of the drivers
 *
 *
 * Part of STARTUP II
 *
 * @param ConfigPtr
 */
//lint -esym(522, EcuM_AL_DriverInitTwo) MISRA exception. Prevent false positive if none of the modules are used
void EcuM_AL_DriverInitTwo(const EcuM_ConfigType* ConfigPtr)
{
    (void)ConfigPtr; //lint !e920 MISRA False positive. Allowed to cast pointer to void here.
#if defined(USE_SPI)
    Spi_Init(ConfigPtr->SpiConfigPtr);  // Setup SPI
#endif
#if defined(USE_EEP)
    NO_DRIVER(Eep_Init(ConfigPtr->EepConfigPtr));   // Setup EEP
#endif
#if defined(USE_FLS)
    NO_DRIVER(Fls_Init(ConfigPtr->FlsConfigPtr));   // Setup Flash
#endif
#if defined(USE_FEE)
    NO_DRIVER(Fee_Init());  // Setup FEE
#endif
#if defined(USE_EA)
    NO_DRIVER(Ea_Init());   // Setup EA
#endif
#if defined(USE_NVM)
    NO_DRIVER(NvM_Init());  // Setup NVRAM Manager and start the read all job
    NO_DRIVER(NvM_ReadAll());
#endif
#if defined(USE_LIN)
    Lin_Init(ConfigPtr->LinConfigPtr);    // Setup Lin driver
#endif
#if defined(USE_LINIF)
    LinIf_Init(ConfigPtr->LinIfConfigPtr);    // Setup LinIf
#endif
#if defined(USE_LINSM)
    LinSM_Init(ConfigPtr->LinSMConfigPtr);    // Setup LinSM
#endif

#if defined(USE_CANTRCV)
    // Setup Can transceiver driver
    CanTrcv_Init(ConfigPtr->CanTrcvConfigPtr);
#endif

#if defined(USE_CAN)
    Can_Init(ConfigPtr->CanConfigPtr);  // Setup Can driver
#endif
#if defined(USE_FR)
    NO_DRIVER(Fr_Init(ConfigPtr->FrConfigPtr));    // Setup Flexray CC driver
#endif
#if defined(USE_CANIF)
    NO_DRIVER(CanIf_Init(ConfigPtr->PostBuildConfig->CanIfConfigPtr));  // Setup CanIf
#endif
#if defined(USE_FRIF)
    NO_DRIVER(FrIf_Init(ConfigPtr->FrIfConfigPtr));    // Setup Flexray Interface
#endif
#if defined(USE_CANTP)
    NO_DRIVER(CanTp_Init(ConfigPtr->PostBuildConfig->CanTpConfigPtr));  // Setup CAN TP
#endif
#if defined(USE_FRTP)
    NO_DRIVER(FrTp_Init(ConfigPtr->FrTpConfigPtr));    // Setup Flexray TP
#endif
#if defined(USE_CANSM)
    NO_DRIVER(CanSM_Init(ConfigPtr->CanSMConfigPtr));
#endif
#if defined(USE_FRSM)
    NO_DRIVER(FrSM_Init(ConfigPtr->FrSMConfigPtr));    // Setup Flexray SM
#endif
#if defined(USE_ETHIF)
    // Setup EthIf before Eth & EthTrcv
    NO_DRIVER(EthIf_Init(ConfigPtr->EthIfConfigPtr));
#endif
#if defined(USE_ETH)
    Eth_Init(ConfigPtr->EthConfigPtr);  // Setup Eth
#endif
#if defined(USE_ETHIF)
    NO_DRIVER(EthIf_Init(ConfigPtr->EthIfConfigPtr));   //  Setup Eth If
#endif
#if defined(USE_ETHSM)
    NO_DRIVER(EthSM_Init());    //  Setup Eth SM
#endif
#if defined(USE_TCPIP)
    NO_DRIVER(TcpIp_Init(ConfigPtr->TcpIpConfigPtr));   // Setup Tcp Ip
#endif
#if defined(USE_J1939TP)
    NO_DRIVER(J1939Tp_Init(ConfigPtr->J1939TpConfigPtr));   // Setup J1939Tp
#endif
#if defined(USE_SOAD)
    NO_DRIVER(SoAd_Init(ConfigPtr->SoAdConfigPtr)); // Setup Socket Adaptor
#endif
#if defined(USE_SD)
    NO_DRIVER(Sd_Init(ConfigPtr->SdConfigPtr)); // Setup Service Discovery
#endif
#if defined(USE_LDCOM)
    NO_DRIVER(LdCom_Init(ConfigPtr->LdComConfigPtr));   // Setup Large Data Com
#endif
#if defined(USE_PDUR)
    NO_DRIVER(PduR_Init(ConfigPtr->PostBuildConfig->PduRConfigPtr));    // Setup PDU Router
#endif
#if defined(USE_OSEKNM)
    NO_DRIVER(OsekNm_Init(ConfigPtr->OsekNmConfigPtr));    // Setup Osek Network Manager
#endif
#if defined(USE_CANNM)
    NO_DRIVER(CanNm_Init(ConfigPtr->PostBuildConfig->CanNmConfigPtr));    // Setup Can Network Manager
#endif
#if defined(USE_FRNM)
    NO_DRIVER(FrNm_Init(ConfigPtr->FrNmConfigPtr));    // Setup Flexray Network Manager
#endif
#if defined(USE_UDPNM)
    NO_DRIVER(UdpNm_Init(ConfigPtr->UdpNmConfigPtr));        // Setup Udp Network Manager
#endif
#if defined(USE_NM)
    NO_DRIVER(Nm_Init());        // Setup Network Management Interface
#endif
#if defined(USE_COM)
    NO_DRIVER(Com_Init(ConfigPtr->PostBuildConfig->ComConfigPtr));  // Setup COM layer
#endif
#if defined(USE_DCM)
    NO_DRIVER(Dcm_Init(ConfigPtr->DcmConfigPtr));   // Setup DCM
#endif
#if defined(USE_IOHWAB)
    IoHwAb_Init();  // Setup IO hardware abstraction layer
#endif
#if defined(USE_XCP)
    NO_DRIVER(Xcp_Init(ConfigPtr->XcpConfigPtr));   // Setup XCP
#endif
#if defined(USE_ETHTSYN)
    // Setup EthTSyn
    NO_DRIVER(EthTSyn_Init(ConfigPtr->EthTSynConfigPtr));
#endif
#if defined(USE_STBM)
    // Setup StbM
    NO_DRIVER(StbM_Init(ConfigPtr->StbMConfigpPtr));
#endif
}


/**
 * Part of STARTUP II
 *
 * @param ConfigPtr
 */
void EcuM_AL_DriverInitThree(const EcuM_ConfigType* ConfigPtr)
{
    (void)ConfigPtr; //lint !e920 MISRA False positive. Allowed to cast pointer to void here.

#if defined(USE_DEM)
    // Setup DEM
    Dem_Init();
#endif

#if defined(USE_DLT)
    Dlt_Init(ConfigPtr->DltConfigPtr); /* Needs to be done after nvram has been initialised */
#endif

#if defined(USE_COMM)
    // Setup Communication Manager
    ComM_Init(ConfigPtr->ComMConfigPtr);
#endif

#if defined(USE_SOMEIPXF)
    // Setup SomeIp Transformer
    SomeIpXf_Init(NULL);
#endif

#if defined(USE_E2EXF)
    // Setup E2E Transformer
    E2EXf_Init(NULL);
#endif

#if defined (USE_SECOC)
    SecOC_Init(ConfigPtr->SecOCConfigPtr);
#endif

}

void EcuM_OnEnterRun(void)
{
#if defined(CFG_ECUM_CS_SETEVENT)
    SetEvent(EVENT_MASK_Event_APP_RUN,TASK_ID_Application);
#endif
}

void EcuM_OnExitRun(void)
{

}

void EcuM_OnExitPostRun(void)
{

}

void EcuM_OnPrepShutdown(void)
{

}

void EcuM_OnGoSleep(void)
{

}
#endif

boolean EcuM_LoopDetection(void)
{
    //IMPROVEMENT: How is reset loop detection implemented?
    return FALSE;
}

void EcuM_OnGoOffOne(void)
{

}

void EcuM_OnGoOffTwo(void)
{

}



/**
 * This function should be the last this called before reset.
 * Normally this manipulates some pin that controls DC to power off
 * the board (ie it will never get to Mcu_PerformReset())
 */
void EcuM_AL_SwitchOff(void)
{

    /* ADD CODE BELOW */
#if 0
    /* Example */
    Dio_WriteChannel(DIO_CHANNEL_NAME_MY_POWER,STD_LOW);
#endif

#if defined(USE_MCU) && (MCU_PERFORM_RESET_API == STD_ON)
    Mcu_PerformReset();
#endif
}

#if (defined(USE_ECUM_FLEXIBLE))
void EcuM_AL_Reset(EcuM_ResetType reset)
{
    // IMPROVEMENT: Perform reset based on the reset parameter
#if (MCU_PERFORM_RESET_API == STD_ON)
    Mcu_PerformReset();
#else
    for(;;)
    {
        ;
    }
#endif
}
#endif

/**
 *
 * Called to check other wakeup sources. Assume for example that
 * we want to check something else than the actual wakeup pin.
 *
 * Can be called from interrupt context.
 * @param wakeupSource
 */

void EcuM_CheckWakeup(EcuM_WakeupSourceType source) {

#if defined(USE_MCU)
    /* Re-enable PLL again */
    const EcuM_ConfigType *ecuMConfigPtr;
    ecuMConfigPtr = EcuM_DeterminePbConfiguration();
    (void) Mcu_InitClock(ecuMConfigPtr->McuConfigPtr->McuDefaultClockSettings);
    //Wait for PLL to sync.
    while (Mcu_GetPllStatus() != MCU_PLL_LOCKED) {
        ;
    }
#endif
    /* ADD CODE BELOW */
#if 0
    /* Example */
    if (ECUM_WKSOURCE_SWITCH & wakeupSource) {
        if (CRP.PSCR.R & 0x00020000) {
            EcuM_SetWakeupEvent(ECUM_WKSOURCE_SWITCH);
        }
    }
#endif
    (void)source; /* added for lint exception*/
}


/**
 * Enable wakeup sources that should wake us. This is when going to sleep.
 *
 * This function is called once for each wakeup source.(ie source
 * can only have one bit set at any time). See ECUM2389 for example.
 *
 * @param source
 */
void EcuM_EnableWakeupSources( EcuM_WakeupSourceType source ) {

 (void)source; /* Added for lint exception*/
    /* ADD CODE BELOW */
}

/**
 *
 * @param wakeupSource
 */
void EcuM_DisableWakeupSources(EcuM_WakeupSourceType wakeupSource) {

    (void)wakeupSource; /* Added for lint exception*/
	/* ADD CODE BELOW */
}


/**
 * Start wakeup sources that need validation.
 *
 * 1. For wakeup sources that don't need validation nothing needs to be done.
 * 2. For wakeup sources that may have false wakeup, start then up, e.g. CanIf
 *
 * Called once when entering ECUM_STATE_WAKEUP_VALIDATION state
 *
 * @param wakeupSource
 */

void EcuM_StartWakeupSources(EcuM_WakeupSourceType wakeupSource) {
	VALIDATE_STATE( ECUM_STATE_WAKEUP_VALIDATION );
	 (void)wakeupSource; /* Added for lint exception*/
	/* ADD CODE BELOW */
}

/**
 * Stop not validated events
 *
 * @param wakeupSource
 */
void EcuM_StopWakeupSources(EcuM_WakeupSourceType wakeupSource)
{
    VALIDATE_STATE( ECUM_STATE_WAKEUP_VALIDATION);
    (void)wakeupSource;
}

/**
 *
 * @param wakeupSource
 */
void EcuM_CheckValidation(EcuM_WakeupSourceType wakeupSource) {

    VALIDATE_STATE( ECUM_STATE_WAKEUP_VALIDATION);

    /* ADD CODE BELOW */

    (void) wakeupSource;
}

/**
 * Restart drivers. Have no restart list, instead the
 * drivers restarted are drivers that have had it's hardware
 * registers reset.  If memory is also lost (=all memory is not
 * powered during sleep), this strategy does not work.
 *
 * This calls:
 * - EcuM_AL_DriverInitOne()
 * - EcuM_AL_DriverInitTwo()
 *
 */
#if defined(ECUM_BACKWARD_COMPATIBLE)
void EcuM_AL_DriverRestart(const struct EcuM_ConfigS* ConfigPtr) {
	const EcuM_ConfigType* config;
    (void) *ConfigPtr;
	VALIDATE_STATE( ECUM_STATE_WAKEUP_ONE);

    config = EcuM_DeterminePbConfiguration();

    /* Start all drivers for now */
    EcuM_AL_DriverInitOne(config);

    /* Setup the systick interrupt */
#if defined(USE_MCU)
    {
#if defined(CFG_OS_SYSTICK2)
        Os_SysTickStart2(OsTickFreq);
#else
        Os_SysTickStart(Mcu_Arc_GetSystemClock() / (uint32_t)OsTickFreq);
#endif

    }
#endif
#if defined(USE_ECUM_FIXED)
    EcuM_AL_DriverInitTwo(config);
#endif
}
#endif

/**
 * Called once validation is done and a may change the wakeup at a very
 * late stage.
 *
 * For example if a wakeup event have been verified (=ECUM_WKACT_RUN) but here
 * a switch or something indicating immediate shutdown the reaction
 * can be changed here from ECUM_WKACT_RUN to ECUM_WKACT_SHUTDOWN.
 * This makes would make EcuM to go to
 * ECUM_STATE_PREP_SHUTDOWN(ECUM_STATE_WAKEUP_WAKESLEEP) instead of
 * ECUM_STATE_WAKEUP_TWO
 *
 * @param wact
 * @return
 */
EcuM_WakeupReactionType EcuM_OnWakeupReaction( EcuM_WakeupReactionType wact ) {

    VALIDATE_STATE( ECUM_STATE_WAKEUP_REACTION );

    /* ADD CODE BELOW */
    return wact;
}


/**
 * Generate RAM hash.
 * We are in ECUM_STATE_SLEEP here.
 *
 */
void EcuM_GenerateRamHash(void)
{
    /* De-init drivers.
     * There is really no suiteable place to do this but here
     */
    VALIDATE_STATE( ECUM_STATE_SLEEP );
     
#if defined(USE_GPT) && (GPT_DEINIT_API == STD_ON)
    Gpt_DeInit();
#endif
#if defined(USE_ADC) && (ADC_DEINIT_API == STD_ON)
    Adc_DeInit();
#endif
#if defined(USE_PWM) && (PWM_DE_INIT_API == STD_ON)
    Pwm_DeInit();
#endif
#if defined(USE_SPI)
    Spi_DeInit();
#endif
}

/**
 * Check RAM hash.
 * We are still in ECUM_STATE_SLEEP here.
 *
 * @return
 */
uint8 EcuM_CheckRamHash( void ) {
    VALIDATE_STATE( ECUM_STATE_SLEEP );

    /* ADD CODE BELOW */

    return 0;
}

/**
 * This callout is invoked periodically in all reduced clock sleep modes.
 * It is explicitely allowed to poll wakeup sources from this callout and to call wakeup
 * notification functions to indicate the end of the sleep state to the ECU State Manager.
 */
void EcuM_SleepActivity(void)
{
    /*
     * Note: If called from the Poll sequence the EcuMcalls this callout functions in a
     * blocking loop at maximum frequency. The callout implementation must ensure by
     * other means if callout code shall be executed with a lower period. The integrator may
     * choose any method to control this, e.g. with the help of OS counters, OS alarms,
     * or Gpt timers
     */
    /* ADD CODE BELOW */
}

/**
 * This ARC definition is called when mcu definition of enum Mcu_ResetType has additional enum values than standard Autosar definition.
 * Integrator has to map the additional enum value with the appropriate wakeup source and provide code for remembering the reset reason here by calling
 * EcuM_ValidateWakeupEvent(wakeup source) as per requirement EcuMf2623 .
 *
 * @param resetReason
 *
 *  */
void EcuM_Arc_RememberWakeupEvent(uint32 resetReason)
{
    (void)resetReason;

}

#if defined(CFG_ARC_ECUM_NVM_READ_INIT)
/**
 * Used to quickly let the fee / ea module read the admin block to enter idle state.
 *
 * Is enabled by setting "CFG += ARC_ECUM_NVM_READ_INIT" in proj build_config.mk.
 * @param tickTimerStart
 * @param EcuMNvramReadAllTimeout
 */
void EcuM_Arc_InitFeeReadAdminBlock(TickType tickTimerStart, uint32 EcuMNvramReadAllTimeout) {
    MemIf_StatusType status = MEMIF_IDLE;
    TickType tickTimerElapsed;
    TickType curr;

    /* Let it execute until it's done with reading all admin blocks */
    do
    {
        curr = GetOsTick();
        tickTimerElapsed = OS_TICKS2MS_OS_TICK(curr - tickTimerStart);

#if defined(USE_FEE)
        Fee_MainFunction();
#endif
#if defined(USE_FLS)
        Fls_MainFunction();
#endif

#if defined(USE_FEE)
        status = Fee_GetStatus();
#endif

    } while ( (MEMIF_BUSY_INTERNAL == status) &&  (tickTimerElapsed < EcuMNvramReadAllTimeout));
}

/**
 * Calls the memory main functions to quickly finish the NvM_ReadAll job.
 */
void EcuM_Arc_InitMemReadAllMains(void) {
#if defined(USE_NVM)
    NvM_MainFunction();
#endif
#if defined(USE_FEE)
    Fee_MainFunction();
#endif
#if defined(USE_FLS)
    Fls_MainFunction();
#endif
#if defined(USE_EA)
    Ea_MainFunction();
#endif
#if defined(USE_EEP)
    Eep_MainFunction();
#if defined(USE_SPI)
    Spi_MainFunction_Handling();
#endif
#endif
}
#endif
