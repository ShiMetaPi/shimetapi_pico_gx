SDK_DIR ?= $(shell cd $(CURDIR)/../../../.. && /bin/pwd)

include $(SDK_DIR)/build/base.mk

objects :=
objects += msensor

obj-$(XMEDIA_DRV_BUILDTYPE) += ${addsuffix /,${objects}}

objects_clean := $(addsuffix _clean, $(objects))
.PHONY: all clean prepare $(objects) $(objects_clean)

all: prepare $(objects)

clean: $(objects_clean)

$(objects):
	$(AT)$(MAKE) -C $@

$(objects_clean):
	$(AT)$(MAKE) -C $(patsubst %_clean, %, $@) clean

prepare:

