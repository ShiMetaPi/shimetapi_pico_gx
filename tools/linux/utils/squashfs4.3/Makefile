###############################################################################
#!/usr/bin/env make
###############################################################################
#
#  Copyright (c) XMEDIA.
#  All rights reserved.
#  Create By Caizhiyong 2021/3/16
#
###############################################################################

SQUASHFS := squashfs4.3

XZLIB    := xz-5.2.2

SQUASHFS_PREFIX = $(TOOLS_BUILDDIR)/$(SQUASHFS)/.install

XZLIB_PREFIX    = $(TOOLS_BUILDDIR)/$(XZLIB)/.install

all: $(TOOLS_PREFIX)/mksquashfs

clean: $(SQUASHFS).clean $(XZLIB).clean

$(SQUASHFS).clean:
	rm -rf $(TOOLS_BUILDDIR)/$(SQUASHFS)

$(TOOLS_PREFIX)/mksquashfs: $(TOOLS_BUILDDIR)/$(SQUASHFS)/.built
	$(call mkdir,$(@D))
	cp -arf $(SQUASHFS_PREFIX)/bin/mksquashfs $(TOOLS_PREFIX)
	touch $@

$(TOOLS_BUILDDIR)/$(SQUASHFS)/.built: $(TOOLS_BUILDDIR)/$(SQUASHFS)/.extracted \
									$(TOOLS_BUILDDIR)/$(ZLIB)/.built \
									$(TOOLS_BUILDDIR)/$(XZLIB)/.built
	cd $(<D)/squashfs-tools; \
	make EXTRA_CFLAGS="-I$(ZLIB_PREFIX)/include -I$(XZLIB_PREFIX)/include" \
		 LDFLAGS="-L$(ZLIB_PREFIX)/lib  -L$(XZLIB_PREFIX)/lib -static";\
	make INSTALL_DIR=$(SQUASHFS_PREFIX)/bin install
	touch $@

$(TOOLS_BUILDDIR)/$(SQUASHFS)/.extracted:
	$(call mkdir,$(@D))
	tar xf $(OPEN_SOURCE_DIR)/squashfs/$(SQUASHFS).tar.gz -C $(TOOLS_BUILDDIR)
	cp -arf $(TOOLS_SRCDIR)/squashfs4.3/$(SQUASHFS).patch/. $(@D)
	touch $@

###################################################################################
$(XZLIB).clean:
	rm -rf $(TOOLS_BUILDDIR)/$(XZLIB)

$(TOOLS_BUILDDIR)/$(XZLIB)/.built: $(TOOLS_BUILDDIR)/$(XZLIB)/.extracted
	cd $(<D); ./configure --prefix=$(XZLIB_PREFIX); \
	make CC=gcc LDFLAGS="-static"
	make -C $(<D) install
	touch $@

$(TOOLS_BUILDDIR)/$(XZLIB)/.extracted:
	$(call mkdir,$(@D))
	tar xf $(OPEN_SOURCE_DIR)/xz/$(XZLIB).tar.bz2 -C $(TOOLS_BUILDDIR)
	touch $@