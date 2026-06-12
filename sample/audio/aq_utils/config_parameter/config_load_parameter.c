/*
 * Copyright (c) XMEDIA. All rights reserved.
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "config_load_parameter.h"
#include "xmedia_audio_vqe_enhance_v1.h"
#include "xmedia_audio_vqe_enhance_v2.h"
#include "xmedia_audio_vqe_enhance_v3.h"
#include "xmedia_audio_vqe_bcd.h"
#include "xmedia_audio_vqe_detect.h"

#define LEVEL_COEFFICIENT 1

struct config {
    xmedia_char *key;
    xmedia_char *value;
};

struct section {
    xmedia_char *name;
    xmedia_s32 num_configs;
    struct config **configs;
};

struct ini {
    xmedia_s32 num_sections;
    struct section **sections;
};

typedef enum {
    START,
    NEW_SECTION,
    IN_SECTION,
    END_OF_FILE
} States;

struct read_ini {
    xmedia_char *filename;
    FILE *fin;
    xmedia_char *tmp;
    xmedia_s32 tmp_cap;
    xmedia_s32 current_line;
    xmedia_s32 state;
};

struct read_ini;

/* Trims whitespace from the passed in string between begin_ind and
 * end_ind xmedia_characters in the string. It returns the begin index and
 * the end index of the first and last valid (non-whitespace)
 * xmedia_characters in the string. */
static xmedia_void ini_trim(xmedia_char *str, xmedia_s32 *begin_ind, xmedia_s32 *end_ind)
{
    xmedia_s32 b = *begin_ind;
    xmedia_s32 e = *end_ind;
    xmedia_s32 i;

    /* find first index of non-whitespace xmedia_character */
    for (i=b; i<e; i++) {
        xmedia_char c = str[i];

        if (c != ' ' && c != '\t' && c != '\n' && c != '\r' && c != '{') {
            break;
        }
    }
    *begin_ind = i;
    /* find last index of non-whitespace xmedia_character */
    for (i=e-1; i>=b; i--) {
        xmedia_char c = str[i];
        if (c != ' ' && c != '\t' && c != '\n' && c != '\r' && c != '}') {
            break;
        }
    }
    *end_ind = i;
}

static xmedia_s32 ini_read_line(struct read_ini *read_ini)
{
    xmedia_s32 i = 0;
    xmedia_s32 b = 0;
    xmedia_s32 new = 0;

    for (;;) {
        xmedia_char c;

        /* resize buffer if necessary */
        if (i >= read_ini->tmp_cap) {
            read_ini->tmp_cap *= 2;
            read_ini->tmp = realloc(read_ini->tmp, sizeof(*read_ini->tmp) * read_ini->tmp_cap);
        }

        /* read next byte from file */
        b = getc(read_ini->fin);

        /* if end of file then end */
        if (b == EOF) {
            read_ini->state = END_OF_FILE;
            break;
        }

        /* test for end of line */
        c = b;
        if (c == '\r' || c == '\n') {
            read_ini->current_line++;
            break;
        }

        /* place xmedia_char in buffer */
        read_ini->tmp[i++] = c;
        new = 1;
    }

    /* place end-of-string marker */
    if (new) {
        read_ini->tmp[i] = '\0';
    }

    return new;
}

static struct section *ini_parse_section(struct read_ini *read_ini)
{
    xmedia_s32 new = 0;
    struct section *section = malloc(sizeof(*section));
    section->num_configs = 0;
    section->configs = NULL;

    for (;;) {
        switch (read_ini->state) {
            case START:
            case IN_SECTION:
                new = ini_read_line(read_ini);
                break;
            case NEW_SECTION:
                new = 1;
                break;
            case END_OF_FILE:
                return section;
            default:
                printf("invalid state %d in parse_section\n", read_ini->state);
                break;
        }

        if (new) {
            xmedia_s32 i;
            xmedia_s32 x;
            xmedia_s32 y;
            xmedia_s32 b = 0;
            xmedia_s32 e = strlen(read_ini->tmp);
            struct config *cfg;

            ini_trim(read_ini->tmp, &b, &e);

            /* check for comments */
            if (read_ini->tmp[b] == '#' || read_ini->tmp[b] == ';') {
                continue;
            }

            /* check for new section */
            if (read_ini->tmp[b] == '[') {
                if (read_ini->state == IN_SECTION) {
                    read_ini->state = NEW_SECTION;
                    break;
                } else {
                    /* fill section name */
                    read_ini->tmp[e] = '\0';
                    section->name = strdup(&read_ini->tmp[b+1]);

                    read_ini->state = IN_SECTION;
                    continue;
                }
            }

            cfg = malloc(sizeof(*cfg));

            /* read key */
            for (i = b; i <= e; i++) {
                xmedia_char c = read_ini->tmp[i];
                if (c == ':' || c == '=') {
                    break;
                }
            }

            x = b;
            y = i;

            ini_trim(read_ini->tmp, &x, &y);
            read_ini->tmp[y+1] = '\0';
            cfg->key = strdup(&read_ini->tmp[x]);

            /* read value */

            x = i+1;
            y = e+1;
            ini_trim(read_ini->tmp, &x, &y);
            read_ini->tmp[y+1] = '\0';
            cfg->value = strdup(&read_ini->tmp[x]);

            /* add the config to the section */
            if (section->configs) {
                section->num_configs++;
                section->configs = realloc(section->configs, sizeof(*section->configs) * section->num_configs);
            } else {
                section->configs = malloc(sizeof(*section->configs));
                section->num_configs = 1;
            }
            section->configs[section->num_configs-1] = cfg;
        }
    }

    return section;
}

/* parse ini */
static struct ini* ini_parse(struct read_ini *read_ini)
{
    struct ini *ini = malloc(sizeof(*ini));
    xmedia_char finished = 0;

    ini->num_sections = 0;
    ini->sections = NULL;

    while (!finished) {
        switch (read_ini->state) {
        case START:
        case NEW_SECTION:
        {
            /* read a section */
            struct section * section = ini_parse_section(read_ini);

            if (!section) {
                break;
            }

            /* add it to the sections structure */
            if (ini->sections) {
                ini->num_sections++;
                ini->sections = realloc(ini->sections, sizeof(*ini->sections) * ini->num_sections);
            } else {
                ini->sections = malloc(sizeof(*ini->sections));
                ini->num_sections = 1;
            }

            ini->sections[ini->num_sections-1] = section;
            break;
        }
        case END_OF_FILE:
            finished = 1;
            break;
        default:
            printf("error parsing file at line %d\n", read_ini->current_line);
            finished = 1;
            break;
        }
    }

    return ini;
}

/* read ini file */
struct ini* create_ini(xmedia_char *filename)
{
    struct ini *ini = XMEDIA_NULL;
    struct read_ini read_ini;

    read_ini.fin = fopen(filename, "r");
    if (read_ini.fin == NULL) {
        printf("open ini file %s failed\n", filename);
        return NULL;
    }

    read_ini.filename = filename;
    read_ini.current_line = 0;
    read_ini.state = START;
    read_ini.tmp_cap = 4;
    read_ini.tmp = malloc(sizeof(*read_ini.tmp) * 4);
    if (read_ini.tmp == XMEDIA_NULL) {
        goto EXIT;
    }

    ini = ini_parse(&read_ini);

EXIT:
    fclose(read_ini.fin);
    if (read_ini.tmp != XMEDIA_NULL) {
        free(read_ini.tmp);
    }
    return ini;
}

xmedia_void destroy_ini(struct ini* ini)
{
    xmedia_s32 s;
    xmedia_s32 c;
    for (s=0; s<ini->num_sections; s++) {
        for (c=0; c<ini->sections[s]->num_configs; c++) {
            free(ini->sections[s]->configs[c]->key);
            free(ini->sections[s]->configs[c]->value);
            free(ini->sections[s]->configs[c]);
        }
        free(ini->sections[s]->name);
        free(ini->sections[s]->configs);
        free(ini->sections[s]);
    }

    free(ini->sections);
    free(ini);
}



/* This does a linear search through the keys in the parsed ini file
 * so should happen in O(n) time. This could be made better through
 * using hashtables but I don't think this will generally be the
 * bottleneck (how large can ini files get?)
 */
xmedia_char* ini_get_value(struct ini* ini, xmedia_char *section, xmedia_char *key)
{
    xmedia_s32 s, c;

    for (s=0; s<ini->num_sections; s++) {
        if (strcmp(section, ini->sections[s]->name) == 0) {
            for (c=0; c<ini->sections[s]->num_configs; c++) {
                if (strcmp(key, ini->sections[s]->configs[c]->key) == 0) {
                    return ini->sections[s]->configs[c]->value;
                }
            }
        }
    }

    return XMEDIA_NULL;
}


