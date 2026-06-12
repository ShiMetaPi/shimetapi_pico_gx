// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2012
 * Armando Visconti, ST Microelectronics, armando.visconti@st.com.
 *
 * (C) Copyright 2018
 * Quentin Schulz, Bootlin, quentin.schulz@bootlin.com
 *
 * Driver for ARM PL022 SPI Controller.
 */

#include <clk.h>
#include <common.h>
#include <dm.h>
#include <dm/platform_data/spi_pl022.h>
#include <asm/io.h>
#include <spi.h>
#include <mapmem.h>

/*
 * This macro is used to define some register default values.
 * reg is masked with mask, the OR:ed with an (again masked)
 * val shifted sb steps to the left.
 */
#define SSP_WRITE_BITS(reg, val, mask, sb) \
((reg) = (((reg) & ~(mask)) | (((val)<<(sb)) & (mask))))

/*
 * This macro is also used to define some default values.
 * It will just shift val by sb steps to the left and mask
 * the result with mask.
 */
#define GEN_MASK_BITS(val, mask, sb) \
(((val)<<(sb)) & (mask))

#define DRIVE_TX		0
#define DO_NOT_DRIVE_TX		1

#define DO_NOT_QUEUE_DMA	0
#define QUEUE_DMA		1

#define RX_TRANSFER		1
#define TX_TRANSFER		2

/*
 * Macros to access SSP Registers with their offsets
 */
#define SSP_CR0(r)	(r + 0x000)
#define SSP_CR1(r)	(r + 0x004)
#define SSP_DR(r)	(r + 0x008)
#define SSP_SR(r)	(r + 0x00C)
#define SSP_CPSR(r)	(r + 0x010)
#define SSP_IMSC(r)	(r + 0x014)
#define SSP_RIS(r)	(r + 0x018)
#define SSP_MIS(r)	(r + 0x01C)
#define SSP_ICR(r)	(r + 0x020)
#define SSP_DMACR(r)	(r + 0x024)
#define SSP_TX_FIFO_CR	(r + 0x028)
#define SSP_RX_FIFO_CR	(r + 0x02C)
#define SSP_CSR(r)	(r + 0x030) /* vendor extension */
#define SSP_ITCR(r)	(r + 0x080)
#define SSP_ITIP(r)	(r + 0x084)
#define SSP_ITOP(r)	(r + 0x088)
#define SSP_TDR(r)	(r + 0x08C)

#define SSP_PID0(r)	(r + 0xFE0)
#define SSP_PID1(r)	(r + 0xFE4)
#define SSP_PID2(r)	(r + 0xFE8)
#define SSP_PID3(r)	(r + 0xFEC)

#define SSP_CID0(r)	(r + 0xFF0)
#define SSP_CID1(r)	(r + 0xFF4)
#define SSP_CID2(r)	(r + 0xFF8)
#define SSP_CID3(r)	(r + 0xFFC)

/*
 * SSP Control Register 0  - SSP_CR0
 */
#define SSP_CR0_MASK_DSS	(0x0FUL << 0)
#define SSP_CR0_MASK_FRF	(0x3UL << 4)
#define SSP_CR0_MASK_SPO	(0x1UL << 6)
#define SSP_CR0_MASK_SPH	(0x1UL << 7)
#define SSP_CR0_SCR_SHIFT	(8)
#define SSP_CR0_MASK_SCR	(0xFFUL << SSP_CR0_SCR_SHIFT)
#define SSP_CR0_BIT_MODE(x)	((x) - 1)

/*
 * SSP Control Register 0  - SSP_CR1
 */
#define SSP_CR1_MASK_LBM	(0x1UL << 0)
#define SSP_CR1_MASK_SSE	(0x1UL << 1)
#define SSP_CR1_MASK_MS		(0x1UL << 2)
#define SSP_CR1_MASK_MD_ALTS	(0x1UL << 4)


/*
 * The lotus version of this block adds some bits
 * in SSP_CR1
 */
#define SSP_CR1_MASK_BIGEND_LOTUS	(0x1UL << 4)
#define SSP_CR1_MASK_ALTASENS_LOTUS	(0x1UL << 6)

