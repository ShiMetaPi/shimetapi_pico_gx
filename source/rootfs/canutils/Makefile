#!/usr/bin/env make
###############################################################################
#
#Copyright (c) XMEDIA. All rights reserved.
#
#
###############################################################################
CANTOOL          := canutils
CANUTILS         := canutils-4.0.6
CANLIBSOCKET     := libsocketcan-0.0.12
CANTOOL_BUILDDIR := $(ROOTFS_BUILDDIR)/$(CANUTILS)
LIBSOCKET_PREFIX := $(CANTOOL_BUILDDIR)/$(CANLIBSOCKET)/.install
LIBSOCKET_INC_OUT:= $(CANTOOL_BUILDDIR)/$(CANLIBSOCKET)/.install/include
LIBSOCKET_LIB_OUT:= $(CANTOOL_BUILDDIR)/$(CANLIBSOCKET)/.install/lib
CANTOOL_PREFIX   := $(CANTOOL_BUILDDIR)/.install

rootfs_unstrip: $(ROOTFS_PREFIX)/bin/canconfig

clean: $(CANTOOL).clean

$(CANTOOL).clean:
	rm -rf $(ROOTFS_BUILDDIR)/$(CANTOOL)

$(ROOTFS_PREFIX)/bin/canconfig: $(CANTOOL_BUILDDIR)/.built
	$(call mkdir,$(@D))
	cp -arf $(LIBSOCKET_LIB_OUT)/libsocketcan.so.2.3.1 $(ROOTFS_PREFIX)/lib
	cp -arf $(LIBSOCKET_LIB_OUT)/libsocketcan.so $(ROOTFS_PREFIX)/lib
	cp -arf $(LIBSOCKET_LIB_OUT)/libsocketcan.so.2 $(ROOTFS_PREFIX)/lib
	cp -arf $(CANTOOL_PREFIX)/sbin/canconfig $(ROOTFS_PREFIX)/bin
	touch $@

$(CANTOOL_BUILDDIR)/.built: $(CANTOOL_BUILDDIR)/.extracted
	$(call mkdir,$(@D))
	make -C $(CANTOOL_BUILDDIR)/$(CANLIBSOCKET)
	make install -C $(CANTOOL_BUILDDIR)/$(CANLIBSOCKET)
	cd $(CANTOOL_BUILDDIR)/$(CANUTILS); sh autogen.sh;\
	./configure --prefix=$(CANTOOL_PREFIX) --host=$(CROSS_COMPILE) LDFLAGS=-L$(LIBSOCKET_LIB_OUT) \
	CFLAGS=-I$(LIBSOCKET_INC_OUT) libsocketcan_LIBS=-lsocketcan  libsocketcan_CFLAGS=-I$(LIBSOCKET_INC_OUT)
	make -C $(CANTOOL_BUILDDIR)/$(CANUTILS)
	make install -C $(CANTOOL_BUILDDIR)/$(CANUTILS)
	touch $@

$(CANTOOL_BUILDDIR)/.extracted:
	$(call mkdir,$(@D))
	tar -xf $(OPEN_SOURCE_DIR)/can/$(CANLIBSOCKET).tar.bz2 -C $(CANTOOL_BUILDDIR)
	cp -arf $(ROOTFS_SRCDIR)/$(CANTOOL)/$(CANLIBSOCKET).patch/. $(@D)/$(CANLIBSOCKET)
	cd $(CANTOOL_BUILDDIR)/$(CANLIBSOCKET); sh autogen.sh;\
	./configure --prefix=$(LIBSOCKET_PREFIX) --host=$(CROSS_COMPILE)
	tar -xf $(OPEN_SOURCE_DIR)/can/$(CANUTILS).tar.bz2 -C $(CANTOOL_BUILDDIR);
	cp -arf $(ROOTFS_SRCDIR)/$(CANTOOL)/$(CANUTILS).patch/. $(@D)/$(CANUTILS)
	touch $@