xmedia_s32 ini_get_int_value(struct ini* ini, xmedia_char* section, xmedia_char* key)
{
    const xmedia_char* str;
    str = ini_get_value(ini, section, key);
    if (str == XMEDIA_NULL) {
        return 0;
    }
    return strtol(str, XMEDIA_NULL, 0);
}

/* pretty prxmedia_s32 the structure */
xmedia_void ini_pretty_print(struct ini* ini)
{
    xmedia_s32 s;
    xmedia_s32 c;
    //printf("num sections: %d\n", ini->num_sections);
    for (s=0; s<ini->num_sections; s++) {
        //printf("section: \"%s\" ", ini->sections[s]->name);
        //printf("(num configs: %d)\n", ini->sections[s]->num_configs);
        for (c=0; c<ini->sections[s]->num_configs; c++) {
            //printf("  key: \"%s\", value: \"%s\"\n",
            //       ini->sections[s]->configs[c]->key,
            //       ini->sections[s]->configs[c]->value);
        }
    }
}

xmedia_void get_base_parameter(struct ini* ini, user_base_config *base_config)
{
    // 获取基本参数
    base_config->work_samplerate  = ini_get_int_value(ini, "basic", "sampleRate");
    base_config->in_chn_cnt       = ini_get_int_value(ini, "basic", "inchnnum");
    base_config->out_chn_cnt      = ini_get_int_value(ini, "basic", "outchnnum");
    base_config->sample_per_frame = ini_get_int_value(ini, "basic", "frameSample");
    base_config->frame_nums       = ini_get_int_value(ini, "basic", "frameNums");
    base_config->vqe_version      = ini_get_int_value(ini, "basic", "vqe_version");
    base_config->codec_input_vol  = ini_get_int_value(ini, "volume", "codec_input_vol");
    base_config->ai_vqe_vol       = ini_get_int_value(ini, "volume", "vqe_ai_vol");
    base_config->ao_vqe_vol       = ini_get_int_value(ini, "volume", "vqe_ao_vol");

    printf("get_base_parameter:\n");
    printf("   sampleRate: %d\n", base_config->work_samplerate);
    printf("   in_chn_cnt: %d\n", base_config->in_chn_cnt);
    printf("   out_chn_cnt: %d\n", base_config->out_chn_cnt);
    printf("   sample_per_frame: %d\n", base_config->sample_per_frame);
    printf("   frame_nums: %d\n", base_config->frame_nums);
    printf("   vqe_version: %d\n", base_config->vqe_version);
    printf("   codec_input_vol: %d\n", base_config->codec_input_vol);
    printf("   ao_vqe_vol: %d\n", base_config->ao_vqe_vol);
    printf("   ai_vqe_vol: %d\n", base_config->ai_vqe_vol);
}

