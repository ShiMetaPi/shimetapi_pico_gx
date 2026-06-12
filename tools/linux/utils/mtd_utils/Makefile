###############################################################################
#!/usr/bin/env make
###############################################################################
#
#  Copyright (c) XMEDIA.
#  All rights reserved.
#  Create By Caizhiyong 2021/3/16
#
###############################################################################
MTDUTILS := mtd-utils-2.0.2

MTDUTILS_PREFIX = $(TOOLS_BUILDDIR)/$(MTDUTILS)/.install

all: $(TOOLS_PREFIX)/mkfs.ubifs

clean: $(MTDUTILS).clean

$(MTDUTILS).clean:
	rm -rf $(TOOLS_BUILDDIR)/$(MTDUTILS)

$(TOOLS_PREFIX)/mkfs.ubifs: $(TOOLS_BUILDDIR)/$(MTDUTILS)/.built
	$(call mkdir,$(@D))
	cp -arf $(MTDUTILS_PREFIX)/sbin/mkfs.jffs2 $(TOOLS_PREFIX)
	cp -arf $(MTDUTILS_PREFIX)/sbin/ubinize $(TOOLS_PREFIX)
	cp -arf $(MTDUTILS_PREFIX)/sbin/mkfs.ubifs $(TOOLS_PREFIX)
	cp -arf $(MTDUTILS_PREFIX)/sbin/ubinize $(TOOLS_PREFIX)
	touch $@

$(TOOLS_BUILDDIR)/$(MTDUTILS)/.built: $(TOOLS_BUILDDIR)/$(MTDUTILS)/.extracted \
									$(TOOLS_BUILDDIR)/$(ZLIB)/.built
	cd $(<D); ./configure \
			--prefix=$(MTDUTILS_PREFIX) CC=gcc \
			CFLAGS="-static" \
			LDFLAGS="-static" \
			ZLIBCPPFLAGS="-I$(ZLIB_PREFIX)/include -static" \
			ZLIBLDFLAGS="-L$(ZLIB_PREFIX)/lib -static"
	make -C $(<D)
	make -C $(<D) install
	touch $@

$(TOOLS_BUILDDIR)/$(MTDUTILS)/.extracted:
	$(call mkdir,$(@D))
	tar xf $(OPEN_SOURCE_DIR)/mtd-utils/$(MTDUTILS).tar.bz2 -C $(TOOLS_BUILDDIR)
	touch $@