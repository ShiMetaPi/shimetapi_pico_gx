
#ifndef _IMAGE_H_
#define _IMAGE_H_

#include <sys/types.h>

struct mcu_fw_head {
	u32	j_instruction;     /* Jump to real excutable entry */
	u32	fw_magic;          /* MCU Firmware magic */
	u32	fw_ver;            /* MCU Firmware version */
	u32	fw_total_size;     /* MCU Firmware total size */
	u32	fw_stage1_size;    /* MCU Stage1 size */
	u8	resv[CONFIG_MCU_HEAD_SIZE - 20]; /* Reserved */
};

#endif	/* _IMAGE_H_ */

