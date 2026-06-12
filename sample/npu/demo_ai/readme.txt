# sample使用指南
    7206 npu sample使用指南

# 驱动依赖
    在out/xm7206xxx/ko目录中
    使用命令加载ko
    例如：./load xm7206v11a -i

# 目录结构描述
.
├── common
│   ├── data_base
│   │   └── ...
│   ├── demo_common.c
│   ├── demo_common.h
│   ├── font
│   └── ...
├── sample_svp                                                  // 可执行文件
├── sample_svp_main.c
├── sample_svp_main.h
├── Makefile
├── model                                                       // 模型文件夹
│   ├── gnn_car_detect_640x360_rgb888hwc_v0101_20250523.bin     // 模型文件，机动车检测
│   ├── gnn_person_detect_640x360_rgb888hwc_v0102_20240909.bin  // 模型文件，人形检测
│   └── ...
├── res
│   ├── face_feature_data.txt                                   // 人脸识别相关文件
│   ├── person_name_id                                          // 人脸识别相关文件
│   └── convert_jpg_to_yuv.sh                                   // jpg转换yuv脚本
└── ...

# 使用说明
    1.SDK包整编、sample整编、npu sample位置：XMIPCLinuxV100R005C00SPCXXX/sample/npu/demo_ai
    2.如修改sample仅需demo_ai目录下编译，在demo_ai文件夹中找到可执行文件
    3.运行demo：./sample_svp <index>名称 ,例如：./sample_svp 0
    4.<index>为下列功能列表对应功能，选择对应数字即为启动对应功能
    5.运行程序后输入字母q退出程序

# 功能介绍
| AI功能 | 描述 | 涉及模型 |
0) person detect | 人形检测 | (yolov5)gnn_person_detect_640x360_rgb888hwc_v0101_20241124.bin
1) person keypoint detect | 人体关键点 | (yolov8)gnn_person_detect_keypoint_640x360_rgb888hwc_v0101_20241205.bin
2) face detect | 人脸检测 | (yolov5)gnn_face_detect_640x360_rgb888hwc_v0101_20241205.bin
3) pet detect | 宠物检测 | (yolov5)gnn_face_detect_640x360_rgb888hwc_v0101_20241205.bin
4) car detect | 车辆检测 | (yolov5)gnn_car_detect_640x360_rgb888hwc_v0101_20250523.bin
5) non motorized vehicle detect | 非机动车检测 | (yolov5)gnn_nocar_detect_640x360_rgb888hwc_v0102_20240417.bin
6) head detect | 人头检测 | (yolov5)gnn_head_detect_640x360_rgb888hwc_v0102_20240318.bin
7) fireworks detect | 烟火检测 | (yolov5)gnn_fireworks_detect_640x360_rgb888hwc_v0101_20250523.bin
8) package detect | 包裹检测 | (yolov5)gnn_package_detect_640x360_rgb888hwc_v0101_20250523.bin
9) aov person | AOV人形检测 | (yolov5)gnn_aov_person_detect_640x360_rgb888hwc_v0101_20240909.bin
10) adas detect | adas检测 | 车牌检测 gnn_license_plate_detection_rgb888hwc_v0101_20241205.bin
                            非机动车识别 gnn_nocar_detect_640x360_rgb888hwc_v0102_20240417.bin
                            车辆检测 gnn_car_detect_640x360_rgb888hwc_v0101_20250523.bin
                            车道线检测 gnn_laneline_detect_640x360_rgb888hwc_v0101_20250616.bin
                            人形检测 gnn_person_detect_640x360_rgb888hwc_v0102_20240909.bin
11) fire detect | 烟火检测 | (yolov8)gnn_fire_detect_640x360_rgb888hwc_v0101_20250313.bin
12) gesture recognition | 手势识别 | 手势检测 gnn_gesture_detection_640x360_rgb888hwc_v0101_20241011.bin
                                    手势分类 gnn_gesture_classification_96x96_rgb888hwc_v0101_20241203.bin
13) face emotion classification | 人脸表情分类 | 人脸检测 gnn_face_detect_640x360_rgb888hwc_v0102_20240131.bin
                                               人脸关键点检测 gnn_face_keypoint_48x48_rgb888hwc_v0101_20250818.bin
                                               人脸表情分类 gnn_face_emotion_112x112_rgb888hwc_v0101_20250904.bin
14) face recognition | 人脸识别 | 人脸检测 gnn_face_detect_640x360_rgb888hwc_v0102_20240131.bin
                                 人脸关键点检测 gnn_face_keypoint_48x48_rgb888hwc_v0101_20250818.bin
                                 人脸识别 gnn_face_recognition_112x112_rgb888hwc_v0101_20250818.bin
15) person rcnn detect | 人形检测+二阶段 | 人形检测 gnn_person_detect_640x360_rgb888hwc_v0102_20240909.bin
                                         二阶段分类 gnn_second_stage_128x128_rgb888hwc_v0101_20250721.bin
16) car rcnn detect | 车辆检测+二阶段 | 车辆检测 gnn_person_detect_640x360_rgb888hwc_v0102_20240909.bin
                                      二阶段分类 gnn_second_stage_128x128_rgb888hwc_v0101_20250721.bin
