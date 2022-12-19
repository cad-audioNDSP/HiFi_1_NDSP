# makefile for building IntegrIT(R) DSP Library
# Version 0.34 tuned for Cadence HiFi cores
# ---------------------------------------------------
# Target platform: gcc/xcc
#
#  make invocation:
#  make clean   - clean up all temporary files
#  make depends - refresh dependencies (makedeps.mk)
#  make all     - make depends and release
#-----------------------------------------------------
#  build under xcc
#  make sure that all neccesary Xtensa system variables 
#  and pathes configured properly. makefile uses 
#  XTENSA_CORE system variable to detect the name of core
#  so, if makefile is running from the command-line shell
#  executed from proper processor config of Xtensa Exmplorer 
#  it does not require setting of any additional parameters
#
#  System variables XTENSA_CORE,XTENSA_SYSTEM should.
#  be properly configured for correct procedure.
#  if XTENSA_SYSTEM is undefined, g++ build procedure
#  will use cstub files and headers from local
#  directory, not from system path
#
#  Other variables:
#  LANG=C++    build under g++, otherwise xcc used
#  SAVETEMPS=1 save temporary files
#  MAP=1       generate map file
#  MEM_MODEL=1,2 enables memory modeling
#-----------------------------------------------------  
#  build under gcc require unsetting of variable XTENSA_SYSTEM
#  (either by command line unset XTENSA_SYSTEM or via make 
#  command line option -e XTENSA_SYSTEM=)
#  and selecting proper core by XTENSA_CORE variable. 
#  By default, it use C language, C++ will be selected by 
#  using LANG=C++ system variable.
# 
#  examples:
#  1. build with with cstubs :
#  make -e XTENSA_SYSTEM= XTENSA_CORE=XRC_B64EP_PM_20apr
#  2. build under g++ with cstubs:
#  make -e XTENSA_SYSTEM= XTENSA_CORE=XRC_B64EP_PM_20apr LANG=C++
#-----------------------------------------------------

SRCDIRS  =	../../../../testdriver

OUTDIR  =	../../../bin

INCDIRS =	../../../../library/include		\
		../../../../library/include_private

TARGET := testdriver
LIBS   := m
DLIBS  := ../../../bin/NatureDSP_Signal 

BUILD_TYPE		:= EXECUTABLE
INTEGRIT_OLD_SUFFIXES	:= 1
COMPILE_CSTUB		:= 0
LSP			:=
EXTERNAL_PROJECTS = ../library 

# check if c++ compilation required
ifeq (C++,$(LANG))
CPP = 1
HAVE_XTENSA = 0
else
CPP = 0
HAVE_XTENSA = 1
endif  

EXTERNAL_PROJECTS.0 = ../cstub


EXTERNAL_PROJECTS += $(EXTERNAL_PROJECTS.$(HAVE_XTENSA))
ifeq (0,$(HAVE_XTENSA))
USE_PATCHES=1  # use patched cstubs for gcc, otherwise use REFERENCE variable
else
ifeq (1,$(REFERENCE))
USE_PATCHES=0
REF_SUFFIX=_reference
else
USE_PATCHES=1
REF_SUFFIX=
endif
endif

ifeq (1,$(MEM_MODEL))
MEM_SUFFIX=_mem_model
else
ifeq (2,$(MEM_MODEL))
MEM_SUFFIX=_mem_model2
else
MEM_SUFFIX=
endif
endif


MODULES = \
          main \
          malloc16 \
          mips_cfft \
          mips_dct \
          mips_fft \
          mips_fir \
          mips_iir \
          mips_math \
          mips_mtx \
          mips_mtxinv \
          mips_rfft \
          mips_vec \
          profiler \
          rms \
          test_cfft \
          test_dct \
          test_fft \
          test_fir \
          test_iir \
          test_math \
          test_mips \
          test_mtx \
          test_mtxinv \
          test_rfft \
          test_vec \
          testeng \
          testeng_dct \
	  testeng_errh \
          testeng_fft \
          testeng_fir_old \
          testeng_iir_lat \
          testeng_iir_old \
          testeng_mtx \
          testeng_mtxinv \
          utils \
          vectools \
          vecutils \
          wavreader \
          wavwriter 
 
