#include <linux/kernel.h>

char *param_get_command_line(void)
{
	return saved_command_line;
}
EXPORT_SYMBOL(param_get_command_line);
