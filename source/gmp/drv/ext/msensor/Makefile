ifneq ($(srctree),)
    KERNEL_DIR := $(srctree)
    SDK_DIR := $(shell cd $(KERNEL_DIR)/../../.. && /bin/pwd)
else
    SDK_DIR := $(shell cd $(CURDIR)/../../../../.. && /bin/pwd)
endif

include $(SDK_DIR)/build/base.mk

objects := msensor_spi msensor_mng msensor_chip
objects_clean := $(addsuffix _clean, $(objects))

obj-y += $(addsuffix /, $(objects))

.PHONY: all clean $(objects) $(objects_clean)

all: $(objects)
#	$(AT)cp -rf $(BUILD_DIR)/load $(SDK_KO_DIR)

clean: $(objects_clean)

$(objects):
	$(AT)$(MAKE) -C $@

$(objects_clean):
	$(AT)$(MAKE) -C $(patsubst %_clean, %, $@) clean
