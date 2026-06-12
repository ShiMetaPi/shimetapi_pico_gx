SDK_DIR := $(shell cd $(shell pwd)/../../.. && /bin/pwd)
include $(SDK_DIR)/build/base.mk

###############################################################################
#
#  Copyright (c) XMEDIA.
#  All rights reserved.
#  Create By Caizhiyong 2021/3/16
#
###############################################################################

TOOLS_BUILDDIR ?= $(shell pwd)/.build
TOOLS_SRCDIR   := $(TOOLS_DIR)
TOOLS_PREFIX   := $(TOOLS_DIR)/bin

CC := gcc
AR := ar

MKFLAGS := PREFIX=$(TOOLS_PREFIX)

.PHONY: all clean

all:
	@echo;
	@echo 'PC tools is ready at "$(TOOLS_PREFIX)"'
	@echo;

clean:
	rm -rf $(TOOLS_BUILDDIR)

PCTOOLS.y := ext4_utils
PCTOOLS.y += gzip
PCTOOLS.y += cramfs_tool
PCTOOLS.y += squashfs4.3
PCTOOLS.y += mtd_utils
PCTOOLS.y += mkyaffs2image
PCTOOLS.y += mkbootargs
PCTOOLS.y += nand_product


include $(foreach utils,$(PCTOOLS.y),$(TOOLS_SRCDIR)/$(utils)/Makefile)
