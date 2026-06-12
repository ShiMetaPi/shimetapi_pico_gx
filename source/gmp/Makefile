ifeq ($(CFG_SDK_EXPORT_FLAG),)
	SDK_DIR := $(shell cd $(CURDIR)/../.. && /bin/pwd)
endif

include $(SDK_DIR)/build/base.mk

.PHONY: all clean

all:
	$(AT)test -d $(SDK_STATIC_LIB_DIR) || mkdir -p $(SDK_STATIC_LIB_DIR)
	$(AT)test -d $(SDK_SHARE_LIB_DIR) || mkdir -p $(SDK_SHARE_LIB_DIR)
	$(AT)cp $(GMP_DIR)/lib/$(CONFIG_XMEDIA_USR_ARM_TOOLCHAINS)/$(OBJ_LOG_MODE)/static/*.a $(SDK_STATIC_LIB_DIR) -rf
	$(AT)cp $(GMP_DIR)/lib/$(CONFIG_XMEDIA_USR_ARM_TOOLCHAINS)/$(OBJ_LOG_MODE)/share/*.so $(SDK_SHARE_LIB_DIR) -rf
	$(AT)cp -rf $(GMP_DIR)/include $(OUT_DIR)
	$(AT)$(MAKE) -C drv/
	$(AT)$(MAKE) -C usr/
clean:
	$(AT)$(MAKE) -C drv/ clean
	$(AT)$(MAKE) -C usr/ clean