xmedia_void get_aivqe_v1_parameter(struct ini* ini, ai_vqe_attr_v1* vqe_config)
{
    memset(vqe_config, 0, sizeof(ai_vqe_attr_v1));
    // 获取模块开启状态
    xmedia_u32 enableOpenMask = 0;

    if (ini_get_int_value(ini, "HPF", "enable")) {
        enableOpenMask |= VQE_V1_MASK_HPF;
        vqe_config->hpf_attr.mode = ini_get_int_value(ini, "HPF", "mode");
        if (vqe_config->hpf_attr.mode == VQE_V1_USR_MODE_USER) { //手动模式
            vqe_config->hpf_attr.freq = ini_get_int_value(ini, "HPF", "freq");
        }
        printf("get ai hpf parameter:\n");
        printf("   mode: %d\n", vqe_config->hpf_attr.mode);
        printf("   freq: %d\n", vqe_config->hpf_attr.freq);
    }

    if (ini_get_int_value(ini, "AEC", "enable")) {
        enableOpenMask |= VQE_V1_MASK_AEC;
        vqe_config->aec_attr.mode = ini_get_int_value(ini, "AEC", "mode");
        if (vqe_config->aec_attr.mode == VQE_V1_USR_MODE_USER) { //手动模式
            vqe_config->aec_attr.cng_enable = ini_get_int_value(ini, "AEC", "cng_enable");
            vqe_config->aec_attr.freq_boundary = ini_get_int_value(ini, "AEC", "freq_boundary");
            vqe_config->aec_attr.high_suppress_level = LEVEL_COEFFICIENT * ini_get_int_value(ini, "AEC", "high_suppress_level");
            vqe_config->aec_attr.low_suppress_level = LEVEL_COEFFICIENT * ini_get_int_value(ini, "AEC", "low_suppress_level");
            vqe_config->aec_attr.low_enhance_voice_protect_level = LEVEL_COEFFICIENT * ini_get_int_value(ini, "AEC", "low_enhance_voice_protect_level");
            vqe_config->aec_attr.high_enhance_voice_protect_level = LEVEL_COEFFICIENT * ini_get_int_value(ini, "AEC", "high_enhance_voice_protect_level");
        } else if (vqe_config->aec_attr.mode == VQE_V1_USR_MODE_ADVANCED) { //高级模式
            //update later
        }
        printf("get ai aec parameter:\n");
        printf("   mode: %d\n", vqe_config->aec_attr.mode);
        printf("   cng_enable: %d\n", vqe_config->aec_attr.cng_enable);
        printf("   freq_boundary: %d\n", vqe_config->aec_attr.freq_boundary);
        printf("   high_suppress_level: 0x%x\n", vqe_config->aec_attr.high_suppress_level);
        printf("   low_suppress_level: 0x%x\n", vqe_config->aec_attr.low_suppress_level);
        printf("   low_enhance_voice_protect_level: 0x%x\n", vqe_config->aec_attr.low_enhance_voice_protect_level);
        printf("   high_enhance_voice_protect_level: 0x%x\n", vqe_config->aec_attr.high_enhance_voice_protect_level);
    }

    if (ini_get_int_value(ini, "ANR", "enable")) {
        enableOpenMask |= VQE_V1_MASK_ANR;
        vqe_config->anr_attr.mode = ini_get_int_value(ini, "ANR", "mode");
        if (vqe_config->anr_attr.mode == VQE_V1_USR_MODE_USER) { //手动模式
            vqe_config->anr_attr.usr_scene = LEVEL_COEFFICIENT * ini_get_int_value(ini, "ANR", "usr_scene");
            vqe_config->anr_attr.nr_mode = LEVEL_COEFFICIENT * ini_get_int_value(ini, "ANR", "nr_mode");
            vqe_config->anr_attr.max_suppress_gain = LEVEL_COEFFICIENT * ini_get_int_value(ini, "ANR", "max_suppress_gain");
            vqe_config->anr_attr.suppress_level = LEVEL_COEFFICIENT * ini_get_int_value(ini, "ANR", "suppress_level");
            vqe_config->anr_attr.nonstationary_suppress_level = LEVEL_COEFFICIENT * ini_get_int_value(ini, "ANR", "nonstationary_suppress_level");
        }
        printf("get ai anr parameter:\n");
        printf("   mode: %d\n", vqe_config->anr_attr.mode);
        printf("   usr_scene: 0x%x\n", vqe_config->anr_attr.usr_scene);
        printf("   nr_mode: %d\n", vqe_config->anr_attr.nr_mode);
        printf("   max_suppress_gain: 0x%x\n", vqe_config->anr_attr.max_suppress_gain);
        printf("   suppress_level: %d\n", vqe_config->anr_attr.suppress_level);
        printf("   nonstationary_suppress_level: 0x%x\n", vqe_config->anr_attr.nonstationary_suppress_level);
    }

    if (ini_get_int_value(ini, "AGC", "enable")) {
        enableOpenMask |= VQE_V1_MASK_AGC;
        vqe_config->agc_attr.mode = ini_get_int_value(ini, "AGC", "mode");
        if (vqe_config->agc_attr.mode == VQE_V1_USR_MODE_USER) { //手动模式
            vqe_config->agc_attr.target_level = ini_get_int_value(ini, "AGC", "target_level");
            vqe_config->agc_attr.max_boost_gain = ini_get_int_value(ini, "AGC", "max_boost_gain");
            vqe_config->agc_attr.noise_floor = ini_get_int_value(ini, "AGC", "noise_floor");
            vqe_config->agc_attr.ratio = ini_get_int_value(ini, "AGC", "ratio");
            vqe_config->agc_attr.attack_time = ini_get_int_value(ini, "AGC", "attack_time");
            vqe_config->agc_attr.release_time = ini_get_int_value(ini, "AGC", "release_time");
        }
        printf("get ai agc parameter:\n");
        printf("   mode: %d\n", vqe_config->agc_attr.mode);
        printf("   target_level: %d\n", vqe_config->agc_attr.target_level);
        printf("   max_boost_gain: %d\n", vqe_config->agc_attr.max_boost_gain);
        printf("   noise_floor: %d\n", vqe_config->agc_attr.noise_floor);
        printf("   ratio: %d\n", vqe_config->agc_attr.ratio);
        printf("   attack_time: %d\n", vqe_config->agc_attr.attack_time);
        printf("   release_time: %d\n", vqe_config->agc_attr.release_time);
    }

    if (ini_get_int_value(ini, "VAD", "enable")) {
        enableOpenMask |= VQE_V1_MASK_VAD;
        vqe_config->vad_attr.mode = ini_get_int_value(ini, "VAD", "mode");
        if (vqe_config->vad_attr.mode == VQE_V1_USR_MODE_USER) { //手动模式
            vqe_config->vad_attr.sensitivity_level = ini_get_int_value(ini, "VAD", "sensitivity_level");
        }
        printf("get ai vad parameter:\n");
        printf("   mode: %d\n", vqe_config->vad_attr.mode);
        printf("   sensitivity_level: %d\n", vqe_config->vad_attr.sensitivity_level);
    }

    if (ini_get_int_value(ini, "DEREVERB", "enable")) {
        enableOpenMask |= VQE_V1_MASK_DEREVERB;
        vqe_config->dereverb_attr.mode = ini_get_int_value(ini, "DEREVERB", "mode");
        if (vqe_config->dereverb_attr.mode == VQE_V1_USR_MODE_USER) { //手动模式
            vqe_config->dereverb_attr.delay_offset = ini_get_int_value(ini, "DEREVERB", "delay_offset");
            vqe_config->dereverb_attr.delay_count = ini_get_int_value(ini, "DEREVERB", "delay_count");
        }
        printf("get ai dereverb parameter:\n");
        printf("   mode: %d\n", vqe_config->dereverb_attr.mode);
        printf("   delay_offset: %d\n", vqe_config->dereverb_attr.delay_offset);
        printf("   delay_count: %d\n", vqe_config->dereverb_attr.delay_count);
    }

    if (ini_get_int_value(ini, "WNS", "enable")) {
        enableOpenMask |= VQE_V1_MASK_WNS;
        vqe_config->wns_attr.mode = ini_get_int_value(ini, "WNS", "mode");
        if (vqe_config->wns_attr.mode == VQE_V1_USR_MODE_USER) { //手动模式
            vqe_config->wns_attr.suppress_level = ini_get_int_value(ini, "WNS", "suppress_level");
        }
        printf("get ai wns parameter:\n");
        printf("   mode: %d\n", vqe_config->wns_attr.mode);
        printf("   suppress_level: %d\n", vqe_config->wns_attr.suppress_level);
    }

    if (ini_get_int_value(ini, "EQ", "enable")) {
        enableOpenMask |= VQE_V1_MASK_EQ;
        vqe_config->eq_attr.gain[0] = ini_get_int_value(ini, "EQ", "gain_0");
        vqe_config->eq_attr.gain[1] = ini_get_int_value(ini, "EQ", "gain_1");
        vqe_config->eq_attr.gain[2] = ini_get_int_value(ini, "EQ", "gain_2");
        vqe_config->eq_attr.gain[3] = ini_get_int_value(ini, "EQ", "gain_3");
        vqe_config->eq_attr.gain[4] = ini_get_int_value(ini, "EQ", "gain_4");
        vqe_config->eq_attr.gain[5] = ini_get_int_value(ini, "EQ", "gain_5");
        vqe_config->eq_attr.gain[6] = ini_get_int_value(ini, "EQ", "gain_6");
        vqe_config->eq_attr.gain[7] = ini_get_int_value(ini, "EQ", "gain_7");
        vqe_config->eq_attr.gain[8] = ini_get_int_value(ini, "EQ", "gain_8");
        vqe_config->eq_attr.gain[9] = ini_get_int_value(ini, "EQ", "gain_9");
        vqe_config->eq_attr.gain[10] = ini_get_int_value(ini, "EQ", "gain_10");
        vqe_config->eq_attr.gain[11] = ini_get_int_value(ini, "EQ", "gain_11");
        vqe_config->eq_attr.gain[12] = ini_get_int_value(ini, "EQ", "gain_12");
        vqe_config->eq_attr.gain[13] = ini_get_int_value(ini, "EQ", "gain_13");
        vqe_config->eq_attr.gain[14] = ini_get_int_value(ini, "EQ", "gain_14");
        vqe_config->eq_attr.gain[15] = ini_get_int_value(ini, "EQ", "gain_15");
        vqe_config->eq_attr.gain[16] = ini_get_int_value(ini, "EQ", "gain_16");
        vqe_config->eq_attr.gain[17] = ini_get_int_value(ini, "EQ", "gain_17");
        vqe_config->eq_attr.gain[18] = ini_get_int_value(ini, "EQ", "gain_18");
        vqe_config->eq_attr.gain[19] = ini_get_int_value(ini, "EQ", "gain_19");
        vqe_config->eq_attr.gain[20] = ini_get_int_value(ini, "EQ", "gain_20");
        vqe_config->eq_attr.gain[21] = ini_get_int_value(ini, "EQ", "gain_21");
        vqe_config->eq_attr.gain[22] = ini_get_int_value(ini, "EQ", "gain_22");
        vqe_config->eq_attr.gain[23] = ini_get_int_value(ini, "EQ", "gain_23");
        vqe_config->eq_attr.gain[24] = ini_get_int_value(ini, "EQ", "gain_24");
        vqe_config->eq_attr.gain[25] = ini_get_int_value(ini, "EQ", "gain_25");
        vqe_config->eq_attr.gain[26] = ini_get_int_value(ini, "EQ", "gain_26");
        vqe_config->eq_attr.gain[27] = ini_get_int_value(ini, "EQ", "gain_27");
        vqe_config->eq_attr.gain[28] = ini_get_int_value(ini, "EQ", "gain_28");
        vqe_config->eq_attr.gain[29] = ini_get_int_value(ini, "EQ", "gain_29");
        printf("get ai eq parameter: enable\n");
        xmedia_s32 i;
        printf("   gain[0-14]: ");
        for (i = 0; i < 15; i++) {
                printf(" %d", vqe_config->eq_attr.gain[i]);
        }
        printf("\n   gain[15-29]: ");
        for (i = 0; i < 15; i++) {
                printf(" %d", vqe_config->eq_attr.gain[15 + i]);
        }
        printf("\n");
    }
    vqe_config->mask = enableOpenMask;
}

