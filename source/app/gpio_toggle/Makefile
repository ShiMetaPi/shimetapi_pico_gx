SDK_DIR := $(shell cd $(shell pwd)/../../.. && /bin/pwd)

include $(SDK_DIR)/build/base.mk


TARGET := gpio

SRCS := main.c gpio_hal.c

OBJS := $(SRCS:.c=.o)

.PHONY: all clean install

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(SDK_LD_CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(SDK_USR_CFLAGS) -c -o $@ $<

install: all
	@mkdir -p $(XMEDIA_ROOTFS_DIR)/usr/bin
	@cp -f $(TARGET) $(XMEDIA_ROOTFS_DIR)/usr/bin/$(TARGET)
	@chmod 755 $(XMEDIA_ROOTFS_DIR)/usr/bin/$(TARGET)
	@echo "Installed $(TARGET) to $(XMEDIA_ROOTFS_DIR)/usr/bin/$(TARGET)"

clean:
	rm -f $(TARGET) $(OBJS)