ifneq (0, $(HAVE_XTENSA))
#CFLAGS_FILE_module_name   += some compiler switches
CFLAGS_FILE_main += -Wno-error
CFLAGS_FILE_test_iir += -Wno-error
CFLAGS_FILE_mips_iir += -Wno-error
CFLAGS_FILE_utils  += -Wno-error
endif
 
default:	all

.PHONY:	all clean cleanest cleanlib debug ferret default profile release
.PHONY: $(foreach proj,$(EXTERNAL_PROJECTS), $(proj))
.PHONY: $(foreach proj,$(EXTERNAL_PROJECTS), $(proj)/clean)
.PHONY: $(foreach proj,$(EXTERNAL_PROJECTS), $(proj)/cleanest)

debug:		all
ferret:		all
profile:	all
release:	all 
 

ifneq (1, $(HAVE_XTENSA)) ############################
CORE			:= cstub
else #################################################
ifneq (,$(XTENSA_TOOLS))
XTPATH=$(XTENSA_TOOLS)/bin/
else
ifneq (,$(XTTOOLS))
XTPATH=$(XTENSA_TOOLS)/bin/
endif
endif
endif ################################################

ifeq ("cstub", "$(CORE)") ############################
ifeq ("","$(PLATFORM)")
ifeq (1, $(CPP))
PLATFORM		:= $(XTENSA_CORE)-C++
PLATFORM_LIB		:= $(XTENSA_CORE)-C++
else
PLATFORM		:= $(XTENSA_CORE)
PLATFORM_LIB		:= $(XTENSA_CORE)
endif
endif
else #################################################
ifeq ("", "$(CORE)")
ifeq (1,$(HAVE_XTENSA))
PLATFORM		:= $(XTENSA_CORE)$(REF_SUFFIX)$(MEM_SUFFIX)
PLATFORM_LIB		:= $(XTENSA_CORE)$(REF_SUFFIX)
else
endif
else
PLATFORM		:= $(CORE)
PLATFORM_LIB		:= $(CORE)
endif
endif ################################################

ifneq (, $(findstring cstub, $(PLATFORM))) ###########
SOLID_LIBRARY		:= 1
CCFLAGS_DIR_testdriver	+= -Wno-uninitialized
CCFLAGS_DIR_testdriver	+= -Wno-unused-result
CCFLAGS_DIR_cstub 		+= -Wno-uninitialized
CCFLAGS					+= -Wno-strict-aliasing
else #################################################
SOLID_LIBRARY		:= 0
CCFLAGS_DIR_fft_files += -OPT:alias=restrict
CCFLAGS_DIR_P2_files += -fvectorize
endif ################################################

###############################################################
##                      Host OS detection                    ##
###############################################################


WINDIR+=$(windir)

ifeq (,$(WINDIR))
# Unix-like system
ULIS=1
override HOSTOS=$(shell uname)
else
ULIS=0
override HOSTOS=win32
SHELL=cmd
endif

###############################################################
##              Platform-specific configuration              ##
###############################################################
HIFI_CSTUB_MODULES += cstub-Xm_$(XTENSA_CORE)

ifeq (,$(XTENSA_SYSTEM))
HIFI_CSTUB_INCDIRS	+= ../../../../cstub ../../../../cstub/xtensa/tie ../../../../cstub/xtensa/config
HIFI_CSTUB_SRCDIRS	+= ../../../../cstub
CFLAGS_FILE_$(subst -,_,$(HIFI_CSTUB_MODULES)) = -Dasm=__asm__ -O0
else
HIFI_CSTUB_INCDIRS += $(XTENSA_SYSTEM)/../src/cstub $(XTENSA_SYSTEM)/../src/cstub/xtensa/tie $(XTENSA_SYSTEM)/../xtensa-elf/arch/include/xtensa/config
HIFI_CSTUB_SRCDIRS += $(XTENSA_SYSTEM)/../src/cstub
endif

