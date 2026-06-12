#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>

#include "drv_codec_es7210.h"
#include "drv_i2c.h"

/** 寄存器 */
#define RESET_CONTROL               0x00
#define CLOCK_CONTROL               0x01
#define MAIN_CLOCK_CONTROL          0x02
#define MASTER_CLOCK_CONTROL        0x03
#define MASTER_LRCK_DIV1            0x04
#define MASTER_LRCK_DIV0            0x05
#define POWER                       0x06
#define ADC_OSR_CFG                 0x07
#define MODE_CFG                    0x08
#define CHIP_INIT_TIME_CONTROL0     0x09
#define CHIP_INIT_TIME_CONTROL1     0x0A
#define CHIP_STATUS                 0x0B
#define CHIP_INTERRUPT_CONTROL      0x0C
#define MISR_CONTROL                0x0D
#define DMIC_CONTROL                0x10
#define SDP_INTERFACE_CFG1          0x11
#define SDP_INTERFACE_CFG2          0x12
#define ADC12_CONTROL               0x15
#define ADC34_CONTROL               0x14
#define ALC_SELECT                  0x16
#define ALC_COMMON_CFG1             0x17
#define ALC_COMMON_CFG2             0x1A
#define ADC34_ALC_LEVEL             0x18
#define ADC12_ALC_LEVEL             0x19
#define ADC4_MAX_GAIN               0x1B
#define ADC3_MAX_GAIN               0x1C
#define ADC2_MAX_GAIN               0x1D
#define ADC1_MAX_GAIN               0x1E
#define ADC34_HPF2                  0x20
#define ADC34_HPF1                  0x21
#define ADC12_HPF1                  0x22
#define ADC12_HPF2                  0x23
#define CHIP_ID1                    0x3D
#define CHIP_ID0                    0x3E
#define CHIP_VERSION                0x3F
#define CHIP_ANALOG_SYSTEM          0x40
#define MIC12_BIAS                  0x41
#define MIC34_BIAS                  0x42
#define MIC1_GAIN                   0x43
#define MIC2_GAIN                   0x44
#define MIC3_GAIN                   0x45
#define MIC4_GAIN                   0x46
#define MIC1_LP                     0x47
#define MIC2_LP                     0x48
#define MIC3_LP                     0x49
#define MIC4_LP                     0x4A
#define MIC12_PD                    0x4B
#define MIC34_PD                    0x4C

typedef enum MIC_GAIN {
    GAIN_0_DB = 0,
    GAIN_3_DB,
    GAIN_6_DB,
    GAIN_9_DB,
    GAIN_12_DB,
    GAIN_15_DB,
    GAIN_18_DB,
    GAIN_21_DB,
    GAIN_24_DB,
    GAIN_27_DB,
    GAIN_30_DB,
    GAIN_33_DB,
    GAIN_34_5_DB,
    GAIN_36_DB,
    GAIN_37_5_DB,
} MIC_GAIN_E;

typedef struct {
    unsigned int ratio;
    unsigned char main_clock_control_value;
    unsigned char main_clock_control_double_speed_value;
    unsigned char adc_osr_cfg_value;
} ES7210_MCLKFS_RATIO_CFG_S;

static const ES7210_MCLKFS_RATIO_CFG_S es7210_mclkfs_ratio_cfg[] = {
    //Ratio     main_clock_control_v    main_clock_control_ds_v     adc_osr_cfg_v
    {64,        0x41,                   0x01,                       0x20},  // 512fs
    {128,       0x01,                   0xC1,                       0x20},
    {192,       0x43,                   0x03,                       0x20},
    {256,       0xC1,                   0x81,                       0x20},
    {384,       0x03,                   0xC3,                       0x20},
    {400,       0xC1,                   0x81,                       0x32},  // 800fs
    {512,       0x81,                   0x82,                       0x20},
    {600,       0x05,                   0xC5,                       0x1E},  // 480fs
    {768,       0xC3,                   0x83,                       0x20},
    {800,       0x81,                   0x82,                       0x32},
    {1024,      0x82,                   0x84,                       0x20},
    {1200,      0xC5,                   0x85,                       0x1E},
    {1500,      0x4F,                   0x0F,                       0x32},
    {1600,      0x82,                   0x84,                       0x32},
    {2048,      0x84,                   0x88,                       0x20},
    {2400,      0x85,                   0x8A,                       0x1E},
    {3000,      0x0F,                   0xCF,                       0x32},
};

