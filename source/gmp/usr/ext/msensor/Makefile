SDK_DIR ?= $(shell cd $(CURDIR)/../../../../.. && /bin/pwd)
$(warning SDK_DIR=$(SDK_DIR))
include $(SDK_DIR)/build/base.mk

LIB_NAME := libxmedia_msensor

SRCS := xmedia_msensor.c

SDK_USR_CFLAGS += -I$(GMP_DIR)/include
SDK_USR_CFLAGS += -I$(GMP_DIR)/usr/include
SDK_USR_CFLAGS += -I$(GMP_DIR)/drv/include
SDK_USR_CFLAGS += -I$(GMP_DIR)/drv/ext/msensor/include
SDK_USR_CFLAGS += -I$(GMP_DIR)/drv/ext/msensor/msensor_mng/include

$(info SDK_USR_CFLAGS=$(SDK_USR_CFLAGS))
$(info SRCS=$(SRCS))
$(info SDK_DIR=$(SDK_DIR))

include $(SDK_DIR)/build/sdk_lib_rules.mk
