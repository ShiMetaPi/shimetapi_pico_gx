各算子功能说明:
    1. sample_ive_sobel:
        --功能描述:5*5模板sobel-like梯度计算任务。
        --数据流:file->IVE->file
        --主要参数:
            (1) en_out_ctrl:sobel输出控制
                0:同时输出用模板和转置模板滤波的结果
                1:仅输出用模板直接滤波的结果
                2:仅输出用转置模板滤波的结果
            (2) as8_mask[25]:5*5模板系数

    2. sample_ive_edge:
        --功能描述:S16数据到8bit数据的阈值化任务
        --数据流:file->IVE->file
        --主要参数:
                (1) en_out_ctrl:梯度幅值与角度计算的输出格式
                    0:仅输出幅值
                    1:同时输出幅值和角度值
                (2) en_mode:定义16bit无符号图像的阈值化模式
                     0:像素值小于u16_low_thr、在u16_low_thr和u16_high_thr中间、大于u16_high_thr,分别阈值化输出为u8_min_val、
                                u8_mid_val、u8_max_val
                     1:像素值小于u16_low_thr、在u16_low_thr和u16_high_thr中间、大于u16_high_thr分别阈值化为u8_min_val、
                                原值、u8_max_val

    3. sample_ive_gmm2:
        --功能描述:创建GMM背景建模任务,支持1-5个高斯模型,支持灰度图和RGB_PACKAGE图输入,
                支持全局及像素级别的灵敏度系数以及前景模型时长更新系数。
        --数据流:file->IVE->file
        --主要参数:
            (1) en_sns_factor_mode:灵敏度模式,分为两种
                0:全局模式(默认),当选择全局模式时使用参数 (12)u8_glb_sns_factor 作为背景建模的灵敏度系数;
                1:像素模式,当使用像素模式时使用pstFactor中的低8位为灵敏度系数。在初始化时初始化为8。
            (2) en_life_update_factor_mode:前景模型时长更新的模式
                0:全局模式(默认),在该模式下使用参数(3)u16_glb_life_update_factor,作为前景模型更新参数。
                1:像素模式,使用pstFactor中的高8位为前景模型时长更新参数,初始化时设置为4。
                前景模型的更新时长参数控制某一个像素被识别为前景的时长,在处理静止或慢速移动前景时具有重要意义。
            (3) u16_glb_life_update_factor:全局模型更新参数,范围: 0-65535,默认: 4
            (4) u16_life_thr:背景模型生成时间, 表示一个模型从前景模型转成背景模型需要的时间,范围: 0-65535,默认: 5000
            (5) u16_freq_init_val:初始频率,范围: 0-65535,默认: 20000。
            (6) u16_freq_redu_factor:频率衰减系数,范围: 0-65535,默认: 0xFF00
            (7) u16_freq_add_factor:模型匹配频率增加系数,范围: 0-65535,默认:0xEF。
            (8) u16_freq_thr:模型失效频率阈值,范围: 0-65535,默认:12000。
            (9) u16_var_rate:方差更新率,范围: 0-65535,默认: 1。
            (10) u9q7_max_var:方差最大值,范围: 0-65535,默认: (16*16)<<7。
            (11) u9q7_min_var:方差最小值,范围: 0-方差最大值,默认:(8*8)<<7
            (12) u8_glb_sns_factor:全局灵敏度参数,范围: 0-255,默认: 8。
            (13) u8_model_num:模型数量,范围 1-5,默认: 3


    4. sample_ive_test_memory:
        --功能描述:实现内存测试。
        --数据流:file->IVE->file

    5. sample_ive_md:
        --功能描述:移动检测
        --数据流:VI->VPSS->IVE->VGS->VO_HDMI
        --主要参数:
            (1) en_alg_mode:移动检测算法模式。默认:背景法,XMEDIA_MD_ALG_MODE_BG。
            (2)xmedia_ive_sad_ctrl_s:SAD图计算时的相关参数
                1)en_sad_mode:sad图的计算模式。
                    0:按4*4像素块计算 SAD图
                    1:按8*8像素块计算 SAD图
                    2:按16*16像素块计算 SAD图
                2)xmedia_ive_sad_out_ctrl_e: SAD 输出控制模式。
                    0:按16bit的SAD图和阈值化后的图共同输出。
                    1:按8bit的SAD图和阈值化后的图共同输出。
                    2:仅输出16bitSAD图。
                    3:仅输出8bitSAD图。
                    4:仅输出阈值化后的图。
                3)u16_sad_thr: 对计算的SAD图进行阈值化的阈值,大于该阈值取值为u8_max_val,小于该阈值输出为u8_min_val。
                               该参数受输出模式控制取值范围不同:SAD输出控制模式为1时取值为[0,255],输出模式为0或4时取值为[0,65535]。
            (3) xmedia_ive_ccl_ctrl_s:连通域计算时的相关参数
                1)xmedia_ive_ccl_mode_e:连通域模式
                    0:4-连通
                    1:8-连通
                2)u16_init_area_thr:初始面积阈值,当得到的连通域面积大于该阈值时标记为有效连通域,最多标记254个有效连通域。
                                    当连通域数目较多,会按3)面积阈值步长增大u16_init_area_thr,进一步筛选连通域。
                3)u16_step :面积阈值增长步长
        --参数影响:
            en_sad_mode: sad图的计算模式。计算的像素块越小越能检测细微的运动和小目标,但更容易受噪声影响,
                         并且在计算时需要处理更多的像素块,计算量更大,更适合需要检测细微变化的运动场景。
            u16_sad_thr: 该参数影响在MD检测时对移动目标的识别。参数的增大可以有效降低 由噪声、光照变化、相机抖动等干扰造成的误检,
                         能够检测出像素变化较大的运动。但值过高可能会导致缓慢移动的目标或者与背景颜色相似的物体被漏掉。
            xmedia_ive_ccl_mode_e: 该参数会影响MD检测时检测到的目标的数量。4连通相较于8连通对噪声更加敏感,
                                   单个像素的缺失就可能导致连通域断裂,从而可能漏掉小目标或将某个大目标分割成多个小目标。
            u16_init_area_thr: 该参数影响移动检测时目标的识别。阈值过小会导致噪声无法过滤,检测结果包含大量的散点。
                               阈值过大会导致对小目标、小幅度的运动不敏感,造成漏检。

    6. sample_ive_canny:
        --功能描述:canny边缘检测
        --数据流:file->IVE->file

    7. sample_ive_od:
        --功能描述:遮挡检测
        --数据流:VI->VPSS->IVE->VGS->VENC
        --主要参数:
            (1) IVE_CHAR_CALW: 将积分图化分成IVE_CHAR_CALW * IVE_CHAR_CALH 个像素块,默认为:8。
            (2) IVE_CHAR_CALH: 将积分图化分成IVE_CHAR_CALW * IVE_CHAR_CALH 个像素块,默认为:8。
            (3) s32_thresh_num: 满足约束条件的像素块大于该阈值判定为遮挡。默认为:1/2。

    8. sample_ive_st_lk:
        --功能描述:光流金字塔追踪
        --数据流:file->IVE->file
        --主要参数:
            (1) xmedia_ive_lk_optical_flow_pyr_ctrl_s: 金字塔LK光流计算的控制参数
                1) b_use_init_flow: 是否使用初始光流计算的控制参数。
                2) u16_pts_num: 光流金字塔中第0层,包括当前帧和前一帧的特征点的个数,也就是pstPrevPts、pstNextPts中特征点个数,
                                同时也是pstStatus 和 pstErr 的数组大小。取值范围:[1, 500]。默认:500
                3) u8_max_level: u8_max_level+1 为金字塔层数。取值范围为[0,3],默认为:3
                4) u0q8_min_eigthr:最小特征值阈值,取值范围为[1,255]。用于在光流检测前判断一个像素点是否适合被跟踪。
                5) u8_iter_cnt: 最大迭代次数。取值范围:[1,20]。
                6) u0q8_eps   :收敛条件阈值 dx^2 + dy^2 < u0q8_epsilon 时收敛
            (2) xmedia_ive_st_corner_ctrl_s: 特征角点筛选的控制参数
                1)u16_max_corner_num: 最大角点数目。默认为:500,该值需不大于u16_pts_num
                2)u16_min_dist: 相邻角点最小距离。默认为:5。
            (3) u0q8_quality_level:角点质量控制参数,角点响应值小于u0q8_quality_level 最大角点响应值的点将直接被确认为非角点
        --主要参数影响:
            (1) u8_max_level:构建光流金字塔的层数,最大值为3对应的层数为4。
                            光流金字塔层数决定了算法处理大位移运动的能力:层数越多,能处理的运动幅度越大,但计算量也相应增加。
            (2)u0q8_min_eigthr: 最小特征阈值主要影响光流跟踪时跟踪点的数量和质量。阈值过高只能跟踪高质量的特征点
            (2) u8_iter_cnt: 限制每次光流计算的最大迭代次数。
            (3) u16_max_corner_num、u16_min_dist、u0q8_quality_level:共同限制特征点的数量与质量。
                u16_max_corner_num限制特征点的数量不超过该值,
                u16_min_dist避免特征点过于密集,距离小于该值的特征点被筛除,
                u0q8_quality_level用于筛选出质量高的显著特征点。
                当这三共同作用时可以提高特征点的质量,筛选掉大部分特征点,使得光流检测速度变快,但是设置过量有可能会丢失关键特征点。

    9. sample_ive_dma:
        --功能描述:使用dma实现crop和padding
        --数据流:file->IVE->file
        --主要参数
            (1) crop实现参数
                1) crop_dst_w、crop_dst_h:按需修改crop目标区域的宽、高。
                2) start_x、start_y: 从原图中的该位置处开始crop。
            (2) padding实现参数
                1) padding_dst_w、padding_dst_h:padding到目标图片的宽高
                2) dst_start_x、dst_start_y :指定padding到目标图片的初始位置

