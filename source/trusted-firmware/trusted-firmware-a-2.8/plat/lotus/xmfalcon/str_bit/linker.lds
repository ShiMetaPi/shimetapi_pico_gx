
OUTPUT_FORMAT("elf64-littleaarch64")
OUTPUT_ARCH(aarch64)

ENTRY(lotus_pm_ddr_saving)
SECTIONS
{
	. = 0x00000000;

	. = ALIGN(8);
	.text	:
	{
		str.o
		delay.o
		*(.text)
	}

	. = ALIGN(8);
	.rodata :
	{
		*(.rodata)
	}

	. = ALIGN(8);
	.data :
	{
	  *(.data)
	}

}
