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

#include "Std_Types.h"
#include "Dio.h"
#if defined(USE_DET)
#include "Det.h"
#endif
#include <string.h>
#include "regs.h"

#define DIO_GET_PORT_FROM_CHANNEL_ID(_channelId) (_channelId / 8)
#define DIO_GET_BIT_FROM_CHANNEL_ID(_channelId) (1 << (_channelId % 8))
#define CHANNEL_PTR		(&DioChannelConfigData)
#define CHANNEL_GRP_PTR	(&DioConfigData)
#define PORT_PTR		(&DioPortConfigData)

#if ( DIO_DEV_ERROR_DETECT == STD_ON )
static int Channel_Config_Contains(Dio_ChannelType channelId)
{
    Dio_ChannelType* ch_ptr=(Dio_ChannelType*)CHANNEL_PTR;
    int rv=0;
    while ((Dio_ChannelType)DIO_END_OF_LIST!=*ch_ptr)
    {
        if (*ch_ptr==channelId)
        {
            rv=1;
            break;
        }
        ch_ptr++;
    }
    return rv;
}

static int Port_Config_Contains(Dio_PortType portId)
{
    Dio_PortType* port_ptr=(Dio_PortType*)PORT_PTR;
    int rv=0;
    while ((Dio_PortType)DIO_END_OF_LIST!=*port_ptr)
    {
        if (*port_ptr==portId)
        {
            rv=1;
            break;
        }
        port_ptr++;
    }
    return rv;
}

static int Channel_Group_Config_Contains(const Dio_ChannelGroupType* _channelGroupIdPtr)
{
    Dio_ChannelGroupType* chGrp_ptr=(Dio_ChannelGroupType*)CHANNEL_GRP_PTR;
    int rv=0;

    while ((Dio_PortType)DIO_END_OF_LIST!=chGrp_ptr->port)
    {
        if (chGrp_ptr->port==_channelGroupIdPtr->port
                && chGrp_ptr->offset==_channelGroupIdPtr->offset
                && chGrp_ptr->mask==_channelGroupIdPtr->mask)
        {
            rv=1;
            break;
        }
        chGrp_ptr++;
    }
    return rv;
}

#define VALIDATE_CHANNEL(_channelId, _api) \
    if(0==Channel_Config_Contains(channelId)) {	\
        Det_ReportError(DIO_MODULE_ID,0,_api,DIO_E_PARAM_INVALID_CHANNEL_ID ); \
        level = 0;	\
        goto cleanup;	\
        }
#define VALIDATE_PORT(_portId, _api)\
    if(0==Port_Config_Contains(_portId)) {\
        Det_ReportError(DIO_MODULE_ID,0,_api,DIO_E_PARAM_INVALID_PORT_ID ); \
        level = STD_LOW;\
        goto cleanup;\
    }
#define VALIDATE_CHANNELGROUP(_channelGroupIdPtr, _api)\
    if(0==Channel_Group_Config_Contains(_channelGroupIdPtr)) {\
        Det_ReportError(DIO_MODULE_ID,0,_api,DIO_E_PARAM_INVALID_GROUP_ID ); \
        level = STD_LOW;\
        goto cleanup;\
    }
#else
#define VALIDATE_CHANNEL(_channelId, _api)
#define VALIDATE_PORT(_portId, _api)
#define VALIDATE_CHANNELGROUP(_channelGroupIdPtr, _api)
#endif

Dio_LevelType Dio_ReadChannel(Dio_ChannelType channelId)
{
    Dio_LevelType level;
    VALIDATE_CHANNEL(channelId, DIO_READCHANNEL_ID);

    Dio_PortLevelType portVal = Dio_ReadPort(DIO_GET_PORT_FROM_CHANNEL_ID(channelId));
    Dio_PortLevelType bit = DIO_GET_BIT_FROM_CHANNEL_ID(channelId);

    if ((portVal & bit) != STD_LOW){
        level = STD_HIGH;
    } else {
        level = STD_LOW;
    }

#if ( DIO_DEV_ERROR_DETECT == STD_ON )
    cleanup:
#endif
    return (level);
}

