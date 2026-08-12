#ifndef GPIO_HAL_H
#define GPIO_HAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <linux/gpio.h>

#define GPIO_CONSUMER_LABEL_LEN 32

typedef struct gpio_handle_t {
	int chip_fd;
	const char *chip_path;
	unsigned int line_offset;
	unsigned int gpio_mode;
	int default_value;
	int line_fd;
	char consumer_label[GPIO_CONSUMER_LABEL_LEN];
} gpio_handle_t;

int gpio_handle_init(gpio_handle_t *gpio_handle);
int gpio_set_value(gpio_handle_t *gpio_handle, int value);
void gpio_handle_close(gpio_handle_t *gpio_handle);

#ifdef __cplusplus
}
#endif

#endif