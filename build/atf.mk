###############################################################################
#
#  Copyright (c) xmedia. 2021.
#  All rights reserved.
#
###############################################################################
SDK_DIR := $(shell cd $(CURDIR)/../.. && /bin/pwd)
include $(SDK_DIR)/build/base.mk

ifneq ($(findstring $(CONFIG_XMEDIA_CHIP_TYPE), xm7206v10 xm7206v10b xm7206v11a xm7206v12a xm7206v11 xm7206v11at), )
COMMON_CHIPSET := xmorca
else
$(error Unkown SoC!! Add $(CONFIG_XMEDIA_CHIP_TYPE) branch here for building!!)
endif

ATF_VER = trusted-firmware-a-2.8
ATF_BUILD_TYPE = $(if $(CONFIG_XMEDIA_QUICK_START),release,debug)
ATF_SRCDIR   := $(SDK_DIR)/source/trusted-firmware/$(ATF_VER)
ATF_BUILDDIR ?= $(shell pwd)/.build
ATF_BL31_IMAGE   := $(ATF_BUILDDIR)/build/$(COMMON_CHIPSET)/$(ATF_BUILD_TYPE)/bl31.bin

ATF_IMAGE.y =
ATF_IMAGE.$(CONFIG_XMEDIA_NAND_FLASH_SUPPORT) += $(XMEDIA_NAND_IMAGE_DIR)/bl31
ATF_IMAGE.$(CONFIG_XMEDIA_EMMC_SUPPORT)       += $(XMEDIA_EMMC_IMAGE_DIR)/bl31
ATF_IMAGE.$(CONFIG_XMEDIA_SPI_SUPPORT)        += $(XMEDIA_SPI_IMAGE_DIR)/bl31

MKFLAGS_ATF = -C $(ATF_SRCDIR) BUILD_BASE=$(ATF_BUILDDIR)/build \
    CROSS_COMPILE=aarch64-gcc12.2.0-linux- PLAT=$(COMMON_CHIPSET) $(if $(CONFIG_XMEDIA_QUICK_START),,DEBUG=1) NEED_BL32=no SPD=none

.PHONY: all clean
all: $(ATF_IMAGE.y)
$(ATF_IMAGE.y): $(ATF_BL31_IMAGE)
	@$(call mkdir,$(@D))
	@cp -fv $< $@

# 此处添加 .PHONY, 强制内核每次都编译
.PHONY: $(ATF_BL31_IMAGE)
$(ATF_BL31_IMAGE): $(ATF_SRCDIR) prepare
	$(AT)$(MAKE) $(MKFLAGS_ATF) bl31

clean:
	rm -rf $(ATF_BUILDDIR)
	rm -f $(ATF_IMAGE.y) $(ATF_IMAGE.n)

prepare:
