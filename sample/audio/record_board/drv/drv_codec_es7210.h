#ifndef __DRV_CODEC_ES7210_H__
#define __DRV_CODEC_ES7210_H__

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <fcntl.h>

typedef void *codec_handle;
typedef void (*fn_i2c_gpio_map_cb)(void);
typedef void (*fn_i2s_gpio_map_cb)(void);

typedef enum ES7210_CHANNEL_E {
    ES7210_CHANNELS_2 = 2,
    ES7210_CHANNELS_4 = 4,
    ES7210_CHANNELS_6 = 6,
    ES7210_CHANNELS_8 = 8,
    ES7210_CHANNELS_10 = 10,
    ES7210_CHANNELS_12 = 12,
    ES7210_CHANNELS_14 = 14,
    ES7210_CHANNELS_16 = 16,
} ES7210_CHANNEL_E;

typedef enum ES7210_TRACK_MODE_E {
    ES7210_TRACK_NORMAL = 0,
    ES7210_TRACK_BOTH_LEFT,
    ES7210_TRACK_BOTH_RIGHT,
    ES7210_TRACK_EXCHANGE,
} ES7210_TRACK_MODE_E;

typedef enum ES7210_SAMPLE_RATE_E {
    ES7210_SAMPLE_RATE_8000   = 8000,
    ES7210_SAMPLE_RATE_12000  = 12000,
    ES7210_SAMPLE_RATE_11025  = 11025,
    ES7210_SAMPLE_RATE_16000  = 16000,
    ES7210_SAMPLE_RATE_22050  = 22050,
    ES7210_SAMPLE_RATE_24000  = 24000,
    ES7210_SAMPLE_RATE_32000  = 32000,
    ES7210_SAMPLE_RATE_44100  = 44100,
    ES7210_SAMPLE_RATE_48000  = 48000,
    ES7210_SAMPLE_RATE_64000  = 64000,
    ES7210_SAMPLE_RATE_96000  = 96000,
} ES7210_SAMPLE_RATE_E;

typedef enum ES7210_BIT_WIDTH_E {
    ES7210_BIT_WIDTH_24  = 0,
    ES7210_BIT_WIDTH_20  = 1,
    ES7210_BIT_WIDTH_18  = 2,
    ES7210_BIT_WIDTH_16  = 3,
    ES7210_BIT_WIDTH_32  = 4,
} ES7210_BIT_WIDTH_E;

typedef enum ES7210_I2S_MODE_E {
    ES7210_SLAVER = 0,
    ES7210_MASTER = 1
} ES7210_I2S_MODE_E;

typedef enum ES7210_I2S_FORMAT_E {
    ES7210_I2S_STD = 0,             /** i2s standard mode, only transfer two channels data */
    ES7210_I2S_LJ,                  /** left justified mode, only transfer two channels data */
    ES7210_DSPA,                    /** pcm mode A */
    ES7210_DSPB,                    /** pcm mode B */
    ES7210_TDM_I2S,                 /** tdm i2s mode */
    ES7210_TDM_LJ,                  /** tdm left justified mode */
    ES7210_TDM_DSPA,                /** tdm pcm mode A */
    ES7210_TDM_DSPB,                /** tdm pcm mode B */
    ES7210_TDM_NLRCK_I2S,
    ES7210_TDM_NLRCK_LJ,
    ES7210_TDM_NLRCK_DSPA,
    ES7210_TDM_NLRCK_DSPB,
} ES7210_I2S_FORMAT_E;

typedef enum ES7210_CHIPFLAG_E {
    ES7210_LAST = 0,                /* TMD级联模式，最后一片codec */
    ES7210_NO_LAST,                 /* TMD级联模式，不是最后一片codec */
} ES7210_CHIPFLAG_E;

typedef enum ES7210_CHIP_NUMBER_E {
    ES7210_CHIP_NUMBER_0 = 0,
    ES7210_CHIP_NUMBER_1,
    ES7210_CHIP_NUMBER_2,
    ES7210_CHIP_NUMBER_3,
} ES7210_CHIP_NUMBER_E;

typedef enum ES7210_VDDA_E {
    ES7210_VDDA_3V3 = 0x00,
    ES7210_VDDA_1V8 = 0x01,
} ES7210_VDDA_E;

typedef enum ES7210_VOL_TYPE_E {
    ES7210_PGA_VOL = 0,
    ES7210_ADC_VOL,
} ES7210_VOL_TYPE_E;

typedef enum ES7210_MCLK_SOURCE_E {
    ES7210_MCLK_SOURCE_DEFAULT = 0,
    ES7210_MCLK_SOURCE_BCLK = 1,
} ES7210_MCLK_SOURCE_E;

typedef enum ES7210_BCLK_INV_MODE_E {
    ES7210_BCLK_NORMAL = 0,
    ES7210_BCLK_INV    = 1,
} ES7210_BCLK_INV_MODE_E;

typedef enum ES7210_MICBIAS_E {
    ES7210_MICBIAS_2V18 = 0,
    ES7210_MICBIAS_2V26,
    ES7210_MICBIAS_2V36,
    ES7210_MICBIAS_2V45,
    ES7210_MICBIAS_2V55,
    ES7210_MICBIAS_2V66,
    ES7210_MICBIAS_2V78,
    ES7210_MICBIAS_2V87,
} ES7210_MICBIAS_E;

typedef struct {
    ES7210_TRACK_MODE_E adc12;
    ES7210_TRACK_MODE_E adc34;
} ES7210_TRACK_MODE_S;


typedef struct {
    char dev_node[128];                     /** i2c驱动节点描述符 */
    unsigned int addr;                      /** i2c地址 */
    fn_i2c_gpio_map_cb i2c_gpio_map_cb;     /** i2c管脚映射回调函数 */
} ES7210_I2C_CFG_S;

typedef struct {
    unsigned int mclk;
    ES7210_I2S_MODE_E mode;                 /** 主从模式配置 */
    ES7210_I2S_FORMAT_E format;             /** 数据传输格式配置 */
    ES7210_MCLK_SOURCE_E mclk_source;       /** MCLK时钟源 */
    ES7210_BCLK_INV_MODE_E bclk_inv_mode;   /** BCLK极性是否反转 */
    fn_i2s_gpio_map_cb i2s_gpio_map_cb;     /** i2s管脚映射回调函数 */
} ES7210_I2S_CFG_S;

typedef struct {
    ES7210_VDDA_E vdda;                     /** vdda必须配置和io电压相等，否则会出现i2s读数据出错 */
    ES7210_MICBIAS_E micbias;               /** 硅麦保持默认即可，驻极体麦克风根据硬件配置 */
    ES7210_CHIP_NUMBER_E chip_number;       /** 芯片级联时配置芯片编号 */
    ES7210_CHIPFLAG_E chip_flag;            /** 芯片级联时是否为最后一片 */

    ES7210_I2C_CFG_S i2c_cfg;               /** i2c 配置 */
    ES7210_I2S_CFG_S i2s_cfg;               /** i2s 配置 */

    ES7210_CHANNEL_E channels;              /** 声道 */
    ES7210_SAMPLE_RATE_E samplerate;        /** 采样率 */
    ES7210_BIT_WIDTH_E bitwidth;            /** 位宽 */
    ES7210_TRACK_MODE_S track_mode;         /** 声道模式 */
} ES7210_CODEC_CFG_S;

int es7210_init(codec_handle *h_codec, ES7210_CODEC_CFG_S *cfg);
int es7210_deinit(codec_handle *h_codec);
int es7210_set_volume(codec_handle *h_codec, unsigned int vol);

#endif