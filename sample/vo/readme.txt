************************************************************************************************************
7206V11A_EVB_VA版型
1、
--------PLCD硬件改版(原始单板未做修改的情况下的改动如下，如已做修改，相关电阻可能会有区别)：
去掉：R250,R251,R281,R288,R291,R322,R301,R329；R84
补焊：R249,R253,R283,R285,R312,R315,R324,R326；R83
--------点PLCD gpio复用：
xmmm 0x100c0010 0x9 \n
xmmm 0x100c0014 0x9 \n
xmmm 0x100c0018 0x9 \n
xmmm 0x100c001c 0x9 \n
xmmm 0x100c0020 0x9 \n 
xmmm 0x100c0024 0x9 \n
xmmm 0x100c0028 0x9 \n
xmmm 0x100c002c 0x9 \n
xmmm 0x100c0030 0x9 \n
xmmm 0x100c0034 0x9 \n
xmmm 0x100c0038 0x9 \n
xmmm 0x100c003c 0x9 \n
xmmm 0x100c0040 0x9 \n
xmmm 0x100c0044 0x9 \n
xmmm 0x100c0048 0x9 \n
xmmm 0x100c004c 0x9 \n
xmmm 0x100c0050 0x9 \n
xmmm 0x100c0054 0x9 \n
xmmm 0x100c0058 0x9 \n
xmmm 0x100c005c 0x9 \n
xmmm 0x100c0060 0x9 \n 
xmmm 0x100c0064 0x9 \n 
xmmm 0x100c0068 0x9 \n
xmmm 0x100c006c 0x9 \n
xmmm 0x100c0070 0x9 \n
xmmm 0x100c0074 0x9 \n
xmmm 0x100c0078 0x9 \n
xmmm 0x100c007c 0x9 \n 
./sample_vo 0 1 0 0

2、
--------bt1120/bt656硬件改版(原始单板未做修改的情况下的改动如下，如已做修改，相关电阻可能会有区别)：
拆掉：R387,R394,R398,R402,R406,R410,R414,R418,R422,R426,R430,R434,R438,R442,R446,R450,R454; R301,R329,R325,R327,R322,R318
补焊：R390,R397,R401,R405,R409,R413,R417,R421,R425,R429,R433,R437,R441,R445,R449,R453,R457; R324,R326,R323,R328,R315,R319
------- 点bt1120/bt656
bt1120 gpio复用
xmmm 0x100c0020 0xb  \n
xmmm 0x100c0024 0xb \n
xmmm 0x100c0028 0xb \n
xmmm 0x100c002c 0xb \n
xmmm 0x100c0030 0xb \n
xmmm 0x100c0034 0xb \n
xmmm 0x100c0038 0xb \n
xmmm 0x100c003c 0xb  \n
xmmm 0x100c0040 0xb \n
xmmm 0x100c0044 0xb \n
xmmm 0x100c0048 0xb \n
xmmm 0x100c004c 0xb  \n
xmmm 0x100c0050 0xb \n
xmmm 0x100c0054 0xb \n
xmmm 0x100c0058 0xb  \n
xmmm 0x100c005c 0xb  \n
xmmm 0x100c0018 0xb  \n
./sample_vo 0 1 16 0
./sample_vo 0 1 5 0

3、
--------mcu 18bit硬件改版(原始单板未做修改的情况下的改动如下，如已做修改，相关电阻可能会有区别)：
拆掉：R250,R251,R380,R384,R388,R392,R395,R399,R403,R407,R411,R415,R419,R423,R427,R431,R435,R439,R443,R447,R451,R455,R458,R460,R466；
补焊：R249,R253,R381,R385,R389,R393,R396,R400,R404,R408,R412,R416,R420,R424,R428,R432,R436,R440,R444,R448,R452,R456,R459,R461,R467
--------点mcu 18bit并行屏（mcu屏点屏需要修改dtsi节点，具体可参考“屏幕对接 使用指南.word"，”mcu屏“相关章节）
#mcu屏gpio复用
xmmm 0x100c0010 0x8 \n
xmmm 0x100c0014 0x8 \n
xmmm 0x100c0018 0x8 \n
xmmm 0x100c001c 0x8 \n
xmmm 0x100c0020 0x8 \n
xmmm 0x100c0024 0x8 \n
xmmm 0x100c0028 0x8 \n
xmmm 0x100c002c 0x8 \n
xmmm 0x100c0030 0x8 \n
xmmm 0x100c0034 0x8 \n
xmmm 0x100c0038 0x8 \n
xmmm 0x100c003c 0x8 \n
xmmm 0x100c0040 0x8 \n
xmmm 0x100c0044 0x8 \n
xmmm 0x100c0048 0x8 \n
xmmm 0x100c004c 0x8 \n
xmmm 0x100c0050 0x8 \n
xmmm 0x100c0054 0x8 \n
xmmm 0x100c0058 0x8 \n
xmmm 0x100c005c 0x8 \n
xmmm 0x100c0060 0x8 \n
xmmm 0x100c0064 0x8 \n
#GPIO3_5 复用以及复位mcu屏
xmmm 0x100c0000  0x0 \n
xmmm 0x120B3400  0xff \n
xmmm 0x120B33fc  0xff   \n
sleep 0.001  \n
xmmm 0x120B33fc  0x00  \n
sleep 0.01 \n
xmmm 0x120B33fc  0xff  \n
sleep 0.12 \n
./sample_vo 0 1 14 0
4、
--------mcu spi串行屏硬件改版(原始单板未做修改的情况下的改动如下，如已做修改，相关电阻可能会有区别)：
去掉：R403,R407,R411,R415；466
补焊：R462,R584,R463,R464,R465,R585；467
--------点mcu spi串行屏（mcu屏点屏需要修改dtsi节点，具体可参考“屏幕对接 使用指南.word"，”mcu屏“相关章节）
#mcu spi屏gpio复用
xmmm 0x100c0028 0x5 \n
xmmm 0x100c002c 0x5 \n
xmmm 0x100c0030 0x5 \n
xmmm 0x100c0034 0x5 \n
#GPIO3_5 复用以及复位mcu屏
xmmm 0x100c0000  0x0 \n
xmmm 0x120B3400  0xff \n
xmmm 0x120B33fc  0xff   \n
sleep 0.001  \n
xmmm 0x120B33fc  0x00  \n
sleep 0.01 \n
xmmm 0x120B33fc  0xff  \n
sleep 0.12 \n
./sample_vo 0 1 12 0
************************************************************************************************************


