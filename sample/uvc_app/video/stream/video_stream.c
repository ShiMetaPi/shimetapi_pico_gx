/*
 * Copyright (c) XMEDIA. All rights reserved.
 */
#include <stdio.h>
#include <errno.h>

#include "video_stream.h"

#include "uvc_log.h"
#include "uvc_adapter.h"

static struct stream __stream = {
    .mpi_sc_ops = NULL,
    .mpi_pu_ops = NULL,
    .mpi_it_ops = NULL,
    .mpi_eu_ops = NULL,
    .streaming = 0,
    .exposure_auto_stall = 0,
    .brightness_stall = 0,
};

static uint32_t focus_absolute_value = 0;

static stream *get_stream(void)
{
    return &__stream;
}

static void update_req_err_code(struct uvc_device *dev, unsigned char code)
{
    dev->request_error_code.length  = 1;
    dev->request_error_code.data[0] = code;
}

/* stream Stream Control Operation Functions End */
static void _stream_event_default_setup(uint8_t req, struct uvc_request_data *resp)
{
    switch (req) {
        case UVC_GET_MIN:
            resp->length = 4;
            break;
        case UVC_GET_LEN:
            resp->data[0] = 0x04;
            resp->data[1] = 0x00;
            resp->length = 2;
            break;
        case UVC_GET_INFO:
            resp->data[0] = 0x03;
            resp->length = 1;
            break;
        case UVC_GET_CUR:
            resp->length = 1;
            resp->data[0] = 0x01;
            break;
        default:
            resp->length = 1;
            resp->data[0] = 0x06;
            break;
    }
}

/*******************************************************
 *  stream Processing Unit Operation Functions
 *******************************************************/
static void _stream_pu_brightness_ctrl(struct uvc_device *dev, uint8_t req, struct uvc_request_data *resp)
{
    uint16_t v;
    uint8_t err_code = UVC_REC_NO_ERROR;

    v = (uint16_t)stream_pu_get_brightness();

    switch (req) {
        case UVC_SET_CUR:
            resp->length = 2;
            break;
        case UVC_GET_CUR:
            if(get_stream()->brightness_stall) {
                err_code = UVC_REC_WRONG_STATE;
                resp->length = -EL2HLT;
                get_stream()->brightness_stall = 0;
            } else {
                resp->length = 2;
                resp->data[0] = (v & 0xff);
                resp->data[1] = ((v >> 8) & 0xff);
            }
            break;
        case UVC_GET_MIN:
            resp->length = 2;
            resp->data[0] = 0x00;
            resp->data[1] = 0x00;
            break;
        case UVC_GET_MAX:
            resp->length = 2;
            resp->data[0] = 0x64;
            resp->data[1] = 0x00;
            break;
        case UVC_GET_DEF:
            resp->length = 2;
            resp->data[0] = 0x32;
            resp->data[1] = 0x00;
            break;
        case UVC_GET_RES:
            resp->data[0] = 0x01;
            resp->data[1] = 0x00;
            resp->length = 2;
            break;
        case UVC_GET_INFO:
            resp->data[0] = 0x03;
            resp->length = 1;
            break;
        default:
            resp->length = -EL2HLT;
            err_code = UVC_REC_INVALID_REQUEST;
            break;
    }

    update_req_err_code(dev, err_code);
}

static void _stream_pu_brightness_set(struct uvc_request_data *data)
{
    uint16_t v;

    v = data->data[0] + (data->data[1] << 8);
    stream_pu_set_brightness(v);
}

void _stream_pu_contrast_ctrl(struct uvc_device *dev, uint8_t req, struct uvc_request_data *resp)
{
    uint8_t err_code = UVC_REC_NO_ERROR;
    uint16_t v = stream_pu_get_contrast();

    switch (req) {
        case UVC_SET_CUR:
            resp->length = 2;
            break;
        case UVC_GET_CUR:
            resp->length = 2;
            resp->data[0] = (v & 0xff);
            resp->data[1] = ((v >> 8) & 0xff);
            break;
        case UVC_GET_MIN:
            resp->length = 2;
            resp->data[0] = 0x00;
            resp->data[1] = 0x00;
            break;
        case UVC_GET_MAX:
            resp->length = 2;
            resp->data[0] = 0x64;
            resp->data[1] = 0x00;
            break;
        case UVC_GET_LEN:
            resp->data[0] = 0x02;
            resp->data[1] = 0x00;
            resp->length = 2;
            break;
        case UVC_GET_INFO:
            resp->data[0] = 0x03;
            resp->length = 1;
            break;
        default:
            resp->length = -EL2HLT;
            err_code = UVC_REC_INVALID_REQUEST;
            break;
    }

    update_req_err_code(dev, err_code);
}

static void _stream_pu_contrast_set(struct uvc_request_data *data)
{
    uint16_t v = 0;

    if (get_stream()->streaming != 1) {
        return;
    }

    v = data->data[0] + (data->data[1] << 8);
    stream_pu_set_contrast(v);
}

