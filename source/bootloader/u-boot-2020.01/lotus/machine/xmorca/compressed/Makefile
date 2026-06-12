################################################################################

SHELL             := /bin/bash
PWD                = $(shell pwd)
HW_CROSS_COMPILE   = $(CROSS_COMPILE)
BOOT_TOPDIR       ?= $(PWD)/../../../../../
BOOT_OUTDIR       ?= $(BOOT_TOPDIR)

include $(BOOT_OUTDIR)/include/autoconf.mk
include $(BOOT_OUTDIR)/include/config/auto.conf

GZIP              ?= $(if $(wildcard $(BOOT_TOPDIR)/../../../tools/utils/bin/gzip), $(BOOT_TOPDIR)/../../../tools/utils/bin/gzip, $(BOOT_TOPDIR)/tools/gzip)
SOC               := xmorca
SRCDIR            := $(PWD)

################################################################################
CC       := $(HW_CROSS_COMPILE)gcc
AR       := $(HW_CROSS_COMPILE)ar
LD       := $(HW_CROSS_COMPILE)ld
OBJCOPY  := $(HW_CROSS_COMPILE)objcopy


################################################################################
.PHONY: all $(BOOT).bin
BOOT     := u-boot
TEXTBASE := $(CONFIG_SYS_TEXT_BASE_ORI)

CFLAGS   :=-Os -fno-builtin -ffreestanding \
	-D__KERNEL__ -DTEXT_BASE=$(TEXTBASE) \
	-I$(BOOT_TOPDIR)/include \
	-I$(BOOT_OUTDIR)/include \
	-I$(BOOT_TOPDIR)/arch/arm/include \
	-I$(BOOT_TOPDIR)/lib/hw_dec \
	-fno-pic -ffunction-sections \
	-fdata-sections -fno-common -ffixed-r9    \
	-fno-common -pipe -march=armv7-a \
	-Wall -Wstrict-prototypes -fno-stack-protector \
	-D__LINUX_ARM_ARCH__=7 -D__ARM__ \
	-DCONFIG_MMC\
	$(MKFLAGS) -fno-strict-aliasing \
	$(EXTRA_CFLAGS) \


UBOOT_Z_LDS := u-boot.lds

################################################################################

START := start.o
COBJS := uart.o \
	startup.o \
	image_data.o \
	reset.o \
	exceptions.o \
	time.o \
	tail.o

SSRC  := \
	lotus/hw_dec/hw_decompress.c \
	lotus/hw_dec/hw_decompress_$(SOC).c \
	lotus/hw_dec/hw_decompress_v1.c \
	lotus/hw_dec/hw_decompress_v1.h \

SRC := $(notdir $(SSRC))

REG := $(wildcard *.reg .reg)
IMAGE_DATA_Z := image_data.gzip

CFLAGS += -DCONFIG_BOOTREG_DEFAULT=\"$(BOOT_OUTDIR)/.reg\"
CFLAGS += -DCONFIG_AUXIMAGE=\"$(SRCDIR)/auxcode$(if $(CONFIG_LOTUS_FPGA),_fpga,).bin\"
CFLAGS += -DCONFIG_AUXCODE_2048_ENABLE=y

CFLAGS += -DCONFIG_MCU_FW_IMAGE=\"$(if $(wildcard $(BOOT_OUTDIR)/qs_mcu.bin),$(BOOT_OUTDIR)/qs_mcu.bin,$(SRCDIR)/qs_mcu.bin)\"

################################################################################
.PHONY: all $(BOOT).bin
all: $(BOOT).bin
$(BOOT).bin: $(BOOT).tmp
	$(Q)cat $(BOOT).tmp > $(BOOT).bin
	$(Q)chmod 754 $(BOOT).bin
	$(Q)cp -fv $@ $(BOOT_OUTDIR)/u-boot-$(SOC).bin
	$(Q)echo $(BOOT).bin is Ready.

$(BOOT).tmp: $(BOOT).elf
	$(OBJCOPY) -O srec $< $(BOOT).srec
	$(OBJCOPY) -j .text -O binary $< $(BOOT).text
	$(OBJCOPY) --gap-fill=0xff -O binary $< $@

$(BOOT).elf: $(IMAGE_DATA_Z) $(SRC) $(START) $(COBJS) $(UBOOT_Z_LDS)
	$(LD) -Bstatic -T $(UBOOT_Z_LDS) -Ttext $(TEXTBASE) $(START) \
		$(COBJS) -Map $(BOOT).map -o $@
	$(OBJDUMP) -d  $@ > $@.asm

.PHONY: regfile
regfile:
	@if [ "$(words $(REG))" = "0" ]; then ( \
		echo '***' Need '.reg' or '*.reg' file in directory $(PWD); \
		exit 1; \
	) fi
	@if [ "$(words $(REG))" != "1" ]; then ( \
		echo '***' Found multi '.reg' or '*.reg' file in directory $(PWD); \
		echo '***' Files: $(notdir $(REG)); \
		exit 1; \
	) fi

################################################################################
$(START): start.S regfile
	$(CC) -D__ASSEMBLY__ $(CFLAGS) -o $@ $< -c

# -1 : --fast      -9 : --best
$(IMAGE_DATA_Z): $(BINIMAGE)
ifneq ($(CONFIG_LOTUS_GZIP_BOOT),)
	$(GZIP) -fNqc -7 $< > $@
else
	cp $<  $@
endif

%.o: %.c
	$(CC) $(CFLAGS) -Wall -Wstrict-prototypes \
		-fno-stack-protector -o $@ $< -c

%.o: %.S
	$(CC) -D__ASSEMBLY__ $(CFLAGS) -o $@ $< -c

image_data.o: image_data.S $(IMAGE_DATA_Z)
	$(CC) -D__ASSEMBLY__ $(CFLAGS) -o $@ $< -c

#############################################################################

$(SRC):
	ln -sf $(BOOT_TOPDIR)/$(filter %/$@,$(SSRC)) $@
################################################################################
TMPS := $(COBJS) $(START) $(SRC) \
	$(BOOT).map $(BOOT).elf $(BOOT).elf.asm $(BOOT).srec $(BOOT).bin $(BOOT).text $(BOOT).tmp \
	$(IMAGE_DATA_Z)

cmd_rm_files = @echo 'RM $(1)';$(shell [[ -n "$(1)" && "$(1)" != "*" ]] && rm -f $(1))

.PHONY: clean distclean

distclean: clean
clean:
	$(call cmd_rm_files, $(REG))
	$(call cmd_rm_files, $(TMPS))
	$(call cmd_rm_files, $(SRC))

################################################################################
.PHONY: clean
FORCE:
################################################################################