/*
 * SSP Status Register - SSP_SR
 */
#define SSP_SR_MASK_TFE		(0x1UL << 0) /* Transmit FIFO empty */
#define SSP_SR_MASK_TNF		(0x1UL << 1) /* Transmit FIFO not full */
#define SSP_SR_MASK_RNE		(0x1UL << 2) /* Receive FIFO not empty */
#define SSP_SR_MASK_RFF		(0x1UL << 3) /* Receive FIFO full */
#define SSP_SR_MASK_BSY		(0x1UL << 4) /* Busy Flag */

/*
 * SSP Clock Prescale Register  - SSP_CPSR
 */
#define SSP_CPSR_MASK_CPSDVSR	(0xFFUL << 0)

/*
 * SSP Interrupt Mask Set/Clear Register - SSP_IMSC
 */
#define SSP_IMSC_MASK_RORIM (0x1UL << 0) /* Receive Overrun Interrupt mask */
#define SSP_IMSC_MASK_RTIM  (0x1UL << 1) /* Receive timeout Interrupt mask */
#define SSP_IMSC_MASK_RXIM  (0x1UL << 2) /* Receive FIFO Interrupt mask */
#define SSP_IMSC_MASK_TXIM  (0x1UL << 3) /* Transmit FIFO Interrupt mask */

/*
 * SSP Raw Interrupt Status Register - SSP_RIS
 */
/* Receive Overrun Raw Interrupt status */
#define SSP_RIS_MASK_RORRIS		(0x1UL << 0)
/* Receive Timeout Raw Interrupt status */
#define SSP_RIS_MASK_RTRIS		(0x1UL << 1)
/* Receive FIFO Raw Interrupt status */
#define SSP_RIS_MASK_RXRIS		(0x1UL << 2)
/* Transmit FIFO Raw Interrupt status */
#define SSP_RIS_MASK_TXRIS		(0x1UL << 3)

/*
 * SSP Masked Interrupt Status Register - SSP_MIS
 */
/* Receive Overrun Masked Interrupt status */
#define SSP_MIS_MASK_RORMIS		(0x1UL << 0)
/* Receive Timeout Masked Interrupt status */
#define SSP_MIS_MASK_RTMIS		(0x1UL << 1)
/* Receive FIFO Masked Interrupt status */
#define SSP_MIS_MASK_RXMIS		(0x1UL << 2)
/* Transmit FIFO Masked Interrupt status */
#define SSP_MIS_MASK_TXMIS		(0x1UL << 3)

/*
 * SSP Interrupt Clear Register - SSP_ICR
 */
/* Receive Overrun Raw Clear Interrupt bit */
#define SSP_ICR_MASK_RORIC		(0x1UL << 0)
/* Receive Timeout Clear Interrupt bit */
#define SSP_ICR_MASK_RTIC		(0x1UL << 1)

/*
 * SSP DMA Control Register - SSP_DMACR
 */
/* Receive DMA Enable bit */
#define SSP_DMACR_MASK_RXDMAE		(0x1UL << 0)
/* Transmit DMA Enable bit */
#define SSP_DMACR_MASK_TXDMAE		(0x1UL << 1)

/*
 * SSP Chip Select Control Register - SSP_CSR
 * (vendor extension)
 */
#define SSP_CSR_CSVALUE_MASK		(0x1FUL << 0)

/*
 * SSP Integration Test control Register - SSP_ITCR
 */
#define SSP_ITCR_MASK_ITEN		(0x1UL << 0)
#define SSP_ITCR_MASK_TESTFIFO		(0x1UL << 1)

/*
 * SSP Integration Test Input Register - SSP_ITIP
 */
#define ITIP_MASK_SSPRXD		 (0x1UL << 0)
#define ITIP_MASK_SSPFSSIN		 (0x1UL << 1)
#define ITIP_MASK_SSPCLKIN		 (0x1UL << 2)
#define ITIP_MASK_RXDMAC		 (0x1UL << 3)
#define ITIP_MASK_TXDMAC		 (0x1UL << 4)
#define ITIP_MASK_SSPTXDIN		 (0x1UL << 5)

