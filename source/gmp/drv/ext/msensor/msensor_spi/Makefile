ifneq ($(srctree),)
    KERNEL_DIR := $(srctree)
    SDK_DIR := $(shell cd $(KERNEL_DIR)/../../.. && /bin/pwd)
else
    SDK_DIR := $(shell cd $(CURDIR)/../../../../../.. && /bin/pwd)
endif

include $(SDK_DIR)/build/base.mk

MOD_NAME := xm_msensor_spi

SRCS := msensor_spi.c

SDK_KER_CFLAGS += -I$(GMP_DIR)/include
SDK_KER_CFLAGS += -I$(GMP_DIR)/drv/osal/include
SDK_KER_CFLAGS += -I$(GMP_DIR)/drv/include

$(info SDK_KER_CFLAGS=$(SDK_KER_CFLAGS))
$(info SRCS=$(SRCS))
$(info SDK_DIR=$(SDK_DIR))

include $(SDK_DIR)/build/sdk_ko_rules.mk