typedef struct {
    int         i2c_fd;
    ES7210_CHIP_NUMBER_E chip_number;
} codec_state_s;


#define ES7210_MAX_GAIN         (68)

#define BCLK_DIV(mclk, sr, ch, bw) (mclk / sr / ch / bw)

static int open_i2c_fd(ES7210_I2C_CFG_S i2c_cfg)
{
    int ret;
    int i2c_fd;

    i2c_fd = open(i2c_cfg.dev_node, O_RDWR);
    if (i2c_fd < 0) {
        printf("open %s failed\n", i2c_cfg.dev_node);
        return -1;
    }

    ret = ioctl(i2c_fd, I2C_SLAVE, i2c_cfg.addr);
    if (ret != 0) {
        printf("set i2c slave addr(0x%x) failed\n", i2c_cfg.addr);
        return -1;
    }
    return i2c_fd;
}

static int es7210_i2c_write(codec_handle *h_codec, unsigned int addr, unsigned int data)
{
    int ret;
    unsigned char buf[2];

    codec_state_s *state = (codec_state_s*)h_codec;

    if (state->i2c_fd < 0) {
        printf("not init i2c fd\n");
        return -1;
    }

    buf[0] = addr;
    buf[1] = data;

    ret = write(state->i2c_fd, buf, 2);
    if (ret < 0) {
        printf("i2c fd: %d write addr 0x%x data 0x%x failed\n", state->i2c_fd, addr, data);
        return -1;
    }
    // printf("i2c fd: %d write addr 0x%x data 0x%x\n", state->i2c_fd, addr, data);

    return 0;
}

static void close_i2c_fd(int fd)
{
    if (fd) {
        close(fd);
    }
}

static int get_mclkfs_ratio_cfg(int ratio)
{
    for (int i = 0; i < sizeof(es7210_mclkfs_ratio_cfg); i++) {
        if (es7210_mclkfs_ratio_cfg[i].ratio == ratio) {
            return i;
        }
    }
    return -1;
}

static unsigned int get_bit_width(ES7210_BIT_WIDTH_E bit_width)
{
    unsigned int cur_bit_width;

    switch (bit_width) {
        case ES7210_BIT_WIDTH_16:
            cur_bit_width = 16;
            break;
        case ES7210_BIT_WIDTH_18:
            cur_bit_width = 18;
            break;
        case ES7210_BIT_WIDTH_20:
            cur_bit_width = 20;
            break;
        case ES7210_BIT_WIDTH_24:
            cur_bit_width = 24;
            break;
        case ES7210_BIT_WIDTH_32:
            cur_bit_width = 32;
            break;
        default:
            cur_bit_width = 16;
            break;
    }

    return cur_bit_width;
}

static int es7210_master_clk_ctl(codec_handle *handle, int mclk, unsigned int samplerate, unsigned int double_speed)
{
    int index;
    unsigned int ratio = 0;

    ratio = mclk / samplerate;

    es7210_i2c_write(handle, MASTER_LRCK_DIV1, ((ratio >> 8) & 0xFF));
    es7210_i2c_write(handle, MASTER_LRCK_DIV0, (ratio & 0xFF));

    index = get_mclkfs_ratio_cfg(ratio);
    if (index == -1) {
        printf("get mclkfs ratio failed\n");
        return -1;
    }
    es7210_i2c_write(handle, MAIN_CLOCK_CONTROL, es7210_mclkfs_ratio_cfg[index].main_clock_control_value);
    if (double_speed) {
        es7210_i2c_write(handle, MAIN_CLOCK_CONTROL, es7210_mclkfs_ratio_cfg[index].main_clock_control_double_speed_value);
    }
    es7210_i2c_write(handle, ADC_OSR_CFG, es7210_mclkfs_ratio_cfg[index].adc_osr_cfg_value);
    return 0;
}