/*
 * SSP Integration Test output Register - SSP_ITOP
 */
#define ITOP_MASK_SSPTXD		 (0x1UL << 0)
#define ITOP_MASK_SSPFSSOUT		 (0x1UL << 1)
#define ITOP_MASK_SSPCLKOUT		 (0x1UL << 2)
#define ITOP_MASK_SSPOEn		 (0x1UL << 3)
#define ITOP_MASK_SSPCTLOEn		 (0x1UL << 4)
#define ITOP_MASK_RORINTR		 (0x1UL << 5)
#define ITOP_MASK_RTINTR		 (0x1UL << 6)
#define ITOP_MASK_RXINTR		 (0x1UL << 7)
#define ITOP_MASK_TXINTR		 (0x1UL << 8)
#define ITOP_MASK_INTR			 (0x1UL << 9)
#define ITOP_MASK_RXDMABREQ		 (0x1UL << 10)
#define ITOP_MASK_RXDMASREQ		 (0x1UL << 11)
#define ITOP_MASK_TXDMABREQ		 (0x1UL << 12)
#define ITOP_MASK_TXDMASREQ		 (0x1UL << 13)

/*
 * SSP Test Data Register - SSP_TDR
 */
#define TDR_MASK_TESTDATA		(0xFFFFFFFF)

/*
 * Message State
 * we use the spi_message.state (void *) pointer to
 * hold a single state value, that's why all this
 * (void *) casting is done here.
 */
#define STATE_START			((void *) 0)
#define STATE_RUNNING			((void *) 1)
#define STATE_DONE			((void *) 2)
#define STATE_ERROR			((void *) -1)
#define STATE_TIMEOUT			((void *) -2)

/*
 * SSP State - Whether Enabled or Disabled
 */
#define SSP_DISABLED			(0)
#define SSP_ENABLED			(1)

/*
 * SSP DMA State - Whether DMA Enabled or Disabled
 */
#define SSP_DMA_DISABLED		(0)
#define SSP_DMA_ENABLED			(1)

/*
 * SSP Clock Defaults
 */
#define SSP_DEFAULT_CLKRATE 0x2
#define SSP_DEFAULT_PRESCALE 0x40

/*
 * SSP Clock Parameter ranges
 */
#define CPSDVR_MIN 0x02
#define CPSDVR_MAX 0xFE
#define SCR_MIN 0x00
#define SCR_MAX 0xFF

/*
 * SSP Interrupt related Macros
 */
#define DEFAULT_SSP_REG_IMSC  0x0UL
#define DISABLE_ALL_INTERRUPTS DEFAULT_SSP_REG_IMSC
#define ENABLE_ALL_INTERRUPTS ( \
	SSP_IMSC_MASK_RORIM | \
	SSP_IMSC_MASK_RTIM | \
	SSP_IMSC_MASK_RXIM | \
	SSP_IMSC_MASK_TXIM \
)

#define CLEAR_ALL_INTERRUPTS  0x3

#define XS_SPI_MAX_TIMEOUT 1 * 1000 *1000

/*
 * The type of reading and writing going on this chip
 */
enum xm_ssp_rw_bit {
	XM_RW_BIT_U8,
	XM_RW_BIT_U16
};

struct xmedia_ssp_regs {
	u32 spicr0;
	u32 spicr1;
	u32 spidr;
	u32 spisr;
	u32 spicpsr;
	u32 spiimsc;
	u32 spiris;
	u32 spimis;
	u32 spiicr;
	u32 spidmacr;
	u32 spitxfifocr;
	u32 spirxfifocr;
};

struct xmedia_spi_slave {
	struct spi_slave	slave;
	u32			max_hz;
	enum xm_ssp_rw_bit	rw_mask;
	struct xmedia_ssp_regs	*regs;
};

enum xm_spi_cs_cr_elments {
	XM_SPI_CS_CRG = 0,
	XM_SPI_CS_SHIFT,
	XM_SPI_CS_MASK
};

