/*
 * Copyright (c) XMEDIA. All rights reserved.
 */
#ifndef __CHIP_REGS_H__
#define __CHIP_REGS_H__

#define BIT(nr)			(1 << (nr))

#define RAM_START_ADRS			0x04010500
#define STACK_TRAINING			0x04018000

#define DDR_DDRT_REG_BASE		0x11330000
#define TIMER0_REG_BASE			0x12000000
#define TIMER1_REG_BASE			0x12000100
#define TIMER2_REG_BASE			0x12000200
#define TIMER3_REG_BASE			0x12000300
#define REG_TIMER_RELOAD		0x0
#define REG_TIMER_VALUE			0x4
#define REG_TIMER_CONTROL		0x8

#define CRG_REG_BASE			0x12010000
#define REG_PERI_CRG110			(CRG_REG_BASE + 0x01B8)

#define SYS_CTRL_REG_BASE		0x12020000
#define REG_BASE_SCTL			SYS_CTRL_REG_BASE
#define REG_SC_CTRL			0
#define REMAPCLEAR			BIT(8)
#define REMAPCLEAR_SHIFT		8
#define TIME0_CLK_SEL			BIT(16)
#define TIME0_CLK_SEL_SHIFT		16
#define TIME0_CLK_SEL_3M		0x0
#define TIME0_CLK_SEL_APB		0x1

#define REG_SC_SYSRES			0x8
#define REG_SYSSTAT			0x008C

#define STOPWATCH_CTRL_REG2		(SYS_CTRL_REG_BASE + 0x8110)

/* Generic register */
#define REG_SC_DDRT0			(SYS_CTRL_REG_BASE + 0x90)
#define REG_SC_DDRT1			(SYS_CTRL_REG_BASE + 0x94)
#define REG_SC_DDRT2			(SYS_CTRL_REG_BASE + 0x98)
#define REG_SC_DDRT3			(SYS_CTRL_REG_BASE + 0x9c)
#define REG_SC_DDRT4			(SYS_CTRL_REG_BASE + 0xa0)
#define REG_SC_DDRT5			(SYS_CTRL_REG_BASE + 0xa4)
#define REG_SC_DDRT6			(SYS_CTRL_REG_BASE + 0xa8)
#define REG_SC_DDRT7			(SYS_CTRL_REG_BASE + 0xac)
#define REG_SC_DDRT8			(SYS_CTRL_REG_BASE + 0xb0)
#define REG_SC_DDRT9			(SYS_CTRL_REG_BASE + 0xb4)
#define REG_SC_DDRT10			(SYS_CTRL_REG_BASE + 0xb8)
#define REG_SC_DDRT11			(SYS_CTRL_REG_BASE + 0xbc)
#define REG_SC_DDRT12			(SYS_CTRL_REG_BASE + 0xc0)
#define REG_SC_DDRT13			(SYS_CTRL_REG_BASE + 0xc4)
#define REG_SC_DDRT14			(SYS_CTRL_REG_BASE + 0xc8)
#define REG_SC_DDRT15			(SYS_CTRL_REG_BASE + 0xcc)
#define REG_SC_SYSBOOT0			(SYS_CTRL_REG_BASE + 0x130)
#define REG_SC_SYSBOOT1			(SYS_CTRL_REG_BASE + 0x134)
#define REG_SC_SYSBOOT2			(SYS_CTRL_REG_BASE + 0x138)
#define REG_SC_SYSBOOT3			(SYS_CTRL_REG_BASE + 0x13c)
#define REG_SC_SYSBOOT4			(SYS_CTRL_REG_BASE + 0x140)
#define REG_SC_SYSBOOT5			(SYS_CTRL_REG_BASE + 0x144)
#define REG_SC_SYSBOOT6			(SYS_CTRL_REG_BASE + 0x148)
#define REG_SC_SYSBOOT7			(SYS_CTRL_REG_BASE + 0x14c)
#define REG_SC_SYSBOOT8			(SYS_CTRL_REG_BASE + 0x150)
#define REG_SC_SYSBOOT9			(SYS_CTRL_REG_BASE + 0x154)
#define REG_SC_SYSBOOT10		(SYS_CTRL_REG_BASE + 0x158)
#define REG_SC_SYSBOOT11		(SYS_CTRL_REG_BASE + 0x15c)
#define REG_SC_SYSBOOT12		(SYS_CTRL_REG_BASE + 0x160)
#define REG_SC_SYSBOOT13		(SYS_CTRL_REG_BASE + 0x164)
#define REG_SC_SYSBOOT14		(SYS_CTRL_REG_BASE + 0x168)
#define REG_SC_SYSBOOT15		(SYS_CTRL_REG_BASE + 0x16c)

