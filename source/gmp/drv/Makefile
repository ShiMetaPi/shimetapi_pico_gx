SDK_DIR := $(shell cd $(CURDIR)/../../.. && /bin/pwd)

include $(SDK_DIR)/build/base.mk

objects := osal lsadc misc ext

ifeq ($(XMEDIA_DRV_BUILDTYPE),y)
objects += init
endif

obj-$(XMEDIA_DRV_BUILDTYPE) += $(addsuffix /, $(objects))
objects_clean := $(addsuffix _clean, $(objects))

.PHONY: all clean $(objects) $(objects_clean)

all: $(objects)

ifeq ($(XMEDIA_DRV_BUILDTYPE),m)
	$(AT)cp -rf $(BUILD_DIR)/load $(SDK_KO_DIR)
endif

clean: $(objects_clean)

$(objects):
	$(AT)$(MAKE) -C $@

$(objects_clean):
	$(AT)if [ -d "$(patsubst %_clean, %, $@)" ]; then \
		$(MAKE) -C $(patsubst %_clean, %, $@) clean; \
	fi
