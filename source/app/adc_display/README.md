# LSADC GPIO1_1 实时电压采样 + SPI 屏显示

板端读 LSADC 通道 1（物理 pad GPIO1_1），ST7789 SPI 屏大字显示当前电压。

## 硬件接线

| 信号 | 板上 GPIO | 物理 pad | 控制器 / 节点 | 说明 |
|---|---|---|---|---|
| **AO** | GPIO1_1 | iocfg_reg21 @ 0x12090004 | `/dev/lsadc` channel 1 | SARADC 12-bit |
| VCC | 1.8V | — | — | **必须 ≤ 3.3V**；典型传感器输出 0~VCC |
| GND | GND | — | — | |

- 主控：GK7602V11A（SDK 内部 `xm7206v11a`，架构 `xmorca`）
- ADC：SoC 内置 LSADC（**12-bit**，3 通道），不需要外挂芯片

## 为什么 GPIO1_1 是 CH1

查 7206V11A PIN_OUT 表3 iocfg_reg21：
```
func0 = GPIO1_1       （默认）
func4 = LSADC_CH1     ← 我们用这个
```

`main.c:padmux_init()` 写 `0x12090004 → 0x00001004`。

## 数据换算

```
10-bit ADC 值：0..1023
电压：raw * VREF / 1023
VREF：默认 1.8V（adc.h 里 ADC_VREF_MV = 1800）
```

如果你的 VREF 实际是 3.3V，改 `adc.h`：
```c
#define ADC_VREF_MV  3300
```

## 编译

```bash
export PATH=$PATH:<SDK>/tools/linux/toolchains/arm-gcc12.2.0-linux-uclibceabi/bin
cd source/app/adc_display
make
ls -l adc_display
```

## 部署与运行

```bash
scp adc_display root@<板子IP>:/usr/bin/
ssh root@<板子IP> '/usr/bin/adc_display'
```

**首次跑**会自动 insmod `xm_lsadc.ko`（如果 `/opt/ko/load` 启动脚本没包含）。

预期日志：
```
[adc] pad 复用：LSADC_CH1 pad (GPIO1_1) -> func4
[adc] pad 0x12090004 -> 0x00001004
[adc] 初始化 SPI 屏...
...
[adc] 初始化 ADC（CH1 GPIO1_1）...
[adc] @ CH1 VREF=1800mV (/dev/lsadc CONTINUOUS 模式)
[adc] 采集中，每 100ms 刷新，Ctrl+C 退出。
[adc] raw= 523/1023  V=0.920
[adc] raw= 524/1023  V=0.922
```

## 屏上显示

- 标题：`LSADC`
- 副标题：`GPIO1_1 CH1`
- 大字（4 倍）：`X.XXX V`（毫伏精度）
- 底部：`raw=XXX/1023`（原始值）+ `Ctrl+C quit`

## 排错

1. **`open /dev/lsadc: No such file or directory`**：
   - 程序会**自动 insmod /opt/ko/xm_lsadc.ko**，等 300ms 再试
   - 如果还失败：`ls /opt/ko/xm_lsadc.ko` 看 ko 是否存在
2. **`raw 一直是 0 或 1023`**：
   - 检查 AO 引脚电压（应该 0~VCC 之间）
   - 检查 VCC 引脚（不能浮空）
3. **电压值不准**：
   - VREF 假设错误：实测 AO = 1.0V 但 raw = 500（VREF 应该是 2.0V）
   - 在 `adc.h` 改 `ADC_VREF_MV`
4. **屏显示 ERR**：ADC 读失败，看串口错误信息。

## 想换通道？

`adc.h` 顶部改 `ADC_CHN`：
- `1` = GPIO1_1（默认，本程序用的）
- `2` = GPIO1_2 / LSADC_CH2 pad 0x12090008
- `0` = GPIO1_0 / LSADC_CH0 pad 0x12090000（驱动没开 CH0，要用得绕过驱动）

然后 `main.c` 把 `LSADC_CH1_PAD` / `LSADC_CH1_FUN` 改成对应 pad 和 func4。

## 文件说明

| 文件 | 作用 |
|---|---|
| `adc.c/h` | LSADC 驱动：`adc_init`（开 /dev/lsadc + 自动 insmod + MODEL_SEL/CONTINUOUS/CHN_ENABLE/START）/ `adc_read`（GET_CHNVAL + 换算电压） |
| `main.c` | pad mux（仅 LSADC_CH1 pad）+ SPI 屏 + 主循环采集显示 |
| `spi_hal.c/h` `st7789.c/h` `gpio_hal.c/h` `font8x16.h` | 复用自 spi_display / sht20_display |

## LSADC ioctl 数值（硬编码）

uClibc 的 `<sys/ioctl.h>` 没暴露 `_IO`/`_IOW`/`_IOWR` 宏，直接硬编码：

| 操作 | 数值 |
|---|---|
| MODEL_SEL | `0xC0004C00` |
| CHN_ENABLE | `0x40044C01` |
| CHN_DISABLE | `0x40044C02` |
| START | `0x00004C03` |
| STOP | `0x00004C04` |
| GET_CHNVAL | `0xC0044C05` |

完整定义参考 `source/gmp/drv/lsadc/adc.h`。