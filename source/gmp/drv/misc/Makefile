ifneq ($(srctree),)
    KERNEL_DIR := $(srctree)
    SDK_DIR := $(shell cd $(KERNEL_DIR)/../../.. && /bin/pwd)
else
    SDK_DIR := $(shell cd $(CURDIR)/../../../.. && /bin/pwd)
endif

include $(SDK_DIR)/build/base.mk
include $(SDK_DIR)/build/version.mk

MOD_NAME := xm_misc

SRCS := ./src/drv_misc.c
SRCS += ./src/drv_osd_draw_time.c
SRCS += ./src/drv_sensor.c

SDK_KER_CFLAGS += -I$(GMP_DIR)/include
SDK_KER_CFLAGS += -I$(GMP_DIR)/drv/osal/include
SDK_KER_CFLAGS += -I$(GMP_DIR)/drv/common/include
SDK_KER_CFLAGS += -I$(GMP_DIR)/drv/common/task
SDK_KER_CFLAGS += -I$(GMP_DIR)/drv/include
SDK_KER_CFLAGS += -I$(GMP_DIR)/drv/misc/include
SDK_KER_CFLAGS += -I$(GMP_DIR)/usr/include
SDK_KER_CFLAGS += -I$(GMP_DIR)/usr/common/include

$(info SDK_KER_CFLAGS=$(SDK_KER_CFLAGS))
$(info SRCS=$(SRCS))
$(info SDK_DIR=$(SDK_DIR))

include $(SDK_DIR)/build/sdk_ko_rules.mk
