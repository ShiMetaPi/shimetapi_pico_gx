OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)

#include <config.h>

ENTRY(_start)

SECTIONS
{
	. = CONFIG_TEXT_BASE;

	. = ALIGN(4);
	.text : {
		. = ALIGN(4);
		__text_start = .;
			start.o (.text)
			*(.text)
		__text_end = .;
	}

	. = ALIGN(4);
	.rodata : { *(SORT_BY_ALIGNMENT(SORT_BY_NAME(.rodata*))) }

	. = ALIGN(4);
	.data : {
		__data_start = .;
			*(.data)
		__data_end = .;
	}

        .tail : {
                tail.o(.tail)
        }

	. = ALIGN(4);
	.bss : {
		__bss_start = .;
		*(.bss)
		__bss_end = .;
		ASSERT(((__bss_end - __bss_start) == 0), "All global and static
			variables MUST be initialized with non-zero value!");
	}

	_end = .;
}
