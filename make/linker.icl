# |
# o---------------------------------------------------------------------o
# |
# | MAD makefile - icl/icc linker settings
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
# linker flags
#

LDFLAGS += /nologo /O3 /extlnk:.o

#
# options flags
#

ifeq ($(DEBUG),yes)
LDFLAGS += /debug:all
endif

ifeq ($(PROFILE),yes)
LDFLAGS += /Qprof-use
endif

ifeq ($(STATIC),yes)
LDFLAGS += /MD
endif

ifeq ($(PLUGIN),yes)
LDFLAGS += /MT
endif

#
# command translator
#

ICL_LD1 := -o%
ICL_LD2 := /Fe%

LD_tr = $(strip $(subst $(SPACE)/Fe , /Fe,$(call trans,$(ICL_LD1),$(ICL_LD2),$1)))

# end of makefile
