###############################################################################
#
#  Copyright (c) Hunan xmedia,Chengdu xmedia,Shandong xmedia. 2021.
#  All rights reserved.
#  Create By Caizhiyong 2021/3/16
#
###############################################################################
SDK_DIR := $(shell cd $(CURDIR)/../.. && /bin/pwd)
include $(SDK_DIR)/build/base.mk

ARM_ARCH=$(CONFIG_XMEDIA_CPU_ARCH)

KERNEL_SRCDIR   := $(SDK_DIR)/source/kernel/$(CONFIG_XMEDIA_KERNEL_VERSION)
KERNEL_BUILDDIR ?= $(shell pwd)/.build
KERNEL_uIMAGE   := $(KERNEL_BUILDDIR)/arch/$(ARM_ARCH)/boot/uImage
KERNEL_zIMAGE   := $(KERNEL_BUILDDIR)/arch/$(ARM_ARCH)/boot/zImage
KERNEL_HWGZIP_IMAGE-dtb := $(KERNEL_BUILDDIR)/arch/$(ARM_ARCH)/boot/hwzImage-dtb

FAKEROOT_INITRAMDISK := fakeroot -- bash $(TOOLS_DIR)/bin/fakeroot-scripts-initramdisk

#$(1)=$(INSTALL_DIR), $(2)=$(KERNEL_BUILDDIR)
install_modules = ( test -d $(1) || mkdir -p $(1); \
                     rm -rf $(1)/lib/modules; \
                     $(MAKE) ARCH=$(ARM_ARCH) -C $(2) modules_install INSTALL_MOD_PATH=$(1); \
                     rm -f $(1)/lib/modules/*/build; \
                     rm -f $(1)/lib/modules/*/source; \
                     rm -rf $(1)/lib/modules/*/kernel/drivers/gmp; \
                     list=$(find $(1)/lib/modules/ -name *.ko) && if [ -n "${list}" ]; then ($(CONFIG_XMEDIA_KERNEL_ARM_TOOLCHAINS)-strip --strip-unneeded ${list}) fi; )

KERNEL_IMAGE.y =
KERNEL_IMAGE.$(CONFIG_XMEDIA_NAND_FLASH_SUPPORT) += $(XMEDIA_NAND_IMAGE_DIR)/kernel
KERNEL_IMAGE.$(CONFIG_XMEDIA_EMMC_SUPPORT)       += $(XMEDIA_EMMC_IMAGE_DIR)/kernel
KERNEL_IMAGE.$(CONFIG_XMEDIA_SPI_SUPPORT)        += $(XMEDIA_SPI_IMAGE_DIR)/kernel

MKFLAGS_KERNEL = -C $(KERNEL_SRCDIR) O=$(KERNEL_BUILDDIR) \
    ARCH=$(ARM_ARCH) CROSS_COMPILE=$(CONFIG_XMEDIA_KERNEL_ARM_TOOLCHAINS)- CONFIG_GMP=$(XMEDIA_DRV_BUILDTYPE)

MKFLAGS_KERNEL += KBUILD_MODPOST_WARN=1 LOCALVERSION=

MKFLAGS_QUIK_START = CONFIG_INITRAMFS_SOURCE="$(XMEDIA_INITRD_DIR)" CONFIG_HW_GZIP_TOOL=$(HWGZIP)

create_initramfs_kernel = $(if $(CONFIG_XMEDIA_QUICK_START),$(call install_modules,$(XMEDIA_ROOTFS_DIR),$(KERNEL_BUILDDIR)) && \
			  $(FAKEROOT_INITRAMDISK) "$(XMEDIA_INITRD_DIR)" "$(MAKE) $(MKFLAGS_KERNEL) $(MKFLAGS_QUIK_START) $(1)",)

.PHONY: all clean
all: $(KERNEL_IMAGE.y)
ifeq (_$(CONFIG_XMEDIA_QUICK_START),_y)
$(KERNEL_IMAGE.y): $(KERNEL_HWGZIP_IMAGE-dtb)
	@$(call mkdir,$(@D))
	@cp -fv $< $@

$(KERNEL_HWGZIP_IMAGE-dtb):$(KERNEL_uIMAGE)
	@echo;
	@echo $(KERNEL_HWGZIP_IMAGE-dtb) is ready!
else
$(KERNEL_IMAGE.y): $(KERNEL_uIMAGE)
	@$(call mkdir,$(@D))
	@cp -fv $< $@
endif

# 此处添加 .PHONY, 强制内核每次都编译
.PHONY: $(KERNEL_uIMAGE)
$(KERNEL_uIMAGE): $(KERNEL_SRCDIR) prepare
	$(AT)$(MAKE) $(MKFLAGS_KERNEL) $(CONFIG_XMEDIA_KERNEL_DEFCONFIG)
	$(AT)$(MAKE) $(MKFLAGS_KERNEL) uImage
	$(AT)$(MAKE) $(MKFLAGS_KERNEL) modules
	$(call create_initramfs_kernel, uImage)

clean:
	rm -rf $(KERNEL_BUILDDIR)
	rm -f $(KERNEL_IMAGE.y) $(KERNEL_IMAGE.n)
	$(AT)rm -rf $(KERNEL_SRCDIR)/drivers/gmp
	$(AT)rm -rf $(GMP_DIR)/drv/obj_$(OBJ_KERNEL_VERSION)/$(KERNEL_TOOLCHAIN)/$(OBJ_BUILD_TYPE)/$(OBJ_LOG_MODE)/xm_* 
	$(AT)rm -rf $(GMP_DIR)/drv/obj_$(OBJ_KERNEL_VERSION)/$(KERNEL_TOOLCHAIN)/$(OBJ_BUILD_TYPE)/$(OBJ_LOG_MODE)/.*.cmd

prepare:
ifeq (_$(CONFIG_XMEDIA_LINUX_4_9_Y),_y) 
	$(call mkdir,$(KERNEL_BUILDDIR)/drivers/gmp/drv) 
	$(AT)(cd $(KERNEL_BUILDDIR)/drivers && rm -rf gmp && ln -s $(GMP_DIR)/drv gmp) 
endif
	$(AT)(cd $(KERNEL_SRCDIR)/drivers && rm -rf gmp && ln -s $(GMP_DIR)/drv gmp)
