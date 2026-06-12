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
#define TIMER1_REG_BASE			0x12000020
#define TIMER2_REG_BASE			0x12001000
#define TIMER3_REG_BASE			0x12001020
#define REG_TIMER_RELOAD		0x0
#define REG_TIMER_VALUE			0x4
#define REG_TIMER_CONTROL		0x8

#define CRG_REG_BASE			0x12010000

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

#define REG_OTP_PO_BIT2			0x88

/* Generic register */
#define REG_SC_DDRT0			0x90
#define REG_SC_DDRT1			0x94
#define REG_SC_DDRT2			0x98
#define REG_SC_DDRT3			0x9c
#define REG_SC_DDRT4			0xa0
#define REG_SC_DDRT5			0xa4
#define REG_SC_DDRT6			0xa8
#define REG_SC_DDRT7			0xac
#define REG_SC_DDRT8			0xb0
#define REG_SC_DDRT9			0xb4
#define REG_SC_DDRT10			0xb8
#define REG_SC_DDRT11			0xbc
#define REG_SC_DDRT12			0xc0
#define REG_SC_DDRT13			0xc4
#define REG_SC_DDRT14			0xc8
#define REG_SC_DDRT15			0xcc
#define REG_SC_SYSBOOT0			0x130
#define REG_SC_SYSBOOT1			0x134
#define REG_SC_SYSBOOT2			0x138
#define REG_SC_SYSBOOT3			0x13c
#define REG_SC_SYSBOOT4			0x140
#define REG_SC_SYSBOOT5			0x144
#define REG_SC_SYSBOOT6			0x148
#define REG_SC_SYSBOOT7			0x14c
#define REG_SC_SYSBOOT8			0x150
#define REG_SC_SYSBOOT9			0x154
#define REG_SC_SYSBOOT10		0x158
#define REG_SC_SYSBOOT11		0x15c
#define REG_SC_SYSBOOT12		0x160
#define REG_SC_SYSBOOT13		0x164
#define REG_SC_SYSBOOT14		0x168
#define REG_SC_SYSBOOT15		0x16c

#define DDRCA_UPDATE			(SYS_CTRL_REG_BASE + 0x8034)
#define DDRCA_RANDOM0			(SYS_CTRL_REG_BASE + 0x8600)
#define DDRCA_RANDOM1			(SYS_CTRL_REG_BASE + 0x8604)
#define DDRCA_RANDOM2			(SYS_CTRL_REG_BASE + 0x8608)
#define DDRCA_RANDOM3			(SYS_CTRL_REG_BASE + 0x860c)

#define MISC_REG_BASE			0x12028000
#define DDRC0_REG_BASE			0x11330000
#define UART0_REG_BASE			0x12040000
#define FMC_MEM_BASE			0x14000000
#define DDR_MEM_BASE			0x40000000
#define TIMER0_BASE                     0x12000000

#define SERIAL_BASE		UART0_REG_BASE

#define UART_PL01x_DR		0x00
#define UART_PL01x_RSR		0x04
#define UART_PL01x_ECR		0x04
#define UART_PL01x_FR		0x18

#define UART_PL01x_FR_TXFE	0x80
#define UART_PL01x_FR_RXFF	0x40
#define UART_PL01x_FR_TXFF	0x20
#define UART_PL01x_FR_RXFE	0x10
#define UART_PL01x_FR_BUSY	0x08
#define UART_PL01x_FR_TMSK	(UART_PL01x_FR_TXFF + UART_PL01x_FR_BUSY)

#define START_MAGIC         0x444f574e

#define SUCCESS              0
#define FAILURE              -1

#define REG_OTP_PO_BIT0	     0x12020080

/* i2c0 reg */
#define REG_I2C_BASE                   (0x12060000)
/* clk reg control */
#define CRG_I2C                         (CRG_REG_BASE + 0x1B8)
#define I2C_CKEN                        BIT(16)
#define I2C_SRST                        BIT(24)

#endif
