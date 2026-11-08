# PSL1GHT standalone AirPlay receiver. Inputs remain fixed and local-first.
.SUFFIXES:
.DEFAULT_GOAL := all

ifeq ($(strip $(PSL1GHT)),)
$(error Set PSL1GHT to the installed ps3dev directory)
endif
ifeq ($(strip $(MBEDTLS_INCLUDE)),)
$(error Set MBEDTLS_INCLUDE to Mbed TLS 3.6.7 headers)
endif
ifeq ($(strip $(MBEDTLS_CRYPTO_LIBRARY)),)
$(error Set MBEDTLS_CRYPTO_LIBRARY to the matching PPU libmbedcrypto.a)
endif
ifeq ($(strip $(PS3DK)),)
PS3DK := $(PS3DEV)/ps3dk
endif
export PS3DK

include $(PSL1GHT)/ppu_rules

TARGET := ps3-airplay-receiver
BUILD := build
SOURCES := source
PROJECT_SOURCES := ../../src/discovery ../../src/protocol ../../src/crypto ../../src/codec ../../src/audio ../../src/observability
PROJECT_CFILES := \
	../../src/discovery/raop_mdns.c \
	../../src/discovery/raop_mdns_service.c \
	../../src/protocol/raop_session.c \
	../../src/protocol/raop_sdp.c \
	../../src/protocol/raop_rtp.c \
	../../src/protocol/raop_receiver.c \
	../../src/crypto/raop_crypto.c \
	../../src/codec/alac.c \
	../../src/codec/raop_alac.c \
	../../src/audio/pcm_ring.c \
	../../src/audio/pcm_gain.c \
	../../src/observability/receiver_log.c
INCLUDES := include
TITLE := PS3 AirPlay 1 Receiver
APPID := AIRPLAY02
CONTENTID := UP0001-$(APPID)_00-0000000000000000
APP_VERSION ?= 0.2.4
RTSP_PORT ?= 5000

ifeq ($(LEGACY_RUNTIME),1)
# The 2020 PSL1GHT headers declare select() through net/select.h. Do not set
# __SELECT_H__ here; that guard would hide the declaration needed by RAOP.
SELECT_HEADER_COMPAT :=
else
# PS3DK's newer sys/select.h needs its guard pre-defined to avoid a conflict
# with the network header included by the SDK.
SELECT_HEADER_COMPAT := -D__SELECT_H__
endif

CFLAGS := -O2 -Wall -Wextra -Werror \
	-Wno-pointer-to-int-cast -Wno-int-to-pointer-cast \
	-mcpu=cell $(MACHDEP) \
	-DMBEDTLS_NO_PLATFORM_ENTROPY -DMBEDTLS_PLATFORM_MS_TIME_ALT \
	-DMBEDTLS_USER_CONFIG_FILE=\"mbedtls_ps3_user_config.h\" \
	-DRAOP_USE_PSL1GHT -DRAOP_VERBOSE_LOGGING=1 \
	-DAPP_VERSION_STRING=\"$(APP_VERSION)\" \
	-DRAOP_RTSP_PORT=$(RTSP_PORT) $(SELECT_HEADER_COMPAT) \
	-include $(PS3DK)/ppu/include/sys/select.h \
	-I$(MBEDTLS_INCLUDE) $(INCLUDE)
LIBS := -laudio -lgcm_sys -lrsx -lio -lsysutil -lnetctl -lnet -lsysmodule -lrt -lm \
	$(MBEDTLS_CRYPTO_LIBRARY)

ifneq ($(BUILD),$(notdir $(CURDIR)))
export OUTPUT := $(CURDIR)/$(TARGET)
export VPATH := $(CURDIR)/$(SOURCES) $(foreach dir,$(PROJECT_SOURCES),$(CURDIR)/$(dir))
export DEPSDIR := $(CURDIR)/$(BUILD)
export BUILDDIR := $(CURDIR)/$(BUILD)
CFILES := $(notdir $(wildcard $(SOURCES)/*.c) $(PROJECT_CFILES))
export OFILES := $(CFILES:.c=.o)
export INCLUDE := $(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
	-I$(PS3DK)/ppu/include \
	$(foreach dir,$(PROJECT_SOURCES),-I$(CURDIR)/$(dir)) \
	-I$(MBEDTLS_INCLUDE) $(LIBPSL1GHT_INC) -I$(CURDIR)/$(BUILD)
export LIBPATHS := -L$(PS3DK)/ppu/lib $(LIBPSL1GHT_LIB)
export LD := $(CC)

.PHONY: all clean build
all: $(BUILD)
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile
$(BUILD):
	@mkdir -p $@
clean:
	@rm -rf $(BUILD) $(TARGET).elf $(TARGET).self
else
.DEFAULT_GOAL := $(OUTPUT).elf
DEPENDS := $(OFILES:.o=.d)
$(OUTPUT).elf: $(OFILES)
-include $(DEPENDS)
endif
