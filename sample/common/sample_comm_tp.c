#include <stdio.h>
#include "sample_comm_tp.h"
#include "sample_comm.h"

xmedia_s32 sample_comm_tp9963_decoder_init(tp9963_chn ch, tp9963_fmt fmt, tp9963_std std)
{
    xmedia_u32 tmp;
    const xmedia_uchar reg42_43[] = { 0x01, 0x02, 0x00, 0x00, 0x03 };
    const xmedia_uchar mask42_43[] = { 0xfe, 0xfd, 0xff, 0xff, 0xfc };

    sample_comm_tp_i2c_write(0x40, ch);
    sample_comm_tp_i2c_write(0x06, 0x12); // default value
    sample_comm_tp_i2c_write(0x50, 0x00); // VIN1/3
    sample_comm_tp_i2c_write(0x51, 0x00); //
    sample_comm_tp_i2c_write(0x54, 0x03);

    if (fmt == TP_FMT_HD25) {
        sample_comm_tp_i2c_read(0x42, &tmp);
        tmp |= reg42_43[ch];
        sample_comm_tp_i2c_write(0x42, tmp);

        sample_comm_tp_i2c_read(0x43, &tmp);
        tmp &= mask42_43[ch];
        sample_comm_tp_i2c_write(0x43, tmp);

        sample_comm_tp_i2c_write(0x02, 0x42);
        sample_comm_tp_i2c_write(0x07, 0xc0);
        sample_comm_tp_i2c_write(0x0b, 0xc0);
        sample_comm_tp_i2c_write(0x0c, 0x13);
        sample_comm_tp_i2c_write(0x0d, 0x50);

        sample_comm_tp_i2c_write(0x15, 0x13);
        sample_comm_tp_i2c_write(0x16, 0x15);
        sample_comm_tp_i2c_write(0x17, 0x00);
        sample_comm_tp_i2c_write(0x18, 0x19);
        sample_comm_tp_i2c_write(0x19, 0xd0);
        sample_comm_tp_i2c_write(0x1a, 0x25);
        sample_comm_tp_i2c_write(0x1c, 0x07); // 1280*720, 25fps
        sample_comm_tp_i2c_write(0x1d, 0xbc); // 1280*720, 25fps

        sample_comm_tp_i2c_write(0x20, 0x30);
        sample_comm_tp_i2c_write(0x21, 0x84);
        sample_comm_tp_i2c_write(0x22, 0x36);
        sample_comm_tp_i2c_write(0x23, 0x3c);

        sample_comm_tp_i2c_write(0x2b, 0x60);
        sample_comm_tp_i2c_write(0x2c, 0x2a);
        sample_comm_tp_i2c_write(0x2d, 0x30);
        sample_comm_tp_i2c_write(0x2e, 0x70);

        sample_comm_tp_i2c_write(0x30, 0x48);
        sample_comm_tp_i2c_write(0x31, 0xbb);
        sample_comm_tp_i2c_write(0x32, 0x2e);
        sample_comm_tp_i2c_write(0x33, 0x90);

        sample_comm_tp_i2c_write(0x35, 0x25);
        sample_comm_tp_i2c_write(0x39, 0x08);

        if (std == TP_STD_HDA) {
            sample_comm_tp_i2c_write(0x02, 0x46);

            sample_comm_tp_i2c_write(0x0d, 0x71);
            sample_comm_tp_i2c_write(0x18, 0x1b);

            sample_comm_tp_i2c_write(0x20, 0x40);
            sample_comm_tp_i2c_write(0x21, 0x46);

            sample_comm_tp_i2c_write(0x25, 0xfe);
            sample_comm_tp_i2c_write(0x26, 0x01);

            sample_comm_tp_i2c_write(0x2c, 0x3a);
            sample_comm_tp_i2c_write(0x2d, 0x5a);
            sample_comm_tp_i2c_write(0x2e, 0x40);

            sample_comm_tp_i2c_write(0x30, 0x9e);
            sample_comm_tp_i2c_write(0x31, 0x20);
            sample_comm_tp_i2c_write(0x32, 0x10);
            sample_comm_tp_i2c_write(0x33, 0x90);
        }
    } else if (fmt == TP_FMT_HD30) {
        sample_comm_tp_i2c_read(0x42, &tmp);
        tmp |= reg42_43[ch];
        sample_comm_tp_i2c_write(0x42, tmp);

        sample_comm_tp_i2c_read(0x43, &tmp);
        tmp &= mask42_43[ch];
        sample_comm_tp_i2c_write(0x43, tmp);

        sample_comm_tp_i2c_write(0x02, 0x42);
        sample_comm_tp_i2c_write(0x07, 0xc0);
        sample_comm_tp_i2c_write(0x0b, 0xc0);
        sample_comm_tp_i2c_write(0x0c, 0x13);
        sample_comm_tp_i2c_write(0x0d, 0x50);

        sample_comm_tp_i2c_write(0x15, 0x13);
        sample_comm_tp_i2c_write(0x16, 0x15);
        sample_comm_tp_i2c_write(0x17, 0x00);
        sample_comm_tp_i2c_write(0x18, 0x19);
        sample_comm_tp_i2c_write(0x19, 0xd0);
        sample_comm_tp_i2c_write(0x1a, 0x25);
        sample_comm_tp_i2c_write(0x1c, 0x06); // 1280*720, 30fps
        sample_comm_tp_i2c_write(0x1d, 0x72); // 1280*720, 30fps

        sample_comm_tp_i2c_write(0x20, 0x30);
        sample_comm_tp_i2c_write(0x21, 0x84);
        sample_comm_tp_i2c_write(0x22, 0x36);
        sample_comm_tp_i2c_write(0x23, 0x3c);

        sample_comm_tp_i2c_write(0x2b, 0x60);
        sample_comm_tp_i2c_write(0x2c, 0x2a);
        sample_comm_tp_i2c_write(0x2d, 0x30);
        sample_comm_tp_i2c_write(0x2e, 0x70);

        sample_comm_tp_i2c_write(0x30, 0x48);
        sample_comm_tp_i2c_write(0x31, 0xbb);
        sample_comm_tp_i2c_write(0x32, 0x2e);
        sample_comm_tp_i2c_write(0x33, 0x90);

        sample_comm_tp_i2c_write(0x35, 0x25);
        sample_comm_tp_i2c_write(0x39, 0x08);

        if (std == TP_STD_HDA) {
            sample_comm_tp_i2c_write(0x02, 0x46);

            sample_comm_tp_i2c_write(0x0d, 0x70);
            sample_comm_tp_i2c_write(0x18, 0x1b);
            sample_comm_tp_i2c_write(0x20, 0x40);
            sample_comm_tp_i2c_write(0x21, 0x46);

            sample_comm_tp_i2c_write(0x25, 0xfe);
            sample_comm_tp_i2c_write(0x26, 0x01);

            sample_comm_tp_i2c_write(0x2c, 0x3a);
            sample_comm_tp_i2c_write(0x2d, 0x5a);
            sample_comm_tp_i2c_write(0x2e, 0x40);

            sample_comm_tp_i2c_write(0x30, 0x9d);
            sample_comm_tp_i2c_write(0x31, 0xca);
            sample_comm_tp_i2c_write(0x32, 0x01);
            sample_comm_tp_i2c_write(0x33, 0xd0);
        }
    } else if (fmt == TP_FMT_FHD25) {
        sample_comm_tp_i2c_read(0x42, &tmp);
        tmp &= mask42_43[ch];
        sample_comm_tp_i2c_write(0x42, tmp);

        sample_comm_tp_i2c_read(0x43, &tmp);
        tmp &= mask42_43[ch];
        sample_comm_tp_i2c_write(0x43, tmp);

        sample_comm_tp_i2c_write(0x02, 0x40);
        sample_comm_tp_i2c_write(0x07, 0xc0);
        sample_comm_tp_i2c_write(0x0b, 0xc0);
        sample_comm_tp_i2c_write(0x0c, 0x03);
        sample_comm_tp_i2c_write(0x0d, 0x50);

        sample_comm_tp_i2c_write(0x15, 0x03);
        sample_comm_tp_i2c_write(0x16, 0xd2);
        sample_comm_tp_i2c_write(0x17, 0x80);
        sample_comm_tp_i2c_write(0x18, 0x29);
        sample_comm_tp_i2c_write(0x19, 0x38);
        sample_comm_tp_i2c_write(0x1a, 0x47);

        sample_comm_tp_i2c_write(0x1c, 0x0a); // 1920*1080, 25fps
        sample_comm_tp_i2c_write(0x1d, 0x50); //

        sample_comm_tp_i2c_write(0x20, 0x30);
        sample_comm_tp_i2c_write(0x21, 0x84);
        sample_comm_tp_i2c_write(0x22, 0x36);
        sample_comm_tp_i2c_write(0x23, 0x3c);

        sample_comm_tp_i2c_write(0x2b, 0x60);
        sample_comm_tp_i2c_write(0x2c, 0x2a);
        sample_comm_tp_i2c_write(0x2d, 0x30);
        sample_comm_tp_i2c_write(0x2e, 0x70);

        sample_comm_tp_i2c_write(0x30, 0x48);
        sample_comm_tp_i2c_write(0x31, 0xbb);
        sample_comm_tp_i2c_write(0x32, 0x2e);
        sample_comm_tp_i2c_write(0x33, 0x90);

        sample_comm_tp_i2c_write(0x35, 0x05);
        sample_comm_tp_i2c_write(0x39, 0x0C);

        if (std == TP_STD_HDA) {
            sample_comm_tp_i2c_write(0x02, 0x44);

            sample_comm_tp_i2c_write(0x0d, 0x73);

            sample_comm_tp_i2c_write(0x15, 0x01);
            sample_comm_tp_i2c_write(0x16, 0xf0);
            sample_comm_tp_i2c_write(0x18, 0x2a);
            sample_comm_tp_i2c_write(0x20, 0x3c);
            sample_comm_tp_i2c_write(0x21, 0x46);

            sample_comm_tp_i2c_write(0x25, 0xfe);
            sample_comm_tp_i2c_write(0x26, 0x0d);

            sample_comm_tp_i2c_write(0x2c, 0x3a);
            sample_comm_tp_i2c_write(0x2d, 0x54);
            sample_comm_tp_i2c_write(0x2e, 0x40);

            sample_comm_tp_i2c_write(0x30, 0xa5);
            sample_comm_tp_i2c_write(0x31, 0x86);
            sample_comm_tp_i2c_write(0x32, 0xfb);
            sample_comm_tp_i2c_write(0x33, 0x60);
        }
    } else if (fmt == TP_FMT_FHD30) {
        sample_comm_tp_i2c_read(0x42, &tmp);
        tmp &= mask42_43[ch];
        sample_comm_tp_i2c_write(0x42, tmp);

        sample_comm_tp_i2c_read(0x43, &tmp);
        tmp &= mask42_43[ch];
        sample_comm_tp_i2c_write(0x43, tmp);

        sample_comm_tp_i2c_write(0x02, 0x40);
        sample_comm_tp_i2c_write(0x07, 0xc0);
        sample_comm_tp_i2c_write(0x0b, 0xc0);
        sample_comm_tp_i2c_write(0x0c, 0x03);
        sample_comm_tp_i2c_write(0x0d, 0x50);

        sample_comm_tp_i2c_write(0x15, 0x03);
        sample_comm_tp_i2c_write(0x16, 0xd2);
        sample_comm_tp_i2c_write(0x17, 0x80);
        sample_comm_tp_i2c_write(0x18, 0x29);
        sample_comm_tp_i2c_write(0x19, 0x38);
        sample_comm_tp_i2c_write(0x1a, 0x47);
        sample_comm_tp_i2c_write(0x1c, 0x08); // 1920*1080, 30fps
        sample_comm_tp_i2c_write(0x1d, 0x98); //

        sample_comm_tp_i2c_write(0x20, 0x30);
        sample_comm_tp_i2c_write(0x21, 0x84);
        sample_comm_tp_i2c_write(0x22, 0x36);
        sample_comm_tp_i2c_write(0x23, 0x3c);

        sample_comm_tp_i2c_write(0x2b, 0x60);
        sample_comm_tp_i2c_write(0x2c, 0x2a);
        sample_comm_tp_i2c_write(0x2d, 0x30);
        sample_comm_tp_i2c_write(0x2e, 0x70);

        sample_comm_tp_i2c_write(0x30, 0x48);
        sample_comm_tp_i2c_write(0x31, 0xbb);
        sample_comm_tp_i2c_write(0x32, 0x2e);
        sample_comm_tp_i2c_write(0x33, 0x90);

        sample_comm_tp_i2c_write(0x35, 0x05);
        sample_comm_tp_i2c_write(0x39, 0x0C);

        if (std == TP_STD_HDA) {
            sample_comm_tp_i2c_write(0x02, 0x44);

            sample_comm_tp_i2c_write(0x0d, 0x72);

            sample_comm_tp_i2c_write(0x15, 0x01);
            sample_comm_tp_i2c_write(0x16, 0xf0);
            sample_comm_tp_i2c_write(0x18, 0x2a);

            sample_comm_tp_i2c_write(0x20, 0x38);
            sample_comm_tp_i2c_write(0x21, 0x46);

            sample_comm_tp_i2c_write(0x25, 0xfe);
            sample_comm_tp_i2c_write(0x26, 0x0d);

            sample_comm_tp_i2c_write(0x2c, 0x3a);
            sample_comm_tp_i2c_write(0x2d, 0x54);
            sample_comm_tp_i2c_write(0x2e, 0x40);

            sample_comm_tp_i2c_write(0x30, 0xa5);
            sample_comm_tp_i2c_write(0x31, 0x95);
            sample_comm_tp_i2c_write(0x32, 0xe0);
            sample_comm_tp_i2c_write(0x33, 0x60);
        }
    } else if (fmt == TP_FMT_QHD25) {
            sample_comm_tp_i2c_read(0x42, &tmp);
            tmp &= mask42_43[ch];
            sample_comm_tp_i2c_write(0x42, tmp);

            sample_comm_tp_i2c_read(0x43, &tmp);
            tmp &= mask42_43[ch];
            sample_comm_tp_i2c_write(0x43, tmp);

            sample_comm_tp_i2c_write(0x02, 0x50);
            sample_comm_tp_i2c_write(0x07, 0xc0);
            sample_comm_tp_i2c_write(0x0b, 0xc0);
            sample_comm_tp_i2c_write(0x0c, 0x03);
            sample_comm_tp_i2c_write(0x0d, 0x50);

            sample_comm_tp_i2c_write(0x15, 0x23);
            sample_comm_tp_i2c_write(0x16, 0x1b);
            sample_comm_tp_i2c_write(0x17, 0x00);
            sample_comm_tp_i2c_write(0x18, 0x38);
            sample_comm_tp_i2c_write(0x19, 0xa0);
            sample_comm_tp_i2c_write(0x1a, 0x5a);
            sample_comm_tp_i2c_write(0x1c, 0x0f);  //2560*1440, 25fps
            sample_comm_tp_i2c_write(0x1d, 0x76);  //

            sample_comm_tp_i2c_write(0x20, 0x50);
            sample_comm_tp_i2c_write(0x21, 0x84);
            sample_comm_tp_i2c_write(0x22, 0x36);
            sample_comm_tp_i2c_write(0x23, 0x3c);

            sample_comm_tp_i2c_write(0x27, 0xad);

            sample_comm_tp_i2c_write(0x2b, 0x60);
            sample_comm_tp_i2c_write(0x2c, 0x2a);
            sample_comm_tp_i2c_write(0x2d, 0x58);
            sample_comm_tp_i2c_write(0x2e, 0x70);

            sample_comm_tp_i2c_write(0x30, 0x74);
            sample_comm_tp_i2c_write(0x31, 0x58);
            sample_comm_tp_i2c_write(0x32, 0x9f);
            sample_comm_tp_i2c_write(0x33, 0x60);

            sample_comm_tp_i2c_write(0x35, 0x15);
            sample_comm_tp_i2c_write(0x36, 0xdc);
            sample_comm_tp_i2c_write(0x38, 0x40);
            sample_comm_tp_i2c_write(0x39, 0x48);
            if (std == TP_STD_HDA) {
                sample_comm_tp_i2c_read(0x14, &tmp);
                tmp &= 0x9f;
                sample_comm_tp_i2c_write(0x14, tmp);
                sample_comm_tp_i2c_write(0x13, 0x00);
                sample_comm_tp_i2c_write(0x15, 0x23);
                sample_comm_tp_i2c_write(0x16, 0x16);
                sample_comm_tp_i2c_write(0x18, 0x32);
                sample_comm_tp_i2c_write(0x1c, 0x8f);
                sample_comm_tp_i2c_write(0x1d, 0x76);
                sample_comm_tp_i2c_write(0x0d, 0x70);
                sample_comm_tp_i2c_write(0x20, 0x80);
                sample_comm_tp_i2c_write(0x21, 0x86);
                sample_comm_tp_i2c_write(0x22, 0x36);
                sample_comm_tp_i2c_write(0x2b, 0x60);
                sample_comm_tp_i2c_write(0x2d, 0xa0);
                sample_comm_tp_i2c_write(0x2e, 0x40);
                sample_comm_tp_i2c_write(0x30, 0x48);
                sample_comm_tp_i2c_write(0x31, 0x6f);
                sample_comm_tp_i2c_write(0x32, 0xb5);
                sample_comm_tp_i2c_write(0x33, 0x80);
                sample_comm_tp_i2c_write(0x39, 0x40);
            }
    } else {
        SAMPLE_PRT("Unsupported fmt %d\n", fmt);
        return XMEDIA_FAILURE;
    }

    sample_comm_tp_i2c_read(0x06, &tmp);
    tmp |= 0x80;
    sample_comm_tp_i2c_write(0x06, tmp);

    return XMEDIA_SUCCESS;
}