void _stream_pu_hue_ctrl(struct uvc_device *dev, uint8_t req, struct uvc_request_data *resp)
{
    uint16_t v = 50;
    uint8_t err_code = UVC_REC_NO_ERROR;

    if (get_stream()->streaming == 1) {
        if (get_stream()->mpi_pu_ops &&
                get_stream()->mpi_pu_ops->hue_get)
            v = get_stream()->mpi_pu_ops->hue_get();
    }

    switch (req) {
        case UVC_SET_CUR:
            resp->length = 2;
            break;
        case UVC_GET_CUR:
            resp->length = 2;
            resp->data[0] = (v & 0xff);
            resp->data[1] = ((v >> 8) & 0xff);
            break;
        case UVC_GET_MIN:
            resp->length = 2;
            resp->data[0] = 0x00;
            resp->data[1] = 0x00;
            break;
        case UVC_GET_MAX:
            resp->length = 2;
            resp->data[0] = 0x64;
            resp->data[1] = 0x00;
            break;
        case UVC_GET_LEN:
            resp->data[0] = 0x02;
            resp->data[1] = 0x00;
            resp->length = 2;
            break;
        case UVC_GET_INFO:
            resp->data[0] = 0x03;
            resp->length = 1;
            break;
        default:
            resp->length = -EL2HLT;
            err_code = UVC_REC_INVALID_REQUEST;
            break;
    }

    update_req_err_code(dev, err_code);
}

static void _stream_pu_hue_set(struct uvc_request_data *data)
{
    uint16_t v = 0;

    if (get_stream()->streaming != 1) {
        return;
    }

    v = data->data[0] + (data->data[1] << 8);

    if (get_stream()->mpi_pu_ops &&
            get_stream()->mpi_pu_ops->hue_set) {
        get_stream()->mpi_pu_ops->hue_set(v);
    }
}

void _stream_pu_power_line_frequency_ctrl(struct uvc_device *dev, uint8_t req, struct uvc_request_data *resp)
{
    uint8_t v = 0;
    uint8_t err_code = UVC_REC_NO_ERROR;

    if (get_stream()->streaming == 1) {
        if (get_stream()->mpi_pu_ops &&
                get_stream()->mpi_pu_ops->power_line_frequency_get)
            v = get_stream()->mpi_pu_ops->power_line_frequency_get();
    }

    switch (req) {
        case UVC_SET_CUR:
            resp->length = 1;
            break;
        case UVC_GET_CUR:
            resp->length = 1;
            resp->data[0] = v;
            break;
        case UVC_GET_DEF:
            resp->length = 1;
            resp->data[0] = 0x1;
            break;
        case UVC_GET_INFO:
            resp->data[0] = 0x03;
            resp->length = 1;
            break;
        default:
            resp->length = -EL2HLT;
            err_code = UVC_REC_INVALID_REQUEST;
            break;
    }

    update_req_err_code(dev, err_code);
}

static void _stream_pu_power_line_frequency_set(struct uvc_request_data *data)
{
    uint8_t v = 0;

    if (get_stream()->streaming != 1) {
        return;
    }

    v = data->data[0];

    if (get_stream()->mpi_pu_ops &&
            get_stream()->mpi_pu_ops->power_line_frequency_set) {
        get_stream()->mpi_pu_ops->power_line_frequency_set(v);
    }
}

void _stream_pu_saturation_ctrl(struct uvc_device *dev, uint8_t req, struct uvc_request_data *resp)
{
    uint16_t v = 50;
    uint8_t err_code = UVC_REC_NO_ERROR;

    if (get_stream()->streaming == 1) {
        if (get_stream()->mpi_pu_ops &&
                get_stream()->mpi_pu_ops->saturation_get)
            v = get_stream()->mpi_pu_ops->saturation_get();
    }

    switch (req) {
        case UVC_SET_CUR:
            resp->length = 2;
            break;
        case UVC_GET_CUR:
            resp->length = 2;
            resp->data[0] = (v & 0xff);
            resp->data[1] = ((v >> 8) & 0xff);
            break;
        case UVC_GET_MIN:
            resp->length = 2;
            resp->data[0] = 0x00;
            resp->data[1] = 0x00;
            break;
        case UVC_GET_MAX:
            resp->length = 2;
            resp->data[0] = 0x64;
            resp->data[1] = 0x00;
            break;
        case UVC_GET_LEN:
            resp->data[0] = 0x02;
            resp->data[1] = 0x00;
            resp->length = 2;
            break;
        case UVC_GET_INFO:
            resp->data[0] = 0x03;
            resp->length = 1;
            break;
        default:
            resp->length = -EL2HLT;
            err_code = UVC_REC_INVALID_REQUEST;
            break;
    }

    update_req_err_code(dev, err_code);
}

