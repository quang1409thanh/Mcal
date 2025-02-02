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

/** @tagSettings DEFAULT_ARCHITECTURE=NOT_USED */

#include "arc_assert.h"
#include <stdlib.h>
#include "Mcu.h"
#include "Adc.h"
#if defined(USE_DET)
#include "Det.h"
#endif
#include "Os.h"
#include "isr.h"
#include "regs.h"
#include "arc.h"
#include "Adc_Internal.h"


// ATDCTL2
#define BM_ADPU 0x80
#define BM_AFFC 0x40
#define BM_AWAI 0x20
#define BM_ETRIGLE 0x10
#define BM_ETRIGP 0x08
#define BM_ETRIG_E 0x04
#define BM_ASCIE 0x02
#define BM_ASCIF 0x01

// ATDCTL3
#define BM_S8C 0x40
#define BM_S4C 0x20
#define BM_S2C 0x10
#define BM_S1C 0x08
#define BM_FIFO 0x04
#define BM_FRZ1 0x02
#define BM_FRZ0 0x01

// ATDCTL4
#define BM_SRES8 0x80
#define BM_SMP1 0x40
#define BM_SMP0 0x20
#define BM_PRS4 0x10
#define BM_PRS3 0x08
#define BM_PRS2 0x04
#define BM_PRS1 0x02
#define BM_PRS0 0x01

// ATDCTL5
#define BM_DJM 0x80
#define BM_DSGN 0x40
#define BM_SCAN 0x20
#define BM_MULT 0x10
#define BM_CC 0x04
#define BM_CB 0x02
#define BM_CA 0x01

/* Development error macros. */
#if ( ADC_DEV_ERROR_DETECT == STD_ON )
#define VALIDATE(_exp,_api,_err ) \
        if( !(_exp) ) { \
          Det_ReportError(ADC_MODULE_ID,0,_api,_err); \
          return; \
        }

#define VALIDATE_W_RV(_exp,_api,_err,_rv ) \
        if( !(_exp) ) { \
          Det_ReportError(ADC_MODULE_ID,0,_api,_err); \
          return (_rv); \
        }
#else
#define VALIDATE(_exp,_api,_err )
#define VALIDATE_W_RV(_exp,_api,_err,_rv )
#endif

/* Function prototypes. */
ISR(Adc_GroupConversionComplete);

static Adc_StateType adcState = ADC_STATE_UNINIT;

/* Pointer to configuration structure. */
static const Adc_ConfigType *AdcConfigPtr;


#if (ADC_DEINIT_API == STD_ON)
void Adc_DeInit (void)
{
    if (E_OK == Adc_CheckDeInit(adcState, AdcConfigPtr))
    {
        /* @req SWS_Adc_00110 */
        /* Clean internal status. */
        AdcConfigPtr = (Adc_ConfigType *)NULL;
        adcState = ADC_STATE_UNINIT;
    }
}
#endif

void Adc_Init (const Adc_ConfigType *ConfigPtr)
{
  Adc_GroupType group;
  /* @req SWS_Adc_00342 */
  if (E_OK == Adc_CheckInit(adcState, ConfigPtr))
  {
    /* First of all, store the location of the configuration data. */
    AdcConfigPtr = ConfigPtr;

    // Connect interrupt to correct isr
    ISR_INSTALL_ISR2("ADC",Adc_GroupConversionComplete,IRQ_TYPE_ATD0,6,0);


    ATD0CTL2   = BM_ADPU | BM_AFFC | BM_ASCIE;	/* power enable, Fast Flag Clear, irq enable*/
    ATD0CTL3   = 0x03;	/* 8 conversions per sequence default, freeze enable */

    ATD0CTL4   = (ConfigPtr->hwConfigPtr->resolution << 7) |
                 (ConfigPtr->hwConfigPtr->convTime << 5) |
                  ConfigPtr->hwConfigPtr->adcPrescale;

    for (group = 0; group < ADC_NBR_OF_GROUPS; group++)
    {
      /* @req  SWS_Adc_00307 */
      ConfigPtr->groupConfigPtr[group].status->groupStatus = ADC_IDLE;
    }

    /* Move on to INIT state. */
    adcState = ADC_STATE_INIT;
  }
}

Std_ReturnType Adc_SetupResultBuffer (Adc_GroupType group, Adc_ValueGroupType *bufferPtr)
{
  Std_ReturnType returnValue;

  /* Check for development errors. */
  if (E_OK == Adc_CheckSetupResultBuffer (adcState, AdcConfigPtr, group, bufferPtr))
  {

      /* @req SWS_Adc_00420 */
    AdcConfigPtr->groupConfigPtr[group].status->resultBufferPtr = bufferPtr;
    returnValue = E_OK;
  }
  else
  {
    /* An error have been raised from Adc_CheckSetupResultBuffer(). */
    returnValue = E_NOT_OK;
  }

  return (returnValue);
}