xmedia_void get_aivqe_v2_parameter(struct ini* ini, ai_vqe_attr_v2* vqe_config)
{
    memset(vqe_config, 0, sizeof(ai_vqe_attr_v2));
    // 获取模块开启状态
    xmedia_u32 enableOpenMask = 0;

    if (ini_get_int_value(ini, "HPF", "enable")) {
        enableOpenMask |= VQE_V2_MASK_HPF;
        vqe_config->hpf_attr.mode = ini_get_int_value(ini, "HPF", "mode");
        if (vqe_config->hpf_attr.mode == VQE_V2_USR_MODE_USER) { //手动模式
            vqe_config->hpf_attr.freq = ini_get_int_value(ini, "HPF", "freq");
        }
        printf("get ai hpf parameter:\n");
        printf("   mode: %d\n", vqe_config->hpf_attr.mode);
        printf("   freq: %d\n", vqe_config->hpf_attr.freq);
    }

    if (ini_get_int_value(ini, "AEC", "enable")) {
        enableOpenMask |= VQE_V2_MASK_AEC;
        vqe_config->aec_attr.mode = ini_get_int_value(ini, "AEC", "mode");
        if (vqe_config->aec_attr.mode == VQE_V2_USR_MODE_USER) { //手动模式
            vqe_config->aec_attr.cng_enable = ini_get_int_value(ini, "AEC", "cng_enable");
            vqe_config->aec_attr.freq_boundary = ini_get_int_value(ini, "AEC", "freq_boundary");
            vqe_config->aec_attr.high_suppress_level = LEVEL_COEFFICIENT * ini_get_int_value(ini, "AEC", "high_suppress_level");
            vqe_config->aec_attr.low_suppress_level = LEVEL_COEFFICIENT * ini_get_int_value(ini, "AEC", "low_suppress_level");
            vqe_config->aec_attr.low_enhance_voice_protect_level = LEVEL_COEFFICIENT * ini_get_int_value(ini, "AEC", "low_enhance_voice_protect_level");
            vqe_config->aec_attr.high_enhance_voice_protect_level = LEVEL_COEFFICIENT * ini_get_int_value(ini, "AEC", "high_enhance_voice_protect_level");
        }

        printf("get ai aec parameter:\n");
        printf("   mode: %d\n", vqe_config->aec_attr.mode);
        printf("   cng_enable: %d\n", vqe_config->aec_attr.cng_enable);
        printf("   freq_boundary: %d\n", vqe_config->aec_attr.freq_boundary);
        printf("   high_suppress_level: 0x%x\n", vqe_config->aec_attr.high_suppress_level);
        printf("   low_suppress_level: 0x%x\n", vqe_config->aec_attr.low_suppress_level);
        printf("   high_enhance_voice_protect_level: 0x%x\n", vqe_config->aec_attr.high_enhance_voice_protect_level);
        printf("   low_enhance_voice_protect_level: 0x%x\n", vqe_config->aec_attr.low_enhance_voice_protect_level);
    }

    if (ini_get_int_value(ini, "ANR", "enable")) {
        enableOpenMask |= VQE_V2_MASK_ANR;
        vqe_config->anr_attr.mode = ini_get_int_value(ini, "ANR", "mode");
        if (vqe_config->anr_attr.mode == VQE_V2_USR_MODE_USER) { //手动模式
            vqe_config->anr_attr.usr_scene = LEVEL_COEFFICIENT * ini_get_int_value(ini, "ANR", "usr_scene");
            vqe_config->anr_attr.nr_mode = LEVEL_COEFFICIENT * ini_get_int_value(ini, "ANR", "nr_mode");
            vqe_config->anr_attr.max_suppress_gain = LEVEL_COEFFICIENT * ini_get_int_value(ini, "ANR", "max_suppress_gain");
            vqe_config->anr_attr.suppress_level = LEVEL_COEFFICIENT * ini_get_int_value(ini, "ANR", "suppress_level");
            vqe_config->anr_attr.nonstationary_suppress_level = LEVEL_COEFFICIENT * ini_get_int_value(ini, "ANR", "nonstationary_suppress_level");
        }
        printf("get ai anr parameter:\n");
        printf("   mode: %d\n", vqe_config->anr_attr.mode);
        printf("   usr_scene: 0x%x\n", vqe_config->anr_attr.usr_scene);
        printf("   nr_mode: %d\n", vqe_config->anr_attr.nr_mode);
        printf("   max_suppress_gain: 0x%x\n", vqe_config->anr_attr.max_suppress_gain);
        printf("   suppress_level: %d\n", vqe_config->anr_attr.suppress_level);
        printf("   nonstationary_suppress_level: 0x%x\n", vqe_config->anr_attr.nonstationary_suppress_level);
    }

    if (ini_get_int_value(ini, "AGC", "enable")) {
        enableOpenMask |= VQE_V2_MASK_AGC;
        vqe_config->agc_attr.mode = ini_get_int_value(ini, "AGC", "mode");
        if (vqe_config->agc_attr.mode == VQE_V2_USR_MODE_USER) { //手动模式
            vqe_config->agc_attr.target_level = ini_get_int_value(ini, "AGC", "target_level");
            vqe_config->agc_attr.max_boost_gain = ini_get_int_value(ini, "AGC", "max_boost_gain");
            vqe_config->agc_attr.noise_floor = ini_get_int_value(ini, "AGC", "noise_floor");
            vqe_config->agc_attr.ratio = ini_get_int_value(ini, "AGC", "ratio");
            vqe_config->agc_attr.attack_time = ini_get_int_value(ini, "AGC", "attack_time");
            vqe_config->agc_attr.release_time = ini_get_int_value(ini, "AGC", "release_time");
        }
        printf("get ai agc parameter:\n");
        printf("   mode: %d\n", vqe_config->agc_attr.mode);
        printf("   target_level: %d\n", vqe_config->agc_attr.target_level);
        printf("   max_boost_gain: %d\n", vqe_config->agc_attr.max_boost_gain);
        printf("   noise_floor: %d\n", vqe_config->agc_attr.noise_floor);
        printf("   ratio: %d\n", vqe_config->agc_attr.ratio);
        printf("   attack_time: %d\n", vqe_config->agc_attr.attack_time);
        printf("   release_time: %d\n", vqe_config->agc_attr.release_time);
    }

    if (ini_get_int_value(ini, "BF", "enable")) {
        enableOpenMask |= VQE_V2_MASK_BF;
        printf("   BF enable\n");
    }

    if (ini_get_int_value(ini, "VAD", "enable")) {
        enableOpenMask |= VQE_V2_MASK_VAD;
        vqe_config->vad_attr.mode = ini_get_int_value(ini, "VAD", "mode");
        if (vqe_config->vad_attr.mode == VQE_V2_USR_MODE_USER) { //手动模式
            vqe_config->vad_attr.sensitivity_level = ini_get_int_value(ini, "VAD", "sensitivity_level");
        }
        printf("get ai vad parameter:\n");
        printf("   mode: %d\n", vqe_config->vad_attr.mode);
        printf("   sensitivity_level: %d\n", vqe_config->vad_attr.sensitivity_level);
    }

    if (ini_get_int_value(ini, "DEREVERB", "enable")) {
        enableOpenMask |= VQE_V2_MASK_DEREVERB;
        vqe_config->dereverb_attr.mode = ini_get_int_value(ini, "DEREVERB", "mode");
        if (vqe_config->dereverb_attr.mode == VQE_V2_USR_MODE_USER) { //手动模式
            vqe_config->dereverb_attr.delay_offset = ini_get_int_value(ini, "DEREVERB", "delay_offset");
            vqe_config->dereverb_attr.delay_count = ini_get_int_value(ini, "DEREVERB", "delay_count");
        }
        printf("get ai dereverb parameter:\n");
        printf("   mode: %d\n", vqe_config->dereverb_attr.mode);
        printf("   delay_offset: %d\n", vqe_config->dereverb_attr.delay_offset);
        printf("   delay_count: %d\n", vqe_config->dereverb_attr.delay_count);
    }

    if (ini_get_int_value(ini, "WNS", "enable")) {
        enableOpenMask |= VQE_V2_MASK_WNS;
        vqe_config->wns_attr.mode = ini_get_int_value(ini, "WNS", "mode");
        if (vqe_config->wns_attr.mode == VQE_V2_USR_MODE_USER) { //手动模式
            vqe_config->wns_attr.suppress_level = ini_get_int_value(ini, "WNS", "suppress_level");
        }
        printf("get ai wns parameter:\n");
        printf("   mode: %d\n", vqe_config->wns_attr.mode);
        printf("   suppress_level: %d\n", vqe_config->wns_attr.suppress_level);
    }

    if (ini_get_int_value(ini, "EQ", "enable")) {
        enableOpenMask |= VQE_V2_MASK_EQ;
        vqe_config->eq_attr.gain[0] = ini_get_int_value(ini, "EQ", "gain_0");
        vqe_config->eq_attr.gain[1] = ini_get_int_value(ini, "EQ", "gain_1");
        vqe_config->eq_attr.gain[2] = ini_get_int_value(ini, "EQ", "gain_2");
        vqe_config->eq_attr.gain[3] = ini_get_int_value(ini, "EQ", "gain_3");
        vqe_config->eq_attr.gain[4] = ini_get_int_value(ini, "EQ", "gain_4");
        vqe_config->eq_attr.gain[5] = ini_get_int_value(ini, "EQ", "gain_5");
        vqe_config->eq_attr.gain[6] = ini_get_int_value(ini, "EQ", "gain_6");
        vqe_config->eq_attr.gain[7] = ini_get_int_value(ini, "EQ", "gain_7");
        vqe_config->eq_attr.gain[8] = ini_get_int_value(ini, "EQ", "gain_8");
        vqe_config->eq_attr.gain[9] = ini_get_int_value(ini, "EQ", "gain_9");
        vqe_config->eq_attr.gain[10] = ini_get_int_value(ini, "EQ", "gain_10");
        vqe_config->eq_attr.gain[11] = ini_get_int_value(ini, "EQ", "gain_11");
        vqe_config->eq_attr.gain[12] = ini_get_int_value(ini, "EQ", "gain_12");
        vqe_config->eq_attr.gain[13] = ini_get_int_value(ini, "EQ", "gain_13");
        vqe_config->eq_attr.gain[14] = ini_get_int_value(ini, "EQ", "gain_14");
        vqe_config->eq_attr.gain[15] = ini_get_int_value(ini, "EQ", "gain_15");
        vqe_config->eq_attr.gain[16] = ini_get_int_value(ini, "EQ", "gain_16");
        vqe_config->eq_attr.gain[17] = ini_get_int_value(ini, "EQ", "gain_17");
        vqe_config->eq_attr.gain[18] = ini_get_int_value(ini, "EQ", "gain_18");
        vqe_config->eq_attr.gain[19] = ini_get_int_value(ini, "EQ", "gain_19");
        vqe_config->eq_attr.gain[20] = ini_get_int_value(ini, "EQ", "gain_20");
        vqe_config->eq_attr.gain[21] = ini_get_int_value(ini, "EQ", "gain_21");
        vqe_config->eq_attr.gain[22] = ini_get_int_value(ini, "EQ", "gain_22");
        vqe_config->eq_attr.gain[23] = ini_get_int_value(ini, "EQ", "gain_23");
        vqe_config->eq_attr.gain[24] = ini_get_int_value(ini, "EQ", "gain_24");
        vqe_config->eq_attr.gain[25] = ini_get_int_value(ini, "EQ", "gain_25");
        vqe_config->eq_attr.gain[26] = ini_get_int_value(ini, "EQ", "gain_26");
        vqe_config->eq_attr.gain[27] = ini_get_int_value(ini, "EQ", "gain_27");
        vqe_config->eq_attr.gain[28] = ini_get_int_value(ini, "EQ", "gain_28");
        vqe_config->eq_attr.gain[29] = ini_get_int_value(ini, "EQ", "gain_29");
        printf("get ai eq parameter: enable\n");
        xmedia_s32 i;
        printf("   gain[0-14]: ");
        for (i = 0; i < 15; i++) {
                printf(" %d", vqe_config->eq_attr.gain[i]);
        }
        printf("\n   gain[15-29]: ");
        for (i = 0; i < 15; i++) {
                printf(" %d", vqe_config->eq_attr.gain[15 + i]);
        }
        printf("\n");
    }
    vqe_config->mask = enableOpenMask;
}