ifneq (0, $(HAVE_XTENSA)) ############################
##############################################################
VARPREFIX.$(PLATFORM)          = $(PLATFORM)
CC.$(PLATFORM)		       = xt-clang
LD.$(PLATFORM)		       = xt-clang
AR.$(PLATFORM)		       = xt-ar
MAKE.$(PLATFORM)	       = xt-make
OBJCOPY.$(PLATFORM)	       = xt-objcopy
RUN.$(PLATFORM)		       = xt-run
RUNMEM.$(PLATFORM)	       = xt-run --mem_model
RUNFAST.$(PLATFORM)	       = xt-run --turbo
PROF.$(PLATFORM)	       = xt-prof
ARCH.$(PLATFORM)	       = $(PLATFORM)
CFLAGS.$(PLATFORM)	       += -DCONFIG_$(PLATFORM)
CFLAGS.$(PLATFORM)	       += -DPROC_$(PLATFORM)
CFLAGS.$(PLATFORM)	       += -fmessage-length=0 -mcoproc -mlongcalls -INLINE:requested -fno-builtin
CFLAGS.profile.$(PLATFORM)     += -O3 -fvectorize
CFLAGS.release.$(PLATFORM)     += -O3 -fvectorize
LFLAGS.profile.$(PLATFORM)     += --profile
RUNFLAGS.debug.$(PLATFORM)     = --turbo
RUNFLAGS.release.$(PLATFORM)   = --turbo
RUNFLAGS.ferret.$(PLATFORM)   = --turbo
TARGETOS.$(PLATFORM)	       = Xtensa
else
VARPREFIX.$(PLATFORM)		 = $(PLATFORM)
ifeq (1, $(CPP))
CC.$(PLATFORM)      		 = g++
LD.$(PLATFORM)      		 = g++
LFLAGS.$(PLATFORM) 		 += -Wl,--section-start=unused_section=0
else
VARPREFIX.$(PLATFORM)		 = $(PLATFORM)
CC.$(PLATFORM)      		 = gcc
LD.$(PLATFORM)      		 = gcc
LFLAGS.$(PLATFORM) 		 +=  -Wl,--section-start=unused_section=0
endif
AR.$(PLATFORM)      		 = ar
#MAKE.$(PLATFORM)      		 = make
OBJCOPY.$(PLATFORM)              = objcopy
RUN.$(PLATFORM)     		 = 
PROF.$(PLATFORM)      		 = 
CFLAGS.$(PLATFORM)         	 += -m32
CFLAGS.$(PLATFORM) 		 += -DXTENSA_CORE=Xm_$(XTENSA_CORE)
ifeq (XRC_B64_LP,$(XTENSA_CORE))
CFLAGS.$(PLATFORM)         	 += -DB64_LP
endif
INCDIRS			 	 += $(HIFI_CSTUB_INCDIRS)
#SRCDIRS			 	 += $(HIFI_CSTUB_SRCDIRS)
ifeq (1, $(COMPILE_CSTUB))
MODULES				 += $(HIFI_CSTUB_MODULES)
endif
CFLAGS.profile.$(PLATFORM)       += -O3 
CFLAGS.release.$(PLATFORM)	 += -O3 
RUNFLAGS.debug.$(PLATFORM)	 = 
RUNFLAGS.release.$(PLATFORM)	 = 
RUNFLAGS.ferret.$(PLATFORM)	 = 
TARGETOS.$(PLATFORM)		 = Linux
LFLAGS.$(PLATFORM) 		 += -m32
DLIBS  += ../../../bin/cstub
endif

CFLAGS.release.$(PLATFORM)	 += -DUSE_ISA_PATCHES=$(USE_PATCHES)
###############################################################
##                     Well-known suffixes                   ##
###############################################################
#POSTFIX.$(PLATFORM)-Xtensa-release.1=_$(PLATFORM)
#POSTFIX.$(PLATFORM)-Xtensa-debug.1=_$(PLATFORM)
#POSTFIX.$(PLATFORM)-Xtensa-ferret.1=_$(PLATFORM)
#POSTFIX.$(PLATFORM)-Xtensa-profile.1=_$(PLATFORM)

#POSTFIX_DLC.$(PLATFORM)-Xtensa-release.1=_$(PLATFORM_LIB)
#POSTFIX_DLC.$(PLATFORM)-Xtensa-debug.1=_$(PLATFORM_LIB)
#POSTFIX_DLC.$(PLATFORM)-Xtensa-profile.1=_$(PLATFORM_LIB)

###############################################################
##                 Host-specific configuration               ##
###############################################################

ifeq (1,$(ULIS)) ##############################################
ECHO=echo
override RM=rm -f
CP=cp
PSEP=/
NULLDEV=/dev/null
MKDIR=mkdir
JOIN=&&
else ##########################################################
ECHO=@echo
override RM=del
CP=copy
PSEP=\\\\#
NULLDEV=NUL
MKDIR=mkdir
JOIN=&
endif #########################################################

