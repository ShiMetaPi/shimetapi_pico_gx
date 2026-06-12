###############################################################################
#!/usr/bin/env make
###############################################################################
#
#  Copyright (c) XMEDIA.
#  All rights reserved.
#  Create By Caizhiyong 2021/3/16
#
###############################################################################

GZIP := gzip-1.8

all: $(TOOLS_PREFIX)/gzip

clean: $(GZIP).clean

$(GZIP).clean:
	rm -rf $(TOOLS_BUILDDIR)/$(GZIP)

$(TOOLS_PREFIX)/gzip: $(TOOLS_BUILDDIR)/$(GZIP)/.built
	$(call mkdir,$(@D))
	cp -arf $(TOOLS_BUILDDIR)/$(GZIP)/gzip $(TOOLS_PREFIX)
	cp -arf $(TOOLS_BUILDDIR)/$(GZIP)/gzip_ex $(TOOLS_PREFIX)
	cp -arf $(TOOLS_BUILDDIR)/$(GZIP)/gzip $(TOOLS_PREFIX)/hwgzip
	touch $@

$(TOOLS_BUILDDIR)/$(GZIP)/.built: $(TOOLS_BUILDDIR)/$(GZIP)/.extracted
	cd $(<D); ./configure CFLAGS="-static -m32" LDFLAGS="-static"; \
	make; \
	$(CC) main.c -static -m32 -o gzip_ex
	touch $@

$(TOOLS_BUILDDIR)/$(GZIP)/.extracted:
	$(call mkdir,$(@D))
	tar xf $(OPEN_SOURCE_DIR)/gzip/$(GZIP).tar.gz -C $(TOOLS_BUILDDIR)
	cp -arf $(TOOLS_SRCDIR)/gzip/$(GZIP).patch/. $(@D)
	cp -arf $(TOOLS_SRCDIR)/gzip/main.c $(@D)
	touch $@
