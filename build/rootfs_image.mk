.PHONY: bootargs

bootargs:
ifeq ($(CONFIG_XMEDIA_NAND_FLASH_SUPPORT),y)
	$(AT)test -d $(XMEDIA_NAND_IMAGE_DIR) || mkdir -p $(XMEDIA_NAND_IMAGE_DIR)
	$(AT)test ! -f $(NAND_PARTITIONS) || cp -f $(NAND_PARTITIONS) $(XMEDIA_NAND_IMAGE_DIR)/
	$(AT)(test ! -f $(NAND_BOOTARGS) || (cd $(XMEDIA_NAND_IMAGE_DIR) && $(MKBOOTARGS) \
		-s $(CONFIG_XMEDIA_BOOT_ENV_SIZE) -r $(NAND_BOOTARGS) -o bootargs.bin > /dev/null))
endif

ifeq ($(CONFIG_XMEDIA_EMMC_SUPPORT),y)
	$(AT)test -d $(XMEDIA_EMMC_IMAGE_DIR) || mkdir -p $(XMEDIA_EMMC_IMAGE_DIR)
	$(AT)test ! -f $(EMMC_PARTITIONS) || cp -f $(EMMC_PARTITIONS) $(XMEDIA_EMMC_IMAGE_DIR)/
	$(AT)(test ! -f $(EMMC_BOOTARGS) || (cd $(XMEDIA_EMMC_IMAGE_DIR) && $(MKBOOTARGS) \
		-s $(CONFIG_XMEDIA_BOOT_ENV_SIZE) -r $(EMMC_BOOTARGS) -o bootargs.bin > /dev/null))
endif

ifeq ($(CONFIG_XMEDIA_SPI_SUPPORT),y)
	$(AT)test -d $(XMEDIA_SPI_IMAGE_DIR) || mkdir -p $(XMEDIA_SPI_IMAGE_DIR)
	$(AT)test ! -f $(SPI_PARTITIONS) || cp -f $(SPI_PARTITIONS) $(XMEDIA_SPI_IMAGE_DIR)/
	$(AT)(test ! -f $(SPI_BOOTARGS) || (cd $(XMEDIA_SPI_IMAGE_DIR) && $(MKBOOTARGS) \
		-s $(CONFIG_XMEDIA_BOOT_ENV_SIZE) -r $(SPI_BOOTARGS) -o bootargs.bin > /dev/null))
endif

.PHONY: fs_image fs_image_clean
fs_image: rootfs
	$(AT)$(MAKE) fs -j1
fs_image_clean:

FS_IMAGES.yy :=

FAKEROOT := fakeroot -- bash $(TOOLS_DIR)/bin/fakeroot-scripts

# $(call mkfs,<rootfs dir>,<cmd>)
mkfs = $(FAKEROOT) "$(1)" '$(2)'

$(if $(shell /usr/bin/env fakeroot -v 2>/dev/null),;, \
    $(error Command "fakeroot" not found, please install)))

# $(call fsimage_target,<fs>,<target image>)
define fsimage_target
.PHONY: $(call build_target,$(1))
$(call build_target,$(1)):$(2)

.PHONY: $(call clean_target,$(1))
$(call clean_target,$(1)):
	@rm -vf $(2)

endef

###############################################################################

.PHONY: tar
tar:
	@$(call mkdir,$(XMEDIA_IMAGE_DIR))
	@$(call mkfs,$(XMEDIA_ROOTFS_DIR), \
	    tar -C $(XMEDIA_ROOTFS_DIR) -cf $(XMEDIA_IMAGE_DIR)/rootfs.tar .)

###############################################################################

ifdef CONFIG_XMEDIA_SPI_BLOCK_SIZE
  SPI_BLOCK_SIZE := $(CONFIG_XMEDIA_SPI_BLOCK_SIZE)
else
  SPI_BLOCK_SIZE := 64k 128k 256k
endif

JFFS2_SUFFIX = $(addsuffix .jffs2,$(SPI_BLOCK_SIZE))
JFFS2_IMAGE := $(addprefix $(XMEDIA_SPI_IMAGE_DIR)/rootfs.,$(JFFS2_SUFFIX))

FS_IMAGES.y$(CONFIG_XMEDIA_ROOTFS_JFFS2) += $(JFFS2_IMAGE)

$(eval $(call fsimage_target,jffs2,$(JFFS2_IMAGE)))

$(XMEDIA_SPI_IMAGE_DIR)/rootfs.64k.jffs2:  JFFS2_BLOCK_SIZE=0x10000
$(XMEDIA_SPI_IMAGE_DIR)/rootfs.128k.jffs2: JFFS2_BLOCK_SIZE=0x20000
$(XMEDIA_SPI_IMAGE_DIR)/rootfs.256k.jffs2: JFFS2_BLOCK_SIZE=0x40000

$(XMEDIA_SPI_IMAGE_DIR)/rootfs.%.jffs2: $(MKJFFS2) FORCE
	@$(call mkdir,$(@D))
	@$(call mkfs,$(XMEDIA_ROOTFS_DIR),$(MKJFFS2) -d $(XMEDIA_ROOTFS_DIR) -l -e \
		$(JFFS2_BLOCK_SIZE) -o $@)

###############################################################################

SQUASHFS_SUFFIX = .squashfs
SQUASHFS_IMAGE := $(addprefix $(XMEDIA_SPI_IMAGE_DIR)/rootfs,$(SQUASHFS_SUFFIX))

FS_IMAGES.y$(CONFIG_XMEDIA_ROOTFS_SQUASHFS) += $(SQUASHFS_IMAGE)

$(eval $(call fsimage_target,squashfs,$(SQUASHFS_IMAGE)))

