#include "gpio_hal.h"

#include <linux/gpio.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

/*
 * 翻转 GPIO1_6（pin 59，物理 pad SFC_MISO_IO1）。
 * 数据全部来自 7206V11A PIN_OUT 表 “3.管脚控制寄存器” sheet（权威）：
 *
 *   iocfg_reg29  Pin SFC_MISO_IO1  addr 0x11980018  bits[3:0]:
 *                0x0=GPIO1_6（默认就是 GPIO）
 *                0x1=PWM9
 *                0x2=I2C2_SDA
 *                0x3=SDIO0_CARD_POWER_EN
 *                其它：保留。
 *
 * 功能值 = bit12(输入使能 0x1000) | func 号：
 *   GPIO1_6 -> 0x1000（默认即 GPIO，显式写一次以防被 boot 阶段复用踩过）
 *
 * 万用表量 pin 59 0V<->3.3V 跳，说明 pad mux 通路对、gpiochip line 对；
 * 量不到说明 iocfg 寄存器写没生效 / gpiochip 行号不对 / 物理断线。
 *
 * gpiochip 映射：GPIO1_x -> /dev/gpiochip1 line x（同 bank 顺序对应）。
 */
#define PAGE_SIZE 0x1000u

struct pin_def {
	const char *name;
	const char *chip_path;
	unsigned int line;
	uint32_t pad;
	uint32_t func;
};

static const struct pin_def g_pins[] = {
	{ "GPIO1_6(SFC_MISO_IO1)", "/dev/gpiochip1", 6, 0x11980018u, 0x1000u },
};
#define N_PINS ((int)(sizeof(g_pins) / sizeof(g_pins[0])))

/* 把单个 pad 切到目标功能（经 /dev/mem mmap 写 iocfg 寄存器）。 */
static void set_pad_func(uint32_t phys, uint32_t func_val)
{
	int fd = open("/dev/mem", O_RDWR);
	if (fd < 0) {
		perror("open /dev/mem");
		return;
	}
	volatile uint8_t *m = (volatile uint8_t *)mmap(NULL, PAGE_SIZE,
			PROT_READ | PROT_WRITE, MAP_SHARED, fd, phys & ~(PAGE_SIZE - 1u));
	if (m == MAP_FAILED) {
		perror("mmap pad");
		close(fd);
		return;
	}
	volatile uint32_t *pad = (volatile uint32_t *)(m + (phys & (PAGE_SIZE - 1u)));
	*pad = func_val;
	printf("[pad] 0x%08X -> 0x%08X\n", phys, *pad);
	munmap((void *)m, PAGE_SIZE);
	close(fd);
}

int main(void)
{
	gpio_handle_t h[N_PINS];
	int v = 0;
	int i;

	setvbuf(stdout, NULL, _IONBF, 0);   /* 串口控制台立即输出 */

	/* 1) pad 切到 GPIO 功能。 */
	for (i = 0; i < N_PINS; i++) {
		printf("[%s] pad 0x%08X func 0x%04X\n",
		       g_pins[i].name, g_pins[i].pad, g_pins[i].func);
		set_pad_func(g_pins[i].pad, g_pins[i].func);
	}

	/* 2) 申请 GPIO 输出。 */
	for (i = 0; i < N_PINS; i++) {
		memset(&h[i], 0, sizeof(h[i]));
		h[i].chip_path = g_pins[i].chip_path;
		h[i].line_offset = g_pins[i].line;
		h[i].gpio_mode = GPIOHANDLE_REQUEST_OUTPUT;
		h[i].default_value = 0;
		snprintf(h[i].consumer_label, sizeof(h[i].consumer_label),
		         "pin_%s", g_pins[i].name);
		if (gpio_handle_init(&h[i]) < 0) {
			fprintf(stderr, "[%s] gpio_handle_init 失败 (chip=%s line=%u): %s\n",
			        g_pins[i].name, g_pins[i].chip_path, g_pins[i].line,
			        strerror(errno));
			return 1;
		}
		printf("[%s] gpio ok (chip=%s line=%u)\n",
		       g_pins[i].name, g_pins[i].chip_path, g_pins[i].line);
	}

	printf("\nGPIO1_6 每 1s 翻转一次，万用表量 pin 59 应 0V<->3.3V 跳。Ctrl+C 退出\n");
	while (1) {
		for (i = 0; i < N_PINS; i++) {
			gpio_set_value(&h[i], v);
		}
		printf("GPIO1_6 = %d\n", v);
		v ^= 1;
		sleep(1);
	}

	for (i = 0; i < N_PINS; i++) {
		gpio_handle_close(&h[i]);
	}
	return 0;
}
