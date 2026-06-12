/*
 * Copyright (c) LOTUS. All rights reserved.
 */

#ifndef __FTGMAC030_H
#define __FTGMAC030_H

/* The registers offset table of ftgmac030 */
struct ftgmac030 {
	volatile unsigned int isr; /* 0x00 */
	volatile unsigned int ier; /* 0x04 */
	volatile unsigned int mac_madr; /* 0x08 */
	volatile unsigned int mac_ladr; /* 0x0C */
	volatile unsigned int maht0; /* 0x10 */
	volatile unsigned int maht1; /* 0x14 */
	volatile unsigned int nptxpd; /* 0x18 */
	volatile unsigned int rxpd; /*0x1C */
	volatile unsigned int nptxdesc_addr; /* 0x20 */
	volatile unsigned int rxdesc_addr; /* 0x24 */
	volatile unsigned int hptxpd; /* 0x28 */
	volatile unsigned int hptxdesc_addr; /*0x2C */
	volatile unsigned int txitc; /* 0x30 */
	volatile unsigned int rxitc; /* 0x34 */
	volatile unsigned int aptc; /* 0x38 */
	volatile unsigned int dblac; /* 0x3C */
	volatile unsigned int dmafifos; /* 0x40 */
	volatile unsigned int rsvd1; /* 0x44 */
	volatile unsigned int tfafcr; /* 0x48 */
	volatile unsigned int rbsr; /* 0x4C */
	volatile unsigned int maccr; /* 0x50 */
	volatile unsigned int macsr; /* 0x54 */
	volatile unsigned int tm; /* 0x58 */
	volatile unsigned int rsvd2; /* 0x5C */
	volatile unsigned int phycr; /* 0x60 */
	volatile unsigned int phydata;/* 0x64 */
	volatile unsigned int flow_cntrl; /* 0x68 */
	volatile unsigned int back_pressure; /* 0x6C */
	volatile unsigned int wakeOnLan_cntrl; /* 0x70 */
	volatile unsigned int wakeOnLan_sts; /* 0x74 */
	volatile unsigned int wfc_crc; /* 0x78 */
	volatile unsigned int rsvd3; /* 0x7C */
	volatile unsigned int wfbm1; /* 0x80 */
	volatile unsigned int wfbm2; /* 0x84 */
	volatile unsigned int wfbm3; /* 0x88 */
	volatile unsigned int wfbm4; /* 0x8C */
	volatile unsigned int nptxr_ptr; /* 0x90 */
	volatile unsigned int hptxr_prt; /* 0x94 */
	volatile unsigned int rxr_ptr; /* 0x98 */
	volatile unsigned int rsvd4; /* 0x9C */
	volatile unsigned int tpkt_cnt; /* 0xA0 */
	volatile unsigned int tx_m_s_col; /* 0xA4 */
	volatile unsigned int tx_e_f_cnt; /* 0xA8 */
	volatile unsigned int tx_l_u_cnt; /* 0xAC */
	volatile unsigned int rpkt_cnt; /* 0xB0 */
	volatile unsigned int bropkt_cnt; /* 0xB4 */
	volatile unsigned int mulpkt_cnt; /* 0xB8 */
	volatile unsigned int rpf_aep_cnt; /* 0xBC */
	volatile unsigned int runt; /* 0xC0 */
	volatile unsigned int crcer_ftl_cnt; /* 0xC4 */
	volatile unsigned int rcol_rlost_cnt; /* 0xC8 */
	volatile unsigned int bistr; /* 0xCC */
	volatile unsigned int bmrcr; /* 0xD0 */
	volatile unsigned int rsvd5[4]; /* 0xD4 ~ 0xE3 */
	volatile unsigned int acir; /* 0xE4 AXI additional Control Information */
	volatile unsigned int gisr; /* 0xE8 GMAC Interface Selection */
	volatile unsigned int rsvd6;/* 0xEC */
	volatile unsigned int eee; /* 0xF0 */
	volatile unsigned int revr; /* 0xF4 */
	volatile unsigned int fear; /* 0xF8 */
	volatile unsigned int rsvd7[1]; /* 0xFC */
	volatile unsigned int ptp_rx_addr; /* 0x100 PTP Destination Unicast IP Address on Rx path */
	volatile unsigned int ptp_tx_addr; /* 0x104  PTP Destination Unicast IP Address on Tx path */
	volatile unsigned int rsvd8[2]; /* 0x108 ~  0x10C*/
	volatile unsigned int ptp_tx_sec; /* 0x110 PTP TX Event Frame Time-stamp for seconds */
	volatile unsigned int ptp_tx_nsec; /* 0x114 PTP TX Event Frame Time-stamp for nanoseconds */
	volatile unsigned int ptp_rx_sec; /* 0x118 PTP RX Event Frame Time-stamp for seconds */
	volatile unsigned int ptp_rx_nsec; /* 0x11C PTP RX Event Frame Time-stamp for nanoseconds */
	volatile unsigned int ptp_tx_p_sec; /* 0x120 PTP TX Peer Frame Time-stamp for seconds */
	volatile unsigned int ptp_tx_p_nsec; /* 0x124 PTP TX Peer Frame Time-stamp for nanoseconds */
	volatile unsigned int ptp_rx_p_sec; /* 0x128 PTP RX Peer Frame Time-stamp for seconds */
	volatile unsigned int ptp_rx_p_nsec; /* 0x12C PTP RX Peer Frame Time-stamp for nanoseconds */
	volatile unsigned int ptp_nnsec_tmr; /* 0x130 PTP Timer Nano-nansecond */
	volatile unsigned int ptp_nsec_tmr; /* 0x134 PTP Timer nanosecond */
	volatile unsigned int ptp_sec_tmr; /* 0x138 PTP Second */
	volatile unsigned int ptp_ns_period; /* 0x13C PTP period increment, max is 255 ns */
	volatile unsigned int ptp_nns_period; /* 0x140 PTP period increment, max is 0.999999999 ns */
	volatile unsigned int ptp_offset; /* 0x144 PTP period offset increment */
	volatile unsigned int ptp_tmr_adj; /* 0x148 PTP Timer Adjustment */
};