xmedia_void get_aivqe_v3_parameter(struct ini* ini, ai_vqe_attr_v3* vqe_config)
{
    memset(vqe_config, 0, sizeof(ai_vqe_attr_v3));
    // 获取模块开启状态
    xmedia_u32 enableOpenMask = 0;

    if (ini_get_int_value(ini, "HPF", "enable")) {
        enableOpenMask |= VQE_V3_MASK_HPF;
        vqe_config->hpf_attr.mode = ini_get_int_value(ini, "HPF", "mode");
        if (vqe_config->hpf_attr.mode == VQE_V3_USR_MODE_USER) { //手动模式
            vqe_config->hpf_attr.freq = ini_get_int_value(ini, "HPF", "freq");
        }
        printf("get ai hpf parameter:\n");
        printf("   mode: %d\n", vqe_config->hpf_attr.mode);
        printf("   freq: %d\n", vqe_config->hpf_attr.freq);
    }

    if (ini_get_int_value(ini, "ANR", "enable")) {
        enableOpenMask |= VQE_V3_MASK_ANR;
        vqe_config->anr_attr.mode = ini_get_int_value(ini, "ANR", "mode");
        if (vqe_config->anr_attr.mode == VQE_V3_USR_MODE_USER) { //手动模式
            vqe_config->anr_attr.level = LEVEL_COEFFICIENT * ini_get_int_value(ini, "ANR", "level");
        }
        printf("get ai anr parameter:\n");
        printf("   mode: %d\n", vqe_config->anr_attr.mode);
        printf("   level: %d\n", vqe_config->anr_attr.level);
    }

    if (ini_get_int_value(ini, "AGC", "enable")) {
        enableOpenMask |= VQE_V3_MASK_AGC;
        vqe_config->agc_attr.mode = ini_get_int_value(ini, "AGC", "mode");
        if (vqe_config->agc_attr.mode == VQE_V3_USR_MODE_USER) { //手动模式
            vqe_config->agc_attr.target_level = ini_get_int_value(ini, "AGC", "target_level");
            vqe_config->agc_attr.max_boost_gain = ini_get_int_value(ini, "AGC", "max_boost_gain");
            vqe_config->agc_attr.noise_floor = ini_get_int_value(ini, "AGC", "noise_floor");
            vqe_config->agc_attr.ratio = ini_get_int_value(ini, "AGC", "ratio");
            vqe_config->agc_attr.attack_time = ini_get_int_value(ini, "AGC", "attack_time");
            vqe_config->agc_attr.release_time = ini_get_int_value(ini, "AGC", "release_time");
        }
        printf("get ai agc parameter:\n");
        printf("   mode: %d\n", vqe_config->agc_attr.mode);
        printf("   target_level: %d\n", vqe_config->agc_attr.target_level);
        printf("   max_boost_gain: %d\n", vqe_config->agc_attr.max_boost_gain);
        printf("   noise_floor: %d\n", vqe_config->agc_attr.noise_floor);
        printf("   ratio: %d\n", vqe_config->agc_attr.ratio);
        printf("   attack_time: %d\n", vqe_config->agc_attr.attack_time);
        printf("   release_time: %d\n", vqe_config->agc_attr.release_time);
    }

    if (ini_get_int_value(ini, "EQ", "enable")) {
        enableOpenMask |= VQE_V3_MASK_EQ;
        vqe_config->eq_attr.gain[0] = ini_get_int_value(ini, "EQ", "gain_0");
        vqe_config->eq_attr.gain[1] = ini_get_int_value(ini, "EQ", "gain_1");
        vqe_config->eq_attr.gain[2] = ini_get_int_value(ini, "EQ", "gain_2");
        vqe_config->eq_attr.gain[3] = ini_get_int_value(ini, "EQ", "gain_3");
        vqe_config->eq_attr.gain[4] = ini_get_int_value(ini, "EQ", "gain_4");
        vqe_config->eq_attr.gain[5] = ini_get_int_value(ini, "EQ", "gain_5");
        vqe_config->eq_attr.gain[6] = ini_get_int_value(ini, "EQ", "gain_6");
        vqe_config->eq_attr.gain[7] = ini_get_int_value(ini, "EQ", "gain_7");
        vqe_config->eq_attr.gain[8] = ini_get_int_value(ini, "EQ", "gain_8");
        vqe_config->eq_attr.gain[9] = ini_get_int_value(ini, "EQ", "gain_9");
        vqe_config->eq_attr.gain[10] = ini_get_int_value(ini, "EQ", "gain_10");
        vqe_config->eq_attr.gain[11] = ini_get_int_value(ini, "EQ", "gain_11");
        vqe_config->eq_attr.gain[12] = ini_get_int_value(ini, "EQ", "gain_12");
        vqe_config->eq_attr.gain[13] = ini_get_int_value(ini, "EQ", "gain_13");
        vqe_config->eq_attr.gain[14] = ini_get_int_value(ini, "EQ", "gain_14");
        vqe_config->eq_attr.gain[15] = ini_get_int_value(ini, "EQ", "gain_15");
        vqe_config->eq_attr.gain[16] = ini_get_int_value(ini, "EQ", "gain_16");
        vqe_config->eq_attr.gain[17] = ini_get_int_value(ini, "EQ", "gain_17");
        vqe_config->eq_attr.gain[18] = ini_get_int_value(ini, "EQ", "gain_18");
        vqe_config->eq_attr.gain[19] = ini_get_int_value(ini, "EQ", "gain_19");
        vqe_config->eq_attr.gain[20] = ini_get_int_value(ini, "EQ", "gain_20");
        vqe_config->eq_attr.gain[21] = ini_get_int_value(ini, "EQ", "gain_21");
        vqe_config->eq_attr.gain[22] = ini_get_int_value(ini, "EQ", "gain_22");
        vqe_config->eq_attr.gain[23] = ini_get_int_value(ini, "EQ", "gain_23");
        vqe_config->eq_attr.gain[24] = ini_get_int_value(ini, "EQ", "gain_24");
        vqe_config->eq_attr.gain[25] = ini_get_int_value(ini, "EQ", "gain_25");
        vqe_config->eq_attr.gain[26] = ini_get_int_value(ini, "EQ", "gain_26");
        vqe_config->eq_attr.gain[27] = ini_get_int_value(ini, "EQ", "gain_27");
        vqe_config->eq_attr.gain[28] = ini_get_int_value(ini, "EQ", "gain_28");
        vqe_config->eq_attr.gain[29] = ini_get_int_value(ini, "EQ", "gain_29");
        printf("get ai eq parameter: enable\n");
        xmedia_s32 i;
        printf("   gain[0-14]: ");
        for (i = 0; i < 15; i++) {
                printf(" %d", vqe_config->eq_attr.gain[i]);
        }
        printf("\n   gain[15-29]: ");
        for (i = 0; i < 15; i++) {
                printf(" %d", vqe_config->eq_attr.gain[15 + i]);
        }
        printf("\n");
    }
    vqe_config->mask = enableOpenMask;
}

