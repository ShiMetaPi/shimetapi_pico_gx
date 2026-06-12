SUPPORT_CHIP :=
SUPPORT_CHIP += xm7206v10 xm7206v10b xm7206v11a

UBOOT_FILE := uboot.bin

ifeq ($(AES_ENC), true)
UBOOT_SIGNED_FILE := uboot_$(CHIP)_sig_enc.bin
else
UBOOT_SIGNED_FILE := uboot_$(CHIP)_sig.bin
endif

all: rsa4096pem_gen uboot_rsa_sig.bin

rsa4096pem_gen:
	@./rsa4096pem.sh

uboot_rsa_sig.bin: rsa4096pem_gen
ifeq ($(findstring $(CHIP),$(SUPPORT_CHIP)),)
	$(error you must set valid CHIP:$(SUPPORT_CHIP)!)
endif
	./create_secure_boot_v4.sh $(UBOOT_FILE) $(UBOOT_SIGNED_FILE) $(AES_ENC)

clean:
	rm -fr output/

distclean:clean
	rm -fr rsa4096pem/
	rm -rf aes_otp_cfg.txt
