#####################################################
# Build Dependence
# prepare: bootargs
# boot: prepare
# linux: prepare
# gmp: linux
# component: linux gmp
# sample: gmp component
# rootfs: linux gmp component
# fs_image: rootfs
#
#
#####################################################

.EXPORT_ALL_VARIABLES:

SDK_DIR := $(shell pwd)

include $(SDK_DIR)/build/base.mk

#build_targe
ifeq ($(CHIPARCH), xmorca)
build_targe = prepare uboot $(if $(CONFIG_XMEDIA_QUICK_START),bootstrap,) linux gmp rootfs bootargs fs_image sample
else
build_targe = prepare
endif

CFG_SDK_EXPORT_FLAG := y

.DEFAULT_GOAL := help

.PHONY: help
help:
	$(AT)cat $(BUILD_DIR)/MakeHelp

.PHONY: build clean prepare

build: $(build_targe)

clean:linux_clean uboot_clean atf_clean rootfs_clean gmp_clean sample_clean
	$(AT)rm $(OUT_DIR) -rf

prepare:

.PHONY: uboot uboot_clean
uboot: $(if $(CONFIG_XMEDIA_QUICK_START),qs_mcu,) prepare
	$(AT)$(MAKE) -C $(BOOTLOADER_DIR)

uboot_clean: $(if $(CONFIG_XMEDIA_QUICK_START),qs_mcu_clean,)
	$(AT)$(MAKE) -C $(BOOTLOADER_DIR) clean

.PHONY: bootstrap bootstrap_clean
bootstrap:
	$(AT)$(MAKE) -C $(BOOTLOADER_DIR) -f $(SDK_DIR)/build/bootstrap.mk
bootstrap_clean:
	$(AT)$(MAKE) -C $(BOOTLOADER_DIR) -f $(SDK_DIR)/build/bootstrap.mk clean

.PHONY: linux linux_clean
linux:$(if $(CONFIG_XMEDIA_QUICK_START),initrd,)
	$(AT)$(MAKE) -C $(KERNEL_DIR)

linux_clean:$(if $(CONFIG_XMEDIA_QUICK_START),initrd_clean,)
	$(AT)$(MAKE) -C $(KERNEL_DIR) clean

.PHONY: atf atf_clean
atf:
	$(AT)$(MAKE) -C $(ATF_DIR)

atf_clean:
	$(AT)$(MAKE) -C $(ATF_DIR) clean

.PHONY: qs_mcu qs_mcu_clean
qs_mcu:
	$(AT)$(MAKE) -C $(QS_MCU_DIR)

qs_mcu_clean:
	$(AT)$(MAKE) -C $(QS_MCU_DIR) clean

.PHONY: gmp gmp_clean
gmp:linux
	$(AT)$(MAKE) -C $(GMP_DIR)

gmp_clean:
	$(AT)$(MAKE) -C $(GMP_DIR) clean

.PHONY: component component_clean
component: linux gmp
	$(AT)$(MAKE) -C $(COMPONENT_DIR)

component_clean:
	$(AT)$(MAKE) -C $(COMPONENT_DIR) clean

.PHONY: sample sample_clean
sample:gmp
	$(AT)$(MAKE) -C $(SAMPLE_DIR)

sample_clean:
	$(AT)$(MAKE) -C $(SAMPLE_DIR) clean

.PHONY: tools tools_clean
tools:
	$(AT)$(MAKE) -C $(TOOLS_DIR)

tools_clean:
	$(AT)$(MAKE) -C $(TOOLS_DIR) clean

.PHONY: project project_clean
project:sample
	$(AT)$(MAKE) -C $(PROJECT_DIR)

project_clean:
	$(AT)$(MAKE) -C $(PROJECT_DIR) clean

.PHONY: rootfs rootfs_clean rootfs_post
rootfs: linux
	$(AT)$(MAKE) -C $(ROOTFS_DIR)
	$(AT)$(MAKE) rootfs_post

rootfs_post:
	$(AT)$(MAKE) -C $(ROOTFS_DIR) rootfs_post

rootfs_clean:
	$(AT)$(MAKE) -C $(ROOTFS_DIR) clean

.PHONY: initrd initrd_clean
initrd:
	$(AT)$(MAKE) -C $(INITRD_DIR)

initrd_clean:
	$(AT)$(MAKE) -C $(INITRD_DIR) clean

.PHONY: menuconfig
$(KCONFIG_EXE):
	$(AT)$(MAKE) -C $(KCONFIG_SRC_DIR)

menuconfig: $(KCONFIG_EXE)
	$(AT)$(KCONFIG_EXE) $(KCONFIG_CFG)
	$(AT)echo "$(MAKE) $@ over"

include $(SDK_DIR)/build/rootfs_image.mk
