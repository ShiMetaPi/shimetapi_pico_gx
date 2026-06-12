/*
 * Copyright (c) LOTUS. All rights reserved.
 */
//#define DEBUG
#include <config.h>
#include <common.h>
#include <malloc.h>
#include <net.h>
#include <wait_bit.h>
#include <linux/io.h>
#include <linux/mii.h>
#include <linux/iopoll.h>
#include <cpu_func.h>
#include <miiphy.h>
#include <hexdump.h>

#include "ftgmac030.h"
#include "marvell_phy.h"
#include "realtek_phy.h"
#include "sys.h"

#define ETH_ZLEN	60

/* Receive Buffer Size Register - HW default is 0x640 */
#define FTGMAC030_RBSR_DEFAULT_VALUE	0x640

/* PKTBUFSTX/PKTBUFSRX must both be power of 2 */
#define  FTGMAC030_PKTBUFSTX	4	/* must be power of 2 */
#define  FTGMAC030_PKTBUFSRX	PKTBUFSRX /*  must be power of 2 */

/* Timeout for transmit */
#define FTGMAC030_TX_TIMEOUT_MS		1000

/* Timeout for a mdio read/write operation */
#define FTGMAC030_MDIO_TIMEOUT_USEC	10000

#define MAC_LEN 6

struct ftgmac030_data {
	struct ftgmac030_txdes txdes[FTGMAC030_PKTBUFSTX] __attribute__((aligned(ARCH_DMA_MINALIGN)));
	struct ftgmac030_rxdes rxdes[FTGMAC030_PKTBUFSRX] __attribute__((aligned(ARCH_DMA_MINALIGN)));
	int rsvd1 __attribute__((aligned(ARCH_DMA_MINALIGN)));
	int tx_index;
	int rx_index;
	int phy_addr;
	enum ftgmac030_if_mode phy_intf;
};

/*
 * struct mdio functions
 */
int ftgmac030_mdio_read(struct eth_device *dev, int phy_addr,
	int regnum)
{
	struct ftgmac030 *ftgmac030 = (struct ftgmac030 *)dev->iobase;
	int phycr;
	int data;
	int ret;

	phycr = readl(&ftgmac030->phycr);

	/* preserve MDC cycle threshold */
	phycr &= FTGMAC030_PHYCR_MDC_CYCTHR_MASK;

	phycr |= FTGMAC030_PHYCR_PHYAD(phy_addr)
	      |  FTGMAC030_PHYCR_REGAD(regnum)
	      |  FTGMAC030_PHYCR_OP(FTGMAC030_MDIO_OP_RD)
	      |  FTGMAC030_PHYCR_SOF(FTGMAC030_MDIO_SOF)
	      |  FTGMAC030_PHYCR_PHYRD;

	writel(phycr, &ftgmac030->phycr);

	ret = readl_poll_timeout(&ftgmac030->phycr, phycr,
				 !(phycr & FTGMAC030_PHYCR_PHYRD),
				 FTGMAC030_MDIO_TIMEOUT_USEC);
	if (ret) {
		pr_err("mdio read failed (phy:%d reg:%x)\n", phy_addr, regnum);
		return ret;
	}

	data = readl(&ftgmac030->phydata);

	return FTGMAC030_PHYDATA_MIIRDATA(data);
}

int ftgmac030_mdio_write(struct eth_device *dev, int phy_addr,
	int regnum, u16 value)
{
	struct ftgmac030 *ftgmac030 = (struct ftgmac030 *)dev->iobase;
	int phycr;
	int data;
	int ret;

	phycr = readl(&ftgmac030->phycr);

	/* preserve MDC cycle threshold */
	phycr &= FTGMAC030_PHYCR_MDC_CYCTHR_MASK;

	phycr |= FTGMAC030_PHYCR_PHYAD(phy_addr)
	      |  FTGMAC030_PHYCR_REGAD(regnum)
	      |  FTGMAC030_PHYCR_OP(FTGMAC030_MDIO_OP_WR)
	      |  FTGMAC030_PHYCR_SOF(FTGMAC030_MDIO_SOF)
	      |  FTGMAC030_PHYCR_PHYWR;

	data = FTGMAC030_PHYDATA_MIIWDATA(value);

	writel(data, &ftgmac030->phydata);
	writel(phycr, &ftgmac030->phycr);

	ret = readl_poll_timeout(&ftgmac030->phycr, phycr,
				 !(phycr & FTGMAC030_PHYCR_PHYWR),
				 FTGMAC030_MDIO_TIMEOUT_USEC);
	if (ret) {
		pr_err("mdio write failed (phy:%d reg:%x)\n", phy_addr, regnum);
	}

	return ret;
}

