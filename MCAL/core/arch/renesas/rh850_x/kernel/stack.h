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


#ifndef CONTEXT_H_
#define CONTEXT_H_


#define SC_PATTERN		0xde
#define LC_PATTERN		0xad

/* Minimum alignment req */
#define ARCH_ALIGN		4

/* Small context (task swap==function call) */
#define SAVE_NVGPR(_x,_y)
#define RESTORE_NVGPR(_x,_y)

/* Save volatile regs, NOT preserved by function calls */
#define SAVE_VGPR(_x,_y)
#define RESTORE_VGPR(_x,_y)

/* Large context (interrupt) */
#define SAVE_ALL_GPR(_x,_y)
#define RESTORE_ALL_GPR(_x,_y)


// NVREGS: r0+r1+r2+r3+r4+r5+r6+r7+r8+r9+r10+r11+r12+lr = 14*4 = 40
#define NVGPR_SIZE		56
// VGPR: 9*4 = 36
//#define VGPR_SIZE		36
// SP + context
#define C_SIZE			8
#define VGPR_LR_OFF		(C_SIZE+NVGPR_SIZE-4)
#define C_CONTEXT_OFFS  4
#define C_SP_OFF 		0
#define SC_SIZE			(NVGPR_SIZE+C_SIZE)



#endif /* CONTEXT_H_ */
