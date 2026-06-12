ifeq ($(CFG_XMEDIA_EXPORT_FLAG),)
        SDK_DIR := $(shell  cd $(CURDIR)/../../.. && /bin/pwd)
endif

include $(SDK_DIR)/build/base.mk
include $(SAMPLE_DIR)/sample_base.mk

LIBS := $(SAMPLE_LIBS) $(SAMPLE_COMMON_LIB)

INCLUDES += -I$(SDK_DIR)/sample/common
INCLUDES += -I$(SDK_DIR)/source/gmp/include

SDK_USR_CFLAGS += $(SAMPLE_CFLAGS) $(LIBS) $(INCLUDES)

sample_audio_target = aq_utils_test_speaker_vol \
                      aq_utils_test_mic_vol \
                      aq_utils_test_record \
                      aq_utils_test_talk \
                      aq_utils_test_aiao \
                      aq_utils_test_detect

SAMPLE_AUDIDO_COMMON_SRCS := $(wildcard ./config_parameter/*.c)
SAMPLE_AUDIDO_COMMON_OBJS := $(patsubst %.c, %.o, $(SAMPLE_AUDIDO_COMMON_SRCS))

.PHONY: all clean $(SAMPLE_AUDIDO_COMMON_OBJS)

all : $(sample_audio_target) $(SAMPLE_AUDIDO_COMMON_OBJS)

$(SAMPLE_AUDIDO_COMMON_OBJS): $(SAMPLE_AUDIDO_COMMON_SRCS)
	$(AT)$(CONFIG_XMEDIA_USR_ARM_TOOLCHAINS)-gcc -c $< -o $@ $(SDK_USR_CFLAGS)

$(sample_audio_target): %: %.o $(SAMPLE_AUDIDO_COMMON_OBJS)
	$(AT)$(CONFIG_XMEDIA_USR_ARM_TOOLCHAINS)-gcc -o $@ $(SAMPLE_AUDIDO_COMMON_OBJS) $< $(SDK_USR_CFLAGS)

%.o : %.c
	$(AT)$(CONFIG_XMEDIA_USR_ARM_TOOLCHAINS)-gcc -c $< $(SDK_USR_CFLAGS)

clean :
	$(AT)rm -rf $(sample_audio_target) *.o $(SAMPLE_AUDIDO_COMMON_OBJS)
