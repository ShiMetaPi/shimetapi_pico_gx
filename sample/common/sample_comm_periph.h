#ifndef __SAMPLE_COMM_PERIPHERAL_H__
#define __SAMPLE_COMM_PERIPHERAL_H__

#include "xmedia_type.h"

xmedia_s32 sample_comm_periph_mount_sdcard(const xmedia_char* mount_point, xmedia_s32 timeout_ms);
xmedia_s32 sample_comm_periph_umount_sdcard(const xmedia_char *mount_path);
xmedia_s32 sample_comm_periph_drv_bind(const xmedia_char *device, const xmedia_char *driver, const xmedia_char *notice_str);
xmedia_s32 sample_comm_periph_drv_unbind(const xmedia_char *device, const xmedia_char *driver, const xmedia_char *notice_str);


#endif /*__SAMPLE_COMM_PERIPHERAL_H__*/
