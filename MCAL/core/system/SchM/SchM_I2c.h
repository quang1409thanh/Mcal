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
#ifndef SCHM_I2C_H_
#define SCHM_I2C_H_

#include "Os.h"

#define SchM_Enter_I2c_EA_0() 			SuspendOSInterrupts()
#define SchM_Exit_I2c_EA_0()  			ResumeOSInterrupts()

#define SCHM_MAINFUNCTION_I2C() 	    SCHM_MAINFUNCTION(I2C,I2c_MainFunction())


#endif /* SCHM_I2C_H_ */