enum xm_spi_clk_cr_elments {
	XM_SPI_CLK_CRG = 0,
	XM_SPI_RST_BIT_SHIFT,
	XM_SPI_ENABLE_BIT_SHIFT
};

static unsigned long spi_bases[] = {0x12070000UL, 0x12071000UL, 0x12072000UL};

/* SPI0 can only connect 1 slave device, SPI1 can connect 2 slave device*/
static unsigned long spi_cs_num[] = {1, 2};

/* {CS reg, CS shift, mask bit} */
static unsigned long spi_cs_cr[][3]= {
	{0, 0, 0},
	{0x12028000UL, 2, 0x4}
};

/* {CLK reg, soft reset bit, clk enable bit} */
static unsigned long spi_clk_cr[][3] = {
	{0x120101bcUL, 16, 12},
	{0x120101bcUL, 17, 13},
	{0x120101bcUL, 18, 14}
};

static inline struct xmedia_spi_slave *to_xmedia_slave(struct spi_slave *slave)
{
	return container_of(slave, struct xmedia_spi_slave, slave);
}

static int xmedia_wait_mask_set(void *reg, u32 mask, u32 timeout)
{
	while (--timeout) {
		if (readw(reg) & mask)
			break;
		udelay(1);
	}

	return !timeout;
}
#if 0
static int xmedia_wait_mask_clr(void *reg, u32 mask, u32 timeout)
{
	while (--timeout) {
		if (!xm_read_bits(reg, mask))
			break;
		udelay(1);
	}

	return !timeout;
}
#endif
static void flush(struct xmedia_spi_slave *xs)
{
	int count = 10000000;

	do {
		while (readw(&xs->regs->spisr) & SSP_SR_MASK_RNE) {
			readw(&xs->regs->spidr);
		}

		udelay(1);
		count --;
	} while (readw(&xs->regs->spisr) & SSP_SR_MASK_RNE && count);
}

int spi_tx(struct xmedia_spi_slave *xs, u16 tx_value)
{
	if (xmedia_wait_mask_set(&xs->regs->spisr, SSP_SR_MASK_TNF,
	    XS_SPI_MAX_TIMEOUT)) {
		printf("xmedia SPI TX: Timeout waiting for send\n");
		return -ETIMEDOUT;
	}

	writew(tx_value, &xs->regs->spidr);

	return 0;
}

int spi_rx(struct xmedia_spi_slave *xs, void* rxp)
{
	u16 value;

	if (xmedia_wait_mask_set(&xs->regs->spisr, SSP_SR_MASK_RNE,
	    XS_SPI_MAX_TIMEOUT)) {
		printf("xmedia SPI RX: Timeout waiting for data\n");
		return -ETIMEDOUT;
	}

	value = readw(&xs->regs->spidr);
	if (xs->rw_mask == XM_RW_BIT_U8 && rxp) {
		*(u8*)rxp = value & 0xff;
	} else if (xs->rw_mask == XM_RW_BIT_U16 && rxp) {
		*(u16*)rxp = value;
	}

	return 0;
}


int spi_xfer(struct spi_slave *slave, unsigned int bitlen,
	     const void *dout, void *din, unsigned long flags)
{
	struct xmedia_spi_slave *xs = to_xmedia_slave(slave);
	int ret;
	u32 len_tx = 0, len_rx = 0, len;
	const u8 *txp_8 = NULL;
	u8 *rxp_8 = NULL;
	const u16 *txp_16 = NULL;
	u16 *rxp_16 = NULL;

	if (bitlen == 0)
		return 0;

	if (bitlen % slave->wordlen) {
		/* Errors always terminate an ongoing transfer */
		flags |= SPI_XFER_END;
		return -1;
	}

	len = bitlen / slave->wordlen;

	/* No data */
	if (!din && !dout)
		return 0;

	if (xs->rw_mask == XM_RW_BIT_U8) {
		txp_8 = dout;
		rxp_8 = din;
	} else {
		txp_16 = dout;
		rxp_16 = din;
	}

	while (len_tx < len) {
		if (xs->rw_mask == XM_RW_BIT_U8) {
			ret = spi_tx(xs, (txp_8 ? *txp_8++ : 0));
			ret |= spi_rx(xs, rxp_8);
			rxp_8++;
		}
		else {
			ret = spi_tx(xs, (txp_16 ? *txp_16++ : 0));
			ret |= spi_rx(xs, rxp_16);
			rxp_16++;
		}
		if (ret)
			return ret;
		len_tx++;
		len_rx++;
	}

	while (len_rx < len_tx) {
		if (xs->rw_mask == XM_RW_BIT_U8) {
			ret = spi_rx(xs, rxp_8);
			rxp_8++;
		}
		else {
			ret = spi_rx(xs, rxp_16);
			rxp_16++;
		}
		if (ret)
			return ret;
		len_rx++;
	}

	return 0;
}

