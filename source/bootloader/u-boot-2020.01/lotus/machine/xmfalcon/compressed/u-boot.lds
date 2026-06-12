
OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_start)
SECTIONS
{
	__image_copy_start =.;
	. = ALIGN(4);
	.text :
	{
	__text_start = .;
		start.o (.text*)
		uart.o (.text*)
		image_data.o (.text*)
		startup.o(.text*)
		reset.o(.text*)
		*(.text*)
	}
	__text_end = .;

	. = ALIGN(4);
	.image : { *(.image) }

	. = ALIGN(4);
	.rodata : { *(SORT_BY_ALIGNMENT(SORT_BY_NAME(.rodata*))) }

	. = ALIGN(4);
	.data : { *(.data) }

	. = ALIGN(4);
	.got : { *(.got) }

	. = ALIGN(4);
	.tail :
	{
		tail.o (.tail)
	}

	. = ALIGN(4);
	__image_copy_end =.;

	__bss_start = .;
	.bss : { *(.bss) }
	__bss_end = .;

	_end = .;
}
