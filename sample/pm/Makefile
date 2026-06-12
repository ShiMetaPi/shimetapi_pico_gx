ifeq ($(CFG_XMEDIA_EXPORT_FLAG),)
        SDK_DIR := $(shell  cd $(CURDIR)/../.. && /bin/pwd)
endif

include $(SDK_DIR)/build/base.mk
include $(SAMPLE_DIR)/sample_base.mk

TARGET := sample_pm

LIBS := $(SAMPLE_LIBS) $(SAMPLE_COMMON_LIB) -lxmedia_pm -lxmedia_common -lxmedia_cipher

INCLUDES := $(SAMPLE_INCLUDES)

CFLAGS := $(SAMPLE_CFLAGS) $(LIBS) $(INCLUDES)

SRCS := $(wildcard ./*.c)

OBJS := $(patsubst %.c, %.o, $(SRCS))

.PHONY: all clean

all: $(OBJS)
	$(AT)$(CC) -o $(TARGET) $^ $(CFLAGS)

%.o : %.c
	$(AT)$(CC) -c -o $@ $< $(CFLAGS)

clean:
	$(AT)rm -rf $(OBJS) $(TARGET)

