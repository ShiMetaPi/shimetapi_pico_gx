1. 确认sensor类型
-- mcu 修改
 a. 修改sdk/mcu/media/sensors/Makefile.riscv，保留需要用到的sensor
 b. 修改sdk/mcu/media/include/media_sample.h  仅放开需要用到的sensor
 c. 修改sdk/mcu/riscv/configs/xmorca_defconfig 放开CONFIG_MEDIA_SAMPLE_QUICKSTART 支持
-- sample 修改
 a. 修改sdk/sample/quickstart/quickstart_base.mk 支持对应sensor

2. 完成以上修改后，将该文件夹放在sdk/目录下
3. 请根据具体的版型型号更新quickstart_build.sh中相应路径和命名
4. 进入quickstart_build，source quickstart_build.sh
5. 该脚本仅作参考使用，相关启动异常或编译异常请参考<<快启使用指南>> 
