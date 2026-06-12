#!/usr/bin/env make
###############################################################################
#
#
#Copyright (c) Lotus. All rights reserved.
#
#
###############################################################################

ETHTOOL := ethtool-6

ETHTOOL_PREFIX = $(ROOTFS_BUILDDIR)/$(ETHTOOL)/.install

rootfs_unstrip: $(ROOTFS_PREFIX)/sbin/ethtool

clean: $(ETHTOOL).clean

$(ETHTOOL).clean:
	rm -rf $(ROOTFS_BUILDDIR)/$(ETHTOOL)

$(ROOTFS_PREFIX)/sbin/ethtool: $(ROOTFS_BUILDDIR)/$(ETHTOOL)/.built
	$(call mkdir,$(@D))
	cp -arf $(ETHTOOL_PREFIX)/sbin/ethtool $(ROOTFS_PREFIX)/sbin
	touch $@

$(ROOTFS_BUILDDIR)/$(ETHTOOL)/.built: $(ROOTFS_BUILDDIR)/$(ETHTOOL)/.extracted
	cd $(<D); CC=$(CROSS_COMPILE)-gcc ./configure \
		--prefix=$(ETHTOOL_PREFIX) --host=$(CROSS_COMPILE) \
		CFLAGS="$(filter-out -Werror,$(SDK_USR_CFLAGS))" \
		--enable-static --disable-shared
	make -C $(<D)
	make -C $(<D) install
	touch $@

$(ROOTFS_BUILDDIR)/$(ETHTOOL)/.extracted:
	$(call mkdir,$(@D))
	tar xf $(OPEN_SOURCE_DIR)/ethtool/$(ETHTOOL).tar.gz -C $(ROOTFS_BUILDDIR)
	cp -arf $(ROOTFS_SRCDIR)/ethtools/$(ETHTOOL).patch/* $(@D)
	touch $@
