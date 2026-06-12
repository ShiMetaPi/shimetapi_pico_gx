################################################################################
#!/usr/bin/env make
###############################################################################
#
#  Copyright (c) XMEDIA.
#  All rights reserved.
#  Create By Caizhiyong 2021/3/16
#
###############################################################################
LOCAL_CFLAGS := -fPIE

LOCAL_LDFLAGS := -Wl,-z,relro -Wl,-z,now -Wl,-z,noexecstack -fPIE -pie

GCC_VERSION_GE49 := $(shell echo `gcc -dumpversion | cut -f1-2 -d.` \>= 4.9 | sed -e 's/\./*100+/g' | bc)

ifeq ($(GCC_VERSION_GE49),1)
LOCAL_CFLAGS += -fstack-protector-strong
else
LOCAL_CFLAGS += -fstack-protector-all
endif

NAND_PRODUCT := nand_product

all: $(TOOLS_PREFIX)/nand_product

clean: $(NAND_PRODUCT).clean

$(NAND_PRODUCT).clean:
	rm -rf $(TOOLS_BUILDDIR)/$(NAND_PRODUCT)

$(TOOLS_PREFIX)/nand_product: $(TOOLS_BUILDDIR)/$(NAND_PRODUCT)/.built
	$(call mkdir,$(@D))
	cp -arf $(TOOLS_BUILDDIR)/$(NAND_PRODUCT)/nand_product $(TOOLS_PREFIX)
	touch $@

$(TOOLS_BUILDDIR)/$(NAND_PRODUCT)/.built: $(TOOLS_BUILDDIR)/$(NAND_PRODUCT)/.extracted
	cd $(<D); $(CC) $(LOCAL_CFLAGS) -c -o nandc_ecc.o nandc_ecc.c; \
	$(CC) $(LOCAL_CFLAGS) -c -o nandc_random.o nandc_random.c; \
	$(CC) $(LOCAL_CFLAGS) -c -o oob_config_v100.o oob_config_v100.c; \
	$(CC) $(LOCAL_CFLAGS) -c -o main.o main.c; \
	$(CC) $(LOCAL_LDFLAGS) -static -o nand_product nandc_ecc.o nandc_random.o oob_config_v100.o main.o
	touch $@

$(TOOLS_BUILDDIR)/$(NAND_PRODUCT)/.extracted:
	$(call mkdir,$(@D))
	cp -arf $(TOOLS_SRCDIR)/$(NAND_PRODUCT)/. $(@D)
	touch $@