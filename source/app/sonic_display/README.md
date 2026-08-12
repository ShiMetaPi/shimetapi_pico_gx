# HC-SR04 超声波测距 + SPI 屏显示

板端驱动 HC-SR04 超声波模块测距，实时把距离（厘米）显示在 ST7789 SPI 屏上。

## 硬件接线

| 信号 | 板上 GPIO | 节点 | 说明 |
|---|---|---|---|
| TRIG | GPIO6_7 | `/dev/gpiochip6` line7 | 输出 ≥10us 触发脉冲（3.3V 可直接触发） |
| ECHO | GPIO7_0 | `/dev/gpiochip7` line0 | 输入测脉宽。**HC-SR04 输出 5V，需分压到 3.3V** |
| VCC | 5V | — | HC-SR04 必须 5V 供电 |
| GND | GND | — | — |

> ⚠️ **ECHO 分压**：HC-SR04 的 ECHO 脚输出 5V 高电平，直接接 3.3V 的 GPIO7_0 有风险。
> 建议分压：`ECHO → 10kΩ → GPIO7_0 → 20kΩ → GND`（分得约 3.3V）。若用的是 3.3V 版本的
> 超声波模块（如 JSN-SR04T 等），ECHO 可直接接。TRIG 是输出，3.3V 直连即可。

- 主控：GK7602V11A（SDK 内部 `xm7206v11a`，架构 `xmorca`）
- 传感器：HC-SR04，量程 2cm~4m，精度约 ±3mm

## 引脚复用结论（已核实，无需改 dts / 重编内核）

GPIO6_7 / GPIO7_0 默认就是 GPIO 功能，开箱可用（查 `7206V11A...PIN_OUT.xlsx`「3.管脚控制寄存器」sheet）：

| pad | 物理地址 | 默认值 | func |
|---|---|---|---|
| TRIG (GPIO6_7, iocfg_reg70) | `0x100C0068` | `0x1000` | func0(GPIO6_7) |
| ECHO (GPIO7_0, iocfg_reg71) | `0x100C006C` | `0x1000` | func0(GPIO7_0) + bit12 输入使能 |

`main.c` 的 `padmux_init()` 显式写一遍 `0x1000` 保险。设备树 `&gpio_chip6`/`&gpio_chip7`/`&spi_bus2` 均 `status="ok"`，节点开箱即用。

> 注：这两个 pad 复用的另一功能是 SDIO1_DATA1/DATA0。只要 pad 写 func0(GPIO)，引脚归 GPIO 控制器管，与 SDIO1 无关。若板子实际启用了 SDIO1 外设，需避开。

## 测距原理

1. TRIG 拉高 ≥10us（本驱动自旋 20us）→ 模块发 8 个 40kHz 超声波。
2. ECHO 拉高，声波发出；收到回波后 ECHO 拉低。
3. **距离 cm = ECHO 高电平时间(us) / 58**（声速 340m/s，往返，1cm≈58us）。

脉宽用 `clock_gettime(CLOCK_MONOTONIC_RAW)` busy-wait 轮询 ECHO 测量，上升沿/下降沿各设 30ms 超时保护，最坏约 30ms 返回、不会卡死。用户态分辨率 us 级，对应 mm~cm 级距离（受系统调度抖动影响，板子空闲时足够）。

## 编译

```bash
export PATH=$PATH:<SDK>/tools/linux/toolchains/arm-gcc12.2.0-linux-uclibceabi/bin
cd source/app/sonic_display
make            # 产物 ./sonic_display
```

## 部署与运行

```bash
scp sonic_display root@<板子IP>:/usr/bin/
ssh root@<板子IP> '/usr/bin/sonic_display'
```

```bash
sonic_display     # 每 300ms 测距刷新，Ctrl+C 清屏退出
sonic_display -h  # 帮助
```

SPI 屏显示：标题 / 引脚 / **3 倍放大的距离数值**（cm）/ 状态（in range 绿、no echo 红、out of range 红）/ 公式。串口同步打印每次距离。

## 排错

1. **屏显示 `no echo`**（ECHO 一直没拉高）：前方无障碍 / 太近(<2cm) / ECHO 没接或没分压好 /
   模块没供电（VCC 5V）。先用手在模块前方 10~50cm 挡一下看有无读数。
2. **屏显示 `out of range`**（ECHO 拉高后没拉低）：障碍物 >4m，或声波发散没反射回来。
3. **TRIG/ECHO chardev 申请失败**：`ls -l /dev/gpiochip6 /dev/gpiochip7` 确认节点存在；
   pad 是否被其他驱动占用（SDIO1）。`gpio_get_value` 检查可临时在 sonic.c 打印 ECHO 原始电平。
4. **数值抖动大 / 偏差**：用户态轮询测脉宽固有一定抖动；可对连续几次读数取中值（本驱动未做，
   需要时在 main 循环里加）。近距离(<5cm) HC-SR04 本身不准。
5. **屏不亮**：参考 `source/app/spi_display` 排错。

## 文件说明

| 文件 | 作用 |
|---|---|
| `sonic.c/h` | HC-SR04 驱动：TRIG 触发脉冲 + `clock_gettime` busy-wait 测 ECHO 脉宽 + 超时保护 + 换算 cm |
| `gpio_hal.c/h` | GPIO chardev 封装（输出 `gpio_set_value` + 输入 `gpio_get_value`），复制自 mpu6050_display |
| `spi_hal.c/h` | SPI 屏传输层，复制自 spi_display |
| `st7789.c/h` | ST7789 驱动（RGB565 帧缓冲 + 绘图 API），复制自 spi_display |
| `font8x16.h` | 8×16 ASCII 字模（main 里用它做 3 倍放大大字显示距离） |
| `main.c` | pad 复用 + SPI屏/HC-SR04 初始化 + 主循环测距刷新 |
