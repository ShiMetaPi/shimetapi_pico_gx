#!/usr/bin/env make
###############################################################################
#
#
#Copyright (c) XMEDIA. All rights reserved.
#
#
###############################################################################

# $(call _rt_file,<libc>)
_rt_file = $(shell $(CONFIG_XMEDIA_USR_ARM_TOOLCHAINS)-gcc \
    $(filter-out -Werror,$(SDK_USR_CFLAGS)) -print-file-name=$(1))

# $(call rt_file,<libc1, libc2, ...>)
rt_file = $(foreach lib,$(1),$(shell if [ -f "$(call _rt_file,$(lib))" ]; \
    then ( echo "$(call _rt_file,$(lib))"; ) else ( echo ""; ) fi ))

# $(call rt_notdir,<libc1, libc2, ...>)
rt_notdir = $(if $(strip $(call rt_file,$(1))),\
    $(notdir $(call rt_file,$(1))),\
    $(info *** $(CONFIG_XMEDIA_USR_ARM_TOOLCHAINS)-gcc $(SDK_USR_CFLAGS)) \
    $(error not found $(1), please check compiler and cflags))

ifeq ($(filter clean,$(MAKECMDGOALS)),clean)
RT-LIBS :=
else
# 兼容 uclibc(libc.so.0), glibc(libc.so.6), musllibc(libc.so)
MUSL_LIBC=$(findstring musl, $(shell $(CONFIG_XMEDIA_USR_ARM_TOOLCHAINS)-gcc --version))
RT-LIBS = $(if $(MUSL_LIBC),$(call rt_notdir,libc.so),\
	$(call rt_notdir,libc.so.0 libc.so.6))
RT-LIBS += $(call rt_notdir,libgcc_s.so)
ifeq ($(CONFIG_XMEDIA_LIBCPP_SUPPORT),y)
RT-LIBS += $(call rt_notdir,libstdc++.so)
endif
endif

rootfs_unstrip: $(ROOTFS_PREFIX)/etc/inittab \
    $(addprefix $(ROOTFS_PREFIX)/lib/,$(RT-LIBS))

$(ROOTFS_PREFIX)/etc/inittab:
	$(call mkdir,$(@D))
	tar xzf $(ROOTFS_SRCDIR)/scripts/rootfs.tgz -C $(ROOTFS_PREFIX)/..
	@echo "[scripts] rootfs.tgz 解压完成, 重新覆盖 busybox(WGET)..."
	@# rootfs.tgz 是 baked-in 骨架, 里面的 bin/busybox 是老版本无 wget 的 busybox。
	@# 这里把 busybox-1.36.1 build 出来的新 busybox 覆盖回去, 并重新建好 /usr/bin/wget 等软链。
	-@cp -af $(ROOTFS_BUILDDIR)/busybox-1.36.1/.install/bin/busybox $(ROOTFS_PREFIX)/bin/busybox
	-@cd $(ROOTFS_PREFIX)/bin && for lnk in $$(ls -la $(ROOTFS_BUILDDIR)/busybox-1.36.1/.install/bin/ 2>/dev/null | awk '/^l/{print $$NF}' | grep -v '^busybox$$'); do \
	    [ -n "$$lnk" ] && ln -sf busybox "$$lnk" || true; \
	done
	-@cd $(ROOTFS_PREFIX)/usr/bin && for lnk in $$(ls -la $(ROOTFS_BUILDDIR)/busybox-1.36.1/.install/usr/bin/ 2>/dev/null | awk '/^l/{print $$NF}'); do \
	    [ -n "$$lnk" ] && ln -sf ../../bin/busybox "$$lnk" || true; \
	done
	@touch $@

# To compatible with old dynamic lib linked with uClibc, create a link ld-uClibc.so.0.
$(ROOTFS_PREFIX)/lib/libc.so:
	@$(call mkdir,$(@D))
	DIR=$(dir $(call rt_file,$(@F))); \
		cp -avf $${DIR}/*.so $(@D)
	cd $(@D);ln -s $(notdir $@) ld-musl-arm.so.1;chmod +x $@;cd -
	cd $(@D);ln -s $(notdir $@) ld-uClibc.so.0;chmod +x $@;cd -

$(ROOTFS_PREFIX)/lib/libc.so.0 $(ROOTFS_PREFIX)/lib/libc.so.6:
	@$(call mkdir,$(@D))
	DIR=$(dir $(call rt_file,$(@F))); \
		cp -avf $${DIR}/{*.so,*.so.*} $(@D)

$(ROOTFS_PREFIX)/lib/libgcc_s.so:
	@$(call mkdir,$(@D))
	DIR=$(dir $(call rt_file,$(@F))); \
		cp -avf $${DIR}/libgcc_s.s* $(@D)

$(ROOTFS_PREFIX)/lib/libstdc++.so:
	@$(call mkdir,$(@D))
	DIR=$(dir $(call rt_file,$(@F))); \
		cp -avf $${DIR}/{libstdc++.so,libstdc++.so.*[0-9]} $(@D)
