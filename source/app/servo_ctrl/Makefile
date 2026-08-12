# PCA9685 16 路舵机控制应用（I2C3）。
# 本目录 source/app/servo_ctrl/，往上三级回到 SDK 根目录。
SDK_DIR := $(shell cd $(shell pwd)/../../.. && /bin/pwd)

# 引入 SDK 公共构建配置（定义 CC / SDK_USR_CFLAGS / SDK_LD_CFLAGS / XMEDIA_ROOTFS_DIR 等）。
include $(SDK_DIR)/build/base.mk

TARGET := servo_ctrl

SRCS := main.c pca9685.c i2c_hal.c gpio_hal.c
OBJS := $(SRCS:.c=.o)

.PHONY: all clean install

all: $(TARGET)

# 链接：.o -> 可执行文件。
$(TARGET): $(OBJS)
	$(CC) $(SDK_LD_CFLAGS) -o $@ $^

# 编译：.c -> .o。
%.o: %.c
	$(CC) $(SDK_USR_CFLAGS) -c -o $@ $<

# 安装到 staging rootfs，板端路径 /usr/bin/servo_ctrl。
install: all
	@mkdir -p $(XMEDIA_ROOTFS_DIR)/usr/bin
	@cp -f $(TARGET) $(XMEDIA_ROOTFS_DIR)/usr/bin/$(TARGET)
	@chmod 755 $(XMEDIA_ROOTFS_DIR)/usr/bin/$(TARGET)
	@echo "Installed $(TARGET) to $(XMEDIA_ROOTFS_DIR)/usr/bin/$(TARGET)"

clean:
	rm -f $(TARGET) $(OBJS)