/*
 * Interrupt Status Register 0x00 and 0x04
 */
#define FTGMAC030_INT_PDELAY_RESP_OUT (1 << 24)
#define FTGMAC030_INT_PDELAY_RESP_IN  (1 << 23)
#define FTGMAC030_INT_PDELAY_REQ_OUT  (1 << 22)
#define FTGMAC030_INT_PDELAY_REQ_IN   (1 << 21)
#define FTGMAC030_INT_DELAY_REQ_OUT   (1 << 20)
#define FTGMAC030_INT_DELAY_REQ_IN    (1 << 19)
#define FTGMAC030_INT_SYNC_OUT        (1 << 18)
#define FTGMAC030_INT_SYNC_IN         (1 << 17)
#define FTGMAC030_INT_TSU_SEC_INC     (1 << 16)
#define FTGMAC030_INT_RX_LPI_IN       (1 << 12)
#define FTGMAC030_INT_RX_LPI_EXIT     (1 << 11)
#define FTGMAC030_INT_HPTXBUF_UNAVA   (1 << 10)
#define FTGMAC030_INT_PHYSTS_CHG      (1 << 9)
#define FTGMAC030_INT_AHB_ERR         (1 << 8)
#define FTGMAC030_INT_TPKT_LOST       (1 << 7)
#define FTGMAC030_INT_NPTXBUF_UNAVA   (1 << 6)
#define FTGMAC030_INT_TPKT2F          (1 << 5)
#define FTGMAC030_INT_TPKT2E          (1 << 4)
#define FTGMAC030_INT_RPKT_LOST       (1 << 3)
#define FTGMAC030_INT_RXBUF_UNAVA     (1 << 2)
#define FTGMAC030_INT_RPKT2F          (1 << 1)
#define FTGMAC030_INT_RPKT2B          (1 << 0)

