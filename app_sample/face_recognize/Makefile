ifeq ($(CFG_SDK_EXPORT_FLAG),)
        SDK_DIR := $(shell cd $(CURDIR)/../.. && /bin/pwd)
endif

include $(SDK_DIR)/build/base.mk
include $(SAMPLE_DIR)/sample_base.mk

TARGET := face_recognize

LIBS := -lxmedia_svp -lxmedia_npu $(SAMPLE_LIBS) $(SAMPLE_COMMON_LIB) -lpthread

INCLUDES := $(SAMPLE_INCLUDES)
INCLUDES += -I$(SDK_DIR)/app_sample/common

CFLAGS := $(SAMPLE_CFLAGS) $(LIBS) $(INCLUDES)

SRCS := $(wildcard src/*.c) $(SDK_DIR)/app_sample/common/web_server.c
OBJS := $(patsubst %.c, %.o, $(SRCS))

.PHONY: all clean

all: $(OBJS)
	$(AT)$(CC) -o $(TARGET) $^ $(CFLAGS)

%.o : %.c
	$(AT)$(CC) -c -o $@ $< $(CFLAGS)

clean:
	$(AT)rm -rf $(OBJS) $(TARGET)
