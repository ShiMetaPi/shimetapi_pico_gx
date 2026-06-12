/*
 * Copyright (c) LOTUS. All rights reserved.
 */

#include <common.h>
#include <watchdog.h>
#include <command.h>
#include <serial.h>
#include <usb.h>
#include <crc16.h>
#include <linux/lotus/usb.h>

#define	XHEAD	0xAB
#define	XCMD	0xCD
#define	ACK	0xAA            /* ACK VALUE */
#define	NAK	0x55            /* NAK VALUE */

#define START_FRAME_LEN		5
#define MAX_BUFF_SIZE		1024
#define MAX_SEND_SIZE		20

static char recv_buf[MAX_BUFF_SIZE];

#undef DUMP_SWITCH
#ifdef DUMP_SWITCH
static void dump_buf(char *buf, int len)
{
	int i;
	char *p = buf;
	printf("%s->%d, buf=0x%.8x, len=%d\n",
			__FUNCTION__, __LINE__,
			(int)(buf), (int)(len));
	for (i = 0; i < (len); i++) {
		printf("0x%.2x ", *(p + i));
		if (!((i + 1) & 0x07))
			printf("\n");
	}
	printf("\n");
}
#endif

static unsigned short calc_crc16(const unsigned char *packet, unsigned long length)
{
	unsigned short crc16 = 0;
	unsigned long i;

	const unsigned short *crc16_table = get_crc16_table();
	if (crc16_table == NULL) {
		return 0;
	}

	for (i = 0; i < length; i++)
		crc16 = ((crc16 << 8) |     /* fetches High or low 8 bit */
			packet[i]) ^ crc16_table[(crc16 >> 8) & 0xFF];

	for (i = 0; i < 2; i++)  /* 2: Cycle and fetches High or low 8 bit */
		crc16 = ((crc16 << 8) | 0) ^ crc16_table[(crc16 >> 8) & 0xFF];

	return crc16;
}

static int recv_byte(void)
{
	if (serial_tstc())
		return serial_getc();

	return -1;
}

static char recv_data(void)
{
	int ret = -1;

	while (ret == -1)
		ret = recv_byte();

	return (char)ret;
}

void download_process(void)
{
	int i = 0, cr = 0, ret = -1;
	unsigned int head_frame_len = 0;
	unsigned int cmd_len = 0;
	unsigned char send_buf[MAX_SEND_SIZE] = {0};
	unsigned short cksum;

	do {
		do {
			do {
				cr = recv_byte();
			} while (cr != XHEAD);

			head_frame_len = START_FRAME_LEN;
			recv_buf[0] = (char)cr;

			/* RECV: head frame */
			for (i = 0; i < (head_frame_len - 1) && i < (sizeof(recv_buf) - 1); i++) {
				recv_buf[i + 1] = recv_data();
			}

			/* crc check */
			cksum = calc_crc16((unsigned char*)recv_buf,3);
			if (cksum == (((unsigned char)recv_buf[3] << 8) | (unsigned char)recv_buf[4])) {
				/* init */
				cmd_len = (unsigned short)(((unsigned char)recv_buf[1] << 8) | (unsigned char)recv_buf[2]) + 3;
				if (cmd_len >= sizeof(recv_buf))
					goto head_fail;

				/* SEND: ack */
				send_buf[0] = ACK;
				serial_putc(send_buf[0]);
				break;
			}

head_fail:
			/* init */
			cmd_len = 0;
			memset(recv_buf, 0, sizeof(recv_buf));

			/* SEND: nak */
			send_buf[0] = NAK;
			serial_putc(send_buf[0]);

		} while (1);

		do {
			do {
				cr = recv_byte();
			} while (cr != XCMD);

			recv_buf[0] = (char)cr;

			/* RECV: cmd data, cmd_len check in head flow */
			for (i = 0;i < (cmd_len - 1); i++) {
				recv_buf[i + 1] = recv_data();
			}

			/* crc check */
			cksum = calc_crc16((unsigned char*)recv_buf, cmd_len - 2);
			if (cksum == (((unsigned char)recv_buf[cmd_len - 2] << 8) | (unsigned char)recv_buf[cmd_len - 1])) {
				/* SEND: ack wait result */
				send_buf[0] = ACK;
				serial_putc(send_buf[0]);
				break;
			}

			memset(recv_buf, 0, sizeof(recv_buf));
			/* SEND: nak */
			send_buf[0] = NAK;
			serial_putc(send_buf[0]);
#ifdef DUMP_SWITCH
			dump_buf(recv_buf, cmd_len);
#endif
		} while (1);

		/* clean crc */
		recv_buf[cmd_len - 1] = 0;
		recv_buf[cmd_len - 2] = 0;

		/* cmd process */
		ret = run_command((recv_buf + 1), 0);
		if (ret != 0) {
			/* SEND: end flag */
			serial_puts("[EOT](ERROR)\n");
		} else{
			/* SEND: end flag */
			serial_puts("[EOT](OK)\n");
		}

	} while (1);
}

void set_update_mode_flag(int flag)
{
#ifdef REG_UPDATE_MODE_FLAG
	writel(flag, REG_UPDATE_MODE_FLAG);
#endif
}

int is_bootstrap(void)
{
	if (readl(REG_START_FLAG) == START_MAGIC)
		return 1;
	else
		return 0;
}

void set_bootstrap_flag(u32 flag)
{
	writel(flag, REG_START_FLAG);
	writel(flag, SYS_CTRL_REG_BASE + REG_SC_DDRT14);
}

void usb_eth_init(void)
{
	/* Support usb eth */
	run_command("usb start", 0);
}

int is_usb_device_bootstrap(void)
{
	if (readl(SYS_CTRL_REG_BASE + REG_SC_SYSBOOT9) == SELF_BOOT_TYPE_USBDEV)
		return 1;
	else
		return 0;
}

void download_boot(void)
{
	if (!is_bootstrap())
		return;

	/* Enable uart output for download */
	serial_enable_output(true);

	/* Clear flag */
	set_bootstrap_flag(0);

	if (!is_usb_device_bootstrap())
		usb_eth_init();

	serial_puts("start download process.\n");

	/* Wait cmd from tool */
	for (;;) {
		if (is_usb_device_bootstrap()) {
			/* Clear updata mode flag */
			set_update_mode_flag(1);
#ifdef CONFIG_USB_GADGET
			udc_connect();
#else
			printf("[ERROR]: Downloading boot from usb NOT supported.\n");
#endif
		} else
			download_process();
	}

	serial_enable_output(false);
}
