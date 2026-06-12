/*
 * Copyright (c) LOTUS. All rights reserved.
 */

static void enable_card_clk(struct sdhci_host *host)
{
	u16 clk;

	clk = sdhci_readw(host, SDHCI_CLOCK_CONTROL);
	clk |= SDHCI_CLOCK_CARD_EN;
	sdhci_writew(host, clk, SDHCI_CLOCK_CONTROL);
}

#ifdef MMC_SUPPORTS_TUNING
static void disable_card_clk(struct sdhci_host *host)
{
	u16 clk;

	clk = sdhci_readw(host, SDHCI_CLOCK_CONTROL);
	clk &= ~SDHCI_CLOCK_CARD_EN;
	sdhci_writew(host, clk, SDHCI_CLOCK_CONTROL);
}
#endif

static void enable_internal_clk(struct sdhci_host *host)
{
	u16 clk;
	u16 timeout = 20;

	clk = sdhci_readw(host, SDHCI_CLOCK_CONTROL);
	clk |= SDHCI_CLOCK_INT_EN | SDHCI_CLOCK_PLL_EN;
	sdhci_writew(host, clk, SDHCI_CLOCK_CONTROL);

	/* Wait max 20 ms */
	clk = sdhci_readw(host, SDHCI_CLOCK_CONTROL);
	while (!(clk & SDHCI_CLOCK_INT_STABLE)) {
		if (timeout == 0) {
			printf("%s: Internal clock never stabilised.\n",
					__func__);
			return;
		}
		timeout--;
		udelay(1000); /* delay 1000us */
		clk = sdhci_readw(host, SDHCI_CLOCK_CONTROL);
	}
}

static void __maybe_unused disable_internal_clk(struct sdhci_host *host)
{
	u16 clk;

	clk = sdhci_readw(host, SDHCI_CLOCK_CONTROL);
	clk &= ~SDHCI_CLOCK_INT_EN;
	sdhci_writew(host, clk, SDHCI_CLOCK_CONTROL);
}

static void set_drv_phase(struct sdhci_host *host, unsigned int phase)
{
	uintptr_t crg_addr;
	unsigned int reg;

	crg_addr = (host->type == MMC_TYPE_MMC) ?
		REG_EMMC_DRV_DLL_CTRL : REG_SDIO0_DRV_DLL_CTRL;
	reg = readl(crg_addr);
	reg &= ~SDIO_DRV_PHASE_SEL_MASK;
	reg |= sdio_drv_sel(phase);
	writel(reg, crg_addr);
}

static void enable_sample(struct sdhci_host *host)
{
	unsigned int reg;

	reg = sdhci_readl(host, SDHCI_AT_CTRL);
	reg |= SDHCI_SAMPLE_EN;
	sdhci_writel(host, reg, SDHCI_AT_CTRL);
}

static void set_sampl_phase(struct sdhci_host *host, unsigned int phase)
{
	unsigned int reg;

	reg = sdhci_readl(host, SDHCI_AT_STAT);
	reg &= ~SDHCI_PHASE_SEL_MASK;
	reg |= phase;
	sdhci_writel(host, reg, SDHCI_AT_STAT);
}

static void wait_sampl_dll_slave_ready(struct sdhci_host *host)
{
	unsigned int reg;
	unsigned int timeout = 20;
	uintptr_t reg_addr;

	reg_addr = (host->type == MMC_TYPE_MMC) ?
		REG_EMMC_SAMPL_DLL_STATUS : REG_SDIO0_SAMPL_DLL_STATUS;
	do {
		reg = readl(reg_addr);
		if (reg & SDIO_SAMPL_DLL_SLAVE_READY)
			return;

		udelay(1000); /* delay 1000us */
		timeout--;
	} while (timeout > 0);

	printf("sdhci: SAMPL DLL slave not ready.\n");
}