/* MDIO Bus Interface */
static int ftgmac030_mdiobus_read(struct mii_dev *bus, int addr, int devad, int reg)
{
	struct eth_device *dev = (struct eth_device *)bus->priv;

	return ftgmac030_mdio_read(dev, addr, reg);
}

static int ftgmac030_mdiobus_write(struct mii_dev *bus, int addr, int devad,
		  int reg, u16 value)
{
	struct eth_device *dev = (struct eth_device *)bus->priv;

	return ftgmac030_mdio_write(dev, addr, reg, value);
}

/*
* Init MIDO MDC CYCTHR
*/
static void ftgmac030_mdc_cycthr(struct eth_device *dev, unsigned int mdc_cycthr)
{
	struct ftgmac030 *ftgmac030 = (struct ftgmac030 *)dev->iobase;
	int phycr;

	phycr = readl(&ftgmac030->phycr);
	phycr &= ~FTGMAC030_PHYCR_MDC_CYCTHR_MASK;
	phycr |= FTGMAC030_PHYCR_MDC_CYCTHR(mdc_cycthr);
	writel(phycr, &ftgmac030->phycr);
}

static void ftgmac030_set_phy_intf(struct eth_device *dev)
{
	struct ftgmac030 *ftgmac030 = (struct ftgmac030 *)dev->iobase;
	struct ftgmac030_data *priv = dev->priv;

	priv->phy_intf = (bspeth_get_phy_intf() ? FTGMAC030_MODE_RGMII : FTGMAC030_MODE_RMII);

	printf("%s (%s)\n", __func__, (FTGMAC030_MODE_RMII == priv->phy_intf) ? "rmii" : "rgmii");

	writel(priv->phy_intf, &ftgmac030->gisr);

	/* set the mdc cycthr */
	/* set the mdc cycthr sys_clk=200M*/
	ftgmac030_mdc_cycthr(dev, 0xc8);
}

int ftgmac030_mdiobus_init(struct eth_device *dev)
{
#if defined(CONFIG_MII) || defined(CONFIG_CMD_MII)
	struct mii_dev *bus = mdio_alloc();

	if (!bus) {
		printf("Failed to allocate MDIO bus\n");
		return -ENOMEM;
	}

	bus->priv = dev;
	bus->read = ftgmac030_mdiobus_read;
	bus->write = ftgmac030_mdiobus_write;
	snprintf(bus->name, sizeof(bus->name), dev->name);

	if (mdio_register(bus)) {
		mdio_free(bus);
		return -1;
	}

	miiphy_set_current_dev(dev->name);

#endif
	return 0;
}

static int ftgmac030_phy_reset(struct eth_device *dev)
{
	struct ftgmac030_data *priv = dev->priv;
	int i;
	u16 status, adv;

	adv = ADVERTISE_CSMA | ADVERTISE_ALL;

	ftgmac030_mdio_write(dev, priv->phy_addr, MII_ADVERTISE, adv);

	printf("%s: Starting autonegotiation...\n", dev->name);

	ftgmac030_mdio_write(dev, priv->phy_addr,
		MII_BMCR, (BMCR_ANENABLE | BMCR_ANRESTART));

	for (i = 0; i < 10000; i++) {
		status = ftgmac030_mdio_read(dev, priv->phy_addr, MII_BMSR);
		if (status & BMSR_ANEGCOMPLETE)
			break;
		mdelay(1);
	}

	if (status & BMSR_ANEGCOMPLETE) {
		printf("%s: Autonegotiation complete\n", dev->name);
	} else {
		printf("%s: Autonegotiation timed out (status=0x%04x)\n",
		       dev->name, status);
		return 0;
	}

	return 1;
}