static inline u32 spi_rate(u32 rate, u16 cpsdvsr, u16 scr)
{
	return rate / (cpsdvsr * (1 + scr));
}

static int xm_set_ssp_busclock(struct xmedia_spi_slave *xs)
{
	/* Lets calculate the frequency parameters */
	u16 cpsdvsr = CPSDVR_MIN, scr = SCR_MIN;
	u32 rate, max_tclk, min_tclk, best_freq = 0, best_cpsdvsr = 0,
		best_scr = 0, tmp, found = 0;
	u32 freq = xs->max_hz;
	u16 reg;

	rate = 100000000;
	/* cpsdvscr = 2 & scr 0 */
	max_tclk = spi_rate(rate, CPSDVR_MIN, SCR_MIN);
	/* cpsdvsr = 254 & scr = 255 */
	min_tclk = spi_rate(rate, CPSDVR_MAX, SCR_MAX);

	if (freq > max_tclk)
		printf("Max speed that can be programmed is %d Hz, you requested %d\n",
			max_tclk, freq);

	if (freq < min_tclk) {
		printf("Requested frequency: %d Hz is less than minimum possible %d Hz\n",
			freq, min_tclk);
		return -EINVAL;
	}

	/*
	 * best_freq will give closest possible available rate (<= requested
	 * freq) for all values of scr & cpsdvsr.
	 */
	while ((cpsdvsr <= CPSDVR_MAX) && !found) {
		while (scr <= SCR_MAX) {
			tmp = spi_rate(rate, cpsdvsr, scr);

			if (tmp > freq) {
				/* we need lower freq */
				scr++;
				continue;
			}

			/*
			 * If found exact value, mark found and break.
			 * If found more closer value, update and break.
			 */
			if (tmp > best_freq) {
				best_freq = tmp;
				best_cpsdvsr = cpsdvsr;
				best_scr = scr;

				if (tmp == freq)
					found = 1;
			}
			/*
			 * increased scr will give lower rates, which are not
			 * required
			 */
			break;
		}
		cpsdvsr += 2;
		scr = SCR_MIN;
	}

	printf("SSP Target Freq is: %u, Effective Freq is %u\n", freq, best_freq);

	reg = readw(&xs->regs->spicr0);
	writew((reg & ~SSP_CR0_MASK_SCR) | best_scr << SSP_CR0_SCR_SHIFT, &xs->regs->spicr0);
	reg = readw(&xs->regs->spicpsr);
	writew((reg & ~SSP_CPSR_MASK_CPSDVSR) | best_cpsdvsr, &xs->regs->spicpsr);

	return 0;
}

void xmedia_clk_init(struct xmedia_spi_slave *xs)
{
	void *clk_reg = map_sysmem(spi_clk_cr[xs->slave.bus][XM_SPI_CLK_CRG], 4);
	u32 rst_bit = spi_clk_cr[xs->slave.bus][XM_SPI_RST_BIT_SHIFT];
	u32 enable_bit = spi_clk_cr[xs->slave.bus][XM_SPI_ENABLE_BIT_SHIFT];

	/* enable SPI clk && set SPI soft reset*/
	writel(readl(clk_reg) | (1 << enable_bit | (1 << rst_bit)), clk_reg);
	udelay(200000);
	/* cancel reset SPI */
	writel(readl(clk_reg) & ~(1 << rst_bit), clk_reg);
}

