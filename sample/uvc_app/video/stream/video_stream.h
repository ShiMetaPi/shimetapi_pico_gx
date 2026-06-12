/*
 * Copyright (c) XMEDIA. All rights reserved.
 */
#ifndef __STREAM_H__
#define __STREAM_H__

#include "uvc_hal.h"
#include "sample_video.h"

//USB_Video_Payload_H_264_1.0 3.3
#define UVCX_PICTURE_TYPE_CONTROL 0x09

// H264 extension unit id, must same as to driver
#define UNIT_XU_H264 10

// CAMERA
#define UNIT_XU_CAMERA (0x11)

// VENDOR
#define UNIT_XU_VENDOR 11

typedef struct stream_control_ops {
    int (*init)(void);
    int (*deinit)(void);
    int (*startup)(void);
    int (*shutdown)(void);
    int (*set_idr)(void);
    int (*set_property)(struct encoder_property *p);
} stream_control_ops_st;

typedef struct processing_unit_ops {
    uint16_t (*brightness_get)(void);
    uint16_t (*contrast_get)(void);
    uint16_t (*hue_get)(void);
    uint8_t (*power_line_frequency_get)(void);
    uint16_t (*saturation_get)(void);
    uint8_t (*white_balance_temperature_auto_get)(void);
    uint16_t (*white_balance_temperature_get)(void);

    void (*brightness_set)(uint16_t v);
    void (*contrast_set)(uint16_t v);
    void (*hue_set)(uint16_t v);
    void (*power_line_frequency_set)(uint8_t v);
    void (*saturation_set)(uint16_t v);
    void (*white_balance_temperature_auto_set)(uint8_t v);
    void (*white_balance_temperature_set)(uint16_t v);
} processing_unit_ops_st;

typedef struct input_terminal_ops {
    uint32_t (*exposure_ansolute_time_get)(void);
    uint8_t (*exposure_auto_mode_get)(void);
    void (*exposure_ansolute_time_set)(uint32_t v);
    void (*exposure_auto_mode_set)(uint8_t v);
} input_terminal_ops_st;

typedef struct extension_unit_ops {
    /*todo eu-h264 ops*/

    /*todo eu-camera ops*/
} extension_unit_ops_st;

typedef struct stream
{
    struct stream_control_ops *mpi_sc_ops;
    struct processing_unit_ops *mpi_pu_ops;
    struct input_terminal_ops *mpi_it_ops;
    struct extension_unit_ops *mpi_eu_ops;
    int streaming;
    int exposure_auto_stall;
    int brightness_stall;
} stream;

/* media control functions */
extern int stream_register_mpi_ops(struct stream_control_ops *sc_ops,
        struct processing_unit_ops *pu_ops,
        struct input_terminal_ops *it_ops,
        struct extension_unit_ops *eu_ops);

extern int video_stream_set_enc_property(struct encoder_property *p);
extern int video_stream_init(void);
extern int video_stream_deinit(void);
extern int video_stream_shutdown(void);
extern int video_stream_startup(void);
extern int video_stream_set_enc_idr(void);

extern unsigned int stream_pu_get_brightness(void);
extern void stream_pu_set_brightness(unsigned int val);
extern unsigned int stream_pu_get_contrast(void);
extern void stream_pu_set_contrast(unsigned int val);

extern void stream_event_pu_setup(struct uvc_device *dev,
        uint8_t req,
        uint8_t unit_id,
        uint8_t cs,
        struct uvc_request_data* resp);

extern void stream_event_it_setup(struct uvc_device *dev,
        uint8_t req,
        uint8_t unit_id,
        uint8_t cs,
        struct uvc_request_data *resp);

extern void stream_event_eu_h264_setup(uint8_t req,
        uint8_t unit_id,
        uint8_t cs,
        struct uvc_request_data *resp);

extern void stream_event_eu_camera_setup(uint8_t req,
        uint8_t unit_id,
        uint8_t cs,
        struct uvc_request_data *resp);

extern void stream_event_eu_vendor1_setup(struct uvc_device *dev,
		uint8_t req,
        uint8_t unit_id,
        uint8_t cs,
        struct uvc_request_data *resp);

extern void stream_event_eu_h264_data(int unit_id, int control, struct uvc_request_data *data);
extern void stream_event_pu_data(int unit_id, int control, struct uvc_request_data *data);
extern void stream_event_it_data(int unit_id, int control, struct uvc_request_data *data);
extern void stream_event_eu_camera_data(int unit_id, int control, struct uvc_request_data *data);
extern void stream_event_eu_vendor1_data(int unit_id, int control, struct uvc_request_data *data);


#endif //__STREAM_H__