xmedia_s32 sample_comm_tp9963_set_mipi_mode(tp9963_mipi_mode mode)
{
    // mipi setting
    sample_comm_tp_i2c_write(0x40, TP_CH_MIPI_PAGE); // MIPI page
    sample_comm_tp_i2c_write(0x02, 0x78);
    sample_comm_tp_i2c_write(0x03, 0x70);
    sample_comm_tp_i2c_write(0x04, 0x70);
    sample_comm_tp_i2c_write(0x05, 0x70);
    sample_comm_tp_i2c_write(0x06, 0x70);

    sample_comm_tp_i2c_write(0x13, 0xef);

    sample_comm_tp_i2c_write(0x20, 0x00);
    sample_comm_tp_i2c_write(0x21, 0x22);
    sample_comm_tp_i2c_write(0x22, 0x30);
    sample_comm_tp_i2c_write(0x23, 0x9e);

    // colorbar
//    sample_comm_tp_i2c_write(0x20, 0x80); // TP_ENA, Test function to enable BT.1120 test pattern output to mipi module

    if (mode == TP_MIPI_MODE_2CH2LANE_594M) {
        sample_comm_tp_i2c_write(0x21, 0x22);

        sample_comm_tp_i2c_write(0x14, 0x00);
        sample_comm_tp_i2c_write(0x15, 0x01);

        sample_comm_tp_i2c_write(0x2a, 0x08);
        sample_comm_tp_i2c_write(0x2b, 0x08);
        sample_comm_tp_i2c_write(0x2c, 0x10);
        sample_comm_tp_i2c_write(0x2e, 0x0a);

        sample_comm_tp_i2c_write(0x10, 0xa0);
        sample_comm_tp_i2c_write(0x10, 0x20);
    } else if (mode == TP_MIPI_MODE_1CH2LANE_594M) {
        sample_comm_tp_i2c_write(0x21, 0x12);

        sample_comm_tp_i2c_write(0x14, 0x00);
        sample_comm_tp_i2c_write(0x15, 0x01);

        sample_comm_tp_i2c_write(0x2a, 0x08);
        sample_comm_tp_i2c_write(0x2b, 0x08);
        sample_comm_tp_i2c_write(0x2c, 0x10);
        sample_comm_tp_i2c_write(0x2e, 0x0a);

        sample_comm_tp_i2c_write(0x10, 0xa0);
        sample_comm_tp_i2c_write(0x10, 0x20);
    } else if (mode == TP_MIPI_MODE_2CH2LANE_297M) {
        sample_comm_tp_i2c_write(0x21, 0x22);

        sample_comm_tp_i2c_write(0x14, 0x41);
        sample_comm_tp_i2c_write(0x15, 0x02);

        sample_comm_tp_i2c_write(0x2a, 0x04);
        sample_comm_tp_i2c_write(0x2b, 0x03);
        sample_comm_tp_i2c_write(0x2c, 0x09);
        sample_comm_tp_i2c_write(0x2e, 0x02);

        sample_comm_tp_i2c_write(0x10, 0xa0);
        sample_comm_tp_i2c_write(0x10, 0x20);
    } else if (mode == TP_MIPI_MODE_1CH2LANE_297M) {
        sample_comm_tp_i2c_write(0x21, 0x12);

        sample_comm_tp_i2c_write(0x14, 0x41);
        sample_comm_tp_i2c_write(0x15, 0x02);

        sample_comm_tp_i2c_write(0x2a, 0x04);
        sample_comm_tp_i2c_write(0x2b, 0x03);
        sample_comm_tp_i2c_write(0x2c, 0x09);
        sample_comm_tp_i2c_write(0x2e, 0x02);

        sample_comm_tp_i2c_write(0x10, 0xa0);
        sample_comm_tp_i2c_write(0x10, 0x20);
    } else if (mode == TP_MIPI_MODE_2CH4LANE_594M) {
        sample_comm_tp_i2c_write(0x21, 0x24);

        sample_comm_tp_i2c_write(0x14, 0x00);
        sample_comm_tp_i2c_write(0x15, 0x00);

        sample_comm_tp_i2c_write(0x2a, 0x08);
        sample_comm_tp_i2c_write(0x2b, 0x08);
        sample_comm_tp_i2c_write(0x2c, 0x10);
        sample_comm_tp_i2c_write(0x2e, 0x0a);

        sample_comm_tp_i2c_write(0x10, 0xa0);
        sample_comm_tp_i2c_write(0x10, 0x20);
    } else if (mode == TP_MIPI_MODE_2CH4LANE_297M) {
        sample_comm_tp_i2c_write(0x21, 0x24);

        sample_comm_tp_i2c_write(0x14, 0x41);
        sample_comm_tp_i2c_write(0x15, 0x01);

        sample_comm_tp_i2c_write(0x2a, 0x04);
        sample_comm_tp_i2c_write(0x2b, 0x03);
        sample_comm_tp_i2c_write(0x2c, 0x09);
        sample_comm_tp_i2c_write(0x2e, 0x02);

        sample_comm_tp_i2c_write(0x10, 0xa0);
        sample_comm_tp_i2c_write(0x10, 0x20);
    } else if (mode == TP_MIPI_MODE_1CH4LANE_297M) {
        sample_comm_tp_i2c_write(0x21, 0x14);

        sample_comm_tp_i2c_write(0x14, 0x41);
        sample_comm_tp_i2c_write(0x15, 0x01);

        sample_comm_tp_i2c_write(0x2a, 0x04);
        sample_comm_tp_i2c_write(0x2b, 0x03);
        sample_comm_tp_i2c_write(0x2c, 0x09);
        sample_comm_tp_i2c_write(0x2e, 0x02);

        sample_comm_tp_i2c_write(0x10, 0xa0);
        sample_comm_tp_i2c_write(0x10, 0x20);
    } else if (mode == TP_MIPI_MODE_2CH2LANE_432M) {
        sample_comm_tp_i2c_write(0x21, 0x22);

        sample_comm_tp_i2c_write(0x13, 0x0f);
        sample_comm_tp_i2c_write(0x12, 0x5e);

        sample_comm_tp_i2c_write(0x14, 0x00);
        sample_comm_tp_i2c_write(0x15, 0x01);

        sample_comm_tp_i2c_write(0x2a, 0x06);
        sample_comm_tp_i2c_write(0x2b, 0x05);
        sample_comm_tp_i2c_write(0x2c, 0x0d);
        sample_comm_tp_i2c_write(0x2e, 0x0a);

        sample_comm_tp_i2c_write(0x10, 0xa0);
        sample_comm_tp_i2c_write(0x10, 0x20);
    } else if (mode == TP_MIPI_MODE_2CH4LANE_216M) {
        sample_comm_tp_i2c_write(0x21, 0x24);

        sample_comm_tp_i2c_write(0x13, 0x0f);
        sample_comm_tp_i2c_write(0x12, 0x5e);

        sample_comm_tp_i2c_write(0x14, 0x41);
        sample_comm_tp_i2c_write(0x15, 0x01);

        sample_comm_tp_i2c_write(0x2a, 0x06);
        sample_comm_tp_i2c_write(0x2b, 0x05);
        sample_comm_tp_i2c_write(0x2c, 0x0d);
        sample_comm_tp_i2c_write(0x2e, 0x0a);

        sample_comm_tp_i2c_write(0x10, 0xa0);
        sample_comm_tp_i2c_write(0x10, 0x20);
    } else if (mode == TP_MIPI_MODE_2CH2LANE_445M) {
        sample_comm_tp_i2c_write(0x21, 0x22);

        sample_comm_tp_i2c_write(0x13, 0x07);
        sample_comm_tp_i2c_write(0x12, 0x5f);

        sample_comm_tp_i2c_write(0x14, 0x00);
        sample_comm_tp_i2c_write(0x15, 0x01);

        sample_comm_tp_i2c_write(0x2a, 0x06);
        sample_comm_tp_i2c_write(0x2b, 0x05);
        sample_comm_tp_i2c_write(0x2c, 0x0d);
        sample_comm_tp_i2c_write(0x2e, 0x0a);

        sample_comm_tp_i2c_write(0x10, 0xa0);
        sample_comm_tp_i2c_write(0x10, 0x20);
    } else {
        SAMPLE_PRT("Unsupported mipi mode %d\n", mode);
        return XMEDIA_FAILURE;
    }

    /* Enable MIPI CSI2 output */
    sample_comm_tp_i2c_write(0x28, 0x02);
    sample_comm_tp_i2c_write(0x28, 0x00);

    return XMEDIA_SUCCESS;
}

