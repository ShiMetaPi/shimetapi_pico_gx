# MAX30100 血氧/心率 + SPI 屏显示

板端读 MAX30100 模块的 FIFO，提取 IR/R 原始值 + 滑窗峰值检测算心率（BPM），屏上显示。IRD/RD 悬空（用模块内部 LED）。

## 硬件接线

| 信号 | 板上 GPIO | 物理 pad | 控制器 / 节点 | 说明 |
|---|---|---|---|---|
| **SCL** | GPIO4_1 | iocfg_reg48 @ 0x100C0010 | I2C3 → `/dev/i2c-3` | 与 OLED(0x3C)/MPU6050(0x68)/SHT20(0x40) 共享 |
| **SDA** | GPIO4_2 | iocfg_reg49 @ 0x100C0014 | I2C3 → `/dev/i2c-3` | 同上 |
| **INT** | GPIO4_4 | iocfg_reg51 @ 0x100C001C | `/dev/gpiochip4` line 4 | 默认 JTAG_TDI，要写 func5(0x1005) 切回 GPIO |
| **IRD** | 悬空 | — | — | 用 sensor 内部 LED |
| **RD**  | 悬空 | — | — | 同上 |
| VCC | 3.3V | — | — | MAX30100 工作电压 1.8~3.3V |
| GND | GND | — | — | |

- 主控：GK7602V11A（SDK 内部 `xm7206v11a`，架构 `xmorca`）
- 模块：MAX30100（Maxim Integrated），I2C 从地址 **0x57**（部分板用 0x5A）

## pad 复用结论（已查 PIN_OUT 表3）

```
I2C3_SCL   iocfg_reg48 @ 0x100C0010  func2 -> 0x1002
I2C3_SDA   iocfg_reg49 @ 0x100C0014  func2 -> 0x1002
GPIO4_4    iocfg_reg51 @ 0x100C001C  func5 -> 0x1005（默认 JTAG_TDI，写 0x1005 切回 GPIO）
```

`main.c:padmux_init()` 在 init 前调用三次 `set_pad_func`。SPI 屏的 pad 由 `spi_hal_init()` 内部切。

## 工作原理

1. **init**：reset → 读 PART_ID(0xFF)/REV_ID(0xFE) 认芯片 → 配 SPO2_CFG/LED_CONFIG/MODE → 验证 MODE 读回 → 申请 GPIO4_4 输入
2. **采样线程**（100ms 一次）：清 INT_STATUS → 读 WR/OVF/RD 三字节 → 算出未读 sample 数 → burst 读 FIFO_DATA → push 到 ring
3. **主线程**（200ms 一次）：从 ring 弹出所有 sample 灌进 hist（满则滚窗）→ 算 BPM（用最近 10 秒 hist）→ 算 SpO2（用最近 1 秒 R/IR 均值）→ 刷屏
4. **HR 估算**：取最近 2 秒 IR 原始值算 amp/baseline，阈值 `baseline + amp * 30%`，峰间最小 300ms 去抖后数上升沿 → 算 BPM（30..220 范围）
5. **SpO2 估算**：最近 1 秒 R/IR 均值的简单比值套 `SpO2 = 110 - 30 * ratio`，仅作演示，**不是医学级**

## 配置寄存器（代码实写）

```
MODE_CONFIG(0x06) = 0x03   SpO2 模式（HR + SpO2）
SPO2_CONFIG(0x07) = 0x43   HI_RES_EN=1 | SR=50Hz | LED_PW=1600us → 16-bit ADC
LED_CONFIG(0x09)  = 0x99   [7:4]RED_PA=0x9 | [3:0]IR_PA=0x9 → 各 30.6mA（单寄存器双 nibble）
```