static int check_parameters(ES7210_CODEC_CFG_S cfg)
{
    if (cfg.vdda < ES7210_VDDA_1V8 && cfg.vdda > ES7210_VDDA_3V3) {
        printf("check para vdda config error\n");
        return -1;
    }

    if (cfg.micbias < ES7210_MICBIAS_2V18 && cfg.micbias > ES7210_MICBIAS_2V87) {
        printf("check para micbiad config error\n");
        return -1;
    }

    if (cfg.chip_number < ES7210_CHIP_NUMBER_0 && cfg.chip_number > ES7210_CHIP_NUMBER_3) {
        printf("check para chip_number config error\n");
        return -1;
    }

    if (cfg.chip_flag < ES7210_LAST && cfg.chip_flag > ES7210_NO_LAST) {
        printf("check para chip_flag config error\n");
        return -1;
    }

    if (cfg.i2s_cfg.mode != ES7210_SLAVER && cfg.i2s_cfg.mode != ES7210_MASTER) {
        printf("check para i2s_cfg.mode config error\n");
        return -1;
    }

    if (cfg.i2s_cfg.format < ES7210_I2S_STD && cfg.i2s_cfg.format > ES7210_TDM_NLRCK_DSPB) {
        printf("check para i2s_cfg.format config error\n");
        return -1;
    }

    if (cfg.i2s_cfg.mclk_source < ES7210_MCLK_SOURCE_DEFAULT && cfg.i2s_cfg.mclk_source > ES7210_MCLK_SOURCE_BCLK) {
        printf("check para i2s_cfg.mclk_source config error\n");
        return -1;
    }

    if (cfg.i2s_cfg.bclk_inv_mode < ES7210_BCLK_NORMAL && cfg.i2s_cfg.bclk_inv_mode > ES7210_BCLK_INV) {
        printf("check para i2s_cfg.bclk_inv_mode config error\n");
        return -1;
    }

    if (cfg.channels < ES7210_CHANNELS_2 && cfg.channels > ES7210_CHANNELS_16) {
        printf("check para channels config error\n");
        return -1;
    }

    if (cfg.samplerate < ES7210_SAMPLE_RATE_8000 && cfg.samplerate > ES7210_SAMPLE_RATE_96000) {
        printf("check para samplerate config error\n");
        return -1;
    }

    if (cfg.bitwidth < ES7210_BIT_WIDTH_24 && cfg.bitwidth > ES7210_BIT_WIDTH_32) {
        printf("check para bitwidth config error\n");
        return -1;
    }

    return 0;
}