xmedia_s32 sample_comm_tp9963_init(sample_comm_tp9963_cfg *cfg)
{
    xmedia_s32 i;
    xmedia_s32 ret;

    ret = sample_comm_tp_i2c_init(cfg->i2c_dev);
    if (ret != XMEDIA_SUCCESS) {
        return ret;
    }

    for (i = 0; i < TP_MAX_CHN; i++) {
        if (cfg->chn[i] != TP_CH_1 && cfg->chn[i] != TP_CH_2 && cfg->chn[i] != TP_CH_ALL) {
            continue;
        }
        ret = sample_comm_tp9963_decoder_init(cfg->chn[i], cfg->fmt[i], cfg->std[i]);
        if (ret != XMEDIA_SUCCESS) {
            return ret;
        }
    }

    ret = sample_comm_tp9963_set_mipi_mode(cfg->mipi_mode);
    if (ret != XMEDIA_SUCCESS) {
        return ret;
    }

    return XMEDIA_SUCCESS;
}

xmedia_s32 sample_comm_tp9963_deinit(xmedia_void)
{
    /* Disable MIPI CSI2 output */
    sample_comm_tp_i2c_write(0x40, TP_CH_MIPI_PAGE);
    sample_comm_tp_i2c_write(0x28, 0x00);
    sample_comm_tp_i2c_write(0x28, 0x02);

    sample_comm_tp_i2c_exit();

    return XMEDIA_SUCCESS;
}
