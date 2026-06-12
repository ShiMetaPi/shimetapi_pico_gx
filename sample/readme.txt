各用例使用说明:
    sample_vio：
        功能描述：mpp框架串联
        命令格式：./sample_vio [work_mode] [stream_mode]
                  work_mode：
                  0 vi在线vpss在线
                  1 vi离线vpss在线（可用于支持4K分辨率sensor输入）
                  2 vi离线vpss离线
                  3 vi在/离线vpss离线,wdr模式 （默认vi在线）
                  4 vi离线vpss离线，vi选择线性/wdr切换
                  5 vi离线vpss离线，分辨率切换
                  6 vi离线vpss在线，双目(支持任意2lane输入的sensor组合)
                  7 vi离线vpss在线，双目拼接
                  8 vi在线vpss在线，双目+内置switch
                  9 vi离线vpss离线，预录制及回读用例(CV4003 sensor)
                  10 vi离线vpss在线，ldc畸变矫正
                  11 vi离线vpss在线，ahd sensor输入
                  stream_mode：
                  0: 输出到显示设备
                  1: 编码输出文件
    sample_ainr:
        功能描述：ai isp 功能使用
        命令格式：./sample_ainr [scene_mode]
                  scene_mode：
                  0  单路线性ainr 15fp
                  1  双路线性ainr+内置switch 7fps
                  2  双线性ainr 7fps

    sample_region:
        功能描述：region功能使用
        命令格式：./sample_region [index]
                  index:
                  0 VPSS OVERLAYEX.
                  1 VPSS COVEREX.
                  2 VPSS MOSAICEX.
                  3 VPSS CORNER RECTEX.
                  4 VO OVERLAYEX.
                  5 VO COVEREX.
                  6 VO CORNER RECTEX.
                  7 VENC OVERLAY.
                  8 VENC COVER.
                  9 VENC MOSAIC.
                  10 VENC CORNER RECT
    sample_ircut:
        功能描述：ircut功能使用，需结合单板硬件设计进行适配
        命令格式：./sample_ircut [mode] [board_type] [videv] [status] [normal2ir_iso_thr] [ir2normal_iso_thr] [rgmax] [rgmin] [bgmax] [bgmin]
                  mode:
                  0 手动模式
                  1 标定模式
                  2 自动模式
                  board_type: pcb板型号
                  videv: use vi-dev num
                  status:
                  0  ir模式切换到normal模式 （手动模式下）
                  1  normal模式切换到ir模式 （手动模式下）
                  0  当前ir状态为normal模式  (自动模式下)
                  1  当前ir状态为ir模式      (自动模式下)
                  normal2ir_iso_thr：自动模式下normal切换到ir状态的iso阈值
                  ir2normal_iso_thr：自动模式下ir切换到normal状态的iso阈值
                  rgmax/rgmin/bgmax/bgmin: 自动模式下ir切换到normal状态的r/g（b/g）的阈值
         注：运行此程序,先确保图像采集程序运行比如sample_venc

    sample_venc：
        功能描述：采集编码用例
        命令格式：1)./sample_venc [index]
                    0 卷绕/低延时 h265+h264编码 主码流+子码流+jpeg
                    1 h265+h264编码 主码流+子码流
                    2 h265+h264编码 qpmap
                    3 h265+h264编码 roi
                    4 jpeg/mjpeg编码
                  2)./sample_venc [nv21 yuv file] + [width] + [height] + [target format]
                    nv21 yuv file:   输入yuv文件
                    width：          输出分辨率宽
                    height：         输出分辨率高
                    target format：  输出编码类型h265, h264, jpeg, mjpeg

    sample_vo：
        功能描述：vo用例
        命令格式：
                  0 单路显示设备输出，可配置显示路数(1/2)，以及区域模式
                  ./sample_vo 0 [vo_chn_num] [screen0_TYPE] [vo_part_mode]
                  vo_chn_num：             图像路数1~2
                  screen0_TYPE：           显示接口类型
                  vo_part_mode：           0 单区域模式 1 多区域模式
                  4 虚拟vo->venc,可配置显示路数(1/4)
                 ./sample_vo 4 [vo_virt_dev] [vo_chn_num] [screen0_TYPE]
                    vo_virt_dev:             虚拟vo设备(2)
                    vo_chn_num：             图像路数1~2
                    screen0_TYPE：           显示接口类型
                  5 vi->vpss->vo,可配置显示路数(1/2)
                  ./sample_vo 5 [vo_chn_num] [screen0_TYPE] [sensor_type]
                  vo_chn_num：             图像路数1~2
                  screen0_TYPE：           显示接口类型
                  sensor_type:             sensor类型(0)，只支持sc485sl sensor

                  screen0_TYPE:
                  0:PLCD800x480      8:SLCD_240X320_18BIT    12:KD026_320X240P10_4LINE_RGB565
                  14:KD026_320X240P60_18BIT_18BPP_RGB666_8080I    16:SIL9024_BT1120_1280X720P60 
                  17:SIL9024_BT656_1280X720P30_UYVY422
        注意：1 需要准备相应的输入文件和输出文件的内存空间
              2 需要加载相应的驱动,各接口驱动配置具体参考vo目录下的readme

    sample_ive：
        功能描述：ive算子用例
        命令格式：./sample_ive [index]
                  index:
                  0  Sobel.(FILE->IVE->FILE).
                  1  Edge,complete edge by mag_and_ang.(FILE->IVE->FILE).
                  2  Gmm2.(FILE->IVE->FILE).
                  3  MemoryTest.(FILE->IVE->FILE).
                  4  Motion detected.(VI->VPSS->IVE->VGS->VENC).
                  5  Canny.(FILE->IVE->FILE).
                  6  Od.(VI->VPSS->IVE->VGS->VENC).
                  7  St Lk.(FILE->IVE->FILE).
                  8  dma.(FILE->IVE->FILE).
                  注意： 需要准备相应的输入文件和输出文件的内存空间

    sample_lsadc：
        功能描述：adc功能 支持配置扫描模式和通道选择，可根据用例运行时的提示去做配置
        命令格式：./sample_lsadc
        注意：sdk驱动默认不加载xm_lsadc.ko,使用前请先加载xm_lsadc.ko和编译sample_lsadc

    sample_tde：
        功能描述：tde模块用例,通过vo演示叠加图片到fb的效果
        命令格式：./sample_tde

    sample_vgs：
        功能描述：vgs模块用例
        命令格式：./sample_vgs [index]
                  index:
                  1 cover+osd+luma+scale 对1帧yuv图像进行遮挡,叠osd，提取亮度 缩放操作
                  2 旋转

    sample_fb：
        功能描述：图形层叠加功能
        命令格式：./sample_gfbg [test_mode] [screen0_TYPE] [sleep_ms]
                  test mode:         0:normal test    2:tde-fb test   4:one buffer test
                  screen0_TYPE:      0:PLCD800x480   5:BT656_1280X720_UYVY422     8:SLCD_240X320_18BIT    12:KD026_320X240P10_4LINE_RGB565
                  screen0_TYPE:      14:KD026_320X240P60_18BIT_18BPP_RGB666_8080I    16:SIL9024_BT1120_1280X720P60
                  sleep_ms:          20:send frame gap 20ms
                  gfbg_test example :                  ./sample_gfbg 0 16 20
                  gfbg_tde_test example :              ./sample_gfbg 2 16 20
                  gfbg_one_buffer_test example :       ./sample_gfbg 4 16 20
        注意事项：此功能依赖vo才能查看效果，需要配置显示部分的相关驱动

    音频：
        sample_audio_ai:
            功能描述：audio ai功能使用
            命令格式：./sample_audio_ai [dev] [send_object] [binder_mode] [samplerate] [usr_samplerate] [channels] [workmode] [outfile/send_object_dev] [vqe_type/share_mode]
                      dev: dev id
                      send_object: 0:not send data and save ai file, 1:ai send data to aenc, 2:ai send data to ao
                      binder_mode: use binder mode
                      samplerate: dev samplerate
                      usr_samplerate: ai user data samplerate
                      channels: dev chn cnt
                      workmode: mono:0, queue:1
                      outfile/send_object_dev: ai out file or aenc encoder out file(.pcm/.g711a/.g711u/.g726/.aac/.mp3/.adpcm) or ao dev
                      vqe_type/share_mode: 0: not open vqe, 1: open vqe v1, 2: open vqe v2, 3: open vqe v1 and detect, 4: open vqe detect, 5: open share mode, 6: open vqe v3
        sample_audio_ao:
            功能描述：audio ao功能使用
            命令格式：./sample_audio_ao [infile] [dev] [dev_rate] [dev_chn] [frm_rate] [frm_chn] [mode] [pcm_sample] [vqe]
                      infile:     input file name,only support pcm file
                      dev:        (0/1)->(DAC0/I2S0)
                      dev_rate:   (8000/16000/32000/48000) -> dev attr chn
                      dev_chn:    (1/2)-> user data chn\n
                      frm_rate:   (8000/16000/32000/48000) -> user frame chn
                      frm_chn:    (1/2)-> user data chn
                      mode:       (0/1)->(mono/queue)
                      pcm_sample: frame_pcm_samples
                      vqe:        (0/1/2)-> vqe disable/enable v1/enable v2
        sample_audio_record_detect:
            功能描述：audio 算法功能使用
            命令格式：./sample_audio_record_detect [samplerate] [usr_samplerate] [channels] [codec_input_vol] [outfile] [vqe_type]
                      dev_rate:  dev samplerate(8000/16000/32000/48000)
                      usr_rate:  out file samplerate(8000/16000/32000/48000)
                      dev_chn:   dev chn cnt(1/2/8)
                      out_file:  pcm out file(.pcm)
                      vqe_type: 0:not open detect, 1:open bcd, 2:open gbd, 3:open ssl, 4:open ved, 5:open bcd + ssl + ved

        sample_audio_encoder:
            功能描述：audio 编码
            命令格式：./sample_audio_encoder [infile samplerate] [infile channels] [infile] [outfile]
                      infile samplerate:      input file samplerate
                      infile channels:        input file channels
                      infile:                 input file path for aenc
                      outfile:                .g711a/.g711u/.g726/.aac/.mp3
        sample_audio_decoder:
            功能描述：audio 解码
            命令格式：./sample_audio_decoder [mode] [infile samplerate] [infile]
                      mode :   0:save file 1:send ao 2:bind ao 3: drop
                      in_rate: input file samplerate
                      in_file: input file for adec(.g711a/.g711u/.g726/.aac/.mp3)
        sample_audio_doubletalk_v1/v2:
            功能描述：audio 语音双讲
            命令格式：./sample_audio_doubletalk_v1 [samplerate] [usr_samplerate] [channels] [binder_mode] [outfile] [infile] [infile samplerate]
                      dev_rate:  dev samplerate(8000/16000/32000/48000)
                      usr_rate:  out file samplerate(8000/16000/32000/48000
                      dev_chn:   dev chn cnt(1/2/8)
                      mode:      binder mode(0/1)
                      out_file:  aenc encoder out file(.g711a/.g711u/.g726/.aac/.mp3/.pcm)
                      in_file:   input file for adec(.g711a/.g711u/.g726/.aac/.mp3/.pcm)
                      file_rate: input file samplerate for voice
        sample_audio_record_v1:
            功能描述：audio 语音录制
            命令格式：./sample_audio_record_v1 [samplerate] [usr_samplerate] [channels] [binder_mode] [codec_input_vol] [outfile] [vqe_type]
                      dev_rate:  dev samplerate(8000/16000/32000/48000)
                      usr_rate:  out file samplerate(8000/16000/32000/48000)
                      dev_chn:   dev chn cnt(1/2/8);
                      mode:      binder mode(0/1)
                      out_file:  aenc encoder out file(.g711a/.g711u/.g726/.aac/.mp3/.pcm/.adpcm)
                      vqe_type: 0:only open vqe v1, 1:open v1 + bcd, 2:open v1 + gbd, 3:open v1 + bcd + ved
        sample_audio_record_v2:
            功能描述：audio 语音录制
            命令格式：./sample_audio_record_v2 [samplerate] [usr_samplerate] [channels] [binder_mode] [codec_input_vol] [outfile]  [detect_type]
                      dev_rate:  dev samplerate(8000/16000/32000/48000)
                      usr_rate:  out file samplerate(8000/16000/32000/48000)
                      dev_chn:   dev chn cnt(1/2/8);
                      mode:      binder mode(0/1)
                      out_file:  aenc encoder out file(.g711a/.g711u/.g726/.aac/.mp3/.pcm/.adpcm)
                      detect_type: 0:only open vqe v2, 1:open v2 + bcd, 2:open v2 + gbd, 3:open v2 + bcd + ved + ssl

    sample_cipher用例：
        功能描述：加密接口使用流程演示,具体参见目录下的readme.txt文件
        命令格式：./sample_cipher [index]
                  index:
                  1: CIPHER
                  2: CIPHER-MUTILT
                  3: CIPHER-EFUSEcd
                  4: HASH
                  5: RNG
                  6: RSA-SIGN
                  7: RSA-ENC
                  8: CIPHER-VIA

    npu用例：
        功能描述：npu算法调用流程演示
        命令格式：请参见目录下的README.md和demo_ai目录下的readme.txt
        注意：需要准备相应的模型文件

    scene_auto：
        功能描述：sensor自适应
        命令格式：具体参考scene_auto目录下的readme.txt

    sample_fpn：
        功能描述：fpn标定示例
        命令格式：./sample_fpn [index]
                  0 vi离线,fpn标定
        注意：需要搭配pqtool使用，先运行 ./sample_fpn 0 ，在开始标定前需要通过pqtool设置ae和blc参数，并用镜头盖盖上镜头

    sample_aov:
        功能描述：aov样例demo
        命令格式：./sample_aov
        注意：具体使用及参数配置请参考《AOV软件使用指南》

    sample_pm：
        功能描述：待机唤醒示例
        命令格式：./sample_pm [count]
                  count:  执行待机唤醒次数，默认1000000次

    pq_bin:
        功能描述：导入预制的isp参数文件，或导出当前板端的isp参数文件
        命令格式：./pq_bin [pipe] [file]
                  pipe:     vi pipe.
                  file:     bin data file path.
        e.g:
                  export: ./pq_bin 0
                  import: ./pq_bin 0 PQBin_xxx.bin

    quickstart:
        功能描述：快启Linux侧的演示用例
        命令格式：具体请参考《快启使用指南》

    uvc_app:
        功能描述：作为uvc摄像头演示用例
        命令格式：./uvc_app
        注意：具体操作流程请参考《UVC UAC 使用指南》