###############################################################
##                     Target OS definition                  ##
###############################################################

TARGETOS=$(firstword $(TARGETOS.$(PLATFORM)) $(HOSTOS))

VARPREFIX.Darwin=MAC
VARPREFIX.iOS=MAC
VARPREFIX.linux=LINUX
VARPREFIX.Linux=LINUX
VARPREFIX.unix-like.1=UNIX_LIKE
VARPREFIX.win32=WIN32

###############################################################
##                  Select build configuration               ##
###############################################################

CONFIG=$(firstword $(findstring release, $(MAKECMDGOALS)) $(findstring debug,\
 $(MAKECMDGOALS)) $(findstring ferret, $(MAKECMDGOALS)) $(findstring profile, $(MAKECMDGOALS)) release)

CONFIG_ID	= $(PLATFORM)-$(TARGETOS)-$(CONFIG)
DLIBS_CONFIG_ID	= $(PLATFORM_LIB)-$(TARGETOS)-$(firstword \
		  $(DLIBS_CONFIG) $(CONFIG))

# Intermediate directory

INTERMEDIATE_DIR= $(CONFIG_ID)
USE_SUFFIXES	= $(firstword $(INTEGRIT_SUFFIXES) $(INTEGRIT_OLD_SUFFIXES))
POSTFIX		= $(firstword \
	$(POSTFIX.$(CONFIG_ID).$(USE_SUFFIXES)) -$(CONFIG_ID))
POSTFIX_DLC	= $(firstword \
	$(POSTFIX_DLC.$(DLIBS_CONFIG_ID).$(USE_SUFFIXES)) \
	$(POSTFIX.$(DLIBS_CONFIG_ID).$(USE_SUFFIXES)) -$(DLIBS_CONFIG_ID))

$(INTERMEDIATE_DIR)/dirinfo.txt:
	-$(MKDIR) $(INTERMEDIATE_DIR)
	-$(ECHO) Intermediate directory.>$(INTERMEDIATE_DIR)/dirinfo.txt
	
###############################################################
##                Creating devtools options                  ##
###############################################################

IDIRS_OPT = $(foreach f,$(INCDIRS),-I $(f))
LDIRS_OPT = $(foreach f,$(LIBDIRS),-L $(f))

CC			= $(firstword $(CC.$(PLATFORM)) gcc)
LD			= $(firstword $(LD.$(PLATFORM)) gcc)
AR			= $(firstword $(AR.$(PLATFORM)) ar)
#MAKE			= $(firstword $(MAKE.$(PLATFORM)) make)
OBJCOPY			= $(firstword $(OBJCOPY.$(PLATFORM)) objcopy)
RUN			= $(RUN.$(PLATFORM))
PROF			= $(firstword $(PROF.$(PLATFORM)) gprof)

CFLAGS_DEPENDENT	+= $(CFLAGS.$(PLATFORM))

CFLAGS.default.debug.	= $(CFLAGS.default.debug) -g -O0 -D_DEBUG
CFLAGS.default.profile.	= $(CFLAGS.default.profile) -g -pg -O3
CFLAGS.default.release.	= $(CFLAGS.default.release) -O3
CFLAGS.default.ferret.	= $(CFLAGS.default.ferret) -g -O0 -D_DEBUG
LFLAGS.default.debug.	= $(LFLAGS.default.debug)
LFLAGS.default.ferret.	= $(LFLAGS.default.ferret)
LFLAGS.default.profile.	= $(LFLAGS.default.profile) -g -pg -O
LFLAGS.default.release.EXECUTABLE = -s 
LFLAGS.default.ferret.EXECUTABLE =  
LFLAGS.default.release.	= $(LFLAGS.default.release) -O \
			$(LFLAGS.default.release.$(BUILD_TYPE))
ifeq (1,$(MAP))
LFLAGS += -Wl,-Map,$(TARGET)-map.txt
endif

# support memory modeling
ifeq (1,$(MEM_MODEL))
LFLAGS += -mlsp=sim-local
CFLAGS += -DMEM_MODEL=1
else
ifeq (2,$(MEM_MODEL))
LFLAGS += -mlsp=sim
CFLAGS += -DMEM_MODEL=2
else
endif
endif

CFLAGS.debug		= $(CFLAGS.debug.$(PLATFORM)) \
			$(CFLAGS.default.debug.$(CFLAGS.debug.$(PLATFORM)))