static int ftgmac030_phy_init(struct eth_device *dev)
{
	struct ftgmac030_data *priv = dev->priv;

	int phy_addr;
	u16 phy_id, status, adv, lpa;
	u16 stat_ge;
	int media, speed, duplex;
	int i;

	/* Check if the PHY is up to snuff... */
	for (phy_addr = 0; phy_addr < 32; phy_addr++) {
		phy_id = ftgmac030_mdio_read(dev, phy_addr, MII_PHYSID1);
		/*
		 * When it is unable to found PHY,
		 * the interface usually return 0xffff or 0x0000
		 */
		if (phy_id != 0xffff && phy_id != 0x0) {
			printf("%s: found PHY(0x%x) at 0x%02x\n",
				dev->name, phy_id, phy_addr);
			priv->phy_addr = phy_addr;
			break;
		}
	}

	if (phy_id == 0xffff || phy_id == 0x0) {
		printf("%s: no PHY present\n", dev->name);
		return 0;
	}

	if (phy_id == PHY_MARVELL)
		marvell_phy_init(dev, phy_addr);

	if (phy_id == PHY_REALTEK)
		realtek_phy_init(dev, phy_addr);

	status = ftgmac030_mdio_read(dev, priv->phy_addr, MII_BMSR);
	if (!(status & BMSR_LSTATUS)) {
		/* Try to re-negotiate if we don't have link already. */
		ftgmac030_phy_reset(dev);

		for (i = 0; i < 100000 / 100; i++) {
			status = ftgmac030_mdio_read(dev, priv->phy_addr, MII_BMSR);
			if (status & BMSR_LSTATUS)
				break;
			udelay(100);
		}
	}

	if (!(status & BMSR_LSTATUS)) {
		printf("%s: link down\n", dev->name);
		return 0;
	}

	if (FTGMAC030_MODE_RGMII == priv->phy_intf) {
		/* 1000 Base-T Status Register */
		stat_ge = ftgmac030_mdio_read(dev, priv->phy_addr, MII_STAT1000);
		speed = (stat_ge & (LPA_1000FULL | LPA_1000HALF)
			 ? 1 : 0);

		duplex = ((stat_ge & LPA_1000FULL)
			 ? 1 : 0);

		if (speed) { /* Speed is 1000 */
			printf("%s: link up, 1000bps %s-duplex\n",
				dev->name, duplex ? "full" : "half");
			return 1;
		}
	}

	adv = ftgmac030_mdio_read(dev, priv->phy_addr, MII_ADVERTISE);
	lpa = ftgmac030_mdio_read(dev, priv->phy_addr, MII_LPA);

	media = mii_nway_result(lpa & adv);
	speed = (media & (ADVERTISE_100FULL | ADVERTISE_100HALF) ? 1 : 0);
	duplex = (media & ADVERTISE_FULL) ? 1 : 0;

	printf("%s: link up, %sMbps %s-duplex\n",
	       dev->name, speed ? "100" : "10", duplex ? "full" : "half");

	return 1;
}