xmedia_void get_aovqe_v1_parameter(struct ini* ini, ao_vqe_attr_v1* vqe_config)
{
    memset(vqe_config, 0, sizeof(ao_vqe_attr_v1));
    // 获取模块开启状态
    xmedia_u32 enableOpenMask = 0;
    if (ini_get_int_value(ini, "HPF", "enable")) {
        enableOpenMask |= VQE_V1_MASK_HPF;
        vqe_config->hpf_attr.mode = ini_get_int_value(ini, "HPF", "mode");
        if (vqe_config->hpf_attr.mode == VQE_V1_USR_MODE_USER) { //手动模式
            vqe_config->hpf_attr.freq = ini_get_int_value(ini, "HPF", "freq");
        }
        printf("get ao hpf parameter:\n");
        printf("   mode: %d\n", vqe_config->hpf_attr.mode);
        printf("   freq: %d\n", vqe_config->hpf_attr.freq);
    }

    if (ini_get_int_value(ini, "ANR", "enable")) {
        enableOpenMask |= VQE_V1_MASK_ANR;
        vqe_config->anr_attr.mode = ini_get_int_value(ini, "ANR", "mode");
        if (vqe_config->anr_attr.mode == VQE_V1_USR_MODE_USER) { //手动模式
            vqe_config->anr_attr.usr_scene = LEVEL_COEFFICIENT * ini_get_int_value(ini, "ANR", "usr_scene");
            vqe_config->anr_attr.nr_mode = LEVEL_COEFFICIENT * ini_get_int_value(ini, "ANR", "nr_mode");
            vqe_config->anr_attr.max_suppress_gain = LEVEL_COEFFICIENT * ini_get_int_value(ini, "ANR", "max_suppress_gain");
            vqe_config->anr_attr.suppress_level = LEVEL_COEFFICIENT * ini_get_int_value(ini, "ANR", "suppress_level");
            vqe_config->anr_attr.nonstationary_suppress_level = LEVEL_COEFFICIENT * ini_get_int_value(ini, "ANR", "nonstationary_suppress_level");
        }
        printf("get ao anr parameter:\n");
        printf("   mode: %d\n", vqe_config->anr_attr.mode);
        printf("   usr_scene: 0x%x\n", vqe_config->anr_attr.usr_scene);
        printf("   nr_mode: %d\n", vqe_config->anr_attr.nr_mode);
        printf("   max_suppress_gain: 0x%x\n", vqe_config->anr_attr.max_suppress_gain);
        printf("   suppress_level: %d\n", vqe_config->anr_attr.suppress_level);
        printf("   nonstationary_suppress_level: 0x%x\n", vqe_config->anr_attr.nonstationary_suppress_level);
    }

    if (ini_get_int_value(ini, "AGC", "enable")) {
        enableOpenMask |= VQE_V1_MASK_AGC;
        vqe_config->agc_attr.mode = ini_get_int_value(ini, "AGC", "mode");
        if (vqe_config->agc_attr.mode == VQE_V1_USR_MODE_USER) { //手动模式
            vqe_config->agc_attr.target_level = ini_get_int_value(ini, "AGC", "target_level");
            vqe_config->agc_attr.max_boost_gain = ini_get_int_value(ini, "AGC", "max_boost_gain");
            vqe_config->agc_attr.noise_floor = ini_get_int_value(ini, "AGC", "noise_floor");
            vqe_config->agc_attr.ratio = ini_get_int_value(ini, "AGC", "ratio");
            vqe_config->agc_attr.attack_time = ini_get_int_value(ini, "AGC", "attack_time");
            vqe_config->agc_attr.release_time = ini_get_int_value(ini, "AGC", "release_time");
        }
        printf("get ao agc parameter:\n");
        printf("   mode: %d\n", vqe_config->agc_attr.mode);
        printf("   target_level: %d\n", vqe_config->agc_attr.target_level);
        printf("   max_boost_gain: %d\n", vqe_config->agc_attr.max_boost_gain);
        printf("   noise_floor: %d\n", vqe_config->agc_attr.noise_floor);
        printf("   ratio: %d\n", vqe_config->agc_attr.ratio);
        printf("   attack_time: %d\n", vqe_config->agc_attr.attack_time);
        printf("   release_time: %d\n", vqe_config->agc_attr.release_time);
    }

    if (ini_get_int_value(ini, "EQ", "enable")) {
        enableOpenMask |= VQE_V1_MASK_EQ;
        vqe_config->eq_attr.gain[0] = ini_get_int_value(ini, "EQ", "gain_0");
        vqe_config->eq_attr.gain[1] = ini_get_int_value(ini, "EQ", "gain_1");
        vqe_config->eq_attr.gain[2] = ini_get_int_value(ini, "EQ", "gain_2");
        vqe_config->eq_attr.gain[3] = ini_get_int_value(ini, "EQ", "gain_3");
        vqe_config->eq_attr.gain[4] = ini_get_int_value(ini, "EQ", "gain_4");
        vqe_config->eq_attr.gain[5] = ini_get_int_value(ini, "EQ", "gain_5");
        vqe_config->eq_attr.gain[6] = ini_get_int_value(ini, "EQ", "gain_6");
        vqe_config->eq_attr.gain[7] = ini_get_int_value(ini, "EQ", "gain_7");
        vqe_config->eq_attr.gain[8] = ini_get_int_value(ini, "EQ", "gain_8");
        vqe_config->eq_attr.gain[9] = ini_get_int_value(ini, "EQ", "gain_9");
        vqe_config->eq_attr.gain[10] = ini_get_int_value(ini, "EQ", "gain_10");
        vqe_config->eq_attr.gain[11] = ini_get_int_value(ini, "EQ", "gain_11");
        vqe_config->eq_attr.gain[12] = ini_get_int_value(ini, "EQ", "gain_12");
        vqe_config->eq_attr.gain[13] = ini_get_int_value(ini, "EQ", "gain_13");
        vqe_config->eq_attr.gain[14] = ini_get_int_value(ini, "EQ", "gain_14");
        vqe_config->eq_attr.gain[15] = ini_get_int_value(ini, "EQ", "gain_15");
        vqe_config->eq_attr.gain[16] = ini_get_int_value(ini, "EQ", "gain_16");
        vqe_config->eq_attr.gain[17] = ini_get_int_value(ini, "EQ", "gain_17");
        vqe_config->eq_attr.gain[18] = ini_get_int_value(ini, "EQ", "gain_18");
        vqe_config->eq_attr.gain[19] = ini_get_int_value(ini, "EQ", "gain_19");
        vqe_config->eq_attr.gain[20] = ini_get_int_value(ini, "EQ", "gain_20");
        vqe_config->eq_attr.gain[21] = ini_get_int_value(ini, "EQ", "gain_21");
        vqe_config->eq_attr.gain[22] = ini_get_int_value(ini, "EQ", "gain_22");
        vqe_config->eq_attr.gain[23] = ini_get_int_value(ini, "EQ", "gain_23");
        vqe_config->eq_attr.gain[24] = ini_get_int_value(ini, "EQ", "gain_24");
        vqe_config->eq_attr.gain[25] = ini_get_int_value(ini, "EQ", "gain_25");
        vqe_config->eq_attr.gain[26] = ini_get_int_value(ini, "EQ", "gain_26");
        vqe_config->eq_attr.gain[27] = ini_get_int_value(ini, "EQ", "gain_27");
        vqe_config->eq_attr.gain[28] = ini_get_int_value(ini, "EQ", "gain_28");
        vqe_config->eq_attr.gain[29] = ini_get_int_value(ini, "EQ", "gain_29");
        printf("get ao eq parameter: enable\n");
        xmedia_s32 i;
        printf("   gain[0-14]: ");
        for (i = 0; i < 15; i++) {
                printf(" %d", vqe_config->eq_attr.gain[i]);
        }
        printf("\n   gain[15-29]: ");
        for (i = 0; i < 15; i++) {
                printf(" %d", vqe_config->eq_attr.gain[15 + i]);
        }
        printf("\n");
    }
    vqe_config->mask = enableOpenMask;
}

