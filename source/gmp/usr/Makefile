SDK_DIR := $(shell cd $(CURDIR)/../../.. && /bin/pwd)

include $(SDK_DIR)/build/base.mk

objects := isp
objects += svp
objects += ext

objects_clean := $(addsuffix _clean, $(objects))

objects_dumptools := dump_tools

object_dumptools_clean := $(addsuffix _clean, $(objects_dumptools))

.PHONY: all clean $(objects) $(objects_clean) $(objects_dumptools) $(object_dumptools_clean)

all: $(objects) $(objects_dumptools)

clean: $(objects_clean) $(object_dumptools_clean)

$(objects_dumptools):$(objects)
	$(AT)$(MAKE) -C $@

$(object_dumptools_clean):
	$(AT)$(MAKE) -C $(patsubst %_clean, %, $@) clean

$(objects):
	$(AT)$(MAKE) -C $@

$(objects_clean):
	$(AT)$(MAKE) -C $(patsubst %_clean, %, $@) clean

