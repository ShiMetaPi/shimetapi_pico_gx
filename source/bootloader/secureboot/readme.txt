编译方法：
1. 拷贝非安全uboot到当前目录。

2. 生成普通安全uboot镜像：
   make CHIP=<chip_name>
   chip_name可选值：
                    xm7206v10 xm7206v10b xm7206v11a
   签名成功后生成的文件: output/uboot_<chip_name>_sig.bin

3. 生成加密安全uboot镜像:
   make CHIP=<chip_name> AES_ENC=true
   chip_name可选值:
                    xm7206v10 xm7206v10b xm7206v11a
   签名成功后生成的文件: output/uboot_<chip_name>_sig_enc.bin

4. 清理：make clean 或 make distclean

注意：如果需要用已有的rsa4096pem格式的key，需要将公钥'rsa_priv_4096.pem'和私钥'rsa_pub_4096.pem'拷贝到rsa4096pem目录。
