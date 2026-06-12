SDK_DIR := $(shell cd $(shell pwd)/../../.. && /bin/pwd)
SAMPLE_DIR := $(SDK_DIR)/sample

include $(SDK_DIR)/build/base.mk
include $(SDK_DIR)/sample/sample_base.mk

TARGET := factory_server
SRCS := src/factory_server.c src/factory_mjpeg.c
OBJS := $(SRCS:.c=.o)
LIBS := $(SAMPLE_LIBS) $(SAMPLE_COMMON_LIB)
INCLUDES := $(SAMPLE_INCLUDES)
CFLAGS := $(SAMPLE_CFLAGS) $(LIBS) $(INCLUDES)
WEB_FILES := web/index.html web/style.css web/app.js
SCRIPT_FILES := bin/test_rtc.sh bin/test_led.sh bin/test_sdcard.sh bin/test_spi_flash.sh bin/test_sensor.sh bin/test_mjpeg.sh
INIT_SCRIPT := bin/S60factory_server

.PHONY: all clean install factory_test_clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(SDK_LD_CFLAGS) -o $@ $^ $(CFLAGS)

%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(SAMPLE_CFLAGS) $(SAMPLE_INCLUDES) -c -o $@ $<

install: all
	@mkdir -p $(XMEDIA_ROOTFS_DIR)/usr/bin
	@cp -f $(TARGET) $(XMEDIA_ROOTFS_DIR)/usr/bin/$(TARGET)
	@chmod 755 $(XMEDIA_ROOTFS_DIR)/usr/bin/$(TARGET)
	@mkdir -p $(XMEDIA_ROOTFS_DIR)/www
	@cp -f $(WEB_FILES) $(XMEDIA_ROOTFS_DIR)/www/
	@chmod 644 $(XMEDIA_ROOTFS_DIR)/www/index.html $(XMEDIA_ROOTFS_DIR)/www/style.css $(XMEDIA_ROOTFS_DIR)/www/app.js
	@mkdir -p $(XMEDIA_ROOTFS_DIR)/usr/factory/bin
	@cp -f $(SCRIPT_FILES) $(XMEDIA_ROOTFS_DIR)/usr/factory/bin/
	@chmod 755 $(XMEDIA_ROOTFS_DIR)/usr/factory/bin/test_rtc.sh \
		$(XMEDIA_ROOTFS_DIR)/usr/factory/bin/test_led.sh \
		$(XMEDIA_ROOTFS_DIR)/usr/factory/bin/test_sdcard.sh \
		$(XMEDIA_ROOTFS_DIR)/usr/factory/bin/test_spi_flash.sh \
		$(XMEDIA_ROOTFS_DIR)/usr/factory/bin/test_sensor.sh \
		$(XMEDIA_ROOTFS_DIR)/usr/factory/bin/test_mjpeg.sh
	@mkdir -p $(XMEDIA_ROOTFS_DIR)/etc/init.d
	@cp -f $(INIT_SCRIPT) $(XMEDIA_ROOTFS_DIR)/etc/init.d/S60factory_server
	@chmod 755 $(XMEDIA_ROOTFS_DIR)/etc/init.d/S60factory_server
	@echo "Installed $(TARGET) to $(XMEDIA_ROOTFS_DIR)/usr/bin/$(TARGET)"
	@echo "Installed web files to $(XMEDIA_ROOTFS_DIR)/www"
	@echo "Installed test scripts to $(XMEDIA_ROOTFS_DIR)/usr/factory/bin"
	@echo "Installed init script to $(XMEDIA_ROOTFS_DIR)/etc/init.d/S60factory_server"

clean: factory_test_clean

factory_test_clean:
	rm -f $(TARGET) $(OBJS)
