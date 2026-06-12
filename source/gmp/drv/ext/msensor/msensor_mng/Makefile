ifneq ($(srctree),)
    KERNEL_DIR := $(srctree)
    SDK_DIR := $(shell cd $(KERNEL_DIR)/../../.. && /bin/pwd)
else
    SDK_DIR := $(shell cd $(CURDIR)/../../../../../.. && /bin/pwd)
endif

include $(SDK_DIR)/build/base.mk

MOD_NAME := xm_msensor_mng

SRCS = ./src/msensor_exe.c
SRCS += ./src/msensor_buf.c
SRCS += ./src/msensor_mng_proc.c

SDK_KER_CFLAGS += -I$(GMP_DIR)/include
SDK_KER_CFLAGS += -I$(GMP_DIR)/drv/include
SDK_KER_CFLAGS += -I$(GMP_DIR)/usr/include
SDK_KER_CFLAGS += -I$(GMP_DIR)/drv/osal/include

SDK_KER_CFLAGS += -I$(GMP_DIR)/drv/ext/msensor/msensor_mng/include
SDK_KER_CFLAGS += -I$(GMP_DIR)/drv/ext/msensor/include
SDK_KER_CFLAGS += -I$(GMP_DIR)/drv/ext/msensor/ext_inc
#SDK_KER_CFLAGS += -I$(GMP_DIR)/drv/msensor/msensor_chip/main/

SDK_KER_CFLAGS += -DCONFIG_PROC_SHOW_SUPPORT

include $(SDK_DIR)/build/sdk_ko_rules.mk

