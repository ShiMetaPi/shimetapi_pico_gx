#!/usr/bin/env make
###############################################################################
#
#Copyright (c) XMEDIA. All rights reserved.
#
#
###############################################################################
REGTOOLS := regtools

REGTOOLS_BUILD := $(ROOTFS_BUILDDIR)/regtools

CROSS_STRIP   := $(CONFIG_XMEDIA_USR_ARM_TOOLCHAINS)-strip

CFG_FLAGS := -DVENDOR_I2C
CFG_FLAGS += -g3
CFG_FLAGS += -Wall -Wunused
CFG_FLAGS += -D__LINUX_ARM_ARCH__=7
CFG_FLAGS += -mtune=arm9tdmi
CFG_FLAGS += -DTARGET_ARM
CFG_FLAGS += -mlittle-endian
CFG_FLAGS += $(filter-out -Werror,$(SDK_USR_CFLAGS))
CFG_FLAGS += -I$(ROOTFS_SRCDIR)/$(REGTOOLS)/include/common
CFG_FLAGS += -I$(ROOTFS_SRCDIR)/$(REGTOOLS)/include/utils


COMMON_SRC=$(wildcard $(addprefix $(ROOTFS_SRCDIR)/$(REGTOOLS)/source/,common/*.c) )
COMMON_OBJ=$(patsubst %.c,$(REGTOOLS_BUILD)/source/common/%.o, $(notdir $(COMMON_SRC)))

TOOLS_SRC=$(wildcard $(addprefix $(ROOTFS_SRCDIR)/$(REGTOOLS)/source/,tools/*.c) )
TOOLS_OBJ=$(patsubst %.c,$(REGTOOLS_BUILD)/source/tools/%.o, $(notdir $(TOOLS_SRC)))

UITILS_SRC=$(wildcard $(addprefix $(ROOTFS_SRCDIR)/$(REGTOOLS)/source/,utils/*.c) )
UITILS_OBJ=$(patsubst %.c,$(REGTOOLS_BUILD)/source/utils/%.o, $(notdir $(UITILS_SRC)))

LINK_OBJ=$(wildcard $(REGTOOLS_BUILD)/*.o)
COMPILE=$(CROSS_COMPILE)-gcc $(CFG_FLAGS) -c -w -o "$(REGTOOLS_BUILD)/$(*F).o" "$<"

###############################################################################
rootfs_unstrip: $(ROOTFS_PREFIX)/bin/regtools

clean: $(REGTOOLS).clean

$(REGTOOLS).clean:
	rm -rf $(REGTOOLS_BUILD)

install_regtools = cd $(REGTOOLS_BUILD); ln -sf regtools $1; \
    cp -rf $(REGTOOLS_BUILD)/$1 $(ROOTFS_PREFIX)/bin

install_ddr_stat_tools = cd $(REGTOOLS_BUILD);\
			 echo "\#!/bin/sh" >$1;\
			 echo "" >>$1;\
			 echo "while true; do (cat /proc/ddr_bandwidth;sleep 1) done | grep \"Bandwidth Util:\"" >>$1;\
			 echo "" >>$1; \
			 chmod +x $(REGTOOLS_BUILD)/$1;\
			 cp -f $(REGTOOLS_BUILD)/$1 $(ROOTFS_PREFIX)/bin

$(ROOTFS_PREFIX)/bin/regtools: $(REGTOOLS_BUILD)/.built
	$(call mkdir,$(@D))
	$(call install_regtools,xmmd)
	$(call install_regtools,xmmd.l)
	$(call install_regtools,xmmm)
	$(call install_regtools,xmmc)
	$(call install_regtools,i2c_read)
	$(call install_regtools,i2c_write)
	$(call install_regtools,ssp_read)
	$(call install_regtools,ssp_write)
	$(call install_ddr_stat_tools,xmddrs)
	cp -rf $(REGTOOLS_BUILD)/regtools $(ROOTFS_PREFIX)/bin
	touch $@

$(REGTOOLS_BUILD)/.built: $(COMMON_OBJ) $(TOOLS_OBJ) $(UITILS_OBJ) prepare
	$(CROSS_COMPILE)-gcc -o $(REGTOOLS_BUILD)/regtools -lpthread -lrt $(filter-out -Werror,$(SDK_USR_CFLAGS)) $(LINK_OBJ)
	$(CROSS_STRIP) $(REGTOOLS_BUILD)/regtools
	touch $@

$(REGTOOLS_BUILD)/%.o : $(ROOTFS_SRCDIR)/$(REGTOOLS)/%.c
	$(call mkdir,$(@D))
	$(COMPILE)
	touch $@

prepare:
	@$(call mkdir,$(REGTOOLS_BUILD))
	(test ! -f $(ROOTFS_SRCDIR)/$(REGTOOLS)/*.o) || cp -af $(ROOTFS_SRCDIR)/$(REGTOOLS)/*.o $(REGTOOLS_BUILD)/
