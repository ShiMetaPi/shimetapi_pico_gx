ccflags-y  += -I$(srctree)/product/driver/include
ccflags-y  += -I$(srctree)/product/driver/vo/hal
ccflags-y  += -I$(srctree)/product/driver/vo/hal/$(CHIPARCH)
ccflags-y  += -I$(srctree)/product/driver/vo/hal/$(CHIPARCH)/disp
ccflags-y  += -I$(srctree)/product/driver/vo/hal/$(CHIPARCH)/gamma
ccflags-y  += -I$(srctree)/product/driver/vo/hal/$(CHIPARCH)/csc
ccflags-y  += -I$(srctree)/product/driver/vo/hal/$(CHIPARCH)/clock
ccflags-y  += -I$(srctree)/product/driver/vo/hal/$(CHIPARCH)/intf
ccflags-y  += -I$(srctree)/product/driver/vo/hal/include
ccflags-y  += -I$(srctree)/product/driver/vo/include
ccflags-y  += -I$(srctree)/product/driver/vo/common
ccflags-y  += -I$(srctree)/product/driver/vo/common/xmfalcon

obj-y += cmd_vo.o
obj-y += common/$(CHIPARCH)/sys_hal.o
obj-y += common/$(CHIPARCH)/sys_drv.o
obj-y += common/drv_sys.o

obj-y += hal/$(CHIPARCH)/clock/hal_vo_clock.o
obj-y += hal/$(CHIPARCH)/csc/hal_vo_csc.o
obj-y += hal/$(CHIPARCH)/disp/hal_vo_disp.o
obj-y += hal/$(CHIPARCH)/gamma/hal_vo_gamma.o
obj-y += hal/$(CHIPARCH)/hal_vo.o
obj-y += hal/$(CHIPARCH)/hal_vo_chip_cfg.o
obj-y += hal/$(CHIPARCH)/hal_vo_reg.o
obj-y += hal/$(CHIPARCH)/intf/hal_vo_intf.o
obj-y += drv_vo.o

EXTRA_CFLAGS += -DVO_BOOT
EXTRA_CFLAGS += -D$(CHIPARCH)