int spi_cs_is_valid(unsigned int bus, unsigned int cs)
{
	if (bus >= ARRAY_SIZE(spi_bases))
		return 0;

	if (cs > spi_cs_num[bus] - 1)
		return 0;

	return 1;
}

struct spi_slave *spi_setup_slave(unsigned int bus, unsigned int cs,
				  unsigned int max_hz, unsigned int mode)
{
	struct xmedia_spi_slave *xs;

	if (!spi_cs_is_valid(bus, cs)) {
		printf("xmedia_spi: invalid bus %d / chip select %d\n", bus, cs);
		return NULL;
	}

	xs = spi_alloc_slave(struct xmedia_spi_slave, bus, cs);
	if (!xs)
		return NULL;

	xs->max_hz = max_hz;
	xs->slave.mode = mode;
	xs->slave.bus = bus;
	xs->slave.cs = cs;
	xs->regs = map_sysmem(spi_bases[bus], sizeof(struct xmedia_ssp_regs));

	return &xs->slave;
}

void spi_free_slave(struct spi_slave *slave)
{
	struct xmedia_spi_slave *xs = to_xmedia_slave(slave);

	free(xs);
}

int spi_claim_bus(struct spi_slave *slave)
{
	struct xmedia_spi_slave *xs = to_xmedia_slave(slave);
	struct xmedia_ssp_regs *ssp_regs = xs->regs;
	void *cs_reg;
	u32 cs_shfit = spi_cs_cr[slave->bus][XM_SPI_CS_SHIFT];
	u32 cs_mask = spi_cs_cr[slave->bus][XM_SPI_CS_MASK];
	u16 reg = 0, bit_per_world = 0;

	bit_per_world = SSP_CR0_BIT_MODE(slave->wordlen);

	if (bit_per_world <= SSP_CR0_BIT_MODE(8))
		xs->rw_mask = XM_RW_BIT_U8;
	else if (bit_per_world <= SSP_CR0_BIT_MODE(16))
		xs->rw_mask = XM_RW_BIT_U16;

	xmedia_clk_init(xs);

	if (spi_cs_num[slave->bus] > 1) {
		u32 old;
		cs_reg = map_sysmem(spi_cs_cr[slave->bus][XM_SPI_CS_CRG], 4);

		old = readl(cs_reg);
		writel((old & ~cs_mask) | slave->cs << cs_shfit, cs_reg);
	}

	/* Disable SPI */
	writew(readw(&ssp_regs->spicr1) & ~SSP_CR1_MASK_SSE, &ssp_regs->spicr1);

	reg = readw(&ssp_regs->spicr0);
	/* Configure SPI */
	reg |= bit_per_world;
	reg |= (slave->mode & SPI_CPOL) ? SSP_CR0_MASK_SPO : 0;
	reg |= (slave->mode & SPI_CPHA) ? SSP_CR0_MASK_SPH : 0;
	writew(reg, &ssp_regs->spicr0);

	reg = readw(&ssp_regs->spicr1);
	if (slave->mode & SPI_LSB_FIRST)
		reg |= SSP_CR1_MASK_BIGEND_LOTUS;

	if (slave->mode & SPI_LOOP)
		reg |= SSP_CR1_MASK_LBM;

	// reg |= SSP_CR1_MASK_MD_ALTS;
	writew(reg, &ssp_regs->spicr1);

	writew(0, &ssp_regs->spitxfifocr);
	writew(0, &ssp_regs->spirxfifocr);

	if (xm_set_ssp_busclock(xs))
		return 1;

	/* Enable SPI */
	writew(readw(&ssp_regs->spicr1) | SSP_CR1_MASK_SSE, &ssp_regs->spicr1);

	flush(xs);

	return 0;
}

void spi_release_bus(struct spi_slave *slave)
{
	struct xmedia_spi_slave *xs = to_xmedia_slave(slave);
	struct xmedia_ssp_regs *ssp_regs = xs->regs;

	flush(xs);

	/* Disable the SPI hardware */
	writew(readw(&ssp_regs->spicr1) & ~SSP_CR1_MASK_SSE, &ssp_regs->spicr1);
}

