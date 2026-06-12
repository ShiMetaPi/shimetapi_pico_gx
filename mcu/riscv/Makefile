CROSS_COMPILE ?= riscv64-unknown-elf-
O             ?= $(shell pwd)/out
ARCH          ?= riscv

CONFIG_INC  := $(O)/include/autoconf.h
CONFIG_MAKE := $(O)/.makefile
CONFIG_FILE := $(O)/.config
export CONFIG_FILE CONFIG_INC

sinclude $(CONFIG_FILE)

TOPDIR   := $(shell pwd)
export TOPDIR
CC       := $(CROSS_COMPILE)gcc
AR       := $(CROSS_COMPILE)ar
LD       := $(CROSS_COMPILE)ld
OBJCOPY  := $(CROSS_COMPILE)objcopy
SIZE     := $(CROSS_COMPILE)size
STRIP    := $(CROSS_COMPILE)strip
OBJDUMP  := $(CROSS_COMPILE)objdump
RM       := $(shell if [ -x "/bin/rm" ]; then echo /bin/rm; else type -P rm; fi)
LS       := $(shell if [ -x "/bin/ls" ]; then echo /bin/ls; else type -P ls; fi)
MCU     := mcu
RM       := $(RM) -f

export CC AR LD OBJCOPY RM TOPDIR MCU SIZE STRIP LS

include $(TOPDIR)/scripts/Makefile.define

V ?= 0

ifeq ($(V),1)
    Q     :=
    QUIET :=
else
    Q     := @
    QUIET := -s
endif

MKFLAGS := $(QUIET) -f $(TOPDIR)/scripts/Makefile.build
CLFLAGS := $(QUIET) -f $(TOPDIR)/scripts/Makefile.clean

export MKFLAGS CLFLAGS Q QUIET

MKDEP := .mkdep
export MKDEP

ARFLAGS  += rcs
LDFLAGS  +=

CFLAGS   += -fno-builtin -fno-common -ffreestanding -nostdinc \
	-I$(shell pwd)/include -I$(O)/include -I$(TOPDIR) -pipe -g -gdwarf-4 -Wall -Werror \
	-DCONFIG_$(strip $(call upper, $(ARCH)))_ARCH

GC_ENABLE ?= y
CFLAGS   += $(if $(GC_ENABLE),-fdata-sections -ffunction-sections)
CFLAGS   += $(shell echo $(CONFIG_CFLAGS))
LDFLAGS  += $(shell echo $(CONFIG_LDFLAGS))

ARCH_DIR := $(ARCH)/

CFLAGS  += -I$(TOPDIR)/$(ARCH_DIR)include

ASFLAGS  += $(CFLAGS) -D__ASSEMBLY__

export ARFLAGS LDFLAGS CFLAGS ASFLAGS

LIB_DIRS := init/ libs/ drivers/ common/

ifdef CONFIG_MEDIA_SUPPORT
MEDIA = $(if $(wildcard $(TOPDIR)/media/Makefile),y,)
endif

LIB_DIRS-stage2 += $(call is_enabled, MEDIA,media/,)
CFLAGS += $(call is_enabled, MEDIA,-I$(TOPDIR)/media/include,)

ifeq ($(strip $(CONFIG_UT_ENABLE)),y)
LIB_DIRS-stage2 += test/
endif

LIBS := $(foreach LIB,$(LIB_DIRS),$(LIB)lib$(LIB:/=).a)
LIBS-stage2 := $(foreach LIB,$(LIB_DIRS-stage2),$(LIB)lib$(LIB:/=).a)

################################################################################
sinclude $(CONFIG_MAKE)

