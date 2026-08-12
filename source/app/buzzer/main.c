/*
 * buzzer - 无源蜂鸣器示例：启动响 1s 后关闭。
 *
 * 硬件：GK7602V11A，无源蜂鸣器控制脚接 GPIO5_5（/dev/gpiochip5 line5）。
 * 行为：pad 复用 -> 输出 2kHz 方波 1s -> 置低关闭 -> 退出。
 *   无源蜂鸣器必须方波驱动；时长/频率在 main 里 buzzer_beep(...) 改。
 */
#include "buzzer.h"

#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

/*
 * pad 复用：GPIO5_5 = iocfg_reg60 @ 0x100C0040，默认 func0(GPIO5_5)，
 * 写 0x1000（func0 + bit12 输入使能）= 默认值，保险显式写一遍。
 */
#define PAGE_SIZE   0x1000u
#define BUZZER_PAD  0x100C0040u
#define GPIO_FUNC   0x1000u

static void set_pad_func(uint32_t phys, uint32_t func_val)
{
    int fd = open("/dev/mem", O_RDWR);
    if (fd < 0) {
        perror("[buzzer] open /dev/mem");
        return;
    }
    volatile uint8_t *m = (volatile uint8_t *)mmap(NULL, PAGE_SIZE,
            PROT_READ | PROT_WRITE, MAP_SHARED, fd, phys & ~(PAGE_SIZE - 1u));
    if (m == MAP_FAILED) {
        perror("[buzzer] mmap pad");
        close(fd);
        return;
    }
    volatile uint32_t *pad = (volatile uint32_t *)(m + (phys & (PAGE_SIZE - 1u)));
    *pad = func_val;
    printf("[buzzer] pad 0x%08X -> 0x%08X\n", phys, *pad);
    munmap((void *)m, PAGE_SIZE);
    close(fd);
}

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    setvbuf(stdout, NULL, _IONBF, 0);

    printf("[buzzer] pad 复用：GPIO5_5 -> func0(GPIO)\n");
    set_pad_func(BUZZER_PAD, GPIO_FUNC);

    if (buzzer_init() < 0) {
        fprintf(stderr, "[buzzer] GPIO5_5 申请输出失败，退出。\n");
        return 1;
    }

    printf("[buzzer] 响 1s（2kHz 方波）...\n");
    buzzer_beep(1000, 2000);
    printf("[buzzer] 已关闭，退出。\n");

    buzzer_deinit();
    return 0;
}
