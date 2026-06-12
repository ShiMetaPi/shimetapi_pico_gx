
ifneq ($(srctree),)
    KERNEL_DIR := $(srctree)
    SDK_DIR := $(shell cd $(KERNEL_DIR)/../../.. && /bin/pwd)
else
    SDK_DIR := $(shell cd $(CURDIR)/../../../../../.. && /bin/pwd)
endif

include $(SDK_DIR)/build/base.mk

export TRANSFER_TYPE := SPI

MOD_NAME := xm_msensor_chip

SRCS := ./main/msensor.c
SRCS += ./proc/msensor_chip_proc.c
SRCS += ./sensor_dev/icm42670/icm42670.c
SRCS += ./sensor_dev/icm42670/msensor_gpio.c
SRCS += ./transfer_type/spi/spi_dev.c

SDK_KER_CFLAGS += -I$(GMP_DIR)/include
SDK_KER_CFLAGS += -I$(GMP_DIR)/drv/include
SDK_KER_CFLAGS += -I$(GMP_DIR)/usr/include
SDK_KER_CFLAGS += -I$(GMP_DIR)/drv/osal/include
SDK_KER_CFLAGS += -I$(GMP_DIR)/drv/ext/msensor/msensor_chip/proc
SDK_KER_CFLAGS += -I$(GMP_DIR)/drv/ext/msensor/msensor_chip/sensor_dev/icm42670
SDK_KER_CFLAGS += -I$(GMP_DIR)/drv/ext/msensor/msensor_chip/transfer_type/spi
SDK_KER_CFLAGS += -I$(GMP_DIR)/drv/ext/msensor/msensor_spi/spi

SDK_KER_CFLAGS += -I$(GMP_DIR)/drv/ext/msensor/include
SDK_KER_CFLAGS += -I$(GMP_DIR)/drv/ext/msensor/msensor_chip/main
#SDK_KER_CFLAGS += -I$(PWD)/pub
SDK_KER_CFLAGS += -I$(GMP_DIR)/drv/ext/msensor/ext_inc
SDK_KER_CFLAGS += -I$(GMP_DIR)/drv/ext/msensor/msensor_spi
SDK_KER_CFLAGS += -I$(GMP_DIR)/drv/ext/msensor/msensor_mng/include

SDK_KER_CFLAGS += -DTRANSFER_SPI
SDK_KER_CFLAGS += -DICM42670_PARAM_PROC

SDK_KER_CFLAGS += -DCONFIG_PROC_SHOW_SUPPORT

$(info SDK_KER_CFLAGS=$(SDK_KER_CFLAGS))
$(info SRCS=$(SRCS))
$(info SDK_DIR=$(SDK_DIR))
$(info CURDIR=$(CURDIR))
include $(SDK_DIR)/build/sdk_ko_rules.mk
