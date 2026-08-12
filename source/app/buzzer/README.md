# 无源蜂鸣器（GPIO5_5 软件方波）

启动后让无源蜂鸣器响 1 秒（2kHz 方波）后关闭。最小示例。

## 硬件接线

| 信号 | 板上 GPIO | 节点 | 说明 |
|---|---|---|---|
| 控制脚（CTRL/SIG） | GPIO5_5 | `/dev/gpiochip5` line5 | 方波驱动 |
| VCC | 3.3V 或 5V | — | 按蜂鸣器模块标称（多数 3.3V/5V 均可） |
| GND | GND | — | — |

- 主控：GK7602V11A（SDK 内部 `xm7206v11a`，架构 `xmorca`）
- GPIO5_5 = iocfg_reg60 @ `0x100C0040`，默认 func0(GPIO)，开箱可用。

## 无源 vs 有源

- **无源蜂鸣器**（本模块）：没有内部振荡器，**必须用方波驱动**——给直流只咔一声不响。
  本模块用 GPIO5_5 翻转输出 ~2kHz 方波，`clock_gettime` busy-wait 控制半周期。
- 有源蜂鸣器：给直流就响（直接拉高 1s 再拉低），不需要方波。本模块的方波对有源蜂鸣器也能响（只是方波的平均电压驱动）。

> GPIO5_5 还能复用 func1=**PWM0**（硬件 PWM）。本模块用软件方波（简单、不依赖 PWM 子系统）；
> 如果要精确频率或长时间连续响（省 CPU），可改用硬件 PWM（pad 写 func1=0x1001 + /sys/class/pwm）。

## 编译

```bash
export PATH=$PATH:<SDK>/tools/linux/toolchains/arm-gcc12.2.0-linux-uclibceabi/bin
cd source/app/buzzer
make            # 产物 ./buzzer
```

## 部署与运行

```bash
scp buzzer root@<板子IP>:/usr/bin/
ssh root@<板子IP> '/usr/bin/buzzer'
```

启动后蜂鸣器响 1s（2kHz）后自动关闭、程序退出。

## 改时长 / 频率

编辑 `main.c` 里的调用后重新编译：
```c
buzzer_beep(1000, 2000);   /* 时长 ms, 频率 Hz */
```
- 频率：2~4kHz 最响；过低发闷、过高发尖。常用 2000 / 2700 / 4000。
- 想做多声（如滴滴滴）：在 main 里循环 `buzzer_beep(100, 2700); usleep(100000);` 几次。

## 排错

1. **不响/只咔一声**：确认是无源蜂鸣器（本模块给方波）；若是有源，方波也能驱动它响，不响则查接线/VCC/极性。
2. **声音太小**：调频率到 2700~4000 试试（无源蜂鸣器有谐振点，某频率最响）；或模块 VCC 不足。
3. **GPIO5_5 申请失败**：`ls -l /dev/gpiochip5` 确认节点；pad 是否被占用。启动日志看 `pad 0x100C0040 -> 0x00001000`。

## 文件说明

| 文件 | 作用 |
|---|---|
| `buzzer.c/h` | 无源蜂鸣器驱动：`buzzer_beep(时长ms, 频率Hz)` 软件方波 + `buzzer_init/off/deinit` |
| `gpio_hal.c/h` | GPIO chardev 封装（输出 `gpio_set_value`），复制自 sonic_display |
| `main.c` | pad 复用 + 初始化 + 响 1s + 关闭退出 |