PHONYS += help
help:
	@echo "Usage: make [config]"
	@echo "       make clean"
	@echo "       make distclean"
	@echo "       make help"
	@echo "       make"
	@echo "  support configs:"
	@for ix in $(notdir $(wildcard $(TOPDIR)/configs/*)); do ( \
		echo "    $${ix}"; \
	) done
	@echo;

PHONYS += $(MCU).bin
$(MCU).bin: $(MCU).elf
	$(call show_cmd,OBJCOPY,$@)
	$(Q)$(OBJCOPY) -R .comment --gap-fill=0xff -O binary $(O)/$< $(O)/$@
	$(call show_cmd,MCU,$@)
	$(Q)$(OBJDUMP) -D $(O)/$(MCU).elf > $(O)/mcu.asm
	@echo "Config File: $(CONFIG_PLATFORM)_defconfig"
	@echo "Entry Point: $(CONFIG_TEXT_BASE)"
	@MSG=$$(cd $(O); $(LS) -gG --time-style=long-iso $@); \
		echo "Image Size:  $$(echo $${MSG} | awk '{print $$3}') Bytes"; \
		echo "Create:      $$(echo $${MSG} | awk '{print $$4" "$$5}')"
	@echo "Output Path: $(O)"
	@echo "  Image $(@) is ready"

PHONYS += $(MCU).bin
$(MCU).elf: $(LIBS) $(LIBS-stage2)
	$(Q)$(MAKE) $(MKFLAGS) SRCDIR="$(ARCH_DIR)" LIBS="$(LIBS)" LIBS-stage2="$(LIBS-stage2)" O="$(O)/" $(@)

PHONYS += $(LIBS) $(LIBS-stage2)
$(LIBS): $(O)/$(MKDEP) force
	@touch init/main.c
	$(Q)$(MAKE) $(MKFLAGS) -C $(TOPDIR)/$(@D)/ SRCDIR="$(@D)/" O="$(O)/" $(@F)

$(LIBS-stage2): $(O)/$(MKDEP) force
	$(Q)$(MAKE) $(MKFLAGS) -C $(TOPDIR)/$(@D)/ SRCDIR="$(@D)/" O="$(O)/" $(@F)


%: $(TOPDIR)/configs/%
	$(call show_cmd,GEN,$(notdir $(CONFIG_MAKE)))
	$(call checkdir,$(dir $(CONFIG_MAKE)))
	$(Q)(echo "# auto produce, don't modify this ") > $(CONFIG_MAKE)
	$(Q)(echo "all: $(MCU).bin";) >> $(CONFIG_MAKE)
	$(call show_cmd,GEN,$(notdir $(CONFIG_FILE)))
	$(call checkdir,$(dir $(CONFIG_FILE)))
	$(Q)cp "$<" "$(CONFIG_FILE)"
	$(call show_cmd,GEN,$(notdir $(CONFIG_INC)))
	$(call checkdir,$(dir $(CONFIG_INC)))
	$(Q)(echo "/* auto produce, don't modify this */") > $(CONFIG_INC)
	$(Q)set -e; cat $(CONFIG_FILE) | sed -n -f $(TOPDIR)/scripts/env2inc.sed >> $(CONFIG_INC)

force:;

$(O)/$(MKDEP): $(TOPDIR)/Makefile
	@touch $@

PHONYS += clean
clean: $(addsuffix .clean,$(LIBS) $(ARCH_DIR))
	$(Q)$(RM) -f $(O)/{$(MCU).bin,$(MCU).elf,$(MCU).map,$(MKDEP)}

$(addsuffix .clean,$(LIBS) $(ARCH_DIR)):
	$(call show_cmd,ENTRY,$(TOPDIR)/$(@D))
	$(Q)$(MAKE) $(CLFLAGS) SRCDIR="$(@D)/" O="$(O)/" clean

PHONYS += distclean
distclean:
	$(Q)$(RM) -rf $(O)
	$(Q)$(RM) -rf cscope.files cscope.in.out cscope.out cscope.po.out tags

PHONYS += cscope
cscope:
	$(Q)find -name "*.S" -o -name "*.c" -o -name "*.h" -o -name "*.s" >cscope.files
	$(Q)cscope -bkq -i ./cscope.files
	$(Q)ctags -R

################################################################################
.PHONY: $(PHONYS)
################################################################################