> **FIFO_CONFIG(0x08) 不写**，保留 reset 后的默认值（FIFO_ROLLOVER=0，FIFO 满就停）。
>
> **LED_PA 的取值**：MAX30100 内部 LED 电流按档位（0..15）：
>
> | 值 | 电流 |  | 值 | 电流 |
> |---|---|---|---|---|
> | 0x7 | 24.0mA |  | 0xC | 40.2mA |
> | 0x8 | 27.1mA |  | 0xD | 43.6mA |
> | 0x9 | 30.6mA |  | 0xE | 46.8mA |
> | 0xA | 33.8mA |  | 0xF | 50.0mA |
> | 0xB | 37.0mA |  |   |       |
>
> 调参记录（手贴模块的经验值）：
> - `0x77`（24mA）→ R 只到 30k，脉搏幅度 ~700，太弱
> - `0xBB`（37mA）→ R 频繁打满 65535（饱和），但能看到 ~30k 的脉搏幅度
> - `0x99`（30.6mA）← 当前：期望 R 在 50k~63k 不饱和，脉搏幅度 5k~10k
>
> 若 R 仍打满 → 退到 0x77（24mA）或换 800µs LED_PW 减积分时间。

## 编译

```bash
export PATH=$PATH:<SDK>/tools/linux/toolchains/arm-gcc12.2.0-linux-uclibceabi/bin
cd source/app/spo2_sensor
make
ls -l spo2_sensor
```

## 部署与运行

```bash
scp spo2_sensor root@<板子IP>:/usr/bin/
ssh root@<板子IP> '/usr/bin/spo2_sensor'
```

```bash
spo2_sensor        # 采样 100ms 一次，刷屏 200ms 一次
spo2_sensor -h     # 帮助
```

把手指轻轻按住 MAX30100 表面（轻触，不要太紧），等 2-3 秒让波形稳定，屏上会出 BPM。

## 屏上显示

- 标题：`MAX30100` + `I2C3 0x57 n=<本批 sample 数>`
- 大字（2 倍）：`IR=<原始值>` 白色 / `R=<原始值>` 青色
- 一行：`HR: <BPM 或 -->` + `SpO2: <百分比 或 -->`
- 注：FIFO 拿到 sample 才更新 IR/R；ring 空时画 `--` 而不是 `0`，避免误判没信号

## 排错

1. **屏 `MAX30100 FAIL`**：
   - 量 VCC-GND 是否 3.3V
   - `i2cdetect -y 3` 看 0x57 是否有应答
   - 总线同时跑 OLED/MPU6050/SHT20 时，先把它们停掉排除冲突
2. **IR 一直是 ~0 或 ~16万**：
   - LED 电流档位不对，调 `CFG_LED_CONFIG`（max30100.c 里）
   - 没放手指 / 手指太靠边
3. **BPM 一直是 --**：
   - 至少等 10 秒（hist 攒满 512 个 sample 才算 BPM）
   - 手指没贴紧、信号太弱 → 加大 LED_PA
4. **FIFO overflow**：
   - 屏刷得不够快 / 主循环 sleep 太长；可以缩 usleep 或改用 INT 触发（INT 现在被 SPI 屏的 RES 占了）

## 文件说明

| 文件 | 作用 |
|---|---|
| `max30100.c/h` | MAX30100 驱动：`max30100_init`（reset + 配寄存器 + 申请 GPIO4_4）/ `max30100_read_fifo`（burst 读 FIFO）/ `max30100_est_bpm`（滑窗 amp + 峰间去抖的峰值检测） |
| `main.c` | pad mux + SPI 屏 + 采样线程（100ms）+ 主循环采集显示（200ms） |
| `i2c_hal.c/h` `spi_hal.c/h` `st7789.c/h` `gpio_hal.c/h` `font8x16.h` | 复用自 mpu6050_display / sht20_display |

## 注意事项

- **真做医学产品不能这么写**。本程序只是把 raw 数据 + 简单算法堆出来玩，BPM 误差大，SpO2 完全不准
- MAX30100 比 MAX30102 老，停产了。如果买新模块建议换 MAX30102（兼容 I2C 接口但寄存器地址不一样）
- INT 引脚本程序**只申请了输入，没用中断**——轮询 FIFO 够用，省一层 IRQ 处理（INT 实际还会被 SPI 屏的 RES 占住）
- 本模块 LED 物理极性反了——datasheet 标 IR 的 byte0/1 实际是 RED，标 R 的 byte2/3 实际是 IR。驱动里把两通道软件互换，后续算法按软件通道走