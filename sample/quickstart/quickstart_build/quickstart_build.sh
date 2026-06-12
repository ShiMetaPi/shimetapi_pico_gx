SDK_DIR=$(pwd)/..
INITRAMDISK_DIR=${SDK_DIR}/source/initramdisk/scripts
INITRAMDISK_PARAM_DIR=${SDK_DIR}/tools/linux/utils/bin
CHIP=xm7206v11a
BOARD=rba
AI_MODLE=gnn_person_detect_640x360_rgb888hwc_v0102_20240909.bin

function make_img()
{
	cd ${SDK_DIR}
	cp ${SDK_DIR}/configs/${CHIP}/${CHIP}${BOARD}_linux-5.10_quickstart_cfg.mk ${SDK_DIR}/cfg.mk
	source build/env.sh
	make clean
	make build -j 32
	cd -
}

function make_samlpe()
{
	cd ${SDK_DIR}/sample/quickstart
	make clean
	make -j32
	cd -
}

function update_initramdisk()
{
	cd ${INITRAMDISK_DIR}
	tar -zxvf initramdisk.tgz
	cp ${SDK_DIR}/sample/quickstart/quickstart_ramfs/sample_quickstart ${INITRAMDISK_DIR}/initramdisk/root
	arm-gcc12.2.0-linux-uclibceabi-strip ${INITRAMDISK_DIR}/initramdisk/root/sample_quickstart
	rm -rf ${INITRAMDISK_DIR}/initramdisk/param
	cd -
}

function update_ai_modle()
{
	cd ${INITRAMDISK_DIR}/initramdisk
	mkdir param
	cd -
	cp ${SDK_DIR}/sample/npu/demo_ai/model/${AI_MODLE} ${INITRAMDISK_DIR}/initramdisk/param/${AI_MODLE}
}

function re_compress_initramdisk()
{
	cd ${INITRAMDISK_DIR}
	rm -rf initramdisk.tgz
	tar -zcvf initramdisk.tgz initramdisk/
	rm -rf initramdisk/
	cd -
}

function update_kernel_param()
{
	cp ${SDK_DIR}/sample/quickstart/quickstart_build/src/fakeroot-scripts-initramdisk ${INITRAMDISK_PARAM_DIR}/
}

function update_kernel_img()
{
	cd ${SDK_DIR}
	make initrd_clean
	make linux -j32
	cd -
}

function update_rootfs()
{
	cd ${SDK_DIR}
	cp ${SDK_DIR}/sample/quickstart/quickstart_rootfs/quickstart_rootfs ${SDK_DIR}/out/${CHIP}/rootfs/usr/bin
	make jffs2_clean
	make jffs2
	cd -
}

make_img
make_samlpe
update_initramdisk
update_ai_modle
re_compress_initramdisk
update_kernel_param
update_kernel_img
update_rootfs
