/*
 * Copyright (c) XMEDIA. All rights reserved.
 */
#ifndef __UVC_ADAPTER_H__
#define __UVC_ADAPTER_H__

/* For Requests */
/* 和内核中的定义一致 */
/* For VideoControl Requests */
#define UVC_VC_INTERFACE_ID    0x00
#define UVC_CAMERA_TERMINAL_ID 0x01
#define UVC_PROCESSING_UNIT_ID 0x02
#define UVC_SELECTOR_UNIT_ID   0x03
#define UVC_OUTPUT_TERMINAL_ID 0x05
#define UVC_EXTENSION_UNIT_H264_ID  0x0A
#define UVC_EXTENSION_UNIT_CAMERA_ID  0x11
#define UVC_EXTENSION_UNIT_VENDOR1_ID 0x0B

/* For VideoStreaming Requests*/
#define UVC_VS_INTERFACE_ID    0x01

/* Requests Error Code */
#define UVC_REC_NO_ERROR        0x00
#define UVC_REC_NO_READY        0x01
#define UVC_REC_WRONG_STATE     0x02
#define UVC_REC_POWER           0x03
#define UVC_REC_OUT_OF_RANGE    0x04
#define UVC_REC_INVALID_UNIT    0x05
#define UVC_REC_INVALID_CONTROL 0x06
#define UVC_REC_INVALID_REQUEST 0x07
#define UVC_REC_UNKNOW          0xFF

/* Stream Error Code */

/* CTRL id */
#define XUID_SET_RESET      0x01
#define XUID_SET_STREAM     0x02
#define XUID_SET_RESOLUTION 0x03
#define XUID_SET_IFRAME     0x04
#define XUID_SET_BITRATE    0x05
#define XUID_UPDATE_SYSTEM  0x06

/* VENDOR1 CTRL id */
#define XUID_VENDOR_01 0x01
#define XUID_VENDOR_02 0x02
#define XUID_VENDOR_03 0x03
#define XUID_VENDOR_04 0x04
#define XUID_VENDOR_05 0x05
#define XUID_VENDOR_06 0x06
#define XUID_VENDOR_07 0x07
#define XUID_VENDOR_08 0x08
#define XUID_VENDOR_09 0x09
#define XUID_VENDOR_10 0x0A
// ...
//#define XUID_VENDOR_32 0x20

/* */
#define MASK_INTF_ID(usb_ctrl) ((usb_ctrl)->wIndex & 0xFF)
#define MASK_ENTITY_ID(usb_ctrl) (((usb_ctrl)->wIndex & 0xFF00) >> 8)
#define MASK_CS_CODE(usb_ctrl) ((usb_ctrl)->wValue >> 8)
#define MASK_REQ_CODE(usb_ctrl) ((usb_ctrl)->bRequest)

void uvc_requests_infos(struct usb_ctrlrequest* ctrl);

#endif
