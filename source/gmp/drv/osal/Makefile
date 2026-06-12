ifneq ($(srctree),)
    KERNEL_DIR := $(srctree)
    SDK_DIR := $(shell cd $(KERNEL_DIR)/../../.. && /bin/pwd)
else
    SDK_DIR := $(shell cd $(CURDIR)/../../../.. && /bin/pwd)
endif

include $(SDK_DIR)/build/base.mk

obj-$(XMEDIA_DRV_BUILDTYPE) += linux/kernel/

.PHONY: all clean

all:
	$(AT)$(MAKE) -C linux/kernel

clean:
	$(AT)$(MAKE) -C linux/kernel clean