static void _stream_pu_saturation_set(struct uvc_request_data *data)
{
    uint16_t v = 0;

    if (get_stream()->streaming != 1) {
        return;
    }

    v = data->data[0] + (data->data[1] << 8);

    if (get_stream()->mpi_pu_ops &&
            get_stream()->mpi_pu_ops->saturation_set) {
        get_stream()->mpi_pu_ops->saturation_set(v);
    }
}

void _stream_pu_white_balance_temperature_auto_ctrl(struct uvc_device *dev, uint8_t req, struct uvc_request_data *resp)
{
    uint8_t v = 0;
    uint8_t err_code = UVC_REC_NO_ERROR;

    if (get_stream()->streaming == 1) {
        if (get_stream()->mpi_pu_ops &&
                get_stream()->mpi_pu_ops->white_balance_temperature_auto_get)
            v = get_stream()->mpi_pu_ops->white_balance_temperature_auto_get();
    }

    switch (req) {
        case UVC_SET_CUR:
            resp->length = 1;
            break;
        case UVC_GET_CUR:
            resp->length = 1;
            resp->data[0] = v;
            break;
        case UVC_GET_DEF:
            resp->length = 1;
            resp->data[0] = 0x1;
            break;
        case UVC_GET_INFO:
            resp->data[0] = 0x03;
            resp->length = 1;
            break;
        default:
            resp->length = -EL2HLT;
            err_code = UVC_REC_INVALID_REQUEST;
            break;
    }

    update_req_err_code(dev, err_code);
}

static void _stream_pu_white_balance_temperature_auto_set(struct uvc_request_data *data)
{
    uint8_t v = 0;

    if (get_stream()->streaming != 1) {
        return;
    }

    v = data->data[0];

    if (get_stream()->mpi_pu_ops &&
            get_stream()->mpi_pu_ops->white_balance_temperature_auto_set) {
        get_stream()->mpi_pu_ops->white_balance_temperature_auto_set(v);
    }
}

void _stream_pu_white_balance_temperature_ctrl(struct uvc_device *dev, uint8_t req, struct uvc_request_data *resp)
{
    uint16_t v = 15000;
    uint8_t err_code = UVC_REC_NO_ERROR;

    if (get_stream()->streaming == 1) {
        if (get_stream()->mpi_pu_ops &&
                get_stream()->mpi_pu_ops->white_balance_temperature_get)
            v = get_stream()->mpi_pu_ops->white_balance_temperature_get();
    }

    switch (req) {
        case UVC_SET_CUR:
            resp->length = 2;
            break;
        case UVC_GET_CUR:
            resp->length = 2;
            resp->data[0] = (v & 0xff);
            resp->data[1] = ((v >> 8) & 0xff);
            break;
        case UVC_GET_MIN:
            resp->length = 2;
            resp->data[0] = (1500 & 0xff);
            resp->data[1] = (1500 / 256);
            break;
        case UVC_GET_MAX:
            resp->length = 2;
            resp->data[0] = (15000 & 0xff);
            resp->data[1] = (15000 / 256);
            break;
        case UVC_GET_LEN:
            resp->data[0] = 0x02;
            resp->data[1] = 0x00;
            resp->length = 2;
            break;
        case UVC_GET_INFO:
            resp->data[0] = 0x03;
            resp->length = 1;
            break;
        default:
            resp->length = -EL2HLT;
            err_code = UVC_REC_INVALID_REQUEST;
            return;
    }

    update_req_err_code(dev, err_code);
}

static void _stream_pu_white_balance_temperature_set(struct uvc_request_data *data)
{
    uint16_t v = 0;

    if (get_stream()->streaming != 1) {
        return;
    }

    v = data->data[0] + (data->data[1] << 8);

    if (get_stream()->mpi_pu_ops &&
            get_stream()->mpi_pu_ops->white_balance_temperature_set) {
        get_stream()->mpi_pu_ops->white_balance_temperature_set(v);
    }
}

void stream_event_pu_setup(struct uvc_device *dev,
        uint8_t req,
        uint8_t unit_id,
        uint8_t cs,
        struct uvc_request_data* resp)
{
    if (unit_id != UVC_PROCESSING_UNIT_ID) {
        uvc_loge("Invalid unit.\n");
        update_req_err_code(dev, UVC_REC_INVALID_UNIT);
        return ;
    }