#ifdef MMC_SUPPORTS_TUNING
static void enable_edge_tuning(struct sdhci_host *host)
{
	uintptr_t crg_addr;
	unsigned int reg;

	crg_addr = (host->type == MMC_TYPE_MMC) ?
		REG_EMMC_SAMPLB_DLL_CTRL : REG_SDIO0_SAMPLB_DLL_CTRL;

	reg = readl(crg_addr);
	reg &= ~((host->type == MMC_TYPE_MMC) ?
		EMMC_SAMPLB_DLL_CLK_MASK : SDIO_SAMPLB_DLL_CLK_MASK);

#ifdef CONFIG_LOTUS_FPGA
	/* fpga 1 -> 45 degree  */
	reg |= ((host->type == MMC_TYPE_MMC) ?
		emmc_samplb_sel(2) : sdio_samplb_sel(2));
#else
	/* soc 1-> 11.25 degree  */
	reg |= ((host->type == MMC_TYPE_MMC) ?
		emmc_samplb_sel(8) : sdio_samplb_sel(8)); /* sel 8 */
#endif

	writel(reg, crg_addr);

	reg = sdhci_readl(host, SDHCI_MULTI_CYCLE);
	reg |= SDHCI_EDGE_DETECT_EN;
	sdhci_writel(host, reg, SDHCI_MULTI_CYCLE);
}

static void disable_edge_tuning(struct sdhci_host *host)
{
	unsigned int reg;

	reg = sdhci_readl(host, SDHCI_MULTI_CYCLE);
	reg &= ~SDHCI_EDGE_DETECT_EN;
	sdhci_writel(host, reg, SDHCI_MULTI_CYCLE);
}

static void select_sampl_phase(struct sdhci_host *host, unsigned int phase)
{
	disable_card_clk(host);
	set_sampl_phase(host, phase);
	wait_sampl_dll_slave_ready(host);
	enable_card_clk(host);
	udelay(100); /* delay 100us */
}

static int send_tuning(struct sdhci_host *host, u32 opcode, int* cmd_error)
{
	int count, err;
	const int tuning_num = 1;

	count = 0;
	do {
		err = mmc_send_tuning(host->mmc, opcode, NULL);
		if (err)
			break;

		count++;
	} while (count < tuning_num);

	return err;
}

static int lotus_mmc_exec_tuning(struct sdhci_host *host, unsigned int opcode)
{
	unsigned int index, val;
	unsigned int edge_p2f, edge_f2p, start, end, phase;
	unsigned int fall, rise, fall_updat_flag;
	unsigned int found;
	unsigned int prev_found = 0;
	int err;
	int prev_err = 0;
	unsigned short ctrl;

	wait_drv_dll_lock(host);
	enable_sampl_dll_slave(host);
	enable_sample(host);
	enable_edge_tuning(host);
	host->is_tuning = 1;

	start = 0;
	end = PHASE_SCALE / EDGE_TUNING_PHASE_STEP;

	edge_p2f = start;
	edge_f2p = end;
	for (index = 0; index <= end; index++) {
		select_sampl_phase(host, index * EDGE_TUNING_PHASE_STEP);

		err = mmc_send_tuning(host->mmc, opcode, NULL);
		if (!err) {
			val = sdhci_readl(host, SDHCI_MULTI_CYCLE);
			found = val & SDHCI_FOUND_EDGE;
		} else {
			found = 1;
		}

		if (prev_found && !found)
			edge_f2p = index;
		else if (!prev_found && found)
			edge_p2f = index;

		if ((edge_p2f != start) && (edge_f2p != end))
			break;

		prev_found = found;
	}

	if ((edge_p2f == start) && (edge_f2p == end)) {
		printf("sdhci: tuning failed! can not found edge!\n");
		return -1;
	}

	disable_edge_tuning(host);

	start = edge_p2f * EDGE_TUNING_PHASE_STEP;
	end = edge_f2p * EDGE_TUNING_PHASE_STEP;
	if (end <= start)
		end += PHASE_SCALE;

	fall = start;
	rise = end;
	fall_updat_flag = 0;
	for (index = start; index <= end; index++) {
		select_sampl_phase(host, index % PHASE_SCALE);

		err = send_tuning(host, opcode, NULL);
		if (err)
			debug("sdhci: send tuning CMD%u fail! phase:%u err:%d\n",
				opcode, index, err);

		if (err && index == start) {
			if (!fall_updat_flag) {
				fall_updat_flag = 1;
				fall = start;
			}
		} else {
			if (!prev_err && err) {
				if (!fall_updat_flag) {
					fall_updat_flag = 1;
					fall = index;
				}
			}
		}

		if (prev_err && !err)
			rise = index;

		if (err && index == end)
			rise = end;

		prev_err = err;
	}

	phase = ((fall + rise) / 2 + PHASE_SCALE / 2) % PHASE_SCALE; /* 2 for cal average */

	printf("sdhci: tuning done! valid phase shift [%u, %u] Final Phase:%u\n",
			rise % PHASE_SCALE, fall % PHASE_SCALE, phase);

	host->tuning_phase = phase;
	select_sampl_phase(host, phase);

	ctrl = sdhci_readw(host, SDHCI_HOST_CONTROL2);
	ctrl |= SDHCI_CTRL_TUNED_CLK;
	sdhci_writew(host, ctrl, SDHCI_HOST_CONTROL2);
	host->is_tuning = 0;

	return 0;
}
#endif

