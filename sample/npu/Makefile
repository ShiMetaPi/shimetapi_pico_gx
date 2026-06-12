ifeq ($(CFG_SDK_EXPORT_FLAG),)
    SDK_DIR := $(shell cd $(CURDIR)/../.. && /bin/pwd)
endif

include $(SDK_DIR)/build/base.mk

objects :=
objects += xmm
objects += xmm_stat
objects += xmm_batch
objects += xmm_submit
objects += xmm_profiling
objects += demo_ai
objects += xmm_workspace
objects += xmm_memory_reuse
objects += xmm_private_data

objects_clean := $(addsuffix _clean, $(objects))

.PHONY: all clean $(objects) $(objects_clean)

all: $(objects)

clean: $(objects_clean)

$(objects): 
	$(AT)$(MAKE) -C $@

$(objects_clean): 
	$(AT)$(MAKE) -C $(patsubst %_clean, %, $@) clean
