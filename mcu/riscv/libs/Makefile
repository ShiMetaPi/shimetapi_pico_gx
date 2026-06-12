SRCS-y += lib.c stdio.c div64.c
SRCS-$(CONFIG_PRINTF) += printf.c
ifdef CONFIG_MALLOC
SRCS-y += malloc.c memlib.c
else
SRCS-y += simple_malloc.c
endif
SRCS-y += reserved_mem.c
