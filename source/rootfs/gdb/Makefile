#!/usr/bin/env make
###################################################################################
#
#
#Copyright (c) XMEDIA. All rights reserved.
#
#
###################################################################################
GDB             := gdb-13.2

NCURSES         := ncurses-6.4

GMP             := gmp-6.3.0

GDB_PREFIX      := $(ROOTFS_BUILDDIR)/$(GDB)/.install

NCURSES_PREFIX  := $(ROOTFS_BUILDDIR)/$(NCURSES)/.install

GMP_PREFIX  := $(ROOTFS_BUILDDIR)/$(GMP)/.install


###################################################################################
rootfs_unstrip: $(ROOTFS_PREFIX)/bin/gdb

clean: $(GDB).clean $(NCURSES).clean $(GMP).clean

$(GDB).clean:
	rm -rf $(ROOTFS_BUILDDIR)/$(GDB)

$(ROOTFS_PREFIX)/bin/gdb: $(ROOTFS_BUILDDIR)/$(GDB)/.built
	$(call mkdir,$(@D))
	cp -arf $(GDB_PREFIX)/bin/gdb $(ROOTFS_PREFIX)/bin
	cp -arf $(GDB_PREFIX)/bin/gdbserver $(ROOTFS_PREFIX)/bin
	cp -arf $(GMP_PREFIX)/lib/libgmp.so* $(ROOTFS_PREFIX)/usr/lib/
	touch $@

$(ROOTFS_BUILDDIR)/$(GDB)/.built: $(ROOTFS_BUILDDIR)/$(GDB)/.extracted \
    $(ROOTFS_BUILDDIR)/$(NCURSES)/.built  $(ROOTFS_BUILDDIR)/$(GMP)/.built
	cd $(<D); CC=$(CROSS_COMPILE)-gcc ./configure \
		--prefix=$(GDB_PREFIX) \
		--host=$(CROSS_COMPILE) \
		CFLAGS='-lm'\
		--disable-tui\
		--with-gmp=$(GMP_PREFIX) \
		LDFLAGS="-static-libstdc++ -static-libgcc -L$(NCURSES_PREFIX)/lib" \
		gdb_cv_prfpregset_t_broken=no \
		host_configargs="--disable-tui CFLAGS='-lm' --with-libgmp-prefix=$(GMP_PREFIX)"
	make -C $(<D)
	make -C $(<D) install
	touch $@

$(ROOTFS_BUILDDIR)/$(GDB)/.extracted:
	$(call mkdir,$(@D))
	tar xf $(OPEN_SOURCE_DIR)/gdb/$(GDB).tar.xz -C $(ROOTFS_BUILDDIR)
	touch $@

#cp -arf $(ROOTFS_SRCDIR)/gdb/$(GDB).patch/* $(@D)
###################################################################################
$(NCURSES).clean:
	rm -rf $(ROOTFS_BUILDDIR)/$(NCURSES)

$(ROOTFS_BUILDDIR)/$(NCURSES)/.built: $(ROOTFS_BUILDDIR)/$(NCURSES)/.extracted
	cd $(<D); CC=$(CROSS_COMPILE)-gcc ./configure \
		CPPFLAGS="-P" \
		--prefix=$(NCURSES_PREFIX) \
		--host=$(CROSS_COMPILE) --disable-stripping --disable-db-install
	make -C $(<D)
	make -C $(<D) install
	touch $@

$(ROOTFS_BUILDDIR)/$(NCURSES)/.extracted:
	$(call mkdir,$(@D))
	tar xf $(OPEN_SOURCE_DIR)/ncurses/$(NCURSES).tar.gz -C $(ROOTFS_BUILDDIR)
	touch $@
###################################################################################
$(GMP).clean:
	rm -rf $(ROOTFS_BUILDDIR)/$(GMP)

$(ROOTFS_BUILDDIR)/$(GMP)/.built: $(ROOTFS_BUILDDIR)/$(GMP)/.extracted
	cd $(<D); CC=$(CROSS_COMPILE)-gcc ./configure \
		--prefix=$(GMP_PREFIX) \
		--host=$(CROSS_COMPILE)
	make -C $(<D)
	make -C $(<D) install
	touch $@

$(ROOTFS_BUILDDIR)/$(GMP)/.extracted:
	$(call mkdir,$(@D))
	tar xf $(OPEN_SOURCE_DIR)/gdb/$(GMP).tar.xz -C $(ROOTFS_BUILDDIR)
	touch $@
