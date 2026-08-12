# MQ2 气体传感器 + ST7789 SPI 屏显示应用（LSADC 读 AO + chardev 读 DO）。
# 本目录 source/app/gas_sensor/，往上三级回到 SDK 根目录。
SDK_DIR := $(shell cd $(shell pwd)/../../.. && /bin/pwd)

# 引入 SDK 公共构建配置（定义 CC / SDK_USR_CFLAGS / SDK_LD_CFLAGS / XMEDIA_ROOTFS_DIR 等）。
include $(SDK_DIR)/build/base.mk

TARGET := gas_sensor

SRCS := main.c mq2.c spi_hal.c gpio_hal.c st7789.c
OBJS := $(SRCS:.c=.o)

.PHONY: all clean install

all: $(TARGET)

# 链接：.o -> 可执行文件。
$(TARGET): $(OBJS)
	$(CC) $(SDK_LD_CFLAGS) -o $@ $^

# 编译：.c -> .o。
%.o: %.c
	$(CC) $(SDK_USR_CFLAGS) -c -o $@ $<

# 安装到 staging rootfs，板端路径 /usr/bin/gas_sensor。
install: all
	@mkdir -p $(XMEDIA_ROOTFS_DIR)/usr/bin
	@cp -f $(TARGET) $(XMEDIA_ROOTFS_DIR)/usr/bin/$(TARGET)
	@chmod 755 $(XMEDIA_ROOTFS_DIR)/usr/bin/$(TARGET)
	@echo "Installed $(TARGET) to $(XMEDIA_ROOTFS_DIR)/usr/bin/$(TARGET)"

clean:
	rm -f $(TARGET) $(OBJS)