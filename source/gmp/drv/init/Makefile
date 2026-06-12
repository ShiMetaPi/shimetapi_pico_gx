ifeq ($(CFG_SDK_EXPORT_FLAG),)
    ifneq ($(srctree),)
    KERNEL_DIR := $(srctree)
    SDK_DIR := $(shell cd $(KERNEL_DIR)/../../.. && /bin/pwd)
    else
    SDK_DIR := $(shell cd $(CURDIR)/../../../.. && /bin/pwd)
    endif
endif

include $(SDK_DIR)/build/base.mk

MOD_NAME := xm_drv_init
SRCS := xmedia_drv_init.c

SDK_KER_CFLAGS += -I$(GMP_DIR)/include \
		-I$(GMP_DIR)/drv/include \
		-I$(GMP_DIR)/drv/osal/include

include $(SDK_DIR)/build/sdk_ko_rules.mk
