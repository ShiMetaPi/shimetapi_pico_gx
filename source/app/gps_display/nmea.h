/*
 * nmea.h - NMEA-0183 语句解析（$xxGGA / $xxRMC → 结构体）。
 *
 * 兼容 talker 前缀 $GP（GPS）/ $GN（GNSS，多模模块如 ATGM336H）/ $BD（BDS）：
 *   按句子后缀 GGA/RMC 匹配，不看 $ 后两位。
 * 纬度/经度保留 NMEA 原始格式（ddmm.mmmm + N/S），显示时直接展示。
 */
#ifndef NMEA_H
#define NMEA_H

typedef struct {
    int   fix;          /* 1=已定位, 0=未定位 */
    int   sat;          /* 可见卫星数 */
    char  time[10];     /* hhmmss（UTC）*/
    char  date[7];      /* ddmmyy */
    char  lat[20];      /* ddmm.mmmm N/S（未定位时空）*/
    char  lon[20];      /* dddmm.mmmm E/W（未定位时空）*/
    float speed_kn;     /* 速度（节）*/
    float alt_m;        /* 海拔（米）*/
} nmea_data_t;

/* 清零（main 启动时调一次）。 */
void nmea_init(nmea_data_t *out);

/*
 * 解析一行 NMEA。是 GGA/RMC 则更新 out 对应字段并返回 1；其它句子或非 NMEA 返回 0。
 * 空字段（GPS 未定位时常缺纬经度）跳过，不覆盖已有值。
 */
int nmea_parse(const char *line, nmea_data_t *out);

#endif /* NMEA_H */
