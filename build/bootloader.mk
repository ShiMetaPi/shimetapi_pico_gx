###############################################################################
#
#  Copyright (c) Hunan xmedia,Chengdu xmedia,Shandong xmedia. 2021.
#  All rights reserved.
#  Create By Caizhiyong 2021/3/16
#
###############################################################################
SDK_DIR := $(shell cd $(CURDIR)/../.. && /bin/pwd)
include $(SDK_DIR)/build/base.mk


BOOT_SRCDIR   := $(BOOTLOADER_DIR)/$(CONFIG_XMEDIA_UBOOT_VER)
ifneq ($(findstring $(CONFIG_XMEDIA_CHIP_TYPE), xm7206v10 xm7206v10b xm7206v11a xm7206v12a xm7206v11 xm7206v11at), )
COMMON_CHIPSET := xmorca
else
$(error Unkown SoC!! Add $(CONFIG_XMEDIA_CHIP_TYPE) branch here for building!!)
endif

BOOT_BUILDDIR ?= $(shell pwd)/.build
SYSREG_DIR    := $(XMEDIA_CONFIGS_DIR)/$(CONFIG_XMEDIA_CHIP_TYPE)/reg
BOOT_OBJECT_DIR := $(shell find $(BOOT_SRCDIR) -name *.o)

BOOT_zIMAGE   := $(BOOT_BUILDDIR)/u-boot-$(COMMON_CHIPSET).bin
TARGET_XLSM   := $(CONFIG_XMEDIA_BOOT_REG_NAME).xlsm

BOOT_IMAGE.$(CONFIG_XMEDIA_NAND_FLASH_SUPPORT) += $(XMEDIA_NAND_IMAGE_DIR)/uboot.bin
BOOT_IMAGE.$(CONFIG_XMEDIA_EMMC_SUPPORT)       += $(XMEDIA_EMMC_IMAGE_DIR)/uboot.bin
BOOT_IMAGE.$(CONFIG_XMEDIA_SPI_SUPPORT)        += $(XMEDIA_SPI_IMAGE_DIR)/uboot.bin

MKFLAGS_BOOT = -C $(BOOT_SRCDIR) O=$(BOOT_BUILDDIR) \
    ARCH=arm CROSS_COMPILE=$(CONFIG_XMEDIA_USR_ARM_TOOLCHAINS)-

MKFLAGS_AUXCODE = -C $(BOOTLOADER_DIR)/auxcode \
		  CROSS_COMPILE=$(CONFIG_XMEDIA_USR_ARM_TOOLCHAINS)- \
		  O=$(BOOT_BUILDDIR)/auxcode

###############################################################################
.PHONY: all clean
all: $(BOOT_IMAGE.y)

$(BOOT_IMAGE.y): $(BOOT_zIMAGE)
	@$(call mkdir,$(@D))
	@cp -fv $< $@

# 此处添加 .PHONY, 强制每次都编译
.PHONY: $(BOOT_zIMAGE)
$(BOOT_zIMAGE):$(BOOT_SRCDIR) prepare
	@make $(MKFLAGS_BOOT) $(CONFIG_XMEDIA_BOOT_DEFCONFIG)
	@make $(MKFLAGS_BOOT) u-boot-z.bin

clean:
	rm -rf $(BOOT_BUILDDIR)
	rm -f $(BOOT_IMAGE.y)
###############################################################################
.PHONY: prepare auxcode
prepare: $(BOOT_BUILDDIR)/.reg $(if $(wildcard $(BOOTLOADER_DIR)/auxcode),auxcode,) FORCE
	@$(call mkdir,$(BOOT_BUILDDIR))
	for x in $(BOOT_OBJECT_DIR); do \
		(i=$${x##*$(BOOT_SRCDIR)}; j=$${i%/*}; \
		$(call mkdir,$(BOOT_BUILDDIR)/$$j); \
		cp -af $$x $(BOOT_BUILDDIR)/$$j) \
	done

auxcode:
	make $(MKFLAGS_AUXCODE) $(COMMON_CHIPSET)_config
	make $(MKFLAGS_AUXCODE) all
	cp $(BOOT_BUILDDIR)/auxcode/auxcode_*.bin $(BOOT_SRCDIR)/lotus/machine/$(COMMON_CHIPSET)/compressed/

FORCE:;

$(BOOT_BUILDDIR)/.reg: $(SYSREG_DIR)/$(TARGET_XLSM) FORCE
	$(AT)chmod 0755 $(TOOLS_DIR)/uboot_tools/regbin
	$(AT)$(TOOLS_DIR)/uboot_tools/regbin $(SYSREG_DIR)/$(TARGET_XLSM) $(BOOT_BUILDDIR)/.reg;