void sdhci_set_host_caps(struct sdhci_host *host)
{
	host->host_caps = MMC_MODE_HS | MMC_MODE_HS_52MHz |
			  MMC_MODE_DDR_52MHz | MMC_MODE_HS200 | MMC_MODE_4BIT;
#ifdef CONFIG_MMC_8BIT
	host->host_caps |= MMC_MODE_8BIT;
#endif
#if !defined(CONFIG_AUTO_SD_UPDATE) && defined(CONFIG_TARGET_XMFALCON) && defined(CONFIG_MMC_HS400_SUPPORT)
	host->host_caps |= MMC_MODE_HS400 |
			   MMC_MODE_8BIT;
#if defined(CONFIG_MMC_HS400_ES_SUPPORT)
	host->host_caps |= MMC_MODE_HS400_ES;
#endif
#endif
}

int sdhci_add_port(int index, uintptr_t regbase, u32 type)
{
	struct sdhci_host *host = NULL;

	if (type == MMC_TYPE_MMC)
		emmc_hardware_init();
	else
		sd_hardware_init();

	host = calloc(1, sizeof(struct sdhci_host));
	if (host == NULL) {
		puts("sdhci_host malloc fail!\n");
		return -ENOMEM;
	}

	host->name = "sdhci";
	host->index = index;
	host->type = type;
	host->ioaddr = (void *)regbase;
	host->quirks = 0;
	host->set_clock = lotus_mmc_set_clk;
	host->priv_init = lotus_mmc_priv_init;
	host->set_control_reg = lotus_mmc_set_ioconfig;
#ifdef MMC_SUPPORTS_TUNING
	host->execute_tuning = lotus_mmc_exec_tuning;
#endif
#ifdef CONFIG_TARGET_XMORCA
	host->set_io_cfg = lotus_set_io_cfg;
#endif
	sdhci_set_host_caps(host);
	add_sdhci(host, CONFIG_LOTUS_SDHCI_MAX_FREQ, MIN_FREQ);
	return 0;
}

void sdhci_hs400_enhanced_stobe(struct mmc *mmc, bool enable)
{
	struct sdhci_host *host = mmc->priv;
	u32 ctrl;

	ctrl = sdhci_readl(host, SDHCI_EMMC_CTRL);
	if (enable)
		ctrl |= SDHCI_ENH_STROBE_EN;
	else
		ctrl &= ~SDHCI_ENH_STROBE_EN;

	sdhci_writel(host, ctrl, SDHCI_EMMC_CTRL);

#if 0 //FIXME: XXX
	ctrl  = sdhci_readl(host, SDHCI_MULTI_CYCLE);
	if (enable)
		ctrl |= SDHCI_CMD_DLY_EN;
	else
		ctrl &= ~SDHCI_CMD_DLY_EN;

	sdhci_writel(host, ctrl, SDHCI_MULTI_CYCLE);
#endif
}