************************************************************************************************************
7206V10_RB_A_VA版型
--------此版型只能点4line mcu屏
--------硬件改动：硬件设计没有接屏复位引脚，需找飞线， “j42座子，29pin”（屏复位引脚） ----- R148 电阻 (即gpio3_5)
--------点mcu spi串行屏（mcu屏点屏需要修改dtsi节点，具体可参考“屏幕对接 使用指南.word"，”mcu屏“相关章节）

#mcu 屏 spi引脚复用配置
xmmm 0x100c0010 0x4 \n
xmmm 0x100c0014 0x4 \n
xmmm 0x100c0028 0x5 \n
xmmm 0x100c002c 0x5 \n

#GPIO3_5 复用以及复位mcu屏 （如硬件飞线不是GPIO3_5  此处复位命令也需要修改）
xmmm 0x100c0000  0x0 \n
xmmm 0x120B3400  0xff \n
xmmm 0x120B33fc  0xff   \n
sleep 0.001  \n
xmmm 0x120B33fc  0x00  \n
sleep 0.01 \n
xmmm 0x120B33fc  0xff  \n
sleep 0.12 \n

./sample_vo 0 1 12 0
*************************************************************************************************************

************************************************************************************************************
7206V11A_RB_A_VA版型
--------此版型只能点PLCD 800x480屏

--------点PLCD gpio复用：
xmmm 0x100c0010 0x9 \n
xmmm 0x100c0014 0x9 \n
xmmm 0x100c0018 0x9 \n
xmmm 0x100c001c 0x9 \n
xmmm 0x100c0020 0x9 \n
xmmm 0x100c0024 0x9 \n
xmmm 0x100c0028 0x9 \n
xmmm 0x100c002c 0x9 \n
xmmm 0x100c0030 0x9 \n
xmmm 0x100c0034 0x9 \n
xmmm 0x100c0038 0x9 \n
xmmm 0x100c003c 0x9 \n
xmmm 0x100c0040 0x9 \n
xmmm 0x100c0044 0x9 \n
xmmm 0x100c0048 0x9 \n
xmmm 0x100c004c 0x9 \n
xmmm 0x100c0050 0x9 \n
xmmm 0x100c0054 0x9 \n
xmmm 0x100c0058 0x9 \n
xmmm 0x100c005c 0x9 \n
xmmm 0x100c0060 0x9 \n
xmmm 0x100c0064 0x9 \n
xmmm 0x100c0068 0x9 \n
xmmm 0x100c006c 0x9 \n
xmmm 0x100c0070 0x9 \n
xmmm 0x100c0074 0x9 \n
xmmm 0x100c0078 0x9 \n
xmmm 0x100c007c 0x9 \n

./sample_vo 0 1 0 0
*************************************************************************************************************

************************************************************************************************************
7206V10B_RB_A_VA版型
--------此版型只能点SLCD 240x320
--------硬件改动: 1、取掉 R426 R427 R379 R277 R278
                  2、补焊 R428 R429 R382
                  3、飞线 SLCD_SCK->J51 SLCD_SODI->J52 SLCD_CS->J54
#SPI时钟打开  注意：此处将spi0 spi1 spi2时钟都开启，时钟选择396N，实际应依场景配置
xmmm 0x120101BC 0x0f000 \n

# 复位引脚GPIO5_5复用 及 复位
xmmm 0x100c0040 0x0 \n
xmmm 0x120B5400 0xff \n
xmmm 0x120B53fc 0xff \n
xmmm 0x120B53fc 0x0 \n
xmmm 0x120B53fc 0xff \n

# SPI0 管脚复用
xmmm 0x120f0018 0x100 \n
xmmm 0x12044000 0x3  \n
xmmm 0x120f0028 0x100 \n
xmmm 0x12044004 0x3  \n
xmmm 0x120f0038 0x100 \n
xmmm 0x1204400c 0x3 \n

# SLCD管脚复用  此处复用只配置了data0-data5  如果是8bit屏则需要配置data0-data7
xmmm 0x100c0018 0x9 \n
xmmm 0x100c001c 0x9 \n
xmmm 0x100c0020 0x9 \n
xmmm 0x100c0024 0x9 \n
xmmm 0x100c0028 0x9 \n
xmmm 0x100c002c 0x9 \n
xmmm 0x100c0030 0x9 \n
xmmm 0x100c0034 0x9 \n
xmmm 0x100c0038 0x6 \n
xmmm 0x100c003c 0x6 \n

# 加载spi st7789ko 配置屏参
insmod xm_ssp_st7789_ex.ko \n

./sample_vo 0 1 8 0
************************************************************************************************************