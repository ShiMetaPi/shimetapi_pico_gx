ifneq ($(srctree),)
    KERNEL_DIR := $(srctree)
    SDK_DIR := $(shell cd $(KERNEL_DIR)/../../.. && /bin/pwd)
else
    SDK_DIR := $(shell cd $(CURDIR)/../../../../../.. && /bin/pwd)
endif

include $(SDK_DIR)/build/base.mk
include $(SDK_DIR)/build/version.mk

OSAL_ROOT := $(GMP_DIR)/drv/osal

MOD_NAME := xm_osal

SRCS := osal_fileops.c \
	osal_vmalloc.c \
	osal_addr.c \
	osal_init.c \
	osal_atomic.c \
	osal_barrier.c \
	osal_cache.c \
	osal_debug.c \
	osal_device.c \
	osal_interrupt.c \
	osal_math.c \
	osal_mutex.c \
	osal_proc.c \
	osal_schedule.c \
	osal_semaphore.c \
	osal_spinlock.c \
	osal_string.c \
	osal_task.c \
	osal_timer.c \
	osal_wait.c \
	osal_workqueue.c \
	osal_notifier.c \
	osal_platform.c

SRCS += ./media/base.c \
	./media/media.c \
	./mmz/media-mem.c \
	./mmz/mmz-userdev.c \
	./mmz/drv_mmz.c \
	./mmz/allocator.c 

ifeq ($(CONFIG_CMA),y)
SRCS += ./mmz/cma_allocator.c
endif 

SDK_KER_CFLAGS += -D$(CHIPSET) -DRELEASE -DUSER_BIT_32 -DKERNEL_BIT_32 -Wno-date-time -D_GNU_SOURCE

SDK_KER_CFLAGS += -I$(OSAL_ROOT)/include \
		-I$(OSAL_ROOT)/linux/kernel \
		-I$(OSAL_ROOT)/linux/kernel/media \
		-I$(OSAL_ROOT)/linux/kernel/mmz \
		-I$(GMP_DIR)/drv/include \
		-I$(GMP_DIR)/include  

include $(SDK_DIR)/build/sdk_ko_rules.mk
