#! /bin/bash

#If want to decrypt ddr_init.bin,u-boot.bin and their signatures,
#you must to set the KEY and IV for aes, and IV can't be zero;
#otherwise, u-boot.bin and their signatures would not be decrypted.

#The AUX_IV, BOOT_IV and ROOT_KEY0 must be used at the same time.
# AUX_IV=00112233445566778899aabbccddeeff
# BOOT_IV=00ffeeddccbbaa998877665544332211
# M4_FW_IV=
# ROOT_KEY0=67452301efcdab8998badcfe103254763322110077665544bbaa9988ffeeddcc

# if AES_ENABLE=true, the auxcode area and bootcode area will be encrypted by AES
AES_ENABLE=$3

if [ "is${AES_ENABLE}" = "istrue" ];then
# AES encryption iv
BOOT_IV=00ffeeddccbbaa998877665544332211
# AES encryption key
ROOT_KEY0=67452301efcdab8998badcfe103254763322110077665544bbaa9988ffeeddcc
else
BOOT_IV=
ROOT_KEY0=
fi

# uboot.bin original file
uboot_file=${1}
# signed uboot.bin file
uboot_sig_file=${2}

if [ -f ${uboot_file} ];then
echo ""
echo "     uboot_file    = $uboot_file";
#echo "     AUX_IV        = $AUX_IV";
echo "     BOOT_IV       = $BOOT_IV";
echo "     ROOT_KEY0     = $ROOT_KEY0";
echo "";
else
echo -e "\033[0;31;1mError:${uboot_file} is not exist!\033[0m"
exit
fi

dec2hex(){
	printf "0x%08x" $1
}

function H_TO_NL {
        local tmp1=$[$1 & 0xff]
        local tmp2=$[$[$1 & 0xff00] >> 8]
        local tmp3=$[$[$1 & 0xff0000] >> 16]
        local tmp4=$[$[$1 & 0xff000000] >> 24]
        local val=$[$[$tmp1 << 24] | $[$tmp2 << 16] | $[$tmp3 << 8] | $tmp4]

        echo $val
}

function HASH_OTP_TABLE {
	Str=`cat $1`
	echo $Str > $2
	echo ================================================================================== >> $2
	echo "input_file: $1" >> $2
	echo ================================================================================== >> $2
	for((i=0; i<64; i=i+8))
	do
		word=${Str:i:8};
		word=$(printf "0x%s" $word)
		word=$(H_TO_NL $word)
		word=$(printf "0x%08x" $word)
		reg=$[0x100A000C + $(($i/2))]     # OTP reg
		reg=$(printf "%08x" $reg)
		echo rootkey_hash[$(($i/8))]=mw 0x$reg $word  >> $2
	done
}

function AES_OTP_TABLE {
	Str=$1
	echo "AES KEY FILE" > $2
	echo ================================================================================== >> $2
	echo "AES KEY:: $1" >> $2
	echo ================================================================================== >> $2
	for((i=0; i<64; i=i+8))
	do
		word=${Str:i:8};
		word=$(printf "0x%s" $word)
		word=$(H_TO_NL $word)
		word=$(printf "0x%08x" $word)
		reg=$[0x100A000C + $(($i/2))]     # OTP reg
		reg=$(printf "%08x" $reg)
		echo aes_key_val[$(($i/8))]=mw 0x$reg $word  >> $2
	done
}

################ uboot.bin layout#######################
HEAD_AREA_LEN=10240

# 1. key area
KEY_AREA_POS=0
KEY_AREA_LEN=1024

# 2. param area
PARAMS_AREA_POS=1024
PARAMS_AREA_LEN=8640

# 3. unchecked area
UNCHECK_AREA_POS=10176
UNCHECK_AREA_LEN=64

# 4. auxcode area
AUXCODE_AREA_POS=10240
########################################################

