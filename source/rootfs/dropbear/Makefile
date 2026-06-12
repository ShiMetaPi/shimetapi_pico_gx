#!/usr/bin/env make

DROPBEAR := dropbear-2024.86

DROPBEAR_BUILDDIR := $(ROOTFS_BUILDDIR)/$(DROPBEAR)

DROPBEAR_CONFIGURE_FLAGS := \
	--host=$(CONFIG_XMEDIA_USR_ARM_TOOLCHAINS) \
	--prefix=/usr \
	--sbindir=/usr/sbin \
	--disable-zlib \
	--disable-lastlog \
	--disable-utmp \
	--disable-utmpx \
	--disable-wtmp \
	--disable-wtmpx

rootfs_unstrip: \
	$(ROOTFS_PREFIX)/usr/sbin/dropbear \
	$(ROOTFS_PREFIX)/usr/bin/dropbearkey \
	$(ROOTFS_PREFIX)/usr/bin/dbclient \
	$(ROOTFS_PREFIX)/usr/bin/scp

clean: $(DROPBEAR).clean

$(DROPBEAR).clean:
	$(AT)rm -rf $(DROPBEAR_BUILDDIR)

$(ROOTFS_PREFIX)/usr/sbin/dropbear: $(DROPBEAR_BUILDDIR)/.built
	$(call mkdir,$(@D))
	cp -af $(DROPBEAR_BUILDDIR)/dropbear $@
	chmod 755 $@
	$(call mkdir,$(ROOTFS_PREFIX)/etc/dropbear)
	touch $@
$(ROOTFS_PREFIX)/usr/bin/dropbearkey: $(DROPBEAR_BUILDDIR)/.built
	$(call mkdir,$(@D))
	cp -af $(DROPBEAR_BUILDDIR)/dropbearkey $@
	chmod 755 $@
$(ROOTFS_PREFIX)/usr/bin/dbclient: $(DROPBEAR_BUILDDIR)/.built
	$(call mkdir,$(@D))
	cp -af $(DROPBEAR_BUILDDIR)/dbclient $@
	chmod 755 $@
$(ROOTFS_PREFIX)/usr/bin/scp: $(DROPBEAR_BUILDDIR)/.built
	$(call mkdir,$(@D))
	cp -af $(DROPBEAR_BUILDDIR)/scp $@
	chmod 755 $@

$(DROPBEAR_BUILDDIR)/.built: $(DROPBEAR_BUILDDIR)/.configured
	make -C $(DROPBEAR_BUILDDIR) PROGRAMS="dropbear dropbearkey dbclient scp"
	touch $@

$(DROPBEAR_BUILDDIR)/.configured: $(DROPBEAR_BUILDDIR)/.extracted
	cd $(DROPBEAR_BUILDDIR) && \
		CC=$(CONFIG_XMEDIA_USR_ARM_TOOLCHAINS)-gcc \
		STRIP=$(CONFIG_XMEDIA_USR_ARM_TOOLCHAINS)-strip \
		./configure $(DROPBEAR_CONFIGURE_FLAGS)
	touch $@

$(DROPBEAR_BUILDDIR)/.extracted:
	$(call mkdir,$(ROOTFS_BUILDDIR))
	tar -xf $(OPEN_SOURCE_DIR)/dropbear/$(DROPBEAR).tar.bz2 -C $(ROOTFS_BUILDDIR)
	touch $@