17) pet rcnn detect | 宠物检测+二阶段 | 宠物检测 gnn_person_detect_640x360_rgb888hwc_v0102_20240909.bin
                                      二阶段分类 gnn_second_stage_128x128_rgb888hwc_v0101_20250721.bin
18) companion robot | 宠物陪伴机器人 | 宠物检测 gnn_pet_detect_640x360_rgb888hwc_v0103_20250520.bin
                                     宠物分类 gnn_pet_classify_128x128_rgb888hwc_v0101_20250926.bin
19) person rcnn 720p detect | 远距离人形检测 | 远距离人形检测 gnn_person_nocar_detect_1280x720_rgb888hwc_v0101_202507025.bin
                                             二阶段分类 gnn_second_stage_128x128_rgb888hwc_v0101_20250721.bin
20) car rcnn 720p detect | 远距离车型检测 | 远距离车辆检测 gnn_car_detect_1280x720_rgb888hwc_v0101_20251103.bin
                                          二阶段分类 gnn_second_stage_128x128_rgb888hwc_v0101_20250721.bin
21) pet rcnn 720p detect | 远距离宠物检测 | 远距离宠物检测 gnn_pet_detect_1280x720_rgb888hwc_v0101_20251103.bin
                                          二阶段分类 gnn_second_stage_128x128_rgb888hwc_v0101_20250721.bin

# attr属性介绍
    detect_threshold = 0.65f;      // 检测阈值，建议值0.65f
                                          开启二阶段检测|远距离检测时，需要设置为0.1f
    classifier_threshold = 0.01f;  // 分类阈值，默认0.01f
                                          开启二阶段检测|远距离检测时，需设置三个档位，低：0.3f，中：0.5f，高：0.7f，默认：0.5f
    iou_threshold = 0.5f;          // iou相交比阈值，建议值0.5f
    max_target_num = 10;           // 输出最大检测目标数，默认值10，最大支持50
    bytetrack_enable               // 追踪开关，开启目标追踪
                                          单个目标(tracker_id)可持续存在30帧，丢失目标后重新追踪目标会重置ID，ID从0-1500循坏使用
                                          目标出现累计帧数(tracker_age)出现后会累加，最大值为240，消失后会重新累加
    motionless_filter_enable       // 运动状态检测开关，开启后输出运动状态
                                          0：未知（目标刚出现，等待判断具体状态），1：静止，2：运动
    stillness_thres = 0.95f;       // 当前帧与历史帧相交大于95%算当前帧静止，静止计数+1，阈值越低越容易被判断为静止
    movement_fps_thres = 7;        // 静止帧计数连续7帧大于stillness_thres，则目标被判断为静止，判断帧数越低越容易被判断为静止

# svp接口介绍
/*
 *  函数功能：创建一路svp_task处理流程
 *  函数参数：
 *      handle  - 输出参数，创建成功返回SVP句柄
 *      cfg     - 输入参数，svp_task配置信息
 *  返回值：
 *      XMEDIA_SUCCESS - 函数执行成功
 *      XMEDIA_FAILURE - 函数执行失败
 *  注意：
 *      每个句柄对应一路svp_task处理流程
 */
    xmedia_svp_task_create(xmedia_s32 *handle, const xmedia_svp_task_cfg cfg)

/*
 *  函数功能：获取svp_task处理流程的属性
 *  函数参数：
 *      handle    - 输入参数，svp_task处理流程句柄
 *      task_attr - 输出参数，获取的svp_task属性
 *  返回值：
 *      XMEDIA_SUCCESS - 函数执行成功
 *      XMEDIA_FAILURE - 函数执行失败
 */
    xmedia_s32 xmedia_svp_task_get_attr(xmedia_s32 handle, xmedia_void *task_attr);

/*
 *  函数功能：设置svp_task处理流程的属性
 *  函数参数：
 *      handle    - 输入参数，svp_task处理流程句柄
 *      task_attr - 输入参数，设置svp_task属性
 *  返回值：
 *      XMEDIA_SUCCESS - 函数执行成功
 *      XMEDIA_FAILURE - 函数执行失败
 *  注意：
 *      创建后，销毁前的任意时间均可调用
 */
    xmedia_svp_task_set_attr(xmedia_s32 handle, const xmedia_void *task_attr)

/*
 *  函数功能：开始svp_task处理流程
 *  函数参数：
 *      handle     - 输入参数，SVP处理流程句柄
 *      input      - 输入参数，输入图像帧信息
 *      output     - 输出参数，处理结果信息
 *  返回值：
 *      XMEDIA_SUCCESS - 函数执行成功
 *      XMEDIA_FAILURE - 函数执行失败
 *  注意：
 *      创建后，销毁前的任意时间均可调用
 */
    xmedia_svp_task_process(xmedia_s32 handle, const xmedia_svp_task_input *input, xmedia_void *output)

/*
 *  函数功能：销毁一路svp_task处理流程
 *  函数参数：
 *      handle  - 输入参数，SVP处理流程句柄
 *  返回值：
 *      XMEDIA_SUCCESS - 函数执行成功
 *      XMEDIA_FAILURE - 函数执行失败
 */
    xmedia_svp_task_destroy(xmedia_s32 handle)
