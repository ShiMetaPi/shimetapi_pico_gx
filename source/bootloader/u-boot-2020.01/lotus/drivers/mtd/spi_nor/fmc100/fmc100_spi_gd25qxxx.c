/*
 * Copyright (c) LOTUS. All rights reserved.
 */

#define GD_SPI_CMD_RDSR1        0x35    /* Read Status Register-1 */

/*
 * enable QE bit if QUAD read write is supported by GD "25qxxx" SPI
 */
 #ifndef CONFIG_DTR_MODE_SUPPORT
static void clear_dtr_mode_gd(struct fmc_spi *spi)
{
	unsigned int regval;
	struct fmc_host *host = (struct fmc_host *)spi->host;
	unsigned char config;
	unsigned short reg;

	config = spi_general_get_flash_register(spi, SPI_CMD_RDSR3);
	fmc_pr(DTR_DB, "Get Status Register-3[%#x]\n", config);
	regval = fmc_read(host, FMC_GLOBAL_CFG);
	if ((regval >> DTR_MODE_REQUEST_SHIFT) & 0x1) {
		regval &= (~(1 << DTR_MODE_REQUEST_SHIFT));
		regval = fmc_write(host, FMC_GLOBAL_CFG, regval);
	}

	if (config & 0x1) {
		config &= (0xfe);
		reg = ((unsigned short)config);
		writew(reg, host->iobase);
		spi->driver->write_enable(spi);
		fmc100_op_reg(spi, SPI_CMD_WRSR3, sizeof(unsigned char),
                              fmc_op_write_data_en(ENABLE));
	}
}
#endif
static void set_cmd(struct fmc_spi* const spi, u8 cmd, u8 len)
{
	struct fmc_host *host = (struct fmc_host *)spi->host;
	unsigned int regval;

	regval = fmc_cmd_cmd1(cmd);
	fmc_write(host, FMC_CMD, regval);
	fmc_pr(QE_DBG, "\t|-Set CMD[%#x]%#x\n", FMC_CMD, regval);

	regval = op_cfg_fm_cs(spi->chipselect) | OP_CFG_OEN_EN;
	fmc_write(host, FMC_OP_CFG, regval);
	fmc_pr(QE_DBG, "\t|-Set OP_CFG[%#x]%#x\n", FMC_OP_CFG, regval);

	regval = fmc_data_num_cnt(len);
	fmc_write(host, FMC_DATA_NUM, regval);
	fmc_pr(QE_DBG, "\t|-Set DATA_NUM[%#x]%#x\n", FMC_DATA_NUM, regval);

	regval = fmc_op_cmd1_en(ENABLE) |
		 fmc_op_write_data_en(ENABLE) |
		 FMC_OP_REG_OP_START;
	fmc_write(host, FMC_OP, regval);
	fmc_pr(QE_DBG, "\t|-Set OP[%#x]%#x\n", FMC_OP, regval);

	fmc_cmd_wait_cpu_finish(host);

	spi->driver->wait_ready(spi);
}

static int spi_gd25q256_entry_4addr(struct fmc_spi *spi, int enable)
{
	return 0;
}