static int ftgmac030_update_link_speed(struct eth_device *dev)
{
	struct ftgmac030 *ftgmac030 = (struct ftgmac030 *)dev->iobase;
	struct ftgmac030_data *priv = dev->priv;

	unsigned short stat_fe;
	unsigned short stat_ge = 0 ;
	unsigned int maccr;

	if (FTGMAC030_MODE_RGMII == priv->phy_intf) {
		/* 1000 Base-T Status Register */
		stat_ge = ftgmac030_mdio_read(dev, priv->phy_addr, MII_STAT1000);
	}

	stat_fe = ftgmac030_mdio_read(dev, priv->phy_addr, MII_BMSR);

	if (!(stat_fe & BMSR_LSTATUS))	/* link status up? */
		return 0;

	/* read MAC control register and clear related bits */
	maccr = readl(&ftgmac030->maccr) &
		~(FTGMAC030_MACCR_SPEED_1000 |
		  FTGMAC030_MACCR_SPEED_100 |
		  FTGMAC030_MACCR_FULLDUP);

	if (FTGMAC030_MODE_RGMII == priv->phy_intf) {
		if (stat_ge & LPA_1000FULL) {
			/* set gmac for 1000BaseTX and Full Duplex */
			maccr |= FTGMAC030_MACCR_SPEED_1000 | FTGMAC030_MACCR_FULLDUP;
		}

		if (stat_ge & LPA_1000HALF) {
			/* set gmac for 1000BaseTX and Half Duplex */
			maccr |= FTGMAC030_MACCR_SPEED_1000;
		}
	}

	if (!(maccr & FTGMAC030_MACCR_SPEED_1000)) {
		if (stat_fe & BMSR_100FULL) {
			/* set MII for 100BaseTX and Full Duplex */
			maccr |= FTGMAC030_MACCR_SPEED_100 | FTGMAC030_MACCR_FULLDUP;
		}

		if (stat_fe & BMSR_10FULL) {
			/* set MII for 10BaseT and Full Duplex */
			maccr |= FTGMAC030_MACCR_FULLDUP;
		}

		if (stat_fe & BMSR_100HALF) {
			/* set MII for 100BaseTX and Half Duplex */
			maccr |= FTGMAC030_MACCR_SPEED_100;
		}

		if (stat_fe & BMSR_10HALF) {
			/* set MII for 10BaseT and Half Duplex */
			/* we have already clear these bits, do nothing */
			;
		}
	}

	/* update MII config into maccr */
	writel(maccr, &ftgmac030->maccr);

	return 1;
}

/*
 * Reset MAC
 */
static void ftgmac030_reset(struct eth_device *dev)
{
	struct ftgmac030 *ftgmac030 = (struct ftgmac030 *)dev->iobase;

	debug("%s()\n", __func__);

	writel(FTGMAC030_MACCR_SW_RST, &ftgmac030->maccr);

	while (readl(&ftgmac030->maccr) & FTGMAC030_MACCR_SW_RST)
		;
}

/*
 * Set MAC address
 */
static void ftgmac030_set_mac(struct eth_device *dev,
	const unsigned char *mac)
{
	struct ftgmac030 *ftgmac030 = (struct ftgmac030 *)dev->iobase;
	unsigned int maddr = mac[0] << 8 | mac[1];
	unsigned int laddr = mac[2] << 24 | mac[3] << 16 | mac[4] << 8 | mac[5];

	debug("%s(%x %x)\n", __func__, maddr, laddr);

	writel(maddr, &ftgmac030->mac_madr);
	writel(laddr, &ftgmac030->mac_ladr);
}

static void ftgmac030_set_mac_from_env(struct eth_device *dev)
{
	unsigned char mac[MAC_LEN];

	memset(mac, 0, sizeof(mac));

	if (!eth_env_get_enetaddr("ethaddr", mac)) {
		printf("MAC address invalid!\n");
#ifdef CONFIG_NET_RANDOM_ETHADDR
		net_random_ethaddr(mac);
		printf("Set Random MAC address!\n");
		eth_env_set_enetaddr("ethaddr", mac);
#endif
	}

	memcpy(dev->enetaddr, mac, MAC_LEN);

	ftgmac030_set_mac(dev, dev->enetaddr);
}

/*
 * disable transmitter, receiver
 */
static void ftgmac030_halt(struct eth_device *dev)
{
	struct ftgmac030 *ftgmac030 = (struct ftgmac030 *)dev->iobase;

	debug("%s()\n", __func__);

	writel(0, &ftgmac030->maccr);

	bspeth_sys_exit();
}

