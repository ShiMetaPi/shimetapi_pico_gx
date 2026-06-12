
#ifndef _LOTUS_IMAGE_H_
#define _LOTUS_IMAGE_H_

#include <config.h>
#include <linux/lotus/hwzimage.h>

#define _MCU_HEAD_SIZE 64
#define _MCU_FW_MAGIC 0x57465652

struct mcu_fw_head {
	u32	j_instruction;     /* Jump to real excutable entry */
	u32	fw_magic;          /* MCU Firmware magic */
	u32	fw_ver;            /* MCU Firmware version */
	u32	fw_total_size;     /* MCU Firmware total size */
	u32	fw_stage1_size;    /* MCU Stage1 size */
	u8	resv[_MCU_HEAD_SIZE - 20]; /* Reserved */
};

#endif /* ifndef _LOTUS_IMAGE_H_ */
