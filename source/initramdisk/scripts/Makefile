#!/usr/bin/env make
###############################################################################
#
#  Create By Lane 2024/1/29
#
###############################################################################

# $(call _rt_file,<libc>)
_rt_file = $(shell $(CONFIG_XMEDIA_USR_ARM_TOOLCHAINS)-gcc \
    $(SDK_USR_CFLAGS) -print-file-name=$(1))

# $(call rt_file,<libc1, libc2, ...>)
rt_file = $(foreach lib,$(1),$(shell if [ -f "$(call _rt_file,$(lib))" ]; \
    then ( echo "$(call _rt_file,$(lib))"; ) else ( echo ""; ) fi ))

# $(call rt_notdir,<libc1, libc2, ...>)
rt_notdir = $(if $(strip $(call rt_file,$(1))),\
    $(notdir $(call rt_file,$(1))),\
    $(info *** $(CONFIG_XMEDIA_ARM_TOOLCHAINS_NAME)-gcc $(SDK_USR_CFLAGS)) \
    $(error not found $(1), please check compiler and cflags))

# libc.so
# RT-LIBS = $(call rt_notdir,libc.so) //拷贝c库会增加initrd的大小，toybox采用静态链接, 不需要动态c库，
#                                       根据实际业务app需求来确定是否拷贝动态c库。

initrd_unstrip: $(INITRD_PREFIX)/init \
    $(addprefix $(INITRD_PREFIX)/lib/,$(RT-LIBS))

$(INITRD_PREFIX)/init:
	$(call mkdir,$(@D))
	tar xzf $(INITRD_SRCDIR)/scripts/initramdisk.tgz -C $(INITRD_PREFIX)/..
	touch $@

# To compatible with old dynamic lib linked with uClibc, create a link ld-uClibc.so.0.
$(INITRD_PREFIX)/lib/libc.so:
	@$(call mkdir,$(@D))
	DIR=$(dir $(call rt_file,$(@F))); \
		cp -avf $${DIR}/*.so $(@D)
	cd $(@D);ln -s $(notdir $@) ld-musl-arm.so.1;chmod +x $@;cd -
	cd $(@D);ln -s $(notdir $@) ld-uClibc.so.0;chmod +x $@;cd -

