ifeq ($(CFG_SDK_EXPORT_FLAG),)
    SDK_DIR := $(shell cd $(CURDIR)/.. && /bin/pwd)
endif

include $(SDK_DIR)/build/base.mk

objects :=
objects += venc
objects += vio
objects += ainr
objects += audio
objects += npu
objects += ircut
objects += fb
objects += vo
objects += vgs
objects += tde
objects += pq_bin
objects += region
objects += cipher
objects += ive
objects += uvc_app
objects += fpn
objects += pm
objects += aov
objects_clean := $(addsuffix _clean, $(objects))

.PHONY: all clean prepare $(objects) $(objects_clean) common common_clean live_rtsp_server live_rtsp_server_clean

all: prepare $(objects)

clean: $(objects_clean)

$(objects): common live_rtsp_server
	$(AT)$(MAKE) -C $@

$(objects_clean): common_clean live_rtsp_server_clean
	$(AT)$(MAKE) -C $(patsubst %_clean, %, $@) clean

prepare:

common:
	$(AT)$(MAKE) -C common

common_clean:
	$(AT)$(MAKE) -C common clean

live_rtsp_server:
	$(AT)$(MAKE) -C live_rtsp_server

live_rtsp_server_clean:
	$(AT)$(MAKE) -C live_rtsp_server clean

