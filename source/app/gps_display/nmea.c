/*
 * nmea.c - NMEA-0183 GGA/RMC 解析实现。
 *
 * 字段以逗号分隔，句子末尾 "*XX" 是异或校验和（本驱动不校验，直接按逗号取字段）。
 *
 * $xxGGA 字段：  时间, 纬度, N/S, 经度, E/W, 质量(0无效/>0定位), 卫星数, HDOP, 海拔, M, ...
 * $xxRMC 字段：  时间, 状态(A=定位/V=警告), 纬度, N/S, 经度, E/W, 速度(节), 航向, 日期ddmmyy, ...
 */
#include "nmea.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void nmea_init(nmea_data_t *o)
{
    if (o == NULL) {
        return;
    }
    o->fix = 0;
    o->sat = 0;
    o->time[0] = o->date[0] = o->lat[0] = o->lon[0] = '\0';
    o->speed_kn = 0.0f;
    o->alt_m = 0.0f;
}

/*
 * 从 line 取第 idx 个逗号字段（idx=0 是句子头 $xxGGA），写入 out（不含逗号/星号）。
 * 返回字段长度；0 表示该字段为空或不存在。out 至少 maxlen 字节。
 */
static int get_field(const char *line, int idx, char *out, int maxlen)
{
    int cur = 0;
    int len = 0;
    const char *p = line;

    while (*p != '\0' && cur < idx) {
        if (*p == ',') {
            cur++;
        }
        p++;
    }
    while (*p != '\0' && *p != ',' && *p != '*' && len < maxlen - 1) {
        out[len++] = *p++;
    }
    out[len] = '\0';
    return len;
}

/* 安全拷贝字段到目标（仅当字段非空）。 */
static void set_str(char *dst, int dstlen, const char *src)
{
    if (src[0] != '\0') {
        strncpy(dst, src, dstlen - 1);
        dst[dstlen - 1] = '\0';
    }
}

int nmea_parse(const char *line, nmea_data_t *o)
{
    char f_time[16], f_a[12], f_b[4], f_c[12], f_d[4], f_e[8], f_alt[16], f_spd[16];

    if (line == NULL || o == NULL || line[0] != '$') {
        return 0;
    }

    if (strstr(line, "GGA") != NULL) {
        get_field(line, 1, f_time, sizeof(f_time));   /* 时间 */
        get_field(line, 2, f_a, sizeof(f_a));          /* 纬度 */
        get_field(line, 3, f_b, sizeof(f_b));          /* N/S */
        get_field(line, 4, f_c, sizeof(f_c));          /* 经度 */
        get_field(line, 5, f_d, sizeof(f_d));          /* E/W */
        get_field(line, 6, f_e, sizeof(f_e));          /* 质量 */
        get_field(line, 7, f_b, sizeof(f_b));          /* 复用：卫星数（覆盖 N/S 暂存）*/
        get_field(line, 9, f_alt, sizeof(f_alt));      /* 海拔 */

        set_str(o->time, sizeof(o->time), f_time);
        /* 卫星数：f_b 此时是第 7 字段 */
        if (f_b[0] != '\0') {
            o->sat = atoi(f_b);
        }
        /* 质量 > 0 视为已定位，更新经纬度/海拔 */
        if (f_e[0] != '\0' && f_e[0] != '0') {
            o->fix = 1;
            get_field(line, 3, f_b, sizeof(f_b));      /* 重新取 N/S */
            if (f_a[0] && f_b[0]) {
                snprintf(o->lat, sizeof(o->lat), "%s %s", f_a, f_b);
            }
            get_field(line, 5, f_d, sizeof(f_d));      /* 重新取 E/W */
            if (f_c[0] && f_d[0]) {
                snprintf(o->lon, sizeof(o->lon), "%s %s", f_c, f_d);
            }
            if (f_alt[0]) {
                o->alt_m = strtof(f_alt, NULL);
            }
        }
        return 1;
    }

    if (strstr(line, "RMC") != NULL) {
        char f_status[8], f_date[16];
        get_field(line, 1, f_time, sizeof(f_time));   /* 时间 */
        get_field(line, 2, f_status, sizeof(f_status));/* 状态 A/V */
        get_field(line, 3, f_a, sizeof(f_a));          /* 纬度 */
        get_field(line, 4, f_b, sizeof(f_b));          /* N/S */
        get_field(line, 5, f_c, sizeof(f_c));          /* 经度 */
        get_field(line, 6, f_d, sizeof(f_d));          /* E/W */
        get_field(line, 7, f_spd, sizeof(f_spd));      /* 速度 */
        get_field(line, 9, f_date, sizeof(f_date));    /* 日期 */

        set_str(o->time, sizeof(o->time), f_time);
        set_str(o->date, sizeof(o->date), f_date);
        if (f_status[0] == 'A') {
            o->fix = 1;
            if (f_a[0] && f_b[0]) {
                snprintf(o->lat, sizeof(o->lat), "%s %s", f_a, f_b);
            }
            if (f_c[0] && f_d[0]) {
                snprintf(o->lon, sizeof(o->lon), "%s %s", f_c, f_d);
            }
            if (f_spd[0]) {
                o->speed_kn = strtof(f_spd, NULL);
            }
        } else if (f_status[0] == 'V') {
            o->fix = 0;
        }
        return 1;
    }

    return 0;
}