#define FTGMAC030_INT_DEFAULT ( \
				FTGMAC030_INT_RPKT2B | \
				FTGMAC030_INT_RXBUF_UNAVA | \
				FTGMAC030_INT_RPKT_LOST | \
				FTGMAC030_INT_TPKT2E | \
				FTGMAC030_INT_TPKT2F | \
				FTGMAC030_INT_TPKT_LOST | \
				FTGMAC030_INT_AHB_ERR | \
				FTGMAC030_INT_PHYSTS_CHG \
				)

#define FTGMAC030_INT_ALL ( \
				FTGMAC030_INT_RPKT2B | \
				FTGMAC030_INT_RPKT2F | \
				FTGMAC030_INT_RXBUF_UNAVA | \
				FTGMAC030_INT_RPKT_LOST | \
				FTGMAC030_INT_TPKT2E | \
				FTGMAC030_INT_TPKT2F | \
				FTGMAC030_INT_NPTXBUF_UNAVA | \
				FTGMAC030_INT_TPKT_LOST | \
				FTGMAC030_INT_AHB_ERR | \
				FTGMAC030_INT_PHYSTS_CHG | \
				FTGMAC030_INT_HPTXBUF_UNAVA \
				)
/*
 * MAC Most Significant Address Register 0x08
 */
#define FTGMAC030_MADR(x) ((x & 0xffff) << 0)

/*
 * MAC Least Siginificant Address Register 0x0C
 */
#define FTGMAC030_LADR(x) ((x & 0xffffffff) << 0)

/*
 * TX Interrupt Timer Control Register 0x30
 */
#define FTGMAC030_TXITC_TIME_SEL     (1 << 16)
#define FTGMAC030_TXITC_CYL(x)       ((x & 0xff) << 8)
#define FTGMAC030_TXITC_THR(x)       ((x & 0x7) << 4)
#define FTGMAC030_TXITC_THR_UNIT(x)  ((x & 0x3) << 0)

#define FTGMAC030_TXITC_DEFAULT       FTGMAC030_TXITC_THR(1)

/*
 * RX Interrupt Timer Control Register 0x34
 */
#define FTGMAC030_RXITC_RST(x)       ((x & 0xff) << 16)
#define FTGMAC030_RXITC_TIME_SEL     (1 << 16)
#define FTGMAC030_RXITC_CYL(x)       ((x & 0xff) << 8)
#define FTGMAC030_RXITC_THR(x)       ((x & 0x7) << 4)
#define FTGMAC030_RXITC_THR_UNIT(x)  ((x & 0x3) << 0)

#define FTGMAC030_RXITC_DEFAULT       FTGMAC030_RXITC_THR(1)

/*
 * Automatic Polling Timer Control Register 0x34
 */
#define FTGMAC030_APTC_TX_TIME   (1 << 12)
#define FTGMAC030_APTC_TX_CNT(x) ((x & 0xf) << 8)
#define FTGMAC030_APTC_RX_TIME   (1 << 4)
#define FTGMAC030_APTC_RX_CNT(x) ((x & 0xf) << 0)

#define FTGMAC030_APTC_DEFAULT FTGMAC030_APTC_RX_CNT(1)

/*
 * DMA Burst Length and Arbitration Control Register 0x38
 */
#define FTGMAC030_DBLAC_IFG_INC        (1 << 23)
#define FTGMAC030_DBLAC_IFG_CNT(x)     ((x & 0x7) << 20)
#define FTGMAC030_DBLAC_TXDES_SIZE(x)  ((x & 0xf) << 16)
#define FTGMAC030_DBLAC_RXDES_SIZE(x)  ((x & 0xf) << 12)
#define FTGMAC030_DBLAC_TXBST_SIZE(x)  ((x & 0x3) << 10)
#define FTGMAC030_DBLAC_RXBST_SIZE(x)  ((x & 0x3) << 8)
#define FTGMAC030_DBLAC_RX_THR_EN      (1 << 6)
#define FTGMAC030_DBLAC_RXFIFO_HTHR(x) ((x & 0x7) << 3)
#define FTGMAC030_DBLAC_RXFIFO_LTHR(x) ((x & 0x7) << 0)

