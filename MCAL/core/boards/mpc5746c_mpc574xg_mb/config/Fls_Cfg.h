/* -------------------------------- Arctic Core ------------------------------
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

/** @addtogroup Fls Flash Driver
 *  @{ */

/** @file Fls_Cfg.h
 *  Definitions of configuration parameters for Flash Driver.
 */



#ifndef FLS_CFG_H_
#define FLS_CFG_H_

#define USE_FLS_INFO        STD_ON


/* STD container : Fls
 * FlsConfigSet             1..*
 * FlsGeneral               1
 * FlsPublishedInformation  1
 */

#include "MemIf_Types.h"

/* FlsGeneral, 3.0 OK */
#define FLS_VARIANT_PB              STD_OFF
#define FLS_AC_LOAD_ON_JOB_START    STD_OFF         /* NO SUPPORT */
#define FLS_BASE_ADDRESS            0x00000000//0x00610000
#define FLS_CANCEL_API              STD_OFF         /* NO SUPPORT */
#define FLS_COMPARE_API             STD_ON
#define FLS_DEV_ERROR_DETECT        STD_ON
#define FLS_DRIVER_INDEX            0               /* NO SUPPORT */
#define FLS_GET_JOB_RESULT_API      STD_ON
#define FLS_GET_STATUS_API          STD_ON
#define FLS_SET_MODE_API            STD_OFF         /* NO SUPPORT */
#define FLS_USE_INTERRUPTS          STD_OFF         /* NO SUPPORT */
#define FLS_VERSION_INFO_API        STD_ON

/* FlsPublishedInformation, 3.0 OK */
#define FLS_AC_LOCATION_ERASE       0               /* NO SUPPORT */
#define FLS_AC_LOCATION_WRITE       0               /* NO SUPPORT */
#define FLS_AC_SIZE_ERASE           0               /* NO SUPPORT */
#define FLS_AC_SIZE_WRITE           0               /* NO SUPPORT */
#define FLS_ERASE_TIME              0               /* NO SUPPORT */
#define FLS_ERASED_VALUE            0xff            /* NO SUPPORT */
#define FLS_EXPECTED_HW_ID          0               /* NO SUPPORT */
#define FLS_SPECIFIED_ERASE_CYCLES  0               /* NO SUPPORT */
#define FLS_WRITE_TIME              0               /* NO SUPPORT */

/* MCU Specific */
#if defined(CFG_MPC5746C)

#define FLASH_BANK_CNT              1
#define FLASH_PAGE_SIZE             8
#define FLASH_MAX_SECTORS           33
#define FLS_FLASH_WRITE_BUFFER_SIZE 128     /* Page buffer size for C55 flash*/

/*LOW : 16*1 + 32*4 + 64*4  =  400KB*/
/*MID : 16*8 + 32*2         =  192KB*/
/*HIGH : 16*2               =   32KB*/
/*LARGE : 256*10            = 2560KB*/
#define FLS_TOTAL_SIZE        0x01280000ULL /* This is used to indicate total size including the reserved areas
the effective FLASH size is computed above 0xC70000UL */

#else
#error CPU not supported
#endif

#if (USE_FLS_INFO==STD_ON)

typedef struct Flash {
    uint32 size;
    uint32 sectCnt;
    uint32 bankSize;
    uint32 regBase;
    uint32 sectAddr[FLASH_MAX_SECTORS+1];
    uint16 addrSpace[FLASH_MAX_SECTORS+1];
} FlashType;


#else
typedef struct {
  Fls_LengthType FlsNumberOfSectors;
  Fls_LengthType FlsPageSize;
  Fls_LengthType FlsSectorSize;
  Fls_AddressType FlsSectorStartaddress;
} Fls_SectorType;
#endif


struct Flash;

typedef struct {
	void (*FlsAcErase)(void);                   /* NO SUPPORT */
	void (*FlsAcWrite)(void);                   /* NO SUPPORT */
	// FlsCallCycle N/A in core.
	void (*FlsJobEndNotification)(void);
	void (*FlsJobErrorNotification)(void);
	uint32 FlsMaxReadFastMode;
	uint32 FlsMaxReadNormalMode;
	uint32 FlsMaxWriteFastMode;
	uint32 FlsMaxWriteNormalMode;
	uint32 FlsProtection;                       /* NO SUPPORT */
#if (USE_FLS_INFO==STD_ON)
	const struct Flash *FlsInfo;
#else
	const Fls_SectorType *FlsSectorList;
#endif
//	const uint32 FlsSectorListSize;             /* NO SUPPORT */
} Fls_ConfigSetType;

typedef Fls_ConfigSetType Fls_ConfigType;

extern const Fls_ConfigSetType FlsConfigSet[];

#endif /*FLS_CFG_H_*/
/** @} */
