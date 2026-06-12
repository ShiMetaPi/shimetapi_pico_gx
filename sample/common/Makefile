ifeq ($(CFG_XMEDIA_EXPORT_FLAG),)
        SDK_DIR := $(shell  cd $(CURDIR)/../.. && /bin/pwd)
endif

include $(SDK_DIR)/build/base.mk
include $(SAMPLE_DIR)/sample_base.mk

LIB_NAME := libsample_common

SDK_USR_CFLAGS += $(SAMPLE_CFLAGS) $(SAMPLE_LIBS) $(SAMPLE_INCLUDES)

#SRCS := sample_comm_vdec.c
SRCS += sample_comm_vo.c
SRCS += sample_comm_sys.c
SRCS += sample_comm_vi.c
SRCS += sample_comm_vpss.c
#SRCS += sample_comm_mcf.c
SRCS += sample_comm_venc.c
SRCS += sample_comm_isp.c
SRCS += sample_comm_vgs.c
SRCS += sample_comm_audio.c
SRCS += sample_comm_ircut.c
SRCS += loadbmp.c
SRCS += sample_comm_region.c
SRCS += sample_comm_livestream.c
SRCS += sample_comm_periph.c
SRCS += sample_comm_aov.c
SRCS += sample_comm_pq_bin.c
SRCS += sample_comm_thrd_pool.c
SRCS += sample_comm_tp.c
SRCS += sample_comm_tp_i2c.c

include $(SDK_DIR)/build/sdk_lib_rules.mk