    switch(cs) {
        case UVC_PU_BRIGHTNESS_CONTROL:
            _stream_pu_brightness_ctrl(dev, req, resp);
            break;
        case UVC_PU_HUE_CONTROL:
            _stream_pu_hue_ctrl(dev, req, resp);
            break;
        case UVC_PU_CONTRAST_CONTROL:
            _stream_pu_contrast_ctrl(dev, req, resp);
            break;
        case UVC_PU_SATURATION_CONTROL:
            _stream_pu_saturation_ctrl(dev, req, resp);
            break;
        case UVC_PU_WHITE_BALANCE_COMPONENT_AUTO_CONTROL:
            break;
        case UVC_PU_WHITE_BALANCE_COMPONENT_CONTROL:
            break;
        case UVC_PU_WHITE_BALANCE_TEMPERATURE_CONTROL:
            _stream_pu_white_balance_temperature_ctrl(dev, req, resp);
            break;
        case UVC_PU_WHITE_BALANCE_TEMPERATURE_AUTO_CONTROL:
            _stream_pu_white_balance_temperature_auto_ctrl(dev, req, resp);
            break;
        case UVC_PU_POWER_LINE_FREQUENCY_CONTROL:
            _stream_pu_power_line_frequency_ctrl(dev, req, resp);
            break;
        default:
            resp->length = -EL2HLT;
            update_req_err_code(dev, UVC_REC_INVALID_CONTROL);
            break;
    }
}

void stream_event_pu_data(int unit_id, int control, struct uvc_request_data *data)
{
    if (unit_id != UVC_PROCESSING_UNIT_ID)
        return ;

    switch(control) {
        case UVC_PU_BRIGHTNESS_CONTROL:
            return _stream_pu_brightness_set(data);
        case UVC_PU_CONTRAST_CONTROL:
            return _stream_pu_contrast_set(data);
        case UVC_PU_HUE_CONTROL:
            return _stream_pu_hue_set(data);
        case UVC_PU_SATURATION_CONTROL:
            return _stream_pu_saturation_set(data);
        case UVC_PU_WHITE_BALANCE_TEMPERATURE_AUTO_CONTROL:
            return _stream_pu_white_balance_temperature_auto_set(data);
        case UVC_PU_WHITE_BALANCE_TEMPERATURE_CONTROL:
            return _stream_pu_white_balance_temperature_set(data);
        case UVC_PU_POWER_LINE_FREQUENCY_CONTROL:
            return _stream_pu_power_line_frequency_set(data);
        default:
            break;
    }
}
/* stream Processing Unit Operation Functions End */

/*******************************************************
 *  stream Input Terminal Operation Functions
 *******************************************************/
static void _stream_it_exposure_auto_mode_ctrl(struct uvc_device *dev,
        uint8_t req,
        struct uvc_request_data *resp)
{
    uint8_t v = 0x04;
    uint8_t e = UVC_REC_NO_ERROR;

    if (get_stream()->streaming == 1) {
        if (get_stream()->mpi_it_ops &&
                get_stream()->mpi_it_ops->exposure_auto_mode_get)
            v = get_stream()->mpi_it_ops->exposure_auto_mode_get();
    }

    switch (req) {
        case UVC_SET_CUR:
            resp->length = 1;
            break;
        case UVC_GET_CUR:
            if (get_stream()->exposure_auto_stall) {
                resp->length = -EL2HLT;
                get_stream()->exposure_auto_stall = 0;
                e = UVC_REC_WRONG_STATE;
            } else {
                resp->data[0] = v;
                resp->length = 1;
            }
            break;
        case UVC_GET_RES:
            resp->data[0] = 0x05;
            resp->length = 1;
            break;
        case UVC_GET_DEF:
            resp->data[0] = 0x04;
            resp->length = 1;
            break;
        case UVC_GET_INFO:
            resp->data[0] = 0x3;
            resp->length = 1;
            break;
        default:
            resp->length = -EL2HLT;
            e = UVC_REC_INVALID_REQUEST;
            break;
    }

    update_req_err_code(dev, e);
}

static void _stream_it_exposure_auto_mode_set(struct uvc_request_data *data)
{
    uint8_t v = 0x04;

    if (get_stream()->streaming != 1) {
        return;
    }

    v = data->data[0];
    switch(v) {
        case 0x01:
        case 0x02:
        case 0x04:
        case 0x08:
            get_stream()->exposure_auto_stall = 0;
            break;
        default:
            get_stream()->exposure_auto_stall = 1;
            return;
    }

    if (get_stream()->mpi_it_ops &&
            get_stream()->mpi_it_ops->exposure_auto_mode_set) {
        get_stream()->mpi_it_ops->exposure_auto_mode_set(v);
    }
}

static void _stream_it_exposure_absolute_time_ctrl(struct uvc_device *dev,
        uint8_t req,
        struct uvc_request_data *resp)
{
    uint32_t v = 2000;
    uint8_t e = UVC_REC_NO_ERROR;

    if (get_stream()->streaming == 1) {
        if (get_stream()->mpi_it_ops &&
                get_stream()->mpi_it_ops->exposure_ansolute_time_get)
            v = get_stream()->mpi_it_ops->exposure_ansolute_time_get();
    }

