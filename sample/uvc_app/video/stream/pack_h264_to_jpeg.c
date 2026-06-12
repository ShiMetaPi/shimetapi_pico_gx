#include <stdio.h>
#include <string.h>

/* 序号        字段            字节        说明
 * 0           标识码          2           0xFFE1 ~ 0xFFEF
 * 1           数据长度        2           字段长度，不包括标识码，但包括本字段。JPEG标志定义，解码器可以根据长度跳过此标识段
 * 2           视频信息        1           码流格式, H264:0x01
 * 3           视频帧率        1           每秒帧数，如25表示25FPS
 * 4           保留信息        6           预留字段，默认都为0x00
 * 5           视频数据长度    2           视频数据字段长度，包括本字段
 * 6           视频数据        n           视频数据字节数 = (视频数据长度 - 2)
 *
 * 说明：每个字段（0xFFEX）仅支持填充(65535 - 12)字节数据。该封装协议支持最大填充(65523 * 15)字节数据
 */

#define PROCOTOL_HEAD_SIZE 14
#define SECTIONS_NUMS      9
#define SECTIONS_SIZE      (65535 - 12)
#define APP1 0xFFE7

static unsigned char* fill_protocol_head(unsigned char* out, unsigned short id, unsigned char fps, unsigned short video_len)
{
    unsigned char* pout = out;
    unsigned short val;

    // Identification code
    val = id;
    *pout++ = (unsigned char)((val >> 8) & 0xFF);
    *pout++ = (unsigned char)(val & 0xFF);

    // Data length
    val = 12 + video_len;
    *pout++ = (unsigned char)((val >> 8) & 0xFF);
    *pout++ = (unsigned char)(val & 0xFF);

    // Video information
    *pout++ = (unsigned char)0x01; // 0x01: H264

    // FPS
    *pout++ = fps;

    // Reserve
    memset(pout, 0, 6);
    pout += 6;

    // Video data length
    val = video_len + 2;
    *pout++ = (unsigned char)((val >> 8) & 0xFF);
    *pout++ = (unsigned char)(val & 0xFF);

    return pout;
}

static unsigned int merge_h264_to_jpeg(unsigned char* out, unsigned char* frame_h264,
        unsigned int size_h264, unsigned short fps)
{
    unsigned int size;
    unsigned short i;
    unsigned char* pout = out;
    unsigned char* psrc = frame_h264;
    unsigned short id = (unsigned short)APP1;
    unsigned short sec_size  = SECTIONS_SIZE;
    unsigned short sections  = size_h264 / sec_size;
    unsigned short remaining = size_h264 % sec_size;

    size = 0;
    for (i = 0; i < sections; i++) {
        pout = fill_protocol_head(pout, id, fps, sec_size);

        // fill frame data
        memcpy(pout, psrc, sec_size);
        pout += sec_size;
        psrc += sec_size;

        id += 1;
        size += (sec_size + PROCOTOL_HEAD_SIZE);
    }

    if (remaining) {
        pout = fill_protocol_head(pout, id, fps, remaining);

        // fill frame data
        memcpy(pout, psrc, remaining);
        pout += remaining;

        id += 1;
        size += (remaining + PROCOTOL_HEAD_SIZE);
    }

    //printf("merge h264 size(%u) to jpeg ok.\n", size);

    return size;
}

static unsigned char* refactoring_jpeg(unsigned char* frame_jpeg, unsigned int size_jpeg, unsigned int len_jpeg, unsigned int size_h264)
{

    unsigned char* ph264_start = NULL;
    unsigned char* pjpeg_old = NULL;
    unsigned char* pjpeg_new = NULL;

    unsigned int size;
    unsigned int move_size;
    unsigned int h264_pack_size, total_pack_size;
    unsigned int sections_num;

    sections_num = (size_h264 / SECTIONS_SIZE);
    if (size_h264 % SECTIONS_SIZE) {
        sections_num += 1;
    }
    h264_pack_size = (sections_num * PROCOTOL_HEAD_SIZE) + size_h264;
    total_pack_size = h264_pack_size + size_jpeg;

    if ((total_pack_size > len_jpeg) || (sections_num > SECTIONS_NUMS)) {
        printf("[ERROR] H264 frame is too large, I will drop it!\n");
        return NULL;
    }

    ph264_start = frame_jpeg;

    // skip 0xFFD8 and 0xFFE0
    ph264_start += 4;

    // skip APP0 字段
    size = (ph264_start[0] << 8) & 0xFF00;
    size +=  (ph264_start[1] & 0xFF);
    ph264_start += size;

    // Move the data after 'APP0 field' and leave space for H264
    pjpeg_old = ph264_start;
    pjpeg_new = (pjpeg_old + h264_pack_size);
    move_size = size_jpeg - 4 - size;
    memmove(pjpeg_new, pjpeg_old, move_size);

    return ph264_start;
}

/* @brief Package a frame of H264 into mjpeg
 *
 * @param [in, out] frame_jpeg Mjpeg frame buffer address
 * @param [in]      size_jpeg  Mjpeg frame size
 * @param [in]      len_jpeg   Mjpeg frame buffer size
 * @param [in]      frame_h264 H264 frame buffer address
 * @param [in]      size_h264  H264 frame size
 * @param [in]      fps        H264 FPS
 *
 * @return The total length of valid data in frame_jpeg.
 */
unsigned int pack_h264_to_jpeg(unsigned char* frame_jpeg, unsigned int size_jpeg, unsigned int len_jpeg,
        unsigned char* frame_h264, unsigned int size_h264, unsigned int fps)
{
    unsigned int ret, total_size;
    unsigned char* ph264 = NULL;

    total_size = size_jpeg;
    ph264 = refactoring_jpeg(frame_jpeg, size_jpeg, len_jpeg, size_h264);
    if (ph264 == NULL) {
        return total_size;
    }

    ret = merge_h264_to_jpeg(ph264, frame_h264, size_h264, (unsigned short)fps);
    total_size += ret;

    return total_size;
}

#if 0
static void dexdump(const unsigned char* str, unsigned int size)
{
    unsigned int i;

    for (i = 0; i < size; i++) {
        printf("0x%02X, ", str[i]);
    }

    printf("\n");
}

int main(unsigned int argc, unsigned char* argv[])
{
    unsigned char frame_jpeg[2048] = {0xFF, 0xD8, 0xFF, 0xE0, 0x00, 0x10, 0x4A, 0x46,
        0x49, 0x46, 0x00, 0x01, 0x01, 0x00, 0x00, 0x01,
        0x00, 0x01, 0x00, 0x00, 0xFF, 0xDB, 0x00, 0xC5,
        0x00, 0x28, 0x1C, 0x1E, 0x23, 0x1E, 0x19, 0x28,
        0x23, 0x21, 0x23, 0x2D, 0x2B, 0x28, 0x30, 0x3C};
    unsigned char frame_h264[61];
    unsigned char i;
    unsigned int size;

    for (i = 0; i < sizeof(frame_h264); i++) {
        frame_h264[i] = i;
    }

    size = pack_h264_to_jpeg(frame_jpeg, 40, 1024, frame_h264, sizeof(frame_h264), 15);
    dexdump(frame_jpeg, size);

    return 0;
}
#endif
