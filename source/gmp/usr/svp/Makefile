SDK_DIR ?= $(shell cd $(CURDIR)/../../../.. && /bin/pwd)

include $(SDK_DIR)/build/base.mk

objects :=
objects += src

objects_clean := $(addsuffix _clean, $(objects))

.PHONY: all clean prepare $(objects) $(objects_clean)

all: prepare $(objects)

clean: $(objects_clean) common_clean

$(objects): common
	$(AT)$(MAKE) -C $@

$(objects_clean):
	$(AT)$(MAKE) -C $(patsubst %_clean, %, $@) clean

common:
	$(AT)$(MAKE) -C src/alg/warp_affine

common_clean:
	$(AT)$(MAKE) -C src/alg/warp_affine clean

prepare:
