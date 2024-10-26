
HOST := $(shell uname)
export prefix

# If we are using codesourcery and cygwin..
ifneq ($(findstring CYGWIN,$(UNAME)),)
cygpath:= $(shell cygpath -m $(shell which cygpath))
export CYGPATH=$(cygpath)
endif


# Normal msys32: MINGW32_NT-6.2
# msys2-32bit : MINGW32_NT-6.3
# msys2-64bit : MINGW64_NT-6.3
ifneq ($(findstring MINGW32,$(HOST)),)
  OS_32BIT=y
else 
 OS_64BIT=y
endif 

# Arch specific settings
ifneq ($(ARCH),)
 include $(ROOTDIR)/$(ARCH_PATH-y)/scripts/gcc.mk
endif

# Find version
gcc_version = $(shell ${CROSS_COMPILE}gcc --version | gawk -v VER=$(1) '{ if( VER >= strtonum(gensub(/\./,"","g",$$3)) ) print "y";exit  }' )
CC_INFO := "$(shell ${CROSS_COMPILE}gcc --version | sed -n 1p )"
GCC_V430 = $(call gcc_version,430)
GCC_V340 = $(call gcc_version,340)  

cflags-y += -DCC_INFO=$(CC_INFO)

#$(error $(gcc_info))


# ---------------------------------------------------------------------------
# Compiler
# CCFLAGS - compile flags

ifeq ($(CLANG_COMPILE),)
CC	= 	$(CROSS_COMPILE)gcc
else 
CC	= 	$(CROSS_COMPILE)$(CLANG_COMPILE)/clang
CFG_CLANG=y
endif

opt-cflags-$(CFG_OPT_RELEASE) += -O2
opt-cflags-$(CFG_OPT_DEBUG)   += -g -O0
opt-cflags-$(CFG_OPT_SIZE)   += -g -Os
opt-cflags-$(CFG_OPT_FLAGS)   += $(SELECT_OPT)

cflags-y+= -DBUILD_OPT_FLAGS="$(opt-cflags-y)"
cflags-y += $(opt-cflags-y)

#cflags-$(CFG_CLANG)   += -Weverything

 
# Remove sections if needed.. may be problems with other compilers here.

ifneq ($(CROSS_COMPILE),)
cflags-y += -ffunction-sections
endif

#ifneq ($(filter -O2 -O3 -O1,$(cflags-y)),) 
#	cflags-y += -fno-schedule-insns -fno-schedule-insns2
#endif

cflags-y += -c
#cflags-y 		+= -fno-common
cflags-y 		+= -std=gnu99
#cflags-y 		+= -fmax-errors=20
cflags-$(CFG_CLANG_SAFECODE) += -fmemsafety

# Generate dependencies
cflags-y 		+= -MMD

# Warnings
cflags-y           += -Wall
cflags-$(GCC_V340) += -Wextra
#cflags-y          += -Wstrict-prototypes      # 3.4.6
#cflags-y          += -Wold-style-definition   # 3.4.6
#cflags-y          += -Wmissing-prototypes     # 3.4.6
#cflags-y          += -Wmissing-declarations   # 3.4.6
#cflags-y          += -Wredundant-decls        # 3.4.6
#cflags-y          += -Wpointer-arith		   # 3.4.6	
#cflags-y          += -Wpadded                 # 3.4.6
#cflags-y          += -Wconversion             # 3.4.6 but not usable until 4.3.0

#cflags-$(GCC_V340)+= -Wextra
#cflags-$(GCC_V430)+= -Wconversion
#cflags-y          += -pedantic

# Conformance
cflags-y          += -fno-strict-aliasing	# Use restict keyword instead.
cflags-y          += -fno-builtin

# gcov
cflags-$(CFG_GCOV) += -fprofile-arcs
cflags-$(CFG_GCOV) += -ftest-coverage

# Get machine cflags
#cflags-y		+= $(cflags-$(ARCH))

cflags-y += -gdwarf-2

CFLAGS = $(cflags-y) $(cflags-yy)

CCOUT 		= -o $@ 

# ---------------------------------------------------------------------------
# Preprocessor

CPP	= 	$(CC) -E -P
CPPOUT = -o
CPP_ASM_FLAGS = -x assembler-with-cpp 

comma = ,
empty = 
space = $(empty) $(empty)

# Note!
# Libs related to GCC(libgcc.a, libgcov.a) is located under 
# lib/gcc/<machine>/<version>/<multilib>
# Libs related to the library (libc.a,libm.a,etc) are under:
# <machine>/lib/<multilib>
# 
# Can't remember why haven't I just used gcc to link instead of ld? (it should 
# figure out the things below by itself)

# It seems some versions of make want "\=" and some "="
# "=" - msys cpmake on windows 7 
gcc_lib_path := "$(subst /libgcc.a,,$(shell $(CC) $(CFLAGS) --print-libgcc-file-name))" 
gcc_lib_path := $(subst \libgcc.a,,$(gcc_lib_path)) 

