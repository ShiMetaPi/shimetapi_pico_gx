ifeq ($(SDK_CFGFILE),)
    export SDK_CFGFILE=cfg.mk
endif

include $(SDK_DIR)/$(SDK_CFGFILE)

export CHIPSET := $(CONFIG_XMEDIA_CHIP_TYPE)
export CHIPARCH := $(CONFIG_XMEDIA_CHIP_ARCH)
export TOOLCHAIN := $(CONFIG_XMEDIA_USR_ARM_TOOLCHAINS)
export KERNEL_TOOLCHAIN := $(CONFIG_XMEDIA_KERNEL_ARM_TOOLCHAINS)
export OSTYPE := linux

##### the build flags #####
ifeq ($(CONFIG_XMEDIA_CHIP_ARCH), xmorca)
SDK_USR_CFLAGS := -mcpu=cortex-a7 -mfloat-abi=hard -mfpu=neon-vfpv4 -Wall -fPIC -Os -fno-aggressive-loop-optimizations -ldl -ffunction-sections -fdata-sections -fstack-protector-strong -mthumb
SDK_LD_CFLAGS := -mcpu=cortex-a7 -mfloat-abi=hard -mfpu=neon-vfpv4 -fno-aggressive-loop-optimizations -Wl,-z,relro -Wl,-z,noexecstack -Wl,-z,now,-s -ldl -fPIC
else
$(error Unkown SoC!! Add $(CONFIG_XMEDIA_CHIP_TYPE) branch here for building!!)
endif

SDK_KER_CFLAGS := -D$(CHIPSET) -D$(CHIPARCH) -DRELEASE -D_USE_LLI_ -Wunused-but-set-variable -DUSER_BIT_32 -DKERNEL_BIT_32 -Wno-date-time -D_GNU_SOURCE --warn-stack-usage=500

ifeq ($(CONFIG_XMEDIA_SDK_MODE_LOG),y)
SDK_USR_CFLAGS += -DCONFIG_LOG_TRACE_SUPPORT=1
SDK_KER_CFLAGS += -DCONFIG_LOG_TRACE_SUPPORT=1
endif

SDK_USR_CFLAGS += -Wall -Werror
SDK_KER_CFLAGS += -Wall -Werror

CC := $(TOOLCHAIN)-gcc
AR := $(TOOLCHAIN)-ar
NM := $(TOOLCHAIN)-nm
STRIP := $(TOOLCHAIN)-strip

ascii_to_lower = $(shell echo $(1) | tr '[:upper:]' '[:lower:]')

AT :=

BUILD_DIR := $(SDK_DIR)/build
SOURCE_DIR := $(SDK_DIR)/source
SAMPLE_DIR := $(SDK_DIR)/sample
PROJECT_DIR := $(SDK_DIR)/project
OPEN_SOURCE_DIR := $(SDK_DIR)/open_source
SCRIPTS_DIR := $(SDK_DIR)/scripts
ROOTFS_DIR := $(SDK_DIR)/source/rootfs
INITRD_DIR := $(SDK_DIR)/source/initramdisk
BOOTLOADER_DIR := $(SDK_DIR)/source/bootloader
KERNEL_DIR := $(SDK_DIR)/source/kernel
ATF_DIR := $(SDK_DIR)/source/trusted-firmware
QS_MCU_DIR := $(SDK_DIR)/mcu
GMP_DIR := $(SDK_DIR)/source/gmp
COMPONENT_DIR := $(SDK_DIR)/source/component
OUT_DIR := $(SDK_DIR)/out/$(CHIPSET)
KERNEL_BUILDDIR := $(OUT_DIR)/$(CONFIG_XMEDIA_KERNEL_VERSION)
ATF_BUILDDIR := $(OUT_DIR)/trusted-firmware
QS_MCU_BUILDDIR := $(OUT_DIR)/qs_mcu
ROOTFS_BUILDDIR := $(OUT_DIR)/rootfs_builddir
INITRD_BUILDDIR := $(OUT_DIR)/initrd_builddir
SDK_STATIC_LIB_DIR := $(OUT_DIR)/lib/static
SDK_SHARE_LIB_DIR := $(OUT_DIR)/lib/share
SDK_KO_DIR := $(OUT_DIR)/ko
SDK_INCLUDE_DIR := $(OUT_DIR)/include
SDK_KO_BUILDER_DIR := $(OUT_DIR)/ko_builder
SDK_IMAGE_DIR := $(OUT_DIR)/image
TOOLS_DIR := $(SDK_DIR)/tools/linux/utils
TOOLS_BUILDDIR := $(OUT_DIR)/tools_builddir
KCONFIG_SRC_DIR := $(SDK_DIR)/tools/linux/kconfig
KCONFIG_CFG := version_config/main_config
BOOT_BUILDDIR := $(OUT_DIR)/boot_builddir
BOOTSTRAP_BUILDDIR := $(OUT_DIR)/bootstrap_builddir
XMEDIA_CONFIGS_DIR        := $(SDK_DIR)/configs
XMEDIA_INSTALL_DIR        := $(OUT_DIR)
XMEDIA_IMAGE_DIR          := $(XMEDIA_INSTALL_DIR)/image
XMEDIA_SPI_IMAGE_DIR      := $(XMEDIA_IMAGE_DIR)/spi_image
XMEDIA_NAND_IMAGE_DIR     := $(XMEDIA_IMAGE_DIR)/nand_image
XMEDIA_EMMC_IMAGE_DIR     := $(XMEDIA_IMAGE_DIR)/emmc_image
XMEDIA_SPI_EMMC_IMAGE_DIR := $(XMEDIA_IMAGE_DIR)/spi_emmc_image
XMEDIA_SPI_NAND_IMAGE_DIR := $(XMEDIA_IMAGE_DIR)/spi_nand_image
XMEDIA_MODULE_DIR         := $(XMEDIA_INSTALL_DIR)/kmod
XMEDIA_ROOTFS_DIR         := $(XMEDIA_INSTALL_DIR)/rootfs
XMEDIA_INITRD_DIR         := $(XMEDIA_INSTALL_DIR)/initramdisk