#define DDRCA_UPDATE			(SYS_CTRL_REG_BASE + 0x8034)
#define DDRCA_RANDOM0			(SYS_CTRL_REG_BASE + 0x8600)
#define DDRCA_RANDOM1			(SYS_CTRL_REG_BASE + 0x8604)
#define DDRCA_RANDOM2			(SYS_CTRL_REG_BASE + 0x8608)
#define DDRCA_RANDOM3			(SYS_CTRL_REG_BASE + 0x860c)

#define MISC_REG_BASE			0x12028000
#define DDRC0_REG_BASE			0x11330000
#define UART0_REG_BASE			0x12040000
#define UART3_REG_BASE			0x12043000
#define FMC_MEM_BASE			0x14000000
#define DDR_MEM_BASE			0x40000000
#define TIMER0_BASE                     0x12000000
#define TIMER1_BASE                     0x12000100
#define TIMER2_BASE                     0x12001000

#define SERIAL_BASE		UART3_REG_BASE

#define UART_PL01x_DR		(SERIAL_BASE + 0x00)
#define UART_PL01x_RSR		(SERIAL_BASE + 0x04)
#define UART_PL01x_ECR		(SERIAL_BASE + 0x04)
#define UART_PL01x_FR		(SERIAL_BASE + 0x18)
#define UART_PL01x_IBRD		(SERIAL_BASE + 0x0024)
#define UART_PL01x_FBRD		(SERIAL_BASE + 0x0028)
#define UART_PL01x_LCRH		(SERIAL_BASE + 0x002C)
#define UART_PL01x_CR		(SERIAL_BASE + 0x0030)

#define UART_PL01x_FR_TXFE	0x80
#define UART_PL01x_FR_RXFF	0x40
#define UART_PL01x_FR_TXFF	0x20
#define UART_PL01x_FR_RXFE	0x10
#define UART_PL01x_FR_BUSY	0x08
#define UART_PL01x_FR_TMSK	(UART_PL01x_FR_TXFF + UART_PL01x_FR_BUSY)

/*
 *  PL011 definitions
 *
 */
#define UART_PL011_LCRH_SPS             (1 << 7)
#define UART_PL011_LCRH_WLEN_8          (3 << 5)
#define UART_PL011_LCRH_WLEN_7          (2 << 5)
#define UART_PL011_LCRH_WLEN_6          (1 << 5)
#define UART_PL011_LCRH_WLEN_5          (0 << 5)
#define UART_PL011_LCRH_FEN             (1 << 4)
#define UART_PL011_LCRH_STP2            (1 << 3)
#define UART_PL011_LCRH_EPS             (1 << 2)
#define UART_PL011_LCRH_PEN             (1 << 1)
#define UART_PL011_LCRH_BRK             (1 << 0)

#define UART_PL011_CR_CTSEN             (1 << 15)
#define UART_PL011_CR_RTSEN             (1 << 14)
#define UART_PL011_CR_OUT2              (1 << 13)
#define UART_PL011_CR_OUT1              (1 << 12)
#define UART_PL011_CR_RTS               (1 << 11)
#define UART_PL011_CR_DTR               (1 << 10)
#define UART_PL011_CR_RXE               (1 << 9)
#define UART_PL011_CR_TXE               (1 << 8)
#define UART_PL011_CR_LPE               (1 << 7)
#define UART_PL011_CR_IIRLP             (1 << 2)
#define UART_PL011_CR_SIREN             (1 << 1)
#define UART_PL011_CR_UARTEN            (1 << 0)

