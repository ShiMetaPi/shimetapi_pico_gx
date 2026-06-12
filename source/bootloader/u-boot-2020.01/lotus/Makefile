
ccflags-y        += -Wall -Werror
subdir-ccflags-y += -Wall -Werror

obj-y += machine/
obj-y += atags/
obj-y += ddr/
obj-y += timestamp/
obj-y += flash_api/
obj-$(CONFIG_HWDEC) += hw_dec/
obj-y += securec/
obj-y += misc/
ifneq ($(CONFIG_TARGET_XMORCA),y)
obj-y += atf/
endif

obj-y += drivers/
