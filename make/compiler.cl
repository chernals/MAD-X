# |
# o---------------------------------------------------------------------o
# |
# | MAD makefile - cl compiler settings (Visual Studio C++)
# |
# o---------------------------------------------------------------------o
# |
# | Methodical Accelerator Design
# |
# | Copyright (c) 2011+ CERN, mad@cern.ch
# |
# | For more information, see http://cern.ch/mad
# |
# o---------------------------------------------------------------------o
# |
# | $Id$
# |

#
# makedep
#

# must use mcpp, http://mcpp.sourceforge.net/
# TODO: not supported by cl!!!
# CDEP := $(CC) /nolog /c /Zs /showIncludes

#
# compiler
#

CFLAGS   := /nologo /Wall /O2 /c
CXXFLAGS := /nologo /Wall /O2 /c

#
# options flags
#

ifeq ($(DEBUG),yes)
CFLAGS   += /Zi /Yd
CXXFLAGS += /Zi /Yd
endif

ifeq ($(PROFILE),yes)
CFLAGS   +=
CXXFLAGS +=
endif

#
# extra flags
#

CFLAGS   += /nologo /fp:precise /EHc /Zm1000
CXXFLAGS += /nologo /fp:precise /EHc /Zm1000

#
# command translator
#

CL_CC1 := -D%  -I%  -o%
CL_CC2 := /D%  /I%  /Fo%

CC_tr  = $(strip $(subst $(SPACE)/Fo , /Fo,$(call trans,$(CL_CC1),$(CL_CC2),$1)))
CXX_tr = $(CC_tr)

# end of makefile