static int gd_16pin_qe_enable(struct fmc_spi * const spi, int op)
{
	struct fmc_host *host = (struct fmc_host *)spi->host;
	unsigned char config;
	unsigned char status;
	const char *str[] = {"Disable", "Enable"};

#ifndef CONFIG_DTR_MODE_SUPPORT
	clear_dtr_mode_gd(spi);
#endif
	config = spi_general_get_flash_register(spi, GD_SPI_CMD_RDSR1);
	fmc_pr(QE_DBG, "\t|-Read GD SR-1[%#x], val: %#x\n", GD_SPI_CMD_RDSR1,
	       config);
	if (op && (op == spi_nor_get_qe_by_cr(config))) {
		fmc_pr(QE_DBG, "\t* Quad was %sd, status:%#x\n", str[op],
		       config);
		return op;
	}

	/* First, we enable/disable QE for 16Pin GD flash, use WRSR[01h] cmd */
	fmc_pr(QE_DBG, "\t|-First, 16Pin GD flash %s Quad.\n", str[op]);

	status = spi_general_get_flash_register(spi, SPI_CMD_RDSR);
	fmc_pr(QE_DBG, "\t|-Read Status Register[%#x]%#x\n", SPI_CMD_RDSR,
	       status);

	spi->driver->write_enable(spi);

	if (op)
		config |= SPI_NOR_CR_QE_MASK;
	else
		config &= ~SPI_NOR_CR_QE_MASK;
	writeb(status, host->iobase);
	writeb(config, host->iobase + SPI_NOR_SR_LEN);
	fmc_pr(QE_DBG, "\t|-Write IO[%p]%#x\n", host->iobase,
	       *(unsigned short *)host->iobase);

	set_cmd(spi, SPI_CMD_WRSR, SPI_NOR_SR_LEN + SPI_NOR_CR_LEN);

	config = spi_general_get_flash_register(spi, GD_SPI_CMD_RDSR1);
	fmc_pr(QE_DBG, "\t|-Read GD SR-1[%#x], val: %#x\n", GD_SPI_CMD_RDSR1,
	       config);
	if (op == spi_nor_get_qe_by_cr(config)) {
		fmc_pr(QE_DBG, "\t|-16P %s Quad success reg: %#x\n", str[op],
		       config);
		return op;
	} else {
		fmc_pr(QE_DBG, "\t|-16P %s Quad failed, reg: %#x\n", str[op],
		       config);
	}

	return 0;
}

static void gd_8pin_qe_enable(struct fmc_spi * const spi, int op)
{
	unsigned char config;
	unsigned char status;
	const char *str[] = {"Disable", "Enable"};
	struct fmc_host *host = (struct fmc_host *)spi->host;

	fmc_pr(QE_DBG, "\t|-Second, 8Pin GD flash %s Quad.\n", str[op]);

	status = spi_general_get_flash_register(spi, SPI_CMD_RDSR);
	fmc_pr(QE_DBG, "\t|-Read Status Register[%#x]:%#x\n", SPI_CMD_RDSR,
	       status);
	if (!(status & STATUS_WEL_MASK))
		spi->driver->write_enable(spi);

	config = spi_general_get_flash_register(spi, SPI_CMD_RDSR2);
	fmc_pr(QE_DBG, "\t|-Read SR-2[%#x], val: %#x\n", SPI_CMD_RDSR2,
	       config);

	if (op && (op == spi_nor_get_qe_by_cr(config))) {
		fmc_pr(QE_DBG, "\t* Quad was %sd, status:%#x\n", str[op],
		       config);
		return;
	}

	if (op)
		config |= SPI_NOR_CR_QE_MASK;
	else
		config &= ~SPI_NOR_CR_QE_MASK;

	writeb(config, host->iobase);
	fmc_pr(QE_DBG, "\t|-Write IO[%p]%#x\n", host->iobase,
	       *(unsigned char *)host->iobase);

	set_cmd(spi, SPI_CMD_WRSR2, SPI_NOR_CR_LEN);

	config = spi_general_get_flash_register(spi, SPI_CMD_RDSR2);
	fmc_pr(QE_DBG, "\t|-Read GD SR-2[%#x], val: %#x\n", SPI_CMD_RDSR2,
	       config);
	if (op == spi_nor_get_qe_by_cr(config))
		fmc_pr(QE_DBG, "\t|-8P %s Quad success, reg: %#x.\n", str[op],
		       config);
	else
		db_msg("Error: %s Quad failed, reg: %#x\n", str[op], config);

	return;
}

