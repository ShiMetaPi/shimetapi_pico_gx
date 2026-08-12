# MQ2 气体传感器 + SPI 屏显示

板端读 MQ2 模块的 DO（数字）+ AO（模拟），在 ST7789 SPI 屏上大字显示浓度百分比 + 报警状态。

## 硬件接线（最终版，AO 走 GPIO1_0 / LSADC_CH0）

| 信号 | 板上 GPIO | 物理 pad | 控制器 / 节点 | 说明 |
|---|---|---|---|---|
| **DO** | GPIO0_1 | iocfg_reg1 @ 0x112C0004 | `/dev/gpiochip0` line 1 | chardev 输入读电平 |
| **AO** | GPIO1_0 / LSADC_CH0 | iocfg_reg20 @ 0x12090000 | `/dev/mem` mmap 0x120a0000 | 直读 LSADC 寄存器（绕过驱动） |
| VCC | 5V | — | — | MQ2 heater 要 5V（板上 LDO 出来的 3.3V 不够） |
| GND | GND | — | — | |

- 主控：GK7602V11A（SDK 内部 `xm7206v11a`，架构 `xmorca`）
- 模块：MQ2（板载 LM393 比较器 + 电位器调阈值）

## 为什么 AO 不走 /dev/lsadc 而是直接 mmap 寄存器

`source/gmp/drv/lsadc/adc.c:16` 里：

```c
//#define USE_LSADC_CHANNEL_0
```

驱动 `USE_LSADC_CHANNEL_0` 是注释掉的。所以：

```c
// adc.c:lsadc_chn_valid()
switch (chn) {
#ifdef USE_LSADC_CHANNEL_0
    case 0:
        lsadc_reg_write(value << 8, 1 << 8, LSADC_CONFIG);
        break;
#endif
    case 1: ... case 2: ...
    default:
        lsadc_print("error chn:%d\n", chn);
        return -1;
}
```

**`ioctl(CHN_ENABLE, 0)` 走 default 直接返回 -1**——驱动层不让用户开 CH0。但**硬件本身支持 CH0**：CONFIG bit 8 = CH0 使能，CHNDATA 偏移 `0x2C + (0 << 2) = 0x2C`。

我们的做法：

1. `open("/dev/lsadc")` → 触发 `lsadc_open()` 让驱动帮忙使能 clock + 退出 reset + 使能控制器。
2. **不调任何 ioctl**——避免驱动改 CONFIG。
3. `open("/dev/mem")` + mmap 0x120a0000，自己写寄存器：
   - 写 CONFIG bit8 = 1（使能 CH0）
   - 写 START = 1
   - `usleep(2000)` 等转换
   - 读 CHNDATA + 0x2C
   - 写 STOP = 1
4. `close("/dev/lsadc")` → 触发 `lsadc_release()` 把控制器拉回 reset。

这样驱动既能给时钟，又不挡 CH0。

## pad 复用（已查 PIN_OUT 表3）

```
GPIO0_1     iocfg_reg1  @ 0x112C0004   func0=GPIO（默认 0x1000，DO 不写）
LSADC_CH0   iocfg_reg20 @ 0x12090000   func4=LSADC_CH0 -> 写 0x1004
```

`main.c` 在 mq2_init 之前 `set_pad_func(0x12090000, 0x1004)` 切一次。SPI 屏的 pad 由 `spi_hal_init()` 内部切。

## 显示内容

- 标题：`MQ2 GAS`
- 副标题：`ADC0/GPIO1_0`
- 第一行：`DO  GPIO0_1=<0/1>`
- 第二行：`AO  CH0=<原始值或 ERR>`
- 大字（3 倍）：`<百分比%>` 或 `--`（读失败）
- 报警色块：`ALERT`（红）/ `NORMAL`（绿），阈值默认 70%

阈值在 `main.c:draw_gas()` 的 `TH=70` 改。

DO 电平极性（高=有气/低=有气）取决于 MQ2 模块比较器电路。本程序以 `DO=1` 当作 ALERT，反了的话改 `main.c:draw_gas()` 里的判断。

## 编译

```bash
export PATH=$PATH:<SDK>/tools/linux/toolchains/arm-gcc12.2.0-linux-uclibceabi/bin
cd source/app/gas_sensor
make
ls -l gas_sensor
```

## 部署与运行

```bash
scp gas_sensor root@<板子IP>:/usr/bin/
ssh root@<板子IP> '/usr/bin/gas_sensor'
```

```bash
gas_sensor        # 200ms 刷新
gas_sensor -h     # 帮助
```

## 排错

0. **`open /dev/lsadc: No such file or directory`**：
   - **板端 `/opt/ko/load` 启动脚本里没有 `insmod xm_lsadc.ko`**，所以默认 `/dev/lsadc` 节点不存在。
   - **自动 fallback**：本程序检测到 ENOENT 时会自动 `insmod /opt/ko/xm_lsadc.ko`，等 300ms 后再 open；日志会看到 `自动 insmod xm_lsadc.ko ...`。
   - **永久方案**：在 `/opt/ko/load` 的 `insert_ko()` 函数里加一行 `insmod xm_lsadc.ko`（放在 audio 之前），重启后节点开机就有。
1. **屏显示 `MQ2 FAIL`**：
   - `ls /dev/gpiochip0` 和 `ls /dev/lsadc`：节点不存在说明内核没 probe
   - `dmesg | grep lsadc`：看 SARADC 驱动是否报错
   - 量 MQ2 模块 VCC-GND 是否有 5V
2. **AO 一直 ERR**：
   - 程序会打印 `pad 0x12090000 -> 0x00001004`，没这句说明 pad 没切成 LSADC_CH0
   - `lsmod | grep lsadc`：驱动是否加载（`xm_lsadc`）
   - 第一次 mmap 失败可能是 `/dev/mem` 权限不足
3. **AO 一直 0 或 1023**：
   - 检查 iocfg_reg20 是否真的写到了 0x1004
   - 检查 AO 杜邦线是否接到 GPIO1_0 而不是别的脚
4. **CH1/CH2 资源冲突**：本程序只动 CH0（CONFIG bit8），不碰 bit9/bit10，但如果别的进程也在用 `/dev/lsadc`，会在 CONFIG 上撞车。确保板端没有其他 ADC 用户。

## 文件说明

| 文件 | 作用 |
|---|---|
| `mq2.c/h` | MQ2 驱动：`mq2_init`（开 LSADC 拿时钟 + mmap 寄存器 + chardev 申请 GPIO 输入）/ `mq2_read`（一次 LSADC + GPIO 读） |
| `main.c` | pad mux（仅 LSADC_CH0 pad）+ SPI 屏 + 主循环采集显示 |
| `spi_hal.c/h` `st7789.c/h` `gpio_hal.c/h` `font8x16.h` | 复用自 spi_display / sht20_display |

## LSADC 寄存器速查

基址 `0x120a0000`，偏移：

| 偏移 | 寄存器 | 关键位 |
|---|---|---|
| 0x00 | CONFIG | bit8=CH0 en, bit9=CH1 en, bit10=CH2 en, bit13=mode (0=single,1=cont), bit15=reset |
| 0x08 | TIMESCAN | time_scan > 20 |
| 0x1C | START | 写 1 触发 |
| 0x20 | STOP | 写 1 停止 |
| 0x2C | CH0 data | 10-bit 原始值 |
| 0x30 | CH1 data | 同上 |
| 0x34 | CH2 data | 同上 |

完整定义在 `source/gmp/drv/lsadc/adc.c` 头几行。