XMEDIA_LIB_DIR            := $(XMEDIA_INSTALL_DIR)/lib
XMEDIA_STATIC_LIB_DIR     := $(XMEDIA_LIB_DIR)/static
XMEDIA_SHARED_LIB_DIR     := $(XMEDIA_LIB_DIR)/share
XMEDIA_EXTERN_LIB_DIR     := $(XMEDIA_LIB_DIR)/extern

XMEDIA_PREBUILTS_DIR      := $(XMEDIA_CONFIGS_DIR)/$(CONFIG_XMEDIA_CHIP_TYPE)/prebuilts

KCONFIG_EXE := $(KCONFIG_SRC_DIR)/mconf

ifeq ($(CONFIG_XMEDIA_GMP_BUILDIN),y)
XMEDIA_DRV_BUILDTYPE := y
else
XMEDIA_DRV_BUILDTYPE := m
endif

MKBOOTARGS  := $(TOOLS_DIR)/bin/mkbootargs
MKCRAMFS    := $(TOOLS_DIR)/bin/mkfs.cramfs
MKEXT4FS    := $(TOOLS_DIR)/bin/make_ext4fs
MKSQUASHFS  := $(TOOLS_DIR)/bin/mksquashfs
MKJFFS2     := $(TOOLS_DIR)/bin/mkfs.jffs2
MKYAFFS     := $(TOOLS_DIR)/bin/mkyaffs2image
MKUBIIMG    := $(TOOLS_DIR)/bin/mkubiimg.sh
HWGZIP      := $(TOOLS_DIR)/bin/hwgzip


ifeq ($(CONFIG_XMEDIA_QUICK_START),y)
SPI_PARTITIONS      := $(XMEDIA_PREBUILTS_DIR)/spi_partitions_quickstart.xml
SPI_BOOTARGS        := $(XMEDIA_PREBUILTS_DIR)/spi_bootargs_quickstart.txt
NAND_PARTITIONS     := $(XMEDIA_PREBUILTS_DIR)/nand_partitions_quickstart.xml
NAND_BOOTARGS       := $(XMEDIA_PREBUILTS_DIR)/nand_bootargs_quickstart.txt
else
	ifeq ($(XMEDIA_DRV_BUILDTYPE),y)
	NAND_PARTITIONS     := $(XMEDIA_PREBUILTS_DIR)/nand_partitions_buildin.xml
	EMMC_PARTITIONS     := $(XMEDIA_PREBUILTS_DIR)/emmc_partitions_buildin.xml
	SPI_PARTITIONS      := $(XMEDIA_PREBUILTS_DIR)/spi_partitions_buildin.xml
	NAND_BOOTARGS       := $(XMEDIA_PREBUILTS_DIR)/nand_bootargs_buildin.txt
	EMMC_BOOTARGS       := $(XMEDIA_PREBUILTS_DIR)/emmc_bootargs_buildin.txt
	SPI_BOOTARGS        := $(XMEDIA_PREBUILTS_DIR)/spi_bootargs_buildin.txt
	else
	NAND_PARTITIONS     := $(XMEDIA_PREBUILTS_DIR)/nand_partitions.xml
	EMMC_PARTITIONS     := $(XMEDIA_PREBUILTS_DIR)/emmc_partitions.xml
	SPI_PARTITIONS      := $(XMEDIA_PREBUILTS_DIR)/spi_partitions.xml
	NAND_BOOTARGS       := $(XMEDIA_PREBUILTS_DIR)/nand_bootargs.txt
	EMMC_BOOTARGS       := $(XMEDIA_PREBUILTS_DIR)/emmc_bootargs.txt
	SPI_BOOTARGS        := $(XMEDIA_PREBUILTS_DIR)/spi_bootargs.txt
	endif
endif


ifeq ($(CONFIG_XMEDIA_KERNEL_SUPPORT_SMP),y)
OBJ_KERNEL_VERSION:=$(CONFIG_XMEDIA_KERNEL_VERSION)/multi
else
OBJ_KERNEL_VERSION:=$(CONFIG_XMEDIA_KERNEL_VERSION)/single
endif
OBJ_LOG_MODE:=$(CONFIG_XMEDIA_SDK_LOG_MODE)

ifeq ($(CONFIG_XMEDIA_GMP_BUILDIN),y)
    OBJ_BUILD_TYPE=static
else
    OBJ_BUILD_TYPE=dynamic
endif


SHELL=/bin/bash -o pipefail

# $(call mkdir,$1)
mkdir = if [ ! -d $(1) ]; then ( mkdir -p $(1); ) fi

build_target=$(1) $(addsuffix _build,$(1)) $(addsuffix .build,$(1))

clean_target=$(addsuffix _clean,$(1)) $(addsuffix .clean,$(1))

# $(call export_target,<taget1 target2>,<dir>)
define export_target
.PHONY: $(call build_target,$(1))
$(call build_target,$(1)):
	@$(MAKE) -C $(2)

.PHONY: $(call clean_target,$(1))
$(call clean_target,$(1)):
	@$(MAKE) -C $(2) clean

endef

