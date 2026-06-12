ifeq ($(CFG_XMEDIA_EXPORT_FLAG),)
        SDK_DIR := $(shell  cd $(CURDIR)/../.. && /bin/pwd)
endif

include $(SDK_DIR)/build/base.mk
include $(SAMPLE_DIR)/sample_base.mk

LIBTARGET := libpq_bin
TARGET := pq_bin

LIBS := $(SAMPLE_LIBS) $(SAMPLE_COMMON_LIB) -llive_rtsp

INCLUDES := $(SAMPLE_INCLUDES)

CFLAGS := $(SAMPLE_CFLAGS) $(LIBS) $(INCLUDES)

SRCS := $(wildcard ./*.c)

OBJS := $(patsubst %.c, %.o, $(SRCS))

.PHONY: all clean

all: $(OBJS)
	$(AT)$(CC) -fPIC -shared -o $(LIBTARGET).so $(OBJS)
	$(AT)$(CC) -o $(TARGET) $^ $(CFLAGS)

clean:
	$(AT)rm -rf $(OBJS) $(TARGET) $(LIBTARGET).so
