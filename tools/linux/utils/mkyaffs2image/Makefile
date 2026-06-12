###############################################################################
#!/usr/bin/env make
###############################################################################
#
#  Copyright (c) XMEDIA.
#  All rights reserved.
#  Create By Caizhiyong 2021/3/16
#
###############################################################################

YAFFS2 := yaffs2utils-0.2.9

all: $(TOOLS_PREFIX)/mkyaffs2image

clean: $(YAFFS2).clean

$(YAFFS2).clean:
	rm -rf $(TOOLS_BUILDDIR)/$(YAFFS2)

$(TOOLS_PREFIX)/mkyaffs2image: $(TOOLS_BUILDDIR)/$(YAFFS2)/.built
	$(call mkdir,$(@D))
	cp -arf $(TOOLS_BUILDDIR)/$(YAFFS2)/mkyaffs2image100 $(TOOLS_PREFIX)/mkyaffs2image
	touch $@

$(TOOLS_BUILDDIR)/$(YAFFS2)/.built: $(TOOLS_BUILDDIR)/$(YAFFS2)/.extracted
	cd $(<D); make;
	touch $@

$(TOOLS_BUILDDIR)/$(YAFFS2)/.extracted:
	$(call mkdir,$(@D))
	tar xf $(OPEN_SOURCE_DIR)/yaffs2/$(YAFFS2).tar.gz -C $(TOOLS_BUILDDIR)
	cp -arf $(TOOLS_SRCDIR)/mkyaffs2image/$(YAFFS2).patch/. $(@D)
	touch $@