int es7210_init(codec_handle *h_codec, ES7210_CODEC_CFG_S *cfg)
{
    int ret = -1;
    int fd = -1;
    unsigned int max_channel = 2;
    unsigned int double_speed = 0;
    unsigned int temp_value = 0x00;
    codec_state_s *state = NULL;

    ES7210_CODEC_CFG_S *init_cfg = cfg;

    ret = check_parameters(*init_cfg);
    if (ret < 0) {
        printf("check parameters failed\n");
        return ret;
    }

    init_cfg->i2c_cfg.i2c_gpio_map_cb();
    init_cfg->i2s_cfg.i2s_gpio_map_cb();

    fd = open_i2c_fd(init_cfg->i2c_cfg);
    if (fd < 0) {
        printf("open i2c failed. fd=%d\n", fd);
        return -1;
    }
    printf("get i2c fd: %d, success.\n", fd);

    state = (codec_state_s*)malloc(sizeof(codec_state_s));
    if (state == NULL) {
        printf("malloc codec_state_s handle failed\n");
        return -1;
    }
    memset(state, 0x00, sizeof(codec_state_s));

    state->i2c_fd = fd;
    state->chip_number = init_cfg->chip_flag;

    *h_codec = state;

    /* reset adc and power down */
    es7210_i2c_write(*h_codec, RESET_CONTROL, 0xff);
    /* reset adc and digital */
    es7210_i2c_write(*h_codec, RESET_CONTROL, 0x32);
    /* chip init period control */
    es7210_i2c_write(*h_codec, CHIP_INIT_TIME_CONTROL0, 0x20);
    es7210_i2c_write(*h_codec, CHIP_INIT_TIME_CONTROL1, 0x10);
    /* adc12 hpf */
    es7210_i2c_write(*h_codec, ADC12_HPF2, 0x2A);
    es7210_i2c_write(*h_codec, ADC12_HPF1, 0x0A | (init_cfg->track_mode.adc12 << 6));
    /* adc34 hpf */
    es7210_i2c_write(*h_codec, ADC34_HPF1, 0x2A);
    es7210_i2c_write(*h_codec, ADC34_HPF2, 0x0A | (init_cfg->track_mode.adc34 << 6));

    /* Mode Config */
    /** 超过48K采样率，必须使能DoubleSpeed才能工作 */
    if (init_cfg->samplerate > ES7210_SAMPLE_RATE_48000) {
        double_speed = 1;
    }
    /** TDM模式下，使用倍频LRCK，才需要配置，其他模式下默认为2即可 */
    if (init_cfg->i2s_cfg.format >= ES7210_TDM_NLRCK_I2S) {
        max_channel = init_cfg->channels;
    }
    temp_value = (max_channel << 3) + (init_cfg->i2s_cfg.bclk_inv_mode << 3) + 0x04 + (double_speed << 1) + init_cfg->i2s_cfg.mode;
    es7210_i2c_write(*h_codec, MODE_CFG, temp_value);

    /* SDP Interface Config */
    switch (init_cfg->i2s_cfg.format) {
        case ES7210_I2S_STD:
            es7210_i2c_write(*h_codec, SDP_INTERFACE_CFG1, (init_cfg->bitwidth << 5) + 0x00);
            es7210_i2c_write(*h_codec, SDP_INTERFACE_CFG2, 0x00);
            break;
        case ES7210_I2S_LJ:
            es7210_i2c_write(*h_codec, SDP_INTERFACE_CFG1, (init_cfg->bitwidth << 5) + 0x01);
            es7210_i2c_write(*h_codec, SDP_INTERFACE_CFG2, 0x00);
            break;
        case ES7210_DSPA:
            es7210_i2c_write(*h_codec, SDP_INTERFACE_CFG1, (init_cfg->bitwidth << 5) + 0x03);
            es7210_i2c_write(*h_codec, SDP_INTERFACE_CFG2, 0x00);
            break;
        case ES7210_DSPB:
            es7210_i2c_write(*h_codec, SDP_INTERFACE_CFG1, (init_cfg->bitwidth << 5) + 0x23);
            es7210_i2c_write(*h_codec, SDP_INTERFACE_CFG2, 0x00);
            break;
        case ES7210_TDM_NLRCK_I2S:
            es7210_i2c_write(*h_codec, SDP_INTERFACE_CFG1, (init_cfg->bitwidth << 5) + 0x00);
            es7210_i2c_write(*h_codec, SDP_INTERFACE_CFG2, 0x03);
            es7210_i2c_write(*h_codec, SDP_INTERFACE_CFG2, (init_cfg->chip_flag << 2) + 0x03);
            break;
        case ES7210_TDM_NLRCK_LJ:
            es7210_i2c_write(*h_codec, SDP_INTERFACE_CFG1, (init_cfg->bitwidth << 5) + 0x01);
            es7210_i2c_write(*h_codec, SDP_INTERFACE_CFG2, 0x03);
            es7210_i2c_write(*h_codec, SDP_INTERFACE_CFG2, (init_cfg->chip_flag << 2) + 0x03);
            break;
        case ES7210_TDM_NLRCK_DSPA:
            es7210_i2c_write(*h_codec, SDP_INTERFACE_CFG1, (init_cfg->bitwidth << 5) + 0x03);
            es7210_i2c_write(*h_codec, SDP_INTERFACE_CFG2, 0x03);
            es7210_i2c_write(*h_codec, SDP_INTERFACE_CFG2, (init_cfg->chip_flag << 2) + 0x03);
            break;
        case ES7210_TDM_NLRCK_DSPB:
            es7210_i2c_write(*h_codec, SDP_INTERFACE_CFG1, (init_cfg->bitwidth << 5) + 0x23);
            es7210_i2c_write(*h_codec, SDP_INTERFACE_CFG2, 0x03);
            es7210_i2c_write(*h_codec, SDP_INTERFACE_CFG2, (init_cfg->chip_flag << 2) + 0x03);
            break;
        case ES7210_TDM_I2S:
            es7210_i2c_write(*h_codec, SDP_INTERFACE_CFG1, (init_cfg->bitwidth << 5) + 0x00);
            es7210_i2c_write(*h_codec, SDP_INTERFACE_CFG2, 0x02);
            es7210_i2c_write(*h_codec, SDP_INTERFACE_CFG2, (init_cfg->chip_flag << 2) + 0x02);
            break;
        case ES7210_TDM_LJ:
            es7210_i2c_write(*h_codec, SDP_INTERFACE_CFG1, (init_cfg->bitwidth << 5) + 0x01);
            es7210_i2c_write(*h_codec, SDP_INTERFACE_CFG2, 0x02);
            es7210_i2c_write(*h_codec, SDP_INTERFACE_CFG2, (init_cfg->chip_flag << 2) + 0x02);
            break;
        case ES7210_TDM_DSPA:
            es7210_i2c_write(*h_codec, SDP_INTERFACE_CFG1, (init_cfg->bitwidth << 5) + 0x03);
            es7210_i2c_write(*h_codec, SDP_INTERFACE_CFG2, 0x01);
            es7210_i2c_write(*h_codec, SDP_INTERFACE_CFG2, (init_cfg->chip_flag << 2) + 0x01);
            break;
        case ES7210_TDM_DSPB:
            es7210_i2c_write(*h_codec, SDP_INTERFACE_CFG1, (init_cfg->bitwidth << 5) + 0x23);
            es7210_i2c_write(*h_codec, SDP_INTERFACE_CFG2, 0x01);
            es7210_i2c_write(*h_codec, SDP_INTERFACE_CFG2, (init_cfg->chip_flag << 2) + 0x01);
            break;
        default:
            break;
   }

    /* analog sysytem */
    es7210_i2c_write(*h_codec, CHIP_ANALOG_SYSTEM, 0xC3 - (init_cfg->vdda << 5) + (0x0C * init_cfg->vdda));

    /* adc control */
    es7210_i2c_write(*h_codec, ADC34_CONTROL, 0x3C);
    es7210_i2c_write(*h_codec, ADC12_CONTROL, 0x3C);

    /* alc common config 1 */
    es7210_i2c_write(*h_codec, ALC_COMMON_CFG1, 0x00);   // 0.25dB/2LRCK

    /* mic bias config */
    es7210_i2c_write(*h_codec, MIC12_BIAS, (init_cfg->micbias << 4));
    es7210_i2c_write(*h_codec, MIC34_BIAS, (init_cfg->micbias << 4));

    /* mic low power, set PGA low power */
    es7210_i2c_write(*h_codec, MIC1_LP, 0x08);
    es7210_i2c_write(*h_codec, MIC2_LP, 0x08);
    es7210_i2c_write(*h_codec, MIC3_LP, 0x08);
    es7210_i2c_write(*h_codec, MIC4_LP, 0x08);

    /* default ADC PGA Gain = 10dB */
    es7210_i2c_write(*h_codec, MIC1_GAIN, 0x1A);
    es7210_i2c_write(*h_codec, MIC2_GAIN, 0x1A);
    es7210_i2c_write(*h_codec, MIC3_GAIN, 0x1A);
    es7210_i2c_write(*h_codec, MIC4_GAIN, 0x1A);

    /* default ADC Gain = 0dB */
    es7210_i2c_write(*h_codec, ADC4_MAX_GAIN, 0xBF);
    es7210_i2c_write(*h_codec, ADC3_MAX_GAIN, 0xBF);
    es7210_i2c_write(*h_codec, ADC2_MAX_GAIN, 0xBF);
    es7210_i2c_write(*h_codec, ADC1_MAX_GAIN, 0xBF);

    /* BLCK DIV */
    temp_value = BCLK_DIV(init_cfg->i2s_cfg.mclk, init_cfg->samplerate, (init_cfg->channels * 2), get_bit_width(init_cfg->bitwidth));
    if (temp_value > 127) {
        printf("BCLK DIV is too lager. div=%d\n", temp_value);
        goto EXIT;
    }
    printf("mclk: %d, sr: %d, ch: %d, bw: %d, div: %d(0x%x)\n",
            init_cfg->i2s_cfg.mclk, init_cfg->samplerate, init_cfg->channels, get_bit_width(init_cfg->bitwidth), temp_value, temp_value);
    es7210_i2c_write(*h_codec, MASTER_CLOCK_CONTROL, temp_value);

    /* power on */
    es7210_i2c_write(*h_codec, POWER, 0x00);

    if (init_cfg->i2s_cfg.mode == ES7210_MASTER) {
        /* master mode config clk div */
        es7210_master_clk_ctl(*h_codec, init_cfg->i2s_cfg.mclk, init_cfg->samplerate, double_speed);
    }

    // /* mic power down */
    es7210_i2c_write(*h_codec, MIC12_PD, 0x0F);
    es7210_i2c_write(*h_codec, MIC34_PD, 0x0F);

    if (init_cfg->i2s_cfg.mode == ES7210_MASTER) {
        es7210_i2c_write(*h_codec, CLOCK_CONTROL, 0x40);
        es7210_i2c_write(*h_codec, RESET_CONTROL, 0x71);
        es7210_i2c_write(*h_codec, RESET_CONTROL, 0x01);
        es7210_i2c_write(*h_codec, MODE_CFG,
            (init_cfg->channels << 3) + (init_cfg->i2s_cfg.bclk_inv_mode << 3) + 0x04 + (double_speed << 1) + init_cfg->i2s_cfg.mode);
    }

    if (init_cfg->i2s_cfg.mode == ES7210_SLAVER) {
        es7210_i2c_write(*h_codec, RESET_CONTROL, 0x71);
        es7210_i2c_write(*h_codec, RESET_CONTROL, 0x41);
    }

    return 0;

EXIT:
    free(state);
    return ret;
}

