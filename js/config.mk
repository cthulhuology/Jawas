
# Set os+release dependent make variables
OS_ARCH         := $(subst /,_,$(shell uname -s | sed /\ /s//_/))

# Attempt to differentiate between SunOS 5.4 and x86 5.4
OS_CPUARCH      := $(shell uname -m)
ifeq ($(OS_CPUARCH),i86pc)
OS_RELEASE      := $(shell uname -r)_$(OS_CPUARCH)
endif

# Virtually all Linux versions are identical.
# Any distinctions are handled in linux.h
ifeq ($(OS_ARCH),Linux)
OS_CONFIG      := Linux_All
else
ifeq ($(OS_ARCH),Darwin)
OS_CONFIG      := Darwin
else
OS_CONFIG       := $(OS_ARCH)$(OS_OBJTYPE)$(OS_RELEASE)
endif
endif

ASFLAGS         =
DEFINES         =

INSTALL	= $(DEPTH)/../../dist/$(OBJDIR)/bin/nsinstall
CP = cp

ifdef BUILD_OPT
OPTIMIZER  = -O
DEFINES    += -UDEBUG -DNDEBUG -UDEBUG_$(shell whoami)
OBJDIR_TAG = _OPT
else
OPTIMIZER  = -g
DEFINES    += -DDEBUG -DDEBUG_$(shell whoami)
OBJDIR_TAG = _DBG
endif

ifeq ($(OS_ARCH),Darwin)
SO_SUFFIX = dylib
else
SO_SUFFIX = so
endif

NS_USE_NATIVE = 1

include $(DEPTH)/config/$(OS_CONFIG).mk

# Name of the binary code directories
ifdef BUILD_IDG
OBJDIR          = $(OS_CONFIG)$(OBJDIR_TAG).OBJD
else
OBJDIR          = $(OS_CONFIG)$(OBJDIR_TAG).OBJ
endif
VPATH           = $(OBJDIR)

# Automatic make dependencies file
DEPENDENCIES    = $(OBJDIR)/.md

# Library name
LIBDIR := lib
ifeq ($(CPU_ARCH), x86_64)
LIBDIR := lib64
endif

