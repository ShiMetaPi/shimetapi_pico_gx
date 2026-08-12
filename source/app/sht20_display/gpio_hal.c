#include "gpio_hal.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

int gpio_handle_init(gpio_handle_t *gpio_handle)
{
	struct gpiohandle_request req;

	if (gpio_handle == NULL || gpio_handle->chip_path == NULL) {
		errno = EINVAL;
		return -1;
	}

	gpio_handle->chip_fd = -1;
	gpio_handle->line_fd = -1;

	gpio_handle->chip_fd = open(gpio_handle->chip_path, O_RDONLY);
	if (gpio_handle->chip_fd < 0) {
		perror("open gpiochip");
		return -1;
	}

	memset(&req, 0, sizeof(req));
	req.lineoffsets[0] = gpio_handle->line_offset;
	req.flags = gpio_handle->gpio_mode;
	req.default_values[0] = gpio_handle->default_value ? 1 : 0;
	req.lines = 1;

	if (gpio_handle->consumer_label[0] != '\0') {
		strncpy(req.consumer_label,
			gpio_handle->consumer_label,
			sizeof(req.consumer_label) - 1);
		req.consumer_label[sizeof(req.consumer_label) - 1] = '\0';
	} else {
		strncpy(req.consumer_label, "gpio-led", sizeof(req.consumer_label) - 1);
		req.consumer_label[sizeof(req.consumer_label) - 1] = '\0';
	}

	if (ioctl(gpio_handle->chip_fd, GPIO_GET_LINEHANDLE_IOCTL, &req) < 0) {
		perror("GPIO_GET_LINEHANDLE_IOCTL");
		close(gpio_handle->chip_fd);
		gpio_handle->chip_fd = -1;
		return -1;
	}

	gpio_handle->line_fd = req.fd;
	return 0;
}

int gpio_set_value(gpio_handle_t *gpio_handle, int value)
{
	struct gpiohandle_data data;

	if (gpio_handle == NULL || gpio_handle->line_fd < 0) {
		errno = EINVAL;
		return -1;
	}

	memset(&data, 0, sizeof(data));
	data.values[0] = value ? 1 : 0;

	if (ioctl(gpio_handle->line_fd, GPIOHANDLE_SET_LINE_VALUES_IOCTL, &data) < 0) {
		perror("GPIOHANDLE_SET_LINE_VALUES_IOCTL");
		return -1;
	}

	return 0;
}

int gpio_get_value(gpio_handle_t *gpio_handle)
{
	struct gpiohandle_data data;

	if (gpio_handle == NULL || gpio_handle->line_fd < 0) {
		errno = EINVAL;
		return -1;
	}

	memset(&data, 0, sizeof(data));

	if (ioctl(gpio_handle->line_fd, GPIOHANDLE_GET_LINE_VALUES_IOCTL, &data) < 0) {
		perror("GPIOHANDLE_GET_LINE_VALUES_IOCTL");
		return -1;
	}

	return data.values[0] ? 1 : 0;
}

void gpio_handle_close(gpio_handle_t *gpio_handle)
{
	if (gpio_handle == NULL)
		return;

	if (gpio_handle->line_fd >= 0) {
		close(gpio_handle->line_fd);
		gpio_handle->line_fd = -1;
	}

	if (gpio_handle->chip_fd >= 0) {
		close(gpio_handle->chip_fd);
		gpio_handle->chip_fd = -1;
	}
}