# Rock-Paper-Scissors YOLOv8 Detection Application for GK7206
# Based on classification sample Makefile

ifeq ($(CFG_SDK_EXPORT_FLAG),)
    SDK_DIR := $(shell cd $(CURDIR)/../.. && /bin/pwd)
endif

include $(SDK_DIR)/build/base.mk
include $(SAMPLE_DIR)/sample_base.mk

TARGET := project_07_rock_paper_scissors

# Link libraries
LIBS := -lxmedia_svp -lxmedia_npu $(SAMPLE_LIBS) $(SAMPLE_COMMON_LIB) -lpthread -lm

# Include paths - use app_sample/common for web_server
INCLUDES := $(SAMPLE_INCLUDES)
INCLUDES += -I$(SDK_DIR)/app_sample/common

# Compiler flags
CFLAGS := $(SAMPLE_CFLAGS) $(LIBS) $(INCLUDES)

# Source files - include web_server.c from app_sample/common
SRCS := $(wildcard src/*.c) $(SDK_DIR)/app_sample/common/web_server.c
OBJS := $(patsubst %.c, %.o, $(SRCS))

.PHONY: all clean

all: $(OBJS)
	$(AT)$(CC) -o $(TARGET) $^ $(CFLAGS)

%.o : %.c
	$(AT)$(CC) -c -o $@ $< $(CFLAGS)

clean:
	$(AT)rm -rf $(OBJS) $(TARGET)

# Additional files to copy to target
install:
	@echo "Installing RPS application..."
	@mkdir -p $(SDK_DIR)/out/$(CFG_BOARD_TYPE)/rootfs/usr/bin/
	@mkdir -p $(SDK_DIR)/out/$(CFG_BOARD_TYPE)/rootfs/var/www/rps/
	@cp -f $(TARGET) $(SDK_DIR)/out/$(CFG_BOARD_TYPE)/rootfs/usr/bin/
	@cp -f neuron_network.xmm $(SDK_DIR)/out/$(CFG_BOARD_TYPE)/rootfs/usr/bin/
	@cp -rf web/ $(SDK_DIR)/out/$(CFG_BOARD_TYPE)/rootfs/var/www/rps/
	@echo "Installation complete!"