# ShiMetaPi-Pico-G1 SDK

> **Version:** V1.0.0
> **Chip:** GK7206V11A / GK7206V12A 
> **Board:** ShiMetaPi-Pico-G1 系列

ShiMetaPi-Pico-G1 SDK 基于国科微官方提供的SDK进行了二次开发，针对ShiMetaPi-Pico-G1系列开发板的硬件特性与应用场景进行了适配和优化，旨在为开发者提供更加高效、便捷、友好的开发体验。

---

## 1 快速开始
  
### 1.1 安装宿主机依赖

SDK 自带了交叉编译工具链和固件打包工具，但仍需安装以下宿主机依赖：


```bash
sudo apt update
```

```bash
sudo apt install -y build-essential make gcc fakeroot bc bison flex libncurses5-dev libssl-dev libelf-dev zlib1g-dev liblzma-dev liblzo2-dev liblz4-tool libzstd-dev device-tree-compiler
```

### 1.2 获取SDK

* github

```
git clone https://github.com/ShiMetaPi/shimetapi_pico_g1.git
```

* gitee

```
git clone https://gitee.com/ShiMetaPi/shimetapi_pico_g1.git
```

### 1.3 SDK目录结构

```
GK7206/
├── app_sample/            # 应用示例（人脸识别、运动检测等）
├── build/                 # 构建系统
│   ├── base.mk            # 全局构建变量、工具链、输出路径
│   ├── env.sh             # 环境变量与 PATH 设置
│   └── rootfs_image.mk    # 固件镜像打包规则
├── configs/               # 板级配置文件
│   ├── xm7206v11a/        # ShiMetaPi-Pico-G1-128M_DDR
│   └── xm7206v12a/        # ShiMetaPi-Pico-G1-256M_DDR
├── out/                   # 编译输出（自动生成）
│   └── xm7206v12a/
│       ├── image/         # 固件镜像（JFFS2/YAFFS2/EXT4/UBIFS）
│       ├── rootfs/        # 根文件系统
│       ├── lib/           # SDK 库（static/ + share/）
│       ├── ko/            # 内核模块
│       └── linux-5.10.y/  # 内核编译产物
├── sample/                # 官方示例程序（27 个分类）
├── source/                # 源码
│   ├── bootloader/        # U-Boot (2020.01)
│   ├── kernel/            # Linux Kernel (5.10.y / 4.9.y)
│   ├── gmp/               # 媒体处理库（预编译）
│   ├── rootfs/            # 根文件系统构建
│   ├── component/         # 系统组件
│   └── initramdisk/       # Initramdisk
├── tools/                 # 工具与工具链
│   └── linux/toolchains/  # 交叉编译工具链（已内置）
├── cfg.mk                 # 当前 SDK 配置（由 lunch 生成）
├── run.sh                 # 主构建脚本
└── tar_rootfs.sh          # 打包 rootfs 脚本
```

---

### 1.4 run.sh使用说明

#### 1.4.1 选择板级配置

> 本开发板提供 GK7206V12A / GK7206V11A 两种芯片版本，以满足不同应用场景的需求。其中，GK7206V12A 版本配备 256MB DDR，GK7206V11A 版本配备 128MB DDR。

```bash
./run.sh lunch
```

交互式选择开发板和内核版本：(按提示输入编号回车进行选择)

```
========================================
  Select Board Configuration
========================================

Board:
  1. ShiMetaPi-Pico-G1-128M_DDR(GK7206V11A)
  2. ShiMetaPi-Pico-G1-256M_DDR(GK7206V12A)

Select board [1-2]: 2
  -> ShiMetaPi-Pico-G1-256M_DDR(GK7206V12A)

Kernel:
  1. linux-5.10
  2. linux-4.9

Select kernel [1-2]: 1
  -> linux-5.10

========================================
  Board:    ShiMetaPi-Pico-G1-256M_DDR(GK7206V12A)
  Kernel:   linux-5.10
========================================
Configuration written to cfg.mk

Done.
```