#define FTGMAC030_DBLAC_DEFAULT FTGMAC030_DBLAC_RXFIFO_LTHR(2) | \
				FTGMAC030_DBLAC_RXFIFO_HTHR(6) | \
				FTGMAC030_DBLAC_RX_THR_EN | \
				FTGMAC030_DBLAC_RXBST_SIZE(3) | \
				FTGMAC030_DBLAC_TXBST_SIZE(3)
/*
 * DMA/FIFO State Register 0x3c
 */
#define FTGMAC030_DMAFIFOS_TXD_REQ       (1 << 31)
#define FTGMAC030_DMAFIFOS_RXD_REQ       (1 << 30)
#define FTGMAC030_DMAFIFOS_DARB_TXGNT    (1 << 29)
#define FTGMAC030_DMAFIFOS_DARB_RXGNT    (1 << 28)
#define FTGMAC030_DMAFIFOS_TXFIFO_EMPTY  (1 << 27)
#define FTGMAC030_DMAFIFOS_RXFIFO_EMPTY  (1 << 26)
#define FTGMAC030_DMAFIFOS_TXDMA3_SM(x)  ((x & 0xf) << 18)
#define FTGMAC030_DMAFIFOS_TXDMA2_SM(x)  ((x & 0xf) << 16)
#define FTGMAC030_DMAFIFOS_TXDMA1_SM(x)  ((x & 0xf) << 12)
#define FTGMAC030_DMAFIFOS_RXDMA3_SM(x)  ((x & 0xf) << 8)
#define FTGMAC030_DMAFIFOS_RXDMA2_SM(x)  ((x & 0xf) << 4)
#define FTGMAC030_DMAFIFOS_RXDMA1_SM(x)  ((x & 0xf) << 0)

/*
 * Revision Register 0x40
 */
#define FTGMAC030_REV_B1(x) ((x >> 16) & 0xff)
#define FTGMAC030_REV_B2(x) ((x >> 8) & 0xff)
#define FTGMAC030_REV_B3(x) (x & 0xff)

/*
 * Feature Register 0xF8
 */
#define FTGMAC030_FEAR_TFIFO_RSIZE(x) ((x & 0x7) >> 4)
#define FTGMAC030_FEAR_RFIFO_RSIZE(x) ((x & 0x7) >> 0)

/*
 * Transmit Priority Arbitration and FIFO Control Register 0x48
 */
#define FTGMAC030_TPAFCR_TFIFO_SIZE(x)  ((x & 0x7) << 27)
#define FTGMAC030_TPAFCR_RFIFO_SIZE(x)  ((x & 0x7) << 24)
#define FTGMAC030_TPAFCR_EARLY_TXTHR(x) ((x & 0xff) << 16)
#define FTGMAC030_TPAFCR_EARLY_RXTHR(x) ((x & 0xff) << 8)
#define FTGMAC030_TPAFCR_HPKT_THR(x)    ((x & 0xf) << 4)
#define FTGMAC030_TPAFCR_NPKT_THR(x)    ((x & 0xf) << 0)

/*
 * Receive Buffer Size Register 0x4c
 */
#define FTGMAC030_RBSR_RXBUF_SIZE(x) ((x & 0x3fff) << 0)

/*
 * MAC Control Register 0x50
 */
