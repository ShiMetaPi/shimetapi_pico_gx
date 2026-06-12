SDK_DIR := $(shell cd $(shell pwd)/../.. && /bin/pwd)
include $(SDK_DIR)/build/base.mk

###############################################################################
#
#
#Copyright (c) XMEDIA. All rights reserved.
#
#
###############################################################################

CROSS_COMPILE   := $(CONFIG_XMEDIA_USR_ARM_TOOLCHAINS)
STRIP := $(CROSS_COMPILE)-strip
ROOTFS_PREFIX   := $(XMEDIA_ROOTFS_DIR)
ROOTFS_BUILDDIR ?= $(shell pwd)/.build
ROOTFS_SRCDIR   := $(if $(ROOTFS_DIR),$(ROOTFS_DIR),$(shell pwd))

MKFLAGS := PREFIX=$(ROOTFS_PREFIX) \
	CROSS_COMPILE=$(CROSS_COMPILE)- \
	CONFIG_LIBCPP_SUPPORT=$(CONFIG_XMEDIA_LIBCPP_SUPPORT) \
	CONFIG_ETH_SUPPORT=y

.PHONY: all clean

all: rootfs_strip_$(CONFIG_XMEDIA_ROOTFS_STRIP)
	@echo;
	@echo 'File system is ready at "$(ROOTFS_PREFIX)"'
	@echo;

clean:
	rm -rf $(ROOTFS_BUILDDIR)
	rm -rf $(ROOTFS_PREFIX)

.PHONY: rootfs_strip_y
rootfs_strip_y: rootfs_unstrip
	@set -e; for ix in $$(find $(ROOTFS_PREFIX)/ -type f -a ! -name "*.ko"); \
	    do ( \
	        FILEINFO=$$(file -b $${ix}); \
	        if [ "s$$(echo $${FILEINFO} | \
	              grep '^ELF ..-bit LSB ')" != "s" ]; \
	            then ( \
	                 $(STRIP) $${ix}; \
	        ) fi; \
	    ) done

.PHONY: rootfs_strip_ rootfs_strip_n rootfs_unstrip
rootfs_strip_ rootfs_strip_n: rootfs_unstrip

TOOLS.y = scripts busybox dropbear
TOOLS.$(CONFIG_XMEDIA_UDEV_SUPPORT)       += eudev
TOOLS.$(CONFIG_XMEDIA_E2FSPROGS_SUPPORT)  += e2fsprogs
TOOLS.$(CONFIG_XMEDIA_GDB_SUPPORT)        += gdb
TOOLS.$(CONFIG_XMEDIA_REG_SUPPORT)        += regtools
TOOLS.$(CONFIG_XMEDIA_MIITOOLS_SUPPORT)   += ethtools
TOOLS.$(CONFIG_XMEDIA_DOSFS_SUPPORT)      += dosfstools
TOOLS.$(CONFIG_XMEDIA_MTDUTILS_SUPPORT)   += mtd-utils
TOOLS.$(CONFIG_XMEDIA_CANTOOLS_SUPPORT)   += can-utils canutils

include $(foreach utils,$(TOOLS.y),$(ROOTFS_SRCDIR)/$(utils)/Makefile)

.PHONY: rootfs_post
rootfs_post:
	@rm -rf $(ROOTFS_PREFIX)/opt/sample