CFLAGS.ferret		= $(CFLAGS.ferret.$(PLATFORM)) \
			$(CFLAGS.default.ferret.$(CFLAGS.ferret.$(PLATFORM)))			
CFLAGS.profile		= $(CFLAGS.profile.$(PLATFORM)) \
			$(CFLAGS.default.profile.$(CFLAGS.profile.$(PLATFORM)))
CFLAGS.release		= $(CFLAGS.release.$(PLATFORM)) \
			$(CFLAGS.default.release.$(CFLAGS.release.$(PLATFORM)))

CFLAGADDINCDIR		= $(firstword $(CFLAGADDINCDIR.$(PLATFORM)) -I)
CFLAGMAKEDEP		= $(firstword $(CFLAGMAKEDEP.$(PLATFORM)) -MM)
CFLAGMAKEOBJ		= $(firstword $(CFLAGMAKEDEP.$(PLATFORM)) -c)
CFLAGSAVETEMP.1		= $(firstword $(CFLAGSAVETEMP.$(PLATFORM)) -save-temps)

LFLAGS_DEPENDENT	+= $(LFLAGS.$(PLATFORM))
LFLAGS.debug		= $(LFLAGS.debug.$(PLATFORM)) \
			$(LFLAGS.default.debug.$(LFLAGS.debug.$(PLATFORM))) 
LFLAGS.profile		= $(LFLAGS.profile.$(PLATFORM)) \
			$(LFLAGS.default.profile.$(LFLAGS.profile.$(PLATFORM)))
LFLAGS.release		= $(LFLAGS.release.$(PLATFORM)) \
			$(LFLAGS.default.release.$(LFLAGS.release.$(PLATFORM)))
LFLAGS.ferret		= $(LFLAGS.ferret.$(PLATFORM)) \
			$(LFLAGS.default.ferret.$(LFLAGS.ferret.$(PLATFORM))) -Wl,-u -Wl,malloc -lferret
LFLAGS.LIBRARY.1	+=--no-standard-libraries
LFLAGS.LIBRARY.1	+=-Wl,-r,--retain-symbols-file,$(TARGET_SYMFILE)

ARFLAGS			= $(firstword $(ARFLAGS.$(PLATFORM)) rcs)

RUNFLAGS		+= $(RUNFLAGS.$(PLATFORM))
RUNFLAGS		+= $(RUNFLAGS.$(CONFIG).$(PLATFORM))

PROFFLAGS		+= $(PROFFLAGS.$(PLATFORM))

LFLAGADDLIBDIR		= $(firstword $(LFLAGADDLIBDIR.$(PLATFORM)) -L)

INCDIRS			+= $($(VARPREFIX.$(PLATFORM))_INCDIRS)
SRCDIRS			+= $($(VARPREFIX.$(PLATFORM))_SRCDIRS)
LIBDIRS			+= $($(VARPREFIX.$(PLATFORM))_LIBDIRS)
SRCDIRS			+= $(SRCDIR)

# Header search options
CFLAGS_DEPENDENT += $(foreach idir, $(INCDIRS), $(CFLAGADDINCDIR) $(idir))

CFLAGS_DEPENDENT	+= $(CFLAGSAVETEMP.$(SAVETEMPS))

# Library search options
LFLAGS_DEPENDENT += $(foreach ldir, $(LIBDIRS), $(LFLAGADDLIBDIR) $(ldir))

# Configuration-dependent options
CFLAGS_DEPENDENT += $(CFLAGS.$(CONFIG))
LFLAGS_DEPENDENT += $(LFLAGS.$(CONFIG)) $(LFLAGS.$(BUILD_TYPE).$(SOLID_LIBRARY))
ifeq (0, $(CPP))
CFLAGS_DEPENDENT += -std=c99
endif
CFLAGS_DEPENDENT += $(IDIRS_OPT) -c -Dasm=__asm__
CFLAGS_DEPENDENT += -Wall -Wno-error
CFLAGS_DEPENDENT += '-DTESTDIR="../../vectors"' '-DTE_SANITY_VECTOR_DIR="../../testdriver/vectors_sanity/"'

###############################################################
##                     Create target spec                    ##
###############################################################

#TARGET_SUFFIX.win32	= .exe

