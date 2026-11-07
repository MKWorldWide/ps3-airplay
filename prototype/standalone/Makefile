# PSL1GHT standalone low-volume tone proof of concept.
.SUFFIXES:
.DEFAULT_GOAL := all

ifeq ($(strip $(PSL1GHT)),)
$(error Set PSL1GHT to the installed ps3dev directory)
endif

include $(PSL1GHT)/ppu_rules

TARGET   := ps3-airplay-tone
BUILD    := build
SOURCES  := source
EXTRA_SOURCES := ../../src/audio
INCLUDES := include

TITLE := PS3 AirPlay Audio Tone Test
APPID := AIRPLAY1
CONTENTID := UP0001-$(APPID)_00-0000000000000000

CFLAGS := -O2 -Wall -mcpu=cell $(MACHDEP) $(INCLUDE)
LIBS   := -laudio -lsysutil -lrt -llv2 -lm

ifneq ($(BUILD),$(notdir $(CURDIR)))
export OUTPUT := $(CURDIR)/$(TARGET)
export VPATH := $(CURDIR)/$(SOURCES) $(CURDIR)/$(EXTRA_SOURCES)
export DEPSDIR := $(CURDIR)/$(BUILD)
export BUILDDIR := $(CURDIR)/$(BUILD)

CFILES := $(notdir $(wildcard $(SOURCES)/*.c) $(wildcard $(EXTRA_SOURCES)/*.c))
export OFILES := $(CFILES:.c=.o)
export INCLUDE := $(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) -I$(CURDIR)/$(EXTRA_SOURCES) $(LIBPSL1GHT_INC) -I$(CURDIR)/$(BUILD)
export LIBPATHS := $(LIBPSL1GHT_LIB)
export LD := $(CC)

.PHONY: all $(BUILD) clean
all: $(BUILD)

$(BUILD):
	@mkdir -p $@
	@$(MAKE) --no-print-directory -C $@ -f $(CURDIR)/Makefile

clean:
	@rm -rf $(BUILD) $(TARGET).elf $(TARGET).self
else
.DEFAULT_GOAL := $(OUTPUT).elf
DEPENDS := $(OFILES:.o=.d)
$(OUTPUT).self: $(OUTPUT).elf
$(OUTPUT).elf: $(OFILES)
-include $(DEPENDS)
endif
