### module driver need define the SRCS, MOD_NAME, SDK_KER_CFLAGS ###

KERNEL_ROOT := $(KERNEL_BUILDDIR)

OBJS := $(SRCS:%.c=%.o) $(ASM_SRCS:%.S=%.o)

OBJ_DIR := $(OUT_DIR)/obj/$(subst $(SDK_DIR)/,,$(CURDIR))

FULL_OBJS += $(addprefix $(OBJ_DIR)/,$(OBJS))

TARGET := $(OBJ_DIR)/$(MOD_NAME).ko

$(MOD_NAME)-objs := $(OBJS)

obj-$(XMEDIA_DRV_BUILDTYPE) += $(MOD_NAME).o

EXTRA_CFLAGS += $(SDK_KER_CFLAGS)

.PHONY: all clean $(TARGET)

all: $(TARGET)

ifeq ($(XMEDIA_DRV_BUILDTYPE),m)
	$(AT)test -d $(SDK_KO_DIR) || mkdir -p $(SDK_KO_DIR)
	$(AT)cp -rf $(TARGET) $(SDK_KO_DIR)
endif

clean:
	$(AT)rm -rf $(OBJ_DIR)/* $(SDK_KO_DIR)/$(MOD_NAME).ko

$(TARGET):
	$(AT)mkdir -p $(OBJ_DIR)
	$(AT)touch $(OBJ_DIR)/Makefile
ifeq (_$(CONFIG_XMEDIA_LINUX_5_10_Y),_y)
	$(AT)make -C $(KERNEL_ROOT) ARCH=$(CONFIG_XMEDIA_CPU_ARCH) CROSS_COMPILE=$(CONFIG_XMEDIA_KERNEL_ARM_TOOLCHAINS)- M=$(OBJ_DIR) KBUILD_MODPOST_WARN=1 src=$(CURDIR) modules
else
	$(AT)make -C $(KERNEL_ROOT) ARCH=$(CONFIG_XMEDIA_CPU_ARCH) CROSS_COMPILE=$(CONFIG_XMEDIA_KERNEL_ARM_TOOLCHAINS)- M=$(OBJ_DIR) src=$(CURDIR) modules
endif

ifeq ($(XMEDIA_DRV_BUILDTYPE),m)
	$(AT)$(CONFIG_XMEDIA_KERNEL_ARM_TOOLCHAINS)-strip --strip-unneeded $(TARGET)
endif