    switch (req) {
        case UVC_SET_CUR:
            resp->length = 4;
            break;
        case UVC_GET_CUR:
            resp->length = 4;
            resp->data[0] = (v & 0xff);
            resp->data[1] = ((v >> 8) & 0xff);
            resp->data[2] = ((v >> 16) & 0xff);
            resp->data[3] = ((v >> 24) & 0xff);
            break;
        case UVC_GET_MAX:
            resp->length = 4;
            resp->data[0] = (2000 & 0xff);
            resp->data[1] = ((2000 >> 8) & 0xff);
            resp->data[2] = 0x0;
            resp->data[3] = 0x0;
            break;
        case UVC_GET_MIN:
            resp->length = 4;
            resp->data[0] = 0x0a;
            resp->data[1] = 0x0;
            resp->data[2] = 0x0;
            resp->data[3] = 0x0;
            break;
        case UVC_GET_LEN:
            resp->data[0] = 0x04;
            resp->data[1] = 0x00;
            resp->length = 2;
            break;
        case UVC_GET_RES:
            resp->data[0] = 0x0a;
            resp->data[1] = 0x0;
            resp->data[2] = 0x0;
            resp->data[3] = 0x0;
            resp->length = 4;
            break;
        case UVC_GET_DEF:
            resp->data[0] = 0x64;
            resp->data[1] = 0x0;
            resp->data[2] = 0x0;
            resp->data[3] = 0x0;
            resp->length = 4;
            break;
        case UVC_GET_INFO:
            resp->data[0] = 0xF;
            resp->length = 1;
            break;
        default:
            resp->length = -EL2HLT;
            e = UVC_REC_INVALID_REQUEST;
            break;
    }

    update_req_err_code(dev, e);
}

static void _stream_it_exposure_absolute_time_set(struct uvc_request_data *data)
{
    uint32_t v = 0;

    if (get_stream()->streaming != 1) {
        return;
    }

    v = (data->data[0] +
            (data->data[1] << 8) +
            (data->data[2] << 16) +
            (data->data[3] << 24)) * 100;

    if (get_stream()->mpi_it_ops &&
            get_stream()->mpi_it_ops->exposure_ansolute_time_set) {
        get_stream()->mpi_it_ops->exposure_ansolute_time_set(v);
    }
}

static void _stream_it_focus_absolute_value_ctrl(struct uvc_device *dev,
        uint8_t req,
        struct uvc_request_data *resp)
{
    uint32_t v = 0;
    uint8_t e = UVC_REC_NO_ERROR;

    if (get_stream()->streaming == 1) {
        v = focus_absolute_value;
    }

    switch (req) {
        case UVC_SET_CUR:
            resp->length = 2;
            break;
        case UVC_GET_CUR:
            resp->length = 2;
            resp->data[0] = (v & 0xff);
            resp->data[1] = ((v >> 8) & 0xff);
            break;
        case UVC_GET_MAX:
            resp->length = 2;
            resp->data[0] = (0x20 & 0xff);
            resp->data[1] = (0x00 & 0xff);
            break;
        case UVC_GET_MIN:
            resp->length = 2;
            resp->data[0] = 0x0;
            resp->data[1] = 0x0;
            break;
        case UVC_GET_LEN:
            resp->data[0] = 0x02;
            resp->data[1] = 0x00;
            resp->length = 2;
            break;
        case UVC_GET_RES:
            resp->data[0] = 0x1;
            resp->length = 2;
            break;
        case UVC_GET_DEF:
            resp->data[0] = 0x00;
            resp->length = 2;
            break;
        case UVC_GET_INFO:
            resp->data[0] = 0x3;
            resp->length = 1;
            break;
        default:
            resp->length = -EL2HLT;
            e = UVC_REC_INVALID_REQUEST;
            break;
    }

    update_req_err_code(dev, e);
}

static void _stream_it_focus_absolute_value_set(struct uvc_request_data *data)
{
    uint32_t v = 0;

    if (get_stream()->streaming != 1) {
        return;
    }

    v = data->data[0] + (data->data[1] << 8);
    focus_absolute_value = v;
    uvc_logi("focus_absolute: %u.\n", focus_absolute_value);
}

void stream_event_it_setup(struct uvc_device *dev,
        uint8_t req,
        uint8_t unit_id,
        uint8_t cs,
        struct uvc_request_data *resp)
{
    if (unit_id != UVC_CAMERA_TERMINAL_ID) {
        resp->length = -EL2HLT;
        update_req_err_code(dev, UVC_REC_INVALID_UNIT);
        uvc_loge("Invalid unit.\n");
        return;
    }

    if (get_stream()->exposure_auto_stall) {
        resp->length = -EL2HLT;
        get_stream()->exposure_auto_stall = 0;
        update_req_err_code(dev, UVC_REC_WRONG_STATE);
        uvc_loge("Wrong state.\n");
        return;
    }