ifeq ($(COMPILER_FLAVOR),s32_newlib)
gcc-sysroot="--sysroot=$(dir $(CC))../powerpc-eabivle/newlib"
endif
lib_lib_path := "$(subst /libc.a,,$(shell $(CC) $(CFLAGS) $(gcc-sysroot) --print-file-name=libc.a))"
lib_lib_path := $(subst \libc.a,,$(lib_lib_path))

ifeq ($(COMPILER_FLAVOR),hightec_dnk) 
SELECT_CLIB?=CLIB_DNK
endif

#$(error $(lib_lib_path)) 

# Convert from 
# c:/devtools/freescale/s32_power_v1.0/cross_tools/powerpc-eabivle-4_9/bin/../lib/gcc/powerpc-eabivle/4.9.2/fp/e200z3
#   into 
# c:/devtools/freescale/s32_power_v1.0/cross_tools/powerpc-eabivle-4_9/powerpc-eabivle/newlib/lib/fp/e200z3


# Get information from GCC and save it in gcc_path_probe.tmp
gcc_info := $(shell touch gcc_path_probe.c;$(CC) -v -c gcc_path_probe.c &> gcc_path_probe.tmp; rm gcc_path_probe.c)

# Extract the built in include paths from the information
gcc_include := $(subst \,/,$(shell gawk -f $(TOPDIR)/scripts/gcc_getinclude.awk gcc_path_probe.tmp; rm gcc_path_probe.tmp))

cc_inc_path := $(realpath $(gcc_include))

libpath-y += -L$(lib_lib_path)
libpath-y += -L$(gcc_lib_path)

ifeq ($(CC),gcc)
SELECT_CLIB?=CLIB_NATIVE
else
SELECT_CLIB?=CLIB_NEWLIB
endif

ifeq ($(SELECT_CLIB),CLIB_NEWLIB)
CFG_ARC_CLIB?=y
endif

ifeq ($(SELECT_CLIB),CLIB_DNK)
CFG_ARC_CLIB?=y
endif


# ---------------------------------------------------------------------------
# Linker
#
# LDFLAGS 		- linker flags
# LDOUT   		- How to Generate linker output file
# LDMAPFILE     - How to generate mapfile 
# ldcmdfile-y	- link cmd file
# libpath-y		- lib paths
# libitem-y		- the libs with path
# lib-y			- the libs, without path


LD = $(CROSS_COMPILE)ld


LD_FILE = -T

LDOUT 		= -o $@
ifeq ($(CROSS_COMPILE),)
TE=out
else
TE = elf
endif

ifneq ($(CROSS_COMPILE),)
ifneq ($(BOARDDIR),gnulinux)
LDFLAGS += --gc-section
endif
endif

ldflags-$(CFG_CLANG_SAFECODE) += -fmemsafety
ldflags-$(CFG_CLANG_SAFECODE) += -L$(CLANG_COMPILE)/../lib
 
# Don't use a map file if we are compiling for native target.
# gnulinux defines its own map file argument.
ifneq ($(CROSS_COMPILE),) 
ifneq ($(BOARDDIR),gnulinux)
LDFLAGS += -Map $(subst .$(TE),.map, $@)
endif
endif

LDFLAGS += $(ldflags-y)


lib-$(CFG_GCOV) += -lgcov 

libitem-y += $(libitem-yy)

LD_START_GRP = --start-group
LD_END_GRP = --end-group

#LDFLAGS += --gc-section
ifeq ($(CROSS_COMPILE)$(COMPILER),gcc)
ifeq ($(CFG_TCPIP_TEST),y)
LDFLAGS +=-lpthread
endif
endif

# ---------------------------------------------------------------------------
# Assembler
#
# ASFLAGS 		- assembler flags
# ASOUT 		- how to generate output file

AS	= 	$(CROSS_COMPILE)as

ASFLAGS += $(asflags-y) $(asflags-y-y)
ASFLAGS += --gdwarf2
ASOUT = -o $@

# ---------------------------------------------------------------------------
# Dumper

OBJCOPY 		= $(CROSS_COMPILE)objcopy
OBJCOPY_FLAGS   = -O srec

# ---------------------------------------------------------------------------
# Archiver
#
# AROUT 		- archiver flags

AR	= 	$(CROSS_COMPILE)ar
AROUT 	= $@
ARFLAGS = 

define do-objcopy
	$(OBJCOPY) $(OBJCOPY_FLAGS) $< $@ 
endef

ifdef CFG_HC1X
define do-memory-footprint
	@$(CROSS_COMPILE)objdump -h $@ | gawk -f $(ROOTDIR)/scripts/hc1x_memory.awk
endef	
else
define do-memory-footprint
	@gawk --non-decimal-data -f $(ROOTDIR)/scripts/memory_footprint_gcc.awk $(subst .elf,.map,$@)
endef
endif	