#define FTGMAC030_MACCR_SW_RST         (1 << 31)
#define FTGMAC030_MACCR_FULLDUP        (1 << 26)
#define FTGMAC030_MACCR_SPEED_MASK     (0x3 << 24)
#define FTGMAC030_MACCR_SPEED_1000     (2 << 24)
#define FTGMAC030_MACCR_SPEED_100      (1 << 24)
#define FTGMAC030_MACCR_SPEED_10       (0 << 24)
#define FTGMAC030_MACCR_HPTXR          (1 << 22)
#define FTGMAC030_MACCR_LOOPBACK       (1 << 21)
#define FTGMAC030_MACCR_PTP_EN         (1 << 20)
#define FTGMAC030_MACCR_REMOVE_VLAN    (1 << 18)
#define FTGMAC030_MACCR_CRC_APD        (1 << 17)
#define FTGMAC030_MACCR_DROP_CRC_ERR   (1 << 16)
#define FTGMAC030_MACCR_ENRX_IN_HALFTX (1 << 14)
#define FTGMAC030_MACCR_JUMBO_LF       (1 << 13)
#define FTGMAC030_MACCR_RX_RUNT        (1 << 12)
#define FTGMAC030_MACCR_BROADPKT       (1 << 11)
#define FTGMAC030_MACCR_MULTIPKT       (1 << 10)
#define FTGMAC030_MACCR_HT_EN          (1 << 9)
#define FTGMAC030_MACCR_ALLADDR        (1 << 8)
#define FTGMAC030_MACCR_RXMAC          (1 << 3)
#define FTGMAC030_MACCR_TXMAC          (1 << 2)
#define FTGMAC030_MACCR_RXDMA          (1 << 1)
#define FTGMAC030_MACCR_TXDMA          (1 << 0)

#define FTGMAC030_MACCR_DEFAULT ( \
					FTGMAC030_MACCR_RX_RUNT | \
					FTGMAC030_MACCR_CRC_APD | \
					FTGMAC030_MACCR_FULLDUP | \
					FTGMAC030_MACCR_TXDMA | \
					FTGMAC030_MACCR_RXMAC | \
					FTGMAC030_MACCR_RXDMA | \
					FTGMAC030_MACCR_TXMAC)

/*
 * MAC Status Register 0x54
 */
#define FTGMAC030_MACSR_COL_EXCEED
#define FTGMAC030_MACSR_LATE_COL
#define FTGMAC030_MACSR_XPKT_LOST
#define FTGMAC030_MACSR_XPKT_OK
#define FTGMAC030_MACSR_RUNT
#define FTGMAC030_MACSR_FTL
#define FTGMAC030_MACSR_CRC_ERR
#define FTGMAC030_MACSR_RPKT_LOST
#define FTGMAC030_MACSR_RPKT_SAVE
#define FTGMAC030_MACSR_COL
#define FTGMAC030_MACSR_BROADCAST
#define FTGMAC030_MACSR_MULTICAST

/*
 * Test Mode Register 0x58
 */
#define FTGMAC030_TM_PTIMER_TEST    (1 << 20)
#define FTGMAC030_TM_ITIMER_TEST    (1 << 19)
#define FTGMAC030_TM_TEST_COL       (1 << 15)
#define FTGMAC030_TM_TEST_BKOFF(x)  ((x & 0x3ff) << 5)
#define FTGMAC030_TM_TEST_EXSTHR(x) ((x & 0x1f) << 0)

/*
 * PHY Control Register 0x60
 */
#define FTGMAC030_MDIO_SOF            1
#define FTGMAC030_MDIO_EXT_SOF        0
#define FTGMAC030_MDIO_OP_RD          2
#define FTGMAC030_MDIO_OP_WR          1

#define FTGMAC030_PHYCR_PHYWR         (1 << 27)
#define FTGMAC030_PHYCR_PHYRD         (1 << 26)
#define FTGMAC030_PHYCR_REGAD(x)      ((x & 0x1f) << 21)
#define FTGMAC030_PHYCR_PHYAD(x)      ((x & 0x1f) << 16)
#define FTGMAC030_PHYCR_OP(x)         ((x & 0x3) << 14)
#define FTGMAC030_PHYCR_SOF(x)        ((x & 0x3) << 12)

#define FTGMAC030_PHYCR_MDC_CYCTHR(x)   ((x) & 0xff)
#define FTGMAC030_PHYCR_MDC_CYCTHR_MASK 0xff

/*
 * PHY Data Register 0x64
 */
#define FTGMAC030_PHYDATA_MIIRDATA(x) ((x & 0xffff0000) >> 16)
#define FTGMAC030_PHYDATA_MIIWDATA(x) ((x & 0xffff) >> 0)

