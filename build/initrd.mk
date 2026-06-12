###############################################################################
#
#  Create By Lane 2024/1/29
#
###############################################################################

SDK_DIR := $(shell cd $(shell pwd)/../.. && /bin/pwd)
include $(SDK_DIR)/build/base.mk

CROSS_COMPILE   := $(CONFIG_XMEDIA_USR_ARM_TOOLCHAINS)
STRIP := $(CROSS_COMPILE)-strip
INITRD_PREFIX   := $(XMEDIA_INITRD_DIR)
INITRD_BUILDDIR ?= $(shell pwd)/.build
INITRD_SRCDIR   := $(if $(INITRD_DIR),$(INITRD_DIR),$(shell pwd))

MKFLAGS := PREFIX=$(INITRD_PREFIX) \
	CROSS_COMPILE=$(CROSS_COMPILE)-

.PHONY: all clean

all: initrd_strip
	@echo;
	@echo 'File system is ready at "$(INITRD_PREFIX)"'
	@echo;

clean:
	rm -rf $(INITRD_BUILDDIR)
	rm -rf $(INITRD_PREFIX)

.PHONY: initrd_strip initrd_unstrip
initrd_strip: initrd_unstrip
	@set -e; for ix in $$(find $(INITRD_PREFIX)/ -type f  -a ! -name "*.ko"); \
	    do ( \
	        FILEINFO=$$(file -b $${ix}); \
	        if [ "s$$(echo $${FILEINFO} | \
	              grep '^ELF ..-bit LSB ')" != "s" ]; \
	            then ( \
		         chmod +w $${ix}; \
	                 $(STRIP) $${ix}; \
	        ) fi; \
	    ) done

TOOLS.y = scripts toybox

include $(foreach utils,$(TOOLS.y),$(INITRD_SRCDIR)/$(utils)/Makefile)
