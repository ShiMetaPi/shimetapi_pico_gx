/*
 * Copyright (c) XMEDIA. All rights reserved.
 */

#include <stdio.h>
#include <errno.h>
#include <string.h>

#include <linux/videodev2.h>
#include <linux/usb/ch9.h>
#include <linux/usb/video.h>

#include "uvc_log.h"
#include "uvc_adapter.h"

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * +++++++++++++++++++ Requests infos +++++++++++++++++++++++++++
 */

/* 4.2.2.1 Camera Terminal Control Requests */
static char* vc_camera_terminal_cs(unsigned char cs_code)
{
    switch (cs_code) {
        case UVC_CT_SCANNING_MODE_CONTROL:
            return "Sanning Mode";
        case UVC_CT_AE_MODE_CONTROL:
            return "Auto-Exposure Mode";
        case UVC_CT_AE_PRIORITY_CONTROL:
            return "Auto-Exposure Priority";
        case UVC_CT_EXPOSURE_TIME_ABSOLUTE_CONTROL:
            return "Exposure Time(Absolute)";
        case UVC_CT_EXPOSURE_TIME_RELATIVE_CONTROL:
            return "Exposure Time(Relative)";
        case UVC_CT_FOCUS_ABSOLUTE_CONTROL:
            return "Focus(Absolute)";
        case UVC_CT_FOCUS_RELATIVE_CONTROL:
            return "Focus(Relative)";
        case UVC_CT_FOCUS_AUTO_CONTROL:
            return "Focus(Auto)";
        case UVC_CT_IRIS_ABSOLUTE_CONTROL:
            return "Iris(Absolute)";
        case UVC_CT_IRIS_RELATIVE_CONTROL:
            return "Iris(Relative)";
        case UVC_CT_ZOOM_ABSOLUTE_CONTROL:
            return "Zoom(Absolute)";
        case UVC_CT_ZOOM_RELATIVE_CONTROL:
            return "Zoom(Relative)";
        case UVC_CT_PANTILT_ABSOLUTE_CONTROL:
            return "PanTilt(Absolute)";
        case UVC_CT_PANTILT_RELATIVE_CONTROL:
            return "PanTilt(Relative)";
        case UVC_CT_ROLL_ABSOLUTE_CONTROL:
            return "Roll(Absolute)";
        case UVC_CT_ROLL_RELATIVE_CONTROL:
            return "Roll(Relative)";
        case UVC_CT_PRIVACY_CONTROL:
            return "Privacy Shutter";
        case UVC_CT_CONTROL_UNDEFINED:
        default:
            return "Invalid control";
    }
}

/* 4.2.2.2 Selector Unit Control Requests */
static char* vc_selector_unit_cs(unsigned char cs_code)
{
    switch (cs_code) {
        case UVC_SU_INPUT_SELECT_CONTROL:
            return "Selector Unit";
        case UVC_SU_CONTROL_UNDEFINED:
        default:
            return "Invalid control";
    }
}

/* 4.2.2.3 Processing Unit Control Requests */
static char* vc_processing_unit_cs(unsigned char cs_code)
{
    switch (cs_code) {
        case UVC_PU_BACKLIGHT_COMPENSATION_CONTROL:
            return "Backlight compensation";
        case UVC_PU_BRIGHTNESS_CONTROL:
            return "Brightness";
        case UVC_PU_CONTRAST_CONTROL:
            return "Contrast";
        case UVC_PU_GAIN_CONTROL:
            return "Gain";
        case UVC_PU_POWER_LINE_FREQUENCY_CONTROL:
            return "Power Line Frequency";
        case UVC_PU_HUE_CONTROL:
            return "Hue";
        case UVC_PU_HUE_AUTO_CONTROL:
            return "Hue,Auto";
        case UVC_PU_SATURATION_CONTROL:
            return "Saturation";
        case UVC_PU_SHARPNESS_CONTROL:
            return "Sharpness";
        case UVC_PU_GAMMA_CONTROL:
            return "Gamma";
        case UVC_PU_WHITE_BALANCE_TEMPERATURE_CONTROL:
            return "White Balance Temperature";
        case UVC_PU_WHITE_BALANCE_TEMPERATURE_AUTO_CONTROL:
            return "White Balance Temperature,Auto";
        case UVC_PU_WHITE_BALANCE_COMPONENT_CONTROL:
            return "White Balance Component";
        case UVC_PU_WHITE_BALANCE_COMPONENT_AUTO_CONTROL:
            return "White Balance Component,Auto";
        case UVC_PU_DIGITAL_MULTIPLIER_CONTROL:
            return "Digital Multiplier";
        case UVC_PU_DIGITAL_MULTIPLIER_LIMIT_CONTROL:
            return "Digital Multiplier Limit";
        case UVC_PU_ANALOG_VIDEO_STANDARD_CONTROL:
            return "Analog Video Standard";
        case UVC_PU_ANALOG_LOCK_STATUS_CONTROL:
            return "Analog Video Lock Status";
        case UVC_PU_CONTROL_UNDEFINED:
        default:
            return "Invalid control";
    }
}

/* 4.2.2.4 Extension Unit Control Requests */
static char* vc_extension_unit_h264_cs(unsigned char cs_code)
{
    return "Invalid control";
}

/* 4.2.1 Intercace Control Requests */
static char* vc_interface_cs(unsigned char cs_code)
{
    switch (cs_code) {
        case UVC_VC_VIDEO_POWER_MODE_CONTROL:
            return "Device Power Mode";
        case UVC_VC_REQUEST_ERROR_CODE_CONTROL:
            return "Request Error Code Control";
        case UVC_VC_CONTROL_UNDEFINED:
        default:
            return "Invalid control";
    }
}