TARGET_SUFFIX		= $(firstword $(TARGET_SUFFIX.$(BUILD_TYPE))\
			  $(TARGET_SUFFIX.$(HOSTOS)))

TARGET_SPEC.LIBRARY	= $(OUTDIR)/$(TARGET)$(POSTFIX).a
TARGET_SPEC.EXECUTABLE	= $(OUTDIR)/$(TARGET)$(POSTFIX)$(TARGET_SUFFIX)

TARGET_INTERMEDIATE.LIBRARY.1.1	= $(INTERMEDIATE_DIR)/$(TARGET).solid.1.o
TARGET_INTERMEDIATE.LIBRARY.1.2	= $(INTERMEDIATE_DIR)/$(TARGET).solid.2.o
TARGET_INTERMEDIATE.LIBRARY.1.3	= $(INTERMEDIATE_DIR)/$(TARGET).solid.o


###############################################################
##                    Complete module list                   ##
###############################################################

MODULES += $($(VARPREFIX.$(PLATFORM))_MODULES)
MODULES += $($(VARPREFIX.$(TARGETOS))_MODULES)
MODULES += $($(VARPREFIX.unix-like.$(ULIS))_MODULES)
# Keep compatibility with IntegrIT makefiles below 0.30
MODULES += $(OBJS:.o=)
MODULES += $($(VARPREFIX.$(PLATFORM))_OBJS:.o=)
MODULES += $($(VARPREFIX.$(TARGETOS))_OBJS:.o=)
MODULES += $($(VARPREFIX.unix-like.$(ULIS))_OBJS:.o=)

###############################################################
##           Generate list of objects to be built            ##
###############################################################

ALL_OBJECTS = $(foreach module, $(MODULES), $(INTERMEDIATE_DIR)/$(module).o)

###############################################################
##        Generate list of extra libraries to be linked      ##
###############################################################

ALL_DLIBS=$(foreach lib,$(DLIBS),$(lib)$(POSTFIX_DLC).a)

###############################################################
##                     Dependency generator                  ##
###############################################################

ifneq (1,$(CPP))
vpath %.c $(SRCDIRS) $(HIFI_CSTUB_SRCDIRS)
else
vpath %.c $(SRCDIRS)
vpath %.cpp $(SRCDIRS) $(HIFI_CSTUB_SRCDIRS)
endif

COMMAND_DEPGEN =  $(CC)
COMMAND_DEPGEN += $(CFLAGS_DEPENDENT)
COMMAND_DEPGEN += $(CFLAGS_DIR_$(subst -,_,$(notdir $(dir $(1)))))
COMMAND_DEPGEN += $(CFLAGS_FILE_$(subst -,_,$(notdir $(basename $(1)))))
COMMAND_DEPGEN += $(CFLAGS)
COMMAND_DEPGEN += $(CFLAGMAKEDEP)
COMMAND_DEPGEN += -MQ $(2)
COMMAND_DEPGEN += -MQ $(2:.d=.o)
COMMAND_DEPGEN += $(1)
COMMAND_DEPGEN += -o $(2)

$(INTERMEDIATE_DIR)/%.d: %.c $(INTERMEDIATE_DIR)/dirinfo.txt
	$(call COMMAND_DEPGEN,$<,$@)

$(INTERMEDIATE_DIR)/%.d: %.cpp $(INTERMEDIATE_DIR)/dirinfo.txt
	$(call COMMAND_DEPGEN,$<,$@)

# Include dependencies
-include $(addprefix $(INTERMEDIATE_DIR)/, $(addsuffix .d, $(MODULES)))

###############################################################
##                        Compiling rules                    ##
###############################################################

COMMAND_COMPILE =  $(CC)
COMMAND_COMPILE += $(CFLAGS_DEPENDENT)
COMMAND_COMPILE += $(CFLAGS_DIR_$(subst -,_,$(notdir $(dir $(1)))))
COMMAND_COMPILE += $(CFLAGS_FILE_$(subst -,_,$(notdir $(basename $(1)))))
COMMAND_COMPILE += $(CFLAGS)
COMMAND_COMPILE += $(CFLAGMAKEOBJ)
COMMAND_COMPILE += $(1)
COMMAND_COMPILE += -o $(2) 

$(INTERMEDIATE_DIR)/%.o: %.c $(INTERMEDIATE_DIR)/dirinfo.txt
	$(call COMMAND_COMPILE,$<,$@)

