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


/** @reqSettings DEFAULT_SPECIFICATION_REVISION=4.0.3 */


#ifndef FEE_H_
#define FEE_H_

#include "Modules.h"
/* @req FEE185 */
#define FEE_MODULE_ID			MODULE_ID_FEE
#define FEE_VENDOR_ID			VENDOR_ID_ARCCORE

#define FEE_SW_MAJOR_VERSION	1u
#define FEE_SW_MINOR_VERSION	0u
#define FEE_SW_PATCH_VERSION	0u
#define FEE_AR_RELEASE_MAJOR_VERSION        4u
#define FEE_AR_RELEASE_MINOR_VERSION        0u
#define FEE_AR_RELEASE_REVISION_VERSION     3u
#define FEE_AR_MAJOR_VERSION	FEE_AR_RELEASE_MAJOR_VERSION
#define FEE_AR_MINOR_VERSION	FEE_AR_RELEASE_MINOR_VERSION
#define FEE_AR_PATCH_VERSION	FEE_AR_RELEASE_REVISION_VERSION

#include "Std_Types.h"
#include "Fee_Cfg.h"
#if defined(USE_FLS)
#include "Fls.h"
#endif

#if (FEE_DEV_ERROR_DETECT == STD_ON)
// Error codes reported by this module defined by AUTOSAR
/* !req FEE010 *//* Errors missing */
/* @req FEE048 */
#define FEE_E_UNINIT						0x01u
#define FEE_E_INVALID_BLOCK_NO				0x02u
#define FEE_E_INVALID_BLOCK_OFS				0x03u
#define FEE_E_INVALID_DATA_PTR				0x04u
#define FEE_E_INVALID_BLOCK_LEN				0x05u
#define FEE_E_BUSY							0x06u
#define FEE_E_BUSY_INTERNAL                 0x07u

// Other error codes reported by this module
#define FEE_PARAM_OUT_OF_RANGE				0x40u
#define FEE_UNEXPECTED_STATE				0x41u
#define FEE_FLASH_CORRUPT					0xfau
#define FEE_UNEXPECTED_STATUS				0xfbu
#define FEE_E_WRONG_CONFIG					0xfcu
#define FEE_E_UNEXPECTED_EXECUTION			0xfdu
#define FEE_E_NOT_SUPPORTED					0xfeu
#define FEE_E_NOT_IMPLEMENTED_YET			0xffu

// Service ID in this module
#define FEE_INIT								0x00u
#define FEE_SET_MODE_ID							0x01u
#define FEE_READ_ID								0x02u
#define FEE_WRITE_ID							0x03u
#define FEE_CANCEL_ID							0x04u
#define FEE_GET_STATUS_ID						0x05u
#define FEE_GET_JOB_RESULT_ID					0x06u
#define FEE_INVALIDATE_BLOCK_ID					0x07u
#define FEE_GET_VERSION_INFO_ID					0x08u
#define FEE_ERASE_IMMEDIATE_ID					0x09u
#define FEE_JOB_END_NOTIFICTION_ID				0x10u
#define FEE_JOB_ERROR_NOTIFICTION_ID			0x11u
#define FEE_MAIN_FUNCTION_ID					0x12u

#define FEE_STARTUP_ID							0x40u
#define FEE_GARBAGE_WRITE_HEADER_ID				0x41u
#define FEE_GLOBAL_ID							0xffu

#endif

/* @req FEE065 */
#if ( FEE_VERSION_INFO_API == STD_ON )
/* @req FEE064 */
/* @req FEE082 */
/* !req FEE147 */
#define Fee_GetVersionInfo(_vi) STD_GET_VERSION_INFO(_vi, FEE)	/** @req FEE093 */
#endif /* FEE_VERSION_INFO_API */

void Fee_MainFunction(void);	/** @req FEE097 */

void Fee_Init(void);	/** @req FEE085 */
void Fee_SetMode(MemIf_ModeType mode);	/** @req FEE086 */
Std_ReturnType Fee_Read(uint16 blockNumber, uint16 blockOffset, uint8* dataBufferPtr, uint16 length); /** @req FEE087 */
Std_ReturnType Fee_Write(uint16 blockNumber, uint8* dataBufferPtr); /** @req FEE088 */
void Fee_Cancel(void);
MemIf_StatusType Fee_GetStatus(void);	/** @req FEE090 */
MemIf_JobResultType Fee_GetJobResult(void);	/** @req FEE091 */
Std_ReturnType Fee_InvalidateBlock(uint16 blockNumber);	/** @req FEE092 */
Std_ReturnType Fee_EraseImmediateBlock(uint16 blockNumber);



#endif /*FEE_H_*/