/* 4.3.1 Interface Control Requests */
static char* vs_interface_cs(unsigned char cs_code)
{
    switch (cs_code) {
        case UVC_VS_PROBE_CONTROL:
            return "Video Probe";
        case UVC_VS_COMMIT_CONTROL:
            return "Video Commit";
        case UVC_VS_STILL_PROBE_CONTROL:
            return "Video Still Probe";
        case UVC_VS_STILL_COMMIT_CONTROL:
            return "Video Still Commit";
        case UVC_VS_STILL_IMAGE_TRIGGER_CONTROL:
            return "Still Image Trigger";
        case UVC_VS_STREAM_ERROR_CODE_CONTROL:
            return "Stream Error Code";
        case UVC_VS_GENERATE_KEY_FRAME_CONTROL:
            return "Generate Key Frame";
        case UVC_VS_UPDATE_FRAME_SEGMENT_CONTROL:
            return "Update Frame Segment";
        case UVC_VS_SYNC_DELAY_CONTROL:
            return "Sync Delay";
        case UVC_VS_CONTROL_UNDEFINED:
        default:
            return "Invalid control";
    }
}

/* 4.2.2 Unit and Terminal Control Requests */
static char* vc_entity_name(unsigned char entity_id)
{
    switch (entity_id) {
        case UVC_CAMERA_TERMINAL_ID:
            return "Camera Terminal";
        case UVC_PROCESSING_UNIT_ID:
            return "Processing Unit";
        case UVC_OUTPUT_TERMINAL_ID:
            return "Output Terminal";
        case UVC_SELECTOR_UNIT_ID:
            return "Selector Unit";
        case UVC_EXTENSION_UNIT_H264_ID:
            return "Extension Unit H264";
        default:
            return "Invalid Unit";
    }
}

static char* control_entity_cs(unsigned char entity_id, unsigned char cs_code)
{
    switch (entity_id) {
        case UVC_CAMERA_TERMINAL_ID:
            return vc_camera_terminal_cs(cs_code);
        case UVC_PROCESSING_UNIT_ID:
            return vc_processing_unit_cs(cs_code);
        case UVC_OUTPUT_TERMINAL_ID:
            return "Unsupported unit";
        case UVC_SELECTOR_UNIT_ID:
            return vc_selector_unit_cs(cs_code);
        case UVC_EXTENSION_UNIT_H264_ID:
            return vc_extension_unit_h264_cs(cs_code);
        default:
            return "Invalid unit";
    }
}

/* 4.2.1 Intercace Control Requests */
/* 4.3.1 Interface Control Requests */
static char* control_interface_name(unsigned char interface_id)
{
    switch (interface_id) {
        case UVC_VS_INTERFACE_ID:
            return "VS Interface";
        case UVC_VC_INTERFACE_ID:
            return "VC Interface";
        default:
            return "Invalid Interface";
    }
}

static char* control_interface_cs(unsigned char interface_id, unsigned char cs_code)
{
    switch (interface_id) {
        case UVC_VC_INTERFACE_ID:
            return vc_interface_cs(cs_code);
        case UVC_VS_INTERFACE_ID:
            return vs_interface_cs(cs_code);
        default:
            return "Invalid Interface";
    }
}

/* 4.1 Requests Layout */
static char* request_name(unsigned char req_id)
{
    switch (req_id) {
        case UVC_SET_CUR:
            return "Set Cur";
        case UVC_GET_CUR:
            return "Get Cur";
        case UVC_GET_MIN:
            return "Get Min";
        case UVC_GET_MAX:
            return "Get Max";
        case UVC_GET_RES:
            return "Get Res";
        case UVC_GET_LEN:
            return "Get Len";
        case UVC_GET_INFO:
            return "Get Info";
        case UVC_GET_DEF:
            return "Get Def";
        case UVC_RC_UNDEFINED:
        default:
            return "Invalid request";
    }
}

/* +++++++++++++++++++ Requests infos +++++++++++++++++++++++++++
 * ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */

/* Requests Debug */
/*
 * Node: f_uvc.c 中定义 entity_id，各entity的ID不会为0;
 *       f_uvc.c 中定义 video control interface 为 0;
 *       f_uvc.c 中定义 video streaming interface 为1;
 */
void uvc_requests_infos(struct usb_ctrlrequest* ctrl)
{
    // unit_id != 0, video Control
    unsigned char entity_id = MASK_ENTITY_ID(ctrl);

    // interface_id == 0 : video control interface
    // interface_id == 1 : video streaming interface
    unsigned char interface_id = MASK_INTF_ID(ctrl);

    unsigned char cs_code = MASK_CS_CODE(ctrl);
    unsigned char req_id = MASK_REQ_CODE(ctrl);

    uvc_logd("reqeust type     : INTERFACE.\n");
    uvc_logd("class-specific   : 0x%02x(%s).\n",
            interface_id, interface_id ? "Video Streaming" : "Video Control");
    if (entity_id) {
        uvc_logd("entity id        : 0x%02x(%s)\n", entity_id, vc_entity_name(entity_id));
        uvc_logd("control selector : 0x%02x(%s)\n", cs_code, control_entity_cs(entity_id, cs_code));
    } else {
        uvc_logd("interface        : 0x%02x(%s)\n", interface_id, control_interface_name(interface_id));
        uvc_logd("control selector : 0x%02x(%s)\n", cs_code, control_interface_cs(interface_id, cs_code));
    }
    uvc_logd("requests         : 0x%02x(%s)\n",req_id, request_name(req_id));
}
