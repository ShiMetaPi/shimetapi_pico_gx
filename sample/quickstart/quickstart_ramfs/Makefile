ifeq ($(CFG_XMEDIA_EXPORT_FLAG),)
        SDK_DIR := $(shell  cd $(CURDIR)/../../../ && /bin/pwd)
endif

include $(SDK_DIR)/build/base.mk
include $(SDK_DIR)/sample/quickstart/quickstart_base.mk

INCLUDES := -I$(SDK_DIR)/sample/quickstart/media_comm
INCLUDES += -I$(SDK_DIR)/sample/quickstart
INCLUDES += $(SAMPLE_INCLUDES)

LIBS := $(SAMPLE_LIBS)

TARGET:= sample_quickstart

CFLAGS := $(SAMPLE_CFLAGS) $(LIBS) $(INCLUDES)

SRCS := $(wildcard ./quickstart_ramfs.c)
SRCS += $(wildcard ./../quickstart_comm.c)
SRCS += $(wildcard ./../media_comm/media_comm.c)

OBJS := $(patsubst %.c, %.o, $(SRCS))


.PHONY: all clean

all: $(OBJS)
	$(AT)$(CC) -o $(TARGET) $^ $(CFLAGS)
	$(AT)$(STRIP) $(TARGET)

%.o : %.c
	$(AT)$(CC) -c -o $@ $< $(CFLAGS)

clean:
	$(AT)rm -rf $(OBJS) $(TARGET)
