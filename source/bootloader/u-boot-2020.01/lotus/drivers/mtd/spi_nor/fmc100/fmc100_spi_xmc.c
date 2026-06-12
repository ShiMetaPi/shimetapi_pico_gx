/*
 * Copyright (c) LOTUS. All rights reserved.
 */
#ifdef CONFIG_DTR_MODE_SUPPORT
void spi_xmc_set_reg(struct fmc_spi *spi)
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

int spi_xmc_output_driver_strength_set(struct fmc_spi *spi, int dtr_en)
{
	unsigned char config;
	unsigned short val;
	unsigned int ix;
	struct fmc_host *host = NULL;

	/* DC[1:0]  |  Numbers of Dummy clock cycles|  Quad IO DTR Read     */
	/*   00(default)|       8       |   90      */
	/*   01     |       4       |   66      */
	/*   10     |       6       |   66      */
	/*   11     |       10      |   108     */
	unsigned int str_dummy[] = {
		DTR_DUMMY_CYCLES_4,      dtr_rdcr_dc_mask(1),
		DTR_DUMMY_CYCLES_6,      dtr_rdcr_dc_mask(2),
		DTR_DUMMY_CYCLES_8,      dtr_rdcr_dc_mask(0),
		DTR_DUMMY_CYCLES_10,     dtr_rdcr_dc_mask(3),
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
	config = spi_general_get_flash_register(spi, SPI_CMD_RDCR_MX);
	fmc_pr(DTR_DB, "Get Config Register[%#x]\n", config);

	if (dtr_en == ENABLE) {
		/* setting DC value */
		fmc_pr(DTR_DB, "Get the dummy value[%#x]\n", spi->read->dummy);
		/* Only the element with an even number of arrays is required, so increase is 2 */
		for (ix = 0; str_dummy[ix]; ix += _2B) {
			if (spi->read->dummy < str_dummy[ix])
				break;
			val = (unsigned short)str_dummy[ix + 1];
		}
	} else {
		val = dtr_rdcr_dc_mask(0);
	}

	config = dtr_xmc_dc_bit_clr(config) | val;

	spi->driver->write_enable(spi);
	writeb(config, host->iobase);
	fmc_pr(DTR_DB, "Write IO[%p]%#x\n", host->iobase,
	       *(unsigned short *)host->iobase);
	spi_xmc_set_reg(spi);
	fmc_cmd_wait_cpu_finish(host);

	config = spi_general_get_flash_register(spi, SPI_CMD_RDCR_MX);
	if ((config & 3) != (unsigned char)val) {
		printf("* Set DC dummy fail.\n");
		return -1;
	}
	return 0;
}
#endif /* CONFIG_DTR_MODE_SUPPORT */