static int spi_gd25qxxx_qe_enable(struct fmc_spi *spi)
{
	unsigned char op;
	const char *str[] = {"Disable", "Enable"};
	if (!spi || !spi->host)
		return -1;
	op = spi_is_quad(spi);

	fmc_pr(QE_DBG, "\t*-Start GD SPI nor %s Quad.\n", str[op]);

	/* First, we enable/disable QE for 16Pin GD flash, use WRSR[01h] cmd */
	if (gd_16pin_qe_enable(spi, op))
		goto qe_end;

	/* Second, we enable/disable QE for 8Pin GD flash, use WRSR2[31h] cmd */
	gd_8pin_qe_enable(spi, op);

qe_end:
	/* Enable the reset pin when working on dual mode for 8PIN */
	if (!op)
		spi_nor_reset_pin_enable(spi, ENABLE);

	fmc_pr(QE_DBG, "\t*-End GD SPI nor %s Quad end.\n", str[op]);
	return op;
}
#ifdef CONFIG_DTR_MODE_SUPPORT
void spi_gd_set_reg(struct fmc_spi *spi)
{
	unsigned int regval;
	struct fmc_host *host = (struct fmc_host *)spi->host;

	regval = fmc_cmd_cmd1(SPI_CMD_WRSR3);
	fmc_write(host, FMC_CMD, regval);
	fmc_pr(DTR_DB, " Set CMD[%#x]%#x\n", FMC_CMD, regval);

	regval = op_cfg_fm_cs(spi->chipselect) | OP_CFG_OEN_EN;
	fmc_write(host, FMC_OP_CFG, regval);
	fmc_pr(DTR_DB, " Set OP_CFG[%#x]%#x\n", FMC_OP_CFG, regval);

	regval = fmc_data_num_cnt(SPI_NOR_SR_LEN);
	fmc_write(host, FMC_DATA_NUM, regval);
	fmc_pr(DTR_DB, " Set DATA_NUM[%#x]%#x\n", FMC_DATA_NUM, regval);

	regval = fmc_op_cmd1_en(ENABLE) 	|
		fmc_op_write_data_en(ENABLE) 	|
		FMC_OP_REG_OP_START;
	fmc_write(host, FMC_OP, regval);
	fmc_pr(DTR_DB, " Set OP[%#x]%#x\n", FMC_OP, regval);
}

int spi_gd_output_driver_strength_set(struct fmc_spi *spi, int dtr_en)
{
	unsigned char config;
	unsigned char reg;
	unsigned char val;
	unsigned int ix;
	struct fmc_host *host = NULL;

	/* DC  |  Numbers of Dummy clock cycles|  Quad IO DTR Read     */
	/*   0(default)|       8       |   66      */
	/*   1         |       10      |   80      */
	unsigned int str_dummy[] = {
		DTR_DUMMY_CYCLES_8,      dtr_rdcr_dc_mask(0),
		DTR_DUMMY_CYCLES_10,     dtr_rdcr_dc_mask(1),
		0,      0,
	};
	val = 0;
	if (!spi || !spi->driver)
		return -1;
	host = (struct fmc_host *)spi->host;
	if (!host)
		return -1;
	/* get the RDCR and RDSR */
	spi->driver->wait_ready(spi);
	/* setting the DC value to match high system clock */
	config = spi_general_get_flash_register(spi, SPI_CMD_RDSR3);
	fmc_pr(DTR_DB, "Get Status Register-3[%#x]\n", config);

	if (dtr_en == ENABLE) {
		/* setting DC value */
		fmc_pr(DTR_DB, "Get the dummy value[%#x]\n", spi->read->dummy);
		/* Only the element with an even number of arrays is required, so increase is 2 */
		for (ix = 0; str_dummy[ix]; ix += _2B) {
			if (spi->read->dummy < str_dummy[ix])
				break;
			val = (unsigned char)str_dummy[ix + 1];
		}
	} else {
		val = dtr_rdcr_dc_mask(0);
	}

	reg = dtr_gd_dc_bit_clr(config) | val;
	fmc_pr(DTR_DB, "Get the reg value[%#x]\n", reg);

	spi->driver->write_enable(spi);
	writew(reg, host->iobase);
	fmc_pr(DTR_DB, "Write IO[%p]%#x\n", host->iobase,
	       *(unsigned short *)host->iobase);
	spi_gd_set_reg(spi);
	fmc_cmd_wait_cpu_finish(host);

	config = spi_general_get_flash_register(spi, SPI_CMD_RDSR3);
	fmc_pr(DTR_DB, "Get Status Register-3[%#x]\n", config);
	if ((config & 0x1) != (unsigned char)val) {
		printf("* Set DC dummy fail.\n");
		return -1;
	}
	return 0;
}
#endif /* CONFIG_DTR_MODE_SUPPORT */