static void print_mmcinfo(struct mmc *mmc)
{
	int i;

	printf("Device: %s\n", mmc->cfg->name);
	printf("Manufacturer ID: %x\n", mmc->cid[0] >> 24);
	printf("OEM: %x\n", (mmc->cid[0] >> 8) & 0xffff);
	printf("Name: %c%c%c%c%c\n", mmc->cid[0] & 0xff,
			(mmc->cid[1] >> 24), (mmc->cid[1] >> 16) & 0xff,
			(mmc->cid[1] >> 8) & 0xff, mmc->cid[1] & 0xff);

	printf("Bus Speed: %d\n", mmc->clock);
#if CONFIG_IS_ENABLED(MMC_VERBOSE)
	printf("Mode: %s\n", mmc_mode_name(mmc->selected_mode));
	mmc_dump_capabilities("card capabilities", mmc->card_caps);
	mmc_dump_capabilities("host capabilities", mmc->host_caps);
#endif
	printf("Rd Block Len: %d\n", mmc->read_bl_len);

	printf("%s version %d.%d", IS_SD(mmc) ? "SD" : "MMC",
			EXTRACT_SDMMC_MAJOR_VERSION(mmc->version),
			EXTRACT_SDMMC_MINOR_VERSION(mmc->version));
	if (EXTRACT_SDMMC_CHANGE_VERSION(mmc->version) != 0)
		printf(".%d", EXTRACT_SDMMC_CHANGE_VERSION(mmc->version));
	printf("\n");

	printf("High Capacity: %s\n", mmc->high_capacity ? "Yes" : "No");
	print_to_tool("Capacity: %lld\r\n", mmc->capacity);

	printf("Bus Width: %d-bit%s\n", mmc->bus_width,
			mmc->ddr_mode ? " DDR" : "");

#if CONFIG_IS_ENABLED(MMC_WRITE)
	puts("Erase Group Size: ");
	print_size(((u64)mmc->erase_grp_size) << 9, "\n");
#endif

	if (!IS_SD(mmc) && mmc->version >= MMC_VERSION_4_41) {
		bool has_enh = (mmc->part_support & ENHNCD_SUPPORT) != 0;
		bool usr_enh = has_enh && (mmc->part_attr & EXT_CSD_ENH_USR);

#if CONFIG_IS_ENABLED(MMC_HW_PARTITIONING)
		puts("HC WP Group Size: ");
		print_size(((u64)mmc->hc_wp_grp_size) << 9, "\n");
#endif

		puts("User Capacity: ");
		print_size(mmc->capacity_user, usr_enh ? " ENH" : "");
		if (mmc->wr_rel_set & EXT_CSD_WR_DATA_REL_USR)
			puts(" WRREL\n");
		else
			putc('\n');
		if (usr_enh) {
			puts("User Enhanced Start: ");
			print_size(mmc->enh_user_start, "\n");
			puts("User Enhanced Size: ");
			print_size(mmc->enh_user_size, "\n");
		}
		puts("Boot Capacity: ");
		print_size(mmc->capacity_boot, has_enh ? " ENH\n" : "\n");
		puts("RPMB Capacity: ");
		print_size(mmc->capacity_rpmb, has_enh ? " ENH\n" : "\n");

		for (i = 0; i < ARRAY_SIZE(mmc->capacity_gp); i++) {
			bool is_enh = has_enh &&
				(mmc->part_attr & EXT_CSD_ENH_GP(i));
			if (mmc->capacity_gp[i]) {
				printf("GP%i Capacity: ", i+1);
				print_size(mmc->capacity_gp[i],
					   is_enh ? " ENH" : "");
				if (mmc->wr_rel_set & EXT_CSD_WR_DATA_REL_GP(i))
					puts(" WRREL\n");
				else
					putc('\n');
			}
		}
	}
}

int lotus_mmc_init(int dev_num)
{
	struct mmc *mmc = find_mmc_device(dev_num);
	int ret;

	if (mmc == NULL) {
		printf("mmc device not found!!\n");
		return -EINVAL;
	}

	ret = mmc_init(mmc);
	if (ret)
		return ret;

	print_mmcinfo(mmc);

	if (!IS_SD(mmc))
		return mmc_set_boot_config(mmc);

	return 0;
}

void printf_mmc(int dev_num)
{
	struct mmc *mmc = find_mmc_device(dev_num);

	if (mmc != NULL)
		print_mmcinfo(mmc);
}
