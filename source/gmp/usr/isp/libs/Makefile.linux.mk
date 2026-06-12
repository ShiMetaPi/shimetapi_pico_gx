ISP_DIR := $(shell cd .. && /bin/pwd)
SDK_DIR ?= $(shell cd $(ISP_DIR)/../../../.. && /bin/pwd)

include $(SDK_DIR)/build/base.mk

ifeq ($(CONFIG_XMEDIA_SDK_LOG_MODE), log)
LIB_DIR := $(shell cd $(ISP_DIR)/libs/linux/log && /bin/pwd)
else
LIB_DIR := $(shell cd $(ISP_DIR)/libs/linux/nolog && /bin/pwd)
endif

STATIC_LIB := $(wildcard $(LIB_DIR)/*.a)
SHARE_LIB := $(wildcard $(LIB_DIR)/*.so)

.NOTPARALLEL:
.PHONY: all clean

all:
	$(AT)test -d $(SDK_STATIC_LIB_DIR) || mkdir -p $(SDK_STATIC_LIB_DIR)
	$(AT)test -d $(SDK_SHARE_LIB_DIR) || mkdir -p $(SDK_SHARE_LIB_DIR)
	$(AT)cp $(STATIC_LIB) $(SDK_STATIC_LIB_DIR)
	$(AT)cp $(SHARE_LIB) $(SDK_SHARE_LIB_DIR)

clean:
	$(AT)rm -f $(STATIC_LIB:$(LIB_DIR)/%=$(SDK_STATIC_LIB_DIR)/%)
	$(AT)rm -f $(SHARE_LIB:$(LIB_DIR)/%=$(SDK_SHARE_LIB_DIR)/%)
