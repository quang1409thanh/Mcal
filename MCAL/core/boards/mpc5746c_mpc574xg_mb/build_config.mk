
# ARCH defines
ARCH=mpc55xx
ARCH_FAM=ppc

# CFG (y/n) macros
CFG=PPC E200Z4 MPC55XX MPC57XX MPC5746C CAN_OSCILLATOR_CLOCK BRD_MPC5746c_MPC574XG_MB
#CFG+=EFPU
CFG+=MCU_ARC_CONFIG
CFG+=CREATE_SREC
CFG+=VLE
#CFG+=TIMER
CFG+=TIMER_STM
CFG+=MCU_CACHE_I


# What buildable modules does this board have, 
# default or private

# Memory + Peripherals
MOD_AVAIL+= MCU CAN LIN PORT DIO ADC SPI EEP FLS WDG ETH PWM GPT TCPIP LWIP FR ICU

# Required modules
MOD_USE += MCU KERNEL

# Default cross compiler
COMPILER?=ghs
COMPILER_FLAVOR=s32_newlib

ifneq ($(filter CFG_VLE,$(CFG)),)
DEFAULT_CROSS_COMPILE = /opt/powerpc-eabispe/bin/powerpc-eabispe-
else 
#DEFAULT_CROSS_COMPILE = /opt/powerpc-eabivle/bin/powerpc-eabivle-
# S32 Tools
DEFAULT_CROSS_COMPILE = /c/devtools/Freescale/S32_Power_v1.1/Cross_Tools/powerpc-eabivle-4_9/bin/powerpc-eabivle-
endif


DEFAULT_CW_COMPILE= /c/devtools/Freescale/cw_mpc5xxx_2.10
DEFAULT_DIAB_COMPILE = /c/devtools/WindRiver/diab/5.9.3.0/WIN32
DEFAULT_GHS_COMPILE = /c/devtools/ghs/comp_201314p

#vle=$(if $(filter $(CFG),VLE),y)
#novle=$(if $(vle),n,y)
#efpu=$(if $(filter $(CFG),EFPU),y)
#nofpu=$(if $(efpu),n,y)

# VLE
GHS_TARGET?=ppc5746mz420

# Defines
def-y += __BIG_ENDIAN__
def-y += SRAM_SIZE=0x80000