#if (ADC_READ_GROUP_API == STD_ON)
Std_ReturnType Adc_ReadGroup (Adc_GroupType group, Adc_ValueGroupType *dataBufferPtr)
{
  Std_ReturnType returnValue;
  Adc_ChannelType channel;

  if ((E_OK == Adc_CheckReadGroup (adcState, AdcConfigPtr, group)) && (dataBufferPtr != NULL))
  {
    if ((ADC_CONV_MODE_CONTINUOUS == AdcConfigPtr->groupConfigPtr[group].conversionMode) &&
         ((ADC_STREAM_COMPLETED    == AdcConfigPtr->groupConfigPtr[group].status->groupStatus) ||
          (ADC_COMPLETED           == AdcConfigPtr->groupConfigPtr[group].status->groupStatus)))
    {
        /** @req SWS_Adc_00331. */
        /** @req SWS_Adc_00329. */
      AdcConfigPtr->groupConfigPtr[group].status->groupStatus = ADC_BUSY;
      returnValue = E_OK;
    }
    else if ((ADC_CONV_MODE_ONESHOT == AdcConfigPtr->groupConfigPtr[group].conversionMode) &&
             (ADC_STREAM_COMPLETED  == AdcConfigPtr->groupConfigPtr[group].status->groupStatus))
    {
        /** @req SWS_Adc_00330 */
      AdcConfigPtr->groupConfigPtr[group].status->groupStatus = ADC_IDLE;

      returnValue = E_OK;
    }
    else
    {
      /* Keep status. */
      returnValue = E_OK;
    }

    if (E_OK == returnValue)
    {
      /* Copy the result to application buffer. */
      for (channel = 0; channel < AdcConfigPtr->groupConfigPtr[group].numberOfChannels; channel++)
      {
          /* @req SWS_Adc_00075 */
          dataBufferPtr[channel] = AdcConfigPtr->groupConfigPtr[group].resultBuffer[channel];
      }
    }
  }
  else
  {
    /* An error have been raised from Adc_CheckReadGroup(). */
    returnValue = E_NOT_OK;
  }

  return (returnValue);
}
#endif

ISR(Adc_GroupConversionComplete)
{
  uint8 index;
  Adc_GroupDefType *groupPtr = NULL;
  /* @req SWS_Adc_00078 */
  /* Clear SCF flag. Not needed if AFFC is set but better to always do it  */
  ATD0STAT0 = SCF;

  // Check which group is busy, only one is allowed to be busy at a time in a hw unit
  for (index = 0; index < ADC_NBR_OF_GROUPS; index++)
  {
      if(AdcConfigPtr->groupConfigPtr[index].status->groupStatus == ADC_BUSY)
      {
          groupPtr = (Adc_GroupDefType *)&AdcConfigPtr->groupConfigPtr[index];
          break;
      }
  }
  if(groupPtr != NULL)
  {
      // Read hw buffer,
      volatile uint16_t *ptr = &ATD0DR0;
      for(index=0; index<groupPtr->numberOfChannels; index++)
      {
          groupPtr->resultBuffer[index] = *(ptr + groupPtr->channelList[index]);
      }

      /* Sample completed. */
      /* @req SWS_Adc_00325 */
      groupPtr->status->groupStatus = ADC_STREAM_COMPLETED;

      /* Call notification if enabled. */
    #if (ADC_GRP_NOTIF_CAPABILITY == STD_ON)
      if (groupPtr->status->notifictionEnable && groupPtr->groupCallback != NULL)
      {
          groupPtr->groupCallback();
      }
    #endif
  }
}

#if (ADC_ENABLE_START_STOP_GROUP_API == STD_ON)
void Adc_StartGroupConversion (Adc_GroupType group)
{
  /* Run development error check. */
  if (E_OK == Adc_CheckStartGroupConversion (adcState, AdcConfigPtr, group))
  {
    /* Set single scan enable bit if this group is one shot. */
    if (AdcConfigPtr->groupConfigPtr[group].conversionMode == ADC_CONV_MODE_ONESHOT)
    {
       /* Start next AD conversion. */
       ATD0CTL5 = BM_DJM | BM_MULT;	/* 10010000 Right Justified,unsigned */
                                    /* No SCAN/MULT/AD0 start select */
      /* Set group state to BUSY. */
      AdcConfigPtr->groupConfigPtr[group].status->groupStatus = ADC_BUSY;
    }
    else
    {
        // Continous mode
        /* Start AD conversion. */
        ATD0CTL5 = BM_DJM | BM_MULT | BM_SCAN; /* 10010000 Right Justified,unsigned */
                                               /* SCAN/MULT/AD0 start select */
       /* Set group state to BUSY. */
       AdcConfigPtr->groupConfigPtr[group].status->groupStatus = ADC_BUSY;
    }
  }
  /* @req SWS_Adc_00156 */
}

void Adc_StopGroupConversion (Adc_GroupType group)
{
  /* Run development error check. */
  if (E_OK == Adc_CheckStopGroupConversion (adcState, AdcConfigPtr, group))
  {
      /*@req SWS_Adc_00385 */
      /*@req SWS_Adc_00386 */
       ATD0CTL3 = 0x03; /* Hard write to stop current conversion */
#if (ADC_GRP_NOTIF_CAPABILITY == STD_ON)
       /* Disable group notification if enabled. */
       if(1 == AdcConfigPtr->groupConfigPtr[group].status->notifictionEnable){
           /* @req SWS_Adc_00155 */
           Adc_DisableGroupNotification (group);
       }
#endif
  }
  else
  {
    /* Error have been set within Adc_CheckStartGroupConversion(). */
  }
}
#endif

#if (ADC_GRP_NOTIF_CAPABILITY == STD_ON)
void Adc_EnableGroupNotification (Adc_GroupType group)
{
    /* @req SWS_Adc_00057 */
    Adc_EnableInternalGroupNotification(adcState, AdcConfigPtr, group);
}

void Adc_DisableGroupNotification (Adc_GroupType group)
{
    /* @req SWS_Adc_00058 */
    Adc_InternalDisableGroupNotification(adcState, AdcConfigPtr, group);
}
#endif

Adc_StatusType Adc_GetGroupStatus (Adc_GroupType group)
{
    return Adc_InternalGetGroupStatus(adcState, AdcConfigPtr, group);
}

#if (STD_ON == ADC_VERSION_INFO_API)
void Adc_GetVersionInfo(Std_VersionInfoType* versioninfo)
{
    Adc_InternalGetVersionInfo(versioninfo);
}
#endif
