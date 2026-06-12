// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2002
 * Gerald Van Baren, Custom IDEAS, vanbaren@cideas.com
 */
/*
 * SPI Read/Write Utilities
 */
#include <common.h>
#include <command.h>
#include <errno.h>
#include <spi.h>
#include <asm/io.h>
/*-----------------------------------------------------------------------
 * Definitions
 */
void spi_write_a9byte(struct spi_slave *slave, unsigned char cmd_dat,unsigned char dat)
{
    unsigned short spi_data = 0;
    if (cmd_dat)
        spi_data = 1 << 8;
    else
        spi_data = 0 << 8;
    spi_data = spi_data | dat;
    spi_xfer(slave, 9, &spi_data, NULL, 0);
    return;
}
void ssp_write_dat(struct spi_slave *slave, unsigned char dat)
{
    spi_write_a9byte(slave, 1, dat);
}
void ssp_write_cmd(struct spi_slave *slave,unsigned char dat)
{
    spi_write_a9byte(slave, 0, dat);
}
int do_lcd_st7789_init (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    struct spi_slave *slave;
    int ret = 0;
    /*
     * We use the last specified parameters, unless new ones are
     * entered.
     */
    if ((flag & CMD_FLAG_REPEAT) == 0) {
        slave = spi_setup_slave(0, 0, 1000000, SPI_MODE_3);
        if (!slave) {
            printf("Invalid device \n");
            return -EINVAL;
        }
        spi_set_wordlen(slave, 9);
        ret = spi_claim_bus(slave);
        if (ret)
            return -EINVAL;
        /* st7789v turn on sleep mode */
        ssp_write_cmd(slave,0x10);
        /* st7789v software reset */
        ssp_write_cmd(slave,0x01);
        udelay(120000);
        /* st7789v turn off sleep mode */
        ssp_write_cmd(slave,0x11);
        udelay(120000);
        /* Display Setting */
        ssp_write_cmd(slave,0x53);
        ssp_write_dat(slave,0x04);
        ssp_write_cmd(slave,0x36);
        ssp_write_dat(slave,0x00);
        ssp_write_cmd(slave,0x3a);
        ssp_write_dat(slave,0x60);
        ssp_write_cmd(slave,0x21);
        /* st7789v frame rate setting */
        ssp_write_cmd(slave,0xb0);
        ssp_write_dat(slave,0x11);
        ssp_write_dat(slave,0x04);
        ssp_write_cmd(slave,0xb2);
        ssp_write_dat(slave,0x05);
        ssp_write_dat(slave,0x05);
        ssp_write_dat(slave,0x00);
        ssp_write_dat(slave,0x33);
        ssp_write_dat(slave,0x33);
        ssp_write_cmd(slave,0xb7);
        ssp_write_dat(slave,0x35);
        /* st7789v power setting */
        ssp_write_cmd(slave,0xb8);
        ssp_write_dat(slave,0x2f);
        ssp_write_dat(slave,0x2b);
        ssp_write_dat(slave,0x2f);
        ssp_write_cmd(slave,0xbb);
        ssp_write_dat(slave,0x20);
        ssp_write_cmd(slave,0xc0);
        ssp_write_dat(slave,0x2c);
        ssp_write_cmd(slave,0xc2);
        ssp_write_dat(slave,0x01);
        ssp_write_cmd(slave,0xc3);
        ssp_write_dat(slave,0x0b);
        ssp_write_cmd(slave,0xc4);
        ssp_write_dat(slave,0x20);
        ssp_write_cmd(slave,0xc6);
        ssp_write_dat(slave,0x11);
        ssp_write_cmd(slave,0xd0);
        ssp_write_dat(slave,0xa4);
        ssp_write_dat(slave,0xa1);
        ssp_write_cmd(slave,0xe8);
        ssp_write_dat(slave,0x03);
        ssp_write_cmd(slave,0xe9);
        ssp_write_dat(slave,0x0d);
        ssp_write_dat(slave,0x12);
        ssp_write_dat(slave,0x00);
        /* st7789v gamma setting */
        ssp_write_cmd(slave,0xe0);
        ssp_write_dat(slave,0xd0);
        ssp_write_dat(slave,0x06);
        ssp_write_dat(slave,0x0b);
        ssp_write_dat(slave,0x0a);
        ssp_write_dat(slave,0x09);
        ssp_write_dat(slave,0x05);
        ssp_write_dat(slave,0x2e);
        ssp_write_dat(slave,0x43);
        ssp_write_dat(slave,0x44);
        ssp_write_dat(slave,0x09);
        ssp_write_dat(slave,0x16);
        ssp_write_dat(slave,0x15);
        ssp_write_dat(slave,0x23);
        ssp_write_dat(slave,0x27);
        ssp_write_cmd(slave,0xe1);
        ssp_write_dat(slave,0xd0);
        ssp_write_dat(slave,0x06);
        ssp_write_dat(slave,0x0b);
        ssp_write_dat(slave,0x09);
        ssp_write_dat(slave,0x08);
        ssp_write_dat(slave,0x06);
        ssp_write_dat(slave,0x2e);
        ssp_write_dat(slave,0x44);
        ssp_write_dat(slave,0x44);
        ssp_write_dat(slave,0x3a);
        ssp_write_dat(slave,0x15);
        ssp_write_dat(slave,0x15);
        ssp_write_dat(slave,0x23);
        ssp_write_dat(slave,0x26);
        ssp_write_cmd(slave,0x29);
        spi_release_bus(slave);
    }
    return 0;
}
/***************************************************/
U_BOOT_CMD(
    lcd_st7789_init,    1,    0,    do_lcd_st7789_init,
    "do lcd st7789 init",
    ""
);

