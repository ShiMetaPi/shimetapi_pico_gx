###############################################################################
#!/usr/bin/env make
###############################################################################
#
#  Copyright (c) XMEDIA.
#  All rights reserved.
#  Create By Caizhiyong 2021/3/16
#
###############################################################################

EXT4_UTILS := extras-refs_tags_android-4.1.1_r1-ext4_utils

all: $(TOOLS_PREFIX)/make_ext4fs

clean: $(EXT4_UTILS).clean

$(EXT4_UTILS).clean:
	rm -rf $(TOOLS_BUILDDIR)/$(EXT4_UTILS)

$(TOOLS_PREFIX)/make_ext4fs: $(TOOLS_BUILDDIR)/$(EXT4_UTILS)/.built
	$(call mkdir,$(@D))
	cp -arf $(TOOLS_BUILDDIR)/$(EXT4_UTILS)/make_ext4fs $(TOOLS_PREFIX)
	touch $@

$(TOOLS_BUILDDIR)/$(EXT4_UTILS)/.built: $(TOOLS_BUILDDIR)/$(EXT4_UTILS)/.extracted
	cd $(<D); make;
	touch $@

$(TOOLS_BUILDDIR)/$(EXT4_UTILS)/.extracted:
	$(call mkdir,$(@D))
	tar xf $(OPEN_SOURCE_DIR)/ext4_utils/$(EXT4_UTILS).tar.gz -C $(TOOLS_BUILDDIR)
	cp -arf $(TOOLS_SRCDIR)/ext4_utils/$(EXT4_UTILS).patch/. $(@D)
	touch $@