## 1. key area
if [ -f rsa4096pem/rsa_pub_4096.pem ];then
echo "....................start........................."
echo "---------------calc hash for keys---------------"
test -d output || mkdir -p output
rm -rf output/*
openssl base64 -d -in rsa4096pem/rsa_pub_4096.pem -out private_4096.bin
dd if=./private_4096.bin of=./fb1 bs=1 skip=33 count=512
for((i=1;i<=509;i++))
do
	echo 0x00 | xxd -r >> fb2
done
dd if=./private_4096.bin of=./fb3 bs=1 skip=547 count=3
cat fb1 fb2 fb3 > output/rsa_pub_4096.bin

filesize=`wc -c < output/rsa_pub_4096.bin`
if [ $filesize == $KEY_AREA_LEN ];then
	echo ""
	echo 0:RSA_PUB creat OK!
	echo RSA_PUB file_size = $filesize
	echo ""
else
	echo 0:RSA_PUB creat error!
	echo RSA_PUB file_size = $filesize
	echo ""
fi
## calc key area hash
openssl dgst -sha256 -r  -hex output/rsa_pub_4096.bin > rsa4096pem/rsa_pub_4096_sha256.txt
rm -f fb1 fb2 fb3 private_4096.bin

## 2. params area
echo "---------------make signature for params---------------"
dd if=$uboot_file of=output/params.bin bs=1 skip=$PARAMS_AREA_POS count=$PARAMS_AREA_LEN conv=notrunc
## aux iv/flag
# if [ $AUX_IV ];then
# echo 0x$AUX_IV | xxd -r > aux_iv.bin
# echo 0x12c8132a | xxd -r > aux_enc_flag.bin
# dd if=./aux_enc_flag.bin of=output/params.bin bs=1 seek=12 count=4 conv=notrunc
# dd if=./aux_iv.bin of=output/params.bin bs=1 seek=24 count=16 conv=notrunc
# rm aux_iv.bin aux_enc_flag.bin
# fi

## m4 fw iv/flag
# if [ $M4_FW_IV ];then
# echo 0x$M4_FW_IV | xxd -r > m4_iv.bin
# echo 0x12c8132a | xxd -r > m4_enc_flag.bin
# dd if=./m4_enc_flag.bin of=output/params.bin bs=1 seek=20 count=4 conv=notrunc
# dd if=./m4_iv.bin of=output/params.bin bs=1 seek=48 count=16 conv=notrunc
# rm m4_iv.bin m4_enc_flag.bin
# fi

## boot iv/flag
if [ $BOOT_IV ];then
echo 0x$BOOT_IV | xxd -r > boot_iv.bin
echo 0x12c8132a | xxd -r > boot_enc_flag.bin
dd if=./boot_enc_flag.bin of=output/params.bin bs=1 seek=16 count=4 conv=notrunc
dd if=./boot_iv.bin of=output/params.bin bs=1 seek=40 count=16 conv=notrunc
rm boot_iv.bin boot_enc_flag.bin
fi

## make params area signature
openssl dgst -sha256 -sign rsa4096pem/rsa_priv_4096.pem -sigopt rsa_padding_mode:pss -sigopt rsa_pss_saltlen:-1 -out output/params_sig.bin output/params.bin

## 3. unchecked area
dd if=$uboot_file of=output/unchecked_area.bin bs=1 skip=$UNCHECK_AREA_POS count=$UNCHECK_AREA_LEN conv=notrunc

## 4. auxcode area
echo "---------------prepare auxcode area---------------"
val=$(hexdump -e '16/4 "%04x"' -n 4 -s 0 output/params.bin)
auxcode_area_len=$(printf "%d\n" 0x$val)
dd if=$uboot_file of=output/auxcode_check.bin bs=1 skip=$AUXCODE_AREA_POS count=$auxcode_area_len conv=notrunc

# echo "---------------make signature for auxcode---------------"
## make auxcode signature
# openssl dgst -sha256 -sign rsa4096pem/rsa_priv_4096.pem -sigopt rsa_padding_mode:pss -sigopt rsa_pss_saltlen:-1 -out output/auxcode_sig.bin output/auxcode_check.bin
## enc auxcode area
# if [ $AUX_IV ]; then
# echo "---------------Auxcode encrypt enable---------------"
## IV and KEY have set,
## Obtain a new KEY by decrypting the ECB mode.
# echo 0x1b7b45c0abd412c0382a0242c2824a61 | xxd -r > seed_1.bin
# echo 0xcad7733ee80bd0741d9128627ea880ad | xxd -r > seed_2.bin
# openssl enc -nopad -d -nosalt -aes-256-ecb -K "$ROOT_KEY0"  -in seed_1.bin -out out_1.bin
# openssl enc -nopad -d -nosalt -aes-256-ecb -K "$ROOT_KEY0"  -in seed_2.bin -out out_2.bin
# cat out_2.bin >> out_1.bin
# KEY_ecb=$(xxd -ps out_1.bin | sed 'N;s/\n//g')
# rm out_*.bin seed_*.bin
# echo Obtain a new KEY by decrypting the ECB mode!
# echo new_KEY = $KEY_ecb
# echo ""
## Use the new KEY and IV to encrypt the image in CBC mode.
# openssl enc -aes-256-cbc -nopad -K "$KEY_ecb"  -iv "$AUX_IV" -in output/auxcode_check.bin -out output/auxcode_check_enc.bin
# mv output/auxcode_check_enc.bin output/auxcode_check.bin
# fi

## MCU FW
val=$(hexdump -e '16/4 "%04x"' -n 4 -s 8252 output/params.bin)
mcu_fw_area_len=$(printf "%d\n" 0x$val)
if [ $mcu_fw_area_len == 0 ]; then
touch output/mcu_fw_area.bin
else
skip_offset=$[ $HEAD_AREA_LEN + $auxcode_area_len ]
dd if=$uboot_file of=output/mcu_fw_area.bin bs=1 skip=$skip_offset count=$mcu_fw_area_len conv=notrunc
fi

## 5. boot area
echo "---------------make signature for boot area---------------"
val=$(hexdump -e '16/4 "%04x"' -n 4 -s 4 output/params.bin)
boot_check_area_len=$(printf "%d\n" 0x$val)
echo boot_check_area_len=$boot_check_area_len
boot_check_area_pos=$[ $HEAD_AREA_LEN + $auxcode_area_len + $mcu_fw_area_len ]
dd if=$uboot_file of=output/boot_area.bin bs=1 skip=$boot_check_area_pos count=$boot_check_area_len conv=notrunc
## make boot area signature
openssl dgst -sha256 -sign rsa4096pem/rsa_priv_4096.pem -sigopt rsa_padding_mode:pss -sigopt rsa_pss_saltlen:-1 -out output/boot_sig.bin output/boot_area.bin
## encrypt boot area
if [ $BOOT_IV ]; then
echo "---------------Boot area encrypt enable---------------"
## IV and KEY have set,
## Obtain a new KEY by decrypting the ECB mode.
echo 0x50db86c592c52f0c436cca6f2ffecaf5 | xxd -r > seed_1.bin
echo 0x4a96ae013fc60e205e9da4c9d5ad9b99 | xxd -r > seed_2.bin
openssl enc -nopad -d -nosalt -aes-256-ecb -K "$ROOT_KEY0"  -in seed_1.bin -out out_1.bin
openssl enc -nopad -d -nosalt -aes-256-ecb -K "$ROOT_KEY0"  -in seed_2.bin -out out_2.bin
cat out_2.bin >> out_1.bin
KEY_ecb=$(xxd -ps out_1.bin | sed 'N;s/\n//g')
rm out_*.bin seed_*.bin
echo Obtain a new KEY by decrypting the ECB mode!
echo new_KEY = $KEY_ecb
echo ""
## Use the new KEY and IV to encrypt the image in CBC mode.
openssl enc -aes-256-cbc -nopad -K "$KEY_ecb"  -iv "$BOOT_IV" -in output/boot_area.bin -out output/boot_area_enc.bin
mv output/boot_area_enc.bin output/boot_area.bin
fi

## 6. lbist test pattern
val1=$(hexdump -e '16/4 "%04x"' -n 4 -s 8260 output/params.bin)
lbist_test_pattern_len=$(printf "%d\n" 0x$val1)
echo lbist_test_pattern_len=$lbist_test_pattern_len
lbist_test_pattern_pos=$[ $HEAD_AREA_LEN + $auxcode_area_len + $mcu_fw_area_len + $boot_check_area_len + 0x200 ]
echo lbist_test_pattern_pos=$lbist_test_pattern_pos
dd if=$uboot_file of=output/lbist_test_pattern.bin bs=1 skip=$lbist_test_pattern_pos count=$lbist_test_pattern_len conv=notrunc

# 7. combine to Finalboot.bin
cat output/rsa_pub_4096.bin > output/Finalboot.bin
cat output/params.bin >> output/Finalboot.bin
cat output/params_sig.bin >> output/Finalboot.bin
cat output/unchecked_area.bin >> output/Finalboot.bin
cat output/auxcode_check.bin >> output/Finalboot.bin
cat output/mcu_fw_area.bin >> output/Finalboot.bin
cat output/boot_area.bin >> output/Finalboot.bin
cat output/boot_sig.bin >> output/Finalboot.bin
cat output/lbist_test_pattern.bin >> output/Finalboot.bin

cp output/Finalboot.bin output/${uboot_sig_file}

HASH_OTP_TABLE rsa4096pem/rsa_pub_4096_sha256.txt rsa4096pem/rsa4096_pem_hash_val.txt
cat rsa4096pem/rsa4096_pem_hash_val.txt

if [ $ROOT_KEY0 ]; then
AES_OTP_TABLE $ROOT_KEY0 aes_otp_cfg.txt
echo  create aes_otp_cfg.txt over!
fi

echo "....................end........................."
fi
exit

