SDK_DIR ?= $(shell cd $(CURDIR)/../../../.. && /bin/pwd)

include $(SDK_DIR)/build/base.mk

.PHONY: all clean
all: msensor
clean: msensor_clean


.PHONY: msensor msensor_clean
msensor:
	$(AT)$(MAKE) -C msensor
msensor_clean:
	$(AT)$(MAKE) -C msensor clean

