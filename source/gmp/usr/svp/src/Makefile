SDK_DIR ?= $(shell cd $(CURDIR)/../../../../.. && /bin/pwd)
include $(SDK_DIR)/build/base.mk

LIB_NAME := libxmedia_svp

SRCS := $(wildcard ./*.c)
SRCS += $(wildcard ./detect/*.c)
SRCS += $(wildcard ./recognition/*.c)
SRCS += $(wildcard ./video_attributes/*.c)
SRCS += $(wildcard ./xmedia_hal/*.c)
SRCS += $(wildcard ./alg/*.c)
SRCS += $(wildcard ./common/*.c)
SRCS += $(wildcard ./post_process/*.c)
SRCS += $(wildcard ./task_instance/*.c)

CPP_SRCS += $(wildcard ./recognition/lpr/*.cpp)
CPP_SRCS += $(wildcard ./recognition/*.cpp)
CPP_SRCS += $(wildcard ./recognition/lpr/cvi_utils/*.cpp)
CPP_SRCS += $(wildcard ./recognition/lpr/general_detect/*.cpp)
CPP_SRCS += $(wildcard ./recognition/lpr/general_track/*.cpp)
CPP_SRCS += $(wildcard ./recognition/lpr/ipu_common/*.cpp)
CPP_SRCS += $(wildcard ./recognition/lpr/vehicle_reco/*.cpp)
#CFLAGS_MODULE += -O3 -mfpu=neon -mfloat-abi=softfp -ffreestanding

BASE_LIB = $(GMP_DIR)/usr/svp/src/alg/warp_affine/lib/libwarp_affine.a
LIB_EXTRACT = $(shell if [ -f $(3) ] ; then \
        mkdir -p $(1) && cd $(1) && $(2)-ar -x $(3); \
    fi)

OBJ_DIR := $(OUT_DIR)/obj/$(subst $(SDK_DIR)/,,$(CURDIR))

$(call LIB_EXTRACT, $(OBJ_DIR), $(TOOLCHAIN), $(BASE_LIB))

BASE_LIB_OBJS := $(shell $(TOOLCHAIN)-ar -t $(BASE_LIB))
SRCS += $(BASE_LIB_OBJS)

SDK_USR_CFLAGS += -I$(GMP_DIR)/include
SDK_USR_CFLAGS += -I$(GMP_DIR)/usr/include
SDK_USR_CFLAGS += -I$(GMP_DIR)/usr/common/include
SDK_USR_CFLAGS += -I$(GMP_DIR)/drv/osal/include

SDK_USR_CFLAGS += -I../include
SDK_USR_CFLAGS += -I../include/common
SDK_USR_CFLAGS += -I../include/detect
SDK_USR_CFLAGS += -I../include/recognition
SDK_USR_CFLAGS += -I../include/video_attributes
SDK_USR_CFLAGS += -I../include/xmedia_hal
SDK_USR_CFLAGS += -I../include/post_process
SDK_USR_CFLAGS += -I../include/task_instance
SDK_USR_CFLAGS += -I../common/Eigen
SDK_USR_CFLAGS += -I../include/alg
SDK_USR_CFLAGS += -I../src/recognition/lpr
#SDK_USR_CFLAGS += -DSVP_DEBUG_SAVE_FILE
#SDK_USR_CFLAGS += -DRCNN_DEBUG_SAVE_FILE
#SDK_USR_CFLAGS += -DFR_DEBUG_SAVE_FILE
SDK_USR_CFLAGS += -DDEBUG_PROC_INFO

include $(SDK_DIR)/build/sdk_lib_rules.mk