    switch(cs) {
        case UVC_CT_AE_MODE_CONTROL:
            _stream_it_exposure_auto_mode_ctrl(dev, req, resp);
            break;
        case UVC_CT_EXPOSURE_TIME_ABSOLUTE_CONTROL:
            _stream_it_exposure_absolute_time_ctrl(dev, req, resp);
            break;
        case UVC_CT_FOCUS_ABSOLUTE_CONTROL:
            _stream_it_focus_absolute_value_ctrl(dev, req, resp);
            break;
        case UVC_CT_EXPOSURE_TIME_RELATIVE_CONTROL:
        case UVC_CT_AE_PRIORITY_CONTROL:
        default:
            resp->length = -EL2HLT;
            update_req_err_code(dev, UVC_REC_INVALID_CONTROL);
            uvc_logd("Invalid control.\n");
            break;
    }
}

void stream_event_it_data(int unit_id, int control, struct uvc_request_data *data)
{
    if (unit_id != UVC_CAMERA_TERMINAL_ID) {
        uvc_loge("Invalid unit.\n");
        return ;
    }

    switch(control) {
        case UVC_CT_AE_MODE_CONTROL:
            _stream_it_exposure_auto_mode_set(data);
            break;
        case UVC_CT_EXPOSURE_TIME_ABSOLUTE_CONTROL:
            _stream_it_exposure_absolute_time_set(data);
            break;
        case UVC_CT_FOCUS_ABSOLUTE_CONTROL:
            _stream_it_focus_absolute_value_set(data);
            break;
        default:
            uvc_loge("Invalid control.\n");
            break;
    }
}

/* stream Input Terminal Operation Functions End */

/*******************************************************
 *  stream Extension Unit vendor1 Operation Functions
 *******************************************************/
static uint32_t g_vctrl01_test = 0;
static void _stream_eu_vendor1_ctrl01_setup(struct uvc_device *dev, uint8_t req, struct uvc_request_data* resp)
{
    uint32_t v = g_vctrl01_test;
    uint8_t e = UVC_REC_NO_ERROR;

    switch (req) {
        case UVC_SET_CUR:
            resp->length = 4;
            break;
        case UVC_GET_CUR:
            resp->data[0] = v & 0xFF;
            resp->data[1] = ((v >> 8) & 0xFF);
            resp->data[2] = ((v >> 16) & 0xFF);
            resp->data[3] = ((v >> 24) & 0xFF);
            resp->length  = 4;
            break;
        case UVC_GET_MIN:
            v = 0;
            resp->data[0] = v & 0xFF;
            resp->data[1] = ((v >> 8) & 0xFF);
            resp->data[2] = ((v >> 16) & 0xFF);
            resp->data[3] = ((v >> 24) & 0xFF);
            resp->length  = 4;
            break;
        case UVC_GET_MAX:
            v = 100;
            resp->data[0] = v & 0xFF;
            resp->data[1] = ((v >> 8) & 0xFF);
            resp->data[2] = ((v >> 16) & 0xFF);
            resp->data[3] = ((v >> 24) & 0xFF);
            resp->length  = 4;
            break;
        case UVC_GET_RES:
            v = 1;
            resp->data[0] = v & 0xFF;
            resp->data[1] = ((v >> 8) & 0xFF);
            resp->data[2] = ((v >> 16) & 0xFF);
            resp->data[3] = ((v >> 24) & 0xFF);
            resp->length  = 4;
            break;
        case UVC_GET_LEN:
            resp->data[0] = 0x04;
            resp->data[1] = 0x00;
            resp->length  = 2;
            break;
        case UVC_GET_INFO:
            resp->data[0] = 0x03;
            resp->length  = 1;
            break;
        case UVC_GET_DEF:
            v = 50;
            resp->data[0] = v & 0xFF;
            resp->data[1] = ((v >> 8) & 0xFF);
            resp->data[2] = ((v >> 16) & 0xFF);
            resp->data[3] = ((v >> 24) & 0xFF);
            resp->length  = 4;
            break;
        default:
            uvc_loge("Invalid request.\n");
            resp->length = -EL2HLT;
            e = UVC_REC_INVALID_REQUEST;
            break;
    }
    update_req_err_code(dev, e);
}

void stream_event_eu_vendor1_setup(struct uvc_device *dev,
        uint8_t req,
        uint8_t unit_id,
        uint8_t cs,
        struct uvc_request_data* resp)
{
    if (unit_id != UVC_EXTENSION_UNIT_VENDOR1_ID) {
        uvc_loge("Invalid unit.\n");
        return;
    }

    switch (cs) {
        case XUID_VENDOR_01:
            _stream_eu_vendor1_ctrl01_setup(dev, req, resp);
            break;
        default:
            uvc_loge("Invalid control selector: %02x\n", cs);
            resp->length = -EL2HLT;
            update_req_err_code(dev, UVC_REC_INVALID_CONTROL);
            break;
    }
}

