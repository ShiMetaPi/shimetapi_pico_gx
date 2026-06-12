// SPDX-License-Identifier: GPL-2.0

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
	struct ftgmac030_txdes txdes[FTGMAC030_PKTBUFSTX] __aligned(ARCH_DMA_MINALIGN);
	struct ftgmac030_rxdes rxdes[FTGMAC030_PKTBUFSRX] __aligned(ARCH_DMA_MINALIGN);
	char rsvd1[0] __aligned(ARCH_DMA_MINALIGN);
	int tx_index;
	int rx_index;
	int phy_addr;
	enum ftgmac030_if_mode phy_intf;
	struct mii_dev *bus;
	struct phy_device *phydev;
	u32 max_speed;
	u8 mdc_cycthr;
};

/*
 * struct mdio functions
 */
int ftgmac030_mdio_read(struct eth_device *dev, int phy_addr,
	int regnum)
{
	struct ftgmac030 *ftgmac030 = (struct ftgmac030 *)dev->iobase;
	struct ftgmac030_data *priv = dev->priv;
	int phycr;
	int data;
	int ret;

	phycr = FTGMAC030_PHYCR_PHYAD(phy_addr)
	      |  FTGMAC030_PHYCR_REGAD(regnum)
	      |  FTGMAC030_PHYCR_OP(FTGMAC030_MDIO_OP_RD)
	      |  FTGMAC030_PHYCR_SOF(FTGMAC030_MDIO_SOF)
	      |  FTGMAC030_PHYCR_PHYRD
	      |  priv->mdc_cycthr;

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
	struct ftgmac030_data *priv = dev->priv;
	int phycr;
	int data;
	int ret;

	phycr = FTGMAC030_PHYCR_PHYAD(phy_addr)
	      |  FTGMAC030_PHYCR_REGAD(regnum)
	      |  FTGMAC030_PHYCR_OP(FTGMAC030_MDIO_OP_WR)
	      |  FTGMAC030_PHYCR_SOF(FTGMAC030_MDIO_SOF)
	      |  FTGMAC030_PHYCR_PHYWR
	      |  priv->mdc_cycthr;

	data = FTGMAC030_PHYDATA_MIIWDATA(value);

	writel(data, &ftgmac030->phydata);
	writel(phycr, &ftgmac030->phycr);

	ret = readl_poll_timeout(&ftgmac030->phycr, phycr,
				 !(phycr & FTGMAC030_PHYCR_PHYWR),
				 FTGMAC030_MDIO_TIMEOUT_USEC);
	if (ret)
		pr_err("mdio write failed (phy:%d reg:%x)\n", phy_addr, regnum);


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

static void ftgmac030_set_phy_intf(struct eth_device *dev)
{
	struct ftgmac030 *ftgmac030 = (struct ftgmac030 *)dev->iobase;
	struct ftgmac030_data *priv = dev->priv;

	priv->phy_intf = bspeth_get_phy_intf();

	printf("%s (%s)\n", __func__, (priv->phy_intf == FTGMAC030_MODE_MII_GMII) ?
		"mii" : ((priv->phy_intf == FTGMAC030_MODE_RMII) ? "rmii" : "rgmii"));

	writel(priv->phy_intf, &ftgmac030->gisr);

	if (priv->phy_intf == FTGMAC030_MODE_RGMII) {
		/* Set rgmii tx delay for 2ns */
		bspeth_set_tx_delay();
	}

	/* set the mdc cycthr sys_clk mpw:200M pliot:297*/
	priv->mdc_cycthr = 0xc8;

	/* update max_speed*/
	priv->max_speed = (priv->phy_intf == FTGMAC030_MODE_RGMII) ?
		SPEED_1000 : SPEED_100;

	/* update phy_intf to libphy */
	priv->phy_intf = (priv->phy_intf == FTGMAC030_MODE_MII_GMII) ?
		PHY_INTERFACE_MODE_MII : ((priv->phy_intf == FTGMAC030_MODE_RMII) ?
		PHY_INTERFACE_MODE_RMII :PHY_INTERFACE_MODE_RGMII_RXID);

}

int ftgmac030_mdiobus_init(struct eth_device *dev)
{
#if defined(CONFIG_MII) || defined(CONFIG_CMD_MII)
	struct ftgmac030_data *priv = dev->priv;
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

	priv->bus = bus;

#endif
	return 0;
}

static int ftgmac030_phy_init(struct eth_device *dev)
{
	int ret;
	struct ftgmac030_data *priv = dev->priv;
	struct phy_device *phydev;

	int phy_addr;
	u32 phy_id;

	/* Check if the PHY is up to snuff... */
	for (phy_addr = 0; phy_addr < 32; phy_addr++) {
		phy_id = ftgmac030_mdio_read(dev, phy_addr, MII_PHYSID1);
		/*
		 * When it is unable to found PHY,
		 * the interface usually return 0xffff or 0x0000
		 */
		if (phy_id != 0xffff && phy_id != 0x0) {

			priv->phy_addr = phy_addr;
			break;
		}
	}

	if (phy_id == 0xffff || phy_id == 0x0) {
		printf("%s: no PHY present\n", dev->name);
		return -ENODEV;
	}

	phy_id = ((phy_id & 0xffff) << 16) |
		ftgmac030_mdio_read(dev, phy_addr, MII_PHYSID2);

	printf("%s: found PHY(0x%x) at 0x%02x\n",
		dev->name, phy_id, phy_addr);

	phydev = phy_connect(priv->bus, priv->phy_addr, dev, priv->phy_intf);
	if (!phydev)
		return -ENODEV;

	printf("%s connected to %s\n", dev->name, phydev->drv->name);

	phydev->supported &= PHY_GBIT_FEATURES;

	if (priv->max_speed) {
		ret = phy_set_supported(phydev, priv->max_speed);
		if (ret)
			return ret;
	}

	phydev->advertising = phydev->supported;
	priv->phydev = phydev;
	phy_config(phydev);

	return 0;
}

static int ftgmac030_phy_link_speed(struct eth_device *dev)
{
	struct ftgmac030 *ftgmac030 = (struct ftgmac030 *)dev->iobase;
	struct ftgmac030_data *priv = dev->priv;
	struct phy_device *phydev = priv->phydev;
	unsigned int maccr;

	if (!phydev->link) {
		dev_err(phydev->dev, "No link\n");
		return -EREMOTEIO;
	}

	/* read MAC control register and clear related bits */
	maccr = readl(&ftgmac030->maccr) &
		~(FTGMAC030_MACCR_SPEED_1000 |
		FTGMAC030_MACCR_SPEED_100 |
		FTGMAC030_MACCR_FULLDUP);

	if (phy_interface_is_rgmii(phydev) && phydev->speed == 1000)
		maccr |= FTGMAC030_MACCR_SPEED_1000;

	if (phydev->speed == 100)
		maccr |= FTGMAC030_MACCR_SPEED_100;

	if (phydev->duplex)
		maccr |= FTGMAC030_MACCR_FULLDUP;

	/* update MII config into maccr */
	writel(maccr, &ftgmac030->maccr);

	return 0;
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
	struct phy_device *phydev = priv->phydev;
	unsigned int maccr;
	int ret;
	int i;

	debug("%s()\n", __func__);

	if (phydev == NULL) {
		/* sys-func-sel */
		bspeth_sys_init();

		/* set the MII interface */
		ftgmac030_set_phy_intf(dev);

		ftgmac030_mdiobus_init(dev);

		ret = ftgmac030_phy_init(dev);
		if (ret) {
			dev_err(dev, "Failed to initialize PHY\n");
			return ret;
		}

		phydev = priv->phydev;
	}

	ftgmac030_reset(dev);

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

	for (i = 0; i < FTGMAC030_PKTBUFSRX; i++) {
		/* RXBUF_BADR */
		priv->rxdes[i].rxdes3 = (unsigned int)net_rx_packets[i];
		priv->rxdes[i].rxdes0 = 0;
	}
	priv->rxdes[FTGMAC030_PKTBUFSRX - 1].rxdes0	= FTGMAC030_RXDES0_EDORR;

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


	ret = phy_startup(phydev);
	if (ret) {
		dev_err(phydev->dev, "Could not start PHY\n");
		return ret;
	}

	ret = ftgmac030_phy_link_speed(dev);
	if (ret) {
		dev_err(phydev->dev,  "Could not adjust link\n");
		return ret;
	}

	printf("%s: link up, %d Mbps %s-duplex\n", phydev->dev->name,
		phydev->speed, phydev->duplex ? "full" : "half");

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
	ulong data_start = curr_des->rxdes3;
	ulong data_end;

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

	debug("%s(): RX buffer %d, %x received(%x)\n",
	       __func__, priv->rx_index, rxlen, curr_des->rxdes3);

	/* Invalidate received data */
	data_end = data_start + roundup(rxlen, ARCH_DMA_MINALIGN);
	invalidate_dcache_range(data_start, data_end);

	/* pass the packet up to the protocol layers. */
	net_process_received_packet((void *)curr_des->rxdes3, rxlen);

	/* Release buffer to DMA*/
	curr_des->rxdes0 &= FTGMAC030_RXDES0_EDORR;
	curr_des->rxdes1 = 0x0;

	/* Move to next descriptor */
	priv->rx_index = (priv->rx_index + 1) % FTGMAC030_PKTBUFSRX;

	return 0;
}

static u32 ftgmac030_read_txdesc(const void *desc)
{
	const struct ftgmac030_txdes *txdes = desc;

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
	ulong data_start;
	ulong data_end;
	int rc;

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
			    FTGMAC030_TXDES0_TXDMA_OWN;

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

	dev = malloc(sizeof(*dev));
	if (!dev) {
		printf("%s(): failed to allocate dev\n", __func__);
		goto out;
	}

	/* Transmit and receive descriptors should align to 16 bytes */
	priv = (struct ftgmac030_data *)noncached_alloc(sizeof(struct ftgmac030_data),
			ARCH_DMA_MINALIGN);
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

	eth_register(dev);

	return 1;

free_dev:
	free(dev);
out:
	return 0;
}
