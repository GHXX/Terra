
CC       := gcc
LD       := $(CC)
AR       := ar -cvq
RM       := rm -rf

CFLAGS   += -g -c -Wall -D_DEBUG

WORKDIR  := $(shell pwd)
OBJSDIR  := .objs/

OUT      := terra
SOURCES  := $(shell find -type f -name '*.c' -not -iwholename '*tests*' | cut -c 3-)
OBJECTS  := $(SOURCES:%.c=$(OBJSDIR)%.o)
PCH_FILE := stdafx.h
OBJSPCH  := $(PCH_FILE:%.h=$(OBJSDIR)%.h.gch)
CMF      := $(word $(words $(MAKEFILE_LIST)),$(MAKEFILE_LIST))

TSRC     := $(shell find -type f -name '*.c' -iwholename '*tests*' | cut -c 3-)
TOBJS    := $(TSRC:%.c=$(OBJSDIR)%.o)
TLIBS    := $(OUT) X11 pthread m
TOUT     := $(OUT)_test

#all
all: lib $(TOBJS)
	$(LD) -o $(TOUT) $(TOBJS) -L$(WORKDIR) $(TLIBS:%=-l%)

#cleaning
.PHONY: clean
clean:
	$(RM) $(OBJSDIR) $(TOUT) lib$(OUT).a
	
#tests
.PHONY: lib
lib: $(OBJSPCH) $(OBJECTS)
	$(AR) lib$(OUT).a $(OBJECTS)

#compiling the precompiled header
$(OBJSDIR)%.h.gch:%.h $(CMF)
	@dirname $@ | xargs mkdir -p 2>/dev/null || echo "$@ already exists" >/dev/null
	@echo "Precompiling header $@..."
	@$(CC) $(CFLAGS) -o $@ -c $< || echo "error. Disabling precompiled header"
	@echo "...Done"

#compiling source files
$(OBJSDIR)%.o: %.c $(CMF)
	@dirname $@ | xargs mkdir -p 2>/dev/null || echo "$@ already exists" >/dev/null
	$(CC) $(CFLAGS) -I$(WORKDIR) $(PCH_FILE:%.h=-include %.h) -c $< -o $@
