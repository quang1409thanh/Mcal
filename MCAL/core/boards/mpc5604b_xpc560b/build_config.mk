
# ARCH defines
ARCH=mpc55xx
ARCH_FAM=ppc

# CFG (y/n) macros
CFG=PPC E200Z0 MPC55XX MPC560X MPC560XB MPC5604B BRD_MPC5604B_XPC560B
CFG+=MCU_ARC_CONFIG
CFG+=CREATE_SREC
CFG+=VLE

# MCAL
MOD_AVAIL+=ADC DIO DMA CAN GPT LIN MCU PORT PWM WDG SPI FLS

# Required modules
MOD_USE += MCU KERNEL

# Default cross compiler
COMPILER?=cw
DEFAULT_CROSS_COMPILE = /opt/powerpc-eabivle/bin/powerpc-eabivle-
DEFAULT_CW_COMPILE= /c/devtools/Freescale/cw_mpc5xxx_2.10
DEFAULT_DIAB_COMPILE = /c/devtools/WindRiver/diab/5.9.3.0/WIN32
DEFAULT_GHS_COMPILE = /c/devtools/ghs/comp_201314p

# Defines
def-y += SRAM_SIZE=0xc000


# Software floating point, PowerPC No Small-Data ELF EABI Object Format
DIAB_TARGET?=-tPPCE200Z0VFS:simple
GHS_TARGET?=ppc560xb


