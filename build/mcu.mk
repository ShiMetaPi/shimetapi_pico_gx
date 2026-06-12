###############################################################################
#
#  Copyright (c) xmedia. 2021.
#  All rights reserved.
#
###############################################################################
SDK_DIR := $(shell cd $(CURDIR)/.. && /bin/pwd)
include $(SDK_DIR)/build/base.mk

ifneq ($(findstring $(CONFIG_XMEDIA_CHIP_TYPE), xm7206v10 xm7206v10b xm7206v11a xm7206v12a xm7206v11 xm7206v11at), )
COMMON_CHIPSET := xmorca
else
$(error Unkown SoC!! Add $(CONFIG_XMEDIA_CHIP_TYPE) branch here for building!!)
endif

MCU_SRCDIR   := $(SDK_DIR)/mcu/riscv
QS_MCU_BUILDDIR ?= $(shell pwd)/.build
QS_MCU_IMAGE   := $(QS_MCU_BUILDDIR)/mcu.bin

MKFLAGS_QS_MCU = -C $(MCU_SRCDIR) CROSS_COMPILE=riscv64-unknown-elf- O=$(QS_MCU_BUILDDIR)

.PHONY: all qs_mcu clean
all: qs_mcu

qs_mcu: $(QS_MCU_IMAGE)
	@$(call mkdir,$(BOOT_BUILDDIR))
	@cp -fv $< $(BOOT_BUILDDIR)/qs_mcu.bin

# 此处添加 .PHONY, 强制每次都编译
.PHONY: $(QS_MCU_IMAGE)
$(QS_MCU_IMAGE): $(MCU_SRCDIR) prepare
	$(AT)$(MAKE) $(MKFLAGS_QS_MCU) $(COMMON_CHIPSET)_defconfig
	$(AT)$(MAKE) $(MKFLAGS_QS_MCU)

clean:
	rm -rf $(QS_MCU_BUILDDIR)
	rm -f $(BOOT_BUILDDIR)/qs_mcu.bin

prepare:
