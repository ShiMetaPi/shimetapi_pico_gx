OUTPUT_FORMAT("elf32-littleriscv", "elf32-littleriscv", "elf32-littleriscv")

#include <config.h>

MEMORY
{
	STAGE1_MEM : ORIGIN = CONFIG_TEXT_BASE, LENGTH = (CONFIG_STAGE2_START - CONFIG_TEXT_BASE)
	STAGE2_MEM : ORIGIN = CONFIG_STAGE2_START, LENGTH = (CONFIG_MCU_SRAM_SIZE - (CONFIG_STAGE2_START - CONFIG_TEXT_BASE) - CONFIG_PARAM_MEM_SIZE)
	PARAM_MEM  : ORIGIN = CONFIG_PARAM_MEM_START, LENGTH = CONFIG_PARAM_MEM_SIZE
}

ENTRY(_start)
SECTIONS
{
	. = ALIGN(4);
	.text : {
		. = ALIGN(4);
		__text_start = .;
			start.o (.text)
			libstage1.a(.text)
			libstage1.a(.text.*)
			libstage1.a(.text.stage1)
			libstage2.a(.text.stage1)
		__text_end = .;
	} >STAGE1_MEM

	. = ALIGN(4);
	.rodata : {
		__rodata_start = .;
			start.o(.rodata*)
			libstage1.a(.rodata)
			libstage1.a(.rodata.*)
			libstage1.a(.rodata.stage1)
			libstage2.a(.rodata.stage1)
			__global_pointer$ = .;
			libstage1.a(.srodata)
			libstage1.a(.srodata.*)
			libstage1.a(.srodata2.*)
		__rodata_end = .;
	} >STAGE1_MEM

	. = ALIGN(4);
	.data : {
		__data_start = .;
			KEEP(*start.o(*.vectors*))
			libstage1.a(.data)
			libstage1.a(.data.*)
			libstage1.a(.data.stage1)
			libstage2.a(.data.stage1)
			libstage1.a(.sdata)
			libstage1.a(.sdata.*)
			libstage1.a(.sdata2.*)
		__data_end = .;
		ASSERT((__data_end <= CONFIG_STAGE2_START), "No more Stage1 Space!!");
	} >STAGE1_MEM

	. = ALIGN(4);
	.text2 : {
		. = ALIGN(4);
		__text2_start = .;
			KEEP(libstage2.a(.text))
			KEEP(libstage2.a(.text.*))
			KEEP(libstage2.a(.text.stage2))
			libstage1.a(.text.stage2)
		__text2_end = .;
	} >STAGE2_MEM

	. = ALIGN(4);
	.rodata2 : {
		__rodata2_start = .;
			KEEP(libstage2.a(.rodata))
			KEEP(libstage2.a(.rodata.*))
			KEEP(libstage2.a(.rodata.stage2))
			KEEP(libstage2.a(.srodata))
			KEEP(libstage2.a(.srodata.*))
			KEEP(libstage2.a(.srodata2.*))
			libstage1.a(.rodata.stage2)
		__rodata2_end = .;
	} >STAGE2_MEM

	. = ALIGN(4);
	.data2 : {
		__data2_start = .;
			KEEP(libstage2.a(.data))
			KEEP(libstage2.a(.data.*))
			KEEP(libstage2.a(.data.stage2))
			KEEP(libstage2.a(.sdata))
			KEEP(libstage2.a(.sdata.*))
			KEEP(libstage2.a(.sdata2.*))
			libstage1.a(.data.stage2)
		__data2_end = .;
	} >STAGE2_MEM

	.tail : {
		tail.o(.tail)
	} >STAGE2_MEM

	. = ALIGN(4);
	.bin_end : {
		__bin_end_pad = .;
		LONG(CONFIG_BIN_END_PAD)
		__bin_end = .;
	} >STAGE2_MEM

	. = ALIGN(4);
	.bss : {
		__bss_start = .;
			*(.sbss)
			*(.sbss.*)
			*(.bss)
			*(.bss.*)
		__bss_end = .;
		KEEP(*vectors.o(.stack.trap))
		KEEP(*start.o(.stack))
		__stack_top = .;
	} >STAGE2_MEM

	. = ALIGN(2);
	_end = .;

	.assert : {
		ASSERT(((_end - CONFIG_TEXT_BASE) <= (CONFIG_MCU_SRAM_SIZE - CONFIG_PARAM_MEM_SIZE)), "No more Stage2 Space!!");
	}

	. = ALIGN(4);
	.param : {
		__param_start = .;
		__param_end = .;
	} >PARAM_MEM
}
