
CFLAGS_start += -DTEXT_BASE=$(CONFIG_TEXT_BASE)
CFLAGS_start += -DSTACK_TOP=$(CONFIG_STACK_TOP)
CFLAGS += -DSTACK_TOP=$(CONFIG_STACK_TOP)
ASFLAGS += -DSTACK_TOP=$(CONFIG_STACK_TOP)

LINKLDS := firmware.lds
START = start.S
TAIL  = tail.S
SRCS-y  += vectors.S
SRCS-y  += chip.c system.c
SRCS-y  += isr.c trap_c.c drv_irq.c
