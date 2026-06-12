#!/usr/bin/env make
###############################################################################
#
#
#Copyright (c) XMEDIA. All rights reserved.
#
#
###############################################################################

E2FSPROGS := e2fsprogs-1.47.0

E2FSPROGS_PREFIX = $(ROOTFS_BUILDDIR)/$(E2FSPROGS)/.install

rootfs_unstrip: $(ROOTFS_PREFIX)/sbin/mke2fs

clean: $(E2FSPROGS).clean

$(E2FSPROGS).clean:
	rm -rf $(ROOTFS_BUILDDIR)/$(E2FSPROGS)

$(ROOTFS_PREFIX)/sbin/mke2fs: $(ROOTFS_BUILDDIR)/$(E2FSPROGS)/.built
	$(call mkdir,$(@D))
	for x in mkfs.ext4 mke2fs fsck.ext4 e2fsck; do ( \
		test -L $(ROOTFS_PREFIX)/sbin/$$x && unlink $(ROOTFS_PREFIX)/sbin/$$x; \
		cp -af $(E2FSPROGS_PREFIX)/sbin/$$x $(ROOTFS_PREFIX)/sbin; \
	) done
	touch $@

$(ROOTFS_BUILDDIR)/$(E2FSPROGS)/.built: $(ROOTFS_BUILDDIR)/$(E2FSPROGS)/.extracted
	cd $(<D); CC=$(CROSS_COMPILE)-gcc ./configure \
		--prefix=$(E2FSPROGS_PREFIX) \
		--with-udev-rules-dir=$(E2FSPROGS_PREFIX)/etc/udev/rules.d \
		--with-crond-dir=$(E2FSPROGS_PREFIX)/etc/cron.d \
		--with-systemd-unit-dir=$(E2FSPROGS_PREFIX)/lib/systemd/system \
		--host=$(CROSS_COMPILE) \
		--disable-tls \
		--disable-defrag
	make -C $(<D)
	make -C $(<D) install
	touch $@

$(ROOTFS_BUILDDIR)/$(E2FSPROGS)/.extracted:
	$(call mkdir,$(@D))
	tar xf $(OPEN_SOURCE_DIR)/e2fsprogs/$(E2FSPROGS).tar.gz -C $(ROOTFS_BUILDDIR)
	touch $@

