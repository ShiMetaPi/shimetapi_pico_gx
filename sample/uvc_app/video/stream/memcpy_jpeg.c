#include <stdio.h>
#include <string.h>

#define MARK_SOI  0xFFD8
#define MARK_APP0 0xFFE0
#define MARK_DQT  0xFFDB
#define MARK_SOF0 0xFFC0
#define MARK_DHT  0xFFC4
#define MARK_SOS  0xFFDA
#define MARK_DRI  0xFFDD
#define MARK_DOI  0xFFD9

#define SPLIT_DHT   1
/* 7205 7606, There are 3 quantitative tables (DQT0 DQT1 DQT2)
 * 7203 7206, There are 2 quantitative tables (DQT0 DQT1) */
#define REMOVE_DQT2 0
#define REMOVE_DRI  1

/*
 * Delete DRI field in JPEG, delete quantization table 2 (DQT2), split Huffman table.
 */
void memcpy_jpeg_user(unsigned char* src_buffer, unsigned int src_len,
        unsigned char* dest_buffer, unsigned int dest_length, unsigned int* dest_offset)
{
    unsigned int mark, sec_len;
    unsigned int copy_size;
    unsigned int src_off = 0;
    unsigned int dst_len = dest_length;
    unsigned int dst_off = *dest_offset;
    unsigned char* src_buf = src_buffer;
    unsigned char* dst_buf = (dest_buffer + dst_off);

    while ((src_len - src_off) > 0) {
        mark    = ((src_buf[0] << 8) | src_buf[1]);
        sec_len = ((src_buf[2] << 8) | src_buf[3]);

        switch (mark) {
            case MARK_SOI:
                //printf("MARK_SOI\n");
                /* copy SOI */
                copy_size = 2;
                copy_size = ((dst_len - dst_off) > copy_size) ? copy_size : (dst_len - dst_off);
                memcpy(dst_buf, src_buf, copy_size);
                src_buf += 2;
                src_off += 2;
                dst_buf += copy_size;
                dst_off += copy_size;
                break;

            case MARK_APP0:
                /* copy APP0 */
                //printf("MARK_APP0\n");
                copy_size = (2 + sec_len);
                copy_size = ((dst_len - dst_off) > copy_size) ? copy_size : (dst_len - dst_off);
                memcpy(dst_buf, src_buf, copy_size);
                src_buf += (2 + sec_len);
                src_off += (2 + sec_len);
                dst_buf += copy_size;
                dst_off += copy_size;
                break;

            case MARK_DQT:
                //printf("MARK_DQT\n");
                copy_size = (2 + sec_len);
#if REMOVE_DQT2
                /* Delete the third quantification table */ 
                /* After JPEG encoding on the Xmdeia platform, there are 3 quantization tables, 
                 * while some decoding platforms only support 2 quantization tables.*/
                if ((dst_len - dst_off) > copy_size) {
                    /* copy DQT0 and refactoring DQT */
                    dst_buf[0] = 0xFF;
                    dst_buf[1] = 0xDB;
                    dst_buf[2] = 0x00;
                    dst_buf[3] = 0x43;
                    dst_buf[4] = 0x00; /* DQT0 */
                    dst_buf += 5;
                    dst_off += 5;
                    src_buf += 5;
                    memcpy(dst_buf, src_buf, 0x40); /* 0x40: DQT0 size */
                    dst_buf += 0x40;
                    dst_off += 0x40;
                    src_buf += 0x40;

                    /* copy DQT1 and refactoring DQT*/
                    dst_buf[0] = 0xFF;
                    dst_buf[1] = 0xDB;
                    dst_buf[2] = 0x00;
                    dst_buf[3] = 0x43;
                    dst_buf[4] = 0x01; /* DQT1 */
                    dst_buf += 5;
                    dst_off += 5;
                    src_buf += 1;
                    memcpy(dst_buf, src_buf, 0x40); /* 0x40: DQT1 size */
                    dst_buf += 0x40;
                    dst_off += 0x40;
                    src_buf += 0x40;
#if 0
                    /* remove DQT2 */
                    src_buf += 0x41;
                    src_off += (2 + sec_len);
#endif
                } else {
                    copy_size = ((dst_len - dst_off) > copy_size) ? copy_size : (dst_len - dst_off);
                    memcpy(dst_buf, src_buf, copy_size);
                    src_buf += (2 + sec_len);
                    src_off += (2 + sec_len);
                    dst_buf += copy_size;
                    dst_off += copy_size;
                }
#else
                copy_size = ((dst_len - dst_off) > copy_size) ? copy_size : (dst_len - dst_off);
                memcpy(dst_buf, src_buf, copy_size);
                src_buf += (2 + sec_len);
                src_off += (2 + sec_len);
                dst_buf += copy_size;
                dst_off += copy_size;
#endif
                break;

            case MARK_SOF0:
                //printf("MARK_SOF0\n");
                copy_size = (2 + sec_len);
#if 0
                /* copy SOF0 and refactoring DQT */
                if ((dst_len - dst_off) > copy_size) {
                    memcpy(dst_buf, src_buf, copy_size);
                    dst_buf[copy_size - 1] = 0x01; /* refactoring DQT info */
                    src_buf += (2 + sec_len);
                    src_off += (2 + sec_len);
                    dst_buf += copy_size;
                    dst_off += copy_size;
                } else {
                    copy_size = ((dst_len - dst_off) > copy_size) ? copy_size : (dst_len - dst_off);
                    memcpy(dst_buf, src_buf, copy_size);
                    src_buf += (2 + sec_len);
                    src_off += (2 + sec_len);
                    dst_buf += copy_size;
                    dst_off += copy_size;
                }
#else
                copy_size = ((dst_len - dst_off) > copy_size) ? copy_size : (dst_len - dst_off);
                memcpy(dst_buf, src_buf, copy_size);
                src_buf += (2 + sec_len);
                src_off += (2 + sec_len);
                dst_buf += copy_size;
                dst_off += copy_size;
#endif
                break;

            case MARK_DHT:
                /* copy DHT*/
                //printf("MARK_DHT\n");
                copy_size = (2 + sec_len);
#if SPLIT_DHT
                /* Split Huffman table */
                /* The Huffman table encoded by JPEG on the XMedia platform consists of four sets in a single DHT.
                 * Some decoding platforms do not support this and need to split it into four sets */
                if ((dst_len - dst_off) > copy_size) {
                    /* copy DHT0 */
                    dst_buf[0] = 0xFF;
                    dst_buf[1] = 0xC4;
                    dst_buf[2] = 0x00;
                    dst_buf[3] = 0x1F;
                    dst_buf += 4;
                    dst_off += 4;
                    src_buf += 4;
                    copy_size = 0x1F - 2;
                    memcpy(dst_buf, src_buf, copy_size);
                    dst_buf += copy_size;
                    dst_off += copy_size;
                    src_buf += copy_size;

                    /* copy DHT1 */
                    dst_buf[0] = 0xFF;
                    dst_buf[1] = 0xC4;
                    dst_buf[2] = 0x00;
                    dst_buf[3] = 0xB5;
                    dst_buf += 4;
                    dst_off += 4;
                    copy_size = 0xB5 - 2;
                    memcpy(dst_buf, src_buf, copy_size);
                    dst_buf += copy_size;
                    dst_off += copy_size;
                    src_buf += copy_size;

                    /* copy DHT2 */
                    dst_buf[0] = 0xFF;
                    dst_buf[1] = 0xC4;
                    dst_buf[2] = 0x00;
                    dst_buf[3] = 0x1F;
                    dst_buf += 4;
                    dst_off += 4;
                    copy_size = 0x1F - 2;
                    memcpy(dst_buf, src_buf, copy_size);
                    dst_buf += copy_size;
                    dst_off += copy_size;
                    src_buf += copy_size;

                    /* copy DHT3 */
                    dst_buf[0] = 0xFF;
                    dst_buf[1] = 0xC4;
                    dst_buf[2] = 0x00;
                    dst_buf[3] = 0xB5;
                    dst_buf += 4;
                    dst_off += 4;
                    copy_size = 0xB5 - 2;
                    memcpy(dst_buf, src_buf, copy_size);
                    dst_buf += copy_size;
                    dst_off += copy_size;
                    src_buf += copy_size;

                    src_off += (2 + sec_len);
                } else {
                    copy_size = ((dst_len - dst_off) > copy_size) ? copy_size : (dst_len - dst_off);
                    memcpy(dst_buf, src_buf, copy_size);
                    src_buf += (2 + sec_len);
                    src_off += (2 + sec_len);
                    dst_buf += copy_size;
                    dst_off += copy_size;
                }
#else
                copy_size = ((dst_len - dst_off) > copy_size) ? copy_size : (dst_len - dst_off);
                memcpy(dst_buf, src_buf, copy_size);
                src_buf += (2 + sec_len);
                src_off += (2 + sec_len);
                dst_buf += copy_size;
                dst_off += copy_size;
#endif
                break;

            case MARK_DRI:
                //printf("MARK_DRI\n");
                copy_size = (2 + sec_len);
#if REMOVE_DRI
                /* remove DRI */
                src_buf += copy_size;
                src_off += copy_size;
#else
                copy_size = ((dst_len - dst_off) > copy_size) ? copy_size : (dst_len - dst_off);
                memcpy(dst_buf, src_buf, copy_size);
                src_buf += (2 + sec_len);
                src_off += (2 + sec_len);
                dst_buf += copy_size;
                dst_off += copy_size;
#endif
                break;

            case MARK_SOS:
                /* copy SOS */
                //printf("MARK_SOS\n");
            case MARK_DOI:
                /* copy DOI */
            default:
                /* copy data */
                copy_size = src_len - src_off;
                copy_size = ((dst_len - dst_off) > copy_size) ? copy_size : (dst_len - dst_off);
                memcpy(dst_buf, src_buf, copy_size);
                dst_off += copy_size;
                *dest_offset = dst_off;
                return;
        }
    }

    *dest_offset = dst_off;
}
