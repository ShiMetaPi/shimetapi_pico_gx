ifeq ($(CFG_XMEDIA_EXPORT_FLAG),)
        SDK_DIR := $(shell  cd $(CURDIR)/../../.. && /bin/pwd)
endif

include $(SDK_DIR)/build/base.mk
include $(SAMPLE_DIR)/sample_base.mk

LIBS := $(SAMPLE_LIBS) $(SAMPLE_COMMON_LIB)

SAMPLE_AUDIO_AI_TARGET := ./sample_record_board_record
SAMPLE_AUDIO_DETECT_TARGET := ./sample_record_board_detect

INCLUDES += -I$(SDK_DIR)/sample/common
INCLUDES += -I$(SDK_DIR)/source/gmp/include
INCLUDES += -I$(SDK_DIR)/sample/audio/record_board/drv

SDK_USR_CFLAGS += $(SAMPLE_CFLAGS) $(LIBS) $(INCLUDES)

SAMPLE_AUDIO_AI_SRCS := ./sample_record_board_record.c
SAMPLE_AUDIO_AI_SRCS += ./drv/drv_i2c.c
SAMPLE_AUDIO_AI_SRCS += ./drv/drv_codec_es7210.c

SAMPLE_AUDIO_AI_OBJS := $(patsubst %.c, %.o, $(SAMPLE_AUDIO_AI_SRCS))


SAMPLE_AUDIO_DETECT_SRCS := ./sample_record_board_detect.c
SAMPLE_AUDIO_DETECT_SRCS += ./drv/drv_i2c.c
SAMPLE_AUDIO_DETECT_SRCS += ./drv/drv_codec_es7210.c

SAMPLE_AUDIO_DETECT_OBJS := $(patsubst %.c, %.o, $(SAMPLE_AUDIO_DETECT_SRCS))

.PHONY: all  clean

all: sample_record_board_record sample_record_board_detect

sample_record_board_record: $(SAMPLE_AUDIO_AI_OBJS)
	$(AT)$(CONFIG_XMEDIA_USR_ARM_TOOLCHAINS)-gcc -o $(SAMPLE_AUDIO_AI_TARGET) $^ $(SDK_USR_CFLAGS)

sample_record_board_detect: $(SAMPLE_AUDIO_DETECT_OBJS)
	$(AT)$(CONFIG_XMEDIA_USR_ARM_TOOLCHAINS)-gcc -o $(SAMPLE_AUDIO_DETECT_TARGET) $^ $(SDK_USR_CFLAGS)

%.o : %.c
	$(AT)$(CONFIG_XMEDIA_USR_ARM_TOOLCHAINS)-gcc -c -o $@ $< $(SDK_USR_CFLAGS)

clean:
	$(AT)rm -rf $(SAMPLE_AUDIO_AI_OBJS) $(SAMPLE_AUDIO_AI_TARGET)
	$(AT)rm -rf $(SAMPLE_AUDIO_DETECT_OBJS) $(SAMPLE_AUDIO_DETECT_TARGET)
