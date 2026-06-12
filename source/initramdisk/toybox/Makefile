#!/usr/bin/env make
###############################################################################
#
#  Create By Lane 2024/1/29
#
###############################################################################

TOYBOX := toybox-0.8.8

TOYBOX_PREFIX = $(INITRD_BUILDDIR)/$(TOYBOX)/.install

MKFLAGS_TOYBOX = $(MKFLAGS)
MKFLAGS_TOYBOX += CROSS_COMPILE=$(CONFIG_XMEDIA_USR_ARM_TOOLCHAINS)-
MKFLAGS_TOYBOX += PREFIX=$(TOYBOX_PREFIX)
MKFLAGS_TOYBOX += CFLAGS="-mthumb -Os" LDFLAGS="--static" HOSTCC=gcc CC=gcc STRIP=strip

initrd_unstrip: $(INITRD_PREFIX)/usr/bin/toybox

clean: $(TOYBOX).clean

$(TOYBOX).clean:
	$(AT)rm -rf $(INITRD_BUILDDIR)/$(TOYBOX)

$(INITRD_PREFIX)/usr/bin/toybox: $(INITRD_BUILDDIR)/$(TOYBOX)/.built
	$(call mkdir,$(@D))
	cp -arf $(TOYBOX_PREFIX)/* $(INITRD_PREFIX)
	touch $@

$(INITRD_BUILDDIR)/$(TOYBOX)/.built: $(INITRD_BUILDDIR)/$(TOYBOX)/.extracted
	make -C $(<D) $(MKFLAGS_TOYBOX) toybox
	make -C $(<D) $(MKFLAGS_TOYBOX) install
	touch $@

$(INITRD_BUILDDIR)/$(TOYBOX)/.extracted:
	$(call mkdir,$(@D))
	tar -xf $(OPEN_SOURCE_DIR)/toybox/$(TOYBOX).tar.gz -C $(INITRD_BUILDDIR)
	cp -arf  $(INITRD_SRCDIR)/toybox/$(CONFIG_XMEDIA_TOYBOX_CFG_NAME).patch/*  $(@D)
	cp -f $(INITRD_SRCDIR)/toybox/$(CONFIG_XMEDIA_TOYBOX_CFG_NAME).config $(@D)/.config
	touch $@
