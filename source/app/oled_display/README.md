# OLED 显示应用（SSD1306 128×64, I2C）

板端运行的 OLED 显示示例：欢迎画面 + 实时系统信息（uptime / loadavg / 内存）。

## 硬件接线

| 信号 | 板上 GPIO | 对应控制器 | 设备节点 |
|---|---|---|---|
| SCL | GPIO4_1 | I2C3 | `/dev/i2c-3` |
| SDA | GPIO4_2 | I2C3 | `/dev/i2c-3` |
| VCC | 3.3V | — | — |
| GND | GND | — | — |

- 主控：GK7602V11A（SDK 内部 `xm7206v11a`，架构 `xmorca`）
- OLED：SSD1306，128×64，I2C 从地址 **0x3C**（SA0 接地；SA0 接高为 0x3D）

## 引脚复用结论（已核实，无需改 dts / 重编内核）

GPIO4_1/GPIO4_2 这对 pad 物理上复用为 I2C3。本 SoC 没有 Linux 标准 pinctrl 驱动，pad 功能由 `iocfg` 寄存器块选择（I2C pad 在 `iocfg_ctrl2 @ 0x112C0000`）。源码里只有 U-Boot 的调试命令 `i2c_cmd.c` 配过 i2c0/i2c2，**没有任何代码配 i2c3**——这对 pad 靠 **SoC 复位默认值 / BootROM** 已处于 I2C3 功能。

设备树 `i2c_bus3` 已 `status="ok"`（100kHz），别名 `i2c3`，加 `CONFIG_I2C_CHARDEV=y` / `CONFIG_I2C_LOTUS=y`，因此 **`/dev/i2c-3` 由内核自动创建，开箱即用**。

> 注意：`i2c_bus3` 节点没有 `interrupts` 属性，`i2c-lotus` 驱动会回退到**轮询模式**（日志可见 `falling back to polling mode`），I2C_RDWR 照常可用。

## 编译

通过 `run.sh`（自动设置工具链 PATH）或手动：

```bash
export PATH=$PATH:<SDK>/tools/linux/toolchains/arm-gcc12.2.0-linux-uclibceabi/bin
cd source/app/oled_display
make            # 产物 ./oled_display
```

## 部署与运行

**快速调试（不烧录，推荐）**：板子 SSH（`root@<板子IP>`）可达时直接 scp：

```bash
scp oled_display root@<板子IP>:/usr/bin/
ssh root@<板子IP> '/usr/bin/oled_display'
```

**运行参数**：

```bash
oled_display            # 欢迎画面，随后每秒刷新系统信息，Ctrl+C 退出
oled_display --once     # 仅显示欢迎画面，3 秒后退出
oled_display -h         # 帮助
```

**永久装进 SPI 镜像**（可选）：在 `source/rootfs/Makefile` 的 `rootfs_post` 里加一行：

```makefile
@$(MAKE) -C $(SDK_DIR)/source/app/oled_display install
```

之后 `./run.sh rootfs -j` 即把应用打进 `rootfs.64k.jffs2`，板端落点 `/usr/bin/oled_display`。

## 修改配置

编辑 `ssd1306.h` 顶部宏后重新编译：

```c
#define OLED_I2C_BUS   3      // 换总线
#define OLED_I2C_ADDR  0x3C   // 换从地址
```

分辨率/页数由 `OLED_WIDTH`/`OLED_HEIGHT` 决定（改 128×32 需同步改初始化序列里的 `0xA8`/`0xDA` 参数）。

## 排错（OLED 不亮 / 花屏）

1. **确认节点存在**：`ls -l /dev/i2c-3`（不存在说明内核没 probe，检查 `i2c_bus3` 的 `status`）。
2. **探测从设备应答**（板载 regtools）：
   ```bash
   i2c_read 3 0x3c 0x00 1 1     # 能返回任意数据即说明 0x3C 有 ACK
   ```
   - 也可依次试 `0x3d`，确认地址跳线。
3. **无应答时**：
   - 复查接线（SCL/SDA 别接反）、VCC/GND、地址跳线。
   - 确认这对 pad 处于 I2C 功能。极少数板子复位默认为 GPIO，需手动写 `iocfg_ctrl2`（基址 `0x112C0000`）的功能寄存器为 I2C。参考 `source/bootloader/u-boot-2020.01/lotus/drivers/i2c/i2c_cmd.c` 对 i2c0/i2c2 写 `0x1d01` 的写法；i2c3 的具体寄存器偏移需查芯片 datasheet（源码未配置过）。可用板载用户态工具 `xmmm`（基于 `/dev/mem`，`CONFIG_DEVMEM=y` 且非 STRICT）写寄存器，用法见 `source/app/factory_test`。
4. **黑屏但有应答**：初始化序列已含 `0x8D 0x14`（charge pump ON，0.96″ 模组必须），若仍黑屏多半是 VCC 不足或对比度问题，可调 `0x81` 后的对比度值。

## 文件说明

| 文件 | 作用 |
|---|---|
| `i2c_hal.c/h` | `/dev/i2c-X` + `I2C_RDWR` 用户态读写封装 |
| `ssd1306.c/h` | SSD1306 驱动：显存帧缓冲 + 绘图 API + 整屏刷新 |
| `font8x16.h` | 8×16 ASCII 点阵字模（0x20~0x7E） |
| `gen_font8x16.py` | 字模生成工具（本机 PIL 运行，板端不依赖） |
| `main.c` | 示例程序 |
