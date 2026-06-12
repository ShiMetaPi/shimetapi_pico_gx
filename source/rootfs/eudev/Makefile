#!/usr/bin/env make
###############################################################################
#
#  Copyright (c) XMEDIA. All rights reserved.
#
###############################################################################

EUDEV := eudev-3.2.14

EUDEV_PREFIX = $(ROOTFS_BUILDDIR)/$(EUDEV)/.install

rootfs_unstrip: $(ROOTFS_PREFIX)/sbin/udevd

clean: $(EUDEV).clean

$(EUDEV).clean:
	rm -rf $(ROOTFS_BUILDDIR)/$(EUDEV)

$(ROOTFS_PREFIX)/sbin/udevd: $(ROOTFS_BUILDDIR)/$(EUDEV)/.built
	$(call mkdir,$(@D))
	cp -af $(ROOTFS_BUILDDIR)/$(EUDEV)/src/udev/udevd $(ROOTFS_PREFIX)/sbin
	cp -af $(ROOTFS_BUILDDIR)/$(EUDEV)/src/udev/udevadm $(ROOTFS_PREFIX)/bin
	tar -xf $(ROOTFS_SRCDIR)/eudev/rootfs.tar.bz2 -C $(ROOTFS_PREFIX)/..
	touch $@

$(ROOTFS_BUILDDIR)/$(EUDEV)/.built: $(ROOTFS_BUILDDIR)/$(EUDEV)/.extracted
	cd $(<D); CC=$(CROSS_COMPILE)-gcc CFLAGS="$(filter-out -Werror,$(SDK_USR_CFLAGS))" ./configure \
		--prefix=/ \
		--host=$(CROSS_COMPILE)
	make -C $(<D) USE_STATIC=false
	touch $@

#cp -arf $(ROOTFS_SRCDIR)/eudev/$(EUDEV).patch/. $(@D)
$(ROOTFS_BUILDDIR)/$(EUDEV)/.extracted:
	$(call mkdir,$(@D))
	tar xf $(OPEN_SOURCE_DIR)/eudev/$(EUDEV).tar.gz -C $(ROOTFS_BUILDDIR)
	touch $@
