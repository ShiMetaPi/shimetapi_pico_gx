ifeq ($(CFG_XMEDIA_EXPORT_FLAG),)
        SDK_DIR := $(shell  cd $(CURDIR)/../.. && /bin/pwd)
endif

include $(SDK_DIR)/build/base.mk
include $(SAMPLE_DIR)/sample_base.mk

LIBS := $(SAMPLE_LIBS) $(SAMPLE_COMMON_LIB)

INCLUDES += -I$(SDK_DIR)/sample/common
INCLUDES += -I$(SDK_DIR)/source/gmp/include

SDK_USR_CFLAGS += $(SAMPLE_CFLAGS) $(LIBS) $(INCLUDES)

sample_audio_target = sample_audio_ai \
                      sample_audio_ao \
                      sample_audio_encoder \
                      sample_audio_decoder \
                      sample_audio_doubletalk_v1 \
                      sample_audio_doubletalk_v2 \
                      sample_audio_record_v1 \
                      sample_audio_record_v2 \
                      sample_audio_record_v3 \
                      sample_audio_record_detect

audio_subdir_target := aq_utils 
audio_subdir_target_clean := $(addsuffix _clean, $(audio_subdir_target))

.PHONY: all clean $(audio_subdir_target) $(audio_subdir_target_clean)

all : $(sample_audio_target) $(audio_subdir_target)

$(audio_subdir_target):
	$(AT)$(MAKE) -C $@

$(audio_subdir_target_clean):
	$(AT)$(MAKE) -C $(patsubst %_clean, %, $@) clean

$(sample_audio_target): %: %.o
	$(AT)$(CONFIG_XMEDIA_USR_ARM_TOOLCHAINS)-gcc -o $@ $< $(SDK_USR_CFLAGS)

%.o : %.c
	$(AT)$(CONFIG_XMEDIA_USR_ARM_TOOLCHAINS)-gcc -c $< $(SDK_USR_CFLAGS)

clean : $(audio_subdir_target_clean)
	$(AT)rm -rf $(sample_audio_target) *.o
