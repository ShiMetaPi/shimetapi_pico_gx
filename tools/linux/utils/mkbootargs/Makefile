################################################################################
#!/usr/bin/env make
###############################################################################
#
#  Copyright (c) XMEDIA.
#  All rights reserved.
#  Create By Caizhiyong 2021/3/16
#
###############################################################################
MKBOOTARGS := mkbootargs

all: $(TOOLS_PREFIX)/mkbootargs

clean: $(MKBOOTARGS).clean

$(MKBOOTARGS).clean:
	rm -rf $(TOOLS_BUILDDIR)/$(MKBOOTARGS)

$(TOOLS_PREFIX)/mkbootargs: $(TOOLS_BUILDDIR)/$(MKBOOTARGS)/.built
	$(call mkdir,$(@D))
	cp -arf $(TOOLS_BUILDDIR)/$(MKBOOTARGS)/mkbootargs $(TOOLS_PREFIX)
	touch $@

$(TOOLS_BUILDDIR)/$(MKBOOTARGS)/.built: $(TOOLS_BUILDDIR)/$(MKBOOTARGS)/.extracted
	cd $(<D); $(CC) -m32 -static -o mkbootargs crc32.c mkbootargs.c;
	touch $@

$(TOOLS_BUILDDIR)/$(MKBOOTARGS)/.extracted:
	$(call mkdir,$(@D))
	cp -arf $(TOOLS_SRCDIR)/$(MKBOOTARGS)/. $(@D)
	touch $@