static int ftgmac030_init(struct eth_device *dev, bd_t *bd)
{
	struct ftgmac030 *ftgmac030 = (struct ftgmac030 *)dev->iobase;
	struct ftgmac030_data *priv = dev->priv;
	unsigned int maccr;
	ulong start, end;
	int i;

	debug("%s()\n", __func__);

	/* sys-func-sel */
	bspeth_sys_init();

	ftgmac030_reset(dev);

	/* set the MII interface */
	ftgmac030_set_phy_intf(dev);

	/* set the ethernet address */
	ftgmac030_set_mac_from_env(dev);

	/* disable all interrupts */
	writel(0, &ftgmac030->ier);

	/* initialize descriptors */
	priv->tx_index = 0;
	priv->rx_index = 0;

	for (i = 0; i < FTGMAC030_PKTBUFSTX; i++) {
		/* TXBUF_BADR */
		priv->txdes[i].txdes3 = 0;
		priv->txdes[i].txdes0 = 0;
	}
	priv->txdes[FTGMAC030_PKTBUFSTX - 1].txdes0	= FTGMAC030_TXDES0_EDOTR;

	start = ((ulong)&priv->txdes[0]) & ~(ARCH_DMA_MINALIGN - 1);
	end = start + roundup(sizeof(priv->txdes), ARCH_DMA_MINALIGN);
	flush_dcache_range(start, end);

	for (i = 0; i < FTGMAC030_PKTBUFSRX; i++) {
		/* RXBUF_BADR */
		priv->rxdes[i].rxdes3 = (unsigned int)net_rx_packets[i];
		priv->rxdes[i].rxdes0 = 0;
	}
	priv->rxdes[FTGMAC030_PKTBUFSRX - 1].rxdes0	= FTGMAC030_RXDES0_EDORR;

	start = ((ulong)&priv->rxdes[0]) & ~(ARCH_DMA_MINALIGN - 1);
	end = start + roundup(sizeof(priv->rxdes), ARCH_DMA_MINALIGN);
	flush_dcache_range(start, end);

	/* transmit ring */
	writel((unsigned int)priv->txdes, &ftgmac030->nptxdesc_addr);

	/* receive ring */
	writel((unsigned int)priv->rxdes, &ftgmac030->rxdesc_addr);

	/* poll receive descriptor automatically */
	writel(FTGMAC030_APTC_DEFAULT, &ftgmac030->aptc);

	/* config receive buffer size register */
	writel(FTGMAC030_RBSR_RXBUF_SIZE(FTGMAC030_RBSR_DEFAULT_VALUE), &ftgmac030->rbsr);

	/* enable transmitter, receiver */
	maccr = FTGMAC030_MACCR_DEFAULT;
	writel(maccr, &ftgmac030->maccr);

	if (ftgmac030_phy_init(dev)) {
		if (!ftgmac030_update_link_speed(dev))
			return -1;
	}

	return 0;
}

/*
 * Get a data block via Ethernet
 */
static int ftgmac030_recv(struct eth_device *dev)
{
	struct ftgmac030_data *priv = dev->priv;
	struct ftgmac030_rxdes *curr_des = &priv->rxdes[priv->rx_index];
	unsigned short rxlen;
	ulong des_start = ((ulong)curr_des) & ~(ARCH_DMA_MINALIGN - 1);
	ulong des_end = des_start +
		roundup(sizeof(*curr_des), ARCH_DMA_MINALIGN);
	ulong data_start = curr_des->rxdes3;
	ulong data_end;

	invalidate_dcache_range(des_start, des_end);

	if (!(curr_des->rxdes0 & FTGMAC030_RXDES0_RXPKT_RDY))
		return -EAGAIN;

	if (curr_des->rxdes0 & (FTGMAC030_RXDES0_RX_ERR |
				FTGMAC030_RXDES0_CRC_ERR |
				FTGMAC030_RXDES0_FTL |
				FTGMAC030_RXDES0_RUNT |
				FTGMAC030_RXDES0_RX_ODD_NB)) {
		return -EAGAIN;
	}

	rxlen = FTGMAC030_RXDES0_VDBC(curr_des->rxdes0);

	debug("%s(): RX buffer %d, %x received\n",
	       __func__, priv->rx_index, rxlen);

	/* Invalidate received data */
	data_end = data_start + roundup(rxlen, ARCH_DMA_MINALIGN);
	invalidate_dcache_range(data_start, data_end);

	/* pass the packet up to the protocol layers. */
	net_process_received_packet((void *)curr_des->rxdes3, rxlen);

	/* Release buffer to DMA and flush descriptor */
	curr_des->rxdes0 &= FTGMAC030_RXDES0_EDORR;
	curr_des->rxdes1 = 0x0;
	flush_dcache_range(des_start, des_end);

	/* Move to next descriptor */
	priv->rx_index = (priv->rx_index + 1) % FTGMAC030_PKTBUFSRX;

	return 0;
}

