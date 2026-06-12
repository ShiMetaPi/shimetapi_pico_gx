ifeq ($(CFG_XMEDIA_EXPORT_FLAG),)
        SDK_DIR := $(shell  cd $(CURDIR)/../.. && /bin/pwd)
endif

include $(SDK_DIR)/build/base.mk
include $(SAMPLE_DIR)/sample_base.mk

LIB_NAME := liblive_rtsp

SDK_USR_CFLAGS += $(SAMPLE_CFLAGS) $(SAMPLE_LIBS) $(SAMPLE_INCLUDES)

SRCS := $(wildcard ./common/*.c)
SRCS += $(wildcard ./liveserver/*.c)
SRCS += $(wildcard ./livestream/*.c)
SRCS += $(wildcard ./mbuffer/*.c)

include $(SDK_DIR)/build/sdk_lib_rules.mk