$(XMEDIA_SPI_IMAGE_DIR)/rootfs.squashfs: $(MKSQUASHFS) FORCE
	@$(call mkdir,$(@D))
	@$(call mkfs,$(XMEDIA_ROOTFS_DIR),$(MKSQUASHFS) $(XMEDIA_ROOTFS_DIR) $@ -comp xz)

###############################################################################

YAFFS2_IMAGE := $(addprefix $(XMEDIA_NAND_IMAGE_DIR)/, \
    rootfs_2k_4bit.yaffs2  \
    rootfs_2k_24bit.yaffs2 \
    rootfs_4k_4bit.yaffs2  \
    rootfs_4k_24bit.yaffs2)

FS_IMAGES.$(CONFIG_XMEDIA_ROOTFS_YAFFS)$(CONFIG_XMEDIA_NAND_FLASH_SUPPORT) += $(YAFFS2_IMAGE)

$(eval $(call fsimage_target,yaffs,$(YAFFS2_IMAGE)))

$(XMEDIA_NAND_IMAGE_DIR)/rootfs_2k_4bit.yaffs2  : ARGS=1 2
$(XMEDIA_NAND_IMAGE_DIR)/rootfs_2k_24bit.yaffs2 : ARGS=1 4
$(XMEDIA_NAND_IMAGE_DIR)/rootfs_4k_4bit.yaffs2  : ARGS=2 2
$(XMEDIA_NAND_IMAGE_DIR)/rootfs_4k_24bit.yaffs2 : ARGS=2 4

$(XMEDIA_NAND_IMAGE_DIR)/%.yaffs2: $(MKYAFFS) FORCE
	@$(call mkdir,$(@D))
	@$(call mkfs,$(XMEDIA_ROOTFS_DIR),$(MKYAFFS) $(XMEDIA_ROOTFS_DIR) $@ $(ARGS))
	@chmod 644 $(@)

###############################################################################

UBIFS_IMAGE := $(addprefix $(XMEDIA_NAND_IMAGE_DIR)/, \
    rootfs_2k_128k.ubiimg rootfs_4k_256k.ubiimg)

FS_IMAGES.$(CONFIG_XMEDIA_ROOTFS_UBIIMG)$(CONFIG_XMEDIA_NAND_FLASH_SUPPORT) += $(UBIFS_IMAGE)

$(eval $(call fsimage_target,ubiimg,$(UBIFS_IMAGE)))

$(XMEDIA_NAND_IMAGE_DIR)/%.ubiimg: $(TOOLS_DIR)/bin/mkubiimg.sh FORCE
	@$(call mkdir,$(@D))
	@$(call mkfs,$(XMEDIA_ROOTFS_DIR),bash $(<) \
	    $(wordlist 2,3,$(subst _, ,$(basename $(@F)))) $(XMEDIA_ROOTFS_DIR) $@)

###############################################################################

EXT4_IMAGE := $(addprefix $(XMEDIA_EMMC_IMAGE_DIR)/, \
    rootfs_$(CONFIG_XMEDIA_EMMC_ROOTFS_SIZE)M.ext4)

FS_IMAGES.$(CONFIG_XMEDIA_ROOTFS_EXT4)$(CONFIG_XMEDIA_EMMC_SUPPORT) += $(EXT4_IMAGE)

$(eval $(call fsimage_target,ext4,$(EXT4_IMAGE)))

$(XMEDIA_EMMC_IMAGE_DIR)/%.ext4: $(TOOLS_DIR)/bin/make_ext4fs FORCE
	@$(call mkdir,$(@D))
	@$(call mkfs,$(XMEDIA_ROOTFS_DIR),$(<) -l $(CONFIG_XMEDIA_EMMC_ROOTFS_SIZE)M -s $@ $(XMEDIA_ROOTFS_DIR))

###############################################################################
# $(XMEDIA_ROOTFS_DIR) 目录里文件变化(添加/删除/修改), 触发重新生成文件系统镜像
.PHONY: $(call build_target,fs)
$(call build_target,fs):
	@$(call mkdir,$(XMEDIA_ROOTFS_DIR))
	@echo '#this is auto generation, do not modify' > $(XMEDIA_ROOTFS_DIR).mk
	@echo 'all: $(FS_IMAGES.yy)' >> $(XMEDIA_ROOTFS_DIR).mk
	@echo '$(strip $(FS_IMAGES.yy)): \' >> $(XMEDIA_ROOTFS_DIR).mk
	@echo $$(find $(XMEDIA_ROOTFS_DIR) -type f -and ! -size 0)  >> $(XMEDIA_ROOTFS_DIR).mk
	@echo '	make -C $(SDK_DIR) $(FS_IMAGES.yy)' >> $(XMEDIA_ROOTFS_DIR).mk
	@echo 'force:; make -C $(SDK_DIR) $(FS_IMAGES.yy)'  >> $(XMEDIA_ROOTFS_DIR).mk
	@if cmp -s $(XMEDIA_ROOTFS_DIR).mk $(XMEDIA_ROOTFS_DIR)/../.rootfs.mk.old; \
		then make -f $(XMEDIA_ROOTFS_DIR).mk all; \
		else make -f $(XMEDIA_ROOTFS_DIR).mk force; fi
	@mv -f $(XMEDIA_ROOTFS_DIR).mk $(XMEDIA_ROOTFS_DIR)/../.rootfs.mk.old

.PHONY: $(call clean_target,fs)
$(call clean_target,fs):
	rm -f $(FS_IMAGES.yy) $(FS_IMAGES.yn) $(FS_IMAGES.ny) $(FS_IMAGES.nn)

FORCE:;