/*
 * Flow Control Register 0x68
 */
#define FTGMAC030_FCR_PAUSE_TIME(x) ((x & 0xffff) << 16)
#define FTGMAC030_FCR_FC_H_L(x)     ((x & 0x7f) << 9)
#define FTGMAC030_FCR_HTHR          (1 << 8)
#define FTGMAC030_FCR_RX_PAUSE      (1 << 4)
#define FTGMAC030_FCR_TXPAUSED      (1 << 3)
#define FTGMAC030_FCR_FCTHR_EN      (1 << 2)
#define FTGMAC030_FCR_TX_PAUSE      (1 << 1)
#define FTGMAC030_FCR_FC_EN         (1 << 0)

/*
 * Back Pressure Register 0x6c
 */
#define ftgmac030_BPR_BK_LOW(x)    ((x & 0x7f) << 8)
#define ftgmac030_BPR_BKJAM_LEN(x) ((x & 0xf) << 4)
#define ftgmac030_BPR_BKADR_MODE   (1 << 1)
#define ftgmac030_BPR_BKEN         (1 << 0)

/*
 * Wake-On-LAN Control Register 0x70
 */
#define FTGMAC030_WOLCR_WOL_TYPE(x)   ((x & 0x3) << 24)
#define FTGMAC030_WOLCR_SW_PDNPHY     (1 << 18)
#define FTGMAC030_WOLCR_WAKEUP_SEL(x) ((x & 0x3) << 16)
#define FTGMAC030_WOLCR_WAKEUP4       (1 << 6)
#define FTGMAC030_WOLCR_WAKEUP3       (1 << 5)
#define FTGMAC030_WOLCR_WAKEUP2       (1 << 4)
#define FTGMAC030_WOLCR_WAKEUP1       (1 << 3)
#define FTGMAC030_WOLCR_MAGICPKT      (1 << 2)
#define FTGMAC030_WOLCR_LINKCHG1      (1 << 1)
#define FTGMAC030_WOLCR_LINKCHG0      (1 << 0)

/*
 * Wake-On-LAN Status Register 0x74
 */
#define FTGMAC030_WOLSR_WAKEUP4  (1 << 6)
#define FTGMAC030_WOLSR_WAKEUP3  (1 << 5)
#define FTGMAC030_WOLSR_WAKEUP2  (1 << 4)
#define FTGMAC030_WOLSR_WAKEUP1  (1 << 3)
#define FTGMAC030_WOLSR_MAGICPKT (1 << 2)
#define FTGMAC030_WOLSR_LINKCHG1 (1 << 1)
#define FTGMAC030_WOLSR_LINKCHG0 (1 << 0)

/*
 * Transmit descriptor, aligned to 16 bytes
 */
struct ftgmac030_txdes {
	volatile unsigned int txdes0;
	volatile unsigned int txdes1;
	volatile unsigned int txdes2; /* not used by HW */
	volatile unsigned int txdes3; /* TXBUF_BADR */
} __attribute__ ((aligned(16)));

#define FTGMAC030_TXDES0_TXDMA_OWN   (1 << 31)
#define FTGMAC030_TXDES0_FTS         (1 << 29)
#define FTGMAC030_TXDES0_LTS         (1 << 28)
#define FTGMAC030_TXDES0_CRC_ERR     (1 << 19)
#define FTGMAC030_TXDES0_EDOTR       (1 << 15)
#define FTGMAC030_TXDES0_BUF_SIZE(x) ((x & 0x3fff) << 0)

#define FTGMAC030_TXDES1_TXIC         (1 << 31)
#define FTGMAC030_TXDES1_TX2FIC       (1 << 30)
#define FTGMAC030_TXDES1_LLC_PKT      (1 << 22)
#define FTGMAC030_TXDES1_IPV6_PKT     (1 << 20)
#define FTGMAC030_TXDES1_OTHER_PKT    (2 << 20)
#define FTGMAC030_TXDES1_IPCS_EN      (1 << 19)
#define FTGMAC030_TXDES1_UDPCS_EN     (1 << 18)
#define FTGMAC030_TXDES1_TCPCS_EN     (1 << 17)
#define FTGMAC030_TXDES1_INS_VLAN     (1 << 16)
#define FTGMAC030_TXDES1_VLAN_TAGC(x) ((x & 0xffff) << 0)

