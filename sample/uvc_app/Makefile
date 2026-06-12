ifeq ($(CFG_XMEDIA_EXPORT_FLAG),)
        SDK_DIR := $(shell  cd $(CURDIR)/../.. && /bin/pwd)
endif

include $(SDK_DIR)/build/base.mk
include $(SAMPLE_DIR)/sample_base.mk

CUR_DIR := $(shell /bin/pwd)

TARGET := uvc_app

UVC_ENABLE := true
UAC_ENABLE := false

INCLUDES := $(SAMPLE_INCLUDES)

INCLUDES += -I$(CUR_DIR)
INCLUDES += -I$(CUR_DIR)/debug
INCLUDES += -I$(CUR_DIR)/common

APP_CFLAGS :=
#APP_CFLAGS += -DPRINT_COLOR_ON=1
#APP_CFLAGS += -DDUMP_STREAM_DATA=1

SRCS := $(wildcard ./*.c)
SRCS += $(wildcard ./common/*.c)
SRCS += $(wildcard ./debug/*.c)

ifeq ($(UVC_ENABLE),true)
APP_CFLAGS += -DUVC_COMPILE=1
INCLUDES += -I$(CUR_DIR)/video
INCLUDES += -I$(CUR_DIR)/video/hal
INCLUDES += -I$(CUR_DIR)/video/stream
INCLUDES += -I$(CUR_DIR)/video/hal/uapi

SRCS += $(wildcard ./video/*.c)
SRCS += $(wildcard ./video/hal/*.c)
#SRCS += $(wildcard ./video/stream/*.c)
SRCS += video/stream/video_stream.c
SRCS += video/stream/sample_video.c
SRCS += video/stream/sample_video_h264.c
SRCS += video/stream/sample_video_mjpeg.c
SRCS += video/stream/sample_video_yuv.c
#SRCS += video/stream/sample_video_mjpeg_h264.c
#SRCS += video/stream/memcpy_jpeg.c
#SRCS += video/stream/pack_h264_to_jpeg.c
endif

ifeq ($(UAC_ENABLE),true)
APP_CFLAGS += -DUAC_COMPILE=1
INCLUDES += -I$(CUR_DIR)/audio
INCLUDES += -I$(CUR_DIR)/audio/hal
INCLUDES += -I$(CUR_DIR)/audio/stream
INCLUDES += -I$(CUR_DIR)/audio/alsa/install/include

APP_CFLAGS += -L$(CUR_DIR)/audio/alsa/install/lib

SRCS += $(wildcard ./audio/*.c)
SRCS += $(wildcard ./audio/hal/*.c)
SRCS += $(wildcard ./audio/stream/*.c)

GMP_LIBS += -lasound
endif

LIBS := $(SAMPLE_LIBS) $(SAMPLE_COMMON_LIB)

CFLAGS := $(SAMPLE_CFLAGS) $(APP_CFLAGS) $(LIBS) $(INCLUDES)

OBJS := $(patsubst %.c, %.o, $(SRCS))

.PHONY: all clean prepare

all: prepare target

clean:
	$(AT)rm -rf $(OBJS) $(TARGET)

target: $(OBJS)
	$(AT)$(CC) -o $(TARGET) $^ $(CFLAGS)

%.o : %.c
	$(AT)$(CC) -c -o $@ $< $(CFLAGS)

$(OBJS): prepare

ifeq ($(UAC_ENABLE),true)
prepare:
	make -C audio/alsa all

distclean: clean
	make -C audio/alsa clean
else
prepare:

endif

