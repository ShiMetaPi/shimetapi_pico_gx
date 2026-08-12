# MPU6050 六轴采集 + SPI 屏显示

板端读取 MPU6050（加速度 / 角速度 / 温度），实时显示在 ST7789 SPI 屏上。

## 硬件接线

| 信号 | 板上 GPIO | 控制器 / 节点 | 说明 |
|---|---|---|---|
| SCL | GPIO4_1 | I2C3 → `/dev/i2c-3` | 与 OLED(0x3C) 共享总线 |
| SDA | GPIO4_2 | I2C3 → `/dev/i2c-3` | 与 OLED 共享总线 |
| INT | GPIO0_0 | `/dev/gpiochip0` line0 | 配输入，读电平作状态指示 |
| AD0 | 接 GND | — | 从地址 = **0x68** |
| XCL / XDA | 悬空 | — | 不用（第二 I2C/外接磁力计，本例不用） |
| 显示 | SPI2 | `/dev/spidev2.0` | ST7789 240x240（复用 `spi_display` 传输层） |

- 主控：GK7602V11A（SDK 内部 `xm7206v11a`，架构 `xmorca`）
- 传感器：MPU6050，I2C 从地址 **0x68**（AD0 接地；AD0 接高为 0x69，改 `mpu6050.h` 的 `MPU6050_ADDR`）

## 引脚复用结论（已核实，无需改 dts / 重编内核）

本 SoC 没有 Linux 标准 pinctrl，pad 功能由 `iocfg` 寄存器 bits[3:0] 选择（func 号每脚不同，
查 `7206V11A...PIN_OUT.xlsx` 的「3.管脚控制寄存器」sheet，bit12 为输入使能）。本程序在
`main.c` 的 `padmux_init()` 里经 `/dev/mem` mmap 写三处 pad：

| pad | 物理地址 | 写入值 | 说明 |
|---|---|---|---|
| I2C3_SCL (GPIO4_1, iocfg_reg48) | `0x100C0010` | `0x1002` | func2 = I2C3_SCL |
| I2C3_SDA (GPIO4_2, iocfg_reg49) | `0x100C0014` | `0x1002` | func2 = I2C3_SDA |
| INT (GPIO0_0, iocfg_reg0) | `0x112C0000` | `0x1000` | func0(GPIO)+输入使能（即默认值）|

> 注意 GPIO0_0 在 `0x112C0000` 块（sensor/I2C pad），与 GPIO4_x/5_x 的 `0x100C00xx`（LCD pad）
> 是两个不同的 iocfg 控制器。SPI 屏的 pad（4_4/4_5/4_7/5_0/5_1）由 `spi_hal_init()` 内部切。

设备树 `xmorca.dts` 的 `&i2c_bus3` / `&gpio_chip0` / `&spi_bus2` 均 `status="ok"`，
`/dev/i2c-3`、`/dev/gpiochip0`、`/dev/spidev2.0` 开箱即用。

## 编译

通过 `run.sh`（自动设工具链 PATH）或手动：

```bash
export PATH=$PATH:<SDK>/tools/linux/toolchains/arm-gcc12.2.0-linux-uclibceabi/bin
cd source/app/mpu6050_display
make            # 产物 ./mpu6050_display
```

## 部署与运行

**快速调试（不烧录，推荐）**：板子 SSH（`root@<板子IP>`）可达时直接 scp：

```bash
scp mpu6050_display root@<板子IP>:/usr/bin/
ssh root@<板子IP> '/usr/bin/mpu6050_display'
```

```bash
mpu6050_display     # 每 150ms 采集刷新，Ctrl+C 清屏退出
mpu6050_display -h  # 帮助
```

启动后串口会依次打印 pad 写入值、SPI/st7789 init、`WHO_AM_I = 0x68`，随后 SPI 屏出现数值表。

## 显示内容

```
MPU6050 0x68          ← 标题
I2C-3  INT:0          ← 总线 + INT 脚电平（>0 绿色）
--------------------
ACCEL  g
 X +0.01  Y -0.02
 Z +0.98              ← 静止时竖直放：Z≈±1g
GYRO  dps
 X +0.3  Y -0.5
 Z +0.1
T  26.5 C             ← 温度
Ctrl+C quit
```

- 量程：加速度 ±2g（灵敏度 16384 LSB/g）、陀螺仪 ±250°/s（131 LSB/°/s）—— MPU6050 上电默认档。
- 数据为原始物理量，未做滤波 / 零偏校准 / 姿态解算。
- INT 仅作数据就绪电平指示，采集走 150ms 定时轮询（不依赖中断）。

## 排错

1. **串口报 `读 WHO_AM_I 失败` / 屏显示 `MPU6050 FAIL`**：I2C 没通。
   - `ls -l /dev/i2c-3`（不存在说明内核没 probe `i2c_bus3`）。
   - 复查 SCL/SDA 别接反、AD0 是否真接地、MPU6050 VCC 3.3V。
   - 板载 regtools 探测：`i2c_read 3 0x68 0x75 1 1`（应能返回 0x68）。
2. **`WHO_AM_I=0xFF` / 全 0xFF**：典型 I2C 没应答（SDA/SCL 没接或 pad 没切 func2）。
   **全 0x00**：设备没唤醒或读错寄存器（检查 `PWR_MGMT_1` 写入）。
3. **`WHO_AM_I` 非 0x68**：市售"MPU6050"模块很多实为 MPU6500，WHO_AM_I 返回 **0x70**
   （MPU9250/9255 为 0x71）。驱动已兼容 0x68/0x70/0x71（寄存器布局相同），照常采集，
   启动日志会打印识别到的型号。AD0 接高则地址 0x69。
4. **屏不亮 / 花屏**：参考 `source/app/spi_display` 的排错（DC/RES pad 需 func5、CS func0、
   `spi_bus2` 的 `com-mode` 需为 1=POLLING）。

## 文件说明

| 文件 | 作用 |
|---|---|
| `mpu6050.c/h` | MPU6050 驱动：寄存器定义 + init（校验 WHO_AM_I/唤醒/配量程）+ read（14 字节块读 + 物理量换算） |
| `i2c_hal.c/h` | `/dev/i2c-X` + `I2C_RDWR` 用户态读写封装（含 read：复合事务块读），复制自 `oled_display` 并扩展 |
| `gpio_hal.c/h` | GPIO chardev 封装（含 `gpio_get_value` 读输入），复制自 `spi_display` 并扩展 |
| `spi_hal.c/h` | SPI 屏传输层（硬件 SPI2 + chardev DC/RES/CS），复制自 `spi_display` |
| `st7789.c/h` | ST7789 驱动：RGB565 帧缓冲 + 绘图 API + 整屏刷新，复制自 `spi_display` |
| `font8x16.h` | 8×16 ASCII 点阵字模，复制自 `spi_display` |
| `main.c` | pad 复用 + SPI屏/MPU6050/INT 初始化 + 主循环采集显示 |