#define UART_PL011_IMSC_OEIM            (1 << 10)
#define UART_PL011_IMSC_BEIM            (1 << 9)
#define UART_PL011_IMSC_PEIM            (1 << 8)
#define UART_PL011_IMSC_FEIM            (1 << 7)
#define UART_PL011_IMSC_RTIM            (1 << 6)
#define UART_PL011_IMSC_TXIM            (1 << 5)
#define UART_PL011_IMSC_RXIM            (1 << 4)
#define UART_PL011_IMSC_DSRMIM          (1 << 3)
#define UART_PL011_IMSC_DCDMIM          (1 << 2)
#define UART_PL011_IMSC_CTSMIM          (1 << 1)
#define UART_PL011_IMSC_RIMIM           (1 << 0)

#define UART_BAUDRATE	115200
#define UART_CLOCK	24000000

#define START_MAGIC         0x444f574e

#define SUCCESS              0
#define FAILURE              -1

#define REG_OTP_PO_BIT0		0x12020080
#define REG_OTP_PO_BIT2		0x12020088

#if defined(CONFIG_PLATFORM_XMFALCON) || defined(CONFIG_PLATFORM_XMORCA)

#define PERI_CRG110 0x01b8
#define I2C_CRG_REG_BASE (CRG_REG_BASE + PERI_CRG110)

#define I2C0_REG_BASE 0x12060000
#define I2C1_REG_BASE 0x12061000
#define I2C2_REG_BASE 0x12062000
#define I2C3_REG_BASE 0x12063000
#define I2C4_REG_BASE 0x12064000
#define I2C5_REG_BASE 0x12065000
#define I2C6_REG_BASE 0x12066000
#define I2C7_REG_BASE 0x12067000
#define I2C_NUM 8

#elif defined(CONFIG_TARGET_XM720XXX)

#define PERI_CRG110 0x01b8
#define I2C_CRG_REG_BASE (CRG_REG_BASE + PERI_CRG110)

#define I2C0_REG_BASE 0x12060000
#define I2C1_REG_BASE 0x12061000
#define I2C2_REG_BASE 0x12062000
#define I2C_NUM 3

#endif
/*
 * I2C Registers offsets
 */
#define I2C_GLB 0x0
#define I2C_SCL_H 0x20
#define I2C_SCL_L 0x24
#define I2C_DATA1 0x28
#define I2C_TXF 0x10
#define I2C_RXF 0x14
#define I2C_CMD_BASE 0x40
#define I2C_LOOP1 0xc0
#define I2C_DST1 0xc4
#define I2C_TX_WATER 0x18
#define I2C_RX_WATER 0x1c
#define I2C_CTRL1 0x04
#define I2C_CTRL2 0x08
#define I2C_STAT 0x0c
#define I2C_INTR_RAW 0x30
#define I2C_INTR_EN 0x34
#define I2C_INTR_STAT 0x38

/*
 * I2C Global Config Register -- I2C_GLB
 *  */
#define GLB_EN_MASK BIT(0)
#define GLB_SDA_HOLD_MASK 0xffff0
#define GLB_SDA_HOLD_SHIFT (4)

/*
 * I2C Timing CMD Register -- I2C_CMD_BASE + n * 4 (n = 0, 1, 2, ... 31)
 *  */
#define CMD_EXIT 0x0
#define CMD_TX_S 0x1
#define CMD_TX_D1_2 0x4
#define CMD_TX_D1_1 0x5
#define CMD_TX_FIFO 0x9
#define CMD_RX_FIFO 0x12
#define CMD_RX_ACK 0x13
#define CMD_IGN_ACK 0x15
#define CMD_TX_ACK 0x16
#define CMD_TX_NACK 0x17
#define CMD_JMP1 0x18
#define CMD_UP_TXF 0x1d
#define CMD_TX_RS 0x1e
#define CMD_TX_P 0x1f

/*
 * I2C Control Register 1 -- I2C_CTRL1
 */
#define CTRL1_CMD_START_MASK BIT(0)

/*
 * I2C Status Register -- I2C_STAT
 */
#define STAT_RXF_NOE_MASK BIT(0) /* RX FIFO not empty flag */
#define STAT_TXF_NOF_MASK BIT(3) /* TX FIFO not full flag */

/*
 * I2C Interrupt status and mask Register --
 * I2C_INTR_RAW, I2C_STAT, I2C_INTR_STAT
 */
