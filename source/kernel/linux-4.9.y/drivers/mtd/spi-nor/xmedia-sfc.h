#ifndef _XMEDIA_SF_H
#define _XMEDIA_SF_H

int bsp_spi_nor_read_reg_with_dummy(struct spi_nor *nor, u8 opcode, u8 *buf, int len, u32 addr, u8 addr_len, u8 dummy_len);

#endif
