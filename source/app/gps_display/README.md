# GPS 模块（UART1 NMEA）+ SPI 屏显示

板端通过 UART1 读 GPS 模块的 NMEA-0183 语句，解析定位参数实时显示在 ST7789 SPI 屏上。

## 硬件接线

| 信号 | 板上 GPIO | iocfg pad | func | 设备节点 |
|---|---|---|---|---|
| GPS TX → 主控 RX | GPIO3_6 | `0x100C0004` | func3(UART1_RXD)=`0x1003` | `/dev/ttyAMA1` |
| GPS RX ← 主控 TX | GPIO3_5 | `0x100C0000` | func3(UART1_TXD)=`0x1003` | `/dev/ttyAMA1` |
| PPS（可选） | GPIO4_6 | `0x100C0024` | func5(GPIO)=`0x1005`（默认 JTAG！）| `/dev/gpiochip4` line6 |
| 显示 | SPI2 | （复用 spi_display）| — | `/dev/spidev2.0` |
| VCC / GND | 3.3V / GND | — | — | — |

- 主控：GK7602V11A（SDK 内部 `xm7206v11a`，架构 `xmorca`）
- UART1 = PL011 @0x12041000，`status="ok"`，节点 `/dev/ttyAMA1`（uart0/ttyAMA0 是控制台，空闲）。
- GPS 默认 **9600 8N1**。PPS 是 1Hz 秒脉冲，可选；GPIO4_6 默认是 JTAG_TRSTN，必须写 func5 才归 GPIO。

> 接法：GPS 模块的 **TX 脚接 GPIO3_6**（主控 UART1_RXD 收 NMEA），**RX 脚接 GPIO3_5**（主控 UART1_TXD，可发配置命令，本驱动不发）。

## 引脚复用结论（已核实，无需改 dts / 重编内核）

`main.c` 的 `padmux_init()` 经 `/dev/mem` mmap 写三处 iocfg：
- GPIO3_5/GPIO3_6 → func3（UART1），查 PIN_OUT 表3。
- GPIO4_6 → func5（GPIO，默认 JTAG_TRSTN，同 DC/RES 一样要先解 JTAG）。
- SPI 屏的 pad 由 `spi_hal_init()` 内部切。

设备树 `&uart1` `status="ok"`，`/dev/ttyAMA1` 开箱即用。

## NMEA 解析

兼容 talker 前缀 `$GP`（GPS）/ `$GN`（GNSS，国产 ATGM336H 等多模模块）/ `$BD`（北斗），按句子后缀匹配：
- **$xxGGA** → 时间、纬度、经度、定位质量（>0 即定位）、卫星数、海拔。
- **$xxRMC** → 时间、状态（A=定位 / V=警告）、纬度、经度、速度（节）、日期。

未定位时（冷启动）GGA 质量=0、RMC 状态=V、经纬度为空，屏幕显示 `--`、`FIX NO FIX`。纬经度保留 NMEA 原始格式（`ddmm.mmmm N/S`）。不做异或校验和（数据一般可靠，需要时可后加）。

## 编译

```bash
export PATH=$PATH:<SDK>/tools/linux/toolchains/arm-gcc12.2.0-linux-uclibceabi/bin
cd source/app/gps_display
make            # 产物 ./gps_display
```

## 部署与运行

```bash
scp gps_display root@<板子IP>:/usr/bin/
ssh root@<板子IP> '/usr/bin/gps_display'
```

```bash
gps_display     # 持续读 NMEA，每 500ms 刷新 SPI 屏，Ctrl+C 清屏退出
gps_display -h  # 帮助
```

屏幕显示：标题 / 卫星数 / TIME / DATE / LAT / LON / ALT / SPD / FIX（VALID 绿、NO FIX 红）/ PPS 电平。

## 排错

1. **一直 NO FIX**：GPS 冷启动需到窗户/室外，室内/楼下收不到卫星属正常。检查天线接好。冷启动首次定位可能 30 秒~几分钟。
2. **一直 SAT:00、TIME 空（NO DATA）**：UART 没通。
   - 确认 GPS-TX 接到了 GPIO3_6、GPS-RX 接 GPIO3_5（**TX/RX 别接反**）。
   - 确认 pad 已切 func3（看启动日志 `pad 0x100C0004 -> 0x00001003`）。
   - 个别 GPS 模块默认波特率 4800/38400，改 `uart_hal.c` 的 `B9600` 重编。
   - 板端直接 `cat /dev/ttyAMA1`（先停程序）确认能看到 `$G...` 句子。
3. **VCC/GND**：GPS 模块多数 3.3V 供电，确认电源。
4. **屏不亮**：参考 `source/app/spi_display` 排错。

## 文件说明

| 文件 | 作用 |
|---|---|
| `uart_hal.c/h` | `/dev/ttyAMA1` + termios 9600 8N1 raw + `uart_read_line` 按行读 NMEA |
| `nmea.c/h` | NMEA 解析：$xxGGA/$xxRMC → `nmea_data_t`（状态/时间/经纬度/卫星数/速度/海拔/日期） |
| `gpio_hal.c/h` | GPIO chardev 封装（`gpio_get_value` 读 PPS），复制自 sonic_display |
| `spi_hal.c/h` | SPI 屏传输层（24MHz 固定速度），复制自 spi_display |
| `st7789.c/h` | ST7789 驱动（RGB565 帧缓冲 + 绘图 API），复制自 spi_display |
| `font8x16.h` | 8×16 ASCII 字模 |
| `main.c` | pad 复用 + SPI屏/UART/PPS 初始化 + 主循环读 NMEA 解析 + 节流刷屏 |