int es7210_deinit(codec_handle *h_codec)
{
    codec_state_s *state = (codec_state_s*)h_codec;

    if (state == NULL) {
        printf("codec handle is NULL, don't deinit\n");
        return -1;
    }

    close_i2c_fd(state->i2c_fd);

    memset(state, 0x00, sizeof(codec_state_s));
    free(state);

    return 0;
}

int es7210_set_volume(codec_handle *h_codec, unsigned int vol)
{
    unsigned int pga_gain = 0;
    unsigned int adc_gain = 0;

    if (!h_codec) {
        printf("handle is not init\n");
        return -1;
    }

    if (vol < 0 && vol > ES7210_MAX_GAIN) {
        printf("set volume value error\n");
        return -1;
    }
#if 1
    if (vol > 36) {
        pga_gain = 36;
        adc_gain = vol - 36;
    } else {
        pga_gain = vol;
    }
#else
	if (vol >= 18) {
		pga_gain = 18;
		adc_gain = vol - 18;
	} else {
		pga_gain = vol;
	}
#endif
    es7210_i2c_write(h_codec, MIC1_GAIN, 0x10 + (pga_gain / 3));
    es7210_i2c_write(h_codec, MIC2_GAIN, 0x10 + (pga_gain / 3));
    es7210_i2c_write(h_codec, MIC3_GAIN, 0x10 + (pga_gain / 3));
    es7210_i2c_write(h_codec, MIC4_GAIN, 0x10 + (pga_gain / 3));

    /** ADC Gain Step 0.5dB, range: [-95.5, 32]: [0x00, 0xFF], 0xBF=0dB*/
    es7210_i2c_write(h_codec, ADC4_MAX_GAIN, 0xBF + (adc_gain << 1));
    es7210_i2c_write(h_codec, ADC3_MAX_GAIN, 0xBF + (adc_gain << 1));
    es7210_i2c_write(h_codec, ADC2_MAX_GAIN, 0xBF + (adc_gain << 1));
    es7210_i2c_write(h_codec, ADC1_MAX_GAIN, 0xBF + (adc_gain << 1));

    printf("set es7210 vol(%d) success\n", vol);

    return 0;
}