选择结果会写入 `cfg.mk`。

---

#### 1.4.2 构建命令

| 命令 | 说明 |
|------|------|
| `./run.sh uboot`  | 编译 U-Boot 引导程序 |
| `./run.sh kernel` | 编译 Linux 内核 |
| `./run.sh rootfs` | 构建根文件系统 |
| `./run.sh driver` | 编译内核驱动（GMP） |
| `./run.sh sysdrv` | 编译 uboot + kernel + rootfs |
| `./run.sh osal`   | 编译 OSAL 适配层 |
| `./run.sh media`  | 编译媒体库 |
| `./run.sh sample` | 编译MPP示例应用 |
| `./run.sh mcu`    | 编译 MCU 固件 |
| `./run.sh all`    | 编译全部组件 |

#### 1.4.3 清理命令

| 命令 | 说明 |
|------|------|
| `./run.sh clean uboot` | 清理 U-Boot |
| `./run.sh clean kernel` | 清理内核 |
| `./run.sh clean rootfs` | 清理 rootfs |
| `./run.sh clean driver` | 清理驱动 |
| `./run.sh clean sysdrv` | 清理 uboot/kernel/rootfs |
| `./run.sh clean media` | 清理媒体库 |
| `./run.sh clean sample` | 清理MPP示例应用 |
| `./run.sh clean osal` | 清理 OSAL 适配层 |
| `./run.sh clean mcu` | 清理 MCU 固件 |
| `./run.sh clean all` | 清理全部组件 |

#### 1.4.4 信息命令

| 命令 | 说明 |
|------|------|
| `./run.sh info` | 显示当前配置信息 |
| `./run.sh check` | 检查编译环境 |
| `./run.sh env` | 显示工具链路径 |
| `./run.sh list` | 列出可用板级配置 |
| `./run.sh help` | 显示帮助信息 |

#### 1.4.5 并行编译

所有编译命令都支持 `-j[N]` 参数：

```bash
./run.sh kernel -j       # 自动使用所有核心
./run.sh kernel -j4      # 使用 4 个核心
./run.sh kernel -j1      # 仅使用 1 个核心--调试用
```

---

## 2 示例应用（app_sample/）

| 示例 | 说明 |
|------|------|
| `app_sample/face_recognize` | 人脸检测与识别（MTCNN + NPU，~8fps） |
| `app_sample/rpi_detector` | 运动检测器 |

### 2.1 应用编译示例

```bash
cd app_sample/face_recognize
make
```

### 2.2 板端运行示例

* 在板端执行
```bash
mkdir sd_card
mount /dev/mmcblk1p1 /sd_card
```

* 在SDK中执行
```
scp app_sample/face_recognize/face_recognize root@<board_ip>:/sd_card/
```

* 在板端执行
```bash
cd /sd_card
chmod +x face_recognize
./face_recognize
```

---

## 3 常见问题

### 3.1 编译报错 `arm-gcc12.2.0-linux-uclibceabi-gcc: command not found`

确保通过 `./run.sh` 或 `source build/env.sh` 设置了 PATH，工具链已内置在 `tools/linux/toolchains/` 中。

### 3.2 如何切换芯片型号

```bash
./run.sh lunch    # 重新选择 Board
```

### 3.3 如何查看当前配置

```bash
./run.sh info
```

### 3.4 编译后 rootfs 中没有我的应用程序

SDK 不会自动将 `app_sample/` 中的应用放入 rootfs。需要手动复制：

```bash
cp app_sample/face_recognize/face_recognize out/xm7206v12a/rootfs/usr/bin/
```

然后重新生成 rootfs 镜像：

```bash
./run.sh rootfs
```

### 3.5 如何重新打包 rootfs

```bash
./run.sh rootfs
```

该命令会重新构建 rootfs 目录并打包为对应格式的固件镜像。

---