#define INTR_ABORT_MASK (BIT(0) | BIT(3))
#define INTR_RX_MASK BIT(5)
#define INTR_TX_MASK BIT(6)
#define INTR_CMD_DONE_MASK BIT(4)
#define INTR_USE_MASK (INTR_ABORT_MASK | INTR_RX_MASK | INTR_TX_MASK | INTR_CMD_DONE_MASK)
#define INTR_ALL_MASK 0xffffffff
#define I2C_TXF_DEPTH 64
#define I2C_RXF_DEPTH 64
#define I2C_TXF_WATER 32
#define I2C_RXF_WATER 32
/* for i2c rescue */
#define CHECK_SDA_IN_SHIFT (5)
#define GPIO_MODE_SHIFT (0)
#define FORCE_SCL_OEN_SHIFT (1)
#define FORCE_SDA_OEN_SHIFT (2)


#define GPIO_DATA 0x0
#define GPIO_DIR  0x400
#define GPIO0_BASE 0x120B0000


/*pwm reg*/
#define PWM_ADDR_BASE       0x12080000
#define PWM_CLOCK_ADDR_BASE 0x120101BC
#define PWM_CLOCK_3M 3000000
#define PWM_CLOCK_24M 24000000
#define PWM_CLOCK_50M 50000000

#define PWM_CTRL_ADDR(x) (((x)*0x100) + 0x0)
#define PWM_CFG_PLUSE_ADDR(x) (((x)*0x100) + 0x10)
#define PWM_CFG_HLINT_ADDR(x) (((x)*0x100) + 0x20)
#define PWM_CFG_CYCLE_ADDR(x) (((x)*0x100) + 0x30)

#define PWM_ENABLE_SHIFT 2
#define PWM_ENABLE_MASK  BIT(2)

#define PWM_POLARITY_SHIFT 1
#define PWM_POLARITY_MASK  BIT(1)

#define PWM_KEEP_SHIFT 0
#define PWM_KEEP_MASK  BIT(0)

#define PWM_PERIOD_MASK 0xffffffff
#define PWM_DUTY_MASK   0xffffffff
#define PWM_MAX_NUM   17

#define CONFIG_PARAM_MEM_START (CONFIG_TEXT_BASE + CONFIG_MCU_SRAM_SIZE - CONFIG_PARAM_MEM_SIZE)

#define CONFIG_DDR_BASE 0x40000000

/* -------------------------  Interrupt Number Definition  ------------------------ */
#define IRQ_NUM_M_SW		3      /* Machine software interrupt */
#define IRQ_NUM_CORE_TIMER	7      /* core Timer Interrupt */
#define IRQ_NUM_M_EXT		11     /* Machine external interrupt */
#define IRQ_NUM_RTC		16     /* rtc Interrupt */
#define IRQ_NUM_SOFTWARE	17     /* software Interrupt */
#define IRQ_NUM_WDG2		20     /* watchdog2 Interrupt */
#define IRQ_NUM_IR		21     /* ir Interrupt */
#define IRQ_NUM_TIMER3		26     /* timer3 Interrupt */
#define IRQ_NUM_I2C0		36     /* i2c0 Interrupt */
#define IRQ_NUM_I2C1		37     /* i2c1 Interrupt */
#define IRQ_NUM_I2C2		38     /* i2c2 Interrupt */
#define IRQ_NUM_I2C3		39     /* i2c3 Interrupt */
#define IRQ_NUM_I2C4		40     /* i2c4 Interrupt */
#define IRQ_NUM_I2C5		41     /* i2c5 Interrupt */
#define IRQ_NUM_I2C6		42     /* i2c6 Interrupt */
#define IRQ_NUM_I2C7		43     /* i2c7 Interrupt */
#define IRQ_NUM_TIMER4		44     /* timer4 Interrupt */
#define IRQ_NUM_TIMER5		45     /* timer5 Interrupt */
#define IRQ_NUM_SPI0		46     /* spi0 Interrupt */
#define IRQ_NUM_SPI1		47     /* spi1 Interrupt */
#define IRQ_NUM_SPI2		48     /* spi2 Interrupt */
#define IRQ_NUM_SPI3		49     /* spi3 Interrupt */
#define IRQ_NUM_MBOX		105    /* CPU2MCU Mailbox Interrupt */

#endif