void Dio_WriteChannel(Dio_ChannelType channelId, Dio_LevelType level)
{
    VALIDATE_CHANNEL(channelId, DIO_WRITECHANNEL_ID);

    Dio_PortLevelType portVal = Dio_ReadPort(DIO_GET_PORT_FROM_CHANNEL_ID(channelId));
    Dio_PortLevelType bit = DIO_GET_BIT_FROM_CHANNEL_ID(channelId);

    if(level == STD_HIGH){
        portVal |= bit;
    } else {
        portVal &= ~bit;
    }

    Dio_WritePort(DIO_GET_PORT_FROM_CHANNEL_ID(channelId), portVal);

#if ( DIO_DEV_ERROR_DETECT == STD_ON )
    cleanup:
#endif
    return;
}

Dio_PortLevelType Dio_ReadPort(Dio_PortType portId)
{
    Dio_PortLevelType level = 0;
    VALIDATE_PORT(portId, DIO_READPORT_ID);

    if(portId == DIO_PORT_A){level = PORTA;}
    else if(portId == DIO_PORT_B){level = PORTB;}
    else if(portId == DIO_PORT_E){level = PORTE;}
    else if(portId == DIO_PORT_J){level = PTJ;}
    else if(portId == DIO_PORT_K){level = PORTK;}
    else if(portId == DIO_PORT_M){level = PTM;}
    else if(portId == DIO_PORT_P){level = PTP;}
    else if(portId == DIO_PORT_S){level = PTS;}
    else if(portId == DIO_PORT_T){level = PTT;}
    else if(portId == DIO_PORT_H){level = PTH;}

#if ( DIO_DEV_ERROR_DETECT == STD_ON )
    cleanup:
#endif
    return level;
}

void Dio_WritePort(Dio_PortType portId, Dio_PortLevelType level)
{
    VALIDATE_PORT(portId, DIO_WRITEPORT_ID);

    if(portId == DIO_PORT_A){PORTA = level;}
    else if(portId == DIO_PORT_B){PORTB = level;}
    else if(portId == DIO_PORT_E){PORTE = level;}
    else if(portId == DIO_PORT_J){PTJ = level;}
    else if(portId == DIO_PORT_K){PORTK = level;}
    else if(portId == DIO_PORT_M){PTM = level;}
    else if(portId == DIO_PORT_P){PTP = level;}
    else if(portId == DIO_PORT_S){PTS = level;}
    else if(portId == DIO_PORT_T){PTT = level;}
    else if(portId == DIO_PORT_H){PTH = level;}

#if ( DIO_DEV_ERROR_DETECT == STD_ON )
    cleanup:
#endif
    return;
}

Dio_PortLevelType Dio_ReadChannelGroup(
    const Dio_ChannelGroupType *channelGroupIdPtr)
{
    Dio_PortLevelType level;
    VALIDATE_CHANNELGROUP(channelGroupIdPtr,DIO_READCHANNELGROUP_ID);

    // Get masked values
    level = Dio_ReadPort(channelGroupIdPtr->port) & channelGroupIdPtr->mask;

    // Shift down
    level = level >> channelGroupIdPtr->offset;

#if ( DIO_DEV_ERROR_DETECT == STD_ON )
    cleanup:
#endif
    return level;
}

void Dio_WriteChannelGroup(const Dio_ChannelGroupType *channelGroupIdPtr,
    Dio_PortLevelType level)
{
    VALIDATE_CHANNELGROUP(channelGroupIdPtr,DIO_WRITECHANNELGROUP_ID);

    // Shift up and apply mask so that no unwanted bits are affected
    level = (level << channelGroupIdPtr->offset) & channelGroupIdPtr->mask;

    // Read port and clear out masked bits
    Dio_PortLevelType portVal = Dio_ReadPort(channelGroupIdPtr->port) & (~channelGroupIdPtr->mask);

    // Or in the upshifted masked level
    portVal |= level;

    Dio_WritePort(channelGroupIdPtr->port, portVal);

#if ( DIO_DEV_ERROR_DETECT == STD_ON )
    cleanup:
#endif
    return;
}