$(INTERMEDIATE_DIR)/%.o: %.cpp $(INTERMEDIATE_DIR)/dirinfo.txt
	$(call COMMAND_COMPILE,$<,$@)

###############################################################
##                    Linking/archiving rules                ##
###############################################################

$(foreach lib,$(ALL_DLIBS),$(lib)): $(EXTERNAL_PROJECTS)

ALL_LIBS = $(ALL_DLIBS) $(foreach lib,$(LIBS),-l$(lib))

LD_LIST.  = $(ALL_OBJECTS)
LD_LIST.0 = $(ALL_OBJECTS)
LD_LIST.1 = $(TARGET_INTERMEDIATE.$(BUILD_TYPE).$(SOLID_LIBRARY).3)

$(TARGET_INTERMEDIATE.LIBRARY.1.1): $(ALL_OBJECTS) $(ALL_DLIBS)
	$(LD) $(LFLAGS_DEPENDENT) $(LFLAGS) -o \
	$(TARGET_INTERMEDIATE.LIBRARY.1.1) $(ALL_OBJECTS) $(ALL_LIBS)

$(TARGET_INTERMEDIATE.LIBRARY.1.2):	$(TARGET_INTERMEDIATE.LIBRARY.1.1)
	$(OBJCOPY) --keep-global-symbols=$(TARGET_SYMFILE) \
	$(TARGET_INTERMEDIATE.LIBRARY.1.1) $(TARGET_INTERMEDIATE.LIBRARY.1.2)

$(TARGET_INTERMEDIATE.LIBRARY.1.3):	$(TARGET_INTERMEDIATE.LIBRARY.1.2)
	$(OBJCOPY) --strip-unneeded \
	$(TARGET_INTERMEDIATE.LIBRARY.1.2) $(TARGET_INTERMEDIATE.LIBRARY.1.3)

$(INTERMEDIATE_DIR)/$(TARGET).ld: $(LD_LIST.$(SOLID_LIBRARY))
	$(shell $(RM) $@) $(foreach mod,$(LD_LIST.$(SOLID_LIBRARY)),$(shell $(ECHO) $(mod)>>$@))

$(TARGET_SPEC.LIBRARY): $(EXTERNAL_PROJECTS) $(LD_LIST.$(SOLID_LIBRARY)) $(INTERMEDIATE_DIR)/$(TARGET).ld
	$(RM) $@
	$(AR) rcs $@ @$(INTERMEDIATE_DIR)/$(TARGET).ld

$(TARGET_SPEC.EXECUTABLE): $(EXTERNAL_PROJECTS) $(ALL_OBJECTS) $(ALL_DLIBS)
	$(LD) $(LFLAGS_DEPENDENT) $(LFLAGS) -o \
	$(TARGET_SPEC.EXECUTABLE) $(ALL_OBJECTS) $(ALL_LIBS) 

all:    $(TARGET_SPEC.$(BUILD_TYPE))

$(foreach proj,$(EXTERNAL_PROJECTS),$(proj)):
	+make -C $@ CONFIG=$(CONFIG) all

###############################################################
##                         Clean rules                       ##
###############################################################
$(foreach proj,$(EXTERNAL_PROJECTS),$(proj)/clean):
	$(MAKE) -C $(dir $@) CONFIG=$(CONFIG) clean
$(foreach proj,$(EXTERNAL_PROJECTS),$(proj)/cleanest):
	$(MAKE) -C $(dir $@) CONFIG=$(CONFIG) cleanest

clean:	$(foreach proj,$(EXTERNAL_PROJECTS),$(proj)/clean)
#clean:	
	-$(RM) $(subst /,$(PSEP),$(INTERMEDIATE_DIR)/*.o)
	-$(RM) $(subst /,$(PSEP),$(TARGET_SPEC.$(BUILD_TYPE)))

cleanest:	$(foreach proj,$(EXTERNAL_PROJECTS),$(proj)/cleanest)
	-$(RM) $(subst /,$(PSEP),$(INTERMEDIATE_DIR)/*.d)
	-$(RM) $(subst /,$(PSEP),$(INTERMEDIATE_DIR)/*.o)
	-$(RM) $(subst /,$(PSEP),$(TARGET_SPEC.$(BUILD_TYPE)))

cleanlib:
	-$(RM) $(subst /,$(PSEP),$(TARGET_SPEC.LIBRARY)) 2>$(NULLDEV)