#define FTGMAC030_TXDES1_DEFAULT (FTGMAC030_TXDES1_TXIC | FTGMAC030_TXDES1_TX2FIC)

/*
 * Receive descriptor, aligned to 16 bytes
 */
struct ftgmac030_rxdes {
	volatile unsigned int rxdes0;
	volatile unsigned int rxdes1;
	volatile unsigned int rxdes2; /* not used by HW */
	volatile unsigned int rxdes3; /* RXBUF_BADR */
} __attribute__ ((aligned(16)));

#define FTGMAC030_RXDES0_RXPKT_RDY    (1 << 31)
#define FTGMAC030_RXDES0_FRS          (1 << 29)
#define FTGMAC030_RXDES0_LRS          (1 << 28)
#define FTGMAC030_RXDES0_PAUSE_FRAME  (1 << 25)
#define FTGMAC030_RXDES0_PAUSE_OPCODE (1 << 24)
#define FTGMAC030_RXDES0_FIFO_FULL    (1 << 23)
#define FTGMAC030_RXDES0_RX_ODD_NB    (1 << 22)
#define FTGMAC030_RXDES0_RUNT         (1 << 21)
#define FTGMAC030_RXDES0_FTL          (1 << 20)
#define FTGMAC030_RXDES0_CRC_ERR      (1 << 19)
#define FTGMAC030_RXDES0_RX_ERR       (1 << 18)
#define FTGMAC030_RXDES0_BROADCAST    (1 << 17)
#define FTGMAC030_RXDES0_MULTICAST    (1 << 16)
#define FTGMAC030_RXDES0_EDORR        (1 << 15)
#define FTGMAC030_RXDES0_VDBC(x)      ((x & 0x3fff) << 0)

#define FTGMAC030_RXDES1_IPCS_FAIL     (1 << 27)
#define FTGMAC030_RXDES1_UDPCS_FAIL    (1 << 26)
#define FTGMAC030_RXDES1_TCPCS_FAIL    (1 << 25)
#define FTGMAC030_RXDES1_VLAN_AVA      (1 << 24)
#define FTGMAC030_RXDES1_DF            (1 << 23)
#define FTGMAC030_RXDES1_LLC_PKT       (1 << 22)
#define FTGMAC030_RXDES1_PROTL_TYPE(x) ((x >> 20) & 0x3)
#define FTGMAC030_RXDES1_IP6_TYPE(x)   ((x >> 19) & 0x1)
#define FTGMAC030_RXDES1_VLAN_PRIO(x)  ((x & 7) <<  13)
#define FTGMAC030_RXDES1_VLAN_CFI(x)   ((x & 1) <<  12)
#define FTGMAC030_RXDES1_VLAN_VID(x)   (x & 0x0fff)
#define FTGMAC030_RXDES1_VLAN_TAGC(x)  (x & 0xffff)

#define FTGMAC030_RXDES1_PROT_MASK	(0x3 << 20)
#define FTGMAC030_RXDES1_PROTL_NOTIP   0
#define FTGMAC030_RXDES1_PROTL_IP4     1
#define FTGMAC030_RXDES1_PROTL_TCPIP   2
#define FTGMAC030_RXDES1_PROTL_UDPIP   3

/* 0:MII or GMII,(don't suport) 1:RMII, 2: RGMII */
enum ftgmac030_if_mode {
	FTGMAC030_MODE_MII_GMII,
	FTGMAC030_MODE_RMII,
	FTGMAC030_MODE_RGMII,
};

int ftgmac030_mdio_read(struct eth_device *dev, int phy_addr, int regnum);
int ftgmac030_mdio_write(struct eth_device *dev, int phy_addr, int regnum, u16 value);

#endif /* __FTGMAC030_H */
