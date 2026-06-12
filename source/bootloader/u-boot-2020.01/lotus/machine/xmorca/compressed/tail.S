 /*
  * Copyright (c) LOTUS. All rights reserved.
  */
 #include <config.h>

.section .tail,#alloc

.align  9 /* align to 0x200 */

.globl _checked_area_end
_checked_area_end:

/* boot code area signature    */
_boot_sign_area:
	.word CONFIG_BOOT_CODE_AREA_MAGIC
	.fill 0x1FC,1,0 /* align to the signature size: 512 bytes */

.globl _total_boot_area_end
_total_boot_area_end:

