# SHT20 温湿度 + SPI 屏显示

板端读 SHT20（或 SI7021/HTU21，命令兼容）的温湿度，大字显示在 ST7789 SPI 屏上。

## 硬件接线

| 信号 | 板上 GPIO | 控制器 / 节点 | 说明 |
|---|---|---|---|
| SCL | GPIO4_1 | I2C3 → `/dev/i2c-3` | 与 OLED(0x3C)/MPU6050(0x68) 共享总线 |
| SDA | GPIO4_2 | I2C3 → `/dev/i2c-3` | 共享总线 |
| VCC | 3.3V | — | SHT20 供电 2.1~3.6V |
| GND | GND | — | — |

- 主控：GK7602V11A（SDK 内部 `xm7206v11a`，架构 `xmorca`）
- 芯片：SHT20 / SI7021 / HTU21，I2C 从地址 **0x40**（这三种命令兼容）。

## 引脚复用结论（已核实，无需改 dts / 重编内核）

GPIO4_1/GPIO4_2 复用为 I2C3，`main.c` 的 `padmux_init()` 写 func2(`0x1002`)（同 oled_display / mpu6050_display）。`&i2c_bus3` `status="ok"`，`/dev/i2c-3` 开箱即用。

## 测量原理（no-hold-master）

SHT20 测量分两步，温度/湿度各一次：
1. 发命令 `0xF3`（温度）/ `0xF5`（湿度）触发转换。
2. 等 ~85ms（12~14bit 转换时间）后**直接读 3 字节**（MSB, LSB, CRC）——读阶段不带寄存器号（`i2c_hal_read_noreg`）。

不用 hold-master（`0xE3/0xE5`，依赖 I2C clock-stretching，lotus i2c-lotus 是否支持未知），no-hold 最稳。

换算（datasheet 公式，raw 低 2 位是状态位已清）：
- 温度：`T(℃) = -46.85 + 175.72 × raw / 65536`
- 湿度：`RH(%) = -6 + 125 × raw / 65536`（边界裁剪到 0~100）

## 编译

```bash
export PATH=$PATH:<SDK>/tools/linux/toolchains/arm-gcc12.2.0-linux-uclibceabi/bin
cd source/app/sht20_display
make            # 产物 ./sht20_display
```

## 部署与运行

```bash
scp sht20_display root@<板子IP>:/usr/bin/
ssh root@<板子IP> '/usr/bin/sht20_display'
```

```bash
sht20_display     # 每 2s 读温湿度刷新 SPI 屏，Ctrl+C 清屏退出
sht20_display -h  # 帮助
```

屏幕：标题 / I2C3 / **TEMP（大字 ℃）** / **HUMI（大字 %RH）**。串口同步打印 `T=.. C RH=.. %`。

## 排错

1. **屏显示 `--.-` 或启动 `SHT20 FAIL`**：I2C 没通或没设备。
   - 确认 SCL 接 GPIO4_1、SDA 接 GPIO4_2（**别接反**）、VCC 3.3V。
   - 确认地址是 0x40（SHT20/SI7021/HTU21；若芯片是 AHT20=0x38、SHT3x=0x44 则命令不同，本驱动不适用）。
   - 板端 `i2c_read 3 0x40 0x00 1 1`（或 `i2cdetect`）确认 0x40 有应答。
2. **温湿度读数恒定不变/异常**：CRC 未校验（本驱动简化），个别坏帧可能跳变一次，下次刷新即恢复；可加大采样间隔或加 CRC。
3. **湿度恒 0 或 100**：探头结露/极干，或接线接触不良。
4. **屏不亮**：参考 `source/app/spi_display` 排错。

## 文件说明

| 文件 | 作用 |
|---|---|
| `sht20.c/h` | SHT20 驱动：`sht20_init`（open+软复位）/ `sht20_read`（0xF3/0xF5 no-hold + 85ms + 读 3B + 换算） |
| `i2c_hal.c/h` | `/dev/i2c-X` + `I2C_RDWR` 封装（含 `i2c_hal_read_noreg` 纯读），复制自 mpu6050_display 并扩展 |
| `spi_hal.c/h` | SPI 屏传输层（24MHz），复制自 spi_display |
| `st7789.c/h` | ST7789 驱动（RGB565 帧缓冲 + 绘图 API），复制自 spi_display |
| `font8x16.h` | 8×16 ASCII 字模（main 用它做 3 倍放大大字显示） |
| `main.c` | I2C3 pad 复用 + SPI屏/SHT20 初始化 + 主循环采集显示 |
