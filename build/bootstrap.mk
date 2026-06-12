###############################################################################
#
#  Create By Lane 2024/1/31
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

BOOTSTRAP_BUILDDIR ?= $(shell pwd)/.build
SYSREG_DIR    := $(XMEDIA_CONFIGS_DIR)/$(CONFIG_XMEDIA_CHIP_TYPE)/reg
BOOT_OBJECT_DIR := $(shell find $(BOOT_SRCDIR) -name *.o)

BOOT_zIMAGE   := $(BOOTSTRAP_BUILDDIR)/u-boot-$(COMMON_CHIPSET).bin
UBOOT_REG_BIN :=bootstrap_reg_info.bin
TARGET_XLSM   := $(CONFIG_XMEDIA_BOOT_REG_NAME).xlsm

BOOT_IMAGE.$(CONFIG_XMEDIA_NAND_FLASH_SUPPORT) += $(XMEDIA_NAND_IMAGE_DIR)/bootstrap.bin
BOOT_IMAGE.$(CONFIG_XMEDIA_EMMC_SUPPORT)       += $(XMEDIA_EMMC_IMAGE_DIR)/bootstrap.bin
BOOT_IMAGE.$(CONFIG_XMEDIA_SPI_SUPPORT)        += $(XMEDIA_SPI_IMAGE_DIR)/bootstrap.bin

MKFLAGS_BOOT = -C $(BOOT_SRCDIR) O=$(BOOTSTRAP_BUILDDIR) \
    ARCH=arm CROSS_COMPILE=$(CONFIG_XMEDIA_USR_ARM_TOOLCHAINS)-

###############################################################################
.PHONY: all clean
all: $(BOOT_IMAGE.y)

$(BOOT_IMAGE.y): $(BOOT_zIMAGE)
	@$(call mkdir,$(@D))
	@cp -fv $< $@

# 此处添加 .PHONY, 强制每次都编译
.PHONY: $(BOOT_zIMAGE)
$(BOOT_zIMAGE):$(BOOT_SRCDIR) prepare
	@make $(MKFLAGS_BOOT) $(shell echo $(CONFIG_XMEDIA_BOOT_DEFCONFIG) | sed 's/tiny_//' | sed 's/quickstart_//')
	@make $(MKFLAGS_BOOT) u-boot-z.bin


clean:
	rm -rf $(BOOTSTRAP_BUILDDIR)
	rm -f $(BOOT_IMAGE.y)

###############################################################################
.PHONY: prepare
prepare:$(BOOTSTRAP_BUILDDIR)/.reg FORCE
	@$(call mkdir,$(BOOTSTRAP_BUILDDIR))
	for x in $(BOOT_OBJECT_DIR); do \
		(i=$${x##*$(BOOT_SRCDIR)}; j=$${i%/*}; \
		$(call mkdir,$(BOOTSTRAP_BUILDDIR)/$$j); \
		cp -af $$x $(BOOTSTRAP_BUILDDIR)/$$j) \
	done

FORCE:;

$(BOOTSTRAP_BUILDDIR)/.reg: $(SYSREG_DIR)/$(TARGET_XLSM)
	$(AT)chmod 0755 $(TOOLS_DIR)/uboot_tools/regbin
	$(AT)$(TOOLS_DIR)/uboot_tools/regbin $(SYSREG_DIR)/$(TARGET_XLSM) $(BOOTSTRAP_BUILDDIR)/.reg;

