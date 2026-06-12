###############################################################################
#!/usr/bin/env make
###############################################################################
#
#  Copyright (c) XMEDIA.
#  All rights reserved.
#  Create By Caizhiyong 2021/3/16
#
###############################################################################

CRAMFS := util-linux-2.31

ZLIB     :=zlib-1.2.11

CRAMFS_PREFIX = $(TOOLS_BUILDDIR)/$(CRAMFS)/.install

ZLIB_PREFIX     = $(TOOLS_BUILDDIR)/$(ZLIB)/.install

all: $(TOOLS_PREFIX)/mkfs.cramfs

clean: $(CRAMFS).clean $(ZLIB).clean

$(CRAMFS).clean:
	rm -rf $(TOOLS_BUILDDIR)/$(CRAMFS)

$(TOOLS_PREFIX)/mkfs.cramfs: $(TOOLS_BUILDDIR)/$(CRAMFS)/.built
	$(call mkdir,$(@D))
	cp -arf $(CRAMFS_PREFIX)/sbin/mkfs.cramfs $(TOOLS_PREFIX)
	touch $@

$(TOOLS_BUILDDIR)/$(CRAMFS)/.built: $(TOOLS_BUILDDIR)/$(CRAMFS)/.extracted \
									$(TOOLS_BUILDDIR)/$(ZLIB)/.built
	cd $(<D); ./configure \
		--prefix=$(CRAMFS_PREFIX) ZLIB_LIBS=$(ZLIB_PREFIX)/lib/libz.a \
		CFLAGS="-static " LDFLAGS="-static" \
		--enable-static \
		--disable-libuuid --disable-libblkid --disable-libmount \
		--disable-mount --disable-losetup --disable-fsck \
		--disable-partx --disable-uuidd --disable-mountpoint \
		--disable-fallocate --disable-unshare --disable-eject \
		--disable-agetty --disable-switch_root --disable-pivot_root \
		--disable-kill --disable-utmpdump --disable-rename \
		--disable-login --disable-sulogin --disable-su \
		--disable-schedutils --disable-wall \
		--disable-bash-completion; \
	make CC=gcc LDFLAGS="-static"
	make -C $(<D) install
	touch $@

$(TOOLS_BUILDDIR)/$(CRAMFS)/.extracted:
	$(call mkdir,$(@D))
	tar xf $(OPEN_SOURCE_DIR)/util-linux/$(CRAMFS).tar.gz -C $(TOOLS_BUILDDIR)
	touch $@

###################################################################################
$(ZLIB).clean:
	rm -rf $(TOOLS_BUILDDIR)/$(ZLIB)

$(TOOLS_BUILDDIR)/$(ZLIB)/.built: $(TOOLS_BUILDDIR)/$(ZLIB)/.extracted
	cd $(<D); ./configure \
		--prefix=$(ZLIB_PREFIX) --static;\
	make CC=gcc LDFLAGS="-static"
	make -C $(<D) install
	touch $@

$(TOOLS_BUILDDIR)/$(ZLIB)/.extracted:
	$(call mkdir,$(@D))
	tar xf $(OPEN_SOURCE_DIR)/zlib/$(ZLIB).tar.gz -C $(TOOLS_BUILDDIR)
	touch $@