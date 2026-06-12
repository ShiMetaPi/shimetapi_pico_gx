ifeq ($(CFG_XMEDIA_EXPORT_FLAG),)
        SDK_DIR := $(shell  cd $(CURDIR)/../../ && /bin/pwd)
endif

include $(SDK_DIR)/build/base.mk

objects :=
objects += quickstart_ramfs
objects += quickstart_rootfs
objects_clean := $(addsuffix _clean, $(objects))

.PHONY: all clean $(objects) $(objects_clean)

all: $(objects)

clean: $(objects_clean)

$(objects):
	$(AT)$(MAKE) -C $@

$(objects_clean):
	$(AT)$(MAKE) -C $(patsubst %_clean, %, $@) clean

