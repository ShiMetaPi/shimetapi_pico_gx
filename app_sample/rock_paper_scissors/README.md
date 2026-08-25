# Rock-Paper-Scissors YOLOv8 Detection Application

## 简介

基于 GK7206 NPU 的石头剪刀布实时手势检测应用，使用 YOLOv8 目标检测模型进行手势识别。

## 功能特性

- ✅ **实时 YOLOv8 检测**: 640×640 输入，支持多手势同时检测
- ✅ **Web UI 界面**: MJPEG 视频流 + 检测框可视化
- ✅ **对战模式**: 玩家 vs 电脑，实时计分
- ✅ **检测框显示**: 前端 Canvas 绘制边界框和标签

## 模型信息

- **模型文件**: `neuron_network.xmm` (YOLOv8 格式)
- **输入尺寸**: 1×640×640×3 RGB
- **输出层**: 6 个检测层输出 (3 个尺度 × 2 个分支)
  - `output0`: [1,64,80,80] - bbox regression
  - `output1`: [1,3,80,80] - class scores
  - `output2`: [1,64,40,40] - bbox regression
  - `output3`: [1,3,40,40] - class scores
  - `output4`: [1,64,20,20] - bbox regression
  - `output5`: [1,3,20,20] - class scores
- **类别**: Rock (石头), Paper (布), Scissors (剪刀)

## 目录结构

```
rock_paper_scissors/
├── neuron_network.xmm    # YOLOv8 模型文件
├── labels.txt            # 类别标签 (rock/paper/scissors)
├── model_meta.txt        # 模型元信息 (reg_max/strides/阈值)
├── Makefile              # 编译脚本
├── src/
│   ├── main.c            # 主程序 (VI + VPSS + NPU + Web + 后处理)
│   └── yolov8_postprocess.h  # YOLOv8 DFL 后处理 (反量化/DFL解码/NMS)
├── web/
│   ├── index.html        # Web UI 主页面
│   ├── app.js            # 前端交互逻辑
│   └── style.css         # 样式表
├── input_data0_*.bin     # 参考 I/O（离线校验后处理，非运行时必需）
├── output_data*.bin      # 同上：6 个检测层参考输出
└── README.md             # 本文档
```

## 编译步骤

1. **设置环境变量**:
   ```bash
   cd /home/ljh/GK7206
   source build/env.sh
   ./run.sh lunch  # 选择对应板级配置
   ```

2. **编译应用**:
   ```bash
   cd app_sample/rock_paper_scissors
   make clean
   make
   ```

3. **编译成功后生成**:
   - `project_07_rock_paper_scissors` 可执行文件

## 运行步骤

1. **复制文件到开发板**:
   ```bash
   # 通过 NFS 或 SSH 复制以下文件到开发板 /usr/bin/
   project_07_rock_paper_scissors
   neuron_network.xmm

   # 复制 Web UI 到 /var/www/rps/
   web/index.html
   web/app.js
   web/style.css
   ```

2. **在开发板运行**:
   ```bash
   cd /usr/bin
   ./project_07_rock_paper_scissors
   ```

3. **访问 Web UI**:
   - 打开浏览器访问: `http://<开发板IP>/`
   - 视频流地址: `http://<开发板IP>/mjpeg`

## API 接口

| 接口 | 功能 | 返回示例 |
|------|------|----------|
| `/api/status` | 系统状态 | `{"fps": 30, "model": "yolov8_rps", "input_size": "640x640", "detections": 1}` |
| `/api/detections` | 检测结果列表 | `{"count": 1, "detections": [{"class": "Rock", "score": 0.85, "bbox": [100,150,200,250]}]}` |
| `/api/gesture/current` | 当前手势 | `{"gesture": "Rock", "confidence": 0.85}` |
| `/api/game/start` | 开始对战 | `{"player": "Rock", "computer": "Paper", "result": "lose"}` |
| `/api/game/score` | 计分板 | `{"player": 5, "computer": 3, "rounds": 8}` |

## 技术架构

### 视频流水线

```
VI (摄像头采集)
  ↓
VPSS (图像处理)
  ├─ ochn0 (原分辨率) → VENC (MJPEG 编码) → Web Server
  └─ ochn1 (640×640)  → NPU (YOLOv8 推理) → 检测结果
```

### NPU 推理流程

1. VPSS 输出 640×640 YUV420SP 图像
2. `yuv420sp_to_rgb888()` 转 RGB888，写入模型输入缓冲并 flush cache
3. NPU 加载 `neuron_network.xmm`，执行推理获取 6 个输出层
4. 后处理 `postprocess_yolov8()`：按张量形状识别 box(`[1,64,h,w]`)/cls(`[1,3,h,w]`)
   → 反量化(uint8 affine) → 类别 sigmoid 阈值过滤 → DFL 解码 bbox → NMS
5. 结果写入 `g_detection`(最佳检测，供对战)与 `g_dets[]`(列表，供 `/api/detections`)

### 后处理实现

文件 `src/yolov8_postprocess.h` 提供（参考 SDK `source/gmp/usr/svp/src/post_process/yolov8.c`）：
- `dequant_u8()`: uint8 affine 反量化 `(q - zp) * scale`
- `dfl_decode()`: 对 reg_max=16 个 bin 做 softmax 后按下标加权求和 → 距离值
- `compute_iou()`: IoU 计算
- `yolov8_nms()`: 按类贪心 NMS（`yolov8_default_param` 设 conf=0.25, nms=0.45）

bbox 由 DFL 距离还原：`x1=(-d0+gx+0.5)*stride` 等（与 SDK 一致），并 clamp 到 `[0,640]`。
`src/main.c` 的 `postprocess_yolov8()` 遍历网格、配对 box/cls、解码、NMS，更新全局结果。

## 注意事项

1. **模型输入格式**: 当前假设 VPSS 输出为 RGB 格式，实际可能需要 YUV→RGB 转换
2. **内存管理**: NPU 推理需要 MMZ 分配专用内存缓冲区
3. **性能调优**: 可调整检测阈值 (`conf_threshold`, `nms_threshold`)
4. **多线程**: NPU 推理在独立线程中运行，避免阻塞视频流

## 故障排查

### 问题: 视频流无法显示
- 检查 VENC 是否启动: `cat /proc/umap/venc`
- 检查 ISP 是否正常: `cat /proc/umap/isp`

### 问题: NPU 推理失败
- 检查模型文件: `ls -l neuron_network.xmm`
- 检查 NPU 设备: `cat /proc/umap/npu`
- 查看错误日志: `dmesg | grep -i npu`

### 问题: Web UI 无法访问
- 检查 web_server 进程: `ps | grep web_server`
- 检查网络连接: `ping <开发板IP>`
- 检查端口占用: `netstat -tuln | grep 80`

## 后续改进

- [ ] 优化 NPU 推理性能 (FP16/INT8)、调整阈值 (`model_meta.txt` 的 nms_conf/nms_iou)
- [ ] 检测框 OSD 叠加到 MJPEG（当前框由前端 Canvas 绘制）
- [ ] 手势稳定性滤波（连续 N 帧同类才出拳）
- [ ] 支持多人对战模式

## 参考资料

- GK7206 SDK 文档: `/home/ljh/GK7206/README_CN.md`
- NPU 示例代码: `/home/ljh/GK7206/sample/npu/`
- YOLOv8 后处理源码: `/home/ljh/GK7206/source/gmp/usr/svp/src/post_process/yolov8.c`

---

**版本**: V1.0
**日期**: 2025-06-16
**作者**: Claude Code Assistant