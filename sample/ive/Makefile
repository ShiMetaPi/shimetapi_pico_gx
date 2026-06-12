ifeq ($(CFG_XMEDIA_EXPORT_FLAG),)
        SDK_DIR := $(shell  cd $(CURDIR)/../.. && /bin/pwd)
endif

include $(SDK_DIR)/build/base.mk
include $(SAMPLE_DIR)/sample_base.mk

TARGET := sample_ive

LIBS := -lxmedia_svp -lxmedia_md $(SAMPLE_LIBS) $(SAMPLE_COMMON_LIB)

INCLUDES := $(SAMPLE_INCLUDES)

INCLUDES += -I$(SDK_DIR)/sample/ive/sample
INCLUDES += -I$(SDK_DIR)/sample/ive/common

CFLAGS := $(SAMPLE_CFLAGS) $(LIBS) $(INCLUDES)

SRCS := $(wildcard ./*.c)
SRCS += $(wildcard ./sample/*.c)
SRCS += $(wildcard ./common/*.c)

OBJS := $(patsubst %.c, %.o, $(SRCS))

.PHONY: all clean

all: $(OBJS)
	$(AT)$(CC) -o $(TARGET) $^ $(CFLAGS)

%.o : %.c
	$(AT)$(CC) -c -o $@ $< $(CFLAGS)

clean:
	$(AT)rm -rf $(OBJS) $(TARGET)