xmedia_void get_aovqe_v2_parameter(struct ini* ini, ao_vqe_attr_v2* vqe_config)
{
    memset(vqe_config, 0, sizeof(ao_vqe_attr_v2));
    // 获取模块开启状态
    xmedia_u32 enableOpenMask = 0;
    if (ini_get_int_value(ini, "HPF", "enable")) {
        enableOpenMask |= VQE_V2_MASK_HPF;
        vqe_config->hpf_attr.mode = ini_get_int_value(ini, "HPF", "mode");
        if (vqe_config->hpf_attr.mode == VQE_V2_USR_MODE_USER) { //手动模式
            vqe_config->hpf_attr.freq = ini_get_int_value(ini, "HPF", "freq");
        }
        printf("get ao hpf parameter:\n");
        printf("   mode: %d\n", vqe_config->hpf_attr.mode);
        printf("   freq: %d\n", vqe_config->hpf_attr.freq);
    }

    if (ini_get_int_value(ini, "ANR", "enable")) {
        enableOpenMask |= VQE_V2_MASK_ANR;
        vqe_config->anr_attr.mode = ini_get_int_value(ini, "ANR", "mode");
        if (vqe_config->anr_attr.mode == VQE_V2_USR_MODE_USER) { //手动模式
            vqe_config->anr_attr.usr_scene = LEVEL_COEFFICIENT * ini_get_int_value(ini, "ANR", "usr_scene");
            vqe_config->anr_attr.nr_mode = LEVEL_COEFFICIENT * ini_get_int_value(ini, "ANR", "nr_mode");
            vqe_config->anr_attr.max_suppress_gain = LEVEL_COEFFICIENT * ini_get_int_value(ini, "ANR", "max_suppress_gain");
            vqe_config->anr_attr.suppress_level = LEVEL_COEFFICIENT * ini_get_int_value(ini, "ANR", "suppress_level");
            vqe_config->anr_attr.nonstationary_suppress_level = LEVEL_COEFFICIENT * ini_get_int_value(ini, "ANR", "nonstationary_suppress_level");
        }
        printf("get ao anr parameter:\n");
        printf("   mode: %d\n", vqe_config->anr_attr.mode);
        printf("   usr_scene: 0x%x\n", vqe_config->anr_attr.usr_scene);
        printf("   nr_mode: %d\n", vqe_config->anr_attr.nr_mode);
        printf("   max_suppress_gain: 0x%x\n", vqe_config->anr_attr.max_suppress_gain);
        printf("   suppress_level: %d\n", vqe_config->anr_attr.suppress_level);
        printf("   nonstationary_suppress_level: 0x%x\n", vqe_config->anr_attr.nonstationary_suppress_level);
    }

    if (ini_get_int_value(ini, "AGC", "enable")) {
        enableOpenMask |= VQE_V2_MASK_AGC;
        vqe_config->agc_attr.mode = ini_get_int_value(ini, "AGC", "mode");
        if (vqe_config->agc_attr.mode == VQE_V2_USR_MODE_USER) { //手动模式
            vqe_config->agc_attr.target_level = ini_get_int_value(ini, "AGC", "target_level");
            vqe_config->agc_attr.max_boost_gain = ini_get_int_value(ini, "AGC", "max_boost_gain");
            vqe_config->agc_attr.noise_floor = ini_get_int_value(ini, "AGC", "noise_floor");
            vqe_config->agc_attr.ratio = ini_get_int_value(ini, "AGC", "ratio");
            vqe_config->agc_attr.attack_time = ini_get_int_value(ini, "AGC", "attack_time");
            vqe_config->agc_attr.release_time = ini_get_int_value(ini, "AGC", "release_time");
        }
        printf("get ao agc parameter:\n");
        printf("   mode: %d\n", vqe_config->agc_attr.mode);
        printf("   target_level: %d\n", vqe_config->agc_attr.target_level);
        printf("   max_boost_gain: %d\n", vqe_config->agc_attr.max_boost_gain);
        printf("   noise_floor: %d\n", vqe_config->agc_attr.noise_floor);
        printf("   ratio: %d\n", vqe_config->agc_attr.ratio);
        printf("   attack_time: %d\n", vqe_config->agc_attr.attack_time);
        printf("   release_time: %d\n", vqe_config->agc_attr.release_time);
    }

    if (ini_get_int_value(ini, "EQ", "enable")) {
        enableOpenMask |= VQE_V2_MASK_EQ;
        vqe_config->eq_attr.gain[0] = ini_get_int_value(ini, "EQ", "gain_0");
        vqe_config->eq_attr.gain[1] = ini_get_int_value(ini, "EQ", "gain_1");
        vqe_config->eq_attr.gain[2] = ini_get_int_value(ini, "EQ", "gain_2");
        vqe_config->eq_attr.gain[3] = ini_get_int_value(ini, "EQ", "gain_3");
        vqe_config->eq_attr.gain[4] = ini_get_int_value(ini, "EQ", "gain_4");
        vqe_config->eq_attr.gain[5] = ini_get_int_value(ini, "EQ", "gain_5");
        vqe_config->eq_attr.gain[6] = ini_get_int_value(ini, "EQ", "gain_6");
        vqe_config->eq_attr.gain[7] = ini_get_int_value(ini, "EQ", "gain_7");
        vqe_config->eq_attr.gain[8] = ini_get_int_value(ini, "EQ", "gain_8");
        vqe_config->eq_attr.gain[9] = ini_get_int_value(ini, "EQ", "gain_9");
        vqe_config->eq_attr.gain[10] = ini_get_int_value(ini, "EQ", "gain_10");
        vqe_config->eq_attr.gain[11] = ini_get_int_value(ini, "EQ", "gain_11");
        vqe_config->eq_attr.gain[12] = ini_get_int_value(ini, "EQ", "gain_12");
        vqe_config->eq_attr.gain[13] = ini_get_int_value(ini, "EQ", "gain_13");
        vqe_config->eq_attr.gain[14] = ini_get_int_value(ini, "EQ", "gain_14");
        vqe_config->eq_attr.gain[15] = ini_get_int_value(ini, "EQ", "gain_15");
        vqe_config->eq_attr.gain[16] = ini_get_int_value(ini, "EQ", "gain_16");
        vqe_config->eq_attr.gain[17] = ini_get_int_value(ini, "EQ", "gain_17");
        vqe_config->eq_attr.gain[18] = ini_get_int_value(ini, "EQ", "gain_18");
        vqe_config->eq_attr.gain[19] = ini_get_int_value(ini, "EQ", "gain_19");
        vqe_config->eq_attr.gain[20] = ini_get_int_value(ini, "EQ", "gain_20");
        vqe_config->eq_attr.gain[21] = ini_get_int_value(ini, "EQ", "gain_21");
        vqe_config->eq_attr.gain[22] = ini_get_int_value(ini, "EQ", "gain_22");
        vqe_config->eq_attr.gain[23] = ini_get_int_value(ini, "EQ", "gain_23");
        vqe_config->eq_attr.gain[24] = ini_get_int_value(ini, "EQ", "gain_24");
        vqe_config->eq_attr.gain[25] = ini_get_int_value(ini, "EQ", "gain_25");
        vqe_config->eq_attr.gain[26] = ini_get_int_value(ini, "EQ", "gain_26");
        vqe_config->eq_attr.gain[27] = ini_get_int_value(ini, "EQ", "gain_27");
        vqe_config->eq_attr.gain[28] = ini_get_int_value(ini, "EQ", "gain_28");
        vqe_config->eq_attr.gain[29] = ini_get_int_value(ini, "EQ", "gain_29");
        printf("get ao eq parameter: enable\n");
        xmedia_s32 i;
        printf("   gain[0-14]: ");
        for (i = 0; i < 15; i++) {
                printf(" %d", vqe_config->eq_attr.gain[i]);
        }
        printf("\n   gain[15-29]: ");
        for (i = 0; i < 15; i++) {
                printf(" %d", vqe_config->eq_attr.gain[15 + i]);
        }
        printf("\n");
    }
    vqe_config->mask = enableOpenMask;
}

