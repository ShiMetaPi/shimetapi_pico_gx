
#include <stdio.h>
#include <string.h>
#include <config.h>
#include <lib.h>
#include <timer.h>
#include <serial.h>


#define CMD_BUFF_LEN 16
#define REV_BUFF_LEN 128

typedef enum {
    CMD_0 = 0,
    CMD_1,
    CMD_2,
    CMD_3,
    CMD_4,
    CMD_5,
    CMD_6,
    CMD_7,
    CMD_8,
    CMD_9,
    CMD_10,
    CMD_11,
    CMD_12,
    CMD_13,
    CMD_14,
    CMD_15,
    CMD_16
} cmd_len;

static void show_help(void)
{
    puts("usage:                                                                                 \n");
    puts("      help                           -- show command infomation                        \n");
    puts("      mw [addr] [value]              -- write [value] to [addr]                        \n");
    puts("                                        both need be hex and start with 0x or 0X       \n");
    puts("      md [addr] [length]             -- read from [addr] and print to the console      \n");
    puts("                                        total read [length] bytes data                 \n");
    puts("                                        both need be hex and start with 0x or 0X       \n");
}

static u32 read_line(u8 *buff, u32 buff_len)
{
    u32 recv_len = 0;
    char c = 0;

    while (1) {
        c = getchar();
        if (c == '\n' || c == '\r') {
            serial_putc('\n');
            break;
        }
        if (recv_len < buff_len) {
            serial_putc(c);
            buff[recv_len++] = c;
        } else {
            return FAILURE;
        }
    }

    if (recv_len < (buff_len - 1)) {
        buff[recv_len + 1] = '\0';
    }

    return recv_len;
}

/* return value : index is started from 0 */
static u32 str_get_pos(u8 *buff, u32 buff_len, char c)
{
    u32 index = 0;

    for (; index < buff_len; index++) {
        if (c == buff[index]) {
            return index;
        }
    }

    if (index == buff_len) {
        return index;
    }

    return FAILURE;
}

/* find the cmd dilived by the space */
static u32 get_cmd(u8 *cmd, u32 cmd_buff_len, u8 *buff, u32 buff_len)
{
    u32 cmd_len = 0;

    /* the cmd is divided by space */
    cmd_len = str_get_pos(buff, buff_len, ' ');
    if (cmd_len == 0 || cmd_len == FAILURE || cmd_len > cmd_buff_len) {
        return FAILURE;
    }

    for (u32 i = 0; i < cmd_len; i++) {
        cmd[i] = buff[i];
    }

    if (cmd_len < (cmd_buff_len - 1)) {
        cmd[cmd_len + 1] = '\0';
    }

    return cmd_len;
}

static u8 char_to_hex(u8 value)
{
    if((value <= '9') && (value >= '0')) {
        return (value - '0');
    } else if ((value <= 'f') && ((value >= 'a'))) {
        return (value - 'a' + 0x0a);
    } else if ((value <= 'F') && (value >= 'A')) {
        return (value - 'A' + 0x0a);
    } else {
        puts("Data format error!\n");
        return -1;
    }
}

static u32 pow_t(u32 v, u32 n)
{
    u32 value = v;

    if (n == 0) {
        return 1;
    } else if (n == 1) {
        return v;
    }

    for (u32 i = 1; i < n; i++) {
        value = value * v;
    }

    return value;
}

static u32 str_to_hex(u8 *str, u32 str_len)
{
    u32 hex = 0;

    for (int i = 0; i < str_len; i++) {
        hex += char_to_hex(str[i]) * pow_t(16, (str_len - i - 1));
    }

    return hex;
}

static u32 get_params(u8 * buff, u32 buff_Len, u32 *param_len)
{
    u8 param_s[10];

    *param_len = get_cmd(param_s, 10, buff, buff_Len);
    if (*param_len < 2 || *param_len == FAILURE) {
        puts("command error, too few params!\n");
        return FAILURE;
    }

    if (param_s[0] != '0' && (param_s[1] != 'x' || param_s[1] != 'X')) {
        puts("command error, params should start with 0x or 0X\n");
        return FAILURE;
    }

    return str_to_hex(param_s + 2, *param_len - 2);
}

/*
 * input: [addr] [value]
 */
void memory_write(u8 *buff, u32 buff_Len)
{
    u32 addr_len = 0;
    u32 value_len = 0;
    u32 addr = 0;
    u32 value = 0;

    /* get address */
    addr = get_params(buff, buff_Len, &addr_len);
    if (addr == FAILURE) {
        return;
    }

    /* get value */
    value = get_params(buff + addr_len + 1, buff_Len - addr_len - 1, &value_len);
    if (value == FAILURE) {
        return;
    }

    puts("write address=0x");
    serial_put_hex(addr);
    puts(" value=0x");
    serial_put_hex(value);
    puts("\n");

    writel(value, addr);
}

/*
 * input: [addr] [length]
 */
void memory_read(u8 *buff, u32 buff_Len)
{
    u32 addr_len = 0;
    u32 length_len = 0;
    u32 addr = 0;
    u32 length = 0;
    u32 value = 0;

    /* get address */
    addr = get_params(buff, buff_Len, &addr_len);
    if (addr == FAILURE) {
        return;
    }

    /* get length */
    length = get_params(buff + addr_len + 1, buff_Len - addr_len - 1, &length_len);
    if (length == FAILURE) {
        return;
    }

    if (length % 4 != 0) {
        puts("error: length should be align with 4\n");
        return;
    }

    /* read and print */
    for (u32 i = 0; i < length / 4; i++) {
        value = readl(addr + i * 4);
        if (i % 4 == 0) {
            serial_put_hex(addr + i * 4);
            puts(": ");
        }
        serial_put_hex(value);
        puts(" ");
        if (i % 4 == 3) {
            puts("\n");
        }
    }

    puts("\n");
}

u32 start_shell(void)
{
    u8 buff[REV_BUFF_LEN];
    u32 recv_len = 0;
    u8 cmd[CMD_BUFF_LEN];
    u32 cmd_len = 0;

    puts("\n## Micro shell ##\n");

    /* cmd [param1] [param2] ... */
    while (1) {
        puts("# ");
        memset_s(buff, REV_BUFF_LEN, 0, REV_BUFF_LEN);
        memset_s(cmd, CMD_BUFF_LEN, 0, CMD_BUFF_LEN);
        recv_len = read_line(buff, REV_BUFF_LEN);
        if (recv_len == 0 || recv_len == FAILURE) {
            continue;
        }
        cmd_len = get_cmd(cmd, CMD_BUFF_LEN, buff, recv_len);
        if (cmd_len == FAILURE) {
            continue;
        }
        switch (cmd_len) {
            case CMD_2:
                if (memcmp(cmd, "mw", CMD_2) == SUCCESS) {
                    /* mw [addr] [value]; addr and value should start with 0x/0X */
                    memory_write(buff + cmd_len + 1, recv_len - cmd_len - 1);
                } else if (memcmp(cmd, "md", CMD_2) == SUCCESS) {
                    /* md [addr] [length]; addr and length should start with 0x/0X */
                    memory_read(buff + cmd_len + 1, recv_len - cmd_len - 1);
                }
                break;
            case CMD_4:
                if (memcmp(cmd, "help", CMD_4) == SUCCESS) {
                    show_help();
                }
            default:
                puts("Unknown command!\n");
                break;
        }
    }

    return SUCCESS;
}
