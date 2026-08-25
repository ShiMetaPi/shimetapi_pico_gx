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

# SDK 端 rootfs 覆盖源（etc/profile、etc/init.d/*、opt/ko/load）
# 路径：source/rootfs/scripts/etc/、source/rootfs/scripts/opt/ko/
ROOTFS_OVERLAY_DIR := $(ROOTFS_SRCDIR)/scripts

.PHONY: rootfs_post
rootfs_post:
	@# ---- SDK 源覆盖 tgz baked-in（必须在 tar xzf 之后、factory_test install 之前）----
	@mkdir -p $(ROOTFS_PREFIX)/etc/init.d $(ROOTFS_PREFIX)/opt/ko
	@cp -f $(ROOTFS_OVERLAY_DIR)/etc/profile         $(ROOTFS_PREFIX)/etc/profile
	@cp -f $(ROOTFS_OVERLAY_DIR)/etc/init.d/rcS      $(ROOTFS_PREFIX)/etc/init.d/rcS
	@cp -f $(ROOTFS_OVERLAY_DIR)/etc/init.d/S00devs  $(ROOTFS_PREFIX)/etc/init.d/S00devs
	@cp -f $(ROOTFS_OVERLAY_DIR)/etc/init.d/S01udev  $(ROOTFS_PREFIX)/etc/init.d/S01udev
	@cp -f $(ROOTFS_OVERLAY_DIR)/etc/init.d/S40network   $(ROOTFS_PREFIX)/etc/init.d/S40network
	@cp -f $(ROOTFS_OVERLAY_DIR)/etc/init.d/S50dropbear  $(ROOTFS_PREFIX)/etc/init.d/S50dropbear
	@cp -f $(ROOTFS_OVERLAY_DIR)/etc/init.d/S80network   $(ROOTFS_PREFIX)/etc/init.d/S80network
	@cp -f $(ROOTFS_OVERLAY_DIR)/etc/init.d/S99init      $(ROOTFS_PREFIX)/etc/init.d/S99init
	@cp -f $(ROOTFS_OVERLAY_DIR)/opt/ko/load             $(ROOTFS_PREFIX)/opt/ko/load
	@chmod 755 $(ROOTFS_PREFIX)/etc/init.d/* $(ROOTFS_PREFIX)/opt/ko/load
	@# 写 chip_type 让 S99init 派生加载参数（v11a/v12a/v11at 都能正确加载 ko）
	@echo '$(CONFIG_XMEDIA_CHIP_TYPE)' > $(ROOTFS_PREFIX)/etc/chip_type
	@chmod 644 $(ROOTFS_PREFIX)/etc/chip_type

	@rm -rf $(ROOTFS_PREFIX)/opt/sample
	@# 自带 shimetad_GK7206 到 /usr/bin（rootfs_post 每次 make rootfs 都会执行）
	@mkdir -p $(ROOTFS_PREFIX)/usr/bin
	@install -m 755 $(SDK_DIR)/shimetad_GK7206 $(ROOTFS_PREFIX)/usr/bin/
ifneq ($(filter y,$(CONFIG_XM7206V11A) $(CONFIG_XM7206V12A)),)
	@$(MAKE) -C $(SDK_DIR)/source/app/factory_test install
	@# ---- 根除 factory_server（避免它抢占 80 端口，留给 app_sample 用）----
	@rm -f $(ROOTFS_PREFIX)/usr/bin/factory_server
	@rm -f $(ROOTFS_PREFIX)/etc/init.d/S60factory_server
	@find $(ROOTFS_PREFIX) -type d \( -name factory -o -name factory_test -o -name factory_server \) -prune -exec rm -rf {} +
	@echo "[rootfs_post] factory_server purged"
endif
