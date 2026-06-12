#!/usr/bin/env make
###############################################################################
#
#  Copyright (c) XMEDIA. All rights reserved.
#
###############################################################################
ifeq ($(CFG_XMEDIA_EXPORT_FLAG),)
SDK_DIR := $(shell  cd $(CURDIR)/../../../../ && /bin/pwd)
endif

include $(SDK_DIR)/build/base.mk

USER := $(shell whoami)

CROSS_COMPILE:=$(TOOLCHAIN)

ALSALIB := alsa-lib-1.1.7

ALSALIB_BUILDDIR = $(SDK_DIR)/sample/uvc_app/audio/alsa

ALSALIB_INSTALLDIR = $(ALSALIB_BUILDDIR)/install

ALSALIB_PREFIX = $(ALSALIB_BUILDDIR)/opensource/$(ALSALIB)/.install

.PHONY: all clean

all:$(ALSALIB_BUILDDIR)/install/lib/libasound.a

clean: $(ALSALIB).clean

$(ALSALIB).clean:
	rm -rf $(ALSALIB_BUILDDIR)/opensource/$(ALSALIB)
	rm -rf $(ALSALIB_BUILDDIR)/install

$(ALSALIB_BUILDDIR)/install/lib/libasound.a: $(ALSALIB_BUILDDIR)/opensource/$(ALSALIB)/.built
	touch $@

$(ALSALIB_BUILDDIR)/opensource/$(ALSALIB)/.built: $(ALSALIB_BUILDDIR)/opensource/$(ALSALIB)/.extracted
	cd $(<D); CC=$(CROSS_COMPILE)-gcc STRIP=$(CROSS_COMPILE)-strip ./configure \
		--prefix=$(ALSALIB_INSTALLDIR) \
		--host=$(CROSS_COMPILE) \
		--enable-static=yes \
		--enable-shared=no \
		--disable-python \
		--with-configdir=/home/$(USER)/audio/alsa
	make -C $(<D)
	make -C $(<D) install
	touch $@

$(ALSALIB_BUILDDIR)/opensource/$(ALSALIB)/.extracted:
	tar -xjvf $(ALSALIB_BUILDDIR)/opensource/$(ALSALIB).tar.bz2 -C $(ALSALIB_BUILDDIR)/opensource
	touch $@
