
/* serial.c - print to uart */
#include <platform.h>
#include <serial.h>
#include <common.h>


static int serial_test_enable(void)
{
#ifdef CONFIG_SERIAL_DISABLE
	return 0;
#else
	return 1;
#endif
}

static void pl01x_putc(char c)
{
	/* Wait until there is space in the FIFO */
	while (readl(UART_PL01x_FR) & UART_PL01x_FR_TXFF);

	/* Send the character */
	writel(c, UART_PL01x_DR);
}

static u32 pl01x_getc(void)
{
    u32 data;

    while (readl(UART_PL01x_FR) & UART_PL01x_FR_RXFE) {
        /* Wait until there is data in the FIFO */
    }

    data = readl(UART_PL01x_DR);

    /* Check for an error flag */
    if (data & 0xFFFFFF00) {
	/* Clear the error */
	writel(0xFFFFFFFF, UART_PL01x_ECR);
	return 0;
    }

    return data;
}
void serial_putc(const char c)
{
	if (!serial_test_enable())
		return;

	if (c == '\n')
		pl01x_putc('\r');

	pl01x_putc(c);
}

char serial_getc(void)
{
    return (pl01x_getc() & 0xFF);
}

void serial_puts(const char *s)
{
	if (!serial_test_enable())
		return;

	if (s == NULL)
		return;
	while (*s) {
		serial_putc(*s);
		s++;
	}
}

void serial_puts_always(const char *s)
{
	if (s == NULL)
		return;

	while (*s) {
		if (*s == '\n')
			pl01x_putc('\r');

		pl01x_putc(*s);

		s++;
	}
}

void serial_put_dec(u32 dec)
{
	u32 num = dec;
	char c[20] = {0}; /* size 20 */
	int i = 0;

	if (!serial_test_enable())
		return;

	if (num == 0) {
		pl01x_putc('0');
		return;
	}

	while (num) {
		c[i] = '0' + (num % 10); /* bit size 10 */
		++i;
		num /= 10; /* bit size 10 */
	}

	for (i -= 1; i >= 0; --i)
		pl01x_putc(c[i]);
}

void serial_put_hex(u32 hex)
{
	int i;
	char c;

	if (!serial_test_enable())
		return;

	for (i = 28; i >= 0; i -= 4) { /* bit size 28 4 */
		c = ((unsigned int)hex >> (unsigned int)i) & 0x0F;
		if (c < 10) /* bit size 10 */
			c += '0';
		else
			c += 'A' - 10; /* bit size 10 */
		pl01x_putc(c);
	}
}

void serial_put_hex_always(u32 hex)
{
	int i;
	char c;

	for (i = 28; i >= 0; i -= 4) { /* bit size 28 4 */
		c = ((unsigned int)hex >> (unsigned int)i) & 0x0F;
		if (c < 10) /* bit size 10 */
			c += '0';
		else
			c += 'A' - 10; /* bit size 10 */
		pl01x_putc(c);
	}
}

static void _serial_crg_init(void)
{
	u32 tmp;

	/* configure pinmux of uart */
	/* writel(0x1502, 0x112C00F0);
	 * writel(0x1402, 0x119800E0); */

	/* enable uart clock */
	tmp = readl(REG_PERI_CRG110);
	tmp |= 1 << 3;
	writel(tmp, REG_PERI_CRG110);

	/* cancel uart reset */
	tmp = readl(REG_PERI_CRG110);
	tmp &= ~(1 << 11);
	writel(tmp, REG_PERI_CRG110);
}

void serial_init(void)
{
	u32 tmp;
	u32 divider;
	u32 remainder;
	u32 fraction;

	_serial_crg_init();

	/* first, disable everything */
	writel(0, UART_PL01x_CR);

	/*
	 * Set baud rate
	 *
	 * IBRD = UART_CLK / (16 * BAUD_RATE)
	 * FBRD = RND((64 * MOD(UART_CLK,(16 * BAUD_RATE)))
	 *               / (16 * BAUD_RATE))
	 */
	tmp = 16 * UART_BAUDRATE;
	divider = UART_CLOCK / tmp;
	remainder = UART_CLOCK % tmp;
	tmp = (8 * remainder) / UART_BAUDRATE;
	fraction = (tmp >> 1) + (tmp & 1);

	writel(divider, UART_PL01x_IBRD);
	writel(fraction, UART_PL01x_FBRD);

	writel(UART_PL011_LCRH_WLEN_8 | UART_PL011_LCRH_FEN, UART_PL01x_LCRH);
	/* Finally, enable the UART */
	writel(UART_PL011_CR_UARTEN | UART_PL011_CR_TXE |
			UART_PL011_CR_RXE, UART_PL01x_CR);

	return;
}