static u32 ftgmac030_read_txdesc(const void *desc)
{
	const struct ftgmac030_txdes *txdes = desc;
	ulong des_start = ((ulong)txdes) & ~(ARCH_DMA_MINALIGN - 1);
	ulong des_end = des_start + roundup(sizeof(*txdes), ARCH_DMA_MINALIGN);

	invalidate_dcache_range(des_start, des_end);

	return txdes->txdes0;
}

BUILD_WAIT_FOR_BIT(ftgmac030_txdone, u32, ftgmac030_read_txdesc)

/*
 * Send a data block via Ethernet
 */
static int ftgmac030_send(struct eth_device *dev, void *packet, int length)
{
	struct ftgmac030_data *priv = dev->priv;
	struct ftgmac030 *ftgmac030 = (struct ftgmac030 *)dev->iobase;
	struct ftgmac030_txdes *curr_des = &priv->txdes[priv->tx_index];
	ulong des_start = ((ulong)curr_des) & ~(ARCH_DMA_MINALIGN - 1);
	ulong des_end = des_start +
		roundup(sizeof(*curr_des), ARCH_DMA_MINALIGN);
	ulong data_start;
	ulong data_end;
	int rc;

	invalidate_dcache_range(des_start, des_end);

	if (curr_des->txdes0 & FTGMAC030_TXDES0_TXDMA_OWN) {
		debug("%s(): no TX descriptor available\n", __func__);
		return -1;
	}

	debug("%s(%x, %x)\n", __func__, (int)packet, length);

	length = (length < ETH_ZLEN) ? ETH_ZLEN : length;

	/* initiate a transmit sequence */
	curr_des->txdes3 = (unsigned int)packet;	/* TXBUF_BADR */

	/* Flush data to be sent */
	data_start = curr_des->txdes3;
	data_end = data_start + roundup(length, ARCH_DMA_MINALIGN);
	flush_dcache_range(data_start, data_end);

	/* only one descriptor on TXBUF */
	curr_des->txdes0 &= FTGMAC030_TXDES0_EDOTR;
	curr_des->txdes0 |= FTGMAC030_TXDES0_FTS |
			    FTGMAC030_TXDES0_LTS |
			    FTGMAC030_TXDES0_BUF_SIZE(length) |
			    FTGMAC030_TXDES0_TXDMA_OWN ;

	/* Flush modified buffer descriptor */
	flush_dcache_range(des_start, des_end);

	/* start transmit */
	writel(1, &ftgmac030->nptxpd);

	/* wait for transfer to succeed */
	rc = wait_for_bit_ftgmac030_txdone(curr_des,
					   FTGMAC030_TXDES0_TXDMA_OWN, false,
					   FTGMAC030_TX_TIMEOUT_MS, true);
	if (rc)
		return rc;

	debug("%s(): packet sent\n", __func__);

	/* Move to next descriptor */
	priv->tx_index = (priv->tx_index + 1) % FTGMAC030_PKTBUFSTX;

	return 0;
}

int ftgmac030_initialize(bd_t *bd)
{
	struct eth_device *dev;
	struct ftgmac030_data *priv;

	dev = malloc(sizeof *dev);
	if (!dev) {
		printf("%s(): failed to allocate dev\n", __func__);
		goto out;
	}

	/* Transmit and receive descriptors should align to 16 bytes */
	priv = memalign(ARCH_DMA_MINALIGN, sizeof(struct ftgmac030_data));
	if (!priv) {
		printf("%s(): failed to allocate priv\n", __func__);
		goto free_dev;
	}

	memset(dev, 0, sizeof(*dev));
	memset(priv, 0, sizeof(*priv));

	sprintf(dev->name, "FTGMAC030");
	dev->iobase	= GMAC_REG_BASE;
	dev->init	= ftgmac030_init;
	dev->halt	= ftgmac030_halt;
	dev->send	= ftgmac030_send;
	dev->recv	= ftgmac030_recv;
	dev->priv	= priv;

	ftgmac030_mdiobus_init(dev);

	eth_register(dev);

	return 1;

free_dev:
	free(dev);
out:
	return 0;
}
