#!/usr/bin/env make
###############################################################################
#
#Copyright (c) XMEDIA. All rights reserved.
#
#
###############################################################################
CAN_UTILS       := can-utils-v2023.03
CAN_UTILS_BUILDDIR := $(ROOTFS_BUILDDIR)/$(CAN_UTILS)
CAN_UTILS_PREFIX := $(ROOTFS_BUILDDIR)/$(CAN_UTILS)/.install
MKFLAG_CAN_UTILS := CFLAGS=-static LDFLAGS=-static

CC := $(CROSS_COMPILE)-gcc

rootfs_unstrip: $(ROOTFS_PREFIX)/bin/cangen

clean: $(CAN_UTILS).clean

$(CAN_UTILS).clean:
	rm -rf $(ROOTFS_BUILDDIR)/$(CAN_UTILS)

$(ROOTFS_PREFIX)/bin/cangen: $(CAN_UTILS_BUILDDIR)/.built
	$(call mkdir,$(@D))
	cp -arf $(CAN_UTILS_PREFIX)/bin/cangen $(ROOTFS_PREFIX)/bin
	cp -arf $(CAN_UTILS_PREFIX)/bin/candump $(ROOTFS_PREFIX)/bin
	touch $@

$(CAN_UTILS_BUILDDIR)/.built: $(CAN_UTILS_BUILDDIR)/.extracted
	make -C $(<D) PREFIX=$(CAN_UTILS_PREFIX) CC=$(CC) $(MKFLAG_CAN_UTILS)
	make -C $(<D) install PREFIX=$(CAN_UTILS_PREFIX) CC=$(CC) $(MKFLAG_CAN_UTILS)
	touch $@

$(CAN_UTILS_BUILDDIR)/.extracted:
	$(call mkdir,$(@D))
	unzip -q $(OPEN_SOURCE_DIR)/can/$(CAN_UTILS).zip -d $(ROOTFS_BUILDDIR)/
	touch $@