static void _stream_eu_vendor1_ctrl01_set(struct uvc_request_data* resp)
{
    uint32_t i, v;

    v = 0;
    uvc_logi("length = %u\n", resp->length);
    for (i = 0; i < resp->length; i++) {
        v |= (resp->data[i] << (i * 8));
    }

    g_vctrl01_test = v;
    uvc_logi("set val=%u\n", g_vctrl01_test);
}

void stream_event_eu_vendor1_data(int unit_id, int control, struct uvc_request_data *data)
{
    if (unit_id != UVC_EXTENSION_UNIT_VENDOR1_ID) {
        uvc_loge("Invalid unit.\n");
        return;
    }

    switch (control) {
        case XUID_VENDOR_01:
            _stream_eu_vendor1_ctrl01_set(data);
            break;
        default:
            uvc_loge("Invalid control.\n");
            break;
    }
}

/* stream Extension Unit vendor1 Operation Functions End */

/*******************************************************
 *  stream Extension Unit H264 Operation Functions
 *******************************************************/
static void _stream_eu_h264_picture_type_setup(uint8_t req, struct uvc_request_data* resp)
{
    switch (req) {
        case UVC_SET_CUR:
            resp->length = 4;
            break;
        case UVC_GET_LEN:
            resp->data[0] = 0x04;
            resp->data[1] = 0x00;
            resp->length = 2;
            break;
        case UVC_GET_INFO:
            resp->data[0] = 0x03;
            resp->length = 1;
            break;
        default:
            uvc_loge("Invalid request.\n");
            break;
    }
}

void stream_event_eu_h264_setup(uint8_t req,
        uint8_t unit_id,
        uint8_t cs,
        struct uvc_request_data* resp)
{
    if (unit_id != UVC_EXTENSION_UNIT_H264_ID) {
        uvc_loge("Invalid unit.\n");
        return;
    }

    switch (cs) {
        case UVCX_PICTURE_TYPE_CONTROL:
            _stream_eu_h264_picture_type_setup(req, resp);
            break;
        default:
            _stream_event_default_setup(req, resp);
            break;
    }
}

void stream_event_eu_h264_data(int unit_id, int control, struct uvc_request_data *data)
{
    if (unit_id != UVC_EXTENSION_UNIT_H264_ID) {
        uvc_loge("Invalid unit.\n");
        return;
    }

    switch (control) {
        case UVCX_PICTURE_TYPE_CONTROL:
            /*todo*/
        default:
            uvc_loge("Invalid control.\n");
            break;
    }
}

/* stream Extension Unit H264 Operation Functions End */

/*******************************************************
 *  stream Extension Unit Camera Operation Functions
 *******************************************************/
static void _stream_eu_camera_iframe_setup(uint8_t req,
        struct uvc_request_data *resp)
{
    switch (req) {
        case UVC_SET_CUR:
            resp->length = 4;
            break;
        case UVC_GET_LEN:
            resp->data[0] = 0x04;
            resp->data[1] = 0x00;
            resp->length = 2;
            break;
        case UVC_GET_INFO:
            resp->data[0] = 0x03;
            resp->length = 1;
            break;
        default:
            uvc_loge("Invalid request.\n");
            break;
    }
}

static void _stream_eu_camera_stream_setup(uint8_t req,
        struct uvc_request_data *resp)
{
    switch (req) {
        case UVC_SET_CUR:
            resp->length = 4;
            break;
        case UVC_GET_LEN:
            resp->data[0] = 0x04;
            resp->data[1] = 0x00;
            resp->length = 2;
            break;
        case UVC_GET_INFO:
            resp->data[0] = 0x03;
            resp->length = 1;
            break;
        default:
            uvc_loge("Invalid request.\n");
            break;
    }
}

static void _stream_eu_camera_reset_setup(uint8_t req,
        struct uvc_request_data *resp)
{
    switch (req) {
        case UVC_SET_CUR:
            resp->length = 4;
            break;
        case UVC_GET_LEN:
            resp->data[0] = 0x04;
            resp->data[1] = 0x00;
            resp->length = 2;
            break;
        case UVC_GET_INFO:
            resp->data[0] = 0x03;
            resp->length = 1;
            break;
        default:
            uvc_loge("Invalid request.\n");
            break;
    }
}

static void _stream_eu_camera_update_system(uint8_t req,
        struct uvc_request_data *resp)
{
    switch (req) {
        case UVC_SET_CUR:
            resp->length = 4;
            break;
        case UVC_GET_LEN:
            resp->data[0] = 0x04;
            resp->data[1] = 0x00;
            resp->length = 2;
            break;
        case UVC_GET_INFO:
            resp->data[0] = 0x03;
            resp->length = 1;
            break;
        default:
            uvc_loge("Invalid request.\n");
            break;
    }
}

