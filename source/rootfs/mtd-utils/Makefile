#!/usr/bin/env make
###################################################################################
#
#Copyright (c) Lotus. All rights reserved.
#
###################################################################################
MTDUTILS := mtd-utils-2.0.2

ZLIB     :=zlib-1.2.11

UUIDLIB  :=libuuid-1.0.3

MTDUTILS_PREFIX = $(ROOTFS_BUILDDIR)/$(MTDUTILS)/.install

ZLIB_PREFIX     = $(ROOTFS_BUILDDIR)/$(ZLIB)/.install

UUIDLIB_PREFIX  = $(ROOTFS_BUILDDIR)/$(UUIDLIB)/.install

###################################################################################
rootfs_unstrip: $(ROOTFS_PREFIX)/bin/mkfs.ubifs

clean: $(MTDUTILS).clean $(UUIDLIB).clean $(ZLIB).clean

$(MTDUTILS).clean:
	rm -rf $(ROOTFS_BUILDDIR)/$(MTDUTILS)

$(ROOTFS_PREFIX)/bin/mkfs.ubifs: $(ROOTFS_BUILDDIR)/$(MTDUTILS)/.built
	$(call mkdir,$(@D))
	cp -arf $(MTDUTILS_PREFIX)/sbin/* $(ROOTFS_PREFIX)/bin
	touch $@

$(ROOTFS_BUILDDIR)/$(MTDUTILS)/.built: $(ROOTFS_BUILDDIR)/$(MTDUTILS)/.extracted \
									   $(ROOTFS_BUILDDIR)/$(UUIDLIB)/.built \
									   $(ROOTFS_BUILDDIR)/$(ZLIB)/.built
	cd $(<D); CC=$(CROSS_COMPILE)-gcc ./configure \
		--prefix=$(MTDUTILS_PREFIX) \
		--host=$(CROSS_COMPILE) \
		CFLAGS='--static -I$(ZLIB_PREFIX)/include' \
		ZLIB_LIBS=$(ZLIB_PREFIX)/lib/libz.a \
		UUID_LIBS=$(UUIDLIB_PREFIX)/lib/libuuid.a \
		PKG_CONFIG_PATH=$(UUIDLIB_PREFIX)/lib/pkgconfig \
		--without-lzo \
		--disable-tests
	make -C $(<D)
	make -C $(<D) install
	touch $@

$(ROOTFS_BUILDDIR)/$(MTDUTILS)/.extracted:
	$(call mkdir,$(@D))
	tar xf $(OPEN_SOURCE_DIR)/mtd-utils/$(MTDUTILS).tar.bz2 -C $(ROOTFS_BUILDDIR)
	touch $@

###################################################################################
$(UUIDLIB).clean:
	rm -rf $(ROOTFS_BUILDDIR)/$(UUIDLIB)

$(ROOTFS_BUILDDIR)/$(UUIDLIB)/.built: $(ROOTFS_BUILDDIR)/$(UUIDLIB)/.extracted
	cd $(<D); CC=$(CROSS_COMPILE)-gcc ./configure \
		--prefix=$(UUIDLIB_PREFIX) \
		--host=$(CROSS_COMPILE)
	make -C $(<D)
	make -C $(<D) install
	touch $@

$(ROOTFS_BUILDDIR)/$(UUIDLIB)/.extracted:
	$(call mkdir,$(@D))
	tar xf $(OPEN_SOURCE_DIR)/libuuid/$(UUIDLIB).tar.gz -C $(ROOTFS_BUILDDIR)
	touch $@

###################################################################################
$(ZLIB).clean:
	rm -rf $(ROOTFS_BUILDDIR)/$(ZLIB)

$(ROOTFS_BUILDDIR)/$(ZLIB)/.built: $(ROOTFS_BUILDDIR)/$(ZLIB)/.extracted
	cd $(<D); CC=$(CROSS_COMPILE)-gcc ./configure \
		--prefix=$(ZLIB_PREFIX)
	make -C $(<D)
	make -C $(<D) install
	touch $@

$(ROOTFS_BUILDDIR)/$(ZLIB)/.extracted:
	$(call mkdir,$(@D))
	tar xf $(OPEN_SOURCE_DIR)/zlib/$(ZLIB).tar.gz -C $(ROOTFS_BUILDDIR)
	touch $@
