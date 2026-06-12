#!/usr/bin/env make
###############################################################################
#
#
#Copyright (c) XMEDIA. All rights reserved.
#
#
###############################################################################

BUSYBOX := busybox-1.36.1

BUSYBOX_PREFIX = $(ROOTFS_BUILDDIR)/$(BUSYBOX)/.install

MKFLAGS_BUSYBOX = $(MKFLAGS) V=1
MKFLAGS_BUSYBOX += CROSS_COMPILE=$(CONFIG_XMEDIA_USR_ARM_TOOLCHAINS)-
MKFLAGS_BUSYBOX += CONFIG_PREFIX=$(BUSYBOX_PREFIX)
MKFLAGS_BUSYBOX += CFLAGS="$(filter-out -Werror,$(SDK_USR_CFLAGS)) $(if $(CONFIG_XMEDIA_ARM-GCC7.3-LINUX-MUSLEABI),-Ouserspace,)"

rootfs_unstrip: $(ROOTFS_PREFIX)/bin/busybox

clean: $(BUSYBOX).clean

$(BUSYBOX).clean:
	$(AT)rm -rf $(ROOTFS_BUILDDIR)/$(BUSYBOX)

$(ROOTFS_PREFIX)/bin/busybox: $(ROOTFS_BUILDDIR)/$(BUSYBOX)/.built
	$(call mkdir,$(@D))
	cp -arf $(BUSYBOX_PREFIX)/. $(ROOTFS_PREFIX)
	touch $@

$(ROOTFS_BUILDDIR)/$(BUSYBOX)/.built: $(ROOTFS_BUILDDIR)/$(BUSYBOX)/.extracted
	make -C $(<D) $(MKFLAGS_BUSYBOX) all
	make -C $(<D) $(MKFLAGS_BUSYBOX) install
	touch $@

$(ROOTFS_BUILDDIR)/$(BUSYBOX)/.extracted:
	$(call mkdir,$(@D))
	tar -xf $(OPEN_SOURCE_DIR)/busybox/$(BUSYBOX).tar.bz2 -C $(ROOTFS_BUILDDIR)
	cp -f $(ROOTFS_SRCDIR)/busybox/$(CONFIG_XMEDIA_BUSYBOX_CFG_NAME).config $(@D)/.config
	touch $@