void stream_event_eu_camera_setup(uint8_t req,
        uint8_t unit_id,
        uint8_t cs,
        struct uvc_request_data* resp)
{
    if (unit_id != UVC_EXTENSION_UNIT_CAMERA_ID) {
        uvc_loge("Invalid unit.\n");
        return;
    }

    switch (cs) {
        case XUID_SET_RESET:
            _stream_eu_camera_reset_setup(req, resp);
            break;

        case XUID_SET_STREAM:
            _stream_eu_camera_stream_setup(req, resp);
            break;

        case XUID_SET_IFRAME:
            _stream_eu_camera_iframe_setup(req, resp);
            break;

        case XUID_UPDATE_SYSTEM:
            _stream_eu_camera_update_system(req, resp);
            break;

        default:
            _stream_event_default_setup(req, resp);
            break;
    }
}

void stream_event_eu_camera_data(int unit_id, int control, struct uvc_request_data *data)
{
    if (unit_id != UVC_EXTENSION_UNIT_CAMERA_ID) {
        uvc_loge("Invalid unit.\n");
        return;
    }

    switch (control) {
        case XUID_SET_RESET:
            /*todo*/
            break;
        default:
            break;
    }
}
/* stream Extension Unit Camera Operation Functions End */

/*******************************************************
 *  stream Stream Control Operation Functions
 *******************************************************/
int video_stream_init(void)
{
    if (get_stream()->mpi_sc_ops && get_stream()->mpi_sc_ops->init) {
        return get_stream()->mpi_sc_ops->init();
    }

    return 0;
}

int video_stream_deinit(void)
{
    if (get_stream()->mpi_sc_ops && get_stream()->mpi_sc_ops->init) {
        return get_stream()->mpi_sc_ops->deinit();
    }

    return 0;
}

int video_stream_startup(void)
{
    int ret = -1;

    uvc_logi("UVC stream startup.\n");
    if (get_stream()->mpi_sc_ops && get_stream()->mpi_sc_ops->startup) {
        ret = get_stream()->mpi_sc_ops->startup();
    }

    if (ret == 0) {
        get_stream()->streaming = 1;
        uvc_logi("UVC stream startup ok.\n");
    } else {
        uvc_logw("UVC stream startup fail.\n");
    }

    return ret;
}

int video_stream_shutdown(void)
{
    int ret = 0;

    if (get_stream()->mpi_sc_ops && get_stream()->mpi_sc_ops->shutdown) {
        ret = get_stream()->mpi_sc_ops->shutdown();
    }

    if (ret == 0) {
        get_stream()->streaming = 0;
    }

    return ret;
}

int video_stream_set_enc_property(struct encoder_property *p)
{
    if (get_stream()->mpi_sc_ops && get_stream()->mpi_sc_ops->set_property) {
        return get_stream()->mpi_sc_ops->set_property(p);
    }

    return 0;
}

int video_stream_set_enc_idr(void)
{
    if (get_stream()->mpi_sc_ops && get_stream()->mpi_sc_ops->set_idr) {
        return get_stream()->mpi_sc_ops->set_idr();
    }

    return 0;
}

unsigned int stream_pu_get_brightness(void)
{
    unsigned int val = 50;

    if (get_stream()->streaming == 1) {
        if (get_stream()->mpi_pu_ops && get_stream()->mpi_pu_ops->brightness_get) {
            val = get_stream()->mpi_pu_ops->brightness_get();
        }
    }

    return val;
}

void stream_pu_set_brightness(unsigned int val)
{
    if (val > 0x64 || val < 0) {
        get_stream()->brightness_stall = 1;
    }

    if (get_stream()->mpi_pu_ops && get_stream()->mpi_pu_ops->brightness_set) {
        get_stream()->mpi_pu_ops->brightness_set(val);
    }

    return;
}

unsigned int stream_pu_get_contrast(void)
{
    unsigned int val = 50;

    if (get_stream()->streaming == 1) {
        if (get_stream()->mpi_pu_ops && get_stream()->mpi_pu_ops->contrast_get) {
            val = get_stream()->mpi_pu_ops->contrast_get();
        }
    }

    return val;
}

void stream_pu_set_contrast(unsigned int val)
{
    if (get_stream()->mpi_pu_ops && get_stream()->mpi_pu_ops->contrast_set) {
        get_stream()->mpi_pu_ops->contrast_set(val);
    }

    return;
}

int stream_register_mpi_ops(struct stream_control_ops *sc_ops,
        struct processing_unit_ops *pu_ops,
        struct input_terminal_ops *it_ops,
        struct extension_unit_ops *eu_ops)
{
    if (sc_ops) {
        get_stream()->mpi_sc_ops = sc_ops;
    }

    if (pu_ops) {
        get_stream()->mpi_pu_ops = pu_ops;
    }

    if (it_ops) {
        get_stream()->mpi_it_ops = it_ops;
    }

    if (eu_ops) {
        get_stream()->mpi_eu_ops = eu_ops;
    }

    return 0;
}
