#!/usr/bin/env make
###############################################################################
#
#
#Copyright (c) XMEDIA. All rights reserved.
#
#
###############################################################################
DOSFS       := dosfstools-4.2

DOSFS_PREFIX = $(ROOTFS_BUILDDIR)/$(DOSFS)/.install

CC := $(CROSS_COMPILE)-gcc

rootfs_unstrip: $(ROOTFS_PREFIX)/sbin/fsck.fat

clean: $(DOSFS).clean

$(DOSFS).clean:
	rm -rf $(ROOTFS_BUILDDIR)/$(DOSFS)

$(ROOTFS_PREFIX)/sbin/fsck.fat: $(ROOTFS_BUILDDIR)/$(DOSFS)/.built
	$(call mkdir,$(@D))
	cp -arf $(DOSFS_PREFIX)/sbin/dosfsck $(ROOTFS_PREFIX)/sbin
	cp -arf $(DOSFS_PREFIX)/sbin/fsck.fat $(ROOTFS_PREFIX)/sbin
	cp -arf $(DOSFS_PREFIX)/sbin/fsck.vfat $(ROOTFS_PREFIX)/sbin
	cp -arf $(DOSFS_PREFIX)/sbin/mkfs.fat $(ROOTFS_PREFIX)/sbin
	cp -arf $(DOSFS_PREFIX)/sbin/mkfs.vfat $(ROOTFS_PREFIX)/sbin
	touch $@

$(ROOTFS_BUILDDIR)/$(DOSFS)/.built: $(ROOTFS_BUILDDIR)/$(DOSFS)/.extracted
	cd $(<D); \
	CC=$(CC) ./configure \
	   --prefix=$(DOSFS_PREFIX) \
	   --host=$(CROSS_COMPILE) \
	   --enable-compat-symlinks \
	   --without-iconv
	make -C $(<D)
	make -C $(<D) install
	touch $@

$(ROOTFS_BUILDDIR)/$(DOSFS)/.extracted:
	$(call mkdir,$(@D))
	tar xf $(OPEN_SOURCE_DIR)/dosfstools/$(DOSFS).tar.gz -C $(ROOTFS_BUILDDIR)
	touch $@