xmedia_void get_aivqe_detect_parameter(struct ini* ini, ai_vqe_attr_detect* vqe_config)
{
    memset(vqe_config, 0, sizeof(ai_vqe_attr_detect));
    // 获取模块开启状态
    xmedia_u32 enableOpenMask = 0;

    if (ini_get_int_value(ini, "HPF", "enable")) {
        enableOpenMask |= VQE_DETECT_MASK_HPF;
        vqe_config->hpf_attr.mode = ini_get_int_value(ini, "HPF", "mode");
        if (vqe_config->hpf_attr.mode == VQE_DET_USR_MODE_USER) { //手动模式
            vqe_config->hpf_attr.freq = ini_get_int_value(ini, "HPF", "freq");
        }
        printf("get ai hpf parameter:\n");
        printf("   mode: %d\n", vqe_config->hpf_attr.mode);
        printf("   freq: %d\n", vqe_config->hpf_attr.freq);
    }

    if (ini_get_int_value(ini, "SSL", "enable")) {
        enableOpenMask |= VQE_DETECT_MASK_SSL;
        vqe_config->ssl_attr.mic_distance  = LEVEL_COEFFICIENT * ini_get_int_value(ini, "SSL", "mic_distance");
        vqe_config->ssl_attr.sensitivity_level = LEVEL_COEFFICIENT * ini_get_int_value(ini, "SSL", "sensitivity_level");
        vqe_config->ssl_attr.shape = LEVEL_COEFFICIENT * ini_get_int_value(ini, "SSL", "shape");
        printf("get ai ssl parameter:\n");
        printf("   mic_distance: %d\n", vqe_config->ssl_attr.mic_distance);
        printf("   sensitivity_level: %d\n", vqe_config->ssl_attr.sensitivity_level);
        printf("   shape: %d\n", vqe_config->ssl_attr.shape);
    }

    if (ini_get_int_value(ini, "BCD", "enable")) {
        enableOpenMask |= VQE_DETECT_MASK_BCD;
        vqe_config->bcd_attr.usr_mode = ini_get_int_value(ini, "BCD", "usr_mode");
        if (vqe_config->bcd_attr.usr_mode == XMEDIA_TRUE) { //手动模式
            vqe_config->bcd_attr.bypass = ini_get_int_value(ini, "BCD", "bypass");
            vqe_config->bcd_attr.alarm_threshold = ini_get_int_value(ini, "BCD", "alarm_threshold");
            vqe_config->bcd_attr.time_limit = ini_get_int_value(ini, "BCD", "time_limit");
            vqe_config->bcd_attr.time_limit_threshold_count = ini_get_int_value(ini, "BCD", "time_limit_threshold_count");
            vqe_config->bcd_attr.interval_time = ini_get_int_value(ini, "BCD", "interval_time");
        }
        printf("get ai bcd parameter:\n");
        printf("   usr_mode: %d\n", vqe_config->bcd_attr.usr_mode);
        printf("   bypass: %d\n", vqe_config->bcd_attr.bypass);
        printf("   alarm_threshold: %d\n", vqe_config->bcd_attr.alarm_threshold);
        printf("   time_limit: %d\n", vqe_config->bcd_attr.time_limit);
        printf("   time_limit_threshold_count: %d\n", vqe_config->bcd_attr.time_limit_threshold_count);
        printf("   interval_time: %d\n", vqe_config->bcd_attr.interval_time);
    }

    if (ini_get_int_value(ini, "GBD", "enable")) {
        enableOpenMask |= VQE_DETECT_MASK_GBD;
        vqe_config->gbd_attr.usr_mode = ini_get_int_value(ini, "GBD", "usr_mode");
        if (vqe_config->gbd_attr.usr_mode == XMEDIA_TRUE) { //手动模式
            vqe_config->gbd_attr.bypass = ini_get_int_value(ini, "GBD", "bypass");
            vqe_config->gbd_attr.alarm_threshold = ini_get_int_value(ini, "GBD", "alarm_threshold");
            vqe_config->gbd_attr.time_limit = ini_get_int_value(ini, "GBD", "time_limit");
            vqe_config->gbd_attr.time_limit_threshold_count = ini_get_int_value(ini, "GBD", "time_limit_threshold_count");
            vqe_config->gbd_attr.interval_time = ini_get_int_value(ini, "GBD", "interval_time");
        }
        printf("get ai gbd parameter:\n");
        printf("   usr_mode: %d\n", vqe_config->gbd_attr.usr_mode);
        printf("   bypass: %d\n", vqe_config->gbd_attr.bypass);
        printf("   alarm_threshold: %d\n", vqe_config->gbd_attr.alarm_threshold);
        printf("   time_limit: %d\n", vqe_config->gbd_attr.time_limit);
        printf("   time_limit_threshold_count: %d\n", vqe_config->gbd_attr.time_limit_threshold_count);
        printf("   interval_time: %d\n", vqe_config->gbd_attr.interval_time);
    }

    if (ini_get_int_value(ini, "VED", "enable")) {
        enableOpenMask |= VQE_DETECT_MASK_VED;
        vqe_config->ved_attr.mode = ini_get_int_value(ini, "VED", "usr_mode");
        if (vqe_config->ved_attr.mode == VQE_DET_USR_MODE_USER) { //手动模式
            vqe_config->ved_attr.volume_threshold = ini_get_int_value(ini, "VED", "volume_threshold");
            vqe_config->ved_attr.mutation_flag = ini_get_int_value(ini, "VED", "mutation_flag");
            vqe_config->ved_attr.fast_change_threshold = ini_get_int_value(ini, "VED", "fast_change_threshold");
            vqe_config->ved_attr.slow_change_threshold = ini_get_int_value(ini, "VED", "slow_change_threshold");
        }
        printf("get ai ved parameter:\n");
        printf("   mode: %d\n", vqe_config->ved_attr.mode);
        printf("   volume_threshold: %d\n", vqe_config->ved_attr.volume_threshold);
        printf("   mutation_flag: %d\n", vqe_config->ved_attr.mutation_flag);
        printf("   fast_change_threshold: %d\n", vqe_config->ved_attr.fast_change_threshold);
        printf("   slow_change_threshold: %d\n", vqe_config->ved_attr.slow_change_threshold);
    }
    vqe_config->mask = enableOpenMask;
}

xmedia_s32 load_base_parameter(xmedia_char* ini_name, user_base_config * base_config)
{
    struct ini* ini_data;

    ini_data = create_ini(ini_name);
    if (ini_data == XMEDIA_NULL) {
        printf("load_aivqe_parameter create_ini failed\n");
        return XMEDIA_FAILURE;
    }
    get_base_parameter(ini_data, base_config);
    destroy_ini(ini_data);
    return XMEDIA_SUCCESS;
}

xmedia_s32 load_aivqe_parameter(xmedia_char* ini_name, ai_vqe_attr* vqe_config)
{
    struct ini* ini_data;

    ini_data = create_ini(ini_name);
    if (ini_data == XMEDIA_NULL) {
        printf("load_aivqe_parameter create_ini failed\n");
        return XMEDIA_FAILURE;
    }
    if (vqe_config->version == AI_VQE_ENHANCE_ATTR_VERSION_1) {    //only support vqe v1
        get_aivqe_v1_parameter(ini_data, (ai_vqe_attr_v1 *)(vqe_config->attr));
    } else if (vqe_config->version == AI_VQE_ENHANCE_ATTR_VERSION_2){
        get_aivqe_v2_parameter(ini_data, (ai_vqe_attr_v2 *)(vqe_config->attr));
    } else if (vqe_config->version == AI_VQE_ENHANCE_ATTR_VERSION_3){
        get_aivqe_v3_parameter(ini_data, (ai_vqe_attr_v3 *)(vqe_config->attr));
    } else if (vqe_config->version == AI_VQE_DETECT_ATTR_VERSION_1) {
        get_aivqe_detect_parameter(ini_data, (ai_vqe_attr_detect *)(vqe_config->attr));
    } else {
        printf("load_aivqe_parameter not support vqe version(%d)\n", vqe_config->version);
        return XMEDIA_FAILURE;
    }
    destroy_ini(ini_data);
    return XMEDIA_SUCCESS;
}

xmedia_s32 load_aovqe_parameter(xmedia_char* ini_name, ao_vqe_attr* vqe_config)
{
    struct ini* ini_data;

    ini_data = create_ini(ini_name);
    if (ini_data == XMEDIA_NULL) {
        printf("load_aivqe_parameter create_ini failed\n");
        return XMEDIA_FAILURE;
    }

    if (vqe_config->version == AO_VQE_ENHANCE_ATTR_VERSION_1) {    //only support vqe v1
        get_aovqe_v1_parameter(ini_data, (ao_vqe_attr_v1 *)(vqe_config->attr));
    } else if (vqe_config->version == AO_VQE_ENHANCE_ATTR_VERSION_2) {
        get_aovqe_v2_parameter(ini_data, (ao_vqe_attr_v2 *)(vqe_config->attr));
    } else {
        printf("load_aovqe_parameter not support vqe version(%d)\n", vqe_config->version);
        return XMEDIA_FAILURE;
    }
    destroy_ini(ini_data);
    return XMEDIA_SUCCESS;
}
